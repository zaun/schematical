# plan-18-A: Generic LED data, backend and file format

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d)
Effort: large (3h–1d)
Depends on: nothing

The library has 55 LED parts today: 5 colors × 11 packages, such as
`LED-RED-0603`. plan-18 replaces them with one part, `GENERIC-LED`. Each placed
LED keeps its own color and footprint, and both are saved in the file.

This sub-plan does everything except the new controls. It adds two loader
features: a part that offers several footprints (`footprintChoices`), and an
LED with a list of colors (`colors`). It rewrites `led.json` to the one part
and moves the backend tests to it. The app copies the color and footprint
choice onto each placed part, and file format 4 saves them. Older files that
name one of the 55 old parts open as `GENERIC-LED` with that part's color and
footprint. plan-18-B adds the Color and Footprint pickers to the Schematic
sidebar, and the new-LED defaults to Settings.

Outcome: opening `be-6502.schematical` gives U22 as `GENERIC-LED`, Green,
footprint `LED-RADIAL-5MM`, with the same PCB position. Saving writes
`"partNumber": "GENERIC-LED", "color": "Green", "footprint": "LED-RADIAL-5MM"`
and version 4. The drawer shows one LED row.

References:

- `docs/parts-format.md` § Parts, § The `"footprint"` part key, § `led`
- `docs/file-format.md` § Parts (and the version history at the top)
- `docs/backend.md` § Tests (golden regeneration)
- `backend/src/library/library.c:load_part`, `load_part_footprint`
- `backend/src/models/led.c` (device keys)
- `backend/include/parts.h:Part`, `PartLed`
- `schematical/schematical/Backend/PartCatalog.swift`, `PartFacts.swift`
- `schematical/schematical/Canvas/SchematicModel.swift:PlacedChip`, `placeChip`
- `schematical/schematical/Canvas/BoardModel.swift:PlacedChip.footprint`
- `schematical/schematical/Document/SchematicFile.swift` (`renamedParts`, `apply`)

## Prerequisites

None. Every file this plan touches is on `main` today, and no other plan has
to land first. plan-15 (multi-select) and plan-16 (board layers and traces)
edit some of the same Swift files (`SchematicModel.swift`, the sidebar). Code
from either one that lands first gets merged in; neither is a dependency.

| Must be true | Command | Status |
|---|---|---|
| The 55 old LED parts are still the only `led` parts | `grep -c '"name":"LED-' schematical/schematical/Parts/led.json` → 55 | MET (2026-11-07, re-run in worktree: 55) |
| Backend tests pass before starting | `cmake -S backend -B /tmp/p18-build && cmake --build /tmp/p18-build -j8 && ctest --test-dir /tmp/p18-build -j8` → 21/21 | MET (2026-11-07, re-run in worktree with a fresh `/tmp/p18-build`: 21/21, 5.9 s) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The library has one LED part, `GENERIC-LED`, in place of the 55 old ones.
  It has colors Red, Green, Blue, Amber and White, and 11 footprint choices.
  Its default is Red with `LED-RADIAL-5MM` (package Radial).
- Each placed LED in the app has its own color and footprint, saved in the
  file (format version 4) and used by the LED symbol's glow and by the PCB.
- Every file that names an old LED part (`LED-<COLOR>-<PACKAGE>`) opens with
  `GENERIC-LED` and that part's color and footprint, at the same place on the
  canvas and the PCB.

### Non-goals (explicit constraints)

- No new controls. The sidebar pickers and the Settings defaults are plan-18-B.
  Until B lands, a newly placed LED is always Red / Radial, and an LED's color
  and footprint can be changed only by editing the file.
- Simulation does not change. An LED lights while its anode reads 1 and its
  cathode reads 0, as before. Color and footprint have no effect on it.
- Wire ends do not change. A wire on an old LED's pin `1` (K) or `2` (A) stays
  on that pin. Both old pinouts put K at pin 1 and A at pin 2.
- No footprint changes: `footprints.json` and `backend/tests/golden/footprints.txt`
  stay byte-identical.
- 7-segment displays and the character-LCD backlight `color` key are untouched.
- Files of version 1–3 still open. A version 4 file in an older build gets the
  existing "saved by a newer version" message.
- The repo's sample files `be-6502.schematical` and `test.schematical` are not
  re-saved. They open through the migration.

## 2. Current State

- **Parts data.** `schematical/schematical/Parts/led.json` has 5 devices
  (`led-red` … `led-white`). Each has two pinouts: `two-pin` (1 K, 2 A) and
  `dip4` (1 K, 2 A, 3 NC, 4 NC). Each has a `color` string and 11 parts,
  0402, 0603, 0805, 1204, 1205, 1206, 1209, 1411, 1414, DIP and Radial,
  with footprints `LED-0402` … `LED-1414`, `LED-DIP4` and `LED-RADIAL-5MM`.
  Only DIP uses `dip4`. Drawer path `["Indicators","LED"]`, function `LED`.
