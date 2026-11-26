# plan-81-F: App: OLED state polling, `OLEDSymbolView`, facts, drawer, Part Editor kind, tests, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-81-E

F shows the new parts in the app: an `oled` part is drawn as its glass with the simulated pixels in
the panel colour; a facts-only `oled` part turns Run off as an unsimulated `mcu` does; a character
OLED in `charlcd` draws lit dots in its OLED colour on black. The drawer lists both under
**Displays & Indicators → OLED → Graphic / Character**, and the Part Editor can edit `oled` parts.

References:

- `schematical/schematical/Simulation/SimulationEngine.swift` (`LCDFrame` :17, `lcdParts` :82,
  the snapshot loop `circuit_lcd_state` :247-252), `Simulation/Simulation.swift` (`lcdFrames`
  :402, :580, :636).
- `schematical/schematical/Canvas/CharacterLCDSymbolView.swift` (229 lines: `LCDFrame` drawing,
  the precedent for `OLEDSymbolView`); `Canvas/SchematicCanvasView.swift` (where symbols are placed).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details` enum ≈ :697, builder chain
  ≈ :1522–1615, `symbol(for:)` ≈ :2031–2044, the charlcd panel enum :465-495),
  `Backend/PartFacts.swift` (`kindName`, :393 `Character LCD`, :427 `Op-amp`),
  `Preferences/ReferencePrefixes.swift` (:75 `ReferenceCategory.of`).
- Part Editor: `Editor/KindInfo.swift` (:127 charlcd's `KindInfo`, `infoOrder` :214-223),
  `Editor/KindSchema.swift` (`PartKinds.all` :202-211, `schemas` :218),
  `Editor/BundledPartFile.swift:136`, `scripts/part-editor-schema.py` (`KIND_FILES` :44),
  `Editor/CharLcdDraft.swift:30` (the panel field).
- Test classes in `schematicalTests/`: `PartKindKeysTests`, `PartKindRoundTripTests`,
  `KindInfoTests`, `PartEditorWindowTests`, `KindGuideTests`, `WizardStepsTests`,
  `DrawerPerformanceTests`; precedents `TvsTests`, `EthernetTests`, `IoExpanderTests` (plans 69–74).
- `scripts/harness/build.sh` and its `*-app.swift` harnesses (e.g. `avr-app.swift`).
- AGENTS.md § Testing in the app (pointer to (0, 0) when done); memory notes on GUI probing.

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-81-E complete | `ls planning/completed/plan-81-E-*.md` → exists | NOT MET (2026-11-25) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p81-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at planning (2026-11-25 at 686408f4, run by the planner: `** BUILD SUCCEEDED **`) |
| Kind counts at F's start | `grep -n 'XCTAssertEqual(.*count' schematical/schematicalTests/KindInfoTests.swift schematical/schematicalTests/PartEditorWindowTests.swift schematical/schematicalTests/KindGuideTests.swift` | at planning: `KindInfoTests` 49 (:11), `PartEditorWindowTests` 49 (:14), `KindGuideTests` 75 (:191). Plans 69–80 may raise them first; F adds 1 kind to what it finds |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- In play mode, a placed simulated `oled` part shows `circuit_oled_state`'s pixels: ON in the panel
  colour (per zone on a two-colour glass) at an alpha of 0.35 + 0.65 × contrast / 255, OFF black,
  X a grey hatch (the charlcd X look); a `fault` is shown as the symbol's tooltip. Outside play
  mode the glass is black.
- A placed facts-only `oled` part disables Simulation › Run, as an unsimulated `mcu` does.
- A `charlcd` part with an `oled-*` panel draws lit dots in that colour on black and no backlight.
- The inspector shows kind name "OLED display" and facts: controller, format and size, colour,
  depth, interfaces, simulated or not.
- The drawer lists the parts under Displays & Indicators → OLED → Graphic and → Character.
- The Part Editor lists the `oled` kind and round-trips every shipped `oled` part.

### Non-goals (explicit constraints)

- No backend change (B–E). No new preference.
- No pixel editing or pixel-level hit testing on the symbol.

## 2. Current State

- `SimulationEngine.snapshotNow()` polls `circuit_lcd_state` for `lcdParts` (:247-252) and
  `Simulation` keeps `lcdFrames` by part UUID; `CharacterLCDSymbolView` draws an `LCDFrame` with
  the panel colours of `PartCatalog`'s charlcd panel enum (4 LCD cases, :465-495).
- `PartCatalog` maps kinds to `Details`; there is no `oled` case. `ReferenceCategory` has
  `.display`-style categories for the charlcd and 7-segment kinds (`ReferencePrefixes.swift`).

## 3. Design

### 3.1 State

`OLEDFrame` (in `SimulationEngine.swift`, next to `LCDFrame`): `width`, `height`, `displayOn`,
`contrast`, `inverse`, `fault`, `pixels: [UInt8]` (row-major `OLED_PIXEL_*`). `SimulationEngine`
takes `oledParts: [Int]` (the parts whose catalog details are `.oled(simulated: true)`) and polls
`circuit_oled_state` in the same snapshot as the LCDs; `Simulation` keeps `oledFrames: [UUID:
OLEDFrame]` and clears it where it clears `lcdFrames` (:580).

### 3.2 Symbol

`Canvas/OLEDSymbolView.swift`: a module outline with the glass area at the module's aspect; the
glass is a `Canvas` of width × height cells, each the pixel's colour (§ 1). Pins on the outline's
edge in pinout order (the charlcd symbol's pin placement). The panel colours: white
`#E8F0FF`, yellow `#FFD200`, green `#30FF60`, blue `#40A0FF`, red `#FF4030`, rgb drawn white (a colour
module's facts-only glass shows only its outline and "facts only" text, since it never runs).

