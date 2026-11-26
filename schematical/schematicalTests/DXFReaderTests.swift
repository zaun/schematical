import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The DXF reader and its flatteners (plan-65-H Phase 1).
final class DXFReaderTests: XCTestCase {
    private func parse(_ text: String) throws -> DXFDrawing { try DXFReader.parse(text) }

    private func counts(_ drawing: DXFDrawing) -> [String: Int] {
        Dictionary(drawing.entities.map { ($0.typeName, 1) }, uniquingKeysWith: +)
    }

    // MARK: The fixtures

    func testEveryFixtureParsesToTheEntitiesItHolds() throws {
        let chain = try parse(DXFFixtures.chainBoard)
        XCTAssertEqual(counts(chain), ["LINE": 4, "ARC": 1])
        XCTAssertEqual(chain.units, .millimetre)

        let curves = try parse(DXFFixtures.curves)
        XCTAssertEqual(counts(curves), ["SPLINE": 1, "ELLIPSE": 1])

        let inch = try parse(DXFFixtures.inchBoard)
        XCTAssertEqual(counts(inch), ["POLYLINE": 1, "CIRCLE": 1])
        XCTAssertEqual(inch.units, .inch)
        XCTAssertEqual(inch.units?.millimetres, 25.4)

        let rounded = try parse(DXFFixtures.roundedBoard)
        XCTAssertEqual(counts(rounded), ["POLYLINE": 2, "CIRCLE": 1], "an LWPOLYLINE is a polyline entity")
        XCTAssertEqual(rounded.skipped, ["TEXT": 1])
        XCTAssertEqual(Set(rounded.layers), ["Edge"])

        let open = try parse(DXFFixtures.openChain)
        XCTAssertEqual(open.unitsCode, 0)
        XCTAssertNil(open.units)
        XCTAssertEqual(counts(open), ["POLYLINE": 1, "LINE": 2])

        let kicad = try parse(DXFFixtures.kicadEdgeProbe)
        XCTAssertEqual(counts(kicad), ["LINE": 5, "CIRCLE": 1, "ARC": 1])
        XCTAssertEqual(kicad.units, .millimetre)
        XCTAssertTrue(kicad.skipped.isEmpty)
    }

    func testTheClosedFlagOfAPolylineIsRead() throws {
        let drawing = try parse(DXFFixtures.roundedBoard)
        let closed = drawing.entities.compactMap { entity -> Bool? in
            if case let .polyline(_, _, closed) = entity { closed } else { nil }
        }
        XCTAssertEqual(closed, [true, true])
    }

    // MARK: Errors

    func testBinaryDXFIsRefused() {
        XCTAssertThrowsError(try DXFReader.parse(data: Data("AutoCAD Binary DXF\r\n\u{1A}\u{0}".utf8))) {
            XCTAssertEqual($0 as? DXFError, .binary)
            XCTAssertEqual(($0 as? DXFError)?.errorDescription, "Binary DXF isn't supported; save the drawing as ASCII DXF.")
        }
    }

    func testAnOddNumberOfLinesIsTruncated() {
        XCTAssertThrowsError(try parse("  0\nSECTION\n  2\nENTITIES\n  0")) { XCTAssertEqual($0 as? DXFError, .truncated) }
    }

    func testAFileWithNoEntitiesSectionIsRefused() {
        XCTAssertThrowsError(try parse("  0\nSECTION\n  2\nHEADER\n  0\nENDSEC\n  0\nEOF\n")) {
            XCTAssertEqual($0 as? DXFError, .noEntities)
        }
        XCTAssertThrowsError(try parse("hello world\nthis is not a drawing\n")) { XCTAssertEqual($0 as? DXFError, .truncated) }
    }

    func testALatin1LayerNameReads() throws {
        let text = "  0\nSECTION\n  2\nENTITIES\n  0\nLINE\n  8\nKontur\u{E4}\n 10\n0\n 20\n0\n 11\n5\n 21\n0\n  0\nENDSEC\n  0\nEOF\n"
        let data = try XCTUnwrap(text.data(using: .isoLatin1))
        XCTAssertNil(String(data: data, encoding: .utf8), "not UTF-8")
        let drawing = try DXFReader.parse(data: data)
        XCTAssertEqual(drawing.layers, ["Kontur\u{E4}"])
    }

    func testWindowsLineEndingsAndAMissingLayerRead() throws {
        let text = "  0\r\nSECTION\r\n  2\r\nENTITIES\r\n  0\r\nCIRCLE\r\n 10\r\n1\r\n 20\r\n2\r\n 40\r\n3\r\n  0\r\nENDSEC\r\n  0\r\nEOF\r\n"
        let drawing = try parse(text)
        XCTAssertEqual(drawing.entities, [.circle(layer: "0", centre: CGPoint(x: 1, y: 2), radius: 3)])
    }

    // MARK: Flatteners

    private func distanceFromOrigin(_ points: [CGPoint]) -> [Double] { points.map { Double(hypot($0.x, $0.y)) } }

