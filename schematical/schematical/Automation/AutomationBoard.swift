//
//  AutomationBoard.swift
//  schematical
//
//  Automation tools that edit the PCB page. Positions are board millimetres,
//  y down, from the board's top-left.
//

import CoreGraphics
import Foundation

extension AutomationTools {
    static let boardTools: [AutomationTool] = [
        AutomationTool(
            name: "set_board",
            summary: "Change the board: width and height (mm), type (a free rectangle or a standard card; see "
                + "get_board), copper layers (1, 2, 4 or 6), grid (mm), units (mm or in), color.",
            properties: [
                "width": Schema.number("mm"), "height": Schema.number("mm"),
                "type": Schema.string("Board type id, e.g. custom"),
                "layers": Schema.integer("1, 2, 4 or 6"), "grid": Schema.number("Component grid, mm"),
                "units": Schema.string("mm or in", options: ["mm", "in"]),
                "color": Schema.string("Solder mask color", options: BoardColor.allCases.map(\.rawValue)),
            ],
            edits: true,
            run: { context, args in Out.text(try setBoard(context!, args)) }),
        AutomationTool(
            name: "set_layer",
            summary: "Configure a copper layer. An inner layer (inner2…inner5) is a plane or wire layer. A plane "
                + "is a pour of copper on one net: set net to ground, a voltage like 5V or 3.3V, \"pin:U1.3\", "
                + "or none. Top and bottom are component layers.",
            properties: [
                "layer": Schema.string("top, bottom, inner2…inner5"),
                "type": Schema.string("component, plane or wire", options: LayerType.allCases.map(\.rawValue)),
                "net": Schema.string("ground, 5V, 3.3V, pin:U1.3 or none (planes)"),
                "clearance": Schema.number("Plane clearance to other nets, mm"),
            ],
            required: ["layer"], edits: true,
            run: { context, args in Out.text(try setLayer(context!, args)) }),
        AutomationTool(
            name: "move_footprint",
            summary: "PCB: place a part's footprint. x,y = its centre (mm; snaps to the grid), rotation = 0, 90, 180 "
                + "or 270 clockwise, side = top or bottom. Any of them may be omitted. Reports where it landed "
                + "and whether it is inside the board.",
            properties: [
                "reference": Schema.string("e.g. U1"), "x": Schema.number("Centre x, mm"),
                "y": Schema.number("Centre y, mm"), "rotation": Schema.integer("0, 90, 180, 270"),
                "side": Schema.string("top or bottom", options: ["top", "bottom"]),
            ],
            required: ["reference"], edits: true,
            run: { context, args in Out.text(try moveFootprint(context!, args)) }),
        AutomationTool(
            name: "route",
            summary: "PCB: lay a copper trace from one pad to another (\"U1.3\" → \"R1.1\") with 45°/90° bends. "
                + "waypoints are optional [x,y] points (mm) to pass through. Through-hole pads reach every layer; "
                + "a surface-mount pad only its own side. Check with run_drc.",
            properties: [
                "from": Schema.string("Start pad, e.g. U1.3"), "to": Schema.string("End pad, e.g. R1.1"),
                "layer": Schema.string("top (default), bottom, inner2…"),
                "width": Schema.integer("Trace width in mil (5–100, default 10)"),
                "waypoints": Schema.points("Points to pass through, mm"),
                "angle": Schema.integer("Bend angle: 45 (default) or 90 for plain L bends"),
            ],
            required: ["from", "to"], edits: true,
            run: { context, args in Out.text(try route(context!, args)) }),
        AutomationTool(
            name: "add_trace",
            summary: "PCB: add a trace through raw points [[x,y],…] (mm). Prefer route, which ends exactly on pads.",
            properties: [
                "points": Schema.points("Corner points, mm; each segment 0/45/90°"),
                "layer": Schema.string("top (default), bottom, inner2…"),
                "width": Schema.integer("mil"),
            ],
            required: ["points"], edits: true,
            run: { context, args in Out.text(try addTrace(context!, args)) }),
        AutomationTool(
            name: "set_trace",
            summary: "Change a trace's width (mil) or move it to another layer. id from get_board.",
            properties: [
                "id": Schema.string("Trace id"), "width": Schema.integer("mil"), "layer": Schema.string("Layer"),
            ],
            required: ["id"], edits: true,
            run: { context, args in
                let context = context!
                let trace = try context.find(context.document.traces, try args.string("id"), kind: "trace")
                if let width = args.optInt("width") { context.document.setTraceWidth(trace.id, width) }
                if let layer = args.optString("layer") {
                    guard let parsed = CopperLayer(rawValue: layer), context.document.board.traceLayers.contains(parsed) else {
                        throw AutomationFailure("layer must be one of " + context.document.board.traceLayers.map(\.rawValue).joined(separator: ", "))
                    }
                    context.document.setTraceLayer(trace.id, parsed)
                }
                return Out.text("trace \(context.short(trace.id)) updated")
            }),
        AutomationTool(
            name: "add_via",
            summary: "PCB: add a via (a plated hole joining all layers) at x,y mm. Put it on the end of a trace "
                + "to change layers.",
            properties: [
                "x": Schema.number("mm"), "y": Schema.number("mm"),
                "drill": Schema.number("Hole diameter, mm"), "diameter": Schema.number("Pad diameter, mm"),
            ],
            required: ["x", "y"], edits: true,
            run: { context, args in Out.text(try addVia(context!, args)) }),
        AutomationTool(
            name: "add_hole",
            summary: "PCB: add a mounting hole at x,y mm. size is a screw size (m2, m2.5, m3, m4, m5, m6, "
                + "#2…#10) or custom with drill and diameter. Plated holes get a copper ring.",
            properties: [
                "x": Schema.number("mm"), "y": Schema.number("mm"),
                "size": Schema.string("Screw size, default m3"), "drill": Schema.number("mm (custom)"),
                "diameter": Schema.number("Ring diameter, mm (custom)"), "plated": Schema.boolean("Plated?"),
            ],
            required: ["x", "y"], edits: true,
            run: { context, args in Out.text(try addHole(context!, args)) }),
        AutomationTool(
            name: "add_pour",
            summary: "PCB: flood a rectangle with copper on one net (a copper pour). x, y, width, height in mm, or "
                + "whole_board. It keeps its clearance from other nets' copper and from keep-outs, and joins the pads "
                + "of its net (thermal reliefs by default). layer is a trace layer (top by default, not a plane).",
            properties: pourProperties(needsPlace: true),
            edits: true,
            run: { context, args in Out.text(try addPour(context!, args)) }),
        AutomationTool(
            name: "set_pour",
            summary: "PCB: change a pour: id (from get_board, or pour:ID), then any of layer, net, x, y, width, height, "
                + "whole_board, clearance, connection, thermal_gap, spoke_width, min_width, locked.",
            properties: pourProperties(needsPlace: false).merging(["id": Schema.string("pour:ID or the id")]) { a, _ in a },
            required: ["id"], edits: true,
            run: { context, args in Out.text(try setPour(context!, args)) }),
        AutomationTool(
            name: "add_keepout",
            summary: "PCB: add a keep-out that pours stay out of (it blocks copper pours only). x, y, width, height "
                + "in mm, or whole_board. layers is a list of layer names, or all (the default).",
            properties: [
                "x": Schema.number("mm"), "y": Schema.number("mm"), "width": Schema.number("mm"),
                "height": Schema.number("mm"), "whole_board": Schema.boolean("Cover the board"),
                "layers": Schema.strings("Layer names, or [\"all\"]"),
            ],
            edits: true,
            run: { context, args in Out.text(try addKeepout(context!, args)) }),
        AutomationTool(
            name: "set_keepout",
            summary: "PCB: change a keep-out: id (from get_board, or keepout:ID), then any of x, y, width, height, "
                + "whole_board, layers, locked.",
            properties: [
                "id": Schema.string("keepout:ID or the id"), "x": Schema.number("mm"), "y": Schema.number("mm"),
                "width": Schema.number("mm"), "height": Schema.number("mm"), "whole_board": Schema.boolean("Cover the board"),
                "layers": Schema.strings("Layer names, or [\"all\"]"), "locked": Schema.boolean("Lock position"),
            ],
            required: ["id"], edits: true,
            run: { context, args in Out.text(try setKeepout(context!, args)) }),
        AutomationTool(
            name: "add_board_text",
            summary: "PCB: add text at x,y mm (its top-left), on the silkscreen (default) or document layer.",
            properties: [
                "text": Schema.string("The words"), "x": Schema.number("mm"), "y": Schema.number("mm"),
                "size": Schema.integer("Font size in mil (10–1000, default 50)"),
                "layer": Schema.string("silkscreen or document", options: TextLayer.allCases.map(\.rawValue)),
                "side": Schema.string("top or bottom", options: ["top", "bottom"]),
                "rotation": Schema.integer("0, 90, 180, 270"),
            ],
            required: ["text", "x", "y"], edits: true,
            run: { context, args in Out.text(try addBoardText(context!, args)) }),
        AutomationTool(
            name: "lock",
            summary: "Lock or unlock the position of PCB items so dragging and moves leave them: \"fp:U1\", "
                + "\"trace:ID\", \"via:ID\", \"hole:ID\", \"pour:ID\", \"keepout:ID\", \"text:ID\".",
            properties: ["items": Schema.strings("Items"), "locked": Schema.boolean("true (default) or false")],
            required: ["items"], edits: true,
            run: { context, args in
                let context = context!
                let locked = args.optBool("locked") ?? true
                var count = 0
                for spec in try args.strings("items") {
                    guard case let .board(item) = try item(spec, context, forDelete: false) else {
                        throw AutomationFailure("\(spec) is not a PCB item (use fp:\(spec) for a footprint)")
                    }
                    context.document.setLocked(item, locked)
                    count += 1
                }
                return Out.text("\(locked ? "locked" : "unlocked") \(count) item\(count == 1 ? "" : "s")")
            }),
    ]

