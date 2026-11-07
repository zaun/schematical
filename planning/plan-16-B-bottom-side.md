# plan-16-B: Bottom side

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-16-A

Parts and board texts can sit on the bottom of the board. A selected part's
Position section gets a **Layer** picker (Top | Bottom). On Bottom, the
footprint is drawn mirrored left to right: pads, body, pin-1 mark, and the
reference text too. Board texts get the same picker and are drawn mirrored
when on Bottom. The side is saved in the file.

References:

- `schematical/schematical/Canvas/BoardModel.swift:BoardPlacement`, `PlacedChip` board extension, `BoardLayout`
- `schematical/schematical/Backend/PartCatalog.swift:Footprint.Corners.rotated(by:)`
- `schematical/schematical/Canvas/PCBCanvasView.swift:draw(in:size:)`
- `schematical/schematical/Canvas/PCBSidebarView.swift:FootprintPanel`, `TextPanel`
- `schematical/schematical/Canvas/SchematicModel.swift:rotateFootprint` (the keep-the-origin-fixed precedent)
- `docs/file-format.md` § Parts (`pcb`), § Board (texts)

## Prerequisites

See plan-16-A § Prerequisites (plan-15 complete). plan-16-A must be
complete: `rg -n 'enum CopperLayer' schematical/schematical/Canvas/BoardSettings.swift` → 1 match.

## 1. Goal

- With a part on Bottom, every pad, the body outline, the pin-1 dot and the
  reference are the mirror image of the Top drawing. The mirror line is
  vertical, through the body centre, applied before rotation. Switching sides
  leaves the footprint's origin point where it was. A text on Bottom draws
  mirrored inside its own box. Both sides survive Save and Open.

### Non-goals

- No layer visibility (that is C). Bottom items stay drawn, under Top items.
- Rotation does not change meaning. It is still clockwise as seen from above,
  on both sides.
- No "view from below" mode. The canvas always looks from the top.
- No new mark layers. A bottom part's silkscreen and document marks use the
  existing silkscreen and document inks.

## 2. Current State

- `BoardPlacement.board(_ local:)` rotates a footprint-local point, then adds
  `centre`. `board(centre:size:)` turns a pad rect the same way. The type's
  comment says "Top side only" (`BoardModel.swift:30`).
- Seven `.board(` call sites use the placement transform:
  `rg -n '\.board\(' schematical/schematical --glob '*.swift' | wc -l` → 7.
  Footprint rects go through `CGRect.applying(_ placement)` (3 call sites:
  `rg -n 'applying\(' schematical/schematical/Canvas --glob '*.swift' | wc -l` → 3).
  All of these go through `board(_:)` and `board(centre:size:)`, so a mirror
  in those two functions reaches every one.
- Chamfered pad corners are turned with `Corners.rotated(by:)`
  (`PartCatalog.swift:135`, called from `BoardModel.swift:109`). Mirroring
  must swap left and right corners before the turn.
- The reference text is drawn unmirrored at the body centre
  (`PCBCanvasView.swift`, "Reference: 1 mm tall").
- Board texts draw in a `drawLayer`: translated to the anchor, rotated, then
  scaled by `drawScale` (`PCBCanvasView.swift`, "Texts").
- `rotateFootprint` keeps the origin point fixed: it computes the origin
  before and after the change, shifts the centre by the difference, then
  snaps (`SchematicModel.swift:930-941`). `setFootprintSide` copies this.

### Measured populations

| What | Count | Command |
|---|---|---|
| Placement-transform call sites | 7 | `rg -n '\.board\(' schematical/schematical --glob '*.swift' \| wc -l` → 7 |
| Corner-rotation call sites | 2 (definition + 1 use) | `rg -n 'rotated\(by' schematical/schematical --glob '*.swift' \| wc -l` → 2 |
| Parts with a `pcb` placement in the repo sample | 9 | `rg -c '"pcb"' be-6502.schematical` → 9 |

### Verified properties

- Mirroring before rotation keeps rotation clockwise from above. A local
  point (x, y) on Bottom maps to `rotate((−x, y), by: rotation) + centre`.
  `board(centre:size:)` turns sizes only by rotation. A mirror leaves a size
  unchanged, so pad rects stay right.
