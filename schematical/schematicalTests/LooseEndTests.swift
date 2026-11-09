import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-30: loose wire ends. Removing a pin keeps the wires that were on it,
/// ending on a loose junction at the pin's old tip.
@MainActor
final class LooseEndTests: XCTestCase {
    // MARK: Phase A1 — the flag and the file

    /// A loose junction reopens loose; a plain one reopens plain and writes
    /// no `loose` key.
    func testLooseJunctionRoundTrips() throws {
        let doc = TestSupport.newDocument()
        let loose = Junction(point: CGPoint(x: 100, y: 100), isLoose: true)
        let plain = Junction(point: CGPoint(x: 200, y: 100))
        doc.replaceContents(
            chips: [],
            wires: [Wire(start: .junction(loose.id), end: .junction(plain.id), interior: [])],
            junctions: [loose, plain],
            buses: [],
            nextReference: 1,
            board: BoardSettings(),
            texts: [],
            traces: []
        )
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
        let junctions = try XCTUnwrap(json["junctions"] as? [[String: Any]])
        let byID = Dictionary(uniqueKeysWithValues: junctions.map { ($0["id"] as? String ?? "", $0) })
        XCTAssertEqual(byID[loose.id.uuidString]?["loose"] as? Bool, true)
        XCTAssertNotNil(byID[plain.id.uuidString])
        XCTAssertNil(byID[plain.id.uuidString]?["loose"])

        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.junctions.first { $0.id == loose.id }?.isLoose, true)
        XCTAssertEqual(opened.junctions.first { $0.id == plain.id }?.isLoose, false)
    }
}
