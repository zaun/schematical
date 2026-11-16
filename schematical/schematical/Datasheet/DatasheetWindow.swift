//
//  DatasheetWindow.swift
//  schematical
//
//  A part's datasheet: the manufacturer's PDF, downloaded and shown in its
//  own window. Opened from the Open Datasheet button under a part's Details.
//

import PDFKit
import SwiftUI

/// Which datasheet to open: the PDF's URL and the part it was opened for
/// (the window title). One window per URL.
nonisolated struct DatasheetWindowTarget: Codable, Hashable, Sendable {
    let url: URL
    let partName: String

    static func == (lhs: Self, rhs: Self) -> Bool { lhs.url == rhs.url }
    func hash(into hasher: inout Hasher) { hasher.combine(url) }
}

/// The Open Datasheet button: opens (or brings forward) the datasheet window.
struct OpenDatasheetButton: View {
    let target: DatasheetWindowTarget

    @Environment(\.openWindow) private var openWindow

    var body: some View {
        Button("Open Datasheet") {
            openWindow(id: "datasheet", value: target)
        }
        .frame(maxWidth: .infinity)
    }
}

/// The datasheet window: a spinner while the PDF downloads, then the PDF,
/// or what went wrong with a Try Again button and a link to open it in the
/// browser.
struct DatasheetWindowView: View {
    let target: DatasheetWindowTarget

    private enum Phase {
        case loading
        case loaded(PDFDocument)
        case failed(String)
    }

    @State private var phase = Phase.loading

    var body: some View {
        Group {
            switch phase {
            case .loading:
                ProgressView("Downloading datasheet…")
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            case .loaded(let document):
                PDFDocumentView(document: document)
            case .failed(let message):
                ContentUnavailableView {
                    Label("Can't Open Datasheet", systemImage: "doc.questionmark")
                } description: {
                    Text(message)
                } actions: {
                    Button("Try Again") { Task { await load() } }
                    Link("Open in Browser", destination: target.url)
                }
            }
        }
        .frame(minWidth: 400, minHeight: 400)
        .navigationTitle("\(target.partName) Datasheet")
        .navigationSubtitle(target.url.host() ?? "")
        .task { await load() }
    }

    private func load() async {
        phase = .loading
        switch await DatasheetLoader.load(target.url) {
        case let .success(document): phase = .loaded(document)
        case let .failure(failure): phase = .failed(failure.message)
        }
    }
}

/// PDFKit's view: continuous pages, scaled to the window's width. With a
/// `finder` (the Part Wizard's pane), the finder scrolls it to what it finds.
struct PDFDocumentView: NSViewRepresentable {
    let document: PDFDocument
    var finder: DatasheetFinder? = nil

    func makeNSView(context: Context) -> PDFView {
        let view = PDFView()
        view.autoScales = true
        view.displayMode = .singlePageContinuous
        view.displaysPageBreaks = true
        view.document = document
        finder?.view = view
        return view
    }

    func updateNSView(_ view: PDFView, context: Context) {
        if view.document !== document {
            view.document = document
        }
        finder?.view = view
    }
}
