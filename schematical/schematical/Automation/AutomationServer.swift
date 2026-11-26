//
//  AutomationServer.swift
//  schematical
//
//  A loopback HTTP server that speaks MCP (Streamable HTTP, JSON replies
//  only) so an AI agent can drive the running app. Off until Settings ▸ AI
//  Agents turns it on (`PreferenceKey.automationEnabled`). See
//  docs/agent-control.md.
//
//  Safety: binds 127.0.0.1 only; every request needs the bearer token shown
//  in Settings; the Host header must be loopback; a request carrying an
//  Origin header (a web page) is refused; the body is size-capped. Every
//  tool call runs on the main actor, against the same SchematicDocument the
//  window draws.
//

import Foundation
import Network
import Observation
import Security

@MainActor
@Observable
final class AutomationServer {
    static let shared = AutomationServer()

    /// The port the server listens on.
    nonisolated static let port: UInt16 = 47800

    /// Where an agent connects.
    static let url = "http://127.0.0.1:\(port)/mcp"

    /// The largest request body taken.
    nonisolated static let maxRequestBytes = 8 << 20

    // MARK: Status (shown in Settings)

    /// Whether the listener is up.
    private(set) var isListening = false
    /// The agent that last called, from its `initialize`.
    private(set) var clientName: String?
    /// When any authorised request last came in.
    private(set) var lastRequest: Date?
    /// Tool calls since the server started.
    private(set) var callCount = 0

    @ObservationIgnored
    private var listener: NWListener?

    /// The port this server binds: `AutomationServer.port`; 0 (any free
    /// port) for a test's own server.
    let bindPort: UInt16
    /// The port it is listening on once ready (`bindPort`, or the one the
    /// system picked for 0).
    private(set) var listeningPort: UInt16?
    /// Makes the listener (tests make it fail).
    @ObservationIgnored var makeListener: (NWParameters) throws -> NWListener = { try NWListener(using: $0) }

    init(port: UInt16 = AutomationServer.port) {
        bindPort = port
    }

    nonisolated deinit {}

    // MARK: Switching

    /// Whether Settings has the server on.
    static var isEnabledInSettings: Bool {
        UserDefaults.standard.bool(forKey: PreferenceKey.automationEnabled)
    }

    /// Starts the server at launch when Settings has it on.
    func startIfEnabled() {
        if Self.isEnabledInSettings { start() }
    }

    /// Starts or stops the server to match the Settings toggle.
    func setEnabled(_ enabled: Bool) {
        if enabled { start() } else { stop() }
    }

    private func stop() {
        listener?.cancel()
        listener = nil
        isListening = false
        listeningPort = nil
        clientName = nil
        lastRequest = nil
        callCount = 0
    }

    private func start() {
        guard listener == nil else { return }
        AutomationFolder.shared.load()
        do {
            let parameters = NWParameters.tcp
            // `requiredInterfaceType = .loopback` alone still binds the
            // wildcard address (lsof shows *:port); pin the local endpoint.
            parameters.requiredLocalEndpoint = NWEndpoint.hostPort(
                host: .ipv4(.loopback), port: NWEndpoint.Port(rawValue: bindPort)!)
            let listener = try makeListener(parameters)
            listener.newConnectionHandler = { [weak self] connection in
                Self.serve(connection) { [weak self] request in await self?.respond(to: request) ?? Data() }
            }
            let bindPort = bindPort
            listener.stateUpdateHandler = { [weak self, weak listener] state in
                NSLog("automation server \(bindPort): \(state)")
                let port = listener?.port?.rawValue
                let event: ListenerState = if case .ready = state { .ready } else if case .failed = state { .failed } else { .other }
                Task { @MainActor [weak self] in self?.listenerChanged(event, port: port) }
            }
            listener.start(queue: .global(qos: .userInitiated))
            self.listener = listener
        } catch {
            NSLog("automation server failed to start: \(error)")
        }
    }

    /// What the listener's state means here.
    enum ListenerState: Sendable { case ready, failed, other }

    /// The listener's state changed: ready (on `port`) or failed.
    func listenerChanged(_ state: ListenerState, port: UInt16?) {
        switch state {
        case .ready:
            isListening = true
            listeningPort = port
        case .failed:
            isListening = false
            listeningPort = nil
        case .other:
            break
        }
    }

    // MARK: Token

    /// The secret an agent sends as `Authorization: Bearer …`. Made on first
    /// use and kept in the app's preferences; Settings can replace it.
    static var token: String {
        let defaults = UserDefaults.standard
        if let saved = defaults.string(forKey: PreferenceKey.automationToken), saved.count >= 32 { return saved }
        return regenerateToken()
    }

    @discardableResult
    static func regenerateToken() -> String {
        var bytes = [UInt8](repeating: 0, count: 24)
        _ = SecRandomCopyBytes(kSecRandomDefault, bytes.count, &bytes)
        let value = bytes.map { String(format: "%02x", $0) }.joined()
        UserDefaults.standard.set(value, forKey: PreferenceKey.automationToken)
        return value
    }

    /// The command that adds this server, with its token, to Claude Code.
    static var addCommand: String {
        "claude mcp add --transport http schematical \(url) --header \"Authorization: Bearer \(token)\""
    }

    // MARK: Request checks (pure, so they can be tested)

