# plan-21-B: The `mux` kind — model `select`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-21-A

B adds the part kind `mux` (`PART_MUX`) and its main model `select`, with
`mux_test`. No part data (D), no golden change.

Outcome of B: `ctest` T0 + 1 (`mux_test`); goldens unchanged.

References: plan-21-A §3; `backend/src/library/kind.h`, `kinds.c`,
`backend/src/models/seq.h`, `latch.c` (multi-model kind pattern),
`backend/include/parts.h:25-27` (`PartKind`), `backend/CMakeLists.txt:8-29`
(sources) and `:116` (tests), `docs/parts-format.md` § Sequential kinds.

## Prerequisites

plan-21-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-21-A complete | `ls planning/completed/plan-21-A-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p21` → T0 + 1 of T0 + 1; `mux_test` all PASS.
- `git diff --stat backend/tests/golden` → empty.

### Non-goals

- No change to existing kinds; the public API gains only `PART_MUX`,
  appended to `PartKind` (after any value another of plans 19–22 appended).

## 3. Design

- `backend/src/models/mux.c` + `mux.h` (model table, shared with C's
  `mux_stored.c`): kind name `"mux"`, device keys `model`, `units`,
  `timing`, `output` + the model's; part keys `family`, `vcc`, `inputMax`;
  sets `Part.logic`. `pin_open_drain` for `"output": "open-drain"` devices.

### 3.1 Model `select`

Main path data → Y. `units` = the number of unit objects.

| Key | Value |
|---|---|
| `"units"` | 1–6 objects: `data` (2–16 pin names; `data[i]` chosen by select code i), `select` (1–4 pin names, LSB first; usually shared by every unit), `y` (optional pin: true output), `w` (optional pin: complement; at least one of `y`/`w`), `strobe` (optional control `{"pin","active"}`: deasserted forces the outputs to their `disabled` state), `oe` (optional control: output enable, separate from the strobe — '251's G̅ is an OE, CD4512 has both INH (strobe) and DIS (OE)) |
| `"disabled"` | what a deasserted strobe gives: `"low"` (Y = 0, W = 1: '151, '153, '157) or `"z"` ('251, '253, '257, '258, '353) |
| `"invert"` | bool: the `y` pin shows NOT the chosen data ('158, '258, '352, '1T158) |
| `"selectLatch"` | optional `{"pin","active"}`: transparent latch on the select inputs, holds while asserted ('850/'851 clock-controlled select latch: their datasheet's latch enable) |
| `"output"` | `"push-pull"` (default) / `"open-drain"` |

Rules: OE deasserted → Z; else strobe deasserted → `disabled`; else Y =
data[code] (inverted per `invert`), W = NOT Y. X select bit → X unless every
candidate data value is equal and known; X strobe/OE → X.

## 4. Tests — `backend/tests/mux_test.c`

| Fixture | Cases |
|---|---|
| '151 (8 data, 3 select, strobe low, Y + W) | all 8 codes × data 0/1; strobe high → Y 0, W 1; X on S1 with D0 = D2 = 1 → Y 1, with D0 ≠ D2 → X |
| '153 (2 units, shared select, separate strobes) | units independent |
| '157 (4 units, 1 select, shared strobe) | both select values; strobe high → all Y 0 |
| '158 (`invert`) | Y = NOT data |
| '251 (3-state OE) | OE high → Y, W Z; low → driven |
| '257 (`disabled` z) | strobe high → Z |
| CD4512 (INH strobe → low, DIS OE → Z) | INH and DIS rows of its truth table |
| '850 (`selectLatch`) | select held while latched, follows while transparent |
| 2G157 (`y` and complement pin only) | its function table |

`library_check`: one failure case per validation error (`units[0]: "data"
needs 2^n entries or fewer for n select pins`, missing `y`/`w`, bad
`disabled`, …).

## Phases

### Phase 1 — Kind and `select`

- [ ] `PART_MUX`; `KINDS[]` entry; `mux.c`, `mux.h`; `CMakeLists.txt` source lines.
- [ ] `mux_test.c` with §4; test list entry; `library_check` cases.
- [ ] `docs/parts-format.md` § `mux` (intro, model `select`); `docs/backend.md` kind list.

Acceptance: `ctest --test-dir /tmp/logic-p21` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

B's gate: Phase 1's `ctest` (about 13 s). Landing tier at the end of E.

## Decisions

- One kind for all plan-21 models (plan-21-A §3.1).

## Corrections

## Summary

B delivers and tests the selector that 583 of the 634 candidates use.
