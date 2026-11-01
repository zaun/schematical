# plan-01-A: Part library rewrite — design and behavior baseline

Last updated: 2026-10-31
Overall Effort: huge (>3d)
Effort: large (3h–1d)
Depends on: nothing

plan-01 replaces the backend's part layer. Today every kind of part (gates,
flip-flops, SRAM, switches, LEDs, 7-segment displays, oscillators, supplies)
is wired in its own way, its data is compiled into C tables, and `LogicFamily`,
the catalog `"family"` field and `gate_count` each mean different things for
different kinds. After plan-01:

- all part data lives in JSON files with **one shared format**, one file per
  kind, in `schematical/schematical/Parts/` (`gate.json`, `dff.json`,
  `jkff.json`, `sram.json`, `switch.json`, `led.json`, `7seg.json`,
  `oscillator.json`, `supply.json`);
- the backend loads them with `load_parts()` into a `PartLibrary`, and the app
  reads the parts back as **typed C structs** (no catalog JSON, no netlist
  text, no snapshot text);
- a new kind of part is added the same way every time: one model file, one
  line in the kind table, one info struct, one JSON file.

The single behavioral outcome: **every one of the 3,817 orderable parts
simulates exactly as it does today (identical net-change traces) and shows
the same catalog facts, while all part data comes from the JSON files.**

This sub-plan (A) holds the **overall design** that B–G implement, and lands
the safety net: two golden files recorded from today's backend, a full
catalog listing and a per-part simulation trace hash, that every later
sub-plan must reproduce.

References:

- `AGENTS.md` — Git rules (no branch unless the owner names it; no
  tree-wide checkout/reset/restore/stash; commit only files you changed,
  itemized). Datasheet rules
  (`PART_FLAG_OBSOLETE` wording changes in plan-01-G).
- `backend/catalog.h` — today's catalog JSON layout (being removed).
- `backend/netlist_sim.h` — today's text netlist API (being removed).
- `backend/sim.h` — the event engine (kept).
- `docs/file-format.md` — the `.schematical` file format (unchanged by this plan).

Sub-plans, in implementation order:

| Letter | File | Delivers | Effort |
|---|---|---|---|
| A | `plan-01-A-parts-baseline.md` | Design; golden listing + golden traces from today's backend | large |
| B | `plan-01-B-library-core.md` | cJSON, `parts.h`, `load_parts`, kind table, engine hooks, exporter, supply/led/oscillator kinds | large |
| C | `plan-01-C-gate-kind.md` | `gate` kind replacing `74_*.c`, `gates.c`, `cd4000.c` | large |
| D | `plan-01-D-flipflop-switch.md` | `dff`, `jkff`, `switch` kinds | large |
| E | `plan-01-E-sram-7seg.md` | `sram`, `7seg` kinds; library complete | large |
| F | `plan-01-F-circuit-api.md` | `circuit.h` replacing `netlist_sim`; basicsim port | large |
| G | `plan-01-G-app-cutover.md` | Swift on the new API; Xcode folder sync; old backend deleted; docs | large |

## Prerequisites

These are a precondition on all of plan-01, stated once here; B–G point here.

