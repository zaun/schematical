# plan-82-D: Indicator LEDs: Broadcom specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-82-C

D fills the Broadcom rows of `specs.tsv` and draws Broadcom's footprints (`LED-AV-` ids), by the
rules of plan-82-C § 3, with C's tools unchanged. Broadcom (the former Avago / HP optoelectronics
line: HSMx chip LEDs, HLMP through-hole lamps, ASMT PLCC) prints series datasheets: one PDF covers
several part numbers and packages, so D's rows are (datasheet, package option).

References: plan-82-C (§ 3 rules, `fp-match.py`, `fp-merge.py`, `specs-check.py`); plan-82-B
(`specs.tsv`, `datasheets/led/broadcom/`).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-C complete | `ls planning/completed/plan-82-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25) |
| Footprint count at D's start | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` | D records F2 |

## 1. Goal

- Every Broadcom row of `specs.tsv` passes `specs-check.py broadcom`.
- `footprints-broadcom.json` is merged; the footprints golden lists its new ids.

### Non-goals

No change to `Parts/led.json`, `backend/` or the app; no change to C's tools or rules (a case the
rules do not cover is a C correction, recorded in C's Corrections and applied to every maker).

## 2. Current State

After C: Kingbright's footprints and specs exist. Broadcom's PDFs are in `datasheets/led/broadcom/`
(B). A series PDF names each part's colour and lens by an ordering-code table; D reads the table to
map each `add` part to its package and dies.

## 3. Design

As plan-82-C § 3. Broadcom-specific: the ordering-code table decides each part's colour and lens
(`specs.tsv` `parts` lists the part numbers of the row); a PLCC-4 with one die and paralleled pins
lists the duplicate pins under one name (`1:K, 2:A, 3:A, 4:K`), so the die binds both (`led` F's
rule: a name may name several pins of one node).

## 4. Phases

### Phase 1: specs

- [ ] Fill the Broadcom rows.

Acceptance: `python3 -I planning/plan-82-data/census/specs-check.py broadcom` → `rows N ok N` —
~2 h of reading.

Commit:

### Phase 2: footprints

- [ ] `footprints-broadcom.json`, `fp-merge.py`, `footprint_check --print` into the golden, hand
      check.

Acceptance: `ctest --test-dir /tmp/logic-p82 -R footprint_check` passes; `wc -l <
backend/tests/golden/footprints.txt` = F2 + the new id count — 2 min.

Commit:

## 5. Validation Plan

D's gate is Phase 2's footprint test. The landing tier is J's.
