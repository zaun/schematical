import AppKit
import Foundation
import Network
import XCTest
@testable import schematical

/// plan-68-X: the agent-control server (AutomationServer.swift) answering
/// MCP requests, its listener on a loopback port of the test's own, and the
/// shared folder (AutomationFolder.swift): choosing, keeping and forgetting it.
/// A test's server binds port 0 (a free port), never the app's own.
@MainActor
final class AutomationServerTests: XCTestCase {
    private static var keptServers: [AutomationServer] = []
    private static var keptFolders: [AutomationFolder] = []
    private var servers: [AutomationServer] = []
    private var savedDefaults: [String: Any?] = [:]
    private var made: [URL] = []

    override func setUp() async throws {
        for key in [PreferenceKey.automationFolder, PreferenceKey.automationEnabled, PreferenceKey.automationToken] {
            savedDefaults[key] = UserDefaults.standard.object(forKey: key)
        }
    }

    override func tearDown() async throws {
        servers.forEach { $0.setEnabled(false) }
        servers = []
        for (key, value) in savedDefaults {
            if let value { UserDefaults.standard.set(value, forKey: key) }
            else { UserDefaults.standard.removeObject(forKey: key) }
        }
        made.forEach { try? FileManager.default.removeItem(at: $0) }
        made = []
    }

    private func server(port: UInt16 = 0) -> AutomationServer {
        let server = AutomationServer(port: port)
        Self.keptServers.append(server)
        servers.append(server)
        return server
    }

    private func wait(_ seconds: TimeInterval = 15, until condition: () -> Bool) -> Bool {
        let end = Date().addingTimeInterval(seconds)
        while Date() < end {
            if condition() { return true }
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
        }
        return condition()
    }

    // MARK: Answering

    private var token: String { AutomationServer.token }

    private func raw(_ method: String = "POST", path: String = "/mcp", body: Data,
                     authorization: String? = nil) -> Data {
        var head = "\(method) \(path) HTTP/1.1\r\nHost: 127.0.0.1\r\nNoColonLine\r\n"
        head += "Authorization: \(authorization ?? "Bearer \(token)")\r\nContent-Length: \(body.count)\r\n\r\n"
        return Data(head.utf8) + body
    }

    private func request(_ json: [String: Any]) throws -> AutomationServer.HTTPRequest {
        let body = try JSONSerialization.data(withJSONObject: json)
        return try XCTUnwrap(AutomationServer.HTTPRequest(parsing: raw(body: body)))
    }

    /// A response's status line and JSON body.
    private func split(_ response: Data) throws -> (status: String, headers: String, json: [String: Any]?) {
        let separator = try XCTUnwrap(response.range(of: Data("\r\n\r\n".utf8)), "no head end")
        let head = String(decoding: response[..<separator.lowerBound], as: UTF8.self)
        let body = response[separator.upperBound...]
        let json = body.isEmpty ? nil : try JSONSerialization.jsonObject(with: Data(body)) as? [String: Any]
        return (head.components(separatedBy: "\r\n")[0], head, json)
    }

    private func call(_ server: AutomationServer, _ method: String, id: Any? = 1,
                      params: [String: Any] = [:]) async throws -> (status: String, json: [String: Any]?) {
        var message: [String: Any] = ["jsonrpc": "2.0", "method": method, "params": params]
        if let id { message["id"] = id }
        let response = await server.respond(to: try request(message))
        let reply = try split(response)
        return (reply.status, reply.json)
    }

