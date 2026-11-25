# plan-65-I: File ▸ Import ▸ DXF…

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-H (complete)

The reader (H) turns a DXF into loops and an import plan; nothing a person can reach calls it. This sub-plan adds **File ▸ Import ▸ DXF…**: a file panel, then a sheet that previews the drawing's loops, lets the person choose
the layers, the units and one of the two modes (board outline plus cutouts, or cutouts only), says what it ignored, and applies the result to the document as one undo step.

The single outcome: choosing a DXF in the File menu opens the sheet; Import in *Board outline and cutouts* mode makes the board a free-form Custom board of the DXF's largest loop with the loops inside it as cutouts; in *Cutouts only* mode adds each loop as a cutout of the existing board; each is one undo step that restores the board, its size and its cutouts exactly.

References:

- plan-65-H (`DXFReader`, `DXFLoops`, `DXFImportPlan`, `DXFUnits`, `DXFLayerPick`); plan-65-B § 4.2 (`setBoardOutline`); plan-65-A § 4.4
- `Document/SchematicExporter.swift`: `exportImage` (`:158`), `runImageOptions` and `runSheet` (`:185`–`:215`), `present` and `run(_:)` (`:385`–`:410`); `SchematicFileCommands` (`:517`–`:560`); `Document/SchematicExportCommands.swift` (`SchematicExportMenu`); `Document/ImageExportDialog.swift` (a sheet to mirror)
- `Canvas/SchematicModel.swift`: `recordEdit` and its doc comment on nesting ("Nested calls are part of the outermost one", `:2192`)
- `schematicalTests/SchematicExportActionTests.swift` (how export actions are tested without a window)

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-H is complete | `ls planning/completed \| grep -c '^plan-65-H'` → 1 | NOT MET (2026-11-25: H is written, not landed) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `File ▸ Import ▸ DXF…` (a new `Import` submenu before `Export`, one item) is enabled whenever a document is focused; it opens a file panel for `.dxf` and then `DXFImportSheet`.
- The sheet shows: the file name; a Mode picker (`Board outline and cutouts`, `Cutouts only`); a layer list with check boxes (hidden when the drawing has one layer); a Units menu (mm, cm, in, ft, m) preset from `$INSUNITS`, else from the board's units; in cutouts-only mode the X and Y of where the drawing's origin lands on the board (default x 0, y board height: the board's bottom-left, y up); a preview of every loop (outline accent, cutouts red, ignored gray, the existing board outline in cutouts-only mode); a summary line and the reasons for anything ignored or skipped; Cancel and Import.
- Import applies `SchematicDocument.applyDXFImport(_:)`: one undo step `Import DXF`.
- *Outline and cutouts*: the DXF is flipped (y down) and moved so the outline's top-left is (0, 0); the board becomes free-form (`setBoardOutline`); every existing cutout is removed and the DXF's cutouts are added (as polygons); parts, traces and everything else stay where they are.
- *Cutouts only*: each loop becomes a polygon cutout at `(offsetX + x·scale, offsetY − y·scale)`; existing cutouts and the board's outline are kept.
- Import is disabled, with the reason shown, when the plan has no outline (mode 1) or no cutouts (mode 2), or the outline fails `BoardSettings.problem(outline:)` (e.g. wider than 1000 mm: `board outline is 1200 mm wide; sizes are 1 to 1000 mm`).

### Non-goals (explicit constraints)

- No DXF export; no import of anything but loops (no text, dimensions, layers, blocks); no replacing of parts; no automation tool (J).
- The import does not switch the window's page. The sheet's preview is the confirmation; the PCB page shows the result.
- No recognition of circles or slots in imported loops: each is a polygon (a rounded cutout is only made by the Cutout tool).
- No change to documents, files or exports of anything that does not import.

## 2. Current State

- File menu: `SchematicFileCommands` puts `SchematicExportMenu` after the document's Save group (`SchematicExporter.swift:545`–`:547`). Export actions are methods of `SchematicExporter` (`exportImage`, `exportGerber`, …) that run panels as sheets on `window` (`run(_:)`) and sheets by `runSheet { finish in … }`.
- `ImageExportDialog` is the nearest sheet: SwiftUI content with `onCancel` and `onExport` closures that call `finish`.
- `recordEdit(.pcb, name) { … }` records one undo step; nested calls fold into the outermost one.
- No board import exists: `rg -n "Import" schematical/schematical/Document/SchematicExportCommands.swift schematical/schematical/Document/SchematicExporter.swift` → no output (checked 2026-11-25).

### Measured populations

| What | Count | Command |
|---|---|---|
| `NSOpenPanel`s in the exporter | 2: `kiCadFolderPanel` (`:74`, a folder to write into) and the part-file open (`:435`); none opens a board file | `rg -n "NSOpenPanel" schematical/schematical/Document --glob '*.swift'` → `SchematicExporter.swift:74`, `:435` |
| image/other sheets run with `runSheet` | 1 (`runImageOptions`) | `rg -n "runSheet" schematical/schematical/Document --glob '*.swift'` → `:187`, `:193` |
| tests of export actions this mirrors | `SchematicExportActionTests` | `ls schematical/schematicalTests/SchematicExportActionTests.swift` → exists |

### Verified properties

- `recordEdit` nesting: `setBoardOutline`, `deleteCutout` and `addCutout` each call `recordEdit`; called from inside `applyDXFImport`'s own `recordEdit` they are one step (the doc comment of `recordEdit`, `SchematicModel.swift:2192`: "Nested calls are part of the outermost one").
- The reader's output frame: the DXF's y-up coordinates, counter-clockwise polygons (plan-65-H § 3); KiCad's own board DXF lies at negative y (`planning/plan-65-data/README.md`), so the outline mode's "move the top-left to (0, 0)" is needed, not optional.

## 3. Design Overview

Pure state (`Document/DXFImportState.swift`, `@MainActor`, `Observable`): holds the `DXFDrawing`, the file name, the chosen layers (`DXFLayerPick.defaultSelection`), `units: DXFUnits`, `mode`, `offsetX`, `offsetY`, and computes `loops`, `plan`, `placed: DXFPlacedImport` (`outline: [CGPoint]?`, `cutouts: [[CGPoint]]` in board mm) and `problem: String?`. The sheet is a view of it; tests drive it without a view. `DXFPlacement.place(plan:mode:scale:offset:)` is the pure placement function (§ 1).

`SchematicExporter.importDXF()`: `NSOpenPanel` (`allowedContentTypes` from `UTType(filenameExtension: "dxf")`), read the file, `DXFReader.parse`, a parse error → `present(error, title: "Couldn't import “name”")`, else `runSheet { DXFImportSheet(state:…) }` and on Import `document.applyDXFImport(state.placed, mode:)`. The reading and the state are separate functions (`loadDXF(from url:) throws -> DXFDrawing`) so a test can call them with a temp file.

`SchematicDocument.applyDXFImport(_ placed:mode:)`: `recordEdit(.pcb, "Import DXF")`; mode 1: `setBoardOutline`, delete every cutout, add each placed cutout; mode 2: add each; the new cutouts become the selection.

Where the risk concentrates: the frame (y flip and the top-left move) and "one undo step restores everything" (board, width, height, type, cutouts together). Both have exact-equality tests. Byte-identity is irrelevant (new feature).

Rejected: importing straight after the panel with default choices (the layer and unit guesses are wrong often enough that a preview is the safe path); switching the window to the PCB page (the exporter does not know the page; every export is page-independent too); recognising circles (`BoardCutout.Kind` is derived from a rounded cutout's numbers, and a flattened circle's polygon would not equal one).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — State, placement and the document step

- [ ] `Document/DXFImportState.swift`: `DXFImportState`, `DXFPlacedImport`, `DXFPlacement.place`; `problem` per § 1 (`BoardSettings.problem(outline:)`, `BoardCutout.problem` for each cutout, and "no outline" / "no cutouts" texts).
- [ ] `Canvas/SchematicModel.swift`: `applyDXFImport(_ placed: DXFPlacedImport, mode: DXFImportMode)` per § 3.
- [ ] Tests `schematicalTests/DXFImportTests.swift` (from `DXFFixtures`): `roundedBoard` in mode 1 → outline bounds (0, 0, 60, 40), 2 cutouts, board type `.custom`, width 60 and height 40, `isFreeForm`; `kicadEdgeProbe` (negative y) in mode 1 → outline's top-left (0, 0) and the circle at y 12…18 (the y flip: DXF −15 → 15 from the top of a 30-high outline) ; `inchBoard` with the units menu left at the file's → 50.8 × 25.4; `openChain` (units code 0) takes the document's units; mode 2 with offset (0, 40) puts the `roundedBoard`'s circle centre at (15, 20) from a 40-high board's top-left; undo of an import restores `board`, `cutouts` and the selection exactly (compare `DocumentState`) and is one step; a mode-1 import over a board with 2 cutouts and a card type ends with only the DXF's cutouts and type `.custom`; `problem` is the width message for a 1200 mm outline and "no cutouts" for a DXF with only an outline in mode 2; layer defaults (`Edge`, `Dims` → `Edge`).

Acceptance: both modes place loops as above and each import is one exactly-undoable step.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/DXFImportTests -only-testing:schematicalTests/CutoutModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 2 — The sheet and the menu

- [ ] `Document/DXFImportSheet.swift` (SwiftUI): the controls and preview of § 1, bound to `DXFImportState`; a `Canvas` that draws the loops scaled to fit (outline accent, cutouts red, ignored gray, the existing outline dashed in mode 2).
- [ ] `Document/SchematicExporter.swift`: `importDXF()` and `loadDXF(from:)`; `Document/SchematicExportCommands.swift`: `SchematicImportMenu` (`Menu("Import") { Button("DXF…") }`, disabled with no file); `SchematicFileCommands` shows it before `SchematicExportMenu`.
- [ ] Tests `DXFImportTests` (second half): `loadDXF(from:)` of a temp file with `DXFFixtures.roundedBoard` gives its entities; a temp file holding binary DXF's header or random bytes throws and the message names "Binary DXF" or "isn't a DXF drawing" respectively; the state's summary line for `roundedBoard` reads `1 outline, 2 cutouts`; for `kicadEdgeProbe` in mode 2 the summary reads `1 cutout` plus an ignored line `2 inside a cutout` (the rectangle is the one depth-0 loop, the circle and half disc are inside it).

Acceptance: the file panel and the sheet open from the menu, the summary and reasons match the plan, and a parse error shows an alert and changes nothing.
  Check: `xcodebuild … test -only-testing:schematicalTests/DXFImportTests -only-testing:schematicalTests/SchematicExportActionTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min); the real menu is driven in J.
Commit: —

## Validation Plan

- Tests: `DXFImportTests` (both modes, units, offset, flip, undo, errors, the 1200 mm refusal).
- Coverage check: the sheet and menu are compiled into the app target, and `SchematicExportActionTests` builds the same `SchematicExporter`.
- Runtime proof: J chooses the real KiCad DXF in the real File menu and watches the PCB page.
- Doc sync: the import is documented in J (`docs/export.md` § Import, the in-app help); `docs/file-format.md` is B's.
- Final gate (run ONCE, after Phase 2): the Phase 2 check plus `-only-testing:schematicalTests/DXFReaderTests` (est. 6 min).

## Decisions

- Both modes, chosen in the sheet — decided by the owner, 2026-11-25.
- No page switch, no circle recognition, units menu preset from the file — decided by the author (no user-visible fork beyond the above), 2026-11-25.

## Corrections

<Filled in during execution.>

## Summary

Risk: the coordinate frame and the undo step; both are exact-equality tests. Left untouched: the reader (H), the board model (A, B), exports.
