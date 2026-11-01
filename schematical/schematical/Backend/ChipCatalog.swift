import Foundation
import Observation

// MARK: - Backend contract

/// Pin role as reported by the backend (`PinRole` in chip_definition.h).
nonisolated enum PinRole: String, Codable, Hashable, Sendable {
    case input
    case output
    case inOut = "inout"
    case power
    case ground
    case noConnect
}

nonisolated struct ChipPin: Codable, Hashable, Sendable, Identifiable {
    let number: Int
    let name: String
    let role: PinRole

    var id: Int { number }
}

/// One orderable part number, with the package and family the model reports for it.
/// The flags come from the backend's `PART_FLAG_*` bits. Parts the backend
/// sends without them (and LEDs and oscillators, built in the app) count as
/// current, commercial parts.
nonisolated struct ChipPart: Codable, Hashable, Sendable, Identifiable {
    let name: String
    let package: String
    let family: String
    let obsolete: Bool
    let military: Bool
    let automotive: Bool

    var id: String { name }

    init(name: String, package: String, family: String,
         obsolete: Bool = false, military: Bool = false, automotive: Bool = false) {
        self.name = name
        self.package = package
        self.family = family
        self.obsolete = obsolete
        self.military = military
        self.automotive = automotive
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        name = try container.decode(String.self, forKey: .name)
        package = try container.decode(String.self, forKey: .package)
        family = try container.decode(String.self, forKey: .family)
        obsolete = try container.decodeIfPresent(Bool.self, forKey: .obsolete) ?? false
        military = try container.decodeIfPresent(Bool.self, forKey: .military) ?? false
        automotive = try container.decodeIfPresent(Bool.self, forKey: .automotive) ?? false
    }
}

/// One chip family, exactly as the backend describes it.
nonisolated struct ChipDefinition: Codable, Hashable, Sendable, Identifiable {
    let id: String
    let function: String
    let description: String
    let category: String
    let gateCount: Int
    let parts: [ChipPart]
    let pins: [ChipPin]
}

/// One generic LED, as the backend reports it. `color` and `package` are labels
/// such as "Red" and "0603". Pins are 1 = cathode (K) and 2 = anode (A).
nonisolated struct LedDefinition: Codable, Hashable, Sendable, Identifiable {
    let name: String
    let color: String
    let package: String
    let pins: [ChipPin]

    var id: String { name }
}

/// A constant source, VCC or GND, as the backend reports it. Each has one pin.
nonisolated struct SupplyDefinition: Codable, Hashable, Sendable, Identifiable {
    let name: String
    let pins: [ChipPin]

    var id: String { name }
}

/// One orderable oscillator, as the backend reports it. `enablePin` is the
/// 1-based pin number of the output-enable or standby pin, when the part has one.
nonisolated struct OscillatorDefinition: Codable, Hashable, Sendable, Identifiable {
    let name: String
    let family: String
    let package: String
    let enablePin: Int?
    let pins: [ChipPin]

    var id: String { name }
}

/// Oscillators in one frequency band, as the backend groups them.
nonisolated struct OscillatorGroup: Codable, Hashable, Sendable {
    let range: String
    let parts: [OscillatorDefinition]
}

/// The oscillator section of the catalog: its drawer category and its bands.
nonisolated struct OscillatorSection: Codable, Hashable, Sendable {
    let category: String
    let groups: [OscillatorGroup]
}

nonisolated struct ChipCatalog: Codable, Hashable, Sendable {
    let chips: [ChipDefinition]
    let leds: [LedDefinition]
    let supplies: [SupplyDefinition]
    let oscillators: OscillatorSection?
}

/// Reads the catalog from the statically linked backend (`catalog_json_copy`).
nonisolated enum BackendCatalog {
    enum LoadError: LocalizedError {
        case unavailable
        case malformed(String)

        var errorDescription: String? {
            switch self {
            case .unavailable:
                return "Backend returned no catalog"
            case .malformed(let detail):
                return "Backend catalog could not be read (\(detail))"
            }
        }
    }

    static func load() throws -> ChipCatalog {
        guard let raw = catalog_json_copy() else {
            throw LoadError.unavailable
        }
        defer { free(raw) }

        let data = Data(String(cString: raw).utf8)
        do {
            return try JSONDecoder().decode(ChipCatalog.self, from: data)
        } catch {
            throw LoadError.malformed(error.localizedDescription)
        }
    }
}

