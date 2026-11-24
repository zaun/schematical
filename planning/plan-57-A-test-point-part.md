# plan-57-A: Test point part: loader, footprints, part data and the app's recognition

Last updated: 2026-11-23
Overall Effort: large (3h–1d)
Effort: medium
Depends on: nothing (Prerequisites below)

plan-57 adds a **Test Point** part: one generic bare copper pad (or plated
hole) that a person probes on the board. It is listed as the leaf **Test Point**
directly under **Connector** (no sub-category), is one passive pin, and has a
**Size** option and a **Through hole** toggle (default off) in the part's
Options. It is not bought, so it is left out of the Bill of Materials, and it
draws on the schematic as a small circle on a pin stub with a `TP` reference.

This sub-plan (A) lands the part end to end in its first usable form: the
`connector` kind's new `"testPoint"` device key, the ten footprints, the
`GENERIC-TESTPOINT` part and its goldens, the Part Editor's schema, the docs,
and the smallest app change that stops the app drawing the part as a barrel
jack. After A the part is placeable, wires, routes and simulates (as a passive
terminal); it still draws as a plain one-pin box with a `J` reference, appears
in the BOM and offers one flat footprint menu. plan-57-B fixes the schematic
look, reference and BOM; plan-57-C adds the Size menu and the Through hole
toggle.

References:

- `docs/parts-format.md` — § The drawer's categories (line 125), § Footprints
  (§ `explicit`), § The `"footprint"` part key
  (`footprintChoices`, lines 879–895), § `connector` (line 1581), § Datasheets.
- `docs/backend.md` § Tests (goldens: `listing_check --print`,
  `footprint_check --print`, lines 553–585).
- `backend/src/models/connector.c`, `backend/include/parts.h` (`PartConnector`),
  `backend/tests/{library_check,connector_test}.c`, `backend/tests/support/listing.c`.
- The card-slot precedent: commit `c4b0a12c` (plan-54-L, the `connector` kind
  gained `"card"`) and `schematical/schematicalTests/CardSlotTests.swift`
  (plan-54-O, the app side).
- `AGENTS.md` (git rules; datasheet rules do not apply: the part is `Generic`
  and has no datasheet).

## Prerequisites

These are a precondition on the whole feature (A, B and C), not a dependency to
negotiate.

| Must be true | Command | Status |
|---|---|---|
| plan-54-L landed: the `connector` kind loads card slots | `rg -c '"card"' backend/src/models/connector.c` → ≥ 1 | MET (→ 4) |
| No part, key or footprint named test point exists yet | `rg -c -i 'testpoint\|test_point' backend/src backend/include schematical/schematical/Backend schematical/schematical/Canvas` → no match; `rg -c TESTPOINT schematical/schematical/Parts/footprints.json` → no match | MET (both exit 1) |
| Plan number 57 is free | `ls planning planning/completed \| grep plan-57` → only this plan's files | MET |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `GENERIC-TESTPOINT` loads from `Parts/connector.json` as a one-pin passive
  `connector` device with `"testPoint": true`, drawer `["Connector"]`, function
  `"Test Point"`; the app lists it as the leaf **Test Point** under Connector;
  placed, wired and simulated it is a passive terminal that joins nothing.
- Ten footprints exist, `TESTPOINT-PAD-<d>` (surface-mount round pad) and
  `TESTPOINT-THT-<d>` (plated hole with a round pad), for `<d>` = 1.0, 1.5,
  2.0, 2.5, 3.0 mm, all named in the part's `footprintChoices`; the default is
  `TESTPOINT-PAD-1.5`.
- The Board Preview draws no body over a test point (only its copper).
- The app's catalog knows the part (`CatalogPart.isTestPoint`), names its kind
  "Test point", and draws it with the plain block symbol.

### Non-goals (explicit constraints)

- No new part kind. The test point is a mode of the `connector` kind, as the
  card slot is (plan-57 owner decision, § Decisions).
- No change to the schematic file format, the board file format or the backend
  C API beyond one new `PartConnector` field. A placed test point stores its
  footprint choice in the existing per-part `footprintID`
  (`docs/file-format.md` § Parts).
- No manufacturer test-point parts (Keystone, Harwin): only the generic pad.
  No datasheet entry; `"manufacturer": "Generic"` parts have none.
- Existing connector parts (barrel, USB, card slots) load and list exactly as
  before: `git diff backend/tests/golden/parts.txt` shows one added line and no
  changed line.

## 2. Current State

