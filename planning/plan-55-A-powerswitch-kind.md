# plan-55-A: Backend — the `powerswitch` kind, the switched level, the input rating

Last updated: 2026-11-22
Overall Effort: huge (> 3d) — plan-55 is A → B → C → D → E → F → G → H → I → J → K
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-55 adds **power switches** to the parts library and the app, from the top three
makers of each family, **Active parts only**, and **no generic part** (owner, § Decisions):

- **integrated ideal diodes** (DigiKey "OR Controllers, Ideal Diodes", category 758, filtered to
  parts with an internal switch: the LM66100 and LM66200 style) from **Texas Instruments**,
  **Analog Devices** and **Monolithic Power Systems**;
- **load switches** (DigiKey "Power Distribution Switches, Load Drivers", category 726: a
  high-side one- or two-channel switch with no current-limit protection) from **Texas Instruments**,
  **onsemi** and **Microchip Technology**;
- **current-limited and USB power switches** (category 726: a high-side one- to four-channel
  switch with current limiting, with a fault flag) from **Texas Instruments**,
  **Microchip Technology** and **Diodes Incorporated**.

They sit in the parts drawer under **Power → Power Switches → Load Switches / Current-Limited /
Ideal Diodes**, in a new kind, `powerswitch`. In the simulation a switch is a gate for a
supply: while its input pin reads 1 and its enable (when it has one) is active it drives its
output pin strong 1 (an ideal diode blocks the other way by driving nothing); in the voltage check
its output carries the input's level, two switch outputs on one net read the higher level
(ORing), and an input above the part's maximum is refused. External-FET OR-ing controllers,
power muxes with select logic, eFuses and hot-swap controllers, relay and multi-channel drivers
and switches controlled over I²C or SPI are out of scope (owner, § Decisions).

The single behavioral outcome of plan-55: loading the part files gives the baseline P0 parts
(§ Prerequisites) plus one `powerswitch` part per `add` row of
`planning/plan-55-data/candidates.tsv` (1037 candidates at most, before the verdict letters'
skips: ideal diodes 90, load switches 241, current-limited 706, § 2); the app lists them under the
drawer paths above; a placed part has the pins and pads of its datasheet; with its input at 5 V and
its enable active its output reads 1 and 5 V in play mode and in the voltage check; a part wired
with its input above its maximum is refused in the voltage check.

| Letter | Content | Effort |
|---|---|---|
| **A** | backend: the `powerswitch` kind, its model, the switched level, the input rating, fixtures, tests, docs | large |
| B | data: the listing, stock re-check, datasheet downloads and placement, text scan, `specs.tsv` skeleton | large (plus two listings of at least 30 DigiKey calls each) |
| C | verdicts: ideal diodes and load switches (331 candidates at most) | large |
| D | verdicts: current-limited and USB switches, Texas Instruments (366) | large |
| E | verdicts: current-limited and USB switches, Microchip and Diodes (340) | large |
| F | footprints: Texas Instruments packages (68 DigiKey package labels at most) | large |
| G | footprints: Microchip, Diodes, onsemi, Analog Devices and Monolithic Power Systems packages (69 labels at most) | large |
| H | parts: generator, ideal diodes and load switches, datasheets, goldens, the Part Editor's kind lists | large |
| I | parts: current-limited and USB switches, datasheets, goldens | large |
| J | app: details, facts, kind name, block symbol, the harness | medium |
| K | landing tier | small |

Outcome of A: `powerswitch_test` passes § 6, `ctest` passes T0 + 1 programs, every golden is
byte-identical (no part uses the kind yet; H adds them), the app still builds, the docs describe the
kind.

References:

- `AGENTS.md` (§ Datasheets); `docs/backend.md` § Adding a kind, § Voltages, § Tests;
  `docs/parts-format.md` § Kinds, § Devices, § Parts, § Values, § `regulator`.
- `backend/src/models/regulator.c` (the kind this one copies: loader, bind, model, ops),
  `backend/src/voltage.c` (§ 2), `backend/include/voltage.h`, `backend/src/models/model_pins.h`,
  `backend/src/library/kind.h`, `backend/src/library/kinds.c`, `backend/include/parts.h`.
