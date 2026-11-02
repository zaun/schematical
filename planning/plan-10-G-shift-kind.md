# plan-10-G: The `shift` kind and the shift-register model

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-F

G adds the `shift` kind and its main model, `shift`: a parametric shift
register with serial and parallel inputs, mode selection, a storage register
or latch, extra serial outputs and shared I/O pins. 29 of the 36 shift
devices (549 of 583 parts) use it. H adds the five special models; I adds
the parts.

Outcome of G: `shift_test` passes every case of §5; `ctest` 15 of 15; no
golden change.

References: plan-10-A §3, plan-10-B §4 (helpers, bench), plan-10-D §4
(clock gating helpers); `plan-10-data/shift-a.md`, `shift-b.md`.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-F complete | `ls planning/completed/plan-10-F-counter-parts.md` → exists | NOT MET (2026-11-01: written) |

## 1. Goal

- `ctest` → `100% tests passed, 0 tests failed out of 15`.
- `shift_test` → one `PASS` per case of §5, no `FAIL`.
- `git diff --stat backend/tests/golden` → no change; docs § `shift`.

### Non-goals (explicit constraints)

- No golden change; no part file. `PART_SHIFT` appended after `PART_COUNTER`.

## 2. Current State

After F: kinds `latch` and `counter`, `seq.c` with control, clock and
gating helpers, the bench; `TRACE_LAST_KIND` = `PART_COUNTER`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `shift`-model devices / parts | 29 / 549 | §4.2 table; `parts.tsv` device counts: the 583 shift parts minus H's 7 devices (74_673 8, 74_674 6, 4517B 2, 4034B 2, 40105B 3, 74_40105 6, 74_8153 7 = 34) |
| Longest register | 64 stages (4031B) | `shift-b.md` § 4031B |

## 3. Design Overview

`backend/src/models/shift.c` holds the kind, the dispatch and the `shift`
model; `models/shift.h` (internal) declares the `ShiftModel` struct for H's
`shift_special.c`. Each unit keeps its stages, its storage register and its
extra serial-output flip-flops as `SeqBits` (≤ 64 bits).

Rejected: separate models for "SIPO", "PISO", "universal" and "with
storage": the same stage array with different input and output wiring; one
model with optional keys keeps one shifting core under 29 devices.

Risk: the 595-class storage timing (tied clocks give the storage the pre-
shift contents), the 299/323 I/O direction rules, and the TPIC clear scope.
Each has its own case.

## 4. Detailed Design

### 4.1 Model `shift`

Device keys: `"units"` (1–2 unit objects). `Part.logic.units` = number of
units. Unit keys:

| Key | Value |
|---|---|
| `"stages"` | 1–64 |
| `"clock"` | `{"pin", "edge"}` |
| `"clockGate"` | optional `{"pin", "op"}` (165/166 CLK INH: `or`) |
| `"serial"` | pin names ANDed into stage 1 on a right shift (164: [A, B]) — or `"serialJK"` `{"j", "kbar"}` (stage 1 ← J·¬Q1 + K̄·Q1) — or `"serialSelect"` `{"select", "low", "high"}` (stage 1 ← the `low` pin while `select` is 0, the `high` pin while 1); exactly one of the three |
| `"leftSerial"` | optional pin: stage N's input on a left shift |
| `"modes"` | optional `{"pins": [S0, S1], "table": [4 of "hold", "right", "left", "load"]}`, row = S0 + 2·S1; without it every active edge shifts right unless `load` says otherwise |
| `"enables"` | optional controls, all asserted for an edge to act (sampled at the edge) |
| `"load"` | optional `{"pin", "active", "when": "async"|"clock", "from": "data"|"inputRegister"|"io"}`; async = level-transparent |
| `"data"` | parallel input pins in stage order (with `from` data, or a `modes` load) |
| `"clear"` | optional `{"pin", "active", "when"}`: stages 0 |
| `"clearScope"` | `"shift"` (default) or `"all"`: the clear also clears `storage` and `serialOutputs` |
| `"outputs"` | `[{"stage", "pin", "invert"}]` (stage 1-based; `invert` default false) |
| `"trueComplement"` | optional control (4035B): while not asserted, every `outputs` pin shows NOT its stage |
| `"io"` | optional `{"pins": [stage names, inout], "outputEnables": [controls, all asserted], "notInModes": [rows]}` — the stages drive the pins while every enable is asserted and the current `modes` row is not listed; `load` `from` io and the input register read the pins |
| `"serialOutputs"` | optional `[{"pin", "stage"}]`: a flip-flop that loads that stage on the clock's opposite edge (4094 Q'S, 4031 Q', TPIC 596 SER OUT) |
| `"storage"` | optional `{"clock": {"pin","edge"}` or `"latch": control` (transparent while asserted), `"clear": control` (async), `"outputs": [stages names]`, `"outputEnable": control`, `"output": "push-pull"|"open-drain"`, `"invert": bool}` |
| `"inputRegister"` | optional `{"clock": {"pin","edge"}, "data": [names]}` (absent `data` → the `io` pins) |
| `"clockOut"` | optional pin: follows the clock pin's level, delayed by the entry's rise/fall (4031B CLD) |

