# plan-10-E: Counter models — Johnson, 7-segment, divide-by-N, rate multipliers

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-D

E adds the four remaining `counter` models — `johnson` (4017B, 4022B,
74_4017, 4018B), `seg7` (4026B, 4033B, 40110B), `divn` (4059A, 74_4059) and
`rate` (4089B, 4527B, 74_97) — and their `counter_test` cases. These are the
12 devices (62 parts) whose behavior is not a binary/BCD count with carries.

Outcome of E: `counter_test` passes D's 29 cases and the 24 of §5; `ctest`
14 of 14; no golden change.

References:

- plan-10-D (the kind, dispatch, fixtures style), plan-10-B §4 (helpers).
- `plan-10-data/divider.md` (4017B, 4022B, 74_4017, 4018B, 4059A, 74_4059,
  4089B, 4527B, 74_97), `counter-c.md` § 4026B / 4033B, § 40110B.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-D complete | `ls planning/completed/plan-10-D-counter-kind.md` → exists | NOT MET (2026-11-01: written) |

## 1. Goal

- `counter_test` → 53 `PASS` (29 + 24), no `FAIL`; `ctest` 14 of 14.
- `git diff --stat backend/tests/golden` → no change.
- `docs/parts-format.md` § `counter` documents the four models.

### Non-goals (explicit constraints)

- No golden change, no part file, no change to `sync` or `ripple` behavior.

## 2. Current State

After D: `counter.c` with the kind and `sync`/`ripple`; `counter_test` with
29 cases.

### Measured populations

| What | Count | Command |
|---|---|---|
| Devices / parts in E's models | 12 / 62 | `parts.tsv` device counts: 4017B 13, 4022B 7, 74_4017 10, 4018B 10, 4026B 5, 4033B 2, 40110B 1, 4059A 2, 74_4059 1, 4089B 4, 4527B 4, 74_97 3 |

## 3. Design Overview

Four models in `backend/src/models/counter_div.c`; `counter.c` (D) keeps the
kind, the dispatch, `sync` and `ripple`. The two files share
`models/counter.h` (internal: the `CounterModel` struct — name, keys,
load/bind/create/start/pin_changed/timer — and the four `extern` model
entries the dispatch table in `counter.c` lists). Each model keeps its state
as small integers (Johnson stages as a bit vector with a known mask, decade
digit, down-counter value). Rejected:
expressing 4017/4026 as `sync` bcd counters with decoded outputs — the
Johnson anti-lock recovery from illegal states and the 4018's external
feedback need the stage-level model.

Risk: `divn`'s master-preset and extra-count sequencing (no gate-level
diagram printed; §4.3 fixes the reading) and the rate multipliers' pulse
patterns (§4.4 gives every selected-state set).

## 4. Detailed Design

### 4.1 Model `johnson`

