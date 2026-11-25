# plan-61-A: One `UndoManager` per document

Last updated: 2026-11-24
Overall Effort: x-large (1d–3d)
Effort: large (3h–1d)
Depends on: nothing

Plan 61 moves the app from the hand-built `SchematicFileController` to the
system's document architecture: an `NSDocument` subclass whose window hosts the
SwiftUI editor. plan-59 (a spike) built that and a `DocumentGroup` alternative,
ran 27 probes on each and chose the `NSDocument` route by a rule fixed
beforehand; the owner then answered eight questions. This plan is the result.

The four sub-plans, in order: **A** undo on one `UndoManager` per document;
**B** the document class, its hosted window, the File menu's document items and
the child windows; **C** what the user sees change (title bar, tab setting,
launch, exports, print, part files, the restored page); **D** automation, the
deletion of the old controller, tests and docs.

The single outcome of 61-A: `SchematicDocument` records every step on one
`UndoManager` it owns; the two per-page lists are gone; Edit › Undo and Redo
act on the document.

References:

- `planning/plan-59-data/findings.md` (verdicts U1–U5, § Owner answers),
  `planning/plan-59-data/probes.md`.
- `planning/plan-59-data/spike-H.patch`: the working code. `SchematicUndoBridge.swift`
  and the `SchematicModel.swift` hunks in it are this sub-plan's reference.
- `schematical/schematical/Canvas/UndoHistory.swift`,
  `Canvas/SchematicModel.swift` (`recordEdit`, `finishRecording`, `undo(on:)`),
  `Document/UndoCommands.swift`.
- AGENTS.md § Git.

## Prerequisites

These are a precondition on the whole of plan 61, stated once here; the other
letters point here.

| Must be true | Command | Status |
|---|---|---|
| plan-59 is complete and archived | `ls planning/plan-59-*.md` → no matches; `ls planning/completed/plan-59-*.md \| wc -l` → `4` | MET (`4`, re-run 2026-11-24 at execution) |
| The spike's reference code is in the tree | `ls planning/plan-59-data/spike-H.patch planning/plan-59-data/findings.md` → both listed | MET (re-run at execution) |
| Xcode 26 toolchain | `xcodebuild -version \| head -1` → `Xcode 26.3` | MET (re-run at execution) |
| The undo tests pass before the change | `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/PCBUndoTests 2>&1 \| tail -3` → `** TEST SUCCEEDED **` | MET (`** TEST SUCCEEDED **`, 87 tests, 2026-11-24, before any change) |

Everything below is written against these holding.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `SchematicDocument` has `let undoManager: UndoManager` and `undo()`, `redo()`,
  `undoName`, `redoName`. One recorded step is one undo group on it.
- `UndoHistory` (the struct with two per-page lists) and `undo(on:)`,
  `redo(on:)`, `undoName(on:)`, `redoName(on:)` are deleted.
