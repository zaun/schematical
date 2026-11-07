import SwiftUI

/// The Schematic page's inspector. It follows the selection: the parts
/// library with nothing selected, the part's header, reference fields and
/// details with a part selected, the bus panel with a bus selected. The library stays in the view tree under
/// the part panel, so its search, filters, open groups and scroll position
/// are as they were when the selection clears. The PCB page keeps it in the
/// tree too, inactive (`isActive` false), so a visit there doesn't reset it.
struct SchematicSidebarView: View {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let simulation: SimulationController
    var isActive = true

    private var selected: PlacedChip? {
        guard let id = document.selectedChipID else { return nil }
        return document.chips.first { $0.id == id }
    }

    private var selectedBus: UUID? {
        guard let id = document.selectedBusID, document.bus(id) != nil else { return nil }
        return id
    }

    var body: some View {
        let chip = isActive ? selected : nil
        // Chip panel first, then the bus panel, then the library.
        let busID = isActive && chip == nil ? selectedBus : nil
        let showsLibrary = isActive && chip == nil && busID == nil
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
            } else if let busID {
                BusPanel(document: document, busID: busID)
                    // Fresh field state for each bus.
                    .id(busID)
            }
        }
    }
}
