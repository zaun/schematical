# plan-10-B: The `latch` kind, shared sequential helpers, latch models

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-A

B adds the first of plan-10's three kinds. It writes the helpers every new
model shares (`backend/src/models/seq.{h,c}`), the test bench the three model
tests share (`backend/tests/support/bench.{h,c}`), the `latch` kind with its
five models, and `latch_test`, which proves each model against the function
tables of the devices that will use it — on inline fixture devices, because
no latch part exists until C.

Outcome of B: `latch_test` passes every case of §6; `ctest` 13 of 13; the
goldens are unchanged (no latch part yet).

References:

- plan-10-A §3 (the whole design: kinds, models, §3.4 data conventions, §3.5
  signal rules) — read first.
- `plan-10-data/latch-a.md`, `latch-b.md` (the behavior every case of §6
  encodes).
- `backend/src/models/dff.c` (the model this one mirrors: key parsing,
  timing entries, change-suppressed inertial outputs), `backend/src/library/kind.h`,
  `backend/tests/sram_test.c` (the bench pattern `bench.c` generalizes).

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-A complete | `ls planning/completed/plan-10-A-groundwork.md` → exists | MET (2026-11-01: exists, archived at 2183572) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p10` → `100% tests passed, 0 tests failed out of 13`
  (A's 12 + `latch_test`).
- `latch_test` prints one `PASS` line per case of §6 and no `FAIL`.
- `git diff --stat backend/tests/golden` → no change.
- `docs/parts-format.md` has a § `latch` and § "Sequential kinds" (§4);
  `docs/backend.md` lists `latch.c`, `seq.c`, `latch_test`.

### Non-goals (explicit constraints)

- No existing part's listing line or trace hash changes (no golden diff).
- No public API beyond `PART_LATCH` (appended to `PartKind` after
  `PART_EEPROM`); `Part.logic` is reused, no new info struct.
- No part file: `latch.json` is C's.

## 2. Current State

- Kinds are registered in `KINDS[]` (`backend/src/library/kinds.c`), 10 today.
  `PartKind` ends `… PART_SUPPLY, PART_EEPROM` (`parts.h`).
- Test support names kinds in two places: `tests/support/listing.c`
  (`listing_kind_name`, `print_details`; the `logic` test on
  `PART_GATE || PART_DFF || PART_JKFF` decides whether the family column is
  printed) and `tests/trace_check.c:96,163` (`per_kind[PART_EEPROM + 1]`,
  `i <= PART_EEPROM`).
- The engine API the models use: `sim_read_pin`, `sim_schedule_inertial_drive`,
  `sim_schedule_timer`, the `pin_changed`/`timer`/`start` callbacks
  (`backend/src/engine/sim.h`).
- Each model test today builds its own bench `SimComponentType` that drives the
  part's input pins (`sram_test.c:38`, `eeprom_test.c:95`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Latch devices per model (data in C) | transparent 17, addressable 6, sr 3, bcd7seg 1, bus-mux 1 | §5 tables; device ids from `cut -f2 planning/plan-10-data/parts.tsv` |
| Largest unit count (transparent) | 32 (`74_32373`) | `latch-a.md` § 74_32373 |
| Largest pin count of a latch device | 96 (`74_32373`, NMJ) | `parts.tsv` `pins` column |

## 3. Design Overview

```
backend/src/models/seq.h, seq.c     shared helpers (§4.2)
backend/src/models/latch.c          the kind: dispatch, the five models (§5)
backend/tests/support/bench.h, .c   pin-driving bench for model tests (§6.1)
backend/tests/latch_test.c          fixture devices + cases (§6)
```

The kind's `load_device` reads `"model"`, finds it in a static table, checks
the device's keys against that model's list, and calls the model's loader.
`bind_pinout` and `load_part` dispatch the same way. The kind's single
`SimComponentType` forwards `create`/`start`/`pin_changed`/`timer`/`destroy`
to the model through `part_internal(part)->device`. Rejected: one
`PartKindOps` per model (five kinds named `latch-…` in `KINDS[]`, five listing
cases, five app cases, for one drawer leaf).

Correctness risk: the transparent model carries 17 devices and 640 parts
(§5.1); its LE/OE/PRE/CLR combinations get one case per function-table row.

## 4. Detailed Design — shared pieces

### 4.1 Format: what every sequential kind shares (`docs/parts-format.md`)

A new section "Sequential kinds (`latch`, `counter`, `shift`)" documents:

- Device key `"model"`: required string, one of the kind's models. Each model
  lists its own device keys; a key of another model is an error
  (`"<key>" is not a key of model "<model>"`).
- Device key `"timing"`: logic family → entry `{rise, fall}` (ns, the model's
  main path) plus optional `note`, `vcc`, `inputMax` and the model's extra
  timing keys. The family must be one `kind_logic_family` accepts.
- Part keys: `"family"` (a key of `timing`), optional `"vcc"`, `"inputMax"`
  (as on `dff`). `Part.logic` = {family, units}; each model defines `units`.
- **Control**: an object `{"pin": <name>, "active": "high"|"low"}`; the name
  is exactly one pin in every pinout. Models that take a control state whether
  it is asynchronous (level-acting) or synchronous (sampled at a clock edge);
  where a model offers both, the control has `"when": "async"|"clock"`.
- The signal rules of plan-10-A §3.5, stated once here.

### 4.2 `seq.h` / `seq.c`

```c
#define SEQ_NO_PIN ((size_t)-1)

