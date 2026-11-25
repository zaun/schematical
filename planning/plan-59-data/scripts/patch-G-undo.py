#!/usr/bin/env python3
"""plan-59-A phase 4: the NSUndoManager bridge (probe U3)."""
import sys, os, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
S = os.path.dirname(os.path.abspath(__file__))
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
for name in ("SchematicDocumentModel.swift", "SpikeProbe.swift", "SchematicUndoBridge.swift"):
    shutil.copy(f"{S}/overlay-G/Document/{name}", f"{p.root}/schematical/Document/{name}")
M = "schematical/Canvas/SchematicModel.swift"
if "var onStep" not in open(f"{p.root}/{M}").read():
    p.sub(M, """    private var history = UndoHistory()
""", """    private var history = UndoHistory()
    /// plan-59 spike: when set, each recorded step goes here (to the
    /// document's NSUndoManager) and not onto the per-page lists.
    @ObservationIgnored var onStep: ((UndoStep, EditorPage) -> Void)?
    @ObservationIgnored var onEndCoalescing: (() -> Void)?
    @ObservationIgnored var onHistoryCleared: (() -> Void)?
""")
    p.sub(M, """        history.push(UndoStep(name: recordingName ?? "Edit", coalescing: outerCoalescing ?? recordingCoalescing,
                              changes: changes, pages: pages))
""", """        let step = UndoStep(name: recordingName ?? "Edit", coalescing: outerCoalescing ?? recordingCoalescing,
                            changes: changes, pages: pages)
        if let onStep { onStep(step, recordingPage) } else { history.push(step) }
""")
    p.sub(M, """    func endCoalescing() {
        history.endCoalescing()
    }
""", """    func endCoalescing() {
        history.endCoalescing()
        onEndCoalescing?()
    }

    /// Applies one step's diff, backwards (undo) or forwards (redo), as
    /// `undo(on:)` and `redo(on:)` do. False when it changes nothing any
    /// more, or while playing. For the NSUndoManager bridge.
    func applyStep(_ step: UndoStep, backwards: Bool) -> Bool {
        if isGestureOpen { endGesture() }
        guard !isPlaying else { return false }
        var s = state
        guard UndoDiff.apply(step.changes, backwards: backwards, to: &s) else { return false }
        setState(s)
        return true
    }
""")
    p.sub(M, "        history.clear()\n", "        history.clear()\n        onHistoryCleared?()\n", count=2)
    V = "schematical/ContentView.swift"
    p.sub(V, """    @Environment(\\.openWindow) private var openWindow
""", """    @Environment(\\.openWindow) private var openWindow
    /// The document's undo manager, from DocumentGroup (plan-59 spike, U3).
    @Environment(\\.undoManager) private var undoManager
""")
    p.sub(V, """            .onChange(of: documentURL) { _, url in
""", """            .onChange(of: undoManager, initial: true) { _, manager in
                model.connectUndo(manager)
            }
            .onChange(of: documentURL) { _, url in
""")
    p.sub("schematical/schematicalApp.swift", "            UndoCommands()\n", "")
p.done()
