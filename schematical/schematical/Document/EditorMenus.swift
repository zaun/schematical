import SwiftUI

/// What one document's editor gives the app's menus (plan-61-C).
///
/// The menus (`EditCommands`, `SimulationCommands`, `RouteCommands`, the
/// Window and Help items) get the key window's `SchematicEditorModel` as a
/// focused value. It is one object for the life of the window. What changes
/// under it (the page, the selection, a running simulation or autoroute) the
/// menus read through the closures here, which the editor view installs when
/// it appears. A menu's views call them in `body`, so SwiftUI's observation
/// renders an item again when what the closure read has changed.
///
/// Before the editor was hosted in its document's window it published a new
/// value (`EditActions`, `RouteActions`, …) as a focused scene value on every
/// change. From a hosted view SwiftUI delivers a focused value to the menus
/// when the window becomes key and not again: on the PCB page Route and
/// Design Rule Check stayed disabled, and Simulation › Run stayed enabled
/// (measured in the built app through accessibility, with the menu open).
@MainActor
@Observable
final class EditorMenus {
    /// Edit › Cut … Reset References.
    var edit: (() -> EditActions)?
    /// Simulation › Run / Stop.
    var simulation: (() -> SimulationTarget)?
    /// The Route menu; nil off the PCB page and while playing.
    var route: (() -> RouteActions?)?
    /// Window › Bill of Materials.
    var openBillOfMaterials: (() -> Void)?
    /// Window › Design Rule Check (the PCB page's).
    var openDesignRuleCheck: (() -> Void)?

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

    /// Lets the closures go: they hold the editor view, and through it the
    /// model that owns this object. The document calls this when it closes.
    func removeAll() {
        edit = nil
        simulation = nil
        route = nil
        openBillOfMaterials = nil
        openDesignRuleCheck = nil
    }
}

extension FocusedValues {
    /// The key schematic window's editor, for the menus.
    @Entry var schematicEditor: SchematicEditorModel?
}
