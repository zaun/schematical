# plan-68-Y: App coverage — ElectricalRulesCheck, Document

Last updated: 2026-11-26
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

- [x] Close every uncovered line of these files per § 3 — done; ratchet rows: parent (every file at 0).
  Classes: ElectricalRulesCheckWindowTests, ErcResultsModelTextTests, SchematicExporterTests, PartFileLoadingTests, FileMenuItemsTests, ExportSheetsTests, SchematicFileErrorTests, SchematicFileReadTests (new); DXFReaderTests, ErcDrawingChecksTests, EasyEDAProExportTests, GerberAttributeTests, GerberExportTests, KiCadExportTests, PrintExportTests (extended), run with the 66 existing classes that test these files
  (the list is `--tests` below). Check, 2026-11-26: the scoped run of all 74
  classes → 899 passed, 0 failed, 1 skipped (`testClosingAKeyWindowFreesTheDocument`:
  the host is not the front app); `xccov view --archive --file` → 0 uncovered
  lines in each of the 20 files (and in every piece of the 9 of Phase 2).
  Measured in a private copy of the tree (this letter's files, the others'
  as of 01:55, their test files that did not compile then left out), because
  the shared tree's test target kept failing to compile in other letters'
  files (`VerifyDatasheetWindow`, `BusPanelModel`, `PartEditorModelTests`,
  `PreferencesViewTests`, `AutomationServerTests`, `AutorouteSheetTests`,
  last `BillOfMaterialsWindowTests`). The same `coverage.js --app --tests …`
  in the shared tree at 02:50: the app target and this letter's test files
  compile; only `BillOfMaterialsWindowTests.swift` fails, so no run. The
  parent re-runs `COV --app --tests <the list> --only <file>` once the tree
  builds. The list:
  Ratchet rows: removed by the parent in this letter's commit (25 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

  ```
  DXFLoopTests,DXFReaderTests,DXFImportTests,CutoutExportTests,CutoutFileTests,EasyEDAProExportTests,ErcCompareTests,ErcDrawingChecksTests,ErcResultsModelTests,ErcMarkerTests,ErcSettingsTests,ErcPinChecksTests,GerberJobTests,GerberAttributeTests,GerberExportTests,KiCadExportTests,PourExportTests,PickAndPlaceExportTests,PrintExportTests,SchematicNSDocumentTests,SchematicFileV5Tests,SchematicExportActionTests,SplitPartExportTests,SplitPartFileTests,PourFileTests,ActiveLowExportTests,DocumentUndoTests,PowerFlagTests,BusTypeTests,NetClassTests,SocketModeTests,TitleBlockTests,ElectricalRulesCheckWindowTests,SchematicExporterTests,PartFileLoadingTests,FileMenuItemsTests,ExportSheetsTests,FileDesignRulesTests,BoardLayersTests,BoardStackupTests,NetRulesTests,NetLabelTests,HoleTests,SchematicNoteTests,DocumentInfoTests,BoardCardTests,AddOnBoardTests,HeaderBoardTests,CapacitorTests,ResistorTests,LockPositionTests,ReferencePositionTests,ViaTests,TraceRouteTests,PullResistorTests,DisplayReferenceTests,TestPointTests,SlideSwitchTests,TactileSwitchTests,TransistorTests,PlaneNetTests,FixtureTests,BoardGridTests,BoardSideTests,LooseEndTests,SchematicUndoTests,SchematicRotateTests,UndoHistoryTests,PCBUndoTests,SelectionModelTests,SchematicFileErrorTests,SchematicFileReadTests,ErcResultsModelTextTests
  ```

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 9 files, `KiCadBoardWriter.swift` … `SchematicFile.swift`

- [x] As Phase 1, for these files — done; ratchet rows: parent (every file at 0).
  Same run as Phase 1: 0 uncovered lines in `KiCadBoardWriter`, `KiCadExport`,
  `KiCadSchematicWriter`, `PrintDialog`, `PrintExport`, `SaveFormat`,
  `SchematicExportCommands`, `SchematicExporter`, `SchematicFile` (+`Document`,
  +`Board`). `SZ`: `node scripts/source-size.js --check` (new files staged) →
  exit 0; largest new test file 498 lines (`SchematicExporterTests.swift`).
  Ratchet rows: removed by the parent in this letter's commit (25 rows). Re-measured in the
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

