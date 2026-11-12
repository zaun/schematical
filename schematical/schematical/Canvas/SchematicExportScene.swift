import SwiftUI

/// A snapshot of a schematic as an export draws it: every text block, bus,
/// port, wire, junction, net label and part, with nothing selected or hovered, no tool
/// preview and no play-mode state. It paints itself through any
/// `SchematicPainter`, in the live canvas's order (text blocks, buses,
/// ports, wires, net labels, parts), minus the grid and pin markers (plan-28-B § 3.2).
struct SchematicScene {
    var chips: [PlacedChip]
    var buses: [Bus]
    /// Every wire, styled `.normal`.
    var routes: [RoutedWire]
    /// Every junction that shows a dot (`showsDot`), none selected.
    var junctions: [JunctionDot]
    var notes: [SchematicNote]
    var netLabels: [NetLabel]
    /// Per chip, the pins with a wire on them (no pin number drawn).
    var connectedPins: [UUID: Set<Int>]

    init(_ document: SchematicDocument) {
        chips = document.chips
        buses = document.buses
        routes = document.wires.map { RoutedWire(id: $0.id, points: document.path(forWire: $0.id)) }
        // A loose end gets a dot only where it branches, and it exports black.
        junctions = document.junctions.filter(document.showsDot).map { JunctionDot(point: $0.point) }
        notes = document.notes
        netLabels = document.netLabels
        connectedPins = document.connectedPins
    }

    var isEmpty: Bool {
        chips.isEmpty && buses.isEmpty && routes.isEmpty && notes.isEmpty && netLabels.isEmpty
    }

    func paint<P: SchematicPainter>(into painter: inout P) {
        NoteLayer.paint(&painter, notes: notes, selectedNoteIDs: [], hoveredNoteID: nil)
        BusLayer.paint(&painter, buses: buses, selectedBusID: nil, hoveredBusID: nil, preview: nil)
        PortLayer.paint(&painter, buses: buses, selectedPortID: nil, hoveredPortID: nil)
        WireLayer.paint(&painter, routes: routes, junctions: junctions, selectedWireIDs: [],
                        hoveredWireID: nil, preview: nil)
        NetLabelLayer.paint(&painter, labels: netLabels, selectedIDs: [], hoveredID: nil)
        for chip in chips {
            var placed = painter
            let wired = connectedPins[chip.id] ?? []
            placed.translateBy(x: chip.viewOrigin.x, y: chip.viewOrigin.y)
            // No `default`: a new symbol must say how it exports.
            switch chip.symbol {
            case .block, .oscillator:
                ChipBlockView.paint(&placed, chip: chip, isSelected: false, isHovered: false, connectedPins: wired)
            case .barrelJack:
                BarrelJackSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false)
            case .header:
                HeaderSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false)
            case .diode:
                DiodeSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, level: 0)
            case .plainDiode:
                DiodeSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, level: 0,
                                      mark: DiodeMark(part: chip.part))
            case .capacitor:
                CapacitorSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false)
            case .vcc, .ground, .pullUp, .pullDown:
                SupplySymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false)
            case .dipSwitch:
                DipSwitchSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, connectedPins: wired)
            case .tactileSwitch:
                TactileSwitchSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, connectedPins: wired)
            case .sevenSegment:
                SevenSegmentSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, connectedPins: wired, levels: [])
            case .characterLCD:
                CharacterLCDSymbolView.paint(&placed, chip: chip, isSelected: false, isHovered: false, connectedPins: wired,
                                             frame: nil, backlights: [])
            }
        }
    }
}

/// Measures where an export's ink lands: the union of every drawn thing's
/// box, in the painter's starting coordinates. Ink the export palette drops
/// (paper, display faces, screen-only colors) does not count. Text boxes are
/// estimated from the character count (plan-28-B § 3.3); the export's
/// padding covers the estimate's error.
struct BoundsPainter: SchematicPainter {
    /// Shared by copies, so a turned or shifted copy adds to the same result.
    /// Nonisolated: a main-actor class's isolated deinit aborts on macOS 15.8
    /// (see `TestSupport.keptDocuments`).
    nonisolated final class Box {
        var rect: CGRect?
    }

    private(set) var box = Box()
    var transform = CGAffineTransform.identity

    init() {}

    /// The ink rectangle; nil when nothing was drawn.
    var bounds: CGRect? { box.rect }

    mutating func fill(_ path: Path, with ink: Ink) {
        guard ExportPalette.alpha(for: ink) != nil else { return }
        add(path.boundingRect)
    }

    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        guard ExportPalette.alpha(for: ink) != nil else { return }
        let rect = path.boundingRect
        guard !rect.isNull else { return }
        add(rect.insetBy(dx: -style.lineWidth / 2, dy: -style.lineWidth / 2))
    }

    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        guard ExportPalette.alpha(for: text.ink) != nil, !text.string.isEmpty else { return }
        let size = Self.textSize(text)
        add(CGRect(x: point.x - size.width * anchor.x, y: point.y - size.height * anchor.y,
                   width: size.width, height: size.height))
    }

    mutating func translateBy(x: CGFloat, y: CGFloat) {
        transform = CGAffineTransform(translationX: x, y: y).concatenating(transform)
    }

    mutating func rotate(by angle: Angle) {
        transform = CGAffineTransform(rotationAngle: angle.radians).concatenating(transform)
    }

    /// The estimated size of a label: 0.6 em per character monospaced,
    /// 0.55 em otherwise; 1.2 em tall.
    static func textSize(_ text: PaintedText) -> CGSize {
        let advance: CGFloat = text.monospaced ? 0.6 : 0.55
        return CGSize(width: CGFloat(text.string.count) * text.size * advance, height: text.size * 1.2)
    }

    /// Adds a rectangle in the current coordinates: its four corners go
    /// through the transform.
    private func add(_ rect: CGRect) {
        guard !rect.isNull else { return }
        let placed = rect.applying(transform)
        box.rect = box.rect.map { $0.union(placed) } ?? placed
    }
}
