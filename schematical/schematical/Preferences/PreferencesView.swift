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
}

/// The options a newly placed part starts with, from Settings. Nil means the
/// part's default; a value the part doesn't have also ends up as the
/// default, through `PlacedChip.init`.
enum NewPartOptions {
    static let genericLED = "GENERIC-LED"

    static func color(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        stored(PreferenceKey.genericLEDColor, for: part, defaults: defaults)
    }

    static func footprintID(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        stored(PreferenceKey.genericLEDFootprint, for: part, defaults: defaults)
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
        }
        .formStyle(.grouped)
        .frame(width: 420)
        .fixedSize(horizontal: false, vertical: true)
        .task {
            if catalog.loadState != .loaded { catalog.load() }
        }
    }
}
