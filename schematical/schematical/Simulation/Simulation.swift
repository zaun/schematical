//
//  Simulation.swift
//  schematical
//
//  Connects the schematic to the backend simulator (circuit.h). The circuit
//  is built from the wires, one simulation runs while the circuit is in play
//  mode on its own thread (SimulationEngine), and the snapshot it publishes
//  each frame becomes what the canvas draws.
//

import Foundation
import SwiftUI

/// The schematic as the backend circuit needs it: the parts in order, which
/// net each connected pin is on, and the lookups needed to map simulation
/// results back onto the canvas.
struct SchematicNetlist {
    /// Part names, in circuit order.
    let partNames: [String]
    /// Number of nets.
    let netCount: Int
    /// Each connected pin, once: part index, pin designator ("1", "A1"), net index.
    let connections: [(part: Int, pin: String, net: Int)]
    /// Switch positions (1-based) that start ON, per part index.
    let switchesOn: [(part: Int, position: Int)]
    /// Chip for each part index.
    let chipIDs: [UUID]
    /// Net index for each wire.
    let netOfWire: [UUID: Int]
}

enum NetlistBuilder {
    /// Two wire ends are the same electrical node when a wire joins them, or
    /// when they meet at a junction. Union-find over the wire ends gives the nets.
    static func build(chips: [PlacedChip], wires: [Wire]) -> SchematicNetlist {
        var parent: [WireEnd: WireEnd] = [:]

        func find(_ end: WireEnd) -> WireEnd {
            var current = end
            while let next = parent[current], next != current {
                current = next
            }
            return current
        }

        func union(_ a: WireEnd, _ b: WireEnd) {
            let rootA = find(a)
            let rootB = find(b)
            if rootA != rootB {
                parent[rootA] = rootB
            }
        }

        for wire in wires {
            union(wire.start, wire.end)
        }

        var netIndexByRoot: [WireEnd: Int] = [:]

        func netIndex(of end: WireEnd) -> Int {
            let root = find(end)
            if let index = netIndexByRoot[root] {
                return index
            }
            let index = netIndexByRoot.count
            netIndexByRoot[root] = index
            return index
        }

        var netOfWire: [UUID: Int] = [:]
        for wire in wires {
            netOfWire[wire.id] = netIndex(of: wire.start)
        }

        var partIndexByChip: [UUID: Int] = [:]
        for (index, chip) in chips.enumerated() {
            partIndexByChip[chip.id] = index
        }

        // Each connected pin, once, with the net it sits on.
        var pinNets: [PinRef: Int] = [:]
        for wire in wires {
            for end in [wire.start, wire.end] {
                if case let .pin(ref) = end {
                    pinNets[ref] = netIndex(of: end)
                }
            }
        }

        var connections: [(part: Int, pin: String, net: Int)] = []
        for (ref, net) in pinNets.sorted(by: { ($0.key.index, $0.key.chipID.uuidString) < ($1.key.index, $1.key.chipID.uuidString) }) {
            guard let part = partIndexByChip[ref.chipID],
                  chips[part].part.pins.indices.contains(ref.index) else { continue }
            connections.append((part: part, pin: chips[part].part.pins[ref.index].designator, net: net))
        }
        // Switch positions that start ON. The rest start OFF.
        var switchesOn: [(part: Int, position: Int)] = []
        for (index, chip) in chips.enumerated() {
            for (position, isOn) in chip.switchesOn.enumerated() where isOn {
                switchesOn.append((part: index, position: position + 1))
            }
        }

        return SchematicNetlist(
            partNames: chips.map(\.partNumber),
            netCount: netIndexByRoot.count,
            connections: connections,
            switchesOn: switchesOn,
            chipIDs: chips.map(\.id),
            netOfWire: netOfWire
        )
    }
}

/// A memory part's size: its word count and word width in bits.
nonisolated struct MemorySize: Equatable, Sendable {
    let words: Int
    let bits: Int
}

/// One memory word. `known` has a 1 for every bit that is defined; the others are X.
nonisolated struct MemoryWord: Equatable, Sendable {
    let value: UInt32
    let known: UInt32
}

/// How a wire is drawn. In order of precedence: a shorted net beats a
/// toggling one, which beats a floating one.
enum WireStyle: Equatable {
    /// Not in play mode, or a net at 0.
    case normal
    case shorted
    /// Changed at least twice since the last frame: too fast to show a value.
    case toggling
    /// No driver.
    case floating
    case value(Color)
}