    /// Why a request is refused, as an HTTP status line, or nil when it may
    /// go on. `headers` have lowercased names.
    nonisolated static func refusal(method: String, path: String, headers: [String: String],
                                    token: String) -> String? {
        // A browser page can reach 127.0.0.1 too; MCP clients send no Origin.
        if headers["origin"] != nil { return "403 Forbidden" }
        let host = (headers["host"] ?? "").lowercased()
        let hosts: Set<String> = ["127.0.0.1:\(port)", "localhost:\(port)", "127.0.0.1", "localhost"]
        if !hosts.contains(host) { return "403 Forbidden" }
        let supplied = (headers["authorization"] ?? "")
        let expected = "Bearer \(token)"
        if !constantTimeEqual(supplied, expected) { return "401 Unauthorized" }
        if path != "/mcp" { return "404 Not Found" }
        if method != "POST" { return "405 Method Not Allowed" }
        return nil
    }

    nonisolated static func constantTimeEqual(_ a: String, _ b: String) -> Bool {
        let x = Array(a.utf8), y = Array(b.utf8)
        var difference = x.count ^ y.count
        for index in 0..<min(x.count, y.count) { difference |= Int(x[index] ^ y[index]) }
        return difference == 0
    }

    // MARK: HTTP (just enough)

    /// What answers a whole request: the reply's bytes (none once the
    /// server is gone).
    typealias Responder = @MainActor @Sendable (HTTPRequest) async -> Data

    private nonisolated static func serve(_ connection: NWConnection, _ respond: @escaping Responder) {
        connection.start(queue: .global(qos: .userInitiated))
        receive(connection, buffered: Data(), respond)
    }

    /// Reads until one whole request (headers + Content-Length body) is in.
    private nonisolated static func receive(_ connection: NWConnection, buffered: Data, _ respond: @escaping Responder) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 1 << 20) { chunk, _, isDone, error in
            var data = buffered
            if let chunk { data.append(chunk) }
            if error != nil || (isDone && chunk == nil) { connection.cancel(); return }
            if data.count > maxRequestBytes + 16_384 {
                connection.send(content: Data("HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n".utf8),
                                completion: .contentProcessed { _ in connection.cancel() })
                return
            }
            guard let request = HTTPRequest(parsing: data) else {
                receive(connection, buffered: data, respond)
                return
            }
            Task { @MainActor in
                let response = await respond(request)
                connection.send(content: response, completion: .contentProcessed { _ in
                    connection.cancel()
                })
            }
        }
    }

    nonisolated struct HTTPRequest: Sendable {
        let method: String
        let path: String
        let headers: [String: String]
        let body: Data

        /// nil until the whole request has arrived.
        init?(parsing data: Data) {
            guard let split = data.range(of: Data("\r\n\r\n".utf8)),
                  let head = String(data: data[..<split.lowerBound], encoding: .utf8) else { return nil }
            var lines = head.components(separatedBy: "\r\n")
            let first = lines.removeFirst().split(separator: " ")
            guard first.count >= 2 else { return nil }
            var headers: [String: String] = [:]
            for line in lines {
                guard let colon = line.firstIndex(of: ":") else { continue }
                headers[line[..<colon].lowercased()] = line[line.index(after: colon)...]
                    .trimmingCharacters(in: .whitespaces)
            }
            let length = Int(headers["content-length"] ?? "0") ?? 0
            let body = data[split.upperBound...]
            guard body.count >= length else { return nil }
            self.method = String(first[0])
            self.path = String(first[1])
            self.headers = headers
            self.body = Data(body.prefix(length))
        }
    }

    func http(_ status: String, json: Any? = nil) -> Data {
        var body = Data()
        if let json, let data = try? JSONSerialization.data(withJSONObject: json) { body = data }
        var head = "HTTP/1.1 \(status)\r\nConnection: close\r\nContent-Length: \(body.count)\r\n"
        if status.hasPrefix("401") { head += "WWW-Authenticate: Bearer\r\n" }
        if !body.isEmpty { head += "Content-Type: application/json\r\n" }
        return Data((head + "\r\n").utf8) + body
    }

    // MARK: MCP

    func respond(to request: HTTPRequest) async -> Data {
        if let refusal = Self.refusal(method: request.method, path: request.path,
                                      headers: request.headers, token: Self.token) {
            return http(refusal)
        }
        if request.body.count > Self.maxRequestBytes { return http("413 Payload Too Large") }
        guard let message = (try? JSONSerialization.jsonObject(with: request.body)) as? [String: Any],
              let method = message["method"] as? String else {
            return http("400 Bad Request")
        }
        lastRequest = Date()
        // Notifications (no id) get 202 and no body.
        guard let id = message["id"] else { return http("202 Accepted") }
        let params = message["params"] as? [String: Any] ?? [:]
        func reply(_ result: [String: Any]) -> Data {
            http("200 OK", json: ["jsonrpc": "2.0", "id": id, "result": result])
        }
        switch method {
        case "initialize":
            if let info = params["clientInfo"] as? [String: Any], let name = info["name"] as? String {
                clientName = name
            }
            return reply([
                "protocolVersion": params["protocolVersion"] as? String ?? "2025-06-18",
                "capabilities": ["tools": [String: Any]()],
                "serverInfo": ["name": "schematical", "version": "1.0"],
                "instructions": AutomationGuide.text,
            ])
        case "ping":
            return reply([:])
        case "tools/list":
            return reply(["tools": AutomationTools.definitions])
        case "resources/list":
            return reply(["resources": [Any]()])
        case "prompts/list":
            return reply(["prompts": [Any]()])
        case "tools/call":
            let name = params["name"] as? String ?? ""
            let arguments = params["arguments"] as? [String: Any] ?? [:]
            callCount += 1
            return reply(await AutomationTools.call(name, arguments))
        default:
            return http("200 OK", json: ["jsonrpc": "2.0", "id": id,
                                         "error": ["code": -32601, "message": "unknown method \(method)"]])
        }
    }
}
