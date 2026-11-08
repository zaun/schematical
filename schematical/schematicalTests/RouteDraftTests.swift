import CoreGraphics
import XCTest
@testable import schematical

/// plan-25-A phase 2: a route being drawn with locks (Space), unlocks (Esc)
/// and flips (`/`).
final class RouteDraftTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testLockedLegThenPinMakesAU() {
        var draft = RouteDraft(start: p(0, 0), startAxis: .horizontal)
        XCTAssertTrue(draft.lock(to: p(50, 100), endAxis: nil))
        XCTAssertEqual(draft.path(to: p(0, 100), endAxis: .horizontal),
                       [p(0, 0), p(50, 0), p(50, 100), p(0, 100)])
    }

    func testLockUnlockLockGivesTheSamePath() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        draft.lock(to: p(60, 40), endAxis: nil)
        let once = draft.path(to: p(120, 0), endAxis: nil)
        XCTAssertTrue(draft.unlock())
        XCTAssertFalse(draft.hasLocks)
        draft.lock(to: p(60, 40), endAxis: nil)
        XCTAssertEqual(draft.path(to: p(120, 0), endAxis: nil), once)
    }

    func testUnlockWithNothingLockedIsFalse() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        XCTAssertFalse(draft.unlock())
    }

    func testLockOnTheLegStartChangesNothing() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        XCTAssertFalse(draft.lock(to: p(0, 0), endAxis: nil))
        XCTAssertFalse(draft.hasLocks)
        draft.lock(to: p(40, 0), endAxis: nil)
        XCTAssertFalse(draft.lock(to: p(40, 0), endAxis: nil))
        XCTAssertEqual(draft.legs.count, 1)
    }

    func testLiveLegTurnsAfterAVerticalRun() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        draft.lock(to: p(0, 100), endAxis: nil)
        XCTAssertEqual(draft.preferredAxis, .horizontal)
        XCTAssertEqual(draft.liveLeg(to: p(80, 160), endAxis: nil), [p(80, 100), p(80, 160)])
        // After a horizontal run the next leg starts down.
        draft.lock(to: p(80, 100), endAxis: nil)
        XCTAssertEqual(draft.preferredAxis, .vertical)
        XCTAssertEqual(draft.liveLeg(to: p(140, 160), endAxis: nil), [p(80, 160), p(140, 160)])
    }

    func testFlipChangesAFreeLegOnly() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        XCTAssertTrue(draft.canFlip(endAxis: nil))
        XCTAssertEqual(draft.liveLeg(to: p(100, 60), endAxis: nil), [p(100, 0), p(100, 60)])
        draft.flip()
        XCTAssertEqual(draft.liveLeg(to: p(100, 60), endAxis: nil), [p(0, 60), p(100, 60)])
        XCTAssertFalse(draft.canFlip(endAxis: .horizontal), "a pin at the end forces the leg")
        let pinned = RouteDraft(start: p(0, 0), startAxis: .vertical)
        XCTAssertFalse(pinned.canFlip(endAxis: nil), "a pin at the start forces the first leg")
    }

    func testFlipResetsOnLockAndUnlock() {
        var draft = RouteDraft(start: p(0, 0), startAxis: nil)
        draft.flip()
        XCTAssertTrue(draft.flipped)
        draft.lock(to: p(100, 60), endAxis: nil)
        XCTAssertFalse(draft.flipped)
        draft.flip()
        draft.unlock()
        XCTAssertFalse(draft.flipped)
    }

    func testFirstLegFollowsTheStartAxis() {
        let draft = RouteDraft(start: p(0, 0), startAxis: .vertical)
        XCTAssertEqual(draft.path(to: p(100, 60), endAxis: nil), [p(0, 0), p(0, 60), p(100, 60)])
        // A later leg is free again.
        var locked = draft
        locked.lock(to: p(100, 60), endAxis: nil)
        XCTAssertTrue(locked.canFlip(endAxis: nil))
    }

    func testBusCandidatesAddTheFlippedLiveLeg() {
        let draft = RouteDraft(start: p(0, 0), startAxis: nil)
        XCTAssertEqual(draft.busCandidates(to: p(100, 60)), [
            [p(0, 0), p(100, 0), p(100, 60)],
            [p(0, 0), p(0, 60), p(100, 60)],
        ])
        XCTAssertEqual(draft.busCandidates(to: p(100, 0)), [[p(0, 0), p(100, 0)]], "a straight leg has no flip")
    }
}
