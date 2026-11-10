# plan-31-B: Undo on the Schematic page

Last updated: 2026-11-09
Effort: medium (1h–2h)
Depends on: plan-31-A

Every Schematic edit records itself through plan-31-A's `recordEdit`, and
every mouse drag on the Schematic canvas is one gesture. After B, any action
on the Schematic page (a test calls the same model functions the views call)
undoes and redoes exactly, as one step per action. That includes the two
cases plan-30 left in `planning/todo.md`: a drop that auto-connects loose
wire ends undoes with its move, and undoing a block delete puts its wires
back on its pins. Adding or deleting a part is on the PCB list too. The Edit
menu comes in D.

References:

- `planning/plan-31-A-undo-history.md` — the engine, the key-owner rule,
  Decisions
- `schematical/schematical/Canvas/SchematicModel.swift` — the edit functions
- `schematical/schematical/Canvas/SchematicCanvasView.swift` —
  `handleChanged`, `handleEnded` (the drag gesture), the drop handler
  (`placeChip`), the double-click (`insertNode`)
- `schematical/schematical/Drawer/SchematicSidebarView.swift` — the note
  `TextEditor` (`setNoteText` on every keystroke)
- `schematical/schematical/Simulation/Simulation.swift` — `start(document:)`,
  `stop()`

## Prerequisites

See plan-31-A § Prerequisites (they hold for every letter). Also:

| Must be true | Command | Status |
|---|---|---|
| plan-31-A is complete | `ls planning/plan-31-A-*` → no match (archived to `planning/completed/`) | MET (2026-11-09: `ls planning/plan-31-A-*` → no match; archived at 42e5c58) |

## 1. Goal

- Each function in the table below, called with arguments that change the
  document, leaves one step on the Schematic list (and the PCB list where
  marked). `undo(on: .schematic)` returns `SchematicFile(document:)` to its
  value before the call; `redo` returns it to the value after.
- A whole drag on the Schematic canvas is one step.

### Non-goals (explicit constraints)

- No edit's behavior changes. Wrapping only records.
- PCB edits are C's. The Edit menu is D's.
- `setButton`, `releaseAllButtons` and `restoreSwitches` are not recorded:
  pressed state is never saved, and Stop's reset is not an edit.

## 2. Current State

The 38 functions (overloads counted) that change Schematic data, read in
`SchematicModel.swift`, with the step name each records:

| Function(s) | Step name | Lists |
|---|---|---|
| `placeChip` | Add Part | both (adds `.part`, `.footprint`) |
| `setPartColor` | Change Color | Schematic |
| `setPartGender` | Change Gender | Schematic |
| `setHeaderSize` | Change Size | both (part name changes `.footprint`) |
| `setPartOhms` | Change Resistance | Schematic |
| `setPartFootprint` | Change Footprint | both |
| `renameChip` | Rename | both (`.reference`) |
| `flipChip` | Flip | Schematic |
| `moveSchematicGroup`, `settleSchematicGroup` | Move | Schematic |
| `addNote` | Add Text | Schematic |
| `setNoteText` | Edit Text (coalescing `"note-text:<id>"`) | Schematic |
| `setNoteSize` | Change Font Size | Schematic |
| `setNoteFramed` | Change Frame | Schematic |
| `deleteNote` | Delete Text | Schematic |
| `setMemoryFile` | Change Load File | Schematic |
| `toggleSwitch` | Flip Switch | Schematic |
| `connect` | Add Wire | Schematic |
| `settleWires(attachedToPort:)`, `settleWires(attachedToBus:)`, `settleWires(attachedTo:)` | Move | Schematic |
| `adoptLooseEnds` | Connect | Schematic |
| `setWirePath` | Move Wire | Schematic |
| `insertNode` | Add Bend | Schematic |
| `addBus(from:to:)`, `addBus(candidates:)` | Add Bus | Schematic |
| `setBusWireCount` | Change Bus Width | Schematic |
| `renameBusWire` | Rename Bus Wire | Schematic |
| `addPort` | Add Port | Schematic |
| `setPortWires`, `requestPortWires` | Change Port Pins | Schematic |
| `setPortSide` | Change Port Side | Schematic |
| `slidePort(_:to:)`, `slidePort(_:from:by:)` | Move Port | Schematic |
| `setBusLines`, `dragBus` | Move Bus | Schematic |
| `confirmPendingRemoval` | Delete | Schematic (it removes only ports and bus lines) |
| `erase` | Delete | both when it removes a part, else Schematic |

"Lists" follows from A's key-owner rule; it is not set by hand. Each
function passes `.schematic` as its page, except `renameChip`, which gains a
`page: EditorPage` parameter: the Schematic sidebar passes `.schematic`, the
PCB sidebar (C) passes `.pcb`. Both pages' lists get the step either way.

