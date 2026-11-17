import CoreGraphics
import Foundation

/// The `.kicad_pcb` of a KiCad export (`KiCadExport`): KiCad 8's board
/// format.
///
/// Board millimetres are KiCad's (both y down), moved so the board sits
/// inside the page. A footprint is written at its body centre with the
/// pads in its own frame: a turn of `r` degrees clockwise is KiCad's angle
/// `-r` (it counts counter-clockwise), and a bottom-side footprint has its
/// x mirrored, which is how the app draws it before the turn
/// (`BoardPlacement.board(_:)`).
@MainActor
struct KiCadBoardWriter {
    private let context: KiCadExport.Context
    private let document: SchematicDocument
    private let copper: BoardConnectivity
    /// Millimetres added to every board point.
    private let shift: CGPoint
    private let page: CGSize
    /// The KiCad net code of each net the pads carry (`copper` net → code),
    /// and its name.
    private var codes: [Int: Int] = [:]
    private var names: [Int: String] = [:]
    /// The net of each pad (`copper.netOfPad`, and a net of its own for a pin no wire reaches).
    private var padNets: [Int: Int] = [:]

    init(_ context: KiCadExport.Context) {
        self.context = context
        document = context.document
        copper = context.copper
        let box = Self.extent(of: context.document)
        let margin = 30.0
        shift = CGPoint(x: (margin - box.minX).rounded(), y: (margin - box.minY).rounded())
        page = CGSize(width: box.maxX + shift.x + margin, height: box.maxY + shift.y + margin)
        assignNets()
    }

    // MARK: Layout

    private func id(_ key: String) -> String { KiCad.uuid("pcb/\(context.base)/\(key)") }

    private func xy(_ point: CGPoint) -> String {
        "\(KiCad.n(point.x + shift.x)) \(KiCad.n(point.y + shift.y))"
    }

    /// The box of the board and everything on it, in mm.
    private static func extent(of document: SchematicDocument) -> CGRect {
        var box = CGRect.null
        for point in document.board.outline { box = box.union(CGRect(x: point.x, y: point.y, width: 0, height: 0)) }
        for chip in document.chips { if let bounds = chip.boardBounds { box = box.union(bounds) } }
        for trace in document.traces { for p in trace.points { box = box.union(CGRect(x: p.x, y: p.y, width: 0, height: 0)) } }
        for via in document.vias { box = box.union(via.rect) }
        for hole in document.holes { box = box.union(hole.rect) }
        for text in document.texts { box = box.union(CGRect(x: text.anchor.x, y: text.anchor.y, width: 0, height: 0)) }
        return box.isNull ? CGRect(x: 0, y: 0, width: 100, height: 80) : box
    }

    /// KiCad's name for a copper layer.
    private func layerName(_ layer: CopperLayer) -> String {
        switch layer {
        case .top: "F.Cu"
        case .bottom: "B.Cu"
        case let .inner(n): "In\(n - 1).Cu"
        }
    }

    // MARK: Nets

    /// Gives every net a pad carries (and every Plane's) a KiCad code and
    /// the name KiCad gives it when it reads the schematic, and each pin
    /// with no wire a net of its own, as KiCad does (`unconnected-(U1-A0-Pad1)`).
    private mutating func assignNets() {
        padNets = copper.netOfPad
        // Pins no wire reaches.
        var nextNet = (copper.netOfPad.values.max() ?? 0) + 1
        var ownNet: [String: Int] = [:]
        var pinNames: [Int: String] = [:]
        var padIndex = 0
        for (index, chip) in document.chips.enumerated() where chip.board?.side != nil {
            for pad in chip.boardPads {
                defer { padIndex += 1 }
                guard padNets[padIndex] == nil, let designator = pad.designator,
                      let pin = chip.part.pins.first(where: { $0.designator == designator }), !pin.isHidden else { continue }
                let key = "\(index)/\(designator)"
                if ownNet[key] == nil {
                    ownNet[key] = nextNet
                    pinNames[nextNet] = Self.pinNetName(chip: chip, pin: pin, unconnected: true)
                    nextNet += 1
                }
                padNets[padIndex] = ownNet[key]
            }
        }
        var nets = Set(padNets.values)
        nets.formUnion(copper.planeNets.values)
        var used: [String: Int] = [:]
        for net in nets.sorted() {
            let base = pinNames[net] ?? baseName(ofNet: net)
            let count = (used[base] ?? 0) + 1
            used[base] = count
            names[net] = count == 1 ? base : "\(base)_\(count)"
            codes[net] = codes.count + 1
        }
    }

