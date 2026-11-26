import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-64-C: the title block on the Schematic page: its layout, drawing,
/// showing and hiding, selecting, moving and deleting.
@MainActor
final class TitleBlockTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func filledInfo() -> DocumentInfo {
        var info = DocumentInfo()
        info.title = "Clock board"
        info.author = "J. Smith"
        info.revision = "B"
        info.description = "Divides the 4 MHz clock by 8 and drives the counter."
        info.created = Date(timeIntervalSince1970: 1_795_615_629)
        info.updated = Date(timeIntervalSince1970: 1_795_619_999)
        return info
    }

    // MARK: Layout

    func testLayoutIsOnTheGridAndAFixedWidth() {
        let origin = p(100, 200)
        let layout = TitleBlockLayout(filledInfo(), origin: origin)
        XCTAssertEqual(layout.rect.width, 360)
        XCTAssertEqual(layout.rect.origin, origin)
        XCTAssertEqual(layout.rect.height.truncatingRemainder(dividingBy: 10), 0)
        XCTAssertEqual(layout.cells.count, 6)
        func onGrid(_ v: CGFloat, _ base: CGFloat) -> Bool { (v - base).truncatingRemainder(dividingBy: 10) == 0 }
        for cell in layout.cells {
            XCTAssertTrue(onGrid(cell.rect.minX, origin.x) && onGrid(cell.rect.minY, origin.y), cell.label)
            XCTAssertTrue(onGrid(cell.rect.maxX, origin.x) && onGrid(cell.rect.maxY, origin.y), cell.label)
        }
        for (a, b) in layout.separators {
            XCTAssertTrue(onGrid(a.x, origin.x) && onGrid(a.y, origin.y) && onGrid(b.x, origin.x) && onGrid(b.y, origin.y))
        }
        XCTAssertEqual(layout.cells.map(\.label), ["Title", "Author", "Revision", "Created", "Updated", "Description"])
        // The cells of the last row end where the frame does.
        XCTAssertEqual(layout.cells.last?.rect.maxY, layout.rect.maxY)
    }

    func testAnEmptyInfoHasTheMinimumHeightAndOneLinePerCell() {
        let layout = TitleBlockLayout(DocumentInfo(), origin: .zero)
        XCTAssertTrue(layout.cells.allSatisfy { $0.lines == [""] })
        let busy = TitleBlockLayout(filledInfo(), origin: .zero)
        XCTAssertLessThanOrEqual(layout.rect.height, busy.rect.height)
        XCTAssertGreaterThan(layout.rect.height, 0)
    }

    func testALongDescriptionWrapsAndGrowsTheBlock() {
        var info = filledInfo()
        let short = TitleBlockLayout(info, origin: .zero)
        info.description = Array(repeating: "word", count: 40).joined(separator: " ")
        let long = TitleBlockLayout(info, origin: .zero)
        let lines = long.cells.last?.lines ?? []
        XCTAssertGreaterThan(lines.count, 3)
        XCTAssertEqual(lines.joined(separator: " "), info.description, "wrapping at spaces loses nothing")
        XCTAssertGreaterThan(long.rect.height, short.rect.height)
        XCTAssertEqual(long.rect.width, 360)
    }

    func testAWordWiderThanALineIsCutByCharacters() {
        let word = String(repeating: "x", count: 100)
        let lines = TitleBlockLayout.wrap(word, width: 100, size: 12)
        XCTAssertGreaterThan(lines.count, 1)
        XCTAssertEqual(lines.joined(), word)
        let font = SchematicNote.metrics(12).font
        for line in lines { XCTAssertLessThanOrEqual(SchematicNote.width(of: line, in: font), 100, line) }
    }

    func testLineBreaksAreKeptAndEmptyTextIsOneLine() {
        XCTAssertEqual(TitleBlockLayout.wrap("a\n\nb", width: 300, size: 12), ["a", "", "b"])
        XCTAssertEqual(TitleBlockLayout.wrap("", width: 300, size: 12), [""])
    }

    // MARK: Showing and hiding

    func testAnEmptySchematicShowsTheBlockAtTheOriginAsOneNamedStep() {
        let doc = newDocument()
        XCTAssertNil(doc.titleBlockLayout)
        doc.setTitleBlockShown(true)
        XCTAssertEqual(doc.documentInfo.titleBlock, .zero)
        XCTAssertEqual(doc.undoName, "Show Title Block")
        XCTAssertNotNil(doc.titleBlockRect)
        XCTAssertTrue(doc.schematicSelection.isEmpty, "showing it does not select it")
        XCTAssertTrue(doc.undo())
        XCTAssertNil(doc.documentInfo.titleBlock)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.documentInfo.titleBlock, .zero)
    }

    func testItShowsAtTheBottomRightOfTheDrawing() throws {
        let doc = newDocument()
        _ = try TestSupport.place(doc, try TestSupport.firstDip().name, at: p(200, 200))
        var painter = BoundsPainter()
        SchematicScene(doc).paint(into: &painter)
        let ink = try XCTUnwrap(painter.bounds)
        doc.setTitleBlockShown(true)
        let origin = try XCTUnwrap(doc.documentInfo.titleBlock)
        XCTAssertEqual(origin.x + TitleBlockLayout.width, ink.maxX, accuracy: 5, "its right edge is the drawing's")
        XCTAssertEqual(origin.y, ink.maxY + 20, accuracy: 5, "a little below it")
        XCTAssertEqual(origin.x.truncatingRemainder(dividingBy: 10), 0)
        XCTAssertEqual(origin.y.truncatingRemainder(dividingBy: 10), 0)
    }

    func testHidingKeepsTheInformation() {
        let doc = newDocument()
        doc.setInfoTitle("Clock board")
        doc.endCoalescing()
        doc.setTitleBlockShown(true)
        doc.setTitleBlockShown(false)
        XCTAssertNil(doc.documentInfo.titleBlock)
        XCTAssertEqual(doc.documentInfo.title, "Clock board")
        XCTAssertEqual(doc.undoName, "Hide Title Block")
        // Showing again places it afresh.
        doc.setTitleBlockShown(true)
        XCTAssertEqual(doc.documentInfo.titleBlock, .zero)
        // Already so: no step.
        let name = doc.undoName
        doc.setTitleBlockShown(true)
        XCTAssertEqual(doc.undoName, name)
    }

    // MARK: Selecting, moving, deleting (Phase 2)

    private func shownDocument(with part: Bool = false) throws -> (SchematicDocument, PlacedChip?) {
        let doc = newDocument()
        let chip = part ? try TestSupport.place(doc, try TestSupport.firstDip().name, at: p(200, 200)) : nil
        doc.setInfoTitle("Clock board")
        doc.endCoalescing()
        doc.setTitleBlockShown(true)
        return (doc, chip)
    }

    func testAPointInsideHitsTheBlockAndOutsideDoesNot() throws {
        let (doc, _) = try shownDocument()
        let rect = try XCTUnwrap(doc.titleBlockRect)
        XCTAssertTrue(doc.titleBlock(at: p(rect.midX, rect.midY)))
        XCTAssertFalse(doc.titleBlock(at: p(rect.maxX + 5, rect.midY)))
        doc.setTitleBlockShown(false)
        XCTAssertFalse(doc.titleBlock(at: p(rect.midX, rect.midY)), "hidden: nothing there")
    }

    func testBoxSelectAndSelectAllIncludeItOnlyWhenShown() throws {
        let doc = newDocument()
        doc.selectAllSchematic()
        XCTAssertFalse(doc.schematicSelection.contains(.titleBlock))
        doc.setTitleBlockShown(true)
        let rect = try XCTUnwrap(doc.titleBlockRect)
        XCTAssertTrue(doc.schematicItems(in: rect.insetBy(dx: 20, dy: 20)).contains(.titleBlock))
        XCTAssertFalse(doc.schematicItems(in: CGRect(x: rect.maxX + 50, y: 0, width: 10, height: 10)).contains(.titleBlock))
        doc.selectAllSchematic()
        XCTAssertTrue(doc.schematicSelection.contains(.titleBlock))
    }

    func testPressSelectsAndAGroupDragMovesItWithAPartAndUndoesInOneStep() throws {
        let (doc, chip) = try shownDocument(with: true)
        let part = try XCTUnwrap(chip)
        let before = try XCTUnwrap(doc.documentInfo.titleBlock)
        _ = doc.pressSelection(.titleBlock, shift: false)
        XCTAssertTrue(doc.selectedTitleBlock)
        _ = doc.pressSelection(.chip(part.id), shift: true)
        let base = doc.schematicGroupBase()
        XCTAssertEqual(base.titleBlock, before)
        doc.beginGesture(.schematic)
        doc.moveSchematicGroup(base, by: CGSize(width: 44, height: -33))
        doc.moveSchematicGroup(base, by: CGSize(width: 54, height: -28))
        doc.endGesture()
        XCTAssertEqual(doc.documentInfo.titleBlock, p(before.x + 50, before.y - 30))
        XCTAssertEqual(doc.chips.first?.origin, p(part.origin.x + 50, part.origin.y - 30))
        XCTAssertEqual(doc.undoName, "Move")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.documentInfo.titleBlock, before)
        XCTAssertEqual(doc.chips.first?.origin, part.origin)
        XCTAssertEqual(doc.undoName, "Show Title Block", "the whole drag was one step")
    }

    func testDeleteWithAPartSelectedRemovesThePartAndHidesTheBlockKeepingTheInfo() throws {
        let (doc, chip) = try shownDocument(with: true)
        doc.selectAllSchematic()
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertNil(doc.documentInfo.titleBlock)
        XCTAssertEqual(doc.documentInfo.title, "Clock board")
        XCTAssertFalse(doc.schematicSelection.contains(.titleBlock))
        XCTAssertNotNil(chip)
        XCTAssertTrue(doc.undo())
        XCTAssertNotNil(doc.documentInfo.titleBlock)
        XCTAssertEqual(doc.chips.count, 1)
    }

    func testDeleteOnTheBlockAloneHidesIt() throws {
        let (doc, _) = try shownDocument()
        doc.select(.titleBlock)
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertNil(doc.documentInfo.titleBlock)
        XCTAssertEqual(doc.documentInfo.title, "Clock board")
        XCTAssertTrue(doc.schematicSelection.isEmpty)
    }

    func testTheEraserHidesTheBlock() throws {
        let (doc, _) = try shownDocument()
        let rect = try XCTUnwrap(doc.titleBlockRect)
        doc.erase(at: p(rect.midX, rect.midY))
        XCTAssertNil(doc.documentInfo.titleBlock)
        XCTAssertEqual(doc.documentInfo.title, "Clock board")
    }

    func testANoteOverTheBlockIsHitFirst() throws {
        let (doc, _) = try shownDocument()
        let rect = try XCTUnwrap(doc.titleBlockRect)
        let id = doc.addNote(at: p(rect.minX + 20, rect.minY + 20))
        XCTAssertEqual(doc.note(at: p(rect.minX + 25, rect.minY + 25))?.id, id)
        XCTAssertTrue(doc.titleBlock(at: p(rect.minX + 25, rect.minY + 25)), "both are under the point; the canvas takes the note first")
    }

    func testCutLeavesTheBlockAndCopyIgnoresIt() throws {
        let (doc, chip) = try shownDocument(with: true)
        doc.select(.titleBlock)
        XCTAssertNil(doc.schematicCopy(), "the block alone is not copyable")
        doc.selectAllSchematic()
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertEqual(copy.chips.count, 1)
        XCTAssertNotNil(chip)
        XCTAssertTrue(doc.deleteSchematicSelection(named: "Cut", includingTitleBlock: false))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertNotNil(doc.documentInfo.titleBlock, "cut does not take the block")
    }

    func testTheSidebarAndTheSelectionList() throws {
        let (doc, chip) = try shownDocument(with: true)
        doc.select(.titleBlock)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .titleBlock)
        _ = doc.pressSelection(.chip(try XCTUnwrap(chip).id), shift: true)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .selection)
        let rows = SelectionRows.schematic(doc)
        XCTAssertTrue(rows.contains { $0.item == .titleBlock && $0.title == "Title Block" && $0.detail == "Clock board" })
        // Hidden with the block selected: the selection clears of it.
        doc.select(.titleBlock)
        doc.setTitleBlockShown(false)
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .library)
    }

    func testTheBlockSurvivesTheFile() throws {
        let (doc, _) = try shownDocument()
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.documentInfo.titleBlock, doc.documentInfo.titleBlock)
        XCTAssertEqual(opened.titleBlockLayout, doc.titleBlockLayout)
    }

    // MARK: Drawing

    func testThePaintedTextHasTheTitleLabelsAndDates() {
        let layout = TitleBlockLayout(filledInfo(), origin: p(10, 10))
        var painter = RecordingPainter()
        TitleBlockLayer.paint(&painter, layout: layout)
        let text = painter.record.calls.filter { $0.kind == .text }.map(\.string)
        for expected in ["Title", "Author", "Revision", "Created", "Updated", "Description",
                         "Clock board", "J. Smith", "B",
                         DocumentInfo.dateText(filledInfo().created), DocumentInfo.dateText(filledInfo().updated)] {
            XCTAssertTrue(text.contains(expected), expected)
        }
    }

    func testASceneWithOnlyATitleBlockIsNotEmptyAndMeasuresTheBlock() throws {
        let doc = newDocument()
        XCTAssertTrue(SchematicScene(doc).isEmpty)
        doc.setInfoTitle("T")
        doc.setTitleBlockShown(true)
        let scene = SchematicScene(doc)
        XCTAssertFalse(scene.isEmpty)
        var painter = BoundsPainter()
        scene.paint(into: &painter)
        let ink = try XCTUnwrap(painter.bounds)
        let rect = try XCTUnwrap(doc.titleBlockRect)
        XCTAssertTrue(ink.contains(rect.insetBy(dx: 2, dy: 2)))
    }
}
