# plan-10-H: Special shift models — LS673/674, 4517B, 4034B, FIFO, LV8153

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-G

H adds the five `shift` models for the seven devices the parametric model
does not fit: `ls673` (74_673, 74_674), `tap64` (4517B), `bus-register`
(4034B), `fifo` (40105B, 74_40105) and `serial-in` (74_8153). They carry 34
parts. Each is a small state machine of its own; the FIFO and the LV8153 are
timed with engine timers.

Outcome of H: `shift_test` passes G's 23 cases and the 20 of §5; `ctest` 15
of 15; no golden change.

References: plan-10-G (kind, dispatch, `shift.h`), plan-10-B §4;
`plan-10-data/shift-b.md` §§ 74_673, 74_674, 4517B, 4034B, 40105B, 74_40105,
74_8153.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-G complete | `ls planning/completed/plan-10-G-shift-kind.md` → exists | MET (2026-11-01: exists, archived at 39c82f5) |

## 1. Goal

- `shift_test` → 43 `PASS` (23 + 20), no `FAIL`; `ctest` 15 of 15.
- No golden change; docs § `shift` lists the five models.

### Non-goals (explicit constraints)

- No golden change, no part file, no change to the `shift` model.

## 2. Current State

After G: `shift.c`/`shift.h` with the kind and the `shift` model; 23 cases.

### Measured populations

| What | Count | Command |
|---|---|---|
| Devices / parts | 7 / 34 | `parts.tsv`: 74_673 8, 74_674 6, 4517B 2, 4034B 2, 40105B 3, 74_40105 6, 74_8153 7 |

## 3. Design Overview

