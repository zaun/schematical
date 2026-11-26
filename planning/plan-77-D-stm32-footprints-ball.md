# plan-77-D: STM32 footprints: UFBGA, TFBGA, LFBGA, VFBGA and WLCSP packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-77-C

D draws every ball package of `planning/plan-77-data/footprints-needed.txt` as `grid`
footprints (ball names = the XML positions B used) and merges them.

References:

- `docs/parts-format.md` § `grid` — ball grid (line 739: rows skip I, O, Q, S, X, Z; rows ≤ 20,
  columns ≤ 30), § `explicit` — literal pads (line 822; ball names).
- `planning/plan-77-B-stm32-pinouts.md` § 3.6; `planning/plan-77-data/README.md` (ball rows
  measured in the XML: A–H, J–N, P, R, T, U, V, W, Y, AA–AG).
- Precedent: `planning/completed/plan-36-J-explicit-ball-pads.md`.

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-77-C complete | `ls planning/completed/plan-77-C-*.md` → exists | NOT MET (2026-11-25) |
| None of D's ids exists | `python3 -I -c "import json;print(sum(f['id'].endswith('-ST') or '-ST-' in f['id'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints'] if f['id'].split('-')[0] in ('UFBGA','TFBGA','LFBGA','VFBGA','WLCSP')))"` → 0 | MET (2026-11-25: 0) |
| Shared ids added once | plan-77-C's rule | a rule |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

Every ball line of `footprints-needed.txt` names a footprint in `Parts/footprints.json` whose
pitch and ball pad diameter equal the datasheet's footprint example and whose `balls` are exactly
the pinout's ball names; `footprints.txt` gains one line per new id.

### Non-goals

- No part names them yet (F). No via-in-pad or escape routing data.

## 2. Current State

The library has one BGA (`BGA-24-MICROSTAR`) and 13 small WLCSP footprints from other makers;
none fits an STM32 ball map. ST prints a "Footprint example" per ball package with pitch and pad
(e.g. `stm32f103c8.pdf` Figure 50, UFBGA100).

## 3. Design

- `grid` footprint: `rows` × `columns` from the package mechanical table (the ball matrix),
  `pitch`, pad = the figure's pad (round, diameter), `balls` = the pinout's ball names (from
  `devices.json`; every ball of every device's pinout for that footprint must be a grid name —
  `census/fp-check.py --balls`).
- A package whose populated balls differ between the dies using it gets one footprint per die
  (`<PACKAGE>-ST-<die>`, B § 3.6).
- STM32U3 WLCSP33 / WLCSP42 positions are numbers, not grid names (README): those are `explicit`
  footprints with `ball` = the XML position and x/y read from the datasheet's ball table (a rule
  for any `add` part on those two packages; `footprints-needed.txt` lists which).
- Same files as C: `footprints-D.json`, `fp-pages.tsv`, `fp-merge.py`.

## 4. Phases

### Phase 1 — Read the figures

- [ ] `footprints-D.json` and `fp-pages.tsv` rows for every ball package.

Acceptance: `python3 -I planning/plan-77-data/census/fp-check.py --balls footprints-D.json` →
every footprint's balls equal the union of its pinouts' balls, 0 off-grid.

Commit:

### Phase 2 — Merge and goldens

- [ ] `fp-merge.py footprints-D.json`; regenerate `footprints.txt`.

Acceptance: `ctest --test-dir /tmp/logic-p77 -R footprint` → passes; the golden diff adds one
line per id of `footprints-D.json`, 0 deletions.

Commit:

## 5. Validation Plan

D's acceptance commands (under a minute). The landing tier is H's.
