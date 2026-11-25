# plan-59 findings

Filled in while plan-59-A, -B and -C run. Verdicts and their rules are in
`probes.md`. A cell is `—` until its probe has run. An evidence cell holds one
command with its output, or `evidence/<probe>-<candidate>.png`.

## Baselines

| Baseline | Value | Command |
|---|---|---|
| B1 edit-lag median / p90 | label name 13.8 / 14.3 ms; label type 13.7 / 14.8 ms; bus wire name 16.5 / 18.3 ms; add wire + undo 34.1 / 35.6 ms; pan 2.0 / 16.0 ms (each the median of three runs; Debug app, unoptimized harness; screen locked) | `<scratch>/h-edit/H.app/Contents/MacOS/harness examples/6502.schematical both` ×3, output in `evidence/B1-edit-lag-base.txt` |
| B1 drag-timing median / p90 | moveChip 23.08 / 24.97 ms; dragBusLine 11.97 / 13.35 ms; pan 2.12 / 16.01 ms (median of three runs) | `<scratch>/h-drag/H.app/Contents/MacOS/harness examples/6502.schematical both` ×3, output in `evidence/B1-drag-timing-base.txt` |
| B2 relative `memoryFile.path` for Save As into a second folder | `../A/rom.bin` (load file `…/A/rom.bin`, schematic `…/B/design.schematical`); `rom.bin` when both are in one folder; the absolute path when the schematic has no URL | `scripts/baselines.swift` through `build.sh` on an export of 2d10a029 → `B2 part AT28C64B-15JU memoryFile.path: ../A/rom.bin`, `B2 same-folder memoryFile.path: rom.bin`, `B2 no-URL memoryFile.path: /tmp/…/b2/A/rom.bin` |
| B3 `SchematicFile(document:fileURL:)` main-thread median of 20 | 14.726 ms (min 14.606, max 15.528; 103 parts; unoptimized harness) | same run → `B3 snapshot ms: median 14.726 min 14.606 max 15.528 (n=20, parts 103)` |

## Verdicts

