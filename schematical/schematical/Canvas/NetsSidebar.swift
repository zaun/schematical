import SwiftUI

// MARK: - Nets

/// Every net on the board (`SchematicDocument.netRows`): its name, how many
/// pads it has and, in red, how many of its connections are unrouted; a
/// menu for its class and one for its own layer rule. A click on a net's
/// name highlights its copper on the canvas; a second click, or Esc there,
/// clears it. The list is rebuilt shortly after the board or schematic
/// changes, not on every redraw.
struct NetsSection: View {
    let document: SchematicDocument
    let highlightedNet: Binding<PlaneNet?>

    @State private var rows: [NetRow] = []
    @State private var filter = ""
    @State private var hasBuilt = false

    var body: some View {
        Section("Nets") {
            TextField("Filter", text: $filter, prompt: Text("Filter by net or pad"))
                .labelsHidden()
            let shown = rows.filter { $0.matches(filter) }
            if rows.isEmpty {
                Text(hasBuilt ? "No nets on the board" : "…").foregroundStyle(.secondary)
            } else if shown.isEmpty {
                Text("No net matches").foregroundStyle(.secondary)
            }
            ForEach(shown) { row in
                NetRowView(document: document, row: row, highlightedNet: highlightedNet)
            }
        }
        .task(id: NetListInput(document)) {
            // The first build is at once; later ones wait out a burst of edits.
            if hasBuilt { try? await Task.sleep(for: .milliseconds(150)) }
            guard !Task.isCancelled else { return }
            rows = document.netRows()
            hasBuilt = true
        }
    }
}

/// One net: name (a click highlights it), pads, unrouted count, class and
/// layer rule menus.
private struct NetRowView: View {
    let document: SchematicDocument
    let row: NetRow
    let highlightedNet: Binding<PlaneNet?>

    var body: some View {
        let isHighlighted = highlightedNet.wrappedValue == row.key
        HStack(spacing: 6) {
            Button {
                highlightedNet.wrappedValue = isHighlighted ? nil : row.key
            } label: {
                Text(row.name)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .fontWeight(isHighlighted ? .semibold : .regular)
                    .foregroundStyle(isHighlighted ? Color.accentColor : Color.primary)
            }
            .buttonStyle(.plain)
            .help(isHighlighted ? "Stop highlighting \(row.name)" : "Highlight \(row.name) on the board")
            Text("\(row.pads)")
                .foregroundStyle(.secondary)
                .monospacedDigit()
                .help("\(row.pads) \(row.pads == 1 ? "pad" : "pads")")
            if row.unrouted > 0 {
                Text("\(row.unrouted)")
                    .foregroundStyle(.red)
                    .monospacedDigit()
                    .help("\(row.unrouted) unrouted \(row.unrouted == 1 ? "connection" : "connections")")
            }
            Spacer(minLength: 4)
            NetClassMenu(document: document, key: row.key)
            NetLayerRuleMenu(document: document, key: row.key)
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

/// A net's own layer rule: Any (its class's, if it has one), or Only on /
/// Prefer a set of the board's trace layers, one toggle per layer.
struct NetLayerRuleMenu: View {
    let document: SchematicDocument
    let key: PlaneNet

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
        let rule = document.board.netOverrides.first { $0.net == key }?.layerRule
        let layers = document.board.traceLayers
        Menu(Self.title(rule)) {
            Button("Any Layer") { document.setNetLayerRule(key, nil) }
            ForEach([LayerRuleMode.only, .prefer], id: \.self) { mode in
                Section(mode.title) {
                    ForEach(layers, id: \.self) { layer in
                        Toggle(layer.title, isOn: Binding(
                            get: { rule?.mode == mode && rule?.layers.contains(layer) == true },
                            set: { _ in document.setNetLayerRule(key, Self.toggled(rule, layer, mode: mode)) }
                        ))
                    }
                }
            }
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
        .help("The net's own layer rule, before its class's: Only on some layers, or Prefer them")
    }
}
