# plan-22-D: Comparator, adder, ALU and parity parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-22-C

D writes `schematical/schematical/Parts/arith.json` from
`planning/plan-22-data/parts.tsv` and `specs.md`; regenerates the goldens.

Outcome of D: `grep -c '|arith|' backend/tests/golden/parts.txt` = N22;
`parts.txt` and `traces.txt` each grow by N22; no line removed.

## Prerequisites

plan-22-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-22-C complete | `ls planning/completed/plan-22-C-*.md` → exists | NOT MET (2026-11-07) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-22-data/parts.tsv \| wc -l` → N22 | NOT MET (A not run) |

## 1. Goal

- `arith.json` loads; device coverage complete; goldens +N22 each, 0 removed; `ctest` T0 + 1.

### Non-goals

- No model change (a needed one is a B/C bug, fixed there with a test and recorded).

## 3. Design

`arith.json`, `"kind": "arith"`, devices in plan-22-A §3.1 order; parts
generated from `parts.tsv` by a script under `/tmp/p22/` (not committed).

## Phases

### Phase 1 — Part file and goldens

- [ ] Generate `arith.json`; `ctest -R library_check`; device coverage (set difference → empty).
- [ ] Regenerate `parts.txt`, `traces.txt`; removed 0, added N22 each.
- [ ] Spot checks: `SN74LS85N` (`Comparator`), `SN74HC688N` (`PDIP-20`), `SN74LS283N` (`Adder`), `SNJ54LS181J` (`ALU`, `military`, `ceramic`), `SN74LS280N` (`Parity`), `CD4585BE` (`C`).
- [ ] `trace_check --verbose SN74LS283N`: Σ and C4 equal A + B + C0 at every settled step.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds).
Commit: —

## Validation Plan

D's gate: Phase 1. Landing tier in E.

## Decisions

None beyond plan-22-A.

## Corrections

## Summary

Data entry against exhaustively tested models.
