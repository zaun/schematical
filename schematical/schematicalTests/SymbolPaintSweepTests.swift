import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AJ: the canvas symbols drawn through a recording painter, for
/// every catalog part of the kinds whose drawing branches on the part (the
/// transistor, test point, 7-segment display, relay, slide and tactile
/// switch, supply, resistor), every turn and flip, highlighted or not; their
/// reference labels; and each symbol drawn for real through `ScreenPainter`
/// in both palettes.
@MainActor
final class SymbolPaintSweepTests: XCTestCase {
    private func parts(_ kind: PartSymbol) -> [CatalogPart] {
        (TestSupport.allParts + SchematicCanvasRenderTests.supplies).filter { symbol(for: $0) == kind }.sorted { $0.name < $1.name }
    }

    /// The part's chip in every placement its symbol allows.
    private func placements(_ part: CatalogPart) -> [PlacedChip] {
        let base = PartPreview.chip(for: part)
        var chips: [PlacedChip] = []
        for rotation in base.symbol.canRotate ? [0, 90, 180, 270] : [0] {
            for flipped in base.symbol.canFlip ? [false, true] : [false] {
                var chip = base
                chip.rotation = rotation
                chip.isFlipped = flipped
                chips.append(chip)
            }
        }
        return chips
    }

    private func calls(_ draw: (inout RecordingPainter) -> Void) -> [PaintCall] {
        var painter = RecordingPainter()
        draw(&painter)
        return painter.record.calls
    }

    private func usesAccent(_ calls: [PaintCall]) -> Bool {
        calls.contains { $0.ink.role == .accent && $0.ink.opacity == 1 }
    }

    // MARK: Transistor, test point

