import SwiftUI
import XCTest
@testable import schematical

/// One painter call, as a test records it.
struct PaintCall {
    enum Kind { case fill, stroke, text }
    var kind: Kind
    var ink: Ink
    var lineWidth: CGFloat = 0
    var string = ""
}

/// Records every call; copies share the record.
struct RecordingPainter: SchematicPainter {
    nonisolated final class Record {
        var calls: [PaintCall] = []
    }

    let record = Record()

    mutating func fill(_ path: Path, with ink: Ink) {
        record.calls.append(PaintCall(kind: .fill, ink: ink))
    }

    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        record.calls.append(PaintCall(kind: .stroke, ink: ink, lineWidth: style.lineWidth))
    }

    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        record.calls.append(PaintCall(kind: .text, ink: text.ink, string: text.string))
    }

    mutating func translateBy(x: CGFloat, y: CGFloat) {}
    mutating func rotate(by angle: Angle) {}
}

/// The parts of plan-28-A § 3.4 item 1: every `PartSymbol` kind.
let everySymbolPart = [
    "SN74HC00N", "ECS-100A-010", "GENERIC-LED", "TE-GDS0204", "LTC-4727JR", "NHD-0216K1Z-FL-YBW",
    "PJ-002A", "GENERIC-HEADER-1X02", "5V", "GND", "PULLUP-5V", "PULLDOWN",
]

/// plan-28-B: the export scene and the ink-bounds painter.
@MainActor
final class SchematicSceneTests: XCTestCase {
    private func assertRect(_ rect: CGRect?, _ expected: CGRect, file: StaticString = #filePath, line: UInt = #line) {
        guard let rect else { return XCTFail("nil rect", file: file, line: line) }
        for (a, b) in [(rect.minX, expected.minX), (rect.minY, expected.minY),
                       (rect.width, expected.width), (rect.height, expected.height)] {
            XCTAssertEqual(a, b, accuracy: 1e-9, "\(rect) ≠ \(expected)", file: file, line: line)
        }
    }

    func testBoundsOfAStroke() {
        var painter = BoundsPainter()
        let path = Path(CGRect(x: 10, y: 20, width: 30, height: 40))
        painter.stroke(path, with: .primary, lineWidth: 2)
        assertRect(painter.bounds, CGRect(x: 9, y: 19, width: 32, height: 42))

        var shifted = BoundsPainter()
        shifted.translateBy(x: 5, y: 5)
        shifted.stroke(path, with: .primary, lineWidth: 2)
        assertRect(shifted.bounds, CGRect(x: 14, y: 24, width: 32, height: 42))
    }

    func testPaperAddsNothing() {
        var painter = BoundsPainter()
        painter.fill(Path(CGRect(x: 0, y: 0, width: 10, height: 10)), with: .paper)
        painter.fill(Path(CGRect(x: 0, y: 0, width: 10, height: 10)), with: .display(0.1, 0.2, 0.3))
        painter.stroke(Path(CGRect(x: 0, y: 0, width: 10, height: 10)), with: .color(.red), lineWidth: 1)
        XCTAssertNil(painter.bounds)
    }

    func testBoundsOfText() {
        let text = PaintedText("ABCD", size: 10, monospaced: true)
        var painter = BoundsPainter()
        painter.draw(text, at: CGPoint(x: 100, y: 50), anchor: .center)
        assertRect(painter.bounds, CGRect(x: 88, y: 44, width: 24, height: 12))

        // Turned about its point, as a port label is: a copy is transformed,
        // and the original's bounds see it.
        let turnedPainter = BoundsPainter()
        var turned = turnedPainter
        turned.translateBy(x: 100, y: 50)
        turned.rotate(by: .degrees(-90))
        turned.draw(text, at: .zero, anchor: .center)
        assertRect(turnedPainter.bounds, CGRect(x: 94, y: 38, width: 12, height: 24))
    }

