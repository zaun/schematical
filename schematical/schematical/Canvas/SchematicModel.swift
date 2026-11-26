import CoreGraphics
import Foundation
import Observation

// MARK: - Document

/// Proof that a change to saved content runs inside a recorded edit
/// (`SchematicDocument.edit`): only this file can make one, so a sheet's
/// `update…` cannot be called from anywhere else (plan-68-D).
struct EditScope {
    fileprivate init() {}
}

/// The schematic being edited: placed blocks, wires, and each page's selection.
@MainActor
@Observable
final class SchematicDocument {
    /// Not isolated: closing a document frees this while SwiftUI tears the
    /// editor down, and an isolated deinit there aborts in the concurrency
    /// runtime (`StopLookupScope`; see `VerifyDatasheetLifetimeTests`).
    /// Nothing here needs the main actor to be freed.
    nonisolated deinit {}

    private(set) var chips: [PlacedChip] = [] {
        didSet {
            chipsRevision &+= 1
            ownCopperRevision &+= 1
        }
    }
    /// What is drawn on the Schematic page: wires, junctions, buses, net
    /// labels and text blocks (plan-68-D). Changed only through recorded
    /// edits (`edit`), so every change is an undo step.
    let sheet = SchematicSheet()
    var wires: [Wire] { sheet.wires }
    var junctions: [Junction] { sheet.junctions }
    /// Buses, in drawing order (plan-13). They carry no current themselves.
    var buses: [Bus] { sheet.buses }

    /// Counts every change to what `connectivity()` reads (parts, wires,
    /// buses, net labels, traces, vias, holes, board settings), so the
    /// connectivity and ratsnest are built once per change, not once per
    /// redraw. Observed: a view that reads only the kept result still
    /// redraws when it goes stale.
    var copperRevision: Int { ownCopperRevision &+ sheet.copperRevision &+ layout.copperRevision }
    /// The document's part of `copperRevision`; the sheet counts its own.
    private var ownCopperRevision = 0
    /// Counts every change to `chips`, for `chipPads`.
    @ObservationIgnored private var chipsRevision = 0
    /// The last `connectivity()` and `ratsnest()`, and the `copperRevision`
    /// they were built at (see Ratsnest.swift).
    var keptConnectivity: (revision: Int, copper: BoardConnectivity)? {
        get { layout.keptConnectivity }
        set { layout.keptConnectivity = newValue }
    }
    /// The last pour fills, with the pours, keep-outs and board edge they were made for (plan-52-D, plan-65-C).
    var keptPourFills: (revision: Int, pours: [BoardPour], keepouts: [BoardKeepout], edge: BoardEdge,
                        fills: [UUID: PourFill])? {
        get { layout.keptPourFills }
        set { layout.keptPourFills = newValue }
    }
    var keptRatsnest: (revision: Int, lines: [(CGPoint, CGPoint)])? {
        get { layout.keptRatsnest }
        set { layout.keptRatsnest = newValue }
    }
    @ObservationIgnored private var keptChipPads: (revision: Int, pads: [[PlacedPad]])?
    @ObservationIgnored private var keptVoltageIssues: (revision: Int, issues: Set<VoltageIssue>)?
    @ObservationIgnored private var keptChipIndex: (revision: Int, index: [UUID: Int])?

    /// The voltage problems the drawing has now, kept until anything in it
    /// changes: an edit's check only needs to build the netlist of the
    /// drawing it would make.
    func currentVoltageIssues(_ library: PartLibraryHandle) -> Set<VoltageIssue> {
        let revision = copperRevision
        if let kept = keptVoltageIssues, kept.revision == revision { return kept.issues }
        let found = VoltageCheck.issues(chips: chips, wires: wires, buses: buses, labels: netLabels, library: library)
        keptVoltageIssues = (revision, found)
        return found
    }

    /// Each part's place in `chips`, by id, built once per change to the
    /// parts: a wire's end finds its part here, not by searching `chips`.
    private var chipIndex: [UUID: Int] {
        if let kept = keptChipIndex, kept.revision == chipsRevision { return kept.index }
        var index: [UUID: Int] = [:]
        index.reserveCapacity(chips.count)
        for (position, chip) in chips.enumerated() where index[chip.id] == nil { index[chip.id] = position }
        keptChipIndex = (chipsRevision, index)
        return index
    }

    /// Each part's pads on the board (`PlacedChip.boardPads`), in `chips`
    /// order, built once per change to the parts.
    var chipPads: [[PlacedPad]] {
        let chips = chips
        if let kept = keptChipPads, kept.revision == chipsRevision { return kept.pads }
        let pads = chips.map(\.boardPads)
        keptChipPads = (chipsRevision, pads)
        return pads
    }
    /// Text blocks, in drawing order. Drawn under everything else.
    var notes: [SchematicNote] { sheet.notes }
    /// The document's title, author, dates and title block (plan-64).
    private(set) var documentInfo = DocumentInfo()
    /// Net labels, in drawing order. Labels with the same name are one net.
    var netLabels: [NetLabel] { sheet.netLabels }
    /// A text block whose words the sidebar should start editing: one just
    /// placed, or double-clicked. The sidebar clears it. Editor state only.
    var editingNoteID: UUID?
    /// What is selected on the Schematic page: parts, wires and junctions
    /// (plan-15). Editor state only, never saved. Changing it clears the bus
    /// and port selection.
    private(set) var schematicSelection = ItemSelection<SchematicItem>() {
        didSet {
            if !schematicSelection.isEmpty { clearSelection(except: .items) }
            prunePicks()
            mirrorSchematicSelectionToBoard()
        }
    }
    /// What is selected on the PCB page: footprints, board texts and traces.
    /// A part and its footprint are one selection with the Schematic page
    /// (see `mirrorSchematicSelectionToBoard`); the rest is kept apart.
    private(set) var boardSelection = ItemSelection<BoardItem>() {
        didSet { mirrorBoardSelectionToSchematic() }
    }

    /// Set while one page's selection is being copied to the other, so the
    /// copy does not copy back.
    private var isMirroringSelection = false

    /// The parts selected on the Schematic page.
    private var schematicSelectedParts: Set<UUID> {
        Set(schematicSelection.items.compactMap { if case let .chip(id) = $0 { id } else { nil } })
    }

    /// The parts whose footprint is selected on the PCB page.
    private var boardSelectedParts: Set<UUID> {
        Set(boardSelection.items.compactMap { if case let .footprint(id) = $0 { id } else { nil } })
    }

    /// Selecting a part on the Schematic page selects its footprint on the
    /// PCB page. When the two pages disagree about which parts are selected,
    /// the PCB selection becomes just those footprints (a part without a
    /// footprint has none). Texts and traces are left alone while the parts
    /// agree.
    private func mirrorSchematicSelectionToBoard() {
        guard !isMirroringSelection else { return }
        let parts = schematicSelectedParts
        let withFootprint = Set(chips.lazy.filter { $0.board != nil }.map(\.id))
        let footprints = parts.intersection(withFootprint)
        guard footprints != boardSelectedParts else { return }
        isMirroringSelection = true
        defer { isMirroringSelection = false }
        boardSelection.set(Set(footprints.map(BoardItem.footprint)))
    }

    /// Selecting a footprint on the PCB page selects its part on the
    /// Schematic page, as `mirrorSchematicSelectionToBoard`. A text or trace
    /// has no part, so selecting one leaves no part selected.
    private func mirrorBoardSelectionToSchematic() {
        guard !isMirroringSelection else { return }
        let parts = boardSelectedParts
        guard parts != schematicSelectedParts else { return }
        isMirroringSelection = true
        defer { isMirroringSelection = false }
        schematicSelection.set(Set(parts.map(SchematicItem.chip)))
    }

    /// The bus, when it is the only thing selected on the Schematic page
    /// (buses are items of `schematicSelection`). Setting one makes it the
    /// whole selection; setting nil clears a lone selected bus.
    var selectedBusID: UUID? {
        get {
            if case let .bus(id)? = schematicSelection.sole { return id }
            return nil
        }
        set {
            if let newValue {
                if schematicSelection.items != [.bus(newValue)] { schematicSelection.set([.bus(newValue)]) }
            } else if selectedBusID != nil {
                schematicSelection.clear()
            }
        }
    }
    /// The selected bus port. Selecting one clears the other Schematic selections.
    var selectedPortID: UUID? {
        didSet {
            if selectedPortID != nil { clearSelection(except: .port) }
        }
    }

    private enum SelectionKind { case items, port }

    /// On the Schematic page, the item set (buses included) and a port
    /// exclude each other: clears the one that isn't `kept`.
    private func clearSelection(except kept: SelectionKind) {
        if kept != .items, !schematicSelection.isEmpty { schematicSelection.clear() }
        if kept != .port, selectedPortID != nil { selectedPortID = nil }
    }

    /// The part, when it is the only thing selected on the Schematic page.
    var selectedChipID: UUID? {
        if case let .chip(id)? = schematicSelection.sole { return id }
        return nil
    }

    /// The part whose footprint is the only thing selected on the PCB page.
    var selectedFootprintID: UUID? {
        if case let .footprint(id)? = boardSelection.sole { return id }
        return nil
    }

    /// Whether the title block is the only thing selected on the Schematic page.
    var selectedTitleBlock: Bool { schematicSelection.sole == .titleBlock }

    /// The text block, when it is the only thing selected on the Schematic page.
    var selectedNoteID: UUID? {
        if case let .note(id)? = schematicSelection.sole { return id }
        return nil
    }

    /// The net label, when it is the only thing selected on the Schematic page.
    var selectedNetLabelID: UUID? {
        if case let .label(id)? = schematicSelection.sole { return id }
        return nil
    }

