# plan-28-B: Layers on the painter, and the export scene

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-28-A

Moves `WireLayer`, `BusLayer` and `PortLayer` onto `SchematicPainter` (same
pixel-identity gate as A), then adds `SchematicScene` — a value snapshot of the
document that paints itself, in the canvas's own draw order, through any
painter in the export state (nothing selected or hovered, no simulation) — and
`BoundsPainter`, a painter that measures where ink lands. The behavioral outcome
of B: **`SchematicScene(document).paint(into:)` draws a whole schematic through
any `SchematicPainter`, and `BoundsPainter` returns its exact ink rectangle.**
C writes SVG from it; D writes PNG/JPG and adds the menu.

References:

- `planning/plan-28-A-export-painter.md` (archived in `planning/completed/` once
  done) — the design of `Ink`, `PaintedText`, `SchematicPainter`, `ExportPalette`
  and the harness gate. Read § 3 first; B uses all of it.
- `schematical/schematical/Canvas/SchematicCanvasView.swift:196` (`worldLayer`) and
  `:845–1025` (`WireLayer`, `BusLayer`, `PortLayer`).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-28-A complete | `ls planning/completed/plan-28-A-* \| wc -l` → `1` | MET (2026-11-07 at 5bc1592: 1) |
| The painter exists | `rg -c 'protocol SchematicPainter' schematical/schematical/Canvas/SchematicPainter.swift` → `1` | MET (2026-11-07 at 5bc1592: 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `WireLayer`, `BusLayer`, `PortLayer` expose
  `static func paint<P: SchematicPainter>(_ painter: inout P, …)` and their bodies
  are `Canvas` closures that wrap `ScreenPainter`. No `GraphicsContext`/`with: .color(`
  remains in those three structs' drawing: `awk 'NR>=845' SchematicCanvasView.swift | rg -c 'context\.(fill|stroke|draw)\(|with: \.color\('` → no match.
- The four A gate images are still `cmp`-identical after the move (against a baseline taken at B's start).
- `Canvas/SchematicExportScene.swift` defines `SchematicScene` (init from a
  `SchematicDocument`, `isEmpty`, `paint(into:)`) and `BoundsPainter`.
- `SchematicScene.paint` has a `switch chip.symbol` with one case per `PartSymbol`
  (12) and **no `default`**, so a new symbol (plan-27-J's `.tactileSwitch`) fails to
  compile until it is exportable.

### Non-goals (explicit constraints)

- No visible change in the app (layer geometry, widths, dashes, hover/selection
  tints, simulation wire styles are as before).
- `GridBackground` and `PinMarkerLayer` stay on `GraphicsContext`; they are screen-only.
- No SVG or raster writing yet (C, D). No menu yet.
- The export never contains selection, hover, tool previews or play-mode state.

## 2. Current State

- `WireLayer` (`SchematicCanvasView.swift:845`) strokes each `RoutedWire.points` in a
  style by `WireStyle` (`.normal` primary 1.5, `.value(Color)`, `.floating` dashed
  secondary, `.shorted` red, `.toggling` secondary + green dashes), a 6 pt
  accent underlay when hovered, 2.5 pt accent when selected, junction dots (radius
  2.5, 4 selected) and the dashed preview. It draws after
  `context.shiftToWorld(area)`.
- `BusLayer` (`:922`) strokes bus lines `Bus.lineWidth` (6) wide with butt ends,
  fills a square per `.corner` joint and a dot per `.tee`, and a dashed preview.
- `PortLayer` (`:981`) fills each port body with `controlBackgroundColor`, strokes its
  outline and pin stubs, and draws each pin's wire name; on a horizontal line it
  rotates the text with `var turned = context; turned.translateBy; turned.rotate(by: .degrees(-90))`.
- The document exposes what the scene needs: `chips`, `buses` (`SchematicModel.swift:957,961`),
  `wires`, `junctions`, `path(forWire:)` (`:2206`), and `Junction.point`.
- Draw order in the live canvas (`worldLayer`): grid, `BusLayer`, `PortLayer`,
  `WireLayer`, chips in `document.chips` order each at `chip.viewOrigin`, pin
  markers. The scene repeats it minus grid and markers.
- `PlacedChip` is a `nonisolated struct` carrying everything a drawer reads
  (`switchesOn`, `color`, `ohms`, `gender`, `footprintChoice`, `isFlipped`).
- Each symbol view's Canvas clips to its frame (`.frame(width: layout.size.width + 2 * margin, height: …)`),
  so no chip ink lies outside its frame; the bounds test uses that.

### Measured populations

| What | Count | Command |
|---|---|---|
| Draw-call lines in the three layers | 20 | `awk 'NR>=845' schematical/schematical/Canvas/SchematicCanvasView.swift \| rg -c '(context\|turned)\.(fill\|stroke\|draw)\('` |
| `PartSymbol` cases the scene's switch must cover | 12 | `awk '/enum PartSymbol/,/^}/' schematical/schematical/Canvas/SchematicModel.swift \| rg -c '^    case '` |
| `case .…` arms in the live canvas's chip switch | 8 | `awk '/ForEach\(document.chips\)/,/offset\(x: chip.viewOrigin/' schematical/schematical/Canvas/SchematicCanvasView.swift \| rg -c '^\s+case \.'` (`.block, .oscillator` and `.vcc, .ground, .pullUp, .pullDown` are one arm each, so 8 arms cover 12 cases) |

### Verified properties

| Claim | How verified |
|---|---|
| A painter copy that is transformed must not lose accumulated results | `PortLayer` already copies the context to rotate it; a `BoundsPainter`/SVG painter that accumulated into a plain struct field would drop everything drawn in the copy. Therefore every accumulating painter keeps its results in a `final class` box that copies share. |
| `Bus.body(of:)`, `Bus.line(_:)`, `Bus.pinWires(of:)`, `Bus.point(on:along:out:)` give the port geometry in world units | `PortLayer.body` (`:989`) calls them with no view-specific input |

## 3. Design Overview

### 3.1 Layers

Each layer keeps its parameters; `body` becomes
`Canvas { context, _ in var shifted = context; shifted.shiftToWorld(area); var painter = ScreenPainter(shifted); Self.paint(&painter, …) }`.
In `paint`, colors become `Ink` (`.accent.opacity(0.35)`, `.primary`, `.secondary`,
`.paper`; `WireStyle.value(color)` → `Ink(role: .color(color))`; `.red`/`.green` →
`Ink(role: .color(.red))`/`.color(.green)`; the refused bus preview
`Ink(role: .color(Color.red.opacity(0.6)))`) and text becomes
`PaintedText(wire.name, size: 10, weight: .medium, monospaced: true)`. The rotated
label: `var turned = painter; turned.translateBy(x: label.x, y: label.y); turned.rotate(by: .degrees(-90)); turned.draw(text, at: .zero, anchor: .center)`.

### 3.2 SchematicScene

```swift
nonisolated struct SchematicScene {
    var chips: [PlacedChip]
    var buses: [Bus]
    var routes: [RoutedWire]      // all .normal
    var junctions: [JunctionDot]  // none selected
    @MainActor init(_ document: SchematicDocument)
    var isEmpty: Bool { chips.isEmpty && buses.isEmpty && routes.isEmpty }
    func paint<P: SchematicPainter>(into painter: inout P)
}
```
`paint` order: `BusLayer.paint`, `PortLayer.paint`, `WireLayer.paint` (no selection, no hover, no preview),
then per chip a copy of the painter translated by `chip.viewOrigin` and the symbol's
`paint` with `isSelected: false, isHovered: false`, `level: 0`, `levels: []`,
`frame: nil`, `backlight: 0`. Switch arms: `.block, .oscillator` → `ChipBlockView`;
`.barrelJack`, `.header`, `.diode`, `.vcc/.ground/.pullUp/.pullDown` → `SupplySymbolView`,
`.dipSwitch`, `.sevenSegment`, `.characterLCD`. The scene is a value, so D can paint it
inside a `Canvas` closure (which is not main-actor isolated) without touching the document.

### 3.3 BoundsPainter

Accumulates a rectangle in a shared `final class` box; ignores calls whose export
alpha is nil (`ExportPalette.alpha(for: ink) == nil`); applies the current
`CGAffineTransform` (`translateBy`, `rotate`). Per call:

- `fill`: the path's `boundingRect`.
- `stroke`: `boundingRect` outset by `style.lineWidth / 2`.
- `draw(text)`: a box `width = string.count × size × (monospaced ? 0.6 : 0.55)`,
  `height = size × 1.2`, placed by `anchor` (`UnitPoint`: x from `anchor.x`, y from `anchor.y`)
  at `point`; its four corners go through the transform. These are estimates; the
  export adds 12 pt of padding (C), which is larger than the estimate error on any
  label in the app (the longest are 7-character designators at 9–11 pt).

`bounds: CGRect?` is `nil` for an empty scene. Because it ignores ink the export
palette drops, the LCD dot matrix, chip body fills and the 7-segment face fill do
not widen the rectangle.

Why measure the ink instead of summing view frames: each symbol view's frame has a
30 pt label margin on both sides (`ChipLayout.labelMargin`, `SchematicModel.swift:171`)
that holds no ink, so frames would leave a 30 pt blank border, and a new symbol
needs no size helper.

Rejected: a `size(of:)` helper per view (more API and drift; the margin problem above).

### 3.4 Correctness risk

The layer move is the same 20-line mechanical edit as A, gated the same way
(byte-identical PNGs). `SchematicScene` is new code with no existing behavior to
break; its risk is a symbol drawn with the wrong state, covered by the recorder test below.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>`; never delete it.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Wire, bus and port layers on the painter

- [x] Re-run the Prerequisites commands; record outputs in the table. (both 1; the layers still had 20 draw-call lines, as measured)
- [x] Baseline: on the tree as it is (A landed, so it renders the same as before plan-28), `scripts/harness/build.sh scripts/harness/painter-gate.swift /tmp/p28-hB && /tmp/p28-hB/H.app/Contents/MacOS/harness /tmp/p28-baseB` → four `wrote …` lines. (taken with `/tmp/p28-scripts/gate.sh /tmp/p28-base /tmp/p28-baseB`, which builds the harness as A does and also `cmp`s against A's baseline: four `same` lines, so B starts from the pre-plan-28 pixels)
- [x] `SchematicCanvasView.swift`: `WireLayer`, `BusLayer`, `PortLayer` → static `paint`
      plus Canvas wrappers per § 3.1. `GridBackground`/`PinMarkerLayer` untouched.
- [x] Re-run the harness: `scripts/harness/build.sh scripts/harness/painter-gate.swift /tmp/p28-h && /tmp/p28-h/H.app/Contents/MacOS/harness /tmp/p28-new` and `cmp` against `/tmp/p28-baseB`. → `same symbols`, `same lit`, `same be-6502`, `same test.schematical`; the `awk … | rg -c` check → no output (exit 1) after the fix in Corrections; gate re-run after that fix: four `same` lines.

Acceptance: all four A images are `cmp`-identical; no `context.` draw call remains in the three layers.
  Check: `for f in symbols lit be-6502 test.schematical; do cmp /tmp/p28-baseB/$f.png /tmp/p28-new/$f.png && echo same $f; done` → four `same` lines; `awk 'NR>=845' schematical/schematical/Canvas/SchematicCanvasView.swift | rg -c 'context\.(fill|stroke|draw)\(|with: \.color\('` → no output (est. 10 min, same reason as A Phase 3).
Commit: —

### Phase 2 — SchematicScene and BoundsPainter

- [ ] `Canvas/SchematicExportScene.swift` (new): `SchematicScene` and `BoundsPainter` per § 3.2–3.3.
- [ ] `schematicalTests/SchematicSceneTests.swift` (new), using `TestSupport.newDocument()` and `TestSupport.place`:
      - `BoundsPainter`: stroke of `Path(CGRect(x: 10, y: 20, width: 30, height: 40))` at width 2 → bounds `(9, 19, 32, 42)`; after `translateBy(x: 5, y: 5)` → `(14, 24, 32, 42)`; a fill with `Ink.paper` adds nothing (bounds nil); centered monospaced 10 pt text "ABCD" at (100, 50) → `(88, 44, 24, 12)`; the same text after `rotate(by: .degrees(-90))` about its point has width 12 and height 24.
      - A scene of one `SN74HC00N`: `isEmpty` false; bounds contain `chip.hitRect` and lie inside `CGRect(x: chip.viewOrigin.x, y: chip.viewOrigin.y, width: chip.layout.size.width + 2 * ChipLayout.labelMargin, height: chip.layout.size.height)`.
      - A new document → `isEmpty` true; `SchematicScene(doc).paint` into a `BoundsPainter` → bounds nil.
      - Export state: a recording painter (test-local type conforming to `SchematicPainter`, results in a class box) paints a scene with one of each of the 12 symbols (names in plan-28-A § 3.4 item 1) and one wire; the recorded inks contain no `.accent`, no `.color`, no `.lamp`, and at least one `.secondary`; with the DIP lever 1 on, a `.switchOn` and `.knobOn` appear, and with it off, neither.
      - Order: the recorder sees a bus stroke before a wire stroke before any chip text.

Acceptance: the new tests pass, and the full set of A and B tests still pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicSceneTests -only-testing:schematicalTests/SchematicPainterTests CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `SchematicSceneTests` (Phase 2) incl. the negative case (no accent/color/lamp ink in an export).
- Coverage check: the recorder test paints all 12 `PartSymbol` kinds, so every arm of the scene's switch runs.
- Runtime proof: Phase 1's `cmp` for the layers; the scene's end-to-end output is proven in C and D.
- Doc sync: none in B.
- Final gate (ONCE, after Phase 2): Phase 1's image check plus Phase 2's `xcodebuild … test` command (est. 16 min together).

## Decisions

- Ink bounds are measured by a painter, not summed from view frames (§ 3.3) — author, 2026-11-07. Settles export size and padding.
- The scene is a value snapshot, painted off the document (§ 3.2) — author, 2026-11-07, so D's `ImageRenderer` Canvas never touches main-actor state.
- The chip switch has no `default` (§ 1) — author, 2026-11-07; makes adding a symbol without an export a compile error. Coordinated with plan-27-J (see plan-28-A § Decisions).

## Corrections

- Phase 1: the first layer move wrote the screen-only inks as `with: .color(color)` (the `Ink.color` shorthand), which the acceptance regex `with: \.color\(` also matches (3 hits) although no SwiftUI `Color` shading is left. Changed those to `Ink(role: .color(…))`, as § 3.1 writes them, rather than loosen the regex; the gate was re-run on the final code.

## Summary

Risk is split: the layer move is another mechanical edit under the pixel gate;
the new code is small and test-covered. Left untouched: grid, pin markers, PCB,
the eight symbol views (done in A), and everything user-facing.
