# plan-07-A: Voltage keys in the part format, loader and structs; six supply voltages

Last updated: 2026-11-01
Overall Effort: huge (>3d)
Effort: large (3h–1d)
Depends on: nothing

plan-07 gives every part a voltage range and every supply a voltage, and
refuses connections whose voltages don't fit. The simulation stays
logic-only (0/1/X/Z). The voltage check is a separate static check over the
netlist. It runs in the editor before a wire or switch change is accepted,
at `circuit_start`, and when a switch closes during a run.

This sub-plan builds the data model, with no check and no part data yet:

- the new JSON keys (`vcc`, `inputMax`, `vf`, `ratedVolts`, `volts`);
- their C structs on `Part`;
- the listing fields;
- the docs;
- the supply parts `5V`, `3V3`, `2V5`, `1V8`, `1V2` and `12V`, next to
  today's `VCC` and `GND`.

The voltage keys are optional while sub-plans C–E fill in the data.
Sub-plan F makes them required.

Outcome: `ctest` passes. `listing_check` lists the six new supplies, and
the supply lines show `mv=`. A part file with `"vcc": [2, 6]` on an HC
timing entry loads, and listing prints `|vcc=2000-6000,in=6000` on that
entry's parts.

References:

- `docs/parts-format.md` (whole file; § Kinds, § Values, § Loading rules)
- `docs/backend.md` § Tests (golden regeneration)
- `backend/include/parts.h`, `backend/src/library/library.c`,
  `backend/src/library/kind_util.c`, `backend/src/library/kind.h`
- `AGENTS.md` § Datasheets (goldens after part changes)

### The plan-07 sub-plans

| Letter | Content | Effort |
|---|---|---|
| A | format, structs, loader, listing, docs, the six new supplies | large |
| B | the voltage check (`voltage.h`), circuit start/switch integration | large |
| C | gate data: AND and NAND devices | large |
| D | gate data: OR, NOR, XOR and XNOR devices | large |
| E | data: dff, jkff, SRAM, EEPROM | large |
| F | data: oscillator, 7-seg, switch; the keys become required | medium |
| G | app: VCC voltage dropdown, editor refusal, voltages shown, `VCC` → `5V` | large |

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Backend builds and all tests pass on `main` | `cmake -S backend -B /tmp/p07 && cmake --build /tmp/p07 -j8 && ctest --test-dir /tmp/p07` → `100% tests passed, 0 tests failed out of 11` | MET (2026-11-01, re-run in worktree-P-07: 11/11, 2.88 s) |

Sub-plans A–F have no other prerequisite. Sub-plan G has one more (plan-06
complete), stated in G.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- The loader accepts and stores these keys:
  - the supply range `vcc: [min, max]` and an optional `inputMax`, on the
    places listed in §4.1;
  - `vf` on 7-segment displays;
  - `ratedVolts` on switches;
  - `volts` on supplies.
- Each key fails with a precise message when its value is out of range.
- Parts expose the values through `Part.vcc`, `PartDisplay`, `PartSwitch` and
  `PartSupply`.
- The supply library has eight parts: `VCC` (5 V), `GND` (0 V), `5V`, `3V3`,
  `2V5`, `1V8`, `1V2` and `12V`.

### Non-goals (explicit constraints)

- No change to any simulated behavior. `traces.txt` gains exactly six lines,
  one per new supply part, and no existing line changes.
- No part data beyond supplies (sub-plans C–F).
- No voltage check (B) and no app change (G). `PartSupply.level` stays, so
  `PartCatalog.swift:317` and `supply.c:58` keep working unchanged.
- `VCC` stays in the library until G.
- Values are volts in JSON and millivolts (`uint32_t`) in C, the same
  pattern as ns → ps (`docs/parts-format.md` § Values).

## 2. Current State

- `Part` (`backend/include/parts.h:85-101`) has no voltage. The structs are:
  - `PartSupply { int level; }` (`:58`)
  - `PartSwitch { unsigned positions; const char *actuator; }` (`:46`)
  - `PartDisplay` (`:49-55`)
  - `PartLogic { family; units }` (`:44`)
- The common key lists are in `library.c:184-189`:
  `DEVICE_KEYS = {id,function,description,drawer,note,pinouts,parts}` and
  `PART_KEYS = {name,package,pinout,flags,footprint}`. A kind adds its own
  keys through `PartKindOps.device_keys` / `.part_keys` (`kind.h:32-33`).
  `kind_check_keys` (`kind_util.c:118-139`) rejects any key not listed.
