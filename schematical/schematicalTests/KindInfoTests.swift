import Foundation
import XCTest
@testable import schematical

/// plan-46-D Phase 1: every kind has a `KindInfo` (plain name, blurb, group,
/// example parts), and the examples are real bundled parts.
@MainActor
final class KindInfoTests: XCTestCase {
    func testEveryKindHasAnEntry() {
        XCTAssertEqual(Set(PartKinds.info.keys), Set(PartKinds.all))
        XCTAssertEqual(PartKinds.info.count, 39)
        for (kind, info) in PartKinds.info {
            XCTAssertEqual(info.kind, kind)
            XCTAssertFalse(info.name.isEmpty, kind)
            XCTAssertFalse(info.blurb.isEmpty, kind)
            XCTAssertEqual(PartDraft.displayName(of: kind), info.name)
        }
        // Names are distinct: two kinds must not read the same in the picker.
        XCTAssertEqual(Set(PartKinds.info.values.map(\.name)).count, PartKinds.info.count)
    }

    func testExamplesAreBundledPartsOfTheKind() {
        for info in PartKinds.info.values {
            if info.kind == "generic" {
                XCTAssertEqual(info.examples, [], "generic ships no parts")
                continue
            }
            XCTAssertTrue((1...3).contains(info.examples.count), "\(info.kind): \(info.examples.count) examples")
            for example in info.examples {
                guard let part = PartCatalogStore.shared.part(named: example) else {
                    XCTFail("\(info.kind): no bundled part named \(example)")
                    continue
                }
                XCTAssertEqual(PartKinds.name(of: part.kind), info.kind, "\(example) is not a \(info.kind)")
            }
        }
    }

    func testUnsimulatedKinds() {
        let unsimulated = PartKinds.info.values.filter { !$0.simulated }.map(\.kind).sorted()
        XCTAssertEqual(unsimulated, ["generic", "mcu"])
    }

    func testEveryGroupHasAKindAndEveryKindIsListedOnce() {
        var listed: [String] = []
        for group in KindGroup.allCases {
            let kinds = PartKinds.kinds(in: group)
            XCTAssertFalse(kinds.isEmpty, "no kind in \(group.rawValue)")
            listed += kinds.map(\.kind)
        }
        XCTAssertEqual(listed.sorted(), PartKinds.all.sorted())
        XCTAssertEqual(PartKinds.kinds(in: .gates).map(\.kind), ["gate", "buffer", "dff", "jkff", "latch"])
        XCTAssertEqual(PartKinds.kinds(in: .other).map(\.kind), ["generic"])
    }
}
