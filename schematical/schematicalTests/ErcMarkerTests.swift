import CoreGraphics
import SwiftUI
import Foundation
import XCTest
@testable import schematical

/// plan-63-F phase 2: the marks the Electrical Rules Check puts on the schematic.
@MainActor
final class ErcMarkerTests: XCTestCase {
    private func model(_ doc: SchematicDocument) -> ErcResultsModel {
        let model = ErcResultsModel(document: doc, library: TestSupport.catalog.library)
        model.run()
        return model
    }

    func testMarkersFollowTheResultsAndGoWithTheWindow() throws {
        let doc = try ErcCases.pinNotConnected()
        XCTAssertTrue(doc.ercMarkers.isEmpty)
        let results = model(doc)
        XCTAssertEqual(doc.ercMarkers.count, results.violations.count)
        XCTAssertEqual(Set(doc.ercMarkers.map(\.key)), Set(results.violations.map(\.key)))
        // An excluded violation has no mark.
        let first = try XCTUnwrap(results.violations.first)
        results.selection = [first.id]
        results.excludeSelected()
        XCTAssertEqual(doc.ercMarkers.count, results.violations.count)
        XCTAssertFalse(doc.ercMarkers.contains { $0.key == first.key })
        // Closing the window takes them off.
        results.clearMarkers()
        XCTAssertTrue(doc.ercMarkers.isEmpty)
        XCTAssertTrue(doc.ercSelectedKeys.isEmpty)
    }

    func testAMarkerIsOnItsPinAndColouredBySeverity() throws {
        let doc = try ErcCases.pinConflict()
        let results = model(doc)
        let conflict = try XCTUnwrap(results.violations.first { $0.rule == .pinToPin })
        let marker = try XCTUnwrap(doc.ercMarkers.first { $0.key == conflict.key })
        XCTAssertEqual(marker.point, conflict.point)
        XCTAssertEqual(marker.severity, .error)
        let slot = try XCTUnwrap(doc.chips.last?.layout.slots.first { $0.pin.name == "1Y" })
        XCTAssertEqual(marker.point, doc.chips.last?.tip(of: slot), "on the later pin's tip")
    }

    func testSelectingALineMarksItSelectedAndAsksForFocus() throws {
        let doc = try ErcCases.pinConflict()
        let results = model(doc)
        let line = try XCTUnwrap(results.violations.first)
        XCTAssertNil(doc.ercFocus)
        results.select([line.id])
        XCTAssertEqual(doc.ercSelectedKeys, [line.key])
        XCTAssertEqual(doc.ercFocus?.point, line.point)
        let serial = doc.ercFocus?.serial
        results.select([line.id])
        XCTAssertNotEqual(doc.ercFocus?.serial, serial, "a second request for one point is a new one")
    }

    func testTheViewportCentresAndRevealsAPoint() {
        let viewport = CanvasViewport()
        viewport.updateSize(CGSize(width: 800, height: 600))
        let far = CGPoint(x: 5000, y: 3000)
        viewport.centre(on: far)
        let middle = viewport.world(CGPoint(x: 400, y: 300))
        XCTAssertEqual(middle.x, far.x, accuracy: 0.001)
        XCTAssertEqual(middle.y, far.y, accuracy: 0.001)

        // A point already well inside the view moves nothing.
        let before = viewport.offset
        viewport.reveal(CGPoint(x: far.x + 10, y: far.y - 10))
        XCTAssertEqual(viewport.offset, before)
        // One outside it is centred.
        let outside = CGPoint(x: 100, y: 100)
        viewport.reveal(outside)
        let centred = viewport.world(CGPoint(x: 400, y: 300))
        XCTAssertEqual(centred.x, outside.x, accuracy: 0.001)
        XCTAssertEqual(centred.y, outside.y, accuracy: 0.001)
        // A point near the edge (inside the margin) is brought in.
        let edge = CGPoint(x: viewport.visibleWorldRect.maxX - 5, y: outside.y)
        viewport.reveal(edge)
        XCTAssertEqual(viewport.world(CGPoint(x: 400, y: 300)).x, edge.x, accuracy: 0.001)
    }

    func testTheMarkersNeverReachAnExport() throws {
        let doc = try ErcCases.pinConflict()
        let plain = try XCTUnwrap(SchematicExport.svg(SchematicScene(doc)))
        _ = model(doc)
        XCTAssertFalse(doc.ercMarkers.isEmpty)
        let marked = try XCTUnwrap(SchematicExport.svg(SchematicScene(doc)))
        XCTAssertEqual(plain, marked, "the SVG export is the same with markers up")
    }

    func testMarkerTrianglesAreOnThePoint() {
        let point = CGPoint(x: 100, y: 200)
        let path = ErcMarkerLayer.triangle(at: point, size: 7)
        XCTAssertTrue(path.boundingRect.contains(point))
        XCTAssertEqual(path.boundingRect.midX, point.x, accuracy: 0.001)
    }
}
