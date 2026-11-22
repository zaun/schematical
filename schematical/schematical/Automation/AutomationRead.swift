//
//  AutomationRead.swift
//  schematical
//
//  Automation tools that read the design and the part catalog, and draw it.
//

import AppKit
import Foundation

extension AutomationTools {
    static let readTools: [AutomationTool] = [
        AutomationTool(
            name: "get_design",
            summary: "The whole schematic: every part (reference, part number, centre, size, bounds, PCB placement, "
                + "pins with * on wired ones), wires with their routes, junctions, net labels and notes with their "
                + "bounds, buses with their ports. geometry=true adds every pin's tip (where a wire meets it) and "
                + "side, and the pin tips of bus ports. format=json always has pin tips.",
            properties: [
                "format": Schema.string("text (default) or json", options: ["text", "json"]),
                "geometry": Schema.boolean("Text: add pin tips and sides, bus port pin tips (default false)"),
            ],
            readOnly: true,
            run: { context, args in
                let context = context!
                return try args.choice("format", among: ["text", "json"], default: "text") == "json"
                    ? Out.json(designJSON(context)) : Out.text(designText(context, geometry: args.optBool("geometry") == true))
            }),
        AutomationTool(
            name: "get_nets",
            summary: "The schematic's nets: each with its name (from a net label, when it has one) and the pins on it. "
                + "Nets with one pin are listed as dangling.",
            readOnly: true,
            run: { context, _ in Out.text(netsText(context!)) }),
        AutomationTool(
            name: "search_parts",
            summary: "Search the part catalog. All words in query must match the name, description, function, "
                + "manufacturer or package. With no query and no filter, lists the categories.",
            properties: [
                "query": Schema.string("Words, e.g. \"nand 74hc\" or \"led\""),
                "category": Schema.string("Drawer category or subcategory, e.g. \"Memory\" (partial match)"),
                "package": Schema.string("Package, e.g. DIP, SOIC, Radial"),
                "min_pins": Schema.integer("Fewest pins"),
                "max_pins": Schema.integer("Most pins"),
                "limit": Schema.integer("Results to return (default 15, at most 60)"),
            ],
            needsWindow: false, readOnly: true,
            run: { _, args in Out.text(searchParts(args)) }),
        AutomationTool(
            name: "get_part",
            summary: "Everything about one catalog part: description, pins (designator, name, role, group), "
                + "footprint choices, options (LED colors, header genders), datasheet link.",
            properties: ["part": Schema.string("Part number, e.g. SN74HC00N")],
            required: ["part"], needsWindow: false, readOnly: true,
            run: { _, args in Out.text(try partText(try args.string("part"))) }),
        AutomationTool(
            name: "get_selection",
            summary: "What is selected now on the schematic and on the PCB.",
            readOnly: true,
            run: { context, _ in Out.text(selectionText(context!)) }),
        AutomationTool(
            name: "get_board",
            summary: "The PCB: size, type, copper layers (type, net), grid, footprint placements, traces, vias, "
                + "holes, texts, and the design rules the check uses.",
            readOnly: true,
            run: { context, _ in Out.text(boardText(context!)) }),
        AutomationTool(
            name: "run_drc",
            summary: "Run the design rule check on the PCB. One issue per line: unrouted nets, shorts, clearance, "
                + "off-board parts, drill, silkscreen. Run it after routing.",
            readOnly: true,
            run: { context, _ in
                let issues = context!.document.designRuleCheck(rules: context!.document.board.rules)
                if issues.isEmpty { return Out.text("no issues") }
                let lines = issues.map { "\($0.index). [\($0.kind)] \($0.reference): \($0.issue)" }
                return Out.text("\(issues.count) issues\n" + lines.joined(separator: "\n"))
            }),
        AutomationTool(
            name: "render",
            summary: "A picture of the design (PNG) so you can check your work. view: schematic, pcb (the editing "
                + "page with ratsnest lines), front or back (the made board). region crops to a rectangle given as "
                + "fractions [x0, y0, x1, y1] of the whole picture, to zoom in on a part.",
            properties: [
                "view": Schema.string("schematic (default), pcb, front, back",
                                      options: ["schematic", "pcb", "front", "back"]),
                "region": Schema.points("Crop: [[x0, y0], [x1, y1]] as fractions 0…1 of the picture"),
                "show_ratsnest": Schema.boolean("PCB view: draw the unrouted-net lines (default true)"),
            ],
            readOnly: true,
            run: { context, args in try render(context!, args) }),
    ]

