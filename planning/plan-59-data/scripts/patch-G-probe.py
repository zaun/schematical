#!/usr/bin/env python3
"""The in-app probe driver: copy the overlay files, install the driver, tell the model its window."""
import sys, os, shutil
sys.path.insert(0, os.path.dirname(__file__))
from patchlib import Patch

S = os.path.dirname(os.path.abspath(__file__))
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
for name in ("SchematicDocumentModel.swift", "SpikeProbe.swift"):
    shutil.copy(f"{S}/overlay-G/Document/{name}", f"{p.root}/schematical/Document/{name}")

app = open(f"{p.root}/schematical/schematicalApp.swift").read()
if "SpikeProbe.install()" not in app:
    p.sub("schematical/schematicalApp.swift", """        OptionOnlyMenuItems.install()
    }
""", """        OptionOnlyMenuItems.install()
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        SpikeProbe.install()
    }
""")
view = open(f"{p.root}/schematical/ContentView.swift").read()
if "model.window = $0" not in view:
    p.sub("schematical/ContentView.swift", "            .background(WindowReader { file.window = $0 })",
          "            .background(WindowReader { file.window = $0; model.window = $0 })")
p.done()
