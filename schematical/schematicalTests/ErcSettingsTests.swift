import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-63-E phase 1: the ERC's settings: KiCad's defaults, undo, the file.
@MainActor
final class ErcSettingsTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    private func reopened(_ doc: SchematicDocument) throws -> SchematicDocument {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    private func open(erc: String) throws -> SchematicDocument {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [], "junctions": [], "wires": [], "erc": \(erc) }
        """
        let doc = newDocument()
        try SchematicFile.decode(Data(json.utf8)).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        return doc
    }

    func testKiCadsDefaults() {
        let expected: [ErcRule: ErcSeverity] = [
            .pinNotConnected: .error, .pinNotDriven: .error, .powerPinNotDriven: .error,
            .wireDangling: .error, .labelDangling: .error, .duplicateReference: .error,
            .noConnectConnected: .warning, .noConnectDangling: .warning, .unconnectedWireEndpoint: .warning,
            .isolatedPinLabel: .warning, .multipleNetNames: .warning, .similarLabels: .warning,
            .similarLabelAndPower: .warning, .singleGlobalLabel: .ignore, .fourWayJunction: .ignore,
        ]
        XCTAssertEqual(ErcRule.allCases.count, 16)
        let settings = ErcSettings()
        for rule in ErcRule.allCases where rule != .pinToPin {
            XCTAssertEqual(settings.severity(of: rule), expected[rule], rule.rawValue)
        }
        XCTAssertFalse(ErcRule.pinToPin.hasSeverity)
        XCTAssertTrue(settings.isDefault)
    }

    func testTheDefaultPinMapIsKiCads() {
        let map = ErcSettings.defaultPinMap
        XCTAssertEqual(map.count, 12)
        XCTAssertTrue(ErcSettings.isValid(pinMap: map))
        XCTAssertEqual(map[1][1], 2, "output + output is an error")
        XCTAssertEqual(map[1][3], 1, "output + tri_state is a warning")
        XCTAssertEqual(map[3][3], 0, "tri_state + tri_state is fine")
        XCTAssertEqual(map[7][8], 0, "power_in + power_out is fine")
        XCTAssertEqual(map.flatMap { $0 }.filter { $0 == 2 }.count, 37)
        XCTAssertEqual(map.flatMap { $0 }.filter { $0 == 1 }.count, 31)
    }

    func testAnEditIsOneUndoStepAndRedoRestoresIt() {
        let doc = newDocument()
        doc.setErcSeverity(.pinNotConnected, .warning)
        XCTAssertEqual(doc.erc.severity(of: .pinNotConnected), .warning)
        XCTAssertEqual(doc.undoName, "Change ERC Settings")
        doc.setErcPinMap(1, 1, to: 1)
        XCTAssertEqual(doc.erc.effectivePinMap[1][1], 1)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.erc.effectivePinMap[1][1], 2)
        XCTAssertEqual(doc.erc.severity(of: .pinNotConnected), .warning)
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.erc.isDefault)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.erc.severity(of: .pinNotConnected), .warning)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.erc.effectivePinMap[1][1], 1)

        doc.excludeErc(["pin_not_connected|chip:X:1"])
        XCTAssertEqual(doc.undoName, "Exclude Violation")
        doc.includeErc(["pin_not_connected|chip:X:1"])
        XCTAssertTrue(doc.erc.exclusions.isEmpty)
        XCTAssertEqual(doc.undoName, "Include Violation")
        doc.resetErcSettings()
        XCTAssertTrue(doc.erc.isDefault)
    }

    func testSettingTheDefaultChangesNothing() {
        let doc = newDocument()
        doc.setErcSeverity(.wireDangling, .error)
        XCTAssertNil(doc.undoName, "the default severity is no change")
        doc.setErcPinMap(1, 1, to: 2)
        XCTAssertNil(doc.undoName)
        doc.setErcSeverity(.pinToPin, .ignore)
        XCTAssertTrue(doc.erc.isDefault, "pin_to_pin has no severity")
    }

    func testAPinMapCellAndItsMirror() {
        let doc = newDocument()
        doc.setErcPinMap(1, 3, to: 0)
        XCTAssertEqual(doc.erc.effectivePinMap[1][3], 0)
        XCTAssertEqual(doc.erc.effectivePinMap[3][1], 0)
        XCTAssertTrue(ErcSettings.isValid(pinMap: doc.erc.effectivePinMap))
        doc.setErcPinMap(1, 3, to: 1)
        XCTAssertNil(doc.erc.pinMap, "back to the default map")
    }

    func testTheFileRoundTripKeepsEverything() throws {
        let doc = newDocument()
        doc.setErcSeverity(.pinNotConnected, .warning)
        doc.setErcSeverity(.fourWayJunction, .error)
        doc.setErcSeverity(.similarLabels, .ignore)
        doc.setErcPinMap(1, 1, to: 0)
        doc.excludeErc(["pin_not_driven|chip:A:1", "label_dangling|label:B"])
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let text = String(decoding: data, as: UTF8.self)
        XCTAssertTrue(text.contains("\"erc\""))
        XCTAssertTrue(text.contains("\"pin_not_connected\" : \"warning\""))
        let opened = try reopened(doc)
        XCTAssertEqual(opened.erc, doc.erc)
        XCTAssertEqual(opened.erc.severity(of: .fourWayJunction), .error)
        XCTAssertEqual(opened.erc.effectivePinMap[1][1], 0)
        XCTAssertEqual(opened.erc.exclusions, ["pin_not_driven|chip:A:1", "label_dangling|label:B"])
    }

    func testADefaultDocumentWritesNoErcKey() throws {
        let text = String(decoding: try SchematicFile(document: newDocument(), fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(text.contains("\"erc\""))
        let doc = newDocument()
        doc.setErcSeverity(.wireDangling, .warning)
        doc.setErcSeverity(.wireDangling, .error)
        let again = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(again.contains("\"erc\""), "a change put back is the default")
    }

    func testAFileWithNoErcOpensWithDefaults() throws {
        let doc = try open(erc: "{}")
        XCTAssertTrue(doc.erc.isDefault)
        let none = newDocument()
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [], "junctions": [], "wires": [] }
        """
        try SchematicFile.decode(Data(json.utf8)).apply(to: none, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertTrue(none.erc.isDefault)
    }