    // MARK: Design

    private static func options(_ chip: PlacedChip) -> String {
        var parts: [String] = []
        if let color = chip.color { parts.append("color=\(color)") }
        if let ohms = chip.ohms { parts.append("ohms=\(num(ohms))") }
        if let farads = chip.farads { parts.append("farads=\(farads)") }
        if let gender = chip.gender { parts.append("gender=\(gender)") }
        if let footprint = chip.footprint { parts.append("footprint=\(footprint.id)") }
        if chip.isFlipped { parts.append("flipped") }
        if chip.rotation != 0 { parts.append("rotation=\(chip.rotation)") }
        if !chip.switchesOn.isEmpty {
            parts.append("switches=" + chip.switchesOn.map { $0 ? "on" : "off" }.joined(separator: ","))
        }
        return parts.joined(separator: " ")
    }

    /// "(x1,y1)-(x2,y2)": a rectangle's corners.
    private static func box(_ rect: CGRect) -> String {
        "(\(num(rect.minX)),\(num(rect.minY)))-(\(num(rect.maxX)),\(num(rect.maxY)))"
    }

    /// A part's body with its pin stubs, in canvas units.
    private static func bounds(of chip: PlacedChip) -> CGRect {
        CGRect(origin: chip.origin, size: chip.layout.size)
    }

    private static func designText(_ context: AutomationContext, geometry: Bool) -> String {
        let document = context.document
        var out = ["WINDOW \(context.shortID)  parts=\(document.chips.count) wires=\(document.wires.count)"
                   + (document.isPlaying ? "  SIMULATION RUNNING (read-only)" : "")]
        out.append("PARTS  (centre x,y on the schematic; * = wired pin)")
        for chip in document.chips {
            let wired = document.connectedPins[chip.id] ?? []
            let size = chip.layout.size
            let centre = context.centre(of: chip)
            let pins = chip.part.pins.filter { !$0.isHidden }.map { pin -> String in
                var text = "\(pin.designator):\(pin.name)\(wired.contains(pin.index) ? "*" : "")"
                if geometry, let slot = chip.layout.slots.first(where: { $0.pin.index == pin.index }) {
                    let tip = chip.tip(of: slot)
                    text += "@(\(num(tip.x)),\(num(tip.y)))\(slot.side)"
                }
                return text
            }.joined(separator: " ")
            var line = "\(chip.reference) \(chip.part.name) centre=(\(num(centre.x)),\(num(centre.y))) "
                + "size=\(num(size.width))x\(num(size.height)) body=\(box(bounds(of: chip)))"
            if let board = chip.board {
                line += " pcb=(\(num(board.centre.x)),\(num(board.centre.y)))mm rot=\(board.rotation) \(board.side.rawValue)"
                    + (board.locked ? " locked" : "")
            }
            let extra = options(chip)
            if !extra.isEmpty { line += " " + extra }
            out.append(line)
            out.append("    pins: \(pins)")
        }
        out.append("WIRES")
        for wire in document.wires {
            let route = document.path(for: wire).map { "(\(num($0.x)),\(num($0.y)))" }.joined(separator: " ")
            out.append("\(context.short(wire.id))  \(context.endName(wire.start)) -> \(context.endName(wire.end))"
                       + (route.isEmpty ? "" : "  via \(route)"))
        }
        if !document.junctions.isEmpty {
            out.append("JUNCTIONS")
            for junction in document.junctions {
                out.append("junction:\(context.short(junction.id))  at (\(num(junction.point.x)),\(num(junction.point.y)))"
                           + (junction.isLoose ? "  loose end" : ""))
            }
        }
        if !document.netLabels.isEmpty {
            out.append("NET LABELS")
            for label in document.netLabels {
                out.append("\(context.short(label.id))  \(label.name) \(label.style.rawValue) point=(\(num(label.origin.x)),\(num(label.origin.y)))"
                           + " runs \(label.isFlipped ? "left" : "right") bounds=\(box(label.rect))")
            }
        }
        if !document.notes.isEmpty {
            out.append("NOTES")
            for note in document.notes {
                out.append("\(context.short(note.id))  bounds=\(box(note.rect)) size \(note.size): "
                           + note.text.replacingOccurrences(of: "\n", with: " ⏎ "))
            }
        }
        if !document.buses.isEmpty {
            out.append("BUSES")
            for bus in document.buses {
                let wires = bus.wires.map(\.name).joined(separator: ",")
                let lines = bus.lines.map { "(\(num($0.a.x)),\(num($0.a.y)))-(\(num($0.b.x)),\(num($0.b.y)))" }
                    .joined(separator: " ")
                out.append("\(bus.name): wires \(wires); lines \(lines); ports \(bus.ports.count)")
                for (index, port) in bus.ports.enumerated() {
                    var line = "  port \(index + 1): \(describe(port, in: bus))"
                    if geometry {
                        let tips = bus.wires.compactMap { wire in
                            bus.pinTip(port, wire: wire.id).map { "\(wire.name)@(\(num($0.x)),\(num($0.y)))" }
                        }
                        line += "; pin tips " + tips.joined(separator: " ")
                    }
                    out.append(line)
                }
            }
        }
        return out.joined(separator: "\n")
    }

