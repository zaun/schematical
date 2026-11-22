import CoreGraphics
import CryptoKit
import Foundation

/// File > Save As… (and Save of an untitled window), format KiCad Project:
/// the schematic and its board as a KiCad 8 project, three files that
/// KiCad 8 and later open. See `docs/export.md` § KiCad.
///
/// - `<name>.kicad_pro`: the project (design rules, one net class).
/// - `<name>.kicad_sch`: the schematic. Every part is a generated symbol
///   (a rectangle with its pins on the sides the app draws them), supplies
///   are power symbols, wires, junctions, net labels, No Connects, buses
///   (their lines and ports, with a net label on every wired port pin) and
///   text blocks.
/// - `<name>.kicad_pcb`: the board. Footprints with their pads, traces,
///   vias, Plane zones, holes, board texts and the outline; every pad on a
///   schematic net carries that net, named as KiCad names it.
///
/// The format is lossy (KiCad has no simulation, no part library of this
/// app's, no net classes of these rules), so a save to it is an export:
/// the window's own file and its unsaved-changes state do not change.
@MainActor
enum KiCadExport {
    /// One file of the project: its name and its bytes.
    struct File: Equatable {
        let name: String
        let data: Data

        var text: String { String(decoding: data, as: UTF8.self) }
    }

    /// The library name every generated symbol and footprint lives in.
    static let library = "Schematical"
    /// The extensions of the three files, project first.
    static let extensions = ["kicad_pro", "kicad_sch", "kicad_pcb"]

    /// KiCad millimetres per canvas unit: the app's 10-unit grid is KiCad's
    /// 1.27 mm (50 mil) connection grid, its 20-unit pin pitch 2.54 mm.
    static let mmPerUnit = 0.127

    /// The three files of the project, each named `base` plus its extension.
    static func files(for document: SchematicDocument, base: String) -> [File] {
        let context = Context(document: document, base: base)
        let schematic = KiCadSchematicWriter(context).text()
        let board = KiCadBoardWriter(context).text()
        return [
            File(name: "\(base).kicad_pro", data: Data(project(context).utf8)),
            File(name: "\(base).kicad_sch", data: Data(schematic.utf8)),
            File(name: "\(base).kicad_pcb", data: Data(board.utf8)),
        ]
    }

    /// Writes the project's files into `directory`, replacing files of the
    /// same names.
    static func write(_ document: SchematicDocument, base: String, to directory: URL) throws {
        for file in files(for: document, base: base) {
            try file.data.write(to: directory.appendingPathComponent(file.name), options: .atomic)
        }
    }

    /// A project name from a window's name: anything a file name can't hold
    /// (`/`, `:`) becomes `_`; never empty.
    static func baseName(_ name: String) -> String {
        let cleaned = String(name.map { "/:\\".contains($0) ? "_" : $0 })
            .trimmingCharacters(in: .whitespacesAndNewlines)
        return cleaned.isEmpty ? "Untitled" : cleaned
    }

    // MARK: Project

    /// The `.kicad_pro` JSON: the design rules and the Default net class.
    private static func project(_ context: Context) -> String {
        let rules = context.document.board.rules
        func n(_ value: Double) -> String { KiCad.n(value) }
        return """
        {
          "board": {
            "design_settings": {
              "defaults": {
                "board_outline_line_width": 0.1
              },
              "rule_severities": {
                "lib_footprint_issues": "ignore",
                "lib_footprint_mismatch": "ignore"
              },
              "rules": {
                "min_clearance": \(n(rules.minClearance)),
                "min_copper_edge_clearance": \(n(rules.minCopperToEdge)),
                "min_hole_to_hole": \(n(rules.minHoleToHole)),
                "min_text_height": \(n(rules.minSilkscreenHeight)),
                "min_through_hole_diameter": \(n(rules.minDrill)),
                "min_track_width": \(n(rules.minTraceWidth)),
                "min_via_annular_width": \(n(rules.minAnnularRing)),
                "min_via_diameter": \(n(rules.minViaDiameter))
              }
            }
          },
          "erc": {
            "erc_exclusions": [],
            "meta": {
              "version": 0
            },
            "rule_severities": {
              "footprint_link_issues": "ignore",
              "lib_symbol_issues": "ignore",
              "lib_symbol_mismatch": "ignore"
            }
          },
          "meta": {
            "filename": \(KiCad.json("\(context.base).kicad_pro")),
            "version": 1
          },
          "net_settings": {
            "classes": [
              {
                "bus_width": 12,
                "clearance": \(n(rules.minClearance)),
                "diff_pair_gap": 0.25,
                "diff_pair_via_gap": 0.25,
                "diff_pair_width": 0.2,
                "line_style": 0,
                "microvia_diameter": 0.3,
                "microvia_drill": 0.1,
                "name": "Default",
                "pcb_color": "rgba(0, 0, 0, 0.000)",
                "schematic_color": "rgba(0, 0, 0, 0.000)",
                "track_width": \(n(max(rules.minTraceWidth, 0.25))),
                "via_diameter": \(n(max(rules.minViaDiameter, 0.8))),
                "via_drill": \(n(max(rules.minViaDrill, 0.4))),
                "wire_width": 6
              }
            ],
            "meta": {
              "version": 3
            },
            "net_colors": null,
            "netclass_assignments": null,
            "netclass_patterns": []
          },
          "sheets": [
            [
              \(KiCad.json(context.rootUUID)),
              "Root"
            ]
          ],
          "text_variables": {}
        }

        """
    }