    func testBadSettingsAreRefusedNamingTheField() {
        func refused(_ erc: String, mentions word: String, file: StaticString = #filePath, line: UInt = #line) {
            XCTAssertThrowsError(try open(erc: erc), file: file, line: line) { error in
                guard case let SchematicFileError.badErc(message) = error else {
                    return XCTFail("\(error)", file: file, line: line)
                }
                XCTAssertTrue(message.contains(word), message, file: file, line: line)
            }
        }
        refused(#"{ "severities": { "no_such_rule": "error" } }"#, mentions: "erc.severities")
        refused(#"{ "severities": { "pin_to_pin": "error" } }"#, mentions: "erc.severities")
        refused(#"{ "severities": { "wire_dangling": "fatal" } }"#, mentions: "wire_dangling")
        refused(#"{ "pinMap": [[0]] }"#, mentions: "erc.pinMap")
        let twelve = (0..<12).map { _ in "[" + Array(repeating: "0", count: 12).joined(separator: ",") + "]" }
        var asymmetric = twelve
        asymmetric[0] = "[0,0,0,0,0,0,0,0,0,0,0,2]"   // [0][11] = 2 but [11][0] = 0
        refused("{ \"pinMap\": [\(asymmetric.joined(separator: ","))] }", mentions: "erc.pinMap")
        var big = twelve
        big[0] = "[3,0,0,0,0,0,0,0,0,0,0,0]"
        refused("{ \"pinMap\": [\(big.joined(separator: ","))] }", mentions: "erc.pinMap")
    }
}
