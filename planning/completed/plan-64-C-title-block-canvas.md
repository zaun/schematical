# plan-64-C: Document Info — the title block on the schematic

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-64-B

The form from B gets an **Include on Schematic** toggle. On, a **title block** appears on the Schematic page,
bottom-right of the drawing: a framed table of the title, author, revision, created, updated and description.
It selects, drags (alone or with other items) and draws like a text block; double-click opens Document Info;
the sidebar shows the same editable form while it is selected. Delete (⌘⌫, Delete, the Eraser) on it only
switches it off: the information stays.

References:

- plan-64-A (`DocumentInfo.titleBlock`, `setInfo…`), plan-64-B (`DocumentInfoForm`, `DocumentInfoTarget`, `ChildWindowOpener`).
- `schematical/schematical/Canvas/SchematicNote.swift` — the precedent for a movable, grid-framed, selectable block
  (`rect`, `lineOrigin`, the metrics lock).
- `schematical/schematical/Canvas/SchematicCanvasView.swift` (`NoteLayer` :1041-1085, the press handlers :780-800).
- `schematical/schematical/Drawer/SchematicSidebarView.swift` (`Content`, `content(for:)`, `NotePanel`).

## Prerequisites

Stated once in plan-64-A; re-run its commands. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-64-B is complete | `grep -c "^Commit: —" planning/plan-64-B-document-info-window.md planning/completed/plan-64-B-document-info-window.md 2>/dev/null` → `0` for whichever file exists | MET (2026-11-25: 0 unticked boxes, 0 pending Commit lines) |

## 1. Goal

