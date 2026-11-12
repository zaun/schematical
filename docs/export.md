# Export

**File > Export** (after Save As…) writes the focused window's schematic to a
file (added in plan-28), or its board as Gerber files (see Gerber below).

| Item | File | Background | Size |
|---|---|---|---|
| SVG Schematic | `.svg`, vectors, labels as live `<text>` | transparent | the padded ink rectangle, in points |
| PNG Schematic | `.png`, RGBA | transparent | 4 px per canvas point |
| JPG Schematic | `.jpg`, quality 0.92 | white | 4 px per canvas point |

- The picture is the ink plus 12 pt of padding on every side
  (`SchematicExport.padding`). The ink rectangle is measured by
  `BoundsPainter`, so empty label margins do not add a border.
- PNG and JPG are 4 px per point unless the longest side would pass 8192 px;
  then the scale drops so the longest side is 8192 px (`pixelScale(for:)`).
- An export is not a save: the window's file and its unsaved-changes state do
  not change. An empty schematic shows "Nothing to export" and no save panel.
- Exports never include the selection, hover, tool previews, the grid, pin
  markers, play-mode state (wire values, lit LEDs, LCD dots) or the PCB.
- Wires with a loose end are gray on screen and black in an export, like every
  wire. A loose end gets a dot only where 3 or more wires meet, on screen and
  in an export.

## Gerber

**File > Export > Gerber PCB** writes the board as Gerber X2 (RS-274X, mm,
`%FSLAX46Y46*%`) files, one per layer, in one zip (stored, not compressed),
named `<schematic> Gerbers.zip`. The files inside are named after the
schematic, with anything but letters, digits, `-` and `_` turned into `_`
(`gerberBaseName`), and use the Protel extensions fabs expect:

| File | Layer | What is on it |
|---|---|---|
| `.GTL` | Top copper | traces on Top, Top surface-mount pads and card fingers, every plated through-hole pad, via and plated hole land |
| `.G1`…`.G4` | inner copper, in stack order | as above for the layer; a Plane is the board pulled back by the min copper-to-edge, less its clearances, as the PCB page draws it |
| `.GBL` | Bottom copper | as Top, for Bottom |
| `.GTS` / `.GBS` | solder mask (`FilePolarity,Negative`: drawn = opening) | the side's surface-mount pads and fingers, every plated through-hole pad, every bare hole, each grown by the Mask expansion design rule. Vias are tented |
| `.GTP` / `.GBP` | paste | the side's surface-mount part pads (not fingers) |
| `.GTO` / `.GBO` | silkscreen | silkscreen texts, references (1 mm, centred on the body), pin-1 dots (0.4 mm), body outlines and cathode bands (0.15 mm pen), each only when its layer is Silkscreen |
| `.GKO` | outline (`Profile,NP`) | the board outline, 0.1 mm pen |

- Everything is seen from the top, as on the PCB page: bottom silkscreen
  reads mirrored. Board y grows down; Gerber y grows up, so a file's y is
  the board's top edge minus the board y, and the board sits at y ≥ 0.
- A 1-layer board has only the Top files and the outline.
- Circles and plain rectangles are aperture flashes; ellipses, rounded and
  chamfered rectangles are regions of the same outline the canvas fills
  (`PCBCanvasView.outline(of:)`), curves cut into steps of at most 0.05 mm.
  A grown shape (mask expansion, Plane clearance) is the shape plus its
  outline stroked with a round pen twice the growth wide.
- Text is real glyph outlines, from the same fonts the canvas uses. Each
  glyph's counters (the hole in an O) are drawn in clear polarity, so a
  silkscreen file draws its text before its lines and dots.
- Not included yet: drill files (Excellon), pick-and-place, and the
  Document layer (never printed).

## Black line art

Drawing code never names a color. It names an `Ink` role, and the painter
turns the role into a color. On screen the palette is the app's colors; in an
export every drawn thing is black at an alpha, or not drawn (`ExportPalette`).

