import SwiftUI

/// The Part Editor's PCB preview (plan-32-D): the part's footprint drawn the
/// way the PCB page draws a placed one, from the same `PlacedChip` geometry
/// (`boardPads`, `boardBody`, `pin1Mark`) and the same pad outline
/// (`PCBCanvasView.outline(of:)`) and colours.
enum FootprintPreview {
    /// The preview's chip: the part placed at the board origin, unrotated, on top.
    static func chip(for part: CatalogPart) -> PlacedChip {
        PlacedChip(id: UUID(uuidString: "00000000-0000-0000-0000-000000000002")!, reference: "U1", part: part,
                   symbol: symbol(for: part), origin: .zero,
                   board: BoardPlacement(centre: .zero, rotation: 0))
    }
}

struct FootprintPreviewView: View {
    let part: CatalogPart
    /// Changes whenever `part` does (see `PartEditorModel.previewRevision`).
    var revision = 0
    /// Zoom (the PCB page's viewport type); 100 % fits the footprint.
    @State private var viewport = CanvasViewport(baseScale: 1)
    @State private var panStart: CGSize?

    var body: some View {
        let chip = FootprintPreview.chip(for: part)
        VStack(spacing: 4) {
            Canvas { context, size in
                draw(chip, in: &context, size: size)
            }
            .background(PCBCanvasView.background)
            .gesture(DragGesture()
                .onChanged { value in
                    let start = panStart ?? viewport.offset
                    panStart = start
                    viewport.pan(to: CGSize(width: start.width + value.translation.width,
                                            height: start.height + value.translation.height))
                }
                .onEnded { _ in panStart = nil })
            HStack {
                Text(part.footprint.map { "\($0.id): \($0.pads.count) pads" } ?? "No footprint")
                    .font(.caption).foregroundStyle(.secondary)
                Spacer()
                Button { viewport.zoomOut() } label: { Image(systemName: "minus.magnifyingglass") }
                Button { viewport.setZoom(1); viewport.pan(to: .zero) } label: { Text("Fit") }
                Button { viewport.zoomIn() } label: { Image(systemName: "plus.magnifyingglass") }
            }
            .buttonStyle(.borderless)
        }
    }

    private func draw(_ chip: PlacedChip, in context: inout GraphicsContext, size: CGSize) {
        guard let footprint = chip.footprint, let bounds = chip.boardBounds, bounds.width > 0, bounds.height > 0 else {
            return
        }
        // Fit the footprint (with a margin), then the viewport's zoom and pan.
        let fit = min(size.width / (bounds.width * 1.3), size.height / (bounds.height * 1.3))
        let scale = fit * viewport.zoom
        context.translateBy(x: size.width / 2 + viewport.offset.width, y: size.height / 2 + viewport.offset.height)
        context.scaleBy(x: scale, y: scale)
        context.translateBy(x: -bounds.midX, y: -bounds.midY)

        let board = BoardColor.green
        let ink = board.silkscreen
        let line = 1.5 / scale
        if let body = chip.boardBody {
            let path = body.round ? Path(ellipseIn: body.rect) : Path(body.rect)
            context.stroke(path, with: .color(BoardColor.document), lineWidth: line)
        }
        for pad in chip.boardPads {
            let outline = PCBCanvasView.outline(of: pad)
            if pad.plated {
                context.fill(outline, with: .color(PCBCanvasView.copper))
            } else {
                context.stroke(outline, with: .color(ink), lineWidth: line)
            }
            if let hole = pad.holeSegment {
                var path = Path()
                path.move(to: hole.a)
                path.addLine(to: hole.b)
                context.stroke(path, with: .color(PCBCanvasView.background),
                               style: StrokeStyle(lineWidth: hole.radius * 2, lineCap: .round))
            }
            if let designator = pad.designator {
                let height = min(pad.rect.width, pad.rect.height)
                context.draw(Text(designator).font(.system(size: max(height * 0.5, 0.05), design: .monospaced))
                    .foregroundColor(.black), at: CGPoint(x: pad.rect.midX, y: pad.rect.midY))
            }
        }
        if let mark = chip.pin1Mark {
            let r = max(min(footprint.body.width, footprint.body.height) * 0.04, 0.15)
            context.fill(Path(ellipseIn: CGRect(x: mark.x - r, y: mark.y - r, width: 2 * r, height: 2 * r)),
                         with: .color(ink))
        }
    }
}
