# plan-35-G: 20x4 / 40x2 / 40x4 LCDs — the app

Last updated: 2026-11-11
Effort: medium (1h–2h)
Depends on: plan-35-F

G makes the app draw and describe the new modules:

- the `gray` and `white` panels get face colors;
- a 40 × 4 symbol shows RS, R/W, E1, E2 above the data pins;
- an RGB module's face takes the color of its lit LEDs;
- the facts and drawer labels name the new panels and multi-LED backlights.

Four-row faces and the `16x2`…`40x4` drawer leaves need no app code: the
face size comes from `columns`/`rows`, `LCDFrame` copies up to
`LCD_STATE_MAX_ROWS`, and the drawer is the part file's path. A new test
file covers what G adds.

Outcome of G: the app builds; `CharacterLCDTests` and the existing
`SchematicSVGTests` pass; a placed NHD-0440WH-ATFH-JT# shows E1 and E2
on its left side.

References:

- plan-35-A § Prerequisites; plan-35-D § 4.1 (the new `PartCharLcd`
  fields), § 4.5 (`LcdState`).
- `planning/completed/plan-11-D-charlcd-app.md` (the symbol, palette and
  facts this extends).

## Prerequisites

The gate is plan-35-A § Prerequisites. G adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-F complete | `grep -c '^Commit: —' planning/plan-35-F-charlcd-large-parts-focus.md` → 0 (or in `planning/completed/`) | MET (2026-11-11, F's last hash recorded in this commit: 0) |
| `PartCharLcd.enables` exists | `grep -n 'unsigned enables;' backend/include/parts.h` → one line | MET (2026-11-11: parts.h:98) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p35-dd CODE_SIGNING_ALLOWED=NO build`
  → `** BUILD SUCCEEDED **`.
- `xcodebuild … test -only-testing:schematicalTests/CharacterLCDTests -only-testing:schematicalTests/SchematicSVGTests CODE_SIGNING_ALLOWED=NO`
  → `** TEST SUCCEEDED **`.

### Non-goals (explicit constraints)

- The 16 × 2 symbols, faces and facts look exactly as before (same
  palette, same pin column, same facts rows).
- No change to `kindName` (`PartFacts.swift:155` already returns
  "Character LCD"); this step is moot:
  `grep -c 'case PART_CHARLCD: return "Character LCD"' schematical/schematical/Backend/PartFacts.swift` → 1.
- No new window, and no drawer code (the leaves come from the part files).

## 2. Current State

- **`LCDPanel`** (`Backend/PartCatalog.swift:111-123`) has `.yellowGreen`
  and `.blueNegative`; `title` "Yellow-green" / "Blue negative".
- **Catalog mapping** (`PartCatalog.swift:645-654`): `panel` is
  `blue-negative` → `.blueNegative`, anything else → `.yellowGreen`.
  `CharacterLCDInfo` (line 126) holds `columns, rows, controller, rom,
  panel, backlightColor, busPins`.
- **Facts** (`Backend/PartFacts.swift:81-86`): "Panel" is "STN blue,
  negative" or "STN yellow-green, positive"; "Backlight" is the color or
  "None".
- **Drawer second line** (`PartCatalog.swift:483-484`): panel title, plus
  "<color> backlight".
- **Layout** (`Canvas/SchematicModel.swift:268-293`): the left column is
  `stack([bus.prefix(3), bus.dropFirst(3)])` (RS, R/W, E, a gap, DB0–7);
  every other pin goes on the right in pin order.
- **Palette** (`Canvas/CharacterLCDSymbolView.swift`, `palette(_:backlight:)`):
  a switch over the two panels; `backlight` is one level 0…1.
- **Canvas** (`Canvas/SchematicCanvasView.swift:370`): passes
  `simulation.lights[chip.id]?.first ?? 0`.
- **Export** (`Canvas/SchematicExportScene.swift:67-69`) passes
  `frame: nil, backlight: 0`.
- **Tests:** no test file is about character LCDs
  (`ls schematical/schematicalTests | grep -i lcd` → nothing);
  `SchematicSVGTests.swift` exports a scene with one.

## 3. Design

- **`LCDPanel`** gains `.gray` (title "Gray") and `.white` (title "White").
  The catalog maps all four strings; facts read "STN gray, positive" and
  "FSTN white, positive".
- **`CharacterLCDInfo`** gains `enables: Int` and `backlightColors:
  [String]` (from `PartCharLcd.backlight_colors`). `backlightColor`
  becomes `backlightColors.first`.
  - Facts "Backlight": the colors joined with ", " ("Red, Green, Blue"),
    or "None".
  - Drawer: "<color> backlight" for one LED, "RGB backlight" for three.
- **Layout:** the left column is
  `stack([bus.prefix(2 + enables), bus.dropFirst(2 + enables)])`, so a
  40 × 4 reads RS, R/W, E1, E2, a gap, DB0–7. For one-enable parts this is
  `prefix(3)` as today.
- **Palette:**
  - `.gray`: background `0x7E817A` → `0xC4C8BE` with the backlight; dots
    `0x20241C`.
  - `.white`: background `0x8E9296` → `0xEEF1F3`; dots `0x111417`.
- **The symbol takes `backlights: [Double]`** (every light's level) in
  place of `backlight: Double`:
  - one LED: the panel palette at `backlights[0]`, as today;
  - several LEDs: each LED's level times its color (Red `0xFF3B30`,
    Green `0x34C759`, Blue `0x0A84FF`), added per channel and clamped to
    1. The lit background is the panel's lit background multiplied by
    that sum, then mixed from the dark background by the largest level.
  - The canvas passes `simulation.lights[chip.id] ?? []`; the export
    passes `[]`.
  - A color name with no entry in the table above draws as the panel's
    own lit color. Today's single-LED colors (White, Yellow-green) never
    reach the table, because one-LED faces use the panel palette.

**Correctness gate:**

- **app tests**, which exercise the catalog mapping, the layout and the
  palette function directly;
- **one look in the real app**: a 20 × 4, a 40 × 4 and an RGB module
  placed, with a screenshot in the commit message. Text on the faces is
  the backend's job and is tested there (plan-35-E's boards). Per
  AGENTS.md, the mouse goes back to (0, 0) afterwards.

**Rejected alternative:** *tinting single-LED faces by their backlight
color.* Rejected: it would change how the five 16 × 2 faces look
(non-goal), and the panel palettes already model the usual
glass/backlight pairs.

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — catalog, facts, layout, palette

- [x] `PartCatalog.swift`: `LCDPanel` cases, the mapping, the
      `CharacterLCDInfo` fields, and the drawer label. — `LCDPanel(_:)`
      maps the four strings; `backlightColors`, `enables`, and
      `backlightTitle` ("RGB" for Red, Green, Blue) for the drawer.
- [x] `PartFacts.swift`: the panel and backlight rows.
- [x] `SchematicModel.swift`: the `2 + enables` left column.
- [x] `CharacterLCDSymbolView.swift`: the two palettes and `backlights:
      [Double]` with the RGB mix; `SchematicCanvasView.swift:370` and
      `SchematicExportScene.swift:69` callers. — the mix is
      `faceColors(_:backlights:)` (internal, for the tests).
- [x] `schematical/schematicalTests/CharacterLCDTests.swift`:
  - (a) NHD-0440WH-ATFH-JT# from the real catalog: `enables` 2, 12 bus
    pins, left slots in order RS, R/W, E1, E2, then DB0 after one empty
    row;
  - (b) NHD-0216K1Z-NSW-BBW-L's left slots unchanged (RS, R/W, E, gap,
    DB0);
  - (c) the panels of C204CXBSGSW6WT33XAA (`.gray`) and
    NHD-0420DZ-FSW-FBW (`.white`) and their facts strings;
  - (d) MD42004A6W-FPTLRGB: `backlightColors` = Red, Green, Blue, and its
    face background with levels [0, 1, 0] has g > r and g > b;
  - (e) an `LCDFrame` built from an `LcdState` with `rows` 4 holds
    4 × 8 × `columns` × 5 dots.
  - (plus) a one-LED face keeps the panel palette.

Check: the two `xcodebuild` commands of § 1 (est. 8 min: an app build
plus two test classes; no smaller check compiles the Swift). — 2026-11-11:
`** BUILD SUCCEEDED **`; test: CharacterLCDTests 6 tests, SchematicSVGTests
6 tests, "Executed 12 tests, with 0 failures", `** TEST SUCCEEDED **`.

Commit: 69bef71

### Phase 2 — the real app

- [x] Launch the app (`run` skill) and place NHD-0420DZ-FL-YBW,
      NHD-0440WH-ATFH-JT# and MD44005A6W-FPTLRGB on a new schematic. Check:
      - the 20 × 4 face has 4 rows;
      - the 40 × 4 shows E1 and E2 on the left;
      - in play mode, with VCC on the RGB module's LED+ and GND on K(G)
        only, its face turns green.
      Take one screenshot, then move the pointer to (0, 0). — 2026-11-11,
      Debug build of 69bef71 (`/tmp/p35-dd`): a schematic with the three
      modules plus 5V on MD44005's LED+ and GND on its K(G) (written by a
      throwaway test with `TestSupport.place` / `doc.connect`, opened with
      File › Open). Screenshot `planning/plan-35-data/g-app-check.png`:
      the 20 × 4 face has 4 rows; NHD-0440WH's left column reads RS, R/W,
      E1, E2, gap, DB0…DB7; in play mode the RGB face is green while the
      other two stay unlit. Pointer moved to (0, 0); the app instance quit.

Check: the screenshot shows the three faces as described (est. 10 min;
a visual result can only be checked by looking).

Commit: 216d5d8

## Validation Plan

Per-phase checks. The landing tier is H.

## Decisions

- **RGB faces mix lit LEDs; single-LED faces keep the panel palette**
  (author, § 3 rejected alternative).
- **Panel titles "Gray" and "White"** (author): they match the part-file
  names and the drawer's short second line.

## Corrections

- **Phase 2's schematic was built by a throwaway test, not by hand**
  (2026-11-11): the three parts, a 5V and a GND supply and two wires were
  placed with `TestSupport.place` / `doc.connect` and saved with
  `SchematicFile`, then opened in the real app. The test file was deleted
  afterwards (not committed); the check itself (what the app shows) is as
  planned.
- **The commit carries the screenshot as a file**
  (`planning/plan-35-data/g-app-check.png`), since a commit message cannot
  hold an image.

## Summary

Small, contained UI work. The 16 × 2 non-goal is protected by test (b) and
by unchanged palette code for the two existing panels.
