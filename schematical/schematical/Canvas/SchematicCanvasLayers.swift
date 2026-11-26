import AppKit
import SwiftUI

/// A press on the canvas: where it began and is now, and how far it moved
/// (screen points). Differs from the system's drag value only by a scroll
/// during the press (`shifted(by:)`).
struct DragPress {
    var startLocation: CGPoint
    var location: CGPoint
    var translation: CGSize

    init(_ value: DragGesture.Value) {
        startLocation = value.startLocation
        location = value.location
        translation = value.translation
    }

    private init(startLocation: CGPoint, location: CGPoint, translation: CGSize) {
        self.startLocation = startLocation
        self.location = location
        self.translation = translation
    }

    /// The same press with its start moved by `shift` (the canvas scrolled by
    /// that much), the pointer staying where it is.
    func shifted(by shift: CGSize) -> DragPress {
        DragPress(
            startLocation: CGPoint(x: startLocation.x + shift.width, y: startLocation.y + shift.height),
            location: location,
            translation: CGSize(width: translation.width - shift.width, height: translation.height - shift.height)
        )
    }
}

/// The box a Shift-drag on empty canvas draws, between two screen points.
struct SelectionBox: View {
    let from: CGPoint
    let to: CGPoint

    var body: some View {
        let rect = CGRect(x: min(from.x, to.x), y: min(from.y, to.y),
                          width: abs(from.x - to.x), height: abs(from.y - to.y))
        Rectangle()
            .fill(Color.accentColor.opacity(0.12))
            .overlay(Rectangle().strokeBorder(Color.accentColor.opacity(0.8), lineWidth: 1))
            .frame(width: rect.width, height: rect.height)
            .offset(x: rect.minX, y: rect.minY)
            .allowsHitTesting(false)
    }
}

/// Dot grid in world coordinates. Equatable so it only redraws when the visible area changes.
struct GridBackground: View, Equatable {
    let rect: CGRect
    let step: CGFloat

    var body: some View {
        Canvas { context, _ in
            var dots = Path()

            var x = rect.minX
            while x <= rect.maxX {
                var y = rect.minY
                while y <= rect.maxY {
                    dots.addEllipse(in: CGRect(
                        x: x - rect.minX - 0.75,
                        y: y - rect.minY - 0.75,
                        width: 1.5,
                        height: 1.5
                    ))
                    y += step
                }
                x += step
            }

            context.fill(dots, with: .color(Color.secondary.opacity(0.5)))
        }
        .frame(width: rect.width, height: rect.height)
        .offset(x: rect.minX, y: rect.minY)
    }
}

/// The route being drawn from a pin, and the pin it would land on, if any.
struct ConnectionPreview {
    let points: [CGPoint]
    let target: CGPoint?
}

struct RoutedWire: Identifiable {
    let id: UUID
    let points: [CGPoint]
    /// Play mode: the net's value, toggling, floating or shorted. Edit
    /// mode: `.loose` for a wire with a loose end (plan-30).
    var style: WireStyle = .normal
}

extension View {
    /// Sizes a world-coordinate `Canvas` to `rect` (the visible world area)
    /// and places it there. A Canvas only shows what falls inside its own frame,
    /// so without this anything at a negative coordinate, or past the view's
    /// size, is cut off. The Canvas must shift its drawing by `-rect.origin`.
    func worldCanvas(_ rect: CGRect) -> some View {
        frame(width: rect.width, height: rect.height)
            .offset(x: rect.minX, y: rect.minY)
    }
}

/// Moves a Canvas's drawing so world point `rect.origin` is at its top-left;
/// pairs with `worldCanvas(_:)`.
extension GraphicsContext {
    mutating func shiftToWorld(_ rect: CGRect) {
        translateBy(x: -rect.minX, y: -rect.minY)
    }
}

/// A small circle over each pin, shown in Line mode. The connection point under
/// the pointer, or the one a line would land on, is filled green.
struct PinMarkerLayer: View {
    let tips: [CGPoint]
    let highlight: CGPoint?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            context.shiftToWorld(area)
            for tip in tips where tip != highlight {
                let rect = CGRect(x: tip.x - 4, y: tip.y - 4, width: 8, height: 8)
                context.stroke(Path(ellipseIn: rect), with: .color(.accentColor), lineWidth: 1.25)
            }

