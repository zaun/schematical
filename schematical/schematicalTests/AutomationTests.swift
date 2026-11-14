import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The agent-control server's request checks and its tools (docs/agent-control.md).
@MainActor
final class AutomationTests: XCTestCase {
    /// Simulations made by tests, kept like `TestSupport`'s documents.
    private static var kept: [SimulationController] = []

    private struct Result {
        let text: String
        let isError: Bool
        let images: Int
    }

    private func newWindow() -> (id: String, doc: SchematicDocument) {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.kept.append(simulation)
        let id = UUID()
        SchematicSessions.shared.register(id, document: doc, simulation: simulation)
        return (id.uuidString, doc)
    }

    private func call(_ tool: String, _ window: String, _ args: [String: Any] = [:]) async -> Result {
        var args = args
        args["window"] = window
        let result = await AutomationTools.call(tool, args)
        let content = result["content"] as? [[String: Any]] ?? []
        return Result(text: content.compactMap { $0["text"] as? String }.joined(separator: "\n"),
                      isError: result["isError"] as? Bool ?? false,
                      images: content.filter { $0["type"] as? String == "image" }.count)
    }

    // MARK: Request checks

    private let token = "t0ken-t0ken-t0ken-t0ken-t0ken-t0ken-xx"

    private func headers(_ extra: [String: String] = [:]) -> [String: String] {
        var base = ["host": "127.0.0.1:\(AutomationServer.port)", "authorization": "Bearer \(token)"]
        base.merge(extra) { _, new in new }
        return base
    }

    func testAGoodRequestIsAccepted() {
        XCTAssertNil(AutomationServer.refusal(method: "POST", path: "/mcp", headers: headers(), token: token))
        XCTAssertNil(AutomationServer.refusal(method: "POST", path: "/mcp",
                                              headers: headers(["host": "localhost:\(AutomationServer.port)"]), token: token))
    }