- `backend/src/models/connector.c:14` — `DEVICE_KEYS` is `centerPin, ratedVolts,
  ratedAmps, contacts, usb, card, cardPitch`. `connector_load_device`
  is a chain of modes: `card` (`card_load`), then `usb` (`usb_load`), else a
  barrel jack (requires `centerPin`, `ratedVolts`, `ratedAmps`). Each mode
  rejects the other modes' keys (the `FORBIDDEN` list in `card_load`).
- `connector_bind_pinout` rejects a pinout of fewer than two
  pins (`pin_count < 2`, `connector.c:141`: `a connector has at least two pins`).
- `backend/include/parts.h:159` — `PartConnector` carries `center_diameter_um,
  rated_mv, rated_ma, contacts, usb, card, card_pitch_um`; the engine model is
  passive (`connector.c:CONNECTOR_MODEL`), a part with `contacts == 0` joins
  nothing.
- `backend/tests/support/listing.c:390` prints one `PART_CONNECTOR` branch per
  mode (`card=…`, `usb=…`, barrel `center_um=…`) into `tests/golden/parts.txt`.
- `docs/parts-format.md:125` lists Connector's groups as "Barrel, Header, USB;
  later Terminal block, JST, Card slot, IC Socket". Parts with a drawer of one
  level are shown as function leaves of the category, after its sub-groups
  (`ChipDrawerView.swift:732–735`: `category.subcategories` first, then
  `category.functions`); `["Clocks"]` RTC and `["Processor"]` 6502 are existing
  leaves of this kind (measured below).
- `schematical/schematical/Backend/PartCatalog.swift:555` — `PartDetails`
  `.connector(…)` and `.cardSlot(card:pitchUM:)`; the details are built at
  `PartCatalog.swift:1179–1186`; `symbol(for:)` at `PartCatalog.swift:1566`
  returns `.barrelJack` for any connector that is not USB or a card slot
  (`PartCatalog.swift:1573–1577`), and the barrel layout
  (`SchematicModel.swift:559`) looks its pins up by the names `Centre`,
  `Switch`, `Sleeve`, so a pin named `TP` would get an empty layout.
- `PartLook.kind(of:)` (`BoardPreview/PartLook.swift:102`) falls through to
  `.barrel` (an 11 mm dark box) for any connector that is not USB or a card slot.
- `PartFacts.swift:352` — `kindName` returns "Barrel connector" for the kind
  unless a USB / card-slot branch names it.
- The Part Editor's form for the kind (`Editor/ConnectorDraft.swift`) is
  generated by `scripts/part-editor-schema.py` from `connector.c`'s
  `DEVICE_KEYS` and the shipped values in `Parts/connector.json`.
  `PartKindKeysTests.testFormKeysCoverTheBackendKeys` fails when a backend key has
  no form field.

### Measured populations

