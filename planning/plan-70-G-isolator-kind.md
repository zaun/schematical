# plan-70-G: Backend: the `isolator` kind and model, ERC pin types, `isolator_test`, docs

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-F

G adds the `isolator` kind to the backend: its loader, its part facts, and
a logic model for its four types. It loads two supply domains with
plan-75's shared loader (`kind_domains.c`), so a digital isolator's two sides are judged by the voltage check
like a level translator's. It gives the kind's pins their ERC types, adds
`isolator_test`, and documents the kind. No shipped part uses the kind yet
(H writes `isolator.json`). The tests run on fixture devices.

References:

- `docs/backend.md` § Adding a kind, § ERC pin types, § Tests;
  `docs/parts-format.md` § `relay`, § `transistor`, § `buffer` (model
  `translator`), § Two supply domains.
- `backend/src/models/relay.c` (the coil rule, contacts, lights),
  `transistor.c` (a contact that follows its control 1 ns later),
  `buffer.c` (open-drain outputs), `backend/src/library/kind_domains.c`
  (plan-75: the two-supply loader), `regulator.c` (`read_input`: a model that reads its power pins),
  `opamp.c` (the newest kind: the lines a new kind touches).
- plan-70-C § 3, plan-70-D § 3.1 (the spec columns H turns into these keys).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-F complete | `ls planning/completed/plan-70-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |
| Golden sizes at G's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | recorded here at G's start as **P1**, **R1**, **F1**, **E1** (P0 = R0 = E0 = 40832 and F0 = 3993 at planning) |
| The kind is still absent | `grep -c 'PART_ISOLATOR' backend/include/parts.h` → 0 | MET (2026-11-25: 0) |
| plan-75 complete (the shared supply-domain loader) | `ls planning/completed/plan-75-*.md` → exists; `grep -c 'kind_domains_load' backend/src/library/kind.h` → 1 | NOT MET (2026-11-25: plan-75 written, not started) |
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

`isolator_test` passes. On fixture devices it shows each of the following:

- **transistor**: a switch on 5 V into `A` (through a resistor), `K` on
  GND, `C` to an LED's anode pulled from 5 V, `E` on GND: the LED net
  follows the switch 1 ns later. The LED stays off with `A` floating, with
  `A` and `K` swapped (no AC input), and on X.
- **AC input**: the same with `"ac": true` lights for either polarity.
- **triac**: `MT1`–`MT2` conduct while the LED is driven. A strong 1 and a
  strong 0 across a closed contact is `smoke:`, as for a closed switch.
- **logic**: an inverting open-collector output (`VO!`) with a pull-up
  reads 0 while the LED is on and 1 (the pull-up) while it is off. A
  push-pull non-inverting output follows the LED. With `VCC` not driven to
  1 the output is Z. An enable `VE` (active high, open = high) left
  floating keeps the output on, and at 0 it turns it off (Z for tri-state,
  and the inactive level for push-pull, per § 3.4).
- **digital**: `VIA` → `VOA` across the barrier with both `VDD1`, `VDD2` at
  1. With `VDD1` floating, `VOA` drives the part's `failsafe` level
  (`high` part: 1, `low` part: 0). With `VDD2` floating, `VOA` is Z. A
  floating `VIA` drives `failsafe`. `EN2` at its inactive level makes `VOA`
  Z. A reverse channel (`VIB` on side 2 → `VOB` on side 1) works the same
  way.
- **lights**: one light per channel, on while its input is asserted.
- **loader**: each rejection of § 3.3 reports its message.
- **voltage check**: a digital fixture with 3.3 V on `VDD1` and 5 V on
  `VDD2` gives no `VOLTAGE_*` problem. The same with 5 V on a side whose
  `vcc` max is 3.6 V reports the over-voltage on that side's pins only. The
  LED and contact pins are never judged.
- **ERC**: `erc_pins_test` gives `passive` for LED, base and contact pins,
  `open_collector` for an open-drain output, `tri_state` for a gated
  tri-state output, and `power_in` for the supply pins. These are the types
  of KiCad 10's `Isolator.kicad_sym` symbols (§ 2).

Plus: every other ctest program still passes, and the goldens are
unchanged (no shipped part uses the kind), apart from the per-kind count
line the listing and trace checks add for a new kind (§ 3.7).

### Non-goals (explicit constraints)

- No shipped part, no app change (H, I).
- No analog behaviour: no LED current or forward drop, no CTR, no
  saturation, no triac holding current or AC zero crossing, no propagation
  delay from the datasheet, no common-mode transient, no glitch filter
  (§ 3.6 lists what is not modeled).
- No change to the buffer kind's behaviour or file format, and no change to
  `kind_domains.c` (plan-75's API is used as it is).
- No new pin role.

## 2. Current State

- **Coil rule.** `relay.c`'s header: a coil `{a, b}` is driven forward when
  `a` reads 1 and `b` reads 0, and X or Z on either pin is not driven. A
  channel's contacts close with `sim_set_contact`, and the lights hooks
  report one light per channel (`relay.c:508`, `:571`, `relay_lights`
  `:574`).
- **A contact that follows a control 1 ns later.** `transistor.c`
  (`TRANSISTOR_DELAY_PS` 1000, from a timer that re-reads the pin). A
  conducting channel is a strong two-way contact.
- **Power pins can be read.** `regulator.c:619` `read_input` reads its input pins
  (role `power`) with `sim_read_pin`: X if any reads X, 1 if one reads 1,
  else 0 or Z. A supply drives its pin strong 1 above 0 V and 0 at 0 V
  (`supply.c` header).
- **Domains.** At planning they are loaded only by `buffer.c`
  (`load_domains` `:710`, `voltage_b` `:743`, `bind_domain` `:833`, and the
  domain block of `buffer_load_part`). plan-75 (a prerequisite) moves them,
  unchanged, to `backend/src/library/kind_domains.c`. `Part.domains` and the
  voltage check's per-domain pass are general (`docs/parts-format.md` § Two
  supply domains; `voltage.c` header item 3).
- **Hooks.** `PartKindOps.pin_open_drain`, `pin_analog`, `pin_tristate`
  (`backend/src/library/kind.h:40`–`:49`). The voltage check skips analog
  pins (`voltage.c:772`, `:787`, `:807`, `:905`). `erc_pins.c:13`–`:30`
  lists the kinds whose signal pins are all `passive`. `isolator` is not one
  of them: only its LED and contact pins are passive (KiCad's choice, below).
- **The newest kind's lines.** `parts.h:32` ends the enum with
  `PART_TIMER, PART_OPAMP`; `kinds.c:54` / `:106` hold `OPAMP_KIND`;
  `tests/trace_check.c:22` `#define TRACE_LAST_KIND PART_OPAMP`;
  `tests/listing_check.c:50`–`:51` size by `PART_OPAMP + 1`;
  `tests/support/listing.c:57` names `"opamp"`; `CMakeLists.txt:282`
  `logic_test(opamp_test)`.
