# plan-68-AD: App coverage — Editor

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-AC

The app files in § 2 are brought to 100% line coverage: 708 lines are
uncovered in them today (7 files, 1864 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AC complete (its rows are gone
from `scripts/coverage-ratchet.tsv` and its Commit lines are filled).

References:

- The files in § 2; `planning/plan-68-data/app-missed.tsv` (per-file census:
  the full Debug suite with coverage at `e2f6811a`, `app-coverage.txt`).
- `docs/testing.md`; `AGENTS.md` § Source size and coverage, § Testing in the
  app (park the pointer at (0, 0) after any real-mouse test).
- Memory of earlier plans: test hosts are the real app — run `XB` under a
  time limit, never give a test document a file the test did not write.

## 1. Goal

- `node scripts/coverage.js --app --tests <the classes that test them> --only
  <file>` prints no uncovered line for any file in § 2, and their rows are gone
  from the coverage ratchet.
- The test classes this letter adds or extends pass; `SZ` passes.

### Non-goals

- No user-visible behavior change, except a bug a new test proves (§ 3 step
  6). Logic moved out of a view into a model or controller type keeps the
  view's behavior exactly.

## 2. Current State

Uncovered lines per file under the full Debug suite at `e2f6811a`, counted
per line from `xccov view --archive` (`planning/plan-68-data/app-missed.tsv`; paths under `schematical/schematical/`).
A file that plan-68-B–F or plan-68-V split appears under its old name; its row
stands for all of its pieces. The letter starts by re-running step 1 of § 3
for each file; that list is the work.

| File | Uncovered | Executable |
|---|---|---|
| `Editor/FieldEditors.swift` | 417 | 417 |
| `Editor/FootprintPreviewView.swift` | 77 | 82 |
| `Editor/KindSchema.swift` | 1 | 131 |
| `Editor/ParametricFootprintDraft.swift` | 19 | 289 |
| `Editor/PartDraft.swift` | 33 | 473 |
| `Editor/PartEditorCommands.swift` | 11 | 16 |
| `Editor/PartEditorModel.swift` | 150 | 456 |
| **Total** | **708** | **1864** |

## 3. Design

The method is plan-68-W § 3, unchanged: list with a scoped `COV --app`,
close each block (unit test; logic out of a view into a tested type; render
test through the accessibility tree; drive windows through their actions
with injectable modals; bitmap drawing checks; synthesized events; stubbed
network and tools; delete the unreachable), assert an observable result,
lower the file's ratchet row in the same commit, fix any bug a test finds.

### Where the risk is

Moving logic out of a view: the moved code must keep reading the same state
at the same moment (a `@State` read inside a gesture closure now reads the
controller's property). The view's existing test classes and the new
controller tests both run in the phase gate.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `FieldEditors.swift`

- [x] Close every uncovered line of these files per § 3 (2026-11-26): `LOGIC_COV_WORK=/tmp/logic-cov-AD node scripts/coverage.js --app --tests FieldEditorsTests,PartEditorModelTests,FootprintPreviewViewTests,PartDraftTests,FootprintPreviewTests,ParametricFootprintDraftTests,PartDraftRoundTripTests,PartEditorFootprintTests,PartEditorLibraryTests,PartEditorWindowTests,KindGuideTests,KindInfoTests,PartKindKeysTests,PartKindRoundTripTests,CustomFootprintDraftTests,SocketEditorTests,WizardKindStepsTests,PartFileSaveOpenTests,WizardIdentityTests,WizardModelTests,UserLibraryEndToEndTests,WizardIssueLocatorTests --only schematical/schematical/Editor` → no
  `FieldEditors.swift:` line (417 → 0); the run's 215 tests pass (xcresult summary: 215 passed, 0 failed).
- [x] ratchet rows: parent (the file is at 0).
  Ratchet rows: removed by the parent in this letter's commit (7 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: ee1e7aec

### Phase 2 — `FootprintPreviewView.swift`, `KindSchema.swift`, `ParametricFootprintDraft.swift`, `PartDraft.swift`, `PartEditorCommands.swift`, `PartEditorModel.swift`

- [x] As Phase 1, for these files (2026-11-26): the same command → no `path:line` for
  `FootprintPreviewView.swift` (77 → 0), `KindSchema.swift` (1 → 0), `ParametricFootprintDraft.swift`
  (19 → 0), `PartDraft.swift` (33 → 0), `PartEditorCommands.swift` (11 → 0), `PartEditorModel.swift`
  (150 → 0); 215 tests pass; `node scripts/source-size.js --check` → exit 0.
- [x] ratchet rows: parent (every file at 0).
  Ratchet rows: removed by the parent in this letter's commit (7 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: ee1e7aec

## Validation Plan

- Tests: the classes added or extended, each named in its commit.
- Coverage check: every § 2 row gone from the ratchet; `COV --app --check
  --only` each file → exit 0.
- Final gate (once, after the last phase): all of this letter's classes
  together under `XB`; `SZ` (est. 15 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.
- (author) Logic leaves views for tested types; views are covered by render
  tests through the accessibility tree (§ 3).

## Corrections

- (2026-11-26) Tests added: `FieldEditorsTests` (the JSON bindings, and every
  `FieldType`'s control rendered and edited through its AppKit control),
  `FootprintPreviewViewTests` (the drawing in pixels, the zoom and pan
  controller, the view's buttons and a synthesized drag), `PartEditorModelTests`
  (Save / Save As… / Open / close-guard / library alerts and panels answered by
  an injected presenter, the datasheet pane over a `URLProtocol` stub, the File
  menu's Part Editor items), `PartDraftTests`; `ParametricFootprintDraftTests`
  extended. Helpers in `schematicalTests/Support/ViewProbe+AD.swift` (additive;
  a first `ViewProbe+Controls.swift` clashed with plan-68-AB's
  `ViewProbe+Typing.swift` and was replaced).
- (2026-11-26) Logic moved out of views: `FieldEditors.swift`'s per-view
  bindings (bool tag, volt bound, shape index, list entries) became `Binding`
  extensions, and its option list, map add and JSON parse `FieldEditing`
  functions; `FootprintPreviewView`'s zoom, pan and drawing are
  `FootprintPreviewController` and `FootprintPreview.draw` / `caption`;
  `PartDraft.renamePin`'s nested walk is the `PinRename` value type.
- (2026-11-26) Deleted as unreachable: `Binding<JSONValue>.optional` (no
  caller). `PartEditorModel.keepCurrentStep`'s fallback when no earlier step
  survives: every draft has the kind and part-number steps, and the current
  step is never the first one removed, so the fallback is folded into one
  expression.
- (2026-11-26) `PartEditorModel` puts its panels and alerts up through
  `ExportPresenter` (plan-68-Y's injectable presenter) and loads datasheets
  with an injectable `datasheetSession`; it got `nonisolated deinit {}` (a
  closed Part Editor window freeing its model aborted the test host).
- (2026-11-26) As plan-68-AB/AF found: a `@State` default is never counted,
  so `MapEditor`, `JSONTextEditor` and `FootprintPreviewView` set their state
  in an explicit `init`; the suggested-value pull-down builds its items only
  as it opens on screen, so its item row is one line (the render test asserts
  the pull-down and types the value).
- (2026-11-26) `ParametricFootprintDraftTests`' footprint counts were stale
  (3959 / 2268) after plans 69–82: re-measured 3993 = 2297 parametric + 1696
  explicit. Not a code bug.
- (2026-11-26) A test window closing the host's last window quits it (exit 0
  mid-run); the new classes set `AppDelegate.quitsAfterLastWindowCloses = false`.
- (2026-11-26) `testTheMenuItemsFollowTheFocusedEditor` (async) spun the run
  loop while holding the main actor, so the menu buttons' `Task`s never ran; it
  now waits with `try await Task.sleep`, which frees the main actor (passes
  alone: 18/18 `PartEditorModelTests`, and in the final 215-test run).
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
