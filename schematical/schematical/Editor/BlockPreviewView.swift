import SwiftUI

/// The Part Editor's Block pane (plan-32-E): the schematic page's own
/// drawing of the part (`ChipBlockView.paint`), at the page's default scale,
/// centred. Not interactive; every pin's designator shows (nothing is wired).
struct BlockPreviewView: View {
    let chip: PlacedChip
    /// Changes whenever the part does (see `PartEditorModel.previewRevision`).
    var revision = 0

    /// The schematic page's default scale (`ContentView`'s `CanvasViewport(baseScale: 0.75)`).
    static let scale: CGFloat = 0.75

    var body: some View {
        let layout = chip.layout
        let width = layout.size.width + 2 * ChipLayout.labelMargin
        let height = layout.size.height
        let drawing = Canvas { context, _ in
            context.scaleBy(x: Self.scale, y: Self.scale)
            var painter = ScreenPainter(context)
            ChipBlockView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
        }
        .frame(width: width * Self.scale, height: height * Self.scale)
        .padding(16)
        // Centred when it fits; a large part (a processor) scrolls.
        ViewThatFits {
            drawing.frame(maxWidth: .infinity, maxHeight: .infinity)
            ScrollView([.horizontal, .vertical]) { drawing }
        }
    }
}
