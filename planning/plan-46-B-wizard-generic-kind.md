# plan-46-B: Part Wizard — the `generic` kind (symbol and footprint only)

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-A

Most parts a hobbyist wants to draw have no simulation model here: a sensor
module, a relay, a motor driver, a breakout board. Today they must pretend
to be another kind. B adds the **`generic` kind**: any pins, any roles, an
optional supply range, no kind keys, and `"simulated": false` — the
`mcu` kind's precedent (plan-36-F): the circuit refuses its parts and
Simulation › Run is disabled while one is placed, with the existing
"… is not simulated" message. The wizard's first screen offers it as
"Just a symbol and footprint (no simulation)" (plan-46-F).

Behavioral outcome: a `.schpart` of kind `generic` with `"simulated":
false` and any pinout loads, lists as `generic` with `|unsimulated`, draws
as a plain block of its pins, prefixes `U`, and `circuit_add_part` refuses
it; the bundled library and every golden are unchanged.

References:

- `docs/backend.md` § Adding a kind (the five steps and the three test
  lines); § Tests.
- `backend/src/models/header.c` (the passive model shape: no hooks,
  `create` returns a non-NULL pointer), `backend/src/models/mcu.c` (the
  `"simulated": false` rule :67–70, `kind_voltage` / `kind_part_voltage`).
- `backend/src/library/library.c:811–819` (the generic `"simulated"` device
  key), `backend/src/circuit.c:137` (the refusal).
- `backend/include/parts.h:29` (`PartKind` ends with `PART_TRANSISTOR`),
  `backend/src/library/kinds.c` (`KINDS[]`), `backend/CMakeLists.txt`
  (`src/models/*.c` list, `logic_test(…)` lines).
- `backend/tests/support/listing.c:37–39` (kind names), `:153–180` (facts),
  `backend/tests/trace_check.c:22` (`TRACE_LAST_KIND`),
  `backend/tests/listing_check.c:50–51` (arrays sized by the last kind).
- App: `Editor/KindSchema.swift` (`PartKinds.all`, `displayNames`),
  `Editor/BundledPartFile.swift:97–99` (`PartKinds.name(of:)`),
  `Backend/PartFacts.swift:200–213` (kind names),
  `Preferences/ReferencePrefixes.swift:72–93` (prefix; `default` →
  `.integratedCircuit`), `BoardPreview/PartLook.swift` (`default: .block`),
  `Simulation/SimulationAvailability.swift` (reads `simulated`).
- `schematicalTests/PartEditorWindowTests.swift:15–17` (`formless`),
  `:21–34` (`testKindNamesAreThePartFiles`: one part file per kind).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-A complete | `ls planning/completed/plan-46-A-*.md` → exists | NOT MET (2026-11-16: not started) |
| `PartKind` still ends with the kind B appends after | `grep -n 'PART_TRANSISTOR$' backend/include/parts.h` → line 29; `grep -c 'PART_CRYSTAL\|PART_REGULATOR' backend/include/parts.h` → 0 | MET (2026-11-16 at 9e13cbe4: line 29; 0) |

