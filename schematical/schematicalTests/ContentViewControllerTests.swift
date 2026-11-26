import AppKit
import CoreGraphics
import XCTest
@testable import schematical

/// plan-68-AA: what a schematic window's editor holds and does
/// (`ContentViewController`), without its view.
@MainActor
final class ContentViewControllerTests: XCTestCase {
    nonisolated deinit {}

    private var defaults: UserDefaults!
    private var suite = ""
    /// Models made by tests, kept for the life of the test process (as
    /// `TestSupport.newDocument()` keeps its documents).
    private static var kept: [AnyObject] = []

    override func setUp() {
        suite = "ContentViewControllerTests-\(UUID().uuidString)"
        defaults = UserDefaults(suiteName: suite)
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        defaults.removePersistentDomain(forName: suite)
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// A controller on a new document of the test catalog.
    private func make(_ doc: SchematicDocument? = nil) -> ContentViewController {
        let model = SchematicEditorModel(catalog: TestSupport.catalog, document: doc ?? TestSupport.newDocument())
        let controller = ContentViewController(model: model, catalog: TestSupport.catalog, defaults: defaults)
        Self.kept.append(model)
        Self.kept.append(controller)
        addTeardownBlock { @MainActor in
            controller.simulation.stop()
            ChildWindowCenter.shared.closeAll(model.id)
        }
        return controller
    }

    private func spin(_ seconds: TimeInterval = 0.05) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    // MARK: Rotate and flip

    func testRotateAndFlipAPartOnTheSchematic() throws {
        let c = make()
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        XCTAssertFalse(c.canRotate)
        XCTAssertFalse(c.rotateSelection(), "nothing selected")
        XCTAssertFalse(c.canFlip)
        XCTAssertFalse(c.flipSelection())

        c.document.select(.chip(led.id))
        XCTAssertTrue(c.canRotate)
        XCTAssertTrue(c.canFlip)
        XCTAssertTrue(c.rotateSelection())
        let turned = try XCTUnwrap(c.document.chips.first)
        XCTAssertNotEqual(turned, led)
        XCTAssertTrue(c.rotateSelection(counterclockwise: true))
        let back = try XCTUnwrap(c.document.chips.first)
        XCTAssertNotEqual(back, turned, "turned back")
        c.rotate()
        XCTAssertNotEqual(c.document.chips.first, back, "the toolbar's Rotate")
        let before = try XCTUnwrap(c.document.chips.first)
        XCTAssertTrue(c.flipSelection())
        XCTAssertNotEqual(c.document.chips.first, before)

        // Not while the simulation runs.
        c.simulation.start(document: c.document)
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        XCTAssertFalse(c.canRotate)
        XCTAssertFalse(c.canFlip)
        XCTAssertFalse(c.flipSelection())
    }

    func testRotateAndFlipOneUnitOfAPartDrawnAsItsUnits() throws {
        let c = make()
        let gate = try TestSupport.place(c.document, "SN74HC00N", at: p(300, 300))
        c.document.setPartSplit(gate.id, true)
        c.document.select(.chip(gate.id))
        c.document.pick(UnitRef(chipID: gate.id, key: "B"), extend: false)
        let before = try XCTUnwrap(c.document.chips.first)
        XCTAssertTrue(c.canRotate)
        XCTAssertTrue(c.rotateSelection())
        let turned = try XCTUnwrap(c.document.chips.first)
        XCTAssertNotEqual(turned, before)
        XCTAssertTrue(c.flipSelection())
        XCTAssertNotEqual(c.document.chips.first, turned)
    }

    func testFlipANetLabel() throws {
        let c = make()
        let label = c.document.addNetLabel(at: p(100, 100))
        c.document.select(.label(label))
        let before = try XCTUnwrap(c.document.netLabels.first)
        XCTAssertTrue(c.canFlip)
        XCTAssertTrue(c.flipSelection())
        XCTAssertNotEqual(c.document.netLabels.first, before)
        c.page = .pcb
        XCTAssertFalse(c.canFlip, "not on the PCB")
        XCTAssertFalse(c.flipSelection())
    }

    func testRotateOnThePCB() throws {
        let c = make()
        c.page = .pcb
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        XCTAssertFalse(c.canRotate, "nothing selected")
        let text = c.document.addText(at: p(20, 20))
        c.document.select(.text(text))
        XCTAssertTrue(c.canRotate)
        let shown = try XCTUnwrap(c.document.texts.first)
        XCTAssertTrue(c.rotateSelection())
        XCTAssertNotEqual(c.document.texts.first?.rotation, shown.rotation)
        c.document.setLocked(.text(text), true)
        XCTAssertFalse(c.canRotate, "a locked text")
        XCTAssertFalse(c.rotateSelection())

        c.document.select(.footprint(led.id))
        XCTAssertTrue(c.canRotate)
        let placed = try XCTUnwrap(c.document.chips.first?.board)
        XCTAssertTrue(c.rotateSelection(counterclockwise: true))
        XCTAssertNotEqual(c.document.chips.first?.board, placed)
        c.document.setLocked(.footprint(led.id), true)
        XCTAssertEqual(c.document.chips.first?.board?.locked, true)
        XCTAssertFalse(c.canRotate, "a locked footprint")
    }

    // MARK: The editing keys

    func testPerformEachKey() throws {
        let c = make()
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.document.select(.chip(led.id))
        XCTAssertTrue(c.perform(.rotate))
        let turned = try XCTUnwrap(c.document.chips.first)
        XCTAssertTrue(c.perform(.rotateBack))
        XCTAssertNotEqual(c.document.chips.first, turned)
        XCTAssertTrue(c.perform(.flip))
        XCTAssertFalse(c.perform(.angle), "A is the PCB Trace tool's")
        XCTAssertTrue(c.perform(.copy))
        XCTAssertTrue(c.perform(.paste))
        XCTAssertEqual(c.document.chips.count, 2)
        XCTAssertTrue(c.perform(.cut))
        XCTAssertEqual(c.document.chips.count, 1)
        XCTAssertTrue(c.perform(.selectAll))
        XCTAssertTrue(c.perform(.delete))
        XCTAssertTrue(c.document.chips.isEmpty)
    }

    func testTheAngleKeyStepsAndFlashesTheTraceAngle() {
        let c = make()
        c.angleFlashHold = .milliseconds(20)
        c.page = .pcb
        XCTAssertFalse(c.perform(.angle), "Select is the tool")
        c.pcbTool = .trace
        let first = c.traceAngle
        XCTAssertTrue(c.perform(.angle))
        XCTAssertEqual(c.traceAngle, TraceRoute.nextAngle(after: first))
        XCTAssertEqual(c.angleFlash, c.traceAngle)
        XCTAssertEqual(c.angleFlashOpacity, 1)
        // A newer press restarts the flash: the first one's fade is skipped.
        c.angleFlashHold = .milliseconds(400)
        c.flashAngle(c.traceAngle)
        spin(0.15)
        XCTAssertEqual(c.angleFlashOpacity, 1, "the first press's fade does not end the newer flash")
        spin(0.6)
        XCTAssertEqual(c.angleFlashOpacity, 0)
    }

    func testHandleKeyActsOnlyInItsWindowAndNotWhileTyping() throws {
        let c = make()
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 200, height: 100), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.orderFront(nil)
        defer { window.close() }
        func key(_ characters: String, _ modifiers: NSEvent.ModifierFlags, keyCode: UInt16 = 0,
                 in target: NSWindow? = nil) throws -> NSEvent {
            try XCTUnwrap(NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: modifiers,
                                           timestamp: 0, windowNumber: (target ?? window).windowNumber,
                                           context: nil, characters: characters,
                                           charactersIgnoringModifiers: characters, isARepeat: false,
                                           keyCode: keyCode))
        }
        let selectAll = try key("a", .command)
        XCTAssertNotNil(c.handleKey(selectAll), "no window yet")
        c.model.file.window = window
        XCTAssertNil(c.handleKey(selectAll), "⌘A is taken")
        XCTAssertNotNil(c.handleKey(try key("q", .command)), "not an editing key")
        XCTAssertNotNil(c.handleKey(try key("r", .command)), "nothing to rotate")

