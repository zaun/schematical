# plan-06-D: PCB toolbar with Select and Text tools, the text panel, and delete

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-06-C

The PCB page gets a tool picker in the toolbar: **Select** (default — what
the page does today) and **Text**. With Text, a click on the board adds a
free text box there ("Text", 50 mil, silkscreen), selects it and goes back
to Select. Texts can be dragged, rotated with the Rotate button, and
deleted. A selected text's sidebar panel edits its text, layer (Document |
Silkscreen), font size in mil, X/Y and rotation, with a Delete button.

Outcome: on the PCB, pick Text, click the board: a "Text" label appears
selected and the sidebar shows its panel; type "REV A", size 80, Document,
rotation 90: the canvas shows gray "REV A" turned; drag it; ⌘⌫ removes it.

References:

- plan-06-A §4.1, §4.3 (text methods), plan-06-B §4.2 (text drawing),
  plan-06-C §4.2 (sidebar, `LengthField`).
- `schematical/schematical/ContentView.swift` (toolbar, `SchematicTool`
  picker), `Canvas/PCBCanvasView.swift` (gestures).

## Prerequisites

See plan-06-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-06-C complete | `ls planning/completed/plan-06-C-pcb-sidebar.md` → exists | NOT MET (2026-11-01) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- `PCBTool` picker (Select, Text) on the PCB page.
- Text tool places a text and returns to Select.
- Select tool: press on a text selects and drags it (anchor snapped);
  texts are hit before footprints (they draw on top).
- Rotate (⌥⌘R) turns the selected footprint or text.
- Text panel in the sidebar; Delete button with ⌘⌫.

### Non-goals (explicit constraints)

- Footprints cannot be deleted on the PCB (they follow the schematic).
- Schematic tools and shortcuts unchanged (⌘R, ⌥⌘R, ⌘O, ⌘S, ⇧⌘S, ⌘-, ⌘=,
  ⌘0 keep their meaning; ⌘⌫ is not used today:
  `grep -rn 'keyboardShortcut(.delete' schematical/schematical` → no
  matches).
- No model or file change.

## 2. Current State

- `ContentView` `.principal`: page picker, then on Schematic the
  `SchematicTool` segmented, icon-only picker, disabled while running.
  After plan-06-B: no simulation controls on the PCB page, and switching
  to PCB stops the simulation, so nothing on the PCB checks `isRunning`.
  `.primaryAction` on PCB: Rotate (`rotate.right`, ⌥⌘R, `canRotate`).
- `PCBCanvasView.handleChanged`: press on a footprint → select and move;
  else clear selection and pan (plan-06-B removed the running rule).
- After plan-06-A/B/C: text methods on the document, text drawing and
  selection box, `PCBSidebarView` showing a placeholder for a selected text.

## 3. Design Overview

1. `PCBTool` enum and picker (`ContentView`), passed to `PCBCanvasView`
   with an `onPlaced` callback, the same pattern as the schematic's tool
   and its return-to-pointer closure.
2. Gestures: text placement and text dragging.
3. Rotate and delete.
4. Text panel.

Risk: gesture priority (text vs footprint vs pan); proven by harness calls
to the same hit-test order and by the window capture.

Rejected alternative: *plain Delete key* — on macOS an unmodified ⌫
shortcut on a button can fire while a text field in the sidebar is being
edited; ⌘⌫ is Finder's "Move to Trash" key and does not collide with
typing.

## 4. Detailed Design

### 4.1 Tool and toolbar (`ContentView.swift`, `Canvas/PCBCanvasView.swift`)

- `enum PCBTool: String, CaseIterable, Identifiable { case select, text }`,
  titles "Select" / "Text", symbols `cursorarrow` / `textformat`, in
  `PCBCanvasView.swift`.
- `@State private var pcbTool = PCBTool.select`; on the PCB page the
  `.principal` group shows a segmented icon-only `Picker` like the
  schematic's.
- `PCBCanvasView(document:viewport:tool:onPlaced:)`.

### 4.2 Gestures (`PCBCanvasView.handleChanged`)

On the first change of a drag:

1. Tool Text → `document.addText(at: board(startLocation))`, call
   `onPlaced` (ContentView sets Select), then treat the rest of the drag as
   a move of that text.
2. Tool Select → `document.text(at:)` hit → select it,
   `interaction = .moveText(id:, startAnchor:)`; else footprint hit as
   today; else clear both selections and pan.

`.moveText` sets the anchor to start + translation / scale through
`moveText` (snapped).

