import AppKit
import XCTest

/// plan-68-AC: additive `ViewProbe` helpers for controls that are not
/// buttons (a segmented picker's radio buttons, a toggle's check box, a
/// stepper's arrows) and for a window's sheet.
extension ViewProbe {
    /// Presses the control whose label is `title` (or, failing that, contains it), of `role` when given and
    /// never static text, then settles. For a radio button, check box or link-style button, whose roles
    /// SwiftUI reports variously.
    func pressControl(_ title: String, role: String? = nil, file: StaticString = #filePath, line: UInt = #line) throws {
        let candidates = find(title, role: role).filter { Self.role($0) != "AXStaticText" && Self.role($0) != "AXGroup" }
        let exact = candidates.first { Self.label($0).split(separator: "|").first?
            .trimmingCharacters(in: .whitespaces) == title }
        // A form's toggle is often an unlabelled check box after its label's static text.
        let all = elements()
        let besideLabel = all.firstIndex { Self.role($0) == "AXStaticText" && Self.label($0).hasPrefix(title) }
            .flatMap { index in all[(index + 1)...].first { ["AXCheckBox", "AXRadioButton"].contains(Self.role($0)) } }
        let control = try XCTUnwrap(exact ?? candidates.first ?? besideLabel,
                                    "no control \"\(title)\" in \(rolesAndLabels())", file: file, line: line)
        _ = control.perform(NSSelectorFromString("accessibilityPerformPress"))
        settle()
    }

    /// "role: label" of every labelled element, for a failure message.
    func rolesAndLabels() -> [String] {
        elements().compactMap { element in
            let label = Self.label(element)
            return label.isEmpty ? nil : "\(Self.role(element)): \(label)"
        }
    }

    /// Steps the first incrementor (a stepper's arrows) up or down, then settles.
    func step(up: Bool, file: StaticString = #filePath, line: UInt = #line) throws {
        let stepper = try XCTUnwrap(elements().first { Self.role($0) == "AXIncrementor" },
                                    "no stepper in \(labels())", file: file, line: line)
        _ = stepper.perform(NSSelectorFromString(up ? "accessibilityPerformIncrement" : "accessibilityPerformDecrement"))
        settle()
    }

    /// Waits up to `seconds` for `condition`, turning the run loop.
    @discardableResult
    func wait(_ seconds: TimeInterval = 3, until condition: () -> Bool) -> Bool {
        let end = Date().addingTimeInterval(seconds)
        while !condition() && Date() < end { settle(0.05) }
        return condition()
    }

    /// The sheet on the window, once one is shown (waits up to 3 s).
    func sheet(file: StaticString = #filePath, line: UInt = #line) throws -> NSWindow {
        wait { window.attachedSheet != nil }
        return try XCTUnwrap(window.attachedSheet, "no sheet on the window", file: file, line: line)
    }

    /// The text every text field and label under `view` shows (a table cell's), depth first.
    static func texts(in view: NSView) -> [String] {
        var found: [String] = []
        func walk(_ object: Any, _ depth: Int) {
            guard depth < 30, let element = object as? NSObject else { return }
            let label = label(element)
            if !label.isEmpty { found.append(label) }
            for child in attribute(element, "accessibilityChildren") as? [Any] ?? [] { walk(child, depth + 1) }
        }
        walk(view, 0)
        return found
    }

    /// The context menu a right click on `row` of `table` would show, built but not shown.
    func contextMenu(_ table: NSTableView, row: Int, file: StaticString = #filePath, line: UInt = #line) throws -> NSMenu {
        let rect = table.rect(ofRow: row)
        let point = table.convert(NSPoint(x: rect.midX, y: rect.midY), to: nil)
        let event = try XCTUnwrap(NSEvent.mouseEvent(with: .rightMouseDown, location: point, modifierFlags: [],
            timestamp: ProcessInfo.processInfo.systemUptime, windowNumber: table.window?.windowNumber ?? 0,
            context: nil, eventNumber: 0, clickCount: 1, pressure: 1), file: file, line: line)
        let menu = try XCTUnwrap(table.menu(for: event), "no context menu", file: file, line: line)
        menu.delegate?.menuNeedsUpdate?(menu)
        settle()
        return menu
    }

    /// Runs `table`'s double-click action (a SwiftUI table's primary action) on `row`. The table's clicked
    /// row is set from a synthesized right click on the row, since the action reads it; a
    /// synthesized left click does not reach a table in a window that is not key.
    func doubleClick(_ table: NSTableView, row: Int, file: StaticString = #filePath, line: UInt = #line) throws {
        let rect = table.rect(ofRow: row)
        let point = table.convert(NSPoint(x: rect.midX, y: rect.midY), to: nil)
        let click = try XCTUnwrap(NSEvent.mouseEvent(with: .rightMouseDown, location: point, modifierFlags: [],
            timestamp: ProcessInfo.processInfo.systemUptime, windowNumber: table.window?.windowNumber ?? 0,
            context: nil, eventNumber: 0, clickCount: 1, pressure: 1), file: file, line: line)
        // The menu itself may be empty (nil); only the clicked row is wanted.
        _ = table.menu(for: click)
        let action = try XCTUnwrap(table.doubleAction, "the table has no double-click action", file: file, line: line)
        XCTAssertEqual(table.clickedRow, row, "the clicked row", file: file, line: line)
        NSApp.sendAction(action, to: table.target, from: table)
        settle()
    }
}