- **KiCad's pin types for isolators** (KiCad 10, `~/Applications/KiCad/KiCad.app/Contents/SharedSupport/symbols/Isolator.kicad_sym`,
  read at planning 2026-11-25). `PC817` and `4N25`: every pin `passive`
  (pin 3 of 4N25 `no_connect`). `HCPL-261A` (the base of `6N137`): `A`, `C`,
  `NC` `passive`, `GND` and `VCC` `power_in`, `VO` `open_collector`, `EN`
  `input`. `ISO7721D` and `ADuM1201AR`: `VCC1`/`VDD1`, `GND1`, `GND2`,
  `VCC2`/`VDD2` `power_in`, `IN*`/`VI*` `input`, `OUT*`/`VO*` `output`.

## 3. Design

### 3.1 The shared domain helpers (plan-75)

`isolator.c` loads and binds two domains with plan-75's API
(`backend/src/library/kind_domains.c`, declared in `kind.h`):

```c
typedef struct { const char *power; const char *const *pins; size_t pin_count; } KindDomainNames;
typedef struct { int present; KindDomainNames d[2]; } KindDomains;
int kind_domains_load(KindContext *ctx, const cJSON *json, KindDomains *out);
int kind_voltage_b(KindContext *ctx, const cJSON *json, const PartVoltage *base,
                   const PartVoltage **out);
typedef struct { size_t *power[2], power_count[2], *pins[2], pin_count[2]; } KindDomainPins;
int kind_domains_bind(KindContext *ctx, const PartPin *pins, size_t count,
                      const KindDomains *names, KindDomainPins *out);
int kind_domains_part(KindContext *ctx, const KindDomainPins *b, const PartVoltage *vcc,
                      const PartVoltage *vcc_b, Part *out);
```

