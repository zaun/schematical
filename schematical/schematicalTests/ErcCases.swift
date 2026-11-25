import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The drawings the ERC tests (plan-63-E) and the KiCad comparison (plan-63-G)
/// are made of: one named function per case, each returning a document built
/// with the catalog's part library, so pin types and rails are the app's.
@MainActor
enum ErcCases {
    static func newDocument() -> SchematicDocument {
        TestSupport.newDocument(library: TestSupport.catalog.library)
    }

    @discardableResult
    static func place(_ doc: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) throws -> PlacedChip {
        try TestSupport.place(doc, name, at: CGPoint(x: x, y: y))
    }

    static func pin(_ chip: PlacedChip, _ name: String) throws -> PinRef { try TestSupport.pin(chip, name) }

    /// Wires two ends; the wire must be accepted.
    static func wire(_ doc: SchematicDocument, _ a: LineAnchor, _ b: LineAnchor,
                     file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertNil(doc.connect(a, b), file: file, line: line)
    }

    // MARK: Pin rules

    /// Two NAND gates whose outputs are wired together: `pin_to_pin` (output + output).
    static func pinConflict() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        wire(doc, .pin(try pin(one, "1Y")), .pin(try pin(two, "1Y")))
        return doc
    }

    /// An unwired gate input: `pin_not_connected`.
    static func pinNotConnected() throws -> SchematicDocument {
        let doc = newDocument()
        try place(doc, "SN74HC00N", 200, 200)
        return doc
    }

    /// Two gate inputs wired together and nothing drives them: `pin_not_driven`.
    static func pinNotDriven() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        wire(doc, .pin(try pin(one, "1A")), .pin(try pin(two, "1A")))
        return doc
    }

    /// A gate's VCC wired to a header pin and nothing drives the net: `power_pin_not_driven`.
    static func powerPinNotDriven() throws -> SchematicDocument {
        let doc = newDocument()
        let gate = try place(doc, "SN74HC00N", 200, 200), header = try place(doc, "GENERIC-HEADER-1X02", 700, 200)
        wire(doc, .pin(try pin(gate, "VCC")), .pin(try pin(header, "1")))
        return doc
    }

    /// A No Connect on an input that is also wired to an output: `no_connect_connected`.
    static func noConnectConnected() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        let input = try pin(one, "1A")
        wire(doc, .pin(input), .pin(try pin(two, "1Y")))
        let flag = doc.addNetLabel(at: CGPoint(x: 100, y: 500), style: .noConnect)
        wire(doc, .pin(input), .label(flag))
        return doc
    }
}
