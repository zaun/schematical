import Foundation

// Each manufacturer's published design rules, for Settings ▸ DRC Defaults.
// Every set is for a standard 2-layer board with 1 oz copper, from the
// manufacturer's own pages as read in Nov 2026. Where a page gives no value,
// the set uses the fallback in `DesignRules.Fallback`: the most conservative
// value any manufacturer here publishes. Each set's comment names what it
// takes from where.

/// Whose design rules Settings ▸ DRC Defaults shows.
nonisolated enum DRCManufacturer: String, CaseIterable, Hashable, Sendable {
    // Custom first, then the manufacturers by name. Raw values are stored.
    case custom
    case advancedCircuits
    case aisler
    case allpcb
    case elecrow
    case eurocircuits
    case jlcpcb
    case nextpcb
    case oshpark
    case pcbgogo
    case pcbway
    case seeedFusion
    case sunstone

    var title: String {
        switch self {
        case .custom: "Custom"
        case .advancedCircuits: "Advanced Circuits"
        case .aisler: "AISLER"
        case .allpcb: "ALLPCB"
        case .elecrow: "Elecrow"
        case .eurocircuits: "Eurocircuits"
        case .jlcpcb: "JLCPCB"
        case .nextpcb: "NextPCB"
        case .oshpark: "OSH Park"
        case .pcbgogo: "PCBgogo"
        case .pcbway: "PCBWay"
        case .seeedFusion: "Seeed Fusion"
        case .sunstone: "Sunstone"
        }
    }

    /// The manufacturer's published rules; nil for Custom, whose rules are
    /// the user's.
    var fixedRules: DesignRules? {
        switch self {
        case .custom: nil
        case .advancedCircuits: .advancedCircuits
        case .aisler: .aisler
        case .allpcb: .allpcb
        case .elecrow: .elecrow
        case .eurocircuits: .eurocircuits
        case .jlcpcb: .jlcpcb
        case .nextpcb: .nextpcb
        case .oshpark: .oshpark
        case .pcbgogo: .pcbgogo
        case .pcbway: .pcbway
        case .seeedFusion: .seeedFusion
        case .sunstone: .sunstone
        }
    }
}

extension DesignRules {
    /// What a set uses for a value its manufacturer doesn't publish: the
    /// most conservative value published by the manufacturers here.
    enum Fallback {
        /// Elecrow's IC mask dam.
        static let solderMaskDam = 0.25
        /// Sunstone's text height.
        static let silkscreenHeight = 1.27
        /// JLCPCB's and Seeed Fusion's hole to hole.
        static let holeToHole = 0.45
        /// NextPCB's and Seeed Fusion's 6 mil.
        static let silkscreenToPad = 0.1524
        /// Advanced Circuits' 0.0025" per side: the widest opening, so the
        /// mask stays off the pad through the most registration error.
        static let solderMaskExpansion = 0.0635
    }

    /// Advanced Circuits' (4pcb.com, now advancedpcb.com) standard 2-layer,
    /// 1 oz spec (resources/tolerances and services/standard-pcbs). Annular
    /// ring is the component-hole minimum (vias may be 0.127 mm); via
    /// diameter is the hole plus their 0.010" pad rule. Not published, so
    /// the fallbacks: hole to hole, silkscreen text height,
    /// silkscreen to pad and mask dam.
    static let advancedCircuits = DesignRules(
        minTraceWidth: 0.127,
        minClearance: 0.127,
        minDrill: 0.254,
        minViaDrill: 0.254,
        minViaDiameter: 0.508,
        minAnnularRing: 0.1778,
        minHoleToHole: Fallback.holeToHole,
        minCopperToEdge: 0.254,
        minSilkscreenWidth: 0.127,
        minSilkscreenHeight: Fallback.silkscreenHeight,
        minSilkscreenToPad: Fallback.silkscreenToPad,
        minSolderMaskDam: Fallback.solderMaskDam,
        solderMaskExpansion: 0.0635
    )

    /// AISLER's 2-layer 1.6 mm, 35 µm HASL design rules
    /// (community.aisler.net, updated Jul 2025). Annular ring is for plated
    /// holes; via diameter is the via drill plus their 0.2 mm via ring.
    static let aisler = DesignRules(
        minTraceWidth: 0.2,
        minClearance: 0.15,
        minDrill: 0.5,
        minViaDrill: 0.3,
        minViaDiameter: 0.7,
        minAnnularRing: 0.3,
        minHoleToHole: 0.3,
        minCopperToEdge: 0.3,
        minSilkscreenWidth: 0.15,
        minSilkscreenHeight: 0.8,
        minSilkscreenToPad: 0.125,
        minSolderMaskDam: 0.1,
        solderMaskExpansion: 0.05
    )

