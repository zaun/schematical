import CoreGraphics
import XCTest
@testable import schematical

/// A part drawn as its units (plan-67-C): where the placements are kept and
/// how the part answers geometry questions from its pieces. An unsplit part
/// answers exactly as before.
@MainActor
final class SplitPartModelTests: XCTestCase {
    private func placed(_ name: String = "SN74HC00N") throws -> PlacedChip {
        try TestSupport.place(TestSupport.newDocument(), name, at: CGPoint(x: 260, y: 345))
    }

    func testSplitLaysUnitsInAColumn() throws {
        var chip = try placed()
        let corner = chip.origin
        chip.split()
        XCTAssertTrue(chip.isSplit)
        XCTAssertEqual(chip.unitPlacements.map(\.key), ["A", "B", "C", "D", "P"])
        // Each gate is 40 tall, the next one a 20 gap below.
        XCTAssertEqual(chip.unitPlacements.map(\.origin),
                       [0, 60, 120, 180, 240].map { CGPoint(x: corner.x, y: corner.y + $0) })
        XCTAssertEqual(chip.unitPlacements.map(\.isFlipped), [false, false, false, false, false])
        XCTAssertEqual(chip.unitPlacements.map(\.rotation), [0, 0, 0, 0, 0])
        XCTAssertEqual(chip.origin, corner)
        XCTAssertEqual(chip.pieces.map(\.symbol), [.gate, .gate, .gate, .gate, .block])
        XCTAssertEqual(chip.pieces.map(\.drawnReference), ["U1-A", "U1-B", "U1-C", "U1-D", "U1-P"])
        // Splitting again changes nothing.
        let once = chip
        chip.split()
        XCTAssertEqual(chip, once)
    }

    func testEveryPinHasATipOnItsPiece() throws {
        var chip = try placed()
        let pins = Set(chip.layout.slots.map(\.pin.index))
        chip.split()
        let slots = chip.layout.slots
        XCTAssertEqual(slots.count, 14)
        XCTAssertEqual(Set(slots.map(\.pin.index)), pins)
        for slot in slots {
            let key = try XCTUnwrap(chip.unitKey(ofPin: slot.pin.index))
            let piece = try XCTUnwrap(chip.piece(forUnit: key))
            let own = try XCTUnwrap(piece.layout.slots.first { $0.pin.index == slot.pin.index })
            XCTAssertEqual(chip.tip(of: slot), piece.tip(of: own), slot.pin.name)
            XCTAssertTrue(piece.hitRect.insetBy(dx: -1, dy: -1).contains(chip.tip(of: slot)), slot.pin.name)
        }
        // No two pins share a tip.
        XCTAssertEqual(Set(slots.map { "\(chip.tip(of: $0))" }).count, 14)
        // The layout's size is the box around the pieces.
        XCTAssertEqual(chip.bodyRect, chip.pieces.map(\.bodyRect).reduce(chip.pieces[0].bodyRect) { $0.union($1) })
    }

    func testSettingOriginMovesEveryUnit() throws {
        var chip = try placed()
        chip.split()
        let before = chip.unitPlacements.map(\.origin)
        chip.origin = CGPoint(x: chip.origin.x + 70, y: chip.origin.y - 30)
        XCTAssertEqual(chip.unitPlacements.map(\.origin), before.map { CGPoint(x: $0.x + 70, y: $0.y - 30) })
    }

    func testJoinPutsTheBlockAtTheUnionCorner() throws {
        var chip = try placed()
        let whole = chip
        chip.split()
        chip.updatePlacement("A") { $0.origin = CGPoint(x: $0.origin.x - 100, y: $0.origin.y - 50) }
        let corner = chip.origin
        XCTAssertEqual(corner, CGPoint(x: whole.origin.x - 100, y: whole.origin.y - 50))
        chip.join()
        XCTAssertFalse(chip.isSplit)
        XCTAssertEqual(chip.origin, corner)
        XCTAssertEqual(chip.layout, whole.layout)
        XCTAssertEqual(chip.pieces, [chip])
    }

    func testHitTests() throws {
        var chip = try placed()
        chip.split()
        let c = try XCTUnwrap(chip.piece(forUnit: "C"))
        XCTAssertEqual(chip.piece(at: CGPoint(x: c.bodyRect.midX, y: c.bodyRect.midY))?.unitKey, "C")
        // The 20 gap between B and C is empty.
        let b = try XCTUnwrap(chip.piece(forUnit: "B"))
        XCTAssertNil(chip.piece(at: CGPoint(x: b.bodyRect.midX, y: b.bodyRect.maxY + 10)))
        let box = b.bodyRect.union(c.bodyRect).insetBy(dx: 5, dy: 5)
        XCTAssertEqual(chip.pieces(in: box).compactMap(\.unitKey), ["B", "C"])
        XCTAssertTrue(chip.hitRect.contains(c.hitRect))
    }

    func testSetPlacementsRefusesWrongKeys() throws {
        let whole = try placed()
        func placement(_ key: String, rotation: Int = 0) -> UnitPlacement {
            UnitPlacement(key: key, origin: CGPoint(x: 100, y: 100), isFlipped: false, rotation: rotation)
        }
        var chip = whole
        XCTAssertFalse(chip.setPlacements(["A", "B", "C", "D"].map { placement($0) }), "a unit is missing")
        XCTAssertFalse(chip.setPlacements(["A", "B", "C", "D", "P", "Z"].map { placement($0) }), "an unknown unit")
        XCTAssertFalse(chip.setPlacements(["A", "A", "C", "D", "P"].map { placement($0) }), "a unit twice")
        XCTAssertEqual(chip, whole)
        // Any order is taken, kept in unit order; a block does not turn, and a turn is a quarter turn.
        XCTAssertTrue(chip.setPlacements([placement("P", rotation: 90), placement("D"), placement("C"),
                                          placement("B", rotation: 45), placement("A", rotation: 270)]))
        XCTAssertEqual(chip.unitPlacements.map(\.key), ["A", "B", "C", "D", "P"])
        XCTAssertEqual(chip.unitPlacements.map(\.rotation), [270, 0, 0, 0, 0])
        // A part with no units takes none.
        var resistor = try placed("GENERIC-RESISTOR")
        XCTAssertFalse(resistor.setPlacements([]))
        resistor.split()
        XCTAssertFalse(resistor.isSplit)
    }

    func testUnsplitPartIsUnchanged() throws {
        let chip = try placed()
        XCTAssertFalse(chip.isSplit)
        XCTAssertEqual(chip.pieces, [chip])
        XCTAssertNil(chip.unitKey)
        XCTAssertEqual(chip.layout, ChipLayout(part: chip.part, symbol: .block))
        XCTAssertEqual(chip.bodyRect, CGRect(origin: chip.origin, size: chip.layout.size))
        XCTAssertEqual(chip.hitRect, chip.bodyRect.insetBy(dx: -ChipLayout.stubLength, dy: 0))
        for slot in chip.layout.slots {
            let x = slot.side == .left ? chip.origin.x - 10 : chip.origin.x + chip.layout.size.width + 10
            XCTAssertEqual(chip.tip(of: slot), CGPoint(x: x, y: chip.origin.y + slot.centerY))
        }
        XCTAssertEqual(chip.piece(at: CGPoint(x: chip.bodyRect.midX, y: chip.bodyRect.midY)), chip)
        var moved = chip
        moved.origin = CGPoint(x: 10, y: 20)
        XCTAssertEqual(moved.origin, CGPoint(x: 10, y: 20))
    }
}