- **LED kind.** `backend/src/models/led.c` takes device keys `color`, `anode`
  and `cathode` (`DEVICE_KEYS`). `PartLed` is `{ const char *color; }`
  (`backend/include/parts.h:50`). Color is used only by the app (drawing and
  drawer filter) and by `tests/support/listing.c:69` (`color=%s`).
- **Footprints on a part.** `library.c:load_part` reads `footprint` and the
  optional `connectorFootprint` through `load_part_footprint`. That function
  requires the footprint's pad count to equal the pin count, and a pad for
  every pin designator. `PART_KEYS` (`library.c:189`) lists the allowed part
  keys. `footprint_check.c:149` counts a footprint as used when some part
  names it as `footprint` or `connector_footprint`.
- **Precedent for a second footprint:** `connectorFootprint` (character LCDs).
  `PlacedChip.footprint` (`BoardModel.swift:95`) picks it from the placement's
  style. `SchematicDocument.setFootprintStyle` (`SchematicModel.swift:1172`)
  swaps it and keeps pad 1 where it was. plan-18-B copies that for footprint
  changes.
- **Precedent for a renamed part:** `SchematicFile.renamedParts = ["VCC": "5V"]`
  (`SchematicFile.swift:20`). A file naming `VCC` opens as `5V` and is
  saved as `5V`.
- **App catalog.** `CatalogPart.Details.led(color:)` (`PartCatalog.swift:436`).
  `CatalogPart.color` (`:296`) feeds `DiodeSymbolView.lightColor`
  (`DiodeSymbolView.swift:20`), the drawer's color menu (`colors(in:)`) and the
  color filter (`groups(matching:)`, `:764`). `detailLabels` returns `[color]`
  for an LED (`:335`). `PartFacts.rows(for: CatalogPart)` lists Package,
  Footprint and Color from the part (`PartFacts.swift:14-43`).
- **Placed part.** `PlacedChip.part` is a `let` `CatalogPart`
  (`SchematicModel.swift:498`). `placeChip` (`:987`) places a part's
  footprint with `BoardLayout.nextSpot(for: part.footprint, …)`.
  `SchematicFile.apply` (`SchematicFile.swift:398-457`) snaps a stored
  placement with `catalogPart.footprint`, and places footprints that have no
  stored position with `chips[index].part.footprint`.
- **PCB pads.** `PlacedChip.boardPads` (`BoardModel.swift:102`) draws every
  footprint pad except those of hidden (`absent`) pins. A pad with no pin of
  its designator is still drawn. So `LED-DIP4`'s pads 3 and 4 still show
  when the part has only pins 1 and 2.
- **Files.** `SchematicFile.currentVersion = 3`. Parts are stored by
  `partNumber`. Unknown part numbers fail the open with `unknownParts`.
- **Harness.** `scripts/harness/build.sh` builds the app's sources into a CLI
  inside a copy of the Debug `.app`. It runs `xcodebuild` only when
  `<out dir>/dd` is missing, so a changed part file needs a fresh out dir.
  A fresh build plus `open-file.swift` on `be-6502.schematical` took 62 s
  (measured 2026-11-07).

### Measured populations

| What | Count | Command |
|---|---|---|
| LED parts / devices | 55 / 5 | `grep -c '"name":"LED-' …/Parts/led.json` → 55; `grep -c '"id": "led-' …` → 5 |
| `parts.txt` lines for them | 55 | `grep -c '^LED-' backend/tests/golden/parts.txt` → 55 (3 more `LED` hits are 7-seg/LCD text) |
| `traces.txt` lines for them | 55 | `grep -c '^LED-' backend/tests/golden/traces.txt` → 55 |
| LED footprints in `footprints.txt` | 11 | `grep -cE '^LED-[0-9A-Z-]+\|' backend/tests/golden/footprints.txt` → 11 |
| Old LED names in backend tests | 9 lines | `grep -cE 'LED-(RED\|GREEN\|BLUE\|AMBER\|WHITE)-'`: `circuit_check.c` 5 (lines 56, 74, 450, 494, 693), `voltage_test.c` 2 (358, 371), `sim_bench.c` 2 (7 comment, 151) |
| `library_check.c` LED fixtures with `'color'` | 6 | `grep -c "'color':'Red','anode'" backend/tests/library_check.c` → 6 (lines 268, 353, 853, 1406, 1423, 1486) |
| Old LED names in harness scripts | 6 lines | `bus-landing.swift` 2, `bus-erase.swift` 1, `bus-slide.swift` 1, `support/BusProof.swift` 2 |
| Old LED names in docs/headers | 4 | `circuit.h:10`, `docs/backend.md:48`, `docs/file-format.md:89, 351` |
| Sample files naming an old LED | 2 | `be-6502.schematical` (U22 `LED-GREEN-RADIAL`, v3), `test.schematical` (`LED-AMBER-0603`, v1) |

