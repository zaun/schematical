import Foundation
import XCTest
@testable import schematical

/// plan-37-G: register parts are logic chips. The sidebar names the kind
/// "Register", the drawer lists them under Logic → Registers in a "Register"
/// group, and the drawer's family filter narrows them as it does gates.
@MainActor
final class RegisterTests: XCTestCase {
    func testKindName() throws {
        XCTAssertEqual(PartFacts.kindName(PART_REGISTER), "Register")
        XCTAssertEqual(PartFacts.kindName(try TestSupport.part("SN74LS173AN")), "Register")
    }

    /// SN74LS173AN: an LS logic chip with 4 stored bits.
    func testFacts() throws {
        let part = try TestSupport.part("SN74LS173AN")
        XCTAssertEqual(part.kind, PART_REGISTER)
        XCTAssertTrue(part.isLogic)
        XCTAssertEqual(part.details, .logic(family: "LS", units: 4))
    }

    /// The parts of a Logic → Registers → Register group, under a filter.
    private func registerGroup(_ filter: DrawerFilter) -> [String] {
        let logic = TestSupport.catalog.groups(matching: filter).first { $0.category == "Logic" }
        let registers = logic?.subcategories.first { $0.subcategory == "Registers" }
        let group = registers?.functions.first { $0.function == "Register" }
        return group?.parts.map(\.name) ?? []
    }

    func testDrawerGroup() {
        let names = registerGroup(DrawerFilter())
        XCTAssertTrue(names.contains("SN74LS173AN"))
        XCTAssertTrue(names.contains("CD4076BE"))
    }

    /// The family filter applies to register parts (before plan-37-G it
    /// skipped every part that was not `isLogic`).
    func testFamilyFilter() {
        var filter = DrawerFilter()
        filter.category = "Logic"
        filter.family = "HC"
        XCTAssertFalse(registerGroup(filter).contains("SN74LS173AN"))
        filter.family = "LS"
        XCTAssertTrue(registerGroup(filter).contains("SN74LS173AN"))
        XCTAssertTrue(TestSupport.catalog.families(in: "Logic").contains("LCX"))
        XCTAssertTrue(TestSupport.catalog.logicCounts(in: "Logic").contains(22))
    }
}
