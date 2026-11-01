# plan-06-B: Board rectangle, board colour, document/silkscreen marks and texts on the PCB canvas

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-06-A

The PCB canvas draws the board: a rectangle from (0, 0) of the board's size,
filled with the board colour and edged with a line; the canvas outside it
stays dark. Each footprint's body outline, pin-1 mark and reference are
drawn per their mark layer — not at all (Hidden), gray (Document), or in the
silkscreen colour that suits the board colour (Silkscreen). Board texts are
drawn the same way. Selected texts get the selection box.

Outcome: open plan-06-A's `/tmp/p06/rt.schematical` (purple, 120.5 × 60
mm, LED outline hidden, pin 1 document, two texts): the render shows a
purple rectangle on a dark canvas, white silkscreen references, the LED
with no outline and a gray pin-1 mark, both texts; switching the colour to
white turns silkscreen black.

References:

- plan-06-A §4.1 (types), §4.2 (`BoardPlacement` marks), §4.1 text
  geometry (`BoardText.font`, `boardRect`).
- `schematical/schematical/Canvas/PCBCanvasView.swift` (`draw(in:size:)`,
  `drawGrid`).

## Prerequisites

See plan-06-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-06-A complete | `ls planning/completed/plan-06-A-board-model.md` → exists | NOT MET (2026-11-01) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- Board rectangle in the board colour, 0.2 mm edge line in the silkscreen
  colour; canvas outside `Color(white: 0.13)` as today.
- Body outline, pin-1 mark and reference each follow their `MarkLayer`.
- Board texts drawn on their layer, rotated about their anchor.
- A selected text gets the accent-colour selection box (its `boardRect`).

### Non-goals (explicit constraints)

- No sidebar or toolbar change (C, D). Pads and drill holes look as today.
- No change to hit testing of footprints, grid spacing or zoom.
- No model or file change (A owns them).

## 2. Current State

- `PCBCanvasView.draw` (`PCBCanvasView.swift:59`): translate/scale to board
  mm, `drawGrid`, then per chip: outline stroke 0.15 mm
  `Color(white: 0.85)`, pads in `copper`, drill holes in `background`,
  pin-1 mark (0.4 mm circle, `outline` colour), reference (1 mm, min 8
  points on screen, `outline` colour) at the body centre, selection box.
- `.background(Self.background)` is `Color(white: 0.13)`; the grid dots are
  `Color(white: 0.42)`, 2.5 points, every 2.54 mm.
- After plan-06-A: `document.board` (colour, size), `document.texts`,
  `document.selectedTextID`, `BoardPlacement.outline/pin1/reference`.

## 3. Design Overview

One file changes, `PCBCanvasView.swift`, plus a colour table on the A
types. Draw order: grid is drawn **after** the board rectangle so the dots
show on it; then footprints, then texts, then selection boxes.

No correctness risk beyond looks; proven by renders.

Rejected alternative: *board colour as the whole canvas background* — the
owner chose colour inside the board rectangle only.

## 4. Detailed Design

### 4.1 Colours (`Canvas/BoardColors.swift`, new, `import SwiftUI`)

| `BoardColor` | fill (sRGB) | silkscreen |
|---|---|---|
| green | (0.07, 0.36, 0.17) | white |
| black | (0.08, 0.08, 0.08) | white |
| red | (0.62, 0.10, 0.12) | white |
| blue | (0.08, 0.22, 0.55) | white |
| yellow | (0.85, 0.72, 0.12) | black |
| white | (0.93, 0.93, 0.91) | black |
| purple | (0.36, 0.16, 0.50) | white |

White silkscreen = `Color(white: 0.97)`, black = `Color(white: 0.05)`,
document = `Color(white: 0.6)`. `extension BoardColor { var fill: Color;
var silkscreen: Color }`; `MarkLayer.color(on: BoardColor) -> Color?` (nil
for hidden); `TextLayer.color(on:)`.

### 4.2 Drawing (`PCBCanvasView.swift`)

1. Board: `CGRect(x: 0, y: 0, width: board.width, height: board.height)`
   filled with `fill`; edge stroked 0.2 mm in `silkscreen`.
2. Grid as today.
3. Per footprint: outline with `outline.color(on:)`, skipped when hidden;
   pads unchanged; drill holes filled with `Self.background` (you see
   through the hole); pin-1 mark with `pin1.color`; reference with
   `reference.color`.
4. Texts, in `texts` order: `context.drawLayer`, translate to `anchor`,
   rotate by `rotation` degrees, draw
   `Text(text).font(Font(text.font)).foregroundStyle(layer.color)` at
   `.zero`, anchor `.topLeading`. The font is in board mm, so no minimum
   on-screen size (unlike the reference).
5. Selection: the selected chip as today; the selected text's `boardRect`
   with the same 3-point margin and 2-point accent stroke.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Board, marks and texts

- [ ] `Canvas/BoardColors.swift` (new): §4.1.
- [ ] `Canvas/PCBCanvasView.swift`: §4.2 steps 1–5.

Acceptance: renders match §Outcome.
  Check: plan-06-A § Harness, mode `render-b`: open
  `/tmp/p06/rt.schematical` (recreate with plan-06-A's `roundtrip` mode if
  `/tmp` was cleared), render `PCBCanvasView` 900 × 600 at zoom 1 to
  `/tmp/p06/b-purple.png`; set colour white, select the 90° text, render
  `/tmp/p06/b-white.png`. Inspect both: purple rectangle with a white edge
  on a dark canvas; LED body outline absent, its pin-1 dot gray; references
  white on purple and black on white; the 90° text reads top-to-bottom and
  is gray; the selected text has a blue box (est. 5 min).
  Check: harness prints the sRGB of the pixel at the board's centre in
  `b-purple.png` (away from parts) → within 0.02 of (0.36, 0.16, 0.50)
  (est. 1 min).
  Check: plan-06-A §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **`
  (est. 1 min).
Commit: —

## Validation Plan

- Tests: none in code (no Swift test target); renders above.
- Coverage check: the renders exercise every `MarkLayer` value (LED
  outline hidden, LED pin 1 document, others silkscreen and the default
  document outline), both text layers, both silkscreen colours.
- Runtime proof: the two PNGs above, plus `b-green.png` of
  `test.schematical` (defaults: gray outlines, white pin-1 marks and
  references on green).
- Doc sync: none (no format change).
- Final gate (once): the `xcodebuild` (est. 1 min) + the renders (est.
  5 min). Backend untouched.

## Decisions

- Board colour fills only the board rectangle; the edge is drawn — owner,
  2026-11-01.
- Silkscreen white on green, black, red, blue, purple; black on white and
  yellow — owner, 2026-11-01.
- Hidden = not drawn; Document = gray; Silkscreen = the colour above —
  owner, 2026-11-01.
- Author's choices: the fill colours in §4.1; document gray
  `Color(white: 0.6)`; the board edge uses the silkscreen colour, 0.2 mm;
  drill holes show the dark canvas colour; texts have no minimum on-screen
  size.

## Corrections

## Summary

A drawing-only change in one view; the risk is visual, checked by renders
covering every layer value and both silkscreen colours.
