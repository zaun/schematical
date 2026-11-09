# plan-27-J: Tactile switches in the app — drawer, symbol, press, sidebar

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-I

The library now has tactile switches (plan-27-E…I). J puts them in the app:
they show in the parts drawer under **Switch → Tactile → Normal Open / Normal
Close**, draw as a push-button symbol, and in play mode a mouse-down on the
button closes (Normal Open) or opens (Normal Close) the contact until the
mouse is released. The sidebar names them "Tactile switch" and shows their
facts.

Outcome: place a Normal Open tactile switch between a 5 V supply and an LED
circuit, start the simulation, hold the mouse on the button: the LED lights;
release: it goes dark. A Normal Close switch does the opposite. Saving and
reopening a schematic keeps the switch and starts it released.

References:

- `planning/plan-27-D-tactile-kind.md` § 3–4 (contact numbering: positions
  first, joined pairs after; `circuit_set_switch` ON = closed).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details`, `count`,
  `detailLabels`, `symbol(for:)`, `groups(matching:)`),
  `Backend/PartFacts.swift`, `Canvas/SchematicModel.swift` (`PartSymbol`,
  `ChipLayout`, `PlacedChip`, `toggleSwitch`), `Canvas/SchematicCanvasView.swift`
  (`Interaction`, `handleChanged`, `handleEnded`, `toggleSwitchLever`),
  `Canvas/DipSwitchSymbolView.swift` (the model for the new view),
  `Simulation/Simulation.swift` (`NetlistBuilder`, `setSwitch`),
  `Simulation/VoltageCheck.swift`, `Document/SchematicFile.swift`.
- `docs/file-format.md` (`switches`), `AGENTS.md` (§ Testing in the app).

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-I is complete | `ls planning/completed/plan-27-I-*` → one file | moot for J (Correction 2: J needs E's parts, not I); E archived 2026-11-08 (1f61382) |
| Tactile parts are in the library | `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` → ≥ 1 | MET 2026-11-08: 488 |
| No tactile code in the app yet | `grep -rc 'tactile' schematical/schematical --include='*.swift' \| grep -v ':0' \| wc -l` → 0 | MET at the start (2026-11-08, before 96d9115) |
| plan-28-A complete (symbols draw through `SchematicPainter`) | `ls planning/completed/plan-28-A-* \| wc -l` → 1 and `grep -c 'protocol SchematicPainter' schematical/schematical/Canvas/*.swift` → ≥ 1 in some file | MET 2026-11-08: `planning/completed/plan-28-A-export-painter.md`; `protocol SchematicPainter` in `Canvas/SchematicPainter.swift:74` |
| The other open plans add no `PartSymbol` case | `grep -c 'PartSymbol' planning/plan-22-E-arith-app.md planning/plan-24-I-converter-app.md` → 0 and 0 | MET (2026-11-07) |

If plan-27-I is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The drawer lists tactile switches under Switch → Tactile with the leaves
  `Normal Open` and `Normal Close`, each part row showing its package
  (`SMD`, `THT`, …), actuator and `N V max`, without the DIP "(n)" count.
- A tactile chip draws as a push-button with its terminals (two or four) and a
  contact that shows open or closed.
- In play mode, mouse-down on its button presses it, mouse-up releases it; the
  simulation and the voltage check see the contact exactly as the backend does
  (plan-27-D).
- The sidebar's Kind row says "Tactile switch"; Positions is replaced by Contact
  (`Normal open, momentary`), Terminals, Actuator, Rating.
- The file never stores the pressed state; reopening starts released.

### Non-goals (explicit constraints)

- DIP switch drawing, click-to-toggle, saved `switches`, drawer count menu:
  unchanged.
- No per-instance option (no latching mode, no Options section).
- No change to the backend, part files, goldens.
- The pressed state is not undoable and not part of the document's dirty state.

## 2. Current State

- `CatalogPart.Details.dipSwitch(positions:actuator:ratedMV:)` is built from
  `part.dip_switch` (`PartCatalog.swift:494`); `symbol(for:)` returns
  `.dipSwitch` for every `PART_SWITCH` (`PartCatalog.swift:675`);
  `PartFacts.kindName` returns "DIP switch" for `PART_SWITCH`
  (`PartFacts.swift:120`, one caller, `PartFacts.swift:15`).
- `PlacedChip.switchesOn` is `[Bool]`, one entry per position
  (`SchematicModel.swift:608`), saved as `switches`
  (`SchematicFile.swift:371`, read at `:536`); `toggleSwitch` flips it after a
  voltage-check guard (`SchematicModel.swift:2067`).
- In play mode every press starts a pan; a click ends in
  `toggleSwitchLever` (`SchematicCanvasView.swift:695`, `:721`).
- `NetlistBuilder.build` lists closed positions as `switchesOn`
  (`Simulation.swift:122`); `Simulation` start calls `circuit_set_switch(…, 1)`
  for each (`:301`); `VoltageCheck.sortedIssues` reads the same list as the
  closed contacts (`VoltageCheck.swift:76`).
- The drawer is a `List` of `DisclosureGroup`s, closed by default
  (`ChipDrawerView.swift:62–70`); its Package picker takes
  `catalog.packages`, so the new package names appear without code.

### Measured populations

| What | Count | Command |
|---|---|---|
| Exhaustive switches over `PartSymbol` | 2 | `grep -rn --include='*.swift' 'case .barrelJack' schematical/schematical \| wc -l` → 2 (SchematicModel.swift:204 layout, SchematicCanvasView.swift:243 drawing) |
| Sites reading the `dipSwitch` details | 7 | `grep -rn --include='*.swift' 'case let .dipSwitch\|.dipSwitch(positions' schematical/schematical \| wc -l` → 7 (PartFacts :41; PartCatalog :270 :363 :387 :495 :834; SchematicModel :608) |
| Tactile parts in the library | N_total (after I) | `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` |
| Lowest `ratedVolts` among the added tactile parts | 12 (DigiKey voltages: 12, 15, 16, 18, 24, 28, 30, 32, 35, 50, 100) | `awk -F'\t' '$18=="add"' planning/plan-27-data/candidates.tsv \| grep -o '@ [0-9.]*V' \| sort -u` |
| Highest library supply | 12 V | `grep -o '"volts": *[0-9.]*' schematical/schematical/Parts/supply.json \| sort -u` |

### Verified properties

- No tactile part is rated below the highest supply the library has, so a press
  can never be refused by the voltage check in the shipped library; the press
  path still runs the same guard as `toggleSwitch` (§ 4.4).
- Contact numbering the app must match is the backend's: position 1 is the
  button's contact; joined pairs are positions 2 and 3 on a four-terminal part
  (plan-27-D § 3). Verified by reading `switch.c` after D lands (Phase 2 test 2).

## 3. Design Overview

Catalog first (everything else reads `Details`), then the drawing, then the
interaction. The app's `switchesOn[0]` for a tactile chip means **pressed**.
What the backend and the voltage check need is **contact closed** =
`pressed != normallyClosed`, plus the joined contacts, which are always closed.
`NetlistBuilder` is the one place that converts, so the simulation and the
voltage check cannot disagree. Risk concentrates in that conversion and in
the gesture (a press must always be matched by a release). Byte-identity is
not a gate; DIP behavior is protected by leaving its code paths alone and by the
existing tests.

## 4. Detailed Design

### 4.1 Catalog (`PartCatalog.swift`, `PartFacts.swift`)

- `Details.tactileSwitch(actuator: String, ratedMV: UInt32, normallyClosed:
  Bool, joined: Int)`, built when `part.dip_switch.momentary != 0`; the
  `dipSwitch` case stays for the others.
- `count` → nil for it (no "(n)" label; the position count menu ignores it);
  `detailLabels` → `[actuator]`; `voltageLabel` → `"<V> V max"` (also in the
  drawer's voltage filter at `:834`).
- `symbol(for:)`: `.tactileSwitch` for it, `.dipSwitch` for other switches.
- `PartFacts`: `kindName(_ part: CatalogPart)` replaces `kindName(_ kind:)`
  (one caller) and returns "Tactile switch" for a tactile part; rows: Contact
  (`Normal open, momentary` / `Normal close, momentary`), Terminals (pin count),
  Actuator, Rating.

### 4.2 Model and drawing

- `PartSymbol.tactileSwitch`; `ChipLayout.tactileSwitch(pins)`: pins whose
  `name` starts with `A` on the left, `B` on the right, in pin order, one row per
  pair (a 4-terminal part has two rows, a 2-terminal part one); body width
  `switchBodyWidth`, height `headerHeight + pitch × rows`.
- `PlacedChip`: for a tactile part `switchesOn` is `[false]` whatever the saved
  file says; `buttonRect` is the middle third of the body below the header;
  `func buttonHit(at:)` is true inside it (with the DIP lever's 4 × 3 pt slop).
- `Canvas/TactileSwitchSymbolView.swift` (new, modelled on
  `DipSwitchSymbolView`): box with the part number, pin stubs and designators,
  for a 4-terminal part a bar joining the two pins of each side; in the middle
  two contacts and a plunger: closed (`pressed != normallyClosed`) draws the
  bar touching both contacts in green, open draws a gap; pressed draws the cap
  pushed down.
  It draws through `SchematicPainter` (colors as `Ink` roles, text as
  `PaintedText`), the same shape as the plan-28-A migrated
  `DipSwitchSymbolView`, never into a raw `GraphicsContext`, so exports
  include it.
- `SchematicCanvasView` draws it for `.tactileSwitch`, and the export scene's
  `switch chip.symbol` (`SchematicExportScene.swift`, plan-28-B) gains the
  `.tactileSwitch` case.

### 4.3 Netlist and simulation (`Simulation.swift`, `VoltageCheck.swift`)

For a tactile chip, `NetlistBuilder` adds position 1 to `switchesOn` when
`pressed != normallyClosed`, and for a four-terminal part positions 2 and 3
(`1 + joined` pairs) always. `Simulation`'s start loop calls
`circuit_set_switch` only for a position within the part's `positions` (the
netlist carries `positionCounts: [Int]`, 1 for a tactile part, `switchesOn.count`
of the chip for a DIP); joined positions are backend-owned and are never sent.
For a normally-closed part pressed at start (never: pressed is always false at
start) nothing else is needed.

### 4.4 Press and release

- `SchematicDocument.setButton(_ id: UUID, pressed: Bool) -> Bool?`: refuses a
  non-tactile chip (nil); pressing a Normal Open part runs the same candidate
  check as `toggleSwitch` (`VoltageCheck.firstNew(… edit: .turnOn)`), sets
  `editorMessage` and returns nil on refusal; otherwise sets `switchesOn[0]` and
  returns the new pressed state. Releasing never checks. It does not touch the
  undo stack or the dirty flag.
- `SchematicCanvasView`: `Interaction.holdButton(chip: UUID)`. In
  `handleChanged`, play mode, `interaction == nil`, and the press began on a
  tactile chip's `buttonHit` → `setButton(pressed: true)`, then
  `simulation.setSwitch(chip, position: 1, on: pressed != normallyClosed)`,
  interaction `.holdButton`; no pan starts. `handleEnded` for `.holdButton` →
  `setButton(pressed: false)` and `setSwitch(…, on: normallyClosed)`. A
  `.onChange(of: isRunning)` that sees play mode end releases every pressed
  button (`document.releaseAllButtons()`).
- Edit mode: a press on a tactile chip behaves as on any chip (select, drag);
  the button does nothing.

### 4.5 File

`SchematicFile` writes `switches` only for non-momentary switches and ignores it
when reading a tactile part (`PlacedChip.init` forces `[false]`).
`docs/file-format.md` § `switches`: "DIP switches only; tactile switches never
save it (they start released)".

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, drawer, facts

- [x] `PartCatalog.swift`, `PartFacts.swift`: § 4.1, plus the drawer's
      function level for tactile switches (Correction 3).
- [x] Tests: `schematicalTests/TactileSwitchTests.swift` (new): from
      `TestSupport.catalog`, every part whose details are `.tactileSwitch`
      has symbol `.tactileSwitch`, `count == nil`, `kindName` "Tactile switch";
      `groups(matching: DrawerFilter())` has a "Switch" category whose
      "Tactile" subcategory has exactly the functions `Normal Close`,
      `Normal Open`; every NC part's function is `Normal Close`.

Acceptance: the drawer data is right.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/TactileSwitchTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 4 min).
  Result 2026-11-08 (with E's 488 C&K parts): `Executed 10 tests, with 0 failures` — testCatalogFacts, testDrawerLeaves, testSidebarRows, testNormalOpenTwoTerminal, testNormalOpenFourTerminal, testNormalClose, testFileNeverSavesPressed, testDipSwitchUnchanged, testButtonHit, testPressAndRelease; `** TEST SUCCEEDED **`.
Commit: 96d9115 (code and tests)

### Phase 2 — Model, symbol, file, netlist

- [x] `SchematicModel.swift`, `Canvas/TactileSwitchSymbolView.swift`,
      `SchematicCanvasView.swift` (draw case): § 4.2.
- [x] `Simulation.swift`: § 4.3, plus private nets for a tactile switch's unwired pins (Correction 4). `SchematicFile.swift`, `docs/file-format.md`: § 4.5.
- [x] Tests (`TactileSwitchTests`): (1) place the first NO 2-terminal part:
      `switchesOn == [false]`; `NetlistBuilder.build` → `switchesOn` empty;
      `setButton(pressed: true)` → `[true]` and the netlist has position 1;
      (2) first NO 4-terminal part: netlist always has positions 2 and 3, and
      position 1 only while pressed; (3) first NC part: netlist has position 1
      at rest and not while pressed; (4) save then reopen with the part pressed:
      reopened `switchesOn == [false]` and the file has no `switches` key for it;
      (5) a DIP switch's `switchesOn` and netlist are unchanged.

Acceptance: the model and netlist behave as § 4.3 says.
  Check: the Phase 1 command → all pass (est. 4 min). Result: as Phase 1 (the five cases are testNormalOpenTwoTerminal, testNormalOpenFourTerminal, testNormalClose, testFileNeverSavesPressed, testDipSwitchUnchanged).
Commit: 96d9115

### Phase 3 — Press interaction

- [x] `SchematicModel.swift` `setButton`, `releaseAllButtons`;
      `SchematicCanvasView.swift` `.holdButton`, `.onChange(of: isRunning)`: § 4.4.
- [x] Tests (`TactileSwitchTests`): `setButton` on a DIP chip → nil; on a
      tactile chip pressed true then false → `[true]` then `[false]`;
      `releaseAllButtons` after two presses → both `[false]`.

Acceptance: a press is always matched by a release; DIP lever click unchanged.
  Check: the Phase 1 command plus `-only-testing:schematicalTests/SelectionGestureTests` → all pass (est. 5 min; the gesture file shares `Interaction`).
  Result 2026-11-08: `Executed 19 tests, with 0 failures`; `** TEST SUCCEEDED **`.
Commit: 96d9115

## Validation Plan

- Tests: `TactileSwitchTests` (Phases 1–3).
- Coverage check: `grep -c 'tactile' schematical/schematicalTests/TactileSwitchTests.swift` ≥ 10.
- Runtime proof: plan-27-K (the harness script and the `run` skill).
- Doc sync: `docs/file-format.md` § `switches`; `docs/parts-format.md` needs nothing here.
- Final gate (once, after Phase 3): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → 0 failures (est. 8 min).
  Result 2026-11-08: `Executed 191 tests, with 0 failures`; `** TEST SUCCEEDED **`. Coverage: `grep -c 'tactile' …TactileSwitchTests.swift` → 16.

## Decisions

- **Pressed state is `switchesOn[0]`, transient, not saved** — author, after the
  owner's "momentary hold" (plan-27-A § Decisions). Rejected: a separate
  `pressed` property (a second state array for one bool).
- **Mouse-down presses, mouse-up releases, play mode only** — owner, 2026-11-07.
- **No latching option** — owner chose "Momentary hold"; the "Both, per-instance" option was declined.
- **Symbol and label choices** (`Normal open, momentary`; terminals A left, B right) — author.

## Corrections

1. 2026-11-07 — coordination with plan-28 (Export SVG/PNG/JPG), owner's
   request: plan-28-A moves every symbol drawer onto `SchematicPainter`; J now
   requires it (Prerequisites row "plan-28-A complete") and § 4.2 draws the
   new view through the painter and adds its export-scene case.
2. 2026-11-08 — **J does not need I, only E** (the first tactile parts in
   the library: the tests pick the first NO 2-terminal, NO 4-terminal and NC
   part). Its code was written while A's census waited on the network; its
   tests run once E lands. plan-28 (A–D) is archived on main (merged into
   this branch at 55140df), so the plan-28-A row is MET.
3. 2026-11-08 — **missing task: the drawer's function level.**
   `PartCatalogStore.listsPartsDirectly` lists every `PART_SWITCH`
   subcategory with no function level (`PartCatalog.swift`, "Switches … are
   listed with no function level"), which would hide the Normal Open /
   Normal Close leaves. Tactile switches now keep their function level; the
   DIP position-count menu and filter skip them.
4. 2026-11-08 — **missing task: unwired joined pins in the app.** The app's
   voltage check builds its own connections (`VoltageCheck.sortedIssues` from
   `NetlistBuilder`), so the backend's `wire_joined_pins` (plan-27-D
   Correction 4) does not reach it. `NetlistBuilder` gives each unwired pin of
   a tactile switch with joined pairs a net of its own, so the simulation and
   the voltage check see the same A1–B1–B2 chain.
5. 2026-11-08 — `PartFacts.kindName(_ part:)` already existed (plan-24 added
   it for converters); the tactile case was added to it.

## Summary

Risk: the gesture (a lost mouse-up leaves a switch pressed — covered by release on
`handleEnded` and on leaving play mode) and the closed-contact conversion in
`NetlistBuilder`. Untouched: DIP switches, the backend, part files.
