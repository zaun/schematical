# Multi-select Plan

Last updated: 2026-11-03
Effort: large (3h–1d)

Add Shift multi-selection to both the schematic and PCB canvases. The selection
is editor state only: it is never written to a schematic file. Users can
Shift-click or Shift-drag every canvas object, see the entire selection in the
active sidebar, and drag the selected group by one snapped delta.

References:

- `schematical/schematical/Canvas/SchematicModel.swift:SchematicDocument`
- `schematical/schematical/Canvas/SchematicCanvasView.swift:Interaction`
- `schematical/schematical/Canvas/PCBCanvasView.swift:Interaction`
- `schematical/schematical/Drawer/SchematicSidebarView.swift:SchematicSidebarView`
- `schematical/schematical/Canvas/PCBSidebarView.swift:PCBSidebarView`
- `schematical/schematical/Document/SchematicFile.swift:init(document:fileURL:)`

## 1. Goal

- Shift selection works for parts, wires, junctions, PCB footprints, and board
  text; dragging a selected item moves the selected group as one unit and the
  sidebar enumerates the selected objects.

### Non-goals

- Do not persist selection, change the schematic-file version, alter circuit
  simulation, add copy/paste or bulk editing, or add a marquee/lasso gesture.
- Do not make an unselected part move merely because an attached wire is
  selected.

## 2. Current State

`SchematicDocument` has mutually exclusive `selectedChipID` and
`selectedTextID`. `SchematicCanvasView` moves one chip then settles wires
attached to it; `PCBCanvasView` separately moves one footprint or text. Both
canvases use a single zero-distance drag recognizer. The schematic sidebar
derives one selected chip, while the PCB sidebar derives one footprint or text.
`SchematicFile` documents that selection is not saved.

Measured scope:

| What | Count | Command |
|---|---:|---|
| Swift files using singular selection IDs | 6 | `rg -l 'selectedChipID|selectedTextID' schematical/schematical --glob '*.swift' \| wc -l → 6` |
| Singular-selection references | 37 | `rg -n 'selectedChipID|selectedTextID' schematical/schematical --glob '*.swift' \| wc -l → 37` |
| Canvas zero-distance drag recognizers | 2 | `rg -n 'DragGesture\\(minimumDistance: 0\\)' schematical/schematical/Canvas --glob '*.swift' \| wc -l → 2` |
| Existing document move/route methods | 6 | `rg -n 'func (moveChip|moveFootprint|moveText|settleWires|setWirePath)' schematical/schematical/Canvas/SchematicModel.swift \| wc -l → 6` |

## 3. Design

Replace singular IDs with typed, page-specific sets: schematic items are chip,
wire, and junction IDs; PCB items are footprint/chip and board-text IDs. Keep
single-item convenience accessors for existing inspectors. A normal hit replaces
the active-page selection, a Shift hit toggles the hit item, a Shift-drag first
adds the hit item and then drags the resulting group, and a normal empty drag
clears the active-page selection and pans. An empty Shift drag pans without
changing selection.

Each group drag captures all starting geometry, then applies the total snapped
pointer delta on every update. Chips, junctions, footprints, and board text
translate directly. A selected wire translates only stored interior route nodes;
its pin/junction endpoints remain attached. On release, settle every selected
wire and every wire attached to a moved chip or junction once, preserving
connectivity and orthogonal routing.

Render accent selection for all selected items. A zero selection retains the
current drawer/board-settings sidebars. A sole selected object retains its
existing detailed inspector. A multi-selection shows `Selection (N)` with a
read-only row per item; activating a row reduces selection to that item and
Clear empties the set. PCB Rotate remains enabled for exactly one eligible item.

## Phases

### Phase 1 — Document selection and group movement

- [x] In `SchematicModel.swift`, introduce typed schematic and PCB selection
      sets, replace/toggle/clear APIs, and sole-item compatibility accessors.
      — `Canvas/Selection.swift` (`SchematicItem`, `BoardItem`,
      `ItemSelection`, `SelectionPress`); document `schematicSelection` /
      `boardSelection`, `select`/`toggleSelection`/`pressSelection`/
      `releaseSelection`/`clear*Selection`; read-only `selectedChipID`,
      `selectedFootprintID`, `selectedTextID`. Verified by
      `testReplaceToggleClear`, `testPressAndRelease`,
      `testSchematicItemsAndBusExcludeEachOther`, `testPagesKeepSeparateSelections` (pass).
- [x] Add starting-geometry snapshots and group-move methods for chips, wires,
      junctions, footprints, and board text; preserve wire endpoints and settle
      the complete affected-wire set at release.
      — `schematicGroupBase`/`moveSchematicGroup`/`settleSchematicGroup`,
      `boardGroupBase`/`moveBoardGroup`; `Junction.point` is now `var`.
      Verified by `testMixedGroupGetsOneSnappedDelta`,
      `testSelectedWireKeepsAttachedEnds`, `testWireNotSelectedDoesNotMovePart`,
      `testBoardGroupGetsOneSnappedDelta` (pass).
