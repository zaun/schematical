# plan-68-AA: App coverage — the app root, Preferences

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-Z

The app files in § 2 are brought to 100% line coverage: 727 lines are
uncovered in them today (4 files, 1354 executable lines).

Prerequisites: plan-68-A's table, and plan-68-Z complete (its rows are gone
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
| `ContentView.swift` | 366 | 813 |
| `schematicalApp.swift` | 12 | 120 |
| `Preferences/DesignRulePreferences.swift` | 36 | 65 |
| `Preferences/PreferencesView.swift` | 313 | 356 |
| **Total** | **727** | **1354** |

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

### Phase 1 — `ContentView.swift`, `schematicalApp.swift`, `DesignRulePreferences.swift`, `PreferencesView.swift`

- [x] Close every uncovered line of these files per § 3. `LOGIC_COV_WORK=/tmp/logic-cov-AA node
  scripts/coverage.js --app --tests ContentViewControllerTests,ContentViewRenderTests,PreferencesTests,PreferencesViewTests,EditKeyTests,SchematicNSDocumentTests
  --only schematical/schematical/` → no `path:line` for `ContentView.swift`, `ContentViewController.swift`
  (new, split out of `ContentView.swift`), `schematicalApp.swift`, `Preferences/DesignRulePreferences.swift`,
  `Preferences/PreferencesView.swift` (0 uncovered each). `XB -only-testing` those classes → pass (24 + 14 + 14
  + 8 + EditKeyTests + SchematicNSDocumentTests); the view's other classes (EditMenuTests, NetLabelTests,
  PullResistorTests, DesignRuleSettingsTests, AutomationWindowToolsTests, SelectionSidebarTests,
  FileDesignRulesTests, DesignRuleCheckWizardTests) → 66 tests, 0 failures. `SZ --check` → exit 0.
- [x] ratchet rows: parent (all four rows go to 0: `ContentView.swift`, `schematicalApp.swift`,
  `Preferences/DesignRulePreferences.swift`, `Preferences/PreferencesView.swift`).
  Ratchet rows: removed by the parent in this letter's commit (4 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: 7422c8ec

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

- (AA) `ContentView`'s state and logic moved into `ContentViewController` (new file beside it, an
  `@Observable` held in the view's `@State`): rotate/flip, the editing keys (`handleKey` / `perform`), the
  Edit / Route / Simulation menus' actions, the child windows it opened (one map of closers in place of four
  sets and five flags; the same windows close with the window), the supply, trace layer, cutout and zoom
  menus, the Autoroute sheet, the message expiry and the removal alert's binding. The view only draws it and
  passes method references. `ContentView(controller:)` is added so render tests reach the controller.
  `ContentViewControllerTests` (unit) and `ContentViewRenderTests` (hosted as `SchematicNSDocument` hosts it,
  toolbar, sheets and alerts answered in place) test them. The angle-flash and editor-message delays are
  properties (500 ms and 4 s, as before) so tests can shorten them.
- (AA) `ContentView`'s `#Preview` deleted: no test runs a preview macro, and the line can't be covered.
- (AA) `schematicalApp.swift`: the scene windows' `{ $target in if let target { … } }` closures (run only when
  such a window opens) became `SceneTargetRoot<Content>.init` and `PartEditorSceneRoot.init(target:)`
  references; `SceneTargetRoot` is render-tested. The plain-launch rule became
  `openUntitledIfNeeded(launchEvent:documentCount:isTestHost:newDocument:)`, unit-tested with a stand-in.
- (AA) `DesignRulePreferences.swift`: the Custom field's set closure and Reset button moved to
  `DesignRulePreferences.setCustom` / `resetCustom` (stored defaults injectable), passed by reference.
- (AA) `PreferencesView.swift`: SwiftUI's menu pickers fill their menus only while open, so a test cannot pick
  in them; the pickers' `Binding(get:set:)` fallback logic moved into tested `Binding` helpers (`choice(of:default:)`,
  `within(_:default:)`, `footprintID(in:default:)`, `store`). The private page views became internal for render
  tests; `AutomationPreferencesView` takes its server, folder and pasteboard (defaults: the shared ones) so tests
  never start the real server, write the owner's folder bookmark or the general pasteboard; its status line is
  the pure `statusText(isListening:lastRequest:clientName:callCount:now:)`. Tests that touch the app's own
  preferences put the owner's values back.
- (AA) No bug found. Not a bug: `SimulationEngine` traps on `setSpeed(1e12)` (UInt64 overflow), a speed no menu
  offers; the slow-down test uses 1e5.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
