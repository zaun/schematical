import SwiftUI

/// How a placed part looks in the board preview: a generic body for its
/// package family, built from boxes and cans in the footprint's own frame
/// (mm, origin = body centre, y down) and placed through its
/// `BoardPlacement`. Nothing part-specific: a DIP is a blank DIP.
struct PartLook {
    enum Kind {
        case dip(ceramic: Bool)
        case leadedIC(ceramic: Bool)
        case leadlessIC(ceramic: Bool)
        case chipResistor
        case chipCapacitor
        case chipLED(Tint)
        case moldedTantalum
        case axialResistor
        case axialDiode(glass: Bool)
        case smdDiode
        case electrolytic(smd: Bool)
        case tantalumDrop
        case upright(Tint)
        case radialLED(Tint)
        case squareLED(Tint)
        case sevenSegment(digits: Int, tint: Tint)
        /// A nixie tube: a glass envelope standing on the board (side view)
        /// or a low one read from above (end view).
        case nixie(endView: Bool, tint: Tint)
        case lcd(columns: Int, rows: Int)
        case dipSwitch(positions: Int)
        case tactile
        case header(female: Bool)
        case canOscillator
        case smdOscillator
        case usb(String?)
        /// A card slot (plan-54-O): a dark housing over the footprint's pads,
        /// `height` mm tall (by what the slot takes).
        case cardSlot(height: CGFloat)
        /// A PCB terminal block (plan-45-H): a green housing with a dark wire
        /// entry over each pole.
        case terminalBlock
        /// A JST board header (plan-45-H): a natural-white housing with a
        /// polarization notch at pin 1.
        case jstHeader
        /// A test point (plan-57): a bare pad or plated hole with no body.
        case testPad
        case barrel
        case toPackage
        case block(height: CGFloat)
    }

    let chip: PlacedChip
    let kind: Kind
    let footprint: Footprint
    let placement: BoardPlacement
    /// The pads on the board (hidden pins left out), in the footprint frame.
    let pads: [Footprint.Pad]

    // MARK: Classification

    static func of(_ chip: PlacedChip) -> PartLook {
        let footprint = chip.footprint!
        let placement = chip.board!
        let hidden = Set(chip.part.pins.filter(\.isHidden).map(\.designator))
        let pads = footprint.pads.filter { $0.designator.map { !hidden.contains($0) } ?? true }
        return PartLook(chip: chip, kind: kind(of: chip, footprint: footprint, pads: pads),
                        footprint: footprint, placement: placement, pads: pads)
    }

