import AppKit
import XCTest

/// plan-68-AB: additive `ViewProbe` helpers for text fields: find one, type
/// into it as the field editor would, press Return, and move focus away.
extension ViewProbe {
    /// The editable text fields in the window, top to bottom as laid out.
    func textFields() -> [NSTextField] {
        views(NSTextField.self).filter(\.isEditable)
    }

    /// The editable text field showing `value`; fails the test when none.
    func textField(showing value: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSTextField {
        try XCTUnwrap(textFields().first { $0.stringValue == value },
                      "no field showing \"\(value)\" in \(textFields().map(\.stringValue))", file: file, line: line)
    }

    /// Focuses `field` and replaces its text with `text`, as typing would.
    func type(_ text: String, into field: NSTextField) {
        window.makeFirstResponder(field)
        settle(0.1)
        field.stringValue = text
        field.currentEditor()?.string = text
        NotificationCenter.default.post(name: NSControl.textDidChangeNotification, object: field)
        settle()
    }

    /// Presses Return in `field` (its action: SwiftUI's `onSubmit`).
    func submit(_ field: NSTextField) {
        field.sendAction(field.action, to: field.target)
        settle()
    }

    /// Moves focus out of every field.
    func blur() {
        window.makeFirstResponder(nil)
        settle()
    }
}

/// plan-68-AB: menus, built as a click would build them but never shown.
extension ViewProbe {
    /// The context menu a right click on `element` would show, or nil.
    func contextMenu(on element: NSObject) -> NSMenu? {
        guard let frame = Self.attribute(element, "accessibilityFrame") as? NSRect,
              let content = window.contentView else { return nil }
        let point = window.convertPoint(fromScreen: NSPoint(x: frame.midX, y: frame.midY))
        guard let event = NSEvent.mouseEvent(with: .rightMouseDown, location: point, modifierFlags: [], timestamp: 0,
                                             windowNumber: window.windowNumber, context: nil, eventNumber: 0,
                                             clickCount: 1, pressure: 1) else { return nil }
        var view = content.hitTest(point)
        while let current = view {
            if let menu = current.menu(for: event) { return populated(menu) }
            view = current.superview
        }
        return nil
    }

    /// `menu` with its items filled in, as it would be just before it opens.
    func populated(_ menu: NSMenu) -> NSMenu {
        menu.delegate?.menuNeedsUpdate?(menu)
        menu.update()
        settle(0.1)
        return menu
    }

    /// Chooses the item titled `title` in `menu`, as a click on it would.
    func choose(_ title: String, in menu: NSMenu, file: StaticString = #filePath, line: UInt = #line) throws {
        let index = menu.indexOfItem(withTitle: title)
        XCTAssertGreaterThanOrEqual(index, 0, "no \"\(title)\" in \(menu.items.map(\.title))", file: file, line: line)
        guard index >= 0 else { return }
        menu.performActionForItem(at: index)
        settle()
    }
}
