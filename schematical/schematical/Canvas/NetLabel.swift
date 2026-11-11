import SwiftUI

/// A net label: a named flag with one connection point. Every label with
/// the same name (ignoring case) is one net, wherever it is on the
/// schematic, so wires can join without being drawn to each other. It is
/// never simulated itself.
///
/// Its point is `origin`, on the grid; a wire ends there, leaving sideways.
/// The flag runs `length` to the right of it, or to the left when flipped.
/// Its type sets its ends: an input points away from the wire, an output
/// points in at the wire, and an in/out points both ways.
nonisolated struct NetLabel: Identifiable, Hashable, Sendable {
    enum Style: String, CaseIterable, Identifiable, Sendable {
        case input
        case output
        case `inout`

        var id: String { rawValue }

        var title: String {
            switch self {
            case .input: "Input"
            case .output: "Output"
            case .inout: "In/Out"
            }
        }

        /// The end away from the wire is a point.
        var pointsOut: Bool { self != .output }
        /// The end at the wire is a point.
        var pointsIn: Bool { self != .input }
    }

    let id: UUID
    /// 1 to `maxNameLength` characters, no spaces.
    var name: String
    var style: Style = .input
    /// The connection point. On the grid.
    var origin: CGPoint
    /// The flag runs to the left of `origin` instead of the right.
    var isFlipped = false

    static let maxNameLength = 4
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

    /// The key labels are joined by: the name, ignoring case.
    var netKey: String { name.uppercased() }

    /// +1 when the flag runs right of its point, -1 when it runs left.
    private var direction: CGFloat { isFlipped ? -1 : 1 }

    /// Where a wire meets it.
    var tip: CGPoint { origin }

    /// The flag's bounds.
    var rect: CGRect {
        let minX = isFlipped ? origin.x - Self.length : origin.x
        return CGRect(x: minX, y: origin.y - Self.halfHeight, width: Self.length, height: 2 * Self.halfHeight)
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

    var path: Path {
        Path { path in
            path.addLines(outline)
            path.closeSubpath()
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
            painter.fill(path, with: .paper)
            painter.stroke(path, with: isSelected ? .accent : .primary, lineWidth: isSelected ? 2 : 1.25)
            painter.draw(PaintedText(label.name, size: NetLabel.fontSize, weight: .semibold, monospaced: true),
                         at: label.nameCentre, anchor: .center)
        }
    }
}
