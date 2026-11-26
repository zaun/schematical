#!/usr/bin/env python3
"""Fix-up for patch-G-app.py: the closeGuard closure's body was left behind (its first line ends in `in`)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/Document/SchematicFileController.swift", """        guard let self else { return true }
        guard self.hasUnsavedChanges else { return true }
        Task {
            if await self.confirmDiscardingChanges() {
                window.close()
            }
        }
        return false
    }

""", "")
p.done()
