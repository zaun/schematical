import SwiftUI

/// The standard copper weights: 1 oz of copper is 35 µm thick.
nonisolated enum CopperWeight: CaseIterable, Hashable, Sendable {
    case half, one, two

    /// mm.
    var thickness: Double {
        switch self {
        case .half: 0.0175
        case .one: 0.035
        case .two: 0.07
        }
    }

    var title: String {
        switch self {
        case .half: "½ oz"
        case .one: "1 oz"
        case .two: "2 oz"
        }
    }

    /// The weight whose thickness `thickness` (mm) is, within 0.1 µm; nil
    /// for any other thickness (Custom).
    init?(thickness: Double) {
        guard let weight = Self.allCases.first(where: { abs($0.thickness - thickness) < 0.0001 }) else { return nil }
        self = weight
    }
}

/// One row of the stackup table: a copper layer or the dielectric below it.
nonisolated struct StackupRow: Equatable, Sendable {
    enum Kind: Equatable, Sendable {
        case copper(Int)
        case dielectric(Int)
    }

    var kind: Kind
    var title: String
}

nonisolated enum StackupRows {
    /// The rows top to bottom: each copper layer, then the dielectric below
    /// it (a 1-layer board's one dielectric is its substrate).
    static func rows(for stackup: BoardStackup, layers: [CopperLayer]) -> [StackupRow] {
        var rows: [StackupRow] = []
        for (index, layer) in layers.enumerated() where stackup.copper.indices.contains(index) {
            rows.append(StackupRow(kind: .copper(index), title: "\(layer.title) copper"))
            if stackup.dielectrics.indices.contains(index) {
                let title = layers.count == 1 ? "Substrate"
                    : "\(layer.title) to \(layers[min(index + 1, layers.count - 1)].title)"
                rows.append(StackupRow(kind: .dielectric(index), title: title))
            }
        }
        return rows
    }
}

/// Board Details' Stackup tab: a Surface section with the finish and the
/// temperature rise for the current estimate, then Layers: the copper weight
/// of each layer and the thickness, kind and Er of each dielectric. Every edit is an undo step.
struct StackupSection: View {
    @Bindable var document: SchematicDocument

    var body: some View {
        let stackup = document.board.stackup
        let units = document.board.units
        Section("Surface") {
            Picker("Finish", selection: Binding(get: { stackup.finish }, set: { document.setStackupFinish($0) })) {
                ForEach(SurfaceFinish.allCases) { Text($0.title).tag($0) }
            }
            .pickerStyle(.menu)
            .help("The finish on bare copper. Board Preview and the renders draw it")
            LabeledContent("Temperature rise") {
                HStack(spacing: 4) {
                    ValueField(title: "Temperature rise", value: stackup.temperatureRise, places: 0) {
                        document.setStackupTemperatureRise($0)
                        return document.board.stackup.temperatureRise
                    }
                    Text("°C").foregroundStyle(.secondary)
                }
            }
            .help("How far a trace may heat above the board, for the current estimate (1 to 100 °C)")
        }
        Section("Layers") {
            ForEach(StackupRows.rows(for: stackup, layers: document.board.copperLayers), id: \.title) { row in
                switch row.kind {
                case let .copper(index): copperRow(row.title, index: index, stackup: stackup)
                case let .dielectric(index): dielectricRow(row.title, index: index, stackup: stackup, units: units)
                }
            }
            LabeledContent("Thickness") {
                Text(LengthField.format(stackup.thickness, units: units, precise: true) + " " + units.title)
            }
            .help("Copper, dielectrics and the solder mask on each side")
            Button("Reset to Standard") { document.resetStackup() }
                .help("The standard stackup for \(document.board.layers) \(document.board.layers == 1 ? "layer" : "layers")")
        }
    }

    /// The rows' columns, so copper and dielectric rows line up: the weight
    /// or kind menu, the thickness, its unit, the Er and its label.
    private static let kindWidth: CGFloat = 110
    private static let thicknessWidth: CGFloat = 72
    private static let unitWidth: CGFloat = 26
    private static let erWidth: CGFloat = 44

