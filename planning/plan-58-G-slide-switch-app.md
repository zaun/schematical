# plan-58-G: Slide switches in the app — drawer, symbol, click, sidebar, file, automation

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-58-F

The library now has slide switches (plan-58-C…F). G puts them in the app: they
show in the parts drawer under **Switches & Relays → Slide → `SPDT On-On`** (one
leaf per circuit and action), draw as a slide-switch symbol whose blade and knob
show the position, and in play mode a click moves the knob to the next
position. The simulation and the voltage check see the contacts exactly as the
backend does (plan-58-B). The sidebar names them "Slide switch" and shows their
facts; the file keeps the position; the agent `set_switch` tool selects a position.

Outcome: place an SPDT On-On switch, wire 5 V → common, throw A → LED 1 → GND,
throw B → LED 2 → GND, start the simulation: LED 1 is lit; click the switch:
LED 2 is lit and LED 1 dark; click again: back. A center-off switch goes
1 → 2 (nothing connected) → 3 → 2 → 1. Saving and reopening keeps the position;
ending play mode restores the position the schematic had before it.

References:

- `planning/plan-58-B-slide-switch-kind.md` § 4 (states, `rest`,
  `part_switch_state_contacts`, `circuit_set_switch_state`, pin names).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details`, `count`,
  `symbol(for:)`, drawer `groups`, filters), `Backend/PartFacts.swift`,
  `Canvas/SchematicModel.swift` (`PartSymbol`, `ChipLayout`, `PlacedChip`,
  `toggleSwitch`, `restoreSwitches`), `Canvas/SchematicCanvasView.swift`
  (`Interaction`, `handleChanged`, `handleEnded`), `Canvas/TactileSwitchSymbolView.swift`
  and `Canvas/DipSwitchSymbolView.swift` (the models for the new view),
  `Canvas/SchematicExportScene.swift`, `Canvas/ReferenceLabel.swift`,
  `BoardPreview/PartLook.swift`, `Simulation/Simulation.swift` (`NetlistBuilder`, `setSwitch`),
  `Simulation/VoltageCheck.swift`, `Document/SchematicFile.swift`,
  `Automation/AutomationSimulation.swift` (`set_switch`).
- `docs/file-format.md` (`switches`), `docs/agent-control.md`, `docs/parts-format.md`
  (§ drawer categories), `AGENTS.md` (§ Testing in the app),
  `planning/completed/plan-27-J-tactile-app.md` (the same letter for tactile switches).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-F is complete | `ls planning/completed/plan-58-F-* \| wc -l` → 1 | MET (2026-11-24; re-run: 1) |
| Slide parts are in the library | `grep -c 'states=' backend/tests/golden/parts.txt` → ≥ 1 | MET (2026-11-24; re-run: 271) |
| No slide code in the app yet | `grep -rc 'slideSwitch' schematical/schematical --include='*.swift' \| grep -v ':0' \| wc -l` → 0 | MET (2026-11-24; re-run before any edit: 0) |
| The drawer's Switches & Relays group exists | `grep -c 'Switches & Relays' schematical/schematical/Parts/switch.json` → ≥ 1 | MET (2026-11-24; re-run: 220) |

If plan-58-F is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The drawer lists slide switches under Switches & Relays → Slide with one leaf
  per `function` (`SPDT On-On`, …); each part row shows its package, actuator and
  `N V max`; the DIP "(n)" position-count menu does not appear for it.
- A slide chip draws as a slide switch with its terminals; the blade (and the
  knob) shows the current position.
- In play mode and in edit mode, a click on the body moves the knob (§ 4.3); the
  simulation and the voltage check see the contacts; the move is refused with the
  voltage check's message when it would join a net the rating forbids.
- The sidebar's Kind row says "Slide switch"; Positions is replaced by Circuit,
  Action, Position, Actuator, Rating.
- The file saves `switches` (per contact, as for a DIP switch); a file whose
  `switches` matches no position opens in the part's rest position.
- The agent `set_switch` tool takes `position` = the knob position for a slide
  switch (§ 4.5).

### Non-goals (explicit constraints)

- DIP and tactile switch drawing, click behavior, saved `switches`, drawer count
  menu and sidebar: unchanged.
- No change to the backend (B), part files or goldens, and no new file-format
  key or version (`switches` is the saved form).
- No per-instance option; no spring-back; no knob-drag interaction (a click only).

## 2. Current State

- `CatalogPart.Details` has `.dipSwitch` and `.tactileSwitch`; the loader picks
  `.tactileSwitch` when `dip.momentary != 0` and `.dipSwitch` otherwise
  (`PartCatalog.swift:1141–1148`); `symbol(for:)` returns `.tactileSwitch` or
  `.dipSwitch` for every `PART_SWITCH` (`PartCatalog.swift:1632`);
  `PartFacts.kindName` returns "Tactile switch" / "DIP switch" (`PartFacts.swift:397`).
- The drawer's DIP position-count menu keys on `part.kind == PART_SWITCH,
  !part.isTactileSwitch, part.count` (`PartCatalog.swift:1892`, `:2450`, `:2544`).
- `PlacedChip.switchesOn` is `[Bool]`, one entry per contact (`SchematicModel.swift:900`),
  saved as `switches` unless the part is tactile (`SchematicFile.swift:772`),
  read back as `part.switches ?? []` (`SchematicFile.swift:1061`);
  `toggleSwitch(_:position:)` flips one contact with the voltage check
  (`SchematicModel.swift:4900`); `restoreSwitches` puts every switch back when
  play mode ends (`SchematicModel.swift:4884`).
- `Simulation.NetlistBuilder` turns `chip.switchesOn` into the contacts that are
  ON at start (`Simulation.swift:211–227`) and `setSwitch` flips one contact in a
  running simulation (`Simulation.swift:420`).
- The agent tool `set_switch` flips a DIP position or presses a tactile switch
  (`AutomationSimulation.swift:54`, `setSwitch` below it).

### Measured populations

| What | Count | Command |
|---|---|---|
| Places that name the tactile symbol or details (the places a new switch kind must also handle) | 33 lines in 10 files: PartCatalog 15, SchematicModel 8, PartFacts 2, Simulation 2, and one each in AutomationSimulation, PartLook (`BoardPreview/`), ReferenceLabel, SchematicCanvasView, SchematicExportScene, SchematicFile | `grep -rn 'tactileSwitch\|isTactileSwitch' schematical/schematical --include='*.swift' \| cut -d: -f1 \| sort \| uniq -c` (2026-11-24) |
| App test classes about switches | 3 (`TactileSwitchTests`, `PlaySwitchResetTests`, `PowerSwitchTests`) | `ls schematical/schematicalTests \| grep -i switch` |
| Harness scripts for switches | 3 (`tactile-switch.swift`, `tactile-footprints.swift`, `powerswitch-app.swift`) | `ls scripts/harness \| grep -i -E 'switch\|tactile'` |

### Verified properties

- `switchesOn` per contact is exactly the form `VoltageCheck` and
  `NetlistBuilder` consume (`Simulation.swift:211–227`, `VoltageCheck.swift:56`):
  a slide switch needs no second representation; its position is the one state
  whose mask equals `switchesOn` (§ 4.2).
- The app compiles the backend sources (`schematical-Bridging-Header.h`), so
  `part_switch_state_contacts` and `circuit_set_switch_state` of plan-58-B are
  visible to Swift once their headers are included there.

## 3. Design Overview

A slide switch is a chip whose `switchesOn` always equals one of its states'
contact masks. The catalog gives each part its state masks and rest position
(`Details.slideSwitch`); the model's one new operation, `setSlideState`, replaces
`switchesOn` with a state's mask after the voltage check (the same pattern as
`toggleSwitch`), as an undoable edit outside play mode and as a play-mode input
(restored when play ends) inside it. The symbol is a new `SlideSwitchSymbolView`
drawn through `SchematicPainter`, with circuit-specific geometry (SPST, SPDT,
DPST, DPDT). Risk is in the places that assume "a switch is DIP or tactile":
every place of the measured list must handle the third kind, and the DIP
drawer count menu must skip it. Byte-identity does not apply (the app's
behavior changes on purpose); the gates are the new `SlideSwitchTests`, the
three existing switch test classes unchanged, and the harness.

Rejected: reusing `.dipSwitch` with positions (the DIP symbol and click flip one
contact; a slide switch moves between states); a new file key `slideState` (the
per-contact `switches` is already saved, loaded and restored by every path);
moving the knob by drag (a click is enough and works in the harness).

## 4. Detailed Design

### 4.1 Catalog

- `Details.slideSwitch(circuit: String, positions: Int, rest: Int, masks: [UInt32], actuator: String, ratedMV: UInt32)`;
  `circuit` is the first word of the part's `function` (`SPST` …).
  Loader (`PartCatalog.swift:1141`): `dip.states > 0` is checked first, then `momentary`.
  `masks` come from `part_switch_state_contacts` for states `1…dip.states`.
- `isSlideSwitch`; `isTactileSwitch` stays false for it. The DIP count menu
  conditions at `:1892`, `:2450`, `:2544` gain `!part.isSlideSwitch`.
- `symbol(for:)` → `.slideSwitch`. The drawer path is the part's `drawer`
  (`["Switches & Relays","Slide"]`), leaf = `function`; its row detail reads
  `<package> · <actuator> · <N> V`.

### 4.2 Chip state

- `PlacedChip.init` for a slide switch: `switchesOn` = the mask of `rest` when
  the saved array matches no state's mask (including empty).
- `var slidePosition: Int?` (1-based) = the index of the state whose mask equals
  `switchesOn`; nil for other parts.
- `SchematicFile.swift:772`: write `switches` for slide switches like DIP.

### 4.3 Interaction and model

- Click anywhere on a slide chip's body (edit or play mode, the tool that
  flips DIP levers) calls `setSlideState(id, to: next)` where `next` is the
  position after `slidePosition` in the order 1 → 2 → … → N → N-1 → … → 1
  (two positions flip; three go 1, 2, 3, 2, 1, …): the chip remembers the direction
  in a non-saved `slideDirection`.
- `SchematicModel.setSlideState(_ id: UUID, to state: Int) -> Int?`: returns nil
  and sets `editorMessage` when the state does not exist or the voltage check
  refuses it (`VoltageCheck.firstNew(before:after:…)`, as `toggleSwitch`); otherwise
  sets `switchesOn` to the mask, clears `editorMessage`, returns the state. Outside
  play mode it is a recorded edit ("Move Switch"); in play mode it is an input
  restored by `restoreSwitches`.
- `Simulation.setSlide(_ id: UUID, state: Int)` calls `circuit_set_switch_state`
  (break before make) instead of per-contact `setSwitch`.

### 4.4 Symbol and sidebar

`SlideSwitchSymbolView` (model: `TactileSwitchSymbolView`): body box with the
terminals on its two sides (common side left; throws right, top `A` and bottom `B`
in SPDT/DPDT, one row per pole), a blade from the common terminal to the throw the
state connects (none for an Off state: the blade rests mid-way), a knob that sits
at the position's end of a slot under the body, 1 / 2 / 3 dots for the positions.
SPST/DPST: one/two contacts, blade closes onto the terminal in the On state.
`SchematicExportScene` and `ReferenceLabel` get the new symbol case. Sidebar
(`PartFacts.swift`): Kind "Slide switch"; rows Circuit (`SPDT`), Action (`On-On`),
Position (`2 of 2`), Actuator, Rating (`N V`).

### 4.5 Automation

`set_switch` on a slide switch: `position` is the knob position (1…N, required: a
missing one is refused with `"<ref> is a slide switch: give position 1 … N"`); `on`
must be true or absent (`on=false` is refused: `"a slide switch is always in one
position"`); the result is `"<ref> position <n>"`. Out of play it is an edit; during
play an input. `docs/agent-control.md` and the tool's `summary` say so.