    func testRefusalsAndBadBodies() async throws {
        let server = server()
        let wrong = try XCTUnwrap(AutomationServer.HTTPRequest(parsing: raw(body: Data("{}".utf8),
                                                                            authorization: "Bearer nope")))
        let wrongReply = await server.respond(to: wrong)
        let refused = try split(wrongReply)
        XCTAssertEqual(refused.status, "HTTP/1.1 401 Unauthorized")
        XCTAssertTrue(refused.headers.contains("WWW-Authenticate: Bearer"), refused.headers)

        let huge = try XCTUnwrap(AutomationServer.HTTPRequest(
            parsing: raw(body: Data(count: AutomationServer.maxRequestBytes + 1))))
        let hugeReply = await server.respond(to: huge)
        XCTAssertEqual(try split(hugeReply).status, "HTTP/1.1 413 Payload Too Large")

        let garbage = try XCTUnwrap(AutomationServer.HTTPRequest(parsing: raw(body: Data("not json".utf8))))
        let garbageReply = await server.respond(to: garbage)
        XCTAssertEqual(try split(garbageReply).status, "HTTP/1.1 400 Bad Request")
        XCTAssertNil(server.lastRequest, "a refused request is not an agent's call")

        let note = try await call(server, "notifications/initialized", id: nil)
        XCTAssertEqual(note.status, "HTTP/1.1 202 Accepted")
        XCTAssertNil(note.json)
        XCTAssertNotNil(server.lastRequest)
    }

    func testRequestLineWithoutAPathIsNotARequest() {
        XCTAssertNil(AutomationServer.HTTPRequest(parsing: Data("GARBAGE\r\n\r\n".utf8)))
    }

    func testMcpMethods() async throws {
        let server = server()
        let hello = try await call(server, "initialize", params: ["protocolVersion": "2025-03-26",
                                                                  "clientInfo": ["name": "test-agent"]])
        XCTAssertEqual(hello.status, "HTTP/1.1 200 OK")
        let result = try XCTUnwrap(hello.json?["result"] as? [String: Any])
        XCTAssertEqual(result["protocolVersion"] as? String, "2025-03-26")
        XCTAssertEqual(result["instructions"] as? String, AutomationGuide.text)
        XCTAssertEqual(server.clientName, "test-agent")

        let plain = try await call(server, "initialize", id: "two")
        XCTAssertEqual((plain.json?["result"] as? [String: Any])?["protocolVersion"] as? String, "2025-06-18")
        XCTAssertEqual(plain.json?["id"] as? String, "two")

        let ping = try await call(server, "ping")
        XCTAssertNotNil(ping.json?["result"])
        let tools = try await call(server, "tools/list").json?["result"] as? [String: Any]
        XCTAssertEqual((tools?["tools"] as? [Any])?.count, AutomationTools.all.count)
        let resources = try await call(server, "resources/list").json?["result"] as? [String: Any]
        XCTAssertEqual((resources?["resources"] as? [Any])?.count, 0)
        let prompts = try await call(server, "prompts/list").json?["result"] as? [String: Any]
        XCTAssertEqual((prompts?["prompts"] as? [Any])?.count, 0)

        XCTAssertEqual(server.callCount, 0)
        let unknown = try await call(server, "tools/call", params: ["name": "no_such_tool"])
        let content = (unknown.json?["result"] as? [String: Any])?["isError"] as? Bool
        XCTAssertEqual(content, true)
        XCTAssertEqual(server.callCount, 1)

        let missing = try await call(server, "nothing/here")
        let error = try XCTUnwrap(missing.json?["error"] as? [String: Any])
        XCTAssertEqual(error["code"] as? Int, -32601)
        XCTAssertEqual(error["message"] as? String, "unknown method nothing/here")
    }

    /// With no token saved (or one too short), reading it makes and saves one.
    func testATokenIsMadeOnFirstUse() {
        UserDefaults.standard.removeObject(forKey: PreferenceKey.automationToken)
        let made = AutomationServer.token
        XCTAssertEqual(made.count, 48)
        XCTAssertEqual(UserDefaults.standard.string(forKey: PreferenceKey.automationToken), made)
        XCTAssertEqual(AutomationServer.token, made, "kept")
        UserDefaults.standard.set("short", forKey: PreferenceKey.automationToken)
        XCTAssertNotEqual(AutomationServer.token, "short")
    }

    func testAddCommandNamesTheUrlAndToken() {
        XCTAssertTrue(AutomationServer.addCommand.contains(AutomationServer.url))
        XCTAssertTrue(AutomationServer.addCommand.hasSuffix("Bearer \(AutomationServer.token)\""))
    }

    // MARK: Listening

