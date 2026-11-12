import AppKit
import CoreGraphics
import CoreText
import Foundation
import SwiftUI

/// File > Export > Gerber PCB: the board as Gerber X2 files, one per
/// layer, zipped the way fabs take them. See `docs/export.md` § Gerber.
///
/// Everything is drawn as the PCB page draws it: the same pads, traces,
/// vias, holes, Planes (with their clearances) and silkscreen marks. The
/// holes go in Excellon drill files alongside (`DrillExport`).
@MainActor
enum GerberExport {
    /// One file of the set: its name in the zip and its bytes.
    struct File: Equatable {
        let name: String
        let data: Data

        var text: String { String(decoding: data, as: UTF8.self) }
    }

    /// The silkscreen's line width for body outlines (mm), as on screen.
    static let outlineWidth = 0.15
    /// The pin-1 dot's diameter (mm).
    static let pin1Diameter = 0.4
    /// A footprint reference's font size (mm), as on screen at full zoom.
    static let referenceSize = 1.0
    /// The board outline's line width in the profile file (mm).
    static let profileWidth = 0.1

    /// Every layer file of the board, names starting `base`. Copper Top,
    /// inner layers and Bottom first, then solder mask, paste, silkscreen
    /// and the outline, then the drill files: plated holes, and non-plated
    /// ones when the board has any. A 1-layer board has no bottom files.
    static func files(for document: SchematicDocument, base: String,
                      rules: DesignRules = .current()) -> [File] {
        let board = document.board
        let copper = document.connectivity()
        let top = Double(board.outline.map(\.y).max() ?? CGFloat(board.height))
        let layers = board.copperLayers
        let sides: [BoardSide] = layers.count > 1 ? [.top, .bottom] : [.top]
        let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "1"
        func writer(_ function: String, polarity: String = "Positive") -> GerberWriter {
            GerberWriter(attributes: ["GenerationSoftware,jgz,Schematical,\(version)", "FileFunction,\(function)",
                                      "FilePolarity,\(polarity)", "SameCoordinates,Original"],
                         top: top)
        }

        var files: [File] = []
        for (index, layer) in layers.enumerated() {
            let position: String
            let suffix: String
            switch layer {
            case .top: (position, suffix) = ("Top", "GTL")
            case .bottom: (position, suffix) = ("Bot", "GBL")
            case .inner: (position, suffix) = ("Inr", "G\(index)")
            }
            var file = writer("Copper,L\(index + 1),\(position)")
            drawCopper(on: layer, board: board, document: document, copper: copper, rules: rules, into: &file)
            files.append(File(name: "\(base).\(suffix)", data: file.data()))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            let name = side == .top ? "Top" : "Bot"
            var mask = writer("Soldermask,\(name)", polarity: "Negative")
            drawMask(on: side, copper: copper, expansion: rules.solderMaskExpansion, into: &mask)
            files.append(File(name: "\(base).G\(s)S", data: mask.data()))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            var paste = writer("Paste,\(side == .top ? "Top" : "Bot")")
            for entry in copper.pads where entry.chip >= 0 && entry.pad.drill == 0 && entry.side == side {
                paste.fill(shape(of: entry.pad))
            }
            files.append(File(name: "\(base).G\(s)P", data: paste.data()))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            var silk = writer("Legend,\(side == .top ? "Top" : "Bot")")
            drawSilkscreen(on: side, document: document, into: &silk)
            files.append(File(name: "\(base).G\(s)O", data: silk.data()))
        }
        var profile = writer("Profile,NP")
        profile.stroke(board.outline, width: profileWidth, closed: true)
        files.append(File(name: "\(base).GKO", data: profile.data()))
        let holes = DrillExport.holes(copper)
        let software = "Schematical,\(version)"
        files.append(File(name: "\(base)-PTH.drl",
                          data: DrillExport.file(holes.plated, plated: true, layers: layers.count, top: top,
                                                 software: software)))
        if !holes.bare.isEmpty {
            files.append(File(name: "\(base)-NPTH.drl",
                              data: DrillExport.file(holes.bare, plated: false, layers: layers.count, top: top,
                                                     software: software)))
        }
        return files
    }

    /// The set as one zip.
    static func zip(for document: SchematicDocument, base: String, rules: DesignRules = .current()) -> Data {
        ZipArchive.stored(files(for: document, base: base, rules: rules).map { ($0.name, $0.data) })
    }

    // MARK: Layers

    /// One copper layer: a Plane's copper (the board less its clearances),
    /// then traces on the layer, the surface-mount pads and card fingers of
    /// its side, and every plated through-hole pad and via.
    private static func drawCopper(on layer: CopperLayer, board: BoardSettings, document: SchematicDocument,
                                   copper: BoardConnectivity, rules: DesignRules, into file: inout GerberWriter) {
        let style = board.style(of: layer)
        if style.type == .plane {
            let clearance = style.clearance(rules: rules), edge = rules.minCopperToEdge
            file.region(board.outline)
            file.setDark(false)
            if edge > 0 { file.stroke(board.outline, width: 2 * edge, closed: true) }
            let cleared = copper.planeClearances(on: layer)
            for p in cleared.pads {
                let pad = copper.pads[p].pad
                if pad.plated {
                    file.fill(shape(of: pad), grownBy: clearance)
                } else if let hole = holeShape(of: pad) {
                    file.fill(hole, grownBy: edge)
                }
            }
            for v in cleared.vias {
                let via = copper.vias[v]
                file.fill(.circle(centre: via.centre, diameter: via.diameter), grownBy: clearance)
            }
            file.setDark(true)
        }
        for trace in document.traces where trace.layer == layer {
            file.stroke(trace.points, width: trace.widthMM)
        }
        let side: BoardSide? = layer == .top ? .top : layer == .bottom ? .bottom : nil
        for entry in copper.pads {
            if entry.pad.drill > 0 {
                if entry.pad.plated { file.fill(shape(of: entry.pad)) }
            } else if entry.side == side {
                file.fill(shape(of: entry.pad))
            }
        }
        for via in document.vias {
            file.fill(.circle(centre: via.centre, diameter: via.diameter))
        }
    }

