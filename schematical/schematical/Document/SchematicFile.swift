import CoreGraphics
import Foundation
import UniformTypeIdentifiers

// MARK: - File format

/// The on-disk form of a schematic, version 2. See `docs/file-format.md`.
/// Version 1 files (pins by number) are converted when opened.
///
/// Parts are stored by part number only. Pins, layout and symbol come from the
/// backend catalog when the file is opened.
nonisolated struct SchematicFile: Codable, Equatable, Sendable {
    static let formatName = "schematical"
    static let currentVersion = 2
    static let fileExtension = "schematical"

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
        /// footprint (VCC, GND).
        var pcb: Placement? = nil
        /// EEPROMs only: the file loaded into the chip when the simulation
        /// starts. Left out when there is none and for other parts.
        var memoryFile: MemoryFileEntry? = nil
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
    }

    /// The PCB: size in mm, copper layers, colour and sidebar units (raw
    /// values of `BoardColor` and `BoardUnits`), and free texts. Missing keys
    /// mean the defaults of `BoardSettings`.
    struct Board: Codable, Equatable, Sendable {
        var width: Double? = nil
        var height: Double? = nil
        var layers: Int? = nil
        var color: String? = nil
        var units: String? = nil
        var texts: [Text]? = nil

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
        }
    }

    struct Junction: Codable, Equatable, Sendable {
        var id: UUID
        var x: Double
        var y: Double
    }

    /// How a wire end names its pin: by designator (version 2, `"A1"`), or by
    /// number (version 1, `14`: the part's 14th pin in its pinout).
    enum PinKey: Equatable, Sendable {
        case designator(String)
        case position(Int)
    }

    /// One end of a wire: `{"part": <id>, "pin": "<designator>"}` or `{"junction": <id>}`.
    enum End: Codable, Equatable, Sendable {
        case pin(part: UUID, pin: PinKey)
        case junction(UUID)

        private enum CodingKeys: String, CodingKey {
            case part, pin, junction
        }

        init(from decoder: Decoder) throws {
            let container = try decoder.container(keyedBy: CodingKeys.self)
            if let junction = try container.decodeIfPresent(UUID.self, forKey: .junction) {
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

    var format: String
    var version: Int
    var nextReference: Int
    var parts: [Part]
    var junctions: [Junction]
    var wires: [Wire]
    /// The PCB's settings and texts. Always written; files from before it
    /// open with the defaults.
    var board: Board? = nil
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
    case badRotation(reference: String, rotation: Int)
    /// A board setting, footprint origin or mark, or board text that isn't
    /// valid; the detail names it.
    case badPCB(String)

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
        case let .badRotation(reference, rotation):
            "\(reference) has PCB rotation \(rotation); it must be 0, 90, 180 or 270."
        case let .badPCB(detail):
            "The PCB settings are not valid: \(detail)."
        }
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
                switches: chip.switchesOn.isEmpty ? nil : chip.switchesOn,
                pcb: chip.board.map {
                    Placement(x: $0.centre.x, y: $0.centre.y, rotation: $0.rotation,
                              origin: $0.origin.rawValue, outline: $0.outline.rawValue,
                              pin1: $0.pin1.rawValue, reference: $0.reference.rawValue)
                },
                memoryFile: chip.memoryFile.map {
                    MemoryFileEntry(path: MemoryFileAccess.path(of: $0.url, schematic: fileURL),
                                    bookmark: $0.bookmark)
                }
            )
        }
        junctions = document.junctions.map { Junction(id: $0.id, x: $0.point.x, y: $0.point.y) }
        wires = document.wires.map { wire in
            Wire(
                id: wire.id,
                from: End(wire.start, chips: document.chips),
                to: End(wire.end, chips: document.chips),
                bends: wire.interior.map { Point(x: $0.x, y: $0.y) }
            )
        }
        let settings = document.board
        board = Board(
            width: settings.width,
            height: settings.height,
            layers: settings.layers,
            color: settings.color.rawValue,
            units: settings.units.rawValue,
            texts: document.texts.map { text in
                Board.Text(id: text.id, text: text.text, x: text.anchor.x, y: text.anchor.y,
                           size: text.size, layer: text.layer.rawValue, rotation: text.rotation)
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
            guard let catalogPart = catalog.part(named: part.partNumber) else {
                unknown.append(part.partNumber)
                continue
            }
            let partSymbol = symbol(for: catalogPart)
            // A stored placement counts only for a part with a footprint.
            var board: BoardPlacement?
            if let pcb = part.pcb, let footprint = catalogPart.footprint {
                guard BoardPlacement.rotations.contains(pcb.rotation) else {
                    throw SchematicFileError.badRotation(reference: part.reference, rotation: pcb.rotation)
                }
                // A placement from an older file (no origin) is snapped by its
                // top-left, as those builds did. One with an origin is kept as
                // saved: changing the origin doesn't move a footprint, so its
                // origin point may be off the grid, and opening must not move it.
                let stored = try placement(pcb, reference: part.reference)
                board = pcb.origin == nil
                    ? BoardLayout.snapped(stored, footprint: footprint, pad1: catalogPart.firstPinDesignator)
                    : stored
            }
            // A load file counts only on an EEPROM.
            var memoryFile: MemoryFile?
            if catalogPart.kind == PART_EEPROM, let entry = part.memoryFile {
                memoryFile = MemoryFileAccess.file(path: entry.path, bookmark: entry.bookmark, schematic: fileURL)
            }
            chips.append(PlacedChip(
                id: part.id,
                reference: part.reference,
                part: catalogPart,
                symbol: partSymbol,
                origin: SchematicGrid.snap(CGPoint(x: part.x, y: part.y)),
                isFlipped: part.flipped && partSymbol.canFlip,
                switchesOn: part.switches ?? [],
                board: board,
                memoryFile: memoryFile
            ))
        }
        if !unknown.isEmpty {
            throw SchematicFileError.unknownParts(Array(Set(unknown)).sorted())
        }
        // Footprints with no stored place (older files) go after the stored
        // ones, in file order.
        for index in chips.indices where chips[index].board == nil {
            if let footprint = chips[index].part.footprint {
                chips[index].board = BoardLayout.nextSpot(for: footprint, among: chips)
            }
        }

        var placedJunctions: [SchematicModelJunction] = []
        for junction in junctions {
            try claim(junction.id)
            placedJunctions.append(SchematicModelJunction(id: junction.id, point: CGPoint(x: junction.x, y: junction.y)))
        }
        let junctionIDs = Set(placedJunctions.map(\.id))
        let chipsByID = Dictionary(uniqueKeysWithValues: chips.map { ($0.id, $0) })

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
            texts.append(try boardText(text))
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
            nextReference: max(nextReference, highest + 1),
            board: settings,
            texts: texts
        )
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
        return settings
    }

    /// A stored footprint placement, with defaults for missing keys.
    private func placement(_ pcb: Placement, reference: String) throws -> BoardPlacement {
        var placement = BoardPlacement(centre: CGPoint(x: pcb.x, y: pcb.y), rotation: pcb.rotation)
        placement.origin = try Self.value(pcb.origin, of: FootprintOrigin.self, "\(reference) origin",
                                          default: placement.origin)
        placement.outline = try Self.value(pcb.outline, of: MarkLayer.self, "\(reference) outline",
                                           default: placement.outline)
        placement.pin1 = try Self.value(pcb.pin1, of: MarkLayer.self, "\(reference) pin1", default: placement.pin1)
        placement.reference = try Self.value(pcb.reference, of: MarkLayer.self, "\(reference) reference",
                                             default: placement.reference)
        return placement
    }

    /// A stored board text, with defaults for missing keys. Its anchor is
    /// snapped to the grid.
    private func boardText(_ text: Board.Text) throws -> BoardText {
        let name = "text \(text.id.uuidString)"
        guard !text.text.isEmpty else {
            throw SchematicFileError.badPCB("\(name) is empty")
        }
        var result = BoardText(id: text.id, text: text.text, anchor: BoardGrid.snap(CGPoint(x: text.x, y: text.y)),
                               size: BoardText.defaultSize)
        if let size = text.size {
            guard BoardText.sizeRange.contains(size) else {
                throw SchematicFileError.badPCB("\(name) size \(size) must be 10 to 1000 mil")
            }
            result.size = size
        }
        result.layer = try Self.value(text.layer, of: TextLayer.self, "\(name) layer", default: result.layer)
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

/// The model's types, named apart from the file's nested types of the same name.
typealias SchematicModelWire = Wire
typealias SchematicModelJunction = Junction

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
        }
    }
}
