# plan-67-B: Unit view — the gate symbol

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-67-A

Draws one unit of a part as a standard logic-gate symbol: the distinctive AND,
OR, XOR and buffer shapes, an inversion bubble on an inverted output, a wide
input edge for gates with many inputs, a second output where a unit has one, a
Schmitt mark and an open-drain mark. The symbol flips and turns to the four
positions like every other part drawn as its own symbol.

This letter adds the symbol, its pin layout and a way to build **a piece**: a
`PlacedChip` value that stands for one unit of a part, for drawing and
geometry. No document places a piece yet; the tests build them directly.

References:

- plan-67-A (the feature, the prerequisites, `PartUnit` / `GateShape`).
- `schematical/schematical/Canvas/SchematicModel.swift`: `PartSymbol` (151-243),
  `ChipLayout` (251-992), `PlacedChip` (1026-1302).
- `schematical/schematical/Canvas/TurnedSymbol.swift` (the turn helpers),
  `Canvas/TransistorSymbolView.swift` (the model for a turned symbol view),
  `Canvas/ChipBlockView.swift`, `Canvas/ReferenceLabel.swift`.
- `schematical/schematicalTests/SchematicRotateTests.swift`.
- `AGENTS.md` § Git.

Test command form `XB`: see plan-67-A.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites; re-run them. This
letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-A has landed | `grep -c 'let units: \[PartUnit\]' schematical/schematical/Backend/PartCatalog.swift` → 1 | NOT MET (2026-11-25: 0) |

## 1. Goal

- `PlacedChip.piece(_:origin:isFlipped:rotation:)` returns a chip value
  whose layout holds only that unit's pins, whose symbol is `.gate` for a gate
  unit and `.block` for a block unit, and whose `drawnReference` is
  `"<reference>-<key>"`.
- A gate piece's pin tips are on the 10-unit grid at every flip and turn, and
  `GateSymbolView.paint` ends every lead exactly on its pin's tip.
- The gate symbol is drawn for each of the unit shapes the catalog holds
  (§2 table), with the reference above it at every turn.

### Non-goals (explicit constraints)

- No part on the canvas changes: `symbol(for:)` never returns `.gate`, so a
  placed gate IC is still a block.
- No IEC rectangular symbols (`&`, `≥1`); the shapes are the distinctive ones.
- A gate piece shows pin numbers, not pin names, and no part number; the part
  number is on the `P` block.
- `ChipBlockView` keeps drawing every existing block exactly as it does.

## 2. Current State

- A part's symbol is a `PartSymbol` case (`SchematicModel.swift:151`) chosen by
  `symbol(for:)` (`Backend/PartCatalog.swift:1875`); `canRotate` (234-242)
  lists the symbols that turn.
- `ChipLayout` holds only `size` and `slots` (301-302). `init(part:symbol:)`
  (322) switches on the symbol to a private builder; `.block` uses
  `init(pins:)` (902), which already lays out any list of pins. `flipped()`
  (703) and `rotated(by:)` (729) work on any layout.
- `PlacedChip` stores `let symbol` and `private let baseLayout` (1033, 1077);
  `layout` (1244) is `baseLayout`, flipped, then turned. `tip(of:)` (1266),
  `viewOrigin` (1283), `bodyRect` (1290), `hitRect` (1297) derive from `origin`
  and `layout`.
- A symbol that turns sizes its frame with `chip.symbolFrameSize`, draws in
  unturned coordinates sized `chip.unturnedSize` and maps every point through
  `chip.symbolPoint(x, y)` (`TurnedSymbol.swift:48`); `TransistorSymbolView`
  (155 lines) is the nearest precedent. All drawing goes through a
  `SchematicPainter` (`SchematicPainter.swift:95`: `fill`, `stroke`, `draw`
  text), so one `paint` function serves the canvas and every export.
- The reference text is placed by `ReferenceLabel.anchor(for:)`
  (`ReferenceLabel.swift:16`), with a `switch chip.symbol` per case, and
  `turnedAnchor` (87) for a turned part. It prints `chip.reference` (131).
- `ChipBlockView.paint` (30) draws the box, the part number in the header and
  the function words in the body, then each pin.

### Measured populations

| What | Count | Command |
|---|---|---|
| Exhaustive `switch` sites over a chip's symbol that need a `.gate` case | 8 in 6 files: `ReferenceLabel.swift:20, 59, 99`; `SchematicModel.swift:324`; `SchematicCanvasView.swift:1217`; `SchematicExportScene.swift:51`; `KiCadSchematicWriter.swift:374`; `EasyEDAProSchematicWriter.swift:113` | `grep -rnE 'switch chip\.symbol\|switch symbol' schematical/schematical --include='*.swift'` |
| Unit shapes in the catalog (units, inputs) the symbol must draw | buffer 1 in (80 buf + 142 not gates); and/or/xor with 2, 3, 4, 8, 13 inputs; two outputs on 3 devices | plan-67-A § Measured populations |

