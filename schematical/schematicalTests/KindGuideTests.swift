import Foundation
import XCTest
@testable import schematical

/// plan-46-L, M: the drift test for the Part Wizard's curated guides. A guide
/// names keys of a kind's schema; when the schema changes, a guide that no
/// longer fits fails here, naming the kind and the key.
@MainActor
final class KindGuideTests: XCTestCase {
    /// The kinds with a curated guide, sorted.
    static var curated: [String] { PartKinds.guides.keys.sorted() }

    /// A kind's variants (nil for a kind without them).
    static func variants(of kind: String) -> [String?] {
        let values = PartKinds.schemas[kind]?.variantValues ?? []
        return values.isEmpty ? [nil] : values.map { Optional($0) }
    }

    /// What a guide's steps miss or repeat for a variant, as messages naming
    /// the kind and the key; empty when every key is in exactly one step.
    static func coverageProblems(kind: String, variant: String?, schema: KindSchema, guide: KindGuide) -> [String] {
        let wanted = schema.topLevelFields(variant: variant).map(\.key.qualified)
        let got = guide.steps(variant).flatMap(\.keys)
        let label = "\(kind)\(variant.map { " (\($0))" } ?? "")"
        var problems: [String] = []
        for key in wanted where !got.contains(key) {
            problems.append("\(label): no step of the guide has \(key)")
        }
        for key in Set(got) where got.filter({ $0 == key }).count > 1 {
            problems.append("\(label): \(key) is in more than one step")
        }
        for key in got where !wanted.contains(key) {
            problems.append("\(label): the guide names \(key), which the kind does not have")
        }
        return problems.sorted()
    }

    /// (1) Every curated guide, for every variant, covers the kind's
    /// top-level keys exactly once.
    func testCuratedGuidesCoverEveryKeyOnce() throws {
        XCTAssertTrue(Self.curated.contains("gate"))
        for kind in Self.curated {
            let schema = try XCTUnwrap(PartKinds.schemas[kind], "\(kind): a guide for a kind with no schema")
            let guide = try XCTUnwrap(PartKinds.guides[kind])
            for variant in Self.variants(of: kind) {
                let problems = Self.coverageProblems(kind: kind, variant: variant, schema: schema, guide: guide)
                XCTAssertEqual(problems, [], problems.joined(separator: "\n"))
                let steps = guide.steps(variant)
                XCTAssertEqual(Set(steps.map(\.id)).count, steps.count, "\(kind): duplicate step ids")
                XCTAssertTrue(steps.allSatisfy { $0.id.hasPrefix("kind.") }, "\(kind): step ids start kind.")
                for step in steps {
                    XCTAssertFalse(step.keys.isEmpty, "\(kind): step \(step.id) has no keys")
                    XCTAssertNotNil(step.intro, "\(kind): step \(step.id) has no intro")
                }
            }
        }
    }

    /// The field a dotted path names, walking objects, lists, maps and the
    /// alternatives of a one-of; nil when the schema has none.
    static func field(at path: String, in schema: KindSchema) -> Field? {
        var parts = path.split(separator: ".").map(String.init)
        guard parts.count >= 2 else { return nil }
        let side = parts.removeFirst()
        var fields: [Field]
        switch side {
        case "device": fields = schema.device
        case "part": fields = schema.part
        default: return nil
        }
        var found: Field?
        for key in parts {
            guard let next = fields.first(where: { $0.key == key }) else { return nil }
            found = next
            fields = children(of: next.type)
        }
        return found
    }

    static func children(of type: FieldType) -> [Field] {
        switch type {
        case let .object(fields), let .list(fields), let .map(_, fields): fields
        case let .valueMap(_, inner), let .listOf(inner): children(of: inner)
        case let .oneOf(types): types.flatMap(children(of:))
        default: []
        }
    }

