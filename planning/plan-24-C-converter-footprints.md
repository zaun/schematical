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
| plan-24-B complete | `ls planning/completed/plan-24-B-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- Python check: set of `add` footprint ids − `footprints.json` ids → empty.
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

- [ ] Draw the footprints into `footprints.json`, smallest pin counts first; regenerate `footprints.txt`.
- [ ] Coverage check of §1.

Acceptance: `ctest` T0 of T0; coverage empty; only added golden lines (seconds).
Commit: —

## Validation Plan

C's gate: Phase 1. Landing tier at the end of I.

## Decisions

- Exposed thermal pads are the unnumbered `centre` pad (the `VQFN-14` precedent).

## Corrections

## Summary

C is drawing work; `footprint_check` catches overlapping or malformed pads.
