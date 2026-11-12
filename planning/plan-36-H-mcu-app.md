# plan-36-H: App — Run disabled for unsimulated parts, microcontroller facts and kind

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-G

H makes the app aware of `Part.simulated` and the `mcu` kind:

- `CatalogPart.isSimulated`. While any placed chip has `isSimulated ==
  false`, **Simulation › Run** is disabled, and the ⇧⌘R key path does
  nothing. A disabled line under Run names the first such chip:
  `U1 (ATMEGA328P-PU) is not simulated`, or `U1 (ATMEGA328P-PU) and 2
  more are not simulated`.
- The `mcu` kind: `kindName` "Microcontroller", the `.mcu` facts (core,
  clock, flash, RAM, EEPROM, PSRAM), a "Simulation: Not simulated" fact
  row, the `.microcontroller` reference category (prefix `U`), and the
  Part Editor kind lists (no form).
- The keyboard help line for ⇧⌘R mentions the rule.

The single behavioral outcome: in the running app, placing an
ATMEGA328P-PU on an empty schematic disables Simulation › Run and shows
the line `U1 (ATMEGA328P-PU) is not simulated`. Pressing ⇧⌘R does not
start the simulation. Deleting the part enables Run again, and Run then
starts. `SimulationAvailabilityTests` proves the same without the GUI.

References:

- `schematical/schematical/Simulation/SimulationCommands.swift`
  (`SimulationTarget` :7–13, the Run button :42–46 with `.disabled(target?.isAvailable != true)`).
- `schematical/schematical/ContentView.swift` (the ⇧⌘R monitor case
  `.rotateBack` :184–189, which calls `toggleSimulation()` directly and
  skips `isAvailable`; `toggleSimulation` :277–285; the
  `.focusedSceneValue(\.schematicSimulation, …)` :409–410).
- `Backend/PartCatalog.swift` (`CatalogPart` :251, `Details` :268–321, the
  `init(_ part: Part, …)` :635–661 and its details chain :690–760).
- `Backend/PartFacts.swift` (`rows(for:)` :8, `kindName(_ kind:)`
  :145–176).
- `Preferences/ReferencePrefixes.swift` (`.microcontroller` :38, `of(_:)`
  :72–91: `PART_CPU` → `.processor`, `default` → `.integratedCircuit`).
- `Editor/BundledPartFile.swift:66–95` (`PartKinds.name(of:)`),
  `Editor/KindSchema.swift:203–235` (`PartKinds.all`, `displayNames`,
  `schemas`).
- `Help/KeyboardHelp.swift:100` (the ⇧⌘R line).
- Tests: `schematicalTests/PartEditorWindowTests.swift:12–32`,
  `ReferencePrefixTests.swift`, `CapacitorTests.swift:20` (the per-kind
  `kindName` precedent), `TestSupport.swift:49` (`allParts`).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-G complete | `ls planning/completed/plan-36-G-*.md` → exists | NOT MET (2026-11-12: plan written) |
| The owner's uncommitted edits in the files H changes are committed | `git status --short schematical/schematical/ContentView.swift schematical/schematical/Simulation/SimulationCommands.swift schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Preferences/ReferencePrefixes.swift schematical/schematical/Help/KeyboardHelp.swift schematical/schematical/Editor/KindSchema.swift schematical/schematical/Editor/BundledPartFile.swift schematical/schematicalTests/PartEditorWindowTests.swift schematical/schematicalTests/ReferencePrefixTests.swift` → empty | NOT MET (2026-11-12: `ContentView.swift` modified; `ReferencePrefixes.swift` and `ReferencePrefixTests.swift` untracked) |

AGENTS.md: touch and commit only your own changes. H cannot edit a file
that holds someone else's uncommitted work, so this row is a hard gate.

## 1. Goal

