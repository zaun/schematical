import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AC: what the Schematic sidebar shows for each selection, as it is
/// shown, and the text block panel's edits.
@MainActor
final class SchematicSidebarViewTests: XCTestCase {
    private var probes: [ViewProbe] = []
    private static var keptSimulations: [SimulationController] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        for simulation in Self.keptSimulations where simulation.isRunning { simulation.stop() }
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    private func show(_ doc: SchematicDocument, simulation: SimulationController? = nil, isActive: Bool? = nil)
        -> ViewProbe {
        let simulation = simulation ?? self.simulation()
        // Without `isActive`, as the Schematic page shows it; with it, as the PCB page keeps it.
        let sidebar = if let isActive {
            SchematicSidebarView(catalog: TestSupport.catalog, document: doc, simulation: simulation, isActive: isActive)
        } else {
            SchematicSidebarView(catalog: TestSupport.catalog, document: doc, simulation: simulation)
        }
        let probe = ViewProbe(sidebar, size: NSSize(width: 380, height: 760))
        probes.append(probe)
        return probe
    }

    func testAPartShowsItsHeaderOptionsDetailsAndDatasheet() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        doc.select(.chip(gate.id))
        let probe = show(doc)
        XCTAssertTrue(probe.has("Details"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Options"), "\(probe.labels())")
        XCTAssertTrue(probe.has(gate.reference), "\(probe.labels())")
        XCTAssertNotNil(gate.part.datasheetMarkdown)
        XCTAssertTrue(probe.has("Show Datasheet", role: "AXButton"), "\(probe.labels())")
        for row in PartFacts.rows(for: gate) { XCTAssertTrue(probe.has(row.label), row.label) }
    }

    func testAPartWithOnlyAPDFDatasheetOpensIt() throws {
        let doc = TestSupport.newDocument()
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.datasheetMarkdown == nil && $0.datasheetURL != nil
            && !PartOptionsSection.applies(to: $0) && !PartOptionsSection.isSocketable($0, in: TestSupport.catalog) })
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 100, y: 100))
        doc.select(.chip(chip.id))
        let probe = show(doc)
        XCTAssertTrue(probe.has("Open Datasheet", role: "AXButton"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Options"), "\(part.name) has no options: \(probe.labels())")
    }

    func testARunningTimerShowsItsTiming() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 700))
        let timer = try TestSupport.place(doc, "GENERIC-555", at: CGPoint(x: 400, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(vcc, "VCC")), .pin(try TestSupport.pin(timer, "VCC"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gnd, "GND")), .pin(try TestSupport.pin(timer, "GND"))))
        doc.select(.chip(timer.id))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        XCTAssertFalse(try XCTUnwrap(simulation.timerSections[timer.id]).isEmpty)
        let probe = show(doc, simulation: simulation)
        XCTAssertTrue(probe.has("Timing"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Mode"), "\(probe.labels())")
        simulation.stop()
    }

    func testTheTimingSectionPerMode() {
        let sections = [
            TimerSection(mode: TIMER_ASTABLE, reason: TIMER_OK, firstPs: 0, highPs: 762_461_899, lowPs: 693_147_181),
            TimerSection(mode: TIMER_ONE_SHOT, reason: TIMER_OK, firstPs: 0, highPs: 1_098_612_289, lowPs: 0),
            TimerSection(mode: TIMER_STOPPED, reason: TIMER_SHAPE, firstPs: 0, highPs: 0, lowPs: 0),
            TimerSection(mode: TIMER_LATCH, reason: TIMER_OK, firstPs: 0, highPs: 0, lowPs: 0),
        ]
        let probe = ViewProbe(Form { TimingSection(sections: sections) }.formStyle(.grouped),
                              size: NSSize(width: 420, height: 900))
        probes.append(probe)
        for index in 1...4 { XCTAssertTrue(probe.has("Timing: Timer \(index)"), "\(probe.labels())") }
        XCTAssertTrue(probe.has(TimerText.frequency(sections[0])), "\(probe.labels())")
        XCTAssertTrue(probe.has("Duty"))
        XCTAssertTrue(probe.has("Pulse"))
        XCTAssertTrue(probe.has(TimerText.reason(TIMER_SHAPE)), "\(probe.labels())")
        XCTAssertTrue(probe.has("Latch"))

        let one = ViewProbe(Form { TimingSection(sections: [sections[1]]) }.formStyle(.grouped))
        probes.append(one)
        XCTAssertTrue(one.has("Timing"), "\(one.labels())")
        XCTAssertFalse(one.has("Timer 1"))
    }

    func testTheOtherPanels() throws {
        let doc = TestSupport.newDocument()
        // Nothing selected: the library.
        let library = show(doc)
        XCTAssertFalse(library.has("Details"))
        library.close()

        doc.setTitleBlockShown(true)
        doc.select(.titleBlock)
        let title = show(doc)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .titleBlock)
        XCTAssertTrue(title.has("Title"), "\(title.labels())")

        let label = doc.addNetLabel(at: CGPoint(x: 300, y: 300))
        doc.select(.label(label))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .label(label))
        title.settle()
        XCTAssertTrue(title.has("Net Label") || title.has("Label"), "\(title.labels())")

        let bus = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 500), to: CGPoint(x: 400, y: 500)))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .bus(bus))
        title.settle()
        XCTAssertTrue(title.has("BUS 1"), "\(title.labels())")
        XCTAssertNil(doc.addPort(at: CGPoint(x: 300, y: 500)))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .port(try XCTUnwrap(doc.selectedPortID)))
        title.settle()
        XCTAssertTrue(title.has("Port"), "\(title.labels())")
    }

    func testSeveralItemsShowTheSelectionList() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let right = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        doc.select(.chip(left.id))
        doc.toggleSelection(.chip(right.id))
        let probe = show(doc)
        XCTAssertTrue(probe.has("2 selected items"), "\(probe.labels())")
        XCTAssertTrue(probe.has(right.reference), "\(probe.labels())")
        try probe.press(right.reference)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(right.id)], "a row selects only its item")

        doc.toggleSelection(.chip(left.id))
        probe.settle()
        try probe.pressControl("Clear selection")
        XCTAssertTrue(doc.schematicSelection.isEmpty)

        // On the PCB page the sidebar keeps only the library, whatever is selected.
        doc.select(.chip(left.id))
        let inactive = show(doc, isActive: false)
        XCTAssertFalse(inactive.has("Details"), "\(inactive.labels())")
    }

    // MARK: Text block

    func testATextBlockEditsItsWordsSizeAndFrame() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(doc.editingNoteID, id)
        let probe = show(doc)
        let note = { try XCTUnwrap(doc.notes.first { $0.id == id }) }
        // A block just placed takes focus, and the request is used up.
        XCTAssertNil(doc.editingNoteID)
        let editor = try XCTUnwrap(probe.views(NSTextView.self).first)
        XCTAssertTrue(probe.wait { probe.window.firstResponder === editor }, "the words take focus")
        XCTAssertEqual(editor.string, try note().text)

        // Typing applies at once.
        editor.selectAll(nil)
        editor.insertText("Power in", replacementRange: editor.selectedRange())
        probe.settle()
        XCTAssertEqual(try note().text, "Power in")

        // Emptied and left: the block keeps its words and the editor shows them again.
        editor.selectAll(nil)
        editor.delete(nil)
        probe.settle()
        XCTAssertEqual(try note().text, "Power in")
        probe.window.makeFirstResponder(nil)
        probe.settle()
        XCTAssertEqual(editor.string, "Power in")

        // Undo changes the words from outside the editor.
        doc.setNoteText(id, "Ground")
        probe.settle()
        XCTAssertEqual(editor.string, "Ground")

        let size = try note().size
        try probe.step(up: true)
        XCTAssertEqual(try note().size, size + 1)
        XCTAssertTrue(probe.has("\(size + 1) pt"), "\(probe.labels())")
        let framed = try note().framed
        try probe.pressControl("Frame")
        XCTAssertEqual(try note().framed, !framed)
    }

    func testADoubleClickedBlockTakesFocusWhileShown() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: CGPoint(x: 100, y: 100))
        doc.editingNoteID = nil
        let probe = show(doc)
        let editor = try XCTUnwrap(probe.views(NSTextView.self).first)
        probe.window.makeFirstResponder(nil)
        probe.settle()
        XCTAssertFalse(probe.window.firstResponder === editor)
        // Another block's request is not this panel's.
        doc.editingNoteID = UUID()
        probe.settle()
        XCTAssertNotNil(doc.editingNoteID)
        doc.editingNoteID = id
        XCTAssertTrue(probe.wait { probe.window.firstResponder === editor }, "the words take focus")
        XCTAssertNil(doc.editingNoteID)
    }
}
