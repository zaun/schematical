import CoreGraphics
import Foundation

/// The `.kicad_sch` of a KiCad export (`KiCadExport`): KiCad 8's schematic
/// format, one sheet.
///
/// Canvas units become millimetres at `KiCadExport.mmPerUnit` (the app's
/// grid is KiCad's 1.27 mm), moved so the drawing sits 25.4 mm inside the
/// sheet. A part's symbol is generated from the pins the app draws: its
/// body rectangle, each pin where its wire meets the part, so every wire
/// end lands on a KiCad pin end.
@MainActor
struct KiCadSchematicWriter {
    private let context: KiCadExport.Context
    private let document: SchematicDocument
    private let k = KiCadExport.mmPerUnit
    /// Canvas units added to every point, so the drawing is inside the sheet.
    private let shift: CGPoint
    private let sheet: CGSize

    init(_ context: KiCadExport.Context) {
        self.context = context
        document = context.document
        let box = Self.extent(of: context.document)
        let margin: CGFloat = 200
        // Whole grid steps, so everything stays on KiCad's grid.
        shift = CGPoint(x: margin - (box.minX / 10).rounded(.down) * 10,
                        y: margin - (box.minY / 10).rounded(.down) * 10)
        sheet = CGSize(width: (box.maxX + shift.x + margin) * KiCadExport.mmPerUnit,
                       height: (box.maxY + shift.y + margin) * KiCadExport.mmPerUnit)
    }

    // MARK: Coordinates

    /// A canvas point on the sheet, as "x y" in mm.
    private func xy(_ point: CGPoint) -> String {
        "\(KiCad.n((point.x + shift.x) * k)) \(KiCad.n((point.y + shift.y) * k))"
    }

    /// A canvas offset (a point relative to a symbol's origin) in the
    /// library's frame, where y grows up.
    private func local(_ offset: CGPoint) -> String {
        "\(KiCad.n(offset.x * k)) \(KiCad.n(-offset.y * k))"
    }

    private func id(_ key: String) -> String { KiCad.uuid("sch/\(context.base)/\(key)") }

    /// The box of everything drawn, in canvas units.
    private static func extent(of document: SchematicDocument) -> CGRect {
        var box = CGRect.null
        func add(_ rect: CGRect) { box = box.union(rect) }
        func add(_ point: CGPoint) { box = box.union(CGRect(x: point.x, y: point.y, width: 0, height: 0)) }
        for chip in document.chips { add(chip.hitRect.insetBy(dx: -20, dy: -30)) }
        for wire in document.wires {
            for end in [wire.start, wire.end] { if let tip = document.endTip(end) { add(tip) } }
            wire.interior.forEach { add($0) }
        }
        for label in document.netLabels { add(label.tip); add(CGPoint(x: label.tip.x + NetLabel.length, y: label.tip.y)) }
        for note in document.notes { add(note.rect) }
        if let block = document.titleBlockRect { add(block) }
        for bus in document.buses {
            for line in bus.lines {
                add(line.a); add(line.b)
                add(CGPoint(x: line.a.x - BusPort.depth, y: line.a.y - BusPort.depth))
                add(CGPoint(x: line.b.x + BusPort.depth, y: line.b.y + BusPort.depth))
            }
        }
        return box.isNull ? CGRect(x: 0, y: 0, width: 200, height: 200) : box
    }

    // MARK: File

