#!/usr/bin/env python3
"""Moves the document wiring (URL, undo manager) out of ContentView's body into a modifier."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
V = "schematical/ContentView.swift"
p.sub(V, """    /// The document's undo manager, from DocumentGroup (plan-59 spike, U3).
    @Environment(\\.undoManager) private var undoManager
""", "")
p.sub(V, """            .onChange(of: undoManager, initial: true) { _, manager in
                model.connectUndo(manager)
            }
            .onChange(of: documentURL) { _, url in
                model.attach(fileURL: url)
            }
""", "")
p.sub("schematical/schematicalApp.swift", """            ContentView(model: configuration.document, fileURL: configuration.fileURL)
""", """            ContentView(model: configuration.document, fileURL: configuration.fileURL)
                .modifier(DocumentWiring(model: configuration.document, fileURL: configuration.fileURL))
""")
p.done()