        let other = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 100, height: 100), styleMask: [.titled],
                             backing: .buffered, defer: false)
        other.isReleasedWhenClosed = false
        other.orderFront(nil)
        defer { other.close() }
        XCTAssertNotNil(c.handleKey(try key("a", .command, in: other)), "another window")

        let field = NSTextView(frame: NSRect(x: 0, y: 0, width: 100, height: 20))
        window.contentView?.addSubview(field)
        XCTAssertTrue(window.makeFirstResponder(field))
        XCTAssertNotNil(c.handleKey(selectAll), "typing in a field")
    }

    func testTheKeyMonitorIsInstalledOnceAndRemoved() {
        let c = make()
        XCTAssertNil(c.model.editKeyMonitor)
        c.installEditKeyMonitor()
        let monitor = c.model.editKeyMonitor
        XCTAssertNotNil(monitor)
        let count = c.model.eventMonitors.count
        c.installEditKeyMonitor()
        XCTAssertEqual(c.model.editKeyMonitor, monitor, "installed once")
        XCTAssertEqual(c.model.eventMonitors.count, count)
        c.removeEditKeyMonitor()
        XCTAssertNil(c.model.editKeyMonitor)
        XCTAssertEqual(c.model.eventMonitors.count, count - 1)
    }

    // MARK: Edit menu

    func testSchematicClipboard() throws {
        let c = make()
        XCTAssertFalse(c.copySelection(), "nothing selected")
        XCTAssertFalse(c.cutSelection())
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.document.select(.chip(led.id))
        XCTAssertTrue(c.copySelection())
        c.schematicPointer.point = p(600, 400)
        XCTAssertTrue(c.pasteClipboard())
        let pasted = try XCTUnwrap(c.document.chips.last)
        XCTAssertEqual(pasted.bodyRect.midX, 600, accuracy: 40, "centred on the pointer")
        c.schematicPointer.point = nil
        XCTAssertTrue(c.pasteClipboard(), "one step on from the last paste")
        XCTAssertEqual(c.document.chips.count, 3)

        c.simulation.start(document: c.document)
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        XCTAssertFalse(c.pasteClipboard())
        XCTAssertFalse(c.cutSelection())
        XCTAssertFalse(c.deleteSelection())
        c.simulation.stop()
        c.document.select(.chip(led.id))
        XCTAssertTrue(c.deleteSelection())
        XCTAssertEqual(c.document.chips.count, 2)
    }

    func testBoardClipboard() throws {
        let c = make()
        c.page = .pcb
        BoardClipboard.copy([])
        XCTAssertFalse(c.pasteClipboard(), "the clipboard is empty")
        XCTAssertFalse(c.copySelection(), "nothing selected")
        XCTAssertFalse(c.cutSelection())
        let trace = try XCTUnwrap(c.document.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        c.document.select(.trace(trace))
        XCTAssertTrue(c.copySelection())
        c.boardPointer.point = p(40, 40)
        XCTAssertTrue(c.pasteClipboard())
        XCTAssertEqual(c.document.traces.count, 2)
        let pasted = try XCTUnwrap(c.document.traces.last)
        XCTAssertEqual((pasted.points.first!.x + pasted.points.last!.x) / 2, 40, accuracy: 2, "centred on the pointer")
        c.boardPointer.point = nil
        XCTAssertTrue(c.pasteClipboard())
        XCTAssertEqual(c.document.traces.count, 3)
        c.document.select(.trace(trace))
        XCTAssertTrue(c.cutSelection())
        XCTAssertFalse(c.document.traces.contains { $0.id == trace })
        c.document.select(.trace(try XCTUnwrap(c.document.traces.first).id))
        XCTAssertTrue(c.deleteSelection())
        XCTAssertEqual(c.document.traces.count, 1)
    }

    func testSelectAllTakesWhatThePageShows() throws {
        let c = make()
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        let bottom = try XCTUnwrap(c.document.addTrace(points: [p(5, 10), p(15, 10)], layer: .bottom))
        XCTAssertTrue(c.selectAll())
        XCTAssertTrue(c.document.schematicSelection.items.contains(.chip(led.id)))
        c.document.clearAllSelections()
        c.page = .pcb
        c.hiddenLayers = [.copper(.bottom)]
        XCTAssertEqual(c.visibility, BoardVisibility(hidden: [.copper(.bottom)]))
        XCTAssertTrue(c.selectAll())
        XCTAssertTrue(c.document.isSelected(.footprint(led.id)))
        XCTAssertFalse(c.document.isSelected(.trace(bottom)), "a hidden layer's trace")
    }

    func testEditActions() throws {
        let c = make()
        var actions = c.editActions
        XCTAssertFalse(actions.canResetReferences, "no parts")
        XCTAssertFalse(actions.canRotate)
        XCTAssertFalse(actions.rotate(false))
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.document.select(.chip(led.id))
        actions = c.editActions
        XCTAssertTrue(actions.canResetReferences)
        XCTAssertTrue(actions.canRotate)
        XCTAssertTrue(actions.canFlip)
        let before = try XCTUnwrap(c.document.chips.first)
        XCTAssertTrue(actions.rotate(true))
        XCTAssertNotEqual(c.document.chips.first, before)
        XCTAssertTrue(actions.selectAll())
        actions.resetReferences()
        XCTAssertTrue(c.isConfirmingResetReferences, "asks first")

        XCTAssertNil(c.document.renameChip(led.id, prefix: "D", number: 7, page: .schematic))
        c.resetReferences()
        XCTAssertNotEqual(c.document.chips.first?.reference, "D7")
    }

    // MARK: Simulation

    func testToggleSimulation() throws {
        let c = make()
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.page = .pcb
        c.toggleSimulation()
        XCTAssertFalse(c.simulation.isRunning, "not on the PCB")
        c.page = .schematic
        c.tool = .line
        c.pcbTool = .trace
        c.document.select(.chip(led.id))
        c.toggleSimulation()
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        XCTAssertEqual(c.tool, .pointer)
        XCTAssertEqual(c.pcbTool, .select)
        XCTAssertTrue(c.document.schematicSelection.items.isEmpty)
        XCTAssertTrue(c.simulationTarget.isRunning)
        c.simulationTarget.toggle()
        XCTAssertFalse(c.simulation.isRunning, "Stop")
    }

    func testRunDoesNothingWithAPartThatIsNotSimulated() throws {
        let c = make()
        let part = try XCTUnwrap(TestSupport.catalog.part(named: "ATTINY13A-PU"))
        XCTAssertFalse(part.isSimulated, "the Run check's example part")
        c.document.placeChip(part: part, symbol: symbol(for: part), centeredAt: p(200, 200))
        c.toggleSimulation()
        XCTAssertFalse(c.simulation.isRunning)
        XCTAssertNotNil(c.simulationTarget.blockedReason)
    }

    // MARK: Menus and child windows

    func testMenusAndChildWindowsGoWithTheWindow() throws {
        let doc = TestSupport.newDocument()
        let c = make(doc)
        let system = TestSupport.keep(SchematicNSDocument(model: c.model))
        NSDocumentController.shared.addDocument(system)
        addTeardownBlock { @MainActor in NSDocumentController.shared.removeDocument(system) }
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        c.appear(speed: 0.25)
        XCTAssertEqual(c.simulation.speed, 0.25)
        XCTAssertNotNil(c.model.editKeyMonitor)
        let menus = c.model.menus
        XCTAssertNotNil(menus.edit?())
        XCTAssertEqual(menus.simulation?().isRunning, false)
        XCTAssertNil(menus.route?() ?? nil, "no Route menu on the Schematic")
        c.page = .pcb
        XCTAssertNotNil(menus.route?() ?? nil)

        menus.openBillOfMaterials?()
        menus.openDocumentInfo?()
        menus.openDesignRuleCheck?()
        menus.openElectricalRulesCheck?()
        c.openMemory(led.id)
        c.openCpu(led.id)
        c.openTerminal(led.id)
        c.openRegisters(led.id)
        c.openBoardPreview()
        let titles = Set(ChildWindowCenter.shared.windows(of: c.documentID).map(\.title))
        XCTAssertEqual(titles, ["Bill of Materials", "Document Info", "Design Rule Check", "Electrical Rules Check",
                                "Memory", "Processor", "Terminal", "Registers", "Board Preview"])
        c.openDocumentInfo()
        XCTAssertEqual(c.openedWindows.count, 9, "asked again, the same window")

        c.disappear()
        XCTAssertTrue(ChildWindowCenter.shared.windows(of: c.documentID).isEmpty)
        XCTAssertTrue(c.openedWindows.isEmpty)
        XCTAssertNil(menus.edit)
        XCTAssertNil(c.model.editKeyMonitor)
    }

    func testThePCBPageStopsTheSimulation() throws {
        let c = make()
        let system = TestSupport.keep(SchematicNSDocument(model: c.model))
        c.model.platformDocument = system
        _ = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.simulation.start(document: c.document)
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        c.pageChanged(to: .schematic)
        XCTAssertTrue(c.simulation.isRunning)
        c.pageChanged(to: .pcb)
        XCTAssertFalse(c.simulation.isRunning)
    }

    // MARK: Tools

    func testToolsAndTheSupplyMenu() {
        let c = make()
        XCTAssertEqual(c.supplyVoltage, SupplyChoice.defaultName)
        c.tool = .line
        c.partPlaced()
        XCTAssertEqual(c.tool, .pointer)
        c.pcbTool = .text
        c.textPlaced()
        XCTAssertEqual(c.pcbTool, .select)

        let name = SupplyChoice.names.last!
        c.pickSupply(name)
        XCTAssertEqual(c.supplyVoltage, name)
        XCTAssertEqual(defaults.string(forKey: ContentViewController.supplyVoltageKey), name)
        XCTAssertEqual(c.tool, .vcc)
        c.tool = .pullUp
        c.pickSupply(SupplyChoice.names.first!)
        XCTAssertEqual(c.tool, .pullUp, "Pull-up stays the tool")
    }

    func testCustomSupplyVoltage() {
        let c = make()
        c.pickSupply(SupplyChoice.defaultName)
        c.customVoltageRefused = true
        c.askCustomVoltage()
        XCTAssertTrue(c.isAskingCustomVoltage)
        XCTAssertFalse(c.customVoltageRefused)
        XCTAssertEqual(c.customVoltageText,
                       SupplyChoice.millivolts(name: SupplyChoice.defaultName).map { VoltageCheck.volts(UInt32($0)) })
        XCTAssertEqual(c.customVoltageMessage, "The voltage the VCC and Pull-up tools place, up to 500 V.")
        defaults.set("not a rail", forKey: ContentViewController.supplyVoltageKey)
        c.askCustomVoltage()
        XCTAssertEqual(c.customVoltageText, "", "no voltage to start from")

        c.isAskingCustomVoltage = false
        c.customVoltageText = "nonsense"
        c.useCustomVoltage()
        XCTAssertTrue(c.customVoltageRefused)
        XCTAssertEqual(c.customVoltageMessage,
                       "That is not a voltage the tools can use. Enter a number above 0, up to 500 V.")
        XCTAssertFalse(c.isAskingCustomVoltage)
        spin()
        XCTAssertTrue(c.isAskingCustomVoltage, "asked again")

        c.customVoltageText = "170"
        c.tool = .pointer
        c.useCustomVoltage()
        XCTAssertEqual(c.supplyVoltage, SupplyChoice.name(typed: "170"))
        XCTAssertEqual(c.tool, .vcc)
    }

    func testTraceLayerAndCutoutShape() {
        let c = make()
        c.document.setLayerCount(2)
        XCTAssertEqual(c.traceLayers, [.top, .bottom])
        c.hiddenLayers = [.copper(.bottom), .silkscreen]
        c.showLayer(.bottom)
        XCTAssertEqual(c.hiddenLayers, [.silkscreen])
        c.hiddenLayers = [.copper(.bottom)]
        c.changeTraceLayer(.bottom)
        XCTAssertEqual(c.traceLayer, .bottom)
        XCTAssertTrue(c.hiddenLayers.isEmpty)
        c.traceLayer = .top
        c.hiddenLayers = [.copper(.bottom)]
        c.pickTraceLayer(.bottom)
        XCTAssertEqual([c.traceLayer], [.bottom])
        XCTAssertEqual(c.pcbTool, .trace)
        XCTAssertTrue(c.hiddenLayers.isEmpty)
        c.pickCutoutShape(.circle)
        XCTAssertEqual(c.cutoutShape, .circle)
        XCTAssertEqual(c.pcbTool, .cutout)

        c.traceLayersChanged([.top, .bottom])
        XCTAssertEqual(c.traceLayer, .bottom)
        c.traceLayersChanged([.top])
        XCTAssertEqual(c.traceLayer, .top, "a layer that left the board goes back to Top")
    }

    func testEditorMessagesExpire() async {
        let c = make()
        c.editorMessageHold = .milliseconds(10)
        await c.expireEditorMessage()
        XCTAssertNil(c.document.editorMessage)
        c.document.editorMessage = "Refused"
        await c.expireEditorMessage()
        XCTAssertNil(c.document.editorMessage)

        c.editorMessageHold = .seconds(5)
        c.document.editorMessage = "Kept"
        let task = Task { await c.expireEditorMessage() }
        task.cancel()
        await task.value
        XCTAssertEqual(c.document.editorMessage, "Kept", "a newer message cancelled the wait")
    }

    func testTheRemovalConfirm() {
        let c = make()
        XCTAssertFalse(c.isConfirmingRemoval)
        c.document.pendingRemoval = .init(message: "Delete it?", action: .port(UUID()))
        XCTAssertTrue(c.isConfirmingRemoval)
        c.isConfirmingRemoval = true
        XCTAssertNotNil(c.document.pendingRemoval)
        c.isConfirmingRemoval = false
        XCTAssertNil(c.document.pendingRemoval, "closing the alert cancels")
    }

    // MARK: Zoom

    func testZoomActsOnThePageShown() {
        let c = make()
        XCTAssertTrue(c.activeViewport === c.viewport)
        let start = c.viewport.zoom
        c.zoomIn()
        XCTAssertGreaterThan(c.viewport.zoom, start)
        c.zoomOut()
        XCTAssertEqual(c.viewport.zoom, start)
        c.page = .pcb
        XCTAssertTrue(c.activeViewport === c.boardViewport)
        XCTAssertEqual(c.boardViewport.zoom, 0.25)
        c.setZoom(2)
        XCTAssertEqual(c.boardViewport.zoom, 2)
        XCTAssertEqual(c.viewport.zoom, start)
        XCTAssertEqual(ContentViewController.percent(0.75), "75%")
        XCTAssertEqual(ContentViewController.percent(1.5), "150%")
    }

    // MARK: Autoroute

    /// Two LEDs wired K to A, with an autorouted trace between their pads.
    private func routedPair() throws -> (doc: SchematicDocument, trace: UUID) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        doc.moveFootprint(one.id, to: p(30, 40))
        doc.moveFootprint(two.id, to: p(60, 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        func centre(_ ref: PinRef) throws -> CGPoint {
            let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
            let designator = chip.part.pins[ref.index].designator
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
            return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        }
        let k = try centre(kRef), a = try centre(aRef)
        let problem = doc.routingProblem(options: .standard, rules: .customDefault)
        let net = try XCTUnwrap(problem.nets.first)
        var result = RoutingResult()
        result.traces = [RoutedTrace(layer: .top, width: net.width, points: [k, p(k.x, k.y + 6), p(a.x, k.y + 6), a],
                                     net: net.id)]
        result.nets = [NetOutcome(net: net.id, name: net.name, status: .complete, unroutedConnections: 0)]
        doc.applyRouting(result, problem: problem)
        return (doc, try XCTUnwrap(doc.traces.first { $0.auto }).id)
    }

    func testRouteActions() throws {
        let (doc, trace) = try routedPair()
        let c = make(doc)
        c.page = .pcb
        XCTAssertNil(c.model.autoroute)
        let made = c.autorouteModel, again = c.autorouteModel
        XCTAssertFalse(made === again, "made, not kept, until the sheet opens")
        XCTAssertNil(c.routePreview)
        XCTAssertFalse(c.isRouting)

        doc.clearBoardSelection()
        var actions = c.routeActions
        XCTAssertFalse(actions.hasSelection)
        XCTAssertFalse(actions.isRouting)
        actions.selectAutorouted()
        XCTAssertTrue(doc.isSelected(.trace(trace)))
        actions = c.routeActions
        XCTAssertTrue(actions.hasSelection)
        actions.unrouteSelected()
        XCTAssertFalse(doc.traces.contains { $0.id == trace })
        XCTAssertTrue(doc.undo())
        actions.unrouteAllAutorouted()
        XCTAssertFalse(doc.traces.contains { $0.auto })
        XCTAssertTrue(doc.undo())

        actions.autoroute()
        let model = try XCTUnwrap(c.model.autoroute)
        XCTAssertTrue(c.autorouteModel === model, "kept once opened")
        XCTAssertEqual(model.scopeChoice, .all)
        XCTAssertTrue(c.isAutorouteOpen)
        c.closeAutoroute()
        XCTAssertFalse(c.isAutorouteOpen)

        doc.select(.trace(trace))
        actions.routeSelected()
        XCTAssertEqual(model.scopeChoice, .selection)
        XCTAssertTrue(c.isAutorouteOpen)
        try waitForTheRun(c)
        c.closeAutoroute()

        let net = PlaneNet.pin(chip: doc.chips[0].id, designator: "1")
        c.highlight(net)
        XCTAssertEqual(c.highlightedNet, net)
    }

    func testRouteRoutesAScopeUnlessPlayingOrRouting() throws {
        let (doc, _) = try routedPair()
        let c = make(doc)
        doc.isPlaying = true
        c.route(.all)
        XCTAssertFalse(c.isAutorouteOpen, "not while playing")
        doc.isPlaying = false
        c.route(.all)
        XCTAssertTrue(c.isAutorouteOpen)
        XCTAssertEqual(c.model.autoroute?.scopeChoice, .scope(.all))
        if c.isRouting {
            XCTAssertNotNil(c.routePreview)
            c.isAutorouteOpen = false
            c.route(.all)
            XCTAssertFalse(c.isAutorouteOpen, "not while a run is under way")
        }
        try waitForTheRun(c)
    }

    /// Waits up to 30 s for an Autoroute run to finish.
    private func waitForTheRun(_ c: ContentViewController) throws {
        let deadline = Date().addingTimeInterval(30)
        while c.isRouting && Date() < deadline { spin(0.05) }
        XCTAssertFalse(c.isRouting, "the run finished")
    }
}