- `SimulationAvailabilityTests` (new) passes. It covers: no unsimulated
  chip → available; one → not available, with the reason line; three → `…
  and 2 more …`; toggling with an unsimulated chip placed leaves
  `isRunning` false; the PCB page → not available (today's rule, kept).
- The full Swift suite passes: the count when H starts, plus the new
  tests.
- The real-app check of § Goal's outcome, with a screenshot in
  `planning/plan-36-data/h-app-check.png`.

### Non-goals (explicit constraints)

- No Part Editor form for `mcu` (`PartKinds.schemas` stays without it, so
  "New Part…" does not offer it; the plan-33 capacitor precedent).
- No new symbol: `symbol(for:)`'s `default` (`.block`) draws the MCU
  (`PartCatalog.swift:991–1017`).
- No change to the file format: placed parts are saved by name, and
  `simulated` comes from the library.
- No change to the BOM or the PCB page beyond what follows from the new
  parts.

## 2. Current State

- Run is disabled only off the Schematic page (`isAvailable: page ==
  .schematic`, `ContentView.swift:409–410`). ⇧⌘R reaches
  `toggleSimulation()` without that check (`:184–189`).
  `toggleSimulation()` guards only `page == .schematic`.
- If Run were reached with an unsimulated part, F's refusal would show as
  "Simulation stopped: part "…" is not simulated" in the error capsule
  (`ContentView.swift:565–573`; `Simulation.swift:311–322` copies
  `circuit_error`).
- Editing while running is already blocked: the drawer is disabled
  (`SchematicSidebarView.swift:64`), as are paste (`ContentView.swift:251`)
  and delete (`:219`). So an unsimulated part cannot be added during a run.
  Opening a file stops the run first (`ContentView.swift:91–96`).
- `ReferencePrefixes.swift` already has a `.microcontroller` category
  (title "Microcontroller", prefix `U`, `:38`, `:65`) that no kind maps to
  yet (`:4–5`).
- `PartEditorWindowTests` pins the kind lists: `enabledKinds == kindNames
  minus "capacitor"` and count 26 (`:14–15`), `files.count == 27` and
  `kindNames == files` (`:27–28`), and a `displayNames` entry for each kind
  (`:29–31`). Adding `mcu.json` breaks them until they are updated.
- `TestSupport.allParts` is used by tests that pick "the first part whose
  footprint id has prefix X" (`BoardPreviewTests.swift:13`,
  `FootprintPreviewTests.swift:11`). A new `QFN-`/`TQFP-` footprint can
  change which part they pick.

## 3. Design

### 3.1 Facts (`PartCatalog.swift`, `PartFacts.swift`)

- `CatalogPart.isSimulated: Bool` = `part.simulated != 0`.
- `Details.mcu(core: String, maxHz: UInt64, flash: UInt32, ram: UInt32,
  eeprom: UInt32, psram: UInt32)` from `part.mcu?.pointee`, as a new
  branch in the details chain.
- `PartFacts.rows`: `Core` (`avr` → "AVR", `avrrc` → "AVR (reduced core)",
  `avrxt` → "AVRxt", `avrxm` → "AVR XMEGA", `cortex-m0plus` → "Dual
  Cortex-M0+", `xtensa-lx6` → "Xtensa LX6", `xtensa-lx7` → "Xtensa LX7",
  `riscv` → "RISC-V"). `Clock` uses the existing frequency formatter (e.g.
  "20 MHz"). `Flash` / `RAM` / `EEPROM` / `PSRAM` are shown in bytes as
  KB/MB (binary, "32 KB"), and a zero size is omitted. For every part with
  `!isSimulated`, a row `Simulation` → "Not simulated".
- `kindName`: `case PART_MCU: return "Microcontroller"`.

### 3.2 Run availability (`ContentView.swift`, `SimulationCommands.swift`)

- `SimulationTarget` gains `blockedReason: String?`.
- `ContentView`:

  ```swift
  /// Placed chips the backend has no model for (plan-36). Run is off while any is on the schematic.
  private var unsimulatedChips: [PlacedChip] { document.chips.filter { !$0.part.isSimulated } }
  ```

  `isAvailable: page == .schematic && unsimulatedChips.isEmpty`, with
  `blockedReason` from `SimulationAvailability.reason(unsimulatedChips)`.
  The text is `"\(ref) (\(part)) is not simulated"`, or `"… and \(n) more
  are not simulated"`, with the first chip in reference order.
- `toggleSimulation()`: when not running and `!unsimulatedChips.isEmpty`,
  return without starting (the ⇧⌘R path). Stop always works.
- `SimulationCommands`: under Run, when `blockedReason` is non-nil, a
  disabled `Text(reason)` item. The Speed submenu keeps its current rule.
- The rule lives in a small pure type, `Simulation/SimulationAvailability.swift`:
  `static func reason(_ chips: [PlacedChip]) -> String?`, so the test does
  not need a window.

### 3.3 Kind lists, references, help

- `BundledPartFile.swift` `name(of:)`: `case PART_MCU: "mcu"`.
- `KindSchema.swift`: `"mcu"` in `PartKinds.all`, and `displayNames["mcu"] =
  "Microcontroller"`. No `schemas` entry.
- `ReferencePrefixes.swift` `of(_:)`: `case PART_MCU: return .microcontroller`.
  Update the `:4–5` comment, since the category now has parts.
- `KeyboardHelp.swift:100`: `entry("⇧⌘R", "Run or stop the simulation
  (Simulation menu, with Speed). Not available while a part that isn't
  simulated, such as a microcontroller, is on the schematic")`.