            if let highlight {
                let rect = CGRect(x: highlight.x - 6, y: highlight.y - 6, width: 12, height: 12)
                context.fill(Path(ellipseIn: rect), with: .color(.green))
                context.stroke(Path(ellipseIn: rect), with: .color(.green.opacity(0.6)), lineWidth: 1)
            }
        }
        .worldCanvas(area)
    }
}

/// Draws text blocks: each line of the words from the top-left, inside an
/// optional frame. A selected block's frame (or, unframed, its outline) is
/// accent; the hovered one gets a soft accent underlay. Not private, so the
/// export can draw it too.
struct NoteLayer: View {
    let notes: [SchematicNote]
    var selectedNoteIDs: Set<UUID> = []
    var hoveredNoteID: UUID?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, notes: notes, selectedNoteIDs: selectedNoteIDs, hoveredNoteID: hoveredNoteID)
        }
        .worldCanvas(area)
    }

    /// Draws the blocks in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, notes: [SchematicNote],
                                           selectedNoteIDs: Set<UUID>, hoveredNoteID: UUID?) {
        for note in notes {
            let rect = note.rect
            let frame = Path(rect)
            let isSelected = selectedNoteIDs.contains(note.id)
            if note.id == hoveredNoteID, !isSelected {
                painter.stroke(frame, with: Ink.accent.opacity(0.35), lineWidth: 5)
            }
            if note.framed {
                painter.fill(frame, with: .paper)
                painter.stroke(frame, with: isSelected ? .accent : .secondary, lineWidth: isSelected ? 2 : 1)
            } else if isSelected {
                painter.stroke(frame, with: .accent,
                               style: StrokeStyle(lineWidth: 1.5, dash: [4, 3]))
            }
            for (index, line) in note.lines.enumerated() where !line.isEmpty {
                painter.draw(PaintedText(line, size: CGFloat(note.size), monospaced: true), at: note.lineOrigin(index),
                             anchor: .topLeading)
            }
        }
    }
}

/// Draws the title block (plan-64-C): a framed table of the document's
/// information. A selected block's frame is accent; the hovered one gets a
/// soft accent underlay. Not private, so the export can draw it too.
struct TitleBlockLayer: View {
    let layout: TitleBlockLayout?
    var isSelected = false
    var isHovered = false
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            guard let layout else { return }
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, layout: layout, isSelected: isSelected, isHovered: isHovered)
        }
        .worldCanvas(area)
    }

    /// Draws the block in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, layout: TitleBlockLayout,
                                           isSelected: Bool = false, isHovered: Bool = false) {
        let frame = Path(layout.rect)
        if isHovered, !isSelected {
            painter.stroke(frame, with: Ink.accent.opacity(0.35), lineWidth: 5)
        }
        painter.fill(frame, with: .paper)
        for (a, b) in layout.separators {
            var line = Path()
            line.move(to: a)
            line.addLine(to: b)
            painter.stroke(line, with: .secondary, lineWidth: 1)
        }
        painter.stroke(frame, with: isSelected ? .accent : .primary, lineWidth: isSelected ? 2 : 1.5)
        for cell in layout.cells {
            painter.draw(PaintedText(cell.label, size: CGFloat(TitleBlockLayout.labelSize), ink: .secondary),
                         at: cell.labelOrigin, anchor: .topLeading)
            for (index, line) in cell.lines.enumerated() where !line.isEmpty {
                painter.draw(PaintedText(line, size: CGFloat(cell.size), weight: cell.weight, monospaced: true),
                             at: CGPoint(x: cell.textOrigin.x, y: cell.textOrigin.y + CGFloat(index) * cell.lineHeight),
                             anchor: .topLeading)
            }
        }
    }
}

/// A junction's dot, accent when it is selected.
struct JunctionDot {
    let point: CGPoint
    var isSelected = false
    /// A loose end where wires branch: gray on screen. Exports leave it false.
    var isLoose = false
}

/// The wires and junction dots, in their own view so that only a change to
/// the wires, junctions, parts (their pins move wires) or the simulation's
/// wire styles draws them again: the layers around it can change without
/// every route being worked out again.
struct SchematicWiresLayer: View {
    let document: SchematicDocument
    let simulation: SimulationController
    let selectedWireIDs: Set<UUID>
    let hoveredWireID: UUID?
    let preview: ConnectionPreview?
    let area: CGRect

