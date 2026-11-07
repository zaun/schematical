# plan-16-C: Layer view

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-16-B

A **View** button sits just left of the zoom controls on the PCB page. It
opens a popover with a checkbox for each copper layer (Top, Layer 2…, Bottom),
then Silkscreen and Document, plus Show All. The canvas draws only the
checked layers, and the Select tool only hits what is drawn. Copper is drawn
in its layer's colour from plan-16-A:

- surface-mount pads in their side's colour;
- a Plane layer as a tinted fill over the board;
- through-hole pads in copper, since they go through every layer.

References:

- `schematical/schematical/ContentView.swift` (PCB toolbar, `zoomControls`)
- `schematical/schematical/Canvas/PCBCanvasView.swift:draw(in:size:)`, `begin(at:)`
- `schematical/schematical/Canvas/BoardColors.swift`
- plan-16-A § Copper layers, plan-16-B § Draw order

## Prerequisites

See plan-16-A § Prerequisites. plan-16-B must be complete:
`rg -n 'enum BoardSide' schematical/schematical/Canvas/BoardSettings.swift` → 1 match.

## 1. Goal

- Unchecking a layer in View removes it from the canvas at once:
  - its pads, traces and plane fill;
  - for Top or Bottom, that side's marks and texts too.

  Clicking where a hidden item was selects nothing there. The checked set is
  per window, is not saved, and starts with everything checked.

### Non-goals

- The visible set is not saved and not part of the file.
- No per-layer opacity, and no "active layer on top" reordering.
- No traces yet (D and E). C's filter covers them through the same rule.

## 2. Current State

- The PCB toolbar's primary actions are Rotate, `zoomControls` (−, menu, +)
  and the Inspector toggle (`ContentView.swift`, `if page == .pcb` …
  `ToolbarItemGroup(placement: .primaryAction) { zoomControls }`).
- `ContentView` holds per-window PCB state as `@State` (`boardViewport`,
  `pcbTool`), and passes it to `PCBCanvasView(document:viewport:tool:)`.
- Every pad is filled with `PCBCanvasView.copper`. Drill holes are filled
  with the canvas background colour.
- After plan-16-B the draw order is: bottom-side footprints and texts, then
  top-side footprints and texts, then selection. Hit testing tries Top first,
  then Bottom.
- `Footprint.Pad.drill > 0` marks a through-hole pad (`PlacedPad.drill`);
  `drill == 0` marks a surface-mount pad.

### Measured populations

| What | Count | Command |
|---|---|---|
| Pad fills to recolour | 1 | `rg -n 'with: .color\(Self.copper\)' schematical/schematical/Canvas/PCBCanvasView.swift \| wc -l` → 1 |
| PCB hit-test entry points | 2 (`text(at:)`, `footprint(at:)`) | `rg -n 'func (text\|footprint)\(at' schematical/schematical/Canvas/SchematicModel.swift \| wc -l` → 2 |

### Verified properties

- A SwiftUI `Menu` closes after each item on macOS, so it can't toggle
  several layers in one go. A `Button` that opens a `.popover` holding
  `Toggle`s stays open until the user clicks away. That gives the
  multi-select the owner asked for.

## 3. Design

### Visible set

```swift
nonisolated enum BoardViewLayer: Hashable, Sendable {
    case copper(CopperLayer)
    case silkscreen
    case document
}
```

