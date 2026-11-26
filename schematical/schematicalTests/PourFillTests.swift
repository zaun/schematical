import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// The pour fill (plan-52-C): what `PourFiller.fill` keeps of a pour's
/// rectangle, with synthetic copper on a 100 × 80 mm board.
@MainActor
final class PourFillTests: XCTestCase {
    private let outline = [CGPoint(x: 0, y: 0), CGPoint(x: 100, y: 0), CGPoint(x: 100, y: 80), CGPoint(x: 0, y: 80)]
    private let ground = 1, other = 2

    private var rules: DesignRules {
        var rules = DesignRules.customDefault
        rules.minClearance = 0.3
        rules.minCopperToEdge = 0.5
        return rules
    }

    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func pour(_ rect: CGRect, layer: CopperLayer = .top, connection: PourConnection = .solid) -> BoardPour {
        var pour = BoardPour(id: UUID(), rect: rect, layer: layer)
        pour.connection = connection
        return pour
    }

    private func fill(_ pours: [BoardPour], nets: [Int?]? = nil, keepouts: [BoardKeepout] = [],
                      copper: [PourCopper]) -> [UUID: PourFill] {
        var map: [UUID: Int] = [:]
        for (i, pour) in pours.enumerated() {
            let net = nets.map { $0[i] } ?? ground
            if let net { map[pour.id] = net }
        }
        return PourFiller.fill(pours: pours, nets: map, keepouts: keepouts, copper: copper, board: BoardEdge(outline: outline),
                               edge: rules.minCopperToEdge, rules: rules)
    }

    /// A same-net pad in the middle of a 40 × 30 pour at (10, 10).
    private var anchor: PourCopper { .pad(0, rect: r(28, 23, 2, 2), net: ground) }

    // MARK: Phase 1

    func testWithNoOtherCopperTheFillIsTheRectangle() throws {
        let a = pour(r(10, 10, 40, 30))
        let result = try XCTUnwrap(fill([a], copper: [anchor])[a.id])
        XCTAssertEqual(result.pieces.count, 1)
        let box = result.path.boundingBoxOfPath
        XCTAssertEqual(box.minX, 10, accuracy: 1e-6)
        XCTAssertEqual(box.maxY, 40, accuracy: 1e-6)
        XCTAssertEqual(box.width, 40, accuracy: 1e-6)
    }

    func testOtherNetPadKeepsItsClearance() throws {
        let a = pour(r(10, 10, 40, 30))
        let pad = PourCopper.pad(1, rect: r(40, 20, 2, 2), net: other)
        let path = try XCTUnwrap(fill([a], copper: [anchor, pad])[a.id]).path
        XCTAssertTrue(path.contains(p(39.65, 21), using: .winding), "0.35 mm away: copper")
        XCTAssertFalse(path.contains(p(39.8, 21), using: .winding), "0.2 mm away: cleared")
        XCTAssertFalse(path.contains(p(41, 21), using: .winding), "inside the pad")
        XCTAssertTrue(path.contains(p(42.35, 21), using: .winding))
        XCTAssertFalse(path.contains(p(42.2, 21), using: .winding))
    }

    func testTraceAcrossSplitsThePour() throws {
        let a = pour(r(10, 10, 40, 30))
        let trace = PourCopper.trace(UUID(), points: [p(35, 5), p(35, 45)], width: 0.25, layer: .top, net: other)
        let left = PourCopper.pad(0, rect: r(15, 20, 2, 2), net: ground)
        let right = PourCopper.pad(1, rect: r(42, 20, 2, 2), net: ground)
        let result = try XCTUnwrap(fill([a], copper: [left, right, trace])[a.id])
        XCTAssertEqual(result.pieces.count, 2)
        XCTAssertEqual(result.joined.map(\.count), [1, 1])
        XCTAssertFalse(result.path.contains(p(35, 20), using: .winding))
        XCTAssertFalse(result.path.contains(p(35.3, 20), using: .winding), "0.125 + 0.3 clearance")
        XCTAssertTrue(result.path.contains(p(35.5, 20), using: .winding))
    }

