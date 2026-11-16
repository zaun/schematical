import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The Design Rule Check's two steps (plan-49), hosted in a window and
/// driven through accessibility as a user's clicks and typing would.
@MainActor
final class DesignRuleCheckWizardTests: XCTestCase {
    private var window: NSWindow!

    override func tearDown() async throws {
        window?.orderOut(nil)
        window = nil
    }

    private func host(_ doc: SchematicDocument) {
        let controller = NSHostingController(rootView: NavigationStack { DesignRuleCheckWizard(document: doc) })
        window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 720, height: 900),
                          styleMask: [.titled, .resizable], backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.contentViewController = controller
        window.setContentSize(NSSize(width: 720, height: 900))
        window.orderFront(nil)
        // SwiftUI builds its accessibility tree only once a client asks.
        let app = AXUIElementCreateApplication(getpid())
        AXUIElementSetAttributeValue(app, "AXManualAccessibility" as CFString, kCFBooleanTrue)
        AXUIElementSetAttributeValue(app, "AXEnhancedUserInterface" as CFString, kCFBooleanTrue)
        settle()
    }

    private func settle(_ seconds: TimeInterval = 0.6) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    /// An accessibility attribute of `element`, nil when it has none.
    private func value(_ element: NSObject, _ key: String) -> Any? {
        element.responds(to: NSSelectorFromString(key)) ? element.value(forKey: key) : nil
    }

    /// Every accessibility element under the window, depth first.
    private func elements() -> [NSObject] {
        var found: [NSObject] = []
        func walk(_ object: Any, _ depth: Int) {
            guard depth < 60, let element = object as? NSObject else { return }
            found.append(element)
            let children = value(element, "accessibilityChildren") as? [Any] ?? []
            for child in children { walk(child, depth + 1) }
        }
        walk(window as Any, 0)
        return found
    }

    private func role(_ element: NSObject) -> String {
        (value(element, "accessibilityRole") as? NSAccessibility.Role)?.rawValue
            ?? (value(element, "accessibilityRole") as? String) ?? ""
    }

    private func label(_ element: NSObject) -> String {
        ["accessibilityLabel", "accessibilityTitle", "accessibilityValue"].compactMap {
            value(element, $0).flatMap { "\($0)" }
        }.filter { !$0.isEmpty }.joined(separator: " | ")
    }

    private func button(_ title: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSObject {
        try XCTUnwrap(elements().first { role($0) == "AXButton" && label($0).contains(title) },
                      "no button \(title)", file: file, line: line)
    }

    private func press(_ title: String, file: StaticString = #filePath, line: UInt = #line) throws {
        let found = try button(title, file: file, line: line)
        // The return value isn't dependable for SwiftUI's nodes; callers check what the press did.
        if let button = found as? NSAccessibilityButton {
            _ = button.accessibilityPerformPress()
        } else {
            _ = found.perform(NSSelectorFromString("accessibilityPerformPress"))
        }
        settle()
    }

    /// Every view of `type` in the window.
    private func views<V: NSView>(_ type: V.Type) -> [V] {
        var found: [V] = []
        func walk(_ view: NSView) {
            if let match = view as? V { found.append(match) }
            view.subviews.forEach(walk)
        }
        window.contentView.map(walk)
        return found
    }

    /// Types `text` over field number `index` (top to bottom) and presses Return.
    private func type(_ text: String, intoField index: Int, file: StaticString = #filePath, line: UInt = #line) throws {
        let fields = views(NSTextField.self).filter(\.isEditable)
            .sorted { $0.convert($0.bounds, to: nil).midY > $1.convert($1.bounds, to: nil).midY }
        XCTAssertEqual(fields.count, 13, "one field per rule", file: file, line: line)
        let field = fields[index]
        XCTAssertTrue(window.makeFirstResponder(field), file: file, line: line)
        let editor = try XCTUnwrap(field.currentEditor() as? NSTextView, file: file, line: line)
        editor.selectAll(nil)
        editor.insertText(text, replacementRange: editor.selectedRange())
        editor.insertNewline(nil)
        settle()
    }

    private func texts() -> [String] { elements().map(label).filter { !$0.isEmpty } }

    /// Saves what the window shows, for looking at.
    private func snapshot(_ name: String) {
        guard let directory = ProcessInfo.processInfo.environment["DRC_WIZARD_SNAPSHOTS"],
              let view = window.contentView,
              let bitmap = view.bitmapImageRepForCachingDisplay(in: view.bounds) else { return }
        view.cacheDisplay(in: view.bounds, to: bitmap)
        try? bitmap.representation(using: .png, properties: [:])?
            .write(to: URL(fileURLWithPath: directory).appendingPathComponent("\(name).png"))
    }

    func testTheTwoSteps() throws {
        let doc = try Fixture.open("RegFile")
        doc.setDesignRules(.pcbway, manufacturer: .pcbway)
        host(doc)

        // Step 1: the board's rules, with Run Check.
        snapshot("1-rules")
        let first = texts()
        XCTAssertTrue(first.contains { $0.contains("Min clearance") }, "\(first)")
        XCTAssertTrue(first.contains { $0.contains("saved with this board") }, "\(first)")
        _ = try button("Run Check")

        // Typing a rule changes the board's, as one undo step.
        XCTAssertEqual(elements().filter { role($0) == "AXTextField" }.map(label).prefix(2),
                       ["Min trace width | 0.1", "Min clearance | 0.1"])
        try type("0.5", intoField: 1)
        XCTAssertEqual(doc.board.rules.minClearance, 0.5)
        XCTAssertEqual(doc.board.rulesTitle, "PCBWay (modified)")
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Design Rules")
        snapshot("2-rules-changed")

        // Step 2: the results, under the changed rule.
        try press("Run Check")
        settle(1)
        snapshot("3-results")
        let strict = doc.designRuleCheck(rules: doc.board.rules)
        XCTAssertFalse(strict.isEmpty)
        XCTAssertEqual(views(NSTableView.self).first?.numberOfRows, strict.count)
        XCTAssertTrue(window.title.contains("Step 2 of 2 · \(strict.count) issues · PCBWay (modified) rules")
                      || (window.subtitle.contains("\(strict.count) issues · PCBWay (modified) rules")),
                      window.title + " / " + window.subtitle)

        // Undo puts the rule back and the list follows.
        XCTAssertTrue(doc.undo(on: .pcb))
        settle(1)
        XCTAssertEqual(doc.board.rules, .pcbway)
        snapshot("4-results-after-undo")
        let plain = doc.designRuleCheck(rules: .pcbway)
        XCTAssertNotEqual(plain.count, strict.count)
        XCTAssertEqual(views(NSTableView.self).first?.numberOfRows, plain.count)

        // Back to step 1.
        try press("Rules")
        snapshot("5-rules-again")
        _ = try button("Run Check")

        // Reset to App Defaults copies Settings ▸ DRC Defaults into the board.
        doc.setDesignRules(.sunstone, manufacturer: .sunstone)
        settle()
        try press("Reset to App Defaults")
        XCTAssertEqual(doc.board.rules, DesignRules.appDefaults())
        XCTAssertEqual(doc.board.rulesManufacturer, DRCManufacturer.appDefault())
    }
}
