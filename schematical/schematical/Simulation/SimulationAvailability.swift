import Foundation

/// Whether a schematic's simulation may start (plan-36-H). A part the
/// backend has no model for (`CatalogPart.isSimulated == false`: the
/// microcontrollers) keeps Simulation › Run off while it is placed; the
/// circuit would refuse it anyway. Stop always works.
@MainActor
enum SimulationAvailability {
    /// The placed parts that are not simulated, in reference order
    /// (U2 before U10).
    static func unsimulated(_ chips: [PlacedChip]) -> [PlacedChip] {
        chips.filter { !$0.part.isSimulated }
            .sorted { $0.reference.localizedStandardCompare($1.reference) == .orderedAscending }
    }

    /// The line under Run while it is off: "U1 (ATMEGA328P-PU) is not
    /// simulated", or "U1 (ATMEGA328P-PU) and 2 more are not simulated".
    /// Nil when every placed part is simulated.
    static func reason(_ chips: [PlacedChip]) -> String? {
        let blocked = unsimulated(chips)
        guard let first = blocked.first else { return nil }
        let name = "\(first.reference) (\(first.part.name))"
        return blocked.count == 1 ? "\(name) is not simulated"
            : "\(name) and \(blocked.count - 1) more are not simulated"
    }

    /// The Simulation menu's view of a window: Run is available on the
    /// Schematic page when every placed part is simulated; the reason is
    /// given on the Schematic page only.
    static func target(page: EditorPage, chips: [PlacedChip], isRunning: Bool,
                       toggle: @escaping () -> Void) -> SimulationTarget {
        let blocked = page == .schematic ? Self.reason(chips) : nil
        return SimulationTarget(isAvailable: page == .schematic && blocked == nil, blockedReason: blocked,
                                isRunning: isRunning, toggle: toggle)
    }

    /// The document's simulation may start: no placed part is unsimulated.
    static func canStart(_ document: SchematicDocument) -> Bool {
        !document.chips.contains { !$0.part.isSimulated }
    }

    /// Run / Stop, as the menu and ⌘P do it: Stop always; Run only when
    /// `canStart`.
    static func toggle(_ simulation: SimulationController, document: SchematicDocument) {
        if !simulation.isRunning && !canStart(document) { return }
        simulation.toggle(document: document)
    }
}
