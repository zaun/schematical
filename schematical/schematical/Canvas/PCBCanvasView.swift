import SwiftUI

/// The PCB page: every placed part that has a footprint, drawn as its pads,
/// drill holes, body outline, pin-1 mark and reference, on a 0.635 mm grid.
///
/// Press on a footprint to select it and drag to move it (snapped to the
/// grid); press on empty space to clear the selection and pan. Nothing runs
/// while this page shows (`ContentView` stops a run when switching to it).
///
/// Board positions are millimetres; the view draws `BoardScale.pointsPerMM`
/// world units per mm and the viewport maps world units to the screen.
struct PCBCanvasView: View {
    let document: SchematicDocument
    let viewport: CanvasViewport

    @State private var interaction: Interaction?

    private enum Interaction {
        case move(id: UUID, startCentre: CGPoint)
        case pan(startOffset: CGSize)
    }

    static let background = Color(white: 0.13)
    static let outline = Color(white: 0.85)
    static let copper = Color(red: 0.80, green: 0.62, blue: 0.30)

    /// Screen points per board millimetre.
    private var scale: CGFloat { viewport.zoom * BoardScale.pointsPerMM }

    var body: some View {
        Canvas { context, size in
            draw(in: &context, size: size)
        }
        .background(Self.background)
        .contentShape(Rectangle())
        .clipped()
        .onGeometryChange(for: CGSize.self) { proxy in
            proxy.size
        } action: { newSize in
            viewport.updateSize(newSize)
        }
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { value in handleChanged(value) }
                .onEnded { _ in interaction = nil }
        )
    }

    /// A screen point in board millimetres.
    private func board(_ screen: CGPoint) -> CGPoint {
        let world = viewport.world(screen)
        return CGPoint(x: world.x / BoardScale.pointsPerMM, y: world.y / BoardScale.pointsPerMM)
    }

    // MARK: Drawing

    private func draw(in context: inout GraphicsContext, size: CGSize) {
        // Everything below is in board mm.
        context.translateBy(x: viewport.offset.width, y: viewport.offset.height)
        context.scaleBy(x: scale, y: scale)
        drawGrid(in: context, size: size)

        for chip in document.chips {
            guard let body = chip.boardBody else { continue }
            let outline = Path { path in
                if body.round {
                    path.addEllipse(in: body.rect)
                } else {
                    path.addRect(body.rect)
                }
            }
            context.stroke(outline, with: .color(Self.outline), lineWidth: 0.15)

            for pad in chip.boardPads {
                let shape = Path { path in
                    if pad.shape == .round {
                        path.addEllipse(in: pad.rect)
                    } else {
                        path.addRect(pad.rect)
                    }
                }
                context.fill(shape, with: .color(Self.copper))
                if pad.drill > 0 {
                    let d = pad.drill
                    let hole = CGRect(x: pad.rect.midX - d / 2, y: pad.rect.midY - d / 2, width: d, height: d)
                    context.fill(Path(ellipseIn: hole), with: .color(Self.background))
                }
            }

            if let mark = chip.pin1Mark {
                context.fill(Path(ellipseIn: CGRect(x: mark.x - 0.2, y: mark.y - 0.2, width: 0.4, height: 0.4)),
                             with: .color(Self.outline))
            }

            // Reference: 1 mm tall, but never under 8 points on screen.
            let height = max(1, 8 / scale)
            let text = Text(chip.reference)
                .font(.system(size: height, weight: .medium))
                .foregroundStyle(Self.outline)
            context.draw(text, at: CGPoint(x: body.rect.midX, y: body.rect.midY), anchor: .center)

            if chip.id == document.selectedChipID, let bounds = chip.boardBounds {
                let margin = 3 / scale
                context.stroke(Path(bounds.insetBy(dx: -margin, dy: -margin)),
                               with: .color(.accentColor), lineWidth: 2 / scale)
            }
        }
    }

    /// Dots every 2.54 mm (4 grid steps), hidden when they would be closer
    /// than 8 points on screen.
    private func drawGrid(in context: GraphicsContext, size: CGSize) {
        let step = BoardGrid.spacing * 4
        guard step * scale >= 8 else { return }
        let topLeft = board(.zero)
        let bottomRight = board(CGPoint(x: size.width, y: size.height))
        let dot = 2.5 / scale
        var dots = Path()
        var x = (topLeft.x / step).rounded(.down) * step
        while x <= bottomRight.x {
            var y = (topLeft.y / step).rounded(.down) * step
            while y <= bottomRight.y {
                dots.addEllipse(in: CGRect(x: x - dot / 2, y: y - dot / 2, width: dot, height: dot))
                y += step
            }
            x += step
        }
        context.fill(dots, with: .color(Color(white: 0.42)))
    }

    // MARK: Gestures

    private func handleChanged(_ value: DragGesture.Value) {
        if interaction == nil {
            if let hit = document.footprint(at: board(value.startLocation)),
               let placement = hit.board {
                document.selectedChipID = hit.id
                interaction = .move(id: hit.id, startCentre: placement.centre)
            } else {
                document.selectedChipID = nil
                interaction = .pan(startOffset: viewport.offset)
            }
        }
        switch interaction {
        case let .move(id, startCentre)?:
            document.moveFootprint(id, to: CGPoint(
                x: startCentre.x + value.translation.width / scale,
                y: startCentre.y + value.translation.height / scale
            ))
        case let .pan(startOffset)?:
            viewport.pan(to: CGSize(
                width: startOffset.width + value.translation.width,
                height: startOffset.height + value.translation.height
            ))
        case nil:
            break
        }
    }
}