    func text() -> String {
        var symbols = SymbolLibrary()
        var body: [String] = []
        let chips = document.chips

        // Symbol instances, with a power symbol on every pull resistor's far
        // pin, and one power flag on each rail (the pins are power inputs; a
        // flag is the output KiCad's ERC wants on the net).
        var powerNumber = 0
        var flagged = Set<String>()
        func power(_ rail: String, at tip: CGPoint, seed: String) {
            powerNumber += 1
            body.append(powerInstance(rail, at: tip, seed: seed, number: powerNumber, symbols: &symbols))
            if flagged.insert(rail).inserted {
                body.append(flagInstance(at: tip, number: flagged.count, rail: rail, symbols: &symbols))
            }
        }
        for chip in chips {
            if let supply = supplyName(of: chip), chip.symbol == .vcc || chip.symbol == .ground {
                power(supply, at: chip.tip(of: chip.layout.slots[0]), seed: chip.id.uuidString)
            } else {
                body.append(instance(of: chip, symbols: &symbols))
                if let far = farPin(of: chip), let rail = supplyName(of: chip) {
                    power(rail, at: far.tip, seed: "\(chip.id)/far")
                }
            }
        }

        var drawing: [String] = []
        let edge = edgeConnector(symbols: &symbols, powerNumber: &powerNumber, flagged: &flagged)
        body += edge.symbol
        // The drawing's own Power Flags (plan-63-A), numbered after the rails'.
        var flagNumber = flagged.count
        for label in document.netLabels where label.isPowerFlag {
            flagNumber += 1
            body.append(flagInstance(at: label.tip, number: flagNumber, rail: "label/\(label.id)", symbols: &symbols))
        }
        drawing += edge.drawing
        drawing += wires()
        drawing += labels()
        drawing += buses()
        drawing += notes()
        drawing += titleBlock()

        let page = KiCad.paper(width: sheet.width, height: sheet.height)
        var out = """
        (kicad_sch
        \t(version 20231120)
        \t(generator "schematical")
        \t(generator_version "8.0")
        \t(uuid \(KiCad.q(context.rootUUID)))
        \t\(page)
        \t(title_block
        \(titleBlockFields())
        \t)
        \t(lib_symbols

        """
        out += symbols.definitions.joined(separator: "\n") + "\n\t)\n"
        out += drawing.joined(separator: "\n") + "\n"
        out += body.joined(separator: "\n") + "\n"
        out += "\t(sheet_instances\n\t\t(path \"/\"\n\t\t\t(page \"1\")\n\t\t)\n\t)\n)\n"
        return out
    }

    // MARK: Card connector

    /// The card's connector as a symbol with one pin per connector pin,
    /// beside the drawing, so the board's connector footprint has a part to
    /// belong to. A pin on a net carries it: a label with the net's label
    /// text, or a power symbol on a rail.
    private func edgeConnector(symbols: inout SymbolLibrary, powerNumber: inout Int,
                               flagged: inout Set<String>) -> (symbol: [String], drawing: [String]) {
        let pins = context.edgePins
        guard !pins.isEmpty else { return ([], []) }
        let board = document.board
        let name = "EDGE_" + KiCad.libraryName(board.connectorName)
        let reference = board.connectorName.uppercased() + "1"
        let pitch = ChipLayout.pitch, header = ChipLayout.headerHeight
        let size = CGSize(width: ChipLayout.bodyWidth, height: header + pitch * CGFloat(pins.count))
        // To the right of everything else, on the grid.
        let box = Self.extent(of: document)
        let origin = CGPoint(x: (box.maxX / 10).rounded(.up) * 10 + 160, y: (box.minY / 10).rounded(.down) * 10)
        func tip(_ row: Int) -> CGPoint {
            CGPoint(x: origin.x - ChipLayout.stubLength, y: origin.y + header + pitch * CGFloat(row) + pitch / 2)
        }
        var libraryPins: [String] = []
        for (row, entry) in pins.enumerated() {
            let t = tip(row)
            libraryPins.append(pin(type: "passive", at: CGPoint(x: t.x - origin.x, y: t.y - origin.y), side: .left,
                                   name: entry.name, number: entry.name))
        }
        if symbols.names.insert(name).inserted {
            symbols.definitions.append("""
            \t\t(symbol \(KiCad.q("\(KiCadExport.library):\(name)"))
            \t\t\t(pin_names (offset 0.508))
            \t\t\t(exclude_from_sim no)
            \t\t\t(in_bom no)
            \t\t\t(on_board yes)
            \(property("Reference", "J", at: CGPoint(x: 0, y: 1.27)))
            \(property("Value", board.connectorName, at: CGPoint(x: 0, y: -(size.height * k + 1.27))))
            \(property("Footprint", "", at: .zero, hide: true))
            \(property("Datasheet", "", at: .zero, hide: true))
            \t\t\t(symbol \(KiCad.q("\(name)_0_1"))
            \t\t\t\t(rectangle
            \t\t\t\t\t(start 0 0)
            \t\t\t\t\t(end \(KiCad.n(size.width * k)) \(KiCad.n(-size.height * k)))
            \t\t\t\t\t(stroke (width 0.254) (type default))
            \t\t\t\t\t(fill (type background))
            \t\t\t\t)
            \t\t\t)
            \t\t\t(symbol \(KiCad.q("\(name)_1_1"))
            \(libraryPins.joined(separator: "\n"))
            \t\t\t)
            \t\t)
            """)
        }
        let at = CGPoint(x: (origin.x + shift.x) * k, y: (origin.y + shift.y) * k)
        var instance = """
        \t(symbol
        \t\t(lib_id \(KiCad.q("\(KiCadExport.library):\(name)")))
        \t\t(at \(KiCad.n(at.x)) \(KiCad.n(at.y)) 0)
        \t\t(unit 1)
        \t\t(exclude_from_sim no)
        \t\t(in_bom no)
        \t\t(on_board yes)
        \t\t(dnp no)
        \t\t(uuid \(KiCad.q(context.edgeSymbolUUID)))
        \(property("Reference", reference, at: CGPoint(x: at.x, y: at.y - 2.54), justify: "left bottom"))
        \(property("Value", board.connectorName, at: CGPoint(x: at.x, y: at.y + (size.height + 20) * k), justify: "left top"))
        \(property("Footprint", "\(KiCadExport.library):\(board.connectorName)", at: at, hide: true))
        \(property("Datasheet", "", at: at, hide: true))

        """
        for entry in pins {
            instance += "\t\t(pin \(KiCad.q(entry.name)) (uuid \(KiCad.q(id("edgepin/\(entry.name)")))))\n"
        }
        instance += instances(reference: reference) + "\t)"

        var out: [String] = [], extra: [String] = []
        for (row, entry) in pins.enumerated() {
            guard let net = entry.net else { continue }
            if let rail = context.rail(ofNet: net) {
                powerNumber += 1
                extra.append(powerInstance(rail, at: tip(row), seed: "edge/\(entry.name)", number: powerNumber,
                                           symbols: &symbols))
                if flagged.insert(rail).inserted {
                    extra.append(flagInstance(at: tip(row), number: flagged.count, rail: rail, symbols: &symbols))
                }
            } else if let text = context.labelText[net] {
                out.append("""
                \t(label \(KiCad.q(text))
                \t\t(at \(xy(tip(row))) 180)
                \t\t(effects (font (size 1.27 1.27)) (justify right bottom))
                \t\t(uuid \(KiCad.q(id("edgelabel/\(entry.name)"))))
                \t)
                """)
            }
        }
        return ([instance] + extra, out)
    }

