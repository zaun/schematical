# Export

**File › Export** (after the document's Save, Duplicate, Rename…, Move To… and Revert To
items) has five items. **KiCad Project…** ([KiCad](#kicad)) and **EasyEDA Pro
Project…** ([EasyEDA Pro](#easyeda-pro)) write the whole design for another tool; the Save
panel has no Format menu. **Image…** (⇧⌘E), a
picture of the focused window's schematic, PCB page or made board, and
**PDF…**, the schematic and each PCB layer at 1:1 (see [PDF and print](#pdf-and-print)), and
**Gerber PCB…**, its board as Gerber files (see Gerber below). **File > Print…** (⌘P) prints the same pages.
The exports and Print are `SchematicExporter`'s, one per document; an export is never a save, and
does not mark the document edited.

## Image

**Image…** opens an options sheet over the window, with a live preview,
then the save panel (`<window name> <Schematic|PCB|Front|Back>.<ext>`). The
choices are remembered between exports (`ImageExportOptions.saved`), except
the PCB layers.

| Image | Formats | Backgrounds (first is the default) | 1× is | Extra options |
|---|---|---|---|---|
| Schematic | PNG, JPG, SVG | Transparent, White | 1 px per canvas point | — (a shown title block, Window ▸ Document Info, is drawn with it) |
| PCB | PNG, JPG | Dark (the PCB page's canvas), White, Transparent | 1 px per PCB page point (20 per mm) | Layers: PCB, each copper layer, Silkscreen, Document, Ratsnest (off by default) |
| Front Render / Back Render | PNG, JPG | Backdrop (Board Preview's), White, Transparent | 1 px per render point (10 per mm) | View (Top, Angled) |

- A render draws bare copper (pads with no part on them, rings, mounting holes) in the board's finish
  (PCB sidebar ▸ Board Details… ▸ Stackup ▸ Finish): HASL and lead-free HASL silver, ENIG gold, OSP bare copper. Board
  Preview does the same and has no finish menu of its own.
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
named `<schematic> Gerbers.zip`. Before the save panel the export runs the Design Rule Check under the board's own
rules; a board with any problem gets a warning ("1 short circuit, 2 unrouted connections and 3 other problems") with
**Cancel** and **Export Anyway**, and a clean board goes straight to the panel. The files inside are named after the
schematic, with anything but letters, digits, `-` and `_` turned into `_`
(`gerberBaseName`), and use the Protel extensions fabs expect:

| File | Layer | What is on it |
|---|---|---|
| `.GTL` | Top copper | the copper pours on Top, traces on Top, Top surface-mount pads and card fingers, every plated through-hole pad, via and plated hole land |
| `.G1`…`.G4` | inner copper, in stack order | as above for the layer; a Plane is the board pulled back by the min copper-to-edge, less its clearances, as the PCB page draws it; a pour on a Wire layer as on Top |
| `.GBL` | Bottom copper | as Top, for Bottom |

  A Plane clears every cutout: after the board region, each cutout is a clear-polarity region and a clear stroke twice the min copper-to-edge wide round
  its loop, so the Plane keeps the copper-to-edge band from a cutout as it does from the outline (plan-65). Nothing else in the set changes for a cutout.

  A **copper pour** (plan-52) is its fill as the PCB page computes it (`PourFiller`: the pour's rectangle, less the other nets' copper and the keep-outs
  grown by the clearance, the thermal rings less their spokes): one dark region (`G36`) per kept piece of the fill, then each hole of the piece as a
  clear-polarity region, drawn before the layer's traces, pads and vias so these sit on it. A pour on no net is not drawn. Keep-outs are not drawn: they
  are already cut out of the fill.

| `.GTS` / `.GBS` | solder mask (`FilePolarity,Negative`: drawn = opening) | the side's surface-mount pads (an opposite-side pad is on the side opposite its part's) and fingers, every plated through-hole pad, every bare hole, each grown by the Mask expansion design rule. Vias are tented unless opened (Via ▸ Tented on Top / Bottom): an open side gets an opening the via's metal grown by the Mask expansion |
| `.GTP` / `.GBP` | paste | the side's surface-mount part pads (not fingers) |
| `.GTO` / `.GBO` | silkscreen | silkscreen texts, references (1 mm, centred on the body, or where the part's reference was moved to), pin-1 dots (0.4 mm), body outlines and cathode bands (0.15 mm pen), each only when its layer is Silkscreen |
| `.GKO` | outline (`Profile,NP`) | the board outline, 0.1 mm pen, then each [cutout](file-format.md#cutouts) as a closed loop of its own (a fab routes the profile; a cutout is not in the drill files) |
| `-PTH.drl` | plated drill (Excellon) | through-hole pads, vias, plated holes, header pads |
| `-NPTH.drl` | non-plated drill (Excellon), only when the board has any | bare holes, a standard's mounting holes, non-plated footprint holes |
| `-CPL.csv` | pick and place (CSV), only when the board has surface-mount parts | see [Pick and place](#pick-and-place) |
| `-job.gbrjob` | job file (JSON) | the layer stack, thickness, finish, design rules and each file's role: see [Job file](#job-file) |

- **Attributes on copper** (Gerber X2, spec § 5.6): every aperture and region of a copper file has an
  `.AperFunction`, and every object a net `.N` (and a part's pad its pin `.P`). Each file also has
  `.ProjectId` (the file base name, a GUID made from it, revision `1`) and `.Part,Single`, and the outline's pen
  is `Profile`. There is no creation date, so one document always gives the same bytes.

  | Object | `.AperFunction` | `.N` | `.P` |
  |---|---|---|---|
  | surface-mount part pad | `SMDPad,CuDef` | its net | `<reference>,<pin>` |
  | unnumbered surface-mount pad (a centre pad) | `HeatsinkPad` | its net, else none | `<reference>,` and an empty pin |
  | plated through-hole part pad, header pad | `ComponentPad` | its net | `<reference>,<pin>` (header: `HEADER1`) |
  | card edge finger | `ConnectorPad` | its net | `EDGE1,<finger name>` |
  | land of a plated board hole | `OtherPad,PlatedHole` | its net, else none | — |
  | via | `ViaPad` | its net | — |
  | trace, a Plane's copper, a pour's fill | `Conductor` | its net (a Plane's or pour's net) | — |
  | a Plane's clearances, a pour's holes (clear polarity) | none | none | — |

  A net with no pad, or none, is `%TO.N,*%` (the spec's "no net"). Net names are the KiCad export's
  (`Net-(U1-A0)`, `unconnected-(U1-A0-Pad1)`, a rail's name, `/LABEL`), and `,`, `*`, `%` and `\` in one are
  written `\u002C` and so on (spec § 3.4.3). Mask, paste and silkscreen files carry no object attributes.
- Everything is seen from the top, as on the PCB page: bottom silkscreen
  reads mirrored. Board y grows down; Gerber y grows up, so a file's y is
  the board's top edge minus the board y, and the board sits at y ≥ 0.
- A 1-layer board has only the Top files and the outline.
- Circles and plain rectangles are aperture flashes; ovals (stadiums),
  rounded and chamfered rectangles are regions of the same outline the canvas fills
  (`PCBCanvasView.outline(of:)`), curves cut into steps of at most 0.05 mm.
  A grown shape (mask expansion, Plane clearance) is the shape plus its
  outline stroked with a round pen twice the growth wide.
- Text is real glyph outlines, from the same fonts the canvas uses. Each
  glyph's counters (the hole in an O) are drawn in clear polarity, so a
  silkscreen file draws its text before its lines and dots.
- Not included yet: the Document layer (never printed).

### Drill files

Excellon (`DrillExport`), as KiCad writes it: `M48` header with
`FMAT,2`, `METRIC` and the X2 `; #@! TF.FileFunction,Plated,1,<layers>,PTH`
(or `NonPlated…NPTH`) comment, one tool per hole diameter (to 1 µm,
smallest first), then `%`, `G90`, `G05`, each tool's hits and `M30`.
Coordinates are absolute decimal mm in the Gerber files' frame (y up from
the board's bottom edge), so they line up with the copper. A slot is a
routed `G85` slot from one end centre to the other, its width the tool.

Each tool is preceded by its function, `; #@! TA.AperFunction,Plated,PTH,ViaDrill` (or `NonPlated,NPTH,…`):
`ViaDrill` for a via, `ComponentDrill` for a part's hole and a header's, `MechanicalDrill` for the board's own
(a standard's mounting hole, a placed hole, plated or not). Holes of one diameter but two functions are two tools
(spec § 5.6.10: a fab compensates each function's tools separately); tools are ordered by diameter, then via,
component, mechanical.

### Pick and place

`<name>-CPL.csv` (`PickAndPlaceExport`) is the placement list JLCPCB and PCBWay assembly take, in the zip before the job file
(which does not list it: it is not a Gerber file). It is written only when the board has surface-mount parts.

`Designator,Mid X,Mid Y,Layer,Rotation`, CRLF line ends, one row per surface-mount part placed on the board (a part with a
footprint whose pads are all undrilled; through-hole parts, card fingers and holes are left out, as fabs' machines place
SMD only), in natural reference order.

- **Mid X / Mid Y**: the footprint's centre (the body centre) in mm, in the Gerber files' frame: x as on the board, y up
  from the board's bottom edge, so it lines up with the copper and the drill files.
- **Layer**: `Top` or `Bottom`.
- **Rotation**: whole degrees **counter-clockwise** seen from the top, the footprint's angle in the KiCad export
  (the Schematical turn is clockwise, so `(360 − turn) % 360`). A fab's own library has its own zero direction per
  package, and JLCPCB treats bottom-side parts differently from KiCad, so some parts need a correction there:
  check the placement preview on the fab's order page. Nothing is corrected here, the numbers are the board's.

### Job file

`<name>-job.gbrjob` is JSON in Ucamco's Gerber job-file layout (valid against its published schema, revision
2023.06), written last in the zip. It has no date and sorted keys, so one document always gives the same bytes.

| Key | What |
|---|---|
| `Header.GenerationSoftware` | `jgz`, `Schematical`, the app version |
| `GeneralSpecs` | `ProjectId` (the same name and GUID as the files' `.ProjectId`, revision `1`), `Size` (the outline's box, mm), `LayerNumber`, `BoardThickness` (the stackup's total: copper, dielectrics and 0.01 mm of mask on each side), `Finish` (`HAL SnPb`, `HAL lead-free`, `ENIG` or `OSP`) |
| `DesignRules` | `Outer` (and `Inner` above 2 layers): the board's min clearance as `PadToPad`, `PadToTrack`, `TrackToTrack`, `TrackToRegion`, `RegionToRegion`; `MinLineWidth` (min trace width), `MinRing` (min annular ring), `MinClearanceToProfile` (min copper to edge) |
| `FilesAttributes` | every other file in the zip: `Path`, and its `FileFunction` (the string in the file's own `.FileFunction`) and `FilePolarity` |
| `MaterialStackup` | top to bottom: silkscreen, paste, solder mask (with the board colour), then each copper layer and the dielectric below it (`FR4`, thickness, `DielectricConstant`), then the bottom mask, paste and silkscreen |

## Bill of materials

The Bill of Materials window (⇧⌘B) has two exports for its checked lines: **Export CSV…** (the whole list: Quantity,
References, Part Number, Package, Value, Color, Description) and **Export JLCPCB / PCBWay BOM…**
(`BillOfMaterials.jlcpcbCSV`), the assembly layout:

`"Comment","Designator","Footprint","Quantity","Manufacturer","Manufacturer Part Number","LCSC Part #"`

- Every field is quoted; designators are joined with `,` and no space (`"R1,R2,R3"`); a line of more than 200
  designators is split, JLCPCB's limit per row.
- Only **surface-mount** lines are written, the same parts as the [pick-and-place file](#pick-and-place), so the two
  files name the same designators (JLCPCB reports any that differ). The button is off when no checked line is surface
  mount.
- `Comment` is a resistor's or capacitor's value (`4.7kΩ`, `100 nF`), else the part number (an LED's color after it).
  `Footprint` is the package. Manufacturer and its part number are empty for Generic parts.
- `LCSC Part #` is empty: the catalog has no LCSC numbers, so match the parts on the fab's site.

## PDF and print

**File > Print…** (⌘P) and **File > Export > PDF…** open one options sheet over the window, with a live
preview of the PDF; its button is **Print…** (the system print panel, for the same PDF) or **Save PDF…** (the save
panel, `<window name>.pdf`). Printing goes through the document: after the sheet, Print… asks the document to
print (`NSDocument.print`), and the document's `printOperation(withSettings:)` makes the operation from
`PrintExport`'s PDF with the sheet's choices. The system's own print action (with no sheet) reaches the same
method and prints every page with the saved paper and marks. Simulation's Run / Stop is ⌥⌘P. The paper, Mirror, Negative and Registration marks
are remembered between prints (`PrintOptions.saved`); the page choices are not (a board's layers differ).

**Paper** is Letter, Legal, Tabloid, A4 or A3 (Letter where the system measures in inches, else A4). Every page is
laid out for it, turned to the orientation that needs fewer sheets, with a 10 mm margin and a 9 mm footer
(`PrintExport.margin`, `footer`). Pages the print panel is asked to print are not scaled (`.pageScaleNone`): pick
the same paper there.

| Page | What is on it | On by default |
|---|---|---|
| Schematic | the schematic as the black line-art image (vector, with its title block when shown), scaled down to fit one sheet, never up past 1×; the footer says the scale | yes |
| `<Layer>` copper, one per layer | the layer's artwork as in its Gerber file: planes with their clearances, traces, pads, vias, black on white, seen from the layer's own side (a Bottom page is mirrored against the PCB page). A dot of at most 0.4 mm (`drillDot`) at each hole's centre, in the paper colour, for the bit | yes |
| `<Side>` parts (footprint check) | pads (light gray, outlined), each drill hole at its true size outlined, and that side's silkscreen: lay a real part on it | yes |
| `<Side>` silkscreen | the side's silkscreen alone | no |
| Board outline and drills | the outline and every cutout's loop (0.2 mm) and every hole at its true size with a centre cross | yes |

- **1:1**: a board page is in mm (72 / 25.4 points per mm), not scaled to fit. Its footer says "1:1, print at 100 %
  (Actual Size)" and draws a 50 mm ruler with ticks every 10 mm (`rulerLength`) to check the printer's scale.
- **Tiles**: a board that does not fit one sheet (with 2 mm round it, or the marks' room) is cut into evenly spaced
  tiles that overlap by at least 10 mm (`overlap`), in rows from the top left; the footer says "tile 2 × 1 of 3 × 2".
  The orientation with fewer tiles wins; a tie goes to the one that suits the board's shape.
- **Mirror** flips the copper pages left to right, for toner transfer or film laid face down. The other pages do not
  flip. **Negative** paints the board (grown 1.5 mm, `inkMargin`) black and draws the copper paper-white, for
  positive-resist boards. **Registration marks** are four 3 mm crosses 4 mm outside the board's corners
  (`markOffset`), the same on every copper page.
- The artwork is `GerberExport.drawCopper` / `drawSilkscreen`, recorded by `GerberWriter` (`records`, `Mark`), so a
  printout is what the fab gets. Clear polarity (plane clearances, letter counters) is drawn in the paper colour.
- A board with no part, trace or via still has its outline page and copper pages; a schematic page appears only when
  the schematic has something.
- An export is not a save: the window's file and its unsaved-changes state do not change.

**Agents**: `export_pdf` (path in the shared folder, `paper`, `pages` of schematic / copper / parts / silkscreen /
board, `mirror`, `negative`, `marks`), see [agent-control.md](agent-control.md).

## KiCad

**File › Export › KiCad Project…** writes the schematic and its
board as a KiCad 8 project (KiCad 8 and later open it). It asks for a
**folder** and writes three files into it, named after the schematic:
`<name>.kicad_pro`, `<name>.kicad_sch` and `<name>.kicad_pcb`
(`KiCadExport`). A folder, not a file name, because the app is sandboxed: a
save panel lets it write only the one file named in it, and the other two
were refused. When the folder already has any of the three, the app asks
before replacing them. Like any export it is not a save: the document's file and
its Edited state do not change.

**Schematic** (`KiCadSchematicWriter`). One sheet, 0.127 mm per canvas unit, so
the app's 10-unit grid is KiCad's 1.27 mm grid and every wire end lands on a pin
end.

- Each part is a generated symbol in the file's own `Schematical` library: a
  rectangle with the pins on the sides the app draws them (a flipped or turned part is its
  own symbol; a two-terminal part's rectangle is just its pins' span). Pin names
  keep their active-low bars (`~{CE}`), and a pin's type is the electrical rules
  check's (plan-63; `docs/backend.md` § ERC pin types): `passive` for a resistor,
  connector or transistor, `tri_state` for a 3-state output, `open_collector`,
  `power_out` for a regulator's output, `bidirectional`, `input`, `output`, and
  `power_in` for power and ground pins, so KiCad's own ERC on the export sees what
  the app's sees (the pin's role when the document has no part library). Value is the part number (a resistor's or generic
  capacitor's value), Footprint is `Schematical:<footprint id>`, Datasheet is the
  datasheet URL.
- VCC and GND symbols are power symbols named for the rail (`5V`, `3V3`, `GND`),
  so KiCad joins every one of a voltage, as the app does; each rail also gets a
  `PWR_FLAG`. A pull resistor gets a second pin, on its far side, joined to a power
  symbol of its rail.
- Wires (one per straight run), junctions where 3 or more runs meet, net labels
  (local labels; the flag shape is not kept), No Connects and text blocks (framed
  ones with a box). A Power Flag net label (plan-63-A) becomes a `PWR_FLAG` power
  symbol (`#FLG<n>`, pin type `power_out`) at its point; EasyEDA Pro has no such
  symbol, so the flag is left out there. A shown title block (Window ▸ Document Info) is lines and text the same way,
  in the canvas's layout; and KiCad's own `(title_block …)` always carries the Document Info: its
  title (the file's name when it has none), the updated date, revision, the author as the company
  and the description as comment 1. Buses are drawn as KiCad buses with each port as a box, and every
  wired port pin carries a net label of its bus wire's name, so the nets are the
  same as the app's (linked buses share the label; unrelated wires of one name get
  `D0`, `D0_2`).
- A card board's connector (`EDGE1`, `HEADER1`) is a symbol beside the drawing,
  one pin per connector pin, each on its net by a label or a power symbol.

**Parts drawn as units** (plan-67). A part in Gate view (a gate IC drawn as
separate gates) is written as a real KiCad multi-unit symbol: one library
symbol, named after the part plus `_units` and each unit's flip and turn
(`SN74HC00N_units_An0_Bf90_Cn0_Dn0_Pn0`), with one KiCad unit per drawn piece
(`…_1_1`, `…_2_1`, …; no common `_0_1` graphics), each holding that piece's
body rectangle and its pins. The sheet gets one instance per piece at the
piece's place, `(unit 1)`, `(unit 2)`, … in the order of the part's units, all
with the part's one reference, so KiCad shows `U1A`, `U1B`, … and still sees
one part with one footprint. The first instance carries the part's own uuid,
which the board's footprint points at. The block of leftover pins (`U1-P`
here: power, ground, NC) is the last unit, so a quad gate's power unit reads
`U1E` in KiCad. As for every part, each unit is a rectangle with pins: the
gate outlines are not written. Checked with KiCad 10.0.8 on the app's own
`ExportSamples.split()` (a quad NAND with one gate moved, flipped and turned):
`kicad-cli sch export netlist` lists one component `U1` (SN74HC00N, footprint
`Schematical:PDIP-14`) with units A–E holding pins 1 2 3 / 4 5 6 / 8 9 10 /
11 12 13 / 7 14, and its pins on the app's nets (pin 3 with pin 4 and the
LED, 14 on `5V`, 7 on `GND`, the ten unused pins each on their own
`unconnected-(…)` net); `kicad-cli sch erc` reports none of `missing_unit`,
`extra_units`, `different_unit_net` or `different_unit_footprint`. The outputs
are in `planning/plan-67-data/kicad/`.

**Board** (`KiCadBoardWriter`). Millimetres as on the PCB page, moved 30 mm inside
the page. Footprints are written with the part's reference, value, body outline,
pin-1 dot and diode band on the layers the part's marks name, pads in the footprint's
own frame (a turn of `r` degrees clockwise is KiCad's angle `-r`, a Bottom footprint
has its x mirrored and its pads on `B.*`; an opposite-side pad (`"opposite": true`, plan-54-E) is on the other copper layer, its `(layers …)`
those of the other side), traces, vias (each with its two tenting flags, `(tenting (front yes) (back no))`; `front` is Top), Plane layers as unfilled
zones (press B in KiCad to fill), each copper pour on a net as an unfilled zone over its rectangle (`connect_pads` with no keyword is thermal relief,
`connect_pads yes` solid; `thermal_gap`, `thermal_bridge_width` and `min_thickness` are the pour's gap, spoke width and minimum width; KiCad fills it with Fill All Zones (B),
so its fill differs from the app's in detail), each keep-out as a rule area on its layers that blocks only copper pour (`(keepout (tracks allowed) (vias allowed) (pads allowed)
(copperpour not_allowed) (footprints allowed))`; a keep-out on no layer is not written), the outline on `Edge.Cuts` and each cutout's loop beside it (`gr_line`s, uuids `cutout/<n>/<i>`; the Plane zone's polygon stays the outline, and KiCad's fill keeps out of a cutout with its clearance: `planning/plan-65-data/probe_zone_edge_cuts.py`), board texts, the card's
fingers or header pads, and holes. A 1-layer board has the two copper layers KiCad
needs. Pads carry the net of their pin, named as KiCad names it: a rail's name, a net
label as `/NAME` (a `/` in it as `{slash}`), else `Net-(U1-A0)` after the
alphabetically first pin, `unconnected-(U1-A0-Pad1)` for a pin on nothing. So the
board and the schematic agree (KiCad's Update PCB from Schematic finds no difference),
and each footprint's path is its part's uuid.

**Pours** were checked with KiCad 10.0.8: the app's own `pours.kicad_pcb` (two pours and a keep-out, `ExportSamples.pours()`) loads in `kicad-cli pcb drc` (`Found 0 unconnected items`), `pcbnew` reads
back the first pour as thermal relief with gap 0.6, spoke 0.4, minimum width 0.25 and clearance 0.3 mm, the second as solid, the rule area on `F.Cu` blocking only copper pour, and after
`ZONE_FILLER.Fill` the Top fill has no copper inside the keep-out (`planning/plan-52-data/probe_kicad_zones.py`). The Gerber's regions read back in gerbonara 1.6.3 as one dark region and the
pad rings as clear regions (`planning/plan-52-data/probe_gerber_regions.py`).

**Stackup.** The board's stackup is written to `(setup (stackup …))` as KiCad lays it out: silkscreen, paste and
mask (with the board colour) around each copper layer and its dielectric (`core` or `prepreg`, `FR4`, thickness,
`epsilon_r`, loss tangent 0.02), then `copper_finish` (`HAL SnPb`, `HAL lead-free`, `ENIG`, `OSP`), and
`(general (thickness …))` is the sum KiCad takes (copper, dielectrics, 0.01 mm of mask on each side: 1.6, 1.6062,
1.566 for the standard 2-, 4-, 6-layer boards). A 1-layer board gets the bottom copper KiCad's layer set needs,
0.035 mm taken out of its substrate: 1.61 mm in KiCad.

**Not exported**: net classes, per-net layer rules and groups, simulation state,
the Document layer's contents on the board other than texts, and a real symbol or
footprint library (the `Schematical` library is only in the files, so the project
file turns off KiCad's missing-library checks). Design rules go to the project's
design settings.

**Checking it.** `KiCadExportTests` checks the files' shape, that a footprint's pads
land where the app puts them at every turn and side, and the folder chooser and the three files it leads to. The rest was
checked with KiCad 10's `kicad-cli`: `sch export netlist` gives the same pin groups
and the same net names as the pads on the board (on `AFU`, `LFU` and `RegFile`), `pcb
drc` finds no schematic parity difference, a board of DIP and SOIC parts at every
turn on both sides has the same hole positions as the app's drill file, and its
pad outlines in `pcb export svg` are the app's. A board with a trace and four vias (one of each Top / Bottom
tenting) loads in KiCad 10 (`ExportSamples.vias()`), and `pcbnew` reads each via's two tenting modes back as
written (a track's or via's net is `(net <code>)`, not code and name, which KiCad refuses).

## EasyEDA Pro

**File › Export › EasyEDA Pro Project…** asks where to save and writes the schematic and its
board as one `.epro` file (`EasyEDAProExport`), which EasyEDA Pro takes with **File ▸
Import ▸ EasyEDA Professional**. Like the KiCad project it is an export: the
document's file and its Edited state do not change.

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
text blocks and a shown title block are `TEXT` (framed ones with a box), and a card board's connector is a
symbol beside the drawing with a named stub or a net flag on each pin.

**Parts drawn as units** (plan-67). A part in Gate view stays one `COMPONENT`
with one Designator. Its generated symbol holds one `RECT` per drawn piece and
every pin where the app draws it, measured from the corner of the box around
the pieces, so every wire still ends on its pin and the board link is
unchanged. The pieces cannot be moved apart in EasyEDA: there is no
multi-part symbol. The symbol's key includes each unit's place, flip and
turn, so two differently arranged parts never share one.

**Board** (`EasyEDAProBoardWriter`): `LAYER`s (a 1-layer board gets Top and Bottom, EasyEDA's
minimum), the design rules' track width, a `NET` for every net a pad carries, the outline
(`POLY` on the outline layer, then one more `POLY` on the same layer for each cutout), a `POUR` for each inner Plane on a net and for each copper pour on a net (over its rectangle; keep-outs are not exported, the format's keep-out record can't be checked here), a `LINE` per trace
segment, `VIA`s, a `PAD` for each card finger and hole, `STRING` for board texts, and each
part as a `COMPONENT` with `Designator`, `Device` and `Footprint` attributes and a
`PAD_NET` per pad on a net. A net of one pin is left without a net. A footprint file is the footprint as seen
from its own side: its pads are on layer 1, an opposite-side pad (plan-54-E) on layer 2.

**Not exported**: net classes and layer rules, No Connect flags, the schematic's per-part
silkscreen marks (a footprint's body is on the assembly layer and its pin-1 dot on Top
silkscreen whatever the part's marks say), cathode bands, a chamfered pad's chamfer, and
3D models.

**Cutouts** (plan-65) were checked with KiCad 10.0.8: the app's own `cutouts` board (a circle, a slot and a triangle on a 4-layer board with two Planes; `ExportSamples.cutouts()`,
printed with `TEST_RUNNER_KICAD_OUT=1` by `CutoutExportTests.testPrintCutoutSamplesWhenAsked`) has the same 3 copper-edge and 2 silkscreen-edge findings from `kicad-cli pcb drc` as the same
board without cutouts (the sample's own), `kicad-cli pcb export dxf -l Edge.Cuts` gives 73 `LINE`s (4 for the outline, 69 for the three loops: the file's `gr_line` count) and
KiCad's EasyEDA Pro importer reads 73 `Edge.Cuts` segments from the `.epro`.

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

## Import

**File › Import › DXF…** reads a 2D drawing and makes the board's edge and cutouts from its closed loops (plan-65). It asks for a `.dxf` file, then shows a sheet with a
preview of the loops, which layers to use, the drawing's units (from `$INSUNITS`; the board's units when the file has none), and one of two modes:

- **Board outline and cutouts**: the largest outer loop becomes the board's edge (a free-form Custom board whose width and height are its bounds), the loops inside it become cutouts, and
  the board's old cutouts are replaced. The drawing is flipped (a DXF's y goes up) and moved so the outline's top-left is (0, 0). Parts and traces stay where they are.
- **Cutouts only**: every outer loop is added as a cutout of the board the document has. The drawing's origin lands at the X and Y the sheet shows (mm from the board's
  top-left; by default the board's bottom-left corner, so a drawing with its origin at the board's corner lines up).

It is one undo step. What it reads: ASCII DXF with `LINE`, `ARC`, `CIRCLE`, `LWPOLYLINE`, `POLYLINE`, `ELLIPSE` and `SPLINE` entities; curves are cut into segments no more than 0.01 mm from the true
curve, and pieces whose ends are within 0.01 mm are joined into loops. Everything else (`TEXT`, `HATCH`, `DIMENSION`, `INSERT` blocks, 3D) is left out and the sheet says how many; so are
open chains, loops that cross themselves or another loop, loops outside the outline and loops inside a cutout. A binary DXF is refused. It was checked with files written by ezdxf (areas and
bounding boxes match ezdxf's own) and by KiCad (`kicad-cli pcb export dxf` of the app's own board reads back as the same four loops): `planning/plan-65-data/`.

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

A part drawn as its units (plan-67, Gate view) is drawn in every picture export as it is on the canvas: each
gate with its own symbol and label (`U1-A`, `U1-B`, …) and the block of leftover pins (`U1-P`); the export
scene paints `PlacedChip.pieces`.

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
| `PrintPaper`, `PrintPageKind`, `PrintOptions`, `PrintPage`, `BoardArtwork`, `PrintExport` (pages, tiles, the PDF, the print operation) | `schematical/schematical/Document/PrintExport.swift` |
| `PrintDialog` (the options sheet with its PDF preview) | `schematical/schematical/Document/PrintDialog.swift` |
| `SchematicExportMenu` (the submenu; `SchematicFileCommands` adds it to File, with Print…) | `schematical/schematical/Document/SchematicExportCommands.swift` |
| `exportImage()`, `writeImage(_:to:)`, `printPages()`, `exportPDF()`, `writePDF(_:to:)`, `exportGerber()`, `writeGerber(to:)`, `exportKiCadProject()`, `writeKiCadProject(into:)`, `writeKiCad(to:)`, `exportEasyEDAProProject()`, `writeEasyEDAPro(to:)`, `printOperation(_:jobTitle:)` (`SchematicExporter`) | `schematical/schematical/Document/SchematicExporter.swift` |
| `printOperation(withSettings:)` (the document's print) | `schematical/schematical/Document/SchematicNSDocument.swift` |
| `SaveFormat` (each written format's extension and content type) | `schematical/schematical/Document/SaveFormat.swift` |
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