| What | Count | Command |
|---|---|---|
| Existing devices with a one-level drawer (precedent for `["Connector"]`) | 14 devices in 3 categories (Clocks › RTC 9, Processor 4, Microcontroller 1) | python over `Parts/*.json` devices: `len(dev['drawer']) == 1` → 14 rows printed |
| `PART_CONNECTOR` sites in the Swift app | 5 | `rg -n 'PART_CONNECTOR' schematical/schematical` → `ReferencePrefixes.swift:87`, `PartCatalog.swift:1573`, `PartFacts.swift:307`, `PartLook.swift:102`, `BundledPartFile.swift:109` |
| Exhaustive `switch` sites over `PartLook.Kind` | 2 | `rg -n 'case \.barrel' schematical/schematical/BoardPreview/PartLook.swift` → `:231` (height), `:594` (solids) |
| Sub-modes the `connector` kind already has | 3 (barrel, USB, card slot) | `connector_load_device` branches |
| Test files are picked up with no project-file edit | yes | `rg -c PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → 5, and `rg -c CardSlotTests …project.pbxproj` → 0 |

### Verified properties

| Claim | How verified |
|---|---|
| The Part Editor schema script reports an unshipped device key as `missing` and emits no field for it, so the loader key and a shipped part carrying it must land in one commit | Probe in the scratchpad: a copy of `scripts/part-editor-schema.py`, `connector.c` with `"testPoint"` appended to `DEVICE_KEYS` and `Parts/connector.json`, run as `python3 scripts/part-editor-schema.py . connector` → `connector: 481 devices, 951 parts, variant None, 8 source keys, missing ['testPoint'] -> …/ConnectorDraft.swift`, and `ConnectorDraft.swift` has no `testPoint` field |
| `footprint_check` fails on a footprint no part uses, so the ten footprints must land with the part | `docs/parts-format.md:877–880` (read) |
| An `explicit` footprint may hold one round pad, with or without `drill` | `docs/parts-format.md` § `explicit` (read): pads are `{pin,x,y,width,height,shape}` plus optional `drill`; `drill` must be smaller than both pad sides |
| `footprintChoices` entries need distinct `footprint` ids and the part's own `footprint` and `package` must be one of them | `docs/parts-format.md:883–895` (read) |
| Parts the Part Editor shows and the automation API (`set_part`, `get_part`) name a choice by its `package` string, so each choice needs its own `package` | `AutomationSchematic.swift:243–247`, `AutomationRead.swift:373–374` (read): `"\($0.package)=\($0.footprint.id)"` |
| Test-only Swift files need no project edit | the two commands in the table above |

## 3. Design Overview

Four pieces, landed in two phases:

1. **Loader and tests** (C): `"testPoint": true` on a `connector` device; one
   pin; none of the other modes' keys; `PartConnector.test_point`.
2. **Data**: ten `explicit` footprints, one device `testpoint` in
   `connector.json`, one part `GENERIC-TESTPOINT`; goldens; the Part Editor
   schema regenerated; docs.
3. **App slice** (Swift, minimal): `.testPoint` details case, `isTestPoint`,
   kind name, `symbol(for:)` → `.block`, `PartLook` `.testPad` (draws nothing).
4. Everything user-visible beyond that is B and C.

The **loader key, the shipped part and the regenerated form must be in the
same commit** (the Part Editor drift test is red with a key and no shipped
value: Verified properties). So Phase 1 is the whole backend-and-data slice, and
Phase 2 is the app slice that stops the new part drawing as a barrel jack. They
land in the same sitting, Phase 1 first; the app between them is the only
interval in which the part is mis-drawn, and only on main, not in a release.

**Correctness risk** is small and sits in the loader's mode chain: the new
branch must come first (before `card`), reject every other mode's key with the
same message shape, and not change what any existing device does (the
`parts.txt` golden for every existing connector line must not move).

**Byte-identity is not the gate.** The gate is behavior: the loader tests, the
golden diff (one new `parts.txt` line, ten new `footprints.txt` lines, one new
`traces.txt` line, nothing else), and the app test. The expected diffs: `parts.txt` +1 line,
`footprints.txt` +10 lines, `traces.txt` +1 line (Corrections 1).

**Probed before writing:** the schema-script probe above (a key with no shipped
value is `missing`).

**Rejected alternatives:** a new `testpoint` kind (about 25 files for `header`,
all boilerplate for a passive one-pin part, nothing it would do differently
today); the `header` kind (generated file, Rows / Gender options); the `generic`
kind (not simulated, so a design holding one cannot run).

## 4. Detailed Design

### 4.1 The device key

`"testPoint": true` on a `connector` device (never on a part). `false` or any
non-boolean is `"testPoint" must be true`. A test-point device has no
`centerPin`, `ratedVolts`, `ratedAmps`, `contacts`, `usb`, `card` or
`cardPitch` (each rejected with `a test point has no "<key>"`), and its pinout
has exactly one pin (`a test point has exactly one pin`); any other connector
keeps `a connector has at least two pins`. `PartConnector` gains
`unsigned test_point` (1 for a test point); everything else in it is 0 / NULL.

### 4.2 The footprints

`explicit`, one pad `{"pin":1,"x":0,"y":0}`, `body` = the pad's diameter
(`"round": true`), origin the pad centre. SMD pad: `shape "round"`, no drill.
Through-hole: the same pad with `drill`, and pad diameter `d`.

| `d` (mm) | `TESTPOINT-PAD-<d>` | `TESTPOINT-THT-<d>` drill (mm) |
|---|---|---|
| 1.0 | pad 1.0 | 0.5 |
| 1.5 | pad 1.5 | 0.8 |
| 2.0 | pad 2.0 | 1.0 |
| 2.5 | pad 2.5 | 1.3 |
| 3.0 | pad 3.0 | 1.5 |

Descriptions: `Test point, 1.5 mm round pad` and `Test point, 1.5 mm plated
hole`; `note`: `generic; plan-57-A § 4.2` (no drawing: it is a generic land
pattern). Ids are `A-Z 0-9 . -` (legal: `TESTPOINT-PAD-1.5`).

### 4.3 The part

```json
{"id": "testpoint", "function": "Test Point", "description": "Test point: a bare copper pad or plated hole to probe",
 "drawer": ["Connector"], "testPoint": true,
 "pinouts": {"p1": [{"number": 1, "name": "TP", "role": "inout"}]},
 "parts": [{"name": "GENERIC-TESTPOINT", "manufacturer": "Generic", "package": "Pad 1.5 mm",
            "pinout": "p1", "footprint": "TESTPOINT-PAD-1.5",
            "footprintChoices": [
              {"package": "Pad 1.0 mm", "footprint": "TESTPOINT-PAD-1.0"}, …,
              {"package": "Through hole 1.0 mm", "footprint": "TESTPOINT-THT-1.0"}, …]}]}
