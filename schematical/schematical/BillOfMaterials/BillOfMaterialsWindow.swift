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
    let documentID: UUID
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
    /// The part's maker; empty for a Generic part (no orderable part number).
    var manufacturer = ""
    /// Whether a fab's machines place the part: surface mount, on the board (`PlacedChip.isSurfaceMount`).
    var isSurfaceMount = false

    var id: String { "\(partNumber)\u{1F}\(package)\u{1F}\(value)\u{1F}\(color)" }
    var quantity: Int { references.count }
    var referenceText: String { references.joined(separator: ", ") }
}

nonisolated enum BillOfMaterials {
    /// The lines for `chips`. Parts with no footprint (VCC, GND) are not on
    /// the board, so they are left out; so are test points, which are not
    /// bought (plan-57). Pull-ups, pull-downs and
    /// `GENERIC-RESISTOR`s are one `RESISTOR` line per value and package,
    /// whatever their rail (plan-34-G: the same thing to buy). Lines are
    /// in the order of their first reference (C before R before U, U2 before U10).
    ///
    /// A chip in a socket mode (plan-51-H) adds the socket's line: for IC + Socket the
    /// chip's own line and the socket's, for Socket only the socket's alone. The socket
    /// line's reference is `X` plus the chip's (`XU1`, `PlacedChip.fabricationReference`),
    /// and chips with the same socket part share the line.
    static func lines(for chips: [PlacedChip]) -> [BillOfMaterialsLine] {
        var groups: [String: (make: ([String]) -> BillOfMaterialsLine, references: [String])] = [:]
        func add(_ line: BillOfMaterialsLine, reference: String, make: @escaping ([String]) -> BillOfMaterialsLine) {
            groups[line.id, default: (make, [])].references.append(reference)
        }
        for chip in chips where chip.part.footprint != nil && !chip.part.isTestPoint {
            if chip.socket?.mode != .socketOnly {
                add(line(for: chip, references: []), reference: chip.reference) { line(for: chip, references: $0) }
            }
            if let socket = chip.socket {
                add(socketLine(socket, references: []), reference: chip.fabricationReference) {
                    socketLine(socket, references: $0)
                }
            }
        }
        return groups.values
            .map { $0.make($0.references.sorted(by: naturalOrder)) }
            .sorted { naturalOrder($0.references[0], $1.references[0]) }
    }

    /// The line of a chip's socket: its part number and package ("DIP socket").
    private static func socketLine(_ socket: ChipSocket, references: [String]) -> BillOfMaterialsLine {
        let pads = socket.part.footprint?.pads ?? []
        return BillOfMaterialsLine(
            partNumber: socket.part.name,
            package: socket.part.package,
            value: "",
            color: "",
            description: socket.part.description,
            references: references,
            manufacturer: socket.part.manufacturer == "Generic" ? "" : socket.part.manufacturer,
            isSurfaceMount: !pads.isEmpty && pads.allSatisfy { $0.drill == 0 }
        )
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
                references: references,
                isSurfaceMount: chip.isSurfaceMount && chip.socket == nil
            )
        }
        if chip.part.isGenericCapacitor {
            return BillOfMaterialsLine(
                partNumber: capacitorPartNumber,
                package: package,
                value: CapacitorValue.text(chip.farads ?? CapacitorValue.defaultFarads),
                color: "",
                description: capacitorDescription,
                references: references,
                isSurfaceMount: chip.isSurfaceMount && chip.socket == nil
            )
        }
        return BillOfMaterialsLine(
            partNumber: chip.partNumber,
            package: package,
            value: chip.part.capacitorFarads.map(CapacitorValue.text)
                ?? chip.part.resistorOhms.map(ResistorValue.text) ?? "",
            color: chip.color ?? "",
            description: chip.part.description,
            references: references,
            manufacturer: chip.part.manufacturer == "Generic" ? "" : chip.part.manufacturer,
            // A chip in a socket is plugged in by hand; the socket's own line is the placed one.
            isSurfaceMount: chip.isSurfaceMount && chip.socket == nil
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

    private static func csvField(_ text: String) -> String { CSV.field(text) }

    /// The most designators JLCPCB takes on one BOM row.
    static let jlcpcbMaxDesignators = 200

    /// The lines as the BOM JLCPCB and PCBWay assembly take (`docs/export.md` § JLCPCB / PCBWay BOM):
    /// `Comment,Designator,Footprint,Quantity,Manufacturer,Manufacturer Part Number,LCSC Part #`, every field quoted, designators
    /// joined with `,` and no space. Only surface-mount lines are written, the parts the pick-and-place file
    /// (`PickAndPlaceExport`) lists, so the two files name the same designators. `Comment` is the part's value
    /// (a resistor's or capacitor's), else its part number (and an LED's color). `LCSC Part #` is left empty: the
    /// catalog has no LCSC numbers, so it is filled in on the fab's site. A line of more than 200 designators is
    /// split, as JLCPCB's limit is 200 per row.
    static func jlcpcbCSV(_ lines: [BillOfMaterialsLine]) -> String {
        let header = ["Comment", "Designator", "Footprint", "Quantity", "Manufacturer", "Manufacturer Part Number",
                      "LCSC Part #"]
        var rows: [[String]] = []
        for line in lines where line.isSurfaceMount {
            let comment = line.value.isEmpty ? ([line.partNumber] + (line.color.isEmpty ? [] : [line.color])).joined(separator: " ")
                : line.value
            let number = line.manufacturer.isEmpty ? "" : line.partNumber
            var rest = line.references[...]
            while !rest.isEmpty {
                let chunk = rest.prefix(jlcpcbMaxDesignators)
                rest = rest.dropFirst(chunk.count)
                rows.append([comment, chunk.joined(separator: ","), line.package, String(chunk.count),
                             line.manufacturer, number, ""])
            }
        }
        return ([header] + rows)
            .map { $0.map { "\"" + $0.replacingOccurrences(of: "\"", with: "\"\"") + "\"" }.joined(separator: ",") }
            .joined(separator: "\r\n") + "\r\n"
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
        if let session = SchematicEditorModel.session(target.documentID) {
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
            ToolbarItem(placement: .primaryAction) {
                Button {
                    exportJLCPCB(checked)
                } label: {
                    Label("Export JLCPCB / PCBWay BOM…", systemImage: "square.and.arrow.up.on.square")
                }
                .help("Save the checked surface-mount lines in the assembly BOM layout JLCPCB and PCBWay take (pair it with the pick-and-place file in the Gerber export)")
                .disabled(!checked.contains { $0.isSurfaceMount })
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
        save(BillOfMaterials.csv(lines), as: "Bill of Materials.csv")
    }

    private func exportJLCPCB(_ lines: [BillOfMaterialsLine]) {
        save(BillOfMaterials.jlcpcbCSV(lines), as: "Bill of Materials JLCPCB.csv")
    }

    private func save(_ text: String, as name: String) {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.commaSeparatedText]
        panel.nameFieldStringValue = name
        guard panel.runModal() == .OK, let url = panel.url else { return }
        do {
            try text.write(to: url, atomically: true, encoding: .utf8)
        } catch {
            NSAlert(error: error).runModal()
        }
    }
}
