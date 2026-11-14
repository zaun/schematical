# plan-41-A: Reference position (data, drawing, file)

Last updated: 2026-11-14
Overall Effort: large (3h–1d)
Effort: medium (1h–2h)
Depends on: nothing

Each footprint's reference ("U1", "R3") is drawn on the PCB at the centre of
its body, with no way to move it. plan-41 lets the user move a reference
off the body (Cmd-drag on the PCB page), so it clears pads and other
silkscreen. A saves where it is: a per-footprint offset, in the footprint's
own frame, so the reference follows the part when it moves, turns or goes
to the other side. A makes the offset real in the model, the four places
that draw a reference, the file and the docs; B adds the gesture, the
sidebar's reset button and the help.

Outcome of A: a part whose placement carries a reference offset draws its
reference there on the PCB page, in the Gerber silkscreen, in the board
preview and in the design rule check, and the offset survives save and open
and undo. Nothing in the app sets it yet except the document functions B's
gesture will call, which A's tests call directly.

References:

- `docs/file-format.md` (the `pcb` row, line 130), `docs/export.md`
  (line 62, silkscreen references).
- `schematical/schematical/Canvas/BoardModel.swift` (`BoardPlacement`,
  `PlacedChip.boardBody`).
- `planning/plan-40-H-transistor-app.md` (§ Prerequisites, the xcodebuild
  command shape).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
B points here.

| Must be true | Command | Status |
|---|---|---|
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p41-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-14: `** BUILD SUCCEEDED **`) |
| No other plan is rewriting the reference drawing | `git log --oneline -5 -- schematical/schematical/Canvas/PCBCanvasView.swift schematical/schematical/Document/GerberExport.swift` shows no commit named for a plan other than plan-41 since this plan's date | MET (2026-11-14: newest commit on those files is 4059af27 "PCB: hovering a pad…", not a plan) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `BoardPlacement` carries `referenceOffset` (mm, footprint frame, default
  zero). Every reference drawn or checked on the PCB uses
  `PlacedChip.boardReferenceCentre`, not the body centre.
- The offset is saved as `refX` / `refY` in the part's `pcb` object, only when
  not zero, and read back.
- The document has `moveReference`, `resetReference`; a footprint-changing
  edit clears the offset.

### Non-goals (explicit constraints)

- The reference text stays horizontal on screen and in the Gerber, as today;
  only its position follows the part's turn and mirror. (`drawMarks`
  does not turn it: PCBCanvasView.swift:562-567.)
- The file `version` does not change (5 with custom parts, else 4): `refX` and
  `refY` are optional keys like `locked`. An older build ignores them and draws
  the reference on the body centre.
- Nothing changes for a part whose offset is zero: its file, its Gerber and its
  design rule check are byte-identical to today's.
- No gesture, sidebar or help change (B). No automation tool change.
- The schematic's reference label (`ReferenceLabel`) is untouched.

## 2. Current State

- `BoardPlacement` (Canvas/BoardModel.swift:35): `centre` is the body centre;
  `board(_ local:)` takes a footprint point (origin = body centre, y down),
  mirrors it on Bottom, turns it, and adds `centre`.
- `PlacedChip.boardBody` (BoardModel.swift:159) is `board.board(centre: .zero,
  size: footprint.body)`: its midpoint is `board.board(.zero)`, which is
  `centre`.
- Four places put the reference at that midpoint:
  - `PCBCanvasView.drawMarks` (Canvas/PCBCanvasView.swift:564)
    `layer.translateBy(x: body.rect.midX, y: body.rect.midY)`;
  - `BoardRenderer` silkscreen (BoardPreview/BoardRenderer.swift:447), the
    same call;
  - `GerberExport.drawSilkscreen` (Document/GerberExport.swift:183) passes
    `centre: CGPoint(x: body.rect.midX, y: body.rect.midY)` to
    `referenceGlyphs`;
  - `DesignRuleChecker.checkSilkscreen` (DesignRuleCheck/DesignRuleCheck.swift:672)
    builds the reference's `rect` around `body.rect.mid*`.
- The reference's size lives twice: `GerberExport.referenceSize = 1.0`
  (GerberExport.swift:28) and the private `DesignRuleChecker.referenceHeight`
  (DesignRuleCheck.swift:69) with `referenceSize(_:)` (line 752). Nothing else
  uses either (`grep -rn 'referenceSize\|referenceHeight' schematical schematicalTests`
  shows only those, plus the unrelated `BoardTextGeometry.referenceSize`).
