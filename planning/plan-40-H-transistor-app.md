# plan-40-H: The app — transistors in the drawer, on the canvas, in the sidebar and the BOM

Last updated: 2026-11-14
Effort: large (3h–1d)
Depends on: plan-40-G

H shows the transistor parts G added. The drawer lists them under
**Discrete → Transistors → BJT** (leaves `Array`, `NPN`, `NPN Darlington`,
`NPN pre-biased`, `PNP`, `PNP Darlington`, `PNP pre-biased`) and **Discrete →
Transistors → MOSFET** (leaves `Array`, `N-channel`, `N-channel logic-level`,
`P-channel`, `P-channel logic-level`), with **Discrete → Transistors →
Generic** beside them. A single transistor draws with the standard schematic
symbol (a BJT: base bar, collector and emitter leads, the emitter arrow
pointing out for NPN and in for PNP; a MOSFET: gate plate, channel, drain and
source leads, the body arrow in for N-channel and out for P-channel), with its
part name beside it; an array draws as a block. The sidebar shows the kind
"Transistor" ("Transistor array" for arrays) with its ratings. Every
transistor-kind part takes the lowest free `Q<n>` reference. `GENERIC-NPN`,
`GENERIC-PNP`, `GENERIC-NMOS` and `GENERIC-PMOS` carry the footprint picker with
their five choices. The behavioral outcome: placing `GENERIC-NPN` gives `Q1`
on `TO-92`; wiring VCC, a 1 kΩ `GENERIC-RESISTOR`, a `GENERIC-LED` and the
NPN's collector, its emitter to `GND` and its base to a pull-up lights the LED
in play mode, and a pull-down on the base darkens it.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` :292–352,
  `detailLabels` :588, `symbol(for:)` :1057–1090, the details builder :770–830,
  `functionGroups`), `Backend/PartFacts.swift` (`kindName(_ kind:)`,
  `kindName(_ part:)`, the facts rows :130–140).
- `Canvas/SchematicModel.swift` (`PartSymbol` :110–135, the layout switch
  :225–240, `ChipLayout.diode` :490, `ChipLayout.capacitor` :475),
  `Canvas/SchematicCanvasView.swift` (:335–345), `Canvas/SchematicExportScene.swift`
  (:59–62), `Canvas/ReferenceLabel.swift` (the anchor switch :22–40),
  `Canvas/DiodeSymbolView.swift`, `Canvas/ResistorSymbolView.swift` (the
  symbol views H mirrors).
- `Preferences/ReferencePrefixes.swift` (`ReferenceCategory.transistor` "Q",
  `of(_:)` :72).
- `Drawer/PartOptionsSection.swift` (the footprint picker, :98),
  `BillOfMaterials/BillOfMaterialsWindow.swift` (`line(for:references:)`).
- `schematical/schematicalTests/ResistorTests.swift` and
  `scripts/harness/resistor-app.swift`, `diode-app.swift` (the test and harness
  precedents); `planning/completed/plan-34-G-resistor-app.md`,
  `plan-29-M-diode-app.md`.

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-G complete | `ls planning/completed/plan-40-G-*.md` → exists | MET (2026-11-15: archived, b6ece0b9) |
| The generics and the parts are loaded | `grep -c '^GENERIC-NPN|transistor|' backend/tests/golden/parts.txt` → 1; `grep -c '|transistor|' backend/tests/golden/parts.txt` → N + 4 | MET (2026-11-15: 1; 1246 = 1242 + 4) |
| The editor's kind lists name the kind (G Phase 1) | `grep -c '"transistor"' schematical/schematical/Editor/KindSchema.swift` → at least 2 | MET (2026-11-15: 2) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p40-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-15: `** BUILD SUCCEEDED **` with G's parts) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/TransistorTests
  -only-testing:schematicalTests/PartEditorWindowTests
  -only-testing:schematicalTests/BillOfMaterialsTests` passes (Phase 3).
- The harness (`scripts/harness/transistor-app.swift`) prints and checks: the
  drawer's `Transistors` has the function `Generic` and the subcategories `BJT`,
  `MOSFET` with the leaves above; `kindName` of `2N3904` is "Transistor" with
  the facts Type, Vceo, Ic, Power, hFE; of a MOSFET Type, Vds, Id, Rds(on),
  Vgs(th); of a dual array "Transistor array"; `symbol(for:)` is `.transistor`
  for single parts and `.block` for arrays; the outcome above (Q1, the LED
  lit and dark, a PMOS high-side switch lit).

### Non-goals (explicit constraints)

- No new field in the schematic file (`docs/file-format.md` is unchanged): a
  placed transistor saves what any placed part saves, including its
  footprint choice.
