# plan-31-C: Undo on the PCB page

Last updated: 2026-11-09
Effort: medium (1h–2h)
Depends on: plan-31-B

Every PCB edit records itself through plan-31-A's `recordEdit` on the PCB
list, and every mouse drag on the PCB canvas is one gesture. The two board
settings the sidebar writes directly (units and colour) get edit functions,
so they record too. After C, every action on the PCB page undoes and redoes
exactly, one step per action. The PCB list stays independent of the
Schematic list, except for steps that change both pages (A § Decisions).

References:

- `planning/plan-31-A-undo-history.md` — engine, key-owner rule, Decisions
- `planning/plan-31-B-undo-schematic.md` — the same wrapping on the
  Schematic side; `renameChip(page:)`
- `schematical/schematical/Canvas/SchematicModel.swift` — PCB edit
  functions (`// MARK: Footprints (PCB)` to `// MARK: Group drags (PCB)`)
- `schematical/schematical/Canvas/PCBCanvasView.swift` — `handle(start:
  translation:)`, `begin(at:)`, `end(translation:)`, `TraceDrawing.finish`
- `schematical/schematical/Canvas/PCBSidebarView.swift` — board, footprint,
  text and trace inspectors
- `schematical/schematical/ContentView.swift` — ⌘R rotate
  (`rotateText`/`rotateFootprint`)

## Prerequisites

See plan-31-A § Prerequisites. Also:

| Must be true | Command | Status |
|---|---|---|
| plan-31-B is complete | `ls planning/plan-31-B-*` → no match (archived) | MET (2026-11-09: `ls planning/plan-31-B-*` → no match; archived at d294d25) |

## 1. Goal

- Each function in the table below, called with arguments that change the
  document, leaves one step on the PCB list. `undo(on: .pcb)` returns
  `SchematicFile(document:)` to its value before; `redo` to the value after.
- A whole drag on the PCB canvas is one step.
- Undoing on one page never changes what the other page's own steps did.

### Non-goals (explicit constraints)

- No PCB edit's behavior changes.
- `BoardSettings` keeps its fields and file keys; only the write path for
  `units` and `color` moves into the model.
- Hidden layers (the View popover) are not saved and not undoable.

## 2. Current State

The 28 PCB functions (overloads counted), read in `SchematicModel.swift`,
plus the two new ones, with step names:

| Function(s) | Step name |
|---|---|
| `moveFootprint` (called by `moveBoardGroup`) | Move |
| `moveFootprintOrigin` | Move |
| `rotateFootprint` | Rotate |
| `setFootprintSide` | Change Side |
| `setFootprintOrigin` | Change Origin |
| `setFootprintStyle` | Change Style |
| `setFootprintMarks` | Change Marks |
| `setBoardSize` | Change Board Size |
| `setLayerCount` | Change Layers |
| `setLayerColor` | Change Layer Color |
| `setLayerType` | Change Layer Type |
| `addText` | Add Text |
| `setText` | Edit Text |
| `setTextSize` | Change Font Size |
| `setTextLayer` | Change Layer |
| `setTextSide` | Change Side |
| `moveText` | Move |
| `setTextRotation`, `rotateText` | Rotate |
| `deleteText` | Delete Text |
| `addTrace` | Add Trace |
| `join` (called by `addTrace`, `setTraceLayer`, `moveTraces`, `settleBoardGroup`) | Join Traces |
| `setTraceWidth` | Change Trace Width |
| `setTraceLayer` | Change Layer |
| `deleteTrace` | Delete Trace |
| `moveTraces` | Move |
| `moveBoardGroup`, `settleBoardGroup` | Move |
| new `setBoardUnits` | Change Units |
| new `setBoardColor` | Change Board Color |

All pass `.pcb`. None changes a `.part` key, so none of them lands on the
Schematic list; a rename from the PCB sidebar is `renameChip(page: .pcb)`
(B), on both lists.

Board settings written outside the model, read in `PCBSidebarView.swift`:
line 37 `Picker("Units", selection: $document.board.units)` and line 125
`document.board.color = colour`. `board` is `var` (public set) in
`SchematicDocument`.

PCB gesture, read in `PCBCanvasView.swift`: `handle(start:translation:)`
runs each frame and calls `begin(at:)` on the first (`interaction == nil`):
the Text tool calls `addText` and then the drag moves it
(`.moveGroup`); Select picks an item and moves it, or box-selects, or pans;
Trace starts a draft. `end(translation:)` runs `TraceDrawing.finish`
(`addTrace`), `settleBoardGroup`, `releaseSelection`, then `interaction =
nil`.

### Measured populations

| What | Count | Command |
|---|---|---|
| PCB edit functions (table, without the two new ones) | 28 | for each name: `awk 'NR>=1040' schematical/schematical/Canvas/SchematicModel.swift \| grep -cE "^    func <name>\("`, summed under `bash` → 28 |
| Direct writes to `document.board` outside the model | 2 | `rg -n 'document.board.color = \|\$document.board.units' schematical/schematical` → `PCBSidebarView.swift:37`, `:125` |

