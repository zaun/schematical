import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Regression anchors (plan-66-A): the router's exact output with every option
/// left as it is, on four small boards and RegFile. Push and shove (plan-66)
/// is off by default, so none of these numbers may move when it lands; a
/// change here with the option off is a bug to root-cause, not a number to
/// refresh. The literals were taken from the unmodified tree.
@MainActor
final class RoutingAnchorTests: XCTestCase {
    struct Anchor {
        var routed: Int
        var vias: Int
        var bends: Int
        var rips: Int
        var wireLength: Double
        var hash: UInt64
    }

    /// FNV-1a over every trace and via of the result, each point rounded to
    /// 1e-6 mm, in the order the result returns them.
    static func geometryHash(_ result: RoutingResult) -> UInt64 {
        var text = ""
        for trace in result.traces {
            text += "T \(trace.layer) \(trace.net ?? -1) \(String(format: "%.6f", trace.width))"
            for p in trace.points { text += String(format: " %.6f,%.6f", p.x, p.y) }
            text += "\n"
        }
        for via in result.vias {
            text += String(format: "V %d %.6f,%.6f %.6f %.6f\n", via.net ?? -1, via.centre.x, via.centre.y, via.drill, via.diameter)
        }
        var hash: UInt64 = 0xcbf29ce484222325
        for byte in text.utf8 {
            hash ^= UInt64(byte)
            hash = hash &* 0x100000001b3
        }
        return hash
    }

    private func check(_ name: String, _ result: RoutingResult, _ expected: Anchor?, file: StaticString = #filePath, line: UInt = #line) {
        let s = result.stats
        let hash = Self.geometryHash(result)
        print(String(format: "ANCHOR %@ routed %d vias %d bends %d rips %d wire %.9f hash 0x%016llx",
                     name, s.routed, s.vias, s.bends, s.rips, s.wireLength, hash))
        guard let expected else {
            XCTFail("\(name): no literals yet", file: file, line: line)
            return
        }
        XCTAssertEqual(s.routed, expected.routed, "\(name) routed", file: file, line: line)
        XCTAssertEqual(s.vias, expected.vias, "\(name) vias", file: file, line: line)
        XCTAssertEqual(s.bends, expected.bends, "\(name) bends", file: file, line: line)
        XCTAssertEqual(s.rips, expected.rips, "\(name) rips", file: file, line: line)
        XCTAssertEqual(s.wireLength, expected.wireLength, accuracy: 1e-6, "\(name) wire", file: file, line: line)
        XCTAssertEqual(hash, expected.hash, "\(name) geometry", file: file, line: line)
    }

    private func route(_ board: RoutingTestBoard, _ strategy: RoutingOptions.Strategy) -> RoutingResult {
        var board = board
        board.options.strategy = strategy
        return Router.route(board.problem())
    }

    func testTwoGapsFast() { check("twoGaps fast", route(.twoGaps(), .fast),
          Anchor(routed: 2, vias: 0, bends: 4, rips: 1, wireLength: 36.615642097, hash: 0xdf4e88531f67279b)) }
    func testTwoGapsBalanced() { check("twoGaps balanced", route(.twoGaps(), .balanced),
          Anchor(routed: 2, vias: 0, bends: 4, rips: 0, wireLength: 36.615642097, hash: 0xab4987f85173f95b)) }
    func testReversedChannelFast() { check("reversedChannel fast", route(.reversedChannel(), .fast),
          Anchor(routed: 6, vias: 10, bends: 8, rips: 0, wireLength: 114.538390976, hash: 0x53d2b02a4abf77b5)) }
    func testReversedChannelBalanced() { check("reversedChannel balanced", route(.reversedChannel(), .balanced),
          Anchor(routed: 6, vias: 0, bends: 21, rips: 0, wireLength: 146.413203436, hash: 0x203e2d4abac6d269)) }
    func testCrossingFast() { check("crossing fast", route(.crossing(), .fast),
          Anchor(routed: 8, vias: 8, bends: 35, rips: 0, wireLength: 227.55709164, hash: 0x6ae004434ab0d123)) }
    func testCrossingBalanced() { check("crossing balanced", route(.crossing(), .balanced),
          Anchor(routed: 8, vias: 2, bends: 37, rips: 0, wireLength: 247.658509196, hash: 0xbb5ed40bd4dd812e)) }
    func testParallelRowsFast() { check("parallelRows fast", route(.parallelRows(), .fast),
          Anchor(routed: 3, vias: 0, bends: 0, rips: 0, wireLength: 42.0, hash: 0xe2a57a72bfa28764)) }
    func testParallelRowsBalanced() { check("parallelRows balanced", route(.parallelRows(), .balanced),
          Anchor(routed: 3, vias: 0, bends: 0, rips: 0, wireLength: 42.0, hash: 0xe2a57a72bfa28764)) }

    /// RegFile at the fixture strategy (Fast in Debug), through the whole
    /// document path. Balanced in Release is `RouterQualityFixtureTests`.
    func testRegFileFastIsPinned() throws {
        #if !DEBUG
        throw XCTSkip("the anchor is the Debug (Fast) run; Release's Balanced is RouterQualityFixtureTests")
        #endif
        let problem = try Fixture.open("RegFile").routingProblem(options: .fixture, rules: .customDefault)
        check("RegFile fast", Router.route(problem),
          Anchor(routed: 24, vias: 29, bends: 87, rips: 54, wireLength: 301.450727907, hash: 0x555ca4e6e0ccf37d))
    }
}
