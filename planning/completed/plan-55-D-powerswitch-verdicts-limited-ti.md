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

- `planning/completed/plan-55-C-powerswitch-verdicts-ideal-load.md` § 3 (the verdicts, the pin, enable,
  flag and fact rules, the `specs.tsv` columns), `planning/completed/plan-55-A-powerswitch-kind.md` § 3.1.
- `datasheets/skipped-parts.md`; `planning/plan-55-data/census/` (`verdicts.py`, `pinout-check.py`).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-C complete | `ls planning/completed/plan-55-C-*.md` → exists | MET (2026-11-23: archived at 735d4c8e) |

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
| Candidate rows after B | the TI `limited` rows with `state` `candidate` | `awk -F'\t' '$1=="limited" && $2=="ti" && $12=="candidate"' planning/plan-55-data/candidates.tsv \| wc -l` (recorded here when D starts: **328** candidate rows after B; 37 TI `limited` rows are `out`, `no datasheet`) |

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

- [x] Read every TI PDF of the family (`pdftotext -layout`): ordering tables, pin tables, current
      limit, input range; fill `specs.tsv` (C § 3.1, § 3.3). Parts with an enable of both
      polarities by part number are two rows. Observed: read in the readers' packets (plan-55-C Correction 2): `pinout-check.py --family limited --company ti --complete` → `199 rows checked; 0 pinout errors`.

Acceptance: every TI `limited` row has a spec row or a skip reason.
  Check: `python3 -I planning/plan-55-data/census/pinout-check.py --family limited --company ti` → `0 pinout errors` (est. 2 min).
Commit: cced7528

### Phase 2 — Verdicts, skips and the hand check

- [x] `verdicts.py --family limited --company ti` → every row `add` or `skip`; the skips in
      `datasheets/skipped-parts.md`. Observed: `limited: 299 add, 66 skip` (skips: no datasheet 37 (www.ti.com 27, rocelec.widen.net 7, mm.digikey.com 3), no pin table 6, external pass element 4, not in the ordering table 4, select logic 4, control 11), `rows without a verdict: 0`; `--verify-skipped --family limited --company ti` → `0 skip rows missing from skipped-parts.md`.
- [x] Hand-check five `add` rows (one single-channel USB switch, one dual, one quad, one active-low
      enable, one smart high-side switch) against their PDFs: name, pin order, enable polarity,
      `vin_max`, `ilimit`; record in § Corrections (Correction 1).

Acceptance: every TI row has a verdict, every skip is listed, the hand checks agree.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --family limited --company ti --report` → no row without a verdict; `… --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 2 min).
Commit: cced7528

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

1. **Hand checks (2026-11-23), all agree with the PDFs**: TPS2051B SOT-23-5 (`tps2042b.pdf` Table 5-1: OUT 1, GND 2, OC 3, EN 4 active high, IN 5; `ilimit` 1 A, `vin_max` 5.5); TPS2041B SOT-23-5 (the same table, EN active low); TPS2042B 8-SOIC dual (Table 5-2: GND 1, IN 2, EN1 3, EN2 4, OC2 5, OUT2 6, OUT1 7, OC1 8; ENx active low); TPS2054B 16-SOIC quad (Table 5-3/5-4: GND 1 and 5, IN1 2, EN1 3, EN2 4, IN2 6, EN3 7, EN4 8, OC4 9, OUT4 10, OUT3 11, OC3 12, OC2 13, OUT2 14, OUT1 15, OC1 16; IN1 feeds OUT1 and OUT2); TPS4H000A-Q1 20-HTSSOP smart high-side (`tps4h000-q1.pdf` Pin Functions version A: IN1-IN4 1-4 active high, ST1-ST4 5-8 open-drain per-channel flags, CL 9, GND 10, DIAG_EN 11, THER 12, NC 13 and 14, OUT4 15, OUT3 16, VS 17 and 18, OUT2 19, OUT1 20). The checks of D's Phase 1 and 2 ran in plan-55-C's reader packets, because a PDF serves one family and is read once.

## Summary

The largest single reading job of the plan: TI is more than half of the family. The risk is the
same as C's (a wrong name or polarity), caught by the checker and the hand checks.