### Verified properties

- **Trace hashes are per part name.** All 55 old trace hashes are different
  (`grep '^LED-' traces.txt | awk '{print $2}' | sort -u | wc -l` → 55), so
  `GENERIC-LED`'s hash can't be predicted. The trace golden changes as
  55 lines removed and 1 added. That diff is expected.
- **Pin positions match across the old pinouts.** `two-pin` and `dip4` both
  have K at pin 1 and A at pin 2 (read from `led.json`). `ChipLayout.diode`
  draws only A and K (`SchematicModel.swift:386`), so no wire can end on a
  `dip4` NC pin. Every old wire end resolves on the two-pin `GENERIC-LED`.
- **A string-array helper exists.** `kind_names(ctx, obj, key, &out, &count)`
  (`kind_util.c:218`) reads an array of strings.
- **New Swift files need no project edit.** The Xcode project uses
  synchronized folders
  (`grep -c PBXFileSystemSynchronizedRootGroup …/project.pbxproj` → 4).

## 3. Design Overview

Three layers, landed in order: loader, then data and tests, then app. Phase 1
adds the two loader features with their own tests. The real `led.json` keeps
using `color` until Phase 2. Phase 2 moves `led.json` to the new format and
then removes `color`. Phase 3 changes the app to match.

Phases 2 and 3 land together. Phase 2 changes `PartLed`, and the app reads
`led.color` (`PartCatalog.swift:436`), so the app stops compiling until
Phase 3 is in. Phases 2 and 3 are one commit series and one sitting.

**Correctness risk** is in Phase 3's file migration and footprint
selection. If the migration table is wrong, old files open with the wrong
color or footprint. If one footprint call site is missed, the PCB uses the
default (Radial) footprint for a part that should have another. Hence the
Phase 3 check covers both sample files and an `LED-0805` placement.

**Gate class:** behavior changes on purpose. `parts.txt` and `traces.txt` are
expected to diff, by exactly −55 / +1 lines each. `footprints.txt` is
expected to stay byte-identical. Any other golden diff is a bug to root-cause
(`listing_check --verbose`, `trace_check --verbose <part>`). It never means
the design is wrong.

Rejected alternatives:

- **Keep 55 parts and only collapse them in the UI.** The owner chose a single
  `GENERIC-LED` part number (Decisions).
- **One pinout per footprint choice** (`dip4` for the DIP choice). A part
  would then have different pins depending on a placed instance's choice.
  The circuit API (`circuit_add_part(c, name, …)`) and `PlacedChip`'s layout
  are built from the part's single pinout. Instead, a choice footprint may
  have more pads than the part has pins. The extra pads are unconnected,
  which is what `dip4`'s NC pins were.
- **An LED-only kind key for footprint choices.** Footprint loading and
  `footprint_check` belong to the generic loader, like `connectorFootprint`.
  So `footprintChoices` is a generic part key. Only `GENERIC-LED` uses it
  today.
- **Keeping `color` next to `colors`.** Only one LED device remains, so the
  single-color key is removed rather than kept as a second way to say the same
  thing.

## 4. Loader: `footprintChoices` (generic part key)

Part key, optional:

```json
"footprintChoices": [{"package": "0402", "footprint": "LED-0402"}, …]
```

- Must be a non-empty array of objects with exactly the keys `package`
  (non-empty string) and `footprint` (string). Errors:
  `"footprintChoices" must be a non-empty array of {"package", "footprint"}`.
- Each `footprint` must be a loaded footprint
  (`unknown footprintChoices footprint "X"`). It must have a pad for every
  pin designator (`pin "2" has no pad in footprintChoices footprint "T1"`).
  It **may have more pads than the part has pins**. The extra pads are
  unconnected copper.
- No footprint id twice (`footprintChoices footprint "T2" listed twice`).
- The part's own `footprint` must be one of the choices, and that choice's
  `package` must equal the part's `package`. The part's `footprint` is the
  default choice. Errors: `"footprint" "T4" is not in footprintChoices` and
  `"package" "X" does not match footprintChoices "T2" ("0603")`.
