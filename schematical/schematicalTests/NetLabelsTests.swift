import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `NetLabels` on hand-built labels, with no document (plan-68-D).
@MainActor
final class NetLabelsTests: XCTestCase {
    func testNamesAddAndJoins() {
        var value = NetLabels(labels: [])
        XCTAssertEqual(value.nextName, "N1")
        let first = value.add(at: CGPoint(x: 41, y: 59), style: .input)
        XCTAssertEqual(first.name, "N1")
        XCTAssertEqual(first.origin, CGPoint(x: 40, y: 60))
        let second = value.add(at: CGPoint(x: 200, y: 0), style: .output)
        XCTAssertEqual(second.name, "N2")
        XCTAssertTrue(value.joins(second.id, named: "N1"))
        XCTAssertFalse(value.joins(second.id, named: "VCC"))
        XCTAssertFalse(value.joins(UUID(), named: "N1"))
    }

    func testEdits() {
        var value = NetLabels(labels: [])
        let label = value.add(at: .zero, style: .input)
        XCTAssertTrue(value.setName(label.id, "CLK"))
        XCTAssertTrue(value.setName(label.id, "CLK"), "the same name")
        XCTAssertFalse(value.setName(UUID(), "X"))
        XCTAssertEqual(value.labels[0].name, "CLK")
        value.setStyle(label.id, .output)
        value.setStyle(label.id, .output)
        value.setStyle(UUID(), .input)
        XCTAssertEqual(value.labels[0].style, .output)
        value.flip(label.id)
        value.flip(UUID())
        XCTAssertTrue(value.labels[0].isFlipped)
        value.move([label.id: CGPoint(x: 20, y: 20), UUID(): .zero], by: { CGPoint(x: $0.x + 20, y: $0.y) })
        XCTAssertEqual(value.labels[0].origin, CGPoint(x: 40, y: 20))
        value.move([label.id: CGPoint(x: 20, y: 20)], by: { CGPoint(x: $0.x + 20, y: $0.y) })
        XCTAssertEqual(value.labels[0].origin, CGPoint(x: 40, y: 20), "already there")
        value.remove([label.id])
        XCTAssertTrue(value.labels.isEmpty)
    }

    func testHits() {
        var value = NetLabels(labels: [])
        let label = value.add(at: CGPoint(x: 100, y: 100), style: .input)
        let rect = value.labels[0].rect
        XCTAssertEqual(value.label(at: CGPoint(x: rect.midX, y: rect.midY))?.id, label.id)
        XCTAssertNil(value.label(at: CGPoint(x: -500, y: -500)))
        let tip = value.labels[0].tip
        XCTAssertEqual(value.tip(at: CGPoint(x: tip.x + 2, y: tip.y), radius: 8), label.id)
        XCTAssertNil(value.tip(at: CGPoint(x: -500, y: -500), radius: 8))
        XCTAssertEqual(value.tip(label.id), tip)
        XCTAssertNil(value.tip(UUID()))
    }
}

/// `SheetNotes` on hand-built text blocks, with no document (plan-68-D).
@MainActor
final class SheetNotesTests: XCTestCase {
    func testEdits() {
        var value = SheetNotes(notes: [])
        let note = value.add(at: CGPoint(x: 104, y: 57))
        XCTAssertEqual(note.origin, CGPoint(x: 100, y: 60))
        XCTAssertEqual(note.text, SchematicNote.placeholder)
        value.setText(note.id, "Hello")
        value.setText(note.id, "")
        value.setText(note.id, "Hello")
        value.setText(UUID(), "X")
        XCTAssertEqual(value.notes[0].text, "Hello")
        value.setSize(note.id, 10_000)
        XCTAssertEqual(value.notes[0].size, SchematicNote.sizeRange.upperBound)
        value.setSize(UUID(), 1)
        value.setFramed(note.id, true)
        value.setFramed(UUID(), false)
        XCTAssertTrue(value.notes[0].framed)
        value.move([note.id: note.origin, UUID(): .zero], by: { CGPoint(x: $0.x, y: $0.y + 20) })
        XCTAssertEqual(value.notes[0].origin, CGPoint(x: 100, y: 80))
        value.move([note.id: CGPoint(x: 100, y: 60)], by: { CGPoint(x: $0.x, y: $0.y + 20) })
        XCTAssertEqual(value.notes[0].origin, CGPoint(x: 100, y: 80), "already there")
        let rect = value.notes[0].rect
        XCTAssertEqual(value.note(at: CGPoint(x: rect.midX, y: rect.midY))?.id, note.id)
        XCTAssertNil(value.note(at: CGPoint(x: -500, y: -500)))
        value.remove([note.id])
        XCTAssertTrue(value.notes.isEmpty)
    }
}
