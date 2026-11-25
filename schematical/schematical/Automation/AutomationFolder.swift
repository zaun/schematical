//
//  AutomationFolder.swift
//  schematical
//
//  The one folder an agent may open, save and export files in. The app is
//  sandboxed, so the user grants it (Settings ▸ AI Agents ▸ Files) and the
//  grant is kept as a security-scoped bookmark. With no folder, the file
//  tools are off. Paths are checked to stay inside it.
//

import AppKit
import Foundation
import Observation

@MainActor
@Observable
final class AutomationFolder {
    static let shared = AutomationFolder()

    /// The chosen folder, while its access is open.
    private(set) var url: URL?

    @ObservationIgnored
    private var isAccessing = false

    /// Opens access to the saved folder. Safe to call again.
    func load() {
        guard url == nil,
              let bookmark = UserDefaults.standard.data(forKey: PreferenceKey.automationFolder) else { return }
        var stale = false
        guard let resolved = try? URL(resolvingBookmarkData: bookmark, options: [.withSecurityScope, .withoutUI],
                                      relativeTo: nil, bookmarkDataIsStale: &stale) else { return }
        open(resolved)
        if stale { remember(resolved) }
    }

    /// Asks the user for the folder.
    func choose() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.canCreateDirectories = true
        panel.allowsMultipleSelection = false
        panel.prompt = "Allow"
        panel.message = "Choose the folder AI agents may open and save schematics in."
        guard panel.runModal() == .OK, let chosen = panel.url else { return }
        close()
        open(chosen)
        remember(chosen)
    }

    /// Forgets the folder: the file tools go off.
    func clear() {
        close()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.automationFolder)
    }

    private func open(_ folder: URL) {
        isAccessing = folder.startAccessingSecurityScopedResource()
        url = folder
    }

    /// Shares `folder` for now, saving no bookmark (the unit tests use a
    /// temporary folder). Call the returned closure to put back what was
    /// shared before.
    func shareTemporarily(_ folder: URL) -> () -> Void {
        load()
        let before = (url, isAccessing)
        url = folder
        isAccessing = false
        return { [weak self] in
            self?.url = before.0
            self?.isAccessing = before.1
        }
    }

    private func close() {
        if isAccessing { url?.stopAccessingSecurityScopedResource() }
        isAccessing = false
        url = nil
    }

    private func remember(_ folder: URL) {
        if let data = try? folder.bookmarkData(options: .withSecurityScope, includingResourceValuesForKeys: nil,
                                               relativeTo: nil) {
            UserDefaults.standard.set(data, forKey: PreferenceKey.automationFolder)
        }
    }

    /// The file `path` names: relative to the folder, or absolute inside it.
    /// Throws when no folder is chosen or the path leaves it.
    func resolve(_ path: String) throws -> URL {
        load()
        guard let folder = url else {
            throw AutomationFailure("No folder is shared with agents. In Schematical ▸ Settings ▸ AI Agents, "
                                    + "choose one under Files.")
        }
        return try Self.contain(path, in: folder)
    }

    /// The file `path` names inside `folder`: relative to it, or absolute.
    /// Symlinks and ".." are resolved first, so nothing leads out.
    nonisolated static func contain(_ path: String, in folder: URL) throws -> URL {
        let root = folder.resolvingSymlinksInPath().standardizedFileURL
        let candidate = path.hasPrefix("/") ? URL(fileURLWithPath: path) : root.appendingPathComponent(path)
        // Resolve the parent (the file may not exist yet) so symlinks can't lead out.
        let parent = candidate.deletingLastPathComponent().resolvingSymlinksInPath().standardizedFileURL
        let resolved = parent.appendingPathComponent(candidate.lastPathComponent)
        guard resolved.path.hasPrefix(root.path + "/"), candidate.lastPathComponent != ".." else {
            throw AutomationFailure("\(path) is outside the shared folder \(root.path)")
        }
        return resolved
    }

    /// The schematic files in the folder (one level deep), as relative paths.
    func schematics() throws -> [String] {
        load()
        guard let folder = url else {
            throw AutomationFailure("No folder is shared with agents (Settings ▸ AI Agents ▸ Files).")
        }
        let root = folder.resolvingSymlinksInPath().standardizedFileURL
        let names = (try? FileManager.default.contentsOfDirectory(atPath: root.path)) ?? []
        return names.filter { $0.hasSuffix(".\(SchematicFile.fileExtension)") }.sorted()
    }
}
