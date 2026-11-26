# plan-69-E: Backend — the `tvs` kind (facts, a passive model, the standoff check, ERC passive pins)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-D

E adds the `tvs` kind: the loader and facts of a TVS diode (one or more
unidirectional or bidirectional diodes) and of an ESD steering array (lines steered
to a rail and to ground). A TVS part is in the circuit and drives nothing; the
voltage check rates every pin it protects by the part's working standoff voltage;
the ERC types its pins passive, as KiCad's own TVS symbols do.

References:

- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Kinds (`diode`,
  `capacitor`, `opamp`).
- `backend/src/models/opamp.c` and `capacitor.c` (the passive, simulated shape E
  copies: no `start`, `pin_changed` or `timer`); `backend/src/models/diode.c`
  (the `diodes` array E's diode type mirrors).
- `backend/src/voltage.c` (`fuse_limit_mv`, the rating chain at the
  `VOLTAGE_CAPACITOR_OVER` / `VOLTAGE_FUSE_OVER` branches, `voltage_describe`),
  `backend/include/voltage.h` (`VoltageProblemKind`).
- `backend/src/erc_pins.c` (`kind_is_passive`).
- `planning/completed/plan-48-F-opamp-kind.md` (the precedent letter);
  `planning/completed/plan-47-I-ifrb-kinds-inductor-fuse.md` (the fuse rating check).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-69-D complete | `ls planning/completed/plan-69-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| The kind is absent | `grep -c 'PART_TVS' backend/include/parts.h` → 0; `ls backend/src/models/tvs.c` → no such file | MET (2026-11-25: 0; no file) |
| T1 and goldens measured | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | at planning 77 and 40832 (A § Prerequisites); E records its own T1 here before Phase 1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `tvs.json` files load: kind `PART_TVS`, `Part.tvs` facts, `simulated` 1, the
  rejections of § 5.
- In a circuit a TVS part drives no net; a circuit with a TVS and simulated parts
  starts and runs.
- A net above the part's `vrwm` on any pin that is not a `ground` pin is a
  `VOLTAGE_TVS_OVER` problem with `max_mv` = `vrwm`; `voltage_describe` gives
  `voltage: 12 V on pin 1 (K) of TESTTVS5; it is rated 5 V`.
- `erc_pin_type` gives `ERC_PASSIVE` for every pin of a TVS part.
- `ctest` passes T1 + 1 programs (`tvs_test`); goldens unchanged (E adds no part).

### Non-goals (explicit constraints)

- No analog behavior: no clamping, no breakdown, no forward conduction. A TVS links
  no nets (unlike a `diode`, whose pairs carry levels).
- No change to the rules of the existing voltage problems or to `circuit.c`.
- No part file, no app change (F, G, H).

## 2. Current State

- A passive kind that stays simulated (`opamp.c`, `capacitor.c`: no callbacks)
  neither turns Run off nor drives a net (plan-48-F § 2).
- The voltage check's rating chain (`voltage.c`, the `else if` after
  `VOLTAGE_INPUT_OVER`) tests per kind: `VOLTAGE_SWITCH_OVER`,
  `VOLTAGE_CAPACITOR_OVER` (either pin against the rating), `VOLTAGE_TRANSISTOR_OVER`,
  `VOLTAGE_FUSE_OVER` (`fuse_limit_mv`), the regulator and power-switch input
  maxima, `VOLTAGE_NIXIE_UNDER`. `VOLTAGE_FUSE_OVER` is the last enum value
  (`backend/include/voltage.h`); `voltage_describe` gives the four "rated" kinds one
  text: `voltage: %g V on pin %s (%s) of %s; it is rated %g V`.
- The first two branches need `part->vcc`; a TVS has none, so only its own branch
  applies to it.
- `kind_is_passive` (`erc_pins.c`) lists the kinds whose every pin is KiCad
  `passive`: connector, header, socket, resistor, capacitor, inductor, diode, LED,
  fuse, crystal, relay, buzzer, switch, transistor, the display tubes and LCD glass.
- `PartKind` ends with `PART_OPAMP` (`grep -n 'PART_OPAMP' backend/include/parts.h`
  → line 32); `TRACE_LAST_KIND` is `PART_OPAMP` (`backend/tests/trace_check.c:22`);
  `listing_check.c:50-51` size their arrays `PART_OPAMP + 1`.

## 3. The kind

### 3.1 Part file format (`"kind": "tvs"`)

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"diode"` (DigiKey's Zener type: one or more TVS diodes) or `"steering"` (a rail-to-rail steering array) |
| `"diodes"` | `diode` only, required: 1–32 entries, each `{"anode", "cathode"}` (unidirectional) or `{"terminals": [<pin>, <pin>]}` (bidirectional); pin names |
| `"lines"` | `steering` only, required: 1–32 pin names, the protected lines |
| `"rail"` | `steering` only, optional: the pin name of the rail (VBUS, VCC, VP) |

Pin rules (`bind_pinout`): every pin is `inout`, `ground`, `nc` or `absent` (`tvs
pin "X" must be inout, ground or nc`); every `inout` pin is named by a `diodes`
entry, a line or the rail (`pin "X" is in no diode or line`), and no name is a line
twice; a `diode` device has no `ground` pin (its common terminal is a diode end,
`inout`: `a TVS diode's pins are inout`); a `steering` device has at least one
`ground` pin (`a steering array needs a ground pin`); a name the device keys give
is a pin of every pinout.

Part keys (numbers, the part's own line of the datasheet table):

| Key | Value |
|---|---|
| `"vrwm"` | required: working standoff voltage, volts, above 0 and at most 1000 |
| `"vbr"` | optional: minimum breakdown voltage, volts, at least `vrwm`, at most 1000 |
| `"vc"` | optional: maximum clamping voltage at `ipp`, volts, at least `vbr` (or `vrwm`), at most 2000 |
| `"ipp"` | optional: peak pulse current, amps, above 0 and at most 100000; needs `"ippWave"` |
| `"ippWave"` | with `ipp`: `"8/20"` or `"10/1000"` (µs) |
| `"ppk"` | optional: peak pulse power, watts, above 0 and at most 1000000 |
| `"cj"` | optional: junction capacitance, pF, from 0 to 100000 |

```c
/* A TVS diode or ESD steering array (plan-69-E): facts only. type "diode" or
   "steering"; diode_count diodes, bidirectional_count of them bidirectional;
   line_count lines and has_rail (steering); vrwm_mv, vbr_mv, vc_mv in mV,
   ipp_ma in mA with ipp_wave "8/20" or "10/1000" (NULL = none), ppk_w in W,
   cj_ff in fF; 0 = not given (vrwm_mv is always given). */
typedef struct { const char *type; size_t diode_count, bidirectional_count, line_count; uint8_t has_rail;
                 uint32_t vrwm_mv, vbr_mv, vc_mv, ipp_ma, ppk_w; uint64_t cj_ff; const char *ipp_wave; } PartTvs;
```

`const PartTvs *tvs;` on `Part`.

### 3.2 The model and the standoff check

- `TVS_MODEL`: the passive shape of `opamp.c` (`.name`, `.pin_count`, `.create`
  returning the `Part`; no `start`, `pin_changed` or `timer`).
- `voltage.c`: `static uint32_t tvs_limit_mv(const Part *part)` (`vrwm_mv`, or 0 for
  another kind), and one branch in the rating chain after the fuse's:
  `part->kind == PART_TVS && role != PIN_GROUND && e.mv > tvs_limit_mv(part)` →
  `VOLTAGE_TVS_OVER`, `min_mv` 0, `max_mv` the limit. Its comment: a pin's net level
  against ground is the voltage across a protection element whose other end is on
  ground (the capacitor's reasoning, plan-33-E § 3.3).
- `voltage.h`: `VOLTAGE_TVS_OVER` after the current last value, with the comment
  `/* TVS or ESD-array pin above its working standoff voltage (plan-69-E; max_mv) */`;
  `voltage_describe` adds it to the four "rated" kinds' case.
- `erc_pins.c`: `case PART_TVS:` in `kind_is_passive`.

## 4. Docs

- `docs/parts-format.md`: a `### tvs` section (keys, pin rules, the two types, an
  example unidirectional device and a four-line steering device; facts only, the
  standoff check); the drawer table's Discrete row gains `TVS diodes ›
  Unidirectional / Bidirectional / ESD arrays (plan-69)`.
- `docs/backend.md` § Layout: `tvs.c`; the voltage check's list of problems:
  `VOLTAGE_TVS_OVER`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind, the check, the tests

- [ ] `backend/include/parts.h`: `PART_TVS` after the current last `PartKind`
      (`grep -n 'PART_' backend/include/parts.h | sed -n '/PartKind/,/}/p'` first: plans
      70–74 may have added kinds), `PartTvs`, `const PartTvs *tvs;` on `Part`.
- [ ] `backend/src/models/tvs.c`: `TVS_KIND` and `TVS_MODEL` per § 3.1–3.2.
- [ ] `backend/src/library/kinds.c`: the `extern` and `&TVS_KIND` at the end of
      `KINDS[]` (`grep -c 'TVS_KIND' backend/src/library/kinds.c` → 0 before, 2
      after); `backend/CMakeLists.txt`: `src/models/tvs.c` and `logic_test(tvs_test)`.
- [ ] `backend/include/voltage.h`, `backend/src/voltage.c`: `VOLTAGE_TVS_OVER` after
      the current last value, `tvs_limit_mv`, the branch and the describe case per
      § 3.2 (`grep -c 'VOLTAGE_TVS_OVER' backend/include/voltage.h` → 0 before).
- [ ] `backend/src/erc_pins.c`: `case PART_TVS:` in `kind_is_passive`.
- [ ] `backend/tests/support/listing.c`: `case PART_TVS: return "tvs"` and the facts
      line `type=%s diodes=%zu bi=%zu lines=%zu rail=%u vrwm=%u vbr=%u vc=%u ipp=%u
      wave=%s ppk=%u cj=%llu` (`-` for a NULL wave); `TRACE_LAST_KIND` and
      `listing_check.c`'s two array sizes → the last `PartKind` entry in
      `parts.h` after `PART_TVS` is added (read the enum at this step).
- [ ] `backend/tests/support/tvs_fixture.h` and `backend/tests/tvs_test.c`: the
      cases of § 5.

Acceptance: the kind loads, stays passive, and the checks treat it as § 1 says.
  Check: `cmake -S backend -B /tmp/logic-p69-e && cmake --build /tmp/logic-p69-e -j8 && /tmp/logic-p69-e/tvs_test` → every line `PASS`; `ctest --test-dir /tmp/logic-p69-e -j8` → `100% tests passed`, T1 + 1 programs, `listing_check`, `trace_check`, `erc_pins_check` goldens unchanged (est. 4 min: a build from scratch).
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check`
  → exit 0 and `node scripts/coverage.js --check --backend` → exit 0, so no new or
  changed backend file is over 1000 code lines or has an uncovered line (est. 6 min:
  one instrumented backend build with `ctest`, the only tool that shows every new
  line runs).
Commit:

### Phase 2 — Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name the kind and the problem.
  Check: `grep -n '^### `tvs`' docs/parts-format.md` → 1 line; `grep -c 'tvs.c\|VOLTAGE_TVS_OVER' docs/backend.md` → 2 or more (est. < 1 min).
Commit:

## 5. `tvs_test` cases

Fixture (`tvs_fixture.h`): a `diode` device with one unidirectional diode, pinout
`two-pin` (`1 K inout`, `2 A inout`), parts `TESTTVS5` (`"vrwm": 5, "vbr": 6.4,
"vc": 9.2, "ipp": 65.2, "ippWave": "10/1000", "ppk": 600`, footprint `DIODE-SMB`) and
`TESTTVS12` (`"vrwm": 12`); a `diode` device with one bidirectional diode (`1 1
inout`, `2 2 inout`), part `TESTTVS5C` (`"vrwm": 5`); a `steering` device, pinout
`p6` (`1 IO1 inout`, `2 GND ground`, `3 IO2 inout`, `4 IO3 inout`, `5 VBUS inout`,
`6 IO4 inout`), `"lines": ["IO1","IO2","IO3","IO4"]`, `"rail": "VBUS"`, part
`TESTESD4` (`"vrwm": 5.5, "cj": 0.8, "ipp": 5, "ippWave": "8/20"`, footprint `SOT23-6`).

- (a) `TESTTVS5`: kind `PART_TVS`, `simulated` 1, 2 pins, `type` "diode",
  `diode_count` 1, `bidirectional_count` 0, `vrwm_mv` 5000, `vbr_mv` 6400, `vc_mv`
  9200, `ipp_ma` 65200, `ipp_wave` "10/1000", `ppk_w` 600, `cj_ff` 0.
- (b) `TESTTVS5C`: `bidirectional_count` 1, `ipp_wave` NULL. `TESTESD4`: `type`
  "steering", `line_count` 4, `has_rail` 1, `vrwm_mv` 5500, `cj_ff` 800, `ipp_ma`
  5000, `ipp_wave` "8/20".
- (c) `"type": "zener"` → `unknown TVS type "zener"`; `"lines"` on a `diode` device →
  `"lines" is only for a steering array`; `"diodes"` on a `steering` device →
  `"diodes" is only for a TVS diode`; 33 lines → `"lines" must have 1 to 32 entries`;
  a `diodes` entry with both `anode` and `terminals` → `a diode has "anode" and
  "cathode", or "terminals"`.
- (d) a `steering` pinout with no `ground` pin → `a steering array needs a ground
  pin`; a `GND` pin with role `ground` on a `diode` device → `a TVS diode's pins are
  inout`; an `inout` pin no entry names → `pin "IO5" is in no diode or line`; a pin
  with role `output` → `tvs pin "IO1" must be inout, ground or nc`.
- (e) `"vrwm"` missing → `missing "vrwm"`; `"vbr": 4` with `"vrwm": 5` →
  `"vbr" must be at least "vrwm"`; `"ipp": 10` without `"ippWave"` → `"ipp" needs
  "ippWave"`; `"ippWave": "1/2"` → `"ippWave" must be "8/20" or "10/1000"`.
- (f) a circuit of `TESTTVS5` (K on `5V`, A on `GND`) and an `SN74HC04N` whose
  input is on `5V` and output drives an LED to `GND`: `circuit_add_part` and
  `circuit_start` return 0; after 1 ms the inverter's output net reads 0 and
  `circuit_net_changes` on the TVS's nets is 0.
- (g) the same with K on `12V`: `circuit_start` returns -1;
  `circuit_voltage_problem` gives `VOLTAGE_TVS_OVER` on pin 1, `mv` 12000, `max_mv`
  5000; `voltage_describe` → `voltage: 12 V on pin 1 (K) of TESTTVS5; it is rated 5 V`.
- (h) `TESTTVS12` with K on `12V`: starts (12 V is not above 12 V).
- (i) `TESTESD4` with VBUS on `5V`, GND on `GND`, IO1 on a `3V3` supply: starts;
  with VBUS on `12V`: `VOLTAGE_TVS_OVER` on pin 5 (`max_mv` 5500); its GND pin is
  never checked.
- (j) `erc_pin_type` of every pin of `TESTTVS5` and `TESTESD4` → `ERC_PASSIVE`.

## Validation Plan

- Tests: `tvs_test`; `listing_check`, `trace_check`, `erc_pins_check` with
  unchanged goldens; `voltage_test` (the chain it already covers).
- Coverage check: `ctest --test-dir /tmp/logic-p69-e -N | grep -c tvs_test` → 1.
- Runtime proof: cases (f)–(i): runs with a TVS in the circuit, and the problem it
  raises.
- Doc sync: § 4.
- Final gate: plan-69-I.

## Decisions

- Owner, 2026-11-25: a TVS / ESD part is facts only in play mode.
- Author: **a passive simulated model, not `"simulated": false`**, so a placed TVS
  never turns Run off (plan-48-F's reasoning).
- Author: **no diode links**: a `diode` part's pairs carry levels anode → cathode;
  a TVS's would short a bidirectional part's two nets and, for a unidirectional one
  wired line → GND, do nothing a logic run can see. It links nothing.
- Author: **the standoff check rates every non-ground pin by `vrwm`**, the voltage
  above which the part starts to conduct; this is the one rule the voltage check
  can test, as capacitors and fuses are tested by their ratings.
- Author: **ERC passive**: KiCad's TVS and ESD-array symbols type their pins
  passive, so `erc_pins.txt` matches KiCad's view (plan-63's rule).
- Author: **one kind, two types**, so a multi-line Zener-type array and a steering
  array share facts and the app's sidebar rows.

## Corrections

## Summary

A loader, facts, a passive model and one new voltage rule. The rule is the part
with consequences: it is what makes a placed TVS useful in the app.
