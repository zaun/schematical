# plan-03-D: Schematic and PCB tabs

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-03-C

The window gets two tabs: **Schematic** (today's canvas, unchanged) and
**PCB**. The PCB tab shows every placed part except VCC/GND as its footprint
— pads, drill holes, body outline, pin-1 mark, reference — on a 0.635 mm
grid. Footprints can be selected, dragged and rotated in 90° steps. Placement
is saved in the `.schematical` file. Footprints only: no wires, nets,
ratsnest, board outline, layers, bottom side or design-rule checks.

Outcome: open `test.schematical`, switch to PCB: the oscillator and the LED
appear as footprints (VCC and GND do not); drag one, rotate one, save,
reopen — positions and rotations come back; the Schematic tab looks and works
exactly as before.

References:

- plan-03-A §4.3 (`PartFootprint`, pad coordinates, units) and the Swift
  `CatalogPart.footprint` added in plan-03-A Phase 2.
- `schematical/schematical/ContentView.swift`,
  `Canvas/SchematicCanvasView.swift`, `Canvas/SchematicModel.swift`
  (`PlacedChip`, `SchematicDocument`), `Canvas/CanvasViewport.swift`,
  `Document/SchematicFile.swift`, `Document/SchematicFileController.swift`.
- `docs/file-format.md` (§ Changing the format).

## Prerequisites

See plan-03-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-03-C complete | `ls planning/completed/plan-03-C-other-footprints.md` → exists | MET (2026-11-01: exists, 9f962cd) |
| Every non-supply part has a footprint | `ctest --test-dir /tmp/logic-plan03 -R library_check` passes with the required-key cases (plan-03-C Phase 3) | MET (2026-11-01: `library_check … Passed`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- A "Schematic | PCB" switch in the toolbar; Schematic shows today's canvas,
  tools and drawer; PCB shows footprints.
- Every non-supply part is on the PCB from the moment it is placed; erasing it
  on the schematic removes it from the PCB.
- PCB: click selects, drag moves (snapped to 0.635 mm), Rotate turns the
  selected footprint 90° clockwise; drag on empty space pans; zoom controls
  work on the active tab.
- Positions and rotations round-trip through Save / Open; files written
  before this plan open, with footprints auto-placed.

### Non-goals (explicit constraints)

- Schematic tab behavior and look unchanged (tools, drawer, play mode, memory
  windows).
- File format `version` stays `2` (plan-02-B); files written by this build still open in
  the previous build (the new field is ignored by its decoder).
- No copper, nets, ratsnest, board outline, layers, bottom side, DRC,
  Gerber or other export.
- The PCB does not show simulation state.

## 2. Current State

- `ContentView.swift:body` shows one `SchematicCanvasView` with an
  `.inspector` (the parts drawer) and a toolbar: file controls and Run in
  `.navigation`, the tool `Picker` in `.principal`, zoom controls and the
  drawer toggle in `.primaryAction`. Shortcuts in use: ⌘O, ⌘S, ⇧⌘S
  (`SchematicFileController.swift:199-213`), ⌘R (Run), ⌘-, ⌘=, ⌘0
  (`ContentView.swift`).
- `CanvasViewport` (`CanvasViewport.swift`) is a plain `@Observable` class:
  screen = world × zoom + offset, zoom levels 0.25…3. A second instance
  serves a second canvas with no change.
- `SchematicCanvasView` pans on a drag over empty space (`Interaction.pan`,
  `handleChanged`), draws a dot grid (`GridBackground`) and its world layer
  in an overlay scaled by the viewport.
- `SchematicDocument` (`SchematicModel.swift:659`) owns `chips`, `wires`,
  `junctions`, `selectedChipID`, `nextReference`; `placeChip` appends a
  `PlacedChip`; `erase(at:)` removes a chip and its wires.
- `PlacedChip` (`SchematicModel.swift:383`) holds the `CatalogPart`, symbol,
  schematic origin, flip and switch states.
- `SchematicFile.Part` (`SchematicFile.swift:35`) stores id, reference,
  partNumber, x, y, flipped, optional switches. After plan-02-B the file is
  version 2 (wire ends name pins by designator) and version 1 files are
  converted on open; `test.schematical` is version 1. `docs/file-format.md`
  § Changing the format: an optional field keeps `version`. Swift's
  `JSONDecoder` ignores unknown keys, so older builds read newer files.
- Unsaved changes are detected by comparing `SchematicFile(document:)`
  snapshots (`SchematicFileController.swift:73`), so anything added to the
  file struct is covered automatically.
- The app has no unit-test target (`grep -c xctest project.pbxproj` → 0);
  footprint geometry is tested in C (plan-03-A).

## 3. Design Overview

Four pieces, lowest risk first:

1. **Model** — `BoardPlacement` (centre in mm, rotation) on `PlacedChip`,
   set for every part with a footprint; document methods to auto-place, move,
   rotate and hit-test footprints.
2. **File** — optional `"pcb"` object per part; read, written, validated;
   auto-placement for parts without it.
3. **PCB view** — `PCBCanvasView`: grid, footprints, selection, gestures.
4. **Tabs** — `ContentView` switches between the two canvases and their
   toolbars.

Correctness risk: the rotation transform (pads, hit box) and the
world/mm scale — a mistake shows as misplaced pads, caught by the runtime
proof's screenshots of a rotated SOIC/LED. The file change is additive and
covered by the round-trip check.

Byte-identity is not a gate: no backend change. The backend tests must still
pass (nothing in `backend/` changes).

Rejected alternatives:

- *Separate PCB document / list of PCB parts* — the owner chose automatic
  sync; one list of chips with an optional placement cannot drift.
- *A separate selection per tab* — sharing `selectedChipID` lets a part
  selected on one tab stay selected on the other; no reason to keep two.
- *macOS `TabView`* — its tab strip takes a row of height and does not fit
  the unified compact toolbar; a segmented picker in the toolbar matches the
  existing tool picker.
- *A Swift test target* — needs hand edits to `project.pbxproj`; the
  geometry is already golden-tested in C, and the Swift side is a transform
  plus drawing, proven at runtime.

## 4. Detailed Design

### 4.1 Units and scale

Board positions are millimetres. The PCB view draws 1 mm as 20 world units
(`BoardScale.pointsPerMM = 20`), so at 100 % a 14-pin SOIC (8.65 mm) is 173
points tall. `BoardGrid.spacing = 0.635` mm; `BoardGrid.snap` rounds a point
to it. 2.54 mm through-hole pads land on the grid (2.54 = 4 × 0.635, and a
DIP's ±3.81 = 6 × 0.635).

### 4.2 Model (`Canvas/BoardModel.swift`, new, and `SchematicModel.swift`)

```swift
struct BoardPlacement: Hashable {
    var centre: CGPoint          // mm, footprint origin (body centre)
    var rotation: Int            // 0, 90, 180, 270, clockwise
}
```

- `PlacedChip.board: BoardPlacement?` — non-nil exactly when
  `part.footprint != nil`.
- `PlacedChip.boardPads: [PlacedPad]` — the footprint's pads, each paired
  with the `ChipPin` of the same `designator` (plan-02-A); pads of `absent`
  pins are skipped, an unnumbered centre pad is kept; rotated and translated to board mm. Rotation is
  clockwise in a y-down frame: 90° maps (x, y) → (−y, x); width and height
  swap at 90°/270°.
- `PlacedChip.boardBody: (rect: CGRect, round: Bool)` and
  `boardBounds: CGRect` (body ∪ pads, rotated) for drawing and hit tests.
- `PlacedChip.pin1Mark: CGPoint` — 0.5 mm outside the body corner nearest
  the pad of the part's first pin (`pins[0]`: "1", or "A1" on a BGA), rotated.
- `SchematicDocument`:
  - `placeChip` sets `board = nextBoardSpot(for:)` when the part has a
    footprint: first footprint at (0, 0); later ones to the right of the union
    of all `boardBounds`, 2.54 mm gap, top edge aligned to the union's top,
    snapped.
  - `moveFootprint(_ id:, to centre:)` (snaps), `rotateFootprint(_ id:)`
    (+90 mod 360), `footprint(at mm:) -> PlacedChip?` (last chip whose
    `boardBounds` contains the point; drawing order = `chips` order).
  - `replaceContents` takes chips as built by the file loader (placements
    included).

### 4.3 File (`SchematicFile.swift`, `docs/file-format.md`)

`SchematicFile.Part` gains `var pcb: Placement? = nil` with
`struct Placement: Codable { var x: Double; var y: Double; var rotation: Int }`.

```json
{ "id": "...", "reference": "U2", "partNumber": "LED-AMBER-0603",
  "x": 200, "y": 100, "flipped": false,
  "pcb": { "x": 5.08, "y": 0, "rotation": 90 } }
```

- Written for every part with a placement; left out for supplies.
- On open: `rotation` not in {0, 90, 180, 270} → new error
  `SchematicFileError.badRotation(reference:)` ("U2 has PCB rotation 45; it
  must be 0, 90, 180 or 270."); a `pcb` on a part without a footprint is
  ignored; a part with a footprint and no `pcb` is auto-placed after all
  stored placements are applied, in file order; stored positions are snapped
  to the 0.635 mm grid.
- `version` stays 2 (optional field, § Changing the format). A version 1
  file is converted as plan-02-B describes and then auto-placed.
- `docs/file-format.md`: the `pcb` field in the Parts table and example, the
  rule above in § Validation on open, the coordinate note (mm, footprint
  centre, y down).

### 4.4 PCB view (`Canvas/PCBCanvasView.swift`, new)

- Background `Color(white: 0.13)`; grid dots every 2.54 mm (4 grid steps),
  hidden when the on-screen spacing is under 8 points.
- For each chip with a placement, in `chips` order, in one `Canvas`:
  body outline (stroke 0.15 mm, `Color(white: 0.85)`; ellipse if round),
  pads (fill `Color(red: 0.80, green: 0.62, blue: 0.30)`; rect or ellipse;
  a drill hole filled with the background colour), pin-1 mark (filled
  circle, 0.4 mm), reference text centred on the body (height 1 mm, min 8
  points on screen). Selected chip: accent-colour stroke round
  `boardBounds`, 2 points on screen.
- Gestures (`DragGesture(minimumDistance: 0)`, the `SchematicCanvasView`
  pattern): press on a footprint → select it and move it while dragging
  (centre = start centre + delta, snapped); press on empty space → clear the
  selection and pan. While the simulation runs, every drag pans and nothing
  is selected or moved (same rule as the schematic).
- Uses its own `CanvasViewport` (`boardViewport` in `ContentView`).

### 4.5 Tabs (`ContentView.swift`)

- `enum EditorPage: String, CaseIterable, Identifiable { case schematic, pcb }`,
  titles "Schematic", "PCB"; `@State private var page = EditorPage.schematic`.
- `.principal` becomes a `ToolbarItemGroup`: a segmented `Picker` for the
  page, then the tool `Picker` only when `page == .schematic`.
- Body: `SchematicCanvasView` (unchanged arguments) or `PCBCanvasView`.
- Drawer: `.inspector(isPresented:)` bound to `isDrawerShown && page ==
  .schematic` (a `Binding` whose setter writes `isDrawerShown`); the drawer
  toggle is shown only on the Schematic page.
- PCB page, `.primaryAction`: a Rotate button (`rotate.right`, help "Rotate
  the selected footprint 90° (⌥⌘R)", shortcut ⌥⌘R), disabled with no
  selection, no footprint selected, or while running.
- Zoom controls act on `page == .schematic ? viewport : boardViewport`.
- Run, Open, Save and the error overlay are unchanged on both pages.

## Compatibility / Format Impact

- `.schematical`: optional `pcb` per part. Old files open (auto-placed); new
  files open in the previous build (field ignored). `version` 2.
- No backend, part file or golden change.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Model and file

Placements exist and are saved; nothing draws them yet.

- [x] `Canvas/BoardModel.swift`: `BoardScale`, `BoardGrid`, `BoardPlacement`,
      `PlacedPad`, the rotation transform, `PlacedChip` extensions (§4.2).
- [x] `SchematicModel.swift`: `PlacedChip.board`; `SchematicDocument`
      `nextBoardSpot`, `moveFootprint`, `rotateFootprint`, `footprint(at:)`;
      `placeChip` assigns a placement.
- [x] `SchematicFile.swift`: `Part.pcb`, write, read, `badRotation`,
      auto-placement on open (§4.3).
- [x] `docs/file-format.md`: § Parts, example, § Validation on open (§4.3).

Acceptance: placements round-trip and old files gain placements on open.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min). — 2026-11-01: `** BUILD SUCCEEDED **`.
  Check: launch the Debug build (run skill), open `test.schematical`, Save As `/tmp/p03d.schematical`; `python3 -I -c 'import json;d=json.load(open("/tmp/p03d.schematical"));print(sorted((p["partNumber"],"pcb" in p) for p in d["parts"]))'` → `GND False`, `LED-AMBER-0603 True`, `OSC-SIT1534-1508 True`, `VCC False` (est. 3 min). — 2026-11-01, headless (screen locked; Corrections): `[('GND', False), ('GND', False), ('LED-AMBER-0603', True), ('OSC-SIT1534-1508', True), ('VCC', False)]` (the file has two GNDs); the version 1 file was auto-placed (U2 at (0, 0), U3 at (3.81, 0)); reopening the saved file and saving again gives the same `pcb` values (`round trip pcb equal: True`).
Commit: —

### Phase 2 — PCB view and tabs

- [ ] `Canvas/PCBCanvasView.swift` (§4.4).
- [ ] `ContentView.swift`: `EditorPage`, page picker, `boardViewport`, body
      switch, drawer binding, Rotate button, zoom target (§4.5).

Acceptance: the PCB tab draws, selects, moves and rotates footprints; the
Schematic tab is unchanged.
  Check: the Phase 1 `xcodebuild` → `** BUILD SUCCEEDED **` (est. 3 min).
  Check: runtime proof below, steps 1–5 (est. 10 min).
Commit: —

## Validation Plan

- Tests: backend `ctest` unchanged and passing (no backend edits); geometry
  is covered by `footprint_check` (plan-03-A…C).
- Coverage check: the runtime proof exercises each new path — auto-place,
  move, rotate, save, open with and without `pcb`, `badRotation`.
- Runtime proof (Debug build, `run` skill; screenshots to `/tmp/p03d-proof/`):
  1. Open `test.schematical`; Schematic tab looks as before (screenshot
     compared by eye with one taken before Phase 2).
  2. PCB tab: two footprints (OSC-SIT1534-1508, LED-AMBER-0603) side by side,
     no VCC/GND; drawer hidden; tool picker hidden.
  3. Drag the LED; rotate the oscillator twice; the pads turn with the body
     and the pin-1 mark follows pin 1.
  4. Back to Schematic, drag a `SN74HC00N` from the drawer; PCB tab shows a
     14-pin DIP to the right of the others, pin 1 square, top-left; erase it on
     the Schematic tab; it is gone from the PCB.
  5. Save, close, reopen: same positions and rotations. Edit the saved file's
     LED `rotation` to 45 → Open shows "U… has PCB rotation 45; it must be 0,
     90, 180 or 270." and the canvas is unchanged.
- Doc sync: `docs/file-format.md`.
- Final gate (plan-03, once, after this phase): `cmake -S backend -B /tmp/logic-final && cmake --build /tmp/logic-final -j8 && ctest --test-dir /tmp/logic-final` (est. 1 min) + the `xcodebuild` (est. 3 min) + the runtime proof (est. 10 min).

## Decisions

- Automatic sync of schematic parts to the PCB — owner, 2026-10-31 (§4.2).
- Grid 0.635 mm, move and rotate in 90° steps, top side only — owner,
  2026-10-31 (§4.1, §4.5).
- Author's choices: 20 points per mm at 100 %; dark board background and
  copper-coloured pads; shared selection across tabs; ⌥⌘R for Rotate
  (⌘R is Run); segmented page picker in the toolbar instead of a `TabView`;
  auto-placement to the right of existing footprints.

## Corrections

- 2026-11-01: runtime checks run headlessly: the session is locked
  (`CGSSessionScreenIsLocked` true), so no window can be driven or
  screenshotted. `/tmp/p03/harness/build.sh` compiles the app's Swift
  sources (all but `schematicalApp.swift`) and the backend into a CLI inside
  a copy of the Debug `.app` (plan-03-A Corrections); it opens and saves
  through `SchematicFile.decode`/`apply`/`encoded`, drives the document
  calls the gestures make, and renders the views with `ImageRenderer`.
- 2026-11-01: `badRotation` carries the rotation as well as the reference
  (`badRotation(reference:rotation:)`), since §4.3's message quotes it.
- 2026-11-01: `pin1Mark` picks the body corner nearest pin 1's pad by
  distance (ties → top-left first), so a pin 1 on an axis (LCCC-20's
  top-centre pin 1) still gets a defined corner.

## Summary

The risk is the rotation/scale transform, proven by screenshots of rotated
footprints. The file change is one optional field; the Schematic tab and the
backend are untouched.
