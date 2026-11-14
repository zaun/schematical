//
//  AutomationTools.swift
//  schematical
//
//  SPIKE (planning/spike-agent-control.md): the tools the automation server
//  offers an agent. Each one is a thin, name-addressed layer over the same
//  SchematicDocument methods the canvas calls, so an agent's edit is one
//  undo step like a person's, and the open window redraws by itself.
//
//  Agents address parts by reference ("U1") and pins by designator or name
//  ("U1.14", "D1.K"), never by UUID or coordinate.
//

import AppKit
import Foundation

@MainActor
enum AutomationTools {
    // MARK: Definitions

    private static func tool(_ name: String, _ description: String,
                             _ properties: [String: Any] = [:], required: [String] = []) -> [String: Any] {
        var props = properties
        props["window"] = ["type": "string",
                           "description": "Window id from list_windows. Omit when exactly one window is open."]
        return ["name": name, "description": description,
                "inputSchema": ["type": "object", "properties": props, "required": required]]
    }

    private static let string: [String: Any] = ["type": "string"]
    private static let number: [String: Any] = ["type": "number"]

    static let definitions: [[String: Any]] = [
        ["name": "list_windows",
         "description": "The open schematic windows: id, part count, wire count.",
         "inputSchema": ["type": "object", "properties": [String: Any]()]],
        tool("search_parts", "Find catalog parts by name or description. Returns name, package, description, pins.",
             ["query": string, "limit": number], required: ["query"]),
        tool("get_design", "The whole design as text: every part with reference, position and pins (with what each is wired to), every wire, net labels."),
        tool("place_part", "Place a catalog part on the schematic, centred at x,y (canvas units, y down, 10-unit grid). Returns its reference.",
             ["part": string, "x": number, "y": number], required: ["part", "x", "y"]),
        tool("connect", "Draw a wire between two pins, e.g. from \"R1.2\" to \"D1.A\". Pin = reference.designatorOrName.",
             ["from": string, "to": string], required: ["from", "to"]),
        tool("move_footprint", "PCB: move a part's footprint so its centre is at x,y (board mm, y down).",
             ["reference": string, "x": number, "y": number], required: ["reference", "x", "y"]),
        tool("add_trace", "PCB: add a copper trace through the points [[x,y],...] (board mm) on a layer (top, bottom, inner2..).",
             ["points": ["type": "array", "items": ["type": "array", "items": number]], "layer": string],
             required: ["points"]),
        tool("run_drc", "Run the design rule check on the PCB. Returns the issues, one per line."),
        tool("render", "A picture of the design: view = schematic | pcb | front | back. Use it to look at your work.",
             ["view": string]),
        tool("undo", "Undo the last edit on a page (schematic or pcb).", ["page": string]),
        tool("redo", "Redo on a page (schematic or pcb).", ["page": string]),
    ]

    // MARK: Dispatch

    static func call(_ name: String, _ args: [String: Any]) -> [String: Any] {
        do {
            if name == "list_windows" { return text(listWindows()) }
            let (document, _) = try window(args["window"] as? String)
            switch name {
            case "search_parts": return text(searchParts(args))
            case "get_design": return text(describe(document))
            case "place_part": return text(try placePart(document, args))
            case "connect": return text(try connect(document, args))
            case "move_footprint": return text(try moveFootprint(document, args))
            case "add_trace": return text(try addTrace(document, args))
            case "run_drc": return text(runDRC(document))
            case "render": return try render(document, args)
            case "undo", "redo": return text(try undoRedo(document, args, redo: name == "redo"))
            default: throw Failure("unknown tool \(name)")
            }
        } catch let failure as Failure {
            return ["isError": true, "content": [["type": "text", "text": failure.message]]]
        } catch {
            return ["isError": true, "content": [["type": "text", "text": "\(error)"]]]
        }
    }

    private struct Failure: Error {
        let message: String
        init(_ message: String) { self.message = message }
    }

    private static func text(_ string: String) -> [String: Any] {
        ["content": [["type": "text", "text": string]]]
    }

    // MARK: Windows

