import Foundation
import Observation

// MARK: - The C library

/// Owns the backend's part library (`parts.h`) for the app's life. Circuits
/// borrow it, so it outlives every simulation.
nonisolated final class PartLibraryHandle: @unchecked Sendable {
    let pointer: OpaquePointer

    init() {
        pointer = part_library_create()
    }

    deinit {
        part_library_destroy(pointer)
    }
}

// MARK: - Parts as Swift values

/// How a pin is used (`PinRole` in parts.h).
nonisolated enum ChipPinRole: Hashable, Sendable {
    case input
    case output
    case inOut
    case power
    case ground
    case noConnect
    /// A numbered position with no lead. Never drawn; nothing can be wired to it.
    case absent
}

nonisolated struct ChipPin: Hashable, Sendable, Identifiable {
    /// The group a pin is in unless the part file names another.
    static let defaultGroup = "default"

    let number: Int
    let name: String
    let role: ChipPinRole
    /// Pins in one group are drawn together on the block, such as an SRAM's
    /// address pins. Most pins are in the default group.
    let group: String

    var id: Int { number }

    var isInDefaultGroup: Bool { group == Self.defaultGroup }

    /// Positions with no lead are left out of every symbol.
    var isHidden: Bool { role == .absent }
}

/// How a 7-segment display is drawn and which LED is which.
nonisolated struct DisplayInfo: Hashable, Sendable {
    /// One LED: a segment of a digit (1 = leftmost), or a colon or dot LED
    /// (digit 0; segment "L1", "L2" or "L3").
    struct LED: Hashable, Sendable {
        let digit: Int
        let segment: String
    }

    let digits: Int
    let commonAnode: Bool
    /// Digit the colon (L1, L2) and the raised dot (L3) sit after; 0 = none.
    let colonAfter: Int
    let dotAfter: Int
    let color: String
    /// Digit height, such as "0.56 in".
    let size: String
    /// The LED chip, such as "AlInGaP Super Red".
    let chip: String
    /// In the order the simulation reports their lights.
    let leds: [LED]
    /// Pin numbers for the left (segments) and right (commons) sides, top to bottom.
    let segmentPins: [Int]
    let commonPins: [Int]
}

/// One orderable part, copied from the backend's `Part` once at load.
nonisolated struct CatalogPart: Identifiable, Hashable, Sendable {
    struct Flags: Hashable, Sendable {
        var obsolete = false
        var military = false
        var automotive = false
        var enhanced = false
    }

    /// The facts that depend on the part's kind.
    enum Details: Hashable, Sendable {
        case logic(family: String, units: Int)
        case sram(words: Int, bits: Int, accessNs: Int, grade: String)
        case dipSwitch(positions: Int, actuator: String)
        case led(color: String)
        case display(DisplayInfo)
        case oscillator(family: String, minHz: UInt64, maxHz: UInt64, defaultHz: UInt64, enablePin: Int?)
        case supply(level: Int)
    }

    let name: String
    let kind: PartKind
    let package: String
    /// The parts-drawer leaf: "NAND", "32 KB", "LED", a frequency band.
    let function: String
    let description: String
    /// The parts-drawer path above the function, such as ["Memory", "SRAM"].
    /// Empty for parts that are never listed (VCC, GND).
    let drawer: [String]
    let flags: Flags
    let pins: [ChipPin]
    let details: Details

    var id: String { name }

    // Names are unique across the library, so a part is its name.
    static func == (a: CatalogPart, b: CatalogPart) -> Bool { a.name == b.name }
    func hash(into hasher: inout Hasher) { hasher.combine(name) }

    /// Gate, D or JK flip-flop: a logic chip with a family and a unit count.
    var isLogic: Bool { kind == PART_GATE || kind == PART_DFF || kind == PART_JKFF }

    var logicFamily: String? {
        if case let .logic(family, _) = details { return family }
        return nil
    }

    var display: DisplayInfo? {
        if case let .display(info) = details { return info }
        return nil
    }

    /// The color of an LED or a 7-segment display.
    var color: String? {
        switch details {
        case let .led(color): return color
        case let .display(info): return info.color
        default: return nil
        }
    }

    /// The count shown next to the name in the drawer: gates or flip-flops per
    /// package, switch positions, display digits. Nil for other parts.
    var count: Int? {
        switch details {
        case let .logic(_, units): return units
        case let .dipSwitch(positions, _): return positions
        case let .display(info): return info.digits
        default: return nil
        }
    }

    /// The drawer's second line after the package: family, speed grade,
    /// actuator, color or size.
    var detailLabels: [String] {
        switch details {
        case let .logic(family, _): return [family]
        case let .sram(_, _, accessNs, _): return ["\(accessNs) ns"]
        case let .dipSwitch(_, actuator): return [actuator]
        case let .led(color): return [color]
        case let .display(info): return [info.color, info.size, info.commonAnode ? "CA" : "CC"]
        case let .oscillator(family, _, _, _, _): return [family]
        case .supply: return []
        }
    }
}

