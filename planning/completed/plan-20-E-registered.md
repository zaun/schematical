# plan-20-E: `buffer` model `registered` (latched and registered transceivers)

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-20-D

E adds the model for transceivers with storage in each direction:
'646/'648/'652/'654/'651/'653 (registers and a real-time/stored select),
'543/'544/16543 (latches with CE/LE/OE per direction), '2952/'2953/16952
(registers with clock enables), and the other `registered` devices of
`specs.md`.

Outcome of E: `buffer_test` passes the §3 cases; `ctest` T0 + 1; goldens unchanged.

References: plan-20-A §3.1, plan-20-D §2 (kind, ports), `models/seq.h`
(edges, controls), `specs.md` (each device's function table).

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-D complete | `ls planning/completed/plan-20-D-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-D-buffer-kind.md`) |

## 1. Goal

- `buffer_test` PASS for every §3 case; `ctest` T0 + 1 of T0 + 1; golden diff empty.

## 2. Design (`backend/src/models/buffer_reg.c`)

Keys: `units` objects with `a`, `b` (bits), and per direction `ab`, `ba`
objects:

| Key (inside `ab` / `ba`) | Value |
|---|---|
| `"store"` | `"latch"` or `"register"` |
| `"le"` | latch: control, the latch is transparent while asserted ('543 LEAB̅ low = transparent) |
| `"clk"` | register: `{"pin","edge"}` |
| `"ce"` | optional control: clock/latch enable ('543 CEAB, '2952 CLKENAB) |
| `"oe"` | control: the destination port drives while asserted ('543 OEAB with CEAB, '652 OEAB/OEBA, '646 via `dir`+`oe` at unit level) |
| `"select"` | optional control: asserted → the destination drives the stored value; deasserted → real-time (the source's current value) ('646 SAB/SBA) |
| `"invert"` | bool ('648, '654, '544) |

Unit-level `dir` + `oe` (the '646 form) map to the two `oe`s as in the
'245. Stored bits 0 at power-on; the register loads from the source port's
value whether or not that port is being driven by the part (the '646 can
store the bus while it drives it, per its function table, specs.md).

## 3. Tests (in `buffer_test.c`)

| Fixture | Cases |
|---|---|
| '646 | every row of its function table: real-time transfer A→B, B→A; store A in A register while B drives; stored transfer; isolation |
| '652 | OEAB/OEBA independent; both enabled with stored select (bus hold through the registers) |
| '543 | CEAB/LEAB/OEAB rows; latch hold while LEAB high |
| '2952 | clock enables gate the edge |
| 16952 (2 units) | units independent |
| X on a clock | stored bits X |

## Phases

### Phase 1 — Model and tests

- [x] `buffer_reg.c` (+ CMake line; the model table in `buffer.c` now holds pointers so it can name `BUF_REGISTERED_MODEL`), §3 cases in `buffer_test` ('646: isolation and storing while OE high, real-time A→B and B→A, stored transfer, the A register storing the bus the part drives; '652: OEAB/OEBA independent, both enabled with stored select; '543: CEAB/LEAB/OEAB rows, hold while LEAB high, CEAB high turns B off; '2952: CLKENAB gates the edge, no real-time path; 16952: units independent; X on a clock → the bits that would change are X), docs § Model `registered`, loader cases (6, in `buffer_test` as D's). `buffer_test` → `buffer_test: all passed` (67 PASS lines). Dry run: the 28 real `registered` specs (155 parts) load with the tristate/transceiver ones (`listing_check`: `8243 parts compared` — only `extra part` lines, no load error). `ctest --test-dir /tmp/logic-p20` → `100% tests passed, 0 tests failed out of 22`; `git diff --stat backend/tests/golden` → empty.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: 9735f28

## Validation Plan

E's gate: Phase 1. Landing tier at the end of I.

## Decisions

- One parametric model for every storage transceiver (the four TI shapes
  differ only in store type, enables and select).

## Corrections

1. **`ce` means two things**, by the datasheets (plan-20-A reports G05, G07, G08): on a latch it gates both the latch and the outputs ('543: B drives only while CEAB and OEAB are low), on a register only the clock ('2952 CLKENAB, CY29FCT52 CE). §2's table now says so; no separate key needed.
2. **Unit-level `dir` + `oe` excludes per-direction `oe`** ('646 form) — a loader error otherwise.
3. **Test benches must not drive a port in the instant the part still drives it**: that shorts the net and the engine stops; `step()` in `buffer_test` now fails on any engine error (`bench_error`), which the first '646 draft hit.
4. **The listing printed no family for buffer parts** (`tests/support/listing.c` `logic` test missed `PART_BUFFER`, a D gap found by E's dry run); fixed before any buffer part exists, so no golden moves.

## Summary

E covers the about 310 registered and latched transceiver candidates.