    private static func rectJSON(_ rect: CGRect) -> [String: Double] {
        ["x1": rect.minX, "y1": rect.minY, "x2": rect.maxX, "y2": rect.maxY]
    }

    /// A pin's "activeLow" as the part file writes it (plan-39-B): true when
    /// every alternative of the name is active low, the flagged alternatives
    /// when some are, nil when none.
    private static func activeLowJSON(_ pin: ChipPin) -> Any? {
        let alternatives = pin.alternatives
        let flagged = alternatives.filter(\.isActiveLow).map(\.text)
        if flagged.isEmpty { return nil }
        if flagged.count == alternatives.count { return true }
        var unique: [String] = []
        for text in flagged where !unique.contains(text) { unique.append(text) }
        return unique
    }

    /// " low", " low=A;B" or "" for `get_part`'s pin lines.
    private static func activeLowText(_ pin: ChipPin) -> String {
        switch activeLowJSON(pin) {
        case is Bool: return " low"
        case let list as [String]: return " low=" + list.joined(separator: ";")
        default: return ""
        }
    }

    private static func designJSON(_ context: AutomationContext) -> [String: Any] {
        let document = context.document
        let parts: [[String: Any]] = document.chips.map { chip in
            let wired = document.connectedPins[chip.id] ?? []
            let centre = context.centre(of: chip)
            var part: [String: Any] = [
                "reference": chip.reference, "part": chip.part.name,
                "x": centre.x, "y": centre.y,
                "width": chip.layout.size.width, "height": chip.layout.size.height,
                "flipped": chip.isFlipped, "rotation": chip.rotation,
                "bounds": rectJSON(bounds(of: chip)),
                "pins": chip.part.pins.filter { !$0.isHidden }.map { pin -> [String: Any] in
                    var entry: [String: Any] = ["designator": pin.designator, "name": pin.name,
                                                "wired": wired.contains(pin.index)]
                    if let slot = chip.layout.slots.first(where: { $0.pin.index == pin.index }) {
                        entry["side"] = "\(slot.side)"
                    }
                    if let activeLow = activeLowJSON(pin) { entry["activeLow"] = activeLow }
                    if let tip = document.pinTip(PinRef(chipID: chip.id, index: pin.index)) {
                        entry["x"] = tip.x
                        entry["y"] = tip.y
                    }
                    return entry
                },
            ]
            if let board = chip.board {
                part["pcb"] = ["x": board.centre.x, "y": board.centre.y, "rotation": board.rotation,
                               "side": board.side.rawValue, "locked": board.locked]
            }
            if let color = chip.color { part["color"] = color }
            if let ohms = chip.ohms { part["ohms"] = ohms }
            if let farads = chip.farads { part["farads"] = farads }
            if let footprint = chip.footprint { part["footprint"] = footprint.id }
            return part
        }
        return [
            "window": context.shortID,
            "parts": parts,
            "wires": document.wires.map { wire in
                ["id": context.short(wire.id), "from": context.endName(wire.start), "to": context.endName(wire.end),
                 "route": document.path(for: wire).map { [$0.x, $0.y] }] as [String: Any]
            },
            "junctions": document.junctions.map {
                ["id": context.short($0.id), "x": $0.point.x, "y": $0.point.y, "loose": $0.isLoose] as [String: Any]
            },
            "labels": document.netLabels.map {
                ["id": context.short($0.id), "name": $0.name, "style": $0.style.rawValue,
                 "x": $0.origin.x, "y": $0.origin.y, "flipped": $0.isFlipped,
                 "bounds": rectJSON($0.rect)] as [String: Any]
            },
            "notes": document.notes.map {
                ["id": context.short($0.id), "text": $0.text, "x": $0.origin.x, "y": $0.origin.y,
                 "size": $0.size, "framed": $0.framed, "bounds": rectJSON($0.rect)] as [String: Any]
            },
            "buses": document.buses.map { bus in
                ["name": bus.name, "wires": bus.wires.map(\.name),
                 "lines": bus.lines.map { [[$0.a.x, $0.a.y], [$0.b.x, $0.b.y]] },
                 "ports": bus.ports.enumerated().map { index, port -> [String: Any] in
                     ["number": index + 1, "side": "\(port.side)",
                      "bounds": bus.body(of: port).map { rectJSON($0) } as Any,
                      "pins": bus.wires.compactMap { wire -> [String: Any]? in
                          bus.pinTip(port, wire: wire.id).map { ["wire": wire.name, "x": $0.x, "y": $0.y] }
                      }]
                 }] as [String: Any]
            },
        ]
    }

