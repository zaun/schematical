import AppKit
import PDFKit
import XCTest
@testable import schematical

/// plan-46-G Phase 1: Find in datasheet walks the hits of the first section
/// title a PDF has, and the datasheet step's address rules.
@MainActor
final class DatasheetFinderTests: XCTestCase {
    /// A PDF with one heading per page, as real text (PDFKit can search it).
    static func pdf(headings: [String]) throws -> PDFDocument {
        let data = NSMutableData()
        var box = CGRect(x: 0, y: 0, width: 612, height: 792)
        let consumer = try XCTUnwrap(CGDataConsumer(data: data as CFMutableData))
        let context = try XCTUnwrap(CGContext(consumer: consumer, mediaBox: &box, nil))
        for heading in headings {
            context.beginPDFPage(nil)
            let graphics = NSGraphicsContext(cgContext: context, flipped: false)
            NSGraphicsContext.saveGraphicsState()
            NSGraphicsContext.current = graphics
            NSAttributedString(string: heading, attributes: [.font: NSFont.systemFont(ofSize: 24)])
                .draw(at: CGPoint(x: 72, y: 700))
            NSGraphicsContext.restoreGraphicsState()
            context.endPDFPage()
        }
        context.closePDF()
        return try XCTUnwrap(PDFDocument(data: data as Data))
    }

    /// Kept for the life of the test process: freeing a main-actor object
    /// inside a test aborts the host (see TestSupport.newDocument()).
    private static var kept: [DatasheetFinder] = []

    private func newFinder() -> DatasheetFinder {
        let finder = DatasheetFinder()
        Self.kept.append(finder)
        return finder
    }

    private func finder() throws -> DatasheetFinder {
        let finder = newFinder()
        finder.document = try Self.pdf(headings: ["Features", "Pin Configuration and Functions", "Pin Functions",
                                                  "Pin Functions (continued)"])
        XCTAssertEqual(finder.document?.pageCount, 4)
        return finder
    }

    func testTheFirstTermWithHitsWins() throws {
        let finder = try finder()
        XCTAssertNil(finder.lastResult)
        XCTAssertEqual(finder.find(["Pin Configuration", "Pin Functions"]), 2)
        XCTAssertEqual(finder.lastResult, "Page 2 of 4 — “Pin Configuration”")
        // It has one hit, so the next press stays on it.
        XCTAssertEqual(finder.find(["Pin Configuration", "Pin Functions"]), 2)
        // A term with no hit is passed over for the next.
        XCTAssertEqual(finder.find(["Thermal", "Features"]), 1)
        XCTAssertEqual(finder.lastResult, "Page 1 of 4 — “Features”")
    }

    func testEachPressMovesToTheNextHitAndWraps() throws {
        let finder = try finder()
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        XCTAssertEqual(finder.lastResult, "Page 3 of 4 — “Pin Functions”")
        XCTAssertEqual(finder.find(["Pin Functions"]), 4)
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        // Case does not matter.
        XCTAssertEqual(finder.find(["pin functions (CONTINUED)"]), 4)
        // Other terms start over.
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
    }

    /// A contents page names every section; the section's own page comes first.
    func testATableOfContentsPageComesLast() throws {
        let finder = newFinder()
        finder.document = try Self.pdf(headings: ["Features", "Table of Contents: Pin Functions, Timing",
                                                  "Pin Functions"])
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        XCTAssertEqual(finder.find(["Pin Functions"]), 2)
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        // Found only on the contents page, it is still found.
        XCTAssertEqual(finder.find(["Timing"]), 2)
    }

    func testNotFound() throws {
        let finder = try finder()
        XCTAssertNil(finder.find(["Thermal"]))
        XCTAssertEqual(finder.lastResult, "Not found: Thermal")
        XCTAssertNil(finder.find(["Thermal Information", "Land Pattern"]))
        XCTAssertEqual(finder.lastResult, "Not found: Thermal Information, Land Pattern")
        XCTAssertNil(finder.find([]))

        let empty = newFinder()
        XCTAssertNil(empty.find(["Features"]))
        XCTAssertEqual(empty.lastResult, "Load the datasheet first")
    }

    func testANewDocumentStartsOver() throws {
        let finder = try finder()
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        finder.document = try Self.pdf(headings: ["Pin Functions", "Features"])
        XCTAssertNil(finder.lastResult)
        XCTAssertEqual(finder.find(["Pin Functions"]), 1)
        XCTAssertEqual(finder.lastResult, "Page 1 of 2 — “Pin Functions”")
    }