## Phases

In the checks below `XC` = `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p58-dd CODE_SIGNING_ALLOWED=NO`.

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` partial; moot tasks stay with
> evidence; fill each phase's `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — catalog, drawer, chip state, file

- [x] Re-run the measured list (§ 2) and record the files (33 lines in the same 10 files: AutomationSimulation 1,
      PartCatalog 15, PartFacts 2, PartLook 1, ReferenceLabel 1, SchematicCanvasView 1, SchematicExportScene 1,
      SchematicModel 8, SchematicFile 1, Simulation 2); `PartCatalog.swift`:
      `Details.slideSwitch`, `isSlideSwitch`, loader, `symbol(for:)`, drawer row detail,
      the three DIP-count guards (§ 4.1). `PartSymbol.slideSwitch` with pin layout
      (`ChipLayout`) from the pinout; `PartFacts.swift` kind name and rows (§ 4.4).
- [x] `SchematicModel.swift` / `SchematicFile.swift`: `PlacedChip` init rest
      mask, `slidePosition`, saved `switches` (§ 4.2).
- [x] Tests: `schematical/schematicalTests/SlideSwitchTests.swift` — drawer path and leaves
      (`Switches & Relays/Slide/SPDT On-On`), details of an SPDT On-On, an SPDT On-Off-On
      and a DPDT part (masks, rest), symbol, kind name and sidebar rows, DIP count menu
      absent for a slide leaf, file round-trip of the position, a file with a bad
      `switches` opens in the rest position, a new chip starts in `rest`.

