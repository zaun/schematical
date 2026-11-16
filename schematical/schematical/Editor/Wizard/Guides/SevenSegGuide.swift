import Foundation

/// LED 7-segment displays (plan-46-M; docs/parts-format.md § `7seg`).
enum SevenSegGuide {
    private static let circuit = ["Internal Circuit Diagram", "Circuit Diagram", "Pin Connection", "Pin Configuration"]
    private static let optical = ["Electrical / Optical Characteristics", "Electro-Optical Characteristics",
                                  "Optical Characteristics", "Features"]

    static let guide = KindGuide(
        steps: { _ in [
            WizardStep(id: "kind.display", title: "The display",
                       intro: "How many digits it has, how big they are, and its LEDs' color and forward "
                           + "voltage. These are on the datasheet's first page and in its characteristics "
                           + "table.",
                       page: .kindFields,
                       keys: ["device.digits", "device.size", "device.color", "device.chip", "device.vf"],
                       searchTerms: optical + ["Description"]),
            WizardStep(id: "kind.wiring", title: "Wiring",
                       intro: "Which pin lights which segment, and which pins are the digits' commons. The "
                           + "datasheet's internal circuit diagram shows it: every LED sits between one "
                           + "segment pin and its digit's common pin.",
                       page: .kindFields,
                       keys: ["device.commonAnode", "device.commonPins", "device.segmentPins", "device.leds",
                              "device.commonSide", "device.segmentSide"],
                       searchTerms: circuit),
            WizardStep(id: "kind.dots", title: "Dots and colon",
                       intro: "Where a clock display's colon and raised dot sit. A plain digit display has "
                           + "neither: both are 0.",
                       page: .kindFields, keys: ["device.dotAfter", "device.colonAfter"],
                       searchTerms: circuit + ["Package Dimensions"]),
        ] },
        fields: [
            "device.digits": FieldGuide(
                label: "Digits", help: "How many digits the display has, 1 to 8.",
                whereToFind: ["Description", "Features", "Package Dimensions"],
                alsoCalled: ["Single digit", "Dual digit", "Quadruple digit"], example: "4"),
            "device.size": FieldGuide(
                label: "Digit height", help: "The height of a digit, as the datasheet gives it.",
                whereToFind: ["Features", "Description", "Package Dimensions"],
                alsoCalled: ["Digit height", "Character height"], example: "0.56 in"),
            "device.color": FieldGuide(
                label: "Color", help: "The color the segments light in.",
                whereToFind: optical, alsoCalled: ["Emitted color"], example: "Red"),
            "device.chip": FieldGuide(
                label: "LED chip", help: "The LED material the datasheet names. It is shown, not used.",
                whereToFind: optical + ["Description"], alsoCalled: ["Chip material", "Source color"],
                example: "AlInGaP Red"),
            "device.vf": FieldGuide(
                label: "Forward voltage per segment",
                help: "Lowest and highest forward voltage of one segment, in volts. Where the datasheet prints "
                    + "no minimum, use its typical value for the lowest.",
                whereToFind: optical, alsoCalled: ["VF", "Forward voltage per segment"], example: "1.7, 2.4"),
            "device.commonAnode": FieldGuide(
                label: "Common anode",
                help: "Yes when the digits' common pins are the LEDs' anodes (wired to the supply); No for "
                    + "common cathode (commons wired to ground).",
                whereToFind: circuit + ["Description"], alsoCalled: ["CA", "CC", "Common cathode"]),
            "device.commonPins": FieldGuide(
                label: "Common pins",
                help: "Each common pin's name and the digit it belongs to (1 is the leftmost). A common that "
                    + "serves the colon or dot LEDs is “extras”.",
                whereToFind: circuit, alsoCalled: ["Digit 1 common", "CC1", "CA1", "COM"]),
            "device.segmentPins": FieldGuide(
                label: "Segment pins",
                help: "Each segment pin's name, the digit it serves (0 when one pin serves every digit, as "
                    + "on a multiplexed display) and the segments it lights.",
                whereToFind: circuit, alsoCalled: ["A to G", "DP", "Anode A", "Cathode A"]),
            "device.segmentPins.digit": FieldGuide(
                label: "Digit", help: "The digit this pin serves; 0 when it serves every digit.",
                whereToFind: circuit, example: "0"),
            "device.segmentPins.segments": FieldGuide(
                label: "Segments", help: "The segments this pin lights: A to G, DP for the decimal point.",
                whereToFind: circuit, alsoCalled: ["a, b, c, d, e, f, g, dp"]),
            "device.leds": FieldGuide(
                label: "Every LED, in order",
                help: "One entry per LED of the display, digit by digit: its digit (0 for the colon and dot "
                    + "group) and its segment. This is the order the display's lights are reported in.",
                whereToFind: circuit, alsoCalled: ["Segments"]),
            "device.leds.digit": FieldGuide(
                label: "Digit", help: "The LED's digit, 1 for the leftmost; 0 for a colon or dot LED.",
                whereToFind: circuit, example: "1"),
            "device.leds.segment": FieldGuide(
                label: "Segment",
                help: "A to G around the digit (A at the top, clockwise, G in the middle), DP for the "
                    + "decimal point, L1 to L3 for colon and dot LEDs.",
                whereToFind: circuit, alsoCalled: ["a, b, c, d, e, f, g, dp"], example: "A"),
            "device.commonSide": FieldGuide(
                label: "Common pins, in symbol order",
                help: "The common pins in the order the schematic symbol lists them.",
                whereToFind: circuit),
            "device.segmentSide": FieldGuide(
                label: "Segment pins, in symbol order",
                help: "The segment pins in the order the schematic symbol lists them, usually A to G then DP.",
                whereToFind: circuit),
            "device.dotAfter": FieldGuide(
                label: "Raised dot after digit",
                help: "The digit a raised dot LED (L3) sits after; 0 when the display has none.",
                whereToFind: circuit + ["Package Dimensions"], example: "0"),
            "device.colonAfter": FieldGuide(
                label: "Colon after digit",
                help: "The digit the colon (L1, L2) sits after, 2 on a clock display; 0 when the display "
                    + "has none.",
                whereToFind: circuit + ["Package Dimensions"], example: "0"),
        ])
}
