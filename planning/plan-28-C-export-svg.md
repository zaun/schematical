# plan-28-C: SVG writer

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-28-B

Adds `SVGPainter`, a `SchematicPainter` that writes SVG elements, and
`SchematicExport.svg(_ scene:)`, which measures a `SchematicScene` with
`BoundsPainter`, paints it through `SVGPainter` and returns a complete SVG
document: black line art on a transparent background, live `<text>` elements,
padded by 12 pt. The behavioral outcome of C: **`SchematicExport.svg(scene)`
returns a well-formed SVG of the whole schematic that rasterizes to the same
drawing as the app's export render, with text as `<text>`.** D adds the menu and
the PNG/JPG writers.

References:

- `planning/plan-28-A-export-painter.md` § 3 (design: `Ink`, `ExportPalette`,
  `SchematicPainter`), `planning/plan-28-B-export-scene.md` § 3 (`SchematicScene`,
  `BoundsPainter`) — both archived in `planning/completed/` once done.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-28-B complete | `ls planning/completed/plan-28-B-* \| wc -l` → `1` | MET (2026-11-07 at 3e7096d: 1) |
| The scene and bounds painter exist | `rg -c 'struct SchematicScene\|final class .*BoundsPainter\|struct BoundsPainter' schematical/schematical/Canvas/SchematicExportScene.swift` → `2` | MET (2026-11-07 at 3e7096d: 2) |
| `xmllint` and `rsvg-convert` are installed (used by this plan's checks) | `which xmllint rsvg-convert` → two paths | MET (2026-11-07 at 3e7096d: `/usr/bin/xmllint`, `/Users/justinzaun/local/brew/bin/rsvg-convert`; `rsvg-convert --version` → 2.61.3) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `Document/SchematicExport.swift` defines `SchematicExport` with
  `static let padding: CGFloat = 12`, `static func bounds(of scene:) -> CGRect?`
  (the ink rectangle outset by `padding`) and `static func svg(_ scene: SchematicScene) -> Data?`
  (`nil` for an empty scene).
- `Canvas/SVGPainter.swift` defines `SVGPainter: SchematicPainter` writing into a
  shared `final class` output box.
- The SVG of any schematic is well-formed XML, has `viewBox="0 0 W H"`, no
  background element, and only `#000` or `none` as `fill`/`stroke` values.
- SVG text is `<text>` (not paths).

### Non-goals (explicit constraints)

- No PNG/JPG, no menu, no save panel (D).
- No change to the live canvas or to A/B code beyond adding the writer.
- No embedded fonts, no CSS, no `<style>` element: attributes only, so the file
  works in Inkscape, Illustrator, Figma and browsers.

## 2. Current State

- After B, `SchematicScene(document)` paints through any `SchematicPainter`, and
  `BoundsPainter` returns the ink rectangle in world units (y down).
- `ExportPalette.alpha(for:)` (A) is the one place that decides what is drawn and how dark.
- Anchors used by migrated drawers are only `.center`, `.leading`, `.trailing`
  (`UnitPoint.y` = 0.5): `rg -n 'anchor:' <the eight views> | rg -o 'anchor: [^)]*' | sort | uniq -c`
  → 9 `.center`, 2 `.leading`, 1 `.trailing`, 3 `isLeft ? .leading : .trailing`,
  4 `isLeft ? .trailing : .leading`, 1 `left ? .leading : .trailing`; the layers add 2 `.center`.
- Strokes in use: plain width (`lineWidth: 1…3`), round caps/joins
  (`StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)`), dashes
  (`dash: [4, 3]`, `[8, 6]`), butt caps with miter joins (bus lines).
- `Path` exposes its elements through `Path.forEach` (`move`, `line`, `quadCurve`,
  `curve`, `closeSubpath`); ellipses (`Path(ellipseIn:)`) arrive as curves.

### Measured populations

| What | Count | Command |
|---|---|---|
| Text anchors | 22 (all y = 0.5) | the `uniq -c` command above plus 2 in the layers |

## 3. Design Overview

### 3.1 Element mapping (the whole format)

| Painter call | SVG |
|---|---|
| `fill(path, ink)` | `<path d="…" fill="#000" fill-opacity="α"/>` (`fill-opacity` omitted at α = 1). Not written when `ExportPalette.alpha` is nil. Default non-zero fill rule, as SwiftUI's. |
| `stroke(path, ink, style)` | `<path d="…" fill="none" stroke="#000" stroke-opacity="α" stroke-width="w" …/>`. Optional attributes only when not the SVG default: `stroke-linecap` (`round`/`square`), `stroke-linejoin` (`round`/`bevel`), `stroke-miterlimit` (when ≠ 4; SwiftUI's default is 10), `stroke-dasharray` ("4 3"), `stroke-dashoffset` (phase ≠ 0). |
| `draw(text, point, anchor)` | `<text x y transform="matrix(a b c d e f)" font-family font-size font-weight text-anchor dominant-baseline fill>escaped</text>`; `transform` only when the painter's transform is not the identity. `text-anchor`: x 0 → `start`, 0.5 → `middle`, 1 → `end`. `dominant-baseline="central"` (y = 0.5). Weight: regular 400, medium 500, semibold 600, bold 700. Family: monospaced → `Menlo, 'SF Mono', Consolas, 'DejaVu Sans Mono', monospace`; otherwise `'Helvetica Neue', Helvetica, Arial, sans-serif`. Fill `#000` with `fill-opacity` as for paths. |
| `translateBy`, `rotate` | Update `transform: CGAffineTransform` as `transform = operation.concatenating(transform)` (the new operation applies first, as `GraphicsContext` does); paths are written with the transform applied (`Path.applying`), so `d` is always absolute. |

Document: `<?xml version="1.0" encoding="UTF-8"?>` then
`<svg xmlns="http://www.w3.org/2000/svg" width="W" height="H" viewBox="0 0 W H">`, one
`<g>` holding the elements in paint order, `</svg>`. `W`, `H` = `ceil` of the padded bounds'
size; the painter is first translated by `(-bounds.minX, -bounds.minY)` of the padded
rectangle. No background element: transparency is the absence of one.

Numbers: at most 2 decimals, trailing zeros and the dot trimmed, `-0` written `0`,
`.` as separator regardless of locale (`String(format:)` is used without a locale).
`d` uses `M x y`, `L x y`, `Q cx cy x y`, `C c1x c1y c2x c2y x y`, `Z` separated by spaces.
XML text escapes `&`, `<`, `>`, `"` and `'`.

### 3.2 Why live text (decision) and what it costs

The owner chose `<text>` over outlines (2026-11-07). Consequence the implementer
must not "fix": glyph shapes and advance widths come from the viewer's fonts, so
text sits within about a pixel or two of where the PNG puts it. The 12 pt padding
(B's estimate margin) is why a viewer's wider font does not clip at the edge.

### 3.3 Correctness risk

The risk is the numeric mapping (stroke attributes, anchors, a rotated label) and a
color that slips past `ExportPalette`. Both are unit-tested on exact strings;
the end-to-end check rasterizes the SVG with `rsvg-convert` and compares it with the
app's own export render of the same scene (`ScreenPainter` with `.export`), expecting
differences only inside text boxes.

Rejected: outlines via Core Text (owner chose text); a `<style>` block with classes
(Illustrator/Figma import attributes more reliably); SVG `<rect>`/`<circle>` elements
(every shape is a `Path`, and rebuilding shape kinds from a `Path` is guesswork).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>`; never delete it.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — SVGPainter

- [x] Re-run the Prerequisites commands; record outputs in the table. (1; 2; both paths; also `which magick compare` → both in `/Users/justinzaun/local/brew/bin`, used by Phase 2)
- [x] `Canvas/SVGPainter.swift` (new): `SVGPainter` per § 3.1, with `static func d(for path: Path) -> String`,
      `static func number(_ value: CGFloat) -> String`, `static func escaped(_ text: String) -> String`, and an
      `Output` box holding the element strings.
- [x] `schematicalTests/SVGPainterTests.swift` (new), asserting exact strings:
      - `d`: a path `move(10,20) line(40,20) quadCurve(to:(60,40), control:(60,20)) curve(to:(10,60), c1:(40,60), c2:(10,40)) close` → `M10 20 L40 20 Q60 20 60 40 C40 60 10 40 10 60 Z`
      - `number`: 1.5 → `1.5`; 2.0 → `2`; 0.333333 → `0.33`; -0.001 → `0`; 100 → `100`.
      - stroke: `StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round, dash: [4, 3])` with `Ink.primary` → the element contains `stroke="#000"` `stroke-width="1.5"` `stroke-linecap="round"` `stroke-linejoin="round"` `stroke-dasharray="4 3"` and no `stroke-opacity`; `StrokeStyle(lineWidth: 1)` → contains `stroke-miterlimit="10"` (SwiftUI's default 10 ≠ SVG's 4) and none of `stroke-linecap`, `stroke-linejoin`, `stroke-dasharray`, `stroke-dashoffset`.
      - opacity: `Ink.primary.opacity(0.5)` stroke → `stroke-opacity="0.5"`; `Ink.secondary` → `0.6`; `Ink.secondary.opacity(0.5)` → `0.3`; `Ink.paper` fill → no element; `Ink(role: .color(.red))` → no element.
      - text: `.leading`/`.center`/`.trailing` → `start`/`middle`/`end`; `A&B<` → `A&amp;B&lt;`; weight `.semibold` → `font-weight="600"`; `monospaced: true` → family starts `Menlo`; `Ink.secondary` text → `fill-opacity="0.6"`.
      - transform: after `translateBy(x: 5, y: 5)`, `M0 0 L1 0` is written `M5 5 L6 5`; text drawn after `rotate(by: .degrees(-90))` carries a `transform` starting `matrix(0 -1 1 0 ` followed by the accumulated translation.
      - a copy of the painter that draws writes into the same output (copy-then-draw adds to the original's output).
      Also covered: `square` cap, `bevel` join, `stroke-dashoffset`, an `unlit` fill's `fill-opacity="0.15"`, a `.color` text writes nothing, `"'>` escapes. → 7 tests passed, `** TEST SUCCEEDED **`.

Acceptance: `SVGPainterTests` pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SVGPainterTests CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — SchematicExport.svg, tests, and the visual proof

- [ ] `Document/SchematicExport.swift` (new): `SchematicExport` with `padding`, `bounds(of:)`,
      `svg(_:)` per § 3.1 (measure with `BoundsPainter`; return `nil` when the scene is empty).
- [ ] `schematicalTests/SchematicSVGTests.swift` (new), using `TestSupport`:
      - empty document → `svg` is `nil`.
      - One `SN74HC00N` and one wire: parses with `XMLParser` (no error); root `svg` has `viewBox` equal to
        `0 0 W H` with `W == ceil(bounds.width)`, `H == ceil(bounds.height)`; no `<rect`; every `fill=`/`stroke=`
        value is `none` or `#000`; a `<text` containing `SN74HC00N`.
      - All 12 symbols (plan-28-A § 3.4 item 1): well-formed; contains `<text` for `SN74HC00N`, `GENERIC-LED`
        function text, `TE-GDS0204`'s designators; DIP lever 1 on vs off produce different SVG and the on one contains `fill-opacity="0.25"`.
      - `NHD-0216K1Z-FL-YBW` alone: fewer than 100 `<path` elements (its 5×8-dot matrix is 16×2 characters = 1 280 dots, so emitting them would exceed it).
      - Padding: the viewBox is exactly 24 larger than `BoundsPainter`'s result in each dimension, up to the `ceil`.
- [ ] `scripts/harness/export-svg.swift` (new): opens `be-6502.schematical` (21 parts, 148 wires, 2 buses) and
      builds the 12-symbol document; for each writes `<name>.svg` via `SchematicExport.svg`, and `<name>-app.png`
      by rendering the same `SchematicScene` in a `Canvas` with `ScreenPainter(…, palette: .export)` at scale 4
      through `ImageRenderer` (padding and translation as `SchematicExport.bounds`). Output dir is `CommandLine.arguments[1]`.
- [ ] Run it: `rm -rf /tmp/p28-hc && scripts/harness/build.sh scripts/harness/export-svg.swift /tmp/p28-hc && /tmp/p28-hc/H.app/Contents/MacOS/harness /tmp/p28-svg`.
      Then, per schematic: `xmllint --noout /tmp/p28-svg/<name>.svg` (no output),
      `rsvg-convert -z 4 /tmp/p28-svg/<name>.svg -o /tmp/p28-svg/<name>-svg.png`,
      `compare -metric AE -fuzz 30% /tmp/p28-svg/<name>-svg.png /tmp/p28-svg/<name>-app.png /tmp/p28-svg/<name>-diff.png`.
      View `<name>-svg.png`, `<name>-app.png` and `<name>-diff.png` with the Read tool.
- [ ] Record in the Corrections section the pixel counts `compare` printed and what the diff image showed.

Acceptance: both SVGs pass `xmllint --noout`; in the diff image the differing pixels lie inside label text boxes only — no
stroke, symbol outline or bus is missing, displaced or the wrong weight; the recorded image sizes of `-svg.png` and `-app.png`
are equal to within 1 px.
  Check: the three commands above per schematic, plus `xcodebuild … test -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/SVGPainterTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min: one harness build, 2 renders, one test run; the image diff is the only check that sees a displaced stroke).
Commit: —

## Validation Plan

- Tests: `SVGPainterTests` (exact strings, positive and negative: nil-alpha inks write nothing), `SchematicSVGTests` (document structure, empty, LCD size).
- Coverage check: `SchematicSVGTests` paints all 12 symbols; `SVGPainterTests` covers each optional stroke attribute (cap, join, miter limit, dash).
- Runtime proof: Phase 2's rsvg render and diff against the app's own export render, on the real `be-6502.schematical`.
- Doc sync: none in C (`docs/export.md` is written in D).
- Final gate (ONCE, after Phase 2): Phase 2's Check (est. 15 min). The full app suite runs once at the end of D.

## Decisions

- Live `<text>` elements, not outlines — decided by the owner, 2026-11-07.
- Black line art on transparent, no background element — decided by the owner, 2026-11-07 ("SVG and PNG … transparent").
- Padding 12 pt around the measured ink — author, 2026-11-07 (§ 3.1).
- Attributes only, no `<style>`; every shape is a `<path>` — author, 2026-11-07 (§ 3.3).

## Corrections

- § 3.1 says a `<text>` carries `transform` whenever the painter's transform is not the identity. Every export starts by translating by the padded bounds' corner, so that would put a `matrix(1 0 0 1 tx ty)` on every label. A translation-only transform is folded into `x`/`y` instead (as paths fold every transform into `d`); `transform="matrix(…)"` is written only when the painter is turned (the rotated port labels). `testTransforms` checks both: a rotated label → `transform="matrix(0 -1 1 0 5 7)" … x="0" y="0"`; `testText` checks an unturned label has no `transform`.
- `SVGPainter.Output` is a `nonisolated final class`, for the deinit abort recorded in plan-28-B's Corrections.

## Summary

Risk is the stroke/anchor mapping and unexpected colors; exact-string tests and a
rasterized diff cover them. Left untouched: the live canvas, A/B code, the menu,
PNG/JPG.
