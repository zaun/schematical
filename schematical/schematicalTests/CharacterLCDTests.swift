import CoreGraphics
import XCTest
@testable import schematical

/// Character LCD modules in the app (plan-35-G): two-enable pin columns,
/// the gray and white panels, RGB backlights, and four-row frames.
@MainActor
final class CharacterLCDTests: XCTestCase {
    private func info(_ name: String) throws -> CharacterLCDInfo {
        try XCTUnwrap(try TestSupport.part(name).characterLCD, "\(name) is not a character LCD")
    }

    /// The left column's pin names top to bottom, "" for an empty row.
    private func leftColumn(_ name: String) throws -> [String] {
        let part = try TestSupport.part(name)
        let layout = ChipLayout(part: part, symbol: .characterLCD)
        let left = layout.slots.filter { $0.side == .left }.sorted { $0.centerY < $1.centerY }
        guard let first = left.first else { return [] }
        let pitch = ChipLayout.pitch
        var names: [String] = []
        var y = first.centerY
        for slot in left {
            while slot.centerY - y > pitch / 2 {
                names.append("")
                y += pitch
            }
            names.append(slot.pin.name)
            y = slot.centerY + pitch
        }
        return names
    }

    private func facts(_ name: String) throws -> [String: String] {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, name, at: CGPoint(x: 300, y: 300))
        return Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
    }

    /// (a) A 40 x 4 module: two enables, 12 bus pins, RS R/W E1 E2 on the left.
    func testTwoEnableLeftColumn() throws {
        let lcd = try info("NHD-0440WH-ATFH-JT#")
        XCTAssertEqual(lcd.enables, 2)
        XCTAssertEqual(lcd.busPins.count, 12)
        XCTAssertEqual(lcd.rows, 4)
        XCTAssertEqual(lcd.columns, 40)
        let left = try leftColumn("NHD-0440WH-ATFH-JT#")
        XCTAssertEqual(Array(left.prefix(6)), ["RS", "R/W", "E1", "E2", "", "DB0"])
        XCTAssertEqual(left.last, "DB7")
    }

    /// (b) A 16 x 2 module's left column is unchanged: RS R/W E, a gap, DB0.
    func testOneEnableLeftColumnUnchanged() throws {
        let lcd = try info("NHD-0216K1Z-NSW-BBW-L")
        XCTAssertEqual(lcd.enables, 1)
        XCTAssertEqual(lcd.busPins.count, 11)
        let left = try leftColumn("NHD-0216K1Z-NSW-BBW-L")
        XCTAssertEqual(Array(left.prefix(5)), ["RS", "R/W", "E", "", "DB0"])
    }

    /// (c) The gray and white panels and their facts.
    func testGrayAndWhitePanels() throws {
        XCTAssertEqual(try info("C204CXBSGSW6WT33XAA").panel, .gray)
        XCTAssertEqual(try info("NHD-0420DZ-FSW-FBW").panel, .white)
        XCTAssertEqual(LCDPanel.gray.title, "Gray")
        XCTAssertEqual(LCDPanel.white.title, "White")
        XCTAssertEqual(try facts("C204CXBSGSW6WT33XAA")["Panel"], "STN gray, positive")
        XCTAssertEqual(try facts("NHD-0420DZ-FSW-FBW")["Panel"], "FSTN white, positive")
        XCTAssertEqual(try facts("NHD-0216K1Z-NSW-BBW-L")["Panel"], "STN blue, negative")
        XCTAssertEqual(try facts("NHD-0216K1Z-NSW-BBW-L")["Backlight"], "White")
    }

    /// (d) An RGB module: three colors, and green lit gives a green face.
    func testRGBBacklight() throws {
        let lcd = try info("MD42004A6W-FPTLRGB")
        XCTAssertEqual(lcd.backlightColors, ["Red", "Green", "Blue"])
        XCTAssertEqual(lcd.backlightColor, "Red")
        XCTAssertEqual(lcd.backlightTitle, "RGB")
        XCTAssertEqual(try facts("MD42004A6W-FPTLRGB")["Backlight"], "Red, Green, Blue")
        let green = CharacterLCDSymbolView.faceColors(lcd, backlights: [0, 1, 0]).background
        XCTAssertGreaterThan(green.g, green.r)
        XCTAssertGreaterThan(green.g, green.b)
        let red = CharacterLCDSymbolView.faceColors(lcd, backlights: [1, 0, 0]).background
        XCTAssertGreaterThan(red.r, red.g)
        XCTAssertGreaterThan(red.r, red.b)
        // All off: the panel's dark background, as with one LED off.
        let off = CharacterLCDSymbolView.faceColors(lcd, backlights: [0, 0, 0]).background
        XCTAssertEqual(off, CharacterLCDSymbolView.faceColors(lcd, backlights: []).background)
    }

    /// A one-LED face ignores its color and uses the panel palette.
    func testSingleLEDUsesPanelPalette() throws {
        let lcd = try info("NHD-0216K1Z-NSW-BBW-L")
        let lit = CharacterLCDSymbolView.faceColors(lcd, backlights: [1]).background
        let dark = CharacterLCDSymbolView.faceColors(lcd, backlights: [0]).background
        XCTAssertNotEqual(lit, dark)
        XCTAssertGreaterThan(lit.b, lit.r)  // still the blue panel
    }

    /// (e) A four-row frame keeps all four rows.
    func testFourRowFrame() {
        var state = LcdState()
        state.columns = 20
        state.rows = 4
        let frame = LCDFrame(state)
        XCTAssertEqual(frame.rows, 4)
        XCTAssertEqual(frame.columns, 20)
        XCTAssertEqual(frame.dots.count, 4 * Int(LCD_STATE_CELL_HEIGHT) * 20 * Int(LCD_STATE_CELL_WIDTH))
    }
}
