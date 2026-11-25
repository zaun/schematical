# plan-67-E: Unit view — the canvas and the sidebar

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-67-D

Puts the unit view in the user's hands. The sidebar of a part with units gets a
**View** choice (Block / Gate). A split part is drawn as its pieces; a click
picks the piece under the pointer; a drag moves it; Rotate and Flip act on it;
its sibling pieces are shown faintly so it is clear they are one IC. Delete,
Cut and the eraser take the whole IC, and ask first.

References:

- plan-67-A (the feature and its prerequisites), plan-67-B (`GateSymbolView`,
  `isSibling`), plan-67-C (`setPartSplit`, `flipUnit`, `rotateUnit`, picks).
- `schematical/schematical/Canvas/SchematicCanvasView.swift`: hover (744-768),
  `beginPointerInteraction` (782), box select (649-655), `SchematicPartsLayer`
  (1207-1386).
- `schematical/schematical/ContentView.swift`: `rotateSelection` (154),
  `flipSelection` (168), `rotatableChipID` / `flippableChipID` (185, 193),
  `deleteSelection` (248), `cutSelection` (271), the `pendingRemoval` alert (790).
- `schematical/schematical/Canvas/SchematicModel.swift`: `PendingRemoval`
  (6875-6905), `erase(at:)` (7018), `deleteSchematicSelection` (7082),
  `boxSelectSchematic` (1970).
- `schematical/schematical/Drawer/PartOptionsSection.swift`,
  `Drawer/SchematicSidebarView.swift`.
- `AGENTS.md` § Git and § Testing in the app.

Test command form `XB`: see plan-67-A.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites; re-run them. This
letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-D has landed | `grep -c 'var units: \[Unit\]?' schematical/schematical/Document/SchematicFile.swift` → 1 | MET (2026-11-25: 1, `ec716949`) |

## 1. Goal

- Selecting a gate IC shows a **View** picker with **Block** and **Gate** in
  the sidebar's Options; choosing Gate splits the part, Block joins it. A newly
  placed part is a block.
- Each piece of a split part is drawn with its own symbol and its own reference
  label (`U1-A` … `U1-P`).
- A click on a piece selects the part and picks that piece: it is highlighted
  fully, the other pieces of the part faintly. Shift-click adds or removes a
  piece. A marquee picks the pieces it touches.
- Dragging moves the picked pieces. ⌘R / ⇧⌘R and ⌘F act on the picked piece
  when exactly one is picked.
- Deleting, cutting or erasing a split part asks
  "Delete U1 and all 5 of its symbols?" first.

### Non-goals (explicit constraints)

- A block-view part, and every other item, deletes at once as today. No dialog
  is added for a multi-item selection.
- The app has no arrow-key nudge, align or duplicate command
  (`ContentView.editKey`, 392-414, handles only ⌘R, ⇧⌘R, ⌘F, ⌘⌫, ⌘X/C/V/A and
  A); this letter adds none.
- Copy and paste take the whole IC with its arrangement (letter C).
- Double-click actions (memory, CPU, terminal, registers windows), play-mode
  presses and the fuse and switch clicks keep using `chip(at:)` and act on the
  part.
- The PCB page is unchanged; selecting the footprint selects the part with
  every piece picked.

## 2. Current State

- `SchematicPartsLayer` (`SchematicCanvasView.swift:1207`) runs
  `ForEach(document.chips)` with an exhaustive `switch chip.symbol`, passing
  `isSelected: document.isSelected(.chip(chip.id))` and
  `isHovered: chip.id == hoveredChipID`, placing each view at `chip.viewOrigin`
  (1371); a second `ForEach` draws `ReferenceLabelView` (1375-1383).
- A press: `beginPointerInteraction` (782) — `document.chip(at:)`, then
  `pressSelection(.chip(hit.id), shift:)` and
  `.moveGroup(base: document.schematicGroupBase(), lever: shift ? nil : hit.id)`.
