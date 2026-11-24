import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-46-J Phase 1: a footprint made from numbers. For each parametric
/// style, a draft read from a bundled entry writes the same entry back, and
/// the backend expands it to exactly the bundled footprint's pads.
@MainActor
final class ParametricFootprintDraftTests: XCTestCase {
    /// `footprints.json`'s entries, in file order.
    static let entries: [JSONValue] = {
        let url = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .appendingPathComponent("schematical/Parts/footprints.json")
        guard let data = try? Data(contentsOf: url),
              case let .object(root)? = try? JSONDecoder().decode(JSONValue.self, from: data),
              case let .array(items)? = root["footprints"] else { return [] }
        return items
    }()

    static func string(_ entry: JSONValue, _ key: String) -> String? {
        guard case let .object(keys) = entry, case let .string(text)? = keys[key] else { return nil }
        return text
    }

    /// SOIC-14 for `dual`; the first entry of each other style.
    static func sample(_ style: String) throws -> JSONValue {
        if style == "dual" {
            return try XCTUnwrap(entries.first { string($0, "id") == "SOIC-14" })
        }
        return try XCTUnwrap(entries.first { string($0, "style") == style }, "no \(style) footprint")
    }

    func testEachStyleReadsAndWritesItsEntry() throws {
        // 2201 = 1269 parametric + 932 explicit (the explicit count of CustomFootprintDraftTests); it was 1823 (1210 + 613) before plan-54 and 2198 (1266 + 932) before the plan-56 LCD glasses
        // before the card-slot footprints of plan-54-H to K (SOCK-, DIMM-, EDGE-) and the explicit ones since.
        XCTAssertEqual(Self.entries.count, 2201)
        for style in ["dual", "quad", "grid", "header"] {
            let entry = try Self.sample(style)
            let draft = try XCTUnwrap(ParametricFootprintDraft(json: entry), style)
            XCTAssertEqual(draft.style.rawValue, style)
            XCTAssertEqual(draft.encoded(), entry, style)
        }
        // An explicit entry is not parametric.
        let explicit = try XCTUnwrap(Self.entries.first { Self.string($0, "style") == "explicit" })
        XCTAssertNil(ParametricFootprintDraft(json: explicit))
    }

    /// Every bundled parametric entry round-trips: 1082 of them.
    func testEveryBundledParametricEntryRoundTrips() {
        var count = 0
        for entry in Self.entries where Self.string(entry, "style") != "explicit" {
            guard let draft = ParametricFootprintDraft(json: entry) else {
                XCTFail("\(Self.string(entry, "id") ?? "?") did not read")
                continue
            }
            if draft.encoded() != entry { XCTFail("\(draft.id) did not write back the same") }
            count += 1
        }
        XCTAssertEqual(count, 1269)
    }

    /// A generic part with the footprint's pins, naming an inline copy of it.
    private func draft(with footprint: ParametricFootprintDraft, pinout: [PinDraft],
                       style: PinoutDraft.Style) -> PartDraft {
        var draft = PartDraft(newOf: "generic")
        draft.name = "FP-\(UUID().uuidString.prefix(8))"
        draft.package = "TEST"
        draft.pinouts[0].pins = pinout
        draft.pinouts[0].style = style
        var custom = footprint
        custom.id = CustomFootprintDraft.id(forPart: draft.uuid)
        draft.inlineFootprint = .parametric(custom)
        return draft
    }

    private func assertSamePads(_ a: Footprint, _ b: Footprint, _ label: String) {
        XCTAssertEqual(a.pads.count, b.pads.count, label)
        for (x, y) in zip(a.pads, b.pads) {
            XCTAssertEqual(x.designator, y.designator, label)
            XCTAssertEqual(x.centre.x, y.centre.x, accuracy: 1e-9, "\(label) pad \(x.designator ?? "-") x")
            XCTAssertEqual(x.centre.y, y.centre.y, accuracy: 1e-9, "\(label) pad \(x.designator ?? "-") y")
            XCTAssertEqual(x.size.width, y.size.width, accuracy: 1e-9, label)
            XCTAssertEqual(x.size.height, y.size.height, accuracy: 1e-9, label)
            XCTAssertEqual(x.shape, y.shape, label)
            XCTAssertEqual(x.drill, y.drill, accuracy: 1e-9, label)
            XCTAssertEqual(x.radius, y.radius, accuracy: 1e-9, label)
            XCTAssertEqual(x.chamfer, y.chamfer, accuracy: 1e-9, label)
            XCTAssertEqual(x.corners, y.corners, label)
        }
        XCTAssertEqual(a.body.width, b.body.width, accuracy: 1e-9, label)
        XCTAssertEqual(a.body.height, b.body.height, accuracy: 1e-9, label)
    }