`backend/src/models/shift_special.c`, registered through `shift.h`. The
FIFO and the LV8153 use `sim_schedule_timer` (the FIFO's word ripple, the
LV8153's mid-bit sampling and its SOUT pulse). Rejected: modeling the FIFO
as 16 cascaded `shift` units (the per-position marker handshake is not a
clocked shift) and the LV8153 as a UART bit clock (its rate is measured from
each frame's start bits, not set).

Risk: the LV8153's frame decoding (the datasheet does not say how frame 1 is
told from frame 2 — §4.5 fixes the rule) and the FIFO's flag timing.

## 4. Detailed Design

### 4.1 Model `ls673`

| Key | Value |
|---|---|
| `"clock"` | pin; the gated clock CLK·(CS = 0) acts on its falling edge |
| `"chipSelect"` | pin CS (active low; gates both clocks literally, so a CS edge with the clock high is an edge of the gated clock) |
| `"readWrite"` | pin R/W (1 = read) |
| `"mode"` | pin MODE/STRCLK |
| `"serialIo"` | pin SER/Q15 (inout) |
| `"storageOutputs"` | 16 pins Y0–Y15 (74_673) |
| `"storageClear"` | control STRCLR low (74_673) |
| `"parallel"` | 16 pins P0–P15 (74_674) |

Behavior (`shift-b.md` §§): at a falling edge of the gated shift clock:
read mode (R/W 1) with MODE 1 → Q ← storage (673) or ← P (674); read mode
with MODE 0 → shift Q0 ← Q15 (recirculate); write mode (R/W 0) → shift Q0 ←
the SER/Q15 pin. 673: the store clock MODE/STRCLK·(CS = 0)·(R/W = 0) loads
storage ← Q on its rising edge; STRCLR low clears storage at once (not gated
by CS); storage outputs always driven. SER/Q15 drives Q15 while CS = 0 and
R/W = 1, else Z. `units` = 1. Main path: clock → SER/Q15.

### 4.2 Model `tap64`

Device key `"units"`: 1–2 objects `{"clock", "data", "writeEnable", "taps"}`
— taps = 4 pins (Q16, Q32, Q48 inout; Q64 output). Per unit: 64 stages on
CLOCK ↑, stage 1 ← DATA. WE = 0: the four taps drive stages 16/32/48/64;
stage 17 ← 16, 33 ← 32, 49 ← 48. WE = 1: all taps Z; stages 17, 33, 49 ←
the tap 16/32/48 pins at the edge. `units` = number of units. Main path:
clock → tap.

### 4.3 Model `bus-register`

| Key | Value |
|---|---|
| `"a"`, `"b"` | 8 inout pins each |
| `"clock"` | pin (rising edge) |
| `"aEnable"` | pin AE (1 enables the A side) |
| `"direction"` | pin A/B (1: A in, B out) |
| `"parallelSerial"` | pin P/S (1 parallel) |
| `"asyncSync"` | pin A/S (1 asynchronous parallel transfer) |
| `"serialIn"` | pin SI |

Behavior (`shift-b.md` § 4034B): input bus = A when A/B = 1 (with AE = 1;
AE = 0 → the register keeps itself, the "recirculation"), B when A/B = 0.
P/S = 0: CLOCK ↑ → stage 1 ← SI, stage k ← k−1 (A/S ignored). P/S = 1 and
A/S = 0: CLOCK ↑ → register ← input bus. P/S = 1 and A/S = 1: register
follows the input bus continuously. Output bus: A/B = 1 → B driven; A/B = 0
→ A driven when AE = 1, else Z. The input bus pins are never driven.
`units` = 1. Main path: clock → output bus.

### 4.4 Model `fifo`

| Key | Value |
|---|---|
| `"data"`, `"outputs"` | 4 pins each (D0–D3, Q0–Q3) |
| `"shiftIn"`, `"shiftOut"` | pins SI, SO |
| `"inputReady"`, `"outputReady"` | pins DIR, DOR |
| `"reset"` | control MR high (async) |
| `"outputEnable"` | control (40105B pin 1 "3-state control" and HC OE: enabled while low) |
| `"words"` | 16 |
| `"unloadNeedsEnable"` | bool: 40105B true — unloading uses the falling edge of (SO OR the disable), so it is inhibited while the outputs are disabled and an enable with SO low unloads; HC false — SO alone |
| `"loadWhileHigh"` | bool: HC true — a word is taken when position 1 frees while SI is still high, and after reset with SI high |

Timing extra key `"fallThrough"` (ns): the datasheet's ripple-through time
from position 1 to 16 — 40105B (family C) 2000 typ at VDD = 5 V; 74_40105 HC
and HCT 400 (SI → DOR, the 25 °C maximum at 4.5 V, the only figure printed;
`shift-b.md` § 74_40105). One position step = fallThrough / 15.

State: 16 positions, each `full` + 4 data bits. SI ↑ (or the
`loadWhileHigh` case) with position 1 empty → position 1 ← D, full. Every
step time, every full position whose successor is empty moves forward one
(processed from the output end, one move per position per step, so words
ripple and bubbles rise). DIR = NOT position 1 full; DOR = position 16 full;
outputs show position 16's data (holding the last word while DOR is low),
through the output enable. Unload (SO ↓, or the 40105B rule) with DOR high
→ position 16 empties. MR → every position empty (data kept, invalid), DIR
1, DOR 0. Flag and output changes use the entry's rise/fall. `units` = 1.
Main path: SO → Q.

### 4.5 Model `serial-in` (SN74LV8153)

| Key | Value |
|---|---|
| `"data"` | pin D (idle high) |
| `"address"` | 3 pins A0–A2 |
| `"outputs"` | 8 pins Y0–Y7 |
| `"outputSelect"` | pin OUTSEL (1 push-pull, 0 open-collector inverted) |
| `"reset"` | control RESET low (async: clears the register) |
| `"outputEnable"` | control OE (outputs on while low) |
| `"serialOut"` | pin SOUT |

Frame decoding (`shift-b.md` § 74_8153): a D falling edge while idle starts
a frame at t0; the next D rising edge at t1 gives the bit period P = t1 − t0
(ST0 → ST1). Bits k = 2…8 (A0, A1, A2, then the four data bits) are sampled
at t0 + (k + ½)·P by timers; the stop bit is not checked. A frame whose
address bits differ from the A0–A2 pins is ignored. Frames alternate first /
second, starting with first after power-on and after RESET: a first frame
stores its nibble (D0–D3) and address; a second frame whose address equals
the first's completes the byte (D4–D7) — the 8-bit register loads at its D7
sample point and SOUT goes 0 for one period P from then; a second frame
whose address differs drops the pair, and the next frame is a first one.
(The datasheet does not say how the chip tells the frames apart; alternation
from reset is the reading that makes its two-frame protocol work.)

Outputs: OE inactive → Z; OUTSEL 1 → Y = bit (push-pull); OUTSEL 0 → Y = 0
when the bit is 1, else Z (open collector, inverted). RESET low → register
0. `part_pin_open_drain` → 0 for Y (OUTSEL can make them push-pull). Both
VCC1 and VCC2 are power pins under one range, 3–5.5 V (VCC2's 13.2 V
maximum is not modeled: the voltage check has one range per part). `units`
= 1. Main path: RESET → Y.

## 5. `shift_test` cases (added to G's)

| # | Fixture | Case |
|---|---|---|
| 24 | 74_673 | write mode shifts SER pin in on the gated clock's falling edge |
| 25 | 74_673 | store clock (R/W 0) copies Q to Y; STRCLR clears Y with CS high |
| 26 | 74_673 | read mode MODE 1 loads Q from Y; MODE 0 recirculates; SER/Q15 driven only in read with CS 0 |
| 27 | 74_673 | CS falling with the clock high is a falling gated-clock edge (the datasheet's warning) |
| 28 | 74_674 | MODE 1 in read mode loads P0–P15 |
| 29 | 4517B | 64 clocks from D to Q64 |
| 30 | 4517B | WE 1: taps Z; tap 16 pin feeds stage 17 |
| 31 | 4034B | serial shift into B outputs (A/B 1) |
| 32 | 4034B | sync parallel load from A on the edge; async load follows A; AE 0 recirculates |
| 33 | 4034B | A/B 0: B is the input, A driven only with AE 1 |
| 34 | 40105B | write one word into an empty FIFO: DOR high and Q valid after the fall-through time |
| 35 | 40105B | 16 writes fill it: DIR stays low; a 17th SI is ignored |
| 36 | 40105B | SO ↓ unloads; DOR low then high when the next word arrives |
| 37 | 40105B | MR empties it: DIR 1, DOR 0 |
| 38 | 40105B | output disable blocks unloading; enabling with SO low unloads (`unloadNeedsEnable`) |
| 39 | 74_40105 | SO unloads while disabled; SI held high loads when position 1 frees |
| 40 | 74_8153 | two frames at 9600 bit/s with A = 5 and pins A = 5 → Y = the byte at the D7 sample point; SOUT low for one bit |
| 41 | 74_8153 | a frame with another address is ignored; a pair with mismatched addresses is dropped |
| 42 | 74_8153 | OUTSEL 0: Y = 0 for 1 bits, Z for 0 bits; OE high → Z; RESET low → 0 |
| 43 | 74_8153 | the bit rate is re-measured per frame (2400 then 19200 bit/s frames both decode) |

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — `ls673`, `tap64`, `bus-register`

- [x] `models/shift_special.c` (listed in `backend/CMakeLists.txt`) with the
  three models; docs. (`cc -Wall -Wextra -fsyntax-only` on `shift_special.c`
  and `shift.c` prints nothing.)

Acceptance: `ctest` 15 of 15 (seconds). (Ran: `100% tests passed, 0 tests
failed out of 15`; golden status empty.)
Commit: b841505

### Phase 2 — `fifo`, `serial-in`

- [x] Both models; docs.

Acceptance: `ctest` 15 of 15. (Same run as Phase 1: all five models were
written in one pass of `shift_special.c`; one commit, see Corrections.)
Commit: b841505

### Phase 3 — Cases

- [x] §5 cases in `shift_test`. (7 more fixture devices from
  `/tmp/p10/shift_devs_h.py`; the FIFO fixtures' `fallThrough` is 1500 ns,
  100 ns per step, so each check has margin.)

Acceptance: `shift_test` → 43 `PASS`, 0 `FAIL`; `ctest` 15 of 15. (Ran: 43
`PASS`, no `FAIL`; `100% tests passed, 0 tests failed out of 15`; golden
status empty.)
Commit: 4e82901

## Validation Plan

Phase 3's acceptance. Landing tier: plan-10-J.

## Decisions

- LV8153 frames alternate first/second from reset (§4.5).
- LV8153's two supply pins share one range (§4.5).
- FIFO words move one position per fallThrough/15 (the datasheet gives only
  the total ripple time).

## Corrections

- 2026-11-01, Phases 1–2: the five models land in one commit (one file, one
  `ctest` 15 of 15 run for both phases' identical acceptance).
- 2026-11-01, Phase 2: an unknown (X) FIFO master reset is taken as a reset
  (every marker empty), and an LV8153 frame whose sampled address bits are
  unknown is ignored like a frame for another device; §4.4/§4.5 did not say.
- 2026-11-01, Phase 3: case 27 as written ("CS falling with the clock high")
  inverts the datasheet: the gated clock is CLK·(CS = 0), so CS falling with
  CLK high is a *rising* gated edge; CS *rising* with CLK high is the false
  falling edge the datasheet warns about ("the shift clock should be low
  during the low-to-high transition of chip select"). The case tests CS
  rising. Its first run also caught the power-up X → 0 of the gated clock
  acting as a falling edge (Verilog negedge, §3.5), so the case settles SER
  low first.
- 2026-11-01, Phase 3: §4.5's "a second frame whose address differs drops
  the pair" cannot happen: a frame whose address differs from the A0–A2 pins
  is ignored before pairing, and the pins are the same for both frames.
  Case 41 tests the reachable rule (other-address frames are ignored, even
  between the two halves of a pair); the model keeps the drop branch for an
  address-pin change between the frames.

## Summary

H adds the five one-off shift models — LS673/674, 4517B, 4034B, the FIFO
and the LV8153 — with 20 cases, completing the `shift` kind.