    /// POSTs `body` to 127.0.0.1:`port`/mcp; the status code, or nil when
    /// the connection failed. URLSession sends its own Host header (with the
    /// port), which the server refuses as not loopback's: 403 is an answer.
    private nonisolated func post(_ port: UInt16, _ body: Data) async -> Int? {
        var request = URLRequest(url: URL(string: "http://127.0.0.1:\(port)/mcp")!, timeoutInterval: 20)
        request.httpMethod = "POST"
        request.httpBody = body
        let session = URLSession(configuration: .ephemeral)
        defer { session.invalidateAndCancel() }
        guard let (_, response) = try? await session.data(for: request) else { return nil }
        return (response as? HTTPURLResponse)?.statusCode
    }

    private func listening(_ server: AutomationServer) throws -> UInt16 {
        XCTAssertTrue(wait { server.isListening && server.listeningPort != nil }, "the listener never got ready")
        return try XCTUnwrap(server.listeningPort)
    }

    func testServesRequestsOverTheSocket() async throws {
        let server = server()
        server.setEnabled(true)
        server.setEnabled(true)  // already on: nothing changes
        // An async test can't spin the run loop for the main actor's jobs: sleep instead.
        for _ in 0..<300 where server.listeningPort == nil {
            try await Task.sleep(nanoseconds: 50_000_000)
        }
        let port = try XCTUnwrap(server.listeningPort, "the listener never got ready")
        XCTAssertTrue(server.isListening)

        let ping = try JSONSerialization.data(withJSONObject: ["jsonrpc": "2.0", "id": 7, "method": "ping"])
        let answered = await post(port, ping)
        XCTAssertEqual(answered, 403, "read whole and answered (refused: URLSession's Host has the port)")

        // A body bigger than one read comes in pieces; past the cap it is refused before it is all read.
        let tooBig = await post(port, Data(count: AutomationServer.maxRequestBytes + 20_000))
        XCTAssertTrue(tooBig == 413 || tooBig == nil, "\(String(describing: tooBig))")

        // The same server answers MCP directly (the socket only carries it).
        _ = try await call(server, "ping")
        XCTAssertNotNil(server.lastRequest)
        server.setEnabled(false)
        XCTAssertFalse(server.isListening)
        XCTAssertNil(server.listeningPort)
        XCTAssertNil(server.lastRequest, "stopping forgets the session")
    }

    /// A server or folder let go is freed (their deinits are nonisolated).
    func testServersAndFoldersAreFreed() {
        weak var server: AutomationServer?
        weak var folder: AutomationFolder?
        do {
            let made = AutomationServer(port: 0)
            let chosen = AutomationFolder()
            server = made
            folder = chosen
            XCTAssertEqual(made.bindPort, 0)
            XCTAssertNil(chosen.url)
        }
        XCTAssertNil(server)
        XCTAssertNil(folder)
    }

    func testListenerStates() throws {
        let server = server()
        server.listenerChanged(.ready, port: 4242)
        XCTAssertTrue(server.isListening)
        XCTAssertEqual(server.listeningPort, 4242)
        server.listenerChanged(.other, port: nil)
        XCTAssertTrue(server.isListening, "other states change nothing")
        server.listenerChanged(.failed, port: nil)
        XCTAssertFalse(server.isListening)
        XCTAssertNil(server.listeningPort)
    }

    /// A port already taken: the second server's listener fails.
    func testATakenPortFails() throws {
        let first = server()
        first.setEnabled(true)
        let port = try listening(first)
        let second = server(port: port)
        second.setEnabled(true)
        XCTAssertFalse(wait(1) { second.isListening }, "the port is taken")
        XCTAssertNil(second.listeningPort)
    }

    func testAListenerThatCannotBeMadeLeavesTheServerOff() throws {
        struct Refused: Error {}
        let server = server()
        let make = server.makeListener
        server.makeListener = { _ in throw Refused() }
        server.setEnabled(true)
        XCTAssertFalse(wait(0.5) { server.isListening })
        server.makeListener = make
        server.setEnabled(true)
        _ = try listening(server)
    }