    // MARK: Board and layers

    private static func setBoard(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var done: [String] = []
        var problem: String?
        document.recordEdit(.pcb, "Change Board") {
            if let id = args.optString("type") {
                if let type = BoardType.allCases.first(where: { $0.rawValue == id }) {
                    document.setBoardType(type)
                    done.append("type")
                } else {
                    problem = "unknown board type \(id); types: " + BoardType.allCases.map(\.rawValue).joined(separator: ", ")
                }
            }
            if let layers = args.optInt("layers") {
                if let refusal = document.canSetLayerCount(layers) { problem = refusal }
                else { document.setLayerCount(layers); done.append("layers") }
            }
            let width = args.optDouble("width"), height = args.optDouble("height")
            if width != nil || height != nil {
                document.setBoardSize(width: width ?? document.board.width, height: height ?? document.board.height)
                done.append("size")
            }
            if let grid = args.optDouble("grid") { document.setBoardGrid(grid); done.append("grid") }
            if let units = args.optString("units") {
                if let parsed = BoardUnits(rawValue: units) { document.setBoardUnits(parsed); done.append("units") }
                else { problem = "units must be mm or in" }
            }
            if let color = args.optString("color") {
                if let parsed = BoardColor(rawValue: color) { document.setBoardColor(parsed); done.append("color") }
                else { problem = "color must be one of " + BoardColor.allCases.map(\.rawValue).joined(separator: ", ") }
            }
        }
        if let problem { throw AutomationFailure(problem) }
        guard !done.isEmpty else { throw AutomationFailure("nothing to change: pass width, height, type, layers, grid, units or color") }
        let board = document.board
        return "board is now \(num(board.width))x\(num(board.height)) mm, \(board.layers) layers (changed \(done.joined(separator: ", ")))"
    }

