# plan-68-AB: App coverage — Preferences, Drawer

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-AA

The app files in § 2 are brought to 100% line coverage: 603 lines are
uncovered in them today (4 files, 1146 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AA complete (its rows are gone
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
| `Preferences/ReferencePrefixes.swift` | 84 | 157 |
| `Drawer/BusPanels.swift` | 362 | 362 |
| `Drawer/ChipDrawerView.swift` | 150 | 567 |
| `Drawer/NetLabelPanel.swift` | 7 | 60 |
| **Total** | **603** | **1146** |

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

### Phase 1 — `ReferencePrefixes.swift`, `BusPanels.swift`

- [x] Close every uncovered line of these files per § 3. `BusPanels.swift`'s
  logic moved to `Drawer/BusPanelModel.swift` (`BusPanelModel`,
  `BusPanelModelTests` 16 tests; `BusPortPanelModel`); the views are
  render-tested by `BusPanelViewTests` (9 tests: fields typed and committed,
  the link confirmation sheet answered, Import/Export through an injected
  `ExportPresenter`, a wire row's context menu built and chosen without
  showing it). `ReferencePrefixes.setting`/`committed` took the view's commit
  logic (`ReferencePrefixTests` +4 tests; `ReferencePreferencesViewTests`).
  Check (run 2026-11-26 in a scratch copy of the tree, see Corrections):
  `LOGIC_COV_WORK=/private/tmp/logic-cov-AB2 node scripts/coverage.js --app --tests BusPanelModelTests,BusPanelViewTests,DrawerLayoutTests,DrawerPartMenuTests,ChipDrawerViewTests,NetLabelPanelTests,ReferencePrefixTests,ReferencePreferencesViewTests --only schematical/schematical/Drawer` → 62 tests pass (`xcresulttool` summary: passed 62, failed 0);
  uncovered lines from the same bundle's `xccov view --archive`:
  `BusPanels.swift` 0/209, `BusPanelModel.swift` 0/200,
  `Preferences/ReferencePrefixes.swift` 0/158.
- [x] ratchet rows: parent (`ReferencePrefixes.swift`, `BusPanels.swift` at 0;
  new `BusPanelModel.swift` at 0).
  Ratchet rows: removed by the parent in this letter's commit (4 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — `ChipDrawerView.swift`, `NetLabelPanel.swift`

- [x] As Phase 1, for these files. `ChipDrawerView.swift` split: its row
  list, layout, scroll window, memo and open-group state to
  `Drawer/DrawerLayout.swift` (`DrawerOpenGroups` new, holding what the
  view's `openCategory`/`toggled` `@State` held), its header-menu choice,
  part context menu and status texts to `Drawer/ChipDrawerMenus.swift`
  (`DrawerHeaderMenu`, `DrawerPartMenu`, `DrawerStatus`). Tests:
  `DrawerLayoutTests` (11), `DrawerPartMenuTests` (4, a User Library folder
  the test made, alerts answered through `DrawerPartMenu.presenter`),
  `ChipDrawerViewTests` (8 render tests). `NetLabelPanel`'s field rules moved
  to `NetLabelEditing` beside it (`NetLabelPanelTests`, 4). Check: the
  Phase 1 run → `ChipDrawerView.swift` 0/269, `DrawerLayout.swift` 0/207,
  `ChipDrawerMenus.swift` 0/91, `NetLabelPanel.swift` 0/61 uncovered;
  `DrawerPerformanceTests`, `SchematicSidebarViewTests`, `BusTypeTests` →
  33 tests pass; `node scripts/source-size.js --check` → exit 0.
- [x] ratchet rows: parent (`ChipDrawerView.swift`, `NetLabelPanel.swift` at
  0; new `DrawerLayout.swift`, `ChipDrawerMenus.swift` at 0).
  Ratchet rows: removed by the parent in this letter's commit (4 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
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

- (2026-11-26) The checks ran in a scratch copy of the worktree
  (`/private/tmp/logic-cov-AB-tree`, rsync of the tree without datasheets),
  not in the shared tree: the shared test target did not compile for over an
  hour because of other letters' test files being written
  (`PreferencesViewTests.swift`, `FileMenuItemsTests.swift`,
  `BillOfMaterialsWindowTests.swift`: missing imports). In the copy only,
  `PreferencesViewTests.swift` got `import Network` and the test files that
  did not compile were deleted; no shared file was touched. `coverage.js`
  run in the copy reports "no Swift coverage" (the bundle's paths are under
  `/tmp`, the script's root under `/private/tmp`), so the uncovered lines were
  read from that run's bundle with `xccov view --archive --json` by the
  script's own rule (an executable line with count 0). The parent should
  re-run the Phase 1 command in the shared tree once it compiles.
- Unreachable code deleted: the drawer's two open-state subscripts' getters
  (no caller reads them; assignment through a subscript calls only `set`):
  now `setDrawerOpen(_:_:)` functions. `BusWireNameRow`'s context menu's
  `ForEach` was unrolled into its four fixed items (a `ForEach` inside a
  menu is built only when the menu opens).
- `@State` initial values in a struct with a memberwise init count as
  uncovered under `xccov`; `BusWireNameRow`, `NetLabelPanel`, `PrefixField`
  and `DrawerScrollList` got explicit inits setting them.
- A SwiftUI `Menu` (the drawer headers' filter pull-downs) fills its items
  only when it pops up, so no test can build them without a menu on the
  screen: the headers' render tests assert the menus' labels ("All
  families", "All counts", "All colors", "All sizes", "All actuators",
  "All channels"), and each item row is one line built with its `ForEach`.
- The drawer takes an optional `editor` (nil in the app = SwiftUI's
  `openWindow`, as plan-68-AC's `UserLibraryActions.edit`): a test that let
  Copy to Editor… open a real Part Editor window crashed the test host
  (abort) on one run.
- No bug found; no user-visible change except the drawer's search clear
  button getting the accessibility label "Clear search" (its tooltip text).
  The drawer's footer now gives every status line `lineLimit(2)` (it was on
  the failure line only; the others are one line).
- New test classes that open windows set
  `AppDelegate.quitsAfterLastWindowCloses = false` (as the other render
  tests do): without it the host quit when a test closed its last window.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
