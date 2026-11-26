# plan-82-G: App: multi-die LED symbol, per-die lights, colours, facts, Part Editor, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-82-F

G makes the app show what F loads: a multi-die LED is drawn as its dies (two or three diode
triangles joined at the common pin, or two anti-parallel for a bidirectional pair), each die glows
in its own colour from its own light; a part with a fixed colour has no colour picker; the
sidebar shows the LED facts; the Part Editor edits `dies` and the fact keys. Fixture devices only:
H and I ship the parts.

References: plan-82-F § 3 (the keys, `PartLed`); `schematical/schematical/Canvas/DiodeSymbolView.swift`
(`lightColor(named:)` :50, `lightName(for:)` :66, `paint` :82); `Canvas/SchematicCanvasView.swift:1269`
(`level: simulation.lights[chip.id]?.first`); `Canvas/SchematicExportScene.swift:63`;
`Backend/PartCatalog.swift` (`case led(colors:)` :698, :1199, :1259, the builder :1522, `symbol(for:)`
:2031); `Backend/PartFacts.swift:385`; `Drawer/PartOptionsSection.swift:114` (picker rule);
`Canvas/SchematicModel.swift:1180` (default colour on placement); `Document/SchematicFile.swift:1106`
(colour validation); `BoardPreview/PartLook.swift:108`; `Editor/LedDraft.swift`,
`scripts/part-editor-schema.py`; `scripts/harness/generic-led-ui.swift` (harness precedent);
`planning/plan-74-J-ethernet-app.md` (an app letter's precedent).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-F complete | `ls planning/completed/plan-82-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p82-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at planning for plans 69–74 (2026-11-25 at 63e2f7d2); G re-runs it at its start |

## 1. Goal

- A placed common-anode RGB fixture draws three diodes sharing the anode lead; with A at 1 and
  only K2 at 0, only the middle (green) die glows.
- A bidirectional fixture draws two anti-parallel diodes; each glows for its own polarity.
- A single-die part with `"colors": ["Red"]` shows no colour picker and glows red;
  `GENERIC-LED` keeps its five-colour picker.
- The sidebar lists Wavelength, Luminous intensity, Forward voltage (at If), Viewing angle and Lens
  when given.
- The Part Editor round-trips a `dies` device with facts.

### Non-goals

- The flipped-LED arrow look (todo.md:56) is not changed.
- KiCad and EasyEDA exports draw a multi-die LED as the body box with its pins, the shape every
  multi-pin symbol uses there (`KiCadSchematicWriter.bodyBox` default); single-die LEDs export as
  today.
- No change to `GENERIC-LED`'s symbol, placement or file format.

## 2. Current State

`.led(colors:)` is the LED's `Details`; `symbol(for:)` gives `.diode`; `DiodeSymbolView` draws one
diode and glows from `level` = the first light; `lightColor(named:)` knows nine names (no
Yellow-green, Violet, Pink). The picker shows when `colors.count > 1`. `SchematicModel` sets the
placed colour to the first colour.

## 3. Design

- `PartCatalog`: `Details.led(colors:dies:facts:)` with `dies: [LedDie]` (`anodePins`,
  `cathodePins`, `color`) read from `PartLed`; `symbol(for:)` returns `.ledArray` when the device
  has more than one die, else `.diode`.
- `Canvas/LedArraySymbolView.swift` (new): the dies stacked 20 pt apart, each the existing diode
  glyph (`DiodeSymbolView`'s path code moved into a shared `DiodeGlyph` painter so both draw
  identically); a shared pin is one lead with a junction dot; a bidirectional pair is two glyphs
  side by side, reversed. Level per die: `simulation.lights[chip.id]?[i]`. Pin labels as in the
  datasheet (`A`, `K1`, …).
- `SchematicCanvasView` and `SchematicExportScene` place `.ledArray`.
- `lightColor(named:)` adds `Yellow-green`, `Violet`, `Pink` (the vocabulary of plan-82-C § 3; the
  others exist).
- Picker rule: `part.kind == PART_LED && part.colors.count > 1 && dies are not fixed` (a `dies`
  device's colours are fixed). `SchematicModel` stores no colour for a fixed-colour part, and
  `SchematicFile`'s validation accepts a missing colour for one.
- `PartLook` (3D board preview): a multi-die part uses the first die's colour tint.
- `PartFacts`: rows Wavelength (nm), Luminous intensity (mcd), Forward voltage (V at If mA),
  Viewing angle (°), Lens; per-die values joined by ` / `.
- Part Editor: regenerate `LedDraft.swift` with `python3 scripts/part-editor-schema.py . led`
  (after F's docs and keys), then add the `dies` table editor by hand as for other array keys.
- Tests: `schematicalTests/LedTests.swift` (new) for the picker rule, the symbol choice, the
  per-die levels and the facts; `PartKindKeysTests`, `PartKindRoundTripTests` (a `dies` fixture).
- Harness: `scripts/harness/led-dies-app.swift`: places the RGB fixture, drives K2 low, and prints
  each die's level from the canvas model.

## 4. Phases

### Phase 1: catalog, symbol, lights

- [ ] `PartCatalog`, `LedArraySymbolView`, `DiodeGlyph`, canvas and export placement, colours.

Acceptance: `xcodebuild … test -only-testing:schematicalTests/LedTests` → `** TEST SUCCEEDED **`
— ~6 min (build + one class).

Commit:

### Phase 2: picker, facts, Part Editor

- [ ] Picker rule, `SchematicModel`, `SchematicFile`, `PartFacts`, `PartLook`, `LedDraft`.

Acceptance: `xcodebuild … test -only-testing:schematicalTests/LedTests
-only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests`
→ `** TEST SUCCEEDED **` — ~8 min.

Commit:

### Phase 3: harness

- [ ] `scripts/harness/led-dies-app.swift`, built with `scripts/harness/build.sh`.

Acceptance: the harness prints `die1 0 die2 1 die3 0` for the K2-low case — 3 min.

Commit:

## 5. Validation Plan

G's gate: Phase 2's three test classes and Phase 3's harness line. The landing tier is J's.
