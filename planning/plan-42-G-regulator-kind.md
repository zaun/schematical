# plan-42-G: Backend — the `regulator` kind, the regulated level, the divider, the VIN rating

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-F (G reads no census output; it follows F because letter
order is implementation order)

G adds the `regulator` part kind. A regulator part is one to four
**regulators** (elements); each takes power on its input pin and, while that
input reads 1 and its enable (if it has one) is active, drives its output pin
strong 1. In the voltage check the output is a **regulated level**: the
element's output voltage, fixed by the part, or, for an adjustable element,
worked out from the two resistors of its feedback divider (Vout = Vref × (1 +
Rtop / Rbottom), or Vref × (1 + Rbottom / Rtop) for an LM317-style ADJ pin).
An input above the part's maximum is refused. The behavioral outcome:
`regulator_test` passes the cases of § 6, `ctest` passes T0 + 1 programs,
and every golden is byte-identical (no part uses the kind yet; H adds them).

References:

- `backend/src/voltage.c` (the whole file: groups, supplies and clashes,
  the fixed point, the checks; § 2 below), `backend/include/voltage.h`
  (`VoltageProblemKind`, `voltage_check`, `voltage_describe`).
- `backend/src/models/transistor.c` (the precedent for elements, a 1 ns
  timer per element, tied same-name pins), `backend/src/models/supply.c`
  (strong drives with `sim_schedule_drive`), `backend/src/models/resistor.c`
  (`resistor_pair`, `PartResistor.milliohms`).
- `backend/src/library/kind.h` (`PartKindOps`: `pin_open_drain`,
  `pin_analog`), `backend/src/library/kinds.c` (`KINDS[]`).
- `backend/include/parts.h`, `backend/include/circuit.h`,
  `backend/src/circuit.c` (`voltage_failed`).
- `docs/backend.md` § Adding a kind, § Voltages; `docs/parts-format.md` §
  Kinds.
- `planning/completed/plan-40-F-transistor-kind.md` (the kind precedent:
  fixture parts, the § 6 case list, the rating branch).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-A § Prerequisites MET | plan-42-A's table | NOT MET (2026-11-15: plan-42 not started) |
| plan-42-F complete | `ls planning/completed/plan-42-F-*.md` → exists | NOT MET (2026-11-15: not started) |
| No `regulator` kind yet | `grep -c 'PART_REGULATOR' backend/include/parts.h` → 0 | MET (2026-11-15 at 40ccd5fc: 0) |
| `voltage_check` has the planning-time signature (3 call sites outside the tests) | `grep -rn 'voltage_check(' backend/src schematical/schematical \| grep -v '^backend/src/voltage.c' \| wc -l` → 3 | MET (2026-11-15 at 40ccd5fc: 3 — `circuit.c:261`, `VoltageCheck.swift:81`, `:87`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p42` → `100% tests passed` with T0 + 1
  programs (T0 = 36 at 40ccd5fc: `grep -c '^logic_test' backend/CMakeLists.txt` → 36).
- `regulator_test` passes the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` are
  byte-identical (P0 = R0 = 32854, F0 = 1624 at 40ccd5fc).
- The app still builds (`VoltageCheck.swift` passes `nil` for the new
  argument; I passes real values).

### Non-goals (explicit constraints)

- No analog behaviour in the simulation: no ramp, no load current, no
  current limit, no thermal shutdown, no ripple, no line or load
  regulation. VOUT is a digital 1 while the element is on.
- No inductor, diode or capacitor is needed or checked around a switching
  regulator (the library has no inductor kind; the output pin is the one §
  3.1 names).
- Every circuit without a regulator part resolves and checks exactly as
  before: the new level class is empty, the new argument may be NULL.
- The divider is two resistors (each side may be several in parallel). A
  series chain, a potentiometer, or anything else on the adjust net leaves
  the level unknown; it is not an error.
- No part-editor form for the `regulator` kind (plan-42-I § Decisions).
- No negative output: every element's output is positive (plan-42-A §
  Decisions).

