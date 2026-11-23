# plan-55-J: App — details, facts, kind name, the input-rating message, the harness

Last updated: 2026-11-22
Effort: medium (1h–2h)
Depends on: plan-55-I

J makes the app show and run the power switches: the sidebar's kind and facts (`PowerSwitchInfo`
from the backend's `PartPowerSwitch`), the kind names ("Load switch", "Dual current-limited
switch", "Ideal diode"), the input-rating message of the voltage check, a bill-of-materials line,
and a harness that drives the real app. The part editor's lists are H's; the voltage check and the
simulation need no change (A: the backend resolves the switched level; `voltage_check` has no new
argument). The behavioral outcome: a placed power switch draws as a block with its pins named; the
sidebar shows its kind and facts; in play mode with its input on 5 V and its enable active its
output drives a part, and the voltage check refuses an input above the part's maximum with a
message naming the part and the limit.

References:

- `planning/plan-55-A-powerswitch-kind.md` § 3.1 (the facts), § 3.4 (the clash records and the
  input rating); `planning/completed/plan-42-I-regulator-app.md` (the precedent, Phases 1 and 3).
- `schematical/schematical/Backend/PartCatalog.swift` (`RegulatorInfo` line 171, `Details.regulator`
  line 533, the accessor line 717, the details builder line 1109, `symbol(for:)` line 1464),
  `Backend/PartFacts.swift` (the facts rows line 213, `kindName` lines 291 and 304),
  `Simulation/VoltageCheck.swift` (`message`, line 161).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-I complete | `ls planning/completed/plan-55-I-*.md` → exists | NOT MET (2026-11-22: not started) |
