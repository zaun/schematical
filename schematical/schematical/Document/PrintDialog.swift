import PDFKit
import SwiftUI

/// What the sheet's button does with the pages.
enum PrintAction {
    case print, savePDF

    var button: String { self == .print ? "Print…" : "Save PDF…" }
    var title: String { self == .print ? "Print" : "Export PDF" }
}

/// File > Print…'s and File > Export > PDF…'s options sheet: which pages, the
/// paper, and the etching choices, over a preview of the PDF it makes.
struct PrintDialog: View {
    let document: SchematicDocument
    let name: String
    let action: PrintAction
    @State var options: PrintOptions
    let onCancel: () -> Void
    let onDone: (PrintOptions) -> Void

    @State private var artwork: BoardArtwork?
    @State private var preview: Data?
    private let kinds: [PrintPageKind]

    init(document: SchematicDocument, name: String, action: PrintAction, options: PrintOptions,
         onCancel: @escaping () -> Void, onDone: @escaping (PrintOptions) -> Void) {
        self.document = document
        self.name = name
        self.action = action
        self._options = State(initialValue: options)
        self.onCancel = onCancel
        self.onDone = onDone
        self.kinds = PrintExport.kinds(of: document)
    }

    private var pages: [PrintPage] { PrintExport.pages(for: document, options) }
    private var hasCopper: Bool { kinds.contains { if case .copper = $0 { true } else { false } } }

    var body: some View {
        VStack(alignment: .leading, spacing: 14) {
            Text(action.title)
                .font(.headline)
            HStack(alignment: .top, spacing: 18) {
                VStack(alignment: .leading, spacing: 14) {
                    Form {
                        Picker("Paper:", selection: $options.paper) {
                            ForEach(PrintPaper.allCases) { Text($0.title).tag($0) }
                        }
                    }
                    GroupBox("Pages") {
                        VStack(alignment: .leading, spacing: 6) {
                            ForEach(kinds, id: \.self) { kind in
                                Toggle(isOn: shown(kind)) {
                                    HStack {
                                        Text(kind.title)
                                        Spacer()
                                        Text(count(of: kind))
                                            .foregroundStyle(.secondary)
                                            .monospacedDigit()
                                    }
                                }
                                .toggleStyle(.checkbox)
                            }
                        }
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .padding(4)
                    }
                    if hasCopper {
                        GroupBox("Copper pages (etching)") {
                            VStack(alignment: .leading, spacing: 6) {
                                Toggle("Mirror (toner transfer, film face down)", isOn: $options.mirror)
                                Toggle("Negative (positive-resist board)", isOn: $options.negative)
                                Toggle("Registration marks", isOn: $options.marks)
                            }
                            .toggleStyle(.checkbox)
                            .frame(maxWidth: .infinity, alignment: .leading)
                            .padding(4)
                        }
                    }
                    Text("Board pages print 1:1: choose Actual Size, or 100 %, in the printer's dialog, and check the 50 mm ruler on each page. A board bigger than the paper is cut into overlapping tiles.")
                        .font(.callout)
                        .foregroundStyle(.secondary)
                        .fixedSize(horizontal: false, vertical: true)
                }
                .frame(width: 300)
                PDFPreview(data: preview)
                    .frame(width: 340, height: 440)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
                    .overlay(RoundedRectangle(cornerRadius: 6).strokeBorder(.separator))
            }
            HStack {
                Text(pages.isEmpty ? "No pages chosen." : "\(pages.count) page\(pages.count == 1 ? "" : "s")")
                    .font(.callout)
                    .foregroundStyle(.secondary)
                Spacer()
                Button("Cancel", action: onCancel)
                    .keyboardShortcut(.cancelAction)
                Button(action.button) { onDone(options) }
                    .keyboardShortcut(.defaultAction)
                    .disabled(pages.isEmpty)
            }
        }
        .padding(20)
        .task(id: options) {
            try? await Task.sleep(for: .milliseconds(120))
            guard !Task.isCancelled else { return }
            if artwork == nil { artwork = PrintExport.artwork(for: document) }
            preview = PrintExport.pdf(for: document, name: name, options, artwork: artwork)
        }
    }

    private func shown(_ kind: PrintPageKind) -> Binding<Bool> {
        Binding(
            get: { !options.hidden.contains(kind) },
            set: { on in
                if on { options.hidden.remove(kind) } else { options.hidden.insert(kind) }
            }
        )
    }

    /// "1 page", "4 pages (2 × 2 tiles)".
    private func count(of kind: PrintPageKind) -> String {
        var only = options
        only.hidden = Set(kinds).subtracting([kind])
        let own = PrintExport.pages(for: document, only)
        guard let first = own.first else { return "" }
        if own.count == 1 { return "1 page" }
        return "\(own.count) pages (\(first.columns) × \(first.rows) tiles)"
    }
}

/// A PDF shown scrolled and fitted to the width.
private struct PDFPreview: NSViewRepresentable {
    let data: Data?

    func makeNSView(context: Context) -> PDFView {
        let view = PDFView()
        view.autoScales = true
        view.displayMode = .singlePageContinuous
        view.displayDirection = .vertical
        view.backgroundColor = .underPageBackgroundColor
        return view
    }

    func updateNSView(_ view: PDFView, context: Context) {
        view.document = data.flatMap { PDFDocument(data: $0) }
    }
}
