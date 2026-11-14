//
//  AutomationSession.swift
//  schematical
//
//  Automation tools about windows, files, undo and batching.
//

import AppKit
import Foundation

extension AutomationTools {
    /// Tools that may not run inside a `batch`: they wait, open windows, or
    /// run for a long time.
    private static let notBatchable: Set<String> = [
        "batch", "wait", "new_window", "open", "simulation_start", "list_windows", "focus_window",
    ]

    static let sessionTools: [AutomationTool] = [
        AutomationTool(
            name: "list_windows",
            summary: "The open schematic windows (tabs): id, name, part and wire counts, unsaved changes, "
                + "which is frontmost, whether a simulation is running. Pass the id as window to other tools.",
            needsWindow: false, readOnly: true,
            run: { _, _ in
                let all = sessions()
                if all.isEmpty { return Out.text("no windows open (use new_window)") }
                let front = (try? resolveWindow(nil))?.id
                let lines = all.map { window in
                    "\(window.shortID)  \(window.file?.displayName ?? "?")"
                        + "  parts=\(window.document.chips.count) wires=\(window.document.wires.count)"
                        + (window.file?.hasUnsavedChanges == true ? "  unsaved" : "")
                        + (window.simulation.isRunning ? "  simulating" : "")
                        + (window.id == front ? "  (frontmost)" : "")
                }
                return Out.text(lines.joined(separator: "\n"))
            }),
        AutomationTool(
            name: "list_files",
            summary: "The .schematical files in the folder the user shared with agents (Settings ▸ AI Agents ▸ Files).",
            needsWindow: false, readOnly: true,
            run: { _, _ in
                let files = try AutomationFolder.shared.schematics()
                let root = AutomationFolder.shared.url?.path ?? ""
                return Out.text(files.isEmpty ? "no schematics in \(root)" : "in \(root):\n" + files.joined(separator: "\n"))
            }),
        AutomationTool(
            name: "focus_window",
            summary: "Bring a window to the front.",
            readOnly: true,
            run: { context, _ in
                guard let window = context!.file?.window else { throw AutomationFailure("that window isn't on screen") }
                window.makeKeyAndOrderFront(nil)
                return Out.text("window \(context!.shortID) is frontmost")
            }),
        AutomationTool(
            name: "new_window",
            summary: "Open a new blank schematic as a tab of the frontmost window. Returns its id.",
            run: { context, _ in
                guard let file = context?.file else { throw AutomationFailure("no window to open a tab from") }
                let known = Set(sessions().map(\.id))
                SchematicFileController.tabNewProject(focused: file)
                file.openNewWindow?()
                for _ in 0..<40 {
                    try await Task.sleep(nanoseconds: 50_000_000)
                    if let fresh = sessions().first(where: { !known.contains($0.id) }) {
                        return Out.text("opened window \(fresh.shortID)")
                    }
                }
                throw AutomationFailure("the new window didn't appear")
            }),
        AutomationTool(
            name: "open",
            summary: "Open a .schematical file (in a new tab, or this window if it is blank). Only files in the "
                + "folder the user shared in Settings ▸ AI Agents ▸ Files; see list_files.",
            properties: ["path": Schema.string("File name or path inside the shared folder")], required: ["path"],
            run: { context, args in
                let url = try AutomationFolder.shared.resolve(try args.string("path"))
                guard let file = context?.file else { throw AutomationFailure("no window to open it from") }
                do { _ = try SchematicFile.decode(try Data(contentsOf: url)) }
                catch { throw AutomationFailure("can't open \(url.lastPathComponent): \(error.localizedDescription)") }
                let known = Set(sessions().map(\.id))
                await file.open(url)
                try await Task.sleep(nanoseconds: 400_000_000)
                let fresh = sessions().first { !known.contains($0.id) }
                return Out.text(fresh.map { "opened in window \($0.shortID)" } ?? "opened (in window \(context!.shortID))")
            }),
        AutomationTool(
            name: "save",
            summary: "Save the design to path inside the shared folder (Settings ▸ AI Agents ▸ Files), or to the "
                + "window's own file when no path is given.",
            properties: ["path": Schema.string("File name or path inside the shared folder, ending .schematical (optional)")],
            readOnly: false,
            run: { context, args in
                let url = try args.optString("path").map { try AutomationFolder.shared.resolve($0) }
                guard let file = context?.file else { throw AutomationFailure("no file controller for this window") }
                if let problem = file.saveWithoutPanel(to: url) { throw AutomationFailure(problem) }
                return Out.text("saved to \(file.fileURL?.path ?? "?")")
            }),
        AutomationTool(
            name: "export_gerber",
            summary: "Write the board's Gerber and drill files as a zip to path inside the shared folder (for a fab). "
                + "Run run_drc first.",
            properties: ["path": Schema.string("File name or path inside the shared folder, ending .zip")],
            required: ["path"], readOnly: true,
            run: { context, args in
                let url = try AutomationFolder.shared.resolve(try args.string("path"))
                guard let file = context?.file else { throw AutomationFailure("no file controller for this window") }
                do { try file.writeGerber(to: url) }
                catch { throw AutomationFailure("couldn't write \(url.lastPathComponent): \(error.localizedDescription)") }
                return Out.text("wrote \(url.path)")
            }),
        AutomationTool(
            name: "undo",
            summary: "Undo the last edit on a page (schematic or pcb), exactly as the user's Undo would.",
            properties: ["page": Schema.string("schematic (default) or pcb", options: ["schematic", "pcb"])],
            edits: true,
            run: { context, args in Out.text(try undoRedo(context!, args, redo: false)) }),
        AutomationTool(
            name: "redo",
            summary: "Redo the last undone edit on a page.",
            properties: ["page": Schema.string("schematic (default) or pcb", options: ["schematic", "pcb"])],
            edits: true,
            run: { context, args in Out.text(try undoRedo(context!, args, redo: true)) }),
        AutomationTool(
            name: "batch",
            summary: "Run several tool calls in order as ONE undo step. calls is a list of "
                + "{\"tool\": name, \"arguments\": {…}}. Stops at the first error and says which call failed "
                + "(the earlier ones stay applied, and one undo reverts them together). Use it to build a "
                + "circuit in one request.",
            properties: [
                "calls": ["type": "array", "description": "The calls, in order",
                          "items": ["type": "object",
                                    "properties": ["tool": ["type": "string"], "arguments": ["type": "object"]],
                                    "required": ["tool"]]],
                "page": Schema.string("Which page's undo history the step goes in: schematic (default) or pcb",
                                      options: ["schematic", "pcb"]),
            ],
            required: ["calls"], edits: true,
            run: { context, args in try await batch(context!, args) }),
    ]

