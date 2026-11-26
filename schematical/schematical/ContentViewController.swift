import AppKit
import Observation
import SwiftUI

/// What a schematic window's editor (`ContentView`) holds and does, out of
/// the view so it can be tested on its own (plan-68-AA): the tools, the
/// pages' viewports, the PCB's layer state, the dialogs and sheets it shows,
/// the Edit and Route menus' actions and the child windows it opened. The
/// view draws this and calls it.
@MainActor
@Observable
final class ContentViewController {
    nonisolated deinit {}

    /// What this window edits (plan-61-B): made by the window's owner.
    @ObservationIgnored let model: SchematicEditorModel
    @ObservationIgnored let catalog: PartCatalogStore
    /// Where the app preferences this reads and writes live (`supplyVoltage`).
    @ObservationIgnored let defaults: UserDefaults
    /// How long the angle flash stays before it fades, and how long an
    /// editor message stays (shorter in tests).
    @ObservationIgnored var angleFlashHold: Duration = .milliseconds(500)
    @ObservationIgnored var editorMessageHold: Duration = .seconds(4)

    var document: SchematicDocument { model.document }
    var simulation: SimulationController { model.simulation }

    /// The schematic's pan and zoom. Its 100 % draws at 0.75 points per
    /// schematic unit (what used to be 75 %).
    let viewport = CanvasViewport(baseScale: 0.75)
    /// The PCB page's own pan and zoom. It opens at 25 %, so a whole board
    /// fits.
    let boardViewport = CanvasViewport(zoom: 0.25)
    /// The page shown: the model's, so the menus and the document (which
    /// restores it with the window) see it.
    var page: EditorPage {
        get { model.page }
        set { model.page = newValue }
    }
    var tool: SchematicTool = .pointer
    var pcbTool = PCBTool.select
    /// Layers the PCB page doesn't draw (the View popover). The hidden set,
    /// so layers added later start shown. Per window; not saved.
    var hiddenLayers: Set<BoardViewLayer> = []
    var isLayerViewShown = false
    /// The net Board Details' Nets tab highlights on the canvas.
    /// Per window; not saved, not undone.
    var highlightedNet: PlaneNet?
    /// Route ▸ Autoroute…'s sheet.
    var isAutorouteOpen = false
    /// The layer the PCB Trace tool draws on (the trace layer menu).
    var traceLayer: CopperLayer = .top
    /// What the Cutout tool draws (plan-65).
    var cutoutShape: CutoutShape = .rectangle
    /// Whether the sidebar is shown: the parts drawer on the Schematic page,
    /// the inspector on the PCB page. One flag for both pages.
    var isSidebarShown = true
    /// Edit › Reset References is asking whether to go on.
    var isConfirmingResetReferences = false
    /// The supply menu's Custom…: the dialog, what is typed in it, and
    /// whether the last entry was refused.
    var isAskingCustomVoltage = false
    var customVoltageText = ""
    var customVoltageRefused = false
    /// The Trace tool's angle, kept until changed; not saved in the file.
    var traceAngle = TraceRoute.defaultAngle
    /// The angle shown in the middle of the PCB after A, and how opaque it
    /// is; `angleFlashID` lets only the latest press fade it.
    var angleFlash: Int?
    var angleFlashOpacity = 0.0
    @ObservationIgnored private var angleFlashID = UUID()
    /// Where the pointer is over the PCB, for ⌘V.
    @ObservationIgnored let boardPointer = BoardPointer()
    /// Where the pointer is over the Schematic, for ⌘V.
    @ObservationIgnored let schematicPointer = BoardPointer()
    /// The child windows this window opened, each with how to close it:
    /// closed along with this window.
    @ObservationIgnored private(set) var openedWindows: [AnyHashable: () -> Void] = [:]

    init(model: SchematicEditorModel, catalog: PartCatalogStore? = nil, defaults: UserDefaults = .standard) {
        self.model = model
        self.catalog = catalog ?? .shared
        self.defaults = defaults
    }

    /// Identifies this window's document to the child windows it opens.
    var documentID: UUID { model.id }

