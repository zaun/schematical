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
    /// Closed switch contacts (1-based positions), per part index: DIP
    /// positions that are ON; a tactile switch's contact while closed and its
    /// joined pairs always (plan-27-J). The voltage check reads them all.
    let switchesOn: [(part: Int, position: Int)]
    /// Positions `circuit_set_switch` can address, per part index (a DIP
    /// switch's position count, 1 for a tactile switch or a fuse (its
    /// contact: ON = intact, plan-47-M), 0 otherwise). A
    /// tactile switch's joined pairs are numbered after them and are the
    /// backend's own, never sent.
    var positionCounts: [Int] = []
    /// A slide switch's knob position (1-based), per slide switch part index
    /// (plan-58-G). `switchesOn` lists its closed contacts for the voltage
    /// check; the simulation starts it with `circuit_set_switch_state`
    /// (`positionCounts` is 0 for it), which also opens the rest position's
    /// contacts the knob is not at.
    var slideStates: [(part: Int, state: Int)] = []
    /// A placed `GENERIC-RESISTOR`'s own value in ohms, per part index (nil
    /// for every other part, and for a generic resistor still at its part
    /// value). The voltage check reads them for an adjustable regulator's
    /// feedback divider (plan-42-I); a pull resistor's is not sent (a pull is
    /// a supply, not a resistor link).
    var ohms: [Double?] = []
    /// Chip for each part index.
    let chipIDs: [UUID]
    /// Net index for each wire.
    let netOfWire: [UUID: Int]
    /// Part indices of the character LCDs (`PART_CHARLCD`), whose dot image
    /// the engine reads each frame.
    var lcdParts: [Int] = []
    /// Part indices of the fuses and PTCs (`PART_FUSE`), whose one position
    /// is the contact a click blows (plan-47-M).
    var fuseParts: [Int] = []

    /// The buzzers (`PART_BUZZER`) of the circuit, whose tone `BuzzerTone`
    /// reads each frame (plan-47-N).
    var buzzers: [BuzzerTone.Buzzer] = []

    func isFuse(part: Int) -> Bool { fuseParts.contains(part) }
}

