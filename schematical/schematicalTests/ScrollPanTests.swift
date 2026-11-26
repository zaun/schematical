import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AJ: two-finger scrolling pans a canvas (`View.scrollPans`). The
/// canvas's scroll view is found in its window and handed scrolls as its
/// local monitor hands them (a made event's location is the pointer's, so
/// the place is given to `takes` directly).
@MainActor
final class ScrollPanTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    func testAScrollOverTheCanvasPansIt() throws {
        let viewport = CanvasViewport()
        var scrolled: [CGSize] = []
        let probe = ViewProbe(Color.clear.frame(width: 300, height: 200).scrollPans(viewport) { scrolled.append($0) },
                              size: NSSize(width: 300, height: 200))
        probes.append(probe)
        let window = probe.window
        let view = try XCTUnwrap(probe.views(ScrollPanNSView.self).first, "the canvas's scroll view")
        let inside = CGPoint(x: 150, y: 100)

        XCTAssertTrue(view.takes(window, at: inside, modifiers: [], dx: 12, dy: -8))
        XCTAssertEqual(viewport.offset, CGSize(width: 12, height: -8))
        XCTAssertEqual(scrolled, [CGSize(width: 12, height: -8)])

        // A zero scroll is taken and does nothing; ⌘ or ⌃, outside the view
        // or another window's scroll is left alone.
        XCTAssertTrue(view.takes(window, at: inside, modifiers: [], dx: 0, dy: 0))
        XCTAssertFalse(view.takes(window, at: inside, modifiers: .command, dx: 5, dy: 5))
        XCTAssertFalse(view.takes(window, at: inside, modifiers: .control, dx: 5, dy: 5))
        XCTAssertFalse(view.takes(window, at: CGPoint(x: 150, y: 400), modifiers: [], dx: 5, dy: 5))
        XCTAssertFalse(view.takes(nil, at: inside, modifiers: [], dx: 5, dy: 5))
        XCTAssertEqual(viewport.offset, CGSize(width: 12, height: -8))
        XCTAssertEqual(scrolled.count, 1)

        // A real scroll event goes through the same check: another window's is passed on.
        let cg = try XCTUnwrap(CGEvent(scrollWheelEvent2Source: nil, units: .pixel, wheelCount: 2, wheel1: 3, wheel2: 3,
                                       wheel3: 0))
        let event = try XCTUnwrap(NSEvent(cgEvent: cg))
        if event.window !== window { XCTAssertTrue(view.handle(event) === event) }

        // Moved to another window, it watches that one instead.
        let other = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 300, height: 200), styleMask: [.titled],
                             backing: .buffered, defer: false)
        other.isReleasedWhenClosed = false
        probes.append(ViewProbe(window: other, owns: true))
        let hosting = try XCTUnwrap(window.contentView)
        window.contentView = nil
        other.contentView = hosting
        probe.settle()
        XCTAssertFalse(view.takes(window, at: inside, modifiers: [], dx: 4, dy: 4), "the old window's scrolls are not its own")
        XCTAssertTrue(view.takes(other, at: inside, modifiers: [], dx: 4, dy: 0))
        XCTAssertEqual(viewport.offset.width, 16, accuracy: 0.01)
        other.contentView = nil
        probe.settle()
        XCTAssertFalse(view.takes(other, at: inside, modifiers: [], dx: 4, dy: 0), "out of any window: nothing")
    }
}
