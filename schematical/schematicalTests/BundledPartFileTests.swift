import Foundation
import XCTest
@testable import schematical

/// plan-68-AC: the bundled part files read for the Part Editor's copy path
/// and the kind names of the backend's kinds.
@MainActor
final class BundledPartFileTests: XCTestCase {
    func testACopyHoldsOnlyThatPartWithANewUUID() throws {
        let copy = try XCTUnwrap(BundledPartFile.copy(name: "SN74HC00N", kind: "gate", uuid: "U-1"))
        guard case let .object(root) = copy, case let .array(devices)? = root["devices"],
              case let .object(device)? = devices.first, case let .array(parts)? = device["parts"] else {
            return XCTFail("not a part file: \(copy)")
        }
        XCTAssertEqual(root["uuid"], .string("U-1"))
        XCTAssertEqual(root["kind"], .string("gate"))
        XCTAssertEqual(devices.count, 1)
        XCTAssertEqual(parts.count, 1)
        XCTAssertNil(BundledPartFile.copy(name: "NO-SUCH-PART", kind: "gate"))
        XCTAssertNil(BundledPartFile.copy(name: "SN74HC00N", kind: "no-such-kind"))
        XCTAssertNotNil(BundledPartFile.copy(of: try TestSupport.part("SN74HC00N")))
    }

    func testPartNamesAreInFileOrder() {
        let names = BundledPartFile.partNames(kind: "gate")
        XCTAssertTrue(names.contains("SN74HC00N"))
        XCTAssertEqual(Set(names).count, names.count)
        XCTAssertEqual(BundledPartFile.partNames(kind: "no-such-kind"), [])
        XCTAssertNil(BundledPartFile.file(kind: "no-such-kind"))
    }

    func testExamplesByVariantGiveTwoNamesPerVariant() {
        let examples = BundledPartFile.examplesByVariant(kind: "serial")
        XCTAssertFalse(examples.isEmpty)
        XCTAssertTrue(examples.values.allSatisfy { (1...2).contains($0.count) })
        XCTAssertEqual(BundledPartFile.examplesByVariant(kind: "gate"), [:], "gate has no variants")
    }

    /// A kind the backend may add before this switch knows it has no part file name.
    func testAnUnknownKindHasNoName() {
        XCTAssertNil(PartKinds.name(of: PartKind(rawValue: 9999)))
        XCTAssertEqual(PartKinds.name(of: PART_OPAMP), "opamp")
    }
}
