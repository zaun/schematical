import Foundation

/// The finish on the board's bare copper: pads with no part on them, rings,
/// mounting holes. Raw values are the file values.
nonisolated enum SurfaceFinish: String, CaseIterable, Hashable, Identifiable, Sendable {
    case hasl, leadFreeHasl, enig, osp

    var id: String { rawValue }

    var title: String {
        switch self {
        case .hasl: "HASL (leaded)"
        case .leadFreeHasl: "HASL (lead-free)"
        case .enig: "ENIG"
        case .osp: "OSP"
        }
    }

    /// The name KiCad's `copper_finish` (and a Gerber job file's `Finish`) uses.
    var kiCadName: String {
        switch self {
        case .hasl: "HAL SnPb"
        case .leadFreeHasl: "HAL lead-free"
        case .enig: "ENIG"
        case .osp: "OSP"
        }
    }
}

/// What a dielectric layer is: a cured core, or prepreg that bonds layers.
nonisolated enum DielectricKind: String, CaseIterable, Hashable, Sendable {
    case core, prepreg

    var title: String { self == .core ? "Core" : "Prepreg" }
}

/// One dielectric between two copper layers. Thickness in mm, Er relative.
nonisolated struct Dielectric: Hashable, Sendable {
    var kind: DielectricKind
    var thickness: Double
    var er: Double
}

/// The board's layer stack: the copper thickness of each layer (Top first,
/// Bottom last), the dielectric between each two copper layers (one for a
/// 1-layer board), the surface finish and the temperature rise the current
/// estimates use. Always agrees with `BoardSettings.layers`
/// (`BoardSettings.layers`' `didSet`).
nonisolated struct BoardStackup: Hashable, Sendable {
    /// mm, one per copper layer, Top first.
    var copper: [Double]
    /// Between `copper[i]` and `copper[i + 1]`; one for a 1-layer board.
    var dielectrics: [Dielectric]
    var finish: SurfaceFinish
    /// °C, within `temperatureRiseRange`.
    var temperatureRise: Double

    static let copperRange: ClosedRange<Double> = 0.005...0.2
    static let thicknessRange: ClosedRange<Double> = 0.02...5
    static let erRange: ClosedRange<Double> = 1...20
    static let temperatureRiseRange: ClosedRange<Double> = 1...100
    static let defaultTemperatureRise = 10.0
    /// mm: the solder mask on each outer face, as KiCad writes it.
    static let maskThickness = 0.01

    /// How many dielectrics a board of `layers` copper layers has.
    static func dielectricCount(layers: Int) -> Int { max(layers - 1, 1) }

    /// The whole board, mm: copper, dielectrics and the solder mask on each
    /// side that has copper (KiCad's board thickness sums the same).
    var thickness: Double {
        copper.reduce(0, +) + dielectrics.reduce(0) { $0 + $1.thickness }
            + Double(copper.count == 1 ? 1 : 2) * Self.maskThickness
    }

    /// The standard stackup for a board of `layers` layers (1, 2, 4 or 6):
    /// JLCPCB's published 4- and 6-layer 1.6 mm stackups, and a 1.6 mm FR4
    /// core for 1 and 2 layers. `finish` and `temperatureRise` keep the
    /// caller's values; without a finish, lead-free HASL (ENIG at 6 layers,
    /// where JLCPCB offers no HASL).
    static func standard(layers: Int, finish: SurfaceFinish? = nil,
                         temperatureRise: Double = defaultTemperatureRise) -> BoardStackup {
        func prepreg(_ t: Double, _ er: Double) -> Dielectric { Dielectric(kind: .prepreg, thickness: t, er: er) }
        func core(_ t: Double, _ er: Double) -> Dielectric { Dielectric(kind: .core, thickness: t, er: er) }
        let copper: [Double]
        let dielectrics: [Dielectric]
        var defaultFinish = SurfaceFinish.leadFreeHasl
        switch layers {
        case 1:
            copper = [0.035]
            dielectrics = [core(1.555, 4.5)]
        case 4:
            copper = [0.035, 0.0152, 0.0152, 0.035]
            dielectrics = [prepreg(0.2104, 4.4), core(1.065, 4.6), prepreg(0.2104, 4.4)]
        case 6:
            copper = [0.035, 0.0152, 0.0152, 0.0152, 0.0152, 0.035]
            dielectrics = [prepreg(0.0994, 4.1), core(0.55, 4.6), prepreg(0.1164, 4.16), core(0.55, 4.6),
                           prepreg(0.0994, 4.1)]
            defaultFinish = .enig
        default:
            copper = [0.035, 0.035]
            dielectrics = [core(1.51, 4.5)]
        }
        return BoardStackup(copper: copper, dielectrics: dielectrics, finish: finish ?? defaultFinish,
                            temperatureRise: temperatureRise)
    }

    /// Why these values can't make a stackup for a `layers`-layer board, as
    /// a file error detail after "board stackup ", or nil.
    static func problem(copper: [Double], dielectrics: [Dielectric], temperatureRise: Double,
                        layers: Int) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if copper.count != layers {
            return "copper has \(copper.count) \(copper.count == 1 ? "entry" : "entries"); "
                + "a \(layers)-layer board needs \(layers)"
        }
        let wanted = dielectricCount(layers: layers)
        if dielectrics.count != wanted {
            return "dielectrics has \(dielectrics.count) \(dielectrics.count == 1 ? "entry" : "entries"); "
                + "a \(layers)-layer board needs \(wanted)"
        }
        for value in copper where !copperRange.contains(value) {
            return "copper thickness \(n(value)) must be 0.005 to 0.2 mm"
        }
        for dielectric in dielectrics {
            if !thicknessRange.contains(dielectric.thickness) {
                return "dielectric thickness \(n(dielectric.thickness)) must be 0.02 to 5 mm"
            }
            if !erRange.contains(dielectric.er) { return "dielectric er \(n(dielectric.er)) must be 1 to 20" }
        }
        if !temperatureRiseRange.contains(temperatureRise) {
            return "temperatureRise \(n(temperatureRise)) must be 1 to 100 °C"
        }
        return nil
    }
}
