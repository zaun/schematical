//
//  ErcChecker.swift
//  schematical
//
//  The electrical rules check (plan-63-E): KiCad's ERC on a schematic
//  drawing. The pin rules (`erc_check`, backend/include/erc.h) run on the
//  pins' KiCad types, the drawing rules (dangling wires and labels, No Connect
//  flags, similar names, duplicate references) here, on the wires and labels.
//  Each violation has the severity the document's `ErcSettings` give its rule,
//  and a stable key an exclusion is kept by.
//

import CoreGraphics
import Foundation

/// One violation of an ERC rule.
nonisolated struct ErcViolation: Identifiable, Hashable, Sendable {
    /// Its line number, from 1.
    var index = 0
    let rule: ErcRule
    /// `error` or `warning`; an `ignore` rule has no violations.
    let severity: ErcSeverity
    /// KiCad's wording of what is wrong.
    let message: String
    /// The part and pin ("U1.14"), the label's name, or "Wire".
    let reference: String
    /// What selecting the line selects on the schematic.
    let items: Set<SchematicItem>
    /// Where the marker goes, in schematic coordinates.
    let point: CGPoint
    /// What an exclusion is kept by: the rule, then the sorted ids of what it names.
    let key: String
    /// The document's exclusions name it (only returned when asked for).
    var isExcluded = false

    var id: Int { index }
}

extension SchematicDocument {
    /// Every violation of the drawing under its ERC settings, errors first, then
    /// by rule, then top to bottom, numbered from 1. `library`: the part library
    /// (the document's own when nil); without one pin types come from the pin
    /// roles. Excluded violations are left out unless `includeExcluded`, which
    /// returns them marked.
    func electricalRulesCheck(library: PartLibraryHandle? = nil, includeExcluded: Bool = false) -> [ErcViolation] {
        var checker = ErcChecker(document: self, library: library ?? self.library)
        return checker.run(includeExcluded: includeExcluded)
    }
}

/// One run of the check. Built on the main actor (it reads the document), run once.
@MainActor
struct ErcChecker {
    let document: SchematicDocument
    let library: PartLibraryHandle?
    let chips: [PlacedChip]
    let wires: [Wire]
    let labels: [NetLabel]
    let buses: [Bus]
    let settings: ErcSettings
    var violations: [ErcViolation] = []

    init(document: SchematicDocument, library: PartLibraryHandle?) {
        self.document = document
        self.library = library
        chips = document.chips
        wires = document.wires
        labels = document.netLabels
        buses = document.buses
        settings = document.erc
    }

    mutating func run(includeExcluded: Bool) -> [ErcViolation] {
        checkPins()
        // Errors first, then by rule, then top to bottom, left to right.
        let ruleOrder = Dictionary(uniqueKeysWithValues: ErcRule.allCases.enumerated().map { ($1, $0) })
        var found = violations.filter { includeExcluded || !settings.exclusions.contains($0.key) }
        found = found.map { violation in
            var marked = violation
            marked.isExcluded = settings.exclusions.contains(violation.key)
            return marked
        }
        found.sort { a, b in
            if a.severity != b.severity { return a.severity == .error }
            if a.rule != b.rule { return ruleOrder[a.rule]! < ruleOrder[b.rule]! }
            if a.point.y != b.point.y { return a.point.y < b.point.y }
            if a.point.x != b.point.x { return a.point.x < b.point.x }
            return a.key < b.key
        }
        for i in found.indices { found[i].index = i + 1 }
        return found
    }

    /// Adds a violation unless its rule is ignored.
    mutating func add(_ rule: ErcRule, severity: ErcSeverity? = nil, message: String, reference: String,
                      items: Set<SchematicItem>, point: CGPoint, ids: [String]) {
        let level = severity ?? settings.severity(of: rule)
        guard level != .ignore else { return }
        let key = ([rule.rawValue] + ids.sorted()).joined(separator: "|")
        violations.append(ErcViolation(rule: rule, severity: level, message: message, reference: reference,
                                       items: items, point: point, key: key))
    }

    // MARK: Pins

    /// What each entry of the `erc_check` input is: a chip's pin or a Power Flag.
    private enum Slot {
        case pin(chip: Int, pin: ChipPin)
        case flag(UUID)
    }

