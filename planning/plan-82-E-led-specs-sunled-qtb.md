# plan-82-E: Indicator LEDs: SunLED and QT Brightek specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-82-D

E fills the SunLED and QTB rows of `specs.tsv` and draws their footprints (`LED-SL-`, `LED-QTB-`)
by plan-82-C § 3, with C's tools unchanged. SunLED prints one PDF per part (as Kingbright); QTB
prints series PDFs (as Broadcom, plan-82-D § 3).

References: plan-82-C, plan-82-D; plan-82-B (`datasheets/led/sunled/`, `datasheets/led/qtb/`).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-D complete | `ls planning/completed/plan-82-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25) |
| Footprint count at E's start | as plan-82-D | E records F3 |

## 1. Goal

- Every SunLED and QTB row of `specs.tsv` passes `specs-check.py`.
- `footprints-sunled.json` and `footprints-qtb.json` are merged; the golden lists their ids.
- After E, `python3 -I planning/plan-82-data/census/specs-check.py all` → every row of the four
  makers ok, every footprint id in `footprints.json`.

### Non-goals

As plan-82-D.

## 2. Current State

After D: Kingbright and Broadcom done. SunLED's PDFs (one per part, e.g. `XZMG53W-1.pdf`, 4 pages,
fetched at planning) and QTB's series PDFs are in place (B).

## 3. Design

plan-82-C § 3; for QTB's ordering tables, plan-82-D § 3.

## 4. Phases

### Phase 1: SunLED specs and footprints

- [ ] Fill SunLED rows; `footprints-sunled.json`; merge; golden; hand check.

Acceptance: `specs-check.py sunled` → `rows N ok N`; `ctest --test-dir /tmp/logic-p82 -R
footprint_check` passes — ~2 h.

Commit:

### Phase 2: QTB specs and footprints

- [ ] Fill QTB rows; `footprints-qtb.json`; merge; golden; hand check.

Acceptance: `specs-check.py qtb` → `rows N ok N`; `specs-check.py all` → `ok` for every row and
`footprints missing 0`; `ctest … -R footprint_check` passes — ~2 h.

Commit:

## 5. Validation Plan

E's gate is Phase 2's `specs-check.py all` plus the footprint test. The landing tier is J's.
