import CoreGraphics
import Foundation
import Observation
import XCTest
@testable import schematical

/// `BoardFeatures` on hand-built pours, keep-outs, cutouts and holes, with
/// no document (plan-68-E).
@MainActor
final class BoardFeaturesTests: XCTestCase {
    private func empty() -> BoardFeatures {
        BoardFeatures(holes: [], pours: [], keepouts: [], cutouts: [])
    }

    func testPours() {
        var value = empty()
        let first = value.addPour(rect: CGRect(x: 0.2, y: 0.2, width: 10, height: 10), layer: .top, grid: 1, ground: true)
        XCTAssertEqual(value.pours[0].rect.origin, .zero)
        XCTAssertEqual(value.pours[0].net, .ground)
        XCTAssertEqual(value.pours[0].clearance, BoardPour.startClearance)
        value.setPourNet(first, .rail(mv: 5000))
        value.setPourNet(first, .rail(mv: 5000))
        let second = value.addPour(rect: CGRect(x: 0, y: 0, width: 5, height: 5), layer: .bottom, grid: 1, ground: false)
        XCTAssertEqual(value.pours[1].net, .rail(mv: 5000), "the last pour's settings")
        _ = value.addPour(rect: CGRect(x: 0, y: 0, width: 5, height: 5), layer: .bottom, grid: 1, ground: false)
        var other = empty()
        _ = other.addPour(rect: CGRect(x: 0, y: 0, width: 5, height: 5), layer: .top, grid: 1, ground: false)
        XCTAssertNil(other.pours[0].net)
        value.movePour(second, to: CGPoint(x: 3.4, y: 3.6), grid: 1)
        XCTAssertEqual(value.pours[1].rect.origin, CGPoint(x: 3, y: 4))
        value.setPourRect(second, CGRect(x: 0, y: 0, width: 20, height: 20), grid: 1)
        XCTAssertEqual(value.pours[1].rect.size, CGSize(width: 20, height: 20))
        value.setPourLayer(second, .top)
        value.setPourClearance(second, 1_000)
        XCTAssertEqual(value.pours[1].clearance, LayerStyle.clearanceRange.upperBound)
        value.setPourClearance(second, nil)
        value.setPourConnection(second, .solid)
        value.setPourThermalGap(second, 1_000)
        value.setPourSpokeWidth(second, 1_000)
        value.setPourMinWidth(second, 1_000)
        XCTAssertEqual(value.pours[1].thermalGap, BoardPour.gapRange.upperBound)
        XCTAssertEqual(value.pours[1].spokeWidth, BoardPour.spokeRange.upperBound)
        XCTAssertEqual(value.pours[1].minWidth, BoardPour.minWidthRange.upperBound)
        value.setPourNet(UUID(), nil)
        value.setLocked(.pour(second), true)
        value.setLocked(.pour(UUID()), true)
        value.movePour(second, to: CGPoint(x: 50, y: 50), grid: 1)
        value.setPourRect(second, CGRect(x: 0, y: 0, width: 2, height: 2), grid: 1)
        XCTAssertEqual(value.pours[1].rect.origin, CGPoint(x: 0, y: 0), "locked")
        XCTAssertEqual(BoardFeatures.snapped(CGRect(x: 10, y: 10, width: -10, height: -10), grid: 1).origin, .zero)
    }

    func testKeepoutsAndCutouts() {
        var value = empty()
        let keepout = value.addKeepout(rect: CGRect(x: 0, y: 0, width: 10, height: 10), layers: nil, grid: 1)
        value.moveKeepout(keepout, to: CGPoint(x: 2, y: 2), grid: 1)
        value.moveKeepout(keepout, to: CGPoint(x: 2, y: 2), grid: 1)
        XCTAssertEqual(value.keepouts[0].rect.origin, CGPoint(x: 2, y: 2))
        value.setKeepoutRect(keepout, CGRect(x: 0, y: 0, width: 4, height: 4), grid: 1)
        value.setKeepoutLayers(keepout, [.top])
        XCTAssertEqual(value.keepouts[0].layers, [.top])
        value.setKeepoutLayers(UUID(), nil)
        value.setLocked(.keepout(keepout), true)
        value.setLocked(.keepout(UUID()), true)
        value.moveKeepout(keepout, to: CGPoint(x: 9, y: 9), grid: 1)
        XCTAssertEqual(value.keepouts[0].rect.origin, .zero, "locked")

        let circle = value.addCutout(.rounded(rect: CGRect(x: 0.4, y: 0, width: 5.4, height: 5.4), radius: 2.7), grid: 1)
        guard case let .rounded(box, radius) = value.cutouts[0].shape else { return XCTFail() }
        XCTAssertEqual(box.width, box.height, "a circle stays square")
        XCTAssertEqual(radius, box.width / 2)
        let square = value.addCutout(.rounded(rect: CGRect(x: 0, y: 0, width: 4, height: 6), radius: 0), grid: 1)
        let polygon = value.addCutout(.polygon([.zero, CGPoint(x: 0, y: 5), CGPoint(x: 5, y: 0)]), grid: 1)
        value.moveCutout(polygon, to: CGPoint(x: 10, y: 10), grid: 1)
        XCTAssertEqual(value.cutouts[2].shape.bounds.origin, CGPoint(x: 10, y: 10))
        value.setCutoutRect(polygon, CGRect(x: 0, y: 0, width: 5, height: 5), grid: 1)
        value.setCutoutRadius(polygon, 1)
        value.setCutoutRect(square, CGRect(x: 0, y: 0, width: 8, height: 8), grid: 1)
        value.setCutoutRect(circle, CGRect(x: 0, y: 0, width: 8, height: 8), grid: 1)
        value.setCutoutRadius(square, 1)
        guard case let .rounded(_, r) = value.cutouts[1].shape else { return XCTFail() }
        XCTAssertEqual(r, 1)
        value.setCutoutRadius(square, 1)
        value.setLocked(.cutout(square), true)
        value.setLocked(.cutout(UUID()), true)
        value.setCutoutRadius(square, 2)
        guard case let .rounded(_, still) = value.cutouts[1].shape else { return XCTFail() }
        XCTAssertEqual(still, 1, "locked")
    }

