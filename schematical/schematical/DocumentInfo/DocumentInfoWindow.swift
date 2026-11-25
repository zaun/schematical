import SwiftUI

// Document Info (plan-64): the title, author, revision, description and dates
// of a schematic, in a window beside the document's own (⌘I), the way Bill of
// Materials is. The same form is the sidebar's panel for the title block.

/// Which schematic window's Document Info to open.
nonisolated struct DocumentInfoTarget: Codable, Hashable, Sendable {
    let documentID: UUID
}

// MARK: - Menu command

/// Window › Document Info (⌘I).
struct DocumentInfoCommands: Commands {
    @FocusedValue(\.schematicEditor) private var editor

    var body: some Commands {
        CommandGroup(before: .windowList) {
            DocumentInfoMenuItem(editor: editor)
        }
    }
}

/// A view, so it is rendered again when the editor's menus change
/// (`EditorMenus`).
private struct DocumentInfoMenuItem: View {
    let editor: SchematicEditorModel?

    var body: some View {
        let open = editor?.menus.openDocumentInfo
        Button("Document Info") { open?() }
            .keyboardShortcut("i", modifiers: .command)
            .disabled(open == nil)
    }
}

// MARK: - Window

/// Root of a Document Info window.
struct DocumentInfoWindowView: View {
    let target: DocumentInfoTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
            Form {
                DocumentInfoForm(document: session.document, isLocked: session.simulation.isRunning)
            }
            .formStyle(.grouped)
            .frame(minWidth: 380, minHeight: 440)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "info.circle",
                description: Text("The schematic this information belongs to is no longer open.")
            )
            .frame(minWidth: 380, minHeight: 240)
        }
    }
}

// MARK: - The form

/// The fields, as sections of a `Form`: the window's content, and the title
/// block's sidebar panel. Every edit is an undo step on the document; none
/// can be made while the simulation runs (`isLocked`), when the document's
/// undo is locked.
struct DocumentInfoForm: View {
    let document: SchematicDocument
    let isLocked: Bool

    var body: some View {
        let info = document.documentInfo
        Group {
            Section {
                Toggle("Include on Schematic", isOn: Binding(get: { document.documentInfo.titleBlock != nil },
                                                             set: { document.setTitleBlockShown($0) }))
            }
            Section("Document") {
                TextField("Title", text: Binding(get: { document.documentInfo.title },
                                                 set: { document.setInfoTitle($0) }))
                TextField("Author", text: Binding(get: { document.documentInfo.author },
                                                  set: { document.setInfoAuthor($0) }))
                TextField("Revision", text: Binding(get: { document.documentInfo.revision },
                                                    set: { document.setInfoRevision($0) }))
                VStack(alignment: .leading, spacing: 4) {
                    Text("Description")
                    TextEditor(text: Binding(get: { document.documentInfo.description },
                                             set: { document.setInfoDescription($0) }))
                        .font(.system(.body, design: .monospaced))
                        .frame(height: 96)
                }
            }
            Section("Dates") {
                Toggle("Set dates automatically", isOn: Binding(get: { document.documentInfo.automaticDates },
                                                                set: { document.setAutomaticDates($0) }))
                dateRow("Created", info.created, automatic: info.automaticDates) { document.setInfoCreated($0) }
                dateRow("Updated", info.updated, automatic: info.automaticDates) { document.setInfoUpdated($0) }
            }
        }
        .disabled(isLocked)
    }

    /// One date: read-only text while dates are automatic (the saved value, or
    /// what will fill it), else a date and time picker, or a button that sets
    /// the date to now when it has none.
    @ViewBuilder
    private func dateRow(_ label: String, _ date: Date?, automatic: Bool,
                         set: @escaping (Date?) -> Void) -> some View {
        if automatic {
            LabeledContent(label, value: date == nil ? "Set when the file is saved" : DocumentInfo.dateText(date))
        } else if let date {
            DatePicker(label, selection: Binding(get: { date }, set: { set($0) }),
                       displayedComponents: [.date, .hourAndMinute])
        } else {
            LabeledContent(label) { Button("Set to Now") { set(Date()) } }
        }
    }
}
