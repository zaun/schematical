# plan-11-D: Character LCDs in the app — drawer, live face, PCB Style option

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-11-C

This sub-plan brings the character LCD modules (plan-11-C) into the app:

- The parts drawer lists them under **Displays → Character**, with their
  size (16×2), panel and backlight.
- The schematic sidebar shows their facts.
- On the schematic, a placed module is a box with the bus pins on the left
  and the power pins on the right. Between them is the **display face**.
  During a simulation the face shows the dots the controller displays,
  every frame, lit by the backlight.
- On the PCB page, the footprint panel gets a **Style** option for any part
  with a connector footprint: **Full Module** (default) or **Connector
  Only**. Switching keeps the part in place and moves nothing else; the
  choice is saved in the file.

Outcome of D, and of plan-11: the CPU board of plan-11-C §4.6, opened as a
schematic and run, shows `HELLO` on the module's face. Its PCB footprint
switches between the 80 × 36 mm module and the 1×16 header, and the choice
survives save and reopen.

References:

- plan-11-A § Prerequisites (the gate), § Decisions; plan-11-B §4.7
  (`LcdState`, the dot image); plan-11-C §4.1–4.3 (the parts and
  footprints), §4.6 (the CPU board).
- `docs/file-format.md` (§ Parts `pcb`, § Validation on open, § Changing
  the format).
- `planning/completed/plan-09-E-cpu-app.md` (the last kind brought into the
  app; same steps).

## Prerequisites

