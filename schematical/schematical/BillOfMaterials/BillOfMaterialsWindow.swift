//
//  BillOfMaterialsWindow.swift
//  schematical
//
//  The Bill of Materials: every part on one schematic that goes on the
//  board, one line per orderable part (same part number, package and LED
//  color; pull resistors by value and package), with its quantity and
//  references. Each line has a checkmark; only checked lines are exported
//  or copied. Opened from Window > Bill of
//  Materials; it follows the schematic as it is edited.
//

import AppKit
import SwiftUI
import UniformTypeIdentifiers

/// Which schematic window's Bill of Materials to open.
nonisolated struct BillOfMaterialsTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
}

/// One line of the Bill of Materials.
nonisolated struct BillOfMaterialsLine: Identifiable, Hashable, Sendable {
    let partNumber: String
    let package: String
    /// A pull resistor's resistance ("4.7kΩ") or a capacitor's capacitance
    /// ("100 nF", plan-33-H); empty for every other part.
    let value: String
    /// An LED's color; empty for every other part.
    let color: String
    let description: String
    /// In natural order: R2 before R10.
    let references: [String]

    var id: String { "\(partNumber)\u{1F}\(package)\u{1F}\(value)\u{1F}\(color)" }
    var quantity: Int { references.count }
    var referenceText: String { references.joined(separator: ", ") }
}

nonisolated enum BillOfMaterials {
    /// The lines for `chips`. Parts with no footprint (VCC, GND) are not on
    /// the board, so they are left out. Pull-ups, pull-downs and
    /// `GENERIC-RESISTOR`s are one `RESISTOR` line per value and package,
    /// whatever their rail (plan-34-G: the same thing to buy). Lines are
    /// in the order of their first reference (C before R before U, U2 before U10).
    static func lines(for chips: [PlacedChip]) -> [BillOfMaterialsLine] {
        var groups: [String: (sample: PlacedChip, references: [String])] = [:]
        for chip in chips where chip.part.footprint != nil {
            let line = line(for: chip, references: [])
            groups[line.id, default: (chip, [])].references.append(chip.reference)
        }
        return groups.values
            .map { line(for: $0.sample, references: $0.references.sorted(by: naturalOrder)) }
            .sorted { naturalOrder($0.references[0], $1.references[0]) }
    }

    /// The part number and description of a pull resistor's or
    /// `GENERIC-RESISTOR`'s line.
    static let resistorPartNumber = "RESISTOR"
    static let resistorDescription = "Resistor"
    /// `GENERIC-CAPACITOR`'s line (plan-33-H): any value, so its own part number.
    static let capacitorPartNumber = "CAPACITOR"
    static let capacitorDescription = "Capacitor"

    private static func line(for chip: PlacedChip, references: [String]) -> BillOfMaterialsLine {
        let package = chip.footprintChoice?.package ?? chip.part.package
        if chip.part.hasOhmsField {
            return BillOfMaterialsLine(
                partNumber: resistorPartNumber,
                package: package,
                value: ResistorValue.text(chip.ohms ?? ResistorValue.defaultOhms),
                color: "",
                description: resistorDescription,
                references: references
            )
        }
        if chip.part.isGenericCapacitor {
            return BillOfMaterialsLine(
                partNumber: capacitorPartNumber,
                package: package,
                value: CapacitorValue.text(chip.farads ?? CapacitorValue.defaultFarads),
                color: "",
                description: capacitorDescription,
                references: references
            )
        }
        return BillOfMaterialsLine(
            partNumber: chip.partNumber,
            package: package,
            value: chip.part.capacitorFarads.map(CapacitorValue.text)
                ?? chip.part.resistorOhms.map(ResistorValue.text) ?? "",
            color: chip.color ?? "",
            description: chip.part.description,
            references: references
        )
    }

    private static func naturalOrder(_ a: String, _ b: String) -> Bool {
        a.localizedStandardCompare(b) == .orderedAscending
    }

    /// The lines as CSV (RFC 4180), with a header row.
    static func csv(_ lines: [BillOfMaterialsLine]) -> String {
        let header = ["Quantity", "References", "Part Number", "Package", "Value", "Color", "Description"]
        let rows = lines.map { line in
            [String(line.quantity), line.referenceText, line.partNumber, line.package,
             line.value, line.color, line.description]
        }
        return ([header] + rows)
            .map { $0.map(csvField).joined(separator: ",") }
            .joined(separator: "\r\n") + "\r\n"
    }

    private static func csvField(_ text: String) -> String {
        guard text.contains(where: { $0 == "," || $0 == "\"" || $0.isNewline }) else { return text }
        return "\"" + text.replacingOccurrences(of: "\"", with: "\"\"") + "\""
    }
}

// MARK: - Menu command

extension FocusedValues {
    /// Opens the focused schematic window's Bill of Materials.
    @Entry var openBillOfMaterials: (() -> Void)?
}