- A supply rejects the key (`a supply has no footprint`, the existing message).
- The part's own `footprint` is still checked strictly by
  `load_part_footprint`: pad count equals pin count. So the default choice
  is always an exact fit.

Implementation in `library.c`:

- Add `"footprintChoices"` to `PART_KEYS`.
- Move the pin/pad loop out of `load_part_footprint` into
  `check_pads(ctx, part, footprint, what, id)`. `load_part_footprint` calls it
  with `what = key`. The new `load_footprint_choices` calls it with
  `what = "footprintChoices footprint"` and skips the pad-count equality
  test.
- `parts.h` gets:
  ```c
  typedef struct { const char *package; const PartFootprint *footprint; } PartFootprintChoice;
  ```
  and on `Part`:
  `const PartFootprintChoice *footprint_choices; size_t footprint_choice_count; /* 0 = only footprint */`.
  The choices are stored in file order.
- `footprint_check.c:149` also counts `part->footprint_choices[k].footprint`
  as a use.
- `tests/support/listing.c`, after `|connector=`, writes
  `|choices=<package>:<id>,…` for a part with choices.

## 5. LED kind: `colors`

- `led.c` `DEVICE_KEYS` becomes `{"colors", "anode", "cathode"}`, read with
  `kind_names`. An empty array fails with `"colors" must not be empty`. A
  device that still has `color` fails with the generic `unknown key "color"`.
- `PartLed` becomes `{ const char *const *colors; size_t color_count; }`.
  `colors[0]` is the default.
- `listing.c` writes `colors=Red,Green,Blue,Amber,White`.

## 6. Data: `led.json`

One device, replacing all five:

```json
{
  "format": "schematical-parts",
  "version": 1,
  "kind": "led",
  "devices": [
    {
      "id": "led-generic",
      "function": "LED",
      "description": "Generic LED",
      "drawer": ["Indicators","LED"],
      "pinouts": {
        "two-pin": [
          {"number":1,"name":"K","role":"input"},
          {"number":2,"name":"A","role":"input"}
        ]
      },
      "colors": ["Red","Green","Blue","Amber","White"],
      "anode": "A",
      "cathode": "K",
      "parts": [
        {"name":"GENERIC-LED","package":"Radial","pinout":"two-pin","footprint":"LED-RADIAL-5MM",
         "footprintChoices":[
           {"package":"0402","footprint":"LED-0402"},
           {"package":"0603","footprint":"LED-0603"},
           {"package":"0805","footprint":"LED-0805"},
           {"package":"1204","footprint":"LED-1204"},
           {"package":"1205","footprint":"LED-1205"},
           {"package":"1206","footprint":"LED-1206"},
           {"package":"1209","footprint":"LED-1209"},
           {"package":"1411","footprint":"LED-1411"},
           {"package":"1414","footprint":"LED-1414"},
           {"package":"DIP","footprint":"LED-DIP4"},
           {"package":"Radial","footprint":"LED-RADIAL-5MM"}
         ]}
      ]
    }
  ]
}
```

The choice order is the old part order, and it is the picker order in
plan-18-B. The `dip4` pinout is gone. The DIP choice uses `LED-DIP4`, whose
pads 3 and 4 are unconnected (§4).

## 7. App

**Catalog (`PartCatalog.swift`).**

- `Details.led(colors: [String])`, read from `PartLed.colors`.
- `struct FootprintChoice: Hashable, Sendable { let package: String; let footprint: Footprint }`.
  `CatalogPart.footprintChoices: [FootprintChoice]` is empty for parts
  without choices. `BackendCatalog.load` fills it from the footprints
  dictionary, as it does for `footprint`.
- `CatalogPart.color: String?` is replaced by `colors: [String]`: an LED's
  list, `[display.color]` for a 7-segment display, empty otherwise.
- `detailLabels` for an LED: `[colors[0]]` for one color, else
  `["\(n) colors"]`.
- The drawer subtitle's package slot (`ChipDrawerView.subtitle(for:)`) reads
  `"\(n) footprints"` for a part with choices. So the row is
  `GENERIC-LED` / `11 footprints · 5 colors`.
- `PartCatalogStore.packages` includes every choice's package (0402 … Radial
  stay in the Package menu). The package filter keeps a part whose package
  or any choice's package matches. The color filter keeps a part whose
  `colors` contains the color (parts with no colors pass, as today).
  `colors(in:)` lists every color of the parts in the category.

**Placed part (`SchematicModel.swift`, `BoardModel.swift`).**

- `PlacedChip` gains `var color: String?` and `var footprintID: String?`.
  `init` takes both (default nil) and normalizes them. For an LED part
  (`part.kind == PART_LED`), a nil or unknown color becomes
  `part.colors[0]`; for other parts it is nil. For a part with choices, a nil
  or unknown id becomes `part.footprint!.id`; for other parts it is nil.
