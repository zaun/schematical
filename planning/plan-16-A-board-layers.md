# plan-16-A: Board layers

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: plan-15 (see Prerequisites)

plan-16 updates the PCB page in five steps:

- A: the board's copper layers get names, colours and types, edited in a new
  Layers section, and the PCB page opens at 25 %.
- B: parts and board texts can go on the bottom side, mirrored.
- C: a View popover picks which layers the canvas draws.
- D: the trace model: 45° routing, joins on contact, file format.
- E: the Trace tool, its layer menu and the trace sidebar.

This sub-plan (A) delivers the layer stack. With nothing selected, the PCB
sidebar shows a **Layers** section under **Board**, with one row per copper
layer: a colour well, the name (Top, Layer 2 … Layer N−1, Bottom) and a Type
(Top and Bottom are always Component; inner layers are Plane or Wire). The
colours and types are saved in the file. The PCB page opens at 25 % zoom.

References:

- `schematical/schematical/Canvas/BoardSettings.swift:BoardSettings`
- `schematical/schematical/Canvas/PCBSidebarView.swift:BoardPanel`
- `schematical/schematical/Canvas/CanvasViewport.swift:CanvasViewport`
- `schematical/schematical/ContentView.swift:boardViewport`
- `schematical/schematical/Document/SchematicFile.swift:Board`, `boardSettings()`
- `docs/file-format.md` § Board, § errors
- `planning/plan-15-multiselect.md` (the selection model and the XCTest target this plan builds on)

## Prerequisites

These apply to all of plan-16. Sub-plans B–E point here.

| Must be true | Command | Status |
|---|---|---|
| plan-15 is complete and archived | `ls planning/plan-15-* 2>/dev/null` → no output, and `ls planning/completed/plan-15-multiselect.md` → listed | NOT MET (2026-11-07: `planning/plan-15-multiselect.md` is still open) |
| The app has a unit-test target (added by plan-15 Phase 1) | `grep -c 'com.apple.product-type.bundle.unit-test' schematical/schematical.xcodeproj/project.pbxproj` → 1 or more | NOT MET (2026-11-07: 0) |
| PCB selection is set-based (plan-15) | `rg -n 'selectedChipID\|selectedTextID' schematical/schematical --glob '*.swift' \| wc -l` → 0 outside plan-15's single-item compatibility accessors | NOT MET (2026-11-07: 37) |

If plan-15 is not complete, plan-16 cannot start, full stop. Everything below
assumes plan-15 has landed: PCB selection is a typed set of footprint and
board-text items, Shift-click toggles items, a group drag moves every
selected item, and there is an XCTest target. In the check commands,
`<tests>` means the name of that test target
(`xcodebuild -list -project schematical/schematical.xcodeproj` prints it).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all*
> prerequisites.

## 1. Goal

- With nothing selected on the PCB page, the sidebar shows a Layers section
  with one row per copper layer: a colour well, the layer's name and its type.
  Changing a colour or an inner layer's type persists across Save and Open.
  The PCB page opens at 25 % zoom.

### Non-goals

- Nothing yet draws in layer colours. B, C and E use them.
- No nets on planes, no pours, no clearances.
- The schematic page's zoom, the zoom steps and "Actual Size" (⌘0 = 100 %)
  stay as they are.
- The file version stays 2. Every new key is optional.

## 2. Current State

- `BoardSettings.layers` is an `Int` in `layerCounts = [1, 2, 4, 6]`. Its
  comment says "Stored only; the top side is drawn." The only UI is the
  `Picker("Layers")` in `BoardPanel` (`PCBSidebarView.swift:54`).
- `CanvasViewport` starts at `zoom = 1` (`CanvasViewport.swift:11`). Its zoom
  steps are `[0.25, 0.5, 0.75, 1, 1.5, 2, 3]`, so 25 % is the smallest.
  `ContentView` creates `boardViewport = CanvasViewport()`
  (`ContentView.swift:30`), separate from the schematic's `viewport`.
- `SchematicFile.Board` decodes `width/height/layers/color/units/texts`, each
  optional. `boardSettings()` validates them and throws
  `SchematicFileError.badPCB("<detail>")` (`SchematicFile.swift:559-578`).
- Precedent for raw-value enums with defaults: `BoardColor`, `BoardUnits`
  (`BoardSettings.swift`), decoded with `Self.value(_:of:_:default:)`.
- The backend never reads the board: `rg -l '"board"|texts' backend` → no
  output, so `backend/` is untouched.

### Measured populations

| What | Count | Command |
|---|---|---|
| Code that reads or writes `board.layers` | 5 lines in 2 files | `rg -n 'board.layers\|\.layers\b' schematical/schematical --glob '*.swift'` → `PCBSidebarView.swift:54`, `SchematicFile.swift:332,571,573,575` |
| Board viewports to start at 25 % | 1 | `rg -n 'boardViewport = CanvasViewport' schematical/schematical/ContentView.swift` → line 30 |
| Repo sample files with a board | 1 (`be-6502.schematical`, `"layers" : 2`) | `rg -n '"layers"' be-6502.schematical test.schematical` → `be-6502.schematical:5` |

