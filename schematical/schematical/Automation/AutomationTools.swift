//
//  AutomationTools.swift
//  schematical
//
//  The tools the automation server offers an agent (docs/agent-control.md).
//  Each is a thin, name-addressed layer over the same SchematicDocument
//  methods the canvas calls, so an agent's edit is one undo step like a
//  person's, and the open window redraws by itself.
//
//  Agents address parts by reference ("U1") and pins by designator or name
//  ("U1.14", "D1.K"), never by UUID. Wires, labels, notes, traces, vias,
//  holes and board texts have short ids (the first 8 hex digits) that
//  `get_design` and `get_board` list.
//
//  The tools are in topical files: AutomationRead, AutomationSchematic,
//  AutomationBoard, AutomationSimulation, AutomationSession.
//

import AppKit
import Foundation

// MARK: - Plumbing

/// What a tool works on: one schematic window.
struct AutomationContext {
    let id: UUID
    let document: SchematicDocument
    let simulation: SimulationController
    let file: SchematicExporter?
    /// The window's model, and through it its system document.
    var model: SchematicEditorModel?

    /// The window's id as agents see it.
    var shortID: String { String(id.uuidString.prefix(8)) }
}

/// A refusal an agent can read and act on.
nonisolated struct AutomationFailure: Error {
    let message: String
    init(_ message: String) { self.message = message }
}

/// A tool call's arguments, with typed, validating readers.
struct AutomationArgs {
    let raw: [String: Any]

    func string(_ key: String) throws -> String {
        guard let value = raw[key] as? String, !value.isEmpty else {
            throw AutomationFailure("\(key) is required (a string)")
        }
        return value
    }

    func optString(_ key: String) -> String? {
        guard let value = raw[key] as? String, !value.isEmpty else { return nil }
        return value
    }

    func double(_ key: String) throws -> Double {
        guard let number = raw[key] as? NSNumber, !(raw[key] is Bool) else {
            throw AutomationFailure("\(key) is required (a number)")
        }
        return number.doubleValue
    }

    func optDouble(_ key: String) -> Double? {
        guard let number = raw[key] as? NSNumber, !(raw[key] is Bool) else { return nil }
        return number.doubleValue
    }

    func optInt(_ key: String) -> Int? {
        optDouble(key).map { Int($0.rounded()) }
    }

    func optBool(_ key: String) -> Bool? {
        raw[key] as? Bool
    }

    func strings(_ key: String) throws -> [String] {
        guard let list = raw[key] as? [Any] else { throw AutomationFailure("\(key) is required (a list of strings)") }
        return try list.map {
            guard let text = $0 as? String else { throw AutomationFailure("\(key) must hold only strings") }
            return text
        }
    }

    func points(_ key: String) throws -> [CGPoint] {
        guard let list = raw[key] as? [Any] else { throw AutomationFailure("\(key) must be [[x, y], …]") }
        return try list.map {
            guard let pair = $0 as? [Any], pair.count == 2,
                  let x = (pair[0] as? NSNumber)?.doubleValue, let y = (pair[1] as? NSNumber)?.doubleValue else {
                throw AutomationFailure("\(key) must be [[x, y], …]")
            }
            return CGPoint(x: x, y: y)
        }
    }

    /// A choice from a fixed list; `fallback` when the argument is absent.
    func choice(_ key: String, among options: [String], default fallback: String) throws -> String {
        let value = optString(key) ?? fallback
        guard options.contains(value) else {
            throw AutomationFailure("\(key) must be one of \(options.joined(separator: ", ")), got \(value)")
        }
        return value
    }
}

/// One tool: its MCP definition and what it does.
struct AutomationTool {
    let name: String
    let summary: String
    var properties: [String: Any] = [:]
    var required: [String] = []
    /// Works on one window (adds the `window` argument).
    var needsWindow = true
    /// Changes nothing (an MCP hint).
    var readOnly = false
    /// Changes the design: refused while the simulation runs.
    var edits = false
    let run: (AutomationContext?, AutomationArgs) async throws -> [[String: Any]]

    var definition: [String: Any] {
        var props = properties
        if needsWindow {
            props["window"] = ["type": "string",
                               "description": "Window id from list_windows. Omit to use the frontmost window."]
        }
        return ["name": name, "description": summary,
                "inputSchema": ["type": "object", "properties": props, "required": required],
                "annotations": ["readOnlyHint": readOnly]]
    }
}

