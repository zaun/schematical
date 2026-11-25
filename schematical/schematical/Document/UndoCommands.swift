import AppKit
import SwiftUI

// Edit › Undo and Redo (plan-31-D, plan-61-A). In a schematic window they act
// on its document, whichever page it shows; while a text field is being
// edited they are that field's own typing undo, as in any Mac app.

/// The focused schematic window's document.
struct UndoTarget {
    let document: SchematicDocument
}

extension FocusedValues {
    /// Set by each schematic window while it is key.
    @Entry var schematicUndo: UndoTarget?
}

/// What ⌘Z / ⇧⌘Z do.
enum UndoRoute: Equatable {
    /// To the first responder: text editing, or a window with no schematic.
    case responderChain
    /// The document's undo manager.
    case document
}

enum UndoRouting {
    static func route(textIsEditing: Bool, target: UndoTarget?) -> UndoRoute {
        guard !textIsEditing, let target else { return .responderChain }
        return .document
    }

    /// "Undo Move", or "Undo" with nothing to undo or while playing.
    static func undoTitle(_ target: UndoTarget?) -> String {
        title("Undo", target.flatMap { $0.document.isPlaying ? nil : $0.document.undoName })
    }

    /// "Redo Move", or "Redo" with nothing to redo or while playing.
    static func redoTitle(_ target: UndoTarget?) -> String {
        title("Redo", target.flatMap { $0.document.isPlaying ? nil : $0.document.redoName })
    }

    private static func title(_ verb: String, _ name: String?) -> String {
        name.map { "\(verb) \($0)" } ?? verb
    }
}

/// Replaces the Edit menu's Undo and Redo. The items are disabled only while
/// the simulation runs: the first responder is AppKit state SwiftUI doesn't
/// observe, so they cannot follow whether there is something to undo.
struct UndoCommands: Commands {
    @FocusedValue(\.schematicUndo) private var target

    var body: some Commands {
        CommandGroup(replacing: .undoRedo) {
            Button(UndoRouting.undoTitle(target)) { perform(undo: true) }
                .keyboardShortcut("z", modifiers: .command)
                .disabled(target?.document.isPlaying == true)
            Button(UndoRouting.redoTitle(target)) { perform(undo: false) }
                .keyboardShortcut("z", modifiers: [.command, .shift])
                .disabled(target?.document.isPlaying == true)
        }
    }

    private func perform(undo: Bool) {
        let textIsEditing = (NSApp.keyWindow?.firstResponder as? NSTextView)?.isEditable == true
        switch UndoRouting.route(textIsEditing: textIsEditing, target: target) {
        case .responderChain:
            NSApp.sendAction(Selector(undo ? "undo:" : "redo:"), to: nil, from: nil)
        case .document:
            guard let document = target?.document else { return }
            let done = undo ? document.undo() : document.redo()
            if !done { NSSound.beep() }
        }
    }
}