    private static func pinNetName(chip: PlacedChip, pin: ChipPin, unconnected: Bool) -> String {
        ExportNetNames.pinNetName(chip: chip, pin: pin, unconnected: unconnected)
    }

    /// A net's name as KiCad gives it: a label's text comes after the sheet's
    /// "/", a "/" in it written `{slash}`.
    private func baseName(ofNet net: Int) -> String {
        ExportNetNames.baseName(ofNet: net, context: context) { "/" + $0.replacingOccurrences(of: "/", with: "{slash}") }
    }

    private func netText(_ net: Int?) -> String {
        guard let net, let code = codes[net], let name = names[net] else { return "" }
        return "\n\t\t\t(net \(code) \(KiCad.q(name)))"
    }

    // MARK: File

    func text() -> String {
        let board = document.board
        var out = """
        (kicad_pcb
        \t(version 20240108)
        \t(generator "schematical")
        \t(generator_version "8.0")
        \t(general
        \t\t(thickness 1.6)
        \t\t(legacy_teardrops no)
        \t)
        \t\(KiCad.paper(width: page.width, height: page.height))
        \t(layers

        """
        var copperLayers = ["0 \"F.Cu\" signal"]
        if board.layers > 2 { for n in 1...(board.layers - 2) { copperLayers.append("\(2 + 2 * n) \"In\(n).Cu\" signal") } }
        copperLayers.append("2 \"B.Cu\" signal")
        let others = [
            "9 \"F.Adhes\" user \"F.Adhesive\"", "11 \"B.Adhes\" user \"B.Adhesive\"",
            "13 \"F.Paste\" user", "15 \"B.Paste\" user",
            "5 \"F.SilkS\" user \"F.Silkscreen\"", "7 \"B.SilkS\" user \"B.Silkscreen\"",
            "1 \"F.Mask\" user", "3 \"B.Mask\" user",
            "17 \"Dwgs.User\" user \"User.Drawings\"", "19 \"Cmts.User\" user \"User.Comments\"",
            "21 \"Eco1.User\" user \"User.Eco1\"", "23 \"Eco2.User\" user \"User.Eco2\"",
            "25 \"Edge.Cuts\" user", "27 \"Margin\" user",
            "31 \"F.CrtYd\" user \"F.Courtyard\"", "29 \"B.CrtYd\" user \"B.Courtyard\"",
            "35 \"F.Fab\" user", "33 \"B.Fab\" user",
        ]
        for entry in copperLayers + others { out += "\t\t(\(entry))\n" }
        out += "\t)\n"
        out += """
        \t(setup
        \t\t(pad_to_mask_clearance \(KiCad.n(board.rules.solderMaskExpansion)))
        \t\t(allow_soldermask_bridges_in_footprints no)
        \t)
        \t(net 0 "")

        """
        for net in codes.keys.sorted(by: { codes[$0]! < codes[$1]! }) {
            out += "\t(net \(codes[net]!) \(KiCad.q(names[net] ?? "")))\n"
        }
        out += footprints().joined(separator: "\n") + "\n"
        out += outline().joined(separator: "\n") + "\n"
        out += texts().joined(separator: "\n") + "\n"
        out += tracks().joined(separator: "\n") + "\n"
        out += zones().joined(separator: "\n") + "\n"
        out += ")\n"
        return out
    }

