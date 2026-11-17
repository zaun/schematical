# Export

**File > Save As…** (and Save of a window with no file yet) has a **Format**
menu: Schematical, **KiCad Project** ([KiCad](#kicad)) or **EasyEDA Pro
Project** ([EasyEDA Pro](#easyeda-pro)). **File > Export** (after Save As…) has two items: **Image…** (⇧⌘E), a
picture of the focused window's schematic, PCB page or made board, and
**Gerber PCB…**, its board as Gerber files (see Gerber below).

## Image

**Image…** opens an options sheet over the window, with a live preview,
then the save panel (`<window name> <Schematic|PCB|Front|Back>.<ext>`). The
choices are remembered between exports (`ImageExportOptions.saved`), except
the PCB layers.

| Image | Formats | Backgrounds (first is the default) | 1× is | Extra options |
|---|---|---|---|---|
| Schematic | PNG, JPG, SVG | Transparent, White | 1 px per canvas point | — |
| PCB | PNG, JPG | Dark (the PCB page's canvas), White, Transparent | 1 px per PCB page point (20 per mm) | Layers: PCB, each copper layer, Silkscreen, Document, Ratsnest (off by default) |
| Front Render / Back Render | PNG, JPG | Backdrop (Board Preview's), White, Transparent | 1 px per render point (10 per mm) | View (Top, Angled), Finish (HASL, ENIG) |

- **Size** is 1×, 2×, 4× or 8× (pixels per point); the sheet shows the
  pixel size. Any picture's longest side is capped at 8192 px
  (`pixelScale(for:wanted:)`).
- A JPG is never transparent: choosing JPG moves a Transparent background
  to the first opaque one. SVG is the schematic's only (vectors, labels as
  live `<text>`, transparent, sized in points); the other images offer PNG
  and JPG only (`ImageExportOptions.normalized`).
- **Schematic**: the ink plus 12 pt of padding on every side
  (`SchematicExport.padding`). The ink rectangle is measured by
  `BoundsPainter`, so empty label margins do not add a border. An empty
  schematic shows "Nothing to export" in the preview and disables Export….
- **PCB**: the PCB page as drawn (`PCBCanvasView` with `exporting`), framed
  on the board plus anything placed off it, plus 2 mm. No grid, hover, tool
  preview or selection. On a White or Transparent background the drill
  holes are cut through (`clearsHoles`); on Dark they are the canvas colour,
  as on screen.
- **Front / Back Render**: Board Preview's picture (`BoardRenderer`),
  framed tightly on the board and its tallest part with 24 pt round it
  (`fittedSize(pointsPerMM:)`).
- An export is not a save: the window's file and its unsaved-changes state do
  not change.
- Schematic exports never include the selection, hover, tool previews, the
  grid, pin markers or play-mode state (wire values, lit LEDs, LCD dots).
- Wires with a loose end are gray on screen and black in an export, like every
  wire. A loose end gets a dot only where 3 or more wires meet, on screen and
  in an export.

## Gerber

**File > Export > Gerber PCB** writes the board as Gerber X2 (RS-274X, mm,
`%FSLAX46Y46*%`) files, one per layer, and its Excellon drill files, in one zip (stored, not compressed),
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
| `.GTO` / `.GBO` | silkscreen | silkscreen texts, references (1 mm, centred on the body, or where the part's reference was moved to), pin-1 dots (0.4 mm), body outlines and cathode bands (0.15 mm pen), each only when its layer is Silkscreen |
| `.GKO` | outline (`Profile,NP`) | the board outline, 0.1 mm pen |
| `-PTH.drl` | plated drill (Excellon) | through-hole pads, vias, plated holes, header pads |
| `-NPTH.drl` | non-plated drill (Excellon), only when the board has any | bare holes, a standard's mounting holes, non-plated footprint holes |

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
- Not included yet: pick-and-place, and the Document layer (never printed).

### Drill files

Excellon (`DrillExport`), as KiCad writes it: `M48` header with
`FMAT,2`, `METRIC` and the X2 `; #@! TF.FileFunction,Plated,1,<layers>,PTH`
(or `NonPlated…NPTH`) comment, one tool per hole diameter (to 1 µm,
smallest first), then `%`, `G90`, `G05`, each tool's hits and `M30`.
Coordinates are absolute decimal mm in the Gerber files' frame (y up from
the board's bottom edge), so they line up with the copper. A slot is a
routed `G85` slot from one end centre to the other, its width the tool.

## KiCad

**File > Save As…**, Format **KiCad Project**, writes the schematic and its
board as a KiCad 8 project (KiCad 8 and later open it): next to the chosen
file, `<name>.kicad_pro`, `<name>.kicad_sch` and `<name>.kicad_pcb`
(`KiCadExport`). The panel's Format menu changes the file type and the
extension of the name. Like any export it is not a save: the window's file and
its unsaved-changes state do not change.

**Schematic** (`KiCadSchematicWriter`). One sheet, 0.127 mm per canvas unit, so
the app's 10-unit grid is KiCad's 1.27 mm grid and every wire end lands on a pin
end.

- Each part is a generated symbol in the file's own `Schematical` library: a
  rectangle with the pins on the sides the app draws them (a flipped part is its
  own symbol; a two-terminal part's rectangle is just its pins' span). Pin names
  keep their active-low bars (`~{CE}`), pin types follow the roles, and power and
  ground pins are power inputs. Value is the part number (a resistor's or generic
  capacitor's value), Footprint is `Schematical:<footprint id>`, Datasheet is the
  datasheet URL.
- VCC and GND symbols are power symbols named for the rail (`5V`, `3V3`, `GND`),
  so KiCad joins every one of a voltage, as the app does; each rail also gets a
  `PWR_FLAG`. A pull resistor gets a second pin, on its far side, joined to a power
  symbol of its rail.
- Wires (one per straight run), junctions where 3 or more runs meet, net labels
  (local labels; the flag shape is not kept), No Connects and text blocks (framed
  ones with a box). Buses are drawn as KiCad buses with each port as a box, and every
  wired port pin carries a net label of its bus wire's name, so the nets are the
  same as the app's (linked buses share the label; unrelated wires of one name get
  `D0`, `D0_2`).
- A card board's connector (`EDGE1`, `HEADER1`) is a symbol beside the drawing,
  one pin per connector pin, each on its net by a label or a power symbol.

**Board** (`KiCadBoardWriter`). Millimetres as on the PCB page, moved 30 mm inside
the page. Footprints are written with the part's reference, value, body outline,
pin-1 dot and diode band on the layers the part's marks name, pads in the footprint's
own frame (a turn of `r` degrees clockwise is KiCad's angle `-r`, a Bottom footprint
has its x mirrored and its pads on `B.*`), traces, vias, Plane layers as unfilled
zones (press B in KiCad to fill), the outline on `Edge.Cuts`, board texts, the card's
fingers or header pads, and holes. A 1-layer board has the two copper layers KiCad
needs. Pads carry the net of their pin, named as KiCad names it: a rail's name, a net
label as `/NAME` (a `/` in it as `{slash}`), else `Net-(U1-A0)` after the
alphabetically first pin, `unconnected-(U1-A0-Pad1)` for a pin on nothing. So the
board and the schematic agree (KiCad's Update PCB from Schematic finds no difference),
and each footprint's path is its part's uuid.

**Not exported**: net classes, per-net layer rules and groups, simulation state,
the Document layer's contents on the board other than texts, and a real symbol or
footprint library (the `Schematical` library is only in the files, so the project
file turns off KiCad's missing-library checks). Design rules go to the project's
design settings.

**Checking it.** `KiCadExportTests` checks the files' shape, that a footprint's pads
land where the app puts them at every turn and side, and the Format menu. The rest was
checked with KiCad 10's `kicad-cli`: `sch export netlist` gives the same pin groups
and the same net names as the pads on the board (on `AFU`, `LFU` and `RegFile`), `pcb
drc` finds no schematic parity difference, a board of DIP and SOIC parts at every
turn on both sides has the same hole positions as the app's drill file, and its
pad outlines in `pcb export svg` are the app's.

## EasyEDA Pro

**File > Save As…**, Format **EasyEDA Pro Project**, writes the schematic and its
board as one `.epro` file (`EasyEDAProExport`), which EasyEDA Pro takes with **File ▸
Import ▸ EasyEDA Professional**. Like the KiCad project it is an export: the
window's file and its unsaved-changes state do not change.

The format is EasyEDA Pro's "V2" project archive (the one its published spec and
KiCad's importer describe; EasyEDA Pro 3 still imports it, and older Pro versions open it,
which they would not a V3 `.epro2`): a zip of

| File | What |
|---|---|
| `project.json` | the devices (symbol + footprint + attributes), symbols, footprints, the schematic and its one sheet, the PCB and the board |
| `SYMBOL/<id>.esym` | one per distinct part, flipped part, power rail and pull resistor |
| `FOOTPRINT/<id>.efoo` | one per distinct footprint (and set of absent pins) |
| `SHEET/<schematic id>/1.esch` | the schematic |
| `PCB/<pcb id>.epcb` | the board |

Every file but `project.json` is JSON lines: one JSON array per line, the first
element naming the item (`["WIRE", "e12", …]`). Ids are 16 hex digits made from
what they name, so one document always gives the same bytes.

**Units and axes.** The board is in mil with y up (a board point is
`(x / 0.0254, -y / 0.0254)`); a footprint is written around its body centre, a component
turned `r` degrees clockwise is turned `-r` (counter-clockwise), and one on Bottom is flipped
top to bottom first and turned `180 - r`. The schematic is in 0.01 inch, one canvas unit
each, the sheet's y up and a symbol's own y down, the part's origin at its body's top-left;
a part is at `(origin.x, -origin.y)`, a pin at its offset from the origin.

**Schematic** (`EasyEDAProSchematicWriter`): a part is its generated symbol (a body
rectangle, a pin where the app draws each one, active-low names with a `~` bar), placed
with its device, designator, footprint and value; VCC and GND are net-flag symbols
(`Global Net Name`), and a pull resistor has its far pin on a flag of its rail. Each wire is
one `WIRE` with a `NET` attribute: a rail's name, a net label's text (shown at the
label), a bus pin's wire name, or after its first pin. Bus lines and ports are plain lines,
text blocks are `TEXT` (framed ones with a box), and a card board's connector is a
symbol beside the drawing with a named stub or a net flag on each pin.

**Board** (`EasyEDAProBoardWriter`): `LAYER`s (a 1-layer board gets Top and Bottom, EasyEDA's
minimum), the design rules' track width, a `NET` for every net a pad carries, the outline
(`POLY` on the outline layer), a `POUR` for each inner Plane on a net, a `LINE` per trace
segment, `VIA`s, a `PAD` for each card finger and hole, `STRING` for board texts, and each
part as a `COMPONENT` with `Designator`, `Device` and `Footprint` attributes and a
`PAD_NET` per pad on a net. A net of one pin is left without a net.

**Not exported**: net classes and layer rules, No Connect flags, the schematic's per-part
silkscreen marks (a footprint's body is on the assembly layer and its pin-1 dot on Top
silkscreen whatever the part's marks say), cathode bands, a chamfered pad's chamfer, and
3D models.

**What was checked, and what was not.** `EasyEDAProExportTests` checks the files' shape and
that every pad, at every turn and side, is where the app puts it. The rest was checked by
loading the output with KiCad 10's EasyEDA Pro importer (`pcbnew`'s
`PCB_IO_MGR.EASYEDAPRO`, from Python): all 224 pads of a board of DIP and SOIC parts turned
every way on both sides land at the same relative positions as the app's, the nets of the
pads, the tracks (position, layer, width), the via, the inner-layer pour, the text and the
outline come out as written, and `AFU`, `LFU` and `RegFile` load whole. KiCad cannot import an
`.epro` schematic from the command line, so the schematic is checked only against the format's
published layouts (the item fields KiCad's parser reads) and for being self-consistent: every wire end
that is a pin lands on that pin's place under the convention above. **No file has been opened in EasyEDA
Pro itself**, so a field the spec and KiCad's importer leave open may still differ
(text alignment codes, which the spec and KiCad number differently, are the likely one).

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
| `ImageExportView`, `ImageExportFormat`, `ImageExportBackground`, `ImageExportOptions`, `ImageExport` (every image, its size, PNG/JPG encoding) | `schematical/schematical/Document/ImageExport.swift` |
| `ImageExportDialog` (the options sheet) | `schematical/schematical/Document/ImageExportDialog.swift` |
| `SchematicExportMenu` (the submenu; `SchematicFileCommands` adds it to File) | `schematical/schematical/Document/SchematicExportCommands.swift` |
| `exportImage()`, `writeImage(_:to:)`, `exportGerber()`, `writeGerber(to:)`, `saveAs()`, `exportKiCad(to:)`, `writeKiCad(to:)` | `schematical/schematical/Document/SchematicFileController.swift` |
| `SaveFormat`, `SaveFormatPicker` (Save As's Format menu) | `schematical/schematical/Document/SaveFormatPicker.swift` |
| `KiCadExport` (the three files, project file, shared names), `KiCad` (number, string, uuid formats) | `schematical/schematical/Document/KiCadExport.swift` |
| `KiCadSchematicWriter` | `schematical/schematical/Document/KiCadSchematicWriter.swift` |
| `KiCadBoardWriter` | `schematical/schematical/Document/KiCadBoardWriter.swift` |
| `ExportNetNames` (the net names both exports use) | `schematical/schematical/Document/KiCadExport.swift` |
| `EasyEDAProExport` (the archive, `project.json`), `EProJSON` | `schematical/schematical/Document/EasyEDAProExport.swift` |
| `EasyEDAProSchematicWriter` (`.esch`, `.esym`) | `schematical/schematical/Document/EasyEDAProSchematicWriter.swift` |
| `EasyEDAProBoardWriter` (`.epcb`, `.efoo`) | `schematical/schematical/Document/EasyEDAProBoardWriter.swift` |
| `GerberExport` (the layer files) | `schematical/schematical/Document/GerberExport.swift` |
| `GerberWriter` (apertures, flashes, strokes, regions, polarity), `GerberOutline` (path flattening, glyph outlines) | `schematical/schematical/Document/GerberWriter.swift` |
| `DrillExport` (the Excellon files) | `schematical/schematical/Document/DrillExport.swift` |
| `ZipArchive` | `schematical/schematical/Document/ZipArchive.swift` |

Tests: `SchematicPainterTests`, `SchematicSceneTests`, `SVGPainterTests`,
`SchematicSVGTests`, `SchematicRasterTests`, `SchematicExportActionTests`,
`GerberExportTests`.
Harness: `scripts/harness/painter-gate.swift` (the canvas must stay
pixel-identical when drawing code moves onto the painter),
`scripts/harness/export-svg.swift` (SVG next to the app's own export render),
`scripts/harness/export-menu.swift` (the plan-28 menu of three schematic
items in a running app; it predates the Image… sheet and no longer matches
the menu).
