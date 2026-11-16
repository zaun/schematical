import Foundation

/// The gate kind's curated wizard screens (plan-46-L): the worked example of
/// a guide. Three steps cover the gate's ten top-level keys; the field guides
/// say what each one is and where a logic datasheet prints it.
enum GateGuide {
    static let guide = KindGuide(
        steps: { _ in [
            WizardStep(id: "kind.supply", title: "Supply voltage",
                       intro: "The voltage range the chip runs at. A 74HC part runs from 2 V to 6 V; a 74LVC part "
                           + "from 1.65 V to 3.6 V. The schematic warns when a part is wired to a supply outside "
                           + "its range.",
                       page: .supply, keys: ["part.vcc", "part.inputMax"],
                       searchTerms: StepSearchTerms.supply),
            WizardStep(id: "kind.gates", title: "Gates",
                       intro: "One entry per gate in the package: what it does and which pins are its inputs "
                           + "and its output. A 74HC00 has four NAND gates, so it has four entries.",
                       page: .kindFields, keys: ["device.gates", "device.output", "device.units"],
                       searchTerms: ["Function Tables", "Function Table", "Truth Table", "Logic Diagram",
                                     "Description"]),
            WizardStep(id: "kind.timing", title: "Logic family and timing",
                       intro: "How fast the gate is. The logic family is the letters after 74 in the part "
                           + "number; the delays are in the datasheet's switching table.",
                       page: .kindFields, keys: ["part.family", "device.timing"],
                       searchTerms: ["Switching Characteristics", "AC Characteristics", "Timing Characteristics"]),
        ] },
        fields: [
            "device.gates": FieldGuide(
                label: "Gates",
                help: "Add one entry for each gate in the package.",
                whereToFind: ["Function Tables", "Description", "Logic Diagram"],
                alsoCalled: ["Channels", "Units"]),
            "device.gates.op": FieldGuide(
                label: "What the gate does",
                help: "and, nand, or, nor, xor, xnor, not (an inverter) or buf (a plain buffer). "
                    + "The function table shows it: a NAND's output is low only when every input is high.",
                whereToFind: ["Function Tables", "Function Table", "Truth Table", "Description"],
                alsoCalled: ["Function", "Logic function"]),
            "device.gates.inputs": FieldGuide(
                label: "Input pins",
                help: "The gate's input pins, by name. An inverter or a buffer has one; other gates have "
                    + "two or more.",
                whereToFind: ["Pin Functions", "Pinout", "Pin Configuration"],
                alsoCalled: ["A, B", "1A, 1B"]),
            "device.gates.output": FieldGuide(
                label: "Output pin",
                help: "The gate's output pin, by name.",
                whereToFind: ["Pin Functions", "Pinout", "Pin Configuration"],
                alsoCalled: ["Y", "1Y"]),
            "device.output": FieldGuide(
                label: "Output type",
                help: "Push-pull drives high and low and is what most gates have: leave this out. "
                    + "Open-drain only pulls low and needs a pull-up resistor (a 74HC03, a 7405).",
                whereToFind: ["Description", "Features"],
                alsoCalled: ["Open-drain", "Open-collector", "Totem-pole"]),
            "device.units": FieldGuide(
                label: "Gates counted in the drawer",
                help: "The number the drawer shows beside the part. Leave it out: it is the number of gates.",
                example: "4"),
            "device.timing": FieldGuide(
                label: "Delays, per logic family",
                help: "One entry per logic family the part comes in, each with its delays and its supply "
                    + "range. Name the entry with the family's letters (HC), then give the delays.",
                whereToFind: ["Switching Characteristics", "AC Characteristics", "Timing Characteristics"],
                alsoCalled: ["Propagation delay", "tpd", "tPLH", "tPHL"]),
        ])
}
