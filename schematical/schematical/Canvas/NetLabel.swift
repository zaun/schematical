import SwiftUI

/// A net label: a named flag with one connection point. Every label with
/// the same name (ignoring case) is one net, wherever it is on the
/// schematic, so wires can join without being drawn to each other. It is
/// never simulated itself.
///
/// Its point is `origin`, on the grid; a wire ends there, leaving sideways.
/// The flag runs `length` to the right of it, or to the left when flipped.
/// Its type sets its ends: an output points away from the wire (`---|__>`),
/// an input points in at the wire (`---<__|`), an in/out points both ways
/// (`---<__>`) and a passive one is a plain box (`---|__|`). A No Connect is not a label at all: an X on its point, with
/// no name, that marks a pin as deliberately left unconnected. It joins
/// nothing, not even the wires that end on it. A Power Flag (plan-63-A) is a
/// small flag on a stem at its point: no name, and nothing joined but the wire
/// that ends on it. It marks its net as driven for the electrical rules check
/// (KiCad's `PWR_FLAG`).
nonisolated struct NetLabel: Identifiable, Hashable, Sendable {
    enum Style: String, CaseIterable, Identifiable, Sendable {
        case input
        case output
        case `inout`
        case passive
        case noConnect
        case powerFlag

        var id: String { rawValue }

        var title: String {
            switch self {
            case .input: "Input"
            case .output: "Output"
            case .inout: "In/Out"
            case .passive: "Passive"
            case .noConnect: "No Connect"
            case .powerFlag: "Power Flag"
            }
        }

        /// The end away from the wire is a point.
        var pointsOut: Bool { self == .output || self == .inout }
        /// The end at the wire is a point.
        var pointsIn: Bool { self == .input || self == .inout }
        /// A named net label; false for No Connect and Power Flag.
        var isNet: Bool { self != .noConnect && self != .powerFlag }
        /// A No Connect: joins nothing, not even its wires.
        var isNoConnect: Bool { self == .noConnect }
        /// A Power Flag: no name, marks its net as driven.
        var isPowerFlag: Bool { self == .powerFlag }
    }

    /// Half the size of a No Connect's X.
    static let crossSize: CGFloat = 5
    /// What a press hits round a No Connect's point.
    static let crossHit: CGFloat = 10

    /// The name is drawn and joins labels; not for a No Connect or a Power Flag.
    var isNet: Bool { style.isNet }
    var isNoConnect: Bool { style.isNoConnect }
    var isPowerFlag: Bool { style.isPowerFlag }
    /// What the Net Label panel and the selection list call it when it has no name.
    var title: String { style.title }

    /// A Power Flag's stem, up from its point, and its pennant.
    static let flagStem: CGFloat = 20
    static let flagWidth: CGFloat = 14
    static let flagHeight: CGFloat = 10

    let id: UUID
    /// 1 to `maxNameLength` characters, no spaces.
    var name: String
    var style: Style = .input
    /// The connection point. On the grid.
    var origin: CGPoint
    /// The flag runs to the left of `origin` instead of the right.
    var isFlipped = false

    static let maxNameLength = 8
    /// The flag: six grid steps long, two high, its points one step deep.
    static let length: CGFloat = 60
    static let halfHeight: CGFloat = 10
    static let pointDepth: CGFloat = 10
    static let fontSize: CGFloat = 11

    /// A name as it would be stored: no whitespace, at most `maxNameLength`
    /// characters. Nil when nothing is left.
    static func cleanName(_ value: String) -> String? {
        let kept = String(value.filter { !$0.isWhitespace }.prefix(maxNameLength))
        return kept.isEmpty ? nil : kept
    }

    /// The key labels are joined by: the name, case and all ("CLK" and
    /// "clk" are two nets).
    var netKey: String { name }

    /// +1 when the flag runs right of its point, -1 when it runs left.
    private var direction: CGFloat { isFlipped ? -1 : 1 }

    /// Where a wire meets it.
    var tip: CGPoint { origin }

    /// The flag's bounds; a square round the point for a No Connect; the stem
    /// and pennant (and the No Connect's square) for a Power Flag.
    var rect: CGRect {
        if isPowerFlag {
            return CGRect(x: origin.x - Self.crossHit, y: origin.y - Self.flagStem - 4,
                          width: Self.crossHit + Self.flagWidth, height: Self.flagStem + 4 + Self.crossHit)
        }
        if isNoConnect {
            return CGRect(x: origin.x - Self.crossHit, y: origin.y - Self.crossHit,
                          width: 2 * Self.crossHit, height: 2 * Self.crossHit)
        }
        let minX = isFlipped ? origin.x - Self.length : origin.x
        return CGRect(x: minX, y: origin.y - Self.halfHeight, width: Self.length, height: 2 * Self.halfHeight)
    }

    /// The name's font size: `fontSize`, shrunk for a long name so it stays
    /// inside the flag, which keeps its length whatever the name.
    var nameSize: CGFloat {
        PaintedText.monospacedSize(Self.fontSize, characters: name.count,
                                   fitting: Self.length - 2 * Self.pointDepth - 4)
    }

    /// The middle of the flag, where the name is drawn.
    var nameCentre: CGPoint {
        CGPoint(x: origin.x + direction * Self.length / 2, y: origin.y)
    }

    /// The flag's outline, clockwise from the wire end's top.
    var outline: [CGPoint] {
        let l = Self.length, h = Self.halfHeight, d = Self.pointDepth
        var local: [CGPoint] = []
        // The wire end: a point at the wire, or a flat end.
        if style.pointsIn {
            local += [CGPoint(x: 0, y: 0), CGPoint(x: d, y: -h)]
        } else {
            local += [CGPoint(x: 0, y: -h)]
        }
        // The far end.
        if style.pointsOut {
            local += [CGPoint(x: l - d, y: -h), CGPoint(x: l, y: 0), CGPoint(x: l - d, y: h)]
        } else {
            local += [CGPoint(x: l, y: -h), CGPoint(x: l, y: h)]
        }
        local += [style.pointsIn ? CGPoint(x: d, y: h) : CGPoint(x: 0, y: h)]
        return local.map { CGPoint(x: origin.x + direction * $0.x, y: origin.y + $0.y) }
    }

    /// The flag's outline, closed; a No Connect's X; a Power Flag's stem with
    /// its pennant (the one closed subpath, so a fill paints the pennant).
    var path: Path {
        Path { path in
            if isNet {
                path.addLines(outline)
                path.closeSubpath()
            } else if isPowerFlag {
                let top = origin.y - Self.flagStem
                path.move(to: origin)
                path.addLine(to: CGPoint(x: origin.x, y: top))
                path.move(to: CGPoint(x: origin.x, y: top))
                path.addLine(to: CGPoint(x: origin.x + Self.flagWidth, y: top + Self.flagHeight / 2))
                path.addLine(to: CGPoint(x: origin.x, y: top + Self.flagHeight))
                path.closeSubpath()
            } else {
                let d = Self.crossSize
                path.move(to: CGPoint(x: origin.x - d, y: origin.y - d))
                path.addLine(to: CGPoint(x: origin.x + d, y: origin.y + d))
                path.move(to: CGPoint(x: origin.x + d, y: origin.y - d))
                path.addLine(to: CGPoint(x: origin.x - d, y: origin.y + d))
            }
        }
    }
}

