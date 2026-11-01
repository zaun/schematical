# plan-01-G: App cutover and removal of the old backend

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-F

Switches the app to the new backend: it loads the bundled part files through
`load_parts`, reads parts as C structs, simulates through `circuit.h`, and
decides symbols and drawer filters by `PartKind` instead of category strings.
Then the old backend, the exporter and the baseline tools are deleted, the
engine moves to `backend/src/engine/`, and the Xcode project compiles
`backend/src/` through a synchronized folder.

Outcome: the app builds and runs on the new backend only; the drawer lists the
same parts with the same filters; play mode works; no code outside the
backend mentions `ChipDefinition`, `catalog_json_copy`, `netlist_sim_` or a
category string.

References:

- `planning/plan-01-A-parts-baseline.md` §3 (design), §3.6 (expected changes).
- `planning/plan-01-F-circuit-api.md` (`circuit.h`), `docs/backend.md`.
- App: `schematical/schematical/Backend/ChipCatalog.swift`,
  `Drawer/ChipDrawerView.swift`, `Canvas/SchematicModel.swift`,
  `Canvas/SchematicCanvasView.swift`, `Canvas/DiodeSymbolView.swift`,
  `Canvas/SevenSegmentSymbolView.swift`, `Canvas/DipSwitchSymbolView.swift`,
  `Simulation/Simulation.swift`, `Memory/MemoryWindow.swift`,
  `schematical-Bridging-Header.h`, `schematical.xcodeproj/project.pbxproj`.
- Docs: `AGENTS.md`, `docs/file-format.md`.

## Prerequisites

See plan-01-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-F complete | `ls planning/completed/plan-01-F-circuit-api.md` → exists | MET (2026-10-31: exists, e2c33d6) |
| Library complete | `ctest --test-dir /tmp/logic-build -R 'listing_check|trace_check' --output-on-failure` → `3817 parts compared, 0 skipped` | MET (2026-10-31: both `3817 parts compared, 0 skipped`) |

## 1. Goal

- `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical build` succeeds with
  the bridging header including only `parts.h` and `circuit.h`.
- `grep -rnE 'ChipDefinition|catalog_json_copy|netlist_sim_|familyCategory|ledCategory|ledGroup|sevenSegmentGroup|switchCategory|memoryCategory|"Logic"|"Indicators"|"LED 7-Seg"' schematical/schematical --include='*.swift'`
  → no matches. (`"Memory"` stays legitimately as the memory window's title,
  `schematicalApp.swift:42` and `MemoryWindow.swift:104`; the category
  constant `memoryCategory` at `MemoryWindow.swift:14` goes.)
- `backend/` contains only `CMakeLists.txt`, `include/`, `src/`, `tests/`.
- All backend CTests pass.

### Non-goals

- `.schematical` files keep their format and open unchanged (part numbers are
  unchanged; plan-01-A §Non-goals).
- The drawer's look and grouping stay the same, apart from the
  multi-pinout merges (plan-01-A §3.6 item 1).

## 2. Current State (read)

- `ChipCatalog.swift:181-210` loads `catalog_json_copy()` and decodes
  `ChipCatalog`; `:216-229` hard-codes the strings `"Logic"`, `"Indicators"`,
  `"LED"`, `"LED 7-Seg"`, `"Switch"`; `:364-378` picks the symbol from category,
  `display` presence and LED/oscillator name sets; `:516-561` wraps
  oscillators, supplies and LEDs as fake chips with `gateCount: 0`.
- `DiodeSymbolView.swift:19` picks LED color by switching on `family`.
- `SchematicCanvasView.swift:95` opens the memory window when
  `category == memoryCategory`.
- `Simulation.swift` builds netlist text (`NetlistBuilder`) and parses snapshot
  text; it calls `netlist_sim_*` 8 times; LEDs come back as `litChips`,
  displays as `glowingLEDs`.
- Xcode: the backend is an explicit group `../backend` with 42 hand-listed
  build/file entries (`grep -c A0B0C0E0 project.pbxproj` → 42); the app folder
  is a synchronized root group, and JSON files placed in it land in
  `Contents/Resources/` (verified 2026-10-31 by building a copy with
  `Parts/gate.json` → `schematical.app/Contents/Resources/gate.json`).
- A synchronized root group with `path = ../backend/src` compiles the `.c`
  files under it into the app (verified 2026-10-31 on a copy of the project:
  `probe_kind.c` compiled, `_probe_kind_value` present in the binary).

### Measured populations

| What | Count | Command |
|---|---|---|
| Old backend files to delete (all but `CMakeLists.txt`, `sim.*`, `net.*`) | 57 | `ls backend \| grep -vE '^(CMakeLists.txt\|sim.c\|sim.h\|net.c\|net.h)$' \| wc -l` (before plan-01-A added `include/ src/ tests/ tools/`) |
| Swift references to `ChipCatalog`/`ChipDefinition`/`PlacedChip` | 46 | `grep -rn 'ChipCatalog\b\|ChipDefinition\b\|PlacedChip\b' schematical/schematical \| wc -l` |
| Swift files calling the old C API | 2 (`Simulation.swift` 8 calls, `ChipCatalog.swift` 2) | `grep -rcE 'netlist_sim_\|catalog_json_copy'` |
| Explicit backend entries in `project.pbxproj` | 42 | `grep -c A0B0C0E0` |

