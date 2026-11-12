import AppKit
import XCTest
@testable import schematical

/// A bare R presses the PCB Rotate button, A steps the trace angle, F flips
/// the selected schematic part, ⌘⌫ / ⌘D delete, ⌘X / ⌘C / ⌘V cut, copy and
/// paste and ⌘A selects all; anything else is left alone.
final class EditKeyTests: XCTestCase {
    func testBareKeys() {
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "r", [], isRepeat: false), .rotate)
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "f", [], isRepeat: false), .flip)
        // Caps Lock doesn't count as a modifier.
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "r", [.capsLock], isRepeat: false), .rotate)
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "f", [.capsLock], isRepeat: false), .flip)
    }

    func testAngleAndDelete() {
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "a", [], isRepeat: false), .angle)
        // 51 is the Delete key; only ⌘⌫ deletes.
        XCTAssertEqual(ContentView.editKey(keyCode: 51, "\u{7F}", [.command], isRepeat: false), .delete)
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [.command, .shift], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [.command], isRepeat: true))
    }

    func testCopyAndPaste() {
        XCTAssertEqual(ContentView.editKey(keyCode: 8, "c", [.command], isRepeat: false), .copy)
        XCTAssertEqual(ContentView.editKey(keyCode: 9, "v", [.command], isRepeat: false), .paste)
        XCTAssertNil(ContentView.editKey(keyCode: 9, "v", [.command, .shift], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 9, "v", [.command, .option], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 9, "v", [.command], isRepeat: true))
        XCTAssertEqual(ContentView.editKey(keyCode: 7, "x", [.command], isRepeat: false), .cut)
        XCTAssertNil(ContentView.editKey(keyCode: 7, "x", [.command, .shift], isRepeat: false))
    }

    func testDeleteAndSelectAll() {
        XCTAssertEqual(ContentView.editKey(keyCode: 2, "d", [.command], isRepeat: false), .delete)
        XCTAssertNil(ContentView.editKey(keyCode: 2, "d", [.command, .shift], isRepeat: false), "⇧⌘D is the Design Rule Check")
        XCTAssertNil(ContentView.editKey(keyCode: 2, "d", [], isRepeat: false))
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "a", [.command], isRepeat: false), .selectAll)
        XCTAssertNil(ContentView.editKey(keyCode: 0, "a", [.command], isRepeat: true))
    }

    func testOtherKeysDoNot() {
        for key in ["r", "f"] {
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [], isRepeat: true))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.command], isRepeat: false))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.command, .option], isRepeat: false))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.control], isRepeat: false))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.shift], isRepeat: false))
        }
        XCTAssertNil(ContentView.editKey(keyCode: 0, "t", [], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 0, nil, [], isRepeat: false))
    }
}