    /// The board text, when it is the only thing selected on the PCB page.
    var selectedTextID: UUID? {
        if case let .text(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The trace, when it is the only thing selected on the PCB page.
    var selectedTraceID: UUID? {
        if case let .trace(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The via, when it is the only thing selected on the PCB page.
    var selectedViaID: UUID? {
        if case let .via(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The hole, when it is the only thing selected on the PCB page.
    var selectedHoleID: UUID? {
        if case let .hole(id)? = boardSelection.sole { return id }
        return nil
    }

    // MARK: Selecting

    /// Selects only `item` on the Schematic page.
    func select(_ item: SchematicItem) {
        schematicSelection.replace(with: item)
    }

    /// Selects only `item` on the PCB page.
    func select(_ item: BoardItem) {
        boardSelection.replace(with: item)
    }

    func toggleSelection(_ item: SchematicItem) {
        schematicSelection.toggle(item)
    }

    func toggleSelection(_ item: BoardItem) {
        boardSelection.toggle(item)
    }

    func isSelected(_ item: SchematicItem) -> Bool {
        schematicSelection.contains(item)
    }

    func isSelected(_ item: BoardItem) -> Bool {
        boardSelection.contains(item)
    }

    /// A Pointer press on `item`; see `ItemSelection.press`.
    func pressSelection(_ item: SchematicItem, shift: Bool) -> SelectionPress<SchematicItem> {
        schematicSelection.press(item, shift: shift)
    }

    /// A Select press on `item`; see `ItemSelection.press`.
    func pressSelection(_ item: BoardItem, shift: Bool) -> SelectionPress<BoardItem> {
        boardSelection.press(item, shift: shift)
    }

    func releaseSelection(_ press: SelectionPress<SchematicItem>, wasClick: Bool) {
        schematicSelection.release(press, wasClick: wasClick)
        // A click on one of several picked units keeps only that one.
        if wasClick, let pick = pendingPick, isSelected(.chip(pick.chipID)) { pickedUnits[pick.chipID] = [pick.key] }
        pendingPick = nil
    }

    func releaseSelection(_ press: SelectionPress<BoardItem>, wasClick: Bool) {
        boardSelection.release(press, wasClick: wasClick)
    }

    /// A Pointer press on empty canvas, which pans: it clears the Schematic
    /// selection, unless Shift is down.
    func pressEmptySchematic(shift: Bool) {
        if !shift { clearSchematicSelection() }
    }

    /// A Select press on empty board, which pans: it clears the PCB
    /// selection, unless Shift is down.
    func pressEmptyBoard(shift: Bool) {
        if !shift { clearBoardSelection() }
    }

    // MARK: Box select

    /// The parts, junctions, wires, text blocks, net labels and buses a box
    /// (schematic coordinates) touches. A bus is in when any of its lines
    /// is.
    func schematicItems(in rect: CGRect) -> Set<SchematicItem> {
        var items = Set<SchematicItem>()
        for chip in chips where !chip.pieces(in: rect).isEmpty {
            items.insert(.chip(chip.id))
        }
        for junction in junctions where rect.contains(junction.point) {
            items.insert(.junction(junction.id))
        }
        for note in notes where note.rect.intersects(rect) {
            items.insert(.note(note.id))
        }
        if let block = titleBlockRect, block.intersects(rect) { items.insert(.titleBlock) }
        for label in netLabels where label.rect.intersects(rect) {
            items.insert(.label(label.id))
        }
        for wire in wires {
            let path = path(for: wire)
            if zip(path, path.dropFirst()).contains(where: { Self.segment($0, $1, touches: rect) }) {
                items.insert(.wire(wire.id))
            }
        }
        for bus in buses where bus.lines.contains(where: { Self.segment($0.a, $0.b, touches: rect) }) {
            items.insert(.bus(bus.id))
        }
        return items
    }

    /// The drawn footprints, texts and traces a box (board mm) touches.
    func boardItems(in rect: CGRect, visible: BoardVisibility = BoardVisibility()) -> Set<BoardItem> {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            if let bounds = chip.boardBounds, bounds.intersects(rect) { items.insert(.footprint(chip.id)) }
        }
        for text in texts where visible.showsText(text) && text.boardRect.intersects(rect) {
            items.insert(.text(text.id))
        }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            let grown = rect.insetBy(dx: -trace.widthMM / 2, dy: -trace.widthMM / 2)
            if trace.segments.contains(where: { Self.segment($0.0, $0.1, touches: grown) }) {
                items.insert(.trace(trace.id))
            }
        }
        if visible.showsAnyCopper {
            for via in vias where via.rect.intersects(rect) { items.insert(.via(via.id)) }
            for hole in holes where hole.rect.intersects(rect) { items.insert(.hole(hole.id)) }
        }
        // A pour or keep-out is taken whole: its rectangle inside the box.
        for pour in pours where visible.shows(.copper(pour.layer)) && rect.contains(pour.rect) {
            items.insert(.pour(pour.id))
        }
        for keepout in keepouts where keepoutIsShown(keepout, visible) && rect.contains(keepout.rect) {
            items.insert(.keepout(keepout.id))
        }
        // A cutout likewise: its bounds inside the box.
        for cutout in cutouts where rect.contains(cutout.shape.bounds) { items.insert(.cutout(cutout.id)) }
        return items
    }

    /// A Shift-drag on empty Schematic canvas: the selection is what it was
    /// at the press (`base`) plus everything the box touches.
    func boxSelectSchematic(_ rect: CGRect, base: Set<SchematicItem>) {
        let items = base.union(schematicItems(in: rect))
        if items != schematicSelection.items { schematicSelection.set(items) }
        // Of a part drawn as its units, the box picks the units it touches
        // (plan-67). A part that was selected before the box keeps its picks.
        for chip in chips where chip.isSplit && items.contains(.chip(chip.id)) && !base.contains(.chip(chip.id)) {
            let touched = Set(chip.pieces(in: rect).compactMap(\.unitKey))
            if pickedUnits[chip.id] != touched { pickedUnits[chip.id] = touched }
        }
    }

    /// A Shift-drag on empty board: as `boxSelectSchematic`, on the PCB page.
    func boxSelectBoard(_ rect: CGRect, base: Set<BoardItem>, visible: BoardVisibility = BoardVisibility()) {
        let items = base.union(boardItems(in: rect, visible: visible))
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Selects exactly `items` on the PCB page (a Design Rule Check line).
    func selectBoardItems(_ items: Set<BoardItem>) {
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Selects exactly `items` on the Schematic page (an Electrical Rules Check line).
    func selectSchematicItems(_ items: Set<SchematicItem>) {
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// Edit › Select All on the Schematic: every part, wire, junction, text
    /// block, net label and bus.
    func selectAllSchematic() {
        var items = Set<SchematicItem>()
        items.formUnion(chips.map { .chip($0.id) })
        items.formUnion(wires.map { .wire($0.id) })
        items.formUnion(junctions.map { .junction($0.id) })
        items.formUnion(notes.map { .note($0.id) })
        items.formUnion(netLabels.map { .label($0.id) })
        items.formUnion(buses.map { .bus($0.id) })
        if documentInfo.titleBlock != nil { items.insert(.titleBlock) }
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// Edit › Select All on the PCB: every footprint, text, trace, via and
    /// hole drawn with `visible` (as a box select would take them).
    func selectAllBoard(visible: BoardVisibility = BoardVisibility()) {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            items.insert(.footprint(chip.id))
        }
        for text in texts where visible.showsText(text) { items.insert(.text(text.id)) }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            items.insert(.trace(trace.id))
        }
        if visible.showsAnyCopper {
            items.formUnion(vias.map { .via($0.id) })
            items.formUnion(holes.map { .hole($0.id) })
        }
        for pour in pours where visible.shows(.copper(pour.layer)) { items.insert(.pour(pour.id)) }
        for keepout in keepouts where keepoutIsShown(keepout, visible) { items.insert(.keepout(keepout.id)) }
        for cutout in cutouts { items.insert(.cutout(cutout.id)) }
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Clears the Schematic page's selection: items (buses included) and
    /// port.
    func clearSchematicSelection() {
        schematicSelection.clear()
        selectedPortID = nil
    }

    func clearBoardSelection() {
        boardSelection.clear()
    }

    /// Clears both pages' selections (opening a file, starting a run).
    func clearAllSelections() {
        clearSchematicSelection()
        clearBoardSelection()
    }

    /// Drops selected ids that are no longer in the document.
    private func pruneSelection() {
        let chipIDs = Set(chips.map(\.id)), wireIDs = Set(wires.map(\.id))
        let junctionIDs = Set(junctions.map(\.id)), textIDs = Set(texts.map(\.id))
        let noteIDs = Set(notes.map(\.id)), labelIDs = Set(netLabels.map(\.id))
        let busIDs = Set(buses.map(\.id))
        schematicSelection.filter { item in
            switch item {
            case let .chip(id): chipIDs.contains(id)
            case let .wire(id): wireIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case let .note(id): noteIDs.contains(id)
            case let .label(id): labelIDs.contains(id)
            case let .bus(id): busIDs.contains(id)
            case .titleBlock: documentInfo.titleBlock != nil
            }
        }
        let footprints = Set(chips.filter { $0.board != nil }.map(\.id))
        let traceIDs = Set(traces.map(\.id)), viaIDs = Set(vias.map(\.id)), holeIDs = Set(holes.map(\.id))
        let pourIDs = Set(pours.map(\.id)), keepoutIDs = Set(keepouts.map(\.id)), cutoutIDs = Set(cutouts.map(\.id))
        boardSelection.filter { item in
            switch item {
            case let .footprint(id): footprints.contains(id)
            case let .text(id): textIDs.contains(id)
            case let .trace(id): traceIDs.contains(id)
            case let .via(id): viaIDs.contains(id)
            case let .hole(id): holeIDs.contains(id)
            case let .pour(id): pourIDs.contains(id)
            case let .keepout(id): keepoutIDs.contains(id)
            case let .cutout(id): cutoutIDs.contains(id)
            }
        }
    }

    /// What is on the PCB page: the board's settings, texts, copper, holes,
    /// pours, keep-outs and cutouts (plan-68-E). Changed only through
    /// recorded edits (`edit`), so every change is an undo step.
    let layout = BoardContent()
    /// The PCB's size, layers, colour and units.
    var board: BoardSettings { layout.settings }
    /// The electrical rules check's settings (plan-63-E): severities, the pin
    /// map and the excluded violations. Changed through the edit functions
    /// (`setErcSeverity` …), so every change is an undo step.
    private(set) var erc = ErcSettings()
    /// Free texts on the PCB, in drawing order.
    var texts: [BoardText] { layout.texts }
    /// Copper traces on the PCB, in drawing order (plan-16).
    var traces: [BoardTrace] { layout.traces }
    /// Vias on the PCB, in drawing order.
    var vias: [BoardVia] { layout.vias }
    /// Drilled holes on the PCB (mounting holes and others), in drawing
    /// order.
    var holes: [BoardHole] { layout.holes }
    /// Copper pours (plan-52), in drawing order: a pour fills before the
    /// ones after it, and gives way to the ones before it.
    var pours: [BoardPour] { layout.pours }
    /// Keep-outs (plan-52): rectangles the pours stay out of.
    var keepouts: [BoardKeepout] { layout.keepouts }
    /// Cutouts (plan-65): regions of the board with no material, in drawing order.
    var cutouts: [BoardCutout] { layout.cutouts }

    /// The board's edge: its outline and every cutout's loop (plan-65). What
    /// every reader of the edge asks.
    var boardEdge: BoardEdge {
        BoardEdge(outline: board.outline, cutouts: cutouts.map { $0.shape.loop })
    }

    /// Lowest number tried for the next part's reference ("U" plus a number).
    private(set) var nextReference = 1

    /// Why the last wire or switch edit was refused (a voltage out of range),
    /// or nil. Cleared by the next accepted one; the window clears it after a
    /// few seconds.
    var editorMessage: String?

    /// The Electrical Rules Check window's marks on the schematic (plan-63-F):
    /// editor state like the selection, never saved and not an undo step.
    var ercMarkers: [ErcMarker] = []
    /// The keys of the violations whose lines are selected in that window.
    var ercSelectedKeys: Set<String> = []
    /// The point a selected line asks the canvas to bring into view.
    private(set) var ercFocus: ErcFocus?
    private var ercFocusSerial = 0

    /// Asks the canvas to bring `point` into view.
    func focusErc(on point: CGPoint) {
        ercFocusSerial += 1
        ercFocus = ErcFocus(point: point, serial: ercFocusSerial)
    }

    /// The part library, for the voltage check. With none, no edit is
    /// refused.
    /// The backend library (nil in some tests); the voltage check and the ERC read it.
    let library: PartLibraryHandle?

    init(library: PartLibraryHandle? = nil) {
        self.library = library
        // A new board starts from Settings ▸ DRC Defaults; from then on its
        // rules are its own, saved in its file.
        layout.updateSettings(EditScope()) {
            $0.rules = DesignRules.appDefaults()
            $0.rulesManufacturer = DRCManufacturer.appDefault()
        }
    }

    /// Gives a new, blank schematic the standard net classes
    /// (`NetClass.standardClasses`), outside the undo history. Does nothing
    /// once the board has any class.
    func addStandardNetClasses() {
        guard board.netClasses.isEmpty else { return }
        layout.updateSettings(EditScope()) { $0.netClasses = NetClass.standardClasses(rules: $0.rules) }
    }

    /// Replaces everything on the canvas, for example with a file that was
    /// just opened. Clears the selection.
    func replaceContents(chips: [PlacedChip], wires: [Wire], junctions: [Junction], buses: [Bus],
                         nextReference: Int, board: BoardSettings, texts: [BoardText],
                         traces: [BoardTrace], vias: [BoardVia] = [], holes: [BoardHole] = [],
                         pours: [BoardPour] = [], keepouts: [BoardKeepout] = [], cutouts: [BoardCutout] = [],
                         notes: [SchematicNote] = [], netLabels: [NetLabel] = [],
                         erc: ErcSettings = ErcSettings(),
                         documentInfo: DocumentInfo = DocumentInfo()) {
        self.erc = erc
        self.chips = chips
        self.documentInfo = documentInfo
        // Buses from older files have no name: number them in order.
        var named: [Bus] = []
        for var bus in buses {
            if bus.name.isEmpty { bus.name = Bus.defaultBusName(taken: (buses + named).map(\.name)) }
            named.append(bus)
        }
        // A file load replaces the content outside the undo history.
        sheet.restore(wires: wires, junctions: junctions, buses: named, netLabels: netLabels, notes: notes, EditScope())
        self.nextReference = max(nextReference, 1)
        layout.restore(settings: board, texts: texts, traces: traces, vias: vias, holes: holes, pours: pours,
                       keepouts: keepouts, cutouts: cutouts, EditScope())
        defer { dropStaleEdgeBus(EditScope()) }
        editingNoteID = nil
        clearAllSelections()
        editorMessage = nil
        clearUndoManager()
        recordingDepth = 0
        recordingStart = nil
        recordingName = nil
        recordingCoalescing = nil
        isGestureOpen = false
    }

    // MARK: Undo

    /// The document's one undo manager (plan-61-A). One recorded step is one
    /// undo group on it. An `NSDocument` showing this document is given it.
    @ObservationIgnored let undoManager = SchematicUndoManager()
    /// The step on top of the manager's undo stack while typing may still
    /// merge into it (`UndoStep.coalescing`).
    @ObservationIgnored private var openStep: UndoStep?
    /// Bumped when the manager's stacks change, so views reading `undoName`
    /// and `redoName` follow (an `UndoManager` is not observable).
    private var undoRevision = 0
    /// How many `recordEdit` calls (and an open gesture) are running. Only
    /// the outermost records.
    private var recordingDepth = 0
    private var recordingStart: DocumentState?
    /// The name of the step: of the outermost edit that changed something,
    /// the first one (a gesture's edits are siblings, the first names it).
    private var recordingName: String?
    /// The depth of the edit that gave `recordingName`.
    private var recordingNameDepth = 0
    private var recordingCoalescing: String?
    /// The outermost call's coalescing key, which wins when it gave one.
    private var outerCoalescing: String?
    /// True while the simulation runs: edits are not recorded and undo/redo
    /// do nothing. Set by `SimulationController`.
    var isPlaying = false {
        didSet { undoManager.isLocked = isPlaying }
    }
    /// A mouse drag is being recorded as one step.
    private(set) var isGestureOpen = false

    /// The saved part of the document, as a value.
    var state: DocumentState {
        DocumentState(chips: chips, wires: wires, junctions: junctions, buses: buses, notes: notes,
                      netLabels: netLabels, nextReference: nextReference, board: board, texts: texts, traces: traces, vias: vias, holes: holes,
                      pours: pours, keepouts: keepouts, cutouts: cutouts, erc: erc, documentInfo: documentInfo)
    }

    /// Writes a state back (undo, redo), then drops selections and editor
    /// state that point at items no longer there.
    private func setState(_ s: DocumentState) {
        if chips != s.chips { chips = s.chips }
        sheet.restore(wires: s.wires, junctions: s.junctions, buses: s.buses, netLabels: s.netLabels,
                      notes: s.notes, EditScope())
        if nextReference != s.nextReference { nextReference = s.nextReference }
        layout.restore(settings: s.board, texts: s.texts, traces: s.traces, vias: s.vias, holes: s.holes,
                       pours: s.pours, keepouts: s.keepouts, cutouts: s.cutouts, EditScope())
        if erc != s.erc { erc = s.erc }
        if documentInfo != s.documentInfo { documentInfo = s.documentInfo }
        pruneSelection()
        if let id = editingNoteID, !notes.contains(where: { $0.id == id }) { editingNoteID = nil }
        if let id = selectedBusID, bus(id) == nil { selectedBusID = nil }
        if let id = selectedPortID, portLocation(id) == nil { selectedPortID = nil }
    }

    /// Runs `body` as one undo step named `name` on the document's undo
    /// manager (`page` says where the edit was made; the step is the
    /// document's). Nested calls are part of the outermost
    /// one. The step is named by the outermost call that changed something
    /// (the first such, among a gesture's calls). An edit
    /// that changes nothing records nothing. Pushes with the same
    /// `coalescing` key merge until `endCoalescing()`.
    @discardableResult
    func recordEdit<T>(_ page: EditorPage, _ name: String, coalescing: String? = nil,
                       _ body: () throws -> T) rethrows -> T {
        guard !isPlaying else { return try body() }
        if recordingDepth == 0 { startRecording(coalescing: coalescing) }
        recordingDepth += 1
        let before = state
        defer {
            if recordingName == nil || recordingDepth < recordingNameDepth, state != before {
                recordingName = name
                recordingNameDepth = recordingDepth
                recordingCoalescing = coalescing
            }
            recordingDepth -= 1
            if recordingDepth == 0 { finishRecording() }
        }
        return try body()
    }

    /// `recordEdit`, handing `body` the scope a sheet change needs: every
    /// change to saved sheet content is inside a recorded edit (plan-68-D).
    @discardableResult
    func edit<T>(_ page: EditorPage, _ name: String, coalescing: String? = nil,
                 _ body: (EditScope) throws -> T) rethrows -> T {
        try recordEdit(page, name, coalescing: coalescing) { try body(EditScope()) }
    }

    /// A card-edge bus link that no longer counts goes (see `finishRecording`).
    private func dropStaleEdgeBus(_ scope: EditScope) {
        if board.edgeBus != nil, edgeBusID == nil { layout.updateSettings(scope) { $0.edgeBus = nil } }
    }

    private func startRecording(coalescing: String?) {
        recordingStart = state
        recordingName = nil
        recordingCoalescing = nil
        outerCoalescing = coalescing
    }

    private func finishRecording() {
        defer {
            recordingStart = nil
            recordingName = nil
            recordingCoalescing = nil
            outerCoalescing = nil
        }
        guard let start = recordingStart else { return }
        // A card-edge bus link that no longer counts (the card's pin count
        // changed, the board stopped being a card, or the bus is gone) is
        // set back to None, in the same step, so no hidden link stays.
        dropStaleEdgeBus(EditScope())
        let changes = UndoDiff.changes(from: start, to: state)
        guard !changes.isEmpty else { return }
        record(UndoStep(name: recordingName ?? "Edit", coalescing: outerCoalescing ?? recordingCoalescing,
                        changes: changes))
    }

    /// Puts a recorded step on the undo manager, or folds it into the step
    /// on top while typing with the same coalescing key.
    private func record(_ step: UndoStep) {
        if let key = step.coalescing, let top = openStep, top.coalescing == key {
            // The registered handler reads `top.changes` when it runs, so
            // nothing new is registered.
            for (item, change) in step.changes {
                if let older = top.changes[item] {
                    top.changes[item] = UndoChange(before: older.before, after: change.after)
                } else {
                    top.changes[item] = change
                }
            }
            top.changes = top.changes.filter { $0.value.before != $0.value.after }
            if top.changes.isEmpty {
                // Typed back to where it started: the step goes, as its
                // handler registers nothing for a step that changes nothing.
                openStep = nil
                undoManager.dropTop()
                undoRevision &+= 1
            }
            return
        }
        openStep = step.coalescing == nil ? nil : step
        register(step, backwards: true)
    }

    /// One undo group holding one action: apply the step's diff `backwards`
    /// (undo) or forwards (redo), then register the other direction. A step
    /// that changes nothing any more (its items are gone) registers nothing,
    /// so it leaves the stacks.
    private func register(_ step: UndoStep, backwards: Bool) {
        undoManager.beginUndoGrouping()
        undoManager.registerUndo(withTarget: self) { document in
            document.openStep = nil
            var s = document.state
            guard UndoDiff.apply(step.changes, backwards: backwards, to: &s) else { return }
            document.setState(s)
            document.undoManager.applied = true
            document.register(step, backwards: !backwards)
        }
        undoManager.setActionName(step.name)
        undoManager.endUndoGrouping()
        undoRevision &+= 1
    }

    private func clearUndoManager() {
        undoManager.removeAllActions()
        openStep = nil
        undoRevision &+= 1
    }

    /// Undoes the document's last step, whichever page made it. A step that
    /// changes nothing any more is dropped and the next one undone. False
    /// when there is nothing to undo, or while playing.
    @discardableResult
    func undo() -> Bool {
        if isGestureOpen { endGesture() }
        openStep = nil
        guard !isPlaying, undoManager.canUndo else { return false }
        undoManager.undo()
        undoRevision &+= 1
        return undoManager.applied
    }

    /// Redoes the last undone step, as `undo()`.
    @discardableResult
    func redo() -> Bool {
        if isGestureOpen { endGesture() }
        openStep = nil
        guard !isPlaying, undoManager.canRedo else { return false }
        undoManager.redo()
        undoRevision &+= 1
        return undoManager.applied
    }

    /// The name of the step `undo()` would undo ("Move"), or nil.
    var undoName: String? {
        _ = undoRevision
        return undoManager.canUndo ? undoManager.undoActionName : nil
    }

    /// The name of the step `redo()` would redo, or nil.
    var redoName: String? {
        _ = undoRevision
        return undoManager.canRedo ? undoManager.redoActionName : nil
    }

    /// The document is about to be written: the next typed character starts
    /// a new step, so a save never splits a step in two states.
    func willSave() {
        openStep = nil
    }

    /// Starts recording a whole mouse drag on `page` as one step. A gesture
    /// still open (one SwiftUI ended without `onEnded`) is closed first.
    func beginGesture(_ page: EditorPage) {
        guard !isPlaying else { return }
        if isGestureOpen { endGesture() }
        isGestureOpen = true
        if recordingDepth == 0 { startRecording(coalescing: nil) }
        recordingDepth += 1
    }

    /// Ends the drag `beginGesture` started and records what it changed.
    func endGesture() {
        guard isGestureOpen else { return }
        isGestureOpen = false
        // The pours were not refilled while the drag moved what they read.
        if !pours.isEmpty { ownCopperRevision &+= 1 }
        recordingDepth -= 1
        if recordingDepth == 0 { finishRecording() }
    }

    /// The next edit starts a new step even with the same coalescing key
    /// (a note editor lost focus).
    func endCoalescing() {
        openStep = nil
    }

    // MARK: Chips

    /// Places a block with its centre under `point`, snapped to the grid.
    /// The new part is not selected, so the parts library stays open for
    /// the next drop.
    /// `color`, `footprintID` and `ohms` are the part's options (LED, parts
    /// with footprint choices, pull resistors); nil (or a value the part
    /// doesn't have) is the part's default.
    func placeChip(part: CatalogPart, symbol: PartSymbol, centeredAt point: CGPoint,
                   color: String? = nil, footprintID: String? = nil, gender: String? = nil,
                   ohms: Double? = nil, farads: Double? = nil) {
        recordEdit(.schematic, "Add Part") {
            let layout = ChipLayout(part: part, symbol: symbol)
            let origin = SchematicGrid.snap(
                CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
            )
            let reference = takeReference(for: part, gender: gender)
            var placed = PlacedChip(
                reference: reference,
                part: part,
                symbol: symbol,
                origin: origin,
                color: color,
                footprintID: footprintID,
                gender: gender,
                ohms: ohms,
                farads: farads
            )
            placed.board = placed.footprint.map {
                BoardLayout.nextSpot(for: $0, among: chips, pad1: placed.part.firstPinDesignator,
                                     hasPinOneMark: placed.part.hasPinOneMark, grid: self.board.grid)
            }
            chips.append(placed)
            adoptLooseEnds(chips: [placed.id])
        }
    }

    /// The reference for a new part, taken, with its prefix from Settings ▸
    /// Reference Defaults (`ReferencePrefixes`). A part with the Integrated Circuit
    /// prefix gets the first number from `nextReference` on that no part
    /// uses (a part may have been renamed to one), which moves
    /// `nextReference` past it; any other part the lowest number no part
    /// uses. `alsoUsed` are references taken but not yet in `chips` (a
    /// paste in progress); `gender` is a header's.
    private func takeReference(for part: CatalogPart, gender: String?,
                               alsoUsed: [String] = []) -> String {
        let used = Set(chips.map(\.reference) + alsoUsed)
        let prefix = ReferencePrefixes.prefix(for: part, gender: gender)
        let counted = prefix == ReferencePrefixes.prefix(for: .integratedCircuit)
        var number = counted ? nextReference : 1
        while used.contains(ReferenceParts.reference(prefix: prefix, number: number)) {
            number += 1
        }
        if counted { nextReference = number + 1 }
        return ReferenceParts.reference(prefix: prefix, number: number)
    }

    /// Sets an LED's color. Does nothing for a part that isn't an LED, or a
    /// color the part doesn't have.
    func setPartColor(_ id: UUID, _ color: String) {
        recordEdit(.schematic, "Change Color") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_LED,
                  chips[index].part.colors.contains(color) else { return }
            chips[index].color = color
        }
    }

    /// Turns the reference above a part on the Schematic on or off.
    func setPartShowsReference(_ id: UUID, _ shows: Bool) {
        recordEdit(.schematic, shows ? "Show Reference" : "Hide Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].showsReference != shows else { return }
            chips[index].showsReference = shows
        }
    }

    /// Sets a header's gender. Does nothing for a part that isn't a header,
    /// or a gender the part doesn't list.
    func setPartGender(_ id: UUID, _ gender: String) {
        recordEdit(.schematic, "Change Gender") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_HEADER,
                  chips[index].part.genders.contains(gender) else { return }
            chips[index].gender = gender
        }
    }

    /// Changes a header's size by swapping its part for `newPart` (the
    /// caller looks up `HeaderFamily.name(rows:perRow:)` in its catalog).
    /// Reference, origin, flip, gender, footprint type and board placement
    /// are kept, and on the PCB pad 1 keeps its board point. Wires on pins
    /// the new size doesn't have stay, with loose ends at those pins' old
    /// tips; the rest are re-fitted. Does
    /// nothing unless both are headers and `newPart` is another size.
    func setHeaderSize(_ id: UUID, to newPart: CatalogPart) {
        edit(.schematic, "Change Size") { scope in
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_HEADER, newPart.kind == PART_HEADER,
                  chips[index].part != newPart else { return }
            swapPart(of: id, to: newPart, scope)
        }
    }

    /// Puts chip `id` on `newPart` (a header's new size, or a library part
    /// updated in place): wires on pins the new part lacks stay with loose
    /// ends at those pins' old tips, the chip keeps its reference, origin,
    /// flip, options and board placement, on the PCB pad 1 keeps its board
    /// point, and the rest of the wires are re-fitted. Not recorded: callers
    /// wrap it in `recordEdit` or clear the history.
    private func swapPart(of id: UUID, to newPart: CatalogPart, symbol newSymbol: PartSymbol? = nil, _ scope: EditScope) {
        guard let index = chips.firstIndex(where: { $0.id == id }) else { return }
        let old = chips[index]
        let pad1 = old.part.firstPinDesignator
        let anchor = old.board.flatMap { board in
            old.footprint?.pads.first { $0.designator == pad1 }.map { board.board($0.centre) }
        }
        let pinCount = newPart.pins.count
        loosen(scope) { end in
            if case let .pin(ref) = end { return ref.chipID == id && ref.index >= pinCount }
            return false
        }
        chips[index] = old.resized(to: newPart, symbol: newSymbol)
        // The reference was placed for the old body.
        chips[index].board?.referenceOffset = .zero
        if let anchor, var board = chips[index].board,
           let pad = chips[index].footprint?.pads.first(where: { $0.designator == pad1 }) {
            let landed = board.board(pad.centre)
            board.centre.x += anchor.x - landed.x
            board.centre.y += anchor.y - landed.y
            chips[index].board = board
        }
        settleWires(attachedTo: id)
    }

    /// Every chip of the custom part `uuid` becomes `newPart` (plan-53-D: a
    /// User Library part edited in place). Wires on pins the new version lacks
    /// become loose ends. Not undoable, and the undo history ends here: its
    /// older steps hold chips of the old version, while the simulation and the
    /// voltage check find a chip's part by name, which now is the new one.
    /// Returns whether any chip was on the part.
    @discardableResult
    func updatePart(uuid: String, to newPart: CatalogPart) -> Bool {
        let ids = chips.filter { $0.part.uuid == uuid }.map(\.id)
        guard !ids.isEmpty else { return false }
        let newSymbol = symbol(for: newPart)
        // Not undoable: the history ends here (below), so no step is recorded.
        let scope = EditScope()
        for id in ids { swapPart(of: id, to: newPart, symbol: newSymbol, scope) }
        clearUndoManager()
        return true
    }

    /// Sets the resistance of a pull resistor or `GENERIC-RESISTOR`
    /// (`hasOhmsField`). Does nothing for any other part, or a value that
    /// isn't `ResistorValue.isValid`.
    func setPartOhms(_ id: UUID, _ ohms: Double) {
        recordEdit(.schematic, "Change Resistance") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.hasOhmsField,
                  ResistorValue.isValid(ohms) else { return }
            chips[index].ohms = ohms
        }
    }

    /// Sets the generic capacitor's capacitance (plan-33-H). Does nothing for
    /// any other part, or a value that isn't `CapacitorValue.isValid`.
    func setPartFarads(_ id: UUID, _ farads: Double) {
        recordEdit(.schematic, "Change Capacitance") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.isGenericCapacitor,
                  CapacitorValue.isValid(farads) else { return }
            chips[index].farads = farads
        }
    }

    /// Picks one of a part's footprint choices. On the PCB the pad of the
    /// part's first pin keeps its board point (no re-snap, like
    /// `setFootprintStyle`); rotation, origin, marks and style are kept. Does
    /// nothing for an id that isn't one of the part's choices, or the current one.
    func setPartFootprint(_ id: UUID, _ footprintID: String) {
        recordEdit(.schematic, "Change Footprint") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].footprintID != footprintID,
                  chips[index].part.footprintChoices.contains(where: { $0.footprint.id == footprintID })
            else { return }
            changingFootprint(of: index) { chips[index].footprintID = footprintID }
        }
    }

    /// Runs `change`, which swaps the footprint of the chip at `index`, and
    /// keeps the pad of the part's first pin at its board point: the
    /// placement moves by the difference between the old and new pad 1, the
    /// reference text goes back to its default offset. The chip's rotation,
    /// origin, marks and style are kept (`setPartFootprint`, `setPartSocket`).
    private func changingFootprint(of index: Int, _ change: () -> Void) {
        let before = chips[index].footprint
        change()
        guard var changed = chips[index].board, let before, let after = chips[index].footprint else { return }
        // The reference was placed for the old body.
        changed.referenceOffset = .zero
        let pad1 = chips[index].part.firstPinDesignator
        if let old = before.pads.first(where: { $0.designator == pad1 }),
           let new = after.pads.first(where: { $0.designator == pad1 }) {
            let anchor = changed.board(old.centre)
            let landed = changed.board(new.centre)
            changed.centre.x += anchor.x - landed.x
            changed.centre.y += anchor.y - landed.y
        }
        chips[index].board = changed
    }

    /// Puts a chip in or out of a socket (plan-51-H). `mode` nil is a plain IC;
    /// otherwise `socket` is the socket part (a chip already in a socket mode
    /// keeps its own when `socket` is nil). Does nothing for a socket that
    /// does not hold the chip's footprint, a chip a socket cannot take, or
    /// when nothing changes. Going between IC and a socket mode puts the
    /// socket's footprint (or the chip's back) on the PCB with the first pin's
    /// pad where it was; between the two socket modes only the mode changes,
    /// and between two sockets that share a footprint nothing moves.
    func setPartSocket(_ id: UUID, mode: ChipSocket.Mode?, socket: CatalogPart?) {
        recordEdit(.schematic, "Change Socket") {
            guard let index = chips.firstIndex(where: { $0.id == id }) else { return }
            var new: ChipSocket?
            if let mode {
                guard let part = socket ?? chips[index].socket?.part,
                      ChipSocket.fits(part, chips[index].part) else { return }
                new = ChipSocket(part: part, mode: mode)
            }
            guard chips[index].socket != new else { return }
            if chips[index].socket?.part.footprint?.id == new?.part.footprint?.id {
                chips[index].socket = new
            } else {
                changingFootprint(of: index) { chips[index].socket = new }
            }
        }
    }

    /// Renames a part to `prefix` (uppercased) plus `number`. Returns why it
    /// can't (a bad prefix or number, or a reference another part has), or
    /// nil when it was renamed. `page` is the page the rename was made on;
    /// the step is on both pages' lists either way.
    @discardableResult
    func renameChip(_ id: UUID, prefix: String, number: Int?, page: EditorPage) -> String? {
        return recordEdit(page, "Rename") { () -> String? in
            if let message = ReferenceParts.validate(prefix: prefix, number: number) {
                return message
            }
            guard let number, let index = chips.firstIndex(where: { $0.id == id }) else { return nil }
            let reference = ReferenceParts.reference(prefix: prefix, number: number)
            if chips.contains(where: { $0.id != id && $0.reference == reference }) {
                return "\(reference) is already used by another part."
            }
            chips[index].reference = reference
            return nil
        }
    }

    /// Edit › Reset References: every part gets its prefix from Settings ▸
    /// Reference Defaults back (`ReferencePrefixes`) and is numbered again from 1
    /// within its prefix, in reading order on the Schematic (top to bottom,
    /// then left to right), as one step. The next new part continues the
    /// Integrated Circuit prefix's numbers.
    /// Returns whether any reference changed.
    @discardableResult
    func resetReferences(page: EditorPage) -> Bool {
        let order = chips.indices.sorted { a, b in
            let ca = chips[a], cb = chips[b]
            if ca.origin.y != cb.origin.y { return ca.origin.y < cb.origin.y }
            if ca.origin.x != cb.origin.x { return ca.origin.x < cb.origin.x }
            return ca.reference.localizedStandardCompare(cb.reference) == .orderedAscending
        }
        var counts: [String: Int] = [:]
        var renamed = chips
        for index in order {
            let prefix = ReferencePrefixes.prefix(for: chips[index].part, gender: chips[index].gender)
            let number = (counts[prefix] ?? 0) + 1
            counts[prefix] = number
            renamed[index].reference = ReferenceParts.reference(prefix: prefix, number: number)
        }
        let next = (counts[ReferencePrefixes.prefix(for: .integratedCircuit)] ?? 0) + 1
        guard renamed != chips || next != nextReference else { return false }
        recordEdit(page, "Reset References") {
            chips = renamed
            nextReference = next
        }
        return true
    }

    /// Swaps the side of every pin on a placed part, and re-fits the wires
    /// attached to it.
    func flipChip(_ id: UUID) {
        recordEdit(.schematic, "Flip") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].symbol.canFlip, !chips[index].isSplit else { return }
            chips[index].isFlipped.toggle()
            settleWires(attachedTo: id)
            adoptLooseEnds(chips: [id])
        }
    }

    /// Turns a part that can rotate (`PartSymbol.canRotate`) a quarter turn,
    /// clockwise or not, and re-fits the wires attached to it.
    func rotateChip(_ id: UUID, counterclockwise: Bool = false) {
        recordEdit(.schematic, "Rotate") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].symbol.canRotate, !chips[index].isSplit else { return }
            chips[index].rotation = PlacedChip.normalized(chips[index].rotation + (counterclockwise ? 270 : 90))
            settleWires(attachedTo: id)
            adoptLooseEnds(chips: [id])
        }
    }

    // MARK: Units (plan-67)

    /// The sidebar's View option: draws a part as its units (a gate IC as
    /// separate gates) or as one block again. The wires stay on their pins.
    /// Nothing for a part with no units, or one already drawn that way.
    func setPartSplit(_ id: UUID, _ split: Bool) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              !chips[index].part.units.isEmpty, chips[index].isSplit != split else { return }
        recordEdit(.schematic, "Change View") {
            if split { chips[index].split() } else { chips[index].join() }
            settleWires(attachedTo: id)
            adoptLooseEnds(chips: [id])
        }
    }

    /// Whether `ref` names a unit of a part drawn as its units.
    private func placementIndex(_ ref: UnitRef) -> Int? {
        guard let index = chips.firstIndex(where: { $0.id == ref.chipID }),
              chips[index].unitPlacements.contains(where: { $0.key == ref.key }) else { return nil }
        return index
    }

    /// Moves one unit's body corner to `origin` (snapped to the grid).
    func moveUnit(_ ref: UnitRef, to origin: CGPoint) {
        guard let index = placementIndex(ref) else { return }
        recordEdit(.schematic, "Move") {
            chips[index].updatePlacement(ref.key) { $0.origin = SchematicGrid.snap(origin) }
            settleWires(attachedTo: ref.chipID)
            adoptLooseEnds(chips: [ref.chipID])
        }
    }

    /// Mirrors one unit left to right.
    func flipUnit(_ ref: UnitRef) {
        guard let index = placementIndex(ref) else { return }
        recordEdit(.schematic, "Flip") {
            chips[index].updatePlacement(ref.key) { $0.isFlipped.toggle() }
            settleWires(attachedTo: ref.chipID)
            adoptLooseEnds(chips: [ref.chipID])
        }
    }

    /// Turns one unit a quarter turn. Nothing for a unit drawn as a block.
    func rotateUnit(_ ref: UnitRef, counterclockwise: Bool = false) {
        guard let index = placementIndex(ref),
              chips[index].piece(forUnit: ref.key)?.symbol.canRotate == true else { return }
        recordEdit(.schematic, "Rotate") {
            chips[index].updatePlacement(ref.key) {
                $0.rotation = PlacedChip.normalized($0.rotation + (counterclockwise ? 270 : 90))
            }
            settleWires(attachedTo: ref.chipID)
            adoptLooseEnds(chips: [ref.chipID])
        }
    }

    /// What Rotate or Flip acts on: a part, or one unit of a part drawn as
    /// its units.
    enum EditTarget: Equatable {
        case chip(UUID)
        case unit(UnitRef)
    }

    /// What Rotate would turn on the Schematic: the sole selected part when
    /// its symbol turns, or, of a part drawn as its units, the one picked
    /// unit when that turns (a gate; not the block of leftover pins).
    var rotateTarget: EditTarget? { editTarget { $0.symbol.canRotate } }

    /// What Flip would mirror: as `rotateTarget`, for symbols that flip.
    var flipTarget: EditTarget? { editTarget { $0.symbol.canFlip } }

    private func editTarget(_ can: (PlacedChip) -> Bool) -> EditTarget? {
        guard let id = selectedChipID, let chip = chips.first(where: { $0.id == id }) else { return nil }
        guard chip.isSplit else { return can(chip) ? .chip(id) : nil }
        let keys = picked(chip)
        guard keys.count == 1, let key = keys.first, let piece = chip.piece(forUnit: key), can(piece) else { return nil }
        return .unit(UnitRef(chipID: id, key: key))
    }

    /// Which units of selected parts drawn as their units are picked: the
    /// ones a drag, Flip or Rotate acts on, drawn fully marked (their
    /// siblings faintly). A part with no entry has every unit picked. Editor
    /// state like the selection: never saved, not part of undo.
    private(set) var pickedUnits: [UUID: Set<String>] = [:]

    /// The picked units of a part: every unit unless some were picked.
    func picked(_ chip: PlacedChip) -> Set<String> {
        let all = Set(chip.unitPlacements.map(\.key))
        guard isSelected(.chip(chip.id)), let some = pickedUnits[chip.id]?.intersection(all), !some.isEmpty else { return all }
        return some
    }

    /// A click on a unit: it alone is picked; with `extend` (Shift) it is
    /// added to, or taken out of, the picked ones.
    func pick(_ ref: UnitRef, extend: Bool) {
        guard extend, var some = pickedUnits[ref.chipID] else {
            pickedUnits[ref.chipID] = [ref.key]
            return
        }
        if some.contains(ref.key) { some.remove(ref.key) } else { some.insert(ref.key) }
        pickedUnits[ref.chipID] = some.isEmpty ? nil : some
    }

    /// Whether a drawn thing is shown selected: a selected part, or a picked
    /// unit of one (`piece` is a part or a piece of one).
    func isPicked(_ piece: PlacedChip) -> Bool {
        guard isSelected(.chip(piece.id)) else { return false }
        guard let key = piece.unitKey, let some = pickedUnits[piece.id], !some.isEmpty else { return true }
        return some.contains(key)
    }

    /// A unit of a selected part that is not itself picked: shown faintly,
    /// so it reads as part of the same IC.
    func isSibling(_ piece: PlacedChip) -> Bool {
        piece.unitKey != nil && isSelected(.chip(piece.id)) && !isPicked(piece)
    }

    /// The unit a click will leave as the only picked one, when the press
    /// was on one of several picked things (see `pressPiece`).
    private var pendingPick: UnitRef?

    /// A Select press on what is drawn at a point (`piece(at:)`): a part, as
    /// `pressSelection`; or a unit of a part drawn as its units, which picks
    /// that unit.
    ///
    /// - No Shift: the unit becomes the only picked one, unless it is one of
    ///   several picked things: then the drag moves them all and a click
    ///   keeps only it.
    /// - Shift: the unit joins the picked ones; a click on a picked unit
    ///   takes it out again, and the part with it when it was the last.
    func pressPiece(_ piece: PlacedChip, shift: Bool) -> SelectionPress<SchematicItem> {
        guard let key = piece.unitKey, let chip = chips.first(where: { $0.id == piece.id }) else {
            return pressSelection(.chip(piece.id), shift: shift)
        }
        let item = SchematicItem.chip(piece.id)
        let wasSelected = isSelected(item)
        let before = picked(chip)
        pendingPick = nil
        if shift {
            if !wasSelected {
                let press = schematicSelection.press(item, shift: true)
                pickedUnits[piece.id] = [key]
                return press
            }
            if before == [key] {
                // The last picked unit: a click takes the part out of the selection.
                return schematicSelection.press(item, shift: true)
            }
            var some = before
            if some.contains(key) { some.remove(key) } else { some.insert(key) }
            pickedUnits[piece.id] = some
            return SelectionPress(dragsGroup: true, onClick: .none)
        }
        if wasSelected, before.contains(key), before.count > 1 || schematicSelection.count > 1 {
            pendingPick = UnitRef(chipID: piece.id, key: key)
            let press = schematicSelection.press(item, shift: false)
            return SelectionPress(dragsGroup: true, onClick: press.onClick)
        }
        let press = schematicSelection.press(item, shift: false)
        pickedUnits[piece.id] = [key]
        return press
    }

    /// Picks exactly these units of their parts (a box select).
    func pickUnits(_ refs: Set<UnitRef>) {
        for chipID in Set(refs.map(\.chipID)) {
            pickedUnits[chipID] = Set(refs.filter { $0.chipID == chipID }.map(\.key))
        }
    }

    /// Forgets the picks of parts that are no longer selected, so a part
    /// selected again starts with every unit picked.
    private func prunePicks() {
        guard !pickedUnits.isEmpty else { return }
        let selected = schematicSelectedParts
        let kept = pickedUnits.filter { selected.contains($0.key) }
        if kept.count != pickedUnits.count { pickedUnits = kept }
    }

    func chip(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.piece(at: point) != nil }
    }

    /// What is drawn under a point: a part, or the piece of a part drawn as
    /// its units (plan-67; `PlacedChip.unitKey` says which).
    func piece(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.piece(at: point) != nil }?.piece(at: point)
    }

    /// The junction within `tolerance` of `point`, if any.
    func junctionHit(at point: CGPoint, tolerance: CGFloat = 6) -> Junction? {
        junctions.last { Self.distance(from: point, to: $0.point) <= tolerance }
    }

    // MARK: Group drags (Schematic)

    /// Where every selected part, junction and wire is now, for a group drag.
    func schematicGroupBase() -> SchematicGroupBase {
        var base = SchematicGroupBase()
        for item in schematicSelection.items {
            switch item {
            case let .chip(id):
                guard let chip = chips.first(where: { $0.id == id }) else { break }
                if chip.isSplit {
                    // Only the picked units move (plan-67).
                    let keys = picked(chip)
                    for placement in chip.unitPlacements where keys.contains(placement.key) {
                        base.units[UnitRef(chipID: id, key: placement.key)] = placement.origin
                    }
                } else {
                    base.chips[id] = chip.origin
                }
            case let .junction(id):
                if let junction = junctions.first(where: { $0.id == id }) { base.junctions[id] = junction.point }
            case let .wire(id):
                if let wire = wires.first(where: { $0.id == id }) { base.wires[id] = wire.interior }
            case let .note(id):
                if let note = notes.first(where: { $0.id == id }) { base.notes[id] = note.origin }
            case let .label(id):
                if let label = netLabels.first(where: { $0.id == id }) { base.labels[id] = label.origin }
            case .titleBlock:
                base.titleBlock = documentInfo.titleBlock
            case let .bus(id):
                if let bus = bus(id) {
                    base.buses[id] = bus
                    base.ports.formUnion(bus.ports.map(\.id))
                }
            }
        }
        for wire in wires where base.wires[wire.id] != nil || moves(wire.start, in: base) || moves(wire.end, in: base) {
            base.affectedWires.insert(wire.id)
            let route = path(for: wire)
            if OrthogonalRoute.isOrthogonal(route) {
                base.orthogonalWires.insert(wire.id)
                if base.wires[wire.id] == nil { base.paths[wire.id] = route }
            }
        }
        return base
    }

    /// Whether a group drag moves this wire end: a pin of a moved part, a
    /// moved junction or a moved net label.
    private func moves(_ end: WireEnd, in base: SchematicGroupBase) -> Bool {
        switch end {
        case let .pin(ref):
            if base.chips[ref.chipID] != nil { return true }
            guard !base.units.isEmpty, let chip = chips.first(where: { $0.id == ref.chipID }),
                  let key = chip.unitKey(ofPin: ref.index) else { return false }
            return base.units[UnitRef(chipID: ref.chipID, key: key)] != nil
        case let .junction(id): return base.junctions[id] != nil
        case let .busPin(ref): return base.ports.contains(ref.portID)
        case let .label(id): return base.labels[id] != nil
        }
    }

    /// `bus` moved by (`dx`, `dy`): its lines, and its ports along with
    /// them (a port's start is along its line).
    static func shifted(_ bus: Bus, dx: CGFloat, dy: CGFloat) -> Bus {
        var moved = bus
        var horizontal: [UUID: Bool] = [:]
        for index in moved.lines.indices {
            horizontal[moved.lines[index].id] = moved.lines[index].isHorizontal
            moved.lines[index].a.x += dx
            moved.lines[index].a.y += dy
            moved.lines[index].b.x += dx
            moved.lines[index].b.y += dy
        }
        for index in moved.ports.indices {
            moved.ports[index].start += horizontal[moved.ports[index].line] == true ? dx : dy
        }
        return moved
    }

    /// One frame of a group drag: every part, junction, selected wire bend
    /// and bus in `base` moves by `delta` snapped to the grid, from where it
    /// was at the press. Selected wires keep their ends: only their bends
    /// move. A frame that would make a moved bus touch another bus changes
    /// nothing, so the group stays at the last place it could go.
    func moveSchematicGroup(_ base: SchematicGroupBase, by delta: CGSize) {
        edit(.schematic, "Move") { scope in
            let dx = SchematicGrid.snap(delta.width), dy = SchematicGrid.snap(delta.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            if !base.buses.isEmpty {
                let moved = buses.map { bus in base.buses[bus.id].map { Self.shifted($0, dx: dx, dy: dy) } ?? bus }
                guard Self.busesAreApart(moved) else { return }
                sheet.updateBuses(scope) { $0.buses = moved }
            }
            for index in chips.indices {
                guard let origin = base.chips[chips[index].id] else { continue }
                let moved = shifted(origin)
                if chips[index].origin != moved { chips[index].origin = moved }
            }
            for (ref, origin) in base.units {
                guard let index = chips.firstIndex(where: { $0.id == ref.chipID }) else { continue }
                let moved = shifted(origin)
                if chips[index].unitPlacements.first(where: { $0.key == ref.key })?.origin != moved {
                    chips[index].updatePlacement(ref.key) { $0.origin = moved }
                }
            }
            sheet.updateNotes(scope) { $0.move(base.notes, by: shifted) }
            sheet.updateLabels(scope) { $0.move(base.labels, by: shifted) }
            if let origin = base.titleBlock {
                let moved = shifted(origin)
                if documentInfo.titleBlock != moved { documentInfo.titleBlock = moved }
            }
            let tips = tips
            sheet.updateWiring(scope) { wiring in
                wiring.moveGroup(junctions: base.junctions, bends: base.wires, by: shifted)
                wiring.refit(base.paths, tips)
            }
        }
    }

    /// The end of a group drag: re-fits each affected wire once, as
    /// `settleWires(attachedTo:)` does. A wire that was all right angles at
    /// the press gets corners wherever the move left a slanted segment.
    func settleSchematicGroup(_ base: SchematicGroupBase) {
        edit(.schematic, "Move") { scope in
            let tips = tips
            sheet.updateWiring(scope) { $0.settleGroup(affected: base.affectedWires, orthogonal: base.orthogonalWires, tips) }
            adoptLooseEnds(chips: Set(base.chips.keys).union(base.units.keys.map(\.chipID)), ports: base.ports,
                           junctions: Set(base.junctions.keys), labels: Set(base.labels.keys))
        }
    }

    // MARK: Copy and paste (Schematic)

    /// Adds a copy (`SchematicCopy`) moved by `offset` (snapped to the
    /// grid), as one step, and makes it the selection. Every item gets a new
    /// id; parts get new references (as a placed part does) and a footprint
    /// spot to the right of the board's parts. Net labels keep their names,
    /// so they join the nets of those names. A bus keeps its name when no
    /// bus has it, or when the bus that has it carries the same wires and
    /// type (so the two link, as labels join); else it takes the next free
    /// "BUS n". Loose ends that land on a pin connect. Refused, with nothing
    /// changed, when a pasted bus would touch another bus, or when it would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Everything pasted is selected, wire ends (junctions) included, so the
    /// whole paste drags as one; a loose end that joins a pin is no longer a
    /// junction and drops out of the selection. Returns whether anything was
    /// pasted.
    @discardableResult
    func pasteSchematic(_ copy: SchematicCopy, offset: CGSize) -> Bool {
        guard !copy.isEmpty else { return false }
        return edit(.schematic, "Paste") { scope -> Bool in
            let dx = SchematicGrid.snap(offset.width), dy = SchematicGrid.snap(offset.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            var newID: [UUID: UUID] = [:]
            func renamed(_ id: UUID) -> UUID {
                if let id = newID[id] { return id }
                let fresh = UUID()
                newID[id] = fresh
                return fresh
            }

            let savedNextReference = nextReference
            var pastedChips: [PlacedChip] = []
            for chip in copy.chips {
                var placed = PlacedChip(
                    id: renamed(chip.id), reference: takeReference(for: chip.part, gender: chip.gender,
                                                                   alsoUsed: pastedChips.map(\.reference)),
                    part: chip.part,
                    symbol: chip.symbol, origin: shifted(chip.origin), isFlipped: chip.isFlipped,
                    rotation: chip.rotation, switchesOn: chip.switchesOn, memoryFile: chip.memoryFile, color: chip.color,
                    footprintID: chip.footprintID, gender: chip.gender, ohms: chip.ohms, farads: chip.farads,
                    socket: chip.socket
                )
                placed.showsReference = chip.showsReference
                if chip.isSplit {
                    placed.setPlacements(chip.unitPlacements.map {
                        UnitPlacement(key: $0.key, origin: shifted($0.origin), isFlipped: $0.isFlipped, rotation: $0.rotation)
                    })
                }
                placed.board = placed.footprint.map {
                    BoardLayout.nextSpot(for: $0, among: chips + pastedChips, pad1: placed.part.firstPinDesignator,
                                         hasPinOneMark: placed.part.hasPinOneMark, grid: self.board.grid)
                }
                pastedChips.append(placed)
            }
            let pastedJunctions = copy.junctions.map {
                Junction(id: renamed($0.id), point: shifted($0.point), isLoose: $0.isLoose)
            }
            let pastedLabels = copy.labels.map {
                NetLabel(id: renamed($0.id), name: $0.name, style: $0.style, origin: shifted($0.origin),
                         isFlipped: $0.isFlipped)
            }
            let pastedNotes = copy.notes.map {
                SchematicNote(id: renamed($0.id), text: $0.text, origin: shifted($0.origin), size: $0.size,
                              framed: $0.framed)
            }
            var pastedBuses: [Bus] = []
            for bus in copy.buses {
                let lines = bus.lines.map {
                    BusLine(id: renamed($0.id), a: shifted($0.a), b: shifted($0.b))
                }
                let busWires = bus.wires.map { BusWire(id: renamed($0.id), name: $0.name) }
                var horizontal: [UUID: Bool] = [:]
                for line in bus.lines { horizontal[line.id] = line.isHorizontal }
                let ports = bus.ports.map { port in
                    BusPort(id: renamed(port.id), line: renamed(port.line),
                            start: port.start + (horizontal[port.line] == true ? dx : dy), side: port.side,
                            wireIDs: Set(port.wireIDs.map(renamed)))
                }
                let lowered = bus.name.lowercased()
                let others = buses + pastedBuses
                let named = others.filter { $0.name.lowercased() == lowered }
                let links = named.allSatisfy { $0.wires.map(\.name) == bus.wires.map(\.name) && $0.card == bus.card }
                let name = !bus.name.isEmpty && links ? (named.first?.name ?? bus.name)
                    : Bus.defaultBusName(taken: others.map(\.name))
                pastedBuses.append(Bus(id: renamed(bus.id), name: name, card: bus.card, lines: lines,
                                       wires: busWires, ports: ports))
            }
            guard Self.busesAreApart(buses + pastedBuses) else {
                nextReference = savedNextReference
                editorMessage = "Can't paste there: a pasted bus would touch another bus."
                return false
            }
            func end(_ end: WireEnd) -> WireEnd {
                switch end {
                case let .pin(ref): .pin(PinRef(chipID: renamed(ref.chipID), index: ref.index))
                case let .junction(id): .junction(renamed(id))
                case let .label(id): .label(renamed(id))
                // Only pins of copied buses (`schematicCopy` loosens the rest).
                case let .busPin(ref): .busPin(BusPinRef(portID: renamed(ref.portID), wireID: renamed(ref.wireID)))
                }
            }
            let pastedWires = copy.wires.map {
                Wire(id: renamed($0.id), start: end($0.start), end: end($0.end), interior: $0.interior.map(shifted))
            }

            // Labels join existing nets by name, which can put a voltage
            // out of range.
            if let library,
               let issue = VoltageCheck.firstNew(before: chips, wires, after: chips + pastedChips,
                                                 wires + pastedWires, buses: buses + pastedBuses, labels: netLabels,
                                                 afterLabels: netLabels + pastedLabels, library: library) {
                nextReference = savedNextReference
                editorMessage = VoltageCheck.message(issue, chips: chips + pastedChips, edit: .connect)
                return false
            }
            editorMessage = nil
            chips += pastedChips
            sheet.updateWiring(scope) { $0.junctions += pastedJunctions }
            sheet.updateLabels(scope) { $0.labels += pastedLabels }
            sheet.updateNotes(scope) { $0.notes += pastedNotes }
            sheet.updateBuses(scope) { $0.buses += pastedBuses }
            sheet.updateWiring(scope) { $0.wires += pastedWires }

            var selection = Set<SchematicItem>()
            selection.formUnion(pastedChips.map { .chip($0.id) })
            selection.formUnion(pastedJunctions.map { .junction($0.id) })
            selection.formUnion(pastedLabels.map { .label($0.id) })
            selection.formUnion(pastedNotes.map { .note($0.id) })
            selection.formUnion(pastedWires.map { .wire($0.id) })
            selection.formUnion(pastedBuses.map { .bus($0.id) })
            clearSchematicSelection()
            for item in selection { toggleSelection(item) }
            pruneJunctions(scope)
            adoptLooseEnds(chips: Set(pastedChips.map(\.id)),
                           ports: Set(pastedBuses.flatMap { $0.ports.map(\.id) }),
                           junctions: Set(pastedJunctions.filter(\.isLoose).map(\.id)),
                           labels: Set(pastedLabels.map(\.id)))
            return true
        }
    }

    // MARK: Net labels (Schematic)

    /// The lowest "N<n>" no label is named, for a new label.
    func nextNetLabelName() -> String {
        sheet.labels.nextName
    }

    /// Adds a net label with its point at `point` (snapped), named
    /// `nextNetLabelName()`, and selects it. A loose wire end at that point
    /// joins it.
    @discardableResult
    func addNetLabel(at point: CGPoint, style: NetLabel.Style = .input) -> UUID {
        return edit(.schematic, "Add Net Label") { scope -> UUID in
            let label = sheet.updateLabels(scope) { $0.add(at: point, style: style) }
            select(.label(label.id))
            adoptLooseEnds(labels: [label.id])
            return label.id
        }
    }

    /// Renames a label (`NetLabel.cleanName`). Refused, with the name
    /// kept, when nothing is left of `value`, or when joining the nets would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Returns whether the label now has the cleaned name. Typing one name
    /// is one undo step, until `endCoalescing()`.
    @discardableResult
    func setNetLabelName(_ id: UUID, _ value: String) -> Bool {
        edit(.schematic, "Rename Net Label", coalescing: "label-name:\(id)") { scope -> Bool in
            guard let name = NetLabel.cleanName(value),
                  let label = netLabels.first(where: { $0.id == id }) else { return false }
            guard label.name != name else { return true }
            var renamed = sheet.labels
            renamed.setName(id, name)
            // Joining two nets can put a voltage out of range on a part; a name
            // no other label has joins nothing (and leaving a net only
            // removes connections), so there is nothing to check.
            if sheet.labels.joins(id, named: name), refusesLabels(renamed.labels) { return false }
            sheet.updateLabels(scope) { $0 = renamed }
            return true
        }
    }

    /// Whether the labels `after` would put a new voltage out of range on a
    /// part; sets `editorMessage` when they would.
    private func refusesLabels(_ after: [NetLabel]) -> Bool {
        guard let library,
              let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, wires, buses: buses,
                                                labels: netLabels, afterLabels: after,
                                                known: currentVoltageIssues(library), library: library) else { return false }
        editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
        return true
    }

    /// Sets a label's type. Refused, with the type kept, when the change
    /// would join nets that put a voltage out of range on a part (a No
    /// Connect turning back into a named label); `editorMessage` says why.
    func setNetLabelStyle(_ id: UUID, _ style: NetLabel.Style) {
        edit(.schematic, "Change Net Label Type") { scope in
            guard let label = netLabels.first(where: { $0.id == id }), label.style != style else { return }
            var changed = sheet.labels
            changed.setStyle(id, style)
            // Only becoming or leaving No Connect changes what joins what.
            if label.isNoConnect != style.isNoConnect, refusesLabels(changed.labels) { return }
            sheet.updateLabels(scope) { $0 = changed }
        }
    }

    /// Turns a label to run the other way from its point. Its wires stay on
    /// the point.
    func flipNetLabel(_ id: UUID) {
        edit(.schematic, "Flip") { scope in
            sheet.updateLabels(scope) { $0.flip(id) }
            settle(scope) { $0 == .label(id) }
        }
    }

    /// Deletes a label; its wires stay, with loose ends at its point.
    func deleteNetLabel(_ id: UUID) {
        edit(.schematic, "Delete") { scope in
            loosen(scope) { $0 == .label(id) }
            sheet.updateLabels(scope) { $0.remove([id]) }
            pruneSelection()
        }
    }

    /// The label under a point: the last one drawn there.
    func netLabel(at point: CGPoint) -> NetLabel? {
        sheet.labels.label(at: point)
    }

    /// The label whose point is within `radius` of `point`.
    func netLabelTip(at point: CGPoint, radius: CGFloat) -> UUID? {
        sheet.labels.tip(at: point, radius: radius)
    }

    func netLabelTip(_ id: UUID) -> CGPoint? {
        sheet.labels.tip(id)
    }

    // MARK: Document info (plan-64)

    /// Runs `change` on the document info as one undo step named `name`.
    /// Nothing is recorded when it changes nothing. Typing in one field
    /// merges into one step through `coalescing`.
    func editDocumentInfo(_ name: String, coalescing: String? = nil, _ change: (inout DocumentInfo) -> Void) {
        recordEdit(.schematic, name, coalescing: coalescing) {
            var edited = documentInfo
            change(&edited)
            if edited != documentInfo { documentInfo = edited }
        }
    }

    func setInfoTitle(_ value: String) {
        editDocumentInfo("Edit Title", coalescing: "info-title") { $0.title = value }
    }

    func setInfoAuthor(_ value: String) {
        editDocumentInfo("Edit Author", coalescing: "info-author") { $0.author = value }
    }

    func setInfoRevision(_ value: String) {
        editDocumentInfo("Edit Revision", coalescing: "info-revision") { $0.revision = value }
    }

    func setInfoDescription(_ value: String) {
        editDocumentInfo("Edit Description", coalescing: "info-description") { $0.description = value }
    }

    func setInfoCreated(_ date: Date?) {
        editDocumentInfo("Edit Created Date") { $0.created = date }
    }

    func setInfoUpdated(_ date: Date?) {
        editDocumentInfo("Edit Updated Date") { $0.updated = date }
    }

    func setAutomaticDates(_ on: Bool) {
        editDocumentInfo("Change Automatic Dates") { $0.automaticDates = on }
    }

    /// What a write of the file does to the dates (`DocumentInfo.stamp`).
    /// Not an undo step, and it does not mark the document edited.
    func stampDates(now: Date = Date()) {
        var stamped = documentInfo
        stamped.stamp(now: now)
        if stamped != documentInfo { documentInfo = stamped }
    }

    // MARK: Title block (plan-64-C)

    /// The title block as drawn, nil when it is not on the schematic.
    var titleBlockLayout: TitleBlockLayout? {
        documentInfo.titleBlock.map { TitleBlockLayout(documentInfo, origin: $0) }
    }

    /// The title block's frame, nil when it is not on the schematic.
    var titleBlockRect: CGRect? { titleBlockLayout?.rect }

    /// Whether the title block is shown and covers `point`.
    func titleBlock(at point: CGPoint) -> Bool {
        titleBlockRect?.contains(point) ?? false
    }

    /// Shows or hides the title block (one undo step). Turned on, it goes at
    /// the bottom-right of the drawing: its right edge on the drawing's, a
    /// little below it; an empty schematic puts it at (0, 0). Turned off, the
    /// information stays. It is not selected when shown.
    func setTitleBlockShown(_ on: Bool) {
        guard on != (documentInfo.titleBlock != nil) else { return }
        if !on {
            editDocumentInfo("Hide Title Block") { $0.titleBlock = nil }
            pruneSelection()
            return
        }
        var origin = CGPoint.zero
        var painter = BoundsPainter()
        SchematicScene(self).paint(into: &painter)
        if let bounds = painter.bounds {
            origin = SchematicGrid.snap(CGPoint(x: bounds.maxX - TitleBlockLayout.width, y: bounds.maxY + 20))
        }
        editDocumentInfo("Show Title Block") { $0.titleBlock = origin }
    }

    // MARK: Text blocks (Schematic)

    /// Adds a text block with its top-left at `point` (snapped), selects it
    /// and asks the sidebar to edit its words.
    @discardableResult
    func addNote(at point: CGPoint) -> UUID {
        return edit(.schematic, "Add Text") { scope -> UUID in
            let note = sheet.updateNotes(scope) { $0.add(at: point) }
            select(.note(note.id))
            editingNoteID = note.id
            return note.id
        }
    }

    /// Changes a block's words. An empty string is refused and the words kept.
    func setNoteText(_ id: UUID, _ value: String) {
        edit(.schematic, "Edit Text", coalescing: "note-text:\(id)") { scope in
            sheet.updateNotes(scope) { $0.setText(id, value) }
        }
    }

    /// Sets a block's font size in points, clamped to `SchematicNote.sizeRange`.
    func setNoteSize(_ id: UUID, _ size: Int) {
        edit(.schematic, "Change Font Size") { scope in
            sheet.updateNotes(scope) { $0.setSize(id, size) }
        }
    }

    func setNoteFramed(_ id: UUID, _ framed: Bool) {
        edit(.schematic, "Change Frame") { scope in
            sheet.updateNotes(scope) { $0.setFramed(id, framed) }
        }
    }

    func deleteNote(_ id: UUID) {
        edit(.schematic, "Delete Text") { scope in
            sheet.updateNotes(scope) { $0.remove([id]) }
            if editingNoteID == id { editingNoteID = nil }
            pruneSelection()
        }
    }

    /// The text block under a point: the last one drawn there.
    func note(at point: CGPoint) -> SchematicNote? {
        sheet.sheetNotes.note(at: point)
    }

    // MARK: Footprints (PCB)

    /// Moves a footprint's centre (board mm), then snaps its origin point to
    /// the board grid.
    func moveFootprint(_ id: UUID, to centre: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            var moved = board
            moved.centre = centre
            chips[index].board = BoardLayout.snapped(moved, footprint: footprint,
                                                     pad1: chips[index].part.firstPinDesignator, grid: self.board.grid)
        }
    }

    /// Moves a footprint so its origin point lands on `point` (board mm),
    /// snapped to the grid.
    func moveFootprintOrigin(_ id: UUID, to point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let current = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
            var moved = board
            moved.centre.x += point.x - current.x
            moved.centre.y += point.y - current.y
            chips[index].board = BoardLayout.snapped(moved, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Turns a footprint 90° clockwise about its origin point, which stays
    /// where it was.
    func rotateFootprint(_ id: UUID, counterclockwise: Bool = false) {
        recordEdit(.pcb, "Rotate") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let before = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
            var turned = board
            turned.rotation = (board.rotation + (counterclockwise ? 270 : 90)) % 360
            let after = BoardLayout.originPoint(of: turned, footprint: footprint, pad1: pad1)
            turned.centre.x += before.x - after.x
            turned.centre.y += before.y - after.y
            chips[index].board = BoardLayout.snapped(turned, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Moves a footprint to the other side of the board, mirrored left to
    /// right as seen from above. Its origin point stays where it was, then
    /// snaps, as in `rotateFootprint`. Does nothing on a 1-layer board or when
    /// the side doesn't change.
    func setFootprintSide(_ id: UUID, _ side: BoardSide) {
        recordEdit(.pcb, "Change Side") {
            guard board.layers > 1,
                  let index = chips.firstIndex(where: { $0.id == id }),
                  let placement = chips[index].board, placement.side != side,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let before = BoardLayout.originPoint(of: placement, footprint: footprint, pad1: pad1)
            var flipped = placement
            flipped.side = side
            // A placement mirrors before it turns, so a board-wide left-right
            // mirror of a turned footprint is the mirror plus the opposite
            // turn. Without this, 90° and 270° parts come out mirrored top to
            // bottom and their pad rows swap.
            flipped.rotation = (360 - placement.rotation) % 360
            let after = BoardLayout.originPoint(of: flipped, footprint: footprint, pad1: pad1)
            flipped.centre.x += before.x - after.x
            flipped.centre.y += before.y - after.y
            chips[index].board = BoardLayout.snapped(flipped, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Changes which point of a footprint is its origin. The footprint stays
    /// where it is and is not re-snapped.
    func setFootprintOrigin(_ id: UUID, _ origin: FootprintOrigin) {
        recordEdit(.pcb, "Change Origin") {
            guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil else { return }
            chips[index].board?.origin = origin
        }
    }

    /// Switches a part between its footprint and its connector footprint
    /// (the PCB Style option). Pin 1's first pad stays exactly where it was
    /// (not re-snapped: like an origin change, this may leave the origin
    /// point off the grid); rotation, origin and marks are kept. Does nothing for a part without a connector
    /// footprint or when the style does not change.
    func setFootprintStyle(_ id: UUID, _ style: FootprintStyle) {
        recordEdit(.pcb, "Change Style") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.connectorFootprint != nil,
                  let board = chips[index].board, board.style != style,
                  let before = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            var changed = board
            changed.style = style
            // The reference was placed for the old body.
            changed.referenceOffset = .zero
            chips[index].board = changed
            guard let after = chips[index].footprint else { return }
            if let old = before.pads.first(where: { $0.designator == pad1 }),
               let new = after.pads.first(where: { $0.designator == pad1 }) {
                let anchor = board.board(old.centre)
                let landed = changed.board(new.centre)
                changed.centre.x += anchor.x - landed.x
                changed.centre.y += anchor.y - landed.y
            }
            chips[index].board = changed
        }
    }

    /// Sets where a footprint's marks are drawn. A nil layer is left as it is.
    func setFootprintMarks(_ id: UUID, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        setFootprintMarks([id], outline: outline, pin1: pin1, reference: reference)
    }

    /// Sets where the marks of every footprint in `ids` are drawn, as one
    /// undo step. A nil layer is left as it is; a part with no footprint is
    /// skipped.
    func setFootprintMarks(_ ids: some Collection<UUID>, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        recordEdit(.pcb, "Change Marks") {
            for index in chips.indices where ids.contains(chips[index].id) && chips[index].board != nil {
                if let outline { chips[index].board?.outline = outline }
                if let pin1 { chips[index].board?.pin1 = pin1 }
                if let reference { chips[index].board?.reference = reference }
            }
        }
    }

    /// Moves a footprint's reference so its text is centred at `point` (board
    /// mm), snapped to the trace grid. The reference is stored relative to the
    /// part (`BoardPlacement.referenceOffset`), so it follows the part. A
    /// locked footprint's reference can still be moved.
    func moveReference(_ id: UUID, toCentre point: CGPoint) {
        recordEdit(.pcb, "Move Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }), let board = chips[index].board else { return }
            let grid = CGFloat(self.board.traceGrid)
            let snapped = BoardGrid.snap(point, spacing: grid)
            let local = board.local(snapped)
            // Whole 0.0001 mm, so the file carries no float noise.
            let offset = CGPoint(x: (local.x * 10000).rounded() / 10000 + 0, y: (local.y * 10000).rounded() / 10000 + 0)
            if board.referenceOffset != offset { chips[index].board?.referenceOffset = offset }
        }
    }

    /// Puts a footprint's reference back on the body centre.
    func resetReference(_ id: UUID) {
        recordEdit(.pcb, "Reset Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].board?.referenceOffset != .zero else { return }
            chips[index].board?.referenceOffset = .zero
        }
    }

    // MARK: Board

    /// Sets the board's size in mm, each side clamped to `BoardSettings.sizeRange`.
    func setBoardSize(width: Double, height: Double) {
        editBoard("Change Board Size") { $0.setSize(width: width, height: height) }
    }

    /// Runs `change` on the board's settings as one undo step named `name`.
    private func editBoard(_ name: String, _ change: (inout BoardSettings) -> Void) {
        edit(.pcb, name) { scope in layout.updateSettings(scope, change) }
    }

    /// Sets the card settings (lanes, length, key, size, memory) and fits
    /// the board to them: the standard's height, and its length unless the
    /// card's length is free (PCI Express), when it stays within range.
    func setCardOptions(_ options: CardOptions) {
        editBoard("Change Card") { $0.setCardOptions(options) }
    }

    /// Sets the custom card's edge connector (clamped to fit), and grows
    /// the board to hold its tab.
    func setCustomCard(_ card: CustomCard) {
        editBoard("Change Card Edge") { $0.setCustomCard(card) }
    }

    /// Makes the board a custom rectangle, a custom card (keeping its size,
    /// grown to fit the tab) or a standard card. A card takes
    /// its standard's height and length (the length stays editable), and
    /// at least 2 layers when it has fingers, which are on both sides. Back to
    /// Custom keeps the size. Nothing on the board moves.
    func setBoardType(_ type: BoardType) {
        editBoard("Change Board Type") { $0.setType(type) }
    }

    /// Makes the board a free-form Custom board with this outline (plan-65):
    /// its bounds' top-left at (0, 0), no crossing, each side 1 to 1000 mm. It
    /// is stored clockwise and sets the board's width and height to its
    /// bounds; parts, cutouts and everything else stay where they are.
    /// False, with no undo step, when `BoardSettings.problem(outline:)` refuses it.
    @discardableResult
    func setBoardOutline(_ points: [CGPoint]) -> Bool {
        guard BoardSettings.problem(outline: points) == nil else { return false }
        editBoard("Change Board Outline") { $0.setOutline(points) }
        return true
    }

    /// File ▸ Import ▸ DXF… (plan-65-I), one undo step named `Import DXF`. *Outline and
    /// cutouts*: the board takes the outline (a free-form Custom board), every cutout it had
    /// is replaced by the DXF's, and nothing else moves. *Cutouts only*: the DXF's loops are added.
    /// The new cutouts are the selection. Does nothing, with no step, when the outline is
    /// refused (`BoardSettings.problem(outline:)`) or there is nothing to import.
    func applyDXFImport(_ placed: DXFPlacedImport, mode: DXFImportMode) {
        edit(.pcb, "Import DXF") { scope in
            if mode == .outlineAndCutouts {
                guard let outline = placed.outline, setBoardOutline(outline) else { return }
                layout.updateFeatures(scope) { $0.cutouts.removeAll() }
            }
            guard mode == .outlineAndCutouts || !placed.cutouts.isEmpty else { return }
            var added: [BoardItem] = []
            for loop in placed.cutouts {
                if let id = addCutout(.polygon(loop)) { added.append(.cutout(id)) }
            }
            boardSelection.set(Set(added))
            pruneSelection()
        }
    }

    /// Puts the rectangle back: the free-form outline is dropped and the
    /// board keeps its width and height (the old bounds).
    func resetBoardOutline() {
        editBoard("Reset Board Outline") { $0.resetOutline() }
    }

    /// Sets the component grid (mm), clamped to `BoardSettings.gridRange`.
    /// The trace grid follows it. Nothing on the board moves.
    func setBoardGrid(_ mm: Double) {
        editBoard("Change Grid") { $0.setGrid(mm) }
    }

    /// The units the PCB sidebar shows sizes in.
    func setBoardUnits(_ units: BoardUnits) {
        editBoard("Change Units") { $0.units = units }
    }

    /// The board's solder-mask colour.
    func setBoardColor(_ color: BoardColor) {
        editBoard("Change Board Color") { $0.color = color }
    }

    // MARK: Board layers

    /// Why the board can't have `count` copper layers, or nil when it can.
    /// A count is refused when it would drop a layer that holds something.
    func canSetLayerCount(_ count: Int) -> String? {
        board.layerCountProblem(count, bottomItems: chips.contains { $0.board?.side == .bottom }
                                    || texts.contains { $0.side == .bottom },
                                traceLayers: Set(traces.map(\.layer)), pourLayers: Set(pours.map(\.layer)),
                                hasVias: !vias.isEmpty)
    }

    /// Sets the copper layer count, unless `canSetLayerCount` refuses it.
    func setLayerCount(_ count: Int) {
        guard canSetLayerCount(count) == nil else { return }
        editBoard("Change Layers") { $0.layers = count }
    }

    // MARK: Stackup (PCB)

    func setStackupFinish(_ finish: SurfaceFinish) {
        editBoard("Change Stackup") { $0.stackup.finish = finish }
    }

    /// Sets the temperature rise the current estimate allows, clamped to
    /// `BoardStackup.temperatureRiseRange`.
    func setStackupTemperatureRise(_ degrees: Double) {
        editBoard("Change Stackup") { $0.setTemperatureRise(degrees) }
    }

    /// Sets the thickness (mm) of the copper layer at `index` (Top is 0),
    /// clamped to `BoardStackup.copperRange`.
    func setCopperThickness(_ mm: Double, at index: Int) {
        editBoard("Change Stackup") { $0.setCopperThickness(mm, at: index) }
    }

    /// Changes the dielectric at `index` (the one below copper layer
    /// `index`): any of its kind, thickness (mm) and Er; values are clamped
    /// to `BoardStackup.thicknessRange` and `erRange`.
    func setDielectric(at index: Int, kind: DielectricKind? = nil, thickness: Double? = nil, er: Double? = nil) {
        editBoard("Change Stackup") { $0.setDielectric(at: index, kind: kind, thickness: thickness, er: er) }
    }

    /// Puts the board's standard stackup for its layer count back, keeping
    /// nothing of the old copper and dielectrics, the finish or the rise.
    func resetStackup() {
        editBoard("Reset Stackup") { $0.resetStackup() }
    }

    func setLayerColor(_ layer: CopperLayer, _ color: LayerColor) {
        editBoard("Change Layer Color") { $0.setLayerColor(layer, color) }
    }

    /// Why `layer` can't become `type`, or nil when it can. Top and Bottom
    /// are always Component; an inner layer is Plane or Wire.
    func canSetLayerType(_ layer: CopperLayer, _ type: LayerType) -> String? {
        board.layerTypeProblem(layer, type, hasTraces: traces.contains { $0.layer == layer },
                               hasPours: pours.contains { $0.layer == layer })
    }

    func setLayerType(_ layer: CopperLayer, _ type: LayerType) {
        guard canSetLayerType(layer, type) == nil else { return }
        editBoard("Change Layer Type") { $0.setLayerType(layer, type) }
    }

    /// Sets a Plane layer's net; nil leaves it on no net.
    func setLayerNet(_ layer: CopperLayer, _ net: PlaneNet?) {
        editBoard("Change Plane Net") { $0.setLayerNet(layer, net) }
    }

    /// Sets a Plane layer's clearance (mm), clamped to
    /// `LayerStyle.clearanceRange`; nil goes back to the DRC's minimum.
    func setLayerClearance(_ layer: CopperLayer, _ mm: Double?) {
        editBoard("Change Plane Clearance") { $0.setLayerClearance(layer, mm) }
    }

    /// The nets a Plane can carry, as its Net picker lists them: ground,
    /// then each power rail from low to high (every supply symbol and pull
    /// resistor of one voltage is one rail), then each signal net the
    /// schematic wires, named by up to three of its pins ("U1.3, R2.1, …")
    /// and stored by its first.
    func planeNetChoices() -> [(net: PlaneNet, title: String)] {
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: netLabels)
        var rails = Set<UInt32>()
        var railNets = Set<Int>()
        for chip in chips {
            if case let .supply(_, mv, _) = chip.part.details { rails.insert(mv) }
        }
        for connection in netlist.connections {
            if case .supply = chips[connection.part].part.details { railNets.insert(connection.net) }
        }
        var choices: [(net: PlaneNet, title: String)] = rails.sorted().map { mv in
            (PlaneNet.rail(mv: mv), Self.planeNetTitle(rail: mv))
        }
        var pinsOfNet: [Int: [(chip: PlacedChip, designator: String)]] = [:]
        for connection in netlist.connections where !railNets.contains(connection.net) {
            let chip = chips[connection.part]
            pinsOfNet[connection.net, default: []].append((chip, connection.pin))
        }
        typealias NetPin = (chip: PlacedChip, designator: String)
        func before(_ a: NetPin, _ b: NetPin) -> Bool {
            let byPart = a.chip.reference.localizedStandardCompare(b.chip.reference)
            if byPart != .orderedSame { return byPart == .orderedAscending }
            return a.designator.localizedStandardCompare(b.designator) == .orderedAscending
        }
        let signals = pinsOfNet.values.map { $0.sorted(by: before) }
            .filter { $0.count > 1 }
            .sorted { before($0[0], $1[0]) }
        for pins in signals {
            var title = pins.prefix(3).map { "\($0.chip.reference).\($0.designator)" }.joined(separator: ", ")
            if pins.count > 3 { title += ", …" }
            choices.append((.pin(chip: pins[0].chip.id, designator: pins[0].designator), title))
        }
        return choices
    }

    /// "GND" for the 0 mV rail, else its voltage: "5 V", "3.3 V".
    nonisolated static func planeNetTitle(rail mv: UInt32) -> String {
        mv == 0 ? "GND" : "\(VoltageCheck.volts(mv)) V"
    }

    // MARK: Board texts

    /// Runs `change` on the board's texts as one undo step named `name`.
    private func editTexts<T>(_ name: String, _ change: (inout BoardTexts) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateTexts(scope, change) }
    }

    /// Adds a "Text" on the silkscreen with its top-left at `point` (snapped)
    /// and selects it.
    @discardableResult
    func addText(at point: CGPoint) -> UUID {
        return edit(.pcb, "Add Text") { scope -> UUID in
            let grid = board.grid
            let text = layout.updateTexts(scope) { $0.add(at: point, grid: grid) }
            select(.text(text.id))
            return text.id
        }
    }

    /// Changes a text's words. An empty string is refused and the text kept.
    func setText(_ id: UUID, _ value: String) {
        editTexts("Edit Text") { $0.setText(id, value) }
    }

    /// Sets a text's font size in mil, clamped to `BoardText.sizeRange`.
    func setTextSize(_ id: UUID, _ size: Int) {
        editTexts("Change Font Size") { $0.setSize(id, size) }
    }

    func setTextLayer(_ id: UUID, _ layer: TextLayer) {
        editTexts("Change Layer") { $0.setLayer(id, layer) }
    }

    /// Sets the layer of every text in `ids`, as one undo step.
    func setTextLayer(_ ids: some Collection<UUID>, _ layer: TextLayer) {
        editTexts("Change Layer") { texts in ids.forEach { texts.setLayer($0, layer) } }
    }

    /// Sets the font size (mil) of every text in `ids`, as one undo step.
    func setTextSize(_ ids: some Collection<UUID>, _ size: Int) {
        editTexts("Change Font Size") { texts in ids.forEach { texts.setSize($0, size) } }
    }

    /// Puts a text on the top or bottom of the board. Its box stays where it
    /// is. Does nothing on a 1-layer board, except to bring a text back to Top.
    func setTextSide(_ id: UUID, _ side: BoardSide) {
        let layers = board.layers
        editTexts("Change Side") { $0.setSide(id, side, layers: layers) }
    }

    /// Moves a text's anchor to `point`, snapped to the grid.
    func moveText(_ id: UUID, to point: CGPoint) {
        let grid = board.grid
        editTexts("Move") { $0.move(id, to: point, grid: grid) }
    }

    /// Sets a text's rotation: 0, 90, 180 or 270. Other values are ignored.
    func setTextRotation(_ id: UUID, _ rotation: Int) {
        editTexts("Rotate") { $0.setRotation(id, rotation) }
    }

    /// Turns a text 90° clockwise about its anchor.
    func rotateText(_ id: UUID, counterclockwise: Bool = false) {
        editTexts("Rotate") { $0.rotate(id, counterclockwise: counterclockwise) }
    }

    func deleteText(_ id: UUID) {
        edit(.pcb, "Delete Text") { scope in
            layout.updateTexts(scope) { $0.remove([id]) }
            pruneSelection()
        }
    }

    /// The text under a board point (mm): the last one drawn there, a Top
    /// text before a Bottom one. A text that isn't drawn can't be hit.
    func text(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardText? {
        layout.boardTexts.text(at: point, visible: visible)
    }

    /// The footprint under a board point (mm): the last one drawn there, a
    /// Top footprint before a Bottom one. It is hit by its bounds while
    /// anything of it is drawn.
    func footprint(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> PlacedChip? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: {
                $0.board?.side == side && ($0.boardBounds?.contains(point) ?? false) && visible.showsFootprint($0)
            }) { return hit }
        }
        return nil
    }

    /// The footprint whose drawn reference text is under a board point (mm),
    /// within `tolerance` of its box: Top before Bottom, the last one drawn
    /// first. `height` is the text's drawn height (mm); a reference that is
    /// hidden, or whose layer is off, can't be hit.
    func referenceHit(at point: CGPoint, tolerance: CGFloat, height: Double = BoardReference.height,
                      visible: BoardVisibility = BoardVisibility()) -> UUID? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: { chip in
                guard let placement = chip.board, placement.side == side,
                      visible.showsMark(placement.reference, side: side),
                      let rect = chip.boardReferenceRect(height: height) else { return false }
                return rect.insetBy(dx: -tolerance, dy: -tolerance).contains(point)
            }) { return hit.id }
        }
        return nil
    }

    /// The pad under a board point (mm) and what to call it, for the PCB
    /// page's hover label: a part's pad ("U3.5 A0"; "U3.5" when the pin's
    /// name is just its number; "U3 centre pad" for an unnumbered one), or a
    /// card's connector pin ("Edge.13 A0", with the edge bus's wire name) or
    /// named header pad off the bus. Top before Bottom; a pad that
    /// isn't drawn can't be hit.
    func padLabel(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> (pad: PlacedPad, label: String)? {
        func hits(_ pad: PlacedPad) -> Bool {
            guard pad.rect.contains(point) else { return false }
            return Self.point(point, touches: pad)
        }
        for side in [BoardSide.top, .bottom] {
            for (chip, pads) in zip(chips, chipPads).reversed() {
                guard let placed = chip.board?.side else { continue }
                // A pad is on its layer's side, which an opposite pad's is
                // not its part's.
                guard let pad = pads.last(where: {
                    $0.plated && $0.layerSide(on: placed) == side && visible.showsPad(drill: $0.drill, side: side)
                        && hits($0)
                }) else { continue }
                guard let designator = pad.designator else { return (pad, "\(chip.reference) centre pad") }
                let name = chip.part.pins.first { $0.designator == designator }?.name ?? ""
                let label = "\(chip.reference).\(designator)"
                return (pad, name.isEmpty || name == designator ? label : "\(label) \(name)")
            }
        }
        let named = (board.fingers + board.extraPads).filter { !$0.name.isEmpty }
        guard let finger = named.last(where: {
            visible.showsPad(drill: $0.drill, side: $0.side) && hits($0.pad)
        }) else { return nil }
        // A connector pin is named like a part's pad ("Edge.13"); one on the
        // bus also shows the edge bus's wire of the same position
        // (`BoardSettings.pinNames`), as "Edge.13 A0".
        guard board.fingers.contains(finger) else { return (finger.pad, finger.name) }
        let label = "\(board.connectorName).\(finger.name)"
        if let busID = edgeBusID, let bus = bus(busID),
           let index = board.pinNames.firstIndex(of: finger.name), index < bus.wires.count {
            let wire = bus.wires[index].name
            if !wire.isEmpty, wire != finger.name { return (finger.pad, "\(label) \(wire)") }
        }
        return (finger.pad, label)
    }


    // MARK: Traces (PCB)

    /// Runs `change` on the board's traces and vias as one undo step named `name`.
    private func editCopper<T>(_ name: String, _ change: (inout BoardCopper) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateCopper(scope, change) }
    }

    /// The pads on `layer`, with their parts: a surface-mount pad is on its
    /// layer's side (its part's, or the other for an opposite pad); a
    /// through-hole pad is on every layer.
    func pads(on layer: CopperLayer) -> [(chip: PlacedChip, pad: PlacedPad)] {
        zip(chips, chipPads).flatMap { chip, pads -> [(chip: PlacedChip, pad: PlacedPad)] in
            guard let side = chip.board?.side else { return [] }
            return pads
                .filter { $0.plated && ($0.drill > 0 || $0.layerSide(on: side).copperLayer == layer) }
                .map { (chip: chip, pad: $0) }
        }
    }

    /// Where a trace end pressed or released at `point` lands on `layer`:
    /// the centre of a pad there, else of a via, else the nearest of a
    /// plated hole's four contacts (`BoardHole.contacts`), else the inner end of a
    /// card's edge finger or header pad there (`EdgeFinger.contact`), else a vertex of a trace on that layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point (`BoardSettings.traceGrid`).
    func traceAnchor(at point: CGPoint, layer: CopperLayer, tolerance: CGFloat) -> CGPoint {
        if let hit = pads(on: layer).last(where: { $0.pad.rect.contains(point) }) {
            return CGPoint(x: hit.pad.rect.midX, y: hit.pad.rect.midY)
        }
        if let via = via(at: point) { return via.centre }
        if let hole = holes.last(where: { $0.plated && Self.distance(from: point, to: $0.centre) <= $0.diameter / 2 }),
           let contact = hole.contact(nearest: point) {
            return contact
        }
        if let finger = (board.fingers + board.extraPads).last(where: { $0.isOn(layer) && $0.rect.contains(point) }) {
            return finger.contact
        }
        return layout.copper.nearestTracePoint(to: point, layer: layer, tolerance: tolerance)
            ?? BoardGrid.snap(point, spacing: board.traceGrid)
    }

    /// Adds a trace from `a` to `b` on `layer`, routed at `angle`
    /// (`TraceRoute.route`; `diagonalFirst` puts the angled leg first). See
    /// `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(from a: CGPoint, to b: CGPoint, layer: CopperLayer, angle: Int = TraceRoute.defaultAngle,
                  diagonalFirst: Bool) -> UUID? {
        addTrace(points: TraceRoute.route(from: a, to: b, angle: angle, legFirst: diagonalFirst), layer: layer)
    }

    /// Adds a trace along `points` (simplified) on `layer`, as wide as the
    /// class of the net its first point is on (`netRules(at:layer:)`), and
    /// joins it to every trace it touches there. Returns its id, or nil when
    /// nothing was added: no length, or a layer that is a plane or not on
    /// the board.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer) -> UUID? {
        guard let start = points.first else { return nil }
        return addTrace(points: points, layer: layer, width: netRules(at: start, layer: layer).width)
    }

    /// Adds a trace `width` mil wide (clamped to `BoardTrace.widthRange`);
    /// otherwise as `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer, width: Int) -> UUID? {
        let settings = board
        return editCopper("Add Trace") { $0.addTrace(points: points, layer: layer, width: width, settings: settings) }
    }

    /// Makes every point where trace `id` touches or crosses another trace
    /// on its layer a vertex of both. Inserting a vertex on a straight
    /// segment keeps its angle, so both stay legal.
    func join(_ id: UUID) {
        editCopper("Join Traces") { $0.join(id) }
    }

    /// Replaces the vertices of an unlocked trace with `points` (legal: at
    /// least two, none of zero length), keeping its id, layer, width and
    /// flags; the router's push moves existing copper this way (plan-66-C).
    /// Not joined to anything: the caller keeps the trace's contacts.
    @discardableResult
    func setTracePoints(_ id: UUID, _ points: [CGPoint]) -> Bool {
        guard layout.copper.canSetPoints(id, points) else { return false }
        editCopper("Move Trace") { $0.setPoints(id, points) }
        return true
    }

    /// The traces on the same layer reachable from `id` through shared
    /// vertices, `id` first.
    func connectedRun(of id: UUID) -> [UUID] {
        layout.copper.connectedRun(of: id)
    }

    /// The pads the traces of `run` touch on their layer, as
    /// "<reference>.<designator>" ("U1.3", or "U1.EP" for an unnumbered
    /// centre pad), sorted by reference, then designator.
    func pads(touching run: [UUID]) -> [String] {
        let members = traces.filter { run.contains($0.id) }
        guard let layer = members.first?.layer else { return [] }
        func touched(_ rect: CGRect) -> Bool {
            members.contains { trace in trace.segments.contains { Self.segment($0.0, $0.1, touches: rect) } }
        }
        var found: [(reference: String, designator: String)] = []
        for (chip, pad) in pads(on: layer) {
            let hit = members.contains { trace in trace.segments.contains { Self.segment($0.0, $0.1, touches: pad) } }
            if hit { found.append((chip.reference, pad.designator ?? "EP")) }
        }
        for finger in board.fingers where finger.isOn(layer) && touched(finger.rect) {
            found.append((board.connectorName, finger.name))
        }
        for pad in board.extraPads where pad.isOn(layer) && touched(pad.rect) {
            // "ICSP.1": its group and pin.
            let parts = pad.name.split(separator: ".", maxSplits: 1).map(String.init)
            if parts.count == 2 { found.append((parts[0], parts[1])) }
        }
        return found
            .sorted {
                $0.reference != $1.reference
                    ? $0.reference.localizedStandardCompare($1.reference) == .orderedAscending
                    : $0.designator.localizedStandardCompare($1.designator) == .orderedAscending
            }
            .map { "\($0.reference).\($0.designator)" }
    }

    /// Sets a trace's width in mil, clamped to `BoardTrace.widthRange`.
    func setTraceWidth(_ id: UUID, _ mil: Int) {
        editCopper("Change Trace Width") { $0.setWidth(id, mil) }
    }

    /// Moves one trace to another layer and joins it there. Ignored for a
    /// plane or a layer not on the board. Its run on the old layer stays.
    func setTraceLayer(_ id: UUID, _ layer: CopperLayer) {
        let settings = board
        editCopper("Change Layer") { $0.setLayer(id, layer, settings: settings) }
    }

    // MARK: Routing primitives (plan-38-G)

    /// Removes traces and vias by id, locked or not, inside the caller's
    /// edit (`applyRouting`, `unroute`); the callers decide what may go.
    func removeCopper(traces ids: Set<UUID>, vias viaIDs: Set<UUID>) {
        editCopper("Delete") { $0.remove(traces: ids, vias: viaIDs) }
        pruneSelection()
    }

    /// Appends a trace as it is (no join), inside the caller's edit. False
    /// when the board refuses it: a plane or absent layer, or no length.
    func appendTrace(_ trace: BoardTrace) -> Bool {
        let settings = board
        return editCopper("Add Trace") { $0.append(trace, settings: settings) }
    }

    /// Appends a via, inside the caller's edit. False on a 1-layer board.
    func appendVia(_ via: BoardVia) -> Bool {
        let layers = board.layers
        return editCopper("Add Via") { $0.append(via, layers: layers) }
    }

    func deleteTrace(_ id: UUID) {
        edit(.pcb, "Delete Trace") { scope in
            layout.updateCopper(scope) { $0.remove(traces: [id], vias: []) }
            pruneSelection()
        }
    }

    /// The PCB Delete tool's click: deletes the text under `point`, else the
    /// via, else the trace (as `text(at:)`, `via(at:)` and `trace(at:)` hit
    /// them). Footprints are never deleted. Returns whether something was.
    @discardableResult
    func deleteBoardItem(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> Bool {
        if let hit = text(at: point, visible: visible) {
            deleteText(hit.id)
            return true
        }
        if let hit = via(at: point, visible: visible) {
            deleteVia(hit.id)
            return true
        }
        if let hit = hole(at: point, visible: visible) {
            deleteHole(hit.id)
            return true
        }
        if let hit = trace(at: point, tolerance: tolerance, visible: visible) {
            deleteTrace(hit.id)
            return true
        }
        switch pourItem(at: point, tolerance: tolerance, visible: visible) {
        case let .pour(id)?:
            deletePour(id)
            return true
        case let .keepout(id)?:
            deleteKeepout(id)
            return true
        case let .cutout(id)?:
            deleteCutout(id)
            return true
        default:
            return false
        }
    }

    /// The ids of the selected PCB items `pick` takes.
    private func selected(_ pick: (BoardItem) -> UUID?) -> Set<UUID> {
        Set(boardSelection.items.compactMap(pick))
    }

    /// The selected traces, in drawing order, for ⌘C.
    func selectedTraces() -> [BoardTrace] {
        let ids = selected { if case let .trace(id) = $0 { id } else { nil } }
        return traces.filter { ids.contains($0.id) }
    }

    /// The selected board texts, in creation order, for ⌘C.
    func selectedTexts() -> [BoardText] {
        let ids = selected { if case let .text(id) = $0 { id } else { nil } }
        return texts.filter { ids.contains($0.id) }
    }

    /// The selected vias, in creation order, for ⌘C.
    func selectedVias() -> [BoardVia] {
        let ids = selected { if case let .via(id) = $0 { id } else { nil } }
        return vias.filter { ids.contains($0.id) }
    }

    /// ⌘V on the PCB: adds a copy of each of `copied` moved by `offset`
    /// (mm), with new ids, its width and layer kept, joined to whatever it
    /// touches, as one step. The copies become the whole selection. A trace
    /// whose layer isn't on this board, or is a plane, is skipped. Returns
    /// the new ids.
    @discardableResult
    func pasteTraces(_ copied: [BoardTrace], offset: CGSize) -> [UUID] {
        pasteBoard(traces: copied, texts: [], offset: offset)
    }

    /// ⌘V on the PCB: adds a copy of each of `traces` (as `pasteTraces`), of
    /// each of `texts` and of each of `vias`, moved by `offset` (mm), with
    /// new ids, as one step. A text keeps its words, size, layer, rotation
    /// and side (Top on a 1-layer board), lands on the grid and is not
    /// locked. A via keeps its drill, diameter and tenting, lands on the
    /// trace grid, is not locked and is not the autorouter's; none are
    /// pasted on a 1-layer board. The copies become the whole selection.
    /// Returns the new ids.
    @discardableResult
    func pasteBoard(traces copied: [BoardTrace], texts copiedTexts: [BoardText], vias copiedVias: [BoardVia] = [],
                    offset: CGSize) -> [UUID] {
        let settings = board
        guard copied.contains(where: { settings.traceLayerRefusal($0.layer) == nil }) || !copiedTexts.isEmpty
                || (settings.layers > 1 && !copiedVias.isEmpty) else { return [] }
        return edit(.pcb, "Paste") { scope -> [UUID] in
            let copper = layout.updateCopper(scope) {
                $0.paste(traces: copied, vias: copiedVias, offset: offset, settings: settings)
            }
            let texts = layout.updateTexts(scope) {
                $0.paste(copiedTexts, offset: offset, grid: settings.grid, layers: settings.layers)
            }
            let items = copper.traces.map(BoardItem.trace) + texts.map { .text($0.id) } + copper.vias.map(BoardItem.via)
            boardSelection.set(Set(items))
            return copper.traces + texts.map(\.id) + copper.vias
        }
    }

    /// ⌘⌫ on the PCB: deletes every selected text, trace, via and hole, as
    /// one step. Selected footprints stay. Returns whether anything was
    /// deleted.
    @discardableResult
    func deleteBoardSelection() -> Bool {
        var textIDs: Set<UUID> = [], traceIDs: Set<UUID> = [], viaIDs: Set<UUID> = [], holeIDs: Set<UUID> = []
        var pourIDs: Set<UUID> = [], keepoutIDs: Set<UUID> = [], cutoutIDs: Set<UUID> = []
        for item in boardSelection.items {
            switch item {
            case let .text(id): textIDs.insert(id)
            case let .trace(id): traceIDs.insert(id)
            case let .via(id): viaIDs.insert(id)
            case let .hole(id): holeIDs.insert(id)
            case let .pour(id): pourIDs.insert(id)
            case let .keepout(id): keepoutIDs.insert(id)
            case let .cutout(id): cutoutIDs.insert(id)
            case .footprint: break
            }
        }
        guard !textIDs.isEmpty || !traceIDs.isEmpty || !viaIDs.isEmpty || !holeIDs.isEmpty || !pourIDs.isEmpty
                || !keepoutIDs.isEmpty || !cutoutIDs.isEmpty else { return false }
        edit(.pcb, "Delete") { scope in
            layout.updateTexts(scope) { $0.remove(textIDs) }
            layout.updateCopper(scope) { $0.remove(traces: traceIDs, vias: viaIDs) }
            layout.updateFeatures(scope) {
                $0.remove(holes: holeIDs, pours: pourIDs, keepouts: keepoutIDs, cutouts: cutoutIDs)
            }
            pruneSelection()
        }
        return true
    }

    /// ⌘X on the PCB, after the selection was copied: deletes just the
    /// selected traces, texts and vias (what ⌘C takes), as one step named "Cut".
    /// Returns whether any were deleted.
    @discardableResult
    func cutBoardSelection() -> Bool {
        let traceIDs = Set(selectedTraces().map(\.id))
        let textIDs = Set(selectedTexts().map(\.id))
        let viaIDs = Set(selectedVias().map(\.id))
        guard !traceIDs.isEmpty || !textIDs.isEmpty || !viaIDs.isEmpty else { return false }
        edit(.pcb, "Cut") { scope in
            layout.updateCopper(scope) { $0.remove(traces: traceIDs, vias: viaIDs) }
            layout.updateTexts(scope) { $0.remove(textIDs) }
            pruneSelection()
        }
        return true
    }

    /// Moves every vertex of the traces `ids` by `offset` (mm), then joins
    /// each one where it now lands.
    func moveTraces(_ ids: Set<UUID>, by offset: CGSize) {
        editCopper("Move") { $0.moveTraces(ids, by: offset) }
    }

    /// The trace under a board point (mm): the last one drawn there (Top's
    /// layer first, down the stack) whose layer is shown, within half its
    /// width plus `tolerance`.
    func trace(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> BoardTrace? {
        layout.copper.trace(at: point, tolerance: tolerance, layers: board.copperLayers.filter(visible.showsTrace(on:)))
    }

    // MARK: Vias (PCB)

    /// The via whose metal is under `point` (mm): the last one drawn there.
    /// Vias are drawn while any copper layer is.
    func via(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardVia? {
        guard visible.showsAnyCopper else { return nil }
        return layout.copper.via(at: point)
    }

    /// Where a via placed at `point` lands: a vertex of a trace on any layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point.
    func viaAnchor(at point: CGPoint, tolerance: CGFloat) -> CGPoint {
        layout.copper.nearestTracePoint(to: point, layer: nil, tolerance: tolerance)
            ?? BoardGrid.snap(point, spacing: board.traceGrid)
    }

    /// Adds a via at `point` (already anchored) with the sizes of the class
    /// of the net there (`netRules(at:layer:)`; the defaults on no net or
    /// the Default class) and selects it. Nil on a 1-layer board, which has
    /// nothing to join.
    @discardableResult
    func addVia(at point: CGPoint) -> UUID? {
        guard board.layers > 1 else { return nil }
        let rules = netRules(at: point, layer: nil)
        return edit(.pcb, "Add Via") { scope -> UUID? in
            let id = layout.updateCopper(scope) { $0.addVia(at: point, rules: rules) }
            boardSelection.replace(with: .via(id))
            return id
        }
    }

    /// Moves a via's centre to `point`, snapped to the trace grid.
    func moveVia(_ id: UUID, to point: CGPoint) {
        let target = BoardGrid.snap(point, spacing: board.traceGrid)
        editCopper("Move") { $0.setViaCentre(id, target) }
    }

    /// Moves a via's centre to exactly `point` (the sidebar's X and Y).
    func setViaCentre(_ id: UUID, _ point: CGPoint) {
        editCopper("Move") { $0.setViaCentre(id, point) }
    }

    /// Sets a via's hole, clamped to `BoardVia.drillRange` and to leave the
    /// thinnest ring inside its metal.
    func setViaDrill(_ id: UUID, _ mm: Double) {
        editCopper("Change Via Hole") { $0.setViaDrill(id, mm) }
    }

    /// Sets a via's metal diameter, clamped to `BoardVia.diameterRange` and
    /// to keep the thinnest ring round its hole.
    func setViaDiameter(_ id: UUID, _ mm: Double) {
        editCopper("Change Via Size") { $0.setViaDiameter(id, mm) }
    }

    /// Sets whether the solder mask covers a via on `side`.
    func setViaTenting(_ id: UUID, _ side: BoardSide, _ tented: Bool) {
        editCopper("Change Via Tenting") { $0.setViaTenting(id, side, tented) }
    }

    // MARK: Electrical rules check settings (plan-63-E)

    /// Sets a rule's severity (the default clears the change). One undo step.
    func setErcSeverity(_ rule: ErcRule, _ severity: ErcSeverity) {
        guard rule.hasSeverity else { return }
        recordEdit(.schematic, "Change ERC Settings") {
            var next = erc
            next.set(severity, for: rule)
            if next != erc { erc = next }
        }
    }

    /// Sets one cell of the pin map and its mirror. One undo step.
    func setErcPinMap(_ a: Int, _ b: Int, to level: UInt8) {
        recordEdit(.schematic, "Change ERC Settings") {
            var next = erc
            next.setPinMap(a, b, to: level)
            if next != erc { erc = next }
        }
    }

    /// Back to KiCad's severities and pin map; the exclusions stay. One undo step.
    func resetErcSettings() {
        recordEdit(.schematic, "Reset ERC Settings") {
            var next = erc
            next.severities = [:]
            next.pinMap = nil
            if next != erc { erc = next }
        }
    }

    /// Excludes violations by key (`ErcViolation.key`). One undo step.
    func excludeErc(_ keys: Set<String>) {
        recordEdit(.schematic, "Exclude Violation") {
            var next = erc
            next.exclusions.formUnion(keys)
            if next != erc { erc = next }
        }
    }

    /// Puts excluded violations back. One undo step.
    func includeErc(_ keys: Set<String>) {
        recordEdit(.schematic, "Include Violation") {
            var next = erc
            next.exclusions.subtract(keys)
            if next != erc { erc = next }
        }
    }

    // MARK: Design rules (PCB)

    /// Sets the board's design rules (each 0 or more) and the manufacturer
    /// they are named after. One undo step.
    func setDesignRules(_ rules: DesignRules, manufacturer: DRCManufacturer) {
        editBoard("Change Design Rules") { $0.setDesignRules(rules, manufacturer: manufacturer) }
    }

    // MARK: Net classes (PCB)

    /// Adds a net class named `name` (trimmed, at most 40 characters; blank
    /// is "Class") with the Default class's values and no nets, last in
    /// priority.
    @discardableResult
    func addNetClass(named name: String) -> UUID {
        edit(.pcb, "Add Net Class") { scope in layout.updateSettings(scope) { $0.addNetClass(named: name) } }
    }

    func removeNetClass(_ id: UUID) {
        editBoard("Delete Net Class") { $0.removeNetClass(id) }
    }

    /// Renames a class; a blank name is ignored, a long one cut to 40
    /// characters.
    func renameNetClass(_ id: UUID, to name: String) {
        editBoard("Rename Net Class") { $0.renameNetClass(id, to: name) }
    }

    /// Sets a class's trace width, clamped to `BoardTrace.widthRange`.
    func setNetClassWidth(_ id: UUID, _ mil: Int) {
        editBoard("Change Net Class Width") { $0.setNetClassWidth(id, mil) }
    }

    /// Sets a class's clearance, clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setNetClassClearance(_ id: UUID, _ mm: Double?) {
        editBoard("Change Net Class Clearance") { $0.setNetClassClearance(id, mm) }
    }

    /// Sets a class's via, clamped as `setViaDrill` and `setViaDiameter`
    /// clamp a via: the hole to `BoardVia.drillRange`, then the metal to
    /// `BoardVia.diameterRange` and at least the thinnest ring round it.
    func setNetClassVia(_ id: UUID, drill: Double, diameter: Double) {
        editBoard("Change Net Class Via") { $0.setNetClassVia(id, drill: drill, diameter: diameter) }
    }

    /// Puts `net` in class `classID`, out of every other class; nil puts it
    /// back in the Default class. The copper already on the net takes the
    /// class's rules: every trace its width, every via its hole and metal.
    /// One undo step.
    func assignNet(_ net: PlaneNet, to classID: UUID?) {
        edit(.pcb, "Change Net Class") { scope in
            layout.updateSettings(scope) { $0.assignNet(net, toClass: classID) }
            // Copper drawn before the net changed class matches new copper;
            // the class's clearance is read by the checks.
            let copper = connectivity()
            guard let netID = copper.net(of: net) else { return }
            let rules = board.netRules(ofNet: netID, in: copper)
            let items = copperItems(ofNet: netID, in: copper)
            layout.updateCopper(scope) { $0.apply(rules, traces: items.traces, vias: items.vias) }
        }
    }

    /// Moves a class to `position` in the list (clamped); order is
    /// priority.
    func moveNetClass(_ id: UUID, to position: Int) {
        editBoard("Move Net Class") { $0.moveNetClass(id, to: position) }
    }

    /// The width and via new copper at `point` gets: those of the class of
    /// the net there — a pad's whose copper holds the point on `layer`
    /// (a through-hole pad on any layer; nil `layer` is any), else a via's
    /// whose metal holds it, else a trace's on `layer` whose copper holds
    /// it — or the Default class's when it is on no net or a net in no class.
    func netRules(at point: CGPoint, layer: CopperLayer?) -> NetRules {
        let copper = connectivity()
        return board.netRules(ofNet: net(at: point, layer: layer, in: copper), in: copper)
    }

    /// The net (of `copper`) whose copper holds `point`, by the order
    /// `netRules(at:layer:)` describes: a plated pad's on `layer`, else a
    /// via's, else a trace's on `layer`; nil when none does.
    func net(at point: CGPoint, layer: CopperLayer?, in copper: BoardConnectivity) -> Int? {
        func onLayer(_ pad: BoardConnectivity.Pad) -> Bool {
            layer == nil || pad.pad.drill > 0 || pad.side.copperLayer == layer
        }
        if let p = copper.pads.indices.first(where: {
            copper.pads[$0].pad.plated && onLayer(copper.pads[$0]) && copper.pads[$0].pad.rect.contains(point)
                && copper.netOfPad[$0] != nil
        }) {
            return copper.netOfPad[p]
        }
        let netOfRoot = copper.netOfRoot()
        if let v = copper.vias.indices.first(where: {
            Self.distance(from: point, to: copper.vias[$0].centre) <= copper.vias[$0].diameter / 2
                && netOfRoot[copper.root[copper.viaNode($0)]] != nil
        }) {
            return netOfRoot[copper.root[copper.viaNode(v)]]
        }
        if let t = copper.traces.indices.first(where: { t in
            let trace = copper.traces[t]
            return (layer == nil || trace.layer == layer) && netOfRoot[copper.root[copper.traceNode(t)]] != nil
                && trace.segments.contains { Self.distance(from: point, toSegment: $0.0, $0.1) <= trace.widthMM / 2 }
        }) {
            return netOfRoot[copper.root[copper.traceNode(t)]]
        }
        return nil
    }

    /// The class net `net` of `connectivity` is in: the first, in list
    /// order, any of whose members names it; nil is the Default class.
    func netClass(ofNet net: Int, in connectivity: BoardConnectivity) -> NetClass? {
        board.netClass(ofNet: net, in: connectivity)
    }

    // MARK: Net layer rules and groups (PCB)

    /// Sets a class's layer rule; nil is any trace layer. A rule with no
    /// layers is ignored.
    func setNetClassLayerRule(_ id: UUID, _ rule: LayerRule?) {
        editBoard("Change Layer Rule") { $0.setNetClassLayerRule(id, rule) }
    }

    /// Sets one net's own layer rule, before its class's; nil removes it.
    /// A rule with no layers is ignored.
    func setNetLayerRule(_ net: PlaneNet, _ rule: LayerRule?) {
        editBoard("Change Layer Rule") { $0.setNetLayerRule(net, rule) }
    }

    /// Adds an empty group (trimmed name, at most 40 characters; blank is
    /// "Group"), last.
    @discardableResult
    func addNetGroup(named name: String, mode: NetGroupMode) -> UUID {
        edit(.pcb, "Add Net Group") { scope in layout.updateSettings(scope) { $0.addNetGroup(named: name, mode: mode) } }
    }

    func removeNetGroup(_ id: UUID) {
        editBoard("Delete Net Group") { $0.removeNetGroup(id) }
    }

    func renameNetGroup(_ id: UUID, to name: String) {
        editBoard("Rename Net Group") { $0.renameNetGroup(id, to: name) }
    }

    /// Changes a group's mode. A group with more than 2 nets can't be a
    /// pair: refused, with `editorMessage` saying why. Returns whether the
    /// group has `mode` now.
    @discardableResult
    func setNetGroupMode(_ id: UUID, _ mode: NetGroupMode) -> Bool {
        guard board.netGroupIndex(id) != nil else { return false }
        if let problem = board.netGroupModeProblem(id, mode) {
            editorMessage = problem
            return false
        }
        editBoard("Change Net Group") { $0.setNetGroupMode(id, mode) }
        return true
    }

    /// Sets a group's tolerance, clamped to `NetGroup.toleranceRange`.
    func setNetGroupTolerance(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.tolerance, mm, NetGroup.toleranceRange)
    }

    /// Sets a pair's gap, clamped to `NetGroup.gapRange`.
    func setNetGroupGap(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.gap, mm, NetGroup.gapRange)
    }

    /// Sets how much of a pair may run apart, clamped to
    /// `NetGroup.maxUncoupledRange`.
    func setNetGroupMaxUncoupled(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxUncoupled, mm, NetGroup.maxUncoupledRange)
    }

    /// Sets a group's largest tuning bump, clamped to
    /// `NetGroup.maxMeanderRange`.
    func setNetGroupMaxMeander(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxMeander, mm, NetGroup.maxMeanderRange)
    }

    private func setNetGroupValue(_ id: UUID, _ key: WritableKeyPath<NetGroup, Double>, _ mm: Double,
                                  _ range: ClosedRange<Double>) {
        editBoard("Change Net Group") { $0.setNetGroupValue(id, key, mm, range) }
    }

    /// Puts `net` in group `groupID`, out of any other group. A pair that
    /// already has 2 other nets refuses it, with `editorMessage` saying
    /// why. Returns whether the net is in the group now.
    @discardableResult
    func assignNet(_ net: PlaneNet, toGroup groupID: UUID) -> Bool {
        guard board.netGroupIndex(groupID) != nil else { return false }
        if let problem = board.netGroupJoinProblem(net, groupID) {
            editorMessage = problem
            return false
        }
        editBoard("Change Net Group") { $0.assignNet(net, toGroup: groupID) }
        return true
    }

    /// Takes `net` out of whichever group it is in.
    func unassignNet(_ net: PlaneNet) {
        editBoard("Change Net Group") { $0.unassignNet(net) }
    }

    /// The layer rule net `net` of `connectivity` follows: its own
    /// override, else its class's, else none.
    func layerRule(ofNet net: Int, in connectivity: BoardConnectivity) -> LayerRule? {
        board.layerRule(ofNet: net, in: connectivity)
    }

    /// The trace layers net `net` may use and those it prefers. Only: the
    /// rule's layers that are trace layers on the board (none means it
    /// can't be routed), with no preference. Prefer: every trace layer,
    /// preferring the rule's. No rule: every trace layer, no preference.
    func effectiveLayers(ofNet net: Int, in connectivity: BoardConnectivity) -> EffectiveLayers {
        board.effectiveLayers(ofNet: net, in: connectivity)
    }

    /// The group net `net` of `connectivity` is in: the first any of whose
    /// members names it.
    func group(ofNet net: Int, in connectivity: BoardConnectivity) -> NetGroup? {
        board.group(ofNet: net, in: connectivity)
    }

    func deleteVia(_ id: UUID) {
        edit(.pcb, "Delete Via") { scope in
            layout.updateCopper(scope) { $0.remove(traces: [], vias: [id]) }
            pruneSelection()
        }
    }

    /// The traces whose copper touches a via's metal, on any layer.
    func traces(touching via: BoardVia) -> [BoardTrace] {
        layout.copper.traces(touching: via.centre, radius: via.diameter / 2)
    }

    // MARK: Pours and keep-outs (PCB)

    /// Runs `change` on the board's holes, pours, keep-outs and cutouts as
    /// one undo step named `name`.
    private func editFeatures<T>(_ name: String, _ change: (inout BoardFeatures) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateFeatures(scope, change) }
    }

    /// Adds a pour over `rect` (snapped to the trace grid) on `layer`, and
    /// selects it. It takes the settings of the last pour added; its net is
    /// the last pour's, else ground when the schematic has it, else none.
    /// Nil when `layer` is not a trace layer of the board.
    @discardableResult
    func addPour(rect: CGRect, layer: CopperLayer) -> UUID? {
        guard board.traceLayers.contains(layer) else { return nil }
        let grid = board.traceGrid
        return edit(.pcb, "Add Pour") { scope -> UUID in
            let ground = pours.isEmpty && planeNetChoices().contains(where: { $0.net == .ground })
            let id = layout.updateFeatures(scope) { $0.addPour(rect: rect, layer: layer, grid: grid, ground: ground) }
            boardSelection.replace(with: .pour(id))
            return id
        }
    }

    /// Moves a pour so its top-left corner is at `origin`, snapped to the trace grid.
    func movePour(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.movePour(id, to: origin, grid: grid) }
    }

    /// Sets a pour's rectangle (the sidebar's X, Y, width and height, a
    /// resize), snapped to the trace grid.
    func setPourRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setPourRect(id, rect, grid: grid) }
    }

    /// Puts a pour on `layer`, a trace layer of the board.
    func setPourLayer(_ id: UUID, _ layer: CopperLayer) {
        guard board.traceLayers.contains(layer) else { return }
        editFeatures("Change Pour Layer") { $0.setPourLayer(id, layer) }
    }

    /// Sets a pour's net (`planeNetChoices`), or none.
    func setPourNet(_ id: UUID, _ net: PlaneNet?) {
        editFeatures("Change Pour Net") { $0.setPourNet(id, net) }
    }

    /// Sets a pour's clearance (mm), clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setPourClearance(_ id: UUID, _ mm: Double?) {
        editFeatures("Change Pour") { $0.setPourClearance(id, mm) }
    }

    func setPourConnection(_ id: UUID, _ connection: PourConnection) {
        editFeatures("Change Pour") { $0.setPourConnection(id, connection) }
    }

    func setPourThermalGap(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourThermalGap(id, mm) }
    }

    func setPourSpokeWidth(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourSpokeWidth(id, mm) }
    }

    func setPourMinWidth(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourMinWidth(id, mm) }
    }

    /// Removes holes, pours, keep-outs or cutouts as one undo step named `name`.
    private func deleteFeatures(_ name: String, holes: Set<UUID> = [], pours: Set<UUID> = [],
                                keepouts: Set<UUID> = [], cutouts: Set<UUID> = []) {
        edit(.pcb, name) { scope in
            layout.updateFeatures(scope) { $0.remove(holes: holes, pours: pours, keepouts: keepouts, cutouts: cutouts) }
            pruneSelection()
        }
    }

    func deletePour(_ id: UUID) {
        deleteFeatures("Delete", pours: [id])
    }

    /// Adds a keep-out over `rect` (snapped to the trace grid) on `layers`
    /// (nil: every layer), and selects it.
    @discardableResult
    func addKeepout(rect: CGRect, layers: Set<CopperLayer>? = nil) -> UUID {
        let grid = board.traceGrid
        return edit(.pcb, "Add Keep-out") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addKeepout(rect: rect, layers: layers, grid: grid) }
            boardSelection.replace(with: .keepout(id))
            return id
        }
    }

    /// Moves a keep-out so its top-left corner is at `origin`, snapped to the trace grid.
    func moveKeepout(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.moveKeepout(id, to: origin, grid: grid) }
    }

    func setKeepoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setKeepoutRect(id, rect, grid: grid) }
    }

    /// Sets the layers a keep-out applies to; nil is every layer.
    func setKeepoutLayers(_ id: UUID, _ layers: Set<CopperLayer>?) {
        editFeatures("Change Keep-out") { $0.setKeepoutLayers(id, layers) }
    }

    func deleteKeepout(_ id: UUID) {
        deleteFeatures("Delete", keepouts: [id])
    }

    // MARK: Cutouts (PCB)

    /// Adds a cutout of `shape` and selects it. A rounded shape's box is
    /// snapped to the trace grid (and kept within the allowed sizes); a polygon
    /// is stored clockwise. Nil, with no undo step, when `BoardCutout.problem`
    /// refuses the shape.
    @discardableResult
    func addCutout(_ shape: BoardCutout.Shape) -> UUID? {
        guard BoardCutout.problem(shape) == nil else { return nil }
        let grid = board.traceGrid
        return edit(.pcb, "Add Cutout") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addCutout(shape, grid: grid) }
            boardSelection.replace(with: .cutout(id))
            return id
        }
    }

    /// Moves a cutout so its bounds' top-left corner is at `origin`, snapped to the trace grid.
    func moveCutout(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.moveCutout(id, to: origin, grid: grid) }
    }

    /// Sets a rounded cutout's box (snapped to the trace grid, at least 0.5 mm a side); its radius
    /// stays within the new box. Does nothing to a polygon or a locked cutout.
    func setCutoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setCutoutRect(id, rect, grid: grid) }
    }

    /// Sets a rounded cutout's corner radius, within 0 to half its short side.
    func setCutoutRadius(_ id: UUID, _ radius: Double) {
        editFeatures("Change Cutout") { $0.setCutoutRadius(id, radius) }
    }

    func deleteCutout(_ id: UUID) {
        deleteFeatures("Delete", cutouts: [id])
    }

    /// Whether a keep-out is drawn with `visible`: any layer it applies to shows.
    func keepoutIsShown(_ keepout: BoardKeepout, _ visible: BoardVisibility) -> Bool {
        BoardFeatures.keepoutIsShown(keepout, visible, layers: board.copperLayers)
    }

    /// The pour or keep-out whose edge is within `tolerance` of `point` (mm):
    /// the last pour, else the last keep-out. Pressing inside one does not
    /// hit it (it is as big as a board), only its edge does.
    func pourItem(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        layout.features.item(at: point, tolerance: tolerance, visible: visible, layers: board.copperLayers)
    }

    // MARK: Holes (PCB)

    /// The hole under `point` (mm): the last one drawn there, within its
    /// land (plated) or its hole. Holes are drawn while any copper layer is.
    func hole(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardHole? {
        guard visible.showsAnyCopper else { return nil }
        return layout.features.hole(at: point)
    }

    /// Adds a hole at `point`, snapped to the trace grid, and selects it.
    /// It takes the size, plating and net of the last hole added, so a set
    /// of mounting holes is quick to place; the first is a non-plated M3.
    @discardableResult
    func addHole(at point: CGPoint) -> UUID {
        let grid = board.traceGrid
        return edit(.pcb, "Add Hole") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addHole(at: point, grid: grid) }
            boardSelection.replace(with: .hole(id))
            return id
        }
    }

    /// Moves a hole's centre to `point`, snapped to the trace grid.
    func moveHole(_ id: UUID, to point: CGPoint) {
        let target = BoardGrid.snap(point, spacing: board.traceGrid)
        editFeatures("Move") { $0.setHoleCentre(id, target) }
    }

    /// Moves a hole's centre to exactly `point` (the sidebar's X and Y).
    func setHoleCentre(_ id: UUID, _ point: CGPoint) {
        editFeatures("Move") { $0.setHoleCentre(id, point) }
    }

    /// Sets a hole's size: a screw size sets its hole and land.
    func setHoleSize(_ id: UUID, _ size: HoleSize) {
        editFeatures("Change Hole Size") { $0.setHoleSize(id, size) }
    }

    /// Sets a hole's drill, clamped to `BoardHole.drillRange`. The land
    /// grows to keep the thinnest ring round it. The size becomes Custom.
    func setHoleDrill(_ id: UUID, _ mm: Double) {
        editFeatures("Change Hole Size") { $0.setHoleDrill(id, mm) }
    }

    /// Sets a hole's land diameter, clamped to `BoardHole.diameterRange` and
    /// to keep the thinnest ring round its hole. The size becomes Custom.
    func setHoleDiameter(_ id: UUID, _ mm: Double) {
        editFeatures("Change Hole Size") { $0.setHoleDiameter(id, mm) }
    }

    /// Makes a hole plated (a copper land that can carry a net) or bare.
    func setHolePlated(_ id: UUID, _ plated: Bool) {
        editFeatures(plated ? "Plate Hole" : "Unplate Hole") { $0.setHolePlated(id, plated) }
    }

    /// Sets a plated hole's net (`planeNetChoices`), or none.
    func setHoleNet(_ id: UUID, _ net: PlaneNet?) {
        editFeatures("Change Hole Net") { $0.setHoleNet(id, net) }
    }

    func deleteHole(_ id: UUID) {
        deleteFeatures("Delete Hole", holes: [id])
    }

    /// The traces whose copper touches a plated hole's land, on any layer.
    func traces(touching hole: BoardHole) -> [BoardTrace] {
        guard hole.plated else { return [] }
        return layout.copper.traces(touching: hole.centre, radius: hole.diameter / 2)
    }

    // MARK: Trace geometry

    /// Whether segment a–b meets `rect` (inside it, or crossing or touching
    /// its edge).
    nonisolated static func segment(_ a: CGPoint, _ b: CGPoint, touches rect: CGRect) -> Bool {
        BoardCopper.segment(a, b, touches: rect)
    }

    // MARK: Lock Position (PCB)

    /// Whether a PCB item's position is locked. False for an item that
    /// isn't there.
    func isLocked(_ item: BoardItem) -> Bool {
        switch item {
        case let .footprint(id): chips.first { $0.id == id }?.board?.locked ?? false
        case let .text(id): texts.first { $0.id == id }?.locked ?? false
        case let .trace(id): traces.first { $0.id == id }?.locked ?? false
        case let .via(id): vias.first { $0.id == id }?.locked ?? false
        case let .hole(id): holes.first { $0.id == id }?.locked ?? false
        case let .pour(id): pours.first { $0.id == id }?.locked ?? false
        case let .keepout(id): keepouts.first { $0.id == id }?.locked ?? false
        case let .cutout(id): cutouts.first { $0.id == id }?.locked ?? false
        }
    }

    /// Locks or unlocks a PCB item's position (the sidebar's Lock Position).
    func setLocked(_ item: BoardItem, _ locked: Bool) {
        guard isLocked(item) != locked else { return }
        edit(.pcb, locked ? "Lock Position" : "Unlock Position") { scope in
            switch item {
            case let .footprint(id):
                if let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil {
                    chips[index].board?.locked = locked
                }
            case let .text(id): layout.updateTexts(scope) { $0.setLocked(id, locked) }
            case let .trace(id): layout.updateCopper(scope) { $0.setLocked(trace: id, locked) }
            case let .via(id): layout.updateCopper(scope) { $0.setLocked(via: id, locked) }
            case .hole, .pour, .keepout, .cutout: layout.updateFeatures(scope) { $0.setLocked(item, locked) }
            }
        }
    }

    /// Locks or unlocks every item in `items` as one undo step.
    func setLocked(_ items: some Collection<BoardItem>, _ locked: Bool) {
        guard items.contains(where: { isLocked($0) != locked }) else { return }
        recordEdit(.pcb, locked ? "Lock Position" : "Unlock Position") {
            for item in items { setLocked(item, locked) }
        }
    }

    /// Puts every footprint and text in `items` on `side` of the board, as
    /// one undo step. Other items have no side and are skipped.
    func setSide(of items: some Collection<BoardItem>, _ side: BoardSide) {
        recordEdit(.pcb, "Change Side") {
            for item in items {
                switch item {
                case let .footprint(id): setFootprintSide(id, side)
                case let .text(id): setTextSide(id, side)
                default: break
                }
            }
        }
    }

    // MARK: Group drags (PCB)

    /// Where every selected footprint (its centre) and text (its anchor) is
    /// now, for a group drag. Locked items are left out, so they stay put.
    func boardGroupBase(grab: CGPoint? = nil) -> BoardGroupBase {
        var base = BoardGroupBase()
        base.grab = grab
        for item in boardSelection.items where !isLocked(item) {
            switch item {
            case let .footprint(id):
                if let centre = chips.first(where: { $0.id == id })?.board?.centre { base.footprints[id] = centre }
            case let .text(id):
                if let anchor = texts.first(where: { $0.id == id })?.anchor { base.texts[id] = anchor }
            case let .trace(id):
                if let points = traces.first(where: { $0.id == id })?.points { base.traces[id] = points }
            case let .via(id):
                if let centre = vias.first(where: { $0.id == id })?.centre { base.vias[id] = centre }
            case let .hole(id):
                if let centre = holes.first(where: { $0.id == id })?.centre { base.holes[id] = centre }
            case let .pour(id):
                if let rect = pours.first(where: { $0.id == id })?.rect { base.pours[id] = rect }
            case let .keepout(id):
                if let rect = keepouts.first(where: { $0.id == id })?.rect { base.keepouts[id] = rect }
            case let .cutout(id):
                if let shape = cutouts.first(where: { $0.id == id })?.shape { base.cutouts[id] = shape }
            }
        }
        return base
    }

    /// One frame of a group drag on the PCB. One item moves as it always
    /// has, snapped onto the grid; a lone trace by the end nearer the grab,
    /// onto the trace grid. Several all move by the same `delta`
    /// snapped to the board grid, so they keep their places relative to
    /// each other.
    func moveBoardGroup(_ base: BoardGroupBase, by delta: CGSize) {
        edit(.pcb, "Move") { scope in
            func moved(_ point: CGPoint, by offset: CGSize) -> CGPoint {
                CGPoint(x: point.x + offset.width, y: point.y + offset.height)
            }
            var offset = CGSize(width: BoardGrid.snap(delta.width, spacing: self.board.grid),
                                height: BoardGrid.snap(delta.height, spacing: self.board.grid))
            // A lone trace snaps by its end: the one nearer where it was
            // grabbed (its start without a grab) lands on the trace grid.
            // A click (no movement yet) leaves it where it is.
            if base.count == 1, delta != .zero, let points = base.traces.values.first, let first = points.first,
               let last = points.last {
                let end = base.grab.map {
                    Self.distance(from: $0, to: last) < Self.distance(from: $0, to: first) ? last : first
                } ?? first
                let target = BoardGrid.snap(moved(end, by: delta), spacing: self.board.traceGrid)
                offset = CGSize(width: target.x - end.x, height: target.y - end.y)
            }
            // Traces move by the snapped delta from where they were, alone or
            // in a group; joins wait for the release (`settleBoardGroup`).
            let traceOffset = offset
            layout.updateCopper(scope) { $0.moveGroup(traces: base.traces, vias: [:], by: traceOffset) }
            if base.count == 1 {
                if let (id, centre) = base.footprints.first { moveFootprint(id, to: moved(centre, by: delta)) }
                if let (id, anchor) = base.texts.first { moveText(id, to: moved(anchor, by: delta)) }
                // A via placed on a trace stays there until the pointer moves.
                if let (id, centre) = base.vias.first, delta != .zero { moveVia(id, to: moved(centre, by: delta)) }
                if let (id, centre) = base.holes.first { moveHole(id, to: moved(centre, by: delta)) }
                if let (id, rect) = base.pours.first { movePour(id, to: moved(rect.origin, by: delta)) }
                if let (id, rect) = base.keepouts.first { moveKeepout(id, to: moved(rect.origin, by: delta)) }
                if let (id, shape) = base.cutouts.first { moveCutout(id, to: moved(shape.bounds.origin, by: delta)) }
                return
            }
            layout.updateFeatures(scope) { $0.moveGroup(base, by: traceOffset) }
            layout.updateCopper(scope) { $0.moveGroup(traces: [:], vias: base.vias, by: traceOffset) }
            for index in chips.indices {
                guard let centre = base.footprints[chips[index].id], chips[index].board != nil else { continue }
                let target = moved(centre, by: offset)
                if chips[index].board?.centre != target { chips[index].board?.centre = target }
            }
            layout.updateTexts(scope) { $0.move(base.texts, by: { moved($0, by: traceOffset) }) }
        }
    }

    /// The end of a PCB group drag: each moved trace joins what it now
    /// touches. Traces that weren't selected stay where they are.
    func settleBoardGroup(_ base: BoardGroupBase) {
        editCopper("Move") { copper in base.traces.keys.forEach { copper.join($0) } }
    }

    // MARK: Segment drags (PCB)

    /// What a plain Select drag on trace `id` at `point` (board mm) takes
    /// hold of: the segment nearest the press, the trace's points now and
    /// which of them must stay where they are. Nil for a locked trace or
    /// none.
    func traceSegmentBase(_ id: UUID, at point: CGPoint) -> TraceSegmentBase? {
        guard let layer = traces.first(where: { $0.id == id })?.layer else { return nil }
        let pads = pads(on: layer).map(\.1.rect) + board.fingers.filter { $0.isOn(layer) }.map(\.rect)
            + board.extraPads.filter { $0.isOn(layer) }.map(\.rect)
        return layout.copper.segmentBase(id, at: point, pads: pads)
    }

    /// One frame of a plain drag on a trace segment: the segment slides
    /// sideways by the part of `delta` (mm) at right angles to it, snapped
    /// to the trace grid, and the rest of the trace follows (`TraceRoute.slide`).
    func dragTraceSegment(_ base: TraceSegmentBase, by delta: CGSize) {
        let grid = board.traceGrid
        editCopper("Move Segment") { $0.dragSegment(base, by: delta, grid: grid) }
    }

    /// The end of a segment drag: the trace joins what it now touches.
    func settleTraceSegment(_ base: TraceSegmentBase) {
        editCopper("Move Segment") { $0.join(base.id) }
    }

    /// What a Select press at `point` (board mm) picks: a text, else a via,
    /// else a trace (within `tolerance` of its edge), else a footprint; Top
    /// before Bottom within each. Only drawn items count.
    func boardItem(at point: CGPoint, tolerance: CGFloat,
                   visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        if let text = text(at: point, visible: visible) { return .text(text.id) }
        if let via = via(at: point, visible: visible) { return .via(via.id) }
        if let hole = hole(at: point, visible: visible) { return .hole(hole.id) }
        if let trace = trace(at: point, tolerance: tolerance, visible: visible) { return .trace(trace.id) }
        if let chip = footprint(at: point, visible: visible), chip.board != nil { return .footprint(chip.id) }
        return pourItem(at: point, tolerance: tolerance, visible: visible)
    }

    /// Sets or clears a part's load file: an EEPROM's contents, or a
    /// simulated microcontroller's firmware (plan-62-H). Does nothing for
    /// other parts. The memory and processor windows call it only while the
    /// simulation is stopped.
    func setMemoryFile(_ id: UUID, _ file: MemoryFile?) {
        recordEdit(.schematic, "Change Load File") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.hasLoadFile else { return }
            chips[index].memoryFile = file
        }
    }

    /// Flips a switch position (1-based) between ON and OFF. Returns the new
    /// state, or nil when the part is not a switch or has no such position,
    /// or when turning it ON would put a voltage out of range on a part (the
    /// switch stays OFF and `editorMessage` says why).
    @discardableResult
    /// Presses or releases a tactile switch (plan-27-J). Nil for any other
    /// chip, or when a press would close a contact the voltage check refuses
    /// (`editorMessage` says why); otherwise the new pressed state. Releasing
    /// never checks. Not undoable and does not mark the document changed:
    /// the pressed state is never saved.
    func setButton(_ id: UUID, pressed: Bool) -> Bool? {
        guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].part.isTactileSwitch,
              !chips[index].switchesOn.isEmpty else { return nil }
        if pressed, !chips[index].part.isNormallyClosed, !chips[index].isPressed, let library {
            var candidate = chips
            candidate[index].switchesOn[0] = true
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                 buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                return nil
            }
        }
        if pressed { editorMessage = nil }
        chips[index].switchesOn[0] = pressed
        return pressed
    }

    /// Every switch's positions (DIP and tactile), by part, for
    /// `restoreSwitches` to put back.
    var switchPositions: [UUID: [Bool]] {
        Dictionary(uniqueKeysWithValues: chips.lazy.filter { !$0.switchesOn.isEmpty }.map { ($0.id, $0.switchesOn) })
    }

    /// Puts switches back to `saved` (from `switchPositions`), for example
    /// when play mode ends: flips made while the simulation ran are input to
    /// that run, not edits. Parts not in `saved`, or whose position count
    /// changed, are left alone.
    func restoreSwitches(_ saved: [UUID: [Bool]]) {
        for index in chips.indices {
            guard let positions = saved[chips[index].id],
                  positions.count == chips[index].switchesOn.count,
                  positions != chips[index].switchesOn else { continue }
            chips[index].switchesOn = positions
        }
    }

    /// Releases every pressed tactile switch (play mode ended).
    func releaseAllButtons() {
        for index in chips.indices where chips[index].isPressed {
            chips[index].switchesOn[0] = false
        }
    }

    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        return recordEdit(.schematic, "Flip Switch") { () -> Bool? in
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].switchesOn.indices.contains(position - 1) else { return nil }
            if !chips[index].switchesOn[position - 1], let library {
                var candidate = chips
                candidate[index].switchesOn[position - 1] = true
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                     buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                    return nil
                }
            }
            editorMessage = nil
            chips[index].switchesOn[position - 1].toggle()
            return chips[index].switchesOn[position - 1]
        }
    }

    /// Moves a slide switch's knob to position `state` (1-based): its contacts
    /// become exactly those the position closes. Returns the position, or nil
    /// when the chip is not a slide switch, has no such position, or the move
    /// would close a contact the voltage check refuses (the knob stays and
    /// `editorMessage` says why). Outside play mode a recorded edit ("Move
    /// Switch"); in play mode an input, put back by `restoreSwitches`.
    @discardableResult
    func setSlideState(_ id: UUID, to state: Int) -> Int? {
        return recordEdit(.schematic, "Move Switch") { () -> Int? in
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let slide = chips[index].part.slideStates,
                  (1...slide.positions).contains(state) else { return nil }
            let contacts = chips[index].part.slideContacts
            let closed = PlacedChip.contacts(mask: slide.masks[state - 1], count: contacts)
            if closed.indices.contains(where: { closed[$0] && !chips[index].switchesOn[$0] }), let library {
                var candidate = chips
                candidate[index].switchesOn = closed
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                     buses: buses, labels: netLabels,
                                                     known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                    return nil
                }
            }
            editorMessage = nil
            if let before = chips[index].slidePosition, before != state {
                chips[index].slideDirection = state > before ? 1 : -1
            }
            chips[index].switchesOn = closed
            return state
        }
    }

    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let position = chipIndex[ref.chipID] else { return nil }
        let chip = chips[position]
        guard let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
        return chip.tip(of: slot)
    }

    /// Per chip, the pins (`ChipPin.index`) a wire ends on. The schematic
    /// leaves out those pins' numbers.
    var connectedPins: [UUID: Set<Int>] {
        sheet.wiring.connectedPins
    }

    /// Tip position of every pin on every placed chip and bus port.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, index: slot.pin.index))
            }
        } + busPinRefs.compactMap { busPinTip($0) } + netLabels.map(\.tip)
    }

    /// Every pin on every bus port.
    var busPinRefs: [BusPinRef] {
        sheet.busNetwork.busPinRefs
    }

    /// Where a wire meets a bus port pin; nil when the port is gone or the
    /// wire isn't ticked on it.
    func busPinTip(_ ref: BusPinRef) -> CGPoint? {
        sheet.busNetwork.busPinTip(ref)
    }

    /// Where the sheet's wire ends on part pins, bus port pins and net
    /// labels are now, for the sheet's values (plan-68-D).
    var tips: PinTips {
        PinTips(pin: { [self] in pinTip($0) }, busPin: { [self] in busPinTip($0) }, label: { [self] in netLabelTip($0) })
    }

    /// The bus port pin whose tip is within `radius` of `point`.
    func busPin(at point: CGPoint, radius: CGFloat = 8) -> BusPinRef? {
        busPinRefs.reversed().first { ref in
            busPinTip(ref).map { Self.distance(from: point, to: $0) <= radius } ?? false
        }
    }

    /// The pin whose tip is within `radius` of `point`.
    func pin(at point: CGPoint, radius: CGFloat = 8) -> PinRef? {
        for chip in chips.reversed() {
            for slot in chip.layout.slots {
                let ref = PinRef(chipID: chip.id, index: slot.pin.index)
                if let tip = pinTip(ref), Self.distance(from: point, to: tip) <= radius {
                    return ref
                }
            }
        }
        return nil
    }

    /// Where a wire leaves or enters one of its ends.
    func endTip(_ end: WireEnd) -> CGPoint? {
        sheet.wiring.endTip(end, tips)
    }

    /// The point a line would start or end at.
    func anchorTip(_ anchor: LineAnchor) -> CGPoint? {
        sheet.wiring.anchorTip(anchor, tips)
    }

    /// The axis a wire must leave or enter `anchor` along: sideways for a
    /// pin on a part's left or right, up/down for one on its top or bottom,
    /// and across the line for a bus port pin. Nil for a point on a wire.
    func anchorAxis(_ anchor: LineAnchor) -> RouteAxis? {
        switch anchor {
        case let .pin(ref):
            guard let chip = chips.first(where: { $0.id == ref.chipID }),
                  let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
            switch slot.side {
            case .left, .right: return .horizontal
            case .top, .bottom: return .vertical
            }
        case let .busPin(ref):
            guard let (bus, port) = port(ref.portID), let line = bus.line(port.line) else { return nil }
            return line.isHorizontal ? .vertical : .horizontal
        case .label:
            return .horizontal
        case .point:
            return nil
        }
    }

    /// What a line can start or end on near `point`: a part pin if one is
    /// within `pinRadius`, then a bus port pin, otherwise the grid point
    /// nearest `point` when an existing wire passes through it and it is
    /// within `gridRadius`. Never a point on a bus line.
    func lineAnchor(at point: CGPoint, pinRadius: CGFloat, gridRadius: CGFloat) -> LineAnchor? {
        if let ref = pin(at: point, radius: pinRadius) {
            return .pin(ref)
        }
        if let ref = busPin(at: point, radius: pinRadius) {
            return .busPin(ref)
        }
        if let id = netLabelTip(at: point, radius: pinRadius) {
            return .label(id)
        }
        let grid = SchematicGrid.snap(point)
        guard Self.distance(from: point, to: grid) <= gridRadius,
              segmentHit(at: grid, tolerance: 0.5) != nil else {
            return nil
        }
        return .point(grid)
    }

    // MARK: Wires

    /// Full route of a wire, from one end's tip to the other.
    func path(forWire id: UUID) -> [CGPoint] {
        sheet.wiring.path(forWire: id, tips)
    }

    /// `path(forWire:)` for a wire already in hand: no search of `wires`, so
    /// a loop over every wire stays linear.
    func path(for wire: Wire) -> [CGPoint] {
        sheet.wiring.path(for: wire, tips)
    }

    var wirePaths: [[CGPoint]] {
        let wiring = sheet.wiring, tips = tips
        return wiring.wires.map { wiring.path(for: $0, tips) }
    }

    /// Connects two anchors with an orthogonal route. A grid point on an existing
    /// wire becomes a junction: that wire is split there and the new wire branches
    /// from it.
    ///
    /// Refused, with nothing changed, when the wire would put a voltage out of
    /// range on a part (a problem the drawing doesn't already have). Returns
    /// why, which is also `editorMessage`; nil otherwise.
    func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]? = nil) -> String? {
        return edit(.schematic, "Add Wire") { scope -> String? in
            let tips = tips, wiring = sheet.wiring
            guard start != end,
                  let a = wiring.anchorTip(start, tips), let b = wiring.anchorTip(end, tips), a != b else {
                return nil
            }
            // The check runs on stand-in ends before anything changes: resolving a
            // point on a wire for real splits that wire.
            if let library, let standInStart = wiring.standIn(for: start, tips),
               let standInEnd = wiring.standIn(for: end, tips) {
                let candidate = wires + [Wire(start: standInStart, end: standInEnd, interior: [])]
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate,
                                                     buses: buses, labels: netLabels,
                                                     known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    return editorMessage
                }
            }
            editorMessage = nil
            let first = anchorAxis(start), last = anchorAxis(end)
            if sheet.updateWiring(scope, { $0.connect(start, end, path: path, first: first, last: last, tips) }) {
                pruneSelection()
            }
            return nil
        }
    }

    /// Whether a wire end sits on a pin of the given part.
    private func attaches(_ end: WireEnd, to chipID: UUID) -> Bool {
        if case let .pin(ref) = end {
            return ref.chipID == chipID
        }
        return false
    }

    /// Drops junctions that no wire ends at any more. Every removal of
    /// parts or wires ends here, so it also drops what left the selection.
    private func pruneJunctions(_ scope: EditScope) {
        let tips = tips
        sheet.updateWiring(scope) { $0.pruneJunctions(tips) }
        pruneSelection()
    }

    /// `Wiring.loosen` with the document's tips: run it while the pins that
    /// go still resolve.
    private func loosen(_ scope: EditScope, where matches: (WireEnd) -> Bool) {
        let tips = tips
        sheet.updateWiring(scope) { $0.loosen(where: matches, tips) }
    }

    /// `Wiring.settle` with the document's tips.
    private func settle(_ scope: EditScope, where touches: (WireEnd) -> Bool) {
        let tips = tips
        sheet.updateWiring(scope) { $0.settle(where: touches, tips) }
    }

    /// Re-fits the wires on a bus port's pins after the port changed (pins
    /// added, removed or moved across), as `settleWires(attachedTo:)` does for
    /// a part.
    func settleWires(attachedToPort portID: UUID) {
        edit(.schematic, "Move") { scope in
            settle(scope) { BusNetwork.endsOn($0, port: portID) }
        }
    }

    /// Re-fits the wires on every port of a bus, when a bus drag ends, and
    /// connects loose ends its ports' pins landed on.
    func settleWires(attachedToBus busID: UUID) {
        edit(.schematic, "Move") { scope in
            let ports = bus(busID)?.ports.map(\.id) ?? []
            for port in ports {
                settle(scope) { BusNetwork.endsOn($0, port: port) }
            }
            adoptLooseEnds(ports: Set(ports))
        }
    }

    /// Whether either end of the wire is on a loose junction (plan-30).
    func hasLooseEnd(_ wire: Wire) -> Bool {
        sheet.wiring.hasLooseEnd(wire)
    }

    /// Whether a junction gets a dot: always for a plain junction; for a
    /// loose one only where 3 or more wire ends meet, since a dot reads as a
    /// connection.
    func showsDot(_ junction: Junction) -> Bool {
        sheet.wiring.showsDot(junction)
    }

    /// Connects loose ends to pins after a drop (plan-30). A loose junction
    /// joins a pin when its point is that pin's tip and either the junction
    /// or the pin's part or port is in the dropped set; every wire end on the
    /// junction moves to the pin, and the junction goes away. Each candidate
    /// goes through the same voltage check as `connect`; one that fails stays
    /// loose and sets `editorMessage`. When two pins share a tip, the first
    /// part in `chips` order wins, then bus order. Removals never call this.
    func adoptLooseEnds(chips chipIDs: Set<UUID> = [], ports portIDs: Set<UUID> = [],
                        junctions junctionIDs: Set<UUID> = [], labels labelIDs: Set<UUID> = []) {
        edit(.schematic, "Connect") { scope in
            guard junctions.contains(where: \.isLoose) else { return }
            @MainActor func tips(of placed: [PlacedChip], ports includes: (BusPort) -> Bool,
                                 labels: [NetLabel]) -> [CGPoint: WireEnd] {
                var map: [CGPoint: WireEnd] = [:]
                for chip in placed {
                    for slot in chip.layout.slots {
                        let ref = PinRef(chipID: chip.id, index: slot.pin.index)
                        if let tip = pinTip(ref), map[tip] == nil { map[tip] = .pin(ref) }
                    }
                }
                for bus in buses {
                    for port in bus.ports where includes(port) {
                        for wire in bus.pinWires(of: port) {
                            let ref = BusPinRef(portID: port.id, wireID: wire.id)
                            if let tip = busPinTip(ref), map[tip] == nil { map[tip] = .busPin(ref) }
                        }
                    }
                }
                for label in labels where map[label.tip] == nil {
                    map[label.tip] = .label(label.id)
                }
                return map
            }
            let dropped = tips(of: chips.filter { chipIDs.contains($0.id) }, ports: { portIDs.contains($0.id) },
                               labels: netLabels.filter { labelIDs.contains($0.id) })
            // A dragged loose end may land on any pin.
            let every = junctionIDs.isEmpty ? [:] : tips(of: chips, ports: { _ in true }, labels: netLabels)
            let adopted = sheet.updateWiring(scope) { wiring in
                wiring.adoptLooseEnds(dropped: dropped, every: every, draggedJunctions: junctionIDs) { before, after in
                    guard let library else { return true }
                    // The problems the drawing has before this candidate: an
                    // earlier adoption in this loop may have changed them.
                    let known = before == wires ? currentVoltageIssues(library)
                        : VoltageCheck.issues(chips: chips, wires: before, buses: buses, labels: netLabels, library: library)
                    guard let issue = VoltageCheck.firstNew(before: chips, before, after: chips, after,
                                                            buses: buses, labels: netLabels,
                                                            known: known, library: library) else { return true }
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    return false
                }
            }
            if adopted { pruneSelection() }
        }
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        edit(.schematic, "Move") { scope in
            settle(scope) { attaches($0, to: chipID) }
        }
    }

    /// Stores a new route for a wire. The pin tips stay where they are.
    func setWirePath(_ id: UUID, _ path: [CGPoint]) {
        edit(.schematic, "Move Wire") { scope in
            sheet.updateWiring(scope) { $0.setPath(id, path) }
        }
    }

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        sheet.wiring.nodeHit(at: point, tolerance: tolerance, tips)
    }

    /// A segment of a wire, if `point` is on one. `index` is the segment's
    /// start point within the wire's path.
    func segmentHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        sheet.wiring.segmentHit(at: point, tolerance: tolerance, tips)
    }

    /// Double-click on a wire: adds a bend at that point.
    func insertNode(at point: CGPoint, tolerance: CGFloat = 6) {
        edit(.schematic, "Add Bend") { scope in
            let tips = tips
            sheet.updateWiring(scope) { $0.insertNode(at: point, tolerance: tolerance, tips) }
        }
    }

    // MARK: Buses

    func bus(_ id: UUID) -> Bus? {
        sheet.busNetwork.bus(id)
    }

    /// Whether no line end of one bus touches a line of another.
    static func busesAreApart(_ buses: [Bus]) -> Bool {
        BusNetwork.areApart(buses)
    }

    /// What a Bus tool drag from `start` to `end` would make (both snapped).
    /// Changes nothing; the canvas previews it.
    func busDraft(from start: CGPoint, to end: CGPoint) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, from: SchematicGrid.snap(start), to: SchematicGrid.snap(end))
    }

    /// What the Bus tool would make of a drawn route: the first of
    /// `candidates` (grid paths, as `RouteDraft.busCandidates` gives) the
    /// bus rules accept.
    func busDraft(candidates: [[CGPoint]]) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, candidates: candidates)
    }

    /// The Bus tool: draws a line or an L from `start` to `end`, as a new bus
    /// or joined to the bus it starts or ends on (a T, an L, or a longer
    /// line). Selects that bus and returns its id. A refused drag changes
    /// nothing, sets `editorMessage`, and returns nil.
    @discardableResult
    func addBus(from start: CGPoint, to end: CGPoint) -> UUID? {
        return edit(.schematic, "Add Bus") { scope -> UUID? in
            applyBusDraft(busDraft(from: start, to: end), scope)
        }
    }

    /// The Bus tool along a drawn route: `addBus(from:to:)` for the first of
    /// `candidates` the bus rules accept.
    @discardableResult
    func addBus(candidates: [[CGPoint]]) -> UUID? {
        return edit(.schematic, "Add Bus") { scope -> UUID? in
            applyBusDraft(busDraft(candidates: candidates), scope)
        }
    }

    private func applyBusDraft(_ result: Result<BusDraft, BusJoinRefusal>, _ scope: EditScope) -> UUID? {
        switch result {
        case let .failure(refusal):
            if let message = refusal.message { editorMessage = message }
            return nil
        case let .success(draft):
            if let message = sheet.updateBuses(scope, { $0.place(draft) }) {
                editorMessage = message
                return nil
            }
            if let gone = draft.absorbedIndex {
                absorb(buses[gone], wireMap: draft.wireMap, scope)
                sheet.updateBuses(scope) { $0.buses.remove(at: gone) }
            }
            editorMessage = nil
            selectedBusID = draft.bus.id
            return draft.bus.id
        }
    }

    /// After the Bus tool joined `gone` into another bus: wires on `gone`'s port
    /// pins move to the pin for the wire it became, and are re-fitted (the
    /// pins may have moved along the port).
    private func absorb(_ gone: Bus, wireMap: [UUID: UUID], _ scope: EditScope) {
        let portIDs = Set(gone.ports.map(\.id))
        sheet.updateWiring(scope) { wiring in
            wiring.remapEnds { end in
                guard case let .busPin(ref) = end, portIDs.contains(ref.portID),
                      let wireID = wireMap[ref.wireID] else { return end }
                return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
            }
        }
        for portID in portIDs {
            settle(scope) { BusNetwork.endsOn($0, port: portID) }
        }
    }

    /// Sets how many wires a bus carries, clamped to 2–999. New wires get
    /// fresh ids and default names; removing drops wires from the end. It
    /// can't drop a wire a port uses: that is refused with a message.
    func setBusWireCount(_ id: UUID, _ count: Int) -> String? {
        return edit(.schematic, "Change Bus Width") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setWireCount(id, count, edge: edge) }
        }
    }

    /// Renames one wire of a bus. Another wire of the bus may have the same
    /// name (the wire list warns); each wire is still its own net. Returns
    /// why when the name is refused, and changes nothing then.
    func renameBusWire(bus id: UUID, wire wireID: UUID, to name: String) -> String? {
        return edit(.schematic, "Rename Bus Wire") { scope -> String? in
            sheet.updateBuses(scope) { $0.renameWire(bus: id, wire: wireID, to: name) }
        }
    }

    /// Sets a bus's wires to `names`, in order (the Bus panel's Import).
    /// Wires keep their places and ids (so ports and the wires on their pins
    /// stay) and are renamed; wires are added or dropped at the end. One
    /// undo step. Refused, with why, and nothing changes, when the bus type
    /// sets the wires, a name is invalid, the count is out of range, a port
    /// uses a wire that would go, or the card edge uses the bus and the
    /// count would change.
    func setBusWireNames(_ id: UUID, _ names: [String]) -> String? {
        return edit(.schematic, "Import Bus Wires") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setWireNames(id, names, edge: edge) }
        }
    }

    /// Moves one wire of a bus to place `to` (0-based, in the order after
    /// the move), keeping its id and name, so its net and the wires on its
    /// port pins go with it. Linked buses move the wire at the same place
    /// the same way, so wire i of each stays one net. Ports show their pins
    /// in bus order, so their wires are re-fitted. Refused, with why, while
    /// the bus type sets the wires.
    func moveBusWire(bus id: UUID, wire wireID: UUID, to place: Int) -> String? {
        return edit(.schematic, "Reorder Bus Wires") { scope -> String? in
            let moved = sheet.updateBuses(scope) { $0.moveWire(bus: id, wire: wireID, to: place) }
            for port in moved.ports {
                settle(scope) { BusNetwork.endsOn($0, port: port) }
            }
            return moved.message
        }
    }

    /// The bus the board's card edge uses, while the board is a card and
    /// the bus has as many wires as the card has pins.
    var edgeBusID: UUID? {
        guard board.type.isCard, let id = board.edgeBus, let bus = bus(id),
              bus.wires.count == board.pinCount else { return nil }
        return id
    }

    /// The buses a card edge can use: those with as many wires as the card
    /// has pins. Only the count matters, not the names.
    var edgeBusChoices: [Bus] {
        guard board.type.isCard else { return [] }
        return buses.filter { $0.wires.count == board.pinCount }
    }

    /// Chooses the bus the card edge uses, or none. A bus with the wrong
    /// wire count is ignored.
    func setEdgeBus(_ id: UUID?) {
        if let id, !edgeBusChoices.contains(where: { $0.id == id }) { return }
        editBoard("Change Card Bus") { $0.edgeBus = id }
    }

    /// The bus that renaming bus `id` to `name` would link it with: another
    /// bus of that name (ignoring case) not linked with it already. Nil when
    /// the rename links nothing new.
    func busLinkTarget(_ id: UUID, named name: String) -> Bus? {
        sheet.busNetwork.linkTarget(id, named: name)
    }

    /// Renames a bus. Refused, with why, when the name is empty. A bus
    /// linked with others (same name) leaves them; a new case for the same
    /// name renames them all.
    ///
    /// Another bus's name links the two (`busLinkTarget`), and only with
    /// `link`: this bus takes that one's type and wires (its wires keep
    /// their places and ids; they are renamed, added or dropped at the end)
    /// and its exact name. Without `link` that is refused. Refused too when
    /// a port uses a wire the other bus doesn't have, or this bus is the
    /// card edge and the wire count would change.
    func setBusName(_ id: UUID, _ name: String, link: Bool = false) -> String? {
        return edit(.schematic, "Rename Bus") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setName(id, name, link: link, edge: edge) }
        }
    }

    /// Sets what a bus carries. A card type makes the wires its pins, in
    /// order: existing wires keep their places (and ids, so ports and their
    /// wire ends stay) and are renamed, and wires are added or dropped at
    /// the end. Custom keeps the wires as they are, to edit freely.
    /// Refused, with why, when a port uses a wire the card has no pin for,
    /// or the card has fewer than 2 pins.
    func setBusCard(_ id: UUID, _ card: BusCard) -> String? {
        return edit(.schematic, "Change Bus Type") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setCard(id, card, edge: edge) }
        }
    }

    // MARK: Bus ports

    /// The bus and index of a port.
    func portLocation(_ portID: UUID) -> (bus: Int, port: Int)? {
        sheet.busNetwork.portLocation(portID)
    }

    /// A port and the bus it is on.
    func port(_ portID: UUID) -> (bus: Bus, port: BusPort)? {
        sheet.busNetwork.port(portID)
    }

    /// The Bus Port tool: adds an empty port where `point` is on a bus line,
    /// and selects it. Refused (with `editorMessage` set, and returned) on or
    /// near a line end or a T, or when one slot doesn't fit there.
    @discardableResult
    func addPort(at point: CGPoint, tolerance: CGFloat = 6) -> String? {
        return edit(.schematic, "Add Port") { scope -> String? in
            switch sheet.updateBuses(scope, { $0.addPort(at: point, tolerance: tolerance) }) {
            case nil:
                return nil
            case let .failure(refusal)?:
                editorMessage = refusal.message
                return editorMessage
            case let .success(placed)?:
                editorMessage = nil
                selectedPortID = placed.id
                return nil
            }
        }
    }

    /// Sets which bus wires are pins on a port (ticks, Select All, Deselect
    /// All). More pins than fit is refused with a message and changes
    /// nothing; a port that grows may move left (up) to fit. The wires on
    /// unticked pins stay, with loose ends.
    func setPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        return edit(.schematic, "Change Port Pins") { scope -> String? in
            switch sheet.busNetwork.portWiresFit(portID, ids) {
            case nil:
                return nil
            case let .failure(refusal)?:
                return refusal.message
            case let .success(fit)?:
                if !fit.removing.isEmpty {
                    loosen(scope) { BusNetwork.endsOn($0, port: portID, wireIDs: fit.removing) }
                    pruneJunctions(scope)
                }
                sheet.updateBuses(scope) { $0.setPortWires(portID, ids, start: fit.start) }
                // Pins after an added one shift by a pitch; re-fit their wires.
                settle(scope) { BusNetwork.endsOn($0, port: portID) }
                return nil
            }
        }
    }

    /// Sets the side a port's pins come out of, if it suits the line. Ports
    /// on opposite sides may share a place, so a port on the new side that is
    /// in the way refuses the switch with a message.
    @discardableResult
    func setPortSide(_ portID: UUID, _ side: ChipLayout.Side) -> String? {
        return edit(.schematic, "Change Port Side") { scope -> String? in
            switch sheet.updateBuses(scope, { $0.setPortSide(portID, side) }) {
            case .unchanged:
                return nil
            case let .refused(message):
                return message
            case .changed:
                settle(scope) { BusNetwork.endsOn($0, port: portID) }
                return nil
            }
        }
    }

    /// Moves a port along its line to `start`. Stores it only if the fit
    /// rule keeps it unchanged (on its line, clear of the other ports and the
    /// Ts). Returns whether it did.
    @discardableResult
    func slidePort(_ portID: UUID, to start: CGFloat) -> Bool {
        return edit(.schematic, "Move Port") { scope -> Bool in
            sheet.updateBuses(scope) { $0.slidePort(portID, to: start) }
        }
    }

    /// One frame of a port slide: `base` (the start at the press) plus
    /// `along` (the drag along the line), snapped to the grid and clamped to
    /// where the port may go. It never leaves its line or passes a line
    /// end, another port or a T.
    func slidePort(_ portID: UUID, from base: CGFloat, by along: CGFloat) {
        recordEdit(.schematic, "Move Port") {
            guard let (bus, port) = port(portID), let range = bus.slideRange(of: port) else { return }
            let wanted = SchematicGrid.snap(base + along)
            slidePort(portID, to: min(max(wanted, range.lowerBound), range.upperBound))
        }
    }

    /// The port whose body is under `point` (topmost first).
    func portHit(at point: CGPoint, tolerance: CGFloat = 0) -> UUID? {
        sheet.busNetwork.portHit(at: point, tolerance: tolerance)
    }

    /// The one way a drag changes a bus. Stores `lines` only if the bus stays
    /// valid, keeps the same joints (points may move), keeps every line's
    /// direction, stays apart from other buses, and every port still fits
    /// where it is (its line long enough, no T slid into it). Ports ride
    /// along with their line. Returns whether it stored the lines.
    @discardableResult
    func setBusLines(_ id: UUID, _ lines: [BusLine]) -> Bool {
        return edit(.schematic, "Move Bus") { scope -> Bool in
            sheet.updateBuses(scope) { $0.setLines(id, lines) }
        }
    }

    /// One frame of a bus drag. `step(offset)` gives the lines for a drag of
    /// `offset` from the press (against the press-time bus). The drag walks
    /// from `done` (the last offset stored) toward `target` one grid step at
    /// a time, storing each step through `setBusLines`, and stops at the
    /// first refused one. So a drag never jumps over something it may not
    /// pass, such as a T end over a port. Returns the offset reached.
    @discardableResult
    func dragBus(_ id: UUID, from done: CGSize, to target: CGSize, step: (CGSize) -> [BusLine]?) -> CGSize {
        return recordEdit(.schematic, "Move Bus") { () -> CGSize in
            let dx = target.width - done.width
            let dy = target.height - done.height
            let count = Int((max(abs(dx), abs(dy)) / SchematicGrid.spacing).rounded(.up))
            var reached = done
            guard count > 0 else { return done }
            for k in 1...count {
                let t = CGFloat(k) / CGFloat(count)
                let offset = CGSize(width: SchematicGrid.snap(done.width + dx * t),
                                    height: SchematicGrid.snap(done.height + dy * t))
                guard offset != reached else { continue }
                guard let lines = step(offset), setBusLines(id, lines) else { break }
                reached = offset
            }
            return reached
        }
    }

    /// The bus line under `point`, if any (topmost bus first).
    func busHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID)? {
        sheet.busNetwork.busHit(at: point, tolerance: tolerance)
    }

    /// The bus line end under `point`, if any. A corner is two ends at one
    /// point; either is returned, and the drag moves both lines.
    func busEndHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID, end: BusEnd)? {
        sheet.busNetwork.busEndHit(at: point, tolerance: tolerance)
    }

    // MARK: Removing ports and bus lines

    /// A removal waiting for the user to confirm: it would take more than the
    /// thing clicked (wires on port pins, or a line's ports).
    struct PendingRemoval: Identifiable {
        let id: UUID
        let message: String
        fileprivate let action: Action

        fileprivate init(message: String, action: Action) {
            (id, self.message, self.action) = (UUID(), message, action)
        }

        fileprivate enum Action {
            case port(UUID)
            case busLine(bus: UUID, line: UUID)
            case untick(port: UUID, keeping: Set<UUID>)
            /// A selection with a part drawn as its units (plan-67): the
            /// items, and what `deleteSchematicSelection` was asked.
            case parts(items: Set<SchematicItem>, name: String, includingTitleBlock: Bool)
        }
    }

    /// Set while a confirm dialog is up. Erasing does nothing meanwhile, so
    /// one eraser sweep raises at most one dialog.
    private(set) var pendingRemoval: PendingRemoval?

    func confirmPendingRemoval() {
        if case let .parts(items, name, includingTitleBlock)? = pendingRemoval?.action {
            pendingRemoval = nil
            if items != schematicSelection.items { schematicSelection.set(items) }
            deleteSchematicSelection(named: name, includingTitleBlock: includingTitleBlock, confirmed: true)
            return
        }
        edit(.schematic, "Delete") { scope in
            guard let pending = pendingRemoval else { return }
            pendingRemoval = nil
            switch pending.action {
            case let .port(id): removePort(id, scope)
            case let .busLine(bus, line): removeBusLine(bus: bus, line: line, scope)
            case let .untick(port, keeping): _ = setPortWires(port, keeping)
            case .parts: break
            }
        }
    }

    /// What the confirm asks before parts drawn as several symbols go: a
    /// click on one gate would otherwise take the whole IC unannounced.
    /// `split` are those parts, of `count` selected parts.
    static func removalQuestion(split: [PlacedChip], of count: Int) -> String {
        if count == 1, let chip = split.first {
            return "Delete \(chip.reference) and all \(chip.unitPlacements.count) of its symbols?"
        }
        let names = split.map(\.reference).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
        let shown = names.prefix(3)
        var list = shown.dropLast().joined(separator: ", ")
        if let last = shown.last { list = list.isEmpty ? last : "\(list) and \(last)" }
        if names.count > shown.count { list = shown.joined(separator: ", ") + " and \(names.count - shown.count) more" }
        let verb = names.count == 1 ? "is" : "are"
        return "Delete the \(count) selected parts? \(list) \(verb) drawn as several symbols; all of them go."
    }

    func cancelPendingRemoval() {
        pendingRemoval = nil
    }

    /// How many wires end on the pins of a port that unticking `ids` (or,
    /// with nil, deleting the port) would remove.
    func wiresOnPins(port portID: UUID, removing ids: Set<UUID>? = nil) -> Int {
        wires.filter {
            BusNetwork.endsOn($0.start, port: portID, wireIDs: ids) || BusNetwork.endsOn($0.end, port: portID, wireIDs: ids)
        }.count
    }

    /// The port panel's ticks: like `setPortWires`, but unticking a pin that
    /// has wires asks first (through `pendingRemoval`) and returns nil.
    func requestPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        return recordEdit(.schematic, "Change Port Pins") { () -> String? in
            guard let (_, port) = port(portID) else { return nil }
            let removing = port.wireIDs.subtracting(ids)
            let count = wiresOnPins(port: portID, removing: removing)
            if count > 0 {
                pendingRemoval = PendingRemoval(
                    message: "Remove \(Self.counted(removing.count, "pin"))? The \(Self.counted(count, "wire")) on them stay, with loose ends.",
                    action: .untick(port: portID, keeping: ids))
                return nil
            }
            return setPortWires(portID, ids)
        }
    }

    /// Removes a port. The wires on its pins stay, with loose ends.
    private func removePort(_ portID: UUID, _ scope: EditScope) {
        guard portLocation(portID) != nil else { return }
        loosen(scope) { BusNetwork.endsOn($0, port: portID) }
        sheet.updateBuses(scope) { $0.removePort(portID) }
        pruneJunctions(scope)
        if selectedPortID == portID { selectedPortID = nil }
    }

    /// Removes one bus line, with its ports; the wires on their pins stay,
    /// with loose ends (`BusNetwork.removeLine`): a bus with no lines left
    /// goes, and the pieces a split leaves become buses of their own, the
    /// wire ends on their ports moving to the new wire ids. The card edge's
    /// bus keeps its last line.
    private func removeBusLine(bus id: UUID, line lineID: UUID, _ scope: EditScope) {
        guard let bus = bus(id) else { return }
        if bus.id == edgeBusID, bus.lines.count == 1 {
            editorMessage = BusNetwork.edgeBusUndeletable(bus)
            return
        }
        // Loosen first, while the removed ports' pin tips still resolve.
        for port in bus.ports where port.line == lineID {
            loosen(scope) { BusNetwork.endsOn($0, port: port.id) }
            if selectedPortID == port.id { selectedPortID = nil }
        }
        defer { pruneJunctions(scope) }
        guard let remaps = sheet.updateBuses(scope, { $0.removeLine(bus: id, line: lineID) }) else {
            if selectedBusID == id { selectedBusID = nil }
            return
        }
        sheet.updateWiring(scope) { wiring in
            for remap in remaps {
                wiring.remapEnds { end in
                    guard case let .busPin(ref) = end, remap.ports.contains(ref.portID),
                          let wireID = remap.wires[ref.wireID] else { return end }
                    return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
                }
            }
        }
    }

    /// "1 wire", "3 wires".
    private static func counted(_ n: Int, _ noun: String) -> String {
        "\(n) \(noun)\(n == 1 ? "" : "s")"
    }

    // MARK: Erasing

    /// Removes whatever is under `point`: a block first (its wires stay, with
    /// loose ends where its pins were), then a net label (the same), then
    /// a bus port, then a wire, then a bus line, then a text block. A port with
    /// wires on its pins, or a bus line with ports, asks first (`pendingRemoval`); while
    /// that is up, nothing is erased.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        edit(.schematic, "Delete") { scope in
            guard pendingRemoval == nil else { return }
            if let hit = chip(at: point) {
                if hit.isSplit {
                    // Every symbol of the part would go: ask first (plan-67).
                    schematicSelection.replace(with: .chip(hit.id))
                    pendingRemoval = PendingRemoval(
                        message: Self.removalQuestion(split: [hit], of: 1),
                        action: .parts(items: [.chip(hit.id)], name: "Delete", includingTitleBlock: true))
                    return
                }
                loosen(scope) { attaches($0, to: hit.id) }
                chips.removeAll { $0.id == hit.id }
            } else if let hit = netLabel(at: point) {
                deleteNetLabel(hit.id)
            } else if let portID = portHit(at: point) {
                askOrRemovePort(portID, scope)
                return
            } else if let hit = segmentHit(at: point, tolerance: tolerance) {
                sheet.updateWiring(scope) { $0.wires.removeAll { $0.id == hit.wire } }
            } else if let hit = busHit(at: point, tolerance: tolerance), let bus = bus(hit.bus) {
                // The card edge's bus can't go: not its last line.
                if bus.id == edgeBusID, bus.lines.count == 1 {
                    editorMessage = BusNetwork.edgeBusUndeletable(bus)
                    return
                }
                let ports = bus.ports.filter { $0.line == hit.line }
                if ports.isEmpty {
                    removeBusLine(bus: hit.bus, line: hit.line, scope)
                } else {
                    let count = ports.map { wiresOnPins(port: $0.id) }.reduce(0, +)
                    pendingRemoval = PendingRemoval(
                        message: "Delete this bus line and its \(Self.counted(ports.count, "port"))? The \(Self.counted(count, "wire")) on their pins stay, with loose ends.",
                        action: .busLine(bus: hit.bus, line: hit.line))
                }
                return
            } else if let hit = note(at: point) {
                deleteNote(hit.id)
                return
            } else if titleBlock(at: point) {
                // Only switches it off: the information stays.
                documentInfo.titleBlock = nil
                pruneSelection()
                return
            } else {
                return
            }
            pruneJunctions(scope)
        }
    }

    /// Deletes a port, asking first (`pendingRemoval`) when wires are on its pins.
    private func askOrRemovePort(_ portID: UUID, _ scope: EditScope) {
        let count = wiresOnPins(port: portID)
        if count > 0 {
            pendingRemoval = PendingRemoval(
                message: "Delete this port? The \(Self.counted(count, "wire")) on its pins stay, with loose ends.",
                action: .port(portID))
        } else {
            removePort(portID, scope)
        }
    }

    /// ⌘⌫ on the Schematic: deletes the selected parts (their wires stay,
    /// with loose ends, as the Eraser leaves them), wires and text blocks, as
    /// one step; junctions go once no wire ends on them. A selected bus port
    /// is deleted as the Eraser deletes one, asking first when wires are on
    /// its pins. Selected buses go whole, with their ports; the wires on
    /// their pins stay, with loose ends. The card edge's bus stays
    /// (`editorMessage` says why). Returns whether anything was deleted or
    /// asked.
    /// ⌘X passes `name` "Cut" (with no port selected: a port isn't copied).
    /// A selected title block is only switched off (the information stays),
    /// unless `includingTitleBlock` is false: ⌘X doesn't take it, so it stays
    /// shown.
    /// A selection with a part drawn as its units asks first
    /// (`pendingRemoval`), unless `confirmed`: the agent's delete, and the
    /// confirm itself.
    @discardableResult
    func deleteSchematicSelection(named name: String = "Delete", includingTitleBlock: Bool = true,
                                  confirmed: Bool = false) -> Bool {
        guard pendingRemoval == nil else { return false }
        if let portID = selectedPortID {
            edit(.schematic, "Delete") { scope in askOrRemovePort(portID, scope) }
            return true
        }
        var chipIDs: Set<UUID> = [], wireIDs: Set<UUID> = [], noteIDs: Set<UUID> = []
        var labelIDs: Set<UUID> = [], busIDs: Set<UUID> = []
        var hidesTitleBlock = false
        for item in schematicSelection.items {
            switch item {
            case let .chip(id): chipIDs.insert(id)
            case let .wire(id): wireIDs.insert(id)
            case let .note(id): noteIDs.insert(id)
            case let .label(id): labelIDs.insert(id)
            case let .bus(id): busIDs.insert(id)
            case .titleBlock: hidesTitleBlock = includingTitleBlock
            case .junction: break
            }
        }
        var message: String?
        if let edge = edgeBusID, busIDs.contains(edge), let bus = bus(edge) {
            busIDs.remove(edge)
            message = BusNetwork.edgeBusUndeletable(bus)
        }
        guard !chipIDs.isEmpty || !wireIDs.isEmpty || !noteIDs.isEmpty || !labelIDs.isEmpty || !busIDs.isEmpty
            || hidesTitleBlock else {
            if let message { editorMessage = message }
            return false
        }
        let split = chips.filter { chipIDs.contains($0.id) && $0.isSplit }
        if !confirmed, !split.isEmpty {
            pendingRemoval = PendingRemoval(
                message: Self.removalQuestion(split: split, of: chipIDs.count),
                action: .parts(items: schematicSelection.items, name: name, includingTitleBlock: includingTitleBlock))
            return true
        }
        let portIDs = Set(buses.filter { busIDs.contains($0.id) }.flatMap { $0.ports.map(\.id) })
        edit(.schematic, name) { scope in
            sheet.updateWiring(scope) { $0.wires.removeAll { wireIDs.contains($0.id) } }
            loosen(scope) { end in
                switch end {
                case let .label(id): labelIDs.contains(id)
                case let .busPin(ref): portIDs.contains(ref.portID)
                default: chipIDs.contains { attaches(end, to: $0) }
                }
            }
            sheet.updateBuses(scope) { $0.buses.removeAll { busIDs.contains($0.id) } }
            if message != nil { editorMessage = message }
            chips.removeAll { chipIDs.contains($0.id) }
            sheet.updateLabels(scope) { $0.remove(labelIDs) }
            sheet.updateNotes(scope) { $0.remove(noteIDs) }
            if let editing = editingNoteID, noteIDs.contains(editing) { editingNoteID = nil }
            if hidesTitleBlock { documentInfo.titleBlock = nil }
            pruneJunctions(scope)
        }
        return true
    }

    // MARK: Geometry

    nonisolated static func distance(from a: CGPoint, to b: CGPoint) -> CGFloat {
        hypot(a.x - b.x, a.y - b.y)
    }

    nonisolated static func distance(from point: CGPoint, toSegment a: CGPoint, _ b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy

        guard lengthSquared > 0 else {
            return distance(from: point, to: a)
        }

        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return distance(from: point, to: CGPoint(x: a.x + t * dx, y: a.y + t * dy))
    }
}