```

Choices are listed pad 1.0 … 3.0, then through hole 1.0 … 3.0 (ten). Package
strings are `Pad <d> mm` and `Through hole <d> mm` (`<d>` with one decimal),
each distinct; the part's own `package` is the default's.

### 4.4 The app slice

`PartDetails.testPoint` (no payload); `CatalogPart.isTestPoint` beside
`isCardSlot`; details built from `connector.test_point != 0` before the card
branch; `symbol(for:)` returns `.block` for it (plan-57-B replaces that);
`PartFacts.kindName` returns "Test point"; `PartLook.Kind.testPad` with height 0
and no solids (the board view draws pads itself).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partial (say what remains);
> mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Loader, footprints, part data, goldens, docs

One commit: the loader key, the shipped part carrying it, the regenerated
Part Editor form (they cannot be split: Verified properties).

- [x] `backend/include/parts.h:159`: add `unsigned test_point;` as the last
      `PartConnector` field; extend the comment above it (`test_point: 1 for a
      test point, a single passive pin; 0 for every other connector`).
- [x] `backend/src/models/connector.c`: add `"testPoint"` to `DEVICE_KEYS`
      (line 14); add `test_point_load` that requires `cJSON_IsTrue`, rejects
      each of `centerPin ratedVolts ratedAmps contacts usb card cardPitch`
      with `a test point has no "%s"`, sets `info.test_point = 1`; call it
      first in `connector_load_device` (before the `card` branch) when the key
      is present; in `connector_bind_pinout` keep `pin_count < 2` for every
      device without `info.test_point` and require `pin_count == 1`
      (`a test point has exactly one pin`) for one with it.
- [x] `backend/tests/support/listing.c:391`: print `testpoint=1` first in the
      `PART_CONNECTOR` case when `part->connector->test_point` is set.
- [x] `backend/tests/library_check.c`: add `check_test_point_connector`
      (beside `check_card_slot_connector`, line 1861; call it next to line
      3444): a valid device loads and its part has `connector->test_point == 1`
      and `pin_count == 1`; `expect_fail` for `testPoint:false`,
      `testPoint:'yes'`, and `testPoint:true` with each of `usb`, `centerPin`,
      `ratedVolts`, `ratedAmps`, `contacts`, `card`, `cardPitch` (messages
      above), a two-pin pinout (`a test point has exactly one pin`), and a
      one-pin pinout without the key (`a connector has at least two pins`).
- [x] `schematical/schematical/Parts/footprints.json`: add the ten footprints
      of § 4.2 as one block of ten lines directly after the last `PINHDR-` line
      (`rg -n 'PINHDR-' schematical/schematical/Parts/footprints.json` shows it).
- [x] `schematical/schematical/Parts/connector.json`: append device `testpoint`
      (§ 4.3), the same layout (one device per block) as the file's other devices.
- [x] `backend/tests/connector_test.c`: add `test_point_joins_nothing` after
      the card-slot checks, with the real library: `GENERIC-TESTPOINT` has one
      pin and ten `footprintChoices`' worth of `footprint` ids that exist
      (`part_library_find`), and a circuit with `5V` → net → the test point's
      pin `1` starts, advances one step and `circuit_shorted_net` is false;
      a second test point on the same net leaves it unshorted too.
- [x] Goldens: `cmake --build /tmp/logic-build` then
      `/tmp/logic-build/listing_check --print > backend/tests/golden/parts.txt`
      and `/tmp/logic-build/footprint_check --print >
      backend/tests/golden/footprints.txt`; `git diff --stat backend/tests/golden`
      shows `parts.txt` +1 line, `footprints.txt` +10 lines, nothing else; hand-read
      the ten lines against § 4.2. **Done** (build dir `/tmp/logic-build-p57`,
      see Corrections): `parts.txt | 1 +`, `footprints.txt | 10 ++++++++++`; the ten
      lines read against § 4.2 (pads 1.0–3.0, drills 0.5/0.8/1.0/1.3/1.5) match.
      `traces.txt | 1 +` as well (Corrections 1).
- [x] `python3 scripts/part-editor-schema.py . connector` regenerates
      `schematical/schematical/Editor/ConnectorDraft.swift` with a `testPoint`
      bool device field (`git diff` shows only that field added). **Done by hand**:
      the script did emit `testPoint` (`missing []`, 482 devices) but also
      downgraded the file's hand-tuned `card` field from `.suggested([...])` to
      `.text`, so its output was discarded and the `testPoint` bool field added
      to the committed file by hand (Corrections 2); `git diff` shows the new
      `f7` field and `usb` renumbered `f8`.
- [x] `docs/parts-format.md`: § connector gets a **Test points** subsection
      (key, rules, messages, pinout, the ten footprints, the part, the drawer
      leaf); the category table row (line 125) says "Barrel, Header, USB,
      Card slot; Test Point (a leaf of the category itself)"; line 855's list of
      parts that keep `"footprintChoices"` gains "test points".

Acceptance: the backend loads the part and rejects every malformed device.
  Observed: `ctest --test-dir /tmp/logic-build-p57 -R 'library_check|listing_check|connector_test|footprint_check|trace_check'` → `100% tests passed, 0 tests failed out of 5` (10 s after the build).
  Check: `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|connector_test|footprint_check' --output-on-failure` → `100% tests passed, 0 tests failed out of 4` (est. 5 min first build, 1 min after; scoped to the four tests that read the changed files, not the suite).
  Check: `git diff --stat backend/tests/golden` → `parts.txt | 1 +`, `footprints.txt | 10 ++++++++++`, `traces.txt | 1 +`, no other file (est. seconds).
