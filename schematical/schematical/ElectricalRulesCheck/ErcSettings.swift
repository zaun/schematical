//
//  ErcSettings.swift
//  schematical
//
//  The electrical rules check's settings (plan-63-E): which of KiCad's rules
//  there are, what severity each has, the pin-conflict map and the violations
//  the user excluded. They are part of the document (undoable, saved in the
//  file); only what differs from KiCad's defaults is kept.
//

import Foundation

/// How serious a violation of a rule is. `ignore` hides the rule.
nonisolated enum ErcSeverity: String, CaseIterable, Sendable {
    case error
    case warning
    case ignore

    var title: String {
        switch self {
        case .error: "Error"
        case .warning: "Warning"
        case .ignore: "Ignore"
        }
    }
}

/// The rules of KiCad's ERC that apply to this app (plan-63's README), by the
/// name KiCad gives them.
nonisolated enum ErcRule: String, CaseIterable, Sendable {
    case pinToPin = "pin_to_pin"
    case pinNotConnected = "pin_not_connected"
    case pinNotDriven = "pin_not_driven"
    case powerPinNotDriven = "power_pin_not_driven"
    case noConnectConnected = "no_connect_connected"
    case noConnectDangling = "no_connect_dangling"
    case wireDangling = "wire_dangling"
    case unconnectedWireEndpoint = "unconnected_wire_endpoint"
    case labelDangling = "label_dangling"
    case isolatedPinLabel = "isolated_pin_label"
    case multipleNetNames = "multiple_net_names"
    case similarLabels = "similar_labels"
    case similarLabelAndPower = "similar_label_and_power"
    case duplicateReference = "duplicate_reference"
    case singleGlobalLabel = "single_global_label"
    case fourWayJunction = "four_way_junction"

    /// The words the rules list shows.
    var title: String {
        switch self {
        case .pinToPin: "Pin conflicts on a net"
        case .pinNotConnected: "Pin not connected"
        case .pinNotDriven: "Input pin not driven"
        case .powerPinNotDriven: "Power input pin not driven"
        case .noConnectConnected: "No Connect on a connected pin"
        case .noConnectDangling: "No Connect on no pin"
        case .wireDangling: "Wire not connected to anything"
        case .unconnectedWireEndpoint: "Unconnected wire end"
        case .labelDangling: "Label not connected"
        case .isolatedPinLabel: "Label on a net with one pin"
        case .multipleNetNames: "Two names on one wire"
        case .similarLabels: "Labels differ only in case"
        case .similarLabelAndPower: "Label and power symbol differ only in case"
        case .duplicateReference: "Duplicate reference"
        case .singleGlobalLabel: "Label name used once"
        case .fourWayJunction: "Four wires at a junction"
        }
    }

    /// KiCad's default severity (its stock `.kicad_pro`; `ignore` for the rules
    /// it leaves off). `pin_to_pin` has none of its own: the pin map's cell is
    /// its severity.
    var defaultSeverity: ErcSeverity {
        switch self {
        case .pinNotConnected, .pinNotDriven, .powerPinNotDriven, .wireDangling, .labelDangling,
             .duplicateReference:
            .error
        case .noConnectConnected, .noConnectDangling, .unconnectedWireEndpoint, .isolatedPinLabel,
             .multipleNetNames, .similarLabels, .similarLabelAndPower:
            .warning
        case .singleGlobalLabel, .fourWayJunction:
            .ignore
        case .pinToPin:
            .error
        }
    }

    /// Whether the rules list offers a severity for it.
    var hasSeverity: Bool { self != .pinToPin }
}

/// The twelve KiCad pin types, in the order of the pin map (`ErcPinType`).
nonisolated enum ErcPinTypes {
    static let names = ["input", "output", "bidirectional", "tri_state", "passive", "free",
                        "unspecified", "power_in", "power_out", "open_collector", "open_emitter",
                        "no_connect"]
    static let count = names.count
    /// The words the pin map's headers use.
    static let titles = ["Input", "Output", "Bidirectional", "Tri-state", "Passive", "Free", "Unspecified",
                         "Power input", "Power output", "Open collector", "Open emitter", "Unconnected"]
    /// The row and column of the no_connect type: `pin_to_pin` never reads them.
    static let noConnect = 11
}

/// A document's ERC settings: what differs from KiCad's defaults.
nonisolated struct ErcSettings: Hashable, Sendable {
    /// The rules whose severity is not the default.
    var severities: [ErcRule: ErcSeverity] = [:]
    /// The 12×12 pin map when it is not the default; nil is the default.
    var pinMap: [[UInt8]]?
    /// The excluded violations' keys (`ErcViolation.key`).
    var exclusions: Set<String> = []

    /// KiCad's default pin map, from the backend (`ERC_DEFAULT_PIN_MAP`).
    static let defaultPinMap: [[UInt8]] = {
        let flat = withUnsafeBytes(of: ERC_DEFAULT_PIN_MAP) { Array($0) }
        return (0..<ErcPinTypes.count).map { Array(flat[($0 * ErcPinTypes.count)..<(($0 + 1) * ErcPinTypes.count)]) }
    }()

    /// The pin map the check uses.
    var effectivePinMap: [[UInt8]] { pinMap ?? Self.defaultPinMap }

    var isDefault: Bool { severities.isEmpty && pinMap == nil && exclusions.isEmpty }

    func severity(of rule: ErcRule) -> ErcSeverity { severities[rule] ?? rule.defaultSeverity }

    /// Whether `map` is a valid pin map: 12 rows of 12 values 0, 1 or 2, symmetric.
    static func isValid(pinMap map: [[UInt8]]) -> Bool {
        let n = ErcPinTypes.count
        guard map.count == n, map.allSatisfy({ $0.count == n }) else { return false }
        for a in 0..<n {
            for b in 0..<n where map[a][b] > 2 || map[a][b] != map[b][a] { return false }
        }
        return true
    }

    /// Sets a rule's severity; the default removes the entry.
    mutating func set(_ severity: ErcSeverity, for rule: ErcRule) {
        if severity == rule.defaultSeverity {
            severities[rule] = nil
        } else {
            severities[rule] = severity
        }
    }

    /// Sets one cell of the pin map and its mirror (0 ok, 1 warning, 2 error).
    mutating func setPinMap(_ a: Int, _ b: Int, to level: UInt8) {
        var map = effectivePinMap
        guard map.indices.contains(a), map.indices.contains(b), level <= 2 else { return }
        map[a][b] = level
        map[b][a] = level
        pinMap = map == Self.defaultPinMap ? nil : map
    }

    /// Sets the whole pin map; the default clears it.
    mutating func setPinMap(_ map: [[UInt8]]) {
        guard Self.isValid(pinMap: map) else { return }
        pinMap = map == Self.defaultPinMap ? nil : map
    }
}
