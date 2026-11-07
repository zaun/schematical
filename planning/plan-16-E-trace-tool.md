# plan-16-E: Trace tool and sidebar

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-16-D

The PCB toolbar gets a **Trace** tool and, right after the tool picker, a
**trace layer menu**, which works like the schematic's supply-voltage menu.
With Trace, a drag from one point to another draws a 10 mil, 45° trace on the
chosen layer, in that layer's colour. The trace joins anything it touches on
that layer. Selecting a trace shows a sidebar with:

- Width (5–100 mil);
- Layer;
- the pads its connected run touches;
- Delete.

References:

- `schematical/schematical/Canvas/PCBCanvasView.swift:PCBTool`, `Interaction`, `begin(at:)`, `draw(in:size:)`
- `schematical/schematical/Canvas/SchematicCanvasView.swift` (Line tool: drag preview, landing highlight)
- `schematical/schematical/ContentView.swift` (supply `Menu(supplyVoltage)`, the PCB tool `Picker`)
- `schematical/schematical/Canvas/PCBSidebarView.swift:TextPanel` (field, stepper and Delete precedent)
- plan-15 (PCB selection set, Shift toggling, group drag); plan-16-C (`BoardVisibility`); plan-16-D (trace model)

## Prerequisites

See plan-16-A § Prerequisites. plan-16-D must be complete:
`rg -n 'func addTrace' schematical/schematical/Canvas/SchematicModel.swift` → 1 match.

## 1. Goal

- Picking Trace and dragging on the PCB draws a 45° trace on the menu's
  layer. While dragging, a live preview shows the route. On release the trace
  is added and joined.
- Clicking a trace with Select selects it, its joined run is highlighted, and
  the sidebar edits its width and layer.
- Shift-click and group drag (plan-15) include traces.

### Non-goals

- No click-by-click routing, vias, push-and-shove, or per-segment editing.
- No trace width control on the toolbar. New traces are 10 mil; the sidebar
  changes them.
- The Rotate button does not rotate traces.

## 2. Current State

- `PCBTool` has `select` and `text` (`PCBCanvasView.swift`,
  `enum PCBTool`): `rg -n 'case select|case text' schematical/schematical/Canvas/PCBCanvasView.swift | wc -l` → 2.
- After plan-15 the PCB canvas reads Shift at the start of a press. Selection
  is a typed set of footprint and text items, and a group drag captures
  starting geometry and moves every selected item by one snapped delta.
- The schematic Line tool:
  - is a drag, from `.connecting(from:cursor:)` to release;
  - draws a dashed preview route while dragging;
  - highlights the landing point;
  - stays the active tool after each line (`SchematicCanvasView.swift:333-345`,
    `244-263`).
- The supply menu is `Menu(supplyVoltage) { Button(name) { supplyVoltage =
  name; tool = … .vcc } }`, right after the schematic tool picker
  (`ContentView.swift:231-239`).
- `TextPanel` edits a mil value with a `TextField`, a "mil" label and a
  `Stepper(step: 5)`, and deletes with a destructive button on ⌘⌫
  (`PCBSidebarView.swift:98-190`).

### Measured populations

| What | Count | Command |
|---|---|---|
| PCB tools today | 2 | `rg -n 'case select\|case text' schematical/schematical/Canvas/PCBCanvasView.swift \| wc -l` → 2 |
| PCB `Interaction` cases today | 3 | `rg -n '^        case (move\|moveText\|pan)' schematical/schematical/Canvas/PCBCanvasView.swift \| wc -l` → 3 |

### Verified properties

- `NSEvent.modifierFlags` gives the Option key state during a SwiftUI
  `DragGesture` update. It is AppKit global state, readable on the main
  actor in `onChanged`. plan-15 reads Shift the same way.
- The SF Symbol `line.diagonal` exists from SF Symbols 2, which the app's
  macOS target is newer than.

## 3. Design

### Tool and layer menu

- **`PCBTool.trace`.** Title "Trace", symbol `line.diagonal`.
- **State.** `ContentView` adds `@State private var traceLayer: CopperLayer =
  .top`.