    // MARK: Nets

    private static func netsText(_ context: AutomationContext) -> String {
        let document = context.document
        let netlist = NetlistBuilder.build(chips: document.chips, wires: document.wires, buses: document.buses,
                                           labels: document.netLabels)
        var pinsOfNet: [Int: [String]] = [:]
        for connection in netlist.connections {
            let chip = document.chips.first { $0.id == netlist.chipIDs[connection.part] }
            let name = chip?.part.pins.first { $0.designator == connection.pin }?.name
            pinsOfNet[connection.net, default: []].append(
                "\(chip?.reference ?? "?").\(connection.pin)" + (name.map { $0 == connection.pin ? "" : "(\($0))" } ?? ""))
        }
        var names: [Int: String] = [:]
        for wire in document.wires {
            guard let net = netlist.netOfWire[wire.id] else { continue }
            for end in [wire.start, wire.end] {
                if case let .label(id) = end, let label = document.netLabels.first(where: { $0.id == id }) {
                    names[net] = label.name
                }
            }
        }
        var lines: [String] = []
        var dangling = 0
        for net in pinsOfNet.keys.sorted() {
            let pins = pinsOfNet[net]!.sorted()
            if pins.count < 2, names[net] == nil { dangling += 1; continue }
            lines.append("\(names[net].map { "[\($0)] " } ?? "")\(pins.joined(separator: " "))")
        }
        var header = "\(lines.count) nets"
        if dangling > 0 { header += "; \(dangling) pins are on a net of their own (unconnected)" }
        return ([header] + lines).joined(separator: "\n")
    }

    // MARK: Catalog

    private static func searchParts(_ args: AutomationArgs) -> String {
        let store = PartCatalogStore.shared
        let words = (args.optString("query") ?? "").lowercased().split(separator: " ").map(String.init)
        let category = args.optString("category")?.lowercased()
        let package = args.optString("package")?.lowercased()
        let minPins = args.optInt("min_pins"), maxPins = args.optInt("max_pins")
        if words.isEmpty, category == nil, package == nil, minPins == nil, maxPins == nil {
            let categories = store.categories.joined(separator: ", ")
            let supplies = (["GND"] + SupplyChoice.names).joined(separator: ", ")
            return """
                categories: \(categories)
                (pass query and/or category, package, min_pins, max_pins)
                power symbols, placed by name: \(supplies) (or any voltage up to 300 V, such as 170V or 3V6), PULLDOWN, PULLUP-<voltage>
                """
        }
        let limit = min(max(args.optInt("limit") ?? 15, 1), 60)
        let hits = store.allParts.filter { part in
            let haystack = [part.name, part.description, part.function, part.manufacturer, part.package]
                .joined(separator: " ").lowercased()
            guard words.allSatisfy({ haystack.contains($0) }) else { return false }
            if let category, !part.drawer.contains(where: { $0.lowercased().contains(category) }) { return false }
            if let package, !part.package.lowercased().contains(package) { return false }
            if let minPins, part.pins.count < minPins { return false }
            if let maxPins, part.pins.count > maxPins { return false }
            return true
        }
        if hits.isEmpty { return "no match" }
        var lines = hits.prefix(limit).map { part in
            "\(part.name) [\(part.package)] \(part.description) — \(part.pins.count) pins"
                + (part.drawer.isEmpty ? "" : " — " + part.drawer.joined(separator: " > "))
                + (part.flags.obsolete ? " (obsolete)" : "")
        }
        if hits.count > limit { lines.append("… \(hits.count - limit) more; narrow the search") }
        return "\(hits.count) matches\n" + lines.joined(separator: "\n")
    }