    private static func layer(_ name: String, _ document: SchematicDocument) throws -> CopperLayer {
        guard let layer = CopperLayer(rawValue: name), document.board.copperLayers.contains(layer) else {
            throw AutomationFailure("no layer \(name); this board has " + document.board.copperLayers.map(\.rawValue).joined(separator: ", "))
        }
        return layer
    }

    private static func planeNet(_ spec: String, _ context: AutomationContext) throws -> PlaneNet? {
        let document = context.document
        let lower = spec.lowercased()
        if lower == "none" { return nil }
        if lower == "ground" || lower == "gnd" { return .ground }
        if lower.hasPrefix("pin:") {
            let found = try context.pin(String(spec.dropFirst(4)))
            return .pin(chip: found.chip.id, designator: found.pin.designator)
        }
        if lower.hasSuffix("v"), let volts = Double(lower.dropLast()) {
            let net = PlaneNet.rail(mv: UInt32((volts * 1000).rounded()))
            guard document.planeNetChoices().contains(where: { $0.net == net }) else {
                let rails = document.planeNetChoices().map(\.title)
                throw AutomationFailure("the schematic has no \(spec) rail (add a supply symbol); nets: \(rails.joined(separator: "; "))")
            }
            return net
        }
        throw AutomationFailure("net must be ground, a voltage like 5V, pin:U1.3, or none")
    }