- **Menu.** `Menu(traceLayerTitle)`, placed right after the PCB tool picker,
  lists every board layer whose type is not Plane (Top, the Wire inner
  layers, Bottom). Picking one:
  - sets `traceLayer`;
  - sets `pcbTool = .trace`;
  - removes `.copper(layer)` from `hiddenLayers`, so the layer you draw on
    is shown.

  Help text: "Layer for the Trace tool".
- **Fallback.** `onChange` of the board's layers or styles: when `traceLayer`
  is no longer on the board or has become Plane, it goes back to `.top`.

### Drawing a trace (Trace tool)

The canvas gets `traceLayer`, and a new
`Interaction.drawTrace(start: CGPoint, cursor: CGPoint)`.

- **Press.** `start = document.traceAnchor(at: p, layer: traceLayer,
  tolerance: 6 / scale)`.
- **Drag.** `cursor` follows the pointer. The preview is
  `TraceRoute.route(from: start, to: traceAnchor(cursor), diagonalFirst:
  optionDown)`, stroked in the layer colour at 0.6 opacity, at the trace
  width (`10 mil`), with round caps and joins. The landing point gets the
  Line tool's green ring.
- **Release.** `document.addTrace(from: start, to: anchor, layer:
  traceLayer, diagonalFirst: optionDown)`. The new trace becomes the sole
  selection. If its layer is hidden, it is made visible (the canvas calls an
  `onShowLayer` closure that ContentView maps to `hiddenLayers.remove`).
  A zero-length release adds nothing.
- **After.** The tool stays Trace, as the schematic Line tool does.
- **While idle.** Hovering in Trace mode shows the green ring at the anchor
  the press would use.

### Rendering traces

C's draw order step 2 ("its traces"):

- **Each trace.** For each visible copper layer from Bottom to Top, each
  trace is stroked along its points in its layer's colour, at `widthMM`,
  with round caps and joins.
- **Selected trace.** An accent stroke `widthMM + 4 / scale` wide, drawn
  under it.
- **Its joined run.** The other traces of `connectedRun` get the same
  underlay at 0.35 opacity, so the user sees what is joined.

### Selection (Select tool, with plan-15)

- **Item.** plan-15's PCB selection item type gets a trace case.
- **Hit order.** Hit testing tries texts, then traces
  (`trace(at:tolerance:visible:)`), then footprints, Top before Bottom
  within each. A trace drawn across a part's box can then still be clicked.
- **Shift.** Shift-click toggles a trace like any other item.
- **Group drag.** It captures each selected trace's points and calls
  `moveTraces(ids, by: snappedDelta)` once per update from the starting
  points. Joins with traces that aren't selected are recomputed on release.
  Traces that aren't selected stay where they are, even when a selected
  footprint moves off their ends.
- **Multi-selection list.** It shows a trace row as "Trace, <layer>,
  <width> mil".
- **Rotate.** Stays disabled when the sole selection is a trace.

### Trace sidebar (`TracePanel`)

Shown when a trace is the sole selection.

- **Header.** "Trace", in the `TextPanel` header style.
- **Width.** `TextField` + "mil" + `Stepper(in: 5...100, step: 5)`, calling
  `setTraceWidth`. Typed values are clamped. Width is always in mil,
  whatever the board units, as text size is.
- **Layer.** `Picker(.menu)` over the non-Plane board layers, calling
  `setTraceLayer`. The trace's run on the old layer is not moved with it:
  only this trace changes layer, then it joins the new layer.
- **Connects.** Read-only. `pads(touching: connectedRun(of:))` joined with
  ", ", or "Nothing" when empty, and "Joined to N other traces" when N > 0.
- **Delete Trace.** Destructive, ⌘⌫, as on texts.

### Rejected alternatives

- **Click per bend.** The owner asked for the trace tool to work like the
  schematic line tool, which is a drag. More bends come from drawing more
  traces, which join at their shared ends.
- **Changing the layer of the whole run.** It silently moves traces the
  user didn't select. Shift-select (plan-15) and set each one when that's
  what is wanted.

## Phases

### Phase 1 — Trace tool and rendering

- [ ] `Canvas/PCBCanvasView.swift`: `PCBTool.trace`; `traceLayer` and
      `onShowLayer` properties; the `.drawTrace` interaction, preview and
      landing ring; trace rendering and the selection underlays (§3). Update
      the doc comment to cover the Trace tool.
