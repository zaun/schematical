#!/usr/bin/env python3
"""F7/A9: the 13 auxiliary scenes take no external (open-file) events, so a file the
DocumentGroup does not read cannot land in the first WindowGroup."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/schematicalApp.swift", "        .restorationBehavior(.disabled)\n",
      "        .restorationBehavior(.disabled)\n        .handlesExternalEvents(matching: [])\n", count=13)
p.done()