    func testAddressRules() {
        let ti = "Texas Instruments"
        XCTAssertEqual(DatasheetURLRule.check("https://x/y.pdf", manufacturer: ti), .ok("https://x/y.pdf"))
        XCTAssertEqual(DatasheetURLRule.check("  https://x/y.pdf \n", manufacturer: ti), .ok("https://x/y.pdf"))
        XCTAssertEqual(DatasheetURLRule.check("http://x/y.pdf", manufacturer: ti), .offerHTTPS("https://x/y.pdf"))
        XCTAssertEqual(DatasheetURLRule.check("ftp://x/y.pdf", manufacturer: ti),
                       .message("The address must start with https://"))
        XCTAssertEqual(DatasheetURLRule.check("www.ti.com/x.pdf", manufacturer: ti),
                       .message("The address must start with https://"))
        XCTAssertEqual(DatasheetURLRule.check("https://", manufacturer: ti),
                       .message("The address must start with https://"))
        XCTAssertEqual(DatasheetURLRule.check("https://x/y.pdf", manufacturer: "Generic"),
                       .message("A datasheet needs a manufacturer"))
        XCTAssertEqual(DatasheetURLRule.check("https://x/y.pdf", manufacturer: nil),
                       .message("A datasheet needs a manufacturer"))
        XCTAssertEqual(DatasheetURLRule.check("   ", manufacturer: "Generic"), .empty)
    }

    /// The loader's checks, with the datasheet window's messages.
    func testLoaderChecksTheAnswer() throws {
        let pdf = try XCTUnwrap(Self.pdf(headings: ["One"]).dataRepresentation())
        if case .failure = DatasheetLoader.document(data: pdf, status: 200) { XCTFail("a PDF with 200 loads") }
        if case .failure = DatasheetLoader.document(data: pdf, status: nil) { XCTFail("a file has no status") }
        XCTAssertEqual(failure(DatasheetLoader.document(data: pdf, status: 403)), .status(403))
        XCTAssertEqual(failure(DatasheetLoader.document(data: Data("<html>".utf8), status: 200)), .notPDF)
        XCTAssertEqual(DatasheetLoader.Failure.notPDF.message, "The manufacturer's site did not send a PDF.")
        XCTAssertTrue(DatasheetLoader.Failure.status(403).message.hasPrefix("The manufacturer's site answered 403 ("))
    }

    private func failure(_ result: Result<PDFDocument, DatasheetLoader.Failure>) -> DatasheetLoader.Failure? {
        if case let .failure(failure) = result { return failure }
        return nil
    }

    /// The generic steps name the section titles their information sits under.
    func testGenericStepsHaveSearchTerms() {
        let steps = WizardPlan.steps(for: PartDraft(newOf: "gate"))
        let terms = Dictionary(uniqueKeysWithValues: steps.map { ($0.id, $0.searchTerms) })
        XCTAssertEqual(terms["identity"], ["Ordering Information", "Package Option Addendum",
                                           "Orderable Part Number", "Device Information"])
        XCTAssertEqual(terms["pinout"]?.first, "Pin Configuration and Functions")
        XCTAssertEqual(terms["pinout"]?.count, 7)
        XCTAssertEqual(terms["kind.supply"], ["Recommended Operating Conditions", "Operating Conditions",
                                              "Absolute Maximum Ratings"])
        XCTAssertEqual(terms["footprint"]?.count, 8)
        XCTAssertEqual(terms["footprint"]?.first, "Land Pattern")
        // A derived kind step has none; a curated one names its sections (plan-46-L).
        let derived = WizardPlan.derivedSteps(schema: PartKinds.schemas["gate"]!, variant: nil)
        XCTAssertEqual(derived.first { $0.id == "kind.essentials" }?.searchTerms, [])
        XCTAssertEqual(terms["kind.gates"]?.first, "Function Tables")
        XCTAssertEqual(terms["datasheet"], [])
    }
}

extension DatasheetFinderTests {
    /// With the pane's view attached, a hit scrolls it there and selects the text.
    func testAHitIsShownInTheView() throws {
        let finder = try finder()
        let view = PDFView(frame: NSRect(x: 0, y: 0, width: 300, height: 400))
        view.document = finder.document
        finder.view = view
        XCTAssertEqual(finder.find(["Pin Functions"]), 3)
        XCTAssertEqual(view.currentSelection?.string, "Pin Functions")
        XCTAssertEqual(view.currentSelection?.pages.first, finder.document?.page(at: 2))
    }
}
