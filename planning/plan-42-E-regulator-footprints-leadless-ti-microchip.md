# plan-42-E: Footprints — leadless and chip-scale packages, Texas Instruments and Microchip

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-D

E draws the land patterns of the leadless and chip-scale packages of the TI
and Microchip `add` rows — SON / DFN / WSON / VSON / X2SON, QFN / VQFN /
UQFN, MLF / TMLF (Micrel), DSBGA / WLCSP — from the package drawings and
recommended land patterns of the datasheets plan-42-A placed, by D's rules
(plan-42-D § 3). It writes `planning/plan-42-data/footprints-E.json` and E's
rows of `footprint-map.tsv`. The behavioral outcome: every E line of
`footprints-needed.txt` has a footprint id, and the new objects load
through `footprint_check`.

References:

- plan-42-D § 3 (ids, reuse, shared ids, ready objects, map, style) and its
  tools (`fp-load.sh`, `fp-merge.py`, `fp-check.py`).
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `grid` for ball
  arrays, `explicit`, `"centre"`).
- Library precedents: `WSON-6-NGF`, `SON-8-DRB`, `X2SON-5-DPW`, `VSON-10-DRC`,
  `VQFN-16-RGT`, `UQFN-8`, `DSBGA-4-YFP`, `DSBGA-5-YZP`, `DSBGA-6-YZP`.

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-D complete | `ls planning/completed/plan-42-D-*.md` → exists | NOT MET (2026-11-15: not started) |
| `footprints-needed.txt` is final (B Phase 5) | `grep -c '^E' planning/plan-42-data/footprints-needed.txt` → B's recorded E count (63 at planning) | NOT MET (2026-11-15: the planning file) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-42-data/census/fp-check.py E` → `<n> lines, 0
  without a footprint` (63 at planning).
- `planning/plan-42-data/census/fp-load.sh planning/plan-42-data/footprints-E.json`
  → `<k> new footprints: load ok`.

### Non-goals (explicit constraints)

As plan-42-D § Non-goals (no `Parts/` change, no edited footprint, no
footprint without an `add` row).

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| E lines, picked rows | 63 (169 parts): TI 32 (85 parts), Microchip 31 (84 parts) | `awk -F'\t' '!/^#/ && $1=="E"{n[$2]++; p[$2]+=$7} END{for(k in n) print k, n[k], p[k]}' planning/plan-42-data/footprints-needed.txt` |
| …with an existing footprint named by `footprint-lines.py` | 0 (its regex table has no leadless outline: the candidates are found by TI designator or by dimensions in Phase 1) | the 10th column of those lines |
| …distinct DigiKey packages | 51 | `awk -F'\t' '!/^#/ && $1=="E"{print $4}' … \| sort -u \| wc -l` |
| …lines by family | DFN / SON / MLF / TMLF 36, ball (DSBGA / WLCSP / UFBGA) 18, QFN (incl. UQFN) 9 | `awk -F'\t' '!/^#/ && $1=="E"{p=$4; f="dfn"; if(p~/BGA\|WLCSP/) f="ball"; else if(p~/QFN/) f="qfn"; print f}' … \| sort \| uniq -c` |

### Verified properties

- **TI leadless drawings carry a designator** that the library already uses
  as an id suffix (`WSON-6-NGF`, `SON-8-DRB`, `VQFN-16-RGT`, `DSBGA-5-YZP`):
  a TI line reuses one when its datasheet's package designator and land
  pattern match (D § 3.1).
- **Ball arrays are `grid` or `explicit`** with ball designators (`A1`),
  matching C's BGA pinouts (plan-42-C § 3 rule 9); `DSBGA-4-YFP` is the
  precedent.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Texas Instruments

- [ ] For each TI E line: read the datasheet's mechanical page (package
      designator, outline, recommended land pattern); reuse a library
      footprint of the same designator and pads, else draw one; map rows.
- [ ] Hand-check one object per package family (SON, QFN, DSBGA) against its
      drawing in § Corrections.

Acceptance: the TI lines have footprints that load.
  Check: `fp-check.py E` → only Microchip lines left; `fp-load.sh planning/plan-42-data/footprints-E.json` → `load ok` (est. 2 min).
Commit: —

### Phase 2 — Microchip (with Micrel)

- [ ] As Phase 1 for the Microchip lines (Microchip package codes from each
      datasheet's Product Identification System, e.g. `MAY` for MCP1700's
      2x2 DFN-6; Micrel "MLF" / "TMLF" drawings); ids `<outline>-<pins>-MCHP-<code>`.
- [ ] Hand-check one object per package family in § Corrections.

Acceptance: every E line has a footprint and the objects load.
  Check: `python3 -I planning/plan-42-data/census/fp-check.py E` → `63 lines, 0 without a footprint` (B's count); `planning/plan-42-data/census/fp-load.sh planning/plan-42-data/footprints-E.json` → `<k> new footprints: load ok` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh` on the new objects.
- Coverage check: `fp-check.py E` joins every E line to an id.
- Runtime proof: plan-42-I.
- Doc sync: none.
- Final gate: plan-42-J.

## Decisions

As plan-42-D § Decisions.

## Corrections

## Summary

Leadless packages are where pad mistakes hide (exposed pads, pull-back,
corner pins). The datasheets' own recommended land patterns are the source,
and one hand check per family per maker guards them.
