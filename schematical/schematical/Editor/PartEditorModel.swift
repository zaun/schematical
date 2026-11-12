import AppKit
import Foundation
import Observation

/// One Part Editor window's state: the draft, where it was saved, whether it
/// changed since, and the backend's verdict on it (plan-32-C § 4.6, § 4.7).
@MainActor
@Observable
final class PartEditorModel {
    var draft: PartDraft {
        didSet {
            if draft != oldValue { scheduleValidation() }
        }
    }
    /// The file the draft was last saved to or opened from.
    private(set) var fileURL: URL?
    /// The draft's text when it was last saved or opened; nil for a new part.
    private(set) var savedText: String?
    /// The backend's verdict on the current draft: the part as it would load,
    /// or the loader's message. Nil until the first check finishes.
    private(set) var validation: Result<CatalogPart, PartValidator.Failure>? {
        didSet {
            if case let .success(part)? = validation {
                lastValidPart = part
                previewRevision += 1
            }
        }
    }
    /// Counts accepted checks. `CatalogPart` compares by name and uuid only,
    /// so the previews take this too, to redraw when the same part changes.
    private(set) var previewRevision = 0
    /// The last part the backend accepted, for the previews (they keep
    /// showing it while the draft has an error).
    private(set) var lastValidPart: CatalogPart?
    /// The preview pane's segment (plan-32-E).
    enum PreviewPane: String, CaseIterable, Sendable {
        case block, footprint

        var title: String {
            switch self {
            case .block: "Block"
            case .footprint: "Footprint"
            }
        }
    }

    /// Which preview shows; Block until the user picks Footprint.
    var previewPane: PreviewPane = .block

    /// A one-line result of the last Add to Library or Save.
    var status: String?
    /// A message for a target that could not be opened (an unknown part name).
    private(set) var openError: String?

    let catalog: PartCatalogStore
    let validator: PartValidator
    weak var window: NSWindow? {
        didSet { closeGuard.attach(to: window) }
    }

    @ObservationIgnored private var validationTask: Task<Void, Never>?
    @ObservationIgnored private var validationGeneration = 0
    @ObservationIgnored private lazy var closeGuard = WindowCloseGuard { [weak self] _ in
        guard let self, self.isDirty else { return true }
        Task { await self.closeAfterAsking() }
        return false
    }

    /// `catalog`: the app's catalog when nil.
    init(target: PartEditorTarget, catalog: PartCatalogStore? = nil, validator: PartValidator = .shared) {
        let catalog = catalog ?? PartCatalogStore.shared
        self.catalog = catalog
        self.validator = validator
        switch target.source {
        case let .new(kind):
            draft = PartDraft(newOf: kind)
        case let .copy(name):
            if let draft = Self.copy(named: name, catalog: catalog) {
                self.draft = draft
            } else {
                draft = PartDraft(newOf: "gate")
                openError = "There is no part named \(name) to copy."
            }
        case let .file(url):
            draft = PartDraft(newOf: "gate")
            do {
                let text = try PartFileReader.read(url: url)
                draft = try PartDraft(fileText: text)
                fileURL = url
                savedText = draft.fileText()
            } catch {
                openError = error.localizedDescription
            }
        }
        scheduleValidation(delay: .zero)
    }

    /// A copy of a library part as a draft with a new uuid; nil when the part
    /// is unknown or its kind has no form.
    static func copy(named name: String, catalog: PartCatalogStore) -> PartDraft? {
        guard let part = catalog.part(named: name) else { return nil }
        var file: JSONValue?
        if let uuid = part.uuid, let text = catalog.registeredText(uuid: uuid) {
            file = try? JSONValue(text: text)
        } else {
            file = BundledPartFile.copy(of: part)
        }
        guard let file, var draft = try? PartDraft(file: file) else { return nil }
        draft.uuid = UUID().uuidString
        return draft
    }

    // MARK: State

    var isDirty: Bool {
        draft.fileText() != (savedText ?? PartDraft(newOf: draft.kind).fileText())
    }

    var isValid: Bool {
        if case .success? = validation { return true }
        return false
    }

    var validationMessage: String? {
        if case let .failure(failure)? = validation { return failure.message }
        return nil
    }

    var title: String {
        if let fileURL { return fileURL.deletingPathExtension().lastPathComponent }
        let name = draft.name.trimmingCharacters(in: .whitespaces)
        return name.isEmpty ? "New \(PartDraft.displayName(of: draft.kind)) Part" : name
    }

    /// What Add to Library will name the part: its name, or the suffixed
    /// name when a loaded part has it (plan-32-A § 4.3).
    var libraryName: String {
        let name = draft.name
        if let existing = catalog.part(named: name), existing.uuid != draft.uuid {
            var n = 2
            while catalog.part(named: "\(name).\(n)") != nil { n += 1 }
            return "\(name).\(n)"
        }
        return name
    }

