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

    // MARK: The file

    private func encoded(_ doc: SchematicDocument) throws -> Data {
        try SchematicFile(document: doc, fileURL: nil).encoded()
    }

    private func reopened(_ data: Data) throws -> SchematicDocument {
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    func testADefaultDocumentWritesNoDocumentInfo() throws {
        let text = try String(data: encoded(newDocument()), encoding: .utf8) ?? ""
        XCTAssertFalse(text.contains("documentInfo"))
    }

    func testEveryFieldRoundTrips() throws {
        let doc = newDocument()
        doc.setInfoTitle("Clock board")
        doc.setInfoAuthor("J. Smith")
        doc.setInfoRevision("B-draft")
        doc.setInfoDescription("Line one\nLine two")
        doc.setAutomaticDates(false)
        doc.setInfoCreated(Date(timeIntervalSince1970: 1_795_615_629))
        doc.setInfoUpdated(Date(timeIntervalSince1970: 1_795_619_999))
        doc.editDocumentInfo("Show") { $0.titleBlock = CGPoint(x: 400, y: 520) }
        let opened = try reopened(try encoded(doc))
        XCTAssertEqual(opened.documentInfo, doc.documentInfo)
        XCTAssertEqual(opened.documentInfo.titleBlock, CGPoint(x: 400, y: 520))
        XCTAssertFalse(opened.documentInfo.automaticDates)
        // Saved twice, the same bytes.
        XCTAssertEqual(try encoded(opened), try encoded(doc))
    }

    func testDatesAreWrittenAsISO8601UTC() throws {
        let doc = newDocument()
        doc.setInfoCreated(Date(timeIntervalSince1970: 1_795_615_629))
        let text = try String(data: encoded(doc), encoding: .utf8) ?? ""
        XCTAssertTrue(text.contains("\"created\" : \"2026-11-25T14:07:09Z\""), text)
        XCTAssertFalse(text.contains("automaticDates"), "automatic is the default and is left out")
        XCTAssertFalse(text.contains("titleBlock"))
    }

    func testTitleBlockSnapsToTheGridOnOpen() throws {
        let doc = newDocument()
        doc.editDocumentInfo("Show") { $0.titleBlock = CGPoint(x: 104, y: 57) }
        let opened = try reopened(try encoded(doc))
        XCTAssertEqual(opened.documentInfo.titleBlock, CGPoint(x: 100, y: 60))
    }

    func testABadDateIsRefused() throws {
        let doc = newDocument()
        doc.setInfoCreated(Date(timeIntervalSince1970: 1_000_000))
        let text = try String(data: encoded(doc), encoding: .utf8) ?? ""
        let broken = text.replacingOccurrences(of: "1970-01-12T13:46:40Z", with: "yesterday")
        XCTAssertNotEqual(broken, text)
        let file = try SchematicFile.decode(Data(broken.utf8))
        XCTAssertThrowsError(try file.apply(to: newDocument(), catalog: TestSupport.catalog, fileURL: nil)) { error in
            guard case SchematicFileError.badDocumentInfo = error else { return XCTFail("\(error)") }
        }
    }

    func testAFileWithoutTheKeyOpensWithTheDefault() throws {
        let doc = newDocument()
        doc.setInfoTitle("Clock")
        let opened = newDocument()
        var file = try SchematicFile.decode(try encoded(doc))
        file.documentInfo = nil
        try file.apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.documentInfo, DocumentInfo())
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
