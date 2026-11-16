import Foundation

// MARK: - The Part Wizard's steps (plan-46-D § 3.2)

/// Which page a wizard step shows.
enum WizardPage: Hashable, Sendable {
    case kind, identity, datasheet, pinout, variant, supply, kindFields, footprint, review
}

/// One screen of the Part Wizard.
struct WizardStep: Identifiable, Hashable, Sendable {
    /// "identity", "kind.essentials", "kind.timing", …
    let id: String
    let title: String
    /// One or two sentences under the title; nil for a derived kind step.
    var intro: String? = nil
    let page: WizardPage
    /// The top-level kind keys this step edits (`kindFields`, `supply` and
    /// `variant` pages), side-qualified: `"device.gates"`, `"part.family"`.
    var keys: [String] = []
    /// The step may be skipped (it shows Skip).
    var optional = false
    /// Section titles Find in datasheet looks for (plan-46-G).
    var searchTerms: [String] = []

    // The fixed steps every part has.
    static let kind = WizardStep(id: "kind", title: "What are you making?", intro: StepGuides.kind, page: .kind)
    static let identity = WizardStep(id: "identity", title: "Part number", intro: StepGuides.identity,
                                     page: .identity, searchTerms: StepSearchTerms.identity)
    static let datasheet = WizardStep(id: "datasheet", title: "Datasheet", intro: StepGuides.datasheet,
                                      page: .datasheet)
    static let pinout = WizardStep(id: "pinout", title: "Pins", intro: StepGuides.pinout, page: .pinout,
                                   searchTerms: StepSearchTerms.pinout)
    static let footprint = WizardStep(id: "footprint", title: "Footprint", intro: StepGuides.footprint,
                                      page: .footprint, searchTerms: StepSearchTerms.footprint)
    static let review = WizardStep(id: "review", title: "Review", intro: StepGuides.review, page: .review)
}

/// A kind's hand-written wizard screens (plan-46-L, M): its kind steps for a
/// variant (nil for a kind without variants, or before one is chosen). A
/// kind with no guide gets steps derived from its schema.
struct KindGuide: Sendable {
    let steps: @Sendable (String?) -> [WizardStep]
    /// Guidance for the kind's keys, by side-qualified dotted path
    /// (`"device.gates.op"`, `"part.family"`); plan-46-L.
    var fields: [String: FieldGuide] = [:]
}

/// A top-level kind key with its side: `device.gates`, `part.family`.
struct KindKey: Hashable, Sendable {
    enum Side: String, Sendable { case device, part }

    let side: Side
    let key: String

    var qualified: String { "\(side.rawValue).\(key)" }

    init(_ side: Side, _ key: String) {
        self.side = side
        self.key = key
    }

    /// Reads `"device.gates"`; nil for anything else.
    init?(qualified: String) {
        guard let dot = qualified.firstIndex(of: "."), let side = Side(rawValue: String(qualified[..<dot])) else {
            return nil
        }
        let key = String(qualified[qualified.index(after: dot)...])
        guard !key.isEmpty else { return nil }
        self.init(side, key)
    }
}

extension KindSchema {
    /// The top-level fields of a variant with their sides, device before
    /// part, in schema order.
    func topLevelFields(variant: String?) -> [(key: KindKey, field: Field)] {
        deviceFields(variant: variant).map { (KindKey(.device, $0.key), $0) }
            + partFields(variant: variant).map { (KindKey(.part, $0.key), $0) }
    }

    /// The values of the variant key (its choice or suggestion list), in the
    /// schema's order; empty for a kind without variants.
    var variantValues: [String] {
        guard let variantKey, let field = device.first(where: { $0.key == variantKey }) else { return [] }
        switch field.type {
        case let .choice(values), let .suggested(values): return values
        default: return []
        }
    }

    /// The field a side-qualified top-level key names, for a variant.
    func field(_ qualified: String, variant: String?) -> Field? {
        guard let key = KindKey(qualified: qualified) else { return nil }
        let fields = key.side == .device ? deviceFields(variant: variant) : partFields(variant: variant)
        return fields.first { $0.key == key.key }
    }
}

