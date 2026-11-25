import XCTest
@testable import schematical

/// A part's units (plan-67-A): the pieces the Schematic can draw it as. A
/// gate IC's are its gates, keyed A, B, …, then the block P of the pins no
/// gate owns. Every other part has none.
@MainActor
final class PartUnitTests: XCTestCase {
    private func names(_ part: CatalogPart, _ pins: [Int]) -> [String] {
        pins.map { index in part.pins.first { $0.index == index }?.name ?? "?" }
    }

    func testQuadNand() throws {
        let part = try TestSupport.part("SN74HC00N")
        XCTAssertEqual(part.units.map(\.key), ["A", "B", "C", "D", "P"])
        XCTAssertEqual(part.unitViewName, "Gate")
        guard case let .gate(shape) = part.units[0].shape else { return XCTFail("unit A is not a gate") }
        XCTAssertEqual(shape.body, .and)
        XCTAssertEqual(shape.outputs.map(\.inverted), [true])
        XCTAssertFalse(shape.schmitt)
        XCTAssertFalse(shape.openDrain)
        XCTAssertEqual(names(part, part.units[0].pins), ["1A", "1B", "1Y"])
        XCTAssertEqual(names(part, shape.inputs), ["1A", "1B"])
        XCTAssertEqual(part.units[4].shape, .block)
        XCTAssertEqual(Set(names(part, part.units[4].pins)), ["VCC", "GND"])
    }

    func testTwoOutputs() throws {
        let part = try TestSupport.part("CD4068BE")
        XCTAssertEqual(part.units.map(\.key), ["A", "P"])
        guard case let .gate(shape) = part.units[0].shape else { return XCTFail("unit A is not a gate") }
        XCTAssertEqual(shape.inputs.count, 8)
        XCTAssertEqual(shape.outputs.map(\.inverted), [false, true])
        XCTAssertEqual(names(part, shape.outputs.map(\.pin)), ["K", "J"])
    }

    func testSchmittAndOpenDrain() throws {
        let schmitt = try TestSupport.part("SN74HC14N")
        XCTAssertEqual(schmitt.units.count, 7)
        for unit in schmitt.units.dropLast() {
            guard case let .gate(shape) = unit.shape else { return XCTFail("\(unit.key) is not a gate") }
            XCTAssertTrue(shape.schmitt, unit.key)
            XCTAssertEqual(shape.body, .buffer)
            XCTAssertEqual(shape.outputs.map(\.inverted), [true])
        }
        let openDrain = try TestSupport.part("SN74HC266N")
        guard case let .gate(shape) = openDrain.units[0].shape else { return XCTFail("unit A is not a gate") }
        XCTAssertTrue(shape.openDrain)
        XCTAssertEqual(shape.body, .xor)
    }

    /// Every gate part: one gate unit per counted gate, then P, and the units'
    /// pins are exactly the part's drawn pins, each once.
    func testEveryGatePart() throws {
        var checked = 0
        for part in TestSupport.allParts where part.kind == PART_GATE {
            checked += 1
            guard case let .logic(_, count) = part.details else { return XCTFail("\(part.name) has no logic details") }
            XCTAssertEqual(part.units.count - 1, count, part.name)
            XCTAssertEqual(part.units.last?.key, PartUnit.blockKey, part.name)
            XCTAssertEqual(part.units.last?.shape, .block, part.name)
            let all = part.units.flatMap(\.pins)
            XCTAssertEqual(all.count, Set(all).count, "\(part.name): a pin is in two units")
            XCTAssertEqual(Set(all), Set(part.pins.filter { !$0.isHidden }.map(\.index)), part.name)
            XCTAssertEqual(Set(part.units.map(\.key)).count, part.units.count, part.name)
        }
        // The drawer's default filter lists 2158 of the 3564 gate parts.
        XCTAssertGreaterThan(checked, 2000)
    }

    func testOtherPartsHaveNone() throws {
        let part = try TestSupport.part("SN74HC74N")
        XCTAssertTrue(part.units.isEmpty)
        XCTAssertNil(part.unitViewName)
    }

    func testGateKeysSkipP() {
        XCTAssertEqual((0..<17).map(PartUnit.key(forGate:)).joined(), "ABCDEFGHIJKLMNOQR")
    }
}
