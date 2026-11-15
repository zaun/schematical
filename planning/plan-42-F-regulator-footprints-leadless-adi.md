# plan-42-F: Footprints — leadless and chip-scale packages, Analog Devices

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-E

F draws the land patterns of the leadless and chip-scale packages of the
Analog Devices `add` rows (ADI and Maxim-origin LFCSP / DFN / TDFN, QFN /
TQFN / WQFN, WLCSP / WLP) by plan-42-D's rules. It writes
`planning/plan-42-data/footprints-F.json` and F's rows of
`footprint-map.tsv`. The behavioral outcome: every F line of
`footprints-needed.txt` has a footprint id, and the new objects load
through `footprint_check`.

References:

- plan-42-D § 3 and its tools; plan-42-E (the TI / Microchip leadless
  objects, which an ADI drawing of equal pads reuses).
- `docs/parts-format.md` § Footprints.
- ADI drawings name a package code (`CP-6-3`, `UJ-5`, `CB-4-…`) and Maxim
  ones a "Package Code" / "Land Pattern No." (`T633+2`, `90-0058`); the ids
  carry it (`LFCSP-6-ADI-CP-6-3`).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-E complete | `ls planning/completed/plan-42-E-*.md` → exists | NOT MET (2026-11-15: not started) |
| `footprints-needed.txt` is final (B Phase 5) | `grep -c '^F' planning/plan-42-data/footprints-needed.txt` → B's recorded F count (48 at planning) | NOT MET (2026-11-15: the planning file) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-42-data/census/fp-check.py F` → `<n> lines, 0
  without a footprint` (48 at planning).
- `planning/plan-42-data/census/fp-load.sh planning/plan-42-data/footprints-F.json`
  → `<k> new footprints: load ok`.

### Non-goals (explicit constraints)

As plan-42-D § Non-goals.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| F lines, picked rows | 48 (102 parts) | `awk -F'\t' '!/^#/ && $1=="F"{n++; p+=$7} END{print n, p}' planning/plan-42-data/footprints-needed.txt` |
| …distinct DigiKey packages | 33 | `awk -F'\t' '!/^#/ && $1=="F"{print $4}' … \| sort -u \| wc -l` |
| …lines by family | DFN 21, QFN 18, ball (WLCSP / WLBGA / UFBGA) 9 | the E letter's family `awk`, `$1=="F"` |
| …with a library footprint named by `footprint-lines.py` | 0 | the 10th column |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — DFN / LFCSP / TDFN

- [ ] For each F line of a DFN-type package: the datasheet's outline and land
      pattern (ADI's "Outline Dimensions" page; Maxim's package drawing by
      its code); reuse (D, E or the library) or draw; map rows.

Acceptance: those lines have footprints that load.
  Check: `fp-check.py F` → only QFN and WLCSP lines left; `fp-load.sh planning/plan-42-data/footprints-F.json` → `load ok` (est. 2 min).
Commit: —

### Phase 2 — QFN and WLCSP

- [ ] As Phase 1 for the QFN-type and the ball packages (`grid` or
      `explicit`, ball designators as C's pinouts).
- [ ] Hand-check one object per package family (DFN, QFN, WLCSP) in §
      Corrections.

Acceptance: every F line has a footprint and the objects load.
  Check: `python3 -I planning/plan-42-data/census/fp-check.py F` → `48 lines, 0 without a footprint` (B's count); `planning/plan-42-data/census/fp-load.sh planning/plan-42-data/footprints-F.json` → `<k> new footprints: load ok` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py F`.
- Runtime proof: plan-42-I.
- Doc sync: none.
- Final gate: plan-42-J.

## Decisions

As plan-42-D § Decisions.

## Corrections

## Summary

As plan-42-E, for ADI's and Maxim's leadless and wafer-level packages.
