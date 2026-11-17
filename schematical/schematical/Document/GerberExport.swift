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
    /// One file of the set: its name in the zip and its bytes, and for the job file what the file is:
    /// its `FileFunction` (the string in the file's own `TF.FileFunction`) and `FilePolarity`.
    struct File: Equatable {
        let name: String
        let data: Data
        var function: String? = nil
        var polarity: String? = nil

        var text: String { String(decoding: data, as: UTF8.self) }

        static func == (a: File, b: File) -> Bool { a.name == b.name && a.data == b.data }
    }

    /// The silkscreen's line width for body outlines (mm), as on screen.
    static let outlineWidth = 0.15
    /// The pin-1 dot's diameter (mm).
    static let pin1Diameter = 0.4
    /// The board outline's line width in the profile file (mm).
    static let profileWidth = 0.1

    /// Every layer file of the board, names starting `base`. Copper Top,
    /// inner layers and Bottom first, then solder mask, paste, silkscreen
    /// and the outline, then the drill files: plated holes, and non-plated
    /// ones when the board has any. A 1-layer board has no bottom files.
    static func files(for document: SchematicDocument, base: String,
                      rules: DesignRules? = nil) -> [File] {
        let board = document.board
        let rules = rules ?? board.rules
        let context = KiCadExport.Context(document: document, base: base)
        let copper = context.copper
        let nets = ExportNetNames.assign(context: context)
        let top = Double(board.outline.map(\.y).max() ?? CGFloat(board.height))
        let layers = board.copperLayers
        let sides: [BoardSide] = layers.count > 1 ? [.top, .bottom] : [.top]
        let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "1"
        func writer(_ function: String, polarity: String = "Positive") -> GerberWriter {
            GerberWriter(attributes: ["GenerationSoftware,jgz,Schematical,\(version)",
                                      "ProjectId,\(GerberWriter.fieldEscaped(base)),\(KiCad.uuid("gerber/\(base)")),1",
                                      "Part,Single", "FileFunction,\(function)",
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
            drawCopper(on: layer, board: board, document: document, copper: copper, nets: nets, rules: rules,
                       into: &file)
            files.append(File(name: "\(base).\(suffix)", data: file.data(),
                              function: "Copper,L\(index + 1),\(position)", polarity: "Positive"))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            let name = side == .top ? "Top" : "Bot"
            var mask = writer("Soldermask,\(name)", polarity: "Negative")
            drawMask(on: side, copper: copper, expansion: rules.solderMaskExpansion, into: &mask)
            files.append(File(name: "\(base).G\(s)S", data: mask.data(), function: "Soldermask,\(name)",
                              polarity: "Negative"))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            var paste = writer("Paste,\(side == .top ? "Top" : "Bot")")
            for entry in copper.pads where entry.chip >= 0 && entry.pad.drill == 0 && entry.side == side {
                paste.fill(shape(of: entry.pad))
            }
            files.append(File(name: "\(base).G\(s)P", data: paste.data(),
                              function: "Paste,\(side == .top ? "Top" : "Bot")", polarity: "Positive"))
        }
        for side in sides {
            let s = side == .top ? "T" : "B"
            var silk = writer("Legend,\(side == .top ? "Top" : "Bot")")
            drawSilkscreen(on: side, document: document, into: &silk)
            files.append(File(name: "\(base).G\(s)O", data: silk.data(),
                              function: "Legend,\(side == .top ? "Top" : "Bot")", polarity: "Positive"))
        }
        var profile = writer("Profile,NP")
        profile.setFunction("Profile")
        profile.stroke(board.outline, width: profileWidth, closed: true)
        files.append(File(name: "\(base).GKO", data: profile.data(), function: "Profile,NP", polarity: "Positive"))
        let holes = DrillExport.holes(copper)
        let software = "Schematical,\(version)"
        files.append(File(name: "\(base)-PTH.drl",
                          data: DrillExport.file(holes.plated, plated: true, layers: layers.count, top: top,
                                                 software: software),
                          function: "Plated,1,\(layers.count),PTH"))
        if !holes.bare.isEmpty {
            files.append(File(name: "\(base)-NPTH.drl",
                              data: DrillExport.file(holes.bare, plated: false, layers: layers.count, top: top,
                                                     software: software),
                              function: "NonPlated,1,\(layers.count),NPTH"))
        }
        files.append(File(name: "\(base)-job.gbrjob",
                          data: GerberJob.data(for: document, base: base, rules: rules, version: version, files: files)))
        return files
    }

    /// The set as one zip.
    static func zip(for document: SchematicDocument, base: String, rules: DesignRules? = nil) -> Data {
        ZipArchive.stored(files(for: document, base: base, rules: rules).map { ($0.name, $0.data) })
    }

    // MARK: Layers

    /// One copper layer: a Plane's copper (the board less its clearances),
    /// then traces on the layer, the surface-mount pads and card fingers of
    /// its side, and every plated through-hole pad and via. Every object has
    /// its `.AperFunction`, net and (a part's pad) pin, see `padAttributes`.
    private static func drawCopper(on layer: CopperLayer, board: BoardSettings, document: SchematicDocument,
                                   copper: BoardConnectivity, nets: ExportNets, rules: DesignRules,
                                   into file: inout GerberWriter) {
        let style = board.style(of: layer)
        let netOfRoot = copper.netOfRoot()
        func name(ofNet net: Int?) -> String { net.flatMap { nets.names[$0] } ?? "" }
        if style.type == .plane {
            let clearance = style.clearance(rules: rules), edge = rules.minCopperToEdge
            file.setFunction("Conductor")
            file.setObject(GerberWriter.Object(net: name(ofNet: copper.planeNets[layer])))
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
        file.setFunction("Conductor")
        for (t, trace) in document.traces.enumerated() where trace.layer == layer {
            let net = t < copper.traces.count ? netOfRoot[copper.root[copper.traceNode(t)]] : nil
            file.setObject(GerberWriter.Object(net: name(ofNet: net)))
            file.stroke(trace.points, width: trace.widthMM)
        }
        let side: BoardSide? = layer == .top ? .top : layer == .bottom ? .bottom : nil
        for (p, entry) in copper.pads.enumerated() {
            let draws = entry.pad.drill > 0 ? entry.pad.plated : entry.side == side
            guard draws else { continue }
            let (function, object) = padAttributes(entry, index: p, document: document, nets: nets)
            file.setFunction(function)
            file.setObject(object)
            file.fill(shape(of: entry.pad))
        }
        file.setFunction("ViaPad")
        for (v, via) in document.vias.enumerated() {
            let net = v < copper.vias.count ? netOfRoot[copper.root[copper.viaNode(v)]] : nil
            file.setObject(GerberWriter.Object(net: name(ofNet: net)))
            file.fill(.circle(centre: via.centre, diameter: via.diameter))
        }
        file.setFunction(nil)
        file.setObject(nil)
    }

    /// The `.AperFunction` and the object attributes of a pad's copper:
    /// - a surface-mount part pad is `SMDPad,CuDef`, an unnumbered one (a centre pad) `HeatsinkPad`, a plated
    ///   through-hole part pad `ComponentPad`; each carries the part's reference and the pin;
    /// - a card's edge finger is `ConnectorPad` of `EDGE1` (pin: the finger's name), a header pad `ComponentPad` of
    ///   `HEADER1`, the names the KiCad export gives the connector;
    /// - the land of a plated board hole is `OtherPad,PlatedHole`, with no pin.
    /// The net is the pad's (`nets.padNets`), empty when it has none.
    static func padAttributes(_ entry: BoardConnectivity.Pad, index: Int, document: SchematicDocument,
                              nets: ExportNets) -> (function: String, object: GerberWriter.Object) {
        let net = nets.padNets[index].flatMap { nets.names[$0] } ?? ""
        let pad = entry.pad
        if entry.chip >= 0 {
            let reference = document.chips[entry.chip].reference
            let function = pad.drill > 0 ? "ComponentPad" : pad.designator == nil ? "HeatsinkPad" : "SMDPad,CuDef"
            return (function, GerberWriter.Object(net: net, pin: .init(reference: reference,
                                                                       number: pad.designator ?? "")))
        }
        for (prefix, reference, function) in [("Edge.", "EDGE1", "ConnectorPad"), ("Header.", "HEADER1", "ComponentPad")]
        where entry.label.hasPrefix(prefix) {
            return (function, GerberWriter.Object(net: net, pin: .init(reference: reference,
                                                                       number: String(entry.label.dropFirst(prefix.count)))))
        }
        return ("OtherPad,PlatedHole", GerberWriter.Object(net: net))
    }

    /// A solder mask: the openings (drawn dark, the file is Negative), each
    /// pad's copper grown by the mask expansion: the side's surface-mount
    /// pads and fingers, every plated through-hole pad, and each bare
    /// hole, and each via left open on this side (`BoardVia.isTented(on:)`):
    /// its metal grown the same. Tented vias are covered.
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
        for via in copper.vias where !via.isTented(on: side) {
            file.fill(.circle(centre: via.centre, diameter: via.diameter), grownBy: expansion)
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
            guard let placement = chip.board, placement.reference == .silkscreen,
                  let at = chip.boardReferenceCentre, !chip.reference.isEmpty else { continue }
            for glyph in referenceGlyphs(chip.reference, centre: at,
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
            return t.scaledBy(x: BoardReference.height / points, y: BoardReference.height / points)
                .translatedBy(x: -size.width / 2, y: -size.height / 2)
        }
    }
}
