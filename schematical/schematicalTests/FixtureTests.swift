import CryptoKit
import Foundation
import XCTest
@testable import schematical

/// The router fixtures are the boards the plan-38 spike measured.
@MainActor
final class FixtureTests: XCTestCase {
    /// `planning/plan-38-data/fixtures/SHA256SUMS`.
    private static let sums = [
        "AFU": "498b14cb3efad42a6a8fd9f5b8f338235ed52c3d3c05dd991bcbb6ca07408047",
        "LFU": "e833f664cf00f2383bd87387df0efb0482260bfffdce676aa0dd598a53760c6c",
        "RegFile": "05668c58e85158fa76ceb70267c0f2599adf941b9524e9f745ae68ca541b2a84",
    ]

    func testFixturesMatchTheirHashes() throws {
        for name in Fixture.names {
            let data = try Data(contentsOf: Fixture.url(name))
            let hash = SHA256.hash(data: data).map { String(format: "%02x", $0) }.joined()
            XCTAssertEqual(hash, Self.sums[name], name)
        }
    }

    func testUnroutedCounts() throws {
        let expected = ["AFU": 115, "LFU": 188, "RegFile": 24]
        for name in Fixture.names {
            XCTAssertEqual(try Fixture.open(name).ratsnest().count, expected[name], name)
        }
    }
}