    /// ALLPCB's 1 oz limits (allpcb.com/advanced_pcb_manufacturing_capability).
    /// Vias are their double-sided 0.2 / 0.5 mm; annular ring is their
    /// plated-hole limit. Not published, so the fallbacks: hole to
    /// hole and mask expansion.
    static let allpcb = DesignRules(
        minTraceWidth: 0.0762,
        minClearance: 0.0762,
        minDrill: 0.15,
        minViaDrill: 0.2,
        minViaDiameter: 0.5,
        minAnnularRing: 0.18,
        minHoleToHole: Fallback.holeToHole,
        minCopperToEdge: 0.3,
        minSilkscreenWidth: 0.1,
        minSilkscreenHeight: 0.75,
        minSilkscreenToPad: 0.15,
        minSolderMaskDam: 0.075,
        solderMaskExpansion: Fallback.solderMaskExpansion
    )

    /// Elecrow's Regular Custom PCB, 2 layer, 1 oz (PCB Specification && FAQ
    /// PDF from elecrow.com). Via diameter is the via drill plus the
    /// 2-layer annular ring. Not published, so the fallbacks:
    /// silkscreen to pad and mask expansion.
    static let elecrow = DesignRules(
        minTraceWidth: 0.15,
        minClearance: 0.15,
        minDrill: 0.3,
        minViaDrill: 0.25,
        minViaDiameter: 0.55,
        minAnnularRing: 0.15,
        minHoleToHole: 0.35,
        minCopperToEdge: 0.7,
        minSilkscreenWidth: 0.15,
        minSilkscreenHeight: 0.8,
        minSilkscreenToPad: Fallback.silkscreenToPad,
        minSolderMaskDam: 0.25,
        solderMaskExpansion: Fallback.solderMaskExpansion
    )

    /// Eurocircuits' pattern class 6 and drill class B, 35 µm outer copper
    /// (the tightest classes their STANDARD pool takes without moving to a
    /// costlier class at 7 / C; eurocircuits.com classification table,
    /// May 2024, and design guideline pages). Drills are finished sizes;
    /// via diameter is the tool size (finished + 0.1 mm) plus two rings.
    static let eurocircuits = DesignRules(
        minTraceWidth: 0.15,
        minClearance: 0.15,
        minDrill: 0.35,
        minViaDrill: 0.35,
        minViaDiameter: 0.7,
        minAnnularRing: 0.125,
        minHoleToHole: 0.25,
        minCopperToEdge: 0.25,
        minSilkscreenWidth: 0.1,
        minSilkscreenHeight: 1.0,
        minSilkscreenToPad: 0.1,
        minSolderMaskDam: 0.075,
        solderMaskExpansion: 0.03
    )

    /// JLCPCB's rigid 1–2 layer, 1 oz limits with no extra charge
    /// (jlcpcb.com/capabilities/pcb-capabilities). Holes under 0.2 mm, and
    /// 0.2 mm vias under 0.45 mm across, cost more, so those are the
    /// minimums. Annular ring is the plated-hole absolute minimum; hole to
    /// hole is for pad holes (vias may be 0.2 mm apart). The mask opening
    /// may match the pad (1:1), so expansion is 0.
    static let jlcpcb = DesignRules(
        minTraceWidth: 0.1,
        minClearance: 0.1,
        minDrill: 0.2,
        minViaDrill: 0.2,
        minViaDiameter: 0.45,
        minAnnularRing: 0.18,
        minHoleToHole: 0.45,
        minCopperToEdge: 0.2,
        minSilkscreenWidth: 0.15,
        minSilkscreenHeight: 1.0,
        minSilkscreenToPad: 0.15,
        minSolderMaskDam: 0.1,
        solderMaskExpansion: 0.0
    )

    /// NextPCB's standard 1 oz outer-layer limits (nextpcb.com/pcb-capabilities).
    /// Vias use the PTH drill; via diameter is that plus two annular rings.
    /// Mask dam is for green.
    static let nextpcb = DesignRules(
        minTraceWidth: 0.0762,
        minClearance: 0.0762,
        minDrill: 0.2,
        minViaDrill: 0.2,
        minViaDiameter: 0.38,
        minAnnularRing: 0.0889,
        minHoleToHole: 0.3048,
        minCopperToEdge: 0.2,
        minSilkscreenWidth: 0.127,
        minSilkscreenHeight: 0.762,
        minSilkscreenToPad: 0.1524,
        minSolderMaskDam: 0.0889,
        solderMaskExpansion: 0.0381
    )