- `PlacedChip.footprintChoice: CatalogPart.FootprintChoice?` is the choice
  whose footprint id is `footprintID`.
- `PlacedChip.footprint` (BoardModel) becomes:
  connector style → `part.connectorFootprint ?? base`, else `base`, where
  `base = footprintChoice?.footprint ?? part.footprint`.
- `placeChip(part:symbol:centeredAt:color:footprintID:)`: the two new
  parameters default to nil. The new chip is built first, then placed with
  `BoardLayout.nextSpot(for: chip.footprint, …)`.
- `DiodeSymbolView.lightColor(for:)` reads `chip.color ?? chip.part.colors.first`.

**Facts (`PartFacts.swift`).** `rows(for chip: PlacedChip)` replaces
`rows(for: CatalogPart)`. Package and Footprint come from
`chip.footprintChoice` when there is one. For an LED, the Color row is
`chip.color`. `SchematicSidebarView.swift:58` passes `chip`.

**File (`SchematicFile.swift`).**

- `currentVersion = 4`. `Part` gains `var color: String? = nil` and
  `var footprint: String? = nil`. Both are written only for a part with LED
  colors or with footprint choices, and are always written for those parts.
- `static let legacyLEDs: [String: (color: String, footprint: String)]` maps
  all 55 old names. It is built from colors RED/GREEN/BLUE/AMBER/WHITE →
  Red/Green/Blue/Amber/White and packages 0402, 0603, 0805, 1204, 1205, 1206,
  1209, 1411, 1414 → `LED-<pkg>`, DIP → `LED-DIP4`, RADIAL →
  `LED-RADIAL-5MM`. In `apply`, a part named in `legacyLEDs` loads
  `GENERIC-LED` with that color and footprint, in a file of any version.
  Like `VCC`, it is then saved under the new name.
- A `GENERIC-LED` entry whose `color` is not in the part's colors, or whose
  `footprint` is not one of its choices, fails the open with the new
  `SchematicFileError.badPartOption(reference:detail:)`:
  `"U3: \"Purple\" is not a color of GENERIC-LED"` and
  `"U3: \"LED-9999\" is not a footprint of GENERIC-LED"`. A missing `color`
  or `footprint` takes the part's default.
- In `apply`, the stored-placement snap (`SchematicFile.swift:425`) and
  the no-placement `nextSpot` (`:453`) use the chip's chosen footprint. The
  chip is built before its placement is snapped.

## Compatibility / Format Impact

- **Parts format:** new generic part key `footprintChoices`. The LED device key
  `color` is replaced by `colors`. A part file with an LED `color` key no
  longer loads.
- **Backend API:** `Part.footprint_choices` / `footprint_choice_count` are
  added, and `PartLed` changes shape. Part names `LED-<COLOR>-<PKG>` no longer
  exist, so `circuit_add_part(c, "LED-RED-0603", …)` fails. Callers use
  `GENERIC-LED`.
- **Schematic file:** version 4. Each part entry gains `color` and `footprint`
  (LED only, today). All files of version 1–3, and v4 files with old LED
  names, still open.
- Unchanged: `footprints.json`, every other part file, wire/bus/board formats.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line as it lands.
> An unticked box means NOT DONE.

### Phase 1 — Loader: `footprintChoices` and LED `colors`

The loader accepts both new keys. The real `led.json` is untouched in this
phase: LED devices accept `color` *or* `colors` until Phase 2 removes
`color`. Safe alone: no shipped data uses the new keys yet.

- [x] `backend/include/parts.h`: add `PartFootprintChoice`, the two `Part`
      fields, and `PartLed { const char *const *colors; size_t color_count; }`.
- [x] `backend/src/library/library.c`: `"footprintChoices"` in `PART_KEYS`;
      split out `check_pads`; add `load_footprint_choices` with the §4 rules
      and messages; call it after `load_part_footprint` for `"footprint"`.
- [x] `backend/src/models/led.c`: read `colors` with `kind_names` (non-empty).
      For this phase only, a device with `color` and no `colors` loads as a
      one-entry list. `DEVICE_KEYS` lists both.
- [x] `backend/tests/support/listing.c`: `colors=` joined with commas, and
      `|choices=<package>:<id>,…` after `|connector=`. (Phase 1 writes
      `color=<colors[0]>` so the golden stays byte-identical; `colors=` joined
      lands in Phase 2. See Corrections.)
- [x] `backend/tests/footprint_check.c:149`: count choice footprints as used.
- [x] Swift, so the app still builds: `PartCatalog.swift:436` reads
      `led.colors[0]` into the existing `.led(color:)` (temporary, replaced
      in Phase 3).