    func testKeepoutCutsAndOnlyOnItsLayers() throws {
        let a = pour(r(10, 10, 40, 30))
        let keepout = BoardKeepout(id: UUID(), rect: r(30, 15, 5, 5), layers: [.top])
        let path = try XCTUnwrap(fill([a], keepouts: [keepout], copper: [anchor])[a.id]).path
        XCTAssertFalse(path.contains(p(32, 17), using: .winding))
        XCTAssertTrue(path.contains(p(29.9, 17), using: .winding), "the keep-out has no clearance of its own")
        XCTAssertFalse(path.contains(p(30.1, 17), using: .winding), "the pour stops at the keep-out's edge")
        XCTAssertTrue(path.contains(p(35.1, 17), using: .winding))
        let bottom = BoardKeepout(id: UUID(), rect: r(30, 15, 5, 5), layers: [.bottom])
        let free = try XCTUnwrap(fill([a], keepouts: [bottom], copper: [anchor])[a.id]).path
        XCTAssertTrue(free.contains(p(32, 17), using: .winding), "a keep-out on Bottom doesn't touch a Top pour")
        let all = BoardKeepout(id: UUID(), rect: r(30, 15, 5, 5), layers: nil)
        XCTAssertFalse(try XCTUnwrap(fill([a], keepouts: [all], copper: [anchor])[a.id]).path.contains(p(32, 17), using: .winding))
    }

    func testBoardEdgePullBack() throws {
        let a = pour(r(0, 0, 20, 20))
        let pad = PourCopper.pad(0, rect: r(8, 8, 2, 2), net: ground)
        let path = try XCTUnwrap(fill([a], copper: [pad])[a.id]).path
        XCTAssertFalse(path.contains(p(0.4, 10), using: .winding), "within 0.5 mm of the edge")
        XCTAssertTrue(path.contains(p(0.6, 10), using: .winding))
        XCTAssertFalse(path.contains(p(10, 0.4), using: .winding))
        XCTAssertTrue(path.contains(p(10, 0.6), using: .winding))
    }

    func testNonPlatedHoleKeepsTheEdgeDistance() throws {
        let a = pour(r(10, 10, 40, 30))
        let hole = PourCopper.hole(1, rect: r(15, 15, 2, 2))
        let path = try XCTUnwrap(fill([a], copper: [anchor, hole])[a.id]).path
        XCTAssertFalse(path.contains(p(17.4, 16), using: .winding))
        XCTAssertTrue(path.contains(p(17.6, 16), using: .winding))
        XCTAssertFalse(path.contains(p(16, 16), using: .winding))
    }

    func testALaterPourOfAnotherNetGivesWayToAnEarlierOne() throws {
        let a = pour(r(10, 10, 40, 30)), b = pour(r(30, 10, 40, 30))
        let padA = PourCopper.pad(0, rect: r(15, 15, 2, 2), net: ground)
        let padB = PourCopper.pad(1, rect: r(60, 20, 2, 2), net: other)
        let result = fill([a, b], nets: [ground, other], copper: [padA, padB])
        let pathB = try XCTUnwrap(result[b.id]).path
        XCTAssertFalse(pathB.contains(p(50.2, 20), using: .winding), "inside the earlier pour grown by the clearance")
        XCTAssertTrue(pathB.contains(p(50.4, 20), using: .winding))
        XCTAssertTrue(try XCTUnwrap(result[a.id]).path.contains(p(45, 20), using: .winding), "the earlier pour keeps its area")
    }

    // MARK: Phase 2