    func testHoles() {
        var value = empty()
        let first = value.addHole(at: CGPoint(x: 5.2, y: 5.2), grid: 1)
        XCTAssertEqual(value.holes[0].centre, CGPoint(x: 5, y: 5))
        XCTAssertFalse(value.holes[0].plated)
        value.setHolePlated(first, true)
        value.setHoleNet(first, .ground)
        value.setHoleSize(first, .m4)
        let second = value.addHole(at: .zero, grid: 1)
        XCTAssertEqual(value.holes[1].size, .m4, "the last hole's size")
        XCTAssertTrue(value.holes[1].plated)
        XCTAssertEqual(value.holes[1].net, .ground)
        XCTAssertEqual(value.hole(at: CGPoint(x: 5.5, y: 5))?.id, first)
        XCTAssertNil(value.hole(at: CGPoint(x: 50, y: 50)))
        value.setHoleDrill(second, 1_000)
        XCTAssertEqual(value.holes[1].size, .custom)
        XCTAssertEqual(value.holes[1].drill, BoardHole.drillRange.upperBound)
        value.setHoleDrill(second, 1_000)
        value.setHoleDiameter(second, 0)
        value.setHoleDiameter(second, 0)
        value.setHoleSize(first, .m4)
        value.setHoleDiameter(first, 1_000)
        XCTAssertEqual(value.holes[0].size, .custom)
        value.setHoleCentre(first, CGPoint(x: 7, y: 7))
        value.setHoleCentre(first, CGPoint(x: 7, y: 7))
        value.setLocked(.hole(first), true)
        value.setLocked(.hole(UUID()), true)
        value.setLocked(.trace(UUID()), true)
        value.setHoleCentre(first, .zero)
        XCTAssertEqual(value.holes[0].centre, CGPoint(x: 7, y: 7), "locked")
        value.setHoleNet(UUID(), nil)
    }