- No play-mode glow or state on the symbol (the capacitor and resistor
  precedent); the LED's light is the proof.
- No part-editor form for the `transistor` kind (§ Decisions).
- The simulation and the voltage check are F's; H adds no engine behavior.

## 2. Current State

- `PART_TRANSISTOR` falls to `symbol(for:)`'s `default: return .block`, to
  `kindName`'s `default: return ""`, and `Details` has no transistor case;
  the two exhaustive switches over `Details` are `detailLabels`
  (`PartCatalog.swift:624`) and the facts rows (`PartFacts.swift:136`)
  (`grep -rn '\.resistor(' --include='*.swift' schematical/schematical` → 6
  sites, 2 of them switches).
- Exhaustive `PartSymbol` switches: 4 (`SchematicModel.swift` layout,
  `SchematicCanvasView.swift`, `SchematicExportScene.swift`,
  `ReferenceLabel.swift` anchor) (`grep -rn 'case \.capacitor\|case \.resistor'
  --include='*.swift' schematical/schematical | grep -v Preferences` → the
  sites in those four files).
- `ReferenceCategory.transistor` exists with the prefix `Q`
  (`ReferencePrefixes.swift:8, 32, 61`); `ReferenceCategory.of(_:)` has no
  `PART_TRANSISTOR` case, so a transistor falls to `.integratedCircuit` (`U`).
- The footprint picker shows for any part with `footprintChoices`
  (`PartOptionsSection.swift:98`), so the generics need no new control.
- The drawer takes up to three levels and groups leaves by `function`
  (`PartCatalog.swift` `subgroups`, `functionGroups`), sorted by
  `localizedStandardCompare`, which gives the leaf order above.

### Measured populations

| What | Count | Command |
|---|---|---|
| Exhaustive `PartSymbol` switches | 4 | § 2 first bullets |
| Exhaustive `Details` switches | 2 | `grep -rnE '^ +case let \.resistor' --include='*.swift' schematical/schematical` → 2 |
| Test files that count part kinds | 1 (`PartEditorWindowTests`, G Phase 1) | `grep -rln 'formless' schematical/schematicalTests` → 1 |

## 3. Design

- **Details.** `Details.transistor(TransistorInfo)` from `part.transistor?.pointee`
  (`TransistorInfo`: `type` string, `elements`, `mixed`, `darlington`, `mosfet`,
  `logicLevel`, `vmaxMV`, `imaxMA`, `powerMW`, `hfe`, `vgsthMV`, `vgsMV`,
  `r1`/`r2` milliohms, `rdsonMilliohm`; 0 = not given). `detailLabels`: BJT
  `["40 V", "0.2 A"]`, MOSFET `["60 V", "0.3 A"]` (the maximum rating pair, as the
  diode's).
- **Symbol.** `PartSymbol.transistor`; `symbol(for:)`: `PART_TRANSISTOR` with
  one element and the pin names `B`, `C`, `E` or `G`, `D`, `S` → `.transistor`,
  else `.block` (arrays, and any part whose pins are named otherwise).
  Layout `ChipLayout.transistor(pins)`: the control pin on the left at the
  middle, the collector or drain at the top right, the emitter or source at
  the bottom right (the first pin of each name, as `ChipLayout.diode`; a tab
  pin of the same name is not placed). `TransistorSymbolView.paint`: BJT — a
  circle with the base bar and the two leads, the arrow on the emitter lead;
  MOSFET — the gate plate, the broken channel line, the drain and source leads,
  the body arrow on the source; the polarity from `Details.transistor.type`;
  the part name to the right, the reference above (`ReferenceLabel.anchor` for
  `.transistor`: above the symbol). No glow. The three canvas switches draw
  `.transistor`.
- **Facts.** `kindName(_ kind:)` `PART_TRANSISTOR` → "Transistor";
  `kindName(_ part:)`: more than one element → "Transistor array". Rows: BJT —
  Type (`NPN`, `PNP`, `NPN Darlington`, `NPN pre-biased`, …), Vceo (`40 V`),
  Ic (`0.2 A`), Power (`0.625 W`), hFE (`100`), R1 / R2 (`10 kΩ`, pre-biased
  only); MOSFET — Type (`N-channel`, `N-channel logic-level`, …), Vds, Id,
  Rds(on) (`5 Ω`, `8 mΩ`), Vgs(th) (`≤ 2.1 V`), Vgs (`±20 V`), Power; arrays
  add Elements and, for a complementary pair, `NPN + PNP`.
- **References.** `ReferenceCategory.of(_:)`: `case PART_TRANSISTOR: return
  .transistor` (the standard prefix `Q`, lowest free number, as the resistor
  and capacitor).
- **BOM.** A transistor's line is the generic one (part number, package,
  description); no value column (`value` stays empty).
