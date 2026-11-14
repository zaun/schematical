# plan-41-B: Cmd-drag a reference, reset, help, landing

Last updated: 2026-11-14
Effort: medium (1h–2h)
Depends on: plan-41-A

B gives the user the gesture. On the PCB page's Select tool, Cmd-press on a
reference's text and drag moves that reference only (not the part, not the
rest of the selection), snapped to the trace grid, as one undo step. The
sidebar's Marks section gets a button that puts it back; the PCB help lists
the gesture. B ends with plan-41's landing tier.

Outcome: a user Cmd-drags "R1" off its 0603's pads in the real app, the design
rule check's silkscreen warning for it goes, moving or turning the part carries
the reference along, and Undo puts it back.

References:

- plan-41-A § 2 and § 3 (the offset, `moveReference`, `resetReference`,
  `boardReferenceRect`).
- plan-41-A § Prerequisites (they apply to B).
- `Canvas/PCBCanvasView.swift` (`Interaction`, `begin(at:)`, `handle`, `end`,
  `TraceDrawing` as the precedent for an enum the tests drive in the canvas's
  order).
- `Help/KeyboardHelp.swift` (`pcb`, line 112), `KeyboardHelpTests`.
- `planning/completed/plan-39-L-active-low-landing.md` (landing precedent).

## Prerequisites