    func testThermalReliefHasSpokesAndGaps() throws {
        let a = pour(r(10, 10, 40, 30), connection: .thermal)
        let result = try XCTUnwrap(fill([a], copper: [anchor])[a.id])
        let path = result.path
        XCTAssertTrue(path.contains(p(27.7, 24), using: .winding), "west spoke")
        XCTAssertTrue(path.contains(p(30.3, 24), using: .winding), "east spoke")
        XCTAssertTrue(path.contains(p(29, 22.7), using: .winding), "north spoke")
        XCTAssertTrue(path.contains(p(29, 25.3), using: .winding), "south spoke")
        for corner in [p(27.7, 22.8), p(30.3, 22.8), p(27.7, 25.2), p(30.3, 25.2)] {
            XCTAssertFalse(path.contains(corner, using: .winding), "gap at \(corner)")
        }
        XCTAssertTrue(path.contains(p(27, 24), using: .winding), "outside the ring")
        XCTAssertEqual(result.joined, [[.pad(0)]])
    }

    func testSolidConnectionHasNoRing() throws {
        let a = pour(r(10, 10, 40, 30), connection: .solid)
        let result = try XCTUnwrap(fill([a], copper: [anchor])[a.id])
        XCTAssertTrue(result.path.contains(p(27.9, 22.8), using: .winding))
        XCTAssertTrue(result.path.contains(p(30.1, 25.2), using: .winding))
        XCTAssertEqual(result.joined, [[.pad(0)]])
    }

    func testAnIslandIsDroppedAndAPadInsideALoopKeepsItsPiece() throws {
        let a = pour(r(10, 10, 40, 30))
        let loop = PourCopper.trace(UUID(), points: [p(24, 19), p(34, 19), p(34, 29), p(24, 29), p(24, 19)],
                                    width: 0.25, layer: .top, net: other)
        let far = PourCopper.pad(0, rect: r(12, 12, 2, 2), net: ground)
        var result = try XCTUnwrap(fill([a], copper: [far, loop])[a.id])
        XCTAssertEqual(result.pieces.count, 1, "the loop's inside holds no pin of the net: an island")
        XCTAssertFalse(result.path.contains(p(29, 24), using: .winding))
        result = try XCTUnwrap(fill([a], copper: [far, loop, anchor])[a.id])
        XCTAssertEqual(result.pieces.count, 2, "a pad inside the loop keeps its piece")
        XCTAssertTrue(result.path.contains(p(26, 21), using: .winding))
    }

    func testASliverNarrowerThanTheMinimumWidthIsDropped() throws {
        var a = pour(r(10, 10, 40, 30))
        a.minWidth = 0.3
        let above = BoardKeepout(id: UUID(), rect: r(10, 10, 40, 14.9), layers: nil)
        let below = BoardKeepout(id: UUID(), rect: r(10, 25.1, 40, 14.9), layers: nil)
        let via = PourCopper.via(UUID(), centre: p(30, 25), diameter: 0.2, net: ground)
        XCTAssertTrue(try XCTUnwrap(fill([a], keepouts: [above, below], copper: [via])[a.id]).pieces.isEmpty)
        a.minWidth = 0.05
        XCTAssertEqual(try XCTUnwrap(fill([a], keepouts: [above, below], copper: [via])[a.id]).pieces.count, 1)
    }

    func testASameNetTraceIsNotCutAndJoins() throws {
        let a = pour(r(10, 10, 40, 30))
        let id = UUID()
        let trace = PourCopper.trace(id, points: [p(20, 30), p(40, 30)], width: 1, layer: .top, net: ground)
        let result = try XCTUnwrap(fill([a], copper: [anchor, trace])[a.id])
        XCTAssertTrue(result.path.contains(p(30, 30), using: .winding), "on the trace: part of the pour")
        XCTAssertTrue(result.joined.flatMap { $0 }.contains(.trace(id)))
        XCTAssertTrue(result.joined.flatMap { $0 }.contains(.pad(0)))
    }

    func testAPourWithNoNetFillsNothing() {
        let a = pour(r(10, 10, 40, 30))
        XCTAssertNil(fill([a], nets: [nil], copper: [anchor])[a.id])
    }

    func testTwoPadsWhoseSpokesMeetOnePiece() throws {
        let a = pour(r(10, 10, 40, 30))
        let one = PourCopper.pad(0, rect: r(20, 20, 2, 2), net: ground)
        let two = PourCopper.pad(1, rect: r(30, 20, 2, 2), net: ground)
        let result = try XCTUnwrap(fill([a], copper: [one, two])[a.id])
        XCTAssertEqual(result.pieces.count, 1)
        XCTAssertEqual(result.joined, [[.pad(0), .pad(1)]])
    }

