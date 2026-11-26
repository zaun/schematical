# plan-68-AI: App coverage — Canvas

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-AH

The app files in § 2 are brought to 100% line coverage: 716 lines are
uncovered in them today (1 files, 990 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AH complete (its rows are gone
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
| `Canvas/PCBSidebarView.swift` | 716 | 990 |
| **Total** | **716** | **990** |

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

### Phase 1 — `PCBSidebarView.swift`

- [x] Close every uncovered line of these files per § 3. Step 1 under the
  existing classes (`BoardDetailsSheetTests,SelectionSidebarTests,StackupAnalysisTests`):
  724 of 990 lines uncovered (`PCBSidebarPanels.swift` 447, `PCBSidebarView.swift` 277).
  New classes `PCBSidebarViewTests` (16 tests: units, grid, size, type, layer
  count and layer type menus with their refusals, colour swatches and wells,
  Board Details…, card bus, fixed-size and free-form boards, routing of every
  single selection, several texts, several footprints with marks and Reset
  Reference, the selection list, `CardOptionsSection`, `CustomCardSection`,
  the extracted helpers) and `PCBSidebarPanelsTests` (8 tests: the text, via,
  hole, trace, lone trace, footprint, connector-style and Planes panels), plus
  additive helpers in `schematicalTests/Support/ViewProbe+AI.swift`.
  `LOGIC_COV_WORK=/tmp/logic-cov-AI node scripts/coverage.js --app --tests
  PCBSidebarViewTests,PCBSidebarPanelsTests,BoardDetailsSheetTests,SelectionSidebarTests,StackupAnalysisTests
  --only schematical/schematical/Canvas/PCBSidebar` → `app: 1016 lines, 0
  uncovered, 100.00%` (no `path:line`).
- [x] ratchet rows: parent (both files at 0).
  Ratchet rows: removed by the parent in this letter's commit (2 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

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

- (AI) A menu-style `Picker` inside a `.formStyle(.grouped)` Form never fills
  its `NSPopUpButton` menu in the test host (`menuNeedsUpdate`,
  `menuWillOpen`, `attachPopUp` and `willOpenMenu` all leave it empty); the
  same picker under `.formStyle(.columns)` or in a `VStack` fills. The
  sidebar's own Form is grouped, so the tests that choose from a menu host
  the panel (`BoardPanel` + `LayersSection`, `HolePanel`, `TracePanel`,
  `FootprintPanel`, `PlanesSection`, `CardOptionsSection`) in a columns Form;
  the sidebar itself is still rendered for routing and the grouped-only
  controls. (`NetsSidebarViewTests` and `CanvasPanelsViewTests` choose from
  grouped-Form pop-ups and fail the same way in today's tree.)
- (AI) Logic out of the views, keeping behaviour: `PCBSidebarView.shared(_:)`
  (the one value a selection shares, nil when mixed: the side, layer, size and
  mark pickers), `PCBSidebarView.mil(_:)` (a typed size or width, used by
  `MultiTextSection`, `TextPanel`, `TracePanel`),
  `PCBSidebarView.footprintIDs(of:)` (the selection's placed footprints, was
  inline in `body`) and `MultiPositionSection.side(of:in:)` (was a private
  method); unit-tested in `testSharedMilAndSideHelpers`.
- (AI) xccov never counts a `@State` (or stored property) default value, so
  `PCBSidebarView`, `MultiPositionSection`, `MultiTextSection`, `TextPanel` and
  `TracePanel` set their initial state in an explicit `init` (same values and
  parameter defaults as before).
- (AI) No bug found.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