- `planning/completed/plan-42-G-regulator-kind.md` (the precedent for every step of A) and
  `planning/completed/plan-42-H-regulator-parts.md` (the Part Editor's kind lists: H does them here).
- `planning/plan-55-data/` — `README.md`, `census/families.js`, `families.out` (the planning
  census, 2026-11-22).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p55 && cmake --build /tmp/logic-p55 -j8 && ctest --test-dir /tmp/logic-p55 -j8` → `100% tests passed` | MET (2026-11-23 at e5242c08: `100% tests passed, 0 tests failed out of 42`; T0 = 42 programs, `grep -c '^logic_test' backend/CMakeLists.txt` → 42) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | at planning (7c7b4d31): **P0 = 34375**, **R0 = 34375**, **F0 = 1895**; at e5242c08 (2026-11-23): **P0 = 34436**, **R0 = 34436**, **F0 = 1904**; H, I, F and G re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p55-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-23 at e5242c08: `** BUILD SUCCEEDED **`) |
| The regulator kind this one copies has landed | `ls planning/completed/plan-42-J-*.md` → exists; `grep -c 'REGULATOR_KIND' backend/src/library/kinds.c` → 2 | MET (2026-11-22: plan-42-J is archived; `REGULATOR_KIND` 2 lines in `kinds.c`) |
| No `powerswitch` kind yet | `grep -c 'PART_POWERSWITCH' backend/include/parts.h` → 0; `ls schematical/schematical/Parts \| grep -c powerswitch` → 0; `ls datasheets \| grep -c powerswitch` → 0 | MET (2026-11-22 at 7c7b4d31: 0, 0, 0) |
| No other open plan adds these parts | `grep -il 'ideal diode\|load switch\|power switch\|power distribution' planning/plan-*.md \| grep -v plan-55 \| wc -l` → 0 | MET (2026-11-22: 0) |
| DigiKey credentials and quota (B only) | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --limit 1 "LM66100DCKT"` prints `left` ≥ 40 before each of B's two listings (1000 calls a day, shared with other sessions; it resets at about 00:00 UTC). **Owner (2026-11-23): run plan-55 with `--opt2`** (the second key, `DIGIKEY_CLIENT_ID2` / `DIGIKEY_CLIENT_SECRET2`, its own quota; `grep -cE '^export DIGIKEY_CLIENT_(ID2\|SECRET2)=' ~/.zshrc` → 2) | MET (2026-11-23: `node scripts/digikey.js --opt2 --limit 1 "LM66100DCKT"` → `750 left today` with key 2; key 1 was already spent by other sessions). A's phases need no quota |

Everything below is written against the world where these hold; only the quota row is open at
planning time, and it gates B alone.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you
> decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only the one that
> blocked you.

Other open plans also add kinds (plan-47, plan-48, plan-51 are written and not built) and edit the
same one-line spots as A: the last `PartKind`, `TRACE_LAST_KIND`, the two arrays of
`listing_check.c`, `KINDS[]`, `PartKinds.all`, `PartKindRoundTripTests`. Those lines are edited by
hunk: every A step that adds one greps first (`grep -n 'PART_' backend/include/parts.h`) and appends
after the last value present. That is one rule, not a dependency: A does not wait for those plans.

## 1. Goal

- `powerswitch_test` passes the cases of § 6 (T0 + 1 programs: 43 with T0 = 42).
- `ctest --test-dir /tmp/logic-p55` → `100% tests passed`.
- `backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` are byte-identical (no part
  uses the kind yet).
- The app builds (`VoltageCheck.swift` and `PartCatalog.swift` have `default:` branches for the new
  problem kind and part kind; J gives them cases).
- `docs/parts-format.md` has a `### powerswitch` section, `docs/backend.md` a **Power switches**
  model entry and the switched level in **Voltages**.

### Non-goals (explicit constraints)

- No analog behaviour in the simulation: no ramp or slew, no on-resistance drop, no current limit
  or overcurrent, no thermal shutdown, no quick output discharge, no reverse-current measurement.
  The output is a digital 1 while the element is on; the facts below are for the app and the
  voltage check.
- No new argument or call on `voltage_check`, no change to `circuit.h`: the three outside callers
  of `voltage_check` and the five test call sites stay as they are.
- Every circuit without a power switch resolves and checks exactly as before: the new level class
  is empty.
- No part is added in A (H and I add them); no footprint (F and G).
- No external-FET controller, no power mux with select logic, no eFuse or hot-swap controller, no
  relay or multi-channel driver, no switch with an I²C, SPI or PWM control (owner, § Decisions).
- No part-editor form beyond what H adds from the sources (§ 3.6).

## 2. Current State

- `regulator.c` (693 lines) is the whole shape A copies: `DEVICE_KEYS` / `PART_KEYS`,
  `regulator_load_device` (the element list, key checks, `kind_error` messages),
  `regulator_bind_pinout` (names to pin indices, roles), `regulator_load_part` (the part keys),
  `regulator_pin_open_drain` / `regulator_pin_analog`, `REGULATOR_MODEL` (a 1 ns timer per element
  that re-reads the pins: `regulator_timer`, lines 633–677), `REGULATOR_KIND` (line 689).
- `voltage.c` (888 lines): `effective()` (line 109) is S, else R (regulated), else D, else P, else C;
  the fixed point has a `PART_REGULATOR` branch (lines 519–617) that reads the element's input
  level (the highest known effective level over its input pins), skips an inactive enable, raises
  `regulated[g]` on every output pin's group (`level_raise`) and records the lowest and highest
  level's part per group (`reg_low_*`, `reg_high_part`); step 3c (lines 696–727) reports one
  `VOLTAGE_SUPPLY_CLASH` per group; step 4 reports `VOLTAGE_REGULATOR_INPUT_OVER` (lines 809–815)
  for an input or enable pin above `regulator_input_max_mv`. A part with `vcc == NULL` is skipped by
  the generic driving branch (`voltage.c:613`, `if (part->vcc == NULL) continue;`, after the regulator branch).
- `PartKind` ends with `PART_CRYSTAL` (`parts.h:30`), `TRACE_LAST_KIND` is `PART_CRYSTAL`
  (`trace_check.c:22`), `KINDS[]` ends `&REGULATOR_KIND, &CRYSTAL_KIND` (`kinds.c:79–80`),
  `VoltageProblemKind` ends with `VOLTAGE_REGULATOR_INPUT_OVER` (`voltage.h:38`).
- The Swift side reads a kind through `default:` branches: `PartCatalog.swift:1469` (`default:
  return .block`), `PartFacts.swift:293` (`default: return ""`), `VoltageCheck.swift:164` (`default:`
  "is rated … V"). A new `PartKind` and a new `VoltageProblemKind` build without a Swift change.
- Datasheets read for this plan (2026-11-22, `https://www.ti.com/lit/ds/symlink/<name>.pdf`):
  **LM66100** (VIN 1, GND 2, CE 3, NC 4, ST 5, VOUT 6): CE is "active-low chip enable. Can be
  connected to VOUT for reverse current protection": the enable compares CE with VIN (CE higher than
  VIN by 80 mV: off; lower by 250 mV: on); ST is "active-low open-drain output, pulled low when the
  chip is disabled, Hi-Z when the chip is enabled"; VIN 1.5–5.5 V, absolute maximum 6 V, 1.5 A.
  **LM66200** (GND 1 5, VOUT 2 7, VIN1 3, ON 4, VIN2 6, ST 8): two ideal diodes into one output; ON is
  active low for both channels and has a 1 V reference (a logic level); ST is a status that is high
  while VIN1 powers the output and low while VIN2 does (push-pull, not a fault). **TPS22918** (VIN 1,
  GND 2, ON 3, CT 4, QOD 5, VOUT 6): ON active high ("do not leave floating"), CT is the slew
  capacitor, QOD the output discharge. **TPS2051B** (SOT-23: OUT 1, GND 2, OC 3, EN 4, IN 5; SOIC:
  IN 2 3, OUT 6 7 8): EN active high, OC "overcurrent open-drain output, active-low"; the TPS2041B
  is the active-low EN variant; the TPS2052B has EN1 / EN2, OUT1 / OUT2, OC1 / OC2 and one IN; the
  TPS2053B has IN1 (OUT1, OUT2), IN2 (OUT3), EN1–EN3 and OC1–OC3; the TPS2054B has IN1 (OUT1, OUT2), IN2
  (OUT3, OUT4), EN1–EN4 and OC1–OC4.

