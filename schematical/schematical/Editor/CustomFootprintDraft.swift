import Foundation

/// One pad row of a custom footprint: the `explicit` style's pad keys
/// (`footprints.c`, `PAD_KEYS`; docs/parts-format.md § `explicit`).
struct PadDraft: Hashable, Identifiable, Sendable {
    var id = UUID()
    /// The pinout pin number; 0 = an unnumbered centre pad.
    var pin: Int
    var x: Double
    var y: Double
    var width: Double
    var height: Double
    /// "rect" or "round".
    var shape: String = "rect"
    /// 0 = surface mount.
    var drill: Double = 0
    /// 0 = a round hole.
    var slotLength: Double = 0
    /// False only for a non-plated hole (pin 0).
    var plated = true
    /// Rect only; 0 = square corners.
    var radius: Double = 0
    /// Rect only; 0 = no cut.
    var chamfer: Double = 0
    /// The cut corners, of "tl", "tr", "br", "bl".
    var corners: [String] = []

    static let cornerNames = ["tl", "tr", "br", "bl"]

    static func == (a: PadDraft, b: PadDraft) -> Bool {
        a.pin == b.pin && a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height
            && a.shape == b.shape && a.drill == b.drill && a.slotLength == b.slotLength && a.plated == b.plated
            && a.radius == b.radius && a.chamfer == b.chamfer && a.corners == b.corners
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(pin)
        hasher.combine(x)
        hasher.combine(y)
    }

    /// The pad's entry: required keys, then each optional key only when it
    /// is not its default.
    func encoded() -> JSONValue {
        var keys: [String: JSONValue] = [
            "pin": .number(Double(pin)), "x": .number(x), "y": .number(y),
            "width": .number(width), "height": .number(height), "shape": .string(shape),
        ]
        if drill != 0 { keys["drill"] = .number(drill) }
        if slotLength != 0 { keys["slotLength"] = .number(slotLength) }
        if !plated { keys["plated"] = .bool(false) }
        if radius != 0 { keys["radius"] = .number(radius) }
        if chamfer != 0 { keys["chamfer"] = .number(chamfer) }
        if !corners.isEmpty { keys["corners"] = .array(corners.map { .string($0) }) }
        return .object(keys)
    }

    init(pin: Int, x: Double, y: Double, width: Double, height: Double, shape: String = "rect",
         drill: Double = 0) {
        self.pin = pin
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.shape = shape
        self.drill = drill
    }

    /// A pad of an expanded footprint.
    init(_ pad: Footprint.Pad) {
        pin = pad.designator.flatMap { Int($0) } ?? 0
        x = pad.centre.x
        y = pad.centre.y
        width = pad.size.width
        height = pad.size.height
        shape = pad.shape == .round ? "round" : "rect"
        drill = pad.drill
        slotLength = pad.slotLength
        plated = pad.plated
        radius = pad.radius
        chamfer = pad.chamfer
        corners = Self.cornerNames.enumerated()
            .filter { pad.corners.contains(Footprint.Corners(rawValue: 1 << $0.offset)) }
            .map(\.element)
    }

    /// A pad entry of a file; nil when it is not in the expected shape.
    init?(json: JSONValue) {
        guard case let .object(keys) = json,
              case let .number(pin)? = keys["pin"], case let .number(x)? = keys["x"], case let .number(y)? = keys["y"],
              case let .number(width)? = keys["width"], case let .number(height)? = keys["height"] else { return nil }
        self.init(pin: Int(pin), x: x, y: y, width: width, height: height)
        if case let .string(shape)? = keys["shape"] { self.shape = shape }
        if case let .number(drill)? = keys["drill"] { self.drill = drill }
        if case let .number(slot)? = keys["slotLength"] { slotLength = slot }
        if case let .bool(plated)? = keys["plated"] { self.plated = plated }
        if case let .number(radius)? = keys["radius"] { self.radius = radius }
        if case let .number(chamfer)? = keys["chamfer"] { self.chamfer = chamfer }
        if case let .array(items)? = keys["corners"] {
            corners = items.compactMap { if case let .string(name) = $0 { return name } else { return nil } }
        }
        let known: Set<String> = ["pin", "x", "y", "width", "height", "shape", "drill", "slotLength", "plated",
                                  "radius", "chamfer", "corners"]
        guard Set(keys.keys).isSubset(of: known) else { return nil }
    }
}