    /// OSH Park's 2-layer service (docs.oshpark.com/services/two-layer).
    /// Via diameter is the min drill plus two annular rings; hole to hole is
    /// their recommended clearance. Not published, so the fallbacks:
    /// silkscreen text height, silkscreen to pad and mask expansion.
    static let oshpark = DesignRules(
        minTraceWidth: 0.1524,
        minClearance: 0.1524,
        minDrill: 0.254,
        minViaDrill: 0.254,
        minViaDiameter: 0.508,
        minAnnularRing: 0.127,
        minHoleToHole: 0.127,
        minCopperToEdge: 0.381,
        minSilkscreenWidth: 0.127,
        minSilkscreenHeight: Fallback.silkscreenHeight,
        minSilkscreenToPad: Fallback.silkscreenToPad,
        minSolderMaskDam: 0.1016,
        solderMaskExpansion: Fallback.solderMaskExpansion
    )

    /// PCBgogo's standard FR-4 limits
    /// (pcbgogo.com/Capabilities/fr-4-pcb-manufacturing.html). Via diameter
    /// is the drill plus two annular rings; hole to hole is their via
    /// distance for different nets; text height is the safer 0.8 mm the
    /// page also gives; expansion is their mask-to-pad distance. Mask dam is
    /// for green.
    static let pcbgogo = DesignRules(
        minTraceWidth: 0.0762,
        minClearance: 0.0762,
        minDrill: 0.2,
        minViaDrill: 0.2,
        minViaDiameter: 0.46,
        minAnnularRing: 0.13,
        minHoleToHole: 0.25,
        minCopperToEdge: 0.3,
        minSilkscreenWidth: 0.15,
        minSilkscreenHeight: 0.8,
        minSilkscreenToPad: 0.1,
        minSolderMaskDam: 0.1,
        solderMaskExpansion: 0.05
    )

    /// PCBWay's standard rigid 1–2 layer, 1 oz limits with no extra charge
    /// (pcbway.com/capabilities.html and PCB_Manufacturing_tolerances).
    /// Values the pages don't give directly: via diameter is the min via
    /// drill plus two annular rings; hole to hole is the advanced table's
    /// 16 mil for component holes. Not published, so the fallback:
    /// silkscreen to pad.
    static let pcbway = DesignRules(
        minTraceWidth: 0.1,
        minClearance: 0.1,
        minDrill: 0.2,
        minViaDrill: 0.2,
        minViaDiameter: 0.5,
        minAnnularRing: 0.15,
        minHoleToHole: 0.4064,
        minCopperToEdge: 0.25,
        minSilkscreenWidth: 0.15,
        minSilkscreenHeight: 0.8,
        minSilkscreenToPad: Fallback.silkscreenToPad,
        minSolderMaskDam: 0.1,
        solderMaskExpansion: 0.05
    )

    /// Seeed Fusion PCB's spec (support.seeed.cc, Fusion PCB Specification,
    /// 2019), 1 oz. Annular ring and hole to hole are for plated holes (vias
    /// 0.1 / 0.3 mm); via diameter is the via drill plus the via ring. Mask
    /// dam is their 0.1 mm option for green (0.32 mm otherwise). Not
    /// published, so the fallback: mask expansion.
    static let seeedFusion = DesignRules(
        minTraceWidth: 0.1016,
        minClearance: 0.1016,
        minDrill: 0.2,
        minViaDrill: 0.2,
        minViaDiameter: 0.4,
        minAnnularRing: 0.1524,
        minHoleToHole: 0.45,
        minCopperToEdge: 0.3,
        minSilkscreenWidth: 0.1016,
        minSilkscreenHeight: 0.5842,
        minSilkscreenToPad: 0.1524,
        minSolderMaskDam: 0.1,
        solderMaskExpansion: Fallback.solderMaskExpansion
    )

    /// Sunstone's Limited Review service, 1 oz finished copper
    /// (sunstone.com/capabilities/detailed-capabilities). Pads are their
    /// "larger than finished hole" rules: 0.016" for through holes (the
    /// annular ring) and 0.010" for vias. Not published, so the app's
    /// fallbacks: hole to hole, silkscreen to pad and mask expansion.
    static let sunstone = DesignRules(
        minTraceWidth: 0.127,
        minClearance: 0.127,
        minDrill: 0.254,
        minViaDrill: 0.254,
        minViaDiameter: 0.508,
        minAnnularRing: 0.2032,
        minHoleToHole: Fallback.holeToHole,
        minCopperToEdge: 0.508,
        minSilkscreenWidth: 0.1524,
        minSilkscreenHeight: 1.27,
        minSilkscreenToPad: Fallback.silkscreenToPad,
        minSolderMaskDam: 0.1016,
        solderMaskExpansion: Fallback.solderMaskExpansion
    )
}
