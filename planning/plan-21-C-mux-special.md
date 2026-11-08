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
| plan-21-B complete | `ls planning/completed/plan-21-B-*.md` → exists | B's work is in the same commit as C's (plan-21-B Corrections: `mux.h`, the model table and the tests are shared); B's `- [x]` boxes and acceptance were met before C's code was added to that tree. B is archived with C in D's commit. |

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

- [x] `mux_stored.c` (models `stored`, `regfile`), CMake line, §4 cases for '298 (16–18), '399 (19, added), '354 (20–22), '356 (23), '670 (24–26), two-file register file shape `shape_rf2` (27, in place of the '870); docs § `mux` model tables (`stored`, `regfile`). Check: `/tmp/logic-p21/mux_test` → 27 `PASS`, `mux_test: all passed`.
- [x] ~~LS604 case~~ — moot: SN74LS604N, the only LS604 candidate, has no TI datasheet (no TI product page, no ti.com/lit PDF; DigiKey links a Rochester copy) → `skip` in `parts.tsv`, so there is no function table to test against and no part to carry it.
- [x] `library_check` failure cases for every new `stored`/`regfile` validation error (16 lines: valid stored, valid two-file regfile, no storage key, `dataLatch` + `dataClock`, bad edge, 4 `oes`, oe without `active`, unknown clock pin, 3 words, 9 bits, no files, 3 files, `q` count, `wa` count, missing `we`, unknown file key) → all `ok`.

Acceptance: `ctest --test-dir /tmp/logic-p21 -R mux_test` passes (seconds).
Commit: —

### Phase 2 — `universal` and `bus4`

- [x] ~~'857 (`universal`) and '16460 (`bus4`) models, §4 cases, docs, `library_check` cases~~ — moot: no candidate of either model is addable (`parts.tsv`: 74_857 3 × `status Obsolete`; 74_16460 `status Obsolete` + `status Discontinued at DigiKey`; 74_162460 2 × `status Obsolete` + SN74ABT162460DL `no TI datasheet`; 74_870 3 × `status Obsolete`). A model no part uses would be untested dead code in the engine; plan-21-A's outcome ("every new device runs its datasheet behavior") has no device for them. Recorded in Corrections and `specs.md`'s header.

Acceptance: `ctest --test-dir /tmp/logic-p21` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2's `ctest`. Landing tier at the end of E.

## Decisions

- The register files are `mux` parts (plan-21-A §3.1 rejected alternatives).

## Corrections

- **Models `universal` and `bus4` are not built**: every '857, '870, '16460 and '162460 candidate is Obsolete, Discontinued at DigiKey or has no TI datasheet (Phase 2's moot line has the `parts.tsv` counts). Measured by `parts.tsv`, not assumed. If a later census finds an addable part, the model and its tests come with it.
- **51 storage/register/bus candidates → 19 added**: `stored` 12 parts (74_298 3, 74_399 5, 74_354 3, 74_356 1), `regfile` 7 (74_670). LS354, LS355, LS604 have no TI datasheet; the '870 is Obsolete.
- **`regfile` has no `"output": "3-state"` key**: the read enable already gives Z; the model takes the common `"output"` (`push-pull` default / `open-drain`) instead. `words` 2–16 and `bits` 1–8, so the '870 shape (two files) is tested with `shape_rf2`.
- **'354 E̅ polarity**: the CD74HC354 description says E "passes data when E is high", but pin 9 is E̅ and the function table, block and logic diagrams (cd74hc354 p.2–4) show the data latches transparent while E̅ is low; the table wins (`dataLatch` `{"pin": "E", "active": "high"}`, holding while high).
- **'399 added to §4** (case 19): the LS399 and CY74FCT399T load on the rising edge, the '298 on the falling edge; both edges are tested.
- **X-clock rule**: a clock that leaves its idle level for X or Z (`0 → X` for a rising clock, `1 → X` for a falling one) makes X the stored bits whose new value differs (case 18), the sequential kinds' rule.
- **'670 write-to-read latency** (about 25 ns "latch time" on the sheets) is not modelled: a written word reads back after the main path delay only (specs.md § 74_670).

## Summary

C covers the 51 storage, register-file and bus parts; `bus4`'s
bidirectional turnaround is the highest-risk model and is tested in both
directions.
