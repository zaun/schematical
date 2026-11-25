# plan-61-B: The `NSDocument` and its hosted window

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-61-A (prerequisites are stated there, § Prerequisites)

Replaces the schematic `WindowGroup` with an `NSDocument` subclass whose
window hosts `ContentView` in an `NSHostingController`. After this sub-plan
AppKit owns reading, writing, autosave in place, versions, recents, the save
panel, the close and quit sheets and tabs; the File menu has the standard
document items; the seven child windows are AppKit windows that close with
their document. The single outcome: a `.schematical` file opened, edited,
saved, duplicated, reverted and closed entirely through `NSDocument`, with a
test for each.

References:

- `planning/plan-59-data/findings.md` (H columns of F1–F7, A1, A2, A5, A6,
  A11, A12; § Notes per candidate, H).
- `planning/plan-59-data/spike-H.patch`: `SchematicNSDocument.swift` in it is
  the reference for this sub-plan (document, `ChildWindowCenter`,
  `DocumentMenuCommands`).
- `schematical/schematical/Document/SchematicFileController.swift`,
  `schematicalApp.swift`, `ContentView.swift`, `Memory/MemoryWindow.swift`
  (`SchematicSessions`).
- `docs/file-format.md`.

## Prerequisites

Stated in plan-61-A. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-61-A is complete | `rg -c '^- \[ \]' planning/plan-61-A-document-undo-manager.md` → no output | MET (no output, 2026-11-24, after 304b545d) |
| The print entitlement is in the project | `rg -c 'ENABLE_RESOURCE_ACCESS_PRINTING = YES' schematical/schematical.xcodeproj/project.pbxproj` → `2` | NOT MET in the committed tree: `0` in the worktree at ba3d7e4d+ (2026-11-24). `2` only in the main checkout, where `project.pbxproj` has the owner's uncommitted change that adds it (Corrections 1) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run before you continue and before you stop; report all prerequisites if
> you stop.

## 1. Goal

- `SchematicNSDocument: NSDocument` owns a `SchematicEditorModel` (the
  `SchematicDocument`, its `SimulationController`, the exporter) and hands
  `SchematicDocument.undoManager` to AppKit.
- `Info.plist` declares the schematic document type with that class;
  `open -a Schematical file.schematical`, a Finder double-click and a Dock
  drop open it.
