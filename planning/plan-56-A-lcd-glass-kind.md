# plan-56-A: LCD glass kind (`lcd`): backend kind, model, bundled glass parts, app symbol

Last updated: 2026-11-23
Overall Effort: huge (> 3d) — plan-56 is A → B → C → D → E → F → G
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-56 adds the LED and LCD **display driver ICs** whose datasheets were filed in `datasheets/led-drivers/` and
`datasheets/lcd-drivers/` (30 PDFs; `ls datasheets/led-drivers datasheets/lcd-drivers | grep -c pdf` → 30) as real,
simulated parts, in the backend and the app. The LCD drivers drive glass, and the library has no segment-LCD glass part (only
HD44780 modules, kind `charlcd`), so sub-plan A adds the glass: a new kind `lcd` (owner decision, § Decisions) whose parts read
a driver's segment and backplane pins, plus bundled glass parts so a circuit with an LCD driver shows a working display.

Single behavioral outcome of A: with a Lumex LCD-S401C39TR's COM pin held at 0 and segment pin 1A driven to 1, the glass reports
segment A of digit 1 lit; driven to 0 it reports it off; a multiplexed generic glass lights a cell only while its backplane is
driven and the segment pin differs from it; the app draws the glass as lit 7-segment digits.

References: `AGENTS.md`; `docs/backend.md` § Adding a kind; `docs/parts-format.md` § `7seg` (the lit-segment rules this kind copies);
`backend/src/models/display7.c`; `datasheets/` PDFs from Lumex (below).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| the BCD decoders commit is on main | `git log --oneline \| grep -c "4511B, 4543B, 4054B"` → 1 | MET |
| the display-driver datasheets are filed | `ls datasheets/led-drivers datasheets/lcd-drivers \| grep -c pdf` → 30 | MET |
| the backend builds and its tests pass | `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → 100% | MET |

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command before you continue.

## 1. Goal

- `"kind": "lcd"`: `lcd.json`, `backend/src/models/lcd.c`, `PART_LCD`; loads, lists, simulates, draws.
- Parts: Lumex LCD-S301C31TR (3 digits), LCD-S401C39TR (4 digits, colon, 3 DPs) and LCD-S3X1C50TR/B (3.5 digits, LB, K, 3 DPs), the three
  static (one backplane) 40-pin glasses found on DigiKey in Safari on 2026-11-23 (the Midas MC4DIG+ICON12W-TPT is also static: not
  added); and four bundled **generic multiplexed glasses** (Generic manufacturer, no datasheet) for the drivers: `GENERIC-LCD-4D-1:2`
  (4 digits, 2 backplanes), `GENERIC-LCD-8D-1:3` (8 digits, 3 backplanes), `GENERIC-LCD-8D-1:4` (8 digits, 4 backplanes),
  `GENERIC-LCD-4D-STATIC` (4 digits, 1 backplane).

### Non-goals

- No analog waveform, bias level, contrast or DC-damage model: the engine is digital. Glass never drives a pin.
- `7seg.json`, the `7seg` kind and every existing golden line are untouched.
- No dot-matrix or icon glass; cells are 7-segment digit segments (A–G, DP), colons and named annunciators.

## 2. Current State

- `7seg` kind: `backend/src/models/display7.c`, section `### 7seg` of `docs/parts-format.md`; an LED is on while its anode reads 1 and
  its cathode 0; lights report "on at any time during the last step" (`lights.c`). The app's symbol is
  `Canvas/SevenSegmentSymbolView.swift`, fed by `PartDisplay` (digits, `leds`, `segmentPins`, `commonPins`).
- Adding a kind is a fixed recipe: `docs/backend.md` § Adding a kind (parts.h, model file, `kinds.c` line, part file, app symbol,
  Part Editor: `KindInfo`, `PartKinds.all`, `part-editor-schema.py` `KIND_FILES`, `PartKindKeysTests.files`, `<Kind>Draft.swift`,
  `KindSchema`, `PartKindRoundTripTests`).
