import CoreGraphics
import CryptoKit
import Foundation

/// File > Save As…, format EasyEDA Pro Project: the schematic and its board
/// as an EasyEDA Pro (JLCEDA Pro) project archive, `.epro`. See
/// `docs/export.md` § EasyEDA Pro.
///
/// The archive is the format EasyEDA Pro's File ▸ Import ▸ EasyEDA Professional
/// takes (format "V2", the one EasyEDA's spec and KiCad's importer describe):
/// a zip of
///
/// - `project.json`: devices, symbols, footprints, schematics, pcbs, boards;
/// - `SYMBOL/<id>.esym`: one symbol per distinct part, power symbol and pull
///   resistor (`EasyEDAProSchematicWriter`);
/// - `FOOTPRINT/<id>.efoo`: one footprint per distinct footprint
///   (`EasyEDAProBoardWriter`);
/// - `SHEET/<schematic id>/1.esch`: the schematic's one sheet;
/// - `PCB/<pcb id>.epcb`: the board.
///
/// Every file is JSON lines: each line one JSON array, the first element
/// naming the item (`["WIRE", "e12", …]`). Lengths are mil on the board and
/// 0.01 inch on the schematic. An export is not a save: the window's file
/// and its unsaved-changes state do not change.
@MainActor
enum EasyEDAProExport {
    static let fileExtension = "epro"

    /// The archive's files, in order.
    static func entries(for document: SchematicDocument, base: String) -> [(name: String, data: Data)] {
        let build = Build(document: document, base: base)
        let schematic = EasyEDAProSchematicWriter(build).sheet()
        let board = EasyEDAProBoardWriter(build).board()
        var entries: [(name: String, data: Data)] = [("project.json", Data(project(build).utf8))]
        for symbol in build.symbols { entries.append(("SYMBOL/\(symbol.id).esym", Data(symbol.text.utf8))) }
        for footprint in build.footprints { entries.append(("FOOTPRINT/\(footprint.id).efoo", Data(footprint.text.utf8))) }
        entries.append(("SHEET/\(build.schematicID)/1.esch", Data(schematic.utf8)))
        entries.append(("PCB/\(build.pcbID).epcb", Data(board.utf8)))
        return entries
    }

    /// The project as one `.epro` zip. The same document gives the same bytes.
    static func data(for document: SchematicDocument, base: String) -> Data {
        ZipArchive.stored(entries(for: document, base: base), date: Date(timeIntervalSinceReferenceDate: 0))
    }

    /// Writes `<base>.epro` into `directory`, replacing a file of that name.
    static func write(_ document: SchematicDocument, base: String, to directory: URL) throws {
        try data(for: document, base: base).write(to: directory.appendingPathComponent("\(base).\(fileExtension)"),
                                                  options: .atomic)
    }

    // MARK: project.json

    private static func project(_ build: Build) -> String {
        func attributes(_ pairs: [(String, String)]) -> EProJSON { .object(pairs.map { ($0.0, .string($0.1)) }) }
        let devices: [(String, EProJSON)] = build.devices.map { device in
            (device.id, .object([
                ("title", .string(device.title)),
                ("source", ""),
                ("version", ""),
                ("description", .string(device.description)),
                ("tags", []),
                ("images", []),
                ("attributes", attributes(device.attributes)),
            ]))
        }
        func asset(_ title: String, type: Int) -> EProJSON {
            .object([("title", .string(title)), ("source", ""), ("version", ""), ("type", .int(type)), ("desc", ""),
                     ("tags", [])])
        }
        let symbols: [(String, EProJSON)] = build.symbols.map { ($0.id, asset($0.title, type: $0.type)) }
        let footprints: [(String, EProJSON)] = build.footprints.map { ($0.id, asset($0.title, type: 4)) }
        let project: EProJSON = .object([
            ("devices", .object(devices)),
            ("symbols", .object(symbols)),
            ("footprints", .object(footprints)),
            ("schematics", .object([(build.schematicID, .object([
                ("name", .string(build.base)),
                ("sheets", .array([.object([("id", .int(1)), ("name", "Sheet1"), ("uuid", .string(build.sheetID))])])),
            ]))])),
            ("pcbs", .object([(build.pcbID, .string(build.base))])),
            ("boards", .object([(build.base, .object([("schematic", .string(build.schematicID)),
                                                     ("pcb", .string(build.pcbID))]))])),
            ("config", .object([("title", .string(build.base)), ("defaultSheet", ""), ("cbbProject", false)])),
        ])
        return project.text + "\n"
    }