Acceptance: the catalog and file tests pass; the three existing switch test
classes still pass.
  Check: `$XC test -only-testing:schematicalTests/SlideSwitchTests -only-testing:schematicalTests/TactileSwitchTests -only-testing:schematicalTests/PlaySwitchResetTests -only-testing:schematicalTests/PowerSwitchTests` → `** TEST SUCCEEDED **` (est. 12 min: build plus four small classes).
Measured: 2026-11-24, `** TEST SUCCEEDED **`: SlideSwitchTests 12 tests, TactileSwitchTests 11, PlaySwitchResetTests 1, PowerSwitchTests 7 (31 tests, 0 failures).
Commit: —

### Phase 2 — symbol, click, simulation

- [ ] `Canvas/SlideSwitchSymbolView.swift` (§ 4.4) for the four circuits, and its
      cases in `SchematicExportScene.swift`, `ReferenceLabel.swift`, the canvas'
      hit-testing and drawing switch (`SchematicCanvasView.swift:1193` and the
      other places of the measured list).
- [ ] `SchematicModel.setSlideState` and the click handling (§ 4.3);
      `Simulation.setSlide` and the netlist start state; `restoreSwitches` covers it.
- [ ] Tests (`SlideSwitchTests.swift`): `setSlideState` flips `switchesOn` and
      `slidePosition`, the ping-pong order for three positions, a refused move
      (3 V-rated slide switch on a 5 V net: the message and no change), undo restores,
      the simulation (SPDT between 5 V and two `GENERIC-LED`s: LED 1 lit, LED 2 lit
      after the move, break before make), `restoreSwitches` after play.