- Turning *Include on Schematic* on draws the title block at the bottom-right of the existing drawing (an empty
  schematic: at (0, 0)); the block can be selected, moved (one undo step per drag), and hidden by Delete; the
  sidebar edits the same fields while it is selected. Hiding keeps every field; showing again places it bottom-right
  again. Saved files keep its place (A's `titleBlock`).

### Non-goals (explicit constraints)

- The PCB page shows no title block. Exports, the agent tools and docs are D.
- The block is not copied, cut or pasted: only Delete-class actions hide it (below). It has no font-size or frame
  setting: it is always framed and its text sizes are fixed.
- No change to text blocks (`SchematicNote`) or their file format.

## 2. Current State

- `SchematicItem` has six cases, `chip wire junction note label bus` (`Selection.swift:6-17`); no switch over it has
  a `default`, so the compiler lists each site that needs a new case. The known ones: `pruneSelection`
  (`SchematicModel.swift:2018-2027`), `schematicGroupBase` (:2686-2704), `deleteSchematicSelection` (:6724-6731),
  `schematicCopy` (`SchematicClipboard.swift:68-76`), `SelectionRows.schematic` (`SelectionList.swift:36`),
  the automation's `position` (`AutomationSchematic.swift:399`) and name switch (`AutomationRead.swift:398-410`)
  (D handles the tools; C only keeps them compiling, with `.titleBlock` named `"titleblock"` and positioned at its origin).
- A note: `SchematicNote.rect` is the text plus padding grown to whole grid steps (`SchematicNote.swift:73-83`);
  line widths come from CoreText under `metricsCache`'s lock because `rect` runs on the main thread and on
  SwiftUI's render thread at once (`SchematicNote.swift:30-49`).
- Drawing: `NoteLayer.paint` fills the frame `.paper`, strokes it `.secondary` (accent when selected), draws lines
  with `PaintedText(…, monospaced: true)` (`SchematicCanvasView.swift:1059-1083`); `SchematicScene.paint` calls it
  first (`SchematicExportScene.swift:36`); `SchematicScene.isEmpty` lists notes (:32).
- Interaction: a press on a note → `document.pressSelection(.note(id), shift:)` then `.moveGroup(base:)`
  (`SchematicCanvasView.swift:797-799`); a double-click on a note selects it and sets `editingNoteID` (:207-214);
  the drag applies `moveSchematicGroup(_:by:)` (`SchematicModel.swift:2751`), one "Move" step per drag; the Eraser
  deletes the note under the click (:6685); in play mode only buttons and panning work (:553-560).
- `onOpenRegisters` is the precedent for a canvas callback that opens a child window
  (`SchematicCanvasView.swift:63`, `ContentView.swift:430-434`).
- `SchematicSidebarView.Content` is `library | chip | note | label | port | bus | selection` (:19-27); a lone
  note → `.note(id)` → `NotePanel` in a `Form` (:96-103).

### Measured populations

| What | Count | Command |
|---|---|---|
| Files that match a `SchematicItem.note` in a `case` or `if case` | 6 | `grep -rln "case let .note(\|case .note" schematical/schematical` → 8 files, of which `UndoHistory.swift` (`UndoKey`) and `Editor/FieldEditors.swift` (an unrelated enum) are not `SchematicItem` users |
| `SchematicScene(` constructions | 5 | `grep -rn "SchematicScene(" schematical/schematical \| wc -l` → 5 (`ImageExport` ×2, `PrintExport` ×3) |
| Painter implementations the block draws through | 3 | `grep -rn "struct [A-Za-z]*: SchematicPainter" schematical/schematical` → `SVGPainter`, `BoundsPainter`, `ScreenPainter` |

### Verified properties

- `PaintedText` takes `size`, `weight`, `monospaced`, `ink` (`SchematicPainter.swift:54-73`), so the block's
  labels (8 pt, `.secondary`) and values (12 pt monospaced, `.primary`) need no new painter API.
- `BoundsPainter` measures ink bounds from a `SchematicScene` without drawing (`SchematicExportScene.swift:101-162`),
  so "bottom-right of the drawing" is `SchematicScene(self)` painted into one: no new geometry code.
- Group moves go through `moveSchematicGroup`, which is one `recordEdit` per frame inside a gesture, so a drag is
  one undo step and the new `documentInfo.titleBlock` change is part of it through A's diff key.

## 3. Design Overview

**Layout (`TitleBlockLayout`, pure).** Width is fixed at 360 (36 grid steps). Rows, top to bottom, each a
row of cells separated by 1-pt lines:

| Row | Cells (width) |
|---|---|
| 1 | Title (360) |
| 2 | Author (240), Revision (120) |
| 3 | Created (180), Updated (180) |
| 4 | Description (360) |

A cell is its label (8 pt, `.secondary`, one line) over its value wrapped to the cell's width less 2 × 6 pt of
padding (title 14 pt semibold; others 12 pt monospaced). Wrapping splits on `\n`, then greedily on spaces, and
breaks a word longer than the line by characters; an empty value is one empty line. A row is as tall as its tallest
cell, rounded up to a whole grid step, so the frame and every separator lie on the grid. Dates show
`DocumentInfo.dateText`. All six cells always show (an empty one is a blank cell, as on a printed title block).

**Where the correctness risk is:** (1) the wrap measure runs on two threads: it must use `SchematicNote`'s
lock-protected metrics and CoreText measure, not AppKit text measuring (the crash noted at
`SchematicNote.swift:30-38`); (2) the new `SchematicItem` case must be handled in every switch, and `Delete`
must not delete anything but the flag. Both have tests.

**Not a premise:** nothing here depends on byte-identity; files without a title block are unchanged (A).

**Behaviors (decided):**

- *Show:* `setTitleBlockShown(true)` sets `titleBlock` to `(bounds.maxX − 360, bounds.maxY + 20)` snapped, where
  `bounds` is the drawing's ink bounds (`BoundsPainter` over `SchematicScene`, taken before the block exists);
  an empty schematic gets `(0, 0)`. One step, "Show Title Block". It does not select the block.
- *Hide:* `setTitleBlockShown(false)` sets it to nil, drops it from the selection. One step, "Hide Title Block".
  ⌘⌫ / Delete on a selection that includes it, and an Eraser click on it, do exactly this (inside the same step
  as the other deletions).
- *Cut* copies what is copyable and deletes everything selected except the block (it stays shown); *copy* ignores
  it; a selection of only the block makes ⌘C and ⌘X do nothing.
- *Double-click* on it opens the Document Info window (`onOpenDocumentInfo`).
- *Draw order:* over the grid and **under** text blocks, buses and wires, so a note can sit on the block. Hit order is
  the reverse: notes first, then the block.
- *Sidebar:* a lone selected block shows `DocumentInfoForm` (with the toggle) in a `Form`, `.id` stable.
- *Play mode:* nothing new: the block can't be selected or moved while the simulation runs (existing guard).

Rejected alternatives:

- A block per page template (paper size, margins): the request is a movable block "like text".
- Letting the user pick which cells show: more UI for little value; blank cells read as a real title block.
- Keeping the block's last position when it is shown again: the answer to placement was "bottom-right of the
  drawing"; a drawing that grew since is better served by recomputing.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. Use `- [~]` for
> partly done. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Layout, drawing, model API

- [x] `SchematicNote.swift`: change `private static func width(of:in:)` and `private static func metrics(_:)`
      to internal (`static`), keeping their lock and doc comments; no behavior change. — done
- [x] New `Canvas/TitleBlockLayout.swift`: `nonisolated struct TitleBlockLayout` with `init(_ info: DocumentInfo,
      origin: CGPoint)`; `rect`, `cells: [Cell]` (`rect`, `label`, `lines`, `size`, `weight`, `monospaced`, `textOrigin`),
      `separators: [(CGPoint, CGPoint)]`; `static let width: CGFloat = 360`; the wrap as § 3; heights rounded up to
      `SchematicGrid.spacing`. — done
- [x] `SchematicCanvasView.swift`: `struct TitleBlockLayer: View` (like `NoteLayer`) with `static paint(_ painter:
      inout P, info:, isSelected:, isHovered:)`: frame filled `.paper`, stroked `.primary` 1.5 (accent 2 when
      selected), separators `.secondary` 1, hovered underlay `Ink.accent.opacity(0.35)` 5 as a note's; text per cell.
      Insert it in `worldLayer` between `GridBackground` and `NoteLayer` (:259-266), shown when `titleBlock != nil`. — done
- [x] `SchematicExportScene.swift`: `var titleBlock: DocumentInfo?` (set when shown); `isEmpty` is false when it is
      set; `paint` calls `TitleBlockLayer.paint` before `NoteLayer.paint`. — done (stored as the layout: `TitleBlockLayout?`)
- [x] `SchematicModel.swift`, in § Document info (A): `var titleBlockRect: CGRect?`; `func titleBlock(at:) -> Bool`
      (point in rect); `func setTitleBlockShown(_ on: Bool)` (rules in § 3; uses `SchematicScene(self)` and
      `BoundsPainter`); `var selectedTitleBlock: Bool`. — done; selectedTitleBlock moves to Phase 2 (it needs the new SchematicItem case)
- [x] `DocumentInfoWindow.swift` (B): in `DocumentInfoForm`, a first `Section` with `Toggle("Include on Schematic",
      isOn: Binding(get: { document.documentInfo.titleBlock != nil }, set: { document.setTitleBlockShown($0) }))`. — done
- [x] Tests: new `schematicalTests/TitleBlockTests.swift` (`@MainActor`, `TestSupport.newDocument()`): layout —
      width 360, every cell/separator coordinate a multiple of 10 relative to `origin`, a 200-character description
      wraps to more than 5 lines and a 40-character unbroken word breaks by characters, an empty info has the four-row
      minimum height; show/hide — empty doc shows at (0, 0), a doc with a part shows with `maxX` equal to the drawing's
      ink `maxX`, hide keeps `title`, each is one undo step named "Show Title Block" / "Hide Title Block", undo/redo
      restore; painting — with a `RecordingPainter` (`SchematicSceneTests.swift:10-35`) the text calls include the
      title, the six labels, and the formatted dates. — done: xcodebuild test TitleBlockTests SchematicSceneTests SchematicNoteTests -> ** TEST SUCCEEDED **, 29 tests 0 failures

Acceptance: the toggle shows and hides the block with the right placement, steps and drawn text.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TitleBlockTests -only-testing:schematicalTests/SchematicSceneTests -only-testing:schematicalTests/SchematicNoteTests` → `** TEST SUCCEEDED **` (est. 9 min; the two existing classes prove the note metrics change and the scene still behave).
Commit: b03f4c91

### Phase 2 — Selecting, moving, hiding, sidebar

- [x] `Selection.swift`: `case titleBlock` on `SchematicItem` (doc comment: "the Document Info title block");
      `SchematicGroupBase.titleBlock: CGPoint?`. — done
- [x] `SchematicModel.swift`: handle the new case in every switch the compiler flags, with: `pruneSelection` — kept
      only while `documentInfo.titleBlock != nil`; `schematicItems(in:)` (:1888) and `selectAllSchematic` (:1963) — include it
      when its rect meets the box / when shown; `schematicGroupBase` — records `documentInfo.titleBlock`;
      `moveSchematicGroup` (:2751) — moves `documentInfo.titleBlock` by the snapped delta; `deleteSchematicSelection`
      (:6712) — sets `documentInfo.titleBlock = nil` and counts as something deleted; its Eraser path (:6685)
      likewise; add `deleteSchematicSelection(named:, includingTitleBlock: Bool = true)`. — done (box select, select all, prune, group base, move, delete, eraser; includingTitleBlock added)
- [x] `SchematicClipboard.swift:68`: `case .titleBlock: break` (not copied). `ContentView.cutSelection` (:267):
      `document.deleteSchematicSelection(named: "Cut", includingTitleBlock: false)`. — done
- [x] `SchematicCanvasView.swift`: a press on the block (after the `note(at:)` branch, :797) →
      `pressSelection(.titleBlock, shift:)` and `.moveGroup`; the double-click branch (:207) → select it and call
      `onOpenDocumentInfo()` (a new `var onOpenDocumentInfo: () -> Void = {}` beside `onOpenRegisters`, :63);
      `hoveredTitleBlock` state like `hoveredNoteID` (:86, :748); `selectedNoteIDs`-style `isTitleBlockSelected`.
      `ContentView.swift` passes `onOpenDocumentInfo: { menus.openDocumentInfo?() }` next to `onOpenRegisters` (:430). — done
- [x] `AutomationSchematic.swift` `position` (:399): `case .titleBlock: return document.documentInfo.titleBlock`;
      `AutomationRead.swift` `selectionText` name (:404): `case .titleBlock: return "titleblock"`. (Only to keep the
      switches exhaustive; D adds the tool surface.) — done
- [x] `SelectionList.swift:36`: a `SelectionRow(item: .titleBlock, title: "Title Block", detail: preview(title))`. — done
- [x] `SchematicSidebarView.swift`: `Content.titleBlock`; `content(for:)` returns it when `document.selectedTitleBlock`
      (after the note check); the body shows `Form { DocumentInfoForm(document:, isLocked: simulation.isRunning) }
      .formStyle(.grouped)` for it; update the type's doc comment (:3-8). — done
- [x] Tests (`TitleBlockTests`): hit test — a point inside selects through `pressSelection(.titleBlock…)`, outside
      does not; a note over the block is hit first; box select and Select All include it only when shown; a group
      drag of the block plus a part moves both by the snapped delta and undoes in one step; ⌘⌫ with block + part
      selected removes the part and hides the block (info kept); the Eraser at a point in the block hides it;
      cut with block + part leaves the block shown and removes the part; copy of the block alone is nil; the sidebar
      `content(for:)` is `.titleBlock` for a lone selection and `.selection` for block + part; `SelectionRows` has the
      row; hiding with the block selected clears the selection. — done: xcodebuild test TitleBlockTests SelectionModelTests SelectionSidebarTests SchematicClipboardTests EditMenuTests -> ** TEST SUCCEEDED **

Acceptance: every way of selecting, moving and deleting the block behaves as § 3 says, and undo restores it.
  Check: `xcodebuild … test -only-testing:schematicalTests/TitleBlockTests -only-testing:schematicalTests/SelectionModelTests -only-testing:schematicalTests/SelectionSidebarTests -only-testing:schematicalTests/SchematicClipboardTests -only-testing:schematicalTests/EditMenuTests` → `** TEST SUCCEEDED **` (est. 12 min: five test classes over the changed selection code).
Commit: ac6f2db8

### Phase 3 — Real-window pass

- [x] Build and launch the app; open `examples/6502.schematical` (its drawing is large, so the placement is
      visible). ⌘I → tick *Include on Schematic* → the block appears under the bottom-right of the drawing;
      click it → the sidebar shows the form and typing a title redraws the block; drag it → one Undo moves it back;
      ⌘⌫ → it disappears and the window's toggle is off, the title is still there; untick/tick → it returns
      bottom-right; double-click it → the Document Info window comes to the front. Record what each step showed
      under this phase. Quit the app, move the pointer to (0, 0), and do not save the example (AGENTS.md). — done 2026-11-25 (a copy of the example in /tmp, so autosave could not touch the repo): Include on Schematic put the block under the right end of the drawing (its right edge in line with the drawing right edge, screenshot at 25%); on an empty document it appears at the origin; a click selected it (blue frame) and the sidebar showed Include on Schematic, the fields and the dates; typing in the fields redrew the block live; a drag moved it and one Edit > Undo put it back; Cmd+Delete hid it, the Document Info toggle went off and the title, author and revision were still there; the toggle on again brought it back; a double-click on it brought the Document Info window to the front. The edited copy was autosaved and showed both dates stamped. App quit, pointer parked

Acceptance: the pass above, each step's observed result written in this phase.
  Check: the real-window pass (est. 15 min); no automated check can see the drawn result.
Commit: 686fe631

## Validation Plan

- Tests: `TitleBlockTests` (new), plus the existing classes named in each phase's check.
- Coverage check: the xcodebuild output names `Test Case '-[schematicalTests.TitleBlockTests …]' passed` for the new tests.
- Runtime proof: Phase 3.
- Doc sync: `Help/KeyboardHelp.swift` — a schematic-page line `entry("Click the title block", "Select it; drag to move, ⌘⌫ hides it, double-click opens Document Info")` in the selection section; `docs/file-format.md` already covers `titleBlock` (A).
- Final gate: runs once in plan-64-D.

## Decisions

- The block is placed at the bottom-right of the drawing when turned on — decided by the owner, 2026-11-25.
- Deleting it only toggles it off — decided by the owner, 2026-11-25 (request).
- Fixed width 360, fixed text sizes, always framed, all six cells always shown — author's call.
- Drawn under text blocks, hit after them — author's call (a note can annotate the block).
- Cut and copy do not take the block — author's call (it is one per document, and Delete already hides it).
- An empty schematic's block is placed at (0, 0) — author's call (no drawing to anchor to).

## Corrections

- Phase 1: `selectedTitleBlock` (listed with the Phase 1 model API) needs the new `SchematicItem.titleBlock` case, so
  it was added in Phase 2 with the case. `SchematicScene` stores the title block as its `TitleBlockLayout?`
  (the layout the painter needs) rather than the `DocumentInfo` the plan named.
- Phase 3: the pass on the 6502 example showed the block at the bottom-right of the drawing; on an empty document the
  block appears at the origin, as decided. Autosave of the edited copy stamped Created and Updated on it, as designed.

## Summary

The risk is the new `SchematicItem` case reaching every switch (the compiler finds each one) and the wrap
measure being thread-safe (it reuses the notes' lock). Text blocks, wires and the PCB are untouched.