    func testAQuarterCircleBulgeFlattensWithinTheFlatness() {
        let vertices = [DXFVertex(point: CGPoint(x: 10, y: 0), bulge: tan(Double.pi / 8)), DXFVertex(point: CGPoint(x: 0, y: 10))]
        let points = DXFLoops.polylinePoints(vertices, closed: false)
        XCTAssertGreaterThan(points.count, 8)
        for distance in distanceFromOrigin(points) { XCTAssertEqual(distance, 10, accuracy: 1e-9, "on the arc") }
        for (a, b) in zip(points, points.dropFirst()) {
            let mid = CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2)
            XCTAssertLessThanOrEqual(10 - Double(hypot(mid.x, mid.y)), BoardCutout.flatness + 1e-9)
        }
        XCTAssertEqual(points.first, CGPoint(x: 10, y: 0))
        XCTAssertEqual(Double(points.last!.x), 0, accuracy: 1e-9)
        XCTAssertEqual(Double(points.last!.y), 10, accuracy: 1e-9)
    }

    func testANegativeBulgeGoesTheOtherWay() {
        // Clockwise from (10, 0) to (0, 10): the arc bulges toward the origin's far side, round (10, 10).
        let vertices = [DXFVertex(point: CGPoint(x: 10, y: 0), bulge: -tan(Double.pi / 8)), DXFVertex(point: CGPoint(x: 0, y: 10))]
        let points = DXFLoops.polylinePoints(vertices, closed: false)
        for point in points { XCTAssertEqual(Double(hypot(point.x - 10, point.y - 10)), 10, accuracy: 1e-9) }
    }

    func testAnArcEntityRunsCounterClockwise() {
        let result = DXFLoops.flatten(.arc(layer: "0", centre: CGPoint(x: 5, y: 5), radius: 2, start: 270, end: 90), scale: 1)
        XCTAssertFalse(result.closed)
        XCTAssertEqual(Double(result.points.first!.x), 5, accuracy: 1e-9)
        XCTAssertEqual(Double(result.points.first!.y), 3, accuracy: 1e-9)
        XCTAssertEqual(Double(result.points.last!.y), 7, accuracy: 1e-9)
        XCTAssertGreaterThan(result.points.map(\.x).max() ?? 0, 6.99, "through the right side")
    }

    func testARationalQuarterCircleSplineIsOnTheCircle() {
        let spline = DXFSpline(degree: 2, knots: [0, 0, 0, 1, 1, 1],
                               controls: [CGPoint(x: 1, y: 0), CGPoint(x: 1, y: 1), CGPoint(x: 0, y: 1)],
                               weights: [1, 2.0.squareRoot() / 2, 1])
        let result = DXFLoops.splinePoints(spline, scale: 1)
        XCTAssertGreaterThan(result.points.count, 4)
        for distance in distanceFromOrigin(result.points) { XCTAssertEqual(distance, 1, accuracy: 1e-9) }
        XCTAssertEqual(result.points.first, CGPoint(x: 1, y: 0))
        XCTAssertEqual(Double(result.points.last!.y), 1, accuracy: 1e-9)
    }

    func testAnEllipsesAreaIsPiAB() {
        let ellipse = DXFEntity.ellipse(layer: "0", centre: .zero, major: CGPoint(x: 5, y: 0), ratio: 0.6, start: 0, end: 2 * Double.pi)
        let result = DXFLoops.flatten(ellipse, scale: 1)
        XCTAssertTrue(result.closed)
        let area = abs(BoardGeometry.area(result.points))
        XCTAssertEqual(area, Double.pi * 5 * 3, accuracy: Double.pi * 15 * 0.01)
    }

    func testScaleTurnsInchesIntoMillimetres() {
        let result = DXFLoops.flatten(.line(layer: "0", from: .zero, to: CGPoint(x: 2, y: 1)), scale: 25.4)
        XCTAssertEqual(result.points.last, CGPoint(x: 50.8, y: 25.4))
        XCTAssertEqual(DXFUnits.allCases.map(\.millimetres), [25.4, 304.8, 1, 10, 1000])
        XCTAssertNil(DXFUnits(code: 0))
        XCTAssertNil(DXFUnits(code: nil))
        XCTAssertNil(DXFUnits(code: 3))
    }

    // MARK: plan-68-Y

    func testASplineWithTooFewControlsIsItsFitPoints() {
        let spline = DXFSpline(degree: 3, controls: [CGPoint(x: 1, y: 1)],
                               fit: [CGPoint(x: 0, y: 0), CGPoint(x: 1, y: 2), CGPoint(x: 3, y: 1)], closed: true)
        let result = DXFLoops.splinePoints(spline, scale: 2)
        XCTAssertEqual(result.points, [CGPoint(x: 0, y: 0), CGPoint(x: 2, y: 4), CGPoint(x: 6, y: 2)])
        XCTAssertTrue(result.closed)
    }

    func testASplineWithNoKnotVectorIsClampedAndUniform() {
        let controls = [CGPoint(x: 0, y: 0), CGPoint(x: 1, y: 2), CGPoint(x: 3, y: 2), CGPoint(x: 4, y: 0), CGPoint(x: 6, y: 1)]
        let bare = DXFLoops.splinePoints(DXFSpline(degree: 3, knots: [], controls: controls), scale: 1)
        let clamped = DXFLoops.splinePoints(DXFSpline(degree: 3, knots: [0, 0, 0, 0, 1, 2, 2, 2, 2], controls: controls), scale: 1)
        XCTAssertEqual(bare.points, clamped.points, "the knots made up are the clamped uniform ones")
        XCTAssertEqual(bare.points.first, controls.first)
        XCTAssertEqual(Double(bare.points.last!.x), 6, accuracy: 1e-9)
        XCTAssertGreaterThan(bare.points.count, controls.count)
    }

    func testAStrayVertexOrSequenceEndIsNotCountedAsSkipped() throws {
        let text = "  0\nSECTION\n  2\nENTITIES\n  0\nVERTEX\n  8\nEdge\n  0\nSEQEND\n  0\nCIRCLE\n 10\n1\n 20\n2\n 40\n3\n"
            + "  0\nENDSEC\n  0\nEOF\n"
        let drawing = try parse(text)
        XCTAssertTrue(drawing.skipped.isEmpty, "\(drawing.skipped)")
        XCTAssertEqual(drawing.entities, [.circle(layer: "0", centre: CGPoint(x: 1, y: 2), radius: 3)])
    }
}
