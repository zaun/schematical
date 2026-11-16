import Foundation

/// D flip-flops (plan-46-M; docs/parts-format.md § `dff`).
enum DffGuide {
    /// The polarity and edge keys `dff` and `jkff` share.
    static let controlFields: [String: FieldGuide] = [
        "device.clockEdge": FieldGuide(
            label: "Clock edge",
            help: "Whether the flip-flop takes its input when the clock goes up (rising) or down (falling). "
                + "In the function table an up arrow is rising, a down arrow falling.",
            whereToFind: GuideParts.functionTitles,
            alsoCalled: ["Positive-edge-triggered", "Negative-edge-triggered"],
            example: "rising"),
        "device.clearActiveHigh": FieldGuide(
            label: "Clear is active high",
            help: "Yes when a high level on CLR clears the flip-flop. Most 74-series parts clear on a low "
                + "level (the pin is drawn with a bar): answer No.",
            whereToFind: GuideParts.functionTitles,
            alsoCalled: ["CLR", "Reset", "R"]),
        "device.presetActiveHigh": FieldGuide(
            label: "Preset is active high",
            help: "Yes when a high level on PRE sets the flip-flop. Most 74-series parts set on a low level "
                + "(the pin is drawn with a bar): answer No.",
            whereToFind: GuideParts.functionTitles,
            alsoCalled: ["PRE", "Set", "S"]),
    ]

    static let guide = KindGuide(
        steps: { _ in [
            GuideParts.supplyStep(keys: ["part.vcc"]),
            WizardStep(id: "kind.flipflops", title: "Flip-flops",
                       intro: "One entry per flip-flop in the package, naming its pins; then how its clock, "
                           + "clear and preset behave. A 74HC74 has two flip-flops.",
                       page: .kindFields,
                       keys: ["device.units", "device.clockEdge", "device.clearActiveHigh", "device.presetActiveHigh",
                              "device.output", "device.outputEnable"],
                       searchTerms: GuideParts.functionTitles),
            GuideParts.timingStep(keys: ["part.family", "device.timing"]),
        ] },
        fields: GuideParts.timingFields(extra: controlFields.merging([
            "device.units": FieldGuide(
                label: "Flip-flops",
                help: "Add one entry for each flip-flop in the package. Flip-flops may share a pin, such as a "
                    + "common clock or clear.",
                whereToFind: GuideParts.pinTitles,
                alsoCalled: ["Channels", "Bits"]),
            "device.units.d": FieldGuide(label: "Data input (D)", help: "The pin whose level is stored at the clock edge.",
                                         whereToFind: GuideParts.pinTitles, alsoCalled: ["D", "1D"]),
            "device.units.clk": FieldGuide(label: "Clock", help: "The pin whose edge stores the data.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["CLK", "CP", "C"]),
            "device.units.clr": FieldGuide(label: "Clear", help: "The pin that forces Q low. Leave it out if the part has none.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["CLR", "R", "MR"]),
            "device.units.pre": FieldGuide(label: "Preset", help: "The pin that forces Q high. Leave it out if the part has none.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["PRE", "S", "SD"]),
            "device.units.q": FieldGuide(label: "Output (Q)", help: "The pin that shows the stored level.",
                                         whereToFind: GuideParts.pinTitles, alsoCalled: ["Q", "1Q"]),
            "device.units.qn": FieldGuide(label: "Inverted output", help: "The pin that shows the opposite of Q. Leave it out if the part has none.",
                                          whereToFind: GuideParts.pinTitles, alsoCalled: ["Q with a bar", "QN", "/Q"]),
            "device.output": FieldGuide(
                label: "Output type",
                help: "Push-pull drives high and low and is what most parts have: leave this out. "
                    + "Open-drain only pulls low and needs a pull-up resistor.",
                whereToFind: ["Description", "Features"],
                alsoCalled: ["Open-drain", "Open-collector"]),
            "device.outputEnable": FieldGuide(
                label: "Output enable pin",
                help: "A pin that turns the outputs off (high impedance) while it is high, on parts with "
                    + "3-state outputs such as the 74HC374. Leave it out if the part has none.",
                whereToFind: GuideParts.pinTitles,
                alsoCalled: ["OE", "OC", "Output control"]),
            "device.timing.clearWins": FieldGuide(
                label: "Clear wins over preset",
                help: "What happens when clear and preset are both asserted: Yes if Q goes low, No if both "
                    + "outputs go high (the 74HC74). The function table's row with both asserted shows it.",
                whereToFind: GuideParts.functionTitles),
        ]) { _, new in new }))
}
