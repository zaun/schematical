# plan-42-I: The app — regulators in the drawer, the sidebar, the voltage check and play mode

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-42-H

I shows the regulator parts H added. The drawer lists them under **Power →
Regulators → Linear** (leaves `Adjustable`, `Fixed`, `Generic`,
`Multi-output`) and **Power → Regulators → Switching** (leaves `Boost`,
`Buck`, `Buck-boost`, `Charge pump`, `Multi-output`, `SEPIC`), as H's
`function` values (plan-42-H § 3.3). A regulator draws as a
block (the IC box with its pin names, as every IC). The sidebar shows the
kind "Voltage regulator" ("Dual voltage regulator", "Triple …", "Quad …" for
multi-output parts) and its facts. The app's voltage check and play mode
pass each placed `GENERIC-RESISTOR`'s own value to the backend, so an
adjustable regulator's output follows the resistors the user placed. The
behavioral outcome: placing `GENERIC-REG-3V3` gives `U1` on `TO-220-3`;
wiring 5V → IN, GND, OUT → the VCC of an SN74LVC1G04DBVR with its input tied low lights
an LED on its output in play mode; an SN74AUC1G04DBVR (0.8–2.7 V) on OUT is
refused with "3.3 V"; 24 V on a part's VIN is refused with its input
maximum; a
`GENERIC-REG-ADJ` with 240 Ω / 720 Ω `GENERIC-RESISTOR`s powers its OUT net
at 5 V, and changing the 720 Ω to 390 Ω makes it 3.28 V (the refusal text of
a 2.7 V part names the new value).

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` :386–395,
  `detailLabels` :676, the details builder :874, `symbol(for:)` :1149),
  `Backend/PartFacts.swift` (the facts rows :130–150, `kindName(_ kind:)`
  :211, `kindName(_ part:)` :219).
- `Simulation/Simulation.swift` (`SchematicNetlist` :17, `NetlistBuilder`
  :42, the netlist :200, `build` :375–409), `Simulation/VoltageCheck.swift`
  (:62–100, `message` :123–150).
- `Canvas/SchematicModel.swift` (`PlacedChip.ohms` :672–733),
  `Backend/PartCatalog.swift:534` (`hasOhmsField`).
- `Editor/KindSchema.swift`, `Editor/BundledPartFile.swift` (H's lines).
- `schematical/schematicalTests/TransistorTests.swift`,
  `scripts/harness/transistor-app.swift` (the test and harness
  precedents); `planning/completed/plan-40-H-transistor-app.md`.

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-H complete | `ls planning/completed/plan-42-H-*.md` → exists | NOT MET (2026-11-15: not started) |
| The generics and the parts are loaded | `grep -c '^GENERIC-REG-3V3|regulator|' backend/tests/golden/parts.txt` → 1; `grep -c '|regulator|' backend/tests/golden/parts.txt` → H's add count + 3 | NOT MET (2026-11-15: 0) |
| `circuit_set_resistance` and the `milliohms` argument exist (G) | `grep -c 'circuit_set_resistance' backend/include/circuit.h` → 1; `grep -c 'milliohms' backend/include/voltage.h` → at least 1 | NOT MET (2026-11-15: 0, 0) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p42-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-15 at 40ccd5fc; re-run before G) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/RegulatorTests
  -only-testing:schematicalTests/PartEditorWindowTests
  -only-testing:schematicalTests/BillOfMaterialsTests` passes (Phase 3).
- The harness (`scripts/harness/regulator-app.swift`) prints and checks the
  drawer paths and leaves above, `kindName` and the facts of `LM1117MPX-3.3/NOPB`
  (or H's most-stocked fixed linear part when that one is not added; the
  harness reads it from the parts file), of an adjustable part, of a
  switching part and of a dual part; `symbol(for:)` `.block`; the outcome
  above.

### Non-goals (explicit constraints)

- No new field in the schematic file (`docs/file-format.md` changes only the
  wording G made): a placed regulator saves what any placed part saves.
- No new symbol: regulators draw as blocks (§ Decisions).
- No glow or state on the symbol in play mode; the LED is the proof.
- No part-editor form for the `regulator` kind.
- The model and the check are G's; I adds no engine behavior.
- The reference prefix stays `U` (`ReferenceCategory.integratedCircuit`,
  the IC default): no Settings change.

## 2. Current State

- `PART_REGULATOR` (G) falls to `symbol(for:)`'s `default: return .block`
  (`PartCatalog.swift:1156`), to `kindName`'s `default: return ""`
  (`PartFacts.swift:212`), and `Details` has no regulator case; the two
  exhaustive switches over `Details` are `detailLabels`
  (`PartCatalog.swift:676`) and the facts rows (`PartFacts.swift:141`).
