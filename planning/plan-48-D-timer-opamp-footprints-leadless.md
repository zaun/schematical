# plan-48-D: Footprints — leadless and ball packages (DFN, SON, QFN, LFCSP, LGA, WLP / WLCSP / BGA)

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-C

D gives every leadless and ball package line of the final pick a footprint,
by the rules of plan-48-C § 3.1, in
`planning/plan-48-data/footprints-D.json` and `footprint-map.tsv`.

References:

- plan-48-C § 3.1 (ids, reuse, shared ids, ready objects, the map), § 3.2
  (the tools).
- `docs/parts-format.md` § Footprints (`dual` with `centre`, `quad`, `grid`,
  `explicit`; rounded and cut corners).
- `planning/plan-42-F-regulator-footprints-leadless-adi.md` (ADI LFCSP and
  DFN precedents).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-C complete | `ls planning/completed/plan-48-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-16: not started) |
| The tools exist | `ls planning/plan-48-data/census/fp-check.py planning/plan-48-data/census/fp-load.sh` → two paths | NOT MET (C Phase 1 writes them) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `D` line of `footprints-needed.txt` has a `footprint-map.tsv` row
  naming a footprint in `footprints-D.json` or `Parts/footprints.json`:
  `python3 -I planning/plan-48-data/census/fp-check.py D` → `<n> lines, 0
  without a footprint`.
- `footprints-D.json` loads through `footprint_check` with no error.

At planning time: 26 `D` lines carry a picked row (43 picked parts): ADI 18
lines, ST 4, TI 4; 3 of them have no existing footprint of the family and
pin count (`14-WLCSP (1.128x2.123)`, `9-WLCSP (1.21x1.22)`, `9-WLP
(1.23x1.23)`).

### Non-goals (explicit constraints)

- No existing footprint changes; `Parts/footprints.json` and the goldens are
  not touched here (G).
- No footprint for a package no `add` row uses.

## 2. Current State

The reuse checks start from (the `existing` column): `VSON-10-DRC`,
`WSON-10-DSC`; `SON-8-DRB`, `WSON-8-DSG`, `X1SON-8`; `SON-6-DRY`,
`WSON-6-NGF`; `QFN-16-RSA`, `WQFN-16-RTE`, `QFN-16-3X3-XR`, `UQFN-10`;
`DSBGA-4-YFP`, `DSBGA-5-YZP`, `DSBGA-6-YFP`, `DSBGA-6-YZP`, `DSBGA-8-YFP`,
`DSBGA-8-YZP`. The library has no ADI or ST leadless footprint for an
op-amp package yet; plan-42-F (open) names `LFCSP-6-ADI-CP-6-3`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `D` lines, download set / with a picked row | 35 / 26 (43 picked parts) | `awk -F'\t' '!/^#/ && $1=="D"{n++; if($8>0){c++; p+=$8}} END{print n,c,p}' planning/plan-48-data/footprints-needed.txt` |
| …by maker | ADI 18, ST 4, TI 4 | `awk -F'\t' '!/^#/ && $1=="D" && $8>0{n[$2]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/footprints-needed.txt` |
| …no existing footprint of the family and pin count | 3 | `awk -F'\t' '!/^#/ && $1=="D" && $8>0 && $12==""' planning/plan-48-data/footprints-needed.txt \| wc -l` |
| …ball packages (WLP, WLCSP, UCSP, µSMD, DSBGA) | 12 lines, 13 picked parts | `awk -F'\t' '!/^#/ && $1=="D" && $8>0 && $5 ~ /WLP\|WLCSP\|UCSP\|uSMD\|DSBGA/{n++; p+=$8} END{print n,p}' planning/plan-48-data/footprints-needed.txt` |

## 3. Design

plan-48-C § 3.1 applies. In addition:

- **Styles.** Two-side packages (DFN, SON, TDFN, 2-side LFCSP): `dual` with
  `centre` for the exposed pad. Four-side packages (QFN, 4-side LFCSP,
  `10-LFCSP-UQ`): `quad`. Ball packages: `grid` with the datasheet's ball
  names; a package whose balls are not on one regular grid (the staggered
  `5-WLCSP`) is `explicit` with `"ball"` names.
- **Ball pinouts.** The part's pinout (B's `specs.tsv`) uses the same ball
  names as the footprint; `fp-check.py` compares the two name sets for
  every ball line.
- **Ids.** ADI: `<outline>-<pins>-ADI-<package code>` (`LFCSP-8-ADI-CP-8-13`,
  `WLCSP-9-ADI-CB-9-2`); Maxim drawings by their outline number
  (`UCSP-8-MAXIM-B9-1`, `TDFN-6-MAXIM-T633`); ST: `<outline>-<pins>-ST`
  with the body size when two differ (`DFN-8-2X2-ST`, `QFN-16-3X3-ST`); TI
  by designator (`VSON-10-DRC` exists; `DSBGA-8-YZR`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — DFN, SON and QFN (TI, ST, ADI)

- [ ] TI: `10-VSON (3x3)`, `8-X1SON (3x3)` and every other TI `D` line of
      the final file that is no ball package: reuse check or draw; map rows.
- [ ] ST: `8-DFN (2x2)` (two DigiKey packages), `16-QFN (3x3)` (two): reuse
      check or draw; map rows.
- [ ] ADI: `8-DFN (3x3)`, `8-LFCSP-WD (3x3)`, `8-LFCSP-UD (2x2)`, `6-TDFN
      (3x3)`, `6-DFN (2x3)`, `16-LFCSP-WQ (3x3)`, `16-LFCSP (4x4)`,
      `10-LFCSP-UQ (1.6x1.3)`: reuse check (including plan-42's ids when
      present) or draw; map rows.

Acceptance: every non-ball `D` line is mapped and the objects load.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py D | grep -c 'DFN\|SON\|QFN\|LFCSP'` → 0, and `planning/plan-48-data/census/fp-load.sh planning/plan-48-data/footprints-C.json planning/plan-48-data/footprints-D.json` → only `extra footprint` / `unused footprint` lines for the new ids (est. 4 min).
Commit: —

### Phase 2 — Ball packages

- [ ] TI: `8-uSMD (1.41x1.44)` (LMC555), `8-DSBGA (1.9x0.9)`: reuse check
      against `DSBGA-8-YFP` / `DSBGA-8-YZP`, or draw; map rows.
- [ ] ADI: `4-WLP`, `5-WLCSP`, the `6-WLP` lines, `8-UCSP`, `8-WLCSP`,
      `9-WLCSP`, `9-WLP`, `14-WLCSP`: draw from each datasheet's outline
      (reuse where two share one outline); map rows.
- [ ] `fp-check.py` gains the ball-name comparison of § 3.

Acceptance: every `D` line is mapped; every new object loads; ball names agree.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, no `ball names differ` line; `fp-load.sh footprints-C.json footprints-D.json` → only `extra footprint` / `unused footprint` lines for exactly the new ids (est. 4 min).
Commit: —

## Validation Plan

- Tests: `fp-check.py`, `fp-load.sh` on the ready objects.
- Coverage check: the object count of `footprints-D.json` is recorded in
  § Corrections.
- Doc sync: none.
- Final gate: plan-48-I.

## Decisions

- Author: **ball packages are in**: the pick is by stock, and a part is not
  dropped for its package (13 picked parts at planning time).
- Author: **one footprint per distinct outline**, never one per part: two
  WLP lines with the same ball grid and pad size share a footprint.

## Corrections

## Summary

Small numbers, unusual outlines: 26 lines for 43 parts, almost every ADI
line a drawing of its own. The care is in ball naming and exposed pads.