/// JSON-schema pieces for tool arguments.
enum Schema {
    static func string(_ detail: String, options: [String]? = nil) -> [String: Any] {
        var schema: [String: Any] = ["type": "string", "description": detail]
        if let options { schema["enum"] = options }
        return schema
    }

    static func number(_ detail: String) -> [String: Any] { ["type": "number", "description": detail] }
    static func integer(_ detail: String) -> [String: Any] { ["type": "integer", "description": detail] }
    static func boolean(_ detail: String) -> [String: Any] { ["type": "boolean", "description": detail] }

    static func strings(_ detail: String) -> [String: Any] {
        ["type": "array", "items": ["type": "string"], "description": detail]
    }

    static func points(_ detail: String) -> [String: Any] {
        ["type": "array", "items": ["type": "array", "items": ["type": "number"]], "description": detail]
    }
}

// MARK: - Output

enum Out {
    static func text(_ string: String) -> [[String: Any]] {
        [["type": "text", "text": string]]
    }

    static func image(_ data: Data, caption: String? = nil) -> [[String: Any]] {
        var content: [[String: Any]] = [["type": "image", "data": data.base64EncodedString(), "mimeType": "image/png"]]
        if let caption { content.append(["type": "text", "text": caption]) }
        return content
    }

    /// Pretty-free JSON for structured results.
    static func json(_ object: Any) -> [[String: Any]] {
        guard JSONSerialization.isValidJSONObject(object),
              let data = try? JSONSerialization.data(withJSONObject: object, options: [.sortedKeys]),
              let string = String(data: data, encoding: .utf8) else { return text("\(object)") }
        return text(string)
    }
}

/// A number for text output: up to 3 decimals, no trailing zeros.
func num(_ value: Double) -> String {
    var text = String(format: "%.3f", value)
    while text.hasSuffix("0") { text.removeLast() }
    if text.hasSuffix(".") { text.removeLast() }
    return text == "-0" ? "0" : text
}

func num(_ value: CGFloat) -> String { num(Double(value)) }

// MARK: - Registry and dispatch

enum AutomationTools {
    static let all: [AutomationTool] =
        sessionTools + readTools + schematicTools + boardTools + simulationTools

    static let byName: [String: AutomationTool] = Dictionary(uniqueKeysWithValues: all.map { ($0.name, $0) })

    static var definitions: [[String: Any]] { all.map(\.definition) }

    static func call(_ name: String, _ raw: [String: Any]) async -> [String: Any] {
        guard let tool = byName[name] else {
            return failure("unknown tool \(name)")
        }
        do {
            let context = tool.needsWindow ? try resolveWindow(raw["window"] as? String) : nil
            if tool.edits, let context, context.document.isPlaying {
                throw AutomationFailure("The simulation is running, so the design can't be edited. Call simulation_stop first.")
            }
            return ["content": try await tool.run(context, AutomationArgs(raw: raw))]
        } catch let error as AutomationFailure {
            return failure(error.message)
        } catch {
            return failure("\(error)")
        }
    }

    static func failure(_ message: String) -> [String: Any] {
        ["isError": true, "content": [["type": "text", "text": message]]]
    }

    // MARK: Windows

    /// The open documents, in the order they were opened.
    static func sessions() -> [AutomationContext] {
        NSDocumentController.shared.documents.compactMap { document in
            guard let model = (document as? SchematicNSDocument)?.model else { return nil }
            return AutomationContext(id: model.id, document: model.document, simulation: model.simulation,
                                     file: model.file, model: model)
        }
    }

    /// The window an id names, or the frontmost one when none is given.
    static func resolveWindow(_ id: String?) throws -> AutomationContext {
        let all = sessions()
        if all.isEmpty { throw AutomationFailure("no schematic window is open (use new_window)") }
        if let id {
            let matches = all.filter { $0.id.uuidString.lowercased().hasPrefix(id.lowercased()) }
            guard matches.count == 1 else {
                throw AutomationFailure(matches.isEmpty ? "no window \(id); call list_windows"
                                                        : "window id \(id) is ambiguous; call list_windows")
            }
            return matches[0]
        }
        if all.count == 1 { return all[0] }
        // The frontmost visible schematic window.
        for window in NSApp.orderedWindows where window.isVisible {
            if let hit = all.first(where: { $0.file?.window === window }) { return hit }
        }
        if let key = all.first(where: { $0.file?.window?.isKeyWindow == true }) { return key }
        throw AutomationFailure("\(all.count) windows are open; pass window (see list_windows)")
    }
}

