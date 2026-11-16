import SwiftUI

// MARK: - The Part Wizard's guidance (plan-46-L)

/// What the wizard says about one key: what it is, where a datasheet prints
/// it, and what a typical value looks like.
struct FieldGuide: Sendable, Equatable {
    /// A plain label; nil keeps the schema's.
    var label: String? = nil
    /// One line: what the key is, in plain words.
    var help: String
    /// The datasheet section titles it usually sits under, most common first.
    var whereToFind: [String] = []
    /// The datasheet's other names for the value.
    var alsoCalled: [String] = []
    /// A typical value, shown as the field's placeholder.
    var example: String? = nil
}

/// Guidance for keys many kinds have, by bare key, at any depth: a kind's own
/// guide for a path wins over it.
enum SharedGuides {
    static let fields: [String: FieldGuide] = [
        "vcc": FieldGuide(
            label: "Supply voltage range",
            help: "The lowest and highest supply voltage the part is meant to run at.",
            whereToFind: ["Recommended Operating Conditions", "Operating Conditions"],
            alsoCalled: ["VCC", "VDD", "V+", "Supply voltage"],
            example: "2 to 6 V"),
        "inputMax": FieldGuide(
            label: "Highest input voltage",
            help: "The highest voltage an input pin takes. Leave it out when it is the same as the top of the "
                + "supply range; give it for 5 V-tolerant inputs.",
            whereToFind: ["Recommended Operating Conditions", "Absolute Maximum Ratings"],
            alsoCalled: ["VI", "VIN", "Input voltage"],
            example: "5.5"),
        "rise": FieldGuide(
            label: "Delay, output rising",
            help: "How long after an input changes the output goes high. Use the typical value at the supply "
                + "voltage you will run it at, in ns.",
            whereToFind: ["Switching Characteristics", "AC Characteristics", "Timing Characteristics"],
            alsoCalled: ["tPLH", "tpd"],
            example: "9"),
        "fall": FieldGuide(
            label: "Delay, output falling",
            help: "How long after an input changes the output goes low. Use the typical value at the supply "
                + "voltage you will run it at, in ns.",
            whereToFind: ["Switching Characteristics", "AC Characteristics", "Timing Characteristics"],
            alsoCalled: ["tPHL", "tpd"],
            example: "9"),
        "family": FieldGuide(
            label: "Logic family",
            help: "The letters after 74 in the part number: 74HC00 is HC, 74LVC1G00 is LVC. It picks which "
                + "timing entry the part uses.",
            whereToFind: ["Ordering Information", "Device Information"],
            alsoCalled: ["Technology", "Series"],
            example: "HC"),
        "note": FieldGuide(
            help: "A note for people: where a value came from, or what the datasheet leaves out. "
                + "The simulation ignores it."),
        "otherLow": FieldGuide(
            label: "Delays while the other input is low",
            help: "Only for datasheets that give different delays depending on the other input of a "
                + "2-input gate. Most do not: leave it out.",
            whereToFind: ["Switching Characteristics"]),
        "otherHigh": FieldGuide(
            label: "Delays while the other input is high",
            help: "Only for datasheets that give different delays depending on the other input of a "
                + "2-input gate. Most do not: leave it out.",
            whereToFind: ["Switching Characteristics"]),
    ]
}

enum Guidance {
    /// The guide for a key: the kind's curated guide for the path, else the
    /// shared guide for the key, else nil (the schema's label then shows
    /// alone). `path` is side-qualified and dotted: `"device.gates.op"`.
    static func guide(kind: String, path: String) -> FieldGuide? {
        if let own = PartKinds.guides[kind]?.fields[path] { return own }
        let key = path.split(separator: ".").last.map(String.init) ?? path
        return SharedGuides.fields[key]
    }

