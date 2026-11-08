# Export

**Schematical > Export** (the application menu, after About) writes the focused
window's schematic to a file. Added in plan-28.

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
| `SchematicExportFormat`, `SchematicExportCommands` (the menu) | `schematical/schematical/Document/SchematicExportCommands.swift` |
| `export(_:)`, `writeExport(_:to:)` | `schematical/schematical/Document/SchematicFileController.swift` |

Tests: `SchematicPainterTests`, `SchematicSceneTests`, `SVGPainterTests`,
`SchematicSVGTests`, `SchematicRasterTests`, `SchematicExportActionTests`.
Harness: `scripts/harness/painter-gate.swift` (the canvas must stay
pixel-identical when drawing code moves onto the painter),
`scripts/harness/export-svg.swift` (SVG next to the app's own export render),
`scripts/harness/export-menu.swift` (the menu in a running app; needs an
unlocked screen, since the window must become key).