    // MARK: Shared state

    /// What the three writers share: the document, the project name, the
    /// schematic's nets and the label text each net carries.
    struct Context {
        let document: SchematicDocument
        let base: String
        let rootUUID: String
        /// The connector symbol's uuid, which its footprint's path names.
        var edgeSymbolUUID: String { KiCad.uuid("edge/\(base)") }
        let netlist: SchematicNetlist
        /// The text of the net label KiCad gets on each wired bus port pin
        /// (by bus wire id): the wire's name, made unique where the app keeps
        /// two wires of one name apart.
        let busText: [UUID: String]
        /// The label text a net carries, by `netlist` net index: the first
        /// net label or bus port pin wired onto it, in drawing order.
        let labelText: [Int: String]
        /// The board's copper and nets (`SchematicDocument.connectivity()`).
        let copper: BoardConnectivity
        /// The card's connector pins, each once, in order: the name and the
        /// net on it (nil when none). Empty on a plain board. The schematic
        /// gets a symbol for them, the board a footprint.
        let edgePins: [(name: String, net: Int?)]
        /// The index in `copper.pads` of the connector's first pad.
        let edgeFirstPad: Int

        /// A supply's rail name: "GND", "5V", "3V3".
        static func railName(mv: UInt32) -> String {
            mv == 0 ? "GND" : SupplyChoice.name(volts: Double(mv) / 1000) ?? "\(mv)mV"
        }

        /// The rail net `net` is, if it is one.
        func rail(ofNet net: Int) -> String? {
            copper.railNets.filter { $0.value == net }.keys.min().map(Self.railName(mv:))
        }

        init(document: SchematicDocument, base: String) {
            self.document = document
            self.base = base
            rootUUID = KiCad.uuid("root/\(base)")
            let copper = document.connectivity()
            self.copper = copper
            let first = document.chips.filter { $0.board?.side != nil }.reduce(0) { $0 + $1.boardPads.count }
            edgeFirstPad = first
            var pins: [(name: String, net: Int?)] = []
            var seen = Set<String>()
            for (offset, finger) in (document.board.fingers + document.board.extraPads).enumerated() {
                let net = copper.netOfPad[first + offset]
                if seen.insert(finger.name).inserted {
                    pins.append((finger.name, net))
                } else if let at = pins.firstIndex(where: { $0.name == finger.name }), pins[at].net == nil {
                    pins[at].net = net
                }
            }
            edgePins = pins
            netlist = NetlistBuilder.build(chips: document.chips, wires: document.wires, buses: document.buses,
                                           labels: document.netLabels)
            busText = Self.busText(buses: document.buses, labels: document.netLabels)
            var text: [Int: String] = [:]
            let labels = Dictionary(document.netLabels.map { ($0.id, $0) }, uniquingKeysWith: { first, _ in first })
            let order = Dictionary(document.netLabels.enumerated().map { ($1.id, $0) }, uniquingKeysWith: { first, _ in first })
            // Labels by drawing order first, then bus wires.
            var found: [(rank: Int, net: Int, text: String)] = []
            for wire in document.wires {
                guard let net = netlist.netOfWire[wire.id] else { continue }
                for end in [wire.start, wire.end] {
                    switch end {
                    case let .label(id):
                        if let label = labels[id], label.isNet, let rank = order[id] {
                            found.append((rank, net, label.name))
                        }
                    case let .busPin(ref):
                        if let name = busText[ref.wireID] { found.append((Int.max, net, name)) }
                    default: break
                    }
                }
            }
            for item in found.sorted(by: { $0.rank < $1.rank }) where text[item.net] == nil {
                text[item.net] = item.text
            }
            labelText = text
        }

