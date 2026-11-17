import SwiftUI

// MARK: - Nets, Net Classes, Net Groups

// Board Details' three net tabs (`BoardDetailsSheet`), which share one net
// list (`SchematicDocument.netRows`).

/// Every net on the board, a row of four columns under a heading row: its
/// name, how many of its connections are unrouted ("2 of 5 unrouted", red
/// while any is), a menu for its class and one for its own layer rule. A click on a net's name highlights its copper on
/// the canvas; a second click, or Esc there, clears it.
struct NetsSection: View {
    let document: SchematicDocument
    let rows: [NetRow]
    let hasBuilt: Bool
    let highlightedNet: Binding<PlaneNet?>
    var onRoute: (RoutingOptions.Scope) -> Void = { _ in }

    @State private var filter = ""

    /// What a row's Route Net routes.
    static func routeScope(for row: NetRow) -> RoutingOptions.Scope { .nets([row.key]) }

    var body: some View {
        Section("Nets") {
            TextField("Filter", text: $filter, prompt: Text("Filter by net or pad"))
                .labelsHidden()
            let shown = rows.filter { $0.matches(filter) }
            if !shown.isEmpty {
                HStack(spacing: NetRowView.spacing) {
                    Text("Net").frame(maxWidth: .infinity, alignment: .leading)
                    Text("Connections").frame(width: NetRowView.statusWidth, alignment: .leading)
                    Text("Class").frame(width: NetRowView.classWidth, alignment: .leading)
                    Text("Layers").frame(width: NetRowView.layersWidth, alignment: .leading)
                }
                .font(.callout)
                .foregroundStyle(.secondary)
            }
            if rows.isEmpty {
                Text(hasBuilt ? "No nets on the board" : "…").foregroundStyle(.secondary)
            } else if shown.isEmpty {
                Text("No net matches").foregroundStyle(.secondary)
            }
            ForEach(shown) { row in
                NetRowView(document: document, row: row, highlightedNet: highlightedNet)
                    .contextMenu {
                        Button("Route Net") { onRoute(Self.routeScope(for: row)) }
                            .disabled(row.unrouted == 0 || document.isPlaying)
                        Button("Unroute Net") { document.unroute(nets: [row.net], onlyAuto: false) }
                        Button(highlightedNet.wrappedValue == row.key ? "Stop Highlighting" : "Highlight") {
                            highlightedNet.wrappedValue = highlightedNet.wrappedValue == row.key ? nil : row.key
                        }
                    }
            }
        }
    }
}

/// One net: name (a click highlights it), its unrouted connections, class
/// and layer rule menus, each in its column.
struct NetRowView: View {
    let document: SchematicDocument
    let row: NetRow
    let highlightedNet: Binding<PlaneNet?>

    static let spacing: CGFloat = 12
    static let statusWidth: CGFloat = 120
    static let classWidth: CGFloat = 120
    static let layersWidth: CGFloat = 150

    /// "2 of 5 unrouted": the connections still to route of the net's
    /// connections (its pads less one); "—" for a net with one pad.
    static func status(pads: Int, unrouted: Int) -> String {
        let total = max(pads - 1, unrouted)
        return total == 0 ? "—" : "\(unrouted) of \(total) unrouted"
    }

