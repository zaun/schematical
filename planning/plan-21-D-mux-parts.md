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
| plan-21-C complete | `ls planning/completed/plan-21-C-*.md` → exists | MET: `planning/completed/plan-21-C-mux-special.md` (C at 82d8f17, archived in D's commit) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-21-data/parts.tsv \| wc -l` → N21 | MET: 346 |

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

- [x] Generate `mux.json` (`/tmp/p21/scripts/gen_mux.py`, from plan-19's generator: devices in §3.1 order, `partVcc` and `sn54vcc` resolved onto the parts) → `devices 20 parts 346`; `ctest -R library_check` passes; device coverage: `parts.tsv` add-row devices vs `mux.json` ids → the generator asserts equality (20 = 20), and every `specs.md` section id is a `mux.json` id (20 sections).
- [x] Regenerate `parts.txt` and `traces.txt` (`listing_check --print`, `trace_check --print`): `git diff --stat backend/tests/golden` → `parts.txt | 346 +++`, `traces.txt | 346 +++`, `692 insertions(+)`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `grep -c '|mux|' parts.txt` → 346.
- [x] Spot checks: `SN74HC157N` (`|mux|`, family `HC` in `mux.json`, `units=4`, `footprint=PDIP-16`, `vcc=2000-6000`); `SN74LS151N` (`units=1`, pins `5:Y:output`, `6:W:output`, LS timing 20/16 ns, VCC 4.75–5.25 V); `SN74AS257N` (device `74_257` `disabled` `z`: 3-state, `PDIP-16`); `CD74HC670E` (`|Register file|`); military `CD54HC157F` (`military,ceramic`); automotive `CAHCT157QWBQBRQ1` (`automotive`). ~~`SN74ABT16460DL`~~ — moot: no '16460 part is added (plan-21-C Corrections).
- [x] `trace_check --verbose SN74HC157N`: checked by `/tmp/p21/scripts/trace157.py` (each Y against A/B, G and its data inputs, 13 ns HC delay, inertial): `Y edges checked 118 mismatches 0`.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds).
Commit: —

## Validation Plan

D's gate: Phase 1. Landing tier in E.

## Decisions

None beyond plan-21-A.

## Corrections

- **N21 = 346** (20 devices; plan-21-A Corrections). The '16460 spot check is moot: no '16460 part is added.
- **Spec fields not in `mux.json`**: `pinoutByCode`, `sn54vcc` and `partVcc` are resolved by the generator (part `pinout` from the package code; part `vcc` for SN54 parts whose family range differs, and for -Q1/-EP parts whose sheet gives their own range) and dropped from the device.

## Summary

Data entry against tested models; device coverage, spot checks and the trace
check catch pin and polarity mistakes.