    // MARK: Symbols

    /// The library symbols written so far, by library name.
    private struct SymbolLibrary {
        var names: Set<String> = []
        var definitions: [String] = []
    }

    /// A supply's rail name: "GND", "5V", "3V3". Nil for any other part.
    private func supplyName(of chip: PlacedChip) -> String? {
        guard case let .supply(_, mv, _) = chip.part.details else { return nil }
        return mv == 0 ? "GND" : SupplyChoice.name(volts: Double(mv) / 1000) ?? "\(mv)mV"
    }

    /// A pull resistor's pin the app doesn't have: the pad of its footprint
    /// that no pin names, drawn on the far edge from the signal pin.
    private func farPin(of chip: PlacedChip) -> (designator: String, tip: CGPoint, side: ChipLayout.Side)? {
        guard chip.symbol == .pullUp || chip.symbol == .pullDown,
              let slot = chip.layout.slots.first,
              let footprint = chip.footprint else { return nil }
        let named = Set(chip.part.pins.map(\.designator))
        guard let designator = footprint.pads.compactMap(\.designator).first(where: { !named.contains($0) }) else {
            return nil
        }
        let stub = ChipLayout.stubLength, size = chip.layout.size
        if slot.side == .bottom {
            return (designator, CGPoint(x: chip.origin.x + slot.centerX, y: chip.origin.y - stub), .top)
        }
        return (designator, CGPoint(x: chip.origin.x + slot.centerX, y: chip.origin.y + size.height + stub), .bottom)
    }

    /// The library name of a part's symbol; a flipped part is its own symbol.
    private func libraryName(of chip: PlacedChip) -> String {
        var name = KiCad.libraryName(chip.partNumber)
        if chip.part.uuid != nil { name += "_" + String((chip.part.uuid ?? "").prefix(8)) }
        if chip.isFlipped { name += "_flipped" }
        if chip.rotation != 0 { name += "_r\(chip.rotation)" }
        return name
    }

    /// The KiCad pin type of a part's pin: the electrical rules check's type
    /// (`erc_pin_type`: passive, tri_state, open_collector, power_out, …) when the
    /// document has the part library, else the pin's role.
    private func pinType(of chip: PlacedChip, _ pin: ChipPin) -> String {
        if let library = document.library, let part = part_library_find(library.pointer, chip.partNumber) {
            var type = ERC_INPUT
            if erc_pin_type(part, pin.index, &type) != 0 {
                return String(cString: erc_pin_type_name(type))
            }
        }
        return pinType(pin.role)
    }