    private static func setLayer(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let layer = try self.layer(try args.string("layer"), document)
        let net = try args.optString("net").map { try planeNet($0, context) }
        var problem: String?
        document.recordEdit(.pcb, "Change Layer") {
            if let name = args.optString("type") {
                guard let type = LayerType(rawValue: name) else { problem = "type must be component, plane or wire"; return }
                if let refusal = document.canSetLayerType(layer, type) { problem = refusal; return }
                document.setLayerType(layer, type)
            }
            if let net { document.setLayerNet(layer, net) }
            if let clearance = args.optDouble("clearance") { document.setLayerClearance(layer, clearance) }
        }
        if let problem { throw AutomationFailure(problem) }
        let style = document.board.style(of: layer)
        return "\(layer.rawValue): \(style.type.rawValue)"
            + (style.type == .plane ? ", net " + (style.net.map { $0.fileValue } ?? "none") : "")
    }

    // MARK: Footprints

    private static func moveFootprint(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let chip = try context.chip(try args.string("reference"))
        guard let placement = chip.board, chip.footprint != nil else {
            throw AutomationFailure("\(chip.reference) has no footprint (set one with set_part footprint=…)")
        }
        if placement.locked { throw AutomationFailure("\(chip.reference) is locked; unlock it first") }
        var rotation: Int?
        if let degrees = args.optInt("rotation") {
            guard degrees % 90 == 0 else { throw AutomationFailure("rotation must be 0, 90, 180 or 270") }
            rotation = ((degrees % 360) + 360) % 360
        }
        let side = try args.optString("side").map { name -> BoardSide in
            guard let side = BoardSide(rawValue: name) else { throw AutomationFailure("side must be top or bottom") }
            return side
        }
        if side == .bottom, document.board.layers < 2 { throw AutomationFailure("a 1-layer board has no bottom side") }
        let x = args.optDouble("x"), y = args.optDouble("y")
        guard (x == nil) == (y == nil) else { throw AutomationFailure("give both x and y, or neither") }
        guard rotation != nil || side != nil || x != nil else { throw AutomationFailure("give x and y, rotation or side") }
        document.recordEdit(.pcb, "Place Footprint") {
            if let side { document.setFootprintSide(chip.id, side) }
            if let rotation {
                for _ in 0..<3 where document.chips.first(where: { $0.id == chip.id })?.board?.rotation != rotation {
                    document.rotateFootprint(chip.id)
                }
            }
            if let x, let y { document.moveFootprint(chip.id, to: CGPoint(x: x, y: y)) }
        }
        let now = try context.chip(chip.reference)
        guard let board = now.board else { throw AutomationFailure("lost the footprint") }
        var report = "\(now.reference) footprint centre=(\(num(board.centre.x)),\(num(board.centre.y))) "
            + "rot=\(board.rotation) \(board.side.rawValue)"
        if let bounds = now.boardBounds {
            let inside = bounds.minX >= 0 && bounds.minY >= 0
                && bounds.maxX <= document.board.width && bounds.maxY <= document.board.height
            report += "; bounds x \(num(bounds.minX))…\(num(bounds.maxX)) y \(num(bounds.minY))…\(num(bounds.maxY))"
                + (inside ? " (inside the board)" : " (OUTSIDE the board \(num(document.board.width))x\(num(document.board.height)))")
        }
        return report
    }

