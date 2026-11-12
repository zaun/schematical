# plan-33-H: The app — capacitors in the drawer, on the canvas, in the sidebar, the file and the BOM

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-G

H shows the capacitor parts F and G added. The drawer lists them under
**Capacitors → Ceramic / Electrolytic / Tantalum → <value>**, leaves in
capacitance order (10 pF … 4700 µF), with **Capacitors → Generic** above
them. A capacitor draws as a capacitor symbol (two plates; a polarized one
has a curved negative plate and a `+` by its positive pin). The sidebar
shows the kind "Capacitor" and its facts. `GENERIC-CAPACITOR` has a
Capacitance field (pF / nF / µF) and a Footprint picker, saved in the
schematic file as `farads`. New capacitors get the reference `C<n>`. The
Bill of Materials shows each capacitor's value.

The single behavioral outcome: in the app harness, placing
`GENERIC-CAPACITOR` gives `C1` showing `0.1 µF` on a ceramic disc footprint
(`CAP-DISC-5.08`); setting it to 4.7 nF, saving and reopening keeps
`4.7 nF`; undo restores `0.1 µF`; the drawer's Ceramic leaves run in
capacitance order; a 6.3 V tantalum wired to a 12 V supply is refused with
the rating message.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` :225,
  `isPullResistor` :383, `detailLabels` :470, `symbol(for:)` :818,
  `listsPartsDirectly` :1091, `functionGroups` :1109).
- `Backend/PartFacts.swift` (`kindName(_ kind:)` :139, `PART_DIODE` :166,
  `kindName(_ part:)` :174).
- `Canvas/SchematicModel.swift` (`PartSymbol` :114, the layout switch :222,
  `PlacedChip.ohms` :609 and its init :656, `takeReference` :1680,
  `setPartOhms` :1755, paste :1961), `Canvas/ResistorValue.swift`,
  `Canvas/DiodeSymbolView.swift` (a two-pin symbol view),
  `Canvas/SchematicCanvasView.swift` (:309), `Canvas/SchematicExportScene.swift` (:50).
- `Drawer/PartOptionsSection.swift` (`OhmsField` :109, the pull branch :88).
- `Document/SchematicFile.swift` (`ohms` :92, `badOhms` :433/:471/:685, write :519, read :705),
  `docs/file-format.md` (§ Parts table, `ohms` row :128, `reference` row).
- `BillOfMaterials/BillOfMaterialsWindow.swift` (`line(for:references:)` :58).
- `schematical/schematicalTests/PullResistorTests.swift` (the test precedent),
  `scripts/harness/build.sh`, `scripts/harness/diode-app.swift` (the harness precedent).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-G complete | `ls planning/completed/plan-33-G-*.md` → exists | MET (2026-11-11, at execution: 67ea9ea) |
| No capacitor app code yet | `grep -rc 'PART_CAPACITOR' schematical/schematical --include='*.swift' \| grep -v ':0' \| wc -l` → 0 | MET (2026-11-11 at 8b1770b: 0; at execution, before H's edits: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/CapacitorTests` passes (§ Phase 3).
- The harness (`scripts/harness/capacitor-app.swift`) prints, and checks:
  the drawer's Capacitors category has the function `Generic` and the
  subcategories `Ceramic`, `Electrolytic`, `Tantalum`; each subcategory's
  leaves are in strictly increasing capacitance; `kindName` of the first
  ceramic `add` part is "Capacitor" with facts Type, Capacitance, Rated
  voltage, Dielectric, Tolerance; `symbol(for:)` is `.capacitor`; the
  § intro outcome (C1, 0.1 µF, 4.7 nF round trip, undo, the 12 V refusal).

### Non-goals (explicit constraints)

- No backend or part-data change.
- The pull resistors' `ohms`, references and BOM lines are unchanged
  (`PullResistorTests` passes unchanged).
- File version stays 4: `farads` is optional like `gender`
  (`docs/file-format.md` § Parts); a file without capacitors is written
  byte-identically.
- No Preferences entry for a default capacitance (§ Decisions).
- No part-editor form for the `capacitor` kind (plan-33-A § Decisions).

## 2. Current State

- The drawer tree comes from `drawer` + `function` (`PartCatalog.swift`
  :1060–1085); leaves sort by `localizedStandardCompare` on the leaf text
  (`functionGroups` :1109–1115), so "10 pF", "100 nF", "1 µF" would sort as
  text, not by value.
- `PART_CAPACITOR` parts would fall to `symbol(for:)`'s `default: return .block`
  (:841) and to no `Details` case.
- Two-pin symbols: `.diode` / `.plainDiode` draw through
  `DiodeSymbolView.paint` with `ChipLayout.diode` (pin `A` left, `K`
  right). Exhaustive `PartSymbol` switches: 3 (`grep -rn 'case .barrelJack'
  schematical/schematical | wc -l` → 3, 2026-11-11).
- **Shared places.** plan-34 (resistors; its data folder appeared on
  2026-11-11, no plan file yet) can edit the same switches: `PartSymbol`,
  `symbol(for:)`, `kindName`, `takeReference`, the BOM's `line(for:)` and
  `PartOptionsSection`. H's edits add a capacitor branch beside whatever
  branches exist then; no H step depends on plan-34's state, and none
  edits another kind's branch.
