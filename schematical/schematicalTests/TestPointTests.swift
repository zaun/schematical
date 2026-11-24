import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-57: the generic test point (`GENERIC-TESTPOINT`, a `connector` device
/// with `"testPoint": true`): one passive pin, a bare pad or plated hole, listed
/// as the leaf Test Point directly under Connector.
@MainActor
final class TestPointTests: XCTestCase {
    private func testPoint() throws -> CatalogPart {
        try TestSupport.part("GENERIC-TESTPOINT")
    }

    /// plan-57-A: the part, its pin, drawer leaf, footprints, kind name and
    /// the catalog's recognition of it.
    func testThePartIsOnePassivePinWithTenFootprints() throws {
        let part = try testPoint()
        XCTAssertEqual(part.kind, PART_CONNECTOR)
        XCTAssertTrue(part.isTestPoint)
        XCTAssertEqual(part.pins.count, 1)
        XCTAssertEqual(part.pins.first?.name, "TP")
        XCTAssertEqual(part.drawer, ["Connector"])
        XCTAssertEqual(part.function, "Test Point")
        XCTAssertEqual(part.manufacturer, "Generic")
        XCTAssertEqual(part.footprint?.id, "TESTPOINT-PAD-1.5")
        XCTAssertEqual(part.footprintChoices.count, 10)
        XCTAssertEqual(Set(part.footprintChoices.map(\.footprint.id)).count, 10)
        XCTAssertEqual(Set(part.footprintChoices.map(\.package)).count, 10)
        XCTAssertTrue(part.footprintChoices.contains { $0.footprint.id == "TESTPOINT-PAD-1.5" && $0.package == part.package })
        XCTAssertEqual(PartFacts.kindName(part), "Test point")
    }

    /// Existing connectors keep their details: a barrel jack stays a barrel
    /// jack and a card slot a card slot.
    func testOtherConnectorsAreNotTestPoints() throws {
        let barrel = try TestSupport.part("PJ-002A")
        XCTAssertFalse(barrel.isTestPoint)
        guard case .connector = barrel.details else { return XCTFail("a barrel jack is a connector: \(barrel.details)") }
        XCTAssertEqual(PartFacts.kindName(barrel), "Barrel connector")
        let slots = TestSupport.catalog.allParts.filter(\.isCardSlot)
        XCTAssertFalse(slots.isEmpty)
        XCTAssertTrue(slots.allSatisfy { !$0.isTestPoint })
        XCTAssertEqual(TestSupport.catalog.allParts.filter(\.isTestPoint).map(\.name), ["GENERIC-TESTPOINT"])
    }

    /// Every footprint of the part has one pad, round, at the origin; the
    /// through-hole ones are drilled, the others are not.
    func testFootprintsAreOneRoundPadEach() throws {
        let part = try testPoint()
        for choice in part.footprintChoices {
            let footprint = choice.footprint
            XCTAssertEqual(footprint.pads.count, 1, footprint.id)
            let pad = try XCTUnwrap(footprint.pads.first, footprint.id)
            XCTAssertEqual(pad.centre.x, 0, accuracy: 1e-9, footprint.id)
            XCTAssertEqual(pad.centre.y, 0, accuracy: 1e-9, footprint.id)
            XCTAssertEqual(pad.size.width, pad.size.height, accuracy: 1e-9, footprint.id)
            XCTAssertEqual(pad.drill > 0, footprint.id.hasPrefix("TESTPOINT-THT-"), footprint.id)
        }
    }

    /// The part is placeable, draws as a circle on a stub, and the Board
    /// Preview gives it no body.
    func testPlacedTestPointLooksLikeABareBlock() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.footprintID ?? chip.part.footprint?.id, "TESTPOINT-PAD-1.5")
        XCTAssertEqual(chip.symbol, .testPoint)
        XCTAssertNotNil(chip.board)
        guard case .testPad = PartLook.of(chip).kind else { return XCTFail("a test point has no body: \(PartLook.of(chip).kind)") }
    }

    /// plan-57-B: the symbol is one left pin on a 40 x 20 body, with the
    /// same lead tip as a 1x1 header at the same place, and the pin takes a wire.
    func testSymbolIsOneLeftPinAndTakesAWire() throws {
        XCTAssertEqual(symbol(for: try testPoint()), .testPoint)
        let doc = TestSupport.newDocument()
        let tp = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 300, y: 100))
        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X01", at: CGPoint(x: 100, y: 300))
        XCTAssertEqual(tp.layout.size, CGSize(width: 40, height: ChipLayout.pitch))
        XCTAssertEqual(tp.layout.slots.count, 1)
        let slot = try XCTUnwrap(tp.layout.slots.first)
        XCTAssertEqual(slot.side, .left)
        XCTAssertEqual(slot.centerY, ChipLayout.pitch / 2)
        let headerSlot = try XCTUnwrap(header.layout.slots.first)
        XCTAssertEqual(tp.tip(of: slot).x, header.tip(of: headerSlot).x)
        XCTAssertEqual(tp.tip(of: slot).y - 100, header.tip(of: headerSlot).y - 300)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(tp, "TP")), .pin(try TestSupport.pin(other, "TP"))))
        XCTAssertEqual(doc.wires.count, 1)
    }

    /// plan-57-B: new test points are TP1, TP2, ...; a barrel jack beside
    /// them keeps the J prefix.
    func testReferencesAreTP() throws {
        let part = try testPoint()
        XCTAssertEqual(ReferenceCategory.of(part), .testPoint)
        let doc = TestSupport.newDocument()
        let first = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 300, y: 100))
        let third = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 500, y: 100))
        let jack = try TestSupport.place(doc, "PJ-002A", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual([first, second, third].map(\.reference), ["TP1", "TP2", "TP3"])
        XCTAssertTrue(jack.reference.hasPrefix("J"), jack.reference)
        XCTAssertEqual(ReferenceCategory.of(jack.part), .femaleConnector)
    }

    /// plan-57-C: the size and mounting map to the ten footprint ids and back.
    func testFamilyBuildsAndParsesAllTenIds() throws {
        let ids = Set(try testPoint().footprintChoices.map(\.footprint.id))
        var built: Set<String> = []
        for size in TestPointFamily.sizes {
            for throughHole in [false, true] {
                let id = TestPointFamily.footprintID(size: size, throughHole: throughHole)
                XCTAssertTrue(ids.contains(id), id)
                let parsed = try XCTUnwrap(TestPointFamily.parse(id), id)
                XCTAssertEqual(parsed.size, size, id)
                XCTAssertEqual(parsed.throughHole, throughHole, id)
                built.insert(id)
            }
        }
        XCTAssertEqual(built, ids)
        XCTAssertNil(TestPointFamily.parse("RES-0805"))
        XCTAssertNil(TestPointFamily.parse("TESTPOINT-PAD-9.9"))
        XCTAssertNil(TestPointFamily.parse("TESTPOINT-XYZ-1.5"))
        let part = try testPoint()
        let own = try XCTUnwrap(TestPointFamily.parse(part.footprint?.id ?? ""))
        XCTAssertEqual(own.size, TestPointFamily.defaultSize)
        XCTAssertFalse(own.throughHole)
        XCTAssertEqual(TestPointFamily.label(2.5), "2.5 mm")
    }

    /// plan-57-C: the Size menu and the Through hole toggle each write one
    /// footprint, keep the other's value, undo, and survive a file round trip.
    func testSizeAndThroughHoleWritesUndoAndSave() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        func live() throws -> PlacedChip {
            try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        }
        func current() throws -> (size: Double, throughHole: Bool) {
            let now = try live()
            return try XCTUnwrap(TestPointFamily.parse(now.footprintID ?? now.part.footprint?.id ?? ""))
        }
        func pad() throws -> Footprint.Pad {
            try XCTUnwrap(try live().footprint?.pads.first)
        }
        XCTAssertEqual(try current().size, 1.5)
        XCTAssertFalse(try current().throughHole)
        XCTAssertEqual(try pad().drill, 0)

        doc.setPartFootprint(chip.id, TestPointFamily.footprintID(size: 2.0, throughHole: try current().throughHole))
        XCTAssertEqual(try live().footprintID, "TESTPOINT-PAD-2.0")
        doc.setPartFootprint(chip.id, TestPointFamily.footprintID(size: try current().size, throughHole: true))
        XCTAssertEqual(try live().footprintID, "TESTPOINT-THT-2.0")
        doc.setPartFootprint(chip.id, TestPointFamily.footprintID(size: 3.0, throughHole: try current().throughHole))
        XCTAssertEqual(try live().footprintID, "TESTPOINT-THT-3.0")
        XCTAssertEqual(try pad().size.width, 3.0, accuracy: 1e-9)
        XCTAssertGreaterThan(try pad().drill, 0)

        let data = try SchematicFile(document: doc, fileURL: nil, catalog: TestSupport.catalog).encoded()
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.chips.first?.footprintID, "TESTPOINT-THT-3.0")

        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try live().footprintID, "TESTPOINT-THT-2.0")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try live().footprintID, "TESTPOINT-PAD-2.0")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try current().size, 1.5)
        XCTAssertFalse(try current().throughHole)
        XCTAssertEqual(try pad().drill, 0)
    }

    /// plan-57-C: only the test point swaps its footprint menu; another part
    /// with choices keeps the flat picker.
    func testOnlyTheTestPointIsATestPoint() throws {
        let led = try TestSupport.part("GENERIC-LED")
        XCTAssertFalse(led.footprintChoices.isEmpty)
        XCTAssertFalse(led.isTestPoint)
        XCTAssertTrue(PartOptionsSection.applies(to: led))
        XCTAssertTrue(PartOptionsSection.applies(to: try testPoint()))
    }

    /// Connector's drawer lists the leaf Test Point beside its groups, and
    /// the groups are the ones it had (the terminal blocks and JST headers of plan-45 and the IC sockets of
    /// plan-51 have joined them; the list had gone stale).
    func testTheDrawerListsTheLeafUnderConnector() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let connector = try XCTUnwrap(groups.first { $0.category == "Connector" })
        XCTAssertEqual(connector.functions.map(\.function), ["Test Point"])
        XCTAssertEqual(connector.subcategories.map(\.subcategory), ["Barrel", "Card slot", "Header", "IC Socket", "JST", "Terminal block", "USB"])
        let leaf = try XCTUnwrap(connector.functions.first)
        XCTAssertEqual(leaf.parts.map(\.name), ["GENERIC-TESTPOINT"])
    }
}
