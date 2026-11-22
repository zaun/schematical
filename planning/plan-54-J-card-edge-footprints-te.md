# plan-54-J: Footprints — card-edge sockets, TE Connectivity

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-I

J draws the land patterns of the **TE Connectivity** card-edge sockets that C or D left in the pick:
43 variants and 115 footprint keys (variant, positions) at planning time, from the PDFs
under `datasheets/connector-card-edge/te/`, with the row generator H wrote
(`census/rowgen.py`, plan-54-H § 3.3). The behavioral outcome: `footprints-J.json` holds
one footprint per footprint key of TE Connectivity's `add` rows (or the id of an existing one),
each loads (`footprint_check`), the generator's reference dimensions hold, and the map says
for every `footprints-needed.txt` line of letter `J` which footprint serves it.

References:

- plan-54-H § 3 (ids, reuse, the row generator and its check), plan-54-A § 3 (rule 9);
  `footprints-needed.txt` (letter `J` lines); `docs/parts-format.md` § Footprints.
- `planning/completed/plan-44-C-crystal-footprints-smd.md` and `planning/plan-44-data/census/`
  (the tools H copied to `planning/plan-54-data/census/`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-I complete | `ls planning/completed/plan-54-I-*.md` → exists | NOT MET (2026-11-22: not started) |
| TE Connectivity's pick is final | `awk -F'\t' '$4=="edge" && $2=="TE Connectivity" && $20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | NOT MET (2026-11-22: 300 candidates; D) |
| `footprints-needed.txt` has the `J` lines | `awk -F'\t' '$1=="J"' planning/plan-54-data/footprints-needed.txt \| wc -l` → the variants of the pick (43 at planning time, before skips) | NOT MET (2026-11-22: D writes it) |
| The tools and the generator exist | `python3 -I planning/plan-54-data/census/rowgen.py planning/plan-54-data/census/rowgen-test.tsv \| cmp - planning/plan-54-data/census/rowgen-test.json && echo ok` → `ok` | NOT MET (2026-11-22: H Phase 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-54-data/footprints-J.json` holds one footprint per footprint key of the `J`
  lines that no existing footprint serves; `planning/plan-54-data/footprint-map-J.tsv` maps every
  `J` line to the id that serves it.
- `python3 -I planning/plan-54-data/census/fp-check.py J` prints `<n> lines, 0 without a
  footprint`; `census/fp-load.sh planning/plan-54-data/footprints-J.json` prints `<k> new
  footprints: load ok`; `census/rowgen-check.py` reports 0 dimensions that differ.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app: the footprints enter
  the library in K.
- No footprint for a part C or D did not `add`.

## 2. Current State

H's generator and tools exist; the library has no card-edge footprint. TE Connectivity's drawings:
one customer drawing, data sheet or catalog sheet per part or small group (230 specs rows; the rows by document type are in plan-54-D § 2); a drawing that has a PCB layout is drawn from it, one that has none is drawn by the outline rules R2 – R6.

### Measured populations

| What | Count | Command |
|---|---|---|
| TE Connectivity candidate rows, variants, footprint keys | **300**, **43**, **115** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` (before the skips of D; J re-measures from `footprints-needed.txt`) |
| Mounting of the rows | Through Hole 120, Surface Mount 82, Surface Mount, Right Angle 43, Board Edge, Straddle Mount 32, Through Hole, Right Angle 22, - 1 | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="TE Connectivity"{print $11}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Positions | 4 to 340 | same with `$8` |
| Pitch (mm) | 1.00 122, 2.54 55, 0.60 44, 1.27 30, 0.50 16, none given 13, 0.80 10, 12.90 7, 3.96 2, 2.50 1 | same with `$9` |

### Verified properties

- H § 3.3's generator covers a two-sided row of contacts with a key gap, a stagger and
  pegs; TE Connectivity's variants are two sides of contacts for 298 of the 300 rows (PCI, PCI Express, M.2 and dual-edge sockets: `awk` over `candidates.tsv`, `rows`; the mounting counts are in the table above).
- TE Connectivity's `Board Edge, Straddle Mount` rows (32 candidates) have contacts on both faces of the
  board: they are drawn with `row_layer` `same,opposite` (one side of contacts on the footprint's
  own face, the other on the opposite face) and `edge_mount` 1, from the drawing's board-edge
  layout; the footprint's `note` gives the position of the board edge relative to the pads and the
  board thickness the drawing assumes (plan-54-E § 3).

## 3. Design

As plan-54-H § 3.1 (ids `EDGE-TE-<series slug>-<positions>`), § 3.2 (the pad rules: the
recommended land pattern R1, the origin the body's centre, the card entry toward −y) and
§ 3.3 (`rowgen-params.tsv` rows per footprint key, `rowgen-ref.tsv` reference dimensions of two
or more position counts per variant). A drawing that serves several position counts is one parameter row per count with the same pitch and gap rule; a key gap of a PCI or PCI Express socket is a `gaps` entry.

**Risk**: a wrong row parameter is a whole wrong footprint (up to 340 pads); the reference
dimensions gate it. Rejected: one footprint per series for every position count (the pad count
differs per count).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The map skeleton and the first lines

- [ ] `footprint-map-J.tsv` skeleton: one row per `J` line, the footprint column empty;
      `fp-check.py J` → `<n> lines, <n> without a footprint`.
- [ ] Fill `rowgen-params.tsv` and `rowgen-ref.tsv` (the `J` rows, in file order) for the
      first third of the lines, from `datasheets/connector-card-edge/te/`, six parallel
      readers on a share each (`fp-parts/J-<n>.tsv`), then `rowgen-check.py` and
      `rowgen.py` into `footprints-J.json`. A line whose drawing needs a feature the format
      lacks (a rotated slot or keyhole shell slot, A § 3 rule 9) is not drawn: its parts become `skip` (`footprint not
      expressible: <feature>`) and the number goes in § Corrections.

Acceptance: those lines have footprints that load and hold their reference dimensions.
  Check: `python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/census/rowgen-ref.tsv` → `<n> dimensions, 0 differ`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-J.json` → `<k> new footprints: load ok` (est. 5 min).
Commit: —

### Phase 2 — The second third

- [ ] As Phase 1 for the next third of the lines.

Acceptance: as Phase 1.
  Check: the Phase 1 commands → `0 differ`, `load ok` (est. 5 min).
Commit: —

### Phase 3 — The last third, the hand checks

- [ ] As Phase 1 for the rest of the lines.
- [ ] Hand-check one footprint per series against the drawing's dimensions (the arithmetic is
      in the `note`); compare the footprints of two lines with equal drawings through
      `fp-compare.py` and share the id when equal.

Acceptance: every `J` line has a footprint.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py J` → `<n> lines, 0 without a footprint`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-J.json` → `<k> new footprints: load ok` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `fp-load.sh`, `rowgen-check.py`, the hand checks of Phase 3.
- Coverage check: every footprint of `footprints-J.json` is the id of a map row (`fp-check.py J`
  reports unused ids).
- Runtime proof: N's `footprint_check` run.
- Doc sync: none yet; `docs/parts-format.md` § `connector` is L's.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **Generated `explicit` footprints** (plan-54-H § 3.3); no footprint is drawn pad by pad.
- Owner (2026-11-22): everything the makers list, only Active (plan-54-A § Decisions): J draws a
  line only for an `add` row.

## Corrections

## Summary

J is H's method on TE Connectivity's 43 variants. The risk is a wrong generator row (a whole
footprint of up to 340 pads), bounded by the reference dimensions of two position counts per
variant.
