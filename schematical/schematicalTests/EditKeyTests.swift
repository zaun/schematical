import AppKit
import XCTest
@testable import schematical

/// ⌘R / ⇧⌘R rotate on the PCB, A steps the trace angle, ⌘F flips the
/// selected schematic part, ⌘⌫ deletes, ⌘X / ⌘C / ⌘V cut, copy and paste and
/// ⌘A selects all; anything else is left alone.
final class EditKeyTests: XCTestCase {
    func testFlipAndRotate() {
        XCTAssertEqual(ContentView.editKey(keyCode: 15, "r", [.command], isRepeat: false), .rotate)
        // With Shift held, charactersIgnoringModifiers is the capital.
        XCTAssertEqual(ContentView.editKey(keyCode: 15, "R", [.command, .shift], isRepeat: false), .rotateBack)
        XCTAssertEqual(ContentView.editKey(keyCode: 3, "f", [.command], isRepeat: false), .flip)
        // Caps Lock doesn't count as a modifier.
        XCTAssertEqual(ContentView.editKey(keyCode: 15, "r", [.command, .capsLock], isRepeat: false), .rotate)
        XCTAssertEqual(ContentView.editKey(keyCode: 3, "f", [.command, .capsLock], isRepeat: false), .flip)
        XCTAssertNil(ContentView.editKey(keyCode: 15, "r", [.command, .option], isRepeat: false), "⌥⌘R no longer rotates")
        XCTAssertNil(ContentView.editKey(keyCode: 3, "F", [.command, .shift], isRepeat: false))
    }

    func testAngleAndDelete() {
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "a", [], isRepeat: false), .angle)
        // 51 is the Delete key; only ⌘⌫ deletes.
        XCTAssertEqual(ContentView.editKey(keyCode: 51, "\u{7F}", [.command], isRepeat: false), .delete)
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [.command, .shift], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 51, "\u{7F}", [.command], isRepeat: true))
        XCTAssertNil(ContentView.editKey(keyCode: 2, "d", [.command], isRepeat: false), "⌘D does not delete")
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

    func testSelectAll() {
        XCTAssertEqual(ContentView.editKey(keyCode: 0, "a", [.command], isRepeat: false), .selectAll)
        XCTAssertNil(ContentView.editKey(keyCode: 0, "a", [.command], isRepeat: true))
    }

    func testOtherKeysDoNot() {
        for key in ["r", "f"] {
            // The bare keys no longer rotate or flip.
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [], isRepeat: false))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.command], isRepeat: true))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.control], isRepeat: false))
            XCTAssertNil(ContentView.editKey(keyCode: 0, key, [.shift], isRepeat: false))
        }
        XCTAssertNil(ContentView.editKey(keyCode: 0, "t", [], isRepeat: false))
        XCTAssertNil(ContentView.editKey(keyCode: 0, nil, [], isRepeat: false))
    }
}
