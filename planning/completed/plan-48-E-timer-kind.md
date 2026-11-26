# plan-48-E: Backend — the `timer` kind, the 555 model, timing from the wired resistors and capacitors

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-D (letter order; E's code needs only § Prerequisites)

E adds the `timer` kind: the loader and facts of a 555-type timer with one
or two timer sections, the engine model, and the analysis that reads the
resistors, capacitors, diodes and pull-ups wired to a timer when a circuit
starts and sets its mode and times. Two small circuit calls come with it:
`circuit_set_part_value` (a placed resistor's or capacitor's value) and
`circuit_timer_state` (what a timer is doing and why).

References:

- `docs/backend.md` § Adding a kind, § Tests; `docs/parts-format.md` § Kinds
  (the section E adds).
- `backend/src/models/oscillator.c` (a model driven by engine timers),
  `capacitor.c` (the passive shape), `resistor.c` and `model_pins.h`
  (`resistor_pair`, `diode_pair`), `backend/src/circuit.c`
  (`circuit_start`, `Circuit.connections`), `backend/src/voltage.c`.
- `planning/plan-44-E-crystal-kind.md` (the shape of a kind sub-plan).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-D complete | `ls planning/completed/plan-48-D-*.md` → exists, or every D box ticked | MET (2026-11-24: every D box ticked) |
| The kind is absent | `grep -c 'PART_TIMER' backend/include/parts.h` → 0; `ls backend/src/models/timer.c backend/src/timer_rc.c` → no such file | MET (2026-11-16; again 2026-11-24 before this letter: 0, no file) |
| The two circuit calls are absent | `grep -c 'circuit_set_part_value\|circuit_timer_state' backend/include/circuit.h` → 0 | MET (2026-11-16: 0; 2026-11-24: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `timer.json` files load: kind `PART_TIMER`, `Part.timer` facts, the
  rejections of § 6.
- In a circuit, a timer wired as one of the four circuits of § 4 toggles
  OUT with the times of § 4's formulas (within 1 ps of the rounded value);
  a timer with no capacitor on its timing nodes is a set/reset latch on
  logic levels; any other wiring keeps OUT low with a reason;
  `circuit_timer_state` reports mode, reason and times.
- `ctest` passes T0 + 1 programs (`timer_test`); the goldens are unchanged
  (no part file is added here).

### Non-goals (explicit constraints)

- No analog simulation: nets keep logic levels; the capacitor's voltage is
  not a net value. The timing nodes show the level the weak links give them.
- No change to how resistors, capacitors, diodes or supplies behave in the
  engine or in the voltage check.
- The analysis runs once, in `circuit_start`, on the nets as wired. It does
  not follow switch contacts, and a switch thrown during the run does not
  re-time a timer.
- Series resistors or capacitors, potentiometers, a modulated CONT pin and
  frequency-shift circuits are not recognized (reason `shape` or `control`).
- No part file, no app change (G, H).

## 2. Current State

- A kind is a `PartKindOps` in `backend/src/models/<kind>.c`, listed in
  `KINDS[]` (`backend/src/library/kinds.c:72`, last entry `&GENERIC_KIND`);
  `PartKind` ends with `PART_GENERIC` (`backend/include/parts.h:30`).
- A model sees only its own pins (`SimComponentType`, `backend/src/engine/sim.h`);
  it schedules drives and timers. `circuit.c` knows the netlist:
  `Circuit.connections` (`VoltageConnection {part, pin, net}`), `parts[]`,
  `instances[]`.
- Resistor values: `Part.resistor->milliohms`; links by
  `resistor_pair(part, k, &a, &b)` for `k < resistor_count`
  (`model_pins.h`). Capacitor values: `Part.capacitor->capacitance_ff`; every
  capacitor pinout has 2 pins (138 of 138 pinouts of `capacitor.json`).
  Diodes: `diode_pair(part, k, &anode, &cathode)`. Supplies:
  `Part.supply` `{level, mv, pull}`; the parts are `GND`, `5V`, `3V3`, …,
  `PULLUP-5V`, …, `PULLDOWN`.
- A placed `GENERIC-RESISTOR` or `GENERIC-CAPACITOR` has a value the user
  sets (`PlacedChip.ohms`, `.farads`), which the backend never receives:
  `circuit_add_part` takes a part name only.

### Verified properties

- **The voltage check does not refuse the timing circuits.** It reports
  only over-range pins (`VoltageProblemKind`, `backend/include/voltage.h`);
  a resistor carries each end's level to the other end's net
  (`voltage.c:323`), so the timing nodes carry at most the supply's voltage,
  which is within a timer's `vcc` range by construction.
- **Weak links cause no short**: a resistor link is never a strong drive
  (`sim.h`, the `resistors` hook comment), so DISCH driving 0 on a node a
  resistor ties to the supply is legal.
- **`trace_check` drives a part alone, by pin role** (`trace_part`,
  `backend/tests/trace_check.c:57`): a timer there has no analysis and runs
  in latch mode, which is deterministic.
- **The baseline is green**: 37 of 37 tests at 59b90a8f (plan-48-A § Prerequisites).

## 3. The kind

### 3.1 Part file format (`"kind": "timer"`)

Device keys:

| Key | Value |
|---|---|
| `"process"` | `"bipolar"` or `"cmos"` |
| `"timers"` | 1 or 2 sections, each `{"trigger", "threshold", "discharge", "output"}` (required pin names) and optional `"reset"`, `"control"` |
| `"maxHz"` | the highest astable frequency, 1…100000000 |
| `"tpd"` | trigger-to-output propagation delay in ns (`kind_ns`), above 0 |
| `"vcc"`, `"inputMax"` | as every kind (`kind_voltage`) |

Part keys: `"vcc"`, `"inputMax"` (`kind_part_voltage`: a part ends with a range).

Pin rules (`bind_pinout`): each named pin exists once (`kind_pin`); trigger,
threshold, reset and control pins have role `input`, output and discharge
pins role `output`; no pin is named by two sections; exactly one `power` pin
and one `ground` pin (other pins: `nc`).

```c
/* A 555-type timer (plan-48-E): timer_count sections (1, or 2 for a 556);
   process "bipolar" or "cmos"; max_hz: the highest astable frequency;
   tpd_ps: trigger-to-output delay. */
typedef struct { size_t timer_count; const char *process; uint64_t max_hz, tpd_ps; } PartTimer;
```

`const PartTimer *timer;` on `Part`. `pin_open_drain` answers 1 for a
discharge pin; `pin_analog` answers 1 for a control pin.

### 3.2 The model (`TIMER_MODEL`, `timer.c`)

Per section: a latch `Q` (0, 1 or X), a mode, three times, one pending
engine timer. `OUT` = `Q` (strong); `DISCH` = strong 0 when `Q` is 0, Z
when `Q` is 1, X when `Q` is X. A pin the section does not name (no
`reset`) reads as inactive. RESET reads: 0 = active, 1 or Z = inactive, X =
`Q` X until it resolves. Every output change is scheduled `tpd` after its
cause with `sim_schedule_inertial_drive`, except the astable and one-shot
period ends, which fall exactly on the computed times.

- **Latch mode** (the default, and the mode `trace_check` sees). RESET
  active → `Q` 0. Else TRIG 0 → `Q` 1; TRIG 1 and THRES 1 → `Q` 0; TRIG 1
  and THRES 0 → hold; TRIG X or Z → `Q` X, unless THRES is 1 and `Q` is
  already 0 (hold); TRIG 1 and THRES X or Z → `Q` X when `Q` is 1, else
  hold. `Q` starts 0 and is evaluated at start.
- **Astable mode** (`first_ps`, `high_ps`, `low_ps`). TRIG and THRES are not
  read. At start with RESET inactive: `Q` 1 for `first_ps`, then 0 for
  `low_ps`, 1 for `high_ps`, and so on. RESET active: `Q` 0, the pending
  timer is dropped; on release the cycle restarts with `first_ps`.
- **One-shot mode** (`high_ps`). Idle `Q` 0. TRIG reading 0 while idle
  (also at start) → `Q` 1 for `high_ps`; at its end `Q` returns to 0 when
  TRIG reads 1, else `Q` stays 1 until TRIG reads 1. TRIG during the pulse
  does nothing. RESET active → `Q` 0 at once and the pulse is dropped; on
  release with TRIG 0 a new pulse starts. THRES is not read.
- **Stopped mode**: `Q` 0 always.

`timer.h` (internal, the `model_pins.h` style) gives `circuit.c`:

```c
typedef struct { size_t vcc, gnd, trig, thres, disch, out, reset, cont; } TimerPins; /* SIZE_MAX = none */
int timer_section_pins(const Part *part, unsigned section, TimerPins *out);   /* 0 past the last */
void timer_model_set(void *state, unsigned section, const TimerState *timing); /* before sim_start */
int timer_model_get(const void *state, unsigned section, TimerState *out);
```

### 3.3 Public additions (`backend/include/timer_state.h`, `circuit.h`)

```c
typedef enum { TIMER_LATCH, TIMER_ASTABLE, TIMER_ONE_SHOT, TIMER_STOPPED } TimerMode;
typedef enum { TIMER_OK, TIMER_NO_SUPPLY, TIMER_CONTROL, TIMER_SHAPE, TIMER_NO_VALUE,
               TIMER_TOO_FAST } TimerReason;
typedef struct { TimerMode mode; TimerReason reason; uint64_t first_ps, high_ps, low_ps; } TimerState;
const char *timer_reason_text(TimerReason reason);

/* A resistor's (milliohms), a pull-up's or pull-down's (milliohms) or a
   capacitor's (femtofarads) value for this circuit, in place of the
   library part's; read by the timer analysis only. Before circuit_start.
   -1: "part N has no value" (another kind), "the value must be above 0". */
int circuit_set_part_value(Circuit *circuit, size_t part, uint64_t value);
/* A timer section's mode, reason and times. -1: "part N is not a timer",
   "timer S of part N does not exist", "the circuit is not running". */
int circuit_timer_state(const Circuit *circuit, size_t part, unsigned section, TimerState *out);
```

`timer_reason_text`: `TIMER_NO_SUPPLY` "its VCC and GND pins are not both on
a supply"; `TIMER_CONTROL` "the CONT pin is wired to something other than a
capacitor to ground"; `TIMER_SHAPE` "the resistors and capacitors on TRIG,
THRES and DISCH do not form an astable or a one-shot circuit";
`TIMER_NO_VALUE` "a timing resistor or capacitor has no value";
`TIMER_TOO_FAST` "the timing is faster than this part allows"; `TIMER_OK` "".

`Circuit` gains `uint64_t *values` (one per part, 0 = the library value),
grown with `parts`. A pull part without a set value counts as 1 kΩ
(1000000 mΩ), the app's default (`ResistorValue.defaultOhms`).

## 4. The timing analysis (`backend/src/timer_rc.c`, `timer_rc.h`)

```c
void timer_rc_analyze(const Part *const *parts, const uint64_t *values, size_t part_count,
                      const VoltageConnection *connections, size_t connection_count,
                      size_t part, unsigned section, TimerState *out);
```

`circuit_start` calls it for every section of every timer part after the
voltage check passes and before `sim_start`, and hands the result to
`timer_model_set`.

**Terms.** A *ground net* carries a supply part with `level` 0 and no
`pull`. A *supply net* carries a supply part with `level` 1, no `pull`, and
the same `mv` as the one on the timer's VCC pin's net. `T`, `R`, `D`, `O`,
`K` are the nets of the section's THRES, TRIG, DISCH, OUT and CONT pins (a
pin that is not wired has no net). A *resistor between X and Y* is a
resistor link with one end on net X and the other on Y, or, when Y is a
supply net, a pull-up part on X; parallel ones combine (`1/R = Σ 1/Rᵢ`). A
*capacitor on X* is a capacitor part with one pin on X and the other on a
ground net; several add. A *foreign driver* is any pin on the net, of
another part, with role `output` or `inout`, or a supply part without
`pull`. `C` is the *timing node*.

**Steps**, in order; the first that decides ends the analysis:

1. No capacitor part has a pin on `T`, `R` or `D` → `TIMER_LATCH`, `TIMER_OK`.
2. The VCC pin is not on a net with a `level` 1 supply without `pull`, or
   the GND pin is not on a ground net → `TIMER_STOPPED`, `TIMER_NO_SUPPLY`.
3. `K` exists and carries any pin other than capacitors on `K` (as defined
   above) → `TIMER_STOPPED`, `TIMER_CONTROL`.
4. Match one of the four circuits. In each, `C` and (in A and A-diode) `D`
   carry no foreign driver, no capacitor other than capacitors on `C`, no
   resistor link and no diode link other than the ones the circuit names
   (pull-downs count as resistors to a ground net), and `C` has at least
   one capacitor on it.

   | Circuit | Wiring | `first_ps` | `high_ps` | `low_ps` |
   |---|---|---|---|---|
   | **A** astable | `T` = `R` = `C`, `D` ≠ `C`; `RB` between `C` and `D`; `RA` between `D` and a supply net | ln 3 · (RA + RB) · C | ln 2 · (RA + RB) · C | ln 2 · RB · C |
   | **A-diode** | as A, plus one or more diode links with the anode on `D` and the cathode on `C` | ln 3 · RA · C | ln 2 · RA · C | ln 2 · RB · C |
   | **B** 50 % astable | `T` = `R` = `C`; `R1` between `C` and `O`; the DISCH pin not on `C` (it may be unwired or on any other net, which is then not checked) | ln 3 · R1 · C | ln 2 · R1 · C | ln 2 · R1 · C |
   | **M** one-shot | `T` = `D` = `C`, `R` ≠ `C`; `R1` between `C` and a supply net | 0 | ln 3 · R1 · C | 0 |

   No match → `TIMER_STOPPED`, `TIMER_SHAPE`.
5. A matched resistor or capacitor whose value is 0 (a jumper, or a
   combined value of 0) → `TIMER_STOPPED`, `TIMER_NO_VALUE`.
6. Times: `llround(k · (double)milliohms · (double)femtofarads · 1e-6)`
   picoseconds, with `k` = `0.6931471805599453` (ln 2) or
   `1.0986122886681098` (ln 3); a part's value is `values[part]` when set,
   else the library's. A time that rounds to 0 → `TIMER_NO_VALUE`.
7. Astable with `high_ps + low_ps < 1000000000000 / max_hz`, or one-shot
   with `high_ps < 2 · tpd_ps` → `TIMER_STOPPED`, `TIMER_TOO_FAST`.
8. Else `TIMER_ASTABLE` (A, A-diode, B) or `TIMER_ONE_SHOT` (M), `TIMER_OK`.

The diode drop, the output's saturation voltages and part tolerances are
not modelled: the formulas are the datasheets' nominal ones (thresholds at
1/3 and 2/3 of the supply).

