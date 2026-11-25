#!/usr/bin/env python3
"""A6: Revert makes a new document object; the editor view takes its identity so its state is rebuilt."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical/schematicalApp.swift", """                .modifier(DocumentWiring(model: configuration.document, fileURL: configuration.fileURL))
""", """                .modifier(DocumentWiring(model: configuration.document, fileURL: configuration.fileURL))
                // Revert To makes a new document object: without this the
                // window keeps the reverted-away document in its @State.
                .id(ObjectIdentifier(configuration.document))
""")
p.done()