- [ ] `ContentView.swift`: `traceLayer` state; the layer menu after the PCB
      tool picker; the fallback `onChange`; pass `traceLayer` and
      `onShowLayer` to the canvas.
- [ ] Tests (`<tests>/TraceToolTests.swift`), driving `PCBCanvasView.handle`
      the way plan-15's gesture tests do:
  - a Trace drag from (2.54, 2.54) to (10.16, 5.08) adds one Top trace with
    points `[(2.54,2.54), (7.62,2.54), (10.16,5.08)]` and selects it;
  - a zero-length press adds none;
  - a drag on Layer 3 (Wire) while Layer 3 is hidden calls `onShowLayer`.

Acceptance: a drag adds the routed trace on the menu's layer, and the tool
stays Trace.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:<tests>/TraceToolTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — Selection and trace sidebar

- [ ] `Canvas/SchematicModel.swift` and `PCBCanvasView.swift`: the trace
      selection item; hit order text → trace → footprint; Shift toggling;
      `moveTraces` in the group drag. `replaceContents` and `deleteTrace`
      drop stale trace IDs.
- [ ] `Canvas/PCBSidebarView.swift`: `TracePanel` (§3); a trace row in the
      multi-selection list.
- [ ] `ContentView.swift`: Rotate stays disabled for a trace-only selection.
- [ ] Tests (`TraceToolTests`):
  - clicking a trace selects it over the footprint under it;
  - Shift-click adds a trace to a footprint selection;
  - a group drag moves the selected trace by the snapped delta and re-joins
    it;
  - `setTraceWidth(3)` clamps to 5 and `setTraceWidth(250)` to 100;
  - the Layer picker lists no Plane layer.

Acceptance: a selected trace's width and layer change from the sidebar,
Connects lists the touched pads, and Delete removes it.
  Check: `xcodebuild … test -only-testing:<tests>/TraceToolTests` → all pass (est. 3 min).
Commit: —

## Validation Plan

- Tests: `TraceToolTests` (tool, selection, sidebar model calls), on top of
  D's `TraceRouteTests` and `TraceJoinTests`.
- Runtime proof: run the app (`run` skill), open `be-6502.schematical`, then
  on the PCB page:
  1. Trace on Top from a U1 pad to a U2 pad: a red 10 mil trace with a 45°
     bend, and Connects lists both pads.
  2. Hold Option for a second trace: the diagonal comes first.
  3. Draw across the first trace: they join (the run highlight shows both).
  4. Set the second trace to Bottom: its colour changes and it no longer
     joins.
  5. Width 50: it gets thicker.
  6. Uncheck Top in View: the Top traces go.
  7. Save, reopen: everything is kept.
- Doc sync:
  - `docs/file-format.md` (everything from A, B and D, checked again as one
    file);
  - the `PCBCanvasView`, `PCBSidebarView` and `BoardSettings` doc comments;
  - the `ContentView` toolbar comments.
- Final gate (run once, after this phase):
  - `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO`
    → every test passes, including all plan-15 and plan-16 suites (est. 5 min);
  - `git log --format= --name-only --grep='^plan-16' | grep '^backend/'` →
    no output: plan-16 touches no backend code, so the backend and golden
    tests aren't needed.

## Decisions

- Trace is a drag, like the schematic Line tool, with at most one 45° bend
  per drag — owner, 2026-11-07 ("work like the schematic line tool",
  "limited to 90, 45, 135").
- The layer menu sits after the tools, like the supply menu. Picking a layer
  also picks the Trace tool and shows that layer — owner (placement) and
  author (behaviour, delegated), 2026-11-07.
- Traces are drawn in their layer's colour at their real width. New traces
  are 10 mil. Sidebar width is 5–100 mil — owner, 2026-11-07.
- Changing a trace's layer moves only that trace — author's call, delegated
  by the owner.
- Hit order is text, trace, footprint — author's call. Thin traces over
  part boxes stay clickable.

## Corrections

None yet.

## Summary

The risk is in folding traces into plan-15's selection and group drag. The
model under it is already tested in D, and Phase 2's tests cover the
integration. When E lands, archive all five plan-16 files to
`planning/completed/`.