// MARK: - Reading the C structs

nonisolated private func string(_ pointer: UnsafePointer<CChar>?) -> String {
    pointer.map { String(cString: $0) } ?? ""
}

nonisolated private func pinRole(_ role: PinRole) -> ChipPinRole {
    switch role {
    case PIN_INPUT: return .input
    case PIN_OUTPUT: return .output
    case PIN_INOUT: return .inOut
    case PIN_POWER: return .power
    case PIN_GROUND: return .ground
    case PIN_ABSENT: return .absent
    default: return .noConnect
    }
}

nonisolated private func numbers(_ pointer: UnsafePointer<UInt32>?, _ count: Int) -> [Int] {
    guard let pointer else { return [] }
    return (0..<count).map { Int(pointer[$0]) }
}

extension CatalogPart {
    /// Copies one part out of the library.
    nonisolated init(_ part: Part) {
        name = string(part.name)
        kind = part.kind
        package = string(part.package)
        function = string(part.function)
        description = string(part.description)
        drawer = (0..<part.drawer_count).map { string(part.drawer?[$0]) }
        flags = Flags(
            obsolete: part.flags & UInt32(PART_OBSOLETE) != 0,
            military: part.flags & UInt32(PART_MILITARY) != 0,
            automotive: part.flags & UInt32(PART_AUTOMOTIVE) != 0,
            enhanced: part.flags & UInt32(PART_ENHANCED) != 0
        )
        pins = (0..<part.pin_count).compactMap { index in
            guard let pin = part.pins?[index] else { return nil }
            return ChipPin(number: Int(pin.number), name: string(pin.name),
                           role: pinRole(pin.role), group: string(pin.group))
        }

        if let logic = part.logic?.pointee {
            details = .logic(family: string(logic.family), units: Int(logic.units))
        } else if let sram = part.sram?.pointee {
            details = .sram(words: Int(sram.words), bits: Int(sram.bits),
                            accessNs: Int(sram.access_ps / 1000), grade: string(sram.grade))
        } else if let dip = part.dip_switch?.pointee {
            details = .dipSwitch(positions: Int(dip.positions), actuator: string(dip.actuator))
        } else if let led = part.led?.pointee {
            details = .led(color: string(led.color))
        } else if let display = part.display?.pointee {
            let leds: [DisplayInfo.LED] = (0..<display.led_count).compactMap { index in
                guard let led = display.leds?[index] else { return nil }
                return DisplayInfo.LED(digit: Int(led.digit), segment: string(led.segment))
            }
            details = .display(DisplayInfo(
                digits: Int(display.digits),
                commonAnode: display.common_anode != 0,
                colonAfter: Int(display.colon_after),
                dotAfter: Int(display.dot_after),
                color: string(display.color),
                size: string(display.size),
                chip: string(display.chip),
                leds: leds,
                segmentPins: numbers(display.segment_pins, display.segment_pin_count),
                commonPins: numbers(display.common_pins, display.common_pin_count)
            ))
        } else if let osc = part.oscillator?.pointee {
            details = .oscillator(family: string(osc.family), minHz: osc.min_hz, maxHz: osc.max_hz,
                                  defaultHz: osc.default_hz,
                                  enablePin: osc.enable_pin == 0 ? nil : Int(osc.enable_pin))
        } else if let supply = part.supply?.pointee {
            details = .supply(level: Int(supply.level))
        } else {
            details = .supply(level: 0)
        }
    }
}