    func testEveryTransistorDrawsItsSymbolAndName() throws {
        let all = parts(.transistor)
        XCTAssertFalse(all.isEmpty)
        var mosfets = 0, bjts = 0
        for part in all {
            let mosfet = part.transistor?.mosfet ?? false
            if mosfet { mosfets += 1 } else { bjts += 1 }
            for chip in placements(part) {
                let plain = calls { TransistorSymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: false) }
                // A BJT strokes its circle as well as its leads.
                XCTAssertEqual(plain.filter { $0.kind == .stroke }.count, mosfet ? 1 : 2, part.name)
                XCTAssertEqual(plain.filter { $0.kind == .fill }.count, 1, "the arrow, \(part.name)")
                XCTAssertEqual(plain.last?.string, part.name)
                XCTAssertFalse(usesAccent(plain))
                let hovered = calls { TransistorSymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: true) }
                XCTAssertTrue(usesAccent(hovered), part.name)
                XCTAssertTrue(hovered.filter { $0.kind == .stroke }.allSatisfy { $0.lineWidth == 2 })
                let label = TransistorSymbolView.nameLabel(for: chip)
                if chip.rotation == 90 || chip.rotation == 270 { XCTAssertEqual(label.anchor, .leading) }
            }
        }
        XCTAssertGreaterThan(mosfets, 0)
        XCTAssertGreaterThan(bjts, 0)
        XCTAssertTrue(all.contains { $0.transistor?.pType == true } && all.contains { $0.transistor?.pType == false })
    }

    func testATestPointIsALeadAndACircle() throws {
        for part in parts(.testPoint) {
            for chip in placements(part) {
                let plain = calls { TestPointSymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: false) }
                XCTAssertEqual(plain.map(\.kind), [.stroke, .fill, .stroke])
                XCTAssertEqual(plain[1].ink, .paper)
                XCTAssertEqual(plain[2].lineWidth, 1.5)
                let selected = calls { TestPointSymbolView.paint(&$0, chip: chip, isSelected: true, isHovered: false) }
                XCTAssertEqual(selected[2].lineWidth, 2)
                XCTAssertTrue(usesAccent(selected))
                let centre = TestPointSymbolView.circleCentre(of: chip)
                XCTAssertTrue(CGRect(origin: .zero, size: chip.symbolFrameSize).insetBy(dx: -1, dy: -1).contains(centre))
            }
        }
    }

    // MARK: Displays and switches

    func testEverySevenSegmentDisplayLightsEachLED() throws {
        let all = parts(.sevenSegment)
        XCTAssertFalse(all.isEmpty)
        for part in all {
            let chip = PartPreview.chip(for: part)
            let display = try XCTUnwrap(part.display, part.name)
            let dark = calls { SevenSegmentSymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: false, levels: []) }
            XCTAssertFalse(dark.contains { if case .lamp = $0.ink.role { true } else { false } }, part.name)
            let lit = calls {
                SevenSegmentSymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: true,
                                             connectedPins: Set(part.pins.map(\.index)),
                                             levels: Array(repeating: 1, count: display.leds.count))
            }
            let lamps = lit.filter { if case .lamp = $0.ink.role { true } else { false } }.count
            let unlit = lit.filter { $0.ink.role == .unlit }.count
            XCTAssertEqual(lamps, unlit, "each drawn LED lights over its unlit segment, \(part.name)")
            XCTAssertGreaterThan(lamps, 0, part.name)
            XCTAssertTrue(lit.contains { $0.ink == Ink.accent.opacity(0.12) }, "hovered, \(part.name)")
            let designators = Set(part.pins.map(\.designator))
            XCTAssertFalse(lit.contains { call in call.kind == .text && designators.contains(call.string) },
                           "wired pins show no number, \(part.name)")
        }
        // The glass marks: a minus, AM / PM, dot rows.
        let segments = Set(all.flatMap { $0.display?.leds.map(\.segment) ?? [] })
        for mark in ["MINUS", "AM", "PM", "L1", "L2", "DP"] {
            XCTAssertTrue(segments.contains(mark), "a display with \(mark): \(segments.sorted())")
        }
    }

    func testEveryRelayDrawsItsArmsReleasedAndOperated() throws {
        let all = parts(.relay)
        XCTAssertFalse(all.isEmpty)
        for part in all {
            let chip = PartPreview.chip(for: part)
            let released = calls { RelaySymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: false) }
            let operated = calls {
                RelaySymbolView.paint(&$0, chip: chip, isSelected: false, isHovered: true, levels: [1])
            }
            XCTAssertFalse(released.isEmpty, part.name)
            XCTAssertTrue(usesAccent(operated), part.name)
        }
    }

    func testSlideAndTactileSwitchesDrawHovered() throws {
        for kind in [PartSymbol.slideSwitch, .tactileSwitch] {
            let all = parts(kind)
            XCTAssertFalse(all.isEmpty, "\(kind)")
            for part in all {
                let chip = PartPreview.chip(for: part)
                let draw: (inout RecordingPainter, Bool, Set<Int>) -> Void = { painter, hovered, wired in
                    if kind == .slideSwitch {
                        SlideSwitchSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: hovered,
                                                    connectedPins: wired)
                    } else {
                        TactileSwitchSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: hovered,
                                                      connectedPins: wired)
                    }
                }
                let plain = calls { draw(&$0, false, []) }
                let hovered = calls { draw(&$0, true, Set(part.pins.map(\.index))) }
                XCTAssertTrue(hovered.contains { $0.ink == Ink.accent.opacity(0.12) }, part.name)
                XCTAssertFalse(plain.contains { $0.ink == Ink.accent.opacity(0.12) }, part.name)
                XCTAssertLessThan(hovered.filter { $0.kind == .text }.count, plain.filter { $0.kind == .text }.count,
                                  "wired pins drop their numbers, \(part.name)")
            }
        }
    }

    func testSuppliesAndResistorsDrawTheirValue() throws {
        for kind in [PartSymbol.vcc, .ground, .pullUp, .pullDown, .resistor] {
            let all = parts(kind)
            XCTAssertFalse(all.isEmpty, "\(kind)")
            for part in all.prefix(20) {
                for chip in placements(part) {
                    let plain = calls { painter in
                        if kind == .resistor {
                            ResistorSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
                        } else {
                            SupplySymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
                        }
                    }
                    XCTAssertTrue(plain.contains { $0.kind == .text }, part.name)
                    let selected = calls { painter in
                        if kind == .resistor {
                            ResistorSymbolView.paint(&painter, chip: chip, isSelected: true, isHovered: false)
                        } else {
                            SupplySymbolView.paint(&painter, chip: chip, isSelected: true, isHovered: false)
                        }
                    }
                    XCTAssertTrue(usesAccent(selected), part.name)
                }
            }
        }
    }

    // MARK: Reference labels

    func testEveryKindsReferenceLabelSitsOnItsFrame() throws {
        for (kind, part) in SchematicCanvasRenderTestsSupport.partPerSymbol() {
            for chip in placements(part) {
                let anchor = ReferenceLabel.anchor(for: chip)
                if kind == .vcc || kind == .ground {
                    XCTAssertNil(anchor, "\(kind)")
                    XCTAssertNil(ReferenceLabelView.offsetY(for: chip))
                    continue
                }
                let point = try XCTUnwrap(anchor, "\(kind)")
                let frame = CGRect(origin: .zero, size: chip.symbolFrameSize).insetBy(dx: -40, dy: -40)
                XCTAssertTrue(frame.contains(point), "\(kind) \(chip.rotation) \(point)")
                _ = ReferenceLabel.textAnchor(for: chip)
                let drawn = calls { ReferenceLabel.paint(&$0, chip: chip, isHighlighted: true) }
                XCTAssertEqual(drawn.filter { $0.kind == .text }.map(\.string), [chip.reference], "\(kind)")
                XCTAssertTrue(drawn.contains { $0.ink.role == .accent }, "highlighted, \(kind)")
            }
        }
        // A crystal with a GND lead, upside down, keeps its label clear of the lead.
        let crystal = try XCTUnwrap(parts(.crystal).first { CrystalSymbolView.hasGround($0) })
        var turned = PartPreview.chip(for: crystal)
        turned.rotation = 180
        XCTAssertEqual(ReferenceLabel.textAnchor(for: turned), .bottomTrailing)
        let lead = try XCTUnwrap(ReferenceLabel.anchor(for: turned))
        var upright = turned
        upright.rotation = 0
        XCTAssertNotEqual(lead, ReferenceLabel.anchor(for: upright))
        // A gate's label follows the gate's own.
        let gate = try gatePiece()
        XCTAssertEqual(ReferenceLabel.anchor(for: gate), GateSymbolView.referenceLabel(for: gate).point)
        let turnedGate = gate.piece(try XCTUnwrap(gate.part.units.first), origin: .zero, isFlipped: false, rotation: 90)
        XCTAssertEqual(ReferenceLabel.anchor(for: turnedGate), GateSymbolView.referenceLabel(for: turnedGate).point,
                       "turned too")
        // A test point with its pin on top: the label stands beside the circle.
        let point = try XCTUnwrap(parts(.testPoint).first)
        let beside = placements(point).filter { ReferenceLabel.textAnchor(for: $0) == .bottomLeading }
        XCTAssertFalse(beside.isEmpty)
    }

    /// A unit of a gate part, drawn as a gate.
    private func gatePiece() throws -> PlacedChip {
        let part = try TestSupport.part("SN74HC00N")
        let chip = PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: .zero)
        return chip.piece(try XCTUnwrap(part.units.first), origin: .zero, isFlipped: false, rotation: 0)
    }

    // MARK: Drawn for real

    /// Each symbol's view rendered: on screen, highlighted, turned, and in
    /// the export palette; every render inks something.
    func testEverySymbolRendersThroughTheScreenPainter() throws {
        let chips = try SchematicCanvasRenderTestsSupport.partPerSymbol().flatMap { placements($0.value) } + [gatePiece()]
        for chip in chips {
            let kind = chip.symbol
            do {
                for palette in [ScreenPalette.screen, .export] {
                    let size = chip.symbolFrameSize
                    let view = Canvas { context, _ in
                        var painter = ScreenPainter(context, palette: palette)
                        SymbolPaint.paint(&painter, chip: chip, highlighted: palette == .screen)
                        painter.rotate(by: .degrees(0))
                        painter.translateBy(x: 0, y: 0)
                    }
                    .frame(width: max(size.width, 10) + 80, height: max(size.height, 10) + 40)
                    .background(Color.white)
                    let renderer = ImageRenderer(content: view)
                    let image = try XCTUnwrap(renderer.cgImage, "\(kind)")
                    XCTAssertGreaterThan(image.width, 0)
                }
            }
        }
        // The views themselves, as the canvas hosts them.
        let chip = PartPreview.chip(for: try XCTUnwrap(parts(.transistor).first))
        let views: [AnyView] = [
            AnyView(TransistorSymbolView(chip: chip, isSelected: false, isHovered: false)),
            AnyView(ReferenceLabelView(chip: chip, isHighlighted: true)),
        ]
        for view in views { XCTAssertNotNil(ImageRenderer(content: view).cgImage) }
    }

    /// An overlined pin name is drawn with its bar, measured from the text.
    func testAnOverlinedNameDrawsItsBar() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "AS6C1008-55PCN", at: CGPoint(x: 300, y: 300))
        let text = try XCTUnwrap(chip.layout.slots.lazy.map { $0.pin.paintedName(size: 9, weight: .medium, monospaced: true) }
            .first { !$0.overlines.isEmpty })
        func render(_ overlines: [Range<Int>]) throws -> Int {
            var piece = text
            piece.overlines = overlines
            let view = Canvas { context, _ in
                var painter = ScreenPainter(context)
                painter.draw(piece, at: CGPoint(x: 10, y: 20), anchor: .leading)
            }
            .frame(width: 120, height: 40)
            .background(Color.white)
            .environment(\.colorScheme, .light)
            let renderer = ImageRenderer(content: view)
            renderer.scale = 2
            let image = try XCTUnwrap(renderer.cgImage)
            var data = [UInt8](repeating: 0, count: image.width * image.height * 4)
            let context = CGContext(data: &data, width: image.width, height: image.height, bitsPerComponent: 8,
                                    bytesPerRow: image.width * 4, space: CGColorSpaceCreateDeviceRGB(),
                                    bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
            context.draw(image, in: CGRect(x: 0, y: 0, width: image.width, height: image.height))
            return stride(from: 0, to: data.count, by: 4).filter { data[$0] < 128 }.count
        }
        XCTAssertGreaterThan(try render(text.overlines), try render([]), "the bar adds ink")
        XCTAssertEqual(text.substring(text.overlines[0]).count, text.overlines[0].count)
    }
}

/// Paints any chip with its symbol's paint function, as the export scene does.
@MainActor
enum SymbolPaint {
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, highlighted: Bool) {
        var scene = SchematicScene(TestSupport.newDocument())
        scene.chips = [chip]
        scene.paint(into: &painter)
        if highlighted { ReferenceLabel.paint(&painter, chip: chip, isHighlighted: true) }
    }
}

/// `SchematicCanvasRenderTests.partPerSymbol` for other classes.
@MainActor
enum SchematicCanvasRenderTestsSupport {
    static func partPerSymbol() -> [PartSymbol: CatalogPart] { SchematicCanvasRenderTests.partPerSymbol() }
}