    private static func partText(_ name: String) throws -> String {
        guard let part = PartCatalogStore.shared.part(named: name) else {
            throw AutomationFailure("no part \(name); use search_parts")
        }
        var out = ["\(part.name) [\(part.package)] \(part.description)"]
        out.append("manufacturer: \(part.manufacturer)   function: \(part.function)   simulated: \(part.isSimulated)")
        if !part.drawer.isEmpty { out.append("category: " + part.drawer.joined(separator: " > ")) }
        if let url = part.datasheetURL { out.append("datasheet: \(url.absoluteString)") }
        if !part.colors.isEmpty { out.append("colors: " + part.colors.joined(separator: ", ")) }
        if !part.genders.isEmpty { out.append("genders: " + part.genders.joined(separator: ", ")) }
        if part.footprint != nil || !part.footprintChoices.isEmpty {
            let choices = part.footprintChoices.map { "\($0.package)=\($0.footprint.id)" }
            out.append("footprint: \(part.footprint?.id ?? "none")"
                       + (choices.isEmpty ? "" : "; choices: " + choices.joined(separator: ", ")))
        } else {
            out.append("footprint: none")
        }
        out.append("pins (designator=name [role] group low):")
        for pin in part.pins where !pin.isHidden {
            out.append("  \(pin.designator)=\(pin.name) [\(pin.role)]"
                       + (pin.group == ChipPin.defaultGroup ? "" : " \(pin.group)")
                       + activeLowText(pin))
        }
        return out.joined(separator: "\n")
    }

    // MARK: Selection and board

    private static func selectionText(_ context: AutomationContext) -> String {
        let document = context.document
        func name(_ item: SchematicItem) -> String {
            switch item {
            case let .chip(id): return document.chips.first { $0.id == id }?.reference ?? "part"
            case let .wire(id): return "wire:\(context.short(id))"
            case let .junction(id): return "junction:\(context.short(id))"
            case let .note(id): return "note:\(context.short(id))"
            case let .label(id): return "label:" + (document.netLabels.first { $0.id == id }?.name ?? context.short(id))
            case let .bus(id): return "bus:" + (document.bus(id)?.name ?? context.short(id))
            }
        }
        func name(_ item: BoardItem) -> String {
            switch item {
            case let .footprint(id): return "fp:" + (document.chips.first { $0.id == id }?.reference ?? "?")
            case let .text(id): return "text:\(context.short(id))"
            case let .trace(id): return "trace:\(context.short(id))"
            case let .via(id): return "via:\(context.short(id))"
            case let .hole(id): return "hole:\(context.short(id))"
            case let .pour(id): return "pour:\(context.short(id))"
            case let .keepout(id): return "keepout:\(context.short(id))"
            }
        }
        let schematic = document.schematicSelection.items.map(name).sorted()
        let board = document.boardSelection.items.map(name).sorted()
        return "schematic: \(schematic.isEmpty ? "nothing" : schematic.joined(separator: " "))\n"
            + "pcb: \(board.isEmpty ? "nothing" : board.joined(separator: " "))"
    }

