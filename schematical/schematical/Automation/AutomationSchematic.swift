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
                "view": Schema.string("block (default) or gate: a gate IC as separate gates U1-A, U1-B, … (see set_part)"),
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
            summary: "Move a part on the schematic: its centre to x,y, or by dx,dy. Its wires follow. "
                + "A part in Gate view moves whole (U1), or one gate at a time (U1-A).",
            properties: [
                "reference": Schema.string("e.g. U1, or one gate of a part in Gate view: U1-A"),
                "x": Schema.number("New centre x"),
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
            summary: "Change an existing net label: rename it (up to 8 characters, no spaces; same name = same net), "
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
            name: "set_document_info",
            summary: "Change the schematic's Document Info (Window > Document Info): title, author, revision, "
                + "description, created and updated dates (ISO 8601, e.g. 2026-11-25T14:07:00Z; only with "
                + "automatic_dates false, since saves set them while it is on), and title_block true/false to show or "
                + "hide the title block on the schematic (bottom-right of the drawing; it is item \"titleblock\" for "
                + "move, select and delete, and delete only hides it). One undo step.",
            properties: [
                "title": Schema.string("Title"), "author": Schema.string("Author"), "revision": Schema.string("Revision (free text)"),
                "description": Schema.string("Description; may have line breaks"),
                "created": Schema.string("Created date, ISO 8601; empty clears it"),
                "updated": Schema.string("Updated date, ISO 8601; empty clears it"),
                "automatic_dates": Schema.boolean("Saves set the dates (default for a new document)"),
                "title_block": Schema.boolean("Show (true) or hide (false) the title block on the schematic"),
            ],
            required: [], edits: true,
            run: { context, args in Out.text(try setDocumentInfo(context!, args)) }),
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
            properties: ["reference": Schema.string("e.g. U1, or one gate of a part in Gate view: U1-A")],
            required: ["reference"], edits: true,
            run: { context, args in
                let (chip, key) = try context!.unit(try args.string("reference"))
                if let key {
                    context!.document.flipUnit(UnitRef(chipID: chip.id, key: key))
                    return Out.text("flipped \(chip.reference)-\(key)")
                }
                guard !chip.isSplit else {
                    throw AutomationFailure("\(chip.reference) is drawn as units; name one: \(context!.unitNames(chip))")
                }
                guard chip.symbol.canFlip else { throw AutomationFailure("\(chip.reference) can't be flipped") }
                context!.document.flipChip(chip.id)
                return Out.text("flipped \(chip.reference)")
            }),
        AutomationTool(
            name: "rotate_part",
            summary: "Turn a part drawn as its own symbol a quarter turn on the schematic: an LED, diode, resistor, "
                + "capacitor, crystal, inductor, fuse, buzzer, transistor, test point or barrel jack (its left and "
                + "right pins go to the top and bottom), or one gate of a part in Gate view (U1-A). "
                + "Clockwise unless counterclockwise is true.",
            properties: ["reference": Schema.string("e.g. D1, or U1-A"),
                         "counterclockwise": Schema.boolean("Turn it the other way")],
            required: ["reference"], edits: true,
            run: { context, args in
                let (chip, key) = try context!.unit(try args.string("reference"))
                let counterclockwise = args.optBool("counterclockwise") == true
                if let key {
                    guard chip.piece(forUnit: key)?.symbol.canRotate == true else {
                        throw AutomationFailure("\(chip.reference)-\(key) can't be rotated: it is a block")
                    }
                    context!.document.rotateUnit(UnitRef(chipID: chip.id, key: key), counterclockwise: counterclockwise)
                    let turned = context!.document.chips.first { $0.id == chip.id }?
                        .unitPlacements.first { $0.key == key }?.rotation ?? 0
                    return Out.text("rotated \(chip.reference)-\(key) to \(turned)°")
                }
                guard !chip.isSplit else {
                    throw AutomationFailure("\(chip.reference) is drawn as units; name one: \(context!.unitNames(chip))")
                }
                guard chip.symbol.canRotate else {
                    throw AutomationFailure("\(chip.reference) can't be rotated: only parts drawn as their own symbol turn "
                        + "(LEDs, diodes, resistors, capacitors, crystals, inductors, fuses, buzzers, transistors, "
                        + "test points, barrel jacks)")
                }
                context!.document.rotateChip(chip.id, counterclockwise: counterclockwise)
                let turned = context!.document.chips.first { $0.id == chip.id }?.rotation ?? 0
                return Out.text("rotated \(chip.reference) to \(turned)°")
            }),
        AutomationTool(
            name: "set_part",
            summary: "Change a placed part: new_reference (\"U5\"), color, ohms, farads, footprint, gender, "
                + "show_reference, socket_mode (ic, ic+socket, socket-only), socket (a socket part name; "
                + "see search_parts) and view (a gate IC as one block or as separate gates U1-A, U1-B, … plus "
                + "U1-P for its power pins; the wires stay on their pins).",
            properties: [
                "reference": Schema.string("e.g. R1"),
                "new_reference": Schema.string("Rename, e.g. R7 (a letter prefix and a number)"),
                "color": Schema.string("LED color"), "ohms": Schema.number("Resistance"),
                "farads": Schema.number("Capacitance"), "footprint": Schema.string("Package name or footprint id"),
                "gender": Schema.string("Header gender"),
                "show_reference": Schema.boolean("Show the reference label"),
                "socket_mode": Schema.string("ic, ic+socket or socket-only (a DIP or PLCC chip a socket fits)"),
                "socket": Schema.string("The socket part's name (a mode without it picks the generic socket)"),
                "view": Schema.string("block or gate (gate ICs only)"),
            ],
            required: ["reference"], edits: true,
            run: { context, args in Out.text(try setPart(context!, args)) }),
        AutomationTool(
            name: "add_net_label",
            summary: "Add a net label at x,y. Labels with the same name are one net. Connect a pin to it with "
                + "connect(\"U1.3\", \"label:CLK\"). Names are up to 8 characters.",
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
                + "(up to 8 characters) and a number of wires (2–999, default 8). Wires are named B0, B1, B2… (rename or add some with set_bus); "
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
                + "wires, or move a wire to another place. Wire names are up to 8 characters, no spaces; two wires may share a "
                + "name (each is still its own net, but \"bus:NAME:WIRE\" and rename then pick the first). Refused when the bus type sets the wires, or when a port still uses a wire the "
                + "change would drop. A new wire has no pin yet: give it one with set_bus_port.",
            properties: [
                "bus": Schema.string("Bus name"), "name": Schema.string("New bus name (up to 8 characters)"),
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
            summary: "Delete items: parts by reference (\"U1\"; wires on its pins stay, with loose ends; a part in "
                + "Gate view goes whole, never one gate), "
                + "\"wire:ID\", \"label:NAME\" or \"label:ID\", \"note:ID\", \"junction:ID\", \"bus:NAME\", \"port:BUS:N\" (a bus port, numbered from 1 in get_design; "
                + "wires on its pins stay, with loose ends), and on the PCB "
                + "\"trace:ID\", \"via:ID\", \"hole:ID\", \"text:ID\". Ids are printed by get_design and get_board.",
            properties: ["items": Schema.strings("Items to delete")], required: ["items"], edits: true,
            run: { context, args in Out.text(try delete(context!, args)) }),
        AutomationTool(
            name: "select",
            summary: "Select items so the user sees them (and later tools can act on the selection). Same item "
                + "names as delete, plus \"fp:U1\" for a footprint and \"U1-A\" for one gate of a part in "
                + "Gate view. An empty list clears the selection.",
            properties: [
                "items": Schema.strings("Items to select"),
                "add": Schema.boolean("Add to the selection instead of replacing it"),
            ],
            required: ["items"],
            run: { context, args in Out.text(try select(context!, args)) }),
    ]

    // MARK: Placing

    static func footprintID(_ part: CatalogPart, _ name: String) throws -> String {
        if let choice = part.footprintChoices.first(where: {
            $0.package.caseInsensitiveCompare(name) == .orderedSame || $0.footprint.id == name
        }) { return choice.footprint.id }
        if part.footprint?.id == name { return name }
        let known = part.footprintChoices.map { "\($0.package)=\($0.footprint.id)" }
        throw AutomationFailure("\(part.name) has no footprint \(name)"
                                + (known.isEmpty ? "" : "; choices: " + known.joined(separator: ", ")))
    }

    static func placePart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
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
        // Checked before anything is placed, so a wrong view places nothing.
        let wantsSplit = try args.optString("view").map { try wantsUnits(part, named: part.name, $0) } ?? false
        let before = document.chips.count
        var units = ""
        // One undo step: the part, and its view when one is asked for.
        let added: PlacedChip? = document.recordEdit(.schematic, "Add Part") {
            document.placeChip(part: part, symbol: symbol(for: part), centeredAt: point,
                               color: args.optString("color"), footprintID: footprint,
                               gender: args.optString("gender"), ohms: args.optDouble("ohms"),
                               farads: args.optDouble("farads"))
            guard document.chips.count > before, var placed = document.chips.last else { return nil }
            if wantsSplit {
                document.setPartSplit(placed.id, true)
                placed = document.chips.first { $0.id == placed.id } ?? placed
                units = "; units: " + context.unitNames(placed)
            }
            return placed
        }
        guard let placed = added else { throw AutomationFailure("the part was not placed") }
        let centre = context.centre(of: placed)
        let pins = placed.part.pins.filter { !$0.isHidden }
        let listing = pins.count <= 16
            ? "; pins: " + pins.map { "\($0.designator)=\($0.name)" }.joined(separator: " ")
            : "; \(pins.count) pins (see get_part)"
        return "placed \(placed.reference) (\(part.name)) centre=(\(num(centre.x)),\(num(centre.y))) "
            + "size=\(num(placed.layout.size.width))x\(num(placed.layout.size.height))" + listing + units
    }

    /// `view`'s meaning for a part: false for "block", true for the part's
    /// unit view ("gate" for a gate IC; "units" always works). A part with no
    /// units has no other view.
    static func wantsUnits(_ part: CatalogPart, named label: String, _ view: String) throws -> Bool {
        let wanted = view.lowercased()
        if wanted == "block" { return false }
        guard !part.units.isEmpty else { throw AutomationFailure("\(label) has no other view") }
        let name = (part.unitViewName ?? "units").lowercased()
        guard wanted == name || wanted == "units" else {
            throw AutomationFailure("view is block or \(name), got “\(view)”")
        }
        return true
    }

    // MARK: Wiring

    /// One end of a wire, from its name.
    static func anchor(_ spec: String, _ context: AutomationContext) throws -> LineAnchor {
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
    static func orthogonal(_ points: [CGPoint], start: RouteAxis, end: RouteAxis?) -> [CGPoint] {
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

    static func connect(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
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

    static func disconnect(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
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

}
