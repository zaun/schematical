//
//  VerifyDatasheetWindow.swift
//  schematical
//
//  Window › Verify Datasheet (shown only while Option is held): the app's
//  own datasheet beside the manufacturer's PDF, so a person can check the
//  conversion against its source. A part is picked from the search field
//  at the top (only parts whose datasheet is converted). Here, and only
//  here, the datasheet shows its citations (planning/spike-datasheet-format/
//  format.md § Citations); clicking one scrolls the PDF to that spot.
//
//  Edit turns the datasheet pane into an inline editor (Renderer/datasheet-edit.js):
//  click a block, change it within the format, Apply. Edits stay in memory
//  (`edits`, by datasheet path) and are written nowhere yet.
//

import AppKit
import PDFKit
import SwiftUI

enum VerifyDatasheetScene {
    static let id = "verify-datasheet"
    static let title = "Verify Datasheet"
}

// MARK: - Menu

/// Keeps the Verify Datasheet item (the Window scene's own, in the Window
/// menu) hidden, not disabled, unless Option is held. SwiftUI rebuilds the
/// menu bar now and then, so the state is applied whenever a menu is about to
/// open and, while one is open, a timer in the common run-loop modes follows
/// the Option key as it goes down and up.
@MainActor
enum OptionOnlyMenuItems {
    static let titles: Set<String> = [VerifyDatasheetScene.title]

    private static var observers: [Any] = []
    private static var timer: Timer?

    static func install() {
        guard observers.isEmpty else { return }
        observers.append(NotificationCenter.default.addObserver(
            forName: NSMenu.didBeginTrackingNotification, object: nil, queue: .main
        ) { _ in
            MainActor.assumeIsolated { beginFollowing() }
        })
        observers.append(NotificationCenter.default.addObserver(
            forName: NSMenu.didEndTrackingNotification, object: nil, queue: .main
        ) { _ in
            MainActor.assumeIsolated { stopFollowing() }
        })
        // An item added to the Window menu (SwiftUI's rebuild) starts hidden.
        observers.append(NotificationCenter.default.addObserver(
            forName: NSMenu.didAddItemNotification, object: nil, queue: .main
        ) { note in
            let menu = note.object as? NSMenu
            MainActor.assumeIsolated {
                guard let menu, menu === NSApp.windowsMenu || menu.title == "File" else { return }
                DispatchQueue.main.async { apply(optionDown: false) }
            }
        })
    }

    private static func beginFollowing() {
        apply(optionDown: NSEvent.modifierFlags.contains(.option))
        guard timer == nil else { return }
        let timer = Timer(timeInterval: 0.05, repeats: true) { _ in
            MainActor.assumeIsolated {
                apply(optionDown: NSEvent.modifierFlags.contains(.option))
            }
        }
        RunLoop.main.add(timer, forMode: .common)
        self.timer = timer
    }

    private static func stopFollowing() {
        timer?.invalidate()
        timer = nil
        apply(optionDown: false)
    }

    /// Shows the items when Option is down, hides them otherwise. In the
    /// File menu Save As… takes Duplicate's place while Option is down, as
    /// in any document app (plan-61-B).
    static func apply(optionDown: Bool) {
        if let menu = NSApp.windowsMenu {
            for item in menu.items where titles.contains(item.title) {
                if item.isHidden == optionDown { item.isHidden = !optionDown }
            }
        }
        if let menu = NSApp.mainMenu?.items.first(where: { $0.title == "File" })?.submenu {
            for item in menu.items {
                let isSaveAs = item.title == DocumentMenuCommands.saveAsTitle
                guard isSaveAs || item.title == DocumentMenuCommands.duplicateTitle else { continue }
                // Each keeps its key (⇧⌘S, ⌥⇧⌘S) while the other is shown.
                if !item.allowsKeyEquivalentWhenHidden { item.allowsKeyEquivalentWhenHidden = true }
                let hidden = isSaveAs ? !optionDown : optionDown
                if item.isHidden != hidden { item.isHidden = hidden }
            }
        }
    }
}

// MARK: - Where a citation points

/// Finds, on a PDF page, the spot a citation names. Paragraph numbers are
/// found by the text of the paragraph that cites them; a table or figure
/// by its printed label. When the spot cannot be found the caller shows
/// the page.
enum CitationLocator {
    /// Letters and digits only, lower case, each with its offset in the
    /// original string: line breaks, hyphenation, spacing and dash kinds
    /// do not get in the way of a match.
    struct Folded {
        var units: [UInt16] = []
        var offsets: [Int] = []