        /// A bus wire's label text. Wires of linked buses (one name, same
        /// place) are one net and share the text. Two unrelated wires of one
        /// name get "D0", "D0_2", … unless a net label of that name exists:
        /// the app joins them through it, and so does KiCad.
        private static func busText(buses: [Bus], labels: [NetLabel]) -> [UUID: String] {
            let linked = Bus.linkedWireIDs(buses)
            var nameOfWire: [UUID: String] = [:]
            for bus in buses { for wire in bus.wires { nameOfWire[wire.id] = wire.name } }
            let labelled = Set(labels.filter(\.isNet).map(\.name))
            var assigned: [UUID: String] = [:]
            var owner: [String: UUID] = [:]
            var result: [UUID: String] = [:]
            for bus in buses {
                for wire in bus.wires {
                    let key = linked[wire.id] ?? wire.id
                    if let text = assigned[key] { result[wire.id] = text; continue }
                    let lead = nameOfWire[key] ?? wire.name
                    var text = lead
                    if !labelled.contains(lead) {
                        var n = 2
                        while let other = owner[text], other != key {
                            text = "\(lead)_\(n)"
                            n += 1
                        }
                    }
                    owner[text] = key
                    assigned[key] = text
                    result[wire.id] = text
                }
            }
            return result
        }
    }
}

// MARK: - Formatting

/// KiCad's S-expression text: numbers, strings and the ids.
nonisolated enum KiCad {
    /// A number in millimetres, to 4 decimals with no trailing zeros.
    static func n(_ value: Double) -> String {
        var text = String(format: "%.4f", value)
        if text.contains(".") {
            while text.hasSuffix("0") { text.removeLast() }
            if text.hasSuffix(".") { text.removeLast() }
        }
        return text == "-0" ? "0" : text
    }

    static func n(_ value: CGFloat) -> String { n(Double(value)) }

    /// A quoted S-expression string.
    static func q(_ text: String) -> String {
        var out = "\""
        for character in text {
            switch character {
            case "\"": out += "\\\""
            case "\\": out += "\\\\"
            case "\n": out += "\\n"
            case "\r": break
            default: out.append(character)
            }
        }
        return out + "\""
    }

    /// A quoted JSON string.
    static func json(_ text: String) -> String {
        var out = "\""
        for scalar in text.unicodeScalars {
            switch scalar {
            case "\"": out += "\\\""
            case "\\": out += "\\\\"
            case "\n": out += "\\n"
            default:
                if scalar.value < 0x20 { out += String(format: "\\u%04x", scalar.value) } else { out.unicodeScalars.append(scalar) }
            }
        }
        return out + "\""
    }

    /// A version-4 shaped UUID made from `key`, so exporting the same
    /// document twice writes the same bytes.
    static func uuid(_ key: String) -> String {
        var bytes = Array(SHA256.hash(data: Data(key.utf8)).prefix(16))
        bytes[6] = (bytes[6] & 0x0F) | 0x40
        bytes[8] = (bytes[8] & 0x3F) | 0x80
        let hex = bytes.map { String(format: "%02x", $0) }.joined()
        let h = Array(hex)
        func part(_ range: Range<Int>) -> String { String(h[range]) }
        return [part(0..<8), part(8..<12), part(12..<16), part(16..<20), part(20..<32)].joined(separator: "-")
    }

    /// A symbol or footprint name KiCad takes in a library id: no `:` or
    /// other characters it reserves.
    static func libraryName(_ name: String) -> String {
        let allowed = CharacterSet.alphanumerics.union(CharacterSet(charactersIn: "-_+.,()"))
        let mapped = String(name.unicodeScalars.map { allowed.contains($0) ? Character($0) : "_" })
        return mapped.isEmpty ? "_" : mapped
    }

    /// The smallest standard sheet that holds `width` × `height` mm, or a
    /// user size.
    static func paper(width: Double, height: Double) -> String {
        let sizes: [(String, Double, Double)] = [("A4", 297, 210), ("A3", 420, 297), ("A2", 594, 420),
                                                 ("A1", 841, 594), ("A0", 1189, 841)]
        if let fit = sizes.first(where: { width <= $0.1 && height <= $0.2 }) { return "(paper \(q(fit.0)))" }
        return "(paper \"User\" \(n(max(width, 100).rounded(.up))) \(n(max(height, 100).rounded(.up))))"
    }

    /// The pin name KiCad shows: alternatives joined by `/`, an active-low
    /// one under a bar (`~{CE}`).
    static func pinName(_ pin: ChipPin) -> String {
        if pin.name.isEmpty { return "~" }
        guard pin.isActiveLow else { return pin.name }
        return pin.alternatives.map { alternative in
            guard alternative.isActiveLow else { return alternative.text }
            let bare = alternative.text.hasSuffix("#") ? String(alternative.text.dropLast()) : alternative.text
            return "~{\(bare)}"
        }.joined(separator: "/")
    }
}

// MARK: - Net names

/// How the exporters name a net. KiCad names it after what is on it, so a
/// board and the schematic it came from agree, and so does the EasyEDA export.
struct ExportNets {
    /// The net of each pad (`copper.netOfPad`, and a net of its own for a pin no wire reaches), by the pad's
    /// index in `copper.pads`.
    var padNets: [Int: Int] = [:]
    /// Each net's unique name, and its KiCad code (from 1).
    var names: [Int: String] = [:]
    var codes: [Int: Int] = [:]
}

