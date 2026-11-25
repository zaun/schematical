# plan-67-A: Unit view — what a part's units are

Last updated: 2026-11-25
Overall Effort: huge (>3d)
Effort: large (3h–1d)
Depends on: nothing

Adds a **view option** to parts on the Schematic. A part that has *units* can be
drawn two ways: **Block** (one box with every pin, as today) or as its units,
each a separate symbol that is placed, flipped and turned on its own. The first
parts with units are the logic gate ICs (Logic → Gates): their units are the
gates, each drawn with its standard logic symbol, plus one small block for the
pins no gate owns (power, ground, NC). The option for them reads **Block** /
**Gate**. A unit is named by a letter: `U1-A`, `U1-B` … for the gates and `U1-P`
for the leftover block.

It is a Schematic drawing option only. The part stays **one part**: one
reference, one footprint, one BOM line, one chip in the simulation, the same
netlist.

The mechanism is generic. Nothing in the model, the file, the canvas, the
exports or the agent tools says "gate": they deal in units, and a later plan
can give units to CPUs, flip-flops, op-amps or anything else by describing them.

The whole feature is plan-67 A–G. **This letter delivers the facts**: the
backend tells the app which pins form each gate and what the gate is, and the
app's catalog turns that into a part's list of units. Nothing is drawn or
placed differently yet.

| Letter | Delivers |
|---|---|
| A | unit facts: backend API, the `"input": "schmitt"` key, `CatalogPart.units` |
| B | the gate symbol: layout, drawing, flip and turn |
| C | the model: unit placements, geometry, wires, the edit functions, picks |
| D | the file format and the clipboard |
| E | the canvas and the sidebar: drawing, picking, dragging, the commands, the delete confirm |
| F | exports: image / PDF / print, KiCad multi-unit symbols, EasyEDA Pro |
| G | agent tools, docs, landing |

References:

- `docs/parts-format.md` § `gate` (line 1714: `gates`, `output`, `timing`, `units`), § pins (`group`, `side`, line 162).
- `docs/backend.md` (the C library, `tests/golden/`, the ctest command at line 851).
- `backend/src/models/gate.c`, `backend/src/library/kind.h`, `backend/src/library/kinds.c`, `backend/include/parts.h`.
- `schematical/schematical/Backend/PartCatalog.swift` (`CatalogPart`, `ChipPin`, `symbol(for:)`).
- `AGENTS.md` § Git (commit on the current branch; itemized commits; a new plan is one commit) and § Datasheets (a part whose facts change on purpose needs its golden line regenerated).

Test command forms used by every letter:

- `XB`: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p67-dd CODE_SIGNING_ALLOWED=NO test`; a Check writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.
- `CT`: `cmake -S backend -B /tmp/p67-build && cmake --build /tmp/p67-build -j8 && ctest --test-dir /tmp/p67-build -j8` (measured 2026-11-25 on this tree: `100% tests passed, 0 tests failed out of 74`, 28.9 s of tests, 32.6 s wall with a cold build).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Letters B–G point here.

| Must be true | Command | Status |
|---|---|---|
| The rotation work is on `main` (every part drawn as its own symbol turns; `TurnedSymbol` has `symbolPoint`, `symbolRect`, `symbolText`, `turnedSide`, `sideMargin`) | `git log --oneline -1 -- schematical/schematical/Canvas/TurnedSymbol.swift` names a commit and `grep -c 'func turnedSide' schematical/schematical/Canvas/TurnedSymbol.swift` → 1; `git status --short -- schematical/schematical/Canvas/TurnedSymbol.swift schematical/schematical/Canvas/SchematicModel.swift schematical/schematical/Canvas/ReferenceLabel.swift` → no output | MET (2026-11-25: `08376ba3 Schematic: every part drawn as its own symbol rotates…`; count 1; no output) |
| Nothing of this feature exists yet | `grep -rln 'unitPlacements\|part_unit_count\|PartUnit\b' schematical/schematical backend/include backend/src` → no output | MET (2026-11-25: no output) |
| The backend tests pass on the tree this plan starts from | `CT` → `100% tests passed` | MET (2026-11-25: 74 of 74) |
| KiCad's command-line tool is installed (letter F checks its export with it) | `~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli version` → `10.0.8` or later | MET (2026-11-25: `10.0.8`) |
| The plan number is free | `ls planning planning/completed \| grep -c '^plan-67'` → the 7 files of this plan and no others | MET (2026-11-25: 0 before these files were written) |

Everything below is written against the world where these hold. Line numbers in
plan-67 are from the tree at `08376ba3`; the symbol names are the reference, and
a line number that has moved is not a correction.

Two unlanded plans name files this feature also edits (`grep -ln
'SchematicModel.swift\|parts.h' planning/plan-*.md` → plan-48-A/E/F/H,
plan-66-B). Neither is a prerequisite: plan-48 adds a part kind and reads
`PlacedChip.ohms`, plan-66 cites one PCB function in `SchematicModel.swift`; no
design here rests on either, and neither rests on this.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `part_unit_count(part)` / `part_unit_at(part, i)` in the backend return, for
  every gate part, its gates as units: the shape, the input pins, one or two
  output pins each marked inverted or not, whether the inputs are Schmitt
  triggers and whether the outputs are open drain. Every other part has 0 units.
- `CatalogPart.units` in the app lists a gate part's units in order — the gate
  units keyed `A`, `B`, … then one block unit keyed `P` holding every drawn pin
  no gate owns — and is empty for every part that has no unit view.
- For all 126 gate devices, the number of gate units equals the part's existing
  `units` count, and the units' pins partition the part's drawn pins.

### Non-goals (explicit constraints)

- No simulated behavior changes. `"input": "schmitt"` is a fact for the symbol
  only; the gate model does not read it.
- `backend/tests/golden/parts.txt`, `traces.txt`, `erc_pins.txt` and
  `footprints.txt` do not change: `tests/support/listing.c` is not edited.
- No catalog part is added or removed, so the in-stock rule of `AGENTS.md`
  § Datasheets does not come into play.
- `PartLogic.units` (the drawer's count) keeps its meaning and its value.
- ~~The Part Editor gets no Schmitt control~~ — corrected: it gets one choice
  field (see Corrections).
- Nothing is drawn, placed, saved or exported differently in this letter.

## 2. Current State

- A gate device lists its gates: `"gates": [{"op","inputs","output"}]`
  (`docs/parts-format.md:1724`). `backend/src/models/gate.c:163 gate_load_device`
  parses them into `GateDef`; `gate_bind_pinout` (`:248`) binds the names to
  engine pin indexes (`GatePins`) per pinout. `"output": "open-drain"` sets
  `GateDevice.open_drain` (`:204`). Device keys are checked against
  `DEVICE_KEYS` (`:28`: `gates`, `output`, `timing`, `units`); an unknown key is
  a load error.
- The app sees none of that. `backend/include/parts.h:57` exposes
  `typedef struct { const char *family; unsigned units; } PartLogic;` and the
  app keeps `case logic(family: String, units: Int)`
  (`Backend/PartCatalog.swift:600`, filled at `:1349`). The gates live behind
  `Part.internal` ("model data; not for the app").
- A pin's `"group": "gate-1"` only clusters pins in the block. It is on gate
  **inputs** only (396 pins), never on an output, and 88 of the 126 gate devices
  have no group at all, so the groups cannot say which pins form a gate.
- Kinds answer per-pin questions through optional hooks on `PartKindOps`
  (`backend/src/library/kind.h:28`): `pin_open_drain`, `pin_analog`,
  `pin_tristate`, each wrapped by a public function in
  `backend/src/library/kinds.c:118-148`. `GATE_KIND` (`gate.c:477`) initializes
  the struct positionally and already leaves the trailing hooks NULL. That is
  the precedent the unit hook follows.
- The app reads the C structs directly through its bridging header
  (`schematical/schematical/schematical-Bridging-Header.h` includes `parts.h`);
  `CatalogPart.init(_ part: Part, …)` is at `PartCatalog.swift:1310`.
- A Schmitt-trigger gate is known only by its description text.

### Measured populations

| What | Count | Command |
|---|---|---|
| Gate devices, all in drawer Logic → Gates | 126 | `python3 planning/plan-67-data/census.py` (reads `Parts/gate.json`; run from the repo root): `devices 126`, `drawers {('Logic','Gates'): 126}`; no other part file has a device in that drawer |
| Orderable gate parts | 3564 | same script: sum of `len(device["parts"])` |
| Gates by op | not 142, buf 80, nand 34, nor 28, and 28, or 26, xor 17, xnor 8 | same script |
| Units per device (gates merged when their input lists are equal) | 1: 45, 2: 24, 3: 16, 4: 15, 6: 26 | `python3 -c` over `gate.json`: `len(set(tuple(g["inputs"]) for g in gates))` |
| Devices where that merged count differs from the device's `units` (default: gate count) | 0 | same: `mismatches 0` |
| Devices with two gates on the same inputs | 3 (4068B: nand + and; 4078B: nor + or; 4041UB: 4 × buf + not) | census: `odd [('4068B',…),('4078B',…),('4041UB',…)]` |
| Largest input count | 13 (74_133), then 8 (74_30 class, 4068B, 4078B) | census shapes table |
| Open-drain devices | 19 | census: `('output','"open-drain"'): 19` |
| Devices whose description says Schmitt | 20 (287 orderable parts) | `python3`: `"chmitt" in description` → 4093B, 74_14, 74_6T14, 74_19, 1G14, 1G14_SON, 1T14, 2G14, 3G14, 3G14_RSE, 40106B, 1G16, 1G17, 1G37, 1T17, 1T50, 2G17, 3G17, 74_17, 74_6T17 |
| Pins no gate owns, per pinout | 2 to 11 (2: 128 pinouts, 3: 29, 4: 22, 5: 1, 6: 1, 8: 14, 10: 3, 11: 1); names GND, VCC, VSS, VDD, NC, DNU only | census: `non-gate pins per pinout`, `non-gate pin names` |

### Verified properties

- **"Gates with equal input lists are one unit" reproduces every device's
  `units`.** Verified by the script above over all 126 devices: 0 mismatches.
  So the unit list needs no new catalog key.
- **The trailing-hook pattern compiles.** Read `gate.c:477-480`: `GATE_KIND`
  gives 9 of the struct's 11 members; the rest are zero. A twelfth member added
  at the end is NULL for every kind that does not name it.
- **An unknown device key fails the load.** Read `gate.c:28` and
  `kind_check_keys`: `"input"` must be added to `DEVICE_KEYS` before any device
  uses it.
- **The goldens do not print gates.** Read `tests/support/listing.c:120-131`: a
  gate part's kind field is `units=%u` only.
- **Every pinout has at least 2 pins outside its gates** (table above), so the
  `P` unit always exists for a gate part.

## 3. Design Overview

Three pieces, each landable alone:

1. **Backend API** — a kind hook `units` plus `part_unit_count` /
   `part_unit_at`. The gate kind builds its units when it binds a pinout.
2. **The Schmitt key** — `"input": "schmitt"` on a gate device, beside
   `"output": "open-drain"`, set on the 20 devices above.
3. **`CatalogPart.units`** — the app's list, with the leftover block appended.

Correctness risk is low and sits in the merge rule (two gates on the same
inputs become one unit with two outputs). It is covered by a whole-catalog test.

Byte-identity is **not** this letter's gate; the goldens are expected to stay
unchanged only because nothing they print is touched, and
`git diff --stat backend/tests/golden` → empty is one of the checks.

Rejected:

- *Derive units from `"group": "gate-N"`.* Outputs carry no group and 88 devices
  have none (§2).
- *Have the app read `gate.json` itself.* Custom parts and every later kind would
  need a second parser; the backend already binds names to pin indexes per pinout.
- *Detect Schmitt from the description.* A drawing fact would hang on prose.
- *A `"units"` array in the part file naming each unit's pins.* Redundant for
  gates (the gates are the units); a later kind that needs hand-described units
  adds it then, behind the same API.

## 4. The backend API

In `backend/include/parts.h`, after `PartLogic`:

```c
/* A unit: a piece of a part the Schematic can draw as its own symbol
   (plan-67). Pins are engine pin indexes. */
