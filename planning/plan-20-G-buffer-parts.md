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
| plan-20-F complete | `ls planning/completed/plan-20-F-*.md` → exists | NOT MET (2026-11-07) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add" && ($13=="tristate" \|\| $13=="transceiver")' planning/plan-20-data/parts.tsv \| wc -l` → Ntri + Nxcvr | NOT MET (A not run) |

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

- [ ] Generate the `tristate` devices; `ctest -R library_check`; coverage check.

Acceptance: library loads (seconds).
Commit: —

### Phase 2 — Transceiver devices and goldens

- [ ] Generate the `transceiver` devices; regenerate `parts.txt`, `traces.txt` (0 removed; + Ntri + Nxcvr each).
- [ ] Spot checks: `SN74HC244N`, `SN74LVC1G125DBVR`, `SN74ABT16244ADGGR` (`units` per unit-count rule), `SN74HC245N` (`Transceiver`), `SN74LS640N` (inverting), `SN74ALVCH16245DGGR` (`ALVC`), one `military`, one `automotive`.
- [ ] `trace_check --verbose SN74HC245N`: with DIR high and OE low, every B pin follows its A pin.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts (seconds).
Commit: —

## Validation Plan

G's gate: Phase 2. Landing tier at the end of I.

## Decisions

None beyond plan-20-A.

## Corrections

## Summary

G is the bulk data drop of plan-20 (about 2650 candidates) on two tested models.
