import SwiftUI

/// UserDefaults keys for the app's preferences. `@AppStorage` saves changes as
/// they are made and reads them back on launch.
nonisolated enum PreferenceKey {
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
    /// The resistance (ohms; 0, never set, means 1 kΩ) and footprint id
    /// (`""` means the part's default) a newly placed pull-up or pull-down gets.
    static let pullResistorOhms = "pullResistorOhms"
    static let pullResistorFootprint = "pullResistorFootprint"
    /// The design rule manufacturer (a `DRCManufacturer` raw value) and the
    /// Custom rules (JSON `DesignRules`), for Settings ▸ DRC Defaults.
    static let drcManufacturer = "drcManufacturer"
    static let drcCustomRules = "drcCustomRules"
    /// The reference prefix per part category, for Settings ▸ Reference Defaults
    /// (JSON; see `ReferencePrefixes`).
    static let referencePrefixes = "referencePrefixes"
    /// Net label and bus wire names in uppercase only: what is typed is
    /// uppercased. On unless turned off (names are case sensitive, so
    /// "clk" and "CLK" would be two nets).
    static let uppercaseNames = "uppercaseNames"
    /// Whether a buzzer makes a sound in play mode (plan-47-N). On unless
    /// turned off.
    static let playBuzzerSounds = "playBuzzerSounds"
    /// Whether the AI agent control server runs (`AutomationServer`). Off
    /// unless turned on.
    static let automationEnabled = "automationEnabled"
    /// The secret an agent must send (`AutomationServer.token`).
    static let automationToken = "automationToken"
    /// A security-scoped bookmark of the folder agents may open and save in
    /// (`AutomationFolder`).
    static let automationFolder = "automationFolder"
    /// The last Autoroute options, as JSON (`RoutingOptions`, scope left
    /// out: it comes from the selection each time).
    static let autorouteOptions = "autorouteOptions"
}

/// Settings ▸ General Defaults ▸ Names: how net label and bus wire names
/// are typed.
enum NamePreferences {
    /// Uppercase only? On while the key is unset.
    static func uppercaseOnly(defaults: UserDefaults = .standard) -> Bool {
        defaults.object(forKey: PreferenceKey.uppercaseNames) as? Bool ?? true
    }

    /// `name` as typed into a net label or bus wire name: uppercased while
    /// `uppercaseOnly`.
    static func typed(_ name: String, defaults: UserDefaults = .standard) -> String {
        uppercaseOnly(defaults: defaults) ? name.uppercased() : name
    }
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

    /// The LED's stored footprint id; for a header, the id of the part's
    /// choice whose package is the stored footprint type.
    static func footprintID(for part: CatalogPart, defaults: UserDefaults = .standard) -> String? {
        if part.kind == PART_HEADER {
            guard let type = defaults.string(forKey: PreferenceKey.headerFootprintType), !type.isEmpty
            else { return nil }
            return part.footprintChoices.first { $0.package == type }?.footprint.id
        }
        if part.isPullResistor {
            return stored(PreferenceKey.pullResistorFootprint, defaults: defaults)
        }
        guard part.name == genericLED else { return nil }
        return stored(PreferenceKey.genericLEDFootprint, defaults: defaults)
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

/// A page of the Settings window, in sidebar order.
enum PreferencesPage: String, CaseIterable, Identifiable {
    case partList, defaults, references, designRules, automation

    var id: String { rawValue }

    var title: String {
        switch self {
        case .partList: "Part List"
        case .defaults: "General Defaults"
        case .references: "Reference Defaults"
        case .designRules: "DRC Defaults"
        case .automation: "AI Agents"
        }
    }

    var systemImage: String {
        switch self {
        case .partList: "list.bullet"
        case .defaults: "slider.horizontal.3"
        case .references: "number"
        case .designRules: "checkmark.seal"
        case .automation: "sparkles"
        }
    }
}

/// The Settings window (Schematical ▸ Settings…, ⌘,): a sidebar of pages,
/// like Xcode's, with the picked page beside it under its title.
struct PreferencesView: View {
    @AppStorage("preferencesPage") private var page = PreferencesPage.partList

    var body: some View {
        NavigationSplitView {
            List(PreferencesPage.allCases, selection: Binding(
                get: { page },
                // Clicking empty sidebar space would clear it; keep the page.
                set: { if let picked = $0 { page = picked } }
            )) { page in
                Label(page.title, systemImage: page.systemImage).tag(page)
            }
            .navigationSplitViewColumnWidth(200)
            .toolbar(removing: .sidebarToggle)
        } detail: {
            Group {
                switch page {
                case .partList: PartListPreferencesView()
                case .defaults: DefaultsPreferencesView()
                case .references: ReferencePreferencesView()
                case .designRules: DesignRulePreferencesView()
                case .automation: AutomationPreferencesView()
                }
            }
            .navigationTitle(page.title)
        }
        .frame(width: 720, height: 560)
    }
}

/// Settings ▸ Part List: which parts the parts drawer includes.
private struct PartListPreferencesView: View {
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
    }
}

/// Settings ▸ General Defaults: the options a newly placed part starts with.
private struct DefaultsPreferencesView: View {
    @AppStorage(PreferenceKey.genericLEDColor) private var ledColor = ""
    @AppStorage(PreferenceKey.genericLEDFootprint) private var ledFootprint = ""
    @AppStorage(PreferenceKey.headerRows) private var headerRows = 0
    @AppStorage(PreferenceKey.headerPinsPerRow) private var headerPinsPerRow = 0
    @AppStorage(PreferenceKey.headerGender) private var headerGender = ""
    @AppStorage(PreferenceKey.headerFootprintType) private var headerFootprintType = ""
    @AppStorage(PreferenceKey.pullResistorOhms) private var pullOhms = 0.0
    @AppStorage(PreferenceKey.pullResistorFootprint) private var pullFootprint = ""
    @AppStorage(PreferenceKey.uppercaseNames) private var uppercaseNames = true
    @AppStorage(PreferenceKey.playBuzzerSounds) private var playBuzzerSounds = true
    /// The app-wide catalog, loaded once at launch.
    private let catalog = PartCatalogStore.shared

