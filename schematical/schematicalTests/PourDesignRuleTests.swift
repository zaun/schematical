import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The design rule check and pours (plan-52-H).
@MainActor
final class PourDesignRuleTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }

    /// Two LEDs on a 4-layer board, cathodes on ground (a through-hole pad each), anodes together.
    private func board() throws -> (doc: SchematicDocument, ground: [CGPoint]) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * CGFloat(n), y: 100))
            let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100 + 300 * CGFloat(n), y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            doc.moveFootprint(led.id, to: CGPoint(x: 30 + 30 * CGFloat(n), y: 40))
            leds.append(led)
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(leds[0], "A")), .pin(try TestSupport.pin(leds[1], "A"))))
        guard doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true else { throw XCTSkip("the LED footprint is not through-hole") }
        let copper = doc.connectivity()
        let ground = try XCTUnwrap(copper.net(of: .ground))
        let centres = copper.pads.indices.filter { copper.netOfPad[$0] == ground }.map { copper.pads[$0].centre }
        return (doc, centres)
    }

    private func issues(_ doc: SchematicDocument, _ kind: DesignRuleIssue.Kind) -> [DesignRuleIssue] {
        doc.designRuleCheck(rules: doc.board.rules).filter { $0.kind == kind }
    }

    private func whole(_ doc: SchematicDocument) -> CGRect { CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height) }

    func testATraceEndInAPourIsConnected() throws {
        let (doc, ground) = try board()
        let from = ground[0]
        let end = CGPoint(x: from.x, y: from.y + 6)
        _ = try XCTUnwrap(doc.addTrace(points: [from, end], layer: .top))
        func ends() -> Int { issues(doc, .unconnected).filter { $0.issue.contains("is not connected to anything") }.count }
        XCTAssertEqual(ends(), 1, "the far end lands on nothing")
        _ = doc.addPour(rect: whole(doc), layer: .top)
        XCTAssertEqual(ends(), 0, "inside the pour it is connected")
    }

    func testAViaInAPourIsNotAStub() throws {
        let (doc, ground) = try board()
        _ = try XCTUnwrap(doc.addTrace(points: [ground[0], CGPoint(x: ground[0].x, y: ground[0].y + 6)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: ground[0].x, y: ground[0].y + 6)))
        _ = via
        let before = issues(doc, .unconnected).filter { $0.issue.hasPrefix("Goes nowhere") }.count
        XCTAssertEqual(before, 1)
        _ = doc.addPour(rect: whole(doc), layer: .bottom)
        XCTAssertEqual(issues(doc, .unconnected).filter { $0.issue.hasPrefix("Goes nowhere") }.count, 0, "the Bottom pour joins the via")
    }

    func testEachPourProblemHasItsSentence() throws {
        let (doc, _) = try board()
        let id = try XCTUnwrap(doc.addPour(rect: whole(doc), layer: .top))
        XCTAssertTrue(issues(doc, .pour).isEmpty, "a clean pour: \(issues(doc, .pour).map(\.issue))")
        doc.setPourNet(id, nil)
        XCTAssertEqual(issues(doc, .pour).map(\.issue), ["Pour on Top has no net; it fills nothing"])
        XCTAssertEqual(issues(doc, .pour).first?.items, [.pour(id)])
        doc.setPourNet(id, .rail(mv: 9000))
        XCTAssertEqual(issues(doc, .pour).map(\.issue), ["Pour on Top: its net is not in the schematic"])
        doc.setPourNet(id, .ground)
        doc.setPourRect(id, r(70, 60, 5, 5))
        XCTAssertEqual(issues(doc, .pour).map(\.issue), ["Pour on Top keeps no copper: it reaches no pin of its net"])
        doc.setPourRect(id, whole(doc))
        var rules = doc.board.rules
        rules.minTraceWidth = 0.6
        doc.setDesignRules(rules, manufacturer: doc.board.rulesManufacturer)
        let narrow = issues(doc, .pour).map(\.issue)
        XCTAssertEqual(narrow, ["Pour on Top: thermal spokes 0.5 mm are narrower than the minimum trace width 0.6 mm"])
        doc.setPourConnection(id, .solid)
        XCTAssertTrue(issues(doc, .pour).isEmpty, "solid has no spokes")
        // An overlap of two pours of different nets.
        let other = try XCTUnwrap(doc.addPour(rect: r(0, 0, 20, 20), layer: .top))
        doc.setPourNet(other, try XCTUnwrap(doc.planeNetChoices().last).net)
        let overlap = issues(doc, .pour).map(\.issue).filter { $0.contains("overlaps the pour of") }
        XCTAssertEqual(overlap.count, 1, "\(issues(doc, .pour).map(\.issue))")
        XCTAssertTrue(overlap[0].hasSuffix("; the later one gives way"))
    }

    func testAPourTakesTheUnroutedLineAway() throws {
        let (doc, _) = try board()
        let before = issues(doc, .unrouted).count
        _ = doc.addPour(rect: whole(doc), layer: .top)
        XCTAssertEqual(issues(doc, .unrouted).count, before - 1)
        XCTAssertTrue(issues(doc, .short).isEmpty)
    }

    func testAKeepoutHasNoRule() throws {
        let (doc, _) = try board()
        let before = doc.designRuleCheck(rules: doc.board.rules).count
        doc.addKeepout(rect: r(10, 10, 5, 5))
        XCTAssertEqual(doc.designRuleCheck(rules: doc.board.rules).count, before)
    }
}