- The file's `Placement` (Document/SchematicFile.swift:128) has optional keys
  (`origin`, `outline`, … `locked`); `SchematicFile.placement(_:reference:)`
  (line 1217) turns it into a `BoardPlacement`; the write is at line 603.
- Undo's `DocumentState` holds `chips` (SchematicModel.swift:1586), and a
  `PlacedChip` holds its `BoardPlacement`, so a new field in `BoardPlacement`
  is undone and redone with no extra code.
- Three edits swap a part's footprint and keep its `BoardPlacement`:
  `setPartFootprint` (SchematicModel.swift:1880), `setFootprintStyle`
  (line 2477), and `setHeaderSize` (line 1826, through `PlacedChip.resized`,
  line 724).

### Measured populations

| What | Count | Command |
|---|---|---|
| Reference draw/check sites using the body midpoint | 4, one in each of 4 files | `grep -rc 'body.rect.mid' Canvas BoardPreview Document DesignRuleCheck` (run in `schematical/schematical`) → PCBCanvasView 1, BoardRenderer 1, DesignRuleCheck 1, GerberExport 1 |
| Footprint-swapping edits to reset | 3 | read above |

### Verified properties

| Claim | How verified |
|---|---|
| A footprint-frame offset `o` lands at `board.board(o)` and follows rotation and mirror | Read `BoardPlacement.board(_:)` (BoardModel.swift:62-66): mirror x on Bottom, `rotate`, add `centre`. |
| `board.board(.zero)` equals the body rect's midpoint | Read `board(centre:size:)` (BoardModel.swift:70-76): rect centred on `board(local)`. |
| The reference isn't turned with the part today | Read `drawMarks` (PCBCanvasView.swift:562-567) and `referenceGlyphs` (GerberExport.swift:241-249): only translate, and mirror on Bottom. |
| `showsMark(.hidden, …)` is false, so a hidden reference draws nothing | Read BoardSettings.swift:443-451. |

## 3. Design Overview

- **Frame.** The offset is in the footprint frame (mm, from the body centre,
  before mirror and turn): `referenceOffset`. The reference's board centre is
  `placement.board(referenceOffset)`. Moving the part, turning it, or sending it
  to the other side moves the reference with it for free. The text itself stays
  horizontal and, on Bottom, mirrored in place (as today).
- **Inverse.** B needs board point → offset, so A adds
  `BoardPlacement.local(_ boardPoint:)`, the exact inverse of `board(_:)`.