typedef enum { UNIT_AND, UNIT_OR, UNIT_XOR, UNIT_BUFFER } PartUnitShape;
typedef struct { uint16_t pin; uint8_t inverted; } PartUnitOutput;
typedef struct {
    PartUnitShape shape;
    const uint16_t *inputs; size_t input_count;      /* in the gate's "inputs" order */
    PartUnitOutput outputs[2]; size_t output_count;  /* 1 or 2; the true output first */
    unsigned schmitt;                                /* inputs are Schmitt triggers */
    unsigned open_drain;                             /* outputs are open drain */
} PartUnit;

size_t part_unit_count(const Part *part);                       /* 0 = the part has no units */
const PartUnit *part_unit_at(const Part *part, size_t index);   /* NULL past the end */
```

`PartKindOps` gains a last member
`const PartUnit *(*units)(const Part *part, size_t *count);` (NULL = no units).
`kinds.c` implements the two public functions over it, next to
`part_pin_open_drain`.

The gate kind fills units in `gate_bind_pinout`, stored on `GateBound`:

| op | shape | output inverted |
|---|---|---|
| `and` / `nand` | `UNIT_AND` | no / yes |
| `or` / `nor` | `UNIT_OR` | no / yes |
| `xor` / `xnor` | `UNIT_XOR` | no / yes |
| `buf` / `not` | `UNIT_BUFFER` | no / yes |

Walk the gates in file order. A gate whose bound input list equals that of an
earlier gate **with the same shape** joins that gate's unit as its second
output; otherwise it starts a new unit. Within a unit the non-inverted output
comes first. A third gate on the same inputs, or two gates on the same inputs
with the same inversion, is a load error:
`"gates[%zu]": more than two gates share these inputs` /
`gates on the same inputs must be a true and an inverted output`.
`schmitt` and `open_drain` are copied from the device onto every unit.

## 5. The Schmitt key

`"input"` on a gate device: `"standard"` (default) or `"schmitt"`; anything else
is `"input" must be "standard" or "schmitt"`. Added to `DEVICE_KEYS`, stored as
`GateDevice.schmitt`. Set to `"schmitt"` on the 20 devices of §2's table.

## 6. `CatalogPart.units`

In `Backend/PartCatalog.swift`:

```swift
nonisolated struct GateShape: Hashable, Sendable {
    enum Body: Hashable, Sendable { case and, or, xor, buffer }
    struct Output: Hashable, Sendable { let pin: Int; let inverted: Bool }
    let body: Body
    let inputs: [Int]          // ChipPin.index, top to bottom
    let outputs: [Output]      // 1 or 2, the true output first
    let schmitt: Bool
    let openDrain: Bool
}
nonisolated enum UnitShape: Hashable, Sendable { case gate(GateShape), block }
nonisolated struct PartUnit: Hashable, Sendable, Identifiable {
    let key: String            // "A", "B", … or "P"
    let pins: [Int]            // ChipPin.index of every pin the unit draws
    let shape: UnitShape
    var id: String { key }
}
```

`CatalogPart` gets `let units: [PartUnit]` and
`var unitViewName: String? ` (`"Gate"` when any unit is a `.gate`, else nil).
`init(_ part: Part, …)` builds it:

- one `PartUnit` per backend unit, shape `.gate`, `pins` = its inputs then its
  outputs, key = the next letter of `A…Z` **skipping `P`**;
- if there is at least one such unit: a last unit keyed `P`, shape `.block`,
  `pins` = every pin that is not hidden (`ChipPin.isHidden`) and is in no gate
  unit, in pin-index order;
- a part with no backend units has `units == []`.

A hidden pin is never in a unit's `pins`.

## Compatibility / Format Impact

- Part file format: one new optional device key on the `gate` kind,
  `"input"`. Files without it load as before.
- C API: three new types and two new functions in `parts.h`; nothing removed.
- `.schematical` files, the agent API and every export: unchanged in this letter.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial with one line on what remains; mark a
> moot task `- [x] ~~text~~ — moot: <evidence>`; fill each `Commit:` line when
> the phase lands; add any task you discover you needed. **An unticked box means
> NOT DONE.**

### Phase 1 — backend units

The API with no caller; safe alone because nothing reads it yet.

- [x] `backend/include/parts.h`: add `PartUnitShape`, `PartUnitOutput`,
      `PartUnit`, `part_unit_count`, `part_unit_at` as in §4.
- [x] `backend/src/library/kind.h`: add the `units` member at the end of
      `PartKindOps`, with a comment in the style of `pin_tristate`.
- [x] `backend/src/library/kinds.c`: implement the two public functions after
      `part_pin_open_drain` (NULL part, NULL `internal` or NULL hook → 0 / NULL).
- [x] `backend/src/models/gate.c`: add `PartUnit *units; size_t unit_count;` and
      the input index storage to `GateBound`; fill them in `gate_bind_pinout`
      by §4's rule with the two load errors; add `gate_units` and name it as
      the last member of `GATE_KIND`.
- [x] Tests: new `backend/tests/part_unit_test.c`, registered with
      `logic_test(part_unit_test)` in `backend/CMakeLists.txt` beside
      `logic_test(listing_check)`. It loads the library the way
      `listing_check.c:40-54` does (`part_library_create`, `load_parts_dir`) and
      asserts: `SN74HC00N` → 4 units, each `UNIT_AND`, 2 inputs, 1 inverted
      output, and unit 0's pins are the indexes of `1A`, `1B`, `1Y`; `CD4068BE`
      → 1 unit, `UNIT_AND`, 8 inputs, 2 outputs, `K` not inverted first, `J`
      inverted second; `CD4041UBE` → 4 units, `UNIT_BUFFER`, 1 input, 2 outputs;
      `SN74ALS133N` → 1 unit with 13 inputs; `SN74HC266N` → `open_drain` 1 and
      `UNIT_XOR` inverted; `SN74HC74N` (a dff) → 0 units; and for **every**
      part of kind `PART_GATE`: `part_unit_count == part->logic->units`, no pin
      index appears in two units, every input pin has role `PIN_INPUT` and every
      output pin `PIN_OUTPUT`. Two negative cases through `load_parts` on inline
      JSON: three gates on the same inputs, and two `nand` gates on the same
      inputs, each failing with its message.
      **Done:** the `SN74HC14N` Schmitt assertion is added in Phase 2 with the key. `ctest` → `100% tests passed, 0 tests failed out of 75`; `listing_check --print | diff - backend/tests/golden/parts.txt` → no output.

Acceptance: the new test passes and no golden moves.
  Check: `CT` → `100% tests passed, 0 tests failed out of 75`; then
  `/tmp/p67-build/listing_check --print | diff - backend/tests/golden/parts.txt`
  → no output (est. 1 min).
Commit: d180c8e2

### Phase 2 — the Schmitt key

- [x] `backend/src/models/gate.c`: `"input"` in `DEVICE_KEYS`; parse it in
      `gate_load_device` next to `"output"` (§5); `int schmitt` on `GateDevice`;
      copy it onto each unit in `gate_bind_pinout`. Update the file's header
      comment ("Two device options" becomes three).
- [x] `schematical/schematical/Parts/gate.json`: add `"input": "schmitt"` to the
      20 devices listed in §2, directly after each device's `"gates"` key.
      **Done:** `/tmp/p67/schmitt.py` (line-based, layout kept) printed `20 20 0`: 20 lines added, 20 devices carry the key, 0 devices where the key and the description disagree.
- [x] `docs/parts-format.md` § `gate`: a row for `"input"` in the device-key
      table after `"output"`, and the key in the example.
      **Done:** also a **Units** paragraph on the merge rule and its two load errors.
- [x] Tests: in `part_unit_test.c`: `SN74HC14N` → `schmitt` 1 on all 6 units;
      `SN74HC00N` → 0; a negative inline case `"input": "slow"` failing with
      `"input" must be "standard" or "schmitt"`.
      **Done:** `ctest` → `100% tests passed, 0 tests failed out of 75`; `git diff --stat backend/tests/golden` → no output.

- [x] Added: `schematical/schematical/Editor/GateDraft.swift` gets an `input` field (choice `standard` / `schmitt`) so the Part Editor's schema knows the key (see Corrections).

Acceptance: exactly the 20 devices carry the key and the library still loads.
  Check: `python3 -c 'import json;d=json.load(open("schematical/schematical/Parts/gate.json"))["devices"];print(sum(1 for x in d if x.get("input")=="schmitt"), sum(1 for x in d if ("chmitt" in x["description"])!=(x.get("input")=="schmitt")))'`
  → `20 0`; `CT` → `100% tests passed`; `git diff --stat backend/tests/golden`
  → no output (est. 1 min).
Commit: —

### Phase 3 — the app's unit list

- [ ] `schematical/schematical/Backend/PartCatalog.swift`: add `GateShape`,
      `UnitShape`, `PartUnit` (§6) above `CatalogPart`; add `let units` and
      `unitViewName`; build `units` in `init(_ part: Part, …)` after `pins` from
      `part_unit_count` / `part_unit_at`. Every other `CatalogPart` initializer
      in the file passes `units: []`.
- [ ] `docs/backend.md`: one paragraph on the unit API under the public API
      section, naming plan-67.
- [ ] Tests: new `schematical/schematicalTests/PartUnitTests.swift`, using the
      catalog the way `SchematicRotateTests` gets its parts. Cases:
      `testQuadNand` (`SN74HC00N`: keys `["A","B","C","D","P"]`; `A` is `.gate`
      with body `.and`, one inverted output, pins named `1A`, `1B`, `1Y`; `P`
      is `.block` with the pins named `VCC` and `GND`);
      `testTwoOutputs` (`CD4068BE`: keys `["A","P"]`, 2 outputs, the first not
      inverted); `testSchmittAndOpenDrain` (`SN74HC14N` schmitt, `SN74HC266N`
      open drain); `testEveryGatePart` (for every catalog part of kind
      `PART_GATE`: `units.count - 1 ==` its `.logic` units, the last key is
      `"P"`, the union of `units.pins` equals the set of non-hidden pin indexes,
      and no index is in two units); `testOtherPartsHaveNone` (`SN74HC74N`:
      `units.isEmpty`, `unitViewName == nil`).

Acceptance: the catalog holds a unit list for every gate part and for nothing else.
  Check: `XB -only-testing:schematicalTests/PartUnitTests` → `** TEST SUCCEEDED **`
  (est. 4 min, most of it the build).
Commit: —

## Validation Plan

- Tests: `backend/tests/part_unit_test.c` (positive, whole-kind sweep, three
  negative loads); `schematicalTests/PartUnitTests` (five cases).
- Coverage check: `part_unit_test` iterates every `PART_GATE` part, so all 126
  devices and each of their pinouts go through `gate_bind_pinout`'s new code.
- Runtime proof: none beyond the tests; nothing user-visible changes in A.
- Doc sync: `docs/parts-format.md` (`"input"`), `docs/backend.md` (unit API).
- Final gate: the feature's landing gate is in plan-67-G and runs once, there.

## Decisions

- **The option is a Schematic view only; the part stays one part** — decided by
  the owner, 2026-11-25. Settles that the netlist, PCB, BOM and simulation are
  untouched by every letter.
- **Units are lettered, with a separator** — decided by the owner, 2026-11-25,
  who left the separator to the author. It is `-`: `U1-A`. `:` is refused by the
  agent's item parser (`AutomationSchematic.swift:914` reads the text before the
  first colon as an item kind); `/` and `#` already have a meaning inside pin
  names (`A0/CS` alternatives, `CE#`); a reference is letters then digits, so a
  `-` after it is unambiguous. (plan-67-C §4)
