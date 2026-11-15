import CoreGraphics
import Foundation

/// Length-tuning bumps (plan-38-I § 3): a U standing off a straight host
/// segment, two perpendicular legs `spacing` apart (centre to centre) joined
/// by a flat top, its four corners optionally chamfered at 45°; and an
/// accordion of them, alternating sides.
nonisolated enum Meander {
    /// What one bump adds to the host's length: its two legs, less what
    /// chamfering its four corners takes off. A corner cut by `cut` along
    /// each side replaces `2 × cut` with `cut × √2`.
    static func added(amplitude: Double, cut: Double) -> Double {
        2 * amplitude - 4 * cut * (2 - 2.squareRoot())
    }

    /// The cut a bump of `amplitude` and `spacing` can take at its corners:
    /// `cut`, but never more than half a leg or half the top.
    static func fitted(cut: Double, amplitude: Double, spacing: Double) -> Double {
        max(0, min(cut, amplitude / 2, spacing / 2))
    }

    /// The host segment a–b with one bump on it, `offset` mm from `a`,
    /// standing out `amplitude` mm on `side` (+1 left of a→b, −1 right).
    static func bump(from a: CGPoint, to b: CGPoint, at offset: Double, side: Double, amplitude: Double,
                     spacing: Double, cut: Double) -> [CGPoint] {
        [a] + bumpPoints(from: a, to: b, at: offset, side: side, amplitude: amplitude, spacing: spacing, cut: cut) + [b]
    }

    /// The vertices of one bump, without the host's ends.
    static func bumpPoints(from a: CGPoint, to b: CGPoint, at offset: Double, side: Double, amplitude: Double,
                           spacing: Double, cut: Double) -> [CGPoint] {
        let length = hypot(b.x - a.x, b.y - a.y)
        guard length > 0 else { return [] }
        let u = CGPoint(x: (b.x - a.x) / length, y: (b.y - a.y) / length)
        let n = CGPoint(x: -u.y * side, y: u.x * side)
        func at(_ along: Double, _ out: Double) -> CGPoint {
            CGPoint(x: a.x + u.x * along + n.x * out, y: a.y + u.y * along + n.y * out)
        }
        let c = fitted(cut: cut, amplitude: amplitude, spacing: spacing)
        let x0 = offset, x1 = offset + spacing
        guard c > 1e-9 else {
            return [at(x0, 0), at(x0, amplitude), at(x1, amplitude), at(x1, 0)]
        }
        // Each corner cut: the point `c` before it and `c` after it.
        return [at(x0 - c, 0), at(x0, c), at(x0, amplitude - c), at(x0 + c, amplitude),
                at(x1 - c, amplitude), at(x1, amplitude - c), at(x1, c), at(x1 + c, 0)]
    }

    /// `count` bumps on a–b from `offset`, one leg spacing apart, sides
    /// alternating from `side`; the whole replacement polyline a…b.
    static func accordion(from a: CGPoint, to b: CGPoint, at offset: Double, count: Int, side: Double,
                          amplitude: Double, spacing: Double, cut: Double) -> [CGPoint] {
        var points = [a]
        var s = side
        for k in 0..<count {
            points += bumpPoints(from: a, to: b, at: offset + Double(k) * 2 * spacing, side: s, amplitude: amplitude,
                                 spacing: spacing, cut: cut)
            s = -s
        }
        points.append(b)
        return TraceRoute.clean(points)
    }

    /// The host length one bump needs: its two legs' spacing, and its cuts
    /// on both sides.
    static func footprint(spacing: Double, cut: Double) -> Double { spacing + 2 * cut }

    /// The amplitude whose bump adds `target` mm, by bisection to 1e-6 mm,
    /// within `range`; the range's end when the target is out of it.
    static func amplitude(forAdded target: Double, cut: Double, spacing: Double,
                          in range: ClosedRange<Double>) -> Double {
        func added(_ amplitude: Double) -> Double {
            Self.added(amplitude: amplitude, cut: fitted(cut: cut, amplitude: amplitude, spacing: spacing))
        }
        var low = range.lowerBound, high = range.upperBound
        if added(high) <= target { return high }
        if added(low) >= target { return low }
        for _ in 0..<80 {
            let mid = (low + high) / 2
            if added(mid) < target { low = mid } else { high = mid }
            if high - low < 1e-7 { break }
        }
        return (low + high) / 2
    }
}
