import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The agent tools and a part drawn as its units (plan-67-G): `view` on
/// `set_part` and `place_part`, the unit address `U1-A`, one unit's move,
/// flip and turn, and what `get_design` says about the units.
@MainActor
final class SplitPartAutomationTests: XCTestCase {
    private struct Result {
        let text: String
        let isError: Bool
        let images: [String]
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
                      isError: result["isError"] as? Bool ?? false,
                      images: content.filter { $0["type"] as? String == "image" }.compactMap { $0["data"] as? String })
    }

    /// A window with a 7400 (U1), an LED (D1) and a resistor (R1).
    private func sample() async -> (id: String, doc: SchematicDocument) {
        let (id, doc) = newWindow()
        for (part, x) in [("SN74HC00N", 300.0), ("GENERIC-LED", 700.0), ("GENERIC-RESISTOR", 900.0)] {
            let placed = await call("place_part", id, ["part": part, "x": x, "y": 300.0])
            XCTAssertFalse(placed.isError, placed.text)
        }
        return (id, doc)
    }

    private func gate(_ doc: SchematicDocument) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.reference == "U1" })
    }

    func testSetPartView() async throws {
        let (id, doc) = await sample()
        let split = await call("set_part", id, ["reference": "U1", "view": "gate"])
        XCTAssertFalse(split.isError, split.text)
        XCTAssertEqual(split.text, "U1: changed view gate")
        XCTAssertTrue(try gate(doc).isSplit)
        XCTAssertEqual(doc.undoName, "Change Part")
        XCTAssertTrue(doc.undo())
        XCTAssertFalse(try gate(doc).isSplit)
        XCTAssertTrue(doc.redo())
        let block = await call("set_part", id, ["reference": "U1", "view": "Block"])
        XCTAssertEqual(block.text, "U1: changed view block")
        XCTAssertFalse(try gate(doc).isSplit)
        // "units" always names the unit view.
        let units = await call("set_part", id, ["reference": "U1", "view": "units"])
        XCTAssertFalse(units.isError, units.text)
        XCTAssertTrue(try gate(doc).isSplit)
        // A part with no units, and a view the part does not have.
        let resistor = await call("set_part", id, ["reference": "R1", "view": "gate"])
        XCTAssertTrue(resistor.isError)
        XCTAssertEqual(resistor.text, "R1 has no other view")
        let wrong = await call("set_part", id, ["reference": "U1", "view": "exploded"])
        XCTAssertTrue(wrong.isError)
        XCTAssertEqual(wrong.text, "view is block or gate, got “exploded”")
    }

    func testPlacePartWithView() async throws {
        let (id, doc) = newWindow()
        let placed = await call("place_part", id, ["part": "SN74HC00N", "x": 300.0, "y": 300.0, "view": "gate"])
        XCTAssertFalse(placed.isError, placed.text)
        XCTAssertTrue(placed.text.hasSuffix("; units: U1-A U1-B U1-C U1-D U1-P"), placed.text)
        XCTAssertTrue(try gate(doc).isSplit)
        // One undo step takes the placing and the view together.
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.chips.isEmpty)
        let plain = await call("place_part", id, ["part": "SN74HC00N", "x": 300.0, "y": 300.0, "view": "block"])
        XCTAssertFalse(plain.text.contains("units:"))
        XCTAssertEqual(doc.chips.first?.isSplit, false)
    }

    func testMoveOneUnit() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let before = try gate(doc)
        let moved = await call("move_part", id, ["reference": "U1-B", "x": 600.0, "y": 500.0])
        XCTAssertEqual(moved.text, "U1-B now centred at (600,500)")
        let after = try gate(doc)
        XCTAssertEqual(after.piece(forUnit: "B")?.bodyRect, CGRect(x: 570, y: 480, width: 60, height: 40))
        XCTAssertEqual(after.unitPlacements.filter { $0.key != "B" }, before.unitPlacements.filter { $0.key != "B" })
        let nudged = await call("move_part", id, ["reference": "u1-b", "dx": 20.0, "dy": -10.0])
        XCTAssertEqual(nudged.text, "U1-B now centred at (620,490)")
        // A unit the part does not have; a unit of a part in Block view.
        let missing = await call("move_part", id, ["reference": "U1-Z", "dx": 10.0, "dy": 0.0])
        XCTAssertEqual(missing.text, "U1 has no unit Z; units: A B C D P")
        let none = await call("move_part", id, ["reference": "R1-A", "dx": 10.0, "dy": 0.0])
        XCTAssertEqual(none.text, "R1 has no units: it is one symbol")
        _ = await call("set_part", id, ["reference": "U1", "view": "block"])
        let block = await call("move_part", id, ["reference": "U1-A", "dx": 10.0, "dy": 0.0])
        XCTAssertEqual(block.text, "U1 is in Block view; set_part reference=U1 view=gate first")
    }

    func testMoveWholeSplitPart() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let before = try gate(doc)
        let moved = await call("move_part", id, ["reference": "U1", "dx": 100.0, "dy": 40.0])
        XCTAssertFalse(moved.isError, moved.text)
        XCTAssertEqual(try gate(doc).unitPlacements.map(\.origin),
                       before.unitPlacements.map { CGPoint(x: $0.origin.x + 100, y: $0.origin.y + 40) })
    }

    func testFlipAndRotateAUnit() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let flipped = await call("flip_part", id, ["reference": "U1-C"])
        XCTAssertEqual(flipped.text, "flipped U1-C")
        XCTAssertEqual(try gate(doc).unitPlacements.map(\.isFlipped), [false, false, true, false, false])
        let turned = await call("rotate_part", id, ["reference": "U1-C"])
        XCTAssertEqual(turned.text, "rotated U1-C to 90°")
        let back = await call("rotate_part", id, ["reference": "U1-C", "counterclockwise": true])
        XCTAssertEqual(back.text, "rotated U1-C to 0°")
        // The block of power pins flips but does not turn.
        let block = await call("rotate_part", id, ["reference": "U1-P"])
        XCTAssertTrue(block.isError)
        XCTAssertEqual(block.text, "U1-P can't be rotated: it is a block")
        let blockFlip = await call("flip_part", id, ["reference": "U1-P"])
        XCTAssertFalse(blockFlip.isError, blockFlip.text)
        // The whole part: name a unit.
        let whole = await call("rotate_part", id, ["reference": "U1"])
        XCTAssertEqual(whole.text, "U1 is drawn as units; name one: U1-A U1-B U1-C U1-D U1-P")
        let wholeFlip = await call("flip_part", id, ["reference": "U1"])
        XCTAssertEqual(wholeFlip.text, "U1 is drawn as units; name one: U1-A U1-B U1-C U1-D U1-P")
        // In Block view the part flips as before and does not turn.
        _ = await call("set_part", id, ["reference": "U1", "view": "block"])
        let flippedBlock = await call("flip_part", id, ["reference": "U1"])
        XCTAssertEqual(flippedBlock.text, "flipped U1")
        let turnedBlock = await call("rotate_part", id, ["reference": "U1"])
        XCTAssertTrue(turnedBlock.isError)
        let turnedLED = await call("rotate_part", id, ["reference": "D1"])
        XCTAssertEqual(turnedLED.text, "rotated D1 to 90°")
    }

    func testPinAddresses() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let byUnit = await call("connect", id, ["from": "U1-A.3", "to": "D1.A"])
        XCTAssertFalse(byUnit.isError, byUnit.text)
        let plain = await call("connect", id, ["from": "U1.6", "to": "R1.1"])
        XCTAssertFalse(plain.isError, plain.text)
        let named = await call("connect", id, ["from": "U1-B.2A", "to": "U1-A.1Y"])
        XCTAssertFalse(named.isError, named.text)
        XCTAssertEqual(doc.wires.count, 3)
        let chip = try gate(doc)
        let ends = doc.wires.flatMap { [$0.start, $0.end] }.compactMap { end -> String? in
            guard case let .pin(ref) = end, ref.chipID == chip.id else { return nil }
            return chip.part.pins.first { $0.index == ref.index }?.name
        }
        XCTAssertEqual(ends.sorted(), ["1Y", "1Y", "2A", "2Y"])
        // A pin of another unit is refused, with the unit's pins.
        let wrong = await call("connect", id, ["from": "U1-A.6", "to": "R1.2"])
        XCTAssertTrue(wrong.isError)
        XCTAssertEqual(wrong.text, "U1-A has no pin 6; its pins: 1=1A 2=1B 3=1Y")
        // Nets and wire ends still read U1.3.
        let nets = await call("get_nets", id)
        XCTAssertTrue(nets.text.contains("U1.3"), nets.text)
        XCTAssertFalse(nets.text.contains("U1-A"), nets.text)
    }

    func testSelectPicksAUnit() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let selected = await call("select", id, ["items": ["U1-C"]])
        XCTAssertEqual(selected.text, "selected 1 item")
        XCTAssertEqual(doc.selectedChipID, try gate(doc).id)
        XCTAssertEqual(doc.picked(try gate(doc)), ["C"])
        let one = await call("get_selection", id)
        XCTAssertEqual(one.text, "schematic: U1 (U1-C)\npcb: fp:U1")
        _ = await call("select", id, ["items": ["U1-A", "U1-B", "D1"]])
        XCTAssertEqual(doc.picked(try gate(doc)), ["A", "B"])
        let two = await call("get_selection", id)
        XCTAssertTrue(two.text.hasPrefix("schematic: D1 U1 (U1-A U1-B)"), two.text)
        _ = await call("select", id, ["items": ["U1"]])
        XCTAssertEqual(doc.picked(try gate(doc)).count, 5)
        let all = await call("get_selection", id)
        XCTAssertTrue(all.text.hasPrefix("schematic: U1\n"), all.text)
    }

    func testDeleteAUnitIsRefused() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        let refused = await call("delete", id, ["items": ["U1-A"]])
        XCTAssertTrue(refused.isError)
        XCTAssertEqual(refused.text, "U1-A is one symbol of U1; delete U1 to remove the part")
        XCTAssertEqual(doc.chips.count, 3)
        // The whole part goes at once: the agent is not asked.
        let deleted = await call("delete", id, ["items": ["U1"]])
        XCTAssertEqual(deleted.text, "deleted 1 item")
        XCTAssertNil(doc.pendingRemoval)
        XCTAssertEqual(doc.chips.map(\.reference), ["D1", "R1"])
    }

    func testGetDesignListsUnits() async throws {
        let (id, doc) = await sample()
        _ = await call("set_part", id, ["reference": "U1", "view": "gate"])
        _ = await call("flip_part", id, ["reference": "U1-B"])
        _ = await call("rotate_part", id, ["reference": "U1-B"])
        let text = (await call("get_design", id)).text
        let lines = text.components(separatedBy: "\n")
        let part = try XCTUnwrap(lines.first { $0.hasPrefix("U1 SN74HC00N ") })
        XCTAssertTrue(part.contains(" view=gate"), part)
        XCTAssertFalse(part.contains("flipped"), part)
        let units = lines.filter { $0.hasPrefix("    U1-") }
        XCTAssertEqual(units.map { String($0.dropFirst(4).prefix(4)) }, ["U1-A", "U1-B", "U1-C", "U1-D", "U1-P"])
        XCTAssertTrue(units[0].contains(" size=60x40 "), units[0])
        XCTAssertTrue(units[0].hasSuffix(" pins=1,2,3"), units[0])
        XCTAssertTrue(units[1].contains(" flipped rotation=90 "), units[1])
        XCTAssertTrue(units[4].hasSuffix(" pins=14,7"), units[4])

        let json = (await call("get_design", id, ["format": "json"])).text
        let object = try XCTUnwrap(try JSONSerialization.jsonObject(with: Data(json.utf8)) as? [String: Any])
        let parts = try XCTUnwrap(object["parts"] as? [[String: Any]])
        let u1 = try XCTUnwrap(parts.first { $0["reference"] as? String == "U1" })
        XCTAssertEqual(u1["view"] as? String, "gate")
        let listed = try XCTUnwrap(u1["units"] as? [[String: Any]])
        XCTAssertEqual(listed.map { $0["unit"] as? String }, ["A", "B", "C", "D", "P"])
        XCTAssertEqual(listed.map { $0["reference"] as? String }, ["U1-A", "U1-B", "U1-C", "U1-D", "U1-P"])
        XCTAssertEqual(listed[1]["flipped"] as? Bool, true)
        XCTAssertEqual(listed[1]["rotation"] as? Int, 90)
        let pins = listed.flatMap { $0["pins"] as? [String] ?? [] }
        XCTAssertEqual(pins.count, 14)
        XCTAssertEqual(Set(pins), Set((1...14).map(String.init)))
        // Unit A's box in the JSON is the piece's.
        let a = try XCTUnwrap(try gate(doc).piece(forUnit: "A"))
        XCTAssertEqual(listed[0]["x"] as? Double, Double(a.bodyRect.midX))
        XCTAssertEqual(listed[0]["y"] as? Double, Double(a.bodyRect.midY))
        // Every pin's tip in the part's pin list is on its own unit.
        let tips = try XCTUnwrap(u1["pins"] as? [[String: Any]])
        XCTAssertEqual(tips.count, 14)
        XCTAssertTrue(tips.allSatisfy { $0["x"] != nil && $0["y"] != nil })
    }

    func testUnsplitOutputIsUnchanged() async throws {
        let (id, _) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 300.0, "y": 300.0])
        _ = await call("place_part", id, ["part": "GENERIC-RESISTOR", "x": 500.0, "y": 300.0])
        let text = (await call("get_design", id)).text
        XCTAssertFalse(text.contains("view="), text)
        XCTAssertFalse(text.contains("D1-"), text)
        let json = (await call("get_design", id, ["format": "json"])).text
        let object = try XCTUnwrap(try JSONSerialization.jsonObject(with: Data(json.utf8)) as? [String: Any])
        for part in try XCTUnwrap(object["parts"] as? [[String: Any]]) {
            XCTAssertNil(part["view"])
            XCTAssertNil(part["units"])
        }
    }

    func testBlockViewGateICSaysBlock() async throws {
        let (id, _) = await sample()
        let json = (await call("get_design", id, ["format": "json"])).text
        let object = try XCTUnwrap(try JSONSerialization.jsonObject(with: Data(json.utf8)) as? [String: Any])
        let parts = try XCTUnwrap(object["parts"] as? [[String: Any]])
        let u1 = try XCTUnwrap(parts.first { $0["reference"] as? String == "U1" })
        XCTAssertEqual(u1["view"] as? String, "block")
        XCTAssertNil(u1["units"])
        let text = await call("get_design", id)
        XCTAssertFalse(text.text.contains("view="), text.text)
    }

    /// The whole feature through the tools: place, Gate view, move and turn
    /// one gate, wire it, save, open, and the design reads the same. With
    /// `P67_OUT` set the rendered picture is printed as a `P67_PNG` line.
    func testEndToEnd() async throws {
        let (id, doc) = await sample()
        for (tool, args) in [
            ("set_part", ["reference": "U1", "view": "gate"]),
            ("move_part", ["reference": "U1-B", "x": 520.0, "y": 460.0]),
            ("rotate_part", ["reference": "U1-B"]),
            ("connect", ["from": "U1-B.6", "to": "D1.A"]),
            ("connect", ["from": "U1-A.3", "to": "U1-B.4"]),
            ("connect", ["from": "D1.K", "to": "R1.1"]),
        ] as [(String, [String: Any])] {
            let result = await call(tool, id, args)
            XCTAssertFalse(result.isError, "\(tool): \(result.text)")
        }
        let before = (await call("get_design", id, ["geometry": true])).text
        XCTAssertTrue(before.contains("U1-B "), before)

        // Save and open in a new window.
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let (otherID, other) = newWindow()
        try SchematicFile.decode(data).apply(to: other, catalog: TestSupport.catalog, fileURL: nil)
        let after = (await call("get_design", otherID, ["geometry": true])).text
        func body(_ text: String) -> String { text.components(separatedBy: "\n").dropFirst().joined(separator: "\n") }
        XCTAssertEqual(body(after), body(before))
        XCTAssertEqual(other.chips.first?.unitPlacements, doc.chips.first?.unitPlacements)

        let picture = await call("render", otherID)
        XCTAssertEqual(picture.images.count, 1)
        if ProcessInfo.processInfo.environment["P67_OUT"] != nil, let png = picture.images.first {
            print("P67_PNG \(png)")
        }
    }
}
