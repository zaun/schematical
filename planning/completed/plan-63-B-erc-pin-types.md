# plan-63-B: ERC pin types in the backend

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: nothing (it lands after plan-63-A by letter order; it reads nothing A adds)

Plan 63's overview, the sub-plan list and the Prerequisites are in
`planning/completed/plan-63-A-power-flag.md`; they hold for this plan.

KiCad's ERC reasons about twelve pin types; the parts here carry seven roles
(`input`, `output`, `inout`, `power`, `ground`, `nc`, `absent`). The outcome of
63-B: **`erc_pin_type(part, pin, &type)` answers, for every pin of every part,
the KiCad pin type it stands for**, from the role, the part's kind and what the
kind already knows (open drain), plus a new kind hook, `pin_tristate`,
implemented here for the `buffer`, `latch` and `dff` kinds (2950 of the
3756 parts that name an output enable; the other kinds are 63-C). Nothing checks anything
yet (63-D).

References:

- `planning/plan-63-data/README.md` § "Roles to KiCad types", `count_roles.txt`.
- `backend/src/library/kind.h` (`PartKindOps`), `library/kinds.c`
  (`part_pin_open_drain`), `backend/src/voltage.c` (its use of the open-drain
  hook), `backend/include/parts.h` (`PinRole`, `PartKind`, `PartSupply`),
  `backend/src/models/model_pins.h` (`regulator_pin_out`, `powerswitch_pin_out`).
- `docs/backend.md` § tests and goldens (line 770 on), `docs/parts-format.md`
  (the data keys named below).

## Prerequisites

As plan-63-A § Prerequisites (re-run its four commands).

## 1. Goal

- `backend/include/erc.h` declares `ErcPinType` (KiCad's twelve, in the order
  of the pin map: input, output, bidirectional, tri_state, passive, free,
  unspecified, power_in, power_out, open_collector, open_emitter, no_connect),
  `erc_pin_type`, `erc_pin_drives_power` and `erc_pin_type_name`.
- `PartKindOps` has an optional `pin_tristate` beside `pin_open_drain`, and
  `part_pin_tristate(part, pin)` calls it.
- `buffer`, `latch` and `dff` parts answer it: an `output`-role pin the part
  releases to Z through an output-enable control is tri-state.
- `backend/tests/golden/erc_pins.txt` has one line per part (40174) and
  `erc_pins_check` holds the library to it.

### Non-goals (explicit constraints)

- No simulation change: no model drives a different value, no part file changes.
- `voltage_check` and `part_pin_open_drain` keep their behavior (`voltage_test`,
  `listing_check`, `trace_check` stay green unchanged).
- No part gets `free`, `unspecified` or `open_emitter`: the part files carry
  nothing that says so. The three types exist for the pin map (63-E) and
  `erc_pin_type` never returns them.
- No pin-type override in the Part Editor or the part files.

## 2. Current State

`PinRole` (`backend/include/parts.h:26`): `PIN_INPUT, PIN_OUTPUT, PIN_INOUT,
PIN_POWER, PIN_GROUND, PIN_NC, PIN_ABSENT`. `PartKindOps`
(`library/kind.h:28-47`) carries two optional per-pin questions,
`pin_open_drain` and `pin_analog`, answered through `part_pin_open_drain` /
`part_pin_analog` (`library/kinds.c:119-138`; open drain is asked only of
`output` and `inout` pins). 47 kind tables are defined (`grep -rn 'const
PartKindOps' backend/src | wc -l` → 47), positionally initialised, so a field
added last is NULL in all of them. Open drain is answered in 30 model files
(`grep -ln 'open_drain' backend/src/models/*.c | wc -l` → 30).

A supply symbol is a part of kind `supply` with `Part.supply->pull`
(`parts.h:130`): a rail has one `power` or `ground` pin; a pull-up / pull-down
symbol has an `output` pin. A regulator's and a power switch's output pins are
found by `regulator_pin_out(part, k, pin)` and `powerswitch_pin_out(part, k,
pin)` over elements `k` (`models/model_pins.h:72,88`), which `voltage.c` already
iterates through `regulator_element` / `powerswitch_element`.

The part files: 40174 parts and 116,155 pin entries (the role counts below, summed).

### Measured populations