// MARK: - Looking things up by name

extension AutomationContext {
    func chip(_ reference: String) throws -> PlacedChip {
        guard let chip = document.chips.first(where: { $0.reference.caseInsensitiveCompare(reference) == .orderedSame }) else {
            let known = document.chips.prefix(12).map(\.reference).joined(separator: ", ")
            throw AutomationFailure("no part \(reference)" + (known.isEmpty ? " (the design is empty)" : "; parts: \(known)"))
        }
        return chip
    }

    /// "U1", or "U1-A": one unit of a part drawn as its units (plan-67; a
    /// gate of a gate IC in Gate view, or "U1-P", the block of its power
    /// pins). A reference is letters then a number, so the first "-" starts
    /// the unit. `key` is nil for a whole part.
    func unit(_ spec: String) throws -> (chip: PlacedChip, key: String?) {
        guard let dash = spec.firstIndex(of: "-"), dash != spec.startIndex else { return (try chip(spec), nil) }
        let chip = try chip(String(spec[..<dash]))
        let wanted = String(spec[spec.index(after: dash)...])
        let keys = chip.part.units.map(\.key)
        guard let key = keys.first(where: { $0.caseInsensitiveCompare(wanted) == .orderedSame }) else {
            throw AutomationFailure(keys.isEmpty
                ? "\(chip.reference) has no units: it is one symbol"
                : "\(chip.reference) has no unit \(wanted); units: \(keys.joined(separator: " "))")
        }
        guard chip.isSplit else {
            let view = (chip.part.unitViewName ?? "units").lowercased()
            throw AutomationFailure("\(chip.reference) is in Block view; set_part reference=\(chip.reference) view=\(view) first")
        }
        return (chip, key)
    }

    /// The units of a part drawn as its units, as addresses: "U1-A U1-B …".
    func unitNames(_ chip: PlacedChip) -> String {
        chip.unitPlacements.map { "\(chip.reference)-\($0.key)" }.joined(separator: " ")
    }

    /// "U1.14" or "D1.K": a part's pin by designator, else by name. The part
    /// may name a unit ("U1-A.3"): the pin must then be one of that unit's.
    func pin(_ spec: String) throws -> (chip: PlacedChip, pin: ChipPin) {
        guard let dot = spec.firstIndex(of: "."), dot != spec.startIndex else {
            throw AutomationFailure("a pin looks like U1.14 or D1.K, got “\(spec)”")
        }
        let (chip, key) = try unit(String(spec[..<dot]))
        let name = String(spec[spec.index(after: dot)...])
        var pins = chip.part.pins
        if let key, let unit = chip.part.units.first(where: { $0.key == key }) {
            pins = pins.filter { unit.pins.contains($0.index) }
        }
        let hit = pins.first { $0.designator == name }
            ?? pins.first { $0.name.caseInsensitiveCompare(name) == .orderedSame }
        guard let hit else {
            let owner = key.map { "\(chip.reference)-\($0)" } ?? chip.reference
            throw AutomationFailure("\(owner) has no pin \(name); \(key == nil ? "pins" : "its pins"): "
                                    + pins.map { "\($0.designator)=\($0.name)" }.joined(separator: " "))
        }
        return (chip, hit)
    }

    func pinRef(_ spec: String) throws -> PinRef {
        let found = try pin(spec)
        return PinRef(chipID: found.chip.id, index: found.pin.index)
    }

    /// "R1.2" for a pin end of a wire.
    func pinName(_ ref: PinRef) -> String {
        guard let chip = document.chips.first(where: { $0.id == ref.chipID }),
              let pin = chip.part.pins.first(where: { $0.index == ref.index }) else { return "?" }
        return "\(chip.reference).\(pin.designator)"
    }

    func endName(_ end: WireEnd) -> String {
        switch end {
        case let .pin(ref): return pinName(ref)
        case let .junction(id): return "junction:\(short(id))"
        case let .busPin(ref):
            if let found = document.port(ref.portID), let wire = found.bus.wires.first(where: { $0.id == ref.wireID }) {
                return "bus:\(found.bus.name):\(wire.name)"
            }
            return "bus"
        case let .label(id):
            return "label:" + (document.netLabels.first { $0.id == id }?.name ?? short(id))
        }
    }

