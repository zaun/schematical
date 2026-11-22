# plan-54-F: Footprints — card-edge sockets, Samtec

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-E

F draws the land patterns of the **Samtec** card-edge sockets that C or D left in the pick:
33 variants and 151 footprint keys (variant, positions) at planning time, from the PDFs
under `datasheets/connector-card-edge/samtec/`, with the row generator E wrote
(`census/rowgen.py`, plan-54-E § 3.3). The behavioral outcome: `footprints-F.json` holds
one footprint per footprint key of Samtec's `add` rows (or the id of an existing one),
each loads (`footprint_check`), the generator's reference dimensions hold, and the map says
for every `footprints-needed.txt` line of letter `F` which footprint serves it.

References:

- plan-54-E § 3 (ids, reuse, the row generator and its check), plan-54-A § 3 (rule 9);
  `footprints-needed.txt` (letter `F` lines); `docs/parts-format.md` § Footprints.
- `planning/completed/plan-44-C-crystal-footprints-smd.md` and `planning/plan-44-data/census/`
  (the tools E copied to `planning/plan-54-data/census/`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-E complete | `ls planning/completed/plan-54-E-*.md` → exists | NOT MET (2026-11-22: not started) |
| Samtec's pick is final | `awk -F'\t' '$4=="edge" && $2=="Samtec" && $20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | NOT MET (2026-11-22: 389 candidates; C) |
| `footprints-needed.txt` has the `F` lines | `awk -F'\t' '$1=="F"' planning/plan-54-data/footprints-needed.txt \| wc -l` → the variants of the pick (33 at planning time, before skips) | NOT MET (2026-11-22: C writes it) |
| The tools and the generator exist | `python3 -I planning/plan-54-data/census/rowgen.py planning/plan-54-data/census/rowgen-test.tsv \| cmp - planning/plan-54-data/census/rowgen-test.json && echo ok` → `ok` | NOT MET (2026-11-22: E Phase 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-54-data/footprints-F.json` holds one footprint per footprint key of the `F`
  lines that no existing footprint serves; `planning/plan-54-data/footprint-map-F.tsv` maps every
  `F` line to the id that serves it.
- `python3 -I planning/plan-54-data/census/fp-check.py F` prints `<n> lines, 0 without a
  footprint`; `census/fp-load.sh planning/plan-54-data/footprints-F.json` prints `<k> new
  footprints: load ok`; `census/rowgen-check.py` reports 0 dimensions that differ.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app: the footprints enter
  the library in K.
- No footprint for a part C or D did not `add`.

## 2. Current State

E's generator and tools exist; the library has no card-edge footprint. Samtec's drawings:
one marketing print per series or sub-series (`<series>-mkt.pdf`; the largest series are `Edge Rate HSEC8` 152 rows, `MEC1` 72, `PCI Express (PCIe)` 39, `MECF` 20: `python3 -I -c` over `candidates.tsv`, `series` of Samtec's card-edge rows).

### Measured populations

| What | Count | Command |
|---|---|---|
| Samtec candidate rows, variants, footprint keys | **389**, **33**, **151** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` (before the skips of C; F re-measures from `footprints-needed.txt`) |
| Mounting of the rows | Surface Mount 260, Surface Mount, Right Angle 68, Board Edge, Straddle Mount 28, Surface Mount, Through Hole 13, Through Hole 12, Through Hole, Right Angle 8 | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="Samtec"{print $11}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Positions | 3 to 200 | same with `$8` |
| Pitch (mm) | 0.80 188, 1.00 121, 0.64 22, 1.27 20, 2.00 13, 0.50 11, 0.60 9, 2.54 4, 0.75 1 | same with `$9` |

### Verified properties

- E § 3.3's generator covers a two-sided row of contacts with a key gap, a stagger and
  pegs; Samtec's variants are two sides of contacts at a fixed pitch for 375 of the 389 rows (a one-row family for 14: `MB1` 6, `PEC` 4, `SataLink SAL1` 4), SMT for all the one-row ones.
- Samtec's `Board Edge, Straddle Mount` rows (28 candidates) need pads on both faces of the
  board and were `skip`ped by C (A § 3 rule 9); no `F` line draws one.

## 3. Design

As plan-54-E § 3.1 (ids `EDGE-SAMTEC-<series slug>-<positions>`), § 3.2 (the pad rules: the
recommended land pattern R1, the origin the body's centre, the card entry toward −y) and
§ 3.3 (`rowgen-params.tsv` rows per footprint key, `rowgen-ref.tsv` reference dimensions of two
or more position counts per variant). A one-row family is `single`; a two-row family is `odd-even` or `halves` as the print numbers its contacts.

**Risk**: a wrong row parameter is a whole wrong footprint (up to 200 pads); the reference
dimensions gate it. Rejected: one footprint per series for every position count (the pad count
differs per count).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The map skeleton and the first lines

- [ ] `footprint-map-F.tsv` skeleton: one row per `F` line, the footprint column empty;
      `fp-check.py F` → `<n> lines, <n> without a footprint`.
- [ ] Fill `rowgen-params.tsv` and `rowgen-ref.tsv` (the `F` rows, in file order) for the
      first third of the lines, from `datasheets/connector-card-edge/samtec/`, six parallel
      readers on a share each (`fp-parts/F-<n>.tsv`), then `rowgen-check.py` and
      `rowgen.py` into `footprints-F.json`. A line whose drawing needs a feature the format
      lacks (A § 3 rule 9) is not drawn: its parts become `skip` (`footprint not
      expressible: <feature>`) and the number goes in § Corrections.

Acceptance: those lines have footprints that load and hold their reference dimensions.
  Check: `python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/census/rowgen-ref.tsv` → `<n> dimensions, 0 differ`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-F.json` → `<k> new footprints: load ok` (est. 5 min).
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

Acceptance: every `F` line has a footprint.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py F` → `<n> lines, 0 without a footprint`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-F.json` → `<k> new footprints: load ok` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `fp-load.sh`, `rowgen-check.py`, the hand checks of Phase 3.
- Coverage check: every footprint of `footprints-F.json` is the id of a map row (`fp-check.py F`
  reports unused ids).
- Runtime proof: K's `footprint_check` run.
- Doc sync: none yet; `docs/parts-format.md` § `connector` is I's.
- Final gate: none; the landing tier is M's.

## Decisions

- Author: **Generated `explicit` footprints** (plan-54-E § 3.3); no footprint is drawn pad by pad.
- Owner (2026-11-22): everything the makers list, only Active (plan-54-A § Decisions): F draws a
  line only for an `add` row.

## Corrections

## Summary

F is E's method on Samtec's 33 variants. The risk is a wrong generator row (a whole
footprint of up to 200 pads), bounded by the reference dimensions of two position counts per
variant.
