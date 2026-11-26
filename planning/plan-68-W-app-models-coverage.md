# plan-68-W: App coverage — Backend, Routing, Simulation, Memory, Boards, DesignRuleCheck, BillOfMaterials, DocumentInfo, Help

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-V

The app files in § 2 are brought to 100% line coverage: 1069 lines are
uncovered in them today (45 files, 10716 executable lines).

Prerequisites: plan-68-A's table, and plan-68-V complete (its rows are gone
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
| `Backend/HeaderFamily.swift` | 3 | 3 |
| `Backend/PartCatalog.swift` | 116 | 1738 |
| `Backend/PartFacts.swift` | 181 | 731 |
| `Backend/TimerText.swift` | 0 | 49 |
| `Backend/UserPartLibrary.swift` | 6 | 62 |
| `Routing/AssistRouter.swift` | 0 | 82 |
| `Routing/AutorouteModel.swift` | 19 | 110 |
| `Routing/AutorouteSheet.swift` | 56 | 203 |
| `Routing/MovableChains.swift` | 2 | 329 |
| `Routing/RouteCommands.swift` | 3 | 48 |
| `Routing/Router.swift` | 54 | 631 |
| `Routing/RoutingApply.swift` | 9 | 153 |
| `Routing/RoutingCleanup.swift` | 0 | 346 |
| `Routing/RoutingGrid.swift` | 3 | 248 |
| `Routing/RoutingOptions.swift` | 2 | 81 |
| `Routing/RoutingPair.swift` | 4 | 214 |
| `Routing/RoutingPlan.swift` | 6 | 243 |
| `Routing/RoutingPolish.swift` | 2 | 115 |
| `Routing/RoutingProblem.swift` | 3 | 16 |
| `Routing/RoutingProblemBuilder.swift` | 6 | 214 |
| `Routing/RoutingPush.swift` | 0 | 107 |
| `Routing/RoutingRepair.swift` | 14 | 191 |
| `Routing/RoutingSearch.swift` | 5 | 348 |
| `Routing/RoutingSession.swift` | 0 | 77 |
| `Routing/RoutingState.swift` | 2 | 252 |
| `Routing/RoutingTerminals.swift` | 21 | 80 |
| `Routing/RoutingTuning.swift` | 2 | 152 |
| `Routing/RoutingVerifier.swift` | 7 | 161 |
| `Simulation/BuzzerSound.swift` | 60 | 147 |
| `Simulation/Simulation.swift` | 51 | 586 |
| `Simulation/SimulationCommands.swift` | 9 | 36 |
| `Simulation/SimulationEngine.swift` | 69 | 279 |
| `Simulation/VoltageCheck.swift` | 13 | 117 |
| `Memory/LoadFileBar.swift` | 0 | 21 |
| `Memory/MemoryFileAccess.swift` | 34 | 94 |
| `Memory/MemoryWindow.swift` | 103 | 363 |
| `Boards/BoardCard.swift` | 1 | 100 |
| `Boards/BoardCatalog.swift` | 31 | 367 |
| `Boards/BoardExpression.swift` | 3 | 266 |
| `DesignRuleCheck/DesignRuleCheck.swift` | 64 | 774 |
| `DesignRuleCheck/DesignRuleCheckWindow.swift` | 26 | 156 |
| `BillOfMaterials/BillOfMaterialsWindow.swift` | 65 | 239 |
| `DocumentInfo/DocumentInfoWindow.swift` | 12 | 69 |
| `Help/Acknowledgements.swift` | 2 | 63 |
| `Help/KeyboardHelp.swift` | 0 | 55 |
| **Total** | **1069** | **10716** |

## 3. Design: the method

This is the method every app coverage letter (W–Z, AA–AJ) uses; the others
point here.

1. **List.** `node scripts/coverage.js --app --tests <classes> --only <file>`
   prints every uncovered `path:line` of the file under the named test
   classes (plan-68-A § 6). A scoped run can only under-count, so a file that
   shows no uncovered line under the scoped run has none under the full suite.
2. **Close each uncovered block the one way that fits:**
   - *logic in a model, value or controller type*: a unit test calling it.
   - *logic inside a view* (a SwiftUI view's `private func`, an action
     closure, a gesture handler, an `onChange`): move it into a type beside
     the view (`<View>Model` or `<View>Controller`, holding what the view's
     `@State` held, or a pure function), test that type, and have the view
     call it. This is the "logical units that can be tested on their own"
     rule applied to views, and it is also what shrinks
     `SchematicCanvasView` and `PCBCanvasView`.
   - *a SwiftUI `body` or subview*: a render test. Host the view in an
     `NSHostingView` in each state that selects a different branch, lay it
     out (`layoutSubtreeIfNeeded`), and assert on what is shown through the
     accessibility tree (labels, values, enabled state), with a small helper
     `schematicalTests/Support/ViewProbe.swift` (added by plan-68-W) that
     finds elements by label. The precedents host views the same way (12
     test files use `NSHostingView` or `ImageRenderer`).
   - *AppKit windows, panels and menu commands*: create them in the test
     host, drive them through their actions (`sendAction`, the menu item's
     action), assert the window's content, and close every window in
     `tearDown`. A code path that runs a modal (`NSAlert.runModal`, a save
     panel) gets an injectable presenter so the test answers it; no test
     leaves a modal on the owner's screen.
   - *NSView drawing*: render with `cacheDisplay(in:to:)` into a bitmap and
     assert colours at points the drawing code computes.
   - *NSView event handlers*: call `mouseDown(with:)` etc. with events built
     by `NSEvent.mouseEvent(with:location:…)` for the view's window.
   - *network or external tools* (DigiKey connections, KiCad exports): the
     code already takes or gains an injected session or executable path;
     tests use a `URLProtocol` stub or a fake executable, never the network.
   - *unreachable*: delete it or restructure it; the commit message says why
     no caller reaches it. No exclusion list (owner).
3. **Every new test asserts an observable result** — a returned value, a
   label shown, a pixel drawn, a document change. A test that only executes a
   line passes with the line deleted, and is not accepted.
4. **Where tests go.** `schematicalTests/<Type>Tests.swift` for the type under
   test; an existing class when it already covers that type. Every new test
   file stays at or under 800 code lines.
5. **Ratchet.** `COV --app --tests … --only <file> --write-ratchet` lowers or
   removes that file's row only (plan-68-A § 6), in the same commit as the
   tests.
6. **A test that finds a bug** is the bug's regression test: fix it in the
   same letter and list it under Corrections.

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

### Phase 1 — 25 files, `HeaderFamily.swift` … `RoutingState.swift`

- [ ] `schematicalTests/Support/ViewProbe.swift` (§ 3 step 2) with its own tests.
- [ ] Close every uncovered line of these files per § 3; ratchet rows removed.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 20 files, `RoutingTerminals.swift` … `KeyboardHelp.swift`

- [ ] As Phase 1, for these files.

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

(none yet)

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