| Key | Value |
|---|---|
| `"stages"` | 4 or 5 |
| `"clock"` | pin; the stages advance on the rising edge of the gated clock |
| `"clockGate"` | optional `{"pin", "op"}` (D §4; 4017/4022 INHIBIT and 74_4017 CE use `and-not`) |
| `"reset"` | control, asynchronous: every stage 0 |
| `"outputs"` | optional: 2·stages decoded pins in count order |
| `"carry"` | optional pin: NOT the last stage |
| `"data"` | optional pin (4018B): stage 1's D is this pin; without it, NOT the last stage |
| `"presetEnable"` | optional control, asynchronous (4018B): stage n = `jam`[n] |
| `"jam"` | pin names, one per stage (with `presetEnable`) |
| `"qbar"` | optional: one pin per stage, NOT the stage (4018B's outputs) |

Next state at the edge: D1 = data pin or ¬Q_S; D2 = Q1; D3 = Q2·(Q1 + Q3)
(the anti-lock gate, every device); Dk = Q(k−1) for k ≥ 4. Decoded outputs
(S stages, Q1…QS): output 0 = ¬Q1·¬QS; output k (1 ≤ k < S) = Qk·¬Q(k+1);
output S = Q1·QS; output S+k (1 ≤ k < S) = ¬Qk·Q(k+1) (`divider.md` § 4017B,
§ 4022B). Priority: reset > preset enable > clock. `units` = 1. Main path:
clock → decoded output.

| Device | Keys |
|---|---|
| `4017B` | stages 5; clock CLOCK, clockGate {CLOCK INHIBIT, and-not}; reset RESET high; outputs "0"…"9"; carry CARRY OUT |
| `4022B` | stages 4; as 4017B with outputs "0"…"7" |
| `74_4017` | stages 5; clock CP, clockGate {CE, and-not}; reset MR high; outputs; carry TC |
| `4018B` | stages 5; clock CLOCK; reset RESET high; data DATA; presetEnable PRESET ENABLE high; jam JAM1–JAM5; qbar Q̄1–Q̄5 (names per the spec section) |

### 4.2 Model `seg7`

A decade counter 0–9 with decoded 7-segment outputs.

| Key | Value |
|---|---|
| `"clock"` + optional `"clockGate"` | as `johnson` (4026B, 4033B); or `"clockUp"` and `"clockDown"` pins (40110B: every rising edge of either counts, independently) |
| `"reset"` | control, asynchronous: count 0 |
| `"segments"`, `"glyphs"` | 7 pins a…g; 10 strings of 7 `0`/`1` |
| `"carry"` | optional pin: 1 for counts 0–4, 0 for 5–9 (4026B, 4033B) |
| `"carryPulse"`, `"borrowPulse"` | optional pins (40110B): normally 1; a 0 pulse when the count steps 9→0 (carry) or 0→9 (borrow), as long as the timing entry's extra keys `carryPulse` / `borrowPulse` (ns) |
| `"displayEnable"` | optional `{"in", "out"}` pins (4026B): segments forced 0 while `in` is 0; `out` = `in` |
| `"ungatedC"` | optional pin (4026B): segment c of the count, not gated |
| `"lampTest"` | optional control (4033B): every segment 1 |
| `"rippleBlank"` | optional `{"in", "out"}` (4033B): count 0 and `in` = 0 → segments 0; `out` = NOT(count = 0 AND `in` = 0) |
| `"toggleEnable"` | optional control (40110B): counting only while asserted |
| `"latchEnable"` | optional control (40110B): while asserted the segments hold the count they showed; the counter keeps counting |

Priority: reset > counting. Segments: lamp test > (display enable 0 or ripple
blank) > glyph. `units` = 1. Main path: clock → segment.

| Device | Keys |
|---|---|
| `4026B` | clock CLOCK, clockGate {CLOCK INHIBIT, and-not}; reset RESET high; glyphs = `counter-c.md` § 4026B table (6 and 9 with tails); carry CARRY OUT; displayEnable {DISPLAY ENABLE IN, DISPLAY ENABLE OUT}; ungatedC |
| `4033B` | as 4026B without displayEnable/ungatedC; lampTest LAMP TEST high; rippleBlank {RBI, RBO} |
| `40110B` | clockUp CLOCK UP, clockDown CLOCK DOWN; reset RESET high; toggleEnable TOGGLE ENABLE low; latchEnable LATCH ENABLE high; carryPulse CARRY, borrowPulse BORROW (timing extras 230 / 275 ns at 5 V); glyphs as 4026B |

### 4.3 Model `divn`

| Key | Value |
|---|---|
| `"clock"` | pin (rising edge) |
| `"jam"` | 16 pins, J1…J16 |
| `"modes"` | 3 pins Ka, Kb, Kc |
| `"latchEnable"` | pin |
| `"output"` | pin |

Mode (Ka Kb Kc, `divider.md` § 4059A Table I): 111 → ÷2 (first-section digit
d1 = J1; last-section digit d5 = J2–J4), 011 → ÷4 (d1 = J1–J2; d5 = J3–J4),
101 → ÷5 (d1 = J1–J3; d5 = J4), 001 → ÷8 (d1 = J1–J3; d5 = J4), 110 → ÷10
(d1 = J1–J4; d5 = 0), X00 → master preset. Digits are binary, J1/J5/J9/J13
the LSBs: d2 = J5–J8, d3 = J9–J12, d4 = J13–J16 (values above 9 allowed, the
"extended" range). N = mode × (1000·d5 + 100·d4 + 10·d3 + d2) + d1, and N < 3
counts as 3 ("can never be preset to a count less than 3").

At each rising clock edge (Ka, Kb, Kc and the jam pins sampled then):

1. Master preset: C = N; `preset` = 1; output 0 (unless latched).
2. Else if `preset` = 1: `preset` = 0 and C is unchanged (the datasheet's
   one extra count).
3. Else if C ≤ 1: C = N and the output is 1 until the next edge (a one-clock
   pulse); else C = C − 1 and the output is 0.

Latch enable 1: once the output has gone 1 it stays 1 until latch enable
returns to 0. Power-on: C = N from the pins at start, `preset` = 0. Unknown
mode or jam bits at an edge → C unknown and the output X until the next
master preset. `units` = 1. Main path: clock → output. Both devices
(`4059A`, `74_4059`) use these keys with their spec section's names.

### 4.4 Model `rate`

| Key | Value |
|---|---|
| `"variant"` | `"binary4"` (4089B, count 0–15), `"bcd"` (4527B, 0–9), `"binary6"` (74_97, 0–63) |
| `"clock"` | pin (counts on the rising edge) |
| `"rate"` | pins, LSB first (A…D; B0…B5) |
| `"enable"` | control: counts while asserted (4089/4527 INHIBIT IN low; 97 EN low) |
| `"strobe"` | control: asserted → no pulses |
| `"cascade"` | control: asserted → `out` forced 1 (4089/4527 CASCADE high; 97 UNITY/CAS low) |
| `"clear"`, `"set"` | controls, asynchronous; set (4089 to 15, 4527 to 9) beats clear; 97 has no set |
| `"out"`, `"outBar"` | pins: OUT / OUT̄ (97: Y / Z) |
| `"terminal"` | optional pin: 1 while count = max ("15", "9") |
| `"enableOut"` | pin: 0 while enable is asserted and count = max (INHIBIT OUT; 97 ENABLE) |

G = (strobe not asserted) · (clock = 0) · (any rate bit set whose state set
contains the count). `outBar` = ¬G; `out` = G + cascade asserted. State sets
(`divider.md` §§): binary, rate bit j counted from the MSB (j = 0 for the
MSB): the counts whose low j+1 bits are 2^j − 1 (MSB: even counts; next:
count mod 4 = 1; next: mod 8 = 3; …; LSB: count = 2^(n−1) − 1). bcd: A →
{4}, B → {2, 7}, C → {1, 3, 6, 8}, D → {0, 1, 2, 3, 5, 6, 7, 8}. `units` = 1.
Main path: clock → out.

## 5. `counter_test` cases (added to D's)

| # | Fixture | Case |
|---|---|---|
| 30 | 4017B | each output high for one clock in order 0…9; carry high for 0–4 |
| 31 | 4017B | INHIBIT high blocks; INHIBIT falling with CLOCK high advances |
| 32 | 4017B | RESET async → output 0 high |
| 33 | 4018B | preset JAM = 00100 (an illegal state), DATA ← Q̄5 by a bench wire, then clock: the legal cycle is reached within 10 clocks (the anti-lock gate) |
| 34 | 4022B | 8 outputs, carry high for 0–3 |
| 35 | 74_4017 | CE low counts; MR high resets |
| 36 | 4018B | DATA ← Q̄5: ÷10; DATA ← Q̄4·Q̄5 (bench AND): ÷9 skipping 11111 |
| 37 | 4018B | RESET beats PRESET ENABLE |
| 38 | 4026B | glyph per count 0–9; DEI low → segments 0, ungated C still follows |
| 39 | 4033B | RBI low at 0 → blank and RBO low; LAMP TEST high → all 1 |
| 40 | 40110B | CLOCK UP and CLOCK DOWN counts; 9→0 gives a 230 ns CARRY low pulse; 0→9 a 275 ns BORROW pulse |
| 41 | 40110B | LATCH ENABLE high holds the display while the count moves; TOGGLE ENABLE high blocks counting |
| 42 | 4059A | ÷8 mode, N = 3 (J1–J3 = 3, others 0): master preset, release, first pulse on the 4th edge, then every 3rd |
| 43 | 4059A | the datasheet example A: mode ÷5, J1–J16 = 0011 1010 1001 0110 → pulses 8479 edges apart (the bench clocks 2 × 8479 edges; milliseconds of run time) |
| 44 | 4059A | latch enable 1 keeps the output high after a pulse |
| 45 | 4059A | ÷10 mode N = 25: pulses every 25 edges |
| 46 | 4089B | rate 13 → 13 OUT pulses per 16 clocks, each during clock low |
| 47 | 4089B | STROBE high → no pulses, OUT̄ high; CASCADE high → OUT high |
| 48 | 4089B | SET → count 15, "15" high, INHIBIT OUT low; CLEAR → 0 |
| 49 | 4527B | rate 8 → 8 pulses per 10; rates 10–15 → 8 or 9 per the truth table |
| 50 | 74_97 | M = 40 → 40 Y pulses per 64 clocks; UNITY/CAS low → Y high |
| 51 | 74_97 | ENABLE out low at count 63 with EN low |
| 52 | loader | `divn` with 15 jam pins → error; `rate` `"variant": "x"` → error |
| 53 | loader | `seg7` `"glyphs"` with 9 strings → error |

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — `johnson` and `seg7`

- [ ] `models/counter_div.c` (listed in `backend/CMakeLists.txt`) with both
  models; docs. Acceptance: `ctest` 14 of 14 (seconds).

Commit: —

### Phase 2 — `divn` and `rate`

- [ ] Both models; docs.

Acceptance: `ctest` 14 of 14 (seconds).
Commit: —

### Phase 3 — Cases

- [ ] The cases of §5 in `counter_test`.

Acceptance: `counter_test` → 53 `PASS`, 0 `FAIL`; `ctest` 14 of 14.
Commit: —

## Validation Plan

Phase 3's acceptance. Landing tier: plan-10-J.

## Decisions

- Power-on for `divn` loads N from the pins (the chip's counter starts
  somewhere; loading N means the first pulse comes N clocks after start,
  which is what a user expects from "divide by N").
- The 40110B carry/borrow pulse widths are timing-entry extras (they scale
  with VDD in the datasheet; one entry per family, as for every delay).

## Corrections

(none yet)

## Summary

E adds the Johnson, 7-segment, programmable-divider and rate-multiplier
counter models with 24 cases, completing the `counter` kind before its parts.
