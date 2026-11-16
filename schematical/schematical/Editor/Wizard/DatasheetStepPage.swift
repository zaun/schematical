import AppKit
import PDFKit
import SwiftUI
import UniformTypeIdentifiers

/// Step 3 of the Part Wizard, "Datasheet" (plan-46-G): the address of the
/// manufacturer's PDF. The part keeps the address (an inline datasheet,
/// plan-46-A); the PDF itself is only shown, in the pane beside the wizard,
/// and is never stored.
struct DatasheetStepPage: View {
    @Bindable var model: PartEditorModel
    @State private var text = ""
    @State private var replacing = false

    var body: some View {
        let generic = (model.draft.manufacturer ?? "").isEmpty || model.draft.manufacturer == "Generic"
        if generic {
            Section {
                Label(DatasheetURLRule.needsManufacturer, systemImage: "info.circle")
                Text("A part with no maker (“Generic”, your own design) has no datasheet. "
                    + "Name the manufacturer on the Part number step, or go on without one.")
                    .font(.callout).foregroundStyle(.secondary)
                Button("Go to Part number") { model.go(to: WizardStep.identity.id) }
            }
        } else if let id = model.draft.datasheet, model.draft.datasheetURL == nil, !replacing {
            // A library datasheet (the part was copied from a library part).
            Section("The library's datasheet") {
                LabeledContent("Datasheet", value: id)
                if let url = model.bundledDatasheetURL {
                    LabeledContent("Address", value: url.absoluteString)
                        .textSelection(.enabled)
                }
                HStack {
                    Button("Load") { Task { await model.loadDatasheet() } }
                        .disabled(model.bundledDatasheetURL == nil)
                    Button("Replace…") { replacing = true }
                        .help("Give this part its own datasheet address")
                }
            }
        } else {
            Section {
                TextField("Address", text: $text, prompt: Text("https://www.ti.com/lit/ds/symlink/sn74hc00.pdf"))
                    .onSubmit { commit() }
                    .onChange(of: text) { _, _ in commit() }
                switch DatasheetURLRule.check(text, manufacturer: model.draft.manufacturer) {
                case let .offerHTTPS(address):
                    HStack {
                        Text("Datasheets are opened over https.").foregroundStyle(.secondary)
                        Button("Use \(address)") { text = address }
                    }
                case let .message(message):
                    Label(message, systemImage: "exclamationmark.circle").foregroundStyle(.orange)
                case .ok, .empty:
                    EmptyView()
                }
                HStack {
                    Button("Load") { Task { await model.loadDatasheet() } }
                        .disabled(model.draft.datasheetURL == nil)
                        .keyboardShortcut(.return, modifiers: [.command])
                    Button("Open a downloaded PDF…") { Task { await model.openLocalDatasheet() } }
                        .help("Some sites refuse the download. Save the PDF in your browser and open it here; "
                            + "the part still keeps the address.")
                }
            } header: {
                Text("The manufacturer's PDF")
            } footer: {
                Text("Paste the address of the PDF on the manufacturer's own site. "
                    + "Only the address is saved in the part.")
            }
            .onAppear { text = model.draft.datasheetURL ?? "" }
        }

        Section {
            switch model.datasheetPhase {
            case .none:
                Text("The datasheet opens in the pane on the right and stays there for the rest of the steps.")
                    .foregroundStyle(.secondary)
            case .loading:
                ProgressView("Downloading the datasheet…")
            case let .loaded(source):
                Label("Showing \(source)", systemImage: "doc.richtext")
            case let .failed(message):
                Label(message, systemImage: "exclamationmark.triangle").foregroundStyle(.red)
                Text("If the site refuses the download, save the PDF in your browser and use "
                    + "Open a downloaded PDF….")
                    .font(.callout).foregroundStyle(.secondary)
            }
        }
    }

    /// Keeps the part's datasheet in step with the field: an address the
    /// rules accept is saved; an empty field removes it.
    private func commit() {
        switch DatasheetURLRule.check(text, manufacturer: model.draft.manufacturer) {
        case let .ok(address):
            if model.draft.datasheetURL != address { model.draft.setDatasheetURL(address) }
        case .empty:
            if model.draft.datasheetURL != nil || replacing { model.draft.setDatasheetURL(nil) }
        case .offerHTTPS, .message:
            break
        }
    }
}

/// The datasheet in the wizard's side pane: the PDF once it has loaded.
struct DatasheetPane: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            if let document = model.datasheet.document {
                PDFDocumentView(document: document, finder: model.datasheet)
                if let result = model.datasheet.lastResult {
                    Text(result).font(.caption).foregroundStyle(.secondary).lineLimit(2)
                }
            } else {
                ContentUnavailableView {
                    Label("No datasheet", systemImage: "doc.text.magnifyingglass")
                } description: {
                    switch model.datasheetPhase {
                    case .loading: Text("Downloading…")
                    case let .failed(message): Text(message)
                    default: Text("Give the address on the Datasheet step and press Load.")
                    }
                } actions: {
                    Button("Go to Datasheet") { model.go(to: WizardStep.datasheet.id) }
                }
            }
        }
    }
}

extension PartEditorModel {
    /// Shows a PDF the user downloaded themselves. The part keeps its
    /// address; the file is only viewed.
    func openLocalDatasheet() async {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.pdf]
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.message = "Choose the datasheet PDF you downloaded"
        let response: NSApplication.ModalResponse
        if let window { response = await panel.beginSheetModal(for: window) } else { response = panel.runModal() }
        guard response == .OK, let url = panel.url else { return }
        showLocalDatasheet(url)
    }
}