    private func pinType(_ role: ChipPinRole) -> String {
        switch role {
        case .input: "input"
        case .output: "output"
        case .inOut: "bidirectional"
        case .power, .ground: "power_in"
        case .noConnect: "no_connect"
        case .absent: "passive"
        }
    }

    /// The pin's line in a library symbol: it ends at `at`, its other end
    /// `length` towards the body.
    private func pin(type: String, at: CGPoint, side: ChipLayout.Side, name: String, number: String) -> String {
        // KiCad's pin angle points from the connection point into the body.
        let angle: Int
        switch side {
        case .left: angle = 0
        case .right: angle = 180
        case .top: angle = 270
        case .bottom: angle = 90
        }
        let length = KiCad.n(ChipLayout.stubLength * k)
        return """
        \t\t\t\t(pin \(type) line
        \t\t\t\t\t(at \(local(at)) \(angle))
        \t\t\t\t\t(length \(length))
        \t\t\t\t\t(name \(KiCad.q(name)) (effects (font (size 1 1))))
        \t\t\t\t\t(number \(KiCad.q(number)) (effects (font (size 1 1))))
        \t\t\t\t)
        """
    }

    private func property(_ name: String, _ value: String, at point: CGPoint, hide: Bool = false,
                          justify: String? = nil, size: Double = 1.27) -> String {
        let justification = justify.map { " (justify \($0))" } ?? ""
        return "\t\t(property \(KiCad.q(name)) \(KiCad.q(value)) (at \(KiCad.n(point.x)) \(KiCad.n(point.y)) 0)\n"
            + "\t\t\t(effects (font (size \(KiCad.n(size)) \(KiCad.n(size))))\(justification)\(hide ? " hide" : ""))\n\t\t)"
    }

    /// The library definition of a part's symbol, written once.
    private func define(_ chip: PlacedChip, name: String, in symbols: inout SymbolLibrary) {
        guard symbols.names.insert(name).inserted else { return }
        let size = chip.layout.size
        let box = bodyBox(of: chip)
        var pins: [String] = []
        for slot in chip.layout.slots {
            let tip = chip.tip(of: slot)
            pins.append(pin(type: pinType(of: chip, slot.pin), at: CGPoint(x: tip.x - chip.origin.x, y: tip.y - chip.origin.y),
                            side: slot.side, name: KiCad.pinName(slot.pin), number: slot.pin.designator))
        }
        if let far = farPin(of: chip) {
            pins.append(pin(type: "passive", at: CGPoint(x: far.tip.x - chip.origin.x, y: far.tip.y - chip.origin.y),
                            side: far.side, name: "~", number: far.designator))
        }
        let prefix = referencePrefix(of: chip)
        symbols.definitions.append("""
        \t\t(symbol \(KiCad.q("\(KiCadExport.library):\(name)"))
        \t\t\t(pin_names (offset 0.508))
        \t\t\t(exclude_from_sim no)
        \t\t\t(in_bom yes)
        \t\t\t(on_board yes)
        \(property("Reference", prefix, at: CGPoint(x: 0, y: 1.27)))
        \(property("Value", chip.partNumber, at: CGPoint(x: 0, y: -(size.height * k + 1.27))))
        \(property("Footprint", "", at: .zero, hide: true))
        \(property("Datasheet", "", at: .zero, hide: true))
        \t\t\t(symbol \(KiCad.q("\(name)_0_1"))
        \t\t\t\t(rectangle
        \t\t\t\t\t(start \(local(CGPoint(x: box.minX, y: box.minY))))
        \t\t\t\t\t(end \(local(CGPoint(x: box.maxX, y: box.maxY))))
        \t\t\t\t\t(stroke (width 0.254) (type default))
        \t\t\t\t\t(fill (type background))
        \t\t\t\t)
        \t\t\t)
        \t\t\t(symbol \(KiCad.q("\(name)_1_1"))
        \(pins.joined(separator: "\n"))
        \t\t\t)
        \t\t)
        """)
    }