    /// (2) Every path a guide explains is a key of the kind's schema.
    func testGuidePathsAreSchemaKeys() throws {
        for kind in Self.curated {
            let schema = try XCTUnwrap(PartKinds.schemas[kind])
            for path in try XCTUnwrap(PartKinds.guides[kind]).fields.keys.sorted() {
                XCTAssertNotNil(Self.field(at: path, in: schema), "\(kind): the guide explains \(path), which is no key")
            }
        }
        // The walk itself.
        let gate = try XCTUnwrap(PartKinds.schemas["gate"])
        XCTAssertEqual(Self.field(at: "device.gates.op", in: gate)?.key, "op")
        XCTAssertEqual(Self.field(at: "device.timing.rise", in: gate)?.key, "rise")
        XCTAssertEqual(Self.field(at: "device.timing.otherLow.fall", in: gate)?.key, "fall")
        XCTAssertNil(Self.field(at: "device.gates.nonsense", in: gate))
        XCTAssertNil(Self.field(at: "gates", in: gate))
    }

    /// (3) Every shared guide is for a key some schema has.
    func testSharedGuidesAreKeysOfSomeSchema() {
        let every = PartKinds.schemas.values.reduce(into: Set<String>()) { $0.formUnion($1.keys) }
        for key in SharedGuides.fields.keys.sorted() {
            XCTAssertTrue(every.contains(key), "the shared guide \(key) is no key of any kind")
        }
        XCTAssertTrue(Set(["vcc", "inputMax", "rise", "fall", "note", "family", "otherLow", "otherHigh"])
            .isSubset(of: Set(SharedGuides.fields.keys)))
    }

    /// (4) Every curated step can find its section in a datasheet.
    func testCuratedStepsHaveSearchTerms() throws {
        for kind in Self.curated {
            let guide = try XCTUnwrap(PartKinds.guides[kind])
            for variant in Self.variants(of: kind) {
                for step in guide.steps(variant) where step.page != .variant {
                    XCTAssertFalse(step.searchTerms.isEmpty, "\(kind): step \(step.id) has no search terms")
                }
            }
        }
    }

