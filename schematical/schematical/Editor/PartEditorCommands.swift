import SwiftUI

extension FocusedValues {
    /// The focused Part Editor window, for its Save Part… and Open Part… keys.
    @Entry var partEditor: PartEditorModel?
}

/// File › New Part… (⇧⌘N) and, in a Part Editor window, Save Part As… and
/// Add to Library. ⌘O and ⌘S are `SchematicFileCommands`' Open and Save,
/// which become Open Part… and Save Part… while a Part Editor window is
/// focused (one item per key).
struct PartEditorCommands: View {
    let editor: PartEditorModel?
    @Environment(\.openWindow) private var openWindow

    var body: some View {
        Button("New Part…") {
            openWindow(id: PartKindPicker.windowID)
        }
        .keyboardShortcut("n", modifiers: [.command, .shift])

        if let editor {
            Button("Save Part As…") { Task { await editor.saveAs() } }
                .disabled(!editor.isValid)
            Button("Add to Library") { Task { await editor.addToLibraryFromMenu() } }
                .disabled(!editor.isValid)
        }
    }
}
