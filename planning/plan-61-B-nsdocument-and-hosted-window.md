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
| The print entitlement is in the project | `rg -c 'ENABLE_RESOURCE_ACCESS_PRINTING = YES' schematical/schematical.xcodeproj/project.pbxproj` → `2` | MET (`2`, 2026-11-24, after the owner committed it on `main` as 7f51ec52 and `main` was merged into the worktree, 77c59719). It was `0` in the worktree before that (Corrections 1) |

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
Commit: 8f33d65b

### Phase 2 — The document class, read and write

- [x] New `Document/SchematicNSDocument.swift`: the class with `read`, `writeSafely`, `data(ofType:)`, the autosave rule, `fileURL` forwarding to the model, `undoManager` from the model's document. No window yet. Done. `SchematicFileController.fileURL` became settable and `offerToAdd` internal here (the model forwards the URL to it); the custom-parts offer is made from `read`, and only once the document has a window.
- [x] New `schematicalTests/SchematicNSDocumentTests.swift`: (1) for each of the 10 examples, `read(from:ofType:)` then `data(ofType:)` decodes equal to what `SchematicFile.apply` + `SchematicFile(document:fileURL:)` writes; (2) a design with a load file in `…/A` written with `writeSafely(to: …/B/design.schematical)` stores `../A/rom.bin`, and with a destination under `Autosave Information` stores the absolute path; (3) reading from an autosave location resolves an absolute path; (4) an edit sets `isDocumentEdited`, undo clears it, `data(ofType:)` between two typed characters gives two undos. Done: the four, plus `testReadingAgainStopsTheSimulationAndClearsUndo` (Revert's read). In (4) `isDocumentEdited` is read after a run loop turn (plan-61-A Corrections 8). `Executed 5 tests, with 0 failures`.

Acceptance: the four tests pass.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min). Result: `Executed 5 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: e4f5ea83

### Phase 3 — The hosted window, the menu, the scene removed

- [x] `SchematicNSDocument.makeWindowControllers()` (hosting controller, `sceneBridgingOptions = [.toolbars]`, `toolbarStyle = .unifiedCompact`) and `close()`. Done. The window also gets a frame autosave name, and the document hands it to the exporter for its sheets (so `ContentView` lost its `WindowReader`).
- [x] `Info.plist`: `CFBundleDocumentTypes` for the schematic type. `schematicalApp.swift`: delete the `WindowGroup(id: "schematic")` scene and `applicationShouldTerminate`; move `.commands` to the first remaining scene; give the remaining scenes `.handlesExternalEvents(matching: [])`, `.commandsRemoved()` and `.defaultLaunchBehavior(.suppressed)` (the Part Editor scene's external events are 61-C's). Done: 13 scenes carry the three modifiers; the first (Memory, until phase 4) carries `.commands`.
- [x] New `DocumentMenuCommands` (in `SchematicNSDocument.swift`); `SchematicFileCommands` loses New Project, Open…, Save and Save As…; delete `UndoCommands` and its `.focusedSceneValue`. Done (Corrections 4, 5). `UndoCommandsTests.swift` went with `UndoCommands`; `DocumentUndoTests.testMenuTitlesFollowTheDocument` now reads the manager's `undoMenuItemTitle` and `canUndo`, which is what the system's items show.
- [x] `SchematicFileController`: the deletions of § 4; `ContentView`: remove `file.loadClaimedOpen()`, `.navigationTitle(file.displayName)`; `AutomationSession.swift`: the three `NSDocumentController` calls. Done (Corrections 2, 3): 844 lines → 521. `AutomationSession.swift`: `new_window`, `open`, `save` and the `unsaved` mark of `list_windows`.
- [x] `schematicalTests`: the 9 constructor sites follow `findings.md` § A16 (H column). Done, all 9 in 8 files, through `TestSupport.systemDocument(of:)` / `openSystemDocument(of:catalog:until:)`. New `UserLibraryUpdateTests.testAnUpdateStopsARunningDocument` replaces the stop counters (Corrections 2). 12 classes (with `VerifyDatasheetLifetimeTests`): `Executed 141 tests, with 1 test skipped and 0 failures`, no restart of the test host.
- [x] Added during execution (Corrections 8): `Datasheet/DatasheetFinder.swift` and `VerifyDatasheetModel` get `nonisolated deinit {}`; new `schematicalTests/VerifyDatasheetLifetimeTests.swift` (2 tests). Before the fix the first test aborted the host (`Executed 0 tests`, `pointer being freed was not allocated`, `Restarting after unexpected exit`); after it `Executed 2 tests, with 0 failures`.

Acceptance: a file opened from the shell shows in a document window, and the menu has the document items.
  Check: build, then `open -a <Debug app> examples/6502.schematical; osascript -e 'tell application "System Events" to tell process "schematical" to get {name of window 1, name of every menu item of menu "File" of menu bar 1}'` → `6502.schematical` and a list holding `Save`, `Duplicate`, `Rename…`, `Move To…`, `Revert To` (est. 8 min with the build; run with the owner's installed app closed: both have the bundle id `jgz.schematical`). Result, read by pid through the Accessibility API (`AXUIElementCreateApplication(pid)`; System Events cannot tell two builds apart) with no other copy of the app running: `windows: ['6502.schematical' document=6502.schematical]`; File: `New`, `Open…`, `Open Recent ▸`, `New Part…`, `Load Part…`, `User Library…`, `Close`, `Close All`, `Save`, `Duplicate`, `Save As…`, `Rename…`, `Move To…`, `Revert To ▸ [Last Saved, Browse All Versions…]`, `Export ▸ [Image…, PDF…, Gerber PCB…]`, `Print…`; Edit starts `Undo (disabled)`, `Redo (disabled)`.
Commit: c397e9e3

### Phase 4 — Child windows

- [x] `ChildWindowCenter`, `ChildWindowOpener`, `ChildWindowCloser` (from `spike-H.patch`); `ContentView`'s `openWindow` and `dismissWindow` become the stand-ins. Done, in new `Document/ChildWindows.swift` (Corrections 9).
- [x] `sessionID` → `documentID` in the 8 files (27 lines); delete `SchematicSessions`; the seven views call `SchematicEditorModel.session`; delete the 7 child scenes from `schematicalApp.swift`. Done. `AutomationTools.sessions()` walks the open documents too, and `AutomationTests`' windows are system documents listed with `NSDocumentController` (it had registered with `SchematicSessions`); `scripts/harness/export-menu.swift` follows.
- [x] `SchematicNSDocumentTests`: opening a Bill of Materials child for two documents gives two windows; closing one document closes only its own. Done: `testChildWindowsBelongToTheirDocument`, and `testAHostedViewOpensTheAppsSceneWindows` (Corrections 10).

Acceptance: the new test passes and no `sessionID` is left.
  Check: `rg -c 'sessionID|SchematicSessions' schematical/schematical` → no output; `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min). Result: the search prints nothing; `SchematicNSDocumentTests` with `AutomationTests` and the three library classes: `Executed 62 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`.