## 2. Current State

- `voltage_check` (`voltage.c:113`) is a pure function over parts, nets,
  connections and closed contacts. Step 2 sets the supply level S per group
  from `PART_SUPPLY` parts and records clashes; step 3 is a fixed point in
  which LEDs, diodes and resistors carry levels (C) and every part with a
  `vcc` drives its outputs (D) at the highest level on its power pins; step
  4 checks every connection; `effective()` (`voltage.c:100`) is S, else D,
  else P (pull), else C. A part with `vcc == NULL` is skipped by both the
  driving branch (`voltage.c:359`: `if (part->vcc == NULL) continue;`) and
  the range and input checks (`voltage.c:491–501` test `part->vcc != NULL`),
  so a regulator with no `vcc` gets only what G adds.
- A resistor carries each end's level to the other end at full value
  (`voltage.c:323–357`); there is no divider arithmetic anywhere.
- `GENERIC-RESISTOR` has `"ohms": 1000` in `resistor.json`, and a placed one
  saves its own `ohms` (`docs/file-format.md:133`: "the simulation ignores
  it"). The backend never sees the placed value: `voltage_check` takes only
  `Part` pointers (`circuit.c:261`, `VoltageCheck.swift:81`).
- Kinds provide `pin_open_drain` and `pin_analog` (`kind.h:41–45`); the
  voltage check skips analog pins and open-drain outputs.
- `PartKind` ends with `PART_TRANSISTOR` (`parts.h:29`); `TRACE_LAST_KIND`
  is `PART_TRANSISTOR` (`tests/trace_check.c:22`); `listing_check.c:50–51`
  size two arrays by `PART_TRANSISTOR + 1`; `tests/support/listing.c:39,
  170` name and print the kind's facts.
- `VoltageProblemKind` ends with `VOLTAGE_TRANSISTOR_OVER` (`voltage.h:32`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Test programs T0 | 36 | `grep -c '^logic_test' backend/CMakeLists.txt` → 36 (40ccd5fc) |
| Golden sizes P0 / R0 / F0 | 32854 / 32854 / 1624 | `git show HEAD:backend/tests/golden/{parts,traces,footprints}.txt \| wc -l` (40ccd5fc) |
| `voltage_check(` calls outside `voltage.c` | 3 in the app and the circuit + 4 in the tests | `grep -rn 'voltage_check(' backend/src schematical/schematical \| grep -v '^backend/src/voltage.c'` → 3; `grep -rn 'voltage_check(' backend/tests` → `voltage_test.c:92, 496`, `charlcd_test.c:1639, 1659` |

### Verified properties

- **A regulator needs no `vcc`.** Read `voltage.c:359` and `:491–520`: with
  `vcc` NULL the generic driving branch and the generic range checks never
  touch the part; G's branches are the only ones that do.
- **A model can drive strong 1 and Z from a timer.** `supply.c:63–64` drives
  with `sim_schedule_drive(…, DRIVE_STRONG, 0)`; `transistor.c` schedules a
  1 ns timer from `pin_changed` and acts in `timer` (plan-40-F § 3.2).
- **The problem buffer holds one clash per group.** `found` has
  `connection_count + net_count + 1` slots (`voltage.c:125`); G reports at
  most one clash per group (the supply clash first, else the regulated
  one), so the bound holds.

## 3. Design

### 3.1 The kind

`backend/src/models/regulator.c`, `REGULATOR_KIND` (`"regulator"`), file
`Parts/regulator.json`. `PART_REGULATOR` is appended after `PART_TRANSISTOR`.

Device keys:

| Key | Value |
|---|---|
| `"regulators"` | array of 1 to 4 objects, one per regulator (`"regulators" must have 1 to 4 entries`) |
| `"analog"` | names of the pins the voltage check skips: FB, ADJ, SW (when it is not the output), BOOT/BST, COMP, SS/TR, NR/SS, BYP, SENSE, VREF, RT, ILIM, MODE straps that take resistors; may be `[]` |

Each `"regulators"` entry:

| Key | Value |
|---|---|
| `"type"` | `"linear"`, `"buck"`, `"boost"`, `"buck-boost"`, `"charge-pump"` or `"sepic"` |
| `"in"` | the input pin name (every pin of the name: a VIN pair, a tab) |
| `"out"` | the output pin name: VOUT / OUT / VO / VOS when the part has one, else the switch pin (SW, LX, OUTPUT) — plan-42-C § 3 rule 4 |
| `"enable"` | optional: the enable pin name (EN, ON/OFF, SHDN) |
| `"enableActive"` | optional, with `"enable"`: `"high"` (default) or `"low"` |
| `"pg"` | optional: the power-good pin name (open-drain) |
| `"adjust"` | optional: the feedback / adjust pin name; present ⇔ the element is adjustable |
| `"adjustTo"` | optional, with `"adjust"`: `"ground"` (default: FB is Vref above ground) or `"output"` (LM317 ADJ: Vref is between OUT and ADJ) |

Roles (`regulator pin "X" must be …`): `in` power; `out` output; `enable`
input; `pg` output (open-drain: `pin_open_drain`); `adjust` input and in
`"analog"`; ground pins ground; every other pin is in `"analog"`, `nc` or
`absent` (`pin "X" is in no regulator and not analog`). A name in an entry
must name at least one pin of every pinout.

Part keys (V, A, Hz; a number holds for every element, an array has one
value per element, in `"regulators"` order):

| Key | Value |
|---|---|
| `"vout"` | number or array: the fixed output voltage, above 0 and at most 100; required for every element without `"adjust"` and refused for one with it |
| `"vref"` | number or array: the reference of an adjustable element, above 0 and at most 10; required with `"adjust"`, refused without |
| `"voutMax"` | optional number: the highest output an adjustable part can be set to (facts only) |
| `"vinMin"` | optional number: the lowest input at which the element regulates; for a switching element required on maker parts (the generator always writes it) |
| `"vinMax"` | optional number: the input maximum, above 0 and at most 100 (generics give none) |
| `"dropout"` | optional number, linear only: the dropout at full load, at least 0 and at most 10 |
| `"iout"` | optional number: the output current per element, above 0 and at most 100 |
| `"iq"` | optional number: the quiescent current, above 0 and at most 1 |
| `"fsw"` | optional number, switching only: the switching frequency, above 0 and at most 1e8 |

Facts (`parts.h`):

```c
/* Regulator facts (plan-42-G). regulator_count 1..4; type: the first
   element's type; mixed: 1 when the types differ; adjustable: 1 when any
   element has an adjust pin. vout_mv / vref_mv: one per element (0 where
   not given). All other values 0 = not given. */
typedef struct { const char *type; size_t regulator_count; uint8_t mixed; uint8_t adjustable;
                 const uint32_t *vout_mv; const uint32_t *vref_mv;
                 uint32_t vout_max_mv, vin_min_mv, vin_max_mv, dropout_mv, iout_ma, iq_ua;
                 uint64_t fsw_hz; } PartRegulator;
```

`const PartRegulator *regulator;` on `Part` after `transistor`.
`model_pins.h`: `regulator_element(part, k, &type, &in, &out, &enable,
&enable_low, &pg, &adjust, &adjust_to_output)` (pin indices of the first pin
of each name, `SIZE_MAX` when absent; 0 past the last element),
`regulator_pin_in(part, k, pin)` (1 when pin carries element k's input name),
`regulator_input_max_mv(part)`.

### 3.2 The model

`SimComponentType REGULATOR_MODEL`: `pin_count` = `part->pin_count`;
`create` returns one `state` byte per element (0 off); `start`: closes the
contacts of tied same-name pins (as `transistor.c`) and schedules timer `k`
after `REGULATOR_DELAY_PS` = 1000 (1 ns) for every element; `pin_changed`
on an element's `in` or `enable` pin schedules its timer after 1 ns;
`timer(k)` reads the input (any pin of the name reading 1 counts as 1) and
the enable, then:

| Input | Enable (active level, or no enable pin) | `out` | `pg` |
|---|---|---|---|
| 1 | active, or no pin | strong 1 | Z |
| 1 | inactive, Z | Z | strong 0 |
| 1 | X | strong X | strong X |
| X | any | strong X | Z |
| 0 or Z | any | Z | Z |

The drive is `sim_schedule_drive(…, DRIVE_STRONG, 0)` from the timer, so a
change is always a later event than the read (no zero-time loop; a pulse
shorter than 1 ns never moves the output). A floating enable is inactive
for both polarities (the datasheets say "do not leave floating"; the
generics have no enable pin).

### 3.3 The voltage check

`voltage_check` gains one argument, `const uint64_t *milliohms`
(`part_count` entries or NULL): a non-zero entry replaces that resistor
part's `PartResistor.milliohms` (a placed `GENERIC-RESISTOR`'s value). The
three outside callers are updated: `circuit.c` passes its own array (§ 3.4),
`VoltageCheck.swift:81, 87` pass `nil` (I passes the values).

1. **The regulated level R.** A new `Level regulated[]` per group;
   `effective()` becomes S, else R, else D, else P, else C.
2. **Fixed point, a `PART_REGULATOR` branch** (before `vcc == NULL`): for
   each element k: `vin` = the highest known effective level over the
   element's `in` pins; skip when unknown or 0. Enable: skip when the
   enable pin's group has a known effective level that is inactive (active
   high: 0 mV; active low: above 0 mV); an unknown enable counts as active.
   Level `v`: `vout_mv[k]`, or for an adjustable element the divider of
   step 3 (skip when it is unknown). Headroom: a linear element needs `vin
   ≥ v + dropout_mv` and `vin ≥ vin_min_mv`; a switching element needs
   `vin ≥ vin_min_mv`; else skip (unpowered). Then raise `regulated` on
   every `out` pin's group to `v`; set `changed` as the other branches do.
3. **The divider** (`divider_mv`): O = the `out` group, A = the `adjust`
   group, a ground group = a group whose S is known and 0. Every resistor
   link (`PART_RESISTOR`, `resistor_pair`) with one end on A: its value is
   `milliohms[part]` when non-zero, else `PartResistor.milliohms`; the other
   end on O → Rtop (links in parallel combine), on a ground group → Rbottom;
   anywhere else, or a link of value 0 → unknown. Both sides must have at
   least one link. `adjustTo` ground: `v = vref × (Rtop + Rbottom) /
   Rbottom`; output: `v = vref × (Rtop + Rbottom) / Rtop`; rounded to the
   nearest mV (double arithmetic). Resistors also carry levels as before,
   so A gets O's level as a carried level; the adjust pin is analog and is
   not checked.
4. **Clashes.** After the fixed point, per group with R known: S known and
   non-zero and S ≠ R → `VOLTAGE_SUPPLY_CLASH` (`part` the supply,
   `other_part` the regulator, `mv` the higher, `min_mv = max_mv` the
   lower); two regulators on one group with different levels → the same
   kind (`part` the regulator with the lower level, so the app's message
   prints the lower voltage first, plan-42-I § 3). At most one clash per group: a
   supply clash found in step 2 wins. S known 0 (GND on an output) is the
   engine's short, not a voltage problem (as for a supply).
5. **The VIN rating.** In the checks, for a `PART_REGULATOR` part with
   `vin_max_mv > 0`: an `in` or `enable` pin whose effective level is above
   it → `VOLTAGE_REGULATOR_INPUT_OVER` (appended to `VoltageProblemKind`;
   `min_mv` 0, `max_mv` the rating). `voltage_describe`: `voltage: 24 V on
   pin 3 (VIN) of LM1117MPX-3.3/NOPB; its input max is 15 V`.

### 3.4 The circuit

`circuit.h`: `int circuit_set_resistance(Circuit *circuit, size_t part,
double ohms)` — the placed value of a resistor-kind part whose
manufacturer is `Generic` (`GENERIC-RESISTOR`), more than 0 and at most 1e9;
0, or -1 with `circuit_error`: `part N is not a generic resistor`, `the
resistance is out of range`. Any time after `circuit_add_part`; it changes
only the voltage check (the engine's resistor stays a weak link).
`Circuit` keeps a `uint64_t *milliohms` (grown with `parts`, 0 = the part's
own), passed to `voltage_check` by `voltage_failed`.

### 3.5 Rejected alternatives

- **The regulated level as a supply (S).** S is fixed before the fixed
  point; a regulator's output depends on its input, which may itself come
  from a regulator, so it must rise inside the fixed point. A separate R
  keeps the supply code and its clash rules untouched.
- **The regulated level as a driven level (D).** D never clashes, so a 3.3
  V regulator output tied to the 5 V rail would pass silently.
- **`vcc` for VIN.** The generic branch would drive VOUT at VIN's level and
  report an input below the minimum as a power-range problem; the owner
  chose "below the headroom is unpowered, not refused".
- **The adjustable output set in the inspector.** The owner chose the
  divider (plan-42-A § Decisions).
- **An inductor kind for the switch node.** No inductor kind exists and the
  owner's scope has none; driving the output (or switch) pin is enough for
  a logic simulator.

## 4. The fixture parts

`tests/support/regulator_fixture.h` (as `transistor_fixture.h`): a JSON
string of fixture devices loaded next to the real part files, names
`RFX-…`:

| Part | Elements | Keys |
|---|---|---|
| `RFX-LDO33` | linear, IN / OUT / GND / EN / PG, SOT-23-5 pins | `vout` 3.3, `dropout` 0.3, `vinMax` 6 |
| `RFX-LDO5` | linear, IN / GND / OUT | `vout` 5, `dropout` 2, `vinMax` 35 |
| `RFX-ADJ` | linear, IN / OUT / ADJ, `adjustTo` output | `vref` 1.25, `dropout` 2, `vinMax` 40 |
| `RFX-FB` | linear, IN / OUT / FB / GND / EN active low | `vref` 0.8, `dropout` 0.2, `vinMax` 6 |
| `RFX-BOOST` | boost, VIN / SW / VOUT / GND | `vout` 5, `vinMin` 0.9, `vinMax` 5.5 |
| `RFX-DUAL` | two linear, IN1 / IN2 / OUT1 / OUT2 / EN1 / EN2 / GND | `vout` [1.8, 3.3], `dropout` 0.2, `vinMax` 6 |
| `RFX-GEN` | linear, IN / GND / OUT | `vout` 5, `dropout` 2 (no `vinMax`) |

## 5. Docs

- `docs/parts-format.md`: a `### regulator` section (device keys, roles,
  part keys, the generics' table, the drawer paths of plan-42-H § 3).
- `docs/backend.md`: a **Regulators** bullet beside **Transistors** (the
  model table of § 3.2) and the regulated level, divider and VIN rating in
  **Voltages**; `circuit_set_resistance` with the circuit API.
- `docs/file-format.md:133`: the placed `ohms` now sets the divider of an
  adjustable regulator in the voltage check (wording: "the simulation
  ignores it; the voltage check uses it for an adjustable regulator's
  divider").

## 6. Test cases (`backend/tests/regulator_test.c`)

Kind cases (fixture parts, `load_parts` of a bad file for errors):

- (a) Each fixture loads; `PartRegulator` facts read back (type, count,
  mixed, adjustable, `vout_mv` / `vref_mv` arrays, the scalar keys).
- (b) Refusals, one per rule: `"regulators"` empty or 5 entries; an unknown
  `type`; `"vout"` on an element with `"adjust"`; `"vref"` without it; an
  array of the wrong length; `"dropout"` on a switching element; `"fsw"`
  on a linear one; an `in` pin that is not `power`; a pin in no element and
  not analog; `"enableActive"` without `"enable"`.

Engine cases (one fixture part in a rig with a scripted driver):

- (c) RFX-LDO33, IN 1, EN 1: OUT reads 1 after 1 ns, PG Z; EN 0: OUT Z, PG 0.
- (d) IN Z (not connected): OUT Z, PG Z.
- (e) EN Z: OUT Z (floating enable is inactive).
- (f) IN X: OUT X.
- (g) A 0.5 ns pulse on EN never moves OUT.
- (h) RFX-DUAL: EN1 1, EN2 0 → OUT1 1, OUT2 Z.
- (i) RFX-LDO5's OUT on the same net as GND → `smoke:` (the engine's short).

Voltage cases (`circuit_start` / `voltage_check` on fixture parts and real
supplies):

- (j) 5V → RFX-LDO33 IN, EN to IN; a 74HC00 (2–6 V) on OUT: no problem, and
  OUT's net is 3300 mV (`circuit_voltage_problem` none; a 1.2–2.7 V part on
  OUT is refused with `3.3 V`).
- (k) 3V3 → RFX-LDO33 (3.3 < 3.3 + 0.3): OUT has no level; a 1.2–2.7 V part
  on OUT is not refused.
- (l) 12V → RFX-LDO33: `VOLTAGE_REGULATOR_INPUT_OVER`, `max_mv` 6000, the
  § 3.3 step 5 text.
- (m) RFX-LDO33's OUT on the 5V rail → `VOLTAGE_SUPPLY_CLASH` (5000 vs
  3300).
- (n) RFX-FB from 5V, Rtop 10 kΩ (OUT–FB), Rbottom 4.7 kΩ (FB–GND), two
  maker resistor parts: OUT is 2502 mV. Two 10 kΩ in parallel on top: 1651 mV.
- (o) RFX-ADJ from 12V, R1 240 Ω (OUT–ADJ), R2 720 Ω (ADJ–GND): 5000 mV.
- (p) RFX-FB with only Rtop, or a third resistor from FB to the 5V rail: OUT
  has no level.
- (q) `GENERIC-RESISTOR` on both sides with `circuit_set_resistance` 10000
  and 10000: 1600 mV; without the calls (1000 / 1000): 1600 mV; after
  setting the top to 30000: 3200 mV. `circuit_set_resistance` on a maker
  resistor → `part N is not a generic resistor`; 0 Ω → `the resistance is
  out of range`.
- (r) RFX-BOOST from 3V3: OUT 5000 mV; from 0.5 V (a fixture supply)
  nothing.
- (s) EN of RFX-LDO33 tied to GND: OUT has no level; RFX-FB's active-low
  EN tied to 5V: no level.
- (t) Chain: 12V → RFX-LDO5 → RFX-LDO33 → a 74HC00: powered at 3.3 V.
- (u) RFX-GEN from 12V (no `vinMax`): no input problem, OUT 5000 mV.
- (v) Two RFX-LDO33 outputs on one net: no clash; RFX-LDO33 and RFX-LDO5
  outputs on one net: one `VOLTAGE_SUPPLY_CLASH`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind, its facts and the model

- [ ] `parts.h`: `PART_REGULATOR` after `PART_TRANSISTOR`, `PartRegulator`,
      `Part.regulator` — only if absent (`grep -c 'PART_REGULATOR'
      backend/include/parts.h` → 0 first; a hit from another plan marks
      this step moot with that output).
- [ ] `models/regulator.c`: `REGULATOR_KIND` (§ 3.1 keys, roles, errors),
      `pin_open_drain` (the `pg` pins), `pin_analog` (the `"analog"` list),
      `REGULATOR_MODEL` (§ 3.2); `model_pins.h` helpers; one line in
      `KINDS[]` (`kinds.c`); the file in `backend/CMakeLists.txt`'s library
      sources.
- [ ] Tests: `tests/support/listing.c` (the kind name and its facts line:
      type, count, mixed, adjustable, vout/vref lists, the scalars),
      `TRACE_LAST_KIND` → `PART_REGULATOR` (`trace_check.c:22`), the two
      arrays of `listing_check.c:50–51` → `PART_REGULATOR + 1`;
      `tests/support/regulator_fixture.h` (§ 4); `tests/regulator_test.c`
      cases (a)–(i); `logic_test(regulator_test)` in `CMakeLists.txt`.

Acceptance: the kind loads, refuses bad keys and switches its output.
  Check: `cmake --build /tmp/logic-p42 -j8 && ctest --test-dir /tmp/logic-p42 -R 'regulator_test|listing_check|trace_check|library_check' --output-on-failure` → 4 of 4 passed, goldens unchanged (est. 3 min).
Commit: —

### Phase 2 — The voltage check and the circuit API

- [ ] `voltage.h` / `voltage.c`: the `milliohms` argument, `regulated[]`,
      `effective()` with R, the fixed-point branch, `divider_mv`, the
      clashes, `VOLTAGE_REGULATOR_INPUT_OVER` and its `voltage_describe`
      text (§ 3.3).
- [ ] `circuit.h` / `circuit.c`: `circuit_set_resistance`, the `milliohms`
      array, `voltage_failed` passes it (§ 3.4).
- [ ] The four test call sites of `voltage_check` pass `NULL`
      (`voltage_test.c:92, 496`, `charlcd_test.c:1639, 1659`);
      `VoltageCheck.swift:81, 87` pass `nil`.
- [ ] Tests: `regulator_test.c` cases (j)–(v); `voltage_test.c` one case:
      no regulator, the new argument NULL and an all-zero array give the same
      problems as before for its existing fixture circuits.

Acceptance: the regulated level, the divider and the VIN rating behave as
§ 6, and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p42 -R 'regulator_test|voltage_test|circuit_check' --output-on-failure` → 3 of 3 (est. 2 min); `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p42-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 4 min; the Swift call sites changed).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` § `regulator`, `docs/backend.md` (Regulators,
      Voltages, the circuit call), `docs/file-format.md:133` (§ 5).

Acceptance: the docs name every key, error and rule.
  Check: `grep -c 'adjustTo\|enableActive\|vinMax\|REGULATOR_INPUT_OVER\|circuit_set_resistance' docs/parts-format.md docs/backend.md` → at least 1 per file for its names (est. < 1 min).
Commit: —

## Validation Plan

- Tests: `regulator_test` (§ 6 a–v, including every refusal), the
  `voltage_test` NULL-argument case, `listing_check` / `trace_check` (the
  goldens, unchanged).
- Coverage check: `regulator_test` builds the fixture circuits through
  `circuit_start`, so the new `voltage.c` branches and `circuit.c` path are
  in its run (cases j–v each fail if their branch is removed).
- Runtime proof: plan-42-I's harness (an LDO lighting a 3.3 V part in the
  app).
- Doc sync: § 5.
- Final gate: plan-42-J.

## Decisions

Owner decisions (2026-11-15, plan-42-A § Decisions): the regulated-supply
model; the VIN rating check (above the maximum refused, below the headroom
unpowered); the adjustable output from the divider resistors; dual and
multi-output parts.

Author decisions:

- **A separate regulated level R between S and D** (§ 3.5).
- **A floating enable is off**, for both polarities; an enable driven by
  logic counts as active in the static check (it cannot know the logic
  value).
- **`circuit_set_resistance` for generic resistors only**: maker resistors
  carry their value in the part.
- **The VIN rating also covers the enable pin** (the datasheets rate EN to
  VIN's maximum).
- **One clash per group** (§ Verified properties).

## Corrections

## Summary

The risk is in `voltage.c`: a new level class in `effective()` touches
every check. It is empty for every circuit without a regulator, and the
goldens plus `voltage_test`'s unchanged results prove that. The divider is
deliberately narrow (two resistor sides, else unknown) so it never invents
a level.
