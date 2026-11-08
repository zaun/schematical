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
| plan-20-C complete | `ls planning/completed/plan-20-C-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-C-buf-op.md`) |

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

- [x] `PART_BUFFER` (appended after `PART_PIO`); `KINDS[]`; `buffer.c`, `buffer.h`; CMake lines (`src/models/buffer.c`, `logic_test(buffer_test)`); listing/trace/listing_check last-kind lines; `buffer_test.c` tristate cases ('244, '240, '541, 1G125/1G126, open-drain 1G125); docs § `buffer` (intro, enables, port rule, `tristate`), `docs/backend.md` (source map, test list). `ctest --test-dir /tmp/logic-p20 -R buffer_test` → `Passed`; `buffer_test` prints `buffer_test: all passed`.

Acceptance: `ctest --test-dir /tmp/logic-p20 -R buffer_test` passes (seconds).
Commit: 849cb04 (Phases 1 and 2 together, one source file)

### Phase 2 — `transceiver`

- [x] Model `transceiver` (DIR + OE and `oeab`/`oeba` forms, `invert`, `open-drain`, the '623 hold loop), §3 cases ('245 both ways, external drive passes, turnaround never reads the part's own drive — `PASS: '245: after DIR goes low, A never shows the part's own old B drive`, OE high → both Z, X on DIR → both ports X; '640; '641 open drain; '623 each enable, isolation, both-enabled hold; 16245 units independent), docs § Model `transceiver`, loader error cases (14, in `buffer_test` `check_loader` — Corrections 1). `ctest --test-dir /tmp/logic-p20` → `100% tests passed, 0 tests failed out of 22`; `git diff --stat backend/tests/golden` → empty.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: 849cb04

## Validation Plan

D's gate: Phase 2. Landing tier at the end of I.

## Decisions

- One `buffer` kind for every enabled, bidirectional, registered and
  translating part (plan-20-A §3.2).

## Corrections

1. **Loader error cases live in `buffer_test`** (`check_loader`), not `library_check`: they need fixture footprints, which `buffer_test` already loads; one case per validation error (unknown model, unknown device key, no units, `a`/`y` count, unknown unit key, enable not a control, bad level, unknown pin, bad output style, `a`/`b` count, missing `oeba`, `dir` without `ab`, `dir` with `oeab`).
2. **Open-drain ports**: `part_pin_open_drain` answered only for `PIN_OUTPUT` pins and the voltage check counted every `PIN_INOUT` as a driver, so an open-drain transceiver port ('641, '642) would have driven its domain's level. Both now take `PIN_INOUT` too (`kinds.c`, `voltage.c:280`); no existing kind reports an inout pin as open drain, so no golden or `voltage_test` line moves (`ctest` 22/22, golden diff empty).
3. **Turnaround needs a timer**: "never reads its own value" is implemented as: a port whose drive was released reads X until the release lands (rise delay + 1 ps), and the model re-evaluates then (`sim_schedule_timer`) — a pin change alone would not come when the floating bus keeps the old value.

## Summary

D proves the two models about 2650 candidates use, including the port
turnaround.