### Measured populations

All DigiKey numbers 2026-11-22, in stock and status **Active** only. Planning spent 14 census
calls (`families.js`); the listing is B's.

| What | Count | Command |
|---|---|---|
| Ideal diodes with an internal switch (category 758, `Internal Switch(s)` = Yes) | **101**; Texas Instruments 44, Analog Devices 31 + Maxim 6 = 37, Monolithic Power Systems 9, Richtek 4, Nexperia 3 | `node planning/plan-55-data/census/families.js` → the `== ideal` block (`planning/plan-55-data/families.out`) |
| Load switches (726; `Switch Type` Load Switch or Power Switch, `Output Configuration` High Side, `Number of Outputs` 1 or 2, `Fault Protection` without "Current Limit") | **442**; Texas Instruments 140, onsemi 35 + Fairchild 24 = 59, Microchip 36 + Micrel 6 = 42, Toshiba 29, Infineon 27, Diodes 26, Vishay 26 | same, the `== load` block |
| Current-limited and USB switches (726; `Switch Type` Load Switch, Power Switch or USB Switch, High Side, 1 to 4 outputs, `Fault Protection` with "Current Limit") | **1346**; Texas Instruments 363 + National 3 = 366, Microchip 156 + Micrel 17 = 173, Diodes 167, Infineon 130, STMicroelectronics 106, Rohm 88 | same, the `== limited` block |
| The nine (family, maker) pieces | ideal 44 / 37 / 9; load 140 / 59 / 42; current-limited 366 / 173 / 167 = **1037** (ideal 90, load 241, current-limited 706) | same, the `pick` lines |
| Distinct DigiKey `Package / Case` labels per piece | ideal 9 / 13 / 2; load 22 / 11 / 9; current-limited 37 / 17 / 17 = **137**; Texas Instruments 68, Microchip 26, Diodes 17, Analog Devices 13, onsemi 11, Monolithic Power Systems 2 | same, `packages(case)` |
| Backend test programs T0 | 42 | `grep -c '^logic_test' backend/CMakeLists.txt` → 42 (e5242c08; 41 at 7c7b4d31) |
| Golden sizes P0 / R0 / F0 | 34375 / 34375 / 1895 | `wc -l < backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` (7c7b4d31) |
| Files that name the regulator kind (the touch points of a kind) | 10 files | `grep -rIl 'PART_REGULATOR\|\.regulator\b' schematical/schematical schematical/schematicalTests backend/src backend/include backend/tests scripts` (7c7b4d31) |

