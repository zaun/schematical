import Foundation

/// Pieces the curated guides share (plan-46-M): the steps and field guides
/// that read the same for every logic kind.
enum GuideParts {
    static let switchingTitles = ["Switching Characteristics", "AC Characteristics", "Timing Characteristics",
                                  "Timing Requirements"]
    static let functionTitles = ["Function Table", "Function Tables", "Truth Table", "Logic Diagram", "Description"]
    static let pinTitles = ["Pin Functions", "Pin Configuration and Functions", "Pin Description", "Pinout"]

    /// "Supply voltage": the keys named `vcc` and `inputMax`, on either side.
    static func supplyStep(keys: [String], intro: String? = nil) -> WizardStep {
        WizardStep(id: "kind.supply", title: "Supply voltage",
                   intro: intro ?? "The voltage range the chip runs at. The schematic warns when a part is wired "
                       + "to a supply outside its range.",
                   page: .supply, keys: keys, searchTerms: StepSearchTerms.supply)
    }

    /// "Logic family and timing": the part's family and the device's delays.
    static func timingStep(keys: [String]) -> WizardStep {
        WizardStep(id: "kind.timing", title: "Logic family and timing",
                   intro: "How fast the chip is. The logic family is the letters after 74 in the part number "
                       + "(74HC74 is HC); the delays are in the datasheet's switching table.",
                   page: .kindFields, keys: keys, searchTerms: switchingTitles)
    }

    /// The guides of a logic kind's `timing` table and its entries' own keys.
    static func timingFields(extra: [String: FieldGuide] = [:]) -> [String: FieldGuide] {
        let fields: [String: FieldGuide] = [
            "device.timing": FieldGuide(
                label: "Delays, per logic family",
                help: "One entry per logic family the part comes in, each with its delays and its supply range. "
                    + "Name the entry with the family's letters (HC), then give the delays.",
                whereToFind: switchingTitles,
                alsoCalled: ["Propagation delay", "tpd", "tPLH", "tPHL"]),
            "device.timing.note": FieldGuide(
                label: "Where the delays came from",
                help: "Say which row of the table and which supply voltage you used, so the numbers can be "
                    + "checked later.",
                whereToFind: switchingTitles,
                example: "tpd CLK to Q, typical at 4.5 V"),
        ]
        return fields.overridden(by: extra)
    }

    /// A step for a variant kind's keys, omitted (nil) when it has none.
    static func step(id: String, title: String, intro: String, keys: [String], searchTerms: [String],
                     optional: Bool = false) -> WizardStep? {
        keys.isEmpty ? nil : WizardStep(id: id, title: title, intro: intro, page: .kindFields, keys: keys,
                                        optional: optional, searchTerms: searchTerms)
    }
}

extension Dictionary where Key == String, Value == FieldGuide {
    /// These guides with `more` added; a key in both takes `more`'s guide.
    func overridden(by more: [String: FieldGuide]) -> [String: FieldGuide] {
        var fields = self
        for (key, guide) in more { fields[key] = guide }
        return fields
    }
}