/// Owns one running simulation. Play mode starts it, Stop frees it.
@MainActor
@Observable
final class SimulationController {
    /// True while in play mode, including after the simulation has stopped with
    /// an error. Stop leaves play mode.
    private(set) var isRunning = false
    /// Why the simulation stopped, if it did.
    private(set) var errorMessage: String?
    /// Circuit time of the last snapshot, picoseconds.
    private(set) var circuitTime: UInt64 = 0
    /// Circuit time per wall time, 1 = real time. An app preference, set by
    /// the toolbar's speed menu.
    private(set) var speed: Double = 1
    /// Speed actually reached over the last half second while running.
    private(set) var reachedSpeed: Double?
    /// Nets that changed at least twice since the previous frame.
    private(set) var togglingNets: Set<Int> = []

    /// The part library circuits are built from. It outlives every circuit.
    private let library: PartLibraryHandle
    private var netlist: SchematicNetlist?
    private var engine: SimulationEngine?
    /// Counts engines started, so a snapshot from a stopped one is ignored.
    private var generation = 0
    private var netValues: [CircuitValue] = []
    private var shortedNets: Set<Int> = []
    /// LEDs (one light) and 7-segment displays (one per LED, in display
    /// order): brightness 0…1, the light's on-time in the last frame divided
    /// by the frame's length.
    private(set) var lights: [UUID: [Double]] = [:]
    /// The previous snapshot, to measure the frame against.
    private var previous: SimulationSnapshot?
    /// (wall time, circuit time) of the last half second's snapshots.
    private var speedHistory: [(wall: TimeInterval, circuit: UInt64)] = []
    /// Goes up after every snapshot, so views that show memory contents know
    /// to read them again.
    private(set) var stateVersion = 0

    init(library: PartLibraryHandle) {
        self.library = library
    }

    func toggle(document: SchematicDocument) {
        if isRunning {
            stop()
        } else {
            start(document: document)
        }
    }

    /// Starts the document's circuit. Each EEPROM with a load file gets the
    /// file's contents first; a file that can't be read or parsed stops the
    /// start with a message naming the part.
    func start(document: SchematicDocument) {
        let netlist = NetlistBuilder.build(chips: document.chips, wires: document.wires)
        var loads: [(part: Int, values: [UInt32])] = []
        for (index, chip) in document.chips.enumerated() where chip.part.kind == PART_EEPROM {
            guard let file = chip.memoryFile, let size = chip.part.memorySize else { continue }
            do {
                loads.append((part: index, values: try MemoryFileAccess.image(file, size: size)))
            } catch {
                stop()
                errorMessage = "\(chip.reference): \(error.localizedDescription)"
                return
            }
        }
        start(netlist: netlist, loads: loads)
    }

    /// Builds the circuit and starts it on the simulation thread. `loads`
    /// are memory contents written before the circuit starts, by part index.
    func start(netlist built: SchematicNetlist, loads: [(part: Int, values: [UInt32])] = []) {
        stop()
        errorMessage = nil

        guard let opened = circuit_create(library.pointer) else {
            errorMessage = "The simulation could not start."
            return
        }
        if !Self.build(opened, from: built, loads: loads) {
            errorMessage = circuit_error(opened).map { String(cString: $0) }
                ?? "The simulation could not start."
            // Refused for voltage: the problem's net is drawn like a short
            // until the next start or stop.
            var problem = VoltageProblem()
            if circuit_voltage_problem(opened, &problem) == 1 {
                netlist = built
                shortedNets = [Int(problem.net)]
            }
            circuit_destroy(opened)
            return
        }

        // Parts with lights, found once: they are the ones the engine reads.
        let lightParts = built.partNames.indices.filter {
            circuit_part_light_time(opened, $0, nil, 0) > 0
        }

        generation += 1
        let current = generation
        let engine = SimulationEngine(
            circuit: opened, netCount: built.netCount, lightParts: lightParts, speed: speed,
            publish: { [weak self] snapshot in
                let owner = self
                Task { @MainActor in owner?.apply(snapshot, generation: current) }
            }
        )
        netlist = built
        self.engine = engine
        isRunning = true
        apply(engine.snapshotNow(), generation: current)
        engine.start()
    }

    /// Adds the parts, nets and connections, sets the switches, loads the
    /// memories, and starts.
    private static func build(_ circuit: OpaquePointer, from netlist: SchematicNetlist,
                              loads: [(part: Int, values: [UInt32])]) -> Bool {
        for name in netlist.partNames {
            var index = 0
            if circuit_add_part(circuit, name, &index) != 0 { return false }
        }
        for _ in 0..<netlist.netCount {
            var index = 0
            if circuit_add_net(circuit, &index) != 0 { return false }
        }
        for connection in netlist.connections {
            if circuit_connect(circuit, connection.part, connection.pin, connection.net) != 0 {
                return false
            }
        }
        for item in netlist.switchesOn {
            if circuit_set_switch(circuit, item.part, UInt32(item.position), 1) != 0 { return false }
        }
        for load in loads {
            let result = load.values.withUnsafeBufferPointer {
                circuit_memory_load(circuit, load.part, 0, $0.baseAddress, $0.count)
            }
            if result != 0 { return false }
        }
        return circuit_start(circuit) == 0
    }

