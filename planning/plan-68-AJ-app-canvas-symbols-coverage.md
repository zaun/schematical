# plan-68-AJ: App coverage — Canvas

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-AI

The app files in § 2 are brought to 100% line coverage: 1234 lines are
uncovered in them today (25 files, 4540 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AI complete (its rows are gone
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
| `Canvas/PartInfoViews.swift` | 63 | 63 |
| `Canvas/PourConnectivity.swift` | 4 | 46 |
| `Canvas/PourSection.swift` | 129 | 233 |
| `Canvas/Ratsnest.swift` | 1 | 532 |
| `Canvas/ReferenceLabel.swift` | 1 | 115 |
| `Canvas/RelayShape.swift` | 0 | 157 |
| `Canvas/RelaySymbolView.swift` | 14 | 219 |
| `Canvas/ResistorSymbolView.swift` | 0 | 46 |
| `Canvas/SchematicCanvasView.swift` | 692 | 1243 |
| `Canvas/SchematicClipboard.swift` | 0 | 118 |
| `Canvas/SchematicExportScene.swift` | 8 | 116 |
| `Canvas/SchematicNote.swift` | 0 | 34 |
| `Canvas/SchematicPainter.swift` | 3 | 134 |
| `Canvas/ScrollPan.swift` | 14 | 38 |
| `Canvas/Selection.swift` | 4 | 83 |
| `Canvas/SelectionList.swift` | 47 | 134 |
| `Canvas/SevenSegmentSymbolView.swift` | 28 | 190 |
| `Canvas/SlideSwitchSymbolView.swift` | 3 | 150 |
| `Canvas/StackupAnalysis.swift` | 1 | 153 |
| `Canvas/StackupSection.swift` | 28 | 169 |
| `Canvas/SupplySymbolView.swift` | 0 | 109 |
| `Canvas/TactileSwitchSymbolView.swift` | 11 | 115 |
| `Canvas/TestPointSymbolView.swift` | 29 | 32 |
| `Canvas/TransistorSymbolView.swift` | 115 | 121 |
| `Canvas/WidthCalculator.swift` | 39 | 190 |
| **Total** | **1234** | **4540** |

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

### Phase 1 — 9 files, `PartInfoViews.swift` … `SchematicCanvasView.swift`

- [ ] Close every uncovered line of these files per § 3; ratchet rows removed.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 16 files, `SchematicClipboard.swift` … `WidthCalculator.swift`

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
