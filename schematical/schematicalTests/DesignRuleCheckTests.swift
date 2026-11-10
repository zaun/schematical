import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Window › Design Rule Check (`SchematicDocument.designRuleCheck`).
@MainActor
final class DesignRuleCheckTests: XCTestCase {
    private let rules = DesignRules.pcbway

    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs well inside the board, K of one wired to A of the other.
    private func wiredPair() throws -> (doc: SchematicDocument, one: PlacedChip, two: PlacedChip, k: PinRef, a: PinRef) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (doc, one, two, k, a)
    }

    private func kinds(_ issues: [DesignRuleIssue]) -> [DesignRuleIssue.Kind] { issues.map(\.kind) }

    func testUnroutedConnectionIsListed() throws {
        let (doc, one, two, _, _) = try wiredPair()
        let issues = doc.designRuleCheck(rules: rules)
        let unrouted = issues.filter { $0.kind == .unrouted }
        XCTAssertEqual(unrouted.count, 1, "\(issues)")
        XCTAssertEqual(unrouted.first?.items, [.footprint(one.id), .footprint(two.id)])
        XCTAssertEqual(issues.map(\.index), Array(1...issues.count), "numbered from 1")
    }

    /// Down from K, across under both parts, up to A: a straight line
    /// would cross the parts' other pads.
    @discardableResult
    private func route(_ doc: SchematicDocument, _ k: PinRef, _ a: PinRef) throws -> UUID {
        let from = try centre(doc, k), to = try centre(doc, a)
        return try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: from.x, y: from.y + 6),
                                                   CGPoint(x: to.x, y: from.y + 6), to], layer: .top))
    }

    func testRoutedBoardHasNoConnectionIssues() throws {
        let (doc, _, _, k, a) = try wiredPair()
        try route(doc, k, a)
        let issues = doc.designRuleCheck(rules: rules)
        XCTAssertFalse(issues.contains { [.unrouted, .unconnected, .short, .placement].contains($0.kind) }, "\(issues)")
    }

    func testTraceToNowhereIsUnconnected() throws {
        let (doc, _, _, _, _) = try wiredPair()
        let id = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 70), CGPoint(x: 20, y: 70)], layer: .top))
        let issues = doc.designRuleCheck(rules: rules).filter { $0.items == [.trace(id)] }
        XCTAssertEqual(kinds(issues), [.unconnected], "\(issues)")
    }

    func testDanglingTraceEndIsListed() throws {
        let (doc, _, _, k, _) = try wiredPair()
        let start = try centre(doc, k)
        let id = try XCTUnwrap(doc.addTrace(points: [start, CGPoint(x: start.x, y: start.y + 10)], layer: .top))
        let issues = doc.designRuleCheck(rules: rules).filter { $0.items == [.trace(id)] }
        XCTAssertEqual(issues.count, 1, "\(issues)")
        XCTAssertTrue(issues.first?.issue.hasPrefix("End at") ?? false, "\(issues)")
    }

    func testTraceJoiningTwoNetsIsAShort() throws {
        let (doc, one, two, k, a) = try wiredPair()
        // A second net: A of the first LED to K of the second.
        let a1 = try TestSupport.pin(one, "A"), k2 = try TestSupport.pin(two, "K")
        XCTAssertNil(doc.connect(.pin(a1), .pin(k2)))
        try route(doc, k, a)
        // Then join K and A of the first LED.
        XCTAssertNotNil(doc.addTrace(from: try centre(doc, k), to: try centre(doc, a1), layer: .top, diagonalFirst: false))
        XCTAssertTrue(doc.designRuleCheck(rules: rules).contains { $0.kind == .short })
    }

    func testNarrowTraceBreaksTheWidthRule() throws {
        let (doc, _, _, k, a) = try wiredPair()
        let id = try route(doc, k, a)
        var strict = rules
        strict.minTraceWidth = 0.3
        let issues = doc.designRuleCheck(rules: strict).filter { $0.kind == .traceWidth }
        XCTAssertEqual(issues.map(\.items), [[.trace(id)]])
        XCTAssertTrue(doc.designRuleCheck(rules: rules).allSatisfy { $0.kind != .traceWidth })
    }

    func testPartOffTheBoardIsListed() throws {
        let (doc, one, _, _, _) = try wiredPair()
        doc.moveFootprint(one.id, to: CGPoint(x: -50, y: -50))
        let issues = doc.designRuleCheck(rules: rules)
        XCTAssertTrue(issues.contains { $0.kind == .placement && $0.reference == one.reference && $0.issue == "Off the board" },
                      "\(issues)")
    }

    func testCopperNearTheEdgeIsListed() throws {
        let (doc, _, _, _, _) = try wiredPair()
        let id = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 0.1), CGPoint(x: 20, y: 0.1)], layer: .top))
        XCTAssertTrue(doc.designRuleCheck(rules: rules).contains { $0.kind == .edge && $0.items == [.trace(id)] })
    }

    func testTracesOfTwoNetsTooCloseBreakClearance() throws {
        let (doc, _, _, _, _) = try wiredPair()
        // Two unconnected traces 0.3 mm apart centre to centre, 10 mil
        // (0.254 mm) wide: 0.046 mm of gap.
        let a = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 70), CGPoint(x: 20, y: 70)], layer: .top))
        let b = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 70.3), CGPoint(x: 20, y: 70.3)], layer: .top))
        let clearance = doc.designRuleCheck(rules: rules).filter { $0.kind == .clearance }
        XCTAssertEqual(clearance.map(\.items), [[.trace(a), .trace(b)]], "\(clearance)")
        // On different layers they don't meet.
        doc.setTraceLayer(b, .bottom)
        XCTAssertFalse(doc.designRuleCheck(rules: rules).contains { $0.kind == .clearance && $0.items.contains(.trace(a)) })
    }

    func testSmallViaBreaksTheViaRules() throws {
        let (doc, _, _, _, _) = try wiredPair()
        let id = try XCTUnwrap(doc.addVia(at: CGPoint(x: 50, y: 70)))
        doc.setViaDrill(id, 0.15)
        doc.setViaDiameter(id, 0.3)
        let via = doc.designRuleCheck(rules: rules).filter { $0.kind == .via }
        XCTAssertEqual(via.count, 1)
        XCTAssertTrue(via.first?.issue.lowercased().contains("drill") ?? false)
        XCTAssertTrue(via.first?.issue.contains("diameter") ?? false)
    }

    func testOverlappingBodiesAreListed() throws {
        let (doc, one, two, _, _) = try wiredPair()
        doc.moveFootprint(two.id, to: CGPoint(x: 30.5, y: 40))
        XCTAssertTrue(doc.designRuleCheck(rules: rules).contains {
            $0.kind == .placement && $0.items == [.footprint(one.id), .footprint(two.id)]
        })
    }
}
