# plan-68-AF: App coverage — Editor

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-AE

The app files in § 2 are brought to 100% line coverage: 742 lines are
uncovered in them today (12 files, 1157 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AE complete (its rows are gone
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
| `Editor/Wizard/FootprintStepPage.swift` | 265 | 308 |
| `Editor/Wizard/FootprintThumbnail.swift` | 156 | 176 |
| `Editor/Wizard/Guidance.swift` | 30 | 50 |
| `Editor/Wizard/Guides/DffGuide.swift` | 1 | 12 |
| `Editor/Wizard/Guides/GuideLayout.swift` | 1 | 99 |
| `Editor/Wizard/Guides/GuideParts.swift` | 0 | 34 |
| `Editor/Wizard/Guides/JkffGuide.swift` | 1 | 13 |
| `Editor/Wizard/Guides/VariantKindGuides.swift` | 4 | 26 |
| `Editor/Wizard/IdentityStepPage.swift` | 73 | 77 |
| `Editor/Wizard/KindStepPage.swift` | 85 | 93 |
| `Editor/Wizard/PinTableParser.swift` | 9 | 124 |
| `Editor/Wizard/PinoutStepPage.swift` | 117 | 145 |
| **Total** | **742** | **1157** |

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

### Phase 1 — `FootprintStepPage.swift`, `FootprintThumbnail.swift`, `Guidance.swift`

- [x] Close every uncovered line of these files per § 3. New classes
  `FootprintStepPageTests` (render tests of every tab and style, the
  preview's three states, the model calls behind the buttons),
  `FootprintThumbnailTests` (the thumbnail and each sketch in pixels) and
  `FieldGuideViewTests` (the guidance view, `whereLine`, the shared-guide
  fallback). `LOGIC_COV_WORK=/tmp/logic-cov-AF node scripts/coverage.js
  --app --tests FootprintStepPageTests,FootprintThumbnailTests,FieldGuideViewTests,WizardStepPagesTests,KindGuideTests,PartEditorFootprintTests,ParametricFootprintDraftTests,PinTableParserTests,WizardKindStepsTests,WizardIdentityTests,WizardModelTests,WizardStepsTests,WizardStepPagesTests --only
  schematical/schematical/Editor/Wizard` → no `path:line` and no table row for
  `FootprintStepPage.swift`, `FootprintThumbnail.swift`, `Guidance.swift`
  (exit 0; all tests pass).
- [x] ratchet rows: parent (the three files at 0).
  Ratchet rows: removed by the parent in this letter's commit (11 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: 72c041a3

### Phase 2 — 9 files, `DffGuide.swift` … `PinoutStepPage.swift`

- [x] As Phase 1, for these files. New class `WizardStepPagesTests` (the
  Kind step's search, kind rows and "Change to …?" alert; the Part number
  step's fields, menus and More options; the Pins step's paste, columns,
  Apply / Replace and numbered start; `PastedPinTable`, `requestKind`,
  `applyPins` with no pinout); `FieldGuideViewTests` adds
  `overridden(by:)` and `GuideLayout.isPinsOnly(.listOf)`. The same scoped
  run → no `path:line` and no table row for `DffGuide.swift`,
  `GuideLayout.swift`, `GuideParts.swift`, `JkffGuide.swift`,
  `VariantKindGuides.swift`, `IdentityStepPage.swift`, `KindStepPage.swift`,
  `PinTableParser.swift`, `PinoutStepPage.swift`. `node scripts/source-size.js
  --check` → exit 0.
- [x] ratchet rows: parent (the nine files at 0).
  Ratchet rows: removed by the parent in this letter's commit (11 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: 72c041a3

## Validation Plan

- [x] Final gate (2026-11-26): `XB -only-testing` the 12 classes above plus
  `PartEditorWindowViewTests` (it hosts `PinNameField`, whose init changed)
  → `** TEST SUCCEEDED **`, 127 tests, 0 failures; `SZ` → exit 0.

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

- (2026-11-26) Logic moved out of views into tested types beside them:
  the ball grid's "Use the pinout's balls" and the header's row picker are
  `ParametricFootprintDraft.useBalls(_:)` and `setHeaderRows(_:)`
  (`FootprintStepPage.swift`); a kind row's press is
  `PartEditorModel.requestKind(_:)` (`KindStepPage.swift`); the Pins step's
  pasted text, parsed table and columns are `PastedPinTable`
  (`PinoutStepPage.swift`; the same text pasted again keeps the user's
  columns, as the old `onChange` did). Behavior unchanged.
- (2026-11-26) Paste on the Pins step reads an injected `pasteboard`
  (default `.general`), so the test never touches the owner's clipboard.
- (2026-11-26) Unreachable code restructured: the guides' `merging(…) { _,
  new in new }` closures ran only on a key collision, which no guide has;
  they are `Dictionary.overridden(by:)` (`GuideParts.swift`, tested with a
  collision).
- (2026-11-26) As plan-68-AB/AC found, a `@State` default is never counted
  by xccov: `FootprintStepPage`, `KindStepPage`, `IdentityStepPage`,
  `PinoutStepPage` and `PinNameField` set their state in an explicit `init`.
  The Part number step's two pull-down menus build their items only as they
  open on screen, so each item row is one line (the render test asserts the
  menus' labels; the maker and package are typed instead).
- (2026-11-26) Render tests host the pages in a plain scrolling stack, not
  the grouped `Form` the wizard uses: a grouped form builds only the rows on
  screen, and its pop-ups' items are empty until opened.
- (2026-11-26) The Preview's "drawn here once the part loads" branch is
  shown, deterministically, for a valid part with no footprint (a copy of
  the 5V supply); otherwise it showed only while the first check ran.
- No bug found.
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
