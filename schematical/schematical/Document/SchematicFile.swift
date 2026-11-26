import CoreGraphics
import Foundation
import UniformTypeIdentifiers

// MARK: - File format

/// The on-disk form of a schematic, version 4, or 5 when it embeds custom
/// parts (plan-32-B). See `docs/file-format.md`.
/// Version 1 files (pins by number) are converted when opened; version 1 and
/// 2 files have no buses; version 1–3 files have no part `color` or
/// `footprint`.
///
/// Parts are stored by part number only. Pins, layout and symbol come from the
/// backend catalog when the file is opened.
nonisolated struct SchematicFile: Codable, Equatable, Sendable {
    static let formatName = "schematical"
    /// The newest version this build reads.
    static let currentVersion = 5
    /// The version written for a schematic with no custom part, so older
    /// builds still open it.
    static let plainVersion = 4
    /// The version written when `customParts` is present.
    static let customPartsVersion = 5
    static let fileExtension = "schematical"

    /// Parts removed from the library, and the part each one opens as. A file
    /// that names one opens with the replacement, and is saved with its name.
    static let renamedParts = ["VCC": "5V"]

    /// The 55 LED parts `GENERIC-LED` replaced (plan-18), `LED-<COLOR>-<PACKAGE>`,
    /// and the color and footprint each one opens with. A file of any version
    /// that names one opens as `GENERIC-LED`, and is saved with that name.
    static let legacyLEDs: [String: (color: String, footprint: String)] = {
        let colors = ["RED": "Red", "GREEN": "Green", "BLUE": "Blue", "AMBER": "Amber", "WHITE": "White"]
        var packages = Dictionary(uniqueKeysWithValues:
            ["0402", "0603", "0805", "1204", "1205", "1206", "1209", "1411", "1414"].map { ($0, "LED-\($0)") })
        packages["DIP"] = "LED-DIP4"
        packages["RADIAL"] = "LED-RADIAL-5MM"
        var table: [String: (color: String, footprint: String)] = [:]
        for (code, color) in colors {
            for (package, footprint) in packages {
                table["LED-\(code)-\(package)"] = (color, footprint)
            }
        }
        return table
    }()

    /// The part every name in `legacyLEDs` opens as.
    static let genericLED = "GENERIC-LED"

    /// The file type, declared in the app's Info.plist (`UTExportedTypeDeclarations`).
    static let contentType = UTType(exportedAs: "jgz.schematical.schematic", conformingTo: .json)

    /// Types the open panel accepts. Besides the declared type, this includes
    /// whatever type the system gives a `.schematical` file on its own, in case
    /// the declaration hasn't been registered yet, for example on a fresh build.
    static var openableTypes: [UTType] {
        var types = [contentType]
        if let byExtension = UTType(filenameExtension: fileExtension), byExtension != contentType {
            types.append(byExtension)
        }
        return types
    }

    struct Point: Codable, Equatable, Sendable {
        var x: Double
        var y: Double
    }

    /// One unit of a part drawn as its units (`Part.units`).
    struct Unit: Codable, Equatable, Sendable {
        /// The unit's key: "A", "B", … or "P".
        var unit: String
        /// Top-left corner of the unit's body, in canvas units.
        var x: Double
        var y: Double
        /// Left out unless true.
        var flipped: Bool? = nil
        /// 90, 180 or 270, gates only. Left out when 0.
        var rotation: Int? = nil
    }

    struct Part: Codable, Equatable, Sendable {
        var id: UUID
        var reference: String
        var partNumber: String
        /// Top-left corner of the body, in canvas units.
        var x: Double
        var y: Double
        var flipped: Bool
        /// Parts drawn as their own symbol only (`PartSymbol.canRotate`): how far the part is
        /// turned clockwise on the Schematic, 0, 90, 180 or 270. Left out when 0.
        var rotation: Int? = nil
        /// A part drawn as its units only (plan-67; a gate IC in Gate view):
        /// where each unit is, every unit of the part once. Then `x`, `y` are
        /// the corner of the box around them, `flipped` is false and
        /// `rotation` is left out. Left out for a part drawn as one block; a
        /// list that does not name exactly the part's units is ignored.
        var units: [Unit]? = nil
        /// DIP switches only: whether each position is ON, position 1 first.
        /// Left out for other parts.
        var switches: [Bool]? = nil
        /// The footprint's place on the PCB. Left out for parts without a
        /// footprint (VCC and GND).
        var pcb: Placement? = nil
        /// EEPROMs only: the file loaded into the chip when the simulation
        /// starts. Left out when there is none and for other parts.
        var memoryFile: MemoryFileEntry? = nil
        /// LEDs only (version 4): one of the part's colors. Always written for
        /// an LED; missing means the part's default.
        var color: String? = nil
        /// Parts with footprint choices only (version 4): the chosen
        /// footprint's id. Always written for them; missing means the part's
        /// default footprint.
        var footprint: String? = nil
        /// Headers only (plan-23): one of the part's genders. Always written
        /// for a header; missing means the part's default.
        var gender: String? = nil
        /// Pull resistors (version 4) and `GENERIC-RESISTOR` (plan-34-G) only:
        /// the resistance in ohms. Always written for them; missing means
        /// 1000; ignored on other parts.
        var ohms: Double? = nil
        /// Custom parts only (version 5): the part's uuid, one of the file's
        /// `customParts`. The part resolves by it; `partNumber` is the name it
        /// had when saved.
        var partUUID: String? = nil
        /// `GENERIC-CAPACITOR` only (plan-33-H): the capacitance in farads.
        /// Always written for it; missing means 1e-7 (0.1 µF); ignored on
        /// other parts. Optional like `gender`: the version stays 4.
        var farads: Double? = nil
        /// Written only as `false`, when the Schematic hides the part's
        /// reference (Display Reference off); missing means shown.
        var showReference: Bool? = nil
        /// A chip in a socket mode only (plan-51-H): the socket and the mode.
        /// Left out for a plain IC. Optional like `gender`: the version stays 4.
        var socket: SocketEntry? = nil
    }

    /// A chip's socket: the bundled socket part's name and the mode, `"withSocket"`
    /// (the chip sits in the socket) or `"socketOnly"` (the socket without the chip).
    struct SocketEntry: Codable, Equatable, Sendable {
        var part: String
        var mode: String
    }

    /// An EEPROM's load file: the path relative to the schematic's folder (or
    /// absolute for a schematic never saved), and an app-scoped security
    /// bookmark (base64 in JSON) that is tried first.
    struct MemoryFileEntry: Codable, Equatable, Sendable {
        var path: String
        var bookmark: Data?
    }

    /// A footprint's centre in board millimetres (y down) and its clockwise
    /// rotation: 0, 90, 180 or 270. Then its origin point and where its marks
    /// are drawn, as the raw values of `FootprintOrigin` and `MarkLayer`;
    /// missing (older files) means the defaults.
    struct Placement: Codable, Equatable, Sendable {
        var x: Double
        var y: Double
        var rotation: Int
        var origin: String? = nil
        var outline: String? = nil
        var pin1: String? = nil
        var reference: String? = nil
        /// `FootprintStyle` raw value; written only for parts with a
        /// connector footprint (character LCD modules), ignored on others.
        var style: String? = nil
        /// `BoardSide` raw value; missing means top.
        var side: String? = nil
        /// True when its position is locked; left out otherwise.
        var locked: Bool? = nil
        /// The reference text's centre, mm from the body centre in the
        /// footprint's own frame; written only when not zero, missing is 0.
        var refX: Double? = nil
        var refY: Double? = nil
    }

    /// The PCB: size in mm, copper layers, colour and sidebar units (raw
    /// values of `BoardColor` and `BoardUnits`), the component grid in mm,
    /// and free texts. Missing keys mean the defaults of `BoardSettings`.
    struct Board: Codable, Equatable, Sendable {
        var width: Double? = nil
        var height: Double? = nil
        var layers: Int? = nil
        var color: String? = nil
        var units: String? = nil
        /// The component grid, mm; the trace grid is half of it.
        var grid: Double? = nil
        /// `BoardType` raw value; missing means custom.
        var type: String? = nil
        /// The id of the bus the card edge uses; written only while it
        /// counts (a card, and a bus with as many wires as its pins).
        var edgeBus: UUID? = nil
        /// A custom card's edge (`type` "customCard"); missing keys mean
        /// the defaults.
        var card: CardEntry? = nil
        /// The settings of a type that has them, by option id
        /// (`boards.json`: PCI Express `lanes`, M.2 `key`, …); missing keys
        /// mean the defaults.
        var cardOptions: [String: BoardValue]? = nil

        struct CardEntry: Codable, Equatable, Sendable {
            var pads: Int? = nil
            var pitch: Double? = nil
            var padWidth: Double? = nil
            var padLength: Double? = nil
            var padOffset: Double? = nil
            var depth: Double? = nil
            var indent: Double? = nil
            var margin: Double? = nil
            var chamfer: Double? = nil
            var bothSides: Bool? = nil
        }
        /// One entry per copper layer on the board, in stack order. Missing
        /// entries, colours or types mean the defaults.
        var layerStyles: [LayerStyleEntry]? = nil
        var texts: [Text]? = nil
        /// Copper traces, in drawing order. Always written (`[]` when there
        /// are none); missing means none.
        var traces: [TraceEntry]? = nil
        /// Vias, in drawing order. Written only when there are some;
        /// missing means none.
        var vias: [ViaEntry]? = nil
        /// Drilled holes, in drawing order. Written only when there are
        /// some; missing means none.
        var holes: [HoleEntry]? = nil
        /// Copper pours, in drawing order (plan-52). Written only when there
        /// are some; missing means none.
        var pours: [PourEntry]? = nil
        /// Keep-outs the pours stay out of. Written only when there are
        /// some; missing means none.
        var keepouts: [KeepoutEntry]? = nil
        /// A Custom board's free-form outline (plan-65): clockwise or not, its
        /// bounds' top-left at (0, 0). Written only for such a board; it wins
        /// over `width` and `height`.
        var outline: [Point]? = nil
        /// Cutouts of the board, in drawing order. Written only when there
        /// are some; missing means none.
        var cutouts: [CutoutEntry]? = nil
        /// Net classes, in priority order. Written only when there are
        /// some; missing means none (every net in the Default class).
        var netClasses: [NetClassEntry]? = nil

        /// A net class: its name, trace width in mil, clearance in mm
        /// (missing means the DRC's minimum), via hole and metal in mm, and
        /// its nets (`PlaneNet.fileValue` each).
        struct NetClassEntry: Codable, Equatable, Sendable {
            var id: UUID
            var name: String
            var width: Int
            var clearance: Double? = nil
            var viaDrill: Double
            var viaDiameter: Double
            var members: [String]
            /// The class's layer rule (`LayerRuleMode` raw value) and its
            /// layers (`CopperLayer` raw values); both missing means none.
            var layerMode: String? = nil
            var layers: [String]? = nil
        }
        /// Per-net layer rules, before the net's class's. Written only
        /// when there are some.
        var netLayerRules: [NetLayerRuleEntry]? = nil
        /// Net groups. Written only when there are some.
        var netGroups: [NetGroupEntry]? = nil
        /// The board's design rules. Always written; missing (a file from
        /// before they were kept in the file) means the app's defaults.
        var designRules: DesignRulesEntry? = nil
        /// The layer stack. Always written; a missing stackup, or a missing
        /// key in it, is the standard one for the layer count.
        var stackup: StackupEntry? = nil

        /// The stackup: copper thickness per layer (mm, Top first), the
        /// dielectric between each two layers, the finish
        /// (`SurfaceFinish` raw value) and the temperature rise in °C.
        struct StackupEntry: Codable, Equatable, Sendable {
            var copper: [Double]? = nil
            var dielectrics: [DielectricEntry]? = nil
            var finish: String? = nil
            var temperatureRise: Double? = nil
        }

        /// A dielectric: `type` (`DielectricKind` raw value), thickness in
        /// mm and relative permittivity.
        struct DielectricEntry: Codable, Equatable, Sendable {
            var type: String
            var thickness: Double
            var er: Double
        }

        /// The board's design rules in mm, and the manufacturer
        /// (`DRCManufacturer` raw value) they are named after. A missing
        /// rule is the app's default for it; a missing or unknown
        /// manufacturer is Custom.
        struct DesignRulesEntry: Codable, Equatable, Sendable {
            var manufacturer: String? = nil
            var minTraceWidth: Double? = nil
            var minClearance: Double? = nil
            var minDrill: Double? = nil
            var minViaDrill: Double? = nil
            var minViaDiameter: Double? = nil
            var minAnnularRing: Double? = nil
            var minHoleToHole: Double? = nil
            var minCopperToEdge: Double? = nil
            var minSilkscreenWidth: Double? = nil
            var minSilkscreenHeight: Double? = nil
            var minSilkscreenToPad: Double? = nil
            var minSolderMaskDam: Double? = nil
            var solderMaskExpansion: Double? = nil

            /// Each rule's key here, its place in `DesignRules`, and its
            /// name in the file.
            static let fields: [(entry: WritableKeyPath<DesignRulesEntry, Double?>,
                                 rule: WritableKeyPath<DesignRules, Double>, name: String)] = [
                (\.minTraceWidth, \.minTraceWidth, "minTraceWidth"),
                (\.minClearance, \.minClearance, "minClearance"),
                (\.minDrill, \.minDrill, "minDrill"),
                (\.minViaDrill, \.minViaDrill, "minViaDrill"),
                (\.minViaDiameter, \.minViaDiameter, "minViaDiameter"),
                (\.minAnnularRing, \.minAnnularRing, "minAnnularRing"),
                (\.minHoleToHole, \.minHoleToHole, "minHoleToHole"),
                (\.minCopperToEdge, \.minCopperToEdge, "minCopperToEdge"),
                (\.minSilkscreenWidth, \.minSilkscreenWidth, "minSilkscreenWidth"),
                (\.minSilkscreenHeight, \.minSilkscreenHeight, "minSilkscreenHeight"),
                (\.minSilkscreenToPad, \.minSilkscreenToPad, "minSilkscreenToPad"),
                (\.minSolderMaskDam, \.minSolderMaskDam, "minSolderMaskDam"),
                (\.solderMaskExpansion, \.solderMaskExpansion, "solderMaskExpansion"),
            ]
        }

        /// One net's layer rule: the net (`PlaneNet.fileValue`), the mode
        /// and the layers.
        struct NetLayerRuleEntry: Codable, Equatable, Sendable {
            var net: String
            var mode: String
            var layers: [String]
        }

        /// A net group: its mode (`NetGroupMode` raw value), members
        /// (`PlaneNet.fileValue` each) and sizes in mm; missing sizes are
        /// the defaults.
        struct NetGroupEntry: Codable, Equatable, Sendable {
            var id: UUID
            var name: String
            var mode: String
            var members: [String]
            var tolerance: Double? = nil
            var gap: Double? = nil
            var maxUncoupled: Double? = nil
            var maxMeander: Double? = nil
        }

        /// A drilled hole: its centre, size (`HoleSize` raw value, default
        /// "m3"), hole (`drill`) and land (`diameter`) in mm, whether it is
        /// plated, and its net (`PlaneNet.fileValue`, kept while not plated).
        /// Missing sizes are the size's, or for Custom the M3 sizes.
        struct HoleEntry: Codable, Equatable, Sendable {
            var id: UUID
            var x: Double
            var y: Double
            var size: String? = nil
            var drill: Double? = nil
            var diameter: Double? = nil
            /// True when plated; left out otherwise.
            var plated: Bool? = nil
            var net: String? = nil
            /// True when its position is locked; left out otherwise.
            var locked: Bool? = nil
        }

        /// A copper pour: its rectangle (mm), layer, net (`PlaneNet.fileValue`),
        /// clearance (missing is the DRC's minimum), connection (`thermal` or
        /// `solid`), thermal gap, spoke width and minimum width in mm.
        struct PourEntry: Codable, Equatable, Sendable {
            var id: UUID
            var x: Double
            var y: Double
            var width: Double
            var height: Double
            var layer: String
            var net: String? = nil
            var clearance: Double? = nil
            var connection: String? = nil
            var thermalGap: Double? = nil
            var spokeWidth: Double? = nil
            var minWidth: Double? = nil
            /// True when its position is locked; left out otherwise.
            var locked: Bool? = nil
        }

        /// A keep-out: its rectangle (mm) and the layers it applies to
        /// (missing is every layer).
        struct KeepoutEntry: Codable, Equatable, Sendable {
            var id: UUID
            var x: Double
            var y: Double
            var width: Double
            var height: Double
            var layers: [String]? = nil
            var locked: Bool? = nil
        }

        /// A cutout (plan-65): `shape` "rounded" has `x`, `y`, `width`,
        /// `height` and `radius` (mm; a missing radius is 0), "polygon" has
        /// `points`.
        struct CutoutEntry: Codable, Equatable, Sendable {
            var id: UUID
            var shape: String
            var x: Double? = nil
            var y: Double? = nil
            var width: Double? = nil
            var height: Double? = nil
            var radius: Double? = nil
            var points: [Point]? = nil
            var locked: Bool? = nil
        }

        /// A via: its centre, hole (`drill`) and metal (`diameter`), in mm.
        /// Missing sizes mean the defaults (0.4 and 0.8).
        struct ViaEntry: Codable, Equatable, Sendable {
            var id: UUID
            var x: Double
            var y: Double
            var drill: Double? = nil
            var diameter: Double? = nil
            /// True when its position is locked; left out otherwise.
            var locked: Bool? = nil
            /// True when the autorouter put it down; left out otherwise.
            var auto: Bool? = nil
            /// True when the solder mask leaves it open on that side; left
            /// out (tented) otherwise.
            var openTop: Bool? = nil
            var openBottom: Bool? = nil
        }

        /// A trace: its copper layer (`CopperLayer` raw value), width in
        /// mil (default 10) and points in mm, kept as saved (not snapped).
        struct TraceEntry: Codable, Equatable, Sendable {
            var id: UUID
            var layer: String
            var width: Int? = nil
            var points: [Point]
            /// True when its position is locked; left out otherwise.
            var locked: Bool? = nil
            /// True when the autorouter put it down; left out otherwise.
            var auto: Bool? = nil
        }

        /// A copper layer's colour ("#RRGGBB") and type (`LayerType` raw
        /// value), by its `CopperLayer` raw value.
        struct LayerStyleEntry: Codable, Equatable, Sendable {
            var layer: String
            var color: String? = nil
            var type: String? = nil
            /// A Plane's net (`PlaneNet.fileValue`); missing means none.
            var net: String? = nil
            /// A Plane's clearance in mm; missing means the DRC's minimum.
            var clearance: Double? = nil
        }

        /// A board text: top-left at rotation 0 (`x`, `y`, mm), font size in
        /// mil, layer (`TextLayer` raw value) and clockwise rotation.
        struct Text: Codable, Equatable, Sendable {
            var id: UUID
            var text: String
            var x: Double
            var y: Double
            var size: Int? = nil
            var layer: String? = nil
            var rotation: Int? = nil
            /// `BoardSide` raw value; missing means top.
            var side: String? = nil
            /// True when its position is locked; left out otherwise.
            var locked: Bool? = nil
        }
    }

    /// A text block on the schematic: top-left (`x`, `y`, canvas units),
    /// words, font size in points and whether it has a frame.
    struct Note: Codable, Equatable, Sendable {
        var id: UUID
        var text: String
        var x: Double
        var y: Double
        var size: Int? = nil
        var framed: Bool? = nil
    }

    /// The document's information (plan-64): every key optional. Dates are
    /// ISO 8601, UTC. `titleBlock` is present exactly when the title block
    /// is on the schematic.
    struct DocumentInfoEntry: Codable, Equatable, Sendable {
        var title: String? = nil
        var author: String? = nil
        var revision: String? = nil
        var description: String? = nil
        var created: String? = nil
        var updated: String? = nil
        /// Written only as `false`.
        var automaticDates: Bool? = nil
        var titleBlock: Point? = nil
    }

    /// A net label: its point (`x`, `y`, canvas units), net name, type
    /// (`"input"`, `"output"`, `"inout"`, `"passive"` or `"noConnect"`)
    /// and whether it runs left.
    struct Label: Codable, Equatable, Sendable {
        var id: UUID
        var name: String
        var x: Double
        var y: Double
        var type: String? = nil
        /// True when the flag runs left of its point; left out otherwise.
        var flipped: Bool? = nil
    }

    struct Junction: Codable, Equatable, Sendable {
        var id: UUID
        var x: Double
        var y: Double
        /// True for a loose end (the spot where a deleted pin was); left out
        /// otherwise.
        var loose: Bool? = nil
    }

    /// How a wire end names its pin: by designator (version 2, `"A1"`), or by
    /// number (version 1, `14`: the part's 14th pin in its pinout).
    enum PinKey: Equatable, Sendable {
        case designator(String)
        case position(Int)
    }

    /// One end of a wire: `{"part": <id>, "pin": "<designator>"}`,
    /// `{"junction": <id>}`, `{"port": <id>, "wire": <bus wire id>}`, or
    /// `{"label": <id>}`.
    enum End: Codable, Equatable, Sendable {
        case pin(part: UUID, pin: PinKey)
        case junction(UUID)
        case port(port: UUID, wire: UUID)
        case label(UUID)

        private enum CodingKeys: String, CodingKey {
            case part, pin, junction, port, wire, label
        }

        init(from decoder: Decoder) throws {
            let container = try decoder.container(keyedBy: CodingKeys.self)
            if let label = try container.decodeIfPresent(UUID.self, forKey: .label) {
                self = .label(label)
            } else if let port = try container.decodeIfPresent(UUID.self, forKey: .port) {
                self = .port(port: port, wire: try container.decode(UUID.self, forKey: .wire))
            } else if let junction = try container.decodeIfPresent(UUID.self, forKey: .junction) {
                self = .junction(junction)
            } else {
                let part = try container.decode(UUID.self, forKey: .part)
                if let designator = try? container.decode(String.self, forKey: .pin) {
                    self = .pin(part: part, pin: .designator(designator))
                } else {
                    self = .pin(part: part, pin: .position(try container.decode(Int.self, forKey: .pin)))
                }
            }
        }

        func encode(to encoder: Encoder) throws {
            var container = encoder.container(keyedBy: CodingKeys.self)
            switch self {
            case let .pin(part, .designator(designator)):
                try container.encode(part, forKey: .part)
                try container.encode(designator, forKey: .pin)
            case let .pin(part, .position(number)):
                try container.encode(part, forKey: .part)
                try container.encode(number, forKey: .pin)
            case let .junction(id):
                try container.encode(id, forKey: .junction)
            case let .port(port, wire):
                try container.encode(port, forKey: .port)
                try container.encode(wire, forKey: .wire)
            case let .label(id):
                try container.encode(id, forKey: .label)
            }
        }
    }

    struct Wire: Codable, Equatable, Sendable {
        var id: UUID
        var from: End
        var to: End
        /// Bend points between the two ends, in order.
        var bends: [Point]
    }

    /// A bus: its straight lines (joints are worked out from where they
    /// touch) and its wires in list order.
    struct Bus: Codable, Equatable, Sendable {
        var id: UUID
        /// Always written; missing (older files) means "BUS <n>", numbered
        /// in file order.
        var name: String? = nil
        /// `BoardType` raw value; written only when not custom.
        var type: String? = nil
        /// A custom card's edge, for `type` "customCard".
        var card: Board.CardEntry? = nil
        /// The settings of a type that change its pins (`onBus` options),
        /// by option id.
        var cardOptions: [String: BoardValue]? = nil
        var lines: [BusLineEntry]
        var wires: [BusWireEntry]
        /// Always written (empty allowed); missing means none.
        var ports: [PortEntry]? = nil
    }

    /// A bus port: the line it sits on, its first pin slot's along-axis
    /// coordinate, its pin side, and its ticked bus wires in bus order.
    struct PortEntry: Codable, Equatable, Sendable {
        var id: UUID
        var line: UUID
        var start: Double
        var side: String
        var wires: [UUID]
    }

    /// One straight bus line, from `a` to `b`.
    struct BusLineEntry: Codable, Equatable, Sendable {
        var id: UUID
        var a: Point
        var b: Point
    }

    /// One bus wire: the id ports refer to, and its display name.
    struct BusWireEntry: Codable, Equatable, Sendable {
        var id: UUID
        var name: String
    }

    var format: String
    var version: Int
    var nextReference: Int
    var parts: [Part]
    var junctions: [Junction]
    var wires: [Wire]
    /// Buses. Always written from version 3 on (empty when there are none);
    /// nil in older files, which means none.
    var buses: [Bus]? = nil
    /// The PCB's settings and texts. Always written; files from before it
    /// open with the defaults.
    var board: Board? = nil
    /// Text blocks on the schematic, in drawing order. Always written (`[]`
    /// when there are none); missing means none.
    var notes: [Note]? = nil
    /// Net labels, in drawing order. Written only when there are some;
    /// missing means none.
    var labels: [Label]? = nil
    /// The document's title, author, dates and title block (plan-64).
    /// Written only when it is not the default; missing means the default.
    var documentInfo: DocumentInfoEntry? = nil
    /// Version 5: the single part files (plan-32-A) of the custom parts the
    /// schematic places, once each, in order of first placement. Written only
    /// when there are some.
    var customParts: [JSONValue]? = nil
    /// The electrical rules check's settings (plan-63-E): written only when a
    /// severity, the pin map or an exclusion differs from KiCad's defaults;
    /// missing means the defaults.
    var erc: ErcEntry? = nil

    /// `erc`: what differs from the defaults. `severities` maps a rule name to
    /// `error`, `warning` or `ignore`; `pinMap` is 12 rows of 12 numbers
    /// 0 (ok), 1 (warning) or 2 (error), symmetric, in KiCad's pin-type order;
    /// `exclusions` are `ErcViolation.key` strings.
    struct ErcEntry: Codable, Equatable, Sendable {
        var severities: [String: String]?
        var pinMap: [[Int]]?
        var exclusions: [String]?
    }
}

