import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AJ: the Schematic canvas's layers drawn through a recording
/// painter in each state (hovered, selected, framed or not, each wire
/// style, bus joints, port pins), and each layer view rendered as the canvas
/// hosts it.
@MainActor
final class SchematicLayersPaintTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func calls(_ draw: (inout RecordingPainter) -> Void) -> [PaintCall] {
        var painter = RecordingPainter()
        draw(&painter)
        return painter.record.calls
    }

    private let underlay = Ink.accent.opacity(0.35)
    private let area = CGRect(x: 0, y: 0, width: 1200, height: 900)

    private func rendered(_ view: some View) throws -> CGImage {
        try XCTUnwrap(ImageRenderer(content: view.frame(width: 1200, height: 900)).cgImage)
    }

    func testTextBlocksHoveredSelectedFramedOrNot() {
        let framed = SchematicNote(id: UUID(), text: "One\n\nTwo", origin: p(10, 10))
        var bare = SchematicNote(id: UUID(), text: "Bare", origin: p(200, 10))
        bare.framed = false
        let hovered = calls { NoteLayer.paint(&$0, notes: [framed], selectedNoteIDs: [], hoveredNoteID: framed.id) }
        XCTAssertEqual(hovered.first?.ink, underlay)
        XCTAssertEqual(hovered.filter { $0.kind == .text }.map(\.string), ["One", "Two"], "an empty line draws nothing")
        let selected = calls { NoteLayer.paint(&$0, notes: [framed], selectedNoteIDs: [framed.id], hoveredNoteID: framed.id) }
        XCTAssertFalse(selected.contains { $0.ink == underlay }, "a selected block has no hover underlay")
        XCTAssertTrue(selected.contains { $0.kind == .stroke && $0.ink == .accent && $0.lineWidth == 2 })
        let unframed = calls { NoteLayer.paint(&$0, notes: [bare], selectedNoteIDs: [bare.id], hoveredNoteID: nil) }
        XCTAssertEqual(unframed.filter { $0.kind == .stroke }.map(\.ink), [.accent], "an unframed selected block's outline")
        XCTAssertTrue(calls { NoteLayer.paint(&$0, notes: [bare], selectedNoteIDs: [], hoveredNoteID: nil) }
            .allSatisfy { $0.kind == .text })
    }

    func testTheTitleBlockHoveredAndSelected() throws {
        let doc = TestSupport.newDocument()
        doc.setTitleBlockShown(true)
        let layout = try XCTUnwrap(doc.titleBlockLayout)
        let hovered = calls { TitleBlockLayer.paint(&$0, layout: layout, isHovered: true) }
        XCTAssertEqual(hovered.first?.ink, underlay)
        let selected = calls { TitleBlockLayer.paint(&$0, layout: layout, isSelected: true, isHovered: true) }
        XCTAssertNotEqual(selected.first?.ink, underlay)
        XCTAssertTrue(selected.contains { $0.ink == .accent && $0.lineWidth == 2 })
        XCTAssertNotNil(try rendered(TitleBlockLayer(layout: layout, area: area)))
    }

    func testEveryWireStyleAndJunction() throws {
        let points = [p(10, 10), p(100, 10)]
        let styles: [WireStyle] = [.normal, .value(.orange), .floating, .shorted, .toggling, .loose]
        let routes = styles.map { RoutedWire(id: UUID(), points: points, style: $0) }
        let selected = RoutedWire(id: UUID(), points: points)
        let drawn = calls {
            WireLayer.paint(&$0, routes: routes + [selected], junctions: [
                JunctionDot(point: p(10, 10)), JunctionDot(point: p(20, 10), isSelected: true),
                JunctionDot(point: p(30, 10), isLoose: true),
            ], selectedWireIDs: [selected.id], hoveredWireID: routes[0].id, preview: [p(0, 0), p(0, 50)])
        }
        let strokes = drawn.filter { $0.kind == .stroke }.map(\.ink)
        XCTAssertEqual(strokes.first, underlay, "the hovered wire's underlay first")
        XCTAssertTrue(strokes.contains(Ink(role: .color(.orange))))
        XCTAssertTrue(strokes.contains(Ink(role: .color(.red))), "shorted")
        XCTAssertTrue(strokes.contains(Ink(role: .color(.green))), "toggling")
        XCTAssertTrue(strokes.contains(.accent), "the selected wire")
        XCTAssertEqual(drawn.filter { $0.kind == .fill }.map(\.ink), [.primary, .accent, .secondary])
        XCTAssertEqual(WireLayer.style(simulated: .normal, loose: true), .loose)
        XCTAssertEqual(WireLayer.style(simulated: .shorted, loose: true), .shorted)
        XCTAssertNotNil(try rendered(WireLayer(routes: routes, junctions: [JunctionDot(point: p(10, 10))],
                                               hoveredWireID: nil, preview: nil, area: area)))
    }

    func testBusesHoveredSelectedWithCornersAndTees() throws {
        let doc = TestSupport.newDocument()
        // An L (a corner) and a T off its first line.
        let id = try XCTUnwrap(doc.addBus(candidates: [[p(100, 100), p(500, 100), p(500, 400)]]))
        XCTAssertNotNil(doc.addBus(candidates: [[p(300, 100), p(300, 300)]]))
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertTrue(bus.joints.contains { if case .corner = $0 { true } else { false } }, "\(bus.joints)")
        XCTAssertTrue(bus.joints.contains { if case .tee = $0 { true } else { false } }, "\(bus.joints)")
        let hovered = calls { BusLayer.paint(&$0, buses: [bus], selectedBusIDs: [bus.id], hoveredBusID: bus.id, preview: nil) }
        XCTAssertEqual(hovered.first?.ink, underlay)
        XCTAssertEqual(hovered.filter { $0.kind == .fill }.count, bus.joints.count, "a mark at each joint")
        XCTAssertTrue(hovered.filter { $0.kind == .fill }.allSatisfy { $0.ink == .accent }, "selected")
    }

    func testPortPinsOnHorizontalAndVerticalLines() throws {
        let doc = TestSupport.newDocument()
        let flat = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(600, 100)))
        let upright = try XCTUnwrap(doc.addBus(from: p(800, 100), to: p(800, 600)))
        for (id, at) in [(flat, p(300, 100)), (upright, p(800, 300))] {
            XCTAssertNil(doc.setBusWireCount(id, 3))
            XCTAssertNil(doc.addPort(at: at))
            let bus = try XCTUnwrap(doc.bus(id))
            let port = try XCTUnwrap(bus.ports.first)
            XCTAssertNil(doc.setPortWires(port.id, Set(bus.wires.prefix(2).map(\.id))))
        }
        let buses = doc.buses
        let ports = buses.flatMap(\.ports)
        let drawn = calls { PortLayer.paint(&$0, buses: buses, selectedPortID: ports[0].id, hoveredPortID: ports[1].id) }
        XCTAssertEqual(drawn.filter { $0.kind == .text }.count, 4, "two pin names on each port")
        XCTAssertEqual(drawn.filter { $0.kind == .stroke && $0.ink == .accent }.count, 2 + 4, "both highlighted, with their stubs")
        XCTAssertNotNil(try rendered(PortLayer(buses: buses, selectedPortID: nil, hoveredPortID: nil, area: area)))
    }

    func testPinMarkersAndTheHighlight() throws {
        let tips = [p(100, 100), p(200, 100)]
        XCTAssertNotNil(try rendered(PinMarkerLayer(tips: tips, highlight: tips[0], area: area)))
        XCTAssertNotNil(try rendered(PinMarkerLayer(tips: tips, highlight: nil, area: area)))
    }

    /// The wires layer shows a selected junction's dot, and the parts layer
    /// a part drawn as its gates.
    func testTheDocumentLayersDrawJunctionsAndGates() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 200))
        let c = try TestSupport.place(doc, "GENERIC-LED", at: p(350, 500))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, "K")), .pin(try TestSupport.pin(b, "A"))))
        let path = doc.path(forWire: try XCTUnwrap(doc.wires.first).id)
        let mid = SchematicGrid.snap(p((path[0].x + path[1].x) / 2, (path[0].y + path[1].y) / 2))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(c, "A")), .point(mid)))
        let junction = try XCTUnwrap(doc.junctions.first)
        doc.select(.junction(junction.id))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        SchematicLayersPaintTests.kept.append(simulation)
        XCTAssertNotNil(try rendered(SchematicWiresLayer(document: doc, simulation: simulation,
                                                         selectedWireIDs: [], hoveredWireID: nil, preview: nil, area: area)))

        let gates = try TestSupport.place(doc, "SN74HC00N", at: p(700, 300))
        doc.setPartSplit(gates.id, true)
        let split = try XCTUnwrap(doc.chips.first { $0.id == gates.id })
        XCTAssertTrue(split.isSplit)
        XCTAssertTrue(split.pieces.contains { $0.symbol == .gate })
        doc.select(.chip(gates.id))
        XCTAssertNotNil(try rendered(SchematicPartsLayer(document: doc, simulation: simulation,
                                                         hoveredChipID: gates.id, hoveredUnitKey: split.pieces.first?.unitKey)))
    }

    private static var kept: [SimulationController] = []

    /// The views' defaults: no wired pins, no lights, nothing selected.
    func testTheSymbolViewsDrawWithTheirDefaults() throws {
        let parts = SchematicCanvasRenderTests.partPerSymbol()
        let chip = { (kind: PartSymbol) in PartPreview.chip(for: try XCTUnwrap(parts[kind], "\(kind)")) }
        let views: [AnyView] = [
            AnyView(RelaySymbolView(chip: try chip(.relay), isSelected: false, isHovered: false)),
            AnyView(SevenSegmentSymbolView(chip: try chip(.sevenSegment), isSelected: false, isHovered: false)),
            AnyView(SlideSwitchSymbolView(chip: try chip(.slideSwitch), isSelected: false, isHovered: false)),
            AnyView(TactileSwitchSymbolView(chip: try chip(.tactileSwitch), isSelected: false, isHovered: false)),
            AnyView(NoteLayer(notes: [SchematicNote(id: UUID(), text: "Hi", origin: p(10, 10))], area: area)),
        ]
        for view in views { XCTAssertNotNil(ImageRenderer(content: view).cgImage) }
    }
}