    private static func window(_ id: String?) throws -> (SchematicDocument, SimulationController) {
        let all = SchematicSessions.shared.identifiedSessions
        if let id {
            guard let hit = all.first(where: { $0.id.uuidString.hasPrefix(id.uppercased()) }) else {
                throw Failure("no window \(id); call list_windows")
            }
            return (hit.document, hit.simulation)
        }
        guard all.count == 1 else {
            throw Failure(all.isEmpty ? "no schematic window is open" : "\(all.count) windows are open; pass window (see list_windows)")
        }
        return (all[0].document, all[0].simulation)
    }

    private static func listWindows() -> String {
        let all = SchematicSessions.shared.identifiedSessions
        if all.isEmpty { return "no windows" }
        return all.map {
            "\($0.id.uuidString.prefix(8))  parts=\($0.document.chips.count) wires=\($0.document.wires.count)"
        }.joined(separator: "\n")
    }

    // MARK: Reading

    private static func searchParts(_ args: [String: Any]) -> String {
        let query = (args["query"] as? String ?? "").lowercased()
        let limit = (args["limit"] as? Int) ?? 15
        let store = PartCatalogStore.shared
        let hits = store.allParts.filter {
            $0.name.lowercased().contains(query) || $0.description.lowercased().contains(query)
                || $0.function.lowercased().contains(query)
        }
        var lines = hits.prefix(limit).map {
            "\($0.name) [\($0.package)] \($0.description) | pins: "
                + $0.pins.map { "\($0.designator)=\($0.name)" }.joined(separator: " ")
        }
        if hits.count > limit { lines.append("… \(hits.count - limit) more; narrow the query") }
        return lines.isEmpty ? "no match" : lines.joined(separator: "\n")
    }