/// Loads every part file bundled with the app (`Parts/*.json`, copied flat
/// into the bundle's resources) into a library.
nonisolated enum BackendCatalog {
    enum LoadError: LocalizedError {
        case unavailable
        case malformed(String)

        var errorDescription: String? {
            switch self {
            case .unavailable:
                return "The app has no part files"
            case .malformed(let detail):
                return "A part file could not be read (\(detail))"
            }
        }
    }

    /// Every bundled JSON resource is a part file. Loaded in file-name order.
    static func load(into library: PartLibraryHandle) throws -> [CatalogPart] {
        let urls = (Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil) ?? [])
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
        guard !urls.isEmpty else { throw LoadError.unavailable }
        for url in urls {
            let text: String
            do {
                text = try String(contentsOf: url, encoding: .utf8)
            } catch {
                throw LoadError.malformed("\(url.lastPathComponent): \(error.localizedDescription)")
            }
            var error: UnsafeMutablePointer<CChar>?
            if load_parts(library.pointer, text, url.lastPathComponent, &error) != 0 {
                let message = error.map { String(cString: $0) } ?? url.lastPathComponent
                free(error)
                throw LoadError.malformed(message)
            }
        }
        let count = part_library_count(library.pointer)
        return (0..<count).compactMap { index in
            part_library_at(library.pointer, index).map { CatalogPart($0.pointee) }
        }
    }
}

// MARK: - Filtering

struct DrawerFilter: Equatable {
    var query = ""
    /// The drawer's top level, such as Logic or Memory.
    var category: String?
    var package: String?
    /// Logic family. Only narrows gates and flip-flops.
    var family: String?
    /// Gates or flip-flops per package. Only narrows gates and flip-flops.
    var gateCount: Int?
    /// Positions per switch. Only narrows switches.
    var switchPositions: Int?
    /// LED color ("Red", "Green", ...). Only narrows LEDs and 7-segment displays.
    var color: String?
    /// Digits and digit height. Only narrow 7-segment displays.
    var displayDigits: Int?
    var displaySize: String?
    /// Which flagged parts to list. Set from the user's preferences.
    var visibility = PartVisibility()
}

/// Which kinds of flagged parts the drawer lists. A part with several flags is
/// listed only when every one of its flags is allowed.
struct PartVisibility: Equatable {
    var showObsolete = false
    var showMilitary = false
    var showAutomotive = false

    func allows(_ part: CatalogPart) -> Bool {
        (showObsolete || !part.flags.obsolete)
            && (showMilitary || !part.flags.military)
            && (showAutomotive || !part.flags.automotive)
    }
}

/// How a part is drawn on the canvas.
func symbol(for part: CatalogPart) -> PartSymbol {
    switch part.kind {
    case PART_SWITCH: return .dipSwitch
    case PART_LED: return .diode
    case PART_7SEG: return .sevenSegment
    case PART_OSCILLATOR: return .oscillator
    case PART_SUPPLY:
        if case let .supply(level) = part.details, level == 0 { return .ground }
        return .vcc
    default: return .block
    }
}

// MARK: - Store

/// Holds the part library as the drawer sees it. The part files are bundled
/// with the app, so the library is read once at launch.
@MainActor
@Observable
final class PartCatalogStore {
    enum LoadState: Equatable {
        case notLoaded
        case loaded
        case failed(String)
    }

    struct FunctionGroup: Identifiable, Hashable {
        let function: String
        let parts: [CatalogPart]

        var id: String { function }
    }

    struct SubcategoryGroup: Identifiable, Hashable {
        let subcategory: String
        let functions: [FunctionGroup]
        /// Parts listed straight under the subcategory, with no function level:
        /// switches (the function is only the vendor's series), LEDs and
        /// 7-segment displays.
        var parts: [CatalogPart] = []

        var id: String { subcategory }
    }

    struct CategoryGroup: Identifiable, Hashable {
        let category: String
        /// Parts with a second drawer level, grouped under it (Memory → SRAM → ...).
        let subcategories: [SubcategoryGroup]
        /// Parts without one, listed directly under the category.
        let functions: [FunctionGroup]

        var id: String { category }
    }

    /// The C library; circuits are built from it.
    let library = PartLibraryHandle()
    private(set) var loadState: LoadState = .notLoaded

    /// Listed parts (a non-empty drawer path).
    private var entries: [CatalogPart] = []
    private var partsByName: [String: CatalogPart] = [:]

    var partCount: Int { entries.count }

    var categories: [String] {
        sortedNames(entries.compactMap(\.drawer.first))
    }

    var packages: [String] {
        sortedNames(entries.map(\.package))
    }

    /// The kinds of part listed under a top-level drawer category.
    func kinds(inCategory category: String) -> Set<UInt32> {
        Set(entries.filter { $0.drawer.first == category }.map(\.kind.rawValue))
    }

    /// The kinds of part listed under a second-level drawer group.
    func kinds(inSubcategory subcategory: String) -> Set<UInt32> {
        Set(entries.filter { $0.drawer.count > 1 && $0.drawer[1] == subcategory }.map(\.kind.rawValue))
    }