typedef struct { const char *pin; int active_high; int clocked; } SeqControlName; /* load time */
typedef struct { size_t pin; int active_high; int clocked; } SeqControl;           /* bound; pin SEQ_NO_PIN = none */

typedef struct {                    /* one timing entry */
    const char *family;
    SimTime rise, fall;
    const cJSON *json;              /* for the model's extra keys */
    const PartVoltage *vcc;
    PartLogic logic;
} SeqTiming;

typedef struct { SeqTiming *entries; size_t count; } SeqTimingTable;

typedef struct { uint64_t value, known; } SeqBits;   /* bit i known when known>>i & 1 */

typedef enum { SEQ_PUSH_PULL, SEQ_OPEN_DRAIN } SeqOutputStyle;

typedef struct {                    /* one output pin's drive state */
    size_t pin;
    int scheduled;
    LogicValue driven;
} SeqOutput;
```

Functions (each returns 0 / -1 with `kind_error` set, unless stated):

| Function | Does |
|---|---|
| `seq_check_model_keys(ctx, device_json, model_name, model_keys)` | every key of the device that is not a library device key (`id function description drawer note pinouts parts`) nor `model`/`timing` must be in `model_keys` |
| `seq_timing(ctx, device_json, extra_keys, units, &table)` | parses `"timing"` (as `dff_load_device` does: duplicate families, `kind_logic_family`, `rise`/`fall` via `kind_ns`, `note` a string, `vcc`/`inputMax` via `kind_voltage`, then `extra_keys` allowed); sets each entry's `logic` = {family, units} |
| `seq_timing_extra(ctx, entry, key, required, &ps)` | an extra timing key in ns → ps |
| `seq_part(ctx, &table, part_json, out, &entry)` | reads `"family"`, finds its entry, sets `out->logic`, resolves `out->vcc` with `kind_part_voltage`; error `family "<f>" has no timing entry` |
| `seq_control_name(ctx, json, key, required, allow_when, &name)` | parses a control object |
| `seq_bind(ctx, pins, count, name_or_NULL, &index)` / `seq_bind_control(...)` | name → pin index via `kind_pin` (NULL → `SEQ_NO_PIN`) |
| `seq_names(ctx, json, key, min, max, &names, &count)` | an array of pin names |
| `int seq_level(LogicValue v)` | 1, 0, or -1 (X or Z) |
| `int seq_async(sim, inst, const SeqControl *c)` | asserted 1 / 0 / -1; Z → 0; `SEQ_NO_PIN` → 0 |
| `int seq_sync(sim, inst, const SeqControl *c)` | asserted 1 / 0 / -1; Z → -1; `SEQ_NO_PIN` → 0 |
| `int seq_enable(sim, inst, const SeqControl *c)` | output enable: 1 enabled / 0 disabled / -1 unknown (X or Z); `SEQ_NO_PIN` → 1 |
| `int seq_rising(LogicValue old, LogicValue new)`, `seq_falling` | Verilog posedge / negedge |
| `LogicValue seq_bit(SeqBits b, unsigned i)` | 0, 1 or X |
| `void seq_drive(sim, inst, SeqOutput *o, LogicValue v, SeqOutputStyle s, const SeqTiming *t)` | applies open drain (1 → Z), skips when `o->scheduled && o->driven == v'`, else `sim_schedule_inertial_drive` with fall for 0, rise for 1/Z, the longer for X |
| `LogicValue seq_gate(LogicValue v, int enable)` | enable 1 → v, 0 → Z, -1 → X |

