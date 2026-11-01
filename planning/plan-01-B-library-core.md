# plan-01-B: Part library core, exporter, and the supply / LED / oscillator kinds

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-A

Builds the new part layer next to the old one: vendored cJSON, the public
`parts.h`, `load_parts()`, the kind table, the four engine hooks, the
old-tables-to-JSON exporter, and the three simplest kinds (`supply`, `led`,
`oscillator`). From here on, `listing_check` and `trace_check` compare the new
library against plan-01-A's goldens for every registered kind.

Outcome: `supply.json`, `led.json` and `oscillator.json` load through
`load_parts`, and their 150 parts match `golden/parts.txt` and
`golden/traces.txt` exactly.

References:

- `planning/plan-01-A-parts-baseline.md` §3 (overall design: layout, format,
  `parts.h`, kind table, correctness gate) and §4.1/§4.2 (trace and listing
  formats). That section is the design this sub-plan implements; it is not
  repeated here.
- `backend/led.c`, `backend/osc.c` + `osc_parts.inc`, `backend/supply.c` —
  the models being ported.
- `backend/led7.c:420-435` — the light on-time tracking that `lights.c` takes over.

## Prerequisites

See plan-01-A §Prerequisites (on `main`, clean tree, tools present), plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-A complete | `ls planning/completed/plan-01-A-parts-baseline.md` → exists | MET (2026-10-31: exists, 951e6fc) |
| Goldens present | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt` → 3817 each | MET (2026-10-31: 3817 each) |

## 1. Goal

- `load_parts(library, json, source, &error)` loads any well-formed part file
  for a registered kind and rejects malformed ones with a path-style message,
  adding nothing on failure.
- `supply`, `led`, `oscillator` are registered kinds; their JSON files exist in
  `schematical/schematical/Parts/`; `listing_check` and `trace_check` pass for
  their 150 parts.

### Non-goals

- The old backend and the app are not touched, except for four trailing
  members added to `SimComponentType` in `backend/sim.h` (old initializers
  leave them NULL).
- No `circuit.h` yet (plan-01-F).

## 2. Current State

- LED (`led.c`): passive, all callbacks NULL; 5 colors × 11 packages, 55 parts;
  pins `K`, `A` (+ 2 NC for DIP); lit state is computed in
  `netlist_sim.c` from net values.
- Oscillator (`osc.c`): 5 families (`osc.h:35-41`), 4 packages, 93 parts in
  `osc_parts.inc` (name, family, package, default Hz, enable pin index);
  min/max Hz per family (`osc.c:117-131`); outputs from `output_pins()`
  (`osc.c:70`; LMK6D/H/P are differential); 4 bands (`osc.c:296-301`).
- Supply (`supply.c`): `VCC` drives 1, `GND` drives 0; one pin each.

### Measured populations

| What | Count | Command |
|---|---|---|
| LED parts | 55 | `len(catalog["leds"])` in the plan-01-A dump |
| Oscillator parts | 93 | sum of band part counts in the dump |
| Supply parts | 2 | `len(catalog["supplies"])` |

### Verified properties

- Adding trailing members to `SimComponentType` does not break old
  initializers: every old instance uses positional initializers of the first
  7 members (read `74_00.c:306`, `led.c:104`, `cd4000.c:202`, `gates.c`,
  `sram.c`, `switch.c`, `led7.c`, `osc.c`, `supply.c`); C zero-fills the rest.
  The CMake target sets no `-Werror` (read `backend/CMakeLists.txt`).

## 3. Design

### 3.1 CMake

`backend/CMakeLists.txt` gains:

- `add_library(logic STATIC sim.c net.c src/library/*.c src/models/*.c src/third_party/cjson/cJSON.c)`
  (files listed explicitly), include dirs `include`, `src`, `src/library`,
  `src/third_party/cjson`, and `.` (for today's `sim.h`/`net.h`).
- `target_compile_definitions(<each test> PRIVATE PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts")`.
- `add_executable(export_parts tools/export/*.c sim.c net.c)` — **not** linked
  to either library (each export file `#include`s one old model `.c`).

### 3.2 Library internals (`src/library/`)

- `library.c`: `PartLibrary` = growable `const Part *` array + open-addressing
  hash of names + a list of arenas. `load_parts`:
  1. `cJSON_ParseWithLength`; on failure report the byte offset from
     `cJSON_GetErrorPtr`.
  2. Check file keys; `format == "schematical-parts"`, `version == 1`; find
     `kind` in `KINDS[]`.
  3. For each device: check keys = common ∪ `ops->device_keys`; parse
     `pinouts` (numbers exactly 1..N, roles, groups); `ops->load_device`;
     `ops->bind_pinout` per pinout.
  4. For each part: check keys = common ∪ `ops->part_keys`; `pinout` must name
     a device pinout; flags; fill the common `Part` fields (function,
     description, drawer from the device; pins from the pinout);
     `ops->load_part` fills the kind's info pointer and `internal`.
  5. Reject names already in the library or earlier in this file.
  6. On success move the file's arena into the library and append its parts;
     on any failure free the arena — the library is unchanged.
- `kind.h`: `PartKindOps` from plan-01-A §3.5, plus
  `const char *const *device_keys; const char *const *part_keys;`
  (NULL-terminated).
- `kind_util.c`: `KindContext` helpers — `kind_alloc`, `kind_strdup`,
  `kind_error(ctx, fmt, …)` (prefixes the current JSON path),
  `kind_pin(ctx, pins, n, name, &index)` (exactly one pin),
  `kind_pins(ctx, pins, n, name, out, cap)` (all pins with that name),
  `kind_ns(ctx, json, &ps)`, `kind_logic_family(ctx, text)` (the family list in
  plan-01-A §3.2), `kind_string`, `kind_uint`, `kind_bool`, `kind_names`.
- `kinds.c`: `KINDS[] = { &SUPPLY_KIND, &LED_KIND, &OSCILLATOR_KIND }` (C, D, E
  append).

### 3.3 Engine hooks (`backend/sim.h`, trailing members of `SimComponentType`)

```c
size_t (*lights)(const Sim *sim, const SimInstance *instance, const void *state,
                 SimTime since, unsigned char *out, size_t cap);
int (*set_switch)(Sim *sim, SimInstance *instance, void *state, size_t position, int on);
int (*memory_size)(const void *state, uint32_t *words, unsigned *bits);
size_t (*memory_read)(const void *state, uint32_t start, size_t count,
                      uint32_t *values, uint32_t *known);
```

### 3.4 Models (`src/models/`; engine `config` is `const Part *`)

- `lights.c`/`lights.h`: per-light `{on, ever_on, last_on}` tracking moved from
  `led7.c` (`lights_set(state, i, on, now)`, `lights_read(state, now, since, out, cap)`
  with led7's rule: `on || (ever_on && last_on >= since)`).
- `supply.c`: kind keys `level`; model drives its pin 1/0 strong at start.
- `led.c`: kind keys `color`, `anode`, `cathode`; stateful now — on
  `pin_changed`, light on ⇔ anode = 1 and cathode = 0 (the rule
  `netlist_sim.c` uses today); `lights` hook via `lights.c`.
- `oscillator.c`: device keys `family`, `minHz`, `maxHz`, `outputs`
  ([primary] or [primary, complement]); part keys `defaultHz`, `enablePin`
  (optional pin name). Behavior ported from `osc.c` (half period, enable
  handling, complement).

### 3.5 JSON layout of the three files

- `supply.json`: devices `VCC` (`level` 1) and `GND` (`level` 0), `drawer` [],
  function = name, one pinout `pin` with the old single pin, one part each.
- `led.json`: one device per color (id `led-red`, …), function `LED`, drawer
  `["Indicators", "LED"]`, pinouts `two-pin` and `dip4`, `anode` `A`,
  `cathode` `K`, one part per old LED part.
- `oscillator.json`: one device per (family, band), id `<family>-<band index>`
  (e.g. `lmk6c-2`), function = band label, drawer `["Oscillator"]`, pinouts
  keyed by package label lower-cased with spaces → `-` (`2520-vson`),
  `outputs` from `output_pins()`.

### 3.6 Exporter (`backend/tools/export/`, temporary)

- `export_main.c`: `export_parts <kind> <out.json>`; dispatches to
  `export_<kind>()`.
- `export_writer.c`: writes the file layout: header keys, then each device
  with keys in the order of plan-01-A §3.2, one pin per line, one part per
  line (`cJSON_PrintUnformatted` per pin/part object, assembled with
  newlines and two-space indents).
- `export_supply.c`, `export_led.c`, `export_osc.c`: each `#include
  "../../supply.c"` (etc.) and walks the static tables.
- Notes: exporters write `"note": "@<table>/<key>"` placeholders;
  `tools/export/notes.py <json>` replaces each with the source comment and
  removes keys that have none. Comment sources it reads:
  1. rows `{LOGIC_FAMILY_X, …}, /* c */` inside `static const (GtFamily|FfFamily) NAME[] = {` blocks → key `NAME/X`;
  2. `case LOGIC_FAMILY_X:` followed by a `/* c */` line inside `family_timing` of `74_*.c` → key `<file>/X`;
  3. the comment line directly above `static const SramTiming NAME` → key `NAME/`;
  4. the comment line directly above a `#define NAME_PINS(X)` in `led7_parts.inc` and `/* Pinout from … */` lines in `gates_defs.inc`, and `/* ---- … ---- */` headers in `sram_parts.inc` → device notes `NAME/`.
  B uses none of these (the three kinds here carry no comments) but lands the
  script so C–E only add exporters.

### 3.7 Tests (`backend/tests/`)

- `support/read_file.c`: read a whole file to a NUL-terminated string.
- `support/listing.c`: render one `Part` as a plan-01-A §4.2 line.
- `library_check.c`: inline-JSON cases — valid supply file loads (count,
  `find`, fields); bad JSON (message has offset); wrong `format`; `version` 2;
  unknown `kind`; unknown device key; unknown part key; pin numbers 1,3;
  part names a missing pinout; LED `anode` missing from one of two pinouts;
  duplicate part in one file; duplicate across two calls (second call fails,
  first file's parts still present, count unchanged); bad flag;
  `kind_logic_family("HX")` fails and `"HC"` passes.
- `listing_check.c`: load every `*.json` in `PARTS_DIR`, render, sort,
  compare with the golden lines whose kind is in `KINDS[]`; fail on any
  missing, extra or differing line (print the first 10).
- `trace_check.c`: same load; per part a `TraceSubject` (roles from pins;
  `set_switch` wraps the hook; `lights` only for `PART_7SEG`); compare with the
  golden hashes for registered kinds; `--verbose <part>` prints the log.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — cJSON and the format doc

- [x] Vendor cJSON v1.7.18 (`cJSON.c`, `cJSON.h`, `LICENSE`) into
      `backend/src/third_party/cjson/`, unmodified (fetched from the
      `v1.7.18` tag; `CJSON_VERSION_PATCH 18`).
- [x] `docs/parts-format.md`: the format of plan-01-A §3.2 (common keys,
      pin rules, flags, times, notes, uniqueness, failure atomicity) with the
      three kinds of this sub-plan; C, D, E add theirs.

Acceptance: cJSON compiles inside `logic`; the doc covers every common key.
  Check: `grep -c 'cJSON_ParseWithLength' backend/src/third_party/cjson/cJSON.h` → ≥1; `grep -E '"(format|version|kind|devices|id|function|description|drawer|note|pinouts|parts|name|package|pinout|flags)"' -o docs/parts-format.md | sort -u | wc -l` → 15 (est. seconds).
  Result: `2`; `15`; `cc -std=c11 -c cJSON.c` compiles (and inside `logic` from Phase 2).
Commit: —

### Phase 2 — `parts.h`, loader, kind table

- [ ] `backend/include/parts.h` exactly as plan-01-A §3.3.
- [ ] `src/library/library.c`, `kind.h`, `kind_util.c`, `kinds.c` (§3.2);
      `KINDS[]` is empty until Phase 3.
- [ ] `backend/sim.h`: the four hooks (§3.3).
- [ ] `backend/CMakeLists.txt`: the `logic` library.

Acceptance: the old backend still builds and passes; `logic` builds.
  Check: `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build -R 'baseline|catalog_check|netlist_check' --output-on-failure` → all pass (est. 2 min).
Commit: —

### Phase 3 — supply, LED, oscillator kinds and their JSON

- [ ] `src/models/lights.c`/`.h`, `supply.c`, `led.c`, `oscillator.c` (§3.4);
      register all three in `KINDS[]`.
- [ ] `tools/export/export_main.c`, `export_writer.c`, `export_supply.c`,
      `export_led.c`, `export_osc.c`, `notes.py` (§3.6).
- [ ] Generate `schematical/schematical/Parts/supply.json`, `led.json`,
      `oscillator.json` with `export_parts`.
- [ ] `tests/support/read_file.c`, `listing.c`; `tests/library_check.c`,
      `tests/listing_check.c`, `tests/trace_check.c` (§3.7); CTests
      `library_check`, `listing_check`, `trace_check`.
- [ ] Move `planning/plan-01-B-library-core.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: 150 parts match both goldens; the loader rejects every bad case.
  Check: `ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|trace_check' --output-on-failure` → 3 passed; `listing_check` prints `150 parts compared` (est. 1 min).
Commit: —

## Validation Plan

- Tests: `library_check` (loader, positive and every negative case in §3.7),
  `listing_check`, `trace_check`.
- Coverage check: `listing_check` and `trace_check` print the number of parts
  compared per kind; 2 supply, 55 led, 93 oscillator.
- Runtime proof: `trace_check --verbose LMK6CE001000DDLFR` (1 MHz) prints the
  clock output net changing every 500 000 ps.
- Doc sync: `docs/parts-format.md` (new).
- Final gate (this sub-plan): `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → all pass (est. 2 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

- plan-01-A's last commit (archive + Phase 3) is 951e6fc.
- §3.4/§3.5 oscillator `enablePin`: a pin **number**, not a name. The
  LMK6D/H/P pinout has two pins named `OE/ST/NC` (`osc.c:LMK6_DIFF_PINS`,
  pins 1 and 2), so a name cannot pick pin 1 (`kind_pin` rejects shared
  names, plan-01-A §3.2). It is a part key and a part names exactly one
  pinout, so a number is unambiguous. Measured: every old oscillator with an
  enable pin uses index 0 = pin 1 (`grep` of `osc_parts.inc` → 91 rows `0`,
  2 rows `-1`). Documented in `docs/parts-format.md`.
- Supplies have no package in the old backend; their `package` is `""`
  (allowed; the golden lines have an empty package field).

## Summary

The risk here is the loader: its validation is what keeps 3,800 hand-editable
parts honest later, so every rule in plan-01-A §3.2 has a failing case in
`library_check`. The three kinds are small and mostly prove the pipeline
(export → JSON → load → listing/trace parity) end to end before the large
kinds go through it.