    // MARK: Validation

    func scheduleValidation(delay: Duration = .milliseconds(250)) {
        validationTask?.cancel()
        validationGeneration += 1
        let generation = validationGeneration
        let draft = draft
        let validator = validator
        validationTask = Task { [weak self] in
            if delay > .zero {
                try? await Task.sleep(for: delay)
                if Task.isCancelled { return }
            }
            let result = await validator.validated(draft, source: "\(draft.name.isEmpty ? "part" : draft.name).schpart")
            guard let self, !Task.isCancelled, generation == self.validationGeneration else { return }
            self.validation = result
        }
    }

    /// Validates the current draft now (Save and Add check the latest draft).
    func validateNow() -> Result<CatalogPart, PartValidator.Failure> {
        validationTask?.cancel()
        validationGeneration += 1
        let result = validator.validate(draft, source: "\(draft.name.isEmpty ? "part" : draft.name).schpart")
        validation = result
        return result
    }

    // MARK: Save, open, add

    /// Writes the draft to `url`; refused while the backend rejects the draft.
    func write(to url: URL) throws {
        if case let .failure(failure) = validateNow() {
            throw PartRegistrationError.backend(failure.message)
        }
        let text = draft.fileText()
        try Data(text.utf8).write(to: url, options: .atomic)
        fileURL = url
        savedText = text
        status = "Saved \(url.lastPathComponent)"
    }

    /// Replaces the draft with a file's text. The file must load (the
    /// backend's message otherwise), and the draft is left as it was on any
    /// failure.
    func load(text: String, from url: URL?) throws {
        let next = try PartDraft(fileText: text)
        if case let .failure(failure) = validator.validate(next, source: url?.lastPathComponent ?? "part") {
            throw PartRegistrationError.backend(failure.message)
        }
        draft = next
        fileURL = url
        savedText = next.fileText()
        status = url.map { "Opened \($0.lastPathComponent)" }
    }

    /// Registers the draft's part in the app's library (plan-32-A), for the
    /// rest of the session.
    @discardableResult
    func addToLibrary() throws -> CatalogPart {
        if case let .failure(failure) = validateNow() {
            throw PartRegistrationError.backend(failure.message)
        }
        let part = try catalog.register(fileText: draft.encoded().text(), source: "\(draft.name).schpart")
        status = "Added \(part.name) to the parts drawer"
        return part
    }

    func save() async {
        if let fileURL {
            do { try write(to: fileURL) } catch { await present(error, title: "Couldn't save the part") }
        } else {
            await saveAs()
        }
    }

    func saveAs() async {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [PartFileType.contentType]
        panel.nameFieldStringValue = (draft.name.isEmpty ? "Untitled" : draft.name) + "." + PartFileType.fileExtension
        guard await run(panel) == .OK, let url = panel.url else { return }
        do { try write(to: url) } catch { await present(error, title: "Couldn't save the part") }
    }

    func open() async {
        guard await confirmDiscardingChanges() else { return }
        let panel = NSOpenPanel()
        panel.allowedContentTypes = PartFileType.openableTypes
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try load(text: try PartFileReader.read(url: url), from: url)
        } catch {
            await present(error, title: "Couldn't open “\(url.lastPathComponent)”")
        }
    }

    func addToLibraryFromMenu() async {
        do { try addToLibrary() } catch { await present(error, title: "Couldn't add the part") }
    }

    // MARK: Panels

    private func confirmDiscardingChanges() async -> Bool {
        guard isDirty else { return true }
        let alert = NSAlert()
        alert.messageText = "Discard the changes to this part?"
        alert.informativeText = "Your changes will be lost."
        alert.addButton(withTitle: "Discard")
        alert.addButton(withTitle: "Cancel")
        return await run(alert) == .alertFirstButtonReturn
    }

    private func closeAfterAsking() async {
        let alert = NSAlert()
        alert.messageText = "Save the changes to \(title)?"
        alert.informativeText = "Your changes will be lost if you don't save them."
        alert.addButton(withTitle: "Save…")
        alert.addButton(withTitle: "Cancel")
        alert.addButton(withTitle: "Don't Save")
        switch await run(alert) {
        case .alertFirstButtonReturn:
            await save()
            if !isDirty { window?.close() }
        case .alertThirdButtonReturn:
            savedText = draft.fileText()
            window?.close()
        default:
            break
        }
    }

    private func present(_ error: Error, title: String) async {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = title
        alert.informativeText = error.localizedDescription
        _ = await run(alert)
    }

    private func run(_ panel: NSSavePanel) async -> NSApplication.ModalResponse {
        if let window { return await panel.beginSheetModal(for: window) }
        return panel.runModal()
    }

    private func run(_ alert: NSAlert) async -> NSApplication.ModalResponse {
        if let window { return await alert.beginSheetModal(for: window) }
        return alert.runModal()
    }
}
