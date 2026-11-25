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
| plan-48-C complete | `ls planning/completed/plan-48-C-*.md` → exists, or every C box ticked | MET (2026-11-24: every C box ticked) |
| The tools exist | `ls planning/plan-48-data/census/fp-check.py planning/plan-48-data/census/fp-load.sh` → two paths | MET (2026-11-24) |

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

- [x] TI: `10-VSON (3x3)`, `8-X1SON (3x3)` and every other TI `D` line of
      the final file that is no ball package: reuse check or draw; map rows. Reuse `VSON-10-DRC-J`, `WSON-8-DSG`; new `X1SON-9-DNX` (TI DNX0008A,
      exposed pad numbered 9), `USON-6-TI-NKK0006A`.
- [x] ST: `8-DFN (2x2)` (two DigiKey packages), `16-QFN (3x3)` (two): reuse
      check or draw; map rows. New: `DFN-8-2X2-ST`, `DFN-8-2X2-ST-NOEP`, `QFN-16-3X3-ST`, `QFN-16-3X3-ST-EP1.7`, `QFN-16-3X3-ST-EP0.7`
      (the TSV524 wettable-flank pattern shares `QFN-16-3X3-ST-EP1.7`, exactly on the 0.05 mm limit).
- [x] ADI: `8-DFN (3x3)`, `8-LFCSP-WD (3x3)`, `8-LFCSP-UD (2x2)`, `6-TDFN
      (3x3)`, `6-DFN (2x3)`, `16-LFCSP-WQ (3x3)`, `16-LFCSP (4x4)`,
      `10-LFCSP-UQ (1.6x1.3)`: reuse check (including plan-42's ids when
      present) or draw; map rows. New: `LFCSP-8-ADI-CP-8-6`, `LFCSP-9-ADI-CP-8-11` (exposed pad numbered 9),
      `LFCSP-10-ADI-CP-10-10`, `LFCSP-16-ADI-CP-16-27`, `LFCSP-17-ADI-CP-16-22` (pad numbered 17), `TDFN-6-MAXIM-T633`; reuse `DFN-8-ADI-DD8`,
      `DFN-10-ADI-DD`, `LFCSP-8-ADI-CP-8-11`, `LFCSP-16-ADI-CP-16-23`.

Acceptance: every non-ball `D` line is mapped and the objects load.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py D | grep -c 'DFN\|SON\|QFN\|LFCSP'` → 0, and `planning/plan-48-data/census/fp-load.sh planning/plan-48-data/footprints-C.json planning/plan-48-data/footprints-D.json` → only `extra footprint` / `unused footprint` lines for the new ids (est. 4 min).
  Result: `fp-check.py D` → `28 lines, 0 without a footprint` (no line printed, so `grep -c` → 0); `fp-load.sh` → `34 new footprints: load ok`. MET.
Commit: —

### Phase 2 — Ball packages

- [x] TI: `8-uSMD (1.41x1.44)` (LMC555), `8-DSBGA (1.9x0.9)`: reuse check
      against `DSBGA-8-YFP` / `DSBGA-8-YZP`, or draw; map rows. New: `DSBGA-8-TI-YPB0008` (LMC555, balls A1–C3 without B2), `DSBGA-8-YZD`
      (OPA2347; `dual`, pads 1–8 as the datasheet's pin figure numbers them: the part's pinout is numbered, so the first reading, a ball grid, was changed).
- [x] ADI: `4-WLP`, `5-WLCSP`, the `6-WLP` lines, `8-UCSP`, `8-WLCSP`,
      `9-WLCSP`, `9-WLP`, `14-WLCSP`: draw from each datasheet's outline
      (reuse where two share one outline); map rows. New: `WLCSP-5-ADI-CB-5-1`, `WLP-6-ADI-N60D1`, `UCSP-8-MAXIM-B9-5`, `WLCSP-8-ADI-CB-8-1`,
      `WLCSP-9-ADI-CB-9-3`, `WLP-9-ADI-N91G1`, `WLCSP-14-ADI-CB-14-2`, `LGA-6-MAXIM-Y61A1-1`; reuse `WLP-6-ADI-N60G1`.
- [x] `fp-check.py` gains the ball-name comparison of § 3.

Acceptance: every `D` line is mapped; every new object loads; ball names agree.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, no `ball names differ` line; `fp-load.sh footprints-C.json footprints-D.json` → only `extra footprint` / `unused footprint` lines for exactly the new ids (est. 4 min).
  Result: `28 lines, 0 without a footprint`, `0 spec rows whose pin designators differ from their footprint` (the comparison covers numbers as well as ball names; its first run found
  `ada4096-2_4096-4.pdf`'s numbered pad 17 and the OPA2347's numbered bumps: both fixed above); `fp-load.sh` → `34 new footprints: load ok`. MET.
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

1. **Counts.** `footprints-D.json`: 23 new footprints (`python3 -I -c "import json;print(len(json.load(open('planning/plan-48-data/footprints-D.json'))['footprints']))"` → 23); 28 `D` lines (planning: 26 with a picked row; the final pick of B has 28). The two files
   together: 34 (11 + 23), the number `fp-load.sh` reports.
2. **Numbered exposed pads are pins.** Where the part's pinout (B) names the exposed pad as the last pin (`9=EP`, `17=EP`), the footprint has that pin:
   `LFCSP-9-ADI-CP-8-11`, `LFCSP-17-ADI-CP-16-22`, `X1SON-9-DNX` (`slots` with `0`, `perSide` set by `fp-collect.py` for an odd pin count). Where the datasheet leaves the pad unnumbered
   (`WSON-8-DSG`, `QFN-16-3X3-ST-*`, `HTSSOP-20-TI-PWP0020W`), it is the footprint's `centre` pad and no pin.
3. **Estimated sizes.** `WLP-6-ADI-N60D1` (MAX40007: the datasheet has no outline drawing; pad 0.20 round by the library's other 0.35-pitch footprints), `LGA-6-MAXIM-Y61A1-1` and `UCSP-8-MAXIM-B9-5`
   (pads computed from the outline), `WLCSP-9-ADI-CB-9-3` (ball diameter read from an image) are the least firmly sourced; their `note`s say so.
4. **`fp-check.py`** now compares pin designators (numbers and ball names) of every spec row with its footprint's pads, for both letters (instead of a ball-names-only check).

## Summary

Small numbers, unusual outlines: 26 lines for 43 parts, almost every ADI
line a drawing of its own. The care is in ball naming and exposed pads.
