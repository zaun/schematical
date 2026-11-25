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
