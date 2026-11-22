# plan-54-M: Parts — memory / SIM card sockets and module sockets, the datasheets, the goldens

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-L

M adds the parts of the first two families: one `connector` part per `add` row of
the **memory / SIM card sockets** (134 candidates at planning time, before B's
skips) and the **module sockets** (122), the footprints they name (from H's
`footprints-H.json`), the datasheet entries, the regenerated goldens and the
real-library tests. There is **no generic part**. The behavioral outcome: the library
loads with P0 plus these parts, every new part has a manufacturer, a datasheet and a
footprint, `ctest` passes, and the goldens only gain lines.

References:

- plan-54-B § 3 (the pins and facts), plan-54-L § 3.2 (the naming conventions: pins,
  drawer, function, package, device id, part fields), plan-54-H § 3.1 (footprint ids);
  `docs/parts-format.md` § Parts, § Datasheets, § Footprints, § `connector`;
  `docs/backend.md` § Tests (golden regeneration).
- `scripts/gen-headers.js` (the idempotent generator precedent) and
  `planning/completed/plan-44-F-crystal-parts.md` (`census/gen-parts.py`, the phases per
  maker, `fp-merge.py --only`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-L complete | `ls planning/completed/plan-54-L-*.md` → exists | NOT MET (2026-11-22: not started) |
| The pick is final | `awk -F'\t' '($4=="card"\|\|$4=="module") && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` → the `add` rows (at most 256; 0 at planning, all `candidate`) | NOT MET (2026-11-22: B Phase 4) |
| The footprint table is done | `python3 -I planning/plan-54-data/census/fp-check.py H` → `0 without a footprint` | NOT MET (2026-11-22: H) |
| The goldens are P0 / R0 / F0 | `wc -l < backend/tests/golden/parts.txt` etc. re-measured at the start; M uses its own | MET (2026-11-22 at 7b449718: 34375 / 34375 / 1895) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- One `connector` part per `add` row of the two families is in `Parts/connector.json`.
- `Parts/footprints.json` gains the footprints those parts name and no other
  (`footprint_check` passes: none unused, none missing).
- `Parts/datasheets.json` gains one entry per distinct maker URL of the `add` rows (the
  PDFs are in the tree, A).
- `backend/tests/golden/{parts,traces,footprints}.txt` only gain lines.
- `connector_test` shows a placed part of each family joining nothing
  (`planning/plan-54-data/examples.tsv`).

### Non-goals (explicit constraints)

- No backend change (L did it); no app change (O); no change to the 224 existing connector
  devices (their text stays byte for byte).
- No part outside the `add` rows; **no generic part** (owner).
- No card-edge part (N).

## 2. Current State

`connector.json` is 2894 lines: 7 barrel and 217 USB devices, each a block of lines under
`"devices": [` indented four spaces (`    {` … `    },`), pinouts and parts on single lines.
`datasheets.json` is sorted by id, one entry a line (8558 entries). `footprints.json` has 1895
footprints one per line. The loader checks every device (L § 3.1 for the new keys) and
`listing_check`, `trace_check`, `footprint_check` compare against the goldens.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts to add | the `add` rows of the two families (256 `candidate` at planning time) | `awk -F'\t' '($4=="card"\|\|$4=="module") && $20=="add"' planning/plan-54-data/candidates.tsv \| wc -l` |
| Devices the rows make: one per variant | 83 at planning time (62 card + 21 module) | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` |
| Datasheet entries to add | at most 186 (112 card + 74 module distinct sources at planning time) | same, `download sources` per family |
| Footprints to add | at most 117 (87 card + 30 module keys at planning time) | same |
| Existing `datasheets.json` ids | 8558 | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` |
| Backend goldens | P0 34375, R0 34375, F0 1895 | plan-54-A § Prerequisites |

### Verified properties

- `datasheets.json`'s `url` must be the maker's own site and the same document as the local PDF
  (`docs/parts-format.md` § Datasheets); an Amphenol ICC entry names the
  `cdn.amphenol-cs.com` URL DigiKey gives and the PDF was downloaded from Amphenol's own
  `catsy.925` store (plan-54-A § 4), which `scripts/check-datasheets.js` will report as `HTTP
  403` for `cdn.amphenol-cs.com`; plan-54-A § 2 measured 403 or 429 from every script for those
  hosts, so the docs gain the same note as Suntsu's (Phase 3).
- `fp-merge.py --expect` fails unless the printed footprints beyond the golden are exactly the new
  ids and none has overlapping pads (`planning/plan-44-data/census/fp-merge.py` docstring), so the
  merge itself checks the pad overlap of H's footprints.

## 3. Design

### 3.1 The generator

`planning/plan-54-data/census/gen-parts.py` (idempotent; `--check` exits 1 when `connector.json`
or `datasheets.json` differ from what it would write): splits `connector.json` into its device
blocks by their four-space braces, drops the blocks whose `id` starts with `card-<family>-<maker slug>-` for the
(family, maker) pairs it is asked to generate, keeps every other block byte for byte (so another
plan's blocks, and the pairs not selected, survive), and appends the generated blocks;
inserts the generated `datasheets.json` lines in id order and removes none. `--family socket|module`
and `--maker <slug>` select what is generated; running with none generates everything of the
two families. `fp-merge.py --only <ids>` then appends the named footprints of `footprints-H.json`
to `footprints.json`.

### 3.2 Devices, parts, names

plan-54-L § 3.2 is the rule for the device `id`, `drawer`, `function`, `package`, the pins and
the part fields; in addition:

| Field | Rule |
|---|---|
| device `description` | `<maker> <series> <card> card slot, <N> contacts` (`GCT SIM8055 Nano SIM card slot, 7 contacts`) |
| device `card` | B's `card` column |
| device `cardPitch` | B's `pitch` column for a module socket; omitted for a card socket |
| device `note` | `<maker> <series>, <datasheet file>` |
| pinouts | `p<NN>` per position count the device's parts use; pins numbered 1 … n from B's `pinout`, roles `inout` / `nc`, `"side": "in"` on the first half of a pinout of more than 40 pins |
| `datasheets.json` entry | `{"id", "url"}`: `id` the placed file's name lower-case with `.pdf` dropped (`sim8055-spec`, `2309407`); on a clash with an existing id of another URL, `<id>-<maker slug>`; `url` the maker URL (`url` of `candidates.tsv`) |

### 3.3 Goldens and risk

Three goldens grow by the new parts and footprints; the expected diff is added lines only. The risk
is a wrong pinout or footprint on a part; both surface as a load error, a `footprint_check` failure
(a pad count that differs from the pinout) or the examples test.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator and the first maker

- [ ] Re-measure P0, R0, F0 and `grep -c '"id": "card-' schematical/schematical/Parts/connector.json`
      (0 expected; another plan may have added devices: the generator leaves them). Families
      `socket` (memory / SIM) and `module` are M's.
- [ ] `census/gen-parts.py` (§ 3.1, § 3.2) from `planning/plan-44-data/census/gen-parts.py`;
      generate `--family socket --maker same-sky`, then `fp-merge.py --only` the footprints those
      parts name, then regenerate the goldens (`docs/backend.md` § Tests) and review: only added lines.
- [ ] Tests: `library_check`, `listing_check`, `trace_check`, `footprint_check`.

Acceptance: Same Sky's card sockets load with footprints; goldens only gained.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0; `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + Same Sky's `add` rows (est. 15 min).
Commit: —

### Phase 2 — The other card-socket makers and the module sockets

- [ ] `gen-parts.py --family socket` (Amphenol ICC and GCT), then `--family module` (TE Connectivity,
      Amphenol ICC, Molex); `fp-merge.py --only` for each; goldens; review: only added lines.
- [ ] `python3 -I planning/plan-54-data/census/gen-parts.py --check` → exit 0.

Acceptance: every `add` row of the two families is a part.
  Check: `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + the `add` rows of the two families (`awk -F'\t' '($4=="card"||$4=="module") && $20=="add"' planning/plan-54-data/candidates.tsv | wc -l`); `ctest --test-dir /tmp/logic-p54 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 20 min).
Commit: —

### Phase 3 — The examples, the real-part test, the datasheet note

- [ ] `planning/plan-54-data/examples.tsv`: the most-stocked `add` row of each family (`mpn`, `family`,
      `positions`, `drawer`, `footprint`), written by `census/gen-parts.py --examples`; N adds the
      card-edge row.
- [ ] `backend/tests/connector_test.c`: for each example (a memory / SIM socket and a module socket
      now; N adds the third) a placed part with pins 1 and 2 on two nets driven from `5V` and `GND`:
      after `circuit_start` and `circuit_advance(circuit, 1)` neither net is shorted.
- [ ] `docs/parts-format.md` § Datasheets: the Suntsu paragraph gains the Amphenol ICC sentence (its
      `cdn.amphenol-cs.com` entries answer 403 or 429 to a script; the local PDFs are Amphenol's own
      `catsy.925` copies).

Acceptance: a placed card socket and module socket join nothing.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `grep -c 'catsy.925' docs/parts-format.md` → 1 (est. 15 min).
Commit: —

## Validation Plan

- Tests: `library_check` (every device loads), `listing_check` / `trace_check` / `footprint_check` against the
  goldens, `connector_test` (the examples).
- Coverage check: the new devices are loaded by every test through the shipped `Parts/` directory
  (`logic_test_support`), so a load error fails all of them.
- Runtime proof: `connector_test`'s examples.
- Doc sync: `docs/parts-format.md` § Datasheets.
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): **no generic part**, **Active only**, everything the makers list.
- Author: **One device per variant, one pinout per position count** (§ 3.2), as the USB devices
  (one per part family) rather than one device per part: a variant shares its drawer, function, `card`,
  `cardPitch` and footprint family.
- Author: **The datasheet entry id on a clash is `<id>-<maker slug>`**, deterministic and sorted.

## Corrections

## Summary

M is a generator over B's pick: devices, pinouts, parts, datasheet entries and H's footprints, with the
goldens only gaining lines. The risk is a pinout whose count differs from the footprint's pads; the load
and the example parts' test catch it.