    var body: some View {
        WireLayer(
            routes: document.wires.map { wire in
                RoutedWire(
                    id: wire.id,
                    points: document.path(for: wire),
                    style: WireLayer.style(simulated: simulation.wireStyle(wire.id),
                                           loose: document.hasLooseEnd(wire))
                )
            },
            junctions: document.junctions.compactMap { junction in
                let isSelected = document.isSelected(.junction(junction.id))
                guard isSelected || document.showsDot(junction) else { return nil }
                return JunctionDot(point: junction.point, isSelected: isSelected, isLoose: junction.isLoose)
            },
            selectedWireIDs: selectedWireIDs,
            hoveredWireID: hoveredWireID,
            preview: preview,
            area: area
        )
    }
}

/// Every part and its reference, in their own view for the same reason as
/// `SchematicWiresLayer`: a label or wire edit leaves them alone.
struct SchematicPartsLayer: View {
    let document: SchematicDocument
    let simulation: SimulationController
    let hoveredChipID: UUID?
    /// The unit under the pointer, when `hoveredChipID` is drawn as its units.
    let hoveredUnitKey: String?

    /// Whether the pointer is over this part, or this piece of one.
    private func isHovered(_ chip: PlacedChip) -> Bool {
        chip.id == hoveredChipID && (chip.unitKey == nil || chip.unitKey == hoveredUnitKey)
    }

    var body: some View {
        Group {
            let connectedPins = document.connectedPins
            // What is drawn: each part, or each unit of a part drawn as its units (plan-67).
            let pieces = document.chips.flatMap(\.pieces)
            ForEach(pieces, id: \.drawID) { chip in
                Group {
                    switch chip.symbol {
                    case .gate:
                        GateSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            isSibling: document.isSibling(chip)
                        )
                    case .block, .oscillator:
                        ChipBlockView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            timerWarning: simulation.isRunning
                                ? TimerText.warning(simulation.timerSections[chip.id] ?? []) : nil,
                            isSibling: document.isSibling(chip)
                        )
                    case .barrelJack:
                        BarrelJackSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .header:
                        HeaderSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .testPoint:
                        TestPointSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .diode:
                        DiodeSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            level: simulation.lights[chip.id]?.first ?? 0
                        )
                    case .plainDiode:
                        DiodeSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            mark: DiodeMark(part: chip.part)
                        )
                    case .capacitor:
                        CapacitorSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .crystal:
                        CrystalSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .resistor:
                        ResistorSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .inductor:
                        InductorSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .fuse:
                        FuseSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            isBlown: simulation.blownFuses.contains(chip.id)
                        )
                    case .relay:
                        RelaySymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            levels: simulation.lights[chip.id] ?? []
                        )
                    case .buzzer:
                        BuzzerSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            level: simulation.lights[chip.id]?.first ?? 0
                        )
                    case .transistor:
                        TransistorSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .vcc, .ground, .pullUp, .pullDown:
                        SupplySymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip)
                        )
                    case .dipSwitch:
                        DipSwitchSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? []
                        )
                    case .tactileSwitch:
                        TactileSwitchSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? []
                        )
                    case .slideSwitch:
                        SlideSwitchSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? []
                        )
                    case .sevenSegment:
                        SevenSegmentSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            levels: simulation.lights[chip.id] ?? []
                        )
                    case .nixie:
                        NixieSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            levels: simulation.lights[chip.id] ?? []
                        )
                    case .dekatron:
                        DekatronSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            levels: simulation.lights[chip.id] ?? []
                        )
                    case .characterLCD:
                        CharacterLCDSymbolView(
                            chip: chip,
                            isSelected: document.isPicked(chip),
                            isHovered: isHovered(chip),
                            connectedPins: connectedPins[chip.id] ?? [],
                            frame: simulation.lcdFrames[chip.id],
                            backlights: simulation.lights[chip.id] ?? []
                        )
                    }
                }
                .offset(x: chip.viewOrigin.x, y: chip.viewOrigin.y)
            }

            // Every part's reference above it (Display Reference).
            ForEach(pieces, id: \.drawID) { chip in
                if let top = ReferenceLabelView.offsetY(for: chip) {
                    ReferenceLabelView(
                        chip: chip,
                        isHighlighted: document.isPicked(chip) || isHovered(chip)
                    )
                    .offset(x: chip.viewOrigin.x, y: chip.viewOrigin.y + top)
                }
            }
        }
    }
}