### 4.3 Rotate and delete (`ContentView.swift`)

- Rotate: enabled when the selected chip has a placement
  or a text is selected; calls `rotateFootprint` or `rotateText`. Help
  "Rotate the selection 90° (⌥⌘R)".
- Delete: in the text panel (§4.4), `Button("Delete Text", role:
  .destructive)`, `.keyboardShortcut(.delete, modifiers: .command)`, calls
  `deleteText`. Shown only with a text selected, so ⌘⌫ does nothing
  otherwise.

### 4.4 Text panel (`Canvas/PCBSidebarView.swift`)

Replaces C's placeholder when `selectedTextID` is set.

- Header "Text" in `.title2` bold.
- `TextField("Text", text:, axis: .vertical)`, `lineLimit(1...6)`; commit
  on focus loss or ⌘Return → `setText` (an empty value is refused by the
  document; the field shows the kept text).
- "Layer": segmented Document | Silkscreen → `setTextLayer`.
- "Font Size": `TextField` of integer mil with a " mil" suffix label and a
  `Stepper` (step 5) → `setTextSize` (clamped 10–1000).
- "Position": X, Y `LengthField`s showing the anchor, commit →
  `moveText` (snapped).
- "Rotation": segmented 0° | 90° | 180° | 270° → `setTextRotation`.
- Delete button (§4.3).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Tools, gestures, rotate

- [ ] `Canvas/PCBCanvasView.swift`: `PCBTool`, new init parameters, §4.2.
- [ ] `ContentView.swift`: tool picker, `onPlaced`, Run resets the tool,
      Rotate for texts (§4.1, §4.3).

Acceptance: the Text tool places and selects a text and returns to Select;
texts drag and rotate.
  Check: harness `tools`: call `PCBCanvasView`'s gesture handler (made
  `internal` for this, as `handle(start:translation:)`) with tool Text at a
  board point inside the board → one text, selected, `onPlaced` called
  once; tool Select, press on that text over a footprint → the text is
  selected, not the footprint; translate by 2 mm → anchor moved by 1.905
  (snapped); rotate → 90; press on empty board → both selections nil
  and a pan (each `true`) (est. 3 min).
  Check: plan-06-A `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
Commit: —

### Phase 2 — Text panel and delete

- [ ] `Canvas/PCBSidebarView.swift`: text panel (§4.4), Delete button with
      ⌘⌫ (§4.3).

Acceptance: the panel edits every text property; delete removes the text.
  Check: harness `window-d`: offscreen window (plan-06-C's method), PCB
  page, add a text, select it → `d-panel.png` shows the text panel and the
  toolbar's Select | Text picker; drive the commits: text "REV A", size 80,
  Document, rotation 90, X 5 mm → canvas `d-text.png` shows gray "REV A"
  turned; Delete → no texts, the board panel shown (`d-deleted.png`);
  save/reopen of the pre-delete state keeps every value (`true`) (est.
  10 min: one window run covers the panel; nothing smaller renders it).
  Check: plan-06-A `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
Commit: —

## Validation Plan

- Tests: harness modes `tools`, `window-d`.
- Coverage check: each §4.2 branch (text tool, text hit, footprint hit,
  pan) is driven in `tools`; each panel field in `window-d`.
- Runtime proof: the §Outcome sequence in `window-d`, PNGs in `/tmp/p06/`.
- Doc sync: none (format unchanged since A).
- Final gate (plan-06 overall, once, after this phase):
  `cmake -S backend -B /tmp/logic-p06 && cmake --build /tmp/logic-p06 -j8
  && ctest --test-dir /tmp/logic-p06` (est. 1 min; backend unchanged) +
  the `xcodebuild` (est. 1 min) + every harness mode of plan-06-A…D
  (est. 25 min: the window captures are the only end-to-end proof of the UI
  and each takes a few minutes).

## Decisions

- Toolbar with Select (default) and Text; Text adds a free-form text box —
  owner, 2026-11-01.
- Text panel: text, document/silkscreen, font size in mil, X/Y, rotation
  0/90/180/270, delete — owner, 2026-11-01.
- Author's choices: placing a text returns to Select (as placing a part
  does on the schematic); texts are hit before footprints; ⌘⌫ deletes;
  multi-line text allowed (up to 6 visible lines in the field); stepper
  step 5 mil.

## Corrections

## Summary

Gesture priority is the only logic risk, driven directly in the harness;
the rest is UI over plan-06-A's methods.
