import SwiftUI

/// A kind of part, for the reference prefix it gets (Settings ▸ Reference Defaults).
/// The defaults are the standard ones (IEEE 315 / ASME Y14.44). Some
/// categories have no catalog parts yet; they are ready for when they do.
nonisolated enum ReferenceCategory: String, CaseIterable, Identifiable, Sendable {
    case battery, capacitor, diode, zenerDiode, led, display, fuse, ferriteBead, oscillator
    case femaleConnector, relay, inductor, speaker, maleConnector, powerSymbol, transistor
    case resistor, switchPart, transformer, testPoint
    case integratedCircuit, memory, microcontroller, processor
    case crystal

    var id: String { rawValue }

    var title: String {
        switch self {
        case .battery: "Battery"
        case .capacitor: "Capacitor"
        case .diode: "Diode"
        case .zenerDiode: "Zener Diode"
        case .led: "LED"
        case .display: "Display"
        case .fuse: "Fuse"
        case .ferriteBead: "Ferrite Bead"
        case .oscillator: "Oscillator"
        case .femaleConnector: "Female Connector"
        case .relay: "Relay"
        case .inductor: "Inductor"
        case .speaker: "Speaker / Buzzer"
        case .maleConnector: "Male Connector"
        case .powerSymbol: "Power Symbol"
        case .transistor: "Transistor"
        case .resistor: "Resistor"
        case .switchPart: "Switch"
        case .transformer: "Transformer"
        case .testPoint: "Test Point"
        case .integratedCircuit: "Integrated Circuit"
        case .memory: "Memory"
        case .microcontroller: "Microcontroller"
        case .processor: "Processor"
        case .crystal: "Crystal"
        }
    }

    /// The standard prefix.
    var standardPrefix: String {
        switch self {
        case .battery: "BT"
        case .capacitor: "C"
        case .diode, .zenerDiode, .led: "D"
        case .display: "DS"
        case .fuse: "F"
        case .ferriteBead: "FB"
        case .oscillator: "G"
        case .femaleConnector: "J"
        case .relay: "K"
        case .inductor: "L"
        case .speaker: "LS"
        case .maleConnector: "P"
        case .powerSymbol: "PWR"
        case .transistor: "Q"
        case .resistor: "R"
        case .switchPart: "S"
        case .transformer: "T"
        case .testPoint: "TP"
        case .integratedCircuit, .memory, .microcontroller, .processor: "U"
        case .crystal: "Y"
        }
    }

    /// The category of a part; `gender` is a placed header's.
    static func of(_ part: CatalogPart, gender: String? = nil) -> ReferenceCategory {
        if part.isPullResistor { return .resistor }
        switch part.kind {
        case PART_SUPPLY: return .powerSymbol
        case PART_CAPACITOR: return .capacitor
        case PART_DIODE:
            if case let .diode(type, _, _, _, _, _) = part.details, type == "zener" { return .zenerDiode }
            return .diode
        case PART_LED: return .led
        case PART_7SEG, PART_CHARLCD: return .display
        case PART_OSCILLATOR: return .oscillator
        case PART_SWITCH: return .switchPart
        case PART_CONNECTOR: return .femaleConnector
        case PART_HEADER:
            return (gender ?? part.genders.first) == "Female" ? .femaleConnector : .maleConnector
        case PART_SRAM, PART_EEPROM: return .memory
        case PART_CPU: return .processor
        default: return .integratedCircuit
        }
    }
}

/// The prefixes picked in Settings ▸ Reference Defaults, stored as JSON
/// `[category raw value: prefix]` under `PreferenceKey.referencePrefixes`.
/// A category not stored (or stored badly) uses its standard prefix.
nonisolated enum ReferencePrefixes {
    static func stored(_ data: Data) -> [String: String] {
        (try? JSONDecoder().decode([String: String].self, from: data)) ?? [:]
    }

    static func encoded(_ prefixes: [String: String]) -> Data {
        (try? JSONEncoder().encode(prefixes)) ?? Data()
    }

    /// The prefix a category gets with `data`'s overrides.
    static func prefix(for category: ReferenceCategory, in data: Data) -> String {
        if let value = stored(data)[category.rawValue], isValid(value) { return value.uppercased() }
        return category.standardPrefix
    }

    static func prefix(for category: ReferenceCategory, defaults: UserDefaults = .standard) -> String {
        prefix(for: category, in: defaults.data(forKey: PreferenceKey.referencePrefixes) ?? Data())
    }

    /// The prefix a new part gets; `gender` is a placed header's.
    static func prefix(for part: CatalogPart, gender: String? = nil,
                       defaults: UserDefaults = .standard) -> String {
        prefix(for: ReferenceCategory.of(part, gender: gender), defaults: defaults)
    }

    /// 1 to 8 ASCII letters, as `ReferenceParts.validate` allows.
    static func isValid(_ prefix: String) -> Bool {
        (1...8).contains(prefix.count) && prefix.allSatisfy { $0.isASCII && $0.isLetter }
    }
}

/// Settings ▸ Reference Defaults: the prefix each kind of part's reference starts with.
struct ReferencePreferencesView: View {
    @AppStorage(PreferenceKey.referencePrefixes) private var data = Data()

    var body: some View {
        Form {
            Section {
                ForEach(ReferenceCategory.allCases) { category in
                    LabeledContent(category.title) {
                        PrefixField(prefix: ReferencePrefixes.prefix(for: category, in: data)) { typed in
                            var prefixes = ReferencePrefixes.stored(data)
                            if typed == category.standardPrefix {
                                prefixes[category.rawValue] = nil
                            } else {
                                prefixes[category.rawValue] = typed
                            }
                            data = ReferencePrefixes.encoded(prefixes)
                        }
                        .frame(width: 80)
                    }
                }
            } header: {
                Text("Reference Prefixes")
            } footer: {
                Text("Used for parts placed from now on, and by Edit › Reset References. A prefix is 1 to 8 letters.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            if !ReferencePrefixes.stored(data).isEmpty {
                Section {
                    Button("Reset to Standard Prefixes") { data = Data() }
                }
            }
        }
        .formStyle(.grouped)
    }
}

/// A prefix text field: commits on Return or losing focus; a prefix that
/// isn't 1 to 8 letters goes back to the current one.
private struct PrefixField: View {
    let prefix: String
    let commit: (String) -> Void
    @State private var text = ""
    @FocusState private var focused: Bool

    var body: some View {
        TextField("Prefix", text: $text)
            .labelsHidden()
            .multilineTextAlignment(.trailing)
            .focused($focused)
            .onAppear { text = prefix }
            .onChange(of: prefix) { _, new in text = new }
            .onSubmit(apply)
            .onChange(of: focused) { _, isFocused in if !isFocused { apply() } }
    }

    private func apply() {
        let typed = text.trimmingCharacters(in: .whitespaces).uppercased()
        if ReferencePrefixes.isValid(typed) {
            commit(typed)
            text = typed
        } else {
            text = prefix
        }
    }
}
