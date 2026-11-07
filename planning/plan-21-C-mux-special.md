# plan-21-C: `mux` models `stored`, `regfile`, `universal`, `bus4`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-21-B

C adds the four remaining `mux` models (51 of the 634 candidates: 26 + 17 +
3 + 5, plan-21-A § Measured populations) and their `mux_test` cases.

Outcome of C: `mux_test` passes the §4 cases; `ctest` T0 + 1; goldens unchanged.

References: plan-21-A §3, plan-21-B §3 (kind, `select`), `models/seq.h`
(edges, controls, `SeqBits`), `latch.c` model `bus-mux` (the existing
bidirectional bus pattern: 74_16260), `docs/parts-format.md` § Model `bus-mux`.

## Prerequisites

plan-21-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-21-B complete | `ls planning/completed/plan-21-B-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `mux_test` prints PASS for every §4 case, no FAIL; `ctest` T0 + 1 of T0 + 1.

### Non-goals

- `select` unchanged except for helpers moved into `mux.h` for reuse.

## 3. Design (`backend/src/models/mux_stored.c`)

### 3.1 `stored`

`select` (B) plus storage, keys of `select` plus:

| Key | Value |
|---|---|
| `"dataLatch"` | optional `{"pin","active"}`: transparent latch on every data input, holds while asserted ('354/'355 DC) |
| `"dataClock"` | optional `{"pin","edge"}`: edge-triggered register on the data inputs ('356) |
| `"outputClock"` | optional `{"pin","edge"}`: edge-triggered register on the selected value, the output shows the register ('298/'399: falling edge loads the word chosen by WS) |
| `"selectLatch"` | as `select` ('354/'355/'356 SC) |
| `"oes"` | optional list of controls, all asserted = enabled ('354: three output enables) |

LS604: units `data` = [A_i, B_i], select = A/B, `dataLatch` on the clock pin
per its function table (§ specs.md 74_604), outputs 3-state.

Stored bits are 0 at power-on; X clock edge → stored bits X (plan-10 §3.5).

### 3.2 `regfile`

Main path read address → Q.

| Key | Value |
|---|---|
| `"words"`, `"bits"` | 4 × 4 ('670) or 16 × 4 ('870, two files) |
| `"files"` | 1–2 objects: `d` (bits pins), `wa` (write address pins LSB first), `we` (control: write enable — the addressed word follows D while asserted, transparent), `ra` (read address pins), `re` (control: read enable; deasserted → Q Z), `q` (bits pins) |
| `"output"` | `"3-state"` only (both chips) |

Read is combinational from the array (a write to the word being read shows
on Q after the main path). Power-on: every word 0.

### 3.3 `universal`

'857 hex 2-to-1: units `a`, `b`, `y`; shared `select` and `modes` control
pins (S0/S1 per specs.md) with a 4-row table `"modes": ["a", "b", "zero",
"one"]` keyed by the control code, `"complement"` control (per specs.md:
inverts every Y), `oe` control (3-state).

### 3.4 `bus4`

The ABT16460/162460: an A port and four B ports. Keys: `a` (bits pins), `b`
(4 lists of bits pins), `selA` (2 pins: which B port drives A), `oeA`, `oeB`
(4 controls), `latchB` (4 controls: B-port latch enables, transparent per
specs.md), `latchA` (control: A-side latch), per the device's function table
transcribed in `specs.md`. Pins are `PIN_INOUT`; a port drives only while
its OE is asserted, otherwise reads. Mirrors latch `bus-mux`'s
read-while-not-driving handling (no self-read of a driven value).

## 4. Tests (added to `mux_test.c`)

| Fixture | Cases |
|---|---|
| '298 | falling edge loads word 1 or word 2 per WS; rising edge does nothing; X WS at the edge → X |
| '354 | DC holds data; SC holds select; any OE deasserted → Z |
| '356 | data registered on the clock edge |
| LS604 | its function table rows |
| '670 | write word 2 while reading word 2 (shows), reading word 1 (unchanged); GR high → Z |
| '870 | two files independent |
| '857 | the 4 modes × complement; OE |
| '16460 | each B port to A by `selA`; A to each latched B port; OE turnaround: a port never reads its own drive |

## Phases

### Phase 1 — `stored` and `regfile`

- [ ] `mux_stored.c`, CMake line, §4 cases for '298, '354, '356, LS604, '670, '870; docs § `mux` model tables.

Acceptance: `ctest --test-dir /tmp/logic-p21 -R mux_test` passes (seconds).
Commit: —

### Phase 2 — `universal` and `bus4`

- [ ] '857 and '16460 models, §4 cases, docs, `library_check` failure cases for every new validation error.

Acceptance: `ctest --test-dir /tmp/logic-p21` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2's `ctest`. Landing tier at the end of E.

## Decisions

- The register files are `mux` parts (plan-21-A §3.1 rejected alternatives).

## Corrections

## Summary

C covers the 51 storage, register-file and bus parts; `bus4`'s
bidirectional turnaround is the highest-risk model and is tested in both
directions.