Order of evaluation per unit, on any change:

1. Async clear (stages; with `clearScope` all also storage and serial-output
   flip-flops), then async load (level-transparent from its source). Clear
   and load both asserted → clear wins (74_597/598 "invalid": the safe
   choice, recorded in docs).
2. Shift clock (gated) edge, no async control asserted, `enables` asserted:
   clock clear, else clock load, else the `modes` row (hold / right / left /
   load from `data` or `io`), else (no modes) right shift. Right: stage 1 ←
   serial input, stage k ← stage k−1. Left: stage N ← `leftSerial`, stage k
   ← stage k+1.
3. Storage clock edge: storage ← the stages as they were before any shift at
   the same instant (a shift and a storage edge in the same `pin_changed`
   batch: the storage samples first — the tied-clock rule, `shift-b.md`
   § 74_595). Storage latch asserted: storage follows the stages. Storage
   clear (async) → 0.
4. Opposite clock edge: each `serialOutputs` flip-flop ← its stage.
5. Input register edge: ← its data or the `io` pins.
6. Outputs re-evaluated: `outputs` (with `trueComplement`), `io` drive,
   `storage.outputs` (enable, open drain, invert), `serialOutputs`,
   `clockOut`.

Main timing path: clock → Q (the storage clock → Q for storage devices,
the shift clock → the serial output for parallel-in serial-out devices).

`part_pin_open_drain`: 1 for `storage.outputs` pins of an `open-drain`
storage.

### 4.2 Devices

