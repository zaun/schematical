import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// The gate symbol (plan-67-B): one unit of a part as its own piece, with a
/// layout of only its pins, drawn with the standard logic symbol, at every
/// flip and turn.
@MainActor
final class GateSymbolTests: XCTestCase {
    /// Keeps where every stroked line starts and ends, and every text.
    private nonisolated struct RecordingPainter: SchematicPainter {
        /// Nonisolated: a main-actor class's isolated deinit aborts on macOS
        /// 15.8 (see `BoundsPainter.Box`).
        nonisolated final class Box {
            var ends: [CGPoint] = []
            var texts: [String] = []
        }

        var box = Box()

        mutating func fill(_ path: Path, with ink: Ink) {}

        mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
            path.forEach { element in
                switch element {
                case let .move(to): box.ends.append(to)
                case let .line(to): box.ends.append(to)
                case let .quadCurve(to, _): box.ends.append(to)
                case let .curve(to, _, _): box.ends.append(to)
                case .closeSubpath: break
                }
            }
        }

        mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) { box.texts.append(text.string) }
        mutating func translateBy(x: CGFloat, y: CGFloat) {}
        mutating func rotate(by angle: Angle) {}
    }

    /// The first unit of `name` as a piece at (200, 200), reference U1.
    private func piece(_ name: String, unit key: String = "A", flipped: Bool = false, rotation: Int = 0) throws -> PlacedChip {
        let part = try TestSupport.part(name)
        let chip = PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: CGPoint(x: 200, y: 200))
        let unit = try XCTUnwrap(part.units.first { $0.key == key }, "\(name) has no unit \(key)")
        return chip.piece(unit, origin: CGPoint(x: 200, y: 200), isFlipped: flipped, rotation: rotation)
    }

    private static let samples = ["SN74HC00N", "SN74HC04N", "CD4068BE", "SN74ALS133N"]

    func testLayout() throws {
        let sizes = [CGSize(width: 60, height: 40), CGSize(width: 60, height: 40), CGSize(width: 60, height: 160),
                     CGSize(width: 60, height: 260)]
        for (name, size) in zip(Self.samples, sizes) {
            XCTAssertEqual(try piece(name).layout.size, size, name)
        }
        let nand = try piece("SN74HC00N").layout.slots
        XCTAssertEqual(nand.map(\.pin.name), ["1A", "1B", "1Y"])
        XCTAssertEqual(nand.map(\.side), [.left, .left, .right])
        XCTAssertEqual(nand.map(\.centerY), [10, 30, 20])
        let inverter = try piece("SN74HC04N").layout.slots
        XCTAssertEqual(inverter.map(\.centerY), [20, 20])
        let eight = try piece("CD4068BE").layout.slots
        XCTAssertEqual(eight.filter { $0.side == .left }.map(\.centerY), [10, 30, 50, 70, 90, 110, 130, 150])
        // The true output (K) above the inverted one (J).
        XCTAssertEqual(eight.filter { $0.side == .right }.map(\.pin.name), ["K", "J"])
        XCTAssertEqual(eight.filter { $0.side == .right }.map(\.centerY), [70, 90])
        let thirteen = try piece("SN74ALS133N").layout.slots
        XCTAssertEqual(thirteen.filter { $0.side == .left }.count, 13)
        XCTAssertEqual(thirteen.first?.centerY, 10)
        XCTAssertEqual(thirteen.last?.centerY, 130)
    }

    func testPieceIdentity() throws {
        let gate = try piece("SN74HC00N")
        XCTAssertEqual(gate.unitKey, "A")
        XCTAssertEqual(gate.drawnReference, "U1-A")
        XCTAssertEqual(gate.reference, "U1")
        XCTAssertEqual(gate.symbol, .gate)
        XCTAssertTrue(gate.symbol.canRotate)
        XCTAssertTrue(gate.symbol.canFlip)

        let block = try piece("SN74HC00N", unit: "P", rotation: 90)
        XCTAssertEqual(block.symbol, .block)
        XCTAssertEqual(block.rotation, 0, "a block does not turn")
        XCTAssertEqual(Set(block.layout.slots.map(\.pin.name)), ["VCC", "GND"])
        XCTAssertEqual(block.drawnReference, "U1-P")

        // A whole part is unchanged: no unit, its own reference.
        let part = try TestSupport.part("SN74HC00N")
        let whole = PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: .zero)
        XCTAssertNil(whole.unitKey)
        XCTAssertEqual(whole.drawnReference, "U1")
        XCTAssertEqual(whole.symbol, .block)
    }

    func testTipsOnGridAtEveryTurn() throws {
        for name in Self.samples {
            for flipped in [false, true] {
                for degrees in [0, 90, 180, 270] {
                    let chip = try piece(name, flipped: flipped, rotation: degrees)
                    XCTAssertEqual(chip.rotation, degrees)
                    for slot in chip.layout.slots {
                        let tip = chip.tip(of: slot)
                        XCTAssertEqual(tip.x.truncatingRemainder(dividingBy: 10), 0, "\(name) \(flipped) \(degrees) \(slot.pin.name)")
                        XCTAssertEqual(tip.y.truncatingRemainder(dividingBy: 10), 0, "\(name) \(flipped) \(degrees) \(slot.pin.name)")
                    }
                    let tips = chip.layout.slots.map { chip.tip(of: $0) }
                    XCTAssertEqual(Set(tips.map { "\($0.x),\($0.y)" }).count, tips.count, "\(name): two pins share a tip")
                }
            }
        }
    }

    func testLeadsEndOnTheirPins() throws {
        for name in Self.samples + ["SN74HC86N", "SN74HC32N", "SN74HC14N", "SN74HC266N", "CD4041UBE"] {
            for flipped in [false, true] {
                for degrees in [0, 90, 180, 270] {
                    let chip = try piece(name, flipped: flipped, rotation: degrees)
                    var painter = RecordingPainter()
                    GateSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
                    for slot in chip.layout.slots {
                        let tip = chip.tip(of: slot)
                        let want = CGPoint(x: tip.x - chip.viewOrigin.x, y: tip.y - chip.viewOrigin.y)
                        XCTAssertTrue(painter.box.ends.contains { abs($0.x - want.x) < 0.01 && abs($0.y - want.y) < 0.01 },
                                      "\(name) flipped \(flipped) at \(degrees): no lead ends on \(slot.pin.name) \(want)")
                    }
                    // Every pin's number is drawn.
                    XCTAssertEqual(painter.box.texts.sorted(), chip.layout.slots.map(\.pin.designator).sorted(), name)
                }
            }
        }
    }

    func testInkStaysInTheFrame() throws {
        for name in Self.samples + ["SN74HC86N", "SN74HC14N", "SN74HC266N", "CD4041UBE"] {
            for flipped in [false, true] {
                for degrees in [0, 90, 180, 270] {
                    let chip = try piece(name, flipped: flipped, rotation: degrees)
                    var painter = BoundsPainter()
                    GateSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
                    let ink = try XCTUnwrap(painter.bounds, name)
                    let frame = CGRect(origin: .zero, size: chip.symbolFrameSize).insetBy(dx: -12, dy: -12)
                    XCTAssertTrue(frame.contains(ink), "\(name) flipped \(flipped) at \(degrees): ink \(ink) leaves \(frame)")
                }
            }
        }
    }

    /// Every distinct gate the catalog holds draws something, and every lead
    /// reaches its pin.
    func testEveryShapeDraws() throws {
        var seen: Set<GateShapeKey> = []
        for part in TestSupport.allParts where part.kind == PART_GATE {
            for unit in part.units {
                guard case let .gate(shape) = unit.shape else { continue }
                let key = GateShapeKey(body: shape.body, inputs: shape.inputs.count,
                                       inverted: shape.outputs.map(\.inverted), schmitt: shape.schmitt,
                                       openDrain: shape.openDrain)
                guard seen.insert(key).inserted else { continue }
                let whole = PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: .zero)
                let chip = whole.piece(unit, origin: CGPoint(x: 100, y: 100), isFlipped: false, rotation: 0)
                var bounds = BoundsPainter()
                GateSymbolView.paint(&bounds, chip: chip, isSelected: false, isHovered: false)
                XCTAssertNotNil(bounds.bounds, "\(part.name) \(unit.key) draws nothing")
                var painter = RecordingPainter()
                GateSymbolView.paint(&painter, chip: chip, isSelected: true, isHovered: true, isSibling: true)
                for slot in chip.layout.slots {
                    let tip = chip.tip(of: slot)
                    let want = CGPoint(x: tip.x - chip.viewOrigin.x, y: tip.y - chip.viewOrigin.y)
                    XCTAssertTrue(painter.box.ends.contains { abs($0.x - want.x) < 0.01 && abs($0.y - want.y) < 0.01 },
                                  "\(part.name) \(unit.key): no lead ends on \(slot.pin.name)")
                }
            }
        }
        // AND, OR, XOR and buffer bodies, plain and inverted, at least.
        XCTAssertGreaterThanOrEqual(seen.count, 8)
        XCTAssertEqual(Set(seen.map(\.body)), [.and, .or, .xor, .buffer])
    }

    private struct GateShapeKey: Hashable {
        let body: GateShape.Body
        let inputs: Int
        let inverted: [Bool]
        let schmitt: Bool
        let openDrain: Bool
    }

    func testReferenceAboveTheBody() throws {
        let chip = try piece("SN74HC00N")
        let anchor = try XCTUnwrap(ReferenceLabel.anchor(for: chip))
        XCTAssertLessThan(anchor.y, 0)
        XCTAssertEqual(ReferenceLabel.textAnchor(for: chip), .bottom)
        for degrees in [0, 90, 180, 270] {
            let turned = try piece("SN74HC00N", rotation: degrees)
            let spot = try XCTUnwrap(ReferenceLabel.anchor(for: turned))
            XCTAssertGreaterThanOrEqual(spot.x, 0, "\(degrees)")
            XCTAssertLessThanOrEqual(spot.x, turned.symbolFrameSize.width, "\(degrees)")
        }
        // On its side the leads run up and down, so the label stands beside the body.
        XCTAssertEqual(ReferenceLabel.textAnchor(for: try piece("SN74HC00N", rotation: 90)), .bottomLeading)
        // A wide gate's label sits above its body, which is shorter than its frame.
        let wide = try piece("SN74ALS133N")
        let wideAnchor = try XCTUnwrap(ReferenceLabel.anchor(for: wide))
        XCTAssertEqual(wideAnchor.y, 130 - 30 - 3)
    }

    /// The block of leftover pins draws its pins and part number, not the function.
    func testLeftoverBlockHasNoFunctionWords() throws {
        let block = try piece("SN74HC00N", unit: "P")
        var painter = RecordingPainter()
        ChipBlockView.paint(&painter, chip: block, isSelected: false, isHovered: false, isSibling: true)
        XCTAssertTrue(painter.box.texts.contains("SN74HC00N"))
        XCTAssertFalse(painter.box.texts.contains("NAND"))
        XCTAssertTrue(painter.box.texts.contains("VCC"))
    }
}
