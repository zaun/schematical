# plan-34-G: The app — resistors in the drawer, on the canvas, in the sidebar, the file and the BOM

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-34-F

G shows the resistor parts F added. The drawer lists them under
**Resistors → Chip / Through-hole / Array → <ohms group>** with the groups in
ohms order (`< 1 kΩ` first, `≥ 1 MΩ` last) and **Resistors → Generic**
beside them. A two-terminal resistor draws as a horizontal zig-zag with
its value above it; an array draws as a block. The sidebar shows the kind
"Resistor" ("Resistor array" for arrays) with Resistance, Tolerance, Power
and Resistors. `GENERIC-RESISTOR` has the pulls' Ohms field and footprint
picker; it is saved with `ohms` in the file and is one `RESISTOR` BOM line
per value and package (shared with the pulls). Every resistor-kind part
takes the lowest free `R<n>`.

The behavioral outcome: placing `GENERIC-RESISTOR` gives `R1`, showing
`1kΩ`, on `RES-AXIAL-10.16`; setting 4.7 kΩ shows `4.7kΩ` and saves
`"ohms": 4700`; undo restores 1 kΩ; in play mode an SN74HC04N output
through `GENERIC-RESISTOR` to `GENERIC-LED` (cathode to `GND`) lights the
LED while the output is 1.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` :236–270,
  `drawerLeaf` :358, `isPullResistor` :383, `detailLabels`, the details
  builder, `symbol(for:)` :818, `listsPartsDirectly` :1091,
  `functionGroups` :1109).
- `Backend/PartFacts.swift` (`kindName(_ kind:)` :139, `kindName(_ part:)`
  :174, the facts rows).
- `Canvas/SchematicModel.swift` (`PartSymbol` :114, the layout switch,
  `PlacedChip.ohms` and its init :656, `takeReference` :1680,
  `setPartOhms` :1755), `Canvas/ResistorValue.swift`,
  `Canvas/SupplySymbolView.swift` (the pulls' zig-zag :62),
  `Canvas/SchematicCanvasView.swift`, `Canvas/SchematicExportScene.swift`
  (the other two `PartSymbol` switches).
- `Drawer/PartOptionsSection.swift` (`OhmsField` :109, the pull branch :88).
- `Document/SchematicFile.swift` (`ohms` :92, the read :684–705),
  `docs/file-format.md` (§ Parts, the `ohms` row :128).
- `BillOfMaterials/BillOfMaterialsWindow.swift` (`line(for:references:)` :58).
- `schematical/schematicalTests/PullResistorTests.swift` (the test
  precedent), `scripts/harness/build.sh` and `diode-app.swift` (the harness
  precedent).
- `planning/plan-33-H-capacitor-app.md` (open; edits `functionGroups`,
  `takeReference`, the BOM, the file and the three `PartSymbol` switches
  for capacitors, § 2).

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-34-F complete | `ls planning/completed/plan-34-F-*.md` → exists | NOT MET at writing (2026-11-11) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p34-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at writing (2026-11-11 at 8b1770b) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/ResistorTests
  -only-testing:schematicalTests/PullResistorTests
  -only-testing:schematicalTests/BillOfMaterialsTests` passes (Phase 3).
- The harness (`scripts/harness/resistor-app.swift`) prints and checks:
  the drawer's Resistors category has the function `Generic` and the
  subcategories `Array`, `Chip`, `Through-hole`; each subcategory's leaves
  are in increasing ohms order; `kindName` of the first chip `add` part is
  "Resistor" with facts Resistance, Tolerance, Power; an array's is
  "Resistor array" with Resistors; `symbol(for:)` is `.resistor` for
  two-terminal parts and `.block` for arrays; the outcome above (R1, 1kΩ,
  4.7 kΩ round trip, undo, the LED lit through the resistor).

### Non-goals (explicit constraints)

- The pulls' `ohms`, references, Settings default and BOM lines are
  unchanged (`PullResistorTests` passes unchanged).
- File `version` stays 4: `ohms` on `GENERIC-RESISTOR` is an optional field
  (`docs/file-format.md` § Rules for changing the format); an older
  version-4 build ignores it.
- A maker part's resistance is its part's fact, not editable; only
  `GENERIC-RESISTOR` (and the pulls) have the Ohms field.
