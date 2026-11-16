# plan-47-M: The app — the four kinds in the drawer and sidebar; inductor and fuse symbols; blowing a fuse

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-L

M shows the parts K and L added. For all four kinds it adds the facts
(sidebar rows, drawer second line and trailing label), the kind names, the
reference prefixes and the drawer's value order. For inductors and fuses it
adds the schematic symbols and the fuse's play-mode behavior: a click on a
fuse blows it (its element draws broken and it stops conducting), a second
click restores it, and every play starts with every fuse intact. Relays and
buzzers draw as blocks until N. The behavioral outcome: the drawer lists
**Discrete → Inductors**, **Discrete → Fuses**, **Discrete → Resettable
fuses**, **Relay** and **Indicators → Buzzers** with value-ordered leaves; a
placed inductor is `L1`, a fuse `F1`, a relay `K1`, a buzzer `LS1`; in play
mode an LED fed from a gate through an inductor and a fuse lights, goes dark
when the fuse is clicked and lights again on the second click.

References:

- `schematical/schematical/Backend/PartCatalog.swift` (`Details` `:331`,
  `detailLabels`, `trailingLabel`, `symbol(for:)` `:1126`, `functionGroups`
  and its `value` closure `:1678`), `Backend/PartFacts.swift`
  (`kindName(_ kind:)` `:179`, `kindName(_ part:)` `:220`, the facts rows,
  `hertz` `:394`).
- `Canvas/SchematicModel.swift` (`PartSymbol` `:114`, `ChipLayout`),
  `Canvas/SchematicCanvasView.swift` (`toggleSwitchLever` `:936`, the `.pan`
  click in play mode `:903`), `Canvas/SchematicExportScene.swift`,
  `Canvas/ReferenceLabel.swift`, `Canvas/ResistorSymbolView.swift` and
  `CapacitorSymbolView.swift` (the two-terminal symbol views M mirrors),
  `Canvas/ResistorValue.swift`, `CapacitorValue.swift` (value text).
- `Simulation/Simulation.swift` (`SchematicNetlist.positionCounts` `:28–32`,
  its builder `:178–200`, `setSwitch` `:505`), `SimulationEngine.swift`
  (`setSwitch` `:263`).
- `Preferences/ReferencePrefixes.swift` (`fuse` "F", `relay` "K",
  `inductor` "L", `speaker` "LS"; `of(_:)` `:73`).
- `schematicalTests/ResistorTests.swift`, `CapacitorTests.swift`,
  `DrawerPerformanceTests.swift`; `scripts/harness/resistor-app.swift`,
  `transistor-app.swift`, `build.sh`.
- `planning/completed/plan-34-G-resistor-app.md`,
  `planning/completed/plan-40-H-transistor-app.md` (the precedents).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-L complete | `ls planning/completed/plan-47-L-*.md` → exists | NOT MET (2026-11-16: not started) |