/// A custom footprint for one part (plan-32-D): the `explicit` style, a body
/// and a pad table, stored in the part file's `footprints` (plan-32-A § 4.1),
/// never in `footprints.json`.
struct CustomFootprintDraft: Hashable, Sendable {
    var id: String
    var description: String
    var note: String? = nil
    var bodyWidth: Double
    var bodyHeight: Double
    var bodyRound = false
    var pads: [PadDraft]

    /// The id for a part's custom footprint: `CUSTOM-` and the first eight
    /// characters of its uuid (only A–Z, 0–9 and '-', as `valid_id` asks).
    static func id(forPart uuid: String) -> String {
        "CUSTOM-" + uuid.prefix(8).uppercased()
    }

    /// The number of pinout pins the pads cover (`pins` in the entry).
    var pinCount: Int {
        Set(pads.map(\.pin).filter { $0 > 0 }).count
    }

    /// The `footprints.json` entry (§ 4.2).
    func encoded() -> JSONValue {
        var body: [String: JSONValue] = ["width": .number(bodyWidth), "height": .number(bodyHeight)]
        if bodyRound { body["round"] = .bool(true) }
        var keys: [String: JSONValue] = [
            "id": .string(id), "description": .string(description), "style": .string("explicit"),
            "pins": .number(Double(pinCount)), "body": .object(body), "pads": .array(pads.map { $0.encoded() }),
        ]
        if let note { keys["note"] = .string(note) }
        return .object(keys)
    }

    init(id: String, description: String, bodyWidth: Double, bodyHeight: Double, pads: [PadDraft]) {
        self.id = id
        self.description = description
        self.bodyWidth = bodyWidth
        self.bodyHeight = bodyHeight
        self.pads = pads
    }

    /// The draft of an expanded footprint (any style: its pads are the
    /// expansion), under its own id.
    init(footprint: Footprint) {
        id = footprint.id
        description = footprint.description
        bodyWidth = footprint.body.width
        bodyHeight = footprint.body.height
        bodyRound = footprint.bodyRound
        pads = footprint.pads.map(PadDraft.init)
    }

    /// An `explicit` entry of a part file; nil for any other shape (it then
    /// stays in the file as it is).
    init?(json: JSONValue) {
        guard case let .object(keys) = json, keys["style"] == .string("explicit"),
              case let .string(id)? = keys["id"], case let .string(description)? = keys["description"],
              case let .object(body)? = keys["body"], case let .number(width)? = body["width"],
              case let .number(height)? = body["height"], case let .array(padItems)? = keys["pads"],
              Set(keys.keys).isSubset(of: ["id", "description", "note", "style", "pins", "body", "pads"]),
              Set(body.keys).isSubset(of: ["width", "height", "round"]) else { return nil }
        let pads = padItems.compactMap(PadDraft.init(json:))
        guard pads.count == padItems.count else { return nil }
        self.init(id: id, description: description, bodyWidth: width, bodyHeight: height, pads: pads)
        if case let .bool(round)? = body["round"] { bodyRound = round }
        if case let .string(note)? = keys["note"] { self.note = note }
        // The entry's own pin count must be the one this draft writes back.
        if case let .number(pins)? = keys["pins"], Int(pins) != pinCount { return nil }
    }

    /// A first footprint for a pinout of `pins` pins: a row of 2.54 mm
    /// through-hole pads, to edit from.
    static func starter(id: String, pins: Int) -> CustomFootprintDraft {
        let count = max(pins, 1)
        let pitch = 2.54
        let pads = (1...count).map { pin in
            PadDraft(pin: pin, x: (Double(pin) - Double(count + 1) / 2) * pitch, y: 0, width: 1.6, height: 1.6,
                     shape: pin == 1 ? "rect" : "round", drill: 0.8)
        }
        return CustomFootprintDraft(id: id, description: "Custom footprint", bodyWidth: Double(count) * pitch,
                                    bodyHeight: 2.54, pads: pads)
    }
}