### 3.3 charlcd OLED panels

`PartCatalog`'s charlcd panel enum gains `oledWhite`, `oledYellow`, `oledGreen`, `oledBlue`,
`oledRed` (names "OLED white" …, descriptions "OLED, emissive"); `CharacterLCDSymbolView` draws
them as lit dots in the § 3.2 colour on black, unlit dots at 6 % of it, and no backlight glow.
`CharLcdDraft`'s panel picker lists them.

### 3.4 Catalog, facts, drawer, editor

- `PartCatalog.Details.oled(…)` with the facts; builder case for `PART_OLED`; `symbol(for:)` → `OLEDSymbolView`.
- `PartFacts.kindName` → "OLED display"; fact rows controller, format (`128 × 32 pixels` / `20 × 4
  characters`), colour, depth (`1 bit` …), interfaces, "Simulated: yes/no".
- `ReferenceCategory.of` (`ReferencePrefixes.swift:86`): `PART_OLED` joins `PART_7SEG, PART_CHARLCD, …` → `.display` (prefix `DS`).
- Drawer: the parts' `drawer` arrays put them under Displays & Indicators → OLED (E); the drawer
  needs no code, and `DrawerPerformanceTests` re-measures with the new parts.
- Part Editor: `python3 scripts/part-editor-schema.py . oled` after adding `oled` to `KIND_FILES`
  generates `Editor/OledDraft.swift`; `KindInfo` (group Displays, after charlcd in `infoOrder`),
  `PartKinds.all`, `schemas`, `BundledPartFile`; the `model` key's values in
  `Editor/Wizard/Guides/VariantBlurbs.swift` (one blurb per profile).

### 3.5 Tests and harness

- `schematicalTests/OledTests.swift`: catalog details of a simulated and a facts part; `OLEDFrame`
  from a hand-built `OledState`; the colour and alpha rule; a facts part disables Run; a charlcd
  `oled-yellow` part's panel.
- The count bumps in `KindInfoTests`, `PartEditorWindowTests`, `KindGuideTests`;
  `PartKindKeysTests.files` gains `oled.json`; `PartKindRoundTripTests.testOled`.
- `scripts/harness/oled-app.swift`: opens a schematic with `NHD-2.23-12832UCB3` wired to an
  ATmega328P running `backend/tests/avr/oled_i2c.hex` (E), runs 50 ms, and writes the canvas to a
  PNG; built by `scripts/harness/build.sh scripts/harness/oled-app.swift` (usage at `build.sh:13`).

## 4. Phases

### Phase 1 — catalog, facts, editor, tests

- [ ] § 3.4 (catalog, facts, reference category, editor files, generated draft).
- [ ] § 3.5's test classes and count bumps (grep each count first; add 1 to what is there).

**Acceptance:** `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical
-derivedDataPath /tmp/p81-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/OledTests
-only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests
-only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests
-only-testing:schematicalTests/KindGuideTests` → `** TEST SUCCEEDED **`. (≈ 6 min: build + these classes.)

Commit:

### Phase 2 — state, symbol, charlcd panels

- [ ] § 3.1 (`OLEDFrame`, polling, `Simulation.oledFrames`).
- [ ] § 3.2 `OLEDSymbolView`, wired in `symbol(for:)` and the canvas.
- [ ] § 3.3 the charlcd OLED panels.
- [ ] `OledTests`: frame polling against a live `Circuit` with a fixture part.

**Acceptance:** the Phase 1 test command → `** TEST SUCCEEDED **`, with `OledTests` now holding
the charlcd OLED-panel cases too (no app test class covers charlcd panels today: `grep -l -i
'charlcd' schematical/schematicalTests/*.swift` → only `PartKindKeysTests.swift` and
`PartKindRoundTripTests.swift`, 2026-11-25). (≈ 6 min.)

Commit:

### Phase 3 — harness and window pass

- [ ] `scripts/harness/oled-app.swift` (§ 3.5); run it; check the PNG shows the checkerboard on the glass.
- [ ] One pass in the real app window: place the Goal's three parts from the drawer, run, check
      the checkerboard, the facts part's disabled Run, the yellow character OLED; move the pointer
      to (0, 0) when done (AGENTS.md).

**Acceptance:** `APP=/tmp/p81-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/oled-app.swift /tmp/p81-harness &&
/tmp/p81-harness/H.app/Contents/MacOS/harness /tmp/p81-oled.png` → exit 0, the harness prints
`checkerboard ok` (it compares the rendered glass cells with the 8 × 8 checkerboard), and the PNG
shows it.
(≈ 3 min.)

Commit:

## 5. Validation Plan

End of F: `xcodebuild … build` → `** BUILD SUCCEEDED **` and the Phase 1 and 2 test commands →
`** TEST SUCCEEDED **`; the harness prints `checkerboard ok`. The full app suite is the landing
tier's (G).
