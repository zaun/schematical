# plan-21-D: Multiplexer parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-21-C

D writes `schematical/schematical/Parts/mux.json` from
`planning/plan-21-data/parts.tsv` and `specs.md` and regenerates the goldens.

Outcome of D: `grep -c '|mux|' backend/tests/golden/parts.txt` = N21 (A's
`add` count); `parts.txt` and `traces.txt` grow by N21 lines each; no line
removed.

References: plan-21-A §3.2 (data conventions), plan-21-B §3.1 and
plan-21-C §3 (model keys).

## Prerequisites

plan-21-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-21-C complete | `ls planning/completed/plan-21-C-*.md` → exists | NOT MET (2026-11-07) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-21-data/parts.tsv \| wc -l` → N21 | NOT MET (A not run) |

## 1. Goal

- `mux.json` loads; `ctest` T0 + 1 of T0 + 1.
- Golden growth exactly N21 per file; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- Every device id of `specs.md` is in `mux.json`.

### Non-goals

- No model change (a needed change is a B/C bug: fixed there with a test, recorded in Corrections).

## 3. Design

`mux.json` with the header of the other part files, `"kind": "mux"`,
devices in plan-21-A §3.1 order; `parts` generated from `parts.tsv` by a
script under `/tmp/p21/` (not committed; plan-10 precedent).

## Phases

### Phase 1 — Part file and goldens

- [ ] Generate `mux.json`; `ctest -R library_check` passes; device coverage (python set difference of `parts.tsv` devices vs `mux.json` ids → empty).
- [ ] Regenerate `parts.txt` and `traces.txt`; removed 0, added N21 each.
- [ ] Spot checks: `SN74HC157N` (`|mux|`, `HC`, `units=4`, `PDIP-16`), `SN74LS151N` (`TTL`-style LS timing, Y and W pins), `SN74AS257N` (3-state), `CD74HC670E` (`Register file`), `SN74ABT16460DL` (`SSOP-56`), one `military`, one `automotive`.
- [ ] `trace_check --verbose SN74HC157N`: every Y equals the chosen input after the delay.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds).
Commit: —

## Validation Plan

D's gate: Phase 1. Landing tier in E.

## Decisions

None beyond plan-21-A.

## Corrections

## Summary

Data entry against tested models; device coverage, spot checks and the trace
check catch pin and polarity mistakes.