- No Settings default for the generic resistor (Axial and 1 kΩ, the
  owner's defaults, are fixed).
- No part-editor form for the `resistor` kind (plan-32's scope, §
  Decisions).

## 2. Current State

- The drawer tree comes from `drawer` + `function` (`PartCatalog.swift`
  :1065–1083); groups sort by `localizedStandardCompare`
  (`functionGroups` :1109–1115), so `1 kΩ – 5 kΩ`, `10 kΩ – 50 kΩ`,
  `100 kΩ – 500 kΩ`, `1 MΩ` would sort by their leading number, not by
  ohms.
- `PART_RESISTOR` parts would fall to `symbol(for:)`'s `default: return
  .block` and to `kindName`'s `default: return ""`; `Details` has no
  resistor case.
- Per-placed-part resistance exists for the pulls only: `PlacedChip.ohms`,
  `setPartOhms` (undoable, "Change Resistance"), `OhmsField`,
  `SchematicFile` `ohms` with `badOhms`, BOM `RESISTOR` lines, `R<n>`
  references (`takeReference`), each behind `isPullResistor` (10 uses:
  `grep -rn 'isPullResistor' --include='*.swift' schematical/schematical | wc -l` → 10).
- Exhaustive `PartSymbol` switches: 3 (`SchematicModel.swift` layout,
  `SchematicExportScene.swift`, `SchematicCanvasView.swift`).
- **plan-33-H (open) edits the same places for capacitors**: a capacitor
  branch in `functionGroups`, `C<n>` in `takeReference`, a `farads` field,
  a `PartSymbol.capacitor` case. G adds resistor branches beside whatever
  branches exist; no step of G depends on plan-33's state.

## 3. Design

- **Details.** `Details.resistor(ohms: Double, tolerancePercent: Double?,
  powerW: Double?, resistors: Int)` from `part.resistor?.pointee`
  (`milliohms / 1000`, `tolerance_ppm / 10000`, `power_mw / 1000`; 0 → nil).
  `detailLabels`: `["4.7kΩ", "1%", "0.1 W"]` (missing ones left out; the
  value by `ResistorValue.text`). `CatalogPart.isGenericResistor`: kind
  `PART_RESISTOR` and manufacturer `Generic`. `CatalogPart.hasOhmsField`:
  `isPullResistor || isGenericResistor`.
- **Ohms field and file.** The uses of `isPullResistor` that are about the
  placed resistance switch to `hasOhmsField`: `PlacedChip.init` (default
  1 kΩ), `setPartOhms`, `PartOptionsSection`'s Ohms row, `SchematicFile`'s
  read and `badOhms`, the write. `Preferences` keeps `isPullResistor` (the
  Settings default is the pulls'). `docs/file-format.md`'s `ohms` row: "Pull
  resistors and `GENERIC-RESISTOR`".
- **References.** `takeReference`: every `PART_RESISTOR` part (and the
  pulls) takes the lowest `R<n>` no part uses, without advancing
  `nextReference`.
- **Drawer order.** `functionGroups`: when every part of the groups is a
  resistor, sort the groups by the smallest ohms in each (ties by name);
  `Generic` (ohms 1000 by the part) is a function of the category, not of
  a subcategory, so it is its own list. Other kinds keep text order.
- **Symbol.** `PartSymbol.resistor`; `symbol(for:)`: `PART_RESISTOR` with
  one resistor and pins named `1` and `2` → `.resistor`, else `.block`.
  Layout as `.diode` (pin 1 left, pin 2 right). `ResistorSymbolView.paint`:
  the pulls' zig-zag (six peaks, `SupplySymbolView` :62) laid horizontally
  between the pins, the value (`ResistorValue.text` of the placed chip's
  `ohms` for the generic, of the part's fact otherwise) centred above. No
  glow, nothing in play mode. The three switches draw `.resistor`.
- **Facts.** `kindName(_ kind:)` `PART_RESISTOR` → "Resistor";
  `kindName(_ part:)`: more than one resistor → "Resistor array". Rows:
  Resistance, Tolerance (`±1%`), Power (`0.1 W`), Resistors (arrays only).
- **BOM.** `GENERIC-RESISTOR` → the pulls' `RESISTOR` line (value and
  package; a generic and a pull of the same value and package share a
  line). A maker part → its part-number line with `value` =
  `ResistorValue.text(ohms)` (the `value` column was pulls-only).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, drawer order, symbol, facts, references

- [ ] `PartCatalog.swift`: `Details.resistor`, its builder, `detailLabels`,
      `isGenericResistor`, `hasOhmsField`, `symbol(for:)`, the resistor
      order in `functionGroups`.
- [ ] `SchematicModel.swift`: `PartSymbol.resistor` and its layout case;
      `takeReference` `R<n>` for `PART_RESISTOR`.
- [ ] `Canvas/ResistorSymbolView.swift` (new); the `.resistor` case in
      `SchematicCanvasView.swift` and `SchematicExportScene.swift`.
- [ ] `PartFacts.swift`: "Resistor", "Resistor array", the rows.

Acceptance: the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p34-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 2 — Ohms field, file, BOM

- [ ] `SchematicModel.swift`: `PlacedChip.init` and `setPartOhms` on
      `hasOhmsField`; `PartOptionsSection.swift`: the Ohms row on
      `hasOhmsField`.
- [ ] `SchematicFile.swift`: read, validate (`badOhms`) and write `ohms` for
      `hasOhmsField` parts.
- [ ] `BillOfMaterialsWindow.swift`: the generic's `RESISTOR` line, the
      maker parts' `value`.
- [ ] `docs/file-format.md`: the `ohms` row and the validation line (`R3
      has …`) name `GENERIC-RESISTOR`.

Acceptance: the pulls are unchanged and the app builds.
  Check: the Phase 1 build → `** BUILD SUCCEEDED **`, then `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p34-dd test -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — Tests and the harness

- [ ] `schematical/schematicalTests/ResistorTests.swift` (as
      `PullResistorTests`): placing `GENERIC-RESISTOR` gives `R1`, 1000 Ω,
      footprint `RES-AXIAL-10.16`; a second resistor-kind part and a pull
      take `R2`, `R3`; `setPartOhms` 4700 then undo → 1000; save → `"ohms":
      4700` and reopen restores it; `"ohms": 0` on open → `badOhms`; BOM: a
      generic 4.7 kΩ Axial and a pull 4.7 kΩ Axial share one `RESISTOR`
      line; a maker part's line has `value` `4.7kΩ`; the drawer groups of
      `Resistors → Chip` are in increasing ohms.
- [ ] `scripts/harness/resistor-app.swift`: § 1's harness checks, the
      LED-through-a-resistor run (inverter input 0 → LED lit, 1 → dark,
      each its own 0.6 s document run, as `diode-app.swift`), and offscreen
      renders of a placed generic resistor, a chip resistor and an array
      (`Harness.renderCanvas`), looked at.

Acceptance: the tests pass and the harness prints `all checks ok`.
  Check: `xcodebuild … test -only-testing:schematicalTests/ResistorTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`; `scripts/harness/build.sh scripts/harness/resistor-app.swift /tmp/p34-harness && /tmp/p34-harness/H.app/Contents/MacOS/harness` → `all checks ok`, exit 0 (est. 15 min: the harness build is the slow part; nothing smaller runs the app's catalog code and the backend together).
Commit: —

## Validation Plan

- Tests: `ResistorTests` (new), `PullResistorTests` and
  `BillOfMaterialsTests` (unchanged, must pass).
- Coverage check: the harness prints one line per § 1 check.
- Runtime proof: the LED-through-a-resistor run in the real
  `SimulationController`.
- Doc sync: `docs/file-format.md` (`ohms`).
- Final gate: plan-34-H.

## Decisions

- **The generic shares the pulls' `RESISTOR` BOM line** — author: a pull
  and a generic resistor of the same value and package are the same thing
  to buy.
- **Every resistor-kind part is `R<n>`** — author: the pulls already are;
  a maker resistor numbered `U<n>` would be wrong on the board.
- **Arrays as blocks** — author: their pin maps vary (4–16 pins); the
  block layout already draws any pin set with names.
- **No part-editor form for the `resistor` kind** — author: plan-32 (open)
  covers the 26 existing kinds (its owner's decision); plan-34 does not
  edit plan-32's scope. plan-32-C's test that compares `PartDraft.kindNames`
  with the `Parts/*.json` file names sees `resistor.json` once F lands
  (noted in plan-32-A § Corrections by this plan's commit).
- Owner decisions: plan-34-A § Decisions.

## Corrections

(none yet)

## Summary

G is local: one details case, one symbol, the drawer order, and the pulls'
resistance path widened to the generic resistor. The coupling is plan-33-H
editing the same switch statements and functions; G's edits are written
to land on either version.
