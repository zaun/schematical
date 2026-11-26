#!/usr/bin/env python3
"""plan-59-B on candidate G: commands on the focused model, exports in File > Export, child
windows keyed by the document's id, the hand-built open/save/tab/close/quit code deleted,
automation on NSDocumentController."""
import sys, os, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
S = os.path.dirname(os.path.abspath(__file__))
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
src = "schematical/"
C = src + "Document/SchematicFileController.swift"

# ---- the controller keeps exports and print; open, save, tabs, close and quit go ----------
p.sub(C, "    private(set) var fileURL: URL?\n", "    var fileURL: URL?\n")
p.sub(C, """    weak var window: NSWindow? {
        didSet {
            closeGuard.attach(to: window)
            if oldValue == nil { claimPendingOpen() }
        }
    }
""", """    weak var window: NSWindow?
""")
for signature in ("var openNewWindow: (() -> Void)?", "private struct PendingOpen {",
                  "private static var pendingOpens", "private var claimedOpen: PendingOpen?",
                  "private lazy var closeGuard = WindowCloseGuard",
                  "static func confirmQuit() async -> Bool {", "var hasUnsavedChanges: Bool {",
                  "func save() async {", "func saveAs() async {", "var isBlank: Bool {",
                  "func open() async {", "func open(_ url: URL) async {", "private func claimPendingOpen() {",
                  "static func tabNewProject(focused:", "private static func tabNextWindow(into group: NSWindow) {",
                  "func loadClaimedOpen() {", "private func load(_ file: SchematicFile, from url: URL) {",
                  "func saveWithoutPanel(to url: URL?) -> String? {", "private func write(to url: URL) {",
                  "func confirmDiscardingChanges() async -> Bool {", "struct NewProjectMenuItem: View {",
                  "private var savedSnapshot: SchematicFile"):
    p.delete_block(C, signature)
p.sub(C, "        self.savedSnapshot = SchematicFile(document: document, fileURL: nil)\n", "")
p.sub(C, "    private func offerToAdd(_ parts: [CatalogPart]) async {", "    func offerToAdd(_ parts: [CatalogPart]) async {")
p.sub(C, """    /// Writes the project as `url`, one `.epro` file named for its folder and
""", """    /// File > Export > KiCad Project…: the system save panel, then the three
    /// files beside the chosen one. Like any export, not a save.
    func exportKiCadProject() async {
        guard let url = await chooseExport(.kicad) else { return }
        exportKiCad(to: url)
    }

    /// File > Export > EasyEDA Pro Project…: the system save panel, then the file.
    func exportEasyEDAProProject() async {
        guard let url = await chooseExport(.easyEDAPro) else { return }
        exportEasyEDAPro(to: url)
    }

    private func chooseExport(_ format: SaveFormat) async -> URL? {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [format.contentType]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\\(displayName).\\(format.fileExtension)"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        SpikeLog.log("export panel \\(format.title): accessory=\\(panel.accessoryView == nil ? "none" : "present") name=\\(panel.nameFieldStringValue)")
        guard await run(panel) == .OK else { return nil }
        return panel.url
    }

    /// Writes the project as `url`, one `.epro` file named for its folder and
""")
p.sub(C, """    @FocusedValue(\\.schematicFile) private var file
    @FocusedValue(\\.partEditor) private var editor
""", """    /// The focused window's document (plan-59-B): `.focusedSceneValue` in `DocumentWiring`.
    @FocusedValue(\\.schematicModel) private var model
    @FocusedValue(\\.partEditor) private var editor
    private var file: SchematicFileController? { model?.file }
""")
p.sub(C, """    @Entry var schematicFile: SchematicFileController?
""", """    @Entry var schematicFile: SchematicFileController?
    /// The focused window's document, for the File menu (plan-59-B).
    @Entry var schematicModel: SchematicDocumentModel?
""")

# ---- Export menu ---------------------------------------------------------------------------
p.sub(src + "Document/SchematicExportCommands.swift", """            Button("Gerber PCB…") {
                Task { await file?.exportGerber() }
            }
            .disabled(file == nil)
""", """            Button("Gerber PCB…") {
                Task { await file?.exportGerber() }
            }
            .disabled(file == nil)
            Button("KiCad Project…") {
                Task { await file?.exportKiCadProject() }
            }
            .disabled(file == nil)
            Button("EasyEDA Pro Project…") {
                Task { await file?.exportEasyEDAProProject() }
            }
            .disabled(file == nil)
""")

# ---- ContentView -----------------------------------------------------------------------------
V = src + "ContentView.swift"
p.sub(V, """        _file = State(initialValue: SchematicFileController(
            document: document,
            catalog: catalog,
            willReplaceDocument: { simulation.stop() }
        ))
""", """        _file = State(initialValue: model.file)
""")
p.sub(V, """            .task {
                // A file opened from another window lands here, in a new tab.
                file.loadClaimedOpen()
            }
""", "")
p.sub(V, "                SchematicSessions.shared.register(sessionID, document: document, simulation: simulation, file: file)\n", "")
p.sub(V, "            .navigationTitle(file.displayName)\n", "")
p.sub(V, """    /// Identifies this window to the memory windows it opens.
    @State private var sessionID = UUID()
""", """    /// Identifies this window's document to the child windows it opens.
    private var sessionID: UUID { model.id }
""")