    private static func kind(of chip: PlacedChip, footprint: Footprint, pads: [Footprint.Pad]) -> Kind {
        let id = footprint.id.uppercased()
        let part = chip.part
        let tht = pads.contains { $0.drill > 0 }
        let body = footprint.body
        let short = min(body.width, body.height), long = max(body.width, body.height)
        let twoPads = Set(pads.compactMap(\.designator)).count == 2
        // Two through-hole pads beyond both ends of the body: an axial part.
        let axial = tht && twoPads && pads.allSatisfy { pad in
            body.width >= body.height ? abs(pad.centre.x) > body.width / 2 : abs(pad.centre.y) > body.height / 2
        }

        switch part.kind {
        case PART_HEADER:
            return .header(female: chip.gender == "Female")
        case PART_CHARLCD:
            if case let .characterLCD(info) = part.details { return .lcd(columns: info.columns, rows: info.rows) }
            return .lcd(columns: 16, rows: 2)
        case PART_7SEG:
            if case let .display(info) = part.details {
                // A numitron or VFD is a glass tube standing on its leads.
                if info.type != "led" { return .nixie(endView: false, tint: ledTint(info.color)) }
                return .sevenSegment(digits: max(1, info.digits), tint: ledTint(info.color))
            }
            return .sevenSegment(digits: 1, tint: ledTint("Red"))
        case PART_LCD:
            if case let .display(info) = part.details {
                return .sevenSegment(digits: max(1, info.digits), tint: ledTint(info.color))
            }
            return .sevenSegment(digits: 1, tint: ledTint("Gray"))
        case PART_NIXIE:
            return .nixie(endView: part.nixie?.view == "end", tint: ledTint(part.nixie?.color ?? "Orange"))
        case PART_DEKATRON:
            return .nixie(endView: part.dekatron?.view == "end", tint: ledTint(part.dekatron?.color ?? "Orange"))
        case PART_SWITCH:
            if part.isTactileSwitch { return .tactile }
            // A slide switch is a metal housing, 5 mm tall (plan-58-G).
            if part.isSlideSwitch { return .block(height: 5) }
            return .dipSwitch(positions: max(1, chip.switchesOn.count))
        case PART_LED:
            let tint = ledTint(chip.color ?? "Red")
            if !tht { return .chipLED(tint) }
            return footprint.bodyRound ? .radialLED(tint) : .squareLED(tint)
        case PART_CONNECTOR:
            if part.isTestPoint { return .testPad }
            if case let .connector(_, _, _, _, usb) = part.details, let usb { return .usb(usb) }
            if part.isCardSlot { return .cardSlot(height: cardSlotHeight(of: part)) }
            if case let .wireConnector(kind, _, _, _, _) = part.details { return kind == "jst" ? .jstHeader : .terminalBlock }
            if id.hasPrefix("USB") { return .usb(nil) }
            return .barrel
        case PART_OSCILLATOR:
            return tht ? .canOscillator : .smdOscillator
        case PART_CAPACITOR:
            if id.hasPrefix("CAPE") { return .electrolytic(smd: !tht) }
            if id.hasPrefix("CAPT") { return .tantalumDrop }
            if id.hasPrefix("CAPM") { return .moldedTantalum }
            if id.hasPrefix("CAP-DISC") { return .upright(Tint(0.80, 0.45, 0.18)) }
            if id.hasPrefix("CAP-RADIAL") { return .upright(Tint(0.22, 0.40, 0.72)) }
            if id.hasPrefix("CAP-RAD") { return .upright(Tint(0.88, 0.72, 0.28)) }
            if !tht { return twoPads ? .chipCapacitor : .leadedIC(ceramic: false) }
            return footprint.bodyRound ? .electrolytic(smd: false) : .upright(Tint(0.88, 0.72, 0.28))
        case PART_DIODE:
            if id.hasPrefix("TO-") || id.hasPrefix("ITO-") { return .toPackage }
            if axial { return .axialDiode(glass: id.contains("DO-35")) }
            if tht { return .block(height: short / long < 0.35 ? long * 0.8 : min(max(short * 0.4, 2), 10)) }
            if twoPads { return .smdDiode }
        default:
            break
        }
        if chip.symbol == .pullUp || chip.symbol == .pullDown || id.hasPrefix("RES-") {
            if axial { return .axialResistor }
            if !tht && twoPads { return .chipResistor }
        }
        if id.hasPrefix("TO-") || id.hasPrefix("ITO-") { return .toPackage }

        // Everything else is an IC: its leads by where its pads are.
        let ceramic = part.flags.ceramic
        if tht { return .dip(ceramic: ceramic) }
        let outside = pads.contains { pad in
            abs(pad.centre.x) > body.width / 2 || abs(pad.centre.y) > body.height / 2
        }
        return outside ? .leadedIC(ceramic: ceramic) : .leadlessIC(ceramic: ceramic)
    }

    /// How tall a card slot stands, mm, by its drawer leaf: the common height
    /// of DigiKey's listings (plan-54-O § 3): 3.0 for a memory / SIM socket,
    /// 5.2 for a memory module, 6.0 for a card-edge socket.
    static func cardSlotHeight(of part: CatalogPart) -> CGFloat {
        switch part.drawer.last {
        case "Memory module": 5.2
        case "Card edge": 6.0
        default: 3.0
        }
    }

    static func ledTint(_ name: String) -> Tint {
        switch name.lowercased() {
        case "green": Tint(0.18, 0.82, 0.28)
        case "blue": Tint(0.18, 0.38, 0.98)
        case "amber", "orange": Tint(1.0, 0.62, 0.08)
        case "yellow": Tint(1.0, 0.86, 0.12)
        case "white", "rgb": Tint(0.94, 0.94, 0.92)
        case "warm white": Tint(1.0, 0.82, 0.5)
        case "blue-green": Tint(0.25, 0.9, 0.8)
        default: Tint(0.92, 0.14, 0.10)
        }
    }

    // MARK: Geometry

    private var body: CGSize { footprint.body }
    private var bodyRect: CGRect { CGRect(x: -body.width / 2, y: -body.height / 2, width: body.width, height: body.height) }
    private var short: CGFloat { min(body.width, body.height) }
    private var long: CGFloat { max(body.width, body.height) }
    /// The body's long axis runs along x.
    private var horizontal: Bool { body.width >= body.height }

    /// The footprint frame on the board.
    private var transform: CGAffineTransform {
        var t = CGAffineTransform(translationX: placement.centre.x, y: placement.centre.y)
        t = t.rotated(by: CGFloat(placement.rotation) * .pi / 180)
        if placement.side == .bottom { t = t.scaledBy(x: -1, y: 1) }
        return t
    }

    private func board(_ rect: CGRect) -> CGRect { rect.applying(placement) }
    private func board(_ point: CGPoint) -> CGPoint { placement.board(point) }

    private func pad(named designator: String?) -> Footprint.Pad? {
        guard let designator else { return nil }
        return pads.first { $0.designator == designator }
    }

    private func pin(named names: Set<String>) -> Footprint.Pad? {
        pad(named: chip.part.pins.first { names.contains($0.name) }?.designator)
    }