Commit: 9fcf8fe3

### Phase 2 — The app recognises the part

The part is placeable and drawn as a plain block, not as a barrel jack.

- [x] `schematical/schematical/Backend/PartCatalog.swift`: add `case testPoint`
      to `PartDetails` after `.cardSlot` (line 555, with a doc comment); add
      `var isTestPoint: Bool` beside `isCardSlot` (line 703); in the details
      builder (line 1179) add `else if let connector = part.connector?.pointee,
      connector.test_point != 0 { details = .testPoint }` before the card branch;
      add the case to the labels switch at line 940 (`return []`); `symbol(for:)`
      (line 1573): `if part.isTestPoint { return .block }` first in the
      `PART_CONNECTOR` case.
- [x] `schematical/schematical/Backend/PartFacts.swift`: the facts switch at
      line 146 gets `case .testPoint: break`; `kindName` (line 352) returns
      "Test point" when `part.isTestPoint`.
- [x] `schematical/schematical/BoardPreview/PartLook.swift`: add `case testPad`
      to `Kind` (line 37 region); `kind(of:)` (line 102): `if part.isTestPoint
      { return .testPad }` first in the `PART_CONNECTOR` case; height (line 231)
      `.testPad: 0`; solids (line 594) `case .testPad: break` with the comment
      "a test point has no body; the pad is drawn by the board's copper".
- [x] `schematical/schematicalTests/TestPointTests.swift` (new): the real
      catalog's `GENERIC-TESTPOINT` has `kind == PART_CONNECTOR`, `isTestPoint`,
      one pin named `TP`, `drawer == ["Connector"]`, `function == "Test Point"`,
      ten `footprintChoices` all distinct, `footprint?.id == "TESTPOINT-PAD-1.5"`,
      `symbol(for:) == .block`, `PartFacts.kindName` "Test point"; a placed chip's
      `PartLook.of(chip).kind` is `.testPad`; the drawer's Connector category lists
      a function leaf `Test Point` and its sub-groups are unchanged (the same
      builder `DrawerCategoriesTests` uses); existing connectors keep their
      details (`PJ-002A` is `.connector`, a card slot is `.cardSlot`).

