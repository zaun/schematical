import Foundation
import XCTest
@testable import schematical

/// `PartDraft`'s rows, its reading of a file's odd shapes, its inline
/// footprints and the pin rename walk (plan-68-AD).
@MainActor
final class PartDraftTests: XCTestCase {
    // MARK: Rows compare by content, not by row id

    func testRowsEqualAndHashByContent() {
        let a = PinDraft(designator: "1", name: "A", role: "in")
        var b = PinDraft(designator: "1", name: "A", role: "in")
        XCTAssertNotEqual(a.id, b.id)
        XCTAssertEqual(a, b)
        XCTAssertEqual(Set([a, b]).count, 1)
        b.activeLow = ["A"]
        XCTAssertNotEqual(a, b)
        XCTAssertEqual(Set([a, b]).count, 2)

        let choice = FootprintChoiceDraft(package: "DIP", footprint: "PDIP-14")
        XCTAssertEqual(choice, FootprintChoiceDraft(package: "DIP", footprint: "PDIP-14"))
        XCTAssertNotEqual(choice, FootprintChoiceDraft(package: "DIP", footprint: "PDIP-16"))
        XCTAssertEqual(Set([choice, FootprintChoiceDraft(package: "DIP", footprint: "PDIP-14")]).count, 1)

        let pinout = PinoutDraft(name: "DIP", style: .numbered, pins: [a])
        XCTAssertEqual(Set([pinout, PinoutDraft(name: "DIP", style: .numbered, pins: [b])]).count, 2)
        XCTAssertEqual(Set([pinout, PinoutDraft(name: "DIP", style: .numbered, pins: [a])]).count, 1)
    }

    // MARK: Reading a file

    /// A gate's file, with its one device changed by `change`.
    private func file(_ change: (inout [String: JSONValue]) -> Void) throws -> JSONValue {
        guard case var .object(root) = PartDraft(newOf: "gate").encoded(),
              case var .array(devices)? = root["devices"], case var .object(device)? = devices.first else {
            XCTFail("unexpected gate file shape")
            return .null
        }
        change(&device)
        devices[0] = .object(device)
        root["devices"] = .array(devices)
        return .object(root)
    }

    func testAFileWithoutExactlyOneDeviceIsRefused() throws {
        guard case var .object(root) = PartDraft(newOf: "gate").encoded() else { return XCTFail("not an object") }
        root["devices"] = .array([])
        XCTAssertThrowsError(try PartDraft(file: .object(root))) { error in
            XCTAssertEqual(error as? PartDraftError, .notAPartFile("not exactly one device with one part"))
        }
    }

    func testADrawerWithANonStringIsKeptAsAnExtraKey() throws {
        let draft = try PartDraft(file: file { $0["drawer"] = .array([.string("Custom"), .number(3)]) })
        XCTAssertEqual(draft.drawer, [])
        XCTAssertTrue(draft.extraKeys.contains { $0.contains("drawer") }, "\(draft.extraKeys)")
    }

    func testAPinWithNeitherNumberNorBallLeavesThePinoutsAsTheyAre() throws {
        let pin: JSONValue = .object(["name": .string("A"), "role": .string("in")])
        let draft = try PartDraft(file: file { $0["pinouts"] = .object(["DIP": .array([pin])]) })
        XCTAssertEqual(draft.pinouts, [])
        XCTAssertTrue(draft.extraKeys.contains { $0.contains("pinouts") }, "\(draft.extraKeys)")
    }

    // MARK: Inline footprints

    func testCustomFootprintIsTheEntryThePartNames() throws {
        var draft = PartDraft(newOf: "gate")
        let custom = CustomFootprintDraft.starter(id: "CUSTOM-A", pins: 2)
        draft.customFootprint = custom
        XCTAssertEqual(draft.customFootprint, custom)
        // An entry under another id is not the part's.
        draft.footprint = "CUSTOM-B"
        XCTAssertNil(draft.customFootprint)
        XCTAssertNil(draft.inlineFootprint)
    }

    func testInlineFootprintReadsAParametricEntry() throws {
        var draft = PartDraft(newOf: "gate")
        let parametric = ParametricFootprintDraft.starter(style: .dual, id: "CUSTOM-P", pins: 8)
        draft.inlineFootprint = .parametric(parametric)
        XCTAssertEqual(draft.footprint, "CUSTOM-P")
        XCTAssertEqual(draft.inlineFootprint?.id, "CUSTOM-P")
        guard case let .parametric(read)? = draft.inlineFootprint else { return XCTFail("not parametric") }
        XCTAssertEqual(read, parametric)
        XCTAssertNil(draft.customFootprint)

        // An entry of no known style is no inline footprint.
        draft.footprints = [.object(["id": .string("CUSTOM-P"), "style": .string("spiral")])]
        XCTAssertNil(draft.inlineFootprint)
    }

    // MARK: Renaming a pin through the settings

    func testPinRenameReachesEveryShape() {
        var rename = PinRename(old: "A", new: "IN")
        let pin = Field(key: "pin", label: "Pin", type: .pin)
        let fields: [Field] = [
            pin,
            Field(key: "pins", label: "Pins", type: .pins),
            Field(key: "object", label: "Object", type: .object([pin])),
            Field(key: "list", label: "List", type: .list([pin])),
            Field(key: "map", label: "Map", type: .map(keyLabel: "Name", [pin])),
            Field(key: "values", label: "Values", type: .valueMap(keyLabel: "Name", .pin)),
            Field(key: "listOf", label: "List of", type: .listOf(.pin)),
            Field(key: "oneOf", label: "One of", type: .oneOf([.integer, .pin])),
            Field(key: "oneOfNone", label: "One of", type: .oneOf([.integer])),
            Field(key: "json", label: "JSON", type: .json),
        ]
        let object: [String: JSONValue] = [
            "pin": .string("A"),
            "pins": .array([.string("A"), .string("A/CS"), .string("B")]),
            "object": .object(["pin": .string("A")]),
            "list": .array([.object(["pin": .string("A")]), .string("stray")]),
            "map": .object(["x": .object(["pin": .string("A")]), "y": .number(1)]),
            "values": .object(["x": .string("A")]),
            "listOf": .array([.string("A"), .string("B")]),
            "oneOf": .string("A"),
            "oneOfNone": .string("A"),
            "json": .string("A"),
        ]
        let result = rename.rewrite(object, fields)
        XCTAssertEqual(result["pin"], .string("IN"))
        XCTAssertEqual(result["pins"], .array([.string("IN"), .string("A/CS"), .string("B")]))
        XCTAssertEqual(result["object"], .object(["pin": .string("IN")]))
        XCTAssertEqual(result["list"], .array([.object(["pin": .string("IN")]), .string("stray")]))
        XCTAssertEqual(result["map"], .object(["x": .object(["pin": .string("IN")]), "y": .number(1)]))
        XCTAssertEqual(result["values"], .object(["x": .string("IN")]))
        XCTAssertEqual(result["listOf"], .array([.string("IN"), .string("B")]))
        XCTAssertEqual(result["oneOf"], .string("IN"))
        // No shape of the one-of matches a string, and JSON has no schema: both kept.
        XCTAssertEqual(result["oneOfNone"], .string("A"))
        XCTAssertEqual(result["json"], .string("A"))
        XCTAssertEqual(rename.count, 8)
    }
}