    // MARK: Pads

    /// A pad in a footprint's own frame.
    private struct KPad {
        var designator: String
        var centre: CGPoint
        var size: CGSize
        var drill: Double
        var slotLength: Double
        var shape: Footprint.PadShape
        var radius: Double
        var chamfer: Double
        var corners: Footprint.Corners
        var plated: Bool
        var side: BoardSide
    }

    /// A pad of a catalog footprint, its x mirrored when the footprint is on
    /// Bottom.
    private func kpad(_ pad: Footprint.Pad, side: BoardSide) -> KPad {
        let bottom = side == .bottom
        return KPad(designator: pad.designator ?? "",
                    centre: CGPoint(x: bottom ? -pad.centre.x : pad.centre.x, y: pad.centre.y),
                    size: pad.size, drill: pad.drill, slotLength: pad.slotLength, shape: pad.shape,
                    radius: pad.radius, chamfer: pad.chamfer, corners: pad.corners.mirrored(bottom),
                    plated: pad.plated, side: side)
    }

    /// A pad already on the board (a card finger, a hole), for a footprint
    /// at the page origin.
    private func kpad(placed pad: PlacedPad, side: BoardSide) -> KPad {
        KPad(designator: pad.designator ?? "", centre: CGPoint(x: pad.rect.midX, y: pad.rect.midY),
             size: pad.rect.size, drill: pad.drill, slotLength: pad.slotLength, shape: pad.shape,
             radius: pad.radius, chamfer: pad.chamfer, corners: pad.corners, plated: pad.plated, side: side)
    }

    /// One pad. `angle` is the footprint's angle: KiCad keeps a pad's turn
    /// as an absolute angle, so a pad that follows its footprint has it.
    private func padText(_ pad: KPad, angle: Int, net: Int?, key: String) -> String {
        let through = pad.drill > 0
        let type = !through ? "smd" : pad.plated ? "thru_hole" : "np_thru_hole"
        let w = pad.size.width, h = pad.size.height
        var shape: String
        var extra = ""
        if !pad.plated && through {
            // A bare hole: its pad is the hole.
            let long = max(pad.slotLength, pad.drill)
            let slot = long > pad.drill + 1e-9
            shape = slot ? "oval" : "circle"
            let size = w >= h || !slot ? CGSize(width: long, height: pad.drill) : CGSize(width: pad.drill, height: long)
            return padLine(pad, type: type, shape: shape, size: size, angle: angle, extra: "", net: nil, key: key,
                           layers: "\"*.Cu\" \"*.Mask\"")
        }
        switch pad.shape {
        case .round:
            shape = abs(w - h) < 1e-9 ? "circle" : "oval"
        case .rect:
            let small = min(w, h)
            if pad.chamfer > 0, !pad.corners.isEmpty {
                shape = "roundrect"
                var corners: [String] = []
                if pad.corners.contains(.topLeft) { corners.append("top_left") }
                if pad.corners.contains(.topRight) { corners.append("top_right") }
                if pad.corners.contains(.bottomLeft) { corners.append("bottom_left") }
                if pad.corners.contains(.bottomRight) { corners.append("bottom_right") }
                extra = "\n\t\t\t(roundrect_rratio 0)\n\t\t\t(chamfer_ratio \(KiCad.n(min(pad.chamfer / small, 0.5))))\n\t\t\t(chamfer \(corners.joined(separator: " ")))"
            } else if pad.radius > 0 {
                shape = "roundrect"
                extra = "\n\t\t\t(roundrect_rratio \(KiCad.n(min(pad.radius / small, 0.5))))"
            } else {
                shape = "rect"
            }
        }
        let bottom = pad.side == .bottom
        let layers: String
        if through {
            layers = "\"*.Cu\" \"*.Mask\""
        } else {
            layers = bottom ? "\"B.Cu\" \"B.Paste\" \"B.Mask\"" : "\"F.Cu\" \"F.Paste\" \"F.Mask\""
        }
        return padLine(pad, type: type, shape: shape, size: pad.size, angle: angle, extra: extra, net: net, key: key,
                       layers: layers)
    }

