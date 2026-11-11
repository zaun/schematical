import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `boards.json` and the expressions its numbers are written in: every
/// board loads and builds with every combination of its options.
final class BoardCatalogTests: XCTestCase {
    private func value(_ text: String, _ names: [String: BoardValue] = [:]) throws -> BoardValue {
        try BoardExpression.parse(text).evaluate { names[$0] }
    }

    private func number(_ text: String, _ names: [String: BoardValue] = [:]) throws -> Double {
        guard case .number(let number) = try value(text, names) else { throw BoardExpression.Failure(description: text) }
        return number
    }

    private func failure(_ text: String, _ names: [String: BoardValue] = [:]) -> String? {
        do {
            _ = try value(text, names)
            return nil
        } catch {
            return BoardCatalog.describe(error)
        }
    }

    func testExpressions() throws {
        XCTAssertEqual(try number("2.54"), 2.54)
        XCTAssertEqual(try number("0.1in"), 2.54, accuracy: 1e-12)
        XCTAssertEqual(try number("100mil"), 2.54, accuracy: 1e-12)
        XCTAssertEqual(try number("3mm"), 3)
        XCTAssertEqual(try number("1 + 2 * 3"), 7)
        XCTAssertEqual(try number("(1 + 2) * 3"), 9)
        XCTAssertEqual(try number("-2 - -3"), 1)
        XCTAssertEqual(try number("7 % 3"), 1)
        XCTAssertEqual(try number("height - 0.3in", ["height": .number(100)]), 100 - 7.62, accuracy: 1e-12)
        XCTAssertEqual(try value("n in [12, 13, 50]", ["n": .number(13)]), .bool(true))
        XCTAssertEqual(try value("!(n in [12, 13])", ["n": .number(14)]), .bool(true))
        XCTAssertEqual(try value("key == 'bm' || key == 'b'", ["key": .string("b")]), .bool(true))
        XCTAssertEqual(try number("n == 1 ? 10 : n == 2 ? 20 : 30", ["n": .number(2)]), 20, "nests to the right")
        XCTAssertEqual(try number("a && b ? 1 : 0", ["a": .bool(true), "b": .bool(false)]), 0)
        XCTAssertEqual(try number("ceil(4.04in)"), 103)
        XCTAssertEqual(try number("floor(5 / 2)"), 2)
        XCTAssertEqual(try number("max(1, 4, 2) + min(3, -1)"), 3)
        XCTAssertEqual(try number("cos(180)"), -1, accuracy: 1e-12)
        XCTAssertEqual(try number("interp(5, 0, 0, 10, 1)"), 0.5)
        XCTAssertEqual(try number("interp(20, 0, 0, 10, 1)"), 1, "the end value beyond the points")

        XCTAssertEqual(failure("width"), "unknown name \"width\"")
        XCTAssertEqual(failure("1 +"), "\"1 +\" ends early")
        XCTAssertEqual(failure("2ft"), "unknown unit \"ft\" in \"2ft\"")
        XCTAssertEqual(failure("frob(1)"), "unknown function \"frob\" in \"frob(1)\"")
        XCTAssertEqual(failure("'a' + 1"), "\"a\" is not a number (+)")
        XCTAssertEqual(failure("1 ? 2 : 3"), "1 is not true or false (?)")
        XCTAssertEqual(failure("(1"), "expected \")\" in \"(1\"")
    }

    func testTheBundledCatalogLoads() {
        XCTAssertNil(BoardCatalog.shared.loadError)
        XCTAssertGreaterThanOrEqual(BoardCatalog.shared.boards.count, 23)
        XCTAssertEqual(BoardType.allCases.prefix(2), [.custom, .customCard])
        XCTAssertNil(BoardType(rawValue: "eisa"))
        XCTAssertEqual(BoardType(rawValue: "pcie")?.title, "PCI Express")
    }

    /// Every combination of every board's option choices.
    private func combinations(_ options: [BoardOption]) -> [CardOptions] {
        options.reduce([CardOptions()]) { partial, option in
            partial.flatMap { base in
                option.choices.map { choice in
                    var next = base
                    next[option.id] = choice.value
                    return next
                }
            }
        }
    }

    func testEveryBoardBuildsWithEveryOption() throws {
        for board in BoardCatalog.shared.boards {
            for options in combinations(board.options) {
                let label = "\(board.id) \(options.values)"
                let spec: CardSpec
                do {
                    spec = try board.build(board.resolved(options), width: nil)
                } catch {
                    XCTFail("\(label): \(BoardCatalog.describe(error))")
                    continue
                }
                XCTAssertTrue(spec.minimumLength <= spec.length && spec.length <= spec.maximumLength, label)
                XCTAssertGreaterThanOrEqual(spec.outline.count, 3, label)
                XCTAssertFalse(spec.fingers.isEmpty, label)
                // Within the board's box, and every pin and hole on the board.
                let shape = CGMutablePath()
                shape.addLines(between: spec.outline)
                shape.closeSubpath()
                for point in spec.outline {
                    XCTAssertTrue(point.x > -1e-9 && point.x < spec.length + 1e-9 && point.y > -1e-9
                                  && point.y < spec.height + 1e-9, "\(label) \(point)")
                }
                for pin in spec.fingers + spec.extraPads {
                    XCTAssertTrue(shape.contains(CGPoint(x: pin.rect.midX, y: pin.rect.midY)), "\(label) \(pin.name)")
                }
                for hole in spec.holes { XCTAssertTrue(shape.contains(hole.centre), "\(label) hole") }
                // One pad per name and side.
                let places = spec.fingers.map { "\($0.name) \($0.isThroughHole ? "both" : $0.side.rawValue)" }
                XCTAssertEqual(Set(places).count, places.count, "\(label) duplicate pins")
                // A given pin order names every pin once.
                if let order = spec.pinOrder {
                    XCTAssertEqual(order.count, Set(order).count, label)
                    XCTAssertEqual(Set(order), Set(spec.fingers.map(\.name)), label)
                }
                XCTAssertTrue((spec.pinOrder ?? spec.fingers.map(\.name)).allSatisfy { (1...4).contains($0.count) },
                              "\(label): bus wire names are 1 to 4 characters")
            }
        }
    }

    /// A broken file says where.
    func testErrors() {
        func error(_ json: String) -> String? {
            do {
                _ = try BoardCatalog(data: Data(json.utf8))
                return nil
            } catch {
                return BoardCatalog.describe(error)
            }
        }
        let board = #""title": "T", "height": 10, "width": 10, "outline": [[0, 0], [10, 0], [10, 10]], "pins": []"#
        XCTAssertNil(error(#"{ "boards": [{ "id": "t", \#(board) }] }"#))
        XCTAssertEqual(error(#"{ "boards": [{ "id": "custom", \#(board) }] }"#), "board id \"custom\" is built in")
        XCTAssertEqual(error(#"{ "boards": [{ "id": "t", \#(board) }, { "id": "t", \#(board) }] }"#),
                       "board id \"t\" is used twice")
        XCTAssertEqual(error(#"{ "boards": [{ "id": "t", "title": "T", "height": "1 +", "width": 1, "outline": [], "pins": [] }] }"#),
                       ".boards[0].height: \"1 +\" ends early")
        XCTAssertEqual(error(#"{ "boards": [{ "id": "t", "title": "T", "height": 1, "outline": [], "pins": [] }] }"#),
                       ".boards[0].width: board t needs width or length, not both")
    }
}
