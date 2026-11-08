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
    /// The resistance (ohms; 0, never set, means 1 kΩ) and footprint id
    /// (`""` means the part's default) a newly placed pull-up or pull-down gets.
    static let pullResistorOhms = "pullResistorOhms"
    static let pullResistorFootprint = "pullResistorFootprint"
}

/// The options a newly placed part starts with, from Settings. Nil means the
/// part's default; a value the part doesn't have also ends up as the
/// default, through `PlacedChip.init`.
enum NewPartOptions {
    static let genericLED = "GENERIC-LED"
    /// The part whose footprint choices Settings ▸ New Pull Resistor offers
    /// (every pull part has the same ones).
    static let pullDown = "PULLDOWN"

    static func color(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        guard part.name == genericLED else { return nil }
        return stored(PreferenceKey.genericLEDColor, defaults: defaults)
    }

    static func footprintID(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        if part.isPullResistor {
            return stored(PreferenceKey.pullResistorFootprint, defaults: defaults)
        }
        guard part.name == genericLED else { return nil }
        return stored(PreferenceKey.genericLEDFootprint, defaults: defaults)
    }

    /// A pull resistor's stored resistance, if it is valid.
    static func ohms(for part: CatalogPart, defaults: UserDefaults = .standard) -> Double? {
        guard part.isPullResistor else { return nil }
        let value = defaults.double(forKey: PreferenceKey.pullResistorOhms)
        return ResistorValue.isValid(value) ? value : nil
    }

    private static func stored(_ key: String, defaults: UserDefaults) -> String? {
        guard let value = defaults.string(forKey: key), !value.isEmpty else { return nil }
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
    @AppStorage(PreferenceKey.pullResistorOhms) private var pullOhms = 0.0
    @AppStorage(PreferenceKey.pullResistorFootprint) private var pullFootprint = ""
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
            if let pull = catalog.part(named: NewPartOptions.pullDown) {
                Section {
                    // 0 (never set) or a bad value shows the default.
                    OhmsField(ohms: ResistorValue.isValid(pullOhms) ? pullOhms : ResistorValue.defaultOhms) {
                        pullOhms = $0
                    }
                    if !pull.footprintChoices.isEmpty, let base = pull.footprint {
                        FootprintChoicePicker(choices: pull.footprintChoices, selection: Binding(
                            get: {
                                pull.footprintChoices.contains { $0.footprint.id == pullFootprint }
                                    ? pullFootprint : base.id
                            },
                            set: { pullFootprint = $0 }
                        ))
                    }
                } header: {
                    Text("New Pull Resistor")
                } footer: {
                    Text("Used for pull-ups and pull-downs placed from now on.")
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
