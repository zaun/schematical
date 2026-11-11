import CoreGraphics

/// What a board is: a free rectangle, or a standard add-in card whose
/// outline has the card-edge tab, key notches and gold fingers of its bus.
/// Raw values are the file values.
nonisolated enum BoardType: String, CaseIterable, Hashable, Sendable {
    case custom
    case customCard
    case isa8
    case isa16
    case pci
    case nes
    case atari2600
    case atari5200
    case atari7800
    case atari8bit
    case atariST
    case jaguar
    case lynx
    case coco
    case pcie
    case m2
    case miniPCIe
    case sodimm
    case dimm
    case piHat
    case arduinoShield

    var title: String {
        switch self {
        case .custom: "Custom"
        case .customCard: "Custom Card"
        case .isa8: "ISA 8-bit"
        case .isa16: "ISA 16-bit"
        case .pci: "PCI (32-bit Universal)"
        case .nes: "NES Cartridge"
        case .atari2600: "Atari 2600 Cartridge"
        case .atari5200: "Atari 5200 Cartridge"
        case .atari7800: "Atari 7800 Cartridge"
        case .atari8bit: "Atari 400/800/XL/XE Cartridge"
        case .atariST: "Atari ST Cartridge"
        case .jaguar: "Atari Jaguar Cartridge"
        case .lynx: "Atari Lynx Card"
        case .coco: "TRS-80 CoCo Cartridge"
        case .pcie: "PCI Express"
        case .m2: "M.2"
        case .miniPCIe: "Mini PCIe"
        case .sodimm: "SO-DIMM"
        case .dimm: "DIMM"
        case .piHat: "Raspberry Pi HAT"
        case .arduinoShield: "Arduino Uno Shield"
        }
    }

    /// Whether the type has settings of its own (`CardOptions`), shown in
    /// the sidebar's Card section.
    var hasOptions: Bool { [.pcie, .m2, .miniPCIe, .sodimm, .dimm, .arduinoShield].contains(self) }

    /// Whether its settings change the pins a bus of this type carries. An
    /// Arduino shield's ICSP header is on the board only, not the bus.
    var hasBusOptions: Bool { hasOptions && self != .arduinoShield }

    /// The card's spec with `options`: the types with settings build it
    /// from them; the rest are `card`.
    func card(_ options: CardOptions) -> CardSpec? {
        switch self {
        case .pcie: .pcie(lanes: options.pcieLanes, lowProfile: options.lowProfile)
        case .m2: .m2(length: options.m2Length, key: options.m2Key)
        case .miniPCIe: .miniPCIe(half: options.halfSize)
        case .sodimm: .sodimm(options.memory)
        case .dimm: .dimm(options.memory, registered: options.registered)
        case .arduinoShield: .arduinoShield(icsp: options.icsp)
        default: card
        }
    }

    /// A standard card's spec; nil for a custom board, and for a custom
    /// card, whose spec comes from its settings (`BoardSettings.card`).
    var card: CardSpec? {
        switch self {
        case .custom, .customCard: nil
        case .isa8: .isa(sixteenBit: false)
        case .isa16: .isa(sixteenBit: true)
        case .pci: .pci
        case .nes: .nes
        case .atari2600: .atari2600
        case .atari5200: .atari5200
        case .atari7800: .atari7800
        case .atari8bit: .atari8bit
        case .atariST: .atariST
        case .jaguar: .jaguar
        case .lynx: .lynx
        case .coco: .coco
        case .piHat: .piHat
        case .pcie, .m2, .miniPCIe, .sodimm, .dimm, .arduinoShield: card(CardOptions())
        }
    }

    /// Whether the board has a connector whose pins a bus can carry: a
    /// card's edge tab with fingers, or a HAT's or shield's header.
    var isCard: Bool { self != .custom }
}

/// The settings of the card types that have them: PCI Express lanes and
/// bracket height, M.2 length and key, Mini PCIe size, the memory
/// generation of a SO-DIMM or DIMM, and an Arduino shield's ICSP header.
/// Raw values are the file values.
nonisolated struct CardOptions: Hashable, Sendable {
    var pcieLanes = 1
    /// A low-profile bracket (a shorter card).
    var lowProfile = false
    /// mm: 30, 42, 60, 80 or 110.
    var m2Length = 80
    var m2Key: M2Key = .m
    /// A half-size Mini PCIe card.
    var halfSize = false
    var memory: MemoryGeneration = .ddr4
    /// A registered DDR5 DIMM (RDIMM), whose key sits further right than
    /// an unbuffered one's. Other modules key the same either way.
    var registered = false
    /// An Arduino shield's 2 × 3 ICSP header holes.
    var icsp = false

    static let pcieLaneCounts = [1, 4, 8, 16]
    static let m2Lengths = [30, 42, 60, 80, 110]
}

/// Which M.2 key notches a card has.
nonisolated enum M2Key: String, CaseIterable, Hashable, Sendable {
    case b, m, bm, e, ae

    var title: String {
        switch self {
        case .b: "B"
        case .m: "M"
        case .bm: "B+M"
        case .e: "E"
        case .ae: "A+E"
        }
    }
}

nonisolated enum MemoryGeneration: String, CaseIterable, Hashable, Sendable {
    case ddr3, ddr4, ddr5

    var title: String { rawValue.uppercased() }
}

/// A card edge the user builds: one tab of evenly spaced fingers, on Top
/// (A1, A2, …) and, when `bothSides`, on Bottom (B1, B2, …), numbered from
/// the left. Lengths in mm. `clamped()` keeps the parts fitting together.
nonisolated struct CustomCard: Hashable, Sendable {
    /// Fingers on each side.
    var pads = 20
    /// Finger centre to centre.
    var pitch = 2.54
    var padWidth = 1.524
    var padLength = 7.62
    /// From the card's bottom edge up to the fingers.
    var padOffset = 0.0
    /// How far the tab reaches below the card's body.
    var depth = 7.62
    /// From the card's left edge to the tab.
    var indent = 10.0
    /// From each end of the tab to the centre of its outermost finger.
    var margin = 2.54
    /// The tab's lower corners.
    var chamfer = 0.5
    var bothSides = true

    static let padRange = 1...200
    static let pitchRange: ClosedRange<Double> = 0.5...10
    static let depthRange: ClosedRange<Double> = 1...50
    static let indentRange: ClosedRange<Double> = 0...500

    /// The tab's length along the card.
    var tabLength: Double { 2 * margin + Double(pads - 1) * pitch }

    /// Every setting in range: a finger narrower than its pitch, fingers
    /// on the tab, margins at least half a finger, a chamfer that fits.
    func clamped() -> CustomCard {
        var card = self
        card.pads = min(max(pads, Self.padRange.lowerBound), Self.padRange.upperBound)
        card.pitch = min(max(pitch, Self.pitchRange.lowerBound), Self.pitchRange.upperBound)
        card.padWidth = min(max(padWidth, 0.1), card.pitch - 0.05)
        card.depth = min(max(depth, Self.depthRange.lowerBound), Self.depthRange.upperBound)
        card.padOffset = min(max(padOffset, 0), card.depth - 0.1)
        card.padLength = min(max(padLength, 0.1), card.depth - card.padOffset)
        card.indent = min(max(indent, Self.indentRange.lowerBound), Self.indentRange.upperBound)
        card.margin = min(max(margin, card.padWidth / 2), 100)
        card.chamfer = min(max(chamfer, 0), min(card.depth, card.margin - card.padWidth / 2))
        return card
    }

    /// The card's spec for a card `height` high.
    func spec(height: Double) -> CardSpec {
        let card = clamped()
        let bottom = height
        var fingers: [EdgeFinger] = []
        for n in 1...card.pads {
            let centre = card.indent + card.margin + Double(n - 1) * card.pitch
            let rect = CGRect(x: centre - card.padWidth / 2, y: bottom - card.padOffset - card.padLength,
                              width: card.padWidth, height: card.padLength)
            fingers.append(EdgeFinger(name: "A\(n)", rect: rect, side: .top))
            if card.bothSides { fingers.append(EdgeFinger(name: "B\(n)", rect: rect, side: .bottom)) }
        }
        let end = card.indent + card.tabLength
        return CardSpec(height: height, tabDepth: card.depth, length: max(100, (end + 10).rounded(.up)),
                        minimumLength: end, maximumLength: 1000,
                        tabs: [CardSpec.Tab(start: card.indent, end: end, leftChamfer: card.indent > 0 ? card.chamfer : 0,
                                            rightChamfer: card.chamfer)],
                        notches: [], fingers: fingers)
    }
}

