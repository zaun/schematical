import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// Collects the root element's attributes while parsing.
private nonisolated final class RootReader: NSObject, XMLParserDelegate {
    var root: String?
    var attributes: [String: String] = [:]

    func parser(_ parser: XMLParser, didStartElement name: String, namespaceURI: String?,
                qualifiedName: String?, attributes: [String: String] = [:]) {
        if root == nil {
            root = name
            self.attributes = attributes
        }
    }
}

/// plan-28-C: `SchematicExport.svg` documents.
@MainActor
final class SchematicSVGTests: XCTestCase {
    /// Kept for the life of the test process (an XMLParser delegate is weak).
    private static var readers: [RootReader] = []

    private func svgText(_ doc: SchematicDocument) throws -> String {
        let data = try XCTUnwrap(SchematicExport.svg(SchematicScene(doc)))
        return String(decoding: data, as: UTF8.self)
    }

    /// Parses the SVG; fails on an XML error. Returns the root's attributes.
    private func parse(_ text: String, file: StaticString = #filePath, line: UInt = #line) -> [String: String] {
        let reader = RootReader()
        Self.readers.append(reader)
        let parser = XMLParser(data: Data(text.utf8))
        parser.delegate = reader
        XCTAssertTrue(parser.parse(), "not well-formed: \(parser.parserError.map { "\($0)" } ?? "?")", file: file, line: line)
        XCTAssertEqual(reader.root, "svg", file: file, line: line)
        return reader.attributes
    }

    /// Every `fill="…"` and `stroke="…"` value.
    private func paintValues(_ text: String) -> Set<String> {
        let regex = try! NSRegularExpression(pattern: " (fill|stroke)=\"([^\"]*)\"")
        let range = NSRange(text.startIndex..., in: text)
        return Set(regex.matches(in: text, range: range).map {
            String(text[Range($0.range(at: 2), in: text)!])
        })
    }

    func testEmptyDocumentHasNoSVG() {
        XCTAssertNil(SchematicExport.svg(SchematicScene(TestSupport.newDocument())))
        XCTAssertNil(SchematicExport.bounds(of: SchematicScene(TestSupport.newDocument())))
    }

    func testOneChipAndAWire() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(chip, "1Y")), .pin(try TestSupport.pin(chip, "2A"))))
        XCTAssertEqual(doc.wires.count, 1)
        let text = try svgText(doc)
        let root = parse(text)
        let bounds = try XCTUnwrap(SchematicExport.bounds(of: SchematicScene(doc)))
        let width = SVGPainter.number(ceil(bounds.width)), height = SVGPainter.number(ceil(bounds.height))
        XCTAssertEqual(root["viewBox"], "0 0 \(width) \(height)")
        XCTAssertEqual(root["width"], width)
        XCTAssertEqual(root["height"], height)
        XCTAssertFalse(text.contains("<rect"))
        XCTAssertFalse(text.contains("<style"))
        XCTAssertEqual(paintValues(text), ["none", "#000"])
        XCTAssertTrue(text.contains(">SN74HC00N</text>"))
        XCTAssertTrue(text.hasPrefix("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\""))
    }

    private func everySymbol(leverOn: Bool) throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        for (index, name) in everySymbolPart.enumerated() {
            let chip = try TestSupport.place(doc, name, at: CGPoint(x: 200 + 400 * (index % 4), y: 200 + 400 * (index / 4)))
            if name == "TE-GDS0204", leverOn {
                XCTAssertEqual(doc.toggleSwitch(chip.id, position: 1), true)
            }
        }
        return doc
    }

    func testEverySymbol() throws {
        let on = try svgText(try everySymbol(leverOn: true))
        _ = parse(on)
        XCTAssertEqual(paintValues(on), ["none", "#000"])
        XCTAssertTrue(on.contains(">SN74HC00N</text>"))
        let led = try TestSupport.part("GENERIC-LED")
        XCTAssertTrue(on.contains(">\(SVGPainter.escaped(led.function))</text>"), "no LED function text \(led.function)")
        let dip = try TestSupport.part("TE-GDS0204")
        XCTAssertTrue(on.contains(">TE-GDS0204</text>"))
        for pin in dip.pins {
            XCTAssertTrue(on.contains(">\(pin.designator)</text>"), "no designator \(pin.designator)")
        }
        XCTAssertTrue(on.contains("fill-opacity=\"0.25\""), "no ON lever track")

        let off = try svgText(try everySymbol(leverOn: false))
        XCTAssertNotEqual(on, off)
        XCTAssertFalse(off.contains("fill-opacity=\"0.25\""))
    }

    /// The LCD's 1 280 dots are not exported; its face is one outline.
    func testCharacterLCDIsSmall() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "NHD-0216K1Z-FL-YBW", at: CGPoint(x: 300, y: 300))
        let text = try svgText(doc)
        let paths = text.components(separatedBy: "<path").count - 1
        XCTAssertLessThan(paths, 100)
        XCTAssertGreaterThan(paths, 0)
    }

    /// The viewBox is the measured ink plus 12 on each side, rounded up.
    func testPadding() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let scene = SchematicScene(doc)
        var painter = BoundsPainter()
        scene.paint(into: &painter)
        let ink = try XCTUnwrap(painter.bounds)
        let root = parse(try svgText(doc))
        let numbers = try XCTUnwrap(root["viewBox"]).split(separator: " ").compactMap { Double($0) }
        XCTAssertEqual(numbers.count, 4)
        XCTAssertEqual(numbers[2], ceil(ink.width + 24))
        XCTAssertEqual(numbers[3], ceil(ink.height + 24))
    }
}