| The parts are loaded | `grep -c '|inductor|\||fuse|\||relay|\||buzzer|' backend/tests/golden/parts.txt` → the `add` rows (2953 at planning time) | NOT MET (2026-11-16: 0) |
| The editor's kind lists name the four kinds | `grep -c '"inductor"\|"fuse"\|"relay"\|"buzzer"' schematical/schematical/Editor/KindInfo.swift` → at least 4 | NOT MET (2026-11-16: 0) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-16 at 349f25b3: `** BUILD SUCCEEDED **`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/InductorFuseTests
  -only-testing:schematicalTests/DrawerPerformanceTests
  -only-testing:schematicalTests/PartEditorWindowTests` passes.
- The harness (`scripts/harness/inductor-fuse-app.swift`) prints and checks:
  the drawer paths and leaf order of § 3; `kindName` and the facts of one
  real part per kind and type; `symbol(for:)` `.inductor` / `.fuse` for
  two-terminal parts; the references `L1`, `F1`, `K1`, `LS1`; and the LED
  circuit of the outcome above: lit, dark after the fuse is blown, lit again.

### Non-goals (explicit constraints)

- No new field in the schematic file (`docs/file-format.md` is unchanged): a
  blown fuse is play-mode state, cleared when play stops and never saved.
- No relay or buzzer symbol, state or sound (N).
- No new window, panel or menu: the four kinds use the drawer, the sidebar
  and the canvas that exist.
- No part-editor work beyond what K and L generated.
- No Bill of Materials change: a cartridge fuse's clips are not a BOM line
  (plan-47-A § Decisions).
- The simulation is I's and J's; M adds no engine behavior.

## 2. Current State

- `PART_INDUCTOR`, `PART_FUSE`, `PART_RELAY` and `PART_BUZZER` fall to
  `symbol(for:)`'s `default: return .block`, to `kindName`'s default (an
  empty name) and to the details builder's last branch; the exhaustive
  switches over `Details` are `detailLabels` and the facts rows.
- Exhaustive `PartSymbol` switches: the layout (`SchematicModel.swift`),
  `SchematicCanvasView.swift`, `SchematicExportScene.swift` and
  `ReferenceLabel.swift`'s anchor.
- `ReferenceCategory` has `.fuse`, `.relay`, `.inductor` and `.speaker` with
  their prefixes; `of(_:)` returns none of them, so the new parts fall to
  `.integratedCircuit` (`U`).
- In play mode a click (a `.pan` press that did not move) calls
  `toggleSwitchLever(at:)`, which asks the chip for a switch position under
  the point and calls `document.toggleSwitch` and `simulation.setSwitch`
  (`SchematicCanvasView.swift:903–940`). `Simulation.setSwitch` forwards to
  `circuit_set_switch`. `SchematicNetlist.positionCounts` is a DIP switch's
  position count, 1 for a tactile switch and 0 otherwise; the start loop
  sends only positions within it (`Simulation.swift:393`).
- The drawer's top-level categories are sorted by name
  (`PartCatalog.categories`), so `Relay` needs no order entry; leaves are
  sorted by `localizedStandardCompare` except where `functionGroups`'s
  `value` closure gives a number (capacitors, resistors).

### Measured populations

| What | Count | Command |
|---|---|---|
| `Details` cases today | the enum at `PartCatalog.swift:331` (re-read at M's start; plan-42 and plan-44 add cases) | `sed -n '331,372p' schematical/schematical/Backend/PartCatalog.swift` |
| Exhaustive `PartSymbol` switch sites | 4 files | `grep -rln 'case \.capacitor' schematical/schematical --include='*.swift' \| grep -v Preferences` |
| Parts to list | the `add` rows: inductor 600, fuse + PTC 1200, relay 600, buzzer 553 at planning time | `grep -c '\|<kind>\|' backend/tests/golden/parts.txt` per kind |
| Two-terminal inductors and fuses (symbol candidates) | every inductor and fuse part whose pinout has pins named `1` and `2` (all of them: plan-47-B § 3.3) | `InductorFuseTests` asserts it |

## 3. Design

- **Details.** Four cases from the `Part` structs of I and J:
  `Details.inductor(InductorInfo)` (`pH`, `tolerance`, `ratedMA`, `isatMA`,
  `dcrMicroOhm`, `shielding`), `.fuse(FuseInfo)` (`type`, `ratedMA`,
  `tripMA`, `acMV`, `dcMV`, `speed`, `breakingA`), `.relay(RelayInfo)`
  (`type`, `poles`, `coils`, `latching`, `channels`, `coilMV`, `coilMOhm`,
  `contactMA`, `contactMV`, `operateUS`, `releaseUS`, `onMOhm`),
  `.buzzer(BuzzerInfo)` (`driver`, `technology`, `hz`, `ratedMV`, `minMV`,
  `maxMV`, `uA`, `splCDB`).
- **Value text.** `Canvas/InductorValue.swift`: `text(pH:)` → "4.7 µH",
  "10 nH", "1 mH" (three significant digits, the unit by decade, the
  `CapacitorValue.text` shape); currents "500 mA", "1.6 A" and voltages
  "250 V" through one `PartFacts.amps(mA:)` / `volts(mV:)` pair.
- **Drawer row.** `trailingLabel`: the inductance; a fuse's rated current; a
  PTC's hold current; a relay's coil voltage ("12 V"; a solid-state relay's
  load current); a buzzer's frequency (`PartFacts.hertz`). `detailLabels`
  (the second line): inductor: rated current and shielding; fuse: speed
  ("Fast", "Slow", "Medium") and voltage; PTC: trip current and maximum
  voltage; relay: contact current and "latching" when it is; buzzer: rated
  voltage and technology.
- **Facts rows.** Inductor: Inductance, Tolerance, Rated current, Saturation
  current, DC resistance, Shielding. Fuse: Type ("Fuse"), Rated current,
  Speed, Voltage (AC), Voltage (DC), Breaking capacity. PTC: Type
  ("Resettable fuse (PTC)"), Hold current, Trip current, Maximum voltage,
  Maximum current. Relay: Type ("Electromechanical", "Reed", "Solid-state"),
  Contacts (the part's `function`), Coil voltage, Coil resistance, Contact
  current, Switching voltage, Operate time, Release time, On-resistance,
  Latching ("Yes"). Buzzer: Type ("Indicator (built-in driver)" /
  "Transducer (external drive)"), Technology, Frequency, Rated voltage,
  Voltage range, Current, Sound level ("85 dB"). A row whose fact is not
  given is left out.
- **Kind names.** `kindName(_ kind:)`: "Inductor", "Fuse", "Relay",
  "Buzzer"; `kindName(_ part:)`: "Resettable fuse" for a PTC, "Solid-state
  relay" and "Reed relay" for those types.
- **References.** `ReferenceCategory.of(_:)`: `PART_INDUCTOR` → `.inductor`,
  `PART_FUSE` → `.fuse`, `PART_RELAY` → `.relay`, `PART_BUZZER` → `.speaker`
  (each only if absent: `grep -c 'PART_INDUCTOR'
  schematical/schematical/Preferences/ReferencePrefixes.swift` → 0 before).
- **Drawer order.** `functionGroups`'s `value` closure gets the new parts:
  leaves go by the smallest value of the leaf's parts and parts in a leaf by
  value then name: inductors by `pH`, fuses and PTCs by `ratedMA`, buzzers by
  `hz`; relay leaves keep the name order and their parts go by `coilMV` then
  name.
- **Symbols.** `PartSymbol.inductor` and `.fuse`: `symbol(for:)` returns them
  for a `PART_INDUCTOR` / `PART_FUSE` part with pins named `1` and `2`, else
  `.block`. `ChipLayout.inductor(pins)` / `.fuse(pins)`: pin `1` left, pin
  `2` right on the resistor box's centre line; `NC` pins and extra pins of
  the same name are not drawn (the resistor and crystal rule). Views:
  `Canvas/InductorSymbolView.swift` (four half-circle humps between the
  leads, the value above: `InductorValue.text`) and
  `Canvas/FuseSymbolView.swift` (a rectangle of the resistor's size with the
  lead drawn through it; a PTC adds the diagonal "variable" stroke with a
  flat foot; the rated or hold current above), each with the static
  `paint(_:chip:isSelected:isHovered:)` for the export scene, highlight and
  flip as `ResistorSymbolView`. Wired into the three other switch sites.
- **Blowing a fuse.** `Simulation` keeps `blownFuses: Set<UUID>` (empty at
  every start, cleared at stop). `positionCounts` is 1 for a `PART_FUSE`
  chip, so `setSwitch` reaches it. In play mode a click on a fuse-kind chip
  (any of its body, tested before the switch-lever test in the `.pan` click
  branch) toggles its id in `blownFuses` and calls
  `simulation.setSwitch(id, position: 1, on: !blown)`. `FuseSymbolView`
  draws a blown fuse with a gap in the middle of its lead and the body in
  the canvas's error colour; a fuse drawn as a block (never the case for the
  catalog's parts) shows "blown" under its name. The app's voltage check and
  `switchesOn` are unchanged: the backend treats a fuse as closed for the
  check (plan-47-I § 3.3). Automation: the `setSwitch` command
  (`Automation/AutomationSimulation.swift:95`) accepts a fuse (`on` =
  intact), so an agent can blow one.
- **Help.** `Help/KeyboardHelp.swift` gains, after "Click a switch lever"
  (`:82`), "Click a fuse (in play mode)": "Blow or restore it".

Where the risk concentrates: the play-mode click (it must not steal clicks
from switch levers or start an edit) and the drawer sort on the
performance-tested path (`DrawerPerformanceTests`); the two new symbol
geometries in four switch sites.

Rejected alternatives: (1) saving the blown state in the file (rejected: a
file-format field for a state that a real board fixes by replacing the
part; a tactile switch's press is not saved either); (2) blowing a fuse
from the sidebar only (rejected: DIP switches and push buttons set the
precedent that play-mode state changes with a click on the part); (3) a per-instance value on a generic
part (no generics, owner decision).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, kind names, references (all four kinds)

- [ ] `Backend/PartCatalog.swift`: the four info structs and `Details`
      cases, the builder branches, `detailLabels`, `trailingLabel`.
- [ ] `Canvas/InductorValue.swift`; `Backend/PartFacts.swift`: `amps`,
      `volts`, the facts rows, both `kindName`s.
- [ ] `Preferences/ReferencePrefixes.swift`: `of(_:)` (each case if absent).
- [ ] Tests: `schematicalTests/InductorFuseTests.swift` (from
      `ResistorTests`): one real part per kind and type from the catalog
      (the most-stocked `add` row of each, named in the test): kind name,
      facts rows, trailing label, `ReferenceCategory.of` and the lowest free
      `L<n>`, `F<n>`, `K<n>`, `LS<n>`.

Acceptance: the facts, names and references are right on the real catalog.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/InductorFuseTests` → `** TEST SUCCEEDED **` (est. 8 min: build + one test class).
Commit: —