- Edit › Undo and Redo act on the document, whichever page is shown, and are
  disabled while the simulation runs (owner's answer to U5). They become the
  system's own items in 61-B, when the window belongs to an `NSDocument`.
- Every test that called the per-page API calls the per-document one and passes.

### Non-goals (explicit constraints)

- The `.schematical` file format does not change.
- `UndoStep`, `UndoDiff`, `UndoKey`, `UndoValue` and `recordEdit`'s signature do
  not change: a step is still a per-item diff, named by the outermost edit.
- `SchematicFileController` is not touched here (61-B replaces it). Its
  `hasUnsavedChanges` stays the snapshot compare until then.
- No window, scene or menu other than Edit › Undo / Redo changes.

## 2. Current State

Undo is the app's own, per page. `SchematicDocument` holds
`private var history = UndoHistory()` (`Canvas/SchematicModel.swift:2046`).
`recordEdit` (`:2103`) brackets an edit; `finishRecording` (`:2129`) diffs the
state and pushes an `UndoStep` onto the undo list of every page its keys
belong to. `undo(on:)` (`:2178`) and `redo(on:)` (`:2194`) pop from one page's
list, apply the diff with `UndoDiff.apply`, and skip a step that no longer
changes anything. Typing coalesces: a push with the same `coalescing` key as
the top step merges into it until `endCoalescing()` (`:2170`). `history.clear()`
runs when a file is applied (`:2034`, `:2365`). While `isPlaying`, `recordEdit`
runs its body and records nothing, and `undo(on:)` returns false.

Edit › Undo and Redo are `UndoCommands` (`Document/UndoCommands.swift`), which
replaces `.undoRedo` and routes ⌘Z to the page shown, or to the responder
chain while a text view is editing. `ContentView` publishes
`.focusedSceneValue(\.schematicUndo, UndoTarget(document:page:))`.

The owner decided undo is per document on one `NSUndoManager`
(`findings.md` § Owner answers; plan-59-A § Decisions).

### Measured populations

| What | Count | Command |
|---|---|---|
| App lines calling the per-page API | 12 in 3 files | `rg -c 'undo\(on\|redo\(on\|undoName\|redoName' schematical/schematical` → 3 files, sum 12 |
| Test lines calling it | 274 in 42 files | the same pattern on `schematical/schematicalTests` → 42 files, sum 274 |
| Of those, calls naming the PCB page | 78 in 27 files | `rg -c 'undo\(on: \.pcb\|redo\(on: \.pcb' schematical/schematicalTests` → 27 files, sum 78 |
| Tests in the two undo suites | 50 and 37 | `rg -c 'func test' schematicalTests/SchematicUndoTests.swift schematicalTests/PCBUndoTests.swift` → 50, 37 |
| `UndoHistory` uses outside its file | 1 | `rg -c 'UndoHistory' schematical/schematical` → 2 files, 3 lines (2 in its own file) |
| Files naming `schematicUndo` or `UndoTarget` | 2 | `rg -l 'schematicUndo\|UndoTarget' schematical/schematical` → `ContentView.swift`, `Document/UndoCommands.swift` |

### Verified properties

| Claim | How verified |
|---|---|
| One registration per `UndoStep`, whose handler applies the diff and registers the other direction, undoes and redoes Schematic, PCB, Schematic edits in strict reverse order with the titles `Undo Add Net Label`, `Undo Move` | plan-59 probe U3 on both spike builds: `planning/plan-59-data/evidence/U1-U3-H.txt` |
| Typing merges into the registered step with no second registration (6 characters: 1 registration, 5 merges); a 5-frame drag is one step | the same evidence file, the `coalescing` block |
| With `groupsByEvent = false` and one `beginUndoGrouping` / `endUndoGrouping` pair per step, undo, redo and the action names are right with no run loop, and an `NSDocument` using that manager reports `isDocumentEdited` true after an edit | Probe `planning/plan-59-data/scripts/probe-undo-groups.swift` (an `NSDocument` subclass, a target, three edits; built with `swiftc`, run from the shell): `edit 2: … Undo Add Wire`, `undo: value 1 … Undo Move / Redo Add Wire`, `redo: value 1`, `cleared, edit, no run loop: edited true` |
| By-event grouping needs an `NSEvent` to close a group: a step registered from a timer left `groupingLevel` at 1 | plan-59-A Corrections 12 |
| A text field's field editor has its own undo manager, so typing in a field does not land on the document's | plan-59 probe U4: `responderUndoManagerIsDocuments=false … title='Undo Typing'` |
| An `UndoManager` subclass whose `canUndo` and `canRedo` are false while locked disables a document window's Undo item and blanks its title | Probe `planning/plan-59-data/scripts/probe-undo-lock.swift`: `window.undoManager is the document's: true`, `unlocked: validate = true title: Undo Move`, `locked:   validate = false title: Undo` |
| The undo manager has already popped an action when its handler runs, so a handler cannot refuse; refusing must happen before `undo()` is called | plan-59 probe U5 (`evidence/U5-G.txt`): the spike had to re-register and `redo()` |

## 3. Design Overview

`SchematicDocument` owns the manager, so tests and the harness undo with no
window and no `NSDocument`; 61-B hands that same manager to the `NSDocument`
(`NSDocument.undoManager` is settable). The spike's bridge class is folded
into `SchematicDocument`: it needs the private `state` and `setState`.

- **Recording.** `finishRecording` builds the `UndoStep` as today and calls
  `register(step, backwards: true)`: `beginUndoGrouping`, `registerUndo(withTarget:
  self)`, `setActionName(step.name)`, `endUndoGrouping`. The handler applies the
  diff (`UndoDiff.apply`) and registers the opposite direction. The manager has
  `groupsByEvent = false`: one step is one group, by construction and without
  a run loop.
- **Coalescing.** The document keeps `openStep`, the step on top of the stack
  while typing may still merge. A step with the same `coalescing` key folds its
  changes into `openStep` (the code of `UndoHistory.merge`) and registers
  nothing. `endCoalescing()`, any undo or redo, and `willSave()` (called by the
  document class in 61-B) clear `openStep`.
- **Playing.** The manager is a `SchematicUndoManager` (an `UndoManager`
  subclass) whose `canUndo` and `canRedo` are false while the document is
  playing; `undo()` and `redo()` return false then. Any menu item validated
  against the manager is disabled for the run (probe in § 2).
- **A step that no longer applies** (its items are gone) is skipped: `undo()`
  loops `manager.undo()` while `canUndo` until a handler reports a change.
- **File applied.** `removeAllActions()` where `history.clear()` is today.

Where the risk concentrates: the 274 test lines. They are a mechanical rename
(`undo(on: .schematic)` → `undo()`), except tests that assert page
independence, which assert document order after this plan. Those are in
`PCBUndoTests` (37 tests) and are rewritten by reading each one.

Byte-identity is not a gate here; behavior changes on purpose. Expected to
change: any test that undoes on one page after editing the other.

Rejected alternatives: keeping `undo(on:)` as a wrapper that ignores its page
(274 call sites would lie about what they do); `groupsByEvent = true` (needs an
event per step, so tests and automation would group several steps into one);
a separate bridge class as in the spike (it reached into the document through
three hooks; one owner is simpler).

## 4. Detailed Design

In `Canvas/SchematicModel.swift`:

```swift
@ObservationIgnored let undoManager: SchematicUndoManager   // groupsByEvent = false; isLocked follows isPlaying
@ObservationIgnored private var openStep: UndoStep?

@discardableResult func undo() -> Bool
@discardableResult func redo() -> Bool
var undoName: String? { undoManager.canUndo ? undoManager.undoActionName : nil }
var redoName: String? { undoManager.canRedo ? undoManager.redoActionName : nil }
func willSave()            // openStep = nil
```

`undoName` is read by SwiftUI views; `UndoManager` is not observable, so the
document bumps an observed `undoRevision` counter in `register` and after each
undo and redo, and `undoName` / `redoName` read it first.

`Document/UndoCommands.swift` keeps its `Commands` in this sub-plan (a SwiftUI
`WindowGroup` window has no document to take the manager from): its two
buttons call `document.undo()` and `redo()`, their titles come from `undoName`
and `redoName`, and they are disabled while `document.isPlaying`. `UndoTarget`
loses its `page`. 61-B deletes the struct when the window gets its `NSDocument`
and the system's items take over.

## Compatibility / Format Impact

None on files. Automation's `undo` and `redo` tools
(`Automation/AutomationSession.swift:160`, `:166`) act on the document, not the
page; their text keeps the step's name. `docs/agent-control.md` says so (61-D
syncs the docs for the whole plan; this sentence lands here because the tool
changes here).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes. Use `- [~]` for partial and say what
> remains. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill the
> phase's `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — The manager beside the lists

Adds the per-document path with nothing using it yet.

- [x] `Canvas/SchematicModel.swift`: add `SchematicUndoManager` and `undoManager`, `openStep`, `undoRevision`, `register(_:backwards:)`, `undo()`, `redo()`, `undoName`, `redoName`, `willSave()`; `finishRecording` registers on the manager as well as pushing to `history`; `endCoalescing()` clears `openStep`; both `history.clear()` sites also call `undoManager.removeAllActions()`. Done: the class is in `Canvas/UndoHistory.swift` (Corrections 1, 2); `isLocked` follows `isPlaying` through its `didSet`.
- [x] New `schematicalTests/DocumentUndoTests.swift`: (1) Schematic, PCB, Schematic edits, three `undo()` then three `redo()`, state and `undoName` after each; (2) six `setNetLabelName` calls are one undo; (3) `beginGesture`, five `moveFootprint`, `endGesture` is one undo; (4) `undo()` returns false, `undoManager.canUndo` is false and nothing changes while `isPlaying`; (5) a step whose item was deleted outside undo is skipped; (6) `willSave()` between two typed characters gives two undos; (7) applying a file empties the manager. Done: 7 tests; `Executed 7 tests, with 0 failures`.

Acceptance: the seven new tests pass and the two old suites still pass.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/DocumentUndoTests -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/PCBUndoTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 6 min, most of it the build). Result: `Executed 94 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: b08d9519

### Phase 2 — The app and automation use it

- [x] `Document/UndoCommands.swift`: `UndoTarget` holds only the document; `UndoRouting.route` returns `.document` in place of `.page`; the two buttons call `document.undo()` / `redo()`, take their titles from `undoName` / `redoName`, and are `.disabled(target?.document.isPlaying == true)`. `ContentView.swift`: `.focusedSceneValue(\.schematicUndo, UndoTarget(document: document))`. `schematicalTests/UndoCommandsTests.swift` (the tests of `UndoRouting`) follows the new cases. Done; `UndoCommandsTests`: `Executed 6 tests, with 0 failures`.
- [x] `Automation/AutomationSession.swift` `undo` / `redo` tools: call `document.undo()` / `redo()` and `undoName` / `redoName`; drop the page from the code path. `docs/agent-control.md`: the two tools act on the document. Done: the tools declare no `page` (one still passed is ignored); `AutomationTests`: `Executed 31 tests, with 1 test skipped and 0 failures`.

Acceptance: the menu's title and action follow the document, not the page.
  Check: new test `DocumentUndoTests.testMenuTitlesFollowTheDocument` (a PCB move, then `UndoRouting.undoTitle(UndoTarget(document:))` is `Undo Move`; with `isPlaying` it is `Undo`) passes: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/DocumentUndoTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 4 min). Result: `DocumentUndoTests` `Executed 8 tests, with 0 failures`, `** TEST SUCCEEDED **` (run with `UndoCommandsTests` and `AutomationTests`).
Commit: —

### Phase 3 — The tests move; the lists go

- [ ] In the 42 test files, replace `undo(on: .schematic)`, `undo(on: .pcb)`, `redo(on: …)`, `undoName(on: …)`, `redoName(on: …)` with the per-document calls (274 lines; `rg -c` from § 2 → no output afterwards).
- [ ] `PCBUndoTests.swift`: read each of the 37 tests; a test that edits both pages and undoes one now asserts strict reverse order over the document. Record in this line how many of the 37 changed their assertions.
- [ ] `SchematicUndoTests.swift`: the same read for its 50 tests.
- [ ] Delete `Canvas/UndoHistory.swift`'s `UndoHistory` struct (keep `DocumentState`, `UndoKey`, `UndoValue`, `UndoChange`, `UndoStep`, `UndoDiff`), `history`, `undo(on:)`, `redo(on:)`, `undoName(on:)`, `redoName(on:)`, and `UndoKey.pages` and `UndoStep.pages` once nothing reads them (`rg -n '\.pages' schematical/schematical/Canvas` names the readers).

Acceptance: no per-page undo API is left and every test class that used it passes.
  Check: `rg -c 'undo\(on|redo\(on|undoName\(on|redoName\(on|UndoHistory\(' schematical` → no output; then the 42 classes: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical $(rg -l 'undo\(\)|redo\(\)' schematical/schematicalTests | sed -E 's#.*/(.*)\.swift#-only-testing:schematicalTests/\1#' | tr '\n' ' ') 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 12 min: 42 classes is the set this phase edits, and a rename can only be proved by running each renamed file; a full Debug suite stalls in `AutorouteModelTests testTheSummaryOnAFU`, so the list is explicit).
Commit: —

## Validation Plan

- Tests: `DocumentUndoTests` (new), the 42 edited classes.
- Coverage check: the `rg -c` in phase 3 returning nothing is the denominator: no caller of the old API remains.
- Runtime proof: plan-59's U1–U5 evidence is the reference behavior; phase 1's tests restate it without a window.
- Doc sync: `docs/agent-control.md` (the `undo` and `redo` tools).
- Final gate (run once, after phase 3): the phase 3 command (est. 12 min).

## Decisions

- Undo is per document on one `NSUndoManager`; the per-page lists go — decided by the owner, 2026-11-24 (plan-59-A § Decisions).
- Undo and Redo are disabled while the simulation runs — decided by the owner, 2026-11-24 (`findings.md` § Owner answers, U5).
- The manager belongs to `SchematicDocument` and uses explicit groups — decided by the author, 2026-11-24, after the probe in § 2.

## Corrections

1. `SchematicUndoManager` lives in `Canvas/UndoHistory.swift`, beside the step types, not in `SchematicModel.swift`. It is `nonisolated` like `UndoStep`: `UndoManager` is not a main-actor class.
2. The skip of a step that no longer applies is in the manager's own `undo()` and `redo()` overrides (they loop until a handler sets `applied`), not in `SchematicDocument.undo()`. Reason: from 61-B the system's Undo item calls the manager directly, and a loop in the document would not run then. A skipped step registers nothing, so it leaves both stacks (test 5 asserts it).
3. Phase 1 registers a second `UndoStep` object with the same contents on the manager, not the one pushed to `history`: both paths merge typed characters into their top step, and merging the same object twice restores a key the first merge had removed. Phase 3 deletes the `history` copy.
4. Test 5 as written ("a step whose item was deleted outside undo") deletes a part while playing, the only edit that records no step. The step skipped is the footprint Move; the Add Part step under it still applies, because it also holds the reference counter, so the test asserts the step under the Move is the one undone.

## Summary

The risk is the 274 test lines and the tests that assert page independence;
the model change itself is the spike's bridge moved into the document.