The device loader calls `kind_domains_load`, and a digital isolator with
`present` 0 is the rejection `missing "domains"`. The pinout binder calls
`kind_domains_bind`. The part loader calls `kind_voltage_b` for `"vccB"` /
`"inputMaxB"` (`"vccB"` is required with domains), then `kind_domains_part`.
Its messages are plan-75's (the buffer kind's), unchanged.

### 3.2 Pins and roles

| Pin | Role | Name (H, plan-70-C § 3.1, plan-70-D § 3.1) |
|---|---|---|
| LED anode, cathode | `input` | `A`, `K` (`A1`, `K1`, … with several channels) |
| collector, emitter, base | `inout` | `C`, `E`, `B` (`C1`, `E1`, …) |
| triac main terminals | `inout` | `MT1`, `MT2` |
| logic input, enable | `input` | the datasheet's (`VIA`, `VE`, `EN1`) |
| logic output | `output` | the datasheet's (`VO`, `VOA`, `OUTA`) |
| supply, ground | `power`, `ground` | the datasheet's (`VCC`, `VDD1`, `GND2`) |
| no-connect, substrate | `nc` | `NC` |

Every `input`, `output` and `inout` pin is in a channel or is an enable
(`pin "X" is in no channel`). A channel pin of the wrong role is an error
(`isolator pin "C" must be inout`).

### 3.3 Device keys

| Key | Types | Value |
|---|---|---|
| `"type"` | all | `"transistor"`, `"triac"`, `"logic"` or `"digital"` (`unknown isolator type "X"`) |
| `"channels"` | all | 1 to 16 objects (`"channels" must have 1 to 16 entries`). transistor, triac: `{"anode", "cathode", "a", "b"}`, optional `"base"` (transistor only), optional `"ac": true`. logic: `{"anode", "cathode", "out"}` or `{"in", "out"}`. digital: `{"in", "out"}`. Optional `"invert": true` on logic and digital channels |
| `"darlington"` | transistor | optional, `true`: a Darlington output (a fact) |
| `"zeroCross"` | triac | optional, `true`: a zero-cross triac driver (a fact) |
| `"output"` | logic, digital | `"push-pull"` (default), `"open-drain"` or `"tri-state"`. `tri-state` needs an enable on the outputs' side (`a "tri-state" output needs an enable`) |
| `"power"` | logic with LED inputs | required: the output side's supply pin name, role `power` |
| `"domains"` | digital; logic with `in` channels | required: exactly the buffer kind's format (§ 3.1), the side-1 domain first |
| `"enables"` | logic, digital | optional, 1 or 2 objects `{"pin", "active": "high"\|"low", "open": "high"\|"low", "side": 1\|2}`. `side` is required with `domains`, and is the output side without them. At most one per side |

A key of another type is an error (`"zeroCross" is only for a triac`). A
channel's `in` and `out` of a digital isolator are in different domains
(`channel 2: "VIB" and "VOB" are on one side`).

### 3.4 The model (`backend/src/models/isolator.c`)

