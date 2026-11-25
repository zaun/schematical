# plan-48-H: App — details, facts, kind names, the drawer, placed values to the backend, the timer's play state

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-G

H makes the app show timers and op-amps: their facts in the sidebar, their
kind names, their place in the drawer; it sends the values of placed
resistors, capacitors and pull resistors to the backend so a 555 times from
them; and in play mode it shows what each timer is doing: its mode and
times in the sidebar, and a warning mark with the reason on a timer that is
not timing.

References:

- plan-48-E § 3.3 (`circuit_set_part_value`, `circuit_timer_state`,
  `TimerState`, `timer_reason_text`), plan-48-F § 3.1 (`PartOpamp`),
  plan-48-G § 3.1 (drawer paths), § 3.2 (the generics).
- `schematical/schematical/Backend/PartCatalog.swift`, `PartFacts.swift`,
  `Simulation/Simulation.swift`, `Simulation/SimulationEngine.swift`,
  `Drawer/SchematicSidebarView.swift`, `Canvas/ChipBlockView.swift`,
  `Preferences/ReferencePrefixes.swift`.
- `scripts/harness/resistor-app.swift`, `scripts/harness/build.sh` (the
  harness precedent); `planning/plan-44-G-crystal-app.md`.

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-G complete | `ls planning/completed/plan-48-G-*.md` → exists, or every G box ticked | MET (2026-11-24: every G box ticked) |
| The generics are in the catalog | `grep -c '^GENERIC-555|\|^GENERIC-OPAMP|' backend/tests/golden/parts.txt` → 2 | MET (2026-11-24: `grep -c '^GENERIC-555|\|^GENERIC-OPAMP|' backend/tests/golden/parts.txt` → 2) |
| The app does not know the kinds yet | `grep -c 'PART_TIMER\|PART_OPAMP' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift` → 0 and 0 | MET (2026-11-16; 2026-11-24 before this letter: 0 and 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog, document and `SimulationController`
(`scripts/harness/timer-app.swift`, every line `ok`):

- The drawer has **Analog → Timers** (functions `Generic`, `Single (555)`,
  `Dual (556)`) and **Analog → Op-amps** (`Generic`, and the sub-groups
  `Single`, `Dual`, `Quad` with the functions `General purpose`, `JFET
  input`, `CMOS`).
- `GENERIC-555` placed: reference `U1`, kind name "Timer", a block symbol
  with its eight pins; a `556` part's kind name is "Dual timer".
  `GENERIC-OPAMP`: kind name "Dual op-amp"; a quad part "Quad op-amp"; a
  single "Op-amp". The facts rows of § 3.2.
- A placed `GENERIC-OPAMP` leaves Run on (`SimulationAvailability.canStart`).
- Play mode, the astable circuit (`GENERIC-555`; `GENERIC-RESISTOR` 1 kΩ
  5V–DISCH and 10 kΩ DISCH–THRES/TRIG, set through the sidebar's Ohms
  field; `GENERIC-CAPACITOR` 100 nF to GND; RESET on 5V; OUT through a
  `GENERIC-RESISTOR` to a `GENERIC-LED` to GND): the OUT net changes and the
  LED has on-time; the sidebar's Timing rows read `Astable`, `687 Hz`,
  `52.4 % high`.
- The same circuit with the 10 kΩ resistor's DISCH end moved to 5V: OUT
  stays 0, the timer shows the warning mark, and the Timing rows read `Not
  timing` and the reason text of `TIMER_SHAPE`.

### Non-goals (explicit constraints)

- No new symbol: timers and op-amps draw as blocks (`PartSymbol.block`).
- No new reference category: both are integrated circuits (`U`).
- No analog display (no waveform, no capacitor voltage).
- No change to the saved file format: `PlacedChip.ohms` and `.farads`
  already exist and are already saved.
- A value changed while the simulation runs does not re-time a running
  timer (plan-48-E: the analysis runs at start).

## 2. Current State

- `CatalogPart.Details` (`Backend/PartCatalog.swift:346`…) has one case per
  kind with facts; the builder reads `part.<kind>?.pointee`
  (`PartCatalog.swift:817`, `:873`). `symbol(for:)` (`:1126`) returns
  `.block` for a kind it does not name. `ReferenceCategory.of`
  (`Preferences/ReferencePrefixes.swift:66`) returns `.integratedCircuit`
  ("U") by default. `PartFacts.kindName(_ kind:)` (`PartFacts.swift:179`)
  returns "" for a kind it does not name.
- The drawer groups parts by `drawer[0]`, sorted by name
  (`PartCatalog.swift`, `groups(matching:)`): "Analog" needs no list entry
  and sorts first.
- `SchematicNetlist` (`Simulation/Simulation.swift:17`) carries part names,
  nets, connections and switch states; `build(_:from:…)` (`:373`) sends
  them with `circuit_add_part`, `circuit_connect`, `circuit_set_switch`.
  No placed value is sent.
- `PlacedChip.ohms` / `.farads` (`Canvas/SchematicModel.swift:675`, `:679`)
  hold a placed generic resistor's, pull resistor's or capacitor's value;
  nil means the default (`ResistorValue.defaultOhms` 1000,
  `CapacitorValue.defaultFarads`).
- The sidebar's Details section lists `PartFacts.rows(for:)`
  (`Drawer/SchematicSidebarView.swift:76`).

### Verified properties

- **Run is off only for parts that are not simulated**
  (`SimulationAvailability.unsimulated`: `!$0.part.isSimulated`); an op-amp
  is simulated (plan-48-F), so it never turns Run off.
- **The app builds at 59b90a8f** (plan-48-A § Prerequisites).

## 3. Design

### 3.1 Catalog (`Backend/PartCatalog.swift`)

```swift
struct TimerInfo: Hashable, Sendable { let timers: Int; let process: String; let maxHz: UInt64 }
struct OpampInfo: Hashable, Sendable {
    let type: String; let amplifiers: Int; let railIn: Bool; let railOut: Bool
    let gbwHz: UInt64; let slewMvUs: UInt32; let vosUv: UInt32
}
```

`Details.timer(TimerInfo)`, `Details.opamp(OpampInfo)`; builder branches on
`part.timer?.pointee` / `part.opamp?.pointee`; accessors `timer`, `opamp`
(the `transistor` accessor's shape, `:544`). `detailLabels`: a timer's
process name (`"Bipolar"`, `"CMOS"`); an op-amp's gain bandwidth
(`PartFacts.hertz`) when given. No `trailingLabel`.

### 3.2 Facts and names (`Backend/PartFacts.swift`)

- `kindName(_ kind:)`: `PART_TIMER` "Timer", `PART_OPAMP` "Op-amp".
  `kindName(_ part:)`: "Dual timer" for 2 timers; "Dual op-amp" / "Quad
  op-amp" for 2 / 4 amplifiers.
- Timer rows: `Timers` (2 only), `Process` (Bipolar / CMOS), `Max
  frequency` (`hertz`).
- Op-amp rows: `Amplifiers` (2 or 4 only), `Input` (General purpose / JFET /
  CMOS), `Gain bandwidth` (`hertz`), `Slew rate` (`"0.5 V/µs"`), `Offset
  (max)` (`"7 mV"`, `"150 µV"`), `Rail-to-rail` (`Input and output`,
  `Output`, `Input`; no row when neither), each only when given; and `Play
  mode`: `Not simulated: drives nothing`.

### 3.3 Placed values (`Simulation/Simulation.swift`)

`SchematicNetlist` gains `var values: [(part: Int, value: UInt64)] = []`.
`SchematicNetlist.build(chips:…)` fills it for every chip with `ohms` or
`farads` set **and** for every generic resistor, pull resistor and generic
capacitor without one (the default): a resistor or pull part
`UInt64((ohms * 1000).rounded())` milliohms, a capacitor
`UInt64((farads * 1e15).rounded())` femtofarads. `build(_:from:…)` calls
`circuit_set_part_value` for each after the parts are added; a failure
fails the build like any other call.

### 3.4 Timer state (`Simulation/Simulation.swift`, `SimulationEngine.swift`)

```swift
struct TimerSection: Hashable, Sendable {
    let mode: TimerMode; let reason: TimerReason
    let firstPs: UInt64; let highPs: UInt64; let lowPs: UInt64
}
```

`SimulationEngine.timerSections(part:count:)` calls `circuit_timer_state`
under the engine's lock. `SimulationController.timerSections: [UUID:
[TimerSection]]` (by chip id) is filled once, right after a successful
start, for every placed timer, and cleared on stop.

`Backend/TimerText.swift` (pure functions, tested alone):

- `mode`: `Astable`, `One-shot`, `Latch`, `Not timing`.
- `frequency`: `PartFacts.hertz(UInt64((1e12 / Double(high + low)).rounded()))`.
- `duty`: `"52.4 % high"` (one decimal, `high / (high + low)`).
- `duration(ps)`: under 1 µs `"… ns"`, under 1 ms `"… µs"`, under 1 s `"…
  ms"`, else `"… s"`, three significant digits, trailing zeros dropped
  (`762 µs`, `1.1 ms`).
- `reason`: `String(cString: timer_reason_text(reason))`, first letter in
  upper case.

### 3.5 Views

- **Sidebar** (`Drawer/SchematicSidebarView.swift`): while the simulation
  runs and the selected chip is a timer, a `Section("Timing")` after
  Details. Per section (titled `Timer 1` / `Timer 2` on a dual): `Mode`;
  astable — `Frequency`, `Duty`, `High`, `Low`; one-shot — `Pulse`; latch —
  no more rows; not timing — `Reason`.
- **Block** (`Canvas/ChipBlockView.swift`): while the simulation runs, a
  timer chip with any section in `TIMER_STOPPED` draws the system
  `exclamationmark.triangle.fill` image (yellow, 12 pt) at the block's top
  right corner, with `.help(<reason>)` and the accessibility label `Not
  timing: <reason>`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, kind names, the drawer

- [x] `Backend/PartCatalog.swift` per § 3.1 (each case only if absent: the
      prerequisite's grep).
- [x] `Backend/PartFacts.swift` per § 3.2.
- [x] Tests: `schematicalTests/TimerOpampTests.swift` (from
      `ResistorTests`): `GENERIC-555` and `GENERIC-OPAMP` kind names, facts
      rows and detail labels; a dual timer's and a quad op-amp's names (the
      first such catalog part by name); `ReferenceCategory.of` →
      `.integratedCircuit` and the lowest free `U<n>`; `symbol(for:)` →
      `.block`; the drawer groups and functions of § 1; `isSimulated` true
      for an op-amp and `SimulationAvailability.unsimulated` empty for a
      document with one.

Acceptance: the facts, names, reference and drawer are right on the real
catalog.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p48-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TimerOpampTests` → `** TEST SUCCEEDED **` (est. 8 min: build + one test class).
  Result (derived data `/tmp/p48-dd2`): `TimerOpampTests` (4 tests: the generic 555, the dual and CMOS timers, the op-amps, the drawer) and `KindInfoTests` → `** TEST SUCCEEDED **`. MET.
Commit: —

### Phase 2 — Placed values, the timer state, the Timing section and the mark

- [x] `Simulation/Simulation.swift` per § 3.3 and § 3.4;
      `Simulation/SimulationEngine.swift` `timerSections(part:count:)`.
- [x] `Backend/TimerText.swift` per § 3.4.
- [x] `Drawer/SchematicSidebarView.swift` and `Canvas/ChipBlockView.swift`
      per § 3.5.
- [x] Tests (`TimerOpampTests`): `TimerText` for the times of plan-48-E § 6
      (k) (`687 Hz`, `52.4 % high`, `762 µs`, `693 µs`) and of (r) (`1.1
      ms`); the netlist's `values` for a document with a 4.7 kΩ generic
      resistor (4700000), a default pull-up (1000000) and a 100 nF generic
      capacitor (100000000); a started `SimulationController` on the
      astable document has `timerSections` `TIMER_ASTABLE` with `highPs`
      762461899 ± 1, and on the mis-wired document `TIMER_STOPPED`,
      `TIMER_SHAPE`.

Acceptance: a placed 555 times from the placed values and reports its state.
  Check: `xcodebuild … test -only-testing:schematicalTests/TimerOpampTests -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/ResistorTests -only-testing:schematicalTests/CapacitorTests` → `** TEST SUCCEEDED **` (est. 10 min).
  Result: those four classes and `KindInfoTests` → `** TEST SUCCEEDED **` (8 `TimerOpampTests` tests: + `testTimerText`, `testNetlistValues`, `testAstableCircuit`, `testMisWiredCircuit`). MET.
Commit: —

### Phase 3 — The harness

- [x] `scripts/harness/timer-app.swift` (from `resistor-app.swift`): the
      checks of § 1 on the app's real catalog, document and
      `SimulationController`; two 0.6 s runs (the astable document: the OUT
      net's changes above 0 and the LED's on-time above 0; the mis-wired
      document: 0 changes, LED dark, the mark's accessibility label
      present); renders the placed timer with its mark and a placed op-amp
      to `/tmp/p48-symbols.png` (also `/tmp/p48-timing.png`, the running mis-wired circuit with the mark, and `/tmp/p48-sidebar.png`, its Timing section).
- [x] `DrawerPerformanceTests` and `PartEditorWindowTests` stay green.

Acceptance: the drawer, the sidebar and the play-mode outcome work end to end.
  Check: `rm -rf /tmp/p48-harness && APP=/tmp/p48-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/timer-app.swift /tmp/p48-harness && /tmp/p48-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/TimerOpampTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 15 min: app build, harness, three test classes).
  Result: the harness (`/tmp/p48/harness.sh`: build, then `/tmp/p48-harness/H.app/Contents/MacOS/harness /tmp`) → every line `ok`, `all checks ok`, exit 0 (Timing rows `Astable, 687 Hz, 52.4 % high`; high 762 µs, low 693 µs; the mis-wired run `Not timing`, OUT not toggling, LED dark, mark label `Not timing: The resistors and capacitors on TRIG, THRES and DISCH do not form an astable or a one-shot circuit`); `TimerOpampTests`, `DrawerPerformanceTests`, `PartEditorWindowTests` → `** TEST SUCCEEDED **`. MET.
Commit: —

## Validation Plan

- Tests: `TimerOpampTests`; the neighbours whose code H touches
  (`PullResistorTests`, `ResistorTests`, `CapacitorTests`: the netlist;
  `DrawerPerformanceTests`: a new category).
- Coverage check: `TimerOpampTests` runs on the real catalog, so a missing
  generic or a wrong drawer path fails it.
- Runtime proof: the harness (§ 1), not a unit test: a blinking LED from
  placed values, and the reason on a mis-wired timer.
- Doc sync: none in `docs/` (the backend docs are E's and F's).
- Final gate: plan-48-I.

## Decisions

- Owner, 2026-11-16: the app says why a timer is not timing.
- Author: **where it says so**: a Timing section in the sidebar and a
  warning mark with the reason as its tooltip on the timer's block. No
  alert and no banner: an unrecognized timer is not an error of the run.
- Author: **blocks, not new symbols**: a 555 is drawn as a block in every
  schematic tool; an op-amp triangle would need one symbol per amplifier of
  a multi-amplifier part, which the app's one-symbol-per-part model does
  not have.
- Author: **every generic resistor, pull resistor and generic capacitor
  sends its value**, default included, so the backend and the sidebar never
  disagree.
- Author: **the timer state is read once, at start**, matching the
  analysis.

## Corrections

1. **`circuit_set_resistance` already sends a generic resistor's value** (for the regulators, plan-42-I,
   `SchematicNetlist.ohms`); `SchematicNetlist.values` is the new, wider list for the 555 analysis
   (generic resistors, pull resistors, generic capacitors). Both are sent; the backend reads the
   timer analysis' value first (plan-48-E Corrections 1).
2. **`SchematicNetlist.timerParts`** (part index, chip, number of sections) is new: the controller needs
   the chip → part map to read each timer's sections once at start (`SimulationEngine.timerSections`).
3. **The mark** is an overlay of `ChipBlockView` (the SwiftUI view), not drawn by `paint`: exports and
   the PCB view never show it; the canvas passes `timerWarning` only while the run is on.
4. **Kind tests with counts** that this letter's kinds changed were fixed in G (Corrections 4) and here
   (`KindInfoTests`: 49 kinds, the `other` group is `opamp`, `generic`).
5. **The harness' op-amp facts** include `Supply range` and `Input max` (the generic rows every part has).

## Summary

The one new data path is placed values → backend. Everything else is the
per-kind wiring every kind has. The risk is the netlist change, which the
resistor, pull-resistor and capacitor tests guard.
