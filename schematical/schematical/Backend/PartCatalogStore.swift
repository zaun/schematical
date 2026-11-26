import Foundation
import Observation

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
        /// The group's name: the parts' function, or a tactile switch size
        /// ("6 × 6 mm"; `CatalogPart.drawerLeaf`).
        let function: String
        let parts: [CatalogPart]
        /// For ADCs and DACs, `parts` split by resolution, fewest bits first;
        /// empty for other parts.
        var bitGroups: [BitGroup] = []

        var id: String { function }
    }

    /// A converter function group's parts with one resolution.
    struct BitGroup: Identifiable, Hashable {
        let bits: Int
        let parts: [CatalogPart]

        var id: Int { bits }
        var title: String { "\(bits)-bit" }
    }

    /// A drawer group below a category: its second level (Memory → SRAM)
    /// or third (Discrete → Resistors → Chip).
    struct SubcategoryGroup: Identifiable, Hashable {
        let subcategory: String
        /// The drawer path down to and including this group, e.g.
        /// ["Discrete", "Resistors"].
        let path: [String]
        /// Parts with a drawer level below this one, grouped under it
        /// (Discrete → Resistors → Chip). Empty at the third level.
        var subgroups: [SubcategoryGroup] = []
        let functions: [FunctionGroup]
        /// Parts listed straight under the subcategory, with no function level:
        /// switches (the function is only the vendor's series), LEDs,
        /// 7-segment displays and character LCDs.
        var parts: [CatalogPart] = []

        var id: String { path.joined(separator: "/") }

        /// Every part in the group and the groups below it.
        var allParts: [CatalogPart] {
            parts + functions.flatMap(\.parts) + subgroups.flatMap(\.allParts)
        }
    }

    struct CategoryGroup: Identifiable, Hashable {
        let category: String
        /// Parts with a second drawer level, grouped under it (Memory → SRAM → ...).
        let subcategories: [SubcategoryGroup]
        /// Parts without one, listed directly under the category.
        let functions: [FunctionGroup]

        var id: String { category }

        /// Every part in the category.
        var allParts: [CatalogPart] {
            functions.flatMap(\.parts) + subcategories.flatMap(\.allParts)
        }
    }

    /// The app's one catalog, shared by every window. The part files are
    /// read the first time it's used, at launch.
    static let shared: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        // The user's parts (plan-53-B); only this catalog reads the folder.
        store.attachUserLibrary(UserPartLibrary(directory: UserPartLibrary.defaultDirectory))
        return store
    }()

    /// The C library; circuits are built from it.
    let library = PartLibraryHandle()
    private(set) var loadState: LoadState = .notLoaded

    /// Listed parts (a non-empty drawer path).
    private var entries: [CatalogPart] = []
    private var partsByName: [String: CatalogPart] = [:]
    /// The sockets by the id of each chip footprint they hold (plan-51-H),
    /// built the first time `sockets(fitting:)` asks and dropped whenever
    /// `partsByName` changes.
    @ObservationIgnored private var socketIndex: [String: [CatalogPart]]?
    /// Registered custom parts (plan-32-A), by uuid, and the text each was
    /// registered with.
    private var partsByUUID: [String: CatalogPart] = [:]
    private var registeredTexts: [String: String] = [:]
    /// Where each registered custom part is listed (plan-53-B), by uuid.
    private var listings: [String: PartListing] = [:]
    /// A part file's text that clashed with another part's uuid, and the new
    /// uuid it was registered under (plan-53-A): the same text again is that
    /// part.
    private var freshenedTexts: [String: String] = [:]
    /// The User Library's folder, once attached (`attachUserLibrary`); nil for
    /// a catalog made without one (tests).
    private(set) var userLibrary: UserPartLibrary?
    /// Library files that could not be loaded, for the library window.
    private(set) var userLibraryProblems: [UserPartLibraryProblem] = []
    /// When each User Library part was added (its file's creation date).
    private var userLibraryAdded: [String: Date] = [:]

    /// What the drawer's headers and menus show, built from `entries` in one
    /// pass the first time it is asked for (plan-43 follow-up: each header
    /// used to scan every part on every redraw). `entries` changes only in
    /// `load` and `register`, which drop it.
    @ObservationIgnored private var derivedCache: Derived? {
        didSet { if derivedCache == nil { derivedRevision += 1 } }
    }

    /// Counts the times `entries` changed in a way the drawer must rebuild for,
    /// including a part moving between categories or being replaced, which
    /// leave `partCount` as it was (plan-53: the drawer kept listing an added
    /// part under File Library).
    private(set) var derivedRevision = 0

    private var derived: Derived {
        if let derivedCache { return derivedCache }
        let built = Derived(entries)
        derivedCache = built
        return built
    }

    /// Facts about the listed parts, by drawer group, for the drawer's
    /// header menus, plus what the search and the sort need.
    ///
    /// `nonisolated`: it is plain data, and a main-actor class has an isolated
    /// deinit, which aborts the test host when `register` drops the cache
    /// inside a test ("pointer being freed was not allocated" in
    /// `swift_task_deinitOnExecutor`, macOS 15.8; `TestSupport.newDocument()`
    /// notes the same for documents).
    nonisolated private final class Derived {
        var kindsByGroup: [String: Set<UInt32>] = [:]
        var tactileGroups: Set<String> = []
        var familiesByCategory: [String: Set<String>] = [:]
        var logicCountsByCategory: [String: Set<Int>] = [:]
        var switchPositionsByCategory: [String: Set<Int>] = [:]
        var colorsByCategory: [String: Set<String>] = [:]
        var converterChannelsByType: [String: Set<Int>] = [:]
        var displayDigits: Set<Int> = []
        var displaySizes: Set<String> = []
        var tactileActuators: Set<String> = []
        var packages: Set<String> = []
        /// Every part's searchable text (name, function, group, description
        /// and drawer path), folded for case and accents, as UTF-8, each
        /// ending in a zero byte; `searchStarts[i]` is where entry `i` begins.
        let searchBytes: [UInt8]
        let searchStarts: [Int]
        /// Each part name's place in natural order ("8 KB" before "32 KB").
        let nameRank: [String: Int]
        /// Which entries' text holds `needle` (already folded, not empty).
        func matches(_ needle: String) -> [Bool] {
            var hit = [Bool](repeating: false, count: searchStarts.count)
            let wanted = Array(needle.utf8)
            searchBytes.withUnsafeBufferPointer { text in
                wanted.withUnsafeBufferPointer { pattern in
                    guard let base = text.baseAddress, let find = pattern.baseAddress else { return }
                    var offset = 0
                    while offset < text.count,
                          let found = memmem(base + offset, text.count - offset, find, pattern.count) {
                        let position = base.distance(to: found.assumingMemoryBound(to: UInt8.self))
                        // The entry holding this byte: the last start at or before it.
                        var low = 0, high = searchStarts.count - 1
                        while low < high {
                            let mid = (low + high + 1) / 2
                            if searchStarts[mid] <= position { low = mid } else { high = mid - 1 }
                        }
                        hit[low] = true
                        // The next entry's text may hold it too.
                        offset = low + 1 < searchStarts.count ? searchStarts[low + 1] : text.count
                    }
                }
            }
            return hit
        }

        /// Indices into `entries`, in natural name order: filtering in this
        /// order leaves every group sorted with no sort of its own.
        let nameOrder: [Int]
        /// Each part's flags in `entries` order: 1 obsolete, 2 military, 4 automotive.
        let flagBits: [UInt8]

        init(_ entries: [CatalogPart]) {
            struct Path { var drawer: [String]; var kinds: Set<UInt32> = []; var tactile = false }
            var paths: [String: Path] = [:]
            for part in entries {
                let category = part.drawer.first ?? ""
                let key = part.drawer.joined(separator: "/")
                paths[key, default: Path(drawer: part.drawer)].kinds.insert(part.kind.rawValue)
                if part.isTactileSwitch { paths[key]?.tactile = true }
                if part.isLogic {
                    if let family = part.logicFamily { familiesByCategory[category, default: []].insert(family) }
                    if let count = part.count { logicCountsByCategory[category, default: []].insert(count) }
                }
                if part.kind == PART_SWITCH, !part.isTactileSwitch, !part.isSlideSwitch, let count = part.count {
                    switchPositionsByCategory[category, default: []].insert(count)
                }
                if !part.colors.isEmpty { colorsByCategory[category, default: []].formUnion(part.colors) }
                if case let .converter(type, _, channels) = part.details {
                    converterChannelsByType[type, default: []].insert(channels)
                }
                if let display = part.display {
                    displayDigits.insert(display.digits)
                    displaySizes.insert(display.size)
                }
                if let actuator = part.tactileActuator { tactileActuators.insert(actuator) }
                packages.formUnion(part.allPackages)
            }
            // A group lists the kinds of every path at or below it.
            for path in paths.values {
                for level in 1...max(path.drawer.count, 1) where !path.drawer.isEmpty {
                    let key = path.drawer.prefix(level).joined(separator: "/")
                    kindsByGroup[key, default: []].formUnion(path.kinds)
                    if path.tactile { tactileGroups.insert(key) }
                }
            }
            let options: String.CompareOptions = [.caseInsensitive, .diacriticInsensitive, .widthInsensitive]
            var bytes: [UInt8] = []
            var starts: [Int] = []
            starts.reserveCapacity(entries.count)
            for part in entries {
                starts.append(bytes.count)
                bytes.append(contentsOf: ([part.name, part.function, part.drawerLeaf, part.description] + part.drawer)
                    .joined(separator: "\n")
                    .folding(options: options, locale: nil).utf8)
                bytes.append(0)
            }
            searchBytes = bytes
            searchStarts = starts
            let order = entries.indices.sorted {
                let c = entries[$0].name.localizedStandardCompare(entries[$1].name)
                return c == .orderedSame ? $0 < $1 : c == .orderedAscending
            }
            nameOrder = order
            flagBits = entries.map { part in
                (part.flags.obsolete ? 1 : 0) | (part.flags.military ? 2 : 0) | (part.flags.automotive ? 4 : 0)
            }
            var rank: [String: Int] = [:]
            rank.reserveCapacity(order.count)
            for (place, index) in order.enumerated() where rank[entries[index].name] == nil {
                rank[entries[index].name] = place
            }
            nameRank = rank
        }
    }

    var partCount: Int { entries.count }

    /// Every listed part (automation: search).
    var allParts: [CatalogPart] { entries }

    var categories: [String] {
        Array(Set(derived.kindsByGroup.keys.filter { !$0.contains("/") })).sorted(by: PartCategory.precedes)
    }

    var packages: [String] {
        sortedNames(Array(derived.packages))
    }

    /// The kinds of part listed under a top-level drawer category.
    func kinds(inCategory category: String) -> Set<UInt32> {
        derived.kindsByGroup[category] ?? []
    }

    /// The kinds of part listed under a drawer group: `path` is its drawer
    /// path, e.g. ["Indicators", "LED 7-Seg"].
    func kinds(inGroup path: [String]) -> Set<UInt32> {
        derived.kindsByGroup[path.joined(separator: "/")] ?? []
    }

    /// Logic families of the gates and flip-flops in a category.
    func families(in category: String) -> [String] {
        sortedNames(Array(derived.familiesByCategory[category] ?? []))
    }

    /// Gates (or flip-flops) per package in a category, ascending.
    func logicCounts(in category: String) -> [Int] {
        (derived.logicCountsByCategory[category] ?? []).sorted()
    }

    /// Positions per switch in a category, ascending.
    func switchPositionCounts(in category: String) -> [Int] {
        (derived.switchPositionsByCategory[category] ?? []).sorted()
    }

    /// The converter type ("adc", "dac") of a function group's parts, or nil
    /// when they are not converters.
    func converterType(of parts: [CatalogPart]) -> String? {
        guard case let .converter(type, _, _)? = parts.first?.details else { return nil }
        return type
    }

    /// Channel counts of one converter type, ascending.
    func converterChannels(type: String) -> [Int] {
        (derived.converterChannelsByType[type] ?? []).sorted()
    }

    /// LED and display colors in a category.
    func colors(in category: String) -> [String] {
        sortedNames(Array(derived.colorsByCategory[category] ?? []))
    }

    /// Digit counts and digit heights of the 7-segment displays.
    var displayDigitCounts: [Int] {
        derived.displayDigits.sorted()
    }

    var displaySizes: [String] {
        sortedNames(Array(derived.displaySizes))
    }

    /// Whether a drawer group (`path`, as for `kinds(inGroup:)`) lists
    /// tactile switches.
    func isTactile(group path: [String]) -> Bool {
        derived.tactileGroups.contains(path.joined(separator: "/"))
    }

    /// The tactile switches' actuators ("Round Button", "Standard", ...).
    var tactileActuators: [String] {
        sortedNames(Array(derived.tactileActuators))
    }

    /// The VCC tool's rails (`SupplyChoice`) that at least one listed part
    /// runs at, in menu order, with their voltage in millivolts.
    var voltageChoices: [(name: String, mv: UInt32)] {
        SupplyChoice.names.compactMap { name in
            guard case let .supply(_, mv, _)? = partsByName[name]?.details, mv > 0,
                  entries.contains(where: { $0.vcc.map { $0.minMV...$0.maxMV ~= mv } ?? false })
            else { return nil }
            return (name, mv)
        }
    }

    /// Any part by name, listed or not (VCC and GND are not listed). A rail
    /// of any voltage ("170V", "3V6") and its pull-up ("PULLUP-170V") exist
    /// as soon as they are asked for: see `customSupply`.
    /// The sockets that hold a chip footprint (plan-51-H): the `PART_SOCKET`
    /// parts whose `fits` name it, the generic one first, then by
    /// manufacturer, then by part number in natural order. Bundled sockets
    /// only (see the index).
    func sockets(fitting footprint: Footprint) -> [CatalogPart] {
        if socketIndex == nil {
            var index: [String: [CatalogPart]] = [:]
            // Bundled sockets only: a chip's file names its socket, and a file carries no
            // custom part but a placed chip's own (`customParts`).
            for part in partsByName.values where part.kind == PART_SOCKET && part.uuid == nil {
                for fit in part.fits { index[fit.id, default: []].append(part) }
            }
            socketIndex = index.mapValues { list in
                list.sorted { a, b in
                    let (ga, gb) = (a.manufacturer == "Generic", b.manufacturer == "Generic")
                    if ga != gb { return ga }
                    if a.manufacturer != b.manufacturer {
                        return a.manufacturer.localizedStandardCompare(b.manufacturer) == .orderedAscending
                    }
                    return a.name.localizedStandardCompare(b.name) == .orderedAscending
                }
            }
        }
        return socketIndex?[footprint.id] ?? []
    }

    /// The generic socket for a chip footprint: the first `Generic` entry of
    /// `sockets(fitting:)`; nil when there is none.
    func genericSocket(fitting footprint: Footprint) -> CatalogPart? {
        sockets(fitting: footprint).first { $0.manufacturer == "Generic" }
    }

    func part(named name: String) -> CatalogPart? {
        partsByName[name] ?? customSupplies[name] ?? customSupply(named: name)
    }

    /// The rails made on demand (the VCC tool's Custom… voltage, and a file
    /// that uses one), by name. Not observed: a lookup may add to it while a
    /// view draws, and no view lists these parts.
    @ObservationIgnored private var customSupplies: [String: CatalogPart] = [:]

    /// Loads the supply and the pull-up of the voltage `name` says, when it
    /// is a rail name the library does not have, and returns the one asked
    /// for. Both go into the C library, so circuits find them too.
    private func customSupply(named name: String) -> CatalogPart? {
        guard loadState == .loaded else { return nil }
        let pullPrefix = "PULLUP-"
        let rail = name.hasPrefix(pullPrefix) ? String(name.dropFirst(pullPrefix.count)) : name
        guard let mv = SupplyChoice.millivolts(name: rail) else { return nil }
        // A bundled rail's pull-up is bundled too; never load a name twice.
        let wanted = [rail, pullPrefix + rail].filter { partsByName[$0] == nil && customSupplies[$0] == nil }
        guard wanted.contains(name) else { return nil }
        let volts = String(format: "%g", Double(mv) / 1000)
        let devices = wanted.map { part -> String in
            part == rail
                ? """
                {"id":"\(part)","function":"\(part)","description":"Supply, \(volts) V, drives logic 1","drawer":[],
                 "pinouts":{"pin":[{"number":1,"name":"VCC","role":"power"}]},"volts":\(volts),
                 "parts":[{"name":"\(part)","manufacturer":"Generic","package":"","pinout":"pin"}]}
                """
                : """
                {"id":"\(part)","function":"Pull-up","description":"Pull-up to \(volts) V, drives logic 1 weakly","drawer":[],
                 "pinouts":{"pin":[{"number":1,"name":"PU","role":"output"}]},"volts":\(volts),"pull":true,
                 "parts":[{"name":"\(part)","manufacturer":"Generic","package":"Radial","pinout":"pin",
                  "footprint":"RES-RADIAL-2.54","footprintChoices":[
                   {"package":"0402","footprint":"RES-0402"},{"package":"0603","footprint":"RES-0603"},
                   {"package":"0805","footprint":"RES-0805"},{"package":"1206","footprint":"RES-1206"},
                   {"package":"Axial","footprint":"RES-AXIAL-10.16"},{"package":"Radial","footprint":"RES-RADIAL-2.54"}]}]}
                """
        }
        let text = """
            {"format":"schematical-parts","version":1,"kind":"supply","devices":[\(devices.joined(separator: ","))]}
            """
        var error: UnsafeMutablePointer<CChar>?
        // The text is made here from a rail name `SupplyChoice` parsed: the loader takes it.
        let loaded = load_parts(library.pointer, text, "custom-supply-\(rail).json", &error) == 0
        free(error)
        guard loaded else { return nil }
        for part in wanted {
            guard let loaded = part_library_find(library.pointer, part) else { continue }
            customSupplies[part] = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
        }
        return customSupplies[name]
    }

    /// A registered custom part by its uuid.
    func part(uuid: String) -> CatalogPart? {
        partsByUUID[uuid]
    }

    /// The text a custom part was registered with (sorted keys, its name as
    /// registered, suffix included); nil for an unknown uuid.
    func registeredText(uuid: String) -> String? {
        registeredTexts[uuid]
    }

    /// Whether the file's uuid is registered with other content (the
    /// comparison `register` makes, with the name as registered).
    func uuidConflicts(_ fileText: String) -> Bool {
        guard var file = try? PartFileObject(text: fileText), let uuid = file.uuid,
              let registered = partsByUUID[uuid] else { return false }
        file.name = registered.name
        return file.canonicalText != registeredTexts[uuid]
    }

    /// Adds a single part file's part to the catalog for the rest of the
    /// session (plan-32-A § 4.3). The same uuid with the same content returns
    /// the registered part; with other content the file is registered under a
    /// new uuid (plan-53-A: `PartFileObject.freshenIdentity`), so no two
    /// different parts share one. A name that is taken gets the lowest free
    /// suffix `.2`, `.3`, …. The backend checks the file; a failure changes
    /// nothing. The part is listed as `listing` says (the File Library by
    /// default, plan-53-B); a part that is already registered and listed
    /// `removed` is listed again (as `listing`, never `removed`), and one
    /// listed `file` is moved to the User Library by `listing: .user`.
    @discardableResult
    func register(fileText: String, source: String, listing: PartListing = .file) throws -> CatalogPart {
        var file = try PartFileObject(text: fileText)
        var clashingText: String?
        if let uuid = file.uuid, let registered = partsByUUID[uuid] {
            // The registered text carries the name as registered (maybe
            // suffixed); compare with the new file under that name.
            var compared = file
            compared.name = registered.name
            if compared.canonicalText == registeredTexts[uuid] {
                return relisted(uuid, as: listing) ?? registered
            }
            // The same clashing text again (a schematic opened twice) is the
            // part it became the first time, not another copy.
            if let fresh = freshenedTexts[file.canonicalText], partsByUUID[fresh] != nil {
                return relisted(fresh, as: listing) ?? registered
            }
            clashingText = file.canonicalText
            file.freshenIdentity()
        }
        if let name = file.name, partsByName[name] != nil {
            var n = 2
            while partsByName["\(name).\(n)"] != nil {
                n += 1
            }
            file.name = "\(name).\(n)"
        }
        let text = file.canonicalText
        var error: UnsafeMutablePointer<CChar>?
        if load_part_file(library.pointer, text, source, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? source
            free(error)
            throw PartRegistrationError.backend(message)
        }
        // The loader refuses a part file without a uuid, so a loaded one is found by it.
        guard let uuid = file.uuid, let loaded = part_library_find_uuid(library.pointer, uuid) else { throw PartRegistrationError.backend("\(source): the part did not load") }
        let backendPart = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
        let part = backendPart.listed(in: listing.category ?? PartCategory.fileLibrary)
        partsByName[part.name] = part
        socketIndex = nil
        partsByUUID[uuid] = part
        registeredTexts[uuid] = text
        listings[uuid] = listing
        if let clashingText { freshenedTexts[clashingText] = uuid }
        if !part.drawer.isEmpty {
            entries.append(part)
            derivedCache = nil
        }
        return part
    }

    /// A part that is already registered, found again by `register`: listed
    /// again if it had been removed, and moved to the User Library when that
    /// is what is asked for. Returns the registered part.
    private func relisted(_ uuid: String, as listing: PartListing) -> CatalogPart? {
        let current = listings[uuid]
        let wanted: PartListing = listing == .removed ? .file : listing
        if current == .removed || (wanted == .user && current == .file) {
            setListing(wanted, for: uuid)
        }
        return partsByUUID[uuid]
    }

    /// A part of the User Library with its file, for the library window.
    struct UserLibraryItem: Identifiable {
        let part: CatalogPart
        let added: Date
        let url: URL

        var id: String { part.uuid ?? part.name }
    }

    /// The User Library's parts, in natural name order.
    var userLibraryItems: [UserLibraryItem] {
        guard let userLibrary else { return [] }
        return listings.compactMap { uuid, listing -> UserLibraryItem? in
            guard listing == .user, let part = partsByUUID[uuid] else { return nil }
            return UserLibraryItem(part: part, added: userLibraryAdded[uuid] ?? Date(),
                                   url: userLibrary.url(forUUID: uuid))
        }
        .sorted { $0.part.name.localizedStandardCompare($1.part.name) == .orderedAscending }
    }

    /// Attaches the User Library's folder and registers every part in it,
    /// listed `user` (plan-53-B). A file the loader refuses, or whose uuid an
    /// earlier library file or an already registered part holds with other
    /// content, is recorded in `userLibraryProblems` and skipped.
    func attachUserLibrary(_ library: UserPartLibrary) {
        userLibrary = library
        let found = library.files()
        var problems = found.problems
        var claimed: [String: String] = [:]
        for file in found.files {
            if let earlier = claimed[file.uuid] {
                problems.append(UserPartLibraryProblem(
                    url: file.url, reason: "It has the same identity as \(earlier), which is already loaded."))
                continue
            }
            claimed[file.uuid] = file.url.lastPathComponent
            if partsByUUID[file.uuid] != nil, uuidConflicts(file.text) {
                problems.append(UserPartLibraryProblem(
                    url: file.url, reason: "Another part with this identity is already loaded."))
                continue
            }
            do {
                try register(fileText: file.text, source: file.url.lastPathComponent, listing: .user)
                userLibraryAdded[file.uuid] = file.added
            } catch {
                problems.append(UserPartLibraryProblem(url: file.url, reason: error.localizedDescription))
            }
        }
        userLibraryProblems = problems
    }

    /// Registers a part file and keeps it in the User Library: listed under
    /// User Library and written to the library folder (plan-53-B). A
    /// failed write leaves the part listed in the File Library and throws
    /// `libraryWrite`.
    @discardableResult
    func addToUserLibrary(fileText: String, source: String) throws -> CatalogPart {
        guard userLibrary != nil else { throw PartRegistrationError.noUserLibrary }
        let before = (try? PartFileObject(text: fileText))?.uuid.flatMap { listings[$0] }
        let part = try register(fileText: fileText, source: source, listing: .user)
        try persistToUserLibrary(part, previous: before ?? .file)
        return part
    }

    /// Keeps a part that is already registered (a File Library part) in the
    /// User Library.
    func addToUserLibrary(_ part: CatalogPart) throws {
        guard userLibrary != nil else { throw PartRegistrationError.noUserLibrary }
        guard let uuid = part.uuid, partsByUUID[uuid] != nil else {
            throw PartRegistrationError.notAPartFile("\(part.name) is not a custom part")
        }
        let previous = listings[uuid] ?? .file
        setListing(.user, for: uuid)
        try persistToUserLibrary(partsByUUID[uuid] ?? part, previous: previous)
    }

    private func persistToUserLibrary(_ part: CatalogPart, previous: PartListing) throws {
        // Both callers check the library and register the part (its uuid and text) first.
        guard let userLibrary, let uuid = part.uuid, let text = registeredTexts[uuid] else { throw PartRegistrationError.noUserLibrary }
        do {
            try userLibrary.write(uuid: uuid, text: text)
            userLibraryAdded[uuid] = userLibraryAdded[uuid] ?? Date()
        } catch {
            if previous != .user { setListing(previous == .removed ? .file : previous, for: uuid) }
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
    }

    /// Called after a User Library part is replaced in place (plan-53-D), with
    /// its uuid and the new part, so open schematics can follow. Set by
    /// `SchematicEditorModel` (to `SchematicNSDocument.updateOpenDocuments`).
    @ObservationIgnored var partUpdated: ((String, CatalogPart) -> Void)?

    /// Replaces a User Library part's content under the same uuid, now
    /// (plan-53-D): the library file, the backend's part (retired and loaded
    /// again) and the catalog's records. The kind and the uuid cannot change;
    /// a new name that another part has gets the usual suffix. On any failure
    /// the old version is still the part, and the old text is the file's.
    @discardableResult
    func updateUserLibraryPart(uuid: String, fileText: String) throws -> CatalogPart {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        guard let old = partsByUUID[uuid], listings[uuid] == .user, let oldText = registeredTexts[uuid] else {
            throw PartRegistrationError.notInUserLibrary(name: partsByUUID[uuid]?.name ?? uuid)
        }
        var file = try PartFileObject(text: fileText)
        guard file.uuid == uuid else {
            throw PartRegistrationError.identityChanged(detail: "An update cannot change the part's identity.")
        }
        let oldFile = try PartFileObject(text: oldText)
        guard file.object["kind"] as? String == oldFile.object["kind"] as? String else {
            throw PartRegistrationError.identityChanged(detail: "An update cannot change the part's kind.")
        }
        // A name another part has is suffixed; the part's own old name is free.
        if let name = file.name, let other = partsByName[name], other.uuid != uuid {
            var n = 2
            while let taken = partsByName["\(name).\(n)"], taken.uuid != uuid { n += 1 }
            file.name = "\(name).\(n)"
        }
        let text = file.canonicalText
        if text == oldText { return old }

        func ids(_ key: String, of file: PartFileObject) -> [String] {
            (file.object[key] as? [[String: Any]] ?? []).compactMap { $0["id"] as? String }
        }
        do {
            try userLibrary.write(uuid: uuid, text: text)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        let source = "\(file.name ?? uuid).schpart"
        // The part is registered (checked above), so the backend has it to retire.
        let retired = library.retire(uuid: uuid, footprintIDs: ids("footprints", of: oldFile),
                                     datasheetIDs: ids("datasheets", of: oldFile))
        if !retired { try? userLibrary.write(uuid: uuid, text: oldText) }
        guard retired else { throw PartRegistrationError.backend("\(source): the part could not be replaced") }
        var error: UnsafeMutablePointer<CChar>?
        if load_part_file(library.pointer, text, source, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? source
            free(error)
            // Put the old version back.
            var restoreError: UnsafeMutablePointer<CChar>?
            _ = load_part_file(library.pointer, oldText, "\(old.name).schpart", &restoreError)
            free(restoreError)
            try? userLibrary.write(uuid: uuid, text: oldText)
            throw PartRegistrationError.backend(message)
        }
        guard let loaded = part_library_find_uuid(library.pointer, uuid) else { throw PartRegistrationError.backend("\(source): the part did not load") }
        let part = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
            .listed(in: PartCategory.userLibrary)
        if old.name != part.name { partsByName[old.name] = nil }
        partsByName[part.name] = part
        socketIndex = nil
        partsByUUID[uuid] = part
        registeredTexts[uuid] = text
        if let index = entries.firstIndex(where: { $0.uuid == uuid }) {
            entries[index] = part
        } else if !part.drawer.isEmpty {
            entries.append(part)
        }
        derivedCache = nil
        partUpdated?(uuid, part)
        return part
    }

    /// Removes a part from the User Library: deletes its file and takes it out
    /// of the drawer. It stays registered for the rest of the session, so
    /// documents that hold it still resolve it; the backend cannot unload it.
    func removeFromUserLibrary(uuid: String) throws {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        do {
            try userLibrary.remove(uuid: uuid)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        userLibraryAdded[uuid] = nil
        if listings[uuid] != nil { setListing(.removed, for: uuid) }
    }

    /// Deletes a library file that could not be loaded and drops it from
    /// `userLibraryProblems` (plan-53-F).
    func removeUserLibraryProblem(_ problem: UserPartLibraryProblem) throws {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        do {
            try userLibrary.removeFile(at: problem.url)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        userLibraryProblems.removeAll { $0 == problem }
    }

    /// Where a registered custom part is listed; nil for a bundled part.
    func listing(of part: CatalogPart) -> PartListing? {
        part.uuid.flatMap { listings[$0] }
    }

    /// Changes where a registered custom part is listed, in `entries` and in
    /// both lookups together (plan-53-B). `removed` takes it out of the drawer
    /// only: it stays registered, so documents that hold it still resolve it.
    func setListing(_ listing: PartListing, for uuid: String) {
        guard let current = partsByUUID[uuid], listings[uuid] != nil else { return }
        listings[uuid] = listing
        if let category = listing.category {
            let updated = current.listed(in: category)
            partsByName[updated.name] = updated
            socketIndex = nil
            partsByUUID[uuid] = updated
            if let index = entries.firstIndex(where: { $0.uuid == uuid }) {
                entries[index] = updated
            } else if !updated.drawer.isEmpty {
                entries.append(updated)
            }
        } else {
            entries.removeAll { $0.uuid == uuid }
        }
        derivedCache = nil
    }

    /// Whether `part` passes the menu filters of `filter` (not the query or
    /// the flags, which `groups` checks first).
    private func passes(_ part: CatalogPart, filter: DrawerFilter) -> Bool {
        if let category = filter.category, part.drawer.first != category {
            return false
        }
        if let package = filter.package, !part.allPackages.contains(package) {
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
        if part.kind == PART_SWITCH, !part.isTactileSwitch, !part.isSlideSwitch, let count = filter.switchPositions, part.count != count {
            return false
        }
        if case let .converter(type, _, channels) = part.details,
           let wanted = filter.converterChannels[type], channels != wanted {
            return false
        }
        if let mv = filter.voltageMV {
            if let vcc = part.vcc, !(vcc.minMV...vcc.maxMV ~= mv) {
                return false
            }
            if case let .dipSwitch(_, _, ratedMV) = part.details, ratedMV > 0, ratedMV < mv {
                return false
            }
            if case let .tactileSwitch(_, ratedMV, _, _, _) = part.details, ratedMV > 0, ratedMV < mv {
                return false
            }
            if case let .slideSwitch(_, _, _, _, _, ratedMV) = part.details, ratedMV > 0, ratedMV < mv {
                return false
            }
        }
        if let color = filter.color, !part.colors.isEmpty, !part.colors.contains(color) {
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
        if let wanted = filter.tactileActuator, let actuator = part.tactileActuator, actuator != wanted {
            return false
        }
        return true
    }

    /// Parts grouped by drawer path, then by function, after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)
            .folding(options: [.caseInsensitive, .diacriticInsensitive, .widthInsensitive], locale: nil)
        let derived = self.derived
        let rank = derived.nameRank

        // The flags and the query are checked on flat arrays; the part itself
        // is read only when a menu filter is set.
        let hidden: UInt8 = (filter.visibility.showObsolete ? 0 : 1) | (filter.visibility.showMilitary ? 0 : 2)
            | (filter.visibility.showAutomotive ? 0 : 4)
        let menuFiltered = filter.hasMenuFilter
        let queryHits = needle.isEmpty ? [] : derived.matches(needle)
        var matches: [CatalogPart] = []
        for index in derived.nameOrder where derived.flagBits[index] & hidden == 0 {
            if !needle.isEmpty, !queryHits[index] { continue }
            if menuFiltered, !passes(entries[index], filter: filter) { continue }
            matches.append(entries[index])
        }

        let byCategory = Dictionary(grouping: matches, by: { $0.drawer[0] })
        return byCategory
            .sorted { PartCategory.precedes($0.key, $1.key) }
            .map { category, parts in
                CategoryGroup(
                    category: category,
                    subcategories: Self.subgroups(parts, below: [category], rank: rank),
                    functions: Self.functionGroups(parts.filter { $0.drawer.count == 1 }, rank: rank)
                )
            }
    }

    /// The groups one drawer level below `path`, from the parts listed
    /// under it: each group holds its parts' deeper levels as subgroups, and
    /// the parts whose path ends there by function (or directly, as
    /// `listsPartsDirectly`).
    private static func subgroups(_ parts: [CatalogPart], below path: [String],
                                  rank: [String: Int]) -> [SubcategoryGroup] {
        let level = path.count
        let byName = Dictionary(grouping: parts.filter { $0.drawer.count > level }, by: { $0.drawer[level] })
        return byName
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { name, parts in
                let groupPath = path + [name]
                let own = parts.filter { $0.drawer.count == groupPath.count }
                let subgroups = Self.subgroups(parts, below: groupPath, rank: rank)
                return listsPartsDirectly(own) || repeatsDrawerName(own, name)
                    ? SubcategoryGroup(subcategory: name, path: groupPath, subgroups: subgroups,
                                       functions: [], parts: byCountThenName(own, rank: rank))
                    : SubcategoryGroup(subcategory: name, path: groupPath, subgroups: subgroups,
                                       functions: functionGroups(own, rank: rank))
            }
    }

    /// DIP switches, LEDs, 7-segment displays, character LCDs, nixie tubes and
    /// the generic header are listed with no function level. Tactile switches are grouped
    /// by body size instead (`drawerLeaf`), slide switches by circuit and action
    /// (`SPDT On-On`, their function).
    private static func listsPartsDirectly(_ parts: [CatalogPart]) -> Bool {
        parts.allSatisfy {
            ($0.kind == PART_SWITCH && !$0.isTactileSwitch && !$0.isSlideSwitch) || $0.kind == PART_LED || $0.kind == PART_7SEG || $0.kind == PART_CHARLCD
                || $0.kind == PART_NIXIE || $0.kind == PART_DEKATRON || $0.kind == PART_LCD
                || $0.kind == PART_HEADER
        }
    }

    /// Card slots whose function is the drawer's own name (Card slot → SD →
    /// SD): a function level would repeat the group's title, so they are
    /// listed directly. A drawer that also holds other functions (SIM →
    /// Micro SIM, Nano SIM, SIM) keeps its function level.
    private static func repeatsDrawerName(_ parts: [CatalogPart], _ name: String) -> Bool {
        !parts.isEmpty && parts.allSatisfy { $0.isCardSlot && $0.function.caseInsensitiveCompare(name) == .orderedSame }
    }

    /// Parts by count (positions, digits), then by name.
    private static func byCountThenName(_ parts: [CatalogPart], rank: [String: Int]) -> [CatalogPart] {
        parts.sorted {
            ($0.count ?? 0) != ($1.count ?? 0)
                ? ($0.count ?? 0) < ($1.count ?? 0)
                : rank[$0.name, default: 0] < rank[$1.name, default: 0]
        }
    }

    /// Parts grouped by `drawerLeaf` (the function, or a tactile switch's
    /// size), groups in natural order, parts as given (`groups` passes them in
    /// natural name order) ("8 KB" before "32 KB").
    /// Capacitor leaves ("10 pF" … "4700 µF") go by capacitance instead
    /// (plan-33-H): the smallest `farads` of each group, ties by name; and
    /// resistor leaves ("< 1 kΩ" … "≥ 1 MΩ") by resistance (plan-34-G).
    private static func functionGroups(_ parts: [CatalogPart], rank: [String: Int] = [:]) -> [FunctionGroup] {
        let groups = Dictionary(grouping: parts, by: \.drawerLeaf)
        // The kind the leaf's parts share, if every part has a `drawerValue`
        // (crystal, inductor, fuse, buzzer, relay).
        let sharesDrawerValue = !parts.isEmpty && parts.allSatisfy { $0.drawerValue != nil }
        let relays = !parts.isEmpty && parts.allSatisfy { $0.relay != nil }
        let value: (CatalogPart) -> Double? =
            !parts.isEmpty && parts.allSatisfy({ $0.capacitorFarads != nil }) ? { $0.capacitorFarads }
            : !parts.isEmpty && parts.allSatisfy({ $0.resistorOhms != nil }) ? { $0.resistorOhms }
            // Relay leaves keep the name order: only their parts go by value.
            : sharesDrawerValue && !relays ? { $0.drawerValue }
            : { _ in nil }
        // Crystal leaves ("Under 4 MHz" … "32 MHz and over", "32 kHz" …) go
        // by the frequency of their lowest part (plan-44-G), the parts in a
        // leaf by frequency then name; inductor, fuse and buzzer leaves the
        // same by inductance, rated current and frequency, and a relay's
        // parts by coil voltage then name (plan-47-M).
        let byValue = !parts.isEmpty && parts.allSatisfy { value($0) != nil }
        func smallest(_ parts: [CatalogPart]) -> Double {
            parts.compactMap(value).min() ?? 0
        }
        return groups
            .sorted {
                if byValue {
                    let a = smallest($0.value), b = smallest($1.value)
                    if a != b { return a < b }
                }
                return $0.key.localizedStandardCompare($1.key) == .orderedAscending
            }
            .map { function, parts in
                let ordered = sharesDrawerValue
                    ? parts.sorted {
                        ($0.drawerValue ?? 0) != ($1.drawerValue ?? 0)
                            ? ($0.drawerValue ?? 0) < ($1.drawerValue ?? 0)
                            : rank[$0.name, default: 0] < rank[$1.name, default: 0]
                    }
                    // The card slots of a leaf go by pin count then name (plan-54-O),
                    // and so do a leaf's terminal blocks and JST headers (plan-45-H).
                    : parts.allSatisfy({ $0.isCardSlot || $0.isWireConnector }) ? byCountThenName(parts, rank: rank)
                    : parts
                return FunctionGroup(function: function, parts: ordered, bitGroups: bitGroups(parts))
            }
    }

    /// Converters split by resolution, fewest bits first; empty when the
    /// parts are not converters.
    private static func bitGroups(_ parts: [CatalogPart]) -> [BitGroup] {
        let byBits = Dictionary(grouping: parts) { part -> Int? in
            guard case let .converter(_, bits, _) = part.details else { return nil }
            return bits
        }
        guard byBits[nil] == nil else { return [] }
        return byBits.compactMap { bits, parts in bits.map { BitGroup(bits: $0, parts: parts) } }
            .sorted { $0.bits < $1.bits }
    }

    private func sortedNames(_ names: [String]) -> [String] {
        Array(Set(names)).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    func load() { load(using: BackendCatalog.load(into:)) }

    /// `load()` with the parts `loader` loads into the library (a test's may fail).
    func load(using loader: (PartLibraryHandle) throws -> [CatalogPart]) {
        do {
            let parts = try loader(library)
            partsByName = Dictionary(parts.map { ($0.name, $0) }, uniquingKeysWith: { first, _ in first })
            socketIndex = nil
            // The 80 header sizes are one generic header in the drawer
            // (`HeaderFamily`); `part(named:)` still finds every size.
            entries = parts.filter {
                !$0.drawer.isEmpty && ($0.kind != PART_HEADER || $0.name == HeaderFamily.defaultPart)
            }
            derivedCache = nil
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }
}
