# plan-39-C: Active-low pins — the overline in block symbols

Last updated: 2026-11-13
Effort: medium (1h–2h)
Depends on: plan-39-B

C draws it. Its outcome: a block symbol's pin name has an overline over each
active-low alternative — `CE#` (flagged) draws as C̅E̅ with no `#`, `A0/CS`
flagged `["CS"]` draws A0/C̅S̅ — on the canvas, in the raster export and in
the SVG export. A pin without the flag draws exactly as before.

References:

- plan-39-A § 3 (the key's rules), plan-39-B § 3 (`ChipPin.alternatives`).
- `schematical/schematical/Canvas/SchematicPainter.swift` (`PaintedText`,
  the `SchematicPainter` protocol, `ScreenPainter`), `Canvas/SVGPainter.swift`,
  `Canvas/SchematicExportScene.swift` (`BoundsPainter`).
- The three block-style symbol views that paint pin names:
  `Canvas/ChipBlockView.swift:109`, `Canvas/SevenSegmentSymbolView.swift:81`,
  `Canvas/CharacterLCDSymbolView.swift:83` (`grep -rn 'PaintedText(slot.pin.name' schematical/schematical` → these three).
- Precedent: the datasheet renderer already draws a trailing `#` as an
  overline (`planning/spike-datasheet-format/format.md` § Inline markup).

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-B complete | `grep -c 'activeLowMask' schematical/schematical/Backend/PartCatalog.swift` → ≥ 1, and B's phases are ticked | MET (2026-11-14: 5; B archived, all ticked) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue. If you stop, report all
> prerequisites.

## 1. Goal

- `PaintedText` can carry overline ranges; all three painters draw them
  (screen/raster, SVG; the bounds painter ignores them).
- `ChipPin.paintedName(…)` builds the text: the alternatives joined by `/`,
  each active-low alternative's trailing `#` stripped (when the alternative
  is longer than one character), an overline range over each active-low
  alternative.
- The three symbol views use it.

### Non-goals (explicit constraints)

- No layout change: block sizes, stub lengths and margins are fixed
  (`ChipLayout.bodyWidth` 120, `labelMargin` 30), independent of text width,
  so a shorter name (the `#` removed) moves nothing.
