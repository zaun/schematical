# plan-32-D: Part editor footprints: custom footprint and the PCB footprint preview

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-C

The editor's footprint section gets two things: a **custom footprint** editor (a
pad table, stored in the part file), and a **PCB preview** that draws the footprint
the way the PCB page draws a placed part. The behavioral outcome: a user builds a
footprint from a pad table, sees it on the PCB preview with its pins, and the part
saved with it loads with that footprint.

References:

- `docs/parts-format.md` — § Footprints (the `footprints.json` entry format: `style`,
  `pins`, `body`, the explicit style's `pads`).
- `backend/src/library/footprints.c` — `EXPLICIT_KEYS` (line 41), `COMMON_KEYS` (line 33),
  the style dispatch (lines 868–881), `explicit_hole` (line 685), `pad_spec` (line 211),
  `load_footprint` (line 833).
- `backend/src/library/library.c` — `check_pads` (the pad-to-pin check used by
  `load_part_footprint`, line 433 region).
- `schematical/schematical/Backend/PartCatalog.swift` — `Footprint` (line 141),
  `Footprint.PadShape` and `Footprint.Corners` (lines 142–160).
- `schematical/schematical/Canvas/BoardModel.swift` — `PlacedChip.boardPads` (line 148),
  `PlacedPad` (line 95).
- `schematical/schematical/Canvas/PCBCanvasView.swift` — `outline(of:)` (line 95), the
  pad drawing the preview reuses.
- `schematical/schematical/Canvas/BoardColors.swift` — the copper and silkscreen colors.
- plan-32-A § 4.1 — the `footprints` key of the single-part file.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-C complete: the editor's footprint section chooses a bundled footprint | `xcodebuild test … -only-testing:schematicalTests/PartValidatorTests` → `passed` | MET (run 2026-11-11 in the P-32 worktree: `PartValidatorTests` passed, 4 cases) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- The footprint section offers **Custom footprint…** next to the bundled picker. It
  creates a custom footprint for the part: id `CUSTOM-<first 8 characters of the part's UUID>`,
  style `explicit`, a body size, and a pad table.
- Each pad row: designator, centre x and y (mm), width and height (mm), drill (mm, 0
  for surface mount), shape (`rect` or `round`), corner radius, chamfer and the cut
  corners, slot length (mm), and non-plated (yes/no). These are the explicit style's pad
  keys (`footprints.c`, `explicit_hole` and `pad_spec`).
- The PCB preview draws the custom footprint (or the chosen bundled footprint): copper
  pads in the PCB copper color, drill holes, the body outline, the pin-1 mark, and the
  designators, with the same geometry the PCB page uses for a placed footprint.
- The saved `.schpart` carries the custom footprint in `footprints`; the part's
  `footprint` key names it. Loading the file (plan-32-A) gives the part that footprint.

### Non-goals (explicit constraints)

- **One footprint per custom part.** A custom part has `footprint` only. `connectorFootprint`
  and `footprintChoices` stay bundled-only (decided, § Decisions): the parts that use them
  (`GENERIC-LED`, character LCDs, headers and pull resistors) are bundled parts, not custom ones.
- **`footprints.json` is not written.** The custom footprint lives in the part file only (owner decision).
- **Numbered pinouts only.** A custom footprint's pads are numbered by `pin` (`PAD_KEYS`); a ball pinout (`docs/parts-format.md` § Ball pinouts) has no custom footprint in v1. The editor's pinout style for a custom-footprint part is numbered (decided, § Decisions).
- **Bundled footprint geometry does not change.** `footprint_check`'s golden is untouched.
- **No free drawing.** The footprint is a pad table with numbers, not a canvas drag.

## 2. Current State

- The Swift footprint is `Footprint` (`PartCatalog.swift:141`): its pads carry the
  designator, rectangle, drill, shape (`Footprint.PadShape`, `rect`/`round`), radius,
  chamfer, corners (`Footprint.Corners`, the four-corner option set at `PartCatalog.swift:150`),
  slot length and plated flag.
- The backend's explicit style: the dispatch picks `EXPLICIT_KEYS` for `style: "explicit"`
  (`footprints.c:880–881`); each pad is parsed by `load_explicit` (`footprints.c:740`) with
  `explicit_hole` (`footprints.c:685`) and the shared size and corner checks (`pad_spec`, `footprints.c:211`; `corner_keys`, line 119).
- A part's pad check: `check_pads` (`library.c`, called from `load_part_footprint`) requires
  every pinout pin to have a pad with its designator (`docs/parts-format.md` § Footprints).
- The PCB page draws a placed part's pads through `PlacedChip.boardPads` (`BoardModel.swift:148`),
  which needs a `board` placement (`BoardPlacement`, line 41). The preview builds a
  `PlacedChip` at the origin with an identity `BoardPlacement`, so it reuses that code.
- Pin-1 mark: `PlacedChip.pin1Mark` and `BoardModel.swift:446–447` draw it.
- Measured 2026-11-11: `rg -c '"style":"explicit"' schematical/schematical/Parts/footprints.json`
  → 424 explicit footprints shipped; that is the sample Phase 1 round-trips.

### Verified properties

- **`boardPads` needs only `footprint` and `board`.** Read `BoardModel.swift:146–150`: the
  guard is `footprint` and `board`; the pins come from `part.pins`. A preview `PlacedChip`
  with a zero placement therefore gets its pads.
- **The explicit style takes `pads`, and the pad keys are those of § 4.1.** Read
  `footprints.c:41`, `:47–50` (`PAD_KEYS`), `:72–84` (`shape`), `:119–165` (`corner_keys`,
  the chamfer and corners form), and `:740` (`load_explicit`).
- **Shipped explicit footprints exist with a one-line record.** `rg -o '"id":"SON-6-DRY"'` and
  `"id":"X2SON-8"` both have `"style":"explicit"` (read from `footprints.json`).

## 3. Design Overview

1. **`CustomFootprintDraft`** (Swift): the form values for one footprint (id, description,
   body, pads). `encoded()` gives the `footprints.json`-format entry (§ plan-32-A 4.1).
2. **Footprint section** in `PartEditorWindow`: the bundled picker (plan-32-C) and a
   **Custom footprint…** button that creates or edits the draft; the pad table is a SwiftUI
   `Table` of `PadDraft` rows.
3. **`FootprintPreviewView`**: the PCB preview. It builds a `PlacedChip` from the
   validator's `CatalogPart` (plan-32-C § 4.7) with an identity placement and draws its
   `boardPads` and body with the PCB colors. Zoom and pan are the PCB page's own
   `CanvasViewport` (not a new one).
4. **Validation** (plan-32-C § 4.7): the custom footprint is sent to the backend inside the
   part file, so a pad error arrives with the backend's message.

Where correctness risk concentrates: the **pad geometry of a custom footprint**. A wrong
pad size is not caught by the backend (it checks designators, not geometry), so the preview
is the only check for geometry. The Phase 2 test compares a custom footprint's preview pads
to the same footprint's `footprint_check`-format line. Schedule: the data model and the
backend round trip first (Phase 1), then the editor UI, then the preview.

### Rejected alternatives

- **A canvas footprint editor (drag pads).** Rejected: a number table is exact, testable,
  and matches how the shipped footprints are written. Drawing is out of scope for v1.
- **Writing the custom footprint through `footprints.json`.** Rejected (owner decision, plan-32-A).
- **A new preview canvas.** Rejected: `PlacedChip.boardPads` and `PCBCanvasView`'s pad outline
  are the PCB's own geometry; a second one would drift.

## 4. Detailed Design

### 4.1 Pad row fields and their keys

The keys are `footprints.c`'s explicit style: the entry keys `COMMON_KEYS` (line 33:
`id`, `description`, `note`, `style`, `pins`, `body`) and `EXPLICIT_KEYS` (line 41: `pads`);
the body keys `BODY_KEYS` (line 46: `width`, `height`, `round`); the pad keys `PAD_KEYS`
(line 47: `pin`, `x`, `y`, `width`, `height`, `shape`, `drill`, `slotLength`, `plated`,
`radius`, `chamfer`, `corners`), read by `load_explicit` (line 740).