    private var pad1: Footprint.Pad? { pad(named: chip.part.firstPinDesignator) }

    /// How tall the part stands, mm.
    var height: CGFloat {
        switch kind {
        case let .dip(ceramic): ceramic ? 4.6 : (short > 10 ? 4.6 : 3.9)
        case .leadedIC: 0.1 + min(max(short * 0.2, 0.9), 1.6)
        case .leadlessIC: min(max(short * 0.15, 0.5), 1.2)
        case .chipResistor: min(max(short * 0.6, 0.3), 0.6)
        case .chipCapacitor: min(short * 0.8, 1.25)
        case .chipLED: min(short * 0.8, 0.9)
        case .moldedTantalum: min(max(short * 0.7, 0.8), 2.8)
        case .axialResistor, .axialDiode: 0.3 + short
        case .smdDiode: min(max(short * 0.6, 0.4), 2.3)
        case let .electrolytic(smd): smd ? 0.8 + short * 0.8 + 1 : min(max(short * 1.25 + 1, 5), 30)
        case .tantalumDrop: 1 + short * 1.2
        case .upright: 1 + long * 0.95
        case .radialLED: 8.6
        case .squareLED: 4.5
        case .sevenSegment: 8
        // A side-view tube stands about 2.2 diameters tall; an end-view one
        // is about as tall as it is wide.
        case let .nixie(endView, _): endView ? short * 0.9 : short * 2.2
        case .lcd: 12
        case .dipSwitch: 3.8
        case .tactile: 5
        case .header: 8.5
        case .canOscillator: 5.5
        case .smdOscillator: 0.35 + min(max(short * 0.25, 0.5), 1.4)
        case let .usb(type):
            switch type {
            case "A": 7
            case "B": 10.5
            case "C": 3.2
            default: 4
            }
        case let .cardSlot(height): height
        case .terminalBlock: 9.5
        case .jstHeader: 6
        case .testPad: 0
        case .barrel: 11
        case .toPackage: 15
        case let .block(height): height
        }
    }

    // MARK: Solids

