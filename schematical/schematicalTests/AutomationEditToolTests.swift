import AppKit
import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-X: the branches of the schematic, board and read tools
/// (AutomationSchematic*.swift, AutomationBoard.swift, AutomationRead.swift)
/// the other automation tests leave: each refusal says what is wrong, and the
/// less used options do what they say.
@MainActor
final class AutomationEditToolTests: XCTestCase {
    private struct Result {
        let text: String
        let isError: Bool
    }

    private func newWindow() -> (id: String, doc: SchematicDocument) {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        return (open.model.id.uuidString, doc)
    }

    private func call(_ tool: String, _ window: String, _ args: [String: Any] = [:]) async -> Result {
        var args = args
        args["window"] = window
        let result = await AutomationTools.call(tool, args)
        let content = result["content"] as? [[String: Any]] ?? []
        return Result(text: content.compactMap { $0["text"] as? String }.joined(separator: "\n"),
                      isError: result["isError"] as? Bool ?? false)
    }

    /// Calls a tool that must succeed; returns its text.
    @discardableResult
    private func ok(_ tool: String, _ window: String, _ args: [String: Any] = [:],
                    file: StaticString = #filePath, line: UInt = #line) async -> String {
        let result = await call(tool, window, args)
        XCTAssertFalse(result.isError, "\(tool): \(result.text)", file: file, line: line)
        return result.text
    }

    /// Calls a tool that must fail; returns its message.
    private func refused(_ tool: String, _ window: String, _ args: [String: Any] = [:],
                         file: StaticString = #filePath, line: UInt = #line) async -> String {
        let result = await call(tool, window, args)
        XCTAssertTrue(result.isError, "\(tool) should be refused: \(result.text)", file: file, line: line)
        return result.text
    }

    private func short(_ id: UUID) -> String { String(id.uuidString.prefix(8)).lowercased() }

    // MARK: Parts

