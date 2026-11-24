import Foundation

/// The step layout the curated kinds with models share (plan-46-M § 3): the
/// model picker, then the chosen model's keys sorted by one mechanical rule,
/// so a new key lands in a step without anyone deciding where.
enum GuideLayout {
    /// A key that only names pins: a pin, a list of pins, or a group or list
    /// whose every key is one of those.
    static func isPinsOnly(_ type: FieldType) -> Bool {
        switch type {
        case .pin, .pins:
            return true
        case let .object(fields), let .list(fields):
            return !fields.isEmpty && fields.allSatisfy { isPinsOnly($0.type) }
        case let .listOf(inner):
            return isPinsOnly(inner)
        default:
            return false
        }
    }

    static func isLevel(_ key: String) -> Bool {
        key.hasSuffix("Active") || key.hasSuffix("ActiveHigh") || key.hasSuffix("Inverted")
            || key == "invert" || key == "polarity"
    }

    static let displayKeys: Set<String> = ["glyphs", "segments", "segmentsActive", "blank", "lampTest", "rbi", "rbo", "latch", "phase", "phaseOut"]

    /// The steps for a model of a kind:
    /// 1. the model picker; 2. Supply voltage (`vcc`, `inputMax`); 3. Pins it
    /// uses (keys that only name pins); 4. Active levels (`…Active`,
    /// `…ActiveHigh`, `…Inverted`, `invert`, `polarity`); 5. Display (the
    /// 7-segment keys); 6. Logic family and timing (`family`, `timing`);
    /// 7. Options (the rest). The first rule that fits takes the key; a step
    /// with no key is left out.
    static func byRule(schema: KindSchema, variant: String?) -> [WizardStep] {
        var steps: [WizardStep] = []
        var rest = schema.topLevelFields(variant: variant)
        if let variantKey = schema.variantKey,
           let index = rest.firstIndex(where: { $0.key == KindKey(.device, variantKey) }) {
            var step = WizardPlan.variantStep(key: rest[index].key)
            step.intro = "Pick the model whose behavior matches your chip. The next steps ask only for what "
                + "that model needs."
            steps.append(step)
            rest.remove(at: index)
        }
        func take(_ fits: (KindKey, Field) -> Bool) -> [String] {
            let taken = rest.filter { fits($0.key, $0.field) }
            rest.removeAll { fits($0.key, $0.field) }
            return taken.map(\.key.qualified)
        }
        let supply = take { key, _ in WizardPlan.supplyKeys.contains(key.key) }
        let timing = take { key, _ in key.key == "family" || key.key == "timing" }
        let pins = take { _, field in isPinsOnly(field.type) }
        let levels = take { key, _ in isLevel(key.key) }
        let display = take { key, _ in displayKeys.contains(key.key) }
        let options = rest
        if !supply.isEmpty { steps.append(GuideParts.supplyStep(keys: supply)) }
        if let step = GuideParts.step(
            id: "kind.pins", title: "Pins it uses",
            intro: "Which of the part's pins do what. The names are the ones you gave on the Pins step.",
            keys: pins, searchTerms: GuideParts.pinTitles) { steps.append(step) }
        if let step = GuideParts.step(
            id: "kind.levels", title: "Active levels",
            intro: "Whether each control acts when it is high or when it is low. A pin drawn with a bar over "
                + "its name, or marked L in the function table, acts when low.",
            keys: levels, searchTerms: GuideParts.functionTitles) { steps.append(step) }
        if let step = GuideParts.step(
            id: "kind.display", title: "Display",
            intro: "What the 7-segment outputs show for each code, and the pins that test or blank the display.",
            keys: display, searchTerms: GuideParts.functionTitles) { steps.append(step) }
        if !timing.isEmpty { steps.append(GuideParts.timingStep(keys: timing)) }
        if !options.isEmpty {
            let required = options.contains { $0.field.required }
            steps.append(WizardStep(
                id: "kind.options", title: "Options",
                intro: "The rest of the model's settings: its units and how its outputs behave.",
                page: .kindFields, keys: options.map(\.key.qualified), optional: !required,
                searchTerms: GuideParts.functionTitles))
        }
        return steps
    }

    /// A guide that lays its steps out by the rule.
    static func guide(kind: String, fields: [String: FieldGuide]) -> KindGuide {
        KindGuide(steps: { variant in
            guard let schema = PartKinds.schemas[kind] else { return [] }
            return byRule(schema: schema, variant: variant)
        }, fields: fields)
    }

    // Field guides the variant kinds share, by what the key is.

    static func pins(_ label: String, _ help: String, also: [String] = []) -> FieldGuide {
        FieldGuide(label: label, help: help, whereToFind: GuideParts.pinTitles, alsoCalled: also)
    }

    static func level(_ label: String, _ help: String, also: [String] = []) -> FieldGuide {
        FieldGuide(label: label, help: help, whereToFind: GuideParts.functionTitles, alsoCalled: also,
                   example: "low")
    }

    static func function(_ label: String, _ help: String, also: [String] = [], example: String? = nil) -> FieldGuide {
        FieldGuide(label: label, help: help, whereToFind: GuideParts.functionTitles, alsoCalled: also,
                   example: example)
    }

    /// The model key, the output type and the unit list read the same for
    /// every logic kind with models.
    static func common(units: String) -> [String: FieldGuide] {
        GuideParts.timingFields(extra: [
            "device.model": FieldGuide(
                label: "Model",
                help: "Which behavior the simulation uses for the part.",
                whereToFind: ["Description", "Features", "Function Table"]),
            "device.output": FieldGuide(
                label: "Output type",
                help: "Push-pull drives high and low and is what most parts have: leave this out. "
                    + "Open-drain only pulls low and needs a pull-up resistor.",
                whereToFind: ["Description", "Features"],
                alsoCalled: ["Open-drain", "Open-collector"]),
            "device.units": FieldGuide(
                label: "Units",
                help: units,
                whereToFind: GuideParts.pinTitles + ["Logic Diagram"],
                alsoCalled: ["Channels", "Sections", "Bits"]),
        ])
    }
}

extension SharedGuides {
    /// Guides for the keys of a control (`{"pin", "active"}`) or a clock
    /// (`{"pin", "edge"}`), wherever one appears.
    static let controls: [String: FieldGuide] = [
        "pin": FieldGuide(
            label: "Pin", help: "The pin, by the name you gave it on the Pins step.",
            whereToFind: GuideParts.pinTitles),
        "active": FieldGuide(
            label: "Active level",
            help: "The level at which the pin acts: low for a pin drawn with a bar over its name.",
            whereToFind: GuideParts.functionTitles, alsoCalled: ["Active low", "Active high"], example: "low"),
        "edge": FieldGuide(
            label: "Clock edge",
            help: "Rising when the clock acts as it goes high (an up arrow in the function table), falling "
                + "when it acts as it goes low.",
            whereToFind: GuideParts.functionTitles, alsoCalled: ["Positive edge", "Negative edge"],
            example: "rising"),
    ]
}