enum NetlistBuilder {
    /// Two wire ends are the same electrical node when a wire joins them, or
    /// when they meet at a junction. Every bus port pin for the same bus wire,
    /// on any port of that bus or of a bus linked with it (same name), is
    /// one node too, and so is every net label point of one name (case
    /// sensitive). A bus wire's pins join the net labels of the wire's name
    /// (case sensitive), so bus wires of one name on different buses meet
    /// only through such a label, or through a supply of that name: a bus
    /// wire named like a supply (`5V`) is on that rail. A No Connect joins nothing: a wire
    /// ending on one joins nothing through it, so its pin stays on a net of
    /// its own. Union-find over the wire ends gives the nets.
    static func build(chips: [PlacedChip], wires: [Wire], buses: [Bus],
                      labels: [NetLabel] = []) -> SchematicNetlist {
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

        let noConnects = Set(labels.filter { !$0.isNet }.map(\.id))
        func isNoConnect(_ end: WireEnd) -> Bool {
            if case let .label(id) = end { return noConnects.contains(id) }
            return false
        }
        for wire in wires where !isNoConnect(wire.start) && !isNoConnect(wire.end) {
            union(wire.start, wire.end)
        }

        // Port pins that some wire ends on, grouped by bus wire: each group is
        // one net, whichever line of the bus its ports are on, and on every
        // bus linked with it (same name). Unwired pins add nothing.
        let portBus = Dictionary(buses.flatMap { bus in bus.ports.map { ($0.id, bus.id) } },
                                 uniquingKeysWith: { first, _ in first })
        let linked = Bus.linkedWireIDs(buses)
        var groups: [UUID: [WireEnd]] = [:]
        var seen = Set<WireEnd>()
        for wire in wires {
            for end in [wire.start, wire.end] {
                guard case let .busPin(ref) = end, portBus[ref.portID] != nil,
                      seen.insert(end).inserted else { continue }
                groups[linked[ref.wireID] ?? ref.wireID, default: []].append(end)
            }
        }
        for group in groups.values {
            for end in group.dropFirst() {
                union(group[0], end)
            }
        }

        // Net labels of one name are one node, whether or not wires reach
        // them all.
        var labelGroups: [String: [WireEnd]] = [:]
        for label in labels where label.isNet {
            labelGroups[label.netKey, default: []].append(.label(label.id))
        }
        for group in labelGroups.values {
            for end in group.dropFirst() {
                union(group[0], end)
            }
        }

        // A bus wire is on the net of the labels of its name (case and all):
        // its wired port pins join them.
        var busWireName: [UUID: String] = [:]
        for bus in buses {
            for wire in bus.wires { busWireName[wire.id] = wire.name }
        }
        for (wireID, group) in groups {
            if let name = busWireName[wireID], let label = labelGroups[name]?.first {
                union(group[0], label)
            }
        }

        // A net label or a bus wire named like a supply (GND, 5V, 3V3, …: a
        // power symbol's part number, case sensitive) is on that rail: it
        // joins every wired pin of the supply symbols of that name. A bus
        // wire is treated as a net label of its name here, with or without
        // a label of that name. Pull-up and pull-down symbols are not
        // supplies here.
        var supplyOfChip: [UUID: String] = [:]
        for chip in chips {
            if case .supply(_, _, false) = chip.part.details { supplyOfChip[chip.id] = chip.partNumber }
        }
        if !supplyOfChip.isEmpty {
            var supplyPins: [String: [WireEnd]] = [:]
            for wire in wires {
                for end in [wire.start, wire.end] {
                    if case let .pin(ref) = end, let name = supplyOfChip[ref.chipID] {
                        supplyPins[name, default: []].append(end)
                    }
                }
            }
            for (name, pins) in supplyPins {
                var anchors: [WireEnd] = []
                if let label = labelGroups[name]?.first { anchors.append(label) }
                for (wireID, group) in groups where busWireName[wireID] == name { anchors.append(group[0]) }
                for anchor in anchors {
                    for pin in pins { union(anchor, pin) }
                }
            }
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
            // A wire to a No Connect is on its other end's net.
            netOfWire[wire.id] = netIndex(of: isNoConnect(wire.start) ? wire.end : wire.start)
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
        // A tactile switch with joined pins: each pin no wire reaches gets a
        // net of its own, so the chain A1 - B1 - B2 holds in the simulation
        // and the voltage check alike (the backend does the same in
        // circuit_start for circuits built without the app).
        var netCount = netIndexByRoot.count
        let wired = Set(connections.map { "\($0.part)/\($0.pin)" })
        for (index, chip) in chips.enumerated() {
            guard case let .tactileSwitch(_, _, _, joined, _) = chip.part.details, joined > 0 else { continue }
            for pin in chip.part.pins where !pin.isHidden && !wired.contains("\(index)/\(pin.designator)") {
                connections.append((part: index, pin: pin.designator, net: netCount))
                netCount += 1
            }
        }

        // Closed contacts. DIP positions that are ON (the rest start OFF);
        // a tactile switch's contact when closed, and its joined pairs.
        var switchesOn: [(part: Int, position: Int)] = []
        var positionCounts: [Int] = []
        var slideStates: [(part: Int, state: Int)] = []
        for (index, chip) in chips.enumerated() {
            if let state = chip.slidePosition {
                for (contact, isOn) in chip.switchesOn.enumerated() where isOn {
                    switchesOn.append((part: index, position: contact + 1))
                }
                slideStates.append((part: index, state: state))
                positionCounts.append(0)
                continue
            }
            if case let .tactileSwitch(_, _, _, joined, _) = chip.part.details {
                if chip.tactileContactClosed {
                    switchesOn.append((part: index, position: 1))
                }
                for pair in 0..<joined {
                    switchesOn.append((part: index, position: 2 + pair))
                }
                positionCounts.append(1)
                continue
            }
            for (position, isOn) in chip.switchesOn.enumerated() where isOn {
                switchesOn.append((part: index, position: position + 1))
            }
            // A fuse's one position is its contact; it is closed from the
            // start and the voltage check never sees a blown one (plan-47-I).
            positionCounts.append(chip.part.kind == PART_FUSE ? 1 : chip.switchesOn.count)
        }

        // Buzzers: the net of the `+` pin, for the tone of a transducer.
        var buzzers: [BuzzerTone.Buzzer] = []
        for (index, chip) in chips.enumerated() where chip.part.kind == PART_BUZZER {
            guard let info = chip.part.buzzer else { continue }
            let plus = chip.part.pins.first { $0.name == "+" }
            let net = plus.flatMap { pin in
                connections.first { $0.part == index && $0.pin == pin.designator }?.net
            }
            buzzers.append(BuzzerTone.Buzzer(chip: chip.id, part: index, isTransducer: info.driver == "transducer",
                                             hz: Double(info.hz), plusNet: net))
        }

        return SchematicNetlist(
            partNames: chips.map(\.partNumber),
            netCount: netCount,
            connections: connections,
            switchesOn: switchesOn,
            positionCounts: positionCounts,
            slideStates: slideStates,
            ohms: chips.map { $0.part.isGenericResistor ? $0.ohms : nil },
            chipIDs: chips.map(\.id),
            netOfWire: netOfWire,
            lcdParts: chips.indices.filter { chips[$0].part.kind == PART_CHARLCD },
            fuseParts: chips.indices.filter { chips[$0].part.kind == PART_FUSE },
            buzzers: buzzers
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
    /// Edit mode: a wire with a loose end (plan-30). Gray on screen; the
    /// export scene never uses it, so exports stay black.
    case loose
}

/// Owns one running simulation. Play mode starts it, Stop frees it.
@MainActor
@Observable
final class SimulationController {
    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

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
    /// The fuses blown with a click in this run (plan-47-M). Play-mode state
    /// only: empty at every start, cleared at stop, never saved.
    private(set) var blownFuses: Set<UUID> = []
    /// The buzzers that sound in the last frame, each with its pitch in hertz
    /// (plan-47-N). Empty outside play mode. `BuzzerSound` plays them unless
    /// the setting is off or the process is silenced.
    private(set) var buzzerVoices: [UUID: Double] = [:]
    /// The snapshots of the last `BuzzerTone.windowPS` of circuit time, whose
    /// oldest gives a steady pitch to a transducer.
    private var toneHistory: [SimulationSnapshot] = []
    private var sound: BuzzerSound?
    /// Counts engines started, so a snapshot from a stopped one is ignored.
    private var generation = 0
    private var netValues: [CircuitValue] = []
    private var shortedNets: Set<Int> = []
    /// LEDs (one light) and 7-segment displays (one per LED, in display
    /// order): brightness 0…1, the light's on-time in the last frame divided
    /// by the frame's length.
    private(set) var lights: [UUID: [Double]] = [:]
    /// Character LCDs: the dot image of the last frame.
    private(set) var lcdFrames: [UUID: LCDFrame] = [:]
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
        let netlist = NetlistBuilder.build(chips: document.chips, wires: document.wires, buses: document.buses,
                                           labels: document.netLabels)
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
        if isRunning {
            playDocument = document
            playSwitches = document.switchPositions
            // Nothing done during the run is an edit (plan-31).
            document.isPlaying = true
        }
    }

    /// The document of the run started by `start(document:)`, and its switch
    /// positions at the start. Stop puts them back.
    private weak var playDocument: SchematicDocument?
    private var playSwitches: [UUID: [Bool]] = [:]

    /// Builds the circuit and starts it on the simulation thread. `loads`
    /// are memory contents written before the circuit starts, by part index.
    func start(netlist built: SchematicNetlist, loads: [(part: Int, values: [UInt32])] = []) {
        stop()
        errorMessage = nil

        guard let opened = circuit_create(library.pointer) else {
            errorMessage = "The simulation could not start."
            return
        }
        if !Self.build(opened, from: built, loads: loads, ports: serialPorts) {
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
            circuit: opened, netCount: built.netCount, lightParts: lightParts,
            lcdParts: built.lcdParts, speed: speed,
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
                              loads: [(part: Int, values: [UInt32])],
                              ports: [UUID: [Int: SerialPort]] = [:]) -> Bool {
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
        // Only addressable positions: a tactile switch's joined pairs are the
        // backend's (always closed). A tactile switch starts released (the
        // run starts with every button up), so its contact is already in the
        // backend's default state; sending a closed one again is harmless.
        for item in netlist.switchesOn where item.position <= netlist.positionCounts[item.part] {
            if circuit_set_switch(circuit, item.part, UInt32(item.position), 1) != 0 { return false }
        }
        // Slide switches start in the knob position they are drawn in (the
        // backend's own start is the part's rest position).
        for item in netlist.slideStates {
            if circuit_set_switch_state(circuit, item.part, UInt32(item.state)) != 0 { return false }
        }
        // Placed resistor values, for the voltage check's feedback divider
        // (plan-42-I). A value the backend refuses (outside 0–1e9 Ω) leaves
        // the part's own.
        for (part, ohms) in netlist.ohms.enumerated() {
            if let ohms { _ = circuit_set_resistance(circuit, part, ohms) }
        }
        for load in loads {
            let result = load.values.withUnsafeBufferPointer {
                circuit_memory_load(circuit, load.part, 0, $0.baseAddress, $0.count)
            }
            if result != 0 { return false }
        }
        // USB bridges: the ports the terminal set (a part no longer a bridge is skipped).
        for (chipID, channels) in ports {
            guard let part = netlist.chipIDs.firstIndex(of: chipID) else { continue }
            for (channel, port) in channels where channel < circuit_serial_channels(circuit, part) {
                _ = port.apply(to: circuit, part: part, channel: channel)
            }
        }
        return circuit_start(circuit) == 0
    }

    /// Leaves play mode. Switches flipped during the run go back to where
    /// they were when it started.
    func stop() {
        playDocument?.restoreSwitches(playSwitches)
        playDocument?.isPlaying = false
        playDocument = nil
        playSwitches = [:]
        engine?.stopAndWait()
        engine = nil
        netlist = nil
        isRunning = false
        netValues = []
        shortedNets = []
        blownFuses = []
        buzzerVoices = [:]
        toneHistory = []
        sound?.stop()
        sound = nil
        lights = [:]
        lcdFrames = [:]
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
        updateBuzzers(snapshot, netlist: netlist)
        var newFrames: [UUID: LCDFrame] = [:]
        for (part, frame) in snapshot.lcdFrames where part < netlist.chipIDs.count {
            newFrames[netlist.chipIDs[part]] = frame
        }
        lcdFrames = newFrames
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

    /// Works out which buzzers sound in this frame and plays them (plan-47-N).
    /// A snapshot with an error silences every buzzer: the circuit stopped.
    private func updateBuzzers(_ snapshot: SimulationSnapshot, netlist: SchematicNetlist) {
        guard !netlist.buzzers.isEmpty else {
            buzzerVoices = [:]
            return
        }
        // The oldest snapshot within the window gives a transducer's pitch.
        toneHistory.append(snapshot)
        toneHistory.removeAll { snapshot.circuitTime > $0.circuitTime && snapshot.circuitTime - $0.circuitTime > BuzzerTone.windowPS }
        let window = toneHistory.first { $0.circuitTime < snapshot.circuitTime }
        buzzerVoices = BuzzerTone.voices(netlist: netlist, snapshot: snapshot, previous: previous, window: window)
        guard BuzzerSoundSetting.mayPlay else {
            sound?.stop()
            sound = nil
            return
        }
        if sound == nil, !buzzerVoices.isEmpty { sound = BuzzerSound() }
        sound?.update(buzzerVoices)
    }

    // MARK: Switches

    /// Turns a switch position (1-based) ON or OFF in the running simulation.
    /// It takes effect on the next step. Does nothing when not running.
    func setSwitch(_ chipID: UUID, position: Int, on: Bool) {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID), position > 0 else { return }
        engine.setSwitch(part: part, position: position, on: on)
    }

    /// Moves a slide switch's knob to position `state` (1-based) in the running
    /// simulation: the contacts it does not close open, then the ones it does
    /// close (`circuit_set_switch_state`). Does nothing when not running.
    func setSlide(_ chipID: UUID, state: Int) {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID), state > 0 else { return }
        engine.setSlide(part: part, state: state)
    }

    /// Blows the fuse `chipID`, or restores it when it is blown, in the
    /// running simulation (plan-47-M): position 1 of a fuse is its contact,
    /// ON = intact. Returns whether the fuse is blown now; nil when the
    /// simulation is not running or the chip is not a fuse. A fuse is
    /// blown only for the run: `stop` clears it.
    @discardableResult
    func toggleFuse(_ chipID: UUID) -> Bool? {
        let blown = !blownFuses.contains(chipID)
        return setFuse(chipID, blown: blown) ? blown : nil
    }

    /// Blows (`blown`) or restores the fuse `chipID` (see `toggleFuse`);
    /// false, and nothing changes, when the simulation is not running or
    /// the chip is not a fuse.
    @discardableResult
    func setFuse(_ chipID: UUID, blown: Bool) -> Bool {
        guard engine != nil, let netlist, let part = netlist.chipIDs.firstIndex(of: chipID),
              netlist.positionCounts.indices.contains(part), netlist.isFuse(part: part) else { return false }
        if blown { blownFuses.insert(chipID) } else { blownFuses.remove(chipID) }
        setSwitch(chipID, position: 1, on: !blown)
        return true
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

    // MARK: Serial terminal

    /// A part's serial channels in the running simulation (0 when not running).
    func serialChannels(of chipID: UUID) -> Int {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return 0 }
        return engine.serialChannels(part: part)
    }

    /// Frames a channel has sent since the run started.
    func serialSentCount(of chipID: UUID, channel: Int) -> Int {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return 0 }
        return engine.serialSentCount(part: part, channel: channel)
    }

    /// Frames [from, from + count) of a channel.
    func serialSent(of chipID: UUID, channel: Int, from: Int, count: Int) -> [UInt16] {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return [] }
        return engine.serialSent(part: part, channel: channel, from: from, count: count)
    }