**Where the risk is.** Step 4's exclusions decide between "recognized" and
"stopped"; each has a test case in § 6 (k–u).

## 5. Docs

- `docs/parts-format.md`: a `### timer` section (keys, pin rules, an
  example device with a `555-8` pinout), listed in the Kinds order.
- `docs/backend.md`: § Layout names `timer.c`, `timer.h`, `timer_rc.c`,
  `timer_rc.h`, `timer_state.h`; § Using it gains a paragraph on
  `circuit_set_part_value` and `circuit_timer_state`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind: loader, facts, latch mode

- [x] `backend/include/parts.h`: `PART_TIMER` after the current last
      `PartKind` (`grep -n 'PART_' backend/include/parts.h`: `PART_GENERIC`
      at planning time; after whatever plan-42, -44 or -47 added since),
      `PartTimer`, `const PartTimer *timer;` on `Part`. (`PART_BUZZER` was last; `PART_TIMER` follows it.)
- [x] `backend/src/models/timer.c`, `timer.h`: `TIMER_KIND` (`load_device`,
      `bind_pinout`, `load_part`, `pin_open_drain`, `pin_analog`) per § 3.1
      and `TIMER_MODEL` with latch mode and stopped mode per § 3.2;
      `timer_section_pins`, `timer_model_set`, `timer_model_get`.