/// Draws wires as orthogonal polylines, plus the route being drawn from a pin.
/// The hovered wire gets a soft accent underlay; selected wires are drawn in
/// the accent colour. A wire with a loose end is gray (plan-30), unless play
/// mode gives it another style. Junctions get a solid dot, larger and accent
/// when selected; a loose junction's dot is gray. Not private, so the harness
/// can render it on its own.
struct WireLayer: View {
    let routes: [RoutedWire]
    let junctions: [JunctionDot]
    var selectedWireIDs: Set<UUID> = []
    let hoveredWireID: UUID?
    let preview: ConnectionPreview?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, routes: routes, junctions: junctions, selectedWireIDs: selectedWireIDs,
                       hoveredWireID: hoveredWireID, preview: preview?.points)
        }
        .worldCanvas(area)
    }

    /// Draws the wires in world coordinates. `preview` is the route being drawn.
    /// A route's style: gray `.loose` for a wire with a loose end, unless
    /// the simulation styles it some other way than `.normal`.
    static func style(simulated: WireStyle, loose: Bool) -> WireStyle {
        simulated == .normal && loose ? .loose : simulated
    }

    static func paint<P: SchematicPainter>(_ painter: inout P, routes: [RoutedWire], junctions: [JunctionDot],
                                           selectedWireIDs: Set<UUID>, hoveredWireID: UUID?, preview: [CGPoint]?) {
        let solid = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)
        let underlay = StrokeStyle(lineWidth: 6, lineCap: .round, lineJoin: .round)
        for route in routes {
            if route.id == hoveredWireID {
                painter.stroke(polyline(route.points), with: Ink.accent.opacity(0.35), style: underlay)
            }
            let dashedStyle = StrokeStyle(lineWidth: 1.5, lineCap: .butt, lineJoin: .round, dash: [4, 3])
            let path = polyline(route.points)
            if selectedWireIDs.contains(route.id) {
                painter.stroke(path, with: .accent, style: StrokeStyle(lineWidth: 2.5, lineCap: .round, lineJoin: .round))
                continue
            }
            switch route.style {
            case .normal:
                painter.stroke(path, with: .primary, style: solid)
            case .value(let color):
                painter.stroke(path, with: Ink(role: .color(color)), style: solid)
            case .floating:
                painter.stroke(path, with: .secondary, style: dashedStyle)
            case .shorted:
                painter.stroke(path, with: Ink(role: .color(.red)), style: solid)
            case .toggling:
                painter.stroke(path, with: .secondary, style: solid)
                painter.stroke(path, with: Ink(role: .color(.green)), style: dashedStyle)
            case .loose:
                painter.stroke(path, with: .secondary, style: solid)
            }
        }

        for junction in junctions {
            let r: CGFloat = junction.isSelected ? 5 : 3.5
            let point = junction.point
            let rect = CGRect(x: point.x - r, y: point.y - r, width: 2 * r, height: 2 * r)
            painter.fill(Path(ellipseIn: rect),
                         with: junction.isSelected ? .accent : junction.isLoose ? .secondary : .primary)
        }

        if let preview {
            let dashed = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round, dash: [4, 3])
            painter.stroke(polyline(preview), with: .accent, style: dashed)
        }
    }

    private static func polyline(_ points: [CGPoint]) -> Path {
        var path = Path()
        guard let first = points.first else { return path }
        path.move(to: first)
        for point in points.dropFirst() {
            path.addLine(to: point)
        }
        return path
    }
}

/// The route the Bus tool is drawing. `refused` when releasing would be
/// refused (a joint that is not a T or an L, or too many wires).
struct BusPreview {
    let points: [CGPoint]
    let refused: Bool
}