- Precedent for the same pattern in this repo: commits b2e6ff8a (dekatron backend kind) and bbea2174 (its app half).
- Lumex drawings (read 2026-11-23 from `datasheets/lcd-glass/*.pdf`; all three say DRIVING METHOD: STATIC, 5.0 V AC, TN reflective):
  - LCD-S401C39TR, 40 pins, COM on pins 1 and 40: 1 COM, 2-4 N/C, 5 1E, 6 1D, 7 1C, 8 DP1, 9 2E, 10 2D, 11 2C, 12 DP2, 13 3E, 14 3D, 15 3C, 16 DP3,
    17 4E, 18 4D, 19 4C, 20 4B, 21 4A, 22 4F, 23 4G, 24 3B, 25 3A, 26 3F, 27 3G, 28 COL, 29 2B, 30 2A, 31 2F, 32 2G, 33 N/C, 34 1B, 35 1A, 36 1F,
    37 1G, 38-39 N/C, 40 COM. Pins on 2.54 mm pitch, rows 48.26 mm apart along the length (the drawing's 1.900 in), 22.73 mm across.
  - LCD-S3X1C50TR/B (digit 1 is the half digit: segments B and C joined as `1BC`, plus the minus sign `K`), 40 pins: 1 COM, 2 K, 3 1BC, 4-7 N/C, 8 DP1,
    9 2E, 10 2D, 11 2C, 12 DP2, 13 3E, 14 3D, 15 3C, 16 DP3, 17 4E, 18 4D, 19 4C, 20 4B, 21 4A, 22 4F, 23 4G, 24 3B, 25 3A, 26 3F, 27 3G, 28 N/C,
    29 2B, 30 2A, 31 2F, 32 2G, 33-37 N/C, 38 LB, 39 N/C, 40 COM.
  - LCD-S301C31TR, 24 pins, one COM (pin 1): 1 COM, 2 E1, 3 D1, 4 C1, 5 DP1, 6 E2, 7 D2, 8 C2, 9 DP2, 10 E3, 11 D3, 12 C3, 13 B3, 14 A3, 15 F3, 16 G3, 17 B2,
    18 A2, 19 F2, 20 G2, 21 B1, 22 A1, 23 F1, 24 G1; two rows of 12 on 2.54 mm pitch (27.94 mm length).

## 3. Design Overview

`lcd` is a sibling of `7seg`: the same `digits`/`leds`/`segmentPins` description, but a cell has a segment pin **and a backplane pin**
instead of a segment pin and a common. A cell is lit while its backplane pin is driven (not Z) and its segment pin is driven and
the two levels differ (segment ≠ backplane: the digital form of "the segment waveform is out of phase with the backplane"). A
multiplexed drive therefore gives each backplane its own time slot: the driver drives the active backplane with the square wave and
leaves the others Z; a cell is lit when it was lit in any step of the last interval ("on at any time during the last step", as
`7seg`). Static glass is the one-backplane case. Risk concentrates in the loader's cross-checks (every cell names a known segment pin
and backplane) and in the app symbol reuse; both are small.

Rejected: a `7seg` `type: "lcd"` (would loosen `commonAnode` rules and pollute 7seg goldens); a glass with analog levels (the engine has none).

## 4. Detailed Design

Device keys (as `7seg` where noted): `digits`, `colonAfter`, `dotAfter`, `color`, `size` (as 7seg), `"drive"`: `"static"` | `"1:2"` | `"1:3"` | `"1:4"`
(informational, shown in the inspector; the model does not use it), `"backplanes"`: array of backplane pin names, `"cells"`: array of
`{digit, segment, pin, bp}` (segment names as 7seg: A–G, DP, COL/L1.., annunciators `LB`, `K`, `MINUS`), `"segmentSide"`, `"backplaneSide"` (drawing order).
Pins: segment pins role `input`, backplane pins role `input`; no output is ever driven.

