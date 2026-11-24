# plan-51-E: IC sockets — footprints: PLCC sockets

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-51-D

E extends `scripts/gen-sockets.js` (plan-51-D) to the **PLCC** socket groups,
surface mount (the through-hole PLCC groups are `out`: plan-51-C § Corrections), and writes
their `quad` footprints to a side file that F merges. It includes the footprints the generic
PLCC sockets use (the surface-mount rows marked `generic` in `series.tsv`: Assmann
`ASS_0142` for 28, 32 and 44 pins, Mill-Max 940 for 68).

Outcome of E: `node scripts/gen-sockets.js --letter E --out planning/plan-51-data/footprints-E.json`
writes one `quad` object per distinct `(PLCC row of series.tsv, pin count)`
geometry; each loads with pin 1 at the top centre and no overlapping pads, as the
chip footprints `PLCC-28`, `-32`, `-44`, `-68` do.

References:

- `docs/parts-format.md` § `quad` (`sides`, `pitch`, `spanX`, `spanY`, `slots`).
- plan-51-D § 3 (the generator, ids, `fp-load.sh`); plan-51-C § 3.1–3.3 (rules, `series.tsv`).
- The chip footprints in `schematical/schematical/Parts/footprints.json`: `PLCC-28`,
  `PLCC-32`, `PLCC-44`, `PLCC-44-MC-44J`, `PLCC-68`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-D complete | `ls planning/completed/plan-51-D-*.md` → exists | NOT MET (2026-11-21: not started) |
| `series_check.py` passes | `python3 -I planning/plan-51-data/census/series_check.py` → `ok` | NOT MET (C's output) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `footprints-E.json` loads; the footprint check reports only `extra footprint` and
  `unused footprint` lines for exactly its ids; `gen-sockets.test.js` passes with the PLCC cases.

### Non-goals (explicit constraints)

- No change to `footprints.json`, `Parts/*.json`, goldens, backend or app (F merges).
- No chip footprint changes.

## 2. Current State

- `quad` with `slots`: `PLCC-44` has `sides` `[11,11,11,11]`, `slots` `[39,40,…,44,1,2,…,38]`
  (pin 1 on slot 39, the middle of the top side); `PLCC-32` has `sides` `[9,7,9,7]` and
  `slots` starting `29` (read from `footprints.json`).
- The slot of pin p, for `sides` `[L,B,R,T]` with odd T (pin 1 at the top centre,
  numbers going counter-clockwise: left, then bottom, right, top): the first slot is
  `L+B+R+(T+1)/2`, and pin p sits on slot `((first−1 + p−1) mod total) + 1`. Verified
  against all five existing PLCC footprints by reading `footprints.json`: the first `slots` entry equals the formula for `PLCC-28` (25), `-32` (29), `-44` (39), `-44-MC-44J` (39) and `-68` (60).
- PLCC socket pin counts in the candidates: 28, 32, 44 and 68 (`awk -F'\t' '$4=="PLCC"{print $5}' planning/plan-51-data/candidates.tsv | sort -un` → 28 32 44 68).

### Measured populations

| What | Count | Command |
|---|---|---|
| PLCC variants at planning time (maker, series, mounting, pins) | 15 | the awk of plan-51-D § 2 → `PLCC 15` (Preci-Dip PLCC rows come with B) |
| Chip footprints the PLCC sockets hold | 5 | `python3 -I planning/plan-51-data/census/chips.py` → `PLCC` lines list `PLCC-28`, `PLCC-32`, `PLCC-44`, `PLCC-44-MC-44J`, `PLCC-68` |

## 3. Design

`--letter E`: for each PLCC row and `N` in `pins`: `style` `quad`, `pins` N, `sides`
and `pitch`, `spanX`, `spanY` from the row, `pad` `{pad_w, pad_h, shape}` (`rect`; every
PLCC row left in `series.tsv` is surface mount, no `drill`), `slots` by the § 2 formula,
`body` from the row. Id `SKT-PLCC<N>-SM-<maker code>-<series>` (a series' `_` becomes `-`,
as in D); equal geometry shares one id (D's rule).
A pin count whose `sides` are not `[L,B,R,T]` with odd `T` is not generated (none
of 28 `[7,7,7,7]`, 32 `[9,7,9,7]`, 44 `[11,11,11,11]`, 68 `[17,17,17,17]` is).
The generator fails with a message when a `sides` entry does not add to N.

**Risk.** Pin numbering: a socket pad must carry the chip's pin number. The
formula is checked against the five existing PLCC footprints in the test; the
loader's pad-designator check in `fits` (plan-51-A) checks the numbers' presence,
and E's test checks the position of pin 1.

## 4. Phases

### Phase 1 — PLCC in the generator

- [ ] `scripts/gen-sockets.js`: `--letter E` per § 3.
- [ ] `scripts/gen-sockets.test.js`: the slot formula reproduces the `slots` arrays of all five existing PLCC footprints exactly (read from `footprints.json`); a row with `sides` not adding to N fails with a message; a surface-mount row has no `drill`; the four generic PLCC shapes are required, supplied by the rows' `generic` cell (28, 32, 44 from `ASS_0142`, 68 from Mill-Max 940), each once, as in D.

Acceptance: the tests pass.
  Check: `node --test scripts/gen-sockets.test.js` → `# pass`, no `# fail` (est. 5 s).
Commit: —

### Phase 2 — the PLCC footprints

- [ ] `node scripts/gen-sockets.js --letter E --out planning/plan-51-data/footprints-E.json`.

Acceptance: the side file loads; its ids are exactly the "new" ids.
  Check: `planning/plan-51-data/census/fp-load.sh planning/plan-51-data/footprints-E.json` → `<n> new footprints: load ok` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `gen-sockets.test.js` PLCC cases; `fp-load.sh`.
- Coverage check: the test compares against the five real PLCC footprints, so a formula change fails.
- Runtime proof: F merges and runs the whole `ctest`.
- Doc sync: none (R10 is D's).
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions and plan-51-D § Decisions.

## Corrections

- **2026-11-24 (plan-51-C), no through-hole PLCC.** The 15 through-hole PLCC candidates are `out` (their tails are on staggered 2.54 mm hole rows that `quad` cannot draw), so E generates only surface-mount `quad` footprints: `series.tsv` has 4 PLCC rows (Assmann `ASS_0142`: 28, 32, 44; Mill-Max 540: 32, 44; Mill-Max 940: 32, 68; Preci-Dip 540: 68), and the generic PLCC sockets (F) are surface mount. The planning-time "15 PLCC variants" is now `awk -F'\t' '$4=="PLCC" && $12=="add"' …` (counted in § 2's command by the generator's output).

## Summary

E's risk is pin 1's slot; the test against the existing footprints bounds it.
Untouched: `footprints.json`, parts, backend, app.
