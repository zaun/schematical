# plan-54-N: Parts — card-edge sockets, the datasheets, the goldens

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-M

N adds the card-edge parts: one `connector` part per `add` row of the **card-edge
sockets** (1014 candidates at planning time, before C and D's skips: Samtec 389,
Amphenol ICC 325, TE Connectivity 300), the footprints they name (from I, J and K's
`footprints-I.json`, `-J.json`, `-K.json`), the datasheet entries, the regenerated goldens and
the real-library test of the third example. There is **no generic part**. The behavioral
outcome: the library loads with P0 (as M left it) plus these parts, every new part has a
manufacturer, a datasheet and a footprint, `ctest` passes, and the goldens only gain lines.

References:

- plan-54-C § 3 and plan-54-D § 3 (pins and facts), plan-54-L § 3.2 (the naming conventions),
  plan-54-H § 3.1 (footprint ids), plan-54-M § 3 (the generator, the device and datasheet
  rules); `docs/parts-format.md` § Parts, § Datasheets, § Footprints, § `connector`;
  `docs/backend.md` § Tests (golden regeneration).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-M complete | `ls planning/completed/plan-54-M-*.md` → exists | MET (2026-11-23: I, J, K committed on worktree-P-54) |
| plan-54-K complete | `ls planning/completed/plan-54-K-*.md` → exists | MET (2026-11-23: I, J, K committed on worktree-P-54) |
| The pick is final | `awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` → the `add` rows (at most 1014; 0 at planning, all `candidate`) | MET (2026-11-23) |
| The footprint tables are done | `python3 -I planning/plan-54-data/census/fp-check.py I && python3 -I planning/plan-54-data/census/fp-check.py J && python3 -I planning/plan-54-data/census/fp-check.py K` → each `0 without a footprint` | MET (2026-11-23) |
| The goldens are P0 / R0 / F0 | `wc -l < backend/tests/golden/parts.txt` etc. re-measured at the start; N uses its own | MET (2026-11-22 at 7b449718: 34375 / 34375 / 1895; M moves them) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- One `connector` part per card-edge `add` row is in `Parts/connector.json`.
- `Parts/footprints.json` gains the `EDGE-…` footprints those parts name and no other
  (`footprint_check` passes: none unused, none missing).
- `Parts/datasheets.json` gains one entry per distinct maker URL of the card-edge `add` rows (312
  distinct sources at planning time, before the skips).
- `backend/tests/golden/{parts,traces,footprints}.txt` only gain lines.
- `connector_test` shows the card-edge and straddle-mount examples of `examples.tsv` joining nothing.

### Non-goals (explicit constraints)

- No backend change; no app change (O); no change to the existing connector devices or M's
  (their text stays byte for byte).
- No part outside the `add` rows; **no generic part** (owner).

## 2. Current State

After M, `connector.json` holds the barrel and USB devices and M's `card-…` devices;
`gen-parts.py` keeps every block it does not generate. `footprints.json` holds M's footprints;
the card-edge footprints are in `footprints-I.json`, `-J.json`, `-K.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts to add | the card-edge `add` rows (1014 `candidate` at planning time) | `awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` |
| Devices: one per variant | 132 at planning time (Samtec 33, TE 43, Amphenol ICC 56) | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` |
| Datasheet entries to add | at most 312 sources at planning time | same, `download sources` |
| Footprints to add | at most 408 keys at planning time | same |
| Positions | 2 to 428 (5 parts above 300) | `awk -F'\t' '$20=="candidate" && $4=="edge"{print $8}' planning/plan-54-data/candidates.tsv \| sort -n \| tail -3` |

### Verified properties

- The generator (M § 3.1) drops only the blocks whose `id` starts with `card-<family>-<maker slug>-` of the
  pairs it is asked for (plan-54-L § 3.2: `card-edge-samtec-…`), so a run for the `edge` family keeps
  M's `card-socket-…` and `card-module-…` blocks byte for byte.
- The largest pinout in the library today is 196 pins; a 428-pin pinout is loaded by the same
  `connector_bind_pinout` (no limit, plan-54-L § 2) and its footprint by `pins` ≤ 1000
  (`footprints.c:17`).

## 3. Design

