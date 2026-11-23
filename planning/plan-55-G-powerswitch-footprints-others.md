# plan-55-G: Footprints — Microchip, Diodes, onsemi, Analog Devices and Monolithic Power Systems packages

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-F

G draws the land patterns of the packages of the other five companies of the final pick (the `G`
lines of `footprints-needed.txt`: at most 69 DigiKey `Package / Case` labels, plan-55-A § 2), by the
method and with the tools of plan-55-F, into `planning/plan-55-data/footprints-G.json` and G's rows
of `footprint-map.tsv`. The behavioral outcome: every `G` line has a footprint id in
`footprint-map.tsv`, and the new objects load through `footprint_check`.

References:

- `planning/plan-55-F-powerswitch-footprints-ti.md` § 3 (ids, reuse, the map, the two classes) and its tools;
  `docs/parts-format.md` § Footprints; `planning/plan-55-data/footprints-needed.txt`.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-F complete | `ls planning/completed/plan-55-F-*.md` → exists | NOT MET (2026-11-22: not started) |
| `footprints-needed.txt` has its `G` lines | `grep -c '^G' planning/plan-55-data/footprints-needed.txt` → E's recorded `G` count | NOT MET (2026-11-22: E not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-55-data/census/fp-check.py G` prints `<n> lines, 0 without a footprint`,
  `<n>` = `awk -F'\t' '$1=="G"' planning/plan-55-data/footprints-needed.txt | wc -l`.
- `fp-load.sh planning/plan-55-data/footprints-G.json` prints `<k> new footprints: load ok`.
- `footprint-map.tsv` has, for every `G` line, the footprint id and the pinouts it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, any `Parts/*.json`, `backend/` or the app.
- No footprint F already drew: a `G` line whose pads equal an `F` footprint's takes it
  (`source` `reuse`).

## 2. Current State

F's footprints are in `footprints-F.json` and the library's in `Parts/footprints.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `Package / Case` labels at most: Microchip 9 + 17, Diodes 17, onsemi 11, Analog Devices 13, Monolithic Power Systems 2 | 69 | `grep '^pick' planning/plan-55-data/families.out` → `packages(case)`: ideal ADI 13 and MPS 2, load onsemi 11 and Microchip 9, limited Microchip 17 and Diodes 17 |
| `G` lines after E | the distinct (company, package, supplier package) of the other companies' `add` rows | `awk -F'\t' '$1=="G"' planning/plan-55-data/footprints-needed.txt \| wc -l` (recorded here when G starts) |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The reuse pass

- [ ] Map every `G` line to a library footprint or an `F` footprint whose pads equal its drawing's.

Acceptance: every reusable line is mapped.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py G` → prints the count of lines still without a footprint (est. 1 min).
Commit: —

### Phase 2 — Leaded and through-hole packages

- [ ] Draw each remaining leaded `G` line into `footprints-G.json` (F § 3.2's class, § Footprints rules).

Acceptance: every leaded `G` line has a footprint and the objects load.
  Check: `planning/plan-55-data/census/fp-load.sh planning/plan-55-data/footprints-G.json` → `<k> new footprints: load ok` (est. 1 min).
Commit: —

### Phase 3 — Leadless and ball-grid packages

- [ ] The same for the leadless `G` lines.

Acceptance: every `G` line has a footprint and the objects load.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py G` → `<n> lines, 0 without a footprint`; `fp-load.sh planning/plan-55-data/footprints-G.json` → `<k> new footprints: load ok` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `fp-check.py` and `fp-load.sh`; no backend or app test is touched.
- Coverage check: `fp-check.py G` counts every `G` line once.
- Runtime proof: none (plan-55-J's harness draws a board).
- Doc sync: none.
- Final gate: plan-55-K.

## Decisions

- Author: **reuse beats drawing**, across F's footprints too.

## Corrections

(none yet)

## Summary

The same slow, exact work as F, for five companies; no part depends on it until H and I merge it.
