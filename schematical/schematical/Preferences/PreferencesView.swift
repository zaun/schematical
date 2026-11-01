import SwiftUI

/// UserDefaults keys for the app's preferences. `@AppStorage` saves changes as
/// they are made and reads them back on launch.
enum PreferenceKey {
    static let showObsoleteParts = "showObsoleteParts"
    static let showMilitaryParts = "showMilitaryParts"
    static let showAutomotiveParts = "showAutomotiveParts"
}

/// The Settings window (Schematical ▸ Settings…, ⌘,).
struct PreferencesView: View {
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false

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
        }
        .formStyle(.grouped)
        .frame(width: 420)
        .fixedSize(horizontal: false, vertical: true)
    }
}