### Verified properties

- **`rotated(by:)` keeps a 60 × 20n body's tips on the grid.** Read
  `ChipLayout.turnBox` (721) and `rotated` (729-764): the turn happens inside
  the size rounded up to 10; every size and pin offset in §4 is a multiple
  of 10, so the box is the size itself and each tip stays a multiple of 10 from
  the origin.
- **`symbolPoint` is affine** (`TurnedSymbol.swift:48`: a mirror, a quarter-turn
  and an inset). Bézier control points mapped through it give the mapped curve,
  so the outlines are built from line and Bézier segments only, never `addArc`.
- **`testEverySymbolFollowsItsPins` is unaffected.** Read
  `SchematicRotateTests.swift:115-126`: it walks catalog parts through
  `symbol(for:)`, which never returns `.gate`.

## 3. Design Overview

Three pieces:

1. **The layout** — `ChipLayout(gate:pins:)`: where a gate unit's pins are.
2. **The piece** — `PlacedChip.piece(…)` and `drawnReference`.
3. **The drawing** — `GateSymbolView`, plus the reference label's position.

Correctness risk concentrates in the lead ends: a lead that stops short of its
pin's tip is a wire that looks unconnected. The test pins every lead end to
`tip(of:)` at all eight orientations for every shape.

Byte-identity is not this letter's gate (new drawing, no existing output moves).

Rejected:

- *A new `ChipBody` type with the geometry moved off `PlacedChip`.* 20 symbol
  views, `TurnedSymbol`, `ReferenceLabel` and both exporters take a
  `PlacedChip`; a piece that *is* a `PlacedChip` reuses all of them unchanged.
- *Scale one body to any input count.* A 13-input AND body 260 tall is not a
  symbol anyone knows; the standard wide-gate form (a normal body with an
  extended input edge) is used instead.

## 4. The layout

All in the body's own coordinates, top-left (0, 0), before flip and turn.
`n` = input count, `m` = output count (1 or 2).

- `ChipLayout.gateWidth = 60`. Height `H = max(40, 20 · n)`.
- Inputs on the left side, `centerY = H/2 − 10·(n − 1) + 20·i` for input `i`
  (n = 1 → 20; n = 2 → 10, 30; n = 3 → 10, 30, 50).
- Outputs on the right side: one at `H/2`; two at `H/2 − 10` (the true output)
  and `H/2 + 10` (the inverted one).
- `size = (60, H)`. Slots are in input order then output order.

`ChipLayout(gate shape: GateShape, pins: [ChipPin])` builds it; a pin index in
the shape that is not in `pins` is skipped.

## 5. The piece

`PlacedChip` changes:

- `let symbol` → `private(set) var symbol`; `private let baseLayout` →
  `private var baseLayout`.
- `var unitKey: String?` (nil for a whole part; set on a piece).
- `var drawnReference: String` → `unitKey.map { "\(reference)-\($0)" } ?? reference`.
- `func piece(_ unit: PartUnit, origin: CGPoint, isFlipped: Bool, rotation: Int) -> PlacedChip`:
  a copy with `unitKey = unit.key`; `symbol` `.gate` for `.gate`, `.block` for
  `.block`; `baseLayout` `ChipLayout(gate:pins:)` or `ChipLayout(pins:)` over
  the unit's pins; the given origin and flip; `rotation` normalized and forced
  to 0 unless the new symbol turns.

`PartSymbol` gets `case gate`, listed in `canRotate`. `canFlip` already covers it.

A piece is for drawing and geometry only. It shares the part's `id`; nothing
stores one in `SchematicDocument.chips`.

## 6. The drawing

`Canvas/GateSymbolView.swift`: `struct GateSymbolView: View` with `chip`,
`isSelected`, `isHovered`, `isSibling` and
`static func paint<P: SchematicPainter>(_:chip:isSelected:isHovered:isSibling:)`,
in the shape of `TransistorSymbolView`. Every point goes through
`chip.symbolPoint`. The shape comes from `chip.part.units.first { $0.key == chip.unitKey }`.

Ink: selected or hovered → `.accent`, line width 2; sibling → `.accent`
at 0.5 opacity, width 1.5; otherwise `.primary`, 1.5. The body is filled
`.paper` first; hovered adds the 0.12 accent fill, as `ChipBlockView` does.

