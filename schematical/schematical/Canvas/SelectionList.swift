import SwiftUI

// MARK: - Rows

/// One line of the sidebar's selection list: what the item is and where.
struct SelectionRow<Item: Hashable>: Identifiable, Equatable {
    let item: Item
    let title: String
    let detail: String

    var id: Item { item }

    /// What VoiceOver reads for the row.
    var accessibilityLabel: String { detail.isEmpty ? title : "\(title), \(detail)" }
}

/// Turns a page's selection into sidebar rows, in the order the items are
/// drawn: parts, then junctions, then wires, then text blocks, then net
/// labels, then buses (Schematic); footprints, then
/// texts, then traces (PCB).
@MainActor
enum SelectionRows {
    static func schematic(_ document: SchematicDocument) -> [SelectionRow<SchematicItem>] {
        let selection = document.schematicSelection
        var rows: [SelectionRow<SchematicItem>] = []
        for chip in document.chips where selection.contains(.chip(chip.id)) {
            rows.append(SelectionRow(item: .chip(chip.id), title: chip.reference, detail: chip.partNumber))
        }
        for junction in document.junctions where selection.contains(.junction(junction.id)) {
            rows.append(SelectionRow(item: .junction(junction.id), title: "Junction", detail: point(junction.point)))
        }
        for wire in document.wires where selection.contains(.wire(wire.id)) {
            rows.append(SelectionRow(item: .wire(wire.id), title: "Wire",
                                     detail: "\(end(wire.start, in: document)) – \(end(wire.end, in: document))"))
        }
        if selection.contains(.titleBlock) {
            rows.append(SelectionRow(item: .titleBlock, title: "Title Block", detail: preview(document.documentInfo.title)))
        }
        for note in document.notes where selection.contains(.note(note.id)) {
            rows.append(SelectionRow(item: .note(note.id), title: "Text", detail: preview(note.text)))
        }
        for label in document.netLabels where selection.contains(.label(label.id)) {
            rows.append(label.isNet
                ? SelectionRow(item: .label(label.id), title: "Net Label", detail: "\(label.name) (\(label.style.title))")
                : SelectionRow(item: .label(label.id), title: label.style.title, detail: ""))
        }
        for bus in document.buses where selection.contains(.bus(bus.id)) {
            rows.append(SelectionRow(item: .bus(bus.id), title: "Bus", detail: "\(bus.name), \(bus.wires.count) wires"))
        }
        return rows
    }

    static func board(_ document: SchematicDocument) -> [SelectionRow<BoardItem>] {
        let selection = document.boardSelection
        var rows: [SelectionRow<BoardItem>] = []
        for chip in document.chips where selection.contains(.footprint(chip.id)) {
            let footprint = chip.footprint.map { " — \($0.id)" } ?? ""
            rows.append(SelectionRow(item: .footprint(chip.id), title: chip.reference,
                                     detail: chip.partNumber + footprint))
        }
        for text in document.texts where selection.contains(.text(text.id)) {
            rows.append(SelectionRow(item: .text(text.id), title: "Text", detail: preview(text.text)))
        }
        for trace in document.traces where selection.contains(.trace(trace.id)) {
            rows.append(SelectionRow(item: .trace(trace.id), title: "Trace",
                                     detail: "\(trace.layer.title), \(trace.width) mil"))
        }
        for via in document.vias where selection.contains(.via(via.id)) {
            rows.append(SelectionRow(item: .via(via.id), title: "Via",
                                     detail: "\(LengthField.format(via.drill, units: .mm)) / "
                                         + "\(LengthField.format(via.diameter, units: .mm)) mm"))
        }
        for hole in document.holes where selection.contains(.hole(hole.id)) {
            let size = hole.size == .custom ? LengthField.format(hole.drill, units: .mm) + " mm" : hole.size.title
            rows.append(SelectionRow(item: .hole(hole.id), title: "Hole",
                                     detail: size + (hole.plated ? ", plated" : "")))
        }
        for pour in document.pours where selection.contains(.pour(pour.id)) {
            let net = pour.net.flatMap { net in document.planeNetChoices().first { $0.net == net }?.title } ?? "no net"
            rows.append(SelectionRow(item: .pour(pour.id), title: "Pour", detail: "\(net) on \(pour.layer.title)"))
        }
        for keepout in document.keepouts where selection.contains(.keepout(keepout.id)) {
            rows.append(SelectionRow(item: .keepout(keepout.id), title: "Keep-out",
                                     detail: keepout.layers == nil ? "All layers" : "Some layers"))
        }
        return rows
    }

    /// "U1 K", "junction (250, 100)", "bus port B0".
    static func end(_ end: WireEnd, in document: SchematicDocument) -> String {
        switch end {
        case let .pin(ref):
            guard let chip = document.chips.first(where: { $0.id == ref.chipID }) else { return "?" }
            let name = chip.part.pins.first { $0.index == ref.index }?.name ?? "\(ref.index + 1)"
            return "\(chip.reference) \(name)"
        case let .junction(id):
            return document.junctions.first { $0.id == id }.map { "junction \(point($0.point))" } ?? "junction"
        case let .busPin(ref):
            let name = document.port(ref.portID)?.bus.wires.first { $0.id == ref.wireID }?.name
            return name.map { "bus port \($0)" } ?? "bus port"
        case let .label(id):
            return document.netLabels.first { $0.id == id }.map { $0.isNet ? "net \($0.name)" : ($0.isPowerFlag ? "power flag" : "no connect") }
                ?? "net label"
        }
    }

    /// The first line of a text, at most 24 characters.
    static func preview(_ text: String) -> String {
        let line = text.split(separator: "\n", omittingEmptySubsequences: false).first.map(String.init) ?? text
        return line.count > 24 ? String(line.prefix(23)) + "…" : line
    }

    private static func point(_ point: CGPoint) -> String {
        "(\(Int(point.x.rounded())), \(Int(point.y.rounded())))"
    }
}

// MARK: - View

/// The sidebar for a selection that has no inspector of its own: several
/// items, or one wire or junction. One read-only row per item; activating a
/// row selects only that item (and so opens its inspector, if it has one).
/// Clear empties the page's selection.
struct SelectionListSection<Item: Hashable>: View {
    let rows: [SelectionRow<Item>]
    let onSelect: (Item) -> Void
    let onClear: () -> Void

    var body: some View {
        Section {
            ForEach(rows) { row in
                Button {
                    onSelect(row.item)
                } label: {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(row.title)
                            .font(.body.weight(.medium))
                        if !row.detail.isEmpty {
                            Text(row.detail)
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .contentShape(Rectangle())
                }
                .buttonStyle(.plain)
                .accessibilityLabel(row.accessibilityLabel)
                .accessibilityHint("Selects only this item")
            }
        } header: {
            HStack {
                Text("Selection (\(rows.count))")
                    .accessibilityLabel("\(rows.count) selected \(rows.count == 1 ? "item" : "items")")
                Spacer()
                Button("Clear", action: onClear)
                    .buttonStyle(.link)
                    .accessibilityLabel("Clear selection")
            }
        }
    }
}