Acceptance: the app lists, places and draws the part without barrel artifacts.
  Observed (`xcodebuild … -derivedDataPath /tmp/plan57/dd test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/CardSlotTests`, 12 tests): `TestPointTests` passed (5/5), `CardSlotTests` passed (6/6), `PartKindKeysTests` failed on `testFormKeysCoverTheBackendKeys` with keys missing for `converter` and `rtc` only (plan-56's display-driver keys: `backplanes`, `blink`, `scanLimit` …); no `connector` key is missing, so the form covers `testPoint`. That failure is not caused by this plan (Corrections 5).
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/CardSlotTests` → `** TEST SUCCEEDED **` (est. 8 min incl. build; scoped to the new test, the form-key drift test and the card-slot test that shares the code).
Commit: d0c6d820

## Validation Plan

- Tests: backend `library_check` (loader negatives and positive), `connector_test`
  (the real part joins nothing), `listing_check` and `footprint_check` goldens;
  app `TestPointTests`, `PartKindKeysTests` (form covers the new key).
- Coverage check: `connector.c`'s new branch is exercised by `library_check`'s
  `check_test_point_connector` (positive) and `connector_test`; the Swift cases
  by `TestPointTests`. Both files are in their suites' sources
  (`logic_test(library_check)`, `logic_test(connector_test)` in
  `backend/CMakeLists.txt:149–157`).
- Runtime proof: plan-57-C's app check (placing and probing the part).
- Doc sync: `docs/parts-format.md` (§ connector, drawer table row, line 855).
- Final gate (run once, after Phase 2): the two Check lines above plus
  `ctest --test-dir /tmp/logic-build` (the whole backend suite, est. 10 min:
  the loader change touches every connector, so the full suite is the cheapest
  check that fails if an existing connector regressed).

## Decisions

- The test point is a mode of the `connector` kind, not a new kind — decided by
  the owner, 2026-11-23. Settles § 4.1.
- One generic bare pad, no manufacturer parts — decided by the owner, 2026-11-23.
- Listed as the leaf "Test Point" directly under Connector, no sub-category —
  decided by the owner, 2026-11-23. Settles the drawer `["Connector"]`.
- Sizes 1.0 / 1.5 / 2.0 / 2.5 / 3.0 mm, default 1.5 mm, drills 0.5 / 0.8 / 1.0 /
  1.3 / 1.5 mm — decided by the author (internal; the sizes were shown to the
  owner and not changed). Rejected: 4 mm (not a test point), a free-form size
  field (the footprint library is a closed list).

## Corrections

1. **`traces.txt` also gains a line.** § 3 and Phase 1's golden task say
   `traces.txt` is unchanged. It is not: it holds one line per part, in
   `parts.txt` order, and `listing_check` fails with `traces.txt does not line up
   with parts.txt at line 13886` until the new part has one. Evidence:
   `/tmp/logic-build-p57/trace_check --print > backend/tests/golden/traces.txt`
   → `traces.txt | 1 +` (`GENERIC-TESTPOINT	f412f47acbc3761f`); `docs/backend.md`
   § Tests already lists `trace_check --print` among the regenerations. The
   expected diff is `parts.txt` +1, `footprints.txt` +10, `traces.txt` +1.
2. **The Part Editor schema script cannot be used as is.** Its output replaced
   `ConnectorDraft.swift`'s hand-tuned `card` field (`.suggested([...])`, 18
   entries) with `.text`; the file says "Edit by hand freely". The new
   `testPoint` bool field was added by hand and the script's output discarded.
3. **Build directory.** `/tmp/logic-build` already existed (another session's);
   this plan used `/tmp/logic-build-p57` so the two do not share a CMake cache.
   Every `Check` command below reads the same with that path.
5. **`PartKindKeysTests.testFormKeysCoverTheBackendKeys` is red for other
   reasons.** Phase 2's check includes it to prove the Part Editor form covers
   `testPoint`. It fails, but its message lists keys of `converter` and `rtc`
   only (`rg -c '"connector"' /tmp/plan57/xcb-a2.log` → 0 in the failure text):
   plan-56's display-driver keys (`backplanes`, `blink`, `scanLimit`, …,
   `backend/src/models/converter_load.c`, last changed by `9a2c0969`, plan-56)
   that `ConverterDraft.swift` / `RtcDraft.swift` do not yet have. The
   connector half of the test passes, which is what this plan needs; the
   converter / rtc form keys belong to plan-56 and are left untouched.
4. **Where the docs went.** The card-slot paragraph of `docs/parts-format.md`
   sits (misplaced) under § `grid`, not § `connector`; the Test points
   subsection went at the end of § `connector` (before `### header`) where the
   kind's other modes are described.

## Summary

The risk is the connector loader's mode chain; every existing connector line in
the goldens must stay byte-for-byte. Untouched: the engine model, the file
formats, every other kind.