/// Any JSON value, for the custom parts a schematic embeds whole.
nonisolated enum JSONValue: Codable, Equatable, Sendable {
    case object([String: JSONValue])
    case array([JSONValue])
    case string(String)
    case number(Double)
    case bool(Bool)
    case null

    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()
        if container.decodeNil() {
            self = .null
        } else if let value = try? container.decode(Bool.self) {
            self = .bool(value)
        } else if let value = try? container.decode(Double.self) {
            self = .number(value)
        } else if let value = try? container.decode(String.self) {
            self = .string(value)
        } else if let value = try? container.decode([JSONValue].self) {
            self = .array(value)
        } else {
            self = .object(try container.decode([String: JSONValue].self))
        }
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()
        switch self {
        case let .object(value): try container.encode(value)
        case let .array(value): try container.encode(value)
        case let .string(value): try container.encode(value)
        case let .number(value): try container.encode(value)
        case let .bool(value): try container.encode(value)
        case .null: try container.encodeNil()
        }
    }

    /// The value's JSON text, keys sorted.
    func text() throws -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys, .withoutEscapingSlashes]
        return String(decoding: try encoder.encode(self), as: UTF8.self)
    }

    init(text: String) throws {
        self = try JSONDecoder().decode(JSONValue.self, from: Data(text.utf8))
    }
}

