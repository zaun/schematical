# plan-02-A: Pin designators in the backend and the app

Last updated: 2026-10-31
Overall Effort: large (3h–1d) — plan-02 A + B + C
Effort: large (3h–1d)
Depends on: nothing (plan-01 complete)

Plan-02 makes the part library, the circuit API, the schematic file and the
app name a pin by its **designator**: the package's own label for it. That is
`"14"` on a DIP and `"A1"` or `"H6"` on a BGA. Today every pin is an integer
1..N, so the library's 58 BGA parts fake their balls. The 15 SRAM TFBGAs
number their balls row-major as 1..N. The 43 TI DSBGAs reuse the SOT-23/SON
pinout numbers.

This sub-plan (A) changes **how pins are identified** without changing any
part. It adds:

- `PartPin.designator` in place of `PartPin.number`;
- a `"ball"` pin key in the part files, loaded and checked;
- the oscillator `enablePin` and the 7-segment side lists by designator and
  index;
- `circuit_connect` by designator;
- the app on pin index + designator, with the schematic file still version 1.

Outcome: every part and every trace is byte-identical, the app builds and
behaves as before, and a part file may now contain ball pinouts.

Sub-plans, in implementation order:

- **plan-02-A** (this): the backend and app cutover, behavior-neutral.
- **plan-02-B**: schematic file version 2, which saves pins as designator
  strings and converts version 1 on open.
- **plan-02-C**: the part data. Converts 12 SRAM TFBGA pinouts and adds a
  ball pinout to 25 TI DSBGA devices.

References:

- `docs/parts-format.md` (Pinouts and pins; `oscillator`; `7seg`).
- `docs/backend.md` (API example, tests, golden regeneration).
- `docs/file-format.md` (wire ends; unchanged in A, changed in B).
- `backend/include/parts.h`, `backend/include/circuit.h`.
- `AGENTS.md` (git rules; run the backend tests after part edits).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-01 complete (all letters archived) | `ls planning/plan-01-* 2>/dev/null` → no output | MET (2026-10-31) |
| Backend tests green on `main` | `cmake -S backend -B /tmp/logic-bga && cmake --build /tmp/logic-bga -j8 && ctest --test-dir /tmp/logic-bga` → `100% tests passed, 0 tests failed out of 7` | MET (2026-10-31, 18.9 s) |
| App builds on `main` | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-bga-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-10-31, clean build 29 s) |

Everything below is written for the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all*
> prerequisites.

## 1. Goal

- A part file may describe a pinout with `{"ball": "A1", …}` pins. It loads,
  its kind keys resolve by name, and `circuit_connect(c, part, "A1", net)`
  wires that ball.
- With the part files unchanged, `listing_check` and `trace_check` pass
  against the **unchanged** `backend/tests/golden/` (byte-identical output),
  and the app opens, draws, saves and simulates `test.schematical` exactly as
  before.

### Non-goals (explicit constraints)

- No part file changes. That is plan-02-C.
- `.schematical` stays version 1 with integer `"pin"`; the bytes the app
  writes do not change. Version 2 is plan-02-B.
- Numbered pinouts keep their rules: `"number"` 1..N in order, and
  `absent` is allowed.
- Kind keys still name pins by `name`. No kind's JSON keys change, except that
  `enablePin` may also be a string (§4.3).
- Engine (`backend/src/engine/`) and model behavior are untouched. Models
  already work on 0-based pin indices.
- `basicsim/` is untouched. It wires engine instances by index (`basicsim/main.c:160-168`) and never reads `PartPin.number`.

## 2. Current State

- **Pin struct.** `parts.h:PartPin` is `{unsigned number; const char *name; PinRole role; const char *group;}`.
- **Loader.** `library.c:load_pinout` requires each pin's `"number"` to equal
  its array position + 1 (`"pin numbers must be 1..N in order"`). `PIN_KEYS`
  is `{"number","name","role","group"}`. So **array position == number − 1
  == the engine pin index** everywhere.
