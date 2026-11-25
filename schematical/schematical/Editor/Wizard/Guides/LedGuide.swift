import Foundation

/// LEDs, switches, pin headers (plan-46-M; docs/parts-format.md § `led`,
/// § `switch`, § `header`): the three kinds with one or two screens.
enum LedGuide {
    static let guide = KindGuide(
        steps: { _ in [
            WizardStep(id: "kind.led", title: "The LED",
                       intro: "An LED lights while its anode is high and its cathode is low. Name the two pins "
                           + "and the colors the part comes in.",
                       page: .kindFields, keys: ["device.anode", "device.cathode", "device.colors"],
                       searchTerms: ["Package Dimensions", "Internal Circuit Diagram", "Polarity", "Features"]),
        ] },
        fields: [
            "device.anode": FieldGuide(
                label: "Anode pin", help: "The positive pin: the longer lead of a through-hole LED.",
                whereToFind: ["Package Dimensions", "Internal Circuit Diagram", "Polarity"],
                alsoCalled: ["A", "+", "Anode"]),
            "device.cathode": FieldGuide(
                label: "Cathode pin",
                help: "The negative pin: the shorter lead, on the flat side of a round LED, or the marked "
                    + "end of a chip LED.",
                whereToFind: ["Package Dimensions", "Internal Circuit Diagram", "Polarity"],
                alsoCalled: ["K", "C", "−", "Cathode"]),
            "device.colors": FieldGuide(
                label: "Colors",
                help: "The colors a placed LED can be given; the first is the one it starts with.",
                whereToFind: ["Features", "Electro-Optical Characteristics", "Ordering Information"],
                alsoCalled: ["Emitted color", "Dominant wavelength"],
                example: "Red"),
        ])
}

enum SwitchGuide {
    static let guide = KindGuide(
        steps: { _ in [
            WizardStep(id: "kind.switch", title: "The switch",
                       intro: "A switch joins two pins while it is on. List each position's pair of pins, and "
                           + "say whether it is a push button that springs back.",
                       page: .kindFields,
                       keys: ["device.actuator", "device.contacts", "device.momentary", "device.normallyClosed",
                              "device.joined", "device.states", "device.rest"],
                       searchTerms: ["Circuit Diagram", "Schematic", "Specifications", "Features", "Dimensions"]),
            WizardStep(id: "kind.ratings", title: "Ratings and size",
                       intro: "The voltage the contacts may switch, and the body size the drawer files a push "
                           + "button under.",
                       page: .kindFields, keys: ["device.ratedVolts", "device.bodySize"],
                       searchTerms: ["Specifications", "Electrical", "Contact Rating", "Ratings", "Dimensions"]),
        ] },
        fields: [
            "device.actuator": FieldGuide(
                label: "Actuator style",
                help: "What you push or slide, as the datasheet names it. It is shown in the drawer.",
                whereToFind: ["Features", "Ordering Information", "Dimensions"],
                alsoCalled: ["Actuator", "Lever", "Plunger"],
                example: "Flush actuator"),
            "device.contacts": FieldGuide(
                label: "Positions",
                help: "One entry per switch position: the two pins it joins while it is on. A 4-position DIP "
                    + "switch has four entries; a push button has one.",
                whereToFind: ["Circuit Diagram", "Schematic", "Dimensions"],
                alsoCalled: ["Poles", "Circuit", "Contact arrangement"]),
            "device.momentary": FieldGuide(
                label: "Springs back (push button)",
                help: "Yes for a tactile switch that is only on while pressed. Leave it out for a DIP or "
                    + "slide switch that stays where it is put.",
                whereToFind: ["Features", "Specifications"],
                alsoCalled: ["Momentary", "Tactile"]),
            "device.normallyClosed": FieldGuide(
                label: "Closed at rest",
                help: "Yes for a push button whose contact is closed until it is pressed (normally closed). "
                    + "Most are normally open: leave it out.",
                whereToFind: ["Circuit Diagram", "Specifications"],
                alsoCalled: ["NC", "Normally closed", "SPST-NC"]),
            "device.joined": FieldGuide(
                label: "Pins joined inside",
                help: "Pairs of pins that are always connected inside the part: a four-leg push button has "
                    + "two legs on each side of its contact.",
                whereToFind: ["Circuit Diagram", "Schematic", "Dimensions"],
                alsoCalled: ["Common terminals"]),
            "device.states": FieldGuide(
                label: "Knob positions (slide switch)",
                help: "For a slide switch: one entry per knob position, position 1 first, each listing the "
                    + "contact numbers (1 for the first entry of the contacts) that are closed there. An "
                    + "On-Off-On switch has three, the middle one empty. Leave it out for a DIP or push button.",
                whereToFind: ["Circuit Diagram", "Schematic", "Specifications"],
                alsoCalled: ["Circuit", "Switch action", "On-On", "On-Off-On"],
                example: "[1], [2]"),
            "device.rest": FieldGuide(
                label: "Starting position (slide switch)",
                help: "The knob position a new switch starts in, 1 for the first. A switch with an Off "
                    + "position starts Off. Leave it out to start in position 1.",
                whereToFind: ["Circuit Diagram", "Features"],
                alsoCalled: ["Default position"],
                example: "1"),
            "device.ratedVolts": FieldGuide(
                label: "Switching voltage rating",
                help: "The highest voltage the contacts may switch (the switching rating, not the higher "
                    + "non-switching one), in volts.",
                whereToFind: ["Specifications", "Electrical", "Contact Rating", "Ratings"],
                alsoCalled: ["Contact rating", "Switching voltage", "Rated voltage"],
                example: "24"),
            "device.bodySize": FieldGuide(
                label: "Body size",
                help: "Length then width of the body in mm, the larger first. The drawer lists a push "
                    + "button under it (6 × 6 mm).",
                whereToFind: ["Dimensions", "Mechanical Data", "Outline Drawing"],
                alsoCalled: ["Size", "Dimension"],
                example: "6, 6"),
        ])
}

enum HeaderGuide {
    static let guide = KindGuide(
        steps: { _ in [
            WizardStep(id: "kind.header", title: "The header",
                       intro: "A pin header is just pins: each one is its wire. Say how many rows it has; the "
                           + "pin count comes from the Pins step.",
                       page: .kindFields, keys: ["device.rows", "device.genders"],
                       searchTerms: ["Dimensions", "Specifications", "Part Numbering", "How to Order"]),
        ] },
        fields: [
            "device.rows": FieldGuide(
                label: "Rows",
                help: "1 for a single row of pins, 2 for a double row. The pin count must be a multiple of it. "
                    + "A two-row header numbers its pins in a zigzag: 1 and 2 across, then 3 and 4.",
                whereToFind: ["Dimensions", "Part Numbering", "How to Order"],
                alsoCalled: ["Single row", "Dual row", "Number of rows"],
                example: "2"),
            "device.genders": FieldGuide(
                label: "Genders",
                help: "What a placed header can be: Male (pins) or Female (sockets). The first is the one it "
                    + "starts as.",
                whereToFind: ["Features", "Part Numbering", "How to Order"],
                alsoCalled: ["Pin header", "Socket", "Receptacle"],
                example: "Male"),
        ])
}
