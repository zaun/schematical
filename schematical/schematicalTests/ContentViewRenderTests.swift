import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AA: a schematic window's editor (`ContentView`) as it is shown:
/// hosted the way its document hosts it, read through the accessibility
/// tree, its sheets and alerts answered in place.
@MainActor
final class ContentViewRenderTests: XCTestCase {
    nonisolated deinit {}

    private static var kept: [AnyObject] = []
    private var windows: [NSWindow] = []
    private var savedDefaults: [String: Any?] = [:]

    override func setUp() {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        for window in windows {
            if let sheet = window.attachedSheet { window.endSheet(sheet) }
            (window.contentViewController as? NSHostingController<AnyView>)?.rootView = AnyView(EmptyView())
            window.close()
            window.contentViewController = nil
        }
        windows = []
        for (key, value) in savedDefaults { UserDefaults.standard.set(value ?? nil, forKey: key) }
        savedDefaults = [:]
    }

    /// Puts the owner's value of `key` back after the test.
    private func preserve(_ key: String) {
        if savedDefaults.index(forKey: key) == nil { savedDefaults[key] = UserDefaults.standard.object(forKey: key) }
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// An editor in a window, hosted as `SchematicNSDocument` hosts it.
    private func show(_ doc: SchematicDocument? = nil) -> (ContentViewController, ViewProbe) {
        let model = SchematicEditorModel(catalog: TestSupport.catalog, document: doc ?? TestSupport.newDocument())
        let controller = ContentViewController(model: model, catalog: TestSupport.catalog)
        controller.editorMessageHold = .milliseconds(200)
        controller.angleFlashHold = .milliseconds(100)
        Self.kept.append(model)
        Self.kept.append(controller)
        let host = NSHostingController(rootView: AnyView(ContentView(controller: controller)))
        host.sceneBridgingOptions = [.toolbars]
        let window = NSWindow(contentViewController: host)
        window.isReleasedWhenClosed = false
        window.setContentSize(NSSize(width: 1700, height: 800))
        window.toolbarStyle = .unifiedCompact
        model.file.window = window
        windows.append(window)
        addTeardownBlock { @MainActor in
            controller.simulation.stop()
            ChildWindowCenter.shared.closeAll(model.id)
        }
        let probe = ViewProbe(window: window)
        probe.settle(0.6)
        return (controller, probe)
    }

    /// The sheet on `window`, read through its own probe.
    private func sheet(of window: NSWindow, file: StaticString = #filePath, line: UInt = #line) throws -> ViewProbe {
        let deadline = Date().addingTimeInterval(3)
        while window.attachedSheet == nil && Date() < deadline { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        return ViewProbe(window: try XCTUnwrap(window.attachedSheet, "no sheet", file: file, line: line))
    }

    private func waitForNoSheet(_ window: NSWindow) {
        let deadline = Date().addingTimeInterval(3)
        while window.attachedSheet != nil && Date() < deadline { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
    }

    // MARK: The document's window

    /// The window a document makes hosts this editor, and closing the
    /// document takes its menus and key monitor away.
    func testTheDocumentWindowHostsTheEditor() throws {
        let document = TestSupport.keep(SchematicNSDocument())
        NSDocumentController.shared.addDocument(document)
        document.makeWindowControllers()
        let window = try XCTUnwrap(document.windowControllers.first?.window)
        let probe = ViewProbe(window: window)
        probe.settle(0.6)
        XCTAssertTrue(probe.has("Parts Drawer"), "\(probe.labels())")
        XCTAssertNotNil(document.model.menus.edit, "the menus are this window's")
        XCTAssertNotNil(document.model.editKeyMonitor)
        document.close()
        XCTAssertNil(document.model.menus.edit)
    }

    // MARK: Pages and the toolbar

    func testTheSchematicToolbar() throws {
        let (c, probe) = show()
        XCTAssertTrue(probe.has("Parts Drawer"), "\(probe.labels())")
        XCTAssertTrue(probe.has(c.supplyVoltage), "the supply menu")
        XCTAssertTrue(probe.has("100%"), "the zoom menu")
        let zoom = c.viewport.zoom
        try probe.press("Zoom In")
        XCTAssertGreaterThan(c.viewport.zoom, zoom)
        try probe.press("Zoom Out")
        XCTAssertEqual(c.viewport.zoom, zoom)
        XCTAssertFalse(probe.has("Rotate", role: "AXButton"), "Rotate is the PCB's")
    }

    func testThePCBToolbar() throws {
        let (c, probe) = show()
        let led = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.page = .pcb
        probe.settle(0.6)
        XCTAssertTrue(probe.has("Inspector"), "\(probe.labels())")
        XCTAssertTrue(probe.has("25%"))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Rotate", role: "AXButton")), "nothing selected")
        c.document.select(.footprint(led.id))
        probe.settle()
        let board = c.document.chips.first?.board
        try probe.press("Rotate")
        XCTAssertNotEqual(c.document.chips.first?.board, board)

        try probe.press("Preview")
        XCTAssertEqual(ChildWindowCenter.shared.windows(of: c.documentID).map(\.title), ["Board Preview"])
        try probe.press("Autoroute")
        let sheet = try sheet(of: probe.window)
        XCTAssertTrue(c.isAutorouteOpen)
        XCTAssertEqual(c.model.autoroute?.scopeChoice, .all)
        XCTAssertFalse(sheet.labels().isEmpty)
        c.closeAutoroute()
        waitForNoSheet(probe.window)
        XCTAssertNil(probe.window.attachedSheet)

        try probe.press("View")
        XCTAssertTrue(c.isLayerViewShown)
        let popover = NSApp.windows.first { $0.isVisible && ViewProbe(window: $0).has("Show All") }
        XCTAssertNotNil(popover, "the layer popover")
        c.isLayerViewShown = false
        probe.settle()
    }

    func testThePageChangeStopsTheSimulation() throws {
        let (c, probe) = show()
        _ = try TestSupport.place(c.document, "GENERIC-LED", at: p(200, 200))
        c.simulation.start(document: c.document)
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        c.page = .pcb
        probe.settle()
        XCTAssertFalse(c.simulation.isRunning)
    }

    func testTheSpeedPreferenceReachesTheSimulation() {
        preserve("simulationSpeed")
        let (c, probe) = show()
        UserDefaults.standard.set(0.5, forKey: "simulationSpeed")
        probe.settle()
        XCTAssertEqual(c.simulation.speed, 0.5)
    }

    func testATraceLayerThatLeavesTheBoardGoesBackToTop() {
        let (c, probe) = show()
        c.document.setLayerCount(2)
        c.page = .pcb
        probe.settle()
        c.traceLayer = .bottom
        c.document.setLayerCount(1)
        probe.settle()
        XCTAssertEqual(c.traceLayer, .top)
    }

    /// "running at …×" in the toolbar while the circuit can't keep up.
    func testTheSlowDownNotice() throws {
        XCTAssertEqual(ReachedSpeedLabel.formatSpeed(0.0781), "0.078")
        XCTAssertEqual(ReachedSpeedLabel.formatSpeed(12), "12")
        let (c, probe) = show()
        XCTAssertNil(ReachedSpeedLabel.text(for: c.simulation), "not running")
        // One NAND gate fed back on itself: it oscillates at the gate's delay.
        let gate = try TestSupport.place(c.document, "SN74HC00N", at: p(200, 200))
        let y = try TestSupport.pin(gate, "1Y")
        XCTAssertNil(c.document.connect(.pin(y), .pin(try TestSupport.pin(gate, "1A"))))
        XCTAssertNil(c.document.connect(.pin(y), .pin(try TestSupport.pin(gate, "1B"))))
        c.simulation.start(document: c.document)
        XCTAssertTrue(c.simulation.isRunning, c.simulation.errorMessage ?? "")
        c.simulation.setSpeed(1e5)
        let deadline = Date().addingTimeInterval(5)
        while ReachedSpeedLabel.text(for: c.simulation) == nil && Date() < deadline { probe.settle(0.1) }
        let text = try XCTUnwrap(ReachedSpeedLabel.text(for: c.simulation), "far below the chosen speed")
        probe.settle()
        XCTAssertTrue(probe.has("running at"), "\(text): \(probe.labels())")
    }

    // MARK: Messages

    func testARefusedEditSaysWhyThenClears() throws {
        let (c, probe) = show()
        c.document.editorMessage = "Not here"
        probe.settle(0.1)
        XCTAssertTrue(probe.has("Not here"), "\(probe.labels())")
        probe.settle(0.5)
        XCTAssertNil(c.document.editorMessage)
        XCTAssertFalse(probe.has("Not here"))
    }

    func testASimulationErrorIsShown() throws {
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent("ContentViewRenderTests-\(UUID())")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: folder) }
        let doc = TestSupport.newDocument()
        let eeprom = try TestSupport.place(doc, "AT28C256-15PU", at: p(300, 300))
        // A load file the test made and then removed.
        let rom = folder.appendingPathComponent("rom.bin")
        try Data([0xEA]).write(to: rom)
        doc.setMemoryFile(eeprom.id, MemoryFile(url: rom, bookmark: nil))
        try FileManager.default.removeItem(at: rom)
        let (c, probe) = show(doc)
        c.simulation.start(document: doc)
        probe.settle()
        let message = try XCTUnwrap(c.simulation.errorMessage)
        XCTAssertTrue(probe.has("Simulation stopped: \(message)"), "\(probe.labels())")
    }

    func testTheAngleFlashDrawsOverThePCB() throws {
        let (c, probe) = show()
        c.page = .pcb
        probe.settle()
        func centre() throws -> NSBitmapImageRep {
            let view = try XCTUnwrap(probe.window.contentView)
            let rep = try XCTUnwrap(view.bitmapImageRepForCachingDisplay(in: view.bounds))
            view.cacheDisplay(in: view.bounds, to: rep)
            return rep
        }
        let before = try centre()
        c.flashAngle(45)
        probe.settle(0.05)
        let during = try centre()
        XCTAssertNotEqual(before.tiffRepresentation, during.tiffRepresentation, "the angle is drawn")
        probe.settle(0.6)
        XCTAssertEqual(c.angleFlashOpacity, 0, "faded")
    }

    // MARK: Alerts and sheets

    func testResetReferencesAsksFirst() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        XCTAssertNil(doc.renameChip(led.id, prefix: "D", number: 7, page: .schematic))
        let (c, probe) = show(doc)
        c.editActions.resetReferences()
        var alert = try sheet(of: probe.window)
        XCTAssertTrue(alert.has("Reset References?"), "\(alert.labels())")
        try alert.press("Cancel")
        waitForNoSheet(probe.window)
        XCTAssertEqual(doc.chips.first?.reference, "D7")
        c.isConfirmingResetReferences = true
        alert = try sheet(of: probe.window)
        try alert.press("Reset")
        waitForNoSheet(probe.window)
        XCTAssertEqual(doc.chips.first?.reference, "D1")
    }

