import SwiftUI

/// File ▸ Import ▸ DXF…'s sheet (plan-65-I): the drawing's loops, which layers, which units and which mode,
/// what will be made and what was left out, over a preview. Import goes on to
/// `SchematicDocument.applyDXFImport`.
struct DXFImportSheet: View {
    @Bindable var state: DXFImportState
    let onCancel: () -> Void
    let onImport: () -> Void

    var body: some View {
        let problem = state.problem
        VStack(alignment: .leading, spacing: 14) {
            Text("Import “\(state.fileName)”")
                .font(.headline)

            Picker("Make", selection: $state.mode) {
                ForEach(DXFImportMode.allCases, id: \.self) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()

            preview
                .frame(width: 420, height: 280)

            Form {
                if state.drawing.layers.count > 1 {
                    LabeledContent("Layers:") {
                        VStack(alignment: .leading, spacing: 2) {
                            ForEach(state.drawing.layers, id: \.self) { layer in
                                Toggle(layer, isOn: Binding(
                                    get: { state.layers.contains(layer) },
                                    set: { if $0 { state.layers.insert(layer) } else { state.layers.remove(layer) } }))
                            }
                        }
                    }
                }
                Picker("Units:", selection: $state.units) {
                    ForEach(DXFUnits.allCases, id: \.self) { Text($0.title).tag($0) }
                }
                .help(state.drawing.units == nil ? "The drawing doesn't say its units" : "The drawing says \(state.drawing.units?.title ?? "")")
                if state.mode == .cutoutsOnly {
                    LabeledContent("Origin on the board:") {
                        HStack {
                            TextField("X", value: $state.offsetX, format: .number.precision(.fractionLength(0...3)))
                            TextField("Y", value: $state.offsetY, format: .number.precision(.fractionLength(0...3)))
                            Text("mm")
                        }
                        .frame(width: 220)
                    }
                    .help("Where the drawing's (0, 0) lands, in mm from the board's top-left; the drawing's y goes up")
                } else {
                    Text("Replaces the board's edge and its cutouts. Parts and traces stay where they are.")
                        .font(.callout)
                        .foregroundStyle(.secondary)
                }
            }

            VStack(alignment: .leading, spacing: 2) {
                Text(state.summary.isEmpty ? "Nothing to import" : state.summary)
                    .font(.callout.weight(.medium))
                ForEach(state.notes, id: \.self) { note in
                    Text("Left out: " + note)
                        .font(.callout)
                        .foregroundStyle(.secondary)
                }
                if let problem {
                    Text(problem)
                        .font(.callout)
                        .foregroundStyle(.red)
                }
            }

            HStack {
                Spacer()
                Button("Cancel", action: onCancel)
                    .keyboardShortcut(.cancelAction)
                Button("Import", action: onImport)
                    .keyboardShortcut(.defaultAction)
                    .disabled(problem != nil)
            }
        }
        .padding(20)
        .frame(width: 460)
    }

    /// The loops in board mm, outline in the accent colour, cutouts red, left-out loops gray; in cutouts mode
    /// the board's own outline dashed.
    private var preview: some View {
        Canvas { context, size in
            let loops = state.previewLoops
            var all = loops.flatMap(\.points)
            if state.mode == .cutoutsOnly { all += state.boardOutline }
            guard let minX = all.map(\.x).min(), let maxX = all.map(\.x).max(),
                  let minY = all.map(\.y).min(), let maxY = all.map(\.y).max() else { return }
            let width = max(maxX - minX, 1), height = max(maxY - minY, 1)
            let scale = min((size.width - 16) / width, (size.height - 16) / height)
            func path(_ points: [CGPoint]) -> Path {
                Path { path in
                    for (i, point) in points.enumerated() {
                        let p = CGPoint(x: 8 + (point.x - minX) * scale, y: 8 + (point.y - minY) * scale)
                        if i == 0 { path.move(to: p) } else { path.addLine(to: p) }
                    }
                    path.closeSubpath()
                }
            }
            if state.mode == .cutoutsOnly {
                context.stroke(path(state.boardOutline), with: .color(.secondary),
                               style: StrokeStyle(lineWidth: 1, dash: [4, 3]))
            }
            for loop in loops {
                let color: Color = switch loop.role {
                case .outline: .accentColor
                case .cutout: .red
                case .ignored: .gray
                }
                context.stroke(path(loop.points), with: .color(color), lineWidth: loop.role == .ignored ? 1 : 1.5)
            }
        }
        .background(Color(nsColor: .textBackgroundColor), in: RoundedRectangle(cornerRadius: 6))
        .overlay(RoundedRectangle(cornerRadius: 6).stroke(.quaternary))
    }
}
