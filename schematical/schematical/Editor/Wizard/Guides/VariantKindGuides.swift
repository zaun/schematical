import Foundation

// The curated guides of the simple kinds with models (plan-46-M Phase 2):
// buffer, latch, mux, decoder and diode. The first four lay their steps out
// by `GuideLayout.byRule`; each guide says what its kind's keys mean
// (docs/parts-format.md § `buffer`, § `latch`, § `mux`, § `decoder`,
// § `diode`).

enum BufferGuide {
    static let guide = GuideLayout.guide(kind: "buffer", fields: GuideLayout.common(
        units: "One entry per group of bits that share their enable pins: a 74HC244 has two groups of four.")
        .merging([
            "device.invert": GuideLayout.function(
                "Inverting", "Yes when every output is the opposite of its input (74HC240, 74HC540).",
                also: ["Inverting buffer", "Inverted outputs"]),
            "device.domains": FieldGuide(
                label: "Supply sides",
                help: "For a level translator: one entry per supply, with the supply pin and the signal pins "
                    + "that run at its voltage (the A side on VCCA, the B side on VCCB).",
                whereToFind: GuideParts.pinTitles + ["Description"],
                alsoCalled: ["VCCA", "VCCB", "Port A", "Port B"]),
            "device.domains.pins": GuideLayout.pins("Signal pins on this supply", "The pins whose levels follow this supply."),
            "device.domains.power": GuideLayout.pins("Supply pin", "The power pin of this side.", also: ["VCCA", "VCCB"]),
            "device.units.a": GuideLayout.pins("Inputs (A side)", "The input pins, or the A-side pins of a two-way buffer.",
                                               also: ["A", "1A1"]),
            "device.units.y": GuideLayout.pins("Outputs", "The output pins, in the same order as the inputs.", also: ["Y", "1Y1"]),
            "device.units.b": GuideLayout.pins("B side", "The B-side pins of a two-way buffer, in the same order as the A side.",
                                               also: ["B", "B1"]),
            "device.units.oe": FieldGuide(
                label: "Output enable",
                help: "The pin that turns the outputs on, and the level at which it does. Leave it out for a "
                    + "part that always drives.",
                whereToFind: GuideParts.functionTitles + ["Pin Functions"],
                alsoCalled: ["OE", "G", "Enable"]),
            "device.units.dir": FieldGuide(
                label: "Direction pin",
                help: "The pin that picks which way a transceiver drives, and which way it drives when the "
                    + "pin is high.",
                whereToFind: GuideParts.functionTitles + ["Pin Functions"],
                alsoCalled: ["DIR", "T/R"]),
            "device.units.dir.ab": GuideLayout.level("Level for A to B", "The level of the direction pin that drives from A to B."),
            "device.units.ab": GuideLayout.function("A to B storage", "The latch or register between the A side and the B side."),
            "device.units.ba": GuideLayout.function("B to A storage", "The latch or register between the B side and the A side."),
            "device.units.ab.store": GuideLayout.function("Stores with", "A latch (follows while enabled) or a register (loads on a clock edge).",
                                                         example: "latch"),
            "device.units.ba.store": GuideLayout.function("Stores with", "A latch (follows while enabled) or a register (loads on a clock edge).",
                                                         example: "latch"),
            "part.family": FieldGuide(
                label: "Logic family",
                help: "The letters after 74 in the part number: 74HC245 is HC. It picks which timing entry the "
                    + "part uses.",
                whereToFind: ["Ordering Information", "Device Information"],
                alsoCalled: ["Technology", "Series"], example: "HC"),
        ]) { _, new in new })
}