Commit: aaf1974e

### Phase 5 — A closed document is freed (owner's must-fix)

- [x] `SchematicNSDocumentTests.testClosingFreesTheDocument`: make a document, `makeWindowControllers()`, show the window, run the simulation, open a Memory child, `close()`, spin the run loop for 2 s, assert weak references to the model, the `SchematicDocument` and the `SimulationController` are nil. It fails first (plan-59 A6: alive in all 14 runs). Done. Before any fix it aborted the host (Corrections 12); with the deinits in it failed as plan-59 found: `XCTAssertNil failed: "schematical.SimulationController" - the simulation is still alive`, and the same for the schematic, the exporter and the model (the system document and the window were freed). A second test, `testClosingAKeyWindowFreesTheDocument`, closes a key window in the active app (Corrections 14).
- [x] Remove the holders, in this order, re-running the test after each: (a) the edit-key monitor is removed in `close()` through the model, not only in `onDisappear` (which a hosted view does not get: `rg -c 'editor disappeared'` on the spike's log → 0 before the close fix); ~~(b) the eight `focusedSceneValue` calls publish values that hold the model weakly (a `struct` with a `weak var`), so SwiftUI's storage cannot keep it~~ — (b) moot: measured, not guessed (Corrections 13, 14); (c) the hosted root view is replaced by `EmptyView` and the window's `contentViewController` set to nil in `close()`. Done: (a) alone → the same four still alive; (c) added → the same four; then the real holders (Corrections 13) → `Executed 2 tests, with 0 failures`.
- [x] Added during execution (Corrections 13): new `Document/EditorEventMonitors.swift`; `SchematicCanvasView` and `PCBCanvasView` install their drawing-key monitors through it (`@Environment(\.editorEventMonitors)`), `ContentView` its Edit-key monitor; `SchematicNSDocument.close()` calls `model.eventMonitors.removeAll()`.
- [x] Added during execution (Corrections 12): `nonisolated deinit {}` on `SchematicDocument`, `SimulationController`, `BuzzerSound`, `CanvasViewport`, `BoardPointer`, `DrawerMemo`, `SchematicFileController`, `SchematicEditorModel` and `EditorEventMonitors`.

Acceptance: the test passes.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests/testClosingFreesTheDocument 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 4 min per run). By the owner's decision this acceptance is not weakened: with (a)–(c) in and the test still failing, the phase is reported to the owner with the test's output and the plan stops there. Result: `testClosingFreesTheDocument` `passed (4.539 seconds)`; `testClosingAKeyWindowFreesTheDocument` `passed (5.453 seconds)`, not skipped, with the test host brought to the front from the shell; `** TEST SUCCEEDED **`. The criterion is stronger than written: it also asserts the exporter, the system document and the window are freed, and that a key window's document is.
Commit: —