| Editor field | Key | Rule (as the backend checks it) |
|---|---|---|
| pin | `pin` | integer; the pinout's pin number; `0` = an unnumbered centre pad. Each pinout pin needs one pad |
| x, y | `x`, `y` | mm; body-centre origin (`docs/parts-format.md` § Footprints) |
| width, height | `width`, `height` | mm, > 0 |
| shape | `shape` | `"rect"` or `"round"` (`shape()`, `footprints.c:72`) |
| drill | `drill` | mm; 0 or absent = surface mount; must be smaller than the pad's both sides (`check_drill`) |
| slot length | `slotLength` | needs a drill; more than the drill; less than the pad's long side; the pad must not be square |
| non-plated hole | `plated` | only `false`, and only on `pin: 0`; round, its short side the drill, its long side the `slotLength` or the drill |
| corner radius | `radius` | rect only; at most half the shorter side; excludes `chamfer` and `corners` |
| corner cut | `chamfer`, `corners` | rect only; `chamfer` needs `corners`, a non-empty array of `"tl"`, `"tr"`, `"br"`, `"bl"` |

Body: `width`, `height` (mm), and `round` (boolean, for a round body).

### 4.2 The custom footprint's entry

```json
{ "id": "CUSTOM-6F1C2E8A", "description": "Hand drawn 2.54 mm pad row", "style": "explicit", "pins": 2,
  "body": { "width": 5.08, "height": 2.54 },
  "pads": [ { "pin": 1, "x": -1.27, "y": 0, "width": 1.6, "height": 1.6, "drill": 0.8, "shape": "round" },
            { "pin": 2, "x": 1.27, "y": 0, "width": 1.6, "height": 1.6, "drill": 0.8, "shape": "round" } ] }
```