    func testFootprintChoices() async throws {
        let (id, doc) = newWindow()
        let led = try TestSupport.part("GENERIC-LED")
        var text = await refused("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100, "footprint": "nope"])
        XCTAssertTrue(text.hasPrefix("GENERIC-LED has no footprint nope"), text)
        if let choice = led.footprintChoices.last {
            XCTAssertTrue(text.contains("; choices: "), text)
            await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100, "footprint": choice.package])
            XCTAssertEqual(doc.chips.last?.footprint?.id, choice.footprint.id)
        }
        let gate = try TestSupport.part("SN74HC00N")
        let own = try XCTUnwrap(gate.footprint?.id)
        XCTAssertTrue(gate.footprintChoices.isEmpty)
        await ok("place_part", id, ["part": "SN74HC00N", "x": 400, "y": 300, "footprint": own])
        XCTAssertEqual(doc.chips.last?.footprint?.id, own)
        text = await refused("set_part", id, ["reference": "U1", "footprint": "nope"])
        XCTAssertEqual(text, "SN74HC00N has no footprint nope")
        text = await ok("set_part", id, ["reference": "U1", "footprint": own, "show_reference": false])
        XCTAssertEqual(text, "U1: changed footprint, show_reference")
        XCTAssertEqual(doc.chips.last?.showsReference, false)
    }

    func testSetPartValuesAndRefusals() async throws {
        let (id, doc) = newWindow()
        let capacitor = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_CAPACITOR }, "no capacitor")
        let header = try XCTUnwrap(TestSupport.allParts.first { $0.genders.count > 1 }, "no header with genders")
        await ok("place_part", id, ["part": "GENERIC-RESISTOR", "x": 100, "y": 100])
        await ok("place_part", id, ["part": capacitor.name, "x": 300, "y": 100])
        await ok("place_part", id, ["part": header.name, "x": 500, "y": 300])
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 700, "y": 100])
        let (r, c, j, d) = (doc.chips[0].reference, doc.chips[1].reference, doc.chips[2].reference,
                            doc.chips[3].reference)
        var text = await refused("set_part", id, ["reference": d, "color": "plaid"])
        XCTAssertTrue(text.hasPrefix("GENERIC-LED has no color plaid; colors: "), text)
        text = await refused("set_part", id, ["reference": r, "ohms": -1])
        XCTAssertEqual(text, "-1 ohms isn't a valid resistance")
        text = await ok("set_part", id, ["reference": r, "ohms": 470])
        XCTAssertEqual(text, "\(r): changed ohms")
        XCTAssertEqual(doc.chips[0].ohms, 470)
        text = await ok("set_part", id, ["reference": c, "farads": 1e-6])
        XCTAssertEqual(text, "\(c): changed farads")
        XCTAssertEqual(doc.chips[1].farads, 1e-6)
        let gender = header.genders.last!
        text = await ok("set_part", id, ["reference": j, "gender": gender])
        XCTAssertEqual(text, "\(j): changed gender")
        XCTAssertEqual(doc.chips[2].gender, gender)
        text = await refused("set_part", id, ["reference": r, "new_reference": "5R"])
        XCTAssertEqual(text, "a reference is letters then a number, like U5")
        text = await refused("set_part", id, ["reference": r, "new_reference": d])
        XCTAssertFalse(text.isEmpty)
        XCTAssertEqual(doc.chips[0].reference, r, "a taken reference is refused")
        text = await refused("set_part", id, ["reference": r])
        XCTAssertTrue(text.hasPrefix("nothing to change: pass new_reference"), text)
    }

    func testSocketRefusals() async throws {
        let (id, doc) = newWindow()
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        var text = await refused("set_part", id, ["reference": "D1", "socket_mode": "ic+socket"])
        XCTAssertEqual(text, "GENERIC-LED cannot go in a socket")
        let catalog = TestSupport.catalog
        let bare = try XCTUnwrap(TestSupport.allParts.first { part in
            guard part.kind == PART_GATE, let footprint = part.footprint, part.footprintChoices.isEmpty else { return false }
            return catalog.genericSocket(fitting: footprint) == nil
        }, "no gate chip without a generic socket")
        await ok("place_part", id, ["part": bare.name, "x": 400, "y": 300])
        let reference = try XCTUnwrap(doc.chips.last?.reference)
        text = await refused("set_part", id, ["reference": reference, "socket_mode": "ic+socket"])
        XCTAssertEqual(text, "no socket holds \(bare.footprint!.id)")
    }

    // MARK: Wiring

    func testWireEndRefusals() async throws {
        let (id, doc) = newWindow()
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        var text = await refused("connect", id, ["from": "D1.A", "to": "label:NOPE"])
        XCTAssertEqual(text, "no net label NOPE; add one with add_net_label")
        text = await refused("connect", id, ["from": "D1.A", "to": "bus:NOBUS:D0"])
        XCTAssertEqual(text, "no bus NOBUS; buses: ")
        await ok("add_bus", id, ["x1": 300, "y1": 300, "x2": 700, "y2": 300, "name": "DATA", "wires": 4])
        text = await refused("connect", id, ["from": "D1.A", "to": "bus:DATA:ZZ"])
        XCTAssertEqual(text, "bus DATA has no wire ZZ; wires: B0,B1,B2,B3")
        text = await refused("connect", id, ["from": "D1.A", "to": "@1"])
        XCTAssertEqual(text, "a point looks like @x,y")

        // A point on a wire makes a junction.
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 400])
        await ok("connect", id, ["from": "D1.K", "to": "D2.K"])
        let route = try XCTUnwrap(doc.wires.first.map { doc.path(for: $0) })
        let middle = CGPoint(x: (route[0].x + route[1].x) / 2, y: (route[0].y + route[1].y) / 2)
        await ok("connect", id, ["from": "D2.A", "to": "@\(Int(middle.x)),\(Int(middle.y))"])
        XCTAssertEqual(doc.junctions.count, 1)

        // A pin with no place on the symbol has no tip to route from.
        let hidden = try XCTUnwrap(TestSupport.allParts.first { part in part.pins.contains { $0.isHidden } })
        await ok("place_part", id, ["part": hidden.name, "x": 900, "y": 300])
        let chip = try XCTUnwrap(doc.chips.last)
        let pin = try XCTUnwrap(chip.part.pins.first { $0.isHidden })
        text = await refused("connect", id, ["from": "\(chip.reference).\(pin.designator)", "to": "D1.A",
                                             "via": [[500, 500]]])
        XCTAssertEqual(text, "can't find the ends of that wire")

        text = await ok("disconnect", id, ["pin": "D2.A"])
        XCTAssertEqual(text, "removed 1 wire")
    }

    func testLabelNoteAndWireRefusals() async throws {
        let (id, doc) = newWindow()
        await ok("add_net_label", id, ["name": "PWR", "x": 100, "y": 100])
        await ok("add_note", id, ["text": "hello", "x": 800, "y": 100])
        let note = try XCTUnwrap(doc.notes.first?.id)
        var text = await refused("set_net_label", id, ["label": "note:\(short(note))"])
        XCTAssertEqual(text, "note:\(short(note)) isn't a net label")
        text = await refused("set_net_label", id, ["label": "PWR"])
        XCTAssertEqual(text, "nothing to change: give name, style or flip")
        text = await refused("set_net_label", id, ["label": "PWR", "name": "   "])
        XCTAssertEqual(text, "    isn't a usable label name")
        XCTAssertEqual(doc.netLabels.first?.name, "PWR")
        text = await refused("set_note", id, ["note": "label:PWR"])
        XCTAssertEqual(text, "label:PWR isn't a note")
        text = await refused("set_wire_route", id, ["wire": "note:\(short(note))", "via": [[0, 0]]])
        XCTAssertEqual(text, "note:\(short(note)) isn't a wire with both ends placed")
        text = await refused("add_net_label", id, ["name": "   ", "x": 0, "y": 0])
        XCTAssertFalse(text.isEmpty)
        XCTAssertEqual(doc.netLabels.count, 1, "nothing added")
    }

    /// 12 V on a label, and a No Connect label of the same name on a
    /// 74HC00's supply: making it an input would join them, and is refused.
    func testALabelTypeThatWouldOverVoltIsRefused() async throws {
        let (id, doc) = newWindow()
        let supply = try TestSupport.part("12V")
        doc.placeChip(part: supply, symbol: symbol(for: supply), centeredAt: CGPoint(x: 100, y: 300))
        let source = try XCTUnwrap(doc.chips.last)
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 500, y: 300))
        let power = doc.addNetLabel(at: CGPoint(x: 100, y: 100), style: .input)
        XCTAssertTrue(doc.setNetLabelName(power, "PWR"))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(source, "VCC")), .label(power)))
        let quiet = doc.addNetLabel(at: CGPoint(x: 500, y: 100), style: .noConnect)
        XCTAssertTrue(doc.setNetLabelName(quiet, "PWR"))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "VCC")), .label(quiet)))
        let text = await refused("set_net_label", id, ["label": short(quiet), "style": "input"])
        XCTAssertTrue(text.contains("12 V"), text)
        XCTAssertEqual(doc.netLabels.first { $0.id == quiet }?.style, .noConnect)
    }

    func testMoveRefusals() async throws {
        let (id, _) = newWindow()
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        await ok("place_part", id, ["part": "SN74HC00N", "x": 500, "y": 300, "view": "gate"])
        var text = await refused("move", id, ["items": ["fp:D1"], "dx": 10, "dy": 0])
        XCTAssertEqual(text, "fp:D1 isn't on the schematic page (PCB items move with move_footprint, set_trace…)")
        text = await refused("move", id, ["items": ["D1"]])
        XCTAssertEqual(text, "give dx and dy, or x and y for one item")
        text = await refused("move_part", id, ["reference": "D1"])
        XCTAssertEqual(text, "give x and y (the new centre), or dx and dy")
        text = await refused("move_part", id, ["reference": "U1-A"])
        XCTAssertEqual(text, "give x and y (the new centre), or dx and dy")
        text = await ok("move_part", id, ["reference": "U1-A", "dx": 20, "dy": 0])
        XCTAssertTrue(text.hasPrefix("U1-A now centred at"), text)
    }

    // MARK: Buses

    func testBusRefusals() async throws {
        let (id, doc) = newWindow()
        await ok("add_bus", id, ["x1": 300, "y1": 300, "x2": 900, "y2": 300, "name": "DATA", "wires": 4])
        var text = await refused("add_bus", id, ["x1": 500, "y1": 300, "x2": 1200, "y2": 300])
        XCTAssertTrue(text.hasPrefix("couldn't draw a bus there"), text)
        XCTAssertEqual(doc.buses.count, 1)
        text = await refused("set_bus", id, ["bus": "NOPE", "name": "X"])
        XCTAssertEqual(text, "no bus NOPE; buses: DATA")
        text = await refused("set_bus", id, ["bus": "DATA", "rename": ["ZZ": "Q"]])
        XCTAssertEqual(text, "no wire ZZ on this bus (changes before this one stay applied; the bus now has wires B0,B1,B2,B3)")

        text = await refused("add_bus_port", id, ["bus": "NOPE", "x": 400, "y": 300])
        XCTAssertEqual(text, "no bus NOPE; buses: DATA")
        text = await refused("add_bus_port", id, ["bus": "DATA", "x": 400, "y": 300, "wires": ["B0", "ZZ"]])
        XCTAssertEqual(text, "bus DATA wires are: B0,B1,B2,B3")
        text = await refused("add_bus_port", id, ["bus": "DATA", "x": 400, "y": 900])
        XCTAssertEqual(text, "no port was added")
        text = await ok("add_bus_port", id, ["bus": "DATA", "x": 500, "y": 300])
        XCTAssertEqual(text, "added a port on DATA with 4 pins")

        text = await refused("set_bus_port", id, ["bus": "DATA", "port": 1, "wires": ["ZZ"]])
        XCTAssertEqual(text, "bus DATA has no wire ZZ; wires: B0,B1,B2,B3")
        text = await refused("set_bus_port", id, ["bus": "DATA", "port": 1, "side": "diagonal"])
        XCTAssertEqual(text, "side is top or bottom (on a horizontal bus line), left or right (on a vertical one)")
        text = await refused("set_bus_port", id, ["bus": "DATA", "port": 1])
        XCTAssertEqual(text, "nothing to change: give wires, add_wires, remove_wires, side or at")
        text = await refused("set_bus_port", id, ["bus": "DATA", "port": 1, "at": 5000])
        XCTAssertTrue(text.hasPrefix("the port can't go to 5000"), text)

        text = await ok("select", id, ["items": ["bus:data"]])
        XCTAssertFalse(text.isEmpty)
        XCTAssertEqual(doc.schematicSelection.items, [.bus(try XCTUnwrap(doc.buses.first?.id))])
        text = await refused("select", id, ["items": ["bus:NOPE"]])
        XCTAssertEqual(text, "no bus NOPE")
    }

    // MARK: Reading

    /// get_design (text and JSON) with a switch, a junction, a socket and a
    /// bus port; get_nets naming a labelled net.
    func testDesignListsEveryKindOfItem() async throws {
        let (id, doc) = newWindow()
        await ok("place_part", id, ["part": "TE-GDS0204", "x": 100, "y": 100])
        await ok("place_part", id, ["part": "SN74HC00N", "x": 500, "y": 400])
        await ok("set_part", id, ["reference": "U1", "socket_mode": "ic+socket"])
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 600])
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 900])
        await ok("connect", id, ["from": "D1.K", "to": "D2.K"])
        let route = try XCTUnwrap(doc.wires.first.map { doc.path(for: $0) })
        let middle = CGPoint(x: (route[0].x + route[1].x) / 2, y: (route[0].y + route[1].y) / 2)
        await ok("connect", id, ["from": "D2.A", "to": "@\(Int(middle.x)),\(Int(middle.y))"])
        await ok("add_bus", id, ["x1": 800, "y1": 300, "x2": 1200, "y2": 300, "name": "DATA", "wires": 2])
        await ok("add_bus_port", id, ["bus": "DATA", "x": 1000, "y": 300])
        await ok("add_net_label", id, ["name": "CLK", "x": 300, "y": 100])
        await ok("connect", id, ["from": "D1.A", "to": "label:CLK"])

        var text = await ok("get_design", id, ["geometry": true])
        XCTAssertTrue(text.contains("switches=off,off"), text)
        XCTAssertTrue(text.contains("JUNCTIONS"), text)
        XCTAssertTrue(text.contains("junction:\(short(doc.junctions[0].id))"), text)
        XCTAssertTrue(text.contains("; pin tips B0@("), text)
        XCTAssertTrue(text.contains("socket=GENERIC-SOCKET-DIP-14-300 (ic+socket)"), text)

        text = await ok("get_design", id, ["format": "json"])
        let json = try XCTUnwrap(try JSONSerialization.jsonObject(with: Data(text.utf8)) as? [String: Any])
        let parts = try XCTUnwrap(json["parts"] as? [[String: Any]])
        XCTAssertEqual(parts.first { $0["reference"] as? String == "U1" }?["socket"] as? String,
                       "GENERIC-SOCKET-DIP-14-300 (ic+socket)")
        XCTAssertEqual((json["junctions"] as? [[String: Any]])?.first?["id"] as? String, short(doc.junctions[0].id))
        let bus = try XCTUnwrap((json["buses"] as? [[String: Any]])?.first)
        XCTAssertEqual(bus["name"] as? String, "DATA")
        let port = try XCTUnwrap((bus["ports"] as? [[String: Any]])?.first)
        XCTAssertEqual((port["pins"] as? [[String: Any]])?.map { $0["wire"] as? String }, ["B0", "B1"])

        text = await ok("get_nets", id)
        XCTAssertTrue(text.contains("CLK"), text)
    }

    func testPartWithoutAFootprint() async {
        let (id, _) = newWindow()
        let text = await ok("get_part", id, ["part": "12V"])
        XCTAssertTrue(text.contains("footprint: none"), text)
    }

    func testRenderingAnEmptyDesign() async {
        let (id, _) = newWindow()
        let text = await refused("render", id)
        XCTAssertEqual(text, "nothing to draw yet (the design is empty)")
    }

    // MARK: The board

    func testBoardSettings() async throws {
        let (id, doc) = newWindow()
        let type = try XCTUnwrap(BoardType.allCases.last)
        var text = await ok("set_board", id, ["type": type.rawValue, "units": "in", "color": BoardColor.allCases.last!.rawValue])
        XCTAssertTrue(text.contains("(changed type, units, color)"), text)
        XCTAssertEqual(doc.board.units, .inch)
        text = await refused("set_board", id, ["type": "cardboard"])
        XCTAssertTrue(text.hasPrefix("unknown board type cardboard; types: "), text)
        text = await refused("set_board", id, ["units": "furlongs"])
        XCTAssertEqual(text, "units must be mm or in")
        text = await refused("set_board", id, ["color": "plaid"])
        XCTAssertTrue(text.hasPrefix("color must be one of "), text)
        text = await refused("set_board", id, ["outline": [[1]]])
        XCTAssertEqual(text, "outline must be [[x, y], …]")
    }

    func testPlaneNetsAndLayers() async throws {
        let (id, _) = newWindow()
        await ok("place_part", id, ["part": "5V", "x": 100, "y": 100])
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 400, "y": 100])
        await ok("set_board", id, ["layers": 4])
        var text = await ok("set_layer", id, ["layer": "inner2", "type": "plane", "net": "5V"])
        XCTAssertTrue(text.hasPrefix("inner2: plane, net "), text)
        text = await ok("set_layer", id, ["layer": "inner3", "type": "plane", "net": "pin:D1.A", "clearance": 0.5])
        XCTAssertTrue(text.hasPrefix("inner3: plane, net "), text)
        text = await refused("set_layer", id, ["layer": "inner2", "net": "banana"])
        XCTAssertEqual(text, "net must be ground, a voltage like 5V, pin:U1.3, or none")
        text = await ok("get_board", id)
        XCTAssertTrue(text.contains("inner3: plane net="), text)
        XCTAssertTrue(text.contains("clearance=0.5mm"), text)
        text = await refused("route", id, ["from": "D1.A", "to": "D1.K", "layer": "inner2"])
        XCTAssertTrue(text.hasPrefix("inner2 can't carry traces; trace layers: "), text)
    }

    func testFootprintMovesAndRoutingRefusals() async throws {
        let (id, doc) = newWindow()
        await ok("place_part", id, ["part": "5V", "x": 100, "y": 100])
        await ok("place_part", id, ["part": "GENERIC-LED", "x": 400, "y": 100])
        var text = await refused("move_footprint", id, ["reference": "PWR1", "x": 10, "y": 10])
        XCTAssertEqual(text, "PWR1 has no footprint (set one with set_part footprint=…)")
        text = await refused("move_footprint", id, ["reference": "D1", "side": "middle"])
        XCTAssertEqual(text, "side must be top or bottom")
        text = await ok("move_footprint", id, ["reference": "D1", "side": "bottom"])
        XCTAssertTrue(text.contains(" bottom"), text)
        XCTAssertEqual(doc.chips.last?.board?.side, .bottom)
        text = await refused("route", id, ["from": "PWR1.VCC", "to": "D1.A"])
        XCTAssertEqual(text, "PWR1 has no footprint on the board")
        text = await refused("lock", id, ["items": ["D1"]])
        XCTAssertEqual(text, "D1 is not a PCB item (use fp:D1 for a footprint)")

        // A surface-mount pad on the bottom can't be reached from the top.
        var smd: PlacedChip?
        let surface = TestSupport.allParts.filter { part in
            guard let pads = part.footprint?.pads, !pads.isEmpty, part.footprintChoices.isEmpty else { return false }
            return pads.allSatisfy { $0.drill == 0 }
        }
        for part in surface.prefix(5) {
            await ok("place_part", id, ["part": part.name, "x": 700, "y": 400])
            if let chip = doc.chips.last, !chip.boardPads.isEmpty, chip.boardPads.allSatisfy({ $0.drill == 0 }) {
                smd = chip
                break
            }
            _ = doc.undo()
        }
        let chip = try XCTUnwrap(smd, "no surface-mount LED")
        let pad = try XCTUnwrap(chip.boardPads.compactMap(\.designator).first)
        text = await refused("route", id, ["from": "\(chip.reference).\(pad)", "to": "D1.A", "layer": "bottom"])
        XCTAssertTrue(text.hasPrefix("\(chip.reference).\(pad) is a surface-mount pad on the top side"), text)
    }

    func testTracesViasHolesTextsAndPours() async throws {
        let (id, doc) = newWindow()
        var text = await refused("add_trace", id, ["points": [[1, 1]]])
        XCTAssertEqual(text, "points needs at least two points")
        await ok("set_board", id, ["layers": 4])
        await ok("set_layer", id, ["layer": "inner2", "type": "plane"])
        text = await refused("add_trace", id, ["points": [[2, 2], [10, 2]], "layer": "inner2"])
        XCTAssertTrue(text.hasPrefix("trace refused: a segment isn't 0°, 45° or 90°, or inner2 can't carry traces"), text)
        text = await ok("add_trace", id, ["points": [[2, 2], [10, 2]], "width": 20])
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertEqual(text, "trace \(short(trace.id)) added on top")
        XCTAssertEqual(trace.width, 20)
        text = await ok("set_trace", id, ["id": short(trace.id), "width": 30, "layer": "bottom"])
        XCTAssertEqual(text, "trace \(short(trace.id)) updated")
        XCTAssertEqual(doc.traces.first?.width, 30)
        XCTAssertEqual(doc.traces.first?.layer, .bottom)
        text = await refused("set_trace", id, ["id": short(trace.id), "layer": "inner9"])
        XCTAssertTrue(text.hasPrefix("layer must be one of "), text)

        await ok("add_via", id, ["x": 10, "y": 2])
        text = await refused("add_hole", id, ["x": 20, "y": 20, "size": "huge"])
        XCTAssertTrue(text.hasPrefix("size must be one of "), text)
        await ok("add_hole", id, ["x": 20, "y": 20])
        await ok("add_board_text", id, ["text": "REV A", "x": 15, "y": 15])
        text = await refused("add_pour", id, ["x": 1, "y": 1, "width": 0.01, "height": 0.01])
        XCTAssertTrue(text.hasPrefix("width and height must be at least"), text)

        text = await ok("get_board", id)
        XCTAssertTrue(text.contains("  \(short(try XCTUnwrap(doc.vias.first).id)) ("), text)
        XCTAssertTrue(text.contains("  \(short(try XCTUnwrap(doc.holes.first).id)) ("), text)
        XCTAssertTrue(text.contains(": REV A"), text)

        let (single, _) = newWindow()
        await ok("set_board", single, ["layers": 1])
        text = await refused("add_via", single, ["x": 5, "y": 5])
        XCTAssertEqual(text, "a 1-layer board has no vias")
    }
}