- [x] Update erase/delete and `replaceContents` to remove stale selected IDs
      and clear both selection sets. — `pruneSelection()` runs from
      `pruneJunctions()` (every part/wire removal) and `deleteText`;
      `replaceContents` calls `clearAllSelections()`. Verified by
      `testEraseDropsStaleSelection`, `testDeleteTextDropsIt`,
      `testReplaceContentsClearsBothSelections` (pass).
- [x] Add an XCTest target plus document tests for set algebra, snapping,
      mixed group movement, endpoint preservation, and unsaved selection.
      — `schematicalTests` target (hosted in the app) + shared scheme
      `xcshareddata/xcschemes/schematical.xcscheme`;
      `schematicalTests/SelectionModelTests.swift`, `TestSupport.swift`.
      Unsaved: `testSelectionIsNotSaved` (pass).
- [x] (added) Minimal caller updates so the app builds with the new API:
      `ContentView` Run/Rotate, `PCBCanvasView`, `PCBSidebarView` read
      `selectedFootprintID`/`select(.footprint…)`; `SchematicCanvasView` uses
      `select(.chip…)`/`clearSchematicSelection()`. Phase 2/3 replace these.

Acceptance: document tests prove all selected movable objects receive the same
snapped delta, selected wires retain attached endpoints, and a save/load has no
selection.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → selection-model tests pass (est. 3 min).
  Result (2026-11-07): `Executed 12 tests, with 0 failures` / `** TEST SUCCEEDED **`.
Commit: f427222

### Phase 2 — Canvas interaction and rendering

- [x] Update `SchematicCanvasView.swift` to interpret Shift at pointer-drag
      start, select chips/wires/junctions, capture group geometry, apply group
      drags, and draw all selected parts, wires, and junctions in accent color.
      — `beginPointerInteraction` reads `NSEvent.modifierFlags` once and
      routes part/junction/wire hits through `pressSelection`;
      `.moveGroup(base:lever:)` replaces `.moveChip` (the unused
      `SchematicDocument.moveChip` was removed); release runs
      `settleSchematicGroup` and `releaseSelection`. New `junctionHit`. Wires
      are drawn accent at 2.5 pt and junction dots accent at r 4 when selected.
      Proof: harness `scripts/harness/multiselect.swift` →
      `/tmp/p15img/s1-selected.png` (2 LEDs, branch wire, junction in accent),
      `s2-dragged.png`; output `U1: (80.0, 80.0) -> (120.0, 120.0)`,
      `U3: (230.0, 240.0) -> (270.0, 280.0)`, `U2` unchanged,
      `junction: (250.0, 100.0) -> (290.0, 140.0)`, `all right angles: true`.
- [x] Update `PCBCanvasView.swift` with the same Shift semantics for footprints
      and text, group-drag snapshots, and multi-item accent bounds.
      — `Interaction` is `.moveGroup(base:)`/`.pan`; `end(translation:)`
      releases the press; one accent box per selected item. Proof:
      `/tmp/p15img/p1-selected.png`, `p2-dragged.png`; output: both footprints
      and the text moved (+2.54, +1.27) mm, e.g. `(4.76, -0.135) -> (7.3, 1.135)`.
- [x] Preserve existing tool precedence: line, eraser, flip, supply placement,
      play-mode panning, wire node/segment editing, and Text placement retain
      their current behavior outside Select/Pointer selection gestures.
      — `git diff -U0` hunks in `SchematicCanvasView.swift` touch only the
      Pointer branch (old line 351), `beginPointerInteraction`, `handleEnded`
      and drawing. The play-mode, Line, Bus, Bus Port, Eraser, Flip and supply
      branches (old lines 331–452) are unchanged. A plain press on an
      unselected wire still edits its node or segment
      (`testPlainPressOnWireKeepsRouteEditing`). The PCB Text tool still
      places and drags the new text, now as a one-item group.
- [x] (added) Document helpers `pressEmptySchematic(shift:)` /
      `pressEmptyBoard(shift:)`, and gesture-sequence tests
      `schematicalTests/SelectionGestureTests.swift` (6 tests): Shift-select
      then drag, an empty Shift press keeps the selection, wire press modes,
      bus exclusion, junction drag, PCB Shift group drag.

