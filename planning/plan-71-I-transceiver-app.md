# plan-71-I: App — details, facts, kind names, the drawer, the `transceiver-app` harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-H

I makes the app show line transceivers: their facts in the sidebar, their
kind names, their place in the drawer. It also checks end to end on the
real catalog that a placed pair carries serial bytes in play mode.

References:

- plan-71-G § 3.4 (`PartTransceiver`), plan-71-H § 3.1–3.2 (drawer paths,
  the generics).
- `schematical/schematical/Backend/PartCatalog.swift` (the `details` enum,
  `case opamp(OpampInfo)` at :806, the fill at :1710),
  `Backend/PartFacts.swift` (`kindName` at :377, the facts rows at :316),
  `Simulation/SimulationAvailability.swift`,
  `Preferences/ReferencePrefixes.swift`.
- `scripts/harness/timer-app.swift`, `scripts/harness/build.sh` (the
  harness precedent); `planning/completed/plan-48-H-timer-opamp-app.md`.

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-H complete | `ls planning/completed/plan-71-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |
| The generics are in the catalog | `grep -c '^GENERIC-CAN\|\|^GENERIC-RS485\|\|^GENERIC-RS232\|\|^GENERIC-LIN\|' backend/tests/golden/parts.txt` → 4 | NOT MET (H) |
| The app does not know the kind yet | `grep -c 'PART_TRANSCEIVER' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift` → 0 and 0 | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog, document and `SimulationController`
(`scripts/harness/transceiver-app.swift`, every line `ok`):

- The drawer has **Interface → Transceiver** with the sub-groups `CAN`,
  `LIN`, `RS-232`, `RS-485`. Each has `Generic` among its functions.
- `GENERIC-CAN` placed: reference `U1`, kind name "CAN transceiver", a
  block symbol with its eight pins.
- Kind names:
  - "RS-485 transceiver" (`GENERIC-RS485`), "RS-232 transceiver",
    "LIN transceiver";
  - a part with drivers only: "RS-485 driver";
  - receivers only: "RS-485 receiver".
- Facts rows (§ 3): Protocol, Drivers, Receivers, Duplex, Data rate,
  Delays, Fail-safe (RS-485 only), Isolated (when true).
- A placed transceiver leaves Run on (`SimulationAvailability.canStart`).
- Play mode: two `MCP2200-I/SO` bridges joined through two `GENERIC-CAN`
  (CANH to CANH, CANL to CANL; supplies 5V and GND; S on GND). Bytes sent
  to bridge 1 through the simulation's serial terminal arrive at bridge 2's
  terminal. The same holds through two `GENERIC-RS485`, two
  `GENERIC-RS232` and two `GENERIC-LIN` (VBAT on 12V, RXD through a pull-up).

### Non-goals (explicit constraints)

- No new symbol: transceivers draw as blocks (`PartSymbol.block`).
- No new reference category: they are integrated circuits (`U`).
- No bus view: no differential waveform, no line voltages.
- No change to the saved file format.

## 2. Current State

The catalog maps each kind's facts struct into `CatalogPart.details`
(`PartCatalog.swift:806`, `:1107`, `:1350`, `:1710` for the op-amp case).
`PartFacts.kindName` (`PartFacts.swift:377`) names each `PartKind`
(`case PART_OPAMP: return "Op-amp"` at :427). The drawer's groups come from
the devices' `"drawer"` paths (`PartCatalog.groups(matching:)`), so H's
paths need no drawer code. The serial terminal for a bridge is the
simulation's `circuit_serial_send` / `circuit_serial_sent` path that the
`usbuart` model already uses (plan-12).

## 3. The facts rows

| Row | Value |
|---|---|
| Protocol | "CAN", "RS-485 / RS-422", "RS-232", "LIN" |
| Drivers / Receivers | the counts |
| Duplex | "Half", "Full" or "—" (duplex 0) |
| Data rate | `dataRate` in kbit/s or Mbit/s ("1 Mbit/s", "250 kbit/s", "20 kbit/s") |
| Delays | "driver 50 ns, receiver 60 ns" (only the directions it has) |
| Fail-safe | RS-485: "Yes" / "No (idle bus reads unknown)" |
| Isolated | "Yes", only when true |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, kind names

- [ ] `PartCatalog.swift`: `TransceiverInfo` and `case transceiver(TransceiverInfo)`,
      added only if absent, after the last case; the accessor; filled from
      `part.transceiver`.
- [ ] `PartFacts.swift`: the § 3 rows; `kindName` for `PART_TRANSCEIVER`
      ("Transceiver" for the bare kind); the per-part name of § 1.
- [ ] `schematicalTests/TransceiverTests.swift` (the per-kind test class shape
      of `PldTests.swift`, `DisplayDriverTests.swift`): `testFacts` (the § 3
      rows of `GENERIC-CAN`, `GENERIC-RS485` and a drivers-only RS-422 part
      of the pick) and `testKindNames` (§ 1).

Acceptance: the app builds, and the facts and names are right on the real catalog.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p71-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/TransceiverTests` → `** TEST SUCCEEDED **` (est. 8 min: an app build).
  Result:
Commit:

### Phase 2 — The harness

- [ ] `scripts/harness/transceiver-app.swift` per § 1, the
      `timer-app.swift` shape. It prints one `ok` / `FAIL` line per check,
      then `all checks ok`, and renders `p71-blocks.png` (the four generics
      placed).

Acceptance: every § 1 check passes on the real app build.
  Check: `APP=/tmp/p71-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/transceiver-app.swift /tmp/p71-harness && /tmp/p71-harness/H.app/Contents/MacOS/harness` → `all checks ok` (est. 3 min after the app build).
  Result:
Commit:

## Corrections

(none yet)
