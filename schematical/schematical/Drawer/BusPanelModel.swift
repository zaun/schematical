import AppKit
import Observation
import UniformTypeIdentifiers

/// What the Bus panel's fields hold and what its controls do (plan-68-AB:
/// moved out of `BusPanel`, so `BusPanelModelTests` drive it without a view).
/// Fields commit on Return or when focus leaves them; a refused value keeps
/// the reason in its error and puts the field back.
@MainActor
@Observable
final class BusPanelModel {
    let document: SchematicDocument
    let busID: UUID

    /// The wire count field as typed; read back on commit.
    var countText = ""
    var countError: String?
    /// The name field as typed; read back on commit.
    var nameText = ""
    var nameError: String?
    var typeError: String?
    /// Why a wire move, rename of the list, or import was refused.
    var wireOrderError: String?
    /// The bus a name being committed would link with, while asking.
    var linkTarget: Bus?

    /// How the import and export panels and their alerts are put up; a test
    /// answers them, so no modal is left on the screen.
    @ObservationIgnored var presenter: ExportPresenter
    /// The presenter a new panel's model gets.
    static var presenter = ExportPresenter()

    init(document: SchematicDocument, busID: UUID, presenter: ExportPresenter? = nil) {
        self.document = document
        self.busID = busID
        self.presenter = presenter ?? Self.presenter
    }

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}

    var bus: Bus? { document.bus(busID) }

    // MARK: Header

    /// The PCB's card edge uses this bus: its count is locked.
    var isInUse: Bool { document.edgeBusID == busID }

    /// The wire count can't change: a card type sets it, or the card edge uses it.
    var isCountLocked: Bool { !(bus?.card.isCustom ?? false) || isInUse }

    var title: String {
        let name = bus?.name ?? ""
        return name.isEmpty ? "Bus" : name
    }

    var lineCountText: String {
        let count = bus?.lines.count ?? 0
        return "\(count) \(count == 1 ? "line" : "lines")"
    }

    var countHelp: String {
        isInUse ? "Locked: in use as the card edge" : isCountLocked ? "Set by the bus type" : ""
    }

    /// The wire list's help: why a card type's wires can't be edited.
    var wireHelp: String {
        bus?.card.isCustom ?? false ? "" : "Set by the bus type; set Type to Custom to change it"
    }

    var importHelp: String {
        bus?.card.isCustom ?? false ? "Load the wire names from a CSV or text file" : wireHelp
    }

    /// The wire list's footer: the reorder message, else how to reorder.
    var wireFooter: (text: String, isError: Bool)? {
        if let wireOrderError { return (wireOrderError, true) }
        if bus?.card.isCustom ?? false {
            return ("Drag a wire by its handle to reorder; a wire keeps its net and port pins.", false)
        }
        return nil
    }

    /// How many wires of the bus have each name; more than 1 warns.
    var wireUses: [String: Int] {
        (bus?.wires ?? []).reduce(into: [String: Int]()) { $0[$1.name, default: 0] += 1 }
    }

    // MARK: Name

    func appear() {
        resetCount()
        nameText = bus?.name ?? ""
    }

    /// Focus left the name field.
    func nameFocusChanged(from old: Bool, to new: Bool) {
        if old, !new { commitName() }
    }

    /// Undo, redo and other edits change the name from outside the field.
    func busNameChanged(_ name: String) {
        nameText = name
    }

    /// Commits the typed name; another bus's name asks before linking.
    func commitName() {
        guard linkTarget == nil else { return }
        if let target = document.busLinkTarget(busID, named: nameText) {
            nameError = nil
            linkTarget = target
            return
        }
        nameError = document.setBusName(busID, nameText)
        nameText = bus?.name ?? nameText
    }

    /// The link confirmation's title.
    var linkTitle: String { linkTarget.map { "Link with \($0.name)?" } ?? "" }

    static func linkMessage(_ target: Bus) -> String {
        "A bus named \(target.name) already exists. Renaming this bus will change its settings (type, wire count and wire names) to match \(target.name)'s, and the two buses will be linked: each wire is one net on both, and changing either bus's settings changes both."
    }

    /// Whether the link confirmation is up; dismissing it cancels.
    var isAskingToLink: Bool {
        get { linkTarget != nil }
        set { if !newValue { cancelLink() } }
    }

    func link(to target: Bus) {
        linkTarget = nil
        nameError = document.setBusName(busID, target.name, link: true)
        nameText = bus?.name ?? nameText
    }

    func cancelLink() {
        linkTarget = nil
        nameText = bus?.name ?? nameText
    }

    // MARK: Type

    var type: BoardType {
        get { bus?.card.type ?? .custom }
        set { editCard { $0.type = newValue } }
    }

    var pads: Int {
        get { bus?.card.customCard.pads ?? CustomCard().pads }
        set {
            editCard { card in
                card.customCard.pads = newValue
                card.customCard = card.customCard.clamped()
            }
        }
    }

    var bothSides: Bool {
        get { bus?.card.customCard.bothSides ?? false }
        set { editCard { $0.customCard.bothSides = newValue } }
    }

    var cardOptions: CardOptions { bus?.card.cardOptions ?? CardOptions() }

    func setCardOptions(_ options: CardOptions) {
        editCard { $0.cardOptions = options }
    }

    private func editCard(_ change: (inout BusCard) -> Void) {
        guard var card = bus?.card else { return }
        change(&card)
        typeError = document.setBusCard(busID, card)
    }

    // MARK: Wire count

    /// The count field edits `countText`; it is read back on commit.
    var countValue: Int? {
        get { Int(countText) }
        set { countText = newValue.map(String.init) ?? "" }
    }

    /// The stepper sets the count straight away.
    var stepperValue: Int {
        get { bus?.wires.count ?? Bus.defaultWireCount }
        set {
            countError = document.setBusWireCount(busID, newValue)
            resetCount()
        }
    }

    /// Focus left the count field.
    func countFocusChanged(from old: Bool, to new: Bool) {
        if old, !new { commitCount() }
    }

    func commitCount() {
        if let value = Int(countText.trimmingCharacters(in: .whitespaces)) {
            countError = document.setBusWireCount(busID, value)
        }
        resetCount()
    }

    func resetCount() {
        countText = String(bus?.wires.count ?? 0)
    }

    // MARK: Wires

    /// An item of a row's reorder menu.
    struct WireMove: Equatable {
        let title: String
        let place: Int
        let isEnabled: Bool
    }

    /// The reorder menu of wire `index` of `count`: one place up or down,
    /// then (after a divider) to an end.
    static func moves(index: Int, count: Int) -> [WireMove] {
        [WireMove(title: "Move Up", place: index - 1, isEnabled: index > 0),
         WireMove(title: "Move Down", place: index + 1, isEnabled: index < count - 1),
         WireMove(title: "Move to Top", place: 0, isEnabled: index > 0),
         WireMove(title: "Move to Bottom", place: count - 1, isEnabled: index < count - 1)]
    }

    func moveWire(_ id: UUID, to place: Int) {
        wireOrderError = document.moveBusWire(bus: busID, wire: id, to: place)
    }

    /// A dragged wire dropped on row `index` (holding `target`) takes its
    /// place; false when the drop is not a wire of this bus, or is the row's own.
    func drop(_ items: [BusWireDragItem], on target: UUID, at index: Int) -> Bool {
        guard let item = items.first, item.bus == busID, item.wire != target else { return false }
        moveWire(item.wire, to: index)
        return true
    }

    /// Renames a wire; returns why it was refused (nil when it wasn't) and
    /// the name the field shows after.
    func renameWire(_ wire: BusWire, to name: String) -> (error: String?, name: String) {
        let error = document.renameBusWire(bus: busID, wire: wire.id, to: name)
        return (error, bus?.wires.first { $0.id == wire.id }?.name ?? wire.name)
    }

    /// The warning on a wire whose name `uses` wires share.
    static func sharedNameHelp(_ name: String, uses: Int) -> String {
        uses > 1 ? "\(name) is used \(uses) times in this bus. Each wire is still its own net." : ""
    }

    // MARK: Import and export

    /// The file name Export… suggests.
    var exportFileName: String { "\(title) wires.csv" }

    /// Saves the bus's wire names (see `BusWireList`).
    func exportWires() async {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.commaSeparatedText]
        panel.nameFieldStringValue = exportFileName
        guard let url = await presenter.choose(panel, nil) else { return }
        await exportWires(to: url)
    }

    func exportWires(to url: URL) async {
        do {
            try BusWireList.text((bus?.wires ?? []).map(\.name)).write(to: url, atomically: true, encoding: .utf8)
            wireOrderError = nil
        } catch {
            _ = await presenter.alert(NSAlert(error: error), nil)
        }
    }

    /// Loads wire names from a file onto this bus (see `BusWireList`).
    func importWires() async {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.commaSeparatedText, .plainText]
        panel.allowsMultipleSelection = false
        guard let url = await presenter.choose(panel, nil) else { return }
        await importWires(from: url)
    }

    func importWires(from url: URL) async {
        do {
            let text = try String(contentsOf: url, encoding: .utf8)
            switch BusWireList.names(in: text) {
            case let .success(names): wireOrderError = document.setBusWireNames(busID, names)
            case let .failure(error): wireOrderError = error.message
            }
        } catch {
            _ = await presenter.alert(NSAlert(error: error), nil)
        }
    }
}