### 4.3 Test support

- `listing.c`: `listing_kind_name` → `"latch"` for `PART_LATCH`; `print_details`
  → `units=%u` (as gate/dff/jkff); the `logic` condition includes `PART_LATCH`.
- `trace_check.c`: a file-level `#define TRACE_LAST_KIND PART_LATCH` replaces
  the two `PART_EEPROM` uses (D and G move it to `PART_COUNTER`,
  `PART_SHIFT`).
- `model_pins.h` `part_pin_open_drain`: `PART_LATCH` → the model's answer
  (§5: open-drain devices answer 1 for their output pins).

## 5. Detailed Design — the `latch` kind

`PartKindOps LATCH_KIND = {PART_LATCH, "latch", &LATCH_MODEL, DEVICE_KEYS,
PART_KEYS, …}`, where `DEVICE_KEYS` is `model`, `timing` and the union of the
model keys below, and `PART_KEYS` is `family vcc inputMax`. Registered in
`KINDS[]` after `EEPROM_KIND`.

### 5.1 Model `transparent` (17 devices, 640 parts in C)

Device keys:

| Key | Value |
|---|---|
| `"units"` | 1–40 objects: `d`, `le` (required pin names), `q`, `qn`, `oe`, `pre`, `clr` (optional; at least one of `q`/`qn`). Units may share pins (a common LE or OE). |
| `"enableActive"` | `"high"` or `"low"`: the `le` level that makes a unit transparent. Required unless `"polarity"`. |
| `"polarity"` | optional pin name (CD4042B): a unit is transparent while its `le` level equals this pin's level. Not with `"enableActive"`. |
| `"outputEnableActive"` | `"high"`/`"low"`: the `oe` level that enables the outputs. Required when any unit has `oe`. |
| `"presetActive"`, `"clearActive"` | `"high"`/`"low"`; required when any unit has `pre` / `clr`. |
| `"presetWins"` | bool; required when some unit has both: true → both asserted store 1, false → 0. |
| `"output"` | `"push-pull"` (default) or `"open-drain"`. |

Behavior, per unit, evaluated on any change of its pins: `clr`/`pre`
asynchronous (§3.5); else transparent → stored = D; LE unknown and D ≠ stored
→ X; else hold. `q` = stored, `qn` = NOT stored, both through `seq_gate`
with the unit's `oe`. OE gates outputs only (the latch keeps working). Main
timing path: D → Q. `units` (PartLogic) = number of units.

Devices (C writes them from the spec sections):

