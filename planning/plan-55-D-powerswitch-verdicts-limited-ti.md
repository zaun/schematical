# plan-55-D: Verdicts — current-limited and USB switches, Texas Instruments

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-C

D gives every Texas Instruments candidate row of the `limited` family (366 at most, plan-55-A § 2:
TI's DigiKey labels 296 and 14) a verdict, with its pinout, switches and facts, by the rules of
plan-55-C § 3. TI's current-limited switches include one- to four-channel parts (the TPS204x /
TPS205x family: `EN` high or low by part number, `OC` per channel, shared `IN`), USB power
switches and smart high-side switches with a fault flag. The behavioral outcome: every TI `limited`
row has a row in `verdicts.tsv`; every `add` row names a `specs.tsv` row that passes
`pinout-check.py`; every `skip` row is in `datasheets/skipped-parts.md`. No part file, footprint,
backend or app file changes.

References:

- `planning/plan-55-C-powerswitch-verdicts-ideal-load.md` § 3 (the verdicts, the pin, enable,
  flag and fact rules, the `specs.tsv` columns), `planning/plan-55-A-powerswitch-kind.md` § 3.1.
- `datasheets/skipped-parts.md`; `planning/plan-55-data/census/` (`verdicts.py`, `pinout-check.py`).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-C complete | `ls planning/completed/plan-55-C-*.md` → exists | NOT MET (2026-11-22: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every TI row of the family `limited` in `verdicts.tsv` has `verdict` `add` or `skip` and a
  `reason` of plan-55-C § 3.2.
- `python3 -I planning/plan-55-data/census/pinout-check.py --family limited --company ti` → `0 pinout errors`.
- `datasheets/skipped-parts.md`'s `## Power switches (plan-55)` section lists every TI `limited`
  `skip` row.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new skip reason and no rule change: a part C's rules cannot place is a `skip` of C § 3.2.

## 2. Current State

C's scripts and rules; B's `specs.tsv` skeleton and PDFs for the TI `limited` rows.

### Measured populations

| What | Count | Command |
|---|---|---|
| TI current-limited candidates at most | 366 (label 296: 363, label 14: 3) | `grep '^pick.*limited.*Texas' planning/plan-55-data/families.out` → `parts 366` |
| TI `Package / Case` labels, family `limited` | 37 | same line, `packages(case) 37` |
| Candidate rows after B | the TI `limited` rows with `state` `candidate` | `awk -F'\t' '$1=="limited" && $2=="ti" && $12=="candidate"' planning/plan-55-data/candidates.tsv \| wc -l` (recorded here when D starts) |

### Verified properties

- **TPS2051B's pin table** (SLVS514, read 2026-11-22): SOT-23-5 OUT 1, GND 2, OC 3, EN 4, IN 5; SOIC-8
  IN 2 and 3, OUT 6, 7 and 8; `EN` active high (the TPS2041B: low); `OC` open-drain active low. The
  TPS2052B has EN1 / EN2, OUT1 / OUT2, OC1 / OC2 and one `IN`; the TPS2053B / TPS2054B have IN1 / IN2
  (IN1 serves OUT1 and OUT2). Each is a `limited` device of 1 to 4 switches in the kind
  (plan-55-A § 3.1).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs for the TI `limited` series

- [ ] Read every TI PDF of the family (`pdftotext -layout`): ordering tables, pin tables, current
      limit, input range; fill `specs.tsv` (C § 3.1, § 3.3). Parts with an enable of both
      polarities by part number are two rows.

Acceptance: every TI `limited` row has a spec row or a skip reason.
  Check: `python3 -I planning/plan-55-data/census/pinout-check.py --family limited --company ti` → `0 pinout errors` (est. 2 min).
Commit: —

### Phase 2 — Verdicts, skips and the hand check

- [ ] `verdicts.py --family limited --company ti` → every row `add` or `skip`; the skips in
      `datasheets/skipped-parts.md`.
- [ ] Hand-check five `add` rows (one single-channel USB switch, one dual, one quad, one active-low
      enable, one smart high-side switch) against their PDFs: name, pin order, enable polarity,
      `vin_max`, `ilimit`; record in § Corrections.

Acceptance: every TI row has a verdict, every skip is listed, the hand checks agree.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --family limited --company ti --report` → no row without a verdict; `… --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 2 min).
Commit: —

## Validation Plan

- Tests: the scripts' output is the check; no backend or app test is touched.
- Coverage check: `verdicts.py --report` counts every TI `limited` row once.
- Runtime proof: none (data only).
- Doc sync: `datasheets/skipped-parts.md`.
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): scope, catalog rule, naming.
- Author: **smart high-side switches with a plain enable pin are in** (the kind's model is an enable
  pin and a fault flag); one with an SPI or I²C control is `skip`, reason `control`.

## Corrections

(none yet)

## Summary

The largest single reading job of the plan: TI is more than half of the family. The risk is the
same as C's (a wrong name or polarity), caught by the checker and the hand checks.