- Timing entries and grades:
  - gate: `load_delays` uses `KEYS = {rise,fall,note}` (`gate.c:75-88`); a
    split entry uses `SPLIT_KEYS = {otherLow,otherHigh,note}` (`gate.c:91`).
    A part's `logic` points at its family entry's `PartLogic` (`gate.c:225,277`).
  - dff `TIMING_KEYS = {rise,fall,clearWins,note}` (`dff.c:90`); jkff
    `{rise,fall,note}` (`jkff.c:70`).
  - SRAM/EEPROM grades use a `TIMING_NAMES` array that serves both as the key
    list and as the timing field loop (`sram.c:747-751,813`;
    `eeprom.c:866-868`). New keys go after the timing names, next to `"note"`,
    which the index loop never reaches.
- Supply: `kind_uint(ctx, json, "level", 1, &level)` (`supply.c:18-21`).
  The drive is `level ? LOGIC_1 : LOGIC_0` (`supply.c:58`).
- Number helpers:
  - `kind_number` reads any JSON number; the caller checks the range
    (`kind.h:102`, `kind_util.c:192`).
  - Optional keys are tested with `cJSON_GetObjectItemCaseSensitive(...) != NULL`
    first (precedent `gate.c:193`).
- Listing (`tests/support/listing.c:98-135`) prints, in order:
  `name|kind|package|drawer|function|family|flags|details|pins|footprint=ID`.
  The supply details are `level=%d` (`:87`). `golden/parts.txt` has 4006
  lines (`wc -l` → 4006).
- `trace_check` hashes by part name and simulated outputs, so a non-behavior
  field leaves `traces.txt` unchanged. A new part is an "extra part" in both
  goldens.

### Measured populations

| What | Count | Command |
|---|---|---|
| parts in the library | 4006 | `wc -l backend/tests/golden/parts.txt` → 4006 |
| supply devices / parts | 2 / 2 | python census over `supply.json` → `supply 2 2` |
| gate timing entries that are split (otherLow/otherHigh) | 4 | python: entries with `otherLow` → `split entries 4` |
| library_check fixtures using `'level'` | 12 | `grep -c "'level'" backend/tests/library_check.c` → 12 |
| devices with more than one power-pin name | 0 | python census over all part files → `multi-power-name devices: 0` |

### Verified properties

- **No dual-supply parts.** No device has two power-pin names (census above),
  so one `vcc` range per part is enough. Power-pin names in use are `VCC`,
  `VDD` and `Vcc`.
- **The range belongs to the part, not the family.** It was measured from the
  datasheets (`pdftotext -layout`):
  - CD74AC00 is 1.5–5.5 V, while SN74AC00 is 2–6 V.
  - SN54LS00 is 4.5–5.5 V, while SN74LS00 is 4.75–5.25 V.
  - SN54LVC00A is 2–3.6 V, while SN74LVC00A is 1.65–3.6 V.
  - LMK6 oscillator parts are 1.8 V or 2.5–3.3 V by a letter in the part
    number.

  So a range is set on the family entry, grade or device and can be
  overridden per part.

## 3. Design Overview

The work splits into four independent pieces:

1. **Keys and parsing.** One shared helper parses `vcc` and `inputMax` for
   every place they may appear. Kind-specific keys are `vf`, `ratedVolts`
   and `volts`.
2. **Structs.** A `PartVoltage` on `Part`, plus fields on `PartDisplay`,
   `PartSwitch` and `PartSupply`.
3. **Listing.** The new fields go into the golden lines. This is the
   regression net for sub-plans C–F.
4. **Supplies.** Six new devices in `supply.json`.

**Correctness risk** is in two places:

- **The inheritance rules (§4.2).** A wrong merge silently gives parts the
  wrong limits. Every rule has a `library_check` case.
- **The SRAM/EEPROM `TIMING_NAMES` index loop.** A new key placed before
  `"note"` would be read as a timing value. The key goes after it, and a
  test loads a grade with both keys.

**Gate class.** This is not byte-identity work. `parts.txt` is expected to
change:

- the two supply lines change (`level=1` → `level=1 mv=5000`);
- six supply lines are added.

`traces.txt` is expected to gain exactly the six new supply lines. A diff
in any other existing line of either golden is a bug to root-cause.

**Rejected alternatives**

- **One `vcc` per logic family, globally.** The datasheets differ within a
  family (§2, Verified).
- **`vcc` only on parts (4006 entries).** Most parts of an entry share it.
  Inherit, and override where they differ.
- **A float `double` in C.** Millivolt integers compare exactly and print
  stably in goldens, the same as ps.
- **Replacing `PartSupply.level`.** `level` stays, derived as `mv > 0`, so
  the model and the app don't change in this sub-plan.

