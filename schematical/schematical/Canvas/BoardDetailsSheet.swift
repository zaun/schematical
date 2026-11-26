import SwiftUI

/// The Board Details sheet, opened from the PCB sidebar's board panel: the
/// board settings that are set once rather than while laying out, a tab
/// each. Stackup, Planes, Nets, Net Classes and Net Groups are always there;
/// Card comes first, only for a board type with an edge connector to size or
/// options to choose. Every edit is an undo step, as it was in the sidebar; Done closes.
struct BoardDetailsSheet: View {
    @Bindable var document: SchematicDocument
    let highlightedNet: Binding<PlaneNet?>
    /// Routes a net from its row (Route Net). The sheet closes first.
    var onRoute: (RoutingOptions.Scope) -> Void
    var onClose: () -> Void

    enum Tab: String, CaseIterable, Identifiable {
        case stackup, planes, nets, classes, groups, card

        var id: Self { self }

        var title: String {
            switch self {
            case .stackup: "Stackup"
            case .planes: "Planes"
            case .nets: "Nets"
            case .classes: "Net Classes"
            case .groups: "Net Groups"
            case .card: "Card"
            }
        }
    }

    /// The tabs `board` has, in order: Card first, and only for a custom
    /// card or a board type with options.
    static func tabs(for board: BoardSettings) -> [Tab] {
        let rest = Tab.allCases.filter { $0 != .card }
        return board.type == .customCard || board.type.hasOptions ? [.card] + rest : rest
    }

    /// The tab chosen; nil, as when the sheet opens, shows the first.
    @State var tab: Tab?
    /// The net list the three net tabs share, rebuilt shortly after the
    /// board or the schematic changes rather than on every redraw.
    @State private var rows: [NetRow]
    @State private var hasBuilt: Bool

    /// Opens on `tab` (the first tab when nil), with the net list not yet built.
    init(document: SchematicDocument, highlightedNet: Binding<PlaneNet?>,
         onRoute: @escaping (RoutingOptions.Scope) -> Void = { _ in }, onClose: @escaping () -> Void, tab: Tab? = nil) {
        self.document = document
        self.highlightedNet = highlightedNet
        self.onRoute = onRoute
        self.onClose = onClose
        _tab = State(initialValue: tab)
        _rows = State(initialValue: [])
        _hasBuilt = State(initialValue: false)
    }

    var body: some View {
        let tabs = Self.tabs(for: document.board)
        let shown = tab.flatMap { tabs.contains($0) ? $0 : nil } ?? tabs[0]
        VStack(spacing: 0) {
            Picker("Section", selection: Binding(get: { shown }, set: { tab = $0 })) {
                ForEach(tabs) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()
            .padding([.horizontal, .top])
            .padding(.bottom, 8)
            Form {
                content(shown)
            }
            .formStyle(.grouped)
            Divider()
            HStack {
                Text(Self.summary(of: document.board))
                    .foregroundStyle(.secondary)
                    .font(.callout)
                Spacer()
                Button("Done") { onClose() }
                    .keyboardShortcut(.defaultAction)
            }
            .padding()
        }
        .frame(width: 640, height: 620)
        .task(id: NetListInput(document)) {
            // The first build is at once; later ones wait out a burst of edits.
            if hasBuilt { try? await Task.sleep(for: .milliseconds(150)) }
            guard !Task.isCancelled else { return }
            rows = document.netRows()
            hasBuilt = true
        }
    }

    /// "Custom · 100 × 80 mm · 4 layers", under the tabs' content.
    static func summary(of board: BoardSettings) -> String {
        let size = LengthField.format(board.width, units: board.units) + " × "
            + LengthField.format(board.height, units: board.units) + " " + board.units.title
        return "\(board.type.title) · \(size) · \(board.layers) \(board.layers == 1 ? "layer" : "layers")"
    }

    @ViewBuilder
    private func content(_ tab: Tab) -> some View {
        switch tab {
        case .stackup:
            StackupSection(document: document)
        case .planes:
            if PlanesSection.planes(of: document.board).isEmpty {
                Section("Planes") {
                    Text(document.board.layers > 2
                         ? "No Plane layers. Set an inner layer's type to Plane in the sidebar's Layers."
                         : "No Plane layers. A Plane is an inner layer: a board with 4 or more layers has them.")
                        .foregroundStyle(.secondary)
                }
            } else {
                PlanesSection(document: document)
            }
        case .nets:
            NetsSection(document: document, rows: rows, hasBuilt: hasBuilt, highlightedNet: highlightedNet) { scope in
                onClose()
                onRoute(scope)
            }
        case .classes:
            NetClassesSection(document: document, rows: rows)
        case .groups:
            NetGroupsSection(document: document, rows: rows)
        case .card:
            if document.board.type == .customCard {
                CustomCardSection(document: document)
            }
            if document.board.type.hasOptions {
                CardOptionsSection(type: document.board.type, options: { document.board.cardOptions },
                                   set: { document.setCardOptions($0) })
            }
        }
    }
}
