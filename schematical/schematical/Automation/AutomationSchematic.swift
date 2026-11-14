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
            name: "move",
            summary: "Move schematic items together: parts (\"U1\"), \"label:NAME\" or \"label:ID\", \"note:ID\", "
                + "\"junction:ID\", \"bus:NAME\" and \"wire:ID\" (its bends only). Wires attached to a moved part, label, "
                + "junction or bus port follow, as when dragging in the app. Give dx and dy, or x and y for one "
                + "item (a part's centre, a label's or junction's point, a note's top-left corner).",
            properties: [
                "items": Schema.strings("What to move"), "dx": Schema.number("Move right by"),
                "dy": Schema.number("Move down by"), "x": Schema.number("New x (one item)"),
                "y": Schema.number("New y (one item)"),
            ],
            required: ["items"], edits: true,
            run: { context, args in Out.text(try moveItems(context!, args)) }),
        AutomationTool(
            name: "set_net_label",
            summary: "Change an existing net label: rename it (up to 4 characters, no spaces; same name = same net), "
                + "set its type, or flip which way it runs. Move it with move.",
            properties: [
                "label": Schema.string("label:NAME or label:ID (an id is exact; a name picks the first match)"),
                "name": Schema.string("New name"),
                "style": Schema.string("Type", options: NetLabel.Style.allCases.map(\.rawValue)),
                "flip": Schema.boolean("Turn it to run the other way"),
            ],
            required: ["label"], edits: true,
            run: { context, args in Out.text(try setNetLabel(context!, args)) }),
        AutomationTool(
            name: "set_note",
            summary: "Change an existing note: its text, font size or frame. Move it with move.",
            properties: [
                "note": Schema.string("note:ID, from get_design"), "text": Schema.string("The words (not empty)"),
                "size": Schema.integer("Font size in points, 6–72"), "framed": Schema.boolean("Draw a frame"),
            ],
            required: ["note"], edits: true,
            run: { context, args in Out.text(try setNote(context!, args)) }),
        AutomationTool(
            name: "set_wire_route",
            summary: "Re-route an existing wire through waypoints, keeping its ends: via is a list of [x,y] "
                + "points it passes through, joined with right angles as connect's via is. An empty via gives "
                + "the plain shortest route.",
            properties: ["wire": Schema.string("wire:ID, from get_design"), "via": Schema.points("Waypoints")],
            required: ["wire", "via"], edits: true,
            run: { context, args in Out.text(try setWireRoute(context!, args)) }),
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
                + "(up to 4 characters) and a number of wires (2–999, default 8). Wires are named B0, B1, B2… (rename or add some with set_bus); "
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
            name: "set_bus",
            summary: "Change an existing bus: rename it, set its number of wires, name its wires, rename single "
                + "wires, or move a wire to another place. Wire names are up to 4 characters, no spaces; two wires may share a "
                + "name (each is still its own net, but \"bus:NAME:WIRE\" and rename then pick the first). Refused when the bus type sets the wires, or when a port still uses a wire the "
                + "change would drop. A new wire has no pin yet: give it one with set_bus_port.",
            properties: [
                "bus": Schema.string("Bus name"), "name": Schema.string("New bus name (up to 4 characters)"),
                "wires": Schema.integer("Number of wires (2–999). New wires are added, or dropped, at the end"),
                "wire_names": Schema.strings("Names for all wires in order, e.g. [\"D0\",\"D1\",\"WR\"]; "
                    + "this also sets the number of wires"),
                "rename": ["type": "object", "additionalProperties": ["type": "string"],
                           "description": "Old wire name → new name, e.g. {\"B2\": \"WR\"}"],
                "move_wire": Schema.string("A wire name to move, with move_to"),
                "move_to": Schema.integer("Its new place, 0 = first"),
            ],
            required: ["bus"], edits: true,
            run: { context, args in Out.text(try setBus(context!, args)) }),
        AutomationTool(
            name: "set_bus_port",
            summary: "Change a bus port: which wires have pins, which side the pins come out of, or where it sits "
                + "along its line. Ports are numbered from 1 in get_design (BUSES). Removing a wire's pin leaves "
                + "the wires on it with loose ends. Delete a port with delete [\"port:BUS:N\"].",
            properties: [
                "bus": Schema.string("Bus name"), "port": Schema.integer("Port number, from 1"),
                "wires": Schema.strings("The wire names to have pins, e.g. [\"D0\",\"WR\"]; default: unchanged"),
                "add_wires": Schema.strings("Wire names to add pins for, keeping the rest"),
                "remove_wires": Schema.strings("Wire names to remove pins for, keeping the rest"),
                "side": Schema.string("top or bottom (horizontal bus line), left or right (vertical one)"),
                "at": Schema.number("Where its first pin goes along the line: x on a horizontal bus line, y on a vertical one"),
            ],
            required: ["bus", "port"], edits: true,
            run: { context, args in Out.text(try setBusPort(context!, args)) }),
        AutomationTool(
            name: "delete",
            summary: "Delete items: parts by reference (\"U1\"; wires on its pins stay, with loose ends), "
                + "\"wire:ID\", \"label:NAME\" or \"label:ID\", \"note:ID\", \"junction:ID\", \"bus:NAME\", \"port:BUS:N\" (a bus port, numbered from 1 in get_design; "
                + "wires on its pins stay, with loose ends), and on the PCB "
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

    /// Selects `items`, moves them by `delta` as the app's group drag does, and settles the wires.
    private static func moveGroup(_ document: SchematicDocument, _ items: [SchematicItem], by delta: CGSize) {
        document.recordEdit(.schematic, "Move") {
            document.clearAllSelections()
            for item in items { document.toggleSelection(item) }
            let base = document.schematicGroupBase()
            document.moveSchematicGroup(base, by: delta)
            document.settleSchematicGroup(base)
            document.clearAllSelections()
        }
    }

    /// Where an item is, for "move to x,y".
    private static func position(_ item: SchematicItem, _ context: AutomationContext) -> CGPoint? {
        let document = context.document
        switch item {
        case let .chip(id): return document.chips.first { $0.id == id }.map { context.centre(of: $0) }
        case let .label(id): return document.netLabels.first { $0.id == id }?.origin
        case let .note(id): return document.notes.first { $0.id == id }?.origin
        case let .junction(id): return document.junctions.first { $0.id == id }?.point
        case .wire, .bus: return nil
        }
    }

    private static func moveItems(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let specs = try args.strings("items")
        guard !specs.isEmpty else { throw AutomationFailure("items is empty") }
        let items: [SchematicItem] = try specs.map {
            guard case let .schematic(found) = try item($0, context, forDelete: false) else {
                throw AutomationFailure("\($0) isn't on the schematic page (PCB items move with move_footprint, set_trace…)")
            }
            return found
        }
        let delta: CGSize
        if let x = args.optDouble("x"), let y = args.optDouble("y") {
            guard items.count == 1, let at = position(items[0], context) else {
                throw AutomationFailure("x and y move one part, label, note or junction; use dx and dy for the rest")
            }
            delta = CGSize(width: x - at.x, height: y - at.y)
        } else if let dx = args.optDouble("dx"), let dy = args.optDouble("dy") {
            delta = CGSize(width: dx, height: dy)
        } else {
            throw AutomationFailure("give dx and dy, or x and y for one item")
        }
        moveGroup(document, items, by: delta)
        let places = zip(specs, items).compactMap { spec, item in
            position(item, context).map { "\(spec) at (\(num($0.x)),\(num($0.y)))" }
        }
        return "moved \(specs.count) item\(specs.count == 1 ? "" : "s")" + (places.isEmpty ? "" : ": " + places.joined(separator: "; "))
    }

    private static func setNetLabel(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("label")
        if !spec.contains(":") { spec = "label:" + spec }
        guard case let .schematic(.label(id)) = try item(spec, context, forDelete: false) else {
            throw AutomationFailure("\(spec) isn't a net label")
        }
        guard args.optString("name") != nil || args.optString("style") != nil || args.optBool("flip") == true else {
            throw AutomationFailure("nothing to change: give name, style or flip")
        }
        var problem: String?
        document.recordEdit(.schematic, "Change Net Label") {
            document.editorMessage = nil
            if let name = args.optString("name"), !document.setNetLabelName(id, name) {
                problem = document.editorMessage ?? "\(name) isn't a usable label name"
            }
            if problem == nil, let style = args.optString("style") {
                guard let kind = NetLabel.Style(rawValue: style) else { problem = "unknown style \(style)"; return }
                document.setNetLabelStyle(id, kind)
                if document.netLabels.first(where: { $0.id == id })?.style != kind {
                    problem = document.editorMessage ?? "the type change was refused"
                }
            }
            if problem == nil, args.optBool("flip") == true { document.flipNetLabel(id) }
        }
        document.endCoalescing()
        if let problem { throw AutomationFailure(problem) }
        guard let label = document.netLabels.first(where: { $0.id == id }) else { return "changed" }
        return "label \(context.short(id)) is \(label.name), \(label.style.rawValue)"
    }

    private static func setNote(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("note")
        if !spec.contains(":") { spec = "note:" + spec }
        guard case let .schematic(.note(id)) = try item(spec, context, forDelete: false) else {
            throw AutomationFailure("\(spec) isn't a note")
        }
        let text = args.optString("text"), size = args.optInt("size"), framed = args.optBool("framed")
        guard text != nil || size != nil || framed != nil else { throw AutomationFailure("nothing to change: give text, size or framed") }
        if text?.isEmpty == true { throw AutomationFailure("a note's text can't be empty (delete it instead)") }
        document.recordEdit(.schematic, "Change Text") {
            if let text { document.setNoteText(id, text) }
            if let size { document.setNoteSize(id, size) }
            if let framed { document.setNoteFramed(id, framed) }
        }
        document.endCoalescing()
        guard let note = document.notes.first(where: { $0.id == id }) else { return "changed" }
        return "note \(context.short(id)): size \(note.size), \(note.framed ? "framed" : "no frame")"
    }

    private static func setWireRoute(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("wire")
        if !spec.contains(":") { spec = "wire:" + spec }
        guard case let .schematic(.wire(id)) = try item(spec, context, forDelete: false),
              let wire = document.wires.first(where: { $0.id == id }),
              let a = document.endTip(wire.start), let b = document.endTip(wire.end) else {
            throw AutomationFailure("\(spec) isn't a wire with both ends placed")
        }
        func anchor(_ end: WireEnd) -> LineAnchor? {
            switch end {
            case let .pin(ref): .pin(ref)
            case let .busPin(ref): .busPin(ref)
            case let .label(id): .label(id)
            case .junction: nil
            }
        }
        let waypoints = try args.points("via").map { SchematicGrid.snap($0) }
        let path = OrthogonalRoute.normalize(orthogonal(
            [a] + waypoints + [b], start: anchor(wire.start).flatMap(document.anchorAxis) ?? .horizontal,
            end: anchor(wire.end).flatMap(document.anchorAxis)))
        document.setWirePath(id, path)
        let now = document.path(forWire: id)
        return "wire \(context.short(id)) now runs " + now.map { "(\(num($0.x)),\(num($0.y)))" }.joined(separator: " ")
    }

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
        moveGroup(document, [.chip(chip.id)], by: delta)
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

    private static func bus(_ document: SchematicDocument, _ name: String) throws -> Bus {
        guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) else {
            throw AutomationFailure("no bus \(name); buses: " + document.buses.map(\.name).joined(separator: ", "))
        }
        return bus
    }

    private static func wire(_ bus: Bus, _ name: String) throws -> BusWire {
        guard let wire = bus.wires.first(where: { $0.name == name }) else {
            throw AutomationFailure("bus \(bus.name) has no wire \(name); wires: " + bus.wires.map(\.name).joined(separator: ","))
        }
        return wire
    }

    private static func setBus(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let id = try bus(document, try args.string("bus")).id
        let names = args.raw["wire_names"] != nil ? try args.strings("wire_names") : nil
        let renames = (args.raw["rename"] as? [String: String]) ?? [:]
        let moving = args.optString("move_wire")
        guard names != nil || args.optInt("wires") != nil || !renames.isEmpty || moving != nil
                || args.optString("name") != nil else {
            throw AutomationFailure("nothing to change: give name, wires, wire_names, rename or move_wire")
        }
        var problem: String?
        func check(_ refusal: String?) { if problem == nil { problem = refusal } }
        document.recordEdit(.schematic, "Change Bus") {
            if let name = args.optString("name") { check(document.setBusName(id, name)) }
            if problem == nil, let count = names?.count ?? args.optInt("wires") {
                check(document.setBusWireCount(id, count))
            }
            if problem == nil, let names, let current = document.bus(id) {
                for (place, name) in names.enumerated() where current.wires[place].name != name {
                    check(document.renameBusWire(bus: id, wire: current.wires[place].id, to: name))
                    if problem != nil { break }
                }
            }
            if problem == nil {
                for (old, new) in renames.sorted(by: { $0.key < $1.key }) {
                    guard let current = document.bus(id), let target = current.wires.first(where: { $0.name == old }) else {
                        problem = "no wire \(old) on this bus"
                        break
                    }
                    check(document.renameBusWire(bus: id, wire: target.id, to: new))
                    if problem != nil { break }
                }
            }
            if problem == nil, let moving, let current = document.bus(id) {
                let target = try? wire(current, moving)
                if let target { check(document.moveBusWire(bus: id, wire: target.id, to: args.optInt("move_to") ?? 0)) }
                else { problem = "no wire \(moving) on this bus" }
            }
        }
        document.clearAllSelections()
        if let problem {
            let now = document.bus(id).map { "; the bus now has wires " + $0.wires.map(\.name).joined(separator: ",") } ?? ""
            throw AutomationFailure(problem + " (changes before this one stay applied" + now + ")")
        }
        guard let now = document.bus(id) else { return "changed" }
        return "bus \(now.name) has \(now.wires.count) wires: " + now.wires.map(\.name).joined(separator: ",")
    }

    private static func setBusPort(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let bus = try bus(document, try args.string("bus"))
        let port = try busPort(bus, args.optInt("port"))
        var wanted = port.wireIDs
        if args.raw["wires"] != nil { wanted = Set(try args.strings("wires").map { try wire(bus, $0).id }) }
        if args.raw["add_wires"] != nil { wanted.formUnion(try args.strings("add_wires").map { try wire(bus, $0).id }) }
        if args.raw["remove_wires"] != nil { wanted.subtract(try args.strings("remove_wires").map { try wire(bus, $0).id }) }
        let side = try args.optString("side").map { name -> ChipLayout.Side in
            guard let side = [ChipLayout.Side.top, .bottom, .left, .right].first(where: { "\($0)" == name.lowercased() }) else {
                throw AutomationFailure("side is top or bottom (on a horizontal bus line), left or right (on a vertical one)")
            }
            guard let line = bus.line(port.line), BusPort.sides(for: line).contains(side) else {
                throw AutomationFailure("this port is on a \(bus.line(port.line)?.isHorizontal == false ? "vertical" : "horizontal") "
                                        + "bus line: its side is "
                                        + (bus.line(port.line)?.isHorizontal == false ? "left or right" : "top or bottom"))
            }
            return side
        }
        let along = args.optDouble("at").map { SchematicGrid.snap(CGFloat($0)) }
        guard wanted != port.wireIDs || side != nil || along != nil else {
            throw AutomationFailure("nothing to change: give wires, add_wires, remove_wires, side or at")
        }
        var problem: String?
        document.recordEdit(.schematic, "Change Port") {
            if wanted != port.wireIDs { problem = document.setPortWires(port.id, wanted) }
            if problem == nil, let side, document.port(port.id)?.port.side != side {
                problem = document.setPortSide(port.id, side)
            }
            if problem == nil, let along, !document.slidePort(port.id, to: along) {
                problem = "the port can't go to \(num(Double(along))): it must stay on its line, clear of its ends, "
                    + "the joints and the other ports"
            }
        }
        document.clearAllSelections()
        let now = document.port(port.id)
        let text = now.map { "port \(describe($0.port, in: $0.bus))" } ?? "the port"
        if let problem { throw AutomationFailure(problem + " (changes before this one stay applied; \(text))") }
        return text
    }

    /// "pins D0,D1 on the top, starting at 120" for get_design and the port tools.
    static func describe(_ port: BusPort, in bus: Bus) -> String {
        let pins = bus.wires.filter { port.wireIDs.contains($0.id) }.map(\.name).joined(separator: ",")
        let axis = bus.line(port.line)?.isHorizontal == false ? "y" : "x"
        return "pins \(pins); side \(port.side); first pin at \(axis)=\(num(Double(port.start)))"
    }

    private static func busPort(_ bus: Bus, _ number: Int?) throws -> BusPort {
        guard let number, bus.ports.indices.contains(number - 1) else {
            throw AutomationFailure("bus \(bus.name) has \(bus.ports.count) ports (numbered from 1); add one with add_bus_port")
        }
        return bus.ports[number - 1]
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
        case "junction": return .schematic(.junction(try context.find(document.junctions, rest, kind: "junction").id))
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
                                    + "bus:NAME, junction:ID, trace:ID, via:ID, hole:ID, text:ID or fp:U1")
        }
    }

    private static func delete(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let specs = try args.strings("items")
        // Ports are numbered per bus, so find them all before deleting any.
        let ports = try specs.filter { $0.lowercased().hasPrefix("port:") }.map { try portID($0, document) }
        let items = try specs.filter { !$0.lowercased().hasPrefix("port:") }.map { try item($0, context, forDelete: true) }
        guard !items.isEmpty || !ports.isEmpty else { throw AutomationFailure("items is empty") }
        var removed = 0
        var loose = 0
        document.recordEdit(.schematic, "Delete") {
            document.clearAllSelections()
            for id in ports {
                loose += document.wiresOnPins(port: id)
                document.selectedPortID = id
                if document.deleteSchematicSelection() {
                    // The app asks first when wires are on its pins; the agent has said to delete.
                    if document.pendingRemoval != nil { document.confirmPendingRemoval() }
                    removed += 1
                }
                document.clearAllSelections()
            }
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
            + (loose > 0 ? "; \(loose) wire\(loose == 1 ? "" : "s") on the port pins now have loose ends" : "")
    }

    /// The port "port:BUS:N" names (N counts from 1, as get_design lists them).
    private static func portID(_ spec: String, _ document: SchematicDocument) throws -> UUID {
        let parts = spec.split(separator: ":", omittingEmptySubsequences: false).map(String.init)
        guard parts.count == 3, let number = Int(parts[2]) else { throw AutomationFailure("a port looks like port:BUS:N, e.g. port:DB:1") }
        return try busPort(try bus(document, parts[1]), number).id
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