`pins` is the number of pinout pins, and every pinout pin needs exactly one pad with its number
(`library.c`, `check_pads`). The explicit style has no `note`; it is optional.

## Phases

### Phase 1 — Backend and draft: the explicit pad keys, verified against the shipped explicit footprints

One line: the explicit footprint's keys are fixed from the backend source and the shipped explicit footprints parse back to the same entries.

- [x] `CustomFootprintDraft.encoded()` writes the § 4.1 keys (`pin`, `x`, `y`, `width`, `height`, `shape`, `drill`, `slotLength`, `plated`, `radius`, `chamfer`, `corners`) and the § 4.2 entry shape.
- [x] `schematical/schematical/Editor/CustomFootprintDraft.swift`: `CustomFootprintDraft` (id, description, body, pads as `PadDraft`), `init(footprint: Footprint)` for an explicit bundled footprint, and `encoded()` in the § 4.2 form.
- [x] Tests: `CustomFootprintDraftTests.swift`: for every shipped explicit footprint (424, § 2), `init(footprint:)` → `encoded()` → `load_footprint_entries` (plan-32-A; it is `footprint_read_entries`, reached through `load_footprints` on a one-entry file) accepts it, and the re-expanded `pads` equal the originals.

Acceptance: all 424 shipped explicit footprints round-trip through the draft, and the backend accepts each encoded entry.
  Check: `xcodebuild test … -only-testing:schematicalTests/CustomFootprintDraftTests` → `passed` with the count line `checked 424 explicit footprints, 0 mismatched` (est. 4 min; the population is every explicit footprint, which is the set that exercises every pad key).
  Result (2026-11-11): passed, `checked 424 explicit footprints, 0 mismatched` (pads, body, round flag and description compared; the entry also reads back to the same draft through `CustomFootprintDraft(json:)`).
Commit: 599bc30

### Phase 2 — Preview: the PCB footprint preview draws the same pads as the PCB page

One line: the preview's pad geometry equals the PCB page's geometry for the same footprint.