### Verified properties

- `ColorPicker` with `supportsOpacity: false` and `.labelsHidden()` is a square
  colour well on macOS. Clicking it opens the system colour panel, which is
  the "square colour patch, click to change" requested. SwiftUI only.
- The layer index rule holds for every allowed count. Layer 1 is Top and
  layer N is Bottom; 2…N−1 are inner. For N = 1, Top and Bottom are the same
  layer, so a 1-layer board has only Top.

## 3. Design

### Copper layers

```swift
nonisolated enum CopperLayer: Hashable, Sendable {
    case top
    case inner(Int)   // 2...5, its position in the stack
    case bottom
}
```

- `BoardSettings.copperLayers` lists the layers in stack order. For 1 layer
  it is `[.top]`; for 2, `[.top, .bottom]`; for 4,
  `[.top, .inner(2), .inner(3), .bottom]`; for 6, `.inner(2)` to `.inner(5)`
  between Top and Bottom.
- Raw strings for the file are `"top"`, `"bottom"` and `"inner2"` to
  `"inner5"`.
- Titles are "Top", "Bottom" and "Layer 2" to "Layer 5".

Inner layers keep their number when the count changes. Going from 4 to 6
keeps Layer 2 and Layer 3 (with their colours and types), adds Layer 4 and
Layer 5, and Bottom stays Bottom. Bottom is a fixed identity, so the bottom
side never renumbers.

### Layer style

```swift
nonisolated enum LayerType: String { case component, plane, wire }
nonisolated struct LayerStyle: Hashable, Sendable { var color: LayerColor; var type: LayerType }
```

- `LayerColor` is an sRGB `#RRGGBB` value: `struct LayerColor { var red,
  green, blue: UInt8 }`, with `init?(hex:)`, `var hex: String` and a SwiftUI
  `Color` conversion in `BoardColors.swift`.
- `BoardSettings.styles: [CopperLayer: LayerStyle]` holds a style for each
  layer that has one. `style(of:)` returns the stored style or the default.
- Default colours:

  | Layer | Colour | Name |
  |---|---|---|
  | Top | `#C83737` | red |
  | Layer 2 | `#D8B23A` | gold |
  | Layer 3 | `#3AAE5C` | green |
  | Layer 4 | `#B04AD0` | purple |
  | Layer 5 | `#3AB8C8` | cyan |
  | Bottom | `#3A6FD8` | blue |

- Default types: Top and Bottom are `component`. Inner layers are `plane`,
  because that is the usual 4- and 6-layer stack (ground and power inside).
- A colour or type the user changes is kept even after the layer leaves the
  board. Going from 4 to 2 and back to 4 brings Layer 2's colour back.
- Only styles for layers on the board are saved. Defaults are written too,
  so the file is explicit.

### Layer-count changes

The Layers picker in `BoardPanel` keeps its four choices. From B onward,
content on a layer limits which counts the user can pick. A count is disabled
(greyed, with a help tooltip saying why) when it would drop a layer that
holds content:

- in A, nothing can be on a layer yet, so every count stays enabled;
- B adds parts and texts on Bottom, which blocks 1 layer;
- D adds traces, which block dropping their layer.

`SchematicDocument.canSetLayerCount(_:) -> String?` returns the reason a
count is refused, or nil when it is allowed. A defines it, returning nil for
every count. B and D extend it. The picker calls it.

### Layers section

The Layers section goes under Board in `BoardPanel`. Each copper layer is one
row in an `HStack`: `ColorPicker` (no label, no opacity), then the name, then
the type. Top and Bottom show the type "Component" as plain secondary text.
Inner layers show a menu picker with Plane and Wire.

`SchematicDocument.setLayerColor(_:_:)` and `setLayerType(_:_:)` make the
changes. `setLayerType` refuses `.component` for inner layers, and any change
for Top and Bottom. D also refuses a change to Plane on a layer with traces.

### Start zoom

`CanvasViewport` gets `init(zoom: CGFloat = 1)`, clamped to its zoom range.
`ContentView` creates `boardViewport = CanvasViewport(zoom: 0.25)`. The
offset stays `.zero`, so the board's (0, 0) corner is at the view's top
left, as it is today.

### Rejected alternatives

- **Keying layers by index (1…N).** When going from 2 to 4 layers, Bottom
  would move from index 2 to index 4, and every bottom-side part and trace
  would need renumbering. A named Bottom avoids that.
- **Storing the colour as a SwiftUI `Color`.** `Color` is not `Hashable` in a
  stable way, and doesn't encode to a plain file value. Use `#RRGGBB`.
- **A palette of fixed swatches, like the board colour.** The owner asked for
  a click-to-change patch, which is the system colour well.

## Compatibility / Format Impact

New optional key `board.layerStyles` (version stays 2):

