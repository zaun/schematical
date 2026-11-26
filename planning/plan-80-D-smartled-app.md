# plan-80-D: App: addressable LED colours, symbol, facts, drawer, Part Editor kind, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-80-C

D shows addressable LEDs in the app: the drawer leaf, the kind name and facts, a symbol whose body
is painted with each LED's current colour while the simulation runs, the Part Editor kind, the
test classes, and a harness that runs the ATmega328P chain of C on the app's real catalog.

References:

- `schematical/schematical/Simulation/SimulationEngine.swift` (`SimulationSnapshot` :54,
  `lightParts`/`lcdParts` :81–115, `snapshotNow()` :230–260), `Simulation/Simulation.swift`
  (`lightParts` found with `circuit_part_light_time(opened, $0, nil, 0) > 0` :480, `lcdFrames` :402,
  :580, :637).
- `Canvas/DiodeSymbolView.swift` (`lightColor(named:)` :50: the LED drawing), `Canvas/SchematicCanvasView.swift`
  :1269 (LED placement with `simulation.lights[chip.id]`), :1391 (`CharacterLCDSymbolView` with
  `simulation.lcdFrames[chip.id]`: the per-part frame precedent).
- The opamp sites every new kind copies (the plan-80 code survey, 2026-11-25): `Backend/PartCatalog.swift`
  (`Details` enum ~:697, builder chain ~:1522–1615, `symbol(for:)` ~:2031–2044), `Backend/PartFacts.swift:427`
  (`kindName`), `Preferences/ReferencePrefixes.swift:85` (`case PART_LED: return .led`),
  `Editor/KindInfo.swift` (:198, `infoOrder` :214–223), `Editor/KindSchema.swift` (`PartKinds.all`
  :202–211, `schemas` :218), `Editor/BundledPartFile.swift:136`, `scripts/part-editor-schema.py:44`
  (`KIND_FILES`).
- Test classes: `schematicalTests/PartKindKeysTests.swift` (:47), `PartKindRoundTripTests.swift`
  (:31, :104), `KindInfoTests.swift` (:11, :53), `PartEditorWindowTests.swift` (:14),
  `KindGuideTests.swift` (:191), `WizardStepsTests`, `DrawerPerformanceTests`.
