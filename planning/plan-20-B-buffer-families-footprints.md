# plan-20-B: Families and footprints for buffers and transceivers

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-20-A

B adds the logic family codes and the footprints plan-20's parts need. No
parts, no behavior.

Outcome of B: `kind_logic_family` accepts `FCT`, `ALVT`, `ALB`, `AXC`,
`LXC`; every footprint id `parts.tsv` names loads; `footprints.txt` grows by
the number of new ids; no other golden changes.

References: plan-20-A §3.4 (family folding), `planning/plan-20-data/footprints-needed.txt`
(the 45 DigiKey packages with no footprint, 2026-11-07),
`docs/parts-format.md` § Values, § Footprints; `planning/completed/plan-03-B-logic-footprints.md` §3 (pad rules).

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-A complete | `ls planning/completed/plan-20-A-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-A-buffer-data.md`, 15f4c73) |

## 1. Goal

- `library_check` → `ok   gate part with family <X>` for each of the five codes.
- `ctest --test-dir /tmp/logic-p20` → T0 of T0; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- `awk -F'\t' 'NR>1 && $11=="add"{print $7}' planning/plan-20-data/parts.tsv | sort -u` ⊆ footprint ids in `footprints.json` (python check → empty difference).

## 2. Design

### 2.1 Families

`LOGIC_FAMILIES` (`backend/src/library/kind_util.c:435-438`) appends, each
only if absent (plans 19–22 run in any order; `FCT` is shared — when `grep
-c '"FCT"' kind_util.c` → 1 already, that addition is moot, recorded with
the grep): `FCT`, `ALVT` (3.3 V ABT-class; ALVTH folds in), `ALB` (SN74ALB
2.5/3.3 V), `AXC` (AXC/AXCH translators), `LXC` (LXC/LXCH translators).
`docs/parts-format.md` § Values gets a line each.

### 2.2 Footprints

The packages of `footprints-needed.txt` that `parts.tsv` `add` rows use,
each drawn from its TI package drawing (the PDF's mechanical pages; package
code in the part name: GQL/ZQL 56-ball, ZRD/ZXY 54/20-ball MicroStar Junior,
GKE/ZKE 96-ball, DCT, DQE/DQM X1SON/X2SON, DSBGA 4-ball, RHL VQFN-24, RSV
UQFN-16, …). Ids follow the existing style (`TVSOP-56`, `PDIP-24`, `SOIC-8`,
`PDIP-8`, `SOP-8`, `TSSOP-64`, `TSSOP-80`, `BGA-56-MICROSTAR-JR`,
`BGA-54-MICROSTAR-JR`, `BGA-20-MICROSTAR-JR`, `BGA-96-MICROSTAR`,
`SM8-DCT`, `X1SON-8`, `X2SON-4`, `X2SON-6`, `UQFN-10`, `UQFN-12`,
`UQFN-16-RSV`, `VQFN-24-RHL`, `VSSOP-24`, `USON-6`, `NFBGA-83`,
`LQFP-100`, `TSOT23-8`, `CFP-24`, `CDIP-8`, `DSBGA-4`, `SOT23-THIN-16`).
Each id is added only if absent (another of plans 19–22 may have added
`PDIP-24`, `SM8-DCT`, `SOT23-THIN-16`, `UQFN-16-RSV`,
`BGA-20-MICROSTAR-JR`; then moot with the grep). DigiKey strings that are
an existing footprint under another name (`20-LCC`/`20-LCCC` → `LCCC-20`,
`8-DSBGA (1.9x0.9)` → `DSBGA-8-YZP`) need none.

## Phases

### Phase 1 — Families

- [x] §2.1 codes `FCT`, `AXC`, `LXC`, `TXU`, `TXV` in `LOGIC_FAMILIES`, docs § Values, one `library_check` case each: `ctest --test-dir /tmp/logic-p20 -R library_check` → `100% tests passed`; `library_check` prints `ok   gate part with family FCT` … `TXV`. ~~`ALVT`, `ALB`~~ — moot: no `add` row uses them (`awk -F'\t' 'NR>1 && $11=="add"{print $3}' parts.tsv | grep -cE '^(ALVT|ALB)$'` → `0`; the ALVTH and ALB16244 parts were skipped, plan-20-A Corrections 3). `FCT` was not yet present (`grep -c '"FCT"' kind_util.c` → `0` before the edit).

Acceptance: `ctest --test-dir /tmp/logic-p20 -R library_check` passes (seconds).
Commit: —

### Phase 2 — Footprints

- [ ] §2.2 footprints into `schematical/schematical/Parts/footprints.json`; regenerate `backend/tests/golden/footprints.txt`.
- [ ] Coverage check of §1 (python) → empty.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 of T0; only added lines in `footprints.txt` (seconds).
Commit: —

## Validation Plan

B's gate: Phase 2. Landing tier at the end of I.

## Decisions

- New family codes rather than folding ALVT/ALB/AXC/LXC into neighbors, so
  the drawer filter tells them apart (plan-10-A precedent).

## Corrections

1. **Families**: plan-20-A's parts use `TXU` and `TXV` (TI's fixed-direction translators, their own family in the datasheets) and not `ALVT` or `ALB`; the list is `FCT`, `AXC`, `LXC`, `TXU`, `TXV` (measured from `parts.tsv`'s `family` column).

## Summary

B prepares the families and footprints; no behavior changes.