    func testTheCustomVoltageDialog() throws {
        preserve(ContentViewController.supplyVoltageKey)
        let (c, probe) = show()
        c.askCustomVoltage()
        var alert = try sheet(of: probe.window)
        XCTAssertTrue(alert.has("Custom Supply Voltage"), "\(alert.labels())")
        XCTAssertTrue(alert.has(c.customVoltageMessage))
        c.customVoltageText = "nonsense"
        probe.settle()
        try alert.press("Use")
        waitForNoSheet(probe.window)
        XCTAssertTrue(c.customVoltageRefused)
        alert = try sheet(of: probe.window)
        XCTAssertTrue(alert.has("That is not a voltage"), "asked again: \(alert.labels())")
        try alert.press("Cancel")
        waitForNoSheet(probe.window)
    }

    func testTheRemovalConfirm() throws {
        let (c, probe) = show()
        c.document.pendingRemoval = .init(message: "Delete the port?", action: .port(UUID()))
        var alert = try sheet(of: probe.window)
        XCTAssertTrue(alert.has("Delete the port?"), "\(alert.labels())")
        try alert.press("Cancel")
        waitForNoSheet(probe.window)
        XCTAssertNil(c.document.pendingRemoval)
        c.document.pendingRemoval = .init(message: "Delete the port?", action: .port(UUID()))
        alert = try sheet(of: probe.window)
        try alert.press("Delete")
        waitForNoSheet(probe.window)
        XCTAssertNil(c.document.pendingRemoval)
    }