- `Corners` mirrored left to right swaps topLeft↔topRight and
  bottomLeft↔bottomRight. On the raw bits (TL=1, TR=2, BR=4, BL=8) that is
  1↔2 and 4↔8.

## 3. Design

### Side

`nonisolated enum BoardSide: String, CaseIterable { case top, bottom }`, with
titles "Top" and "Bottom":

- `BoardPlacement.side` and `BoardText.side`, both defaulting to `.top`;
- `BoardSide.copperLayer` maps to `.top` or `.bottom` (C and E use it).

### Mirror in the placement transform

`BoardPlacement.board(_ local:)` becomes
`rotate(side == .bottom ? CGPoint(x: -local.x, y: local.y) : local, by: rotation) + centre`.
`board(centre:size:)` mirrors the centre the same way and leaves the size
alone. Pads get `pad.corners.mirrored(side == .bottom).rotated(by:)`, adding
`Corners.mirrored(_:)` next to `rotated(by:)`.

Every derived geometry (`boardPads`, `boardBody`, `boardBounds`,
`boardOrigin`, `pin1Mark`, `BoardLayout.originPoint/snapped/nextSpot`) then
mirrors with no further change, because each one goes through these two
functions.

### Mirrored text

- **Reference.** On Bottom it is drawn in a `drawLayer` translated to the
  body centre and scaled by (−1, 1), so the letters read backwards, as on a
  real bottom silkscreen.
- **Board text.** On Bottom it is mirrored inside its own box: after the
  existing translate and rotate, translate by `localSize.width` and scale by
  (−1, 1), then scale by `drawScale`. `boardRect`, the anchor and hit testing
  stay the same, so a text keeps its place when it changes side.

### Draw order

Bottom items are drawn first, then Top items:

1. bottom-side footprints (outline, pads, pin-1, reference), then
   bottom-side texts;
2. top-side footprints, then top-side texts;
3. selection.

Hit testing (`text(at:)`, `footprint(at:)`) prefers Top: it searches Top
items in reverse drawing order, then Bottom items.

### Document and sidebar

- **`setFootprintSide(_:_:)`.** Ignored on a 1-layer board, or when the side
  is unchanged. Otherwise it keeps the origin point fixed, as
  `rotateFootprint` does: before/after, shift, snap.
- **`setTextSide(_:_:)`.** Changes only `side`.
- **`FootprintPanel` Position section.** Gets `Picker("Layer")` (segmented
  Top | Bottom) above Origin, disabled on a 1-layer board.
- **`TextPanel`.** Gets the same picker, above its Layer (Document |
  Silkscreen) picker. The text picker is labelled "Side" so the two don't
  share a name. The footprint's picker keeps the owner's word "Layer".
- **`canSetLayerCount(1)`.** Refused with "Move every part and text to Top
  first." while any part or text is on Bottom.

### Rejected alternatives

- **Mirror after rotation, as a horizontal flip on the board.** A 90° bottom
  part would then turn the other way, so Rotate would turn bottom parts
  anticlockwise. Mirroring the footprint before rotating keeps Rotate
  consistent.
- **Mirror board texts about their anchor.** The text would jump to the
  other side of its anchor when it changes side, and its box would move.

## Compatibility / Format Impact

- `pcb.side`: `"top"` (default) or `"bottom"`. It is written for every part
  with a footprint, like the four mark strings.
- Board text `side`: `"top"` (default) or `"bottom"`, always written.
- Older files open with everything on Top. Older builds ignore the key and
  show bottom parts unmirrored on top.
- New errors:
  - `U2 side "left" must be top or bottom` (from the existing
    `Self.value` helper);
  - `text <id> side "left" must be top or bottom`;
  - `U2 is on the bottom of a 1-layer board`;
  - `text <id> is on the bottom of a 1-layer board`.

## Phases

### Phase 1 — Mirrored geometry and file format

- [x] `Canvas/BoardSettings.swift`: `BoardSide`; `BoardText.side`.
- [x] `Canvas/BoardModel.swift`: `BoardPlacement.side`; mirror in
      `board(_:)` and `board(centre:size:)`; mirrored corners in
      `boardPads`. Replace the "Top side only" comment.