| Device | Units / controls |
|---|---|
| `74_373`, `74_573` | 8: nD→nQ, LE, OE; `enableActive` high, `outputEnableActive` low |
| `74_533`, `74_563`, `74_580` | as 373/573, outputs as `qn` (the inverted pins, named as in the spec section) |
| `74_841` | 10, as 373 |
| `74_843` | 9, plus `pre` PRE and `clr` CLR (both active low, `presetWins` true) |
| `74_1G373` | 1: D, LE, Q, OE |
| `74_16373`, `74_162373` | 16: units 1–8 use 1LE/1OE, 9–16 use 2LE/2OE |
| `74_32373` | 32: four banks of 8 (xLE, xOE) |
| `74_16841`, `74_162841` | 20: two banks of 10 |
| `74_75`, `74_375` | 4: `q` and `qn`; latches 1–2 share one enable, 3–4 the other; `enableActive` high; no OE |
| `4042B` | 4: `q`, `qn`, common CLOCK as `le`, `"polarity": "POLARITY"` |
| `4508B` | 8: two halves (STROBE A/B as `le`, RESET A/B as `clr` with `clearActive` high, DISABLE A/B as `oe` with `outputEnableActive` low) |

### 5.2 Model `addressable` (6 devices, 62 parts)

| Key | Value |
|---|---|
| `"address"` | 1–3 pin names, LSB first |
| `"data"` | pin name |
| `"outputs"` | 2^len(address) pin names; `outputs[i]` is latch i |
| `"enable"` | control: the addressed latch is transparent while it is asserted |
| `"clear"` | control |
| `"output"` | `"push-pull"` (default) / `"open-drain"` |
| `"invert"` | bool, default false: the pin shows NOT the bit |

Behavior, per latch i (the TI gate-level reading in `latch-b.md` § 74_259 and
§ TPIC6259): enable asserted and address = i → bit = D; else clear asserted →
0; else hold. Unknown address bits make every latch the address could select
behave as if enable were unknown: bit becomes X unless it already equals D.
Unknown enable: the addressed latch → X unless equal to D. Unknown clear: a
latch not being written whose bit is 1 → X. Pin = bit (or NOT bit), open-drain
drives 0 or releases. Main path: D → Q. `units` = 2^len(address).

| Device | Keys |
|---|---|
| `74_259` | enable G active low, clear CLR active low, push-pull |
| `4099B`, `4724B` | enable WD active low, clear RESET active high, push-pull |
| `TPIC6259`, `TPIC6A259`, `TPIC6B259` | enable G active low, clear CLR active low, `open-drain`, `invert` true (DRAIN on when the bit is 1) |

### 5.3 Model `sr` (3 devices, 34 parts)

| Key | Value |
|---|---|
| `"units"` | 1–8 objects: `s` (array of 1–2 pin names, any asserted sets), `r`, `q` |
| `"inputsActive"` | `"high"` / `"low"` (S and R) |
| `"both"` | `"set"` / `"reset"`: Q while S and R are both asserted |
| `"outputEnable"` | optional control (common) |

