# plan-20-G: Parts — 3-state buffers and transceivers

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-20-F

G writes `schematical/schematical/Parts/buffer.json` with the `tristate` and
`transceiver` devices of `planning/plan-20-data/parts.tsv` and regenerates
the goldens.

Outcome of G: `grep -c '|buffer|' backend/tests/golden/parts.txt` =
Ntri + Nxcvr (the `add` rows with those models); goldens grow by that many
lines each; none removed.

References: plan-20-A §3.4 (conventions), plan-20-D §2 (model keys).

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-F complete | `ls planning/completed/plan-20-F-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-F-dual-supply.md`) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add" && ($13=="tristate" \|\| $13=="transceiver")' planning/plan-20-data/parts.tsv \| wc -l` → Ntri + Nxcvr | MET (2026-11-07: `1749` = Ntri 1331 + Nxcvr 418) |

## 1. Goal

- `buffer.json` loads; device coverage for both models complete; goldens +Ntri+Nxcvr each, 0 removed; `ctest` T0 + 1.

### Non-goals

- No model change (a needed one is a D bug: fixed there with a test, recorded).

## 2. Design

`buffer.json`, `"kind": "buffer"`; devices in function order; Widebus parts
as 2-unit (16-bit) or 4-unit (32-bit) devices; parts generated from
`parts.tsv` by a script under `/tmp/p20/` (not committed).

## Phases

### Phase 1 — Tristate devices

- [x] Generate the `tristate` devices (74 devices, **Ntri = 1331** parts; `gen.py` + `land.py buffer tristate`, which also appends the plan-20-B footprints they use: `QSOP-20`, `QSOP-24`, `TSSOP-80`, `TVSOP-56`, `CFP-24`); `/tmp/logic-p20/library_check` → `library_check: all passed`; coverage (python: `add` rows with model `tristate` vs `buffer.json` part names) → `tristate rows 1331 in buffer.json 1331 missing [] extra []`. Goldens regenerated in the same commit so `ctest` stays green: `git diff --numstat backend/tests/golden` → `5 0 footprints.txt`, `1331 0 parts.txt`, `1331 0 traces.txt`; `ctest` → `100% tests passed, 0 tests failed out of 22`.

Acceptance: library loads (seconds).
Commit: eb3852a

### Phase 2 — Transceiver devices and goldens

- [x] Generate the `transceiver` devices (29 devices, **Nxcvr = 418**; footprints `TVSOP-24`, `LQFP-100-14X14` appended); regenerate `parts.txt`, `traces.txt`, `footprints.txt`: over the two G commits `+1331 +418` each, 0 removed (this commit: `418 0 parts.txt`, `418 0 traces.txt`, `2 0 footprints.txt`); `grep -c '|buffer|' backend/tests/golden/parts.txt` → `1749` = Ntri + Nxcvr; `ctest` → `100% tests passed, 0 tests failed out of 22`.
- [x] Spot checks (`parts.txt`): `SN74HC244N|buffer|DIP|Logic|Buffer|HC||units=8`; `SN74LVC1G125DCKR|buffer|SC-70|Logic|Buffer|LVC||units=1` (SN74LVC1G125DBVR is not in the census: not in stock); `SN74ABT16244ADGGR|buffer|TSSOP|Logic|Buffer|ABT||units=16` (units = bits); `SN74HC245DWR|buffer|SOIC|Logic|Transceiver|HC||units=8` (SN74HC245N not in the census); `SN74LS640N|buffer|DIP|Logic|Transceiver|LS||units=8`, device `74_640` `"invert": true`; `SN74ALVCH16245DGGR|…|Transceiver|ALVC||units=16`; military `5962-8408501VSA|buffer|CFP|Logic|Transceiver|HC|military,ceramic|units=8`; automotive `1P1G125QDCKRG4Q1|buffer|SC-70|Logic|Buffer|LVC|automotive|units=1`.
- [x] `trace_check --verbose SN74HC245DWR` (same device; SN74HC245N not in the census), analysed by `/tmp/p20-scripts/trace245.py` (every moment ≥ 40 ns after the last A/DIR/OE change in a DIR-high, OE-low stretch: B_k = A_k) → `checks 126 mismatches 0`.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts (seconds).
Commit: 4378575

## Validation Plan

G's gate: Phase 2. Landing tier at the end of I.

## Decisions

None beyond plan-20-A.

## Corrections

1. **Each phase lands with its goldens**: Phase 1 regenerates `parts.txt`/`traces.txt`/`footprints.txt` for the tristate parts in its own commit, so `ctest` is green at every commit (the plan put all golden work in Phase 2).
2. **Counts** (plan-20-A): Ntri = 1331, Nxcvr = 418 (the census guessed ~2650 together before the status, ordering-table and datasheet rules).
3. **Spot-check names**: SN74LVC1G125DBVR and SN74HC245N are not candidates (not in stock at DigiKey on 2026-11-07); their device siblings were checked instead.

## Summary

G is the bulk data drop of plan-20 (about 2650 candidates) on two tested models.