## Phases

### Phase 1 — backend kind

- [x] `backend/include/parts.h`: `PART_LCD`, `PartLcd` info (digits, drive, cell count) on `Part`.
- [x] `backend/src/models/lcd.c`: `LCD_KIND` (device/part keys, loader cross-checks: cell pins exist and are `input`, bp is one of `backplanes`, no duplicate (pin,bp) cell, `digits` 1–14), model, lights.
- [x] `backend/src/library/kinds.c`: the `LCD_KIND` line; `backend/CMakeLists.txt`: `src/models/lcd.c`.
- [x] `backend/tests/lcd_test.c` (+ `logic_test(lcd_test)`): static cell lit/unlit, X and Z segment or backplane → unlit-unknown rules, 1:3 multiplex slot (inactive backplane Z), loader rejections (unknown pin, unknown bp, duplicate cell).
- [x] `tests/support/listing.c`, `trace_check.c`, `listing_check.c`: kind name and facts, `TRACE_LAST_KIND`.
- [x] `docs/parts-format.md`: `lcd.json` row in the file table and a `### lcd` section; `docs/backend.md` kind list.

Acceptance: `ctest --test-dir /tmp/logic-build -R "lcd_test|library_check"` passes. Check: that command (est. 30 s).
Commit: —

### Phase 2 — glass parts

- [x] Footprints: 40-pin glass DIP-style leads on 2.54 mm pitch, 48.26 mm row, from the Lumex drawings (`datasheets/glass/` PDFs filed first): `explicit` footprints per `docs/parts-format.md` § Footprints.
- [x] `schematical/schematical/Parts/lcd.json`: the 3 Lumex and 4 generic devices (generic glass uses the 40-pin S401 footprint, unused pins `nc`; Lumex footprints `LCD-GLASS-40-S401`, `LCD-GLASS-40-S3X1`, `LCD-GLASS-24-S301`); datasheet entries in `datasheets.json` (lumex.com URLs); drawer `["Displays & Indicators", "LCD glass"]`.
- [x] Regenerate `backend/tests/golden/{parts,traces,footprints}.txt` and review the diff (additions only).

Acceptance: `ctest --test-dir /tmp/logic-build` 100%; `git diff --stat backend/tests/golden` shows additions only.
Commit: —

### Phase 3 — app

- [x] `PartCatalog.swift`, `PartFacts.swift`, `SchematicModel.swift`, `SchematicCanvasView.swift`, `SchematicExportScene.swift`, `ReferenceLabel.swift`, `PartLook.swift`: the glass uses `SevenSegmentSymbolView` with `PartDisplay` built from `cells` (the lit set from `circuit_part_lights`).
- [x] Part Editor: `KindInfo`, `PartKinds.all`, `KIND_FILES`, `PartKindKeysTests.files`, `LcdDraft.swift` (generated), `KindSchema`, `PartKindRoundTripTests`, ReferencePrefixes (`DS`).
- [x] Swift tests updated (`DrawerCategoriesTests` counts, `KindInfoTests`, `PartEditorWindowTests`).

Acceptance: `xcodebuild test -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/DrawerCategoriesTests -only-testing:schematicalTests/KindInfoTests` → TEST SUCCEEDED (est. 3 min).
Commit: —

## Validation Plan

- Tests: `lcd_test`, goldens, the Swift tests above. Runtime proof: a bench circuit in `lcd_test` drives a 1:3 glass.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`. Final gate: run once in plan-56-G.

## Decisions

- Add an LCD glass kind in this plan so LCD drivers are observable — decided by owner, 2026-11-23.
- DigiKey lookups for glass use Safari, not `scripts/digikey.js` — decided by owner, 2026-11-23 (all DigiKey catalog glass is static drive; multiplexed glass is the bundled generic parts).

## Corrections

(none yet)

## Summary

Small, copies the nixie/dekatron recipe; the only new idea is "segment differs from backplane".