    /// The `##` headings of the converted gate datasheets, lower case.
    static let gateHeadings: Set<String> = {
        let folder = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .appendingPathComponent("schematical/Datasheets/gate-and")
        var headings = Set<String>()
        let files = (try? FileManager.default.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil)) ?? []
        for file in files where file.pathExtension == "md" {
            guard let text = try? String(contentsOf: file, encoding: .utf8) else { continue }
            for line in text.split(separator: "\n") where line.hasPrefix("## ") {
                headings.insert(line.dropFirst(3).trimmingCharacters(in: .whitespaces).lowercased())
            }
        }
        return headings
    }()

    /// (5) The gate guide's pointers are real section titles: each guide the
    /// gate wizard shows leads with a heading of the bundled gate datasheets,
    /// and every other title is one too or a declared alternate name.
    func testGatePointersAreDatasheetHeadings() throws {
        XCTAssertGreaterThanOrEqual(Self.gateHeadings.count, 10)
        XCTAssertTrue(Self.gateHeadings.contains("switching characteristics"))
        let guide = try XCTUnwrap(PartKinds.guides["gate"])
        var guides: [(String, FieldGuide)] = guide.fields.sorted { $0.key < $1.key }.map { ($0.key, $0.value) }
        // The shared guides a gate's keys use (its timing entries, its supply).
        for key in ["vcc", "inputMax", "rise", "fall", "family", "otherLow", "otherHigh"] {
            guides.append(("shared \(key)", try XCTUnwrap(SharedGuides.fields[key])))
        }
        var titles: [(String, [String])] = guides.map { ($0.0, $0.1.whereToFind) }
        for step in guide.steps(nil) { titles.append(("step \(step.id)", step.searchTerms)) }
        func isHeading(_ title: String) -> Bool { Self.gateHeadings.contains(title.lowercased()) }
        for (name, list) in titles {
            if let first = list.first {
                XCTAssertTrue(isHeading(first), "gate \(name): “\(first)” is no heading of the gate datasheets")
            }
            for title in list {
                XCTAssertTrue(isHeading(title) || Guidance.alternateTitles.contains(title),
                              "gate \(name): “\(title)” is neither a datasheet heading nor a declared alternate")
            }
        }
    }

    /// Every required field at `path` and below it, with its dotted path.
    static func requiredFields(_ fields: [Field], path: String) -> [(path: String, field: Field)] {
        fields.filter(\.required).flatMap { field -> [(String, Field)] in
            let own = "\(path).\(field.key)"
            return [(own, field)] + requiredFields(children(of: field.type), path: own)
        }
    }

    /// (6) Every model of every kind with models has a blurb.
    func testEveryModelHasABlurb() {
        var count = 0
        for kind in PartKinds.all.sorted() {
            guard let schema = PartKinds.schemas[kind], schema.variantKey != nil else { continue }
            for value in schema.variantValues {
                let blurb = PartKinds.variantBlurb(kind: kind, value: value) ?? ""
                XCTAssertFalse(blurb.isEmpty, "\(kind): the model \(value) has no blurb")
                XCTAssertTrue(blurb.hasSuffix("."), "\(kind) \(value): a blurb is a sentence")
                count += 1
            }
            // And no blurb is for a model the kind does not have.
            XCTAssertEqual(Set(PartKinds.variantBlurbs[kind]?.keys.map { $0 } ?? []), Set(schema.variantValues), kind)
        }
        XCTAssertEqual(count, 72)
        XCTAssertEqual(PartKinds.variantBlurbs.count, 17)
    }

    /// (7) Every required key of a curated kind, nested ones included, has a
    /// guide that says where a datasheet prints it.
    func testRequiredFieldsOfCuratedKindsSayWhereToFindThem() throws {
        XCTAssertEqual(Self.curated, ["7seg", "buffer", "decoder", "dff", "diode", "gate", "header", "jkff", "latch",
                                      "led", "mux", "oscillator", "switch"])
        var missing: [String] = []
        var checked = 0
        for kind in Self.curated {
            let schema = try XCTUnwrap(PartKinds.schemas[kind])
            let required = Self.requiredFields(schema.device, path: "device")
                + Self.requiredFields(schema.part, path: "part")
            for (path, _) in required {
                checked += 1
                guard let guide = Guidance.guide(kind: kind, path: path) else {
                    missing.append("\(kind): \(path) has no guide")
                    continue
                }
                if guide.whereToFind.isEmpty { missing.append("\(kind): the guide for \(path) does not say where to find it") }
                if guide.help.isEmpty { missing.append("\(kind): the guide for \(path) has no help") }
            }
        }
        XCTAssertEqual(missing, [], missing.joined(separator: "\n"))
        XCTAssertGreaterThan(checked, 100)
        print("checked \(checked) required fields of \(Self.curated.count) curated kinds")
    }

    /// Every top-level key of a curated kind has a guide with a plain label.
    func testEveryTopLevelKeyOfACuratedKindHasAGuide() throws {
        var missing: [String] = []
        for kind in Self.curated {
            let schema = try XCTUnwrap(PartKinds.schemas[kind])
            for entry in schema.topLevelFields(variant: nil) {
                let guide = Guidance.guide(kind: kind, path: entry.key.qualified)
                if guide == nil { missing.append("\(kind): \(entry.key.qualified)") }
            }
        }
        XCTAssertEqual(missing, [], missing.joined(separator: "\n"))
    }

    /// The mechanical layout of the kinds with models (plan-46-M § 3).
    func testLayoutByRule() throws {
        let latch = WizardPlan.steps(for: {
            var draft = PartDraft(newOf: "latch")
            draft.kindDevice["model"] = .string("transparent")
            return draft
        }())
        XCTAssertEqual(latch.filter { $0.id.hasPrefix("kind.") }.map(\.title),
                       ["Which chip does yours behave like?", "Supply voltage", "Pins it uses", "Active levels",
                        "Logic family and timing", "Options"])
        var zener = PartDraft(newOf: "diode")
        zener.kindDevice["type"] = .string("zener")
        let steps = WizardPlan.steps(for: zener)
        // A Zener adds its Zener voltage and power rating to the ratings every diode has.
        XCTAssertEqual(steps.first { $0.id == "kind.ratings" }?.keys, ["part.io", "part.pd", "part.vr", "part.vz"])
        zener.kindDevice["type"] = .string("rectifier")
        XCTAssertEqual(WizardPlan.steps(for: zener).first { $0.id == "kind.ratings" }?.keys, ["part.io", "part.vr"])
        XCTAssertTrue(GuideLayout.isPinsOnly(.pins))
        XCTAssertTrue(GuideLayout.isPinsOnly(.object([Field(key: "a", label: "A", type: .pin)])))
        XCTAssertFalse(GuideLayout.isPinsOnly(.object([Field(key: "a", label: "A", type: .pin),
                                                       Field(key: "b", label: "B", type: .bool)])))
        XCTAssertTrue(GuideLayout.isLevel("enableActive"))
        XCTAssertTrue(GuideLayout.isLevel("clearActiveHigh"))
        XCTAssertTrue(GuideLayout.isLevel("kInverted"))
        XCTAssertFalse(GuideLayout.isLevel("units"))
    }

    func testGuidanceLookup() {
        // The kind's own guide for a path wins.
        XCTAssertEqual(Guidance.guide(kind: "gate", path: "device.gates.op")?.label, "What the gate does")
        // A shared guide answers for its key at any depth, in any kind.
        XCTAssertEqual(Guidance.guide(kind: "gate", path: "part.vcc")?.label, "Supply voltage range")
        XCTAssertEqual(Guidance.guide(kind: "gate", path: "device.timing.rise")?.label, "Delay, output rising")
        XCTAssertEqual(Guidance.guide(kind: "sram", path: "part.vcc")?.label, "Supply voltage range")
        XCTAssertNil(Guidance.guide(kind: "gate", path: "device.gates.nonsense"))
        XCTAssertEqual(Guidance.whereLine(SharedGuides.fields["vcc"]!),
                       "Usually under: Recommended Operating Conditions (also: Operating Conditions)")
        XCTAssertNil(Guidance.whereLine(SharedGuides.fields["note"]!))
        XCTAssertTrue(FieldType.number(unit: "ns").takesPlaceholder)
        XCTAssertFalse(FieldType.voltRange.takesPlaceholder)
    }

    /// The gate's screens, as the wizard lists them.
    func testGateGuideSteps() throws {
        let steps = WizardPlan.steps(for: PartDraft(newOf: "gate"))
        XCTAssertEqual(steps.map(\.title), ["What are you making?", "Part number", "Datasheet", "Pins",
                                            "Supply voltage", "Gates", "Logic family and timing",
                                            "Footprint", "Review"])
        XCTAssertEqual(steps.map(\.id), ["kind", "identity", "datasheet", "pinout", "kind.supply", "kind.gates",
                                         "kind.timing", "footprint", "review"])
        XCTAssertTrue(steps.allSatisfy { $0.intro != nil }, "every step of a gate has an intro")
        XCTAssertEqual(steps.first { $0.id == "kind.gates" }?.keys,
                       ["device.gates", "device.output", "device.input", "device.units"])
    }

    /// A key added to a curated kind's schema with no step fails (1), naming
    /// the kind and the key: what plan-46-N's extendability probe shows.
    func testAMissingKeyIsNamed() throws {
        let gate = try XCTUnwrap(PartKinds.schemas["gate"])
        let grown = KindSchema(kind: "gate", device: gate.device + [Field(key: "probe", label: "Probe", type: .bool)],
                               part: gate.part)
        XCTAssertEqual(Self.coverageProblems(kind: "gate", variant: nil, schema: grown, guide: GateGuide.guide),
                       ["gate: no step of the guide has device.probe"])
        XCTAssertEqual(Self.coverageProblems(kind: "gate", variant: nil, schema: gate, guide: GateGuide.guide), [])
    }
}