    /// A solder mask: the openings (drawn dark, the file is Negative), each
    /// pad's copper grown by the mask expansion: the side's surface-mount
    /// pads and fingers, every plated through-hole pad, and each bare
    /// hole. Vias are tented (covered).
    private static func drawMask(on side: BoardSide, copper: BoardConnectivity, expansion: Double,
                                 into file: inout GerberWriter) {
        for entry in copper.pads {
            let pad = entry.pad
            if pad.drill > 0 {
                if pad.plated {
                    file.fill(shape(of: pad), grownBy: expansion)
                } else if let hole = holeShape(of: pad) {
                    file.fill(hole, grownBy: expansion)
                }
            } else if entry.side == side {
                file.fill(shape(of: pad), grownBy: expansion)
            }
        }
    }

    /// A side's silkscreen: texts first (their counters are cleared, which
    /// would also clear lines under them), then each footprint's silkscreen
    /// marks: body outline (and a diode's cathode band) and pin-1 dot. As
    /// on the PCB page, everything is seen from the top, so bottom text
    /// reads mirrored.
    private static func drawSilkscreen(on side: BoardSide, document: SchematicDocument,
                                       into file: inout GerberWriter) {
        let chips = document.chips.filter { $0.board?.side == side }
        for text in document.texts where text.side == side && text.layer == .silkscreen {
            for glyph in glyphs(of: text) { file.glyph(glyph) }
        }
        for chip in chips {
            guard let placement = chip.board, placement.reference == .silkscreen, let body = chip.boardBody,
                  !chip.reference.isEmpty else { continue }
            for glyph in referenceGlyphs(chip.reference, centre: CGPoint(x: body.rect.midX, y: body.rect.midY),
                                         mirrored: side == .bottom) {
                file.glyph(glyph)
            }
        }
        for chip in chips {
            guard let placement = chip.board, let body = chip.boardBody else { continue }
            if placement.outline == .silkscreen {
                let outline = body.round
                    ? GerberOutline.contours(of: CGPath(ellipseIn: body.rect, transform: nil)).first ?? []
                    : [CGPoint(x: body.rect.minX, y: body.rect.minY), CGPoint(x: body.rect.maxX, y: body.rect.minY),
                       CGPoint(x: body.rect.maxX, y: body.rect.maxY), CGPoint(x: body.rect.minX, y: body.rect.maxY)]
                file.stroke(outline, width: outlineWidth, closed: true)
                if let band = chip.cathodeBand { file.stroke([band.a, band.b], width: outlineWidth) }
            }
            if placement.pin1 == .silkscreen, let mark = chip.pin1Mark {
                file.fill(.circle(centre: mark, diameter: pin1Diameter))
            }
        }
    }

    // MARK: Shapes

    /// A pad's copper: a circle or plain rectangle as an aperture flash;
    /// an ellipse, rounded or chamfered rectangle as its outline, the same
    /// outline the PCB page fills.
    static func shape(of pad: PlacedPad) -> GerberShape {
        let r = pad.rect
        if pad.shape == .round, abs(r.width - r.height) < 1e-9 {
            return .circle(centre: CGPoint(x: r.midX, y: r.midY), diameter: r.width)
        }
        if pad.shape == .rect, pad.radius <= 0, pad.chamfer <= 0 || pad.corners.isEmpty {
            return .rect(r)
        }
        return .polygon(GerberOutline.contours(of: PCBCanvasView.outline(of: pad).cgPath).first ?? [])
    }

    /// A pad's drilled hole: a circle, or a slot as a capsule.
    static func holeShape(of pad: PlacedPad) -> GerberShape? {
        guard let hole = pad.holeSegment else { return nil }
        if TraceRoute.same(hole.a, hole.b) { return .circle(centre: hole.a, diameter: 2 * hole.radius) }
        return .capsule(a: hole.a, b: hole.b, diameter: 2 * hole.radius)
    }

    /// A board text's glyphs, placed as the PCB page draws it: laid out at
    /// the reference size, scaled to the board, turned about its anchor,
    /// mirrored inside its own box on Bottom.
    static func glyphs(of text: BoardText) -> [[[CGPoint]]] {
        let width = text.localSize.width
        var t = CGAffineTransform(translationX: text.anchor.x, y: text.anchor.y)
            .rotated(by: Double(text.rotation) * .pi / 180)
        if text.side == .bottom { t = t.translatedBy(x: width, y: 0).scaledBy(x: -1, y: 1) }
        t = t.scaledBy(x: text.drawScale, y: text.drawScale)
        return GerberOutline.glyphs(text.text, font: text.font as CTFont) { _ in t }
    }

    /// A footprint reference: 1 mm text centred on the body, mirrored on
    /// Bottom.
    static func referenceGlyphs(_ reference: String, centre: CGPoint, mirrored: Bool) -> [[[CGPoint]]] {
        let points: CGFloat = 100
        let font = NSFont.systemFont(ofSize: points, weight: .medium) as CTFont
        return GerberOutline.glyphs(reference, font: font) { size in
            var t = CGAffineTransform(translationX: centre.x, y: centre.y)
            if mirrored { t = t.scaledBy(x: -1, y: 1) }
            return t.scaledBy(x: referenceSize / points, y: referenceSize / points)
                .translatedBy(x: -size.width / 2, y: -size.height / 2)
        }
    }
}
