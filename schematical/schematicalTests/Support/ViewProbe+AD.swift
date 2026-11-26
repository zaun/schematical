import AppKit
import XCTest

/// plan-68-AD: additive `ViewProbe` helpers for the Part Editor's field
/// controls: commit a typed value, type into a text view, choose in a pop-up,
/// a pull-down menu or a segmented control, and press icon-only buttons,
/// without the mouse or a menu on screen.
extension ViewProbe {
    /// Types `text` into `field` (`type(_:into:)`), then moves focus away so
    /// a formatted field (a number) commits its value.
    func enter(_ text: String, into field: NSTextField) {
        type(text, into: field)
        blur()
    }

    /// Replaces the text of a text view (a SwiftUI `TextEditor`) as typing would.
    func enter(_ text: String, into textView: NSTextView) {
        window.makeFirstResponder(textView)
        textView.selectAll(nil)
        textView.insertText(text, replacementRange: textView.selectedRange())
        settle()
    }

    /// The pop-up buttons (pickers, and pull-down menus).
    func popUps() -> [NSPopUpButton] {
        views(NSPopUpButton.self)
    }

    /// Chooses the item titled `title` of `popUp` as a click on it would.
    func choose(_ title: String, in popUp: NSPopUpButton, file: StaticString = #filePath, line: UInt = #line) {
        // A pull-down (a SwiftUI `Menu`) fills its items only as it opens.
        guard let menu = popUp.menu.map(populated), let index = menu.items.firstIndex(where: { $0.title == title }) else {
            XCTFail("no item \"\(title)\" in \(popUp.itemTitles)", file: file, line: line)
            return
        }
        menu.performActionForItem(at: index)
        settle()
    }

    /// Selects segment `index` of `control` as a click on it would.
    func choose(segment index: Int, in control: NSSegmentedControl) {
        control.selectedSegment = index
        _ = control.sendAction(control.action, to: control.target)
        settle()
    }

    /// The buttons whose label contains `text`, ignoring case (an SF Symbol's
    /// own label: "minus.circle" reads "Remove").
    func buttons(labelled text: String) -> [NSObject] {
        elements().filter {
            Self.role($0) == "AXButton" && Self.label($0).localizedCaseInsensitiveContains(text)
        }
    }

    /// Presses `element` (a button from `find` or `buttons(labelled:)`), then settles.
    func pressElement(_ element: NSObject) {
        _ = element.perform(NSSelectorFromString("accessibilityPerformPress"))
        settle()
    }
}
