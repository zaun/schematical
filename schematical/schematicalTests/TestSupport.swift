import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Shared helpers for the app's unit tests.
@MainActor
enum TestSupport {
    /// The part catalog, from the host app's bundle.
    static let catalog: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    /// Documents made by tests, kept for the life of the test process.
    /// Freeing a `SchematicDocument` inside a test method aborts the host
    /// ("pointer being freed was not allocated" in the Swift runtime's
    /// isolated-deinit path, `swift_task_deinitOnExecutor`, on macOS 15.8).
    private static var keptDocuments: [SchematicDocument] = []

    static func newDocument() -> SchematicDocument {
        let doc = SchematicDocument()
        keptDocuments.append(doc)
        return doc
    }

    /// A document with the catalog's part library, so edits run the voltage
    /// check. Kept like `newDocument()`.
    static func newDocument(library: PartLibraryHandle?) -> SchematicDocument {
        let doc = SchematicDocument(library: library)
        keptDocuments.append(doc)
        return doc
    }

    static func part(_ name: String) throws -> CatalogPart {
        try XCTUnwrap(catalog.part(named: name), "no part \(name)")
    }

    /// Places a part centred on `point`, as a drop from the drawer does.
    @discardableResult
    static func place(_ doc: SchematicDocument, _ name: String, at point: CGPoint) throws -> PlacedChip {
        let part = try part(name)
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return try XCTUnwrap(doc.chips.last)
    }

    /// The first DIP switch (by name) with at least two positions.
    static func firstDip() throws -> CatalogPart {
        let parts = catalog.groups(matching: DrawerFilter()).flatMap { category in
            category.functions.flatMap(\.parts) + category.subcategories.flatMap { sub in
                sub.parts + sub.functions.flatMap(\.parts)
            }
        }
        return try XCTUnwrap(parts.filter { $0.kind == PART_SWITCH && !$0.isTactileSwitch && ($0.count ?? 0) >= 2 }
            .sorted { $0.name < $1.name }.first, "no DIP switch with two positions")
    }

    static func pin(_ chip: PlacedChip, _ name: String) throws -> PinRef {
        let slot = try XCTUnwrap(chip.layout.slots.first { $0.pin.name == name }, "\(chip.partNumber) has no pin \(name)")
        return PinRef(chipID: chip.id, index: slot.pin.index)
    }
}
