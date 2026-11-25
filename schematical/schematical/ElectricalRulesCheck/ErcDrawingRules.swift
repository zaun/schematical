//
//  ErcDrawingRules.swift
//  schematical
//
//  The ERC rules that read the drawing, not pin types (plan-63-E): wires and
//  labels that connect to nothing, No Connect flags on no pin, labels that
//  differ only in case, two names on one wire, four wires at a junction and
//  two parts with one reference. Each follows what KiCad 10.0.8 reports
//  (planning/plan-63-data/README.md).
//

import CoreGraphics
import Foundation

/// What a wire-connected group holds.
private nonisolated struct GroupContents {
    var pins: [PinRef] = []
    var named: [NetLabel] = []
    var busPins = 0
}

extension ErcChecker {
    /// A part's rail name when it is a supply symbol (a pull-up or pull-down is not).
    private func supplyName(_ chip: PlacedChip) -> String? {
        if case .supply(_, _, false) = chip.part.details { return chip.partNumber }
        return nil
    }

    mutating func checkDrawing(netlist: SchematicNetlist, groups: WireGroups) {
        let labelsByID = Dictionary(labels.map { ($0.id, $0) }, uniquingKeysWith: { first, _ in first })
        let chipsByID = Dictionary(chips.map { ($0.id, $0) }, uniquingKeysWith: { first, _ in first })

        // What each group holds, and how many wire ends are on each junction.
        var contents: [WireEnd: GroupContents] = [:]
        var junctionEnds: [UUID: Int] = [:]
        var seenEnds = Set<WireEnd>()
        for wire in wires {
            for end in [wire.start, wire.end] {
                if case let .junction(id) = end { junctionEnds[id, default: 0] += 1 }
                guard seenEnds.insert(end).inserted else { continue }
                let root = groups.find(end)
                switch end {
                case let .pin(ref): contents[root, default: GroupContents()].pins.append(ref)
                case let .label(id):
                    if let label = labelsByID[id], label.isNet { contents[root, default: GroupContents()].named.append(label) }
                case .busPin: contents[root, default: GroupContents()].busPins += 1
                case .junction: break
                }
            }
        }
        func group(of label: NetLabel) -> GroupContents { contents[groups.find(.label(label.id))] ?? GroupContents() }

        checkNoConnectMarkers(groups: groups, contents: contents)
        checkWireEnds(junctionEnds: junctionEnds)
        checkLabels(netlist: netlist, groups: groups, contents: contents, group: group(of:))
        checkNames(groups: groups, contents: contents, chipsByID: chipsByID)
        checkFourWayJunctions(junctionEnds: junctionEnds)
        checkReferences()
    }

    // MARK: No Connect markers

    /// `no_connect_dangling`: a marker whose group has no pin.
    private mutating func checkNoConnectMarkers(groups: WireGroups, contents: [WireEnd: GroupContents]) {
        for label in labels where label.isNoConnect {
            if (contents[groups.find(.label(label.id))]?.pins.isEmpty ?? true) {
                add(.noConnectDangling, message: "Unconnected \"no connection\" flag", reference: "No Connect",
                    items: [.label(label.id)], point: label.tip, ids: ["label:\(label.id.uuidString)"])
            }
        }
    }

    // MARK: Wires

    /// `unconnected_wire_endpoint` for each wire end on a junction no other wire end is on, and
    /// `wire_dangling` for a wire with neither end connected.
    private mutating func checkWireEnds(junctionEnds: [UUID: Int]) {
        func isFree(_ end: WireEnd) -> Bool {
            if case let .junction(id) = end { return junctionEnds[id] == 1 }
            return false
        }
        for wire in wires {
            let startFree = isFree(wire.start), endFree = isFree(wire.end)
            for (end, free) in [(wire.start, startFree), (wire.end, endFree)] where free {
                guard let point = document.endTip(end), case let .junction(id) = end else { continue }
                add(.unconnectedWireEndpoint, message: "Unconnected wire endpoint", reference: "Wire",
                    items: [.wire(wire.id)], point: point,
                    ids: ["wire:\(wire.id.uuidString)", "junction:\(id.uuidString)"])
            }
            if startFree && endFree, let point = document.endTip(wire.start) {
                add(.wireDangling, message: "Wires not connected to anything", reference: "Wire",
                    items: [.wire(wire.id)], point: point, ids: ["wire:\(wire.id.uuidString)"])
            }
        }
    }

    // MARK: Labels