As plan-54-M § 3: `gen-parts.py --family edge --maker samtec|te|amphenol`; device `drawer`
`["Connector", "Card slot", "Card edge"]`, `function` `<pitch> mm <card>`, `card` and `cardPitch`
from C's and D's columns (`cardPitch` for every card-edge device), pins named as C § 3.3 (the
`seq`, `AB` and explicit pinouts), `"side": "in"` on the first half of a pinout of more than 40
pins. Footprint ids `EDGE-<MAKER>-<series slug>-<positions>`.

**Risk**: volume (1014 parts, 132 devices, 408 footprints in one file) and the
pins-against-pads match for each part; the load of every device and `footprint_check` catch a
count that differs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Samtec

- [x] Re-measure P0, R0, F0; `census/gen-parts.py --family edge --maker samtec` (adds the `edge`
      family to the generator's families), `fp-merge.py --only` the footprints of `footprints-I.json`
      those parts name, goldens, review: only added lines.

      Result: P0 = R0 = 34569, F0 = 2020 at the start (after M); `gen-parts.py --family edge` generated the three makers at once (§ Corrections); `fp-merge.py` appended the 178 footprints of
      `footprints-I/J/K.json`; goldens regenerated: `diff … | grep -c '^<'` → 0, 0, 0; `ctest --test-dir /tmp/logic-p54 -j8` → `100% tests passed, 0 tests failed out of 42`.

Acceptance: Samtec's card-edge sockets load with footprints; goldens only gained.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0 (est. 20 min).
Commit: ba45a661

### Phase 2 — TE Connectivity

- [x] `gen-parts.py --family edge --maker te`; footprints of `footprints-J.json`; goldens; review.

      Result: Samtec 119, TE 221, Amphenol ICC 59 card-edge parts (399) in one run: `gen-parts.py` → `257 devices, 575 parts, 231 datasheets` for all three families.

Acceptance: TE's card-edge sockets load with footprints; goldens only gained.
  Check: as Phase 1 (est. 20 min).
Commit: ba45a661

### Phase 3 — Amphenol ICC, the third example

- [x] `gen-parts.py --family edge --maker amphenol`; footprints of `footprints-K.json`; goldens; review;
      `gen-parts.py --check` → exit 0.
- [x] `examples.tsv` gains the most-stocked card-edge `add` row and the most-stocked straddle-mount `add`
      row (a part whose footprint has `opposite` pads); `backend/tests/connector_test.c` places both like M's
      two examples (pins 1 and 2 on two nets, neither shorted).

      Result: `gen-parts.py --check` → `up to date`; `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + 176 + 399 = 951; `examples.tsv` gains `7-1734774-7` (card edge, 36 positions) and
      `2387405-2` (straddle mount, 64 positions, footprint `EDGE-TE-2372484-64` with 32 `opposite` pads and `edgeMount`); `connector_test` → `PASS: 7-1734774-7 …`, `PASS: 2387405-2 (a library card slot) joins nothing`.

Acceptance: every card-edge `add` row is a part; the example joins nothing.
  Check: `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + M's parts + the card-edge `add` rows (`awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv | wc -l`); `ctest --test-dir /tmp/logic-p54 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 25 min).
Commit: ba45a661

## Validation Plan

- Tests: as M (`library_check`, the three goldens' checks, `connector_test`).
- Coverage check: the new devices load in every test through the shipped `Parts/` directory.
- Runtime proof: `connector_test`'s card-edge example.
- Doc sync: none beyond L's § Card slots; if a card-edge pin rule changed in C or D's
  Corrections, `docs/parts-format.md` § Card slots is updated here.
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): no generic part, Active only, everything the makers list.
- Author: as plan-54-M § Decisions (one device per variant, one pinout per position count).

## Corrections

- **One generator run for every family** (`gen-parts.py` with no `--family`): a run for one family keeps the other families' blocks and appends its own, so the file order depended on the order of runs and
  `--check` (which regenerates everything) differed; the canonical state is the all-family run (blocks sorted by id after the kept ones).
- **`datasheets.json` ids** of the TE documents named `DDEController.pdf` collide by file name; `place.py` suffixes real clashes by SHA, and the one id with two URLs (`ddecontroller`, the same file served for two DocIds) takes the
  first URL sorted.
- The generator's 399 parts: Samtec 119 (devices from 11 lines), TE 221, Amphenol ICC 59.

## Summary

N is M's generator over the card-edge pick. The risk is volume and the pins-against-pads match;
the goldens only gain lines.
