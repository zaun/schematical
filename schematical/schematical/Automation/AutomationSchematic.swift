//
//  AutomationSchematic.swift
//  schematical
//
//  Automation tools that edit the schematic page.
//

import CoreGraphics
import Foundation

extension AutomationTools {
    static let schematicTools: [AutomationTool] = [
        AutomationTool(
            name: "place_part",
            summary: "Place a catalog part on the schematic, centred at x,y (canvas units, y down, 10-unit grid). "
                + "Returns its reference, size and pins. Use search_parts first for the exact part number. "
                + "Power: parts named VCC and GND are supply symbols.",
            properties: [
                "part": Schema.string("Part number from search_parts"),
                "x": Schema.number("Centre x"), "y": Schema.number("Centre y"),
                "color": Schema.string("LED color (see get_part)"),
                "ohms": Schema.number("Resistance, for resistors and pull resistors"),
                "farads": Schema.number("Capacitance in farads, for capacitors"),
                "footprint": Schema.string("PCB footprint: a package name or id from get_part"),
                "gender": Schema.string("Header gender (see get_part)"),
            ],
            required: ["part", "x", "y"], edits: true,
            run: { context, args in Out.text(try placePart(context!, args)) }),
        AutomationTool(
            name: "connect",
            summary: "Draw a wire between two ends. An end is a pin (\"R1.2\", \"D1.K\", \"U1.VCC\"), a net label "
                + "(\"label:CLK\"), a bus pin (\"bus:BUS 1:D0\", needs a port with that wire) or a point on an "
                + "existing wire (\"@x,y\", which makes a junction). Wires route at right angles.",
            properties: [
                "from": Schema.string("First end"), "to": Schema.string("Second end"),
                "via": Schema.points("Optional waypoints [[x,y],…] (canvas units) the wire passes through, "
                                     + "joined with right angles. Wires otherwise take the shortest L-shape and "
                                     + "ignore parts in the way, so use via to go around them."),
            ],
            required: ["from", "to"], edits: true,
            run: { context, args in Out.text(try connect(context!, args)) }),
        AutomationTool(
            name: "disconnect",
            summary: "Remove the wire(s) drawn directly between two pins (from + to), or every wire on one pin (pin).",
            properties: [
                "from": Schema.string("First pin"), "to": Schema.string("Second pin"),
                "pin": Schema.string("Remove every wire on this pin"),
            ],
            edits: true,
            run: { context, args in Out.text(try disconnect(context!, args)) }),
        AutomationTool(
            name: "move_part",
            summary: "Move a part on the schematic: its centre to x,y, or by dx,dy. Its wires follow.",
            properties: [
                "reference": Schema.string("e.g. U1"), "x": Schema.number("New centre x"),
                "y": Schema.number("New centre y"), "dx": Schema.number("Move right by"),
                "dy": Schema.number("Move down by"),
            ],
            required: ["reference"], edits: true,
            run: { context, args in Out.text(try movePart(context!, args)) }),
        AutomationTool(
            name: "flip_part",
            summary: "Mirror a part left to right on the schematic (swaps which side its pins are on).",
            properties: ["reference": Schema.string("e.g. U1")], required: ["reference"], edits: true,
            run: { context, args in
                let chip = try context!.chip(try args.string("reference"))
                guard chip.symbol.canFlip else { throw AutomationFailure("\(chip.reference) can't be flipped") }
                context!.document.flipChip(chip.id)
                return Out.text("flipped \(chip.reference)")
            }),
        AutomationTool(
            name: "set_part",
            summary: "Change a placed part: new_reference (\"U5\"), color, ohms, farads, footprint, gender, "
                + "show_reference.",
            properties: [
                "reference": Schema.string("e.g. R1"),
                "new_reference": Schema.string("Rename, e.g. R7 (a letter prefix and a number)"),
                "color": Schema.string("LED color"), "ohms": Schema.number("Resistance"),
                "farads": Schema.number("Capacitance"), "footprint": Schema.string("Package name or footprint id"),
                "gender": Schema.string("Header gender"),
                "show_reference": Schema.boolean("Show the reference label"),
            ],
            required: ["reference"], edits: true,
            run: { context, args in Out.text(try setPart(context!, args)) }),
        AutomationTool(
            name: "add_net_label",
            summary: "Add a net label at x,y. Labels with the same name are one net. Connect a pin to it with "
                + "connect(\"U1.3\", \"label:CLK\"). Names are up to 4 characters.",
            properties: [
                "name": Schema.string("Net name, e.g. CLK"), "x": Schema.number("x"), "y": Schema.number("y"),
                "style": Schema.string("input (default), output, inout, passive, noConnect",
                                       options: NetLabel.Style.allCases.map(\.rawValue)),
            ],
            required: ["name", "x", "y"], edits: true,
            run: { context, args in Out.text(try addNetLabel(context!, args)) }),
        AutomationTool(
            name: "add_note",
            summary: "Add a text note on the schematic with its top-left at x,y. Text may have line breaks.",
            properties: [
                "text": Schema.string("The words"), "x": Schema.number("x"), "y": Schema.number("y"),
                "size": Schema.integer("Font size in points, 6–72 (default 12)"),
                "framed": Schema.boolean("Draw a frame (default true)"),
            ],
            required: ["text", "x", "y"], edits: true,
            run: { context, args in Out.text(try addNote(context!, args)) }),
        AutomationTool(
            name: "add_bus",
            summary: "Draw a straight bus from (x1,y1) to (x2,y2) (horizontal or vertical). Give it a name "
                + "(up to 4 characters) and a number of wires (2–999, default 8). Wires are named B0, B1, B2…; "
                + "add ports with add_bus_port, then connect pins to \"bus:NAME:WIRE\".",
            properties: [
                "x1": Schema.number("Start x"), "y1": Schema.number("Start y"),
                "x2": Schema.number("End x"), "y2": Schema.number("End y"),
                "name": Schema.string("Bus name"), "wires": Schema.integer("Number of wires"),
            ],
            required: ["x1", "y1", "x2", "y2"], edits: true,
            run: { context, args in Out.text(try addBus(context!, args)) }),
        AutomationTool(
            name: "add_bus_port",
            summary: "Add a port on a bus, at a point on one of its lines. A port is a column of pins, one per "
                + "ticked wire; connect part pins to them with \"bus:NAME:WIRE\".",
            properties: [
                "bus": Schema.string("Bus name"), "x": Schema.number("x on the bus line"),
                "y": Schema.number("y on the bus line"),
                "wires": Schema.strings("Wire names to give pins to, e.g. [\"B0\",\"B1\"] (default: all)"),
            ],
            required: ["bus", "x", "y"], edits: true,
            run: { context, args in Out.text(try addBusPort(context!, args)) }),
        AutomationTool(
            name: "delete",
            summary: "Delete items: parts by reference (\"U1\"; wires on its pins stay, with loose ends), "
                + "\"wire:ID\", \"label:NAME\" or \"label:ID\", \"note:ID\", \"bus:NAME\", and on the PCB "
                + "\"trace:ID\", \"via:ID\", \"hole:ID\", \"text:ID\". Ids are printed by get_design and get_board.",
            properties: ["items": Schema.strings("Items to delete")], required: ["items"], edits: true,
            run: { context, args in Out.text(try delete(context!, args)) }),
        AutomationTool(
            name: "select",
            summary: "Select items so the user sees them (and later tools can act on the selection). Same item "
                + "names as delete, plus \"fp:U1\" for a footprint. An empty list clears the selection.",
            properties: [
                "items": Schema.strings("Items to select"),
                "add": Schema.boolean("Add to the selection instead of replacing it"),
            ],
            required: ["items"],
            run: { context, args in Out.text(try select(context!, args)) }),
    ]