- A pin without the flag draws byte-identically to today, including a name
  that ends in `#` (`CE#` unflagged stays `CE#`; only shipped data is
  guaranteed flagged, by plan-39-D's check).
- No change to PCB drawing, the datasheet renderer, or any text but pin
  names.

## 2. Current State

- `PaintedText` (`SchematicPainter.swift:54–69`): `string size weight
  monospaced ink`. `SchematicPainter` (`:76–82`) has `fill stroke draw
  translateBy rotate`. Three conformers: `ScreenPainter` (`:146`, wraps
  `GraphicsContext`; the live canvas and the raster export), `SVGPainter`
  (`SVGPainter.swift:7`) and `BoundsPainter` (`SchematicExportScene.swift:83`,
  estimates text boxes at 0.6 em per monospaced character).
- `ScreenPainter.draw` builds a SwiftUI `Text` and calls
  `context.draw(resolved, at:anchor:)` (`:165–169`).
- `SVGPainter.draw` writes one `<text>` element whose body is the escaped
  string (`SVGPainter.swift:50–69`).
- Test painters: `RecordingPainter` (`schematicalTests/SchematicSceneTests.swift:15`)
  records a `PaintCall(kind, ink)` per call, not the text; `CapturePainter`
  (`PartPreviewTests.swift:41`) is another. Phase 2's test defines its own
  text-recording painter.

### Verified properties

| Claim | How verified |
|---|---|
| SwiftUI can measure a resolved text, so the screen painter needs no width estimate | `grep -n 'public struct ResolvedText' -A4 <SDK>/SwiftUICore.framework/.../arm64e-apple-macos.swiftinterface` → `func measure(in size: CGSize) -> CGSize` and `func firstBaseline(in size: CGSize) -> CGFloat` (line 7376–7379 of the macOS SDK's SwiftUICore interface) |
| SwiftUI `Text` has no overline style (only underline and strikethrough), so the overline is a stroked line | the same interface lists `underline`/`strikethrough` modifiers and no overline |
| The three views are the only block-style symbols that print pin names | `grep -rn 'pin.name' schematical/schematical/Canvas --include='*.swift'` → ChipBlockView, SevenSegment, CharacterLCD paint it; BarrelJack uses it as a lookup key only |
| Pin-name text size never feeds layout | `ChipLayout.bodyWidth`/`labelMargin` are constants (`SchematicModel.swift:189`, `:192`); `grep -n 'textSize' schematical/schematical/Canvas/SchematicModel.swift` → 0 |

## 3. Design Overview

**`PaintedText.overlines: [Range<Int>] = []`** — character offsets into
`string` (as `string.count` counts), sorted, non-overlapping, inside the
string. `init` gains the parameter with an empty default.

**`ChipPin.paintedName(size:weight:monospaced:ink:) -> PaintedText`**
(`Canvas/PinNameText.swift`, new): walk `alternatives`; for an active-low
alternative whose text ends in `#` and is longer than one character, drop the
`#`; join with `/`; record the range of each active-low alternative.
Examples: `CE#` flagged → `CE`, `[0..<2]`; `OE` flagged → `OE`, `[0..<2]`;
`A0/CS` `["CS"]` → `A0/CS`, `[3..<5]`; `NRST/PA0` flagged true → `NRST/PA0`,
`[0..<4, 5..<8]`; `#` flagged → `#`, `[0..<1]`; `CE#` unflagged → `CE#`,
no ranges.

**ScreenPainter.** After `context.draw`, for each range: resolve the whole
text and the prefix and segment texts with the same font; with the anchor,
`left = point.x − full.width · anchor.x`; `top = point.y − full.height ·
anchor.y`; `baseline = top + resolved.firstBaseline(in:)`; the line runs from
`left + prefix.width` to `left + prefix.width + segment.width` at `y =
baseline − 0.72 · size − 1.5` (0.72 em is the system font's cap height with a
1.5 pt gap), width `max(0.75, size / 10)`, drawn through `self.stroke`, so
the palette (screen colour, export black) applies to it as to the text.

**SVGPainter.** The `<text>` body is split at the range edges; each overlined
piece is `<tspan text-decoration="overline">…</tspan>`; every piece is
escaped as now.

**BoundsPainter.** Unchanged: its box height is 1.2 em, which already covers
the overline's rise (the line sits within 0.72 em + 1.5 pt above the
baseline of a 1.2 em box).

**Where risk is.** The screen overline's position (a pixel-level look). It
is checked by a rendered-pixel test and by looking at the real app, not by
reading code.

**Gate class.** Behaviour-adding for flagged pins, byte-identical for the
rest: the existing SVG and painter tests pass unchanged, and a pin without
the flag draws the same `PaintedText` as before (`overlines` empty).

**Rejected alternatives.** *Unicode combining overline (U+0305)* — renders
with a different advance in SF Mono and not at all in some SVG fonts.
*Drawing the name as separate `Text` pieces* — three draws per name and
kerning drift. *Making `activeLow` a `Text` attribute* — SwiftUI has none.

## 4. Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; `- [~]` partial; moot tasks stay with a reason; fill `Commit:`
> when a phase lands; an unticked box means NOT DONE.

### Phase 1 — text model and the SVG painter

- [x] `SchematicPainter.swift`: `PaintedText.overlines` (§ 3); the two
      existing initialisers' call sites compile unchanged. — done: plus `PaintedText.substring(_:)` for the painters
- [x] `Canvas/PinNameText.swift` (new): `ChipPin.paintedName(…)` (§ 3).
- [x] `SVGPainter.swift`: the `<tspan text-decoration="overline">` body
      (§ 3). — done: `SVGPainter.body(_:)`
- [x] Tests: `schematicalTests/PinNameTextTests.swift` (new): the seven
      examples of § 3. `SVGPainterTests.testText` stays as is; add
      `testTextOverline`: `PaintedText("A0/CS", …, overlines: [3..<5])`
      emits `A0/<tspan text-decoration="overline">CS</tspan>`, and `&`, `<`
      in the pieces are still escaped. — done: PinNameTextTests 7 tests (the six § 3 examples, of which `A0/CS` counts once, plus `A0/CS#` flagged `["CS#"]`); `testTextOverline` 5 cases

Acceptance: the text model and SVG are right; unflagged SVG is unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p39-dd test -only-testing:schematicalTests/PinNameTextTests -only-testing:schematicalTests/SVGPainterTests -only-testing:schematicalTests/SchematicSVGTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min; the SVG suites prove the unflagged path did not move).
  Observed 2026-11-14: PinNameTextTests 7, SVGPainterTests 8, SchematicSVGTests 6 — `Executed 21 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 02cd6b14

### Phase 2 — the screen painter and the three views

- [x] `SchematicPainter.swift` `ScreenPainter.draw`: draw the overlines
      (§ 3). — done: `drawOverlines`, through `self.stroke`
- [x] `ChipBlockView.swift:109`, `SevenSegmentSymbolView.swift:81`,
      `CharacterLCDSymbolView.swift:83`: build the name with
      `slot.pin.paintedName(…)` (same size, weight, monospaced and ink as
      today).
- [x] Tests: `schematicalTests/PinNameOverlineTests.swift` (new):
      (a) with a painter the test file defines (it records every
      `PaintedText` passed to `draw`; `RecordingPainter` records only the
      kind and ink), paint a flagged block chip with `ChipBlockView.paint`
      and assert the name's `PaintedText` has the overline ranges and an
      unflagged pin's has none; (b) pixel test: render a Canvas (`ImageRenderer`, scale 2) that
      draws `ScreenPainter` text `"OE"` (monospaced, 10 pt) once plain and
      once with `overlines: [0..<2]`; the differing pixels' bounding box is
      ≥ 90 % of the text's measured width, ≤ 4 px tall, and lies entirely
      above the topmost glyph-ink row of the plain render. — done: (a) `testBlockChipPaintsOverlinedNames` + `testShippedSRAMIsOverlined` (AS6C1008-55PCN: CE/OE/WE overlined, no `#`), (b) `testOverlinePixels`; 3 tests

