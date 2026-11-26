# plan-74-I: MagJacks, TE Connectivity: pinouts, footprints, parts, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-H

I adds TE's in-scope RJ45 jacks with magnetics (at most 120, plan-74-A § 2), with G's rules and
generator. A TE part's PDF is its own customer drawing (plan-74-A § 3.4), which has a "SUGGESTED PCB
LAYOUT" and the board-side pins `P1 … Pn` (the planning probe of `5-2301995-2`: 3 pages). TE's
application spec 114-94447 (27 pages) says the P pins' functions differ by part and are on each
drawing. Its figures 5–17 give the hole patterns of the single-port jacks: tab up, tab down and
vertical, with and without LEDs.

References:

- plan-74-G § 3 (pins, footprints, generator), plan-74-E § 3.2.
- TE 114-94447 (`datasheets/connector-rj45/te/114-94447.pdf`, which A downloads with the drawings):
  § 3.4 (P / J numbering), § 3.5 (PCB layout figures).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-H complete | `ls planning/completed/plan-74-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |
| Goldens at I's start | as plan-74-H's row | I records its own |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every TE `add` row is one `rj45-te-` part, with pins from its customer drawing's schematic and a
  footprint from its suggested PCB layout.
- `footprints-rj45-te.json` holds one footprint per distinct layout (`RJ45-TE-<layout>`: drawings
  that print the same layout share it; two layouts are the same when every hole's position and
  diameter match). The footprints are merged.
- Every other block of `connector.json` is unchanged. The goldens only grow.

### Non-goals (explicit constraints)

- As plan-74-G § Non-goals.

## 2. Current State

H's generator writes the Bel and Pulse devices. The TE customer drawings are in
`datasheets/connector-rj45/te/` as `<pn>-cd.pdf` (A).

## 3. Design

G's § 3 unchanged. TE specifics:

- Pin numbers: P1 … Pn → 1 … n; names from the drawing's schematic (the PHY-side labels).
  `SHIELD` last.
- Where a drawing's layout and 114-94447's figure for the same configuration differ, the drawing
  wins (114-94447 § 3.5: "the customer drawing takes priority"). The `note` names both.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Drawings read

- [ ] `planning/plan-74-data/rj45-te.json` and the TE rows of `specs-rj45.tsv`.
- [ ] `planning/plan-74-data/footprints-rj45-te.json`.

Acceptance: every footprint loads.
  Check: `planning/plan-74-data/census/fp-load.sh planning/plan-74-data/footprints-rj45-te.json` → exit 0, only `extra footprint` / `unused footprint` lines for the new ids (est. 5 min).
  Result:
Commit:

### Phase 2 — Parts, goldens

- [ ] `gen-rj45.py` writes the TE devices; `fp-merge.py`; `datasheets.json` (the customer-drawing
      URLs of plan-74-A § 3.4); goldens regenerated and reviewed.

Acceptance: `connector.json` holds every jack `add` row, and the suite passes.
  Check: `cmake --build /tmp/logic-p74 -j8 && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed`; `grep -c '|connector|.*rj45=' backend/tests/golden/parts.txt` equals `awk -F'\t' 'NR>1 && $1=="magjack" && $18=="add"' planning/plan-74-data/candidates.tsv | wc -l`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `gen-rj45.py --check` → exit 0 (est. 6 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks, and a 1:1 hand check of three footprints recorded in Phase 1's Result.
- Final gate: plan-74-K.

## Decisions

- Author: **the customer drawing is TE's datasheet entry**, with its DDEController URL. It is
  the maker's own document for that part number, and 114-94447 defers to it.

## Corrections

## Summary

TE's jacks from their per-part customer drawings, with G's rules.
