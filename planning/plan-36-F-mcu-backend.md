# plan-36-F: Backend — the `simulated` key, the `mcu` kind, the circuit refusal, tests, docs

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-E (letter order; F reads no census, pinout or footprint data)

F adds two things to the backend:

1. A common device key, `"simulated"` (boolean, default `true`), that any
   kind may carry. It becomes `Part.simulated`. `circuit_add_part` refuses
   a part whose `simulated` is 0, with `part "<name>" is not simulated`.
   The test listing appends `|unsimulated` to such a part's line.
2. The `mcu` part kind: `PART_MCU`, the facts struct `PartMcu`, and
   `backend/src/models/mcu.c` with a passive model (no callbacks, the
   `header.c` shape). An `mcu` device must say `"simulated": false`.

`Parts/mcu.json` ships with `"devices": []`. G adds the parts.

The single behavioral outcome: a fixture `mcu` part `TESTMCU` loads, lists
with `|unsimulated`, and `circuit_add_part(c, "TESTMCU", &i)` returns -1
with `circuit_error` = `part "TESTMCU" is not simulated`. A fixture
`header` device with `"simulated": false` is refused the same way. Every
existing part's listing, trace and footprint line stays unchanged.

References:

- `docs/backend.md` § Adding a kind (:304–327) and § Tests.
- `backend/include/parts.h` (`PartKind` :24–30, last value
  `PART_CAPACITOR`; flags :37–39; `Part` :189–229).
- `backend/src/library/library.c` (`DEVICE_KEYS` :238–240; the device
  loader :687–745: `kind_check_keys` :700, `note` checked by hand :721–723,
  `common.kind/drawer` :739–741; `load_part` copies `*part = *common`
  :610).
- `backend/src/library/kind.h` (`PartKindOps` :28–46, `kind_bool` :111,
  `kind_voltage` / `kind_part_voltage` :136–148), `kinds.c` (`KINDS[]`
  :34–63).
- `backend/src/models/header.c:57–67` (the model with no callbacks),
  `capacitor.c:144–161` (the same, with a non-NULL state pointer).
- `backend/src/circuit.c:125–176` (`circuit_add_part`; `fail` :44–52).
- `backend/src/engine/sim.c:255–307` (what `sim_add_instance` needs:
  `pin_count`, and a `create` that does not return NULL).
- `backend/tests/support/listing.c` (`listing_kind_name` :6–34,
  `print_details` :56+, `listing_line` :225+), `listing_check.c:50–51`,
  `trace_check.c:22`, `library_check.c`, `circuit_check.c`.

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-E complete | `ls planning/completed/plan-36-E-*.md` → exists | NOT MET (2026-11-12: plan written) |
| No `PART_MCU` and no `simulated` key yet | `grep -c 'PART_MCU' backend/include/parts.h` → 0; `grep -c '"simulated"' backend/src/library/library.c` → 0 | MET (2026-11-12 at 7ca1070: 0, 0) |

**Shared with plan-34 (resistors), which is open:** it appends
`PART_RESISTOR` as the last `PartKind` and moves the three last-kind test
references. F appends `PART_MCU` after whatever value is last when F runs
(`grep -n '} PartKind;' -B2 backend/include/parts.h`). It moves the same
three references to `PART_MCU`. The grep in Phase 1 shows them.

## 1. Goal

- `ctest` passes T + 1 programs, where T is the count when F starts (T0 =
  32 at 7ca1070, plus any program another plan added since). The new one
  is `mcu_test`.
- `listing_check`, `trace_check` and `footprint_check` pass with the
  goldens unchanged: `git diff --stat backend/tests/golden` → empty after F.
- `docs/parts-format.md` has the `"simulated"` row in § Devices, a kind-table
  row and a `### mcu` section. `docs/backend.md` names the refusal and
  `mcu_test`.

### Non-goals (explicit constraints)

- **No change to any existing part's behavior or facts**: `parts.txt`,
  `traces.txt` and `footprints.txt` stay byte-identical. This is F's
  byte-identity gate for existing parts. The new behavior is checked by
  `mcu_test`, `library_check` and `circuit_check`.
- No model for any microcontroller: no CPU core, no peripherals. The `mcu`
  model is passive, and only `trace_check`'s direct engine harness ever
  instantiates it. Circuits refuse the part.