enum LatchGuide {
    static let guide = GuideLayout.guide(kind: "latch", fields: GuideLayout.common(
        units: "One entry per latch, naming its pins. Latches may share a pin, such as a common latch enable.")
        .merging([
            "device.units.d": GuideLayout.pins("Data input (D)", "The pin the latch follows while it is enabled.", also: ["D", "1D"]),
            "device.units.le": GuideLayout.pins("Latch enable", "The pin that lets the data through.", also: ["LE", "C", "G"]),
            "device.units.q": GuideLayout.pins("Output (Q)", "The pin that shows the latched level.", also: ["Q", "1Q"]),
            "device.units.qn": GuideLayout.pins("Inverted output", "The pin that shows the opposite of Q.", also: ["Q with a bar"]),
            "device.units.oe": GuideLayout.pins("Output enable", "The pin that turns the output on.", also: ["OE", "OC"]),
            "device.units.pre": GuideLayout.pins("Preset", "The pin that forces the latch to 1."),
            "device.units.clr": GuideLayout.pins("Clear", "The pin that forces the latch to 0."),
            "device.units.s": GuideLayout.pins("Set inputs", "The pin or pins that set the latch (any one of them does).", also: ["S"]),
            "device.units.r": GuideLayout.pins("Reset input", "The pin that resets the latch.", also: ["R"]),
            "device.enableActive": GuideLayout.level("Latch enable is active", "The level of the latch enable at which a latch follows its data.",
                                                     also: ["LE", "C", "G"]),
            "device.outputEnableActive": GuideLayout.level("Output enable is active", "The level of the output enable at which the outputs drive.",
                                                           also: ["OE"]),
            "device.presetActive": GuideLayout.level("Preset is active", "The level at which preset forces a latch to 1."),
            "device.clearActive": GuideLayout.level("Clear is active", "The level at which clear forces a latch to 0."),
            "device.presetWins": GuideLayout.function("Preset wins over clear", "What both asserted together store: Yes for 1, No for 0."),
            "device.polarity": GuideLayout.pins("Polarity pin", "A pin whose level sets which latch-enable level is the active one (CD4042B). "
                + "Use it instead of the latch-enable level.", also: ["POLARITY"]),
            "device.phase": GuideLayout.pins("Phase pin", "A pin whose level inverts every latch output, for liquid-crystal drive (CD4054B).",
                                             also: ["DF", "Ph"]),
            "device.address": GuideLayout.pins("Address pins", "The pins that pick the latch, least significant first.", also: ["A0, A1, A2", "S0, S1, S2"]),
            "device.data": GuideLayout.pins("Data pin", "The pin whose level the addressed latch takes.", also: ["D"]),
            "device.outputs": GuideLayout.pins("Outputs", "One pin per latch, in address order.", also: ["Q0 to Q7"]),
            "device.enable": FieldGuide(label: "Enable", help: "The pin that makes the addressed latch follow the data, and its active level.",
                                        whereToFind: GuideParts.functionTitles, alsoCalled: ["G", "E", "LE"]),
            "device.clear": FieldGuide(label: "Clear", help: "The pin that clears every latch at once, and its active level. Leave it out if there is none.",
                                       whereToFind: GuideParts.functionTitles, alsoCalled: ["CLR", "MR"]),
            "device.invert": GuideLayout.function("Outputs are inverted", "Yes when a pin shows the opposite of its latch's bit."),
            "device.inputsActive": GuideLayout.level("Set and reset are active", "The level at which S sets and R resets."),
            "device.both": GuideLayout.function("Both asserted gives", "What Q shows while set and reset are both asserted: set or reset.",
                                                example: "set"),
            "device.outputEnable": FieldGuide(label: "Output enable", help: "A pin that turns every output on, and its active level.",
                                              whereToFind: GuideParts.functionTitles, alsoCalled: ["OE", "E"]),
            "device.inputs": GuideLayout.pins("BCD inputs", "The four code inputs, least significant first.", also: ["A, B, C, D", "D0 to D3"]),
            "device.latch": FieldGuide(label: "Latch enable", help: "The pin that holds the code, and the level at which it holds.",
                                       whereToFind: GuideParts.functionTitles, alsoCalled: ["LE", "Strobe"]),
            "device.lampTest": FieldGuide(label: "Lamp test", help: "A pin that lights every segment, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["LT"]),
            "device.blank": FieldGuide(label: "Blanking", help: "A pin that turns every segment off, and its active level.",
                                       whereToFind: GuideParts.functionTitles, alsoCalled: ["BL", "BI"]),
            "device.segments": GuideLayout.pins("Segment outputs", "The seven segment pins, a to g in order.", also: ["a to g"]),
            "device.glyphs": GuideLayout.function("Segment patterns",
                "Sixteen entries, one per code 0 to 15: seven characters, 1 for a lit segment and 0 for a dark "
                    + "one, in the order a to g. Read them off the datasheet's display figure.",
                also: ["Display", "Font"], example: "1111110"),
            "device.a": GuideLayout.pins("A port", "The twelve A-side pins.", also: ["A1 to A12"]),
            "device.b1": GuideLayout.pins("1B port", "The twelve 1B-side pins.", also: ["1B1 to 1B12"]),
            "device.b2": GuideLayout.pins("2B port", "The twelve 2B-side pins.", also: ["2B1 to 2B12"]),
            "device.select": GuideLayout.pins("Select pin", "High: the A port shows the 1B latch. Low: the 2B latch.", also: ["SEL"]),
            "device.latchEnables": GuideLayout.pins("Latch enable pins", "The four latch enables; a latch follows while its pin is high.",
                                                    also: ["LE1B, LE2B, LEA1B, LEA2B"]),
            "device.outputEnables": GuideLayout.pins("Output enable pins", "The three output enables; a port drives while its pin is low.",
                                                     also: ["OEA, OE1B, OE2B"]),
        ]) { _, new in new })
}

enum MuxGuide {
    static let guide = GuideLayout.guide(kind: "mux", fields: GuideLayout.common(
        units: "One entry per selector in the package: a 74HC153 has two, each with four data inputs.")
        .merging([
            "device.units.data": GuideLayout.pins("Data inputs", "The inputs, in select-code order: the first is chosen by code 0.",
                                                  also: ["D0 to D7", "1C0 to 1C3", "A, B"]),
            "device.units.select": GuideLayout.pins("Select pins", "The pins that pick the input, least significant first. Usually shared by every unit.",
                                                    also: ["S0, S1", "A, B", "SEL"]),
            "device.units.y": GuideLayout.pins("Output (Y)", "The output that shows the chosen input.", also: ["Y", "Z"]),
            "device.units.w": GuideLayout.pins("Inverted output (W)", "The output that shows the opposite.", also: ["W", "Y with a bar"]),
            "device.units.strobe": FieldGuide(label: "Strobe", help: "A pin that forces the outputs to their disabled state, and its active level.",
                                              whereToFind: GuideParts.functionTitles, alsoCalled: ["G", "E", "Strobe"]),
            "device.units.oe": FieldGuide(label: "Output enable", help: "A pin that turns the outputs on, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["OE", "G"]),
            "device.disabled": GuideLayout.function("Outputs while disabled",
                "What the outputs show while the strobe or enable is off: inactive (Y low) or z (off the bus).",
                example: "inactive"),
            "device.invert": GuideLayout.function("Inverting", "Yes when Y shows the opposite of the chosen input (74HC158)."),
            "device.selectLatch": FieldGuide(label: "Select latch", help: "A pin that holds the select inputs, and the level at which it holds.",
                                             whereToFind: GuideParts.functionTitles, alsoCalled: ["LE"]),
            "device.dataLatch": FieldGuide(label: "Data latch", help: "A pin that holds the data inputs, and the level at which it holds.",
                                           whereToFind: GuideParts.functionTitles, alsoCalled: ["E", "LE"]),
            "device.dataClock": FieldGuide(label: "Data clock", help: "A clock that stores the data inputs, and its edge.",
                                           whereToFind: GuideParts.functionTitles, alsoCalled: ["CP", "CLK"]),
            "device.outputClock": FieldGuide(label: "Output clock", help: "A clock that stores the chosen value at the output, and its edge.",
                                             whereToFind: GuideParts.functionTitles, alsoCalled: ["CLK"]),
            "device.oes": FieldGuide(label: "Output enables", help: "Pins that must all be active for the outputs to drive, each with its level.",
                                     whereToFind: GuideParts.functionTitles, alsoCalled: ["OE1, OE2, OE3"]),
            "device.words": GuideLayout.function("Words", "How many words the register file holds: 2, 4, 8 or 16.", example: "4"),
            "device.bits": GuideLayout.function("Bits per word", "How wide each word is, 1 to 8.", example: "4"),
            "device.files": FieldGuide(label: "Register files", help: "One entry per file: its data, write address, write enable, read address, read enable and output pins.",
                                       whereToFind: GuideParts.pinTitles + ["Function Table"], alsoCalled: ["D, WA, WB, GW, RA, RB, GR, Q"]),
            "device.files.d": GuideLayout.pins("Data inputs", "One pin per bit of a word."),
            "device.files.q": GuideLayout.pins("Outputs", "One pin per bit of a word."),
            "device.files.wa": GuideLayout.pins("Write address", "The pins that pick the word to write, least significant first."),
            "device.files.ra": GuideLayout.pins("Read address", "The pins that pick the word to read, least significant first."),
            "device.files.we": FieldGuide(label: "Write enable", help: "The pin that writes the addressed word, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["GW", "WE"]),
            "device.files.re": FieldGuide(label: "Read enable", help: "The pin that puts the addressed word on the outputs, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["GR", "RE"]),
        ]) { _, new in new })
}

enum DecoderGuide {
    static let guide = GuideLayout.guide(kind: "decoder", fields: GuideLayout.common(
        units: "One entry per decoder in the package: a 74HC139 has two.")
        .merging([
            "device.units.address": GuideLayout.pins("Address inputs", "The pins that pick the output, least significant first.",
                                                     also: ["A, B, C", "A0, A1, A2"]),
            "device.units.outputs": GuideLayout.pins("Outputs", "The outputs, in code order: the first is picked by code 0.",
                                                     also: ["Y0 to Y7"]),
            "device.units.enables": FieldGuide(label: "Enables", help: "The enable pins, each with its active level; the decoder works while all are active.",
                                               whereToFind: GuideParts.functionTitles, alsoCalled: ["G1, G2A, G2B", "E1, E2, E3"]),
            "device.units.latch": GuideLayout.pins("Address latch pin", "A pin that holds the address (74HC137).", also: ["LE", "GL"]),
            "device.units.clock": GuideLayout.pins("Clock", "A clock that stores the address, on parts that have one."),
            "device.outputsActive": GuideLayout.level("The selected output is", "The level of the one output that is picked: low on a 74HC138.",
                                                      also: ["Active-low outputs"]),
            "device.unselected": GuideLayout.function("Outputs not selected show", "inactive (the opposite level) or z (off the bus).",
                                                      example: "inactive"),
            "device.disabled": GuideLayout.function("Outputs while disabled show", "inactive, or z when a disabled part lets go of its outputs.",
                                                    example: "inactive"),
            "device.latchActive": GuideLayout.level("The latch holds while", "The level of the address latch pin at which the address is held."),
            "device.monitor": GuideLayout.pins("Supply monitor pin", "An open-collector supply-good output, on the few parts that have one."),
            "device.inputs": GuideLayout.pins("Inputs", "A priority encoder's inputs, lowest priority first; or a 7-segment decoder's four code inputs, least significant first.",
                                              also: ["0 to 7", "A, B, C, D"]),
            "device.inputsActive": GuideLayout.level("Inputs are active", "The level at which an input counts as asserted."),
            "device.code": GuideLayout.pins("Code outputs", "The pins that show the winning input's number, least significant first.",
                                            also: ["A0, A1, A2"]),
            "device.codeActive": GuideLayout.level("Code outputs are active", "Low when each code pin shows the inverted bit (74HC148)."),
            "device.offset": GuideLayout.function("Number of the first input", "Added to the winning input's position: 1 when the inputs are numbered 1 to 9 (74HC147).",
                                                  example: "0"),
            "device.idle": GuideLayout.function("Code with no input active", "The number the code pins show while no input is asserted.",
                                                example: "0"),
            "device.enableIn": FieldGuide(label: "Enable input", help: "The pin that enables the encoder, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["EI"]),
            "device.enableOut": FieldGuide(label: "Enable output", help: "The output that is active while the encoder is enabled and no input is.",
                                           whereToFind: GuideParts.functionTitles, alsoCalled: ["EO"]),
            "device.groupSelect": FieldGuide(label: "Group select output", help: "The output that is active while the encoder is enabled and some input is.",
                                             whereToFind: GuideParts.functionTitles, alsoCalled: ["GS"]),
            "device.segments": GuideLayout.pins("Segment outputs", "The seven segment pins, a to g in order.", also: ["a to g"]),
            "device.segmentsActive": GuideLayout.level("A lit segment is", "The level of a segment pin that lights its segment: low on a 7447.",
                                                       also: ["Active-low outputs"]),
            "device.glyphs": GuideLayout.function("Segment patterns",
                "Sixteen entries, one per code 0 to 15: seven characters, 1 for a lit segment and 0 for a dark "
                    + "one, in the order a to g. Read them off the datasheet's display figure.",
                also: ["Display", "Font"], example: "1111110"),
            "device.lampTest": FieldGuide(label: "Lamp test", help: "A pin that lights every segment, and its active level.",
                                          whereToFind: GuideParts.functionTitles, alsoCalled: ["LT"]),
            "device.blank": FieldGuide(label: "Blanking", help: "A pin that turns every segment off, and its active level.",
                                       whereToFind: GuideParts.functionTitles, alsoCalled: ["BI"]),
            "device.latch": FieldGuide(label: "Latch", help: "The pin that holds the code (CD4056B strobe, CD4543B latch disable), and the level at which it holds.",
                                       whereToFind: GuideParts.functionTitles, alsoCalled: ["STROBE", "LD"]),
            "device.rbi": GuideLayout.pins("Ripple blanking input", "The pin that blanks a leading zero (7447).", also: ["RBI"]),
            "device.rbo": GuideLayout.pins("Ripple blanking output", "The pin that passes zero blanking on to the next digit.", also: ["BI/RBO"]),
            "device.phase": GuideLayout.pins("Phase pin", "A pin that inverts every segment level, for liquid-crystal drive (CD4056B).",
                                             also: ["DF", "Ph"]),
            "device.phaseOut": GuideLayout.pins("Phase output", "An output pin that repeats the phase pin's level (CD4055B DF OUT).",
                                                also: ["DF OUT"]),
        ]) { _, new in new })
}

/// Diodes: the model, the diodes in the package, then the model's ratings.
enum DiodeGuide {
    private static let ratings = ["Absolute Maximum Ratings", "Maximum Ratings", "Electrical Characteristics",
                                  "Characteristics"]

    static let guide = KindGuide(
        steps: { variant in
            guard let schema = PartKinds.schemas["diode"] else { return [] }
            let variantStep = WizardStep(
                id: "kind.variant", title: "What kind of diode is it?",
                intro: "The kind decides which ratings the part has: a Zener has a Zener voltage and a "
                    + "power rating; the others a reverse voltage and a forward current.",
                page: .variant, keys: ["device.type"])
            let part = schema.partFields(variant: variant).map { "part.\($0.key)" }
            return [
                variantStep,
                WizardStep(id: "kind.diodes", title: "Diodes",
                           intro: "One entry per diode in the package, naming its anode and cathode pins. "
                               + "A single diode has one; a bridge rectifier has four.",
                           page: .kindFields, keys: ["device.diodes"],
                           searchTerms: ["Pin Configuration", "Pinning", "Package Outline", "Circuit Diagram",
                                         "Features"]),
                GuideParts.step(id: "kind.ratings", title: "Ratings",
                                intro: "The limits the datasheet gives. They are shown in the drawer and on the "
                                    + "bill of materials.",
                                keys: part, searchTerms: ratings),
            ].compactMap { $0 }
        },
        fields: [
            "device.type": FieldGuide(
                label: "Kind of diode", help: "Rectifier, fast, switching, Schottky, Zener or bridge.",
                whereToFind: ["Features", "Description"], alsoCalled: ["Type"]),
            "device.diodes": FieldGuide(
                label: "Diodes", help: "Add one entry for each diode in the package.",
                whereToFind: ["Pin Configuration", "Pinning", "Circuit Diagram", "Package Outline"],
                alsoCalled: ["Elements"]),
            "device.diodes.anode": FieldGuide(
                label: "Anode pin", help: "The pin current flows into: the end without the band.",
                whereToFind: ["Pin Configuration", "Pinning", "Package Outline"], alsoCalled: ["A", "+"]),
            "device.diodes.cathode": FieldGuide(
                label: "Cathode pin", help: "The pin current flows out of: the end marked with a band.",
                whereToFind: ["Pin Configuration", "Pinning", "Package Outline"], alsoCalled: ["K", "C", "−"]),
            "part.vr": FieldGuide(
                label: "Reverse voltage", help: "The highest repetitive reverse voltage, in volts.",
                whereToFind: ratings, alsoCalled: ["VRRM", "VR", "Peak repetitive reverse voltage"], example: "1000"),
            "part.io": FieldGuide(
                label: "Forward current", help: "The average forward current of one diode, in amps.",
                whereToFind: ratings, alsoCalled: ["IF(AV)", "IO", "Average rectified current"], example: "1"),
            "part.vz": FieldGuide(
                label: "Zener voltage", help: "The nominal Zener voltage, in volts.",
                whereToFind: ["Electrical Characteristics", "Characteristics"], alsoCalled: ["VZ", "Nominal Zener voltage"],
                example: "5.1"),
            "part.pd": FieldGuide(
                label: "Power rating", help: "The highest power the diode may dissipate, in watts.",
                whereToFind: ratings, alsoCalled: ["PD", "Ptot", "Power dissipation"], example: "0.5"),
        ])
}
