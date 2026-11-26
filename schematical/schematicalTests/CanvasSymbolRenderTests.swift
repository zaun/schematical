import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AG: the schematic symbols as the canvas shows them. Each view is
/// rendered as the canvas builds it (its defaults left out), plain, selected
/// and hovered, and the three bitmaps must differ; each drawing's own
/// branches (a lit LED, a Zener's bar, a female header, a barrel jack's
/// switch, a crystal's ground lead, a dekatron's glow) are read back through
/// a painter that records what it was asked to draw.
@MainActor
final class CanvasSymbolRenderTests: XCTestCase {
    /// What a paint call drew: each stroke's points and ink, each fill's ink,
    /// each text and its ink.
    nonisolated struct RecordingPainter: SchematicPainter {
        nonisolated final class Box {
            var strokes: [(points: [CGPoint], ink: Ink)] = []
            var fills: [Ink] = []
            var texts: [(string: String, ink: Ink, weight: Font.Weight)] = []
        }

        var box = Box()

        static func points(of path: Path) -> [CGPoint] {
            var found: [CGPoint] = []
            path.forEach { element in
                switch element {
                case let .move(to), let .line(to): found.append(to)
                case let .quadCurve(to, _), let .curve(to, _, _): found.append(to)
                case .closeSubpath: break
                }
            }
            return found
        }

        mutating func fill(_ path: Path, with ink: Ink) { box.fills.append(ink) }
        mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
            box.strokes.append((Self.points(of: path), ink))
        }
        mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
            box.texts.append((text.string, text.ink, text.weight))
        }
        mutating func translateBy(x: CGFloat, y: CGFloat) {}
        mutating func rotate(by angle: Angle) {}
    }

    // MARK: Helpers

    private func parts(_ symbol: PartSymbol) -> [CatalogPart] {
        TestSupport.allParts.filter { schematical.symbol(for: $0) == symbol }.sorted { $0.name < $1.name }
    }

    private func part(_ symbol: PartSymbol, where test: (CatalogPart) -> Bool = { _ in true }) throws -> CatalogPart {
        try XCTUnwrap(parts(symbol).first(where: test), "no \(symbol) part")
    }

    private func chip(_ part: CatalogPart, symbol: PartSymbol? = nil, rotation: Int = 0,
                      gender: String? = nil, farads: Double? = nil) -> PlacedChip {
        PlacedChip(reference: "U1", part: part, symbol: symbol ?? schematical.symbol(for: part),
                   origin: CGPoint(x: 100, y: 100), rotation: rotation, gender: gender, farads: farads)
    }

    /// The view's pixels at 1×, and its size.
    private func render<V: View>(_ view: V) throws -> (size: CGSize, bytes: [UInt8]) {
        let renderer = ImageRenderer(content: view)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage, "nothing rendered")
        let width = image.width, height = image.height
        var bytes = [UInt8](repeating: 0, count: width * height * 4)
        let context = try XCTUnwrap(CGContext(data: &bytes, width: width, height: height, bitsPerComponent: 8,
                                              bytesPerRow: width * 4, space: CGColorSpaceCreateDeviceRGB(),
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        return (CGSize(width: width, height: height), bytes)
    }

    /// Renders `make(selected, hovered)` three ways: each is drawn, at `size`,
    /// and selecting and hovering each change the drawing.
    private func checkStates<V: View>(_ name: String, size: CGSize, _ make: (Bool, Bool) -> V,
                                      file: StaticString = #filePath, line: UInt = #line) throws {
        let plain = try render(make(false, false))
        let selected = try render(make(true, false))
        let hovered = try render(make(false, true))
        XCTAssertEqual(plain.size, CGSize(width: size.width.rounded(.up), height: size.height.rounded(.up)),
                       name, file: file, line: line)
        XCTAssertTrue(plain.bytes.contains { $0 != 0 }, "\(name) draws something", file: file, line: line)
        XCTAssertNotEqual(plain.bytes, selected.bytes, "\(name): selecting shows", file: file, line: line)
        XCTAssertNotEqual(plain.bytes, hovered.bytes, "\(name): hovering shows", file: file, line: line)
    }

    private func blockSize(_ chip: PlacedChip, extra: CGFloat = 0) -> CGSize {
        CGSize(width: chip.layout.size.width + 2 * ChipLayout.labelMargin, height: chip.layout.size.height + extra)
    }

    // MARK: The views as the canvas makes them

    func testEveryTwoLeadSymbolDrawsAndHighlights() throws {
        let jack = chip(try part(.barrelJack))
        try checkStates("barrel jack", size: jack.symbolFrameSize) {
            BarrelJackSymbolView(chip: jack, isSelected: $0, isHovered: $1)
        }
        let buzzer = chip(try part(.buzzer))
        try checkStates("buzzer", size: buzzer.symbolFrameSize) {
            BuzzerSymbolView(chip: buzzer, isSelected: $0, isHovered: $1)
        }
        let capacitor = chip(try part(.capacitor))
        try checkStates("capacitor", size: capacitor.symbolFrameSize) {
            CapacitorSymbolView(chip: capacitor, isSelected: $0, isHovered: $1)
        }
        let crystal = chip(try part(.crystal))
        try checkStates("crystal", size: crystal.symbolFrameSize) {
            CrystalSymbolView(chip: crystal, isSelected: $0, isHovered: $1)
        }
        let led = chip(try part(.diode))
        try checkStates("LED", size: led.symbolFrameSize) {
            DiodeSymbolView(chip: led, isSelected: $0, isHovered: $1)
        }
        let fuse = chip(try part(.fuse))
        try checkStates("fuse", size: fuse.symbolFrameSize) {
            FuseSymbolView(chip: fuse, isSelected: $0, isHovered: $1)
        }
        let inductor = chip(try part(.inductor))
        try checkStates("inductor", size: inductor.symbolFrameSize) {
            InductorSymbolView(chip: inductor, isSelected: $0, isHovered: $1)
        }
        let gatePart = try TestSupport.part("SN74HC00N")
        let unit = try XCTUnwrap(gatePart.units.first)
        let gate = chip(gatePart).piece(unit, origin: CGPoint(x: 100, y: 100), isFlipped: false, rotation: 0)
        XCTAssertEqual(gate.symbol, .gate)
        try checkStates("gate", size: gate.symbolFrameSize) {
            GateSymbolView(chip: gate, isSelected: $0, isHovered: $1)
        }
    }

    func testEveryBoxedSymbolDrawsAndHighlights() throws {
        let block = chip(try TestSupport.part("SN74HC00N"), symbol: .block)
        try checkStates("block", size: blockSize(block)) {
            ChipBlockView(chip: block, isSelected: $0, isHovered: $1)
        }
        let lcd = chip(try part(.characterLCD))
        try checkStates("character LCD", size: blockSize(lcd)) {
            CharacterLCDSymbolView(chip: lcd, isSelected: $0, isHovered: $1)
        }
        let dekatron = chip(try part(.dekatron))
        try checkStates("dekatron", size: blockSize(dekatron)) {
            DekatronSymbolView(chip: dekatron, isSelected: $0, isHovered: $1)
        }
        let dip = chip(try part(.dipSwitch))
        try checkStates("DIP switch", size: blockSize(dip)) {
            DipSwitchSymbolView(chip: dip, isSelected: $0, isHovered: $1)
        }
        let header = chip(try part(.header))
        try checkStates("header", size: blockSize(header, extra: HeaderSymbolView.labelHeight)) {
            HeaderSymbolView(chip: header, isSelected: $0, isHovered: $1)
        }
    }

    func testABlockNamesWhyItIsNotTiming() throws {
        let block = chip(try TestSupport.part("SN74HC00N"), symbol: .block)
        AppDelegate.quitsAfterLastWindowCloses = false
        let probe = ViewProbe(ChipBlockView(chip: block, isSelected: false, isHovered: false,
                                            timerWarning: "no clock"), size: NSSize(width: 300, height: 200))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Not timing: no clock"), "\(probe.labels())")
        let quiet = ViewProbe(ChipBlockView(chip: block, isSelected: false, isHovered: false),
                              size: NSSize(width: 300, height: 200))
        defer { quiet.close() }
        XCTAssertFalse(quiet.has("Not timing"), "\(quiet.labels())")
    }

    // MARK: Their drawings

    func testABlockDrawsItsOscillatorOrJackGlyph() throws {
        // An oscillator: the square-wave glyph in a circle, with its frequency.
        let oscillator = try part(.oscillator)
        var painter = RecordingPainter()
        ChipBlockView.paint(&painter, chip: chip(oscillator), isSelected: false, isHovered: false)
        let hertz = try XCTUnwrap(ChipBlockView.oscillatorFrequency(oscillator))
        XCTAssertTrue(painter.box.texts.contains { $0.string == hertz }, "\(painter.box.texts.map(\.string))")
        XCTAssertTrue(painter.box.strokes.contains { $0.ink == Ink.primary.opacity(0.6) }, "the glyph's circle")
        // Every oscillator has its frequency (the loader keeps defaultHz at least 1).
        XCTAssertTrue(parts(.oscillator).allSatisfy { ChipBlockView.oscillatorFrequency($0) != nil })
        XCTAssertNil(ChipBlockView.oscillatorFrequency(try TestSupport.part("SN74HC00N")), "not an oscillator")
        // A barrel jack drawn as a block (the Part Editor's preview): the jack glyph.
        var jack = RecordingPainter()
        ChipBlockView.paint(&jack, chip: chip(try part(.barrelJack)), isSelected: true, isHovered: false)
        XCTAssertTrue(jack.box.strokes.contains { $0.ink == Ink.accent.opacity(0.7) }, "the jack's outer ring")
        XCTAssertTrue(jack.box.fills.contains(Ink.accent.opacity(0.7)), "its centre pin")
    }

    func testDiodeMarksAndALitLED() throws {
        func diode(_ type: String) -> CatalogPart? {
            parts(.plainDiode).first {
                if case let .diode(kind, _, _, _, _, _) = $0.details { return kind == type }
                return false
            }
        }
        let led = try part(.diode)
        XCTAssertEqual(DiodeMark(part: led), .led)
        let schottky = try XCTUnwrap(diode("schottky"))
        let zener = try XCTUnwrap(diode("zener"))
        let plain = try XCTUnwrap(parts(.plainDiode).first { DiodeMark(part: $0) == .plain })
        XCTAssertEqual(DiodeMark(part: schottky), .schottky)
        XCTAssertEqual(DiodeMark(part: zener), .zener)
        // The cathode bar (the second stroke): a line, an S of 6 points, a Z of 4.
        func bar(_ part: CatalogPart) -> Int {
            var painter = RecordingPainter()
            DiodeSymbolView.paint(&painter, chip: chip(part), isSelected: false, isHovered: false, level: 0,
                                  mark: DiodeMark(part: part))
            // A plain diode names its function under it.
            XCTAssertTrue(painter.box.texts.contains { $0.string == part.function }, part.name)
            return painter.box.strokes[1].points.count
        }
        XCTAssertEqual(bar(plain), 2)
        XCTAssertEqual(bar(schottky), 6)
        XCTAssertEqual(bar(zener), 4)
        // A lit LED glows in its colour; a diode part never lights.
        let lit = chip(led)
        let light = Ink.lamp(DiodeSymbolView.lightName(for: lit))
        var painter = RecordingPainter()
        DiodeSymbolView.paint(&painter, chip: lit, isSelected: false, isHovered: false, level: 0.5)
        XCTAssertTrue(painter.box.fills.contains(light.opacity(0.15)), "the glow")
        XCTAssertTrue(painter.box.fills.contains(light.opacity(0.5)), "the lit triangle")
        XCTAssertTrue(painter.box.strokes.contains { $0.ink == light.opacity(0.5) }, "the lit arrows")
        var dark = RecordingPainter()
        DiodeSymbolView.paint(&dark, chip: chip(plain), isSelected: false, isHovered: false, level: 1, mark: .plain)
        XCTAssertFalse(dark.box.fills.contains { if case .lamp = $0.role { true } else { false } })
    }

    func testEveryLightColourName() {
        let expected: [(String, Color)] = [
            ("Red", .red), ("Green", .green), ("Blue", .blue), ("Amber", Color(red: 1, green: 0.7, blue: 0)),
            ("Yellow", .yellow), ("Orange", .orange), ("White", Color(white: 0.7)),
            ("Warm white", Color(red: 1, green: 0.82, blue: 0.5)), ("Blue-green", Color(red: 0.25, green: 0.9, blue: 0.8)),
            ("Ultraviolet", .secondary),
        ]
        for (name, color) in expected {
            XCTAssertEqual(DiodeSymbolView.lightColor(named: name), color, name)
        }
    }

    func testAJackWithASwitchAndOnItsSide() throws {
        let switched = try part(.barrelJack) { $0.pins.contains { $0.name == "Switch" } }
        let plain = try part(.barrelJack) { !$0.pins.contains { $0.name == "Switch" } }
        func painted(_ chip: PlacedChip) -> RecordingPainter {
            var painter = RecordingPainter()
            BarrelJackSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
            return painter
        }
        // The switch adds its lead pair and its terminal.
        XCTAssertGreaterThan(painted(chip(switched)).box.strokes.count, painted(chip(plain)).box.strokes.count)
        XCTAssertGreaterThan(painted(chip(switched)).box.strokes[2].points.count,
                             painted(chip(plain)).box.strokes[2].points.count)
        // Turned on its side, each pin number stands beside its lead.
        let upright = painted(chip(switched)).box.texts.map(\.string)
        let turned = painted(chip(switched, rotation: 90)).box.texts.map(\.string)
        XCTAssertEqual(upright, turned, "the same numbers")
        XCTAssertEqual(turned.count, 1 + switched.pins.count)
    }

    func testAFemaleHeaderDrawsSockets() throws {
        let header = try part(.header) { $0.genders.count > 1 }
        let female = try XCTUnwrap(header.genders.first { $0.lowercased() == "female" })
        let male = try XCTUnwrap(header.genders.first { $0.lowercased() == "male" })
        let chosen = chip(header, gender: female)
        XCTAssertTrue(HeaderSymbolView.label(for: chosen).hasSuffix(" female"), HeaderSymbolView.label(for: chosen))
        XCTAssertTrue(HeaderSymbolView.label(for: chip(header, gender: male)).hasSuffix(" male"))
        // A header placed with no gender chosen gets the part's first.
        XCTAssertTrue(HeaderSymbolView.label(for: chip(header)).hasSuffix(" " + header.genders[0].lowercased()))
        var sockets = RecordingPainter()
        HeaderSymbolView.paint(&sockets, chip: chosen, isSelected: false, isHovered: false)
        // Each pin's socket is an open U: a 4-point stroke (as is the body's box).
        var pins = RecordingPainter()
        HeaderSymbolView.paint(&pins, chip: chip(header, gender: male), isSelected: false, isHovered: false)
        XCTAssertEqual(sockets.box.strokes.filter { $0.points.count == 4 }.count
                           - pins.box.strokes.filter { $0.points.count == 4 }.count,
                       chosen.layout.slots.count)
    }

    func testCapacitorPlatesAndValue() throws {
        let polarized = try part(.capacitor) { CapacitorSymbolView.isPolarized($0) }
        let flat = try part(.capacitor) { !CapacitorSymbolView.isPolarized($0) }
        func painted(_ chip: PlacedChip) -> RecordingPainter {
            var painter = RecordingPainter()
            CapacitorSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
            return painter
        }
        // A polarized one adds its + (a stroke of its own) by the positive plate.
        XCTAssertEqual(painted(chip(polarized)).box.strokes.count, painted(chip(flat)).box.strokes.count + 1)
        XCTAssertEqual(CapacitorSymbolView.label(for: chip(flat)), flat.function)
        XCTAssertFalse(CapacitorSymbolView.isPolarized(try TestSupport.part("SN74HC00N")), "not a capacitor")
        let generic = chip(flat, farads: 4.7e-6)
        XCTAssertEqual(CapacitorSymbolView.label(for: generic), CapacitorValue.text(4.7e-6))
        XCTAssertTrue(painted(generic).box.texts.contains { $0.string == CapacitorValue.text(4.7e-6) })
    }

    func testACrystalsGroundLeadAndValue() throws {
        let grounded = try part(.crystal) { CrystalSymbolView.hasGround($0) }
        let twoPin = try part(.crystal) { !CrystalSymbolView.hasGround($0) }
        func leads(_ chip: PlacedChip) -> Int {
            var painter = RecordingPainter()
            CrystalSymbolView.paint(&painter, chip: chip, isSelected: false, isHovered: false)
            XCTAssertTrue(painter.box.texts.contains { $0.string == CrystalSymbolView.label(for: chip) })
            return painter.box.strokes[0].points.count
        }
        XCTAssertEqual(leads(chip(grounded)), leads(chip(twoPin)) + 2, "the ground lead")
        // With a ground lead the value sits beside it, or across from it on the side.
        let upright = CrystalSymbolView.valueLabel(for: chip(grounded))
        XCTAssertEqual(upright.anchor, .leading)
        let right = CrystalSymbolView.valueLabel(for: chip(grounded, rotation: 90))
        let left = CrystalSymbolView.valueLabel(for: chip(grounded, rotation: 270))
        XCTAssertNotEqual(right.anchor, left.anchor)
        XCTAssertEqual(CrystalSymbolView.valueLabel(for: chip(twoPin)).point,
                       chip(twoPin).symbolLabel(y: chip(twoPin).unturnedSize.height - 8).point)
    }

    func testADekatronGlowsWhereItRests() throws {
        let tube = chip(try part(.dekatron))
        var resting = RecordingPainter()
        var levels = [Double](repeating: 0, count: 10)
        levels[3] = 0.8
        let wired = Set(tube.layout.slots.prefix(1).map(\.pin.index))
        DekatronSymbolView.paint(&resting, chip: tube, isSelected: false, isHovered: true, connectedPins: wired,
                                 levels: levels)
        let light = Ink.lamp(DiodeSymbolView.lightName(for: tube))
        XCTAssertTrue(resting.box.texts.contains { $0.string == "3" && $0.weight == .bold && $0.ink == light.opacity(0.8) })
        XCTAssertEqual(resting.box.texts.filter { $0.weight == .bold }.count, 1, "one numeral glows")
        XCTAssertTrue(resting.box.fills.contains(Ink.accent.opacity(0.12)), "hovered")
        // A wired pin's number is not drawn.
        let first = try XCTUnwrap(tube.layout.slots.first)
        XCTAssertFalse(resting.box.texts.contains { $0.string == first.pin.designator && $0.ink == .secondary })
        var dark = RecordingPainter()
        DekatronSymbolView.paint(&dark, chip: tube, isSelected: false, isHovered: false, levels: [])
        XCTAssertTrue(dark.box.texts.contains { $0.string == first.pin.designator && $0.ink == .secondary })
        XCTAssertEqual(dark.box.texts.filter { $0.weight == .bold }.count, 0)
    }
}