    // MARK: The layer popover

    /// `LayerViewPopover` over its own state, reporting each change.
    private struct PopoverHost: View {
        let board: BoardSettings
        @State var hidden: Set<BoardViewLayer>
        let changed: (Set<BoardViewLayer>) -> Void

        var body: some View {
            LayerViewPopover(board: board, hidden: $hidden)
                .onChange(of: hidden) { _, value in changed(value) }
        }
    }

    func testTheLayerPopoverHidesAndShowsLayers() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        var hidden: Set<BoardViewLayer> = []
        let probe = ViewProbe(PopoverHost(board: doc.board, hidden: [], changed: { hidden = $0 }),
                              size: NSSize(width: 220, height: 320))
        defer { probe.close() }
        for title in ["Top", "Bottom", "PCB", "Silkscreen", "Document", "Ratsnest"] {
            XCTAssertTrue(probe.has(title), "\(title): \(probe.labels())")
        }
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Show All", role: "AXButton")), "nothing hidden")
        let silk = try probe.element("Silkscreen", role: "AXCheckBox")
        _ = silk.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(hidden, [.silkscreen])
        _ = silk.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(hidden, [], "shown again")
        _ = try probe.element("Ratsnest", role: "AXCheckBox").perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(hidden, [.ratsnest])
        try probe.press("Show All")
        XCTAssertEqual(hidden, [])
    }
}
