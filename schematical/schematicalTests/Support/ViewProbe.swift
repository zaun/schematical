import AppKit
import SwiftUI
import XCTest

/// A view or window under test, read through its accessibility tree
/// (plan-68-W § 3): host a SwiftUI view in a window, lay it out, find
/// elements by role and label, press buttons, and close the window when done.
///
///     let probe = ViewProbe(SomeView(model: model))
///     defer { probe.close() }
///     XCTAssertTrue(probe.has("Ready"))
///     try probe.press("Run")
@MainActor
final class ViewProbe {
    let window: NSWindow
    private let ownsWindow: Bool

    /// `view` hosted in a new window of `size`, shown (off the main screen's
    /// key status: `orderFront`, not `makeKey`) and settled.
    convenience init<V: View>(_ view: V, size: NSSize = NSSize(width: 640, height: 480)) {
        let window = NSWindow(contentRect: NSRect(origin: .zero, size: size), styleMask: [.titled, .resizable],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.contentView = NSHostingView(rootView: view)
        window.setContentSize(size)
        self.init(window: window, owns: true)
    }

    /// An existing window (a tool window the app made); `close()` closes it
    /// only when `owns` is true.
    init(window: NSWindow, owns: Bool = false) {
        self.window = window
        ownsWindow = owns
        Self.enableAccessibility()
        window.orderFront(nil)
        window.contentView?.layoutSubtreeIfNeeded()
        settle()
    }

    /// A main-actor class's isolated deinit crashes the test host (malloc
    /// "pointer being freed was not allocated", as plan-68-D's WireRoutes).
    nonisolated deinit {}

    func close() {
        if ownsWindow { window.close() }
    }

    /// SwiftUI builds its accessibility tree only once a client asks.
    static func enableAccessibility() {
        let app = AXUIElementCreateApplication(getpid())
        AXUIElementSetAttributeValue(app, "AXManualAccessibility" as CFString, kCFBooleanTrue)
        AXUIElementSetAttributeValue(app, "AXEnhancedUserInterface" as CFString, kCFBooleanTrue)
    }

    /// Runs the main run loop for `seconds`, so SwiftUI applies state changes.
    func settle(_ seconds: TimeInterval = 0.3) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    // MARK: Reading the tree

    /// An accessibility attribute of `element`, nil when it has none.
    static func attribute(_ element: NSObject, _ key: String) -> Any? {
        element.responds(to: NSSelectorFromString(key)) ? element.value(forKey: key) : nil
    }

    /// Every accessibility element under the window, depth first.
    func elements() -> [NSObject] {
        var found: [NSObject] = []
        func walk(_ object: Any, _ depth: Int) {
            guard depth < 60, let element = object as? NSObject else { return }
            found.append(element)
            for child in Self.attribute(element, "accessibilityChildren") as? [Any] ?? [] { walk(child, depth + 1) }
        }
        walk(window as Any, 0)
        return found
    }

    /// The element's role ("AXButton", "AXStaticText", …), "" when it has none.
    static func role(_ element: NSObject) -> String {
        (attribute(element, "accessibilityRole") as? NSAccessibility.Role)?.rawValue ?? ""
    }

    /// The element's label, title and value joined with " | ".
    static func label(_ element: NSObject) -> String {
        ["accessibilityLabel", "accessibilityTitle", "accessibilityValue"].compactMap {
            attribute(element, $0).flatMap { "\($0)" }
        }.filter { !$0.isEmpty }.joined(separator: " | ")
    }

    /// Every label in the window, for a failure message.
    func labels() -> [String] {
        elements().map(Self.label).filter { !$0.isEmpty }
    }

    /// The elements whose label contains `text`, of `role` when given.
    func find(_ text: String, role: String? = nil) -> [NSObject] {
        elements().filter { element in
            (role == nil || Self.role(element) == role) && Self.label(element).contains(text)
        }
    }

    /// Whether any element's label contains `text`.
    func has(_ text: String, role: String? = nil) -> Bool {
        !find(text, role: role).isEmpty
    }

    /// The first element whose label contains `text`; fails the test when none.
    func element(_ text: String, role: String? = nil, file: StaticString = #filePath,
                 line: UInt = #line) throws -> NSObject {
        try XCTUnwrap(find(text, role: role).first, "no \(role ?? "element") \"\(text)\" in \(labels())",
                      file: file, line: line)
    }

    /// Whether the element is enabled (true when it does not say).
    static func isEnabled(_ element: NSObject) -> Bool {
        // The getter is `isAccessibilityEnabled`; KVC's key `accessibilityEnabled` finds it.
        guard element.responds(to: NSSelectorFromString("isAccessibilityEnabled")) else { return true }
        return element.value(forKey: "accessibilityEnabled") as? Bool ?? true
    }

    // MARK: Acting

    /// Presses the button whose label contains `title`, then settles.
    func press(_ title: String, file: StaticString = #filePath, line: UInt = #line) throws {
        let button = try element(title, role: "AXButton", file: file, line: line)
        // The return value isn't dependable for SwiftUI's nodes; callers check what the press did.
        _ = button.perform(NSSelectorFromString("accessibilityPerformPress"))
        settle()
    }

    // MARK: Views

    /// Every view of `type` in the window.
    func views<T: NSView>(_ type: T.Type) -> [T] {
        var found: [T] = []
        func walk(_ view: NSView) {
            if let match = view as? T { found.append(match) }
            view.subviews.forEach(walk)
        }
        if let content = window.contentView { walk(content) }
        return found
    }

    /// Chooses `title` in the `index`th pop-up menu (a menu-style `Picker`),
    /// as a user's pick would, without opening the menu, then settles.
    /// (plan-68-W)
    func choose(_ title: String, inPopUp index: Int = 0, file: StaticString = #filePath, line: UInt = #line) throws {
        let popUps = views(NSPopUpButton.self)
        let popUp = try XCTUnwrap(popUps.indices.contains(index) ? popUps[index] : nil, "no pop-up \(index)",
                                  file: file, line: line)
        if let menu = popUp.menu {
            // SwiftUI fills a pop-up's menu when it is about to show.
            menu.delegate?.menuNeedsUpdate?(menu)
            menu.update()
        }
        let item = try XCTUnwrap(popUp.itemTitles.firstIndex(of: title), "no \"\(title)\" in \(popUp.itemTitles)",
                                 file: file, line: line)
        popUp.selectItem(at: item)
        if let chosen = popUp.selectedItem, let action = chosen.action {
            NSApp.sendAction(action, to: chosen.target, from: chosen)
        } else if let action = popUp.action {
            popUp.sendAction(action, to: popUp.target)
        }
        settle()
    }

    /// Types `text` into the `index`th editable text field, as `type(_:intoField:)` does.
    func type(_ text: String, intoFieldAt index: Int, file: StaticString = #filePath, line: UInt = #line) throws {
        let fields = views(NSTextField.self).filter(\.isEditable)
        let field = try XCTUnwrap(fields.indices.contains(index) ? fields[index] : nil, "no field \(index) of \(fields.count)",
                                  file: file, line: line)
        try typeAndReturn(text, into: field, file: file, line: line)
    }

    /// Types `text` into the text field whose placeholder (else label) is `placeholder`
    /// (replacing what it holds) and presses Return, as a user would, then
    /// settles. (plan-68-W)
    func type(_ text: String, intoField placeholder: String, file: StaticString = #filePath, line: UInt = #line) throws {
        let fields = views(NSTextField.self).filter(\.isEditable)
        let field = try XCTUnwrap(fields.first { $0.placeholderString == placeholder }
                                      ?? fields.first { $0.accessibilityLabel() == placeholder },
                                  "no field \"\(placeholder)\" in \(fields.map { $0.placeholderString ?? $0.accessibilityLabel() ?? "?" })",
                                  file: file, line: line)
        try typeAndReturn(text, into: field, file: file, line: line)
    }

    private func typeAndReturn(_ text: String, into field: NSTextField, file: StaticString, line: UInt) throws {
        XCTAssertTrue(window.makeFirstResponder(field), file: file, line: line)
        let editor = try XCTUnwrap(field.currentEditor() as? NSTextView, file: file, line: line)
        editor.selectAll(nil)
        editor.insertText(text, replacementRange: editor.selectedRange())
        editor.insertNewline(nil)
        settle()
    }
}
