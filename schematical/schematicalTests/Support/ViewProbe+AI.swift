import AppKit
import XCTest

/// plan-68-AI: additive `ViewProbe` helpers for the PCB sidebar: find a
/// segmented control, pop-up or field by what it shows, and pick a segment by
/// its title.
extension ViewProbe {
    /// The `skip`th segmented control (0 first) with a segment titled `label`.
    func segments(with label: String, after skip: Int = 0, file: StaticString = #filePath,
                  line: UInt = #line) throws -> NSSegmentedControl {
        let all = views(NSSegmentedControl.self).filter { control in
            (0..<control.segmentCount).contains { control.label(forSegment: $0) == label }
        }
        return try XCTUnwrap(all.indices.contains(skip) ? all[skip] : nil, "no segment \(label)", file: file, line: line)
    }

    /// Picks the segment titled `label` (the first control offering it, or `control`), as a click would.
    func pick(_ label: String, in control: NSSegmentedControl? = nil, file: StaticString = #filePath,
              line: UInt = #line) throws {
        let control = try control ?? segments(with: label, file: file, line: line)
        let index = try XCTUnwrap((0..<control.segmentCount).first { control.label(forSegment: $0) == label },
                                  file: file, line: line)
        choose(segment: index, in: control)
    }

    /// The pop-up whose items include `title`.
    func popUp(offering title: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSPopUpButton {
        // A menu-style picker fills its items only as it opens (`filled`).
        try XCTUnwrap(popUps().map(filled).first { $0.itemTitles.contains(title) },
                      "no pop-up offering \(title): \(popUps().map(\.itemTitles))", file: file, line: line)
    }

    /// The editable text field showing `value`.
    func field(showing value: String, file: StaticString = #filePath, line: UInt = #line) throws -> NSTextField {
        try XCTUnwrap(textFields().first { $0.stringValue == value },
                      "no field showing \(value): \(textFields().map(\.stringValue))", file: file, line: line)
    }

    /// Types `text` into the field showing `value` and presses Return.
    func retype(_ value: String, as text: String, file: StaticString = #filePath, line: UInt = #line) throws {
        let field = try field(showing: value, file: file, line: line)
        type(text, into: field)
        submit(field)
    }
}
