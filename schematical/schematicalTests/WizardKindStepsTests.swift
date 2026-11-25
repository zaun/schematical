import Foundation
import XCTest
@testable import schematical

/// plan-46-I Phase 1: the model picker's examples, the clean-up when the
/// model changes, and which `bus` keys a bus style shows.
@MainActor
final class WizardKindStepsTests: XCTestCase {
    private static var kept: [PartEditorModel] = []

    func testSerialModelsHaveExampleParts() throws {
        let examples = BundledPartFile.examplesByVariant(kind: "serial")
        let models = try XCTUnwrap(PartKinds.schemas["serial"]).variantValues
        XCTAssertEqual(models.count, 11)
        XCTAssertEqual(Set(examples.keys), Set(models))
        for (model, names) in examples {
            XCTAssertTrue((1...2).contains(names.count), "\(model): \(names)")
            for name in names {
                let part = try XCTUnwrap(PartCatalogStore.shared.part(named: name), name)
                XCTAssertEqual(part.kind, PART_SERIAL, name)
            }
        }
        // A kind without variants has none.
        XCTAssertEqual(BundledPartFile.examplesByVariant(kind: "gate"), [:])
        // Every model of every variant kind has at least one bundled part to show.
        for kind in PartKinds.all {
            guard let schema = PartKinds.schemas[kind], schema.variantKey != nil else { continue }
            let found = BundledPartFile.examplesByVariant(kind: kind)
            XCTAssertEqual(Set(found.keys), Set(schema.variantValues), kind)
        }
    }

    func testKeysOutsideTheNewModelAreReportedAndRemoved() {
        var draft = PartDraft(newOf: "counter")
        draft.kindDevice["model"] = .string("ripple")
        draft.kindDevice["sections"] = .array([.object([:])])
        XCTAssertEqual(draft.keysOutside(variant: "ripple"), [])
        XCTAssertEqual(draft.keysOutside(variant: "sync"), ["device.sections"])
        let cleaned = draft.removing(keys: draft.keysOutside(variant: "sync"))
        XCTAssertNil(cleaned.kindDevice["sections"])
        XCTAssertEqual(cleaned.kindDevice["model"], .string("ripple"), "removing does not choose")
        XCTAssertEqual(cleaned.keysOutside(variant: "sync"), [])
        // A part key that only some models have (a Zener's vz).
        var diode = PartDraft(newOf: "diode")
        diode.kindDevice["type"] = .string("zener")
        diode.kindPart["vz"] = .number(5.1)
        XCTAssertEqual(diode.keysOutside(variant: "rectifier"), ["part.vz"])
        // A kind without variants loses nothing.
        XCTAssertEqual(PartDraft(newOf: "gate").keysOutside(variant: "x"), [])
        XCTAssertEqual(VariantStepPage.lossMessage(["device.sections", "part.vz"]),
                       "These settings do not exist for it and will be removed: sections, vz.")
    }

    func testChoosingAModelRemovesTheOtherModelsKeys() {
        let model = PartEditorModel(target: PartEditorTarget(source: .new(kind: "counter")), catalog: .shared)
        Self.kept.append(model)
        model.chooseVariant("ripple")
        XCTAssertEqual(model.draft.variant, "ripple")
        model.draft.kindDevice["sections"] = .array([.object([:])])
        XCTAssertEqual(model.keysLost(choosingVariant: "sync"), ["device.sections"])
        XCTAssertEqual(model.keysLost(choosingVariant: "ripple"), [])
        model.chooseVariant("sync")
        XCTAssertEqual(model.draft.variant, "sync")
        XCTAssertNil(model.draft.kindDevice["sections"])
        XCTAssertEqual(model.draft.extraKeys, [])
    }

    private func busKeys(_ kind: String) throws -> [Field] {
        let schema = try XCTUnwrap(PartKinds.schemas[kind])
        let bus = try XCTUnwrap(schema.device.first { $0.key == "bus" })
        guard case let .object(fields) = bus.type else {
            XCTFail("\(kind): bus is not an object")
            return []
        }
        return fields
    }