    /// A part's body rectangle relative to its origin. A two-terminal part
    /// (diode, capacitor, resistor) is drawn as a small symbol inside a taller
    /// box that neighbours overlap, so its body is just its pins' span.
    private func bodyBox(of chip: PlacedChip) -> CGRect {
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

    /// The letters of a reference, as the placed part has them ("U1" → "U").
    private func referencePrefix(of chip: PlacedChip) -> String {
        let letters = chip.reference.prefix { $0.isLetter }
        return letters.isEmpty ? "U" : String(letters)
    }

    /// A part's instance on the sheet.
    private func instance(of chip: PlacedChip, symbols: inout SymbolLibrary) -> String {
        let name = libraryName(of: chip)
        define(chip, name: name, in: &symbols)
        let value: String
        if let ohms = chip.ohms {
            value = ResistorValue.text(ohms)
        } else if let farads = chip.farads {
            value = CapacitorValue.text(farads)
        } else {
            value = chip.partNumber
        }
        let footprint = chip.footprint.map { "\(KiCadExport.library):\(KiCad.libraryName($0.id))" } ?? ""
        let origin = CGPoint(x: (chip.origin.x + shift.x) * k, y: (chip.origin.y + shift.y) * k)
        let box = bodyBox(of: chip)
        let referenceAt = CGPoint(x: origin.x, y: origin.y + (box.minY - 20) * k)
        let valueAt = CGPoint(x: origin.x, y: origin.y + (box.maxY + 20) * k)
        var out = """
        \t(symbol
        \t\t(lib_id \(KiCad.q("\(KiCadExport.library):\(name)")))
        \t\t(at \(KiCad.n(origin.x)) \(KiCad.n(origin.y)) 0)
        \t\t(unit 1)
        \t\t(exclude_from_sim no)
        \t\t(in_bom yes)
        \t\t(on_board \(chip.footprint == nil ? "no" : "yes"))
        \t\t(dnp no)
        \t\t(uuid \(KiCad.q(chip.id.uuidString.lowercased())))
        \(property("Reference", chip.reference, at: referenceAt, hide: !chip.showsReference, justify: "left bottom"))
        \(property("Value", value, at: valueAt, justify: "left top"))
        \(property("Footprint", footprint, at: origin, hide: true))
        \(property("Datasheet", chip.part.datasheetURL?.absoluteString ?? "", at: origin, hide: true))

        """
        for slot in chip.layout.slots {
            out += "\t\t(pin \(KiCad.q(slot.pin.designator)) (uuid \(KiCad.q(id("pin/\(chip.id)/\(slot.pin.designator)")))))\n"
        }
        if let far = farPin(of: chip) {
            out += "\t\t(pin \(KiCad.q(far.designator)) (uuid \(KiCad.q(id("pin/\(chip.id)/\(far.designator)")))))\n"
        }
        out += instances(reference: chip.reference)
        out += "\t)"
        return out
    }

    private func instances(reference: String) -> String {
        """
        \t\t(instances
        \t\t\t(project \(KiCad.q(context.base))
        \t\t\t\t(path \(KiCad.q("/" + context.rootUUID))
        \t\t\t\t\t(reference \(KiCad.q(reference))) (unit 1)
        \t\t\t\t)
        \t\t\t)
        \t\t)

        """
    }

    /// A power symbol (`#PWR<n>`) of rail `rail` with its pin at `point`.
    private func powerInstance(_ rail: String, at point: CGPoint, seed: String, number: Int,
                               symbols: inout SymbolLibrary) -> String {
        let name = "PWR_" + KiCad.libraryName(rail)
        if symbols.names.insert(name).inserted {
            let isGround = rail == "GND"
            // A bar on the pin for a voltage, bars hanging below it for ground.
            let drawing = isGround
                ? """
                \t\t\t\t(polyline (pts (xy 0 0) (xy 0 -1.27)) (stroke (width 0) (type default)) (fill (type none)))
                \t\t\t\t(polyline (pts (xy -1.27 -1.27) (xy 1.27 -1.27)) (stroke (width 0) (type default)) (fill (type none)))
                \t\t\t\t(polyline (pts (xy -0.762 -1.778) (xy 0.762 -1.778)) (stroke (width 0) (type default)) (fill (type none)))
                \t\t\t\t(polyline (pts (xy -0.254 -2.286) (xy 0.254 -2.286)) (stroke (width 0) (type default)) (fill (type none)))
                """
                : """
                \t\t\t\t(polyline (pts (xy 0 0) (xy 0 1.27)) (stroke (width 0) (type default)) (fill (type none)))
                \t\t\t\t(polyline (pts (xy -1.016 1.27) (xy 1.016 1.27)) (stroke (width 0) (type default)) (fill (type none)))
                """
            symbols.definitions.append("""
            \t\t(symbol \(KiCad.q("\(KiCadExport.library):\(name)"))
            \t\t\t(power)
            \t\t\t(pin_names (offset 0) hide)
            \t\t\t(exclude_from_sim no)
            \t\t\t(in_bom no)
            \t\t\t(on_board no)
            \(property("Reference", "#PWR", at: CGPoint(x: 0, y: -3), hide: true))
            \(property("Value", rail, at: CGPoint(x: 0, y: isGround ? -3.81 : 2.54)))
            \(property("Footprint", "", at: .zero, hide: true))
            \(property("Datasheet", "", at: .zero, hide: true))
            \t\t\t(symbol \(KiCad.q("\(name)_0_1"))
            \(drawing)
            \t\t\t)
            \t\t\t(symbol \(KiCad.q("\(name)_1_1"))
            \t\t\t\t(pin power_in line (at 0 0 \(isGround ? 90 : 270)) (length 0) hide
            \t\t\t\t\t(name \(KiCad.q(rail)) (effects (font (size 1 1))))
            \t\t\t\t\t(number "1" (effects (font (size 1 1))))
            \t\t\t\t)
            \t\t\t)
            \t\t)
            """)
        }
        let at = CGPoint(x: (point.x + shift.x) * k, y: (point.y + shift.y) * k)
        let reference = String(format: "#PWR%02d", number)
        let isGround = rail == "GND"
        return """
        \t(symbol
        \t\t(lib_id \(KiCad.q("\(KiCadExport.library):\(name)")))
        \t\t(at \(KiCad.n(at.x)) \(KiCad.n(at.y)) 0)
        \t\t(unit 1)
        \t\t(exclude_from_sim no)
        \t\t(in_bom no)
        \t\t(on_board no)
        \t\t(dnp no)
        \t\t(uuid \(KiCad.q(id("power/\(seed)"))))
        \(property("Reference", reference, at: at, hide: true))
        \(property("Value", rail, at: CGPoint(x: at.x, y: at.y + (isGround ? 3.81 : -2.54))))
        \(property("Footprint", "", at: at, hide: true))
        \(property("Datasheet", "", at: at, hide: true))
        \t\t(pin "1" (uuid \(KiCad.q(id("powerpin/\(seed)")))))
        \(instances(reference: reference))\t)
        """
    }

    /// A power flag (`#FLG<n>`) with its pin at `point`, on the net of `rail`.
    /// For a Power Flag label (plan-63-A) `rail` is `label/<its id>`, only a seed for the ids.
    private func flagInstance(at point: CGPoint, number: Int, rail: String, symbols: inout SymbolLibrary) -> String {
        let name = "PWR_FLAG"
        if symbols.names.insert(name).inserted {
            symbols.definitions.append("""
            \t\t(symbol \(KiCad.q("\(KiCadExport.library):\(name)"))
            \t\t\t(power)
            \t\t\t(pin_numbers hide)
            \t\t\t(pin_names (offset 0) hide)
            \t\t\t(exclude_from_sim no)
            \t\t\t(in_bom no)
            \t\t\t(on_board no)
            \(property("Reference", "#FLG", at: .zero, hide: true))
            \(property("Value", "PWR_FLAG", at: CGPoint(x: 0, y: 1.905), hide: true))
            \(property("Footprint", "", at: .zero, hide: true))
            \(property("Datasheet", "", at: .zero, hide: true))
            \t\t\t(symbol \(KiCad.q("\(name)_0_0"))
            \t\t\t\t(pin power_out line (at 0 0 90) (length 0)
            \t\t\t\t\t(name "pwr" (effects (font (size 1 1))))
            \t\t\t\t\t(number "1" (effects (font (size 1 1))))
            \t\t\t\t)
            \t\t\t)
            \t\t\t(symbol \(KiCad.q("\(name)_0_1"))
            \t\t\t\t(polyline (pts (xy 0 0) (xy 0 1.27) (xy -1.016 1.905) (xy 0 2.54) (xy 1.016 1.905) (xy 0 1.27)) (stroke (width 0) (type default)) (fill (type none)))
            \t\t\t)
            \t\t)
            """)
        }
        let at = CGPoint(x: (point.x + shift.x) * k, y: (point.y + shift.y) * k)
        let reference = String(format: "#FLG%02d", number)
        return """
        \t(symbol
        \t\t(lib_id \(KiCad.q("\(KiCadExport.library):\(name)")))
        \t\t(at \(KiCad.n(at.x)) \(KiCad.n(at.y)) 0)
        \t\t(unit 1)
        \t\t(exclude_from_sim no)
        \t\t(in_bom no)
        \t\t(on_board no)
        \t\t(dnp no)
        \t\t(uuid \(KiCad.q(id("flag/\(rail)"))))
        \(property("Reference", reference, at: at, hide: true))
        \(property("Value", "PWR_FLAG", at: at, hide: true))
        \(property("Footprint", "", at: at, hide: true))
        \(property("Datasheet", "", at: at, hide: true))
        \t\t(pin "1" (uuid \(KiCad.q(id("flagpin/\(rail)")))))
        \(instances(reference: reference))\t)
        """
    }

    // MARK: Wires

    private func line(_ a: CGPoint, _ b: CGPoint, key: String, kind: String = "wire", width: Double = 0) -> String {
        "\t(\(kind)\n\t\t(pts (xy \(xy(a))) (xy \(xy(b))))\n\t\t(stroke (width \(KiCad.n(width))) (type default))\n\t\t(uuid \(KiCad.q(id(key))))\n\t)"
    }

    /// Every wire as one KiCad wire per straight run, a junction where three
    /// or more runs meet, and a No Connect where a wire ends on one.
    private func wires() -> [String] {
        var out: [String] = []
        var ends: [String: (point: CGPoint, count: Int)] = [:]
        func count(_ point: CGPoint) {
            let key = "\(Int(point.x.rounded())),\(Int(point.y.rounded()))"
            ends[key, default: (point, 0)].count += 1
        }
        for wire in document.wires {
            guard let start = document.endTip(wire.start), let end = document.endTip(wire.end) else { continue }
            let route = [start] + wire.interior + [end]
            for (index, pair) in zip(route, route.dropFirst()).enumerated() where pair.0 != pair.1 {
                out.append(line(pair.0, pair.1, key: "wire/\(wire.id)/\(index)"))
                count(pair.0)
                count(pair.1)
            }
        }
        for key in ends.keys.sorted() {
            guard let entry = ends[key], entry.count >= 3 else { continue }
            out.append("\t(junction\n\t\t(at \(xy(entry.point)))\n\t\t(diameter 0)\n\t\t(color 0 0 0 0)\n\t\t(uuid \(KiCad.q(id("junction/\(key)"))))\n\t)")
        }
        return out
    }

    // MARK: Labels

    /// Net labels, No Connects, and a label on every wired bus port pin.
    private func labels() -> [String] {
        var out: [String] = []
        for label in document.netLabels {
            if label.isNet {
                let flipped = label.isFlipped
                out.append("""
                \t(label \(KiCad.q(label.name))
                \t\t(at \(xy(label.tip)) \(flipped ? 180 : 0))
                \t\t(effects (font (size 1.27 1.27)) (justify \(flipped ? "right" : "left") bottom))
                \t\t(uuid \(KiCad.q(id("label/\(label.id)"))))
                \t)
                """)
            } else if label.isNoConnect {
                out.append("\t(no_connect\n\t\t(at \(xy(label.tip)))\n\t\t(uuid \(KiCad.q(id("nc/\(label.id)"))))\n\t)")
            }
        }
        var done = Set<BusPinRef>()
        for wire in document.wires {
            for end in [wire.start, wire.end] {
                guard case let .busPin(ref) = end, done.insert(ref).inserted,
                      let tip = document.endTip(end), let text = context.busText[ref.wireID] else { continue }
                // The wire arrives from the port; the text runs away from it.
                out.append("""
                \t(label \(KiCad.q(text))
                \t\t(at \(xy(tip)) 0)
                \t\t(effects (font (size 1.27 1.27)) (justify left bottom))
                \t\t(uuid \(KiCad.q(id("buslabel/\(ref.portID)/\(ref.wireID)"))))
                \t)
                """)
            }
        }
        return out
    }

    // MARK: Buses

    /// Bus lines as KiCad buses, each port as an outlined box, and the
    /// bus's name above its first line.
    private func buses() -> [String] {
        var out: [String] = []
        for bus in document.buses {
            for line in bus.lines {
                out.append(self.line(line.a, line.b, key: "bus/\(line.id)", kind: "bus", width: 0))
            }
            if let first = bus.lines.first, !bus.name.isEmpty {
                let at = CGPoint(x: min(first.a.x, first.b.x), y: min(first.a.y, first.b.y) - 10)
                out.append(text(bus.name, at: at, size: 12, key: "busname/\(bus.id)", justify: "left bottom"))
            }
            for port in bus.ports {
                let along = bus.extent(of: port)
                guard let a = bus.point(on: port, along: along.lowerBound, out: 0),
                      let b = bus.point(on: port, along: along.upperBound, out: 0),
                      let c = bus.point(on: port, along: along.upperBound, out: BusPort.depth),
                      let d = bus.point(on: port, along: along.lowerBound, out: BusPort.depth) else { continue }
                out.append(polyline([a, b, c, d, a], key: "port/\(port.id)"))
            }
        }
        return out
    }

    private func polyline(_ points: [CGPoint], key: String) -> String {
        let pts = points.map { "(xy \(xy($0)))" }.joined(separator: " ")
        return "\t(polyline\n\t\t(pts \(pts))\n\t\t(stroke (width 0) (type default))\n\t\t(uuid \(KiCad.q(id(key))))\n\t)"
    }

    // MARK: Text

    private func text(_ string: String, at point: CGPoint, size: CGFloat, key: String, justify: String) -> String {
        let height = KiCad.n(size * k)
        return """
        \t(text \(KiCad.q(string))
        \t\t(exclude_from_sim no)
        \t\t(at \(xy(point)) 0)
        \t\t(effects (font (size \(height) \(height))) (justify \(justify)))
        \t\t(uuid \(KiCad.q(id(key))))
        \t)
        """
    }

    /// The fields of KiCad's own title block (plan-64-D): the document's title
    /// (the file's name when it has none), updated date, revision, author as
    /// the company, and description as the first comment. Empty ones are left
    /// out.
    private func titleBlockFields() -> String {
        let info = document.documentInfo
        var fields = ["\t\t(title \(KiCad.q(info.title.isEmpty ? context.base : info.title)))"]
        if let updated = info.updated {
            fields.append("\t\t(date \(KiCad.q(String(DocumentInfo.dateText(updated).prefix(10)))))")
        }
        if !info.revision.isEmpty { fields.append("\t\t(rev \(KiCad.q(info.revision)))") }
        if !info.author.isEmpty { fields.append("\t\t(company \(KiCad.q(info.author)))") }
        if !info.description.isEmpty {
            let flat = info.description.replacingOccurrences(of: "\n", with: " ")
            fields.append("\t\t(comment 1 \(KiCad.q(flat)))")
        }
        return fields.joined(separator: "\n")
    }

    /// The title block, when it is on the schematic: its frame and lines, and
    /// each cell's label and wrapped value, from the canvas's layout.
    private func titleBlock() -> [String] {
        guard let layout = document.titleBlockLayout else { return [] }
        let rect = layout.rect
        var out = [polyline([CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                             CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY),
                             CGPoint(x: rect.minX, y: rect.minY)], key: "titleblock/frame")]
        for (index, line) in layout.separators.enumerated() {
            out.append(polyline([line.0, line.1], key: "titleblock/line/\(index)"))
        }
        for cell in layout.cells {
            out.append(text(cell.label, at: cell.labelOrigin, size: CGFloat(TitleBlockLayout.labelSize),
                            key: "titleblock/\(cell.label)/label", justify: "left top"))
            for (index, line) in cell.lines.enumerated() where !line.isEmpty {
                out.append(text(line, at: CGPoint(x: cell.textOrigin.x, y: cell.textOrigin.y + CGFloat(index) * cell.lineHeight),
                                size: CGFloat(cell.size), key: "titleblock/\(cell.label)/\(index)", justify: "left top"))
            }
        }
        return out
    }

    /// Text blocks: the words, and a frame round them when the app draws one.
    private func notes() -> [String] {
        var out: [String] = []
        for note in document.notes {
            let rect = note.rect
            out.append(text(note.text, at: CGPoint(x: rect.minX + SchematicNote.padding, y: rect.minY + SchematicNote.padding),
                            size: CGFloat(note.size), key: "note/\(note.id)", justify: "left top"))
            if note.framed {
                out.append(polyline([CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                                     CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY),
                                     CGPoint(x: rect.minX, y: rect.minY)], key: "frame/\(note.id)"))
            }
        }
        return out
    }
}
