import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-64-A: the document's title, author, revision, description and dates
/// (`DocumentInfo`): editing, undo, the date stamp, the file.
@MainActor
final class DocumentInfoTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    private let utc = TimeZone(identifier: "UTC")!

    // MARK: The value

    func testDefaults() {
        let info = DocumentInfo()
        XCTAssertEqual(info.title, "")
        XCTAssertEqual(info.author, "")
        XCTAssertEqual(info.revision, "")
        XCTAssertEqual(info.description, "")
        XCTAssertNil(info.created)
        XCTAssertNil(info.updated)
        XCTAssertTrue(info.automaticDates)
        XCTAssertNil(info.titleBlock)
        XCTAssertEqual(newDocument().documentInfo, info)
    }

    func testDateText() {
        // 2026-11-25 14:07:09 UTC
        let date = Date(timeIntervalSince1970: 1_795_615_629)
        XCTAssertEqual(DocumentInfo.dateText(date, in: utc), "2026-11-25 14:07")
        XCTAssertEqual(DocumentInfo.dateText(date, in: TimeZone(identifier: "Asia/Tokyo")!), "2026-11-25 23:07")
        XCTAssertEqual(DocumentInfo.dateText(nil), "")
    }

    // MARK: Edits and undo

    func testEachSetterIsOneNamedStep() {
        let doc = newDocument()
        let date = Date(timeIntervalSince1970: 1_000_000)
        let cases: [(String, () -> Void)] = [
            ("Edit Title", { doc.setInfoTitle("Clock") }),
            ("Edit Author", { doc.setInfoAuthor("J. Smith") }),
            ("Edit Revision", { doc.setInfoRevision("B") }),
            ("Edit Description", { doc.setInfoDescription("Divides the clock") }),
            ("Edit Created Date", { doc.setInfoCreated(date) }),
            ("Edit Updated Date", { doc.setInfoUpdated(date) }),
            ("Change Automatic Dates", { doc.setAutomaticDates(false) }),
        ]
        for (name, edit) in cases {
            edit()
            doc.endCoalescing()
            XCTAssertEqual(doc.undoName, name)
        }
        XCTAssertEqual(doc.documentInfo.title, "Clock")
        XCTAssertEqual(doc.documentInfo.author, "J. Smith")
        XCTAssertEqual(doc.documentInfo.revision, "B")
        XCTAssertEqual(doc.documentInfo.description, "Divides the clock")
        XCTAssertEqual(doc.documentInfo.created, date)
        XCTAssertEqual(doc.documentInfo.updated, date)
        XCTAssertFalse(doc.documentInfo.automaticDates)
        for _ in cases { XCTAssertTrue(doc.undo()) }
        XCTAssertEqual(doc.documentInfo, DocumentInfo())
        XCTAssertNil(doc.undoName)
    }

    func testTypingInOneFieldIsOneStepAndTwoFieldsAreTwo() {
        let doc = newDocument()
        doc.setInfoTitle("C")
        doc.setInfoTitle("Cl")
        doc.setInfoTitle("Clo")
        doc.setInfoAuthor("J")
        doc.setInfoAuthor("JS")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.documentInfo.author, "")
        XCTAssertEqual(doc.documentInfo.title, "Clo")
        XCTAssertEqual(doc.undoName, "Edit Title")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.documentInfo.title, "")
        XCTAssertNil(doc.undoName)
    }

    func testUndoThenRedoRestoresTheWholeValue() {
        let doc = newDocument()
        doc.setInfoTitle("Clock")
        doc.endCoalescing()
        doc.setInfoRevision("A")
        let full = doc.documentInfo
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.documentInfo.revision, "")
        XCTAssertEqual(doc.redoName, "Edit Revision")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.documentInfo, full)
    }

    func testTheSameValueRecordsNoStep() {
        let doc = newDocument()
        doc.setInfoTitle("")
        doc.setAutomaticDates(true)
        doc.setInfoCreated(nil)
        XCTAssertNil(doc.undoName)
        doc.setInfoTitle("Clock")
        doc.endCoalescing()
        doc.setInfoTitle("Clock")
        XCTAssertTrue(doc.undo())
        XCTAssertNil(doc.undoName, "re-typing the same words was not a step of its own")
    }

    // MARK: The stamp

    func testAutomaticDatesStampCreatedOnceAndUpdatedEveryTime() {
        let doc = newDocument()
        let first = Date(timeIntervalSince1970: 1_000_000)
        let second = Date(timeIntervalSince1970: 2_000_000)
        doc.stampDates(now: first)
        XCTAssertEqual(doc.documentInfo.created, first)
        XCTAssertEqual(doc.documentInfo.updated, first)
        doc.stampDates(now: second)
        XCTAssertEqual(doc.documentInfo.created, first)
        XCTAssertEqual(doc.documentInfo.updated, second)
    }

    func testManualDatesAreNotTouched() {
        let doc = newDocument()
        let mine = Date(timeIntervalSince1970: 500_000)
        doc.setAutomaticDates(false)
        doc.setInfoCreated(mine)
        doc.stampDates(now: Date(timeIntervalSince1970: 2_000_000))
        XCTAssertEqual(doc.documentInfo.created, mine)
        XCTAssertNil(doc.documentInfo.updated)
    }

    func testStampingIsNotAnUndoStep() {
        let doc = newDocument()
        doc.setInfoTitle("Clock")
        let before = doc.undoName
        doc.stampDates(now: Date(timeIntervalSince1970: 1_000_000))
        XCTAssertEqual(doc.undoName, before)
        XCTAssertNotNil(doc.documentInfo.updated)
    }
}
