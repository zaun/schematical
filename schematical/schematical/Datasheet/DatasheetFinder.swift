import Foundation
import Observation
import PDFKit

/// Finds section titles in a loaded datasheet (plan-46-G): the Part Wizard's
/// "Find in datasheet" buttons. One per editor window, shared by the pane
/// that shows the PDF and every step's button.
@MainActor
@Observable
final class DatasheetFinder {
    /// The loaded PDF; nil until one loads.
    var document: PDFDocument? {
        didSet {
            if document !== oldValue { reset() }
        }
    }
    /// The pane's PDF view, to scroll to a hit. Weak: the view owns itself.
    @ObservationIgnored weak var view: PDFView?
    /// What the last search found, for the UI: "Page 3 of 18 — “Pin
    /// Configuration and Functions”" or "Not found: Pin Configuration, …".
    private(set) var lastResult: String?

    @ObservationIgnored private var lastTerms: [String] = []
    @ObservationIgnored private var hits: [PDFSelection] = []
    @ObservationIgnored private var term = ""
    @ObservationIgnored private var index = 0

    /// The hits with those on a table-of-contents page moved to the end: a
    /// contents page lists every section title, and the section itself is
    /// what the user is looking for.
    static func contentsLast(_ hits: [PDFSelection]) -> [PDFSelection] {
        var contents: [PDFPage: Bool] = [:]
        func isContents(_ selection: PDFSelection) -> Bool {
            guard let page = selection.pages.first else { return false }
            if let known = contents[page] { return known }
            let found = page.string?.range(of: "table of contents", options: .caseInsensitive) != nil
            contents[page] = found
            return found
        }
        return hits.filter { !isContents($0) } + hits.filter(isContents)
    }

    private func reset() {
        lastTerms = []
        hits = []
        term = ""
        index = 0
        lastResult = nil
    }

    /// Looks for the terms in order, case-insensitively, and keeps the hits
    /// of the first term that has any. A call with the same terms moves to
    /// the next hit, wrapping to the first. Scrolls the view there and
    /// returns the hit's page (1-based); nil when no term is found or no
    /// document is loaded.
    @discardableResult
    func find(_ terms: [String]) -> Int? {
        guard let document else {
            lastResult = "Load the datasheet first"
            return nil
        }
        if terms == lastTerms, !hits.isEmpty {
            index = (index + 1) % hits.count
        } else {
            lastTerms = terms
            hits = []
            term = ""
            index = 0
            for candidate in terms where !candidate.isEmpty {
                let found = document.findString(candidate, withOptions: [.caseInsensitive])
                if !found.isEmpty {
                    hits = Self.contentsLast(found)
                    term = candidate
                    break
                }
            }
        }
        guard hits.indices.contains(index), let page = hits[index].pages.first else {
            lastResult = "Not found: " + terms.joined(separator: ", ")
            return nil
        }
        let number = document.index(for: page) + 1
        lastResult = "Page \(number) of \(document.pageCount) — “\(term)”"
        if let view {
            view.go(to: hits[index])
            view.setCurrentSelection(hits[index], animate: true)
        }
        return number
    }
}
