#!/usr/bin/env python3
"""Probes for plan-61's design, on the H worktree (after spike-H.patch was saved):
the title shown, a tab setting, an Untitled document at a plain launch, .schpart to the Part Editor scene."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-H/schematical")
D = "schematical/Document/SchematicNSDocument.swift"
p.sub(D, """        window.titleVisibility = .hidden
""", """        window.titleVisibility = .visible
        switch UserDefaults.standard.string(forKey: "documentTabs") ?? "tabs" {
        case "system": window.tabbingMode = .automatic
        case "windows": window.tabbingMode = .disallowed
        default: window.tabbingMode = .preferred
        }
""")
A = "schematical/schematicalApp.swift"
p.sub(A, """        SpikeProbe.install()
    }
""", """        SpikeProbe.install()
        // A plain launch (the open-application event, not open-documents) with
        // nothing restored starts with an Untitled document.
        let event = NSAppleEventManager.shared().currentAppleEvent
        let plain = event?.eventID == AEEventID(kAEOpenApplication)
        SpikeLog.log("didFinishLaunching: event=\\(event.map { String(describing: $0.eventID) } ?? "nil") plain=\\(plain) documents=\\(NSDocumentController.shared.documents.count)")
        DispatchQueue.main.async {
            let count = NSDocumentController.shared.documents.count
            SpikeLog.log("next main-queue turn: documents=\\(count)")
            if plain, count == 0 { NSDocumentController.shared.newDocument(nil) }
        }
    }
""")
p.sub(A, """            if let target {
                PartEditorWindowView(target: target)
            }
        }
        // Wide enough for the Part Wizard's step list, step and preview.
        .defaultSize(width: 1120, height: 760)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
""", """            Group {
                if let target {
                    PartEditorWindowView(target: target)
                } else {
                    Text("Part Editor scene, no target")
                }
            }
            .onOpenURL { url in SpikeLog.log("Part Editor scene onOpenURL \\(url.lastPathComponent)") }
        }
        // Wide enough for the Part Wizard's step list, step and preview.
        .defaultSize(width: 1120, height: 760)
        .restorationBehavior(.disabled)
""")
p.done()