- `ReferenceCategory.of(_:)` (`Preferences/ReferencePrefixes.swift:72`) has no
  `PART_REGULATOR` case, so a regulator is `.integratedCircuit` (`U`) with no
  change.
- The drawer's top level is sorted by name (`PartCatalog.swift`
  `categories`, `sortedNames`); "Power" is new and sorts between
  "Oscillator" and "Processor". Up to three drawer levels, leaves grouped by
  `function` (plan-40-H § 2).
- The voltage check passes `nil` for the resistances (G Phase 2), and
  `Simulation.build` never calls `circuit_set_resistance`, so a placed
  `GENERIC-RESISTOR`'s value is not seen: the divider uses each generic's
  part value, 1000 Ω.
- `PlacedChip.ohms` holds a placed generic or pull resistor's value
  (`SchematicModel.swift:675`); `CatalogPart.hasOhmsField` says which parts
  have one (`PartCatalog.swift:534`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Exhaustive `Details` switches | 2 | `grep -rnE '^ +case let \.transistor' schematical/schematical/Backend` → 2 (`PartCatalog.swift:676`, `PartFacts.swift:141`) |
| `voltage_check(` calls in the app | 2 | `grep -rn 'voltage_check(' schematical/schematical` → `VoltageCheck.swift:81, 87` |
| `circuit_*` building calls in `Simulation.build` | 1 site | `grep -n 'circuit_add_part' schematical/schematical/Simulation/Simulation.swift` → `:378` |

## 3. Design

- **Details.** `Details.regulator(RegulatorInfo)` from
  `part.regulator?.pointee` (`RegulatorInfo`: `type`, `regulators`, `mixed`,
  `adjustable`, `voutMV: [UInt32]`, `vrefMV: [UInt32]`, `voutMaxMV`,
  `vinMinMV`, `vinMaxMV`, `dropoutMV`, `ioutMA`, `iqUA`, `fswHz`; 0 = not
  given). `detailLabels`: fixed `["3.3 V", "1 A"]` (outputs joined by "/" for
  a dual: `["1.8/3.3 V", "0.3 A"]`), adjustable `["ADJ", "1.5 A"]`.
- **Facts rows** (`PartFacts`): Type (`Linear`, `Buck`, `Boost`, `Buck-boost`,
  `Charge pump`, `SEPIC`; a mixed part lists each, "Buck + Linear"),
  Regulators (when more than 1), Output (`3.3 V`; `1.8 V / 3.3 V`;
  adjustable: `Adjustable, Vref 1.25 V` plus ` (to 37 V)` with `voutMax`),
  Input (`2.7–16 V` from `vinMin`/`vinMax`, `up to 15 V` with no minimum),
  Dropout (`1.2 V`, linear only), Current (`0.8 A`), Quiescent (`5 µA`),
  Switching (`2.1 MHz`). A row whose value is 0 is left out.
- **kindName.** `kindName(_ kind:)` `PART_REGULATOR` → "Voltage regulator";
  `kindName(_ part:)`: 2 → "Dual voltage regulator", 3 → "Triple voltage
  regulator", 4 → "Quad voltage regulator".
- **Symbol.** `symbol(for:)` gets an explicit `case PART_REGULATOR: return
  .block` (documents the choice; no new `PartSymbol`).
- **Resistances to the backend.** `SchematicNetlist` gains `ohms: [Double?]`
  (per part index: `chip.ohms` when `part.hasOhmsField` and the part is a
  `GENERIC-RESISTOR`, else nil); `Simulation.build` calls
  `circuit_set_resistance(circuit, part, ohms)` for each non-nil entry after
  `circuit_add_part`; `VoltageCheck.sortedIssues` builds a `[UInt64]` of
  milliohms (0 where nil) and passes it to both `voltage_check` calls. A
  pull resistor's `ohms` is not sent (a pull is a supply part, not a
  resistor link).
