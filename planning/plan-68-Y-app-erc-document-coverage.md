# plan-68-Y: App coverage — ElectricalRulesCheck, Document

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-X

The app files in § 2 are brought to 100% line coverage: 941 lines are
uncovered in them today (29 files, 7386 executable lines).

Prerequisites: plan-68-A's table, and plan-68-X complete (its rows are gone
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
| `ElectricalRulesCheck/ElectricalRulesCheckWindow.swift` | 199 | 214 |
| `ElectricalRulesCheck/ErcChecker.swift` | 1 | 222 |
| `ElectricalRulesCheck/ErcDrawingRules.swift` | 7 | 167 |
| `ElectricalRulesCheck/ErcResultsModel.swift` | 0 | 58 |
| `ElectricalRulesCheck/ErcSettings.swift` | 28 | 77 |
| `Document/ChildWindows.swift` | 16 | 94 |
| `Document/DXFImportPlan.swift` | 1 | 63 |
| `Document/DXFImportSheet.swift` | 35 | 109 |
| `Document/DXFImportState.swift` | 0 | 71 |
| `Document/DXFLoops.swift` | 7 | 218 |
| `Document/DXFReader.swift` | 1 | 197 |
| `Document/EasyEDAProBoardWriter.swift` | 2 | 262 |
| `Document/EasyEDAProExport.swift` | 5 | 162 |
| `Document/EasyEDAProSchematicWriter.swift` | 3 | 397 |
| `Document/EditCommands.swift` | 25 | 98 |
| `Document/GerberExport.swift` | 9 | 255 |
| `Document/GerberJob.swift` | 0 | 63 |
| `Document/GerberWriter.swift` | 7 | 274 |
| `Document/ImageExport.swift` | 23 | 187 |
| `Document/ImageExportDialog.swift` | 120 | 120 |
| `Document/KiCadBoardWriter.swift` | 28 | 553 |
| `Document/KiCadExport.swift` | 7 | 319 |
| `Document/KiCadSchematicWriter.swift` | 7 | 707 |
| `Document/PrintDialog.swift` | 8 | 112 |
| `Document/PrintExport.swift` | 16 | 437 |
| `Document/SaveFormat.swift` | 0 | 14 |
| `Document/SchematicExportCommands.swift` | 12 | 33 |
| `Document/SchematicExporter.swift` | 299 | 401 |
| `Document/SchematicFile.swift` | 75 | 1502 |
| **Total** | **941** | **7386** |

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

### Phase 1 — 20 files, `ElectricalRulesCheckWindow.swift` … `ImageExportDialog.swift`

- [ ] Close every uncovered line of these files per § 3; ratchet rows removed.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 9 files, `KiCadBoardWriter.swift` … `SchematicFile.swift`

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
