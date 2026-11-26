import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-W: the render-test helper finds, reads and presses what a hosted
/// SwiftUI view shows.
@MainActor
final class ViewProbeTests: XCTestCase {
    private struct Sample: View {
        @State private var count = 0
        let locked: Bool

        var body: some View {
            VStack {
                Text("Count \(count)")
                Button("Add one") { count += 1 }
                Button("Locked") {}.disabled(locked)
                Toggle("Option", isOn: .constant(true))
            }
        }
    }

    func testFindsLabelsAndRoles() throws {
        let probe = ViewProbe(Sample(locked: true))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Count 0"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Add one", role: "AXButton"))
        XCTAssertTrue(probe.has("Count 0", role: "AXStaticText"))
        XCTAssertFalse(probe.has("Count 0", role: "AXCheckBox"))
        XCTAssertFalse(probe.has("Nothing like this"))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Locked", role: "AXButton")))
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Add one", role: "AXButton")))
        XCTAssertTrue(probe.labels().contains { $0.contains("Option") })
        XCTAssertFalse(probe.views(NSView.self).isEmpty)
        XCTAssertNil(ViewProbe.attribute(NSObject(), "accessibilityRole"))
        XCTAssertEqual(ViewProbe.role(NSObject()), "")
    }

    func testPressRunsTheButtonsAction() throws {
        let probe = ViewProbe(Sample(locked: false), size: NSSize(width: 300, height: 200))
        defer { probe.close() }
        try probe.press("Add one")
        XCTAssertTrue(probe.has("Count 1"), "\(probe.labels())")
        try probe.press("Add one")
        XCTAssertTrue(probe.has("Count 2"), "\(probe.labels())")
    }

    func testAnExistingWindowIsLeftOpenUnlessOwned() {
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 200, height: 100), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.contentView = NSHostingView(rootView: Text("Borrowed"))
        let probe = ViewProbe(window: window)
        XCTAssertTrue(probe.has("Borrowed"), "\(probe.labels())")
        probe.close()
        XCTAssertTrue(window.isVisible)
        window.close()
        XCTAssertFalse(window.isVisible)
    }
}