- **The messages.** The app writes its own refusal text
  (`VoltageCheck.message`, `VoltageCheck.swift:123–150`: a `switch` over
  `VoltageProblemKind` with a `default` "is rated" line). G adds
  `case VOLTAGE_REGULATOR_INPUT_OVER`: `Can't connect: U1 pin 3 (VIN) gets
  24 V; LM1117MPX-3.3/NOPB takes at most 15 V in.` The supply-clash case
  already orders the two voltages by which chip is the supply (`if case
  .supply … mv == issue.mv`); G makes the supply the problem's `part` for a
  supply–regulator clash and the lower-level regulator `part` for a
  regulator–regulator one, so the existing case prints the lower voltage
  first in both (`3.3 V (U1) and 5 V (U2) supplies on one net.`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, kindName

- [ ] `PartCatalog.swift`: `RegulatorInfo`, `Details.regulator`, the details
      builder, `detailLabels`, `symbol(for:)`'s explicit `.block` case.
- [ ] `PartFacts.swift`: the facts rows and both `kindName`s (§ 3).
- [ ] Tests: `schematicalTests/RegulatorTests.swift`: `kindName` and the
      facts of one fixed, one adjustable, one switching and one dual part
      from the loaded catalog (picked by `kind` and facts, not by name), the
      generics' `footprintChoices` (`TO-220-3`, `DPAK-3`), `detailLabels`.

Acceptance: the sidebar shows a regulator's kind and facts.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p42-dd test -only-testing:schematicalTests/RegulatorTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min: the app test build; no smaller check runs Swift).
Commit: —

### Phase 2 — The resistances in the voltage check and play mode

- [ ] `Simulation.swift`: `SchematicNetlist.ohms`, filled by
      `NetlistBuilder`; `build` calls `circuit_set_resistance` (§ 3).
- [ ] `VoltageCheck.swift`: the milliohms array to both `voltage_check`
      calls; the `VOLTAGE_REGULATOR_INPUT_OVER` message case (§ 3).
- [ ] Tests: `RegulatorTests`: a `GENERIC-REG-ADJ` netlist with two
      `GENERIC-RESISTOR`s of 240 and 720 Ω and an SN74AUC1G04 on OUT gives one
      issue with `mv` 5000; 240 / 390 Ω gives `mv` 3281; the same with a
      1.65–5.5 V part gives none at either; `VoltageCheck.message` of a 24 V
      VIN issue is the § 3 text; a 3.3 V regulator output on the 5V rail
      gives `3.3 V (U1) and 5 V (…) supplies on one net.`

Acceptance: a placed resistor's value moves the regulated level.
  Check: the Phase 1 test command with the new cases → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 3 — The harness and the BOM check

- [ ] `scripts/harness/regulator-app.swift` (as `transistor-app.swift`):
      the drawer (`Power` → `Regulators` → `Linear` / `Switching` and their
      leaves), the facts, `U1` on `TO-220-3` for `GENERIC-REG-3V3`, the LED
      lit through the regulator in play mode and dark with IN unwired, the
      adjustable divider outcome of the plan's opening paragraph.
- [ ] `BillOfMaterialsTests`: a regulator line reads `U1`, the part name,
      its manufacturer and footprint (the existing line format).

Acceptance: the real app shows and runs regulators.
  Check: `APP=/tmp/p42-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/regulator-app.swift /tmp/p42-harness && /tmp/p42-harness/H.app/Contents/MacOS/harness` → `all checks ok`, exit 0 (est. 6 min); `xcodebuild … test -only-testing:schematicalTests/RegulatorTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

## Validation Plan

- Tests: `RegulatorTests` (facts, kindName, labels, choices, the divider
  through the app's netlist), `BillOfMaterialsTests`, `PartEditorWindowTests`
  (the kind list H changed).
- Coverage check: the Phase 2 cases fail if `circuit_set_resistance` or the
  milliohms array is not passed (they expect 3281, which the 1000 Ω part
  value would not give).
- Runtime proof: the harness (Phase 3), driving the real app; the pointer is
  moved to (0, 0) at the end (AGENTS.md § Testing in the app).
- Doc sync: none beyond G's (`docs/file-format.md` wording).
- Final gate: plan-42-J.

## Decisions

- **Drawer: Power → Regulators → Linear / Switching**, leaves by function
  (author; the owner asked for the regulators, not for a layout; "Power" is
  the IC-family name DigiKey and the todo use).
- **Block symbol** (author): a regulator's pins vary from 3 to 20; the
  standard schematic draws them as a box with IN/OUT/GND, which the block
  already is.
- **Prefix `U`** (author): IEEE 315 has no regulator class; ICs are `U`
  (`ReferencePrefixes.swift` standard prefixes).
- **The part-editor form is generated** (plan-46's rule since plan-46-C:
  every kind has a schema; H Phase 1 generated `RegulatorDraft.swift` and
  the `KindInfo` entry). The Part Wizard derives the regulator's steps
  from it.

## Corrections

## Summary

The app work is presentational except for the resistances: the netlist and
the voltage check must carry each placed `GENERIC-RESISTOR`'s value, which
no code does today. The Phase 2 tests use a value (390 Ω) the part default
cannot produce, so a missed path fails them.