    func testOneChipBounds() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let scene = SchematicScene(doc)
        XCTAssertFalse(scene.isEmpty)
        var painter = BoundsPainter()
        scene.paint(into: &painter)
        let bounds = try XCTUnwrap(painter.bounds)
        XCTAssertTrue(bounds.contains(chip.hitRect), "\(bounds) does not contain \(chip.hitRect)")
        // The body outline (1.5 wide) is stroked on the frame's top and
        // bottom edges, so half of it lies outside; the live Canvas clips it.
        let frame = CGRect(x: chip.viewOrigin.x, y: chip.viewOrigin.y,
                           width: chip.layout.size.width + 2 * ChipLayout.labelMargin,
                           height: chip.layout.size.height)
            .insetBy(dx: -0.75, dy: -0.75)
        XCTAssertTrue(frame.contains(bounds), "\(bounds) is not inside the view frame \(frame)")
        XCTAssertEqual(bounds.minY, frame.minY, accuracy: 1e-9, "the outline's top edge is the ink's top")
    }

    func testEmptyScene() {
        let scene = SchematicScene(TestSupport.newDocument())
        XCTAssertTrue(scene.isEmpty)
        var painter = BoundsPainter()
        scene.paint(into: &painter)
        XCTAssertNil(painter.bounds)
    }

    /// A document with one of each symbol and one wire; the DIP's lever 1 on or off.
    private func everySymbol(leverOn: Bool) throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        var placed: [PlacedChip] = []
        for (index, name) in everySymbolPart.enumerated() {
            placed.append(try TestSupport.place(doc, name, at: CGPoint(x: 200 + 300 * (index % 4),
                                                                      y: 200 + 400 * (index / 4))))
        }
        let nand = placed[0]
        let led = placed[2]
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(nand, "1Y")), .pin(PinRef(chipID: led.id, index: led.layout.slots[0].pin.index))))
        XCTAssertEqual(doc.wires.count, 1)
        if leverOn {
            XCTAssertEqual(doc.toggleSwitch(placed[3].id, position: 1), true)
        }
        return doc
    }

    private func calls(_ doc: SchematicDocument) -> [PaintCall] {
        var recorder = RecordingPainter()
        SchematicScene(doc).paint(into: &recorder)
        return recorder.record.calls
    }

    func testExportStateInks() throws {
        let doc = try everySymbol(leverOn: true)
        XCTAssertEqual(Set(doc.chips.map(\.symbol)).count, 12)
        let on = calls(doc)
        for call in on {
            switch call.ink.role {
            case .accent: XCTFail("accent ink in an export: \(call)")
            case .color: XCTFail("screen-only color in an export: \(call)")
            case .lamp: XCTFail("lamp ink in an export: \(call)")
            default: break
            }
        }
        XCTAssertTrue(on.contains { $0.ink.role == .secondary })
        XCTAssertTrue(on.contains { $0.ink.role == .switchOn })
        XCTAssertTrue(on.contains { $0.ink.role == .knobOn })

        let off = calls(try everySymbol(leverOn: false))
        XCTAssertFalse(off.contains { $0.ink.role == .switchOn })
        XCTAssertFalse(off.contains { $0.ink.role == .knobOn })
    }

    func testPaintOrder() throws {
        let doc = TestSupport.newDocument()
        let nand = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let other = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 500, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(nand, "1Y")), .pin(try TestSupport.pin(other, "1A"))))
        XCTAssertNotNil(doc.addBus(from: CGPoint(x: 100, y: 500), to: CGPoint(x: 400, y: 500)))
        let recorded = calls(doc)
        let busStroke = try XCTUnwrap(recorded.firstIndex { $0.kind == .stroke && $0.lineWidth == Bus.lineWidth })
        let wireStroke = try XCTUnwrap(recorded.firstIndex {
            $0.kind == .stroke && $0.lineWidth == 1.5 && $0.ink.role == .primary
        })
        let chipText = try XCTUnwrap(recorded.firstIndex { $0.kind == .text && $0.string == "SN74HC00N" })
        XCTAssertLessThan(busStroke, wireStroke)
        XCTAssertLessThan(wireStroke, chipText)
        // Every bus stroke comes before the first wire stroke.
        let lastBusStroke = try XCTUnwrap(recorded.lastIndex { $0.kind == .stroke && $0.lineWidth == Bus.lineWidth })
        XCTAssertLessThan(lastBusStroke, wireStroke)
    }

    /// A wired pin's number is left off; an unwired pin keeps it.
    func testWiredPinsDrawNoNumber() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let y = try TestSupport.pin(chip, "1Y"), a = try TestSupport.pin(chip, "2A")
        XCTAssertNil(doc.connect(.pin(y), .pin(a)))
        XCTAssertEqual(doc.connectedPins[chip.id], [y.index, a.index])

        let designator = { (ref: PinRef) in chip.layout.slots.first { $0.pin.index == ref.index }!.pin.designator }
        let other = try TestSupport.pin(chip, "1A")
        let texts = calls(doc).filter { $0.kind == .text }.map(\.string)
        XCTAssertFalse(texts.contains(designator(y)))
        XCTAssertFalse(texts.contains(designator(a)))
        XCTAssertTrue(texts.contains(designator(other)))
    }

    /// A block's function is drawn a word per line.
    func testFunctionIsAWordPerLine() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "MAX3100CPD+", at: CGPoint(x: 200, y: 200))
        let words = chip.part.function.split(separator: " ").map(String.init)
        XCTAssertEqual(words, ["Bridge", "UART"])
        let texts = calls(doc).filter { $0.kind == .text }.map(\.string)
        XCTAssertFalse(texts.contains(chip.part.function))
        for word in words { XCTAssertTrue(texts.contains(word), "no line \(word)") }
    }
}
