# plan-76-J: Magnetic specs, Diodes and Infineon; the magnetic footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-I

J fills the `diodes` and `infineon` rows of `specs-magnetic.tsv` (Diodes ≤ 225, Infineon ≤ 127;
plan-76-A table) by plan-76-C § 3 and plan-76-I § 3, then draws and merges every new magnetic
footprint (all four makers).

References:

- plan-76-C § 3, plan-76-D § 3.2, plan-76-I § 3.
- Existing candidates for reuse (`footprints.json`, 2026-11-25): 31 `SOT23*`, 12 `SC70*`,
  5 `TO-92-3-*` (`TO-92-3-ONSEMI`, `-FORMED`, `TO-92-3-LP`, …), 3 `SIP-*-2.54-VISHAY` (6, 8, 10
  pins), 4 `SON-*`.

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-I complete | `ls planning/completed/plan-76-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-25: I not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `specs-magnetic.tsv` passes `specs-check76.py --family magnetic`.
- Every new magnetic footprint id is drawn in `footprints-magnetic.json`, merged, and its golden
  line added; no existing footprint line changes.

### Non-goals (explicit constraints)

- The facts limits of plan-76-I § Non-goals.

## 2. Current State

Parts seen at planning (`b565`'s first 100 rows): Diodes 11 rows — `AH1806`, `AH1808`, `AH1815`,
`AH1911`, `AH1912`, `AH1913`, `AH1921`, `AH1925`, `AH3322`, `AH3323A`, `AH3522` (package suffixes
`-W`, `-P`, `-SA`, `-HK4`); Infineon 6 rows — `TLE4906`, `TLE4913`, `TLE4946`, `TLE4964`,
`TLI4963`; Infineon in 554 includes the `TLE493D` 3D sensor (the probe's
`infineon-tle493d-w2b6-datasheet-cn.pdf`).

## 3. Design

- Specs: plan-76-I § 3.
- Footprints: plan-76-D § 3.2. Through-hole in-line packages (DigiKey's 3-SIP, 3-SSIP and
  TO-92-3 short body: 67 + 59 + 79 board-mount rows of 565) are `explicit` footprints from the lead pitch and diameter of each drawing;
  an existing `TO-92-3-*` id is reused only when its pads and pitch match within 0.05 mm.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Diodes and Infineon specs

- [ ] Fill every `diodes` and `infineon` row, its pinout files and footprint ids.

Acceptance: the whole magnetic specs file passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family magnetic` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — Magnetic footprints

- [ ] Draw `footprints-magnetic.json`; merge; regenerate `footprints.txt`; hand-check each added
      line.

Acceptance: every magnetic footprint id exists, and the golden only grew.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check magnetic` → `0 missing`; `ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- J's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **all magnetic footprints in one phase**, after all four makers' specs: the SOT-23 and
  3-lead in-line outlines repeat across makers, and one pass reuses ids across them.

## Corrections

## Summary

J finishes the magnetic specs and lands every magnetic footprint.