/// What the Bus Port panel does (plan-68-AB: moved out of `BusPortPanel`).
@MainActor
@Observable
final class BusPortPanelModel {
    let document: SchematicDocument
    let portID: UUID
    /// Why the last change was refused.
    var message: String?

    init(document: SchematicDocument, portID: UUID) {
        self.document = document
        self.portID = portID
    }

    nonisolated deinit {}

    var port: BusPort? { document.port(portID)?.port }

    var side: ChipLayout.Side {
        get { port?.side ?? .bottom }
        set { message = document.setPortSide(portID, newValue) }
    }

    func isPin(_ wire: UUID) -> Bool {
        port?.wireIDs.contains(wire) ?? false
    }

    /// Ticks or unticks one wire.
    func setPin(_ wire: UUID, _ on: Bool) {
        var ids = port?.wireIDs ?? []
        if on { ids.insert(wire) } else { ids.remove(wire) }
        set(ids)
    }

    func selectAll() {
        set(Set((document.port(portID)?.bus.wires ?? []).map(\.id)))
    }

    func deselectAll() {
        set([])
    }

    /// Stores the ticked wires; a refusal shows its message. Unticking a pin
    /// with wires on it asks first (the window's confirm dialog).
    private func set(_ ids: Set<UUID>) {
        message = document.requestPortWires(portID, ids)
    }

    static func title(_ side: ChipLayout.Side) -> String {
        switch side {
        case .top: "Top"
        case .bottom: "Bottom"
        case .left: "Left"
        case .right: "Right"
        }
    }
}