### Verified properties

- **A power switch needs no `vcc`.** Read `voltage.c`: a part with `vcc == NULL` is skipped by the
  generic driving branch and the generic range and input checks, as `regulator.c`'s parts are; the
  new branch is the only code that touches a `PART_POWERSWITCH` part.
- **The model is the regulator's state machine.** Read `regulator_timer`: input X → output X;
  input not 1 → output Z; enable X → output X and flag X; enable inactive (including Z) → output Z
  and flag 0; else output strong 1; every drive is `sim_schedule_drive(…, DRIVE_STRONG, 0)` from a
  timer, so a change is always a later event than the read. § 3.3 adds one enable mode to it.
- **The problem buffer bound holds.** `found` has `connection_count + net_count + 1` slots
  (`voltage.c:250`); A reports at most one clash per group, so the
  bound holds.
- **A single pin name may serve several elements and several pins.** `regulator_bind_pinout`
  binds a name to every pin of the name (a VIN pair, a tab); the TPS2053B's `IN1` serves OUT1 and OUT2.
  `powerswitch` binds names the same way.

## 3. Design

### 3.1 The kind

`backend/src/models/powerswitch.c`, `POWERSWITCH_KIND` (`"powerswitch"`), file
`Parts/powerswitch.json`. `PART_POWERSWITCH` is appended after the last `PartKind` present.

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"load"`, `"limited"` or `"ideal-diode"` (`unknown power switch type`); every element of the package has it |
| `"switches"` | array of 1 to 4 objects (`"switches" must have 1 to 4 entries`), one per switch in the datasheet's numbering |
| `"analog"` | optional array of pin names the voltage check skips and the model never drives: slew and soft-start pins (CT, SS), quick-output-discharge pins (QOD), current-limit set pins (ILIM, SET), bias supplies (VBIAS), mode straps, a status output that is not modelled (the LM66200's ST) |

Each `"switches"` entry (`"in"` and `"out"` required, no other key beyond these):

| Key | Value |
|---|---|
| `"in"` | the input pin name: every pin of the name is the input; role `power` |
| `"out"` | the output pin name: every pin of the name is driven; role `output`; two elements may share one name (the LM66200's VOUT) |
| `"enable"` | optional: the enable pin name; role `input` |
| `"enableActive"` | optional, with `"enable"`: `"high"` (default) or `"low"` |
| `"enableRef"` | optional, with `"enable"`: `"logic"` (default: the pin is a logic input) or `"input"` (the pin is a comparator against the input, the LM66100's CE; needs `"enableActive": "low"`) |
| `"flag"` | optional: a status or fault output pin name, open drain; role `output` |
| `"flagWhen"` | optional, with `"flag"`: `"fault"` (default: the part reports an overcurrent or over-temperature fault, which the model never has, so the pin is never pulled low) or `"disabled"` (pulled low while the element is disabled, the LM66100's ST) |

Every other pin of a pinout is `ground`, `nc`, `absent`, or named in `"analog"`
(`pin "X" is in no switch and not analog`); a name an entry uses names at least one pin of every
pinout, and its pins have the roles above (`power switch pin "X" (in) must be power`). An exposed
pad or a tab with no number is not a pin. A package with no enable pin has no `"enable"`: the
element is on whenever its input reads 1.

Part keys (volts, amps, ohms; one number for the part):

| Key | Value |
|---|---|
| `"vinMin"` | optional, above 0 and at most 100: the lowest input at which a switch conducts (the undervoltage lockout); below it the voltage check gives the output no level |
| `"vinMax"` | optional, above 0 and at most 100 and at least `vinMin`: the input maximum, the datasheet's recommended operating maximum (the absolute maximum when it gives none); the voltage check refuses an input or enable pin above it |
| `"iout"` | optional, above 0 and at most 100: the continuous current per switch |
| `"ilimit"` | optional, above 0 and at most 100, for `"type": "limited"` only (`"ilimit" is only for a current-limited switch`): the fixed current limit |
| `"ron"` | optional, above 0 and at most 10: the on-resistance, ohms |
| `"iq"` | optional, above 0 and at most 1: the quiescent current |

The facts for the app (`parts.h`, after the last info struct): millivolts, milliamps, milliohms,
microamps, 0 = not given.

```c
typedef struct { const char *type; size_t switch_count;
                 uint32_t vin_min_mv, vin_max_mv, iout_ma, ilimit_ma, ron_mohm, iq_ua; } PartPowerSwitch;