    func solids(painter: SolidPainter) -> [RenderSolid] {
        var out: [RenderSolid] = []
        let projection = painter.projection
        let local = transform

        /// A box over a footprint-frame rectangle; `decorate` draws on its
        /// top in the footprint frame.
        func box(_ rect: CGRect, _ z0: CGFloat, _ z1: CGFloat, _ tint: Tint, top: Tint? = nil, radius: CGFloat = 0,
                 depth: CGFloat? = nil, decorate: ((inout GraphicsContext) -> Void)? = nil) {
            let r = board(rect)
            let centre = board(CGPoint(x: rect.midX, y: rect.midY))
            let shape = radius > 0 ? Path(roundedRect: r, cornerRadius: radius) : Path(r)
            out.append(RenderSolid(depth: depth ?? (projection.depth(centre) + z0 * 0.001), height: z1, shadow: shape) { context in
                painter.box(r, from: z0, to: z1, tint: tint, top: top, radius: radius, in: &context,
                            decorate: decorate.map { draw in { layer in
                                layer.concatenate(local)
                                draw(&layer)
                            } })
            })
        }

        func can(_ centre: CGPoint, _ radius: CGFloat, _ z0: CGFloat, _ z1: CGFloat, _ tint: Tint, top: Tint? = nil,
                 decorate: ((inout GraphicsContext) -> Void)? = nil) {
            let c = board(centre)
            let shape = Path(ellipseIn: CGRect(x: c.x - radius, y: c.y - radius, width: 2 * radius, height: 2 * radius))
            out.append(RenderSolid(depth: projection.depth(c) + z0 * 0.001, height: z1, shadow: shape) { context in
                painter.can(c, radius: radius, from: z0, to: z1, tint: tint, top: top, in: &context,
                            decorate: decorate.map { draw in { layer in
                                layer.concatenate(local)
                                draw(&layer)
                            } })
            })
        }

        /// A rectangle given by its centre and size, in the footprint frame.
        func rect(_ cx: CGFloat, _ cy: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect {
            CGRect(x: cx - w / 2, y: cy - h / 2, width: w, height: h)
        }

        /// A rectangle along the body's long axis: from `a` to `b` along it,
        /// `across` wide, centred across.
        func along(_ a: CGFloat, _ b: CGFloat, across: CGFloat, at c: CGFloat = 0) -> CGRect {
            let lo = min(a, b), hi = max(a, b)
            return horizontal ? CGRect(x: lo, y: c - across / 2, width: hi - lo, height: across)
                : CGRect(x: c - across / 2, y: lo, width: across, height: hi - lo)
        }

        /// The coordinate of a point along the long axis.
        func position(_ p: CGPoint) -> CGFloat { horizontal ? p.x : p.y }

        /// Leads from the body's edge out to each pad beyond it.
        func leads(foot z1: CGFloat, shoulder: CGFloat, width: CGFloat, throughHole: Bool) {
            // A lead is sorted against the body by the side it leaves from, not
            // by its own centre: on a long body turned in the view a far-side
            // lead near the body's near end is nearer than the body's centre
            // and would be painted over the body.
            let bodyDepth = projection.depth(board(.zero))
            func leadDepth(exit: CGPoint) -> CGFloat {
                let d = projection.depth(board(exit)) - bodyDepth
                return bodyDepth + (d < 0 ? -0.02 : 0.02)
            }
            for pad in pads {
                let c = pad.centre, w2 = body.width / 2, h2 = body.height / 2
                var leg: CGRect?
                if abs(c.x) > w2, abs(c.x) - w2 >= abs(c.y) - h2 {
                    let s: CGFloat = c.x > 0 ? 1 : -1
                    let across = throughHole ? width : min(pad.size.height * 0.62, width)
                    let end = throughHole ? c.x + s * 0.25 : c.x + s * pad.size.width * 0.32
                    leg = CGRect(x: min(s * w2, end), y: c.y - across / 2, width: abs(end - s * w2), height: across)
                } else if abs(c.y) > h2 {
                    let s: CGFloat = c.y > 0 ? 1 : -1
                    let across = throughHole ? width : min(pad.size.width * 0.62, width)
                    let end = throughHole ? c.y + s * 0.25 : c.y + s * pad.size.height * 0.32
                    leg = CGRect(x: c.x - across / 2, y: min(s * h2, end), width: across, height: abs(end - s * h2))
                }
                guard let leg else { continue }
                // The side's midpoint, not where along the side the lead is: the sort is by which side.
                let exit = horizontalSide(of: c) ? CGPoint(x: c.x > 0 ? w2 : -w2, y: 0)
                    : CGPoint(x: 0, y: c.y > 0 ? h2 : -h2)
                let depth = leadDepth(exit: exit)
                box(leg, 0, throughHole ? shoulder : z1, .tin, top: Tint.tin.shaded(1.15), depth: depth)
                if !throughHole {
                    // The shoulder where the lead leaves the body.
                    let s = horizontalSide(of: c)
                    let shoulderRect = s
                        ? CGRect(x: c.x > 0 ? body.width / 2 : -body.width / 2 - 0.25, y: leg.minY, width: 0.25, height: leg.height)
                        : CGRect(x: leg.minX, y: c.y > 0 ? body.height / 2 : -body.height / 2 - 0.25, width: leg.width, height: 0.25)
                    box(shoulderRect, 0, shoulder, .tin, depth: depth)
                }
            }
        }

        func horizontalSide(of c: CGPoint) -> Bool {
            abs(c.x) > body.width / 2 && abs(c.x) - body.width / 2 >= abs(c.y) - body.height / 2
        }

        /// A pin-1 dimple and, on a DIP, a notch at pin 1's end.
        func icMarks(notch: Bool) -> (inout GraphicsContext) -> Void {
            let r = max(short * 0.07, 0.25)
            let p = pad1?.centre ?? CGPoint(x: -body.width / 2, y: -body.height / 2)
            let dimple = CGPoint(x: min(max(p.x, -body.width / 2 + 2.2 * r), body.width / 2 - 2.2 * r),
                                 y: min(max(p.y, -body.height / 2 + 2.2 * r), body.height / 2 - 2.2 * r))
            let pin1Along = position(p)
            return { layer in
                let ring = Path(ellipseIn: CGRect(x: dimple.x - r, y: dimple.y - r, width: 2 * r, height: 2 * r))
                layer.fill(ring, with: .color(.black.opacity(0.35)))
                layer.stroke(ring, with: .color(.white.opacity(0.08)), lineWidth: r * 0.25)
                guard notch else { return }
                layer.clip(to: Path(bodyRect))
                let n = short * 0.13
                let end = (pin1Along < 0 ? -1 : 1) * long / 2
                let centre = horizontal ? CGPoint(x: end, y: 0) : CGPoint(x: 0, y: end)
                layer.fill(Path(ellipseIn: CGRect(x: centre.x - n, y: centre.y - n, width: 2 * n, height: 2 * n)),
                           with: .color(.black.opacity(0.55)))
            }
        }

        /// Metal end caps of a two-terminal chip, as boxes.
        func chipEnds(_ z: CGFloat, fraction: CGFloat = 0.22) {
            let cap = long * fraction
            box(along(-long / 2, -long / 2 + cap, across: short), 0, z, .tin, top: Tint.tin.shaded(1.1))
            box(along(long / 2 - cap, long / 2, across: short), 0, z, .tin, top: Tint.tin.shaded(1.1))
        }

        /// Wire leads from each pad to the nearer end of an axial body.
        func axialLeads(_ z: CGFloat) {
            for pad in pads {
                let p = position(pad.centre)
                let end = (p < 0 ? -1 : 1) * long / 2
                box(along(p, end, across: 0.6), 0, z, .tin, top: Tint.tin.shaded(1.2))
            }
        }

        let h = height
        switch kind {
        case let .dip(ceramic):
            leads(foot: 0, shoulder: 2.2, width: 1.0, throughHole: true)
            let tint = ceramic ? Tint.ceramicIC : Tint.epoxy
            box(bodyRect, 0.5, h, tint, top: tint.shaded(1.15), decorate: icMarks(notch: true))

        case let .leadedIC(ceramic):
            leads(foot: 0.22, shoulder: (h + 0.1) / 2, width: 0.6, throughHole: false)
            let tint = ceramic ? Tint.ceramicIC : Tint.epoxy
            box(bodyRect, 0.1, h, tint, top: tint.shaded(1.15), decorate: icMarks(notch: false))

        case let .leadlessIC(ceramic):
            let tint = ceramic ? Tint.ceramicIC : Tint.epoxy
            box(bodyRect, 0, h, tint, top: tint.shaded(1.15), decorate: icMarks(notch: false))

        case .chipResistor:
            chipEnds(h)
            box(along(-long / 2 + long * 0.22, long / 2 - long * 0.22, across: short), 0, h, Tint(0.12, 0.12, 0.12))

        case .chipCapacitor:
            chipEnds(h)
            box(along(-long / 2 + long * 0.22, long / 2 - long * 0.22, across: short), 0, h, Tint(0.66, 0.54, 0.38))

        case let .chipLED(tint):
            box(bodyRect, 0, h, Tint(0.93, 0.93, 0.90)) { layer in
                let lens = CGRect(x: -body.width * 0.3, y: -body.height * 0.32, width: body.width * 0.6, height: body.height * 0.64)
                layer.fill(Path(roundedRect: lens, cornerRadius: short * 0.15), with: .color(tint.shaded(0.85).color))
            }

        case .moldedTantalum:
            chipEnds(h * 0.6, fraction: 0.14)
            let plus = pin(named: ["+"]).map { position($0.centre) } ?? -1
            box(along(-long / 2 + long * 0.1, long / 2 - long * 0.1, across: short), 0, h, Tint(0.86, 0.64, 0.10)) { layer in
                let s: CGFloat = plus < 0 ? -1 : 1
                let band = along(s * (long / 2 - long * 0.1), s * (long / 2 - long * 0.24), across: short)
                layer.fill(Path(band), with: .color(Tint(0.48, 0.30, 0.06).color))
            }

        case .axialResistor:
            axialLeads(0.3 + short / 2)
            box(bodyRect, 0.3, h, Tint(0.82, 0.72, 0.54), radius: short / 2) { layer in
                let bands: [(CGFloat, Tint)] = [(0.22, Tint(0.45, 0.25, 0.10)), (0.36, Tint(0.06, 0.06, 0.06)),
                                                (0.50, Tint(0.80, 0.12, 0.10)), (0.78, Tint(0.86, 0.70, 0.30))]
                for (at, colour) in bands {
                    let a = -long / 2 + long * at
                    layer.fill(Path(along(a - long * 0.045, a + long * 0.045, across: short)), with: .color(colour.color))
                }
            }

        case let .axialDiode(glass):
            axialLeads(0.3 + short / 2)
            let cathode = pin(named: ["K"]).map { position($0.centre) } ?? 1
            let tint = glass ? Tint(0.82, 0.38, 0.14) : Tint(0.10, 0.10, 0.11)
            box(bodyRect, 0.3, h, tint, radius: short / 2) { layer in
                let s: CGFloat = cathode < 0 ? -1 : 1
                let band = along(s * long * 0.28, s * long * 0.40, across: short)
                layer.fill(Path(band), with: .color(glass ? Tint(0.12, 0.12, 0.12).color : Tint(0.82, 0.82, 0.84).color))
            }

        case .smdDiode:
            let cathode = pin(named: ["K"]).map { position($0.centre) } ?? 1
            box(bodyRect, 0, h, Tint(0.12, 0.12, 0.13)) { layer in
                let s: CGFloat = cathode < 0 ? -1 : 1
                let band = along(s * long * 0.26, s * long * 0.40, across: short)
                layer.fill(Path(band), with: .color(Tint(0.78, 0.78, 0.80).color))
            }

        case let .electrolytic(smd):
            let r = short / 2 * (smd ? 0.92 : 1)
            let z0: CGFloat = smd ? 0.8 : 0.3
            if smd {
                box(bodyRect, 0, z0, Tint(0.12, 0.12, 0.12))
            }
            let minus = pin(named: ["-", "−"])?.centre
            can(.zero, r, z0, h, Tint(0.10, 0.14, 0.32)) { layer in
                let alu = r * 0.86
                layer.fill(Path(ellipseIn: CGRect(x: -alu, y: -alu, width: 2 * alu, height: 2 * alu)),
                           with: .color(Tint.steel.color))
                // The vent.
                var vent = Path()
                vent.move(to: CGPoint(x: -alu * 0.55, y: 0)); vent.addLine(to: CGPoint(x: alu * 0.55, y: 0))
                vent.move(to: CGPoint(x: 0, y: -alu * 0.55)); vent.addLine(to: CGPoint(x: 0, y: alu * 0.55))
                layer.stroke(vent, with: .color(Tint(0.45, 0.46, 0.48).color), lineWidth: max(r * 0.06, 0.12))
                // The minus stripe, on the rim by the minus lead.
                if let minus {
                    let angle = atan2(minus.y, minus.x)
                    let rim = (r + alu) / 2
                    var arc = Path()
                    arc.addArc(center: .zero, radius: rim, startAngle: .radians(angle - 0.6),
                               endAngle: .radians(angle + 0.6), clockwise: false)
                    layer.stroke(arc, with: .color(Tint(0.70, 0.72, 0.78).color), lineWidth: r - alu)
                }
            }

        case .tantalumDrop:
            can(.zero, short / 2, 1, h, Tint(0.88, 0.58, 0.10))

        case let .upright(tint):
            box(bodyRect, 1, h, tint, radius: short / 2)

        case let .radialLED(tint):
            let r = short / 2
            can(.zero, r, 0.2, 1.2, tint.shaded(0.8))
            can(.zero, r * 0.86, 1.2, h, tint.shaded(0.9)) { layer in
                let g = r * 0.86
                layer.fill(Path(ellipseIn: CGRect(x: -g * 0.6, y: -g * 0.6, width: g * 1.2, height: g * 1.2)),
                           with: .radialGradient(Gradient(colors: [.white.opacity(0.75), tint.color.opacity(0)]),
                                                 center: .zero, startRadius: 0, endRadius: g * 0.6))
            }

        case let .squareLED(tint):
            box(bodyRect, 0, 2, Tint(0.93, 0.93, 0.90))
            can(.zero, short * 0.33, 2, h, tint.shaded(0.9))

        case let .sevenSegment(digits, tint):
            box(bodyRect, 0, h, Tint(0.20, 0.20, 0.21), top: Tint(0.07, 0.07, 0.08)) { layer in
                drawDigits(digits, tint: tint, in: &layer)
            }

        case let .nixie(endView, tint):
            // Glass over a dark base; the glow shows through the top.
            let glass = Tint(0.50, 0.44, 0.38)
            let glow: (inout GraphicsContext) -> Void = { layer in
                let r = short * (endView ? 0.30 : 0.22)
                layer.fill(Path(ellipseIn: CGRect(x: -r, y: -r, width: 2 * r, height: 2 * r)),
                           with: .radialGradient(Gradient(colors: [tint.color.opacity(0.9), tint.color.opacity(0)]),
                                                 center: .zero, startRadius: 0, endRadius: r))
            }
            if footprint.bodyRound {
                can(.zero, short / 2, 0, 1.5, Tint(0.12, 0.12, 0.12))
                can(.zero, short / 2 * 0.97, 1.5, h, glass, top: glass.shaded(1.15), decorate: glow)
            } else {
                box(bodyRect, 0, 1.5, Tint(0.12, 0.12, 0.12), radius: short * 0.2)
                box(bodyRect, 1.5, h, glass, top: glass.shaded(1.15), radius: short * 0.2, decorate: glow)
            }

        case let .lcd(columns, rows):
            // The module's header, its board raised on it, the bezel and
            // the glass.
            let padBounds = pads.filter { $0.drill > 0 }.reduce(CGRect.null) {
                $0.union(CGRect(x: $1.centre.x - 1.27, y: $1.centre.y - 1.27, width: 2.54, height: 2.54))
            }
            if !padBounds.isNull { box(padBounds, 0, 6, Tint(0.08, 0.08, 0.08)) }
            box(bodyRect, 6, 7.6, Tint(0.10, 0.38, 0.16))
            let bezel = rect(0, 0, body.width * 0.9, body.height * 0.66)
            box(bezel, 7.6, h, Tint(0.30, 0.31, 0.33), top: Tint(0.36, 0.37, 0.39)) { layer in
                let glass = bezel.insetBy(dx: bezel.width * 0.06, dy: bezel.height * 0.14)
                layer.fill(Path(glass), with: .linearGradient(
                    Gradient(colors: [Tint(0.62, 0.72, 0.40).color, Tint(0.52, 0.63, 0.30).color]),
                    startPoint: CGPoint(x: glass.minX, y: glass.minY), endPoint: CGPoint(x: glass.maxX, y: glass.maxY)))
                // Faint character cells.
                let cols = max(columns, 1), rws = max(rows, 1)
                let area = glass.insetBy(dx: glass.width * 0.04, dy: glass.height * 0.12)
                let cw = area.width / CGFloat(cols), ch = area.height / CGFloat(rws)
                var cells = Path()
                for row in 0..<rws {
                    for col in 0..<cols {
                        cells.addRect(CGRect(x: area.minX + CGFloat(col) * cw + cw * 0.08,
                                             y: area.minY + CGFloat(row) * ch + ch * 0.06,
                                             width: cw * 0.84, height: ch * 0.88))
                    }
                }
                layer.fill(cells, with: .color(.black.opacity(0.06)))
            }

        case let .dipSwitch(positions):
            let along1 = pad(named: "1")?.centre, along2 = pad(named: "2")?.centre
            // The pads of one row run along this axis.
            let rowX = along1.flatMap { a in along2.map { b in abs(b.x - a.x) >= abs(b.y - a.y) } } ?? horizontal
            let rowLength = rowX ? body.width : body.height, across = rowX ? body.height : body.width
            let on = chip.switchesOn
            box(bodyRect, 0, h, Tint(0.12, 0.28, 0.70)) { layer in
                for i in 0..<positions {
                    let at = pad(named: "\(i + 1)").map { rowX ? $0.centre.x : $0.centre.y }
                        ?? (-rowLength / 2 + rowLength * (CGFloat(i) + 0.5) / CGFloat(positions))
                    let pitch = rowLength / CGFloat(max(positions, 1))
                    let sw = min(pitch * 0.6, 1.6), sl = across * 0.5
                    let slot = rowX ? rect(at, 0, sw, sl) : rect(0, at, sl, sw)
                    layer.fill(Path(slot), with: .color(Tint(0.05, 0.05, 0.06).color))
                    let isOn = i < on.count && on[i]
                    let offset = (isOn ? -1 : 1) * sl / 4
                    let knob = rowX ? rect(at, offset, sw * 0.9, sl / 2) : rect(offset, at, sl / 2, sw * 0.9)
                    layer.fill(Path(knob), with: .color(Tint(0.95, 0.95, 0.93).color))
                }
            }

        case .tactile:
            box(bodyRect, 0, 3.4, Tint(0.13, 0.13, 0.14))
            can(.zero, short * 0.3, 3.4, h, Tint(0.22, 0.22, 0.24))

        case let .header(female):
            if female {
                box(bodyRect, 0, h, Tint(0.09, 0.09, 0.10)) { layer in
                    for pad in pads {
                        layer.fill(Path(rect(pad.centre.x, pad.centre.y, 1.0, 1.0)), with: .color(Tint(0.02, 0.02, 0.02).color))
                    }
                }
            } else {
                box(bodyRect, 0, 2.5, Tint(0.09, 0.09, 0.10))
                for pad in pads {
                    box(rect(pad.centre.x, pad.centre.y, 0.64, 0.64), 2.5, h, .gold, top: Tint.gold.shaded(1.2))
                }
            }

        case .canOscillator:
            box(bodyRect, 0.4, h, .steel, radius: min(1.2, short * 0.12)) { layer in
                let inner = bodyRect.insetBy(dx: 0.8, dy: 0.8)
                layer.stroke(Path(roundedRect: inner, cornerRadius: 0.8), with: .color(.white.opacity(0.35)), lineWidth: 0.25)
            }

        case .smdOscillator:
            box(bodyRect, 0, 0.35, Tint(0.86, 0.82, 0.72))
            box(bodyRect.insetBy(dx: short * 0.1, dy: short * 0.1), 0.35, h, .steel)

        case .usb:
            box(bodyRect, 0, h, .steel) { layer in
                var ridges = Path()
                for f in [0.3, 0.7] as [CGFloat] {
                    let y = -body.height / 2 + body.height * f
                    ridges.move(to: CGPoint(x: -body.width * 0.4, y: y))
                    ridges.addLine(to: CGPoint(x: body.width * 0.4, y: y))
                }
                layer.stroke(ridges, with: .color(.black.opacity(0.18)), lineWidth: 0.2)
            }

        case .cardSlot:
            // The housing is the bounds of the pads: a dark box over them.
            let padBounds = pads.reduce(CGRect.null) {
                $0.union(CGRect(x: $1.centre.x - $1.size.width / 2, y: $1.centre.y - $1.size.height / 2,
                                width: $1.size.width, height: $1.size.height))
            }
            let housing = Tint(0.16, 0.17, 0.19)
            if !padBounds.isNull { box(padBounds, 0, h, housing, top: housing.shaded(1.15)) }

        case .terminalBlock:
            // A green housing over the footprint's body with a dark wire entry
            // over each pole (one per pin: the pad of each designator, or the
            // middle of a pole's two holes).
            let green = Tint(0.17, 0.58, 0.32)
            var poles: [String: [CGPoint]] = [:]
            for pad in pads { if let designator = pad.designator { poles[designator, default: []].append(pad.centre) } }
            var pitch: CGFloat = 5
            if case let .wireConnector(_, _, pitchUM, _, _) = chip.part.details { pitch = CGFloat(pitchUM) / 1000 }
            let entry = min(max(pitch * 0.6, 1.6), 3.2)
            box(bodyRect, 0, h, green, top: green.shaded(1.12)) { layer in
                for (_, points) in poles {
                    let cx = points.map(\.x).reduce(0, +) / CGFloat(points.count)
                    let cy = points.map(\.y).reduce(0, +) / CGFloat(points.count)
                    layer.fill(Path(rect(cx, cy, entry, entry)), with: .color(Tint(0.08, 0.08, 0.09).color))
                }
            }

        case .jstHeader:
            // A natural-white housing; a dark notch on the top edge at pin 1's end.
            let white = Tint(0.93, 0.91, 0.84)
            let p1 = pad1?.centre ?? CGPoint(x: -body.width / 2, y: 0)
            let notch = horizontal
                ? rect(min(max(p1.x, -body.width / 2 + 0.6), body.width / 2 - 0.6), -body.height / 2 + 0.45, 1.2, 0.9)
                : rect(-body.width / 2 + 0.45, min(max(p1.y, -body.height / 2 + 0.6), body.height / 2 - 0.6), 0.9, 1.2)
            box(bodyRect, 0, h, white, top: white.shaded(1.05)) { layer in
                layer.fill(Path(notch), with: .color(Tint(0.35, 0.33, 0.28).color))
            }

        case .testPad:
            // A test point has no body; the pad is drawn by the board's copper.
            break

        case .barrel:
            box(bodyRect, 0, h, Tint(0.10, 0.10, 0.11), top: Tint(0.14, 0.14, 0.15))

        case .toPackage:
            // The body and its metal tab along the long side away from the
            // leads.
            let centroid = pads.isEmpty ? 0 : pads.map { horizontal ? $0.centre.y : $0.centre.x }.reduce(0, +) / CGFloat(pads.count)
            let s: CGFloat = centroid > 0.05 ? -1 : 1
            let tab: CGFloat = min(1.3, short * 0.3)
            let bodyPart = horizontal
                ? CGRect(x: -long / 2, y: s > 0 ? -short / 2 : -short / 2 + tab, width: long, height: short - tab)
                : CGRect(x: s > 0 ? -short / 2 : -short / 2 + tab, y: -long / 2, width: short - tab, height: long)
            let tabPart = horizontal
                ? CGRect(x: -long / 2, y: s > 0 ? short / 2 - tab : -short / 2, width: long, height: tab)
                : CGRect(x: s > 0 ? short / 2 - tab : -short / 2, y: -long / 2, width: tab, height: long)
            box(tabPart, 0, h, .steel)
            box(bodyPart, 0, h * 0.6, .epoxy, top: Tint.epoxy.shaded(1.15))

        case let .block(height):
            box(bodyRect, 0, height, .epoxy, top: Tint.epoxy.shaded(1.15))
        }
        return out
    }

    /// Unlit seven-segment digits across the face, upright in the
    /// footprint frame.
    private func drawDigits(_ digits: Int, tint: Tint, in layer: inout GraphicsContext) {
        let cell = body.width / CGFloat(digits)
        let digitH = min(body.height * 0.72, cell * 1.7)
        let digitW = digitH * 0.52
        let t = digitH * 0.1
        let off = tint.mixed(with: Tint(white: 0.25), 0.55).shaded(0.6).color
        for d in 0..<digits {
            let cx = -body.width / 2 + cell * (CGFloat(d) + 0.5), top = -digitH / 2
            let l = cx - digitW / 2, r = cx + digitW / 2, half = digitH / 2
            var segments = Path()
            // a, g, d across; f, b, e, c down.
            for y in [top, top + half, top + digitH] {
                segments.addRoundedRect(in: CGRect(x: l + t * 0.6, y: y - t / 2, width: digitW - 1.2 * t, height: t),
                                        cornerSize: CGSize(width: t / 2, height: t / 2))
            }
            for x in [l, r] {
                for y in [top, top + half] {
                    segments.addRoundedRect(in: CGRect(x: x - t / 2, y: y + t * 0.6, width: t, height: half - 1.2 * t),
                                            cornerSize: CGSize(width: t / 2, height: t / 2))
                }
            }
            segments.addEllipse(in: CGRect(x: r + t * 0.6, y: top + digitH - t * 0.6, width: t * 1.2, height: t * 1.2))
            layer.fill(segments, with: .color(off))
        }
    }
}
