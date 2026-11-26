# plan-68-AG: App coverage — Editor, Canvas

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-AF

The app files in § 2 are brought to 100% line coverage: 836 lines are
uncovered in them today (40 files, 4156 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AF complete (its rows are gone
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
| `Editor/Wizard/ReviewStepPage.swift` | 100 | 126 |
| `Editor/Wizard/VariantStepPage.swift` | 52 | 65 |
| `Editor/Wizard/WizardIssueLocator.swift` | 5 | 102 |
| `Editor/Wizard/WizardIssues.swift` | 10 | 84 |
| `Editor/Wizard/WizardNavBar.swift` | 21 | 21 |
| `Editor/Wizard/WizardStepList.swift` | 45 | 45 |
| `Editor/Wizard/WizardStepView.swift` | 71 | 71 |
| `Editor/Wizard/WizardSteps.swift` | 1 | 101 |
| `Canvas/BarrelJackSymbolView.swift` | 5 | 79 |
| `Canvas/BoardColors.swift` | 6 | 38 |
| `Canvas/BoardDetailsSheet.swift` | 25 | 92 |
| `Canvas/BoardHole.swift` | 17 | 69 |
| `Canvas/BoardModel.swift` | 6 | 217 |
| `Canvas/BoardPour.swift` | 6 | 39 |
| `Canvas/BoardSettings.swift` | 33 | 281 |
| `Canvas/BoardTrace.swift` | 10 | 188 |
| `Canvas/BusModel.swift` | 85 | 635 |
| `Canvas/BusWireList.swift` | 0 | 61 |
| `Canvas/BuzzerSymbolView.swift` | 10 | 75 |
| `Canvas/CanvasViewport.swift` | 20 | 66 |
| `Canvas/CapacitorSymbolView.swift` | 11 | 67 |
| `Canvas/CapacitorValue.swift` | 1 | 47 |
| `Canvas/CharacterLCDSymbolView.swift` | 4 | 172 |
| `Canvas/ChipBlockView.swift` | 23 | 149 |
| `Canvas/CrystalSymbolView.swift` | 9 | 74 |
| `Canvas/CutoutSection.swift` | 45 | 71 |
| `Canvas/DekatronSymbolView.swift` | 100 | 100 |
| `Canvas/DiodeSymbolView.swift` | 17 | 140 |
| `Canvas/DipSwitchSymbolView.swift` | 14 | 93 |
| `Canvas/ErcMarkerLayer.swift` | 15 | 23 |
| `Canvas/FuseSymbolView.swift` | 10 | 68 |
| `Canvas/GateSymbolView.swift` | 10 | 189 |
| `Canvas/HeaderSymbolView.swift` | 10 | 72 |
| `Canvas/InductorSymbolView.swift` | 9 | 47 |
| `Canvas/InductorValue.swift` | 0 | 26 |
| `Canvas/LengthField.swift` | 6 | 62 |
| `Canvas/NetClass.swift` | 5 | 42 |
| `Canvas/NetLabel.swift` | 4 | 123 |
| `Canvas/NetList.swift` | 2 | 101 |
| `Canvas/NetRules.swift` | 13 | 35 |
| **Total** | **836** | **4156** |

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

### Phase 1 — 7 files, `ReviewStepPage.swift` … `WizardStepView.swift`

- [x] Close every uncovered line of these files per § 3. New class
  `WizardReviewPagesTests` (render tests of Review's still-to-do list and
  Go to buttons, its verdict in each state — checking, loads, rejected with
  the step it points at — Save Part… with its panel answered, Add to Library
  into the test's own User Library, the previews and the pins-and-pads rows;
  the variant page and its "Change to …?" alert; Back / Skip / Next; the
  step list's marks and row selection; every page of `WizardStepView`, its
  issues, open error and Find in datasheet; `Field.issues` for every value
  shape; the locator's common-key rules; a guide with no field guidance).
  The final scoped run (Phase 2's command) → no `path:line` and no table
  row for `ReviewStepPage.swift`, `VariantStepPage.swift`,
  `WizardIssueLocator.swift`, `WizardIssues.swift`, `WizardNavBar.swift`,
  `WizardStepList.swift`, `WizardStepView.swift` (exit 0).
- [x] ratchet rows: parent (the seven files at 0).
  Ratchet rows: removed by the parent in this letter's commit (38 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 33 files, `WizardSteps.swift` … `NetRules.swift`

- [x] As Phase 1, for these files. New classes `CanvasModelCoverageTests`
  (every board-setting, hole, pour, rule and net-label title; text colours;
  the rules' "(modified)" title; an outline off the origin; reference
  prefixes; hole rings, opposite pads, tall slots, pad bounds, pin-1 marks,
  the Pad 1 Center fallback, a round body's cathode band; the assist leg
  lock; the board clipboard; net class and rule defaults; the net list's
  via, pad and unnamed-net lookups; the viewport's zoom steps and pinch; a
  bus's contacts, joints, slides, line moves and end drags, and a refused
  join), `CanvasSymbolRenderTests` (each symbol view rendered plain,
  selected and hovered as the canvas builds it, the bitmaps compared; a
  recording painter for each drawing's branches: the oscillator and jack
  glyphs, diode marks and a lit LED, every light colour, a jack's switch and
  turned labels, a female header, capacitor plates and value, a crystal's
  ground lead, a dekatron's glow; the timer warning's label) and
  `CanvasPanelsRenderTests` (a length field's Return, clamp, bad text, blur
  and live modes; the Cutout panel for a rounded cutout and a polygon; Board
  Details' Planes, Card, Net Classes, Net Groups and Stackup tabs, Done, and
  Route Net from a net's menu; ERC markers and net labels in pixels). With
  `WizardReviewPagesTests`, the existing Canvas and Wizard classes and the
  classes that host the changed views: `LOGIC_COV_WORK=/tmp/logic-cov-AG node scripts/coverage.js --app --tests RelayBuzzerTests,CharacterLCDTests,GateSymbolTests,InductorFuseTests,ResistorTests,SlideSwitchTests,BoardDetailsSheetTests,BoardFeaturesTests,HoleTests,CutoutPourTests,PourFillTests,PourModelTests,BoardSettingsEditTests,BoardStackupTests,BoardLayersTests,BoardCopperTests,TraceRouteTests,NetClassTests,ViaTests,BusPanelModelTests,BusWireListTests,BusTypeTests,BusNetworkTests,BusJoinTests,ErcMarkerTests,CapacitorTests,PartPreviewTests,PinNameOverlineTests,CutoutPanelTests,BoardGridTests,NetRulesTests,NetListTests,NetLabelTests,NetLabelsTests,StackupAnalysisTests,TraceAngleTests,TraceJoinTests,TraceToolTests,DisplayReferenceTests,BoardSideTests,ReferencePositionTests,BoardVisibilityTests,BoardTextsTests,BoardCardTests,CutoutModelTests,PourCanvasTests,AddOnBoardTests,HeaderBoardTests,SchematicSceneTests,WizardReviewPagesTests,CanvasModelCoverageTests,CanvasSymbolRenderTests,CanvasPanelsRenderTests,WizardStepsTests,WizardKindStepsTests,WizardIssueLocatorTests,CathodeBandTests,OppositePadTests,BoardPreviewTests,GerberExportTests,PCBSidebarViewTests,PreferencesViewTests,BlockPreviewRenderTests --only schematical/schematical/` → no `path:line` and no table
  row for any file in § 2 (exit 0; 748 tests, all passed).
  `node scripts/source-size.js --check` → exit 0.
- [x] ratchet rows: parent (the 33 files at 0).
  Ratchet rows: removed by the parent in this letter's commit (38 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: —

## Validation Plan

- [x] Final gate (2026-11-26): the scoped coverage run above ran all of
  this letter's classes with the existing ones and those hosting the
  changed views (`PCBSidebarViewTests`, `PreferencesViewTests`,
  `BlockPreviewRenderTests`, `CathodeBandTests`, `OppositePadTests`,
  `BoardPreviewTests`, `GerberExportTests`) → 748 tests passed, 0 failed;
  `SZ` → exit 0.

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

- (2026-11-26) Initial state moved into explicit `init`s, as xccov never
  counts a `@State` default: `LengthField` (`text`, `shown`) and
  `BoardDetailsSheet` (`rows`, `hasBuilt`, and `tab`, now an init
  argument). Behavior unchanged.
- (2026-11-26) `PlacedChip.cathodeBand`'s geometry is
  `BoardLayout.cathodeBand(of:on:cathode:anode:)`, so a round body (only a
  user part's custom footprint has one) is tested directly. Behavior
  unchanged.
- (2026-11-26) Unreachable code removed: `Bus.joints(of:)` and
  `DiodeSymbolView.lightColor(for:)` had no caller; a header's label no
  longer falls back to the part's first gender (`PlacedChip.init` always
  chooses one for a header with genders); an oscillator block's "glyph with
  no frequency" branch is folded into the one path (the loader keeps every
  oscillator's `defaultHz` at least `minHz` ≥ 1), drawing the same.
- (2026-11-26) No bug found.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