- **The rating facts and the voltage message** need no app work: the sidebar
  reads `part.transistor`, and `VoltageCheck` already prints
  `circuit_voltage_describe` text.

Where the risk is: the symbol's pin placement (a pin left unplaced or wired to
the wrong lead) and the four exhaustive switches. The tests place every kind
of part (single BJT, Darlington, pre-biased, N and P MOSFET, a tab part with a
same-named pin, an array, the generics) and check each pin has a slot.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, references, symbol

- [x] `PartCatalog.swift`: `Details.transistor`, `TransistorInfo`, its builder
      (`part.transistor?.pointee`), `detailLabels`, `symbol(for:)`.
- [x] `PartFacts.swift`: "Transistor", "Transistor array", the rows (§ 3), the
      text helpers (`milliohmsText`, `voltsText` reuse `PartFacts.milli` and
      `ResistorValue.text`).
- [x] `Preferences/ReferencePrefixes.swift`: `case PART_TRANSISTOR: return
      .transistor` in `ReferenceCategory.of(_:)` (only if absent:
      `grep -c 'PART_TRANSISTOR' schematical/schematical/Preferences/ReferencePrefixes.swift` → 0).
- [x] `SchematicModel.swift`: `PartSymbol.transistor` and `ChipLayout.transistor`
      in the layout switch; `Canvas/TransistorSymbolView.swift` (new); the
      `.transistor` case in `SchematicCanvasView.swift`,
      `SchematicExportScene.swift` and `ReferenceLabel.swift`.

Acceptance: the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p40-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min).
  Result (2026-11-15): `** BUILD SUCCEEDED **`.
Commit: c20f84c2

### Phase 2 — Drawer order, BOM, generics

- [x] No sort code: the drawer sorts leaves by `localizedStandardCompare`
      (`functionGroups`), which orders G's `function` strings as the Goal
      lists them (`Array`, `NPN`, `NPN Darlington`, `NPN pre-biased`, `PNP`, …;
      `Array`, `N-channel`, `N-channel logic-level`, `P-channel`, …); Phase 3's
      `TransistorTests` pins that order.
- [x] `BillOfMaterialsWindow.swift`: no transistor branch is needed (the
      generic line applies); a test pins it (Phase 3).
- [x] The generics' footprint picker: the five choices appear in G's order
      (`TO-92` first) and `setPartFootprint` saves the choice.

Acceptance: the drawer and the picker are right.
  Check: the Phase 3 tests for the drawer and the picker (est. 5 min).
  Result (2026-11-15): `TransistorTests` `testDrawer` (BJT leaves `Array, NPN, NPN Darlington, NPN
  pre-biased, PNP, PNP Darlington, PNP pre-biased`; MOSFET `Array, N-channel, N-channel
  logic-level, P-channel, P-channel logic-level`; `Generic` beside them), `testBillOfMaterials`
  (two TO-92 GENERIC-NPN share a line, value empty; the SOT-23 one its own),
  `testPlacingTheGenericNPN` (choices `TO-92, SOT-23, SOT-223, TO-220, DPAK`) and
  `testFootprintChoiceRoundTrip` (SOT-23 kept after save and reopen) pass. No code: the drawer,
  BOM and picker needed none.
Commit: (this commit)

### Phase 3 — Tests and the harness

- [x] `schematical/schematicalTests/TransistorTests.swift` (as `ResistorTests`):
      `GENERIC-NPN` placed → `Q1`, footprint `TO-92`, choices 5; a second
      transistor and a third → `Q2`, `Q3`; `kindName` and facts of `2N3904`, a
      logic-level MOSFET, a pre-biased part (R1/R2 rows) and a dual array;
      `symbol(for:)` `.transistor` for a single BJT, a Darlington, a MOSFET and
      a tab part (`G,D,S,D`: three slots), `.block` for the array; the drawer
      leaves of `BJT` and `MOSFET` and the `Generic` leaf; the BOM: two
      `GENERIC-NPN` on `TO-92` share a line, one on `SOT-23` is its own;
      choosing `SOT-23` and saving then reopening restores it; every pin of
      every placed kind has a layout slot.