- No app change (H). No part-editor form for `mcu` (H keeps it out of the
  editor's enabled kinds).
- The `simulated` key is a device key, not a part key: one device is one
  design, and whether a design has a model is the device's property.

## 2. Current State

- `PartKind` ends with `PART_CAPACITOR` (`parts.h:30`). `KINDS[]` has 27
  kinds (`grep -c '_KIND,$' backend/src/library/kinds.c` → 27). The
  last-kind test references are `trace_check.c:22` (`TRACE_LAST_KIND
  PART_CAPACITOR`) and `listing_check.c:50–51` (`PART_CAPACITOR + 1`,
  twice): `grep -rn 'TRACE_LAST_KIND PART_\|\[PART_[A-Z_]* + 1\]' backend/tests`
  → 3 lines.
- Common device keys are `id function description drawer note pinouts
  parts` (`library.c:238–240`). Unknown keys are rejected by
  `kind_check_keys` (`library.c:700`). There is no common boolean key
  today. `kind_bool` (`kind_util.c:205–216`) reads a required boolean, with
  the message `"%s" must be true or false`.
- `circuit_add_part` (`circuit.c:125–176`) finds the part, grows its
  arrays, and calls `sim_add_instance(circuit->sim, kind_model(part), …)`.
  Its failures use `fail(circuit, "…")`, e.g. `unknown part "%s"`.
- `trace_check` does not use the circuit API. `trace_part`
  (`trace_check.c:56–81`) adds `kind_model(part)` to a bare engine, so a
  refusal in `circuit_add_part` does not reach it. A passive model traces
  as a line with no changes (`GENERIC-LED` and every header have such
  lines).
- Test programs: `grep -c '^logic_test' backend/CMakeLists.txt` → 32
  (2026-11-12 at 7ca1070).

### Verified properties

- **A passive model builds and runs.** `sim_add_instance` needs only
  `pin_count` and a non-NULL `create` result (`sim.c:255–257`, `:301–307`).
  `start`, `pin_changed`, `timer`, `diodes` and `destroy` are each
  NULL-checked (`:1101`, `:1299`, `:1316`, `:1025`, `:192`). `header.c`
  ships exactly this model, and every header part is in `traces.txt`.
- **Common fields reach every part.** `load_part` starts each part as `*part
  = *common` (`library.c:610`), so a `common.simulated` set by the device
  loader is every part's value.
- **The app reads `Part` fields directly** (`Backend/PartCatalog.swift:635–661`:
  `kind = part.kind`, `flags = …`), so `Part.simulated` reaches Swift with
  no bridge change (H uses it).

## 3. Design

### 3.1 `simulated` (`parts.h`, `library.c`)

- `Part` gains `int simulated;` after `unsigned flags;`: 1 = the engine has
  a model for the part, 0 = it does not.
- `DEVICE_KEYS` gains `"simulated"`. The device loader reads it the way
  it reads `note`: absent → 1; `cJSON_IsBool` → its value; any other type
  → `"simulated" must be true or false`. It sets `common.simulated`.
- `docs/parts-format.md` § Devices: a row `"simulated"` | optional,
  default `true`; `false` marks a device with no simulation model (its
  parts can be placed, wired, laid out and listed in the BOM, but a circuit
  holding one does not start).

### 3.2 The refusal (`circuit.c`)

In `circuit_add_part`, right after the `part == NULL` check:

```c
if (!part->simulated) {
    return fail(circuit, "part \"%s\" is not simulated", part->name);
}
```

The app shows this text in its "Simulation stopped: …" capsule if Run is
ever reached with such a part (H disables Run first, so it should not
happen).

### 3.3 The `mcu` kind (`parts.h`, `models/mcu.c`, `kinds.c`)

```c
/* core: "avr", "avrrc", "avrxt", "avrxm", "cortex-m0plus", "xtensa-lx6",
   "xtensa-lx7", "riscv". Sizes in bytes; 0 = none (eeprom, psram) or
   external (RP2040 flash). */
typedef struct { const char *core; uint64_t max_hz; uint32_t flash, ram, eeprom, psram; } PartMcu;
```

`PART_MCU` is appended last (§ Prerequisites), with `const PartMcu *mcu;`
last among the `Part` kind pointers.

Device keys (besides the common ones):

| Key | Value |
|---|---|
| `"core"` | one of § 3.3's eight strings; else `unknown mcu core "X"` |
| `"clock"` | the highest clock, Hz, above 0 (`"clock" must be above 0`) |
| `"flash"`, `"ram"` | bytes, at least 0, at most 4294967295 |
| `"eeprom"` | optional, bytes |
| `"vcc"`, `"inputMax"` | optional at device level (oscillator/charlcd precedent, `kind_voltage`) |

The `mcu` kind requires `"simulated": false`: `load_device` reads the
device object, and a missing or `true` value fails with `an mcu has no
simulation model: "simulated" must be false`.

Part keys: `"vcc"`, `"inputMax"` (the part overrides the device), `"clock"`
(overrides the device's, e.g. the 10 MHz V grades), `"flash"` and
`"psram"` (bytes: an ESP32 module's `N8` flash and `R8` PSRAM, overriding
the device's). Every part must resolve a `vcc` (`kind_part_voltage`:
`missing "vcc"`).

Pins: any count, any role, any names. No key names a pin. `bind_pinout`
only checks that each pinout has at least one `power` and one `ground`
pin (`an mcu pinout needs a power pin` / `… a ground pin`).

The model: `MCU_MODEL = {"mcu", mcu_pin_count, mcu_create, NULL, …}` (the
`header.c` shape). `mcu_create` returns the part's `PartMcu` pointer.

### 3.4 Voltage

`mcu` parts carry `vcc`, so the voltage check judges their power pins and
input levels (an ESP32 module's `3V3` on a 5 V net is refused at edit
time, as for any 3.3 V chip). The RP2040's core-supply pins (`DVDD`,
1.1 V) are `power` pins fed by its own `VREG_VOUT` `output` (B § 3.2). The
check's fixed point drives an output at the part's level
(`voltage.c` step 3), so `DVDD` sees the I/O level and is in range. The
check cannot judge the 1.1 V rail. That is recorded in
`docs/parts-format.md` § `mcu`. No change to `voltage.c`.

### 3.5 Listing (`tests/support/listing.c`)

- `case PART_MCU: return "mcu";`.
- Facts: `core=<core> clock=<max_hz> flash=<flash> ram=<ram>`, then `
  eeprom=<n>` and ` psram=<n>` when non-zero.
- `listing_line` appends `|unsimulated` as the line's last field when
  `!part->simulated`. Today every part has `simulated` 1, so no existing
  line changes.

**Where the risk is:** small and contained. The loader change is one
optional key (absent = today's behavior). The refusal is reached only for
`simulated` 0, and the kind is new. The goldens prove nothing else moved.

### Rejected

- **`simulated` as a part key**: a device's parts share its pins and
  behavior. A model is a property of the design, not of a packing or grade.
- **A NULL model for `mcu`**: `kind_model` would be NULL, and
  `trace_check` and `sim_add_instance` would need special cases. The
  passive model keeps both unchanged.
- **Refusing in the loader or the app only**: the circuit API is the one
  place every caller goes through (the app, the tests, `sim_bench`), so the
  refusal there cannot be bypassed.
- **No `vcc` for `mcu`**: the edit-time voltage check is worth having for
  3.3 V modules on 5 V boards. The one multi-rail case (RP2040) is safe as
  § 3.4 shows.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — `simulated` and the refusal

- [ ] `backend/include/parts.h`: `int simulated;` (§ 3.1).
- [ ] `backend/src/library/library.c`: the key, its parse, `common.simulated`
      (default 1).
- [ ] `backend/src/circuit.c`: the refusal (§ 3.2).
- [ ] `backend/tests/support/listing.c`: `|unsimulated` (§ 3.5).
- [ ] `backend/tests/library_check.c`: a header device with `"simulated":
      false` loads (its parts have `simulated` 0); `"simulated": 1` fails
      with `"simulated" must be true or false`.
- [ ] `backend/tests/circuit_check.c`: that header part is refused with
      `part "<name>" is not simulated`; the same device without the key is
      added fine.

Acceptance: the refusal works, and no golden moved.
  Check: `cmake --build /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36 -R 'library_check|circuit_check|listing_check|trace_check|footprint_check'` → 5 passed; `git diff --stat backend/tests/golden` → empty (est. 3 min).
Commit: —

### Phase 2 — The `mcu` kind

- [ ] `parts.h`: `PART_MCU` last, `PartMcu`, the `mcu` pointer (§ 3.3).
- [ ] `backend/src/models/mcu.c`: keys, `load_device` (core, clock, sizes,
      `simulated` must be false, device `vcc`), `bind_pinout`, `load_part`
      (vcc, inputMax, clock, flash, psram), the passive model. Add it to the
      sources in `backend/CMakeLists.txt` beside `capacitor.c`.
- [ ] `kinds.c`: `extern const PartKindOps MCU_KIND;` and `&MCU_KIND,` last.
- [ ] `schematical/schematical/Parts/mcu.json`: `{"format":
      "schematical-parts", "version": 1, "kind": "mcu", "devices": []}`.
- [ ] Tests' kind arrays: the listing name and facts (§ 3.5); the three
      last-kind references move to `PART_MCU`. After the change, `grep -rn
      'TRACE_LAST_KIND PART_\|\[PART_[A-Z_]* + 1\]' backend/tests` → 3 lines,
      all `PART_MCU`.

Acceptance: the existing tests pass with the goldens untouched.
  Check: `cmake --build /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36 -R 'listing_check|trace_check|library_check|footprint_check'` → 4 passed; `git diff --stat backend/tests/golden` → empty (est. 3 min).
Commit: —

### Phase 3 — `mcu_test`, loader cases, docs

- [ ] `backend/tests/mcu_test.c` and `logic_test(mcu_test)` in
      `backend/CMakeLists.txt`. The fixture lives in
      `tests/support/mcu_fixture.h`: one device `TESTMCU`, core `avr`,
      clock 20000000, flash 32768, ram 2048, eeprom 1024, vcc [1.8, 5.5], a
      `p8` pinout (VCC, GND, six `P*` inout pins), and one part with
      `"clock": 10000000`. The test checks that it loads; that the facts
      read back (clock 10000000 on the part, flash 32768, `simulated` 0);
      that the listing line ends `|unsimulated`; and that
      `circuit_add_part` returns -1 with `part "TESTMCU" is not simulated`.
- [ ] `library_check.c`: one rejection per message of § 3.3: unknown core,
      clock 0, `simulated` missing, `simulated` true, a pinout with no
      ground, `missing "vcc"`.
- [ ] `docs/parts-format.md`: `"simulated"` in § Devices (§ 3.1); `|
      mcu.json | mcu |` in the kind table (add the row only if absent:
      `grep -c '^| `mcu.json`' docs/parts-format.md` → 0 before); a `### mcu`
      section after `### cpu` with § 3.3's tables and § 3.4's voltage note;
      "microcontroller" in the first paragraph's list.
- [ ] `docs/backend.md`: § Adding a kind gains one sentence (a kind with no
      model sets `"simulated": false` on its devices and ships a passive
      model). The test list gains `mcu_test`. `circuit_check`'s entry names
      the refusal.

Acceptance: T + 1 programs pass.
  Check: `cmake -S backend -B /tmp/logic-p36 && cmake --build /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36` → `100% tests passed, 0 tests failed out of <T+1>` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `mcu_test` (load, facts, listing, refusal); `library_check` (every
  rejection, the header `simulated` case); `circuit_check` (the refusal on
  a non-mcu kind).
- Coverage check: `ctest -N --test-dir /tmp/logic-p36 | grep -c mcu_test` → 1.
- Runtime proof: `mcu_test`'s refusal case; the app-level proof is H's.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate: plan-36-I.

## Decisions

- **`simulated` is a common device key that any kind may use** — owner's
  wording ("a `simulated` property set to false") plus the author's
  generalisation. The to-do list also asks for "parts with a symbol,
  footprint and BOM data but no simulation model"
  (`planning/todo.md`, § Parts). A common key serves both without a second
  mechanism. That item stays open: it also needs symbols for analog
  parts, and plan-36 does not draw those.
- **The refusal is in `circuit_add_part`** — author: § 3 Rejected.
- **Passive model** — author: § 3 Rejected.

## Corrections

## Summary

F is one optional loader key, one early return in the circuit API, and a
passive kind. The goldens are its byte-identity gate for everything that
already exists. The new tests cover the new behavior both through the
`mcu` kind and through a plain header with the key set.