| ID | Probe | G | G evidence | H | H evidence |
|---|---|---|---|---|---|
| F1 | File URL while reading | workaround | API: `ReferenceFileDocumentConfiguration.fileURL`. `init(configuration:)` runs off the main thread with no URL (`model.init(configuration:) main=false … filename=design.schematical`), so the model keeps the decoded `SchematicFile` and applies it when the editor view is built with the URL: `model.attach applied file url=…/fx/A/design.schematical parts=1 catalog=loaded`; probe `mem 0` → `U1 AT28C64B-15JU url=…/fx/A/rom.bin bookmark=none read=256 bytes first=00010203`. The fixture sits in the app's container, the one place a sandboxed app reads a file it has no bookmark for. | — | — |
| F2 | File URL while writing | blocked | `evidence/F2-G.txt`. Neither `snapshot(contentType:)` nor `fileWrapper(snapshot:configuration:)` is told where the file goes, and the view's `fileURL` changes only after the write. Autosave in place: `rom.bin`, correct. Save As from A into B wrote `"path":"rom.bin"` (B2: `../A/rom.bin`); the next autosave rewrote it as `../A/rom.bin`. Tried: marking the document edited when the view reports a new URL (`NSDocument.updateChangeCount(.changeDone)` on `window.windowController.document`), which makes that autosave happen at once (`model.attach marked edited so load-file paths are rewritten`); the file is still wrong between the save and the autosave. Save To (a copy) is written relative to the document's own folder and is never rewritten. Duplicate is written with the original's URL and read back with no URL: without a bookmark the load file is lost (`url=…/spike59/A/rom.bin … read FAILED … no such file`); with an app-scoped bookmark it reads (`bookmark=920 bytes read=256 bytes`). No public API gives the writer the destination. | — | — |
| F3 | Round trip | works | `evidence/F3-G.txt` (`scripts/roundtrip.swift`, harness built from G's sources): `SchematicDocumentModel.load` then `encodedSnapshot()`, decoded and compared → `F3: candidate==original 8 of 10; candidate==base 10 of 10`. The two that differ from their original (`nixie-clock`, `numitron-counter`) differ the same way through the unmodified read and write path (`base==original false`), so the candidate writes what the app writes today for all 10. | — | — |
| F4 | Snapshot cost | workaround | In the app: `snapshot ms: median 13.903 min 13.874 max 17.876 (n=20, parts 103)` (B3: 14.726 ms, same unoptimized build). 30 keystrokes in a net-label name with three autosaves running: `30 keystrokes, 30 accepted … median 4.4 ms p90 13.2 ms max 46.7 ms`; each autosave's snapshot took 14.3–14.4 ms. A net-label name holds 8 characters, so the 30 keystrokes retype it. The workaround: SwiftUI calls `snapshot(contentType:)` on a background queue while the main thread waits in `-[NSDocument _waitForUserInteractionUnblocking]` (`evidence/F4-G-snapshot-thread.txt`; a hop to the main queue deadlocked the app), so the main-actor document is read in place from that thread with its isolation cast away (`unsafeBitCast` of a `@MainActor` closure). The cost still lands on the main thread, which is parked for it. | — | — |
| F5 | Catalog ready on cold open | works | App not running; `open -a SchematicalSpike59.app …/fx/6502.schematical`; log `model.attach applied file url=…/6502.schematical parts=103 catalog=loaded`; `grep -o '"partNumber"' 6502.schematical` → 103. `PartCatalogStore.shared` is loaded in `App.init`, before SwiftUI reads the file. | — | — |
| F6 | Sandbox and other writers | works | `evidence/F6-G.txt`. Saved with an app-scoped bookmark, quit, `rom.bin` moved to another folder, relaunched: `mem 0` → `url=fx/F6/moved/rom.bin bookmark=960 bytes read=256 bytes`. The file changed from the shell while the document had an unsaved edit, then `saveDocument:` → sheet `The document “design.schematical” could not be saved. The file has been changed by another application.` with Save Anyway, Revert and Save As. | — | — |
| F7 | Part files (info) | workaround | API: `Scene.handlesExternalEvents(matching: [])` on the 13 auxiliary scenes. Without it `open -a SchematicalSpike59.app test.schpart` opened no document and an empty `Memory` window (`#494638 AppKitWindow 'Memory' … doc=false`): SwiftUI hands a file the `DocumentGroup` does not read to the first `WindowGroup`. With it: `documents=0`, no window. The part file is not opened anywhere; the migration has to route `.schpart` itself. | — | — |
| U1 | Edit marks the document edited | works | `evidence/U1-U3-G.txt`: one PCB move → `edited=true equalsSavedFile=false undo='Undo Move'`. `NSDocument.isDocumentEdited` is true; `NSWindow.isDocumentEdited` stays false (`window '6502.schematical' … edited=false`), as for every document that autosaves in place: the title bar says Edited, the close button gets no dot. The title bar text and its menu were not seen (screen locked). | — | — |
| U2 | Undo back to clean | works | `evidence/U1-U3-G.txt`: after each of edit, undo, edit + save, edit, undo, redo, undo the document's `edited` flag is the opposite of `equalsSavedFile` (the open document re-encoded and compared with the file on disk). | — | — |
| U3 | One `NSUndoManager` per document | works | `evidence/U1-U3-G.txt`. API: `EnvironmentValues.undoManager` (the same object as `NSDocument.undoManager`: `sameUndoManager=true`), `UndoManager.registerUndo(withTarget:handler:)`, `setActionName`. `SchematicUndoBridge` registers one undo per `UndoStep`; its handler applies the diff and registers the other direction. Schematic, PCB, Schematic edits undone three times and redone three times in strict reverse order, titles `Undo Add Net Label`, `Undo Move`, `Redo Move`. Typing 6 characters: 1 registration, 5 merges, one undo. A 5-frame drag: one step. `menu Edit` → `Undo [⌘z] -> undo:`, `Redo [⇧⌘z] -> redo:` (the system items; `UndoCommands` removed). The menu titles while a window is key were not seen (screen locked). An autosave or save ends a typing run, so typing across an autosave is two steps. | — | — |
| U4 | Text field undo | works | Net-label name field focused (`firstResponder=_SystemTextFieldFieldEditor`), `AB` typed: the field's undo manager is not the document's (`responderUndoManagerIsDocuments=false`, title `Undo Typing`), so typing in a field (the parts search field too) does not mark the document edited by itself. First `undo:` through the responder chain: field text back to `T12345`. Second: the document step is undone (`last=T12345 … redo='Redo Rename Net Label'`). Field closed, `undo:` from the window: the next document step. Driven with `insertText` and `tryToPerform(undo:)`, not key presses (screen locked). | — | — |
| U5 | Simulation lock | workaround | `evidence/U5-G.txt`. API: `UndoManager.registerUndo` inside the handler, then `redo()` on the next main-queue turn. While playing, `recordEdit` records nothing (`registrations` stays 9). `undo:` while playing: nothing changes and the stack is as before (`undo='Undo Add Net Label' redo='Redo'`); log `undo refused while playing: Add Net Label`. The undo manager has already taken the action off its stack when the handler runs, so the handler puts it back. Edit › Undo stays enabled while playing; base shows a plain `Undo` then. | — | — |
| A1 | File menu | — | — | — | — |
| A2 | Commands reach the focused document | — | — | — | — |
| A3 | Exports from the Export menu | — | — | — | — |
| A4 | Print | — | — | — | — |
| A5 | Child windows | — | — | — | — |
| A6 | Lifetime | — | — | — | — |
| A7 | Title bar | — | — | — | — |
| A8 | Tabs | — | — | — | — |
| A9 | Opening paths | — | — | — | — |
| A10 | Restoration | — | — | — | — |
| A11 | Close and quit prompts | — | — | — | — |
| A12 | Versions | — | — | — | — |
| A13 | Automation | — | — | — | — |
| A14 | Performance | — | — | — | — |
| A15 | Other scenes (info) | — | — | — | — |

## A16 — test seams

| Test file | Constructor sites | Controller members used | Replacement seam |
|---|---|---|---|
| — | — | — | — |

## Notes per candidate

Anything a probe taught that the verdict cell cannot hold: API names used by a
workaround, a surprising system behavior, the code size of the candidate.

### G (`DocumentGroup` + `ReferenceFileDocument`)

- Size of the core (plan-59-A): `git diff --stat 2d10a029 | tail -1` → `9 files changed, 873 insertions(+), 44 deletions(-)`. Of the insertions, 421 are the probe driver (`SpikeProbe.swift`), 251 the document model, 103 the undo bridge.
- SwiftUI's document object is an `NSDocument` subclass (`FileWrapperPlatformDocument`), reachable through public API as `window.windowController?.document` and in `NSDocumentController.shared.documents`. Its `undoManager` is the object `EnvironmentValues.undoManager` gives the editor view.
- Threads: `init()` and `init(configuration:)` run off the main thread when a file is opened or restored (on the main thread for File › New and Duplicate). `snapshot(contentType:)` runs on the main thread for Duplicate and on a background queue, with the main thread parked, for Save, Save As and autosave. `fileWrapper(snapshot:configuration:)` runs where the snapshot did.
- At every open SwiftUI first makes a spare document with `init()` and frees it (`model.init() … model.deinit` around each `model.init(configuration:)`): the document's `init()` has to stay cheap and without side effects.
- The document autosaves in place: an edit left unsaved was in the file at the next open.
- File › New is a submenu naming every `WindowGroup` (`New Memory Window`, `New Processor Window`, …) once `SchematicFileCommands` no longer replaces `.newItem` (probe A1 in plan-59-B).
- The Part Editor window uses `WindowCloseGuard` too (`Editor/PartEditorModel.swift:356`); it is not a document in either candidate.
- A step that no longer applies (its items are gone) is dropped by today's `undo(on:)`, which then undoes the next one. On `NSUndoManager` that undo does nothing visible and the user presses ⌘Z again.

## Checks that need the unlocked screen

The screen was locked while the probes ran (plan-59-A Corrections 4). These were not observed; each names what was observed instead.

| Check | Probe | Observed instead |
|---|---|---|
| A cold launch with no file shows an Untitled document | plan-59-A phase 2 | `newDocument:` makes one; a locked launch makes none, on base too |
| The title bar says Edited and its menu offers Save, Rename, Move | U1, A7 | `NSDocument.isDocumentEdited`, the File menu's items and their actions |
| Edit › Undo and Redo show `Undo Move` while the window is key | U3 | `undoManager.undoMenuItemTitle`; the items are the system's `undo:` and `redo:` |
| ⌘Z typed in a focused field | U4 | `insertText` on the field editor and `undo:` sent up the responder chain |
