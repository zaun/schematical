# Schematical file format

Schematical saves a schematic as a UTF-8 JSON file with the extension
`.schematical`. This document describes version 4. Version 1, 2 and 3 files
open too (see [Older versions](#older-versions)).

Code: `schematical/schematical/Document/SchematicFile.swift` (the format) and
`SchematicFileController.swift` (Save, Save As, Open).

## Design rules

- **Parts are stored by part number only.** Pins, pin roles, symbol and layout
  come from the part library (`schematical/schematical/Parts/*.json`) when the
  file is opened. Files stay small, and fixes to a part's model apply to
  existing files.
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
rotated), or the centre of pad 1. The app snaps the origin point to the
0.635 mm (25 mil) grid when a footprint is moved or rotated, and rotates
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
  "board": { ... }
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
| `reference`  | string  | Label shown on the PCB and in memory window titles, such as `U1`: 1 to 8 letters then a number of 1 or more. New parts get `U<n>`; the PCB sidebar renames them (letters uppercased; a reference another part has is refused). |
| `partNumber` | string  | A part name from the part library (`schematical/schematical/Parts/*.json`): a logic part, memory (SRAM, EEPROM), switch, LED, 7-segment display, character LCD module, oscillator, processor, or a supply: `5V`, `3V3`, `2V5`, `1V8`, `1V2`, `12V` or `GND`. `VCC` (a supply removed from the library) opens as `5V`, and is saved as `5V`. The 55 LED parts `LED-<COLOR>-<PACKAGE>` (such as `LED-GREEN-RADIAL`, `LED-AMBER-0603`) that `GENERIC-LED` replaced open as `GENERIC-LED` with that part's color and footprint, in a file of any version, and are saved as `GENERIC-LED` (see [Old LED part numbers](#old-led-part-numbers)). |
| `x`, `y`     | number  | Top-left corner of the part body.                                       |
| `flipped`    | boolean | Pins on the opposite sides. Ignored for the supplies.                   |
| `switches`   | array of booleans | DIP switches only, optional: whether each position is ON, position 1 first. Missing positions are OFF; extra entries are ignored. Left out for other parts. |
| `pcb`        | object  | Optional. The footprint on the PCB: `x`, `y` (numbers, mm) are the footprint's centre, `rotation` (integer) is 0, 90, 180 or 270 degrees clockwise. `origin` (string, default `"topLeft"`) is the footprint's origin point (see [Coordinates](#coordinates)): `"center"`, `"topLeft"`, `"centerLeft"`, `"bottomLeft"` (of the bounding box as rotated) or `"pad1Center"` (the centre of the pad of the part's first pin). `outline` (the body outline, default `"document"`), `pin1` (the pin-1 dot, default `"silkscreen"`) and `reference` (the reference text, default `"silkscreen"`) say where each mark is drawn: `"hidden"`, `"document"` or `"silkscreen"`. The four strings are always written; a missing one takes its default. `style` (string, default `"module"`): `"module"` (Full Module) or `"connector"` (Connector Only), which of the part's two footprints is on the board. Written only for parts with a connector footprint (the character LCD modules); ignored on other parts. Written for every part with a footprint; left out for the supplies, and ignored on a part without a footprint. A part with a footprint and no `pcb` (older files) is placed automatically when the file is opened: after the stored ones, in file order, each to the right of everything already placed. |
| `color`      | string  | Version 4. LEDs only (`GENERIC-LED`): one of the part's colors, `"Red"`, `"Green"`, `"Blue"`, `"Amber"` or `"White"`. Used by the symbol's glow in play mode. Always written for an LED; missing means the part's default (`Red`). Ignored on other parts. |
| `footprint`  | string  | Version 4. Parts with footprint choices only (`GENERIC-LED`): the id of the chosen footprint, one of the part's `footprintChoices` (`docs/parts-format.md`), such as `"LED-0805"`. The PCB uses it. Always written for those parts; missing means the part's default footprint (`LED-RADIAL-5MM`). Ignored on other parts. |
| `memoryFile` | object  | EEPROMs only, optional: the file (raw binary, or Intel HEX for `.hex`/`.ihx`/`.ihex`) loaded into the chip at every start; without it the chip starts erased (every byte FF). `path` (string) is the file's path relative to the schematic's folder (`../roms/boot.bin`), or absolute for a schematic that had never been saved. `bookmark` (base64 string, optional) is an app-scoped security bookmark; on the Mac that wrote it, it is tried first, so the sandboxed app can read the file after a restart, and `path` is the fallback. Ignored on other parts. |

## Board

```json
"board": { "width": 100, "height": 80, "layers": 2, "color": "green", "units": "mm",
  "texts": [ { "id": "...", "text": "REV A", "x": 5.08, "y": 2.54,
               "size": 50, "layer": "silkscreen", "rotation": 0 } ] }
```

| Field    | Type    | Meaning                                                                |
|----------|---------|------------------------------------------------------------------------|
| `width`, `height` | number | The board rectangle from (0, 0), in mm, each 1 to 1000. Default 100 × 80. |
| `layers` | integer | Copper layers: 1, 2, 4 or 6. Default 2. Stored only; the PCB page draws the top side. |
| `color`  | string  | The board colour: `"green"` (default), `"black"`, `"red"`, `"blue"`, `"yellow"`, `"white"` or `"purple"`. |
| `units`  | string  | The units the PCB sidebar shows lengths in: `"mm"` (default) or `"in"`. Lengths in the file are always mm. |
| `texts`  | array   | Free texts on the board, in drawing order. Default none.              |

A missing key takes its default. A text:

| Field      | Type    | Meaning                                                              |
|------------|---------|----------------------------------------------------------------------|
| `id`       | UUID    | Unique within the file, across parts, junctions, wires and texts.    |
| `text`     | string  | The words; may hold line breaks (`\n`). Never empty.                 |
| `x`, `y`   | number  | mm: the text's top-left at rotation 0, which it rotates about. Snapped to the 0.635 mm grid on open. |
| `size`     | integer | Font size in mil (1/1000 inch), 10 to 1000. Default 50.             |
| `layer`    | string  | `"silkscreen"` (default) or `"document"`.                           |
| `rotation` | integer | 0 (default), 90, 180 or 270 degrees clockwise.                      |

## Junctions

```json
{ "id": "0D9E...", "x": 340, "y": 160 }
```

A junction is a fixed point where wires meet. At least one wire should end at
each junction. The app removes unused junctions when wires are erased.

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

The ends' own positions are not stored. They come from the part's pin layout
or the junction. The app re-fits the bends next to each end when the wire is
drawn, so slightly stale bends still produce a right-angle route.

## Buses

A bus is a thick line that carries 2 to 999 named wires. It is drawn only on
the schematic and connects to nothing by itself.

```json
"buses": [
  { "id": "…",
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

## Validation on open

Opening fails, and the canvas is left unchanged, if:

- `format` is missing or not `"schematical"`, or `version` is newer than the app supports;
- the JSON doesn't match this structure;
- an ID is used more than once across parts, junctions, wires, buses, bus
  lines, bus wires, ports and board texts;
- a `partNumber` is not in the catalog (the error lists every missing one);
- an LED's `color` is not one of its colors (`U3: "Purple" is not a color of
  GENERIC-LED`), or a `footprint` is not one of the part's footprint choices
  (`U3: "LED-9999" is not a footprint of GENERIC-LED`);
- a wire end names a part or junction that isn't in the file, or a pin the
  part's symbol doesn't draw (for example a pin with role `absent`);
- a wire end names a port that isn't in the file ("Wire <id> connects to
  port <id>, which is not in the file."), or a bus wire that isn't ticked on
  that port ("Wire <id> connects to a pin port <id> does not have.");
- the pins don't match the version: a version 1 file with a string `"pin"`, or a
  version 2 file with an integer `"pin"`. A file uses one or the other;
- a part with a footprint has a `pcb.rotation` other than 0, 90, 180 or 270
  ("U2 has PCB rotation 45; it must be 0, 90, 180 or 270.");
- a board setting, footprint origin or mark, or board text is not valid. The
  message is "The PCB settings are not valid: <detail>.", where the detail is
  one of:
  - `board width 0 must be 1 to 1000 mm` (same for `height`);
  - `board layers 3 must be 1, 2, 4 or 6`;
  - `board color "pink" must be green, black, red, blue, yellow, white or purple`;
  - `board units "cm" must be mm or in`;
  - `U2 origin "middle" must be center, topLeft, centerLeft, bottomLeft or pad1Center`;
  - `U2 style "x" must be module or connector` (a part with a connector
    footprint);
  - `U2 outline "on" must be hidden, document or silkscreen` (same for
    `pin1` and `reference`);
  - `text <id> size 5 must be 10 to 1000 mil`;
  - `text <id> layer "x" must be document or silkscreen`;
  - `text <id> rotation 45 must be 0, 90, 180 or 270`;
  - `text <id> is empty`;
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
               "outline": "document", "pin1": "silkscreen", "reference": "silkscreen" } },
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
    "texts": [
      { "id": "66666666-6666-6666-6666-666666666666", "text": "LED", "x": 5.08, "y": 2.54,
        "size": 50, "layer": "silkscreen", "rotation": 0 }
    ]
  }
}
```

Part numbers in the example are illustrative. Use names from the catalog.