## Validation Plan

- Tests: `SchematicNSDocumentTests` (new); the 8 test files of A16.
- Coverage check: `rg -c 'WindowCloseGuard \{' schematical/schematical` → 1 file (the Part Editor; the pattern as written, with `\(`, matches nothing, before or after: the guard is built with a trailing closure); `rg -c 'tabNextWindow|confirmQuit|savedSnapshot' schematical` → no output. Run 2026-11-24: `Editor/PartEditorModel.swift:1`; no output.
- Runtime proof: phase 3's check on the real app; plan-59's H evidence is the reference for everything else.
- Doc sync: none here; 61-D carries the docs for the plan.
- Final gate (once, after phase 5): `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests -only-testing:schematicalTests/DocumentUndoTests` plus the 8 A16 classes → `** TEST SUCCEEDED **` (est. 8 min). Run 2026-11-24 with `AutomationTests`, `VerifyDatasheetLifetimeTests` and the two undo suites added (14 classes): `Executed 232 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`; the skip is `AutomationTests.testFileToolsAreOffWithoutASharedFolder`, not the key-window test.

## Decisions

- The route is the hosted `NSDocument` — decided by plan-59's selection rule (step 4), 2026-11-24 (`findings.md` § Selection).
- Documents autosave in place — decided by the owner, 2026-11-24.
- A closed document must be freed, proved by a test; no fix means the plan stops and returns to the owner — decided by the owner, 2026-11-24.
- The child windows are AppKit windows hosting their views — decided by the author, 2026-11-24 (probe A5, H).

## Corrections