The gate is plan-11-A § Prerequisites. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-11-C complete | `grep -c '^Commit: —' planning/plan-11-C-charlcd-parts.md` → 0 (or the file is in `planning/completed/`) | MET (2026-11-01 at 36dbe1e: 0 once C's last hash is filled, in the D Phase 1 commit) |
| The parts load | `grep -c '\|charlcd\|' backend/tests/golden/parts.txt` → 5 | MET (2026-11-01 at 36dbe1e: 5) |
| `circuit_lcd_state` exists | `grep -n 'circuit_lcd_state' backend/include/circuit.h` → one line | MET (2026-11-01 at 36dbe1e: circuit.h:86) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- The app builds (`** BUILD SUCCEEDED **`).
- The drawer has a **Displays** category with **Character** under it,
  listing the 5 parts.
- The CPU board schematic (§4.7) shows `HELLO` on the LCD face while
  running (screenshot).
- On the PCB page, Style switches the footprint between `LCM-NHD-0216K1Z`
  and `HDR-1X16-P2.54`, with pin 1's pad staying where it was. Saving
  writes `"style": "connector"`, and reopening restores it (screenshot plus
  `grep`).
- `ctest` still 18 of 18; goldens unchanged.

### Non-goals (explicit constraints)

- **File format**: version stays 2. `pcb.style` is an optional key
  (`docs/file-format.md` § Changing the format). Files without it open as
  Full Module. Files from this build open in older builds, which ignore the
  key (Swift `Codable` skips unknown keys).
- No change to how any existing part is drawn, placed or saved. `style` is
  written only for parts with a connector footprint, so saving a file
  without LCDs gives the same bytes as before.
- No RAM window for the LCD (owner, 2026-11-01).
- The 7-segment displays stay under Indicators (owner, 2026-11-01).
- No backend change. A backend fix found here is a correction to B or C,
  recorded there.

## 2. Current State

- **Catalog.** `CatalogPart` (`Backend/PartCatalog.swift`) copies `Part`.
  Its parts:
  - `Details` has one case per kind (`.display`, `.cpu`, …);
  - `footprint` comes from `part.footprint` (`BackendCatalog.load`, by id);
  - `kindName` (`PartFacts.swift:74-91`) names the kind;
  - `symbol(for:)` (`PartCatalog.swift:516`) maps `PART_7SEG` →
    `.sevenSegment` and `PART_CPU` → block;
  - `listsPartsDirectly` lists switches, LEDs and 7-segs without a
    function level.
- **Drawer.** `ChipDrawerView.categoryHeader` shows filter menus per kind
  (`ChipDrawerView.swift:179-197`). A category with none of the listed
  kinds shows no menu.
- **Symbols.** `PartSymbol` (`SchematicModel.swift:78-86`).
  `ChipLayout(part:symbol:)` builds a layout per symbol: `.sevenSegment`
  puts segment pins left and commons right, with rows enough for the face
  (`SchematicModel.swift:195-230`). `SchematicCanvasView` picks the view per
  symbol and passes `simulation.lights[chip.id]`
  (`SchematicCanvasView.swift:160-192`).
- **Simulation plumbing.**
  - `SimulationEngine.snapshotNow` (`SimulationEngine.swift:188-210`) reads
    nets, light times, the shorted net and the error under the lock.
  - Light parts are found once by asking `circuit_part_light_time`
    (`Simulation.swift:235-237`).
  - `Simulation` turns snapshots into `lights: [UUID: [Double]]`
    (`Simulation.swift:327-345`). `netlist.chipIDs[part]` maps a part
    index to its chip.
- **PCB, one footprint per part.** 13 places use the part's footprint for
  the board (`grep -rn 'part\.footprint\|catalogPart\.footprint'
  --include='*.swift' schematical/schematical | grep -v Backend/ | wc -l`
  → 13):
  - `BoardModel.swift:94,107,113,119,126` (pads, body, bounds, origin,
    pin-1 mark);
  - `SchematicModel.swift:441` (whether a board placement exists), `:763`
    (place a new part), `:811,823,837` (move, move origin, rotate);
  - `SchematicFile.swift:340` (snap an older file's placement), `:376`
    (place a part with no stored spot);
  - `PCBSidebarView.swift:230` (the panel header).
- **Placement and file.**
  - `BoardPlacement` (`BoardModel.swift:32-41`) has centre, rotation,
    origin and the three marks.
  - `SchematicFile.Placement` (`SchematicFile.swift:71-79`) stores them as
    optional strings.
  - `SchematicFile.value(_:of:_:default:)` (`:518-528`) parses an enum and
    fails with `"<name> \"<raw>\" must be a or b"`, shown as `The PCB
    settings are not valid: …`.
- **PCB sidebar.** `FootprintPanel` (`PCBSidebarView.swift:219-283`): a
  header (part, then `footprint.id — description`), Reference, Position
  (Origin picker, X, Y, rotation) and Marks.
- **No app unit tests.** The project has no test target
  (`ls schematical` → `Info.plist schematical schematical.xcodeproj`).
  App checks are the build plus runtime screenshots, as in plan-09-E and
  plan-10-J.

### Measured populations

| What | Count | Command |
|---|---|---|
| Board/file sites reading the part's footprint | 13 | command in §2 |
| Symbol kinds today | 7 | `sed -n '/enum PartSymbol/,/^$/p' Canvas/SchematicModel.swift \| grep -c '    case '` |
| Character LCD parts | 5 | plan-11-C |

### Verified properties

- **Unknown JSON keys are ignored by older builds.** `SchematicFile.Part`
  and `Placement` are plain `Codable` structs with no custom `init(from:)`
  rejecting extra keys (`SchematicFile.swift:40-79`), so an older build
  decoding `"style"` skips it.
- **Every board site can take the chip's footprint instead of the part's.**
  All 13 have the `PlacedChip` (or its index) at hand, except
  `SchematicModel.swift:441` and `:763`. Those two run when a chip is made,
  when the style is the default (Full Module), so they keep
  `part.footprint`.
- **A duplicate designator is safe in the board code** (plan-11-A §2): the
  pin-1 mark and the Pad 1 Center origin use the first pad of the pin.

## 3. Design

1. **Catalog** (§4.1): `CatalogPart.connectorFootprint`, the
   `.characterLCD(CharacterLCDInfo)` details, the kind name, symbol,
   drawer labels and facts.
2. **Live face** (§4.2–4.3): the engine copies each LCD's `LcdState` into
   the snapshot every frame. `Simulation` publishes `lcdFrames: [UUID:
   LCDFrame]`. `CharacterLCDSymbolView` paints dots from the frame, lit by
   the backlight level from `lights`.
3. **PCB Style** (§4.4–4.6): `BoardPlacement.style`, and a computed
   `PlacedChip.footprint` that every board site reads.
   `setFootprintStyle` keeps pin 1's first pad fixed. The file gets
   `pcb.style`, and the sidebar a picker.

**Where the risk is:** the 13 footprint sites. One missed site would draw
or hit-test the wrong footprint after a switch. The runtime check
(Phase 4) switches the style and then moves, rotates, re-origins, saves
and reopens. Each site is on that path.

**Correctness gate:** the app build plus runtime screenshots (there is no
app test target). The backend gate (`ctest`, goldens unchanged) stays
green because no backend file changes.

**Rejected alternatives**

- *Asking the backend which parts are LCDs* by calling `circuit_lcd_state`
  on each part. Rejected: on a non-LCD that call sets `circuit_error`, and
  the snapshot reads `circuit_error` as the run's error. The app selects
  LCD parts by `PART_CHARLCD` from the catalog.
- *Keeping the placement's centre fixed on a style switch.* Rejected: the
  module and the header have different centres, so the header would land
  mid-module, away from where the wires go. Pin 1 staying put is what a
  layout person expects.
- *A per-document default style.* Not asked for. Every new LCD starts as
  Full Module (owner).

## 4. Detailed Design

### 4.1 Catalog and facts

- `CatalogPart`:
  - `let connectorFootprint: Footprint?`, copied like `footprint` (from
    `part.connector_footprint`, by id);
  - `Details` gains `.characterLCD(CharacterLCDInfo)`, where
    `CharacterLCDInfo` has `columns`, `rows`, `controller`, `rom`,
    `panel: LCDPanel` (`.yellowGreen`, `.blueNegative`), `backlightColor:
    String?` and `busPins: [Int]`.
- `PartFacts.kindName`: `PART_CHARLCD` → `"Character LCD"`.
- `symbol(for:)`: `PART_CHARLCD` → `.characterLCD`.
- `listsPartsDirectly`: also true for `PART_CHARLCD`. The parts then sort
  by name, since `count` is nil for them.
- `trailingLabel`: `"16×2"` (`"\(columns)×\(rows)"`).
- `detailLabels`: `[panel title, backlight color]`, e.g.
  `["Yellow-green", "Yellow-green backlight"]`,
  `["Blue negative", "White backlight"]`.
- `voltageLabel`: unchanged code; it shows `4.5–5.5 V` from `vcc`.
- `PartFacts.rows`, for `.characterLCD`, adds:
  - "Characters" `16 × 2`;
  - "Controller";
  - "Character ROM" `A00 (Japanese)`;
  - "Panel": `STN yellow-green, positive` or `STN blue, negative`;
  - "Backlight": the color, or `None`.
  It also adds a "Connector" row, `HDR-1X16-P2.54 — <description>`, for any
  part with a connector footprint, after "Footprint".
- **Drawer.** The category is "Displays" and the subcategory "Character",
  from the part file. `categoryHeader` needs no case: no menu, since five
  parts need no filter.

### 4.2 Symbol and layout (`.characterLCD`)

- `PartSymbol.characterLCD`, with `canFlip` true (as `.block`).
- `ChipLayout`:
  - left side: `busPins` in order (RS, R/W, E, then DB0–7 after a one-row
    gap);
  - right side: every other drawn pin in pin order (VSS, VDD, V0, LED+,
    LED-);
  - rows: enough for the face;
  - body width: name column (36) + face width + name column (36).
- **Face**:
  - cells 5 × 8 dots, dot 2 pt square on a 2.5 pt pitch;
  - a cell is 12 pt wide (4 × 2.5 + 2) and 19.5 pt tall (7 × 2.5 + 2);
    the cell pitch is 15 pt across (one empty dot column between cells) and
    25 pt down (two empty dot rows between lines), plus a 6 pt margin all
    round;
  - for 16×2 that is 15 × 15 + 12 + 12 = 249 pt by 25 + 19.5 + 12 =
    56.5 pt.
- **Colors** (by panel; the backlight level 0…1 from
  `simulation.lights[chip.id]?.first` blends "off" to "on"):

  | Panel | Background on / off | Dot lit on / off | Dot unlit | Dot X |
  |---|---|---|---|---|
  | yellow-green | `#A9C53C` / `#7A8A3A` | `#1E2810` / `#1E2810` | background darkened 6 % | 50 % blend of lit and unlit |
  | blue-negative | `#1F45D6` / `#0E1D5C` | `#EAF2FF` / `#34467F` | background darkened 6 % | 50 % blend of lit and unlit |

  Outside play mode the face draws every dot unlit at backlight 0.
- `CharacterLCDSymbolView`, beside `SevenSegmentSymbolView`, takes `chip`,
  `isSelected`, `isHovered`, `frame: LCDFrame?` and `backlight: Double`.
  It draws the box, header and pins as `SevenSegmentSymbolView` does.
- `SchematicCanvasView`: a `.characterLCD` case passing
  `simulation.lcdFrames[chip.id]` and the light level.

### 4.3 Simulation plumbing

- `LCDFrame` (`Simulation/SimulationEngine.swift`): `columns`, `rows`,
  `dots: [UInt8]`. The dots are row-major over `rows × 8` by
  `columns × 5`, copied from `LcdState.dots` (plan-11-B §4.7).
- `SimulationEngine.init` gains `lcdParts: [Int]`. `snapshotNow` reads
  `circuit_lcd_state` for each one into `SimulationSnapshot.lcdFrames:
  [Int: LCDFrame]`.
- `Simulation`, when it starts:
  `lcdParts = netlist.chipIDs.indices.filter { chip(netlist.chipIDs[$0]).part.kind == PART_CHARLCD }`.
  On each snapshot it publishes `lcdFrames: [UUID: LCDFrame]`, cleared on
  stop.

### 4.4 Placement style

- `BoardSettings.swift`:
  `enum FootprintStyle: String, CaseIterable { case module, connector }`,
  titles "Full Module" and "Connector Only".
- `BoardPlacement.style: FootprintStyle = .module`.
- `PlacedChip.footprint: Footprint?` is
  `board?.style == .connector ? part.connectorFootprint ?? part.footprint : part.footprint`.
- The 13 sites (§2):
  - `BoardModel.swift` ×5, `SchematicModel.swift:811,823,837`,
    `SchematicFile.swift:340` and `PCBSidebarView.swift:230` read
    `chip.footprint` (or `chips[index].footprint`);
  - `SchematicModel.swift:441,763` and `SchematicFile.swift:376` keep
    `part.footprint` (a new placement is Full Module).
- `SchematicDocument.setFootprintStyle(_ id: UUID, _ style: FootprintStyle)`.
  It does nothing unless the part has a connector footprint and the style
  changes. Otherwise:
  1. `anchor` = the board point of the current footprint's first pad named
     `part.firstPinDesignator`;
  2. set the style;
  3. move `centre` so the new footprint's first such pad lands on `anchor`;
  4. `BoardLayout.snapped` with the new footprint.
  Rotation, origin and marks are kept.

### 4.5 File format

- `SchematicFile.Placement` gains `var style: String? = nil`.
- Write: `style` is `$0.style.rawValue` when the chip's part has a
  connector footprint; otherwise nil, so the key is left out.
- Read: for a part with a connector footprint, `placement.style =
  try Self.value(pcb.style, of: FootprintStyle.self, "\(reference) style",
  default: .module)`. A bad value fails Open with `The PCB settings are not
  valid: U2 style "x" must be module or connector.` For other parts the key
  is ignored.
- Older-file snapping (`:340`) uses the footprint of the stored style.
- `docs/file-format.md`:
  - § Parts, `pcb` row: "`style` (string, default `"module"`): `"module"`
    (Full Module) or `"connector"` (Connector Only), which of the part's
    two footprints is on the board. Written only for parts with a connector
    footprint (the character LCD modules); ignored on other parts".
  - § Validation on open gains the `style` message.
  - The `partNumber` row lists character LCD modules.

### 4.6 PCB sidebar

- `FootprintPanel` header: `chip.footprint` (the current one).
- A `Section("Footprint")` before Position, shown only when
  `chip.part.connectorFootprint != nil`, holds a segmented `Picker("Style")`
  with Full Module and Connector Only, bound to `chip.board.style` and
  `document.setFootprintStyle`.

### 4.7 Runtime proof: the CPU board as a schematic

The board is the plan-11-C §4.6 netlist. A generator script outside the
repo, `/tmp/p11-demo/make_board.py` (`python3 -I`), writes
`/tmp/p11-demo/lcd-6502.schematical` (format version 2):

- the parts at grid positions 200 units apart;
- one wire per connection, pin to pin by designator, no bends; a net with
  more than two pins becomes a chain of wires;
- the EEPROM's `memoryFile` points to `/tmp/p11-demo/hello.bin`: 32768
  bytes, the plan-11-C §4.6 program at offset 0 and the vectors at
  7FFA–7FFF;
- the switch's `switches` is `[true, false]` (position 1 closed). It is
  opened by hand in the running app after a second;
- the LCD's `pcb` has `"style": "connector"`.

The file opens without an error, and Run shows `HELLO` once the reset
switch is opened. The MPS6502 is obsolete, so turn on "show obsolete" in
the app's preferences if the drawer hides it. Opening a file does not
filter parts.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work it describes; `- [~]` for partial with one line of
> what remains; mark moot tasks `- [x] ~~text~~ — moot: <evidence>`; fill
> each `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Catalog, drawer and facts

- [x] `Backend/PartCatalog.swift`: `connectorFootprint`, `.characterLCD`
      details, `symbol(for:)`, `listsPartsDirectly`, `trailingLabel`,
      `detailLabels` (§4.1).
- [x] `Backend/PartFacts.swift`: kind name and fact rows (§4.1).
- [x] `Canvas/SchematicModel.swift`: `PartSymbol.characterLCD` and its
      `ChipLayout` (§4.2). Until Phase 2 it is drawn by `ChipBlockView`.
- [x] Added: `Drawer/ChipDrawerView.swift` gains
      `init(catalog:openGroups:)` (default: none open, so the app is
      unchanged), so the harness can render Displays → Character open. And
      the harness itself: `/tmp/p11/harness` (plan-09-E's approach: the app's
      Swift and the backend C in one CLI inside a copy of the Debug app,
      rendering real views offscreen to PNG; see Corrections).

Acceptance: the drawer shows Displays → Character with the 5 parts and
their labels, and the sidebar shows the facts.
  Check: the app build → `** BUILD SUCCEEDED **`; launch it, open the drawer, screenshot Displays → Character and the sidebar of `NHD-0216K1Z-NSW-BBW-L` (est. 6 min; no app test target, so a screenshot is the smallest check of what the drawer lists).
  Ran: app build → `** BUILD SUCCEEDED **`; `run catalog` → `Displays →
  Character: functions 0, parts 5`, each `[16×2]` with `Module · Yellow-green
  · Yellow-green backlight · 4.5–5.5 V` (BZ 4.7–5.3 V; NSW `Blue negative ·
  White backlight`); `run shots` → `/tmp/p11-demo/drawer.png` and
  `sidebar.png`, looked at: the five rows under Displays → Character, and
  the sidebar's Kind, Footprint, Connector, Characters 16 × 2, Controller
  ST7066U, Character ROM A00 (Japanese), Panel STN blue, negative,
  Backlight White, Supply range 4.5–5.5 V.
Commit: eb7e99c

### Phase 2 — Live face

- [x] `Simulation/SimulationEngine.swift`: `LCDFrame`, `lcdParts`,
      `lcdFrames` in the snapshot (§4.3).
- [x] `Simulation/Simulation.swift`: `lcdParts` by kind, `lcdFrames`
      published and cleared (§4.3).
- [x] `Canvas/CharacterLCDSymbolView.swift`: the view (§4.2).
- [x] `Canvas/SchematicCanvasView.swift`: the `.characterLCD` case.
- [x] Harness mode `face`: places the module, wires VDD and LED+ to 5V and
      VSS and LED- to GND, captures the symbol view (with the simulation's
      frame and light, as the canvas passes them) at rest and running.

Acceptance: a placed LCD shows a blank face at rest and a lit, empty face
while running with power and the backlight wired.
  Check: build; place `NHD-0216K1Z-NSW-BBW-L`, wire VDD and LED+ to `5V` and VSS and LED- to `GND`, run, then screenshot the blue face (est. 5 min).
  Ran: app build → `** BUILD SUCCEEDED **`; `run face` → layout 330 × 270,
  face 249 × 56.5 (the §4.2 numbers); running: backlight `[1.0]`, frame
  16x2 with 1280 dots, 0 on, 0 X (display off at power-up); after Stop the
  frame is nil. `/tmp/p11-demo/face-rest.png` (dark navy, unlit cells) and
  `face-running.png` (lit blue, empty cells) were looked at.
Commit: 8fa1fa1

### Phase 3 — PCB Style and the file

- [x] `Canvas/BoardSettings.swift`: `FootprintStyle` (§4.4).
- [x] `Canvas/BoardModel.swift`: `BoardPlacement.style`,
      `PlacedChip.footprint`, the 5 sites (§4.4).
- [x] `Canvas/SchematicModel.swift`: sites `:811,823,837`; `setFootprintStyle` (§4.4).
      (Now at `:862,874,888`; the 13-site count re-measured: 13. No re-snap
      after the switch, see Corrections.)
- [x] `Document/SchematicFile.swift`: `Placement.style`, write and read,
      site `:340` (§4.5).
- [x] `Canvas/PCBSidebarView.swift`: header and Style picker (§4.6).
- [x] `docs/file-format.md` (§4.5).
- [x] Harness mode `pcb`: the module with the Pad 1 Center origin, the
      sidebar and the PCB page captured in each style, save, reopen, a bad
      value, and a new file's keys.

Acceptance: switching the style swaps the footprint with pin 1's pad
fixed, and the choice round-trips through a file.
  Check: build; on the PCB page, select an LCD and note pad 1's X/Y with
  the Pad 1 Center origin; switch to Connector Only; X/Y unchanged, the
  1×16 header is drawn, the header line reads `HDR-1X16-P2.54`. Save as
  `/tmp/p11-demo/style.schematical`; `grep -c '"style" : "connector"\|"style":"connector"' /tmp/p11-demo/style.schematical` → 1. Reopen: still Connector Only. Edit the file to `"style":"x"` and open: the error `U… style "x" must be module or connector` (est. 8 min).
  Ran: build → `** BUILD SUCCEEDED **`; `run pcb` → before `style module,
  footprint LCM-NHD-0216K1Z, origin pad1Center at (-32.005, -15.780), pads
  36`; after `style connector, footprint HDR-1X16-P2.54 … at (-32.005,
  -15.780), pads 16`, `pad 1 moved by 0.000000 mm`; grep → 1 (`"style" :
  "connector"`); reopened → `style connector … (-32.005, -15.780)`; bad
  value → `The PCB settings are not valid: U1 style "x" must be module or
  connector.`; a new file with a gate and an LCD has 1 `style` key (the
  LCD's). PNGs looked at: `pcb-module.png` (both 1×16 rows, 4 holes),
  `pcb-connector.png` (the 1×16 header, square pin 1),
  `pcb-sidebar-connector.png` (Style on Connector Only, header line
  `HDR-1X16-P2.54 — …`, X −32.01, Y −15.78).
Commit: fbe7705

### Phase 4 — Runtime proof

- [x] `/tmp/p11-demo/make_board.py` and `hello.bin` (§4.7; not committed).
      Ran: `python3 -I make_board.py backend/tests/golden/parts.txt` → 9 parts,
      63 wires, 33 nets; hello.bin 32768 bytes (the corrected program of
      plan-11-C §4.6). Harness `run netlist`: the app's netlist equals the
      intended nets (`diff` → same).
- [x] Open `lcd-6502.schematical`, run, open the reset switch, screenshot
      `HELLO` on the face.
      Ran (`run board`, speed 1000): opened 9 parts / 63 wires, the LCD
      `style connector`; reset held until 101 ms of circuit time, then
      opened; at 173 ms the CPU is `Running · 72339 cycles`, last
      `8029  4C 29 80  JMP $8029`, backlight 1.0, and the dot dump spells
      HELLO. `/tmp/p11-demo/hello.png` looked at: HELLO in light dots on
      the lit blue face. The first run failed (every dot X, CPU LOST); the
      cause and fix are in plan-11-B and C Corrections (62adf2d).
- [x] On the PCB page: the LCD is a 1×16 header. Switch to Full Module and
      screenshot the 80 × 36 module with both header rows and 4 holes.
      Move, rotate and change the origin once each; switch back; save,
      reopen and screenshot again.
      Ran: header `HDR-1X16-P2.54` at open; Full Module →
      `LCM-NHD-0216K1Z`, 36 pads (`board-module.png`: both rows, 4 holes);
      moved (+10, +5 mm), rotated to 90°, origin → Pad 1 Center; back to
      Connector Only with `pad 1 kept: true`; saved and reopened →
      identical placement (`same placement after reopen: true`,
      `board-reopened.png` the turned header, sidebar captured).

Acceptance: `HELLO` on the face; both footprint styles drawn; the style
survives move, rotate, origin change, save and reopen.
  Check: the screenshots above, attached to the commit message's description (est. 15 min).
  Ran: as the three tasks above; the PNG paths are in the Phase 4 commit
  message.
Commit: —

## Validation Plan

- Tests: no app test target. Each phase has a screenshot check of the
  behavior it adds. The backend suite stays 18 of 18 with goldens
  unchanged.
- Coverage check: Phase 4 drives every changed site:
  - draw, move, rotate, origin, the Style switch, save and reopen use the
    13 footprint sites;
  - running uses the snapshot, frame and view;
  - the drawer and sidebar use the catalog.
- Runtime proof: Phase 4 (`HELLO` from a CPU program, in the app).
- Doc sync: `docs/file-format.md` (§4.5).
- Final gate (once, after Phase 4): `cmake --build /tmp/logic-p11 -j8 &&
  ctest --test-dir /tmp/logic-p11` → 18 of 18;
  `git diff --exit-code backend/tests/golden` → 0; the app build →
  `** BUILD SUCCEEDED **` (est. 6 min).

## Decisions

- Displays → Character, parts listed with no function level (owner asked
  for the path; author for the listing). (§4.1)
- The Style option is Full Module (default) or Connector Only, the second a
  plain 1×16 header; it shows for parts with a connector footprint (owner,
  2026-11-01). (§4.4, §4.6)
- New parts and files without the key start as Full Module (owner,
  2026-11-01). (§4.5)
- A style switch keeps pin 1's first pad fixed (author). (§4.4)
- No RAM window (owner, 2026-11-01). 7-segs stay under Indicators (owner,
  2026-11-01).
- LCD parts are found by kind, not by probing the circuit (author;
  `circuit_error`). (§3)

## Corrections

- **The §4.7 flow found a backend bug, fixed in B and C (62adf2d).** With
  the reset switch opened by hand after a second (here at 101 ms), the
  first run showed every dot X and the CPU LOST: the 6502 held in reset
  pulses E X past the LCD's power-on. The fix (E X with R/W 1 is a
  possible read; the program first sets AC) and its evidence are in the
  plan-11-B and plan-11-C Corrections, as § Non-goals asks.
- **§4.7 program.** `hello.bin` holds the corrected plan-11-C §4.6
  program (JMP `$8029`, NMI/IRQ `8029`).
- **No re-snap after a style switch (§4.4 step 4 dropped).** `snapped`
  moves the placement so its origin point is on the 0.635 mm grid. With
  the Pad 1 Center origin chosen by hand (an origin change never moves a
  footprint, so pad 1 is usually off the grid), the snap moved pad 1 by
  (0.255, −0.095) mm, and the Phase 3 check ("X/Y unchanged") failed.
  Measured with `run pcb`. `setFootprintStyle` now keeps pin 1's first
  pad exactly where it was, which is what §4.4 and § Decisions ask for
  ("pin 1 staying put"). Like an origin change, this may leave the origin
  point off the grid. The next move snaps it, as before.

- **`lcdParts` is found in `NetlistBuilder`, not `Simulation`.** §4.3 has
  `Simulation` filter `netlist.chipIDs` by kind when it starts, but
  `start(netlist:loads:)` receives only the netlist (no chips to read the
  kind from). `SchematicNetlist` gains `lcdParts`, filled from the chips by
  `part.kind == PART_CHARLCD` when the netlist is built. The rule (by kind,
  never probing the circuit) is unchanged.

- **Prerequisites, plan-11-C row.** Same flaw as plan-11-C's row for B:
  the plain `grep -c 'Commit: —'` counts the row's own text. Measured with
  `grep -c '^Commit: —'`.
- **Screenshots come from the harness, not a desktop session.** This
  session has no tool that drives a desktop GUI (clicking the drawer,
  pressing Run, flipping a switch on the canvas). As in plan-09-E, the
  harness (`/tmp/p11/harness`) compiles the app's own Swift sources with
  the backend and renders the app's real views (`ChipDrawerView`,
  `SchematicSidebarView`, the canvas, the PCB sidebar) offscreen, driving
  the same document and simulation objects the app uses. Clicks are
  replaced by the methods they call.

## Summary

The risk is a missed footprint site after a style switch, and Phase 4
walks every site. The face only paints what the backend renders, so the
dot logic stays in C under `charlcd_test`. The file format grows one
optional key, written only for LCDs.