/// Window > Bill of Materials (⇧⌘B).
struct BillOfMaterialsCommands: Commands {
    @FocusedValue(\.openBillOfMaterials) private var open

    var body: some Commands {
        CommandGroup(before: .windowList) {
            Button("Bill of Materials") { open?() }
                .keyboardShortcut("b", modifiers: [.command, .shift])
                .disabled(open == nil)
            Divider()
        }
    }
}

// MARK: - Window

/// Root of a Bill of Materials window.
struct BillOfMaterialsWindowView: View {
    let target: BillOfMaterialsTarget

    var body: some View {
        if let session = SchematicSessions.shared.session(target.sessionID) {
            BillOfMaterialsTable(document: session.document)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "list.bullet.rectangle",
                description: Text("The schematic this list belongs to is no longer open.")
            )
            .frame(minWidth: 520, minHeight: 240)
        }
    }
}

/// The table, rebuilt whenever the schematic's parts change.
struct BillOfMaterialsTable: View {
    let document: SchematicDocument
    @State private var selection: Set<BillOfMaterialsLine.ID> = []
    /// Lines whose checkmark is cleared. Kept as the unchecked set so a part
    /// newly placed on the schematic starts checked.
    @State private var unchecked: Set<BillOfMaterialsLine.ID> = []

    var body: some View {
        let lines = BillOfMaterials.lines(for: document.chips)
        let checked = lines.filter { !unchecked.contains($0.id) }
        let hasColor = lines.contains { !$0.color.isEmpty }
        let hasValue = lines.contains { !$0.value.isEmpty }
        Group {
            if lines.isEmpty {
                ContentUnavailableView(
                    "No Parts",
                    systemImage: "list.bullet.rectangle",
                    description: Text("Parts placed on the schematic are listed here.")
                )
            } else {
                Table(lines, selection: $selection) {
                    TableColumn("") { line in
                        Toggle("Export", isOn: checkBinding(line.id))
                            .labelsHidden()
                            .help("Include this line in the export")
                    }
                    .width(24)
                    TableColumn("Qty") { Text("\($0.quantity)").monospacedDigit() }
                        .width(min: 32, ideal: 36, max: 60)
                    TableColumn("References", value: \.referenceText)
                        .width(min: 80, ideal: 160)
                    TableColumn("Part Number", value: \.partNumber)
                        .width(min: 90, ideal: 140)
                    TableColumn("Package", value: \.package)
                        .width(min: 60, ideal: 90)
                    if hasValue {
                        TableColumn("Value", value: \.value)
                            .width(min: 40, ideal: 60)
                    }
                    if hasColor {
                        TableColumn("Color", value: \.color)
                            .width(min: 40, ideal: 60)
                    }
                    TableColumn("Description", value: \.description)
                        .width(min: 120, ideal: 260)
                }
                .onCopyCommand {
                    let chosen = lines.filter { selection.contains($0.id) }
                    return [NSItemProvider(object: BillOfMaterials.csv(chosen.isEmpty ? checked : chosen) as NSString)]
                }
            }
        }
        .frame(minWidth: 520, minHeight: 240)
        .navigationTitle("Bill of Materials")
        .navigationSubtitle(summary(lines, checked: checked))
        .toolbar {
            ToolbarItem(placement: .automatic) {
                Toggle("Select All", isOn: Binding(
                    get: { !lines.isEmpty && checked.count == lines.count },
                    set: { unchecked = $0 ? [] : Set(lines.map(\.id)) }
                ))
                .help("Check or clear every line")
                .disabled(lines.isEmpty)
            }
            ToolbarItem(placement: .primaryAction) {
                Button {
                    export(checked)
                } label: {
                    Label("Export CSV…", systemImage: "square.and.arrow.up")
                }
                .help("Save the checked lines of the Bill of Materials as a CSV file")
                .disabled(checked.isEmpty)
            }
        }
    }

    private func checkBinding(_ id: BillOfMaterialsLine.ID) -> Binding<Bool> {
        Binding(
            get: { !unchecked.contains(id) },
            set: { if $0 { unchecked.remove(id) } else { unchecked.insert(id) } }
        )
    }

    private func summary(_ lines: [BillOfMaterialsLine], checked: [BillOfMaterialsLine]) -> String {
        let parts = lines.reduce(0) { $0 + $1.quantity }
        let text = "\(parts) part\(parts == 1 ? "" : "s"), \(lines.count) line\(lines.count == 1 ? "" : "s")"
        return checked.count == lines.count ? text : text + ", \(checked.count) checked"
    }

    private func export(_ lines: [BillOfMaterialsLine]) {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.commaSeparatedText]
        panel.nameFieldStringValue = "Bill of Materials.csv"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        do {
            try BillOfMaterials.csv(lines).write(to: url, atomically: true, encoding: .utf8)
        } catch {
            NSAlert(error: error).runModal()
        }
    }
}
