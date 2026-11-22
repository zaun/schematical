# plan-44-G: The app — crystals in the drawer, on the canvas, in the sidebar and in play mode

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-44-F

G shows the crystal and resonator parts F added. The drawer lists them under
**Crystal → kHz**, **Crystal → MHz** and **Crystal → Ceramic resonator**
(leaves: the frequency bands of plan-44-F § 3.1, ordered by frequency, and
inside a leaf by frequency then name), with **Crystal → Generic** beside
them. A crystal or resonator draws with the standard schematic symbol: a
rectangle between two plates, `X1` on the left and `X2` on the right, and
for a three-pin resonator the middle pin `GND` below it; its frequency sits
under it. A drawer row shows the frequency at its right (as an oscillator's),
and the second line the load capacitance and the tolerance. The sidebar shows
the kind "Crystal" or "Ceramic resonator" with Frequency, Load capacitance,
Tolerance, Stability and ESR (and "Built-in capacitors" for a resonator that
has them). Every crystal-kind part takes the lowest free `Y<n>` reference.
`GENERIC-CRYSTAL-32768`, `-8M`, `-16M` and `GENERIC-RESONATOR-8M` carry the
footprint picker with their choices. The behavioral outcome: placing
`GENERIC-CRYSTAL-8M` gives `Y1` on `HC-49/US`; wiring `GENERIC-CRYSTAL-32768`'s
terminal to the input of an `SN74HC04N` inverter and the inverter's output
through a resistor to a `GENERIC-LED` lights the LED in play mode, and the
same circuit with the inverter's input on a pull-up instead of the crystal
leaves the LED dark.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details`,
  `detailLabels`, `trailingLabel`, `symbol(for:)`, the details builder,
  `functionGroups`), `Backend/PartFacts.swift` (`kindName(_ kind:)`,
  `kindName(_ part:)`, the facts rows, `hertz`).
- `Canvas/SchematicModel.swift` (`PartSymbol`, `ChipLayout.init(part:symbol:)`,
  `ChipLayout.capacitor`), `Canvas/SchematicCanvasView.swift`,
  `Canvas/SchematicExportScene.swift`, `Canvas/ReferenceLabel.swift`,
  `Canvas/CapacitorSymbolView.swift` (the symbol view G mirrors).
- `Preferences/ReferencePrefixes.swift` (`ReferenceCategory.crystal` "Y",
  `of(_:)`).
- `Drawer/PartOptionsSection.swift` (the footprint picker),
  `schematicalTests/CapacitorTests.swift`, `ResistorTests.swift`,
  `TransistorTests.swift`, `DrawerPerformanceTests.swift` and
  `scripts/harness/transistor-app.swift` (the test and harness precedents);
  `planning/completed/plan-40-H-transistor-app.md`,
  `plan-34-G-resistor-app.md`.

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-F complete | `ls planning/completed/plan-44-F-*.md` → exists | MET (2026-11-22: `planning/completed/plan-44-F-crystal-parts.md`) |
| The parts are loaded | `grep -c '^GENERIC-CRYSTAL-8M|crystal|' backend/tests/golden/parts.txt` → 1; `grep -c '|crystal|' backend/tests/golden/parts.txt` → `<add rows> + 4` | MET (2026-11-22: `1` and `604`; golden line `GENERIC-CRYSTAL-8M|crystal|HC-49/US|Crystal|Generic|…|type=crystal hz=8000000 …`) |
| The editor's kind lists name the kind (F Phase 1) | `grep -c '"crystal"' schematical/schematical/Editor/KindSchema.swift` → at least 1 (`PartKinds.all`); `grep -c 'kind: "crystal"' schematical/schematical/Editor/KindInfo.swift` → 1; `ls schematical/schematical/Editor/CrystalDraft.swift` → exists | MET (2026-11-22: `1`, `1`, `CrystalDraft.swift` exists) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p44-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-22: `** BUILD SUCCEEDED **`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/CrystalTests
  -only-testing:schematicalTests/DrawerPerformanceTests
  -only-testing:schematicalTests/PartEditorWindowTests` passes (Phase 3).
- The harness (`scripts/harness/crystal-app.swift`) prints and checks: the
  drawer's `Crystal` category has the groups `kHz`, `MHz`, `Ceramic resonator`
  and the function `Generic`, leaves in frequency order; `kindName` of
  `GENERIC-CRYSTAL-8M` is "Crystal" with the facts Type, Frequency; of
  `GENERIC-RESONATOR-8M` "Ceramic resonator"; `symbol(for:)` is `.crystal` for
  both; a placed `GENERIC-CRYSTAL-8M` is `Y1` on `HC-49/US`; and the LED
  circuit of the outcome above, lit and dark.

### Non-goals (explicit constraints)

- No new field in the schematic file (`docs/file-format.md` is unchanged): a
  placed crystal saves what any placed part saves, including its footprint
  choice. The frequency of a generic is the part's (no per-instance value).
- No play-mode glow or state on the symbol (the capacitor and resistor
  precedent); the LED's light is the proof.
- No part-editor form for the `crystal` kind (§ Decisions).
- The simulation is E's; G adds no engine behavior. The voltage check has no
  crystal rule (E § 1).
- No Bill of Materials change: a crystal's line carries its part number and
  package like any part.

## 2. Current State

- `PART_CRYSTAL` falls to `symbol(for:)`'s `default: return .block`, to
  `kindName`'s `default: return ""`, and the details builder's `else` branch
  gives it `.supply(level: 0, …)`
  (`PartCatalog.swift` end of the builder); the two exhaustive switches over
  `Details` are `detailLabels` and the facts rows (`PartFacts.swift`).
- Exhaustive `PartSymbol` switches: four (`SchematicModel.swift` layout,
  `SchematicCanvasView.swift`, `SchematicExportScene.swift`,
  `ReferenceLabel.swift` anchor).
- `ReferenceCategory.crystal` exists with the prefix `Y`
  (`ReferencePrefixes.swift`); `ReferenceCategory.of(_:)` has no `PART_CRYSTAL`
  case, so a crystal falls to `.integratedCircuit` (`U`).
- The footprint picker shows for any part with `footprintChoices`
  (`PartOptionsSection.swift`), so the generics need no new control.
- The drawer takes up to three levels and groups leaves by `function`
  (`functionGroups`), sorted by `localizedStandardCompare`, except capacitor
  and resistor leaves, which go by value (`value` closure); parts in a leaf go
  by `nameRank`.
- An oscillator's drawer row shows its frequency at the right
  (`trailingLabel`) and its family on the second line (`detailLabels`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Exhaustive `PartSymbol` switches | 4 | § 2 second bullet: `grep -rn 'case \.capacitor' schematical/schematical --include='*.swift' \| grep -v Preferences` (quote the glob) → the sites in those four files |
| Exhaustive `Details` switches | 2 | `grep -rnE '^ +case let \.resistor' --include='*.swift' schematical/schematical` → 2 |
| Test files that keep a kind without a form | 0 since plan-46-C (every kind has a schema; F Phase 1 generated crystal's) | `grep -rln 'formless' schematical/schematicalTests` → 0 |
| Crystal-kind parts to list | `<add rows> + 4` | `grep -c '|crystal|' backend/tests/golden/parts.txt` |

## 3. Design

- **Details.** `Details.crystal(CrystalInfo)` from `part.crystal?.pointee`
  (`CrystalInfo`: `type` ("crystal" or "resonator"), `hz`, `loadFF`,
  `tolerancePPM`, `stabilityPPM`, `esrMOhm`, `capacitors`); `CatalogPart.crystal`
  and `CatalogPart.crystalHz` (the frequency of a crystal part, nil otherwise).
  `detailLabels`: the load capacitance ("18 pF", `CapacitorValue.text`) and the
  tolerance ("±20 ppm") when given. `trailingLabel`: `PartFacts.hertz(hz)`.
- **Facts rows.** Type ("Crystal" / "Ceramic resonator"), Frequency
  (`PartFacts.hertz`), Load capacitance (`CapacitorValue.text`, empty when
  none), Tolerance ("±20 ppm"), Stability, ESR ("80 Ω", "1.5 kΩ"),
  "Built-in capacitors" ("Yes", only for a resonator that has them).
  `kindName(_ kind:)` `PART_CRYSTAL` → "Crystal"; `kindName(_ part:)` →
  "Ceramic resonator" for a resonator.
- **Symbol.** `PartSymbol.crystal`: `symbol(for:)` returns it for
  `PART_CRYSTAL` parts whose pins include `X1` and `X2`, else `.block`.
  `ChipLayout.crystal(pins)`: `X1` on the left and `X2` on the right on the
  diode box's centre line (like `capacitor`), a pin named `GND` at the bottom
  centre (the three-pin resonator), every other pin (`NC`) not drawn, as the
  diode's no-connects. Falls back to the block layout when a terminal is
  missing. `Canvas/CrystalSymbolView.swift`: two plates of 14 pt with a
  rectangle of 8 × 14 pt between them (the usual crystal symbol), leads to the
  pins, the `GND` lead drawn down from the rectangle's centre for a three-pin
  part, the frequency text under it (`PartFacts.hertz`), `paint(_:chip:isSelected:isHovered:)`
  static for the export scene, highlight and flip as `CapacitorSymbolView`.
  Wired into `SchematicCanvasView`, `SchematicExportScene` and
  `ReferenceLabel`'s anchor (over the symbol, like the capacitor).
- **Reference.** `ReferenceCategory.of(_:)`: `case PART_CRYSTAL: return
  .crystal` (only if absent: `grep -c 'PART_CRYSTAL'
  schematical/schematical/Preferences/ReferencePrefixes.swift` → 0 before).
- **Drawer order.** `functionGroups`'s `value` closure gets a third case: when
  every part has `crystalHz`, leaves go by the smallest `hz` of each leaf (ties
  by name), parts in a leaf by `hz` then `nameRank` (`byValueThenName`, as
  `byCountThenName`). The kHz leaves are frequencies (`32 kHz` before
  `32.768 kHz`), the MHz and resonator leaves the bands of F § 3.1 (`Under 4
  MHz` first).
- **Play mode.** Nothing in the app: E's model drives the clock and the app's
  existing simulation runs it.

Where the risk concentrates: the symbol view and layout (a new geometry in
four switch sites) and the drawer sort (the drawer's performance tests must
stay green: `DrawerPerformanceTests`).

Rejected alternatives: (1) a per-instance frequency field on the generics
(rejected: a placed-part value needs a file-format field, and the catalog
holds 600 real frequencies); (2) drawing the three-pin resonator as a block
(rejected: the standard symbol is the crystal with a ground pin and a block
would hide that the part is a resonator).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, kind names, reference

- [x] `Backend/PartCatalog.swift`: `CrystalInfo`, `Details.crystal`, the
      builder branch, `crystal` / `crystalHz`, `detailLabels`, `trailingLabel`.
- [x] `Backend/PartFacts.swift`: the facts rows, `kindName(_ kind:)` and
      `kindName(_ part:)`.
- [x] `Preferences/ReferencePrefixes.swift`: `of(_:)` (if absent).
- [x] Tests: `schematicalTests/CrystalTests.swift` (from `ResistorTests`):
      `GENERIC-CRYSTAL-8M` kind name, facts and trailing label; the resonator's
      name and its Built-in capacitors row; `ReferenceCategory.of` → `.crystal`
      and the lowest free `Y<n>`.

Acceptance: the facts, names and reference are right on the real catalog.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p44-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/CrystalTests` → `** TEST SUCCEEDED **` (est. 8 min: build + one test class). Output 2026-11-22: `Executed 7 tests, with 0 failures`, `** TEST SUCCEEDED **` (`testPlacingTheGenericCrystal`, `testReferences`, `testTheGenericResonator`, `testCatalogFacts`, `testLayout`, `testRendering`, `testDrawerOrder`).
Commit: 7aa4897d

### Phase 2 — The symbol

- [x] `Canvas/SchematicModel.swift`: `PartSymbol.crystal` and
      `ChipLayout.crystal(_:)` (§ 3); `Backend/PartCatalog.swift`
      `symbol(for:)`.
- [x] `Canvas/CrystalSymbolView.swift` (§ 3); the three switch sites
      (`SchematicCanvasView`, `SchematicExportScene`, `ReferenceLabel`).
- [x] Tests (`CrystalTests`): `symbol(for:)` `.crystal` for both generics and
      for a catalog crystal; the layout puts `X1` left, `X2` right, `GND` at
      the bottom centre of the resonator and draws no `NC` pin of a four-pad
      part; a placed part renders (an image of the export scene is not empty).

Acceptance: crystals draw as crystals.
  Check: `xcodebuild … test -only-testing:schematicalTests/CrystalTests` → `** TEST SUCCEEDED **` (est. 8 min). Output 2026-11-22: as Phase 1 (the layout test runs over the 471 drawer-visible crystal-kind parts: X1 left, X2 right, no `NC`, `GND` at the bottom centre where there is one; the render test: the export scene's bounds are over 40 × 14 pt for the generic crystal, the resonator and a four-pad part).
Commit: 7aa4897d

### Phase 3 — Drawer order, the harness

- [x] `Backend/PartCatalog.swift` `functionGroups`: the `crystalHz` value
      order and the within-leaf order (§ 3).
- [x] `scripts/harness/crystal-app.swift` (from `transistor-app.swift`): the
      checks of § 1 on the app's real catalog, document and
      `SimulationController`; two 0.6 s runs: `GENERIC-CRYSTAL-32768` terminal 1
      → `SN74HC04N` 1A (VCC on 14, GND on 7) → 1Y → `GENERIC-RESISTOR` →
      `GENERIC-LED` → GND: the LED is lit; the same circuit with 1A on
      `PULLUP-5V` instead of the crystal (constant 1, output 0): the LED is
      dark. Renders a placed crystal, a resonator and a four-pad part to
      `/tmp`. Output: `all checks ok` (below).
- [x] Tests: `DrawerPerformanceTests` and `PartEditorWindowTests` stay green;
      `CrystalTests` gains the drawer order (the `MHz` leaves in band order,
      a leaf's parts in frequency order).

Acceptance: the drawer, the sidebar and the play-mode outcome work end to end.
  Check: `rm -rf /tmp/p44-harness && APP=/tmp/p44-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/crystal-app.swift /tmp/p44-harness && /tmp/p44-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/CrystalTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 15 min: app build, harness, three test classes). Output 2026-11-22: harness: `ok   1 drawer: Crystal → kHz ["32 kHz", "32.768 kHz"]; MHz ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz to 32 MHz", "32 MHz and over"]; Ceramic resonator ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz and over"]; Generic ["Generic"]`, `crystal parts in the drawer: 471`, `ok   1 each leaf's parts are in frequency order`, three `ok   2 …` lines (GENERIC-CRYSTAL-8M "Crystal", GENERIC-RESONATOR-8M "Ceramic resonator" with `Built-in capacitors: Yes`, ABS07-32.768KHZ-T with `Load capacitance: 12.5 pF`, `Tolerance: ±20 ppm`, `ESR: 70 kΩ`), `ok   3 GENERIC-CRYSTAL-8M placed: Y1 on HC-49/US`, `GENERIC-CRYSTAL-32768 → SN74HC04N → R → LED: light 0.49989590194328093` / `ok   4 … the LED lights`, `PULLUP-5V → … light 0.0` / `ok   4 … the LED stays dark`, `ok   5 images written`, `all checks ok`, exit 0; xcodebuild: `Executed 22 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 7aa4897d

## Validation Plan

- Tests: `CrystalTests`; `DrawerPerformanceTests`, `PartEditorWindowTests`
  unchanged and green.
- Coverage check: the harness runs the app's own catalog and
  `SimulationController`, so the new symbol, facts and sort run against the real
  parts.
- Runtime proof: the harness's LED circuit.
- Doc sync: none (`docs/file-format.md` unchanged); `docs/parts-format.md` is
  E's and F's.
- Final gate: plan-44-H.

## Decisions

- **The frequency of a generic is the part's** (§ 3 rejected alternative 1).
- **A four-pad crystal's `NC` pads are not drawn**, a three-pad resonator's
  `GND` pad is (plan-44-B § 3.3).
- **The part-editor form is generated** (plan-46's rule since plan-46-C:
  every kind has a schema; F Phase 1 generated `CrystalDraft.swift` and
  added the `KindInfo` entry). The Part Wizard derives its crystal steps
  from it; no curated guide (plan-46 curates the simple logic kinds only).
- **The symbol is the plate-rectangle-plate crystal symbol** for both crystals
  and resonators; the sidebar's kind name, not the symbol, says which it is.

## Corrections

1. **A resonator's `GND` pin needs stub room:** `PlacedChip.hasVerticalPins` is
   now also true for a crystal whose layout has a bottom pin, so its frame has
   the stub room above and below that supplies and turned parts get
   (`symbolFrameSize`, `symbolPoint`'s inset); `ReferenceLabel.anchor` adds the
   stub for it. A crystal does not rotate (`canRotate` is unchanged).
2. **`functionGroups(_:rank:)`** takes the drawer's name rank (default empty) to
   order a crystal leaf's parts by frequency then name; the two call sites pass it.
3. **`DrawerPerformanceTests.testPartsAreInNaturalOrder`** expects a leaf whose parts
   are all crystals by (frequency, name) instead of by name: the plan's drawer order.
4. **`TestSupport.allParts` follows the drawer's default filter**, which hides the
   133 AEC-Q200 (`automotive`) parts: `CrystalTests.testLayout` covers 471 of the 604.
5. **The tolerance text is ppm for every part:** a ceramic resonator's 0.5 % reads
   `±5000 ppm` (plan-44-B Corrections 3 converted DigiKey's percentages).
6. **The symbol:** two plates 16 pt apart with an 8 × 14 pt rectangle between them,
   a 5 pt gap from the plate to the rectangle; the GND lead from the rectangle to the
   body's bottom edge and the frequency beside it (read from the rendered
   `/tmp/p44-symbols.png`).

## Summary

G's risk is the new symbol geometry in four switch sites and the drawer sort,
which touches the drawer's performance-tested path. The harness and the
performance test bound both. Untouched: the engine, the part files, the file
format and the Bill of Materials.
