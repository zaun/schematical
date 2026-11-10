import SwiftUI

/// The limits a design rule check holds a board to, in mm. Nothing checks
/// them yet; Settings only stores them.
nonisolated struct DesignRules: Codable, Equatable, Sendable {
    var minTraceWidth: Double
    var minClearance: Double
    var minDrill: Double
    var minViaDrill: Double
    var minViaDiameter: Double
    var minAnnularRing: Double
    var minHoleToHole: Double
    var minCopperToEdge: Double
    var minSilkscreenWidth: Double
    var minSilkscreenHeight: Double
    var minSilkscreenToPad: Double
    var minSolderMaskDam: Double
    var solderMaskExpansion: Double

    /// What Custom starts as: PCBWay's rules.
    static let customDefault = pcbway

    /// One rule's row in Settings.
    struct Rule: Identifiable {
        let title: String
        let field: WritableKeyPath<DesignRules, Double>
        let help: String
        var id: String { title }
    }

    /// The rules in the order Settings shows them, grouped.
    static let groups: [(title: String, rules: [Rule])] = [
        ("Copper", [
            Rule(title: "Min trace width", field: \.minTraceWidth, help: "The narrowest trace"),
            Rule(title: "Min clearance", field: \.minClearance,
                 help: "The smallest gap between copper on different nets"),
            Rule(title: "Min copper to edge", field: \.minCopperToEdge,
                 help: "The smallest gap between copper and the board outline"),
        ]),
        ("Holes", [
            Rule(title: "Min drill", field: \.minDrill, help: "The smallest plated hole"),
            Rule(title: "Min annular ring", field: \.minAnnularRing,
                 help: "The narrowest copper ring around a plated hole"),
            Rule(title: "Min hole to hole", field: \.minHoleToHole,
                 help: "The smallest gap between the edges of two holes"),
        ]),
        ("Vias", [
            Rule(title: "Min via drill", field: \.minViaDrill, help: "The smallest via hole"),
            Rule(title: "Min via diameter", field: \.minViaDiameter, help: "The smallest via pad"),
        ]),
        ("Silkscreen", [
            Rule(title: "Min line width", field: \.minSilkscreenWidth, help: "The thinnest silkscreen line"),
            Rule(title: "Min text height", field: \.minSilkscreenHeight, help: "The smallest silkscreen text"),
            Rule(title: "Min to pad", field: \.minSilkscreenToPad,
                 help: "The smallest gap between silkscreen and an exposed pad"),
        ]),
        ("Solder Mask", [
            Rule(title: "Min mask dam", field: \.minSolderMaskDam,
                 help: "The narrowest strip of solder mask between two pads"),
            Rule(title: "Mask expansion", field: \.solderMaskExpansion,
                 help: "How far the mask opening extends past each pad"),
        ]),
    ]

    /// The rules stored in `data`; Custom's defaults when there are none or
    /// they don't decode.
    static func decoded(_ data: Data) -> DesignRules {
        (try? JSONDecoder().decode(DesignRules.self, from: data)) ?? customDefault
    }

    var encoded: Data { (try? JSONEncoder().encode(self)) ?? Data() }

    /// The rules a check would use: the stored manufacturer's, or the
    /// stored Custom rules.
    static func current(defaults: UserDefaults = .standard) -> DesignRules {
        let manufacturer = defaults.string(forKey: PreferenceKey.drcManufacturer)
            .flatMap(DRCManufacturer.init(rawValue:)) ?? .custom
        return manufacturer.fixedRules
            ?? decoded(defaults.data(forKey: PreferenceKey.drcCustomRules) ?? Data())
    }
}

/// Settings ▸ DRC Defaults: the manufacturer and the design rules it sets.
/// A manufacturer's rules are shown read only; Custom's are editable and kept when
/// another manufacturer is picked.
struct DesignRulePreferencesView: View {
    @AppStorage(PreferenceKey.drcManufacturer) private var manufacturer = DRCManufacturer.custom
    @AppStorage(PreferenceKey.drcCustomRules) private var customData = Data()

    var body: some View {
        // Two columns: the manufacturer, Copper and Holes; then the rest.
        HStack(alignment: .top, spacing: 0) {
            column(DesignRules.groups.prefix(2), showsManufacturer: true)
            column(DesignRules.groups.dropFirst(2), showsReset: true)
        }
        .frame(width: 760)
    }

    /// A column of rule groups; the first column also holds the
    /// manufacturer, the last the reset button.
    private func column(_ groups: ArraySlice<(title: String, rules: [DesignRules.Rule])>,
                        showsManufacturer: Bool = false, showsReset: Bool = false) -> some View {
        let custom = DesignRules.decoded(customData)
        let rules = manufacturer.fixedRules ?? custom
        return Form {
            if showsManufacturer {
                Section {
                    Picker("Manufacturer", selection: $manufacturer) {
                        ForEach(DRCManufacturer.allCases, id: \.self) { Text($0.title).tag($0) }
                    }
                    .pickerStyle(.menu)
                } footer: {
                    Text(manufacturer == .custom
                         ? "All lengths are in mm."
                         : "Standard limits from \(manufacturer.title). Pick Custom to set your own.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            ForEach(groups, id: \.title) { group in
                Section(group.title) {
                    ForEach(group.rules) { rule in
                        if manufacturer == .custom {
                            LabeledContent(rule.title) {
                                HStack(spacing: 4) {
                                    LengthField(title: rule.title, millimetres: rules[keyPath: rule.field],
                                                units: .mm, precise: true) { typed in
                                        var next = custom
                                        next[keyPath: rule.field] = max(0, typed)
                                        customData = next.encoded
                                        return next[keyPath: rule.field]
                                    }
                                    .labelsHidden()
                                    .frame(width: 80)
                                    Text("mm").foregroundStyle(.secondary)
                                }
                            }
                            .help(rule.help)
                        } else {
                            LabeledContent(rule.title) {
                                Text(LengthField.format(rules[keyPath: rule.field], units: .mm, precise: true)
                                     + " mm")
                            }
                            .help(rule.help)
                        }
                    }
                }
            }
            if showsReset && manufacturer == .custom && custom != DesignRules.customDefault {
                Section {
                    Button("Reset to PCBWay Values") { customData = DesignRules.customDefault.encoded }
                }
            }
        }
        .formStyle(.grouped)
        .frame(maxWidth: .infinity)
        .fixedSize(horizontal: false, vertical: true)
    }
}
