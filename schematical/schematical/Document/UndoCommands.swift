import AppKit
import SwiftUI

// Edit › Undo and Redo (plan-31-D). In a schematic window they act on the
// page it shows; while a text field is being edited they are that field's
// own typing undo, as in any Mac app.

/// The focused schematic window's document and the page it shows.
struct UndoTarget {
    let document: SchematicDocument
    let page: EditorPage
}

extension FocusedValues {
    /// Set by each schematic window while it is key.
    @Entry var schematicUndo: UndoTarget?
}

/// What ⌘Z / ⇧⌘Z do.
enum UndoRoute: Equatable {
    /// To the first responder: text editing, or a window with no schematic.
    case responderChain
    /// The page's own list.
    case page(EditorPage)
}

enum UndoRouting {
    static func route(textIsEditing: Bool, target: UndoTarget?) -> UndoRoute {
        guard !textIsEditing, let target else { return .responderChain }
        return .page(target.page)
    }

    /// "Undo Move", or "Undo" with nothing to undo or while playing.
    static func undoTitle(_ target: UndoTarget?) -> String {
        title("Undo", target.flatMap { $0.document.isPlaying ? nil : $0.document.undoName(on: $0.page) })
    }

    /// "Redo Move", or "Redo" with nothing to redo or while playing.
    static func redoTitle(_ target: UndoTarget?) -> String {
        title("Redo", target.flatMap { $0.document.isPlaying ? nil : $0.document.redoName(on: $0.page) })
    }

    private static func title(_ verb: String, _ name: String?) -> String {
        name.map { "\(verb) \($0)" } ?? verb
    }
}

/// Replaces the Edit menu's Undo and Redo. The items are never disabled:
/// the first responder is AppKit state SwiftUI doesn't observe, and a
/// disabled item would also block a text field's own undo.
struct UndoCommands: Commands {
    @FocusedValue(\.schematicUndo) private var target

    var body: some Commands {
        CommandGroup(replacing: .undoRedo) {
            Button(UndoRouting.undoTitle(target)) { perform(undo: true) }
                .keyboardShortcut("z", modifiers: .command)
            Button(UndoRouting.redoTitle(target)) { perform(undo: false) }
                .keyboardShortcut("z", modifiers: [.command, .shift])
        }
    }

    private func perform(undo: Bool) {
        let textIsEditing = (NSApp.keyWindow?.firstResponder as? NSTextView)?.isEditable == true
        switch UndoRouting.route(textIsEditing: textIsEditing, target: target) {
        case .responderChain:
            NSApp.sendAction(Selector(undo ? "undo:" : "redo:"), to: nil, from: nil)
        case let .page(page):
            guard let document = target?.document else { return }
            let done = undo ? document.undo(on: page) : document.redo(on: page)
            if !done { NSSound.beep() }
        }
    }
}