```

`const PartPowerSwitch *powerswitch;` on `Part`. `model_pins.h` gains `powerswitch_element(part, k,
&in, &out, &enable, &enable_low, &enable_ref_input, &flag, &flag_disabled)` (pin indices of the
first pin of each name, `SIZE_MAX` when absent; 0 past the last element), `powerswitch_pin_in(part,
k, pin)`, `powerswitch_pin_out(part, k, pin)`, `powerswitch_input_max_mv(part)`.

### 3.2 The part-file conventions (H and I follow them)

- **Device id:** `psw-<maker>-<series>-<package>` in lower case, the regulator's `reg-ti-lm1117-sot223`
  shape (`psw-ti-lm66100-sc70`). One device per (series, package, pin assignment).
- **`"function"`:** `Single channel`, `Dual channel`, `Triple channel`, `Quad channel` by `"switches"`
  count for `load` and `limited`; `Single input`, `Dual input` for `ideal-diode`.
- **`"drawer"`:** `["Power", "Power Switches", "Load Switches"]` for `load`, `["Power", "Power
  Switches", "Current-Limited"]` for `limited`, `["Power", "Power Switches", "Ideal Diodes"]` for
  `ideal-diode`.
- **Part `"name"`** is the maker's orderable part number as DigiKey prints it (`LM66100DCKR`);
  `"manufacturer"` is `Texas Instruments`, `Analog Devices`, `Microchip Technology`, `onsemi`,
  `Diodes Incorporated` or `Monolithic Power Systems` (a company's DigiKey labels together).
- **Pins** carry the datasheet's names; several pins of one name are one node; an option-dependent
  pin (`EN` active high or low by part) is two devices.

### 3.3 The model

`SimComponentType POWERSWITCH_MODEL` is `REGULATOR_MODEL` with the element list of § 3.1: `create`
returns one state byte per element, `start` schedules every element's timer after
`POWERSWITCH_DELAY_PS` = 1000 (1 ns), `pin_changed` on an element's input pin or enable pin
schedules its timer. `timer(k)` reads the input as `read_input` does (1 if any pin of the name
reads 1 and none X; X if any X; 0 if one reads 0; else Z) and the enable pin, then sets one of five
states and drives `out` and `flag` strong (`sim_schedule_drive(…, DRIVE_STRONG, 0)`):

| State | Input | Enable | `out` | `flag` (`"disabled"`) | `flag` (`"fault"`) |
|---|---|---|---|---|---|
| on | 1 | no pin; active (logic); anything but X (`"input"`) | 1 | Z | Z |
| off | 1 | inactive or Z (logic) | Z | 0 | Z |
| off | 0 or Z | reads 1 (`"input"`) | Z | 0 | Z |
| unknown | X | any | X | Z | Z |
| unknown | 1 | X | X | X | Z |
| none | 0 or Z | anything else | Z | Z | Z |

A floating enable is inactive for a logic enable, for both polarities. An `"input"` enable is a
comparator against the input and a logic simulator sees only 0, 1 and Z: a 1 on it is "not above"
an input that reads 1, so with the input at 1 it never turns the element off (the LM66100 with CE
on VOUT conducts); a 1 on it with the input at 0 or Z is "above", the element is off and a
`"disabled"` flag is low (the battery-backup case: VIN absent, VOUT held up by another source, ST
low). Every other case with the input at 0 or Z gives Z. The one-nanosecond delay means a pulse
shorter than that never moves the output.

### 3.4 The voltage check

A new level class **W, the switched level**, per group, beside R. `effective()` becomes S, else R,
else W, else D, else P, else C.

1. **Fixed point, a `PART_POWERSWITCH` branch** (before `vcc == NULL`): for each element k, `vin` =
   the highest known effective level over the element's input pins; skip when unknown or 0; skip
   when `vin < vin_min_mv`; enable: with `"enableRef": "logic"` skip when the enable pin's group has
   a known effective level that is inactive (active high: 0 mV; active low: above 0 mV), an unknown
   level counts as active; with `"input"` skip when the enable pin's group has a known level above
   `vin`. Then raise `switched[g]` on every output pin's group to `vin` (`level_raise`), record the
   part, pin and net of the highest level per group, and set `changed` as the other branches do.
   Levels only rise, so the loop ends; an enable on the element's own output net (`"input"`)
   reads at most `vin`, so it never switches the element off.
2. **Clashes** (step 3c, after the regulated ones, still one per group): a group with W known: a
   supply level S known, non-zero and different from the highest W → `VOLTAGE_SUPPLY_CLASH` (`part`
   the supply, `other_part` the switch, `mv` the higher, `min_mv = max_mv` the lower); else a
   regulated level R different from the highest W → the same kind (`part` the part with the lower
   level). **Two switch outputs on one group never clash**: the group reads the higher level
   (ORing, owner). A supply level of 0 on a switch output (ground on it) is the engine's short, not
   a voltage problem, as for a regulator.
3. **The input rating.** For a `PART_POWERSWITCH` part with `vin_max_mv > 0`, an input or enable
   pin whose effective level is above it → `VOLTAGE_POWERSWITCH_INPUT_OVER` (appended after the
   last `VoltageProblemKind` present; `min_mv` 0, `max_mv` the rating). `voltage_describe`:
   `voltage: 12 V on pin 1 (VIN) of LM66100DCKR; its input max is 5.5 V`.

### 3.5 Where the risk is

In `voltage.c`: a new level class in `effective()` touches every check. It is empty for every
circuit without a power switch, which the goldens and `voltage_test`'s unchanged results prove;
the clash rules are the part that can invent a problem, so § 6 pins each one (cases (o)–(r)).
The model and the loader copy a tested kind.

### 3.6 Rejected alternatives

- **A new `type` of `regulator`.** The regulator's output level is a part fact (`vout`); a switch
  passes its input's level, two of them must not clash, and an ideal diode has no `vout` or
  dropout. Bending `regulated[]` to cover that puts an exception in every regulator rule.
- **The switched level as the regulated level R.** R clashes with any different R; ORing needs the
  higher of two switch levels to win. A separate W keeps both rules simple.
- **Per-element `type`.** No part mixes a load switch and an ideal diode in one package; one device
  key is simpler for the drawer and the facts.
- **Modelling the LM66200's ST.** It reports which input powers the output (a comparison the
  logic simulator cannot make); the pin goes in `"analog"` and reads Z.
- **A dropout or an on-resistance drop in the level.** A pass element's drop is millivolts at the
  rated current; the voltage check's levels are supply levels, not load-dependent ones.
- **The Part Editor form hand-written.** The schema is generated from `powerswitch.c` and the
  shipped parts (H, `scripts/part-editor-schema.py`), as every kind with shipped parts.

## 4. The fixture parts

`tests/support/powerswitch_fixture.h` (as `regulator_fixture.h`): a JSON string of fixture devices
loaded next to the real part files, names `PFX-…`:

| Part | Type, elements | Keys |
|---|---|---|
| `PFX-LOAD` | load, one: VIN / GND / ON (high) / VOUT, SOT-23-5 pins | `vinMin` 1, `vinMax` 5.5 |
| `PFX-ILIM` | limited, one: IN / GND / EN (high) / OUT / FLG (`"fault"`) | `vinMax` 6, `ilimit` 0.5 |
| `PFX-LOW` | limited, one: IN / GND / EN (low) / OUT / FLG | `vinMax` 6 |
| `PFX-DIODE` | ideal-diode, one: VIN / GND / CE (low, `"input"`) / ST (`"disabled"`) / VOUT | `vinMin` 1.5, `vinMax` 5.5 |
| `PFX-DUAL` | ideal-diode, two: VIN1 and VIN2 into one VOUT, shared ON (low, `"logic"`), ST in `"analog"` | `vinMax` 5.5 |
| `PFX-QUAD` | limited, four: IN1 (OUT1, OUT2), IN2 (OUT3, OUT4), EN1–EN4 (high), OC1–OC4 (`"fault"`) | `vinMax` 6 |
| `PFX-FREE` | load, one with no enable: IN / GND / OUT | none |

## 5. Docs

- `docs/parts-format.md`: the kinds table gains `powerswitch.json`; a `### powerswitch` section
  after `### regulator` (device keys, element keys, roles, part keys, the model table, the
  voltage rules, the conventions of § 3.2 as the drawer paths).
- `docs/backend.md`: a **Power switches** bullet beside **Regulators** (the model table of § 3.3),
  the switched level, the ORing rule and the input rating in **Voltages**; the kind's name in the
  list of kinds that **Adding a kind** names.

## 6. Test cases (`backend/tests/powerswitch_test.c`)

Kind cases (fixture parts; `load_parts` of a bad file for errors):

- (a) Each fixture loads; `PartPowerSwitch` facts read back (type, count, the scalar keys).
- (b) Refusals, one per rule: `"switches"` empty or 5 entries; an unknown `type`; `"ilimit"` on a
  `load` part; `"enableActive"`, `"enableRef"` or `"flagWhen"` without its pin key; `"enableRef":
  "input"` with `"enableActive": "high"`; an `in` pin that is not `power`; a pin in no switch and
  not analog; `vinMax` below `vinMin`; a name that names no pin of a pinout.

Engine cases (one fixture part in a rig with a scripted driver):

- (c) `PFX-LOAD`, VIN 1, ON 1: VOUT reads 1 after 1 ns; ON 0: Z; ON Z: Z.
- (d) VIN 0, then Z: VOUT Z.
- (e) VIN X: VOUT X; VIN 1, ON X: VOUT X.
- (f) A 0.5 ns pulse on ON never moves VOUT.
- (g) `PFX-ILIM` with a pull-up on FLG: FLG reads 1 at every input and enable value.
- (h) `PFX-LOW`: EN 0 → OUT 1, EN 1 → OUT Z.
- (i) `PFX-DIODE` with CE on VOUT: VIN 1 → VOUT 1 and stays 1 (no oscillation); VIN 0 with VOUT held 1
  by a strong driver → ST 0, VOUT not driven; VIN 0 with VOUT 0 → ST Z.
- (j) `PFX-DUAL`: VIN1 1, VIN2 0 → VOUT 1; both 1 → 1; both 0 → Z; ON 1 → Z.
- (k) `PFX-QUAD`: EN1 1, EN2 0 → OUT1 1, OUT2 Z; IN2 0 → OUT3 and OUT4 Z with OUT1 still 1.
- (l) `PFX-FREE` with VIN 1: VOUT 1.
- (m) `PFX-LOAD`'s VOUT on the same net as GND → `smoke:` (the engine's short).