@MainActor
enum ExportNetNames {
    /// The nets of a board and their names, as the KiCad export writes them and the Gerber export attaches them:
    /// every net a pad carries or a Plane has, each pin no wire reaches on a net of its own
    /// (`unconnected-(U1-A0-Pad1)`), names made unique with `_2`, `_3`, and codes from 1 in net order.
    static func assign(context: KiCadExport.Context) -> ExportNets {
        let document = context.document, copper = context.copper
        var result = ExportNets()
        result.padNets = copper.netOfPad
        // Pins no wire reaches.
        var nextNet = (copper.netOfPad.values.max() ?? 0) + 1
        var ownNet: [String: Int] = [:]
        var pinNames: [Int: String] = [:]
        var padIndex = 0
        for (index, chip) in document.chips.enumerated() where chip.board?.side != nil {
            for pad in chip.boardPads {
                defer { padIndex += 1 }
                guard result.padNets[padIndex] == nil, let designator = pad.designator,
                      let pin = chip.part.pins.first(where: { $0.designator == designator }), !pin.isHidden else { continue }
                let key = "\(index)/\(designator)"
                if ownNet[key] == nil {
                    ownNet[key] = nextNet
                    pinNames[nextNet] = pinNetName(chip: chip, pin: pin, unconnected: true)
                    nextNet += 1
                }
                result.padNets[padIndex] = ownNet[key]
            }
        }
        var nets = Set(result.padNets.values)
        nets.formUnion(copper.planeNets.values)
        for pour in document.pours {
            if let net = pour.net.flatMap({ copper.net(of: $0) }) { nets.insert(net) }
        }
        var used: [String: Int] = [:]
        for net in nets.sorted() {
            let base = pinNames[net] ?? baseName(ofNet: net, context: context) {
                "/" + $0.replacingOccurrences(of: "/", with: "{slash}")
            }
            let count = (used[base] ?? 0) + 1
            used[base] = count
            result.names[net] = count == 1 ? base : "\(base)_\(count)"
            result.codes[net] = result.codes.count + 1
        }
        return result
    }

    /// A name for a net of one pin or the pin it is named after:
    /// `Net-(U1-A0)`, `Net-(U1-GND-Pad7)` when the part has several pins of
    /// that name, `Net-(R1-Pad1)` for a pin with no name, and
    /// `unconnected-(U1-A0-Pad1)` for a pin on nothing.
    static func pinNetName(chip: PlacedChip, pin: ChipPin, unconnected: Bool) -> String {
        let name = pin.name.isEmpty || pin.name == "~" ? "" : KiCad.pinName(pin)
        let pad = "Pad\(pin.designator)"
        if unconnected { return "unconnected-(\(chip.reference)-\(name.isEmpty ? pad : "\(name)-\(pad)"))" }
        if name.isEmpty { return "Net-(\(chip.reference)-\(pad))" }
        let twins = chip.part.pins.filter { KiCad.pinName($0) == name }.count
        return "Net-(\(chip.reference)-\(twins > 1 ? "\(name)-\(pad)" : name))"
    }

    /// The name of `net` (a net of `context.copper`): a rail's name, else the
    /// text of a label on it (`label` writes it), else after its first pin
    /// by name, `unconnected-(…)` when it has one pin.
    static func baseName(ofNet net: Int, context: KiCadExport.Context, label: (String) -> String) -> String {
        let document = context.document
        if let rail = context.rail(ofNet: net) { return rail }
        if let text = context.labelText[net] { return label(text) }
        let pads = context.copper.netOfPad.filter { $0.value == net }.keys.map { context.copper.pads[$0] }
        var pins: [String] = []
        var seen = Set<String>()
        for pad in pads where pad.chip >= 0 {
            let chip = document.chips[pad.chip]
            guard let designator = pad.pad.designator, seen.insert("\(pad.chip)/\(designator)").inserted,
                  let pin = chip.part.pins.first(where: { $0.designator == designator }) else { continue }
            pins.append(pinNetName(chip: chip, pin: pin, unconnected: false))
        }
        if pins.count == 1, let pad = pads.first(where: { $0.chip >= 0 }), let designator = pad.pad.designator,
           let pin = document.chips[pad.chip].part.pins.first(where: { $0.designator == designator }) {
            return pinNetName(chip: document.chips[pad.chip], pin: pin, unconnected: true)
        }
        if let first = pins.min() { return first }
        if let pad = pads.min(by: { $0.label < $1.label }) {
            return "Net-(\(pad.label.replacingOccurrences(of: ".", with: "-")))"
        }
        return "Net-\(net + 1)"
    }
}