- [x] `PartEditorWindowTests` unchanged and passing (G Phase 1 edited it).
- [x] `scripts/harness/transistor-app.swift` (as `resistor-app.swift`):
      § 1's harness checks; the LED runs in the real `SimulationController`
      (each its own 0.6 s document run, as `diode-app.swift`): `GENERIC-NPN`
      low-side (collector to the LED's cathode, the LED's anode to VCC through
      `GENERIC-RESISTOR`, emitter `GND`, base on `PULLUP-5V` → lit; on
      `PULLDOWN` → dark), `GENERIC-PMOS` high-side (source VCC, gate `GND` →
      drain 1 → lit); offscreen renders of a placed NPN, a MOSFET and an array
      (`Harness.renderCanvas`), looked at.

Acceptance: the tests pass and the harness prints `all checks ok`.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p40-dd test -only-testing:schematicalTests/TransistorTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`; `APP=/tmp/p40-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/transistor-app.swift /tmp/p40-harness && /tmp/p40-harness/H.app/Contents/MacOS/harness` → `all checks ok`, exit 0 (est. 15 min: the harness build is the slow part; nothing smaller runs the app's catalog code and the backend together).
  Result (2026-11-15): `Executed 10 tests, with 0 failures` (TransistorTests), 22 (BillOfMaterialsTests),
  8 (PartEditorWindowTests) → `** TEST SUCCEEDED **`; the harness prints `all checks ok` (the drawer;
  2N3904BU, IRLML6244TRPBF, BC846SZ kind / symbol / facts; GENERIC-NPN `Q1` on `TO-92`; the NPN low
  side lit on PULLUP-5V and dark on PULLDOWN; the PMOS high side lit; `/tmp/p40/p40-symbols.png` and
  `p40-sidebar.png` looked at: the NPN with its emitter arrow out, the N-channel MOSFET with the body
  arrow in, the part names beside them, the array a block with B1 B2 / C1 C2 E1 E2; the sidebar shows
  Kind Transistor, Function, Package, Footprint and the ratings).
Commit: (this commit)

## Validation Plan

- Tests: `TransistorTests` (new), `PartEditorWindowTests`,
  `BillOfMaterialsTests` and `ReferencePrefixTests` (unchanged, must pass).
- Coverage check: the harness prints one line per § 1 check.
- Runtime proof: the LED runs in the real `SimulationController`.
- Doc sync: none (no format change).
- Final gate: plan-40-I.

## Decisions

- **One symbol for BJT and MOSFET families, the type from the part** —
  author: the polarity is a part fact; Darlington and pre-biased parts draw as
  the plain BJT (the sidebar names them).
- **Arrays as blocks** — author: their pin maps vary (4–8 pins, mixed types);
  the block layout already draws any pin set with names.
- **No play-mode state on the symbol** — author: the resistor and capacitor
  precedent; the LED's light is the observable.
- **Every transistor-kind part is `Q<n>`** — author: the reference category
  `.transistor` exists.
- **No part-editor form for the `transistor` kind** — author: plan-32 covers the
  existing kinds; the resistor, capacitor and mcu kinds are formless the same
  way (`PartEditorWindowTests`).
- Owner decisions: plan-40-A § Decisions.

## Corrections

1. **Phase 1 ran before G (2026-11-15).** Phase 1 needs only the kind
   (`PART_TRANSISTOR`, `PartTransistor`: plan-40-F, landed 5f3c9a25), not
   G's parts, so it was done while plan-40-A's downloads ran; Phases 2–3
   wait for G as written. The Prerequisites rows naming G hold for Phases 2–3.
2. **Phase 1 choices.** A repeated same-name pin (a tab, `G,D,S,D`) gets no
   slot of its own, as `ChipLayout.diode` (three slots; Phase 3's test
   expects three). Resistances print with a space (`8 mΩ`, `5 Ω`, `10 kΩ`,
   as § 3's examples; `milliohmsText` reuses `ResistorValue.split`); a mixed
   MOSFET pair's Type is `N-channel + P-channel` (§ 3 gives only `NPN +
   PNP`); Darlington wins over pre-biased in the Type text; the symbol body
   is 80 × 60 so a part name fits between the right-hand leads. The new
   `TransistorSymbolView.swift` needs no project-file edit (folder-synced
   groups).
3. **Tests find most parts by their facts.** `TransistorTests` uses the
   named parts the final pick holds (2N3904BU, IRLML6244TRPBF, BC846SZ,
   2N7000) and finds a pre-biased part, a Darlington, a complementary pair
   and a `G,D,S,D` tab part by their `TransistorInfo`, so a later pick
   change does not break them; the BOM test's partner uses
   `GENERIC-NPN` only (no value column, as § 3).

## Summary

H is local: one details case, one symbol and its layout, the facts, the
reference category and tests. The coupling is the four exhaustive `PartSymbol`
switches and the two `Details` ones, which the build finds if one is missed.