    /// The backend expands each style's inline copy to the bundled pads.
    func testEachStyleExpandsToTheBundledFootprint() throws {
        for style in ["dual", "quad", "grid", "header"] {
            let entry = try Self.sample(style)
            let parametric = try XCTUnwrap(ParametricFootprintDraft(json: entry))
            let bundled = try XCTUnwrap(PartValidator.shared.footprints.first { $0.id == parametric.id }, style)
            let pins: [PinDraft]
            let pinoutStyle: PinoutDraft.Style
            if style == "grid" {
                pins = parametric.balls.map { PinDraft(designator: $0, name: "P\($0)", role: "inout") }
                pinoutStyle = .ball
            } else {
                pins = (1...parametric.pins).map { PinDraft(designator: String($0), name: "P\($0)", role: "inout") }
                pinoutStyle = .numbered
            }
            let draft = draft(with: parametric, pinout: pins, style: pinoutStyle)
            XCTAssertEqual(draft.extraKeys, [], style)
            // The draft reads its own inline footprint back.
            guard case let .parametric(read)? = try PartDraft(fileText: draft.fileText()).inlineFootprint else {
                XCTFail("\(style): the inline footprint did not read back")
                continue
            }
            XCTAssertEqual(read.style.rawValue, style)
            XCTAssertNil(draft.customFootprint, "\(style): not an explicit footprint")
            switch PartValidator.shared.validate(draft, source: "\(style).schpart") {
            case let .success(part):
                let made = try XCTUnwrap(part.footprint, style)
                XCTAssertEqual(made.id, CustomFootprintDraft.id(forPart: draft.uuid))
                assertSamePads(made, bundled, "\(style) \(bundled.id)")
                print("\(style): \(bundled.id), \(made.pads.count) pads match")
            case let .failure(failure):
                XCTFail("\(style): \(failure.message)")
            }
        }
    }

    /// The behavioral outcome: SOIC-14 from the numbers on its drawing.
    func testASOIC14FromItsNumbers() throws {
        var footprint = ParametricFootprintDraft.starter(style: .dual, id: "X", pins: 14)
        XCTAssertEqual(footprint.pitch, 1.27)
        footprint.pitch = 1.27
        footprint.span = 5.4
        footprint.padWidth = 1.55
        footprint.padHeight = 0.6
        footprint.bodyWidth = 3.9
        footprint.bodyHeight = 8.65
        let pins = (1...14).map { PinDraft(designator: String($0), name: "P\($0)", role: "inout") }
        let draft = draft(with: footprint, pinout: pins, style: .numbered)
        let bundled = try XCTUnwrap(PartValidator.shared.footprints.first { $0.id == "SOIC-14" })
        switch PartValidator.shared.validate(draft) {
        case let .success(part): assertSamePads(try XCTUnwrap(part.footprint), bundled, "SOIC-14 from numbers")
        case let .failure(failure): XCTFail(failure.message)
        }
    }

    /// Each style's starter is a footprint the backend takes, for the
    /// pinout's own pins.
    func testStartersValidate() throws {
        for (style, count) in [(ParametricFootprintDraft.Style.dual, 8), (.dual, 5), (.quad, 16), (.quad, 14),
                               (.header, 6)] {
            let footprint = ParametricFootprintDraft.starter(style: style, id: "X", pins: count)
            let pins = (1...count).map { PinDraft(designator: String($0), name: "P\($0)", role: "inout") }
            let draft = draft(with: footprint, pinout: pins, style: .numbered)
            switch PartValidator.shared.validate(draft) {
            case let .success(part): XCTAssertEqual(part.footprint?.pads.count, count, "\(style) \(count)")
            case let .failure(failure): XCTFail("\(style) \(count): \(failure.message)")
            }
        }
        // A ball pinout's grid: its balls are the pinout's designators.
        let balls = ["A1", "A2", "B1", "B2", "C1", "C2"]
        let grid = ParametricFootprintDraft.starter(style: .grid, id: "X", pins: balls.count, balls: balls)
        XCTAssertEqual(grid.rows, 3)
        XCTAssertEqual(grid.columns, 2)
        XCTAssertEqual(grid.balls, balls)
        let draft = draft(with: grid, pinout: balls.map { PinDraft(designator: $0, name: "P\($0)", role: "inout") },
                          style: .ball)
        XCTAssertTrue(draft.isBallPinout)
        XCTAssertEqual(draft.pinDesignators, balls)
        switch PartValidator.shared.validate(draft) {
        case let .success(part):
            XCTAssertEqual(Set((part.footprint?.pads ?? []).compactMap(\.designator)), Set(balls))
        case let .failure(failure): XCTFail(failure.message)
        }
        // Row letters skip I, O, Q, S, X, Z.
        XCTAssertEqual(ParametricFootprintDraft.position(ofBall: "J3")?.row, 8)
        XCTAssertNil(ParametricFootprintDraft.position(ofBall: "I3"))
        XCTAssertEqual(ParametricFootprintDraft.gridSize(balls: ["A1", "H6"], count: 2).rows, 8)
        XCTAssertEqual(ParametricFootprintDraft.gridSize(balls: ["A1", "H6"], count: 2).columns, 6)
        XCTAssertEqual(ParametricFootprintDraft.gridSize(balls: [], count: 10).rows, 4)
    }

