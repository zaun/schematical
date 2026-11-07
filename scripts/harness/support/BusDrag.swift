import CoreGraphics
import Foundation

/// Pointer drags on a bus as `SchematicCanvasView` does them: the press picks
/// the line or end, each cursor frame calls `SchematicDocument.dragBus` from
/// the offset reached so far, and the release settles the port wires.
/// Frames come every `frame` units of cursor travel.
@MainActor
enum BusDrag {
    /// Drags the bus line under `point` by `delta`. Returns the offset reached.
    @discardableResult
    static func line(_ doc: SchematicDocument, at point: CGPoint, by delta: CGSize, frame: CGFloat = 10) -> CGSize? {
        guard let hit = doc.busHit(at: point), let base = doc.bus(hit.bus) else { return nil }
        doc.selectedBusID = hit.bus
        var done = CGSize.zero
        for target in frames(delta, frame: frame) {
            done = doc.dragBus(hit.bus, from: done, to: target) { BusEdit.moveLine(base, line: hit.line, by: $0) }
        }
        doc.settleWires(attachedToBus: hit.bus)
        return done
    }

    /// Drags the bus line end under `point` toward `target`. Returns where the
    /// end ended up.
    @discardableResult
    static func end(_ doc: SchematicDocument, at point: CGPoint, to target: CGPoint, frame: CGFloat = 10) -> CGPoint? {
        guard let hit = doc.busEndHit(at: point), let base = doc.bus(hit.bus),
              let start = base.line(hit.line)?.point(hit.end) else { return nil }
        doc.selectedBusID = hit.bus
        var done = CGSize.zero
        let delta = CGSize(width: target.x - start.x, height: target.y - start.y)
        for offset in frames(delta, frame: frame) {
            done = doc.dragBus(hit.bus, from: done, to: offset) {
                BusEdit.moveEnd(base, line: hit.line, end: hit.end,
                                to: CGPoint(x: start.x + $0.width, y: start.y + $0.height))
            }
        }
        doc.settleWires(attachedToBus: hit.bus)
        return doc.bus(hit.bus)?.line(hit.line)?.point(hit.end)
    }

    /// Cursor offsets from the press to `delta`, one per `frame` of travel.
    private static func frames(_ delta: CGSize, frame: CGFloat) -> [CGSize] {
        let count = max(Int((max(abs(delta.width), abs(delta.height)) / frame).rounded(.up)), 1)
        return (1...count).map { k in
            let t = CGFloat(k) / CGFloat(count)
            return CGSize(width: delta.width * t, height: delta.height * t)
        }
    }
}