    var body: some View {
        Form {
            Section {
                Toggle("Play buzzer sounds", isOn: $playBuzzerSounds)
            } header: {
                Text("Simulation")
            } footer: {
                Text("While a buzzer is on in play mode, the Mac's speakers play its tone: a buzzer with a built-in driver at its rated frequency, a transducer at the frequency its + pin is driven at.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Section {
                Toggle("Uppercase only", isOn: $uppercaseNames)
            } header: {
                Text("Net Label and Bus Wire Names")
            } footer: {
                Text("Names are case sensitive: CLK and clk are different nets. While this is on, letters typed into a net label or bus wire name become uppercase. Names already in a schematic are left as they are.")
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
    }
}

/// Settings ▸ AI Agents: turns on the server that lets an AI agent drive
/// the running app (`AutomationServer`).
private struct AutomationPreferencesView: View {
    @AppStorage(PreferenceKey.automationEnabled) private var enabled = false
    @AppStorage(PreferenceKey.automationToken) private var token = ""
    private let server = AutomationServer.shared
    private let folder = AutomationFolder.shared

    var body: some View {
        Form {
            Section {
                Toggle("Allow AI agents to control Schematical", isOn: $enabled)
            } footer: {
                Text("Lets an AI agent that speaks MCP, such as Claude Code, work in the open windows: search parts, place them, draw wires, lay out and route the board, run the design rule check, run the simulation, and see pictures of the schematic and board. Its edits happen live and can be undone. The server only accepts connections from this Mac that carry the key below, and only while this is on.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            if enabled {
                Section {
                    LabeledContent("Status") {
                        Text(statusText).foregroundStyle(.secondary)
                    }
                    LabeledContent("Address") {
                        Text(AutomationServer.url).textSelection(.enabled)
                    }
                    LabeledContent("Key") {
                        HStack {
                            Text(token.isEmpty ? AutomationServer.token : token)
                                .textSelection(.enabled).lineLimit(1).truncationMode(.middle)
                            Button("New Key") { token = AutomationServer.regenerateToken() }
                        }
                    }
                    LabeledContent("Claude Code") {
                        VStack(alignment: .trailing, spacing: 6) {
                            Text(AutomationServer.addCommand).textSelection(.enabled).font(.caption.monospaced())
                            Button("Copy Command") {
                                NSPasteboard.general.clearContents()
                                NSPasteboard.general.setString(AutomationServer.addCommand, forType: .string)
                            }
                        }
                    }
                } header: {
                    Text("Connecting")
                } footer: {
                    Text("Run the Claude Code command once in a terminal. Other agents can use the address as a Streamable HTTP MCP server and send the key as “Authorization: Bearer …”. A new key disconnects agents using the old one.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Section {
                    LabeledContent("Folder") {
                        HStack {
                            Text(folder.url?.path ?? "None chosen")
                                .foregroundStyle(folder.url == nil ? .secondary : .primary)
                                .lineLimit(1).truncationMode(.middle)
                            Button("Choose…") { folder.choose() }
                            if folder.url != nil { Button("Clear") { folder.clear() } }
                        }
                    }
                } header: {
                    Text("Files")
                } footer: {
                    Text("Agents can open, save and export (Gerber) files only inside this folder. With none chosen, those tools are off; everything else works.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
        .formStyle(.grouped)
        .onChange(of: enabled) { _, value in
            AutomationServer.shared.setEnabled(value)
        }
        .onAppear {
            if token.isEmpty { token = AutomationServer.token }
            folder.load()
        }
    }

    private var statusText: String {
        guard server.isListening else { return "Starting…" }
        guard let last = server.lastRequest else { return "Listening; no agent has connected yet" }
        let name = server.clientName ?? "An agent"
        let ago = Int(Date().timeIntervalSince(last))
        return "\(name) connected; last request \(ago < 5 ? "just now" : "\(ago) s ago"); \(server.callCount) tool calls"
    }
}