- **The leftover block is unit `P`** — decided by the owner, 2026-11-25. Gate
  letters skip `P`. (§6)
- **Which parts get the option: Logic → Gates, any gate with a known logic
  symbol** — decided by the owner, 2026-11-25. That is the `gate` kind: all 126
  devices; AND, OR, XOR and their inverted forms, buffers and inverters.
- **Built generically** — decided by the owner, 2026-11-25: "this should not be
  a gate only thing". Settles the `PartUnit` / `UnitShape` split and the kind
  hook.
- **Schmitt-trigger and open-drain gates carry their mark** — the author's
  call, following "a known logic symbol". Settles §5.

## Corrections

- **The Part Editor needs the key.** The plan's non-goal said the editor gets no Schmitt control. False: `Editor/GateDraft.swift` holds the gate kind's `KindSchema`, the list of device keys the editor reads and writes (`grep -n 'key: "output"' schematical/schematical/Editor/GateDraft.swift`), so a key missing there is a key the editor cannot keep when a bundled Schmitt part is copied into the user library. One `Field(key: "input", …choice…)` was added in Phase 2. No design rested on the non-goal.

- **The build folder.** `CT` names `/tmp/p67-build`, which was configured from the main checkout while the plan was written; cmake refuses it from the worktree (`The source … does not match the source … used to generate cache`). Every letter's `CT` runs in `/tmp/p67w-build` instead. Nothing else changes.
- **Prerequisites re-run 2026-11-25 in the worktree, all MET**: rotation commit `08376ba3`, `turnedSide` count 1, tree clean; no `unitPlacements` / `part_unit_count` / `PartUnit`; `ctest` 74 of 74; `kicad-cli version` `10.0.8`; `planning` holds only this plan's 7 files and `plan-67-data`.

## Summary

The only judgement in this letter is the merge rule for two gates on the same
inputs, and the whole catalog is swept by a test for it. Simulation, the
goldens and every existing key are untouched.
