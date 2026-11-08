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
| plan-22-C complete | `ls planning/completed/plan-22-C-*.md` → exists | MET (2026-11-07: `planning/completed/plan-22-C-arith-alu.md`) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-22-data/parts.tsv \| wc -l` → N22 | MET (2026-11-07: 107 = N22) |

## 1. Goal

- `arith.json` loads; device coverage complete; goldens +N22 each, 0 removed; `ctest` T0 + 1.

### Non-goals

- No model change (a needed one is a B/C bug, fixed there with a test and recorded).

## 3. Design

`arith.json`, `"kind": "arith"`, devices in plan-22-A §3.1 order; parts
generated from `parts.tsv` by a script under `/tmp/p22/` (not committed).

## Phases

### Phase 1 — Part file and goldens

- [x] Generate `arith.json`; `ctest -R library_check`; device coverage (set difference → empty). `/tmp/p22/scripts/gen_arith.py` (plan-21's `gen_mux.py` adapted) → `devices 16 parts 107`; `ctest -R library_check` → `100% tests passed`; device ids of `arith.json` vs `add`-row devices: `json-tsv set() tsv-json set()`.
- [x] Regenerate `parts.txt`, `traces.txt`; removed 0, added N22 each. Before: `listing_check` → only `extra part (not in the golden)` lines, `FAIL: 107 mismatches`. After `--print`: removed lines `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; added `parts.txt` 107, `traces.txt` 107; `grep -c '|arith|' parts.txt` → 107; `wc -l < parts.txt` → 11233 (P0 11126 + 107).
- [x] Spot checks: `SN74LS85N` (`Comparator`), `SN74HC688N` (`PDIP-20`), `SN74LS283N` (`Adder`), `SNJ54LS181J` (`ALU`, `military`, `ceramic`), `SN74LS280N` (`Parity`), `CD4585BE` (`C`). From `parts.txt`: `SN74LS85N|arith|DIP|Logic|Comparator|LS`; `SN74HC688N … footprint=PDIP-20`; `SN74LS283N|arith|DIP|Logic|Adder|LS`; `SNJ54LS181J|arith|DIP|Logic|ALU|LS|military,ceramic … footprint=CDIP-24-600`; `SN74LS280N|arith|DIP|Logic|Parity|LS`; `CD4585BE|arith|DIP|Logic|Comparator|C`.
- [x] `trace_check --verbose SN74LS283N`: Σ and C4 equal A + B + C0 at every settled step. `/tmp/p22/scripts/trace283.py` (inputs stable ≥ 16 ns, the LS delay being 15 ns) → `settled steps checked 53, mismatches 0`.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds). Met: `100% tests passed, 0 tests failed out of 25`; goldens +107 each, 0 removed.
Commit: —

## Validation Plan

D's gate: Phase 1. Landing tier in E.

## Decisions

None beyond plan-22-A.

## Corrections

- **N22 = 107** (plan-22-A Corrections), 16 devices; no model change was
  needed.

## Summary

Data entry against exhaustively tested models.
