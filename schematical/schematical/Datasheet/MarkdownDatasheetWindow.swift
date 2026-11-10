//
//  MarkdownDatasheetWindow.swift
//  schematical
//
//  A part's datasheet in the app's own Markdown format (the bundled
//  Datasheets folder), rendered in its own window. Opened from the Show
//  Datasheet button under a part's Details, for parts whose datasheet has
//  been converted ("md" in datasheets.json).
//
//  The page is Datasheet/Renderer/datasheet.html: datasheet.js renders the
//  Markdown with markdown-it and draws the timing diagrams with WaveDrom,
//  both bundled, so nothing is fetched from the network. The app hands it
//  what the format generates from the catalog: the devices' pinouts, the
//  claimed parts, their footprints and the shared figures.
//

import AppKit
import SwiftUI
import WebKit

/// Which Markdown datasheet to show: its path under Datasheets and the part
/// it was opened for. One window per file.
nonisolated struct MarkdownDatasheetTarget: Codable, Hashable, Sendable {
    let path: String
    let partName: String

    static func == (lhs: Self, rhs: Self) -> Bool { lhs.path == rhs.path }
    func hash(into hasher: inout Hasher) { hasher.combine(path) }
}

/// The Show Datasheet button: opens (or brings forward) the datasheet window.
struct ShowDatasheetButton: View {
    let target: MarkdownDatasheetTarget

    @Environment(\.openWindow) private var openWindow

    var body: some View {
        Button("Show Datasheet") {
            openWindow(id: "datasheet-md", value: target)
        }
        .frame(maxWidth: .infinity)
    }
}

/// The datasheet window.
struct MarkdownDatasheetWindowView: View {
    let target: MarkdownDatasheetTarget

    @State private var page = DatasheetPage()

    var body: some View {
        Group {
            if let message = page.failure {
                ContentUnavailableView {
                    Label("Can't Show Datasheet", systemImage: "doc.questionmark")
                } description: {
                    Text(message)
                }
            } else {
                DatasheetWebView(page: page)
            }
        }
        .frame(minWidth: 500, minHeight: 400)
        .navigationTitle("\(target.partName) Datasheet")
        .navigationSubtitle(page.title)
        .toolbar {
            ToolbarItem {
                Button {
                    page.print()
                } label: {
                    Label("Print", systemImage: "printer")
                }
                .help("Print the datasheet")
                .disabled(page.failure != nil)
            }
        }
        .task { page.load(target) }
    }
}

/// The web view, its payload and its state.
@MainActor
@Observable
final class DatasheetPage: NSObject, WKNavigationDelegate {
    let webView: WKWebView
    private(set) var title = ""
    private(set) var failure: String?
    @ObservationIgnored private var payload: String?

    override init() {
        webView = WKWebView(frame: .zero, configuration: WKWebViewConfiguration())
        super.init()
        webView.navigationDelegate = self
        webView.setValue(false, forKey: "drawsBackground")
    }

    func load(_ target: MarkdownDatasheetTarget) {
        do {
            let built = try DatasheetPayload.build(path: target.path, partName: target.partName,
                                                   catalog: PartCatalogStore.shared)
            payload = built.json
            title = built.title
            guard let html = Bundle.main.url(forResource: "datasheet", withExtension: "html"),
                  let resources = Bundle.main.resourceURL else {
                failure = "The datasheet viewer is missing from the app."
                return
            }
            webView.loadFileURL(html, allowingReadAccessTo: resources)
        } catch {
            failure = error.localizedDescription
        }
    }

    func print() {
        guard let window = webView.window else { return }
        let info = NSPrintInfo.shared.copy() as? NSPrintInfo ?? NSPrintInfo.shared
        info.horizontalPagination = .fit
        info.verticalPagination = .automatic
        info.isVerticallyCentered = false
        let operation = webView.printOperation(with: info)
        operation.view?.frame = webView.bounds
        operation.runModal(for: window, delegate: nil, didRun: nil, contextInfo: nil)
    }

    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        guard let payload else { return }
        webView.evaluateJavaScript("renderDatasheet(\(payload))") { [weak self] _, error in
            if let error {
                self?.failure = error.localizedDescription
            }
        }
    }

    /// Links to anchors stay in the page; anything else opens in the browser.
    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction,
                 decisionHandler: @escaping @MainActor @Sendable (WKNavigationActionPolicy) -> Void) {
        guard navigationAction.navigationType == .linkActivated,
              let url = navigationAction.request.url, !url.isFileURL else {
            decisionHandler(.allow)
            return
        }
        NSWorkspace.shared.open(url)
        decisionHandler(.cancel)
    }
}

/// WKWebView in SwiftUI.
private struct DatasheetWebView: NSViewRepresentable {
    let page: DatasheetPage