    // MARK: Copper

    /// The centre of the pad of "U1.3" an agent routes to on `layer`.
    private static func pad(_ spec: String, layer: CopperLayer?, _ context: AutomationContext) throws
        -> (centre: CGPoint, side: BoardSide, through: Bool) {
        let found = try context.pin(spec)
        guard let placement = found.chip.board, found.chip.footprint != nil else {
            throw AutomationFailure("\(found.chip.reference) has no footprint on the board")
        }
        let pads = found.chip.boardPads.filter { $0.designator == found.pin.designator }
        guard !pads.isEmpty else { throw AutomationFailure("\(spec) has no pad on its footprint") }
        let reachable = pads.filter {
            guard let layer else { return true }
            return $0.drill > 0 || (layer == .top && placement.side == .top) || (layer == .bottom && placement.side == .bottom)
        }
        guard let pad = reachable.first else {
            throw AutomationFailure("\(spec) is a surface-mount pad on the \(placement.side.rawValue) side, so a trace "
                                    + "on \(layer?.rawValue ?? "?") can't reach it; route on \(placement.side.rawValue) "
                                    + "or add a via")
        }
        return (CGPoint(x: pad.rect.midX, y: pad.rect.midY), placement.side, pad.drill > 0)
    }

    private static func route(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let fromSpec = try args.string("from"), toSpec = try args.string("to")
        let start = try pad(fromSpec, layer: nil, context)
        let layer: CopperLayer
        if let name = args.optString("layer") { layer = try self.layer(name, document) }
        else { layer = start.side == .bottom ? .bottom : .top }
        guard document.board.traceLayers.contains(layer) else {
            throw AutomationFailure("\(layer.rawValue) can't carry traces; trace layers: "
                                    + document.board.traceLayers.map(\.rawValue).joined(separator: ", "))
        }
        let a = try pad(fromSpec, layer: layer, context), b = try pad(toSpec, layer: layer, context)
        let waypoints = args.raw["waypoints"] == nil ? [] : try args.points("waypoints")
        let angle = args.optInt("angle") ?? TraceRoute.defaultAngle
        let corners = [a.centre] + waypoints + [b.centre]

        func build(legFirst: Bool) -> [CGPoint] {
            var points: [CGPoint] = [corners[0]]
            for index in 1..<corners.count {
                for point in TraceRoute.route(from: corners[index - 1], to: corners[index], angle: angle,
                                              legFirst: legFirst).dropFirst() {
                    points.append(point)
                }
            }
            return TraceRoute.simplified(points)
        }
        let points = [false, true].map(build).first(where: TraceRoute.isLegal)
        guard let points else { throw AutomationFailure("couldn't make a legal 45°/90° path; add waypoints") }
        var id: UUID?
        document.recordEdit(.pcb, "Add Trace") {
            id = document.addTrace(points: points, layer: layer)
            if let id, let width = args.optInt("width") { document.setTraceWidth(id, width) }
        }
        guard let id else { throw AutomationFailure("the trace was refused") }
        document.clearAllSelections()
        let length = zip(points, points.dropFirst()).reduce(0.0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
        // A trace over a pad it wasn't meant for joins that pad's net: a short.
        let wanted = Set([fromSpec, toSpec].compactMap { spec in
            (try? context.pin(spec)).map { "\($0.chip.reference).\($0.pin.designator)" }
        })
        let crossed = document.pads(touching: [id]).filter { !wanted.contains($0) }
        let warning = crossed.isEmpty ? ""
            : "\nWARNING: this trace also touches \(crossed.joined(separator: ", ")), joining their nets (a short). "
            + "Undo it and route again with waypoints around those pads, or on another layer."
        return "routed \(fromSpec) → \(toSpec) on \(layer.rawValue): trace \(context.short(id)), \(num(length)) mm, \(points.count - 1) segment\(points.count == 2 ? "" : "s")" + warning
    }

    private static func addTrace(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let points = try args.points("points")
        guard points.count >= 2 else { throw AutomationFailure("points needs at least two points") }
        let layer = try self.layer(args.optString("layer") ?? "top", document)
        var id: UUID?
        document.recordEdit(.pcb, "Add Trace") {
            id = document.addTrace(points: points, layer: layer)
            if let id, let width = args.optInt("width") { document.setTraceWidth(id, width) }
        }
        guard let id else { throw AutomationFailure("trace refused: a segment isn't 0°, 45° or 90°, or \(layer.rawValue) can't carry traces") }
        document.clearAllSelections()
        return "trace \(context.short(id)) added on \(layer.rawValue)"
    }

    private static func addVia(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        var id: UUID?
        document.recordEdit(.pcb, "Add Via") {
            id = document.addVia(at: document.viaAnchor(at: point, tolerance: 1))
            guard let id else { return }
            if let drill = args.optDouble("drill") { document.setViaDrill(id, drill) }
            if let diameter = args.optDouble("diameter") { document.setViaDiameter(id, diameter) }
        }
        guard let id, let via = document.vias.first(where: { $0.id == id }) else {
            throw AutomationFailure("a 1-layer board has no vias")
        }
        document.clearAllSelections()
        return "via \(context.short(id)) at (\(num(via.centre.x)),\(num(via.centre.y))) drill \(num(via.drill)) diameter \(num(via.diameter))"
    }

    private static func addHole(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let size = try args.optString("size").map { name -> HoleSize in
            guard let size = HoleSize(rawValue: name.lowercased()) else {
                throw AutomationFailure("size must be one of " + HoleSize.allCases.map(\.rawValue).joined(separator: ", "))
            }
            return size
        }
        var id: UUID?
        document.recordEdit(.pcb, "Add Hole") {
            let hole = document.addHole(at: point)
            id = hole
            if let size { document.setHoleSize(hole, size) }
            if let drill = args.optDouble("drill") { document.setHoleDrill(hole, drill) }
            if let diameter = args.optDouble("diameter") { document.setHoleDiameter(hole, diameter) }
            if let plated = args.optBool("plated") { document.setHolePlated(hole, plated) }
        }
        document.clearAllSelections()
        guard let id, let hole = document.holes.first(where: { $0.id == id }) else { throw AutomationFailure("no hole added") }
        return "hole \(context.short(id)) at (\(num(hole.centre.x)),\(num(hole.centre.y))) drill \(num(hole.drill)) plated=\(hole.plated)"
    }

    // MARK: Pours and keep-outs

    private static func pourProperties(needsPlace: Bool) -> [String: Any] {
        [
            "layer": Schema.string("top (default), bottom, inner2… (a trace layer, not a plane)"),
            "net": Schema.string("ground, 5V, 3.3V, pin:U1.3 or none"),
            "x": Schema.number("mm"), "y": Schema.number("mm"), "width": Schema.number("mm"), "height": Schema.number("mm"),
            "whole_board": Schema.boolean("Cover the board"),
            "clearance": Schema.number("Gap to other nets, mm (default the DRC minimum)"),
            "connection": Schema.string("thermal (default) or solid", options: PourConnection.allCases.map(\.rawValue)),
            "thermal_gap": Schema.number("Ring cut round each pad, mm"), "spoke_width": Schema.number("mm"),
            "min_width": Schema.number("Narrowest piece kept, mm"), "locked": Schema.boolean("Lock position"),
        ]
    }

    /// The rectangle the arguments name: whole_board, or x, y, width and height (each optional against `current`).
    private static func rect(_ args: AutomationArgs, _ document: SchematicDocument, current: CGRect?) throws -> CGRect? {
        if args.optBool("whole_board") == true {
            guard let bounds = document.boardOutlineBounds else { throw AutomationFailure("the board has no outline") }
            return bounds
        }
        let values = [args.optDouble("x"), args.optDouble("y"), args.optDouble("width"), args.optDouble("height")]
        if values.allSatisfy({ $0 == nil }) {
            if current == nil { throw AutomationFailure("give x, y, width and height (mm), or whole_board") }
            return nil
        }
        guard let base = current ?? ((values.allSatisfy { $0 != nil }) ? CGRect.zero : nil) else {
            throw AutomationFailure("give x, y, width and height (mm), or whole_board")
        }
        let rect = CGRect(x: args.optDouble("x") ?? base.minX, y: args.optDouble("y") ?? base.minY,
                          width: args.optDouble("width") ?? base.width, height: args.optDouble("height") ?? base.height)
        guard rect.width >= BoardPour.minimumSide, rect.height >= BoardPour.minimumSide else {
            throw AutomationFailure("width and height must be at least \(num(BoardPour.minimumSide)) mm")
        }
        return rect
    }

    private static func pourID(_ context: AutomationContext, _ args: AutomationArgs) throws -> UUID {
        var spec = try args.string("id")
        if spec.lowercased().hasPrefix("pour:") { spec = String(spec.dropFirst(5)) }
        return try context.find(context.document.pours, spec, kind: "pour").id
    }

    private static func describe(_ pour: BoardPour, _ context: AutomationContext) -> String {
        let copper = context.document.connectivity()
        return "pour \(context.short(pour.id)) on \(pour.layer.rawValue) net \(pour.net?.fileValue ?? "none") at "
            + "(\(num(pour.rect.minX)),\(num(pour.rect.minY))) \(num(pour.rect.width))x\(num(pour.rect.height)); "
            + PourSection.fillSummary(pour, copper: copper).lowercased()
    }

    private static func applyPour(_ id: UUID, _ context: AutomationContext, _ args: AutomationArgs, rect: CGRect?) throws {
        let document = context.document
        if let name = args.optString("layer") {
            let layer = try self.layer(name, document)
            guard document.board.traceLayers.contains(layer) else { throw AutomationFailure("\(name) is a plane; a pour goes on a trace layer") }
            document.setPourLayer(id, layer)
        }
        if let net = try args.optString("net").map({ try planeNet($0, context) }) { document.setPourNet(id, net) }
        if let rect { document.setPourRect(id, rect) }
        if let clearance = args.optDouble("clearance") { document.setPourClearance(id, clearance) }
        if let name = args.optString("connection") {
            guard let connection = PourConnection(rawValue: name) else { throw AutomationFailure("connection must be thermal or solid") }
            document.setPourConnection(id, connection)
        }
        func ranged(_ key: String, _ range: ClosedRange<Double>) throws -> Double? {
            guard let value = args.optDouble(key) else { return nil }
            guard range.contains(value) else { throw AutomationFailure("\(key) must be \(num(range.lowerBound)) to \(num(range.upperBound)) mm") }
            return value
        }
        if let gap = try ranged("thermal_gap", BoardPour.gapRange) { document.setPourThermalGap(id, gap) }
        if let width = try ranged("spoke_width", BoardPour.spokeRange) { document.setPourSpokeWidth(id, width) }
        if let width = try ranged("min_width", BoardPour.minWidthRange) { document.setPourMinWidth(id, width) }
        if let locked = args.optBool("locked") { document.setLocked(.pour(id), locked) }
    }

    private static func addPour(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let layer = try args.optString("layer").map { try self.layer($0, document) } ?? .top
        guard document.board.traceLayers.contains(layer) else { throw AutomationFailure("\(layer.rawValue) is a plane; a pour goes on a trace layer") }
        // Everything is checked before anything changes.
        let rect = try rect(args, document, current: nil)!
        if let name = args.optString("net") { _ = try planeNet(name, context) }
        var id: UUID?
        var failure: Error?
        document.recordEdit(.pcb, "Add Pour") {
            id = document.addPour(rect: rect, layer: layer)
            guard let id else { return }
            do { try applyPour(id, context, args, rect: nil) } catch { failure = error }
        }
        document.clearAllSelections()
        if let failure { throw failure }
        guard let id, let pour = document.pours.first(where: { $0.id == id }) else { throw AutomationFailure("no pour added") }
        return describe(pour, context)
    }

    private static func setPour(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let id = try pourID(context, args)
        guard let before = document.pours.first(where: { $0.id == id }) else { throw AutomationFailure("no such pour") }
        let rect = try rect(args, document, current: before.rect)
        var failure: Error?
        document.recordEdit(.pcb, "Change Pour") {
            do { try applyPour(id, context, args, rect: rect) } catch { failure = error }
        }
        if let failure { throw failure }
        guard let pour = document.pours.first(where: { $0.id == id }) else { throw AutomationFailure("no such pour") }
        return describe(pour, context)
    }

    private static func keepoutLayers(_ args: AutomationArgs, _ document: SchematicDocument) throws -> Set<CopperLayer>?? {
        guard args.raw["layers"] != nil else { return .none }
        let names = try args.strings("layers")
        if names.contains(where: { $0.lowercased() == "all" }) { return .some(nil) }
        var layers = Set<CopperLayer>()
        for name in names { layers.insert(try layer(name, document)) }
        return .some(layers)
    }

    private static func describe(_ keepout: BoardKeepout, _ context: AutomationContext) -> String {
        "keepout \(context.short(keepout.id)) at (\(num(keepout.rect.minX)),\(num(keepout.rect.minY))) "
            + "\(num(keepout.rect.width))x\(num(keepout.rect.height)) layers "
            + (keepout.layers.map { $0.map(\.rawValue).sorted().joined(separator: ",") } ?? "all")
    }

    private static func addKeepout(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let rect = try rect(args, document, current: nil)!
        let layers = try keepoutLayers(args, document) ?? nil
        let id = document.addKeepout(rect: rect, layers: layers)
        document.clearAllSelections()
        guard let keepout = document.keepouts.first(where: { $0.id == id }) else { throw AutomationFailure("no keep-out added") }
        return describe(keepout, context)
    }

    private static func setKeepout(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("id")
        if spec.lowercased().hasPrefix("keepout:") { spec = String(spec.dropFirst(8)) }
        let id = try context.find(document.keepouts, spec, kind: "keep-out").id
        guard let before = document.keepouts.first(where: { $0.id == id }) else { throw AutomationFailure("no such keep-out") }
        let rect = try rect(args, document, current: before.rect)
        let layers = try keepoutLayers(args, document)
        document.recordEdit(.pcb, "Change Keep-out") {
            if let rect { document.setKeepoutRect(id, rect) }
            if let layers { document.setKeepoutLayers(id, layers) }
            if let locked = args.optBool("locked") { document.setLocked(.keepout(id), locked) }
        }
        guard let keepout = document.keepouts.first(where: { $0.id == id }) else { throw AutomationFailure("no such keep-out") }
        return describe(keepout, context)
    }

    private static func addBoardText(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let text = try args.string("text")
        let layer = TextLayer(rawValue: try args.choice("layer", among: TextLayer.allCases.map(\.rawValue), default: "silkscreen")) ?? .silkscreen
        let side = BoardSide(rawValue: try args.choice("side", among: ["top", "bottom"], default: "top")) ?? .top
        var id: UUID?
        document.recordEdit(.pcb, "Add Text") {
            let made = document.addText(at: point)
            id = made
            document.setText(made, text)
            if let size = args.optInt("size") { document.setTextSize(made, size) }
            document.setTextLayer(made, layer)
            document.setTextSide(made, side)
            if let rotation = args.optInt("rotation") { document.setTextRotation(made, rotation) }
        }
        document.clearAllSelections()
        return "text \(id.map(context.short) ?? "") added"
    }
}