    /// Setting and clearing the inline footprint, beside the explicit one.
    func testInlineFootprintOnTheDraft() {
        var draft = PartDraft(newOf: "generic")
        XCTAssertNil(draft.inlineFootprint)
        let parametric = ParametricFootprintDraft.starter(style: .dual, id: "CUSTOM-ABCDEF12", pins: 8)
        draft.inlineFootprint = .parametric(parametric)
        XCTAssertEqual(draft.footprint, "CUSTOM-ABCDEF12")
        XCTAssertEqual(draft.footprints?.count, 1)
        XCTAssertEqual(draft.inlineFootprint, .parametric(parametric))
        XCTAssertNil(draft.customFootprint)

        let explicit = CustomFootprintDraft.starter(id: "CUSTOM-ABCDEF12", pins: 2)
        draft.inlineFootprint = .explicit(explicit)
        XCTAssertEqual(draft.customFootprint, explicit)
        XCTAssertEqual(draft.inlineFootprint, .explicit(explicit))

        draft.inlineFootprint = nil
        XCTAssertNil(draft.footprint)
        XCTAssertNil(draft.footprints)
        // A bundled footprint is not inline, and clearing leaves it.
        draft.footprint = "SOIC-14"
        XCTAssertNil(draft.inlineFootprint)
        draft.inlineFootprint = nil
        XCTAssertEqual(draft.footprint, "SOIC-14")
    }