```json
"board": { "layers": 4, "layerStyles": [
  { "layer": "top", "color": "#C83737", "type": "component" },
  { "layer": "inner2", "color": "#D8B23A", "type": "plane" },
  { "layer": "inner3", "color": "#3AAE5C", "type": "wire" },
  { "layer": "bottom", "color": "#3A6FD8", "type": "component" } ] }
```

- It is written for every layer on the board, in stack order.
- A missing entry, or a missing `color` or `type`, takes its default.
- A file without the key opens with all defaults. Older builds ignore it.

New `badPCB` details:

- `layer "inner4" is not on a 4-layer board`
- `layer "middle" must be top, inner2 to inner5, or bottom`
- `layer top type "plane" must be component` (same for `bottom`)
- `layer inner2 type "component" must be plane or wire`
- `layer top color "red" must be #RRGGBB`
- `layer top is listed twice`

## Phases

> Tick boxes in the same commit as the work. `- [~]` = partial, with what
> remains. Fill `Commit:` when a phase lands.

### Phase 1 — Layer model and file format

- [ ] `Canvas/BoardSettings.swift`: add `CopperLayer` (raw string, title,
      `allCases(for count:)`), `LayerType`, `LayerColor` (hex parse/print),
      `LayerStyle`, `BoardSettings.styles`, `copperLayers`,
      `style(of:)` and the default colour table. Rewrite the `layers` comment:
      it is the copper layer count, Top first and Bottom last.
- [ ] `Canvas/BoardColors.swift`: `LayerColor.color` (a SwiftUI `Color`) and
      `init(_ color: Color)` (via `NSColor.usingColorSpace(.sRGB)`, rounding
      to bytes).
- [ ] `Canvas/SchematicModel.swift`: `setLayerColor`, `setLayerType` (with the
      refusals in §3), and `canSetLayerCount` (returns nil for every count in
      this sub-plan).
- [ ] `Document/SchematicFile.swift`: add `Board.layerStyles:
      [LayerStyleEntry]?` (`layer`, `color?`, `type?`). Encode one entry per
      `copperLayers` element. Decode and validate it in `boardSettings()`,
      using the error details above.
- [ ] Tests (`<tests>/BoardLayersTests.swift`): `copperLayers` for 1, 2, 4
      and 6; hex round trip; a save/open round trip of a 4-layer board with a
      changed colour and a Wire inner layer; a file without `layerStyles`
      gets the defaults; every new error detail; 4→2→4 restores Layer 2's
      style.

Acceptance: a 4-layer board with a changed colour and an inner Wire layer
survives a round trip, and each bad entry gives its exact message.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:<tests>/BoardLayersTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — Layers section and start zoom

- [ ] `Canvas/PCBSidebarView.swift`: add `LayersSection` under Board in
      `BoardPanel`, with rows as in §3. Each count in the Layers picker is
      disabled when `canSetLayerCount` returns a reason, with that reason as
      its `.help`.
- [ ] `Canvas/CanvasViewport.swift`: add `init(zoom:)`.
      `ContentView.swift:30`: `CanvasViewport(zoom: 0.25)`.
- [ ] Tests: `CanvasViewport(zoom: 0.25).zoom == 0.25`, and
      `CanvasViewport(zoom: 9).zoom == 3`.

Acceptance: the PCB page opens at "25%". With nothing selected, a 6-layer
board lists Top, Layer 2–5 and Bottom. Inner types switch between Plane and
Wire; Top and Bottom read Component; a colour change shows at once.
  Check: `xcodebuild … test -only-testing:<tests>/BoardLayersTests` → pass (est. 3 min). Then run the app (`run` skill) and switch to PCB: the zoom label reads 25% and the Layers rows are as described (est. 3 min).
Commit: —

## Validation Plan

- Tests: `BoardLayersTests` (model, file round trip, errors, viewport init).
- Runtime proof: run the app, open `be-6502.schematical`, switch to PCB.
  Check that the zoom is 25 % and that Layers shows Top and Bottom. Set 4
  layers, make Layer 3 Wire and Top green, save, reopen: all three kept.
- Doc sync: `docs/file-format.md` § Board. Add the `layerStyles` row, rewrite
  the `layers` row (drop "Stored only; the PCB page draws the top side"), add
  the new error details, and add `layerStyles` to the example file.
- Final gate: run once after E (see plan-16-E Validation Plan).

## Decisions

- Inner layers default to Plane — author's call (owner said "do what you
  think is best", 2026-11-07). It matches the usual 4/6-layer stack; one click
  makes a layer Wire.
- Layers are named identities (Top, Layer 2…5, Bottom), not indices. Bottom
  keeps its content when the count changes.
- A layer that holds content cannot be dropped by a count change. The picker
  disables that count and says why; nothing is deleted or moved
  automatically.
- Layer colours are free sRGB colours from the system colour well, saved as
  `#RRGGBB`.
- The PCB page alone starts at 25 %. The schematic stays at 100 %.

## Corrections

None yet.

## Summary

Low risk: a new value type, one sidebar section and optional file keys. The
contract that B–E rely on is that layers are named (`CopperLayer`) and that
`canSetLayerCount` blocks dropping a layer that holds content.
