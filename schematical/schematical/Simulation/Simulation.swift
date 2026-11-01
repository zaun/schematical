//
//  Simulation.swift
//  schematical
//
//  Connects the schematic to the backend simulator (circuit.h). The circuit
//  is built from the wires, one simulation runs while the circuit is in play
//  mode, and its state is read back after every step for the canvas to draw.
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
    /// Each connected pin, once: part index, pin number (1-based), net index.
    let connections: [(part: Int, pin: Int, net: Int)]
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

        var connections: [(part: Int, pin: Int, net: Int)] = []
        for (ref, net) in pinNets.sorted(by: { ($0.key.number, $0.key.chipID.uuidString) < ($1.key.number, $1.key.chipID.uuidString) }) {
            guard let part = partIndexByChip[ref.chipID] else { continue }
            connections.append((part: part, pin: ref.number, net: net))
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
struct MemorySize: Equatable {
    let words: Int
    let bits: Int
}

/// One memory word. `known` has a 1 for every bit that is defined; the others are X.
struct MemoryWord: Equatable {
    let value: UInt32
    let known: UInt32
}

/// Owns one running simulation. Play mode starts it, Stop frees it.
@MainActor
@Observable
final class SimulationController {
    /// Simulated time per timer tick, in picoseconds. 10 us per 16 ms tick.
    static let picosecondsPerTick: UInt64 = 10_000_000
    static let tickInterval: Duration = .milliseconds(16)

    /// True while in play mode, including after the simulation has stopped with
    /// an error. Stop leaves play mode.
    private(set) var isRunning = false
    /// Why the simulation stopped, if it did.
    private(set) var errorMessage: String?

    /// The part library circuits are built from. It outlives every circuit.
    private let library: PartLibraryHandle
    private var netlist: SchematicNetlist?
    private var circuit: OpaquePointer?
    private var task: Task<Void, Never>?
    private var netValues: [CircuitValue] = []
    private var shortedNets: Set<Int> = []
    /// LEDs (one light) and 7-segment displays (one per LED, in display
    /// order): whether each light was on during the last step.
    private(set) var lights: [UUID: [Bool]] = [:]
    /// Goes up after every read of the simulator's state, so views that show
    /// memory contents know to read them again.
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

    func start(document: SchematicDocument) {
        stop()
        errorMessage = nil

        let built = NetlistBuilder.build(chips: document.chips, wires: document.wires)
        guard let opened = circuit_create(library.pointer) else {
            errorMessage = "The simulation could not start."
            return
        }
        if !Self.build(opened, from: built) {
            errorMessage = circuit_error(opened).map { String(cString: $0) }
                ?? "The simulation could not start."
            circuit_destroy(opened)
            return
        }

        netlist = built
        circuit = opened
        isRunning = true
        refresh()

        task = Task { [weak self] in
            while !Task.isCancelled {
                guard let self else { return }
                if !self.step() { return }
                try? await Task.sleep(for: Self.tickInterval)
            }
        }
    }

    /// Adds the parts, nets and connections, sets the switches, and starts.
    private static func build(_ circuit: OpaquePointer, from netlist: SchematicNetlist) -> Bool {
        for name in netlist.partNames {
            var index = 0
            if circuit_add_part(circuit, name, &index) != 0 { return false }
        }
        for _ in 0..<netlist.netCount {
            var index = 0
            if circuit_add_net(circuit, &index) != 0 { return false }
        }
        for connection in netlist.connections {
            if circuit_connect(circuit, connection.part, UInt32(connection.pin), connection.net) != 0 {
                return false
            }
        }
        for item in netlist.switchesOn {
            if circuit_set_switch(circuit, item.part, UInt32(item.position), 1) != 0 { return false }
        }
        return circuit_start(circuit) == 0
    }

    func stop() {
        task?.cancel()
        task = nil
        if let circuit {
            circuit_destroy(circuit)
        }
        circuit = nil
        netlist = nil
        isRunning = false
        netValues = []
        shortedNets = []
        lights = [:]
    }

    /// Advances the simulation one tick. Returns false when it has stopped.
    private func step() -> Bool {
        guard let circuit else { return false }
        let ok = circuit_advance(circuit, Self.picosecondsPerTick) != 0
        refresh()
        return ok && errorMessage == nil
    }

    /// Reads the simulator's state into the observable properties.
    private func refresh() {
        guard let circuit, let netlist else { return }

        netValues = (0..<netlist.netCount).map { circuit_net_value(circuit, $0) }

        var shorted: Set<Int> = []
        var net = 0
        if circuit_shorted_net(circuit, &net) != 0 {
            shorted.insert(net)
        }
        shortedNets = shorted

        var newLights: [UUID: [Bool]] = [:]
        var buffer = [UInt8](repeating: 0, count: 64)
        for (index, chipID) in netlist.chipIDs.enumerated() {
            let count = circuit_part_lights(circuit, index, &buffer, buffer.count)
            if count > 0 {
                newLights[chipID] = buffer.prefix(min(count, buffer.count)).map { $0 != 0 }
            }
        }
        lights = newLights

        if let error = circuit_error(circuit) {
            errorMessage = String(cString: error)
        }
        stateVersion &+= 1
    }

    // MARK: Switches

    /// Turns a switch position (1-based) ON or OFF in the running simulation.
    /// It takes effect on the next tick. Does nothing when not running.
    func setSwitch(_ chipID: UUID, position: Int, on: Bool) {
        guard let circuit, let part = netlist?.chipIDs.firstIndex(of: chipID), position > 0 else { return }
        circuit_set_switch(circuit, part, UInt32(position), on ? 1 : 0)
    }

    // MARK: Memory

    /// Size of a memory part in the running simulation, or nil when the part is
    /// not a memory or the simulation is not running.
    func memorySize(of chipID: UUID) -> MemorySize? {
        guard let circuit, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return nil }
        var words: UInt32 = 0
        var bits: UInt32 = 0
        guard circuit_memory_size(circuit, part, &words, &bits) != 0 else { return nil }
        return MemorySize(words: Int(words), bits: Int(bits))
    }

    /// Words of a memory part, from `start`. Fewer than `count` at the end of
    /// the memory; empty when the simulation is not running.
    func readMemory(of chipID: UUID, start: Int, count: Int) -> [MemoryWord] {
        guard let circuit, let part = netlist?.chipIDs.firstIndex(of: chipID),
              start >= 0, count > 0, start <= Int(UInt32.max) else { return [] }
        var values = [UInt32](repeating: 0, count: count)
        var known = [UInt32](repeating: 0, count: count)
        let read = circuit_memory_read(circuit, part, UInt32(start), count, &values, &known)
        return (0..<read).map { MemoryWord(value: values[$0], known: known[$0]) }
    }

    /// Color for a wire in play mode. Outside play mode, the normal color.
    func wireColor(_ wireID: UUID) -> Color {
        guard isRunning, let net = netlist?.netOfWire[wireID] else { return .primary }
        if shortedNets.contains(net) { return .red }
        switch netValue(net) {
        case CIRCUIT_1: return .green
        case CIRCUIT_X: return .orange
        case CIRCUIT_Z: return .secondary
        default: return .primary
        }
    }

    /// True when a wire is floating (no driver), drawn dashed in play mode.
    func wireIsFloating(_ wireID: UUID) -> Bool {
        guard isRunning, let net = netlist?.netOfWire[wireID] else { return false }
        return netValue(net) == CIRCUIT_Z
    }

    private func netValue(_ net: Int) -> CircuitValue {
        net < netValues.count ? netValues[net] : CIRCUIT_Z
    }
}