## 4. Detailed Design

### 4.1 Keys

All values are JSON numbers in volts. The loader converts them with
`llround(v * 1000)` to millivolts.

| Key | Where | Value | C field |
|---|---|---|---|
| `"vcc"` | gate/dff/jkff timing entry (top level of the entry, also on split entries next to `otherLow`/`otherHigh`); sram/eeprom grade; oscillator device; and as a **part** key on gate, dff, jkff, sram, eeprom and oscillator parts | `[min, max]`, 0 < min < max ≤ 30 | `Part.vcc->min_mv`, `->max_mv` |
| `"inputMax"` | wherever `vcc` may appear | number, max ≤ inputMax ≤ 30 (checked on the resolved values) | `Part.vcc->input_max_mv` |
| `"vf"` | 7seg device | `[min, max]`, 0 < min ≤ max ≤ 10; min is the datasheet minimum, or its typical value when no minimum is printed | `PartDisplay.vf_min_mv`, `.vf_max_mv` |
| `"ratedVolts"` | switch device | number, 0 < v ≤ 1000: the **switching** rating, the lower of the two ratings | `PartSwitch.rated_mv` |
| `"volts"` | supply device; **replaces `"level"`** | number, 0 ≤ v ≤ 30 | `PartSupply.mv`; `PartSupply.level = mv > 0` |

Error messages follow the loader's style, for example
`gate.json: devices[0] (74_00): timing: HC: "vcc": min must be below max`,
`…: parts[3] (X): "inputMax" 3.3 is below the "vcc" max 3.6`, and
`supply.json: devices[0] (VCC): "volts" must be from 0 to 30`.

### 4.2 Resolution (gate, dff, jkff, sram, eeprom, oscillator)

1. The base comes from the part's family timing entry (logic), its grade
   (memory) or its device (oscillator): `vcc` and `inputMax`, each optional.
2. A part's own `vcc` replaces the base range. A part's own `inputMax`
   replaces the base input max. Each is optional.
3. A part with no `vcc` from either level gets `Part.vcc = NULL`, which is
   allowed until sub-plan F.
4. A part with an `inputMax` but no `vcc` anywhere is an error:
   `"inputMax" without "vcc"`.
5. `input_max_mv` is the resolved `inputMax` if there is one, otherwise
   `max_mv`. It must be ≥ `max_mv`.
6. Storage:
   - A part with no part-level key points `Part.vcc` at its entry's,
     grade's or device's `PartVoltage`, which is allocated once.
   - A part with an override gets its own `PartVoltage`.

Shared helper, `kind_util.c` / `kind.h`:

```c
/* Reads optional "vcc" and "inputMax" from json. On return *out is the
   resolved voltage (base merged with json's keys), base itself when json
   has neither key, or NULL when neither has a range. -1 on error. */
int kind_voltage(KindContext *ctx, const cJSON *json, const PartVoltage *base,
                 const PartVoltage **out);
```

The entry, grade and device level call it with `base = NULL`. The part
level calls it with the entry's result.

### 4.3 C structs (`backend/include/parts.h`)

```c
typedef struct { uint32_t min_mv, max_mv, input_max_mv; } PartVoltage;
typedef struct { int level; uint32_t mv; } PartSupply;          /* level = mv > 0 */
typedef struct { unsigned positions; const char *actuator; uint32_t rated_mv; } PartSwitch;
/* PartDisplay gains: uint32_t vf_min_mv, vf_max_mv;  (0 = not given, until F) */
/* Part gains, after `supply`:
   const PartVoltage *vcc;   recommended supply range; NULL = none
                             (supplies, LEDs, 7-seg, switches; and, until
                             plan-07-F, parts whose data is not filled in) */
```

`PartSwitch.rated_mv = 0` means not given (until F).

### 4.4 Listing (`backend/tests/support/listing.c`)

- After the `footprint=` field (or after `pins` for supplies), parts with
  `vcc` get `|vcc=<min_mv>-<max_mv>,in=<input_max_mv>`.
- Supply details become `level=<level> mv=<mv>` (details fields are space-separated, `listing.c:39-96`).
- 7-seg details end with ` vf=<min>-<max>` when `vf_max_mv > 0`.
- switch details end with ` rated=<mv>` when `rated_mv > 0`.

### 4.5 Supplies (`Parts/supply.json`)

Each new device copies `VCC`'s shape: one pinout `pin` with pin 1 named
`VCC`, role power, and `"drawer": []`. Its single part has the same name
as its `id`.