### Verified properties

- `moveFootprint`, `join` and `moveTraces` have no caller outside the model
  (`grep -rn '\bjoin(\|moveTraces(\|moveFootprint(' --include='*.swift'`:
  only `SchematicModel.swift` and `TraceJoinTests.swift:100`). Wrapping them
  is still needed so a direct call (tests) records.
- `BoardText`/`BoardTrace` hold no part ids (A § Verified properties), so
  undoing a Schematic part delete cannot leave a PCB item dangling.

## 3. Design

- **Wrap each function** of the table in `recordEdit(.pcb, "<name>") { … }`.
- **`board` becomes `private(set)`.** Add `setBoardUnits(_:)` and
  `setBoardColor(_:)` (assign when different). `PCBSidebarView` uses a
  `Binding(get: { document.board.units }, set: { document.setBoardUnits($0) })`
  for the Picker and `document.setBoardColor(colour)` in the swatch.
- **Gesture.** `handle(start:translation:)`: when `interaction == nil`, call
  `document.beginGesture(.pcb)` before `begin(at:)`. `end(translation:)`:
  after `releaseSelection`, `document.endGesture()` when
  `document.isGestureOpen`. So "Text tool press + drag" is one "Add Text"
  step, and a group move with its trace re-joins is one "Move".

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; moot as `- [x] ~~text~~ — moot:
> <evidence>`; fill `Commit:` when it lands. **An unticked box means NOT DONE.**

### Phase 1 — Record every PCB edit

- [x] `Canvas/SchematicModel.swift`: wrap the 28 functions; add
      `setBoardUnits` and `setBoardColor`, both recorded; `board` becomes
      `private(set) var`. Same script as B (`/tmp/p31scripts/wrap.py`, spec
      `c.json`) → `total 28`; `grep -c 'recordEdit(.pcb'` → 28 before the two
      new functions. No test wrote `doc.board` directly (`rg 'doc\.board\.[a-z]+
      =|doc\.board = ' schematicalTests` → nothing).
- [x] `Canvas/PCBSidebarView.swift`: the Units picker and the colour swatch
      call the new functions.
- [x] `Canvas/PCBCanvasView.swift`: `beginGesture(.pcb)` / `endGesture()`
      per §3.
- [x] Added: the PCB text panel (words, size), the trace panel (width) and
      `ReferenceSection` (both sidebars) follow their item when undo/redo
      changes it. They kept local copies set only `onAppear` and commit on
      focus loss, so after an undo they showed the old value and leaving the
      field saved it back as a new step. (`LengthField` and the bus panels
      already follow; checked by `rg -n '@State (private )?var' Canvas Drawer
      Document`.)
- [x] Tests: `schematical/schematicalTests/PCBUndoTests.swift`. One test per
      table row (before/after `SchematicFile`, undo → before, redo → after,
      `undoName(on: .pcb)` is the row's name). Plus independence:
      move a part on the Schematic and its footprint on the PCB, undo on
      `.pcb` → footprint back, part still moved; undo on `.schematic` → part
      back, footprint stays where the PCB undo left it;
      PCB-only steps never appear on `undoName(on: .schematic)`;
      a part added on the Schematic, footprint moved on the PCB, then the
      add undone from the Schematic → the PCB "Move" step is skipped and
      `undo(on: .pcb)` undoes the step under it (or returns false when there
      is none);
      a Text-tool gesture (`beginGesture(.pcb)`, `addText`,
      `moveBoardGroup` ×5, `settleBoardGroup`, `endGesture`) is one "Add
      Text" step.
      34 tests. Check run → `Executed 72 tests, with 0 failures`,
      `** TEST SUCCEEDED **`. Final gate (full app target) → `Executed 323
      tests, with 0 failures`, `** TEST SUCCEEDED **`.

Acceptance: every row and case passes; existing PCB tests still pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/TraceJoinTests -only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/BoardLayersTests -only-testing:schematicalTests/BoardSideTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: 8f427dc

## Validation Plan

- Tests: `PCBUndoTests`.
- Coverage: one test per table row.
- Runtime proof: in D.
- Doc sync: none in C.
- Final gate (once, after Phase 1): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; shared model code).

## Decisions

See plan-31-A § Decisions. Board units and colour are undoable because they
are saved (author's call, recorded in A).

## Corrections

1. The table names 28 functions plus two new ones; all wrapped as listed.
   The one addition is outside the model: the sidebar fields that keep a
   local copy (task added above), found while checking that undo shows in
   every panel.

## Summary

Same mechanical wrapping as B for 30 functions, plus moving two direct
writes into the model. The independence tests are the guard that the two
lists do not undo each other's work.
