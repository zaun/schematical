//
//  ErcMarkerLayer.swift
//  schematical
//
//  The marks the Electrical Rules Check window puts on the schematic (plan-63-F):
//  a red marker at each error and a yellow one at each warning. On screen only:
//  the export scene never draws them, and they go with the window.
//

import CoreGraphics
import SwiftUI

/// One violation's mark.
nonisolated struct ErcMarker: Hashable, Sendable {
    let point: CGPoint
    let severity: ErcSeverity
    /// The violation's key, so a selected line can emphasise its mark.
    let key: String
}

/// A request to bring a point into view (a selected line); `serial` makes two
/// requests for one point different.
nonisolated struct ErcFocus: Equatable, Sendable {
    let point: CGPoint
    let serial: Int
}

/// Draws the markers, a triangle on its point.
struct ErcMarkerLayer: View {
    let markers: [ErcMarker]
    /// The keys of the selected lines: their markers are drawn larger.
    var selected: Set<String> = []
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    /// A marker's triangle: its tip on the point, apex up.
    static func triangle(at point: CGPoint, size: CGFloat) -> Path {
        Path { path in
            path.move(to: CGPoint(x: point.x, y: point.y - size))
            path.addLine(to: CGPoint(x: point.x + size * 0.9, y: point.y + size * 0.6))
            path.addLine(to: CGPoint(x: point.x - size * 0.9, y: point.y + size * 0.6))
            path.closeSubpath()
        }
    }

    var body: some View {
        Canvas { context, _ in
            context.translateBy(x: -area.minX, y: -area.minY)
            for marker in markers {
                let size: CGFloat = selected.contains(marker.key) ? 11 : 7
                let path = Self.triangle(at: marker.point, size: size)
                context.fill(path, with: .color(marker.severity == .error ? .red : .yellow))
                context.stroke(path, with: .color(.black.opacity(0.7)), lineWidth: 1)
            }
        }
        .frame(width: area.width, height: area.height)
        .offset(x: area.minX, y: area.minY)
        .allowsHitTesting(false)
    }
}
