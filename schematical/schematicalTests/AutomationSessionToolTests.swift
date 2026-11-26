import AppKit
import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-X: the branches of the automation plumbing (AutomationTools.swift),
/// the session tools (AutomationSession.swift) and the simulation tools
/// (AutomationSimulation.swift) the other automation tests leave: refusals,
/// exports, and switch, fuse and memory reads.
@MainActor
final class AutomationSessionToolTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private var folder: URL!
    private var unshare: (() -> Void)?

    override func setUp() async throws {
        folder = FileManager.default.temporaryDirectory
            .appendingPathComponent("AutomationSessionToolTests-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        folder = folder.resolvingSymlinksInPath()
        unshare = AutomationFolder.shared.shareTemporarily(folder)
    }

    override func tearDown() async throws {
        unshare?()
        unshare = nil
        try? FileManager.default.removeItem(at: folder)
    }

    private struct Result {
        let text: String
        let isError: Bool
    }

    private func newWindow() -> (id: String, doc: SchematicDocument, model: SchematicEditorModel) {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        return (open.model.id.uuidString, doc, open.model)
    }

    private func read(_ result: [String: Any]) -> Result {
        let content = result["content"] as? [[String: Any]] ?? []
        return Result(text: content.compactMap { $0["text"] as? String }.joined(separator: "\n"),
                      isError: result["isError"] as? Bool ?? false)
    }

    private func call(_ tool: String, _ window: String, _ args: [String: Any] = [:]) async -> Result {
        var args = args
        args["window"] = window
        return read(await AutomationTools.call(tool, args))
    }

    /// Runs a tool's body straight on `context` (as the dispatcher would).
    private func run(_ tool: String, _ context: AutomationContext?, _ args: [String: Any] = [:]) async -> Result {
        do {
            let content = try await XCTUnwrap(AutomationTools.byName[tool]).run(context, AutomationArgs(raw: args))
            return read(["content": content])
        } catch let failure as AutomationFailure {
            return Result(text: failure.message, isError: true)
        } catch {
            return Result(text: "\(error)", isError: true)
        }
    }

    /// A context with no system document and no exporter behind it.
    private func bareContext(_ doc: SchematicDocument) -> AutomationContext {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return AutomationContext(id: UUID(), document: doc, simulation: simulation, file: nil, model: nil)
    }

    private func place(_ id: String, _ parts: [(String, Int, Int)]) async {
        for (part, x, y) in parts {
            let result = await call("place_part", id, ["part": part, "x": x, "y": y])
            XCTAssertFalse(result.isError, "\(part): \(result.text)")
        }
    }

    private func connect(_ id: String, _ pairs: [(String, String)]) async {
        for (from, to) in pairs {
            let result = await call("connect", id, ["from": from, "to": to])
            XCTAssertFalse(result.isError, "\(from)→\(to): \(result.text)")
        }
    }

    // MARK: Plumbing

    func testArgumentRefusals() async {
        let (id, _, _) = newWindow()
        var result = await call("set_switch", id)
        XCTAssertEqual(result.text, "reference is required (a string)")
        result = await call("add_trace", id, ["points": [[1], [2, 3]]])
        XCTAssertEqual(result.text, "points must be [[x, y], …]")
        result = await call("batch", id, ["calls": [["tool": "get_design"]], "page": "both"])
        XCTAssertEqual(result.text, "page must be one of schematic, pcb, got both")
        result = await call("batch", id, ["calls": [Any]()])
        XCTAssertEqual(result.text, "calls must be a non-empty list of {tool, arguments}")
        result = read(await AutomationTools.call("no_such_tool", [:]))
        XCTAssertEqual(result.text, "unknown tool no_such_tool")
        result = await call("connect", id, ["from": "U1", "to": "U2.3"])
        XCTAssertEqual(result.text, "a pin looks like U1.14 or D1.K, got “U1”")
    }

    /// An error that is not an `AutomationFailure` (a cancelled wait) is
    /// reported as its description.
    func testAnyOtherErrorIsReported() async {
        let task = Task { @MainActor in await AutomationTools.call("wait", ["milliseconds": 5000]) }
        task.cancel()
        let result = read(await task.value)
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("CancellationError"), result.text)
    }

    /// Two windows, neither on screen: the tools can't tell which is in front.
    func testTwoWindowsOffScreenNeedAWindowId() async {
        _ = newWindow()
        _ = newWindow()
        let result = read(await AutomationTools.call("get_design", [:]))
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.hasSuffix("windows are open; pass window (see list_windows)"), result.text)
    }

    func testABusPinEndWithNoPortIsNamedBus() {
        let context = bareContext(TestSupport.newDocument())
        XCTAssertEqual(context.endName(.busPin(BusPinRef(portID: UUID(), wireID: UUID()))), "bus")
    }

    // MARK: Files

    func testListFiles() async throws {
        let (id, _, _) = newWindow()
        var result = await call("list_files", id)
        XCTAssertEqual(result.text, "no schematics in \(folder.path)")
        try Data().write(to: folder.appendingPathComponent("b.schematical"))
        try Data().write(to: folder.appendingPathComponent("a.schematical"))
        try Data().write(to: folder.appendingPathComponent("notes.txt"))
        result = await call("list_files", id)
        XCTAssertEqual(result.text, "in \(folder.path):\na.schematical\nb.schematical")
    }

    func testSaveRefusals() async {
        let (id, _, _) = newWindow()
        var result = await run("save", bareContext(TestSupport.newDocument()))
        XCTAssertEqual(result.text, "no document for this window")
        result = await call("save", id, ["path": "missing/folder/x.schematical"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.hasPrefix("Couldn't save “x.schematical”"), result.text)
    }

    func testExportGerber() async throws {
        let (id, _, _) = newWindow()
        await place(id, [("GENERIC-LED", 200, 200)])
        var result = await run("export_gerber", bareContext(TestSupport.newDocument()), ["path": "x.zip"])
        XCTAssertEqual(result.text, "no file controller for this window")
        result = await call("export_gerber", id, ["path": "board.zip"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(result.text, "wrote \(folder.appendingPathComponent("board.zip").path)")
        XCTAssertTrue(FileManager.default.fileExists(atPath: folder.appendingPathComponent("board.zip").path))
        result = await call("export_gerber", id, ["path": "missing/board.zip"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.hasPrefix("couldn't write board.zip"), result.text)
    }

    func testExportPdf() async throws {
        let (id, doc, _) = newWindow()
        await place(id, [("GENERIC-LED", 200, 200)])
        var result = await run("export_pdf", bareContext(doc), ["path": "x.pdf"])
        XCTAssertEqual(result.text, "no file controller for this window")
        result = await call("export_pdf", id, ["path": "x.pdf", "paper": "napkin"])
        XCTAssertEqual(result.text, "unknown paper napkin")
        result = await call("export_pdf", id, ["path": "x.pdf", "pages": ["schematic", "poster"]])
        XCTAssertEqual(result.text, "unknown page poster: use schematic, copper, parts, silkscreen or board")

        result = await call("export_pdf", id, ["path": "one.pdf", "paper": "a4", "pages": ["schematic"],
                                               "mirror": true, "negative": true, "marks": false])
        XCTAssertEqual(result.text, "wrote \(folder.appendingPathComponent("one.pdf").path) (1 page)")
        result = await call("export_pdf", id, ["path": "all.pdf",
                                               "pages": ["schematic", "copper", "parts", "silkscreen", "board"]])
        XCTAssertFalse(result.isError, result.text)
        var options = PrintOptions.saved()
        options.hidden = []
        let count = PrintExport.pages(for: doc, options).count
        XCTAssertGreaterThan(count, 1)
        XCTAssertTrue(result.text.hasSuffix("(\(count) pages)"), result.text)
        XCTAssertTrue(FileManager.default.fileExists(atPath: folder.appendingPathComponent("all.pdf").path))

        result = await call("export_pdf", id, ["path": "missing/x.pdf"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.hasPrefix("couldn't write x.pdf"), result.text)
    }

    // MARK: The simulation

    func testStartSpeed() async {
        let (id, _, model) = newWindow()
        var result = await call("simulation_start", id, ["speed": 0])
        XCTAssertEqual(result.text, "speed must be above 0")
        result = await call("simulation_start", id, ["speed": 0.5])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(model.simulation.speed, 0.5)
        _ = await call("simulation_stop", id)
    }

    func testAFuseBlowsOnlyWhileRunning() async throws {
        let (id, doc, _) = newWindow()
        let fuse = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_FUSE }, "no fuse")
        await place(id, [(fuse.name, 200, 200)])
        let reference = try XCTUnwrap(doc.chips.last?.reference)
        var result = await call("set_switch", id, ["reference": reference, "on": false])
        XCTAssertEqual(result.text, "the simulation isn't running; call simulation_start to blow a fuse")
        _ = await call("simulation_start", id)
        result = await call("set_switch", id, ["reference": reference, "on": false])
        XCTAssertEqual(result.text, "\(reference) is blown")
        result = await call("set_switch", id, ["reference": reference])
        XCTAssertEqual(result.text, "\(reference) is intact")
        _ = await call("simulation_stop", id)
    }

    func testAButtonPressAndRelease() async throws {
        let (id, doc, _) = newWindow()
        let button = try XCTUnwrap(TestSupport.allParts.first { $0.isTactileSwitch && !$0.isNormallyClosed })
        await place(id, [(button.name, 200, 200)])
        let reference = try XCTUnwrap(doc.chips.last?.reference)
        var result = await call("set_switch", id, ["reference": reference, "position": 2])
        XCTAssertEqual(result.text, "\(reference) has positions 1–1")
        result = await call("set_switch", id, ["reference": reference])
        XCTAssertEqual(result.text, "\(reference) pressed")
        XCTAssertEqual(doc.chips.last?.switchesOn, [true])
        result = await call("set_switch", id, ["reference": reference, "on": false])
        XCTAssertEqual(result.text, "\(reference) released")
    }

    /// 12 V through a switch onto a 74HC00's supply: every way of closing
    /// the switch is refused, with the voltage check's reason.
    func testSwitchesThatWouldOverVoltAreRefused() async throws {
        let (id, doc, _) = newWindow()
        let button = try XCTUnwrap(TestSupport.allParts.first { part in
            part.isTactileSwitch && !part.isNormallyClosed && Set(part.pins.map(\.name)) == ["A", "B"]
        })
        await place(id, [("12V", 100, 300), ("SN74HC00N", 600, 300), ("TE-GDS0204", 300, 100),
                         (button.name, 300, 500), ("1103M2S3CBE2", 300, 800)])
        let refs = doc.chips.map(\.reference)
        let (dip, tactile, slide) = (refs[2], refs[3], refs[4])
        XCTAssertEqual(doc.chips[4].slidePosition, 2, "the knob starts in the middle (off)")
        await connect(id, [("PWR1.VCC", "\(dip).1A"), ("\(dip).1B", "U1.VCC"),
                           ("PWR1.VCC", "\(tactile).A"), ("\(tactile).B", "U1.VCC"),
                           ("\(slide).A", "U1.VCC"), ("\(slide).B", "U1.VCC"), ("PWR1.VCC", "\(slide).C")])
        var result = await call("set_switch", id, ["reference": dip, "position": 1])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("12 V"), result.text)
        result = await call("set_switch", id, ["reference": tactile])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("12 V"), result.text)
        result = await call("set_switch", id, ["reference": slide, "position": 1])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("12 V"), result.text)
        XCTAssertEqual(doc.chips[4].slidePosition, 2, "refused: the knob stays")
    }

    func testReadPinsNamesAMissingPin() async {
        let (id, _, _) = newWindow()
        await place(id, [("GENERIC-LED", 200, 200)])
        _ = await call("simulation_start", id)
        let result = await call("read_pins", id, ["reference": "D1", "pins": ["Q9"]])
        XCTAssertEqual(result.text, "D1 has no pin Q9")
        _ = await call("simulation_stop", id)
    }

    func testReadMemory() async throws {
        let (id, doc, _) = newWindow()
        let memory = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_SRAM }, "no SRAM")
        await place(id, [(memory.name, 300, 300), ("GENERIC-LED", 800, 300)])
        let reference = doc.chips[0].reference
        var result = await call("read_memory", id, ["reference": reference])
        XCTAssertEqual(result.text, "the simulation isn't running; call simulation_start")
        _ = await call("simulation_start", id)
        result = await call("read_memory", id, ["reference": "D1"])
        XCTAssertEqual(result.text, "D1 isn't a memory")
        result = await call("read_memory", id, ["reference": reference, "start": 1, "count": 2])
        XCTAssertFalse(result.isError, result.text)
        let lines = result.text.split(separator: "\n")
        XCTAssertEqual(lines.count, 3, result.text)
        XCTAssertTrue(lines[0].hasPrefix("\(reference) "), result.text)
        XCTAssertTrue(lines[0].contains(" words x "), result.text)
        XCTAssertTrue(lines[1].hasPrefix("1: "), result.text)
        XCTAssertTrue(lines[1].hasSuffix("?"), "an unpowered SRAM is unknown: \(result.text)")
        XCTAssertTrue(lines[2].hasPrefix("2: "), result.text)
        result = await call("read_memory", id, ["reference": reference, "start": 1 << 30])
        XCTAssertEqual(result.text, "address \(1 << 30) is past the end of the memory")
        _ = await call("simulation_stop", id)
    }
}