| Must be true | Command | Status |
|---|---|---|
| On the intended branch (`main` unless the owner chose another) | `git -C /Users/justinzaun/Development/logic branch --show-current` → `main` | MET (2026-10-31 at /follow-plan start: `main` 7cfcf15; work runs in worktree `worktree-P-01` forked from it) |
| Working tree clean before starting | `git -C /Users/justinzaun/Development/logic status --short` → empty | MET (2026-10-31: empty) |
| Current backend builds and its 5 tests pass | `cmake -S backend -B /tmp/logic-old && cmake --build /tmp/logic-old -j8 && ctest --test-dir /tmp/logic-old` → `100% tests passed, 0 tests failed out of 5` | MET (2026-10-31: `100% tests passed, 0 tests failed out of 5`) |
| CMake, Python 3 and Xcode present | `which cmake python3 && xcodebuild -version` → paths, `Xcode 26.3` | MET (2026-10-31: brew cmake/python3, `Xcode 26.3`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `backend/tests/golden/parts.txt` holds one line per orderable part (3,817
  lines) describing every catalog fact the app uses, produced from today's
  backend.
- `backend/tests/golden/traces.txt` holds one line per orderable part (3,817
  lines) with a 64-bit hash of every net change during a fixed 3 µs stimulus
  run, produced from today's backend.
- Two CTest tests regenerate both from today's backend and diff them against
  the goldens (they pass now, and keep passing until plan-01-G deletes the old
  backend).

### Non-goals (explicit constraints)

- No change to any existing backend source file in this sub-plan. The
  baseline tools only *read* the old API.
- No change to the app.
- The `.schematical` file format and every part number stay exactly as they
  are for all of plan-01 (part names are the join key between old and new).

## 2. Current State

### How parts get into the system today

| Kind | Code | How it registers |
|---|---|---|
| 74x00/02/08/32/86/266 | `backend/74_*.c` (hand-written, near copies; `74_86.c` adds XOR timing that depends on the other input, `74_266.c` is open-drain) | `SN74_xx_DEFINITION` + `_FK_DEFINITION` in `catalog.c:DEFINITIONS` |
| Other gates | `backend/gates.c` + generated `gates_defs.inc` (2,785 lines; no generator in the repo) | `gates_catalog.inc` |
| CD4000 quad gates | `backend/cd4000.c` (`CD_DEFINE`, single delay) | `catalog.c:DEFINITIONS` |
| D / JK flip-flops | `backend/ff-d.c` / `ff-jk.c` (`FF_DEFINE_CHIP`) | `catalog.c:DEFINITIONS` |
| SRAM | `backend/sram.c` + `sram_parts.inc` + `sram_catalog.inc` | `SRAM_CHIP` macro in `catalog.c` |
| DIP switch | `backend/switch.c` | `switch_definition_count/at`, chained in `catalog.c:catalog_definition_at` |
| 7-segment | `backend/led7.c` + `led7_parts.inc` | `led7_definition_count/at`, chained the same way |
| LED | `backend/led.c` | not a `ChipDefinition`; own JSON section |
| Oscillator | `backend/osc.c` + `osc_parts.inc` | not a `ChipDefinition`; own nested JSON section |
| Supply | `backend/supply.c` | not a `ChipDefinition`; own JSON section |

- `sim.h:LogicFamily` mixes real families (HC, LS, …, TTL) with kind tags
  (`LOGIC_FAMILY_SRAM`, `_SWITCH`, `_LED7`) that `led7.c:110`, `switch.c:189`,
  `sram.c:183` and `catalog.c:229` use to tell kinds apart.
- `catalog.c:287-303` writes `"family"` as the logic family, the SRAM speed
  grade, the switch actuator or the display color, depending on the kind.
- `ChipDefinition.gate_count` is gates, switch positions, display digits, or 0.
- `netlist_sim.c:resolve_part` tries LED, supply, oscillator, then every chip
  definition in turn; `netlist_sim.c:23` holds a union of four config types.
- The app (`ChipCatalog.swift:218-229`, `:364-378`, `SchematicCanvasView.swift:95`)
  tells kinds apart by the category strings `"Logic"`, `"Indicators"`,
  `"LED 7-Seg"`, `"Switch"`, `"Memory"`.

### The engine (kept)

`backend/sim.c` + `net.c`: event queue, net resolution, short detection,
contacts. On every net value change it calls `pin_changed` on **every**
instance attached to that net (`sim.c:785-806`, read). A passive "probe"
instance attached to all nets therefore sees every change — the trace harness
below relies on this.

### Measured populations

| What | Count | Command |
|---|---|---|
| Chip definitions in today's catalog | 223 | `dump` of `catalog_write_json` → `len(chips)` = 223 |
| Orderable parts, all kinds | 3,817 | chips' parts 3,667 + `leds` 55 + oscillators 93 + supplies 2 |
| — gate parts (NAND 751, AND 653, OR 434, NOR 421, XOR 270, XNOR 23) | 2,552 | parts per Logic `function` in the dump |
| — D flip-flop parts | 672 | same, function `D flip-flop` |
| — JK flip-flop parts | 186 | same, function `JK flip-flop` |
| — SRAM parts / definitions | 172 / 37 | category `Memory` |
| — switch parts / definitions | 36 / 27 | category `Switch` |
| — 7-segment parts / definitions | 49 / 49 | subcategory `LED 7-Seg` |
| — LEDs, oscillators (in 4 bands), supplies | 55, 93, 2 | `leds`, `oscillators.groups`, `supplies` |
| Duplicate part names across all kinds | 0 | names vs set(names) in the dump |
| Pinouts not numbered exactly 1..N in order | 0 | pin numbers vs `range(1, n+1)` over all pin tables |
| `_FK` (20-pin LCCC) definitions | 19 | `grep -c FK_DEFINITION backend/*.h` → 74_00/02/08/32/86: 1 each, ff-d 3, ff-jk 2, gates 9 |
| Backend C/H lines | 13,870 | `wc -l backend/*.c backend/*.h` |
| Backend `.inc` data lines | 7,723 | `wc -l backend/*.inc` |
| Timing rows/blocks carrying a datasheet comment | 123 gate rows, 34 flip-flop rows, 70 hand-gate cases, 37 SRAM tables | `grep -cE` on `gates_defs.inc`, `ff-*.c`, `74_*.c`, `sram_parts.inc` |

The dump was produced with a 3-line program calling `catalog_write_json`,
linked against `/tmp/plancat/build/liblogic_backend.a`.

### Verified properties

- **Every model's catalog facts are reachable from the old public API** —
  read: `catalog.h`, `led.h`, `osc.h`, `supply.h`, `led7.h`, `switch.h`
  (`switch_family_label`), `sram.h` (`sram_access_time`, `sram_instance_size`).
  The baseline listing tool needs nothing static.
- **The probe approach sees every change** — read `sim.c:785-806`: the
  change loop visits all pins on a changed net and calls `pin_changed` when it
  is non-NULL; a pin attached with `net_attach` drives `LOGIC_Z`
  (`net.c:75`), and resolution skips Z drivers (`net.c:101`), so the probe
  does not disturb values.
- **Pin names can be the key between pinouts** — duplicate non-NC, non-hidden
  pin names inside one pinout occur only on SRAM power/ground pins (25
  definitions) and on 7-segment common pins (13 definitions); in every
  7-segment case the same-named pins are adjacent and ascending in the
  display's `segmentPins`/`commonPins` order (checked over all 49 displays).
- **Pin roles and groups in use**: roles input, output, inout, power, ground,
  noConnect; groups `default`, `address`, `hidden`; every `hidden` pin has
  role noConnect.

## 3. Overall Design (B–G implement this)

### 3.1 Layout after plan-01

```
backend/
  CMakeLists.txt
  include/                 public API; the only headers the app sees
    parts.h                PartLibrary, load_parts, Part + per-kind info structs
    circuit.h              Circuit: build, run, read back
  src/                     compiled into the app by an Xcode synchronized folder
    engine/                sim.c sim.h net.c net.h (today's engine, moved in G)
    library/               library.c kinds.c kind.h kind_util.c
    models/                gate.c dff.c jkff.c sram.c switch.c led.c display7.c
                           oscillator.c supply.c lights.c lights.h
    circuit.c
    third_party/cjson/     cJSON.c cJSON.h LICENSE (v1.7.18)
  tests/                   CTest programs (never in the app)
    golden/parts.txt       from this sub-plan
    golden/traces.txt      from this sub-plan
    support/               trace_core.c listing.c read_file.c
  tools/                   temporary; deleted in G
    baseline/              old_listing.c old_trace.c (this sub-plan)
    export/                old tables -> JSON (B–E)
schematical/schematical/Parts/
  gate.json dff.json jkff.json sram.json switch.json led.json 7seg.json
  oscillator.json supply.json
```

During B–F the new code builds as a separate CMake library `logic` that
compiles `backend/sim.c` and `backend/net.c` in place; the old `logic_backend`
target and the Xcode app keep using the old files untouched. G moves the
engine, deletes the old files and switches the app.

### 3.2 The part file format (one format, every kind)

```json
{
  "format": "schematical-parts",
  "version": 1,
  "kind": "gate",
  "devices": [
    {
      "id": "74_00",
      "function": "NAND",
      "description": "Quad 2-input NAND gate",
      "drawer": ["Logic"],
      "note": "optional provenance text",
      "pinouts": {
        "dip14": [
          {"number": 1, "name": "1A", "role": "input"},
          {"number": 2, "name": "1B", "role": "input"},
          {"number": 3, "name": "1Y", "role": "output"}
        ],
        "lccc20": [ ... ]
      },

      "gates": [{"op": "nand", "inputs": ["1A", "1B"], "output": "1Y"}],
      "timing": {"HC": {"rise": 9, "fall": 9, "note": "Typical at 4.5 V, 50 pF"}},

      "parts": [
        {"name": "SN74HC00N", "package": "DIP", "pinout": "dip14", "family": "HC",
         "flags": ["obsolete"]}
      ]
    }
  ]
}
```

Rules, enforced by the loader:

- **Common keys.** File: `format` (must be `"schematical-parts"`), `version`
  (must be 1), `kind`, `devices`. Device: `id`, `function`, `description`,
  `drawer`, `note`, `pinouts`, `parts`. Part: `name`, `package`, `pinout`,
  `flags`. **Every other key belongs to the kind**, and the kind rejects keys
  it does not know. This is the single rule that keeps all kinds uniform.
- `drawer` is 0–2 strings, the parts-drawer path (`["Memory", "SRAM"]`); `[]`
  means the part is never listed (VCC, GND). It is presentation only — no code
  compares it.
- A pinout is an array of pins numbered exactly 1..N in order. Pin `role` is
  `input`, `output`, `inout`, `power`, `ground`, `nc` or `absent` (`absent`
  replaces today's `hidden` group: a numbered position with no lead). `group`
  is optional (default `"default"`).
- **Kind keys refer to pins by name, never by number**, so one device can carry
  several pinouts (DIP and LCCC, DIP and sTSOP). Every name a kind key uses must
  exist in every pinout of that device. A name shared by several pins means all
  of them, in pin-number order (7-segment commons); kinds that need exactly one
  pin reject such names.
- `flags` ⊆ `obsolete`, `military`, `automotive`, `enhanced`.
- Times are nanoseconds (JSON numbers); the loader converts with
  `llround(ns * 1000)` to the engine's picoseconds.
- `note` is allowed on a device and on any timing entry; the loader ignores it.
- Part names are unique across the whole library (across files); device ids
  are unique within a kind.
- A failed `load_parts` adds nothing from that file.

Kind keys (full definitions live in each kind's sub-plan):

| Kind (file) | Device keys | Part keys |
|---|---|---|
| `gate` | `gates` [{op, inputs, output}], `output` (`push-pull`\|`open-drain`), `timing` {family: {rise, fall} or {otherLow:{rise,fall}, otherHigh:{rise,fall}}} | `family` |
| `dff` | `units` [{d, clk, pre?, clr?, q?, qn?}], `outputEnable?`, `presetActiveHigh`, `clearActiveHigh`, `clockEdge`, `timing` {family: {rise, fall, clearWins}} | `family` |
| `jkff` | `units` [{j, k, clk, pre?, clr?, q?, qn?}], `kInverted`, `presetActiveHigh`, `clearActiveHigh`, `clockEdge`, `timing` {family: {rise, fall}} | `family` |
| `sram` | `address` [names], `data` [names], `chipEnable` [names], `chipEnableHigh` [names], `outputEnable`, `writeEnable`, `byteEnable` [names], `grades` {grade: {24 timing fields}} | `grade` |
| `switch` | `actuator`, `contacts` [[a, b], …] (positions = count) | — |
| `led` | `color`, `anode`, `cathode` | — |
| `7seg` | `digits`, `commonAnode`, `colonAfter`, `dotAfter`, `color`, `size`, `chip`, `leds` [{digit, segment}], `segmentPins` {name: {digit, segments}}, `commonPins` {name: digit\|"extras"}, `segmentSide` [names], `commonSide` [names] | — |
| `oscillator` | `family`, `minHz`, `maxHz`, `outputs` [primary, complement?] | `defaultHz`, `enablePin?` |
| `supply` | `level` (0\|1) | — |

`family` values are validated against one list of real logic families (HC,
LS, ACT, HCT, F, AC, AHCT, ALS, AS, AHC, S, C, H, HCS, LV, LVC, AUP, AUC,
ALVC, TTL). That list is the whole of what `LogicFamily` becomes; there is no
enum and no kind ever stores anything else in it.

### 3.3 Public C API — `backend/include/parts.h`

```c
typedef struct PartLibrary PartLibrary;

typedef enum {
    PART_GATE, PART_DFF, PART_JKFF, PART_SRAM, PART_SWITCH,
    PART_LED, PART_7SEG, PART_OSCILLATOR, PART_SUPPLY
} PartKind;

typedef enum {
    PIN_INPUT, PIN_OUTPUT, PIN_INOUT, PIN_POWER, PIN_GROUND, PIN_NC, PIN_ABSENT
} PinRole;

enum { PART_OBSOLETE = 1u << 0, PART_MILITARY = 1u << 1,
       PART_AUTOMOTIVE = 1u << 2, PART_ENHANCED = 1u << 3 };

typedef struct { unsigned number; const char *name; PinRole role; const char *group; } PartPin;

/* Exactly one of these is set on a Part, matching its kind. */
typedef struct { const char *family; unsigned units; } PartLogic;         /* gate, dff, jkff */
typedef struct { uint32_t words; unsigned bits; uint64_t access_ps; const char *grade; } PartSram;
typedef struct { unsigned positions; const char *actuator; } PartSwitch;
typedef struct { const char *color; } PartLed;
typedef struct { unsigned digit; const char *segment; } PartDisplayLed;  /* digit 0 = colon/dot */
typedef struct {
    unsigned digits; int common_anode; unsigned colon_after, dot_after;
    const char *color, *size, *chip;
    const PartDisplayLed *leds; size_t led_count;
    const unsigned *segment_pins; size_t segment_pin_count;
    const unsigned *common_pins; size_t common_pin_count;
} PartDisplay;
typedef struct { const char *family; uint64_t min_hz, max_hz, default_hz;
                 unsigned enable_pin; /* pin number, 0 = none */ } PartOscillator;
typedef struct { int level; } PartSupply;

typedef struct Part {
    PartKind kind;
    const char *name, *package, *function, *description;
    const char *const *drawer; size_t drawer_count;
    unsigned flags;
    const PartPin *pins; size_t pin_count;
    const PartLogic *logic;
    const PartSram *sram;
    const PartSwitch *dip_switch;
    const PartLed *led;
    const PartDisplay *display;
    const PartOscillator *oscillator;
    const PartSupply *supply;
    const void *internal;              /* model data; not for the app */
} Part;

PartLibrary *part_library_create(void);
void part_library_destroy(PartLibrary *library);

/* Parses one part file's text. 0 on success; -1 and a malloc'd message in
   *error (free it) on failure, with nothing from that file added. `source`
   names the file in messages, e.g. "gate.json". */
int load_parts(PartLibrary *library, const char *json, const char *source, char **error);

size_t part_library_count(const PartLibrary *library);
const Part *part_library_at(const PartLibrary *library, size_t index);
const Part *part_library_find(const PartLibrary *library, const char *name);
```

`load_parts` takes text, not a path: the app reads bundle resources itself,
and tests read files with `tests/support/read_file.c`. Error messages carry a
JSON path: `gate.json: devices[3] (74_00): parts[12] (SN74HC00N): unknown family "HX"`.

### 3.4 Public C API — `backend/include/circuit.h` (plan-01-F)

```c
typedef struct Circuit Circuit;
typedef enum { CIRCUIT_0, CIRCUIT_1, CIRCUIT_X, CIRCUIT_Z } CircuitValue;

Circuit *circuit_create(const PartLibrary *library);   /* library must outlive it */
void circuit_destroy(Circuit *circuit);
const char *circuit_error(const Circuit *circuit);      /* NULL when none */

/* Building: 0 on success, -1 with circuit_error set. Before circuit_start. */
int circuit_add_part(Circuit *circuit, const char *part_name, size_t *index);
int circuit_add_net(Circuit *circuit, size_t *index);
int circuit_connect(Circuit *circuit, size_t part, unsigned pin_number, size_t net);

/* Switches: position is 1-based; works before and after start. */
int circuit_set_switch(Circuit *circuit, size_t part, unsigned position, int on);

int circuit_start(Circuit *circuit);
int circuit_advance(Circuit *circuit, uint64_t picoseconds);  /* 0 once stopped */
uint64_t circuit_time(const Circuit *circuit);

CircuitValue circuit_net_value(const Circuit *circuit, size_t net);
int circuit_shorted_net(const Circuit *circuit, size_t *net);  /* 1 = a net shorted */

/* LEDs (1 light) and 7-segment displays (PartDisplay.leds order): a light is
   on when it was on at any time during the last circuit_advance. */
size_t circuit_part_lights(const Circuit *circuit, size_t part, unsigned char *on, size_t cap);

/* SRAM contents. */
int circuit_memory_size(const Circuit *circuit, size_t part, uint32_t *words, unsigned *bits);
size_t circuit_memory_read(const Circuit *circuit, size_t part, uint32_t start, size_t count,
                           uint32_t *values, uint32_t *known);
```

### 3.5 Adding a kind (the uniform path)

Internal `src/library/kind.h`:

```c
typedef struct PartKindOps {
    PartKind kind;
    const char *name;                       /* the file's "kind" */
    const SimComponentType *model;          /* engine callbacks; config = const Part * */
    void *(*load_device)(KindContext *ctx, const cJSON *device);
    void *(*bind_pinout)(KindContext *ctx, const void *device,
                         const PartPin *pins, size_t pin_count);
    int (*load_part)(KindContext *ctx, const void *device, const void *bound,
                     const cJSON *part, Part *out);
} PartKindOps;
```

`KindContext` (in `kind_util.c`) gives every kind the same helpers: arena
allocation and string copy, error reporting with the JSON path, a
"reject unknown keys" check, pin-name lookup in a pinout, ns→ps conversion and
the logic family check. `src/library/kinds.c` holds `KINDS[]`, one line per
kind.

To add a kind: (1) `PART_<KIND>` and its info struct in `parts.h`;
(2) `src/models/<kind>.c` with the ops and the engine callbacks; (3) one line
in `KINDS[]`; (4) `Parts/<kind>.json`; (5) the app picks a symbol for the new
`PartKind`. Nothing else. G writes this into `docs/backend.md`.

Engine callbacks gain four optional hooks, added at the end of
`SimComponentType` in plan-01-B (trailing members, so the old positional
initializers keep compiling): `lights`, `set_switch`, `memory_size`,
`memory_read`. They replace `netlist_sim.c`'s `is_led` flag and its
`led7_`/`sram_`/`switch_` calls.

### 3.6 Correctness gate

This is a behavior-preserving rewrite with **two expected differences**:

1. Devices that today exist twice only for a second pinout (`_FK`, SRAM
   `_STSOP`/`_TSOP`/`_BGA` layouts) become one device with several pinouts
   when their kind keys are identical. Their parts keep every listed fact
   except `description` and device id, which the listing leaves out.
2. A plain LED's light becomes "on at any time during the last step" (as
   7-segment displays already are) rather than "on at the end of the step".
   Today's traces do not record LED light state, so no golden changes.

The gate is the two goldens from this sub-plan:

- `listing_check` (plan-01-B onward): the new library's listing equals
  `golden/parts.txt` line for line, for every kind registered in `KINDS[]`.
- `trace_check` (plan-01-B onward): each part's trace hash under the new models
  equals `golden/traces.txt`.

A mismatch means a porting bug or an exporter bug: diff the one part's
listing line, or print its full trace with `trace_core`'s verbose mode (old
vs new) and fix the code. A mismatch never means the design is wrong.

### Rejected alternatives

- **Keep a catalog JSON from backend to app** — rejected by the owner: a second
  JSON format that can drift. The app reads C structs.
- **One global part registry** — rejected by the owner; a library handle lets
  tests load only what they need.
- **Hand-write a JSON parser** — rejected by the owner; vendor cJSON.
- **Write the JSON files by hand or with a Python script that parses the C
  sources** — the tables use X-macros, `GT_DEFINE`/`CD_DEFINE`/`FF_DEFINE_CHIP`
  and switch statements; parsing them in Python is fragile. The exporter
  instead `#include`s each old model `.c` file into its own C translation unit
  and walks the real compiled tables, so the data is exactly what the old
  backend uses. Only the comments (datasheet provenance), which the compiler
  drops, come from a small Python pass.
- **Keep `PackageType` and family enums** — strings validated at load are
  enough; no code branches on a package, and families are only shown and
  filtered.
- **Generic `variant` key on parts** — it would rebuild the overloaded `family`
  field under a new name. Each kind names its own part keys instead.
- **Keep the netlist text protocol** — rejected by the owner; C calls.
- **Rewrite the event engine** — `sim.c`/`net.c` are sound and not part of the
  mess; they move and gain four optional hooks, nothing more.

## 4. Detailed Design (this sub-plan)

### 4.1 Trace harness — `backend/tests/support/trace_core.c`

Shared by the old baseline tool (here) and `trace_check` (B onward). It only
uses the engine (`sim.h`).

```c
typedef enum { TRACE_NONE, TRACE_INPUT, TRACE_INOUT, TRACE_POWER, TRACE_GROUND } TraceRole;
typedef struct {
    const char *name;                 /* part name: seeds the stimulus */
    const SimComponentType *type;
    const void *config;
    const TraceRole *roles; size_t pin_count;
    int (*set_switch)(Sim *, SimInstance *, size_t position, int on);  /* NULL if not a switch */
    size_t switch_positions;
    size_t (*lights)(const Sim *, const SimInstance *, SimTime since, char *out, size_t cap); /* NULL unless 7-seg */
} TraceSubject;
uint64_t trace_run(const TraceSubject *subject, FILE *verbose /* NULL = quiet */);
```

The run, exactly:

1. One net per pin; the part's pin `i` on net `i`.
2. A stimulus instance with `pin_count` pins; pin `i` joins net `i` when its
   role is not `TRACE_NONE`. Its `start` drives power pins 1 (strong) and
   ground pins 0 (strong) at t = 0. For each input (strong) and inout (weak)
   pin it schedules 40 drives: an xorshift64 generator seeded with
   `fnv1a64(name) ^ ((i + 1) * 0x9E3779B97F4A7C15)`; first drive value 0 at
   t = 1000 ps; each next drive at `t += 2000 + next() % 150000` ps with the
   value toggled.
3. A probe instance with `pin_count` pins, pin `i` on net `i`; its
   `pin_changed` folds `(time, pin, new value)` into an FNV-1a 64 hash.
4. Switches: before start, position `p` (1-based) is ON when bit `p` of
   `fnv1a64(name)` is set; at 1.5 µs every position flips.
5. Run to 3 µs in 100 ns steps; for subjects with `lights`, fold the light
   string for each step (since = step start) into the hash.
6. Fold `sim_error()` (or `"ok"`) and the final time into the hash.

Roles: input→INPUT, inout→INOUT, power→POWER, ground→GROUND, output/nc/hidden
(absent)→NONE.

### 4.2 Listing format — `backend/tests/golden/parts.txt`

One line per part, sorted by name (`strcmp`), fields separated by `|`:

```
name|kind|package|drawer|function|family|flags|details|pins
```

- `kind`: gate, dff, jkff, sram, switch, led, 7seg, oscillator, supply.
- `drawer`: path joined with `/` (`Logic`, `Memory/SRAM`, `Switch/DIP`,
  `Indicators/LED`, `Indicators/LED 7-Seg`, `Oscillator`); empty for supplies.
- `function`: the drawer leaf (today's `function`; `LED` for LEDs, the band
  label for oscillators, `VCC`/`GND` for supplies).
- `family`: logic family for gate/dff/jkff; empty otherwise.
- `flags`: subset of `obsolete,military,automotive,enhanced` in that order.
- `details`, by kind:
  - gate/dff/jkff: `units=<gate_count>`
  - sram: `words=<n> bits=<n> access_ps=<n>`
  - switch: `positions=<n> actuator=<text>`
  - led: `color=<text>`
  - 7seg: `digits=<n> commonAnode=<0|1> colonAfter=<n> dotAfter=<n> color=<t> size=<t> chip=<t> leds=<digit>:<seg>,… segmentPins=<n>,… commonPins=<n>,…`
  - oscillator: `family=<t> minHz=<n> maxHz=<n> defaultHz=<n> enablePin=<number|none>`
  - supply: `level=<0|1>`
- `pins`: `number:name:role:group` joined by `,`; role in
  `input,output,inout,power,ground,nc,absent`; an old `hidden`-group pin is
  written as role `absent`, group `default`.

Kind for old parts: definitions exported from `ff-d.h` → dff, `ff-jk.h` →
jkff, other `Logic` → gate, `Memory` → sram, `Switch` → switch, `Indicators`
→ 7seg; `led_*` → led; `osc_*` → oscillator; `supply_*` → supply.

### 4.3 Old-side tools — `backend/tools/baseline/`

- `old_listing.c` — walks `catalog_definition_at` (each part number through
  `lookup`), `led_part_*`, `osc_part_*` (band via `osc_part_in_group` /
  `osc_group_label`), `supply_part_*`; SRAM words/bits from a throwaway
  instance via `sram_instance_size`; writes §4.2 lines sorted.
- `old_trace.c` — same walk; builds a `TraceSubject` per part (switch hook =
  `switch_set_position`, lights = `led7_instance_glow` for 7-seg) and prints
  `name<TAB>%016llx`, sorted. `--verbose <part>` prints that part's change log.

Both link `logic_backend` and `tests/support/trace_core.c`. They live under
`tools/` because they die with the old backend in plan-01-G; the goldens and
`trace_core.c` stay forever as regression fixtures.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — Trace harness and golden traces

- [x] `backend/tests/support/trace_core.c` + `trace_core.h`: §4.1 exactly
      (one FNV-1a accumulator shared by probe and light strings; integers
      folded as 8 little-endian bytes).
- [x] `backend/tools/baseline/old_parts.c`/`.h` (added): the old-API walk
      shared by `old_trace` and `old_listing`.
- [x] `backend/tools/baseline/old_trace.c`: §4.3.
- [x] `backend/CMakeLists.txt`: executable `old_trace` (links `logic_backend`
      through the `old_baseline` helper library).
- [x] Generate `backend/tests/golden/traces.txt` with `old_trace > backend/tests/golden/traces.txt`.
      `wc -l` → 3817; second run `diff -q` → identical; 0 duplicate hashes.
- [x] Sensitivity spot check (not committed): change `74_00.c` HC fall delay
      `9000`→`9100` (corrected, see Corrections), rebuild, rerun → 38 lines
      differ, exactly the 38 `gate|…|NAND|HC||units=4` parts (15 `SN74HC00*`
      + CD74HC00/SN54HC00/military numbers of the same device); reverted,
      rerun `diff -q` → identical.

Acceptance: 3,817 lines, deterministic, sensitive to timing.
  Check: `wc -l < backend/tests/golden/traces.txt` → `3817`; `old_trace | diff - backend/tests/golden/traces.txt` → no output (run twice) (est. under 1 min; `catalog_check`, which runs every part, takes 1.6 s today).
  Check: the spot check above → only the 38 HC quad-NAND (`74_00`) lines differ (corrected; est. 3 min).
  Result: `3817`; two runs identical; spot check → 38 lines, all HC quad NAND.
Commit: 057c317

### Phase 2 — Golden listing

- [x] `backend/tools/baseline/old_listing.c`: §4.2/§4.3. Supplies have no
      package in the old backend: their `package` field is empty
      (`VCC|supply|||VCC|||level=1|1:VCC:power:default`).
- [x] `backend/CMakeLists.txt`: executable `old_listing`.
- [x] Generate `backend/tests/golden/parts.txt`. `wc -l` → 3817; every line
      has 9 `|` fields; every pin entry has 4 `:` fields (no separator inside
      a pin name).

Acceptance: one line per part, every kind present with today's counts.
  Check: `cut -d'|' -f2 backend/tests/golden/parts.txt | sort | uniq -c` → gate 2552, dff 672, jkff 186, sram 172, switch 36, 7seg 49, led 55, oscillator 93, supply 2 (est. 1 min).
  Check: `grep '^SN74HC00N|' backend/tests/golden/parts.txt` → `…|gate|DIP|Logic|NAND|HC||units=4|1:1A:input:default,…` (est. seconds).
  Result: `49 7seg, 672 dff, 2552 gate, 186 jkff, 55 led, 93 oscillator, 172 sram, 2 supply, 36 switch`;
  `SN74HC00N|gate|DIP|Logic|NAND|HC||units=4|1:1A:input:default,2:1B:input:default,…,14:VCC:power:default`.
Commit: f84b949

### Phase 3 — Keep the goldens honest until G

- [x] CTest `baseline_listing`: runs `old_listing` and diffs with `golden/parts.txt`.
- [x] CTest `baseline_traces`: runs `old_trace` and diffs with `golden/traces.txt`.
- [x] Move `planning/plan-01-A-parts-baseline.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: both pass on the unchanged old backend.
  Check: `ctest --test-dir /tmp/logic-build -R baseline --output-on-failure` → 2 tests passed (est. 1 min).
  Result: `100% tests passed, 0 tests failed out of 2` (6.8 s). Final gate: full `ctest` → `100% tests passed, 0 tests failed out of 7`.
Commit: this phase's commit is the one that moved this file to `planning/completed/` (hash in plan-01-B's Corrections).

## Validation Plan

- Tests: `baseline_listing`, `baseline_traces` (new CTests).
- Coverage check: the line-count checks above prove all 3,817 parts are in
  both goldens.
- Runtime proof: the timing spot check in Phase 1.
- Doc sync: none in A (docs land in B and G).
- Final gate (this sub-plan): `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → 7 tests pass (5 existing + 2 baseline) (est. 2 min).

## Decisions

- Data moves to JSON files, one per kind, all in one shared format, under
  `schematical/schematical/Parts/` — decided by owner, 2026-10-31.
- Breaking changes are allowed; cleanup or rewrite is the author's call
  (rewrite of the part layer, keep the event engine) — decided by owner, 2026-10-31.
- Vendor cJSON for parsing — decided by owner, 2026-10-31.
- Explicit `PartLibrary` handle, no global registry — decided by owner, 2026-10-31.
- The app gets parts back as C structs, not JSON — decided by owner, 2026-10-31.
- The netlist and snapshot text protocols become C calls (`circuit.h`) —
  decided by owner, 2026-10-31.
- `load_parts` takes JSON text (the author's call, delegated by the owner):
  the app reads bundle resources itself; no file I/O in the library.

## Corrections

- Phase 1 spot check: `74_00.c` has no HC `4500` delay — HC is
  `(FamilyTiming){9000, 9000}` (`74_00.c:139`; `4500` is AC's fall and AS's
  rise). Used HC fall `9000`→`9100`. The predicted changed set "`SN74HC00*`
  only" was miscalibrated: the HC timing applies to every HC part of devices
  `74_00`/`74_00_FK`, so the correct expectation is the 38 HC quad-NAND
  parts (measured: diff of the two runs → 38 names, all
  `gate|NAND|HC|units=4`; `awk -F'|' '$5=="NAND" && $6=="HC" && $8=="units=4"' parts.txt | wc -l` → 38).
- Phase 1: `old_trace` takes ~6 s CPU (22 s wall) for all 3,817 parts.

## Summary

The risk in plan-01 is silent behavior drift while 3,817 parts move from C
tables to JSON and onto new models. This sub-plan removes that risk up front:
two goldens recorded from today's backend, re-verified by CTest on every
build until G deletes the old code. Nothing in the old backend or the app
changes here.