    func testTheBusConditionTableCoversTheBusKeys() throws {
        for kind in ["converter", "rtc", "leddriver", "lcddriver"] {
            let keys = try busKeys(kind).map(\.key)
            XCTAssertEqual(keys.count, 25, kind)
            let table = try XCTUnwrap(PartKinds.conditions[kind])
            let conditioned = Set(table.keys.map { String($0.dropFirst("bus.".count)) })
            XCTAssertTrue(table.keys.allSatisfy { $0.hasPrefix("bus.") })
            XCTAssertTrue(table.values.allSatisfy { $0.sibling == "style" })
            XCTAssertEqual(conditioned.count, 18, kind)
            XCTAssertTrue(conditioned.isSubset(of: Set(keys)), "\(kind): \(conditioned.subtracting(keys))")
            XCTAssertFalse(conditioned.contains("style"))
            let unconditioned = Set(keys).subtracting(conditioned).subtracting(["style"])
            XCTAssertEqual(unconditioned, ["ale", "i2c", "selectPin", "selectSampled", "spi", "spiLevel"], kind)
        }
        XCTAssertEqual(Set(PartKinds.conditions.keys), ["converter", "rtc", "leddriver", "lcddriver"])
    }

    func testAnSPIBusShowsTheSPIKeys() throws {
        let fields = try busKeys("converter")
        let table = try XCTUnwrap(PartKinds.conditions["converter"])
        func layout(_ style: String?) -> GuidedFieldLayout {
            GuidedFieldLayout.make(fields: fields, path: "bus", conditions: table,
                                   object: style.map { ["style": .string($0)] } ?? [:])
        }
        let spi = layout("spi")
        XCTAssertEqual(spi.leading, ["style"])
        XCTAssertEqual(Set(spi.shown), ["cs", "sclk", "mosi", "miso", "mode", "csActive"])
        XCTAssertEqual(Set(spi.hidden), ["address", "data", "rd", "wr", "rw", "byteSelect", "clock", "oe",
                                         "clockOut", "scl", "sda", "table"])
        XCTAssertEqual(Set(spi.more), ["ale", "i2c", "selectPin", "selectSampled", "spi", "spiLevel"])
        XCTAssertEqual(spi.leading.count + spi.shown.count + spi.hidden.count + spi.more.count, 25)

        XCTAssertEqual(Set(layout("i2c").shown), ["scl", "sda", "address", "table"])
        XCTAssertEqual(Set(layout("parallel").shown), ["data", "cs", "rd", "wr", "rw", "address", "byteSelect"])
        XCTAssertEqual(Set(layout("clocked").shown), ["clock", "data", "oe", "clockOut"])
        // No style yet: only the style, and the settings every style has.
        XCTAssertEqual(layout(nil).shown, [])
        XCTAssertEqual(layout(nil).leading, ["style"])
        XCTAssertEqual(layout(nil).hidden.count, 18)

        // An object with no conditions shows every field, with no disclosure.
        let top = try XCTUnwrap(PartKinds.schemas["converter"]).device
        let plain = GuidedFieldLayout.make(fields: top, path: "", conditions: table, object: [:])
        XCTAssertEqual(plain.shown, top.map(\.key))
        XCTAssertEqual(plain.more, [])
        XCTAssertEqual(plain.hidden, [])
    }

    func testGuidedPaths() {
        let top = GuidedFields(kind: "converter", path: "device")
        XCTAssertEqual(top.bare, "")
        let bus = top.descending("bus", label: "Bus")
        XCTAssertEqual(bus.path, "device.bus")
        XCTAssertEqual(bus.bare, "bus")
        XCTAssertEqual(bus.label, "Bus")
        XCTAssertEqual(bus.descending("spi", label: "Spi").bare, "bus.spi")
    }
}
