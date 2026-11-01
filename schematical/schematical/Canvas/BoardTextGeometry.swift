import AppKit

/// A board text's size and place, measured with the same font the PCB page
/// draws it in, so hit testing matches the drawing.
///
/// Texts are a millimetre or so tall, and AppKit rounds line heights up to
/// whole points, so a text is laid out at `referenceSize` points and scaled
/// down by `drawScale` to its size in mm, both when measured and when drawn.
extension BoardText {
    /// The font size texts are laid out at before scaling.
    nonisolated static let referenceSize: CGFloat = 100

    /// The system font at `referenceSize`; drawn scaled by `drawScale`.
    nonisolated var font: NSFont {
        NSFont.systemFont(ofSize: Self.referenceSize, weight: .medium)
    }

    /// Board mm per point of `font`.
    nonisolated var drawScale: CGFloat {
        sizeMM / Self.referenceSize
    }

    /// The text's size in mm at rotation 0.
    nonisolated var localSize: CGSize {
        let laidOut = (text as NSString).boundingRect(
            with: CGSize(width: CGFloat.greatestFiniteMagnitude, height: .greatestFiniteMagnitude),
            options: .usesLineFragmentOrigin,
            attributes: [.font: font]
        ).size
        return CGSize(width: laidOut.width * drawScale, height: laidOut.height * drawScale)
    }

    /// The box the text covers on the board: its rectangle from `anchor`,
    /// turned about `anchor`.
    nonisolated var boardRect: CGRect {
        let size = localSize
        let corners = [CGPoint.zero, CGPoint(x: size.width, y: 0),
                       CGPoint(x: size.width, y: size.height), CGPoint(x: 0, y: size.height)]
            .map { BoardPlacement.rotate($0, by: rotation) }
        let xs = corners.map(\.x), ys = corners.map(\.y)
        return CGRect(x: anchor.x + xs.min()!, y: anchor.y + ys.min()!,
                      width: xs.max()! - xs.min()!, height: ys.max()! - ys.min()!)
    }
}