    func testOnlyItemsOnThePoursLayerCount() throws {
        let a = pour(r(10, 10, 40, 30), layer: .top)
        let bottomPad = PourCopper.pad(1, rect: r(40, 20, 2, 2), net: other, layers: [.bottom])
        let path = try XCTUnwrap(fill([a], copper: [anchor, bottomPad])[a.id]).path
        XCTAssertTrue(path.contains(p(41, 21), using: .winding), "a Bottom-only pad leaves a Top pour alone")
    }

    // MARK: Phase 3

    /// Every pad, via and trace of `doc`'s copper as the fill reads it.
    private func copper(of doc: SchematicDocument) -> (items: [PourCopper], pads: [(net: Int?, layers: Set<CopperLayer>?, samples: [CGPoint])], nets: [PlaneNet: Int]) {
        let connectivity = doc.connectivity()
        let netOfRoot = connectivity.netOfRoot()
        var items: [PourCopper] = []
        var pads: [(net: Int?, layers: Set<CopperLayer>?, samples: [CGPoint])] = []
        for (index, entry) in connectivity.pads.enumerated() {
            let pad = entry.pad
            let net = connectivity.netOfPad[index]
            let layers: Set<CopperLayer>? = pad.drill > 0 ? nil : [entry.side.copperLayer]
            if pad.plated {
                items.append(.pad(index, rect: pad.rect, net: net, layers: layers, shape: PCBCanvasView.outline(of: pad).cgPath))
                pads.append((net, layers, PourCopper.rectSamples(pad.rect)))
            } else if pad.drill > 0 {
                items.append(.hole(index, rect: pad.holeRect ?? pad.rect))
            }
        }
        for (t, trace) in connectivity.traces.enumerated() {
            let net = netOfRoot[connectivity.root[connectivity.traceNode(t)]]
            items.append(.trace(trace.id, points: trace.points, width: trace.widthMM, layer: trace.layer, net: net))
        }
        for (v, via) in connectivity.vias.enumerated() {
            let net = netOfRoot[connectivity.root[connectivity.viaNode(v)]]
            items.append(.via(via.id, centre: via.centre, diameter: via.diameter, net: net))
        }
        return (items, pads, connectivity.railNets.reduce(into: [:]) { $0[.rail(mv: $1.key)] = $1.value })
    }

    func testFillOfARealRoutedBoard() throws {
        let url = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("examples/slow PCB.schematical")
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(Data(contentsOf: url)).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        let read = copper(of: doc)
        let net = try XCTUnwrap(read.nets[.ground], "the example has a ground rail")
        let rect = CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height)
        let top = pour(rect, layer: .top, connection: .thermal), bottom = pour(rect, layer: .bottom, connection: .thermal)
        let started = Date()
        let result = PourFiller.fill(pours: [top, bottom], nets: [top.id: net, bottom.id: net], keepouts: [],
                                     copper: read.items, board: doc.boardEdge, edge: doc.board.rules.minCopperToEdge,
                                     rules: doc.board.rules)
        let elapsed = Date().timeIntervalSince(started)
        print("PourFillTests: real board fill took \(elapsed) s, pieces \(result.values.map(\.pieces.count))")
        XCTAssertLessThan(elapsed, 2.0)
        XCTAssertFalse(try XCTUnwrap(result[top.id]).isEmpty)
        for (id, layer) in [(top.id, CopperLayer.top), (bottom.id, CopperLayer.bottom)] {
            let fill = try XCTUnwrap(result[id])
            for pad in read.pads where pad.net != net && (pad.layers?.contains(layer) ?? true) {
                // The centre of an other-net pad is never inside copper of this net.
                guard let centre = pad.samples.first else { continue }
                XCTAssertFalse(fill.pieces.contains { $0.contains(centre, using: .winding) }, "pad at \(centre)")
            }
        }
    }
}