Voltage cases (`circuit_start` / `voltage_check` on fixture parts and real supplies; a level is
read through the clash it causes with a 1.2 V supply on the same net, as `regulator_test`'s
`level_through_clash`):

- (n) 5V → `PFX-LOAD` VIN, ON to VIN: the output net is 5000 mV; a 74HC00 (2–6 V) on it: no
  problem; VIN at 3V3: 3300 mV.
- (o) ORing: two `PFX-DIODE` with VIN at 5V and at 3V3 and both VOUT on one net: no problem, the
  net is 5000 mV; the same with `PFX-LOAD` and `PFX-DIODE`.
- (p) `PFX-LOAD` VIN at 5V with VOUT on the 3V3 supply: one `VOLTAGE_SUPPLY_CLASH` (5000 vs 3300),
  text `voltage: 3.3 V and 5 V supplies on one net (3V3, PFX-LOAD)`; VOUT on the 5V supply: no
  problem.
- (q) `PFX-LOAD` VIN at 5V and `RFX-LDO33` (from `regulator_fixture.h`) output on one net:
  one `VOLTAGE_SUPPLY_CLASH`.
- (r) Chain: 12V → `RFX-LDO5` → `PFX-LOAD` → a 74HC00: powered at 5 V; `RFX-LDO33` → `PFX-DIODE` →
  `PFX-LOAD` → `SN74AUC00RGYR` (0.8–2.7 V): no problem.