- [x] Tests in `backend/tests/library_check.c`: a valid fixture with
      `'colors':['Red','Blue']` and
      `'footprintChoices':[{'package':'0603','footprint':'T2'},{'package':'DIP','footprint':'T4'}]`
      on a two-pin part with `'footprint':'T2','package':'0603'` loads, with
      2 colors and 2 choices, and `choices[1].footprint` is T4 (4 pads,
      2 pins). One `expect_fail` per §4/§5 message: empty `colors`; empty
      `footprintChoices`; a choice with an extra key; unknown choice
      footprint; choice `T1` (pin 2 has no pad); `T2` listed twice; default
      footprint not among the choices; package mismatch; `footprintChoices`
      on a supply. Result: `ctest` 21/21; `library_check | grep -c '^FAIL'` → 0;
      the 10 new lines print `ok` (`grep -cE` over them → 10);
      `git diff --stat backend/tests/golden` → empty.

Acceptance: the new loader rules accept and reject exactly as §4/§5 say, and
the existing data still loads unchanged.
  Check: `cmake --build /tmp/p18-build -j8 && ctest --test-dir /tmp/p18-build -j8`
  → 21/21 pass. `/tmp/p18-build/library_check | grep -c '^FAIL'` → 0, and
  the 10 new cases print `ok`. `git diff --stat backend/tests/golden` → empty
  (no data changed) (est. 1 min).
Commit: d571090

### Phase 2 — Data and backend tests on `GENERIC-LED`

`led.json` becomes the one part. The `color` key goes away. Goldens are
regenerated. Lands with Phase 3 (the app does not compile in between).

- [x] `schematical/schematical/Parts/led.json`: replace with §6 exactly.
- [x] `backend/src/models/led.c`: remove the `color` fallback; `DEVICE_KEYS` =
      `{"colors","anode","cathode"}`.
- [x] `backend/tests/library_check.c`: the 6 LED fixtures' `'color':'Red'`
      → `'colors':['Red']` (lines 268, 353, 853, 1406, 1423, 1486). Line 279
      checks `dip->led->colors[0]`. Add `expect_fail` for an LED device with
      `'color'` → `unknown key "color"`. (`grep -c` → 6 before the sed.)
- [x] `backend/tests/circuit_check.c:628` (added): the ball-pinout LED fixture
      also had `\"color\":\"Red\"` → `\"colors\":[\"Red\"]`. Before the fix,
      `circuit_check` printed 4 FAILs ("a ball-pinout LED loads" …); after, 0.
- [x] `backend/tests/circuit_check.c` (5 lines), `voltage_test.c` (2),
      `sim_bench.c` (2): every old LED name → `"GENERIC-LED"`.
- [x] `backend/include/circuit.h:10`, `docs/backend.md:48`: example →
      `"GENERIC-LED"`.
- [x] Regenerate: `/tmp/p18-build/listing_check --print > backend/tests/golden/parts.txt`
      and `/tmp/p18-build/trace_check --print > backend/tests/golden/traces.txt`.
- [x] `docs/parts-format.md`: § Parts table gets a `footprintChoices` row.
      § The `"footprint"` part key gets the §4 rules (extra pads allowed in
      a choice, default must be a choice). § `led` table `color` →
      `colors` (first = default). § Footprints `footprint_check` sentence
      counts choices.

Acceptance: the backend sees exactly one LED part, and the golden diffs are
exactly the expected ones.
  Check: `ctest --test-dir /tmp/p18-build -j8` → 21/21.
  `git diff backend/tests/golden/parts.txt | grep -c '^-LED-'` → 55, `| grep -c '^+'` → 2
  (the `+++` header and one `+GENERIC-LED|led|Radial|Indicators/LED|LED|||colors=Red,Green,Blue,Amber,White|1:K:input:default,2:A:input:default|footprint=LED-RADIAL-5MM|choices=0402:LED-0402,…,Radial:LED-RADIAL-5MM`).
  Same counts for `traces.txt` (`^-LED-` → 55, one `+GENERIC-LED` line).
  `git diff --quiet backend/tests/golden/footprints.txt` → exit 0 (est. 1 min).
  Result: `ctest` 21/21. `parts.txt`: 55 `-LED-` lines, one `+GENERIC-LED|…|colors=Red,Green,Blue,Amber,White|…|footprint=LED-RADIAL-5MM|choices=0402:LED-0402,…,Radial:LED-RADIAL-5MM`, no other `-`/`+` line.
  `traces.txt`: 55 `-LED-` lines, one `+GENERIC-LED	97b67472c7b07e45`. `footprints.txt` not in the diff.
