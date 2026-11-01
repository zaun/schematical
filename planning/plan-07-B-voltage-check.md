# plan-07-B: The voltage check and its circuit integration

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-07-A

A static check over a netlist works out the voltage each net carries and
reports every pin whose voltage is out of range. It never touches the
logic simulation. `circuit_start` refuses a circuit that fails the check. A
switch that closes during a run and makes the circuit fail stops the run,
the same way a short does. The app (plan-07-G) calls the same function to
refuse edits.

Outcome: these circuit tests pass, on fixture parts with AUC-like
(0.8–2.7 V) and HC-like (2–6 V) ranges:

- 5V on the 0.8–2.7 V part's VCC is refused at start with
  `voltage: 5 V on pin 14 (VCC) of …; it needs 0.8-2.7 V`.
- A 3V3 net and a 5V net joined by a closing switch stop the run.
- 3V3 on the 2–6 V part runs.

References:

- plan-07-A §4 (the keys and structs this check reads)
- `docs/backend.md` (the circuit API, tests)
- `backend/src/circuit.c`, `backend/src/engine/sim.c:991-999` (smoke),
  `backend/src/models/switch.c`, `led.c`, `gate.c`

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-A complete | `ls planning/plan-07-A-* 2>/dev/null` → no output (archived to `planning/completed/`) | MET (2026-11-01: `ls planning/plan-07-A-*` → no matches; archived in 577a290) |

## 1. Goal

- `voltage_check()` (new public `backend/include/voltage.h`) returns every
  problem in a netlist (parts, pin→net connections, closed switch contacts).
  `voltage_describe()` formats a problem as text.
- `circuit_start` returns −1 when any problem is found:
  - `circuit_error` is `voltage: …`;
  - `circuit_voltage_problem` returns the first problem.
- `circuit_set_switch` closing a contact during a run, when the closed
  contact causes a problem, sets the same error. After that,
  `circuit_advance` returns 0.

### Non-goals (explicit constraints)

- No change to logic simulation: `traces.txt` byte-identical, every existing
  circuit/engine test result unchanged except `check_short` (below).
- A supply and GND on one net stays a **short** (`smoke:` at the first
  advance), not a voltage problem — existing behavior kept.
- Unknown voltage is never a problem: a pin on a net with no supply, no
  powered output and no carried voltage is not checked; a part with
  `vcc == NULL` (data not filled yet) is neither checked nor a source.
- 7-segment VF is not checked (shown only, plan-07-G).
- No app change.

## 2. Current State

- `circuit_connect` (`circuit.c:173-203`) resolves the pin designator to an
  index `i` and calls `sim_connect`; the circuit keeps **no** connection
  record of its own.
- `circuit_set_switch` (`circuit.c:205-221`) calls the model's `set_switch`;
  after start the contact is queued (`sim.c:671-678`) and applied in
  `process_round`. The circuit keeps no switch state.
- `circuit_start` (`circuit.c:223-233`) sets `started` and calls `sim_start`.
- `circuit_advance` returns 0 when `sim_error` is set (`circuit.c:242-245`);
  `set_error` stores a `const char *` (`sim.c:106-110`). `Circuit` has
  `char error[256]` (`circuit.c:12-27`).
- Switch contact pins are bound per pinout as `SwitchPins { positions;
  size_t (*pins)[2]; }` (`switch.c:21-24`) behind `part_internal(part)->bound`.
- LED anode/cathode are `LedPins { anode; cathode; }` (`led.c:20-23`) behind
  the same pointer.
- Gate open-drain is `GateDevice.open_drain` (`gate.c:57,189`), private.
- Pin roles: `PIN_INPUT, PIN_OUTPUT, PIN_INOUT, PIN_POWER, PIN_GROUND,
  PIN_NC, PIN_ABSENT` (`parts.h:30-32`).
- `check_short` (`tests/circuit_check.c:85-101`) wires VCC and GND to one net
  and expects `smoke:` — still true under this design (supply vs GND is a
  short, not a voltage problem).

### Measured populations

| What | Count | Command |
|---|---|---|
| devices with >1 power-pin name | 0 | census (plan-07-A §2) → `multi-power-name devices: 0` |
| open-drain gate devices / parts | 1 / 15 (`74_266`) | census → `open-drain gate devices: 1 parts: 15` |
| inout pin entries by kind | eeprom 216, oscillator 8, sram 448, switch 316 | census → `{'eeprom': 216, 'oscillator': 8, 'sram': 448, 'switch': 316}` |
| circuit_check uses of `"VCC"` | 8 | `grep -c '"VCC"' backend/tests/circuit_check.c` → 8 |

### Verified properties

