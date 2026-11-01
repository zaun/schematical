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
nonisolated struct ChipPart: Codable, Hashable, Sendable, Identifiable {
    let name: String
    let package: String
    let family: String

    var id: String { name }
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

nonisolated struct ChipCatalog: Codable, Hashable, Sendable {
    let chips: [ChipDefinition]
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

struct DrawerFilter: Equatable {
    var query = ""
    var category: String?
    var package: String?
    var family: String?
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

    private(set) var chips: [ChipDefinition] = []
    private(set) var loadState: LoadState = .notLoaded

    private var entries: [PartEntry] = []
    private var chipByPartNumber: [String: ChipDefinition] = [:]

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

    func chip(forPartNumber partNumber: String) -> ChipDefinition? {
        chipByPartNumber[partNumber]
    }

    /// Parts grouped by category, then by readable function (AND, NAND, ...),
    /// after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)

        let matches = entries.filter { entry in
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
            apply(try BackendCatalog.load().chips)
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }

    private func apply(_ newChips: [ChipDefinition]) {
        chips = newChips

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
}