    private static func pinName(_ end: WireEnd, in document: SchematicDocument) -> String {
        switch end {
        case let .pin(ref):
            guard let chip = document.chips.first(where: { $0.id == ref.chipID }),
                  let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return "?" }
            return "\(chip.reference).\(slot.pin.designator)"
        case let .junction(id): return "J\(id.uuidString.prefix(4))"
        case .busPin: return "bus"
        case .label: return "label"
        }
    }

    private static func describe(_ document: SchematicDocument) -> String {
        var out = ["PARTS"]
        for chip in document.chips {
            let wired = document.connectedPins[chip.id] ?? []
            let pins = chip.layout.slots.map { slot in
                "\(slot.pin.designator):\(slot.pin.name)\(wired.contains(slot.pin.index) ? "*" : "")"
            }.joined(separator: " ")
            let pcb = chip.board.map { String(format: " pcb=(%.2f,%.2f)mm rot=%d", $0.centre.x, $0.centre.y, $0.rotation) } ?? ""
            out.append("\(chip.reference) \(chip.part.name) at (\(Int(chip.origin.x)),\(Int(chip.origin.y)))\(pcb)  pins: \(pins)")
        }
        out.append("(* = wired)")
        out.append("WIRES")
        for wire in document.wires {
            out.append("\(pinName(wire.start, in: document)) -> \(pinName(wire.end, in: document))")
        }
        out.append("TRACES \(document.traces.count)  VIAS \(document.vias.count)")
        return out.joined(separator: "\n")
    }

    // MARK: Editing

    private static func resolvePin(_ spec: String, in document: SchematicDocument) throws -> PinRef {
        let parts = spec.split(separator: ".", maxSplits: 1).map(String.init)
        guard parts.count == 2 else { throw Failure("pin must look like U1.14 or D1.K, got \(spec)") }
        guard let chip = document.chips.first(where: { $0.reference == parts[0] }) else {
            throw Failure("no part \(parts[0])")
        }
        let slots = chip.layout.slots
        let slot = slots.first { $0.pin.designator == parts[1] }
            ?? slots.first { $0.pin.name.caseInsensitiveCompare(parts[1]) == .orderedSame }
        guard let slot else {
            throw Failure("\(parts[0]) has no pin \(parts[1]); pins: "
                          + slots.map { "\($0.pin.designator)=\($0.pin.name)" }.joined(separator: " "))
        }
        return PinRef(chipID: chip.id, index: slot.pin.index)
    }

    private static func double(_ args: [String: Any], _ key: String) throws -> Double {
        guard let value = (args[key] as? NSNumber)?.doubleValue else { throw Failure("\(key) must be a number") }
        return value
    }

    private static func placePart(_ document: SchematicDocument, _ args: [String: Any]) throws -> String {
        let name = args["part"] as? String ?? ""
        guard let part = PartCatalogStore.shared.part(named: name) else {
            throw Failure("no part \(name); use search_parts")
        }
        let before = document.chips.count
        document.placeChip(part: part, symbol: symbol(for: part),
                           centeredAt: CGPoint(x: try double(args, "x"), y: try double(args, "y")))
        guard document.chips.count > before, let placed = document.chips.last else {
            throw Failure("the part was not placed")
        }
        return "placed \(placed.reference) (\(part.name)) at (\(Int(placed.origin.x)),\(Int(placed.origin.y)))"
    }

    private static func connect(_ document: SchematicDocument, _ args: [String: Any]) throws -> String {
        let from = try resolvePin(args["from"] as? String ?? "", in: document)
        let to = try resolvePin(args["to"] as? String ?? "", in: document)
        if let refusal = document.connect(.pin(from), .pin(to)) { throw Failure(refusal) }
        return "connected \(args["from"] ?? "") to \(args["to"] ?? "")"
    }

    private static func moveFootprint(_ document: SchematicDocument, _ args: [String: Any]) throws -> String {
        let reference = args["reference"] as? String ?? ""
        guard let chip = document.chips.first(where: { $0.reference == reference }) else {
            throw Failure("no part \(reference)")
        }
        document.moveFootprint(chip.id, to: CGPoint(x: try double(args, "x"), y: try double(args, "y")))
        let now = document.chips.first { $0.id == chip.id }?.board?.centre
        return "\(reference) footprint now at \(now.map { "(\($0.x), \($0.y))" } ?? "none (no footprint)")"
    }

    private static func addTrace(_ document: SchematicDocument, _ args: [String: Any]) throws -> String {
        guard let raw = args["points"] as? [[Double]], raw.count >= 2, raw.allSatisfy({ $0.count == 2 }) else {
            throw Failure("points must be [[x,y],[x,y],...]")
        }
        let layer = CopperLayer(rawValue: args["layer"] as? String ?? "top") ?? .top
        guard document.addTrace(points: raw.map { CGPoint(x: $0[0], y: $0[1]) }, layer: layer) != nil else {
            throw Failure("trace refused: illegal angle, or layer \(layer.rawValue) is not a trace layer")
        }
        return "trace added on \(layer.rawValue); the document now has \(document.traces.count) traces"
    }

    private static func runDRC(_ document: SchematicDocument) -> String {
        let issues = document.designRuleCheck(rules: DesignRules.current())
        if issues.isEmpty { return "no issues" }
        return issues.map { "\($0.index). [\($0.kind)] \($0.reference): \($0.issue)" }.joined(separator: "\n")
    }

    private static func undoRedo(_ document: SchematicDocument, _ args: [String: Any], redo: Bool) throws -> String {
        let page = EditorPage(rawValue: args["page"] as? String ?? "schematic") ?? .schematic
        let name = redo ? document.redoName(on: page) : document.undoName(on: page)
        let done = redo ? document.redo(on: page) : document.undo(on: page)
        return done ? "\(redo ? "redid" : "undid") \(name ?? "an edit")" : "nothing to \(redo ? "redo" : "undo")"
    }

    // MARK: Seeing

    private static func render(_ document: SchematicDocument, _ args: [String: Any]) throws -> [String: Any] {
        guard let view = ImageExportView(rawValue: args["view"] as? String ?? "schematic") else {
            throw Failure("view must be schematic, pcb, front or back")
        }
        var options = ImageExportOptions()
        options.view = view
        options.format = .png
        options.scale = 1
        options.background = .white
        guard let data = ImageExport.data(for: document, options.normalized()) else {
            throw Failure("nothing to draw yet")
        }
        return ["content": [["type": "image", "data": data.base64EncodedString(), "mimeType": "image/png"]]]
    }
}