    func stop() {
        engine?.stopAndWait()
        engine = nil
        netlist = nil
        isRunning = false
        netValues = []
        shortedNets = []
        lights = [:]
        previous = nil
        speedHistory = []
        reachedSpeed = nil
        togglingNets = []
        circuitTime = 0
    }

    /// Sets the speed (1 = real time), for this run and the next.
    func setSpeed(_ newSpeed: Double) {
        guard newSpeed > 0 else { return }
        speed = newSpeed
        speedHistory = []
        reachedSpeed = nil
        engine?.setSpeed(newSpeed)
    }

    /// Turns a snapshot from the simulation thread into what the canvas draws.
    private func apply(_ snapshot: SimulationSnapshot, generation snapshotGeneration: Int) {
        guard snapshotGeneration == generation, engine != nil, let netlist else { return }

        netValues = snapshot.netValues
        shortedNets = snapshot.shortedNet.map { [$0] } ?? []
        circuitTime = snapshot.circuitTime

        if let previous, previous.netChanges.count == snapshot.netChanges.count {
            var toggling: Set<Int> = []
            for (net, count) in snapshot.netChanges.enumerated()
            where count &- previous.netChanges[net] >= 2 {
                toggling.insert(net)
            }
            togglingNets = toggling
        } else {
            togglingNets = []
        }

        // Brightness: on-time gained over circuit time passed. The first
        // snapshot only sets the baseline.
        var newLights: [UUID: [Double]] = [:]
        for (part, times) in snapshot.lightTimes where part < netlist.chipIDs.count {
            let chipID = netlist.chipIDs[part]
            guard let previous, let before = previous.lightTimes[part], before.count == times.count else {
                newLights[chipID] = times.map { _ in 0 }
                continue
            }
            guard snapshot.circuitTime > previous.circuitTime else {
                newLights[chipID] = lights[chipID] ?? times.map { _ in 0 }
                continue
            }
            let span = Double(snapshot.circuitTime - previous.circuitTime)
            newLights[chipID] = times.indices.map { i in
                min(1, max(0, Double(times[i] &- before[i]) / span))
            }
        }
        lights = newLights
        previous = snapshot

        speedHistory.append((snapshot.wallTime, snapshot.circuitTime))
        speedHistory.removeAll { snapshot.wallTime - $0.wall > 0.5 }
        if let first = speedHistory.first, snapshot.wallTime > first.wall {
            reachedSpeed = Double(snapshot.circuitTime &- first.circuit) / 1e12
                / (snapshot.wallTime - first.wall)
        }

        if let error = snapshot.error {
            errorMessage = error
        }
        stateVersion &+= 1
    }

    // MARK: Switches

    /// Turns a switch position (1-based) ON or OFF in the running simulation.
    /// It takes effect on the next step. Does nothing when not running.
    func setSwitch(_ chipID: UUID, position: Int, on: Bool) {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID), position > 0 else { return }
        engine.setSwitch(part: part, position: position, on: on)
    }

    // MARK: Memory

    /// Size of a memory part in the running simulation, or nil when the part is
    /// not a memory or the simulation is not running.
    func memorySize(of chipID: UUID) -> MemorySize? {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return nil }
        return engine.memorySize(part: part)
    }

    /// Words of a memory part, from `start`. Fewer than `count` at the end of
    /// the memory; empty when the simulation is not running.
    func readMemory(of chipID: UUID, start: Int, count: Int) -> [MemoryWord] {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return [] }
        return engine.readMemory(part: part, start: start, count: count)
    }

    // MARK: Processors

    /// A processor part's state in the running simulation, or nil when the
    /// part is not a processor or the simulation is not running.
    func cpuState(of chipID: UUID) -> CpuSnapshot? {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return nil }
        return engine.cpuState(part: part)
    }

    /// How to draw a wire in play mode. Outside play mode, `.normal`, except
    /// the net of a start refused for voltage, which is `.shorted`.
    func wireStyle(_ wireID: UUID) -> WireStyle {
        guard let net = netlist?.netOfWire[wireID] else { return .normal }
        if shortedNets.contains(net) { return .shorted }
        guard isRunning else { return .normal }
        if togglingNets.contains(net) { return .toggling }
        switch netValue(net) {
        case CIRCUIT_Z: return .floating
        case CIRCUIT_1: return .value(.green)
        case CIRCUIT_X: return .value(.orange)
        default: return .normal
        }
    }

    private func netValue(_ net: Int) -> CircuitValue {
        net < netValues.count ? netValues[net] : CIRCUIT_Z
    }
}
