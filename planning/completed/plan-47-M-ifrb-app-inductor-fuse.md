# plan-47-M: The app — the four kinds in the drawer and sidebar; inductor and fuse symbols; blowing a fuse

Last updated: 2026-11-22
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
fuses**, **Switches & Relays → Relay** and **Displays & Indicators → Buzzers** with value-ordered leaves; a
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
| plan-47-L complete | `ls planning/completed/plan-47-L-*.md` → exists | NOT MET as written (2026-11-22: `ls planning/completed/plan-47-L-*.md` → no such file; L's work is in this worktree (the part files, goldens and kind lists exist, the coordinator commits it), so M is worked on top of it; § Corrections) |
| The parts are loaded | `grep -c '|inductor|\||fuse|\||relay|\||buzzer|' backend/tests/golden/parts.txt` → the `add` rows (2953 at planning time) | MET (2026-11-22: `grep -c '\|inductor\|\|\|fuse\|\|\|relay\|\|\|buzzer\|' backend/tests/golden/parts.txt` → 2816 = 600 + 1199 + 504 + 513, the `"manufacturer"` counts of the four part files; the number changes at the final regeneration, no test here hard-codes it) |
| The editor's kind lists name the four kinds | `grep -c '"inductor"\|"fuse"\|"relay"\|"buzzer"' schematical/schematical/Editor/KindInfo.swift` → at least 4 | MET (2026-11-22: 7) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-22 in the P-47 worktree, `-derivedDataPath /tmp/p47-dd-m`: `** BUILD SUCCEEDED **`, before and after the changes) |

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
  (`PartCatalog.categories`), so `Switches & Relays` (the drawer regrouping already created it) needs no order entry; leaves are
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

- [x] (2026-11-22: `InductorInfo`, `FuseInfo`, `RelayInfo`, `BuzzerInfo`, the four `Details` cases, the four builder branches, `detailLabels`, `trailingLabel`; also the accessors `inductor`/`fuse`/`relay`/`buzzer` and `drawerValue`) `Backend/PartCatalog.swift`: the four info structs and `Details`
      cases, the builder branches, `detailLabels`, `trailingLabel`.
- [x] (2026-11-22: `Canvas/InductorValue.swift`; `PartFacts.amps(mA:)`, `volts(mV:)`, `microohmsText`, `capitalized`, `relayType`, the four facts rows, both `kindName`s) `Canvas/InductorValue.swift`; `Backend/PartFacts.swift`: `amps`,
      `volts`, the facts rows, both `kindName`s.
- [x] (2026-11-22: `grep -c PART_INDUCTOR` was 0 before, now 1; the four cases added) `Preferences/ReferencePrefixes.swift`: `of(_:)` (each case if absent).
- [x] (2026-11-22: `InductorFuseTests` `testInductorValueText`, `testInductorFacts`, `testFuseAndPTCFacts`, `testRelayFacts`, `testBuzzerFacts`, `testReferences`, `testEveryPartReadsAsItsKind`; the parts are named: LQG15HN18NJ02D, SS-5F-10A-AP, 0ZCM0010FF2G, RZ03-1A3-D005-R, JWD-172-1, AQV210EHAX, SMT-0825-S-4-R, the most-stocked `add` row of each kind and type by `candidates.tsv`) Tests: `schematicalTests/InductorFuseTests.swift` (from
      `ResistorTests`): one real part per kind and type from the catalog
      (the most-stocked `add` row of each, named in the test): kind name,
      facts rows, trailing label, `ReferenceCategory.of` and the lowest free
      `L<n>`, `F<n>`, `K<n>`, `LS<n>`.

Acceptance: the facts, names and references are right on the real catalog.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/InductorFuseTests` → `** TEST SUCCEEDED **` (est. 8 min: build + one test class).
  Observed (2026-11-22, `-derivedDataPath /tmp/p47-dd-m`): `Test Case '-[schematicalTests.InductorFuseTests testFuseAndPTCFacts]' passed` … `** TEST SUCCEEDED **`.
Commit: 15270503

### Phase 2 — The inductor and fuse symbols

- [x] (2026-11-22: `case inductor`, `case fuse` after `.resistor`; one shared `ChipLayout.twoTerminals(_:)` for both; `symbol(for:)` returns them for pins named `1` and `2`) `Canvas/SchematicModel.swift`: `PartSymbol.inductor`, `.fuse` (each
      only if absent) and the two `ChipLayout`s; `symbol(for:)`.
- [x] (2026-11-22: both views; the other sites: `SchematicCanvasView.swift`, `SchematicExportScene.swift`, `ReferenceLabel.swift`; also the two writers' `bodyBox` lists, § Corrections) `Canvas/InductorSymbolView.swift`, `Canvas/FuseSymbolView.swift`; the
      three other switch sites.
- [x] (2026-11-22: `testSymbolFor`, `testEveryInductorAndFuseGetsItsSymbol`, `testOtherPinNamesStayABlock`, `testRendering`, `testSymbolLabels`; the "four-pad inductor" is the catalog's first inductor with four pins, whose two extra pins are `NC`, § Corrections) Tests (`InductorFuseTests`): `symbol(for:)` for a chip inductor, a
      four-pad inductor, a fuse, a PTC and a cartridge fuse; the layouts put
      `1` left and `2` right and draw no `NC` pin; every inductor and fuse
      part of the catalog gets its symbol (none falls to `.block`); a placed
      part renders (the export scene's image is not empty).

Acceptance: inductors and fuses draw as such.
  Check: `xcodebuild … test -only-testing:schematicalTests/InductorFuseTests` → `** TEST SUCCEEDED **` (est. 8 min).
  Observed (2026-11-22): `Executed 18 tests, with 0 failures` for `InductorFuseTests`, `** TEST SUCCEEDED **`.
Commit: 15270503

### Phase 3 — Drawer order, blowing a fuse, the harness

- [x] (2026-11-22: `CatalogPart.drawerValue` (crystal hz, inductor pH, fuse ratedMA, buzzer hz, relay coilMV) replaces the crystal-only test in `functionGroups`; relay leaves keep the name order) `Backend/PartCatalog.swift` `functionGroups`: the value order (§ 3).
- [x] (2026-11-22: `SimulationController.blownFuses`, `toggleFuse`, `setFuse`, `SchematicNetlist.fuseParts`, `positionCounts` 1 for a fuse, the click in `.pan`, `FuseSymbolView`'s blown drawing, the automation `set_switch` on a fuse (needs a running simulation), the help line) `Simulation/Simulation.swift`: `blownFuses`, `positionCounts` for a
      fuse, the toggle; `Canvas/SchematicCanvasView.swift`: the play-mode
      click; `FuseSymbolView`: the blown drawing;
      `Automation/AutomationSimulation.swift`: `setSwitch` on a fuse;
      `Help/KeyboardHelp.swift`.
- [x] (2026-11-22: written; run result under Acceptance; `scripts/harness/build.sh` needed `-module-name schematical`, § Corrections) `scripts/harness/inductor-fuse-app.swift` (from
      `transistor-app.swift`): the checks of § 1 on the app's real catalog,
      document and `Simulation`; the circuit `5V`/`GND` → `SN74HC04N`
      (input on GND) → 1Y → an inductor → a fuse → `GENERIC-RESISTOR` →
      `GENERIC-LED` → GND, with the most-stocked `add` inductor and fuse:
      three 0.3 s runs (lit; fuse blown: dark; restored: lit). Renders a
      placed inductor, fuse, blown fuse and PTC to `/tmp`.
- [x] (2026-11-22: `testPaths`, `testValueOrder`, `testRelayAndBuzzerOrder`, `testPositionCounts`, `testBlowingAFuse`, `testAFuseIsHitAnywhereOnItsBody`; `DrawerPerformanceTests.testPartsAreInNaturalOrder` now sorts by `drawerValue`; `KeyboardHelpTests.testSchematicFuseLine`) Tests: `DrawerPerformanceTests` and `PartEditorWindowTests` stay
      green; `InductorFuseTests` gains the drawer order (the inductor leaves
      in band order, a leaf's parts in value order) and the blown-fuse state
      (set, cleared at stop); `KeyboardHelpTests` updated for the new line.

Acceptance: the drawer, the sidebar and the play-mode outcome work end to end.
  Check: `rm -rf /tmp/p47-harness && APP=/tmp/p47-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/inductor-fuse-app.swift /tmp/p47-harness && /tmp/p47-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/InductorFuseTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 15 min: app build, harness, four test classes).
  Observed (2026-11-22): the harness (built with `APP=/tmp/p47-dd-m/Build/Products/Debug/schematical.app`) printed 18 `ok` lines, `LED: ["lit", "dark", "lit"]; blown after the clicks: [true, false]`, `all checks ok`, exit 0; `xcodebuild … test` with the four classes (and CrystalTests, ResistorTests, CapacitorTests, TactileSwitchTests, PlaySwitchResetTests, AutomationTests) → `Executed 103 tests, with 0 failures`, `** TEST SUCCEEDED **`. A second run of KiCadExportTests, EasyEDAProExportTests, NetListTests, ReferencePrefixTests, ReferencePositionTests, DatasheetCatalogTests, BoardCatalogTests, DrawerConverterFilterTests, UserLibraryCatalogTests, SimulationAvailabilityTests, PartKindRoundTripTests, PartKindKeysTests: `Executed 117 tests, with 1 failure`: `UserLibraryCatalogTests.testAddListsUnderUserLibraryAndWritesTheFile` compares `/var/folders/…` with `/private/var/folders/…` (a temp-directory path, nothing M touched).