- (s) `PFX-LOAD` with VIN at 1.2 V (the library's lowest supply; `vinMin` 1: the output is 1200 mV) and at 0.5 V (`SFX-0V5`, the fixture
  supply of `regulator_fixture.h`): no level; ON tied to GND: no level.
- (t) `PFX-DIODE` with CE on a 5V supply and VIN at 3V3: no level; CE on its own VOUT: 3300 mV.
- (u) 12V on `PFX-LOAD` VIN → `VOLTAGE_POWERSWITCH_INPUT_OVER`, `max_mv` 5500, the § 3.4 text; ON on 12V
  likewise; `PFX-FREE` (no `vinMax`) from 12V: no input problem, 12000 mV.
- (v) `voltage_test`: no power switch placed, the problems of its existing circuits are unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The kind, its facts and the model

- [x] `parts.h`: `PART_POWERSWITCH` after the last `PartKind` present (`grep -n 'PART_'
      backend/include/parts.h`), `PartPowerSwitch`, `Part.powerswitch`.
- [x] `models/powerswitch.c`: `POWERSWITCH_KIND` (§ 3.1 keys, roles, errors), `pin_open_drain` (the
      `flag` pins), `pin_analog` (the `"analog"` list), `POWERSWITCH_MODEL` (§ 3.3); the
      `model_pins.h` helpers; one line in `KINDS[]` (`kinds.c`); the file in `backend/CMakeLists.txt`'s
      library sources.
- [x] Tests: `tests/support/listing.c` (the kind name and its facts line), `TRACE_LAST_KIND` →
      `PART_POWERSWITCH` (`trace_check.c:22`), the two arrays of `listing_check.c` →
      `PART_POWERSWITCH + 1`; `tests/support/powerswitch_fixture.h` (§ 4);
      `tests/powerswitch_test.c` cases (a)–(m); `logic_test(powerswitch_test)` in `CMakeLists.txt`.
      Observed: `ctest -R 'powerswitch_test|listing_check|trace_check|library_check'` → 4 of 4 passed; `git diff --stat backend/tests/golden` empty.

Acceptance: the kind loads, refuses bad keys and switches its output.
  Check: `cmake --build /tmp/logic-p55 -j8 && ctest --test-dir /tmp/logic-p55 -R 'powerswitch_test|listing_check|trace_check|library_check' --output-on-failure` → 4 of 4 passed, `git diff --stat backend/tests/golden` empty (est. 3 min).
Commit: b7215970

### Phase 2 — The switched level, the ORing rule and the input rating

- [x] `voltage.h` / `voltage.c`: `switched[]`, `effective()` with W, the fixed-point branch, the
      clashes, `VOLTAGE_POWERSWITCH_INPUT_OVER` and its `voltage_describe` text (§ 3.4).
