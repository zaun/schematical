# plan-29-M: The app — diodes in the drawer, on the canvas and in the sidebar

Last updated: 2026-11-09
Effort: medium (1h–2h)
Depends on: plan-29-H (Phase 1), plan-29-L (Phase 2) — corrected 2026-11-09, § Corrections

M shows the diode parts: the drawer lists them under **Discrete → Diodes →
Rectifier / Fast recovery / Switching / Schottky / Zener / Bridge** (that
comes from the data alone), a single diode draws as a diode symbol (plain,
Schottky or Zener cathode bar; no light arrows), arrays and bridges draw as a
block, and the sidebar shows the kind "Diode" with its facts. In play mode a
placed diode passes logic one way (plan-29-H); nothing lights.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` :205,
  `detailLabels` :407, details built at :531–540, `symbol(for:)` :731,
  `listsPartsDirectly` :979).
- `Backend/PartFacts.swift` (`kindName` :118, the facts rows above it).
- `Canvas/SchematicModel.swift` (`PartSymbol` :113, `ChipLayout.init(part:symbol:)`
  :210, `diode(_:)` :428), `Canvas/DiodeSymbolView.swift`,
  `Canvas/SchematicCanvasView.swift` (:270, the symbol switch),
  `Canvas/SchematicExportScene.swift` (:40).
- `planning/completed/plan-24-I-converter-app.md` (the last new-kind app
  letter), plan-27-J (open; edits `kindName` and `PartSymbol` too, § 2).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-L complete | `ls planning/completed/plan-29-L-*.md` → exists | NOT MET (2026-11-08: plan written) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p29-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-08 at 8cce726: `** BUILD SUCCEEDED **`; re-run, H–L have landed since) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`.
- In the app harness (plan-24-I's: the app's sources and backend, a custom
  `main.swift` in a copy of the Debug .app): `kindName` of `1N4001-E3/54`
  (or the first Vishay `add` part) is "Diode"; its facts list Type
  "Rectifier", Reverse voltage "50 V", Forward current "1 A"; a Zener's list
  Zener voltage and Power; `symbol(for:)` is `.plainDiode` for single diodes
  and `.block` for arrays and bridges.
- Runtime: the harness's `SimulationController` runs a diode-OR on 5 V (two
  SN74HC04N outputs → the anodes of two Switching diodes, at planning time
  Taiwan Semi `1N4148 A0G`; common cathode → `PULLDOWN` and `GENERIC-LED`'s
  anode, its cathode to `GND`) and the LED lights while either inverter's
  output is 1.

### Non-goals (explicit constraints)

- No backend change; no part-data change.
- The LED symbol (`.diode`) and its play-mode glow are unchanged.
- No new window, no drawer filter menu.

## 2. Current State

- The drawer's tree comes from `drawer` + `function` (no code per kind):
  diodes appear under Discrete → Diodes once L has landed.
- `CatalogPart.Details` has no diode case: a `PART_DIODE` part would fall
  through to the default the converters fell into before plan-24-I
  (plan-24-I § Corrections: they had fallen through to `.supply`).
- `PartSymbol.diode` is the LED symbol: `ChipLayout.diode` puts the pin
  named `A` left and `K` right (`SchematicModel.swift:428`);
  `DiodeSymbolView.paint` draws the triangle, the bar, two light arrows and
  a glow from `level`.
- Exhaustive switches over `PartSymbol`: 3
  (`grep -rn 'case .barrelJack' schematical/schematical | wc -l` → 3:
  `SchematicModel.swift:215`, `SchematicExportScene.swift:40`,
  `SchematicCanvasView.swift:278`).
- **plan-27-J (open) edits the same places**: it replaces `kindName(_ kind:)`
  with `kindName(_ part:)` and adds `PartSymbol.tactileSwitch`. M's edits are
  made against whichever is there: the `case PART_RTC: return "RTC"` line
  exists in both versions (plan-27-J § 4.1 keeps the kind switch for the
  other kinds), and a new `PartSymbol` case is added beside whatever cases
  exist. No step of M depends on plan-27's state.

## 3. Design

- `Details.diode(type: String, diodes: Int, vrMV: UInt32, ioMA: UInt32,
  vzMV: UInt32, pdMW: UInt32)`, built from `part.diode?.pointee` beside the
  LED's branch. `detailLabels`: `["<vr> V", "<io> A"]`, a Zener
  `["<vz> V", "<pd> W"]` (numbers via `VoltageCheck.volts`-style trimming:
  `50 V`, `0.5 A`).