| What | Count | Command |
|---|---|---|
| Pin entries by role, all kinds | inout 63158, input 21756, output 12915, power 7237, ground 7108, nc 3895, absent 86 | `python3 -I planning/plan-63-data/count_roles.py \| tail -1` |
| Parts | 40174 | `wc -l backend/tests/golden/parts.txt` → 40174 |
| Parts that name an output enable, in the three kinds of this plan | buffer 2089, dff 238, latch 623 (= 2950) | `count_roles.py`, the `oe` figure of each |
| Same, kinds of 63-C | register 162, shift 170, counter 33, mux 25, converter 63, serial 7 (= 460); eeprom 174 and sram 172 answer by role (their data pins are `inout`) | same |
| Kind tables | 47 | `grep -rn 'const PartKindOps' backend/src \| wc -l` |

### Verified properties

- Kinds whose pins are all passive in KiCad's own libraries (README § mapping):
  `PART_CONNECTOR, PART_HEADER, PART_SOCKET, PART_RESISTOR, PART_CAPACITOR,
  PART_INDUCTOR, PART_DIODE, PART_LED, PART_FUSE, PART_CRYSTAL, PART_RELAY,
  PART_BUZZER, PART_SWITCH, PART_TRANSISTOR, PART_7SEG, PART_NIXIE,
  PART_DEKATRON, PART_LCD` all exist in `PartKind` (`parts.h:21-27`). Their
  roles, from `count_roles.txt`: capacitor, buzzer, 7seg, led, nixie, dekatron,
  lcd, relay (coil) carry `input` pins; connector, header, socket, resistor,
  inductor, diode, fuse, crystal, switch, transistor `inout`.