    private func padLine(_ pad: KPad, type: String, shape: String, size: CGSize, angle: Int, extra: String,
                         net: Int?, key: String, layers: String) -> String {
        var drill = ""
        if pad.drill > 0 {
            if pad.slotLength > pad.drill + 1e-9 {
                // The slot runs along the pad's longer side.
                let alongX = pad.size.width >= pad.size.height
                drill = "\n\t\t\t(drill oval \(KiCad.n(alongX ? pad.slotLength : pad.drill)) \(KiCad.n(alongX ? pad.drill : pad.slotLength)))"
            } else {
                drill = "\n\t\t\t(drill \(KiCad.n(pad.drill)))"
            }
        }
        return """
        \t\t(pad \(KiCad.q(pad.designator)) \(type) \(shape)
        \t\t\t(at \(KiCad.n(pad.centre.x)) \(KiCad.n(pad.centre.y))\(angle == 0 ? "" : " \(angle)"))
        \t\t\t(size \(KiCad.n(size.width)) \(KiCad.n(size.height)))\(drill)
        \t\t\t(layers \(layers))\(extra)\(netText(net))
        \t\t\t(uuid \(KiCad.q(id(key))))
        \t\t)
        """
    }

    // MARK: Footprints

    private func footprints() -> [String] {
        var out: [String] = []
        var padIndex = 0
        for chip in document.chips {
            guard let placement = chip.board, let footprint = chip.footprint else { continue }
            let pads = chip.boardPads
            out.append(footprintText(chip, placement: placement, footprint: footprint, firstPad: padIndex))
            padIndex += pads.count
        }
        out += cardFootprints(from: padIndex)
        return out
    }