    var body: some View {
        let isHighlighted = highlightedNet.wrappedValue == row.key
        let key = row.key
        HStack(spacing: Self.spacing) {
            Button {
                highlightedNet.wrappedValue = isHighlighted ? nil : key
            } label: {
                Text(row.name)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .fontWeight(isHighlighted ? .semibold : .regular)
                    .foregroundStyle(isHighlighted ? Color.accentColor : Color.primary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
            .help(isHighlighted ? "Stop highlighting \(row.name)" : "Highlight \(row.name) on the board")
            Text(Self.status(pads: row.pads, unrouted: row.unrouted))
                .foregroundStyle(row.unrouted > 0 ? Color.red : Color.secondary)
                .monospacedDigit()
                .lineLimit(1)
                .frame(width: Self.statusWidth, alignment: .leading)
                .help("\(row.pads) \(row.pads == 1 ? "pad" : "pads"); \(row.unrouted) unrouted "
                      + "\(row.unrouted == 1 ? "connection" : "connections")")
            NetClassMenu(document: document, key: key)
                .frame(width: Self.classWidth, alignment: .leading)
            NetLayerRuleMenu(rule: document.board.netOverrides.first { $0.net == key }?.layerRule,
                             layers: document.board.traceLayers,
                             help: "The net's own layer rule, before its class's: Only on some layers, or Prefer them") {
                document.setNetLayerRule(key, $0)
            }
            .frame(width: Self.layersWidth, alignment: .leading)
        }
    }
}

/// A net's class: Default or one of the board's classes.
private struct NetClassMenu: View {
    let document: SchematicDocument
    let key: PlaneNet

    var body: some View {
        let classes = document.board.netClasses
        let current = classes.first { $0.members.contains(key) }?.id
        Picker("Class", selection: Binding(get: { current }, set: { document.assignNet(key, to: $0) })) {
            Text("Default").tag(UUID?.none)
            ForEach(classes) { netClass in
                Text(netClass.name).tag(Optional(netClass.id))
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
        .fixedSize()
        .help("The net's class: its trace width, clearance and via")
    }
}

/// A layer rule as a menu: Any, or Only on / Prefer a set of the board's
/// trace layers, one toggle per layer.
struct NetLayerRuleMenu: View {
    let rule: LayerRule?
    let layers: [CopperLayer]
    var help = ""
    let set: (LayerRule?) -> Void

    static func title(_ rule: LayerRule?) -> String {
        guard let rule else { return "Any" }
        return (rule.mode == .only ? "Only " : "Prefer ") + rule.layers.map(\.title).joined(separator: ", ")
    }

    /// `rule` with `layer` switched in mode `mode`: on in a rule of another
    /// mode starts a new rule of just that layer; off on the last layer is
    /// no rule.
    static func toggled(_ rule: LayerRule?, _ layer: CopperLayer, mode: LayerRuleMode) -> LayerRule? {
        guard var rule, rule.mode == mode else { return LayerRule(mode: mode, layers: [layer]) }
        if rule.layers.contains(layer) {
            rule.layers.removeAll { $0 == layer }
            return rule.layers.isEmpty ? nil : rule
        }
        rule.layers.append(layer)
        return rule
    }

    var body: some View {
        Menu(Self.title(rule)) {
            Button("Any Layer") { set(nil) }
            ForEach([LayerRuleMode.only, .prefer], id: \.self) { mode in
                Section(mode.title) {
                    ForEach(layers, id: \.self) { layer in
                        Toggle(layer.title, isOn: Binding(
                            get: { rule?.mode == mode && rule?.layers.contains(layer) == true },
                            set: { _ in set(Self.toggled(rule, layer, mode: mode)) }
                        ))
                    }
                }
            }
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
        .help(help)
    }
}

// MARK: Net classes

/// The board's net classes, in priority order (a net is in the first that
/// names it): each one's name, trace width, clearance, via, layer rule and
/// nets. **+** adds one, **−** on a class deletes it. With none, a button
/// makes the Power class from the board's supply rails.
struct NetClassesSection: View {
    let document: SchematicDocument
    let rows: [NetRow]

    var body: some View {
        Section {
            let classes = document.board.netClasses
            if classes.isEmpty {
                Text("Every net is in the Default class: 10 mil, the DRC's min clearance, a 0.4 / 0.8 mm via.")
                    .foregroundStyle(.secondary)
                    .font(.callout)
                Button("Add Power Class") { document.addPowerClass() }
                    .help("A 20 mil class holding every supply rail, ground included")
            }
            ForEach(Array(classes.enumerated()), id: \.element.id) { index, netClass in
                NetClassEditor(document: document, netClass: netClass, index: index, count: classes.count, rows: rows)
            }
        } header: {
            HStack {
                Text("Net Classes")
                Spacer()
                Button {
                    document.addNetClass(named: "Class \(document.board.netClasses.count + 1)")
                } label: {
                    Image(systemName: "plus")
                }
                .buttonStyle(.borderless)
                .help("Add a net class")
            }
        }
    }
}

/// One class's fields, in a disclosure group named by the class.
private struct NetClassEditor: View {
    let document: SchematicDocument
    let netClass: NetClass
    let index: Int
    let count: Int
    let rows: [NetRow]

    @State private var name = ""
    @State private var isOpen = false
    @State private var calculating = false

    /// The layer the class's estimates are for: the first its layer rule allows, else the board's first trace layer.
    private var estimateLayer: CopperLayer {
        let traceLayers = document.board.traceLayers
        return netClass.layerRule?.layers.first { traceLayers.contains($0) } ?? traceLayers.first ?? .top
    }

    var body: some View {
        let id = netClass.id
        let units = document.board.units
        let rules = document.board.rules
        DisclosureGroup(isExpanded: $isOpen) {
            TextField("Name", text: $name)
                .onSubmit { document.renameNetClass(id, to: name); name = current.name }
            LabeledContent("Width") {
                HStack(spacing: 4) {
                    Text("\(netClass.width) mil").monospacedDigit()
                    Stepper("Width", value: Binding(get: { netClass.width }, set: { document.setNetClassWidth(id, $0) }),
                            in: BoardTrace.widthRange, step: 1)
                        .labelsHidden()
                }
            }
            ForEach(EstimateText.rows(board: document.board, layer: estimateLayer, widthMil: netClass.width,
                                      pairGapMM: nil, suffix: " (\(estimateLayer.title))")) { row in
                LabeledContent(row.title, value: row.value).help(row.help)
            }
            Button("Calculate Width…") { calculating = true }
                .help("The width that gives a target impedance or carries a current")
                .sheet(isPresented: $calculating) {
                    WidthCalculatorSheet(document: document, layers: document.board.traceLayers, layer: estimateLayer,
                                         gapMM: NetGroup.defaultGap) { document.setNetClassWidth(id, $0) }
                }
            Toggle("DRC Min Clearance", isOn: Binding(
                get: { netClass.clearance == nil },
                set: { document.setNetClassClearance(id, $0 ? nil : rules.minClearance) }
            ))
            .help("Keep the DRC's min clearance (\(LengthField.format(rules.minClearance, units: units, precise: true)) \(units.title)), or set the class's own")
            if netClass.clearance != nil {
                LengthField(title: "Clearance", millimetres: netClass.clearance(rules: rules), units: units,
                            precise: true) {
                    document.setNetClassClearance(id, $0)
                    return current.clearance(rules: rules)
                }
            }
            LengthField(title: "Via Hole", millimetres: netClass.viaDrill, units: units, precise: true) {
                document.setNetClassVia(id, drill: $0, diameter: current.viaDiameter)
                return current.viaDrill
            }
            LengthField(title: "Via Metal", millimetres: netClass.viaDiameter, units: units, precise: true) {
                document.setNetClassVia(id, drill: current.viaDrill, diameter: $0)
                return current.viaDiameter
            }
            LabeledContent("Layers") {
                NetLayerRuleMenu(rule: netClass.layerRule, layers: document.board.traceLayers,
                                 help: "The class's layer rule; a net's own rule comes first") {
                    document.setNetClassLayerRule(id, $0)
                }
            }
            NetMembersMenu(members: netClass.members, rows: rows) { document.assignNet($0, to: nil) }
            HStack {
                Button("Move Up") { document.moveNetClass(id, to: index - 1) }
                    .disabled(index == 0)
                Button("Move Down") { document.moveNetClass(id, to: index + 1) }
                    .disabled(index == count - 1)
                Spacer()
                Button("Delete", role: .destructive) { document.removeNetClass(id) }
            }
            .buttonStyle(.borderless)
            .help("A net is in the first class, in this order, that names it")
        } label: {
            HStack {
                Text(netClass.name)
                Spacer()
                Text("\(netClass.width) mil · \(netClass.members.count) \(netClass.members.count == 1 ? "net" : "nets")")
                    .foregroundStyle(.secondary)
            }
        }
        .onAppear { name = netClass.name }
        .onChange(of: netClass.name) { _, value in name = value }
    }

    private var current: NetClass {
        document.board.netClasses.first { $0.id == netClass.id } ?? netClass
    }
}

/// "Nets: N", a menu of the members by name; choosing one takes it out.
private struct NetMembersMenu: View {
    let members: [PlaneNet]
    let rows: [NetRow]
    let remove: (PlaneNet) -> Void

    var body: some View {
        LabeledContent("Nets") {
            if members.isEmpty {
                Text("None").foregroundStyle(.secondary)
            } else {
                Menu("\(members.count)") {
                    Section("Choose to remove") {
                        ForEach(members, id: \.self) { member in
                            Button(rows.first { $0.key == member }?.name ?? "Missing net") { remove(member) }
                        }
                    }
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
            }
        }
    }
}

// MARK: Net groups

/// The board's net groups: equal-length groups and differential pairs.
/// **+** adds one of either; each has a name, its mode, its sizes and its
/// nets. A net is in at most one group; a pair takes 2.
struct NetGroupsSection: View {
    let document: SchematicDocument
    let rows: [NetRow]

    var body: some View {
        Section {
            let groups = document.board.netGroups
            if groups.isEmpty {
                Text("No groups. A length-match group's nets end the same length; a differential pair's two nets run side by side.")
                    .foregroundStyle(.secondary)
                    .font(.callout)
            }
            ForEach(groups) { group in
                NetGroupEditor(document: document, group: group, rows: rows)
            }
        } header: {
            HStack {
                Text("Net Groups")
                Spacer()
                Menu {
                    Button("Length Match") {
                        document.addNetGroup(named: "Group \(document.board.netGroups.count + 1)", mode: .lengthMatch)
                    }
                    Button("Differential Pair") {
                        document.addNetGroup(named: "Pair \(document.board.netGroups.count + 1)", mode: .diffPair)
                    }
                } label: {
                    Image(systemName: "plus")
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
                .help("Add a net group")
            }
        }
    }
}

/// One group's fields, in a disclosure group named by the group.
private struct NetGroupEditor: View {
    let document: SchematicDocument
    let group: NetGroup
    let rows: [NetRow]

    @State private var name = ""
    @State private var isOpen = false

    var body: some View {
        let id = group.id
        let units = document.board.units
        DisclosureGroup(isExpanded: $isOpen) {
            TextField("Name", text: $name)
                .onSubmit { document.renameNetGroup(id, to: name); name = current.name }
            Picker("Mode", selection: Binding(get: { group.mode }, set: { document.setNetGroupMode(id, $0) })) {
                ForEach(NetGroupMode.allCases, id: \.self) { mode in
                    Text(mode.title).tag(mode)
                }
            }
            LengthField(title: "Tolerance", millimetres: group.tolerance, units: units, precise: true) {
                document.setNetGroupTolerance(id, $0)
                return current.tolerance
            }
            .help("How far apart the nets' lengths may end")
            if group.mode == .diffPair {
                LengthField(title: "Gap", millimetres: group.gap, units: units, precise: true) {
                    document.setNetGroupGap(id, $0)
                    return current.gap
                }
                .help("The edge-to-edge gap between the pair's two traces")
                LengthField(title: "Max Uncoupled", millimetres: group.maxUncoupled, units: units, precise: true) {
                    document.setNetGroupMaxUncoupled(id, $0)
                    return current.maxUncoupled
                }
                .help("How much of the pair may run apart, near its pads")
            }
            LengthField(title: "Max Meander", millimetres: group.maxMeander, units: units, precise: true) {
                document.setNetGroupMaxMeander(id, $0)
                return current.maxMeander
            }
            .help("The largest sideways size one length-tuning bump may reach")
            let longest = group.members.compactMap { member in rows.first { $0.key == member }?.length }.max()
            ForEach(group.members, id: \.self) { member in
                let row = rows.first { $0.key == member }
                HStack {
                    Text(row?.name ?? "Missing net")
                    if let row, row.pads > 2 {
                        Text("multi-drop").foregroundStyle(.orange).font(.caption)
                            .help("More than one connection: only a net with two pads can be length-tuned or paired")
                    }
                    Spacer()
                    NetLengthLabel(length: row?.length, longest: longest, tolerance: group.tolerance, units: units)
                    Button {
                        document.unassignNet(member)
                    } label: {
                        Image(systemName: "minus.circle")
                    }
                    .buttonStyle(.borderless)
                    .help("Take the net out of the group")
                }
            }
            let free = rows.filter { $0.groupID == nil && !group.members.contains($0.key) }
            let isFull = group.mode == .diffPair && group.members.count >= NetGroup.pairSize
            HStack {
                Menu("Add Net") {
                    ForEach(free) { row in
                        Button(row.pads > 2 ? "\(row.name) (multi-drop)" : row.name) {
                            document.assignNet(row.key, toGroup: id)
                        }
                    }
                }
                .disabled(isFull || free.isEmpty)
                .help(isFull ? "A pair has its 2 nets" : "Add a net that is in no other group")
                Spacer()
                Button("Delete", role: .destructive) { document.removeNetGroup(id) }
                    .buttonStyle(.borderless)
            }
        } label: {
            HStack {
                Text(group.name)
                Spacer()
                Text("\(group.mode.title) · \(group.members.count) \(group.members.count == 1 ? "net" : "nets")")
                    .foregroundStyle(.secondary)
            }
        }
        .onAppear { name = group.name }
        .onChange(of: group.name) { _, value in name = value }
    }

    private var current: NetGroup {
        document.board.netGroups.first { $0.id == group.id } ?? group
    }
}

/// A group member's routed length and how far it is from the group's
/// longest; red when beyond the group's tolerance; "—" when it has none (not
/// routed, or more than two pins).
struct NetLengthLabel: View {
    let length: Double?
    let longest: Double?
    let tolerance: Double
    let units: BoardUnits

    static func text(length: Double?, longest: Double?, units: BoardUnits) -> String {
        guard let length else { return "—" }
        let shown = LengthField.format(length, units: units, precise: false) + " " + units.title
        guard let longest, longest - length > 1e-9 else { return shown }
        return shown + " (−" + LengthField.format(longest - length, units: units, precise: false) + ")"
    }

    static func isOver(length: Double?, longest: Double?, tolerance: Double) -> Bool {
        guard let length, let longest else { return false }
        return longest - length > tolerance + 1e-6
    }

    var body: some View {
        Text(Self.text(length: length, longest: longest, units: units))
            .monospacedDigit()
            .foregroundStyle(Self.isOver(length: length, longest: longest, tolerance: tolerance) ? Color.red : .secondary)
            .help("Its routed length, and how much shorter it is than the group's longest")
    }
}
