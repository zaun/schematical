#!/usr/bin/env python3
"""A3: a KiCad project is three files; the sandbox grants only the one file a save panel
names, so the export asks for the folder (NSOpenPanel, directories) and writes all three in it."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/Document/SchematicFileController.swift", """        guard let url = await chooseExport(.kicad) else { return }
        exportKiCad(to: url)
""", """        let panel = NSOpenPanel()
        panel.canChooseDirectories = true
        panel.canChooseFiles = false
        panel.canCreateDirectories = true
        panel.prompt = "Export"
        panel.message = "Choose the folder for \\(displayName).kicad_pro, .kicad_sch and .kicad_pcb."
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let folder = panel.url else { return }
        let scoped = folder.startAccessingSecurityScopedResource()
        defer { if scoped { folder.stopAccessingSecurityScopedResource() } }
        exportKiCad(to: folder.appendingPathComponent("\\(displayName).kicad_pro"))
""")
p.done()
