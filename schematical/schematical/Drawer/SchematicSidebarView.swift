import SwiftUI

/// The Schematic page's inspector. It follows the selection: the parts
/// library with nothing selected, the part's header, reference fields and
/// details with only a part selected, the text panel with only a text block
/// selected, the bus or port panel with a bus or port selected, and a
/// read-only `Selection (N)` list for several items or
/// a lone wire or junction (plan-15). The library stays in the view tree under
/// the part panel, so its search, filters, open groups and scroll position
/// are as they were when the selection clears. The PCB page keeps it in the
/// tree too, inactive (`isActive` false), so a visit there doesn't reset it.
struct SchematicSidebarView: View {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let simulation: SimulationController
    var isActive = true

    /// What the inspector shows.
    enum Content: Equatable {
        case library
        case chip(UUID)
        case note(UUID)
        case port(UUID)
        case bus(UUID)
        case selection
    }

    /// The panel for the document's selection: a lone part's panel, else the
    /// port panel, else the bus panel, else the selection list for any other
    /// item selection, else the library.
    static func content(for document: SchematicDocument) -> Content {
        if let id = document.selectedChipID, document.chips.contains(where: { $0.id == id }) {
            return .chip(id)
        }
        if let id = document.selectedNoteID, document.notes.contains(where: { $0.id == id }) {
            return .note(id)
        }
        if let id = document.selectedPortID, document.port(id) != nil { return .port(id) }
        if let id = document.selectedBusID, document.bus(id) != nil { return .bus(id) }
        if !document.schematicSelection.isEmpty { return .selection }
        return .library
    }

    var body: some View {
        let content = isActive ? Self.content(for: document) : .library
        let chip: PlacedChip? = if case let .chip(id) = content { document.chips.first { $0.id == id } } else { nil }
        let note: SchematicNote? = if case let .note(id) = content { document.notes.first { $0.id == id } } else { nil }
        let portID: UUID? = if case let .port(id) = content { id } else { nil }
        let busID: UUID? = if case let .bus(id) = content { id } else { nil }
        let showsLibrary = isActive && content == .library
        ZStack {
            ChipDrawerView(catalog: catalog)
                // Skip the library when only the selection's data changed.
                .equatable()
                // Hidden under the part panel or the PCB page: no clicks, no
                // keyboard focus.
                .disabled(simulation.isRunning || !showsLibrary)
                .opacity(showsLibrary ? 1 : 0)
                .allowsHitTesting(showsLibrary)
                .accessibilityHidden(!showsLibrary)
            if let chip {
                Form {
                    Section { PartHeader(chip: chip) }
                    ReferenceSection(document: document, chip: chip)
                    if PartOptionsSection.applies(to: chip.part) {
                        PartOptionsSection(catalog: catalog, document: document, chip: chip)
                    }
                    Section("Details") {
                        let rows = PartFacts.rows(for: chip)
                        ForEach(rows.indices, id: \.self) { index in
                            LabeledContent(rows[index].label, value: rows[index].value)
                        }
                    }
                }
                .formStyle(.grouped)
                // Fresh field state for each part.
                .id(chip.id)
            } else if let note {
                Form {
                    NotePanel(document: document, note: note)
                }
                .formStyle(.grouped)
                // Fresh field state for each block.
                .id(note.id)
            } else if let portID {
                BusPortPanel(document: document, portID: portID)
                    // Fresh field state for each port.
                    .id(portID)
            } else if let busID {
                BusPanel(document: document, busID: busID)
                    // Fresh field state for each bus.
                    .id(busID)
            } else if content == .selection {
                Form {
                    SelectionListSection(
                        rows: SelectionRows.schematic(document),
                        onSelect: { document.select($0) },
                        onClear: { document.clearSchematicSelection() }
                    )
                }
                .formStyle(.grouped)
            }
        }
    }
}

/// The selected text block: its words (several lines; Return starts a new
/// one), font size, frame, and Delete. The words apply as they are typed;
/// emptied, the block keeps its last words.
private struct NotePanel: View {
    let document: SchematicDocument
    let note: SchematicNote

    @State private var words = ""
    @FocusState private var isEditing: Bool

    var body: some View {
        Section {
            Text("Text")
                .font(.title2.bold())
            TextEditor(text: $words)
                .font(.body)
                .frame(minHeight: 80, maxHeight: 240)
                .focused($isEditing)
                .accessibilityLabel("Text")
                .onChange(of: words) { _, value in
                    document.setNoteText(note.id, value)
                }
        }

        Section {
            LabeledContent("Font Size") {
                Stepper(value: Binding(
                    get: { note.size },
                    set: { document.setNoteSize(note.id, $0) }
                ), in: SchematicNote.sizeRange) {
                    Text("\(note.size) pt")
                        .monospacedDigit()
                }
            }
            Toggle("Frame", isOn: Binding(
                get: { note.framed },
                set: { document.setNoteFramed(note.id, $0) }
            ))
        }

        Section {
            // ⌘⌫, not ⌫ alone, so it can't fire while the words are being edited.
            Button("Delete Text", role: .destructive) {
                document.deleteNote(note.id)
            }
            .keyboardShortcut(.delete, modifiers: .command)
        }
        .onAppear {
            words = note.text
            startEditingIfAsked()
        }
        .onChange(of: document.editingNoteID) { _, _ in startEditingIfAsked() }
        .onChange(of: isEditing) { _, editing in
            // Left empty: show the words the block kept.
            if !editing, words.isEmpty { words = note.text }
        }
    }

    /// A block just placed or double-clicked: focus its words.
    private func startEditingIfAsked() {
        guard document.editingNoteID == note.id else { return }
        document.editingNoteID = nil
        // After this pass, so the editor is on screen to take focus.
        DispatchQueue.main.async { isEditing = true }
    }
}