Acceptance: the tests above pass; the symbols draw in every position of the four circuits
(SPST on/off, SPDT 1/2 and 1/2/3, DPST on/off, DPDT 1/2 and 1/2/3) in the harness images.
  Check: `$XC test -only-testing:schematicalTests/SlideSwitchTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 3 — automation, BOM, docs, harness

- [ ] `AutomationSimulation.swift` `set_switch` (§ 4.5) and its test
      (`AutomationTests`: position select, missing position refused, `on=false` refused).
- [ ] BOM: a slide switch lists with its part number and `function`
      (`BillOfMaterialsTests` case).
- [ ] Docs: `docs/file-format.md` `switches` row ("DIP and slide switches"),
      `docs/agent-control.md` (`set_switch`), `docs/parts-format.md` § drawer
      categories (`Switches & Relays | DIP; Tactile; Relay (plan-47); Slide`).
- [ ] `scripts/harness/slide-switch.swift` (model: `tactile-switch.swift`): on the real
      catalog places an SPDT On-On, an SPDT On-Off-On and a DPDT On-On slide
      switch, wires 5 V → switch → `GENERIC-LED`s, checks the drawer leaves,
      the click order, the simulation's LEDs, the file round trip and writes the
      symbol and sidebar images; one `ok` / `FAIL` line per check.

Acceptance: `AutomationTests` and `BillOfMaterialsTests` pass with the new cases;
the harness prints only `ok` lines and its images show the knob positions.
  Check: `rm -rf /tmp/p58-h && scripts/harness/build.sh scripts/harness/slide-switch.swift /tmp/p58-h && /tmp/p58-h/H.app/Contents/MacOS/harness /tmp` → every line `ok`, exit 0 (est. 6 min), and `$XC test -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/BillOfMaterialsTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