- **Kind binding** is by name: `kind_util.c:kind_pin` / `kind_pins` return
  indices, and models store indices (`dff.c:bind_pin`, `led.c`, `gate.c`, …).
  Only two kinds expose pin *numbers* in public structs:
  - `PartDisplay.segment_pins` / `common_pins` are `const unsigned *` of pin
    numbers, filled by `display7.c:expand_side`
    (`numbers[n++] = pins[indices[j]].number`).
  - `PartOscillator.enable_pin` is `unsigned`, a pin number with 0 meaning
    none. It is parsed in `oscillator.c:load_part` with
    `kind_uint(…, out->pin_count, …)`, and the model converts it to an
    index: `oscillator.c:osc_create`, `enable_pin - 1`.
- **Circuit API.** `circuit.c:circuit_connect(Circuit*, size_t part, unsigned pin_number, size_t net)`
  checks 1..pin_count and `PIN_ABSENT`, then `sim_connect(…, pin_number - 1)`.
- **Listing golden.** `tests/support/listing.c:listing_line` prints each pin
  as `%u:%s:%s:%s` (number:name:role:group). It prints 7-segment side lists
  with `print_numbers` and `enablePin=%u` or `none`. `trace_check.c:trace_part`
  hashes behavior by pin index and role only; no pin labels go into
  `traces.txt`.
- **App.** Pin identity is `ChipPin.number`
  (`Backend/PartCatalog.swift:38`, `id` = number). It flows into
  `PinSlot.id`, `PinRef.number` (`Canvas/SchematicModel.swift:484`), layout
  sorting and lookup (`SchematicModel.swift:193-197, 223, 300`), the three
  label views (`ChipBlockView.swift:72`, `SevenSegmentSymbolView.swift:59`,
  `DipSwitchSymbolView.swift:57`), the file (`Document/SchematicFile.swift:56-80, 238-246, 286`)
  and the netlist (`Simulation/Simulation.swift:22, 89-92, 206`).
  `DisplayInfo.segmentPins/commonPins` are numbers (`PartCatalog.swift:75-76, 233-234`).
  `enablePin: Int?` is set at `PartCatalog.swift:239`; no other Swift reads it.

### Measured populations

| What | Count | Command |
|---|---|---|
| `circuit_connect(` call sites in tests | 29 | `grep -c "circuit_connect(" backend/tests/circuit_check.c` → 29 |
| `circuit_connect` call sites in the app | 1 | `grep -rn circuit_connect schematical/schematical` → `Simulation.swift:206` |
| C reads of `.number` / `enable_pin` / side lists outside `src/` | 9 | `grep -n "\.number\b\|pin_number\|enable_pin\|segment_pins\|common_pins" backend/tests/*.c backend/tests/support/*.c` → circuit_check.c:129, sram_test.c:58, library_check.c:94,409,410, listing.c:66,68,77,78 |
| Swift lines using pin numbers | 22 | `grep -rnE 'pin\.number|ref\.number|key\.number|byNumber|let number: Int' schematical/schematical --include='*.swift' \| wc -l` → 22 (the lines cited above) |
| Parts / trace lines in the goldens | 3817 / 3817 | `grep -c . backend/tests/golden/parts.txt backend/tests/golden/traces.txt` |
| Oscillator parts with `enablePin` | 91 | `grep -c '"enablePin"' schematical/schematical/Parts/oscillator.json` → 91 |

### Verified properties

- **A string parameter catches every old `circuit_connect` call.** Probe:
  `int f(const char *p); int g(void){ return f(1); }` compiled with `cc -c`
  gives `error: incompatible integer to pointer conversion … [-Wint-conversion]`, rc=1.
  The one literal that would slip through is `0` (a null pointer constant).
  `grep -n "circuit_connect(c, [a-z_]*, 0," backend/tests/circuit_check.c`
  finds no matches, because pins were 1-based, and `circuit_connect`
  rejects a NULL pin (§4.4).
- **Printing the designator keeps `parts.txt` byte-identical.** For a
  numbered pin the designator is the decimal string of its number, so
  `"%s"` prints the same bytes as `"%u"`. Side lists and `enablePin` become
  designators of the same pins. `traces.txt` never contains pin labels
  (`trace_check.c:53-66` passes roles by index).
- **Models never see pin numbers** except the oscillator enable pin
  (`oscillator.c:130`). Every other model gets indices from `kind_pin` /
  `kind_pins` (read: `dff.c:171-201`, `led.c:37-43`, `display7.c:298-351`).

