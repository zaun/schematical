import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-D phase 1: the trace value, 45° routing and the file format.
@MainActor
final class TraceRouteTests: XCTestCase {
    private let a = CGPoint(x: 10, y: 10)

    /// Every octant, straight first and diagonal first: legal, from a to b,
    /// and the expected bend.
    func testRouteInEveryOctant() {
        // (dx, dy) with |dx| != |dy| and neither 0: one per octant.
        let offsets: [(CGFloat, CGFloat)] = [(5, 2), (2, 5), (-2, 5), (-5, 2), (-5, -2), (-2, -5), (2, -5), (5, -2)]
        for (dx, dy) in offsets {
            let b = CGPoint(x: a.x + dx, y: a.y + dy)
            let d = min(abs(dx), abs(dy))
            let sx: CGFloat = dx < 0 ? -1 : 1, sy: CGFloat = dy < 0 ? -1 : 1

            let straight = TraceRoute.route(from: a, to: b, diagonalFirst: false)
            XCTAssertEqual(straight.count, 3, "\(dx), \(dy)")
            XCTAssertTrue(TraceRoute.isLegal(straight), "\(straight)")
            XCTAssertEqual(straight.first, a)
            XCTAssertEqual(straight.last, b)
            let expectedStraight = abs(dx) > abs(dy)
                ? CGPoint(x: a.x + sx * (abs(dx) - d), y: a.y)
                : CGPoint(x: a.x, y: a.y + sy * (abs(dy) - d))
            XCTAssertEqual(straight[1], expectedStraight, "\(dx), \(dy)")

            let diagonal = TraceRoute.route(from: a, to: b, diagonalFirst: true)
            XCTAssertEqual(diagonal.count, 3)
            XCTAssertTrue(TraceRoute.isLegal(diagonal), "\(diagonal)")
            XCTAssertEqual(diagonal[1], CGPoint(x: a.x + sx * d, y: a.y + sy * d), "\(dx), \(dy)")
            XCTAssertEqual(diagonal.last, b)
        }
    }

    func testAlready45PairIsTwoPoints() {
        for (dx, dy) in [(5.0, 0.0), (0.0, -3.0), (4.0, 4.0), (-2.5, 2.5)] as [(CGFloat, CGFloat)] {
            let b = CGPoint(x: a.x + dx, y: a.y + dy)
            XCTAssertEqual(TraceRoute.route(from: a, to: b, diagonalFirst: false), [a, b])
            XCTAssertEqual(TraceRoute.route(from: a, to: b, diagonalFirst: true), [a, b])
        }
    }

    func testIsLegalRejectsBadSegments() {
        XCTAssertFalse(TraceRoute.isLegal([a, CGPoint(x: a.x + 10 * cos(.pi / 6), y: a.y + 10 * sin(.pi / 6))]),
                       "30°")
        XCTAssertFalse(TraceRoute.isLegal([a, a]), "zero length")
        XCTAssertFalse(TraceRoute.isLegal([a]), "one point")
        XCTAssertTrue(TraceRoute.isLegal([a, CGPoint(x: 15, y: 10), CGPoint(x: 18, y: 13)]))
        XCTAssertEqual(TraceRoute.clean([a, a, CGPoint(x: 15, y: 10), CGPoint(x: 15, y: 10)]),
                       [a, CGPoint(x: 15, y: 10)])
    }

    func testTwoTracesRoundTrip() throws {
        let doc = TestSupport.newDocument()
        var board = BoardSettings()
        board.layers = 4
        board.styles[.inner(3)] = LayerStyle(color: LayerStyle.defaultStyle(of: .inner(3)).color, type: .wire)
        let one = BoardTrace(id: UUID(), layer: .top, width: 10,
                             points: [CGPoint(x: 2.54, y: 5.08), CGPoint(x: 7.62, y: 5.08), CGPoint(x: 10.16, y: 7.62)])
        // Off-grid points are kept as saved.
        let two = BoardTrace(id: UUID(), layer: .inner(3), width: 50,
                             points: [CGPoint(x: 1.2345, y: 1.2345), CGPoint(x: 3.2345, y: 3.2345)])
        doc.replaceContents(chips: [], wires: [], junctions: [], buses: [], nextReference: 1, board: board,
                            texts: [], traces: [one, two])
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.traces, [one, two])

        // Written as [] when empty.
        let empty = TestSupport.newDocument()
        let file = try SchematicFile.decode(SchematicFile(document: empty, fileURL: nil).encoded())
        XCTAssertEqual(file.board?.traces, [])
    }

    func testEveryErrorDetail() {
        let id = UUID()
        func board(layers: Int = 4, layer: String = "top", width: String = "10",
                   points: String = #"[ { "x": 0, "y": 0 }, { "x": 1, "y": 0 } ]"#) -> String {
            """
            { "layers": \(layers), "layerStyles": [ { "layer": "inner3", "type": "wire" } ],
              "traces": [ { "id": "\(id)", "layer": "\(layer)", "width": \(width), "points": \(points) } ] }
            """
        }
        let name = "trace \(id.uuidString)"
        let cases: [(String, String)] = [
            (board(layer: "inner4"), "\(name) layer \"inner4\" is not on a 4-layer board"),
            (board(layer: "middle"), "\(name) layer \"middle\" must be top, inner2 to inner5, or bottom"),
            (board(layer: "inner2"), "\(name) layer inner2 is a plane"),
            (board(width: "3"), "\(name) width 3 must be 5 to 100 mil"),
            (board(points: #"[ { "x": 0, "y": 0 } ]"#), "\(name) has fewer than 2 points"),
            (board(points: #"[ { "x": 0, "y": 0 }, { "x": 1, "y": 0 }, { "x": 1, "y": 0 } ]"#),
             "\(name) segment 2 has no length"),
            (board(points: #"[ { "x": 0, "y": 0 }, { "x": 1, "y": 0 }, { "x": 3, "y": 1 } ]"#),
             "\(name) segment 2 is not at a multiple of 45°"),
        ]
        for (json, message) in cases {
            XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: json) }, message, json)
        }
        // A Wire inner layer takes traces.
        XCTAssertNoThrow(try BoardLayersTests.open(board: board(layer: "inner3")))
    }

    func testTraceIDSharedWithTextIsRefused() {
        let id = UUID()
        let json = """
        { "layers": 2, "texts": [ { "id": "\(id)", "text": "A", "x": 0, "y": 0 } ],
          "traces": [ { "id": "\(id)", "layer": "top", "width": 10, "points": [ { "x": 0, "y": 0 }, { "x": 1, "y": 1 } ] } ] }
        """
        XCTAssertThrowsError(try BoardLayersTests.open(board: json)) { error in
            guard case let SchematicFileError.duplicateID(dup) = error else { return XCTFail("\(error)") }
            XCTAssertEqual(dup, id)
        }
    }
}