## 3. Design

### 3.1 Backend reorganization

- `git mv backend/sim.c backend/sim.h backend/net.c backend/net.h backend/src/engine/` (Phase 1).
- `git rm` the 57 old files, `backend/tools/` (baseline + export), and their
  CTests (`catalog_check`, `netlist_check`, `sram_check`, `switch_check`,
  `led7_check`, `baseline_listing`, `baseline_traces`).
- `backend/CMakeLists.txt`: only the `logic` library and the new tests
  (`library_check`, `listing_check`, `trace_check`, `switch_test`,
  `sram_test`, `display7_test`, `circuit_check`). `golden/` stays: from now on
  the goldens are the regression fixtures for the JSON data.
- `basicsim/CMakeLists.txt`: unchanged target name, links `logic`.

### 3.2 Xcode

- Remove the `../backend` PBXGroup, its 42 file/build entries and the
  backend files from the Sources phase.
- Add `PBXFileSystemSynchronizedRootGroup` `path = ../backend/src`, listed in
  the main group and in the target's `fileSystemSynchronizedGroups` (the shape
  verified in §2).
- `HEADER_SEARCH_PATHS` (Debug and Release, `project.pbxproj:400`, `:435`):
  `$(PROJECT_DIR)/../backend/include`, `…/backend/src`, `…/backend/src/engine`,
  `…/backend/src/library`, `…/backend/src/third_party/cjson`.
- `schematical-Bridging-Header.h`: `#include "parts.h"` and `#include "circuit.h"` only.

### 3.3 Swift

- `Backend/PartCatalog.swift` replaces `ChipCatalog.swift`:
  - `final class PartLibraryHandle` owns the `OpaquePointer` from
    `part_library_create()`; `deinit` destroys it. One instance for the app's
    life (circuits borrow it).
  - Loading: every URL from
    `Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil)`,
    sorted by file name, read as UTF-8 and passed to `load_parts` with the file
    name as `source`. Any error → `LoadError.malformed(message)` (the existing
    error path); all bundled JSON resources are part files.
  - `CatalogPart` (Swift value copied from `Part` once at load): `name`,
    `kind: PartKind`, `package`, `function`, `description`, `drawer: [String]`,
    `flags` (obsolete/military/automotive/enhanced), `pins: [CatalogPin]`,
    `details: Details` with
    `case logic(family: String, units: Int)`, `sram(words:, bits:, accessNs:, grade:)`,
    `dipSwitch(positions:, actuator:)`, `led(color:)`,
    `display(DisplayInfo)`, `oscillator(family:, minHz:, maxHz:, defaultHz:, enablePin:)`,
    `supply(level:)`.
  - Drawer grouping from `drawer` + `function` (same tree as today);
    `drawer == []` parts are not listed.
  - Filters by kind: family and unit count for `.gate/.dff/.jkff`; positions
    for `.switch`; color for `.led/.sevenSeg`; digits and size for `.sevenSeg`.
  - Symbol by kind: gate/dff/jkff/sram → block; switch → dipSwitch; led →
    diode; 7seg → sevenSegment; oscillator → oscillator; supply → vcc/ground by
    `level`.
- `Canvas/SchematicModel.swift`: `PlacedChip` holds `part: CatalogPart`
  instead of a `ChipDefinition` + part number.
- `Canvas/DiodeSymbolView.swift`: color from `.led(color)`.
- `Canvas/SevenSegmentSymbolView.swift`, `DipSwitchSymbolView.swift`: data
  from `details`.
- `Canvas/SchematicCanvasView.swift:95`: memory window when `kind == .sram`.
- `Simulation/Simulation.swift`: `NetlistBuilder` returns the part order and
  pin→net assignments instead of text; `SimulationController` builds a
  `Circuit` (`circuit_create`, `circuit_add_part`, `circuit_add_net`,
  `circuit_connect`, `circuit_set_switch` for positions that start ON,
  `circuit_start`), advances with `circuit_advance`, and reads
  `circuit_net_value`, `circuit_shorted_net`, `circuit_error`,
  `circuit_part_lights`. `litChips` and `glowingLEDs` become one
  `lights: [UUID: [Bool]]` (an LED has one light).
- `Memory/MemoryWindow.swift`: `circuit_memory_size`/`circuit_memory_read`.

### 3.4 Docs

- `AGENTS.md` Datasheets: flag obsolete parts with `"flags": ["obsolete"]` in
  the kind's `schematical/schematical/Parts/<kind>.json` (replaces
  `PART_FLAG_OBSOLETE`); new parts are added by editing those files
  (format: `docs/parts-format.md`).
- `docs/file-format.md:12` and `:74`: "backend catalog" → "part library
  (`schematical/schematical/Parts/*.json`)".
