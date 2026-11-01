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
        }
    }
}

// MARK: - Converting to and from the document

extension SchematicFile {
    /// A snapshot of the document. The selection is not saved.
    @MainActor
    init(document: SchematicDocument) {
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
                switches: chip.switchesOn.isEmpty ? nil : chip.switchesOn
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
        encoder.outputFormatting = [.prettyPrinted, .withoutEscapingSlashes]
        return try encoder.encode(self)
    }

    /// Rebuilds the document's contents, looking up each part in the catalog.
    /// Throws without touching the document if anything doesn't resolve.
    @MainActor
    func apply(to document: SchematicDocument, catalog: PartCatalogStore) throws {
        var seen = Set<UUID>()
        func claim(_ id: UUID) throws {
            guard seen.insert(id).inserted else { throw SchematicFileError.duplicateID(id) }
        }

        var chips: [PlacedChip] = []
        var unknown: [String] = []
        for part in parts {
            try claim(part.id)
            guard let catalogPart = catalog.part(named: part.partNumber) else {
                unknown.append(part.partNumber)
                continue
            }
            let partSymbol = symbol(for: catalogPart)
            chips.append(PlacedChip(
                id: part.id,
                reference: part.reference,
                part: catalogPart,
                symbol: partSymbol,
                origin: SchematicGrid.snap(CGPoint(x: part.x, y: part.y)),
                isFlipped: part.flipped && partSymbol.canFlip,
                switchesOn: part.switches ?? []
            ))
        }
        if !unknown.isEmpty {
            throw SchematicFileError.unknownParts(Array(Set(unknown)).sorted())
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

        // Never hand out a reference that's already used, even if the stored
        // counter is behind.
        let highest = chips.compactMap { Int($0.reference.dropFirst()) }.max() ?? 0
        document.replaceContents(
            chips: chips,
            wires: placedWires,
            junctions: placedJunctions,
            nextReference: max(nextReference, highest + 1)
        )
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