    /// Logic families of the gates and flip-flops in a category.
    func families(in category: String) -> [String] {
        sortedNames(entries.filter { $0.drawer.first == category }.compactMap(\.logicFamily))
    }

    /// Gates (or flip-flops) per package in a category, ascending.
    func logicCounts(in category: String) -> [Int] {
        Array(Set(entries.filter { $0.drawer.first == category && $0.isLogic }.compactMap(\.count))).sorted()
    }

    /// Positions per switch in a category, ascending.
    func switchPositionCounts(in category: String) -> [Int] {
        Array(Set(entries.filter { $0.drawer.first == category && $0.kind == PART_SWITCH }
            .compactMap(\.count))).sorted()
    }

    /// LED and display colors in a category.
    func colors(in category: String) -> [String] {
        sortedNames(entries.filter { $0.drawer.first == category }.compactMap(\.color))
    }

    /// Digit counts and digit heights of the 7-segment displays.
    var displayDigitCounts: [Int] {
        Array(Set(entries.compactMap(\.display).map(\.digits))).sorted()
    }

    var displaySizes: [String] {
        sortedNames(entries.compactMap(\.display).map(\.size))
    }

    /// Any part by name, listed or not (VCC and GND are not listed).
    func part(named name: String) -> CatalogPart? {
        partsByName[name]
    }

    /// Parts grouped by drawer path, then by function, after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)

        let matches = entries.filter { part in
            if !filter.visibility.allows(part) {
                return false
            }
            if let category = filter.category, part.drawer.first != category {
                return false
            }
            if let package = filter.package, part.package != package {
                return false
            }
            if part.isLogic {
                if let family = filter.family, part.logicFamily != family {
                    return false
                }
                if let count = filter.gateCount, part.count != count {
                    return false
                }
            }
            if part.kind == PART_SWITCH, let count = filter.switchPositions, part.count != count {
                return false
            }
            if let color = filter.color, let partColor = part.color, partColor != color {
                return false
            }
            if let display = part.display {
                if let count = filter.displayDigits, display.digits != count {
                    return false
                }
                if let size = filter.displaySize, display.size != size {
                    return false
                }
            }
            let fields = [part.name, part.function, part.description] + part.drawer
            return needle.isEmpty || fields.contains { $0.localizedStandardContains(needle) }
        }

        let byCategory = Dictionary(grouping: matches, by: { $0.drawer[0] })
        return byCategory
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { category, parts in
                let bySubcategory = Dictionary(
                    grouping: parts.filter { $0.drawer.count > 1 },
                    by: { $0.drawer[1] }
                )
                let subcategories = bySubcategory
                    .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
                    .map { subcategory, parts in
                        Self.listsPartsDirectly(parts)
                            ? SubcategoryGroup(subcategory: subcategory, functions: [], parts: Self.byCountThenName(parts))
                            : SubcategoryGroup(subcategory: subcategory, functions: Self.functionGroups(parts))
                    }
                return CategoryGroup(
                    category: category,
                    subcategories: subcategories,
                    functions: Self.functionGroups(parts.filter { $0.drawer.count == 1 })
                )
            }
    }

    /// Switches, LEDs and 7-segment displays are listed with no function level.
    private static func listsPartsDirectly(_ parts: [CatalogPart]) -> Bool {
        parts.allSatisfy { $0.kind == PART_SWITCH || $0.kind == PART_LED || $0.kind == PART_7SEG }
    }

    /// Parts by count (positions, digits), then by name.
    private static func byCountThenName(_ parts: [CatalogPart]) -> [CatalogPart] {
        parts.sorted {
            ($0.count ?? 0) != ($1.count ?? 0)
                ? ($0.count ?? 0) < ($1.count ?? 0)
                : $0.name.localizedStandardCompare($1.name) == .orderedAscending
        }
    }

    /// Parts grouped by function, functions and parts in natural order
    /// ("8 KB" before "32 KB").
    private static func functionGroups(_ parts: [CatalogPart]) -> [FunctionGroup] {
        Dictionary(grouping: parts, by: \.function)
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { function, parts in
                FunctionGroup(
                    function: function,
                    parts: parts.sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
                )
            }
    }

    private func sortedNames(_ names: [String]) -> [String] {
        Array(Set(names)).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    func load() {
        do {
            let parts = try BackendCatalog.load(into: library)
            partsByName = Dictionary(parts.map { ($0.name, $0) }, uniquingKeysWith: { first, _ in first })
            entries = parts.filter { !$0.drawer.isEmpty }
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }
}