    /// The supply the VCC tool places (a `SupplyChoice` name), an app
    /// preference (`@AppStorage("supplyVoltage")` in the view).
    var supplyVoltage: String {
        get { defaults.string(forKey: Self.supplyVoltageKey) ?? SupplyChoice.defaultName }
        set { defaults.set(newValue, forKey: Self.supplyVoltageKey) }
    }
    static let supplyVoltageKey = "supplyVoltage"

    /// The zoom controls act on the page being shown.
    var activeViewport: CanvasViewport {
        page == .schematic ? viewport : boardViewport
    }

    /// The layers the Trace tool can draw on: the board's, less its planes.
    var traceLayers: [CopperLayer] { document.board.traceLayers }

    /// What the PCB page draws.
    var visibility: BoardVisibility { BoardVisibility(hidden: hiddenLayers) }

    /// A run of Autoroute is under way.
    var isRouting: Bool { model.autoroute?.session.isRunning == true }

    /// The copper an Autoroute run has routed so far, while it runs.
    var routePreview: (traces: [RoutedTrace], vias: [RoutedVia])? {
        isRouting ? model.autoroute?.session.preview : nil
    }

    // MARK: Rotate and flip

    /// What Rotate would turn. On the PCB: exactly one footprint or board
    /// text selected (plan-15: never a group), not when it is locked; a trace
    /// doesn't turn. On the Schematic: the sole selected part drawn as its
    /// own symbol (`PartSymbol.canRotate`), while the simulation isn't running.
    var canRotate: Bool {
        if page == .schematic { return rotateTarget != nil }
        if let id = document.selectedTextID { return !document.isLocked(.text(id)) }
        guard let id = document.selectedFootprintID,
              let board = document.chips.first(where: { $0.id == id })?.board else { return false }
        return !board.locked
    }

    /// The Rotate button and ⌘R: turns the selected board text, else the
    /// selected footprint, 90° clockwise; ⇧⌘R turns it the other way.
    /// Returns whether it turned anything.
    @discardableResult
    func rotateSelection(counterclockwise: Bool = false) -> Bool {
        guard canRotate else { return false }
        if page == .schematic, let target = rotateTarget {
            switch target {
            case let .chip(id): document.rotateChip(id, counterclockwise: counterclockwise)
            case let .unit(ref): document.rotateUnit(ref, counterclockwise: counterclockwise)
            }
        } else if let id = document.selectedTextID {
            document.rotateText(id, counterclockwise: counterclockwise)
        } else if let id = document.selectedFootprintID {
            document.rotateFootprint(id, counterclockwise: counterclockwise)
        }
        return true
    }

    /// The toolbar's Rotate button.
    func rotate() { rotateSelection() }

    /// ⌘F: flips the selected net label, else the selected part, on the
    /// Schematic. Returns whether it flipped anything.
    func flipSelection() -> Bool {
        if page == .schematic, !simulation.isRunning, let id = document.selectedNetLabelID {
            document.flipNetLabel(id)
            return true
        }
        guard let target = flipTarget else { return false }
        switch target {
        case let .chip(id): document.flipChip(id)
        case let .unit(ref): document.flipUnit(ref)
        }
        return true
    }

    /// Whether ⌘F has something to flip.
    var canFlip: Bool {
        (page == .schematic && !simulation.isRunning && document.selectedNetLabelID != nil) || flipTarget != nil
    }

    /// What Rotate would turn on the Schematic: the sole selected part, or
    /// the one picked unit of a part drawn as its units (plan-67), when it
    /// can rotate and the simulation isn't running.
    private var rotateTarget: SchematicDocument.EditTarget? {
        guard page == .schematic, !simulation.isRunning else { return nil }
        return document.rotateTarget
    }

    /// What Flip would mirror: the sole selected part on the Schematic, or
    /// the one picked unit of a part drawn as its units, when it can flip
    /// and the simulation isn't running.
    private var flipTarget: SchematicDocument.EditTarget? {
        guard page == .schematic, !simulation.isRunning else { return nil }
        return document.flipTarget
    }

