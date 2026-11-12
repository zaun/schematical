# plan-32-E: Part editor block preview (schematic view)

Last updated: 2026-11-11
Effort: medium (1h–2h)
Depends on: plan-32-D

The editor shows the part as it looks on the schematic: the same block the schematic
page draws for a placed part (header with the part number, pins and names on the
sides, groups, the function text, the symbol glyph for oscillators, barrel jacks and
the other symbol kinds). The behavioral outcome: the preview of an editor part is the
block the schematic page draws for that part once it is placed, with the same size
and pin positions.

References:

- `schematical/schematical/Canvas/ChipBlockView.swift` — `ChipBlockView` and its static
  `paint(_:chip:isSelected:isHovered:connectedPins:)`, which draws into any `SchematicPainter`.
- `schematical/schematical/Canvas/SchematicModel.swift` — `PartSymbol` (line 114),
  `ChipLayout` (line 155, `ChipLayout.labelMargin` at 189), `PlacedChip` and its
  initializer (line 578 onward).
- `schematical/schematical/Canvas/SchematicPainter.swift` — `ScreenPainter` (line 146) and
  the `SchematicPainter` protocol; `Canvas/SVGPainter.swift:7` is the SVG (line-art) painter.
- `schematical/schematical/Backend/PartCatalog.swift` — `symbol(for: CatalogPart) -> PartSymbol`
  (line 815), which picks the symbol kind for a part.
- plan-32-C § 4.7 — the validator that yields the `CatalogPart` the preview draws.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-D complete: the editor's footprint section and its preview exist | `xcodebuild test … -only-testing:schematicalTests/FootprintPreviewTests` → `passed` | MET (run 2026-11-11 in the P-32 worktree: `FootprintPreviewTests` passed, 3 cases) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- A **Block** pane in the editor draws `ChipBlockView.paint` for the editor's current
  validated `CatalogPart`, in a `Canvas` at the same scale as the schematic page's
  default zoom.
- The pane's drawn pin positions and the block's size equal the schematic page's for
  the same part placed at the origin.
- The block updates on every validated change (plan-32-C § 4.7), with the debounce.

### Non-goals (explicit constraints)

- **The schematic page's drawing is unchanged.** `ChipBlockView.paint` is called as it
  is; no parameter is added for the editor.
- **No selection or hover in the preview.** `isSelected` and `isHovered` are false; the
  preview is not interactive in v1.
- **No wiring in the preview.** `connectedPins` is empty, so every pin's designator is
  drawn (the unwired case).
- **No new symbol kinds.** The symbol comes from `symbol(for:)` (`PartCatalog.swift:815`).

## 2. Current State

- `ChipBlockView.paint` draws the header (part number, `PaintedText`), the body box, the
  header rule, the function text, the pins and stubs, the designators of unwired pins,
  and the symbol glyphs by `chip.symbol` (for example `drawSquareWaveGlyph` for an
  oscillator and the barrel-jack symbol). It is `static`, generic over `SchematicPainter`.
- `ChipLayout(part:symbol:)` is the layout the schematic page uses; its `size` is the body
  size and `labelMargin` (30) is added on each side by the view's frame (`ChipBlockView.body`).
- `PlacedChip`'s initializer takes `id`, `reference`, `part`, `symbol`, `origin` and the
  options (`SchematicModel.swift:613`); `symbol` is the `PartSymbol` of `symbol(for:)`.
- Counts (measured 2026-11-11): `rg -n 'static func paint' schematical/schematical/Canvas/ChipBlockView.swift | wc -l` → 1;
  `rg -c 'enum PartSymbol' schematical/schematical/Canvas/SchematicModel.swift` → 1.

### Verified properties

- **`ChipBlockView.paint` needs only a `PlacedChip` and a painter.** Read `ChipBlockView.swift`
  (`static func paint`, the first 60 lines): the signature takes `chip`, the two flags and
  `connectedPins`; the body reads `chip.partNumber`, `chip.symbol` and `chip.layout`.
- **`symbol(for:)` is a pure function of the `CatalogPart`.** Read `PartCatalog.swift:815–835`:
  it switches on `part.kind`, reads `part.details` and `part.pins`, and calls `part.isTactileSwitch`;
  there is no library lookup.

## 3. Design Overview

1. **`PartPreview.chip(for: CatalogPart) -> PlacedChip`**: a `PlacedChip` at origin
   `(0, 0)` with a fixed id and reference (`U1`), `symbol: symbol(for: part)`, and the
   kind options (LED color, header gender) at the part's defaults.
2. **`BlockPreviewView`**: a SwiftUI `Canvas` that calls `ChipBlockView.paint` with a
   `ScreenPainter`, at the page's default zoom (the schematic page's `CanvasViewport`
   default, `ContentView.swift:32`, scale `0.75`), centred in the pane.
3. **The pane** in `PartEditorWindow`: a segment (Block | Footprint), so the block and the
   footprint preview share the space. The footprint preview is plan-32-D's view.

Where correctness risk concentrates: the preview's `PlacedChip` must not diverge from a
placed chip's layout. The Phase 2 test compares the two layouts directly, so a divergence
fails a test rather than a visual check. Schedule: the factory first (Phase 1), then the
view (Phase 2), then the segment (Phase 3).

### Rejected alternatives

- **A separate block drawing for the editor.** Rejected: two drawings of one part would
  drift. The schematic's `ChipBlockView.paint` is the single drawing.
- **Rendering the preview through the schematic canvas view.** Rejected: that view is
  built for a document, with pan, zoom, selection and wiring. The preview needs none of
  those, and a plain `Canvas` is smaller.

## 4. Detailed Design

### 4.1 The factory

