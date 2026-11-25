import Foundation
import XCTest
@testable import schematical

/// Verify Datasheet: finding the spot a citation names on a PDF page's text.
@MainActor
final class CitationLocatorTests: XCTestCase {
    private let page = """
    8.1 Overview
    This device contains four inde-
    pendent 2-input NAND gates. Each gate performs the function.
    Table 6-3
    Recommended operating conditions (see Table 6-3 for limits)
    """

    private func text(_ range: NSRange) -> String {
        (page as NSString).substring(with: range)
    }

    /// Line breaks, hyphenation and spacing do not stop a paragraph being found.
    func testParagraphTextIsFoundAcrossLineBreaks() throws {
        let folded = CitationLocator.Folded(page)
        let range = try XCTUnwrap(CitationLocator.textRange(
            "This device contains four independent 2-input NAND gates.", in: folded))
        XCTAssertTrue(text(range).hasPrefix("This device contains"))
        XCTAssertEqual(range.location, (page as NSString).range(of: "This device").location)
    }

    /// A later window of the paragraph is used when its start is not on the page.
    func testLaterWordsOfTheParagraphAreTried() throws {
        let folded = CitationLocator.Folded(page)
        let range = try XCTUnwrap(CitationLocator.textRange(
            "Unrelated words that are absent here. Each gate performs the function.", in: folded))
        XCTAssertTrue(text(range).contains("Each gate performs"))
    }

    func testAbsentTextIsNotFound() {
        let folded = CitationLocator.Folded(page)
        XCTAssertNil(CitationLocator.textRange("Nothing like this appears anywhere on the page.", in: folded))
    }

    /// A label prefers the start of a line (the caption) over a mention in running text.
    func testLabelPrefersTheCaption() throws {
        let folded = CitationLocator.Folded(page)
        let range = try XCTUnwrap(CitationLocator.labelRange("Table 6\u{2013}3", in: folded, text: page))
        XCTAssertEqual(range.location, (page as NSString).range(of: "Table 6-3\n").location)
        XCTAssertEqual(text(range), "Table 6-3")
    }

    /// Paragraph numbers and unlabelled ordinals are not labels.
    func testNumbersAndOrdinalsAreNotLabels() {
        let folded = CitationLocator.Folded(page)
        XCTAssertNil(CitationLocator.labelRange("2", in: folded, text: page))
        XCTAssertNil(CitationLocator.labelRange("table #2", in: folded, text: page))
    }
}