- Documents autosave in place (owner's answer). Load-file paths are written
  relative to the destination of every write (F2).
- File menu: New, Open…, Open Recent, Close, Close All, Save, Duplicate,
  Save As… (Option), Rename…, Move To…, Revert To ▸.
- The seven child windows (Memory, Processor, Registers, Terminal, Bill of
  Materials, Design Rule Check, Board Preview) open from their document and
  close with it.
- Closing a document stops its simulation and frees its model,
  `SchematicDocument` and `SimulationController` (owner's must-fix, A6).

### Non-goals (explicit constraints)

- The `.schematical` format does not change (F3: 10 of 10 examples equal to
  what the app writes today).
- Exports, print, the title bar, the tab setting, the launch window, part
  files and restoration of the page are 61-C. Automation is 61-D. Until then
  the exporter is today's `SchematicFileController`, reduced, reached through
  the model.
- No window delegate proxy, swizzling, private API or window-notification
  patching (owner's rule, plan-59 `probes.md` § Verdicts). The schematic
  window's `WindowCloseGuard` use and `tabNextWindow` are deleted, not ported.
- The Part Editor window keeps `WindowCloseGuard` (`Editor/PartEditorModel.swift:356`);
  it is not a document in this plan.

## 2. Current State

`schematicalApp.swift:48` declares `WindowGroup(id: "schematic") { ContentView() }`
with the app's nine `Commands`. `ContentView.init` (`ContentView.swift:98`)
builds the `SchematicDocument`, the `SimulationController` and a
`SchematicFileController` (`@State`, `:55`), which does Save, Save As, Open,
the dirty compare, the close guard (`WindowCloseGuard`), quit
(`confirmQuit`, called from `AppDelegate.applicationShouldTerminate`,
`schematicalApp.swift:26`) and tabs (`tabNextWindow`). Child windows are 7 of
the 14 scene declarations, each a `WindowGroup(for: Target.self)` keyed by
`sessionID` (`ContentView.swift:57`) and resolved through `SchematicSessions`
(`Memory/MemoryWindow.swift`), registered at `ContentView.swift:767`.

What plan-59 established for the hosted `NSDocument` (H):

- `read(from:ofType:)` and `writeSafely(to:ofType:for:)` carry the URL, so
  relative load-file paths resolve and are written against the destination
  (`evidence/F2-H.txt`: Save As into another folder wrote `../A/rom.bin` at once).
- A file in AppKit's autosave folder is treated as having no place: paths are
  written absolute, as for today's unsaved schematic; a Duplicate then reads
  its load file.
- AppKit calls the read and write overrides on the main thread (`H read main=true`,
  `H snapshot main=true`); they are nonisolated in the SDK and enter the main
  actor with `MainActor.assumeIsolated`.
- `.focusedSceneValue` from the hosted editor reaches `Commands` (A2).
- `.toolbar` content reaches the window with
  `NSHostingController.sceneBridgingOptions = [.toolbars]` (`evidence/A7-H.png`).
- SwiftUI adds no document items to the File menu without a `DocumentGroup`;
  H declares them (A1).
- Revert reloads the same object and stops the simulation (A6).
- A closed document's model stays alive (A6, `blocked`): a trace named
  SwiftUI's focused-value storage and a captured view closure; emptying the
  hosted view and withdrawing one focused value did not free it.

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines in the controller | 844 | `wc -l schematical/schematical/Document/SchematicFileController.swift` → 844 |
| Files outside it that name it | 8 | `rg -c 'SchematicFileController' schematical/schematical --glob '!Document/SchematicFileController.swift' \| wc -l` → 8 |
| Scene declarations | 14, 13 with `.restorationBehavior(.disabled)` | `rg -n 'WindowGroup\(\|^\s*Window\(' schematical/schematical/schematicalApp.swift \| wc -l` → 14; `rg -c 'restorationBehavior\(.disabled\)' …` → 13 |
| `sessionID` lines | 27 in 8 files | `rg -c 'sessionID' schematical/schematical` → 8 files, sum 27 |
| `focusedSceneValue` calls in `ContentView` | 8 | `rg -c 'focusedSceneValue' schematical/schematical/ContentView.swift` → 8 |
| Test constructor sites of the controller | 9 in 8 files | `rg -c 'SchematicFileController\(document:' schematical/schematicalTests` → 8 files, sum 9 |

### Verified properties

Every row of § 2's list above is a plan-59 probe with its evidence file in
`planning/plan-59-data/evidence/`. In addition:

| Claim | How verified |
|---|---|
| `NSDocument.undoManager` is settable and the document's window returns it | `scripts/probe-undo-lock.swift`: `window.undoManager is the document's: true` |
| With the manager's `canUndo` false while playing, the window's Undo item is disabled | the same probe: `locked:   validate = false title: Undo` |
| The auxiliary scenes need `.defaultLaunchBehavior(.suppressed)`, `.handlesExternalEvents(matching: [])` and `.commandsRemoved()` or SwiftUI opens one at launch, sends it stray open events, and lists it under File › New | plan-59 F7 and A1 (G) and `findings.md` § H notes |
| Deployment target 15.8 has `defaultLaunchBehavior` (macOS 15) | `rg -n MACOSX_DEPLOYMENT_TARGET project.pbxproj` → `15.8`; the H build compiled with it |

## 3. Design Overview

Pieces, in landing order:

1. **`SchematicEditorModel`** (`@MainActor`, `@Observable`, new file): what
   `ContentView.init` builds today (`SchematicDocument` with
   `addStandardNetClasses()`, `SimulationController`), plus `fileURL`, `page`
   and the reduced exporter. It is the spike's `SchematicDocumentModel`
   without its `ReferenceFileDocument` half. `ContentView.init(model:)`.
2. **`SchematicNSDocument`**: `autosavesInPlace` true; `read(from:ofType:)`
   decodes and applies with the URL; `writeSafely` records the destination;
   `data(ofType:)` calls `document.willSave()` and encodes
   `SchematicFile(document:fileURL:)` against it; `makeWindowControllers()`
   hosts the editor; `close()` stops the simulation and closes the child
   windows; its `undoManager` is `model.document.undoManager`.
3. **`DocumentMenuCommands`**: the File menu's document items, each a SwiftUI
   `Button` sending the standard action to the responder chain. `UndoCommands`
   is deleted: the window's manager is the document's, so the system's Undo
   and Redo work and are disabled while playing by the manager (61-A).
4. **`ChildWindowCenter`**: the seven children as `NSWindow`s hosting their
   existing views, keyed by the model's `id` and the target; `ContentView`
   reaches it through two stand-ins with `openWindow`'s and `dismissWindow`'s
   call syntax, so its call sites do not change. The targets' `sessionID`
   becomes `documentID`; `SchematicSessions` is replaced by a lookup of the
   open documents.
5. **Lifetime.** The last phase: a test that a closed document's objects are
   freed, and the fix.

Where the risk concentrates: piece 5. The owner's decision is that it is a
must-fix with a test: the phase's acceptance is the test passing. What is
known going in is in § 2; the phase's tasks start from the trace (the
holders are the eight `focusedSceneValue` calls and the edit-key monitor's
closure in `ContentView`), not from a guess.

Byte-identity is not a gate; F3's decoded-value comparison is.

Rejected alternatives: `DocumentGroup` with a `ReferenceFileDocument`
(plan-59's candidate G: three `blocked` rows, F2 among them, and a snapshot
read off the main actor); keeping the child windows as scenes (a hosted
editor has no scene to open them from; not probed further once the AppKit
windows worked); a full AppKit app life cycle with nibs (the other 13 scenes
and the `Commands` are SwiftUI).

## 4. Detailed Design

- `SchematicNSDocument` is `@objc(SchematicNSDocument)`; `Info.plist` gains
  `CFBundleDocumentTypes` with `LSItemContentTypes = [jgz.schematical.schematic]`,
  role Editor, `NSDocumentClass = SchematicNSDocument`.
- The autosave rule: `url.path.contains("/Autosave Information/")` → relative
  base nil, for reads and writes.
- `SchematicEditorModel.platformDocument`: the `NSDocument` whose `model` it
  is, found in `NSDocumentController.shared.documents`.
- Child lookup: `SchematicEditorModel.session(_ id: UUID)` walks the open
  documents; no registry object is kept.
- The custom-parts offer made after an open (`UserLibraryPrompts.offerable`,
  today in the controller's `load`) moves to the document's `read`.
- `SchematicFileController` loses `open`, `save`, `saveAs`, `write`,
  `saveWithoutPanel`, the pending-open and tab code, `closeGuard`,
  `confirmQuit`, `confirmDiscardingChanges`, `hasUnsavedChanges`, `isBlank`,
  `savedSnapshot` and `NewProjectMenuItem`; `fileURL` becomes settable and is
  set by the document. Automation's three uses of the deleted members
  (`AutomationSession.swift:56–96`) are given the `NSDocumentController` calls
  from `spike-H.patch` in the same commit so the build stays whole; 61-D owns
  their tests and docs.

## Compatibility / Format Impact

Files: none. Behavior the user sees change here: saving is automatic; the
close and quit prompts are the system's and appear only for an untitled
document; File › Save As… is the Option alternate of Duplicate; New and Open
follow the system tab setting until 61-C adds the app's own setting.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The model object, with today's window

- [x] New `Document/SchematicEditorModel.swift` as in § 3 piece 1; `ContentView.init(model:)` takes it; `schematicalApp.swift` builds one per window (`ContentView(model: SchematicEditorModel())` behind a small wrapper view holding it in `@State`). Done: `ContentView` reads the document, simulation and exporter through the model (no `@State` copies); `SchematicWindowRoot` in `schematicalApp.swift` holds the model.
- [x] The 9 test constructor sites keep compiling: the controller's initializer is unchanged in this phase. Done: the test target built and ran.

Acceptance: the app behaves as before with the model in between.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/SchematicExportActionTests -only-testing:schematicalTests/FileDesignRulesTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min). Result: `Executed 21 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: —

### Phase 2 — The document class, read and write

- [ ] New `Document/SchematicNSDocument.swift`: the class with `read`, `writeSafely`, `data(ofType:)`, the autosave rule, `fileURL` forwarding to the model, `undoManager` from the model's document. No window yet.
- [ ] New `schematicalTests/SchematicNSDocumentTests.swift`: (1) for each of the 10 examples, `read(from:ofType:)` then `data(ofType:)` decodes equal to what `SchematicFile.apply` + `SchematicFile(document:fileURL:)` writes; (2) a design with a load file in `…/A` written with `writeSafely(to: …/B/design.schematical)` stores `../A/rom.bin`, and with a destination under `Autosave Information` stores the absolute path; (3) reading from an autosave location resolves an absolute path; (4) an edit sets `isDocumentEdited`, undo clears it, `data(ofType:)` between two typed characters gives two undos.

Acceptance: the four tests pass.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — The hosted window, the menu, the scene removed

- [ ] `SchematicNSDocument.makeWindowControllers()` (hosting controller, `sceneBridgingOptions = [.toolbars]`, `toolbarStyle = .unifiedCompact`) and `close()`.
- [ ] `Info.plist`: `CFBundleDocumentTypes` for the schematic type. `schematicalApp.swift`: delete the `WindowGroup(id: "schematic")` scene and `applicationShouldTerminate`; move `.commands` to the first remaining scene; give the remaining scenes `.handlesExternalEvents(matching: [])`, `.commandsRemoved()` and `.defaultLaunchBehavior(.suppressed)` (the Part Editor scene's external events are 61-C's).
- [ ] New `DocumentMenuCommands` (in `SchematicNSDocument.swift`); `SchematicFileCommands` loses New Project, Open…, Save and Save As…; delete `UndoCommands` and its `.focusedSceneValue`.
- [ ] `SchematicFileController`: the deletions of § 4; `ContentView`: remove `file.loadClaimedOpen()`, `.navigationTitle(file.displayName)`; `AutomationSession.swift`: the three `NSDocumentController` calls.
- [ ] `schematicalTests`: the 9 constructor sites follow `findings.md` § A16 (H column).

Acceptance: a file opened from the shell shows in a document window, and the menu has the document items.
  Check: build, then `open -a <Debug app> examples/6502.schematical; osascript -e 'tell application "System Events" to tell process "schematical" to get {name of window 1, name of every menu item of menu "File" of menu bar 1}'` → `6502.schematical` and a list holding `Save`, `Duplicate`, `Rename…`, `Move To…`, `Revert To` (est. 8 min with the build; run with the owner's installed app closed: both have the bundle id `jgz.schematical`).
Commit: —

### Phase 4 — Child windows

- [ ] `ChildWindowCenter`, `ChildWindowOpener`, `ChildWindowCloser` (from `spike-H.patch`); `ContentView`'s `openWindow` and `dismissWindow` become the stand-ins.
- [ ] `sessionID` → `documentID` in the 8 files (27 lines); delete `SchematicSessions`; the seven views call `SchematicEditorModel.session`; delete the 7 child scenes from `schematicalApp.swift`.
- [ ] `SchematicNSDocumentTests`: opening a Bill of Materials child for two documents gives two windows; closing one document closes only its own.

Acceptance: the new test passes and no `sessionID` is left.
  Check: `rg -c 'sessionID|SchematicSessions' schematical/schematical` → no output; `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 5 — A closed document is freed (owner's must-fix)

- [ ] `SchematicNSDocumentTests.testClosingFreesTheDocument`: make a document, `makeWindowControllers()`, show the window, run the simulation, open a Memory child, `close()`, spin the run loop for 2 s, assert weak references to the model, the `SchematicDocument` and the `SimulationController` are nil. It fails first (plan-59 A6: alive in all 14 runs).
- [ ] Remove the holders, in this order, re-running the test after each: (a) the edit-key monitor is removed in `close()` through the model, not only in `onDisappear` (which a hosted view does not get: `rg -c 'editor disappeared'` on the spike's log → 0 before the close fix); (b) the eight `focusedSceneValue` calls publish values that hold the model weakly (a `struct` with a `weak var`), so SwiftUI's storage cannot keep it; (c) the hosted root view is replaced by `EmptyView` and the window's `contentViewController` set to nil in `close()`.

Acceptance: the test passes.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests/testClosingFreesTheDocument 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 4 min per run). By the owner's decision this acceptance is not weakened: with (a)–(c) in and the test still failing, the phase is reported to the owner with the test's output and the plan stops there.
Commit: —

## Validation Plan

- Tests: `SchematicNSDocumentTests` (new); the 8 test files of A16.
- Coverage check: `rg -c 'WindowCloseGuard\(' schematical/schematical` → 1 file (the Part Editor); `rg -c 'tabNextWindow|confirmQuit|savedSnapshot' schematical` → no output.
- Runtime proof: phase 3's check on the real app; plan-59's H evidence is the reference for everything else.
- Doc sync: none here; 61-D carries the docs for the plan.
- Final gate (once, after phase 5): `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests -only-testing:schematicalTests/DocumentUndoTests` plus the 8 A16 classes → `** TEST SUCCEEDED **` (est. 8 min).

## Decisions

- The route is the hosted `NSDocument` — decided by plan-59's selection rule (step 4), 2026-11-24 (`findings.md` § Selection).
- Documents autosave in place — decided by the owner, 2026-11-24.
- A closed document must be freed, proved by a test; no fix means the plan stops and returns to the owner — decided by the owner, 2026-11-24.
- The child windows are AppKit windows hosting their views — decided by the author, 2026-11-24 (probe A5, H).

## Corrections

1. The print-entitlement prerequisite was written as MET from the main checkout, whose `project.pbxproj` is modified and not committed (`git status` at the start of execution: ` M schematical/schematical.xcodeproj/project.pbxproj`). `diff` of the worktree's file against it: the main checkout adds `ENABLE_RESOURCE_ACCESS_PRINTING = YES` (and seven `ENABLE_RESOURCE_ACCESS_* = NO` lines, and `ASSETCATALOG_COMPILER_INCLUDE_ALL_APPICON_ASSETS = NO`) to both configurations. The committed project has none of it. Commit 8f33d65b recorded this row as `MET (2, re-run at execution)` before the command's output was read; that status was wrong and is corrected here. Phase 1 had landed by then; it does not touch printing.

## Summary

The risk is phase 5: the spike did not free a closed document. Everything
else here ran in the spike and has its evidence file.
