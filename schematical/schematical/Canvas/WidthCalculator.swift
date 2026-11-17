import SwiftUI

// MARK: - Readouts

/// One line of the estimates a trace or net class shows.
nonisolated struct EstimateRow: Identifiable, Equatable, Sendable {
    var title: String
    var value: String
    var help: String
    var id: String { title }
}

/// The impedance and current readouts of a trace, from the stackup (`StackupAnalysis`): the strings both the
/// Trace panel and the net class rows show.
nonisolated enum EstimateText {
    static let accuracy = "An estimate, about ±10%: closed-form formulas from the stackup, ignoring the solder mask."

    /// The rows for a trace `widthMil` wide on `layer`: Impedance, Differential when `pairGapMM` is a pair's gap
    /// (mm, edge to edge) and Current at the stackup's temperature rise. `suffix` follows each title.
    static func rows(board: BoardSettings, layer: CopperLayer, widthMil: Int, pairGapMM: Double?,
                     suffix: String = "") -> [EstimateRow] {
        let analysis = StackupAnalysis(board)
        let widthMM = Double(widthMil) * 0.0254
        let geometry = analysis.geometry(of: layer)
        let note: String
        if case .microstrip(_, _, true)? = geometry {
            note = " Assumes a solid ground on the other side."
        } else {
            note = ""
        }
        let reason = analysis.unavailableReason(for: layer)
        var rows: [EstimateRow] = []
        let z = analysis.impedance(widthMM: widthMM, on: layer)
        rows.append(EstimateRow(title: "Impedance" + suffix, value: z.map { String(format: "%.1f Ω", $0) } ?? "—",
                                help: z == nil ? reason : accuracy + note))
        if let gap = pairGapMM {
            let d = analysis.differentialImpedance(widthMM: widthMM, gapMM: gap, on: layer)
            rows.append(EstimateRow(title: "Differential" + suffix, value: d.map { String(format: "%.1f Ω", $0) } ?? "—",
                                    help: d == nil ? reason : "\(accuracy) At the pair's gap, \(String(format: "%g", gap)) mm." + note))
        }
        let rise = board.stackup.temperatureRise
        let amps = analysis.currentCapacity(widthMil: Double(widthMil), on: layer)
        rows.append(EstimateRow(title: "Current" + suffix, value: String(format: "%.2f A", amps),
                                help: "IPC-2221 for a \(String(format: "%g", rise)) °C rise (Stackup ▸ Temperature rise). "
                                    + "An estimate; the copper of this layer only."))
        return rows
    }

    /// The gap (mm) of the differential-pair group `net` is in, if it is in one.
    static func pairGapMM(board: BoardSettings, net: PlaneNet?) -> Double? {
        guard let net else { return nil }
        return board.netGroups.first { $0.mode == .diffPair && $0.members.contains(net) }?.gap
    }
}

// MARK: - The calculation

/// What the Calculate Width sheet solves for.
nonisolated enum WidthGoal: Equatable, Sendable {
    case impedance(Double)
    case differential(Double, gapMM: Double)
    case current(Double, rise: Double)
}

nonisolated enum WidthCalculation {
    struct Result: Equatable, Sendable {
        /// The width that meets the goal exactly, mil.
        var exactMil: Double?
        /// A whole mil (traces are `Int` mil): the nearest for an impedance, rounded up for a current.
        var roundedMil: Int?
        /// The impedance (Ω) or current (A) the rounded width gives.
        var achieved: Double?
        var message: String?
        /// Whether the rounded width is within `BoardTrace.widthRange`.
        var canApply: Bool
    }

    static func result(goal: WidthGoal, layer: CopperLayer, board: BoardSettings) -> Result {
        let analysis = StackupAnalysis(board)
        var solution: WidthSolution?
        var exact: Double?
        switch goal {
        case let .impedance(target): solution = analysis.width(forImpedance: target, on: layer)
        case let .differential(target, gap): solution = analysis.width(forDifferential: target, gapMM: gap, on: layer)
        case let .current(amps, rise):
            guard amps > 0, rise > 0 else { return Result(message: "Enter a current and a temperature rise.", canApply: false) }
            exact = analysis.width(forCurrent: amps, rise: rise, on: layer)
        }
        if exact == nil {
            switch solution {
            case nil: return Result(message: analysis.unavailableReason(for: layer), canApply: false)
            case .tooLow?: return Result(message: "No width gives an impedance that low.", canApply: false)
            case .tooHigh?: return Result(message: "No width gives an impedance that high.", canApply: false)
            case let .mil(value)?: exact = value
            }
        }
        guard let exact, exact.isFinite else { return Result(message: "No width found.", canApply: false) }
        let isCurrent: Bool
        if case .current = goal { isCurrent = true } else { isCurrent = false }
        let rounded = Int(isCurrent ? exact.rounded(.up) : exact.rounded())
        var achieved: Double?
        switch goal {
        case .impedance: achieved = analysis.impedance(widthMM: Double(rounded) * 0.0254, on: layer)
        case let .differential(_, gap):
            achieved = analysis.differentialImpedance(widthMM: Double(rounded) * 0.0254, gapMM: gap, on: layer)
        case let .current(_, rise):
            achieved = StackupAnalysis.currentCapacity(widthMil: Double(rounded),
                                                       thicknessMM: analysis.copperThickness(of: layer), rise: rise,
                                                       external: StackupAnalysis.isExternal(layer))
        }
        let range = BoardTrace.widthRange
        let fits = range.contains(rounded)
        let message = fits ? nil : "Needs \(rounded) mil; traces are \(range.lowerBound) to \(range.upperBound) mil."
        return Result(exactMil: exact, roundedMil: rounded, achieved: achieved, message: message, canApply: fits)
    }
}