    func testStartsAtLaunchOnlyWhenSettingsSaySo() throws {
        UserDefaults.standard.set(false, forKey: PreferenceKey.automationEnabled)
        XCTAssertFalse(AutomationServer.isEnabledInSettings)
        let off = server()
        off.startIfEnabled()
        XCTAssertFalse(wait(0.5) { off.isListening })
        UserDefaults.standard.set(true, forKey: PreferenceKey.automationEnabled)
        let on = server()
        on.startIfEnabled()
        _ = try listening(on)
    }

    // MARK: The shared folder

    private func folder(_ name: String = UUID().uuidString) throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("AutomationServerTests-\(name)")
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        made.append(url)
        return url.resolvingSymlinksInPath()
    }

    private func newFolder() -> AutomationFolder {
        let folder = AutomationFolder()
        Self.keptFolders.append(folder)
        return folder
    }

    func testChoosingKeepingAndForgettingTheFolder() throws {
        let chosen = try folder()
        let folder = newFolder()
        var asked: NSOpenPanel?
        folder.present = { panel in
            asked = panel
            return nil
        }
        folder.choose()
        XCTAssertNil(folder.url, "cancelled")
        XCTAssertEqual(asked?.prompt, "Allow")
        XCTAssertEqual(asked?.canChooseDirectories, true)
        XCTAssertEqual(asked?.canChooseFiles, false)

        folder.present = { _ in chosen }
        folder.choose()
        XCTAssertEqual(folder.url, chosen)
        XCTAssertNotNil(UserDefaults.standard.data(forKey: PreferenceKey.automationFolder), "kept as a bookmark")

        // Another session opens it from the bookmark, once.
        let later = newFolder()
        later.load()
        XCTAssertEqual(later.url?.standardizedFileURL.path, chosen.standardizedFileURL.path)
        later.load()
        XCTAssertEqual(later.url?.standardizedFileURL.path, chosen.standardizedFileURL.path)
        later.clear()
        XCTAssertNil(later.url)
        XCTAssertNil(UserDefaults.standard.data(forKey: PreferenceKey.automationFolder))
        XCTAssertThrowsError(try later.schematics())
    }

    func testAMovedFolderIsFoundAndRememberedAgain() throws {
        let original = try folder()
        let folder = newFolder()
        folder.present = { _ in original }
        folder.choose()
        let before = try XCTUnwrap(UserDefaults.standard.data(forKey: PreferenceKey.automationFolder))
        let moved = original.deletingLastPathComponent().appendingPathComponent("AutomationServerTests-\(UUID().uuidString)")
        try FileManager.default.moveItem(at: original, to: moved)
        made.append(moved)
        let later = newFolder()
        later.load()
        XCTAssertEqual(later.url?.standardizedFileURL.path, moved.standardizedFileURL.path)
        XCTAssertNotEqual(UserDefaults.standard.data(forKey: PreferenceKey.automationFolder), before,
                          "the stale bookmark is saved again")
    }

    func testAnUnreadableBookmarkOpensNothing() {
        UserDefaults.standard.set(Data("not a bookmark".utf8), forKey: PreferenceKey.automationFolder)
        let folder = newFolder()
        folder.load()
        XCTAssertNil(folder.url)
    }

    /// The real panel, put up and taken down by the test at once.
    func testTheRealPanelCanBeCancelled() {
        // The panel is the app's only window here: closing it must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
        let timer = Timer(timeInterval: 0.5, repeats: false) { _ in
            MainActor.assumeIsolated { NSApp.abortModal() }
        }
        RunLoop.main.add(timer, forMode: .modalPanel)
        let panel = AutomationFolder.panel()
        XCTAssertNil(AutomationFolder.runModal(panel))
        XCTAssertEqual(panel.message, "Choose the folder AI agents may open and save schematics in.")
    }

    // MARK: Windows

    func testANewWindowThatNeverComesIsReported() async {
        do {
            _ = try await AutomationTools.newSession(besides: Set(AutomationTools.sessions().map(\.id)), tries: 2)
            XCTFail("no window was opened")
        } catch let failure as AutomationFailure {
            XCTAssertEqual(failure.message, "the new window didn't appear")
        } catch {
            XCTFail("\(error)")
        }
    }
}