- [x] `Backend/PartCatalog.swift`: `Corners.mirrored(_ on: Bool)`.
- [x] `Canvas/SchematicModel.swift`: `setFootprintSide`, `setTextSide`, and
      the 1-layer refusal in `canSetLayerCount`.
- [x] `Document/SchematicFile.swift`: `Placement.side` and `Board.Text.side`,
      encoded and decoded, with the errors above.
- [x] Tests (`<tests>/BoardSideTests.swift`):
  - a SOIC-14 on Bottom at rotation 0 has pad 1 at the mirrored x of its Top
    position;
  - a chamfered pad's corners swap;
  - at rotation 90 a Bottom part's pad 1 equals `rotate(mirror(p), 90)`;
  - `setFootprintSide` keeps `boardOrigin` for every `FootprintOrigin`;
  - `boardRect` of a text doesn't change with side;
  - side survives a round trip;
  - each new error;
  - `canSetLayerCount(1)` is refused while a part is on Bottom.

  `/tmp/p16t.sh BoardSideTests,BoardLayersTests` (the named xcodebuild
  check, both suites) → "Executed 18 tests, with 0 failures". The chamfer
  test uses SN74AUP1G00DPWR (X2SON-5-DPW), whose signal pads are cut on one
  corner.
- [x] (added, from Validation Plan doc sync) `docs/file-format.md`: `side`
  in the `pcb` row and the text table, the four errors, a bottom part in
  the example.

Acceptance: mirrored pads, corners and origins match the formulas, and side
survives Save and Open.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:<tests>/BoardSideTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — Drawing and sidebar

- [ ] `Canvas/PCBCanvasView.swift`: the bottom-then-top draw order;
      mirrored reference and board text; Top-first hit testing. Update the
      type's doc comment.
- [ ] `Canvas/PCBSidebarView.swift`: the Layer picker in `FootprintPanel`
      Position and the Side picker in `TextPanel`, both disabled on a 1-layer
      board. The 1-layer choice in the Layers picker is disabled, with its
      reason, while anything is on Bottom.

Acceptance: an SOIC on Bottom shows mirrored pads and a backwards reference.
Its X/Y in the sidebar don't change when it switches side. Rotate still turns
it clockwise on screen.
  Check: run the app (`run` skill), open `be-6502.schematical`, select U1 on
  PCB, set Layer to Bottom: the drawing is mirrored, X/Y unchanged, and ⌥⌘R
  turns it clockwise (est. 3 min).
Commit: —

## Validation Plan

- Tests: `BoardSideTests` (transform, corners, origin preservation, text box,
  file, refusal).
- Runtime proof: Phase 2's check, plus a board text set to Bottom that reads
  mirrored in place.
- Doc sync: `docs/file-format.md`. Add `side` to the `pcb` row and the text
  table, add the new errors, and put a bottom part in the example.
- Final gate: run once after E (see plan-16-E).

## Decisions

- The footprint is mirrored before rotation, so rotation stays clockwise as
  seen from above on both sides — author's call, delegated by the owner on
  2026-11-07.
- Board texts also get Top/Bottom, which closes the bottom-silkscreen gap
  raised in discussion — author's call, delegated by the owner. Mirrored
  inside its own box, so it doesn't move.
- The canvas always looks from the top. Bottom items are drawn under Top
  items, and a click picks Top first.
- On a 1-layer board everything is on Top. The side pickers are disabled.

## Corrections

- 2026-11-07: `rg -n '\.board\(' schematical/schematical --glob '*.swift' | wc -l`
  → 8, not 7. The 8th is `SelectionRows.board(document)`
  (`PCBSidebarView.swift:54`, added by plan-15), not the placement
  transform; the 7 transform call sites are as the plan says.
- 2026-11-07: `setTextSide` on a 1-layer board still allows going back to
  Top (only Bottom is refused), so a text can never get stuck.

## Summary

The risk sits in the two placement-transform functions, because every
derived geometry goes through them. The Phase 1 tests pin the mirror against
explicit formulas before any drawing changes. Rotation semantics and the
file version stay the same.
