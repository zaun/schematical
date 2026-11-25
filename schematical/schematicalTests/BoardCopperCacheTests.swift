import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB page keeps the board's connectivity, ratsnest and placed pads
/// between redraws (`SchematicDocument.copperRevision`, `chipPads`). Kept
/// results must equal a fresh build after every kind of edit, undo included.
@MainActor
final class BoardCopperCacheTests: XCTestCase {
    /// The kept connectivity, ratsnest and pads against ones built afresh.
    private func assertFresh(_ doc: SchematicDocument, _ step: String, file: StaticString = #filePath,
                             line: UInt = #line) {
        // Leaving out a trace that doesn't exist builds the same board anew.
        let fresh = doc.connectivity(excludingTraces: [UUID()])
        let kept = doc.connectivity()
        XCTAssertEqual(kept.root, fresh.root, step, file: file, line: line)
        XCTAssertEqual(kept.netOfPad, fresh.netOfPad, step, file: file, line: line)
        XCTAssertEqual(kept.traces.map(\.id), fresh.traces.map(\.id), step, file: file, line: line)
        XCTAssertEqual(kept.vias.map(\.id), fresh.vias.map(\.id), step, file: file, line: line)
        XCTAssertEqual(kept.pads.map(\.pad), fresh.pads.map(\.pad), step, file: file, line: line)
        XCTAssertEqual(kept.pourFills.mapValues(\.pieces.count), fresh.pourFills.mapValues(\.pieces.count), step,
                       file: file, line: line)
        let lines = doc.ratsnest(), freshLines = doc.ratsnest(in: fresh)
        XCTAssertEqual(lines.map(\.0), freshLines.map(\.0), step, file: file, line: line)
        XCTAssertEqual(lines.map(\.1), freshLines.map(\.1), step, file: file, line: line)
        XCTAssertEqual(doc.chipPads, doc.chips.map(\.boardPads), step, file: file, line: line)
    }

    func testKeptResultsFollowEveryEdit() throws {
        let doc = try Fixture.open("AFU")
        assertFresh(doc, "opened")
        let before = doc.ratsnest().count

        // A pour over the whole board, so every later edit is checked with it present.
        XCTAssertNotNil(doc.addPour(rect: CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height), layer: .top))
        assertFresh(doc, "pour added")

        // A footprint moved well away: its pads move, its traces let go.
        let chip = try XCTUnwrap(doc.chips.first { $0.board != nil && !$0.boardPads.isEmpty })
        let centre = try XCTUnwrap(chip.board?.centre)
        doc.moveFootprint(chip.id, to: CGPoint(x: centre.x + 12.7, y: centre.y + 12.7))
        assertFresh(doc, "footprint moved")

        // A trace and a via added.
        XCTAssertNotNil(doc.addTrace(points: [CGPoint(x: 1.27, y: 1.27), CGPoint(x: 6.35, y: 1.27)], layer: .top))
        assertFresh(doc, "trace added")
        _ = doc.addVia(at: CGPoint(x: 6.35, y: 1.27))
        assertFresh(doc, "via added")

        // Undone, one step at a time, back to the opened board.
        while doc.undo() { assertFresh(doc, "undone") }
        XCTAssertEqual(doc.ratsnest().count, before)
    }

    /// The same result twice is one build: the second is the kept one.
    func testUnchangedBoardIsBuiltOnce() throws {
        let doc = try Fixture.open("AFU")
        let revision = doc.copperRevision
        _ = doc.connectivity()
        _ = doc.ratsnest()
        XCTAssertEqual(doc.keptConnectivity?.revision, revision)
        XCTAssertEqual(doc.keptRatsnest?.revision, revision)
        XCTAssertEqual(doc.copperRevision, revision, "reading must not count as a change")
    }

    /// The spanning lines are the shortest joins, first group first on ties.
    func testSpanningLinesJoinNearestGroups() {
        let groups: [[CGPoint]] = [
            [CGPoint(x: 0, y: 0)],
            [CGPoint(x: 10, y: 0), CGPoint(x: 3, y: 0)],
            [CGPoint(x: 0, y: 2)],
            [CGPoint(x: 4, y: 0)],
        ]
        let lines = SchematicDocument.spanningLines(groups)
        XCTAssertEqual(lines.map(\.0), [CGPoint(x: 0, y: 0), CGPoint(x: 0, y: 0), CGPoint(x: 3, y: 0)])
        XCTAssertEqual(lines.map(\.1), [CGPoint(x: 0, y: 2), CGPoint(x: 3, y: 0), CGPoint(x: 4, y: 0)])
    }
}
