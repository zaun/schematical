import CoreGraphics
import XCTest
@testable import schematical

/// The value types plan-68-B moved out of `SchematicModel.swift`: the lines
/// the rest of the suite did not reach (plan-68-C).
@MainActor
final class SchematicValueTypesTests: XCTestCase {
    // MARK: SchematicTool

    func testSupplyNamesPerPowerTool() {
        XCTAssertEqual(SchematicTool.vcc.supplyName(voltage: "3V3"), "3V3")
        XCTAssertEqual(SchematicTool.ground.supplyName(voltage: "3V3"), "GND")
        XCTAssertEqual(SchematicTool.pullUp.supplyName(voltage: "5V"), "PULLUP-5V")
        XCTAssertEqual(SchematicTool.pullDown.supplyName(voltage: "5V"), "PULLDOWN")
        XCTAssertNil(SchematicTool.line.supplyName(voltage: "5V"))
    }

    // MARK: ChipLayout

    func testAPinSlotIsIdentifiedByItsPinIndex() throws {
        let part = try TestSupport.part("SN74HC00N")
        let layout = ChipLayout(part: part, symbol: .block)
        XCTAssertEqual(layout.slots.map(\.id), layout.slots.map(\.pin.index))
    }

    /// A dekatron: the ten cathodes down the left, then the anode and the two
    /// guides on the right, the rest below them; the face sets the height.
    func testDekatronLayout() throws {
        let part = try TestSupport.part("GC10B")
        let layout = ChipLayout(part: part, symbol: .dekatron)
        let info = try XCTUnwrap(part.dekatron)
        let left = layout.slots.filter { $0.side == .left }.sorted { $0.centerY < $1.centerY }.map(\.pin.index)
        XCTAssertEqual(left, info.cathodePins.filter { idx in part.pins.contains { $0.index == idx && !$0.isHidden } })
        let right = layout.slots.filter { $0.side == .right }.sorted { $0.centerY < $1.centerY }.map(\.pin.index)
        XCTAssertEqual(Array(right.prefix(1 + info.guidePins.count)), [info.anodePin] + info.guidePins)
        XCTAssertGreaterThanOrEqual(layout.size.width, ChipLayout.bodyWidth)
        XCTAssertGreaterThanOrEqual(layout.size.height, ChipLayout.headerHeight + ChipLayout.dekatronFaceSize.height)
        // Without dekatron facts the part is a plain block.
        let plain = try TestSupport.part("SN74HC00N")
        XCTAssertEqual(ChipLayout(part: plain, symbol: .dekatron), ChipLayout(pins: plain.pins.filter { !$0.isHidden }))
    }

    /// A slide-switch layout keeps a row on the right for every pin that is
    /// not a pole's contact.
    func testSlideSwitchPlacesOtherPinsBelowThePoles() throws {
        let part = try TestSupport.part("SN74HC00N")
        let pins = part.pins.filter { !$0.isHidden }
        let layout = ChipLayout(part: part, symbol: .slideSwitch)
        XCTAssertEqual(Set(layout.slots.map(\.pin.index)), Set(pins.map(\.index)))
        XCTAssertTrue(layout.slots.allSatisfy { $0.side == .right })
        let rows = layout.slots.map(\.centerY).sorted()
        XCTAssertEqual(rows.count, pins.count)
        XCTAssertEqual(Set(rows).count, pins.count, "each other pin has its own row")
    }

    /// Every own-symbol layout falls back to the block layout when the part
    /// does not have the terminals the symbol draws (the contract each
    /// layout's documentation states).
    func testOwnSymbolsFallBackToTheBlockWithoutTheirTerminals() throws {
        let part = try TestSupport.part("SN74HC00N")
        let block = ChipLayout(pins: part.pins.filter { !$0.isHidden })
        for symbol: PartSymbol in [.capacitor, .resistor, .crystal, .relay, .buzzer, .transistor, .diode, .plainDiode] {
            XCTAssertEqual(ChipLayout(part: part, symbol: symbol), block, "\(symbol)")
        }
    }

    // MARK: PlacedChip

    func testSwitchPositionUnderAPoint() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try TestSupport.firstDip().name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(chip.symbol, .dipSwitch)
        for position in 1...chip.switchesOn.count {
            let lever = ChipLayout.switchLeverRect(position: position, bodyWidth: chip.layout.size.width)
            let point = CGPoint(x: chip.origin.x + lever.midX, y: chip.origin.y + lever.midY)
            XCTAssertEqual(chip.switchPosition(at: point), position)
        }
        XCTAssertNil(chip.switchPosition(at: CGPoint(x: chip.origin.x - 50, y: chip.origin.y - 50)))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 200))
        XCTAssertNil(led.switchPosition(at: led.origin), "only a DIP switch has positions")
    }

    // MARK: OrthogonalRoute

    /// A diagonal step in the middle of a stored route goes across first.
    func testRefitGoesAcrossOnAnInteriorDiagonal() {
        let path = [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 20, y: 10), CGPoint(x: 30, y: 10)]
        let fitted = OrthogonalRoute.refit(path, start: path[0], end: path[3])
        // The stored corner at (10, 0) stays: refit keeps the bends it was given.
        XCTAssertEqual(fitted, [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 20, y: 0),
                                CGPoint(x: 20, y: 10), CGPoint(x: 30, y: 10)])
    }

    func testMoveNode() {
        let path = [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)]
        XCTAssertEqual(OrthogonalRoute.moveNode(path, at: 0, to: CGPoint(x: 5, y: 5)), path, "an end does not move")
        XCTAssertEqual(OrthogonalRoute.moveNode(path, at: 3, to: CGPoint(x: 5, y: 5)), path)
        XCTAssertEqual(OrthogonalRoute.moveNode(path, at: 1, to: CGPoint(x: 10, y: 10)),
                       [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)],
                       "a node moved onto its neighbour merges with it")
    }

    func testMoveSegment() {
        let path = [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)]
        XCTAssertEqual(OrthogonalRoute.moveSegment(path, at: -1, by: CGSize(width: 5, height: 0)), path)
        XCTAssertEqual(OrthogonalRoute.moveSegment(path, at: 3, by: CGSize(width: 5, height: 0)), path)
        XCTAssertEqual(OrthogonalRoute.moveSegment(path, at: 1, by: CGSize(width: 5, height: 0)),
                       [CGPoint(x: 0, y: 0), CGPoint(x: 15, y: 0), CGPoint(x: 15, y: 10), CGPoint(x: 20, y: 10)])
        // The first segment: its start stays, its far end moves.
        XCTAssertEqual(OrthogonalRoute.moveSegment(path, at: 0, by: CGSize(width: 0, height: 4)),
                       [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 4), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)])
        // The last segment: its end stays.
        XCTAssertEqual(OrthogonalRoute.moveSegment(path, at: 2, by: CGSize(width: 0, height: 4)),
                       [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 14), CGPoint(x: 20, y: 10)])
    }
}