Everything below is written against the world where these hold. plan-44
and plan-42 are written to append their kinds after `PART_GENERIC` (edited
in plan-46's commit), so they cannot be ahead of B.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `PART_GENERIC` is the last `PartKind`; `GENERIC_KIND` is in `KINDS[]`.
- `Parts/generic.json` exists with `"devices": []` (the app loads every
  bundled JSON as a part file, and `testKindNamesAreThePartFiles` wants
  one file per kind).
- `generic_test` passes the § 4 cases; `ctest` runs 37 programs (T0 = 36).
- The app names the kind (`"generic"` in `PartKinds.all`, display name
  "Other part (no simulation)", `PART_GENERIC` → `"generic"`, PartFacts
  "Part"), and it is listed as formless until C gives it a schema.

### Non-goals (explicit constraints)

- **No bundled generic parts.** `generic.json` has no devices; the goldens
  (`parts.txt` / `traces.txt` 32897, `footprints.txt` 1646 lines) stay
  byte-identical.
- **No new symbol.** The app's block drawing (`PartLook` `default:
  .block`, the schematic's chip block) is what a generic part gets.
- **No kind keys** beyond `vcc` / `inputMax`; nothing names a pin.

## 2. Current State

- `PartKind` has 31 kinds, `PART_GATE` … `PART_TRANSISTOR`
  (`awk '/PART_GATE/,/} PartKind;/' backend/include/parts.h | grep -o
  'PART_[A-Z0-9_]*' | wc -l` → 31).
- `mcu` is the one facts-only kind: `mcu_load_device` refuses a device
  without `"simulated": false` (`mcu.c:67`), `mcu_bind_pinout` wants a
  power and a ground pin, `mcu_load_part` ends with `kind_part_voltage`,
  and its engine model is the header shape (`MCU_MODEL`, no hooks).
- `"simulated"` is a common device key read for every kind
  (`library.c:811–819`); `circuit_add_part` refuses a part whose device
  says false (`circuit.c:137`); the listing appends `|unsimulated`
  (`docs/backend.md` § Tests).
- `header.c` is 68 lines: device keys, `bind_pinout` (pin count check),
  `load_part` (sets `out->header`), `create` returning that pointer.
- The app loads every bundled `.json` except footprints/datasheets/boards as
  a part file (`PartCatalog.swift` `load(into:)`); an empty `"devices"`
  array loads (read `read_file`'s loop, `library.c:934–948`: it iterates
  the children, none is required).
- App switches on `PartKind` that need a line: `BundledPartFile.name(of:)`
  (returns nil by default — the copy path then fails), `PartFacts`
  kind name (`default: ""`). `ReferencePrefixes` and `PartLook` have a
  `default` that is right for a generic part.

### Measured populations

| What | Count | Command |
|---|---|---|
| `PartKind` values | 31 | the `awk` above → 31 |
| Test programs T0 | 36 | `grep -c '^logic_test' backend/CMakeLists.txt` → 36 |
| `PART_TRANSISTOR` uses outside its own model and tests | 11 lines in 9 files | `grep -rn PART_TRANSISTOR --include='*.c' --include='*.h' --include='*.swift' backend schematical \| grep -v 'models/transistor.c\|transistor_test\|TransistorTests'` → 11 (parts.h, voltage.c, trace_check.c, listing_check.c ×2, listing.c ×2, ReferencePrefixes, PartCatalog, PartFacts, BundledPartFile); only the last-kind sites (parts.h, trace_check, listing_check ×2) move to `PART_GENERIC` |

### Verified properties

- **A part with no `vcc` is allowed** for a passive kind: header parts have
  none (`header.c` has no part keys and never sets `out->vcc`).
- **`kind_part_voltage` requires a `vcc` from the device or the part**
  (mcu's `missing "vcc"`, `docs/parts-format.md` § mcu), so B calls it only
  when either has one.

## 3. Design

`backend/src/models/generic.c` (the header shape):

- `DEVICE_KEYS = {"vcc", "inputMax", NULL}`, `PART_KEYS = {"vcc",
  "inputMax", NULL}`.
- `generic_load_device`: refuses a device whose `"simulated"` is not
  `false` with `a generic part has no simulation model: "simulated" must
  be false`; reads the optional device voltage (`kind_voltage`, as mcu).
- `generic_bind_pinout`: any count ≥ 1 (`a generic part needs at least one
  pin`), any roles.
- `generic_load_part`: `kind_part_voltage` when the device or the part has
  `vcc`; otherwise none.
- Model: `GENERIC_MODEL` `{"generic", pin_count, create, NULL…}`;
  `create` returns the `Part` itself (non-NULL; there is no info struct).
- `parts.h`: `PART_GENERIC` appended; no info struct, no `Part` member.

Listing: `listing.c` prints `generic` and no facts (the line ends after the
common fields and `|unsimulated`).

Rejected alternatives:

- **Reuse `header` or `connector`** — header needs `rows` and a pin count
  that is a multiple of it, and both draw and file as connectors (owner
  decision, plan-46-A § Decisions).
- **A `"simulated": false` flag on any kind** — every kind's loader would
  have to tolerate missing behavior keys; one kind with none is simpler.

## 4. Test cases (`backend/tests/generic_test.c`)

Each loads a `.schpart` text through `load_part_file` on the bundled
library (the `part_file_test` setup):

a. 3 pins (`VCC` power, `GND` ground, `OUT` output), `"simulated": false`,
   no vcc → loads; `kind == PART_GENERIC`, `simulated == 0`, `vcc == NULL`.
b. Same with device `"vcc": [3, 5.5]` → `vcc->min_mv == 3000`.
c. No `"simulated"` → `a generic part has no simulation model: "simulated"
   must be false`; `"simulated": true` → the same.
d. An empty pinout → `a generic part needs at least one pin`.
e. An unknown device key (`"gates"`) → the loader's unknown-key message.
f. `circuit_add_part` on (a)'s part → -1 with `part "<name>" is not
   simulated` (the message `mcu_test.c:68–70` checks for TESTMCU).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands. **An unticked box means
> NOT DONE.**

### Phase 1 — The backend kind

- [ ] `backend/include/parts.h`: `PART_GENERIC` after `PART_TRANSISTOR`.
- [ ] `backend/src/models/generic.c` per § 3; `backend/src/library/kinds.c`:
      `extern const PartKindOps GENERIC_KIND;` and `&GENERIC_KIND` last in
      `KINDS[]`; `backend/CMakeLists.txt`: `src/models/generic.c` and
      `logic_test(generic_test)`.
- [ ] `backend/tests/support/listing.c`: `case PART_GENERIC: return
      "generic"` and an empty facts case; `tests/trace_check.c`
      `TRACE_LAST_KIND` and `tests/listing_check.c:50–51` → `PART_GENERIC`.
- [ ] `backend/tests/generic_test.c`: § 4 a–f.
- [ ] `schematical/schematical/Parts/generic.json`: `{"format":
      "schematical-parts", "version": 1, "kind": "generic", "devices": []}`
      (the format/version strings copied from `header.json`'s first lines).

Acceptance: the kind loads, refuses what § 4 refuses, and nothing bundled moves.
  Check: `cmake --build /tmp/logic-p46 -j8 && ctest --test-dir /tmp/logic-p46 -j8` → `100% tests passed, 0 tests failed out of 37`; `git diff --stat backend/tests/golden` → empty (est. 4 min; the whole backend suite is 9 s at planning, so it is the scoped check).
Commit: —

### Phase 2 — The app knows the kind

- [ ] `Editor/KindSchema.swift`: `"generic"` in `PartKinds.all`,
      `"generic": "Other part (no simulation)"` in `displayNames`.
- [ ] `Editor/BundledPartFile.swift`: `case PART_GENERIC: "generic"`.
- [ ] `Backend/PartFacts.swift`: `case PART_GENERIC: return "Part"`.
- [ ] `schematicalTests/PartEditorWindowTests.swift`: `formless` gains
      `"generic"` (C removes the set).
- [ ] `schematicalTests/GenericPartTests.swift` (new): load § 4 (a)'s text
      with `PartCatalogStore.register` → the part lists with `simulated ==
      false`, `ReferenceCategory.of(part) == .integratedCircuit`, and
      `SimulationAvailability.unsimulated` of a placed copy returns it.

Acceptance: the editor lists agree with the part files and a generic part registers as unsimulated.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/GenericPartTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` § Kinds: a `### \`generic\`` section after
      `mcu` (keys, the simulated rule, the pin rule, no bundled parts, the
      wizard's "no simulation" choice).
- [ ] `docs/backend.md` § Adding a kind: the facts-only paragraph names
      `generic` beside `mcu`.

Acceptance: both docs name the kind.
  Check: `grep -c 'generic`' docs/parts-format.md` → at least 1; `grep -c 'generic' docs/backend.md` → at least 1 more than before (est. 1 min).
Commit: —

## Validation Plan

- Tests: `generic_test` (6 cases), `GenericPartTests`, `PartEditorWindowTests`.
- Coverage: the goldens do not cover the kind (no bundled parts);
  `generic_test` is its coverage.
- Runtime proof: plan-46-F places a wizard-made generic part and sees
  Run disabled.
- Doc sync: Phase 3.
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (the `generic` kind is the owner's choice).

## Corrections

None yet.

## Summary

Low risk: a passive kind with no bundled parts; the only shared lines are
the last-kind array sizes, which move by one.