- Per-placed-part values: the pull resistor's `ohms` — `PlacedChip.ohms`,
  `setPartOhms` (undoable, "Change Resistance"), `OhmsField`,
  `SchematicFile` `ohms` with `badOhms`, BOM `RESISTOR` line, references
  `R<n>` (`takeReference`). The capacitor's `farads` copies that path.

## 3. Design

- **Catalog.** `Details.capacitor(type: String, polarized: Bool, farads:
  Double, ratedMV: UInt32, dielectric: String?, tolerance: String?)` from
  `part.capacitor?.pointee` (`capacitance_ff / 1e15`). `detailLabels`:
  `["100 nF", "50 V", "X7R"]` (missing ones left out).
  `CatalogPart.isGenericCapacitor`: kind `PART_CAPACITOR` and manufacturer
  `Generic`.
- **Drawer order.** `functionGroups`: when every part of the groups is a
  capacitor, sort the groups by the smallest `farads` in each (ties by
  name). Other kinds keep text order.
- **Symbol.** `PartSymbol.capacitor`; `symbol(for:)` `PART_CAPACITOR` →
  `.capacitor`; layout as `.diode` (pin 1 left, pin 2 right; for polarized
  parts pin `+` left). `CapacitorSymbolView.paint`: two 12 pt plates 4 pt
  apart, flat; polarized: the right plate an arc and a `+` above-left of
  the left plate. No glow, nothing in play mode. The three switches draw
  it; the SVG/PNG export uses the same `paint`.
- **Facts.** `kindName` → "Capacitor"; rows Type (`Ceramic`, `Aluminum
  electrolytic`, `Aluminum polymer`, `Tantalum`, `Tantalum polymer`,
  `Niobium oxide`), Capacitance (the generic part: the placed value), Rated
  voltage, Dielectric, Tolerance.
- **The generic value.** `Canvas/CapacitorValue.swift` (as
  `ResistorValue`): `defaultFarads` 1e-7; valid above 0 and at most 1 F and
  at least 1e-13 (0.1 pF); `text` in the largest of pF / nF / µF with a
  number of at least 1 ("100 nF", "4.7 µF", "0.1 pF" below 1 pF); `parse`.
  `PlacedChip.farads` (generic capacitor only, as `ohms`); `setPartFarads`
  (undoable, "Change Capacitance"); paste and duplicate carry it.
  `CapacitanceField` in `PartOptionsSection` (units pF, nF, µF).
