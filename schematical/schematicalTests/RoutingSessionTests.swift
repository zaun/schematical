import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Autoroute in the background (plan-38-G Phase 3): finish, stop, cancel,
/// a board changed meanwhile, refusals, progress.
@MainActor
final class RoutingSessionTests: XCTestCase {
    /// Two LEDs wired K to A: one connection.
    private func pair() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        return doc
    }

    func testARunFinishesAndApplies() async throws {
        let doc = try pair()
        let session = RoutingSession()
        XCTAssertTrue(session.start(document: doc, options: .fixture, rules: .customDefault))
        XCTAssertTrue(session.isRunning)
        await session.wait()
        guard case let .finished(summary) = session.state else { return XCTFail("\(session.state)") }
        XCTAssertGreaterThan(summary.addedTraces, 0)
        XCTAssertTrue(doc.traces.allSatisfy(\.auto))
        XCTAssertTrue(doc.ratsnest().isEmpty)
        XCTAssertEqual(doc.undoName(on: .pcb), "Autoroute")
    }

    func testStopKeepsALegalPartialResult() async throws {
        let doc = try Fixture.open("AFU")
        let session = RoutingSession()
        XCTAssertTrue(session.start(document: doc, options: .fixture, rules: .customDefault))
        session.stop()
        await session.wait()
        guard case .finished = session.state else { return XCTFail("\(session.state)") }
        let result = try XCTUnwrap(session.result)
        let problem = try XCTUnwrap(session.problem)
        XCTAssertTrue(result.stats.stopped)
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
        XCTAssertEqual(doc.traces.count, result.traces.count)
    }

    func testCancelLeavesTheDocumentAsItWas() async throws {
        let doc = try Fixture.open("AFU")
        let before = doc.state
        let session = RoutingSession()
        XCTAssertTrue(session.start(document: doc, options: .fixture, rules: .customDefault))
        session.cancel()
        await session.wait()
        XCTAssertEqual(session.state, .discarded(RoutingSession.cancelledReason))
        XCTAssertEqual(doc.state, before)
    }

    func testABoardChangedMeanwhileIsNotRoutedOnto() async throws {
        let doc = try Fixture.open("AFU")
        let session = RoutingSession()
        XCTAssertTrue(session.start(document: doc, options: .fixture, rules: .customDefault))
        let chip = try XCTUnwrap(doc.chips.first { $0.board != nil })
        doc.moveFootprint(chip.id, to: CGPoint(x: 10, y: 10))
        session.stop()
        await session.wait()
        XCTAssertEqual(session.state, .discarded(RoutingSession.changedReason))
        XCTAssertTrue(doc.traces.isEmpty)
    }

    func testRefusedWhileTheSimulationRunsOrARunIsUnderWay() async throws {
        let doc = try pair()
        doc.isPlaying = true
        let session = RoutingSession()
        XCTAssertFalse(session.start(document: doc, options: .fixture, rules: .customDefault))
        XCTAssertEqual(session.state, .idle)
        XCTAssertNotNil(doc.editorMessage)
        doc.isPlaying = false
        doc.editorMessage = nil

        let busy = try Fixture.open("AFU")
        XCTAssertTrue(session.start(document: busy, options: .fixture, rules: .customDefault))
        XCTAssertFalse(session.start(document: busy, options: .fixture, rules: .customDefault))
        XCTAssertNotNil(busy.editorMessage)
        session.cancel()
        await session.wait()
    }

    func testProgressNeverGoesBackBetweenRounds() async throws {
        let doc = try Fixture.open("AFU")
        let session = RoutingSession()
        XCTAssertTrue(session.start(document: doc, options: .fixture, rules: .customDefault))
        await session.wait()
        // Let the last hops to the main actor land.
        for _ in 0..<5 { await Task.yield() }
        let reports = session.reports
        XCTAssertFalse(reports.isEmpty)
        var lastOfRound: [Int: Int] = [:]
        for report in reports { lastOfRound[report.round] = report.routed }
        let rounds = lastOfRound.keys.sorted()
        for (a, b) in zip(rounds, rounds.dropFirst()) {
            XCTAssertLessThanOrEqual(lastOfRound[a]!, lastOfRound[b]!, "round \(a) to \(b)")
        }
        XCTAssertTrue(reports.allSatisfy { $0.total == 115 })
    }
}