    /// USB-to-UART bridges: each channel's port, as the PC set it in the
    /// terminal (chip → channel → port). Kept for the session, across runs.
    private(set) var serialPorts: [UUID: [Int: SerialPort]] = [:]

    /// A bridge channel's port (the default until the terminal sets one).
    func serialPort(of chipID: UUID, channel: Int) -> SerialPort {
        serialPorts[chipID]?[channel] ?? SerialPort()
    }

    /// Sets a bridge channel's port, now when running and at every later
    /// start: nil when taken, else the reason.
    func setSerialPort(_ port: SerialPort, of chipID: UUID, channel: Int) -> String? {
        serialPorts[chipID, default: [:]][channel] = port
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return nil }
        return engine.serialConfigure(part: part, channel: channel, port: port)
    }

    /// Types bytes into a channel: nil when queued, else the reason it was refused.
    func serialSend(to chipID: UUID, channel: Int, bytes: [UInt8]) -> String? {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else {
            return "The simulation is not running."
        }
        return engine.serialSend(part: part, channel: channel, bytes: bytes)
    }

    // MARK: Processors

    /// A processor part's state in the running simulation, or nil when the
    /// part is not a processor or the simulation is not running.
    func cpuState(of chipID: UUID) -> CpuSnapshot? {
        guard let engine, let part = netlist?.chipIDs.firstIndex(of: chipID) else { return nil }
        return engine.cpuState(part: part)
    }

    // MARK: Converters, RTCs and display drivers

    /// A converter's, RTC's or display driver's registers (and an RTC's clock) in the running
    /// simulation, or nil when the part has none or the simulation is not
    /// running.
    func registerState(of chip: PlacedChip) -> RegisterSnapshot? {
        guard chip.part.hasRegisters, let engine,
              let part = netlist?.chipIDs.firstIndex(of: chip.id) else { return nil }
        return engine.registerState(part: part, rtc: chip.part.kind == PART_RTC)
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

    /// The logic level ("0", "1", "X" for unknown, "Z" for floating) of the
    /// net on a pin of a part, in the running simulation; nil when it isn't
    /// running or the pin is on no net. Used by the agent-control tools.
    func level(ofPin designator: String, on chipID: UUID) -> String? {
        guard isRunning, let netlist, let part = netlist.chipIDs.firstIndex(of: chipID),
              let connection = netlist.connections.first(where: { $0.part == part && $0.pin == designator })
        else { return nil }
        switch netValue(connection.net) {
        case CIRCUIT_0: return "0"
        case CIRCUIT_1: return "1"
        case CIRCUIT_X: return "X"
        default: return "Z"
        }
    }

    private func netValue(_ net: Int) -> CircuitValue {
        net < netValues.count ? netValues[net] : CIRCUIT_Z
    }
}