    /// The pin type from the part's role when the library does not know the part.
    private static func fallbackType(_ role: ChipPinRole) -> ErcPinType {
        switch role {
        case .input: ERC_INPUT
        case .output: ERC_OUTPUT
        case .inOut: ERC_BIDIRECTIONAL
        case .power, .ground: ERC_POWER_IN
        case .noConnect: ERC_NO_CONNECT
        case .absent: ERC_PASSIVE
        }
    }

    private static func typeIndex(_ type: ErcPinType) -> Int { Int(type.rawValue) }

    private mutating func checkPins() {
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: labels)
        var netOfPin: [String: Int] = [:]
        for connection in netlist.connections { netOfPin["\(connection.part)/\(connection.pin)"] = connection.net }

        let groups = WireGroups(wires: wires, labels: labels)
        let labelsByID = Dictionary(labels.map { ($0.id, $0) }, uniquingKeysWith: { first, _ in first })
        var pins: [ErcPin] = []
        var slots: [Slot] = []
        for (ci, chip) in chips.enumerated() {
            let part: UnsafePointer<Part>? = library.flatMap { part_library_find($0.pointer, chip.partNumber) }
            for pin in chip.part.pins where !pin.isHidden {
                var type = Self.fallbackType(pin.role)
                var drives: Int32 = 0
                if let part {
                    var found = ERC_INPUT
                    if erc_pin_type(part, pin.index, &found) != 0 { type = found }
                    drives = erc_pin_drives_power(part, pin.index)
                }
                let ref = PinRef(chipID: chip.id, index: pin.index)
                pins.append(ErcPin(net: netOfPin["\(ci)/\(pin.designator)"] ?? -1, type: type, drives_power: drives,
                                   no_connect_mark: groups.hasNoConnect(.pin(ref)) ? 1 : 0))
                slots.append(.pin(chip: ci, pin: pin))
            }
        }
        // A Power Flag is a power_out pin with no part.
        for label in labels where label.isPowerFlag {
            let wire = wires.first { $0.start == .label(label.id) || $0.end == .label(label.id) }
            pins.append(ErcPin(net: wire.flatMap { netlist.netOfWire[$0.id] } ?? -1, type: ERC_POWER_OUT,
                               drives_power: 0, no_connect_mark: 0))
            slots.append(.flag(label.id))
        }
        // A label or a bus port on a net connects it.
        var nets = [ErcNet](repeating: ErcNet(has_attachment: 0), count: max(netlist.netCount, 1))
        for wire in wires {
            guard let net = netlist.netOfWire[wire.id] else { continue }
            for end in [wire.start, wire.end] {
                switch end {
                case let .label(id) where labelsByID[id]?.isNet == true:
                    nets[net].has_attachment = 1
                case .busPin:
                    nets[net].has_attachment = 1
                default:
                    break
                }
            }
        }