- One power-pin name per device (above): "the part's supply voltage" is the
  voltage of the net(s) on its `PIN_POWER` pins; with several power pins
  (one name, several pins) every one is checked and the highest known
  voltage is the part's supply.
- Building calls work only before start (`check_building`), so the
  connection list is final at `circuit_start`.

## 3. Design Overview

Three pieces: (1) `voltage.c`, a pure function over arrays — no `Circuit`,
no `Sim`, so the app can call it on a candidate netlist; (2) small internal
pin accessors for LED, switch and open-drain gate; (3) circuit.c records
connections and switch states and calls the check at start and on switch
close.

**Correctness risk** is the propagation (§4.2): which nets get a voltage
and from what. Every rule has its own `voltage_test` case. Placement: the
check sits in `circuit.c`, outside the engine, so the engine (and traces)
cannot change.

**Gate class:** not byte-identity. `traces.txt` and `parts.txt` must stay
byte-identical (no part or behavior change); a diff is a bug.

**Rejected alternatives**

- *Voltage as a fifth logic value in the engine* — the owner wants a
  logic-only sim; it would touch every model.
- *Check inside `process_round`* — the engine knows no parts or voltages.
- *Report only the first problem* — the app wants to name the pin; returning
  all lets G show the first and B test each.

## 4. Detailed Design

### 4.1 API (`backend/include/voltage.h`, new)

```c
typedef enum {
    VOLTAGE_SUPPLY_CLASH,   /* two supplies of different non-zero voltage on one net */
    VOLTAGE_POWER_RANGE,    /* power pin outside the part's vcc range */
    VOLTAGE_INPUT_OVER,     /* input/inout pin above the part's input max */
    VOLTAGE_SWITCH_OVER     /* switch contact pin above its rating */
} VoltageProblemKind;

typedef struct { size_t part; size_t pin; size_t net; } VoltageConnection;  /* pin: index into Part.pins */
typedef struct { size_t part; unsigned position; } VoltageContact;          /* a closed switch position, 1-based */

typedef struct {
    VoltageProblemKind kind;
    size_t net;              /* the circuit net index */
    size_t part, pin;        /* the offending pin; for CLASH: one supply */
    size_t other_part;       /* CLASH: the other supply; else = part */
    uint32_t mv;             /* the net's voltage (CLASH: the higher) */
    uint32_t min_mv, max_mv; /* the allowed range (CLASH: the lower voltage in both) */
} VoltageProblem;

/* Returns the number of problems (all of them); writes up to cap into out,
   ordered by net, then part, then pin. */
size_t voltage_check(const Part *const *parts, size_t part_count, size_t net_count,
                     const VoltageConnection *connections, size_t connection_count,
                     const VoltageContact *closed, size_t closed_count,
                     VoltageProblem *out, size_t cap);

/* "voltage: …" text (examples §4.3). Returns the length written (snprintf rules). */
int voltage_describe(const Part *const *parts, const VoltageProblem *problem,
                     char *buffer, size_t size);
```

### 4.2 Algorithm (`backend/src/voltage.c`, new)

1. **Groups.** Union-find over nets; each closed contact joins the nets of
   its two contact pins (pins without a connection join nothing).
2. **Supply voltage** per group: the supply parts (`PART_SUPPLY`) on it.
   Two different non-zero `mv` → one `VOLTAGE_SUPPLY_CLASH` (per group, the
   two lowest-indexed disagreeing supplies). A supply with `mv` 0 next to a
   non-zero one is **not** a problem (it is a short, §1 Non-goals). Group
   voltage `S(g)` = the highest supply `mv` on it (0 if only GND), else
   unknown.
3. **Fixed point** over the rest, repeated until nothing changes (each
   value only rises; at most part_count + 1 rounds):
   - a part with `vcc != NULL` is *powered at V* when the highest known
     voltage among its `PIN_POWER` pins' groups is V > 0;
   - **driven** `D(g)` = max V over powered parts' `PIN_OUTPUT`/`PIN_INOUT`
     pins in g, skipping open-drain gate outputs (`74_266`);
   - **carried** `C(g)` = for each generic LED (`PART_LED`): its cathode's
     group gets the anode group's effective voltage (§ step 4) — anode to
     cathode only;
   - effective voltage `E(g)` = `S(g)` if known, else `D(g)` if any, else
     `C(g)` if any, else unknown. (So a carried voltage on a GND net is
     ignored: GND makes `S` = 0.)
