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
            guard draft != oldValue else { return }
            if !derivingKeys {
                // A key the user changed stops following the part number.
                if draft.deviceID != oldValue.deviceID { autoKeys.remove(.deviceID) }
                if draft.function != oldValue.function { autoKeys.remove(.function) }
                if draft.drawer != oldValue.drawer { autoKeys.remove(.drawer) }
                derivingKeys = true
                let derived = Self.applyingAutoKeys(autoKeys, to: draft)
                if derived != draft { draft = derived }
                derivingKeys = false
            }
            scheduleValidation()
            keepCurrentStep(after: oldValue)
        }
    }

    // MARK: The wizard's first steps (plan-46-F)

    /// Whether the part's kind has been chosen. False only in a File › New
    /// Part… window until "What are you making?" is answered; the draft is a
    /// `generic` placeholder until then.
    private(set) var kindChosen = true

    /// The internal keys the wizard fills in from the part number and the
    /// kind while the user has not edited them (More options).
    enum AutoKey: String, CaseIterable, Sendable { case deviceID, function, drawer }

    /// The keys still derived. Window state: a copy or an opened file has none.
    private(set) var autoKeys: Set<AutoKey> = []
    @ObservationIgnored private var derivingKeys = false

    /// The device id a part number gives: upper case, only `A-Z 0-9 . -`
    /// ("74hc00-x/y" → "74HC00-XY"); "custom" while there is no part number.
    static func deviceID(forName name: String) -> String {
        let allowed = Set("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-")
        let id = String(name.uppercased().filter { allowed.contains($0) })
        return id.isEmpty ? "custom" : id
    }

    /// The draft with its derived keys brought up to date.
    static func applyingAutoKeys(_ keys: Set<AutoKey>, to draft: PartDraft) -> PartDraft {
        guard !keys.isEmpty else { return draft }
        var draft = draft
        let info = PartKinds.info[draft.kind]
        if keys.contains(.deviceID) { draft.deviceID = deviceID(forName: draft.name) }
        if keys.contains(.function), let info { draft.function = info.name }
        if keys.contains(.drawer), let info { draft.drawer = ["Custom", info.group.rawValue] }
        return draft
    }

    /// Every manufacturer a library part names, for the part-number step's
    /// menu ("Generic" is offered as "My own design / unknown").
    @ObservationIgnored private(set) lazy var manufacturers: [String] = {
        Set(catalog.allParts.map(\.manufacturer)).subtracting(["", "Generic"]).sorted()
    }()

    /// Whether the window makes a new part (File › New Part…), not a copy or
    /// a file: All fields then shows the kind with Change….
    private(set) var isNewPart = false

    /// All fields' Change…: back to "What are you making?" in Guided mode.
    func changeKind() {
        mode = .guided
        currentStepID = WizardStep.kind.id
    }

    /// Whether choosing `kind` would clear settings the user entered: the
    /// kind's own keys differ from a new part's.
    func kindChangeClearsSettings(to kind: String) -> Bool {
        guard kindChosen, kind != draft.kind else { return false }
        let blank = PartDraft(newOf: draft.kind)
        return draft.kindDevice != blank.kindDevice || draft.kindPart != blank.kindPart
            || !draft.deviceExtras.isEmpty || !draft.partExtras.isEmpty
    }

    /// Answers "What are you making?": the part becomes that kind, keeping
    /// its name, datasheet, pins and footprint, and the wizard moves to the
    /// part number. The view asks first when `kindChangeClearsSettings`.
    func chooseKind(_ kind: String) {
        guard PartKinds.schemas[kind] != nil else { return }
        if !kindChosen || kind != draft.kind {
            derivingKeys = true
            draft = Self.applyingAutoKeys(autoKeys, to: draft.changingKind(to: kind))
            derivingKeys = false
        }
        kindChosen = true
        currentStepID = WizardStep.identity.id
    }

    /// "Start from a similar part": the draft becomes a copy of a library
    /// part (a new uuid, as Copy to Editor… makes), named as the library
    /// would name it so it never collides, and the wizard moves to the part
    /// number with everything filled in. False when the part cannot be copied.
    @discardableResult
    func chooseSimilar(named name: String) -> Bool {
        guard var copy = Self.copy(named: name, catalog: catalog) else { return false }
        autoKeys = []
        var n = 2
        while catalog.part(named: "\(copy.name).\(n)") != nil { n += 1 }
        copy.name = "\(copy.name).\(n)"
        draft = copy
        kindChosen = true
        currentStepID = WizardStep.identity.id
        return true
    }

    /// The first 20 library parts matching a search, for "Start from a
    /// similar part" (the drawer's own search and order; obsolete parts
    /// hidden as the drawer hides them).
    func similarParts(matching query: String) -> [CatalogPart] {
        let text = query.trimmingCharacters(in: .whitespaces)
        guard !text.isEmpty else { return [] }
        var filter = DrawerFilter()
        filter.query = text
        return Array(catalog.groups(matching: filter).flatMap(\.allParts).prefix(20))
    }

    // MARK: Guided | All fields (plan-46-E)

    /// How the window shows the draft: the Part Wizard's steps, or the full
    /// form. Window state: never written to the file.
    enum EditorMode: String, CaseIterable, Sendable {
        case guided, allFields

        var title: String {
            switch self {
            case .guided: "Guided"
            case .allFields: "All fields"
            }
        }
    }

    /// Guided for a new part; All fields for a copy or an opened file.
    var mode: EditorMode = .allFields
    /// The wizard step shown in Guided mode (a `WizardStep.id`).
    private(set) var currentStepID: String = WizardStep.identity.id

    /// The wizard's steps for the draft as it is now.
    var steps: [WizardStep] { Self.steps(for: draft, kindChosen: kindChosen) }

    /// The steps the window lists. Until the kind is chosen the draft is a
    /// placeholder, so its kind steps are not listed.
    static func steps(for draft: PartDraft, kindChosen: Bool = true) -> [WizardStep] {
        let steps = WizardPlan.steps(for: draft)
        if kindChosen { return steps }
        return steps.filter { [.kind, .identity, .datasheet, .pinout, .footprint, .review].contains($0.page) }
    }

    /// What a step still needs: the draft's issues, and for the kind step,
    /// the kind itself until it is chosen.
    func issues(of step: WizardStep) -> [String] {
        if step.page == .kind { return kindChosen ? [] : ["Choose what you are making"] }
        return step.issues(in: draft)
    }

    var currentStep: WizardStep? {
        let steps = steps
        return steps.first { $0.id == currentStepID } ?? steps.first
    }

    var currentStepIndex: Int {
        steps.firstIndex { $0.id == currentStepID } ?? 0
    }

    var canGoBack: Bool { currentStepIndex > 0 }
    var canGoNext: Bool { currentStepIndex < steps.count - 1 }

    /// Next is never refused: a step's issues show, they do not trap the user.
    func goNext() {
        let steps = steps
        let index = currentStepIndex
        if index < steps.count - 1 { currentStepID = steps[index + 1].id }
    }

    func goBack() {
        let steps = steps
        let index = currentStepIndex
        if index > 0 { currentStepID = steps[index - 1].id }
    }

    func go(to id: String) {
        if steps.contains(where: { $0.id == id }) { currentStepID = id }
    }

    /// When an edit removes the current step (another model was chosen, so
    /// its keys' steps are gone), the current step becomes the first one
    /// after the last earlier step that still exists.
    private func keepCurrentStep(after old: PartDraft) {
        guard old.kind != draft.kind || old.variant != draft.variant
            || WizardPlan.hasFootprint(old) != WizardPlan.hasFootprint(draft) else { return }
        let now = Self.steps(for: draft, kindChosen: kindChosen)
        guard !now.contains(where: { $0.id == currentStepID }) else { return }
        let before = Self.steps(for: old, kindChosen: kindChosen)
        let index = before.firstIndex { $0.id == currentStepID } ?? 0
        let ids = now.map(\.id)
        for earlier in before[..<index].reversed() {
            if let kept = ids.firstIndex(of: earlier.id) {
                currentStepID = now[min(kept + 1, now.count - 1)].id
                return
            }
        }
        currentStepID = now.first?.id ?? WizardStep.identity.id
    }

    /// The file the draft was last saved to or opened from.
    private(set) var fileURL: URL?
    /// The draft's text when it was last saved or opened, or as the window
    /// opened it (a new part or a copy): what `isDirty` compares with.
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
        case .wizard:
            draft = PartDraft(newOf: "generic")
            mode = .guided
            kindChosen = false
            isNewPart = true
            autoKeys = Set(AutoKey.allCases)
        case let .new(kind):
            draft = PartDraft(newOf: kind)
            mode = .guided
            isNewPart = true
            autoKeys = Set(AutoKey.allCases)
        case let .copy(name):
            mode = .allFields
            if let draft = Self.copy(named: name, catalog: catalog) {
                self.draft = draft
            } else {
                draft = PartDraft(newOf: "gate")
                openError = "There is no part named \(name) to copy."
            }
        case let .file(url):
            mode = .allFields
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
        if !autoKeys.isEmpty {
            derivingKeys = true
            draft = Self.applyingAutoKeys(autoKeys, to: draft)
            derivingKeys = false
        }
        // A new part of a known kind starts on the part number; the wizard
        // (no kind yet) on "What are you making?".
        currentStepID = kindChosen ? WizardStep.identity.id : WizardStep.kind.id
        // A new part or a copy is unchanged until it is edited.
        if savedText == nil { savedText = draft.fileText() }
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
        draft.fileText() != savedText
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
        if !name.isEmpty { return name }
        return kindChosen ? "New \(PartDraft.displayName(of: draft.kind)) Part" : "New Part"
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
