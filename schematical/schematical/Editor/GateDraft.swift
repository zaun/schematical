import Foundation

/// Values shared by the kinds' schemas.
enum SchemaValues {
    /// The logic families the backend knows (`kind_logic_family`, `kind_util.c`).
    static let logicFamilies = [
        "HC", "LS", "ACT", "HCT", "F", "AC", "AHCT", "ALS", "AS", "AHC", "S", "C", "H", "HCS", "LV", "LVC",
        "AUP", "AUC", "ALVC", "TTL", "ABT", "BCT", "LVT", "AVC", "TPIC", "FCT", "L", "ALV", "AXC", "LXC",
        "TXU", "TXV",
    ]

    /// A rise/fall delay pair, ns (`kind_ns`).
    static let delays: [Field] = [
        Field(key: "rise", label: "Rise", type: .number(unit: "ns"), required: true),
        Field(key: "fall", label: "Fall", type: .number(unit: "ns"), required: true),
        Field(key: "note", label: "Note", type: .note),
    ]

    /// A part's or timing entry's supply keys (`kind_voltage`, docs/parts-format.md § Supply voltage).
    static let supply: [Field] = [
        Field(key: "vcc", label: "VCC range", type: .voltRange,
              help: "Recommended supply, [min, max] volts"),
        Field(key: "inputMax", label: "Input max", type: .number(unit: "V"),
              help: "Highest input voltage; follows the VCC max when left out"),
    ]
}

/// The gate kind's keys (`backend/src/models/gate.c`): its device's gate
/// operations, output type, timing per family and unit count, and its part's
/// family and supply.
enum GateDraft {
    /// `OP_NAMES` in gate.c.
    static let operations = ["and", "nand", "or", "nor", "xor", "xnor", "not", "buf"]

    static let schema = KindSchema(
        kind: "gate",
        device: [
            Field(key: "gates", label: "Gates", type: .list([
                Field(key: "op", label: "Operation", type: .choice(operations), required: true),
                Field(key: "inputs", label: "Inputs", type: .pins, required: true,
                      help: "1 to 13 pins; not and buf take exactly 1"),
                Field(key: "output", label: "Output", type: .pin, required: true),
            ]), required: true),
            Field(key: "output", label: "Output type", type: .choice(["push-pull", "open-drain"]),
                  help: "Left out: push-pull"),
            Field(key: "timing", label: "Timing", type: .map(keyLabel: "Family", [
                Field(key: "rise", label: "Rise", type: .number(unit: "ns")),
                Field(key: "fall", label: "Fall", type: .number(unit: "ns")),
                Field(key: "otherLow", label: "Other input low", type: .object(SchemaValues.delays),
                      help: "Split timing (2-input gates): delays while the other input is low"),
                Field(key: "otherHigh", label: "Other input high", type: .object(SchemaValues.delays)),
                Field(key: "note", label: "Note", type: .note),
            ] + SchemaValues.supply), required: true,
                  help: "One entry per logic family the parts use"),
            Field(key: "units", label: "Units", type: .integer,
                  help: "How many gates the drawer counts; left out: the number of gates"),
        ],
        part: [
            Field(key: "family", label: "Family", type: .suggested(SchemaValues.logicFamilies), required: true,
                  help: "One of the device's timing families"),
        ] + SchemaValues.supply
    )
}
