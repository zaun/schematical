import CoreGraphics
import Foundation

/// The `.esch` (sheet) and `.esym` (symbol) files of an EasyEDA Pro export
/// (`EasyEDAProExport`).
///
/// The schematic is in 0.01 inch, one canvas unit each, so the app's 10-unit
/// grid is EasyEDA's 0.1 inch. A sheet's y runs up (`-y` of the canvas); a
/// symbol's own frame has y down, the part's origin at the top-left of its
/// body, so a pin at canvas offset `(rx, ry)` from the origin is written at
/// `(rx, ry)` and the part at `(origin.x, -origin.y)`. (That is how EasyEDA's
/// files read, and how KiCad's importer places them.)
nonisolated final class EasyEDAProSchematicWriter {
    private let build: EasyEDAProExport.Build
    private let document: SchematicDocument
    private var ids = EasyEDAProExport.Build.Counter()

    @MainActor init(_ build: EasyEDAProExport.Build) {
        self.build = build
        document = build.document
    }

    // MARK: Styles

    private static let textStyle = "st1"
    private static let leftStyle = "st2"
    private static let rightStyle = "st3"
    private static let wireStyle = "st4"
    private static let busStyle = "st5"
    private static let bodyStyle = "st6"
    private static let noteStyle = "st7"

    /// The styles every file starts with. A font style is `[id, color, background,
    /// font, size, italic, bold, underline, strikethrough, vertical, horizontal]`;
    /// a line style `[id, color, dash, fill, width]`.
    @MainActor private static func styles() -> String {
        func font(_ id: String, horizontal: Int, vertical: Int = 1) -> String {
            EProJSON.line("FONTSTYLE", .string(id), "#000000", "", nil, 7, 0, 0, 0, 0, .int(vertical), .int(horizontal))
        }
        return font(textStyle, horizontal: 0) + font(leftStyle, horizontal: 0) + font(rightStyle, horizontal: 2)
            + EProJSON.line("LINESTYLE", .string(wireStyle), "#008800", 0, "", 1)
            + EProJSON.line("LINESTYLE", .string(busStyle), "#006666", 0, "", 4)
            + EProJSON.line("LINESTYLE", .string(bodyStyle), "#880000", 0, "#FFFFCC", 1)
            + font(noteStyle, horizontal: 0, vertical: 0)
    }

    @MainActor private func header(_ type: String, head: [(String, EProJSON)]) -> String {
        EProJSON.line("DOCTYPE", .string(type), "1.1")
            + EProJSON.line("HEAD", .object(head + [("editorVersion", "Schematical"), ("importFlag", 0)]))
            + Self.styles()
    }

    /// `["ATTR", id, parent, key, value, keyVisible, valueVisible, x, y, rotation, font, locked]`.
    /// An attribute that was never shown has no place.
    @MainActor private func attribute(_ key: String, _ value: String, parent: String = "", visible: Bool = false,
                           at point: CGPoint? = nil, rotation: Int = 0, style: String = textStyle) -> String {
        var row: [EProJSON] = ["ATTR", .string(ids.id()), .string(parent), .string(key), .string(value), 0,
                               visible ? 1 : 0]
        if let point { row += [.num(point.x), .num(point.y)] } else { row += [nil, nil] }
        row += [.int(rotation), .string(style), 0]
        return EProJSON.array(row).text + "\n"
    }

    // MARK: Names

    /// A supply's rail name: "GND", "5V", "3V3". Nil for any other part.
    @MainActor private static func rail(of chip: PlacedChip) -> String? {
        guard case let .supply(_, mv, _) = chip.part.details else { return nil }
        return KiCadExport.Context.railName(mv: mv)
    }

    /// The name EasyEDA shows for a pin: alternatives joined by `/`, an active-low one
    /// with a bar over it (a `~` starts and stops the bar).
    @MainActor private static func pinName(_ pin: ChipPin) -> String {
        if pin.name.isEmpty { return " " }
        guard pin.isActiveLow else { return pin.name }
        return pin.alternatives.map { alternative in
            guard alternative.isActiveLow else { return alternative.text }
            let bare = alternative.text.hasSuffix("#") ? String(alternative.text.dropLast()) : alternative.text
            return "~\(bare)~"
        }.joined(separator: "/")
    }

    @MainActor private static func electric(_ role: ChipPinRole) -> (code: Int, type: String?) {
        switch role {
        case .input: (1, "IN")
        case .output: (2, "OUT")
        case .inOut: (3, "BI")
        case .power, .ground, .noConnect, .absent: (0, nil)
        }
    }

    // MARK: Symbols

    /// A pull resistor's other pin: the footprint pad no pin names, on the far
    /// side from the signal pin.
    @MainActor private static func farPin(of chip: PlacedChip) -> (designator: String, tip: CGPoint, side: ChipLayout.Side)? {
        guard chip.symbol == .pullUp || chip.symbol == .pullDown, let slot = chip.layout.slots.first,
              let footprint = chip.footprint else { return nil }
        let named = Set(chip.part.pins.map(\.designator))
        guard let designator = footprint.pads.compactMap(\.designator).first(where: { !named.contains($0) }) else { return nil }
        let stub = ChipLayout.stubLength, size = chip.layout.size
        if slot.side == .bottom {
            return (designator, CGPoint(x: chip.origin.x + slot.centerX, y: chip.origin.y - stub), .top)
        }
        return (designator, CGPoint(x: chip.origin.x + slot.centerX, y: chip.origin.y + size.height + stub), .bottom)
    }

    /// Each unit of a part drawn as its units (plan-67), with its body rectangle
    /// relative to the part's origin (the corner of the box around them all).
    @MainActor private static func unitBoxes(of chip: PlacedChip) -> [(piece: PlacedChip, box: CGRect)] {
        let origin = chip.origin
        return chip.pieces.map { piece in
            (piece, bodyBox(of: piece).offsetBy(dx: piece.origin.x - origin.x, dy: piece.origin.y - origin.y))
        }
    }

    /// A part's body rectangle relative to its origin: a two-terminal part is just its
    /// pins' span (the app draws it smaller than its box).
    @MainActor private static func bodyBox(of chip: PlacedChip) -> CGRect {
        if chip.isSplit {
            // Drawn as its units (plan-67): the box around every unit's body.
            let boxes = unitBoxes(of: chip).map(\.box)
            return boxes.dropFirst().reduce(boxes.first ?? .zero) { $0.union($1) }
        }
        let size = chip.layout.size
        switch chip.symbol {
        case .diode, .plainDiode, .capacitor, .resistor, .inductor, .fuse, .buzzer:
            if chip.rotation == 90 || chip.rotation == 270 {
                let columns = chip.layout.slots.map(\.centerX)
                guard let low = columns.min(), let high = columns.max() else { return CGRect(origin: .zero, size: size) }
                return CGRect(x: low - 10, y: 0, width: high - low + 20, height: size.height)
            }
            let rows = chip.layout.slots.map(\.centerY)
            guard let low = rows.min(), let high = rows.max() else { return CGRect(origin: .zero, size: size) }
            return CGRect(x: 0, y: low - 10, width: size.width, height: high - low + 20)
        default:
            return CGRect(origin: .zero, size: size)
        }
    }

    /// The symbol and device of `chip`, made once: a netflag for a supply, else the part's
    /// symbol with the pins the app draws. Returns the symbol's id.
    @MainActor static func symbolID(for chip: PlacedChip, build: EasyEDAProExport.Build) -> String {
        if let rail = rail(of: chip), chip.symbol == .vcc || chip.symbol == .ground {
            return netFlagSymbol(rail: rail, build: build)
        }
        var key = chip.partNumber + (chip.part.uuid.map { "|" + $0 } ?? "")
        if chip.isSplit {
            // One symbol holding every unit where it is (plan-67): the arrangement is part of the key.
            key += "|units" + chip.unitPlacements.map {
                let dx = Int($0.origin.x - chip.origin.x), dy = Int($0.origin.y - chip.origin.y)
                return "|\($0.key)\($0.isFlipped ? "f" : "n")\($0.rotation)@\(dx),\(dy)"
            }.joined()
        } else {
            if chip.isFlipped { key += "|flipped" }
            if chip.rotation != 0 { key += "|r\(chip.rotation)" }
        }
        return build.symbol(key, title: chip.partNumber, type: 2) {
            EasyEDAProSchematicWriter(build).partSymbolFile(chip)
        }
    }

    /// The netflag symbol of a rail: one pin at its origin, the bar on the far end.
    @MainActor static func netFlagSymbol(rail: String, build: EasyEDAProExport.Build) -> String {
        build.symbol("netflag|\(rail)", title: rail, type: 18) {
            EasyEDAProSchematicWriter(build).netFlagFile(rail: rail)
        }
    }

    @MainActor private func netFlagFile(rail: String) -> String {
        let ground = rail == "GND"
        var out = header("SYMBOL", head: [("symbolType", 18)])
        out += EProJSON.line("PART", "", .object([("BBOX", .array([-10, ground ? 0 : -14, 10, ground ? 18 : 0]))]))
        let length = Double(ChipLayout.stubLength)
        let pin = ids.id()
        // Ground hangs below its pin (the body is at +y in a symbol's frame), a voltage stands above it.
        out += EProJSON.line("PIN", .string(pin), 1, 0, 0, 0, .num(length), ground ? 270 : 90, "#880000", 0, 0)
        out += attribute("NAME", rail, parent: pin)
        out += attribute("NUMBER", "1", parent: pin)
        let bars: [(Double, Double)] = ground ? [(10, 10), (14, 6), (18, 2)] : [(-10, 10)]
        for bar in bars {
            out += EProJSON.line("POLY", .string(ids.id()), .array([.num(-bar.1), .num(bar.0), .num(bar.1), .num(bar.0)]), 0,
                                 .string(Self.bodyStyle), 0)
        }
        out += attribute("Global Net Name", rail, visible: true, at: CGPoint(x: 0, y: ground ? 24 : -20))
        return out
    }

    /// A pin row, its name and number, in a symbol's frame.
    @MainActor private func pinLines(designator: String, name: String, role: ChipPinRole, at tip: CGPoint, side: ChipLayout.Side,
                          box: CGRect, style: String? = nil) -> String {
        let pin = ids.id()
        let rotation: Int
        switch side {
        case .left: rotation = 0
        case .right: rotation = 180
        case .top: rotation = 270
        case .bottom: rotation = 90
        }
        let kind = Self.electric(role)
        var out = EProJSON.line("PIN", .string(pin), 1, .int(kind.code), .num(tip.x), .num(tip.y),
                                .num(Double(ChipLayout.stubLength)), .int(rotation), "#880000", 0, 0)
        // The name just inside the body, the number over the pin.
        let inside: CGPoint, number: CGPoint, nameStyle: String, turn: Int
        switch side {
        case .left:
            inside = CGPoint(x: box.minX + 4, y: tip.y); number = CGPoint(x: tip.x + 1, y: tip.y - 6)
            nameStyle = Self.leftStyle; turn = 0
        case .right:
            inside = CGPoint(x: box.maxX - 4, y: tip.y); number = CGPoint(x: tip.x - 1, y: tip.y - 6)
            nameStyle = Self.rightStyle; turn = 0
        case .top:
            inside = CGPoint(x: tip.x, y: box.minY + 4); number = CGPoint(x: tip.x - 6, y: tip.y + 1)
            nameStyle = Self.rightStyle; turn = 90
        case .bottom:
            inside = CGPoint(x: tip.x, y: box.maxY - 4); number = CGPoint(x: tip.x - 6, y: tip.y - 1)
            nameStyle = Self.leftStyle; turn = 90
        }
        out += attribute("NAME", name, parent: pin, visible: name != " ", at: inside, rotation: turn, style: nameStyle)
        out += attribute("NUMBER", designator, parent: pin, visible: true, at: number, style: Self.textStyle)
        if let type = kind.type { out += attribute("Pin Type", type, parent: pin) }
        if role == .noConnect { out += attribute("NO_CONNECT", "yes", parent: pin) }
        return out
    }

    @MainActor private func partSymbolFile(_ chip: PlacedChip) -> String {
        let box = Self.bodyBox(of: chip)
        var out = header("SYMBOL", head: [("symbolType", 2)])
        out += EProJSON.line("PART", "", .object([("BBOX", .array([.num(box.minX - 10), .num(box.minY), .num(box.maxX + 10),
                                                                   .num(box.maxY)]))]))
        let prefix = String(chip.reference.prefix { $0.isLetter })
        out += attribute("Designator", (prefix.isEmpty ? "U" : prefix) + "?", visible: true,
                         at: CGPoint(x: box.minX, y: box.minY - 8))
        out += attribute("Name", chip.partNumber)
        if chip.isSplit {
            // Drawn as its units (plan-67): a rectangle per unit, each unit's pins on its own rectangle.
            for (piece, unitBox) in Self.unitBoxes(of: chip) {
                out += EProJSON.line("RECT", .string(ids.id()), .num(unitBox.minX), .num(unitBox.minY), .num(unitBox.maxX),
                                     .num(unitBox.maxY), 0, 0, 0, .string(Self.bodyStyle), 0)
                for slot in piece.layout.slots {
                    let tip = piece.tip(of: slot)
                    out += pinLines(designator: slot.pin.designator, name: Self.pinName(slot.pin), role: slot.pin.role,
                                    at: CGPoint(x: tip.x - chip.origin.x, y: tip.y - chip.origin.y), side: slot.side,
                                    box: unitBox)
                }
            }
            return out
        }
        out += EProJSON.line("RECT", .string(ids.id()), .num(box.minX), .num(box.minY), .num(box.maxX), .num(box.maxY),
                             0, 0, 0, .string(Self.bodyStyle), 0)
        for slot in chip.layout.slots {
            let tip = chip.tip(of: slot)
            out += pinLines(designator: slot.pin.designator, name: Self.pinName(slot.pin), role: slot.pin.role,
                            at: CGPoint(x: tip.x - chip.origin.x, y: tip.y - chip.origin.y), side: slot.side, box: box)
        }
        if let far = Self.farPin(of: chip) {
            out += pinLines(designator: far.designator, name: " ", role: .power,
                            at: CGPoint(x: far.tip.x - chip.origin.x, y: far.tip.y - chip.origin.y), side: far.side, box: box)
        }
        return out
    }

    // MARK: Sheet

    /// The `.esch` of the one sheet.
    @MainActor func sheet() -> String {
        var out = header("SCH", head: [("ORIGIN_X", 0), ("ORIGIN_Y", 0)])
        let mergedNet = mergedNets()

        // Parts. A pull resistor also gets a netflag on its far pin.
        for chip in document.chips {
            let assets = build.assets(for: chip)
            let origin = chip.origin
            let componentID = ids.id()
            if let rail = Self.rail(of: chip), chip.symbol == .vcc || chip.symbol == .ground,
               let slot = chip.layout.slots.first {
                let tip = chip.tip(of: slot)
                out += component(componentID, at: tip)
                out += attribute("Device", assets.device, parent: componentID)
                out += attribute("Symbol", assets.symbol, parent: componentID)
                out += attribute("Global Net Name", rail, parent: componentID, visible: true,
                                 at: CGPoint(x: tip.x, y: -tip.y + (chip.symbol == .ground ? -24 : 20)))
                continue
            }
            out += component(componentID, at: origin)
            out += attribute("Device", assets.device, parent: componentID)
            out += attribute("Symbol", assets.symbol, parent: componentID)
            if let footprint = assets.footprint { out += attribute("Footprint", footprint, parent: componentID) }
            let box = Self.bodyBox(of: chip)
            out += attribute("Designator", chip.reference, parent: componentID, visible: chip.showsReference,
                             at: CGPoint(x: origin.x + box.minX, y: -(origin.y + box.minY - 8)))
            let value = chip.ohms.map { ResistorValue.text($0) } ?? chip.farads.map { CapacitorValue.text($0) }
            if let value {
                out += attribute("Value", value, parent: componentID, visible: true,
                                 at: CGPoint(x: origin.x + box.minX, y: -(origin.y + box.maxY + 14)))
            }
            if let far = Self.farPin(of: chip), let rail = Self.rail(of: chip) {
                let flagSymbol = Self.netFlagSymbol(rail: rail, build: build)
                let flagDevice = build.device("netflag|\(rail)", title: rail, description: "Power symbol \(rail)",
                                              attributes: [("Symbol", flagSymbol), ("Global Net Name", rail)])
                let flagID = ids.id()
                out += component(flagID, at: far.tip)
                out += attribute("Device", flagDevice, parent: flagID)
                out += attribute("Symbol", flagSymbol, parent: flagID)
                out += attribute("Global Net Name", rail, parent: flagID, visible: true,
                                 at: CGPoint(x: far.tip.x, y: -far.tip.y + (rail == "GND" ? -24 : 20)))
            }
        }
        out += edgeConnector()

        // Wires: one line each, and the name of its net.
        var shown = Set<Int>()
        for wire in document.wires {
            guard let start = document.endTip(wire.start), let end = document.endTip(wire.end) else { continue }
            let route = [start] + wire.interior + [end]
            let flat: [EProJSON] = route.flatMap { [EProJSON.num($0.x), EProJSON.num(-$0.y)] }
            let id = ids.id()
            out += EProJSON.line("WIRE", .string(id), .array([.array(flat)]), .string(Self.wireStyle), 0)
            let net = netName(of: wire, merged: mergedNet)
            // A label's name is written where the label is.
            var point: CGPoint?
            for endpoint in [wire.start, wire.end] {
                switch endpoint {
                case let .label(labelID):
                    if let label = document.netLabels.first(where: { $0.id == labelID }), label.isNet, label.name == net.name {
                        point = label.tip
                    }
                case let .busPin(ref):
                    if build.context.busText[ref.wireID] == net.name, let tip = document.endTip(endpoint) { point = tip }
                default: break
                }
            }
            let key = net.id
            let first = shown.insert(key).inserted
            out += attribute("NET", net.name, parent: id, visible: point != nil && first,
                             at: point.map { CGPoint(x: $0.x + 2, y: -$0.y + 6) })
        }

        out += drawing()
        return out
    }

    /// `["COMPONENT", id, part, x, y, rotation, mirror, {}, locked]`, the part
    /// whole and upright: a flipped part is a symbol of its own.
    @MainActor private func component(_ id: String, at origin: CGPoint) -> String {
        EProJSON.line("COMPONENT", .string(id), "", .num(origin.x), .num(-origin.y), 0, 0, .object([]), 0)
    }

    /// A wire's net, merged with the rail it is on, and the name EasyEDA reads it by.
    @MainActor private func netName(of wire: Wire, merged: [Int: Int]) -> (id: Int, name: String) {
        let unmerged = build.context.netlist.netOfWire[wire.id] ?? -1
        let net = merged[unmerged] ?? unmerged
        return (net, build.netName(net) ?? "NC-\(unmerged + 1)")
    }

    /// Each net's number once rails are one net: what the board calls it.
    @MainActor private func mergedNets() -> [Int: Int] {
        var merged: [Int: Int] = [:]
        let netlist = build.context.netlist
        for connection in netlist.connections {
            let id = netlist.chipIDs[connection.part]
            if let net = build.context.copper.pinNets["\(id.uuidString)/\(connection.pin)"] { merged[connection.net] = net }
        }
        return merged
    }

    // MARK: Drawing

    /// Bus lines and ports as plain lines, and the text blocks.
    @MainActor private func drawing() -> String {
        var out = ""
        func poly(_ points: [CGPoint], closed: Bool, style: String) -> String {
            let flat: [EProJSON] = points.flatMap { [EProJSON.num($0.x), EProJSON.num(-$0.y)] }
            return EProJSON.line("POLY", .string(ids.id()), .array(flat), closed ? 1 : 0, .string(style), 0)
        }
        for bus in document.buses {
            for line in bus.lines { out += poly([line.a, line.b], closed: false, style: Self.busStyle) }
            for port in bus.ports {
                let along = bus.extent(of: port)
                guard let a = bus.point(on: port, along: along.lowerBound, out: 0),
                      let b = bus.point(on: port, along: along.upperBound, out: 0),
                      let c = bus.point(on: port, along: along.upperBound, out: BusPort.depth),
                      let d = bus.point(on: port, along: along.lowerBound, out: BusPort.depth) else { continue }
                out += poly([a, b, c, d], closed: true, style: Self.bodyStyle)
            }
        }
        for note in document.notes {
            let rect = note.rect
            if note.framed {
                out += poly([CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                             CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)],
                            closed: true, style: Self.wireStyle)
            }
            out += EProJSON.line("TEXT", .string(ids.id()), .num(rect.minX + SchematicNote.padding),
                                 .num(-(rect.minY + SchematicNote.padding)), 0, .string(note.text), .string(Self.noteStyle), 0)
        }
        // The title block (plan-64-D): the frame, the lines inside it, and each cell's label and value.
        if let layout = document.titleBlockLayout {
            let rect = layout.rect
            out += poly([CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                         CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)],
                        closed: true, style: Self.wireStyle)
            for line in layout.separators { out += poly([line.0, line.1], closed: false, style: Self.wireStyle) }
            func text(_ string: String, at point: CGPoint) -> String {
                EProJSON.line("TEXT", .string(ids.id()), .num(point.x), .num(-point.y), 0, .string(string),
                              .string(Self.noteStyle), 0)
            }
            for cell in layout.cells {
                out += text(cell.label, at: cell.labelOrigin)
                for (index, line) in cell.lines.enumerated() where !line.isEmpty {
                    out += text(line, at: CGPoint(x: cell.textOrigin.x, y: cell.textOrigin.y + CGFloat(index) * cell.lineHeight))
                }
            }
        }
        return out
    }

    // MARK: Card connector

    /// The card's connector as a symbol beside the drawing, one pin per connector pin. A
    /// pin on a net has a short wire named for it, or a netflag when the net is a rail.
    @MainActor private func edgeConnector() -> String {
        let pins = build.context.edgePins
        guard !pins.isEmpty else { return "" }
        let board = document.board
        let pitch = ChipLayout.pitch, top = ChipLayout.headerHeight
        let size = CGSize(width: ChipLayout.bodyWidth, height: top + pitch * CGFloat(pins.count))
        var right: CGFloat = 0
        for chip in document.chips { right = max(right, chip.hitRect.maxX) }
        for bus in document.buses { for line in bus.lines { right = max(right, line.a.x, line.b.x) } }
        let origin = CGPoint(x: (right / 10).rounded(.up) * 10 + 160, y: 0)
        let box = CGRect(origin: .zero, size: size)
        let stub = Double(ChipLayout.stubLength)
        func tip(_ row: Int) -> CGPoint { CGPoint(x: -stub, y: top + pitch * CGFloat(row) + pitch / 2) }

        let name = board.connectorName
        let symbolID = build.symbol("edge|\(name)|\(pins.count)", title: name, type: 2) {
            var out = header("SYMBOL", head: [("symbolType", 2)])
            out += EProJSON.line("PART", "", .object([("BBOX", .array([-10, 0, .num(size.width + 10), .num(size.height)]))]))
            out += attribute("Designator", "J?", visible: true, at: CGPoint(x: 0, y: -8))
            out += attribute("Name", name)
            out += EProJSON.line("RECT", .string(ids.id()), 0, 0, .num(size.width), .num(size.height), 0, 0, 0,
                                 .string(Self.bodyStyle), 0)
            for (row, pin) in pins.enumerated() {
                out += pinLines(designator: pin.name, name: pin.name, role: .inOut, at: tip(row), side: .left, box: box)
            }
            return out
        }
        let deviceID = build.device("edge|\(name)|\(pins.count)", title: name, description: "\(name) connector",
                                    attributes: [("Symbol", symbolID), ("Designator", "J?"), ("Name", name)])
        var out = ""
        let componentID = ids.id()
        out += component(componentID, at: origin)
        out += attribute("Device", deviceID, parent: componentID)
        out += attribute("Symbol", symbolID, parent: componentID)
        out += attribute("Designator", name.uppercased() + "1", parent: componentID, visible: true,
                         at: CGPoint(x: origin.x, y: -(origin.y - 8)))
        for (row, pin) in pins.enumerated() {
            guard let net = pin.net else { continue }
            let world = CGPoint(x: origin.x + tip(row).x, y: origin.y + tip(row).y)
            if let rail = build.context.rail(ofNet: net) {
                let flagSymbol = Self.netFlagSymbol(rail: rail, build: build)
                let flagDevice = build.device("netflag|\(rail)", title: rail, description: "Power symbol \(rail)",
                                              attributes: [("Symbol", flagSymbol), ("Global Net Name", rail)])
                let flagID = ids.id()
                out += component(flagID, at: world)
                out += attribute("Device", flagDevice, parent: flagID)
                out += attribute("Symbol", flagSymbol, parent: flagID)
                out += attribute("Global Net Name", rail, parent: flagID, visible: true,
                                 at: CGPoint(x: world.x, y: -world.y + (rail == "GND" ? -24 : 20)))
            } else if let text = build.netName(net) {
                let id = ids.id()
                let stubStart = CGPoint(x: world.x - 30, y: world.y)
                out += EProJSON.line("WIRE", .string(id), .array([.array([.num(world.x), .num(-world.y), .num(stubStart.x),
                                                                          .num(-stubStart.y)])]), .string(Self.wireStyle), 0)
                out += attribute("NET", text, parent: id, visible: true, at: CGPoint(x: stubStart.x + 2, y: -stubStart.y + 6))
            }
        }
        return out
    }
}