With `W = 60`, `mid = H/2`, body height `h = min(H, 60)`, `top = mid − h/2`,
`bottom = mid + h/2`, `k = 0.5523`:

- **AND**: from (0, top) right to (W − h/2, top); a half circle of radius h/2
  to (W − h/2, bottom) as two cubic quarter arcs; back to (0, bottom); close.
- **OR**: back edge a quadratic from (0, top) to (0, bottom) with control
  (15, mid); top edge a quadratic from (0, top) to (W, mid) with control
  (40, top); the bottom edge its mirror.
- **XOR**: the OR body drawn with its back edge moved 6 right (control
  (21, mid), ends at (6, top) and (6, bottom)); plus an open curve from
  (0, top) to (0, bottom) with control (15, mid).
- **Buffer**: the triangle (0, top), (W, mid), (0, bottom), with `h = 40`.
- **Wide inputs** (`H > h`): a vertical line at x = 0 from (0, 0) to (0, H)
  for AND and buffer; for OR and XOR two lines, (0, 0)–(0, top) and
  (0, bottom)–(0, H).
- **Input leads**: from (−10, y) to (`backX(y)`, y), where `backX` is 0 on a
  straight back or outside `top…bottom`, and on the OR / XOR back curve
  `30·t·(1 − t)` with `t = (y − top)/h` (the outer curve for XOR).
- **Output leads**: from (`frontX(y)`, y) to (W + 10, y). `frontX(mid) = W`.
  For a second output, `frontX` is where the body's front edge is at that y:
  AND `W − h/2 + sqrt((h/2)² − (y − mid)²)`; buffer `W·(1 − |y − mid|/(h/2))`;
  OR / XOR the top-edge quadratic solved for y.
- **Bubble** on an inverted output: a circle of diameter 8 centred
  (W + 4, y), filled `.paper`, stroked; its lead runs (W + 8, y)–(W + 10, y).
  For a second (inverted) output the bubble is centred (`frontX(y)` + 4, y) and
  the lead starts 8 right of `frontX(y)`.
- **Schmitt mark**: the hysteresis glyph, two strokes of line width 1:
  (14, mid + 4)–(20, mid + 4)–(24, mid − 4)–(26, mid − 4) and
  (14, mid + 4)–(16, mid + 4)–(20, mid − 4)–(26, mid − 4).
- **Open-drain mark**: a diamond of half-width 3 centred (W − 16, mid) for an
  AND / OR / XOR body, (30, mid) for a buffer, with a bar under it
  (x ± 3, mid + 5); line width 1.
- **Pin numbers**: each pin's designator, size 9, `.secondary`, over its lead:
  placed with `chip.symbolText(x, y − 5, toward:)` at x = −4 for an input and
  W + 4 for an output, so it reads beside the lead at every turn.

`ReferenceLabel`: `.gate` at rest anchors at `CGPoint(x: x, y: mid − h/2 − 3)`
in the frame (just above the body, which may be shorter than the frame);
turned, it sits above the turned body box: `turnedAnchor` gets a `.gate` case
returning the top centre of `chip.symbolRect(bodyBox)` minus 3 in y, with
`bodyBox = CGRect(x: 0, y: top, width: 60, height: h)`. The label text is
`chip.drawnReference`.

A **block piece** is drawn by `ChipBlockView.paint` as is, with two changes
keyed on `chip.unitKey != nil`: the function words are not drawn (the body is
too short for them and they belong to the gates), and an `isSibling` parameter
(default false) picks the sibling ink.

## Compatibility / Format Impact

None. No stored or exported form changes; no existing part is drawn differently.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks rather than deleting
> them; fill each `Commit:` line; add tasks you discover. **An unticked box
> means NOT DONE.**

### Phase 1 — the gate layout and the piece

Geometry only; nothing draws it yet.

- [ ] `Canvas/SchematicModel.swift` `PartSymbol`: add `case gate` with a doc
      comment; add it to `canRotate`.
- [ ] `ChipLayout`: add `static let gateWidth: CGFloat = 60` and
      `init(gate:pins:)` per §4. In `init(part:symbol:)` (the `switch` at 324),
      `.gate` falls to `init(pins:)` (a whole part is never a gate; the case
      exists for exhaustiveness).
- [ ] `PlacedChip`: the three member changes, `drawnReference` and `piece(…)`
      of §5.
- [ ] `ReferenceLabel.swift`: add `.gate` to the three switches (20, 59, 99)
      per §6; `paint` (128) and `ReferenceLabelView` print `drawnReference`.