    // MARK: What the writers share

    /// The project being written: the shared net names, and the symbols,
    /// footprints and devices as the writers register them.
        nonisolated final class Build {
        struct Symbol {
            let id: String
            let title: String
            let type: Int
            let text: String
        }

        struct Footprint {
            let id: String
            let title: String
            let text: String
        }

        struct Device {
            let id: String
            let title: String
            let description: String
            let attributes: [(String, String)]
        }

        let document: SchematicDocument
        let base: String
        let context: KiCadExport.Context
        let schematicID: String
        let pcbID: String
        let sheetID: String
        private(set) var symbols: [Symbol] = []
        private(set) var footprints: [Footprint] = []
        private(set) var devices: [Device] = []
        private var symbolByKey: [String: String] = [:]
        private var footprintByKey: [String: String] = [:]
        private var deviceByKey: [String: String] = [:]
        private var assetsByChip: [UUID: Assets] = [:]

        /// What a part is made of in the project.
        struct Assets {
            let symbol: String
            let footprint: String?
            let device: String
        }

        /// The symbol, footprint and device of `chip`, made once.
        @MainActor func assets(for chip: PlacedChip) -> Assets {
            if let known = assetsByChip[chip.id] { return known }
            let symbol = EasyEDAProSchematicWriter.symbolID(for: chip, build: self)
            let footprint = EasyEDAProBoardWriter.footprintID(for: chip, build: self)
            var attributes: [(String, String)] = [("Symbol", symbol)]
            if let footprint { attributes.append(("Footprint", footprint)) }
            if case let .supply(_, mv, false) = chip.part.details {
                attributes.append(("Global Net Name", KiCadExport.Context.railName(mv: mv)))
            } else {
                let prefix = String(chip.reference.prefix { $0.isLetter })
                attributes += [("Designator", (prefix.isEmpty ? "U" : prefix) + "?"), ("Name", chip.partNumber),
                               ("Add into BOM", "yes"), ("Convert to PCB", footprint == nil ? "no" : "yes")]
            }
            let key = "\(symbol)|\(footprint ?? "")"
            let device = self.device(key, title: chip.partNumber, description: chip.part.manufacturer, attributes: attributes)
            let assets = Assets(symbol: symbol, footprint: footprint, device: device)
            assetsByChip[chip.id] = assets
            return assets
        }

        @MainActor init(document: SchematicDocument, base: String) {
            self.document = document
            self.base = base
            context = KiCadExport.Context(document: document, base: base)
            schematicID = EProJSON.id("schematic/\(base)")
            pcbID = EProJSON.id("pcb/\(base)")
            sheetID = EProJSON.id("sheet/\(base)")
        }

        /// The id of the symbol made by `make` for `key`, made once.
        @MainActor func symbol(_ key: String, title: String, type: Int, make: () -> String) -> String {
            if let id = symbolByKey[key] { return id }
            let id = EProJSON.id("symbol/\(key)")
            symbolByKey[key] = id
            symbols.append(Symbol(id: id, title: title, type: type, text: make()))
            return id
        }

        @MainActor func footprint(_ key: String, title: String, make: () -> String) -> String {
            if let id = footprintByKey[key] { return id }
            let id = EProJSON.id("footprint/\(key)")
            footprintByKey[key] = id
            footprints.append(Footprint(id: id, title: title, text: make()))
            return id
        }

        @MainActor func device(_ key: String, title: String, description: String, attributes: [(String, String)]) -> String {
            if let id = deviceByKey[key] { return id }
            let id = EProJSON.id("device/\(key)")
            deviceByKey[key] = id
            devices.append(Device(id: id, title: title, description: description, attributes: attributes))
            return id
        }

        /// A net's name in the project: a rail's, a label's, or after its
        /// first pin; nil for a net of one pin (EasyEDA names nothing there).
        @MainActor func netName(_ net: Int) -> String? {
            let name = ExportNetNames.baseName(ofNet: net, context: context) { $0 }
            return name.hasPrefix("unconnected-(") ? nil : name
        }

        /// Element ids `e1`, `e2`, … for one file.
        struct Counter {
            private var next = 0
            mutating func id() -> String {
                next += 1
                return "e\(next)"
            }
        }
    }
}

