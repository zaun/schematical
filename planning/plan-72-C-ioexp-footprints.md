# plan-72-C: Footprints — every package of the pick that `footprints.json` lacks

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-B

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
C gives every `new` line of `planning/plan-72-data/footprints-needed.txt` a
footprint drawn from the datasheet's package drawing, and checks every
`exists` line's land pattern against the drawing. It writes the new objects
to `planning/plan-72-data/footprints-C.json` and the (PDF, package) →
footprint map to `planning/plan-72-data/footprint-map.tsv`; F, G and H load
them into `Parts/footprints.json` with the parts that use them.

References:

- plan-72-B (`footprints-needed.txt`, `specs.tsv`).
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `grid`, `explicit`,
  `centre`, rounded and cut corners, naming).
- `planning/completed/plan-48-C-timer-opamp-footprints-leaded.md` and
  `plan-48-D-timer-opamp-footprints-leadless.md` (§ 3.1 rules: reuse, shared
  ids, the objects ready to paste); `planning/plan-48-data/census/`
  `fp-check.py`, `fp-load.sh`, `fp-merge.py` (copied and adapted here).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-B complete | `grep -c '^- \[ \]' planning/plan-72-B-ioexp-specs.md` → 0 | NOT MET (2026-11-25) |
| The list is final | `python3 -I planning/plan-72-data/census/footprint-lines.py --verify` → `0 rows without a footprint` | NOT MET (B Phase 3) |
| Footprint count at C's start | `python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` | 3993 at planning (63e2f7d2); recorded here at C's start as **N1** |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Every `new` line of `footprints-needed.txt` has an object in
  `footprints-C.json`, drawn from the cited drawing page, that
  `footprint_check` loads with no error.
- Every `exists` line is checked: pad size, pitch, row span and exposed pad
  match the datasheet's recommended land pattern within 0.05 mm, or the line
  becomes `new` (a maker-specific id, `<PKG>-<MAKER>-<CODE>`).
- `footprint-map.tsv`: `pdf package footprint` for every `specs.tsv` row.

### Non-goals

- `Parts/footprints.json` is not edited here (the parts letters merge).
- No footprint for a part outside the pick; no 3D model.

## 2. Current State

Measured at planning (2026-11-25, a regex scan of the ids in
`schematical/schematical/Parts/footprints.json`, 3993 objects): the generic
`TSSOP-8/16/20/24/28/56`, `SOIC-14/16/16W/20W/24W/28W`, `SSOP-16/20/24/28/48`,
`QSOP-16/20/24/28`, `VSSOP-8/10/20/24`, `PDIP-8/14/16/28-600`, `PLCC-44`,
18 ids containing `QFN-16`, 14 containing `QFN-24`, 9 `QFN-28`, 6 `QFN-40`,
8 `X2SON`, 24 `DSBGA`/`YZP`, 13 `MSOP` exist. Packages in the pick with no
generic id at planning: 28-SPDIP (300 mil; `grep -c '"PDIP-28-300"'` → 0),
18-PDIP, 36-SSOP, 100-TQFP (only `HTQFP-100-PZP` and `TQFP-100-MC-100A`), and
the maker-coded QFN/DFN/MSOP variants B names. The exact `new` count is B's
`grep -c ' new ' planning/plan-72-data/footprints-needed.txt`.

## 3. Design

As plan-48-C/-D § 3.1: an id is reused only when its land pattern equals the
datasheet's; a shared id another open plan also needs (plans 69–74) is added
once — grep `footprints.json` and every `planning/plan-*-data/footprints-*.json`
before writing one. Leaded packages use `dual`; QFN/DFN/SON use `quad` or `dual`
with `centre`; BGA/WLCSP use `grid`; anything irregular `explicit`.
Risk concentrates in exposed-pad and corner-pin variants (TI's RGE/RTW/RHL,
NXP's HVQFN, Microchip's ML/MG), which look alike and differ in pad size.

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — tools and leaded packages (DIP, SOIC, SSOP, QSOP, TSSOP, MSOP / VSSOP, TSOT, SC-70, PLCC)

- [ ] Copy `fp-check.py`, `fp-load.sh`, `fp-merge.py` into `planning/plan-72-data/census/`, adapted to `footprints-needed.txt` and `footprint-map.tsv`; `--selftest` where plan-48's had one.
- [ ] Check every leaded `exists` line; draw every leaded `new` line into `footprints-C.json`.

Acceptance: `python3 -I planning/plan-72-data/census/fp-check.py leaded` → `<n> lines, 0 without a footprint`, and `planning/plan-72-data/census/fp-load.sh planning/plan-72-data/footprints-C.json` → only `extra footprint` / `unused footprint` lines for the new ids (est. 4 min: one `footprint_check` build).
Commit:

### Phase 2 — leadless and ball packages (QFN, DFN, SON, X2SON, TQFP-100, BGA, WLCSP)

- [ ] The same for every leadless and ball line.

Acceptance: `fp-check.py all` → `<n> lines, 0 without a footprint`; `fp-load.sh footprints-C.json` as Phase 1 for exactly the ids in `footprints-C.json` (est. 4 min).
Commit:

## Validation Plan

- Tests: `fp-check.py`, `fp-load.sh` (the backend's `footprint_check` on the new objects).
- Runtime proof: F/G/H's goldens (`footprints.txt`) and the app's footprint preview in I.
- Final gate (once): `fp-check.py all` and `fp-load.sh` as Phase 2 (est. 5 min).

## Decisions

- Reuse only on an equal land pattern; maker-coded ids otherwise — plan-42/-48 rule.

## Corrections

(none yet)