extension FieldType {
    /// A type with keys or entries of its own: it gets a screen to itself in
    /// a derived wizard (plan-46-D § 3.2).
    var isComposite: Bool {
        switch self {
        case .object, .list, .map, .valueMap, .listOf: true
        default: false
        }
    }
}

/// Turns a draft into its ordered wizard steps.
enum WizardPlan {
    /// The form's own Supply keys (`PartEditorForm.supplyKeys`).
    static let supplyKeys: Set<String> = ["vcc", "inputMax"]

    /// Every step of the wizard for a draft: kind, part number, datasheet,
    /// pins, the kind's steps, footprint, review.
    static func steps(for draft: PartDraft) -> [WizardStep] {
        var steps: [WizardStep] = [.kind, .identity, .datasheet, .pinout]
        steps += kindSteps(kind: draft.kind, variant: draft.variant)
        if hasFootprint(draft) { steps.append(.footprint) }
        steps.append(.review)
        return steps
    }

    /// A VCC/GND supply has no footprint (`a supply has no footprint`,
    /// docs/parts-format.md § The "footprint" part key); a pull part is a
    /// resistor and has one.
    static func hasFootprint(_ draft: PartDraft) -> Bool {
        draft.kind != "supply" || draft.kindDevice["pull"] == .bool(true)
    }

    /// The kind's own steps: its curated guide's when it has one, otherwise
    /// derived from its schema.
    static func kindSteps(kind: String, variant: String?) -> [WizardStep] {
        if let guide = PartKinds.guides[kind] { return guide.steps(variant) }
        guard let schema = PartKinds.schemas[kind] else { return [] }
        return derivedSteps(schema: schema, variant: variant)
    }

    /// Steps for a kind with no guide: the model picker when the kind has
    /// variants, the supply range, the required keys, one optional screen per
    /// remaining key that holds a group or a list, and the rest together.
    /// Every top-level key of the variant lands in exactly one step.
    static func derivedSteps(schema: KindSchema, variant: String?) -> [WizardStep] {
        var steps: [WizardStep] = []
        var rest = schema.topLevelFields(variant: variant)

        if let variantKey = schema.variantKey,
           let index = rest.firstIndex(where: { $0.key == KindKey(.device, variantKey) }) {
            steps.append(variantStep(key: rest[index].key))
            rest.remove(at: index)
        }

        let supply = rest.filter { supplyKeys.contains($0.key.key) }
        if !supply.isEmpty {
            steps.append(WizardStep(id: "kind.supply", title: "Supply voltage", page: .supply,
                                    keys: supply.map(\.key.qualified), searchTerms: StepSearchTerms.supply))
        }
        rest.removeAll { supplyKeys.contains($0.key.key) }

        let essentials = rest.filter(\.field.required)
        if !essentials.isEmpty {
            steps.append(WizardStep(id: "kind.essentials", title: "Essentials", page: .kindFields,
                                    keys: essentials.map(\.key.qualified)))
        }
        rest.removeAll(where: \.field.required)

        var ids = Set(steps.map(\.id))
        for entry in rest where entry.field.type.isComposite {
            // "kind.timing"; a part key whose name a device key's step took
            // is "kind.part.<key>".
            var id = "kind.\(entry.key.key)"
            if ids.contains(id) { id = "kind.\(entry.key.qualified)" }
            ids.insert(id)
            steps.append(WizardStep(id: id, title: entry.field.label, page: .kindFields,
                                    keys: [entry.key.qualified], optional: true))
        }
        rest.removeAll { $0.field.type.isComposite }

        if !rest.isEmpty {
            steps.append(WizardStep(id: "kind.other", title: "Other settings", page: .kindFields,
                                    keys: rest.map(\.key.qualified), optional: true))
        }
        return steps
    }

    /// The "Which chip does yours behave like?" step of a kind with variants.
    static func variantStep(key: KindKey) -> WizardStep {
        WizardStep(id: "kind.variant", title: "Which chip does yours behave like?", page: .variant,
                   keys: [key.qualified])
    }
}

extension PartKinds {
    /// The curated guides, by kind (plan-46-L, M). A kind without one gets
    /// derived steps.
    static let guides: [String: KindGuide] = [
        "gate": GateGuide.guide,
    ]

    /// Kind → model → one plain sentence (plan-46-M writes them).
    static let variantBlurbs: [String: [String: String]] = [:]
}
