# plan-68-X: App coverage — Registers, Cpu, Terminal, Automation

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-W

The app files in § 2 are brought to 100% line coverage: 911 lines are
uncovered in them today (11 files, 3168 executable lines).

Prerequisites: plan-68-A's table, and plan-68-W complete (its rows are gone
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
| `Registers/RegisterWindow.swift` | 125 | 133 |
| `Cpu/CpuWindow.swift` | 164 | 240 |
| `Terminal/TerminalWindow.swift` | 189 | 189 |
| `Automation/AutomationBoard.swift` | 61 | 513 |
| `Automation/AutomationFolder.swift` | 41 | 80 |
| `Automation/AutomationRead.swift` | 38 | 511 |
| `Automation/AutomationSchematic.swift` | 74 | 803 |
| `Automation/AutomationServer.swift` | 121 | 179 |
| `Automation/AutomationSession.swift` | 38 | 158 |
| `Automation/AutomationSimulation.swift` | 41 | 124 |
| `Automation/AutomationTools.swift` | 19 | 238 |
| **Total** | **911** | **3168** |

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

### Phase 1 — `RegisterWindow.swift`, `CpuWindow.swift`, `TerminalWindow.swift`

- [x] Close every uncovered line of these files per § 3. New classes
  `RegisterWindowTests`, `CpuWindowTests`, `TerminalWindowTests`; the view
  bodies moved into `RegisterStateView`, `CpuStateView` (render tests with
  hand-built snapshots) and `TerminalController` (unit tests on a powered
  XR21V1410 with TX wired to RX, so typed text loops back); `CpuInspector`
  takes an injectable `chooseFile` (the test answers the open panel).
  Check: `LOGIC_COV_WORK=/tmp/logic-cov-X node scripts/coverage.js --app
  --tests RegisterWindowTests,CpuWindowTests,TerminalWindowTests,AutomationServerTests,AutomationSessionToolTests,AutomationEditToolTests,AutomationTests,AutomationWindowToolsTests,CutoutAutomationTests,SplitPartAutomationTests,SocketModeTests,AvrAppTests,DisplayDriverTests,SerialPortTests --only schematical/schematical/` → no `path:line` under
  `Registers/`, `Cpu/`, `Terminal/` (RegisterWindow 0/135, CpuWindow 0/243,
  TerminalWindow 0/207); the run's 164 test cases all pass.
- [x] ratchet rows: parent (all three files at 0).
  Ratchet rows: removed by the parent in this letter's commit (12 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: e437c77c

### Phase 2 — 8 files, `AutomationBoard.swift` … `AutomationTools.swift`

- [x] As Phase 1, for these files (`AutomationSchematic.swift`'s row
  stands for it and `AutomationSchematicEdits.swift`). New classes
  `AutomationServerTests` (MCP answers, a listener of the test's own on port
  0, the shared folder), `AutomationSessionToolTests` (plumbing, files,
  exports, simulation tools) and `AutomationEditToolTests` (schematic, board
  and read tools). Same check as Phase 1 → no `path:line` under
  `Automation/`: Board 0/514, Folder 0/88, Read 0/509, Schematic 0/203,
  SchematicEdits 0/599, Server 0/197, Session 0/160, Simulation 0/124,
  Tools 0/229. `node scripts/source-size.js --check` → exit 0 (new test
  files staged; the largest, `AutomationEditToolTests.swift`, 361 lines).
- [x] ratchet rows: parent (all eight files at 0).
  Ratchet rows: removed by the parent in this letter's commit (12 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: e437c77c

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

- (plan-68-X) Bug, fixed: the terminal's Clear did not redraw the
  transcript (the log is a plain class SwiftUI does not observe, so the
  cleared text stayed on screen until the next simulation step changed
  `stateVersion`). `TerminalController.clear()` now bumps an observed
  counter the transcript reads. Regression test:
  `TerminalWindowTests.testWindowTypesAndShowsTheTranscript`.
- (plan-68-X) `TerminalLog` had no `nonisolated deinit`: a test freeing one
  crashed the host. Added; `AutomationServer`, `AutomationFolder` and the
  new `TerminalController` have one too, each covered by a test that frees one.
- (plan-68-X) Restructured, not just tested: `AutomationServer` is no longer
  hard-wired to `shared` and port 47800 (`init(port:)`, `listeningPort`,
  an injectable `makeListener`, `respond(to:)` and `http` internal, the
  listener's state mapped to its own `ListenerState` so callers need no
  `import Network`); `AutomationFolder.choose()` shows its panel through
  `present` (default `runModal`, tested by aborting the real panel at once);
  `new_window`'s wait is `AutomationTools.newSession(besides:tries:)`.
- (plan-68-X) Deleted as unreachable: `AutomationArgs.point(_:)` (no
  caller); `choice(_:among:default:)`'s missing-argument throw (every caller
  passes a default, now required); `move_part`'s "was not moved" throw (the
  part and unit still exist after `moveUnit`); `render`'s second "nothing to
  draw" throw (merged with the first: `data` is nil only when `pixelSize` is).
  `set_pour`'s properties no longer use `merging` with a closure no key
  reached (`pourProperties(needsPlace:id:)`).
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
