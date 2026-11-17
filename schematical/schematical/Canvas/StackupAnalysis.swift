import Foundation

/// How a trace sits in the stackup: over one reference plane (microstrip), or between two (stripline).
nonisolated enum TraceGeometry: Equatable, Sendable {
    /// `height` mm of dielectric (and any copper of non-plane layers between) to the plane, `er` the
    /// thickness-weighted mean of the dielectrics. `assumedReference`: a 2-layer board, whose other side is
    /// taken as a solid ground.
    case microstrip(height: Double, er: Double, assumedReference: Bool)
    /// The distances to the plane above and the plane below, `er` the thickness-weighted mean over both.
    case stripline(above: Double, below: Double, er: Double)
}

/// What a width would have to be for a target (`StackupAnalysis.width(for…)`): a width in mil, or no width
/// gives that value.
nonisolated enum WidthSolution: Equatable, Sendable {
    case mil(Double)
    /// The target is lower than the impedance of the widest trace tried (1000 mil).
    case tooLow
    /// The target is higher than the impedance of the narrowest (1 mil).
    case tooHigh
}

/// Impedance and current estimates for the traces of a board's layers, from its stackup (plan-50-F): Hammerstad–
/// Jensen microstrip, Cohn stripline and the IPC-2141 pair coupling, IPC-2221 for current. Estimates about
/// ±10%: they ignore the solder mask, loss and frequency. The formulas and what they were checked against are in
/// `planning/plan-50-data/README.md`.
nonisolated struct StackupAnalysis: Sendable {
    let board: BoardSettings

    init(_ board: BoardSettings) { self.board = board }

    // MARK: Geometry

    /// The geometry of a trace on `layer`, or nil when it has none (`unavailableReason`).
    func geometry(of layer: CopperLayer) -> TraceGeometry? {
        let layers = board.copperLayers
        guard layers.count > 1, let index = layers.firstIndex(of: layer), !isPlane(layers[index]) else { return nil }
        let stackup = board.stackup
        if layers.count == 2 {
            let dielectric = stackup.dielectrics[0]
            return .microstrip(height: dielectric.thickness, er: dielectric.er, assumedReference: true)
        }
        if index == 0 || index == layers.count - 1 {
            guard let found = reach(from: index, step: index == 0 ? 1 : -1) else { return nil }
            return .microstrip(height: found.height, er: found.weighted / found.dielectric, assumedReference: false)
        }
        guard let above = reach(from: index, step: -1), let below = reach(from: index, step: 1) else { return nil }
        return .stripline(above: above.height, below: below.height,
                          er: (above.weighted + below.weighted) / (above.dielectric + below.dielectric))
    }

    /// Why `geometry(of:)` is nil, for a help text.
    func unavailableReason(for layer: CopperLayer) -> String {
        let layers = board.copperLayers
        if layers.count == 1 { return "A 1-layer board has no reference plane." }
        guard let index = layers.firstIndex(of: layer) else { return "\(layer.title) is not on this board." }
        if isPlane(layers[index]) { return "A plane layer carries no traces." }
        if index == 0 || index == layers.count - 1 { return "No plane layer under \(layer.title)." }
        return "Needs a plane layer on both sides."
    }

    private func isPlane(_ layer: CopperLayer) -> Bool { board.style(of: layer).type == .plane }

    /// From the layer at `index` toward the nearest plane layer in direction `step`: the height (the dielectrics
    /// plus the copper of non-plane layers passed), the dielectrics' own thickness, and their thickness × Er sum.
    /// Nil without a plane.
    private func reach(from index: Int, step: Int) -> (height: Double, dielectric: Double, weighted: Double)? {
        let layers = board.copperLayers
        let stackup = board.stackup
        var height = 0.0, dielectricThickness = 0.0, weighted = 0.0
        var j = index + step
        while layers.indices.contains(j) {
            let dielectric = stackup.dielectrics[step > 0 ? j - 1 : j]
            height += dielectric.thickness
            dielectricThickness += dielectric.thickness
            weighted += dielectric.thickness * dielectric.er
            if isPlane(layers[j]) { return (height, dielectricThickness, weighted) }
            if layers.indices.contains(j + step) { height += stackup.copper[j] }
            j += step
        }
        return nil
    }

    /// The copper thickness of `layer` in mm.
    func copperThickness(of layer: CopperLayer) -> Double {
        board.copperLayers.firstIndex(of: layer).map { board.stackup.copper[$0] } ?? BoardStackup.standard(layers: 2).copper[0]
    }

    // MARK: Estimates

    /// The single-ended impedance (Ω) of a trace `widthMM` wide on `layer`, nil without a geometry.
    func impedance(widthMM: Double, on layer: CopperLayer) -> Double? {
        guard let geometry = geometry(of: layer), widthMM > 0 else { return nil }
        let t = copperThickness(of: layer)
        switch geometry {
        case let .microstrip(height, er, _): return Self.microstripZ0(width: widthMM, height: height, thickness: t, er: er)
        case let .stripline(above, below, er):
            return Self.striplineZ0(width: widthMM, above: above, below: below, thickness: t, er: er)
        }
    }

    /// The differential impedance (Ω) of a pair of traces `widthMM` wide with `gapMM` between their edges.
    func differentialImpedance(widthMM: Double, gapMM: Double, on layer: CopperLayer) -> Double? {
        guard let geometry = geometry(of: layer), widthMM > 0, gapMM > 0 else { return nil }
        let t = copperThickness(of: layer)
        switch geometry {
        case let .microstrip(height, er, _):
            return Self.microstripDifferential(width: widthMM, gap: gapMM, height: height, thickness: t, er: er)
        case let .stripline(above, below, er):
            return Self.striplineDifferential(width: widthMM, gap: gapMM, above: above, below: below, thickness: t, er: er)
        }
    }

    /// The current (A) a trace `widthMil` wide carries for the stackup's temperature rise (IPC-2221).
    func currentCapacity(widthMil: Double, on layer: CopperLayer) -> Double {
        Self.currentCapacity(widthMil: widthMil, thicknessMM: copperThickness(of: layer),
                             rise: board.stackup.temperatureRise, external: Self.isExternal(layer))
    }

    static func isExternal(_ layer: CopperLayer) -> Bool { !layer.isInner }

    // MARK: Solving for a width

    /// The width (mil) that gives `target` Ω on `layer`; nil without a geometry.
    func width(forImpedance target: Double, on layer: CopperLayer) -> WidthSolution? {
        guard geometry(of: layer) != nil else { return nil }
        return Self.solve(target: target) { impedance(widthMM: $0 * 0.0254, on: layer) ?? 0 }
    }

    /// The width (mil) that gives `target` Ω of differential impedance at `gapMM` on `layer`.
    func width(forDifferential target: Double, gapMM: Double, on layer: CopperLayer) -> WidthSolution? {
        guard geometry(of: layer) != nil else { return nil }
        return Self.solve(target: target) { differentialImpedance(widthMM: $0 * 0.0254, gapMM: gapMM, on: layer) ?? 0 }
    }

    /// The width (mil) that carries `amps` for `rise` °C on `layer`.
    func width(forCurrent amps: Double, rise: Double, on layer: CopperLayer) -> Double {
        Self.widthForCurrent(amps: amps, thicknessMM: copperThickness(of: layer), rise: rise,
                             external: Self.isExternal(layer))
    }

    /// Bisection on the width in mil over 1…1000, for an impedance that falls as the width rises.
    private static func solve(target: Double, impedance: (Double) -> Double) -> WidthSolution {
        var low = 1.0, high = 1000.0
        if impedance(low) < target { return .tooHigh }
        if impedance(high) > target { return .tooLow }
        for _ in 0..<60 {
            let mid = (low + high) / 2
            if impedance(mid) > target { low = mid } else { high = mid }
        }
        return .mil((low + high) / 2)
    }

    // MARK: Formulas

    private static let eta0 = 376.730313668

    /// Hammerstad–Jensen microstrip impedance with Wheeler's thickness correction (mm, Ω).
    static func microstripZ0(width: Double, height: Double, thickness: Double, er: Double) -> Double {
        let u = width / height
        let tt = thickness / height
        let du1 = tt > 0 ? tt / .pi * log(1 + 4 * M_E / tt * pow(tanh((6.517 * u).squareRoot()), 2)) : 0
        let dur = du1 * (1 + 1 / cosh((er - 1).squareRoot())) / 2
        let ur = u + dur
        return hammersteadZ(ur) / hammersteadEffectiveEr(ur, er).squareRoot()
    }

    private static func hammersteadZ(_ u: Double) -> Double {
        let f = 6 + (2 * .pi - 6) * exp(-pow(30.666 / u, 0.7528))
        return eta0 / (2 * .pi) * log(f / u + (1 + pow(2 / u, 2)).squareRoot())
    }

    private static func hammersteadEffectiveEr(_ u: Double, _ er: Double) -> Double {
        let a = 1 + log((pow(u, 4) + pow(u / 52, 2)) / (pow(u, 4) + 0.432)) / 49 + log(1 + pow(u / 18.1, 3)) / 18.7
        let b = 0.564 * pow((er - 0.9) / (er + 3), 0.053)
        return (er + 1) / 2 + (er - 1) / 2 * pow(1 + 10 / u, -a * b)
    }

    /// The differential microstrip impedance: `2·Z0·(1 − 0.48·exp(−0.96·gap/height))` (IPC-2141).
    static func microstripDifferential(width: Double, gap: Double, height: Double, thickness: Double,
                                       er: Double) -> Double {
        2 * microstripZ0(width: width, height: height, thickness: thickness, er: er) * (1 - 0.48 * exp(-0.96 * gap / height))
    }

    /// The width of a strip with its thickness allowed for (Wheeler, halved for a strip between planes).
    private static func effectiveWidth(_ width: Double, _ thickness: Double) -> Double {
        thickness > 0 ? width + 0.5 * (thickness / .pi) * (1 + log(4 * .pi * width / thickness)) : width
    }

    /// `K(k')/K(k)` for the modulus `k` through the arithmetic–geometric mean.
    private static func ellipticRatio(_ k: Double) -> Double {
        func agm(_ a: Double, _ b: Double) -> Double {
            var a = a, b = b
            for _ in 0..<40 {
                let next = ((a + b) / 2, (a * b).squareRoot())
                if abs(next.0 - next.1) < 1e-15 { return next.0 }
                (a, b) = next
            }
            return a
        }
        return agm(1, (1 - k * k).squareRoot()) / agm(1, k)
    }

    /// The impedance of a strip centred between two planes `spacing` apart (Cohn).
    static func striplineCentred(width: Double, spacing: Double, thickness: Double, er: Double) -> Double {
        let k = tanh(.pi * effectiveWidth(width, thickness) / (2 * spacing))
        return 30 * .pi / er.squareRoot() * ellipticRatio(k)
    }

    /// A strip `above` mm from one plane and `below` mm from the other: the two centred values for twice each
    /// distance (plus the copper) in parallel.
    static func striplineZ0(width: Double, above: Double, below: Double, thickness: Double, er: Double) -> Double {
        let za = striplineCentred(width: width, spacing: 2 * above + thickness, thickness: thickness, er: er)
        let zb = striplineCentred(width: width, spacing: 2 * below + thickness, thickness: thickness, er: er)
        return 2 * za * zb / (za + zb)
    }

    /// Twice the odd-mode impedance of two coupled strips centred `spacing` apart in height (Cohn).
    private static func striplineOddCentred(width: Double, gap: Double, spacing: Double, thickness: Double,
                                            er: Double) -> Double {
        let a = tanh(.pi * effectiveWidth(width, thickness) / (2 * spacing))
        let c = tanh(.pi * (width + gap) / (2 * spacing))
        return 2 * 30 * .pi / er.squareRoot() * ellipticRatio(min(a / c, 1 - 1e-9))
    }

    static func striplineDifferential(width: Double, gap: Double, above: Double, below: Double, thickness: Double,
                                      er: Double) -> Double {
        let za = striplineOddCentred(width: width, gap: gap, spacing: 2 * above + thickness, thickness: thickness, er: er)
        let zb = striplineOddCentred(width: width, gap: gap, spacing: 2 * below + thickness, thickness: thickness, er: er)
        return 2 * za * zb / (za + zb)
    }

    /// IPC-2221: `I = k·ΔT^0.44·A^0.725`, `A` the copper's cross-section in mil², `k` 0.048 outside, 0.024 inside.
    static func currentCapacity(widthMil: Double, thicknessMM: Double, rise: Double, external: Bool) -> Double {
        let area = widthMil * thicknessMM / 0.0254
        return (external ? 0.048 : 0.024) * pow(rise, 0.44) * pow(area, 0.725)
    }

    /// The width in mil for `amps`: the inverse of `currentCapacity`.
    static func widthForCurrent(amps: Double, thicknessMM: Double, rise: Double, external: Bool) -> Double {
        let area = pow(amps / ((external ? 0.048 : 0.024) * pow(rise, 0.44)), 1 / 0.725)
        return area / (thicknessMM / 0.0254)
    }
}