1. The print-entitlement prerequisite was written as MET from the main checkout, whose `project.pbxproj` is modified and not committed (`git status` at the start of execution: ` M schematical/schematical.xcodeproj/project.pbxproj`). `diff` of the worktree's file against it: the main checkout adds `ENABLE_RESOURCE_ACCESS_PRINTING = YES` (and seven `ENABLE_RESOURCE_ACCESS_* = NO` lines, and `ASSETCATALOG_COMPILER_INCLUDE_ALL_APPICON_ASSETS = NO`) to both configurations. The committed project has none of it. Commit 8f33d65b recorded this row as `MET (2, re-run at execution)` before the command's output was read; that status was wrong and is corrected here. Phase 1 had landed by then; it does not touch printing. Asked, the owner committed the setting on `main` (7f51ec52); `main` was merged into `worktree-P-61` (77c59719, which also brought plan-58) before phase 2 started.
2. `updateOpenDocuments` moved from the controller to `SchematicNSDocument` and walks `NSDocumentController.shared.documents`, as `findings.md` § A16 (H) says; § 4's list did not name it and 61-D § 2 expected it to stay in the controller. With it went the controller's weak registry of every instance and its `willReplaceDocument` closure, so its initializer is `init(document:catalog:)` and every one of the 9 test sites changed (A16 called 4 of them "construction only"). The three library tests list their document with `NSDocumentController.addDocument` until the test ends. The old tests counted calls of the closure; the part they update (`genericText`) is `not simulated`, so a document holding it cannot be running: the stop is now tested by `testAnUpdateStopsARunningDocument` with a custom NAND that does run (`isRunning` true before the update, false after; a second open document without the part still running).
3. `SchematicEditorModel.platformDocument` is a weak reference the document sets in its initializer, not a search of `NSDocumentController.shared.documents` (§ 4): a document a test builds is not in that list, and the search would be done on every `list_windows`.
4. `Save As…` is shown in place of `Duplicate` only while Option is held, by the app's existing `OptionOnlyMenuItems` (it already hides Window › Verify Datasheet that way); both keep their keys (`allowsKeyEquivalentWhenHidden`). SwiftUI `Commands` cannot declare an alternate item.
5. Export and Print… moved from the group after New to a group after the document's Save items (`CommandGroup(after: .saveItem)`), the order of a document app; the real-app check in phase 3 shows the group renders (the old comment in `SchematicFileCommands` said a `WindowGroup` app never showed `.saveItem`). ⌘O and ⌘S stay the Part Editor's in its own window: the document items drop their key there.
6. The custom-parts offer is made when the document's window is made (or at once for a Revert), not from `read`: `read` runs before there is a window to put the sheet on.
7. Between this phase and phase 4 the seven child windows do not open: the hosted editor still calls SwiftUI's `openWindow`, which has no scene behind it. Phase 4 replaces the calls.
8. A crash of the test host blocked this phase's test run and no task covered it. With the schematic scene gone, SwiftUI rebuilds the scene list while tests run (`AppDelegate.scenesDidChange`, from a run loop observer) and frees the spare `VerifyDatasheetWindowView` it had built, with the `VerifyDatasheetModel` in its `@State`. That model's isolated deinit frees its `DatasheetFinder`, whose isolated deinit is nested in it, and the Swift runtime aborts (`StopLookupScope`, `pointer being freed was not allocated`) when the thread has a task-local value bound, as XCTest's has. Two of two runs of `AutomationTests` died in `testActiveLowIsReported` (crash reports `schematical-2026-11-24-203005.ips`, `…-203331.ips`). It is not new: reports from 2026-11-23 (`…-175423.ips`, `…-182532.ips`, `…-213921.ips`, other builds) have the same stack; the missing scene made it frequent. Fixed as the code base already does for `RoutingSession`, `AssistRouter` and `AutorouteModel` (`nonisolated deinit {}`), with a test that reproduces the abort. This matters to phase 5: freeing a main-actor class with an isolated deinit nested in another's is the same hazard.
9. Beyond the spike's `ChildWindowCenter`: a child window has a delegate (its own `NSWindowDelegate`, on a window the app makes; not a proxy on SwiftUI's) that forgets the window when it closes and empties its hosted view, so a child the user closed is built again when asked for and does not keep the document. The hosting controller bridges `.title` as well as `.toolbars`, so the views' `.navigationTitle` (`ROM1 · AT28C256-15PU`) still names the window. A child is never a tab (`tabbingMode = .disallowed`).
10. § 3 said a hosted editor has no scene to open windows from. That holds for windows of its own; the app's remaining scenes (Datasheet, Part Editor, Help) still open from views inside the hosted editor with SwiftUI's `openWindow` (`ChipDrawerView`, `DatasheetWindow.swift`, `MarkdownDatasheetWindow.swift`). Measured by `testAHostedViewOpensTheAppsSceneWindows`: a view in an `NSHostingController` window calls `openWindow(id: "help", value: .pcb)` and a window titled `PCB Help` appears.
11. The app quits when its last window closes (`applicationShouldTerminateAfterLastWindowClosed`, unchanged). With no schematic window open in the test host, a test that closed a window ended the test run (`Restarting after unexpected exit`, no crash report). `AppDelegate.quitsAfterLastWindowCloses` (true in the app) is set false by the tests that close windows.
12. § 3 expected phase 5 to be about references. The first run of the test did not fail, it aborted the host: SwiftUI's teardown of the hosted editor (`NSHostingView.deinit` → `Update.ensure` → `AGGraphInvalidate`) frees the editor's main-actor classes, and each has an implicit isolated deinit (`SWIFT_DEFAULT_ACTOR_ISOLATION = MainActor`), which aborts in the runtime as in Corrections 8 (`schematical-2026-11-24-204351.ips`: `DrawerMemo.__deallocating_deinit` → `swift_task_deinitOnExecutorImpl` → `StopLookupScope`). That is the hazard `TestSupport` notes for `SchematicDocument` (its tests keep every document forever), and it would have crashed the app the first time a closed document really was freed. The 9 classes a closing document frees have `nonisolated deinit {}`; none had a deinit that needed the main actor. `rg -c 'nonisolated deinit' schematical/schematical` → 16 in 14 files (5 before the plan; this plan added 11, the two of Corrections 8 among them).
13. § 3 and phase 5 named the holders as the Edit-key monitor's closure and the eight focused values. Measured with probe tests (kept out of the tree): a document with no window is freed; one whose window hosts a trivial view holding the model is freed; one whose window hosts `ContentView` is not, with (a) and (c) in and with no focused value published at all. The holders were two more `NSEvent` monitors: `SchematicCanvasView` and `PCBCanvasView` each install a key monitor in `onAppear` whose closure holds the canvas view and so the document, and take it off only in `onDisappear`, which a hosted view does not get at close (emptying the root and forcing layout before the close did not deliver it). So a closed document also went on handling drawing keys. With those two monitors off the document was freed. The fix is one owner for the editor's monitors, emptied by `close()`.
14. (b) was not done: with the monitors fixed, `testClosingAKeyWindowFreesTheDocument` passes with all seven focused values published (`.focusedSceneValue` × 7 in `ContentView`; there were eight before `schematicUndo` went in phase 3) and the window key in the active app, which is when SwiftUI publishes them. The plain test closes a window that is not key (a test host started by `xcodebuild` is not the active app and cannot make itself so), so the key-window test waits up to 8 s for the app to be active and is skipped when it is not; for the result recorded here the host was brought forward from the shell with `open -a` during the run, and the front went back to the app that had it.

## Summary

The risk is phase 5: the spike did not free a closed document. Everything
else here ran in the spike and has its evidence file.
