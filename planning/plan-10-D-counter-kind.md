# plan-10-D: The `counter` kind — synchronous and ripple counters

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-C

D adds the `counter` kind with its two most-used models: `sync` (33 devices:
every synchronous counter, including the counters with input or output
registers, and the 4522B divider) and `ripple` (12 devices). `counter_test`
proves both on inline fixtures copied from the real devices. E adds the
remaining four counter models; F adds the parts.

Outcome of D: `counter_test` passes every case of §6; `ctest` 14 of 14; no
golden change.

References:

- plan-10-A §3 (design), plan-10-B §4 (shared helpers, bench, format
  section) — D only adds to `seq.c`.
- `plan-10-data/counter-a.md`, `counter-b.md`, `counter-c.md`, `divider.md`
  § 4522B.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-C complete | `ls planning/completed/plan-10-C-latch-parts.md` → exists | MET (2026-11-01: exists, archived at 36da6ae) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p10` → `100% tests passed, 0 tests failed out of 14`.
- `counter_test` → one `PASS` per case of §6, no `FAIL`.
- `git diff --stat backend/tests/golden` → no change.
- `docs/parts-format.md` § `counter` documents `sync` and `ripple`.

### Non-goals (explicit constraints)

- No existing golden line changes; no part file.
- `PART_COUNTER` appended after `PART_LATCH`; no new info struct (`Part.logic`).

## 2. Current State

- After C: `latch` kind registered, `seq.{h,c}` and `tests/support/bench.{h,c}`
  exist; `trace_check.c` uses `TRACE_LAST_KIND` (= `PART_LATCH`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `sync` devices / parts | 33 / 450 | §5.1 table rows; parts by device from `parts.tsv` (sum of the device counts) |
| `ripple` devices / parts | 12 / 218 | §5.2 table; same |
| Widest sync counter | 16 bits × 2 units (`74_8154`) | `counter-b.md` § 74_8154 |
| Longest ripple chain | 14 stages (`4020B`, `74_4020`) | `counter-b.md` § 4020B |

## 3. Design Overview

`backend/src/models/counter.c` holds the kind, the dispatch (as `latch.c`)
and the `sync` and `ripple` models; `models/counter.h` (internal) declares the
`CounterModel` struct (name, keys, load/bind/create/start/pin_changed/timer)
so E can add its four models in `counter_div.c`. Both models keep the count per unit
as a `SeqBits` (value + known mask), so an X input degrades exactly the bits
it could change.

Rejected: a separate model per register-counter (590, 592, 593, 697, 8154).
They are a sync counter plus an input or output register, a 3-state bus or a
byte multiplexer; four optional keys of `sync` express all five, and the
counting core is shared with 28 other devices.

Risk: `sync`'s priority order and carry equations. §6 has one case per
function-table row of every sync device family (161/163, 169, 190/191,
192/193, 561, 569, 867/869, 590, 592, 593, 697, 8154, 4029, 4510/4516,
4518/4520, 40102/40103, 40160/40161, 40192/40193, 4522).

## 4. Shared additions

- `seq.c`: `seq_clock_name` / `seq_bind_clock` (`{"pin", "edge"}`),
  `seq_gate_name` (`{"pin", "op"}`, op `"and-not"`, `"or"`, `"or-not"`) and
  `LogicValue seq_gated_clock(clock, gate, op)` — the effective clock: and-not
  = CLK·¬G, or = CLK+G, or-not = CLK+¬G, with X propagation.
- Test support: `listing_kind_name` → `"counter"`, `print_details` and the
  `logic` condition include `PART_COUNTER`; `TRACE_LAST_KIND` → `PART_COUNTER`;
  `part_pin_open_drain` → 0 for every counter (none has open-drain outputs).

## 5. Detailed Design — the `counter` kind

`COUNTER_KIND`: `PART_COUNTER`, `"counter"`, model dispatch as `latch`
(B §3). Part keys `family vcc inputMax`. Main timing path: clock → Q (or
clock → the register's Q for devices whose only Q outputs are a register's).

### 5.1 Model `sync`

Device keys: `"units"` (1–2 unit objects). `Part.logic.units` = number of
units. Unit keys:

| Key | Value |
|---|---|
| `"stages"` | 1–32: count bits |
| `"code"` | `"binary"` (mod 2^stages) or `"bcd"` (stages a multiple of 4; each 4-bit digit counts 0–9; max = every digit 9) |
| `"q"` | optional: `stages` pin names, LSB first |
| `"clock"` | `{"pin", "edge"}`; or instead `"clockUp"` and `"clockDown"` (pin names): count up on `clockUp` rising while `clockDown` is 1, down on `clockDown` rising while `clockUp` is 1 |
| `"clockGate"` | optional `{"pin", "op"}` (§4): the counter steps on the edge of the gated clock |
| `"count"` | `"up"` or `"down"` — or `"direction"`: `{"pin", "up": "high"|"low"}`; or neither when `"modes"` or the dual clocks give it |
| `"enables"` | optional: controls, all asserted to count (synchronous: sampled at the edge) |
| `"enablesAny"` | optional: controls, counting needs at least one asserted (74_593: CCKEN or not CCKENBAR) |
| `"clears"` | optional: `{"pin", "active", "when": "async"|"clock", "to": "zero"|"max"}` |
| `"loads"` | optional: `{"pin", "active", "when", "from": "data"|"register"}` |
| `"data"` | load data pin names, LSB first (required when a load is `from` `"data"`) |
| `"modes"` | optional (74_867/869): `{"pins": [S0, S1], "table": [4 of "clear", "down", "load", "up", "hold"], "clear": "async"|"clock"}`, row = S0 + 2·S1. A `load` row loads `data` at the edge regardless of enables; `up`/`down` rows count when `enables` are asserted |
| `"illegal"` | optional, `bcd` only: `{"up": [16], "down": [16]}` next digit for each digit value 0–15 (the entries 0–9 must be the normal sequence) |
| `"carries"` | optional: `{"pin", "active", "at": "max"|"min"|"terminal", "gates": [controls], "clockLow": <pin name>}` — asserted while the count is at `max` / `min` (0) / the terminal of the current direction (max up, min down; never asserted in a non-counting `modes` row), all `gates` asserted, and (with `clockLow`) that pin is 0 |
| `"outputEnable"` | optional control gating `q` |
| `"register"` | optional output register: `{"clock": {"pin","edge"}, "outputs": [stages names], "outputEnable": control, "select": {"pin", "register": "high"|"low"}}` — loads the count on its clock edge; `outputs` show the register, or with `select` the register when the select pin is at `register` and the count otherwise |
| `"inputRegister"` | optional: `{"clock": {"pin","edge"}, "data": [names] or absent (then the `bus` pins), "enable": control}` |
| `"bus"` | optional (74_593): `{"pins": [stages names, inout], "outputEnables": [controls, all asserted]}` — the count drives the pins while enabled |
| `"zeroLatch"` | optional (4522B): `{"pin", "cascade": <pin name>}` — §5.1.2 |
| `"codeSelect"` | optional (4029B): `{"pin", "binary": "high"|"low"}` — the code (binary or bcd) is read from this pin at each edge; `code` is then absent and `stages` is 4 |

Device keys outside the unit: `"byteBus"` (74_8154 only):
`{"outputs": [8 names], "selects": [{"pin", "unit", "byte"}…]}` — the outputs
show byte `byte` (0 = low) of unit `unit`'s register while that select pin is
low; the first low select in list order wins; none low → Z; an X select → X.

#### 5.1.1 Order of evaluation

On any input change, per unit:

1. Asynchronous clears in list order (asserted → count = `to`), then
   asynchronous loads (asserted → count = data or register, level-
   transparent), then an asynchronous `modes` clear. Unknown → §3.5.
2. If none of those is asserted and the effective clock (gated, or the
   up/down rule) has its edge: clock clears in order, then clock loads, then
   the `modes` row, then counting when the enables are asserted (`enables`
   all, `enablesAny` at least one, both when given), else hold. Counting is
   ±1 in `code` (bcd: per digit with carry/borrow; a digit ≥ 10 steps by
   `illegal`, or without it to value+1 / value−1 within 0–15).
3. Register and input-register clocks act on their own edges, independent of
   1–2 (the input register's enable is sampled at its edge).
4. Outputs: `q`, `carries`, `register.outputs`, `bus`, `byteBus`,
   `zeroLatch` re-evaluated and driven (`seq_drive`).

#### 5.1.2 `zeroLatch` (4522B)

From `divider.md` § 4522B: the pin is a set/reset latch's output. Set while
count = 0 and `cascade` = 1; reset while (the preset-enable load is asserted
and the gated clock CK = CL·¬CI is 0) or (count ≠ 0 and the load is not
asserted). Single-stage use (pin tied to PE) then gives one pulse about one
clock-high time wide per N clocks — the case §6 checks.

#### 5.1.3 Devices

| Device | Keys (beyond `stages`/`code`/`q`/`data` from the spec section) |
|---|---|
| `74_161` | clock CLK ↑; enables ENP high, ENT high; count up; clears CLR low async; loads LOAD low clock; carry RCO high at max, gates ENT high |
| `74_163` | as 161 with CLR `when` clock |
| `74_169` | enables ENP low, ENT low; direction U/D up high; loads LOAD low clock; carry RCO low at terminal, gates ENT low |
| `74_190` (bcd), `74_191` | enables CTEN low; direction D/U up low; loads LOAD low async; carries MAX/MIN high at terminal; RCO low at terminal, gates CTEN low, clockLow CLK; 190 `illegal` from `counter-a.md` § 74_190 (up 10→9, 11→4, 12→9, 13→0, 14→9, 15→0; down 10→9, 11→10, 12→3, 13→4, 14→5, 15→6) |
| `74_192` (bcd), `74_193` | clockUp UP, clockDown DOWN; clears CLR high async; loads LOAD low async; carries CO low at max clockLow UP; BO low at min clockLow DOWN |
| `74_561` | enables ENP, ENT high; count up; clears ACLR low async, SCLR low clock; loads ALOAD low async, SLOAD low clock; outputEnable OE low; carries RCO high at max gates ENT; CCO high at max gates ENT, ENP clockLow CLK |
| `74_569` | enables ENP, ENT low; direction U/D up high; clears ACLR low async, SCLR low clock; loads LOAD low clock; outputEnable OE low; carries RCO low terminal gates ENT low; CCO low terminal gates ENT low, ENP low, clockLow CLK |
| `74_867` / `74_869` | stages 8; modes [S0, S1] table [clear, down, load, up], clear async (867) / clock (869); enables ENP low, ENT low; carry RCO low at terminal, gates ENT low |
| `74_867_AS` / `74_869_AS` | as 867 / 869 with RCO gates ENT low and ENP low |
| `74_590` | stages 8, no `q`; clock CCK ↑; enables CCKEN low; clears CCLR low async; register {RCK ↑, outputs QA–QH, outputEnable G low}; carry RCO low at max |
| `74_592` | stages 8, no `q`; clock CCK ↑; enables CCKEN low; clears CCLR low async; loads CLOAD low async from register; inputRegister {RCK ↑, data A–H}; carry RCO low at max (CCLR listed first, so it wins over CLOAD — the datasheet leaves the pair unspecified) |
| `74_593` | as 592 with enablesAny [CCKEN high, CCKENBAR low]; inputRegister {RCK ↑, enable RCKEN low, from the bus}; bus {pins A/QA–H/QH, outputEnables [G high, GBAR low]} |
| `74_697` | stages 4, no `q`; clock CCK ↑; enables ENP low, ENT low; direction U/D up high; clears CCLR low async; loads LOAD low clock; register {RCK ↑, outputs QA–QD, outputEnable G low, select {R/C, register high}}; carry RCO low at terminal gates ENT low |
| `74_8154` | 2 units of 16, no `q`: A clock CLKA ↑; B clock CLKB ↑, enables CLKBEN low; both clears CCLR low async; register {RCLK ↑} per unit (no outputs); carry RCOA low at max on unit A; byteBus {Y0–Y7, selects GAL (A, 0), GAU (A, 1), GBL (B, 0), GBU (B, 1)} |
| `4029B` | clock CLOCK ↑; enables CARRY IN low; direction UP/DOWN up high; loads PRESET ENABLE high async from data (JAM1–4); codeSelect {BINARY/DECADE, binary high}; carry CARRY OUT low at terminal gates CARRY IN low |
| `4510B` (bcd), `4516B` | as 4029B without codeSelect; clears RESET high async (before the load) |
| `4518B` (bcd), `4520B`, `74_4518` (bcd), `74_4520` | 2 units; clock CLOCK ↑ with clockGate {ENABLE, `or-not`}; count up; clears RESET (MR on 74_) high async |
| `40102B` (bcd, 8), `40103B` (8) | no `q`; clock CLOCK ↑; enables CI/CE low; count down; clears CLR low async to max; loads APE low async, SPE low clock (data J0–J7); carry CO/ZD low at min gates CI/CE low |
| `74_40103` | as 40103B with the 74_ names (MR, PL, PE, TE, TC, P0–P7) |
| `40160B` (bcd), `40161B` | clock CLOCK ↑; enables PE high, TE high; count up; clears CLEAR low async; loads LOAD low clock; carry COUT high at max gates TE high |
| `40192B` (bcd), `40193B` | clockUp CLOCK UP, clockDown CLOCK DOWN; clears RESET high async; loads PRESET ENABLE low async (JAM1–4); carries CARRY low at max clockLow CLOCK UP; BORROW low at min clockLow CLOCK DOWN |
| `4522B` (bcd, 4) | clock CLOCK ↑ with clockGate {INHIBIT, and-not}; count down; clears MASTER RESET high async; loads PRESET ENABLE high async (P0–P3); zeroLatch {"0", cascade CF} |


### 5.2 Model `ripple`

Device keys:

| Key | Value |
|---|---|
| `"units"` | number shown in the drawer (`PartLogic.units`) |
| `"sections"` | 1–4 objects: `{"clock", "structure", "stages", "q", "reset", "set"}` |

Section keys: `clock` (pin; every structure counts on its falling edge),
`structure` `"binary"` (`stages` 1–14, stage k toggles on stage k−1 falling),
`"div5"` (QB, QC, QD count 0–4 per `counter-b.md` § 74_390: QD clocked by the
input, QC by QB), `"div6"` (§ 74_92: QB, QC a ÷3 on the input, QD toggles on
QC falling), `"div2"` (one stage); `q` (pin names per stage, `null` for a
stage with no pin — 4020: Q2, Q3); `reset` `{"pins": [...], "active"}`
(asserted when every listed pin is at `active`); `set` `{"pins", "active",
"value"}` (74_90 R9: the ÷2 section's value 1, the ÷5 section's value 4).
Priority: set > reset > clock. Timing extra key `"stage"` (ns): an output's
delay is the entry's rise/fall plus `stage` × its ripple depth (binary: k for
stage k from 0; div5: QB 0, QC 1, QD 0; div6: QB 0, QC 0, QD 1; div2 0).
`"stage"` is required for ripple devices.

| Device | Sections |
|---|---|
| `74_90` | div2 (CKA → QA), div5 (CKB → QB, QC, QD); reset R0(1)·R0(2) high; set R9(1)·R9(2) high (values 1 / 4); units 1 |
| `74_92` | div2 (CKA → QA), div6 (CKB → QB, QC, QD); reset R0(1)·R0(2); units 1 |
| `74_93`, `74_293` | div2 (CKA → QA), binary 3 (CKB → QB, QC, QD); reset R0(1)·R0(2); units 1 |
| `74_390` | per half: div2 (nCLKA → nQA), div5 (nCLKB → nQB–nQD), reset nCLR high; units 2 |
| `74_393` | per half: binary 4, reset nCLR high; units 2 |
| `4020B`, `74_4020` | binary 14, Q1, Q4–Q14 (Q2, Q3 null); reset high; units 1 |
| `4024B`, `74_4024` | binary 7; units 1 |
| `4040B`, `74_4040` | binary 12; units 1 |

## 6. `counter_test`

Fixtures as B §6.2 (kind `counter`, one per device row of §5.1.3 and §5.2,
copied from the spec sections), bench from B §6.1. Cases:

| # | Fixture | Case |
|---|---|---|
| 1 | 74_161 | power-on 0; counts 0→15→0 on CLK ↑ with ENP = ENT = 1; RCO high only at 15 |
| 2 | 74_161 | CLR low clears at once (no clock); RCO falls with it |
| 3 | 74_161 | LOAD low loads at the edge regardless of ENP/ENT |
| 4 | 74_161 | ENP low holds, RCO still follows ENT; ENT low holds and RCO low |
| 5 | 74_163 | CLR low clears only at the edge; beats LOAD |
| 6 | 74_169 | down count 0 → 15; RCO low at 0 down, at 15 up, only with ENT low |
| 7 | 74_190 | 9 → 0 up, 0 → 9 down; loaded 12, counts up to 9 then 0 (the `illegal` table) |
| 8 | 74_191 | LOAD low is transparent (data changes follow); RCO low only during CLK low at max with CTEN low |
| 9 | 74_193 | UP ↑ counts with DOWN high; CO low while UP low at 15; BO low while DOWN low at 0; CLR high beats LOAD |
| 10 | 74_561 | ACLR > ALOAD > SCLR > SLOAD > count, one step each; OE high → Q = Z, RCO driven; CCO high only during CLK low |
| 11 | 74_569 | SCLR at the edge; CCO low pulse at terminal with both enables low |
| 12 | 74_867 | modes: 00 clears at once; 869: at the edge; 10 loads; 01/11 count; RCO only in count modes; AS fixture: RCO needs ENP low too |
| 13 | 74_590 | count and register separate; RCK ↑ copies; tied clocks → register one behind; G high → Z; CCLR clears the counter only |
| 14 | 74_592 | CLOAD low copies the input register continuously; RCK ↑ during CLOAD low passes through; CCLR wins over CLOAD |
| 15 | 74_593 | bus driven only with G high and GBAR low; register loads from the pins with RCKEN low; count with CCKEN high or CCKENBAR low |
| 16 | 74_697 | R/C high shows the register, low the counter; LOAD at the edge ignores the enables |
| 17 | 74_8154 | GAL low shows A low byte; GBU upper B; RCOA low at FFFF; RCOA → CLKBEN cascade gives a 32-bit count; CCLR clears both counters, not the register |
| 18 | 4029B | binary/decade by the pin; up/down; PRESET ENABLE async; CARRY OUT low at terminal with CARRY IN low |
| 19 | 4510B | RESET beats PRESET ENABLE |
| 20 | 4518B | counts on CLOCK ↑ with ENABLE high and on ENABLE ↓ with CLOCK low; not on CLOCK ↑ with ENABLE low |
| 21 | 40103B | CLR → 255; APE async load; SPE at the edge; CO/ZD low at 0 with CI/CE low; wraps 0 → 255 |
| 22 | 40102B | CLR → 99; BCD down count 10 → 09 |
| 23 | 40193B | as 74_193 with active-low PRESET ENABLE |
| 24 | 4522B | N = 5 with "0" tied to PE: one "0" pulse per 5 clocks, each about one clock-high time |
| 25 | 74_393 | falling-edge ripple; Q3 changes 3 × `stage` after Q0; CLR high async |
| 26 | 74_90 | BCD with QA → CKB (external wire in the bench); R9 sets 9; R9 beats R0 |
| 27 | 74_92 | ÷12 sequence of `counter-b.md` § 74_92 |
| 28 | 4020B | Q2/Q3 have no pins: Q4 toggles every 8 clocks |
| 29 | loader | `stages` 0 → error; `code` bcd with stages 6 → error; `illegal` on a binary unit → error; `codeSelect` with `code` absent loads |

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — Kind and `sync`

- [x] `parts.h` `PART_COUNTER`; `models/counter.{c,h}` kind + dispatch + `sync`
  (`counter.c` listed in `backend/CMakeLists.txt`); `kinds.c`; `seq.c` additions (§4); test support (§4); docs § `counter`
  (`sync`). (`seq.c` also gains `seq_control_object` and `seq_clock_edge`;
  `cc -Wall -Wextra -fsyntax-only` on `counter.c` and `seq.c` prints
  nothing.)

Acceptance: `ctest` 13 of 13; golden diff empty (seconds). (Ran: `100% tests
passed, 0 tests failed out of 13`; golden status empty.)
Commit: —

### Phase 2 — `ripple`

- [x] The `ripple` model; docs.

Acceptance: as Phase 1. (Same run as Phase 1: both models were written in
one pass of `counter.c` and land in one commit; see Corrections.)
Commit: —

### Phase 3 — `counter_test`

- [ ] `backend/tests/counter_test.c`, the 29 cases; `logic_test(counter_test)`.

Acceptance: `counter_test` → 29 `PASS`, 0 `FAIL`; `ctest` 14 of 14 (seconds).
Commit: —

## Validation Plan

Phase 3's acceptance (about 6 s for all 14 tests). Landing tier: plan-10-J.

## Decisions

- Register counters are `sync` options, not models (§3).
- `4029B`'s binary/decade pin is a unit key (`codeSelect`), not two devices:
  the pin is live and selects at each edge.
- Decade counters without a printed illegal-state diagram count on in 4-bit
  binary from 10–15 (10 → 11 … 15 → 0 up; the reverse down) — the model's
  default, recorded in docs.
- CCLR beats CLOAD on the 74_592/593 (unspecified by the datasheet; clear-
  wins is the safe choice).

## Corrections

- 2026-11-01, Phases 1–2: `sync` and `ripple` were written together in
  `counter.c` and land in one commit (both phases' acceptance is the same
  `ctest` 13 of 13, no golden change, measured once).
- 2026-11-01, Phase 1: §5.1.1 puts the register clocks in step 3, after the
  count update; case 13 ("tied clocks → register one behind") needs the
  register to take the count from before the counter's edge. The register
  and input register therefore act first in each evaluation, the output
  register from the pre-edge count (docs § `counter` says so).
- 2026-11-01, Phase 1: §5.1.2 does not say which of set and reset wins when
  both hold (count 0, cascade 1, load asserted, gated clock 0): reset wins,
  so the single-stage loop releases PE on the clock's low half.
- 2026-11-01, Phase 1: a BCD digit carries only from 9 (up) / 0 (down); an
  illegal digit (10–15) steps without a carry (§5.1 said "per digit with
  carry/borrow" without the illegal case). `counter.h` has no
  `COUNTER_DIV_MODELS` list: E appends its models to `MODELS[]` in
  `counter.c`.
- 2026-11-01, Phase 2: the ÷5 section's illegal states (unprinted) follow
  the JK structure of `counter-b.md` § 74_390: 5 → 2, 6 → 2, 7 → 0; the ÷3
  of the 74_92 takes 11 → 00 (§ 74_92, derived).

## Summary

D adds the `counter` kind with `sync` (33 devices) and `ripple` (12) and 29
function-table cases on fixtures.
