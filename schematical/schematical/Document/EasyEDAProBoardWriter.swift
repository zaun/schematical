import CoreGraphics
import Foundation

/// The `.epcb` (board) and `.efoo` (footprint) files of an EasyEDA Pro
/// export (`EasyEDAProExport`).
///
/// Board lengths are mil (1 unit = 0.0254 mm) and the board's y runs up, so
/// a point of the app's board (mm, y down) is `(x / 0.0254, -y / 0.0254)`.
/// A footprint is written around its body centre in the same frame, its pads
/// where they are in the footprint's own (unturned, top-side) frame. A
/// component turned `r` degrees clockwise is turned `-r` (EasyEDA counts
/// counter-clockwise), and one on Bottom is flipped top to bottom first, so its
/// angle is `180 - r`.
nonisolated final class EasyEDAProBoardWriter {
    private let build: EasyEDAProExport.Build
    private let document: SchematicDocument
    private let copper: BoardConnectivity
    private var ids = EasyEDAProExport.Build.Counter()
    /// The net name of each net the pads carry (nil: a net of one pin).
    private var netNames: [Int: String] = [:]

    @MainActor init(_ build: EasyEDAProExport.Build) {
        self.build = build
        document = build.document
        copper = build.context.copper
        var used: [String: Int] = [:]
        for net in Set(copper.netOfPad.values).union(copper.planeNets.values).sorted() {
            guard let base = build.netName(net) else { continue }
            let count = (used[base] ?? 0) + 1
            used[base] = count
            netNames[net] = count == 1 ? base : "\(base)_\(count)"
        }
    }

    // MARK: Units

    static let milPerMM = 1 / 0.0254

    @MainActor private func mil(_ mm: Double) -> Double { mm * Self.milPerMM }

    /// A board point as `x, y` in the file.
    @MainActor private func xy(_ point: CGPoint) -> [EProJSON] { [.num(point.x * Self.milPerMM), .num(-point.y * Self.milPerMM)] }

    /// Layer numbers: Top 1, Bottom 2, silkscreen 3 and 4, mask 5 and 6, paste 7 and 8,
    /// assembly 9 and 10, outline 11, multi-layer 12, document 13, then inner layers from 15.
    @MainActor private func layerID(_ layer: CopperLayer) -> Int {
        switch layer {
        case .top: 1
        case .bottom: 2
        case let .inner(n): 13 + n
        }
    }

    // MARK: Footprints

    /// The id of the footprint file for `chip`'s footprint, written once. Parts
    /// with a different set of absent pins get a footprint of their own.
    @MainActor static func footprintID(for chip: PlacedChip, build: EasyEDAProExport.Build) -> String? {
        guard let footprint = chip.footprint else { return nil }
        let hidden = Set(chip.part.pins.filter(\.isHidden).map(\.designator))
        let key = footprint.id + (hidden.isEmpty ? "" : "|" + hidden.sorted().joined(separator: ","))
        let pads = footprint.pads.filter { $0.designator.map { !hidden.contains($0) } ?? true }
        let title = hidden.isEmpty ? footprint.id : "\(footprint.id)-\(pads.count)P"
        return build.footprint(key, title: title) {
            EasyEDAProBoardWriter(build).footprintFile(chip: chip, footprint: footprint, hidden: hidden)
        }
    }

    private static let layerRows: [(Int, String, String, String)] = [
        (1, "TOP", "Top Layer", "#FF0000"), (2, "BOTTOM", "Bottom Layer", "#0000FF"),
        (3, "TOP_SILK", "Top Silkscreen Layer", "#FFCC00"), (4, "BOT_SILK", "Bottom Silkscreen Layer", "#66CC33"),
        (5, "TOP_SOLDER_MASK", "Top Solder Mask Layer", "#800080"),
        (6, "BOT_SOLDER_MASK", "Bottom Solder Mask Layer", "#AA00FF"),
        (7, "TOP_PASTE_MASK", "Top Paste Mask Layer", "#808080"),
        (8, "BOT_PASTE_MASK", "Bottom Paste Mask Layer", "#800000"),
        (9, "TOP_ASSEMBLY", "Top Assembly Layer", "#33CC99"), (10, "BOT_ASSEMBLY", "Bottom Assembly Layer", "#5555FF"),
        (11, "OUTLINE", "Board Outline Layer", "#FF00FF"), (12, "MULTI", "Multi-Layer", "#C0C0C0"),
        (13, "DOCUMENT", "Document Layer", "#FFFFFF"), (14, "MECHANICAL", "Mechanical Layer", "#F022F0"),
    ]

    @MainActor private func layerLines(inner: Int) -> String {
        var out = ""
        for row in Self.layerRows {
            out += EProJSON.line("LAYER", .int(row.0), .string(row.1), .string(row.2), 3, .string(row.3), 1,
                                 .string(row.3), 1)
        }
        for n in 0..<inner {
            out += EProJSON.line("LAYER", .int(15 + n), "SIGNAL", .string("Inner\(n + 1)"), 3, "#999966", 1, "#4C4C33", 1)
        }
        return out
    }

    @MainActor private func header(_ type: String, inner: Int = 0) -> String {
        EProJSON.line("DOCTYPE", .string(type), "1.6")
            + EProJSON.line("HEAD", .object([("editorVersion", "Schematical"), ("importFlag", 0)]))
            + EProJSON.line("CANVAS", 0, 0, "mil", 10, 10, 1, 1, 0.1, 0.1, 1)
            + layerLines(inner: inner)
    }

    /// A pad, as one file line sees it: its place and shape, in the frame
    /// of the file it goes in.
    struct PadSpec {
        var designator: String?
        var centre: CGPoint
        var size: CGSize
        var drill: Double
        var slotLength: Double
        var shape: Footprint.PadShape
        var radius: Double
        var plated: Bool
        var side: BoardSide
    }

    @MainActor private func spec(_ pad: Footprint.Pad) -> PadSpec {
        PadSpec(designator: pad.designator, centre: pad.centre, size: pad.size, drill: pad.drill,
                slotLength: pad.slotLength, shape: pad.shape, radius: pad.radius, plated: pad.plated, side: .top)
    }

    @MainActor private func spec(placed pad: PlacedPad, side: BoardSide) -> PadSpec {
        PadSpec(designator: pad.designator, centre: CGPoint(x: pad.rect.midX, y: pad.rect.midY), size: pad.rect.size,
                drill: pad.drill, slotLength: pad.slotLength, shape: pad.shape, radius: pad.radius,
                plated: pad.plated, side: side)
    }

    /// `["PAD", id, group, net, layer, number, x, y, rotation, hole, shape, [], holeX, holeY, holeTurn,
    /// plated, function, …, locked]`. A bare hole is its own pad: the hole, and a copper shape as big.
    @MainActor private func padLine(_ pad: PadSpec, net: String) -> String {
        let w = mil(pad.size.width), h = mil(pad.size.height)
        let through = pad.drill > 0
        var hole: EProJSON = nil
        var shape: EProJSON
        var layer = pad.side == .bottom ? 2 : 1
        if through {
            layer = 12
            let d = mil(pad.drill)
            let long = mil(max(pad.slotLength, pad.drill))
            let slot = pad.slotLength > pad.drill + 1e-9
            // A slot runs along the pad's longer side.
            let alongX = pad.size.width >= pad.size.height
            hole = slot ? .array(["SLOT", .num(alongX ? long : d), .num(alongX ? d : long)])
                        : .array(["ROUND", .num(d), .num(d)])
            if !pad.plated {
                shape = slot ? .array(["OVAL", .num(alongX ? long : d), .num(alongX ? d : long)])
                             : .array(["ELLIPSE", .num(d), .num(d)])
                return padRow("", pad, net: net, layer: layer, hole: hole, shape: shape)
            }
        }
        switch pad.shape {
        case .round:
            shape = abs(w - h) < 1e-6 ? .array(["ELLIPSE", .num(w), .num(h)]) : .array(["OVAL", .num(w), .num(h)])
        case .rect:
            let percent = pad.radius > 0 ? min(pad.radius / max(min(pad.size.width, pad.size.height), 1e-6) * 100, 50) : 0
            shape = .array(["RECT", .num(w), .num(h), .num(percent)])
        }
        return padRow(pad.designator ?? "", pad, net: net, layer: layer, hole: hole, shape: shape)
    }

    @MainActor private func padRow(_ number: String, _ spec: PadSpec, net: String, layer: Int, hole: EProJSON, shape: EProJSON) -> String {
        let id = ids.id()
        var items: [EProJSON] = ["PAD", .string(id), 0, .string(net), .int(layer), .string(number)]
        items += xy(spec.centre)
        items += [0, hole, shape, [], 0, 0, 0, .int(spec.plated ? 1 : 0), 0, nil, nil, nil, nil, 0]
        return EProJSON.array(items).text + "\n"
    }

    /// A footprint file: its pads in the footprint's frame, the body outline on
    /// the Top assembly layer and the pin-1 dot on Top silkscreen.
    @MainActor private func footprintFile(chip: PlacedChip, footprint: Footprint, hidden: Set<String>) -> String {
        var out = header("FOOTPRINT")
        for pad in footprint.pads where pad.designator.map({ !hidden.contains($0) }) ?? true {
            out += padLine(spec(pad), net: "")
        }
        let w = footprint.body.width / 2, h = footprint.body.height / 2
        func local(_ x: Double, _ y: Double) -> [EProJSON] { [.num(mil(x)), .num(-mil(y))] }
        var outline: [EProJSON]
        if footprint.bodyRound {
            let points = (0..<32).map { i -> (Double, Double) in
                let t = Double(i) / 32 * 2 * .pi
                return (w * cos(t), h * sin(t))
            }
            outline = local(points[0].0, points[0].1) + ["L"]
            for p in points.dropFirst() + [points[0]] { outline += local(p.0, p.1) }
        } else {
            outline = local(-w, -h) + ["L"] + local(w, -h) + local(w, h) + local(-w, h) + local(-w, -h)
        }
        out += EProJSON.line("POLY", .string(ids.id()), 0, "", 9, 4, .array(outline), 0)
        // The pin-1 dot, placed as the board places it for a part unturned on Top.
        var unturned = chip
        unturned.board = BoardPlacement(centre: .zero, rotation: 0)
        if let mark = unturned.pin1Mark {
            let r = GerberExport.pin1Diameter / 2
            out += EProJSON.line("FILL", .string(ids.id()), 0, "", 3, 0, 0,
                                 .array([.array(["CIRCLE", .num(mil(mark.x)), .num(-mil(mark.y)), .num(mil(r))])]), 0)
        }
        return out
    }

    // MARK: Board

    @MainActor func board() -> String {
        let board = document.board
        let inner = max(board.layers - 2, 0)
        var out = header("PCB", inner: inner)
        out += EProJSON.line("ACTIVE_LAYER", 1)
        let rules = board.rules
        out += EProJSON.line("RULE", "3", "trackWidth", 1,
                             .array(["mil", .num(mil(rules.minTraceWidth)), .num(mil(max(rules.minTraceWidth, 0.25))), 100]))

        // Nets, in code order.
        for name in Set(netNames.values).sorted() {
            out += EProJSON.line("NET", .string(name), nil, nil, 0, nil, 0, nil)
        }
        func net(_ n: Int?) -> String { n.flatMap { netNames[$0] } ?? "" }

        // The outline, closed.
        let outline = board.outline
        if outline.count > 2 {
            var points: [EProJSON] = xy(outline[0]) + ["L"]
            for p in outline.dropFirst() + [outline[0]] { points += xy(p) }
            out += EProJSON.line("POLY", .string(ids.id()), 0, "", 11, .num(mil(0.1)), .array(points), 0)
        }

        // Planes on a net, as a copper pour over the board.
        for layer in board.copperLayers {
            guard board.style(of: layer).type == .plane, let planeNet = copper.planeNets[layer] else { continue }
            var points: [EProJSON] = xy(outline[0]) + ["L"]
            for p in outline.dropFirst() + [outline[0]] { points += xy(p) }
            out += EProJSON.line("POUR", .string(ids.id()), 0, .string(net(planeNet)), .int(layerID(layer)), 10,
                                 .string("\(layer.title) \(net(planeNet))"), 1, .array([.array(points)]),
                                 .array(["SOLID", 0]), 0, 0)
        }

        // Copper.
        let nets = copper.netOfRoot()
        for (t, trace) in document.traces.enumerated() where t < copper.traces.count {
            let traceNet = net(nets[copper.root[copper.traceNode(t)]])
            for segment in trace.segments {
                var items: [EProJSON] = ["LINE", .string(ids.id()), 0, .string(traceNet), .int(layerID(trace.layer))]
                items += xy(segment.0) + xy(segment.1)
                items += [.num(Double(trace.width)), 0]
                out += EProJSON.array(items).text + "\n"
            }
        }
        for (v, via) in document.vias.enumerated() {
            var items: [EProJSON] = ["VIA", .string(ids.id()), 0, .string(net(nets[copper.root[copper.viaNode(v)]])), ""]
            items += xy(via.centre)
            items += [.num(mil(via.drill)), .num(mil(via.diameter)), 0, nil, nil, nil, nil, 0]
            out += EProJSON.array(items).text + "\n"
        }

        // The card's connector and the holes: pads of the board itself.
        var padIndex = build.context.edgeFirstPad
        for finger in board.fingers + board.extraPads {
            out += padLine(spec(placed: finger.pad, side: finger.side), net: net(copper.netOfPad[padIndex]))
            padIndex += 1
        }
        for hole in board.holes {
            out += padLine(spec(placed: hole.pad, side: .top), net: "")
            padIndex += 1
        }
        for hole in document.holes {
            out += padLine(spec(placed: hole.pad, side: .top), net: net(copper.netOfPad[padIndex]))
            padIndex += 1
        }

        // Board texts.
        for text in document.texts {
            let bottom = text.side == .bottom
            let layer = text.layer == .document ? 13 : (bottom ? 4 : 3)
            var items: [EProJSON] = ["STRING", .string(ids.id()), 0, .int(layer)]
            items += xy(text.anchor)
            // Top-left aligned, turned counter-clockwise about its point.
            items += [.string(text.text), "default", .num(mil(text.sizeMM)), .num(mil(text.sizeMM) * 0.15), 0, 0, 1,
                      .int((360 - text.rotation) % 360), 0, 0, .int(bottom ? 1 : 0), 0]
            out += EProJSON.array(items).text + "\n"
        }

        out += components()
        return out
    }

    /// Each part's footprint: where it is, its designator and device, and the
    /// net of each of its pads.
    @MainActor private func components() -> String {
        var out = ""
        var padIndex = 0
        for chip in document.chips {
            guard let placement = chip.board, let footprintID = Self.footprintID(for: chip, build: build) else { continue }
            let pads = chip.boardPads
            defer { padIndex += pads.count }
            let id = ids.id()
            let bottom = placement.side == .bottom
            let angle = bottom ? (540 - placement.rotation) % 360 : (360 - placement.rotation) % 360
            var items: [EProJSON] = ["COMPONENT", .string(id), 0, .int(bottom ? 2 : 1)]
            items += xy(placement.centre)
            items += [.int(angle), .object([]), 0]
            out += EProJSON.array(items).text + "\n"

            let device = build.assets(for: chip).device
            func attribute(_ key: String, _ value: String, visible: Bool = false, at point: CGPoint? = nil) -> String {
                var row: [EProJSON] = ["ATTR", .string(ids.id()), 0, .string(id), .int(bottom ? 4 : 3)]
                if let point { row += xy(point) } else { row += [nil, nil] }
                row += [.string(key), .string(value), 0, visible ? 1 : 0, "default", .num(mil(BoardReference.height) * 1.8),
                        .num(mil(0.15)), 0, 0, 5, 0, 0, 0, .int(bottom ? 1 : 0), 0]
                return EProJSON.array(row).text + "\n"
            }
            let showsReference = placement.reference != .hidden
            out += attribute("Designator", chip.reference, visible: showsReference, at: chip.boardReferenceCentre)
            out += attribute("Device", device)
            out += attribute("Footprint", footprintID)
            for (index, pad) in pads.enumerated() {
                guard let designator = pad.designator, let net = netNames[copper.netOfPad[padIndex + index] ?? -1] else { continue }
                out += EProJSON.line("PAD_NET", .string(id), .string(designator), .string(net))
            }
        }
        return out
    }
}
