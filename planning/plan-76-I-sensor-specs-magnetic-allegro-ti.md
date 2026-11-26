# plan-76-I: Magnetic specs, Allegro and TI

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-H

I fills the `allegro` and `ti` rows of `specs-magnetic.tsv` (linear / compass ICs 554 and
solid-state switches 565: Allegro ≤ 291, TI ≤ 282; plan-76-A table) by plan-76-C § 3 and § 3
below. J does Diodes and Infineon and draws the magnetic footprints.

References:

- plan-76-C § 3; plan-76-L § 3.1 (`magnetic-field` in mT, `"switchType"`, `"axes"`).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-H complete | `ls planning/completed/plan-76-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `allegro` and `ti` row of `specs-magnetic.tsv` passes
  `specs-check76.py --family magnetic --maker <m>`.

### Non-goals (explicit constraints)

- No operate / release point, hysteresis, sensitivity or angle facts: the kind has no keys for
  them. A switch's BOP / BRP go to the row's `note` (mT); no key holds
  them.
- No footprint drawn here (J).

## 2. Current State

Parts seen at planning (the census pages' first 100 rows, read at planning and not kept):
TI 58 rows of 554 — `DRV5053`, `DRV5055`; TI 29 rows of 565 —
`DRV5011`, `DRV5012`, `DRV5023`, `DRV5032`, `DRV5033`, `TMAG5131`,
`TMAG5213` (TI's package letters `DBZ`, `LPG`, `DMR` in the part numbers). Allegro 17 rows of 554 — `A1304`, `A1308`, `A1324`, `A1325`, `A1326`, `A1391`–`A1395`,
`CT100`; Allegro 29 rows of 565 — `A1101`, `A1102`, `A1104`, `A1106`, `A1120`, `A1121`, `A1126`,
`A1203`, `A1220`, `A1230`, `A1260`, `A3212`–`A3214`,
`ATS128`, `ATS627` (gear-tooth), `CT8122`, `CT8132` (TMR switches). Magnetic packages (`show.py
raw/b565.json "Package / Case"`): SOT-23-3 392, SOT-23W 79, TO-92-3 short body 79, 3-SIP 67, 3-SSIP
59, 4-SMD flat leads 36.

## 3. Design

- `measures`: `magnetic-field`, plus `temperature` where reported.
- `output`: `switch` (a Hall or TMR switch: open-drain or push-pull digital output), `analog`
  (a linear Hall sensor), `pwm`, `sent`, `i2c`, `spi`.
- `extra`: `switchType=unipolar|omnipolar|bipolar|latch` for every `switch` part, from the
  datasheet's function title ("unipolar switch", "omnipolar", "bipolar switch", "latch");
  `axes=<1|2|3>` (a 3D Hall sensor: 3).
- `ranges`: for a linear or 3D sensor, the linear range in mT (`[-B, B]`; 1 mT = 10 G); for a
  switch, absent.
- A part-number suffix that selects sensitivity or operate point (`DRV5053`'s `CA`/`EA`/`OA`/
  `PA`/`RA`/`VA`, `DRV5055A1`…`A4`) keeps one device per package with a per-part `ranges` map
  (plan-76-F § 3.2).
- Allegro's gear-tooth (`ATS`) parts are magnetic switches whose `switchType` is the datasheet's
  output behavior; their `note` says "gear-tooth".

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Allegro

- [ ] Fill every `allegro` row and its pinout files and footprint ids.

Acceptance: every Allegro row passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family magnetic --maker allegro` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — TI

- [ ] Fill every `ti` row and its pinout files and footprint ids.

Acceptance: every TI row passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family magnetic --maker ti` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

## Validation Plan

- I's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **a switch has a `switchType`, not a range**: the hobby choice between a switch and a
  latch is the fact a designer needs, and operate points are thresholds, not a measuring range.
- Author: **sensitivity suffixes stay one device** with per-part ranges, as pressure ranges do.

## Corrections

## Summary

I fills the magnetic specs of Allegro and TI.
