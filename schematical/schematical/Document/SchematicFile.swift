import CoreGraphics
import Foundation
import UniformTypeIdentifiers

// MARK: - File format

/// The on-disk form of a schematic, version 4. See `docs/file-format.md`.
/// Version 1 files (pins by number) are converted when opened; version 1 and
/// 2 files have no buses; version 1–3 files have no part `color` or
/// `footprint`.
///
/// Parts are stored by part number only. Pins, layout and symbol come from the
/// backend catalog when the file is opened.
nonisolated struct SchematicFile: Codable, Equatable, Sendable {
    static let formatName = "schematical"
    static let currentVersion = 4
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

    struct Part: Codable, Equatable, Sendable {
        var id: UUID
        var reference: String
        var partNumber: String
        /// Top-left corner of the body, in canvas units.
        var x: Double
        var y: Double
        var flipped: Bool
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
        /// Pull resistors only (version 4): the resistance in ohms. Always
        /// written for them; missing means 1000; ignored on other parts.
        var ohms: Double? = nil
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
        /// The settings of a PCI Express, M.2, Mini PCIe, SO-DIMM or DIMM
        /// card; missing keys mean the defaults.
        var cardOptions: OptionsEntry? = nil

        struct OptionsEntry: Codable, Equatable, Sendable {
            var lanes: Int? = nil
            var lowProfile: Bool? = nil
            var length: Int? = nil
            var key: String? = nil
            var halfSize: Bool? = nil
            var memory: String? = nil
            var registered: Bool? = nil
            /// An Arduino shield's ICSP header holes.
            var icsp: Bool? = nil
        }

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
        }

        /// A copper layer's colour ("#RRGGBB") and type (`LayerType` raw
        /// value), by its `CopperLayer` raw value.
        struct LayerStyleEntry: Codable, Equatable, Sendable {
            var layer: String
            var color: String? = nil
            var type: String? = nil
            /// A Plane's net (`PlaneNet.fileValue`); missing means none.
            var net: String? = nil
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
        /// The settings of a PCI Express, M.2, Mini PCIe, SO-DIMM or DIMM type.
        var cardOptions: Board.OptionsEntry? = nil
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
    case badRotation(reference: String, rotation: Int)
    /// A board setting, footprint origin or mark, or board text that isn't
    /// valid; the detail names it.
    case badPCB(String)
    /// A schematic text block that isn't valid; the detail names it.
    case badNote(String)
    /// A bus whose lines, joints or wires aren't valid; the detail names it.
    case badBus(String)
    /// A part's color, footprint or gender that the part doesn't offer.
    case badPartOption(reference: String, detail: String)
    /// A pull resistor's resistance that isn't more than 0 and at most 1 GΩ.
    case badOhms(reference: String, ohms: Double)

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
        case let .badRotation(reference, rotation):
            "\(reference) has PCB rotation \(rotation); it must be 0, 90, 180 or 270."
        case let .badPCB(detail):
            "The PCB settings are not valid: \(detail)."
        case let .badBus(detail):
            "The bus is not valid: \(detail)."
        case let .badNote(detail):
            "The text block is not valid: \(detail)."
        case let .badPartOption(reference, detail):
            "\(reference): \(detail)"
        case let .badOhms(reference, ohms):
            "\(reference) has resistance \(Self.number(ohms)) Ω; it must be more than 0 and at most 1 GΩ."
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

// MARK: - Converting to and from the document

extension SchematicFile {
    /// A snapshot of the document. The selection is not saved. `fileURL` is
    /// where it will be (or was) saved: load-file paths are relative to its
    /// folder, or absolute when it is nil.
    @MainActor
    init(document: SchematicDocument, fileURL: URL?) {
        format = Self.formatName
        version = Self.currentVersion
        nextReference = document.nextReference
        parts = document.chips.map { chip in
            Part(
                id: chip.id,
                reference: chip.reference,
                partNumber: chip.partNumber,
                x: chip.origin.x,
                y: chip.origin.y,
                flipped: chip.isFlipped,
                switches: chip.switchesOn.isEmpty || chip.part.isTactileSwitch ? nil : chip.switchesOn,
                pcb: chip.board.map {
                    Placement(x: $0.centre.x, y: $0.centre.y, rotation: $0.rotation,
                              origin: $0.origin.rawValue, outline: $0.outline.rawValue,
                              pin1: $0.pin1.rawValue, reference: $0.reference.rawValue,
                              style: chip.part.connectorFootprint == nil ? nil : $0.style.rawValue,
                              side: $0.side.rawValue, locked: $0.locked ? true : nil)
                },
                memoryFile: chip.memoryFile.map {
                    MemoryFileEntry(path: MemoryFileAccess.path(of: $0.url, schematic: fileURL),
                                    bookmark: $0.bookmark)
                },
                color: chip.color,
                footprint: chip.footprintID,
                gender: chip.gender,
                ohms: chip.ohms
            )
        }
        junctions = document.junctions.map {
            Junction(id: $0.id, x: $0.point.x, y: $0.point.y, loose: $0.isLoose ? true : nil)
        }
        wires = document.wires.map { wire in
            Wire(
                id: wire.id,
                from: End(wire.start, chips: document.chips),
                to: End(wire.end, chips: document.chips),
                bends: wire.interior.map { Point(x: $0.x, y: $0.y) }
            )
        }
        buses = document.buses.map { bus in
            Bus(
                id: bus.id,
                name: bus.name,
                type: bus.card.isCustom ? nil : bus.card.type.rawValue,
                card: bus.card.type == .customCard ? Self.entry(bus.card.customCard) : nil,
                cardOptions: bus.card.type.hasBusOptions ? Self.entry(bus.card.cardOptions) : nil,
                lines: bus.lines.map {
                    BusLineEntry(id: $0.id, a: Point(x: $0.a.x, y: $0.a.y), b: Point(x: $0.b.x, y: $0.b.y))
                },
                wires: bus.wires.map { BusWireEntry(id: $0.id, name: $0.name) },
                ports: bus.ports.map { port in
                    PortEntry(id: port.id, line: port.line, start: port.start,
                              side: Self.sideName(port.side), wires: bus.pinWires(of: port).map(\.id))
                }
            )
        }
        notes = document.notes.map { note in
            Note(id: note.id, text: note.text, x: note.origin.x, y: note.origin.y,
                 size: note.size, framed: note.framed)
        }
        labels = document.netLabels.isEmpty ? nil : document.netLabels.map { label in
            Label(id: label.id, name: label.name, x: label.origin.x, y: label.origin.y,
                  type: label.style.rawValue, flipped: label.isFlipped ? true : nil)
        }
        let settings = document.board
        board = Board(
            width: settings.width,
            height: settings.height,
            layers: settings.layers,
            color: settings.color.rawValue,
            units: settings.units.rawValue,
            grid: settings.grid,
            type: settings.type == .custom ? nil : settings.type.rawValue,
            edgeBus: document.edgeBusID,
            card: settings.type == .customCard ? Self.entry(settings.customCard) : nil,
            cardOptions: settings.type.hasOptions ? Self.entry(settings.cardOptions) : nil,
            layerStyles: settings.copperLayers.map { layer in
                let style = settings.style(of: layer)
                return Board.LayerStyleEntry(layer: layer.rawValue, color: style.color.hex, type: style.type.rawValue,
                                             net: style.net?.fileValue)
            },
            texts: document.texts.map { text in
                Board.Text(id: text.id, text: text.text, x: text.anchor.x, y: text.anchor.y,
                           size: text.size, layer: text.layer.rawValue, rotation: text.rotation,
                           side: text.side.rawValue, locked: text.locked ? true : nil)
            },
            traces: document.traces.map { trace in
                Board.TraceEntry(id: trace.id, layer: trace.layer.rawValue, width: trace.width,
                                 points: trace.points.map { Point(x: $0.x, y: $0.y) },
                                 locked: trace.locked ? true : nil)
            },
            vias: document.vias.isEmpty ? nil : document.vias.map { via in
                Board.ViaEntry(id: via.id, x: via.centre.x, y: via.centre.y, drill: via.drill, diameter: via.diameter,
                               locked: via.locked ? true : nil)
            },
            holes: document.holes.isEmpty ? nil : document.holes.map { hole in
                Board.HoleEntry(id: hole.id, x: hole.centre.x, y: hole.centre.y, size: hole.size.rawValue,
                                drill: hole.drill, diameter: hole.diameter, plated: hole.plated ? true : nil,
                                net: hole.net?.fileValue, locked: hole.locked ? true : nil)
            }
        )
    }

    static func decode(_ data: Data) throws -> SchematicFile {
        // Check the header first, so a newer file gets a clear message rather
        // than a decoding error about fields this version doesn't know.
        struct Header: Decodable {
            var format: String?
            var version: Int?
        }
        guard let header = try? JSONDecoder().decode(Header.self, from: data),
              header.format == formatName,
              let version = header.version else {
            throw SchematicFileError.notASchematic
        }
        guard version <= currentVersion else {
            throw SchematicFileError.newerVersion(version)
        }
        let file: SchematicFile
        do {
            file = try JSONDecoder().decode(SchematicFile.self, from: data)
        } catch {
            throw SchematicFileError.unreadable(error.localizedDescription)
        }
        // One pin type per version: numbers in version 1, designators after.
        for wire in file.wires {
            for case let .pin(_, pin) in [wire.from, wire.to] {
                switch (file.version, pin) {
                case (1, .designator):
                    throw SchematicFileError.unreadable("version 1 files name pins by number")
                case (2..., .position):
                    throw SchematicFileError.unreadable("version 2 files name pins by designator, such as \"A1\"")
                default:
                    break
                }
            }
        }
        return file
    }

    func encoded() throws -> Data {
        let encoder = JSONEncoder()
        // Sorted keys, so saving the same schematic twice writes the same bytes.
        encoder.outputFormatting = [.prettyPrinted, .withoutEscapingSlashes, .sortedKeys]
        return try encoder.encode(self)
    }

    /// Rebuilds the document's contents, looking up each part in the catalog.
    /// Throws without touching the document if anything doesn't resolve.
    /// `fileURL` is where the file was read from; EEPROM load-file paths are
    /// resolved against its folder. A missing load file never fails here.
    @MainActor
    func apply(to document: SchematicDocument, catalog: PartCatalogStore, fileURL: URL?) throws {
        var seen = Set<UUID>()
        func claim(_ id: UUID) throws {
            guard seen.insert(id).inserted else { throw SchematicFileError.duplicateID(id) }
        }

        let settings = try boardSettings()

        var chips: [PlacedChip] = []
        var unknown: [String] = []
        for part in parts {
            try claim(part.id)
            let legacy = Self.legacyLEDs[part.partNumber]
            let name = legacy != nil ? Self.genericLED : Self.renamedParts[part.partNumber] ?? part.partNumber
            guard let catalogPart = catalog.part(named: name) else {
                unknown.append(part.partNumber)
                continue
            }
            let partSymbol = symbol(for: catalogPart)
            // The LED options: a stored value must be one the part offers;
            // missing is the part's default (an old LED name gives its own).
            let color = part.color ?? legacy?.color
            if let color, catalogPart.kind == PART_LED, !catalogPart.colors.contains(color) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(color)\" is not a color of \(catalogPart.name)")
            }
            let footprintID = part.footprint ?? legacy?.footprint
            if let footprintID, !catalogPart.footprintChoices.isEmpty,
               !catalogPart.footprintChoices.contains(where: { $0.footprint.id == footprintID }) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(footprintID)\" is not a footprint of \(catalogPart.name)")
            }
            if let gender = part.gender, catalogPart.kind == PART_HEADER, !catalogPart.genders.contains(gender) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(gender)\" is not a gender of \(catalogPart.name)")
            }
            // A pull resistor's resistance; missing is 1 kΩ, ignored elsewhere.
            if catalogPart.isPullResistor, let ohms = part.ohms, !ResistorValue.isValid(ohms) {
                throw SchematicFileError.badOhms(reference: part.reference, ohms: ohms)
            }
            // A load file counts only on an EEPROM.
            var memoryFile: MemoryFile?
            if catalogPart.kind == PART_EEPROM, let entry = part.memoryFile {
                memoryFile = MemoryFileAccess.file(path: entry.path, bookmark: entry.bookmark, schematic: fileURL)
            }
            // Built before its placement, which is snapped with the chosen footprint.
            var chip = PlacedChip(
                id: part.id,
                reference: part.reference,
                part: catalogPart,
                symbol: partSymbol,
                origin: SchematicGrid.snap(CGPoint(x: part.x, y: part.y)),
                isFlipped: part.flipped && partSymbol.canFlip,
                switchesOn: part.switches ?? [],
                memoryFile: memoryFile,
                color: color,
                footprintID: footprintID,
                gender: part.gender,
                ohms: catalogPart.isPullResistor ? part.ohms : nil
            )
            // A stored placement counts only for a part with a footprint.
            var board: BoardPlacement?
            if let pcb = part.pcb, catalogPart.footprint != nil {
                guard BoardPlacement.rotations.contains(pcb.rotation) else {
                    throw SchematicFileError.badRotation(reference: part.reference, rotation: pcb.rotation)
                }
                // A placement from an older file (no origin) is snapped by its
                // top-left, as those builds did. One with an origin is kept as
                // saved: changing the origin doesn't move a footprint, so its
                // origin point may be off the grid, and opening must not move it.
                var stored = try placement(pcb, reference: part.reference)
                if stored.side == .bottom, settings.layers == 1 {
                    throw SchematicFileError.badPCB("\(part.reference) is on the bottom of a 1-layer board")
                }
                if catalogPart.connectorFootprint != nil {
                    stored.style = try Self.value(pcb.style, of: FootprintStyle.self, "\(part.reference) style",
                                                  default: .module)
                }
                chip.board = stored
                let footprint = chip.footprint!
                board = pcb.origin == nil
                    ? BoardLayout.snapped(stored, footprint: footprint, pad1: catalogPart.firstPinDesignator,
                                          grid: settings.grid)
                    : stored
            }
            chip.board = board
            chips.append(chip)
        }
        if !unknown.isEmpty {
            throw SchematicFileError.unknownParts(Array(Set(unknown)).sorted())
        }
        // Footprints with no stored place (older files) go after the stored
        // ones, in file order.
        for index in chips.indices where chips[index].board == nil {
            if let footprint = chips[index].footprint {
                chips[index].board = BoardLayout.nextSpot(for: footprint, among: chips,
                                                          pad1: chips[index].part.firstPinDesignator,
                                                          grid: settings.grid)
            }
        }

        var placedJunctions: [SchematicModelJunction] = []
        for junction in junctions {
            try claim(junction.id)
            placedJunctions.append(SchematicModelJunction(id: junction.id, point: CGPoint(x: junction.x, y: junction.y),
                                                         isLoose: junction.loose ?? false))
        }
        let junctionIDs = Set(placedJunctions.map(\.id))
        let chipsByID = Dictionary(uniqueKeysWithValues: chips.map { ($0.id, $0) })

        // Buses come before wires, since wires may end on their port pins.
        var placedBuses: [SchematicModelBus] = []
        for bus in buses ?? [] {
            try claim(bus.id)
            for line in bus.lines { try claim(line.id) }
            for wire in bus.wires { try claim(wire.id) }
            for port in bus.ports ?? [] { try claim(port.id) }
            placedBuses.append(try Self.modelBus(bus))
        }
        var busNames = Set<String>()
        for bus in placedBuses where !bus.name.isEmpty {
            guard busNames.insert(bus.name.lowercased()).inserted else {
                throw SchematicFileError.badBus("the bus name \"\(bus.name)\" is used twice")
            }
        }
        if let (first, second, point) = SchematicModelBus.firstTouch(placedBuses) {
            throw SchematicFileError.badBus(
                "buses \(first.uuidString) and \(second.uuidString) touch at \(Self.pointText(point))")
        }
        let portsByID = Dictionary(
            placedBuses.flatMap { bus in bus.ports.map { ($0.id, (bus: bus, port: $0)) } },
            uniquingKeysWith: { first, _ in first })

        var placedLabels: [NetLabel] = []
        for entry in labels ?? [] {
            try claim(entry.id)
            placedLabels.append(try Self.label(entry))
        }
        let labelIDs = Set(placedLabels.map(\.id))

        func resolve(_ end: End, wire: UUID) throws -> WireEnd {
            switch end {
            case let .pin(partID, pin):
                guard let chip = chipsByID[partID] else {
                    throw SchematicFileError.missingPart(wire: wire, part: partID)
                }
                // Only pins the symbol draws can take a wire. Version 1 pin n
                // is the part's n-th pin in its pinout.
                let slot: ChipLayout.PinSlot?
                let label: String
                switch pin {
                case let .designator(designator):
                    slot = chip.layout.slots.first { $0.pin.designator == designator }
                    label = designator
                case let .position(number):
                    slot = chip.layout.slots.first { $0.pin.index == number - 1 }
                    label = "\(number)"
                }
                guard let slot else {
                    throw SchematicFileError.missingPin(wire: wire, reference: chip.reference, pin: label)
                }
                return .pin(PinRef(chipID: partID, index: slot.pin.index))
            case let .junction(id):
                guard junctionIDs.contains(id) else {
                    throw SchematicFileError.missingJunction(wire: wire, junction: id)
                }
                return .junction(id)
            case let .port(portID, busWire):
                guard let found = portsByID[portID] else {
                    throw SchematicFileError.missingPort(wire: wire, port: portID)
                }
                guard found.port.wireIDs.contains(busWire) else {
                    throw SchematicFileError.missingPortPin(wire: wire, port: portID, busWire: busWire)
                }
                return .busPin(BusPinRef(portID: portID, wireID: busWire))
            case let .label(id):
                guard labelIDs.contains(id) else {
                    throw SchematicFileError.missingLabel(wire: wire, label: id)
                }
                return .label(id)
            }
        }

        var placedWires: [SchematicModelWire] = []
        for wire in wires {
            try claim(wire.id)
            placedWires.append(SchematicModelWire(
                id: wire.id,
                start: try resolve(wire.from, wire: wire.id),
                end: try resolve(wire.to, wire: wire.id),
                interior: wire.bends.map { CGPoint(x: $0.x, y: $0.y) }
            ))
        }

        var texts: [BoardText] = []
        for text in board?.texts ?? [] {
            try claim(text.id)
            let placed = try boardText(text, grid: settings.grid)
            if placed.side == .bottom, settings.layers == 1 {
                throw SchematicFileError.badPCB("text \(text.id.uuidString) is on the bottom of a 1-layer board")
            }
            texts.append(placed)
        }

        var traces: [BoardTrace] = []
        for entry in board?.traces ?? [] {
            try claim(entry.id)
            traces.append(try Self.boardTrace(entry, settings: settings))
        }

        var vias: [BoardVia] = []
        for entry in board?.vias ?? [] {
            try claim(entry.id)
            let name = "via \(entry.id.uuidString)"
            guard settings.layers > 1 else {
                throw SchematicFileError.badPCB("\(name) is on a 1-layer board")
            }
            let via = BoardVia(id: entry.id, centre: CGPoint(x: entry.x, y: entry.y),
                               drill: entry.drill ?? BoardVia.defaultDrill,
                               diameter: entry.diameter ?? BoardVia.defaultDiameter,
                               locked: entry.locked ?? false)
            if let problem = BoardVia.problem(drill: via.drill, diameter: via.diameter) {
                throw SchematicFileError.badPCB("\(name) \(problem)")
            }
            vias.append(via)
        }

        var holes: [BoardHole] = []
        for entry in board?.holes ?? [] {
            try claim(entry.id)
            let name = "hole \(entry.id.uuidString)"
            var hole = BoardHole(id: entry.id, centre: CGPoint(x: entry.x, y: entry.y))
            hole.apply(try Self.value(entry.size, of: HoleSize.self, "\(name) size", default: .m3))
            hole.drill = entry.drill ?? hole.drill
            hole.diameter = entry.diameter ?? hole.diameter
            hole.plated = entry.plated ?? false
            hole.locked = entry.locked ?? false
            if let raw = entry.net {
                guard let net = PlaneNet(fileValue: raw) else {
                    throw SchematicFileError.badPCB("\(name) net \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>")
                }
                hole.net = net
            }
            if let problem = BoardHole.problem(drill: hole.drill, diameter: hole.diameter) {
                throw SchematicFileError.badPCB("\(name) \(problem)")
            }
            holes.append(hole)
        }

        var placedNotes: [SchematicNote] = []
        for entry in notes ?? [] {
            try claim(entry.id)
            placedNotes.append(try Self.note(entry))
        }


        // Never hand out a U<n> reference that's already used, even if the
        // stored counter is behind.
        let highest = chips.map { ReferenceParts($0.reference) }
            .filter { $0.prefix == "U" }
            .compactMap(\.number)
            .max() ?? 0
        document.replaceContents(
            chips: chips,
            wires: placedWires,
            junctions: placedJunctions,
            buses: placedBuses,
            nextReference: max(nextReference, highest + 1),
            board: settings,
            texts: texts,
            traces: traces,
            vias: vias,
            holes: holes,
            notes: placedNotes,
            netLabels: placedLabels
        )
    }

    /// A stored net label, with defaults for missing keys. Its point is
    /// snapped to the grid.
    private static func label(_ entry: Label) throws -> NetLabel {
        let name = "net label \(entry.id.uuidString)"
        guard NetLabel.cleanName(entry.name) == entry.name else {
            throw SchematicFileError.badLabel(
                "\(name) name \"\(entry.name)\" must be 1 to \(NetLabel.maxNameLength) characters with no spaces")
        }
        var style = NetLabel.Style.input
        if let type = entry.type {
            guard let parsed = NetLabel.Style(rawValue: type) else {
                throw SchematicFileError.badLabel("\(name) type \"\(type)\" must be input, output, inout, passive or noConnect")
            }
            style = parsed
        }
        return NetLabel(id: entry.id, name: entry.name, style: style,
                        origin: SchematicGrid.snap(CGPoint(x: entry.x, y: entry.y)),
                        isFlipped: entry.flipped ?? false)
    }

    /// A stored text block, with defaults for missing keys. Its top-left is
    /// snapped to the grid.
    private static func note(_ entry: Note) throws -> SchematicNote {
        let name = "text block \(entry.id.uuidString)"
        guard !entry.text.isEmpty else { throw SchematicFileError.badNote("\(name) is empty") }
        var result = SchematicNote(id: entry.id, text: entry.text,
                                   origin: SchematicGrid.snap(CGPoint(x: entry.x, y: entry.y)))
        if let size = entry.size {
            let range = SchematicNote.sizeRange
            guard range.contains(size) else {
                throw SchematicFileError.badNote(
                    "\(name) size \(size) must be \(range.lowerBound) to \(range.upperBound) points")
            }
            result.size = size
        }
        result.framed = entry.framed ?? result.framed
        return result
    }

    // MARK: PCB values

    /// The board's settings, with defaults for missing keys.
    private func boardSettings() throws -> BoardSettings {
        var settings = BoardSettings()
        guard let board else { return settings }
        func size(_ value: Double?, _ name: String) throws -> Double? {
            guard let value else { return nil }
            guard BoardSettings.sizeRange.contains(value) else {
                throw SchematicFileError.badPCB("board \(name) \(Self.number(value)) must be 1 to 1000 mm")
            }
            return value
        }
        settings.width = try size(board.width, "width") ?? settings.width
        settings.height = try size(board.height, "height") ?? settings.height
        if let layers = board.layers {
            guard BoardSettings.layerCounts.contains(layers) else {
                throw SchematicFileError.badPCB("board layers \(layers) must be 1, 2, 4 or 6")
            }
            settings.layers = layers
        }
        settings.color = try Self.value(board.color, of: BoardColor.self, "board color", default: settings.color)
        settings.units = try Self.value(board.units, of: BoardUnits.self, "board units", default: settings.units)
        if let grid = board.grid {
            guard BoardSettings.gridRange.contains(grid) else {
                throw SchematicFileError.badPCB("board grid \(Self.number(grid)) must be 0.01 to 10 mm")
            }
            settings.grid = grid
        }
        settings.styles = try Self.layerStyles(board.layerStyles ?? [], layers: settings.layers)
        settings.type = try Self.value(board.type, of: BoardType.self, "board type", default: .custom)
        settings.edgeBus = board.edgeBus
        if let entry = board.card { settings.customCard = Self.customCard(entry) }
        if let entry = board.cardOptions { settings.cardOptions = try Self.cardOptions(entry) }
        if settings.needsTwoLayers, settings.layers < 2 {
            throw SchematicFileError.badPCB("board type \(settings.type.rawValue) needs 2 or more layers")
        }
        if settings.type == .customCard {
            let card = settings.customCard
            settings.width = max(settings.width, card.indent + card.tabLength)
            settings.height = max(settings.height, card.depth + 1)
        }
        if settings.type != .customCard, let card = settings.card {
            // A card's height is its standard's; its length is at least the least.
            settings.height = card.height
            settings.width = min(max(settings.width, card.minimumLength), card.maximumLength)
        }
        return settings
    }

    /// The stored layer styles of a board with `layers` copper layers, with
    /// defaults for missing colours and types.
    private static func layerStyles(_ entries: [Board.LayerStyleEntry],
                                    layers: Int) throws -> [CopperLayer: LayerStyle] {
        let onBoard = CopperLayer.allCases(for: layers)
        var styles: [CopperLayer: LayerStyle] = [:]
        for entry in entries {
            guard let layer = CopperLayer(rawValue: entry.layer) else {
                throw SchematicFileError.badPCB("layer \"\(entry.layer)\" must be top, inner2 to inner5, or bottom")
            }
            let name = "layer \(layer.rawValue)"
            guard onBoard.contains(layer) else {
                throw SchematicFileError.badPCB("layer \"\(layer.rawValue)\" is not on a \(layers)-layer board")
            }
            guard styles[layer] == nil else {
                throw SchematicFileError.badPCB("\(name) is listed twice")
            }
            var style = LayerStyle.defaultStyle(of: layer)
            if let raw = entry.color {
                guard let color = LayerColor(hex: raw) else {
                    throw SchematicFileError.badPCB("\(name) color \"\(raw)\" must be #RRGGBB")
                }
                style.color = color
            }
            if let raw = entry.type {
                let allowed: [LayerType] = layer.isInner ? [.plane, .wire] : [.component]
                guard let type = LayerType(rawValue: raw), allowed.contains(type) else {
                    let list = allowed.map(\.rawValue).joined(separator: " or ")
                    throw SchematicFileError.badPCB("\(name) type \"\(raw)\" must be \(list)")
                }
                style.type = type
            }
            if let raw = entry.net {
                guard layer.isInner, let net = PlaneNet(fileValue: raw) else {
                    throw SchematicFileError.badPCB(layer.isInner
                        ? "\(name) net \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>"
                        : "\(name) is not a plane; it has no net")
                }
                style.net = net
            }
            styles[layer] = style
        }
        return styles
    }

    /// A stored trace, checked against the board's layers.
    private static func boardTrace(_ entry: Board.TraceEntry, settings: BoardSettings) throws -> BoardTrace {
        let name = "trace \(entry.id.uuidString)"
        guard let layer = CopperLayer(rawValue: entry.layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" must be top, inner2 to inner5, or bottom")
        }
        guard settings.copperLayers.contains(layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" is not on a \(settings.layers)-layer board")
        }
        guard settings.style(of: layer).type != .plane else {
            throw SchematicFileError.badPCB("\(name) layer \(layer.rawValue) is a plane")
        }
        let width = entry.width ?? BoardTrace.defaultWidth
        guard BoardTrace.widthRange.contains(width) else {
            throw SchematicFileError.badPCB("\(name) width \(width) must be 5 to 100 mil")
        }
        let points = entry.points.map { CGPoint(x: $0.x, y: $0.y) }
        if let problem = TraceRoute.problem(points) {
            throw SchematicFileError.badPCB("\(name) \(problem)")
        }
        return BoardTrace(id: entry.id, layer: layer, width: width, points: points, locked: entry.locked ?? false)
    }

    /// A stored footprint placement, with defaults for missing keys.
    private func placement(_ pcb: Placement, reference: String) throws -> BoardPlacement {
        var placement = BoardPlacement(centre: CGPoint(x: pcb.x, y: pcb.y), rotation: pcb.rotation)
        // A placement saved without an origin is from before origins: those
        // were all top left. New placements default to pad 1's centre.
        placement.origin = try Self.value(pcb.origin, of: FootprintOrigin.self, "\(reference) origin",
                                          default: .topLeft)
        placement.outline = try Self.value(pcb.outline, of: MarkLayer.self, "\(reference) outline",
                                           default: placement.outline)
        placement.pin1 = try Self.value(pcb.pin1, of: MarkLayer.self, "\(reference) pin1", default: placement.pin1)
        placement.reference = try Self.value(pcb.reference, of: MarkLayer.self, "\(reference) reference",
                                             default: placement.reference)
        placement.side = try Self.value(pcb.side, of: BoardSide.self, "\(reference) side", default: placement.side)
        placement.locked = pcb.locked ?? false
        return placement
    }

    /// A stored board text, with defaults for missing keys. Its anchor is
    /// snapped to the component grid (`grid`, mm).
    private func boardText(_ text: Board.Text, grid: Double) throws -> BoardText {
        let name = "text \(text.id.uuidString)"
        guard !text.text.isEmpty else {
            throw SchematicFileError.badPCB("\(name) is empty")
        }
        var result = BoardText(id: text.id, text: text.text, anchor: BoardGrid.snap(CGPoint(x: text.x, y: text.y), spacing: grid),
                               size: BoardText.defaultSize)
        if let size = text.size {
            guard BoardText.sizeRange.contains(size) else {
                throw SchematicFileError.badPCB("\(name) size \(size) must be 10 to 1000 mil")
            }
            result.size = size
        }
        result.layer = try Self.value(text.layer, of: TextLayer.self, "\(name) layer", default: result.layer)
        result.side = try Self.value(text.side, of: BoardSide.self, "\(name) side", default: result.side)
        result.locked = text.locked ?? false
        if let rotation = text.rotation {
            guard BoardPlacement.rotations.contains(rotation) else {
                throw SchematicFileError.badPCB("\(name) rotation \(rotation) must be 0, 90, 180 or 270")
            }
            result.rotation = rotation
        }
        return result
    }

    /// The case whose raw value is `raw`, `fallback` when there is none, or
    /// a `badPCB` error listing the allowed values.
    // MARK: Card settings (the board's, and a bus's)

    static func entry(_ card: CustomCard) -> Board.CardEntry {
        Board.CardEntry(pads: card.pads, pitch: card.pitch, padWidth: card.padWidth, padLength: card.padLength,
                        padOffset: card.padOffset, depth: card.depth, indent: card.indent, margin: card.margin,
                        chamfer: card.chamfer, bothSides: card.bothSides)
    }

    static func entry(_ options: CardOptions) -> Board.OptionsEntry {
        Board.OptionsEntry(lanes: options.pcieLanes, lowProfile: options.lowProfile, length: options.m2Length,
                           key: options.m2Key.rawValue, halfSize: options.halfSize,
                           memory: options.memory.rawValue, registered: options.registered,
                           icsp: options.icsp)
    }

    /// A stored custom card edge; missing keys are the defaults, and
    /// out-of-range settings are brought into range, as the sidebar does.
    static func customCard(_ entry: Board.CardEntry) -> CustomCard {
        var card = CustomCard()
        card.pads = entry.pads ?? card.pads
        card.pitch = entry.pitch ?? card.pitch
        card.padWidth = entry.padWidth ?? card.padWidth
        card.padLength = entry.padLength ?? card.padLength
        card.padOffset = entry.padOffset ?? card.padOffset
        card.depth = entry.depth ?? card.depth
        card.indent = entry.indent ?? card.indent
        card.margin = entry.margin ?? card.margin
        card.chamfer = entry.chamfer ?? card.chamfer
        card.bothSides = entry.bothSides ?? card.bothSides
        return card.clamped()
    }

    /// Stored card settings; missing keys are the defaults. Throws `badPCB`.
    static func cardOptions(_ entry: Board.OptionsEntry) throws -> CardOptions {
        var options = CardOptions()
        if let lanes = entry.lanes {
            guard CardOptions.pcieLaneCounts.contains(lanes) else {
                throw SchematicFileError.badPCB("card lanes \(lanes) must be 1, 4, 8 or 16")
            }
            options.pcieLanes = lanes
        }
        options.lowProfile = entry.lowProfile ?? options.lowProfile
        if let length = entry.length {
            guard CardOptions.m2Lengths.contains(length) else {
                throw SchematicFileError.badPCB("card length \(length) must be 30, 42, 60, 80 or 110")
            }
            options.m2Length = length
        }
        options.m2Key = try Self.value(entry.key, of: M2Key.self, "card key", default: options.m2Key)
        options.halfSize = entry.halfSize ?? options.halfSize
        options.memory = try Self.value(entry.memory, of: MemoryGeneration.self, "card memory",
                                        default: options.memory)
        options.registered = entry.registered ?? options.registered
        options.icsp = entry.icsp ?? options.icsp
        return options
    }

    private static func value<Choice: RawRepresentable & CaseIterable>(
        _ raw: String?, of type: Choice.Type, _ name: String, default fallback: Choice
    ) throws -> Choice where Choice.RawValue == String {
        guard let raw else { return fallback }
        if let value = Choice(rawValue: raw) { return value }
        let names = Choice.allCases.map(\.rawValue)
        let list = names.count == 2
            ? "\(names[0]) or \(names[1])"
            : names.dropLast().joined(separator: ", ") + " or " + names.last!
        throw SchematicFileError.badPCB("\(name) \"\(raw)\" must be \(list)")
    }

    /// A number as written in a message: 0 or 1000.5, not 0.0.
    private static func number(_ value: Double) -> String {
        value == value.rounded() && abs(value) < 1e15 ? String(Int(value)) : String(value)
    }
}

