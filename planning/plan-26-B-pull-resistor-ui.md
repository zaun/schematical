# plan-26-B: Pull resistor Options, Settings, symbol value and BOM

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-26-A

plan-26-A made every placed pull-up and pull-down a resistor with `ohms` and
a footprint choice. This sub-plan adds the controls and the outputs: an
Options section in the Schematic sidebar (Ohms as a number plus a unit menu,
and Footprint), a "New Pull Resistor" default in Settings, the value drawn
next to the resistor on the schematic, and one Bill of Materials line per
resistance and package.

Outcome: select a pull-up → the sidebar shows Options with Ohms `1` `kΩ` and
Footprint "Radial (standing) resistor, 2.54 mm lead spacing". Typing `4.7`
and Return shows `4.7kΩ` beside the resistor on the canvas. Two pull-ups and
a pull-down at 4.7 kΩ / 0805 are one BOM line: `3`, `R1, R2, R3`,
`RESISTOR`, `0805`, `4.7kΩ`, `Resistor`.

References:

- `planning/completed/plan-26-A-pull-resistor-data.md` (the model this builds on)
- `planning/completed/plan-18-B-generic-led-ui.md` (precedent: LED Color and Footprint pickers, Settings default)
- `schematical/schematical/Drawer/PartOptionsSection.swift`
- `schematical/schematical/Canvas/LengthField.swift` (precedent: number field that commits on Return / focus loss)
- `schematical/schematical/Preferences/PreferencesView.swift` (`PreferenceKey`, `NewPartOptions`)
- `schematical/schematical/Canvas/SupplySymbolView.swift`
- `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift`

## Prerequisites

The gate for the whole feature is in plan-26-A § Prerequisites. This
sub-plan adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-26-A is complete | `ls planning/completed/plan-26-A-*` → one file, and `grep -c 'var ohms' schematical/schematical/Canvas/SchematicModel.swift` → 1 | MET (2026-11-07, re-run by follow-plan: `planning/completed/plan-26-A-pull-resistor-data.md`, `grep -c` → 1) |

If plan-26-A is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A selected pull resistor's sidebar has an Options section: Ohms (number
  field + unit menu Ω / kΩ / MΩ) and Footprint (the six choices).
- Settings has "New Pull Resistor" with the same two controls; new pulls use
  them.
- The schematic shows the value (`1kΩ`) beside each pull's resistor.
- The BOM lists pull resistors as `RESISTOR` lines grouped by value and
  package, with a Value column.

### Non-goals (explicit constraints)

- No change to the model, file format or backend (plan-26-A owns them).
- The pull symbol's size, pin and hit area are unchanged; the value text sits
  in the existing label margin.
- LED Options, LED Settings and every non-pull BOM line are unchanged, except
  that the CSV gains a Value column for every line (empty for non-resistors).
- No milliohm or gigaohm unit.

## 2. Current State

- `PartOptionsSection` (`Drawer/PartOptionsSection.swift`) shows Color (LEDs)
  and Footprint (`FootprintChoicePicker`, any part with `footprintChoices`).
  `applies(to:)` is already true for pulls once plan-26-A gives them choices,
  so the Footprint picker appears with no change; Ohms is the new row.
  Shown from `Drawer/SchematicSidebarView.swift` between Reference and Details.
- `LengthField` is a `TextField` that commits on Return or focus loss, puts
  the old text back on bad input, and shows the value the document kept.
- Settings: `PreferenceKey.genericLEDColor/Footprint` and `NewPartOptions`
  (only for `GENERIC-LED`); used at drop time in
  `SchematicCanvasView.swift:166–167`. The pull tools place at
  `SchematicCanvasView.swift` `case .vcc, .ground, .pullUp, .pullDown` with no
  options.
- `SupplySymbolView` draws the pull resistor as a zig-zag on x = 20, x from
  14 to 26, pull-up from y = 14 to the body height (80), pull-down from 0 to
  `ChipLayout.pullResistorHeight` (60). The frame has
  `ChipLayout.labelMargin` (30) each side of the 40-wide body
  (`SchematicModel.swift:160, 178`).
- BOM: `BillOfMaterials.lines(for:)` groups chips with a footprint by
  `partNumber`, `package`, `color`; columns Qty, References, Part Number,
  Package, Color (only if any line has one), Description; CSV header
  `Quantity,References,Part Number,Package,Color,Description`.

### Measured populations

