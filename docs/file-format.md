# Schematical file format

Schematical saves a schematic as a UTF-8 JSON file with the extension
`.schematical`. This document describes version 5, which is version 4 plus
the custom parts a schematic places ([Custom parts](#custom-parts)); a
schematic with no custom part is still written as version 4. Version 1, 2
and 3 files open too (see [Older versions](#older-versions)).

Code: `schematical/schematical/Document/SchematicFile.swift` (the format) and
`SchematicFileController.swift` (Save, Save As, Open).

## Design rules

- **Parts are stored by part number only.** Pins, pin roles, symbol and layout
  come from the part library (`schematical/schematical/Parts/*.json`) when the
  file is opened. Files stay small, and fixes to a part's model apply to
  existing files. The one exception is a custom part (plan-32): it is in no
  bundled library, so its whole definition is embedded, once
  ([Custom parts](#custom-parts)).
- **Connections are explicit.** A wire end names a part and pin designator, or a
  junction. Connectivity never depends on two coordinates touching.
- **IDs are stable.** Every part, junction, wire, bus, bus line and bus wire
  has a UUID that is kept across save and open.
- **The selection, zoom, scroll position and simulation state are not saved.**

## Coordinates

All positions are in canvas units. The grid spacing is 10 units, and y grows
downward. Part origins are snapped to the grid when a file is opened.

PCB placements (`pcb`, below) and the `board` object are in board
millimetres instead, y down. A placement's `x`/`y` is always the footprint's
centre (the middle of its body). Each footprint also has an **origin point**
(`pcb.origin`): a corner or middle of its bounding box (body plus pads, as
rotated), or the centre of pad 1, which new footprints use. The app snaps
the origin point to the board's component grid (`board.grid`, 0.635 mm =
25 mil by default) when a footprint is moved or rotated, and rotates
about it. Changing the origin does not move the footprint, so a saved origin
point can be off the grid; opening keeps such a placement as saved. A
placement with no `origin` (files from before it existed) is shifted on open
so the top-left of its bounding box sits on the grid. Board texts are snapped
to the grid on open.

## Top level

```json
{
  "format": "schematical",
  "version": 4,
  "nextReference": 4,
  "parts": [ ... ],
  "junctions": [ ... ],
  "wires": [ ... ],
  "buses": [ ... ],
  "board": { ... },
  "notes": [ ... ],
  "labels": [ ... ]
}
```

| Field           | Type    | Meaning                                                              |
|-----------------|---------|----------------------------------------------------------------------|
| `format`        | string  | Always `"schematical"`. Other values are rejected.                    |
| `version`       | integer | Format version. Files newer than the app supports are rejected.      |
| `nextReference` | integer | Lowest number tried for the next placed part's reference (`U` + number); the first `U<n>` no part uses is taken. |
| `parts`         | array   | Placed parts, in drawing order (later parts draw on top).            |
| `junctions`     | array   | Points where wires branch.                                           |
| `wires`         | array   | Wires between pins and junctions.                                    |
| `buses`         | array   | Buses ([Buses](#buses)). Always written from version 3 on, empty when there are none; version 1 and 2 files have none. |
| `board`         | object  | Optional. The PCB's settings and texts ([Board](#board)). Written by every build that has it; a file without it opens with the defaults. |
| `notes`         | array   | Text blocks on the schematic, in drawing order ([Notes](#notes)). Always written, `[]` when there are none; missing means none. Older builds ignore it. |
| `labels`        | array   | Net labels on the schematic, in drawing order ([Net labels](#net-labels)). Written only when there are some; missing means none. Older builds ignore it, but refuse a wire that ends on a label. |
| `customParts`   | array   | Version 5. The custom parts the schematic places ([Custom parts](#custom-parts)). Written only when there are some, and then `version` is 5; missing means none. |

On open, `nextReference` is raised if needed so that it is higher than every
existing `U<n>` reference (references with other prefixes, such as `R3`, do
not count).

The app writes keys in sorted order, so saving the same schematic twice gives
the same bytes. Readers must not depend on key order.

## Parts

```json
{
  "id": "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C10",
  "reference": "U1",
  "partNumber": "SN74HC00N",
  "x": 200,
  "y": 120,
  "flipped": false
}
```

Every part with a footprint (all but the supplies) also stores where its
footprint sits on the PCB:

```json
{ "id": "...", "reference": "U2", "partNumber": "GENERIC-LED",
  "color": "Amber", "footprint": "LED-0603",
  "x": 200, "y": 100, "flipped": false,
  "pcb": { "x": 5.08, "y": 0, "rotation": 90, "origin": "topLeft",
           "outline": "document", "pin1": "silkscreen", "reference": "silkscreen" } }
```

A DIP switch also stores its positions:

```json
{ "id": "...", "reference": "U2", "partNumber": "TE-GDS0404",
  "x": 80, "y": 120, "flipped": false, "switches": [true, false, false, true] }
```

An EEPROM can name the file loaded into it when the simulation starts:

```json
{ "id": "...", "reference": "U4", "partNumber": "AT28C256-15PU",
  "x": 300, "y": 120, "flipped": false,
  "pcb": { "x": 20.32, "y": 0, "rotation": 0 },
  "memoryFile": { "path": "roms/boot.bin", "bookmark": "Ym9va21hcms..." } }
```

| Field        | Type    | Meaning                                                                 |
|--------------|---------|-------------------------------------------------------------------------|
| `id`         | UUID    | Unique within the file.                                                 |
| `reference`  | string  | Label shown on the PCB and in memory window titles, such as `U1`: 1 to 8 letters then a number of 1 or more. New parts get `U<n>`; new pull resistors (pull-ups and pull-downs) get the lowest `R<n>` no part uses, and new capacitors (every `capacitor` part) the lowest `C<n>` no part uses, both without advancing `nextReference`. Pull resistors in older files keep their saved reference (often `U<n>`). The PCB sidebar renames them (letters uppercased; a reference another part has is refused). |
| `partNumber` | string  | A part name from the part library (`schematical/schematical/Parts/*.json`): a logic part, memory (SRAM, EEPROM), switch, LED, 7-segment display, character LCD module, oscillator, processor, capacitor (a ceramic, electrolytic or tantalum part, or `GENERIC-CAPACITOR`), generic pin header (one part per size, `GENERIC-HEADER-1X01` … `GENERIC-HEADER-2X40`: rows, then pins per row; the sidebar changes the size by changing this part number), or a supply: `5V`, `3V3`, `2V5`, `1V8`, `1V2`, `12V` or `GND`. `VCC` (a supply removed from the library) opens as `5V`, and is saved as `5V`. The 55 LED parts `LED-<COLOR>-<PACKAGE>` (such as `LED-GREEN-RADIAL`, `LED-AMBER-0603`) that `GENERIC-LED` replaced open as `GENERIC-LED` with that part's color and footprint, in a file of any version, and are saved as `GENERIC-LED` (see [Old LED part numbers](#old-led-part-numbers)). |
| `x`, `y`     | number  | Top-left corner of the part body.                                       |
| `flipped`    | boolean | Pins on the opposite sides. Ignored for the supplies.                   |
| `switches`   | array of booleans | DIP switches only, optional: whether each position is ON, position 1 first. Missing positions are OFF; extra entries are ignored. Left out for other parts. A tactile switch never saves it (it starts released) and ignores it when read. |
| `pcb`        | object  | Optional. The footprint on the PCB: `x`, `y` (numbers, mm) are the footprint's centre, `rotation` (integer) is 0, 90, 180 or 270 degrees clockwise. `origin` (string; a missing one is `"topLeft"`, as files from before origins were; parts placed in the app start as `"pad1Center"`) is the footprint's origin point (see [Coordinates](#coordinates)): `"center"`, `"topLeft"`, `"centerLeft"`, `"bottomLeft"` (of the bounding box as rotated) or `"pad1Center"` (the centre of the pad of the part's first pin). `outline` (the body outline, default `"document"`), `pin1` (the pin-1 dot, default `"silkscreen"`) and `reference` (the reference text, default `"silkscreen"`) say where each mark is drawn: `"hidden"`, `"document"` or `"silkscreen"`. The four strings are always written; a missing one takes its default. `style` (string, default `"module"`): `"module"` (Full Module) or `"connector"` (Connector Only), which of the part's two footprints is on the board. Written only for parts with a connector footprint (the character LCD modules); ignored on other parts. `side` (string, default `"top"`): `"top"` or `"bottom"`, the side of the board the footprint is on. A bottom footprint is drawn mirrored left to right about its body centre before it is turned, so `rotation` stays clockwise as seen from above; `x`, `y` stay the body centre. Always written; not allowed on a 1-layer board. Older builds ignore it and show the part unmirrored on top. `locked` (boolean, default `false`): the sidebar's Lock Position; a locked footprint can't be dragged, moved by X and Y, or rotated. Written only when `true`; older builds ignore it. Written for every part with a footprint, including the pull resistors; left out for VCC and GND, and ignored on a part without a footprint. A part with a footprint and no `pcb` (older files) is placed automatically when the file is opened: after the stored ones, in file order, each to the right of everything already placed. |
| `color`      | string  | Version 4. LEDs only (`GENERIC-LED`): one of the part's colors, `"Red"`, `"Green"`, `"Blue"`, `"Amber"` or `"White"`. Used by the symbol's glow in play mode. Always written for an LED; missing means the part's default (`Red`). Ignored on other parts. |
| `footprint`  | string  | Version 4. Parts with footprint choices only (`GENERIC-LED`, the pull resistors `PULLUP-…`/`PULLDOWN` and the headers `GENERIC-HEADER-…`): the id of the chosen footprint, one of the part's `footprintChoices` (`docs/parts-format.md`), such as `"LED-0805"`, `"RES-0805"` or `"PINHDR-1X08-RA"`. The PCB uses it. Always written for those parts; missing means the part's default footprint (`LED-RADIAL-5MM` for the LED, `RES-RADIAL-2.54` for a pull resistor, the `-V` (Straight) footprint for a header). Ignored on other parts. |
| `ohms`       | number  | Version 4. Pull resistors only (`PULLUP-…`, `PULLDOWN`): the resistance in ohms, more than 0 and at most 1e9 (1 GΩ), such as `4700`. Shown on the symbol and in the Bill of Materials; the simulation ignores it. Always written for pull resistors; missing means `1000`. Ignored on other parts. An older version-4 build ignores it. |
| `farads`     | number  | `GENERIC-CAPACITOR` only: its capacitance in farads, at least 1e-13 (0.1 pF) and at most 1, such as `4.7e-9`. Shown on the symbol, in the Details sidebar and in the Bill of Materials; the simulation ignores it. Always written for it; missing means `1e-7` (0.1 µF). Ignored on other parts (a real capacitor's value is its part's). An optional field, so the version stays 4. |
| `gender`     | string  | Headers only (`GENERIC-HEADER-…`): one of the part's genders, `"Male"` or `"Female"`. The symbol draws pins or sockets; the footprint doesn't change. Always written for a header; missing means the part's default (`Male`). Ignored on other parts. An optional field, so the version stays 4 (a file with headers names parts an older build doesn't have, which it refuses anyway). |
| `partUUID`   | string  | Version 5. Custom parts only: the part's `uuid`, one of the file's `customParts`. The placed part resolves by it, not by `partNumber`; `partNumber` is the name the part had when saved (with its suffix, such as `SN74HC00N.2`), so a reader that ignores custom parts still sees a name. Written exactly for custom parts. |
| `memoryFile` | object  | EEPROMs only, optional: the file (raw binary, or Intel HEX for `.hex`/`.ihx`/`.ihex`) loaded into the chip at every start; without it the chip starts erased (every byte FF). `path` (string) is the file's path relative to the schematic's folder (`../roms/boot.bin`), or absolute for a schematic that had never been saved. `bookmark` (base64 string, optional) is an app-scoped security bookmark; on the Mac that wrote it, it is tried first, so the sandboxed app can read the file after a restart, and `path` is the fallback. Ignored on other parts. |

## Board

```json
"board": { "width": 100, "height": 80, "layers": 2, "color": "green", "units": "mm",
  "layerStyles": [ { "layer": "top", "color": "#C83737", "type": "component" },
                   { "layer": "bottom", "color": "#3A6FD8", "type": "component" } ],
  "texts": [ { "id": "...", "text": "REV A", "x": 5.08, "y": 2.54,
               "size": 50, "layer": "silkscreen", "rotation": 0 } ] }
```

| Field    | Type    | Meaning                                                                |
|----------|---------|------------------------------------------------------------------------|
| `type`   | string  | Optional. `"custom"` (default; the plain rectangle), `"customCard"` (a card whose edge is built from `card`, below), or the `id` of a standard board in `boards.json` (see [boards-format.md](boards-format.md)): add-in cards (`"isa8"`, `"isa16"`, `"pci"`, `"pcie"`), modules (`"m2"`, `"miniPCIe"`, `"sodimm"`, `"dimm"`), cartridges (`"nes"`, `"atari2600"`, `"atari5200"`, `"atari7800"`, `"atari8bit"`, `"atariST"`, `"jaguar"`, `"lynx"`, `"coco"`) and boards on a header (`"piHat"`, `"arduinoShield"`, `"featherWing"`, `"pmod"`, `"mikroBus"`, `"beagleCape"`). Each board's `notes` there give its size, pin layout and source. An id the app doesn't know is an error. A card's outline has its edge tab along the bottom, the bracket end at x = 0; its fingers are copper pads on Top and Bottom that traces join (not stored; named as `boards.json` names them). A header board's pins are plated through-hole pads (their sizes in `boards.json`) on every layer, and its mounting holes are non-plated (a Plane is cleared round them). A card with fingers needs 2 or more layers; a header board can have 1. Written only for a card; older builds ignore it. |
| `cardOptions` | object | Only with a `type` that has options in `boards.json`: each option's value (a number, a word or a flag) under its `id`, written for every option of the type, defaults included. A missing key takes its default; a key that isn't one of the type's options is ignored (older builds wrote every type's keys); a value that isn't one of the option's choices is an error (`card lanes 2 must be 1, 4, 8 or 16`). The options: `lanes` (PCI Express: 1, 4, 8 or 16, default 1), `lowProfile` (PCI Express: 68.90 mm high and at most 167.65 long instead of 111.15 and 312, default false), `length` (M.2: 30, 42, 60, 80 or 110 mm, default 80), `key` (M.2: `"b"`, `"m"`, `"bm"`, `"e"` or `"ae"`, default `"m"`), `halfSize` (Mini PCIe: 26.80 mm long instead of 50.95, default false), `memory` (SO-DIMM, DIMM: `"ddr3"`, `"ddr4"` or `"ddr5"`, default `"ddr4"`), `registered` (DIMM: a DDR5 RDIMM, whose key is 2.45 mm further right, default false), `icsp` (Arduino shield: the 2 × 3 ICSP header's plated holes ICSP.1–ICSP.6, on no bus and no net, default false), `pins` (Pmod: 6 or 12, default 6) and `size` (mikroBUS: `"s"`, `"m"` or `"l"`, default `"s"`). Each sets the board's size; only a PCI Express card's or a Pmod's length (`width`) is free. |
| `edgeBus` | UUID  | Optional. The schematic bus the card's edge (or a HAT's or shield's header) uses: its wires are the card's pins, in order (only the count has to match, not the names). The edge acts as a hidden port on that bus with every wire: each finger joins its wire's net, so the PCB's unrouted lines run to it. Written only for a card, while that bus exists and has as many wires as the card has pins. The app sets it back to none as soon as that stops being true (the card's pin count changes, or the bus goes), and ignores one that doesn't hold when opening. That bus can't be deleted, and its wire count can't change. |
| `card`   | object  | Only with `type` `"customCard"`: the edge connector, in mm — `pads` (fingers per side, 1–200, default 20), `pitch` (0.5–10, default 2.54), `padWidth` (default 1.524, less than the pitch), `padLength` (default 7.62), `padOffset` (from the card's bottom edge up to the fingers, default 0), `depth` (how far the tab reaches below the card, 1–50, default 7.62), `indent` (card's left edge to the tab, default 10), `margin` (tab end to the outermost finger's centre, default 2.54), `chamfer` (the tab's lower corners, default 0.5) and `bothSides` (fingers B1… on Bottom too, default true). Fingers are A1… on Top, numbered from the left. Out-of-range values are brought into range. |
| `width`, `height` | number | The board rectangle from (0, 0), in mm, each 1 to 1000. Default 100 × 80. For a custom card, both are free, at least big enough for the tab. For a standard board, `height` is the standard's (a stored one is ignored) and `width` is either the standard's too or, for a card whose length is free (ISA, PCI, PCI Express, Pmod), the card's length, kept within the board's `length` range in `boards.json` (at least long enough to clear its connector, at most full length). |
| `layers` | integer | Copper layers: 1, 2, 4 or 6. Default 2. Layer 1 is `top` and the last is `bottom`; the ones between are `inner2` to `inner5`. A 1-layer board has only `top`. |
| `layerStyles` | array | Optional. Each copper layer's colour and type: `layer` (`"top"`, `"inner2"` to `"inner5"`, or `"bottom"`), `color` (`"#RRGGBB"`, sRGB) and `type` (`"component"` for `top` and `bottom`, which is all they can be; `"plane"` or `"wire"` for an inner layer). Written for every layer on the board, in stack order. A missing entry, `color` or `type` takes its default: `top` `#C83737` red, `inner2` `#D8B23A` gold, `inner3` `#3AAE5C` green, `inner4` `#B04AD0` purple, `inner5` `#3AB8C8` cyan, `bottom` `#3A6FD8` blue; inner layers are `plane`. An inner layer may also have `net`, the net a Plane carries: `"ground"` (the GND rail), `"power:<mV>"` (the supply rail of that voltage, e.g. `"power:5000"`) or `"pin:<part id>/<pin designator>"` (the signal net that pin is wired into). Missing means none. It is kept while the layer is `wire` and has no effect there. A Plane on a net joins every plated through-hole pad and via on that net (so the PCB draws no unrouted line between them), and is drawn cut away around every other plated pad and via by its clearance, and around every non-plated hole and the board's edge by the DRC's min copper to edge. An inner layer may also have `clearance`, the Plane's clearance in mm, 0.05 to 10 (a high-voltage Plane wants more than the DRC's minimum); missing means the DRC's min clearance. It is kept while the layer is `wire`. A net the schematic no longer has joins nothing. Older builds ignore both. |
| `color`  | string  | The board colour: `"green"` (default), `"black"`, `"red"`, `"blue"`, `"yellow"`, `"white"` or `"purple"`. |
| `units`  | string  | The units the PCB sidebar shows lengths in: `"mm"` (default) or `"in"`. Lengths in the file are always mm. |
| `grid`   | number  | The component snap grid in mm, 0.01 to 10; default 0.635 (25 mil). Footprints and texts snap to it, and opening snaps text anchors to it. Trace ends snap to the trace grid, half of it, which is not stored. |
| `texts`  | array   | Free texts on the board, in drawing order. Default none.              |
| `traces` | array   | Copper traces, in drawing order ([Traces](#traces)). Always written, `[]` when there are none; missing means none. Older builds ignore it. |
| `vias`   | array   | Vias, in drawing order ([Vias](#vias)). Written only when there are some; missing means none. Older builds ignore it. |
| `holes`  | array   | Drilled holes (mounting holes and others), in drawing order ([Holes](#holes)). Written only when there are some; missing means none. Older builds ignore it. |

A missing key takes its default. A text:

| Field      | Type    | Meaning                                                              |
|------------|---------|----------------------------------------------------------------------|
| `id`       | UUID    | Unique within the file, across parts, junctions, wires and texts.    |
| `text`     | string  | The words; may hold line breaks (`\n`). Never empty.                 |
| `x`, `y`   | number  | mm: the text's top-left at rotation 0, which it rotates about. Snapped to the 0.635 mm grid on open. |
| `size`     | integer | Font size in mil (1/1000 inch), 10 to 1000. Default 50.             |
| `layer`    | string  | `"silkscreen"` (default) or `"document"`.                           |
| `rotation` | integer | 0 (default), 90, 180 or 270 degrees clockwise.                      |
| `side`     | string  | `"top"` (default) or `"bottom"`. A bottom text is drawn mirrored inside its own box. Always written; not allowed on a 1-layer board. |
| `locked`   | boolean | Lock Position: a locked text can't be dragged, moved by X and Y, or rotated. Default `false`. Written only when `true`; older builds ignore it. |

### Traces

```json
"traces": [ { "id": "...", "layer": "top", "width": 10,
              "points": [ { "x": 2.54, "y": 5.08 }, { "x": 7.62, "y": 5.08 }, { "x": 10.16, "y": 7.62 } ] } ]
```

| Field    | Type    | Meaning                                                              |
|----------|---------|----------------------------------------------------------------------|
| `id`     | UUID    | Unique within the file, across every other ID.                       |
| `layer`  | string  | The copper layer: `"top"`, `"inner2"` to `"inner5"`, or `"bottom"`. It must be on the board and not a Plane. |
| `width`  | integer | mil, 5 to 100. Default 10.                                           |
| `points` | array   | `x`, `y` in mm. At least 2; no two consecutive points the same. Segments may run at any angle (the Trace tool draws 20°, 40°, 45°, 60°, 80° or 90° legs). Kept as saved, not snapped: a bend, a crossing of two diagonals, or a pad centre, can be off the grid. |
| `locked` | boolean | Lock Position: a locked trace can't be dragged. Default `false`. Written only when `true`; older builds ignore it. |

Traces that touch or cross on the same layer share a vertex at each contact
point; that shared vertex is what connects them. Traces on different layers
never connect, except through a through-hole pad or a via.

### Vias

```json
"vias": [ { "id": "...", "x": 12.7, "y": 5.08, "drill": 0.4, "diameter": 0.8 } ]
```

A via is a plated hole through every copper layer. It joins every trace
whose copper touches its metal, on any layer, and every pad its metal
overlaps. A 1-layer board has none.

| Field      | Type   | Meaning                                                            |
|------------|--------|--------------------------------------------------------------------|
| `id`       | UUID   | Unique within the file, across every other ID.                     |
| `x`, `y`   | number | The centre, in mm. Kept as saved, not snapped.                     |
| `drill`    | number | The hole's diameter in mm, 0.1 to 5. Default 0.4.                  |
| `diameter` | number | The metal's outer diameter in mm, 0.2 to 10, and at least 0.1 more than `drill` (a 0.05 mm ring). Default 0.8. |
| `locked`   | boolean | Lock Position: a locked via can't be dragged or moved by X and Y. Default `false`. Written only when `true`; older builds ignore it. |

### Holes

```json
"holes": [ { "id": "...", "x": 3.5, "y": 3.5, "size": "m3", "drill": 3.2, "diameter": 6, "plated": true, "net": "ground" } ]
```

A drilled hole through the board, such as a mounting hole for a screw.
A bare (non-plated) hole has no copper: a Plane is cut away round it and
nothing joins it. A plated hole has a round copper land on every layer.
It joins every trace whose copper touches the land on any layer, and
every pad the land overlaps. Its `net` puts it on that net, as a pad wired
to it is: it gets unrouted lines to the net's other pads, and a Plane on
that net joins it. The Trace tool snaps a trace end on its land to the
nearest of four points in the middle of the ring: right of, below, left
of and above the hole. In lists it is `H1`, `H2`, … in drawing order.

| Field      | Type    | Meaning |
|------------|---------|---------|
| `id`       | UUID    | Unique within the file, across every other ID. |
| `x`, `y`   | number  | The centre, in mm. Kept as saved, not snapped. |
| `size`     | string  | `"custom"`, or a screw size, which sets `drill` and `diameter` (a close-fit clearance hole, and a land as wide as the screw's pan head): `"m2"` (2.2 / 4.0), `"m2.5"` (2.7 / 5.0), `"m3"` (3.2 / 6.0), `"m4"` (4.3 / 8.0), `"m5"` (5.3 / 10.0), `"m6"` (6.4 / 12.0), `"#2"` (2.4 / 4.5), `"#4"` (3.0 / 6.0), `"#6"` (3.7 / 7.0), `"#8"` (4.3 / 8.5) or `"#10"` (5.0 / 9.5). Default `"m3"`. |
| `drill`    | number  | The hole's diameter in mm, 0.3 to 10. Default the size's (a custom hole's: 3.2). |
| `diameter` | number  | The land's outer diameter in mm, 0.5 to 20, and at least 0.2 more than `drill` (a 0.1 mm ring). Kept while not plated. Default the size's (a custom hole's: 6.0). |
| `plated`   | boolean | A plated hole, with its land. Default `false`. Written only when `true`. |
| `net`      | string  | The plated hole's net, as a Plane layer's (`"ground"`, `"power:<mV>"` or `"pin:<part UUID>/<designator>"`). Missing is none. Kept while not plated. |
| `locked`   | boolean | Lock Position: a locked hole can't be dragged or moved by X and Y. Default `false`. Written only when `true`. |

## Junctions

```json
{ "id": "0D9E...", "x": 340, "y": 160 }
```

A junction is a fixed point where wires meet. At least one wire should end at
each junction. A junction stays while any wire ends on it; the app removes it
once none does.

| Field   | Type    | Meaning                                                         |
|---------|---------|-----------------------------------------------------------------|
| `loose` | boolean | Optional. `true`: the spot where a deleted pin was; wires on it have a loose end. Written only when true. |

## Wires

```json
{
  "id": "A41B...",
  "from": { "part": "6F1C...", "pin": "3" },
  "to":   { "junction": "0D9E..." },
  "bends": [ { "x": 300, "y": 150 }, { "x": 300, "y": 160 } ]
}
```

| Field   | Type   | Meaning                                                         |
|---------|--------|-----------------------------------------------------------------|
| `id`    | UUID   | Unique within the file.                                         |
| `from`  | end    | Where the wire starts.                                          |
| `to`    | end    | Where the wire ends.                                            |
| `bends` | array  | Bend points between the two ends, in order. May be empty.       |

An end is one of:

- `{ "part": <part id>, "pin": "<designator>" }`: a pin on a placed part.
  The designator is the pin's label from the part library, a string: the
  package pin number on numbered packages (`"14"`), the ball name on ball
  grids (`"A1"`).
- `{ "junction": <junction id> }`: a junction.
- `{ "port": <port id>, "wire": <bus wire id> }`: the pin of a bus port for
  one bus wire ([Ports](#ports)). The wire must be ticked on that port.
- `{ "label": <label id> }`: the point of a net label ([Net labels](#net-labels)).

The ends' own positions are not stored. They come from the part's pin layout
or the junction. The app re-fits the bends next to each end when the wire is
drawn, so slightly stale bends still produce a right-angle route.

## Buses

A bus is a thick line that carries 2 to 999 named wires. It is drawn only on
the schematic and connects to nothing by itself.

```json
"buses": [
  { "id": "…", "name": "BUS 1",
    "lines": [
      { "id": "…", "a": { "x": 100, "y": 200 }, "b": { "x": 300, "y": 200 } },
      { "id": "…", "a": { "x": 180, "y": 200 }, "b": { "x": 180, "y": 300 } }
    ],
    "wires": [ { "id": "…", "name": "B0" }, { "id": "…", "name": "B1" } ] }
]
```

(The second line ends on the middle of the first: a T.)

| Field   | Type  | Meaning                                                              |
|---------|-------|----------------------------------------------------------------------|
| `id`    | UUID  | Unique within the file.                                              |
| `name`  | string | The bus's name, shown in its editor: "BUS 1", "BUS 2", … when drawn. Unique ignoring case. Always written; a bus without one (older files) is named "BUS <n>" in file order. |
| `type`  | string | Optional. What the bus carries: missing for Custom (free wires), or one of the board `type` values for a card (`"isa8"`, `"pcie"`, `"nes"`, …; see [Board](#board)), whose pin names are the bus's `wires`, in the board's `pinOrder` (see [boards-format.md](boards-format.md); natural order when it has none: A1, A2, … A10, B1, numbers before letters). The app locks them until the type is Custom again. A type whose pins don't match `wires` is dropped. The board's `edgeBus` is what ties a bus to the card. |
| `card`, `cardOptions` | object | Optional. The type's settings, as the board's `card` (a custom card: only `pads` and `bothSides` change its pins) and `cardOptions` (only the options that change the pins, `onBus` in `boards.json`: PCI Express `lanes`, M.2 `key`, `memory`, a Pmod's `pins`). Written only for the types that have them. |
| `lines` | array | The bus's straight lines, at least one. Each has an `id` (UUID) and two ends `a` and `b` (canvas units, on the 10-unit grid). A line is horizontal or vertical, with length > 0. |
| `wires` | array | The bus's wires in list order, 2 to 999. Each has an `id` (UUID; ports refer to it) and a `name`: 1 to 4 characters with no spaces, unique in the bus ignoring case. |
| `ports` | array | The bus's ports ([Ports](#ports)). Always written, empty when there are none; missing means none. |

**Joints.** Lines of one bus meet only at right angles, in one of two ways:

- an **L**: exactly two line ends meet at one point;
- a **T**: one line's end lies strictly inside another line.

Nothing else may touch there (no third line, no end on a corner or on a T).
Lines that cross in their middles do not touch: that is a crossing, as for
wires. Joints are not written; they are worked out from the lines on open.
Every line of a bus is joined to the others through joints (one piece), and
no two lines overlap along their length. No line end of one bus may touch a
line of another bus.

### Ports

A port is a box on one bus line with one pin per ticked bus wire. Wires
connect to its pins. Every pin for the same bus wire, on any port of the
bus (whichever line it is on), is one net.

```json
"ports": [ { "id": "…", "line": "<bus line id>", "start": 140,
             "side": "bottom", "wires": [ "<bus wire id>", "…" ] } ]
```

| Field   | Type   | Meaning                                                             |
|---------|--------|---------------------------------------------------------------------|
| `id`    | UUID   | Unique within the file.                                             |
| `line`  | UUID   | The line of this bus the port sits on.                              |
| `start` | number | Along the line (x on a horizontal line, y on a vertical one): the first pin's position. Pin i is at `start + 20·i`. |
| `side`  | string | Where the pins come out: `"top"` or `"bottom"` on a horizontal line, `"left"` or `"right"` on a vertical one. |
| `wires` | array  | The ticked bus wire ids, in bus order. May be empty (the port is then one slot long). |

The port takes `[start − 10, start − 10 + 20·max(n, 1)]` along its line (n
pins). That range lies within the line, overlaps no other port on the same
side of the line (ports on opposite sides may share a place), and has no T
inside it (it may end at one). The body is 40 deep on the pin
side, and each pin's tip is 50 out from the line.

## Notes

Text blocks the schematic's Text tool places: comments and labels. They
are drawn under everything else and never simulated.

```json
"notes": [ { "id": "...", "text": "Clock divider\nfeeds U3", "x": 40, "y": 60,
             "size": 12, "framed": true } ]
```

| Field      | Type    | Meaning                                                              |
|------------|---------|----------------------------------------------------------------------|
| `id`       | UUID    | Unique within the file, across every other ID.                       |
| `text`     | string  | The words; may hold line breaks (`\n`). Never empty.                 |
| `x`, `y`   | number  | Canvas units: the block's top-left. Snapped to the grid on open.     |
| `size`     | integer | Font size in points, 6 to 72. Default 12.                            |
| `framed`   | boolean | Whether the block has a frame around it. Default `true`.             |

## Net labels

Flags the schematic's Net Label tool places. Every label with the same
name, ignoring case, is one net: the wires on them are connected without
being drawn to each other (in the simulation, the voltage check and the
PCB's unrouted lines). A label is never simulated itself.

```json
"labels": [ { "id": "...", "name": "CLK", "x": 250, "y": 100, "type": "input" },
            { "id": "...", "name": "clk", "x": 450, "y": 300, "type": "output", "flipped": true } ]
```

| Field      | Type    | Meaning                                                              |
|------------|---------|----------------------------------------------------------------------|
| `id`       | UUID    | Unique within the file, across every other ID.                       |
| `name`     | string  | The net's name: 1 to 4 characters, no spaces.                        |
| `x`, `y`   | number  | Canvas units: the label's point, where a wire ends. Snapped to the grid on open. |
| `type`     | string  | `"input"` (default; the end at the wire points in at it), `"output"` (the far end points away from the wire), `"inout"` (both ends point), `"passive"` (a plain box) or `"noConnect"` (see below). |
| `flipped`  | boolean | The flag runs left of its point instead of right. Default `false`; written only when `true`. |

The flag is 60 long and 20 high, centred on its point's row.

A `"noConnect"` label is not a net: it is drawn as an X on its point,
with no name and no flag, and marks a pin as left unconnected on purpose.
It joins nothing, not even the wires that end on it, so a pin wired to
one is on a net of its own and the PCB shows no unrouted line for it. It
still stores a `name` (ignored), so it can turn back into a named label.

## Custom parts

A custom part is one loaded from a single part file (`.schpart`,
`docs/parts-format.md` § Single part files) by File › Load Part… or made in
the Part Editor. A schematic that places one embeds it, so the schematic
opens on a Mac where the part was never loaded:

```json
{
  "format": "schematical",
  "version": 5,
  "customParts": [
    { "format": "schematical-part", "version": 1, "uuid": "6F1C2E8A-…", "kind": "gate",
      "footprints": [ … ], "devices": [ … ] }
  ],
  "parts": [
    { "id": "…", "reference": "U1", "partNumber": "SN74HC00N.2", "partUUID": "6F1C2E8A-…",
      "x": 200, "y": 120, "flipped": false }
  ]
}
```

- `customParts` holds each custom part's single part file object, as it was
  loaded (its name with any suffix the app gave it), once per part, in order
  of first placement. Bundled parts are never embedded.
- `version` is 5 exactly when `customParts` is written. A schematic with no
  custom part is written as version 4, byte for byte as before, so an older
  build still opens it; an older build refuses a version 5 file as "saved by a
  newer version of Schematical (file format 5)".
- On open, every `customParts` entry is loaded first, with File › Load Part…'s
  rules: one whose uuid is already loaded with the same content is that part;
  one whose uuid is loaded with other content fails the open ("… has the
  identity …, which a different loaded part already has"); one the part
  loader refuses fails the open with the loader's message. A name already
  taken gets the lowest free suffix (`.2`, `.3`, …). Loaded parts stay for the
  rest of the session, even if the open fails later.
- A placed part with a `partUUID` resolves to the custom part with that uuid;
  one with no matching entry is an unknown part.

## Validation on open

Opening fails, and the canvas is left unchanged, if:

- `format` is missing or not `"schematical"`, or `version` is newer than the app supports;
- a `customParts` entry can't be loaded ([Custom parts](#custom-parts));
- the JSON doesn't match this structure;
- an ID is used more than once across parts, junctions, wires, buses, bus
  lines, bus wires, ports, board texts, traces, notes and net labels;
- a note's `text` is empty, or its `size` is not 6 to 72;
- a net label's `name` is not 1 to 4 characters with no spaces, or its
  `type` is not `input`, `output`, `inout`, `passive` or `noConnect` ("The net label is not valid:
  net label <id> …");
- a `partNumber` is not in the catalog (the error lists every missing one);
- an LED's `color` is not one of its colors (`U3: "Purple" is not a color of
  GENERIC-LED`), or a `footprint` is not one of the part's footprint choices
  (`U3: "LED-9999" is not a footprint of GENERIC-LED`);
- a header's `gender` is not one of its genders (`U4: "Neutral" is not a
  gender of GENERIC-HEADER-1X08`);
- `GENERIC-CAPACITOR`'s `farads` is not at least 1e-13 and at most 1 (`C3 has capacitance 0 F; it must be more than 0 and at most 1 F.`);
- a pull resistor's `ohms` is not more than 0 and at most 1e9 (`R3 has
  resistance 0 Ω; it must be more than 0 and at most 1 GΩ.`);
- a wire end names a part or junction that isn't in the file, or a pin the
  part's symbol doesn't draw (for example a pin with role `absent`);
- a wire end names a port that isn't in the file ("Wire <id> connects to
  port <id>, which is not in the file."), or a bus wire that isn't ticked on
  that port ("Wire <id> connects to a pin port <id> does not have.");
- a wire end names a net label that isn't in the file ("Wire <id> connects
  to net label <id>, which is not in the file.");
- the pins don't match the version: a version 1 file with a string `"pin"`, or a
  version 2 file with an integer `"pin"`. A file uses one or the other;
- a part with a footprint has a `pcb.rotation` other than 0, 90, 180 or 270
  ("U2 has PCB rotation 45; it must be 0, 90, 180 or 270.");
- a board setting, footprint origin or mark, or board text is not valid. The
  message is "The PCB settings are not valid: <detail>.", where the detail is
  one of:
  - `board width 0 must be 1 to 1000 mm` (same for `height`);
  - `board grid 0 must be 0.01 to 10 mm`;
  - `board type "eisa" must be custom, customCard, isa8, isa16, pci, nes, atari2600, atari5200, atari7800, atari8bit, atariST, jaguar, lynx, coco, pcie, m2, miniPCIe, sodimm, dimm, piHat or arduinoShield`;
  - `board type isa8 needs 2 or more layers`;
  - `card lanes 2 must be 1, 4, 8 or 16`;
  - `card length 50 must be 30, 42, 60, 80 or 110`;
  - `board layers 3 must be 1, 2, 4 or 6`;
  - `board color "pink" must be green, black, red, blue, yellow, white or purple`;
  - `board units "cm" must be mm or in`;
  - `layer "inner4" is not on a 4-layer board`;
  - `layer "middle" must be top, inner2 to inner5, or bottom`;
  - `layer top type "plane" must be component` (same for `bottom`);
  - `layer inner2 type "component" must be plane or wire`;
  - `layer top color "red" must be #RRGGBB`;
  - `layer inner2 net "vcc" must be ground, power:<mV> or pin:<part id>/<pin>`;
  - `layer top is not a plane; it has no net` (same for `bottom`, and for `clearance`);
  - `layer inner2 clearance 20 must be 0.05 to 10 mm`;
  - `layer top is listed twice`;
  - `U2 origin "middle" must be center, topLeft, centerLeft, bottomLeft or pad1Center`;
  - `U2 style "x" must be module or connector` (a part with a connector
    footprint);
  - `U2 outline "on" must be hidden, document or silkscreen` (same for
    `pin1` and `reference`);
  - `text <id> size 5 must be 10 to 1000 mil`;
  - `text <id> layer "x" must be document or silkscreen`;
  - `text <id> rotation 45 must be 0, 90, 180 or 270`;
  - `text <id> is empty`;
  - `U2 side "left" must be top or bottom`;
  - `text <id> side "left" must be top or bottom`;
  - `U2 is on the bottom of a 1-layer board`;
  - `text <id> is on the bottom of a 1-layer board`;
  - `trace <id> layer "inner4" is not on a 4-layer board`;
  - `trace <id> layer "middle" must be top, inner2 to inner5, or bottom`;
  - `trace <id> layer inner2 is a plane`;
  - `trace <id> width 3 must be 5 to 100 mil`;
  - `trace <id> has fewer than 2 points`;
  - `trace <id> segment 2 has no length` (segments are numbered from 1);
  - `via <id> is on a 1-layer board`;
  - `via <id> drill 0.05 must be 0.1 to 5 mm`;
  - `via <id> diameter 11 must be 0.2 to 10 mm`;
  - `via <id> diameter 0.55 must be at least 0.1 mm more than its drill 0.5`;
  - `hole <id> size "m7" must be custom, m2, m2.5, m3, m4, m5, m6, #2, #4, #6, #8 or #10`;
  - `hole <id> drill 0.2 must be 0.3 to 10 mm`;
  - `hole <id> diameter 3.1 must be at least 0.2 mm more than its drill 3`;
  - `hole <id> net "vcc" must be ground, power:<mV> or pin:<part id>/<pin>`;
- a bus is not valid. The message is "The bus is not valid: <detail>.",
  where the detail is one of:
  - `bus <id> has no lines`;
  - `line <id> is not a horizontal or vertical line on the grid` (diagonal,
    off the grid, or length 0);
  - `lines <id> and <id> overlap`;
  - `bus <id> has a joint at (<x>, <y>) that is not a T or an L`;
  - `bus <id> is not one piece`;
  - `buses <id> and <id> touch at (<x>, <y>)`;
  - `bus <id> has <n> wires; it must have 2 to 999`;
  - `bus <id> wire name "<name>" must be 1 to 4 characters with no spaces`;
  - `bus <id> uses the wire name "<name>" twice`;
  - `bus <id> type "eisa" must be custom, customCard, isa8, …` (the board types; likewise `card lanes`, `card length`, `card key` and `card memory` for its settings);
  - `the bus name "<name>" is used twice`;
  - `port <id> line <id> is not on bus <id>`;
  - `port <id> side "<s>" must be top or bottom on a horizontal line` (left
    or right on a vertical one);
  - `port <id> wire <id> is not on bus <id>`;
  - `port <id> does not fit on its line` (off the line, overlapping another
    port on the same side, or over a T).

A missing or unreadable EEPROM load file (`memoryFile`) does not fail Open:
the memory window shows the problem, and Run stops with it
(`"U4: can't read boot.bin: …"`).

## Older versions

Version 3 is version 4 without the part keys `color` and `footprint`. Its
LEDs were separate parts per color and package; they open as described in
[Old LED part numbers](#old-led-part-numbers). Saving always writes version
4. Version 3 builds refuse version 4 files as "saved by a newer version of
Schematical (file format 4)".

Version 2 is version 3 without buses: it has no `buses` key. The app opens
version 2 files with no buses. Saving always writes version 4. Version 2
builds refuse version 3 files as "saved by a newer version of Schematical
(file format 3)", rather than opening them and losing the buses on save.

Version 1 is version 2 except for wire ends: `"pin"` is an integer, and
pin n is the part's n-th pin in its pinout (the order its part file lists
them). For example:

```json
{ "format": "schematical", "version": 1, ... }
{ "part": "6F1C...", "pin": 14 }
```

The app opens version 1 files and converts each pin to its designator.
Builds from before version 2 refuse newer files as "saved by a newer version
of Schematical".

### Old LED part numbers

Before version 4 the library had 55 LED parts, `LED-<COLOR>-<PACKAGE>`:
colors `RED`, `GREEN`, `BLUE`, `AMBER`, `WHITE`, and packages `0402`, `0603`,
`0805`, `1204`, `1205`, `1206`, `1209`, `1411`, `1414`, `DIP`, `RADIAL`. A file
of any version that names one opens as `GENERIC-LED` with color `Red`,
`Green`, `Blue`, `Amber` or `White`, and footprint `LED-<package>` (`LED-DIP4`
for `DIP`, `LED-RADIAL-5MM` for `RADIAL`), at the same canvas and PCB place
(`SchematicFile.legacyLEDs`). Wires stay on their pins: every old pinout had
K at pin 1 and A at pin 2. Like `VCC`, it is saved under the new name.

## Changing the format

- Adding an optional field: keep `version` as it is, and decode the field as
  optional with a default.
- Renaming, removing or changing the meaning of a field: bump `version`, and keep
  reading older versions by converting them on open.
- The app refuses files with a newer `version` rather than guessing.
- A part removed from the library may map to its replacement on open
  (`VCC` → `5V`, `SchematicFile.renamedParts`; the old LEDs →
  `GENERIC-LED`, `SchematicFile.legacyLEDs`), without a version bump: the
  `partNumber` still names a library part, and saving writes the new name.

## Example

A red LED between a 5 V supply and GND:

```json
{
  "format": "schematical",
  "version": 4,
  "nextReference": 4,
  "parts": [
    { "id": "11111111-1111-1111-1111-111111111111", "reference": "U1", "partNumber": "5V", "x": 100, "y": 40, "flipped": false },
    { "id": "22222222-2222-2222-2222-222222222222", "reference": "U2", "partNumber": "GENERIC-LED", "color": "Red", "footprint": "LED-RADIAL-5MM", "x": 200, "y": 100, "flipped": false,
      "pcb": { "x": 0, "y": 0, "rotation": 0, "origin": "topLeft",
               "outline": "document", "pin1": "silkscreen", "reference": "silkscreen", "side": "bottom" } },
    { "id": "33333333-3333-3333-3333-333333333333", "reference": "U3", "partNumber": "GND", "x": 300, "y": 180, "flipped": false }
  ],
  "junctions": [],
  "wires": [
    {
      "id": "44444444-4444-4444-4444-444444444444",
      "from": { "part": "11111111-1111-1111-1111-111111111111", "pin": "1" },
      "to": { "part": "22222222-2222-2222-2222-222222222222", "pin": "2" },
      "bends": [ { "x": 120, "y": 120 } ]
    },
    {
      "id": "55555555-5555-5555-5555-555555555555",
      "from": { "part": "22222222-2222-2222-2222-222222222222", "pin": "1" },
      "to": { "part": "33333333-3333-3333-3333-333333333333", "pin": "1" },
      "bends": [ { "x": 320, "y": 120 } ]
    }
  ],
  "buses": [],
  "board": {
    "width": 50, "height": 30, "layers": 2, "color": "green", "units": "mm",
    "layerStyles": [
      { "layer": "top", "color": "#C83737", "type": "component" },
      { "layer": "bottom", "color": "#3A6FD8", "type": "component" }
    ],
    "texts": [
      { "id": "66666666-6666-6666-6666-666666666666", "text": "LED", "x": 5.08, "y": 2.54,
        "size": 50, "layer": "silkscreen", "rotation": 0, "side": "top" }
    ],
    "traces": [
      { "id": "77777777-7777-7777-7777-777777777777", "layer": "bottom", "width": 10,
        "points": [ { "x": 2.54, "y": 10.16 }, { "x": 7.62, "y": 10.16 }, { "x": 10.16, "y": 12.7 } ] }
    ]
  },
  "notes": [
    { "id": "88888888-8888-8888-8888-888888888888", "text": "Power LED\n5 V rail", "x": 100, "y": 220,
      "size": 12, "framed": true }
  ]
}
```

Part numbers in the example are illustrative. Use names from the catalog.