- (plan-68-Y) § 2's `SchematicFile.swift` row stands for its plan-68-V pieces
  `SchematicFile.swift`, `SchematicFile+Document.swift`,
  `SchematicFile+Board.swift`. `Document/SchematicExport.swift` (not in § 2,
  0 uncovered under the full suite) is not this letter's.
- Panels, alerts and print panels of the exports and of File › Load Part… go
  through `ExportPresenter` (`SchematicExporter.swift`): `choose`, `alert`,
  `keyWindow`, `print`, `printDocument`, the system's by default. Tests answer
  them (`ExportAnswers`); the defaults are tested once each with the real
  panel put up and taken down by a timer (`NSApp.abortModal`, `endSheet`),
  the print ones on a saving `NSPrintInfo` so nothing reaches a printer.
- Menu items moved out of `Commands` bodies into views so they render in a
  test: `PartFileOpenItems`, `SchematicFileSaveItems` (from
  `SchematicFileCommands`), `SchematicImportItems`, `SchematicExportItems`
  (inside the Import and Export menus); `EditMenuItems` and
  `ElectricalRulesCheckMenuItem` are no longer `private`. The menus are the
  same items in the same order.
- Logic out of views: `ErcResultsModel.copyText`, `copyItems()` (the table's
  `onCopyCommand` is handed the method: SwiftUI answers Copy only in the key
  window of the active app, which a test host is not), `.summary` and
  `summary(errors:warnings:)` (the ERC table's Copy and subtitle);
  `ElectricalRulesCheckRules.severity(of:in:)` (a rule's menu binding);
  `EditMenuItem.perform(on:firstResponder:send:beep:)` and
  `sendToResponders` (the Edit items' routing, from `EditMenuItems`);
  `KiCad.edgePins` (the connector's pins, one per name, from
  `KiCadExport.Context.init`).
- Unreachable or unused code removed: `DXFIgnoredReason`'s `Comparable`
  (nothing sorts reasons), `EasyEDAProExport.write` and `EProJSON.str` (no
  caller), `GerberWriter.isEmpty` (no caller), `BoardArtwork.cutouts`' and
  `NetGroupEntry`'s default values (every init passes them);
  `GerberOutline.contours`' `@unknown default: break` is now the
  `.closeSubpath` case's `default` (a newer element type ends the contour);
  `PrintExport.boardBounds` is `BoardSettings.bounds(of:)` of the outline (an
  outline is never empty); `KiCadExport`'s net-name fallback for a net with
  no pad is one expression with the board-pad case;
  `ChildWindowOpener`'s unknown-target `assertionFailure` (it traps in
  Debug, so no test can run it) is now a documented no-op.
- More restructuring, no output change: `KiCadSchematicWriter.flagIfNew` is
  the one place a rail's power flag is added (the supplies' and the edge
  connector's; the edge's own copy ran only for a rail with no supply,
  which no catalog part makes); `farPin`'s two guards are one;
  `ExportNetNames.boardPadNetName` names a net of board pads only (tested
  directly: no fixture has such a net) and `baseName(ofNet:)` ends in one
  expression; `PrintExport`'s board page draws the outline always (it has 3
  or more points; the `else` stroked the bounds of an outline that cannot
  occur); `SchematicExporter.print(_:)` is Print…'s second half (the pages
  sheet can't hand back no pages, so the "Couldn't print" path is tested on
  it directly); `PartFileType.openableTypes` and `SchematicFile.openableTypes`
  are one expression (the extension's type differs from the declared one only
  before the declaration registers); `ElectricalRulesCheckWizard` sets its
  step in an `init` (xccov never counted the `@State` default);
  `EditMenuItem.sendToResponders` returns whether anything took the action.
- No bug found; no behavior changed.
- `PrintExport.drawBoard` and `PrintExport.path(of:)` are internal (were
  `private`) so the schematic page's no-op and a stroke's nil path are
  tested directly.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
