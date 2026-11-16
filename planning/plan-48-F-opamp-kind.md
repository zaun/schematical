# plan-48-F: Backend — the `opamp` kind (facts, a passive model, analog pins)

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-48-E

F adds the `opamp` kind: the loader and facts of an operational amplifier
with one, two or four amplifiers. An op-amp is a part of the circuit that
drives nothing: it can be placed next to simulated parts without turning Run
off, the voltage check tests its supply pin against its supply span, and its
input and output pins are analog pins the voltage check skips.

References:

- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Kinds.
- `backend/src/models/capacitor.c` (the passive, simulated shape F copies),
  `backend/src/models/converter*.c` (`pin_analog`), `generic.c` (the
  `"simulated": false` shape F does **not** use).
- plan-48-E (the kind steps, done once there for the shared lines).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-E complete | `ls planning/completed/plan-48-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-16: not started) |
| The kind is absent | `grep -c 'PART_OPAMP' backend/include/parts.h` → 0; `ls backend/src/models/opamp.c` → no such file | MET (2026-11-16) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `opamp.json` files load: kind `PART_OPAMP`, `Part.opamp` facts,
  `simulated` 1, the rejections of § 5.
- In a circuit an op-amp drives no net; a circuit with an op-amp and
  simulated parts starts and runs; a supply above the op-amp's span on its
  `V+` pin is a `VOLTAGE_POWER_RANGE` problem; its amplifier pins are never
  a voltage problem.
- `ctest` passes T0 + 2 programs (`timer_test`, `opamp_test`); goldens
  unchanged.

### Non-goals (explicit constraints)

- No analog behavior of any kind: no comparator action, no follower. The
  output pins stay undriven (Z).
- No negative supply: the app has none, so `V-` is wired to ground and the
  `vcc` range is the single-supply span `V+` − `V-`.
- No change to the voltage check's rules or to `circuit.c`.
- No part file, no app change (G, H).

## 2. Current State

- `circuit_add_part` refuses a part whose device says `"simulated": false`
  (`backend/src/circuit.c:138`), and the app turns Run off while such a part
  is placed (`Simulation/SimulationAvailability.swift:11`, plan-36). A
  passive kind that stays simulated (`capacitor.c`: no `pin_changed`, no
  `timer`) has neither effect.
- `PartKindOps.pin_analog` (`backend/src/library/kind.h`) makes the voltage
  check skip a pin (`voltage.c:454`) and keeps an analog power pin from
  being the part's supply (`voltage.c:411`).
- `PartKind` ends with `PART_TIMER` after plan-48-E.

### Verified properties

- **A pin with role `output` that is never driven is legal**: the engine
  has no "undriven output" error; `net_driver` (`circuit.c:729`) only names
  a serial RX pin's driver for `circuit_serial_send`.
- **The power-range check uses `Part.vcc`** on `PIN_POWER` pins that are
  not analog (`voltage.c:411`), so `V+` with role `power` and `V-` with
  role `ground` gives the span check with no new code.

## 3. The kind

### 3.1 Part file format (`"kind": "opamp"`)

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"general"`, `"jfet"` or `"cmos"` (DigiKey's Standard (General Purpose), J-FET, CMOS) |
| `"amplifiers"` | 1, 2 or 4 entries, each `{"plus", "minus", "output"}` (pin names) |
| `"gbwHz"` | gain bandwidth product, whole hertz, 0…100000000000; optional (0 = not given) |
| `"slewVus"` | slew rate in V/µs, a number from 0 to 100000; optional |
| `"vosMv"` | maximum input offset voltage in mV, a number from 0 to 1000; optional |
| `"railIn"`, `"railOut"` | booleans, default false: rail-to-rail input / output |
| `"vcc"`, `"inputMax"` | the supply span `V+` − `V-` in volts, as every kind |

Part keys: `"vcc"`, `"inputMax"`, `"vosMv"` (a grade's own offset).

Pin rules (`bind_pinout`): each `plus` and `minus` pin exists once and has
role `input`; each `output` pin exists once and has role `output`; no pin
is named twice; at least one `power` pin and one `ground` pin; every other
pin has role `input` (shutdown, enable, offset null, reference), `ground`
(a numbered exposed pad) or `nc`.

```c
/* An operational amplifier (plan-48-F): facts only. type "general", "jfet"
   or "cmos"; amplifier_count 1, 2 or 4; gbw_hz, slew_mv_us (millivolts per
   microsecond) and vos_uv (max input offset, microvolts): 0 = not given. */
typedef struct { const char *type; size_t amplifier_count; uint8_t rail_in, rail_out;
                 uint64_t gbw_hz; uint32_t slew_mv_us, vos_uv; } PartOpamp;
```

`const PartOpamp *opamp;` on `Part`. `pin_analog` answers 1 for every pin an
`amplifiers` entry names.

### 3.2 The model (`OPAMP_MODEL`)

The passive shape of `capacitor.c`: `.name`, `.pin_count`, `.create`
(returns the `Part`), no `start`, `pin_changed` or `timer`.

## 4. Docs

- `docs/parts-format.md`: a `### opamp` section (keys, pin rules, the
  single-supply note, an example dual device).
- `docs/backend.md` § Layout: `opamp.c`; § Adding a kind: one sentence
  after the `"simulated": false` paragraph — a kind with facts only that
  must not block a run stays simulated with a passive model (`opamp.c`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind, the tests

- [ ] `backend/include/parts.h`: `PART_OPAMP` after the current last
      `PartKind`, `PartOpamp`, `const PartOpamp *opamp;` on `Part`.
- [ ] `backend/src/models/opamp.c`: `OPAMP_KIND` and `OPAMP_MODEL` per § 3.
- [ ] `backend/src/library/kinds.c`: the `extern` and `&OPAMP_KIND` at the
      end of `KINDS[]` (`grep -c 'OPAMP_KIND' backend/src/library/kinds.c` →
      0 before, 2 after); `backend/CMakeLists.txt`: `src/models/opamp.c`
      and `logic_test(opamp_test)`.
- [ ] `backend/tests/support/listing.c`: `case PART_OPAMP: return "opamp"`
      and the facts line `type=%s amps=%zu gbw=%llu slew=%u vos=%u rin=%u
      rout=%u`; `TRACE_LAST_KIND` and `listing_check.c`'s two array sizes →
      `PART_OPAMP` (moot when a later kind already sizes them).
- [ ] `backend/tests/support/opamp_fixture.h` and `backend/tests/opamp_test.c`:
      the cases of § 5.

Acceptance: the kind loads, stays passive, and the voltage check treats it
as § 1 says.
  Check: `cmake -S backend -B /tmp/logic-p48-f && cmake --build /tmp/logic-p48-f -j8 && /tmp/logic-p48-f/opamp_test` → every line `PASS`; `ctest --test-dir /tmp/logic-p48-f -j8` → `100% tests passed`, T0 + 2 programs (est. 4 min: a build from scratch).
Commit: —

### Phase 2 — Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name the kind.
  Check: `grep -n '^### `opamp`' docs/parts-format.md` → 1 line; `grep -c 'opamp.c' docs/backend.md` → above 0 (est. < 1 min).
Commit: —

## 5. `opamp_test` cases

Fixture (`opamp_fixture.h`): a `general` device with the `dual-8` pinout of
plan-48-B § 3.1 (`p8`), `"gbwHz": 1000000`, `"slewVus": 0.5`, `"vosMv": 7`,
`"vcc": [3, 32]`, part `TESTOPAMP2` (footprint `PDIP-8`) and part
`TESTOPAMP2A` with `"vosMv": 3`; a `cmos` device with the `single-5a`
pinout, `"railIn": true`, `"railOut": true`, `"vcc": [1.8, 5.5]`, part
`TESTOPAMP1` (footprint `SOT23-5`).

- (a) `TESTOPAMP2`: kind `PART_OPAMP`, `simulated` 1, 8 pins, `type`
  "general", `amplifier_count` 2, `gbw_hz` 1000000, `slew_mv_us` 500,
  `vos_uv` 7000, `rail_in` 0, `rail_out` 0, `vcc` 3000–32000 mV;
  `TESTOPAMP2A`: `vos_uv` 3000.
- (b) `TESTOPAMP1`: `type` "cmos", `amplifier_count` 1, `rail_in` 1,
  `rail_out` 1, `gbw_hz` 0.
- (c) `part_pin_analog` is 1 for pins 1, 2, 3, 5, 6, 7 of `TESTOPAMP2` and 0
  for pins 4 and 8.
- (d) `"type": "bjt"` → `unknown op-amp type "bjt"`; three `amplifiers` →
  `"amplifiers" must have 1, 2 or 4 entries`; an entry without `"output"` →
  `missing "output"`.
- (e) a `plus` pin with role `output` → `op-amp pin "IN1+" must be an
  input`; an `output` pin with role `input` → `op-amp pin "OUT1" must be an
  output`; a pin named by two entries → `op-amp pin "OUT1" is named twice`;
  a pinout with no `ground` pin → `an op-amp needs a power pin and a ground
  pin`.
- (f) `"slewVus": -1` → `"slewVus" must be from 0 to 100000`; `"vosMv":
  2000` → `"vosMv" must be from 0 to 1000`; a part key the kind does not
  list (`"gbwHz": 5` on a part) → `unknown key "gbwHz"`.
- (g) a circuit of `TESTOPAMP1` (`V+` on `5V`, `V-` on `GND`, `IN+` on a
  `5V` net, `IN-` on `GND`, `OUT` on a net of its own): `circuit_add_part`
  and `circuit_start` return 0; after 1 ms the OUT net reads `CIRCUIT_Z`
  and `circuit_net_changes` is 0.
- (h) the same with `V+` on `12V`: `circuit_start` returns -1 and
  `circuit_voltage_problem` gives `VOLTAGE_POWER_RANGE` on that pin.
- (i) `TESTOPAMP1` with `IN+` on `12V` and `V+` on `5V`: `circuit_start`
  returns 0 (an analog pin is not checked).
- (j) (g) plus an `SN74HC04N` with an LED on its output: both start; the
  inverter still works (its output net follows its input).

## Validation Plan

- Tests: `opamp_test`; `listing_check`, `trace_check` unchanged goldens.
- Coverage check: `ctest --test-dir /tmp/logic-p48-f -N | grep -c opamp_test` → 1.
- Runtime proof: case (j): a run with an op-amp in the circuit.
- Doc sync: § 4.
- Final gate: plan-48-I.

## Decisions

- Owner, 2026-11-16: an op-amp is facts only in play mode.
- Author: **facts only is a passive simulated model, not `"simulated":
  false`**: with `false`, one placed op-amp would turn Run off for the
  whole schematic, 555 included. The op-amp is in the circuit and drives
  nothing.
- Author: **amplifier pins are analog pins**: the voltage check skips them,
  since an input above the rail or an output level has no meaning here.
- Author: **`vcc` is the single-supply span**; dual supplies cannot be
  drawn in the app.

## Corrections

## Summary

A small kind: a loader, facts and a model with no callbacks. The one choice
with consequences is staying simulated, so Run stays available.