// MARK: - Filtering

/// What the drawer is showing. `family` only narrows parts in the category
/// that owns families, so it is ignored for other categories.
/// The category whose parts have families. The backend names it; the app only
/// needs to know which one owns the family filter.
let familyCategory = "Logic"

/// The category generic LEDs are listed under in the drawer.
let ledCategory = "Indicators"

struct DrawerFilter: Equatable {
    var query = ""
    var category: String?
    var package: String?
    var family: String?
    /// Gates per package (the sidebar's count). Like `family`, only narrows logic parts.
    var gateCount: Int?
    /// Which flagged parts to list. Set from the user's preferences.
    var visibility = PartVisibility()
}

/// Which kinds of flagged parts the drawer lists. A part with several flags is
/// listed only when every one of its flags is allowed.
struct PartVisibility: Equatable {
    var showObsolete = false
    var showMilitary = false
    var showAutomotive = false

    func allows(_ part: ChipPart) -> Bool {
        (showObsolete || !part.obsolete)
            && (showMilitary || !part.military)
            && (showAutomotive || !part.automotive)
    }
}

// MARK: - Store

/// Holds the backend catalog as the drawer sees it. The backend is compiled
/// into the app, so the catalog is read once at launch and changes take effect
/// when the app is rebuilt.
@MainActor
@Observable
final class ChipCatalogStore {
    enum LoadState: Equatable {
        case notLoaded
        case loaded
        case failed(String)
    }

    struct PartEntry: Identifiable, Hashable {
        let part: ChipPart
        let chip: ChipDefinition

        var id: String { part.name }
    }

    struct FunctionGroup: Identifiable, Hashable {
        let function: String
        let parts: [PartEntry]

        var id: String { function }
    }

    struct CategoryGroup: Identifiable, Hashable {
        let category: String
        let functions: [FunctionGroup]

        var id: String { category }
    }

    /// Logic chips from the backend, followed by one entry per LED. LEDs are
    /// shaped like chips so the drawer lists them the same way.
    private(set) var chips: [ChipDefinition] = []
    private(set) var loadState: LoadState = .notLoaded

    private var entries: [PartEntry] = []
    private var chipByPartNumber: [String: ChipDefinition] = [:]
    private var ledPartNumbers: Set<String> = []
    private var oscillatorPartNumbers: Set<String> = []
    /// VCC and GND, kept out of the drawer. They are placed from the toolbar.
    private var supplyChips: [String: ChipDefinition] = [:]

    var partCount: Int { entries.count }