- `docs/backend.md` (from F): final file layout.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — Engine move, Xcode switch, Swift on the new API

- [x] Move the engine first: `git mv backend/sim.c backend/sim.h
      backend/net.c backend/net.h backend/src/engine/`; in
      `backend/CMakeLists.txt` point both `logic_backend` and `logic` at
      `src/engine/sim.c`/`net.c` and add `src/engine` to both include paths
      (the old target still builds until Phase 2). The synchronized
      `../backend/src` folder can only see the engine once it is there.
      (Also `export_parts`, which compiles the engine too.) After the move:
      `ctest` → 14/14 passed, `basicsim` unchanged.
- [x] Xcode: add the `../backend/src` synchronized group and the header
      paths (§3.2) **and** remove the `../backend` explicit group in the same
      commit: the explicit group's `sim.c`/`net.c` entries point at files
      that just moved, and the app must stop compiling the old models.
      `grep -c A0B0C0 project.pbxproj` → 0. Header paths also include
      `src/models`.
- [x] Bridging header (§3.2).
- [x] Swift changes (§3.3), deleting `Backend/ChipCatalog.swift`.
      `Backend/PartCatalog.swift` (`PartLibraryHandle`, `CatalogPart` with
      `Details`, `PartCatalogStore`, `symbol(for:)` by kind, filters by
      kind); `PlacedChip.part`; drawer headers by the kinds in a category;
      `Simulation.swift` on `circuit.h` with one `lights` map. The Swift
      pin-role enum is `ChipPinRole` (the C `PinRole` is now imported).

Acceptance: the app builds; no old API or category string in Swift.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min).
  Check: the §1 `grep` → no matches (est. seconds).
  Result: clean build `** BUILD SUCCEEDED **` (one warning, at
  `SchematicModel.swift:211`, present on `main` before this plan); §1 grep →
  no matches (exit 1); the app bundle's Resources hold the nine part files.
Commit: —

### Phase 2 — Delete the old backend, move the engine

- [ ] §3.1 in one commit (the engine already moved in Phase 1): `git rm`
      the 57 old files and `backend/tools/`, drop `logic_backend` and the old
      CTests from CMake, test include paths updated.
- [ ] `basicsim/CMakeLists.txt` include paths for `src/engine`.

Acceptance: only the new layout remains and everything still builds and passes.
  Check: `ls backend` → `CMakeLists.txt include src tests`; `cmake -S backend -B /tmp/logic-build-g && cmake --build /tmp/logic-build-g -j8 && ctest --test-dir /tmp/logic-build-g` → 7 tests pass (est. 3 min).
  Check: the Phase 1 `xcodebuild` command → `** BUILD SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 3 — Docs and archive

- [ ] §3.4 doc edits.
- [ ] Move `planning/plan-01-G-app-cutover.md` to `planning/completed/`.

Acceptance: no doc names a removed symbol.
  Check: `grep -rnE 'PART_FLAG_OBSOLETE|catalog_json_copy|netlist_sim|backend catalog' AGENTS.md docs/` → no matches (est. seconds).
Commit: —

## Validation Plan

- Tests: all backend CTests (§3.1 list).
- Coverage check: `listing_check` and `trace_check` still report
  `3817 parts compared, 0 skipped` after the move.
- Runtime proof (manual, with the `run` skill): launch the Debug build, open
  `test.schematical` (VCC, GND, `OSC-SIT1534-1508`, `LED-AMBER-0603`): the
  drawer shows Logic, Memory → SRAM, Switch → DIP, Indicators → LED and
  LED 7-Seg, Oscillator with today's filters; press Play: the LED is drawn lit
  and no error appears; place an `AS6C62256-55PCN`, Play, open its memory
  window: all words show X. Screenshot each.
- Doc sync: `AGENTS.md`, `docs/file-format.md`, `docs/backend.md`,
  `docs/parts-format.md`.
- Final gate (plan-01, run once): `cmake -S backend -B /tmp/logic-final && cmake --build /tmp/logic-final -j8 && ctest --test-dir /tmp/logic-final` (est. 3 min) + the Phase 1 `xcodebuild` (est. 3 min) + the runtime proof (est. 10 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

- plan-01-F's last commit (archive + Phase 2) is e2c33d6.
- §3.3: the Swift enum `PinRole` clashes with the C `PinRole` now imported
  through the bridging header (`parts.h`), so the Swift one is
  `ChipPinRole` (adds `absent`; `ChipPin.isHidden` = role `absent`).
  `CatalogPart` is `Hashable` by name (names are unique), since the imported
  `PartKind` is a C enum.
- The synchronized `../backend/src` folder also copies
  `third_party/cjson/LICENSE` into the app's Resources (the cJSON license
  ships with the app; nothing else in `src/` is a resource).

## Summary

The Swift work is the largest single change in plan-01 (46 references to the
old catalog types), but it sits on a backend whose data and behavior are
already proven equal to today's. The one-commit Xcode swap in Phase 1 is the
riskiest edit (hand-edited `project.pbxproj`); its shape was built and run on
a copy of the project before this plan was written.
