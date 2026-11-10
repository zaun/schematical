import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB sidebar's Lock Position: a locked footprint, text, trace or via
/// stays where it is.
@MainActor
final class LockPositionTests: XCTestCase {
    private func doc() -> SchematicDocument { TestSupport.newDocument() }

    func testNewItemsAreUnlocked() throws {
        let doc = doc()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 0, y: 20), CGPoint(x: 10, y: 20)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 30)))
        for item in [BoardItem.footprint(chip.id), .text(text), .trace(trace), .via(via)] {
            XCTAssertFalse(doc.isLocked(item), "\(item)")
        }
    }

    func testLockedItemsDoNotDrag() throws {
        let doc = doc()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 0, y: 20), CGPoint(x: 10, y: 20)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 30)))
        let items: [BoardItem] = [.footprint(chip.id), .text(text), .trace(trace), .via(via)]
        for item in items {
            doc.setLocked(item, true)
            XCTAssertTrue(doc.isLocked(item))
        }
        let before = (doc.chips.first?.board?.centre, doc.texts.first?.anchor, doc.traces.first?.points,
                      doc.vias.first?.centre)

        // Alone, and as a group.
        for item in items {
            doc.select(item)
            let base = doc.boardGroupBase(grab: .zero)
            XCTAssertTrue(base.isEmpty)
            doc.moveBoardGroup(base, by: CGSize(width: 5, height: 5))
        }
        doc.select(items[0])
        for item in items.dropFirst() { doc.toggleSelection(item) }
        doc.moveBoardGroup(doc.boardGroupBase(), by: CGSize(width: 5, height: 5))

        // Direct moves and rotations are refused too.
        doc.moveFootprint(chip.id, to: CGPoint(x: 50, y: 50))
        doc.moveFootprintOrigin(chip.id, to: CGPoint(x: 50, y: 50))
        doc.rotateFootprint(chip.id)
        doc.moveText(text, to: CGPoint(x: 50, y: 50))
        doc.rotateText(text)
        doc.setTextRotation(text, 180)
        doc.moveTraces([trace], by: CGSize(width: 5, height: 5))
        doc.moveVia(via, to: CGPoint(x: 50, y: 50))
        doc.setViaCentre(via, CGPoint(x: 50, y: 50))

        let after = (doc.chips.first?.board?.centre, doc.texts.first?.anchor, doc.traces.first?.points,
                     doc.vias.first?.centre)
        XCTAssertEqual(after.0, before.0)
        XCTAssertEqual(after.1, before.1)
        XCTAssertEqual(after.2, before.2)
        XCTAssertEqual(after.3, before.3)
        XCTAssertEqual(doc.chips.first?.board?.rotation, 0)
        XCTAssertEqual(doc.texts.first?.rotation, 0)
    }

    /// In a group, only the unlocked items move.
    func testGroupMovesOnlyUnlocked() throws {
        let doc = doc()
        let a = doc.addText(at: CGPoint(x: 5, y: 5))
        let b = doc.addText(at: CGPoint(x: 20, y: 5))
        doc.setLocked(.text(a), true)
        doc.select(.text(a))
        doc.toggleSelection(.text(b))
        let start = doc.texts.map(\.anchor)
        doc.moveBoardGroup(doc.boardGroupBase(), by: CGSize(width: 2.54, height: 0))
        XCTAssertEqual(doc.texts.first { $0.id == a }?.anchor, start[0])
        XCTAssertNotEqual(doc.texts.first { $0.id == b }?.anchor, start[1])
    }

    func testUnlockAllowsMoving() throws {
        let doc = doc()
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        doc.setLocked(.text(text), true)
        doc.setLocked(.text(text), false)
        let start = try XCTUnwrap(doc.texts.first?.anchor)
        doc.moveText(text, to: CGPoint(x: 20, y: 20))
        XCTAssertNotEqual(doc.texts.first?.anchor, start)
    }

    func testRoundTrip() throws {
        let doc = doc()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 0, y: 20), CGPoint(x: 10, y: 20)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 30)))
        let open = try BoardLayersTests.roundTrip(doc)
        for item in [BoardItem.footprint(chip.id), .text(text), .trace(trace), .via(via)] {
            XCTAssertFalse(open.isLocked(item))
            doc.setLocked(item, true)
        }
        let opened = try BoardLayersTests.roundTrip(doc)
        for item in [BoardItem.footprint(chip.id), .text(text), .trace(trace), .via(via)] {
            XCTAssertTrue(opened.isLocked(item), "\(item)")
        }
    }

    func testUnlockedFileHasNoLockedKey() throws {
        let doc = doc()
        _ = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        _ = doc.addText(at: CGPoint(x: 5, y: 5))
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        XCTAssertFalse(String(decoding: data, as: UTF8.self).contains("locked"))
    }
}