Commit: cc539fd

### Phase 3 — App: per-part color and footprint, file format 4, migration

The app compiles again. Every placed LED carries its color and footprint, and
old files migrate. This is the highest-risk phase.

- [x] `Backend/PartCatalog.swift`: §7 Catalog (`.led(colors:)`,
      `FootprintChoice`, `footprintChoices`, `colors`, `detailLabels`,
      `packages`, package/color filters, `colors(in:)`).
- [x] `Drawer/ChipDrawerView.swift:subtitle(for:)`: `"\(n) footprints"` package
      slot for parts with choices.
- [x] `Canvas/SchematicModel.swift`: `PlacedChip.color`, `footprintID`,
      `footprintChoice`, normalizing `init`; `placeChip` gets the two
      parameters and uses `chip.footprint` for `nextSpot`.
- [x] `Canvas/BoardModel.swift:95`: `footprint` uses the choice (§7).
- [x] `Canvas/DiodeSymbolView.swift:20`: color from `chip.color`.
- [x] `Backend/PartFacts.swift`: `rows(for chip:)`;
      `Drawer/SchematicSidebarView.swift:58` passes the chip.
- [x] `Document/SchematicFile.swift`: version 4, `color`/`footprint` fields
      written and read, `legacyLEDs`, `badPartOption`, chosen footprint in
      both placement paths.
- [x] Harness scripts: `bus-landing.swift` (2), `bus-erase.swift` (1),
      `bus-slide.swift` (1), `support/BusProof.swift` (2): `"LED-RED-0603"` →
      `"GENERIC-LED"`.
- [x] (added) `scripts/harness/multiselect.swift` (3 lines) and the unit tests
      `schematicalTests/SelectionModelTests.swift`, `SelectionSidebarTests.swift`,
      `SelectionGestureTests.swift`: `"LED-RED-0603"` → `"GENERIC-LED"`;
      the board-row assertion's prefix `"LED-RED-0603 — "` → `"GENERIC-LED — "`.
      After: `grep -rnE 'LED-(RED|GREEN|BLUE|AMBER|WHITE)-'` outside goldens,
      plans and `.schematical` files hits only `library_check.c`'s own
      fixture part names (269, 270, 278), which are test data, not library
      parts.
- [x] New `scripts/harness/generic-led-file.swift` (kept), which prints one
      `ok`/`FAIL` line per check and exits 1 on any FAIL:
  1. `be-6502.schematical`: U22 is `GENERIC-LED`, `color == "Green"`,
     `footprint?.id == "LED-RADIAL-5MM"`, `board.centre == (9.525, 13.97)`.
  2. `test.schematical` (v1): its LED is `GENERIC-LED`, Amber, `LED-0603`.
  3. Encoding (1) gives `"version" : 4` and, for U22,
     `"color" : "Green"`, `"footprint" : "LED-RADIAL-5MM"`,
     `"partNumber" : "GENERIC-LED"`. Decoding and applying it gives the same
     `SchematicFile` again (round trip equal).
  4. `placeChip(GENERIC-LED)` with no options → Red, `LED-RADIAL-5MM`.
     With `footprintID: "LED-0805"` → `chip.footprint?.id == "LED-0805"`
     and 2 `boardPads`. With `"LED-DIP4"` → 4 `boardPads`.
  5. A v4 file with U22 `"color": "Purple"` fails with
     `U22: "Purple" is not a color of GENERIC-LED`. One with
     `"footprint": "LED-9999"` fails with the footprint message.
  6. `PartFacts.rows(for:)` for the (4) 0805 chip has `Package` = `0805` and
     `Color` = `Red`.
  7. The catalog lists one LED-kind part (`GENERIC-LED`).
- [x] `docs/file-format.md`: version 4 in the version list; `color` and
      `footprint` rows in § Parts; the LED example (line 89) and the full
      example (line 351) use `GENERIC-LED` with both fields. A paragraph on
      the old `LED-<COLOR>-<PACKAGE>` names opening as `GENERIC-LED`, next to
      the `VCC` rename.

Acceptance: old files open with the right color, footprint and PCB place;
new files save and reopen the same; a chosen footprint drives the PCB.
  Check: `rm -rf /tmp/p18-h && scripts/harness/build.sh scripts/harness/generic-led-file.swift /tmp/p18-h && /tmp/p18-h/H.app/Contents/MacOS/harness`
  → 7 `ok` lines, exit 0 (est. 1.5 min. The fresh out dir is required:
  build.sh reuses an existing app copy, whose `led.json` would be old).
  Result: 7 `ok` lines (1 be-6502 U22 Green/LED-RADIAL-5MM at (9.525, 13.97);
  2 test.schematical Amber/LED-0603; 3 v4 encode + round trip; 4 placeChip
  defaults, 0805 → 2 pads, DIP4 → 4 pads; 5 both badPartOption messages;
  6 Package 0805 / Color Red; 7 one LED part), `exit=0`.
