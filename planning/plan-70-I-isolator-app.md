# plan-70-I: App: details, facts, kind names, the drawer, the `isolator-app` harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-H

I makes the app show optocouplers and digital isolators: their facts in the
sidebar, their kind names, and their place in the drawer. A harness proves
they run in play mode in the real app.

References:

- plan-70-G § 3.5 (`PartIsolator`), § 3.4 (the model); plan-70-H § 3.1–3.2
  (drawer paths, generics).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details`,
  `symbol(for:)`, `:2028`), `Backend/PartFacts.swift` (`kindName`, `:377`
  and `:478`), `Preferences/ReferencePrefixes.swift` (`of`, `:75`).
- `planning/completed/plan-48-H-timer-opamp-app.md` (the shape),
  `scripts/harness/relay-buzzer-app.swift`, `scripts/harness/build.sh` (the
  harness precedent).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-H complete | `ls planning/completed/plan-70-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |
| The generics are in the catalog | `grep -c '^GENERIC-OPTO\|\|^GENERIC-ISOLATOR\|' backend/tests/golden/parts.txt` → 2 | NOT MET (2026-11-25: 0) |
| The app does not know the kind yet | `grep -c 'PART_ISOLATOR' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift` → 0 and 0 | MET (2026-11-25: 0 and 0) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog, document and `SimulationController`
(`scripts/harness/isolator-app.swift`, every line `ok`):

- The drawer has **Interface → Optocoupler** (function `Generic`, and the
  sub-groups `Transistor`, `Darlington`, `Triac`, `Logic` with their
  channel-count functions) and **Interface → Digital isolator** (`Generic`
  and the `<n> channels (<f>/<r>)` functions).
- `GENERIC-OPTO` placed: reference `U1`, kind name "Optocoupler", a block
  with its four pins. A two-channel transistor part reads "Dual
  optocoupler", a triac part "Triac driver", a logic part "Logic
  optocoupler", and `GENERIC-ISOLATOR` "Digital isolator". The facts rows
  are those of § 3.2.
- Play mode, the opto circuit. One section of a DIP switch (the first
  catalog DIP switch) runs from 5V to a `GENERIC-RESISTOR`, then to `A`;
  `K` goes to GND. On the output side, 5V runs through a second
  `GENERIC-RESISTOR` to a `GENERIC-LED`'s anode, its cathode goes to `C`,
  and `E` goes to GND. With the switch on, the LED has on-time; with it off,
  the LED is dark.
- Play mode, the isolator circuit. 5V goes on `VDD1` and a 3.3 V supply on
  `VDD2`, each side with its own ground. A switch drives `VIA`, and `VOA`
  runs through a resistor and an LED to GND2. `VOA` follows the
  switch. With side 1's supply wire removed (`VDD1` floating), `VOA` reads 1
  (`failsafe` `high`) whatever the switch does.
- A placed isolator leaves Run on (`SimulationAvailability.canStart`).

### Non-goals (explicit constraints)

