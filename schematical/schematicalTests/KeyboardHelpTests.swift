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
        for key in ["⌘F", "⌘⌫", "⌘A", "⌘X", "⌘C", "⌘V", "Space", "/", "Esc", "⌘P", "⌘?", "⌥⌘E", "⌘I"] {
            XCTAssertTrue(keys.contains(key), key)
        }
        // The PCB's keys are not on the schematic sheet.
        XCTAssertFalse(keys.contains("A"))
    }

    /// plan-64-B: ⌘I is a window command, listed on both pages.
    func testPCBListsDocumentInfo() {
        XCTAssertTrue(keys(.pcb).contains("⌘I"))
    }

    /// plan-47-M: a click on a fuse in play mode blows or restores it.
    func testSchematicFuseLine() {
        let entry = EditorPage.schematic.helpSections.flatMap(\.entries)
            .first { $0.keys == "Click a fuse (in play mode)" }
        XCTAssertEqual(entry?.action, "Blow or restore it")
        XCTAssertFalse(keys(.pcb).contains("Click a fuse (in play mode)"))
    }

    /// plan-47-N: the Simulation section says buzzers play through the speakers.
    func testSimulationBuzzerLine() {
        let entry = EditorPage.schematic.helpSections.first { $0.title == "Simulation" }?.entries
            .first { $0.keys == "Buzzers (in play mode)" }
        XCTAssertEqual(entry?.action, "Buzzers play through the Mac's speakers; turn them off in Settings")
        XCTAssertFalse(keys(.pcb).contains("Buzzers (in play mode)"))
    }

    func testPCBKeys() {
        let keys = keys(.pcb)
        for key in ["⌘R", "⇧⌘R", "A", "⌘A", "⌘X", "⌘C", "⌘V", "⌘⌫", "Space", "/", "Esc", "⇧⌘D", "⌘?",
                    "⌘-drag a reference", "⌥⌘R", "⌥⇧⌘R", "⌥⌘U", "Route", "V", "Trace assist"] {
            XCTAssertTrue(keys.contains(key), key)
        }
        XCTAssertFalse(keys.contains("⌘F"))
    }

    func testTitles() {
        XCTAssertEqual(EditorPage.schematic.helpTitle, "Schematic Help")
        XCTAssertEqual(EditorPage.pcb.helpTitle, "PCB Help")
    }
}