/// One pin of the board's connector, named as the bus names it ("A1",
/// "B31", "40", "D13"): a card's gold finger, a copper pad on Top
/// (component side) or Bottom (solder side); or a plated through-hole pad
/// of a header (`drill` > 0), on every layer, round or (pin 1) square.
nonisolated struct EdgeFinger: Hashable, Sendable {
    let name: String
    /// mm on the board.
    let rect: CGRect
    /// A finger's side; a header pad is on both.
    let side: BoardSide
    /// mm; 0 for a finger.
    var drill: Double = 0
    /// A round header pad; false is square.
    var round = false

    /// A header pad through every layer.
    var isThroughHole: Bool { drill > 0 }

    /// Whether its copper is on `layer`.
    func isOn(_ layer: CopperLayer) -> Bool { isThroughHole || side.copperLayer == layer }

    /// Where a trace meets it: the middle of a finger's inner end (away
    /// from the card edge), or a header pad's centre.
    var contact: CGPoint {
        isThroughHole ? CGPoint(x: rect.midX, y: rect.midY) : CGPoint(x: rect.midX, y: rect.minY)
    }

    /// As a part's pad, for connectivity and drawing.
    var pad: PlacedPad {
        PlacedPad(designator: name, rect: rect, drill: drill, shape: round ? .round : .rect, radius: 0,
                  chamfer: 0, corners: [])
    }

    /// A header pad `size` square (or round) about `centre`.
    static func header(_ name: String, centre: CGPoint, size: Double = 1.8, drill: Double = 1.1,
                       round: Bool = true) -> EdgeFinger {
        EdgeFinger(name: name, rect: CGRect(x: centre.x - size / 2, y: centre.y - size / 2, width: size, height: size),
                   side: .top, drill: drill, round: round)
    }
}

/// A non-plated mounting hole of a board standard.
nonisolated struct MountingHole: Hashable, Sendable {
    /// mm on the board.
    let centre: CGPoint
    let drill: Double

    /// As a non-plated pad, for connectivity (a Plane is cut round it) and
    /// drawing.
    var pad: PlacedPad {
        PlacedPad(designator: nil, rect: CGRect(x: centre.x - drill / 2, y: centre.y - drill / 2, width: drill,
                                                height: drill),
                  drill: drill, shape: .round, radius: 0, chamfer: 0, corners: [], plated: false)
    }
}