Commit: 15270503

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

- **2026-11-22, § Prerequisites.** plan-47-L is not in `planning/completed/`
  (the coordinator commits it with the rest); its part files, goldens and the
  editor's kind lists are in the worktree, so M ran on top of them.
- **2026-11-22, line numbers moved.** `Details` is at
  `PartCatalog.swift:543` (not 331), `trailingLabel` `:942`, `detailLabels`
  `:964`, `symbol(for:)` `:1606` (not 1126), `functionGroups` `:2501` (not
  1678); `PartFacts.kindName(_ kind:)` `:299`, `kindName(_ part:)` `:348`,
  `hertz` `:638`; the play-mode click is the `.pan` case at
  `SchematicCanvasView.swift:787` and `toggleSwitchLever` at `:833` (not 903,
  936); `positionCounts` is declared at `Simulation.swift:33` and built at
  `:218–235`, `setSwitch` is at `:555` (not 505); the help line "Click a
  switch lever" is `KeyboardHelp.swift:83`.
- **2026-11-22, drawer paths.** The part files now draw relays at `["Switches &
  Relays", "Relay", "<Signal|Power|Reed|Solid state|Automotive>"]` and
  buzzers at `["Displays & Indicators", "Buzzers"]` (the drawer regroup done
  in the main checkout); the plan's `Relay` and `Indicators → Buzzers` are
  those. The app's drawer code names no top-level category (they sort by
  name), so no drawer ordering code needed an edit for the paths; the tests
  assert the paths the part files give.
