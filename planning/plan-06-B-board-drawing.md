# plan-06-B: No simulation on the PCB page; board rectangle, colour, marks and texts on the PCB canvas

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-06-A

First, the simulation leaves the PCB page: switching to PCB stops a
running simulation, and the PCB toolbar has no Run/Stop button, speed menu
or reached-speed label. Then the PCB canvas draws the board: a rectangle from (0, 0) of the board's size,
filled with the board colour and edged with a line; the canvas outside it
stays dark. Each footprint's body outline, pin-1 mark and reference are
drawn per their mark layer — not at all (Hidden), gray (Document), or in the
silkscreen colour that suits the board colour (Silkscreen). Board texts are
drawn the same way. Selected texts get the selection box.

Outcome: open plan-06-A's `/tmp/p06/rt.schematical` (purple, 120.5 × 60
mm, LED outline hidden, pin 1 document, two texts): the render shows a
purple rectangle on a dark canvas, white silkscreen references, the LED
with no outline and a gray pin-1 mark, both texts; switching the colour to
white turns silkscreen black. And: run the simulation on the Schematic
page, switch to PCB — it stops, and no Run/Stop, speed or reached-speed
control is on the toolbar.

References:

- plan-06-A §4.1 (types), §4.2 (`BoardPlacement` marks), §4.1 text
  geometry (`BoardText.font`, `boardRect`).
- `schematical/schematical/Canvas/PCBCanvasView.swift` (`draw(in:size:)`,
  `drawGrid`).

## Prerequisites

See plan-06-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-06-A complete | `ls planning/completed/plan-06-A-board-model.md` → exists | MET (2026-11-01, 9d707ad) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- Switching the page to PCB stops a running simulation
  (`SimulationController.stop()`); the PCB toolbar shows no Run/Stop
  button, speed menu or reached-speed label, and no "Simulation stopped"
  error banner. The simulation therefore never runs on the PCB page.
- Board rectangle in the board colour, 0.2 mm edge line in the silkscreen
  colour; canvas outside `Color(white: 0.13)` as today.
- Body outline, pin-1 mark and reference each follow their `MarkLayer`.
- Board texts drawn on their layer, rotated about their anchor.
- A selected text gets the accent-colour selection box (its `boardRect`).

### Non-goals (explicit constraints)

- The Schematic page's Run/Stop, ⌘R, speed menu, reached-speed label and
  error banner are unchanged. Coming back to Schematic does not restart the
  simulation.
- No sidebar (C) or PCB tool (D) change. Pads and drill holes look as today.
- No change to hit testing of footprints, grid spacing or zoom.
- No model or file change (A owns them).

## 2. Current State

- `ContentView.toolbar` `.navigation` (after plan-05-C): file controls,
  the Run/Stop button (⌘R; starting resets the tool and selection), the
  speed `Menu`, `ReachedSpeedLabel` — shown on both pages. The
  `simulation.errorMessage` banner (`.overlay(alignment: .bottom)`) shows on
  both pages. `canRotate` is false while running.
- `PCBCanvasView` takes `simulation` only to read `isRunning`
  (`PCBCanvasView.swift:16`, used at `:137`, `:142`: while running every
  drag pans and nothing is selected).
- `SimulationController.stop()` (`Simulation/Simulation.swift:253`) stops
  the engine and clears run state; it leaves `errorMessage` as it was.
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

Phase 1 (`ContentView.swift`, `PCBCanvasView.swift`) takes the simulation
off the PCB page. Because it can no longer run there, the PCB's "while
running" rules are removed rather than kept as dead code: `PCBCanvasView`
loses its `simulation` parameter, and `canRotate` loses its running check.
plan-06-C and plan-06-D are written against that (no running checks).

Phase 2 changes `PCBCanvasView.swift` drawing, plus a colour table on the A
types. Draw order: grid is drawn **after** the board rectangle so the dots
show on it; then footprints, then texts, then selection boxes.

No correctness risk beyond looks; proven by renders.

Rejected alternative: *board colour as the whole canvas background* — the
owner chose colour inside the board rectangle only.

## 4. Detailed Design

### 4.0 Simulation off the PCB page (`ContentView.swift`, `PCBCanvasView.swift`)

- Wrap the Run/Stop `ToolbarItem`, the speed `Menu` item and the
  `ReachedSpeedLabel` item in `if page == .schematic { … }`. The ⌘R
  shortcut goes with the button, so ⌘R does nothing on the PCB page.
- The error banner overlay shows only when `page == .schematic`.
- `.onChange(of: page) { _, newPage in if newPage == .pcb { simulation.stop() } }`
  (`stop()` on a stopped simulation only resets cleared state, so no
  `isRunning` guard is needed — read: every line of `stop()` is an
  assignment or `engine?.…`).
- `PCBCanvasView`: drop `simulation` and `isRunning`; `handleChanged`
  always selects/moves on a footprint hit. `ContentView` builds it as
  `PCBCanvasView(document:viewport:)`.
- `canRotate`: drop `!simulation.isRunning`.

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
   rotate by `rotation` degrees, scale by `text.drawScale` (plan-06-A
   Corrections 1: the font is laid out at 100 points), draw
   `Text(text).font(Font(text.font)).foregroundStyle(layer.color)` at
   `.zero`, anchor `.topLeading`. The text scales with the board, so no
   minimum on-screen size (unlike the reference).
