# plan-20-D: The `buffer` kind — models `tristate` and `transceiver`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-20-C

D adds the part kind `buffer` (`PART_BUFFER`) with the two models most of
plan-20's parts use, and `buffer_test`. No part data (G).

Outcome of D: `ctest` T0 + 1 (`buffer_test`); goldens unchanged.

References: plan-20-A §3; `backend/src/library/kind.h`, `kinds.c`;
`backend/src/models/seq.h`; `latch.c` model `bus-mux` (bidirectional ports);
`backend/include/parts.h:25-27`; `backend/CMakeLists.txt:8-29`, `:116`.

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-C complete | `ls planning/completed/plan-20-C-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; `buffer_test` all PASS; golden diff empty.

### Non-goals

- No existing kind changes; public API gains only `PART_BUFFER` (appended after any value another of plans 19–22 appended).

## 2. Design

`backend/src/models/buffer.c` + `buffer.h` (model table, shared with E and
F): kind `"buffer"`, device keys `model`, `units`, `timing`, `output`,
`domains` (F) + model keys; part keys `family`, `vcc`, `inputMax`, `vccB`,
`inputMaxB` (F); sets `Part.logic`; `pin_open_drain` for open-drain devices.

### 2.1 `tristate`

| Key | Value |
|---|---|
| `"units"` | 1–8 objects: `a` (bits pin names), `y` (bits pin names), `oe` (list of 1–2 controls `{"pin","active"}`, all asserted = driven; '244 1OE/2OE, '541 OE1·OE2, '125 per-gate OE) |
| `"invert"` | bool ('240, '540, '1G240, '368, CD4502) |
| `"output"` | `"push-pull"` / `"open-drain"` |

OE asserted → Y = A (or NOT A); deasserted → Z; X OE → X.

### 2.2 `transceiver`

| Key | Value |
|---|---|
| `"units"` | 1–4 objects: `a`, `b` (bits pin names, `PIN_INOUT`), then either `dir` (pin: high = A→B per the datasheet) + `oe` (control) — the '245 form — or `oeab` + `oeba` (controls) — the '243/'623/'620 form |
| `"invert"` | bool ('640, '620) |
| `"output"` | `"push-pull"` / `"open-drain"` ('641, '642) |

Rules: the enabled direction's destination port drives the source port's
value; the source port reads; a disabled port reads and drives Z. Both
directions enabled ('623 OEAB high and OEBA low: the datasheet's "latch"
row) → each port drives the other's last value (the bus-latch behavior its
function table describes: the two buffers form a loop that holds), so the
held value is the one present when the second enable arrived. X on DIR/OE →
X on every port that could drive.

## 3. Tests — `backend/tests/buffer_test.c`

| Fixture | Cases |
|---|---|
| '244 (2 units, OE low) | each unit enabled/disabled; A = 0/1/X; Z out |
| '541 (two ANDed OEs) | every OE combination |
| '240 inverting | Y = NOT A |
| 1G125 / 1G126 | OE polarity |
| open-drain '1G07-style tristate ('125 OD variant per specs.md) | 0 driven, 1 → Z; `part_pin_open_drain` |
| '245 | DIR high: A→B; DIR low: B→A; OE high: both Z; an external drive on the source port passes through; the port being driven never reads its own value |
| '640 | inverting both ways |
| '623 | each enable alone; both enabled holds the bus value |
| 16245 (2 units) | units independent |
| X on DIR | both ports X where they could drive |

`library_check`: one failure case per validation error.

## Phases

### Phase 1 — Kind and `tristate`

- [ ] `PART_BUFFER`; `KINDS[]`; `buffer.c`, `buffer.h`; CMake lines; `buffer_test.c` tristate cases; test entry; docs § `buffer` (intro, `tristate`).

Acceptance: `ctest --test-dir /tmp/logic-p20 -R buffer_test` passes (seconds).
Commit: —

### Phase 2 — `transceiver`

- [ ] Model, §3 cases, docs, `library_check` cases.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

D's gate: Phase 2. Landing tier at the end of I.

## Decisions

- One `buffer` kind for every enabled, bidirectional, registered and
  translating part (plan-20-A §3.2).

## Corrections

## Summary

D proves the two models about 2650 candidates use, including the port
turnaround.