// MARK: - Errors

nonisolated enum SchematicFileError: LocalizedError {
    case notASchematic
    case newerVersion(Int)
    case unreadable(String)
    case duplicateID(UUID)
    case unknownParts([String])
    case missingPart(wire: UUID, part: UUID)
    case missingPin(wire: UUID, reference: String, pin: String)
    case missingJunction(wire: UUID, junction: UUID)
    case missingPort(wire: UUID, port: UUID)
    case missingPortPin(wire: UUID, port: UUID, busWire: UUID)
    case missingLabel(wire: UUID, label: UUID)
    /// A net label that isn't valid; the detail names it.
    case badLabel(String)
    /// The electrical rules check's settings that aren't valid; the detail names the field.
    case badErc(String)
    case badRotation(reference: String, rotation: Int)
    /// A board setting, footprint origin or mark, or board text that isn't
    /// valid; the detail names it.
    case badPCB(String)
    /// A schematic text block that isn't valid; the detail names it.
    case badNote(String)
    /// A date in the document info that isn't ISO 8601; the detail names it.
    case badDocumentInfo(String)
    /// A bus whose lines, joints or wires aren't valid; the detail names it.
    case badBus(String)
    /// A part's color, footprint or gender that the part doesn't offer.
    case badPartOption(reference: String, detail: String)
    /// A pull resistor's resistance that isn't more than 0 and at most 1 GΩ.
    case badOhms(reference: String, ohms: Double)
    /// An embedded custom part the part loader refused.
    case badCustomPart(String)
    /// The generic capacitor's capacitance that isn't at least 0.1 pF and at most 1 F.
    case badFarads(reference: String, farads: Double)

    var errorDescription: String? {
        switch self {
        case .notASchematic:
            "This file is not a Schematical schematic."
        case let .newerVersion(version):
            "This schematic was saved by a newer version of Schematical (file format \(version))."
        case let .unreadable(detail):
            "The schematic could not be read: \(detail)"
        case let .duplicateID(id):
            "The schematic uses the ID \(id.uuidString) more than once."
        case let .unknownParts(names):
            "The schematic uses parts that are not in the catalog: \(names.joined(separator: ", "))."
        case let .missingPart(wire, part):
            "Wire \(wire.uuidString) connects to part \(part.uuidString), which is not in the file."
        case let .missingPin(wire, reference, pin):
            "Wire \(wire.uuidString) connects to pin \(pin) of \(reference), which has no such pin."
        case let .missingJunction(wire, junction):
            "Wire \(wire.uuidString) ends at junction \(junction.uuidString), which is not in the file."
        case let .missingPort(wire, port):
            "Wire \(wire.uuidString) connects to port \(port.uuidString), which is not in the file."
        case let .missingPortPin(wire, port, _):
            "Wire \(wire.uuidString) connects to a pin port \(port.uuidString) does not have."
        case let .missingLabel(wire, label):
            "Wire \(wire.uuidString) connects to net label \(label.uuidString), which is not in the file."
        case let .badLabel(detail):
            "The net label is not valid: \(detail)."
        case let .badErc(detail):
            "The electrical rules check settings are not valid: \(detail)."
        case let .badRotation(reference, rotation):
            "\(reference) has PCB rotation \(rotation); it must be 0, 90, 180 or 270."
        case let .badPCB(detail):
            "The PCB settings are not valid: \(detail)."
        case let .badBus(detail):
            "The bus is not valid: \(detail)."
        case let .badNote(detail):
            "The text block is not valid: \(detail)."
        case let .badDocumentInfo(detail):
            "The document info is not valid: \(detail)."
        case let .badPartOption(reference, detail):
            "\(reference): \(detail)"
        case let .badOhms(reference, ohms):
            "\(reference) has resistance \(Self.number(ohms)) Ω; it must be more than 0 and at most 1 GΩ."
        case let .badCustomPart(detail):
            "One of this schematic's custom parts could not be loaded: \(detail)"
        case let .badFarads(reference, farads):
            "\(reference) has capacitance \(Self.number(farads)) F; it must be more than 0 and at most 1 F."
        }
    }

    /// `ohms` as written in a file: no ".0" on a whole number.
    private static func number(_ value: Double) -> String {
        if value.isFinite, value == value.rounded(), abs(value) < 1e15 {
            return String(Int64(value))
        }
        return String(value)
    }
}
