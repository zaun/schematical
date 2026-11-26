# plan-68-AC: App coverage — Drawer, Editor

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-AB

The app files in § 2 are brought to 100% line coverage: 715 lines are
uncovered in them today (8 files, 1115 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AB complete (its rows are gone
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
| `Drawer/PartOptionsSection.swift` | 283 | 334 |
| `Drawer/SchematicSidebarView.swift` | 131 | 181 |
| `Drawer/SocketPickerSheet.swift` | 63 | 95 |
| `Drawer/UserLibraryWindow.swift` | 114 | 129 |
| `Editor/BlockPreviewView.swift` | 18 | 18 |
| `Editor/BundledPartFile.swift` | 1 | 109 |
| `Editor/CustomFootprintDraft.swift` | 7 | 151 |
| `Editor/CustomFootprintEditor.swift` | 98 | 98 |
| **Total** | **715** | **1115** |

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

### Phase 1 — `PartOptionsSection.swift`, `SchematicSidebarView.swift`

- [x] Close every uncovered line of these files per § 3 (2026-11-26). `COV --app --tests PartOptionsModelTests,PartOptionsSectionTests,SchematicSidebarViewTests,UserLibraryWindowViewTests,CustomFootprintEditorTests,BlockPreviewRenderTests,CustomFootprintDraftTests,BundledPartFileTests,SocketPickerTests,UserLibraryWindowTests,SelectionSidebarTests
  --only schematical/schematical/` (in the snapshot, Corrections) → 67 tests passed, 0 failed;
  uncovered/executable: `PartOptionsSection.swift` 0/172, `PartOptionsModel.swift` 0/59,
  `UnitValueField.swift` 0/75, `SchematicSidebarView.swift` 0/185 (was 283/334 and 131/181 at the start).
- [x] ratchet rows: parent
  Ratchet rows: removed by the parent in this letter's commit (8 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: fd6c2af2

### Phase 2 — `SocketPickerSheet.swift`, `UserLibraryWindow.swift`, `BlockPreviewView.swift`, `BundledPartFile.swift`, `CustomFootprintDraft.swift`, `CustomFootprintEditor.swift`

- [x] As Phase 1, for these files (2026-11-26, same run): `SocketPickerSheet.swift` 0/99,
  `UserLibraryWindow.swift` 0/146, `BlockPreviewView.swift` 0/18, `BundledPartFile.swift` 0/109,
  `CustomFootprintDraft.swift` 0/172, `CustomFootprintEditor.swift` 0/77. Gate: `XB -only-testing`
  PartEditorWindowTests, PartEditorFootprintTests, TestPointTests, ResistorTests, WizardKindStepsTests,
  PartPreviewTests → 48 tests, 0 failures; `SZ` (`node scripts/source-size.js --check`) → exit 0, largest
  touched file 197 code lines.
- [x] ratchet rows: parent
  Ratchet rows: removed by the parent in this letter's commit (8 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: fd6c2af2

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

- (2026-11-26) Logic out of the views, behavior kept: `PartOptionsSection`'s bindings now call
  `PartOptionsModel` (new `Drawer/PartOptionsModel.swift`); `OhmsField` and `CapacitanceField` were the
  same code twice, now one `UnitValueField<Scale>` with its logic in `UnitValueEntry` (new
  `Drawer/UnitValueField.swift`; `ResistorValue` and `CapacitorValue` conform to `UnitValueScale` there);
  `UserLibraryWindowView`'s remove logic is `UserLibraryWindowModel`, and Finder, the alerts, the Add
  Part… panel and the Part Editor are `UserLibraryActions`, injectable (the app passes none and gets the
  old behavior); `SocketPickerSheet.socket(named:in:)`; `CustomFootprintEditor`'s Add Pad, pad removal,
  corner toggles and pin menu are `CustomFootprintDraft.addPad`/`removePad`, `PadDraft.setCorner`/
  `pinChoices`. `TimingSection` is no longer `private`, so a test shows each timer mode.
- (2026-11-26) One timing difference: after Remove… with failures, the selection now drops the asked-about
  rows when the failure alert is dismissed, not before it is shown (the rows themselves leave the table at
  once, as before).
- (2026-11-26) A pad row's remove button (an image only) gets the accessibility label "Remove Pad", so VoiceOver
  and the render test can name it.
- (2026-11-26) `@State private var x = value` lines stay uncovered under xccov even when the view is
  built (the initial value is not counted); the four views with one (`PartOptionsSection`, the text block
  panel, `SocketPickerSheet`, `UserLibraryWindowView`) set it in an explicit `init` instead.
- (2026-11-26) `CustomFootprintDraftTests.testEveryExplicitFootprintRoundTrips` failed before any change:
  the explicit footprint count is 1696, not 1691 (plans 78–82 added five); the count is updated with a note.
- (2026-11-26) The Block pane's render test is `BlockPreviewRenderTests` (`BlockPreviewViewTests` already
  exists, in `PartPreviewTests.swift`).
- (2026-11-26) Test-host lessons: a render test class sets `AppDelegate.quitsAfterLastWindowCloses = false`
  (closing its last window otherwise quits the host mid-suite), and a test that waits for a button's `Task`
  must be synchronous (an `async` test holds the main queue, so the run loop it turns never runs the task).
  ViewProbe gains additive helpers in `Support/ViewProbe+ACControls.swift`: `pressControl` (radio buttons,
  check boxes, link buttons; an unlabelled toggle beside its label), `step`, `wait`, `sheet`, `texts(in:)`,
  `contextMenu(_:row:)` (a table's context menu built from a synthesized right click, not shown) and
  `doubleClick` (a table's primary action).
- (2026-11-26) The shared tree did not compile for long stretches (other letters' files mid-edit), so the
  scoped runs were made in a private rsync snapshot of the worktree (`/tmp/p68-AC-src`) with only those
  other letters' non-compiling *test* files left out of the snapshot.
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
