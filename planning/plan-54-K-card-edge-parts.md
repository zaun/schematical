# plan-54-K: Parts — card-edge sockets, the datasheets, the goldens

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-J

K adds the card-edge parts: one `connector` part per `add` row of the **card-edge
sockets** (1014 candidates at planning time, before C and D's skips: Samtec 389,
Amphenol ICC 325, TE Connectivity 300), the footprints they name (from F, G and H's
`footprints-F.json`, `-G.json`, `-H.json`), the datasheet entries, the regenerated goldens and
the real-library test of the third example. There is **no generic part**. The behavioral
outcome: the library loads with P0 (as J left it) plus these parts, every new part has a
manufacturer, a datasheet and a footprint, `ctest` passes, and the goldens only gain lines.

References:

- plan-54-C § 3 and plan-54-D § 3 (pins and facts), plan-54-I § 3.2 (the naming conventions),
  plan-54-E § 3.1 (footprint ids), plan-54-J § 3 (the generator, the device and datasheet
  rules); `docs/parts-format.md` § Parts, § Datasheets, § Footprints, § `connector`;
  `docs/backend.md` § Tests (golden regeneration).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-J complete | `ls planning/completed/plan-54-J-*.md` → exists | NOT MET (2026-11-22: not started) |
| plan-54-H complete | `ls planning/completed/plan-54-H-*.md` → exists | NOT MET (2026-11-22: not started) |
| The pick is final | `awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` → the `add` rows (at most 1014; 0 at planning, all `candidate`) | NOT MET (2026-11-22: C and D) |
| The footprint tables are done | `python3 -I planning/plan-54-data/census/fp-check.py F && python3 -I planning/plan-54-data/census/fp-check.py G && python3 -I planning/plan-54-data/census/fp-check.py H` → each `0 without a footprint` | NOT MET (2026-11-22: F, G, H) |
| The goldens are P0 / R0 / F0 | `wc -l < backend/tests/golden/parts.txt` etc. re-measured at the start; K uses its own | MET (2026-11-22 at 7b449718: 34375 / 34375 / 1895; J moves them) |

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
- `connector_test` shows the card-edge example of `examples.tsv` joining nothing.

### Non-goals (explicit constraints)

- No backend change; no app change (L); no change to the existing connector devices or J's
  (their text stays byte for byte).
- No part outside the `add` rows; **no generic part** (owner).

## 2. Current State

After J, `connector.json` holds the barrel and USB devices and J's `card-…` devices;
`gen-parts.py` keeps every block it does not generate. `footprints.json` holds J's footprints;
the card-edge footprints are in `footprints-F.json`, `-G.json`, `-H.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts to add | the card-edge `add` rows (1014 `candidate` at planning time) | `awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` |
| Devices: one per variant | 132 at planning time (Samtec 33, TE 43, Amphenol ICC 56) | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` |
| Datasheet entries to add | at most 312 sources at planning time | same, `download sources` |
| Footprints to add | at most 408 keys at planning time | same |
| Positions | 2 to 428 (5 parts above 300) | `awk -F'\t' '$20=="candidate" && $4=="edge"{print $8}' planning/plan-54-data/candidates.tsv \| sort -n \| tail -3` |

### Verified properties

- The generator (J § 3.1) drops only the blocks whose `id` starts with `card-<family>-<maker slug>-` of the
  pairs it is asked for (plan-54-I § 3.2: `card-edge-samtec-…`), so a run for the `edge` family keeps
  J's `card-socket-…` and `card-module-…` blocks byte for byte.
- The largest pinout in the library today is 196 pins; a 428-pin pinout is loaded by the same
  `connector_bind_pinout` (no limit, plan-54-I § 2) and its footprint by `pins` ≤ 1000
  (`footprints.c:17`).

## 3. Design

As plan-54-J § 3: `gen-parts.py --family edge --maker samtec|te|amphenol`; device `drawer`
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

- [ ] Re-measure P0, R0, F0; `census/gen-parts.py --family edge --maker samtec` (adds the `edge`
      family to the generator's families), `fp-merge.py --only` the footprints of `footprints-F.json`
      those parts name, goldens, review: only added lines.

Acceptance: Samtec's card-edge sockets load with footprints; goldens only gained.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0 (est. 20 min).
Commit: —

### Phase 2 — TE Connectivity

- [ ] `gen-parts.py --family edge --maker te`; footprints of `footprints-G.json`; goldens; review.

Acceptance: TE's card-edge sockets load with footprints; goldens only gained.
  Check: as Phase 1 (est. 20 min).
Commit: —

### Phase 3 — Amphenol ICC, the third example

- [ ] `gen-parts.py --family edge --maker amphenol`; footprints of `footprints-H.json`; goldens; review;
      `gen-parts.py --check` → exit 0.
- [ ] `examples.tsv` gains the most-stocked card-edge `add` row; `backend/tests/connector_test.c` places it
      like J's two examples (pins 1 and 2 on two nets, neither shorted).

Acceptance: every card-edge `add` row is a part; the example joins nothing.
  Check: `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + J's parts + the card-edge `add` rows (`awk -F'\t' '$4=="edge" && $20=="add"' planning/plan-54-data/candidates.tsv | wc -l`); `ctest --test-dir /tmp/logic-p54 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 25 min).
Commit: —

## Validation Plan

- Tests: as J (`library_check`, the three goldens' checks, `connector_test`).
- Coverage check: the new devices load in every test through the shipped `Parts/` directory.
- Runtime proof: `connector_test`'s card-edge example.
- Doc sync: none beyond I's § Card slots; if a card-edge pin rule changed in C or D's
  Corrections, `docs/parts-format.md` § Card slots is updated here.
- Final gate: none; the landing tier is M's.

## Decisions

- Owner (2026-11-22): no generic part, Active only, everything the makers list.
- Author: as plan-54-J § Decisions (one device per variant, one pinout per position count).

## Corrections

## Summary

K is J's generator over the card-edge pick. The risk is volume and the pins-against-pads match;
the goldens only gain lines.