    /// Section titles that are other manufacturers' names for a section, so
    /// they cannot be checked against the converted (TI) datasheets the app
    /// bundles. Every `whereToFind` title is either a heading of those
    /// datasheets or listed here (`KindGuideTests`).
    static let alternateTitles: Set<String> = [
        "Operating Conditions", "AC Characteristics", "Timing Characteristics", "Device Information",
        "Truth Table", "Function Table", "Logic Diagram", "Pin Configuration", "Pin Description",
        "Electrical Specifications", "DC Characteristics", "Features", "Functional Block Diagram",
        "Characteristics", "Electro-Optical Characteristics", "Optical Characteristics", "Internal Circuit Diagram",
        "Circuit Diagram", "Ratings", "Specifications", "Mechanical Data", "Dimensions", "Package Dimensions",
        "Frequency Stability", "Output Characteristics", "Part Numbering", "How to Order", "Mechanical Characteristics",
        "Maximum Ratings", "Thermal Characteristics", "Timing Diagram", "Logic Symbol", "Outline Drawing",
    ]

    /// "Usually under: A (also: B, C)".
    static func whereLine(_ guide: FieldGuide) -> String? {
        guard let first = guide.whereToFind.first else { return nil }
        let rest = guide.whereToFind.dropFirst()
        return rest.isEmpty ? "Usually under: \(first)" : "Usually under: \(first) (also: \(rest.joined(separator: ", ")))"
    }
}

/// The intros of the steps every part has: one or two plain sentences each.
enum StepGuides {
    static let kind = "Pick the kind of part. It decides what the simulation needs to know about it. "
        + "If a part like yours is already in the library, start from it."
    static let identity = "The part number is how you will find the part in the drawer. "
        + "It is printed on the first page of the datasheet and in its ordering table, with the package."
    static let datasheet = "Give the address of the manufacturer's PDF. It opens beside these steps, and each step "
        + "can then find its section in it. You can go on without one."
    static let pinout = "List every pin: its number, its name as the datasheet writes it, and what it is. "
        + "Power and ground are the supply pins; input, output and inout (both ways) are signals; "
        + "nc is a pin that connects to nothing; absent is a pin position the package leaves out. "
        + "Pin numbers are as seen from the top of the part."
    static let footprint = "The footprint is the pattern of copper pads the part is soldered to. "
        + "Pick one from the library, or make one from the datasheet's drawing."
    static let review = "Check the part before you save it: its symbol, its footprint and that every pin has a pad."
}

extension EnvironmentValues {
    /// Find in datasheet for the fields of a wizard step: jumps the pane to
    /// the first of these section titles the PDF has. Nil outside the wizard.
    @Entry var datasheetFind: (([String]) -> Void)? = nil
    /// The example value of the field being drawn, for its placeholder.
    @Entry var fieldExample: String? = nil
}

/// The guidance under a field in Guided mode: what it is, where to find it
/// (with a Find button), what else it is called.
struct FieldGuideView: View {
    let guide: FieldGuide
    /// Whether the control shows the example as its placeholder already.
    var examplePlaced = false
    @Environment(\.datasheetFind) private var find

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(guide.help).font(.caption).foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)
            if let line = Guidance.whereLine(guide) {
                HStack(spacing: 4) {
                    Text(line).font(.caption).foregroundStyle(.secondary)
                    if let find {
                        Button {
                            find(guide.whereToFind)
                        } label: {
                            Image(systemName: "magnifyingglass")
                        }
                        .buttonStyle(.borderless)
                        .controlSize(.small)
                        .help("Find it in the datasheet")
                        .accessibilityLabel("Find \(guide.whereToFind[0]) in the datasheet")
                    }
                }
            }
            if !guide.alsoCalled.isEmpty {
                Text("Also called: " + guide.alsoCalled.joined(separator: ", "))
                    .font(.caption).foregroundStyle(.secondary)
            }
            if let example = guide.example, !examplePlaced {
                Text("For example: \(example)").font(.caption).foregroundStyle(.secondary)
            }
        }
    }
}

extension FieldType {
    /// Whether the control is a single text field, which can show an example
    /// as its placeholder.
    var takesPlaceholder: Bool {
        switch self {
        case .text, .note, .number, .integer, .suggested: true
        default: false
        }
    }
}
