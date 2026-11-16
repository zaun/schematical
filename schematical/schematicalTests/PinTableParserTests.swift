import Foundation
import XCTest
@testable import schematical

/// plan-46-H Phase 1: a pasted pin table becomes pins — its columns guessed,
/// its rows placed or skipped, roles and active-low flags guessed.
@MainActor
final class PinTableParserTests: XCTestCase {
    /// Fixture 1: a TI "Pin Functions" table as Preview copies it
    /// (tab-separated), a 14-pin quad NAND, rows in the datasheet's order
    /// (by name, not by number).
    static let tiTable = [
        "NAME\tNO.\tI/O\tDESCRIPTION",
        "1A\t1\tI\tChannel 1, Input A",
        "1B\t2\tI\tChannel 1, Input B",
        "1Y\t3\tO\tChannel 1, Output Y",
        "2A\t4\tI\tChannel 2, Input A",
        "2B\t5\tI\tChannel 2, Input B",
        "2Y\t6\tO\tChannel 2, Output Y",
        "3A\t9\tI\tChannel 3, Input A",
        "3B\t10\tI\tChannel 3, Input B",
        "3Y\t8\tO\tChannel 3, Output Y",
        "4A\t12\tI\tChannel 4, Input A",
        "4B\t13\tI\tChannel 4, Input B",
        "4Y\t11\tO\tChannel 4, Output Y",
        "GND\t7\t—\tGround",
        "VCC\t14\t—\tPower",
    ].joined(separator: "\n")

    private func pins(_ text: String) -> (pins: [PinDraft], skipped: Int, columns: [PinColumn]) {
        let table = PinTableParser.parse(text)
        let columns = PinTableParser.guessColumns(table)
        let result = PinTableParser.pins(table, columns: columns)
        return (result.pins, result.skipped, columns)
    }

    func testFixture1TIPinFunctions() {
        let table = PinTableParser.parse(Self.tiTable)
        XCTAssertEqual(table.rows.count, 15)
        XCTAssertEqual(table.columnCount, 4)
        let result = pins(Self.tiTable)
        XCTAssertEqual(result.columns, [.name, .number, .type, .ignore])
        XCTAssertEqual(result.skipped, 1, "the header row")
        XCTAssertEqual(result.pins.count, 14)
        XCTAssertEqual(result.pins.map(\.designator), (1...14).map(String.init))
        XCTAssertEqual(result.pins.map(\.name), ["1A", "1B", "1Y", "2A", "2B", "2Y", "GND",
                                                 "3Y", "3A", "3B", "4Y", "4A", "4B", "VCC"])
        XCTAssertEqual(result.pins.map(\.role), ["input", "input", "output", "input", "input", "output", "ground",
                                                 "output", "input", "input", "output", "input", "input", "power"])
        XCTAssertTrue(result.pins.allSatisfy(\.activeLow.isEmpty))
        XCTAssertEqual(PinTableParser.style(result.columns), .numbered)
    }

    /// Fixture 2: one number column per package; the first is the pin number.
    func testFixture2MultiPackage() {
        let text = ["NAME\tD, N\tPW\tI/O", "A\t1\t2\tI", "B\t2\t3\tI", "Y\t3\t4\tO", "GND\t4\t5\t—", "VCC\t5\t1\t—"]
            .joined(separator: "\n")
        let result = pins(text)
        XCTAssertEqual(result.columns, [.name, .number, .ignore, .type])
        XCTAssertEqual(result.pins.map(\.designator), ["1", "2", "3", "4", "5"])
        XCTAssertEqual(result.pins.map(\.name), ["A", "B", "Y", "GND", "VCC"])
        XCTAssertEqual(result.skipped, 1)

        // The user picks the other package's column instead.
        let table = PinTableParser.parse(text)
        let other = PinTableParser.pins(table, columns: [.name, .ignore, .number, .type])
        XCTAssertEqual(other.pins.map(\.name), ["VCC", "A", "B", "Y", "GND"])
    }

    /// Fixture 3: columns aligned with spaces, and one cell naming two pins.
    func testFixture3SpaceAlignedWithTwoPinsInACell() {
        let text = """
        PIN     NAME    TYPE    DESCRIPTION
        1       IN+     I       Non-inverting input
        2       IN-     I       Inverting input
        4, 7    GND     G       Ground (both pins)
        3       OUT     O       Output
        5       NC      NC      Not connected
        6       V+      P       Supply
        """
        let result = pins(text)
        XCTAssertEqual(result.columns, [.number, .name, .type, .ignore])
        XCTAssertEqual(result.pins.map(\.designator), ["1", "2", "3", "4", "5", "6", "7"])
        XCTAssertEqual(result.pins.map(\.name), ["IN+", "IN-", "OUT", "GND", "NC", "V+", "GND"])
        XCTAssertEqual(result.pins.map(\.role), ["input", "input", "output", "ground", "nc", "power", "ground"])
        XCTAssertEqual(result.skipped, 1)
        // The description kept its single spaces.
        XCTAssertEqual(PinTableParser.parse(text).rows[3], ["4, 7", "GND", "G", "Ground (both pins)"])
    }