    /// `label_dangling`, `isolated_pin_label`, `single_global_label`.
    private mutating func checkLabels(netlist: SchematicNetlist, groups: WireGroups,
                                      contents: [WireEnd: GroupContents],
                                      group: (NetLabel) -> GroupContents) {
        // The pins on each net (labels of one name merged).
        var pinsOnNet: [Int: Set<String>] = [:]
        for connection in netlist.connections { pinsOnNet[connection.net, default: []].insert("\(connection.part)/\(connection.pin)") }
        var netOfLabel: [UUID: Int] = [:]
        for wire in wires {
            guard let net = netlist.netOfWire[wire.id] else { continue }
            for end in [wire.start, wire.end] { if case let .label(id) = end { netOfLabel[id] = net } }
        }
        var uses: [String: Int] = [:]
        for label in labels where label.isNet { uses[label.name, default: 0] += 1 }

        for label in labels where label.isNet {
            let held = group(label)
            let id = "label:\(label.id.uuidString)"
            if held.pins.isEmpty && held.busPins == 0 {
                add(.labelDangling, message: "Label not connected", reference: label.name, items: [.label(label.id)],
                    point: label.tip, ids: [id])
            } else if let net = netOfLabel[label.id], pinsOnNet[net]?.count == 1 {
                add(.isolatedPinLabel, message: "Label connected to only one pin", reference: label.name,
                    items: [.label(label.id)], point: label.tip, ids: [id])
            }
            if uses[label.name] == 1 {
                add(.singleGlobalLabel, message: "Global label only appears once in the schematic",
                    reference: label.name, items: [.label(label.id)], point: label.tip, ids: [id])
            }
        }
    }

    // MARK: Names

    /// `multiple_net_names`, `similar_labels`, `similar_label_and_power`.
    private mutating func checkNames(groups: WireGroups, contents: [WireEnd: GroupContents],
                                     chipsByID: [UUID: PlacedChip]) {
        // Two labels, or two power symbols, of different names on one group.
        var rails: [WireEnd: [(name: String, chip: PlacedChip, ref: PinRef)]] = [:]
        var seen = Set<PinRef>()
        for wire in wires {
            for end in [wire.start, wire.end] {
                guard case let .pin(ref) = end, seen.insert(ref).inserted, let chip = chipsByID[ref.chipID],
                      let name = supplyName(chip) else { continue }
                rails[groups.find(end), default: []].append((name, chip, ref))
            }
        }
        for held in contents.values {
            let names = Set(held.named.map(\.name)).sorted()
            if names.count >= 2 {
                let first = held.named.first { $0.name == names[0] }!, second = held.named.first { $0.name == names[1] }!
                add(.multipleNetNames,
                    message: "Both \(names[0]) and \(names[1]) are attached to the same items; \(names[0]) will be used in the netlist",
                    reference: "\(names[0]), \(names[1])", items: [.label(first.id), .label(second.id)], point: first.tip,
                    ids: held.named.map { "label:\($0.id.uuidString)" })
            }
        }
        for held in rails.values {
            let names = Set(held.map(\.name)).sorted()
            if names.count >= 2 {
                let first = held.first { $0.name == names[0] }!, second = held.first { $0.name == names[1] }!
                add(.multipleNetNames,
                    message: "Both \(names[0]) and \(names[1]) are attached to the same items; \(names[0]) will be used in the netlist",
                    reference: "\(names[0]), \(names[1])", items: [.chip(first.chip.id), .chip(second.chip.id)],
                    point: document.pinTip(first.ref) ?? first.chip.origin,
                    ids: held.map { "part:\($0.chip.id.uuidString)" })
            }
        }

        // Names that differ only in case.
        let named = labels.filter(\.isNet)
        let byLowercase = Dictionary(grouping: named, by: { $0.name.lowercased() })
        for (_, same) in byLowercase {
            let spellings = Set(same.map(\.name)).sorted()
            guard spellings.count >= 2 else { continue }
            let first = same.first { $0.name == spellings[0] }!
            for spelling in spellings.dropFirst() {
                let other = same.first { $0.name == spelling }!
                add(.similarLabels, message: "Labels are similar (lower/upper case difference only)",
                    reference: "\(spellings[0]), \(spelling)", items: [.label(first.id), .label(other.id)],
                    point: other.tip, ids: ["label:\(first.id.uuidString)", "label:\(other.id.uuidString)"])
            }
        }
        // A label and a power symbol.
        let railNames = Set(chips.compactMap(supplyName))
        for label in named {
            for rail in railNames where rail != label.name && rail.lowercased() == label.name.lowercased() {
                add(.similarLabelAndPower, message: "Power pin and label are similar (lower/upper case difference only)",
                    reference: "\(label.name), \(rail)", items: [.label(label.id)], point: label.tip,
                    ids: ["label:\(label.id.uuidString)", "rail:\(rail)"])
            }
        }
    }

    // MARK: Junctions and references

    /// `four_way_junction`: a junction with four or more wire ends.
    private mutating func checkFourWayJunctions(junctionEnds: [UUID: Int]) {
        for junction in document.junctions where (junctionEnds[junction.id] ?? 0) >= 4 {
            add(.fourWayJunction,
                message: "Four items connected at \(Int(junction.point.x)), \(Int(junction.point.y))",
                reference: "Junction", items: [.junction(junction.id)], point: junction.point,
                ids: ["junction:\(junction.id.uuidString)"])
        }
    }

    /// `duplicate_reference`: parts that share a reference (a hand-edited file can; Rename refuses it).
    private mutating func checkReferences() {
        for (reference, same) in Dictionary(grouping: chips, by: \.reference) where same.count >= 2 {
            add(.duplicateReference, message: "Duplicate reference \(reference)", reference: reference,
                items: Set(same.map { .chip($0.id) }), point: same[0].origin,
                ids: same.map { "part:\($0.id.uuidString)" })
        }
    }
}
