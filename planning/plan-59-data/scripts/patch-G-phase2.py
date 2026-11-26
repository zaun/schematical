#!/usr/bin/env python3
"""plan-59-A phase 2: the DocumentGroup scene, ContentView(model:fileURL:), Info.plist document types."""
import sys, os, shutil
sys.path.insert(0, os.path.dirname(__file__))
from patchlib import Patch

S = os.path.dirname(os.path.abspath(__file__))
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")

shutil.copy(f"{S}/overlay-G/Document/SchematicDocumentModel.swift",
            f"{p.root}/schematical/Document/SchematicDocumentModel.swift")

# --- Info.plist: document types -------------------------------------------
p.sub("Info.plist", "<dict>\n\t<key>UTExportedTypeDeclarations</key>", """<dict>
\t<key>CFBundleDocumentTypes</key>
\t<array>
\t\t<dict>
\t\t\t<key>CFBundleTypeName</key>
\t\t\t<string>Schematical Schematic</string>
\t\t\t<key>CFBundleTypeRole</key>
\t\t\t<string>Editor</string>
\t\t\t<key>LSHandlerRank</key>
\t\t\t<string>Owner</string>
\t\t\t<key>LSItemContentTypes</key>
\t\t\t<array>
\t\t\t\t<string>jgz.schematical.schematic</string>
\t\t\t</array>
\t\t</dict>
\t\t<dict>
\t\t\t<key>CFBundleTypeName</key>
\t\t\t<string>Schematical Part</string>
\t\t\t<key>CFBundleTypeRole</key>
\t\t\t<string>Viewer</string>
\t\t\t<key>LSHandlerRank</key>
\t\t\t<string>Owner</string>
\t\t\t<key>LSItemContentTypes</key>
\t\t\t<array>
\t\t\t\t<string>jgz.schematical.part</string>
\t\t\t</array>
\t\t</dict>
\t</array>
\t<key>UTExportedTypeDeclarations</key>""")

# --- schematicalApp.swift: the scene ---------------------------------------
p.sub("schematical/schematicalApp.swift", """        WindowGroup(id: "schematic") {
            ContentView()
        }
""", """        DocumentGroup(newDocument: { SchematicDocumentModel() }) { configuration in
            ContentView(model: configuration.document, fileURL: configuration.fileURL)
        }
""")

# --- ContentView.swift ------------------------------------------------------
p.sub("schematical/ContentView.swift", """    init() {
        let catalog = PartCatalogStore.shared
        let document = SchematicDocument(library: catalog.library)
        // A new schematic starts with Power (5V, 3V3, 12V, 24V) and Logic classes
        // (an opened file replaces them with its own).
        document.addStandardNetClasses()
        let simulation = SimulationController(library: catalog.library)
""", """    /// The DocumentGroup document this window edits, and the URL SwiftUI
    /// reports for it (plan-59 spike).
    private let model: SchematicDocumentModel
    private let documentURL: URL?

    init(model: SchematicDocumentModel, fileURL: URL?) {
        self.model = model
        self.documentURL = fileURL
        // The reader gets no URL from SwiftUI; the view has it. The first
        // call applies the file that was read.
        model.attach(fileURL: fileURL)
        let catalog = PartCatalogStore.shared
        let document = model.document
        let simulation = model.simulation
""")
p.sub("schematical/ContentView.swift", """            .onAppear {
                file.openNewWindow = { openWindow(id: "schematic") }
""", """            .onChange(of: documentURL) { _, url in
                model.attach(fileURL: url)
            }
            .onAppear {
                model.attach(fileURL: documentURL)
""")
p.sub("schematical/ContentView.swift", """    ContentView()
""", """    ContentView(model: SchematicDocumentModel(), fileURL: nil)
""")

# --- File menu: New, Open, Save and Save As are the system's ---------------
p.sub("schematical/Document/SchematicFileController.swift", """        CommandGroup(replacing: .newItem) {
            NewProjectMenuItem(file: file)
        }

""", "")
p.sub("schematical/Document/SchematicFileController.swift", """            } else {
                Button("Open…") {
                    Task { await file?.open() }
                }
                .keyboardShortcut("o", modifiers: .command)
                .disabled(file == nil)
            }
""", """            }
""")
p.sub("schematical/Document/SchematicFileController.swift", """            } else {
                Button("Save") {
                    Task { await file?.save() }
                }
                .keyboardShortcut("s", modifiers: .command)
                .disabled(file == nil)
            }

            Button("Save As…") {
                Task { await file?.saveAs() }
            }
            .keyboardShortcut("s", modifiers: [.command, .shift])
            .disabled(file == nil)
""", """            }
""")
p.done()