        init(_ text: String) {
            let ns = text as NSString
            for index in 0..<ns.length {
                let unit = ns.character(at: index)
                guard let scalar = Unicode.Scalar(unit), CharacterSet.alphanumerics.contains(scalar) else { continue }
                let lower = Array(String(scalar).lowercased().utf16)
                units.append(lower.count == 1 ? lower[0] : unit)
                offsets.append(index)
            }
        }

        /// Where `needle` first occurs at or after `from` (index into `units`).
        func find(_ needle: [UInt16], from: Int = 0) -> Int? {
            guard !needle.isEmpty, units.count >= needle.count, from <= units.count - needle.count else { return nil }
            var start = from
            while start <= units.count - needle.count {
                if units[start] == needle[0], Array(units[start..<(start + needle.count)]) == needle {
                    return start
                }
                start += 1
            }
            return nil
        }
    }

    /// The part of the page the citation names, nil when it is not found.
    static func locate(_ click: CitationClick, on page: PDFPage) -> PDFSelection? {
        guard let text = page.string, !text.isEmpty else { return nil }
        let folded = Folded(text)
        if let range = labelRange(click.reference, in: folded, text: text) {
            return page.selection(for: range)
        }
        if let range = textRange(click.text, in: folded) {
            return page.selection(for: range)
        }
        return nil
    }

    /// A printed label ("Table 6-3", "Figure 8-1"): its first occurrence at
    /// the start of a line (a caption), else its first occurrence.
    static func labelRange(_ reference: String, in folded: Folded, text: String) -> NSRange? {
        guard !reference.contains("#"), reference.rangeOfCharacter(from: .letters) != nil else { return nil }
        let needle = Folded(reference).units
        guard !needle.isEmpty else { return nil }
        let ns = text as NSString
        var first: Int?
        var from = 0
        while let hit = folded.find(needle, from: from) {
            first = first ?? hit
            let offset = folded.offsets[hit]
            if offset == 0 || CharacterSet.newlines.contains(Unicode.Scalar(ns.character(at: offset - 1)) ?? " ") {
                return range(folded, hit, needle.count)
            }
            from = hit + 1
        }
        return first.map { range(folded, $0, needle.count) }
    }

    /// The first few words of the paragraph that are on the page, trying
    /// windows of five words along the paragraph.
    static func textRange(_ paragraph: String, in folded: Folded) -> NSRange? {
        let words = paragraph.split(whereSeparator: \.isWhitespace).map(String.init)
        guard words.count >= 3 else {
            let needle = Folded(paragraph).units
            return needle.count >= 6 ? folded.find(needle).map { range(folded, $0, needle.count) } : nil
        }
        let window = 5
        var start = 0
        var tries = 0
        while start < words.count, tries < 40 {
            let piece = words[start..<min(words.count, start + window)].joined(separator: " ")
            let needle = Folded(piece).units
            if needle.count >= 8, let hit = folded.find(needle) {
                return range(folded, hit, needle.count)
            }
            start += 3
            tries += 1
        }
        return nil
    }

    private static func range(_ folded: Folded, _ start: Int, _ count: Int) -> NSRange {
        let from = folded.offsets[start]
        let to = folded.offsets[start + count - 1] + 1
        return NSRange(location: from, length: to - from)
    }
}

// MARK: - Model

@MainActor
@Observable
final class VerifyDatasheetModel {
    /// As `DatasheetFinder`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}

    enum PDFPhase {
        case none
        case loading
        case loaded(PDFDocument)
        case failed(String)
    }

    /// The parts that have a converted datasheet, by name.
    let candidates: [CatalogPart]
    let page = DatasheetPage(showsCitations: true)
    let finder = DatasheetFinder()

    private(set) var selected: CatalogPart?
    private(set) var pdf = PDFPhase.none
    /// What the last citation click did, for the line under the search field.
    private(set) var status = ""
    /// The datasheet pane is an editor.
    private(set) var isEditing = false
    /// The edited text of every datasheet changed in this window, by its path. Not saved anywhere.
    private(set) var edits: [String: String] = [:]
    /// The blocks of the shown datasheet that differ from its file, and the edits that can be undone.
    private(set) var editedBlocks = 0
    private(set) var undoDepth = 0

    @ObservationIgnored private var documents: [URL: PDFDocument] = [:]
    @ObservationIgnored private var loading: Task<Void, Never>?