| What | Count | Command |
|---|---|---|
| BOM tests | 3 | `grep -c 'func test' schematical/schematicalTests/BillOfMaterialsTests.swift` → 3 |
| Places that build a `BillOfMaterialsLine` | 2 | `grep -rn 'BillOfMaterialsLine(' schematical/` → `BillOfMaterialsWindow.swift` (`line(for:)`) and `BillOfMaterialsTests.swift` (CSV test) |
| Pull-tool placement sites | 1 | `grep -n 'case .vcc, .ground, .pullUp, .pullDown:' schematical/schematical/Canvas/SchematicCanvasView.swift` → 2 hits: line 250 draws the symbol, line 513 places the part (the 1 site) |

### Verified properties

- **The value fits the margin**: text starts at body x = 30 (4 past the
  zig-zag's right edge 26), leaving 40 + 30 − 30 = 40 pt before the frame
  edge (`labelMargin` 30, body width 40). The longest text is 7 characters
  (`1.234kΩ`, 3 decimals). SF Mono advances 0.6 em per character, so 9 pt
  gives 7 × 5.4 = 37.8 pt, which fits; 10 pt (42 pt) would be clipped. The
  label is 9 pt.
- **Pull options are the same on all 7 pull parts** (plan-26-A §6), so
  Settings can read the choices from `PULLDOWN`.

## 3. Design Overview

Four independent UI pieces over plan-26-A's model, all low risk. No
byte-identity gate; tests are unit tests on the formatting, BOM grouping and
`NewPartOptions`, plus one runtime check in the app.

Rejected alternatives:

- *A fixed E-series menu*: the owner chose a free number plus a unit.
- *Keeping the typed unit* (showing `1500 Ω` as typed): the field always
  shows the stored value normalized, so the sidebar, symbol and BOM agree.
- *Separate BOM lines per pull part number*: owner chose one `RESISTOR` line
  per value and package.

## 4. Value formatting (`Canvas/ResistorValue.swift`, added to plan-26-A's file)

- `enum Unit: CaseIterable { case ohm, kilo, mega }`, scale 1 / 1e3 / 1e6,
  symbol `Ω` / `kΩ` / `MΩ`.
- `split(_ ohms) -> (number: Double, unit: Unit)`: the largest unit whose
  number is ≥ 1 (below 1 Ω: `.ohm`).
- `numberText(_:)`: up to 3 decimals, trailing zeros trimmed (the
  `LengthField.format` rounding).
- `text(_ ohms)` = `numberText(number) + unit.symbol`: 1000 → `1kΩ`, 4700 →
  `4.7kΩ`, 470 → `470Ω`, 1e6 → `1MΩ`, 0.5 → `0.5Ω`, 1e9 → `1000MΩ`.
- `ohms(number:unit:) -> Double?`: number rounded to 3 decimals × scale; nil
  if the result is not `isValid`.
- `parse(_ text:) -> Double?`: `Double` of the trimmed text, nil if not finite.

## 5. Sidebar Options (`Drawer/PartOptionsSection.swift`)

- New view `OhmsField(ohms: Double, commit: (Double) -> Void)`: a labelled
  row "Ohms" with a trailing-aligned `TextField` (number) and a menu `Picker`
  (unit, labels hidden). It shows `split(ohms)`.
  - Number: commits on Return or focus loss (`LengthField` pattern). Text
    that doesn't parse, or gives an invalid value with the current unit, puts
    the shown number back.
  - Unit: picking one commits at once with the shown number × the new unit;
    an invalid result (e.g. 2000 MΩ) puts the menu back.
  - After a commit the row shows the normalized stored value (typing `1500`
    with `Ω` shows `1.5` `kΩ`).
- `PartOptionsSection.body`: when `current.part.isPullResistor`, an
  `OhmsField` bound to `current.ohms` / `document.setPartOhms`, placed before
  the existing Footprint picker.

## 6. Settings (`Preferences/PreferencesView.swift`, `SchematicCanvasView.swift`)

- `PreferenceKey.pullResistorOhms` (Double; 0 = never set → 1000) and
  `pullResistorFootprint` (String; "" → part default).
- `NewPartOptions.ohms(for:)` → stored value for a pull resistor if
  `ResistorValue.isValid`, else nil. `NewPartOptions.footprintID(for:)`
  returns `pullResistorFootprint` for a pull resistor, the LED key for
  `GENERIC-LED`, nil otherwise.
- New Settings section "New Pull Resistor" after "New Generic LED": an
  `OhmsField` and a `FootprintChoicePicker` (choices from `PULLDOWN`), footer
  "Used for pull-ups and pull-downs placed from now on."
- `SchematicCanvasView.swift` pull placement (the `case .vcc, .ground,
  .pullUp, .pullDown:` at line 513) passes `ohms: NewPartOptions.ohms(for: part)`
  and `footprintID: NewPartOptions.footprintID(for: part)`.

## 7. Value on the symbol (`Canvas/SupplySymbolView.swift`)

For `.pullUp` and `.pullDown`, draw `ResistorValue.text(chip.ohms ?? 1000)`
at `point(30, mid)` with `anchor: .leading`, 9 pt monospaced, `.primary`
(not the highlight color). `mid` is 47 for a pull-up (resistor 14…80) and
30 for a pull-down (0…60); `point` already mirrors for a flipped symbol.

## 8. Bill of Materials (`BillOfMaterials/BillOfMaterialsWindow.swift`)

- `BillOfMaterialsLine.value: String` (resistance text; empty for others),
  in `id` after `color`.
- `line(for:)`: a pull resistor's line is `partNumber "RESISTOR"`, package
  from its footprint choice, `value ResistorValue.text(ohms)`, `color ""`,
  `description "Resistor"`. So pull-ups at any voltage and pull-downs with
  the same value and package share a line.
- Table: a "Value" column after Package, shown when any line has a value
  (like Color).
- CSV header `Quantity,References,Part Number,Package,Value,Color,Description`;
  every row gets the value field.
- Update the doc comment ("Parts with no footprint (VCC, GND) …").

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Formatting and BOM

- [x] `schematical/schematical/Canvas/ResistorValue.swift`: §4.
- [x] `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift`: §8.
- [x] Tests: `PullResistorTests.swift` gains `ResistorValue` cases (the §4
      examples; `ohms(number: 2000, unit: .mega)` → nil; `ohms(number:
      1.2345, unit: .kilo)` → 1235 (1.2345 rounds to 1.235 × 1000 = 1235; check with
      accuracy 1e-9)). `BillOfMaterialsTests.swift`: new test — `PULLUP-5V`,
      `PULLUP-3V3`, `PULLDOWN` all at 4700 Ω and `RES-0805` → one line
      `RESISTOR`, `0805`, `4.7kΩ`, quantity 3, references `R1, R2, R3`; a
      fourth at 1000 Ω is a second line. `testCSVQuotesFieldsWithCommas`
      expects the new header and an empty Value field. `testGroupsByPartAndSkipsSupplies`
      still passes (GND still left out). (Added `PullResistorTests`
      `testResistorValueText`, `testResistorValueOhmsAndParse`;
      `BillOfMaterialsTests.testPullResistorsGroupByValueAndPackage`, which
      also checks the CSV row `3,"R1, R2, R3",RESISTOR,0805,4.7kΩ,,Resistor`.)

Acceptance: pull resistors are grouped by value and package in the BOM and CSV.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/PullResistorTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
  Result (2026-11-07): `Executed 13 tests, with 0 failures` (BOM 4,
  PullResistor 9), `** TEST SUCCEEDED **`.
Commit: 24d3fda

### Phase 2 — Sidebar, Settings and symbol

- [x] `schematical/schematical/Drawer/PartOptionsSection.swift`: `OhmsField`
      and the Ohms row (§5).
- [x] `schematical/schematical/Preferences/PreferencesView.swift`: keys,
      `NewPartOptions`, "New Pull Resistor" section (§6).
- [x] `schematical/schematical/Canvas/SchematicCanvasView.swift`: pull
      placement passes the Settings options (§6).
- [x] `schematical/schematical/Canvas/SupplySymbolView.swift`: the value
      label (§7); update the type's doc comment.
- [x] Tests: `PullResistorTests.swift` — `NewPartOptions` with a test
      `UserDefaults` suite: unset → nil/nil; `pullResistorOhms` 2200 and
      `pullResistorFootprint` `RES-1206` → 2200 / `RES-1206` for `PULLUP-5V`
      and nil for `SN74HC00D`; `pullResistorOhms` -5 → nil. (Also: the
      pull footprint key gives nil for `GENERIC-LED`.)
- [x] Added: `scripts/harness/pull-resistor-ui.swift` (kept), the runtime
      proof below run on the app's real document and views (see Corrections).

Acceptance: the Options section and Settings set a pull's value and
footprint, and the symbol shows it.
  Check: `xcodebuild … test -only-testing:schematicalTests/PullResistorTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min); then the runtime proof below.
  Result (2026-11-07): `Executed 10 tests, with 0 failures`, `** TEST
  SUCCEEDED **`. Runtime proof: `rm -rf /tmp/p26-h && scripts/harness/build.sh
  scripts/harness/pull-resistor-ui.swift /tmp/p26-h &&
  /tmp/p26-h/H.app/Contents/MacOS/harness /tmp/p26-img` → 8 `ok`, exit 0.
  Images read: sidebar Options shows Ohms `1` `kΩ` and Footprint "Radial
  (standing) resistor, 2.54 mm le…"; the canvas shows `4.7kΩ` right of the
  zig-zag after 4.7 kΩ; the PCB shows two 0805 pads labelled R1; the BOM row
  `1 R1 RESISTOR 0805 4.7kΩ Resistor`; Settings shows "New Pull Resistor"
  with `10` `kΩ` and Axial. The Pull-down placed with those Settings is R2,
  10kΩ, Axial; the saved file has `"ohms" : 4700`, `"ohms" : 10000`,
  `"footprint" : "RES-0805"`, `"footprint" : "RES-AXIAL-10.16"`; reopening
  gives the same reference, part, ohms, footprint and PCB centre for both.
Commit: —

## Validation Plan

- Tests: `ResistorValue` formatting/parsing and limits, BOM grouping and
  CSV, `NewPartOptions` (Phase 1 and 2 tests above).
- Coverage check: `grep -c 'ResistorValue\|isPullResistor' schematical/schematicalTests/PullResistorTests.swift schematical/schematicalTests/BillOfMaterialsTests.swift` → ≥ 1 in each file.
- Runtime proof (the `run` skill; move the mouse to (0, 0) when done, per
  `AGENTS.md`): build and launch the app, open a new schematic, place a
  Pull-up. Sidebar shows Options → Ohms `1` `kΩ`, Footprint Radial; the
  canvas shows `1kΩ`; the reference is `R1`. Type `4.7`, Return → canvas
  `4.7kΩ`. Pick Footprint 0805 → the PCB view shows a 2-pad 0805 at R1.
  Window ▸ Bill of Materials shows `RESISTOR`, `0805`, `4.7kΩ`. Set Settings
  ▸ New Pull Resistor to 10 kΩ / Axial, place a Pull-down → `10kΩ`, Axial.
  Save, close, reopen → all values kept. (est. 10 min; the only check that
  sees the SwiftUI controls and the PCB drawing together.)
- Doc sync: none beyond plan-26-A (no format change here).
- Final gate (once, after Phase 2): `xcodebuild -project
  schematical/schematical.xcodeproj -scheme schematical test
  CODE_SIGNING_ALLOWED=NO` → 0 failures, and `ctest --test-dir
  /tmp/p26-build -j8` → 100% passed (est. 6 min).

## Decisions

- Ohms is entered as a number field plus a unit menu (Ω, kΩ, MΩ) — decided
  by the owner, 2026-11-07. The field shows the stored value normalized to
  the largest unit with a number ≥ 1, up to 3 decimals (author's call, so the
  sidebar, symbol and BOM agree). (§4, §5)
- One BOM line per value and package for all pulls, part number `RESISTOR`,
  with a Value column — decided by the owner, 2026-11-07. (§8)
- The value is drawn on the schematic symbol — decided by the owner,
  2026-11-07. (§7)
- Settings gets a "New Pull Resistor" Ohms and Footprint default — decided
  by the owner, 2026-11-07. (§6)

## Corrections

- **Runtime proof run through the harness, not mouse clicks.** This session
  has no tool to drive the macOS GUI. Following plan-18-B's precedent, the
  check runs on the app's real `SchematicDocument`, `SchematicFile`,
  `NewPartOptions` and views (`SchematicSidebarView`, `SchematicCanvasView`,
  `PCBCanvasView`, `BillOfMaterialsTable`, `PreferencesView`), rendered with
  `Harness.renderHosted` / `render` and read as images. The Ohms field's
  Return is stood in for by its commit path (`ResistorValue.parse("4.7")` →
  `ohms(number:unit: .kilo)` → `setPartOhms`). Kept as
  `scripts/harness/pull-resistor-ui.swift` so it can be re-run.

## Summary

All UI work over plan-26-A's model; nothing here changes data or the
backend. The only fiddly part is the Ohms field's commit rules (number vs
unit menu), covered by `ResistorValue` tests and the runtime check.