| Device | Keys (names from the spec section) |
|---|---|
| `74_164` | 8; CLK ↑; serial [A, B]; clear CLR low async; outputs QA–QH |
| `74_264` | as 164, every output `invert` |
| `74_165` | 8; CLK ↑, clockGate {CLK INH, or}; serial [SER]; load SH/LD low async from data A–H; outputs {8, QH}, {8, QHBAR, invert} |
| `74_166` | 8; CLK ↑, clockGate {CLK INH, or}; serial [SER]; load SH/LD low clock from data A–H; clear CLR low async; outputs {8, QH} |
| `74_194`, `40194B` | 4; CLK ↑; modes [S0, S1] [hold, right, left, load]; serial [SR SER] (40194B: SHIFT RIGHT); leftSerial SL SER (SHIFT LEFT); data A–D (D0–D3); clear CLR low (RESET low) async; outputs QA–QD (Q0–Q3) |
| `74_195` | 4; CP ↑; serialJK {J, K}; load PE low clock from D0–D3; clear MR low async; outputs Q0–Q3, {4, Q3BAR, invert} (CD names) |
| `74_299` | 8; CLK ↑; modes [S0, S1] [hold, right, left, load]; serial [SR], leftSerial SL; load via the `load` row from io; clear CLR low async; io {A/QA–H/QH, [OE1 low, OE2 low], notInModes [load]}; outputs {1, QA'}, {8, QH'} (CD/LS names per the section) |
| `74_323` | as 299 with clear `when` clock |
| `4014B` | 8; CLOCK ↑; serial [SERIAL IN]; load P/S high clock from PI-1–PI-8; outputs {6, Q6}, {7, Q7}, {8, Q8} |
| `4021B` | as 4014B with load async |
| `4015B`, `74_4015` | 2 units of 4; CLOCK ↑; serial [D]; clear RESET (MR) high async; outputs Q1–Q4 (Q0–Q3) |
| `4035B` | 4; CLOCK ↑; serialJK {J, K̄}; load P/S high clock from PI-1–PI-4; clear RESET high async; outputs Q1–Q4; trueComplement T/C high |
| `4031B` | 64; CL ↑; serialSelect {MODE CONTROL, low DATA, high RECIRCULATE}; outputs {64, Q}, {64, QBAR, invert}; serialOutputs [{Q', 64}]; clockOut CLD |
| `74_595` | 8; SRCLK ↑; serial [SER]; clear SRCLR low async; storage {RCLK ↑, outputs QA–QH, outputEnable OE low}; outputs {8, QH'} |
| `74_594` | as 595 with storage {RCLK ↑, clear RCLR low, outputs QA–QH} (no OE) |
| `74_596` | as 595 with storage `output` open-drain |
| `74_597` | 8; SRCLK ↑; serial [SER]; inputRegister {RCLK ↑, data A–H}; load SRLOAD low async from inputRegister; clear SRCLR low async; outputs {8, QH'} |
| `74_598` | as 597 with serialSelect {DS, low SER0, high SER1}; enables [SRCKEN low]; io {A/QA–H/QH, [G low]}; inputRegister from io |
| `TPIC6595`, `TPIC6A595`, `TPIC6B595` | 8; SRCK ↑; serial [SER IN]; clear SRCLR low async (scope shift); storage {RCK ↑, outputs DRAIN0–DRAIN7, outputEnable G low, open-drain, invert}; outputs {8, SER OUT} |
| `TPIC6C595` | as TPIC6595 with clear CLR, clearScope all |
| `TPIC6A596`, `TPIC6B596`, `TPIC6C596` | as TPIC6C595 with SER OUT as serialOutputs [{SER OUT, 8}] instead of an output |
| `4094B`, `74_4094` | 8; CLOCK (CP) ↑; serial [DATA (D)]; storage {latch STROBE (STR) high, outputs Q1–Q8, outputEnable OUTPUT ENABLE (OE) high}; outputs {8, QS (QS1)}; serialOutputs [{Q'S (QS2), 8}] |

That is 29 devices (`74_194`/`40194B`, `4015B`/`74_4015`, the TPIC rows and
`4094B`/`74_4094` count two or three each).

## 5. `shift_test`

Fixtures as B §6.2 (kind `shift`). Cases:

| # | Fixture | Case |
|---|---|---|
| 1 | 74_164 | shifts A·B in on CLK ↑; CLR low async clears |
| 2 | 74_264 | outputs inverted; after clear all 1 |
| 3 | 74_165 | SH/LD low loads and is transparent (H → QH follows); CLK INH high blocks; CLK INH ↑ with CLK low shifts |
| 4 | 74_166 | SH/LD low loads at the edge; CLR async |
| 5 | 74_194 | each S1 S0 row: hold, right, left, load |
| 6 | 74_195 | J/K̄ rows: set, reset, toggle, keep; PE low loads |
| 7 | 74_299 | I/O pins driven only with OE1 = OE2 = 0 and not both S high; load reads the pins; QA'/QH' always driven |
| 8 | 74_323 | CLR clears only at the edge |
| 9 | 4014B / 4021B | sync vs async parallel load |
| 10 | 4015B | two independent units; RESET high |
| 11 | 4035B | T/C low inverts outputs at once |
| 12 | 4031B | 64-clock delay to Q; Q' half a clock later; CLD follows CL; MODE 1 recirculates |
| 13 | 74_595 | shift and storage separate; OE high → Z, QH' still driven; SRCLR clears the shift register only |
| 14 | 74_595 | SRCLK and RCLK tied: storage one clock behind |
| 15 | 74_594 | RCLR clears the storage only |
| 16 | 74_596 | storage bit 0 → output 0, bit 1 → Z |
| 17 | 74_597 | RCLK ↑ loads the input register; SRLOAD low copies it; clear beats load |
| 18 | 74_598 | DS selects SER0/SER1; SRCKEN high blocks shifting; G low drives the stages onto the pins |
| 19 | TPIC6B595 | DRAIN low ⇔ G low and storage bit 1; SRCLR clears the shift register, not the storage |
| 20 | TPIC6B596 | SER OUT changes on SRCK falling; SRCLR clears storage too |
| 21 | 4094B | STROBE high: storage follows; OE low → Z; QS on CLOCK ↑, Q'S on CLOCK ↓ |
| 22 | loader | two of `serial`/`serialJK` → error; `storage` with both `clock` and `latch` → error; `stages` 65 → error |
| 23 | voltage | `part_pin_open_drain` → 1 on FX-TPIC6B595 DRAIN0, FX-74_596 QA; 0 on FX-74_595 QA |

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — Kind and model

- [ ] `PART_SHIFT`; `models/shift.{c,h}` (`shift.c` listed in
  `backend/CMakeLists.txt`); `kinds.c`; test support
  (`listing_kind_name` `"shift"`, `print_details`, `logic` condition,
  `TRACE_LAST_KIND` = `PART_SHIFT`); `part_pin_open_drain`; docs § `shift`.

Acceptance: `ctest` 14 of 14; golden diff empty (seconds).
Commit: —

### Phase 2 — `shift_test`

- [ ] `backend/tests/shift_test.c`, the 23 cases; `logic_test(shift_test)`.

Acceptance: `shift_test` → 23 `PASS`; `ctest` 15 of 15 (seconds).
Commit: —

## Validation Plan

Phase 2's acceptance. Landing tier: plan-10-J.

## Decisions

- The TPIC6C595's SER OUT follows its logic diagram (SRCK rising), not the
  switching table's "SRCK↓" row, which `shift-b.md` flags as copied from the
  6C596.
- TPIC6595/6A595/6B595 SRCLR clears the shift register only (logic
  diagrams), not "all registers" (their text); `shift-b.md` § TPIC6595.

## Corrections

(none yet)

## Summary

G adds the `shift` kind and the parametric shift-register model covering 29
devices, with 23 cases.