| id = part name = function | `volts` | description |
|---|---|---|
| `VCC` | 5 | `Supply, 5 V, drives logic 1` |
| `5V` | 5 | `Supply, 5 V, drives logic 1` |
| `3V3` | 3.3 | `Supply, 3.3 V, drives logic 1` |
| `2V5` | 2.5 | `Supply, 2.5 V, drives logic 1` |
| `1V8` | 1.8 | `Supply, 1.8 V, drives logic 1` |
| `1V2` | 1.2 | `Supply, 1.2 V, drives logic 1` |
| `12V` | 12 | `Supply, 12 V, drives logic 1` |
| `GND` | 0 | `Ground, drives logic 0` |

## Compatibility / Format Impact

- **Part format.**
  - Supply `"level"` is replaced by `"volts"`. Part files live only in this
    repo, so all of them change in this sub-plan.
  - Every other new key is optional until F.
  - `"version"` stays `1`. No file outside the repo reads part files.
- **C API.** New fields only. `PartSupply.level` keeps its meaning.
- **Schematic file format.** Unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work. Use `- [~]` for partial and say what remains. A
> moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a
> phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Structs, helper, keys

The loader and structs, with no data change except the supply key.

- [x] `parts.h`: §4.3 structs and fields.
- [x] `kind.h` / `kind_util.c`: `kind_voltage` (§4.2), plus a local
      range-checked volts reader `kind_volts(ctx, json, key, max_v, &mv)`,
      built on `kind_number`. (Also `kind_volt_range` for `vcc`/`vf`; see
      Corrections.)
- [x] gate.c: add `"vcc"` and `"inputMax"` to `KEYS` and `SPLIT_KEYS`, and
      `{"family","vcc","inputMax"}` to the part keys. The timing entry struct
      holds a `const PartVoltage *`. `load_part` calls `kind_voltage` with
      the entry's value as base and sets `out->vcc`.
- [x] dff.c, jkff.c: the same, through their `TIMING_KEYS` and part keys.
- [x] sram.c, eeprom.c: append `"vcc"` and `"inputMax"` after `"note"` in
      `TIMING_NAMES`. Grades hold a `const PartVoltage *`. Add the part keys
      `{"grade","vcc","inputMax"}`.
- [x] oscillator.c: device keys and part keys gain `"vcc"` and `"inputMax"`.
- [x] display7.c: optional device key `"vf"`. switch.c: optional device key
      `"ratedVolts"`.
- [x] supply.c: `"volts"` replaces `"level"`, and `level = mv > 0`.
      `supply.json`: `"level": 1` → `"volts": 5`, `"level": 0` → `"volts": 0`.
- [x] `library_check.c`: change the 12 `'level':1/0` fixtures to
      `'volts':5/0`. Add rejection cases:
  - `vcc` not an array of 2;
  - min ≥ max;
  - max > 30;
  - `inputMax` below the resolved max (entry level and part level);
  - `inputMax` without `vcc`;
  - `vf` min > max;
  - `ratedVolts` 0;
  - `volts` −1 and 31;
  - an unknown key next to `vcc` in a grade.

  Add acceptance cases:
  - an entry-level `vcc` inherited by two parts (the same pointer);
  - a part override replacing only the range and keeping the entry's
    `inputMax`;
  - an SRAM grade with `vcc`, `inputMax` and timing fields, whose timings
    are unchanged (`access_ps`).

Acceptance: every new library_check case passes. The library still loads
every part file.
  Check: `cmake --build /tmp/p07 -j8 && /tmp/p07/library_check` → exit 0
  (est. 1 min).
  Result (2026-11-01): `library_check` → `library_check: all passed`, exit 0
  (14 new voltage cases print `ok`, among them "entry vcc inherited by two
  parts", "part vcc override keeps the entry inputMax", "sram grade with
  vcc, inputMax and timings", "unknown key next to vcc in a grade:
  … grades.55: unknown key \"vccMax\""); `ctest --test-dir /tmp/p07` →
  `100% tests passed, 0 tests failed out of 11` (real part files load).
Commit: —

### Phase 2 — Listing fields and the six supplies

- [ ] `listing.c`: §4.4 fields.
- [ ] `supply.json`: the §4.5 devices `5V`, `3V3`, `2V5`, `1V8`, `1V2` and
      `12V`, with descriptions updated for `VCC` and `GND`.
- [ ] Regenerate both goldens (`docs/backend.md` § Tests).
- [ ] `docs/parts-format.md`:
  - § Values: volts → mV.
  - § `supply`: `volts` replaces `level`; the table of eight supplies.
  - § Parts: the `vcc` and `inputMax` part keys.
  - Each kind's section: where `vcc`, `inputMax`, `vf` and `ratedVolts`
    go, with §4.1's ranges and §4.2's resolution rules.
  - Say that the keys are optional for now: "required from plan-07-F".