    var categories: [String] {
        Array(Set(chips.map(\.category))).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    var packages: [String] {
        Array(Set(entries.map(\.part.package))).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    /// Families present in one category.
    func families(in category: String) -> [String] {
        let names = entries
            .filter { $0.chip.category == category }
            .map(\.part.family)
        return Array(Set(names)).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    /// Gate counts present in one category, ascending.
    func gateCounts(in category: String) -> [Int] {
        let counts = entries
            .filter { $0.chip.category == category && $0.chip.gateCount > 0 }
            .map(\.chip.gateCount)
        return Array(Set(counts)).sorted()
    }

    func chip(forPartNumber partNumber: String) -> ChipDefinition? {
        chipByPartNumber[partNumber]
    }

    /// How a part is drawn on the canvas.
    func symbol(forPartNumber partNumber: String) -> PartSymbol {
        if let supply = supplyChips[partNumber] {
            return supply.pins.first?.role == .ground ? .ground : .vcc
        }
        if oscillatorPartNumbers.contains(partNumber) {
            return .oscillator
        }
        return ledPartNumbers.contains(partNumber) ? .diode : .block
    }

    /// A VCC or GND source by its part name, for placing on the canvas.
    func supplyChip(named name: String) -> ChipDefinition? {
        supplyChips[name]
    }

    /// Parts grouped by category, then by readable function (AND, NAND, ...),
    /// after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)

        let matches = entries.filter { entry in
            if !filter.visibility.allows(entry.part) {
                return false
            }
            if let category = filter.category, entry.chip.category != category {
                return false
            }
            if let package = filter.package, entry.part.package != package {
                return false
            }
            if let family = filter.family, entry.chip.category == familyCategory,
               entry.part.family != family {
                return false
            }
            if let count = filter.gateCount, entry.chip.category == familyCategory,
               entry.chip.gateCount != count {
                return false
            }
            return needle.isEmpty || [entry.part.name, entry.chip.function, entry.chip.description, entry.chip.category]
                .contains { $0.localizedStandardContains(needle) }
        }

        let byCategory = Dictionary(grouping: matches, by: { $0.chip.category })
        return byCategory
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { category, categoryEntries in
                let byFunction = Dictionary(grouping: categoryEntries, by: { $0.chip.function })
                let functions = byFunction
                    .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
                    .map { function, parts in
                        FunctionGroup(
                            function: function,
                            parts: parts.sorted {
                                $0.part.name.localizedStandardCompare($1.part.name) == .orderedAscending
                            }
                        )
                    }
                return CategoryGroup(category: category, functions: functions)
            }
    }

    func load() {
        do {
            apply(try BackendCatalog.load())
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }

    private func apply(_ catalog: ChipCatalog) {
        let ledChips = catalog.leds.map(Self.chip(forLED:))
        let oscillatorChips = Self.chips(forOscillators: catalog.oscillators)
        let newChips = catalog.chips + ledChips + oscillatorChips
        chips = newChips
        ledPartNumbers = Set(catalog.leds.map(\.name))
        oscillatorPartNumbers = Set(catalog.oscillators?.groups.flatMap(\.parts).map(\.name) ?? [])
        supplyChips = Dictionary(uniqueKeysWithValues: catalog.supplies.map { supply in
            (supply.name, Self.chip(forSupply: supply))
        })

        var newEntries: [PartEntry] = []
        var newIndex: [String: ChipDefinition] = [:]
        for chip in newChips {
            for part in chip.parts {
                newEntries.append(PartEntry(part: part, chip: chip))
                newIndex[part.name] = chip
            }
        }
        entries = newEntries
        chipByPartNumber = newIndex
    }

    /// Each oscillator as a one-part "chip". Its function is its frequency band,
    /// so the drawer lists oscillators under their bands.
    private static func chips(forOscillators section: OscillatorSection?) -> [ChipDefinition] {
        guard let section else { return [] }
        return section.groups.flatMap { group in
            group.parts.map { oscillator in
                ChipDefinition(
                    id: oscillator.name,
                    function: group.range,
                    description: "\(oscillator.family) oscillator, \(oscillator.package) package",
                    category: section.category,
                    gateCount: 0,
                    parts: [ChipPart(name: oscillator.name, package: oscillator.package, family: oscillator.family)],
                    pins: oscillator.pins
                )
            }
        }
    }

    /// A supply as a one-pin "chip". It is never listed in the drawer.
    private static func chip(forSupply supply: SupplyDefinition) -> ChipDefinition {
        ChipDefinition(
            id: supply.name,
            function: supply.name,
            description: "Constant source: \(supply.name)",
            category: "Power",
            gateCount: 0,
            parts: [],
            pins: supply.pins
        )
    }

    /// An LED as a one-part "chip": function "LED", the colour as its family
    /// label, and no gates.
    private static func chip(forLED led: LedDefinition) -> ChipDefinition {
        ChipDefinition(
            id: led.name,
            function: "LED",
            description: "\(led.color) LED, \(led.package) package",
            category: ledCategory,
            gateCount: 0,
            parts: [ChipPart(name: led.name, package: led.package, family: led.color)],
            pins: led.pins
        )
    }
}
