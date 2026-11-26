import Foundation
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-68-Y: what a schematic file that can't be opened says, and the file
/// format's small pieces.
@MainActor
final class SchematicFileErrorTests: XCTestCase {
    func testEveryErrorSaysWhatIsWrong() {
        let a = UUID(uuidString: "00000000-0000-4000-8000-00000000000A")!
        let b = UUID(uuidString: "00000000-0000-4000-8000-00000000000B")!
        let cases: [(SchematicFileError, String)] = [
            (.notASchematic, "This file is not a Schematical schematic."),
            (.newerVersion(9), "This schematic was saved by a newer version of Schematical (file format 9)."),
            (.unreadable("bad"), "The schematic could not be read: bad"),
            (.duplicateID(a), "The schematic uses the ID \(a.uuidString) more than once."),
            (.unknownParts(["X1", "X2"]), "The schematic uses parts that are not in the catalog: X1, X2."),
            (.missingPart(wire: a, part: b), "Wire \(a.uuidString) connects to part \(b.uuidString), which is not in the file."),
            (.missingPin(wire: a, reference: "U1", pin: "9"), "Wire \(a.uuidString) connects to pin 9 of U1, which has no such pin."),
            (.missingJunction(wire: a, junction: b), "Wire \(a.uuidString) ends at junction \(b.uuidString), which is not in the file."),
            (.missingPort(wire: a, port: b), "Wire \(a.uuidString) connects to port \(b.uuidString), which is not in the file."),
            (.missingPortPin(wire: a, port: b, busWire: a), "Wire \(a.uuidString) connects to a pin port \(b.uuidString) does not have."),
            (.missingLabel(wire: a, label: b), "Wire \(a.uuidString) connects to net label \(b.uuidString), which is not in the file."),
            (.badLabel("x"), "The net label is not valid: x."),
            (.badErc("y"), "The electrical rules check settings are not valid: y."),
            (.badRotation(reference: "U2", rotation: 45), "U2 has PCB rotation 45; it must be 0, 90, 180 or 270."),
            (.badPCB("z"), "The PCB settings are not valid: z."),
            (.badBus("w"), "The bus is not valid: w."),
            (.badNote("n"), "The text block is not valid: n."),
            (.badDocumentInfo("d"), "The document info is not valid: d."),
            (.badOhms(reference: "R1", ohms: 0), "R1 has resistance 0 Ω; it must be more than 0 and at most 1 GΩ."),
            (.badOhms(reference: "R1", ohms: 2.5), "R1 has resistance 2.5 Ω; it must be more than 0 and at most 1 GΩ."),
            (.badOhms(reference: "R1", ohms: .infinity), "R1 has resistance inf Ω; it must be more than 0 and at most 1 GΩ."),
            (.badCustomPart("p"), "One of this schematic's custom parts could not be loaded: p"),
            (.badFarads(reference: "C1", farads: 2), "C1 has capacitance 2 F; it must be more than 0 and at most 1 F."),
        ]
        for (error, message) in cases { XCTAssertEqual(error.errorDescription, message) }
        XCTAssertEqual(SchematicFileError.badPartOption(reference: "D1", detail: "\"blue\" is not a color of LED").errorDescription,
                       "D1: \"blue\" is not a color of LED")
    }

    func testTheFileTypeOpensByItsExtension() {
        XCTAssertEqual(SchematicFile.openableTypes.first, SchematicFile.contentType)
        XCTAssertTrue(SchematicFile.openableTypes.allSatisfy { $0.preferredFilenameExtension == SchematicFile.fileExtension
            || $0 == SchematicFile.contentType })
    }

    func testAVersionOneWireEndNamesItsPinByNumber() throws {
        let part = UUID()
        let end = SchematicFile.End.pin(part: part, pin: .position(14))
        let data = try JSONEncoder().encode(end)
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
        XCTAssertEqual(object["pin"] as? Int, 14)
        XCTAssertEqual(object["part"] as? String, part.uuidString)
        XCTAssertEqual(try JSONDecoder().decode(SchematicFile.End.self, from: data), end)
    }

    func testAnEmbeddedPartKeepsEveryKindOfJSONValue() throws {
        let text = #"{"a": null, "b": true, "c": 2.5, "d": "x", "e": [1, false], "f": {"g": null}}"#
        let value = try JSONDecoder().decode(JSONValue.self, from: Data(text.utf8))
        guard case let .object(object) = value else { return XCTFail("an object") }
        XCTAssertEqual(object["a"], .null)
        XCTAssertEqual(object["b"], .bool(true))
        XCTAssertEqual(object["c"], .number(2.5))
        XCTAssertEqual(object["d"], .string("x"))
        XCTAssertEqual(object["e"], .array([.number(1), .bool(false)]))
        XCTAssertEqual(object["f"], .object(["g": .null]))
        XCTAssertEqual(try JSONDecoder().decode(JSONValue.self, from: try JSONEncoder().encode(value)), value)
    }
}
