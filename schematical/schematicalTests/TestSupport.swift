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

    /// A saved file's text without its `board.designRules` object, to
    /// compare with a golden from before the rules were kept in the file
    /// (plan-49). Fails when the object isn't there.
    static func withoutDesignRules(_ text: String, file: StaticString = #filePath, line: UInt = #line) -> String {
        let stripped = text.replacingOccurrences(of: #"\n    "designRules" : \{[^}]*\},"#, with: "",
                                                 options: .regularExpression)
        XCTAssertNotEqual(stripped, text, "the file has no designRules", file: file, line: line)
        return stripped
    }

    /// A saved file's text without its `board.stackup` object, to compare
    /// with a golden from before the stackup was kept in the file (plan-50-A).
    /// Fails when the object isn't there.
    static func withoutStackup(_ text: String, file: StaticString = #filePath, line: UInt = #line) -> String {
        let stripped = text.replacingOccurrences(of: #"(?s)\n    "stackup" : \{.*?\n    \},?"#, with: "",
                                                 options: .regularExpression)
        XCTAssertNotEqual(stripped, text, "the file has no stackup", file: file, line: line)
        return stripped
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

    /// Every part in the drawer.
    static var allParts: [CatalogPart] {
        catalog.groups(matching: DrawerFilter()).flatMap(\.allParts)
    }

    /// The first DIP switch (by name) with at least two positions.
    static func firstDip() throws -> CatalogPart {
        try XCTUnwrap(allParts.filter { $0.kind == PART_SWITCH && !$0.isTactileSwitch && ($0.count ?? 0) >= 2 }
            .sorted { $0.name < $1.name }.first, "no DIP switch with two positions")
    }

    static func pin(_ chip: PlacedChip, _ name: String) throws -> PinRef {
        let slot = try XCTUnwrap(chip.layout.slots.first { $0.pin.name == name }, "\(chip.partNumber) has no pin \(name)")
        return PinRef(chipID: chip.id, index: slot.pin.index)
    }
}