    // MARK: The editing keys

    /// Watches for the editing keys (`handleKey`). Kept by the model, not
    /// here, and installed through the model's monitors: the document
    /// empties those when it closes (a hosted view gets no `onDisappear`
    /// then), and the monitor's closure holds this and so the model.
    func installEditKeyMonitor() {
        guard model.editKeyMonitor == nil else { return }
        model.editKeyMonitor = model.eventMonitors.add(matching: .keyDown) { [self] event in handleKey(event) }
    }

    func removeEditKeyMonitor() {
        model.eventMonitors.remove(model.editKeyMonitor)
        model.editKeyMonitor = nil
    }

    /// The editing keys, in this window only and never while a text field
    /// is being typed in (`ContentView.editKey`, `perform`). Returns nil
    /// for a key it acted on, the event for every other.
    func handleKey(_ event: NSEvent) -> NSEvent? {
        guard let window = model.file.window, event.window === window, !(window.firstResponder is NSText),
              let key = ContentView.editKey(keyCode: event.keyCode, event.charactersIgnoringModifiers,
                                            event.modifierFlags, isRepeat: event.isARepeat),
              perform(key) else { return event }
        return nil
    }

    /// One editing key: ⌘R presses Rotate (when it is enabled: on the PCB,
    /// or on the Schematic for a part drawn as its own symbol)
    /// and ⇧⌘R turns the other way; A steps the Trace tool's angle while
    /// Trace is the PCB tool; ⌘F flips the selected part or net label on the
    /// Schematic; ⌘⌫ deletes the selection (on the PCB only its texts, traces, vias and holes);
    /// ⌘C copies the selected traces, texts and vias on the PCB and ⌘V pastes them centred
    /// on the pointer (`BoardClipboard`); on the Schematic they copy the
    /// selection and paste it the same way (`SchematicClipboard`; not while
    /// the simulation runs); ⌘X copies and then deletes what it copied; ⌘A
    /// selects everything on the page. These are the Edit menu's items too
    /// (`EditCommands`). Returns false for a key with nothing to act on.
    func perform(_ key: ContentView.EditKey) -> Bool {
        switch key {
        case .rotate: return rotateSelection()
        case .rotateBack: return rotateSelection(counterclockwise: true)
        case .flip: return flipSelection()
        case .angle:
            guard page == .pcb, pcbTool == .trace else { return false }
            traceAngle = TraceRoute.nextAngle(after: traceAngle)
            flashAngle(traceAngle)
            return true
        case .delete: return deleteSelection()
        case .cut: return cutSelection()
        case .copy: return copySelection()
        case .paste: return pasteClipboard()
        case .selectAll: return selectAll()
        }
    }

