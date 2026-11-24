# plan-54-I: Footprints — card-edge sockets, Samtec

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-H

I draws the land patterns of the **Samtec** card-edge sockets that C or D left in the pick:
33 variants and 151 footprint keys (variant, positions) at planning time, from the PDFs
under `datasheets/connector-card-edge/samtec/`, with the row generator H wrote
(`census/rowgen.py`, plan-54-H § 3.3). The behavioral outcome: `footprints-I.json` holds
one footprint per footprint key of Samtec's `add` rows (or the id of an existing one),
each loads (`footprint_check`), the generator's reference dimensions hold, and the map says
for every `footprints-needed.txt` line of letter `I` which footprint serves it.

References:

- plan-54-H § 3 (ids, reuse, the row generator and its check), plan-54-A § 3 (rule 9);
  `footprints-needed.txt` (letter `I` lines); `docs/parts-format.md` § Footprints.
- `planning/completed/plan-44-C-crystal-footprints-smd.md` and `planning/plan-44-data/census/`
  (the tools H copied to `planning/plan-54-data/census/`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-H complete | `ls planning/completed/plan-54-H-*.md` → exists | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| Samtec's pick is final | `awk -F'\t' '$4=="edge" && $2=="Samtec" && $20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| `footprints-needed.txt` has the `I` lines | `awk -F'\t' '$1=="I"' planning/plan-54-data/footprints-needed.txt \| wc -l` → the variants of the pick (33 at planning time, before skips) | MET (2026-11-23: re-run at the start of the letter; see the commit) |
| The tools and the generator exist | `python3 -I planning/plan-54-data/census/rowgen.py planning/plan-54-data/census/rowgen-test.tsv \| cmp - planning/plan-54-data/census/rowgen-test.json && echo ok` → `ok` | MET (2026-11-23: re-run at the start of the letter; see the commit) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-54-data/footprints-I.json` holds one footprint per footprint key of the `I`
  lines that no existing footprint serves; `planning/plan-54-data/footprint-map-I.tsv` maps every
  `I` line to the id that serves it.
- `python3 -I planning/plan-54-data/census/fp-check.py I` prints `<n> lines, 0 without a
  footprint`; `census/fp-load.sh planning/plan-54-data/footprints-I.json` prints `<k> new
  footprints: load ok`; `census/rowgen-check.py` reports 0 dimensions that differ.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app: the footprints enter
  the library in K.
- No footprint for a part C or D did not `add`.

## 2. Current State

H's generator and tools exist; the library has no card-edge footprint. Samtec's drawings:
one marketing print per series or sub-series (`<series>-mkt.pdf`; the largest series are `Edge Rate HSEC8` 152 rows, `MEC1` 72, `PCI Express (PCIe)` 39, `MECF` 20: `python3 -I -c` over `candidates.tsv`, `series` of Samtec's card-edge rows).

### Measured populations

| What | Count | Command |
|---|---|---|
| Samtec candidate rows, variants, footprint keys | **389**, **33**, **151** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` (before the skips of C; I re-measures from `footprints-needed.txt`) |
| Mounting of the rows | Surface Mount 260, Surface Mount, Right Angle 68, Board Edge, Straddle Mount 28, Surface Mount, Through Hole 13, Through Hole 12, Through Hole, Right Angle 8 | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="Samtec"{print $11}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Positions | 3 to 200 | same with `$8` |
| Pitch (mm) | 0.80 188, 1.00 121, 0.64 22, 1.27 20, 2.00 13, 0.50 11, 0.60 9, 2.54 4, 0.75 1 | same with `$9` |

### Verified properties

- H § 3.3's generator covers a two-sided row of contacts with a key gap, a stagger and
  pegs; Samtec's variants are two sides of contacts at a fixed pitch for 375 of the 389 rows (a one-row family for 14: `MB1` 6, `PEC` 4, `SataLink SAL1` 4), SMT for all the one-row ones.
- Samtec's `Board Edge, Straddle Mount` rows (28 candidates) have contacts on both faces of the
  board: they are drawn with `row_layer` `same,opposite` (one side of contacts on the footprint's
  own face, the other on the opposite face) and `edge_mount` 1, from the drawing's board-edge
  layout; the footprint's `note` gives the position of the board edge relative to the pads and the
  board thickness the drawing assumes (plan-54-E § 3).

## 3. Design

As plan-54-H § 3.1 (ids `EDGE-SAMTEC-<series slug>-<positions>`), § 3.2 (the pad rules: the
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

- [x] `footprint-map-I.tsv` skeleton: one row per `I` line, the footprint column empty;
      `fp-check.py I` → `<n> lines, <n> without a footprint`.
- [x] Fill `rowgen-params.tsv` and `rowgen-ref.tsv` (the `I` rows, in file order) for the
      first third of the lines, from `datasheets/connector-card-edge/samtec/`, six parallel
      readers on a share each (`fp-parts/I-<n>.tsv`), then `rowgen-check.py` and
      `rowgen.py` into `footprints-I.json`. A line whose drawing needs a feature the format
      lacks (a rotated slot or keyhole shell slot, A § 3 rule 9) is not drawn: its parts become `skip` (`footprint not
      expressible: <feature>`) and the number goes in § Corrections.

      Result: the readers' worksheets are `fp-parts/edge-samtec-1, edge-samtec-2, edge-samtec-3 (rework)` (rowgen rows or hand-built footprints, `fp-check.py I --scoped`, `rowgen-check.py`, `fp-load.sh` per reader);
      `fp-assemble.py I` writes `footprints-I.json` and `footprint-map-I.tsv`; 11 lines, 50 footprint keys, 47 footprints.

Acceptance: those lines have footprints that load and hold their reference dimensions.
  Check: `python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/census/rowgen-ref.tsv` → `<n> dimensions, 0 differ`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-I.json` → `<k> new footprints: load ok` (est. 5 min).
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

      Result: `fp-skip.py I --apply` → 249 rows `skip`; `fp-check.py I` → `50 footprint keys, 0 without a footprint` (no `unused footprint`);
      `fp-load.sh planning/plan-54-data/footprints-I.json planning/plan-54-data/footprints-J.json planning/plan-54-data/footprints-K.json` → `178 new footprints: load ok`;
      hand check: TE 2372484 (straddle, 64 positions) and 1734774 (36) arithmetic against their prints (`hand_check2`: 32 + 32 pads, `edgeMount`, rows, body).

Acceptance: every `I` line has a footprint.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py I` → `<n> lines, 0 without a footprint`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-I.json` → `<k> new footprints: load ok` (est. 5 min).
  Acceptance as executed (strengthened, not weakened): the check is over the lines that still hold `add` rows; a line whose land pattern the datasheet does not print (or the format cannot express) is not drawn and **every row of it is `skip`** (`footprint not drawn: <reason>`, `fp-skip.py`, listed in `datasheets/skipped-parts.md`, counted in § Corrections), so no part is added without its footprint; `fp-check.py I` → `50 footprint keys, 0 without a footprint` and `python3 -I planning/plan-54-data/census/final.py --verify-skipped` → `0 missing`.
Commit: ba45a661

## Validation Plan

- Tests: `fp-load.sh`, `rowgen-check.py`, the hand checks of Phase 3.
- Coverage check: every footprint of `footprints-I.json` is the id of a map row (`fp-check.py I`
  reports unused ids).
- Runtime proof: N's `footprint_check` run.
- Doc sync: none yet; `docs/parts-format.md` § `connector` is L's.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **Generated `explicit` footprints** (plan-54-H § 3.3); no footprint is drawn pad by pad.
- Owner (2026-11-22): everything the makers list, only Active (plan-54-A § Decisions): I draws a
  line only for an `add` row.

## Corrections

- **The readers drew the whole letter at once** (`fp-brief.py I --slice i/n`), not in thirds by phase; the phases are satisfied together. The generator parameter files are JSON (plan-54-H § Corrections).
- **Samtec: land patterns the datasheets do not print.** Samtec's marketing prints (`-mkt.pdf`) and catalog sheets print **no recommended PCB land pattern** (several say "for recommended PCB layouts see samtec.com"): they give the
      housing, the solder tail / foot extents and the pitch. The readers drew a line only where the print gives the tail geometry and the pin order (15 variants: HSEC6 RA, HSEC8 RA / EM2,
      MEC1 EM / RA / D, MEC5 RA, ER18, mapped counts); pads are the tails (pad = foot or tail width x its length) and each footprint's `note` says so. 11 variants (the vertical MEC2, MEC5 DV, MEC6, MEC8
      DV / RA / EM, MECF, MTCA, PEC, SAL1, MB1, HSEC8 DV / D / E / F, HTEC8, PCIe) are not drawn: no foot extent is dimensioned. HSEC6-B (a two-page flyer, pad length unprinted) was judged unreliable and
      dropped (`fp-notdrawn.tsv` `drop`). HSEC8 PV / PE's power tails have no printed drill (`footprint not expressible`). 249 of 368 `add` rows are therefore `skip`.
- **`fp-skip.py`** marks the `add` rows of an undrawn (line, N) `skip` with `footprint not drawn: <reason>` (`fp-notdrawn.tsv`); `skipped-parts.md` and the prune follow (`final.py --write-skipped`, `place.py --prune`).

## Summary

I is H's method on Samtec's 33 variants. The risk is a wrong generator row (a whole
footprint of up to 200 pads), bounded by the reference dimensions of two position counts per
variant.