// MARK: Bus values

extension SchematicFile {
    /// A file bus as a model bus, checked: wire count and names, straight
    /// lines on the grid, no overlaps, only T and L joints, one piece.
    @MainActor
    fileprivate static func modelBus(_ bus: Bus) throws -> SchematicModelBus {
        let id = bus.id.uuidString
        let lines = bus.lines.map {
            BusLine(id: $0.id, a: CGPoint(x: $0.a.x, y: $0.a.y), b: CGPoint(x: $0.b.x, y: $0.b.y))
        }
        var model = SchematicModelBus(
            id: bus.id, lines: lines, wires: bus.wires.map { BusWire(id: $0.id, name: $0.name) })
        model.name = bus.name?.trimmingCharacters(in: .whitespacesAndNewlines) ?? ""
        // Its type, kept only while the wires are still exactly the card's pins.
        do {
            var card = BusCard()
            card.type = try Self.value(bus.type, of: BoardType.self, "type", default: .custom)
            if let entry = bus.card { card.customCard = Self.customCard(entry) }
            if let entry = bus.cardOptions { card.cardOptions = try Self.cardOptions(entry) }
            if card.pinNames == nil || card.pinNames == model.wires.map(\.name) { model.card = card }
        } catch let SchematicFileError.badPCB(detail) {
            throw SchematicFileError.badBus("bus \(id) \(detail)")
        }

        switch model.problem {
        case nil:
            break
        case .noLines?:
            throw SchematicFileError.badBus("bus \(id) has no lines")
        case let .notStraight(line)?:
            throw SchematicFileError.badBus("line \(line.uuidString) is not a horizontal or vertical line on the grid")
        case let .overlap(first, second)?:
            throw SchematicFileError.badBus("lines \(first.uuidString) and \(second.uuidString) overlap")
        case let .badJoint(_, point)?:
            throw SchematicFileError.badBus("bus \(id) has a joint at \(pointText(point)) that is not a T or an L")
        case .notOnePiece?:
            throw SchematicFileError.badBus("bus \(id) is not one piece")
        }

        guard SchematicModelBus.wireCountRange.contains(bus.wires.count) else {
            throw SchematicFileError.badBus("bus \(id) has \(bus.wires.count) wires; it must have 2 to 999")
        }
        var seen = Set<String>()
        for wire in bus.wires {
            guard SchematicModelBus.nameLengthRange.contains(wire.name.count),
                  !wire.name.contains(where: \.isWhitespace) else {
                throw SchematicFileError.badBus("bus \(id) wire name \"\(wire.name)\" must be 1 to 4 characters with no spaces")
            }
            guard seen.insert(wire.name.lowercased()).inserted else {
                throw SchematicFileError.badBus("bus \(id) uses the wire name \"\(wire.name)\" twice")
            }
        }

        // Ports: on a line of this bus, a side that suits the line, wires of
        // this bus, and fitting where they are.
        var withPorts = model
        let wireIDs = Set(model.wires.map(\.id))
        for entry in bus.ports ?? [] {
            let portID = entry.id.uuidString
            guard let line = model.line(entry.line) else {
                throw SchematicFileError.badBus("port \(portID) line \(entry.line.uuidString) is not on bus \(id)")
            }
            guard let side = sideValue(entry.side), BusPort.sides(for: line).contains(side) else {
                let allowed = line.isHorizontal ? "top or bottom on a horizontal line" : "left or right on a vertical line"
                throw SchematicFileError.badBus("port \(portID) side \"\(entry.side)\" must be \(allowed)")
            }
            for wire in entry.wires where !wireIDs.contains(wire) {
                throw SchematicFileError.badBus("port \(portID) wire \(wire.uuidString) is not on bus \(id)")
            }
            withPorts.ports.append(BusPort(id: entry.id, line: entry.line, start: CGFloat(entry.start),
                                           side: side, wireIDs: Set(entry.wires)))
        }
        for port in withPorts.ports
            where withPorts.fit(port: port, slots: withPorts.slots(of: port), preferredStart: port.start) != port.start {
            throw SchematicFileError.badBus("port \(port.id.uuidString) does not fit on its line")
        }
        return withPorts
    }

