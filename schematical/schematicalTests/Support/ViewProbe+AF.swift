import AppKit
import XCTest

/// plan-68-AF: additive `ViewProbe` helpers for the Part Wizard's pages:
/// find an element by its accessibility identifier.
extension ViewProbe {
    /// The element whose accessibility identifier is `identifier`; fails the test when none.
    func element(identifier: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSObject {
        try XCTUnwrap(elements().first { Self.attribute($0, "accessibilityIdentifier") as? String == identifier },
                      "no element \(identifier) in \(rolesAndLabels())", file: file, line: line)
    }

    /// The editable field whose placeholder or accessibility label is `name`.
    func field(named name: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSTextField {
        let fields = textFields()
        return try XCTUnwrap(fields.first { field in
            [field.placeholderString, field.accessibilityLabel(), field.accessibilityTitle()].contains(name)
        },
                             "no field \(name) in \(fields.map { "\($0.placeholderString ?? "")/\($0.accessibilityLabel() ?? "")" })",
                             file: file, line: line)
    }
}

extension ViewProbe {
    /// `popUp` with its menu's items filled in (SwiftUI fills a picker's or
    /// a pull-down's items as it opens), for reading `itemTitles`.
    func filled(_ popUp: NSPopUpButton) -> NSPopUpButton {
        if let menu = popUp.menu {
            _ = populated(menu)
            menu.delegate?.menuWillOpen?(menu)
            menu.delegate?.menuDidClose?(menu)
        }
        return popUp
    }
}