    func testARequestWithoutTheTokenIsRefused() {
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp", headers: ["host": "127.0.0.1"], token: token),
                       "401 Unauthorized")
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp",
                                                headers: headers(["authorization": "Bearer nope"]), token: token),
                       "401 Unauthorized")
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp",
                                                headers: headers(["authorization": token]), token: token),
                       "401 Unauthorized")
    }

    func testABrowserRequestIsRefusedEvenWithTheToken() {
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp",
                                                headers: headers(["origin": "http://evil.example"]), token: token),
                       "403 Forbidden")
    }

    func testANonLoopbackHostIsRefused() {
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp",
                                                headers: headers(["host": "evil.example:\(AutomationServer.port)"]), token: token),
                       "403 Forbidden")
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/mcp", headers: ["authorization": "Bearer \(token)"],
                                                token: token), "403 Forbidden")
    }

    func testOnlyPostToMcpIsServed() {
        XCTAssertEqual(AutomationServer.refusal(method: "GET", path: "/mcp", headers: headers(), token: token),
                       "405 Method Not Allowed")
        XCTAssertEqual(AutomationServer.refusal(method: "POST", path: "/other", headers: headers(), token: token),
                       "404 Not Found")
    }

    func testTokenComparisonAndRegeneration() {
        XCTAssertTrue(AutomationServer.constantTimeEqual("abc", "abc"))
        XCTAssertFalse(AutomationServer.constantTimeEqual("abc", "abd"))
        XCTAssertFalse(AutomationServer.constantTimeEqual("abc", "abcd"))
        let saved = UserDefaults.standard.string(forKey: PreferenceKey.automationToken)
        defer {
            if let saved { UserDefaults.standard.set(saved, forKey: PreferenceKey.automationToken) }
            else { UserDefaults.standard.removeObject(forKey: PreferenceKey.automationToken) }
        }
        let first = AutomationServer.regenerateToken()
        let second = AutomationServer.regenerateToken()
        XCTAssertEqual(first.count, 48)
        XCTAssertNotEqual(first, second)
        XCTAssertEqual(AutomationServer.token, second)
    }

    func testAnHTTPRequestParsesOnlyWhenWhole() throws {
        let body = #"{"a":1}"#
        let head = "POST /mcp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: \(body.utf8.count)\r\n\r\n"
        XCTAssertNil(AutomationServer.HTTPRequest(parsing: Data(head.utf8)))
        XCTAssertNil(AutomationServer.HTTPRequest(parsing: Data((head + "{\"a\"").utf8)))
        let request = try XCTUnwrap(AutomationServer.HTTPRequest(parsing: Data((head + body).utf8)))
        XCTAssertEqual(request.method, "POST")
        XCTAssertEqual(request.path, "/mcp")
        XCTAssertEqual(request.headers["host"], "127.0.0.1")
        XCTAssertEqual(String(data: request.body, encoding: .utf8), body)
    }

    // MARK: The shared folder

    func testAPathMustStayInsideTheSharedFolder() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("automation-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root.appendingPathComponent("sub"), withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let real = root.resolvingSymlinksInPath()
        XCTAssertEqual(try AutomationFolder.contain("a.schematical", in: root).path, real.appendingPathComponent("a.schematical").path)
        XCTAssertEqual(try AutomationFolder.contain("sub/b.schematical", in: root).path,
                       real.appendingPathComponent("sub/b.schematical").path)
        XCTAssertEqual(try AutomationFolder.contain(real.appendingPathComponent("c.schematical").path, in: root).lastPathComponent,
                       "c.schematical")
        XCTAssertThrowsError(try AutomationFolder.contain("../escape.schematical", in: root))
        XCTAssertThrowsError(try AutomationFolder.contain("/etc/passwd", in: root))
        XCTAssertThrowsError(try AutomationFolder.contain("sub/../../escape", in: root))
        XCTAssertThrowsError(try AutomationFolder.contain("..", in: root))
        // A symlink inside the folder that points out doesn't lead out.
        try FileManager.default.createSymbolicLink(at: root.appendingPathComponent("link"),
                                                   withDestinationURL: URL(fileURLWithPath: "/tmp"))
        XCTAssertThrowsError(try AutomationFolder.contain("link/x.schematical", in: root))
    }

    // MARK: Tool list

    func testEveryToolHasAUniqueNameAndASchema() {
        let names = AutomationTools.all.map(\.name)
        XCTAssertEqual(Set(names).count, names.count, "duplicate tool names")
        for definition in AutomationTools.definitions {
            XCTAssertNotNil(definition["name"] as? String)
            XCTAssertFalse((definition["description"] as? String ?? "").isEmpty)
            let schema = definition["inputSchema"] as? [String: Any]
            XCTAssertEqual(schema?["type"] as? String, "object")
            XCTAssertTrue(JSONSerialization.isValidJSONObject(definition), "\(definition["name"] ?? "?") isn't JSON")
        }
        for expected in ["place_part", "connect", "route", "render", "run_drc", "batch", "undo", "simulation_start",
                         "set_switch", "get_nets", "move_footprint", "set_layer", "open", "save"] {
            XCTAssertTrue(names.contains(expected), "missing \(expected)")
        }
    }

    // MARK: Schematic tools

    func testPlaceConnectAndReadBack() async throws {
        let (id, doc) = newWindow()
        var result = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.contains("placed D1"), result.text)
        result = await call("place_part", id, ["part": "GENERIC-LED", "x": 300, "y": 100])
        XCTAssertTrue(result.text.contains("placed D2"), result.text)
        result = await call("connect", id, ["from": "D1.K", "to": "D2.A"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.wires.count, 1)
        let nets = await call("get_nets", id)
        XCTAssertTrue(nets.text.contains("D1.1(K)") && nets.text.contains("D2.2(A)"), nets.text)
        let design = await call("get_design", id)
        XCTAssertTrue(design.text.contains("D1 GENERIC-LED"), design.text)
        XCTAssertTrue(design.text.contains("D1.1 -> D2.2"), design.text)
    }

    func testAMistakeNamesTheValidChoices() async {
        let (id, _) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        var result = await call("connect", id, ["from": "D1.Q", "to": "D1.A"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("pins: 1=K 2=A"), result.text)
        result = await call("connect", id, ["from": "Z9.1", "to": "D1.A"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("no part Z9"), result.text)
        result = await call("place_part", id, ["part": "NOT-A-PART", "x": 0, "y": 0])
        XCTAssertTrue(result.isError)
        result = await call("place_part", id, ["part": "GENERIC-LED", "x": 0, "y": 0, "color": "Plaid"])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("Red"), result.text)
    }

    func testABatchIsOneUndoStepAndStopsAtTheFirstError() async {
        let (id, doc) = newWindow()
        var result = await call("batch", id, ["calls": [
            ["tool": "place_part", "arguments": ["part": "GENERIC-LED", "x": 100, "y": 100]],
            ["tool": "place_part", "arguments": ["part": "GENERIC-LED", "x": 300, "y": 100]],
            ["tool": "connect", "arguments": ["from": "D1.K", "to": "D2.A"]],
        ]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.chips.count, 2)
        XCTAssertEqual(doc.wires.count, 1)
        result = await call("undo", id)
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.chips.count, 0, "one undo reverts the whole batch")
        XCTAssertEqual(doc.wires.count, 0)
        result = await call("redo", id)
        XCTAssertEqual(doc.chips.count, 2)

        result = await call("batch", id, ["calls": [
            ["tool": "place_part", "arguments": ["part": "GENERIC-LED", "x": 500, "y": 100]],
            ["tool": "connect", "arguments": ["from": "D3.K", "to": "D9.A"]],
            ["tool": "place_part", "arguments": ["part": "GENERIC-LED", "x": 700, "y": 100]],
        ]])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("call 2 (connect) FAILED"), result.text)
        XCTAssertEqual(doc.chips.count, 3, "call 1 stays, call 3 never ran")
    }

    func testABatchRefusesAnUnknownToolBeforeRunningAnything() async {
        let (id, doc) = newWindow()
        let result = await call("batch", id, ["calls": [
            ["tool": "place_part", "arguments": ["part": "GENERIC-LED", "x": 100, "y": 100]],
            ["tool": "make_coffee", "arguments": [:]],
        ]])
        XCTAssertTrue(result.isError)
        XCTAssertEqual(doc.chips.count, 0)
        let nested = await call("batch", id, ["calls": [["tool": "batch", "arguments": ["calls": []]]]])
        XCTAssertTrue(nested.isError)
    }

    func testEditsAreRefusedWhileTheSimulationRuns() async {
        let (id, doc) = newWindow()
        doc.isPlaying = true
        defer { doc.isPlaying = false }
        let result = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("simulation_stop"), result.text)
        XCTAssertEqual(doc.chips.count, 0)
        let read = await call("get_design", id)
        XCTAssertFalse(read.isError, "reading is allowed")
    }

    func testMoveFlipSetDeleteAndSelect() async throws {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 300, "y": 100])
        _ = await call("connect", id, ["from": "D1.K", "to": "D2.A"])
        var result = await call("move_part", id, ["reference": "D2", "x": 300, "y": 220])
        XCTAssertFalse(result.isError, result.text)
        let moved = try XCTUnwrap(doc.chips.first { $0.reference == "D2" })
        XCTAssertEqual(moved.origin.y + moved.layout.size.height / 2, 220, accuracy: 10)
        result = await call("set_part", id, ["reference": "D2", "color": "Blue", "new_reference": "D7"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(doc.chips.contains { $0.reference == "D7" && $0.color == "Blue" })
        result = await call("select", id, ["items": ["D7"]])
        XCTAssertEqual(doc.selectedChipID, doc.chips.first { $0.reference == "D7" }?.id)
        result = await call("delete", id, ["items": ["D7"]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.chips.map(\.reference), ["D1"])
        result = await call("delete", id, ["items": ["nonsense:1"]])
        XCTAssertTrue(result.isError)
    }

    func testDisconnectRemovesTheWire() async {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 300, "y": 100])
        _ = await call("connect", id, ["from": "D1.K", "to": "D2.A"])
        var result = await call("disconnect", id, ["from": "D1.K", "to": "D2.A"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.wires.count, 0)
        result = await call("disconnect", id, ["from": "D1.K", "to": "D2.A"])
        XCTAssertTrue(result.isError)
    }

    func testNetLabelsNotesAndBuses() async {
        let (id, doc) = newWindow()
        var result = await call("add_net_label", id, ["name": "clk", "x": 100, "y": 100])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.netLabels.count, 1)
        result = await call("add_note", id, ["text": "hello\nworld", "x": 40, "y": 300, "size": 14])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.notes.first?.text, "hello\nworld")
        XCTAssertNil(doc.editingNoteID, "no text editor is left open")
        result = await call("add_bus", id, ["x1": 400, "y1": 100, "x2": 400, "y2": 300, "name": "DB", "wires": 4])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses.first?.wires.count, 4)
        result = await call("add_bus_port", id, ["bus": "DB", "x": 400, "y": 200, "wires": ["B0", "B1"]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses.first?.ports.first?.wireIDs.count, 2)
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 200, "y": 200])
        result = await call("connect", id, ["from": "D1.A", "to": "bus:DB:B0", "via": [[300, 300]]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(doc.wirePaths.allSatisfy(OrthogonalRoute.isOrthogonal), "via legs are right angles")
        result = await call("connect", id, ["from": "D1.K", "to": "bus:DB:B3"])
        XCTAssertTrue(result.isError, "B3 has no port pin")
    }

    func testEditingAnExistingBus() async {
        let (id, doc) = newWindow()
        _ = await call("add_bus", id, ["x1": 400, "y1": 100, "x2": 400, "y2": 300, "name": "DB", "wires": 3])
        _ = await call("add_bus_port", id, ["bus": "DB", "x": 400, "y": 200, "wires": ["B0", "B1"]])
        var result = await call("set_bus", id, ["bus": "DB", "wire_names": ["D0", "D1", "D2", "WR"]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses.first?.wires.map(\.name), ["D0", "D1", "D2", "WR"])
        result = await call("set_bus", id, ["bus": "DB", "rename": ["D2": "RD"], "move_wire": "WR", "move_to": 0])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses.first?.wires.map(\.name), ["WR", "D0", "D1", "RD"])
        result = await call("set_bus_port", id, ["bus": "DB", "port": 1, "add_wires": ["WR"]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses.first?.ports.first?.wireIDs.count, 3)
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 200, "y": 200])
        result = await call("connect", id, ["from": "D1.A", "to": "bus:DB:WR"])
        XCTAssertFalse(result.isError, result.text)
        result = await call("set_bus", id, ["bus": "DB", "rename": ["D0": "WR"]])
        XCTAssertFalse(result.isError, "two wires may share a name: " + result.text)
        XCTAssertEqual(doc.buses.first?.wires.map(\.name), ["WR", "WR", "D1", "RD"])
        result = await call("set_bus", id, ["bus": "DB"])
        XCTAssertTrue(result.isError, "needs something to change")
        result = await call("set_bus_port", id, ["bus": "DB", "port": 2])
        XCTAssertTrue(result.isError, "only one port")
        let design = await call("get_design", id, [:])
        XCTAssertTrue(design.text.contains("port 1: pins"), design.text)
        // Move a port along its line and to the other side.
        let before = doc.buses[0].ports[0]
        result = await call("set_bus_port", id, ["bus": "DB", "port": 1, "at": before.start + 20])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.buses[0].ports[0].start, before.start + 20)
        result = await call("set_bus_port", id, ["bus": "DB", "port": 1, "side": before.side == .left ? "right" : "left"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertNotEqual(doc.buses[0].ports[0].side, before.side)
        result = await call("set_bus_port", id, ["bus": "DB", "port": 1, "side": "top"])
        XCTAssertTrue(result.isError, "top isn't a side of a vertical line")
        // Delete it, with a wire on its pin left loose.
        result = await call("delete", id, ["items": ["port:DB:1"]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.contains("loose ends"), result.text)
        XCTAssertTrue(doc.buses[0].ports.isEmpty)
        XCTAssertNil(doc.pendingRemoval, "no dialog is left open")
        result = await call("delete", id, ["items": ["port:DB:1"]])
        XCTAssertTrue(result.isError, "no port left")
    }

    // MARK: Board tools

    func testPlaceAndRouteFootprints() async throws {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 300, "y": 100])
        var result = await call("set_board", id, ["width": 60, "height": 40])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.board.width, 60)
        result = await call("move_footprint", id, ["reference": "D1", "x": 15, "y": 20])
        XCTAssertTrue(result.text.contains("inside the board"), result.text)
        result = await call("move_footprint", id, ["reference": "D2", "x": 45, "y": 20, "rotation": 90])
        XCTAssertEqual(doc.chips.first { $0.reference == "D2" }?.board?.rotation, 90)
        result = await call("move_footprint", id, ["reference": "D2", "x": 500, "y": 500])
        XCTAssertTrue(result.text.contains("OUTSIDE"), result.text)
        _ = await call("move_footprint", id, ["reference": "D2", "x": 45, "y": 20])

        _ = await call("connect", id, ["from": "D1.A", "to": "D2.A"])
        let before = await call("run_drc", id)
        XCTAssertTrue(before.text.contains("unrouted"), before.text)
        result = await call("route", id, ["from": "D1.A", "to": "D2.A", "width": 12])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.traces.count, 1)
        XCTAssertEqual(doc.traces.first?.width, 12)
        let a = try XCTUnwrap(doc.chips.first { $0.reference == "D1" }?.boardPads.first { $0.designator == "2" })
        let first = try XCTUnwrap(doc.traces.first?.points.first)
        XCTAssertEqual(first.x, a.rect.midX, accuracy: 0.001, "the trace starts on the pad")
        XCTAssertEqual(first.y, a.rect.midY, accuracy: 0.001)

        let drc = await call("run_drc", id)
        XCTAssertFalse(drc.text.contains("unrouted"), "the only net is routed: \(drc.text)")
        let board = await call("get_board", id)
        XCTAssertTrue(board.text.contains("TRACES") && board.text.contains("D1 ("), board.text)

        result = await call("route", id, ["from": "D1.A", "to": "D2.A", "layer": "inner2"])
        XCTAssertTrue(result.isError, "a 2-layer board has no inner layer")
    }

    func testARouteOverAnotherPadWarnsOfTheShort() async {
        let (id, doc) = newWindow()
        for (x, y) in [(100, 100), (200, 100), (300, 100)] {
            _ = await call("place_part", id, ["part": "GENERIC-LED", "x": x, "y": y])
        }
        _ = await call("move_footprint", id, ["reference": "D1", "x": 10, "y": 20])
        _ = await call("move_footprint", id, ["reference": "D2", "x": 25, "y": 20])
        _ = await call("move_footprint", id, ["reference": "D3", "x": 40, "y": 20])
        // D1.A to D3.A runs straight along y = 20, through D2's pad.
        let result = await call("route", id, ["from": "D1.A", "to": "D3.A"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.contains("WARNING"), result.text)
        XCTAssertEqual(doc.traces.count, 1)
    }

    func testPlanesAndLayerCounts() async {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GND", "x": 100, "y": 100])
        var result = await call("set_board", id, ["layers": 4])
        XCTAssertFalse(result.isError, result.text)
        result = await call("set_layer", id, ["layer": "inner2", "type": "plane", "net": "ground"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.board.style(of: .inner(2)).net, .ground)
        result = await call("set_layer", id, ["layer": "inner2", "net": "9V"])
        XCTAssertTrue(result.isError)
        result = await call("set_layer", id, ["layer": "inner9"])
        XCTAssertTrue(result.isError)
    }

    func testViasHolesTextsAndLocks() async {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        var result = await call("add_via", id, ["x": 30, "y": 30])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.vias.count, 1)
        result = await call("add_hole", id, ["x": 5, "y": 5, "size": "m3"])
        XCTAssertEqual(doc.holes.count, 1)
        result = await call("add_board_text", id, ["text": "HELLO", "x": 10, "y": 10])
        XCTAssertEqual(doc.texts.first?.text, "HELLO")
        result = await call("lock", id, ["items": ["fp:D1"]])
        XCTAssertFalse(result.isError, result.text)
        result = await call("move_footprint", id, ["reference": "D1", "x": 20, "y": 20])
        XCTAssertTrue(result.isError, "a locked footprint stays")
        XCTAssertFalse(doc.isLocked(.via(doc.vias[0].id)))
        result = await call("delete", id, ["items": ["via:\(String(doc.vias[0].id.uuidString.prefix(8)))"]])
        XCTAssertEqual(doc.vias.count, 0)
        result = await call("undo", id, ["page": "pcb"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.vias.count, 1)
    }

    // MARK: Catalog and picture

    func testSearchAndGetPart() async {
        let (id, _) = newWindow()
        var result = await call("search_parts", id, ["query": "nand", "package": "DIP", "limit": 3])
        XCTAssertTrue(result.text.contains("NAND"), result.text)
        result = await call("search_parts", id)
        XCTAssertTrue(result.text.contains("categories:") && result.text.contains("GND"), result.text)
        result = await call("get_part", id, ["part": "GENERIC-LED"])
        XCTAssertTrue(result.text.contains("1=K") && result.text.contains("Red"), result.text)
        result = await call("get_part", id, ["part": "NOPE"])
        XCTAssertTrue(result.isError)
    }

    func testRenderReturnsAPictureOnlyWhenThereIsSomethingToDraw() async {
        let (id, _) = newWindow()
        var result = await call("render", id, ["view": "schematic"])
        XCTAssertTrue(result.isError, "an empty design has nothing to draw")
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        result = await call("render", id, ["view": "schematic"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(result.images, 1)
        result = await call("render", id, ["view": "schematic", "region": [[0, 0], [0.5, 0.5]]])
        XCTAssertEqual(result.images, 1)
        result = await call("render", id, ["view": "schematic", "region": [[0, 0], [2, 2]]])
        XCTAssertTrue(result.isError)
    }

    // MARK: Simulation

    func testSwitchesAndPinsInTheSimulation() async {
        let (id, doc) = newWindow()
        for (part, x, y) in [("CD74HC00E", 400, 200), ("5V", 300, 40), ("GND", 300, 420), ("TE-GDS0204", 160, 265), ("GENERIC-LED", 600, 200)] {
            let result = await call("place_part", id, ["part": part, "x": x, "y": y])
            XCTAssertFalse(result.isError, result.text.replacingOccurrences(of: "\n", with: " | "))
        }
        let wires = [("U1.VCC", "PWR1.VCC"), ("U1.GND", "PWR2.GND"), ("U1.1A", "PWR1.VCC"),
                     ("S1.1A", "PWR1.VCC"), ("S1.1B", "U1.1B"), ("U1.1Y", "D1.A"), ("D1.K", "PWR2.GND")]
        for (from, to) in wires {
            let result = await call("connect", id, ["from": from, "to": to])
            XCTAssertFalse(result.isError, "\(from)→\(to): \(result.text.replacingOccurrences(of: "\n", with: " | "))")
        }
        var result = await call("read_pins", id, ["reference": "U1"])
        XCTAssertTrue(result.isError, "not running yet")
        result = await call("simulation_start", id)
        XCTAssertFalse(result.isError, result.text.replacingOccurrences(of: "\n", with: " | "))
        XCTAssertTrue(doc.isPlaying)
        result = await call("set_switch", id, ["reference": "S1", "position": 1, "on": true])
        XCTAssertFalse(result.isError, result.text.replacingOccurrences(of: "\n", with: " | "))
        _ = await call("wait", id, ["milliseconds": 200])
        result = await call("read_pins", id, ["reference": "U1", "pins": ["1A", "1B", "1Y"]])
        XCTAssertTrue(result.text.contains("1B=1"), "ON: " + result.text.replacingOccurrences(of: "\n", with: " | "))
        XCTAssertTrue(result.text.contains("1Y=0"), result.text.replacingOccurrences(of: "\n", with: " | "))
        _ = await call("set_switch", id, ["reference": "S1", "position": 1, "on": false])
        _ = await call("wait", id, ["milliseconds": 200])
        result = await call("read_pins", id, ["reference": "U1", "pins": ["1B"]])
        XCTAssertTrue(result.text.contains("1B=Z"), result.text.replacingOccurrences(of: "\n", with: " | "))
        result = await call("simulation_stop", id)
        XCTAssertFalse(doc.isPlaying)
        let status = await call("simulation_status", id)
        XCTAssertTrue(status.text.hasPrefix("not running"), status.text)
    }

    // MARK: Windows

    func testWindowAddressing() async {
        let (id, _) = newWindow()
        let list = await AutomationTools.call("list_windows", [:])
        let text = (list["content"] as? [[String: Any]])?.first?["text"] as? String ?? ""
        XCTAssertTrue(text.contains(String(id.prefix(8))), text)
        let bad = await AutomationTools.call("get_design", ["window": "ffffffff-nope"])
        XCTAssertEqual(bad["isError"] as? Bool, true)
        // A short id works.
        let short = await AutomationTools.call("get_design", ["window": String(id.prefix(8))])
        XCTAssertNil(short["isError"])
    }

    func testFileToolsAreOffWithoutASharedFolder() async throws {
        let (id, _) = newWindow()
        AutomationFolder.shared.load()
        try XCTSkipIf(AutomationFolder.shared.url != nil, "a folder is shared on this machine; the test won't clear it")
        for (tool, args) in [("save", ["path": "x.schematical"]), ("open", ["path": "x.schematical"]),
                             ("export_gerber", ["path": "x.zip"])] {
            let result = await call(tool, id, args)
            XCTAssertTrue(result.isError, tool)
            XCTAssertTrue(result.text.contains("Settings"), result.text.replacingOccurrences(of: "\n", with: " | "))
        }
        let files = await call("list_files", id)
        XCTAssertTrue(files.isError)
    }
}