        let map = settings.effectivePinMap.flatMap { $0 }
        let problems = ErcChecker.run(pins: pins, nets: nets, map: map)
        func reference(_ slot: Slot) -> String {
            switch slot {
            case let .pin(chip, pin): "\(chips[chip].reference).\(pin.designator)"
            case .flag: "Power Flag"
            }
        }
        func identity(_ slot: Slot) -> String {
            switch slot {
            case let .pin(chip, pin): "chip:\(chips[chip].id.uuidString):\(pin.designator)"
            case let .flag(id): "label:\(id.uuidString)"
            }
        }
        func item(_ slot: Slot) -> SchematicItem {
            switch slot {
            case let .pin(chip, _): .chip(chips[chip].id)
            case let .flag(id): .label(id)
            }
        }
        func point(_ slot: Slot) -> CGPoint {
            switch slot {
            case let .pin(chip, pin):
                document.pinTip(PinRef(chipID: chips[chip].id, index: pin.index)) ?? chips[chip].origin
            case let .flag(id): labelsByID[id]?.tip ?? .zero
            }
        }
        for problem in problems {
            let slot = slots[problem.pin]
            switch problem.kind {
            case ERC_PIN_TO_PIN:
                let other = slots[problem.other]
                let a = Self.typeIndex(pins[problem.other].type), b = Self.typeIndex(pins[problem.pin].type)
                add(.pinToPin, severity: problem.level >= 2 ? .error : .warning,
                    message: "Pins of type \(ErcPinTypes.titles[a]) and \(ErcPinTypes.titles[b]) are connected",
                    reference: "\(reference(other)), \(reference(slot))", items: [item(other), item(slot)],
                    point: point(slot), ids: [identity(other), identity(slot)])
            case ERC_PIN_NOT_CONNECTED:
                add(.pinNotConnected, message: "Pin not connected", reference: reference(slot), items: [item(slot)],
                    point: point(slot), ids: [identity(slot)])
            case ERC_PIN_NOT_DRIVEN:
                add(.pinNotDriven, message: "Input pin not driven by any Output pins", reference: reference(slot),
                    items: [item(slot)], point: point(slot), ids: [identity(slot)])
            case ERC_POWER_PIN_NOT_DRIVEN:
                add(.powerPinNotDriven, message: "Input Power pin not driven by any Output Power pins",
                    reference: reference(slot), items: [item(slot)], point: point(slot), ids: [identity(slot)])
            default:
                let isType = pins[problem.pin].type == ERC_NO_CONNECT
                add(.noConnectConnected,
                    message: isType ? "Pin with 'no connection' type is connected"
                        : "A pin with a \"no connection\" flag is connected",
                    reference: reference(slot), items: [item(slot)], point: point(slot), ids: [identity(slot)])
            }
        }
    }

    /// Runs the backend's check; `map` is the 12×12 pin map, row by row.
    private static func run(pins: [ErcPin], nets: [ErcNet], map: [UInt8]) -> [ErcProblem] {
        typealias Row = (UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8)
        return map.withUnsafeBufferPointer { mapBuffer in
            mapBuffer.baseAddress!.withMemoryRebound(to: Row.self, capacity: 12) { rows in
                pins.withUnsafeBufferPointer { pinList in
                    nets.withUnsafeBufferPointer { netList in
                        let count = erc_check(pinList.baseAddress, pinList.count, netList.count, netList.baseAddress,
                                              rows, nil, 0)
                        guard count > 0 else { return [] }
                        var out = [ErcProblem](repeating: ErcProblem(), count: count)
                        let filled = out.withUnsafeMutableBufferPointer { buffer in
                            erc_check(pinList.baseAddress, pinList.count, netList.count, netList.baseAddress, rows,
                                      buffer.baseAddress, count)
                        }
                        return Array(out.prefix(min(filled, count)))
                    }
                }
            }
        }
    }
}

/// The wire-connected groups of a drawing: the wire ends joined by wires only,
/// with no merging of labels by name (that is the netlist's net, not a group).
/// A No Connect marker and a Power Flag are ends like any other.
nonisolated struct WireGroups {
    private var parent: [WireEnd: WireEnd] = [:]
    /// The groups made only of the wires that end on a No Connect marker (a pin
    /// wired to a flag is flagged; a pin wired to that pin is not).
    private var flagParent: [WireEnd: WireEnd] = [:]
    /// The roots of `flagParent` that hold a No Connect marker.
    private var flaggedRoots: Set<WireEnd> = []

    init(wires: [Wire], labels: [NetLabel]) {
        let noConnects = Set(labels.filter(\.isNoConnect).map(\.id))
        func isFlag(_ end: WireEnd) -> Bool {
            if case let .label(id) = end { return noConnects.contains(id) }
            return false
        }
        for wire in wires {
            Self.union(&parent, wire.start, wire.end)
            if isFlag(wire.start) || isFlag(wire.end) { Self.union(&flagParent, wire.start, wire.end) }
        }
        for wire in wires {
            for end in [wire.start, wire.end] where isFlag(end) { flaggedRoots.insert(Self.find(flagParent, end)) }
        }
    }

    private static func find(_ parent: [WireEnd: WireEnd], _ end: WireEnd) -> WireEnd {
        var current = end
        while let next = parent[current], next != current { current = next }
        return current
    }

    private static func union(_ parent: inout [WireEnd: WireEnd], _ a: WireEnd, _ b: WireEnd) {
        let ra = find(parent, a), rb = find(parent, b)
        if ra != rb { parent[ra] = rb }
    }

    /// The root of the group `end` is in.
    func find(_ end: WireEnd) -> WireEnd { Self.find(parent, end) }

    /// Whether `end` is on a wire that ends on a No Connect marker (through wires
    /// that end on markers only).
    func hasNoConnect(_ end: WireEnd) -> Bool { flaggedRoots.contains(Self.find(flagParent, end)) }
}
