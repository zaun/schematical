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

    // MARK: Drawing rules

    /// The middle of a block, where a click erases it.
    static func centre(_ chip: PlacedChip) -> CGPoint { CGPoint(x: chip.bodyRect.midX, y: chip.bodyRect.midY) }

    /// A No Connect that is on no pin: `no_connect_dangling`.
    static func noConnectDangling() -> SchematicDocument {
        let doc = newDocument()
        _ = doc.addNetLabel(at: CGPoint(x: 300, y: 300), style: .noConnect)
        return doc
    }

    /// Two gates wired, then both erased: a wire with a loose end at each end
    /// (`wire_dangling`, and two `unconnected_wire_endpoint`).
    static func wireDangling() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        wire(doc, .pin(try pin(one, "1Y")), .pin(try pin(two, "1A")))
        doc.erase(at: centre(one))
        doc.erase(at: centre(two))
        return doc
    }

    /// A gate wired to a gate, and one gate erased: one loose end (`unconnected_wire_endpoint`).
    static func wireToNowhere() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        wire(doc, .pin(try pin(one, "1Y")), .pin(try pin(two, "1A")))
        doc.erase(at: centre(two))
        return doc
    }

    /// A net label with no wire: `label_dangling`.
    static func labelDangling() -> SchematicDocument {
        let doc = newDocument()
        _ = doc.addNetLabel(at: CGPoint(x: 300, y: 300))
        return doc
    }

    /// One pin wired to one label: `isolated_pin_label` (and `single_global_label`, off by default).
    static func isolatedPinLabel() throws -> SchematicDocument {
        let doc = newDocument()
        let gate = try place(doc, "SN74HC00N", 200, 200)
        let label = doc.addNetLabel(at: CGPoint(x: 600, y: 200))
        XCTAssertTrue(doc.setNetLabelName(label, "SIG"))
        wire(doc, .pin(try pin(gate, "1Y")), .label(label))
        return doc
    }

    /// Labels `C1` and `C2` on one wire: `multiple_net_names`.
    static func multipleNetNames() throws -> SchematicDocument {
        let doc = newDocument()
        let gate = try place(doc, "SN74HC00N", 200, 200)
        let one = doc.addNetLabel(at: CGPoint(x: 600, y: 100)), two = doc.addNetLabel(at: CGPoint(x: 600, y: 300))
        XCTAssertTrue(doc.setNetLabelName(one, "C1"))
        XCTAssertTrue(doc.setNetLabelName(two, "C2"))
        wire(doc, .pin(try pin(gate, "1Y")), .label(one))
        wire(doc, .pin(try pin(gate, "1Y")), .label(two))
        return doc
    }

    /// Labels `CLK` and `clk` on two nets, each with a pin: `similar_labels`.
    static func similarLabels() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 200, 700)
        let upper = doc.addNetLabel(at: CGPoint(x: 600, y: 200)), lower = doc.addNetLabel(at: CGPoint(x: 600, y: 700))
        XCTAssertTrue(doc.setNetLabelName(upper, "CLK"))
        XCTAssertTrue(doc.setNetLabelName(lower, "clk"))
        wire(doc, .pin(try pin(one, "1Y")), .label(upper))
        wire(doc, .pin(try pin(two, "1Y")), .label(lower))
        return doc
    }

    /// A `5V` symbol and a label `5v`: `similar_label_and_power`.
    static func similarLabelAndPower() throws -> SchematicDocument {
        let doc = newDocument()
        let gate = try place(doc, "SN74HC00N", 200, 200), rail = try place(doc, "5V", 600, 100)
        wire(doc, .pin(try pin(gate, "VCC")), .pin(try pin(rail, "VCC")))
        let other = try place(doc, "SN74HC00N", 200, 700)
        let label = doc.addNetLabel(at: CGPoint(x: 600, y: 700))
        XCTAssertTrue(doc.setNetLabelName(label, "5v"))
        wire(doc, .pin(try pin(other, "1Y")), .label(label))
        return doc
    }

    /// One label name used once, with two pins on its net: only `single_global_label` (off by default).
    static func singleLabel() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        let label = doc.addNetLabel(at: CGPoint(x: 450, y: 600))
        XCTAssertTrue(doc.setNetLabelName(label, "ONCE"))
        wire(doc, .pin(try pin(one, "1Y")), .label(label))
        wire(doc, .pin(try pin(two, "1A")), .label(label))
        return doc
    }

    /// Four wires at one junction: `four_way_junction` (off by default).
    static func fourWayJunction() throws -> SchematicDocument {
        let doc = newDocument()
        let west = try place(doc, "SN74HC00N", 200, 400), east = try place(doc, "SN74HC00N", 900, 400)
        let north = try place(doc, "SN74HC00N", 550, 100), south = try place(doc, "SN74HC00N", 550, 700)
        wire(doc, .pin(try pin(west, "1Y")), .pin(try pin(east, "1A")))
        // A grid point on that wire: the wire is split there and the others branch from it.
        let wirePath = doc.path(forWire: doc.wires[0].id)
        let point = SchematicGrid.snap(CGPoint(x: (wirePath[0].x + wirePath[1].x) / 2, y: (wirePath[0].y + wirePath[1].y) / 2))
        wire(doc, .pin(try pin(north, "1Y")), .point(point))
        wire(doc, .pin(try pin(south, "1Y")), .point(point))
        return doc
    }

    /// Two parts with one reference (a hand-edited file: Rename refuses it): `duplicate_reference`.
    static func duplicateReference() throws -> SchematicDocument {
        let doc = newDocument()
        let one = try place(doc, "SN74HC00N", 200, 200), two = try place(doc, "SN74HC00N", 700, 200)
        let text = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
            .replacingOccurrences(of: "\"\(two.reference)\"", with: "\"\(one.reference)\"")
        let edited = newDocument()
        try SchematicFile.decode(Data(text.utf8)).apply(to: edited, catalog: TestSupport.catalog, fileURL: nil)
        return edited
    }
}
