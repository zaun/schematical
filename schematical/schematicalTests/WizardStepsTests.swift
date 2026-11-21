import Foundation
import XCTest
@testable import schematical

/// plan-46-D Phase 2: the wizard's steps. For every kind and every variant,
/// the kind steps cover each top-level kind key exactly once, whether the
/// steps are curated or derived from the schema.
@MainActor
final class WizardStepsTests: XCTestCase {
    /// Every kind with each of its variants (nil for a kind without them).
    static var combinations: [(kind: String, variant: String?)] {
        PartKinds.all.sorted().flatMap { kind -> [(String, String?)] in
            guard let schema = PartKinds.schemas[kind] else { return [] }
            let variants = schema.variantValues
            return variants.isEmpty ? [(kind, nil)] : variants.map { (kind, $0) }
        }
    }

    /// The coverage invariant (plan-46-D § 3.2).
    func testEveryKindKeyIsInExactlyOneStep() throws {
        let combinations = Self.combinations
        XCTAssertEqual(combinations.count, 86)
        XCTAssertEqual(combinations.filter { $0.variant == nil }.count, 21)
        XCTAssertEqual(combinations.filter { $0.variant != nil }.count, 65)
        for (kind, variant) in combinations {
            let schema = try XCTUnwrap(PartKinds.schemas[kind])
            let wanted = schema.topLevelFields(variant: variant).map(\.key.qualified).sorted()
            let steps = WizardPlan.kindSteps(kind: kind, variant: variant)
            let got = steps.flatMap(\.keys).sorted()
            XCTAssertEqual(got, wanted, "\(kind) \(variant ?? "-")")
            XCTAssertEqual(Set(steps.map(\.id)).count, steps.count, "\(kind) \(variant ?? "-"): duplicate step ids")
            for step in steps {
                XCTAssertFalse(step.keys.isEmpty, "\(kind) \(variant ?? "-"): \(step.id) has no keys")
                for key in step.keys {
                    XCTAssertNotNil(schema.field(key, variant: variant), "\(kind): \(key)")
                }
            }
            // The variant key is in the variant step, which comes first.
            if let variantKey = schema.variantKey {
                XCTAssertEqual(steps.first?.page, .variant, kind)
                XCTAssertEqual(steps.first?.keys, ["device.\(variantKey)"], kind)
            } else {
                XCTAssertFalse(steps.contains { $0.page == .variant }, kind)
            }
        }
    }

    /// The same invariant with no variant chosen yet (a new part of a kind
    /// with variants): every key of the kind is still reachable.
    func testEveryKeyIsCoveredBeforeAVariantIsChosen() throws {
        for kind in PartKinds.all {
            let schema = try XCTUnwrap(PartKinds.schemas[kind])
            let wanted = schema.topLevelFields(variant: nil).map(\.key.qualified).sorted()
            XCTAssertEqual(WizardPlan.kindSteps(kind: kind, variant: nil).flatMap(\.keys).sorted(), wanted, kind)
        }
    }

    /// The steps derived from a schema, for a kind with no guide: the gate's
    /// schema is the example (the wizard itself shows the gate's curated
    /// guide, plan-46-L).
    func testDerivedGateSteps() throws {
        let steps = WizardPlan.derivedSteps(schema: try XCTUnwrap(PartKinds.schemas["gate"]), variant: nil)
        XCTAssertEqual(steps.map(\.id), ["kind.supply", "kind.essentials", "kind.other"])
        XCTAssertEqual(steps.map(\.page), [.supply, .kindFields, .kindFields])
        let byID = Dictionary(uniqueKeysWithValues: steps.map { ($0.id, $0) })
        XCTAssertEqual(byID["kind.supply"]?.keys, ["part.vcc", "part.inputMax"])
        XCTAssertEqual(byID["kind.essentials"]?.keys, ["device.gates", "device.timing", "part.family"])
        XCTAssertEqual(byID["kind.other"]?.keys, ["device.output", "device.units"])
        XCTAssertEqual(steps.filter(\.optional).map(\.id), ["kind.other"])
        XCTAssertEqual(steps.map(\.title), ["Supply voltage", "Essentials", "Other settings"])
    }

    /// The whole list for an uncurated kind, and for the gate (curated).
    func testStepLists() {
        XCTAssertEqual(WizardPlan.steps(for: PartDraft(newOf: "sram")).map(\.page).prefix(4),
                       [.kind, .identity, .datasheet, .pinout])
        XCTAssertEqual(WizardPlan.steps(for: PartDraft(newOf: "sram")).map(\.page).suffix(2), [.footprint, .review])
        XCTAssertEqual(WizardPlan.steps(for: PartDraft(newOf: "gate")).map(\.id),
                       ["kind", "identity", "datasheet", "pinout", "kind.supply", "kind.gates", "kind.timing",
                        "footprint", "review"])
        XCTAssertEqual(WizardPlan.steps(for: PartDraft(newOf: "gate")).map(\.title).prefix(4),
                       ["What are you making?", "Part number", "Datasheet", "Pins"])
    }

    /// An optional key that holds a group or a list gets a screen of its own,
    /// titled with its label.
    func testACompositeOptionalKeyHasItsOwnStep() throws {
        var draft = PartDraft(newOf: "counter")
        draft.kindDevice["model"] = .string("ripple")
        let steps = WizardPlan.steps(for: draft)
        let sections = try XCTUnwrap(steps.first { $0.id == "kind.sections" })
        XCTAssertEqual(sections.keys, ["device.sections"])
        XCTAssertEqual(sections.title, "Sections")
        XCTAssertTrue(sections.optional)
        XCTAssertEqual(sections.page, .kindFields)
        // Another model has no such key, so no such step.
        draft.kindDevice["model"] = .string("sync")
        XCTAssertNil(WizardPlan.steps(for: draft).first { $0.id == "kind.sections" })
    }

    func testASupplyHasNoFootprintStepUnlessItIsAPull() {
        var draft = PartDraft(newOf: "supply")
        XCTAssertFalse(WizardPlan.steps(for: draft).contains { $0.page == .footprint })
        XCTAssertEqual(WizardPlan.steps(for: draft).last?.page, .review)
        draft.kindDevice["pull"] = .bool(true)
        XCTAssertTrue(WizardPlan.steps(for: draft).contains { $0.page == .footprint })
        draft.kindDevice["pull"] = .bool(false)
        XCTAssertFalse(WizardPlan.steps(for: draft).contains { $0.page == .footprint })
    }

    /// A serial model is small once chosen (plan-46-D § 2): the i8251's keys.
    func testSerialI8251Keys() {
        let keys = WizardPlan.kindSteps(kind: "serial", variant: "i8251").flatMap(\.keys).sorted()
        XCTAssertEqual(keys, ["device.bus", "device.clock", "device.grades", "device.model", "device.serial",
                              "part.ctsA", "part.fiveBitStops", "part.grade", "part.inputMax", "part.intPin",
                              "part.resetClearsData", "part.vcc"])
        XCTAssertEqual(keys.count, 12)
        // All models together: the wall the wizard avoids.
        XCTAssertEqual(WizardPlan.kindSteps(kind: "serial", variant: nil).flatMap(\.keys).count, 126)
    }

    func testKindKey() {
        XCTAssertEqual(KindKey(qualified: "device.gates"), KindKey(.device, "gates"))
        XCTAssertEqual(KindKey(qualified: "part.family")?.qualified, "part.family")
        XCTAssertNil(KindKey(qualified: "gates"))
        XCTAssertNil(KindKey(qualified: "file.gates"))
        XCTAssertNil(KindKey(qualified: "device."))
    }
}
