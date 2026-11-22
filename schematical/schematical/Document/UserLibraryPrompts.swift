import AppKit
import Foundation

/// The user's answer to "add this part to your User Library?" (plan-53-C).
enum LibraryChoice {
    case add, useOnce, cancel
}

/// What File › Load Part… did.
enum LoadPartOutcome {
    case added(CatalogPart)
    case usedOnce(CatalogPart)
    /// The part was already in the User Library, so nothing was asked.
    case alreadyInLibrary(CatalogPart)
    case cancelled
}

/// The decisions behind the User Library's offers (plan-53-C): pure, so tests
/// drive them with a stub answer; the alerts are built separately.
@MainActor
enum UserLibraryPrompts {
    /// The parts of `parts` that are not in the User Library.
    static func offerable(_ parts: [CatalogPart], catalog: PartCatalogStore) -> [CatalogPart] {
        parts.filter { catalog.listing(of: $0) != .user }
    }

    /// Adds each part to the User Library; returns the parts that failed, by
    /// name, with the error. The others are added.
    static func add(_ parts: [CatalogPart], catalog: PartCatalogStore) -> [(name: String, error: Error)] {
        var failures: [(name: String, error: Error)] = []
        for part in parts {
            do {
                try catalog.addToUserLibrary(part)
            } catch {
                failures.append((part.name, error))
            }
        }
        return failures
    }
}

/// The alerts (plan-53-C § 3): built here as plain `NSAlert`s so a test can
/// read their text and buttons; the first button is the default.
extension UserLibraryPrompts {
    static func loadPartAlert(name: String) -> NSAlert {
        let alert = NSAlert()
        alert.messageText = "Add “\(name)” to your User Library?"
        alert.informativeText = "Adding keeps it for every schematic. Using it once keeps it until you quit."
        alert.addButton(withTitle: "Add to Library")
        alert.addButton(withTitle: "Use Once")
        alert.addButton(withTitle: "Cancel")
        return alert
    }

    static func alreadyInLibraryAlert(name: String) -> NSAlert {
        let alert = NSAlert()
        alert.messageText = "“\(name)” is already in your User Library."
        alert.addButton(withTitle: "OK")
        return alert
    }

    /// Up to 8 names, one per line, then `and N more`.
    static func fileOpenAlert(names: [String]) -> NSAlert {
        let alert = NSAlert()
        alert.messageText = "This schematic has custom parts that are not in your User Library"
        var lines = names.prefix(8).map { "• \($0)" }
        if names.count > 8 { lines.append("and \(names.count - 8) more") }
        alert.informativeText = lines.joined(separator: "\n") + "\n\nThey are saved in the schematic either way."
        alert.addButton(withTitle: "Add to Library")
        alert.addButton(withTitle: "Not Now")
        return alert
    }

    static func removeAlert(name: String) -> NSAlert {
        let alert = NSAlert()
        alert.messageText = "Remove “\(name)” from your User Library?"
        alert.informativeText = "Schematics that use it keep their own copy."
        alert.addButton(withTitle: "Remove")
        alert.addButton(withTitle: "Cancel")
        return alert
    }

    /// Removing several parts at once (plan-53-F).
    static func removeAlert(count: Int) -> NSAlert {
        let alert = NSAlert()
        alert.messageText = "Remove \(count) parts from your User Library?"
        alert.informativeText = "Schematics that use them keep their own copy."
        alert.addButton(withTitle: "Remove")
        alert.addButton(withTitle: "Cancel")
        return alert
    }

    /// One alert for the failures of an add or remove: a `NAME: reason` line each.
    static func failuresAlert(title: String, failures: [(name: String, error: Error)]) -> NSAlert {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = title
        alert.informativeText = failures.map { "\($0.name): \($0.error.localizedDescription)" }.joined(separator: "\n")
        alert.addButton(withTitle: "OK")
        return alert
    }

    /// Shows `alert` as a sheet on the key window, or on its own.
    static func run(_ alert: NSAlert, on window: NSWindow? = nil) async -> NSApplication.ModalResponse {
        if let window = window ?? NSApp.keyWindow {
            return await alert.beginSheetModal(for: window)
        }
        return alert.runModal()
    }

    /// Adds `parts`, showing the failures alert when any could not be added.
    static func addShowingFailures(_ parts: [CatalogPart], catalog: PartCatalogStore, on window: NSWindow? = nil) async {
        let failures = add(parts, catalog: catalog)
        if !failures.isEmpty {
            _ = await run(failuresAlert(title: "Couldn't add to your User Library", failures: failures), on: window)
        }
    }
}

extension PartFileLoading {
    /// File › Load Part… after its panel: reads the file's identity, asks
    /// (`ask`, with the part's name) unless the part is already in the User
    /// Library with the same content, then adds it to the library, uses it
    /// once (the File Library) or does nothing. The backend's message is
    /// thrown for a file it refuses.
    static func load(text: String, source: String, catalog: PartCatalogStore,
                     ask: (String) async -> LibraryChoice) async throws -> LoadPartOutcome {
        let file = try PartFileObject(text: text)
        if let uuid = file.uuid, let registered = catalog.part(uuid: uuid),
           catalog.listing(of: registered) == .user, !catalog.uuidConflicts(text) {
            return .alreadyInLibrary(registered)
        }
        switch await ask(file.name ?? source) {
        case .add:
            return .added(try catalog.addToUserLibrary(fileText: text, source: source))
        case .useOnce:
            return .usedOnce(try catalog.register(fileText: text, source: source))
        case .cancel:
            return .cancelled
        }
    }
}