    /// Fixture 4: a ball table.
    func testFixture4Balls() {
        let result = pins("A1  VDD\nA2  CS#\nB1  GND\nB2  nRESET")
        XCTAssertEqual(result.columns, [.ball, .name])
        XCTAssertEqual(PinTableParser.style(result.columns), .ball)
        XCTAssertEqual(result.pins.map(\.designator), ["A1", "A2", "B1", "B2"])
        XCTAssertEqual(result.pins.map(\.name), ["VDD", "CS#", "GND", "nRESET"])
        XCTAssertEqual(result.pins.map(\.role), ["power", "input", "ground", "input"])
        XCTAssertEqual(result.pins.map(\.activeLow), [[], ["CS#"], [], ["nRESET"]])
        XCTAssertEqual(result.skipped, 0)
    }

    /// Fixture 5: a pasted overbar becomes the flag, and leaves the name.
    func testFixture5Overbar() {
        let result = pins("1\tC\u{0305}S\u{0305}\tI\n2\tSCK\tI\n3\tR\u{0304}D\u{0304}/WR\tI")
        XCTAssertEqual(result.pins.map(\.name), ["CS", "SCK", "RD/WR"])
        XCTAssertEqual(result.pins.map(\.activeLow), [["CS"], [], ["RD"]])
        XCTAssertEqual(result.pins[0].activeLowValue, .bool(true))
        XCTAssertEqual(result.pins[2].activeLowValue, .array([.string("RD")]))
    }

    /// Fixture 6: a section row inside the table is skipped and counted.
    func testFixture6SectionRow() {
        let result = pins("1\tA\tI\n2\tB\tI\n3\tC\tI\n4\tY\tO\nPower pins\n5\tVCC\tP\n6\tGND\tG\n\n   \n")
        XCTAssertEqual(result.pins.map(\.name), ["A", "B", "C", "Y", "VCC", "GND"])
        XCTAssertEqual(result.skipped, 1, "the section row; blank lines are not rows")
        // Below 80 % the column is not taken for pin numbers, and nothing is guessed.
        let junk = pins("1\tA\tI\nPower pins\n2\tVCC\tP\nSee note 1")
        XCTAssertFalse(junk.columns.contains(.number))
        XCTAssertEqual(junk.pins, [])
        XCTAssertEqual(junk.skipped, 4)
    }

    /// Pin names such as A0 … A7 look like ball names; with a number column
    /// they are names.
    func testNamesThatLookLikeBallsAreNamesBesideANumberColumn() {
        let result = pins("1\tA0\tI\n2\tA1\tI\n3\tD0\tI/O\n4\tD1\tI/O")
        XCTAssertEqual(result.columns, [.number, .name, .type])
        XCTAssertEqual(result.pins.map(\.name), ["A0", "A1", "D0", "D1"])
        XCTAssertEqual(result.pins.map(\.role), ["input", "input", "inout", "inout"])
    }

    func testNothingIsInvented() {
        // No designator column: nothing to make pins from.
        let none = pins("VCC\tPower\nGND\tGround")
        XCTAssertEqual(none.pins, [])
        XCTAssertEqual(none.skipped, 2)
        // A row with a number and no name is skipped.
        let table = PinTableParser.parse("1\tA\n2\t\n3\tY")
        let result = PinTableParser.pins(table, columns: [.number, .name])
        XCTAssertEqual(result.pins.map(\.designator), ["1", "3"])
        XCTAssertEqual(result.skipped, 1)
        XCTAssertEqual(PinTableParser.parse("").rows, [])
        // Single spaces when nothing wider separates the cells.
        XCTAssertEqual(PinTableParser.parse("1 A I\n2 Y O").rows, [["1", "A", "I"], ["2", "Y", "O"]])
    }