## 3. Design Overview

Three layers, each already index-based inside:

1. **Library (C):** a pin has a *designator* (string) and a *position*
   (array index). The position is the engine pin index. For numbered pinouts
   the designator is `"<number>"` and the position is `number - 1`. For ball
   pinouts the designator is the ball name and the position is the order the
   part file lists the balls in.
2. **Circuit API:** connects by designator, which is what a person or a file
   names.
3. **App:** identifies a pin by position (`ChipPin.index`), shows the
   designator, and connects by designator. The file stays version 1, where
   `"pin": n` means position n − 1. That is exactly today's meaning, because
   today position == number − 1.

**Correctness gate: byte-identity.** A is provably neutral: no part data
changes, and every printed designator equals the old number. The gate is
`listing_check` and `trace_check` passing against **untouched** goldens
(`git diff --quiet backend/tests/golden`), plus the app build and the
runtime proof. If a listing line differs, the designator or index mapping has
a bug. Find it by running `listing_check --print` and diffing **one** part's
line. Fix it, never regenerate.

Risk concentrates in the app (§4.6). Off-by-one between position and
number in the file load/save path, layout lookup, and the netlist. Those
are last in the phase order and covered by the runtime proof.

**Rejected alternatives**

- *Keep `PartPin.number` and add `designator` beside it.* A ball pin has no
  meaningful number, so the field would be a second, misleading position.
  Removing it lets the compiler find every consumer (C and Swift).
- *`circuit_connect` by 0-based index.* Changing `unsigned` (1-based) to
  `size_t` (0-based) compiles silently at all 29 test sites with
  off-by-one meaning. A `const char *` turns every stale call into a
  compile error (probe above).
- *Rows sorted / validated in grid order for ball pinouts.* Position is the
  engine index, the v1-file meaning (plan-02-B) and the trace order. Forcing
  row-major order would break traces and old files for converted parts
  (plan-02-C). So the loader leaves ball order free.

## 4. Detailed Design

### 4.1 `PartPin` and the loader (`parts.h`, `library.c`)

```c
typedef struct {
    const char *designator;  /* the package's label: "14" (numbered) or "A1" (ball) */
    const char *name; PinRole role; const char *group;
} PartPin;
```

`PIN_KEYS` becomes `{"number","ball","name","role","group"}`. In
`load_pinout`, each pin has exactly one of `"number"` or `"ball"`, and the
first pin fixes the pinout's style. The messages below are the exact text;
the loader's path prefix (`… pinouts.bga: pin 3:`) is added by `kind_error`:

| Case | Message |
|---|---|
| both keys | `a pin has "number" or "ball", not both` |
| neither key | `a pin needs "number" or "ball"` |
| style differs from pin 1 | `every pin of a pinout uses "number", or every pin uses "ball"` |
| numbered, wrong order | `pin numbers must be 1..N in order (found %llu)` (unchanged) |
| bad ball name | `"%s" is not a ball name` |
| duplicate ball | `ball "%s" is used twice` |
| `absent` in a ball pinout | `a ball pinout lists only real balls; leave "absent" positions out` |

- **Numbered pin:** designator = `kind_strdup` of the decimal number.
- **Ball name:** 1–2 row letters from `ABCDEFGHJKLMNPRTUVWY` (JEDEC JEP95
  rows: no I, O, Q, S, X, Z), then a column of 1–99 with no leading zero, and
  nothing else. `static int is_ball_name(const char *)` in `library.c`.
