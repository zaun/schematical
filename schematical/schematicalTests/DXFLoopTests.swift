import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// DXF loops against ezdxf's numbers, and the import plan (plan-65-H Phase 2).
final class DXFLoopTests: XCTestCase {
    private struct Expected {
        var scale: Double
        var loops: [(area: Double, box: [Double])]
        var skipped: [String: Int]
    }

    private func expected() throws -> [String: Expected] {
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(DXFFixtures.expectedJSON.utf8)) as? [String: Any])
        var result: [String: Expected] = [:]
        for (name, value) in object {
            let entry = try XCTUnwrap(value as? [String: Any])
            let loops = try XCTUnwrap(entry["loops"] as? [[String: Any]]).map { loop in
                (try XCTUnwrap(loop["area"] as? Double), try XCTUnwrap(loop["box"] as? [Double]))
            }
            result[name] = Expected(scale: try XCTUnwrap(entry["scale_to_mm"] as? Double), loops: loops,
                                    skipped: (entry["skipped"] as? [String: Int]) ?? [:])
        }
        return result
    }

    private func box(_ points: [CGPoint]) -> [Double] {
        [points.map(\.x).min()!, points.map(\.y).min()!, points.map(\.x).max()!, points.map(\.y).max()!].map { Double($0) }
    }

    private func p(_ x: Double, _ y: Double) -> CGPoint { CGPoint(x: x, y: y) }

    private func loop(_ points: [CGPoint]) -> DXFLoop { DXFLoop(points: points, layer: "0") }

    private func square(_ x: Double, _ y: Double, _ side: Double) -> DXFLoop {
        loop([p(x, y), p(x + side, y), p(x + side, y + side), p(x, y + side)])
    }

    // MARK: Against the oracle

    func testEveryFixtureGivesTheLoopsEzdxfGives() throws {
        let numbers = try expected()
        XCTAssertEqual(Set(numbers.keys), Set(DXFFixtures.all.map(\.name)))
        for (name, text) in DXFFixtures.all {
            let drawing = try DXFReader.parse(text)
            let want = try XCTUnwrap(numbers[name])
            let set = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: want.scale)
            let got = set.loops.sorted { $0.area > $1.area }
            let wanted = want.loops.sorted { $0.area > $1.area }
            XCTAssertEqual(got.count, wanted.count, "\(name): \(got.map(\.area))")
            for (loop, goal) in zip(got, wanted) {
                XCTAssertEqual(loop.area, goal.area, accuracy: goal.area * 0.01, "\(name) area")
                for (a, b) in zip(box(loop.points), goal.box) { XCTAssertEqual(a, b, accuracy: 0.02, "\(name) box \(box(loop.points)) vs \(goal.box)") }
                XCTAssertGreaterThan(BoardGeometry.area(loop.points), 0, "\(name): counter-clockwise")
            }
            XCTAssertEqual(drawing.skipped, want.skipped.filter { $0.key != "open_chains" }, name)
            XCTAssertEqual(set.openChains, want.skipped["open_chains"] ?? 0, name)
        }
    }

    func testTheChainFixtureIsOneLoopFromPiecesOutOfOrder() throws {
        let drawing = try DXFReader.parse(DXFFixtures.chainBoard)
        let set = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: 1)
        XCTAssertEqual(set.loops.count, 1)
        XCTAssertEqual(set.openChains, 0)
        XCTAssertEqual(set.crossingSelf, 0)
    }

    func testKiCadsBoardIsThreeLoopsFromLinesACircleAndAnArc() throws {
        let drawing = try DXFReader.parse(DXFFixtures.kicadEdgeProbe)
        let set = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: 1)
        XCTAssertEqual(set.loops.map { Int($0.area.rounded()) }.sorted(), [6, 28, 1200])
    }

    func testALayerWithNoEntitiesHasNoLoops() throws {
        let drawing = try DXFReader.parse(DXFFixtures.roundedBoard)
        XCTAssertTrue(DXFLoops.build(drawing, layers: ["Dims"], scale: 1).loops.isEmpty)
    }

    func testTheOpenChainIsCountedNotImported() throws {
        let drawing = try DXFReader.parse(DXFFixtures.openChain)
        let set = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: 1)
        XCTAssertEqual(set.loops.count, 1)
        XCTAssertEqual(set.openChains, 1)
    }

    // MARK: Joining

    private func lines(_ segments: [(CGPoint, CGPoint)]) -> DXFDrawing {
        DXFDrawing(entities: segments.map { .line(layer: "0", from: $0.0, to: $0.1) })
    }

    func testTwoEndsFiveHundredthsApartDoNotJoinAndOneHundredthDoes() {
        let gap = { (g: Double) in
            self.lines([(self.p(0, 0), self.p(10, 0)), (self.p(10, g), self.p(10, 10)), (self.p(10, 10), self.p(0, 10)), (self.p(0, 10), self.p(0, 0))])
        }
        XCTAssertEqual(DXFLoops.build(gap(0.05), layers: ["0"], scale: 1).loops.count, 0, "a 0.05 mm gap")
        XCTAssertEqual(DXFLoops.build(gap(0.05), layers: ["0"], scale: 1).openChains, 1)
        XCTAssertEqual(DXFLoops.build(gap(0.005), layers: ["0"], scale: 1).loops.count, 1, "a 0.005 mm gap")
    }

    func testABowTieMadeOfFourLinesIsDropped() {
        let drawing = lines([(p(0, 0), p(10, 10)), (p(10, 10), p(10, 0)), (p(10, 0), p(0, 10)), (p(0, 10), p(0, 0))])
        let set = DXFLoops.build(drawing, layers: ["0"], scale: 1)
        XCTAssertTrue(set.loops.isEmpty)
        XCTAssertEqual(set.crossingSelf, 1)
    }

    // MARK: The plan

    func testAnOutlineWithTwoCutoutsInEitherMode() throws {
        let drawing = try DXFReader.parse(DXFFixtures.roundedBoard)
        let loops = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: 1).loops
        let plan = DXFImportPlan.make(loops: loops, mode: .outlineAndCutouts)
        XCTAssertEqual(try XCTUnwrap(plan.outline).count, loops.map(\.points.count).max())
        XCTAssertEqual(plan.cutouts.count, 2)
        XCTAssertTrue(plan.ignored.isEmpty)
        // As cutouts only, the outer loop is the cutout and the two inside it are ignored.
        let only = DXFImportPlan.make(loops: loops, mode: .cutoutsOnly)
        XCTAssertNil(only.outline)
        XCTAssertEqual(only.cutouts.count, 1)
        XCTAssertEqual(only.ignored, [DXFIgnored(reason: .insideACutout, count: 2)])
    }

    func testAFarSquareIsOutsideTheOutline() {
        let loops = [square(0, 0, 60), square(10, 10, 5), square(100, 100, 5)]
        let plan = DXFImportPlan.make(loops: loops, mode: .outlineAndCutouts)
        XCTAssertEqual(plan.outline, loops[0].points)
        XCTAssertEqual(plan.cutouts, [loops[1].points])
        XCTAssertEqual(plan.ignored, [DXFIgnored(reason: .outsideTheOutline, count: 1)])
    }

    func testALoopInsideACutoutIsIgnored() {
        let loops = [square(0, 0, 60), square(10, 10, 20), square(15, 15, 5)]
        let plan = DXFImportPlan.make(loops: loops, mode: .outlineAndCutouts)
        XCTAssertEqual(plan.cutouts, [loops[1].points])
        XCTAssertEqual(plan.ignored, [DXFIgnored(reason: .insideACutout, count: 1)])
    }

    func testTwoLoopsThatCrossAreIgnored() {
        let loops = [square(0, 0, 60), square(10, 10, 20), square(20, 20, 20)]
        let plan = DXFImportPlan.make(loops: loops, mode: .outlineAndCutouts)
        XCTAssertEqual(plan.outline, loops[0].points)
        XCTAssertTrue(plan.cutouts.isEmpty)
        XCTAssertEqual(plan.ignored, [DXFIgnored(reason: .crossesAnotherLoop, count: 2)])
    }

    func testSeparateLoopsAreEachACutoutInCutoutsOnlyMode() {
        let loops = [square(0, 0, 10), square(20, 0, 10), square(40, 0, 10)]
        let plan = DXFImportPlan.make(loops: loops, mode: .cutoutsOnly)
        XCTAssertEqual(plan.cutouts.count, 3)
        XCTAssertTrue(plan.ignored.isEmpty)
        XCTAssertNil(plan.outline)
    }

    func testNoLoopsGiveNoOutline() {
        let plan = DXFImportPlan.make(loops: [], mode: .outlineAndCutouts)
        XCTAssertNil(plan.outline)
        XCTAssertTrue(plan.cutouts.isEmpty)
    }

    // MARK: Layers

    func testTheDefaultLayers() {
        XCTAssertEqual(DXFLayerPick.defaultSelection(layers: ["Edge", "Dims"]), ["Edge"])
        XCTAssertEqual(DXFLayerPick.defaultSelection(layers: ["A", "B"]), ["A", "B"])
        XCTAssertEqual(DXFLayerPick.defaultSelection(layers: ["BOARD_OUTLINE", "0", "Notes"]), ["BOARD_OUTLINE"])
        XCTAssertEqual(DXFLayerPick.defaultSelection(layers: ["Edge.Cuts"]), ["Edge.Cuts"])
        XCTAssertEqual(DXFLayerPick.defaultSelection(layers: []), [])
    }

    // MARK: The app's own board, through KiCad

    @MainActor
    func testTheAppsCutoutsBoardComesBackFromKiCadsDXF() throws {
        let doc = try ExportSamples.cutouts()
        let drawing = try DXFReader.parse(DXFKiCadRoundTrip.text)
        XCTAssertEqual(drawing.units, .millimetre)
        let set = DXFLoops.build(drawing, layers: Set(drawing.layers), scale: 1)
        XCTAssertEqual(set.openChains, 0)
        let wanted = doc.boardEdge.loops.map { abs(BoardGeometry.area($0)) }.sorted()
        let got = set.loops.map(\.area).sorted()
        XCTAssertEqual(got.count, 4)
        XCTAssertEqual(wanted.count, 4)
        for (a, b) in zip(got, wanted) { XCTAssertEqual(a, b, accuracy: b * 0.01 + 1e-6, "\(got) vs \(wanted)") }
        // The plan reads them as an outline with three cutouts.
        let plan = DXFImportPlan.make(loops: set.loops, mode: .outlineAndCutouts)
        XCTAssertNotNil(plan.outline)
        XCTAssertEqual(plan.cutouts.count, 3)
        XCTAssertTrue(plan.ignored.isEmpty)
    }
}