```swift
enum PartPreview {
    static func chip(for part: CatalogPart) -> PlacedChip {
        PlacedChip(id: UUID(uuidString: "00000000-0000-0000-0000-000000000001")!,
                   reference: "U1", part: part, symbol: symbol(for: part),
                   origin: .zero)
    }
}
```

The fixed id is used only by the preview; it never reaches a document.

### 4.2 The view

- `BlockPreviewView(chip:)`: `Canvas { context, _ in var painter = ScreenPainter(context); ChipBlockView.paint(&painter, chip: chip, isSelected: false, isHovered: false) }`
  with the frame from `chip.layout.size` plus `ChipLayout.labelMargin` on each side,
  as `ChipBlockView.body` does.
- Scale: the view is drawn at `CanvasViewport(baseScale: 0.75)` (`ContentView.swift:32`);
  the scale is applied with `context.scaleBy`, not by resizing the frame.

## Phases

### Phase 1 — The preview chip factory

One line: a `PlacedChip` for an editor part, built without a document.

- [x] `schematical/schematical/Editor/PartPreview.swift`: `PartPreview.chip(for:)` (§ 4.1).
- [x] Tests: `PartPreviewTests.swift`: for a bundled gate part (`SN74HC00N`), `PartPreview.chip(for:).layout.size` equals the layout size of a `PlacedChip` the schematic page builds for the same part (built through the same `PlacedChip.init` with the same `symbol(for:)`, so the test checks the factory adds nothing else).

Acceptance: the layout size of the factory's chip equals the placed chip's layout size for `SN74HC00N`, and for every gate part in `Parts/gate.json`'s first device.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartPreviewTests` → `passed` (est. 3 min).
  Result (2026-11-11): passed; the comparison is with a chip the schematic page places in a document (`placeChip`, as a drop does), for SN74HC00N and every part of gate.json's first device (`74_00`): layout size, symbol and pin slot rows equal.
Commit: d0aa1ca

### Phase 2 — The block view

One line: the editor draws the block of its validated part, at the page's scale.

- [x] `schematical/schematical/Editor/BlockPreviewView.swift`: `BlockPreviewView(chip:)` (§ 4.2).
- [x] `PartEditorWindow.swift`: the view takes the validator's `CatalogPart` (plan-32-C § 4.7) through `PartPreview.chip(for:)`; it refreshes on each validated change.
- [x] Tests: `BlockPreviewViewTests.swift`: the view's `pinPositions` (the pin-stub endpoints computed by a test-only `ScreenPainter` capture of `ChipBlockView.paint`'s line segments) for `SN74HC00N` equal those for a placed chip in the schematic (the same painter run on `PlacedChip`).

Acceptance: the pin endpoints for `SN74HC00N` from the preview equal the schematic chip's, within 1e-9 pt.
  Check: `xcodebuild test … -only-testing:schematicalTests/BlockPreviewViewTests` → `passed` (est. 4 min).
  Result (2026-11-11): passed; a test painter (`CapturePainter`, in `PartPreviewTests.swift`) records every stroked path's points and every text's position for both chips: same count, every point within 1e-9 pt.
Commit: d0aa1ca

### Phase 3 — Segment with the footprint preview; run check

One line: the pane shows the block and the footprint, one at a time.

- [x] `PartEditorWindow.swift`: a segmented control (Block | Footprint) above the preview pane; Block shows `BlockPreviewView`, Footprint shows plan-32-D's `FootprintPreviewView`.
- [x] Tests: `PartEditorWindowTests.swift` gains (d): the pane's selected segment defaults to Block.

Acceptance: the segment defaults to Block and switches to Footprint; in a launched app, the block of a gate part shows pins with their names on the sides.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorWindowTests` → `passed` (est. 4 min); the visual check is the run skill's (est. 10 min).
  Result (2026-11-11): passed (d). Real app: Copy to Editor… on SN74HC00N opens with Block selected, the block centred (header SN74HC00N, VCC/1A…4B on the left, 1Y…4Y/GND on the right, designators outside, NAND in the body); Footprint shows PDIP-14.
Commit: d0aa1ca

## Validation Plan

- Tests: `PartPreviewTests` (Phase 1), `BlockPreviewViewTests` (Phase 2), `PartEditorWindowTests` case (d) (Phase 3).
- Coverage check: `rg -n 'PartPreview|BlockPreviewView' schematical/schematical --glob '*.swift' -l` lists the new files; each is in the app target.
- Runtime proof: New Part… (gate) → fill two pins and one gate → the Block pane shows the block with the names → switch to Footprint and back (run skill, est. 15 min).
- Doc sync: none in the user docs; `docs/file-format.md` is untouched (the preview saves nothing).
- Final gate (once, after Phase 3): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **The block is drawn by the schematic's own `ChipBlockView.paint`** — the author's choice, § 3 (rejected: a second drawing).
- **The preview is not interactive in v1** — the author's choice (no user-visible alternative was offered); § 1 non-goals.
- **The preview's scale is the schematic page's default `baseScale` 0.75** — the author's choice, so a user compares like with like.

## Corrections

- **The segment lives in the model** (2026-11-11): `PartEditorModel.previewPane`
  (`.block` by default), so test (d) checks it without a view.
- **The view takes the preview revision** (2026-11-11), as plan-32-D's preview does:
  `CatalogPart` compares by name and uuid, so a redraw needs the revision.
- **Centring** (found in the app, 2026-11-11): a `ScrollView` with
  `defaultScrollAnchor(.center)` put the block in the pane's bottom-right corner; the
  view now centres the block when it fits and scrolls only when it does not
  (`ViewThatFits`).

## Summary

The risk is divergence between the preview's chip and a placed chip; Phase 2 compares
pin endpoints directly. Untouched: the schematic page's drawing and the file format.