Acceptance: a flagged pin draws an overline of the right width above the
glyphs; an unflagged one is unchanged.
  Check: `xcodebuild … test -only-testing:schematicalTests/PinNameOverlineTests -only-testing:schematicalTests/SchematicSceneTests -only-testing:schematicalTests/PartPreviewTests` → `** TEST SUCCEEDED **` (est. 6 min).
  Observed 2026-11-14: PinNameOverlineTests 3, SchematicSceneTests 9, PartPreviewTests 1 — `Executed 13 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: —

### Phase 3 — seen in the real app

- [ ] Build and open the app (`xcodebuild … build`, then `open` the built
      app); File ▸ New Part…, add a pin named `CE#` with Low ticked and a
      pin `A0/CS` with only CS ticked, and look at the block preview and,
      after placing the saved part, the schematic canvas. Move the pointer
      to (0, 0) when done (`AGENTS.md` § Testing in the app).
- [ ] Export the schematic as SVG and PNG (File ▸ Export) and open both; the
      overlines are present in both.
- [ ] Record what was seen in this phase's Commit note (one line).

Acceptance: overlines are visible in the Part Editor preview, the canvas and
both exports, over the right letters.
  Check: the screenshots of the three places (`screencapture -R` of the
  window region; look at each) show the bar over `CE`, over `CS` only in
  `A0/CS`, and not over `A0` (est. 15 min; this is the only check that sees
  real rendering).
Commit: —

## Validation Plan

- Tests: `PinNameTextTests`, `SVGPainterTests`, `SchematicSVGTests`,
  `PinNameOverlineTests`, `SchematicSceneTests`, `PartPreviewTests`.
- Coverage check: each new test file reports ≥ 1 executed test in its run
  (`Test Suite 'PinNameOverlineTests' passed … Executed N tests`, N ≥ 2).
- Runtime proof: Phase 3.
- Doc sync: none; the drawing has no doc page. `docs/parts-format.md` § Pins
  (plan-39-A) already says `#` is dropped when drawn.
- Final gate: plan-39-L, once.

## Decisions

- Names keep their `#`; the drawn name drops it and gets the overline — owner,
  2026-11-13.
- Per-alternative overline — owner, 2026-11-13.
- Author's calls: the overline is a stroked line positioned from measured
  text; layout is not changed; a lone `#` is not stripped.

## Corrections

(empty until execution)

## Summary

The risk is the on-screen position and width of the overline, which only a
rendered check sees (Phases 2 and 3). Layout, simulation and file formats are
untouched.