As plan-41-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-41-A complete | `ls planning/completed/plan-41-A-*.md` → exists, or every A box ticked | NOT MET (A not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Select tool, Cmd held at the press, on a drawn reference's text: dragging
  moves that reference; the footprint, other selected items and the viewport
  stay still.
- Cmd-press anywhere else behaves exactly as without Cmd (Cmd is not read
  today: `grep -n 'command' schematical/schematical/Canvas/PCBCanvasView.swift`
  → only line 201, the key monitor).
- The sidebar's Marks section has "Reset Position" when the offset is not
  zero.
- PCB Help lists "⌘-drag a reference".

### Non-goals (explicit constraints)

- Other tools (Trace, Via, Hole, Text, Delete) ignore Cmd as they do today.
- No new shortcut key, no menu item, no automation tool.
- Plain drag on a reference still moves the part (the reference isn't
  hit-tested without Cmd).
- A reference isn't a selectable item: `BoardItem` has no new case and
  Delete does not delete it.

## 2. Current State

- `PCBCanvasView.begin(at:)` (Canvas/PCBCanvasView.swift:644): reads Shift
  once (`NSEvent.modifierFlags.contains(.shift)`), and for `.select` picks
  `document.boardItem(at:tolerance:visible:)` → `.moveGroup`, else box-select
  or pan.
- `handle(start:translation:)` (line 622) opens the gesture
  (`document.beginGesture(.pcb)`) on the first step, then switches on
  `Interaction`; `end(translation:)` (line 686) closes it
  (`document.endGesture()`), so one press is one undo step
  (plan-31-C; `recordEdit` calls inside a gesture are one step).
- `Interaction` (line 85) cases: `moveGroup`, `pan`, `boxSelect`, `drawTrace`,
  `idle`.
- `SchematicDocument.footprint(at:visible:)` (SchematicModel.swift:2835) is the
  precedent for hit order: Top before Bottom, last chip drawn first, and
  `visible.showsMark(layer, side:)` decides what is drawn.
- `document.select(_ item: BoardItem)` (SchematicModel.swift:1298) makes one
  item the whole selection.
- The sidebar's Marks section is `PCBSidebarView.swift:955-959` (`Section("Marks")` at 955).
- The reference is drawn `max(1, 8 / scale)` mm tall (PCBCanvasView.swift:559)
  so it stays readable zoomed out.

### Verified properties

| Claim | How verified |
|---|---|
| Cmd is free on a PCB press | `grep -n 'command' PCBCanvasView.swift` → line 201 only (a `keyDown` monitor, not a mouse press). |
| One gesture is one undo step on a moved footprint | `PCBUndoTests.testMoveFootprint` asserts one "Move" step; the gesture machinery is `recordEdit`'s nesting (SchematicModel.swift:1619-1635). |
| `BoardGroupBase` and `moveBoardGroup` are not needed | Read both (Selection.swift:166, SchematicModel.swift:3546): they move whole items; a reference is a field of a placement. |

## 3. Design Overview

- **Hit test.** `SchematicDocument.referenceHit(at:tolerance:height:visible:)
  -> UUID?` returns the chip whose reference is drawn
  (`placement.reference != .hidden` and `visible.showsMark(…)`) and whose
  `boardReferenceRect(height:)`, grown by `tolerance` on every side, contains the
  point. Top before Bottom, last chip first. `height` is
  `max(BoardReference.height, 8 / scale)` so the box matches what is on screen.
- **Gesture.** A new `@MainActor enum ReferenceDragging` in PCBCanvasView.swift
  (the `TraceDrawing` pattern, so tests run the canvas's steps):
  `press(document, at:, tolerance:, scale:, visible:) -> (id: UUID, from:
  CGPoint)?` (hit; makes the chip the selection unless it is already selected;
  returns the reference's centre at the press) and
  `drag(document, id:, from:, by offset: CGSize)` →
  `document.moveReference(id, toCentre: from + offset)`.
- **Interaction.** New case `.moveReference(id: UUID, from: CGPoint)`. In
  `begin(at:)` under `.select`, before the existing item pick: if
  `NSEvent.modifierFlags.contains(.command)`, and `ReferenceDragging.press`
  hits, return it; else fall through unchanged. `handle` calls
  `ReferenceDragging.drag`; `end` needs no extra work (no selection press is
  pending).
- **Selection.** The press makes the part the selection (so the sidebar shows
  its Marks, and the reset button). If it is already one of several selected
  items, the selection is kept. Only the reference moves either way.
- **Reset.** In the Marks section, a `Button("Reset Reference Position")`
  calling `document.resetReference(chip.id)`, shown only when
  `placement.referenceOffset != .zero`.
- **Gate class.** Not byte-identity: behaviour is new. The expected diffs are
  none outside the new tests; the existing PCB tests pass unchanged.

Correctness risk is low: the data path is A's; B is input handling. The one
trap is the press moving the viewport or the part: the `.moveReference` case
bypasses `pressSelection`, `boardGroupBase` and `pan`.

Rejected alternatives:

- Make the reference a `BoardItem`: touches selection, delete, box-select, lock
  and clipboard for a feature whose edits are one point.
- Drag the reference with a plain drag when the pointer is on its text: the
  text sits on the body, where a plain drag moves the part today.
- Another modifier (Option, Control): the owner asked for Cmd.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Hit test and the gesture, tested

- [x] `Canvas/SchematicModel.swift`: add `referenceHit(at:tolerance:height:visible:)`
      (§ 3), next to `footprint(at:visible:)`.
- [x] `Canvas/PCBCanvasView.swift`: add `enum ReferenceDragging` (`press`,
      `drag`), the `.moveReference(id:from:)` case, the Cmd branch in
      `begin(at:)`, and the `handle` case; `end(translation:)` needs nothing.
- [x] Tests: new `schematicalTests/ReferenceDragTests.swift`, driving
      `ReferenceDragging` in the canvas's order inside `beginGesture(.pcb)` …
      `endGesture()`: a press on a reference's text hits it and a press on the
      pads or off the part does not; a Top part's reference hides a Bottom
      one under it; a hidden or layer-off reference can't be hit
      (`BoardVisibility`); a drag moves only the reference (the part's `centre`
      and `rotation` unchanged, a second selected footprint unmoved); the
      moved reference lands on the trace grid; the press selects the part and
      keeps a multi-selection; the whole press is one undo step named
      "Move Reference" and undo restores the zero offset; a locked part's
      reference moves; after the drag, moving the part by (10, 5) mm moves the
      reference's board centre by the same, and `rotateFootprint` carries it
      (A's `boardReferenceCentre == board.board(offset)`).

- [x] (added) `ReferenceDragging.drag` does nothing for a zero offset, so a
      Cmd-click with no movement doesn't re-snap the reference; tested by
      `testAPressWithNoMovementLeavesTheReference`.

Acceptance: the tests pass.
  Result: `** TEST SUCCEEDED **`; LockPositionTests 6, PCBUndoTests 34, ReferenceDragTests 9 → 49 tests, 0 failures.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p41-dd test -only-testing:schematicalTests/ReferenceDragTests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/LockPositionTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; the two existing classes prove the plain-drag and lock paths didn't change).
Commit: b751f543

### Phase 2 — Reset button and help

- [x] `Canvas/PCBSidebarView.swift` (Marks section, line 955-959): add
      `Button("Reset Reference Position") { document.resetReference(chip.id) }`
      shown only when `placement.referenceOffset != .zero`.
- [x] `Help/KeyboardHelp.swift`: in `pcb`'s "Selecting and moving" section add
      `entry("⌘-drag a reference", "Move only the reference text, snapped to the trace grid; it keeps its place on the part when the part moves or turns")`.
- [x] Tests: `KeyboardHelpTests.testPCBKeys` list gains `"⌘-drag a reference"`.

Acceptance: the key is listed.
  Result: `** TEST SUCCEEDED **`; KeyboardHelpTests 4 tests, 0 failures.
  Check: `xcodebuild … test -only-testing:schematicalTests/KeyboardHelpTests CODE_SIGNING_ALLOWED=NO` (same project, scheme and derived-data path) → `** TEST SUCCEEDED **` (est. 5 min).
Commit: be81b8d7

### Phase 3 — Landing tier (once)

- [x] Run the Validation Plan's gate table and record the observed values
      under it (§ Observed values).
- [x] Real-app check (below); record what was seen (§ Observed values).
- [x] Move `plan-41-A` and `plan-41-B` to `planning/completed/`.

Acceptance: every gate row shows its expected value; the real-app steps all
behave as written.
Commit: 13baca63

## Validation Plan

- Tests: `ReferencePositionTests` (A), `ReferenceDragTests`,
  `KeyboardHelpTests` (B).
- Coverage check: `ReferenceDragging` is in the test target (an `@testable`
  import of the app); a press that misses falls through to `boardItem`, which
  `LockPositionTests` and `PCBUndoTests` still cover.
- Runtime proof (real app, per AGENTS.md § Testing in the app): open
  `examples/8bit-cpu/AFU.schematical` (it has the 0603 references the todo
  names, planning/todo.md line 98), go to the PCB page, zoom on C1, then
  1. Cmd-drag "C1" off its pads → only the text moves; the part stays;
  2. plain-drag C1 → the part moves and the reference keeps its place on it;
  3. Rotate C1 (⌘R) → the reference turns around the body with it, text still
     horizontal;
  4. ⌘Z once → the reference returns to the body centre;
  5. Cmd-drag, then Reset Reference Position in the sidebar → back on the
     centre;
  6. Run the design rule check: C1's silkscreen warning is gone after the move;
  7. click the PCB page's Preview button (toolbar, `ContentView.swift:533`;
     it opens Board Preview, drawn by `BoardRenderer`) and see the reference
     at the moved place;
  8. Save, reopen: the reference is where it was left.
  Move the pointer to (0, 0) when done. Do not save the example.
- Doc sync: A's file-format and export docs; B changes none (the help is
  in-app). `docs/agent-control.md` is not touched (no tool change).
- Final gate (run ONCE, after the last phase), build dir `/tmp/p41-dd`:

| Gate | Command | Expected |
|---|---|---|
| App build | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p41-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` (est. 6 min) |
| App tests, touched areas | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p41-dd test -only-testing:schematicalTests/ReferencePositionTests -only-testing:schematicalTests/ReferenceDragTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/LockPositionTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/BoardSideTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` (est. 12 min) |

  The backend is untouched (`git diff --stat <plan-41-A first commit>^ -- backend` → empty), so no backend tier.

### Observed values (2026-11-14, tree at be81b8d7)

| Gate | Observed |
|---|---|
| App build | `** BUILD SUCCEEDED **` |
| App tests, touched areas | `** TEST SUCCEEDED **`: 118 tests, 0 failures (ReferencePositionTests 13, ReferenceDragTests 9, KeyboardHelpTests 4, PCBUndoTests 34, LockPositionTests 6, DesignRuleCheckTests 12, GerberExportTests 17, SchematicFileV5Tests 10, BoardPreviewTests 3, BoardSideTests 10) |
| Backend untouched | `git diff --stat 6757080a HEAD -- backend \| wc -l` → 0 |
| Main drift | `main` still at 6757080a (the plan's own commit) when merging, so no merge of main and no re-run |

Real app (the Debug build in `/tmp/p41-dd`, driven with a copy of the plan-32
drive tool that holds Cmd on mouse events), on `examples/8bit-cpu/AFU.schematical`,
PCB page at 100 %, part C9 (its reference was drawn over its pads):

1. Cmd-drag "C9" right by about 70 pt: only the text moved; the part stayed,
   selected (blue box), and the sidebar showed C9.
2. Plain drag of the part 30 pt left: the part moved and the reference moved by
   the same amount.
3. ⌘R: the part turned about pad 1; the reference went round with it (it
   was right of the part, now below), text still horizontal.
4. ⌘Z three times (rotate, part move, reference move): the reference was back on
   the body centre, over the pads, as at the start.
5. Cmd-drag again: "Reset Reference Position" showed under Marks (sidebar
   scrolled); clicking it put the reference back on the body centre and the
   button went.
6. Cmd-drag again, then Design Rule Check (131 issues): C2–C8, R1 and U44 have
   "Reference over pad"; C9's only silkscreen issue is its pin-1 mark.
7. Preview button: Board Preview drew "C9" at the moved place, beside the part.
8. Save As `/tmp/p41-afu.schematical` (the example was not saved: `git status`
   clean): the file has `"refX" : -3.365` and no `refY` for C9 (the part is
   at 180°, so the screen-right move is -x in its frame). Quit, relaunched,
   opened it: C9 was where it was left.
   The pointer went to (0, 0) at the end.

## Decisions

- Cmd-drag on the reference moves it, not the part — the owner's brief,
  2026-11-14. (§ 1)
- Snap to the trace grid; Lock Position doesn't freeze the reference; a
  footprint swap resets it — decided by the owner, 2026-11-14 (plan-41-A §
  Decisions).
- Moving a reference selects its part, and a multi-selection is kept — the
  author's call, since the sidebar's reset button needs the part selected. The
  reference is not a new selectable item. (§ 3)

## Corrections

- The Validation Plan names C1 for the real-app run; the part used was C9,
  because it was the one in view on the 100 % page and has the same 0603
  reference-over-pad warning (the todo lists C1–C9). Nothing else differs.
- Step 8's Save As panel treats a typed `/` as its Go-to field, so the path
  was entered as folder (`/`, `tmp`) then name. Closing the only window quits
  the app, so the reopen was a relaunch. Neither affects the design.
- Phase 1 gained an unlisted task (the zero-offset guard in
  `ReferenceDragging.drag`); it is ticked there.

## Summary

B is input handling over A's data: one hit test, one `Interaction` case, one
sidebar button, one help line. Untouched: selection and delete semantics, the
other tools, the part's own drag.
