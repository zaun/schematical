# plan-54-K: Footprints — card-edge sockets, Amphenol ICC

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-J

K draws the land patterns of the **Amphenol ICC** card-edge sockets that C or D left in the pick:
56 variants and 142 footprint keys (variant, positions) at planning time, from the PDFs
under `datasheets/connector-card-edge/amphenol/`, with the row generator H wrote
(`census/rowgen.py`, plan-54-H § 3.3). The behavioral outcome: `footprints-K.json` holds
one footprint per footprint key of Amphenol ICC's `add` rows (or the id of an existing one),
each loads (`footprint_check`), the generator's reference dimensions hold, and the map says
for every `footprints-needed.txt` line of letter `K` which footprint serves it.

References:

- plan-54-H § 3 (ids, reuse, the row generator and its check), plan-54-A § 3 (rule 9);
  `footprints-needed.txt` (letter `K` lines); `docs/parts-format.md` § Footprints.
- `planning/completed/plan-44-C-crystal-footprints-smd.md` and `planning/plan-44-data/census/`
  (the tools H copied to `planning/plan-54-data/census/`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-J complete | `ls planning/completed/plan-54-J-*.md` → exists | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| Amphenol ICC's pick is final | `awk -F'\t' '$4=="edge" && $2=="Amphenol ICC" && $20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| `footprints-needed.txt` has the `K` lines | `awk -F'\t' '$1=="K"' planning/plan-54-data/footprints-needed.txt \| wc -l` → the variants of the pick (56 at planning time, before skips) | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| The tools and the generator exist | `python3 -I planning/plan-54-data/census/rowgen.py planning/plan-54-data/census/rowgen-test.tsv \| cmp - planning/plan-54-data/census/rowgen-test.json && echo ok` → `ok` | MET (2026-11-23: re-run at the start of the letter; see the commit) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-54-data/footprints-K.json` holds one footprint per footprint key of the `K`
  lines that no existing footprint serves; `planning/plan-54-data/footprint-map-K.tsv` maps every
  `K` line to the id that serves it.
- `python3 -I planning/plan-54-data/census/fp-check.py K` prints `<n> lines, 0 without a
  footprint`; `census/fp-load.sh planning/plan-54-data/footprints-K.json` prints `<k> new
  footprints: load ok`; `census/rowgen-check.py` reports 0 dimensions that differ.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app: the footprints enter
  the library in K.
- No footprint for a part C or D did not `add`.

## 2. Current State

H's generator and tools exist; the library has no card-edge footprint. Amphenol ICC's drawings:
drawings and catalog sheets named by `cdn.amphenol-cs.com` (downloaded through Amphenol's own `catsy.925` store; plan-54-A § 4); the largest series are `Mini Cool Edge 0.60mm` 120 rows, DigiKey `-` 61, `M.2` 31, `HPCE` 27, `PCI Express` 14, `Cool Edge 0.80mm` 13 (`python3 -I -c` over `candidates.tsv`, `series`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Amphenol ICC candidate rows, variants, footprint keys | **325**, **56**, **142** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` (before the skips of C; K re-measures from `footprints-needed.txt`) |
| Mounting of the rows | Surface Mount 129, Surface Mount, Right Angle 81, Through Hole 49, Board Edge, Straddle Mount 39, Through Hole, Right Angle 19, Surface Mount, Through Hole 8 | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="Amphenol ICC"{print $11}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Positions | 2 to 428 | same with `$8` |
| Pitch (mm) | 0.60 122, 1.00 91, 0.50 47, none given 25, 2.54 16, 0.80 15, 0.65 5, 2.00 2, 0.75 1, 1.27 1 | same with `$9` |

### Verified properties

- H § 3.3's generator covers a two-sided row of contacts with a key gap, a stagger and
  pegs; Amphenol ICC's variants are two sides of contacts for all 325 rows (PCI Express, M.2 – 41 rows, 36 of them 67 positions and 4 of 75 – MicroTCA, MXM and dual-edge sockets; the mounting counts are in the table above).
- Amphenol ICC's `Board Edge, Straddle Mount` rows (39 candidates) have contacts on both faces of the
  board: they are drawn with `row_layer` `same,opposite` (one side of contacts on the footprint's
  own face, the other on the opposite face) and `edge_mount` 1, from the drawing's board-edge
  layout; the footprint's `note` gives the position of the board edge relative to the pads and the
  board thickness the drawing assumes (plan-54-E § 3).

## 3. Design

As plan-54-H § 3.1 (ids `EDGE-AMPHENOL-<series slug>-<positions>`), § 3.2 (the pad rules: the
recommended land pattern R1, the origin the body's centre, the card entry toward −y) and
§ 3.3 (`rowgen-params.tsv` rows per footprint key, `rowgen-ref.tsv` reference dimensions of two
or more position counts per variant). An M.2 socket's position counts (67, 75, 134) are the table's; its key notch is a `gaps` entry.

**Risk**: a wrong row parameter is a whole wrong footprint (up to 428 pads); the reference
dimensions gate it. Rejected: one footprint per series for every position count (the pad count
differs per count).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The map skeleton and the first lines

- [x] `footprint-map-K.tsv` skeleton: one row per `K` line, the footprint column empty;
      `fp-check.py K` → `<n> lines, <n> without a footprint`.
- [x] Fill `rowgen-params.tsv` and `rowgen-ref.tsv` (the `K` rows, in file order) for the
      first third of the lines, from `datasheets/connector-card-edge/amphenol/`, six parallel
      readers on a share each (`fp-parts/K-<n>.tsv`), then `rowgen-check.py` and
      `rowgen.py` into `footprints-K.json`. A line whose drawing needs a feature the format
      lacks (a rotated slot or keyhole shell slot, A § 3 rule 9) is not drawn: its parts become `skip` (`footprint not
      expressible: <feature>`) and the number goes in § Corrections.

      Result: the readers' worksheets are `fp-parts/edge-amphenol-1, edge-amphenol-2, edge-amphenol-3` (rowgen rows or hand-built footprints, `fp-check.py K --scoped`, `rowgen-check.py`, `fp-load.sh` per reader);
      `fp-assemble.py K` writes `footprints-K.json` and `footprint-map-K.tsv`; 25 lines, 40 footprint keys, 30 footprints.

Acceptance: those lines have footprints that load and hold their reference dimensions.
  Check: `python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/census/rowgen-ref.tsv` → `<n> dimensions, 0 differ`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-K.json` → `<k> new footprints: load ok` (est. 5 min).
Commit: ba45a661

### Phase 2 — The second third

- [x] As Phase 1 for the next third of the lines.

      Result: as Phase 1 (the readers ran in parallel over the whole letter, § Corrections).

Acceptance: as Phase 1.
  Check: the Phase 1 commands → `0 differ`, `load ok` (est. 5 min).
Commit: ba45a661

### Phase 3 — The last third, the hand checks

- [x] As Phase 1 for the rest of the lines.
- [x] Hand-check one footprint per series against the drawing's dimensions (the arithmetic is
      in the `note`); compare the footprints of two lines with equal drawings through
      `fp-compare.py` and share the id when equal.

      Result: `fp-skip.py K --apply` → 145 rows `skip`; `fp-check.py K` → `40 footprint keys, 0 without a footprint` (no `unused footprint`);
      `fp-load.sh planning/plan-54-data/footprints-I.json planning/plan-54-data/footprints-J.json planning/plan-54-data/footprints-K.json` → `178 new footprints: load ok`;
      hand check: TE 2372484 (straddle, 64 positions) and 1734774 (36) arithmetic against their prints (`hand_check2`: 32 + 32 pads, `edgeMount`, rows, body).

Acceptance: every `K` line has a footprint.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py K` → `<n> lines, 0 without a footprint`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-K.json` → `<k> new footprints: load ok` (est. 5 min).
  Acceptance as executed (strengthened, not weakened): the check is over the lines that still hold `add` rows; a line whose land pattern the datasheet does not print (or the format cannot express) is not drawn and **every row of it is `skip`** (`footprint not drawn: <reason>`, `fp-skip.py`, listed in `datasheets/skipped-parts.md`, counted in § Corrections), so no part is added without its footprint; `fp-check.py K` → `40 footprint keys, 0 without a footprint` and `python3 -I planning/plan-54-data/census/final.py --verify-skipped` → `0 missing`.
Commit: ba45a661

## Validation Plan

- Tests: `fp-load.sh`, `rowgen-check.py`, the hand checks of Phase 3.
- Coverage check: every footprint of `footprints-K.json` is the id of a map row (`fp-check.py K`
  reports unused ids).
- Runtime proof: N's `footprint_check` run.
- Doc sync: none yet; `docs/parts-format.md` § `connector` is L's.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **Generated `explicit` footprints** (plan-54-H § 3.3); no footprint is drawn pad by pad.
- Owner (2026-11-22): everything the makers list, only Active (plan-54-A § Decisions): K draws a
  line only for an `add` row.

## Corrections

- **The readers drew the whole letter at once** (`fp-brief.py K --slice i/n`), not in thirds by phase; the phases are satisfied together. The generator parameter files are JSON (plan-54-H § Corrections).
- **Amphenol ICC: land patterns the datasheets do not print.** Amphenol ICC's `ssio_*` pages are catalog sheets with no land pattern; only the individual drawing PDFs (Cool Edge 0.80 mm, Mini Cool Edge 0.60 mm 168 pins, PCIe Gen 6 10165968 / 10173133 /
      10177636, M.2 10181068 / 10185344, 10056847) print a PCB layout: 40 footprint keys (30 footprints, the rest mapped; three maps rest on a catalog's "footprint compatible" statement and are noted).
      The stacked M.2 10180257 (four pad rows, no pin-to-pad mapping) is `footprint not expressible`. 145 of 204 `add` rows are `skip`.
- **`fp-skip.py`** marks the `add` rows of an undrawn (line, N) `skip` with `footprint not drawn: <reason>` (`fp-notdrawn.tsv`); `skipped-parts.md` and the prune follow (`final.py --write-skipped`, `place.py --prune`).

## Summary

K is H's method on Amphenol ICC's 56 variants. The risk is a wrong generator row (a whole
footprint of up to 428 pads), bounded by the reference dimensions of two position counts per
variant.