// MARK: - The sheet

/// Calculate Width…: a target (impedance, differential impedance or current) and the width that meets it on a
/// layer; Apply hands the whole-mil width to `apply`.
struct WidthCalculatorSheet: View {
    enum Kind: String, CaseIterable, Identifiable {
        case impedance, differential, current

        var id: String { rawValue }
        var title: String {
            switch self {
            case .impedance: "Impedance"
            case .differential: "Differential Impedance"
            case .current: "Current"
            }
        }
    }

    let document: SchematicDocument
    let layers: [CopperLayer]
    let apply: (Int) -> Void

    @Environment(\.dismiss) private var dismiss
    @State private var kind = Kind.impedance
    @State private var layer: CopperLayer
    @State private var impedance = 50.0
    @State private var differential = 100.0
    @State private var gapMM: Double
    @State private var amps = 1.0
    @State private var rise: Double

    init(document: SchematicDocument, layers: [CopperLayer], layer: CopperLayer, gapMM: Double,
         apply: @escaping (Int) -> Void) {
        self.document = document
        self.layers = layers
        self.apply = apply
        _layer = State(initialValue: layer)
        _gapMM = State(initialValue: gapMM)
        _rise = State(initialValue: document.board.stackup.temperatureRise)
    }

    private var goal: WidthGoal {
        switch kind {
        case .impedance: .impedance(impedance)
        case .differential: .differential(differential, gapMM: gapMM)
        case .current: .current(amps, rise: rise)
        }
    }

    var body: some View {
        let units = document.board.units
        let result = WidthCalculation.result(goal: goal, layer: layer, board: document.board)
        VStack(alignment: .leading, spacing: 12) {
            Text("Calculate Width").font(.title3.bold())
            Form {
                Picker("Goal", selection: $kind) {
                    ForEach(Kind.allCases) { Text($0.title).tag($0) }
                }
                Picker("Layer", selection: $layer) {
                    ForEach(layers, id: \.self) { Text($0.title).tag($0) }
                }
                switch kind {
                case .impedance:
                    LabeledContent("Target") {
                        HStack(spacing: 4) {
                            ValueField(title: "Target impedance", value: impedance, places: 1) {
                                impedance = min(max($0, 1), 10_000)
                                return impedance
                            }
                            Text("Ω").foregroundStyle(.secondary)
                        }
                    }
                case .differential:
                    LabeledContent("Target") {
                        HStack(spacing: 4) {
                            ValueField(title: "Target differential impedance", value: differential, places: 1) {
                                differential = min(max($0, 1), 10_000)
                                return differential
                            }
                            Text("Ω").foregroundStyle(.secondary)
                        }
                    }
                    LengthField(title: "Gap", millimetres: gapMM, units: units, precise: true) {
                        gapMM = min(max($0, NetGroup.gapRange.lowerBound), NetGroup.gapRange.upperBound)
                        return gapMM
                    }
                    .help("The gap between the pair's two traces, edge to edge")
                case .current:
                    LabeledContent("Current") {
                        HStack(spacing: 4) {
                            ValueField(title: "Current", value: amps, places: 2) {
                                amps = min(max($0, 0.01), 1000)
                                return amps
                            }
                            Text("A").foregroundStyle(.secondary)
                        }
                    }
                    LabeledContent("Temperature rise") {
                        HStack(spacing: 4) {
                            ValueField(title: "Temperature rise", value: rise, places: 0) {
                                rise = min(max($0, BoardStackup.temperatureRiseRange.lowerBound),
                                           BoardStackup.temperatureRiseRange.upperBound)
                                return rise
                            }
                            Text("°C").foregroundStyle(.secondary)
                        }
                    }
                }
            }
            .formStyle(.columns)
            VStack(alignment: .leading, spacing: 4) {
                if let exact = result.exactMil, let rounded = result.roundedMil {
                    Text("Width \(String(format: "%.2f", exact)) mil (\(LengthField.format(exact * 0.0254, units: units, precise: true)) \(units.title)), "
                         + "\(rounded) mil whole")
                        .monospacedDigit()
                    if let achieved = result.achieved {
                        Text(kind == .current ? "\(rounded) mil carries \(String(format: "%.2f", achieved)) A"
                                              : "\(rounded) mil gives \(String(format: "%.1f", achieved)) Ω")
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                }
                if let message = result.message {
                    Text(message).foregroundStyle(.red)
                }
                Text(EstimateText.accuracy).font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                Spacer()
                Button("Cancel") { dismiss() }
                    .keyboardShortcut(.cancelAction)
                Button("Apply") {
                    if let rounded = result.roundedMil, result.canApply { apply(rounded) }
                    dismiss()
                }
                .keyboardShortcut(.defaultAction)
                .disabled(!result.canApply)
            }
        }
        .padding(20)
        .frame(width: 400)
    }
}
