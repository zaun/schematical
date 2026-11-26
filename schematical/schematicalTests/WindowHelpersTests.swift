import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The close guard in front of a window's own delegate, and the reader of a
/// view's window (plan-68-Z).
@MainActor
final class WindowHelpersTests: XCTestCase {
    nonisolated deinit {}

    /// A delegate that answers `windowShouldClose` and records a resize.
    private final class Original: NSObject, NSWindowDelegate {
        nonisolated deinit {}
        var allowsClose = true
        var resized = 0
        func windowShouldClose(_ sender: NSWindow) -> Bool { allowsClose }
        func windowDidResize(_ notification: Notification) { resized += 1 }
    }

    private var windows: [NSWindow] = []

    /// These tests close windows; the app would quit after the last one.
    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        windows.forEach { $0.close() }
        windows = []
    }

    private func makeWindow() -> NSWindow {
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 200, height: 120), styleMask: [.titled, .closable],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        windows.append(window)
        return window
    }

    func testTheGuardAsksTheOriginalThenItsOwnRule() {
        let window = makeWindow()
        let original = Original()
        window.delegate = original
        var asked = 0
        var answer = false
        let guardian = WindowCloseGuard { _ in asked += 1; return answer }
        guardian.attach(to: window)
        XCTAssertTrue(window.delegate === guardian)
        // Attaching to the same window again changes nothing.
        guardian.attach(to: window)
        XCTAssertTrue(window.delegate === guardian)

        XCTAssertFalse(guardian.windowShouldClose(window))
        XCTAssertEqual(asked, 1)
        answer = true
        XCTAssertTrue(guardian.windowShouldClose(window))
        XCTAssertEqual(asked, 2)
        // The original says no: the guard's rule is not asked.
        original.allowsClose = false
        XCTAssertFalse(guardian.windowShouldClose(window))
        XCTAssertEqual(asked, 2)
    }

    func testOtherMessagesReachTheOriginal() {
        let window = makeWindow()
        let original = Original()
        window.delegate = original
        let guardian = WindowCloseGuard { _ in true }
        guardian.attach(to: window)
        let resize = #selector(NSWindowDelegate.windowDidResize(_:))
        XCTAssertTrue(guardian.responds(to: resize))
        XCTAssertTrue(guardian.forwardingTarget(for: resize) as AnyObject? === original)
        XCTAssertFalse(guardian.responds(to: #selector(NSWindowDelegate.windowDidMiniaturize(_:))))
        XCTAssertNil(guardian.forwardingTarget(for: #selector(NSWindowDelegate.windowDidMiniaturize(_:))))
        window.setContentSize(NSSize(width: 260, height: 140))
        XCTAssertGreaterThan(original.resized, 0)
    }

    func testMovingToAnotherWindowGivesTheFirstItsDelegateBack() {
        let first = makeWindow(), second = makeWindow()
        let original = Original()
        first.delegate = original
        let guardian = WindowCloseGuard { _ in true }
        guardian.attach(to: first)
        guardian.attach(to: second)
        XCTAssertTrue(first.delegate === original)
        XCTAssertTrue(second.delegate === guardian)
        // The first window's delegate is no longer the guard: detaching leaves it.
        first.delegate = nil
        guardian.attach(to: first)
        XCTAssertTrue(first.delegate === guardian)
        XCTAssertNil(second.delegate)
        guardian.attach(to: nil)
        XCTAssertNil(first.delegate)
    }

    func testTheReaderReportsItsWindow() {
        var seen: [NSWindow] = []
        let probe = ViewProbe(Text("Inside").background(WindowReader { seen.append($0) }),
                              size: NSSize(width: 200, height: 100))
        defer { probe.close() }
        XCTAssertTrue(seen.contains { $0 === probe.window })
    }
}

extension WindowHelpersTests {
    func testTheReaderViewIsNeverMadeFromACoder() throws {
        let archive = try NSKeyedArchiver.archivedData(withRootObject: NSView(), requiringSecureCoding: false)
        let coder = try NSKeyedUnarchiver(forReadingFrom: archive)
        XCTAssertNil(WindowReaderView(coder: coder))
        // Out of a window, nothing is reported.
        var reported = 0
        let view = WindowReaderView { _ in reported += 1 }
        view.viewDidMoveToWindow()
        XCTAssertEqual(reported, 0)
    }
}