    init(catalog: PartCatalogStore = .shared) {
        candidates = catalog.allParts
            .filter { $0.datasheetMarkdown != nil }
            .sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
        page.onCitation = { [weak self] click in self?.show(click) }
        page.onEdit = { [weak self] state in self?.record(state) }
        page.onPDFPageRequest = { [weak self] in
            if let number = self?.currentPDFPage() { self?.page.deliverPDFPage(number) }
        }
    }

    func setEditing(_ on: Bool) {
        isEditing = on
        page.setEditing(on)
    }

    func undo() { page.undo() }
    func revertAll() { page.revertAll() }

    /// What the page's editor reported. A report for a datasheet that is no longer shown is dropped.
    private func record(_ state: DatasheetEditState) {
        guard state.path == selected?.datasheetMarkdown else { return }
        edits[state.path] = state.modified ? state.markdown : nil
        editedBlocks = state.edited
        undoDepth = state.undo
    }

    /// The page the PDF shows (from 1), for a citation's "Use PDF page".
    func currentPDFPage() -> Int? {
        guard let view = finder.view, let document = view.document, let current = view.currentPage else { return nil }
        let index = document.index(for: current)
        return index == NSNotFound ? nil : index + 1
    }

    /// The candidates matching what was typed: every word in the name,
    /// maker or description; names that start with the text first.
    func matches(_ query: String, limit: Int = 80) -> [CatalogPart] {
        let words = query.lowercased().split(whereSeparator: \.isWhitespace).map(String.init)
        guard !words.isEmpty else { return Array(candidates.prefix(limit)) }
        let found = candidates.filter { part in
            let haystack = (part.name + " " + part.manufacturer + " " + part.description).lowercased()
            return words.allSatisfy { haystack.contains($0) }
        }
        let first = words[0]
        let starting = found.filter { $0.name.lowercased().hasPrefix(first) }
        let rest = found.filter { !$0.name.lowercased().hasPrefix(first) }
        return Array((starting + rest).prefix(limit))
    }

    func select(_ part: CatalogPart) {
        selected = part
        status = ""
        editedBlocks = 0
        undoDepth = 0
        if let path = part.datasheetMarkdown {
            page.load(MarkdownDatasheetTarget(path: path, partName: part.name),
                      markdown: edits[path], editing: isEditing)
        }
        loadPDF(for: part)
    }

    func loadPDF(for part: CatalogPart) {
        loading?.cancel()
        guard let url = part.datasheetURL else {
            pdf = .failed("\(part.name) has no datasheet address in the catalog.")
            return
        }
        if let cached = documents[url] {
            pdf = .loaded(cached)
            return
        }
        pdf = .loading
        loading = Task {
            let result = await DatasheetLoader.load(url)
            guard !Task.isCancelled, selected?.datasheetURL == url else { return }
            switch result {
            case let .success(document):
                documents[url] = document
                pdf = .loaded(document)
            case let .failure(failure):
                pdf = .failed(failure.message)
            }
        }
    }

    /// Scrolls the PDF to what a citation names.
    func show(_ click: CitationClick) {
        guard case let .loaded(document) = pdf, let view = finder.view else {
            status = "The PDF is not loaded yet."
            return
        }
        guard let pdfPage = document.page(at: click.page - 1) else {
            status = "The PDF has no page \(click.page) (it has \(document.pageCount))."
            return
        }
        if let selection = CitationLocator.locate(click, on: pdfPage) {
            let bounds = selection.bounds(for: pdfPage)
            view.go(to: bounds.insetBy(dx: 0, dy: -140), on: pdfPage)
            view.setCurrentSelection(selection, animate: true)
            status = "Page \(click.page), \(click.reference)"
        } else {
            view.clearSelection()
            view.go(to: pdfPage)
            status = "Page \(click.page), \(click.reference): spot not found, showing the page"
        }
    }
}

// MARK: - Window

struct VerifyDatasheetWindowView: View {
    @State private var model = VerifyDatasheetModel()
    @State private var query = ""
    @State private var showResults = false
    @FocusState private var searchFocused: Bool