- `PartSymbol.plainDiode`; `symbol(for:)`: `PART_DIODE` with one diode and
  pins named `A` and `K` → `.plainDiode`, else `.block`.
- `ChipLayout.init(part:symbol:)`: `.plainDiode` → `Self.diode(pins)` (same
  layout as the LED; a second `K` pin, a tab, is not drawn).
- `DiodeSymbolView.paint` gains `mark: DiodeMark` (`.led`, `.plain`,
  `.schottky`, `.zener`); `.led` draws exactly what it draws now; the
  others skip the arrows and glow and draw the cathode bar plain, with two
  hooks (Schottky: the bar's ends turned back 3 pt, as the standard S), or
  with one end bent (Zener: the bar's ends turned opposite ways, the Z).
  The mark comes from `Details.diode.type` (`schottky`, `zener`, else
  plain). The three `PartSymbol` switches draw `.plainDiode` with it.
- `PartFacts`: `case PART_DIODE: return "Diode"` in the kind switch; rows:
  Type (`Rectifier`, `Fast recovery`, `Switching`, `Schottky`, `Zener`,
  `Bridge`), Diodes (`n`, arrays only), Reverse voltage, Forward current;
  Zeners: Zener voltage, Power.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, symbol

- [x] `PartCatalog.swift`: `Details.diode`, its builder, `detailLabels`,
      `symbol(for:)` (§ 3).
- [x] `SchematicModel.swift`: `PartSymbol.plainDiode`, its layout case.
- [x] `DiodeSymbolView.swift`: `DiodeMark` and the three new marks;
      `SchematicCanvasView.swift`, `SchematicExportScene.swift`: the
      `.plainDiode` case. `DiodeMark(part:)` picks the mark from
      `Details.diode.type`; `paint(…, mark:)` defaults to `.led`, so the
      LED callers are unchanged.
- [x] `PartFacts.swift`: `"Diode"` and the rows. Numbers use a new
      `PartFacts.milli` (three decimals, trimmed: `0.075 A`), since
      `VoltageCheck.volts` rounds to two. Checked: `xcodebuild … -derivedDataPath
      /tmp/p29w-dd … build` → `** BUILD SUCCEEDED **` (2026-11-09).

Acceptance: the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p29-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 2 — Harness proof

- [ ] The plan-24-I harness: print `kindName`, facts and symbol for a
      Vishay rectifier, a Schottky, a Zener, an array and a bridge (§ 1);
      run the diode-OR circuit and print the LED's light per 1 µs step
      while the two inverter inputs walk 00, 01, 10, 11; render a placed
      1N4148, a Schottky and a Zener with `screencapture -l` and look at the
      PNG.

Acceptance: the printed facts and lights match § 1 (LED off only for
inverter inputs 11, i.e. both outputs 0), and the three symbols show their
marks.
  Check: the harness's output lines and the PNG (est. 15 min: the harness
  build is the slow part; nothing smaller runs the app's catalog code and
  the backend together).
Commit: —

## Validation Plan

- Tests: the build; the harness (no Swift test target exists for the
  catalog, as in plan-24-I).
- Coverage check: the harness prints a line for each of the five diode
  shapes (single plain, Schottky, Zener, array, bridge).
- Runtime proof: Phase 2's diode-OR run.
- Doc sync: none (the app has no user docs for parts).
- Final gate: plan-29-N.

## Decisions

- **Arrays and bridges as blocks** — author: their pin maps vary (3–8
  pins); the block layout already draws any pin set with names.
- **No light for diodes** — author: a diode is not an indicator; the LED
  keeps its own symbol and glow.
- **No filter menu** — author: the drawer's package filter and leaves are
  enough for 9–13 thousand parts split six ways.

## Corrections

- **Phase 1 needs only H** (2026-11-09) — Phase 1 reads the `PartDiode`
  struct and `PART_DIODE` (plan-29-H) and no part; it landed right after H,
  before the parts. Phase 2 (the harness on real parts) still waits for L.
  The "plan-29-L complete" prerequisite applies to Phase 2.

## Summary

M is small and local: one details case, one symbol case drawn by an extended
`DiodeSymbolView`, one kind name. The only coupling is plan-27-J editing the
same switch statements; M's edits are written to land on either version.