    /// The centre of a part's block on the schematic: the point `place_part`
    /// and `move_part` take.
    func centre(of chip: PlacedChip) -> CGPoint {
        let size = chip.layout.size
        return CGPoint(x: chip.origin.x + size.width / 2, y: chip.origin.y + size.height / 2)
    }

    func short(_ id: UUID) -> String { String(id.uuidString.prefix(8)).lowercased() }

    /// The one item whose id starts with `prefix`.
    func find<T: Identifiable>(_ items: [T], _ prefix: String, kind: String) throws -> T where T.ID == UUID {
        let lower = prefix.lowercased()
        let hits = items.filter { $0.id.uuidString.lowercased().hasPrefix(lower) }
        guard hits.count == 1 else {
            throw AutomationFailure(hits.isEmpty ? "no \(kind) \(prefix)" : "\(kind) id \(prefix) is ambiguous")
        }
        return hits[0]
    }
}

// MARK: - Guide

/// What an agent is told when it connects (MCP `instructions`).
enum AutomationGuide {
    static let text = """
    Schematical is a schematic and PCB design app. These tools drive the open windows live; \
    everything you do shows on screen and is undoable (undo/redo), one step per call (or per batch).

    Workflow: search_parts → place_part → connect (draw the schematic) → get_design / get_nets / render to \
    check → run_erc → run_drc → move_footprint / route (lay out the PCB) → run_drc → render (pcb, front). \
    Look at your work with render often; it returns a picture.

    Addressing: parts by reference ("U1", "R2"); pins by designator or name ("U1.14", "D1.K", "U1.VCC"). \
    A gate IC can be drawn as separate gates: set_part view=gate. \
    Its gates are then "U1-A", "U1-B", …, and "U1-P" is the block with its power pins; move, flip and rotate them \
    one at a time (move_part, flip_part, rotate_part). Pins are still "U1.3" ("U1-A.3" also works). Deleting takes \
    the whole part. \
    Everything else (wires, labels, notes, traces, vias, holes, pours, keep-outs, board texts) by the short id get_design / \
    get_board prints; the Document Info title block is "titleblock" (set_document_info edits the information and shows or hides it). Net labels connect by name: "label:CLK". A point on an existing wire is "@x,y".

    Power symbols are parts too, placed by name: GND, 5V, 3V3, 2V5, 1V8, 1V2, 12V, or a rail of any voltage up to 500 V named like 170V or 3V6 (pin "VCC"/"GND"), and \
    PULLDOWN / PULLUP-5V (a resistor to a rail). They aren't listed by search_parts. Their references look \
    like PWR1; use the reference place_part returns. Connect IC power pins to them like any other pin.

    Schematic coordinates: canvas units, y grows DOWN, a 10-unit grid (positions snap). Parts are placed and \
    moved by their centre. A typical part is 40–120 units wide (a 14-pin DIP is ~120x290); leave ~100 units \
    between parts. A part's size and bounds, every wire's route and every label's and note's bounds are in get_design; get_design with geometry=true adds each pin's tip. Wires take the shortest right-angle route and do NOT avoid \
    parts: a wire that crosses a part's pins looks (and reads) like a short even though the netlist is right. \
    Place parts so the L-shapes are clear, or pass connect `via` waypoints to go around. Check with render.

    PCB coordinates: millimetres, y grows down, origin at the board's top-left. Footprints snap to the board \
    grid (0.635 mm by default). Every part starts with a footprint auto-placed in a row; use move_footprint \
    to lay them out inside the board (get_board shows its size). Flood a rectangle with copper on one net \
    with add_pour (thermal reliefs by default; it keeps clear of other nets) and keep pours out of an area with \
    add_keepout. Cut a region out of the board with add_cutout (a rectangle, slot, circle or polygon: the \
    board has no material there and copper keeps its distance from its edge; change one with set_cutout), and \
    give the board a free-form \
    edge with set_board outline=[[x, y], …]. Route copper with route(from, to) between \
    pads ("U1.3" → "R1.1"); it uses 45°/90° legs. Through-hole pads reach every layer; surface-mount pads \
    only their side. The ratsnest (unrouted nets) shows in run_drc as "unrouted".

    Rules: the design can't be edited while the simulation runs (simulation_stop first). Edits that would \
    put a part's pin outside its voltage range are refused with the reason. Use batch to make many edits \
    as one undo step. Errors say what's wrong and what the valid choices are.
    """
}