Callers that span several calls, read in `SchematicCanvasView.swift`:

- `handleChanged` runs each frame of the one `DragGesture(minimumDistance:
  0)`; the first frame sets `interaction` (`beginPointerInteraction`, tools
  `.line`, `.busPort`, `.bus`, `.eraser`, `.flip`, `.text`, and the supply
  tools that call `placeChip`). `handleEnded` finishes it: `connect`,
  `addBus`, `settleWires`, `adoptLooseEnds(ports:)`, `settleSchematicGroup`
  (which calls `adoptLooseEnds` — the plan-30 auto-connect),
  `toggleSwitchLever`, `erase`, then `releaseSelection` and `interaction =
  nil`.
- In play mode (`isRunning`) the same gesture pans, holds tactile buttons and
  flips switches; none of that may record.
- The drawer drop calls `placeChip` once; the double-click calls
  `insertNode` once — single calls, recorded by their own wrapper.

`SchematicSidebarView.swift`: the note `TextEditor` calls `setNoteText` on
each change of `words`; `.onChange(of: isEditing)` sees focus leave.

`Simulation.swift`: `start(document:)` sets `playDocument` once running;
`stop()` calls `restoreSwitches` and clears it.

### Measured populations

| What | Count | Command |
|---|---|---|
| Schematic edit functions (table above) | 38 | for each name in the table: `awk 'NR>=1040' schematical/schematical/Canvas/SchematicModel.swift \| grep -cE "^    func <name>\("`, summed (run under `bash`; zsh does not split the name list) → 38 |
| `renameChip` callers | 2 | `rg -n 'renameChip\(' schematical/schematical --glob '!**/SchematicModel.swift'` → `Canvas/PartInfoViews.swift` (`ReferenceSection`, used by `Drawer/SchematicSidebarView.swift:64` and `Canvas/PCBSidebarView.swift:433`) |

### Verified properties

- `ReferenceSection` (`PartInfoViews.swift`) is the one rename UI, shared by
  both sidebars; it needs a `page` property to pass on.
- `settleSchematicGroup` ends with `adoptLooseEnds(chips:junctions:)`
  (read), so with the drag as one gesture the move and the auto-connect are
  one step.
- `erase` on a part calls `loosen` (wire ends move to loose junctions) then
  removes the chip (read). Undo restores the `.wire` keys (ends back on
  `.pin`), the `.junction` keys (the loose ones removed) and the `.part`.

## 3. Design

- **Wrap each function.** The body of each function in the table moves inside
  `recordEdit(.schematic, "<name>") { … }` (returning its result where it has
  one). Nested calls (for example `placeChip` → `adoptLooseEnds`) are fine:
  only the outermost records, and the step is named by the first call that
  changed something (A §4.2), so a drop is "Add Part".
- **Gestures.** `handleChanged`: when not running and `interaction == nil`
  (the first frame), call `document.beginGesture(.schematic)` before deciding
  the interaction. `handleEnded`: after the `switch` and `releaseSelection`,
  call `document.endGesture()` when `document.isGestureOpen`. A gesture SwiftUI
  cancels without `onEnded` is closed by the next `beginGesture` or by
  `undo`/`redo` (A §4.2), so it can never block undo.
- **Play mode.** `SimulationController.start(document:)` sets
  `document.isPlaying = true` with `playDocument`; `stop()` sets it false
  after `restoreSwitches`. Nothing is recorded during a run.
- **Note typing** coalesces into one "Edit Text" step per editing session:
  `SchematicSidebarView`'s `.onChange(of: isEditing)` calls
  `document.endCoalescing()` when editing ends. While the `TextEditor` has
  focus, ⌘Z is the text view's own typing undo (D forwards it).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; moot as `- [x] ~~text~~ — moot:
