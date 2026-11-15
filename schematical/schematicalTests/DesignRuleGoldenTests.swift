import Foundation
import XCTest
@testable import schematical

/// The design rule check's whole output on the router fixtures, line for
/// line, against `Fixtures/drc-<name>.txt` (generated once, before plan-38-D
/// moved the DRC's copper geometry into `Routing/CopperGeometry.swift`, and
/// never regenerated to make a change pass). A missing golden is written
/// and the test fails, so a new one is looked at before it is committed.
@MainActor
final class DesignRuleGoldenTests: XCTestCase {
    static func render(_ issues: [DesignRuleIssue]) -> String {
        issues.map { "\($0.index)\t\($0.reference)\t\($0.issue)" }.joined(separator: "\n") + "\n"
    }

    /// AFU with copper on it, so the golden reaches the clearance, short,
    /// edge, via and width checks (the three fixtures have no traces or
    /// vias): a straight trace along each of its first 60 unrouted lines,
    /// on Top and Bottom in turn, 5 to 30 mil wide, and a via at the middle
    /// of every fourth; then a trace along the board's top edge.
    static func afuWithCopper() throws -> SchematicDocument {
        let doc = try Fixture.open("AFU")
        for (n, line) in doc.ratsnest().prefix(60).enumerated() {
            let layer: CopperLayer = n.isMultiple(of: 2) ? .top : .bottom
            doc.addTrace(points: [line.0, line.1], layer: layer, width: 5 + (n % 6) * 5)
            if n.isMultiple(of: 4), let via = doc.addVia(at: CGPoint(x: (line.0.x + line.1.x) / 2, y: (line.0.y + line.1.y) / 2)) {
                // The size the golden was made with, whatever the Default class is.
                doc.setViaDiameter(via, 0.8)
                doc.setViaDrill(via, 0.4)
            }
        }
        doc.addTrace(points: [CGPoint(x: 1, y: 0.05), CGPoint(x: 30, y: 0.05)], layer: .top, width: 10)
        // A via under the DRC's least drill and ring.
        if let small = doc.addVia(at: CGPoint(x: 40, y: 40)) {
            // From 0.4 / 0.8 mm, as the golden was made, whatever the Default class is.
            doc.setViaDiameter(small, 0.8)
            doc.setViaDrill(small, 0.4)
            doc.setViaDiameter(small, 0.3)
            doc.setViaDrill(small, 0.15)
        }
        return doc
    }

    func testDesignRuleCheckMatchesTheGoldens() throws {
        for name in Fixture.names + ["AFU-copper"] {
            let doc = name == "AFU-copper" ? try Self.afuWithCopper() : try Fixture.open(name)
            let text = Self.render(doc.designRuleCheck(rules: .customDefault))
            let url = Fixture.directory.appendingPathComponent("drc-\(name).txt")
            guard let golden = try? String(contentsOf: url, encoding: .utf8) else {
                try text.write(to: url, atomically: true, encoding: .utf8)
                XCTFail("wrote \(url.lastPathComponent); check it and run again")
                continue
            }
            if text != golden {
                let mine = text.split(separator: "\n", omittingEmptySubsequences: false)
                let theirs = golden.split(separator: "\n", omittingEmptySubsequences: false)
                let first = zip(mine, theirs).enumerated().first { $0.element.0 != $0.element.1 }
                XCTFail("\(name): \(mine.count) lines vs golden \(theirs.count); first difference at line "
                        + "\((first?.offset ?? min(mine.count, theirs.count)) + 1): "
                        + "\(first.map { "\"\($0.element.0)\" vs \"\($0.element.1)\"" } ?? "length")")
            }
        }
    }
}