# ---- child windows: the document's id and the weak registry ---------------------------------
files = [V] + [src + f for f in ("Memory/MemoryWindow.swift", "Cpu/CpuWindow.swift", "BillOfMaterials/BillOfMaterialsWindow.swift",
                                 "Terminal/TerminalWindow.swift", "Registers/RegisterWindow.swift",
                                 "BoardPreview/BoardPreviewWindow.swift", "DesignRuleCheck/DesignRuleCheckWindow.swift")]
p.delete_block(src + "Memory/MemoryWindow.swift", "final class SchematicSessions {")
n = p.rename(files, "sessionID", "documentID")
print("sessionID -> documentID:", n, "places")
for f in files[1:]:
    p.sub(f, "SchematicSessions.shared.session(target.documentID)", "SchematicDocumentModel.session(target.documentID)")

# ---- quit: the system asks ---------------------------------------------------------------------
p.delete_block(src + "schematicalApp.swift", "func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {")

# ---- automation ---------------------------------------------------------------------------------
T = src + "Automation/AutomationTools.swift"
p.sub(T, """    let file: SchematicFileController?

""", """    let file: SchematicFileController?
    /// The DocumentGroup document behind the window (plan-59-B).
    var model: SchematicDocumentModel?

""")
p.sub(T, """        SchematicSessions.shared.identifiedSessions.map {
            AutomationContext(id: $0.id, document: $0.document, simulation: $0.simulation, file: $0.file)
        }
""", """        // The open documents, from NSDocumentController, each mapped to its
        // model through its window (public API only).
        NSDocumentController.shared.documents.compactMap { document in
            let windows = document.windowControllers.compactMap(\\.window)
            guard let model = SchematicDocumentModel.all.first(where: { model in windows.contains { $0 === model.window } })
            else { return nil }
            return AutomationContext(id: model.id, document: model.document, simulation: model.simulation,
                                     file: model.file, model: model)
        }
""")
A = src + "Automation/AutomationSession.swift"
p.sub(A, """                        + (window.file?.hasUnsavedChanges == true ? "  unsaved" : "")
""", """                        + (window.model?.platformDocument?.isDocumentEdited == true ? "  unsaved" : "")
""")
p.sub(A, """                guard let file = context?.file else { throw AutomationFailure("no window to open a tab from") }
                let known = Set(sessions().map(\\.id))
                SchematicFileController.tabNewProject(focused: file)
                file.openNewWindow?()
""", """                let known = Set(sessions().map(\\.id))
                NSDocumentController.shared.newDocument(nil)
""")
p.sub(A, """                guard let file = context?.file else { throw AutomationFailure("no window to open it from") }
                do { _ = try SchematicFile.decode(try Data(contentsOf: url)) }
                catch { throw AutomationFailure("can't open \\(url.lastPathComponent): \\(error.localizedDescription)") }
                let known = Set(sessions().map(\\.id))
                await file.open(url)
""", """                do { _ = try SchematicFile.decode(try Data(contentsOf: url)) }
                catch { throw AutomationFailure("can't open \\(url.lastPathComponent): \\(error.localizedDescription)") }
                let known = Set(sessions().map(\\.id))
                do { _ = try await NSDocumentController.shared.openDocument(withContentsOf: url, display: true) }
                catch { throw AutomationFailure("can't open \\(url.lastPathComponent): \\(error.localizedDescription)") }
""")
p.sub(A, """                guard let file = context?.file else { throw AutomationFailure("no file controller for this window") }
                if let problem = file.saveWithoutPanel(to: url) { throw AutomationFailure(problem) }
                return Out.text("saved to \\(file.fileURL?.path ?? "?")")
""", """                guard let document = context?.model?.platformDocument, let type = document.fileType else {
                    throw AutomationFailure("no document for this window")
                }
                guard let target = url ?? document.fileURL else {
                    throw AutomationFailure("This window has no file yet; pass a path.")
                }
                let failure: Error? = await withCheckedContinuation { continuation in
                    document.save(to: target, ofType: type, for: url == nil ? .saveOperation : .saveAsOperation) {
                        continuation.resume(returning: $0)
                    }
                }
                if let failure {
                    throw AutomationFailure("Couldn't save “\\(target.lastPathComponent)”: \\(failure.localizedDescription)")
                }
                return Out.text("saved to \\(target.path)")
""")

for name in ("SchematicDocumentModel.swift", "SpikeProbe.swift", "SchematicUndoBridge.swift"):
    shutil.copy(f"{S}/overlay-G/Document/{name}", f"{p.root}/schematical/Document/{name}")
p.done()
