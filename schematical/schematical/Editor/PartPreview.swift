import CoreGraphics
import Foundation

/// The Part Editor's schematic preview (plan-32-E): the part as a placed
/// chip, built the way the schematic page builds one, without a document.
enum PartPreview {
    /// The preview's chip: origin (0, 0), reference U1, the part's own symbol
    /// and its default options. The fixed id never reaches a document.
    static func chip(for part: CatalogPart) -> PlacedChip {
        PlacedChip(id: UUID(uuidString: "00000000-0000-0000-0000-000000000001")!,
                   reference: "U1", part: part, symbol: symbol(for: part),
                   origin: .zero)
    }
}
