//
//  MemoryFileAccess.swift
//  schematical
//
//  An EEPROM's load file: choosing it, keeping access to it in the sandbox
//  (an app-scoped security bookmark, plus a path relative to the schematic),
//  and turning it into the chip's start contents through the backend's
//  parser (memory_image.h).
//

import AppKit
import Foundation

/// Why a load file could not be used. The text names the file.
nonisolated enum MemoryFileError: LocalizedError, Equatable {
    case unreadable(name: String, reason: String)
    case invalid(name: String, message: String)

    var errorDescription: String? {
        switch self {
        case let .unreadable(name, reason): "can't read \(name): \(reason)"
        case let .invalid(name, message): "\(name): \(message)"
        }
    }
}

nonisolated enum MemoryFileAccess {
    /// Open panel (any file); returns the file with an app-scoped bookmark,
    /// made while the panel's access is live. nil when cancelled.
    @MainActor
    static func choose(attachedTo window: NSWindow?) async -> MemoryFile? {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.canChooseFiles = true
        panel.prompt = "Choose"
        panel.message = "Choose a binary or Intel HEX file to load when the simulation starts."
        let response: NSApplication.ModalResponse
        if let window {
            response = await panel.beginSheetModal(for: window)
        } else {
            response = panel.runModal()
        }
        guard response == .OK, let url = panel.url else { return nil }
        return MemoryFile(url: url, bookmark: bookmark(for: url))
    }

    /// The file's bytes, read under its security scope.
    static func read(_ file: MemoryFile) throws -> Data {
        var url = file.url
        if let bookmark = file.bookmark, let resolved = resolve(bookmark) {
            url = resolved.url
        }
        let scoped = url.startAccessingSecurityScopedResource()
        defer {
            if scoped { url.stopAccessingSecurityScopedResource() }
        }
        do {
            return try Data(contentsOf: url)
        } catch {
            throw MemoryFileError.unreadable(name: file.name, reason: error.localizedDescription)
        }
    }

    /// The chip's start contents: the parsed file, or all 0xFF without one.
    static func image(_ file: MemoryFile?, size: MemorySize) throws -> [UInt32] {
        var values = [UInt32](repeating: 0xFF, count: size.words)
        guard let file else { return values }
        let data = try read(file)
        var error: UnsafeMutablePointer<CChar>?
        let result = data.withUnsafeBytes { raw in
            values.withUnsafeMutableBufferPointer { out in
                memory_image_parse(raw.bindMemory(to: UInt8.self).baseAddress, raw.count,
                                   memory_image_format_for(file.name), UInt32(size.words),
                                   UInt32(size.bits), out.baseAddress, &error)
            }
        }
        guard result == 0 else {
            let message = error.map { String(cString: $0) } ?? "not a load file"
            free(error)
            throw MemoryFileError.invalid(name: file.name, message: message)
        }
        return values
    }

    // MARK: Bookmarks

    /// An app-scoped security bookmark for a file the app can read now, or nil.
    static func bookmark(for url: URL) -> Data? {
        try? url.bookmarkData(options: [.withSecurityScope, .securityScopeAllowOnlyReadAccess],
                              includingResourceValuesForKeys: nil, relativeTo: nil)
    }

    /// The file a bookmark points to, with the bookmark to keep: the same one,
    /// or a fresh one when it was stale. nil when it no longer resolves.
    static func resolve(_ bookmark: Data) -> (url: URL, bookmark: Data)? {
        var stale = false
        guard let url = try? URL(resolvingBookmarkData: bookmark, options: [.withSecurityScope, .withoutUI],
                                 relativeTo: nil, bookmarkDataIsStale: &stale) else { return nil }
        guard stale else { return (url, bookmark) }
        let scoped = url.startAccessingSecurityScopedResource()
        defer {
            if scoped { url.stopAccessingSecurityScopedResource() }
        }
        return (url, Self.bookmark(for: url) ?? bookmark)
    }

    // MARK: Paths in the schematic file

    /// How a schematic saved at `schematic` names `file`: relative to the
    /// schematic's folder (`../roms/boot.bin`), or the absolute path when the
    /// schematic has never been saved.
    static func path(of file: URL, schematic: URL?) -> String {
        guard let schematic else { return file.standardizedFileURL.path }
        let from = schematic.deletingLastPathComponent().resolvingSymlinksInPath().standardizedFileURL.pathComponents
        let to = file.resolvingSymlinksInPath().standardizedFileURL.pathComponents
        var common = 0
        while common < from.count, common < to.count, from[common] == to[common] {
            common += 1
        }
        let ups = Array(repeating: "..", count: from.count - common)
        return (ups + to[common...]).joined(separator: "/")
    }

    /// The file a stored path names: relative to the schematic's folder, or
    /// absolute as it is.
    static func url(forPath path: String, schematic: URL?) -> URL {
        if path.hasPrefix("/") || schematic == nil {
            return URL(fileURLWithPath: path).standardizedFileURL
        }
        return schematic!.deletingLastPathComponent().appendingPathComponent(path).standardizedFileURL
    }

    /// The load file a schematic stores: the bookmark when it resolves on this
    /// Mac, else the path (with no bookmark). Never fails; a missing file
    /// shows up when it is read.
    static func file(path: String, bookmark: Data?, schematic: URL?) -> MemoryFile {
        if let bookmark, let resolved = resolve(bookmark) {
            return MemoryFile(url: resolved.url, bookmark: resolved.bookmark)
        }
        return MemoryFile(url: url(forPath: path, schematic: schematic), bookmark: nil)
    }
}