Acceptance: manually exercising both canvases shows Shift-selected objects stay
highlighted and move together on the corresponding grid; empty Shift drags pan
without clearing selection.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → gesture/model tests pass (est. 3 min).
  Result (2026-11-07): `Executed 18 tests, with 0 failures` / `** TEST SUCCEEDED **`.
  Manual check: done headlessly with the harness renders above. The harness
  drives the same document calls the gestures make; the live Shift key
  (`NSEvent.modifierFlags`) is the only part not exercised.
Commit: —

### Phase 3 — Sidebar and lifecycle integration

- [ ] Add a shared selection-row resolver for references/part numbers, wire
      endpoints, junction coordinates, footprints, and board-text previews.
- [ ] Update `SchematicSidebarView.swift` and `PCBSidebarView.swift` with
      zero/single/multiple selection states and a `Selection (N)` list.
- [ ] Make summary-row selection replace its page set; make Clear clear that
      page set; leave mutable inspectors available only to a sole selection.
- [ ] Update `ContentView.swift` selection clearing at run start and PCB Rotate
      enablement for exactly one selected eligible item; add accessibility labels
      for the count and summary rows.

Acceptance: every selected object is listed in the active sidebar, selecting a
row opens the existing single-item inspector, and Run clears selection.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → sidebar/lifecycle tests pass (est. 3 min).
Commit: —

## Validation Plan

- Tests: selection replacement/toggling/clearing, group snapping, selected-wire
  endpoint preservation, selection cleanup, sidebar zero/single/multiple states,
  and run-time clearing.
- Runtime proof: Shift-select multiple parts, wires/junctions, footprints, and
  board text; verify highlights, sidebar rows, group drag, Clear, and Run reset.
- Doc sync: update the canvas and sidebar comments describing Pointer/Select
  behavior and selection state.
- Final gate: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` and `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build --output-on-failure` → all tests pass (est. 10 min).

## Decisions

- Shift applies to every canvas object type — owner decision, 2026-11-03.
- Selection is transient and page-specific — preserves existing file behavior
  and avoids treating a schematic wire as a PCB object.
- Selected wires move only their interior route geometry; attached endpoints
  remain fixed unless their chips/junctions are also selected — preserves
  explicit selection and connectivity.
- A multi-selection inspector is read-only; existing editable inspectors remain
  single-selection only — prevents accidental bulk edits.

## Corrections

- 2026-11-07, entry gate: the plan has no Prerequisites section, so there
  was no gate to run. Scope counts re-measured and matching: 6 files, 37
  references, 2 zero-distance recognizers (`rg` commands in §2). The
  "move/route methods" row is 8 by that `rg` (it also matches the three
  `settleWires` overloads and `moveFootprintOrigin`), not 6.
- 2026-11-07, §2 Current State is stale: plan-13 (buses, bus ports) landed
  after this plan was written. The document also has `selectedBusID` and
  `selectedPortID`. Buses and ports are not in this plan's Goal list (parts,
  wires, junctions, footprints, board text), and they cannot take a free
  group delta: a bus drag is walked step by step and refused at joints,
  ports and other buses, and a port only slides along its own line. So they
  stay single-selection. A bus or port selection and the Schematic item set
  exclude each other, and Shift on a bus or port acts like a plain press.
- 2026-11-07, design: "a normal hit replaces the active-page selection"
  conflicts with the Goal ("dragging a selected item moves the selected
  group"). Resolved in `ItemSelection.press`: a plain press on an item of a
  larger selection drags the group, and a plain click (no movement) then
  reduces the selection to that item. A plain press anywhere else replaces
  the selection, as written.
- 2026-11-07, design: selection was one shared `selectedChipID` for both
  pages. It is now per page (§3 "page-specific"). Selecting a part on the
  Schematic no longer selects its footprint on the PCB. ContentView Rotate
  reads `selectedFootprintID`.
- 2026-11-07, design: `OrthogonalRoute.settled` alone can leave a slanted
  segment after a group move. Example: a wire with three or more bends
  whose ends did not move. `settleSchematicGroup` adds corners
  (`OrthogonalRoute.normalize`) to every affected wire that was all right
  angles at the press. Wires the user had slanted on purpose (node drags)
  keep their slants.
- 2026-11-07, PCB snapping: a single item still moves as before (its own
  origin snaps onto the grid). Several items all get one `BoardGrid`-snapped
  delta, so an off-grid footprint keeps its offset relative to the others.
- 2026-11-07, XCTest host: freeing a `SchematicDocument` inside an XCTest
  method aborts the test host on macOS 15.8 ("pointer being freed was not
  allocated" in `swift_task_deinitOnExecutor`, the Swift runtime's
  isolated-deinit path). Tests make documents through
  `TestSupport.newDocument()`, which keeps them for the life of the process.

## Summary

The highest risk is route correctness for a mixed chip/junction/wire selection.
The document-level movement tests land before the UI gesture work, and neither
the simulation nor the file format changes.
