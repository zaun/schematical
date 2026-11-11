import XCTest
@testable import schematical

/// Each page has its own help sheet, and each lists the editing keys that
/// page handles.
final class KeyboardHelpTests: XCTestCase {
    private func keys(_ page: EditorPage) -> [String] {
        page.helpSections.flatMap { $0.entries.map(\.keys) }
    }

    func testEverySectionHasLines() {
        for page in EditorPage.allCases {
            XCTAssertFalse(page.helpSections.isEmpty, page.rawValue)
            for section in page.helpSections {
                XCTAssertFalse(section.entries.isEmpty, section.title)
                for entry in section.entries {
                    XCTAssertFalse(entry.keys.isEmpty)
                    XCTAssertFalse(entry.action.isEmpty)
                }
            }
        }
    }

    func testSchematicKeys() {
        let keys = keys(.schematic)
        for key in ["F", "⌘⌫", "⌘C", "⌘V", "Space", "/", "Esc", "⌘R", "⌘?"] {
            XCTAssertTrue(keys.contains(key), key)
        }
        // The PCB's keys are not on the schematic sheet.
        XCTAssertFalse(keys.contains("A"))
    }

    func testPCBKeys() {
        let keys = keys(.pcb)
        for key in ["R or ⌥⌘R", "A", "⌘C", "⌘V", "⌘⌫", "Space", "/", "Esc", "⇧⌘D", "⌘?"] {
            XCTAssertTrue(keys.contains(key), key)
        }
        XCTAssertFalse(keys.contains("F"))
    }

    func testTitles() {
        XCTAssertEqual(EditorPage.schematic.helpTitle, "Schematic Help")
        XCTAssertEqual(EditorPage.pcb.helpTitle, "PCB Help")
    }
}
