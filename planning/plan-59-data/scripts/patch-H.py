#!/usr/bin/env python3
"""plan-59-C: candidate H. Run on a detached worktree of 2d10a029 that already has
spike-G-app.patch applied (H copies G's model, undo bridge, registry and probe driver);
this turns the DocumentGroup app into an NSDocument app hosting the same editor."""
import sys, os, shutil, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
S = os.path.dirname(os.path.abspath(__file__))
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-H/schematical")
D = "schematical/Document/"
shutil.copy(f"{S}/overlay-H/SchematicNSDocument.swift", f"{p.root}/{D}SchematicNSDocument.swift")

# ---- own bundle id ------------------------------------------------------------------------
p.sub("schematical.xcodeproj/project.pbxproj", "PRODUCT_BUNDLE_IDENTIFIER = jgz.schematical.spike59;",
      "PRODUCT_BUNDLE_IDENTIFIER = jgz.schematical.spike59h;", count=2)

# ---- Info.plist: the document class ---------------------------------------------------------
p.sub("Info.plist", """\t\t\t<string>Editor</string>
""", """\t\t\t<string>Editor</string>
\t\t\t<key>NSDocumentClass</key>
\t\t\t<string>SchematicNSDocument</string>
""")

# ---- the model: the page, and wiring that needs no scene -------------------------------------
M = D + "SchematicDocumentModel.swift"
text = p.read(M)
cut = text.index("/// Hands the model what only the scene knows")
text = text[:cut] + '''/// What the hosted editor still publishes: the focused document, for Commands.
/// The URL, the undo manager and the child windows come from the NSDocument.
struct DocumentWiring: ViewModifier {
    let model: SchematicDocumentModel
    let fileURL: URL?

    func body(content: Content) -> some View {
        content
            .focusedSceneValue(\\.schematicModel, model)
            .onDisappear {
                SpikeLog.log("editor disappeared: \\(model.id.uuidString.prefix(8))")
            }
    }
}
'''
p.write(M, text)
p.sub(M, """    @MainActor var openChild: ((String, UUID?) -> Void)?
""", """    @MainActor var openChild: ((String, UUID?) -> Void)?
    /// The page the editor shows; the NSDocument stores it in its restorable state.
    @MainActor var page = EditorPage.schematic
""")
p.sub(M, """    @MainActor var platformDocument: NSDocument? {
        window?.windowController?.document as? NSDocument
    }""", """    @MainActor var platformDocument: NSDocument? {
        NSDocumentController.shared.documents.first { ($0 as? SchematicNSDocument)?.model === self }
    }""")

# ---- the probe driver finds the model on the document -----------------------------------------
P = D + "SpikeProbe.swift"
p.sub(P, """        let windows = document.windowControllers.compactMap(\\.window)
        return SchematicDocumentModel.all.first { model in windows.contains { $0 === model.window } }
""", """        (document as? SchematicNSDocument)?.model
""")
p.sub("schematical/Automation/AutomationTools.swift", """            let windows = document.windowControllers.compactMap(\\.window)
            guard let model = SchematicDocumentModel.all.first(where: { model in windows.contains { $0 === model.window } })
            else { return nil }
""", """            guard let model = (document as? SchematicNSDocument)?.model else { return nil }
""")

# ---- ContentView: child windows through the center; the page from the model -------------------
V = "schematical/ContentView.swift"
p.sub(V, "    @Environment(\\.openWindow) private var openWindow\n",
      "    private var openWindow: ChildWindowOpener { ChildWindowOpener(model: model) }\n")
p.sub(V, "    @Environment(\\.dismissWindow) private var dismissWindow\n",
      "    private var dismissWindow: ChildWindowCloser { ChildWindowCloser(model: model) }\n")
p.sub(V, "    @SceneStorage(\"editorPage\") private var page = EditorPage.schematic\n", "    @State private var page: EditorPage\n")
p.sub(V, """        model.attach(fileURL: fileURL)
        let catalog = PartCatalogStore.shared
""", """        model.attach(fileURL: fileURL)
        _page = State(initialValue: model.page)
        let catalog = PartCatalogStore.shared
""")
p.sub(V, """            .onChange(of: page) { _, newPage in
""", """            .onChange(of: page) { _, newPage in
                model.page = newPage
                model.platformDocument?.windowControllers.first?.window?.invalidateRestorableState()
                model.platformDocument?.invalidateRestorableState()
""")

# ---- the App: no document scene, no child-window scenes ----------------------------------------
A = "schematical/schematicalApp.swift"
text = p.read(A)
start = text.index("        DocumentGroup(newDocument:")
while text[:start].rstrip(" ").endswith("\n") and text[:start].rstrip().split("\n")[-1].strip().startswith("//"):
    start = text[:start].rstrip().rindex("\n") + 1
end = text.index("        // One window per datasheet, opened from a part's Open Datasheet button.")
text = text[:start] + text[end:]
p.write(A, text)
p.sub(A, "        .commandsRemoved()\n", "        .commandsRemoved()\n        .defaultLaunchBehavior(.suppressed)\n", count=6)
# The Commands hang on the first remaining scene.
text = p.read(A)
first = text.index("        .defaultLaunchBehavior(.suppressed)\n") + len("        .defaultLaunchBehavior(.suppressed)\n")
text = text[:first] + """        .commands {
            AboutCommands()
            DocumentMenuCommands()
            SchematicFileCommands()
            BillOfMaterialsCommands()
            DesignRuleCheckCommands()
            RouteCommands()
            EditCommands()
            SimulationCommands()
            KeyboardHelpCommands()
        }
""" + text[first:]
p.write(A, text)
p.done()