### Phase 2 — The inductor and fuse symbols

- [ ] `Canvas/SchematicModel.swift`: `PartSymbol.inductor`, `.fuse` (each
      only if absent) and the two `ChipLayout`s; `symbol(for:)`.
- [ ] `Canvas/InductorSymbolView.swift`, `Canvas/FuseSymbolView.swift`; the
      three other switch sites.
- [ ] Tests (`InductorFuseTests`): `symbol(for:)` for a chip inductor, a
      four-pad inductor, a fuse, a PTC and a cartridge fuse; the layouts put
      `1` left and `2` right and draw no `NC` pin; every inductor and fuse
      part of the catalog gets its symbol (none falls to `.block`); a placed
      part renders (the export scene's image is not empty).

Acceptance: inductors and fuses draw as such.
  Check: `xcodebuild … test -only-testing:schematicalTests/InductorFuseTests` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 3 — Drawer order, blowing a fuse, the harness

- [ ] `Backend/PartCatalog.swift` `functionGroups`: the value order (§ 3).
- [ ] `Simulation/Simulation.swift`: `blownFuses`, `positionCounts` for a
      fuse, the toggle; `Canvas/SchematicCanvasView.swift`: the play-mode
      click; `FuseSymbolView`: the blown drawing;
      `Automation/AutomationSimulation.swift`: `setSwitch` on a fuse;
      `Help/KeyboardHelp.swift`.
- [ ] `scripts/harness/inductor-fuse-app.swift` (from
      `transistor-app.swift`): the checks of § 1 on the app's real catalog,
      document and `Simulation`; the circuit `5V`/`GND` → `SN74HC04N`
      (input on GND) → 1Y → an inductor → a fuse → `GENERIC-RESISTOR` →
      `GENERIC-LED` → GND, with the most-stocked `add` inductor and fuse:
      three 0.3 s runs (lit; fuse blown: dark; restored: lit). Renders a
      placed inductor, fuse, blown fuse and PTC to `/tmp`.
- [ ] Tests: `DrawerPerformanceTests` and `PartEditorWindowTests` stay
      green; `InductorFuseTests` gains the drawer order (the inductor leaves
      in band order, a leaf's parts in value order) and the blown-fuse state
      (set, cleared at stop); `KeyboardHelpTests` updated for the new line.

Acceptance: the drawer, the sidebar and the play-mode outcome work end to end.
  Check: `rm -rf /tmp/p47-harness && APP=/tmp/p47-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/inductor-fuse-app.swift /tmp/p47-harness && /tmp/p47-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/InductorFuseTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 15 min: app build, harness, four test classes).
Commit: —

## Validation Plan

- Tests: `InductorFuseTests`; `DrawerPerformanceTests`,
  `PartEditorWindowTests`, `KeyboardHelpTests`.
- Coverage check: the harness runs the app's own catalog and `Simulation`,
  so the new symbols, facts, sort and the blow path run against the real
  parts.
- Runtime proof: the harness's LED circuit, lit / dark / lit.
- Doc sync: none (`docs/file-format.md` unchanged; `docs/parts-format.md` is
  I's to L's); the in-app help line.
- Final gate: plan-47-O.

## Decisions

Owner decisions (asked once, 2026-11-16):

- **A fuse can be blown by clicking it in play mode, and restored by a
  second click.**
- **All four types show their ratings in the sidebar and sort in the
  drawer.**

Author decisions:

- **A blown fuse is play-mode state only** (§ 3 rejected alternative 1);
  every play starts intact.
- **Reference prefixes are the Settings' existing ones**: `L`, `F`, `K`, and
  `LS` for a buzzer (the "Speaker / Buzzer" category).
- **The symbols**: the humps for an inductor, the IEC rectangle with a
  through line for a fuse, the same with the thermistor stroke for a PTC.
- **Relays and buzzers are blocks for one letter** (N draws them), so M
  lands a working, complete drawer and sidebar on its own.

## Corrections

## Summary

M's risk is the play-mode click and the drawer sort on a performance-tested
path; the harness and the performance test bound both. Untouched: the
engine, the part files, the file format and the Bill of Materials.