4. **Checks**, per connection with `E(g)` known:
   - `PIN_POWER` of a part with `vcc`: `E` outside `[min_mv, max_mv]` →
     `VOLTAGE_POWER_RANGE` (0 V, i.e. a VCC pin on GND, is below min);
   - `PIN_INPUT`/`PIN_INOUT` of a part with `vcc`: `E > input_max_mv` →
     `VOLTAGE_INPUT_OVER` (`min_mv` = 0, `max_mv` = input max);
   - `PIN_INOUT` of a switch with `rated_mv > 0`: `E > rated_mv` →
     `VOLTAGE_SWITCH_OVER`;
   - nothing else (ground pins, supplies, LEDs, 7-seg, nc).

Internal accessors (`backend/src/models/model_pins.h`, new, implemented in
the model files): `int led_pins(const Part *, size_t *anode, size_t *cathode)`,
`int switch_contact_pins(const Part *, unsigned position, size_t *a, size_t *b)`,
`int gate_open_drain(const Part *)` — each returns 0 for another kind.

### 4.3 Messages

Volts print as `%g` of `mv / 1000.0`; pins as `pin <designator> (<name>)`.

- `voltage: 3.3 V and 5 V supplies on one net (3V3, 5V)`
- `voltage: 5 V on pin 14 (VCC) of SN74AUC00PWR; it needs 0.8-2.7 V`
- `voltage: 5 V on pin 1 (1A) of SN74AUC00PWR; its inputs take at most 3.6 V`
- `voltage: 12 V on pin 1 (1A) of TE-GDS0204; it is rated 5 V`

### 4.4 Circuit integration (`backend/src/circuit.c`, `include/circuit.h`)

- `Circuit` gains a growable `VoltageConnection` array (appended by
  `circuit_connect` after `sim_connect` succeeds), a closed-contact list
  (updated by `circuit_set_switch` before and after start: add on `on`,
  remove on off), `int voltage_stopped` and one `VoltageProblem problem`.
- `circuit_start`: after `check_building`, run `voltage_check`; on ≥ 1
  problem: `voltage_describe` into `error`, store the first problem, leave
  `started` 0 and return −1 (building calls stay usable so the caller can
  destroy; no `sim_start`).
- `circuit_set_switch` after start with `on`: check with the new contact;
  on a problem set `error`, `problem`, `voltage_stopped = 1` (the contact is
  still applied, like a short). `circuit_advance` returns 0 when
  `voltage_stopped`; `circuit_error` returns the voltage text.
- New: `int circuit_voltage_problem(const Circuit *, VoltageProblem *out)` →
  1 and the problem when start or a switch failed the check, else 0.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; `- [~]` partial; moot = `- [x] ~~text~~ — moot: <evidence>`;
> fill `Commit:`. **An unticked box means NOT DONE.**

### Phase 1 — `voltage_check` and its unit test

Pure function, no callers: safe alone.

- [x] `model_pins.h` + the three accessors in `led.c`, `switch.c`, `gate.c`;
      `PartLogic` untouched.
- [x] `include/voltage.h`, `src/voltage.c` (§4.1–4.3); `CMakeLists.txt` adds
      `src/voltage.c` to the library and `tests/voltage_test.c` as a ctest.
- [x] `tests/voltage_test.c`, inline fixture library (plan-07-A keys; one
      gate device with `vcc [2,6]`, one with `vcc [0.8,2.7], inputMax 3.6`,
      an open-drain gate, a switch rated 5, an LED, supplies 5V/3V3/GND).
      One case per rule: in-range power OK; 5 V power on 0.8–2.7 → POWER_RANGE
      with the §4.3 text; VCC pin on GND → POWER_RANGE 0 V; 5 V input on
      inputMax 3.6 → INPUT_OVER; 5 V into a 3.6 inputMax via a powered 5 V
      gate's output → INPUT_OVER; same via an unpowered gate → none;
      open-drain output → none; 3V3 + 5V on one net → CLASH; 5V + GND → none;
      closed contact joining 5V to the 0.8–2.7 power net → POWER_RANGE, open →
      none; 5V on a contact of the switch rated 5 → none, 12V on it →
      SWITCH_OVER; LED anode on 5V, cathode net
      with only an AUC input → INPUT_OVER; same with cathode also on GND →
      none; part with `vcc == NULL` → none; result ordering by net/part/pin.

