//
//  VoltageCheck.swift
//  schematical
//
//  The backend's voltage check (voltage.h) on the drawing: which pins would
//  get a voltage outside their part's range. The editor runs it on a
//  candidate drawing to refuse a wire or a switch that adds a problem.
//

import Foundation

/// One voltage problem in a drawing, by chip.
struct VoltageIssue: Hashable {
    let kind: VoltageProblemKind.RawValue
    let chipID: UUID
    /// The pin's index in its part (`ChipPin.index`).
    let pin: Int
    /// For a supply clash, the other supply; otherwise `chipID`.
    let otherChipID: UUID
    /// The net in the drawing it was found in. Net numbers differ between two
    /// drawings, so two issues are the same problem without comparing it.
    let net: Int
    let mv: UInt32
    let minMV: UInt32
    let maxMV: UInt32

    static func == (a: VoltageIssue, b: VoltageIssue) -> Bool {
        a.kind == b.kind && a.chipID == b.chipID && a.pin == b.pin && a.otherChipID == b.otherChipID
            && a.mv == b.mv && a.minMV == b.minMV && a.maxMV == b.maxMV
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(kind)
        hasher.combine(chipID)
        hasher.combine(pin)
        hasher.combine(otherChipID)
        hasher.combine(mv)
    }
}

enum VoltageCheck {
    /// What the refused edit was, for the message's verb.
    enum Edit {
        case connect
        case turnOn

        var verb: String {
            switch self {
            case .connect: "Can't connect"
            case .turnOn: "Can't turn on"
            }
        }
    }

    /// Problems in this drawing. Closed contacts are the switch positions
    /// that are ON (`PlacedChip.switchesOn`).
    static func issues(chips: [PlacedChip], wires: [Wire], library: PartLibraryHandle) -> Set<VoltageIssue> {
        Set(sortedIssues(chips: chips, wires: wires, library: library))
    }

    /// Problems in this drawing, ordered by net, then part, then pin.
    static func sortedIssues(chips: [PlacedChip], wires: [Wire], library: PartLibraryHandle) -> [VoltageIssue] {
        var parts: [UnsafePointer<Part>?] = []
        for chip in chips {
            guard let part = part_library_find(library.pointer, chip.partNumber) else { return [] }
            parts.append(part)
        }
        let netlist = NetlistBuilder.build(chips: chips, wires: wires)
        let connections: [VoltageConnection] = netlist.connections.compactMap { connection in
            guard let pin = chips[connection.part].part.pins.first(where: { $0.designator == connection.pin })
            else { return nil }
            return VoltageConnection(part: connection.part, pin: pin.index, net: connection.net)
        }
        let closed = netlist.switchesOn.map { VoltageContact(part: $0.part, position: UInt32($0.position)) }

        let problems: [VoltageProblem] = parts.withUnsafeBufferPointer { partList in
            connections.withUnsafeBufferPointer { connectionList in
                closed.withUnsafeBufferPointer { closedList in
                    let count = voltage_check(partList.baseAddress, partList.count, netlist.netCount,
                                              connectionList.baseAddress, connectionList.count,
                                              closedList.baseAddress, closedList.count, nil, 0)
                    guard count > 0 else { return [] }
                    var out = [VoltageProblem](repeating: VoltageProblem(), count: count)
                    let filled = out.withUnsafeMutableBufferPointer { buffer in
                        voltage_check(partList.baseAddress, partList.count, netlist.netCount,
                                      connectionList.baseAddress, connectionList.count,
                                      closedList.baseAddress, closedList.count, buffer.baseAddress, count)
                    }
                    return Array(out.prefix(min(filled, count)))
                }
            }
        }
        return problems.map { problem in
            VoltageIssue(
                kind: problem.kind.rawValue,
                chipID: chips[problem.part].id,
                pin: problem.pin,
                otherChipID: chips[problem.other_part].id,
                net: problem.net,
                mv: problem.mv,
                minMV: problem.min_mv,
                maxMV: problem.max_mv
            )
        }
    }

    /// The first problem in `after` that is not in `before`, in net, part,
    /// pin order; nil when the edit adds none.
    static func firstNew(before: [PlacedChip], _ beforeWires: [Wire],
                         after: [PlacedChip], _ afterWires: [Wire],
                         library: PartLibraryHandle) -> VoltageIssue? {
        let old = issues(chips: before, wires: beforeWires, library: library)
        return sortedIssues(chips: after, wires: afterWires, library: library).first { !old.contains($0) }
    }

    /// Why the edit is refused, for the user, e.g.
    /// "Can't connect: 5 V on U3 pin 14 (VCC). SN74AUC00RGYR needs 0.8–2.7 V."
    static func message(_ issue: VoltageIssue, chips: [PlacedChip], edit: Edit = .connect) -> String {
        guard let chip = chips.first(where: { $0.id == issue.chipID }) else { return "\(edit.verb)." }
        let pin = chip.part.pins.indices.contains(issue.pin) ? chip.part.pins[issue.pin] : nil
        let pinText = pin.map { "pin \($0.designator) (\($0.name))" } ?? "a pin"
        let part = chip.part.name

        switch VoltageProblemKind(issue.kind) {
        case VOLTAGE_SUPPLY_CLASH:
            let other = chips.first { $0.id == issue.otherChipID }
            // The lower voltage first.
            var low = (volts(issue.minMV), chip.reference)
            var high = (volts(issue.mv), other?.reference ?? "?")
            if case let .supply(_, mv) = chip.part.details, mv == issue.mv, issue.mv != issue.minMV {
                low.1 = other?.reference ?? "?"
                high.1 = chip.reference
            }
            return "\(edit.verb): \(low.0) V (\(low.1)) and \(high.0) V (\(high.1)) supplies on one net."
        case VOLTAGE_POWER_RANGE:
            return "\(edit.verb): \(volts(issue.mv)) V on \(chip.reference) \(pinText). "
                + "\(part) needs \(volts(issue.minMV))–\(volts(issue.maxMV)) V."
        case VOLTAGE_INPUT_OVER:
            return "\(edit.verb): \(chip.reference) \(pinText) gets \(volts(issue.mv)) V; "
                + "\(part) inputs take at most \(volts(issue.maxMV)) V."
        default:
            return "\(edit.verb): \(chip.reference) \(pinText) gets \(volts(issue.mv)) V; "
                + "\(part) is rated \(volts(issue.maxMV)) V."
        }
    }

    /// Volts with up to two decimals and no trailing zeros: 5, 3.3, 2.97.
    static func volts(_ mv: UInt32) -> String {
        var text = String(format: "%.2f", Double(mv) / 1000)
        while text.hasSuffix("0") { text.removeLast() }
        if text.hasSuffix(".") { text.removeLast() }
        return text
    }
}
