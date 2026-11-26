import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: what a broken `boards.json` says — a missing or unreadable
/// file, each malformed field, names that depend on themselves — and when
/// an option shows.
final class BoardCatalogEdgeTests: XCTestCase {
    private let finger = #""finger": {"x": "n", "width": 1, "top": 0, "bottom": 1, "side": "top"}"#

    /// A board's JSON with `extra` fields; `pins` replaces its pins.
    private func board(_ extra: String = "", pins: String? = nil, height: String = "10") -> String {
        let pins = pins ?? #"[{ "each": [1], \#(finger) }]"#
        return #"{ "boards": [{ "id": "t", "title": "T", "height": \#(height), "width": 10, "outline": [[0, 0], [10, 0], [10, 10]], "pins": \#(pins)\#(extra) }] }"#
    }

    private func error(_ json: String) -> String? {
        do {
            _ = try BoardCatalog(data: Data(json.utf8))
            return nil
        } catch {
            return BoardCatalog.describe(error)
        }
    }

    private func buildError(_ json: String, options: [String: BoardValue] = [:]) throws -> String? {
        let catalog = try BoardCatalog(data: Data(json.utf8))
        let board = try XCTUnwrap(catalog.board("t"))
        do {
            _ = try board.build(board.resolved(CardOptions()).merging(options) { $1 }, width: nil)
            return nil
        } catch {
            return BoardCatalog.describe(error)
        }
    }

    func testAMissingOrUnreadableFileIsAnEmptyCatalogWithTheReason() throws {
        let missing = BoardCatalog.load(nil)
        XCTAssertEqual(missing.boards.count, 0)
        XCTAssertEqual(missing.loadError, "boards.json is missing from the app")
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boards-\(UUID().uuidString).json")
        try Data("{ not json".utf8).write(to: url)
        defer { try? FileManager.default.removeItem(at: url) }
        let broken = BoardCatalog.load(url)
        XCTAssertEqual(broken.boards.count, 0)
        XCTAssertTrue(broken.loadError?.hasPrefix("boards.json: ") ?? false, broken.loadError ?? "")
        XCTAssertNil(broken.board("t"))
        XCTAssertNil(BoardCatalog.shared.loadError, "the bundled file loads")
    }

    func testEachMalformedFieldSaysWhere() {
        XCTAssertNil(error(board()))
        XCTAssertEqual(error(#"{ "boards": [{ "id": "t" }] }"#), ".boards[0]: title is missing")
        XCTAssertTrue(error(#"{ "boards": [{ "id": "t", "title": 5 }] }"#)?.hasPrefix(".boards[0].title: ") ?? false)
        XCTAssertEqual(error(board(#", "pinOrder": "sorted""#)), ".boards[0].pinOrder: pinOrder must be \"listed\" or a list of names")
        XCTAssertEqual(error(board(#", "options": [{ "id": "o", "title": "O", "choices": [{ "value": 1 }], "default": 2 }]"#)),
                       ".boards[0].options[0].default: option o's default isn't one of its choices")
        XCTAssertEqual(error(#"{ "boards": [{ "id": "t", "title": "T", "height": 1, "width": 1, "outline": [[0, 0, 0]], "pins": [] }] }"#),
                       ".boards[0].outline[0][2]: a point is [x, y]")
        XCTAssertEqual(error(board(pins: #"[{ \#(finger) }]"#)), ".boards[0].pins[0].each: pins need each, or from and to")
        XCTAssertEqual(error(board(pins: #"[{ "each": [1], "name": "P{n", \#(finger) }]"#)),
                       ".boards[0].pins[0].name: unclosed { in pin name \"P{n\"")
        XCTAssertEqual(error(board(pins: #"[{ "each": [1] }]"#)), ".boards[0].pins[0].finger: pins need a finger or a pad")
        XCTAssertEqual(BoardCatalog.describe(NSError(domain: "x", code: 1, userInfo: [NSLocalizedDescriptionKey: "plain"])), "plain")
    }

    func testExpressionsThatCanNotBeRead() {
        func failure(_ text: String) -> String? {
            do {
                _ = try BoardExpression.parse(text).evaluate { _ in nil }
                return nil
            } catch {
                return BoardCatalog.describe(error)
            }
        }
        XCTAssertEqual(failure("1 $ 2"), "unexpected \"$\" in \"1 $ 2\"")
        XCTAssertEqual(failure("interp(1, 2)"), "interp takes x and pairs of points")
        XCTAssertEqual(failure("interp(1, 2, 3, 4)"), "interp takes x and pairs of points")
        XCTAssertNil(failure("interp(1, 0, 0)"))
    }

    func testNamesThatDependOnThemselvesOrOnTheSize() throws {
        XCTAssertEqual(try buildError(board(#", "vars": { "a": "a + 1" }"#, height: "\"a\"")), "variable \"a\" depends on itself")
        XCTAssertEqual(try buildError(board(height: "\"width\"")), "width can't be used in the board's size")
        XCTAssertEqual(try buildError(board(height: "\"height + 1\"")), "height can't be used in the board's size")
        let selfLet = #"[{ "each": [1], "let": { "x": "x + 1" }, "finger": {"x": "x", "width": 1, "top": 0, "bottom": 1, "side": "top"} }]"#
        XCTAssertEqual(try buildError(board(pins: selfLet)), "\"x\" depends on itself")
        XCTAssertNil(try buildError(board(#", "vars": { "a": "4" }"#, height: "\"a + 1\"")))
        XCTAssertEqual(try buildError(board(height: "\"foo\"")), "unknown name \"foo\"")
    }

    func testAnOptionShowsWhenItsConditionHolds() throws {
        let options = #", "options": [{ "id": "big", "title": "Big", "style": "toggle" }, { "id": "size", "title": "Size", "choices": [{ "value": 1 }, { "value": 2 }], "when": "big" }]"#
        let catalog = try BoardCatalog(data: Data(board(options).utf8))
        let board = try XCTUnwrap(catalog.board("t"))
        XCTAssertTrue(board.shows(board.options[0], with: CardOptions()), "no condition: always")
        XCTAssertFalse(board.shows(board.options[1], with: CardOptions()))
        var on = CardOptions()
        on["big"] = .bool(true)
        XCTAssertTrue(board.shows(board.options[1], with: on))
    }

    func testTheBuiltInTypesHaveNoGroup() throws {
        XCTAssertNil(BoardType.custom.group)
        XCTAssertEqual(BoardType.custom.optionsTitle, "Card")
        XCTAssertEqual(BoardType.customCard.optionsTitle, "Card")
        let first = try XCTUnwrap(BoardCatalog.shared.boards.first)
        XCTAssertEqual(try XCTUnwrap(BoardType(rawValue: first.id)).group, first.group)
        XCTAssertEqual(try XCTUnwrap(BoardType(rawValue: first.id)).optionsTitle, first.optionsTitle)
    }
}
