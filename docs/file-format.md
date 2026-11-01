# Schematical file format

Schematical saves a schematic as a UTF-8 JSON file with the extension
`.schematical`. This document describes version 2. Version 1 files are
converted when opened (see [Version 1 files](#version-1-files)).

Code: `schematical/schematical/Document/SchematicFile.swift` (the format) and
`SchematicFileController.swift` (Save, Save As, Open).

## Design rules

- **Parts are stored by part number only.** Pins, pin roles, symbol and layout
  come from the part library (`schematical/schematical/Parts/*.json`) when the
  file is opened. Files stay small, and fixes to a part's model apply to
  existing files.
- **Connections are explicit.** A wire end names a part and pin designator, or a
  junction. Connectivity never depends on two coordinates touching.
- **IDs are stable.** Every part, junction and wire has a UUID that is kept
  across save and open.
- **The selection, zoom, scroll position and simulation state are not saved.**

## Coordinates

All positions are in canvas units. The grid spacing is 10 units, and y grows
downward. Part origins are snapped to the grid when a file is opened.

PCB placements (`pcb`, below) are in board millimetres instead: the
footprint's centre (the middle of its body), y down. When a file is opened
the footprint is shifted so the top-left of its bounding box (body plus pads,
as rotated) sits on the 0.635 mm (25 mil) grid.

## Top level

```json
{
  "format": "schematical",
  "version": 2,
  "nextReference": 4,
  "parts": [ ... ],
  "junctions": [ ... ],
  "wires": [ ... ]
}
```

| Field           | Type    | Meaning                                                              |
|-----------------|---------|----------------------------------------------------------------------|
| `format`        | string  | Always `"schematical"`. Other values are rejected.                    |
| `version`       | integer | Format version. Files newer than the app supports are rejected.      |
| `nextReference` | integer | Number for the next placed part's reference (`U` + number).          |
| `parts`         | array   | Placed parts, in drawing order (later parts draw on top).            |
| `junctions`     | array   | Points where wires branch.                                           |
| `wires`         | array   | Wires between pins and junctions.                                    |

On open, `nextReference` is raised if needed so that it is higher than every
existing `U<n>` reference.

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

Every part with a footprint (all but VCC and GND) also stores where its
footprint sits on the PCB:

```json
{ "id": "...", "reference": "U2", "partNumber": "LED-AMBER-0603",
  "x": 200, "y": 100, "flipped": false,
  "pcb": { "x": 5.08, "y": 0, "rotation": 90 } }
```

A DIP switch also stores its positions:

```json
{ "id": "...", "reference": "U2", "partNumber": "TE-GDS0404",
  "x": 80, "y": 120, "flipped": false, "switches": [true, false, false, true] }
```

| Field        | Type    | Meaning                                                                 |
|--------------|---------|-------------------------------------------------------------------------|
| `id`         | UUID    | Unique within the file.                                                 |
| `reference`  | string  | Label shown on the canvas, such as `U1`.                                |
| `partNumber` | string  | A part name from the part library (`schematical/schematical/Parts/*.json`): a logic part, memory (SRAM), switch, LED, 7-segment display, oscillator, or the supplies `VCC` and `GND`. |
| `x`, `y`     | number  | Top-left corner of the part body.                                       |
| `flipped`    | boolean | Pins on the opposite sides. Ignored for VCC and GND.                    |
| `switches`   | array of booleans | DIP switches only, optional: whether each position is ON, position 1 first. Missing positions are OFF; extra entries are ignored. Left out for other parts. |
| `pcb`        | object  | Optional. The footprint on the PCB: `x`, `y` (numbers, mm) are the footprint's centre, `rotation` (integer) is 0, 90, 180 or 270 degrees clockwise. Written for every part with a footprint; left out for VCC and GND, and ignored on a part without a footprint. A part with a footprint and no `pcb` (older files) is placed automatically when the file is opened: after the stored ones, in file order, each to the right of everything already placed. |

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

The ends' own positions are not stored. They come from the part's pin layout
or the junction. The app re-fits the bends next to each end when the wire is
drawn, so slightly stale bends still produce a right-angle route.

## Validation on open

Opening fails, and the canvas is left unchanged, if:

- `format` is missing or not `"schematical"`, or `version` is newer than the app supports;
- the JSON doesn't match this structure;
- an ID is used more than once across parts, junctions and wires;
- a `partNumber` is not in the catalog (the error lists every missing one);
- a wire end names a part or junction that isn't in the file, or a pin the
  part's symbol doesn't draw (for example the no-connect pins of a 4-pin LED);
- the pins don't match the version: a version 1 file with a string `"pin"`, or a
  version 2 file with an integer `"pin"`. A file uses one or the other;
- a part with a footprint has a `pcb.rotation` other than 0, 90, 180 or 270
  ("U2 has PCB rotation 45; it must be 0, 90, 180 or 270.").

## Version 1 files

Version 1 is the same format except for wire ends: `"pin"` is an integer, and
pin n is the part's n-th pin in its pinout (the order its part file lists
them). For example:

```json
{ "format": "schematical", "version": 1, ... }
{ "part": "6F1C...", "pin": 14 }
```

The app opens version 1 files and converts each pin to its designator, and
saving always writes version 2. Builds from before version 2 refuse version 2
files as "saved by a newer version of Schematical".

## Changing the format

- Adding an optional field: keep `version` as it is, and decode the field as
  optional with a default.
- Renaming, removing or changing the meaning of a field: bump `version`, and keep
  reading older versions by converting them on open.
- The app refuses files with a newer `version` rather than guessing.

## Example

A red LED between VCC and GND:

```json
{
  "format": "schematical",
  "version": 2,
  "nextReference": 4,
  "parts": [
    { "id": "11111111-1111-1111-1111-111111111111", "reference": "U1", "partNumber": "VCC", "x": 100, "y": 40, "flipped": false },
    { "id": "22222222-2222-2222-2222-222222222222", "reference": "U2", "partNumber": "LED-RED-0603", "x": 200, "y": 100, "flipped": false,
      "pcb": { "x": 0, "y": 0, "rotation": 0 } },
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
  ]
}
```

Part numbers in the example are illustrative. Use names from the catalog.