Acceptance: the golden diffs are exactly the expected ones.
  Check: `git diff -U0 backend/tests/golden/parts.txt | grep -c '^[-+][^-+]'`
  → 10 (2 changed supply lines = 2 `-` and 2 `+`, plus 6 new `+`), and
  `git diff -U0 backend/tests/golden/traces.txt | grep -c '^[-+][^-+]'` → 6
  (est. 1 min).
Commit: —

## Validation Plan

- Tests: `library_check` covers every new rule (Phase 1); `listing_check`
  covers the printed fields (Phase 2).
- Coverage check: the Phase 1 acceptance case for the SRAM grade runs code
  that the 21 SRAM devices also run. `listing_check` covers all 4006 parts.
- Runtime proof: none. This sub-plan changes no behavior, and the traces
  prove it.
- Doc sync: `docs/parts-format.md` (Phase 2).
- Final gate (once): `cmake --build /tmp/p07 -j8 && ctest --test-dir /tmp/p07`
  → 11/11 pass (est. 1 min). The app is not built here. No Swift file
  changes, and `PartSupply`/`PartSwitch` only gain fields (`PartCatalog.swift`
  reads `level`, `positions` and `actuator` by name).

## Decisions

Decided by the owner (Justin Zaun), 2026-11-01:

- **The chips' range is the datasheet's Recommended Operating Conditions
  supply range.** Absolute maximum ratings are not stored.
- **Supplies get a voltage.** The VCC toolbar button gets a dropdown:
  5V, 3V3, 2V5, 1V8, 1V2, 12V.
- **Old files' `VCC` opens as `5V`** (sub-plan G).
- **Supplies connect two ways.** They connect to power pins in range, and
  act as a logic 1 on other pins.
- **The sim stays logic-only.** The voltage check is separate and refuses
  out-of-range connections:
  - in the editor, the wire is refused with a message;
  - at start and on a running switch closing, the run stops, like smoke.
- **Input limit.** An input's limit is its part's VCC max, or its declared
  `inputMax` for 5 V-tolerant families (LVC, AHC, …).
- **Chip outputs carry their part's supply voltage.** That voltage is checked
  against the inputs it drives.
- **LEDs, 7-seg and switches.**
  - 7-seg store a forward voltage VF min–max. When no minimum is printed,
    min is the typical value. It is shown, not checked.
  - Switches store their rated voltage, and it is checked.
  - The generic LEDs store no VF: any voltage is allowed on them, and the
    anode's voltage passes to the cathode side for the check (sub-plan B).
- **plan-06 is in progress.** Only sub-plan G waits for it; A–F may run now.

Decided by the author:

- The range is set per entry/grade/device with a per-part override (§2,
  Verified). JSON is in volts and C in millivolts.
- `"volts"` replaces `"level"`, and `level` is kept as derived.
- A switch's `ratedVolts` is its switching rating, because it is the lower
  of its two ratings.

## Corrections

- **The 12 `'level'` fixtures are 11 `'level':1/0` fixtures plus the
  `'level':2` rejection case** (`a1_tests.py` counted 10 × `'level':1,` and
  1 × `'level':0,`). The rejection case became the two `volts` −1 / 31 cases.
- **§4.2 "keeping the entry's `inputMax`" needs to know whether the input max
  was given or derived.** `PartVoltage` stores only the resolved value, so
  `kind_util.c` allocates a private `{PartVoltage voltage; int input_given;}`
  and hands out `&voltage`. Without it, an entry `inputMax` equal to its
  `vcc` max would be lost on a part range override. The public struct is as
  §4.3 says.
- **Added task: `kind_volt_range`.** `vcc` (strict min < max, ≤ 30) and `vf`
  (min ≤ max, ≤ 10) share one `[min, max]` reader next to `kind_volts`.
- **Gate `load_delays` takes an extra-keys list.** Without it, `vcc` would
  be accepted inside `otherLow`/`otherHigh` (both go through
  `load_delays`). A `library_check` case ("vcc inside otherLow") rejects it.
- **Rule 4 (`inputMax` without `vcc`) also fires at the entry/grade/device
  level**, not only for a part: an entry with `inputMax` and no `vcc` is
  rejected ("entry inputMax without vcc" case). §4.1 says `inputMax` is
  checked against the resolved max, which needs a range to exist.

## Summary

The risk is the inheritance merge (§4.2) and the SRAM/EEPROM key list. Both
are covered by `library_check` cases. No behavior, check, data or app
change is in this sub-plan.
