import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// The reference above every part but power symbols, its Display Reference
/// toggle, and the oscillator's frequency on its symbol.
@MainActor
final class DisplayReferenceTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testReferenceSitsAboveEverySymbolButPower() throws {
        let doc = TestSupport.newDocument()
        let block = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        let zener = try TestSupport.place(doc, "1N4728A", at: p(600, 200))
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: p(800, 200))

        let blockAnchor = try XCTUnwrap(ReferenceLabel.anchor(for: block))
        // Centred on the body, outside its top edge.
        XCTAssertEqual(blockAnchor.x, ChipLayout.labelMargin + block.layout.size.width / 2)
        XCTAssertLessThan(blockAnchor.y, 0)
        XCTAssertNotNil(ReferenceLabel.anchor(for: led))
        XCTAssertNotNil(ReferenceLabel.anchor(for: zener))
        XCTAssertNotNil(ReferenceLabel.anchor(for: cap))
        // Power symbols have none.
        XCTAssertNil(ReferenceLabel.anchor(for: try TestSupport.place(doc, "GND", at: p(1000, 200))))
        XCTAssertNil(ReferenceLabel.anchor(for: try TestSupport.place(doc, "5V", at: p(1200, 200))))
        // A pull resistor's sits over its resistance, left-aligned with it.
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(1400, 200))
        XCTAssertEqual(ReferenceLabel.anchor(for: pull), CGPoint(x: ChipLayout.labelMargin + 30, y: 54))
        XCTAssertEqual(ReferenceLabel.textAnchor(for: pull), .bottomLeading)
        XCTAssertEqual(ReferenceLabel.fontSize, 8.25)
    }

    func testToggleHidesTheReferenceAndUndoes() throws {
        let doc = TestSupport.newDocument()
        let block = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        XCTAssertTrue(block.showsReference)
        doc.endCoalescing()

        doc.setPartShowsReference(block.id, false)
        let hidden = try XCTUnwrap(doc.chips.first)
        XCTAssertFalse(hidden.showsReference)
        XCTAssertNil(ReferenceLabel.anchor(for: hidden))
        XCTAssertEqual(doc.undoName(on: .schematic), "Hide Reference")

        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertTrue(try XCTUnwrap(doc.chips.first).showsReference)
    }

    func testFileKeepsAHiddenReferenceAndOmitsTheDefault() throws {
        let doc = TestSupport.newDocument()
        let shown = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let hidden = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        doc.setPartShowsReference(hidden.id, false)

        let file = SchematicFile(document: doc, fileURL: nil)
        XCTAssertNil(file.parts.first { $0.id == shown.id }?.showReference)
        XCTAssertEqual(file.parts.first { $0.id == hidden.id }?.showReference, false)

        let opened = TestSupport.newDocument()
        try SchematicFile.decode(try file.encoded()).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.chips.first { $0.id == shown.id }?.showsReference, true)
        XCTAssertEqual(opened.chips.first { $0.id == hidden.id }?.showsReference, false)
    }

    func testPasteKeepsDisplayReference() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        doc.setPartShowsReference(led.id, false)
        doc.endCoalescing()
        doc.select(.chip(led.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 200)))
        XCTAssertEqual(doc.chips.map(\.showsReference), [false, false])
    }

    func testOscillatorFrequency() throws {
        XCTAssertEqual(ChipBlockView.oscillatorFrequency(try TestSupport.part("OSC-SIT1534-2012")), "32.768 kHz")
        XCTAssertNil(ChipBlockView.oscillatorFrequency(try TestSupport.part("SN74HC00N")))
    }
}