> <evidence>`; fill `Commit:` when it lands. **An unticked box means NOT DONE.**

### Phase 1 — Record every Schematic edit

- [x] `Canvas/SchematicModel.swift`: wrap all 38 functions of the table in
      `recordEdit(.schematic, …)` with the names shown; `setNoteText` passes
      `coalescing: "note-text:\(id)"`; `renameChip` gains `page:
      EditorPage` and records on it. Wrapped by a brace-matching script
      (`/tmp/p31scripts/wrap.py`, spec `b.json`) → `total 38`, every name
      found (3 `settleWires`, 2 `addBus`, 2 `slidePort`). `adoptLooseEnds`'
      nested `tips` needed `@MainActor` inside the closure (Corrections 2).
- [x] `Canvas/PartInfoViews.swift`: `ReferenceSection` gains `let page:
      EditorPage` and passes it to `renameChip`;
      `Drawer/SchematicSidebarView.swift` passes `.schematic`,
      `Canvas/PCBSidebarView.swift` passes `.pcb`. (`BillOfMaterialsTests`
      passes `.schematic` too.)
- [x] `Simulation/Simulation.swift`: set `document.isPlaying` in
      `start(document:)` (true, with `playDocument`) and `stop()` (false,
      after `restoreSwitches`).
- [x] Tests: `schematical/schematicalTests/SchematicUndoTests.swift`. One
      test per table row: build the state the edit needs, record
      `SchematicFile(document:)` before, call the function, record after,
      `undo(on: .schematic)` → equals before, `redo(on: .schematic)` → equals
      after, and `undoName` is the row's name. Plus:
      `placeChip` and `erase`-a-part are on `undoName(on: .pcb)` too, and
      undoing from `.pcb` removes them from the Schematic list;
      `renameChip(page: .pcb)` is on both;
      `setPartColor` is not on the PCB list;
      plan-30 case 1: drop a part (`beginGesture`, `moveSchematicGroup`,
      `settleSchematicGroup`, `endGesture`) so its pin lands on a loose wire
      end → one step; undo leaves the wire loose and the part where it was;
      plan-30 case 2: a part with two wires on its pins, `erase` it, undo →
      both wires end on `.pin` of that part again and no loose junction is
      left (`LooseEndTests` builds the same fixture);
      three `setNoteText` calls then `endCoalescing()` then a fourth → two
      steps;
      `isPlaying = true` then `toggleSwitch` → no step.
      Added: `erase` of a wire is Schematic only; `confirmPendingRemoval`
      not on the PCB list; `SimulationController.start`/`stop` set and clear
      `isPlaying`. 45 tests. Check run (with `UndoHistoryTests`) →
      `Executed 100 tests, with 0 failures`, `** TEST SUCCEEDED **`.

Acceptance: every row and case passes; the existing canvas tests still pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/LooseEndTests -only-testing:schematicalTests/TactileSwitchTests -only-testing:schematicalTests/PlaySwitchResetTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: 7518dc6

### Phase 2 — One step per Schematic drag

- [x] `Canvas/SchematicCanvasView.swift`: `beginGesture(.schematic)` on the
      first non-play frame of `handleChanged`; `endGesture()` at the end of
      `handleEnded` (§3).
- [x] `Drawer/SchematicSidebarView.swift`: `endCoalescing()` when the note
      editor loses focus.
- [x] Added: `NotePanel` follows `note.text` when it changes from outside
      the editor (undo/redo). Its `words` were set only `onAppear`, so after
      an undo the editor showed the old words and the next keystroke saved
      them back over the undo.
- [x] Tests (`SchematicUndoTests`): a gesture of 20 `moveSchematicGroup`
      frames plus `settleSchematicGroup` is one "Move" step; a gesture whose
      only change is `toggleSwitch` (the lever click path: zero-delta
      `moveSchematicGroup` then `toggleSwitch`) is "Flip Switch"; a gesture
      that changes nothing records nothing; `beginGesture` twice without
      `endGesture` records the first gesture's change as its own step.
      Added: `undo` closes an open gesture first. Check run →
      `Executed 50 tests, with 0 failures`, `** TEST SUCCEEDED **`. Final
      gate (full app target) → `Executed 289 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.

Acceptance: the gesture cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicUndoTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: d294d25

## Validation Plan

- Tests: `SchematicUndoTests` (both phases).
- Coverage: one test per table row, so every wrapped function is exercised.
- Runtime proof: in D (the menu is how a person reaches undo).
- Doc sync: none in B.
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; B wraps the shared model functions every canvas test uses).

## Decisions

See plan-31-A § Decisions. B adds none.

## Corrections

1. The step-naming rule of plan-31-A §4.2 ("the first inner `recordEdit`
   that changed state") named `insertNode` "Move Wire", since its inner
   `setWirePath` call finishes first (`testInsertNode`: `("Optional("Move
   Wire")") is not equal to ("Optional("Add Bend")")`). Fixed in
   `recordEdit`: the outermost call that changed something names the step;
   among a gesture's sibling calls, the first such. A's doc records it.
2. A local `func` inside the `recordEdit` closure is not main-actor
   isolated: `adoptLooseEnds`' `tips` failed with "call to main
   actor-isolated instance method 'pinTip' in a synchronous nonisolated
   context". Marked `@MainActor`.
3. `UndoHistoryTests.testPlayingRecordsNothingAndUndoIsOff` expected an
   empty list under the flip; with `placeChip` recorded, "Add Part" is
   under it. Expectation updated.

## Summary

Mechanical wrapping of 38 functions plus two gesture hooks. The risk is a
missed function (a Schematic edit with no undo); one test per table row is
the guard.
