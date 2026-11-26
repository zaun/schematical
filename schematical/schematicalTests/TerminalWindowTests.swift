import AppKit
import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-X: the terminal window (TerminalWindow.swift): frame text, the
/// log, the controller the view drives, and what the window shows.
@MainActor
final class TerminalWindowTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private static var keptControllers: [TerminalController] = []
    private var probes: [ViewProbe] = []

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        try await super.tearDown()
    }

    private func probe<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view)
        probes.append(probe)
        return probe
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    private func controller(_ chip: PlacedChip, _ doc: SchematicDocument,
                            _ simulation: SimulationController) -> TerminalController {
        let controller = TerminalController(chipID: chip.id, document: doc, simulation: simulation)
        Self.keptControllers.append(controller)
        return controller
    }

    private func settle(_ seconds: TimeInterval = 0.5) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    /// A one-channel USB bridge, powered, its TX wired back to its RX: what
    /// the terminal types comes back into its transcript.
    private func loopback() throws -> (doc: SchematicDocument, bridge: PlacedChip) {
        let doc = TestSupport.newDocument()
        let supply = try TestSupport.place(doc, "3V3", at: CGPoint(x: 60, y: 60))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let bridge = try TestSupport.place(doc, "XR21V1410IL16-F", at: CGPoint(x: 400, y: 300))
        for pin in ["VCC", "VUSB"] where bridge.layout.slots.contains(where: { $0.pin.name == pin }) {
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(supply, "VCC")), .pin(try TestSupport.pin(bridge, pin))))
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(ground, "GND")), .pin(try TestSupport.pin(bridge, "GND"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(bridge, "TX")), .pin(try TestSupport.pin(bridge, "RX"))))
        return (doc, bridge)
    }

    /// Waits up to `seconds` for `condition`.
    private func wait(_ seconds: TimeInterval = 3, until condition: () -> Bool) -> Bool {
        let end = Date().addingTimeInterval(seconds)
        while Date() < end {
            if condition() { return true }
            settle(0.05)
        }
        return condition()
    }

    // MARK: Text

    func testFramesRenderAsText() {
        XCTAssertEqual(TerminalText.render([0x48, 0x69, 0x0D, 0x0A, 0x41, 0x0A, 0x42, 0x0D, 0x43]), "Hi\nA\nB\nC")
        XCTAssertEqual(TerminalText.render([0x01, 0x1FF, 0x7F]), "⟨01⟩⟨1FF⟩⟨7F⟩")
        XCTAssertEqual(TerminalText.render([0x0D, 0x1FF, 0x0A]), "\n⟨1FF⟩\n", "a wide frame ends a CR LF pair")
        XCTAssertEqual(TerminalText.channelName(0), "A")
        XCTAssertEqual(TerminalText.channelName(2), "C")
    }

    /// A controller let go is freed (its deinit is nonisolated, plan-68-X).
    func testAControllerIsFreed() {
        weak var gone: TerminalController?
        do {
            let controller = TerminalController(chipID: UUID(), document: TestSupport.newDocument(),
                                                simulation: simulation())
            gone = controller
            XCTAssertEqual(controller.channels, 0)
        }
        XCTAssertNil(gone)
    }

    func testAnEmptyLogHasNoText() {
        let log = TerminalLog()
        XCTAssertEqual(log.text(channel: 0), "")
        log.clear(channel: 0)
        XCTAssertEqual(log.text(channel: 0), "", "clearing a channel never read does nothing")
    }

    // MARK: The controller

    func testControllerReadsThePart() throws {
        let doc = TestSupport.newDocument()
        let dual = try TestSupport.place(doc, "XR21V1412IL32-F", at: CGPoint(x: 300, y: 300))
        let uart = try TestSupport.place(doc, "N82510", at: CGPoint(x: 900, y: 300))
        let simulation = simulation()
        let bridge = controller(dual, doc, simulation)
        XCTAssertEqual(bridge.chip?.id, dual.id)
        XCTAssertEqual(bridge.channels, 2)
        XCTAssertTrue(bridge.isUSBBridge)
        let plain = controller(uart, doc, simulation)
        XCTAssertEqual(plain.channels, 1)
        XCTAssertFalse(plain.isUSBBridge)
        let gone = TerminalController(chipID: UUID(), document: doc, simulation: simulation)
        Self.keptControllers.append(gone)
        XCTAssertNil(gone.chip)
        XCTAssertEqual(gone.channels, 0)
        XCTAssertFalse(gone.isUSBBridge)
    }

    func testControllerSetsThePortPerChannel() throws {
        let doc = TestSupport.newDocument()
        let dual = try TestSupport.place(doc, "XR21V1412IL32-F", at: CGPoint(x: 300, y: 300))
        let controller = controller(dual, doc, simulation())
        controller.channel = 1
        controller.port.baud = 115200
        controller.formatName = "7E1"
        controller.port.rtsCts = true
        XCTAssertNil(controller.error)
        XCTAssertEqual(controller.port.baud, 115200)
        XCTAssertEqual(controller.formatName, "7E1")
        XCTAssertTrue(controller.port.rtsCts)
        controller.formatName = "9X9"
        XCTAssertEqual(controller.formatName, "7E1", "an unknown format changes nothing")
        controller.channel = 0
        XCTAssertEqual(controller.port.formatName, "8N1", "channel A kept its own port")
    }

    func testTypingLoopsBackIntoTheTranscript() throws {
        let (doc, bridge) = try loopback()
        let simulation = simulation()
        let controller = controller(bridge, doc, simulation)
        controller.input = "hi"
        controller.send("hi", carriageReturn: true)
        XCTAssertNotNil(controller.error, "refused while stopped")
        XCTAssertEqual(controller.input, "hi", "a refused line stays")

        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        controller.runningChanged(true)
        XCTAssertNil(controller.error, "a new run clears the refusal")
        controller.runningChanged(false)

        controller.send("hi", carriageReturn: true)
        XCTAssertNil(controller.error)
        XCTAssertEqual(controller.input, "", "a sent line empties")
        XCTAssertTrue(wait { controller.transcript == "hi\n" }, controller.transcript)
        controller.clear()
        XCTAssertEqual(controller.transcript, "")
        controller.send("ok", carriageReturn: false)
        XCTAssertTrue(wait { controller.transcript == "ok" }, controller.transcript)

        // A new run's log starts again below what was read: the transcript empties.
        simulation.stop()
        XCTAssertEqual(controller.transcript, "ok", "kept while stopped")
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        settle(0.2)
        XCTAssertEqual(controller.transcript, "")
        simulation.stop()
    }

    // MARK: The window

    func testWindowOfAClosedSchematic() {
        let probe = probe(TerminalWindowView(target: TerminalWindowTarget(documentID: UUID(), chipID: UUID())))
        XCTAssertTrue(probe.has("Schematic Closed"), "\(probe.labels())")
    }

    func testWindowOfABridgeShowsThePortAndChannels() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let dual = try TestSupport.place(doc, "XR21V1412IL32-F", at: CGPoint(x: 300, y: 300))
        let probe = probe(TerminalWindowView(target: TerminalWindowTarget(documentID: open.model.id, chipID: dual.id)))
        XCTAssertTrue(probe.has("Baud"), "\(probe.labels())")
        XCTAssertTrue(probe.has("RTS/CTS"))
        XCTAssertTrue(probe.has("8N1"))
        XCTAssertTrue(probe.has("B"), "the channel picker")
        let field = try XCTUnwrap(probe.views(NSTextField.self).first { $0.isEditable }, "no text field")
        XCTAssertEqual(field.placeholderString, "Run the simulation to type")
        XCTAssertTrue(doc.undo(), "undo the placement")
        probe.settle()
        XCTAssertTrue(probe.has("Part Removed"), "\(probe.labels())")
    }

    /// Typing Return in the field sends the line; it loops back into the
    /// transcript, which the window shows; Clear empties it.
    func testWindowTypesAndShowsTheTranscript() throws {
        let (doc, bridge) = try loopback()
        let simulation = simulation()
        let probe = probe(TerminalInspector(chipID: bridge.id, document: doc, simulation: simulation))
        XCTAssertFalse(probe.has("Channel"), "one channel: no picker")
        let field = try XCTUnwrap(probe.views(NSTextField.self).first { $0.isEditable }, "no text field")
        XCTAssertFalse(field.isEnabled, "no typing while stopped")
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertEqual(field.placeholderString, "Type, then Return to send")

        // Typed through the field editor, so the field's binding sees it.
        XCTAssertTrue(probe.window.makeFirstResponder(field))
        let editor = try XCTUnwrap(field.currentEditor() as? NSTextView, "no field editor")
        editor.insertText("abc", replacementRange: NSRange(location: NSNotFound, length: 0))
        probe.settle()
        field.sendAction(field.action, to: field.target)
        // The transcript's own text (the field also shows what was typed).
        XCTAssertTrue(wait { probe.labels().contains("abc\n") }, "\(probe.labels())")
        try probe.press("Clear")
        XCTAssertFalse(probe.labels().contains("abc\n"), "Clear redraws the transcript: \(probe.labels())")
        simulation.stop()
        probe.window.makeFirstResponder(nil)
    }

    /// A refusal shows under the field until the next run starts.
    func testWindowShowsARefusalUntilTheNextRun() throws {
        let (doc, bridge) = try loopback()
        let simulation = simulation()
        let probe = probe(TerminalInspector(chipID: bridge.id, document: doc, simulation: simulation))
        let field = try XCTUnwrap(probe.views(NSTextField.self).first { $0.isEditable }, "no text field")
        // Stopped: the field is disabled, but its action still refuses (as the controller does).
        field.stringValue = "x"
        field.sendAction(field.action, to: field.target)
        probe.settle()
        XCTAssertTrue(probe.has("The simulation is not running."), "\(probe.labels())")
        simulation.start(document: doc)
        probe.settle()
        XCTAssertFalse(probe.has("The simulation is not running."), "a run clears it: \(probe.labels())")
        simulation.stop()
    }
}