- Hover: `hoveredChipID = chip.id` (764).
- Box select: `document.boxSelectSchematic(rect, base:)` (649-655), built on
  `schematicItems(in:)`.
- Rotate and Flip act on the sole selected part: `rotatableChipID` (185) and
  `flippableChipID` (193) require `symbol.canRotate` / `canFlip` and a stopped
  simulation; `rotateSelection` (154) and `flipSelection` (168) call
  `rotateChip` / `flipChip`.
- Delete: `deleteSchematicSelection(named:includingTitleBlock:)` (7082) removes
  the selected chips at once. The eraser, `erase(at:)` (7018), removes the chip
  under the point at once. Cut is copy then
  `deleteSchematicSelection(named: "Cut")` (`ContentView.swift:271`).
- The only delete confirmation in the app is `pendingRemoval`
  (`SchematicModel.swift:6875`): a message and a private action (`.port`,
  `.busLine`, `.untick`), shown by one `.alert` (`ContentView.swift:790`),
  confirmed by `confirmPendingRemoval` (6891) under the undo name "Delete".
  While it is set, `erase` and `deleteSchematicSelection` do nothing.
- The agent's `delete` tool selects items and calls
  `deleteSchematicSelection`, then confirms a pending removal itself
  (`AutomationSchematic.swift:960-966`: "The app asks first…; the agent has
  said to delete").
- Per-part options are in `PartOptionsSection` (`Drawer/PartOptionsSection.swift`),
  shown when `applies(to:)` (41, 47) is true; each control calls a document
  function that wraps `recordEdit`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Views in `SchematicPartsLayer` that take `isSelected` / `isHovered` (each now gets a piece) | 1 `switch` over the 26 existing `PartSymbol` cases plus `.gate` (`sed -n 151,216p Canvas/SchematicModel.swift \| grep -cE '^\s+case '` → 26) | read `SchematicCanvasView.swift:1217-1369` |
| Callers of `deleteSchematicSelection` | `ContentView.swift` (delete, cut), `AutomationSchematic.swift` (delete tool) | `grep -rn 'deleteSchematicSelection' schematical/schematical --include='*.swift'` |

### Verified properties

- **A piece can be handed to every existing symbol view.** A piece is a
  `PlacedChip` (plan-67-B §5) whose symbol is `.gate` or `.block`; only
  `GateSymbolView` and `ChipBlockView` ever receive one.
- **`pendingRemoval` blocks a second dialog.** Read 7083 and 7021: both guard on
  `pendingRemoval == nil`.

## 3. Design Overview

1. **Drawing** — the parts layer iterates pieces.
2. **Picking** — press, shift-press, hover and marquee work on pieces.
3. **Commands** — Rotate and Flip target a piece; the View picker.
4. **The confirm** — delete, cut and erase of a split part.

Risk concentrates in the press path, which every part on the canvas goes
through: an unsplit part must behave exactly as before. The existing selection
gesture tests are the guard.

Byte-identity is not this letter's gate.

Rejected:

- *A plain click selects the whole IC and a modifier picks one gate* — the
  owner chose the clicked gate as the target.
- *A dialog on any multi-item delete* — the owner chose to ask only for parts
  drawn as several units.

## 4. Drawing

`SchematicPartsLayer`:

- Both `ForEach`es iterate `document.chips.flatMap(\.pieces)` with
  `id: \.drawID`, where `PlacedChip.drawID` is a `Hashable` pair of `id` and
  `unitKey`.
- `isSelected`: the part is selected **and** (the piece has no `unitKey` or its
  key is in `document.picked(chip)`).
- `isSibling`: the part is selected, the piece has a `unitKey`, and its key is
  not picked. Passed to `GateSymbolView` and `ChipBlockView`.
- `isHovered`: `piece.id == hoveredChipID && piece.unitKey == hoveredUnitKey`
  (a new `@State hoveredUnitKey: String?`, set beside `hoveredChipID` from
  `document.piece(at:)`, cleared with it).
- The reference label is drawn for every piece (the label reads `U1-A`).

## 5. Picking

- `beginPointerInteraction`: `document.piece(at: point)` in place of
  `chip(at:)`. After `pressSelection(.chip(hit.id), shift:)`, when
  `hit.unitKey` is set: `document.pick(UnitRef(chipID: hit.id, key: key), extend: shift)`.
  Then the group base is taken, so the drag moves the picked pieces.
- A press on a picked piece of a part that has several picked pieces keeps the
  picks (the same rule `pressSelection` applies to a selected item), so a
  marquee's pieces can be dragged together.
- `boxSelectSchematic`: after setting the selection, for every split chip in
  it, `pickUnits` with the keys of `chip.pieces(in: rect)`.
- Everything else that selects a part (Select All, the PCB mirror, an ERC row,
  the agent) leaves no pick entry: all pieces are picked.

## 6. Commands

`ContentView`:

- `rotatableChipID` / `flippableChipID` become `rotateTarget` / `flipTarget`
  returning `enum EditTarget { case chip(UUID), unit(UnitRef) }?`. For an
  unsplit sole selected part: `.chip(id)` under today's conditions. For a split
  one: `.unit(ref)` when exactly one piece is picked and that piece's symbol
  can rotate / flip; otherwise nil (Rotate and Flip are disabled, as for a
  multi-part selection).
- `rotateSelection` and `flipSelection` switch on the target and call
  `rotateChip` / `rotateUnit`, `flipChip` / `flipUnit`.

`PartOptionsSection`:

- A segmented `Picker("View", …)` with `Block` and the part's `unitViewName`
  ("Gate"), first in the section, shown when `!chip.part.units.isEmpty`.
  Selection is `current.isSplit`; set calls `document.setPartSplit(chip.id, $0)`.
  Disabled while the simulation runs, as the other schematic edits are.
- `applies(to part:)` (41) also returns true when `!part.units.isEmpty`.

`placeChip` is unchanged: a new part has no placements, so it is a block.

## 7. The confirm

`PendingRemoval.Action` gains `case parts(name: String)`. The message:

- one split part: `Delete U1 and all 5 of its symbols?`
- several parts, at least one split: `Delete the 3 selected parts? U1 and U4 are drawn as several symbols; all of them go.`

`deleteSchematicSelection(named:includingTitleBlock:confirmed: Bool = false)`:
when the selected chips include a split one and `confirmed` is false, it sets
`pendingRemoval` (action `.parts(name:)`) and returns true without removing
anything. `confirmPendingRemoval`, for `.parts`, calls it again with
`confirmed: true` under the carried undo name ("Delete" or "Cut").

`erase(at:)`: when the chip under the point is split, it selects that chip and
raises the same pending removal instead of removing it.

Cut copies first, then deletes through the same path, so the confirm appears
and the clipboard already holds the part.

The agent's `delete` tool confirms a pending removal itself for ports today;
that line (`AutomationSchematic.swift:964`) is made unconditional for parts too,
so the agent is never left behind a dialog.

## Compatibility / Format Impact

None: no file, export or agent contract changes in this letter.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` for partial; mark moot tasks; fill `Commit:`; add tasks you
> discover. **An unticked box means NOT DONE.**

### Phase 1 — the View option and drawing pieces

- [x] `Canvas/SchematicModel.swift`: `PlacedChip.drawID`.
- [x] `Canvas/SchematicCanvasView.swift` `SchematicPartsLayer`: iterate pieces,
      `isSelected` / `isSibling` / `isHovered` per §4; `hoveredUnitKey` state
      set and cleared beside `hoveredChipID` (744, 764) and passed to the layer.
      **Done:** the layer asks the document: `isPicked(_:)`, `isSibling(_:)`.
- [x] `Drawer/PartOptionsSection.swift`: the View picker and `applies` (§6).
      **Done:** not disabled while the simulation runs: no control of this section is (see Corrections).
- [x] Tests: new `schematicalTests/SplitPartCanvasTests.swift` —
      `testViewOptionApplies` (`PartOptionsSection.applies(to:)` true for
      `SN74HC00N`, unchanged (false) for `SN74HC74N`); `testPiecesAreDrawnOnce`
      (for a document with a split 7400 and a resistor,
      `chips.flatMap(\.pieces).map(\.drawID)` has 6 distinct values);
      `testNewPartsAreBlocks` (`TestSupport.place` of a 7400: `isSplit` false).

Acceptance: a gate IC can be switched to Gate view in the sidebar and is drawn
as five symbols with their labels.
  Check: `XB -only-testing:schematicalTests/SplitPartCanvasTests -only-testing:schematicalTests/SelectionSidebarTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: 64c0fe43

### Phase 2 — picking, dragging, Rotate and Flip

- [x] `SchematicCanvasView.beginPointerInteraction` per §5.
      **Done:** it calls one document function, `pressPiece(_:shift:)`, which holds the whole rule (see Corrections).
- [x] `SchematicDocument.boxSelectSchematic` per §5.
- [x] `ContentView`: `EditTarget`, `rotateTarget`, `flipTarget`,
      `rotateSelection`, `flipSelection`, `canRotate`, `canFlip` per §6.
      **Done:** `EditTarget`, `rotateTarget` and `flipTarget` are on `SchematicDocument` (testable); `ContentView` adds the page and simulation conditions.
- [x] Tests in `SplitPartCanvasTests`, driving the document functions the
      gestures call — `testPressPicksThePiece` (`piece(at:)` on unit C,
      `pressSelection` then `pick`: the part is selected, `picked` is `["C"]`);
      `testShiftPressAddsAPiece`; `testMarqueePicksTouchedPieces` (a box over
      B and C: selection has the chip, `picked` is B and C);
      `testSelectAllPicksEverything`; `testUnsplitPressIsUnchanged` (a resistor:
      the same selection and group base as before, no pick entry).
      **Done:** plus `testRotateAndFlipTargets`.

Acceptance: a piece can be picked, dragged, turned and flipped on its own, and
unsplit parts behave as before.
  Check: `XB -only-testing:schematicalTests/SplitPartCanvasTests -only-testing:schematicalTests/SelectionGestureTests -only-testing:schematicalTests/SelectionModelTests -only-testing:schematicalTests/SchematicRotateTests`
  → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 64c0fe43

### Phase 3 — the delete confirm

- [x] `Canvas/SchematicModel.swift`: `PendingRemoval.Action.parts(name:)`, the
      two messages, the `confirmed` parameter, `confirmPendingRemoval`'s new
      case, and `erase(at:)` per §7.
      **Done:** the action carries the selected items too, so the confirm deletes what was asked about even if the selection changed while the question was up.
- [x] `Automation/AutomationSchematic.swift` `delete`: confirm a pending
      removal after the item loop's `deleteSchematicSelection` as it does for
      ports.
      **Done:** the tool passes `confirmed: true`, so no question is ever raised for it.
- [x] Tests in `SplitPartCanvasTests` — `testDeleteAsksForASplitPart` (select a
      split 7400, `deleteSchematicSelection()`: the chip is still there,
      `pendingRemoval?.message == "Delete U1 and all 5 of its symbols?"`;
      `confirmPendingRemoval()`: gone, its wires have loose ends, one undo step
      "Delete" restores it split); `testCancelKeepsThePart`;
      `testBlockPartDeletesAtOnce` (no pending removal for an unsplit 7400 or a
      resistor); `testMixedSelectionMessage`; `testCutAsksAndKeepsTheUndoName`
      (the step is named "Cut"); `testEraserAsks` (`erase(at:)` on unit B).
      In `AutomationTests`: `testDeleteSplitPartDoesNotWait` (the tool removes
      the part and leaves `pendingRemoval` nil).
      **Done:** the agent case is `SplitPartCanvasTests.testAgentCanDeleteWithoutTheQuestion` (the document call the tool makes); the tool itself is exercised end to end in plan-67-G. `XB` over `SplitPartCanvasTests`, `SplitPartEditTests`, `SelectionSidebarTests`, `SelectionGestureTests`, `SelectionModelTests`, `SchematicRotateTests`, `AutomationTests`, `SocketPickerTests`, `TestPointTests` → `Executed 113 tests, with 0 failures`, `** TEST SUCCEEDED **` (16 of them `SplitPartCanvasTests`).

Acceptance: a split part is never removed without the question; everything
else is removed as before.
  Check: `XB -only-testing:schematicalTests/SplitPartCanvasTests -only-testing:schematicalTests/AutomationTests`
  → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 64c0fe43

## Validation Plan

- Tests: `SplitPartCanvasTests` (15 cases) and one automation case; the
  selection, sidebar and rotate classes re-run where their code is touched.
- Coverage check: the gesture code in `SchematicCanvasView` is not reachable
  from a unit test; the tests drive the document functions it calls, and the
  real gestures are covered by the runtime proof in plan-67-G.
- Runtime proof: in plan-67-G (the real window: switch a 7400 to Gate, drag a
  gate, turn it, delete and answer the question).
- Doc sync: none in this letter.
- Final gate: in plan-67-G, once.

## Decisions

- **A new gate IC starts in Block view** — decided by the owner, 2026-11-25.
  Settles that `placeChip` is unchanged. (§6)
- **Rotate, flip and drag act on the clicked gate; siblings are shown faintly;
  a marquee over several gates moves those gates** — decided by the owner,
  2026-11-25. (§4, §5, §6)
- **Delete, copy and select work on the whole IC** — decided by the owner,
  2026-11-25. (§7)
- **Only deleting a part drawn as several units asks first** — decided by the
  owner, 2026-11-25; a block-view part and ordinary multi-item deletes stay
  instant. Cut and the eraser ask too, because they remove the same symbols:
  the author's extension of that answer. (§7)
- **The agent's delete does not wait for the question** — the author's call,
  following the existing rule for ports with wires.

## Corrections

- **A Shift-press on a unit of an already selected part.** §5 said `pressSelection(.chip(id), shift:)` then `pick(…, extend: shift)`. `ItemSelection.press` with Shift on a selected item arms "a click removes it", so Shift-clicking a second gate would have dropped the whole IC from the selection. The rule lives in `SchematicDocument.pressPiece(_:shift:)` instead: Shift on a unit of a selected part adds or removes that unit; only Shift-clicking the last picked unit takes the part out. A plain press on one of several picked things drags them all, and a click keeps only that unit (`pendingPick`, finished in `releaseSelection`), the same rule a multi-item selection has.
- **A box select leaves alone the picks of a part that was selected before the box** (a Shift-drag that extends the selection); it sets the picks only of parts the box itself brings in.
- **The View picker is not disabled while the simulation runs.** §6 said "as the other schematic edits are". No control of `PartOptionsSection` is disabled then (read the section: Gender, Color, Footprint, Mounting have no such modifier), so the picker follows its neighbours.
- **The three phases landed in one commit**, verified by one run (113 tests).
- **Where the agent delete case is.** The plan put `testDeleteSplitPartDoesNotWait` in `AutomationTests`; the document-level call the tool makes is covered here, and the tool is driven for real in plan-67-G's `SplitPartAutomationTests`.

## Summary

The risk is the press path shared by every part; the existing gesture and
selection tests guard the unsplit behavior. Untouched: the PCB page, copy and
paste, double-click and play-mode actions.