    /// Each type token and each name rule once.
    func testRoleTable() {
        let types: [(String, String)] = [
            ("I", "input"), ("IN", "input"), ("Input", "input"),
            ("O", "output"), ("OUT", "output"), ("output", "output"),
            ("I/O", "inout"), ("IO", "inout"), ("B", "inout"), ("BIDIR", "inout"),
            ("P", "power"), ("PWR", "power"), ("POWER", "power"), ("S", "power"), ("SUPPLY", "power"),
            ("G", "ground"), ("GND", "ground"), ("GROUND", "ground"),
            ("NC", "nc"), ("—", "nc"), ("-", "nc"),
        ]
        for (type, role) in types {
            XCTAssertEqual(PinRoleGuesser.role(name: "X", type: type), role, type)
            XCTAssertTrue(PinRoleGuesser.isTypeToken(type), type)
        }
        XCTAssertFalse(PinRoleGuesser.isTypeToken("Description"))
        XCTAssertEqual(PinRoleGuesser.role(name: "X", type: nil), "input")
        XCTAssertEqual(PinRoleGuesser.role(name: "X", type: "analog"), "input")

        for name in ["VCC", "VDD", "VDDA", "AVDD", "DVDD", "VIN", "VBAT", "V+", "VS", "VCC2", "VDD_IO", "vcc"] {
            XCTAssertEqual(PinRoleGuesser.role(name: name, type: "—"), "power", name)
        }
        for name in ["GND", "VSS", "AGND", "DGND", "PGND", "V-", "EP", "PAD", "THERMAL PAD", "GND2", "VSS_A"] {
            XCTAssertEqual(PinRoleGuesser.role(name: name, type: "I"), "ground", name)
        }
        for name in ["NC", "N.C.", "DNC"] {
            XCTAssertEqual(PinRoleGuesser.role(name: name, type: "I"), "nc", name)
        }
        // Names that only start like a supply are not one.
        XCTAssertEqual(PinRoleGuesser.role(name: "VSYNC", type: "O"), "output")
        XCTAssertEqual(PinRoleGuesser.role(name: "EPROM", type: "I"), "input")
        XCTAssertEqual(PinRoleGuesser.role(name: "VINA", type: "I"), "input")
    }

    /// Each active-low marker once; names that are not signals are never flagged.
    func testActiveLowTable() {
        for name in ["CS#", "RESET_N", "OE_B", "WR*", "~CE", "nCS", "C\u{0305}S\u{0305}", "O\u{0304}E\u{0304}"] {
            XCTAssertTrue(PinRoleGuesser.isActiveLow(alternative: name), name)
        }
        for name in ["CS", "A0", "nc", "n", "next", "N", "~", "INT", "/CS"] {
            XCTAssertFalse(PinRoleGuesser.isActiveLow(alternative: name), name)
        }
        // Per alternative of the name.
        let mixed = PinRoleGuesser.pin(designator: "5", name: "A0/CS#/nWR", type: "I")
        XCTAssertEqual(mixed.name, "A0/CS#/nWR")
        XCTAssertEqual(mixed.activeLow, ["CS#", "nWR"])
        XCTAssertEqual(mixed.activeLowValue, .array([.string("CS#"), .string("nWR")]))
        // A supply, ground or no-connect pin takes no flag, whatever its name.
        XCTAssertEqual(PinRoleGuesser.pin(designator: "1", name: "GND#", type: "G").activeLow, [])
        XCTAssertEqual(PinRoleGuesser.pin(designator: "1", name: "GND#", type: "G").role, "ground")
        XCTAssertEqual(PinRoleGuesser.pin(designator: "1", name: "VDD_N", type: "P").activeLow, [])
        XCTAssertEqual(PinRoleGuesser.pin(designator: "1", name: "NC", type: "NC").activeLow, [])
        XCTAssertEqual(PinRoleGuesser.cleaned("R\u{0305}E\u{0305}S\u{0305}E\u{0305}T\u{0305}"), "RESET")
    }

    /// The pasted TI table makes a pinout the backend takes: a gate part with
    /// these pins and one gate validates.
    func testThePastedPinoutValidates() throws {
        var draft = PartDraft(newOf: "gate")
        draft.name = "PASTED-\(UUID().uuidString.prefix(8))"
        draft.package = "SOIC"
        draft.pinouts[0].pins = pins(Self.tiTable).pins
        draft.footprint = "SOIC-14"
        draft.kindDevice["gates"] = .array((1...4).map { n in
            .object(["op": .string("nand"), "inputs": .array([.string("\(n)A"), .string("\(n)B")]),
                     "output": .string("\(n)Y")])
        })
        draft.kindDevice["timing"] = .object(["HC": .object(["rise": .number(9), "fall": .number(9),
                                                             "vcc": .array([.number(2), .number(6)])])])
        draft.kindPart["family"] = .string("HC")
        switch PartValidator.shared.validate(draft) {
        case let .success(part): XCTAssertEqual(part.pins.count, 14)
        case let .failure(failure): XCTFail(failure.message)
        }
    }
}