- **2026-11-22, § 3 "Drawer order".** One `CatalogPart.drawerValue`
  (crystal hz, inductor pH, fuse/PTC rated mA, buzzer hz, relay coil mV) now
  drives both the leaf order and the order of a leaf's parts, instead of one
  closure per kind; the crystal order is unchanged (`CrystalTests` green), and
  `DrawerPerformanceTests.testPartsAreInNaturalOrder` follows it. Relay
  leaves go by name (the closure answers nil for a relay leaf), their parts by
  coil voltage then name.
- **2026-11-22, § 3 "Blowing a fuse".** `positionCounts` is 1 for a fuse, as
  planned; the app also needs to know which part is a fuse, so
  `SchematicNetlist` gained `fuseParts` and `isFuse(part:)`, and
  `SimulationController` gained `toggleFuse(_:)` (nil unless the simulation is
  running and the chip is a fuse) and `setFuse(_:blown:)`. The automation
  `set_switch` on a fuse needs a running simulation (a blown fuse is play-mode
  state) and says so; `on` = intact.
- **2026-11-22, § 3 "Symbols".** The inductor and the fuse share one layout,
  `ChipLayout.twoTerminals`. Neither rotates (`PartSymbol.canRotate` is
  unchanged: the plan asks for flip only). The catalog has no inductor with
  two pads per terminal: its multi-pin inductors have `NC` pads (5 of the
  `1`, `NC`, `NC`, `2` pinout, 2 of `1`, `NC`, `2`, `NC` and 2 of `1`, `2`,
  `NC`), which the layout does not draw; the tests' "four-pad inductor" is the
  first inductor with four pins. No fuse or PTC has more than two pins.
  The reference label sits where a resistor's does (above the value, y = -1),
  as the symbols' value text does.
- **2026-11-22, the exporters.** `KiCadSchematicWriter.bodyBox` and
  `EasyEDAProSchematicWriter.bodyBox` list the two-terminal symbols; `.inductor`
  and `.fuse` were added to both (the plan names no exporter) so a placed
  inductor or fuse exports with a body the size of its pins' span, as a
  resistor does. The crystal is not in those lists; it was left as it is.
- **2026-11-22, harness build.** `scripts/harness/build.sh` failed at HEAD
  (`SchematicModel.swift:2221`: `schematical.symbol(for:)` names the module,
  which `swiftc` called without `-module-name` does not have); `-module-name
  schematical` was added to its `swiftc` line. No harness script is affected.
- **2026-11-22, parts named in the tests.** The tests and the harness name the
  seven most-stocked parts by `planning/plan-47-data/candidates.tsv`
  (§ Phase 1); after the final regeneration of the data they must still exist
  (re-check with `TestSupport.part`), and the leaf names asserted are only
  those the paths and the order give (no count and no band name is asserted).

## Summary

M's risk is the play-mode click and the drawer sort on a performance-tested
path; the harness and the performance test bound both. Untouched: the
engine, the part files, the file format and the Bill of Materials.
