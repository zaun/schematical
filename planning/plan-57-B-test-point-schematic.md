# plan-57-B: Test point on the schematic: symbol, TP reference, no BOM line

Last updated: 2026-11-23
Effort: medium
Depends on: plan-57-A

plan-57-A made `GENERIC-TESTPOINT` a real, placeable part that the app draws as
a plain one-pin box, numbers `J1`, and lists in the Bill of Materials. This
sub-plan makes it look and behave like a test point: a small circle on a pin
stub, a `TP1`, `TP2`, … reference, and no Bill of Materials line.

References:

- plan-57-A (the part, `CatalogPart.isTestPoint`, `PartDetails.testPoint`).
- `schematical/schematical/Canvas/HeaderSymbolView.swift` (the model for a
  one-pin-per-slot symbol view with a `paint` that the export scene reuses),
  `Canvas/SchematicModel.swift` (`PartSymbol`, `ChipLayout.init(part:symbol:)`),
  `Canvas/SchematicExportScene.swift`, `Canvas/ReferenceLabel.swift`.
- `schematical/schematical/Preferences/ReferencePrefixes.swift`
  (`ReferenceCategory.testPoint`, `"TP"`, line 9 / 36 / 65, already present).
- `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift`.

## Prerequisites

The overall prerequisites are in plan-57-A § Prerequisites; this sub-plan adds
one.

