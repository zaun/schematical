# plan-76-K: Current: specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-J

K fills `specs-current.tsv` (Allegro ≤ 236, LEM ≤ 166, TI ≤ 108, Melexis ≤ 69; plan-76-A table) by
plan-76-C § 3 and § 3 below, then draws and merges the current footprints. After K every family's
specs and footprints are done, and L (the kind) and M (the parts) can follow.

References:

- plan-76-C § 3, plan-76-D § 3.2, plan-76-F § 3.2 (per-part ranges); plan-76-L § 3.1 (`current`
  in A, `"isolated"`).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-J complete | `ls planning/completed/plan-76-J-*.md` → exists, or every J box ticked | NOT MET (2026-11-25: J not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `specs-current.tsv` passes `specs-check76.py --family current`.
- Every new current footprint id is drawn in `footprints-current.json`, merged, and its golden
  line added; no existing footprint line changes.

### Non-goals (explicit constraints)

- No primary-conductor or busbar geometry: a LEM transducer through which a cable or busbar
  passes (DigiKey's "Module, Single Pass Through", 89 board-mount rows) has only its PCB pins as
  pads; the aperture is outside the footprint.
- No bandwidth, sensitivity or creepage facts (no keys).

## 2. Current State

Parts seen at planning (`b525`'s first 100 rows): Allegro 51 rows — `ACS37002`, `ACS37003`,
`ACS37010`, `ACS37030`, `ACS37032`, `ACS37041`, `ACS37042`, `ACS37220`, … (range-coded suffixes
such as `-030B3`: 30 A, 3.3 V); LEM 31 rows — `CAS`, `CASR`, `CKSR`, `CTSR`, `DHAB`, `FHS`, `GO …
SME`, `HAS`, `HC5FW`, `HO`, `HTFS`, `LA`, `LAH`, `LESR`; TI 6 rows — `TMCS1100`, `TMCS1101`,
`TMCS1108`, `TMCS1123`; Melexis 3 rows — `MLX91208`, `MLX91221`. Current packages (`show.py
raw/b525.json "Package / Case"`): 8-SOIC 232, Module 151, 16-SOIC wide 116, Module single pass
through 89, 10-SOIC wide 57, 16-PowerSOIC 10-lead 50.

## 3. Design

- `measures`: `current`, plus `temperature` where reported.
- `output`: `analog` (most), `i2c`, `spi`, `pwm`, `switch` (an overcurrent flag only part).
- `ranges`: the primary nominal measuring range in A, symmetric `[-I, I]` for bidirectional parts,
  `[0, I]` for unidirectional; per part where a suffix codes it (Allegro `-030B3`, LEM `HO 6-P` /
  `HO 15-P`, TI `A1`–`A4` sensitivity codes).
- `extra`: `isolated=1` for Hall, fluxgate and TMR parts whose datasheet gives an isolation
  voltage; absent otherwise.
- `vcc`: the supply range. A LEM part with a bipolar supply (±12 V … ±15 V) records `[12, 15]` as
  the magnitude and its note says "±"; its `−V` pin is `power`, not ground.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs

- [ ] Fill every row of `specs-current.tsv`, its pinout files and footprint ids.

Acceptance: every current row passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family current` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — Footprints

- [ ] Draw `footprints-current.json`; merge; regenerate `footprints.txt`; hand-check each added
      line.

Acceptance: every current footprint id exists, and the golden only grew; every family's
footprints are now present.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check current` → `0 missing`; `for f in temperature humidity light pressure motion magnetic current; do python3 -I planning/plan-76-data/census/fp-merge76.py --check $f; done` → `0 missing` seven times; `ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- K's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **a bipolar-supply transducer's −V pin is `power`**: it is a supply the board feeds,
  and `ground` would let the ERC tie it to 0 V.
- Author: **`isolated` only where an isolation voltage is printed**, so the fact is the
  datasheet's, not inferred from the sensing method.

## Corrections

## Summary

K fills the current-sensor specs and lands their footprints; with it, every family is ready for
the kind and the parts.