/// Draws buses: each line 6 wide with square ends, a filled square closing
/// each L corner, and a dot on each T. The hovered bus gets a soft accent
/// underlay and the selected one is drawn in the accent colour. Also draws
/// the route the Bus tool is drawing, dashed. Not private, so the harness can
/// render it on its own.
struct BusLayer: View {
    let buses: [Bus]
    let selectedBusIDs: Set<UUID>
    let hoveredBusID: UUID?
    let preview: BusPreview?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, buses: buses, selectedBusIDs: selectedBusIDs, hoveredBusID: hoveredBusID,
                       preview: preview)
        }
        .worldCanvas(area)
    }

    /// Draws the buses in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, buses: [Bus], selectedBusIDs: Set<UUID>,
                                           hoveredBusID: UUID?, preview: BusPreview?) {
        let width = Bus.lineWidth
        for bus in buses {
            if bus.id == hoveredBusID {
                let underlay = StrokeStyle(lineWidth: width * 2, lineCap: .round)
                for line in bus.lines {
                    painter.stroke(segment(line.a, line.b), with: Ink.accent.opacity(0.35), style: underlay)
                }
            }
            let ink: Ink = selectedBusIDs.contains(bus.id) ? .accent : .primary
            for line in bus.lines {
                painter.stroke(segment(line.a, line.b), with: ink, style: StrokeStyle(lineWidth: width, lineCap: .butt))
            }
            for joint in bus.joints {
                let p = joint.point
                switch joint {
                case .corner:
                    let square = CGRect(x: p.x - width / 2, y: p.y - width / 2, width: width, height: width)
                    painter.fill(Path(square), with: ink)
                case .tee:
                    let dot = CGRect(x: p.x - width, y: p.y - width, width: width * 2, height: width * 2)
                    painter.fill(Path(ellipseIn: dot), with: ink)
                }
            }
        }

        if let preview, let first = preview.points.first {
            var path = Path()
            path.move(to: first)
            for point in preview.points.dropFirst() { path.addLine(to: point) }
            let dashed = StrokeStyle(lineWidth: width, lineCap: .butt, lineJoin: .miter, dash: [8, 6])
            let ink: Ink = preview.refused ? Ink(role: .color(Color.red.opacity(0.6))) : .accent
            painter.stroke(path, with: ink, style: dashed)
        }
    }

    private static func segment(_ a: CGPoint, _ b: CGPoint) -> Path {
        var path = Path()
        path.move(to: a)
        path.addLine(to: b)
        return path
    }
}

/// Draws bus ports: each body (outlined; accent when selected or hovered),
/// a stub per pin out to its tip, and each pin's wire name inside the body
/// next to its pin, turned 90° on a horizontal line where pins are close.
struct PortLayer: View {
    let buses: [Bus]
    let selectedPortID: UUID?
    let hoveredPortID: UUID?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, buses: buses, selectedPortID: selectedPortID, hoveredPortID: hoveredPortID)
        }
        .worldCanvas(area)
    }

    /// Draws the ports in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, buses: [Bus], selectedPortID: UUID?,
                                           hoveredPortID: UUID?) {
        for bus in buses {
            for port in bus.ports {
                guard let body = bus.body(of: port), let line = bus.line(port.line) else { continue }
                let highlighted = port.id == selectedPortID || port.id == hoveredPortID
                let outline: Ink = highlighted ? .accent : .primary
                painter.fill(Path(body), with: .paper)
                painter.stroke(Path(body), with: outline, lineWidth: highlighted ? 2 : 1.5)

                for (index, wire) in bus.pinWires(of: port).enumerated() {
                    let along = bus.pinAlong(port, index: index)
                    guard let edge = bus.point(on: port, along: along, out: BusPort.depth),
                          let tip = bus.point(on: port, along: along, out: BusPort.depth + BusPort.stub),
                          let label = bus.point(on: port, along: along, out: BusPort.depth / 2) else { continue }
                    var stub = Path()
                    stub.move(to: edge)
                    stub.addLine(to: tip)
                    painter.stroke(stub, with: outline, lineWidth: 1.5)

                    // The port keeps its depth: a long name gets smaller text.
                    let size = PaintedText.monospacedSize(10, characters: wire.name.count, fitting: BusPort.depth - 6)
                    let text = PaintedText(wire.name, size: size, weight: .medium, monospaced: true)
                    if line.isHorizontal {
                        var turned = painter
                        turned.translateBy(x: label.x, y: label.y)
                        turned.rotate(by: .degrees(-90))
                        turned.draw(text, at: .zero, anchor: .center)
                    } else {
                        painter.draw(text, at: label, anchor: .center)
                    }
                }
            }
        }
    }
}