- `PartEditorWindowTests`: the kinds without a form become `capacitor` and
  `mcu`. The counts are derived (`kindNames.count - 2`, `files.count`)
  instead of the literals 26 / 27. Plan-34 adds `resistor.json` and moves
  the same lines, so whichever plan lands second derives rather than
  hard-codes. If plan-34 has already made the counts derived, this task is
  moot (`grep -c 'enabledKinds.count, 2[0-9]' schematical/schematicalTests/PartEditorWindowTests.swift`
  → 0).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Facts, kind lists, references

- [ ] § 3.1 in `PartCatalog.swift` and `PartFacts.swift`.
- [ ] § 3.3 in `BundledPartFile.swift`, `KindSchema.swift`,
      `ReferencePrefixes.swift`, `PartEditorWindowTests.swift`.
- [ ] `schematicalTests/McuTests.swift`: `kindName` of ATMEGA328P-PU is
      "Microcontroller"; its facts include `Core` "AVR", `Flash` "32 KB",
      `Simulation` "Not simulated"; ESP32-S3-WROOM-1-N8R8 shows `Flash`
      "8 MB" and `PSRAM` "8 MB"; `ReferenceCategory.of` the part is
      `.microcontroller` and its prefix `U`.

Acceptance: the new and the changed tests pass.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p36-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/McuTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 6 min, mostly the build).
Commit: —

### Phase 2 — Run availability

- [ ] `Simulation/SimulationAvailability.swift` (§ 3.2).
- [ ] `ContentView.swift`, `SimulationCommands.swift` (§ 3.2);
      `KeyboardHelp.swift` (§ 3.3).
- [ ] `schematicalTests/SimulationAvailabilityTests.swift` (§ 1's cases).
      The toggle case builds a `SimulationController` and a document with
      the part, calls `start(document:)` through the same guard
      `toggleSimulation` uses (the guard is a static on
      `SimulationAvailability`), and checks that `isRunning` is false.

Acceptance: the availability rule holds without the GUI.
  Check: `xcodebuild test … -only-testing:schematicalTests/SimulationAvailabilityTests` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — Full suite and the real app

- [ ] The full Swift suite. A test that picks "the first part with
      footprint prefix X" and now gets an `mcu` part is narrowed to its
      original kind (`&& $0.kind == PART_…`), and each such change is noted
      here.
- [ ] Real app (the `run` skill; the memory notes on opening files and on
      other sessions driving the screen apply). On an empty schematic,
      place ATMEGA328P-PU: Simulation › Run is disabled and shows `U1
      (ATMEGA328P-PU) is not simulated`; ⇧⌘R does nothing. Delete it: Run
      is enabled, and Run starts the simulation. Also place an
      ESP32-WROOM-32E-N4 and an SC0914(13): each draws as a block with
      ports on both sides, and the PCB page shows its footprint. Save the
      screenshot of the disabled menu to `planning/plan-36-data/h-app-check.png`.
      Move the pointer to (0, 0) at the end (AGENTS.md).

Acceptance: the suite passes, and the app behaves as § Goal says.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p36-dd CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min: the suite is the only check that catches the `allParts` pick changes); the screenshot shows the disabled Run and its line.
Commit: —

## Validation Plan

- Tests: `McuTests`, `SimulationAvailabilityTests`, the updated
  `PartEditorWindowTests` and `ReferencePrefixTests`.
- Coverage check: both new test classes appear in the `xcodebuild test`
  log (`grep -c 'SimulationAvailabilityTests' <log>` > 0).
- Runtime proof: Phase 3's real-app check and screenshot.
- Doc sync: `Help/KeyboardHelp.swift` (the in-app help). Tick the to-do
  item in `planning/todo.md` (§ Parts, "Add hobby microcontrollers…") in
  plan-36-I.
- Final gate: plan-36-I.

## Decisions

- **Run disabled, with the reason in the menu** — owner: "When any are
  added to a schematic the 'play' button should be disabled". Run has
  lived in the Simulation menu since fbde398. The reason line is the
  author's: a greyed-out item with no explanation reads as a bug.
- **The ⇧⌘R guard is in `toggleSimulation`** — author: the key monitor
  calls it directly (§ 2), so a disabled menu item alone would not stop
  it.
- **No Part Editor form** — author: the editor's forms come from the kinds'
  keys. An `mcu` part's pins come from Microchip's data, and a hand-made
  MCU would need a form for 100-pin tables. Nobody has asked for one, and
  the capacitor kind already ships without one.

## Corrections

## Summary

H is mostly bookkeeping: the facts, the kind name, the lists. One piece is
behavior: Run's availability. It sits in a pure type with its own tests,
and it is guarded on both entry paths, the menu and ⇧⌘R. The full suite
catches tests that quietly start picking a microcontroller.