    private func copperRow(_ title: String, index: Int, stackup: BoardStackup) -> some View {
        let thickness = stackup.copper[index]
        return LabeledContent(title) {
            HStack(spacing: 4) {
                Picker(title, selection: Binding<CopperWeight?>(
                    get: { CopperWeight(thickness: thickness) },
                    set: { weight in
                        if let weight { document.setCopperThickness(weight.thickness, at: index) }
                    }
                )) {
                    ForEach(CopperWeight.allCases, id: \.self) { Text($0.title).tag(Optional($0)) }
                    Text("Custom").tag(CopperWeight?.none)
                }
                .labelsHidden()
                .pickerStyle(.menu)
                .frame(width: Self.kindWidth, alignment: .trailing)
                ValueField(title: "\(title) thickness", value: thickness * 1000, places: 1) {
                    document.setCopperThickness($0 / 1000, at: index)
                    return document.board.stackup.copper[index] * 1000
                }
                .frame(width: Self.thicknessWidth)
                Text("µm").foregroundStyle(.secondary)
                    .frame(width: Self.unitWidth, alignment: .leading)
                // Under the dielectric rows' Er column.
                Color.clear.frame(width: Self.erWidth + Self.unitWidth + 4, height: 1)
            }
        }
        .help("1 oz of copper is 35 µm")
    }

    private func dielectricRow(_ title: String, index: Int, stackup: BoardStackup, units: BoardUnits) -> some View {
        let dielectric = stackup.dielectrics[index]
        return LabeledContent(title) {
            HStack(spacing: 4) {
                Picker("\(title) kind", selection: Binding(
                    get: { dielectric.kind },
                    set: { document.setDielectric(at: index, kind: $0) }
                )) {
                    ForEach(DielectricKind.allCases, id: \.self) { Text($0.title).tag($0) }
                }
                .labelsHidden()
                .pickerStyle(.menu)
                .frame(width: Self.kindWidth, alignment: .trailing)
                LengthField(title: "\(title) thickness", millimetres: dielectric.thickness, units: units,
                            precise: true) {
                    document.setDielectric(at: index, thickness: $0)
                    return document.board.stackup.dielectrics[index].thickness
                }
                .labelsHidden()
                .frame(width: Self.thicknessWidth)
                Text(units.title).foregroundStyle(.secondary)
                    .frame(width: Self.unitWidth, alignment: .leading)
                ValueField(title: "\(title) Er", value: dielectric.er, places: 2) {
                    document.setDielectric(at: index, er: $0)
                    return document.board.stackup.dielectrics[index].er
                }
                .frame(width: Self.erWidth)
                Text("Er").foregroundStyle(.secondary)
                    .frame(width: Self.unitWidth, alignment: .leading)
            }
        }
        .help("The insulator between these two layers: its thickness and relative permittivity (Er)")
    }
}

/// A number field: shows `value` to at most `places` decimals, and on commit
/// (Return or leaving the field; each keystroke when `live`) hands the typed number to `commit`, which
/// returns the value that was stored (so a clamp shows).
struct ValueField: View {
    let title: String
    let value: Double
    let places: Int
    /// Hands each typed number to `commit` as it is typed, not only on
    /// Return or leaving the field: for a field a result is worked out
    /// from, where a button's click doesn't take the focus away.
    var live = false
    let commit: (Double) -> Double

    @State private var text = ""
    @State private var shown = ""
    @FocusState private var isFocused: Bool

    var body: some View {
        TextField(title, text: $text)
            .labelsHidden()
            .focused($isFocused)
            .multilineTextAlignment(.trailing)
            .onSubmit(apply)
            .onChange(of: isFocused) { _, focused in
                if !focused { apply() }
            }
            .onChange(of: value) { refresh() }
            .onChange(of: text) { typed() }
            .onAppear { refresh() }
    }

    private func refresh() {
        if !isFocused || text == shown { show(value) }
    }

    /// A live field stores what is typed so far; the text stays as typed
    /// until Return or leaving the field shows what was stored.
    private func typed() {
        guard live, text != shown,
              let number = Double(text.trimmingCharacters(in: .whitespaces)), number.isFinite else { return }
        _ = commit(number)
    }

    private func show(_ number: Double) {
        shown = Self.format(number, places: places)
        text = shown
    }

    private func apply() {
        guard text != shown else { return }
        guard let number = Double(text.trimmingCharacters(in: .whitespaces)), number.isFinite else {
            show(value)
            return
        }
        show(commit(number))
    }

    /// `number` to `places` decimals, trailing zeros trimmed.
    static func format(_ number: Double, places: Int) -> String {
        var string = String(format: "%.\(places)f", number)
        while string.contains("."), string.hasSuffix("0") { string.removeLast() }
        if string.hasSuffix(".") { string.removeLast() }
        return string == "-0" ? "0" : string
    }
}