- [x] Tests: `powerswitch_test.c` cases (n)–(u); `voltage_test.c` case (v).
      Observed: `ctest -R 'powerswitch_test|voltage_test|regulator_test|circuit_check'` → `100% tests passed, 0 tests failed out of 4` (powerswitch_test prints 20 PASS lines for (n)–(u); voltage_test prints 4 `(v)` PASS lines); `xcodebuild … build` → `** BUILD SUCCEEDED **`.

Acceptance: the switched level, ORing and the input rating behave as § 6, and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p55 -R 'powerswitch_test|voltage_test|regulator_test|circuit_check' --output-on-failure` → 4 of 4 (est. 3 min); `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p55-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 4 min; the backend compiles into the app and the Swift switches take the new values through `default:`).
Commit: efe075c0

### Phase 3 — Docs

- [x] `docs/parts-format.md` § `powerswitch` and the kinds table, `docs/backend.md` (Power
      switches, Voltages, the kind list) (§ 5). Observed: `grep -c 'enableRef\|flagWhen\|ilimit\|POWERSWITCH_INPUT_OVER\|switched level' docs/parts-format.md docs/backend.md` → `docs/backend.md:4`, `docs/parts-format.md:8`.

Acceptance: the docs name every key, error and rule.
  Check: `grep -c 'enableRef\|flagWhen\|ilimit\|POWERSWITCH_INPUT_OVER\|switched level' docs/parts-format.md docs/backend.md` → at least 1 per file (est. < 1 min).
Commit: (filled by the next commit)

## Validation Plan

- Tests: `powerswitch_test` (§ 6 a–v, including every refusal), the `voltage_test` case,
  `listing_check` / `trace_check` (the goldens, unchanged).
- Coverage check: `powerswitch_test` builds its fixture circuits through `circuit_start`, so the
  new `voltage.c` branches are in its run (cases (n)–(u) each fail if their branch is removed).
- Runtime proof: plan-55-J's harness (a placed part passing 5 V to a lit part in the app).
- Doc sync: § 5.
- Final gate: plan-55-K.

## Decisions

Owner decisions (2026-11-22, asked before this plan was written):

- **Scope: integrated ideal diodes, load switches, and current-limited and USB switches.** Power
  muxes (two inputs with select logic) are out; so are external-FET controllers, eFuses and
  hot-swap controllers, drivers.
- **Catalog: the top three makers of each family, Active parts only, no generic part**, as the
  todo list and plan-54 say.
- **ORing: two switch outputs on one net at different input levels do not clash; the net reads the
  higher level.** A switch output tied to a supply or a regulator at a different level is a clash.
- **Naming: the kind is `powerswitch`; the drawer is Power → Power Switches → Load Switches /
  Current-Limited / Ideal Diodes.**

Author decisions:

- **A new kind with a separate switched level**, not a regulator type (§ 3.6).
- **`"type"` is a device key**; `"enableRef": "input"` models the LM66100's comparator enable; the
  `"flagWhen"` pair covers fault flags (never low) and the LM66100's status (low when disabled).
- **A DigiKey company's labels count together** (Analog Devices = Analog Devices Inc. + Maxim,
  onsemi = onsemi + Fairchild, Microchip = Microchip + Micrel, Texas Instruments = Texas
  Instruments + National), as plan-54-A did for Amphenol and TE.
- **The families are DigiKey filters, not part-number lists** (§ 2); the verdict letters decide
  every part the kind cannot express.

## Corrections

1. **T0 is 42, not 41; P0 / R0 / F0 are 34436 / 34436 / 1904** (`grep -c '^logic_test' backend/CMakeLists.txt` → 42, `wc -l < backend/tests/golden/parts.txt` → 34436, `traces.txt` → 34436, `footprints.txt` → 1904 at e5242c08, 2026-11-23): other plans landed after 7c7b4d31. Every "T0 + 1" below reads 43 programs. The goldens' growth does not change A's check (byte-identical).
2. **The DigiKey quota row used key 1; the owner asked for `--opt2`** (2026-11-23): key 1's day was already spent; key 2 has its own quota (750 left). B's two listings run with `--opt2` (B's phases say how).

3. **Case (r)'s second circuit is a refusal, not "no problem".** § 6 (r) says `RFX-LDO33` → `PFX-DIODE` → `PFX-LOAD` → `SN74AUC00RGYR` (0.8–2.7 V) gives "no problem", but the chain carries 3.3 V to a part rated 0.8–2.7 V, which `regulator_test` (j) shows is refused. The test pins the refusal instead (`VOLTAGE_POWER_RANGE`, 3300 mV, `voltage: 3.3 V on pin 14 (VCC) of SN74AUC00RGYR; it needs 0.8-2.7 V`): it proves the level passes through both switches, which is what the case is for (`powerswitch_test` case (r)).

## Summary

The risk is `voltage.c`: a new level class in `effective()` and two clash rules. The kind and the
model copy the regulator's, and the goldens plus `voltage_test`'s unchanged results prove the new
class is empty without a switch. Everything else of plan-55 is data and part files behind this kind.