/// A card standard's shape, in board mm with the bracket (rear) edge at
/// x = 0, the card's top edge at y = 0 and the edge tab along the bottom.
/// The board is seen from the component side (Top), so the bracket is on
/// the left. Numbers are from the IBM PC/XT Technical Reference and the
/// EISA specification (ISA), and the PCI Local Bus Specification 2.2 / 3.0.
nonisolated struct CardSpec: Hashable, Sendable {
    /// One downward part of the bottom edge (a tab, or PCI's bracket foot),
    /// from `start` to `end` along the card, reaching the card's bottom.
    /// Its lower corners are cut by `chamfer` (`leftChamfer` on the
    /// bracket side).
    struct Tab: Hashable, Sendable {
        let start: Double
        let end: Double
        var leftChamfer: Double
        var rightChamfer: Double
        /// How far below the card's bottom edge it reaches, when less than
        /// the card's full tab depth (a foot or key block).
        var depth: Double? = nil
    }

    /// A key notch cut up into a tab from the card's bottom: `width` wide
    /// about `centre`, `depth` high.
    struct Notch: Hashable, Sendable {
        let centre: Double
        let width: Double
        let depth: Double
        /// A full-round end instead of a square one.
        var rounded = false
    }

    /// Card height including the tab.
    let height: Double
    /// How far the tab reaches below the card's bottom edge.
    let tabDepth: Double
    /// The length a new card of this type gets.
    let length: Double
    /// The shortest card that still clears its edge connector.
    let minimumLength: Double
    /// A full-length card.
    let maximumLength: Double
    let tabs: [Tab]
    let notches: [Notch]
    let fingers: [EdgeFinger]
    /// A cartridge's whole outline, traced from its board, used instead of
    /// the body-plus-tabs shape. Cartridges have a fixed size.
    var fixedOutline: [CGPoint]? = nil
    /// The standard's mounting holes (a HAT's or shield's).
    var holes: [MountingHole] = []
    /// Header pads that aren't the connector's pins, so no bus carries
    /// them (an Arduino shield's ICSP header), named "ICSP.1" and so on.
    var extraPads: [EdgeFinger] = []
    /// The pin names in the connector's own order, when natural order
    /// isn't it (an Arduino shield's D0…D13, A0…A5, power pins).
    var pinOrder: [String]? = nil
    /// What the connector's pins are labelled in lists ("Edge.A1",
    /// "Header.40").
    var connectorName = "Edge"

    /// Whether the card's length can't change (a cartridge).
    var isFixedSize: Bool { minimumLength == maximumLength }

    /// The card's bottom edge, above the tab.
    var mainEdge: Double { height - tabDepth }

    /// The outline for a card `length` long, clockwise from the top-left:
    /// along the top, down the front end, then right to left along the
    /// bottom edge, round each tab and notch, and up the bracket edge.
    func outline(length: Double) -> [CGPoint] {
        if let fixedOutline { return fixedOutline }
        var points = [CGPoint(x: 0, y: 0), CGPoint(x: length, y: 0), CGPoint(x: length, y: mainEdge)]
        let cardBottom = height
        var endsAtBracket = false
        for tab in tabs.sorted(by: { $0.start > $1.start }) {
            let bottom = tab.depth.map { mainEdge + $0 } ?? cardBottom
            points.append(CGPoint(x: tab.end, y: mainEdge))
            points.append(CGPoint(x: tab.end, y: bottom - tab.rightChamfer))
            if tab.rightChamfer > 0 { points.append(CGPoint(x: tab.end - tab.rightChamfer, y: bottom)) }
            for notch in notches.sorted(by: { $0.centre > $1.centre })
            where notch.centre > tab.start && notch.centre < tab.end {
                let half = notch.width / 2
                let right = notch.centre + half, left = notch.centre - half
                points.append(CGPoint(x: right, y: bottom))
                if notch.rounded {
                    points += Self.arc(CGPoint(x: notch.centre, y: bottom - notch.depth + half), half,
                                       from: 0, to: -180)
                } else {
                    points += [CGPoint(x: right, y: bottom - notch.depth), CGPoint(x: left, y: bottom - notch.depth)]
                }
                points.append(CGPoint(x: left, y: bottom))
            }
            if tab.leftChamfer > 0 { points.append(CGPoint(x: tab.start + tab.leftChamfer, y: bottom)) }
            points.append(CGPoint(x: tab.start, y: bottom - tab.leftChamfer))
            if tab.start > 0 {
                points.append(CGPoint(x: tab.start, y: mainEdge))
            } else {
                endsAtBracket = true
            }
        }
        if !endsAtBracket { points.append(CGPoint(x: 0, y: mainEdge)) }
        return points
    }

    // MARK: ISA

    /// ISA (PC/XT 8-bit, PC/AT 16-bit). Fingers 0.060" wide at 0.100"
    /// pitch, the full 0.300" tab height; A and C on the component side,
    /// B and D on the solder side, A1/B1 nearest the bracket. The 8-bit tab
    /// starts 0.705" from the bracket edge (PC/XT); the 16-bit section
    /// follows a 0.215" key notch.
    static func isa(sixteenBit: Bool) -> CardSpec {
        let inch = 25.4
        let height = (sixteenBit ? 4.8 : 4.2) * inch
        let tabDepth = 0.3 * inch
        let pitch = 0.1 * inch, fingerWidth = 0.06 * inch
        let chamfer = 0.06 * inch
        func finger(_ name: String, at centre: Double, _ side: BoardSide) -> EdgeFinger {
            EdgeFinger(name: name, rect: CGRect(x: centre - fingerWidth / 2, y: height - tabDepth,
                                                width: fingerWidth, height: tabDepth), side: side)
        }
        var fingers: [EdgeFinger] = []
        for n in 1...31 {
            let centre = 0.8 * inch + Double(n - 1) * pitch
            fingers.append(finger("A\(n)", at: centre, .top))
            fingers.append(finger("B\(n)", at: centre, .bottom))
        }
        var tabs = [Tab(start: 0.705 * inch, end: 3.895 * inch, leftChamfer: chamfer, rightChamfer: chamfer)]
        if sixteenBit {
            for n in 1...18 {
                let centre = 4.2 * inch + Double(n - 1) * pitch
                fingers.append(finger("C\(n)", at: centre, .top))
                fingers.append(finger("D\(n)", at: centre, .bottom))
            }
            tabs.append(Tab(start: 4.110 * inch, end: 5.990 * inch, leftChamfer: chamfer, rightChamfer: chamfer))
        }
        // Keep-clear past the connector: 0.145" past an 8-bit tab, 2.250"
        // past the notch's datum for a 16-bit card.
        let minimum = sixteenBit ? (3.895 + 2.25) * inch : (3.895 + 0.145) * inch
        return CardSpec(height: height, tabDepth: tabDepth, length: 6.875 * inch,
                        minimumLength: minimum.rounded(.up), maximumLength: 13.13 * inch,
                        tabs: tabs, notches: [], fingers: fingers)
    }

    // MARK: PCI

    /// PCI 32-bit, Universal (keyed for 3.3 V and 5 V slots). Fingers
    /// 1.016 mm wide at 1.27 mm pitch, 2.413 to 6.89 mm above the card's
    /// bottom; B on the component side, A on the solder side, position 1
    /// nearest the bracket. Positions 12–13 and 50–51 are key notches and
    /// have no fingers. The tab reaches 12.07 mm below the card edge, and a
    /// 15 mm foot at the bracket end reaches as far.
    static let pci: CardSpec = {
        let height = 106.68, tabDepth = 12.07
        let pitch = 1.27, fingerWidth = 1.016
        let fingerTop = height - 6.89, fingerBottom = height - 2.413
        var fingers: [EdgeFinger] = []
        for n in 1...62 where ![12, 13, 50, 51].contains(n) {
            let centre = 41.605 + Double(n - 1) * pitch
            let rect = CGRect(x: centre - fingerWidth / 2, y: fingerTop, width: fingerWidth,
                              height: fingerBottom - fingerTop)
            fingers.append(EdgeFinger(name: "B\(n)", rect: rect, side: .top))
            fingers.append(EdgeFinger(name: "A\(n)", rect: rect, side: .bottom))
        }
        let chamfer = 0.381
        // The notch depth isn't dimensioned in the spec; 9.4 mm is read off
        // its drawing (past the finger tops, short of the card edge).
        let notches = [Notch(centre: 56.21, width: 1.854, depth: 9.4),
                       Notch(centre: 104.47, width: 1.854, depth: 9.4)]
        return CardSpec(height: height, tabDepth: tabDepth, length: 174.63, minimumLength: 122,
                        maximumLength: 312,
                        tabs: [Tab(start: 0, end: 15, leftChamfer: 0, rightChamfer: chamfer),
                               Tab(start: 40.77, end: 119.91, leftChamfer: chamfer, rightChamfer: chamfer)],
                        notches: notches, fingers: fingers)
    }()

    // MARK: Cartridges

    /// A finger named by its pin number.
    private static func pin(_ number: Int, centre: Double, width: Double, top: Double, bottom: Double,
                            _ side: BoardSide) -> EdgeFinger {
        EdgeFinger(name: "\(number)", rect: CGRect(x: centre - width / 2, y: top, width: width, height: bottom - top),
                   side: side)
    }

    /// A NES cartridge board (100 × 110 mm, the NES-EWROM size; nesdev's
    /// cartridge dimensions). 72 fingers at 2.50 mm pitch, 2.0 mm wide (the
    /// four end ones 3.0 mm, 0.5 mm further out), 12 mm long, 1 mm above
    /// the edge, on a 93.5 mm tab 14.5 mm deep. Pins 1–36 on the label
    /// (component) side, pin 1 at the right; pin N + 36 behind pin N. The
    /// sides have the notches the shell's ribs fit.
    static let nes: CardSpec = {
        let width = 100.0, height = 110.0, depth = 14.5
        let root = height - depth, centre = width / 2
        let top = height - 1 - 12, bottom = height - 1
        var fingers: [EdgeFinger] = []
        for k in 1...36 {
            let offset: Double, padWidth: Double
            switch k {
            case 1: (offset, padWidth) = (44.25, 3.0)
            case 36: (offset, padWidth) = (-44.25, 3.0)
            default: (offset, padWidth) = (41.25 - Double(k - 2) * 2.5, 2.0)
            }
            fingers.append(pin(k, centre: centre + offset, width: padWidth, top: top, bottom: bottom, .top))
            fingers.append(pin(k + 36, centre: centre + offset, width: padWidth, top: top, bottom: bottom, .bottom))
        }
        // Rib notches: 5 deep for 2.5, then 1.5 deep for 6, below a point
        // 25.5 (left) or 28.0 (right) above the tab's root.
        let left = root - 25.5, right = root - 28.0
        let outline: [CGPoint] = [
            CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0),
            CGPoint(x: width, y: right), CGPoint(x: width - 5, y: right), CGPoint(x: width - 5, y: right + 2.5),
            CGPoint(x: width - 1.5, y: right + 2.5), CGPoint(x: width - 1.5, y: right + 8.5),
            CGPoint(x: width, y: right + 8.5),
            CGPoint(x: width, y: root), CGPoint(x: centre + 46.75, y: root),
            CGPoint(x: centre + 46.75, y: height), CGPoint(x: centre - 46.75, y: height),
            CGPoint(x: centre - 46.75, y: root), CGPoint(x: 0, y: root),
            CGPoint(x: 0, y: left + 8.5), CGPoint(x: 1.5, y: left + 8.5), CGPoint(x: 1.5, y: left + 2.5),
            CGPoint(x: 5, y: left + 2.5), CGPoint(x: 5, y: left), CGPoint(x: 0, y: left),
        ]
        return CardSpec(height: height, tabDepth: depth, length: width, minimumLength: width, maximumLength: width,
                        tabs: [], notches: [], fingers: fingers, fixedOutline: outline)
    }()

    /// An Atari 2600 cartridge board (41.3 × 47 mm, as boards that fit the
    /// original shell). 24 fingers at 0.1" pitch, 1.524 mm wide, 6.35 mm
    /// long, 0.25 mm above the edge, on a 32.5 mm tab 15.9 mm deep whose
    /// shoulders are cut at an angle. Components on the pins 13–24 side,
    /// pin 13 at the left; pin N sits behind pin 25 − N. A 2 × 4 mm notch in
    /// each side, 8.5 mm below the top, fits the shell.
    static let atari2600: CardSpec = {
        let width = 41.3, height = 47.0, depth = 15.9, tabWidth = 32.5
        let root = height - depth, centre = width / 2
        let top = height - 0.25 - 6.35, bottom = height - 0.25
        var fingers: [EdgeFinger] = []
        for i in 0..<12 {
            let x = centre - 13.97 + Double(i) * 2.54
            fingers.append(pin(13 + i, centre: x, width: 1.524, top: top, bottom: bottom, .top))
            fingers.append(pin(12 - i, centre: x, width: 1.524, top: top, bottom: bottom, .bottom))
        }
        let tabLeft = centre - tabWidth / 2, tabRight = centre + tabWidth / 2
        let shoulder = 4.6
        let outline: [CGPoint] = [
            CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0),
            CGPoint(x: width, y: 8.5), CGPoint(x: width - 2, y: 8.5), CGPoint(x: width - 2, y: 12.5),
            CGPoint(x: width, y: 12.5),
            CGPoint(x: width, y: root - shoulder), CGPoint(x: tabRight, y: root),
            CGPoint(x: tabRight, y: height), CGPoint(x: tabLeft, y: height), CGPoint(x: tabLeft, y: root),
            CGPoint(x: 0, y: root - shoulder),
            CGPoint(x: 0, y: 12.5), CGPoint(x: 2, y: 12.5), CGPoint(x: 2, y: 8.5), CGPoint(x: 0, y: 8.5),
        ]
        return CardSpec(height: height, tabDepth: depth, length: width, minimumLength: width, maximumLength: width,
                        tabs: [], notches: [], fingers: fingers, fixedOutline: outline)
    }()

    /// A TRS-80 Color Computer Program Pak board (53.34 × 44.45 mm, as the
    /// original Tandy pak). 40 fingers at 0.1" pitch, 1.27 mm wide, 9.525
    /// mm long, 0.445 mm above the edge, across the bottom; odd pins on the
    /// component side with pin 1 at the left, each even pin behind the odd
    /// one before it. The bottom 10.16 mm is 0.254 mm narrower each side
    /// with rounded corners, and a half-round 7.62 mm notch at the top
    /// takes the case screw.
    static let coco: CardSpec = {
        let width = 53.34, height = 44.45, depth = 10.16
        let top = height - 0.445 - 9.525, bottom = height - 0.445
        var fingers: [EdgeFinger] = []
        for i in 0..<20 {
            let x = 2.54 + Double(i) * 2.54
            fingers.append(pin(2 * i + 1, centre: x, width: 1.27, top: top, bottom: bottom, .top))
            fingers.append(pin(2 * i + 2, centre: x, width: 1.27, top: top, bottom: bottom, .bottom))
        }
        // The screw notch, a half circle hanging from the top edge.
        let radius = 3.81, middle = width / 2
        var notch: [CGPoint] = []
        for step in 0...8 {
            let angle = Double.pi * Double(step) / 8
            notch.append(CGPoint(x: middle - radius * cos(angle), y: radius * sin(angle)))
        }
        let inset = 0.254, corner = 1.27, root = height - depth
        let outline: [CGPoint] = [CGPoint(x: 0, y: 0)] + notch + [
            CGPoint(x: width, y: 0), CGPoint(x: width, y: root), CGPoint(x: width - inset, y: root),
            CGPoint(x: width - inset, y: height - corner), CGPoint(x: width - inset - corner, y: height),
            CGPoint(x: inset + corner, y: height), CGPoint(x: inset, y: height - corner),
            CGPoint(x: inset, y: root), CGPoint(x: 0, y: root),
        ]
        return CardSpec(height: height, tabDepth: depth, length: width, minimumLength: width, maximumLength: width,
                        tabs: [], notches: [], fingers: fingers, fixedOutline: outline)
    }()

    // MARK: More Atari cartridges

    /// A cartridge outline: a `width` × `height` body, narrowed at the
    /// bottom to a centred tab `tabWidth` wide and `tabDepth` deep (square
    /// shoulders) whose lower corners are cut by `chamfer`, with `cut`
    /// points inserted along the top edge (left to right) for a notch.
    private static func tabbed(width: Double, height: Double, tabWidth: Double, tabDepth: Double,
                               chamfer: Double = 0, top cut: [CGPoint] = []) -> [CGPoint] {
        let left = (width - tabWidth) / 2, right = left + tabWidth, root = height - tabDepth
        var points = [CGPoint(x: 0, y: 0)] + cut + [CGPoint(x: width, y: 0), CGPoint(x: width, y: root)]
        points += [CGPoint(x: right, y: root), CGPoint(x: right, y: height - chamfer)]
        if chamfer > 0 { points.append(CGPoint(x: right - chamfer, y: height)) }
        if chamfer > 0 { points.append(CGPoint(x: left + chamfer, y: height)) }
        points += [CGPoint(x: left, y: height - chamfer), CGPoint(x: left, y: root), CGPoint(x: 0, y: root)]
        return points
    }

    /// A half-round notch hanging from the top edge, left to right.
    private static func roundNotch(centre: Double, radius: Double) -> [CGPoint] {
        (0...8).map { step in
            let angle = Double.pi * Double(step) / 8
            return CGPoint(x: centre - radius * cos(angle), y: radius * sin(angle))
        }
    }

    private static func cartridge(width: Double, height: Double, tabDepth: Double, fingers: [EdgeFinger],
                                  outline: [CGPoint]) -> CardSpec {
        CardSpec(height: height, tabDepth: tabDepth, length: width, minimumLength: width, maximumLength: width,
                 tabs: [], notches: [], fingers: fingers, fixedOutline: outline)
    }

    /// The Atari 400/800/XL/XE (left-slot) cartridge: 53.8 × 64.8 mm with a
    /// 39.8 × 21.0 mm tab (A8PicoCart). 30 fingers at 0.1" pitch, 1.524 ×
    /// 6.477 mm, 0.57 mm above the edge; 1–15 on the component side from the
    /// left, A–S (no G, I, O, Q) behind them, A behind 1. The 800's right
    /// slot takes the same board.
    static let atari8bit: CardSpec = {
        let width = 53.8, height = 64.8, tabWidth = 39.8
        let left = (width - tabWidth) / 2
        let top = height - 0.57 - 6.477, bottom = height - 0.57
        let letters = ["A", "B", "C", "D", "E", "F", "H", "J", "K", "L", "M", "N", "P", "R", "S"]
        var fingers: [EdgeFinger] = []
        for i in 0..<15 {
            let x = left + 2.105 + Double(i) * 2.54
            fingers.append(pin(i + 1, centre: x, width: 1.524, top: top, bottom: bottom, .top))
            fingers.append(EdgeFinger(name: letters[i], rect: CGRect(x: x - 0.762, y: top, width: 1.524,
                                                                       height: bottom - top), side: .bottom))
        }
        return cartridge(width: width, height: height, tabDepth: 21.0, fingers: fingers,
                         outline: tabbed(width: width, height: height, tabWidth: tabWidth, tabDepth: 21.0,
                                         chamfer: 0.4))
    }()

    /// The Atari 5200 cartridge, small board: 50.8 × 50.8 mm with a 47.625
    /// × 17.78 mm tab (tdididit a52-flashcart). 36 fingers at 0.1" pitch,
    /// 1.524 × 6.477 mm, 0.57 mm above the edge; 1–18 on the component side
    /// from the left, 19–36 behind, 36 behind 1. Pins 18 and 36 (the
    /// interlock) are shorter, 5.207 mm from 1.84 mm above the edge.
    static let atari5200: CardSpec = {
        let width = 50.8, height = 50.8, tabDepth = 17.78
        var fingers: [EdgeFinger] = []
        for k in 1...18 {
            let x = width / 2 - 21.59 + Double(k - 1) * 2.54
            let short = k == 18
            let bottom = height - (short ? 1.84 : 0.57), top = bottom - (short ? 5.207 : 6.477)
            fingers.append(pin(k, centre: x, width: 1.524, top: top, bottom: bottom, .top))
            fingers.append(pin(37 - k, centre: x, width: 1.524, top: top, bottom: bottom, .bottom))
        }
        return cartridge(width: width, height: height, tabDepth: tabDepth, fingers: fingers,
                         outline: tabbed(width: width, height: height, tabWidth: 47.625, tabDepth: tabDepth))
    }()

    /// The Atari 7800 cartridge: 46.99 × 82.55 mm, fingers across the whole
    /// bottom edge, which two 1.588 × 9.525 mm slots split (tdididit
    /// a78-simplecart, fits the original case). 32 fingers at 0.1" pitch
    /// with a skipped place each side: 1, 2, then 3–14 (where the 2600's
    /// 13–24 are), then 15, 16 on the component side from the left; 17–32
    /// behind, 32 behind 1. Stepped notches in both sides fit the shell.
    static let atari7800: CardSpec = {
        let width = 46.99, height = 82.55, centre = width / 2
        let top = height - 6.477, bottom = height
        var fingers: [EdgeFinger] = []
        for k in 1...16 {
            let offset: Double
            switch k {
            case 1: offset = -21.59
            case 2: offset = -19.05
            case 15: offset = 19.05
            case 16: offset = 21.59
            default: offset = -13.97 + Double(k - 3) * 2.54
            }
            fingers.append(pin(k, centre: centre + offset, width: 1.524, top: top, bottom: bottom, .top))
            fingers.append(pin(33 - k, centre: centre + offset, width: 1.524, top: top, bottom: bottom, .bottom))
        }
        // Side profile, from the edge up: full width, then 2.858 in from
        // 33.02, 4.445 in from 34.925 to 39.37, 2.858 in to 49.53.
        func y(_ above: Double) -> Double { height - above }
        func side(_ x: (Double) -> Double) -> [CGPoint] {
            [CGPoint(x: x(0), y: y(49.53)), CGPoint(x: x(2.858), y: y(49.53)), CGPoint(x: x(2.858), y: y(39.37)),
             CGPoint(x: x(4.445), y: y(39.37)), CGPoint(x: x(4.445), y: y(34.925)),
             CGPoint(x: x(2.858), y: y(34.925)), CGPoint(x: x(2.858), y: y(33.02)), CGPoint(x: x(0), y: y(33.02))]
        }
        let rightSide = side { width - $0 }
        let leftSide = Array(side { $0 }.reversed())
        let slotHalf = 1.588 / 2
        var outline = [CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0)] + rightSide + [CGPoint(x: width, y: height)]
        for slot in [centre + 16.986, centre - 16.986] {
            outline += [CGPoint(x: slot + slotHalf, y: height), CGPoint(x: slot + slotHalf, y: height - 9.525),
                        CGPoint(x: slot - slotHalf, y: height - 9.525), CGPoint(x: slot - slotHalf, y: height)]
        }
        outline += [CGPoint(x: 0, y: height)] + leftSide
        return cartridge(width: width, height: height, tabDepth: 9.525, fingers: fingers, outline: outline)
    }()

    /// The Atari ST/STE cartridge: 55.5 × 25.5 mm with a 41.4 × 8.0 mm tab
    /// (pdaehne Atari-ST-5V-Cartridge; there is no original-shell board).
    /// 40 fingers at 2.0 mm pitch, 1.2 × 6.75 mm, 1.25 mm above the edge;
    /// odd pins on the component side with pin 1 at the right, each even
    /// pin behind the odd one before it.
    static let atariST: CardSpec = {
        let width = 55.5, height = 25.5, tabWidth = 41.4, tabDepth = 8.0
        let right = (width + tabWidth) / 2
        let top = height - 1.25 - 6.75, bottom = height - 1.25
        var fingers: [EdgeFinger] = []
        for i in 0..<20 {
            let x = right - 1.7 - Double(i) * 2.0
            fingers.append(pin(2 * i + 1, centre: x, width: 1.2, top: top, bottom: bottom, .top))
            fingers.append(pin(2 * i + 2, centre: x, width: 1.2, top: top, bottom: bottom, .bottom))
        }
        return cartridge(width: width, height: height, tabDepth: tabDepth, fingers: fingers,
                         outline: tabbed(width: width, height: height, tabWidth: tabWidth, tabDepth: tabDepth,
                                         chamfer: 1.27))
    }()

    /// The Atari Jaguar cartridge: about 94 × 49.2 mm with an 82 × 8.9 mm
    /// tab (measured from tursilion jaguar_cart). 54 places at 1.5 mm
    /// pitch on each side, fingers about 0.75 × 10.16 mm from 2 mm above
    /// the edge; places 13 and 14 are a key slot with no fingers, so 52 a
    /// side. Side A is the component side, A1 at the left; B1 behind A1.
    static let jaguar: CardSpec = {
        let width = 94.0, height = 49.2, tabWidth = 82.0, tabDepth = 8.9
        let left = (width - tabWidth) / 2
        let bottom = height - 2.0, top = bottom - 10.16
        var fingers: [EdgeFinger] = []
        for k in 1...54 where k != 13 && k != 14 {
            let x = left + 1.15 + Double(k - 1) * 1.5
            let rect = CGRect(x: x - 0.375, y: top, width: 0.75, height: bottom - top)
            fingers.append(EdgeFinger(name: "A\(k)", rect: rect, side: .top))
            fingers.append(EdgeFinger(name: "B\(k)", rect: rect, side: .bottom))
        }
        // The key slot, the tab's full depth, between places 13 and 14.
        let key = left + 19.9, half = 0.85
        var outline = tabbed(width: width, height: height, tabWidth: tabWidth, tabDepth: tabDepth)
        // Insert the slot into the tab's bottom edge (right to left).
        let bottomLeft = outline.firstIndex(of: CGPoint(x: left, y: height))!
        outline.insert(contentsOf: [CGPoint(x: key + half, y: height), CGPoint(x: key + half, y: height - tabDepth),
                                    CGPoint(x: key - half, y: height - tabDepth), CGPoint(x: key - half, y: height)],
                       at: bottomLeft)
        return cartridge(width: width, height: height, tabDepth: tabDepth, fingers: fingers, outline: outline)
    }()

    /// The Atari Lynx card: 55 × 22 mm, contacts along the whole bottom
    /// edge on the back face only (Karri's TsopCart). 34 fingers at 1/16"
    /// pitch, 1.2 × 5.5 mm, 0.73 mm above the edge (pins 1, 31 and 33 are
    /// 6.0 mm, from 0.18 mm); pin 1 at the left seen from the component
    /// side. A half-round notch in the top edge, 13.5 mm from the left.
    static let lynx: CardSpec = {
        let width = 55.0, height = 22.0
        var fingers: [EdgeFinger] = []
        for k in 1...34 {
            let long = [1, 31, 33].contains(k)
            let bottom = height - (long ? 0.18 : 0.73), top = bottom - (long ? 6.0 : 5.5)
            fingers.append(pin(k, centre: 1.27 + Double(k - 1) * 1.5875, width: 1.2, top: top, bottom: bottom,
                               .bottom))
        }
        let outline = [CGPoint(x: 0, y: 0)] + roundNotch(centre: 13.5, radius: 1.5)
            + [CGPoint(x: width, y: 0), CGPoint(x: width, y: height), CGPoint(x: 0, y: height)]
        return cartridge(width: width, height: height, tabDepth: 7, fingers: fingers, outline: outline)
    }()

    // MARK: Modern expansion cards

    /// A PCI Express add-in card (CEM 2.0 / 4.0): full height 111.15 mm or
    /// low profile 68.90 mm, the bracket at the left. Fingers at 1.0 mm
    /// pitch, 0.70 mm wide, tops 5.60 mm above the edge, from 1.69 mm (the
    /// presence-detect pins A1, B17, B31, B48, B81 from 2.40 mm). Side B is
    /// the component side, A behind it; B1 nearest the bracket. A 1.90 mm
    /// key with a round end between pins 11 and 12, 57.15 mm from the rear
    /// edge. The tab reaches 12.75 mm below the card; at the bracket end a
    /// foot and the block that keeps it out of a PCI slot reach 8.25 mm.
    static func pcie(lanes: Int, lowProfile: Bool) -> CardSpec {
        let height = lowProfile ? 68.90 : 111.15
        let pins = [1: 18, 4: 32, 8: 49, 16: 82][lanes] ?? 18
        let tabEnd = [1: 65.30, 4: 79.30, 8: 96.30, 16: 129.30][lanes] ?? 65.30
        func h(_ above: Double) -> Double { height - above }
        var fingers: [EdgeFinger] = []
        let short: Set<String> = ["A1", "B17", "B31", "B48", "B81"]
        for n in 1...pins {
            let x = 45.65 + Double(n - 1) + (n >= 12 ? 2 : 0)
            for (row, side) in [("B", BoardSide.top), ("A", .bottom)] {
                let name = "\(row)\(n)"
                let bottom = h(short.contains(name) ? 2.40 : 1.69)
                fingers.append(EdgeFinger(name: name, rect: CGRect(x: x - 0.35, y: h(5.60), width: 0.70,
                                                                   height: bottom - h(5.60)), side: side))
            }
        }
        // The tab, with its key; at the bracket end, the foot (to 15 mm) and
        // the block that keeps the card out of a PCI slot, both 8.25 deep.
        return CardSpec(height: height, tabDepth: 12.75, length: 167.65, minimumLength: (tabEnd + 5).rounded(.up),
                        maximumLength: lowProfile ? 167.65 : 312,
                        tabs: [Tab(start: 0, end: 15, leftChamfer: 0, rightChamfer: 0, depth: 8.25),
                               Tab(start: 33.35, end: 41.35, leftChamfer: 0, rightChamfer: 0, depth: 8.25),
                               Tab(start: 45.0, end: tabEnd, leftChamfer: 0.5, rightChamfer: 0.5)],
                        notches: [Notch(centre: 57.15, width: 1.90, depth: 8.40, rounded: true)], fingers: fingers)
    }
    /// Points along an arc about `centre` from `start` to `end` degrees
    /// (y down, so 90° points down), `steps` segments.
    private static func arc(_ centre: CGPoint, _ radius: Double, from start: Double, to end: Double,
                            steps: Int = 6) -> [CGPoint] {
        (0...steps).map { step in
            let angle = (start + (end - start) * Double(step) / Double(steps)) * .pi / 180
            return CGPoint(x: centre.x + radius * cos(angle), y: centre.y + radius * sin(angle))
        }
    }

    /// An M.2 Type 22 card (PCI Express M.2 Specification 1.0): 22 mm wide
    /// and `length` long, contacts along the 22 mm end on a 19.85 mm strip
    /// 4 mm deep. 75 places at 0.5 mm pitch, odd pins on top (pin 1 at the
    /// right) and even ones on the bottom, offset by 0.25 mm; 0.35 mm wide,
    /// from 0.3 mm to 2.0 mm (top) or 2.5 mm (bottom) from the edge. Each key
    /// is a 1.2 mm slot 3.5 mm deep with a round end, and drops its 8 pins.
    /// A 3.5 mm half-circle in the far end takes the screw.
    static func m2(length: Int, key: M2Key) -> CardSpec {
        let height = Double(length), width = 22.0, mid = 11.0
        // From the spec's frame (x from the centreline, y in from the edge).
        func p(_ x: Double, _ y: Double) -> CGPoint { CGPoint(x: mid + x, y: height - y) }
        let keys: [(centre: Double, pins: ClosedRange<Int>)] = {
            let a = (6.625, 8...15), b = (5.625, 12...19), e = (2.625, 24...31), m = (-6.125, 59...66)
            switch key {
            case .b: return [b]
            case .m: return [m]
            case .bm: return [b, m]
            case .e: return [e]
            case .ae: return [a, e]
            }
        }()
        var fingers: [EdgeFinger] = []
        for n in 1...75 where !keys.contains(where: { $0.pins.contains(n) }) {
            let x = (38 - Double(n)) * 0.25
            let far = n % 2 == 1 ? 2.0 : 2.5
            fingers.append(EdgeFinger(name: "\(n)", rect: CGRect(x: mid + x - 0.175, y: height - far, width: 0.35,
                                                                height: far - 0.3),
                                      side: n % 2 == 1 ? .top : .bottom))
        }
        var outline: [CGPoint] = [CGPoint(x: 0, y: 0)]
        // The screw notch in the far end.
        outline += arc(CGPoint(x: mid, y: 0), 1.75, from: 180, to: 0, steps: 8)
        outline += [CGPoint(x: width, y: 0), p(11, 4), p(10.425, 4)]
        // The strip's inside corner, R0.5 about (10.425, 3.5).
        outline += arc(p(10.425, 3.5), 0.5, from: -90, to: -180, steps: 4).dropFirst()
        outline.append(p(9.925, 0))
        // Key slots, right to left: straight sides to 2.9, a round end.
        for slot in keys.map(\.centre).sorted(by: >) {
            outline += [p(slot + 0.6, 0)]
            outline += arc(p(slot, 2.9), 0.6, from: 0, to: -180, steps: 6)
            outline += [p(slot - 0.6, 0)]
        }
        outline.append(p(-9.925, 0))
        outline += arc(p(-10.425, 3.5), 0.5, from: 0, to: -90, steps: 4)
        outline += [p(-11, 4)]
        return cartridge(width: width, height: height, tabDepth: 4, fingers: fingers, outline: outline)
    }
    /// A PCI Express Mini Card (Mini CEM 1.2): 30 mm wide, 50.95 mm long
    /// (26.80 half size), contacts along a 25.70 mm tab 3.20 mm deep. 52
    /// pins at 0.8 mm pitch, 0.60 × 2.55 mm, 0.25 mm above the edge: odd
    /// pins on top with pin 1 at the left, each even pin on the bottom 0.40
    /// mm right of the odd one before it. A 1.50 mm key with a round end,
    /// 4.00 mm deep, between pins 15 and 17, 11.15 mm from the left.
    static func miniPCIe(half: Bool) -> CardSpec {
        let width = 30.0, height = half ? 26.80 : 50.95
        let top = height - 0.25 - 2.55, bottom = height - 0.25
        var fingers: [EdgeFinger] = []
        for n in stride(from: 1, through: 51, by: 2) {
            let x = n <= 15 ? 3.20 + Double(n - 1) / 2 * 0.8 : 12.80 + Double(n - 17) / 2 * 0.8
            for (number, side, dx) in [(n, BoardSide.top, 0.0), (n + 1, .bottom, 0.40)] {
                fingers.append(pin(number, centre: x + dx, width: 0.60, top: top, bottom: bottom, side))
            }
        }
        let key = 11.15
        var outline = [CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0), CGPoint(x: width, y: height - 3.20),
                       CGPoint(x: 27.85, y: height - 3.20), CGPoint(x: 27.85, y: height)]
        outline += [CGPoint(x: key + 0.75, y: height)]
        outline += arc(CGPoint(x: key, y: height - 4.00 + 0.75), 0.75, from: 0, to: -180, steps: 6)
        outline += [CGPoint(x: key - 0.75, y: height), CGPoint(x: 2.15, y: height),
                    CGPoint(x: 2.15, y: height - 3.20), CGPoint(x: 0, y: height - 3.20)]
        return cartridge(width: width, height: height, tabDepth: 3.20, fingers: fingers, outline: outline)
    }
    /// A memory module's outline, clockwise from the top-left. `edge` is
    /// the finger edge's profile, left to right, as (x, height above its
    /// lowest point); `key` a round-ended slot up from it; `notches` cut
    /// both short sides between two heights above the edge (rectangular,
    /// or half-round when `round`), and the corners are cut by `topChamfer`
    /// and `bottomChamfer`.
    private static func module(width: Double, height: Double, edge: [CGPoint],
                               key: (centre: Double, width: Double, depth: Double),
                               notches: [(from: Double, to: Double, depth: Double)], round: (centre: Double, radius: Double)? = nil,
                               topChamfer: Double, bottomChamfer: Double) -> [CGPoint] {
        func y(_ above: Double) -> Double { height - above }
        let leftRaise = edge.first!.y, rightRaise = edge.last!.y
        var points = [CGPoint(x: topChamfer, y: 0), CGPoint(x: width - topChamfer, y: 0)]
        if topChamfer > 0 { points.append(CGPoint(x: width, y: topChamfer)) }
        // Down the right side.
        if let round {
            points += arc(CGPoint(x: width, y: y(round.centre)), round.radius, from: -90, to: -270)
        }
        for notch in notches.sorted(by: { $0.to > $1.to }) {
            points += [CGPoint(x: width, y: y(notch.to)), CGPoint(x: width - notch.depth, y: y(notch.to)),
                       CGPoint(x: width - notch.depth, y: y(notch.from)), CGPoint(x: width, y: y(notch.from))]
        }
        points.append(CGPoint(x: width, y: y(rightRaise) - bottomChamfer))
        if bottomChamfer > 0 { points.append(CGPoint(x: width - bottomChamfer, y: y(rightRaise))) }
        // Along the edge, right to left, with the key.
        let half = key.width / 2
        for (index, point) in edge.enumerated().reversed() {
            if point.x > bottomChamfer, point.x < width - bottomChamfer {
                points.append(CGPoint(x: point.x, y: y(point.y)))
            }
            if index > 0, edge[index - 1].x < key.centre, key.centre <= point.x {
                points.append(CGPoint(x: key.centre + half, y: height))
                points += arc(CGPoint(x: key.centre, y: y(key.depth) + half), half, from: 0, to: -180)
                points.append(CGPoint(x: key.centre - half, y: height))
            }
        }
        if bottomChamfer > 0 { points.append(CGPoint(x: bottomChamfer, y: y(leftRaise))) }
        points.append(CGPoint(x: 0, y: y(leftRaise) - bottomChamfer))
        // Up the left side.
        for notch in notches.sorted(by: { $0.from < $1.from }) {
            points += [CGPoint(x: 0, y: y(notch.from)), CGPoint(x: notch.depth, y: y(notch.from)),
                       CGPoint(x: notch.depth, y: y(notch.to)), CGPoint(x: 0, y: y(notch.to))]
        }
        if let round {
            points += arc(CGPoint(x: 0, y: y(round.centre)), round.radius, from: 90, to: -90)
        }
        if topChamfer > 0 { points.append(CGPoint(x: 0, y: topChamfer)) }
        return points
    }

    /// The edge's height above its lowest point at `x` (straight between
    /// the profile's points).
    private static func raise(_ edge: [CGPoint], at x: Double) -> Double {
        for (a, b) in zip(edge, edge.dropFirst()) where x >= a.x && x <= b.x {
            return b.x == a.x ? a.y : a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x)
        }
        return edge.last!.y
    }

    /// A SO-DIMM (Micron and socket drawings; JEDEC MO-268, MO-310,
    /// MO-337): odd pins on the front, each even pin behind the odd one
    /// before it, half a pitch to the right. DDR3: 67.6 × 30 mm, 204 pins
    /// at 0.6 mm; DDR4: 69.6 × 30, 260 at 0.5; DDR5: 69.6 × 30, 262 at 0.5.
    /// A 1 mm round-ended key 4 mm deep; notches in both short sides 18–22
    /// mm up (half-round on DDR3).
    static func sodimm(_ memory: MemoryGeneration) -> CardSpec {
        let height = 30.0
        let width: Double, pitch: Double, padWidth: Double, padBottom: Double, padTop: Double
        let pairs: Int, firstAfterKey: Int, start: Double, restart: Double, key: Double
        switch memory {
        case .ddr3:
            (width, pitch, padWidth, padBottom, padTop) = (67.6, 0.6, 0.45, 0, 2.55)
            (pairs, firstAfterKey, start, restart, key) = (102, 37, 2.15, 26.15, 24.8)
        case .ddr4:
            (width, pitch, padWidth, padBottom, padTop) = (69.6, 0.5, 0.35, 0.3, 2.55)
            (pairs, firstAfterKey, start, restart, key) = (130, 73, 1.425, 39.425, 38.3)
        case .ddr5:
            (width, pitch, padWidth, padBottom, padTop) = (69.6, 0.5, 0.35, 0.3, 2.85)
            (pairs, firstAfterKey, start, restart, key) = (131, 64, 1.425, 34.425, 33.55)
        }
        var fingers: [EdgeFinger] = []
        for i in 0..<pairs {
            // `firstAfterKey` counts front pins from 1.
            let x = i + 1 < firstAfterKey ? start + Double(i) * pitch : restart + Double(i + 1 - firstAfterKey) * pitch
            fingers.append(pin(2 * i + 1, centre: x, width: padWidth, top: height - padTop, bottom: height - padBottom,
                               .top))
            fingers.append(pin(2 * i + 2, centre: x + pitch / 2, width: padWidth, top: height - padTop,
                               bottom: height - padBottom, .bottom))
        }
        let edge = [CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0)]
        let outline = memory == .ddr3
            ? module(width: width, height: height, edge: edge, key: (key, 1.0, 4.0), notches: [], round: (20, 2),
                     topChamfer: 0.6, bottomChamfer: 0.6)
            : module(width: width, height: height, edge: edge, key: (key, 1.0, 4.0), notches: [(18, 22, 1.75)],
                     topChamfer: memory == .ddr5 ? 0 : 0.6, bottomChamfer: memory == .ddr5 ? 0.7 : 0.6)
        return cartridge(width: width, height: height, tabDepth: 4, fingers: fingers, outline: outline)
    }

    /// A DIMM (Micron, Samsung, Innodisk, Advantech drawings): 133.35 mm
    /// long, pins 1… on the front from the left and the back's behind them
    /// (121 behind 1 on DDR3, 145 on DDR4/DDR5). DDR3: 30 mm high, 240 pins
    /// at 1.0 mm, a straight edge. DDR4/DDR5: 31.25 mm, 288 pins at 0.85 mm
    /// on an edge 0.5 mm higher at the ends, every finger's top level and
    /// its bottom 0.25 mm above the edge under it. Two notches in each short
    /// side. A DDR5 RDIMM's key sits further right than a UDIMM's.
    static func dimm(_ memory: MemoryGeneration, registered: Bool = false) -> CardSpec {
        let width = 133.35
        let height: Double, pitch: Double, padWidth: Double, perSide: Int, split: Int
        let start: Double, restart: Double, key: (centre: Double, width: Double, depth: Double)
        let edge: [CGPoint], notches: [(from: Double, to: Double, depth: Double)]
        var topChamfer = 0.0, bottomChamfer = 0.0
        switch memory {
        case .ddr3:
            (height, pitch, padWidth, perSide, split, start, restart) = (30, 1.0, 0.8, 120, 49, 5.175, 57.175)
            key = (54.675, 1.52, 3.81)
            edge = [CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0)]
            notches = [(8, 11, 2.2), (15.8, 18.8, 2.2), (21.75, 24.85, 1.35)]
            topChamfer = 0.6
            bottomChamfer = 0.6
        case .ddr4:
            (height, pitch, padWidth, perSide, split, start, restart) = (31.25, 0.85, 0.6, 144, 78, 3.35, 73.90)
            key = (72.25, 1.5, 3.85)
            edge = [CGPoint(x: 0, y: 0.5), CGPoint(x: 32.675, y: 0.5), CGPoint(x: 42.025, y: 0),
                    CGPoint(x: 97.275, y: 0), CGPoint(x: 106.625, y: 0.5), CGPoint(x: width, y: 0.5)]
            notches = [(8, 11, 2.2), (14.6, 17.6, 2.2)]
            topChamfer = 1.25
        case .ddr5:
            (height, pitch, padWidth, perSide, split, start, restart) = (31.25, 0.85, 0.6, 144, 76, 3.35, 72.20)
            key = (registered ? 70.55 : 68.10, 1.5, 3.85)
            edge = [CGPoint(x: 0, y: 0.5), CGPoint(x: 36.08, y: 0.5), CGPoint(x: 45.43, y: 0),
                    CGPoint(x: 101.52, y: 0), CGPoint(x: 110.87, y: 0.5), CGPoint(x: width, y: 0.5)]
            notches = [(8, 11, 2.1), (14.6, 17.6, 2.1)]
            bottomChamfer = 0.8
        }
        var fingers: [EdgeFinger] = []
        for n in 1...perSide {
            let x = n < split ? start + Double(n - 1) * pitch : restart + Double(n - split) * pitch
            let bottom: Double, top: Double
            if memory == .ddr3 {
                (bottom, top) = (0, 2.5)
            } else {
                (bottom, top) = (raise(edge, at: x) + 0.25, 2.85)
            }
            fingers.append(pin(n, centre: x, width: padWidth, top: height - top, bottom: height - bottom, .top))
            fingers.append(pin(n + perSide, centre: x, width: padWidth, top: height - top, bottom: height - bottom,
                               .bottom))
        }
        let outline = module(width: width, height: height, edge: edge, key: key, notches: notches,
                             topChamfer: topChamfer, bottomChamfer: bottomChamfer)
        return cartridge(width: width, height: height, tabDepth: 4, fingers: fingers, outline: outline)
    }

    // MARK: Header boards

    /// A `width` × `height` rectangle with its corners rounded to
    /// `radius`, clockwise from the top-left.
    private static func roundedRectangle(width: Double, height: Double, radius r: Double) -> [CGPoint] {
        arc(CGPoint(x: r, y: r), r, from: 180, to: 270)
            + arc(CGPoint(x: width - r, y: r), r, from: 270, to: 360)
            + arc(CGPoint(x: width - r, y: height - r), r, from: 0, to: 90)
            + arc(CGPoint(x: r, y: height - r), r, from: 90, to: 180)
    }

    /// A board with a header and no edge tab: fixed size, outline and holes.
    private static func headerBoard(width: Double, height: Double, pins: [EdgeFinger], outline: [CGPoint],
                                    holes: [MountingHole], pinOrder: [String]? = nil) -> CardSpec {
        CardSpec(height: height, tabDepth: 0, length: width, minimumLength: width, maximumLength: width,
                 tabs: [], notches: [], fingers: pins, fixedOutline: outline, holes: holes, pinOrder: pinOrder,
                 connectorName: "Header")
    }

    /// A Raspberry Pi HAT (HAT board mechanical specification, Raspberry
    /// Pi 2014/2018): 65 × 56.5 mm with 3 mm corners, four 2.75 mm M2.5
    /// holes 3.5 mm in from each corner (58 × 49 apart), and the 40-way
    /// GPIO header centred 29 mm right of the left holes, on their line.
    /// Seen from the top with the header along the top edge, pin 1 is at
    /// the left, odd pins on the inner row and even ones on the edge row.
    /// The optional camera and display flex cutouts are left out.
    static let piHat: CardSpec = {
        let width = 65.0, height = 56.5, inset = 3.5
        let pitch = 2.54, centre = CGPoint(x: inset + 29, y: inset)
        var pins: [EdgeFinger] = []
        for column in 0..<20 {
            let x = centre.x + (Double(column) - 9.5) * pitch
            let odd = 2 * column + 1
            pins.append(.header("\(odd)", centre: CGPoint(x: x, y: centre.y + pitch / 2), round: odd != 1))
            pins.append(.header("\(odd + 1)", centre: CGPoint(x: x, y: centre.y - pitch / 2)))
        }
        let holes = [CGPoint(x: inset, y: inset), CGPoint(x: width - inset, y: inset),
                     CGPoint(x: inset, y: inset + 49), CGPoint(x: width - inset, y: inset + 49)]
            .map { MountingHole(centre: $0, drill: 2.75) }
        return headerBoard(width: width, height: height, pins: pins,
                           outline: roundedRectangle(width: width, height: height, radius: 3), holes: holes)
    }()

    /// An Arduino Uno R3 shield (SparkFun-Boards.lbr, package ARDUINO_R3):
    /// 2.7 × 2.1 in, the USB end at the left, the far end cut at 45°
    /// (0.1 in at the bottom right, 0.1 in from 1.49 to 1.59 in up, and
    /// 0.06 in at the top right), four 0.125 in holes. Its 32 header pins
    /// at 0.1 in, named as the board's silkscreen names them (bus names
    /// are 4 letters at most: IORF is IOREF, RST is RESET; the three GND
    /// pins are GND1–GND3, NC the unused one): the 10-pin SCL…D8 and 8-pin
    /// D7…D0 headers along the top edge, the 8-pin power header (NC, IORF,
    /// RST, 3V3, 5V, GND1, GND2, VIN) and A0…A5 along the bottom. The
    /// ICSP header (2 × 3, pin 1 MISO square, at the far end by the cut
    /// corner) is there when `icsp` is set: its holes only, ICSP.1 to
    /// ICSP.6, on no bus. The bus carries D0–D13, A0–A5, then SDA, SCL, AREF
    /// and the power pins.
    static func arduinoShield(icsp: Bool) -> CardSpec {
        let inch = 25.4, width = 2.7 * inch, height = 2.1 * inch
        // The library's frame: mils from the bottom-left, y up.
        func at(_ x: Double, _ y: Double) -> CGPoint { CGPoint(x: x * inch / 1000, y: height - y * inch / 1000) }
        var pins: [EdgeFinger] = []
        func row(_ names: [String], from x: Double, step: Double, y: Double) {
            for (i, name) in names.enumerated() {
                pins.append(.header(name, centre: at(x + Double(i) * step, y)))
            }
        }
        row(["SCL", "SDA", "AREF", "GND3", "D13", "D12", "D11", "D10", "D9", "D8"], from: 740, step: 100, y: 2000)
        row(["D7", "D6", "D5", "D4", "D3", "D2", "D1", "D0"], from: 1800, step: 100, y: 2000)
        row(["NC", "IORF", "RST", "3V3", "5V", "GND1", "GND2", "VIN"], from: 1100, step: 100, y: 100)
        row(["A0", "A1", "A2", "A3", "A4", "A5"], from: 2000, step: 100, y: 100)
        let outline = [at(0, 2100), at(2540, 2100), at(2600, 2040), at(2600, 1590), at(2700, 1490), at(2700, 200),
                       at(2600, 100), at(2600, 0), at(0, 0)]
        let holes = [at(600, 2000), at(2600, 1400), at(2600, 300), at(550, 100)]
            .map { MountingHole(centre: $0, drill: 0.125 * inch) }
        let order = (0...13).map { "D\($0)" } + (0...5).map { "A\($0)" }
            + ["SDA", "SCL", "AREF", "IORF", "RST", "3V3", "5V", "VIN", "GND1", "GND2", "GND3", "NC"]
        var spec = headerBoard(width: width, height: height, pins: pins, outline: outline, holes: holes,
                               pinOrder: order)
        if icsp {
            // MISO VCC / SCK MOSI / RESET GND, in two columns 0.1 in apart,
            // as SparkFun-Boards.lbr places them (mm from the bottom-left).
            spec.extraPads = (0..<6).map { n in
                let centre = CGPoint(x: 63.64 + Double(n % 2) * 2.54, y: height - (30.43 - Double(n / 2) * 2.54))
                return .header("ICSP.\(n + 1)", centre: centre, round: n != 0)
            }
        }
        return spec
    }
}
