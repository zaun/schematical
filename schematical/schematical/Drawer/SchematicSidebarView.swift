import SwiftUI

/// The Schematic page's inspector. It follows the selection: the parts
/// library with nothing selected, the part's header, reference fields and
/// details with only a part selected, the bus or port panel with a bus or
/// port selected, and a read-only `Selection (N)` list for several items or
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
        if let id = document.selectedPortID, document.port(id) != nil { return .port(id) }
        if let id = document.selectedBusID, document.bus(id) != nil { return .bus(id) }
        if !document.schematicSelection.isEmpty { return .selection }
        return .library
    }

    var body: some View {
        let content = isActive ? Self.content(for: document) : .library
        let chip: PlacedChip? = if case let .chip(id) = content { document.chips.first { $0.id == id } } else { nil }
        let portID: UUID? = if case let .port(id) = content { id } else { nil }
        let busID: UUID? = if case let .bus(id) = content { id } else { nil }
        let showsLibrary = isActive && content == .library
        ZStack {
            ChipDrawerView(catalog: catalog)
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
                    Section("Details") {
                        let rows = PartFacts.rows(for: chip.part)
                        ForEach(rows.indices, id: \.self) { index in
                            LabeledContent(rows[index].label, value: rows[index].value)
                        }
                    }
                }
                .formStyle(.grouped)
                // Fresh field state for each part.
                .id(chip.id)
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
