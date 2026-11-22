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