    var body: some View {
        VStack(spacing: 0) {
            header
            Divider()
            content
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
        .overlay(alignment: .topLeading) {
            if showResults { results }
        }
        .frame(minWidth: 900, minHeight: 500)
        .navigationTitle(VerifyDatasheetScene.title)
        .navigationSubtitle(model.selected.map { "\($0.name) — \(model.page.title)" } ?? "")
        .onAppear { searchFocused = true }
    }

    private var header: some View {
        HStack(spacing: 12) {
            HStack {
                Image(systemName: "magnifyingglass").foregroundStyle(.secondary)
                TextField("Search a part with a converted datasheet", text: $query)
                    .textFieldStyle(.plain)
                    .focused($searchFocused)
                    .onChange(of: query) { _, text in
                        showResults = text != model.selected?.name
                    }
                    .onSubmit {
                        if let first = model.matches(query).first { choose(first) }
                    }
                    .onExitCommand { showResults = false }
                if !query.isEmpty {
                    Button {
                        query = ""
                        showResults = true
                    } label: {
                        Image(systemName: "xmark.circle.fill").foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                    .help("Clear")
                }
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 5)
            .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
            .frame(maxWidth: 420)

            Text(model.status)
                .font(.callout)
                .foregroundStyle(.secondary)
                .lineLimit(1)
            Spacer(minLength: 0)
            if model.selected != nil { editControls }
            Text("\(model.candidates.count) parts")
                .font(.caption)
                .foregroundStyle(.tertiary)
        }
        .padding(10)
    }

    private var editControls: some View {
        HStack(spacing: 8) {
            if model.isEditing, model.editedBlocks > 0 {
                Text(model.editedBlocks == 1 ? "1 block edited" : "\(model.editedBlocks) blocks edited")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            if model.isEditing {
                Button("Undo") { model.undo() }
                    .disabled(model.undoDepth == 0)
                    .help("Undo the last applied edit")
                Button("Revert All") { model.revertAll() }
                    .disabled(model.editedBlocks == 0)
                    .help("Go back to the datasheet as it is in the app (Undo brings the edits back)")
            }
            Toggle(isOn: Binding(get: { model.isEditing }, set: { model.setEditing($0) })) {
                Label("Edit", systemImage: "pencil")
            }
            .toggleStyle(.button)
            .help("Edit the datasheet in place. Edits are not saved yet.")
        }
    }

    private var results: some View {
        let matches = model.matches(query)
        return Group {
            if matches.isEmpty {
                Text("No part with a converted datasheet matches.")
                    .foregroundStyle(.secondary)
                    .padding(12)
            } else {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 0) {
                        ForEach(matches) { part in
                            Button { choose(part) } label: {
                                VStack(alignment: .leading, spacing: 1) {
                                    Text(part.name).fontWeight(.medium)
                                    Text([part.manufacturer, part.description].filter { !$0.isEmpty }
                                        .joined(separator: " · "))
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                        .lineLimit(1)
                                }
                                .frame(maxWidth: .infinity, alignment: .leading)
                                .padding(.horizontal, 10)
                                .padding(.vertical, 4)
                                .contentShape(Rectangle())
                            }
                            .buttonStyle(.plain)
                        }
                    }
                }
            }
        }
        .frame(width: 480)
        .frame(maxHeight: 360)
        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 8))
        .overlay(RoundedRectangle(cornerRadius: 8).stroke(.separator))
        .shadow(radius: 8, y: 3)
        .padding(.leading, 10)
        .padding(.top, 46)
    }

    private func choose(_ part: CatalogPart) {
        model.select(part)
        query = part.name
        showResults = false
    }

    @ViewBuilder
    private var content: some View {
        if model.selected == nil {
            ContentUnavailableView(
                "Pick a Part",
                systemImage: "checkmark.seal",
                description: Text("Search above. The converted datasheet appears beside the manufacturer's PDF; "
                    + "click a citation to jump to its spot in the PDF.")
            )
        } else {
            HSplitView {
                Group {
                    if let message = model.page.failure {
                        ContentUnavailableView {
                            Label("Can't Show Datasheet", systemImage: "doc.questionmark")
                        } description: {
                            Text(message)
                        }
                    } else {
                        DatasheetWebView(page: model.page)
                    }
                }
                .frame(minWidth: 380)
                pdfPane
                    .frame(minWidth: 380)
            }
        }
    }

    @ViewBuilder
    private var pdfPane: some View {
        switch model.pdf {
        case .none:
            Color.clear
        case .loading:
            ProgressView("Downloading datasheet…")
                .frame(maxWidth: .infinity, maxHeight: .infinity)
        case let .loaded(document):
            PDFDocumentView(document: document, finder: model.finder)
        case let .failed(message):
            ContentUnavailableView {
                Label("Can't Open the PDF", systemImage: "doc.questionmark")
            } description: {
                Text(message)
            } actions: {
                if let part = model.selected {
                    Button("Try Again") { model.loadPDF(for: part) }
                    if let url = part.datasheetURL { Link("Open in Browser", destination: url) }
                }
            }
        }
    }
}
