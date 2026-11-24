import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The pld kind (GAL16V8, ATF22V10, ATF750C): facts only, no simulation. The
/// shipped parts list under Logic › CPLD, read their facts, number as an IC
/// and keep Simulation › Run off while one is placed.
@MainActor
final class PldTests: XCTestCase {
    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    func testAShippedPartReadsItsFacts() throws {
        let part = try XCTUnwrap(TestSupport.catalog.part(named: "ATF22V10C-10JU"))
        XCTAssertEqual(part.kind, PART_PLD)
        XCTAssertFalse(part.isSimulated)
        XCTAssertEqual(part.pins.count, 28)
        XCTAssertEqual(part.drawer, ["Logic", "CPLD"])
        XCTAssertEqual(part.function, "22V10")
        guard case let .pld(macrocells, tpdPS) = part.details else { return XCTFail("not a pld: \(part.details)") }
        XCTAssertEqual(macrocells, 10)
        XCTAssertEqual(tpdPS, 10_000)
        XCTAssertEqual(PartFacts.kindName(part.kind), "Programmable logic")
        XCTAssertEqual(PartFacts.duration(tpdPS), "10 ns")
        XCTAssertEqual(ReferenceCategory.of(part), .integratedCircuit)
        XCTAssertEqual(PartKinds.name(of: part.kind), "pld")
    }

    func testTheSpeedGradeAndSupplyDifferByPart() throws {
        let commercial = try XCTUnwrap(TestSupport.catalog.part(named: "ATF22V10C-5JX"))
        guard case let .pld(_, tpdPS) = commercial.details else { return XCTFail("not a pld") }
        XCTAssertEqual(tpdPS, 5_000)
        let military = try XCTUnwrap(TestSupport.catalog.part(named: "ATF750C-15GM/883"))
        XCTAssertTrue(military.flags.military)
        XCTAssertTrue(military.flags.ceramic)
    }

    func testPlacedItKeepsRunOff() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "ATF16V8B-15PU", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(SimulationAvailability.unsimulated(doc.chips).map(\.id), [chip.id])
        XCTAssertEqual(SimulationAvailability.reason(doc.chips), "U1 (ATF16V8B-15PU) is not simulated")
        XCTAssertFalse(SimulationAvailability.canStart(doc))
    }

    func testEveryShippedPldIsInTheCpldDrawer() {
        let plds = TestSupport.allParts.filter { $0.kind == PART_PLD }
        // The drawer hides the two military ceramic parts until the flag filter is on.
        XCTAssertEqual(plds.count, 23)
        XCTAssertTrue(plds.allSatisfy { $0.drawer == ["Logic", "CPLD"] })
    }
}
