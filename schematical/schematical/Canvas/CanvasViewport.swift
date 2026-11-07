import CoreGraphics
import Observation

/// Maps between screen points (the canvas view) and world points (the schematic
/// coordinates the document uses): screen = world * zoom + offset.
@MainActor
@Observable
final class CanvasViewport {
    static let zoomLevels: [CGFloat] = [0.25, 0.5, 0.75, 1, 1.5, 2, 3]

    private(set) var zoom: CGFloat = 1
    /// Screen position of the world origin.
    private(set) var offset: CGSize = .zero
    private(set) var size: CGSize = .zero

    private var zoomRange: ClosedRange<CGFloat> {
        Self.zoomLevels.first!...Self.zoomLevels.last!
    }

    /// A viewport at `zoom` (clamped to the zoom steps' range), with the
    /// world origin at the view's top left.
    init(zoom: CGFloat = 1) {
        self.zoom = min(max(zoom, Self.zoomLevels.first!), Self.zoomLevels.last!)
    }

    func updateSize(_ newSize: CGSize) {
        size = newSize
    }

    func world(_ screen: CGPoint) -> CGPoint {
        CGPoint(x: (screen.x - offset.width) / zoom, y: (screen.y - offset.height) / zoom)
    }

    /// The part of the world currently on screen.
    var visibleWorldRect: CGRect {
        CGRect(
            origin: world(.zero),
            size: CGSize(width: size.width / zoom, height: size.height / zoom)
        )
    }

    func zoomIn() {
        if let next = Self.zoomLevels.first(where: { $0 > zoom + 0.001 }) {
            setZoom(next)
        }
    }

    func zoomOut() {
        if let previous = Self.zoomLevels.last(where: { $0 < zoom - 0.001 }) {
            setZoom(previous)
        }
    }

    /// Changes zoom while keeping the world point at the centre of the view fixed.
    func setZoom(_ value: CGFloat) {
        let newZoom = min(max(value, zoomRange.lowerBound), zoomRange.upperBound)
        let centre = CGPoint(x: size.width / 2, y: size.height / 2)
        let anchor = world(centre)

        zoom = newZoom
        offset = CGSize(
            width: centre.x - anchor.x * newZoom,
            height: centre.y - anchor.y * newZoom
        )
    }

    func pan(to newOffset: CGSize) {
        offset = newOffset
    }
}
