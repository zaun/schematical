# plan-68-AE: App coverage — Editor

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-AD

The app files in § 2 are brought to 100% line coverage: 647 lines are
uncovered in them today (4 files, 720 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AD complete (its rows are gone
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
| `Editor/PartEditorSections.swift` | 181 | 205 |
| `Editor/PartEditorWindow.swift` | 333 | 334 |
| `Editor/PartValidator.swift` | 13 | 61 |
| `Editor/Wizard/DatasheetStepPage.swift` | 120 | 120 |
| **Total** | **647** | **720** |

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

### Phase 1 — `PartEditorSections.swift`, `PartEditorWindow.swift`

- [x] Close every uncovered line of these files per § 3.
  New `PartEditorWindowViewTests` (scene root, both modes and the toolbar in a
  bridged window, the preview panes, the form, footprint choices, the datasheet
  menu) and `PartEditorSectionsTests` (Identity, the verdict, Footprint and Fits,
  Supply, the kind's fields, pinouts, the pin table, the active-low menu).
  `LOGIC_COV_WORK=/tmp/logic-cov-AE node scripts/coverage.js --app --tests PartEditorWindowViewTests,PartEditorSectionsTests,DatasheetStepPageTests,PartValidatorTests,PartEditorWindowTests,PartEditorModelTests,WizardModelTests --only schematical/schematical/Editor` → no `path:line` for `PartEditorSections.swift` (was 181) or
  `PartEditorWindow.swift` (was 333); the AE classes pass.
- [x] ratchet rows: parent (`PartEditorSections.swift`, `PartEditorWindow.swift` at 0)
  Ratchet rows: removed by the parent in this letter's commit (4 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: d68b6f4a

### Phase 2 — `PartValidator.swift`, `DatasheetStepPage.swift`

- [x] As Phase 1, for these files. New `DatasheetStepPageTests` (every branch of
  the step and the pane, the address rule, Open a downloaded PDF… through the
  presenter, a download that hangs until cancelled); `PartValidatorTests`
  extended (a setup failure, the kept lists, a draft that cannot be written, a
  refused text, the async check). Same command → no `path:line` for
  `PartValidator.swift` (was 13) or `DatasheetStepPage.swift` (was 120).
  `node scripts/source-size.js --check` → exit 0.
- [x] ratchet rows: parent (`PartValidator.swift`, `DatasheetStepPage.swift` at 0)
  Ratchet rows: removed by the parent in this letter's commit (4 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: d68b6f4a

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

- (AE) Logic moved out of views into tested functions beside them, behavior
  unchanged: `PartEditorSceneRoot.open(_:in:)` (onOpenURL);
  `FootprintChoicesEditor.adding/removing`; `DatasheetField.matches`;
  `PinoutsSection.add/remove`; `PinoutEditor.rename(to:)`, `commitPinName`,
  `addPin(to:)`, `move(_:by:in:)`; `ActiveLowMenu.label(for:)`, `set(_:activeLow:on:)`;
  `PartEditorModel.optionalText(_:)`, `customFootprintBinding`,
  `startCustomFootprint()` (Custom Footprint…), `commitDatasheetAddress(_:replacing:)`
  (the Datasheet step's `commit()`).
- (AE) `VerdictSection` takes `validation` and `status` instead of the model, so
  its "Checking…" state (before the first check) can be shown.
- (AE) `PartEditorWindowView.init(model:target:)` added, so a test hosts the
  window with its presenter answered.
- (AE) Icon-only buttons (Remove Choice, Remove Pinout, Move Up/Down, Remove
  Pin) are `Label(…).labelStyle(.iconOnly)` instead of a bare `Image`: they look
  the same and now have an accessibility name.
- (AE) `openLocalDatasheet` runs its open panel through `presenter.choose`
  (it ran the panel itself, untestable and on the owner's screen).
- (AE) `PartValidator.init(loadSupport:)` takes the support-file loader, so the
  setup-failure path is tested. Its `guard count > 0, let loaded … else
  { "the part did not load" }` was unreachable (`load_part_file` succeeded, so
  the library holds the part it just added) and is now a force-unwrap with that
  reason in a comment.
- (AE) `DatasheetStepPage` sets its two `@State` values in an explicit `init`:
  the property initializers (`= ""`, `= false`) counted as never run even with
  the view rendered.
- (AE) The shared worktree did not build for long stretches (other letters'
  in-progress files: `BillOfMaterialsWindowTests.swift` missing
  `import UniformTypeIdentifiers`, then `MemoryFileAccessTests.swift`). The
  coverage runs were made in an APFS clone of the worktree (`/tmp/ae-copy`,
  this letter's files identical to the worktree's, the BOM import added there
  only), run with `node --preserve-symlinks-main` from `/tmp/ae-copy` so
  coverage.js's root matches xccov's paths.
- (AE) Seen, not this letter's: `PartEditorModelTests.testTheMenuItemsFollowTheFocusedEditor`
  (plan-68-AD) fails on its own (`-only-testing` that one test → panels 0,
  alerts []). It is an `async` test: a button whose action starts a `Task`
  cannot run it while the test holds the main actor in `settle()`. The same
  pattern made this letter's first Open-a-PDF test fail; it is a sync test now.
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
