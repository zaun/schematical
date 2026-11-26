# plan-77-C: STM32 footprints: LQFP, TSSOP, SO and QFN packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-77-B

C draws every non-ball package of `planning/plan-77-data/footprints-needed.txt` from ST's
recommended footprint figures: `LQFP-<n>-ST`, `UFQFPN-<n>-ST`, `VFQFPN-<n>-ST`, `UQFN-<n>-ST`,
`TSSOP-20-ST`, and merges them into `Parts/footprints.json`. D does the ball packages.

References:

- `docs/parts-format.md` § Footprints (line 601: `dual`, `quad`, `centre`, corners);
  `docs/backend.md` § goldens (`footprints.txt`).
- `planning/plan-77-B-stm32-pinouts.md` § 3.5–3.6 (ids, the measured LQFP48/64 figures).
- Precedent: `planning/completed/plan-36-D-mcu-footprints-quad.md` (fp-pages, the merge script,
  golden lines).

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-77-B complete | `ls planning/completed/plan-77-B-*.md` → exists | NOT MET (2026-11-25) |
| None of C's ids exists | `python3 -I -c "import json;print(sum(f['id'].endswith('-ST') and f['id'].split('-')[0] in ('LQFP','UFQFPN','VFQFPN','UQFN','TSSOP') and f['id']!='TSSOP-14-ST' for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-25: 0) |
| Shared ids added once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0 | a rule, applied per footprint (`fp-merge.py` skips an id already present) |
| Footprint golden size at C's start | `wc -l < backend/tests/golden/footprints.txt` | 3993 at planning. C records its own F1 |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

Every non-ball line of `footprints-needed.txt` names a footprint in `Parts/footprints.json`
whose pads equal the datasheet figure's (pad size, pitch, pad-centre span from the outer and
inner extents, exposed pad), with its `note` naming the datasheet, page, figure and drawing code;
`footprints.txt` gains exactly one line per new id.

### Non-goals

- No part names a new footprint yet (F does). No change to existing footprints.
- No ball packages (D).

## 2. Current State

No `-ST` footprint for these packages exists except `SOIC-8-ST-SO8N` and `TSSOP-14-ST` (reused,
B § 3.6). ST draws its recommended footprints as dimensioned figures (`pdftotext -layout` gives
the numbers, e.g. LQFP48: `0.50`, `1.20`, `0.30`, `9.70 7.30`, `5.80`, `9.70`; DS5319 p.100).

## 3. Design

- Per package, one figure read: pad length L and width W, pitch e, outer extent O and inner
  extent I; pad-centre span = O − L (LQFP48: 9.70 − 1.20 = 8.50, I + L = 7.30 + 1.20 = 8.50, the
  two must agree within 0.01 or the reading is wrong). Body from the package mechanical table (D1
  / E1 typ). QFN exposed pad: the figure's centre pad size → `centre`, unnumbered unless B's
  pinout gave it a number (`HasPowerPad`), then numbered n+1 via `slots`.
- `planning/plan-77-data/footprints-C.json` holds the new footprints; `fp-pages.tsv` the id →
  datasheet, page, figure, drawing code; `census/fp-merge.py` (plan-74-D's pattern: skip an id
  already present, append, keep the file's order otherwise) merges them.
- Each value is cross-checked on a second datasheet that prints the same drawing code when one
  exists (`fp-pages.tsv` column `second`).

## 4. Phases

### Phase 1 — Read the figures

- [ ] `fp-pages.tsv` rows for every non-ball package; `footprints-C.json`.

Acceptance: `python3 -I planning/plan-77-data/census/fp-check.py footprints-C.json` → every
footprint's span check (O − L = I + L) agrees within 0.01 mm, every id is in
`footprints-needed.txt`.

Commit:

### Phase 2 — Merge and goldens

- [ ] `fp-merge.py footprints-C.json`; regenerate `backend/tests/golden/footprints.txt`
      (`docs/backend.md` § goldens).

Acceptance: `ctest --test-dir /tmp/logic-p77 -R footprint` → passes; `git diff --stat
backend/tests/golden/footprints.txt` → insertions = the number of ids in `footprints-C.json`,
0 deletions.

Commit:

## 5. Validation Plan

C's acceptance commands (the footprint test, under a minute). The landing tier is H's.