- **State.** `ContentView` adds `@State private var hiddenLayers:
  Set<BoardViewLayer> = []`. It stores the *hidden* set, so layers added
  later (going from 2 to 4) start visible with no extra step. It passes
  `visible: BoardVisibility(hidden: hiddenLayers)` to the canvas.
  `BoardVisibility` is a small value with `func shows(_ layer:
  BoardViewLayer) -> Bool` and `var showsAnyCopper: Bool` (taking the
  board's `copperLayers`).
- **View button.** `Label("View", systemImage: "square.3.layers.3d")`, with
  help "Choose which layers to show". It sits in its own `ToolbarItem` just
  before `zoomControls`, PCB page only.
- **Popover.** One `Toggle` (checkbox) per `copperLayers` entry, titled with
  its name and a small swatch of its colour. Then a divider, Silkscreen and
  Document. Then a "Show All" button that empties `hiddenLayers`.

### What each layer shows

A part on side S, where S's copper layer is `S.copperLayer`:

| Item | Drawn and hittable when |
|---|---|
| Surface-mount pad | `.copper(S.copperLayer)` is visible |
| Through-hole pad | any copper layer is visible |
| Body outline / pin-1 / reference on `.silkscreen` | `.silkscreen` and `.copper(S.copperLayer)` both visible |
| The same on `.document` | `.document` and `.copper(S.copperLayer)` both visible |
| Board text on side S, layer L | L's view layer and `.copper(S.copperLayer)` both visible |
| Plane fill of layer P | `.copper(P)` visible and P's type is Plane |
| Trace on layer T (D/E) | `.copper(T)` visible |

A footprint can be hit (by its `boardBounds`) when anything of it is drawn.
A text can be hit when it is drawn.

### Colours and order

- Surface-mount pads are filled with `style(of: S.copperLayer).color`.
- Through-hole pads keep `PCBCanvasView.copper`.
- A plane fill is the board rect in the layer's colour at 0.2 opacity.

Draw order, back to front:

1. board fill, then grid;
2. for each copper layer from Bottom up to Top (reverse `copperLayers`):
   - its plane fill (if Plane);
   - its traces (E);
   - if it is Bottom or Top, that side's surface-mount pads;
   - after the Bottom layer, bottom-side marks and texts;
3. through-hole pads and their drill holes;
4. top-side marks and texts;
5. selection.

### Gaps closed

- **Selection on a hidden layer.** Hiding a layer does not clear a
  selection. The sidebar still shows and edits a selected item whose layer is
  hidden. Its selection box is not drawn while nothing of it is visible.
- **Layers removed by a count change.** Their entries stay in
  `hiddenLayers`, which does no harm: they come back hidden if the layer
  returns. Show All clears them.

### Rejected alternatives

- **A `Menu` of toggles.** It closes after every click (see Verified
  properties).
- **Saving the visible set in the file.** It is a view setting, like zoom,
  which is not saved either.
- **Storing the visible set.** New layers would then need adding by hand.
  The hidden set needs no upkeep.

## Phases

### Phase 1 — Visibility model and filtered drawing

- [x] `Canvas/BoardSettings.swift`: `BoardViewLayer`, `BoardVisibility`
      (`shows`, `showsAnyCopper`).
- [x] `Canvas/PCBCanvasView.swift`: add `var visible = BoardVisibility()`.
      Draw in the §3 order with the §3 table. Surface-mount pads take their
      side's colour; Plane layers get their fill. Selection boxes are drawn
      only for visible items.
- [x] `Canvas/SchematicModel.swift`: `text(at:visible:)` and
      `footprint(at:visible:)` skip items that aren't drawn. The canvas's
      `begin(at:)` passes the visibility.
- [x] Tests (`<tests>/BoardVisibilityTests.swift`): the table, row by row,
      for a top part and a bottom part. Hit tests miss a footprint whose side
      and silkscreen are hidden. A through-hole-only footprint stays hittable
      while any copper layer is visible.
      `/tmp/p16t.sh BoardVisibilityTests,BoardSideTests` → "Executed 12
      tests, with 0 failures". The table is `BoardVisibility`'s
      `showsPad` / `showsMark` / `showsText` / `showsPlane` / `showsTrace` /
      `showsFootprint`, which both drawing and hit testing call.

Acceptance: every table row holds in tests, and a hidden item can't be hit.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:<tests>/BoardVisibilityTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — View popover

- [ ] `ContentView.swift`: `hiddenLayers` state; the View button and popover
      (§3) before `zoomControls`, PCB page only; pass `visible` into
      `PCBCanvasView`.

Acceptance: on a 4-layer board, unchecking Top hides top surface-mount pads
and top marks, while through-hole pads stay. Unchecking every copper layer
hides all pads. Show All restores everything. Several toggles work without
reopening the popover.
  Check: run the app (`run` skill), open `be-6502.schematical`, PCB page, and
  try each of those in the View popover (est. 3 min).
Commit: —

## Validation Plan

- Tests: `BoardVisibilityTests`.
- Runtime proof: Phase 2's check; a Plane inner layer shows as a tint and
  goes when unchecked.
- Doc sync: `PCBCanvasView` doc comment (layers, draw order, hit rules). No
  file-format change.
- Final gate: run once after E (see plan-16-E).

## Decisions

- View lists every copper layer plus Silkscreen and Document — author's
  call, delegated by the owner on 2026-11-07. The owner asked for "all the
  layers", and marks are the other layers a board has.
- A part's marks and texts belong to their side, so hiding Top hides Top's
  silkscreen too.
- Through-hole pads show while any copper layer is visible, and are drawn in
  copper.
- The visible set is per window and not saved. Everything starts visible.

## Corrections

- 2026-11-07: `BoardVisibility` also carries the board's `copperLayers`
  (for `showsAnyCopper`, as §3 says it takes them). `ContentView` passes
  only the hidden set; the canvas fills in the board's layers when it
  draws and hit-tests.

## Summary

The risk is drawing order and hit testing drifting apart. Both use the same
visibility table, and the Phase 1 tests check the table for each item kind.
Nothing in the file format changes.