- [ ] `SchematicCanvasView.swift:1217` and `SchematicExportScene.swift:51`:
      a `.gate` case calling `GateSymbolView` / `GateSymbolView.paint`
      (the view is Phase 2; in this phase the case calls `ChipBlockView` so the
      build stays green, replaced in Phase 2).
- [ ] `KiCadSchematicWriter.swift:374` and `EasyEDAProSchematicWriter.swift:113`
      (`bodyBox(of:)`): `.gate` returns the layout's full size box, the same
      branch `.block` takes.
- [ ] Tests: new `schematicalTests/GateSymbolTests.swift`:
      `testLayout` (pieces of `SN74HC00N` unit A, `SN74HC04N` unit A, `CD4068BE` unit A, `SN74ALS133N` unit A: sizes 60×40, 60×40,
      60×160, 60×260; the pin offsets of §4; sides left for inputs, right for
      outputs); `testPieceIdentity` (`unitKey`, `drawnReference == "U1-A"`,
      symbol `.gate`; unit `P` → `.block`, layout pins named VCC and GND, its
      rotation forced to 0 when 90 is asked); `testTipsOnGridAtEveryTurn` (for
      those four pieces, every flip × 0/90/180/270: each `tip(of:)` has x and y
      that are multiples of 10 when the origin is).

Acceptance: a piece's geometry is right at every orientation.
  Check: `XB -only-testing:schematicalTests/GateSymbolTests` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: —

### Phase 2 — the drawing

- [ ] New `Canvas/GateSymbolView.swift` per §6 (the view, `paint`, private
      `backX`, `frontX`, and one outline function per body returning a `Path`).
- [ ] `SchematicCanvasView.swift:1217` and `SchematicExportScene.swift:51`:
      the `.gate` case now uses `GateSymbolView`.
- [ ] `Canvas/ChipBlockView.swift`: `isSibling` parameter (view and `paint`,
      default false) and no function words when `chip.unitKey != nil`.
- [ ] Tests in `GateSymbolTests`: `testLeadsEndOnTheirPins` (a recording
      `SchematicPainter` that keeps every stroked path's end points; for each
      of the four pieces at every flip and turn, each `tip(of: slot) −
      viewOrigin` is an end point of some stroked path);
      `testInkStaysInTheFrame` (`BoundsPainter` over `paint`: the ink box lies
      inside `symbolFrameSize` grown by 12 on every side, for all four pieces
      and all eight orientations); `testEveryShapeDraws` (for every distinct
      `GateShape` in the catalog by (body, input count, output count,
      inverted flags, schmitt, openDrain): `paint` into `BoundsPainter` gives a
      non-empty box); `testReferenceAboveTheBody` (`ReferenceLabel.anchor` of a
      2-input piece at 0° is above y = 0 and inside the frame's width at all
      four turns).
- [ ] Look at it once: a temporary test writes
      `SchematicExport.png` of a scratch scene holding the pieces of
      `SN74HC00N`, `SN74HC86N`, `SN74HC32N`, `SN74HC14N`,
      `CD4068BE`, `SN74ALS133N` and `SN74HC266N`, each at 0° and 90°, to its
      container; open the picture, compare each shape with §6, fix what is
      off, then delete the temporary test. Record in this task line what was
      looked at and what was corrected.

Acceptance: every catalog gate shape draws, every lead reaches its pin.
  Check: `XB -only-testing:schematicalTests/GateSymbolTests -only-testing:schematicalTests/SchematicRotateTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `GateSymbolTests` (seven cases); `SchematicRotateTests` re-run because
  `canRotate` and `ReferenceLabel` changed.
- Coverage check: `testEveryShapeDraws` enumerates the shapes from the catalog
  itself, so a shape added to `gate.json` later is drawn by the test too.
- Runtime proof: the picture of Phase 2's last task.
- Doc sync: none in this letter (the user-visible docs land with letter G).
- Final gate: in plan-67-G, once.

## Decisions

- **Distinctive (ANSI) gate shapes** — the author's reading of the owner's
  "a known logic symbol that you would see in other logic simulators or
  schematic editors", 2026-11-25.
- **Each unit flips and turns on its own; this works for non-block units** —
  decided by the owner, 2026-11-25. Settles `.gate` in `canRotate` and the
  `P` block being flip-only, as every block is.
- **Unit references read `U1-A`** — see plan-67-A § Decisions.

## Corrections

## Summary

The risk is geometric: leads that miss pins and marks that fall outside the
frame. Both are pinned by tests over every shape the catalog holds and all
eight orientations. Existing symbols and blocks are not redrawn.