5. Selection: the selected chip as today; the selected text's `boardRect`
   with the same 3-point margin and 2-point accent stroke.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Simulation off the PCB page

- [x] `ContentView.swift`: §4.0 toolbar, banner, `onChange`, `canRotate`.
      `harness window-b1`: `schematic: running true` (toolbar … `Stop` …),
      `pcb: running false` (toolbar `Open`, `Save`, `Rotate`, zoom — no
      Run/Stop, one hosting item fewer: the speed menu), `back: running false`
      (`Run`); `b1-schematic.png`, `b1-pcb.png`, `b1-back.png`.
- [x] `Canvas/PCBCanvasView.swift`: drop `simulation` / `isRunning` (§4.0).
      `grep -c 'isRunning\|simulation' …/PCBCanvasView.swift` → 0;
      `xcodebuild` → `** BUILD SUCCEEDED **`.
- [x] Test affordance (added): `SchematicSessions.allSessions` (read-only),
      so the harness reaches a live window's document and simulation; the
      page is switched through the toolbar's own `NSSegmentedControl` and Run
      through its toolbar item (Corrections 1).

Acceptance: switching to PCB stops the simulation; the PCB toolbar has no
simulation controls; the Schematic toolbar is unchanged.
  Check: plan-06-A § Harness, mode `window-b1`: host `ContentView` in an
  offscreen 1200 × 800 `NSWindow` (plan-03-D's method: snapshot the frame
  view with `bitmapImageRepForCachingDisplay` + `cacheDisplay`); open
  `test.schematical`, start the simulation on Schematic →
  `simulation.isRunning` `true`, capture `b1-schematic.png` (Stop button,
  speed menu present); set the page to PCB → `isRunning` `false`, capture
  `b1-pcb.png` (no Run/Stop, no speed menu); back to Schematic →
  `isRunning` still `false`, `b1-back.png` shows Run (est. 5 min).
  Check: `grep -c 'isRunning\|simulation' schematical/schematical/Canvas/PCBCanvasView.swift`
  → 0 (est. 1 s).
  Check: plan-06-A §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **`
  (est. 1 min).
Commit: 285c403

### Phase 2 — Board, marks and texts

- [x] `Canvas/BoardColors.swift` (new): §4.1. `harness render-b`: pixel at
      board (40, 25) mm in `b-purple.png` → sRGB (0.361, 0.161, 0.502); in
      `b-green.png` → (0.071, 0.361, 0.169).
- [x] `Canvas/PCBCanvasView.swift`: §4.2 steps 1–5 (selection boxes drawn
      after texts, so on top). Inspected `b-purple.png`: purple board, white
      edge, dark canvas outside; U3 (LED) no outline, gray pin-1 dot; U2/U3
      references white; "REV A" white; the 90° document text gray, reading
      top to bottom. `b-white.png`: references and "REV A" black, the 90°
      text in a blue box that fits it. `b-green.png` (`test.schematical`):
      gray outline, white pin-1 dots and references on green. `b-*-fit.png`
      (zoom 0.35) show the whole 120.5 × 60 board. `xcodebuild` → `** BUILD
      SUCCEEDED **`.

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
- Runtime proof: Phase 1's three window captures; the two PNGs above, plus `b-green.png` of
  `test.schematical` (defaults: gray outlines, white pin-1 marks and
  references on green).
- Doc sync: none (no format change).
- Final gate (once): the `xcodebuild` (est. 1 min) + `window-b1` and the
  renders (est. 10 min). Backend untouched.
- Results (2026-11-01): `xcodebuild` → `** BUILD SUCCEEDED **`;
  `window-b1`, `render-b` as ticked above.

## Decisions

- No Run/Stop interface on the PCB page; switching to PCB stops the
  simulation — owner, 2026-11-01. Author: the speed menu, reached-speed
  label and error banner go with it (they belong to the simulation);
  returning to Schematic does not restart it; the PCB's "while running"
  rules are removed since they can no longer apply.
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

2. The colour check sampled "the pixel at the board's centre", but at zoom 1
   a 900 × 600 render shows only 45 × 30 mm, so the centre of the 120.5 × 60
   board, (60.25, 30) mm, is off-image. Sampled board (40, 25) mm at zoom 1
   (clear of parts and texts), and the true centre in a zoom-0.35 render
   (`b-purple-fit.png`): both (0.361, 0.161, 0.502). The PNG is tagged
   sRGB; the bytes are read as they are (converting `colorAt` to `.sRGB`
   gave a wrong (0.440, 0.242, 0.575) — a harness bug, raw bytes
   `[92, 41, 128]`).

1. Phase 1's window check needs to reach the window's document and
   simulation, which `ContentView` keeps in private `@State`. Added the
   read-only `SchematicSessions.allSessions` (`Memory/MemoryWindow.swift`);
   the harness hosts `ContentView` in an `NSHostingController` with
   `sceneBridgingOptions = [.toolbars, .title]` so the toolbar is real, and
   presses the toolbar's own controls.

## Summary

A drawing-only change in one view; the risk is visual, checked by renders
covering every layer value and both silkscreen colours.
