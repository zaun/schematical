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
    var ownCopperRevision = 0
    /// Counts every change to `chips`, for `chipPads`.
    @ObservationIgnored var chipsRevision = 0
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
    @ObservationIgnored var keptChipPads: (revision: Int, pads: [[PlacedPad]])?
    @ObservationIgnored var keptVoltageIssues: (revision: Int, issues: Set<VoltageIssue>)?
    @ObservationIgnored var keptChipIndex: (revision: Int, index: [UUID: Int])?

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
    var chipIndex: [UUID: Int] {
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
    var schematicSelection = ItemSelection<SchematicItem>() {
        didSet {
            if !schematicSelection.isEmpty { clearSelection(except: .items) }
            prunePicks()
            mirrorSchematicSelectionToBoard()
        }
    }
    /// What is selected on the PCB page: footprints, board texts and traces.
    /// A part and its footprint are one selection with the Schematic page
    /// (see `mirrorSchematicSelectionToBoard`); the rest is kept apart.
    var boardSelection = ItemSelection<BoardItem>() {
        didSet { mirrorBoardSelectionToSchematic() }
    }

    /// Set while one page's selection is being copied to the other, so the
    /// copy does not copy back.
    var isMirroringSelection = false

    /// The parts selected on the Schematic page.
    var schematicSelectedParts: Set<UUID> {
        Set(schematicSelection.items.compactMap { if case let .chip(id) = $0 { id } else { nil } })
    }

    /// The parts whose footprint is selected on the PCB page.
    var boardSelectedParts: Set<UUID> {
        Set(boardSelection.items.compactMap { if case let .footprint(id) = $0 { id } else { nil } })
    }

    /// Selecting a part on the Schematic page selects its footprint on the
    /// PCB page. When the two pages disagree about which parts are selected,
    /// the PCB selection becomes just those footprints (a part without a
    /// footprint has none). Texts and traces are left alone while the parts
    /// agree.
    func mirrorSchematicSelectionToBoard() {
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
    func mirrorBoardSelectionToSchematic() {
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

    enum SelectionKind { case items, port }

    /// On the Schematic page, the item set (buses included) and a port
    /// exclude each other: clears the one that isn't `kept`.
    func clearSelection(except kept: SelectionKind) {
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

    // MARK: Undo

    /// The document's one undo manager (plan-61-A). One recorded step is one
    /// undo group on it. An `NSDocument` showing this document is given it.
    @ObservationIgnored let undoManager = SchematicUndoManager()
    /// The step on top of the manager's undo stack while typing may still
    /// merge into it (`UndoStep.coalescing`).
    @ObservationIgnored var openStep: UndoStep?
    /// Bumped when the manager's stacks change, so views reading `undoName`
    /// and `redoName` follow (an `UndoManager` is not observable).
    var undoRevision = 0
    /// How many `recordEdit` calls (and an open gesture) are running. Only
    /// the outermost records.
    var recordingDepth = 0
    var recordingStart: DocumentState?
    /// The name of the step: of the outermost edit that changed something,
    /// the first one (a gesture's edits are siblings, the first names it).
    var recordingName: String?
    /// The depth of the edit that gave `recordingName`.
    var recordingNameDepth = 0
    var recordingCoalescing: String?
    /// The outermost call's coalescing key, which wins when it gave one.
    var outerCoalescing: String?
    /// True while the simulation runs: edits are not recorded and undo/redo
    /// do nothing. Set by `SimulationController`.
    var isPlaying = false {
        didSet { undoManager.isLocked = isPlaying }
    }
    /// A mouse drag is being recorded as one step.
    var isGestureOpen = false

    /// The saved part of the document, as a value.
    var state: DocumentState {
        DocumentState(chips: chips, wires: wires, junctions: junctions, buses: buses, notes: notes,
                      netLabels: netLabels, nextReference: nextReference, board: board, texts: texts, traces: traces, vias: vias, holes: holes,
                      pours: pours, keepouts: keepouts, cutouts: cutouts, erc: erc, documentInfo: documentInfo)
    }

    /// Writes a state back (undo, redo), then drops selections and editor
    /// state that point at items no longer there.
    func setState(_ s: DocumentState) {
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

    /// Runs `body` on the document info and writes it back when it changed.
    func updateInfo<T>(_ scope: EditScope, _ body: (inout DocumentInfo) -> T) -> T {
        var value = documentInfo
        defer { if value != documentInfo { documentInfo = value } }
        return body(&value)
    }

    /// The parts as a value (plan-68-F).
    var parts: PartList { PartList(chips: chips, nextReference: nextReference) }

    /// Runs `body` on the parts and writes back what it changed. Only a
    /// recorded edit (or the undo code) has the scope.
    func updateParts<T>(_ scope: EditScope, _ body: (inout PartList) throws -> T) rethrows -> T {
        var value = parts
        defer {
            if value.replacedParts || value.chips != chips { chips = value.chips }
            if value.nextReference != nextReference { nextReference = value.nextReference }
        }
        return try body(&value)
    }

    /// Switch inputs while the simulation plays or as it stops (a button
    /// held, positions put back): not edits, so not recorded.
    func updateInputs(_ body: (inout PartList) -> Void) {
        updateParts(EditScope(), body)
    }

    /// A card-edge bus link that no longer counts goes (see `finishRecording`).
    func dropStaleEdgeBus(_ scope: EditScope) {
        if board.edgeBus != nil, edgeBusID == nil { layout.updateSettings(scope) { $0.edgeBus = nil } }
    }

    func startRecording(coalescing: String?) {
        recordingStart = state
        recordingName = nil
        recordingCoalescing = nil
        outerCoalescing = coalescing
    }

    func finishRecording() {
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
    func record(_ step: UndoStep) {
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
    func register(_ step: UndoStep, backwards: Bool) {
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

    func clearUndoManager() {
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

    // MARK: Parts outside the undo history

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

    /// Puts chip `id` on `newPart` (`PartList.swap`): wires on pins the new
    /// part lacks stay with loose ends at those pins' old tips, and the rest
    /// of the wires are re-fitted. Not recorded: callers wrap it in an edit
    /// or clear the history.
    func swapPart(of id: UUID, to newPart: CatalogPart, symbol newSymbol: PartSymbol? = nil, _ scope: EditScope) {
        let pinCount = newPart.pins.count
        loosen(scope) { end in
            if case let .pin(ref) = end { return ref.chipID == id && ref.index >= pinCount }
            return false
        }
        updateParts(scope) { $0.swap(id, to: newPart, symbol: newSymbol) }
        settleWires(attachedTo: id)
    }

    // MARK: Stored state of the extensions

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
    var ercFocus: ErcFocus?

    /// The part library, for the voltage check. With none, no edit is
    /// refused.
    /// The backend library (nil in some tests); the voltage check and the ERC read it.
    let library: PartLibraryHandle?

    // MARK: Stored state of the extensions

    /// What is on the PCB page: the board's settings, texts, copper, holes,
    /// pours, keep-outs and cutouts (plan-68-E). Changed only through
    /// recorded edits (`edit`), so every change is an undo step.
    let layout = BoardContent()

    /// The electrical rules check's settings (plan-63-E): severities, the pin
    /// map and the excluded violations. Changed through the edit functions
    /// (`setErcSeverity` …), so every change is an undo step.
    private(set) var erc = ErcSettings()

    /// Lowest number tried for the next part's reference ("U" plus a number).
    private(set) var nextReference = 1

    var ercFocusSerial = 0

    /// Which units of selected parts drawn as their units are picked: the
    /// ones a drag, Flip or Rotate acts on, drawn fully marked (their
    /// siblings faintly). A part with no entry has every unit picked. Editor
    /// state like the selection: never saved, not part of undo.
    var pickedUnits: [UUID: Set<String>] = [:]

    /// The unit a click will leave as the only picked one, when the press
    /// was on one of several picked things (see `pressPiece`).
    var pendingPick: UnitRef?

    /// Set while a confirm dialog is up. Erasing does nothing meanwhile, so
    /// one eraser sweep raises at most one dialog.
    var pendingRemoval: PendingRemoval?

    // MARK: Stored state of the extensions

    // MARK: The board, the document's setup and file load

    /// The PCB's size, layers, colour and units.
    var board: BoardSettings { layout.settings }
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

    /// Asks the canvas to bring `point` into view.
    func focusErc(on point: CGPoint) {
        ercFocusSerial += 1
        ercFocus = ErcFocus(point: point, serial: ercFocusSerial)
    }

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
}