Acceptance: every case passes.
  Check: `cmake --build /tmp/p07 -j8 && /tmp/p07/voltage_test` → exit 0 (est. 1 min).
  Result (2026-11-01): `voltage_test` → 24 × `PASS`, `voltage_test: all
  passed`, exit 0 (e.g. "PASS: 5V on the 0.8-2.7 V part's VCC is
  POWER_RANGE" with text `voltage: 5 V on pin 5 (VCC) of VFX-AUC1G00; it
  needs 0.8-2.7 V`).
Commit: 5e0149b

### Phase 2 — Circuit integration

- [x] `circuit.c` / `circuit.h`: §4.4.
- [x] `circuit_check.c`: load the fixture devices of `voltage_test` (moved
      to `tests/support/voltage_fixture.h` as one inline JSON string) with
      `load_parts` into the test library next to the real files, so B does
      not depend on part data from C–F. New `check_voltage_start` (5V on the
      0.8–2.7 fixture's VCC → `circuit_start` −1, `circuit_error` = the §4.3
      text, `circuit_voltage_problem` = POWER_RANGE; 3V3 on the 2–6 fixture
      → starts and advances); `check_voltage_switch`
      (running circuit, `circuit_set_switch` closes a contact joining 3V3 to
      a 5V net → `circuit_advance` returns 0, error starts `voltage:`,
      `circuit_voltage_problem` = CLASH); `check_short` unchanged and still
      passing.
- [x] `docs/backend.md`: § Using it notes (`voltage.h`, the start refusal,
      the switch stop, `circuit_voltage_problem`); § Layout (`voltage.c`);
      § Tests (`voltage_test`).

Acceptance: the new circuit cases pass; every other test unchanged.
  Check: `cmake --build /tmp/p07 -j8 && ctest --test-dir /tmp/p07` → 12/12
  pass, and `git diff --quiet backend/tests/golden` → exit 0 (est. 1 min).
  Result (2026-11-01): `ctest --test-dir /tmp/p07` → `100% tests passed, 0
  tests failed out of 12`; `git diff --quiet backend/tests/golden` → exit
  0. `circuit_check` → `0 failure(s)`, including "PASS: 5V on a 0.8-2.7 V
  part's VCC is refused at start", "PASS: the start refusal names the pin
  and the range", "PASS: closing 3V3 onto 5V stops the run", "PASS: the
  stop is a voltage error", "PASS: circuit_voltage_problem returns the
  CLASH", and `check_short`'s three PASS lines unchanged.
Commit: —

## Validation Plan

- Tests: `voltage_test` (every §4.2 rule, positive and negative),
  `circuit_check` (start refusal, running switch stop, short unchanged).
- Coverage check: `voltage_test` builds `src/voltage.c` into its binary
  (CMake target sources); the open-drain and LED accessors are exercised by
  named cases.
- Runtime proof: `check_voltage_switch` runs a real circuit through
  `circuit_advance`.
- Doc sync: `docs/backend.md`.
- Final gate (once): `ctest --test-dir /tmp/p07` → 12/12; goldens unchanged
  (est. 1 min). The app is not built: no Swift file changes, and the
  bridging header gains nothing until G.

## Decisions

Owner decisions: plan-07-A § Decisions. Author decisions here:

- Supply vs GND on one net stays a short (`smoke:`), not a voltage problem —
  it is existing behavior and the smoke display already shows it.
- Unknown voltage and parts without data are never problems, so sub-plans
  C–F can land in any state of data completeness.
- Outputs and inouts of a powered part are sources regardless of direction
  (an SRAM's DQ pins count even when not reading) — the check is static.
- `voltage_check` takes arrays, not a `Circuit`, so the app reuses it on a
  candidate netlist without building a circuit.

## Corrections

- **The fixture lives in `tests/support/voltage_fixture.h` from Phase 1**,
  not moved there in Phase 2, and it is **two** inline part files, not one
  JSON string: a part file holds one kind, and the fixture needs a gate file
  (HC-like `VFX-HC1G00`, AUC-like `VFX-AUC1G00`, open-drain `VFX-OD1G00`)
  and a switch file (`VFX-SW2`, rated 5 V). The LED and the supplies are the
  real `LED-RED-0603`, `5V`, `3V3`, `1V8`, `12V`, `GND` (plan-07-A added the
  supplies), loaded with `load_parts_dir`, so the fixture uses real
  footprint ids (`SOT23-5`, `SW-DIP-4`).
- **The `vcc == NULL` case copies a loaded `Part` and clears `vcc`** instead
  of a fixture device with no `vcc`: plan-07-F makes `vcc` required, which
  would make such a fixture device fail to load. Two cases: not checked,
  and not a source.
- **Added cases** beyond the list: 3V3 on a 3.6 V-tolerant input is fine;
  the CLASH text names the lower voltage first whatever the part order; two
  equal supplies agree; a closed contact carries 12 V to its other rated
  pin; an LED carries anode → cathode only; `cap` smaller than the count.

## Summary

The risk is the propagation rules (§4.2); each has a test. The engine,
models' behavior and goldens are untouched.