- No new symbol: isolators draw as blocks (`symbol(for:)`'s `default`), as
  timers and op-amps do.
- No new reference category: `U` (`ReferenceCategory.of`'s default).
- No light or animation on the block. G's lights hooks are not shown.
- No file-format change.

## 2. Current State

- `CatalogPart.Details` (`PartCatalog.swift:799`–`:806` for relay, timer,
  op-amp) has one case per kind with facts. The builder branches on
  `part.<kind>?.pointee` (`:1696` relay, `:1710` op-amp).
- `PartFacts.kindName(_ kind:)` (`:377`) names `PART_RELAY` "Relay" (`:424`)
  and `PART_OPAMP` "Op-amp" (`:427`), and returns "" for a kind it does not
  name. `kindName(_ part:)` (`:478`) refines by facts.
- The drawer groups by `drawer[0]`, then the group and sub-group, with no
  list to extend. "Interface" already exists (Serial, Parallel I/O, Driver).
- `ReferenceCategory.of` (`ReferencePrefixes.swift:75`) maps `PART_RELAY` to
  `.relay` (`:92`) and every unnamed kind to `.integratedCircuit` ("U").

## 3. Design

### 3.1 Catalog (`Backend/PartCatalog.swift`)

```swift
struct IsolatorInfo: Hashable, Sendable {
    let type: String; let channels: Int; let darlington: Bool; let zeroCross: Bool; let ac: Bool
    let output: String?; let inverting: Bool
    let visoV: UInt32; let ctrMinPct: UInt32; let ctrMaxPct: UInt32; let vceoMV: UInt32
    let vdrmMV: UInt32; let iftUA: UInt32; let rateKbps: UInt32; let failsafeHigh: Bool?
}
```

`Details.isolator(IsolatorInfo)`; the builder branches on
`part.isolator?.pointee`; accessor `isolator`. `detailLabels`: the type
word (`Transistor`, `Darlington`, `Triac`, `Logic`, `Digital`) and the
channel count.

### 3.2 Facts and names (`Backend/PartFacts.swift`)

- `kindName(_ kind:)`: `PART_ISOLATOR` "Optocoupler". `kindName(_ part:)`:
  "Dual optocoupler" / "Quad optocoupler" for 2 / 4 transistor channels,
  "Darlington optocoupler", "Triac driver", "Logic optocoupler", and
  "Digital isolator".
- Rows, each only when given: `Channels` (2 or more), `Isolation`
  (`"5000 V rms"`), `CTR` (`"50–600 %"` or `"≥ 100 %"`), `Vceo` (`"80 V"`),
  `Off-state voltage` (`"400 V"`, triac), `Trigger current` (`"10 mA"`),
  `Zero cross` (`Yes`), `Input` (`AC` for an AC input), `Output`
  (`Open collector`, `Push-pull`, `Tri-state`; `inverting` adds
  `, inverting`), `Data rate` (`"25 Mbit/s"`), `Default output` (`High` /
  `Low`, digital), and `Play mode`. That last row is `Logic: input drives
  the output across the barrier` for every isolator.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Details, facts, kind names

- [ ] `PartCatalog.swift` per § 3.1. Each `case` and branch is added only if
      absent, after the last kind present (plans 69, 71–74 add theirs too).
- [ ] `PartFacts.swift` per § 3.2.
- [ ] Tests: `schematicalTests/IsolatorTests.swift` covers the kind names
      of the four types and a dual, every facts row on fixture parts from
      the real catalog (the generics and the first part of each type), and
      the drawer paths.

Acceptance: the names, rows and paths match § 1.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p70-dd test -only-testing:schematicalTests/IsolatorTests CODE_SIGNING_ALLOWED=NO`
  → `** TEST SUCCEEDED **`. About 8 min, one app test build.
Commit:

### Phase 2: The harness

- [ ] `scripts/harness/isolator-app.swift` (from `relay-buzzer-app.swift`):
      the drawer groups, `U1`, the kind names, the opto circuit on and
      off, and the isolator circuit with and without side 1's supply. It
      checks Run on with an isolator placed. Parts come by name: the
      generics, and the first catalog part of each type, printed.
- [ ] `DrawerPerformanceTests` and `PartEditorWindowTests` stay green.

Acceptance: the harness prints only `ok` lines.
  Check: `APP=/tmp/p70-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/isolator-app.swift /tmp/p70-harness && /tmp/p70-harness/H.app/Contents/MacOS/harness`
  → every line `ok`, exit 0. Then `xcodebuild … test -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests`
  → `** TEST SUCCEEDED **`. About 10 min.
Commit:

## Validation Plan

- Tests: `IsolatorTests` (new); `DrawerPerformanceTests`,
  `PartEditorWindowTests`.
- Runtime proof: `isolator-app.swift` in the real app's catalog and
  simulation.
- Doc sync: none (`docs/parts-format.md` § `isolator` is H's and G's).
- Final gate: plan-70-J.

## Decisions

- Author: **block symbol, no light.** The block shows every pin, and play
  mode shows the effect on the wired LED. A drawn opto symbol is not in
  scope.
- Author: **reference `U`**, as all 432 symbols of KiCad 10's
  `Isolator.kicad_sym` use (`Reference` property counted at planning,
  2026-11-25).

## Corrections

## Summary

Small view work on the plan-48-H pattern. The harness is the proof that a
placed opto and isolator run in the real app.
