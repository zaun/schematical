import SwiftUI

/// UserDefaults keys for the app's preferences. `@AppStorage` saves changes as
/// they are made and reads them back on launch.
enum PreferenceKey {
    static let showObsoleteParts = "showObsoleteParts"
    static let showMilitaryParts = "showMilitaryParts"
    static let showAutomotiveParts = "showAutomotiveParts"
    /// The color and footprint id a newly dropped `GENERIC-LED` gets. `""`
    /// (never picked) means the part's default.
    static let genericLEDColor = "genericLEDColor"
    static let genericLEDFootprint = "genericLEDFootprint"
    /// What a newly dropped generic header (plan-23) is: rows (1 or 2), pins
    /// per row (1–40), gender, and footprint type as a package name
    /// (`Straight` or `Right angle`, which fits every size). 0 or `""`
    /// (never picked) means the data default: 1×2, Male, Straight.
    static let headerRows = "headerRows"
    static let headerPinsPerRow = "headerPinsPerRow"
    static let headerGender = "headerGender"
    static let headerFootprintType = "headerFootprintType"
}

/// The options a newly placed part starts with, from Settings. Nil means the
/// part's default; a value the part doesn't have also ends up as the
/// default, through `PlacedChip.init`.
enum NewPartOptions {
    static let genericLED = "GENERIC-LED"

    static func color(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        stored(PreferenceKey.genericLEDColor, for: part, defaults: defaults)
    }

    /// The LED's stored footprint id; for a header, the id of the part's
    /// choice whose package is the stored footprint type.
    static func footprintID(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        if part.kind == PART_HEADER {
            guard let type = defaults.string(forKey: PreferenceKey.headerFootprintType), !type.isEmpty
            else { return nil }
            return part.footprintChoices.first { $0.package == type }?.footprint.id
        }
        return stored(PreferenceKey.genericLEDFootprint, for: part, defaults: defaults)
    }

    /// A header's stored gender; nil for other parts or when never picked.
    static func gender(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        guard part.kind == PART_HEADER, let value = defaults.string(forKey: PreferenceKey.headerGender),
              !value.isEmpty else { return nil }
        return value
    }

    /// The part a dropped header becomes: the stored rows and pins per row
    /// (each falling back to the dropped part's own when 0 or out of range),
    /// or the dropped part when that size isn't in the catalog. Other parts
    /// are returned unchanged.
    static func headerPart(for part: CatalogPart, catalog: PartCatalogStore,
                           defaults: UserDefaults = .standard) -> CatalogPart {
        guard let size = part.headerSize else { return part }
        let rows = defaults.integer(forKey: PreferenceKey.headerRows)
        let perRow = defaults.integer(forKey: PreferenceKey.headerPinsPerRow)
        let name = HeaderFamily.name(rows: (1...2).contains(rows) ? rows : size.rows,
                                     perRow: (1...HeaderFamily.maxPerRow).contains(perRow) ? perRow : size.perRow)
        return catalog.part(named: name) ?? part
    }

    private static func stored(_ key: String, for part: CatalogPart, defaults: UserDefaults) -> String? {
        guard part.name == genericLED, let value = defaults.string(forKey: key), !value.isEmpty else {
            return nil
        }
        return value
    }
}

/// The Settings window (Schematical ▸ Settings…, ⌘,).
struct PreferencesView: View {
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false
    @AppStorage(PreferenceKey.genericLEDColor) private var ledColor = ""
    @AppStorage(PreferenceKey.genericLEDFootprint) private var ledFootprint = ""
    @AppStorage(PreferenceKey.headerRows) private var headerRows = 0
    @AppStorage(PreferenceKey.headerPinsPerRow) private var headerPinsPerRow = 0
    @AppStorage(PreferenceKey.headerGender) private var headerGender = ""
    @AppStorage(PreferenceKey.headerFootprintType) private var headerFootprintType = ""
    /// The Settings scene has no window catalog, so it loads its own.
    @State private var catalog = PartCatalogStore()

    var body: some View {
        Form {
            Section {
                Toggle("Obsolete parts", isOn: $showObsolete)
                Toggle("Military parts", isOn: $showMilitary)
                Toggle("Automotive parts", isOn: $showAutomotive)
            } header: {
                Text("Include in parts list")
            } footer: {
                Text("Military covers SN54, CD54 and SMD (5962-) numbers. Automotive covers AEC-Q100 (-Q1) parts.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            if let led = catalog.part(named: NewPartOptions.genericLED) {
                Section {
                    if !led.colors.isEmpty {
                        ColorChoicePicker(colors: led.colors, selection: Binding(
                            // "" (never picked) or a color the part lost shows the default.
                            get: { led.colors.contains(ledColor) ? ledColor : led.colors[0] },
                            set: { ledColor = $0 }
                        ))
                    }
                    if !led.footprintChoices.isEmpty, let base = led.footprint {
                        FootprintChoicePicker(choices: led.footprintChoices, selection: Binding(
                            get: {
                                led.footprintChoices.contains { $0.footprint.id == ledFootprint }
                                    ? ledFootprint : base.id
                            },
                            set: { ledFootprint = $0 }
                        ))
                    }
                } header: {
                    Text("New Generic LED")
                } footer: {
                    Text("Used for LEDs dropped from now on.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            if let base = catalog.part(named: HeaderFamily.defaultPart), let baseSize = base.headerSize {
                // Each control shows the data default while its key is empty
                // or out of range.
                let rows = (1...2).contains(headerRows) ? headerRows : baseSize.rows
                let perRow = (1...HeaderFamily.maxPerRow).contains(headerPinsPerRow)
                    ? headerPinsPerRow : baseSize.perRow
                let part = catalog.part(named: HeaderFamily.name(rows: rows, perRow: perRow)) ?? base
                Section {
                    Picker("Rows", selection: Binding(get: { rows }, set: { headerRows = $0 })) {
                        Text("1").tag(1)
                        Text("2").tag(2)
                    }
                    .pickerStyle(.segmented)
                    Picker("Pins per row", selection: Binding(get: { perRow }, set: { headerPinsPerRow = $0 })) {
                        ForEach(1...HeaderFamily.maxPerRow, id: \.self) { Text("\($0)").tag($0) }
                    }
                    .pickerStyle(.menu)
                    if !part.genders.isEmpty {
                        Picker("Gender", selection: Binding(
                            get: { part.genders.contains(headerGender) ? headerGender : part.genders[0] },
                            set: { headerGender = $0 }
                        )) {
                            ForEach(part.genders, id: \.self) { Text($0).tag($0) }
                        }
                        .pickerStyle(.segmented)
                    }
                    if !part.footprintChoices.isEmpty, let footprint = part.footprint {
                        FootprintChoicePicker(choices: part.footprintChoices, selection: Binding(
                            get: {
                                part.footprintChoices.first { $0.package == headerFootprintType }?.footprint.id
                                    ?? footprint.id
                            },
                            set: { id in
                                headerFootprintType = part.footprintChoices.first { $0.footprint.id == id }?.package ?? ""
                            }
                        ))
                    }
                } header: {
                    Text("New Generic Header")
                } footer: {
                    Text("Used for headers dropped from now on.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
        .formStyle(.grouped)
        .frame(width: 420)
        .fixedSize(horizontal: false, vertical: true)
        .task {
            if catalog.loadState != .loaded { catalog.load() }
        }
    }
}