- **File.** `SchematicFile.Part.farads: Double?`, written for
  `GENERIC-CAPACITOR` only; missing → 1e-7; invalid →
  `badFarads(reference:farads:)` "`C3 has capacitance 0 F; it must be more
  than 0 and at most 1 F.`". `docs/file-format.md`: a `farads` row; the
  `reference` row gains "new capacitors get the lowest `C<n>` no part
  uses"; `partNumber` lists capacitor parts and `GENERIC-CAPACITOR`.
- **References.** `takeReference`: capacitor-kind parts take the lowest
  `C<n>` no part uses (the pull resistors' `R<n>` rule), without advancing
  `nextReference`.
- **BOM.** `GENERIC-CAPACITOR` → part number `CAPACITOR`, description
  `Capacitor`, value `CapacitorValue.text(farads)`; any other capacitor →
  its own part number and description, value its capacitance text.

### Rejected

- **A Preferences default** (the pull resistor has one): the generic
  capacitor's default is fixed by the owner (0.1 µF, disc); one more
  preference is out of scope.
- **Sorting every kind's leaves by a parsed number**: other leaves are names
  ("NAND", "32 KB" already sorts right with `localizedStandardCompare`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, drawer order, symbol, facts, references

- [x] `PartCatalog.swift`: `Details.capacitor`, its builder, `detailLabels`,
      `isGenericCapacitor`, `symbol(for:)`, the capacitor leaf order in
      `functionGroups`.
- [x] `SchematicModel.swift`: `PartSymbol.capacitor` and its layout case;
      `takeReference` `C<n>`.
- [x] `Canvas/CapacitorSymbolView.swift` (new); `SchematicCanvasView.swift`,
      `SchematicExportScene.swift`: the `.capacitor` case.
- [x] `PartFacts.swift`: `case PART_CAPACITOR: return "Capacitor"` and the
      rows.

Acceptance: the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p33-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min).
  Observed: `** BUILD SUCCEEDED **`.
Commit: f0b2a93

### Phase 2 — The generic value: field, file, undo, paste, BOM

- [x] `Canvas/CapacitorValue.swift` (new); `PlacedChip.farads` and its init
      rule; `setPartFarads`; the paste path (:1961) carries `farads`.
- [x] `Drawer/PartOptionsSection.swift`: `CapacitanceField` for the generic
      capacitor beside the Footprint picker.
- [x] `Document/SchematicFile.swift`: `farads` read and write, `badFarads`.
- [x] `BillOfMaterials/BillOfMaterialsWindow.swift`: the two capacitor lines.
- [x] `docs/file-format.md`: the `farads` row, the `reference` and
      `partNumber` rows.

Acceptance: the app builds and a file without capacitors is written as before.
  Check: the Phase 1 build → `** BUILD SUCCEEDED **`, then `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p33-dd test -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min).
  Observed: `Executed 14 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: f0b2a93

### Phase 3 — Tests and the harness proof

- [x] `schematical/schematicalTests/CapacitorTests.swift` (new, as
      `PullResistorTests`): placing `GENERIC-CAPACITOR` gives `C1`, then
      `C2`, and a gate after them `U1`; `farads` 1e-7 and footprint
      `CAP-DISC-5.08`; `setPartFarads` 4.7e-9 then undo → 1e-7; file round
      trip keeps 4.7e-9 and the footprint; a file with `"farads": 0` fails
      with `badFarads`; an old file without `farads` opens with 1e-7;
      `CapacitorValue.text` for 1e-13, 1e-12, 4.7e-9, 1e-7, 4.7e-6, 4.7e-3
      → `0.1 pF`, `1 pF`, `4.7 nF`, `100 nF`, `4.7 µF`, `4700 µF`; the BOM
      lines (generic → `CAPACITOR`, `100 nF`; a real ceramic → its part
      number and value). Observed: `Executed 8 tests, with 0 failures`.
- [x] `scripts/harness/capacitor-app.swift` (new, run with
      `scripts/harness/build.sh`): the § 1 checks; renders a placed
      ceramic, an electrolytic and a tantalum with `Harness.renderCanvas` to
      `/tmp/p33-symbols.png`, and the sidebar of the generic capacitor to
      `/tmp/p33-sidebar.png`; look at both PNGs. Looked: the ceramic's two
      flat plates, the electrolytic's and tantalum's curved negative plate
      and `+`, values below; the sidebar shows C1, Capacitance 100 nF,
      the disc footprint and Kind Capacitor.

Acceptance: the tests pass and the harness prints `all checks ok`.
  Check: `xcodebuild … test -only-testing:schematicalTests/CapacitorTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`; `scripts/harness/build.sh scripts/harness/capacitor-app.swift` → `all checks ok`, exit 0 (est. 15 min: the harness build; nothing smaller runs the catalog and the backend together).
  Observed: `** TEST SUCCEEDED **`; the harness: drawer Capacitors → [Generic] and Ceramic / Electrolytic / Tantalum (42 / 22 / 25 leaves, each strictly increasing), `08055C100KAT2A` Capacitor with Type, Capacitance, Rated voltage, Dielectric, Tolerance, GENERIC-CAPACITOR C1 100 nF on CAP-DISC-5.08, 4.7 nF kept through save and reopen, undo to 0.1 µF, `Can't connect: C1 pin 1 (+) gets 12 V; TACL105K006XTA is rated 6.3 V.`, `all checks ok`, exit 0.
Commit: (this commit; hash in the next)

## Validation Plan

- Tests: `CapacitorTests` (value text and parse, references, undo, file
  round trip, bad value, BOM), `PullResistorTests` and
  `BillOfMaterialsTests` unchanged.
- Coverage check: the harness walks every drawer leaf of the three
  subcategories (the order check covers all of them).
- Runtime proof: the harness's 12 V refusal of a 6.3 V tantalum runs the
  app's `VoltageCheck` on the backend's new problem kind.
- Doc sync: `docs/file-format.md`.
- Final gate: plan-33-I.

## Decisions

- **Capacitors → type → value**, with the generic at **Capacitors →
  Generic** — owner (drawer, plan-33-A § Decisions); the generic placement
  is the author's (it is neither type).
- **References `C<n>`** — author: the pull resistors' `R<n>` precedent; a
  capacitor labelled `U` would read wrong on a board.
- **Value only on the generic part** — author: a real part's capacitance is
  its part number's; only the generic one stands for any value.

## Corrections

1. **"0.1 µF" is written "100 nF".** § 3's value rule (the largest unit
   whose number is at least 1, plan-33-F § 3.3) gives `100 nF` for 1e-7 F;
   the § intro's "showing 0.1 µF" is the same value. The harness checks
   `farads` = 1e-7 and the text `100 nF`.
2. **Code written before G landed.** Phases 1–2's code was drafted while
   plan-33-A's census ran (it needs no part data) and built and tested in
   order after G (f0b2a93).
3. **The drawer hides automotive parts by default** (`DrawerFilter`), so it
   lists 375 of the 376 ceramic parts, 318 of 323 electrolytic and 331 of
   332 tantalum ones; the order check covers every listed leaf.
4. **`CapacitorValue.isValid` takes 0.1 pF to 1 F** (§ 3), so `badFarads`'s
   text keeps the plan's wording ("more than 0 and at most 1 F") for the
   common mistake, a 0 value; `farads: 1` opens.

## Summary

H is wide but shallow: a details case, a symbol, a leaf order, one more
per-part value on the pull resistor's path, and the BOM. The risk is the
file format (an optional field, version unchanged), covered by the round-trip
and old-file tests.