- [x] `backend/src/library/kinds.c`: `extern const PartKindOps TIMER_KIND;`
      and `&TIMER_KIND` at the end of `KINDS[]` (`grep -c 'TIMER_KIND'
      backend/src/library/kinds.c` → 0 before, 2 after);
      `backend/CMakeLists.txt`: `src/models/timer.c` in `logic` and
      `logic_test(timer_test)`.
- [x] `backend/tests/support/listing.c`: `case PART_TIMER: return "timer"`
      and the facts line `timers=%zu process=%s maxhz=%llu tpd=%llu`;
      `tests/trace_check.c` `TRACE_LAST_KIND` and `tests/listing_check.c`'s
      two array sizes → `PART_TIMER` (moot when a later kind already sizes
      them: `grep -c 'PART_TIMER' backend/tests/trace_check.c
      backend/tests/listing_check.c` → 1 each after this step, or the later
      kind's name with `PART_TIMER` before it in the enum). (`trace_check.c`'s `TRACE_LAST_KIND`
      and `listing_check.c`'s two array sizes name `PART_TIMER` now.)
- [x] `backend/tests/support/timer_fixture.h` and `backend/tests/timer_test.c`:
      cases (a)–(j) of § 6.

Acceptance: the kind loads, the latch works, goldens unchanged.
  Check: `cmake -S backend -B /tmp/logic-p48-e && cmake --build /tmp/logic-p48-e -j8 && /tmp/logic-p48-e/timer_test` → every line `PASS`, and `/tmp/logic-p48-e/listing_check` and `/tmp/logic-p48-e/trace_check` → exit 0 (est. 4 min: a build from scratch).
  Result (2026-11-24): `timer_test` cases (a)–(j) all `PASS`; `listing_check` → `39903 parts compared, 0 skipped` (exit 0); `trace_check` → `39903 parts compared, 0 skipped` (exit 0); `footprint_check` and `library_check` exit 0. MET.
Commit: 52fdcecb

### Phase 2 — Part values, the analysis, astable and one-shot modes

- [x] `backend/include/timer_state.h`; `circuit.h` / `circuit.c`:
      `circuit_set_part_value`, `circuit_timer_state`, `Circuit.values`
      (each added only if absent: the prerequisite's grep). `timer_reason_text` is in `timer_rc.c`.
- [x] `backend/src/timer_rc.c`, `timer_rc.h` per § 4; `CMakeLists.txt`:
      `src/timer_rc.c` in `logic`; `circuit_start` calls the analysis and
      `timer_model_set` between the voltage check and `sim_start` (`analyze_timers` in `circuit.c`).
- [x] `timer.c`: astable and one-shot modes per § 3.2.
- [x] `timer_test.c`: cases (k)–(z) of § 6.

Acceptance: the four circuits time as computed; every reason is reachable.
  Check: `cmake --build /tmp/logic-p48-e -j8 && /tmp/logic-p48-e/timer_test` → every line `PASS`; `ctest --test-dir /tmp/logic-p48-e -j8` → `100% tests passed`, T0 + 1 programs (est. 3 min).
  Result: `timer_test: all passed` (every case a–z); `ctest --test-dir /tmp/logic-p48-e -j8` → `100% tests passed, 0 tests failed out of 66` (T0 = 65, plus `timer_test`). MET.
Commit: 52fdcecb

### Phase 3 — Docs

- [x] `docs/parts-format.md` and `docs/backend.md` per § 5.

Acceptance: the docs name the kind and the two calls.
  Check: `grep -n '^### `timer`' docs/parts-format.md` → 1 line; `grep -c 'circuit_set_part_value\|circuit_timer_state' docs/backend.md` → above 0 (est. < 1 min).
  Result: `1159:### `timer``; `2`. MET.
Commit: 52fdcecb

## 6. `timer_test` cases

Fixture (`timer_fixture.h`, loaded next to the real part files): a device
`"process": "bipolar"`, `"maxHz": 500000`, `"tpd": 100`, `"vcc": [4.5, 16]`
with pinout `p8` (the `555-8` pinout of plan-48-B § 3.1) and part
`TESTTIMER` (footprint `PDIP-8`); a `cmos` device with two sections, pinout
`p14` (`556-14`), `"maxHz": 2000000`, `"tpd": 50`, `"vcc": [2, 15]` and part
`TESTTIMER2` (footprint `PDIP-14`). Circuits use the library's `5V`, `GND`,
`PULLUP-5V`, `GENERIC-RESISTOR`, `GENERIC-CAPACITOR` and `1N4148`, with
values set through `circuit_set_part_value`. "At t" means after
`circuit_advance` to t; times in picoseconds.

Facts and loader:

- (a) `TESTTIMER`: kind `PART_TIMER`, 8 pins, `timer_count` 1, `process`
  "bipolar", `max_hz` 500000, `tpd_ps` 100000, `vcc` 4500–16000 mV; pin 7
  is open drain (`part_pin_open_drain`), pin 5 analog (`part_pin_analog`).
- (b) `TESTTIMER2`: `timer_count` 2, `process` "cmos", 14 pins.
- (c) `"process": "nmos"` → `unknown timer process "nmos"`; no `"timers"` →
  `missing "timers"`; three sections → `"timers" must have 1 or 2 entries`.
- (d) a section without `"trigger"` → `missing "trigger"`; a section naming
  a pin the pinout lacks → the `kind_pin` error; two sections naming the
  same pin → `timer pin "OUT" is named twice`.
- (e) a trigger pin with role `output` → `timer pin "TRIG" must be an
  input`; an output pin with role `input` → `timer pin "OUT" must be an
  output`; a pinout with two `power` pins → `a timer has one power pin and
  one ground pin`.
- (f) `"maxHz": 0` → `"maxHz" must be a whole number from 1 to 100000000`;
  `"tpd": 0` → `"tpd" must be above 0`; a part key the kind does not list
  (`"ohms": 5`) → `unknown key "ohms"`.

Latch mode (no capacitor; pins driven through `5V` / `GND` parts on
switch-free nets, one circuit per case):

- (g) TRIG on `GND`, THRES on `GND`, RESET on `5V`: at 1 µs OUT = 1 and the
  DISCH net (with a `PULLUP-5V`) = 1; `circuit_timer_state` → `TIMER_LATCH`,
  `TIMER_OK`.
- (h) TRIG on `5V`, THRES on `5V`: OUT = 0, DISCH net = 0.
- (i) TRIG on `GND`, THRES on `5V`: OUT = 1 (trigger wins).
- (j) as (g) with RESET on `GND`: OUT = 0. `TESTTIMER2`: section 1 as (g),
  section 2 as (h): OUT1 = 1, OUT2 = 0.

Astable (circuit A: RA 1 kΩ `5V`–D, RB 10 kΩ D–C, 100 nF C–`GND`, TRIG and
THRES on C, RESET on `5V`, CONT unwired):

- (k) `circuit_timer_state` → `TIMER_ASTABLE`, `TIMER_OK`, `first_ps`
  1208473518, `high_ps` 762461899, `low_ps` 693147181 (each within 1).
- (l) OUT = 1 at 1 ms; = 0 at 1.5 ms (after `first_ps` 1.2085 ms); = 1 at
  2.2 ms (after 1.9016 ms); = 0 at 2.9 ms (after 2.6641 ms);
  `circuit_net_changes(OUT)` after 20 ms is 26 ± 1 (first edge at 1.2085 ms,
  then one every 0.693 / 0.762 ms).
- (m) RA as a `PULLUP-5V` on D with value 1 kΩ, and the capacitor's ground
  on a second `GND` part's net: the same times as (k).
- (n) two 20 kΩ resistors in parallel as RB and two 47 nF + 53 nF
  capacitors: the same times as (k).
- (o) with RESET on a net driven low (a `GND` part): OUT = 0 at every time,
  `circuit_net_changes(OUT)` 0, mode still `TIMER_ASTABLE`.
- (p) A-diode: (k) plus a `1N4148` anode on D, cathode on C: `first_ps`
  109861229, `high_ps` 69314718, `low_ps` 693147181.
- (q) circuit B on `TESTTIMER2` section 2 (10 kΩ OUT2–C, 100 nF, DISCH2
  unwired): `high_ps` = `low_ps` = 693147181, `first_ps` 1098612289;
  section 1 unwired stays `TIMER_LATCH`.

One-shot (circuit M: 10 kΩ `5V`–C, 100 nF C–`GND`, THRES and DISCH on C,
TRIG on a net with a `PULLUP-5V` and a tactile switch part to `GND`, set
with `circuit_set_switch`):

- (r) state `TIMER_ONE_SHOT`, `high_ps` 1098612289; OUT = 0 at 1 ms with the
  switch open; close the switch at 1 ms and open it at 1.1 ms: OUT = 1 at
  1.5 ms, = 0 at 2.2 ms (the pulse ends at 2.0986 ms).
- (s) the switch held closed from 1 ms to 3 ms: OUT = 1 at 2.5 ms (TRIG
  still low after the pulse), = 0 at 3.1 ms.
- (t) a second press at 1.5 ms during the pulse does not extend it: OUT = 0
  at 2.2 ms.

Stopped, one reason each (`circuit_start` returns 0, OUT = 0 at 5 ms,
`circuit_net_changes(OUT)` 0):

- (u) circuit A with RB between C and the supply instead of C and D →
  `TIMER_SHAPE`; circuit A with a second 10 kΩ from C to `GND` →
  `TIMER_SHAPE`; circuit A with an `SN74HC04N` output on C → `TIMER_SHAPE`;
  circuit A with an `SN74HC04N` input on C → still `TIMER_ASTABLE`.
- (v) circuit A with the timer's GND pin unwired → `TIMER_NO_SUPPLY`.
- (w) circuit A with CONT on `5V` → `TIMER_CONTROL`; with a 10 nF from CONT
  to `GND` → `TIMER_ASTABLE`.
- (x) circuit A with RA 10 Ω, RB 10 Ω and 1 nF on `TESTTIMER` (period
  20.8 ns, above 500 kHz) → `TIMER_TOO_FAST`.
- (y) circuit A with a 0 Ω library resistor (`milliohms` 0, the fixture's
  `TESTJUMPER`) as RB → `TIMER_NO_VALUE`.

Calls:

- (z) `circuit_set_part_value` on a timer part → -1, `part N has no value`;
  value 0 → `the value must be above 0`; after `circuit_start` → the
  building error; `circuit_timer_state` on a resistor → `part N is not a
  timer`, section 1 of `TESTTIMER` → `timer 1 of part N does not exist`,
  before start → `the circuit is not running`.

## Validation Plan

- Tests: `timer_test` (§ 6); `listing_check`, `trace_check` unchanged
  goldens; `library_check` stays green.
- Coverage check: `ctest --test-dir /tmp/logic-p48-e -N | grep -c timer_test`
  → 1.
- Runtime proof: case (l): an OUT net toggling 26 times in 20 ms.
- Doc sync: § 5.
- Final gate: plan-48-I.

## Decisions

- Owner, 2026-11-16: timing comes from the wired resistors and capacitors;
  unrecognized wiring keeps OUT low with a reason.
- Author: **the analysis lives in `circuit.c`'s layer, not in the engine**:
  a model sees only its pins; the netlist is the circuit's.
- Author: **four circuits** (astable, diode astable, 50 % astable,
  one-shot) and **the latch**: a timer with no capacitor on its timing
  nodes is the bistable / Schmitt-trigger use, which needs no timing and is
  not "unrecognized".
- Author: **placed values reach the backend through
  `circuit_set_part_value`**; without it the library value holds (a catalog
  resistor's own ohms).
- Author: **a pull-up part is a resistor to the supply** for the analysis
  (the app's usual way to draw one); parallel parts combine; series parts
  do not.
- Author: **ground and supply are by supply parts on the net**, not by net
  identity: every `GND` symbol is its own part and may sit on its own net.
- Author: **an input pin of another part on a timing node is allowed**, a
  driver is not.
- Author: **the first astable period is ln 3 · R · C** (the capacitor starts
  empty), as on a real part.
- Author: **nominal formulas only**: no diode drop, no tolerance.

## Corrections

1. **`circuit_set_resistance` already exists** (plan-42-G landed first): it sets a generic resistor's
   value for the voltage check (`Circuit.milliohms`). The analysis reads `circuit_set_part_value`'s
   value first, then `circuit_set_resistance`'s, then the library's, so a resistor placed through
   either call times a timer (`analyze_timers` builds the effective array). `circuit_set_part_value`
   also takes pull-up and pull-down parts and capacitors, which `circuit_set_resistance` does not.
2. **The first evaluation is an engine event.** A part's pins read Z at its own `start` callback
   (the supplies' time-0 drives have not applied), so a timer started with RESET tied low glitched
   OUT high for 100 ns. `timer_start` schedules one delay-0 timer event per section (`START_ID`) and
   `begin_section` runs there; pin changes before it are ignored. Times are unchanged (the event is at
   time 0).
3. **Net changes.** The plan's "`circuit_net_changes(OUT)` 0" in (o) and the stopped cases
   (u–y) is `<= 1` in the tests: the first drive of OUT (Z to 0) is one change.
4. **Test parts.** (y) uses the library's `CRCW06030000Z0EBC` (0 ohm) instead of a `TESTJUMPER`
   fixture; (r)–(t) use a fixture switch `TESTTACT` (`timer_fixture.h`). Test (l) accepts 25–27
   changes in 20 ms (plan: 26 ± 1).
5. **`timer_rc_analyze` builds its pin→net table, diode, capacitor and link lists on every call**
   (O(connections) per section) instead of once per start; a circuit has a handful of timers.
6. **Prerequisite values moved** (main advanced): T0 = 65 (was 37); the goldens are 39903 parts (was
   32897) and unchanged by this letter.

## Summary

The risk is § 4 step 4: which wirings count. Every exclusion has a test.
The engine, the voltage check and the passive kinds are untouched.
