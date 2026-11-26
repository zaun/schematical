import Foundation
import XCTest
@testable import schematical

/// plan-68-Y: a schematic file the reader refuses, one broken field at a
/// time, and the message that says which. Each case edits the JSON of a small
/// saved schematic (two gates wired together on a 4-layer board with net
/// classes and a hole) and opens it.
@MainActor
final class SchematicFileReadTests: XCTestCase {
    private typealias JSON = [String: Any]

    /// The saved file's JSON.
    private func base() throws -> JSON {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        doc.addStandardNetClasses()
        let one = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let two = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 600, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "1Y")), .pin(try TestSupport.pin(two, "1A"))))
        _ = doc.addHole(at: CGPoint(x: 30, y: 30))
        let data = try SchematicFile(document: doc, fileURL: nil, catalog: TestSupport.catalog).encoded()
        return try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? JSON)
    }

    /// The message opening `json` gives; nil when it opens.
    private func open(_ json: JSON) throws -> String? {
        let data = try JSONSerialization.data(withJSONObject: json)
        do {
            try SchematicFile.decode(data).apply(to: TestSupport.newDocument(), catalog: TestSupport.catalog, fileURL: nil)
            return nil
        } catch {
            return error.localizedDescription
        }
    }

    /// The message after `edit`; fails when the file opens.
    private func refused(file: StaticString = #filePath, line: UInt = #line, _ edit: (inout JSON) throws -> Void) throws -> String {
        var json = try base()
        try edit(&json)
        return try XCTUnwrap(try open(json), "the file opened", file: file, line: line)
    }

    private func board(_ json: inout JSON, _ edit: (inout JSON) -> Void) {
        var board = json["board"] as? JSON ?? [:]
        edit(&board)
        json["board"] = board
    }

    private func first(_ json: inout JSON, _ key: String, _ edit: (inout JSON) -> Void) {
        var list = json[key] as? [JSON] ?? []
        edit(&list[0])
        json[key] = list
    }

    private func assert(_ message: String, contains text: String, file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertTrue(message.contains(text), message, file: file, line: line)
    }

    func testTheBaseOpens() throws {
        XCTAssertNil(try open(try base()))
    }

    // MARK: The file

    func testAFileThatIsNotASchematicOrCannotBeRead() throws {
        assert(try refused { $0["format"] = "other" }, contains: "not a Schematical schematic")
        assert(try refused { $0["parts"] = "none" }, contains: "could not be read")
    }

    func testAPinNamedTheOtherVersionsWay() throws {
        assert(try refused { json in
            json["version"] = 1
        }, contains: "version 1 files name pins by number")
        assert(try refused { json in
            first(&json, "wires") { wire in
                var from = wire["from"] as! JSON
                from["pin"] = 3
                wire["from"] = from
            }
        }, contains: "version 2 files name pins by designator")
    }

    func testAVersionOneFileNamesPinsByNumber() throws {
        var json = try base()
        json["version"] = 1
        var wires = json["wires"] as! [JSON]
        for i in wires.indices {
            for end in ["from", "to"] {
                var value = wires[i][end] as! JSON
                if value["pin"] != nil { value["pin"] = 3 }
                wires[i][end] = value
            }
        }
        json["wires"] = wires
        XCTAssertNil(try open(json), "pin 3 by number")
        first(&json, "wires") { wire in
            var from = wire["from"] as! JSON
            from["pin"] = 99
            wire["from"] = from
        }
        assert(try XCTUnwrap(try open(json)), contains: "connects to pin 99 of U")
    }

    func testAnEmbeddedPartThatCannotBeLoaded() throws {
        assert(try refused { json in
            json["version"] = 5
            json["customParts"] = [["format": "not-a-part"]]
        }, contains: "custom parts could not be loaded")
    }

    // MARK: Parts

    func testAPartOptionItDoesNotOffer() throws {
        assert(try refused { json in first(&json, "parts") { part in
            var pcb = part["pcb"] as! JSON
            pcb["rotation"] = 45
            part["pcb"] = pcb
        } }, contains: "PCB rotation 45")
    }

    func testAnLEDColorOrFootprintAndAHeaderGenderTheyDoNotOffer() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "GENERIC-HEADER-1X02", at: CGPoint(x: 400, y: 100))
        let data = try SchematicFile(document: doc, fileURL: nil, catalog: TestSupport.catalog).encoded()
        var json = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? JSON)
        var parts = json["parts"] as! [JSON]
        let led = try XCTUnwrap(parts.firstIndex { $0["partNumber"] as? String == "GENERIC-LED" })
        let header = try XCTUnwrap(parts.firstIndex { $0["partNumber"] as? String == "GENERIC-HEADER-1X02" })
        parts[led]["color"] = "Plaid"
        json["parts"] = parts
        assert(try XCTUnwrap(try open(json)), contains: "\"Plaid\" is not a color of")
        parts[led]["color"] = nil
        parts[led]["footprint"] = "NO-SUCH-FOOTPRINT"
        json["parts"] = parts
        assert(try XCTUnwrap(try open(json)), contains: "\"NO-SUCH-FOOTPRINT\" is not a footprint of")
        parts[led]["footprint"] = nil
        parts[header]["gender"] = "Neither"
        json["parts"] = parts
        assert(try XCTUnwrap(try open(json)), contains: "\"Neither\" is not a gender of")
    }

    // MARK: Wires

    func testAWireToSomethingThatIsNotThere() throws {
        let nowhere = UUID().uuidString
        func end(_ value: JSON) throws -> String {
            try refused { json in first(&json, "wires") { $0["to"] = value } }
        }
        assert(try end(["part": nowhere, "pin": "1"]), contains: "connects to part \(nowhere)")
        assert(try end(["junction": nowhere]), contains: "ends at junction \(nowhere)")
        assert(try end(["port": nowhere, "wire": nowhere]), contains: "connects to port \(nowhere)")
    }

    // MARK: The board

    func testABoardSizeLayerCountOrLayerStyleOutOfRange() throws {
        assert(try refused { json in board(&json) { $0["width"] = 0 } }, contains: "board width 0 must be 1 to 1000 mm")
        assert(try refused { json in board(&json) { $0["layers"] = 3 } }, contains: "board layers 3 must be 1, 2, 4 or 6")
        assert(try refused { json in board(&json) { $0["layerStyles"] = [["layer": "top", "clearance": 1]] } },
               contains: "layer top is not a plane; it has no clearance")
        assert(try refused { json in board(&json) { $0["layerStyles"] = [["layer": "inner2", "type": "plane", "clearance": 50]] } },
               contains: "layer inner2 clearance 50 must be 0.05 to 10 mm")
    }

    func testAPlaneClearanceIsRead() throws {
        var json = try base()
        board(&json) { $0["layerStyles"] = [["layer": "inner2", "type": "plane", "net": "ground", "clearance": 0.5]] }
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(try JSONSerialization.data(withJSONObject: json))
            .apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(doc.board.styles[.inner(2)]?.clearance, 0.5)
    }

    func testANetClassOrNetRuleThatIsNotValid() throws {
        func netClass(_ edit: @escaping (inout JSON) -> Void) throws -> String {
            try refused { json in board(&json) { board in
                var classes = board["netClasses"] as! [JSON]
                edit(&classes[0])
                board["netClasses"] = classes
            } }
        }
        assert(try netClass { $0["layerMode"] = "only" }, contains: "needs both layerMode and layers, or neither")
        assert(try netClass { $0["layerMode"] = "maybe"; $0["layers"] = ["top"] }, contains: "layer mode \"maybe\" must be only or prefer")

        let rule: JSON = ["net": "ground", "mode": "only", "layers": ["top"]]
        assert(try refused { json in board(&json) { $0["netLayerRules"] = [["net": "nothing", "mode": "only", "layers": ["top"]]] } },
               contains: "net layer rule \"nothing\" net must be")
        assert(try refused { json in board(&json) { $0["netLayerRules"] = [rule, rule] } },
               contains: "net layer rule \"ground\" is listed twice")
    }

    func testANetInTwoGroups() throws {
        let members = ["ground", "power:5000"]
        assert(try refused { json in board(&json) { $0["netGroups"] = [
            ["id": UUID().uuidString, "name": "A", "mode": "lengthMatch", "members": members],
            ["id": UUID().uuidString, "name": "B", "mode": "lengthMatch", "members": members],
        ] } }, contains: "member \"ground\" is in another group")
    }

    func testABoardTextThatIsNotValid() throws {
        func text(_ edit: @escaping (inout JSON) -> Void) throws -> String {
            try refused { json in board(&json) { board in
                var text: JSON = ["id": UUID().uuidString, "text": "T", "x": 5, "y": 5]
                edit(&text)
                board["texts"] = [text]
            } }
        }
        assert(try text { $0["text"] = "" }, contains: "is empty")
        assert(try text { $0["size"] = 5 }, contains: "size 5 must be 10 to 1000 mil")
        assert(try text { $0["rotation"] = 45 }, contains: "rotation 45 must be 0, 90, 180 or 270")
    }

    func testAHoleNetThatIsNotANet() throws {
        assert(try refused { json in board(&json) { board in
            var holes = board["holes"] as! [JSON]
            holes[0]["net"] = "somewhere"
            board["holes"] = holes
        } }, contains: "net \"somewhere\" must be ground, power:<mV> or pin:<part id>/<pin>")
    }

    // MARK: Buses

    private func bus(lines: [(CGPoint, CGPoint)], wires: [String] = ["A", "B"], ports: [JSON] = [],
                     extra: JSON = [:]) -> (JSON, [String]) {
        let lineIDs = lines.map { _ in UUID().uuidString }
        var bus: JSON = [
            "id": UUID().uuidString,
            "lines": zip(lineIDs, lines).map { id, line in
                ["id": id, "a": ["x": line.0.x, "y": line.0.y], "b": ["x": line.1.x, "y": line.1.y]] as JSON
            },
            "wires": wires.map { ["id": UUID().uuidString, "name": $0] as JSON },
            "ports": ports,
        ]
        for (key, value) in extra { bus[key] = value }
        return (bus, lineIDs)
    }

    private func refusedBus(_ buses: [JSON]) throws -> String {
        try refused { $0["buses"] = buses }
    }

    func testABusThatIsNotValid() throws {
        let across = (CGPoint(x: 0, y: 1000), CGPoint(x: 400, y: 1000))
        assert(try refusedBus([bus(lines: []).0]), contains: "has no lines")
        assert(try refusedBus([bus(lines: [(CGPoint(x: 0, y: 1000), CGPoint(x: 400, y: 1300))]).0]),
               contains: "is not a horizontal or vertical line on the grid")
        assert(try refusedBus([bus(lines: [across, (CGPoint(x: 200, y: 1000), CGPoint(x: 600, y: 1000))]).0]),
               contains: "overlap")
        assert(try refusedBus([bus(lines: [across, (CGPoint(x: 600, y: 1000), CGPoint(x: 800, y: 1000))]).0]),
               contains: "is not one piece")
        assert(try refusedBus([bus(lines: [across], wires: ["A"]).0]), contains: "has 1 wires; it must have 2 to 999")
        assert(try refusedBus([bus(lines: [across], wires: ["A", "B C"]).0]), contains: "wire name \"B C\" must be 1 to")
        assert(try refusedBus([bus(lines: [across], extra: ["type": "nothing"]).0]), contains: "The bus is not valid: bus ")
    }

    func testABusJointThatIsNotATOrAnL() throws {
        // Four lines meeting at one point: a cross.
        let c = CGPoint(x: 400, y: 1000)
        let message = try refusedBus([bus(lines: [(CGPoint(x: 200, y: 1000), c), (c, CGPoint(x: 600, y: 1000)),
                                                  (CGPoint(x: 400, y: 800), c), (c, CGPoint(x: 400, y: 1200))]).0])
        assert(message, contains: "has a joint at (400, 1000) that is not a T or an L")
    }

    func testABusPortOffItsBusOrOnTheWrongSide() throws {
        let across = (CGPoint(x: 0, y: 1000), CGPoint(x: 400, y: 1000))
        let (plain, ids) = bus(lines: [across])
        var wrongSide = plain
        wrongSide["ports"] = [["id": UUID().uuidString, "line": ids[0], "side": "left", "start": 100, "wires": []]]
        assert(try refusedBus([wrongSide]), contains: "side \"left\" must be top or bottom on a horizontal line")
        var offBus = plain
        let nowhere = UUID().uuidString
        offBus["ports"] = [["id": UUID().uuidString, "line": nowhere, "side": "top", "start": 100, "wires": []]]
        assert(try refusedBus([offBus]), contains: "line \(nowhere) is not on bus")
    }

    func testTwoBusesThatTouch() throws {
        let one = bus(lines: [(CGPoint(x: 0, y: 1000), CGPoint(x: 400, y: 1000))]).0
        let two = bus(lines: [(CGPoint(x: 400, y: 1000), CGPoint(x: 400, y: 1400))]).0
        assert(try refusedBus([one, two]), contains: "touch at (400, 1000)")
    }

    func testAWireToAPortPinThePortDoesNotHave() throws {
        let (plain, ids) = bus(lines: [(CGPoint(x: 0, y: 1000), CGPoint(x: 400, y: 1000))])
        var withPort = plain
        let port = UUID().uuidString
        let wires = (plain["wires"] as! [JSON]).map { $0["id"] as! String }
        withPort["ports"] = [["id": port, "line": ids[0], "side": "top", "start": 100, "wires": [wires[0]]]]
        let message = try refused { json in
            json["buses"] = [withPort]
            first(&json, "wires") { $0["to"] = ["port": port, "wire": wires[1]] }
        }
        assert(message, contains: "connects to a pin port \(port) does not have")
    }
}
