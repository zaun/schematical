# plan-22-C: `arith` models `adder`, `alu181`, `lookahead`, `wallace`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-22-B

C adds the arithmetic models and their exhaustive `arith_test` cases.

Outcome of C: `arith_test` PASS for every §3 case; `ctest` T0 + 1; goldens unchanged.

References: plan-22-A §3, plan-22-B §3 (kind, X helper); the datasheets in
`datasheets/arith/` (sn74ls283, sn54ls181, sn54s182, the '275 scan) and
their `specs.md` sections.

## Prerequisites

plan-22-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-22-B complete | `ls planning/completed/plan-22-B-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `arith_test` PASS for all §3 cases; `ctest` T0 + 1 of T0 + 1; golden diff empty.

## 2. Design (`backend/src/models/arith_alu.c`)

### 2.1 `adder`

Keys `a`, `b`, `sum` (4 pins each, LSB first), `cin`, `cout`. Σ = A + B +
Cin; Cout the carry. Test: all 512 input combinations against `a+b+c`.

### 2.2 `alu181`

Keys `a`, `b` (4 pins, active-high data per the datasheet's Table 1 —
`specs.md` § 74_181 records which of the two data polarities the pin names
use), `s` (S0–S3), `m`, `cn`, `f` (4), `cn4`, `p`, `g`, `aeqb`
(open-drain). Behavior: the datasheet's function table (16 logic functions
with M = H; 16 arithmetic functions with M = L, with and without carry),
`P`/`G` the group propagate/generate, `A = B` high when every F is high.
Test: every (S, M, Cn) row × 256 (A, B) pairs against a reference table
coded from the datasheet's function table (8192 cases), plus P, G, Cn+4.

### 2.3 `lookahead`

'182: keys `p` (4), `g` (4), `cn`, `cnx`, `cny`, `cnz`, `pout`, `gout`
(active-low P/G per the datasheet). Equations from the datasheet's logic
diagram, transcribed in `specs.md` § 74_182. Test: all 512 input
combinations against the equations; a two-level cascade of four `alu181`
fixtures and one `lookahead` adds 16-bit numbers correctly (100 random
pairs, fixed seed).

### 2.4 `wallace`

'275: the 7-bit-slice Wallace tree of its scanned datasheet: inputs, the two
3-state outputs and the cascade inputs/outputs as `specs.md` § 74_275
transcribes them; output = the sum of the input bits as the datasheet's
truth table gives it. Test: every row of that truth table.

## Phases

### Phase 1 — `adder`, `alu181`, `lookahead`

- [ ] `arith_alu.c` (+ CMake line); §2.1–§2.3 tests; docs model tables.

Acceptance: `ctest --test-dir /tmp/logic-p22 -R arith_test` passes (seconds).
Commit: —

### Phase 2 — `wallace`

- [ ] '275 model, test, docs; `library_check` failure cases for every new validation error.

Acceptance: `ctest --test-dir /tmp/logic-p22` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2's `ctest`. Landing tier at the end of E.

## Decisions

- Exhaustive tests for '283/'181/'182 (every input combination is cheap: at
  most 8192 cases).

## Corrections

## Summary

C carries plan-22's correctness risk and proves it exhaustively.