    func testRemovalGroupMovesAndHits() {
        var value = empty()
        let pour = value.addPour(rect: CGRect(x: 0, y: 0, width: 10, height: 10), layer: .top, grid: 1, ground: false)
        let keepout = value.addKeepout(rect: CGRect(x: 20, y: 0, width: 10, height: 10), layers: [.bottom], grid: 1)
        let cutout = value.addCutout(.rounded(rect: CGRect(x: 40, y: 0, width: 10, height: 10), radius: 0), grid: 1)
        let hole = value.addHole(at: CGPoint(x: 60, y: 5), grid: 1)
        var base = BoardGroupBase()
        base.pours = [pour: value.pours[0].rect, UUID(): .zero]
        base.keepouts = [keepout: value.keepouts[0].rect, UUID(): .zero]
        base.cutouts = [cutout: value.cutouts[0].shape]
        base.holes = [hole: value.holes[0].centre, UUID(): .zero]
        var moved = value
        moved.moveGroup(base, by: CGSize(width: 1, height: 0))
        XCTAssertEqual(moved.pours[0].rect.minX, 1)
        XCTAssertEqual(moved.keepouts[0].rect.minX, 21)
        XCTAssertEqual(moved.cutouts[0].shape.bounds.minX, 41)
        XCTAssertEqual(moved.holes[0].centre.x, 61)
        moved.moveGroup(base, by: CGSize(width: 1, height: 0))
        var unmoved = value
        unmoved.moveGroup(BoardGroupBase(), by: CGSize(width: 1, height: 0))
        XCTAssertEqual(unmoved.pours, value.pours)

        let layers: [CopperLayer] = [.top, .bottom]
        let all = BoardVisibility()
        XCTAssertEqual(value.item(at: CGPoint(x: 0, y: 5), tolerance: 0.5, visible: all, layers: layers), .pour(pour))
        XCTAssertNil(value.item(at: CGPoint(x: 5, y: 5), tolerance: 0.5, visible: all, layers: layers), "inside, not on the edge")
        XCTAssertNil(value.item(at: CGPoint(x: 0, y: 5), tolerance: 0.5, visible: all, layers: [.bottom]))
        XCTAssertEqual(value.item(at: CGPoint(x: 20, y: 5), tolerance: 0.5, visible: all, layers: layers), .keepout(keepout))
        XCTAssertNil(value.item(at: CGPoint(x: 20, y: 5), tolerance: 0.5, visible: all, layers: [.top]))
        XCTAssertEqual(value.item(at: CGPoint(x: 40, y: 5), tolerance: 0.5, visible: all, layers: layers), .cutout(cutout))
        XCTAssertTrue(BoardFeatures.keepoutIsShown(BoardKeepout(id: UUID(), rect: .zero, layers: nil), all, layers: layers))
        XCTAssertEqual(BoardFeatures.distance(from: CGPoint(x: -3, y: -4), toBoundaryOf: CGRect(x: 0, y: 0, width: 10, height: 10)), 5)
        XCTAssertEqual(BoardFeatures.distance(from: CGPoint(x: 2, y: 5), toBoundaryOf: CGRect(x: 0, y: 0, width: 10, height: 10)), 2)

        value.remove(holes: [hole], pours: [pour], keepouts: [keepout], cutouts: [cutout])
        XCTAssertTrue(value.holes.isEmpty && value.pours.isEmpty && value.keepouts.isEmpty && value.cutouts.isEmpty)
    }
}

/// The layout keeps each array observed on its own (plan-68-E), as when the
/// document stored them.
@MainActor
final class BoardObservationTests: XCTestCase {
    private func invalidates(_ read: @escaping () -> Void, by change: () -> Void) -> Bool {
        var fired = false
        withObservationTracking(read) { fired = true }
        change()
        return fired
    }

    func testEditsInvalidateOnlyTheirOwnArrays() {
        let doc = TestSupport.newDocument()
        doc.addHole(at: .zero)
        XCTAssertFalse(invalidates({ _ = doc.vias }, by: { doc.addTrace(points: [.zero, CGPoint(x: 10, y: 0)], layer: .top) }))
        XCTAssertEqual(doc.traces.count, 1)
        XCTAssertFalse(invalidates({ _ = doc.pours }, by: { doc.setHoleNet(doc.holes[0].id, .ground) }))
        XCTAssertFalse(invalidates({ _ = doc.texts }, by: { doc.setTraceWidth(doc.traces[0].id, 30) }))
        XCTAssertTrue(invalidates({ _ = doc.traces }, by: { doc.setTraceWidth(doc.traces[0].id, 40) }))
    }

    func testUpdatesWriteBackOnlyWhatChanged() {
        let layout = BoardContent()
        let doc = TestSupport.newDocument()
        doc.edit(.pcb, "Test") { scope in
            let before = layout.copperRevision
            layout.updateSettings(scope) { _ in }
            layout.updateTexts(scope) { _ in }
            layout.updateCopper(scope) { _ in }
            layout.updateFeatures(scope) { _ in }
            XCTAssertEqual(layout.copperRevision, before)
            layout.updateFeatures(scope) { $0.keepouts.append(BoardKeepout(id: UUID(), rect: .zero, layers: nil)) }
            layout.updateTexts(scope) { _ = $0.add(at: .zero, grid: 1) }
            layout.restore(settings: layout.settings, texts: [], traces: [], vias: [], holes: [], pours: [],
                           keepouts: [], cutouts: [], scope)
            XCTAssertTrue(layout.keepouts.isEmpty && layout.texts.isEmpty)
            layout.restore(settings: BoardSettings(), texts: [BoardText(id: UUID(), text: "A", anchor: .zero, size: 40)],
                           traces: [BoardTrace(id: UUID(), layer: .top, width: 10, points: [.zero, CGPoint(x: 1, y: 0)])],
                           vias: [BoardVia(id: UUID(), centre: .zero, drill: 0.3, diameter: 0.6)],
                           holes: [BoardHole(id: UUID(), centre: .zero)],
                           pours: [BoardPour(id: UUID(), rect: CGRect(x: 0, y: 0, width: 5, height: 5), layer: .top)],
                           keepouts: [], cutouts: [BoardCutout(id: UUID(), shape: .polygon([.zero, CGPoint(x: 1, y: 0),
                                                                                            CGPoint(x: 0, y: 1)]))], scope)
            XCTAssertEqual(layout.traces.count, 1)
        }
    }
}
