import Foundation
import XCTest
@testable import schematical

/// SwiftUI builds the Verify Datasheet window's view, and with it a
/// `VerifyDatasheetModel`, whenever it rebuilds the app's scene list, and
/// frees the spare one from a run loop observer. The app aborted there
/// (`pointer being freed was not allocated`, in the Swift runtime's
/// isolated-deinit path, `StopLookupScope`): the model's isolated deinit
/// freed its `DatasheetFinder`, whose own isolated deinit was nested in it,
/// with a task-local scope open on the thread. SwiftUI's own updates open
/// one (the crash reports show the free under `AppDelegate.scenesDidChange`
/// and under `NSHostingView.deinit`); `withValue` below does the same.
/// Found by plan-61-B: with no schematic scene, the scene list is rebuilt
/// while the tests run. The fix is `nonisolated deinit {}` on both classes.
@MainActor
final class VerifyDatasheetLifetimeTests: XCTestCase {
    @TaskLocal private static var marker = 0

    /// Not async: the free happens on the main thread outside any task,
    /// with a task-local value bound, as in the crash.
    func testAModelCanBeFreedWithATaskLocalBound() {
        weak var freed: VerifyDatasheetModel?
        Self.$marker.withValue(1) {
            let model = VerifyDatasheetModel()
            freed = model
            XCTAssertNotNil(freed)
        }
        XCTAssertNil(freed)
    }

    func testAModelCanBeFreedInsideATask() async {
        weak var freed: VerifyDatasheetModel?
        do {
            let model = VerifyDatasheetModel()
            freed = model
        }
        XCTAssertNil(freed)
    }
}
