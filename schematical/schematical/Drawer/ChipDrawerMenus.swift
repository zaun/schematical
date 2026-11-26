import AppKit

/// The filter menus a drawer group header carries (plan-68-AB: the
/// decisions out of `ChipDrawerView`).
enum DrawerHeaderMenu: Equatable {
    /// Family and gate count: gates, flip-flops and the other logic kinds.
    case logic
    /// A count of positions.
    case switches
    /// A color: LEDs and displays.
    case colors
    /// Digit count and size: a group of 7-segment displays.
    case displays
    /// An actuator: the tactile switch group.
    case tactile
    case none

    /// The kinds whose category header has the family and gate count menus.
    static let logicKinds = Set([PART_GATE, PART_DFF, PART_JKFF, PART_LATCH, PART_COUNTER, PART_SHIFT,
                                 PART_DECODER, PART_BUFFER, PART_MUX, PART_ARITH, PART_REGISTER].map(\.rawValue))

    /// The menus of a category holding parts of `kinds`.
    static func forCategory(kinds: Set<UInt32>) -> DrawerHeaderMenu {
        if !kinds.isDisjoint(with: logicKinds) { return .logic }
        if kinds.contains(PART_SWITCH.rawValue) { return .switches }
        if kinds.contains(PART_LED.rawValue) || kinds.contains(PART_7SEG.rawValue) { return .colors }
        return .none
    }

    /// The menus of a group below a category. A group's kinds include its
    /// sub-groups', so a group with sub-groups leaves the menus to them
    /// (Vintage → Display tubes).
    static func forGroup(_ group: PartCatalogStore.SubcategoryGroup, catalog: PartCatalogStore) -> DrawerHeaderMenu {
        guard group.subgroups.isEmpty else { return .none }
        if catalog.kinds(inGroup: group.path).contains(PART_7SEG.rawValue) { return .displays }
        return catalog.isTactile(group: group.path) ? .tactile : .none
    }

    /// "1 gate", "4 gates": a count of `unit`.
    static func countLabel(_ count: Int, unit: String) -> String {
        count == 1 ? "1 \(unit)" : "\(count) \(unit)s"
    }
}

/// A drawer part's right-click menu (plan-53-C, -E; plan-32-C), and its
/// subtitle (plan-68-AB: out of `ChipDrawerView`).
@MainActor
enum DrawerPartMenu {
    enum Item: Equatable, Identifiable {
        /// A part that came with a schematic can be kept.
        case addToLibrary
        /// Edit the library part itself; Save updates it.
        case edit(uuid: String)
        /// A kept part can be taken out again.
        case remove
        /// A copy with a new uuid; the library part is never edited.
        case copyToEditor

        var title: String {
            switch self {
            case .addToLibrary: "Add to User Library"
            case .edit: "Edit…"
            case .remove: "Remove from User Library…"
            case .copyToEditor: "Copy to Editor…"
            }
        }

        var id: String { title }
    }

    /// How the menu's alerts are put up: as a sheet on the key window, or on
    /// their own. A test answers them.
    static var presenter = ExportPresenter()

    private static func runAlert(_ alert: NSAlert) async -> NSApplication.ModalResponse {
        await presenter.alert(alert, presenter.keyWindow())
    }

    /// The menu of `part`, listed as `listing` in the catalog.
    static func items(for part: CatalogPart, listing: PartListing?) -> [Item] {
        let editable = PartKinds.name(of: part.kind).map { PartDraft.enabledKinds.contains($0) } ?? false
        var items: [Item] = []
        if let uuid = part.uuid {
            switch listing {
            case .file: items.append(.addToLibrary)
            case .user: items += (editable ? [.edit(uuid: uuid)] : []) + [.remove]
            default: break
            }
        }
        if editable { items.append(.copyToEditor) }
        return items
    }

    /// Does what `item` says to `part`; `open` opens the part editor.
    static func perform(_ item: Item, part: CatalogPart, catalog: PartCatalogStore,
                        open: (PartEditorTarget) -> Void) async {
        switch item {
        case .addToLibrary:
            let failures = UserLibraryPrompts.add([part], catalog: catalog)
            if !failures.isEmpty {
                _ = await runAlert(UserLibraryPrompts.failuresAlert(title: "Couldn't add to your User Library",
                                                                    failures: failures))
            }
        case let .edit(uuid):
            open(PartEditorTarget(source: .libraryPart(uuid: uuid)))
        case .remove:
            await remove(part, catalog: catalog)
        case .copyToEditor:
            open(PartEditorTarget(source: .copy(name: part.name)))
        }
    }

    /// Remove from User Library…: after the confirmation, deletes the part's
    /// library file and takes it out of the drawer.
    private static func remove(_ part: CatalogPart, catalog: PartCatalogStore) async {
        guard let uuid = part.uuid,
              await runAlert(UserLibraryPrompts.removeAlert(name: part.name)) == .alertFirstButtonReturn
        else { return }
        do {
            try catalog.removeFromUserLibrary(uuid: uuid)
        } catch {
            _ = await runAlert(UserLibraryPrompts.failuresAlert(
                title: "Couldn't remove from your User Library", failures: [(part.name, error)]))
        }
    }

    /// Package, then the kind's facts: family, speed grade, actuator or color;
    /// for a 7-segment display also its size and whether it is common anode
    /// (CA) or common cathode (CC); last, its voltage (`voltageLabel`). A part
    /// with footprint choices shows their count in the package slot.
    static func subtitle(for part: CatalogPart) -> String {
        if part.kind == PART_HEADER { return HeaderFamily.subtitle }
        let package = part.footprintChoices.isEmpty ? part.package : "\(part.footprintChoices.count) footprints"
        return ([package] + part.detailLabels + [part.voltageLabel].compactMap { $0 })
            .filter { !$0.isEmpty }
            .joined(separator: " · ")
    }
}

/// What the drawer's footer and empty list say about the part library.
struct DrawerStatus: Equatable {
    /// The footer's symbol; nil while loading (a spinner shows).
    let icon: String?
    let text: String
    let isFailure: Bool

    init(_ state: PartCatalogStore.LoadState, partCount: Int) {
        switch state {
        case .notLoaded:
            (icon, text, isFailure) = (nil, "Loading backend", false)
        case .loaded:
            (icon, text, isFailure) = ("checkmark.circle.fill", "\(partCount) parts from backend", false)
        case let .failed(message):
            (icon, text, isFailure) = ("exclamationmark.triangle.fill", message, true)
        }
    }

    /// Why the drawer lists no parts at all.
    static func emptyDescription(_ state: PartCatalogStore.LoadState) -> String {
        switch state {
        case .notLoaded: "Loading the part library…"
        case .loaded: "The backend reports no supported parts."
        case let .failed(message): message
        }
    }
}