    func makeNSView(context: Context) -> WKWebView { page.webView }
    func updateNSView(_ view: WKWebView, context: Context) {}
}

// MARK: - Payload

/// What datasheet.js needs besides the Markdown: the catalog entries the
/// front matter names, as JSON.
enum DatasheetPayload {
    struct Failure: LocalizedError {
        let errorDescription: String?
    }

    /// The part files' raw JSON, by kind ("gate" → its file's object), read
    /// once: the renderer needs the devices' pinouts as written, including
    /// pinouts no catalog part uses.
    @MainActor private static var partFiles: [String: [String: Any]] = {
        var files: [String: [String: Any]] = [:]
        let skip: Set<String> = [BackendCatalog.datasheetsFile, BackendCatalog.footprintsFile]
        for url in Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil) ?? []
            where !skip.contains(url.lastPathComponent) {
            guard let data = try? Data(contentsOf: url),
                  let object = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
                  let kind = object["kind"] as? String else { continue }
            files[kind] = object
        }
        return files
    }()

    @MainActor
    static func build(path: String, partName: String,
                      catalog: PartCatalogStore) throws -> (json: String, title: String) {
        let name = (path as NSString).lastPathComponent
        guard let url = Bundle.main.url(forResource: (name as NSString).deletingPathExtension,
                                        withExtension: "md"),
              let markdown = try? String(contentsOf: url, encoding: .utf8) else {
            throw Failure(errorDescription: "The datasheet file \(path) is missing from the app.")
        }
        let front = frontMatter(markdown)

        var devices: [[String: Any]] = []
        var footprintIDs: [String] = []
        var partsByFootprint: [String: String] = [:]
        for entry in front["devices"] as? [String] ?? [] {
            let pieces = entry.split(separator: ":", maxSplits: 1).map(String.init)
            guard pieces.count == 2,
                  let list = partFiles[pieces[0]]?["devices"] as? [[String: Any]],
                  var device = list.first(where: { $0["id"] as? String == pieces[1] }) else { continue }
            device["kind"] = pieces[0]
            for part in device["parts"] as? [[String: Any]] ?? [] {
                if let footprint = part["footprint"] as? String, let name = part["name"] as? String,
                   partsByFootprint[footprint] == nil {
                    partsByFootprint[footprint] = name
                    footprintIDs.append(footprint)
                }
            }
            device.removeValue(forKey: "note")
            device.removeValue(forKey: "timing")
            devices.append(device)
        }

        // Footprints with their pads expanded by the backend.
        var footprints: [String: Any] = [:]
        for id in footprintIDs {
            guard let partName = partsByFootprint[id], let part = catalog.part(named: partName) else { continue }
            let candidates = [part.footprint].compactMap { $0 } + part.footprintChoices.map(\.footprint)
            guard let footprint = candidates.first(where: { $0.id == id }) else { continue }
            footprints[id] = [
                "description": footprint.description,
                "body": ["w": footprint.body.width, "h": footprint.body.height],
                "round": footprint.bodyRound,
                "pads": footprint.pads.map { pad -> [String: Any] in
                    [
                        "d": pad.designator as Any? ?? NSNull(),
                        "x": pad.centre.x, "y": pad.centre.y,
                        "w": pad.size.width, "h": pad.size.height,
                        "drill": pad.drill,
                        "slot": pad.slotLength,
                        "plated": pad.plated,
                        "shape": pad.shape == .round ? "round" : "rect",
                    ]
                },
            ]
        }

        // Shared figures (ref=) and sidecars (src=).
        var figures: [String: String] = [:]
        let pattern = /(?:ref|src)=("[^"]*"|\S+)/
        for match in markdown.matches(of: pattern) {
            let key = String(match.1).trimmingCharacters(in: CharacterSet(charactersIn: "\""))
            let base = ((key as NSString).lastPathComponent as NSString).deletingPathExtension
            if figures[key] == nil,
               let figureURL = Bundle.main.url(forResource: base, withExtension: "svg"),
               let svg = try? String(contentsOf: figureURL, encoding: .utf8) {
                figures[key] = svg
            }
        }

        let payload: [String: Any] = [
            "markdown": markdown,
            "part": partName,
            "devices": devices,
            "footprints": footprints,
            "figures": figures,
        ]
        let data = try JSONSerialization.data(withJSONObject: payload)
        return (String(decoding: data, as: UTF8.self), front["title"] as? String ?? "")
    }

    /// The JSON between the file's two "---" lines.
    private static func frontMatter(_ markdown: String) -> [String: Any] {
        let lines = markdown.components(separatedBy: "\n")
        guard lines.first == "---", let end = lines.dropFirst().firstIndex(of: "---") else { return [:] }
        let json = lines[1..<end].joined(separator: "\n")
        return (try? JSONSerialization.jsonObject(with: Data(json.utf8))) as? [String: Any] ?? [:]
    }
}
