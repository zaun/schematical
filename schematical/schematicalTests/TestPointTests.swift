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

    /// The part is placeable, draws as a plain block for now, and the Board
    /// Preview gives it no body.
    func testPlacedTestPointLooksLikeABareBlock() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.footprintID ?? chip.part.footprint?.id, "TESTPOINT-PAD-1.5")
        XCTAssertNotNil(chip.board)
        guard case .testPad = PartLook.of(chip).kind else { return XCTFail("a test point has no body: \(PartLook.of(chip).kind)") }
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

    /// Connector's drawer lists the leaf Test Point beside its groups, and
    /// the groups are the ones it had.
    func testTheDrawerListsTheLeafUnderConnector() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let connector = try XCTUnwrap(groups.first { $0.category == "Connector" })
        XCTAssertEqual(connector.functions.map(\.function), ["Test Point"])
        XCTAssertEqual(connector.subcategories.map(\.subcategory), ["Barrel", "Card slot", "Header", "USB"])
        let leaf = try XCTUnwrap(connector.functions.first)
        XCTAssertEqual(leaf.parts.map(\.name), ["GENERIC-TESTPOINT"])
    }
}