| Role | Screen | Export alpha |
|---|---|---|
| `primary` | `Color.primary` | 1 |
| `secondary` | `Color.secondary` (labels: the hierarchical style) | 0.6 |
| `accent` | `Color.accentColor` | 1 (never in an export: nothing is selected) |
| `paper` | `controlBackgroundColor` (chip bodies, port bodies) | not drawn |
| `power` / `ground` | red / blue (pin names) | 1 |
| `switchOn` / `knobOn` | green 0.85 / white (DIP lever ON) | 0.25 / 1 |
| `unlit` | white 0.1 (7-segment bar) | 0.15 |
| `displayFace` | `Color(white: 0.12)` (7-segment face) | not drawn |
| `faceEdge` | not drawn | 1 (outlines the 7-segment / LCD face) |
| `lamp(name)` | the LED color | 1 |
| `display(r, g, b)` | the LCD face and dot colors | not drawn |
| `color(c)` | `c` (simulation wire colors, tool previews) | not drawn |

An ink's `opacity` multiplies the role's alpha (`Ink.secondary.opacity(0.5)`
exports at 0.3).

## Adding a part symbol

A new part symbol draws through `SchematicPainter`, with `Ink` and
`PaintedText`, never `GraphicsContext` or `Color`:

- its view gets `static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, …)`,
  and its `body` is `Canvas { context, _ in var painter = ScreenPainter(context); Self.paint(&painter, …) }`;
- it needs a case in `SchematicScene.paint`'s `switch chip.symbol`. That switch
  has no `default`, so a symbol without one does not compile.

Pick roles, not colors. A fill that should not appear in black line art uses
`paper` or `display`; a face that is filled on screen gets a `faceEdge`
stroke so the export shows its outline.

## SVG

Attributes only (no `<style>`, no CSS, no embedded fonts), one `<g>` in paint
order, every shape a `<path>` with absolute coordinates, `fill`/`stroke` only
`#000` or `none`, alpha in `fill-opacity` / `stroke-opacity`. Labels are
`<text>` with `font-family` Menlo… (monospaced) or 'Helvetica Neue'…, so their
glyphs come from the viewer's fonts and can sit a pixel or two from where the
PNG puts them; the padding keeps a wider font from clipping. A turned label
(a bus port's wire names) carries `transform="matrix(…)"`.

## Where the code lives

| What | File |
|---|---|
| `Ink`, `PaintedText`, `SchematicPainter`, `ExportPalette`, `ScreenPainter` | `schematical/schematical/Canvas/SchematicPainter.swift` |
| `SchematicScene`, `BoundsPainter` | `schematical/schematical/Canvas/SchematicExportScene.swift` |
| `SVGPainter` | `schematical/schematical/Canvas/SVGPainter.swift` |
| `SchematicExport` (`svg`, `png`, `jpg`, padding, pixel scale) | `schematical/schematical/Document/SchematicExport.swift` |
| `SchematicExportFormat`, `SchematicExportMenu` (the submenu; `SchematicFileCommands` adds it to File) | `schematical/schematical/Document/SchematicExportCommands.swift` |
| `export(_:)`, `writeExport(_:to:)`, `exportGerber()`, `writeGerber(to:)` | `schematical/schematical/Document/SchematicFileController.swift` |
| `GerberExport` (the layer files) | `schematical/schematical/Document/GerberExport.swift` |
| `GerberWriter` (apertures, flashes, strokes, regions, polarity), `GerberOutline` (path flattening, glyph outlines) | `schematical/schematical/Document/GerberWriter.swift` |
| `ZipArchive` | `schematical/schematical/Document/ZipArchive.swift` |

Tests: `SchematicPainterTests`, `SchematicSceneTests`, `SVGPainterTests`,
`SchematicSVGTests`, `SchematicRasterTests`, `SchematicExportActionTests`,
`GerberExportTests`.
Harness: `scripts/harness/painter-gate.swift` (the canvas must stay
pixel-identical when drawing code moves onto the painter),
`scripts/harness/export-svg.swift` (SVG next to the app's own export render),
`scripts/harness/export-menu.swift` (the menu in a running app; needs an
unlocked screen, since the window must become key).
