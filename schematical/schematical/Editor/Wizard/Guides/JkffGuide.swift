import Foundation

/// JK flip-flops (plan-46-M; docs/parts-format.md § `jkff`).
enum JkffGuide {
    static let guide = KindGuide(
        steps: { _ in [
            GuideParts.supplyStep(keys: ["part.vcc"]),
            WizardStep(id: "kind.flipflops", title: "Flip-flops",
                       intro: "One entry per flip-flop in the package, naming its pins; then how its clock, K "
                           + "input, clear and preset behave. On a clock edge a JK flip-flop sets (J), clears "
                           + "(K), holds (neither) or toggles (both).",
                       page: .kindFields,
                       keys: ["device.units", "device.clockEdge", "device.kInverted", "device.clearActiveHigh",
                              "device.presetActiveHigh"],
                       searchTerms: GuideParts.functionTitles),
            GuideParts.timingStep(keys: ["part.family", "device.timing"]),
        ] },
        fields: GuideParts.timingFields(extra: DffGuide.controlFields.merging([
            "device.units": FieldGuide(
                label: "Flip-flops",
                help: "Add one entry for each flip-flop in the package (one or two).",
                whereToFind: GuideParts.pinTitles,
                alsoCalled: ["Channels"]),
            "device.units.j": FieldGuide(label: "J input", help: "The pin that sets Q at the clock edge.",
                                         whereToFind: GuideParts.pinTitles, alsoCalled: ["J", "1J"]),
            "device.units.k": FieldGuide(label: "K input", help: "The pin that clears Q at the clock edge.",
                                         whereToFind: GuideParts.pinTitles, alsoCalled: ["K", "1K", "K with a bar"]),
            "device.units.clk": FieldGuide(label: "Clock", help: "The pin whose edge acts on J and K.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["CLK", "CP"]),
            "device.units.clr": FieldGuide(label: "Clear", help: "The pin that forces Q low.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["CLR", "R"]),
            "device.units.pre": FieldGuide(label: "Preset", help: "The pin that forces Q high. Leave it out if the part has none.",
                                           whereToFind: GuideParts.pinTitles, alsoCalled: ["PRE", "S"]),
            "device.units.q": FieldGuide(label: "Output (Q)", help: "The pin that shows the stored level.",
                                         whereToFind: GuideParts.pinTitles, alsoCalled: ["Q", "1Q"]),
            "device.units.qn": FieldGuide(label: "Inverted output", help: "The pin that shows the opposite of Q.",
                                          whereToFind: GuideParts.pinTitles, alsoCalled: ["Q with a bar", "QN"]),
            "device.kInverted": FieldGuide(
                label: "The K input is inverted",
                help: "Yes for a J-K-bar flip-flop such as the 74HC109, whose K input acts when it is low "
                    + "(drawn with a bar). No for an ordinary JK flip-flop (74HC112).",
                whereToFind: GuideParts.functionTitles,
                alsoCalled: ["J-K bar", "K with a bar"]),
        ]) { _, new in new }))
}