## Validation Plan

- Tests: `SlideSwitchTests` (catalog, chip, file, model, simulation, voltage
  refusal, undo), `AutomationTests`, `BillOfMaterialsTests`; the three existing
  switch classes unchanged.
- Coverage check: every file of the measured list (§ 2) that names `.tactileSwitch`
  also names `.slideSwitch` (`for f in $(grep -rl 'tactileSwitch' schematical/schematical --include='*.swift'); do grep -L 'slideSwitch' $f; done` → empty, except files that only read tactile facts, each named in Corrections).
- Runtime proof: the harness (real catalog, document, simulation and views) and
  the images it writes, read by eye.
- Doc sync: `docs/file-format.md`, `docs/agent-control.md`, `docs/parts-format.md`.
- Final gate (once, after Phase 3): `$XC test` with the classes of Phases 1–3
  plus `PartKindKeysTests`, `PartKindRoundTripTests`, `KindInfoTests`, `KindGuideTests`,
  `SchematicFileV5Tests`, `PCBUndoTests`, `KiCadExportTests`, `EasyEDAProExportTests`,
  `ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 40 min).

## Decisions

- Click moves the knob to the next position and back, 1 → 2 → 3 → 2 → 1 for three
  positions — the author's call (no per-side hit testing, works in the harness).
- The saved form is `switches` per contact, no new key — the author's call.
- Agent `set_switch` selects a position for a slide switch — the author's call
  (the tool's `position` already means "which one" for the other switches).
- Latching only, no panel mount, no illuminated — owner and author, plan-58-A § Decisions.

## Corrections

- **Phase 1 — the symbol cases land with `PartSymbol.slideSwitch`.** Swift's
  `switch` over `PartSymbol` is exhaustive (`SchematicCanvasView`,
  `SchematicExportScene`, `ReferenceLabel`, `ChipLayout`), so the case, the layout
  and the canvas / export / label cases compile only together. Phase 1 therefore
  carries `SlideSwitchSymbolView` as far as the box, header and pin stubs; Phase 2
  adds the blades, knob and the click.
- **Phase 1 — `SchematicFile.swift:772` needed no code change.** It already saves
  `switches` for every part except a tactile switch, so a slide switch saves like
  a DIP switch (only a comment was added). Its `isTactileSwitch` is not a
  `.tactileSwitch` pattern, so the Validation Plan's coverage grep does not list it.
- **Phase 1 — `TactileSwitchTests.firstDip` named every non-tactile switch a DIP
  switch** (`kind == PART_SWITCH && !isTactileSwitch`) and picked a slide part
  (`1101M2S3CQE2` sorts first), so `testDipSwitchUnchanged` failed. The helper now
  also says `!isSlideSwitch` (one line; no assertion changed). `PlaySwitchResetTests`
  and `TestSupport.firstDip` already require `count >= 2`, which a slide part
  (count nil) never has.

## Summary

The risk is breadth: every place that treats "a switch" as DIP or tactile
(the measured list), and the DIP count menu that must skip slide leaves. The
backend, part files, goldens and file-format version are untouched.
