import SwiftUI

/// The sockets that fit a placed chip, in a sheet (plan-51-I): a search field,
/// Mounting and Manufacturer filters and a table of the matching sockets,
/// generic first. **Use Socket** switches the chip to the chosen one, keeping
/// its mode; **Cancel** changes nothing.
struct SocketPickerSheet: View {
    @Environment(\.dismiss) private var dismiss

    let catalog: PartCatalogStore
    let document: SchematicDocument
    let chipID: UUID
    /// Called with the chosen socket; the sheet then closes.
    let onUse: (CatalogPart) -> Void

    @State private var search: String
    @State private var mounting: MountingFilter
    @State private var maker: String?
    @State private var selection: String?

    init(catalog: PartCatalogStore, document: SchematicDocument, chipID: UUID, onUse: @escaping (CatalogPart) -> Void) {
        self.catalog = catalog
        self.document = document
        self.chipID = chipID
        self.onUse = onUse
        _search = State(initialValue: "")
        _mounting = State(initialValue: .all)
    }

    /// The Mounting menu's choices.
    enum MountingFilter: String, CaseIterable, Identifiable {
        case all = "All"
        case throughHole = "Through hole"
        case surfaceMount = "Surface mount"

        var id: String { rawValue }
    }

    /// A socket's mounting as the part file says it (`through-hole`, `surface-mount`); "" for a non-socket.
    static func mountingText(_ part: CatalogPart) -> String {
        if case let .socket(_, _, mounting, _) = part.details { return mounting }
        return ""
    }

    /// A socket's contact finish, "" when not given.
    static func finishText(_ part: CatalogPart) -> String {
        if case let .socket(_, _, _, finish) = part.details { return finish ?? "" }
        return ""
    }

    /// A DIP socket's row spacing in inches ("0.3 in"), "" for a PLCC one.
    static func rowSpacingText(_ part: CatalogPart) -> String {
        if case let .socket(_, rowUM, _, _) = part.details, rowUM > 0 {
            return String(format: "%.1f in", Double(rowUM) / 25400)
        }
        return ""
    }

    /// The makers present in `sockets`, in order of first appearance, "Generic" among them.
    static func makers(_ sockets: [CatalogPart]) -> [String] {
        var seen: [String] = []
        for socket in sockets where !seen.contains(socket.manufacturer) { seen.append(socket.manufacturer) }
        return seen
    }

    /// The sockets that pass the filters, in their order: `search` is a case-insensitive substring of the part
    /// number, the manufacturer or the description; `mounting` and `maker` (nil = every maker) must match.
    static func rows(_ sockets: [CatalogPart], search: String, mounting: MountingFilter,
                     maker: String?) -> [CatalogPart] {
        let needle = search.trimmingCharacters(in: .whitespaces)
        return sockets.filter { socket in
            if let maker, socket.manufacturer != maker { return false }
            switch mounting {
            case .all: break
            case .throughHole: if mountingText(socket) != "through-hole" { return false }
            case .surfaceMount: if mountingText(socket) != "surface-mount" { return false }
            }
            guard !needle.isEmpty else { return true }
            return [socket.name, socket.manufacturer, socket.description].contains {
                $0.range(of: needle, options: [.caseInsensitive, .diacriticInsensitive]) != nil
            }
        }
    }

    /// The chip, read again so the sheet follows the document.
    private var chip: PlacedChip? { document.chips.first { $0.id == chipID } }

    private var all: [CatalogPart] {
        guard let base = chip?.part.footprint else { return [] }
        return catalog.sockets(fitting: base)
    }

    var body: some View {
        let sockets = all
        let shown = Self.rows(sockets, search: search, mounting: mounting, maker: maker)
        VStack(alignment: .leading, spacing: 12) {
            Text("Sockets for \(chip?.part.footprint?.description ?? "this part")")
                .font(.headline)
            HStack {
                TextField("Search part number, manufacturer or description", text: $search)
                    .textFieldStyle(.roundedBorder)
                Picker("Mounting", selection: $mounting) {
                    ForEach(MountingFilter.allCases) { Text($0.rawValue).tag($0) }
                }
                .frame(width: 200)
                Picker("Manufacturer", selection: $maker) {
                    Text("All").tag(String?.none)
                    ForEach(Self.makers(sockets), id: \.self) { Text($0).tag(String?.some($0)) }
                }
                .frame(width: 260)
            }
            Table(shown, selection: $selection) {
                TableColumn("Part", value: \.name).width(min: 170, ideal: 215)
                TableColumn("Manufacturer", value: \.manufacturer).width(min: 130, ideal: 190)
                TableColumn("Mounting") { Text(Self.mountingText($0)) }.width(min: 80, ideal: 95)
                TableColumn("Finish") { Text(Self.finishText($0)) }.width(min: 50, ideal: 70)
                TableColumn("Row spacing") { Text(Self.rowSpacingText($0)) }.width(min: 70, ideal: 85)
            }
            .contextMenu(forSelectionType: String.self) { _ in } primaryAction: { use($0.first, in: shown) }
            .frame(minHeight: 260)
            HStack {
                Text("\(shown.count) of \(sockets.count) sockets")
                    .font(.caption).foregroundStyle(.secondary)
                Spacer()
                Button("Cancel", role: .cancel) { dismiss() }
                    .keyboardShortcut(.cancelAction)
                Button("Use Socket") { use(selection, in: shown) }
                .keyboardShortcut(.defaultAction)
                .disabled(Self.socket(named: selection, in: shown) == nil)
            }
        }
        .padding(20)
        .frame(width: 820, height: 460)
        .onAppear { selection = chip?.socket?.part.name }
    }

    /// The shown socket named `name`; nil when there is none.
    static func socket(named name: String?, in rows: [CatalogPart]) -> CatalogPart? {
        rows.first { $0.name == name }
    }

    private func use(_ name: String?, in rows: [CatalogPart]) {
        guard let part = Self.socket(named: name, in: rows) else { return }
        onUse(part)
        dismiss()
    }
}