- Duplicate check: compare against the earlier pins of the same pinout (at
  most 90 in today's data; linear is fine).

Comment on `PIN_ABSENT` in `parts.h` and on `ChipPinRole.absent`: "a
numbered position with no lead". Ball pinouts never use it.

### 4.2 7-segment side lists (`parts.h`, `display7.c`)

`PartDisplay.segment_pins` / `common_pins` become `const size_t *`: 0-based
indices into `Part.pins`. `expand_side` stores `indices[j]` instead of
`pins[indices[j]].number`, and its comment reads "Expands side names to pin
indices".

### 4.3 Oscillator `enablePin` (`parts.h`, `oscillator.c`)

`PartOscillator.enable_pin` becomes `const char *enable_pin; /* the pin's designator, NULL = none */`.
It points at the matching `Part.pins[i].designator`.

In `load_part`, `"enablePin"` is either a JSON integer, for numbered
pinouts, formatted as its decimal string, or a JSON string, the designator
as written in the pinout. The loader finds the pin of the part's pinout
whose designator equals it. A missing pin gives
`"enablePin" %s is not a pin of pinout "%s"`, and any other type gives
`"enablePin" must be a pin number or a ball name`.

`osc_create` sets `state->enable_pin` to the index `i` whose
`part->pins[i].designator == part->oscillator->enable_pin` (pointer
equality), or −1.

### 4.4 Circuit API (`circuit.h`, `circuit.c`)

```c
/* pin is the pin's designator, e.g. "1" or "A1". */
int circuit_connect(Circuit *circuit, size_t part, const char *pin, size_t net);
```

The connect logic:

- Find `i` with `strcmp(info->pins[i].designator, pin) == 0`.
- Errors: NULL gives `no pin given`; not found gives `%s has no pin %s`;
  absent gives `pin %s of %s has no lead`. That keeps today's text for
  numbered pins: `"pin 10 of LTC-4727JR has no lead"`.
- Then `sim_connect(…, i)`.
- Update the header's usage comment to `circuit_connect(c, vcc, "1", net)`.

### 4.5 Test support (`tests/support/listing.c`, tests)

- Pins print as `%s:%s:%s:%s` with the designator first.
- `print_numbers(out, pins, indices, count)` prints `pins[indices[i]].designator`.
- `enablePin=` prints the designator or `none`.
- Output for every current part is unchanged (§2 Verified properties).

### 4.6 App (`schematical/schematical/`)

- `Backend/PartCatalog.swift`:
  - `ChipPin` has `let index: Int` (position in the pinout, 0-based) and
    `let designator: String`, and `id` is `index`. `number` is removed.
  - The copy loop sets `index` from the loop index and `designator` from
    `pin.designator`.
  - `DisplayInfo.segmentPins/commonPins` hold **indices** into `pins`; the
    `numbers(_:_:)` helper becomes `indices(UnsafePointer<Int>?, Int)`
    (Swift imports `size_t` as `Int`).
  - `Details.oscillator(… enablePin: String?)` comes from
    `osc.enable_pin.map(String.init(cString:))`.
- `Canvas/SchematicModel.swift`:
  - `PinSlot.id` is `pin.index`, and `PinRef` is `{chipID, index: Int}`.
  - Layout sorts by `index`, and every `byNumber` dictionary is keyed by
    `index` (`byIndex`).
  - 7-segment lookups use the index lists.
  - DIP switch position k takes the pins at index `k-1` (left) and
    `2N-k` (right), the same pins as numbers k and 2N+1−k today.
  - `pinTip`, `pinTips` and `pin(at:)` use `index`.
- Views: the three `Text("\(slot.pin.number)")` calls become
  `Text(slot.pin.designator)`. The label margin is unchanged: the widest
  ball name in plan-02-C's data, `"H6"` (2 characters), measures 12.7 pt at
  9 pt system font against 11.9 pt for `"48"` (probe: `NSString.size` in
  `/tmp/textw.swift`).
- `Document/SchematicFile.swift` (still version 1): decoding `"pin": n`
  resolves to `PinRef(index: n - 1)` after checking a drawn slot has
  `pin.index == n - 1`. Encoding writes `ref.index + 1`. `missingPin` keeps
  `pin: Int` (the file's value).
- `Simulation/Simulation.swift`: `connections` holds
  `(part: Int, pin: String, net: Int)`, with the designator taken from
  `chips[part].part.pins[ref.index].designator`. It is sorted by
  `(ref.index, uuid)`, the same order as today. It is passed straight to
  `circuit_connect`, since a Swift `String` bridges to `const char *`.

## Compatibility / Format Impact

- **Public C API (breaking, in-repo consumers only):** `PartPin.number` →
  `designator`; `PartDisplay.*_pins` → `size_t` indices;
  `PartOscillator.enable_pin` → `const char *`; `circuit_connect` takes a
  designator. Consumers: `backend/tests/`, the app. `basicsim` does not use
  them.
- **Part file format (additive):** the new `"ball"` pin key and string
  `enablePin`. Every existing file loads unchanged.
- **Unchanged:** `.schematical` bytes, both goldens, engine, models' behavior.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. Use `- [~]` for partial work, plus one line on what
> remains. Mark moot tasks `- [x] ~~text~~ — moot: <evidence>`. Fill
> `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Designators and ball pinouts in the library

The C library changes alone, with the tests updated in the same commit. The
app is not built in this phase; it is fixed in Phase 2 and the two land
back to back.

- [ ] `backend/include/parts.h`: `PartPin.designator` (§4.1), `PartDisplay.segment_pins/common_pins` as `const size_t *` (§4.2), `PartOscillator.enable_pin` as `const char *` (§4.3); fix the `PIN_ABSENT` comment.
- [ ] `backend/src/library/library.c`: `PIN_KEYS` + `"ball"`; `load_pinout` style, ball-name, duplicate and absent rules with the §4.1 messages; designator for numbered pins.
- [ ] `backend/src/models/display7.c:expand_side`: store indices.
- [ ] `backend/src/models/oscillator.c`: `enablePin` integer-or-string by designator (§4.3); `osc_create` index by pointer equality.
- [ ] `backend/include/circuit.h`, `backend/src/circuit.c`: `circuit_connect(…, const char *pin, …)` (§4.4).
- [ ] `backend/tests/support/listing.c`: print designators (§4.5).
- [ ] `backend/tests/circuit_check.c`: the 29 calls take string literals (`1` → `"1"`); line 129 keeps `const char *out = osc_part->pins[i].designator`; line 245's "past the end" case uses `"99"`; add `circuit_connect(c, display, NULL, n0) == -1` ("a NULL pin is rejected").
- [ ] `backend/tests/sram_test.c:58`: return `i`. `backend/tests/library_check.c:94`: `strcmp(vcc->pins[0].designator, "1") == 0`; lines 409-410: `segment_pins[0] == 0`, `common_pins[1] == 2`.
- [ ] `backend/tests/library_check.c` new cases (inline JSON, as the existing ones):
  - accepted: an LED device with a numbered `dip2` and a ball `bga` pinout (`{"ball":"A1","name":"K",…},{"ball":"B2","name":"A",…}`). The ball part has `pins[1].designator` `"B2"` and loads with anode/cathode bound.
  - accepted: an oscillator with a ball pinout and `"enablePin":"A2"`. `enable_pin` equals `pins[i].designator` for the `A2` pin.
  - rejected, one `expect_fail` each, matching the message: both keys; neither key; mixed style; `"I1"`; `"A0"`; `"A01"`; `"AAA1"`; duplicate `"A1"`; `absent` in a ball pinout; `"enablePin":"Z9"`; `"enablePin":true`.
- [ ] `backend/tests/circuit_check.c` new case: the accepted LED ball part from an inline library, connected `"A1"`/`"B2"` between GND and VCC, lights up. Also `circuit_connect(c, led, "C3", n) == -1` with `has no pin C3`.

Acceptance: the whole backend suite passes **with `backend/tests/golden/` untouched**, and the new ball cases run.
  Check: `cmake --build /tmp/logic-bga -j8 && ctest --test-dir /tmp/logic-bga && git diff --quiet backend/tests/golden && echo goldens-untouched` → `100% tests passed, 0 tests failed out of 7` and `goldens-untouched`; `/tmp/logic-bga/library_check | grep -ci ball` → ≥ 1 `ok` line per new case (est. 30 s).
Commit: —

### Phase 2 — The app on pin index and designator

The app builds against Phase 1's headers. The file stays version 1 with the
same bytes.

- [ ] `Backend/PartCatalog.swift`: `ChipPin.index`/`designator`, `DisplayInfo` indices, `enablePin: String?` (§4.6).
- [ ] `Canvas/SchematicModel.swift`: `PinSlot.id`, `PinRef.index`, layout sorting and lookups, 7-segment and DIP switch by index, pin hit-testing (§4.6).
- [ ] `Canvas/ChipBlockView.swift:72`, `Canvas/SevenSegmentSymbolView.swift:59`, `Canvas/DipSwitchSymbolView.swift:57`: draw `slot.pin.designator`.
- [ ] `Document/SchematicFile.swift`: v1 `"pin": n` ↔ `index n - 1` (§4.6).
- [ ] `Simulation/Simulation.swift`: connections carry the designator; `circuit_connect` with it.
- [ ] Remove the stale wording "pin number (1-based)" in `Simulation.swift:21` and "Pin numbers for the left…" in `PartCatalog.swift:74`.

Acceptance: the app builds, and no Swift pin-number identity remains.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-bga-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 30 s; a clean build measured 29 s); `grep -rnE 'pin\.number|ref\.number|key\.number|byNumber|let number: Int' schematical/schematical --include='*.swift'` (22 matches on `main`) → no matches (est. seconds).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` "Pinouts and pins": numbered vs ball pinouts. Cover the `"ball"` key, the ball-name rule, ball order = position (list new devices row-major), no `absent` in ball pinouts, a ball example, and kind keys still by name.
- [ ] `docs/parts-format.md` `oscillator`: `enablePin` is the pin's designator, a number for numbered pinouts or a ball name.
- [ ] `docs/parts-format.md` `7seg`: `PartDisplay.segment_pins`/`common_pins` are pin indices.
- [ ] `docs/backend.md`: the example uses `circuit_connect(c, vcc, "1", anode)` etc.; the note "Pin numbers are 1-based" becomes "pins are named by designator (`"1"`, `"A1"`)".

Acceptance: the docs describe the new API and keys, with no stale number wording.
  Check: `grep -nE 'circuit_connect\(c, [a-z]+, [0-9]' docs/backend.md backend/include/circuit.h; grep -n '"ball"' docs/parts-format.md | head -1` → no matches from the first grep; a line from the second (est. seconds).
Commit: —

## Validation Plan

- Tests: `library_check` (ball pinouts accepted; every §4.1/§4.3 rejection),
  `circuit_check` (connect by designator, NULL, unknown ball, a lit LED on a
  ball part), the existing benches unchanged.
- Coverage check: the new loader paths are exercised by the inline cases.
  `/tmp/logic-bga/library_check` prints one `ok` line per new case, so each
  case is a named, counted check, not a silent pass.
- Runtime proof (with the `run` skill): launch the Debug build and open
  `test.schematical` (VCC, GND, `OSC-SIT1534-1508`, `LED-AMBER-0603`). The
  pins show the same numbers as on `main`. Press Play: the LED blinks lit.
  Save As `/tmp/plan02a.schematical`, then
  `diff <(jq -S . test.schematical) <(jq -S . /tmp/plan02a.schematical)` →
  no output. Same pins, same version 1.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`, the `circuit.h` and `parts.h` comments.
- Final gate (run once, after Phase 3):
  `cmake -S backend -B /tmp/logic-bga-final && cmake --build /tmp/logic-bga-final -j8 && ctest --test-dir /tmp/logic-bga-final && git diff --quiet backend/tests/golden`
  (est. 30 s), the Phase 2 `xcodebuild` (est. 30 s), and the runtime proof
  (est. 10 min).

## Decisions

- Grid packages use a new `"ball"` pin key; numbered pinouts keep `"number"` unchanged. A pinout is all-number or all-ball. Decided by the owner, 2026-10-31. This keeps churn out of the 2,750 existing pin lines (`grep -c '"number"'` over gate/dff/sram = 940+284+1526). (§4.1)
- `.schematical` moves to version 2 with string pins, converting version 1 on open. Decided by the owner, 2026-10-31. That is plan-02-B. A keeps version 1 so its gate stays byte-identical. (plan-02-B)
- All 37 BGA devices (12 SRAM TFBGA, 25 TI DSBGA) get real ball names. Decided by the owner, 2026-10-31. That is plan-02-C. (plan-02-C)

## Corrections

## Summary

The risk is the position-versus-number boundary in the app (file load/save,
layout lookup, netlist). The byte-identical goldens prove the library side,
and the runtime proof's save round-trip proves the app side. The engine,
the models' behavior, the part data and the file bytes are untouched.
