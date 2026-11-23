import Foundation

/// How a relay part's pins are laid out on its symbol (plan-47-N § 3.1): the
/// coil pins on the left, in coil order, and the contact pins on the right,
/// pole by pole. Built from the part's pin names (the backend's relay entries
/// are not part of the part's public facts, so the names are the key):
///
/// - an electromechanical or reed relay's coil is `A1`, `A2` (a second coil
///   `A3`, `A4`); a latching relay's `S1`, `S2` and `R1`, `R2`, or the
///   three-terminal `S`, `C`, `R` (two coils that share `C`);
/// - a solid-state relay's input LED is `A`, `K` (several channels `A1`,
///   `K1`, `A2`, `K2` …);
/// - a pole is `COM`, `NO`, `NC` (several poles `COM1`, `NO1`, `NC1`,
///   `COM2` …), a solid-state pole `D1`, `D2` or `D1A`, `D1B`: the first is
///   the common, the second the normally-open terminal, or the
///   normally-closed one for a Form B part (its function says `NC` and not
///   `NO`);
/// - a pin of the same name as an earlier one (a power relay's doubled
///   terminal) and every no-connect pin is not drawn.
///
/// Rows are `ChipLayout.pitch` apart. A two-pin coil takes rows `3 k` and
/// `3 k + 2` (a gap row between them gives the coil room to draw); a pole
/// takes its `NO`, `COM` and `NC` rows in that order (the ones it has), a
/// gap row between poles.
nonisolated struct RelayShape {
    /// One coil (or a solid-state relay's input LED), between two pins.
    struct Coil {
        /// The pin at the top and the one at the bottom of the coil, and
        /// their rows.
        let top: ChipPin
        let bottom: ChipPin
        let topRow: Int
        let bottomRow: Int
        /// "S" or "R" on a dual-coil latching relay; nil otherwise.
        let label: String?
    }

    /// One pole: the common pin and the contact it makes at rest (`nc`) and
    /// when operated (`no`); a Form A pole has no `nc`, a Form B pole no
    /// `no`.
    struct Pole {
        let common: ChipPin
        let no: ChipPin?
        let nc: ChipPin?
        let noRow: Int?
        let commonRow: Int
        let ncRow: Int?
        /// The channel that moves the pole: an index into the part's lights
        /// (0 for a latching relay).
        let channel: Int
    }

    let coils: [Coil]
    let poles: [Pole]
    /// The coil pins' and the contact pins' rows, by pin index.
    let leftRows: [Int: Int]
    let rightRows: [Int: Int]
    /// The input is an LED, not a coil.
    let isSolidState: Bool
    let isLatching: Bool

    /// The most rows a relay symbol draws; a relay with more falls back to
    /// the block layout (`symbol(for:)`).
    static let maxRows = 16

    var rowCount: Int {
        max((leftRows.values.max() ?? -1) + 1, (rightRows.values.max() ?? -1) + 1)
    }

    /// The pins drawn, once each, with the side they are on.
    var leftPins: [(pin: ChipPin, row: Int)] { pinsByRow(leftPins: true) }
    var rightPins: [(pin: ChipPin, row: Int)] { pinsByRow(leftPins: false) }

    private let pinTable: [Int: ChipPin]

    private func pinsByRow(leftPins: Bool) -> [(pin: ChipPin, row: Int)] {
        (leftPins ? leftRows : rightRows).compactMap { index, row in
            pinTable[index].map { (pin: $0, row: row) }
        }.sorted { $0.row < $1.row }
    }

    /// The shape of a relay part; nil when its pins do not read as a relay
    /// or it needs more than `maxRows` rows.
    init?(part: CatalogPart) {
        guard part.kind == PART_RELAY, let info = part.relay else { return nil }
        self.init(pins: part.pins.filter { !$0.isHidden }, info: info, function: part.function)
    }

    init?(pins: [ChipPin], info: RelayInfo, function: String) {
        let solid = info.type == "solid-state"
        var table: [Int: ChipPin] = [:]
        for pin in pins { table[pin.index] = pin }

        /// The first pin of each name, in pinout order, of a role.
        func firstPins(_ role: ChipPinRole) -> [ChipPin] {
            var seen = Set<String>()
            return pins.sorted { $0.index < $1.index }
                .filter { $0.role == role && seen.insert($0.name).inserted }
        }
        let inputs = firstPins(.input)
        func input(_ name: String) -> ChipPin? { inputs.first { $0.name == name } }

        // Coils: pairs of pins; `label` names the set and reset coil.
        var pairs: [(ChipPin, ChipPin, String?)] = []
        if solid {
            if let a = input("A"), let k = input("K") {
                pairs.append((a, k, nil))
            } else {
                var n = 1
                while let a = input("A\(n)"), let k = input("K\(n)") {
                    pairs.append((a, k, nil))
                    n += 1
                }
            }
        } else if let s = input("S"), let c = input("C"), let r = input("R") {
            pairs = [(s, c, "S"), (c, r, "R")]
        } else if let s1 = input("S1"), let s2 = input("S2") {
            pairs.append((s1, s2, "S"))
            if let r1 = input("R1"), let r2 = input("R2") { pairs.append((r1, r2, "R")) }
        } else {
            var n = 1
            while let a = input("A\(n)"), let b = input("A\(n + 1)") {
                pairs.append((a, b, nil))
                n += 2
            }
            // A lone `A1`/`A2` coil is the common case; no `A` pins is no relay.
        }
        guard !pairs.isEmpty else { return nil }
        // Every input pin must be in a coil.
        let coilPins = Set(pairs.flatMap { [$0.0.name, $0.1.name] })
        guard inputs.allSatisfy({ coilPins.contains($0.name) }) else { return nil }

        var coils: [Coil] = []
        var left: [Int: Int] = [:]
        if pairs.count == 2, pairs[0].2 == "S", pairs[0].1.name == "C" {
            // Three terminals: S at row 0, C at 2, R at 4, both coils sharing C.
            let rows = [0, 2, 4]
            for (position, pin) in [pairs[0].0, pairs[0].1, pairs[1].1].enumerated() { left[pin.index] = rows[position] }
            coils = [Coil(top: pairs[0].0, bottom: pairs[0].1, topRow: 0, bottomRow: 2, label: "S"),
                     Coil(top: pairs[1].0, bottom: pairs[1].1, topRow: 2, bottomRow: 4, label: "R")]
        } else {
            for (k, pair) in pairs.enumerated() {
                left[pair.0.index] = 3 * k
                left[pair.1.index] = 3 * k + 2
                coils.append(Coil(top: pair.0, bottom: pair.1, topRow: 3 * k, bottomRow: 3 * k + 2, label: pair.2))
            }
        }

        // Poles, from the contact pins (role inout).
        let contacts = firstPins(.inOut)
        struct Draft { var common: ChipPin?; var no: ChipPin?; var nc: ChipPin?; var channel = 0 }
        var drafts: [Int: Draft] = [:]
        if solid {
            let formB = function.contains("NC") && !function.contains("NO")
            func other(_ draft: inout Draft, _ pin: ChipPin) { if formB { draft.nc = pin } else { draft.no = pin } }
            for pin in contacts {
                let name = pin.name
                if name == "D1" || name == "D2" {
                    var draft = drafts[1] ?? Draft()
                    if name == "D1" { draft.common = pin } else { other(&draft, pin) }
                    drafts[1] = draft
                } else if name.count == 3, name.hasPrefix("D"), let digit = Int(name.dropFirst().prefix(1)),
                          name.hasSuffix("A") || name.hasSuffix("B") {
                    var draft = drafts[digit] ?? Draft()
                    draft.channel = digit - 1
                    if name.hasSuffix("A") { draft.common = pin } else { other(&draft, pin) }
                    drafts[digit] = draft
                }
            }
        } else {
            for pin in contacts {
                for role in ["COM", "NO", "NC"] where pin.name.hasPrefix(role) {
                    let digits = pin.name.dropFirst(role.count)
                    guard digits.allSatisfy(\.isNumber), digits.count <= 2 else { continue }
                    let key = Int(digits) ?? 1
                    var draft = drafts[key] ?? Draft()
                    switch role {
                    case "COM": draft.common = pin
                    case "NO": draft.no = pin
                    default: draft.nc = pin
                    }
                    drafts[key] = draft
                    break
                }
            }
        }
        let ordered = drafts.keys.sorted().compactMap { drafts[$0] }
        guard !ordered.isEmpty, ordered.allSatisfy({ $0.common != nil && ($0.no != nil || $0.nc != nil) }) else { return nil }

        var poles: [Pole] = []
        var right: [Int: Int] = [:]
        var row = 0
        for (position, draft) in ordered.enumerated() {
            let common = draft.common!
            var noRow: Int?
            var ncRow: Int?
            if let no = draft.no {
                noRow = row
                right[no.index] = row
                row += 1
            }
            let commonRow = row
            right[common.index] = row
            row += 1
            if let nc = draft.nc {
                ncRow = row
                right[nc.index] = row
                row += 1
            }
            row += 1
            // The channel: a latching relay has one; else the pole's coil.
            // A solid-state pole carries its own number; an electromechanical
            // relay with several coils shares its poles out in order.
            var channel = 0
            if !info.latching, coils.count > 1 {
                channel = solid ? min(draft.channel, coils.count - 1)
                    : min(position * coils.count / ordered.count, coils.count - 1)
            }
            poles.append(Pole(common: common, no: draft.no, nc: draft.nc, noRow: noRow, commonRow: commonRow,
                              ncRow: ncRow, channel: channel))
        }

        self.coils = coils
        self.poles = poles
        self.leftRows = left
        self.rightRows = right
        self.isSolidState = solid
        self.isLatching = info.latching
        self.pinTable = table
        guard rowCount <= Self.maxRows else { return nil }
    }
}
