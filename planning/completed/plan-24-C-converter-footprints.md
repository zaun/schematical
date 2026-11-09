# plan-24-C: Footprints for ADCs, DACs and RTCs

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-B

C adds every footprint the plan-24 `add` rows name that does not exist.

Outcome of C: every footprint id in `parts.tsv` `add` rows is in
`footprints.json`; `footprints.txt` grows by exactly the number of new ids;
no other golden changes.

References: `planning/plan-24-data/footprints-needed.txt` (79 DigiKey
packages, 1001 rows, 2026-11-07 — before the status filter of A/B removed
some rows); `docs/parts-format.md` § Footprints (`dual`, `quad`, `grid`,
exposed pads as the unnumbered `centre` pad); `planning/completed/plan-03-B-logic-footprints.md` §3.

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-B complete | `ls planning/completed/plan-24-B-*.md` → exists | MET (`planning/completed/plan-24-B-dac-rtc-data.md`, 2026-11-07 at 69052a8) |

## 1. Goal

- Python check: set of `add` footprint ids − (`footprints.json` ids ∪ `planning/plan-24-data/footprints-new.json` ids) → empty, and every staged footprint is used by an `add` row (Corrections).
- The staged footprints load with the backend's `load_footprints` after `footprints.json` with no error, no overlapping pads, a pad for every pin (`/tmp/p24/fpcheck`), and each spec package's footprint has a pad per pinout designator and the pinout's pin count (`census/fpmatch.py`).
- `ctest --test-dir /tmp/logic-p24` → T0 of T0; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.

## 2. Design

- The ids `parts.tsv` names for the packages of `footprints-needed.txt`
  (largest: DGS VSSOP-10 122 rows, D SOIC-8 104, RTE WQFN-16 70, PFB
  TQFP-48 60, RHB VQFN-32 50, RGE VQFN-24 37, DB SSOP-28 35, RHB/VFP
  TQFP-32 35, DBT TSSOP-38 34, PAP HTQFP-64 24, PM LQFP-64 23, DRC VSON-10
  22, …), each from the TI package drawing in the part's PDF, ids in the
  existing style (`VSSOP-10`, `SOIC-8`, `WQFN-16-RTE`, `TQFP-48-7X7`,
  `VQFN-32-5X5` exists — reuse it where the drawing matches, …).
- An id another plan has already added (plans 19–22 add `SOIC-8`, `PDIP-8`,
  `SOP-8`, `SOT23-THIN-16`, …) is reused; its task line is moot with the
  `grep` showing it.

## Phases

> **Status discipline:** as plan-24-A.

### Phase 1 — Footprints

- [x] Draw the footprints (10 subagent batches against `/tmp/p24/fpbrief.md`, R1 board layouts where TI gives one, R2/R4/R5 from outlines otherwise) into `planning/plan-24-data/footprints-new.json` (Corrections): `fpcheck footprints.json footprints-new.json` → `85 staging footprints loaded, ok`. 77 of the 79 planned ids drawn, 2 reused (`PDIP-16-NFG` → `PDIP-16`, `VQFN-14-RGY` → `VQFN-14`), 2 with no TI drawing anywhere (`CDIP-SB-28-JD`, `WQFN-40-RSB`: their parts skip), 10 second drawings of one designator (`footprint-overrides.tsv`).
- [x] Coverage check of §1: `add` footprint ids 118, missing from footprints.json ∪ footprints-new.json 0, staged but unused 0; `census/fpmatch.py footprints-new.json` → `0 problems`; footprint pin count = pinout pin count for every spec package (0 mismatches); `census/coverage.py ADC/DAC/RTC` → 0 problems (1187 / 724 / 14 add rows).
- [x] Specs and `parts.tsv` follow the drawings: `census/build.py` reads `footprint-overrides.tsv` and skips the two undrawable packages (ADC3541, ADC3542, ADC3544, ADC3642 have only RSB parts, DAC811's JD part: 6 rows; `adc3541.pdf`, `adc3544.pdf`, `adc3643.pdf` deleted, `datasheets/not-included.md`); 58 spec package footprints updated (`/tmp/p24/scripts/syncfp.py`); 4 specs removed.

Acceptance: `ctest` T0 of T0 (`100% tests passed, 0 tests failed out of 24`); coverage empty (above); golden diff 0 lines — the `footprints.txt` lines are added in G/H (Corrections).
Commit: 79cabb9

## Validation Plan

C's gate: Phase 1. Landing tier at the end of I.

## Decisions

- Exposed thermal pads are the unnumbered `centre` pad (the `VQFN-14` precedent).

## Corrections

- **The footprints are staged, not put in `footprints.json` here.**
  `footprint_check` fails on any footprint no part uses ("unused
  footprint"), and the parts that use these arrive in G and H. Landing them
  in `footprints.json` now would turn `ctest` red until H, so C's "T0 of
  T0" could not hold. They go to `planning/plan-24-data/footprints-new.json`
  instead, checked with the real loader (`/tmp/p24/fpcheck`: load, golden
  line, overlap, pin pads) and against every spec pinout; G moves the ones
  its ADC parts use into `footprints.json` and regenerates
  `footprints.txt`, H the rest (tasks added there). Every check still runs,
  at the commit where it can pass.
- **One TI designator, several drawings.** RGE (B, H, F), RTE (C, D), RHA
  (C, H), RGZ (A, D), RGC (A, H), RTC (sheets -2, -3), DRB (A, B), DRC
  (D, J) and DSC (TI vs National SDA10A) have different land patterns in
  different datasheets. The id per designator stays the default; a
  per-datasheet override (`footprint-overrides.tsv`, read by
  `census/build.py`) names the second drawing's footprint (`-H`, `-D`,
  `-B`, …). Same-pad, different-thermal-pad revisions of PHP and PAP share
  one footprint (noted in each footprint's note).
- **No drawing**: `CDIP-SB-28-JD` (DAC811BH) and `WQFN-40-RSB` (ADC3541,
  ADC3542, ADC3544, ADC3641/ADC3642) appear in no datasheet's mechanical
  pages; their 6 parts are skipped (AGENTS.md: a footprint comes from the
  package drawing).
- `CFP-84-HFQ` takes its lead foot from the same package's formed variant
  HKP (`ads1278-ht.pdf` p.57): HFQ ships on a carrier frame with no lead
  length or board layout.

## Summary

C is drawing work; `footprint_check` catches overlapping or malformed pads.
