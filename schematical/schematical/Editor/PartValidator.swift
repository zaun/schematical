import Foundation

/// Checks a Part Editor draft with the backend's own loader (plan-32-C § 4.7):
/// the draft's file goes through `load_part_file` in a scratch library that
/// holds only the bundled footprints and datasheets, so its footprint and
/// datasheet references resolve; the loaded part is copied out and taken back
/// (`part_library_unload_last_part_file`), so one library checks any number
/// of drafts. The message on failure is the loader's, naming the place.
nonisolated final class PartValidator: @unchecked Sendable {
    enum Failure: Error, Equatable {
        case backend(String)
        case unavailable(String)

        var message: String {
            switch self {
            case let .backend(text), let .unavailable(text): text
            }
        }
    }

    private let library = PartLibraryHandle()
    private let lock = NSLock()
    private let setupError: String?

    /// `loadSupport` fills the scratch library (the bundled footprints and
    /// datasheets); when it throws, every check answers `.unavailable`.
    init(loadSupport: (PartLibraryHandle) throws -> Void = BackendCatalog.loadSupportFiles(into:)) {
        do {
            try loadSupport(library)
            setupError = nil
        } catch {
            setupError = error.localizedDescription
        }
    }

    /// The bundled footprints, for the editor's footprint picker.
    var footprints: [Footprint] {
        lock.lock()
        defer { lock.unlock() }
        if let kept = keptFootprints { return kept }
        let found = (0..<part_library_footprint_count(library.pointer)).compactMap { index in
            part_library_footprint_at(library.pointer, index).map { Footprint($0.pointee) }
        }
        keptFootprints = found
        return found
    }
    private var keptFootprints: [Footprint]?

    /// The bundled datasheet ids, for the editor's datasheet picker.
    var datasheetIDs: [String] {
        lock.lock()
        defer { lock.unlock() }
        if let kept = keptDatasheetIDs { return kept }
        let found = (0..<part_library_datasheet_count(library.pointer)).compactMap { index in
            part_library_datasheet_at(library.pointer, index).map { String(cString: $0.pointee.id) }
        }
        keptDatasheetIDs = found
        return found
    }
    private var keptDatasheetIDs: [String]?

    /// Checks a file's text; the part it describes, or the loader's message.
    func validate(text: String, source: String = "part") -> Result<CatalogPart, Failure> {
        if let setupError { return .failure(.unavailable(setupError)) }
        lock.lock()
        defer { lock.unlock() }
        var error: UnsafeMutablePointer<CChar>?
        if load_part_file(library.pointer, text, source, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? source
            free(error)
            return .failure(.backend(message))
        }
        defer { part_library_unload_last_part_file(library.pointer) }
        // A file that loads has added its part last, so the library holds at
        // least that part (`load_part_file` fails on a file with none).
        let loaded = part_library_at(library.pointer, part_library_count(library.pointer) - 1)!
        return .success(BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) })
    }

    func validate(_ draft: PartDraft, source: String = "part") -> Result<CatalogPart, Failure> {
        guard let text = try? draft.encoded().text() else {
            return .failure(.backend("\(source): the draft could not be written"))
        }
        return validate(text: text, source: source)
    }

    /// `validate`, off the main actor.
    func validated(_ draft: PartDraft, source: String = "part") async -> Result<CatalogPart, Failure> {
        await Task.detached(priority: .userInitiated) { self.validate(draft, source: source) }.value
    }

    /// One validator for the app (its scratch library is read once).
    static let shared = PartValidator()
}