    fileprivate static func sideName(_ side: ChipLayout.Side) -> String {
        switch side {
        case .top: "top"
        case .bottom: "bottom"
        case .left: "left"
        case .right: "right"
        }
    }

    fileprivate static func sideValue(_ name: String) -> ChipLayout.Side? {
        switch name {
        case "top": .top
        case "bottom": .bottom
        case "left": .left
        case "right": .right
        default: nil
        }
    }

    /// A point as written in a message: (100, 200).
    fileprivate static func pointText(_ point: CGPoint) -> String {
        "(\(number(Double(point.x))), \(number(Double(point.y))))"
    }
}

/// The model's types, named apart from the file's nested types of the same name.
typealias SchematicModelWire = Wire
typealias SchematicModelJunction = Junction
typealias SchematicModelBus = Bus

private extension SchematicFile.End {
    /// Saves a pin by its designator, looked up on the placed chip.
    init(_ end: WireEnd, chips: [PlacedChip]) {
        switch end {
        case let .pin(ref):
            let pins = chips.first { $0.id == ref.chipID }?.part.pins ?? []
            let designator = pins.indices.contains(ref.index) ? pins[ref.index].designator : "\(ref.index + 1)"
            self = .pin(part: ref.chipID, pin: .designator(designator))
        case let .junction(id):
            self = .junction(id)
        case let .busPin(ref):
            self = .port(port: ref.portID, wire: ref.wireID)
        case let .label(id):
            self = .label(id)
        }
    }
}
