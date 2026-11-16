import Foundation

/// Clock oscillators (plan-46-M; docs/parts-format.md § `oscillator`).
enum OscillatorGuide {
    static let guide = KindGuide(
        steps: { _ in [
            GuideParts.supplyStep(
                keys: ["device.vcc", "device.inputMax", "part.vcc", "part.inputMax"],
                intro: "The supply range of the oscillator family, and of this part number if it is narrower "
                    + "(a letter in the part number often picks 1.8 V, 2.5 V or 3.3 V)."),
            WizardStep(id: "kind.frequency", title: "Frequency",
                       intro: "The range of frequencies the family is made in, and this part's own frequency.",
                       page: .kindFields, keys: ["device.minHz", "device.maxHz", "part.defaultHz"],
                       searchTerms: ["Electrical Characteristics", "Frequency Range", "Ordering Information",
                                     "Specifications"]),
            WizardStep(id: "kind.outputs", title: "Outputs",
                       intro: "The pin the clock comes out of, and the pin that switches it off if the part "
                           + "has one.",
                       page: .kindFields, keys: ["device.outputs", "part.enablePin"],
                       searchTerms: ["Pin Description", "Pin Configuration", "Pin Functions", "Pinout"]),
            WizardStep(id: "kind.family", title: "Oscillator family",
                       intro: "The name of the series the part belongs to. It is shown in the drawer.",
                       page: .kindFields, keys: ["device.family"],
                       searchTerms: ["Ordering Information", "Part Numbering", "Features"]),
        ] },
        fields: [
            "device.vcc": FieldGuide(
                label: "Supply voltage range of the family",
                help: "The lowest and highest supply voltage any part of the family runs at.",
                whereToFind: ["Electrical Characteristics", "Recommended Operating Conditions", "Specifications"],
                alsoCalled: ["VDD", "VCC", "Supply voltage"],
                example: "1.62 to 3.63 V"),
            "part.vcc": FieldGuide(
                label: "Supply voltage range of this part",
                help: "Only when this part number is made for one supply voltage of the family's range. "
                    + "Leave it out otherwise.",
                whereToFind: ["Ordering Information", "Part Numbering", "Electrical Characteristics"],
                alsoCalled: ["VDD", "Supply voltage option"],
                example: "1.71 to 1.89 V"),
            "device.minHz": FieldGuide(
                label: "Lowest frequency of the family",
                help: "In hertz: 1 MHz is 1000000.",
                whereToFind: ["Electrical Characteristics", "Frequency Range", "Specifications"],
                alsoCalled: ["Frequency range", "Output frequency"],
                example: "1000000"),
            "device.maxHz": FieldGuide(
                label: "Highest frequency of the family",
                help: "In hertz: 100 MHz is 100000000.",
                whereToFind: ["Electrical Characteristics", "Frequency Range", "Specifications"],
                alsoCalled: ["Frequency range", "Output frequency"],
                example: "100000000"),
            "part.defaultHz": FieldGuide(
                label: "This part's frequency",
                help: "The frequency of this part number, in hertz, within the family's range. It is in the "
                    + "part number or the ordering table.",
                whereToFind: ["Ordering Information", "Part Numbering", "Electrical Characteristics"],
                alsoCalled: ["Nominal frequency", "Output frequency"],
                example: "16000000"),
            "device.outputs": FieldGuide(
                label: "Output pins",
                help: "The clock output pin; for a part with two opposite outputs, the true one and then its "
                    + "complement.",
                whereToFind: ["Pin Description", "Pin Configuration", "Pin Functions"],
                alsoCalled: ["OUT", "Output", "CLK"]),
            "part.enablePin": FieldGuide(
                label: "Enable pin number",
                help: "The number of the pin that stops the clock while it is low (output enable or standby). "
                    + "Leave it out if the part has none or the pin is not connected.",
                whereToFind: ["Pin Description", "Pin Configuration", "Pin Functions"],
                alsoCalled: ["OE", "E/D", "ST", "Standby", "Tri-state"],
                example: "1"),
            "device.family": FieldGuide(
                label: "Family name",
                help: "The series name, as the datasheet's title gives it. It is not a logic family.",
                whereToFind: ["Ordering Information", "Part Numbering", "Features"],
                alsoCalled: ["Series"],
                example: "ECS-2520MV"),
        ])
}
