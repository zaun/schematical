# plan-26-A: Pull resistor data, backend, model and file format

Last updated: 2026-11-07
Overall Effort: large (3h–1d)
Effort: medium (1h–2h)
Depends on: nothing

Pull-ups and pull-downs are drawn as resistors but are not real parts today:
they have no footprint, so they are missing from the PCB and the Bill of
Materials. plan-26 makes each placed pull a real resistor with its own
resistance (Ohms) and footprint, defaulting to 1 kΩ and Radial.

This sub-plan does everything except the controls. It adds six resistor
footprints, lets pull parts (only) name a footprint whose second pad is the
rail end, gives each placed pull an `ohms` value, an `R<n>` reference and a
footprint choice, and saves both in the file. plan-26-B adds the sidebar
Options, the Settings default, the value on the symbol and the BOM lines.

Outcome: placing a pull-up gives `R1`, 1 kΩ, footprint `RES-RADIAL-2.54`,
placed on the PCB with two pads. Saving writes `"ohms": 1000,
"footprint": "RES-RADIAL-2.54"` and a `pcb` placement; reopening restores
them. VCC and GND still have no footprint.

References:

- `docs/parts-format.md` § `supply`, § The `"footprint"` part key, § `dual`
- `docs/file-format.md` § Parts, § Validation on open, § Rules for changing the format
- `docs/backend.md` § Tests (golden regeneration)
- `planning/completed/plan-18-A-generic-led-data.md` (the precedent: `footprintChoices`, per-part `color`/`footprint`)
- `planning/completed/plan-03-A-*.md` §4.5 (how generic footprints without a datasheet were sized)
- `backend/src/library/library.c:load_part`, `load_part_footprint`
- `schematical/schematical/Canvas/SchematicModel.swift:PlacedChip`, `placeChip`, `setPartFootprint`
- `schematical/schematical/Document/SchematicFile.swift`

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Backend tests pass | `cmake -S backend -B /tmp/p26-build && cmake --build /tmp/p26-build -j8 && ctest --test-dir /tmp/p26-build -j8` → 23/23 | MET (2026-11-07, re-run by follow-plan: 24/24, 7.72 s — one test added since authoring; see Corrections) |
| The 7 pull parts still have no footprint | `grep -c '"pull": true' schematical/schematical/Parts/supply.json` → 7, and `grep -c 'RES-' schematical/schematical/Parts/footprints.json` → 0 | MET (2026-11-07, re-run: 7, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The 7 pull parts (`PULLUP-5V`, `-3V3`, `-2V5`, `-1V8`, `-1V2`, `-12V`,
  `PULLDOWN`) each have package `Radial`, footprint `RES-RADIAL-2.54` and six
  footprint choices: 0402, 0603, 0805, 1206, Axial, Radial.
- Each placed pull has `ohms` (default 1000) and a footprint choice (default
  `RES-RADIAL-2.54`), is on the PCB, and is named `R<n>`.
- Both are saved in the file and restored on open.

### Non-goals (explicit constraints)

- The simulation is unchanged: a pull still drives weakly; `ohms` has no
  effect on logic levels or the voltage check. `backend/tests/golden/traces.txt`
  must not change.
- The pull pinout stays one pin. The schematic symbol, its size and its pin
  are unchanged in this sub-plan.
- VCC, GND and the other supplies keep "no footprint" (`a supply has no
  footprint` still rejects them).
- File `version` stays 4 (`docs/file-format.md` § Rules: an optional field
  keeps the version).
- The PCB has no nets; pad 2 (the rail end) is unconnected copper the user
  routes by hand, like `LED-DIP4`'s pads 3 and 4.

## 2. Current State

- Pull parts are `supply`-kind devices with `"pull": true`
  (`schematical/schematical/Parts/supply.json`, 7 parts), package `""`, no
  footprint. Placed with the Pull-up / Pull-down tools
  (`Canvas/SchematicCanvasView.swift`, `case .vcc, .ground, .pullUp, .pullDown`
  → `document.placeChip`), not from the drawer.
- The loader rejects any footprint key on a supply
  (`backend/src/library/library.c:load_part`, `"a supply has no footprint"`)
  and requires a part's own `footprint` to have exactly one pad per pin
  (`load_part_footprint`: `footprint->pins != part->pin_count`). Choices may
  have more pads than pins (`load_footprint_choices` uses only `check_pads`).
- `ops->load_part` runs after footprint loading. No kind model reads the
  footprint (`grep -ln footprint backend/src/models/*.c` → no files), so
  running `ops->load_part` first changes nothing a kind sees.
- `supply.c:supply_load_part` sets `part->supply` (with `pull`).
- The app treats footprints generically: `PlacedChip.board` is set when
  `part.footprint != nil`; `PlacedChip.footprint` uses `footprintChoice`;
  `placeChip` places the footprint with `BoardLayout.nextSpot`;
  `SchematicFile` writes `pcb` and `footprint` for any part that has them and
  auto-places a part with a footprint and no `pcb` on open
  (`SchematicFile.swift` `apply`, "placed automatically"). So adding
  `footprintChoices` to the pull parts puts them on the PCB with no PCB code.
- `PlacedChip.boardPads` keeps every pad whose designator is not a hidden
  pin, so pad `2` (no pin) is drawn.
- References: `placeChip` gives every part `U<n>`, from `nextReference` up,
  skipping used ones (`SchematicModel.swift:placeChip`).
- `PlacedChip.color` / `footprintID` (plan-18) are the precedent for
  per-placed-part options: normalized in `init`, set by `setPartColor` /
  `setPartFootprint`, saved as optional `color` / `footprint` keys.

### Measured populations

| What | Count | Command |
|---|---|---|
| Pull parts | 7 | `grep -c '"pull": true' schematical/schematical/Parts/supply.json` → 7 |
| Pull lines in the parts golden | 7 | `grep -c '|supply|.*pull|' backend/tests/golden/parts.txt` → 7 (`PULLDOWN`, `PULLUP-12V` … at lines 4224–4230) |
| Existing resistor footprints | 0 | `grep -c 'RES-' schematical/schematical/Parts/footprints.json` → 0 |
| Footprints today | 257 | `grep -c '"id"' schematical/schematical/Parts/footprints.json` → 257 |
| Loader tests on supply footprints | 3 | `grep -c 'a supply has no footprint' backend/tests/library_check.c` → 3 (lines 358, 1740, 1753) |
| Pulls in `be-6502.schematical` | 0 | `python3 -c "…partNumber.startswith('PULL')…"` → 0 |

### Verified properties

- **Kinds don't read footprints**: `grep -ln footprint backend/src/models/*.c`
  → no output; only `library.c` and `footprints.c` mention it. So moving
  `ops->load_part` earlier in `load_part` is neutral for every kind.
- **The library_check fixture has a 2-pad footprint `T2`**
  (`backend/tests/library_check.c:50`, `FIXTURE_DUAL("T2", "2", "")`), usable
  for pull tests.
- **Generic footprint path in Swift**: read `BoardModel.swift:PlacedChip.footprint`,
  `boardPads`, `SchematicModel.swift:placeChip`, `SchematicFile.swift:354–366, 513–547`;
  none checks the part kind.
- **The test target is a synchronized folder** (`project.pbxproj` has 5
  `FileSystemSynchronized` entries; `schematicalTests` is one), so a new test
  file needs no project edit.

## 3. Design Overview

Four pieces, in order:

1. **Footprints** (data only): six `dual` footprints in `footprints.json`.
2. **Loader**: pull supplies must name a footprint; their own footprint may
   have more pads than pins. VCC/GND unchanged.
3. **Part data**: the 7 pull parts get `package`, `footprint`,
   `footprintChoices`.
4. **App model + file**: `ohms` on `PlacedChip`, `R<n>` references, file key
   `ohms`.

Correctness risk is in piece 2 (loader order change) and is covered by
`library_check` plus the unchanged `traces.txt` golden. This is not a
byte-identity plan: `parts.txt` (7 pull lines) and `footprints.txt` (6 new
lines) are **expected** to change; `traces.txt` must not.

Rejected alternatives:

- *A second, hidden pin on the pull pinout* for the rail pad: changes the
  symbol, the simulation's pin count and `supply_bind_pinout` ("exactly one
  pin"), and `boardPads` drops pads of hidden pins anyway.
- *A new `resistor` kind*: a much larger change for the same result; pulls
  already simulate correctly as weak supplies.
- *Reusing the `LED-0402…1206` footprints*: their descriptions ("chip LED")
  show in the Footprint menu.
- *Storing ohms as number + unit*: one number (ohms) is enough; the unit is a
  display choice (plan-26-B).

## 4. Footprints

Sized like plan-03-A §4.5 (no datasheet; generic). Chip sizes use the same
inch-code formula, so the numbers equal the matching LED footprints. All are
`dual`, `pins` 2, `perSide` 1. Pad 1 (left) is the signal pin; pad 2 is the
rail end. Resistors are not polarized: no `pin1Shape`.

| id | description | body W × H | pad W × H | span | pitch | drill |
|---|---|---|---|---|---|---|
| `RES-0402` | 0402 chip resistor (1.0 x 0.5 mm) | 1.0 × 0.5 | 0.55 × 0.75 rect | 0.9 | 1 | — |
| `RES-0603` | 0603 chip resistor (1.5 x 0.75 mm) | 1.5 × 0.75 | 0.7 × 1.0 rect | 1.2 | 1 | — |
| `RES-0805` | 0805 chip resistor (2.05 x 1.25 mm) | 2.05 × 1.25 | 0.85 × 1.5 rect | 1.6 | 1 | — |
| `RES-1206` | 1206 chip resistor (3.05 x 1.5 mm) | 3.05 × 1.5 | 1.15 × 1.75 rect | 2.3 | 1 | — |
| `RES-AXIAL-10.16` | Axial resistor, 1/4 W, 10.16 mm (0.4 in) lead spacing | 6.3 × 2.5 | 1.6 × 1.6 round | 10.16 | 1 | 0.8 |
| `RES-RADIAL-2.54` | Radial (standing) resistor, 2.54 mm lead spacing | 5.08 × 2.5 | 1.6 × 1.6 round | 2.54 | 1 | 0.8 |

`note` on each: `"generic; plan-26-A §4"`. Axial body is a 1/4 W carbon-film
body (6.3 × 2.5 mm); the radial body is a box over both pads (the standing
body plus its bent lead). Drill 0.8 fits the 0.6 mm lead.

## 5. Loader (`backend/src/library/library.c:load_part`)

- Move the `internal->ops/device/bound` setup and the `ops->load_part(...)`
  call (with its `"invalid part"` error) to just after the pinout lookup,
  before any footprint key is read. `part->supply` is then set for supplies.
- Replace the supply rule with: a supply that is **not** a pull and has
  `footprint`, `connectorFootprint` or `footprintChoices` → `a supply has no
  footprint` (same text, so the 3 existing tests pass unchanged). A pull
  supply with `connectorFootprint` → `a pull part has no connectorFootprint`.
- `load_part_footprint` gets a `int extra_pads` argument. When set, the
  `footprint->pins != part->pin_count` check becomes `footprint->pins <
  part->pin_count` (same message). It is set only for `"footprint"` on a pull
  supply.
- Final check: `missing "footprint"` for any part that is not a non-pull
  supply (`ops->kind != PART_SUPPLY || part->supply->pull`).

## 6. Part data (`supply.json`)

Each of the 7 pull parts becomes:

```json
{"name":"PULLUP-5V","package":"Radial","pinout":"pin","footprint":"RES-RADIAL-2.54",
 "footprintChoices":[
   {"package":"0402","footprint":"RES-0402"},
   {"package":"0603","footprint":"RES-0603"},
   {"package":"0805","footprint":"RES-0805"},
   {"package":"1206","footprint":"RES-1206"},
   {"package":"Axial","footprint":"RES-AXIAL-10.16"},
   {"package":"Radial","footprint":"RES-RADIAL-2.54"}
 ]}
```

## 7. App model (`SchematicModel.swift`, `PartCatalog.swift`)

- `CatalogPart.isPullResistor: Bool` — `details` is `.supply(_, _, pull: true)`.
- `PlacedChip.ohms: Double?` — set exactly for pull resistors. In `init`, a
  nil or invalid value (not finite, ≤ 0, or > `ResistorValue.maxOhms`)
  becomes `ResistorValue.defaultOhms` (1000); other parts get nil. New
  `init` parameter `ohms: Double? = nil`.
- `ResistorValue` (new file `Canvas/ResistorValue.swift`, `nonisolated enum`):
  `defaultOhms = 1000`, `maxOhms = 1_000_000_000`, `isValid(_:)`. (plan-26-B
  adds the formatting and units to this file.)
- `SchematicDocument.setPartOhms(_ id: UUID, _ ohms: Double)` — does nothing
  for a part that isn't a pull resistor or a value that isn't valid.
- `placeChip(…, ohms: Double? = nil)` passes `ohms` through. Pull resistors
  get prefix `R`: the lowest `R<n>` (n ≥ 1) no part uses. Other parts keep
  `U<n>` from `nextReference`; `nextReference` only advances for `U`.

## 8. File format (`SchematicFile.swift`, `docs/file-format.md`)

- New optional part key `ohms` (number): pull resistors only, resistance in
  ohms. Always written for them; missing means 1000; ignored on other parts.
- On open, an `ohms` on a pull resistor that is not finite, ≤ 0 or > 1e9
  fails: new `SchematicFileError.badOhms(reference:, ohms:)`, message
  `"R3 has resistance 0 Ω; it must be more than 0 and at most 1 GΩ."`.
- `footprint` and `pcb` need no code change (generic); their doc rows now
  name the pull resistors, and `pcb` is "left out for VCC and GND" instead of
  "for the supplies".
- Existing pulls in old files open with 1000 Ω, Radial, are auto-placed on
  the PCB, and keep their saved reference (`U<n>`).

## Compatibility / Format Impact

- File: one new optional key (`ohms`); version stays 4. An older build opens
  a new file, ignores `ohms`, and ignores the pulls' `pcb`/`footprint`
  because its catalog has no footprint for them.
- Part files: pull parts gain `package`/`footprint`/`footprintChoices`. An
  older backend would reject them (`a supply has no footprint`); part files
  ship in the app bundle, so the two always match.
- Unchanged: VCC/GND, simulation, schematic symbol, every non-pull part.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Footprints and loader

Adds the footprints and the loader rule. No part uses the footprints yet, so
`footprint_check`'s "every footprint is used" rule would fail; the part data
lands in the same phase.

- [x] `schematical/schematical/Parts/footprints.json`: add the six footprints
      of §4 after `LED-DIP4`'s block. (JSON parses; 263 footprints.)
- [x] `backend/src/library/library.c`: the §5 changes (`load_part` order,
      supply/pull rule, `extra_pads`, final missing-footprint check).
- [x] `schematical/schematical/Parts/supply.json`: the 7 pull parts as §6.
- [x] `backend/tests/library_check.c`: `check_valid_pull` gives `PULLDOWN`
      `'package':'0603','footprint':'T2','footprintChoices':[{'package':'0603','footprint':'T2'}]`
      and checks `pd->footprint` is `T2`. New `expect_fail` cases: a pull
      without `footprint` → `missing "footprint"`; a pull with
      `connectorFootprint` → `a pull part has no connectorFootprint`; a
      non-pull supply with `'footprint':'T1'` → `a supply has no footprint`
      (still rejected — already the existing "supply part with footprint"
      case, which passes unchanged; also added "pull part footprint with
      fewer pads than pins" → `pin "1" has no pad in footprint "BGA2"`).
      `library_check` → `library_check: all passed`, the 3 new cases `ok`.
- [x] Regenerate goldens: `/tmp/p26-build/listing_check --print >
      backend/tests/golden/parts.txt` and `/tmp/p26-build/footprint_check
      --print > backend/tests/golden/footprints.txt`; hand-check the 6 new
      footprint lines against §4. (Checked: bodies and pads match, pad
      centres ±span/2 = 0.45, 0.6, 0.8, 1.15, 5.08, 1.27; drill d0.800 on
      Axial/Radial.)
- [x] `docs/parts-format.md`: § `supply` (pull parts name a footprint and
      footprint choices; their own footprint may have more pads than pins,
      pad 2 is the rail end; table column for footprint), § The `"footprint"`
      part key ("except supplies" → "except VCC/GND supplies"; the pull
      exception), § `dual` intro (add "chip resistors").
- [x] Added: `backend/tests/charlcd_test.c` loads the real `footprints.json`
      before the real `supply.json` (its fixture library had only `FX16`, so
      the pull parts failed `unknown footprint "RES-RADIAL-2.54"`). See
      Corrections.

Acceptance: the library loads with the new pull parts; only the expected
golden lines changed.
  Check: `cmake --build /tmp/p26-build -j8 && ctest --test-dir /tmp/p26-build -j8`
  → 100% passed; `git diff --stat backend/tests/golden` → `parts.txt` 7 lines
  changed, `footprints.txt` 6 lines added, `traces.txt` untouched (est. 1 min).
  Result (2026-11-07): `100% tests passed, 0 tests failed out of 24`;
  `footprints.txt | 6 ++++++`, `parts.txt | 14 +++++++-------` (7 lines
  changed), `traces.txt` not in the diff; `listing_check --print | grep -c
  'RES-RADIAL-2.54'` → 7.
Commit: —

### Phase 2 — App model and file format

- [ ] `schematical/schematical/Backend/PartCatalog.swift`: `CatalogPart.isPullResistor`.
- [ ] New `schematical/schematical/Canvas/ResistorValue.swift`: §7 constants
      and `isValid`.
- [ ] `schematical/schematical/Canvas/SchematicModel.swift`: `PlacedChip.ohms`
      and its `init` normalization; `placeChip(ohms:)` and `R<n>` references;
      `setPartOhms`. Update the `PlacedChip.board` doc comment ("every part
      but VCC and GND" stays true).
- [ ] `schematical/schematical/Document/SchematicFile.swift`: `Part.ohms`
      written from `chip.ohms`; read and validated in `apply` (`badOhms`);
      passed to `PlacedChip.init`.
- [ ] `docs/file-format.md`: § Parts `ohms` row; `footprint` row names pull
      resistors and default `RES-RADIAL-2.54`; `pcb` row "left out for VCC and
      GND"; `reference` row "new pull resistors get `R<n>`"; § Validation on
      open: the `badOhms` case.
- [ ] Tests: new `schematical/schematicalTests/PullResistorTests.swift`:
      placing `PULLUP-5V` gives `R1`, `ohms` 1000, `footprintID`
      `RES-RADIAL-2.54`, non-nil `board`, 2 `boardPads`; a second pull is
      `R2` and a following `SN74HC00D` is `U1`; `setPartOhms` keeps 4700,
      ignores 0, -1, `.infinity`, 2e9 and ignores a non-pull part;
      `setPartFootprint(…, "RES-0805")` switches the footprint; a save/open
      round trip (the `BoardLayersTests` pattern) keeps reference, ohms,
      footprint and PCB centre; a hand-written version-4 file with a
      `PULLDOWN` and no `ohms`/`footprint`/`pcb` opens as 1000 Ω, Radial,
      placed; `"ohms": 0` fails with `badOhms`.

Acceptance: pulls are real resistors in the model and survive save/open.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO`
  → all pass (est. 3 min; `BillOfMaterialsTests` is included because pulls
  now have a footprint and show up there — it still passes in this phase,
  since its tests place no pulls).
Commit: —

## Validation Plan

- Tests: `library_check` (pull rules, VCC/GND still rejected),
  `listing_check`/`footprint_check` goldens, `PullResistorTests`.
- Coverage check: `listing_check --print | grep -c 'RES-RADIAL-2.54'` → 7
  proves the pull parts are in the listing the golden compares.
- Runtime proof: plan-26-B's runtime check covers this sub-plan's model too
  (place a pull-up, see R1 on the PCB).
- Doc sync: `docs/parts-format.md`, `docs/file-format.md` (both in the phases).
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/p26-build -j8` →
  100% passed, and `xcodebuild -project schematical/schematical.xcodeproj
  -scheme schematical test CODE_SIGNING_ALLOWED=NO` → 0 failures (est. 6 min).

## Decisions

- Pull resistors offer 0402, 0603, 0805, 1206, Axial (10.16 mm) and Radial
  (standing, 2.54 mm); default Radial — decided by the owner, 2026-11-07. (§4, §6)
- Default resistance 1 kΩ — decided by the owner, 2026-11-07. (§7)
- New pull resistors are named `R<n>`; existing files keep saved names —
  decided by the owner, 2026-11-07. (§7)
- Ohms is a per-placed-part value saved in the file; it does not affect the
  simulation — author's call (the simulation is digital). (§1 Non-goals)

## Corrections

- **Prerequisites test count**: the backend suite has 24 tests, not 23
  (`ctest --test-dir /tmp/p26-build -j8` → `0 tests failed out of 24`,
  2026-11-07). A test landed between authoring and execution; the row is
  still MET (all pass).
- **`charlcd_test` loads the real `supply.json`** into a fixture library
  that holds only its `FX16` footprint
  (`backend/tests/charlcd_test.c` main), so after §6 it failed with
  `supply.json: devices[7] (PULLUP-5V): parts[0] (PULLUP-5V): unknown
  footprint "RES-RADIAL-2.54"`. §2 missed this reader. Fix: it now loads the
  real `footprints.json` first (`load_footprints` accepts several calls).
  Added as a Phase 1 task.
- **The "non-pull supply with `footprint` T1" case already existed**
  (`library_check.c` "supply part with footprint", one of the 3 counted in
  §2), so it was not added twice; it passes unchanged.

## Summary

The risk is the loader-order change in `load_part`; it is neutral for every
kind (no model reads footprints) and covered by `library_check`. Everything
on the app side reuses plan-18's generic footprint-choice path, so the PCB
needs no code. The simulation, symbol and VCC/GND are untouched.
