//
//  AutomationServer.swift
//  schematical
//
//  SPIKE (planning/spike-agent-control.md): a loopback HTTP server that
//  speaks just enough MCP (Streamable HTTP, JSON replies only) for an AI
//  agent to drive the running app. Off until Settings ▸ AI Agents turns it
//  on (`PreferenceKey.automationEnabled`).
//
//  Binds 127.0.0.1 only, refuses any request carrying an Origin header (a
//  browser page), and handles every tool call on the main actor, against
//  the same SchematicDocument the window draws.
//

import Foundation
import Network

@MainActor
final class AutomationServer {
    static let shared = AutomationServer()

    /// The port the server listens on.
    static let port: UInt16 = 47800

    /// Where an agent connects.
    static let url = "http://127.0.0.1:\(port)/mcp"

    /// The command that adds this server to Claude Code.
    static let addCommand = "claude mcp add --transport http schematical \(url)"

    private var listener: NWListener?

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
    }

    private func start() {
        guard listener == nil else { return }
        do {
            let parameters = NWParameters.tcp
            // `requiredInterfaceType = .loopback` alone still binds the
            // wildcard address (lsof shows *:port); pin the local endpoint.
            parameters.requiredLocalEndpoint = NWEndpoint.hostPort(
                host: .ipv4(.loopback), port: NWEndpoint.Port(rawValue: Self.port)!)
            let listener = try NWListener(using: parameters)
            listener.newConnectionHandler = { connection in
                AutomationServer.serve(connection)
            }
            listener.stateUpdateHandler = { state in
                NSLog("automation server \(AutomationServer.port): \(state)")
            }
            listener.start(queue: .global(qos: .userInitiated))
            self.listener = listener
        } catch {
            NSLog("automation server failed to start: \(error)")
        }
    }

    // MARK: HTTP (just enough)

    private nonisolated static func serve(_ connection: NWConnection) {
        connection.start(queue: .global(qos: .userInitiated))
        receive(connection, buffered: Data())
    }

    /// Reads until one whole request (headers + Content-Length body) is in.
    private nonisolated static func receive(_ connection: NWConnection, buffered: Data) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 1 << 20) { chunk, _, isDone, error in
            var data = buffered
            if let chunk { data.append(chunk) }
            if error != nil || (isDone && chunk == nil) { connection.cancel(); return }
            guard let request = HTTPRequest(parsing: data) else {
                receive(connection, buffered: data)
                return
            }
            Task { @MainActor in
                let response = await AutomationServer.shared.respond(to: request)
                connection.send(content: response, completion: .contentProcessed { _ in
                    connection.cancel()
                })
            }
        }
    }

    private nonisolated struct HTTPRequest {
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

    private func http(_ status: String, json: Any? = nil) -> Data {
        var body = Data()
        if let json, let data = try? JSONSerialization.data(withJSONObject: json) { body = data }
        var head = "HTTP/1.1 \(status)\r\nConnection: close\r\nContent-Length: \(body.count)\r\n"
        if !body.isEmpty { head += "Content-Type: application/json\r\n" }
        return Data((head + "\r\n").utf8) + body
    }

    // MARK: MCP

    private func respond(to request: HTTPRequest) async -> Data {
        // A browser page can reach 127.0.0.1 too; MCP clients send no Origin.
        if request.headers["origin"] != nil { return http("403 Forbidden") }
        guard request.path == "/mcp" else { return http("404 Not Found") }
        guard request.method == "POST" else { return http("405 Method Not Allowed") }
        guard let message = (try? JSONSerialization.jsonObject(with: request.body)) as? [String: Any],
              let method = message["method"] as? String else {
            return http("400 Bad Request")
        }
        // Notifications (no id) get 202 and no body.
        guard let id = message["id"] else { return http("202 Accepted") }
        let params = message["params"] as? [String: Any] ?? [:]
        func reply(_ result: [String: Any]) -> Data {
            http("200 OK", json: ["jsonrpc": "2.0", "id": id, "result": result])
        }
        switch method {
        case "initialize":
            return reply([
                "protocolVersion": params["protocolVersion"] as? String ?? "2025-06-18",
                "capabilities": ["tools": [String: Any]()],
                "serverInfo": ["name": "schematical", "version": "0.1-spike"],
            ])
        case "ping":
            return reply([:])
        case "tools/list":
            return reply(["tools": AutomationTools.definitions])
        case "tools/call":
            let name = params["name"] as? String ?? ""
            let arguments = params["arguments"] as? [String: Any] ?? [:]
            return reply(AutomationTools.call(name, arguments))
        default:
            return http("200 OK", json: ["jsonrpc": "2.0", "id": id,
                                         "error": ["code": -32601, "message": "unknown method \(method)"]])
        }
    }
}
