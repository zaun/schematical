# plan-51-C: IC sockets — the drawings: land-pattern facts per series

Last updated: 2026-11-21
Effort: large (3h–1d)
Depends on: plan-51-B

C reads the maker drawings B downloaded and writes **what a footprint generator
needs**, one row per socket group, so D and E write footprints by running a
script and never read a PDF again. C also decides which candidates cannot be
added (no drawing, an odd pattern) and records them.

Outcome of C: `planning/plan-51-data/series.tsv` has one row per
`(maker, series, family, mounting, spacing)` group of the candidates, every cell
filled from a named drawing page; `candidates.tsv` column 12 (`state`) is final
(`add` or `out` with a reason); every `out` row of a drawing reason is in
`datasheets/skipped-parts.md`; `python3 -I planning/plan-51-data/census/series_check.py`
prints `ok`.

References:

- `docs/parts-format.md` § Footprints (styles `dual`, `quad`; § The `"footprint"`
  part key: mid-values, notes); `planning/completed/plan-03-B-logic-footprints.md`
  § Pad rules R1–R6 (read in full: C applies R1, R2, R3 and R5 below).
- `datasheets/skipped-parts.md` (the format of an entry).
- plan-51-B outputs: `datasheets/ic-socket/<maker>/*.pdf`, `candidates.tsv`.
- The existing chip footprints these sockets hold: `planning/plan-51-data/fits.tsv`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-B complete | `ls planning/completed/plan-51-B-*.md` → exists | NOT MET (2026-11-21: not started) |
| Every candidate row has a downloaded PDF | `awk -F'\t' 'NR>1 && $12=="candidate" && $11==""' planning/plan-51-data/candidates.tsv \| wc -l` → 0 | NOT MET (B's output) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `series.tsv` covers every group of the `add` rows (53 at planning time: `python3 -I planning/plan-51-data/census/tally.py` → `groups: 53`; B may add groups).
- `series_check.py` prints `ok`.

### Non-goals (explicit constraints)

- No footprint is written (D, E). No part is written (F).
- The rules are plan-03-B's, applied to sockets as § 3 says; no new geometry rule.
- A shape outside `fits.tsv` stays out (plan-51-A decisions).

## 2. Current State

- The existing chip footprints use `dual` for DIP (`PDIP-14`: pitch 2.54, span 7.62,
  drill 0.75, round pad 1.5, `pin1Shape` `rect`) and `quad` with `slots` for PLCC
  (`PLCC-44`: sides `[11,11,11,11]`, pitch 1.27, pin 1 top centre, `slots` listing
  the pin on each slot counter-clockwise from the first slot of the left side;
  `PLCC-32`: sides `[9,7,9,7]`). A socket footprint uses the same styles and the
  same pin numbering, so a chip's pin *n* lands on the socket's pad *n*.
- The footprint formats: `docs/parts-format.md` § Footprints (`dual`, `quad`,
  `explicit`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Groups at planning time | 53 | `python3 -I planning/plan-51-data/census/tally.py` → `groups: 53` (Preci-Dip PLCC not yet listed) |
| PLCC candidates at planning time | 11 Assmann (28/32/44 TH and SM, 68 TH), 9 Mill-Max (540 SM, 940 TH and SM) | `awk -F'\t' '$4=="PLCC"' planning/plan-51-data/candidates.tsv` |
| Mill-Max 110 through-hole DIP rows | 106, covering all 11 DIP shapes (8, 14, 16, 20, 24, 28 at 0.3 in; 24, 28, 32, 40, 48 at 0.6 in) | `awk -F'\t' '$1=="Mill-Max" && $3=="110" && $4=="DIP" && $7=="Through Hole"' planning/plan-51-data/candidates.tsv \| wc -l` → 106 |
| Assmann A-CCS through-hole PLCC rows | 8, covering 28, 32, 44, 68 | the PLCC list above |

## 3. Design

### 3.1 The pad rules for sockets

| Mounting | Rule |
|---|---|
| through hole, drawing gives the recommended PC-board hole | R1: `drill` = that hole (mid-value), round pad = drill + 0.7 (at least 1.5), `pin1Shape` `rect`, `span` = row spacing 5.08 / 7.62 / 10.16 / 15.24 |
| through hole, drawing gives only the tail | R5: drill = largest tail cross-section + 0.25, rounded up to 0.05, pad as above |
| surface mount, drawing gives a land pattern | R1: its pad size, pitch and span |
| surface mount, gull-wing tails (DIP) | R2 from the drawing's tip-to-tip span and tail length |
| surface mount, J-lead (PLCC) | R3 |

Body: the socket housing's outline (mid-values), rounded to 0.01. DIP length of
an N-pin socket is read from the drawing's table when it has one, else
`N/2 × 2.54 + c` with `c` read from one drawing row.

### 3.2 `series.tsv`

Columns: `maker`, `series`, `family`, `mounting`, `spacing`, `pins` (the candidates'
pin counts, comma-separated), `rule` (`R1`, `R2`, `R3`, `R5`), `drill`, `pad_w`,
`pad_h` (`pad_w` = diameter for a round pad), `span` (row spacing for DIP;
`spanX`/`spanY` for PLCC as `x/y` per pin count), `pitch`, `sides` (PLCC, per pin
count), `body_w`, `body_len` (`N:len;N:len` per pin count), `pin1` (`rect` or the
chamfer/corner the drawing marks), `drawing` (PDF file and page).

### 3.3 Generic sockets

The generic sockets of F reuse footprints: a generic DIP socket uses the footprint
of the Mill-Max 110 through-hole row of its shape (all 11 shapes are covered), a
generic PLCC socket the Assmann A-CCS through-hole footprint of its size (all 4).
C marks those groups `generic` in a column of `series.tsv`.

### Risk

Reading drawings is the whole task; the check script bounds it (every cell
filled, every group present, rule names valid, `drawing` cites an existing file).
Image-only drawings are read from the rendered page (`pdftoppm -r 150 -f <p> -l <p>`).

## 4. Phases

### Phase 1 — the tables

- [ ] `planning/plan-51-data/census/series_check.py`: reads `candidates.tsv` and `series.tsv`; fails on a group with no row, an empty required cell, an unknown `rule`, a `pins` entry outside `fits.tsv`, a `drawing` file that does not exist, or a `generic` group set that does not cover all 11 DIP and 4 PLCC shapes; prints `ok`.
- [ ] Mill-Max rows (series of `candidates.tsv` column 3 for maker Mill-Max) from `088-145.pdf`.
- [ ] Preci-Dip rows from `CATALOG-DILSOCKETS.pdf` and the PDFs B downloaded.
- [ ] Assmann rows from the `ASS_*_CO.pdf` files.

Acceptance: `series_check.py` prints `ok`.
  Check: `python3 -I planning/plan-51-data/census/series_check.py` → `ok` (est. 4 h of reading, 1 s to check).
Commit: —

### Phase 2 — verdicts and skipped parts

- [ ] A candidate group whose drawing gives no land pattern and no tail size, or whose pattern the `dual` and `quad` styles cannot draw, is `out` (`state` column), `reason` naming the page; its rows go to `datasheets/skipped-parts.md` in that file's format.
- [ ] `candidates.tsv` column 12 is `add` or `out` on every row.

Acceptance: no row has an empty `state`; every `out` row of a drawing reason is in `skipped-parts.md`.
  Check: `awk -F'\t' 'NR>1 && $12!="add" && $12!="out"' planning/plan-51-data/candidates.tsv \| wc -l` → 0 (est. 1 s).
Commit: —

## Validation Plan

- Tests: `series_check.py`. Coverage: it names every group, so a skipped group cannot hide.
- Runtime proof: D and E generate footprints from `series.tsv` and their checks run the loader.
- Doc sync: `datasheets/skipped-parts.md`.
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **The generic sockets' footprints are the Mill-Max 110 and Assmann A-CCS through-hole footprints** (§ 3.3): the two largest through-hole groups, each covering every shape the generic needs.
- **Pad rules are plan-03-B's** (§ 3.1); mid-values as `docs/parts-format.md` says.

## Corrections

## Summary

C's risk is reading about 50 drawings correctly; `series_check.py` and the loader's
pad checks in D bound it. Untouched: code, part files, footprints.
