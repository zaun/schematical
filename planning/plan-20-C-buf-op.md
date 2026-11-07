# plan-20-C: The `buf` gate op and the plain-buffer parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-20-B

C adds a one-input, non-inverting gate op `buf` (the twin of the `not` op,
fd9a9fa) and the plain-buffer devices to `gate.json`.

Outcome of C: the `buf` op loads and computes Y = A; `gate.json` gains the
plan-20-A `model == buf` `add` rows (Nbuf); goldens grow by Nbuf lines each.

References: `backend/src/models/gate.c` (`GateOp`, `OP_NAMES`, `load_gate`'s
`not` check, `gate_value`), `docs/parts-format.md` § `gate`, the inverter
devices in `gate.json` (`1G04`, `74_06`, `1G06_SON` — the device shapes to
mirror), plan-20-A §3.1.

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-B complete | `ls planning/completed/plan-20-B-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-B-buffer-families-footprints.md`) |

## 1. Goal

- `library_check` `ok   gate buf with 2 inputs` rejection case passes; a `buf` fixture computes Y = A for 0, 1, X, Z (Z → X).
- `grep -c '"op": *"buf"' schematical/schematical/Parts/gate.json` ≥ 1; goldens +Nbuf each, 0 removed.

### Non-goals

- Every existing gate's listing line and trace unchanged.

## 2. Design

- `gate.c`: `OP_BUF` appended to `GateOp`, `"buf"` to `OP_NAMES`; `load_gate`
  requires exactly 1 input for `buf` (same message form as `not`:
  `a "buf" gate has exactly 1 input`); `gate_value`: `OP_BUF` returns the
  input's level (X for X or Z). `docs/parts-format.md` § `gate`: `buf` =
  buffer, exactly 1 input.
- Devices: one per function and pinout shape in `specs.md` (`74_07`,
  `74_17`, `74_34`, `1G07`, `1G17`, `1G34`, `2G07`, `2G17`, `2G34`, `3G07`,
  `3G17`, `3G34`, `74_6T07`, `74_6T17`, `1T34`, `1T17`, `1T50`, `4050B`,
  `74_4050`, `4041B` (each unit a `buf` and a `not` sharing the input),
  `74_4049` (`not`), `74_756`, `74_757`, `74_760`, `74_1034`, `74_11034`
  …), `function` `"Buffer"`, open-drain where the datasheet says so ('07,
  '17, 1G07, '756, '760 …), SON variants as separate devices (the
  `1G06_SON` precedent).

## Phases

### Phase 1 — Op

- [x] §2 `gate.c` (`OP_BUF`, `"buf"`, the 1-input check shared with `not`, `gate_value` as a 1-input AND) + docs § `gate`; `library_check` → `ok   gate buf with 2 inputs: … a "buf" gate has exactly 1 input`; the behavior fixture is in `engine_test` (fd9a9fa added no `not` behavior case to follow): a SOT-23-5 `buf` part, 4 ns — `PASS: buf: A = 0 gives Y = 0`, `… Y still 0 at 3 ns after A rises`, `… A = 1 gives Y = 1 after 4 ns`, `… A = X gives Y = X`, `… A floating (Z) gives Y = X`. `ctest --test-dir /tmp/logic-p20` → `100% tests passed, 0 tests failed out of 21`; `git diff --stat backend/tests/golden` → empty.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 of T0; golden diff empty (seconds).
Commit: —

### Phase 2 — Parts

- [ ] Generate the devices from `parts.tsv` (`model == buf`) into `gate.json` (script under `/tmp/p20/`, not committed).
- [ ] Regenerate `parts.txt`, `traces.txt`: 0 removed, +Nbuf each.
- [ ] Spot checks: `SN74LS07N` (open-drain), `SN74LVC1G17DBVR`, `CD4050BE` (`C`), `CD74HC4049E` (`not`).

Acceptance: `ctest` T0 of T0; golden counts (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2. Landing tier at the end of I.

## Decisions

- `buf` in the gate kind (plan-20-A §3.2).

## Corrections

## Summary

C is the smallest piece of plan-20 and lands about 540 parts on a one-line model change.