    private func footprintText(_ chip: PlacedChip, placement: BoardPlacement, footprint: Footprint,
                               firstPad: Int) -> String {
        let bottom = placement.side == .bottom
        let angle = (360 - placement.rotation) % 360
        let at = placement.centre
        func local(_ p: CGPoint) -> CGPoint {
            let l = placement.local(p)
            return CGPoint(x: bottom ? -l.x : l.x, y: l.y)
        }
        let copperSide = bottom ? "B.Cu" : "F.Cu"
        let silk = bottom ? "B.SilkS" : "F.SilkS", fab = bottom ? "B.Fab" : "F.Fab"
        let hidden = Set(chip.part.pins.filter(\.isHidden).map(\.designator))
        let visible = footprint.pads.filter { pad in pad.designator.map { !hidden.contains($0) } ?? true }
        let name = "\(KiCadExport.library):\(KiCad.libraryName(footprint.id))"

        var body: [String] = []
        let markLayer: (MarkLayer) -> String? = { $0 == .silkscreen ? silk : $0 == .document ? fab : nil }
        if let layer = markLayer(placement.outline) {
            let w = footprint.body.width, h = footprint.body.height
            if footprint.bodyRound {
                let points = (0..<32).map { i -> String in
                    let t = Double(i) / 32 * 2 * .pi
                    return "(xy \(KiCad.n(w / 2 * cos(t))) \(KiCad.n(h / 2 * sin(t))))"
                }
                body.append("\t\t(fp_poly\n\t\t\t(pts \(points.joined(separator: " ")))\n\t\t\t(stroke (width 0.15) (type default))\n\t\t\t(fill none)\n\t\t\t(layer \(KiCad.q(layer)))\n\t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/body"))))\n\t\t)")
            } else {
                body.append("\t\t(fp_rect\n\t\t\t(start \(KiCad.n(-w / 2)) \(KiCad.n(-h / 2)))\n\t\t\t(end \(KiCad.n(w / 2)) \(KiCad.n(h / 2)))\n\t\t\t(stroke (width 0.15) (type default))\n\t\t\t(fill none)\n\t\t\t(layer \(KiCad.q(layer)))\n\t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/body"))))\n\t\t)")
            }
            if let band = chip.cathodeBand {
                let a = local(band.a), b = local(band.b)
                body.append("\t\t(fp_line\n\t\t\t(start \(KiCad.n(a.x)) \(KiCad.n(a.y)))\n\t\t\t(end \(KiCad.n(b.x)) \(KiCad.n(b.y)))\n\t\t\t(stroke (width 0.15) (type default))\n\t\t\t(layer \(KiCad.q(layer)))\n\t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/band"))))\n\t\t)")
            }
        }
        if let layer = markLayer(placement.pin1), let mark = chip.pin1Mark {
            let p = local(mark)
            body.append("\t\t(fp_circle\n\t\t\t(center \(KiCad.n(p.x)) \(KiCad.n(p.y)))\n\t\t\t(end \(KiCad.n(p.x + GerberExport.pin1Diameter / 2)) \(KiCad.n(p.y)))\n\t\t\t(stroke (width 0.1) (type default))\n\t\t\t(fill solid)\n\t\t\t(layer \(KiCad.q(layer)))\n\t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/pin1"))))\n\t\t)")
        }

        var pads: [String] = []
        for (index, pad) in visible.enumerated() {
            let net = padNets[firstPad + index]
            pads.append(padText(kpad(pad, side: placement.side), angle: angle, net: net, key: "pad/\(chip.id)/\(index)"))
        }
        let through = visible.contains { $0.drill > 0 }
        let attribute = visible.isEmpty ? "" : "\n\t\t(attr \(through ? "through_hole" : "smd"))"

        let refLayer = markLayer(placement.reference) ?? silk
        let offset = CGPoint(x: bottom ? -placement.referenceOffset.x : placement.referenceOffset.x,
                             y: placement.referenceOffset.y)
        let mirror = bottom ? " (justify mirror)" : ""
        let font = "(font (size \(KiCad.n(BoardReference.height)) \(KiCad.n(BoardReference.height))) (thickness 0.15))"
        let datasheet = chip.part.datasheetURL?.absoluteString ?? ""
        let value = chip.ohms.map { ResistorValue.text($0) } ?? chip.farads.map { CapacitorValue.text($0) } ?? chip.partNumber
        return """
        \t(footprint \(KiCad.q(name))
        \t\t(layer \(KiCad.q(copperSide)))
        \t\t(uuid \(KiCad.q(id("fp/\(chip.id)"))))
        \t\t(at \(xy(at))\(angle == 0 ? "" : " \(angle)"))
        \t\t(descr \(KiCad.q(footprint.description)))
        \t\t(property "Reference" \(KiCad.q(chip.reference))
        \t\t\t(at \(KiCad.n(offset.x)) \(KiCad.n(offset.y)) 0)
        \t\t\t(layer \(KiCad.q(refLayer)))\(placement.reference == .hidden ? "\n\t\t\thide" : "")
        \t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/ref"))))
        \t\t\t(effects \(font)\(mirror))
        \t\t)
        \t\t(property "Value" \(KiCad.q(value))
        \t\t\t(at 0 0 0)
        \t\t\t(layer \(KiCad.q(fab)))
        \t\t\thide
        \t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/value"))))
        \t\t\t(effects \(font)\(mirror))
        \t\t)
        \t\t(property "Footprint" \(KiCad.q(name))
        \t\t\t(at 0 0 0)
        \t\t\t(layer \(KiCad.q(fab)))
        \t\t\thide
        \t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/footprint"))))
        \t\t\t(effects \(font))
        \t\t)
        \t\t(property "Datasheet" \(KiCad.q(datasheet))
        \t\t\t(at 0 0 0)
        \t\t\t(layer \(KiCad.q(fab)))
        \t\t\thide
        \t\t\t(uuid \(KiCad.q(id("fp/\(chip.id)/datasheet"))))
        \t\t\t(effects \(font))
        \t\t)
        \t\t(path \(KiCad.q("/" + chip.id.uuidString.lowercased())))
        \t\t(sheetname "Root")
        \t\t(sheetfile \(KiCad.q("\(context.base).kicad_sch")))\(attribute)
        \(body.joined(separator: "\n"))
        \(pads.joined(separator: "\n"))
        \t)
        """
    }

    /// The card's edge fingers or header pads, its standard's mounting
    /// holes, and the drilled holes, each a footprint at the page origin
    /// with its pads where they are on the board. `first` is the index in
    /// `copper.pads` of the first of them.
    private func cardFootprints(from first: Int) -> [String] {
        let board = document.board
        var out: [String] = []
        var index = first
        func footprint(reference: String, name: String, pads: [(KPad, Int?)], key: String, uuid: String? = nil,
                       path: String? = nil, at point: CGPoint = .zero) -> String {
            let lines = pads.enumerated().map { padText($0.element.0, angle: 0, net: $0.element.1, key: "\(key)/\($0.offset)") }
            let through = pads.contains { $0.0.drill > 0 }
            return """
            \t(footprint \(KiCad.q("\(KiCadExport.library):\(name)"))
            \t\t(layer "F.Cu")
            \t\t(uuid \(KiCad.q(uuid ?? id(key))))
            \t\t(at \(xy(point)))
            \t\t(property "Reference" \(KiCad.q(reference))
            \t\t\t(at 0 0 0)
            \t\t\t(layer "F.SilkS")
            \t\t\thide
            \t\t\t(uuid \(KiCad.q(id("\(key)/ref"))))
            \t\t\t(effects (font (size 1 1) (thickness 0.15)))
            \t\t)
            \t\t(property "Value" \(KiCad.q(name))
            \t\t\t(at 0 0 0)
            \t\t\t(layer "F.Fab")
            \t\t\thide
            \t\t\t(uuid \(KiCad.q(id("\(key)/value"))))
            \t\t\t(effects (font (size 1 1) (thickness 0.15)))
            \t\t)
            \t\t\(path.map { "(path \(KiCad.q($0))) (sheetname \"Root\") (sheetfile \(KiCad.q("\(context.base).kicad_sch")))\n\t\t" } ?? "")(attr \(through ? "through_hole" : "smd")\(path == nil ? " board_only" : "") exclude_from_pos_files exclude_from_bom)
            \(lines.joined(separator: "\n"))
            \t)
            """
        }
        let edge = board.fingers + board.extraPads
        if !edge.isEmpty {
            var pads: [(KPad, Int?)] = []
            for finger in edge {
                pads.append((kpad(placed: finger.pad, side: finger.side), padNets[index]))
                index += 1
            }
            out.append(footprint(reference: board.connectorName.uppercased() + "1", name: board.connectorName,
                                 pads: pads, key: "edge", uuid: id("edge"),
                                 path: "/" + context.edgeSymbolUUID))
        }
        for (n, hole) in board.holes.enumerated() {
            out.append(footprint(reference: "MH\(n + 1)", name: "MountingHole",
                                 pads: [(kpad(placed: hole.pad, side: .top), nil)], key: "mounting/\(n)"))
            index += 1
        }
        for (n, hole) in document.holes.enumerated() {
            out.append(footprint(reference: "H\(n + 1)", name: "MountingHole",
                                 pads: [(kpad(placed: hole.pad, side: .top), padNets[index])], key: "hole/\(hole.id)"))
            index += 1
        }
        return out
    }

    // MARK: Board

    private func outline() -> [String] {
        let points = document.board.outline
        guard points.count > 1 else { return [] }
        return points.indices.map { i in
            let a = points[i], b = points[(i + 1) % points.count]
            return "\t(gr_line\n\t\t(start \(xy(a)))\n\t\t(end \(xy(b)))\n\t\t(stroke (width 0.1) (type default))\n\t\t(layer \"Edge.Cuts\")\n\t\t(uuid \(KiCad.q(id("edge/\(i)"))))\n\t)"
        }
    }

    private func texts() -> [String] {
        document.texts.map { text in
            let bottom = text.side == .bottom
            let layer = text.layer == .document ? "Dwgs.User" : (bottom ? "B.SilkS" : "F.SilkS")
            let angle = (360 - text.rotation) % 360
            let size = KiCad.n(text.sizeMM)
            return """
            \t(gr_text \(KiCad.q(text.text))
            \t\t(at \(xy(text.anchor))\(angle == 0 ? " 0" : " \(angle)"))
            \t\t(layer \(KiCad.q(layer)))
            \t\t(uuid \(KiCad.q(id("text/\(text.id)"))))
            \t\t(effects
            \t\t\t(font (size \(size) \(size)) (thickness \(KiCad.n(text.sizeMM * 0.15))))
            \t\t\t(justify left top\(bottom ? " mirror" : ""))
            \t\t)
            \t)
            """
        }
    }

    private func tracks() -> [String] {
        var out: [String] = []
        let nets = copper.netOfRoot()
        for (t, trace) in document.traces.enumerated() where t < copper.traces.count {
            let net = nets[copper.root[copper.traceNode(t)]]
            for (s, segment) in trace.segments.enumerated() {
                out.append("""
                \t(segment
                \t\t(start \(xy(segment.0)))
                \t\t(end \(xy(segment.1)))
                \t\t(width \(KiCad.n(trace.widthMM)))
                \t\t(layer \(KiCad.q(layerName(trace.layer))))\(netText(net))
                \t\t(uuid \(KiCad.q(id("trace/\(trace.id)/\(s)"))))
                \t)
                """)
            }
        }
        for (v, via) in document.vias.enumerated() {
            let net = nets[copper.root[copper.viaNode(v)]]
            out.append("""
            \t(via
            \t\t(at \(xy(via.centre)))
            \t\t(size \(KiCad.n(via.diameter)))
            \t\t(drill \(KiCad.n(via.drill)))
            \t\t(layers "F.Cu" "B.Cu")\(netText(net))
            \t\t(uuid \(KiCad.q(id("via/\(via.id)"))))
            \t)
            """)
        }
        return out
    }

    /// Each inner Plane that carries a net, as a zone over the board. The
    /// zone is not filled: KiCad fills it with Fill All Zones (B).
    private func zones() -> [String] {
        let board = document.board
        var out: [String] = []
        for layer in board.copperLayers {
            let style = board.style(of: layer)
            guard style.type == .plane, let net = copper.planeNets[layer] else { continue }
            let points = board.outline.map { "(xy \(xy($0)))" }.joined(separator: " ")
            out.append("""
            \t(zone
            \t\t(net \(codes[net] ?? 0))
            \t\t(net_name \(KiCad.q(names[net] ?? "")))
            \t\t(layer \(KiCad.q(layerName(layer))))
            \t\t(uuid \(KiCad.q(id("zone/\(layer.rawValue)"))))
            \t\t(hatch edge 0.5)
            \t\t(connect_pads
            \t\t\t(clearance \(KiCad.n(style.clearance(rules: board.rules))))
            \t\t)
            \t\t(min_thickness 0.2)
            \t\t(filled_areas_thickness no)
            \t\t(fill yes
            \t\t\t(thermal_gap 0.5)
            \t\t\t(thermal_bridge_width 0.5)
            \t\t)
            \t\t(polygon
            \t\t\t(pts \(points))
            \t\t)
            \t)
            """)
        }
        return out
    }
}