    private static func boardText(_ context: AutomationContext) -> String {
        let document = context.document
        let board = document.board
        var out = ["BOARD type=\(board.type.rawValue) size=\(num(board.width))x\(num(board.height))mm "
                   + "layers=\(board.layers) grid=\(num(board.grid))mm units=\(board.units.rawValue) color=\(board.color.rawValue)"]
        out.append("LAYERS")
        for layer in board.copperLayers {
            let style = board.style(of: layer)
            var line = "  \(layer.rawValue): \(style.type.rawValue)"
            if style.type == .plane {
                line += " net=" + (style.net.map { $0.fileValue } ?? "none")
                if let clearance = style.clearance { line += " clearance=\(num(clearance))mm" }
            }
            out.append(line)
        }
        out.append("FOOTPRINTS (centre mm)")
        for chip in document.chips {
            guard let placement = chip.board else { continue }
            var line = "  \(chip.reference) (\(num(placement.centre.x)),\(num(placement.centre.y))) rot=\(placement.rotation) \(placement.side.rawValue)"
            if let bounds = chip.boardBounds { line += " bounds=\(num(bounds.width))x\(num(bounds.height))" }
            if placement.locked { line += " locked" }
            out.append(line)
        }
        out.append("TRACES")
        for trace in document.traces {
            out.append("  \(context.short(trace.id)) \(trace.layer.rawValue) \(trace.width)mil "
                       + trace.points.map { "(\(num($0.x)),\(num($0.y)))" }.joined(separator: "→")
                       + (trace.locked ? " locked" : ""))
        }
        out.append("VIAS")
        for via in document.vias {
            out.append("  \(context.short(via.id)) (\(num(via.centre.x)),\(num(via.centre.y))) drill=\(num(via.drill)) diameter=\(num(via.diameter))")
        }
        out.append("HOLES")
        for hole in document.holes {
            out.append("  \(context.short(hole.id)) (\(num(hole.centre.x)),\(num(hole.centre.y))) drill=\(num(hole.drill)) diameter=\(num(hole.diameter)) plated=\(hole.plated)")
        }
        out.append("TEXTS")
        for text in document.texts {
            out.append("  \(context.short(text.id)) (\(num(text.anchor.x)),\(num(text.anchor.y))) \(text.layer.rawValue) \(text.side.rawValue) \(text.size)mil: \(text.text)")
        }
        let rules = document.board.rules
        out.append("RULES minTrace=\(num(rules.minTraceWidth)) minClearance=\(num(rules.minClearance)) "
                   + "minDrill=\(num(rules.minDrill)) minViaDrill=\(num(rules.minViaDrill)) "
                   + "minViaDiameter=\(num(rules.minViaDiameter)) minAnnularRing=\(num(rules.minAnnularRing)) "
                   + "minCopperToEdge=\(num(rules.minCopperToEdge)) (mm)")
        return out.joined(separator: "\n")
    }

    // MARK: Render

    private static func render(_ context: AutomationContext, _ args: AutomationArgs) throws -> [[String: Any]] {
        let name = try args.choice("view", among: ["schematic", "pcb", "front", "back"], default: "schematic")
        guard let view = ImageExportView(rawValue: name) else { throw AutomationFailure("bad view") }
        var options = ImageExportOptions()
        options.view = view
        options.format = .png
        options.background = .white
        if args.optBool("show_ratsnest") == false { options.hiddenLayers = [.ratsnest] }
        else if view == .pcb { options.hiddenLayers = [] }

        var region: (x0: Double, y0: Double, x1: Double, y1: Double)?
        if args.raw["region"] != nil {
            let corners = try args.points("region")
            guard corners.count == 2 else { throw AutomationFailure("region must be [[x0, y0], [x1, y1]]") }
            let r = (x0: min(corners[0].x, corners[1].x), y0: min(corners[0].y, corners[1].y),
                     x1: max(corners[0].x, corners[1].x), y1: max(corners[0].y, corners[1].y))
            guard r.x0 >= 0, r.y0 >= 0, r.x1 <= 1, r.y1 <= 1, r.x1 - r.x0 > 0.01, r.y1 - r.y0 > 0.01 else {
                throw AutomationFailure("region corners must be fractions within 0…1 and enclose some area")
            }
            region = (Double(r.x0), Double(r.y0), Double(r.x1), Double(r.y1))
        }

        // Enough pixels to read: the (cropped) longest side ≥ ~1100 px.
        options.scale = 1
        guard let base = ImageExport.pixelSize(of: context.document, options) else {
            throw AutomationFailure("nothing to draw yet (the design is empty)")
        }
        let share = region.map { max($0.x1 - $0.x0, $0.y1 - $0.y0) } ?? 1
        let longest = Double(max(base.width, base.height)) * share
        options.scale = ImageExportOptions.scales.first { longest * Double($0) >= 1100 } ?? 8

        guard var data = ImageExport.data(for: context.document, options.normalized()) else {
            throw AutomationFailure("nothing to draw yet (the design is empty)")
        }
        if let region, let rep = NSBitmapImageRep(data: data), let image = rep.cgImage {
            let w = Double(image.width), h = Double(image.height)
            let crop = CGRect(x: region.x0 * w, y: region.y0 * h,
                              width: (region.x1 - region.x0) * w, height: (region.y1 - region.y0) * h).integral
            if let cropped = image.cropping(to: crop),
               let png = NSBitmapImageRep(cgImage: cropped).representation(using: .png, properties: [:]) {
                data = png
            }
        }
        return Out.image(data, caption: "\(name) view of window \(context.shortID)"
                         + (region == nil ? "" : " (cropped)"))
    }
}
