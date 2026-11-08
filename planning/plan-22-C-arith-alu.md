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
| plan-22-B complete | `ls planning/completed/plan-22-B-*.md` → exists | MET (2026-11-07: `planning/completed/plan-22-B-arith-kind.md`) |

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

- [x] `arith_alu.c` (+ CMake line); §2.1–§2.3 tests; docs model tables. `adder` and `alu181` in `arith_alu.c`, listed in `arith.c` `MODELS[]`. Tests: `PASS: 74_283 all 512 inputs: S1-S4 and C4 are A + B + C0`; `PASS: 74_181 64 (S, M, Cn) rows x 256 (A, B): F, A=B, Cn+4, P, G against Table 2` (16384 cases; the reference is coded from sn54ls181 Table 2, p.4, as transcribed in specs.md § 74_181, not from the model's p/g equations); X-rule cases for both. `docs/parts-format.md` § `adder`, § `alu181`.
- [x] ~~§2.3 `lookahead` ('182) model and its tests~~ — moot: no '182 part is added (plan-22-A Corrections: SN74S182N, the only in-stock '182, is not in the sn54s182/sdls206 ordering table, which lists only SN54S182J, SNJ54S182J, SNJ54S182FK; `awk -F'\t' '$2=="74_182" && $11=="add"' planning/plan-22-data/parts.tsv | wc -l` → 0). A model no part uses is not built (plan-21-C precedent for `universal`/`bus4`). The '181's lookahead outputs are still tested: G̅ low exactly when X + Y overflows without a carry, G̅ or P̅ low exactly when it overflows with one, for all 16384 cases.

Acceptance: `ctest --test-dir /tmp/logic-p22 -R arith_test` passes (seconds). Met: `arith_test: all passed`.
Commit: —

### Phase 2 — `wallace`

- [x] '275 model, test, docs; `library_check` failure cases for every new validation error. `wallace` (full-adder netlist + 3-state enable); `PASS: 74_275 all 512 slice/carry inputs: the p.2 adder tree, Y0 + 2(Y1 + CY) + 4 Y2 = count` and the enable/X case; `library_check` `check_arith_alu_cases`: 3 valid devices + 14 rejections (adder, alu181, wallace), 56 arith lines, 0 FAIL; `docs/parts-format.md` § `wallace`.

Acceptance: `ctest --test-dir /tmp/logic-p22` → T0 + 1 of T0 + 1; golden diff empty (seconds). Met: `100% tests passed, 0 tests failed out of 25`; `git diff --stat backend/tests/golden` → empty.
Commit: —

## Validation Plan

C's gate: Phase 2's `ctest`. Landing tier at the end of E.

## Decisions

- Exhaustive tests for '283/'181/'182 (every input combination is cheap: at
  most 8192 cases).

## Corrections

- **`lookahead` is not built** (no '182 part; plan-22-A Corrections), so
  the §2.3 test "a two-level cascade of four `alu181` fixtures and one
  `lookahead`" has nothing to cascade into; the '181's P̅/G̅/C̅n+4 are
  checked against the carry-lookahead meaning instead (Phase 1).
- **The '181's active-high table is Table 2 (p.4)**, not "Table 1" as §2.2
  says; Table 1 (p.3) is the active-low one (specs.md § 74_181).
- **§2.2 "8192 cases" is 16384**: 16 S × 2 M × 2 Cn × 256 (A, B); the
  M = H rows are run with both Cn levels too.
- **The '275 sheet has no truth table** (plan-22-A Corrections): the model
  is its p.2 block diagram as a netlist (`adders`, `outputs`, `enable`),
  and the test is all 512 inputs against that netlist coded separately in
  the test, plus the weighted-sum identity.

## Summary

C carries plan-22's correctness risk and proves it exhaustively.