- [x] `schematical/schematical/Editor/FootprintPreviewView.swift`: builds a `PlacedChip` from a `CatalogPart` with a zero `BoardPlacement`, draws `boardPads` (via `PCBCanvasView.outline(of:)`), the body outline (`body.width`/`height`, round when `body_round`), the pin-1 mark (`PlacedChip.pin1Mark`), and designators, using `BoardColors`.
- [x] Zoom and pan use `CanvasViewport` (the PCB page's type), not a new one.
- [x] Tests: `FootprintPreviewTests.swift`: for three shipped footprints, `PDIP-14` (a DIP, the id verified in `footprints.json`), `SON-6-DRY` and `X2SON-8` (both explicit, verified above), the preview's `boardPads` rectangles equal the PCB page's `boardPads` for a placed bundled part that uses the same footprint, to within 1e-9 mm.

Acceptance: the three footprints' pads match the PCB page's pads (within 1e-9 mm).
  Check: `xcodebuild test … -only-testing:schematicalTests/FootprintPreviewTests` → `passed` (est. 4 min). This is the smallest check that compares preview geometry with the PCB page's geometry.
  Result (2026-11-11): passed (testDIP, testSON6, testX2SON8): pad rects (moved by the placed part's centre), drill, shape and corners equal, within 1e-9 mm.
Commit: 599bc30

### Phase 3 — Editor: Custom footprint… and the pad table

One line: a user builds a custom footprint and sees it on the preview.

- [x] `PartEditorWindow.swift`: the Footprint section gets **Custom footprint…**, which creates a `CustomFootprintDraft` (id `CUSTOM-` + first 8 characters of the part's UUID, style `explicit`, pins = pinout pin count) or edits the existing one.
- [x] The pad table: a SwiftUI `Table` of `PadDraft` rows with the § 4.1 columns; add and remove rows; the designator is picked from the pinout's pins.
- [x] The part's `footprint` key is the custom footprint's id; `footprints` in the file carries its entry (plan-32-A § 4.1). Removing the custom footprint restores the bundled picker.
- [x] The preview (Phase 2) sits beside the form and updates on each validated change.
- [x] Tests: `PartEditorFootprintTests.swift`: (a) a part with a custom footprint saves a `footprints` array with one entry and the part's `footprint` equals its id; (b) a pad whose designator is not a pinout pin is shown as a backend error; (c) removing the custom footprint removes the `footprints` key.

Acceptance: the three cases pass, and in a launched app a custom footprint created in the editor appears on the preview with its pads.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorFootprintTests` → `passed` (est. 5 min); the preview's visual check is the run skill's (est. 10 min).
  Result (2026-11-11): passed (a–c; (b)'s message is `b.schpart: footprints[0] (CUSTOM-…): pads[13]: "pin" must be a whole number from 0 to 14`). Real app: Copy to Editor… on SN74HC00N showed PDIP-14 in the preview (copper pads, holes, body, pin-1 mark); Custom Footprint… replaced the bundled picker with the pad table (14 rows) and the preview redrew as `CUSTOM-302737FE: 14 pads`, a 2.54 mm row of 0.8 mm-drill pads.
Commit: 599bc30

## Validation Plan

- Tests: `CustomFootprintDraftTests` (Phase 1), `FootprintPreviewTests` (Phase 2), `PartEditorFootprintTests` (Phase 3, cases a–c).
- Coverage check: `rg -n 'CustomFootprintDraft|FootprintPreviewView' schematical/schematical --glob '*.swift' -l` lists the new files, and each is in the app target.
- Runtime proof: New Part… (gate, a 14-pin pinout) → Custom footprint… → add 14 round pads at 2.54 mm pitch, 0.8 mm drill → the preview shows a DIP-style pad row → Save → File › Load Part… on the file → the placed part's PCB footprint is the custom one (run skill, est. 20 min).
- Doc sync: `docs/parts-format.md` § Footprints gets a paragraph on the custom footprint in a part file (the § plan-32-A 4.1 shape), and the note that `connectorFootprint`/`footprintChoices` stay bundled-only.
- Final gate (once, after Phase 3): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers) and `ctest --test-dir /tmp/logic-build -R footprint` (est. 1 min; `footprint_check` unchanged).

## Decisions

- **One footprint per custom part; `connectorFootprint` and `footprintChoices` are bundled-only** — the author's choice (no user-visible alternative was offered): the parts that use those keys are bundled kinds (`GENERIC-LED`, character LCDs, headers, pull resistors) whose footprint menus the PCB sidebar already owns. § 1.
- **The custom footprint lives in the part file only** — decided by the owner (plan-32-A § Decisions).
- **The custom footprint id is `CUSTOM-` + the part's UUID's first eight characters** — the author's choice; the id must match the backend's rule (`footprints.c`, `valid_id`: A–Z, 0–9, `.`, `-`), and the UUID's hexadecimal digits and hyphens satisfy it.
- **Pads are entered as numbers in a table, not drawn** — the author's choice, § 3.

## Corrections

- **The preview did not redraw for the same part** (found in the app, 2026-11-11).
  `CatalogPart` and `Footprint` compare by name / id only, so SwiftUI skipped a
  `FootprintPreviewView` whose part kept its name while its custom footprint's pads
  changed. `PartEditorModel.previewRevision` (one more per accepted check) is passed to
  the previews, so they redraw on every validated change.
- **The preview shows the last part the backend accepted** (`lastValidPart`), so it stays
  while the draft has an error.
- **The preview sits in a pane beside the form** (an `HSplitView`); plan-32-E adds the
  Block | Footprint segment above it.
- **The PCB preview fits the footprint, then zooms with `CanvasViewport`'s zoom steps
  and pans by drag** (the viewport's `zoom` and `offset`); Fit resets both.
- **Doc sync done**: `docs/parts-format.md` § Footprints › Custom footprints in a part
  file.
- **Runtime proof (Validation Plan)**: the preview half is Phase 3's app check above. The
  Load Part… half (a saved custom-footprint part loads with that footprint) is checked by
  `PartEditorFootprintTests` (a), whose validator loads the draft through
  `load_part_file` and reads back `CUSTOM-…` with 14 pads, and by plan-32-A's
  `part_file_test` (a) and the Load Part… session in plan-32-A Phase 4 (a fixture with an
  inline footprint).

## Summary

The risk is pad geometry, which the backend does not check and the preview is the only
guard for; Phase 2 compares it against the PCB page. Untouched: `footprints.json`, the
shipped footprints, and the bundled footprint picker.
