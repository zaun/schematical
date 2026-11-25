import SwiftUI

/// What the Route menu does in the key window: set only on the PCB page,
/// and only while the simulation isn't playing (plan-38-K).
struct RouteActions {
    var autoroute: () -> Void
    var routeSelected: () -> Void
    var unrouteSelected: () -> Void
    var unrouteAllAutorouted: () -> Void
    var selectAutorouted: () -> Void
    /// A run is under way: the menu waits.
    var isRouting: Bool
    /// Something is selected on the PCB.
    var hasSelection: Bool

    /// Whether a window offers the Route menu: on the PCB page, while the
    /// simulation isn't playing.
    static func available(page: EditorPage, isPlaying: Bool) -> Bool {
        page == .pcb && !isPlaying
    }
}


/// Route ▸ Autoroute… (⌥⌘R), Route Selected Nets (⌥⇧⌘R), Unroute Selected
/// (⌥⌘U), Unroute All Autorouted, Select Autorouted Copper.
struct RouteCommands: Commands {
    @FocusedValue(\.schematicEditor) private var editor

    var body: some Commands {
        CommandMenu("Route") {
            RouteMenuItems(editor: editor)
        }
    }
}

/// A view, so the items are rendered again when what they read (the page, a
/// run, the selection) changes (`EditorMenus`).
private struct RouteMenuItems: View {
    let editor: SchematicEditorModel?

    var body: some View {
        let actions = editor?.menus.route?()
        Group {
            Button("Autoroute…") { actions?.autoroute() }
                .keyboardShortcut("r", modifiers: [.command, .option])
                .disabled(actions == nil || actions?.isRouting == true)
            Button("Route Selected Nets") { actions?.routeSelected() }
                .keyboardShortcut("r", modifiers: [.command, .option, .shift])
                .disabled(actions == nil || actions?.isRouting == true || actions?.hasSelection != true)
            Divider()
            Button("Unroute Selected") { actions?.unrouteSelected() }
                .keyboardShortcut("u", modifiers: [.command, .option])
                .disabled(actions == nil || actions?.isRouting == true || actions?.hasSelection != true)
            Button("Unroute All Autorouted") { actions?.unrouteAllAutorouted() }
                .disabled(actions == nil || actions?.isRouting == true)
            Divider()
            Button("Select Autorouted Copper") { actions?.selectAutorouted() }
                .disabled(actions == nil)
        }
    }
}

extension SchematicDocument {
    /// The nets the PCB selection touches: its footprints' pads' nets and
    /// its traces' and vias' nets.
    func selectedNets() -> Set<Int> {
        let copper = connectivity()
        var nets = Set<Int>()
        for item in boardSelection.items {
            switch item {
            case let .footprint(id):
                guard let index = chips.firstIndex(where: { $0.id == id }) else { continue }
                for p in copper.pads.indices where copper.pads[p].chip == index {
                    if let net = copper.netOfPad[p] { nets.insert(net) }
                }
            case let .trace(id): if let net = net(ofTrace: id, in: copper) { nets.insert(net) }
            case let .via(id): if let net = net(ofVia: id, in: copper) { nets.insert(net) }
            default: break
            }
        }
        return nets
    }

    /// Selects every autorouted trace and via.
    func selectAutorouted() {
        selectBoardItems(Set(traces.filter(\.auto).map { BoardItem.trace($0.id) } + vias.filter(\.auto).map { .via($0.id) }))
    }
}