- **One place for the reference's size.** A new `nonisolated enum
  BoardReference` in BoardModel.swift holds `height = 1.0` and `size(of:
  String, height:) -> CGSize` (the NSFont measure now at DesignRuleCheck.swift:752).
  The Gerber and the checker use it; the two copies go.
- **Rect.** `PlacedChip.boardReferenceCentre` and
  `boardReferenceRect(height:)` (centred on the centre, `height` default
  `BoardReference.height`) are the one definition of where the reference is.
  B's hit test uses the same rect.
- **Edits.** `moveReference(_ id:, toCentre:)` snaps the board point to the
  trace grid (decided: § Decisions), converts to the footprint frame with
  `local`, rounds to 0.0001 mm (so the file doesn't carry float noise), and
  stores it. `resetReference(_ id:)` zeroes it. Neither checks `locked`.
- **Byte-identity is the gate for the zero offset only.** Parts with no offset
  must produce today's file, Gerber and checker output; that is checked by the
  existing tests passing unchanged. Parts with an offset are expected to
  differ — that is the feature.

Correctness risk concentrates in the inverse (a wrong sign on Bottom or at
90°/270° moves a reference to the wrong side of the part); Phase 1's test
covers all 8 rotation × side cases.

Rejected alternatives:

- Store the offset on the board (absolute mm): the reference would stay put
  when the part moves; the owner wants it relative.
- Store it relative to the pad-1 origin: the origin setting changes between
  files and is user-editable; the body centre is the one point every footprint
  has, and the one the reference uses today.
- Turn the text with the part: not asked for, and changes the Gerber of every
  turned part.
- Add a new `MarkLayer` case or reuse `reference`'s string: the existing key is
  the layer; the offset needs its own keys.

## 4. Detailed Design

`BoardPlacement` gains `var referenceOffset = CGPoint.zero` after `reference`.
It is `Hashable`, so equality and undo include it.

`local(_ p: CGPoint) -> CGPoint`: subtract `centre`, rotate by `-rotation`
(`BoardPlacement.rotate(_, by: (360 - rotation) % 360)`), then negate x on
Bottom.

File keys: `refX`, `refY` (Double, mm), written in `Placement` only when the
offset isn't zero. A read rejects a value with |v| above
`BoardSettings.sizeRange.upperBound` (1000) with
`SchematicFileError.badPCB("<reference> refX <v> must be within 1000 mm")`; a
missing one is 0.

## Compatibility / Format Impact

New optional keys `refX`, `refY` in a part's `pcb`. Version unchanged. Older
builds ignore them. A zero offset writes nothing, so every existing file
round-trips byte for byte.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands. `- [~]` is
> partial; a moot task is struck through with the evidence. **An unticked box
> means NOT DONE.**

### Phase 1 — Offset in the model, and the edits

Adds the field, the inverse, the shared size and the document functions. No
caller draws it yet, so it lands alone.

- [x] `Canvas/BoardModel.swift`: add `referenceOffset` to `BoardPlacement`;
      add `local(_:)` (§ 4); add `enum BoardReference { static let height =
      1.0; static func size(of:height:) }` (the measure from
      DesignRuleCheck.swift:752, scaled to `height`); add `PlacedChip`
      `boardReferenceCentre: CGPoint?` (`board.board(board.referenceOffset)`)
      and `boardReferenceRect(height:) -> CGRect?`.
- [x] `Canvas/SchematicModel.swift`: add `moveReference(_ id: UUID, toCentre
      point: CGPoint)` (`recordEdit(.pcb, "Move Reference")`; snap `point` to
      `board.traceGrid` with `BoardGrid.snap`; `local`; round to 0.0001; no
      change when equal) and `resetReference(_ id: UUID)`
      (`recordEdit(.pcb, "Reset Reference")`). Neither checks `locked`.
- [x] `Canvas/SchematicModel.swift`: clear `referenceOffset` in
      `setPartFootprint`, `setFootprintStyle` and `setHeaderSize` (after the
      chip is resized), inside their existing `recordEdit`.
- [x] `DesignRuleCheck/DesignRuleCheck.swift`: delete `referenceHeight` and
      `referenceSize(_:)`; use `BoardReference`. `Document/GerberExport.swift`:
      delete `referenceSize`; use `BoardReference.height`.
- [x] Tests: new `schematicalTests/ReferencePositionTests.swift`:
      `local` inverts `board` for every rotation (0/90/180/270) × side
      (top/bottom) on a non-square footprint; `moveReference` snaps to the
      trace grid and survives rotation (the reference's board centre moves with
      a rotated part: rotate, assert `boardReferenceCentre == board.board(offset)`);
      Bottom side mirrors the offset's x; `resetReference` restores zero;
      the three footprint edits clear it; `moveReference` works on a locked
      footprint; one undo step named "Move Reference", redo too.

Acceptance: those tests pass.
  Result: `** TEST SUCCEEDED **`; ReferencePositionTests 9, DesignRuleCheckTests 11, GerberExportTests 16 → 36 tests, 0 failures.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p41-dd test -only-testing:schematicalTests/ReferencePositionTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/GerberExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: build plus three test classes; the two existing classes prove the zero-offset output is unchanged).
Commit: 9640a841

### Phase 2 — Draw and check at the offset

Moves the four reference sites onto `boardReferenceCentre` / `boardReferenceRect`.

- [x] `Canvas/PCBCanvasView.swift:564`: translate to
      `chip.boardReferenceCentre` (the guard already has `chip.board`).
- [x] `BoardPreview/BoardRenderer.swift:447`: the same.
- [x] `Document/GerberExport.swift:183`: pass `chip.boardReferenceCentre`.
- [x] `DesignRuleCheck/DesignRuleCheck.swift:672`: the `Ink` rect is
      `chip.boardReferenceRect()`.
- [x] Tests: in `GerberExportTests` a part with an offset has its reference
      glyphs centred on `board.board(offset)` (reference glyph bounds' centre
      within 0.05 mm), and the same part with zero offset still centres on the
      body (already covered by the existing test); in `DesignRuleCheckTests`
      a reference that overlaps a pad at zero offset raises the silkscreen
      warning, and after `moveReference` to clear air does not.

Acceptance: moved references are drawn and checked where they are.
  Result: `** TEST SUCCEEDED **`; ReferencePositionTests 9, DesignRuleCheckTests 12, GerberExportTests 17 → 38 tests, 0 failures. Check: same xcodebuild command as Phase 1 (est. 8 min; incremental build).
Commit: 2e8ab127

### Phase 3 — Save, open, docs

- [x] `Document/SchematicFile.swift`: `Placement` gets `refX: Double?`,
      `refY: Double?`; the write (line 603) sets them when the offset is not
      zero; `placement(_:reference:)` reads them with the range check (§ 4).
- [x] `docs/file-format.md` line 130: add `refX`, `refY` to the `pcb` row
      ("the reference text's centre, in mm from the body centre in the
      footprint's own frame (before the side's mirror and the rotation); written
      only when not zero; missing means 0; older builds ignore them and draw it
      on the body centre").
- [x] `docs/export.md` line 62: "references (1 mm, centred on the body, or
      where the part's reference was moved)".
- [x] Tests: in `ReferencePositionTests`: save then open keeps the offset; a
      zero offset writes neither key (the encoded JSON contains no `refX`); a
      file with `refX: 5000` is refused with the `badPCB` message; a file with
      only `refX` reads `refY` as 0.

Acceptance: the offset round-trips and the zero case is unchanged.
  Result: `** TEST SUCCEEDED **`; PCBUndoTests 34, ReferencePositionTests 13, SchematicFileV5Tests 10 → 57 tests, 0 failures.
  Check: `xcodebuild … test -only-testing:schematicalTests/ReferencePositionTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests CODE_SIGNING_ALLOWED=NO` (same project, scheme and derived-data path as Phase 1) → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 0d5b55d8

## Validation Plan

- Tests: `ReferencePositionTests` (all 8 rotation × side cases, snap, reset,
  undo, locked, footprint edits, file round-trip, range refusal), plus the
  Gerber and checker cases of Phase 2.
- Coverage check: the four edited drawing sites are each hit by a test with a
  non-zero offset (Phase 2's); the BoardRenderer site has no test
  (`grep -n 'BoardRenderer\|silkscreen' schematicalTests/BoardPreviewTests.swift` → no
  match) and is covered by B's real-app check.
- Doc sync: `docs/file-format.md`, `docs/export.md` (Phase 3).
- Final gate: B's landing phase runs the landing tier once for plan-41.
  A runs only the scoped checks above.

## Decisions

- The reference snaps to the trace grid (0.3175 mm on the default grid) —
  decided by the owner, 2026-11-14. Fine enough to clear 0603 pads. (§ 3)
- Lock Position fixes the part only; a locked footprint's reference can still
  be moved — decided by the owner, 2026-11-14. (§ 3)
- A footprint swap (choice, style, header size) resets the offset to the body
  centre — decided by the owner, 2026-11-14. (Phase 1)
- The offset is in the footprint frame, so the reference follows the part's
  moves, turns and side — the owner's brief, 2026-11-14. (§ 3)

## Corrections

Found before any code was written (fixed in the plan file before its first
commit): § 2 cited `setPartFootprint` as "SchematicModel.swift:2160-2187
region"; it is at line 1880. Phase 1's Coverage line said `BoardPreviewTests`
exercises `BoardRenderer`; it doesn't (`grep -n 'BoardRenderer\|silkscreen'
schematicalTests/BoardPreviewTests.swift` → no match), so the renderer site is
covered by B's real-app check, which is what the plan now says.

Found during execution:

- Phase 1's first recorded result mislabelled the per-class counts (11/16/9).
  The real ones, alphabetical by class, are DesignRuleCheckTests 11,
  GerberExportTests 16, ReferencePositionTests 9 (36 in all, as recorded);
  fixed in the Phase 1 Result line.
- `BoardReference.size(of:height:)` needs `NSFont`, so `BoardModel.swift` gained
  `import AppKit` (it imported only `CoreGraphics`); the plan didn't say.
- Phase 3's zero-axis test could not set an offset directly: `chips` is
  `private(set)` on the document. The test opens a file with only `refX` and
  saves it instead, which also exercises the read path.
- Phase 3's first commit (`0d5b55d8`) went in without its Result line and
  `Commit:` hash; both were added in B's first commit (`b751f543`).

None of these changed a count that another letter's scope was derived from.

## Summary

The risk is the board-to-footprint inverse on rotated and Bottom parts; it is
tested in all 8 cases before anything uses it. A leaves untouched the gesture,
sidebar and help (B), the schematic's reference label, and every file with no
offset.
