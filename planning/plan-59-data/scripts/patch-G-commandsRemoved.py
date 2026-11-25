#!/usr/bin/env python3
"""A1: the 13 auxiliary scenes add no File > New item (Scene.commandsRemoved())."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/schematicalApp.swift", "        .handlesExternalEvents(matching: [])\n",
      "        .handlesExternalEvents(matching: [])\n        .commandsRemoved()\n", count=13)
p.done()
