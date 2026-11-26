#!/usr/bin/env python3
"""A10: the page a window shows is scene state the system restores (SceneStorage)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/ContentView.swift", "    @State private var page = EditorPage.schematic\n",
      "    @SceneStorage(\"editorPage\") private var page = EditorPage.schematic\n")
p.done()