    /// Shows `angle` in the middle of the PCB for `angleFlashHold`, then
    /// fades it out over 250 ms. A newer press restarts it.
    func flashAngle(_ angle: Int) {
        let id = UUID()
        angleFlashID = id
        angleFlash = angle
        angleFlashOpacity = 1
        let hold = angleFlashHold
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: hold)
            guard let self, self.angleFlashID == id else { return }
            withAnimation(.easeOut(duration: 0.25)) { self.angleFlashOpacity = 0 }
        }
    }

    // MARK: Edit menu

    /// ⌘⌫: deletes the selection (on the PCB only its texts, traces,
    /// vias and holes). Returns whether anything was deleted.
    func deleteSelection() -> Bool {
        page == .pcb
            ? document.deleteBoardSelection()
            : !simulation.isRunning && document.deleteSchematicSelection()
    }

    /// ⌘C: the Schematic selection to `SchematicClipboard`, or the selected
    /// traces, texts and vias to `BoardClipboard`. Returns whether anything was copied.
    func copySelection() -> Bool {
        if page == .schematic {
            guard let copied = document.schematicCopy() else { return false }
            SchematicClipboard.copy(copied)
            return true
        }
        let copied = document.selectedTraces(), copiedTexts = document.selectedTexts()
        let copiedVias = document.selectedVias()
        guard !copied.isEmpty || !copiedTexts.isEmpty || !copiedVias.isEmpty else { return false }
        BoardClipboard.copy(copied, texts: copiedTexts, vias: copiedVias)
        return true
    }

    /// ⌘X: copies as ⌘C, then deletes what was copied (on the PCB, the
    /// selected traces, texts and vias). Not on the Schematic while the simulation runs.
    func cutSelection() -> Bool {
        if page == .schematic {
            guard !simulation.isRunning, copySelection() else { return false }
            return document.deleteSchematicSelection(named: "Cut", includingTitleBlock: false)
        }
        guard copySelection() else { return false }
        return document.cutBoardSelection()
    }

    /// ⌘V: pastes centred on the pointer, or one step on from the last
    /// paste with the pointer off the page. Returns whether it pasted.
    func pasteClipboard() -> Bool {
        if page == .schematic {
            guard !simulation.isRunning, let copied = SchematicClipboard.contents else { return false }
            let offset = schematicPointer.point.map { SchematicClipboard.offset(centring: copied, on: $0) }
                ?? SchematicClipboard.nextOffset()
            return document.pasteSchematic(copied, offset: offset)
        }
        guard !BoardClipboard.isEmpty else { return false }
        let copied = BoardClipboard.traces, copiedTexts = BoardClipboard.texts, copiedVias = BoardClipboard.vias
        let offset = boardPointer.point.map {
            BoardClipboard.offset(centring: copied, texts: copiedTexts, vias: copiedVias, on: $0,
                                  grid: document.board.traceGrid)
        } ?? BoardClipboard.nextOffset()
        return !document.pasteBoard(traces: copied, texts: copiedTexts, vias: copiedVias, offset: offset).isEmpty
    }

    /// ⌘A: everything on the page shown (on the PCB, what is drawn).
    func selectAll() -> Bool {
        if page == .pcb {
            document.selectAllBoard(visible: visibility)
        } else {
            document.selectAllSchematic()
        }
        return true
    }

    /// The Reset References alert's Reset.
    func resetReferences() { document.resetReferences(page: page) }

    /// The Edit menu's actions for this window.
    var editActions: EditActions {
        EditActions(cut: cutSelection, copy: copySelection, paste: pasteClipboard,
                    delete: deleteSelection, selectAll: selectAll,
                    flip: flipSelection, canFlip: canFlip,
                    rotate: { [self] in rotateSelection(counterclockwise: $0) }, canRotate: canRotate,
                    resetReferences: { [self] in isConfirmingResetReferences = true },
                    canResetReferences: !document.chips.isEmpty && !simulation.isRunning)
    }

    // MARK: Simulation

    /// Simulation › Run / Stop. Play mode starts with the Pointer (and PCB
    /// Select) tool and nothing selected. The simulation is the Schematic
    /// page's: nothing happens on the PCB. Run does nothing while a part
    /// that isn't simulated is placed (plan-36: the menu disables Run
    /// itself; this is the check for any other way here).
    func toggleSimulation() {
        guard page == .schematic else { return }
        if !simulation.isRunning {
            guard SimulationAvailability.canStart(document) else { return }
            tool = .pointer
            pcbTool = .select
            document.clearAllSelections()
        }
        SimulationAvailability.toggle(simulation, document: document)
    }

    /// The Simulation menu's view of this window (plan-36: Run is off while a
    /// part that isn't simulated is placed, with the reason under it).
    var simulationTarget: SimulationTarget {
        SimulationAvailability.target(page: page, chips: document.chips, isRunning: simulation.isRunning,
                                      toggle: toggleSimulation)
    }

    // MARK: Menus and child windows

    /// Gives the app's menus this window's actions (`EditorMenus`). Each
    /// closure reads the editor's state when a menu asks, so the menus
    /// follow it; the document removes them when it closes.
    func installMenus() {
        let menus = model.menus
        menus.edit = { [self] in editActions }
        menus.simulation = { [self] in simulationTarget }
        menus.route = { [self] in
            RouteActions.available(page: page, isPlaying: document.isPlaying) ? routeActions : nil
        }
        menus.openBillOfMaterials = { [self] in open("bom", BillOfMaterialsTarget(documentID: documentID)) }
        menus.openDocumentInfo = openDocumentInfo
        menus.openDesignRuleCheck = openDesignRuleCheck
        menus.openElectricalRulesCheck = { [self] in
            open("erc", ElectricalRulesCheckTarget(documentID: documentID))
        }
    }

    /// Opens (or brings forward) a child window of this document and
    /// remembers it, to close it with this window.
    func open<Target: Hashable>(_ id: String, _ target: Target) {
        let documentID = documentID
        openedWindows[AnyHashable(target)] = { ChildWindowCloser(documentID: documentID)(id: id, value: target) }
        ChildWindowOpener(documentID: documentID)(id: id, value: target)
    }

    func openMemory(_ chipID: UUID) { open("memory", MemoryWindowTarget(documentID: documentID, chipID: chipID)) }
    func openCpu(_ chipID: UUID) { open("cpu", CpuWindowTarget(documentID: documentID, chipID: chipID)) }
    func openTerminal(_ chipID: UUID) {
        open("terminal", TerminalWindowTarget(documentID: documentID, chipID: chipID))
    }
    func openRegisters(_ chipID: UUID) {
        open("registers", RegisterWindowTarget(documentID: documentID, chipID: chipID))
    }
    func openDocumentInfo() { open("documentInfo", DocumentInfoTarget(documentID: documentID)) }
    func openDesignRuleCheck() { open("drc", DesignRuleCheckTarget(documentID: documentID)) }
    /// The toolbar's View button: shows or hides the layer popover.
    func toggleLayerView() { isLayerViewShown.toggle() }

    /// A picture of the board as made, in its own window.
    func openBoardPreview() { open("board-preview", BoardPreviewTarget(documentID: documentID)) }

    /// The window appeared: the simulation runs at the app's speed, and the
    /// editing keys and the menus are this window's.
    func appear(speed: Double) {
        simulation.setSpeed(speed)
        installEditKeyMonitor()
        installMenus()
    }

    /// The window went away: its keys, its menus and the windows it opened
    /// go with it.
    func disappear() {
        removeEditKeyMonitor()
        model.menus.removeAll()
        let closers = openedWindows.values
        openedWindows = [:]
        for close in closers { close() }
    }

    /// The page changed. It is restored with the window (plan-61-C). The
    /// simulation never runs on the PCB page; coming back to the Schematic
    /// does not restart it.
    func pageChanged(to newPage: EditorPage) {
        model.platformDocument?.pageDidChange()
        if newPage == .pcb {
            simulation.stop()
        }
    }

    // MARK: Tools

    /// Placing a part from the drawer returns to the Pointer tool.
    func partPlaced() { tool = .pointer }

    /// Placing a text returns to the Select tool.
    func textPlaced() { pcbTool = .select }

    /// The supply menu's rail: also picks VCC, unless Pull-up is the tool.
    func pickSupply(_ name: String) {
        supplyVoltage = name
        tool = tool == .pullUp ? .pullUp : .vcc
    }

    /// The supply menu's Custom…: asks for any other rail, such as a nixie
    /// tube's 170 V, starting from the one in use.
    func askCustomVoltage() {
        let current = SupplyChoice.millivolts(name: supplyVoltage)
        customVoltageText = current.map { VoltageCheck.volts(UInt32($0)) } ?? ""
        customVoltageRefused = false
        isAskingCustomVoltage = true
    }

    /// Takes the Custom… dialog's voltage for the VCC and Pull-up tools, or
    /// asks again when it is no voltage a rail can have.
    func useCustomVoltage() {
        guard let name = SupplyChoice.name(typed: customVoltageText), catalog.part(named: name) != nil else {
            customVoltageRefused = true
            // The alert has closed by now; show it again with the reason.
            DispatchQueue.main.async { [self] in isAskingCustomVoltage = true }
            return
        }
        pickSupply(name)
    }

    /// The Custom… dialog's message.
    var customVoltageMessage: String {
        customVoltageRefused
            ? "That is not a voltage the tools can use. Enter a number above 0, up to 500 V."
            : "The voltage the VCC and Pull-up tools place, up to 500 V."
    }

    /// The Trace tool drew on a hidden layer: show it.
    func showLayer(_ layer: CopperLayer) { hiddenLayers.remove(.copper(layer)) }

    /// A V while drawing moved the trace to `layer`: it is the Trace tool's
    /// layer now, and shown.
    func changeTraceLayer(_ layer: CopperLayer) {
        traceLayer = layer
        showLayer(layer)
    }

    /// The trace layer menu: also picks Trace and shows that layer.
    func pickTraceLayer(_ layer: CopperLayer) {
        changeTraceLayer(layer)
        pcbTool = .trace
    }

    /// The Cutout tool's shape menu: also picks the tool.
    func pickCutoutShape(_ shape: CutoutShape) {
        cutoutShape = shape
        pcbTool = .cutout
    }

    /// A trace layer that left the board or became a plane goes back to Top.
    func traceLayersChanged(_ layers: [CopperLayer]) {
        if !layers.contains(traceLayer) { traceLayer = .top }
    }

    /// A refused wire, switch, route, assisted trace or via says why, then
    /// clears after `editorMessageHold`, unless another replaced it.
    func expireEditorMessage() async {
        guard document.editorMessage != nil else { return }
        try? await Task.sleep(for: editorMessageHold)
        if !Task.isCancelled {
            document.editorMessage = nil
        }
    }

    /// The erase confirm (a port with wires, a bus line with ports, a wired
    /// pin): up while the document has a removal pending; closing it
    /// cancels that.
    var isConfirmingRemoval: Bool {
        get { document.pendingRemoval != nil }
        set { if !newValue { document.cancelPendingRemoval() } }
    }

    // MARK: Zoom

    func zoomOut() { activeViewport.zoomOut() }
    func zoomIn() { activeViewport.zoomIn() }
    func setZoom(_ level: CGFloat) { activeViewport.setZoom(level) }

    /// The zoom menu's title: the page's zoom, e.g. "75%".
    static func percent(_ zoom: CGFloat) -> String { "\(Int((zoom * 100).rounded()))%" }

    // MARK: Autoroute

    /// The window's Autoroute model, or a new one when it has none yet
    /// (kept by `openAutoroute`).
    var autorouteModel: AutorouteModel {
        model.autoroute ?? AutorouteModel(document: document)
    }

    /// Routes `scope` with the last Autoroute options, the sheet showing
    /// the run (the Route tool, a Nets row's Route Net).
    func route(_ scope: RoutingOptions.Scope) {
        guard !document.isPlaying, !isRouting else { return }
        openAutoroute(.scope(scope), run: true)
    }

    /// Opens the Autoroute sheet on `scope`, and starts a run when `run`.
    func openAutoroute(_ scope: AutorouteModel.ScopeChoice, run: Bool = false) {
        let model = autorouteModel
        self.model.autoroute = model
        model.scopeChoice = scope
        isAutorouteOpen = true
        if run { model.route() }
    }

    /// The Autoroute sheet's Close.
    func closeAutoroute() { isAutorouteOpen = false }

    /// A result row's click in the Autoroute sheet.
    func highlight(_ net: PlaneNet) { highlightedNet = net }

    /// The Route menu's actions for this window (plan-38-K).
    var routeActions: RouteActions {
        RouteActions(
            autoroute: { [self] in openAutoroute(.all) },
            routeSelected: { [self] in openAutoroute(.selection, run: true) },
            unrouteSelected: { [self] in document.unroute(nets: document.selectedNets(), onlyAuto: false) },
            unrouteAllAutorouted: { [self] in document.unrouteAll(onlyAuto: true) },
            selectAutorouted: { [self] in document.selectAutorouted() },
            isRouting: isRouting,
            hasSelection: !document.boardSelection.isEmpty
        )
    }
}