- `scripts/harness/build.sh`, `scripts/harness/diode-app.swift` (a kind harness's shape).

## Prerequisites

plan-80-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-80-C complete | `ls planning/completed/plan-80-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |
| Shipped smartled parts exist | `grep -c '|smartled|' backend/tests/golden/parts.txt` → N_add (C § 1) | NOT MET (2026-11-25: 0) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p80-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at 686408f4 for plans 69–74 (plan-74-A § Prerequisites); D re-runs it at its start |
| No app code for the kind yet | `grep -rc 'PART_SMARTLED' schematical/schematical \| grep -v ':0' \| wc -l` → 0 | MET (2026-11-25: 0) |
| Kind counts at D's start | `grep -n 'count' schematical/schematicalTests/KindInfoTests.swift \| head -1`; `PartEditorWindowTests.swift:14` | 49 and 49 at planning; plans 69–74's app letters raise them first. D adds 1 to whatever they are then |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The drawer lists the C parts under **Displays & Indicators → Addressable LED**.
- A placed `IN-PI55TATPRPGPB` shows kind name "Addressable LED" and facts: colour order (`GRB`),
  LEDs per package, T0H / T1H / reset, supply range; reference prefix `LED` (the LED category).
- While the simulation runs, each LED of the package draws its body filled with its current colour;
  an X channel draws the body hatched grey; off draws it as an unlit LED.
- The Part Editor has the `smartled` kind with its keys; the drift test passes.
- `scripts/harness/smartled-app.swift` passes every check.

### Non-goals (explicit constraints)

- No change to how plain LEDs draw or to `simulation.lights`.
- No board-preview look of its own (`BoardPreview/PartLook.swift` draws the footprint as for any part).

## 2. Current State

The snapshot carries `lightTimes` and `lcdFrames` only (`SimulationEngine.swift:241–259`); the
simulation finds light parts once at start. `DiodeSymbolView` draws one LED with one light and a
named colour. There is no symbol with DIN/DOUT and no colour from the circuit. plan-81 (OLED) adds a
frame map to the same snapshot; the shared lines are added after whatever is there (grep first).

## 3. Design

### 3.1 Colours in the snapshot

- `SimulationSnapshot` gets `ledColors: [Int: [LEDColor]]`; `LEDColor` (Swift) holds `r g b w`
  (`UInt8`), `known` and `present` masks, built from `SmartLedColor`.
- `SimulationEngine` takes `colorParts: [Int]` (default `[]`), found once at start like `lightParts`:
  `circuit_part_colors(opened, $0, nil, 0) > 0`. `snapshotNow()` reads each into a buffer of
  `SMARTLED_MAX_LEDS`.
- `Simulation` publishes `ledColors: [UUID: [LEDColor]]` (by chip id, as `lcdFrames`), cleared on stop.

### 3.2 The symbol (`Canvas/SmartLEDSymbolView.swift`)

A rectangle with DIN on the left, DOUT on the right, VDD on top, GND below (pin positions by role,
like the other IC-style symbols), and one LED glyph per package LED (the `DiodeSymbolView` LED with
its two arrows) inside. The glyph's fill: the mix of R, G, B plus W as white at each channel's byte /
255; any unknown present channel → grey with a 45° hatch; all known and 0 → the unlit LED colour
`DiodeSymbolView` uses. `PartCatalog.symbol(for:)` returns it for `.smartLED`.

### 3.3 Catalog, facts, prefix, editor

- `PartCatalog.Details`: `case smartLED(order: String, leds: Int, timing: …)`, built from
  `part.smartled`.
- `PartFacts.kindName`: `case PART_SMARTLED: return "Addressable LED"`; fact rows: Colour order,
  LEDs, T0H, T1H, Reset, Supply.
- `ReferencePrefixes.of`: `PART_SMARTLED` → `.led`.
- Part Editor: `KindInfo` (group Displays & Indicators), `PartKinds.all`, `schemas`,
  `BundledPartFile`, `smartled` in `KIND_FILES`; `Editor/SmartledDraft.swift` generated by
  `python3 scripts/part-editor-schema.py . smartled`.

### 3.4 Rejected alternatives

- **Painting through `DiodeSymbolView` with an RGB colour argument**: it is a two-pin symbol with an
  instance colour picker; the smartled has four or more pins and no instance colour.
- **Polling colours only for parts on screen**: the snapshot already reads every light part each
  frame; colour parts are as few.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill each `Commit:` line with the hash when the phase lands.

### Phase 1: catalog, facts, drawer, prefix, editor

- [ ] `PartCatalog.swift` (`Details`, builder, `symbol(for:)`), `PartFacts.swift`,
      `ReferencePrefixes.swift`, the Part Editor files of § 3.3 — each kind list grepped first and
      the line added after the last kind present.
- [ ] Test classes: `PartKindKeysTests` (`files`), `PartKindRoundTripTests` (`testSmartled`, shipped
      count = N_add), `KindInfoTests` / `PartEditorWindowTests` counts + 1, `KindGuideTests` if the
      variant count moves, and `SmartLEDTests.swift` (details, facts, kind name of
      `IN-PI55TATPRPGPB`).

Acceptance: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical
-derivedDataPath /tmp/p80-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/SmartLEDTests
-only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests
-only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests`
→ `** TEST SUCCEEDED **`. (≈ 8 min: one build, five classes.)

Commit:

### Phase 2: colours and the symbol

- [ ] `SimulationEngine.swift`, `Simulation.swift`: `colorParts`, `ledColors` (§ 3.1).
- [ ] `Canvas/SmartLEDSymbolView.swift` (§ 3.2) and its placement in `SchematicCanvasView.swift`
      with `simulation.ledColors[chip.id]`.
- [ ] `SmartLEDTests`: `LEDColor` from `SmartLedColor` (masks, values); the symbol's fill rule of
      § 3.2 (red, RGBW white mix, X → hatch, off → unlit); a running document with one
      `IN-PI55TATPRPGPB` (VDD on 5V, GND, DIN on PULLDOWN) publishes `ledColors` with one LED, off and
      known.

Acceptance: `xcodebuild test … -only-testing:schematicalTests/SmartLEDTests` → `** TEST SUCCEEDED **`.
(≈ 6 min.)

Commit:

### Phase 3: harness

- [ ] `scripts/harness/smartled-app.swift`: the drawer leaf lists N_add parts; kind name and facts of
      `IN-PI55TATPRPGPB`; the C chain on the real catalog (ATMEGA328P-PU, GENERIC-CRYSTAL-16M, three
      IN-PI55TATPRPGPB) with `backend/tests/avr/ws2812.hex` set as the MCU's file
      (`doc.setMemoryFile`, as `scripts/harness/avr-app.swift` does) shows red, green, blue in
      `simulation.ledColors` after 2 ms of play; renders the symbol (unlit and lit) to
      `p80-symbols.png`.

Acceptance: `rm -rf /tmp/p80-h && APP=/tmp/p80-dd/Build/Products/Debug/schematical.app
scripts/harness/build.sh scripts/harness/smartled-app.swift /tmp/p80-h &&
/tmp/p80-h/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; `p80-symbols.png` shows three
lit bodies red, green, blue (looked at). (≈ 4 min.)

Commit:

## Validation Plan

The Phase 1 and Phase 2 acceptances re-run on the final tree, and the harness once. The full app
suite is not a D step (27 min; it runs in E's landing tier only through the targeted classes).

## Corrections