    // MARK: Placing

    private static func footprintID(_ part: CatalogPart, _ name: String) throws -> String {
        if let choice = part.footprintChoices.first(where: {
            $0.package.caseInsensitiveCompare(name) == .orderedSame || $0.footprint.id == name
        }) { return choice.footprint.id }
        if part.footprint?.id == name { return name }
        let known = part.footprintChoices.map { "\($0.package)=\($0.footprint.id)" }
        throw AutomationFailure("\(part.name) has no footprint \(name)"
                                + (known.isEmpty ? "" : "; choices: " + known.joined(separator: ", ")))
    }

    private static func placePart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let name = try args.string("part")
        guard let part = PartCatalogStore.shared.part(named: name) else {
            throw AutomationFailure("no part \(name); use search_parts")
        }
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        if let color = args.optString("color"), !part.colors.contains(color) {
            throw AutomationFailure("\(part.name) has no color \(color); colors: \(part.colors.joined(separator: ", "))")
        }
        let footprint = try args.optString("footprint").map { try footprintID(part, $0) }
        let document = context.document
        let before = document.chips.count
        document.placeChip(part: part, symbol: symbol(for: part), centeredAt: point,
                           color: args.optString("color"), footprintID: footprint,
                           gender: args.optString("gender"), ohms: args.optDouble("ohms"),
                           farads: args.optDouble("farads"))
        guard document.chips.count > before, let placed = document.chips.last else {
            throw AutomationFailure("the part was not placed")
        }
        let centre = context.centre(of: placed)
        let pins = placed.part.pins.filter { !$0.isHidden }
        let listing = pins.count <= 16
            ? "; pins: " + pins.map { "\($0.designator)=\($0.name)" }.joined(separator: " ")
            : "; \(pins.count) pins (see get_part)"
        return "placed \(placed.reference) (\(part.name)) centre=(\(num(centre.x)),\(num(centre.y))) "
            + "size=\(num(placed.layout.size.width))x\(num(placed.layout.size.height))" + listing
    }

    // MARK: Wiring

    /// One end of a wire, from its name.
    private static func anchor(_ spec: String, _ context: AutomationContext) throws -> LineAnchor {
        let document = context.document
        if spec.hasPrefix("label:") {
            let name = String(spec.dropFirst("label:".count))
            guard let label = document.netLabels.first(where: { $0.name == name })
                ?? document.netLabels.first(where: { $0.id.uuidString.lowercased().hasPrefix(name.lowercased()) }) else {
                throw AutomationFailure("no net label \(name); add one with add_net_label")
            }
            return .label(label.id)
        }
        if spec.hasPrefix("bus:") {
            let parts = spec.dropFirst("bus:".count).split(separator: ":", maxSplits: 1).map(String.init)
            guard parts.count == 2 else { throw AutomationFailure("a bus pin looks like bus:NAME:WIRE") }
            guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(parts[0]) == .orderedSame }) else {
                throw AutomationFailure("no bus \(parts[0]); buses: " + document.buses.map(\.name).joined(separator: ", "))
            }
            guard let wire = bus.wires.first(where: { $0.name == parts[1] }) else {
                throw AutomationFailure("bus \(bus.name) has no wire \(parts[1]); wires: " + bus.wires.map(\.name).joined(separator: ","))
            }
            guard let port = bus.ports.first(where: { $0.wireIDs.contains(wire.id) }) else {
                throw AutomationFailure("bus \(bus.name) has no port with wire \(wire.name); add one with add_bus_port")
            }
            return .busPin(BusPinRef(portID: port.id, wireID: wire.id))
        }
        if spec.hasPrefix("@") {
            let numbers = spec.dropFirst().split(separator: ",").compactMap { Double($0.trimmingCharacters(in: .whitespaces)) }
            guard numbers.count == 2 else { throw AutomationFailure("a point looks like @x,y") }
            return .point(SchematicGrid.snap(CGPoint(x: numbers[0], y: numbers[1])))
        }
        return .pin(try context.pinRef(spec))
    }

    /// `points` joined with right-angle legs. A leg that isn't already
    /// straight moves first along the axis that turns from the last
    /// segment (`start` for the first); the last leg arrives along `end`.
    private static func orthogonal(_ points: [CGPoint], start: RouteAxis, end: RouteAxis?) -> [CGPoint] {
        var out = [points[0]]
        var next = start
        for index in 1..<points.count {
            let a = out[out.count - 1], b = points[index]
            if a.x == b.x || a.y == b.y {
                out.append(b)
                next = a.x == b.x ? RouteAxis.horizontal : RouteAxis.vertical
                continue
            }
            var first = next
            if index == points.count - 1, let end { first = end.other }
            out.append(first == .horizontal ? CGPoint(x: b.x, y: a.y) : CGPoint(x: a.x, y: b.y))
            out.append(b)
            next = first
        }
        return out
    }

    private static func connect(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let from = try args.string("from"), to = try args.string("to")
        let start = try anchor(from, context), end = try anchor(to, context)
        let before = Set(context.document.wires.map(\.id))
        var path: [CGPoint]?
        if args.raw["via"] != nil {
            let document = context.document
            guard let a = document.anchorTip(start), let b = document.anchorTip(end) else {
                throw AutomationFailure("can't find the ends of that wire")
            }
            let waypoints = try args.points("via").map { SchematicGrid.snap($0) }
            path = orthogonal([a] + waypoints + [b], start: document.anchorAxis(start) ?? .horizontal,
                              end: document.anchorAxis(end))
        }
        if let refusal = context.document.connect(start, end, path: path) { throw AutomationFailure(refusal) }
        let added = context.document.wires.filter { !before.contains($0.id) }
        guard !added.isEmpty else { throw AutomationFailure("nothing was connected (already wired?)") }
        return "connected \(from) to \(to) (wire \(added.map { context.short($0.id) }.joined(separator: ", ")))"
    }

    private static func disconnect(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var doomed: [Wire] = []
        if let pin = args.optString("pin") {
            let ref = try context.pinRef(pin)
            doomed = document.wires.filter { $0.start == .pin(ref) || $0.end == .pin(ref) }
        } else {
            let a = try context.pinRef(try args.string("from")), b = try context.pinRef(try args.string("to"))
            doomed = document.wires.filter {
                ($0.start == .pin(a) && $0.end == .pin(b)) || ($0.start == .pin(b) && $0.end == .pin(a))
            }
        }
        guard !doomed.isEmpty else {
            throw AutomationFailure("no wire is drawn directly there (a wire may go via a junction; use delete wire:ID)")
        }
        document.clearAllSelections()
        for wire in doomed { document.toggleSelection(.wire(wire.id)) }
        document.deleteSchematicSelection(named: "Disconnect")
        document.clearAllSelections()
        return "removed \(doomed.count) wire\(doomed.count == 1 ? "" : "s")"
    }

    // MARK: Moving and changing

    private static func movePart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let chip = try context.chip(try args.string("reference"))
        let size = chip.layout.size
        let target: CGPoint
        if let x = args.optDouble("x"), let y = args.optDouble("y") {
            target = SchematicGrid.snap(CGPoint(x: x - size.width / 2, y: y - size.height / 2))
        } else if let dx = args.optDouble("dx"), let dy = args.optDouble("dy") {
            target = SchematicGrid.snap(CGPoint(x: chip.origin.x + dx, y: chip.origin.y + dy))
        } else {
            throw AutomationFailure("give x and y (the new centre), or dx and dy")
        }
        let delta = CGSize(width: target.x - chip.origin.x, height: target.y - chip.origin.y)
        document.recordEdit(.schematic, "Move") {
            document.clearAllSelections()
            document.select(.chip(chip.id))
            let base = document.schematicGroupBase()
            document.moveSchematicGroup(base, by: delta)
            document.settleSchematicGroup(base)
            document.clearAllSelections()
        }
        let moved = try context.chip(chip.reference)
        let centre = context.centre(of: moved)
        return "\(moved.reference) now centred at (\(num(centre.x)),\(num(centre.y)))"
    }

    private static func setPart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let chip = try context.chip(try args.string("reference"))
        var done: [String] = []
        try document.recordEdit(.schematic, "Change Part") {
            if let color = args.optString("color") {
                guard chip.part.colors.contains(color) else {
                    throw AutomationFailure("\(chip.part.name) has no color \(color); colors: \(chip.part.colors.joined(separator: ", "))")
                }
                document.setPartColor(chip.id, color)
                done.append("color")
            }
            if let ohms = args.optDouble("ohms") {
                guard ResistorValue.isValid(ohms) else { throw AutomationFailure("\(num(ohms)) ohms isn't a valid resistance") }
                document.setPartOhms(chip.id, ohms)
                done.append("ohms")
            }
            if let farads = args.optDouble("farads") {
                document.setPartFarads(chip.id, farads)
                done.append("farads")
            }
            if let footprint = args.optString("footprint") {
                document.setPartFootprint(chip.id, try footprintID(chip.part, footprint))
                done.append("footprint")
            }
            if let gender = args.optString("gender") {
                document.setPartGender(chip.id, gender)
                done.append("gender")
            }
            if let shows = args.optBool("show_reference") {
                document.setPartShowsReference(chip.id, shows)
                done.append("show_reference")
            }
            if let name = args.optString("new_reference") {
                guard let match = name.wholeMatch(of: /([A-Za-z]+)(\d+)/), let number = Int(match.2) else {
                    throw AutomationFailure("a reference is letters then a number, like U5")
                }
                if let refusal = document.renameChip(chip.id, prefix: String(match.1), number: number, page: .schematic) {
                    throw AutomationFailure(refusal)
                }
                done.append("reference")
            }
        }
        guard !done.isEmpty else {
            throw AutomationFailure("nothing to change: pass new_reference, color, ohms, farads, footprint, gender or show_reference")
        }
        return "\(chip.reference): changed \(done.joined(separator: ", "))"
    }

    // MARK: Labels, notes, buses

    private static func addNetLabel(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let name = try args.string("name")
        let style = NetLabel.Style(rawValue: try args.choice("style", among: NetLabel.Style.allCases.map(\.rawValue),
                                                              default: "input")) ?? .input
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        var result: UUID?
        document.recordEdit(.schematic, "Add Net Label") {
            let id = document.addNetLabel(at: point, style: style)
            if document.setNetLabelName(id, name) { result = id } else { document.deleteNetLabel(id) }
        }
        guard let id = result, let label = document.netLabels.first(where: { $0.id == id }) else {
            throw AutomationFailure(document.editorMessage ?? "the name “\(name)” isn't allowed for a net label")
        }
        document.clearAllSelections()
        return "added net label \(label.name) (\(context.short(id))) at (\(num(label.origin.x)),\(num(label.origin.y)))"
    }

    private static func addNote(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let text = try args.string("text")
        var id: UUID?
        document.recordEdit(.schematic, "Add Text") {
            let note = document.addNote(at: point)
            document.setNoteText(note, text)
            if let size = args.optInt("size") { document.setNoteSize(note, size) }
            if let framed = args.optBool("framed") { document.setNoteFramed(note, framed) }
            id = note
        }
        document.editingNoteID = nil
        document.clearAllSelections()
        return "added note \(id.map(context.short) ?? "")"
    }

    private static func addBus(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let start = SchematicGrid.snap(CGPoint(x: try args.double("x1"), y: try args.double("y1")))
        let end = SchematicGrid.snap(CGPoint(x: try args.double("x2"), y: try args.double("y2")))
        var notes: [String] = []
        var created: UUID?
        document.recordEdit(.schematic, "Add Bus") {
            created = document.addBus(from: start, to: end)
            guard let id = created else { return }
            if let name = args.optString("name"), let refusal = document.setBusName(id, name) { notes.append(refusal) }
            if let count = args.optInt("wires"), let refusal = document.setBusWireCount(id, count) { notes.append(refusal) }
        }
        guard let id = created, let bus = document.bus(id) else {
            throw AutomationFailure("couldn't draw a bus there: it must be one horizontal or vertical line, "
                                    + "not touching another bus")
        }
        document.clearAllSelections()
        return "added bus \(bus.name) with \(bus.wires.count) wires (\(bus.wires.prefix(4).map(\.name).joined(separator: ","))…)"
            + (notes.isEmpty ? "" : "; note: " + notes.joined(separator: "; "))
    }

    private static func addBusPort(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let name = try args.string("bus")
        guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) else {
            throw AutomationFailure("no bus \(name); buses: " + document.buses.map(\.name).joined(separator: ", "))
        }
        let wanted: Set<UUID>
        if let names = try? args.strings("wires") {
            let hits = bus.wires.filter { names.contains($0.name) }
            guard hits.count == Set(names).count else {
                throw AutomationFailure("bus \(bus.name) wires are: " + bus.wires.map(\.name).joined(separator: ","))
            }
            wanted = Set(hits.map(\.id))
        } else {
            wanted = Set(bus.wires.map(\.id))
        }
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let known = Set(bus.ports.map(\.id))
        var problem: String?
        document.recordEdit(.schematic, "Add Port") {
            if let refusal = document.addPort(at: point) { problem = refusal; return }
            guard let port = document.bus(bus.id)?.ports.first(where: { !known.contains($0.id) }) else {
                problem = "no port was added"
                return
            }
            if let refusal = document.setPortWires(port.id, wanted) { problem = refusal }
        }
        document.clearAllSelections()
        if let problem { throw AutomationFailure(problem) }
        return "added a port on \(bus.name) with \(wanted.count) pins"
    }

    // MARK: Deleting and selecting

    enum Item {
        case schematic(SchematicItem)
        case board(BoardItem)
    }

    static func item(_ spec: String, _ context: AutomationContext, forDelete: Bool) throws -> Item {
        let document = context.document
        guard let colon = spec.firstIndex(of: ":") else {
            return .schematic(.chip(try context.chip(spec).id))
        }
        let kind = String(spec[..<colon]).lowercased()
        let rest = String(spec[spec.index(after: colon)...])
        switch kind {
        case "wire": return .schematic(.wire(try context.find(document.wires, rest, kind: "wire").id))
        case "note": return .schematic(.note(try context.find(document.notes, rest, kind: "note").id))
        case "label":
            if let label = document.netLabels.first(where: { $0.name == rest }) { return .schematic(.label(label.id)) }
            return .schematic(.label(try context.find(document.netLabels, rest, kind: "net label").id))
        case "bus":
            guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(rest) == .orderedSame }) else {
                throw AutomationFailure("no bus \(rest)")
            }
            return .schematic(.bus(bus.id))
        case "trace": return .board(.trace(try context.find(document.traces, rest, kind: "trace").id))
        case "via": return .board(.via(try context.find(document.vias, rest, kind: "via").id))
        case "hole": return .board(.hole(try context.find(document.holes, rest, kind: "hole").id))
        case "text": return .board(.text(try context.find(document.texts, rest, kind: "board text").id))
        case "fp", "footprint":
            if forDelete { throw AutomationFailure("a footprint goes with its part: delete \(rest)") }
            return .board(.footprint(try context.chip(rest).id))
        default:
            throw AutomationFailure("unknown item “\(spec)”: use a reference (U1), wire:ID, label:NAME, note:ID, "
                                    + "bus:NAME, trace:ID, via:ID, hole:ID, text:ID or fp:U1")
        }
    }

    private static func delete(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let items = try args.strings("items").map { try item($0, context, forDelete: true) }
        guard !items.isEmpty else { throw AutomationFailure("items is empty") }
        var removed = 0
        document.recordEdit(.schematic, "Delete") {
            document.clearAllSelections()
            for item in items {
                switch item {
                case let .schematic(item):
                    document.toggleSelection(item)
                case let .board(.trace(id)): document.deleteTrace(id); removed += 1
                case let .board(.via(id)): document.deleteVia(id); removed += 1
                case let .board(.hole(id)): document.deleteHole(id); removed += 1
                case let .board(.text(id)): document.deleteText(id); removed += 1
                case .board: break
                }
            }
            let schematicCount = items.filter { if case .schematic = $0 { true } else { false } }.count
            if schematicCount > 0, document.deleteSchematicSelection() { removed += schematicCount }
            document.clearAllSelections()
        }
        guard removed > 0 else { throw AutomationFailure(document.editorMessage ?? "nothing was deleted") }
        return "deleted \(removed) item\(removed == 1 ? "" : "s")"
    }

    private static func select(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let items = try args.strings("items").map { try item($0, context, forDelete: false) }
        if args.optBool("add") != true { document.clearAllSelections() }
        for item in items {
            switch item {
            case let .schematic(item): if !document.isSelected(item) { document.toggleSelection(item) }
            case let .board(item): if !document.isSelected(item) { document.toggleSelection(item) }
            }
        }
        return "selected \(items.count) item\(items.count == 1 ? "" : "s")"
    }
}