| No app type for the kind yet | `grep -c 'PowerSwitchInfo' schematical/schematical/Backend/PartCatalog.swift` → 0 | MET (2026-11-22 at 7c7b4d31: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `PartCatalog.swift` has `PowerSwitchInfo`, `Details.powerswitch`, `CatalogPart.powerswitch`; the
  details builder fills it from `part.powerswitch`.
- The sidebar rows and kind names of § 3.1.
- `VoltageCheck.message` gives, for a `VOLTAGE_POWERSWITCH_INPUT_OVER` issue, `Can't connect: U1 pin 1 (VIN)
  gets 12 V; <part> takes at most 5.5 V in.`
- `scripts/harness/powerswitch-app.swift` prints `all checks ok`.

### Non-goals (explicit constraints)

- No change to `backend/`, `Parts/*.json`, the part editor's lists (H) or the simulation code.
- No new symbol: a power switch draws as a block (`symbol(for:)` returns `.block`).

## 2. Current State

The regulator's app support is the template (plan-42-I): `RegulatorInfo`, the facts rows, both
`kindName`s, the explicit `.block` symbol, `regulator-app.swift`, `RegulatorTests`. The backend
(A, H, I) loads the kind and its parts; the Swift `switch` statements that read a `PartKind` or a
`VoltageProblemKind` have `default:` branches (`PartCatalog.swift:1469`, `PartFacts.swift:293`,
`VoltageCheck.swift:164`), so the app already builds and shows a power switch as a block with an
empty kind name.

### Measured populations

| What | Count | Command |
|---|---|---|
| Files naming the regulator kind in the app and its tests | 4 | `grep -rIl 'PART_REGULATOR\|\.regulator\b' schematical/schematical schematical/schematicalTests` (7c7b4d31) |

## 3. Design

### 3.1 Facts and names

Sidebar rows (`PartFacts.swift`, `case let .powerswitch(info)`): `Type` (`Load switch`,
`Current-limited switch`, `Ideal diode`), `Channels` (when more than 1), `Input` (`1.5–5.5 V`,
`up to 6 V`, `from 2.7 V`, from `vinMinMV` / `vinMaxMV`), `Current` (`iout`), `Current limit`
(`ilimit`, a `limited` part), `On-resistance` (`milliohmsText`), `Quiescent` (`iq`); a row is
omitted when its value is 0. `kindName(_ kind: PartKind)` gives `Power switch`; `kindName(_ part:)`
gives the type's name with `Dual`, `Triple` or `Quad` in front for 2 to 4 channels (`Dual load
switch`, `Quad current-limited switch`, `Dual ideal diode` for a package with two inputs).

### 3.2 The messages

`VoltageCheck.message` has `case VOLTAGE_POWERSWITCH_INPUT_OVER` beside the regulator's, with the same
wording (`gets <v> V; <part> takes at most <v> V in.`). The supply-clash message already names the
lower voltage first and swaps the two parts when `chip` is the supply at the higher voltage
(`VoltageCheck.swift:146–153`); A records the switch as `other_part` against a supply, so it needs no
change (J's tests pin both directions).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, kind names

- [ ] `PartCatalog.swift`: `PowerSwitchInfo` (type, switches, `vinMinMV`, `vinMaxMV`, `ioutMA`,
      `ilimitMA`, `ronMOhm`, `iqUA`), `Details.powerswitch`, `CatalogPart.powerswitch`, the details
      builder from `part.powerswitch`, `detailLabels`, `symbol(for:)`'s explicit `.block` case.
- [ ] `PartFacts.swift`: the facts rows and both `kindName`s (§ 3.1).
- [ ] Tests: `schematicalTests/PowerSwitchTests.swift`: `kindName` and the facts of one load switch, one
      current-limited, one dual and one ideal diode from the loaded catalog (picked by `kind` and
      facts, not by name).

Acceptance: the sidebar shows a power switch's kind and facts.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p55-dd test -only-testing:schematicalTests/PowerSwitchTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min: the app test build; no smaller check runs Swift).
Commit: —

### Phase 2 — The voltage messages

- [ ] `VoltageCheck.swift`: the `VOLTAGE_POWERSWITCH_INPUT_OVER` case (§ 3.2).
- [ ] Tests: `PowerSwitchTests`: a placed power switch with a 12 V supply on its input pin gives one
      issue whose `message` is the § 1 text; a 5 V input switch whose output sits on the 3V3 rail gives
      `3.3 V (<the supply>) and 5 V (<the switch>) supplies on one net.`; a 3.3 V input switch on the 5V
      rail gives `3.3 V (<the switch>) and 5 V (<the supply>) supplies on one net.`; two switches of 5 V
      and 3.3 V on one net give no issue.

Acceptance: the messages name the part, the limit and both voltages.
  Check: the Phase 1 command → `** TEST SUCCEEDED **` with the new cases (est. 12 min).
Commit: —

### Phase 3 — The harness and the BOM check

- [ ] `scripts/harness/powerswitch-app.swift` (as `regulator-app.swift`): the drawer (`Power` → `Power
      Switches` → `Load Switches` / `Current-Limited` / `Ideal Diodes` and their leaves), the facts, a placed
      load switch on its own footprint, an LED lit through it in play mode and dark with its enable
      inactive, an ideal diode passing 5 V and an input above the part's maximum refused.
- [ ] `BillOfMaterialsTests`: a power switch line reads `U1`, the part name, its manufacturer and
      footprint (the existing line format).

Acceptance: the real app shows and runs power switches.
  Check: `APP=/tmp/p55-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/powerswitch-app.swift /tmp/p55-harness && /tmp/p55-harness/H.app/Contents/MacOS/harness` → `all checks ok` (est. 10 min: it drives the real app; a unit test cannot show the drawer and play mode). When the harness drives the real mouse, the pointer ends at the top-left corner of the screen (`AGENTS.md`).
Commit: —

## Validation Plan

- Tests: `PowerSwitchTests`, `BillOfMaterialsTests`.
- Coverage check: the tests pick parts from the loaded catalog by kind and facts, so they run on the
  real library (H and I's parts).
- Runtime proof: the harness.
- Doc sync: `docs/parts-format.md` § `powerswitch` names the sidebar rows if A left them out.
- Final gate: plan-55-K.

## Decisions

- Author: **a power switch draws as a block**, its pins named (the regulator's choice, plan-42-I):
  the kind's pin counts and names vary too much for one symbol.
- Author: **the kind name carries the type**, since a user looks for "load switch" or "ideal diode",
  not "power switch".

## Corrections

(none yet)

## Summary

Small and mechanical: the facts and names copy the regulator's. The harness is the proof that the
drawer, the placement and play mode work together.