// MARK: - JSON

/// One JSON value, written the way EasyEDA's files are: compact, lengths to
/// 4 decimals with no trailing zeros, no escaped slashes.
indirect enum EProJSON: ExpressibleByStringLiteral, ExpressibleByIntegerLiteral, ExpressibleByFloatLiteral,
                        ExpressibleByArrayLiteral, ExpressibleByNilLiteral, ExpressibleByBooleanLiteral {
    case string(String)
    case number(Double)
    case int(Int)
    case bool(Bool)
    case null
    case array([EProJSON])
    case object([(String, EProJSON)])

    init(stringLiteral value: String) { self = .string(value) }
    init(integerLiteral value: Int) { self = .int(value) }
    init(floatLiteral value: Double) { self = .number(value) }
    init(booleanLiteral value: Bool) { self = .bool(value) }
    init(nilLiteral: ()) { self = .null }
    init(arrayLiteral elements: EProJSON...) { self = .array(elements) }

    static func num(_ value: Double) -> EProJSON { .number(value) }
    static func num(_ value: CGFloat) -> EProJSON { .number(Double(value)) }
    static func str(_ value: String?) -> EProJSON { value.map { .string($0) } ?? .null }

    var text: String {
        switch self {
        case let .string(value): Self.quote(value)
        case let .number(value): Self.format(value)
        case let .int(value): String(value)
        case let .bool(value): value ? "true" : "false"
        case .null: "null"
        case let .array(values): "[" + values.map(\.text).joined(separator: ",") + "]"
        case let .object(pairs): "{" + pairs.map { Self.quote($0.0) + ":" + $0.1.text }.joined(separator: ",") + "}"
        }
    }

    static func format(_ value: Double) -> String {
        guard value.isFinite else { return "0" }
        var text = String(format: "%.4f", value)
        while text.hasSuffix("0") { text.removeLast() }
        if text.hasSuffix(".") { text.removeLast() }
        return text == "-0" || text.isEmpty ? "0" : text
    }

    static func quote(_ text: String) -> String {
        var out = "\""
        for scalar in text.unicodeScalars {
            switch scalar {
            case "\"": out += "\\\""
            case "\\": out += "\\\\"
            case "\n": out += "\\n"
            case "\r": out += "\\r"
            case "\t": out += "\\t"
            default:
                if scalar.value < 0x20 { out += String(format: "\\u%04x", scalar.value) } else { out.unicodeScalars.append(scalar) }
            }
        }
        return out + "\""
    }

    /// An id of 16 hex digits made from `key`, so the same document gives the
    /// same files.
    static func id(_ key: String) -> String {
        SHA256.hash(data: Data(key.utf8)).prefix(8).map { String(format: "%02x", $0) }.joined()
    }

    /// A line of a file: an array, then a newline.
    static func line(_ items: EProJSON...) -> String { EProJSON.array(items).text + "\n" }
}