- The three kinds of this plan gate their outputs with an output enable:
  `buffer` models `tristate`/`transceiver`/`registered` (`docs/parts-format.md`
  § Model `tristate` line 4806, `registered` 4836; unit key `oe`), `latch` and
  the `dff` kind (`outputEnable`, `docs/parts-format.md:1765,1777`: "1 puts Q/QN
  in Z"). `SN74HC574N` is a `dff` part (device `74_574`) and `SN74HC373N` a
  `latch` part (`python3 -I` over `Parts/*.json`: kind and device printed, 2026-11-25).
- `SN74HC595N` (kind `shift`, device `74_595`) names `storage.outputs` QA–QH
  with `outputEnable` OE and `outputs` QH' (not gated): the case 63-C pins down.

## 3. Design Overview

One function, `erc_pin_type`, in new `backend/src/erc_pins.c`:

1. role `absent`: no type (the function returns 0; callers skip the pin).
2. role `nc`: `ERC_NO_CONNECT`.
3. kind `supply`: pull (`Part.supply->pull`) → `ERC_PASSIVE` for every pin;
   a rail's `power` / `ground` pin → `ERC_POWER_IN` (and
   `erc_pin_drives_power` is 1 for it: a supply symbol is a driver on its own,
   63-A's note).
4. kind in the passive list (§ 2) and role `input`, `output` or `inout` →
   `ERC_PASSIVE`.
5. role `power` or `ground` → `ERC_POWER_IN`.
6. role `output`: a regulator or power switch output pin → `ERC_POWER_OUT`;
   else `part_pin_open_drain` → `ERC_OPEN_COLLECTOR`; else `part_pin_tristate`
   → `ERC_TRI_STATE`; else `ERC_OUTPUT`.
7. role `inout`: `part_pin_open_drain` → `ERC_OPEN_COLLECTOR`, else
   `ERC_BIDIRECTIONAL`.
8. role `input` → `ERC_INPUT`.

Why a supply is `power_in` that drives, and not `power_out`: in KiCad a `+5V`
symbol is `power_in` and a `power_out` on the same net is a pin-map error
(`power_out` + `power_out`); a regulator's output pin on a rail with a `5V`
symbol is normal here. So the symbol sits in the pin map as `power_in`,
and the check (63-D) counts it as having its own driver.

The hook: `int (*pin_tristate)(const Part *part, size_t pin)` last in
`PartKindOps`; `part_pin_tristate` (in `kinds.c`) answers 0 unless the pin has
role `output` and the kind sets it. (A transceiver's A and B pins are `inout`, so they are `bidirectional` and need no hook.) Each of the three kinds answers 1 for the
output pins its model gates with an output enable, taken from the same bound
data the model uses to drive Z, so the answer and the simulation cannot differ:
`buffer` units whose model gates its outputs with an `oe` (the `tristate`
model's `y` pins; `registered` outputs with an output enable), the `dff` and
`latch` kinds' `outputEnable`.

Where the correctness risk is: the hooks (a wrong pin set), and the passive list (a kind
missing from it turns resistors into `input`/`output` pins). Both are
pinned by the golden and the named-part test. Gate class: behavior-adding, so
byte-identity is not the gate for the new code; it is the gate for everything
old: `listing_check`, `trace_check`, `voltage_test` run unchanged.

Rejected: deriving tri-state from the pin name or the part's description
(`3-state` in text): 2089 buffer parts are not uniform and a wrong guess would
make a bus-sharing design report a conflict. Probing the engine (drive OE
inactive, read Z) for every part: slower than a data answer and it needs a
per-kind harness anyway. `power_out` for supplies: above.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; a moot task is `- [x]
> ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An unticked box
> means NOT DONE.

### Phase 1 — The type function and the hook, no kind answers it yet

Outputs are `output` until a kind answers the hook; safe to land alone.

- [x] `backend/src/library/kind.h`: `pin_tristate` last in `PartKindOps`, with
      the comment style of `pin_open_drain`. `backend/src/library/kinds.c`:
      `part_pin_tristate`, built like `part_pin_open_drain` (0 unless role
      `output`). `backend/src/models/model_pins.h`: declare it beside
      `part_pin_open_drain` (`:117`).
- [x] New `backend/include/erc.h` and `backend/src/erc_pins.c` as § 1 and § 3:
      `ErcPinType`, `int erc_pin_type(const Part *, size_t pin, ErcPinType *)`
      (0 for an absent pin or `pin >= pin_count`), `int erc_pin_drives_power(const
      Part *, size_t pin)`, `const char *erc_pin_type_name(ErcPinType)` returning
      KiCad's word (`"tri_state"`, `"power_in"`, …). Add `src/erc_pins.c` to
      `add_library(logic …)` in `backend/CMakeLists.txt` (next to `src/voltage.c`,
      line 13) and `#include "erc.h"` to
      `schematical/schematical/schematical-Bridging-Header.h`.
- [x] `backend/tests/erc_pins_test.c` (`logic_test(erc_pins_test)` in
      `CMakeLists.txt`, beside `voltage_test`): the named parts below. Parts
      that do not use a hook (all but the tri-state ones) are asserted now.

| Part | Asserts (every pin of the role named) |
|---|---|
| `SN74HC00N` (gate) | input → input; output → output; power, ground → power_in |
| `SN74LS07N` (gate, open drain) | output → open_collector |
| `SN74HC74N` (dff, no OE) | output → output |
| `SN74HC138N` (decoder), `SN74HC163N` (counter) | output → output |
| `GENERIC-LED`, `GENERIC-RESISTOR`, `GENERIC-HEADER-1X02` | every pin passive |
| `PULLUP-5V`, `PULLDOWN` | every pin passive, `erc_pin_drives_power` 0 |
| `5V`, `GND` | power_in, `erc_pin_drives_power` 1 |
| `LM7805MPX/NOPB` (regulator) | the output pin power_out; the input pin power_in; ground power_in |
| `ATMEGA328P-PU` (mcu) | inout → bidirectional; power, ground → power_in; nc → no_connect |

Acceptance: the table holds; no existing check changes.
  Check: `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build --target erc_pins_test voltage_test listing_check && /tmp/logic-build/erc_pins_test && /tmp/logic-build/voltage_test | tail -1 && /tmp/logic-build/listing_check | tail -1` → every `erc_pins_test` line `PASS`; `voltage_test` and `listing_check` end as before this change (record their last lines in the Result) (est. 6 min, most of it the first build).
  Result: in `/tmp/logic-build-wt63` (plan-63-A Corrections 1): `erc_pins_test` every line PASS, `erc_pins_test: all passed`; `voltage_test: all passed`; `listing_check` last line `40174 parts compared, 0 skipped` (2026-11-25).
Commit: 4b64f493

### Phase 2 — The hook for buffer, latch and dff

- [x] `backend/src/models/buffer.c` (and `buffer_reg.c`): `buffer_pin_tristate`
      in the kind table, beside `buffer_pin_open_drain` (`:919`): 1 for an
      output-role pin that the model gates with an output enable (a `tristate`
      unit with `oe`, a `registered` unit with an output enable), using the
      model's own `is_output` and enable binding; 0 for a unit with no enable.
- [x] `backend/src/models/latch.c` and `dff.c`: the same for a unit whose
      device or unit names `outputEnable`: its Q / QN output pins.
- [x] Extend `erc_pins_test`: `SN74HC244N`, `SN74HC240N`, `SN74HC541N`,
      `SN74HC125N` (buffer), `SN74HC373N` (latch), `SN74HC574N`, `SN74LS374N`
      (dff): every `output`-role pin → tri_state, every `input` pin → input.
- [x] New `backend/tests/erc_pins_check.c` (`logic_test(erc_pins_check)`),
      built like `listing_check.c`: loads `PARTS_DIR`, and for every part writes
      `<name>|<kind>|<letters>` where `letters` has one character per pin in
      pin order, from `i o b t p f u P O c e n` for the twelve types in
      `ErcPinType` order and `-` for an absent pin; compares with
      `tests/golden/erc_pins.txt`; `--print` writes the file (like
      `listing_check --print`).
- [x] Generate and review `backend/tests/golden/erc_pins.txt`: `diff` against
      the table's expectations by kind: no `t` in any kind but buffer, latch, dff;
      no `b` for a connector, header, socket, resistor … part (passive list).
- [x] `docs/backend.md`: a section "ERC pin types" after the voltage check's:
      the table of § 3, the hook, and `erc_pins_check --print >
      backend/tests/golden/erc_pins.txt` beside the other goldens' lines
      (line 780 on).

Acceptance: all 2950 parts of the three kinds answer by their data; the golden is the library.
  Check: `cmake --build /tmp/logic-build --target erc_pins_test erc_pins_check listing_check trace_check && /tmp/logic-build/erc_pins_test && /tmp/logic-build/erc_pins_check && /tmp/logic-build/listing_check | tail -1 && /tmp/logic-build/trace_check | tail -1` → all pass, the last two as before (est. 8 min; `trace_check` runs the engine over every part, which is why this is the one check that proves no model changed).
  Result (recorded when run): the golden lines with a `t`, per kind, `awk -F'|' '$3 ~ /t/ {c[$2]++} END {for (k in c) print k, c[k]}' backend/tests/golden/erc_pins.txt` → `buffer 1324`, `dff 238`, `latch 623` (no other kind). Checks (2026-11-25, `/tmp/logic-build-wt63`): `erc_pins_test: all passed` (7 tri-state parts, 3 roles each); `erc_pins_check: 40174 parts match the golden`; `listing_check` and `trace_check` last line `40174 parts compared, 0 skipped` (trace_check 8.4 s); `voltage_test: all passed`; passive-kind lines with a non-passive type: 0 (`awk` over the golden). `dff` and `latch` equal their parts that name an output enable (238, 623); `buffer` 1324 of the 2089 that name one (the rest are transceiver, registered and level-translator parts, whose ports are `inout`).
Commit: 78440066

## Validation Plan

- Tests: `erc_pins_test` (named parts, both phases), `erc_pins_check` (the 40174-line golden).
- Coverage check: `backend/CMakeLists.txt` lists `erc_pins.c` in the library and both tests as `logic_test`; `ctest --test-dir /tmp/logic-build -R erc_pins` lists 2 tests.
- Runtime proof: none beyond the tests (nothing calls it yet).
- Doc sync: `docs/backend.md` (Phase 2).
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/logic-build -R 'erc_pins|voltage|listing|trace'` → 100% passed (est. 8 min). Result (2026-11-25, `/tmp/logic-build-wt63`): `100% tests passed, 0 tests failed out of 5` (12 s); and the app builds with `erc.h` in the bridging header: `xcodebuild build` → `** BUILD SUCCEEDED **`.

## Decisions

- Tri-state comes from a per-kind data hook, not from names — decided by the author, 2026-11-25: a design with a shared bus must not report conflicts between 3-state outputs (KiCad's own pin map), so the type must be exact.
- A supply symbol is `power_in` and drives its own net — decided by the author, 2026-11-25 (§ 3).
- `free`, `unspecified` and `open_emitter` are never returned — decided by the author, 2026-11-25: no data.

## Corrections

1. Phase 1's tick marks and Result landed in the commit after its code (4b64f493): a compound shell command that did both was refused by the worktree guard and the commit ran without them. No code differs from what the plan said.
2. § 2 said the hook is declared in `backend/include/parts.h` "beside `part_pin_open_drain`"; `part_pin_open_drain` is declared in `backend/src/models/model_pins.h:117`, so `part_pin_tristate` is declared there.
3. The buffer hook is on `BufModel` as a new last field `is_tristate` (the `tristate` model and, forwarded, the `translator`), and the latch hook on `LatchModel.tristate` (the `transparent` and `sr` models); the plan said "buffer_reg.c" and "the `latch` and `dff` kinds' `outputEnable`" generally. `buffer_reg.c`'s registered model has only bidirectional ports (`inout` roles), so it needs no answer and is unchanged; the latch kind's `addressable`, `bcd7seg` and `bus-mux` models have no output enable on a `q`/`out` pin and answer 0.

## Summary

The risk is the three hooks: a pin set that differs from what the model
releases. The golden makes every change visible and `trace_check` shows no model
moved. Untouched: every simulation, every part file, the voltage check.