    /// The footprint step's choices as model rules (plan-46-J Phases 2–3).
    func testFootprintStepChoices() throws {
        let model = PartEditorModel(target: PartEditorTarget(source: .new(kind: "generic")), catalog: .shared)
        Self.kept.append(model)
        model.draft.name = "FP-STEP-\(UUID().uuidString.prefix(8))"
        model.startWithNumberedPins(14)
        for index in model.draft.pinouts[0].pins.indices { model.draft.pinouts[0].pins[index].name = "P\(index + 1)" }
        XCTAssertEqual(FootprintStepPage.choice(for: model.draft), .library)

        // A library footprint.
        model.useLibraryFootprint("SOIC-14")
        XCTAssertEqual(model.draft.footprint, "SOIC-14")
        XCTAssertNil(model.draft.footprints)

        // Start from this one: a parametric library footprint becomes numbers.
        let soic = try XCTUnwrap(model.validator.footprints.first { $0.id == "SOIC-14" })
        XCTAssertEqual(model.startFromLibraryFootprint(soic), .make)
        let id = CustomFootprintDraft.id(forPart: model.draft.uuid)
        guard case let .parametric(copy)? = model.draft.inlineFootprint else { return XCTFail("not parametric") }
        XCTAssertEqual(copy.id, id)
        XCTAssertEqual(copy.span, 5.4)
        XCTAssertEqual(copy.pitch, 1.27)
        XCTAssertEqual(FootprintStepPage.choice(for: model.draft), .make)
        if case let .failure(failure) = model.validateNow() { XCTFail(failure.message) }

        // An explicit library footprint becomes a pad table.
        // (SON-6-DRY: pin 1's pad is longer than the others, so it is explicit.)
        let explicit = try XCTUnwrap(model.validator.footprints.first { $0.id == "SON-6-DRY" })
        XCTAssertNil(BundledFootprints.entry(id: explicit.id).flatMap(ParametricFootprintDraft.init(json:)))
        XCTAssertEqual(model.startFromLibraryFootprint(explicit), .pads)
        XCTAssertEqual(model.draft.customFootprint?.id, id)
        XCTAssertEqual(model.draft.customFootprint?.pads.count, explicit.pads.count)
        XCTAssertEqual(FootprintStepPage.choice(for: model.draft), .pads)

        // Choosing a library footprint again removes the custom one.
        model.useLibraryFootprint("PDIP-14")
        XCTAssertNil(model.draft.footprints)
        XCTAssertEqual(model.draft.footprint, "PDIP-14")

        // Make one: a starter of each style for the pinout's pins.
        model.startParametricFootprint(.dual)
        guard case let .parametric(dual)? = model.draft.inlineFootprint else { return XCTFail("not parametric") }
        XCTAssertEqual(dual.pins, 14)
        XCTAssertEqual(dual.style, .dual)
        if case let .failure(failure) = model.validateNow() { XCTFail(failure.message) }
        model.startPadTable()
        XCTAssertEqual(model.draft.customFootprint?.pads.map(\.pin), Array(1...14))

        // A ball pinout: the grid takes the pinout's balls, the pad table too.
        let bga = PartEditorModel(target: PartEditorTarget(source: .new(kind: "generic")), catalog: .shared)
        Self.kept.append(bga)
        bga.draft.name = "FP-BGA-\(UUID().uuidString.prefix(8))"
        bga.applyPins(["A1", "A2", "B1", "B2"].map { PinDraft(designator: $0, name: "P\($0)", role: "inout") },
                      style: .ball)
        bga.startParametricFootprint(.grid)
        guard case let .parametric(grid)? = bga.draft.inlineFootprint else { return XCTFail("not parametric") }
        XCTAssertEqual(grid.balls, ["A1", "A2", "B1", "B2"])
        XCTAssertEqual(grid.rows, 2)
        XCTAssertEqual(grid.columns, 2)
        if case let .failure(failure) = bga.validateNow() { XCTFail(failure.message) }
        bga.startPadTable()
        XCTAssertEqual(bga.draft.customFootprint?.pads.map(\.ball), ["A1", "A2", "B1", "B2"])
        if case let .failure(failure) = bga.validateNow() { XCTFail(failure.message) }

        // Thumbnails: the drawing's bounds and pin 1.
        XCTAssertEqual(FootprintThumbnail.pin1Index(of: soic), soic.pads.firstIndex { $0.designator == "1" })
        let bounds = FootprintThumbnail.bounds(of: soic)
        XCTAssertEqual(bounds.width, 5.4 + 1.55, accuracy: 1e-9)
        XCTAssertEqual(bounds.height, 8.65, accuracy: 1e-9)
        XCTAssertEqual(FootprintDiagram.dimensions(.dual), ["Pitch", "Row spacing", "Pad length", "Pad width"])
        XCTAssertEqual(FootprintStepPage.guidance.count, 4)
    }

    private static var kept: [PartEditorModel] = []

    func testRankingPutsThePackagesFootprintsFirst() throws {
        let fourteen = PartValidator.shared.footprints.filter { $0.pins == 14 }
        XCTAssertGreaterThanOrEqual(fourteen.count, 21)
        func index(_ id: String, _ package: String) throws -> Int {
            try XCTUnwrap(FootprintRanking.rank(fourteen, package: package).firstIndex { $0.id == id }, id)
        }
        XCTAssertLessThan(try index("SOIC-14", "SOIC"), try index("PDIP-14", "SOIC"))
        XCTAssertLessThan(try index("PDIP-14", "PDIP"), try index("SOIC-14", "PDIP"))
        XCTAssertEqual(FootprintRanking.rank(fourteen, package: "SOIC").first?.id, "SOIC-14")
        // With no package the order is by id.
        XCTAssertEqual(FootprintRanking.rank(fourteen, package: "").map(\.id), fourteen.map(\.id).sorted())
        XCTAssertEqual(FootprintRanking.tokens("soic-14 (D)"), ["SOIC", "14", "D"])
        // Search: every word must appear.
        let found = FootprintRanking.search(fourteen, text: "tssop 14")
        XCTAssertTrue(found.contains { $0.id == "TSSOP-14" })
        XCTAssertFalse(found.contains { $0.id == "SOIC-14" })
        XCTAssertEqual(FootprintRanking.search(fourteen, text: " ").count, fourteen.count)
    }
}