    private static func undoRedo(_ context: AutomationContext, _ args: AutomationArgs, redo: Bool) throws -> String {
        let page = EditorPage(rawValue: try args.choice("page", among: ["schematic", "pcb"], default: "schematic")) ?? .schematic
        let document = context.document
        let name = redo ? document.redoName(on: page) : document.undoName(on: page)
        let done = redo ? document.redo(on: page) : document.undo(on: page)
        guard done else { throw AutomationFailure("nothing to \(redo ? "redo" : "undo") on the \(page.rawValue) page") }
        return "\(redo ? "redid" : "undid") \(name ?? "an edit")"
    }

    private static func batch(_ context: AutomationContext, _ args: AutomationArgs) async throws -> [[String: Any]] {
        guard let calls = args.raw["calls"] as? [[String: Any]], !calls.isEmpty else {
            throw AutomationFailure("calls must be a non-empty list of {tool, arguments}")
        }
        guard calls.count <= 200 else { throw AutomationFailure("at most 200 calls per batch") }
        let page = EditorPage(rawValue: try args.choice("page", among: ["schematic", "pcb"], default: "schematic")) ?? .schematic
        // Check every call's name first, so a typo doesn't leave half a batch.
        for (index, call) in calls.enumerated() {
            guard let name = call["tool"] as? String, let tool = byName[name] else {
                throw AutomationFailure("call \(index + 1): unknown tool \((call["tool"] as? String) ?? "(missing)")")
            }
            if notBatchable.contains(name) || !tool.needsWindow {
                throw AutomationFailure("call \(index + 1): \(name) can't run inside a batch")
            }
        }
        let document = context.document
        document.beginGesture(page)
        defer { document.endGesture() }
        var lines: [String] = []
        var images: [[String: Any]] = []
        for (index, call) in calls.enumerated() {
            let name = call["tool"] as! String
            var inner = call["arguments"] as? [String: Any] ?? [:]
            inner["window"] = context.id.uuidString
            let result = await AutomationTools.call(name, inner)
            let content = result["content"] as? [[String: Any]] ?? []
            let text = content.compactMap { $0["text"] as? String }.joined(separator: " ")
            images += content.filter { $0["type"] as? String == "image" }
            if result["isError"] as? Bool == true {
                lines.append("call \(index + 1) (\(name)) FAILED: \(text)")
                lines.append("stopped; calls 1–\(index) stay applied (one undo reverts them together)")
                throw AutomationFailure(lines.joined(separator: "\n"))
            }
            lines.append("\(index + 1). \(name): \(text)")
        }
        return Out.text(lines.joined(separator: "\n")) + images
    }
}