Commit: —

## Validation Plan

- Tests: `library_check` covers every new loader rule and rejection.
  `circuit_check` and `voltage_test` run `GENERIC-LED` in real circuits (lit,
  dark, oscillator blink, switch).
  `listing_check`/`trace_check`/`footprint_check` pin the data.
  `generic-led-file.swift` covers migration, the v4 round trip, bad values and
  the footprint choice on the PCB.
- Coverage check: `listing_check` prints a `GENERIC-LED` line
  (`/tmp/p18-build/listing_check --print | grep -c '^GENERIC-LED|'` → 1), so
  the new part is in the golden denominator. `footprint_check` reports no
  unused footprint, so all 11 LED footprints are reached through
  `footprintChoices`.
- Runtime proof: `generic-led-file.swift` check 1 on the real
  `be-6502.schematical`. Also `bus-slide.swift`, which places `GENERIC-LED`
  and runs the bus proof; it prints its existing pass lines.
- Doc sync: `docs/parts-format.md`, `docs/file-format.md`, `docs/backend.md`,
  `backend/include/circuit.h` (Phases 2–3).
- Final gate (once, after Phase 3):
  `cmake --build /tmp/p18-build -j8 && ctest --test-dir /tmp/p18-build -j8`
  → 21/21; the Phase 3 harness check; `scripts/harness/build.sh scripts/harness/bus-slide.swift /tmp/p18-h && /tmp/p18-h/H.app/Contents/MacOS/harness`
  → no FAIL lines; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p18-dd CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED` (est. 4 min total).

## Decisions

- One part number, `GENERIC-LED`, replaces the 55 LED parts; color and
  footprint are per placed LED and saved in the file. Decided by the owner,
  2026-11-07. Settles the data model (§4–§7) and format 4.
- A new Generic LED starts Red with the Radial footprint, and the default is
  configurable in Settings. Decided by the owner, 2026-11-07. Here: the data
  default (`colors[0]` = Red, `footprint` = `LED-RADIAL-5MM`). Settings is
  plan-18-B.
- The Color and Footprint options are in the Schematic sidebar only. Decided
  by the owner, 2026-11-07 (plan-18-B).
- The header shows the part number `GENERIC-LED` and the title "Generic LED".
  Decided by the owner, 2026-11-07. Done here by the device `description`
  `"Generic LED"`, which `PartHeader` already shows under the part number.

## Corrections

- **Prerequisite build dir.** `/tmp/p18-build` had been configured from the
  main checkout, so `cmake -S backend` in the worktree failed ("does not match
  the source … used to generate cache"). Fix: `rm -rf /tmp/p18-build` first,
  then the gate command as written → 21/21.
- **Phase 1 `listing.c` vs. its acceptance.** The task said Phase 1 writes
  `colors=`, but the acceptance says the goldens don't change, and every LED
  line would then change `color=Red` → `colors=Red`. Phase 1 writes
  `color=<colors[0]>` (same bytes for the one-color data). Phase 2 switches to
  `colors=` joined with commas, which is the line its acceptance expects.
- **§4 example message.** A default footprint with more pads than pins (`T4`
  on a two-pin part) fails the strict `load_part_footprint` check first, so
  it can never reach "not in footprintChoices". The test uses default `T2`
  with only `T4` offered → `"footprint" "T2" is not in footprintChoices`.
- **Phase 2 population.** The plan counted 6 `'color'` LED fixtures in
  `library_check.c`. `circuit_check.c:628` has a 7th (escaped-quote form,
  `\"color\":\"Red\"`), which the `grep` over `library_check.c` missed. It
  was moved to `colors` as an added task.
- **Old-name population grew (plan-15 landed after this plan was written).**
  `grep -rnE 'LED-(RED|GREEN|BLUE|AMBER|WHITE)-'` now also hits
  `scripts/harness/multiselect.swift` (3 lines) and the unit tests
  `schematical/schematicalTests/SelectionModelTests.swift` (7),
  `SelectionSidebarTests.swift` (7, two assert the row detail text) and
  `SelectionGestureTests.swift` (13). Added to Phase 3 as tasks.

## Summary

The risk is in Phase 3: the migration table and the two `apply` placement
paths that must use the chosen footprint. The harness check exercises all of
them on the repo's own files. The backend change is small and fully covered
by `library_check` and the goldens. Simulation, wires, footprints and every
non-LED part are untouched.
