import SwiftUI

/// File > Export > Image…'s options sheet: what to draw (schematic, PCB
/// page, front or back render), the format, the size and the background,
/// plus the PCB's layers or the render's view, over a live
/// preview. Export… goes on to the save panel.
struct ImageExportDialog: View {
    let document: SchematicDocument
    @State var options: ImageExportOptions
    let onCancel: () -> Void
    let onExport: (ImageExportOptions) -> Void

    private var o: ImageExportOptions { options.normalized() }

    var body: some View {
        VStack(alignment: .leading, spacing: 14) {
            Text("Export Image")
                .font(.headline)

            Picker("Image", selection: $options.view) {
                ForEach(ImageExportView.allCases) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()

            preview

            Form {
                Picker("Format:", selection: $options.format) {
                    ForEach(o.view.formats) { Text($0.title).tag($0) }
                }
                if o.format != .svg {
                    Picker("Size:", selection: $options.scale) {
                        ForEach(ImageExportOptions.scales, id: \.self) { Text("\(Int($0))×").tag($0) }
                    }
                }
                Picker("Background:", selection: $options.background) {
                    ForEach(o.view.backgrounds.filter { o.format.allowsTransparency || $0 != .transparent }) {
                        Text($0.title(for: o.view)).tag($0)
                    }
                }
                if o.view == .pcb {
                    LabeledContent("Layers:") { layers }
                }
                if o.view.face != nil {
                    Picker("View:", selection: $options.angle) {
                        ForEach(BoardRenderAngle.allCases) { Text($0.title).tag($0) }
                    }
                }
            }

            HStack {
                Text(sizeText)
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .monospacedDigit()
                Spacer()
                Button("Cancel", action: onCancel)
                    .keyboardShortcut(.cancelAction)
                Button("Export…") { onExport(o) }
                    .keyboardShortcut(.defaultAction)
                    .disabled(ImageExport.pixelSize(of: document, o) == nil)
            }
        }
        .padding(20)
        .frame(width: 520)
        .onChange(of: options) { _, new in
            let fixed = new.normalized()
            if fixed != new { options = fixed }
        }
    }

    /// The picture fitted into a fixed box, over a checkerboard where it
    /// is transparent.
    private var preview: some View {
        let box = CGSize(width: 480, height: 280)
        let picture = ImageExport.picture(of: document, o)
        return ZStack {
            Checkerboard()
            if let picture {
                Canvas { context, size in
                    let s = min(size.width / picture.size.width, size.height / picture.size.height)
                    let fitted = CGSize(width: picture.size.width * s, height: picture.size.height * s)
                    context.translateBy(x: (size.width - fitted.width) / 2, y: (size.height - fitted.height) / 2)
                    context.clip(to: Path(CGRect(origin: .zero, size: fitted)))
                    context.scaleBy(x: s, y: s)
                    picture.draw(&context)
                }
            } else {
                Text("Nothing to export: add a part, wire or bus first.")
                    .foregroundStyle(.secondary)
            }
        }
        .frame(width: box.width, height: box.height)
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .overlay(RoundedRectangle(cornerRadius: 6).strokeBorder(.separator))
    }

    /// A check box per PCB page layer: PCB, each copper layer, Silkscreen,
    /// Document, Ratsnest.
    private var layers: some View {
        let all: [BoardViewLayer] = [.board] + document.board.copperLayers.map { .copper($0) }
            + [.silkscreen, .document, .ratsnest]
        return LazyVGrid(columns: [GridItem(.adaptive(minimum: 100), alignment: .leading)], alignment: .leading) {
            ForEach(all, id: \.self) { layer in
                Toggle(layer.title, isOn: Binding(
                    get: { !options.hiddenLayers.contains(layer) },
                    set: { shown in
                        if shown { options.hiddenLayers.remove(layer) } else { options.hiddenLayers.insert(layer) }
                    }
                ))
                .toggleStyle(.checkbox)
            }
        }
    }

    private var sizeText: String {
        guard let size = ImageExport.pixelSize(of: document, o) else { return "" }
        let unit = o.format == .svg ? "pt" : "px"
        return "\(Int(size.width)) × \(Int(size.height)) \(unit)"
    }
}

/// Light and mid-gray squares, behind a transparent preview.
private struct Checkerboard: View {
    var body: some View {
        Canvas { context, size in
            context.fill(Path(CGRect(origin: .zero, size: size)), with: .color(Color(white: 0.92)))
            let step: CGFloat = 10
            var squares = Path()
            var y: CGFloat = 0
            var row = 0
            while y < size.height {
                var x: CGFloat = row % 2 == 0 ? 0 : step
                while x < size.width {
                    squares.addRect(CGRect(x: x, y: y, width: step, height: step))
                    x += 2 * step
                }
                y += step
                row += 1
            }
            context.fill(squares, with: .color(Color(white: 0.8)))
        }
    }
}