Behavior: set and not reset → 1; reset and not set → 0; both → per `"both"`;
neither → hold; unknown inputs → X where the result could differ. Release
from both-asserted is the hold of the last value (S released first with R
still asserted gives 0 — the 279's gate behavior). Main path: S → Q. `units`
= number of units.

| Device | Keys |
|---|---|
| `74_279` | 4 units (latches 1 and 3 with two S pins), `inputsActive` low, `both` set |
| `4043B` | 4 units, `inputsActive` high, `both` set, `outputEnable` ENABLE active high |
| `4044B` | 4 units, `inputsActive` low, `both` reset, `outputEnable` ENABLE active high |

### 5.4 Model `bcd7seg` (1 device, 12 parts)

| Key | Value |
|---|---|
| `"inputs"` | 4 pin names, LSB first |
| `"latch"` | control: the code is held while asserted (4511: LE active high) |
| `"lampTest"`, `"blank"` | controls |
| `"segments"` | 7 pin names, a … g |
| `"glyphs"` | 16 strings of 7 characters `0`/`1` (segments a…g) for codes 0–15 |

Behavior: stored code follows the inputs while `latch` is not asserted; lamp
test asserted → every segment 1; else blank asserted → every segment 0; else
the glyph of the stored code. Any unknown bit in the code or an unknown
lamp-test/blank → the segments whose value could differ go X. Main path:
input → segment. `units` = 1.

`74_4511`: inputs D0–D3, `latch` LE active high, `lampTest` LT active low,
`blank` BL active low, `glyphs` = `latch-b.md` § 74_4511 table (6 and 9
without tails, 10–15 blank).

### 5.5 Model `bus-mux` (1 device, 2 parts)

| Key | Value |
|---|---|
| `"a"`, `"b1"`, `"b2"` | 12 pin names each (inout) |
| `"select"` | pin name: high → A shows the 1B latch, low → the 2B latch |
| `"latchEnables"` | object `{"b1", "b2", "a1", "a2"}` → pin names (LE1B, LE2B, LEA1B, LEA2B; transparent while high) |
| `"outputEnables"` | object `{"a", "b1", "b2"}` → pin names (OEA, OE1B, OE2B; enabled while low) |

Behavior (`latch-b.md` § 74_16260): latch L1B takes pin 1Bn, L2B takes 2Bn,
LA1B and LA2B take An; An = OEA low ? (SEL ? L1B : L2B) : Z; 1Bn = OE1B low ?
LA1B : Z; 2Bn = OE2B low ? LA2B : Z. Latches read the pin's net value (the
part's own drive included, as in the hardware). Main path: B → A. `units` = 12.

## 6. `latch_test`

### 6.1 `tests/support/bench.{h,c}`

A bench for any part: one net per pin, the part's instance, and a driver
instance on every input and inout pin. API:

```c
Bench *bench_create(const Part *part);              /* power pins driven 1, ground 0 */
void bench_set(Bench *b, const char *pin_name, LogicValue v);  /* drive strong now (Z = release) */
void bench_run(Bench *b, SimTime ps);               /* advance */
LogicValue bench_get(Bench *b, const char *pin_name);          /* the net's value */
void bench_destroy(Bench *b);
```

`bench_set` names pins by `PartPin.name` (the first pin of that name).
`latch_test`, `counter_test` and `shift_test` use it.

### 6.2 Fixtures

`latch_test.c` holds one inline part file (kind `latch`, the `eeprom_test`
single-quote style) with one fixture device per model row below, each a copy
of the real device's pinout and keys from the spec section, timing
`{"rise": 10, "fall": 10}` for family HC, and one part (`FX-<device>`). The
cases check levels 50 ns after each step.

### 6.3 Cases

| # | Fixture | Case |
|---|---|---|
| 1 | 74_373 | power-on: all Q = 0 |
| 2 | 74_373 | LE high: Q follows D (each bit) |
| 3 | 74_373 | LE falls: Q holds while D changes |
| 4 | 74_373 | OE high: Q = Z; LE high meanwhile loads; OE low shows the new data |
| 5 | 74_373 | LE = X with D ≠ stored → Q = X; with D = stored → Q unchanged |
| 6 | 74_373 | OE = X → Q = X; OE = Z → Q = X |
| 7 | 74_533 | outputs are NOT D |
| 8 | 74_843 | CLR low → 0 regardless of LE; PRE low → 1; both low → 1 (`presetWins`) |
| 9 | 74_16373 | bank 1 LE does not move bank 2 |
| 10 | 74_75 | Q and QBAR complementary; pair enables independent |
| 11 | 4042B | POLARITY 0: transparent at CLOCK 0, latched at 1; POLARITY 1 the reverse; POLARITY change with CLOCK steady opens/closes |
| 12 | 4508B | RESET high → 0 and clears the stored bit; DISABLE high → Z with storage intact |
| 13 | 74_259 | addressable mode: only the addressed latch follows D |
| 14 | 74_259 | memory mode: G high holds all |
| 15 | 74_259 | demux mode: CLR low, G low → addressed = D, others 0 |
| 16 | 74_259 | clear mode: CLR low, G high → all 0 |
| 17 | 74_259 | address change with G low moves the transparent latch; the old one keeps its value |
| 18 | 4099B | RESET high + WD high clears all; RESET high + WD low = demux |
| 19 | TPIC6259 | bit 1 → DRAIN 0; bit 0 → DRAIN Z (open drain, inverted) |
| 20 | 74_279 | set, reset, both low → 1, release S first → 0, release R first → 1 |
| 21 | 4043B | S=R=1 → 1; ENABLE 0 → Z while S/R still act |
| 22 | 4044B | S=R=0 → 0 |
| 23 | 74_4511 | codes 0–15 → the glyph table; LT low → all 1; BL low → all 0; LT beats BL |
| 24 | 74_4511 | LE high holds the code while inputs change |
| 25 | 74_16260 | A→B: LEA1B high loads 1B latch from A; OE1B low drives 1B |
| 26 | 74_16260 | B→A: SEL selects 1B/2B latch onto A when OEA low |
| 27 | loader | `"model": "nope"` → `unknown model "nope"`; a `transparent` device with `"address"` → `"address" is not a key of model "transparent"`; `enableActive` with `polarity` → error |
| 28 | voltage | `part_pin_open_drain(FX-TPIC6259, DRAIN0 index)` → 1; for FX-74_259 Q0 → 0 |

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — Shared helpers and bench

- [x] `seq.h`, `seq.c` (§4.2), listed in `add_library(logic …)` in
  `backend/CMakeLists.txt`; `tests/support/bench.{h,c}` (§6.1) added to
  `logic_test_support` there. (Beyond §4.2's table, `seq.h` also has
  `seq_active`, `seq_optional_name`, `seq_output_style`, `seq_optional_bool`,
  `seq_read`, `seq_set_bit`, `seq_not`, `seq_value`; `bench.h` adds
  `bench_error`.)
- [x] `docs/parts-format.md` § "Sequential kinds" (§4.1).

Acceptance: `cmake --build /tmp/logic-p10` succeeds; `cc -std=c11 -Wall -Wextra -fsyntax-only -Ibackend/include -Ibackend/src/engine -Ibackend/src/library -Ibackend/src/third_party/cjson backend/src/models/seq.c` prints nothing (the project sets no warning flags, so this is checked by hand); `ctest` 12 of 12 (seconds). (Ran: build clean; the `cc -fsyntax-only`
line on `seq.c` printed nothing, same for `bench.c`; `100% tests passed, 0
tests failed out of 12`.)
Commit: 712f5f3

### Phase 2 — The `latch` kind and models

- [x] `parts.h`: `PART_LATCH` after `PART_EEPROM`.
- [x] `models/latch.c`: kind, dispatch, the five models (§5); listed in
  `backend/CMakeLists.txt`; `kinds.c` registration; `model_pins.h` answer
  (`LATCH_KIND.pin_open_drain`, plan-10-A's per-kind member). The
  `cc -Wall -Wextra -fsyntax-only` line on `latch.c` prints nothing.
- [x] Test support (§4.3). (`listing.c`: name, `units=`, family column;
  `trace_check.c`: `TRACE_LAST_KIND`.)
- [x] `docs/parts-format.md` § `latch` (§5 key tables and behavior);
  `docs/backend.md` layout and tests lists.

Acceptance: `ctest` 12 of 12, golden diff empty (seconds). (Ran: `100% tests
passed, 0 tests failed out of 12`; `git status --short backend/tests/golden`
→ no output.)
Commit: —

### Phase 3 — `latch_test`

- [ ] `backend/tests/latch_test.c` with the fixtures (§6.2) and the 28 cases
  (§6.3); `logic_test(latch_test)`.

Acceptance: `/tmp/logic-p10/latch_test` → 28 `PASS`, 0 `FAIL`;
`ctest` 13 of 13 (seconds).
Commit: —

## Validation Plan

Phase 3's acceptance is the gate (latch_test plus the 12 existing tests,
about 5 s). The landing tier is plan-10-J's.

## Decisions

- One kind with model dispatch (§3), the `cpu` precedent.
- Shared helpers live in `models/seq.c`, not in `kind_util.c`: they are model
  code (engine reads, drives), not loader code.
- The model tests use fixtures copied from the real devices, so B can land
  before the part data; C re-runs a subset on real parts.

## Corrections

(none yet)

## Summary

B adds the `latch` kind with five models and the shared sequential helpers,
proven by 28 function-table cases on fixtures; no part data yet.