Every input change schedules a re-evaluation 1 ns later (`ISOLATOR_DELAY_PS`
1000, a timer that re-reads the pins, `transistor.c`'s shape). A pulse
shorter than that changes nothing.

- **Asserted input.** An LED channel is asserted while `anode` reads 1 and
  `cathode` reads 0 (`ac`: or the other way round). X on either pin makes it
  unknown, and anything else is not asserted. A logic `in` is asserted at 1
  and not at 0. Z on a logic opto's input is unknown, and Z on a digital
  isolator's input means the part's `failsafe` level. X is unknown.
- **Contact types** (transistor, triac). The channel's contact `a`–`b` is
  closed while asserted and open otherwise; unknown opens it. `base` is a
  pin with no effect.
- **A side is powered** while its power pin reads 1 (`read_input`'s rule
  over every pin of the name). For a logic opto with LED inputs only the
  output side has a power pin.
- **Enable.** An enable reads asserted at its `active` level, deasserted at
  the other level, its `open` level at Z, and unknown at X.
- **Logic and digital outputs.** Per channel, the output side unpowered →
  Z. Otherwise:
  - enable deasserted: Z for `tri-state` and `open-drain`, and the
    deasserted level for `push-pull`: 1 for a non-inverting channel (the
    6N137 style), applied before `invert`;
  - enable unknown: X;
  - digital with the input side unpowered: `failsafe`;
  - else the input (asserted 1, not 0, unknown X), inverted with `invert`.

  An `open-drain` output drives 0 and releases 1 to Z (`buffer`'s rule).
  Drives use `sim_schedule_inertial_drive`.
- **Lights.** One light per channel (`lights.c`), on while asserted.

### 3.5 Part keys and facts (`PartIsolator`)

| Key | Types | Value |
|---|---|---|
| `"viso"` | all, required | isolation voltage, V rms, above 0 and at most 20000 |
| `"ctrMin"` | transistor, required | %, above 0 and at most 100000 |
| `"ctrMax"` | transistor | %, at least `ctrMin` |
| `"vceo"` | transistor, required | V, above 0 and at most 1000 |
| `"ifMax"` | transistor, triac, logic with LED inputs | mA, above 0 and at most 1000 |
| `"vdrm"` | triac, required | V, above 0 and at most 2000 |
| `"ift"` | triac, required | mA, above 0 and at most 100 |
| `"itrms"` | triac | mA, above 0 and at most 10000 |
| `"rate"` | logic, digital | Mbit/s, above 0 and at most 10000 |
| `"vcc"` | logic, digital, required | `[min, max]` V (the kind_voltage rules): the output side for logic with LED inputs, side 1 otherwise |
| `"vccB"` | with `domains`, required | side 2's range (§ Two supply domains) |
| `"failsafe"` | digital, required | `"high"` or `"low"` |

```c
typedef struct {
    const char *type;            /* "transistor", "triac", "logic", "digital" */
    unsigned channels;
    int darlington, zero_cross, ac, inverting;  /* inverting: any channel inverts */
    const char *output;          /* NULL for contact types */
    uint32_t viso_v, ctr_min_pct, ctr_max_pct, vceo_mv, if_max_ua;
    uint32_t vdrm_mv, ift_ua, itrms_ma, rate_kbps;
    int failsafe_high;           /* -1 when not digital */
} PartIsolator;
```

`Part.isolator` points at it. `PART_ISOLATOR` goes after the last
`PartKind` of the enum at G's start.

### 3.6 Voltage check, ERC, not modeled

- `pin_analog`: LED, base and contact pins. The voltage check skips them,
  since their levels depend on series resistors and loads it does not
  model.
- `pin_open_drain`: the outputs of an `open-drain` device.
- `pin_tristate`: the outputs of a `tri-state` device, and the outputs of
  a side with an enable.
- `erc_pins.c`: an `isolator` pin for which `part_pin_analog` is 1 is
  `passive`, which matches KiCad (§ 2). The other pins take the general
  rows (`power_in`, `open_collector`, `tri_state`, `output`, `input`).
- `docs/backend.md` § ERC pin types gains the row.
- Not modeled, and the docs say so: LED forward voltage and current, CTR
  and saturation (a closed contact is a strong contact), dark current, the
  triac's latching and holding current (it opens when the LED stops), zero
  crossing (it closes at once), the datasheets' propagation delays (1 ns
  for all), glitch filters, UVLO thresholds (powered = 1), the "undetermined"
  output of an unpowered output side (Z here), and channels that share an
  enable across sides.

### 3.7 Shared lines (each added only if absent, after whatever is last; grep first)

`parts.h` enum and the `Part.isolator` pointer; `kinds.c`
`extern const PartKindOps ISOLATOR_KIND;` and its `KINDS[]` line;
`CMakeLists.txt` (the model source, `logic_test(isolator_test)`);
`tests/support/listing.c` (`case PART_ISOLATOR: return "isolator"` and the
facts line); `tests/trace_check.c` `TRACE_LAST_KIND`; `tests/listing_check.c`
array sizes (both moved to the new last kind); `erc_pins.c`. The last-kind
markers point at the last `PartKind` entry once this one is added (read
`parts.h` at the step: plans 69–74 add kinds after `PART_OPAMP` too).

### 3.8 Docs

- `docs/parts-format.md`: `### isolator` (after `### relay`) with § 3.2–3.6.
  Its example device is a 6-pin transistor opto, and a 2-channel digital
  isolator is the second example. The drawer table gains **Interface →
  Optocoupler (Transistor, Darlington, Triac, Logic); Digital isolator**.
  § Two supply domains: "the `buffer` kind's model `translator` and the
  `isolator` kind take these keys".
- `docs/backend.md`: `isolator_test` in § Tests; the ERC row;
  `kind_domains.c` in § Adding a kind (a two-supply kind uses it).

## 4. Tests (`backend/tests/isolator_test.c`, `tests/support/isolator_fixture.h`)

The fixture devices: a 4-pin transistor opto, a 6-pin one with base and an
AC twin, a triac driver, a 6N137-style logic opto (inverting, open drain,
`VE` active high, open high), a push-pull logic opto, and a 2-channel
digital isolator (1/1, `EN2`, `failsafe` part `high` and part `low`). The
cases are those of § 1, one function each. The rejections:

- an unknown type;
- 0 or 17 channels;
- a `base` on a triac;
- `tri-state` without an enable;
- a digital isolator without `domains`;
- a channel on one side;
- a missing `failsafe`;
- `ctrMax` below `ctrMin`;
- a pin in no channel;
- a channel pin of the wrong role.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: The kind, the model, the tests

- [ ] `parts.h`, `isolator.c` (loader, binder, part keys, model, lights,
      the three pin hooks) per § 3.2–3.6; the shared lines of § 3.7.
- [ ] `erc_pins.c` per § 3.6.
- [ ] `tests/support/isolator_fixture.h`, `tests/isolator_test.c` per § 4.

Acceptance: the new program passes and nothing else moves.
  Check: `ctest --test-dir /tmp/logic-p70 -R 'isolator_test|erc_pins|listing_check|trace_check|library_check'`
  passes; `git diff --stat backend/tests/golden` shows no line removed or
  changed (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0);
  `node scripts/source-size.js --check` → exit 0 (`isolator.c`,
  `isolator_test.c` under 1000 code lines); `node scripts/coverage.js --check --backend`
  → exit 0 (no uncovered line in `isolator.c` or the shared lines it
  touches). Takes about 10 min: a scoped rebuild plus one instrumented backend
  build and `ctest`, the only tool that shows every new line runs.
Commit:

### Phase 2: Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 3.8.

Acceptance: the docs name every key and message the loader has.
  Check: `grep -o '"[a-zA-Z]*" is only for a [a-z]*\|unknown isolator type' backend/src/models/isolator.c | sort -u`
  each appear in `docs/parts-format.md` (`grep -c` per string ≥ 1). Takes
  about 1 min.
Commit:

## Validation Plan

- Tests: `isolator_test` (new), `erc_pins_test`, `library_check`,
  `listing_check`, `trace_check`.
- Coverage check: every row of § 3.3 and § 3.5 has a loader case in
  `isolator_test` (`grep -c` of each key in the test file ≥ 1), and
  `node scripts/coverage.js --check --backend` → exit 0 (plan-68's rule: no
  uncovered line).
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate: plan-70-J.

## Decisions

- Owner, 2026-11-25: **an isolator is logic in play mode** (census README
  decision 3).
- Author: **one kind for optocouplers and digital isolators.** They share
  the input → barrier → output shape, the facts (`viso`), and the
  two-supply domains of the logic types.
- Author: **a new kind, not the relay kind's `solid-state` type.** A relay
  has no logic outputs and no supply sides. Its 1 ms armature delay is wrong
  for an optocoupler.
- Author: **one delay, 1 ns, for every isolator**, as for transistors.
  Datasheet propagation delays are facts left for later.
- Author: **an unpowered output side is Z.** The Si86xx datasheet calls it
  undetermined (Table 3), and Z drives nothing, so it never invents a
  level.
- Author: **the domain helpers are shared** (plan-75, a prerequisite)
  rather than copied, so the `isolator` and `transceiver` kinds and `buffer`
  judge two supplies the same way.
- Owner, 2026-11-25: **backend letters wait for plan-68** and meet its size
  and coverage checks (§ Prerequisites, Phase 1's acceptance).

## Corrections

## Summary

The model is small (contacts and copied levels). The care is in the power
and enable rules of the logic types.