| Must be true | Command | Status |
|---|---|---|
| plan-57-A is complete | `rg -c 'isTestPoint' schematical/schematical/Backend/PartCatalog.swift` → ≥ 1 and `rg -c GENERIC-TESTPOINT schematical/schematical/Parts/connector.json` → 1 | MET (`isTestPoint` → 2; `GENERIC-TESTPOINT` → 1; plan-57-A commits 9fcf8fe3, d0c6d820) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue. If you stop,
> report the status of all prerequisites (A's included).

## 1. Goal

- A placed test point draws as a 10 pt open circle at the end of a 20 pt-high,
  40 pt-wide body's left lead, with its pin stub, on the canvas, in exports and
  with the reference above it; it is selectable and hoverable like a header.
- New test points are numbered `TP1`, `TP2`, … (the prefix the Settings ▸
  Reference Defaults "Test Point" row sets).
- The Bill of Materials (window, CSV and JLCPCB CSV) has no line for a test
  point.

### Non-goals (explicit constraints)

- No change to the part data, the backend or any file format.
- No change to how other connectors, headers or any other symbol draw or number.
- The Size menu and the Through hole toggle are plan-57-C; the footprint menu
  stays the flat one until then.

## 2. Current State

- `PartSymbol` (`Canvas/SchematicModel.swift:190`) has `.barrelJack`, `.header`,
  …; plan-57-A makes `symbol(for:)` return `.block` for a test point
  (`PartCatalog.swift`, first line of the `PART_CONNECTOR` case).
- Every place that switches over the symbol with no `default` (so a new symbol
  must say what it does): `ChipLayout.init(part:symbol:)`
  (`SchematicModel.swift:300`), `SchematicExportScene.swift:50`,
  `SchematicCanvasView.swift:1124`, and the grouped case in
  `ReferenceLabel.swift:47`.
- `HeaderSymbolView` draws a body rectangle, a lead from each slot's edge to
  its stub tip and a pin mark; its `paint` is generic over `SchematicPainter`
  and is called by both the view and the export scene (`SchematicExportScene.swift:52`).
- The reference prefix of a new part is `ReferencePrefixes.prefix(for: part,
  gender:)` (`SchematicModel.swift:2142`), from `ReferenceCategory.of(part)`
  (`ReferencePrefixes.swift:71`), whose `PART_CONNECTOR` case is
  `.femaleConnector` (line 87). `ReferenceCategory.testPoint` exists
  (`ReferencePrefixes.swift:9`, label "Test Point", prefix `TP`) and no part
  maps to it.
- `BillOfMaterials.lines(for:)` takes every chip with a footprint
  (`BillOfMaterialsWindow.swift:52`); `csv` and `jlcpcbCSV` write those lines.

### Measured populations

| What | Count | Command |
|---|---|---|
| Symbol switches with no default | 4 | `rg -n 'case \.barrelJack' schematical/schematical` → `SchematicModel.swift:300`, `SchematicExportScene.swift:50`, `SchematicCanvasView.swift:1124`; plus the grouped list at `ReferenceLabel.swift:47` |
| Callers of `BillOfMaterials.lines(for:)` | 1 | `rg -n 'BillOfMaterials\.lines' schematical/schematical` → `BillOfMaterialsWindow.swift:206` |
| Symbol views with a `paint` shared with the export scene | 21 functions in 18 files | `rg -c 'static func paint<P: SchematicPainter>' schematical/schematical/Canvas` |

### Verified properties

| Claim | How verified |
|---|---|
| A new `PartSymbol` case cannot be missed by a switch: the four switches above have no `default` | read each (`SchematicModel.swift:290–330`, `SchematicExportScene.swift:44–70`, `SchematicCanvasView.swift:1112–1135`, `ReferenceLabel.swift:18–50`) |
| `canRotate` is false for every symbol not listed (`SchematicModel.swift:210`), so a test point does not rotate and needs no turned-label code | read |
| The KiCad and EasyEDA schematic writers' `bodyBox` has a `default` that takes the layout's own size (`KiCadSchematicWriter.swift:353–364`, `EasyEDAProSchematicWriter.swift:113–124`), so they need no case | read |
| The one-pin layout needs no flip code: `HeaderSymbolView` draws from `slot.side` and `chip.layout` alone and is symmetric | read `HeaderSymbolView.swift` |

## 3. Design Overview

Two independent pieces, smallest blast radius first:

1. **Reference and BOM** — one line in `ReferenceCategory.of`, one condition in
   `BillOfMaterials.lines`. Pure Swift, no drawing.
2. **The symbol** — a new `PartSymbol.testPoint`, its layout, its view, and the
   four switch sites. Correctness risk concentrates here only in "the canvas,
   the export scene and the reference label must all agree on the geometry";
   one shared `paint` and one layout function make that structural.

The gate is behavior, not byte-identity. Expected changes: every existing symbol
draws identically (the other cases' code is untouched); the only new drawing is
the test point's.

**Symbol geometry (decided):** layout size 40 × 20 (`ChipLayout.pitch`), one
slot `side .left`, `centerY = pitch / 2`. Drawn: a lead from the stub tip
(`margin - stubLength`) to the circle's left edge, a circle of diameter 10
centred at x = `margin + 20`, y = 10, filled `.paper`, stroked `.primary`
(`.accent` and line width 2 when selected or hovered, as `HeaderSymbolView`), no
body rectangle and no pin text (the pin's name `TP` carries nothing).

**Rejected alternatives:** a filled dot (reads as a junction on a schematic);
a two-circle "probe" glyph (more drawing code for no extra information);
excluding test points from the BOM by package name (a kind-level
`isTestPoint` check is the one source of truth).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partial; mark a moot task
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` when a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — `TP` reference and no BOM line

Behavior only; the symbol is still the block from plan-57-A.

- [x] `Preferences/ReferencePrefixes.swift:87`: `case PART_CONNECTOR: return
      part.isTestPoint ? .testPoint : .femaleConnector`.
- [x] `BillOfMaterials/BillOfMaterialsWindow.swift:52`: the loop is `for chip in
      chips where chip.part.footprint != nil && !chip.part.isTestPoint`; the doc
      comment above `lines(for:)` says "Test points are not bought, so they are
      left out too".
- [x] `schematicalTests/TestPointTests.swift` (from plan-57-A): placing
      `GENERIC-TESTPOINT` three times gives references `TP1`, `TP2`, `TP3`; a
      barrel jack placed beside still gets `J1` (the prefix is not shared);
      `ReferenceCategory.of(testPoint) == .testPoint`.
- [x] `schematicalTests/BillOfMaterialsTests.swift`: a document with two test
      points and one `SN74HC00D` has one BOM line (the gate); `csv` and
      `jlcpcbCSV` of those lines contain no `TESTPOINT`.

Acceptance: references and BOM are right with the old block symbol still drawn.
  Observed: `xcodebuild … -derivedDataPath /tmp/plan57/dd test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/BillOfMaterialsTests` → `Executed 13 tests, with 0 failures` and `** TEST SUCCEEDED **` (the new tests are `testReferencesAreTP` and `testTestPointsAreLeftOut`).
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/BillOfMaterialsTests` → `** TEST SUCCEEDED **` (est. 8 min incl. build).
Commit: —

### Phase 2 — The circle-on-a-stub symbol

- [ ] `Canvas/SchematicModel.swift`: add `case testPoint` to `PartSymbol` after
      `.header` (line 195) with a doc comment (the geometry above);
      `ChipLayout.init(part:symbol:)` (line 300) gets `case .testPoint: self =
      Self.testPoint(pins)`; add `static func testPoint(_ pins: [ChipPin]) ->
      ChipLayout` returning size `CGSize(width: 40, height: pitch)` and one
      `PinSlot(pin:, side: .left, centerY: pitch / 2)` for the first pin.
- [ ] `Canvas/TestPointSymbolView.swift` (new): `struct TestPointSymbolView:
      View` (frame `layout.size.width + 2 × labelMargin` × `layout.size.height`,
      a `Canvas` calling `paint`) and `static func paint<P: SchematicPainter>(
      _ painter: inout P, chip:, isSelected:, isHovered:)` drawing the lead and
      the circle as designed; modelled on `HeaderSymbolView.swift`.
- [ ] `Canvas/SchematicCanvasView.swift:1124` area: `case .testPoint:
      TestPointSymbolView(chip:, isSelected: document.isSelected(.chip(chip.id)),
      isHovered: chip.id == hoveredChipID)` beside the `.header` case.
- [ ] `Canvas/SchematicExportScene.swift:50` area: `case .testPoint:
      TestPointSymbolView.paint(&placed, chip: chip, isSelected: false,
      isHovered: false)`.
- [ ] `Canvas/ReferenceLabel.swift:47`: add `.testPoint` to the grouped case that
      returns `CGPoint(x: x, y: -3)` (just above the symbol's frame).
- [ ] `Backend/PartCatalog.swift` (`symbol(for:)`, the `PART_CONNECTOR` case):
      `if part.isTestPoint { return .testPoint }` (replaces plan-57-A's `.block`).
- [ ] Tests: `TestPointTests` — `symbol(for:) == .testPoint`; the chip's layout
      has one slot, `.left`, size 40 × 20, and `chip.layout.slots[0]`'s tip is
      `stubLength` left of the body; the part's pin takes a wire (draw one with
      `doc` helpers used in `CardSlotTests`) and the netlist joins `TP1` to it;
      `schematicalTests/KiCadExportTests.swift`: a document holding a test point
      exports a schematic and a board file in the same way its other cases do
      (well-formed, one `TP1` footprint with one pad); update plan-57-A's
      `symbol(for:) == .block` assertion to `.testPoint`.

Acceptance: the canvas, the export and the reference label draw the circle symbol with `TP1`.
  Check: `xcodebuild … test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/BillOfMaterialsTests` (same flags as Phase 1) → `** TEST SUCCEEDED **` (est. 8 min).
  Check: render a schematic with a test point on a wire through the MCP `render` tool (`mcp__schematical__place_part` GENERIC-TESTPOINT, `connect` to a net label, `render` schematic) → the picture shows a circle at the wire's end and `TP1` above it (est. 2 min; the picture is the runtime proof that the three drawing paths agree; the unit tests cannot see pixels).
Commit: —

## Validation Plan

- Tests: `TestPointTests` (reference, layout, wiring), `BillOfMaterialsTests`
  (excluded line, both CSVs), `KiCadExportTests` (export of a one-pin part).
- Coverage check: the new Swift files are under `schematical/schematical`, which
  the test target imports (`@testable import schematical`); the new test file is
  picked up with no project edit (plan-57-A § Measured populations).
- Runtime proof: the `render` check in Phase 2.
- Doc sync: none (no format or part-file change); the Help window names no
  symbol (`rg -n -i 'barrel' schematical/schematical/Help` → no match).
- Final gate (once, after Phase 2): the Phase 2 xcodebuild line plus
  `-only-testing:schematicalTests/CardSlotTests -only-testing:schematicalTests/DrawerCategoriesTests`
  (the neighbouring connector and drawer tests) → `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- Schematic symbol: a small circle on a pin stub — decided by the owner, 2026-11-23.
- Test points are left out of the Bill of Materials — decided by the owner, 2026-11-23.
- The reference prefix is the existing `TP` category — decided by the author;
  the category already exists in Settings ▸ Reference Defaults.

## Corrections

(none yet)

## Summary

The risk is the four exhaustive symbol switches agreeing on one geometry; one
layout function and one `paint` make them share it. Untouched: every other
symbol, the part data, the backend, the file formats.
