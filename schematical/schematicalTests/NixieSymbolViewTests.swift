import SwiftUI
import XCTest
@testable import schematical

/// How a nixie tube is drawn (`NixieSymbolView`): the box, its pins, the dark
/// front cathode, the glowing ones at their brightness and the decimal points.
@MainActor
final class NixieSymbolViewTests: XCTestCase {
    private func place(_ name: String) throws -> PlacedChip {
        let doc = TestSupport.newDocument()
        return try TestSupport.place(doc, name, at: CGPoint(x: 200, y: 200))
    }

    private func lamp(_ chip: PlacedChip) -> Ink.Role { .lamp(DiodeSymbolView.lightName(for: chip)) }

    private func paint(_ chip: PlacedChip, selected: Bool = false, hovered: Bool = false, connected: Set<Int> = [],
                       levels: [Double] = []) -> NixieRecorder {
        var painter = NixieRecorder()
        NixieSymbolView.paint(&painter, chip: chip, isSelected: selected, isHovered: hovered,
                              connectedPins: connected, levels: levels)
        return painter
    }

    /// A dark tube: the part number, every pin's designator and name, one
    /// dim glyph (the front cathode) and two dim decimal points.
    func testDarkTubeDrawsOneDimGlyphAndTheDots() throws {
        let chip = try place("IN-14")
        let nixie = try XCTUnwrap(chip.part.nixie)
        XCTAssertTrue(nixie.glyphs.contains("DP") && nixie.glyphs.contains("LDP"))
        let drawn = paint(chip)
        XCTAssertEqual(drawn.texts.first?.string, chip.partNumber)
        let slots = chip.layout.slots
        let designators = drawn.texts.filter { $0.ink == .secondary }.map(\.string)
        XCTAssertEqual(Set(designators), Set(slots.map(\.pin.designator)))
        let glyphs = drawn.texts.filter { $0.ink.role == .unlit }
        XCTAssertEqual(glyphs.map(\.string), [nixie.glyphs.first { $0 != "DP" && $0 != "LDP" }])
        XCTAssertEqual(glyphs.first?.size, 26)
        XCTAssertEqual(drawn.fills.filter { $0.role == .unlit }.count, 2, "the two decimal points")
        XCTAssertFalse(drawn.fills.contains { $0.role == lamp(chip) })
        XCTAssertEqual(drawn.strokes.first?.width, 1.5, "a plain outline")
        XCTAssertEqual(drawn.strokes.first?.ink.role, .primary)
        XCTAssertEqual(drawn.fills.filter { $0.role == .accent }.count, 0)
    }

    /// A glowing glyph and dot: the front cathode is not drawn dark, the lit
    /// ones are in the tube's colour at their level.
    func testLitGlyphAndDotGlowAtTheirLevel() throws {
        let chip = try place("IN-14")
        let nixie = try XCTUnwrap(chip.part.nixie)
        var levels = [Double](repeating: 0, count: nixie.glyphs.count)
        let three = try XCTUnwrap(nixie.glyphs.firstIndex(of: "3"))
        let dp = try XCTUnwrap(nixie.glyphs.firstIndex(of: "DP"))
        levels[three] = 0.5
        levels[dp] = 0.25
        let drawn = paint(chip, levels: levels)
        XCTAssertTrue(drawn.texts.filter { $0.ink.role == .unlit }.isEmpty, "no dark front cathode while one glows")
        let lit = drawn.texts.filter { $0.ink.role == lamp(chip) }
        XCTAssertEqual(lit.map(\.string), ["3"])
        XCTAssertEqual(lit.first?.ink.opacity, 0.5)
        XCTAssertEqual(drawn.fills.filter { $0.role == lamp(chip) }.map(\.opacity), [0.25])
    }

    /// A lit decimal point alone leaves the front cathode dark.
    func testALitDotAloneKeepsTheFrontCathodeDark() throws {
        let chip = try place("IN-14")
        let nixie = try XCTUnwrap(chip.part.nixie)
        var levels = [Double](repeating: 0, count: nixie.glyphs.count)
        levels[try XCTUnwrap(nixie.glyphs.firstIndex(of: "LDP"))] = 1
        let drawn = paint(chip, levels: levels)
        XCTAssertEqual(drawn.texts.filter { $0.ink.role == .unlit }.count, 1)
        XCTAssertEqual(drawn.fills.filter { $0.role == lamp(chip) }.map(\.opacity), [1])
    }

    /// A long symbol ("Hz") is drawn smaller.
    func testLongSymbolsAreSmaller() throws {
        let chip = try place("IN-15B")
        let nixie = try XCTUnwrap(chip.part.nixie)
        var levels = [Double](repeating: 0, count: nixie.glyphs.count)
        levels[try XCTUnwrap(nixie.glyphs.firstIndex(of: "Hz"))] = 1
        let drawn = paint(chip, levels: levels)
        let lit = drawn.texts.filter { $0.ink.role == lamp(chip) }
        XCTAssertEqual(lit.map(\.string), ["Hz"])
        XCTAssertEqual(lit.first?.size, 15)
    }

    /// Hovered and selected: an accent outline, 2 wide; hovered also tints
    /// the box. A wired pin's designator is not drawn.
    func testHoverSelectionAndWiredPins() throws {
        let chip = try place("IN-14")
        let hovered = paint(chip, hovered: true)
        XCTAssertEqual(hovered.fills.filter { $0.role == .accent }.map(\.opacity), [0.12])
        XCTAssertEqual(hovered.strokes.first?.width, 2)
        XCTAssertEqual(hovered.strokes.first?.ink.role, .accent)
        let selected = paint(chip, selected: true)
        XCTAssertEqual(selected.fills.filter { $0.role == .accent }.count, 0)
        XCTAssertEqual(selected.strokes.first?.width, 2)

        let first = try XCTUnwrap(chip.layout.slots.first)
        let wired = paint(chip, connected: [first.pin.index])
        let all = paint(chip)
        XCTAssertEqual(wired.texts.count, all.texts.count - 1)
        XCTAssertFalse(wired.texts.contains { $0.ink == .secondary && $0.string == first.pin.designator })
    }

    /// The view draws the tube in a frame as wide as the box plus a label
    /// margin each side: the face's dark fill shows in the middle.
    func testTheViewDrawsTheTube() throws {
        let chip = try place("IN-18")
        let layout = chip.layout
        let view = NixieSymbolView(chip: chip, isSelected: false, isHovered: false)
        let renderer = ImageRenderer(content: view)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        XCTAssertEqual(CGFloat(image.width), layout.size.width + 2 * ChipLayout.labelMargin)
        XCTAssertEqual(CGFloat(image.height), layout.size.height)
        let bitmap = NSBitmapImageRep(cgImage: image)
        let middle = try XCTUnwrap(bitmap.colorAt(x: image.width / 2, y: image.height / 2 + 30))
        XCTAssertLessThan(middle.usingColorSpace(.sRGB)?.brightnessComponent ?? 1, 0.4, "the dark face")
    }
}

/// Records what a nixie drawing paints.
private struct NixieRecorder: SchematicPainter {
    struct Stroke { let ink: Ink; let width: CGFloat }
    var fills: [Ink] = []
    var strokes: [Stroke] = []
    var texts: [PaintedText] = []
    mutating func fill(_ path: Path, with ink: Ink) { fills.append(ink) }
    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        strokes.append(Stroke(ink: ink, width: style.lineWidth))
    }
    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) { texts.append(text) }
    mutating func translateBy(x: CGFloat, y: CGFloat) {}
    mutating func rotate(by angle: Angle) {}
}
