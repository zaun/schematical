import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Autoroute's options (plan-38-D): their JSON, kept between runs.
final class RoutingOptionsTests: XCTestCase {
    func testStandardRoundTrips() throws {
        let data = try JSONEncoder().encode(RoutingOptions.standard)
        XCTAssertEqual(try JSONDecoder().decode(RoutingOptions.self, from: data), .standard)
    }

    func testEveryFieldRoundTrips() throws {
        var options = RoutingOptions()
        options.scope = .nets([.ground, .pin(chip: UUID(), designator: "A1")])
        options.include = .untouchedNets
        options.existing = .clearAuto
        options.layers = [.bottom: .init(enabled: false, direction: .vertical), .inner(3): .init()]
        options.vias = .none
        options.angle = .orthogonal
        options.strategy = .thorough
        options.grid = 0.127
        options.widthOverride = 12
        options.clearanceOverride = 0.2
        options.fanoutPlanes = false
        options.pushAndShove = true
        options.seed = 42
        options.timeLimit = 30
        let data = try JSONEncoder().encode(options)
        XCTAssertEqual(try JSONDecoder().decode(RoutingOptions.self, from: data), options)

        options.scope = .connection(from: CGPoint(x: 1, y: 2), to: CGPoint(x: 3, y: 4))
        XCTAssertEqual(try JSONDecoder().decode(RoutingOptions.self, from: try JSONEncoder().encode(options)), options)
    }

    /// A value saved before plan-66 has no `pushAndShove` key: it decodes as off.
    func testAnOldSavedValueHasPushAndShoveOff() throws {
        let data = try JSONEncoder().encode(RoutingOptions.standard)
        var json = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
        XCTAssertNotNil(json["pushAndShove"], "the key is saved")
        json["pushAndShove"] = nil
        let old = try JSONSerialization.data(withJSONObject: json)
        let decoded = try JSONDecoder().decode(RoutingOptions.self, from: old)
        XCTAssertFalse(decoded.pushAndShove)
        XCTAssertEqual(decoded, RoutingOptions.standard)
        XCTAssertFalse(RoutingOptions.standard.pushAndShove, "off by default")
    }

    func testUnknownStrategyFails() {
        let json = #"{ "strategy": "reckless" }"#
        XCTAssertThrowsError(try JSONDecoder().decode(RoutingOptions.self, from: Data(json.utf8)))
    }

    func testGridIsOneOfFour() throws {
        XCTAssertEqual(RoutingOptions.grids, [0.05, 0.1, 0.127, 0.254])
        XCTAssertTrue(RoutingOptions.grids.contains(RoutingOptions.standard.grid))
        XCTAssertThrowsError(try JSONDecoder().decode(RoutingOptions.self, from: Data(#"{ "grid": 0.3 }"#.utf8)))
        XCTAssertEqual(try JSONDecoder().decode(RoutingOptions.self, from: Data(#"{ "grid": 0.254 }"#.utf8)).grid, 0.254)
    }
}