/// Draws net labels: each flag's outline with its name in the middle. A
/// selected label is accent; the hovered one gets a soft accent underlay.
/// The export draws it too.
struct NetLabelLayer: View {
    let labels: [NetLabel]
    var selectedIDs: Set<UUID> = []
    var hoveredID: UUID?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.translateBy(x: -area.minX, y: -area.minY)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, labels: labels, selectedIDs: selectedIDs, hoveredID: hoveredID)
        }
        .frame(width: area.width, height: area.height)
        .offset(x: area.minX, y: area.minY)
    }

    /// Draws the labels in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, labels: [NetLabel],
                                           selectedIDs: Set<UUID>, hoveredID: UUID?) {
        for label in labels {
            let path = label.path
            let isSelected = selectedIDs.contains(label.id)
            if label.id == hoveredID, !isSelected {
                painter.stroke(path, with: Ink.accent.opacity(0.35), lineWidth: 5)
            }
            if label.isPowerFlag {
                // Power Flag: a stem and a solid pennant, no name.
                painter.fill(path, with: isSelected ? .accent : .primary)
                painter.stroke(path, with: isSelected ? .accent : .primary, lineWidth: isSelected ? 2.5 : 2)
                continue
            }
            guard label.isNet else {
                // No Connect: just the X, no name.
                painter.stroke(path, with: isSelected ? .accent : .primary, lineWidth: isSelected ? 2.5 : 2)
                continue
            }
            painter.fill(path, with: .paper)
            painter.stroke(path, with: isSelected ? .accent : .primary, lineWidth: isSelected ? 2 : 1.25)
            painter.draw(PaintedText(label.name, size: label.nameSize, weight: .semibold, monospaced: true),
                         at: label.nameCentre, anchor: .center)
        }
    }
}
