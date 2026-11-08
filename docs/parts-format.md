# Part file format

Every orderable part (gate, flip-flop, latch, decoder, counter, shift register, SRAM, EEPROM, switch, LED,
7-segment display, character LCD module, oscillator, supply, microprocessor, serial controller) is described in a JSON part file in
`schematical/schematical/Parts/`, one file per kind:

| File | Kind |
|---|---|
| `gate.json` | `gate` |
| `dff.json` | `dff` |
| `jkff.json` | `jkff` |
| `sram.json` | `sram` |
| `eeprom.json` | `eeprom` |
| `switch.json` | `switch` |
| `led.json` | `led` |
| `7seg.json` | `7seg` |
| `oscillator.json` | `oscillator` |
| `supply.json` | `supply` |
| `latch.json` | `latch` |
| `decoder.json` | `decoder` |
| `counter.json` | `counter` |
| `shift.json` | `shift` |
| `cpu.json` | `cpu` |
| `charlcd.json` | `charlcd` |
| `serial.json` | `serial` |
| `connector.json` | `connector` |

Beside them, `footprints.json` holds the land patterns parts name with
`"footprint"` (see [Footprints](#footprints)). It is loaded first, with
`load_footprints()`.

The backend loads each part file with `load_parts()` (`backend/include/parts.h`).
The loader checks everything below; a file that breaks any rule is rejected
as a whole, with a message that names the place, for example:

```
gate.json: devices[3] (74_00): parts[12] (SN74HC00N): unknown family "HX"
```

## The file

```json
{
  "format": "schematical-parts",
  "version": 1,
  "kind": "gate",
  "devices": [ ... ]
}
```

| Key | Value |
|---|---|
| `"format"` | always `"schematical-parts"` |
| `"version"` | always `1` |
| `"kind"` | one of the kinds in the table above |
| `"devices"` | array of devices |

No other key is allowed.

## Devices

A device is one chip design: its pinouts, its behavior, and the orderable
parts that share them.

```json
{
  "id": "74_00",
  "function": "NAND",
  "description": "Quad 2-input NAND gate",
  "drawer": ["Logic"],
  "note": "optional provenance text",
  "pinouts": { "p14": [ ... ], "lccc20": [ ... ] },
  "...kind keys...": "...",
  "parts": [ ... ]
}
```

| Key | Value |
|---|---|
| `"id"` | identifier, unique among the devices of the kind |
| `"function"` | the parts-drawer leaf, e.g. `"NAND"`, `"32 KB"`, `"LED"` |
| `"description"` | one line for people, e.g. `"Quad 2-input NAND gate"` |
| `"drawer"` | the parts-drawer path above the function: 0–2 strings, e.g. `["Memory", "SRAM"]`. `[]` means the parts are never listed in the drawer (VCC, GND). Presentation only. |
| `"note"` | optional; provenance (datasheet, table). The loader ignores it. |
| `"pinouts"` | object: pinout name → array of pins (below); at least one |
| `"parts"` | array of parts |

**Every other key belongs to the kind** (see the kind sections below), and
each kind rejects keys it does not know.

## Pinouts and pins

A pinout is an array of pins. Each pin has a **designator**, the package's
own label for it, and a **position**, its place in the array (from 0), which
is the pin's index in the simulation. A pinout is one of two styles; every
pin of a pinout uses the same style.

- **Numbered** (DIP, SOIC, SOT, QFN, LCCC, …): each pin has `"number"`, and
  the array is exactly 1, 2, … N in order. The designator is the number
  (`"14"`).
- **Ball** (BGA, CSP grids): each pin has `"ball"`, the ball name as the
  datasheet prints it (`"A1"`, `"H6"`). The designator is the ball name.

```json
{"number": 1, "name": "1A", "role": "input"}
{"number": 9, "name": "A0", "role": "input", "group": "address"}
{"ball": "C3", "name": "NC", "role": "nc"}
```

| Key | Value |
|---|---|
| `"number"` | numbered pinouts: the package pin number; the array must be 1, 2, … N |
| `"ball"` | ball pinouts: the ball name, used once per pinout |
| `"name"` | datasheet pin name |
| `"role"` | `input`, `output`, `inout`, `power`, `ground`, `nc` (no connect) or `absent` (a numbered position with no lead; never drawn, nothing can be wired to it) |
| `"group"` | optional, default `"default"`; pins in one group are drawn together (e.g. `"address"`) |

A pin has `"number"` or `"ball"`, never both. Ball pinouts:

- **Ball names** are 1–2 row letters from `ABCDEFGHJKLMNPRTUVWY` (JEDEC JEP95
  rows: no I, O, Q, S, X or Z), then a column 1–99 with no leading zero:
  `A1`, `H6`, `R9`, `AB12`.
- **Order is free, and it is the position.** List the balls of a new device
  row-major (A1, A2, …, B1, …). A device that also has a numbered pinout may
  list its balls in that pinout's order instead, so both draw alike. Once a
  part ships, never reorder its balls: positions are what saved schematics
  and traces depend on.
- **No `absent`**: list only real balls and leave unpopulated grid positions
  out.

A ball pinout, the 5-ball DSBGA of a single gate, in the order of its SOT-23 pins:

```json
"dsbga": [
  {"ball": "A1", "name": "A", "role": "input"},
  {"ball": "B1", "name": "B", "role": "input"},
  {"ball": "C1", "name": "GND", "role": "ground"},
  {"ball": "C2", "name": "Y", "role": "output"},
  {"ball": "A2", "name": "VCC", "role": "power"}
]
```

**Kind keys refer to pins by name, never by number or ball**, so one device
can carry several pinouts (DIP and LCCC, DIP and TSOP-I, SOT-23 and DSBGA).
Every name a kind key uses must exist in every pinout of the device. A name
used by several pins means all of them, in pinout order; kinds that need
exactly one pin reject such names. (One exception, a part key: an
oscillator's `enablePin` is a pin designator; see below.)

## Parts

```json
{"name": "SN74HC00N", "package": "DIP", "pinout": "p14", "footprint": "PDIP-14", "family": "HC", "flags": ["obsolete"]}
```

| Key | Value |
|---|---|
| `"name"` | the orderable part number; unique across the whole library (all files) |
| `"package"` | the package family, shown in the drawer and used by its package filter, e.g. `"DIP"`, `"TSSOP"`, `"VSON"`; may be empty (supplies). Never put a pin count, body size or material in it (no `"CDIP"`, `"DIP-14"`, `"2520 VSON"`): the pin count comes from the pinout and the exact size from the footprint. Ceramic and plastic DIPs are both `"DIP"`, flat packs are `"CFP"`, leadless chip carriers `"LCCC"`, and small and full-size type I TSOPs are both `"TSOP-I"` (type II stays `"TSOP-II"`). |
| `"pinout"` | the name of one of the device's pinouts |
| `"flags"` | optional, any of `obsolete`, `military`, `automotive`, `enhanced`, `ceramic` |
| `"footprint"` | the `id` of a footprint in `footprints.json` (see [Footprints](#footprints)). Required on every part except supplies (`VCC`, `GND`, `5V`, …), which reject it |
| `"connectorFootprint"` | optional: a second footprint, the `id` of one in `footprints.json`, that the PCB Style option switches to (**Connector Only**). Same rules as `footprint`. Character LCD modules use it (§ `charlcd`) |
| `"footprintChoices"` | optional: the footprints a placed part may use, each `{"package": …, "footprint": …}`, in picker order. The part's own `footprint` is one of them (the default), with the part's `package`. `GENERIC-LED` uses it (§ `led`; rules in [The `"footprint"` part key](#the-footprint-part-key)) |
| `"vcc"`, `"inputMax"` | on gate, dff, jkff, sram, eeprom and oscillator parts only: the part's own supply range / input limit, replacing the one it inherits (see [Supply voltage](#supply-voltage-vcc-and-inputmax)) |

Every other key belongs to the kind.

Flags:

- `obsolete`: the datasheet's ordering table lists the part as Obsolete.
- `military`: SN54/SNJ54/SNV54/CD54 names, 5962- and (J)M38510/ numbers, SMD
  drawing numbers, CD4000 …F3A parts.
- `automotive`: AEC-Q100 qualified (-Q1) parts.
- `enhanced`: Enhanced Product (-EP) parts, V62/ numbers.
- `ceramic`: a ceramic package body (CDIP, CFP, LCCC, CPGA, ceramic SMD
  oscillators). Ceramic and plastic DIPs share the package name `"DIP"`, so
  this flag is what tells them apart. Military parts are often, but not
  always, ceramic; set each flag on its own.

## Values

- **Times** are nanoseconds (JSON numbers, fractions allowed). The loader
  converts them to the engine's picoseconds with `llround(ns * 1000)`.
- **Logic families** (`family` on gate, dff, jkff and the sequential and
  buffer kinds' parts) must be one of:
  HC, LS, ACT, HCT, F, AC, AHCT, ALS, AS, AHC, S, C, H, HCS, LV, LVC, AUP,
  AUC, ALVC, TTL, ABT, BCT, LVT, AVC, TPIC, FCT, L, ALV, AXC, LXC, TXU, TXV. Of these:
  - `ABT`, `BCT`: BiCMOS bus-interface logic (5 V).
  - `LVT`: 3.3 V ABT; LVTH (bus-hold) parts use it too.
  - `AVC`: advanced very-low-voltage CMOS.
  - `TPIC`: TI power-logic parts with DMOS open-drain outputs.
  - `FCT`: TI (formerly Cypress, and Harris CD74FCT) fast CMOS bus-interface
    logic with TTL-level inputs, CY74FCT…T and CD74FCT parts (5 V).
  - `L`: low-power standard TTL (SN54L/SN74L).
  - `ALV`: advanced low-voltage CMOS (SN74ALV, 3.3 V).
  - `AXC`, `LXC`: dual-supply translating CMOS (AXC 0.65–3.6 V,
    LXC 1.1–5.5 V per side); AXCH/LXCH bus-hold parts use them too.
  - `TXU`, `TXV`: TI fixed-direction level translators (TXU0104, TXV0106 …).
- **`note`** is allowed on a device and on any timing entry; it is ignored.
- **Voltages** are volts (JSON numbers, fractions allowed). The loader
  converts them to millivolts (`uint32_t`) with `llround(v * 1000)`.

### Supply voltage (`vcc` and `inputMax`)

Chips carry their datasheet's Recommended Operating Conditions supply range,
for the voltage check (`docs/backend.md`). The keys go on the gate, dff and
jkff **timing entries**, the sram and eeprom **grades**, the oscillator
and charlcd **devices**, and on any **part** of those kinds:

| Key | Value |
|---|---|
| `"vcc"` | `[min, max]` volts: the recommended supply range, 0 < min < max ≤ 30 |
| `"inputMax"` | optional: the highest voltage an input (or inout) takes when it is above `vcc` max, e.g. `5.5` for 5 V-tolerant LVC inputs; at least the resolved `vcc` max, at most 30. Without it an input takes at most the `vcc` max. |

Resolution, per part:

1. The base is the part's timing entry (gate, dff, jkff), grade (sram,
   eeprom) or device (oscillator, charlcd): its `vcc` and `inputMax`, each
   optional.
2. The part's own `vcc` replaces the base range; its own `inputMax` replaces
   the base input max. A part that overrides only `vcc` keeps the base's
   `inputMax` if one was given, otherwise its input max is its own `vcc`
   max.
3. `inputMax` with no `vcc` at that level or below it is an error
   (`"inputMax" without "vcc"`), and so is an `inputMax` below the resolved
   `vcc` max.
4. `Part.vcc` points at the resolved `PartVoltage` (`min_mv`, `max_mv`,
   `input_max_mv`); parts with no part-level key share their entry's.

Every gate, dff, jkff, sram, eeprom, oscillator and charlcd part must resolve to a
`vcc` range (a part with none is rejected: `missing "vcc"`). Supplies,
LEDs, 7-segment displays and switches never take these keys; their
`Part.vcc` is NULL.

#### Two supply domains (`domains`, `vccB`, `inputMaxB`)

A dual-supply part (a level translator: SN74LVC1T45, SN74AVC8T245,
TXU0104, CD4504B) has two supply domains, each powered from its own pin
and judged against its own range. Today only the `buffer` kind's model
`translator` takes these keys; the `Part` fields are general.

| Key | Where | Value |
|---|---|---|
| `"domains"` | device | exactly 2 objects `{"power": <pin name>, "pins": [<pin names>]}`: the A side first, then the B side. `power` names the domain's supply pins (every pin of that name, e.g. two `VCCB`; each must have role `power`); `pins` lists the domain's other pins. Every pin of every pinout except ground, NC and absent pins belongs to exactly one domain (`"domains": pin "DIR" is in no domain`, `… is in two domains`). Control pins go to the side the datasheet references them to (usually A). |
| `"vccB"`, `"inputMaxB"` | timing entry, part | the B side's range and input max, with the rules of `vcc`/`inputMax` above. Every timing entry of a device with `domains` needs `vccB`; without `domains` both keys are errors. `vcc`/`inputMax` are the A side's. |

`Part.domains` (`PartSupplyDomain`: power pins, `vcc`, other pins; indices
into `Part.pins`) has `domain_count` 2: domain 0 is the A side, whose `vcc`
is `Part.vcc`; domain 1 the B side. Single-supply parts have
`domain_count` 0. The listing line of a dual-supply part ends in
`|vccb=<min>-<max>,in=<inputMax>` (millivolts) after its `|vcc=` field.

## Loading rules

- A key appearing twice in one object is an error.
- Part names are unique across the whole library (across files); device ids
  are unique within a kind.
- A failed `load_parts` adds nothing from that file; parts from earlier files
  stay.

## Footprints

`Parts/footprints.json` holds every land pattern, by name; a part names
one with `"footprint": "<id>"`. The loader expands each footprint into a
flat list of pads, so the app only draws rectangles and ellipses.

```json
{
  "format": "schematical-footprints",
  "version": 1,
  "footprints": [
    {
      "id": "SOIC-14",
      "description": "14-pin SOIC, 3.9 mm body, 1.27 mm pitch",
      "note": "TI D0014A example board layout (sn74hc00.pdf p.24)",
      "style": "dual",
      "pins": 14,
      "body": {"width": 3.9, "height": 8.65},
      "pitch": 1.27,
      "span": 5.4,
      "pad": {"width": 1.55, "height": 0.6, "shape": "rect"}
    }
  ]
}
```

The file allows only `format` (`"schematical-footprints"`), `version`
(`1`) and `footprints`.

**Units and axes.** Millimetres, JSON numbers. A footprint's origin is the
centre of its body; x grows right, y grows down. Pin 1 is at the top of the
left side. Pads are axis-aligned.

### Common keys

| Key | Value |
|---|---|
| `"id"` | unique name, `A-Z`, `0-9`, `.`, `-` |
| `"description"` | one line for people |
| `"note"` | optional provenance (drawing code, file, page); ignored |
| `"style"` | `"dual"`, `"quad"`, `"grid"` or `"explicit"` |
| `"pins"` | number of designated pads (1–1000). `dual`, `quad` and `explicit` pads are `"1"`…`"<pins>"`; `grid` pads are ball names |
| `"body"` | `{"width", "height"}` > 0, optional `"round": true` (drawn as an ellipse) |
| `"pad"` | `{"width", "height", "shape"}`, sizes > 0, shape `"rect"` or `"round"` (an ellipse; a circle when width = height), plus optional corner keys (below). Not on `explicit` |
| `"drill"` | optional, default 0 (surface mount); a through-hole diameter, smaller than both pad sides. Not on `explicit` |
| `"pin1Shape"` | optional, `dual`/`quad` only: pin 1's pad shape (through-hole parts use `"rect"` to mark pin 1); pin 1 then has square corners |
| `"centre"` | optional, `dual`/`quad` only: one pad at the origin, `{"width", "height", "shape"}` plus optional corner keys — an exposed pad, or the middle terminal of an X2SON-5 |
| `"slots"` | optional, `dual`/`quad` only: `slots[i]` = the slot (1-based) pin i+1 sits on; `0` = the pin is the `centre` pad. Default: pin n on slot n |

`slots` has exactly `pins` entries; non-zero values are distinct and no
larger than the slot count; at most one `0`, and only with `centre`. A slot
no pin names gets no pad. A `centre` no pin names is an unnumbered pad
(designator `NULL`), listed last.

### Rounded and cut corners

A `"rect"` pad (in `pad`, `centre` or an `explicit` pad) may take one of:

| Key | Value |
|---|---|
| `"radius"` | > 0, at most half the pad's shorter side: every corner rounded by this radius |
| `"chamfer"` | > 0: the corners in `"corners"` cut off at 45°, this far along each edge. On every side, chamfer × (cut corners at its ends) is at most the side's length |
| `"corners"` | with `chamfer` only: a non-empty array of distinct `"tl"`, `"tr"`, `"br"`, `"bl"` |

Not both, and neither on a `"round"` pad. Corners are as seen on the board
(top view, y down), before the part is rotated; the app turns them with
the part.

- `centre` and `explicit` pads: the corners as named.
- `pad` of a `dual`, `quad` or `grid`: name the corners of a pad up and to
  the left of the body centre. Every other pad gets them mirrored, so the
  footprint is symmetric about both centre lines: a pad right of centre
  swaps left and right (`tl` ↔ `tr`, `bl` ↔ `br`), a pad below swaps top
  and bottom, and a pad on a centre line gets both. So `["br"]` cuts the
  corner facing the centre on every pad. A quad's turned top and bottom
  pads follow the same rule, by position; the corner names do not turn
  with the pad.

```json
"pad": {"width": 0.42, "height": 0.22, "shape": "rect", "chamfer": 0.16, "corners": ["br"]},
"centre": {"width": 0.354, "height": 0.354, "shape": "rect", "chamfer": 0.177,
           "corners": ["tl", "tr", "br", "bl"]}
```

X2SON-5-DPW: each signal pad's inner corner is cut, and the centre land, a
0.25 square turned 45°, is a 0.354 square cut by half on every corner.

### `dual` — two columns

DIP, SOIC, SOP, SSOP, TSSOP, SOJ, TSOP, SOT-23, SC-70, SON, chip LEDs.

| Key | Value |
|---|---|
| `"pitch"` | > 0, pad-centre distance along a column |
| `"span"` | > 0, centre-to-centre distance between the two columns |
| `"perSide"` | optional, ≥ 1, default `pins / 2`; required when `pins` is odd |

Slot count = 2 × perSide. Slots run counter-clockwise: down the left
column, then up the right one. Without `slots`, `pins` must fit.

```
 1 ▯   ▯ 6        SOT-23-5: perSide 3, slots [1,2,3,4,6]
 2 ▯   ▯ 5        pins 1-3 on slots 1-3, pin 4 on slot 4,
 3 ▯   ▯ 4        pin 5 on slot 6; slot 5 has no pad
```

```
slot s ≤ P:  x = −span/2,  y = (s − 1 − (P−1)/2) · pitch
slot s > P:  x = +span/2,  y = ((2P − s) − (P−1)/2) · pitch
```

### `quad` — four sides

LCCC, QFN.

| Key | Value |
|---|---|
| `"sides"` | `[left, bottom, right, top]` slot counts, whole numbers, at least one slot in total |
| `"pitch"` | > 0 |
| `"spanX"` | > 0, centre distance of the left and right columns |
| `"spanY"` | > 0, centre distance of the top and bottom rows |

Slots run counter-clockwise: left side top→bottom, bottom left→right, right
bottom→top, top right→left; each side's pads are centred on it. Left and
right pads are `pad.width × pad.height`; top and bottom pads are turned
(`pad.height × pad.width`).

```
      16 15 14 13           sides [4,4,4,4]: slot numbers,
   1 ▯          ▯ 12        left 1-4, bottom 5-8,
   2 ▯          ▯ 11        right 9-12, top 13-16
   3 ▯          ▯ 10
   4 ▯          ▯ 9
       5  6  7  8
```

### `grid` — ball grid

BGA, DSBGA. `slots`, `centre` and `pin1Shape` are not allowed.

| Key | Value |
|---|---|
| `"rows"` | 1–20 |
| `"columns"` | 1–30 |
| `"pitch"` | > 0 |
| `"balls"` | the populated ball names, `pins` of them, any order |

Rows are letters A, B, C, … skipping I, O, Q, S, X, Z (row A at the top);
columns are 1…`columns` (column 1 at the left), as on a top-view land
pattern. Each ball must be a ball name inside the grid, used once. A pad's
designator is its ball name — the same name the part's ball pinout uses —
so a grid needs no pin mapping. Pads are listed row by row.

```
column c, row index r (A = 0):  x = (c − 1 − (columns−1)/2) · pitch
                                y = (r − (rows−1)/2) · pitch
```

### `header` — pin header

Through-hole pin headers, one or two rows, numbered zigzag: pin 2 is across
the rows from pin 1, then 3 and 4 in the next column. `slots` and `centre`
are not allowed.

| Key | Value |
|---|---|
| `"rows"` | 1 or 2; `pins` must be a multiple of it |
| `"pitch"` | > 0, pad-centre distance along a row |
| `"rowPitch"` | non-zero, row 2's y minus row 1's; required with 2 rows, not allowed with 1 |
| `"firstRowY"` | y of row 1 (the row with pin 1) from the body centre |
| `"pad"` | pad size and shape, as on `dual` |
| `"drill"` | optional, > 0 |
| `"pin1Shape"` | optional, as on `dual` |

perRow = pins / rows. Pin p (1-based) is in column c = (p − 1) / rows and
row k = (p − 1) % rows:

```
x = (c − (perRow − 1)/2) · pitch
y = firstRowY + k · rowPitch
```

```
 1 ■  3 ●  5 ●  7 ●       2x4, rowPitch > 0: row 1 above row 2
 2 ●  4 ●  6 ●  8 ●
```

A right-angle header's body is its housing; `firstRowY` puts the holes
beside it (the pins past the housing are not drawn).

### `explicit` — literal pads

For land patterns the parametric styles cannot express (mixed pad sizes).
`"pads"` is a non-empty array of `{"pin", "x", "y", "width", "height",
"shape"}` plus optional `"drill"` and corner keys, in the coordinates above. Every pin
1…`pins` has at least one pad. A pin may have several (the same
connection in more places, e.g. a module's two header rows); they are
listed together, in file order. The app draws each and uses the pin's
first pad for the pin-1 mark and the `Pad 1 Center` origin. `"pin": 0` is
an unnumbered pad (any number of them). `pad`, `drill`, `slots`, `centre` and `pin1Shape` are not
allowed at the footprint level.

### The `"footprint"` part key

- Every part names one, except supplies: a non-supply part without
  `"footprint"` is rejected (`missing "footprint"`), and so is a supply
  with one (`a supply has no footprint`).
- Every footprint in the library lives in `footprints.json`. A new part
  reuses one whose land pattern matches its package drawing; otherwise add a
  footprint from the datasheet's package drawing, using the pad rules R1–R6
  (`planning/completed/plan-03-B-logic-footprints.md` §3: the board layout
  if the datasheet gives one, else the outline rules), with a `note` naming
  the drawing, file and page. `footprint_check` fails on a footprint no part
  uses (as its `footprint`, `connectorFootprint` or one of its
  `footprintChoices`).
- `"connectorFootprint"` follows the same rules (unknown id, pad count,
  a pad per pin; messages name `connectorFootprint`), and naming a
  footprint there counts as a use for `footprint_check`.
- `"footprintChoices"` is a non-empty array of objects with exactly the keys
  `package` (non-empty string) and `footprint` (a footprint id); anything
  else fails with `"footprintChoices" must be a non-empty array of
  {"package", "footprint"}`. Each footprint must exist
  (`unknown footprintChoices footprint "X"`) and have a pad for every pin
  (`pin "2" has no pad in footprintChoices footprint "X"`). Unlike
  `footprint`, a choice **may have more pads than the part has pins**: the
  extra pads are unconnected copper (`GENERIC-LED`'s `LED-DIP4` choice has
  pads 3 and 4 with no pin). No footprint may be listed twice
  (`footprintChoices footprint "X" listed twice`). The part's own
  `footprint` must be a choice (`"footprint" "X" is not in
  footprintChoices`) whose `package` equals the part's (`"package" "X" does
  not match footprintChoices "Y" ("Z")`). A supply rejects the key (`a
  supply has no footprint`). The app saves each placed part's choice in the
  schematic file (`docs/file-format.md` § Parts).
- **Mid-values.** Every dimension that a rule takes from a drawing (lead
  width and length, tip-to-tip span, terminal size, ball and pin
  diameter, body) is the mid-value (min + max) / 2 when the drawing gives
  both a min and a max, even where plan-03-B's rule text says "max". A
  dimension with only one bound, a BSC value or a single value is used as
  printed. The `note` records the mid-value and its range, e.g.
  `lead 0.46 mid (0.38-0.53)`.
- **No overlapping pads.** If a land pattern's max-only pad sizes make
  pads touch (PLCC-32's corner pads at Y1 = .100 in max), shorten the pads
  and keep their centres, and say so in the `note`.

- The footprint must already be loaded: `load_footprints` runs before any
  `load_parts` (the app and the tests load `footprints.json` first; a
  missing `footprints.json` is an error).
- The footprint's `pins` must equal the pinout's pin count, and every pin's
  designator (`"14"`, `"A1"`) must have a pad of that designator. Pins are
  matched to pads by designator, never by position.
- A part with `absent` pins keeps their pads in the footprint; the app does
  not draw a pad for an absent pin.

No other key is allowed anywhere in the file, and a key appearing twice is
an error. Messages name the place, e.g.
`footprints.json: footprints[4] (SOT23-5): slots: slot 7 is beyond 6 slots`.

## Kinds

### `supply`

Constant sources. Device key:

| Key | Value |
|---|---|
| `"volts"` | the supply's voltage, 0 to 30. More than 0 drives its pin to 1, strong; 0 (ground) drives 0, strong. `PartSupply.mv` holds it in millivolts and `PartSupply.level` is `mv > 0`. |
| `"pull"` | optional, `true` or `false` (default `false`). `true` makes the device a pull part: a pull-up for `volts` > 0, a pull-down for 0. `PartSupply.pull` is 1. |

The device has one pinout with one pin. Parts carry no kind keys. Each
device's single part has the same name as its `id` and `function`.

```json
{"id": "3V3", "function": "3V3", "description": "Supply, 3.3 V, drives logic 1", "drawer": [],
 "pinouts": {"pin": [{"number": 1, "name": "VCC", "role": "power"}]},
 "volts": 3.3,
 "parts": [{"name": "3V3", "package": "", "pinout": "pin"}]}
```

The library's supplies (`supply.json`):

| Part | `volts` | Pin |
|---|---|---|
| `VCC` | 5 | `VCC` (power) |
| `5V` | 5 | `VCC` (power) |
| `3V3` | 3.3 | `VCC` (power) |
| `2V5` | 2.5 | `VCC` (power) |
| `1V8` | 1.8 | `VCC` (power) |
| `1V2` | 1.2 | `VCC` (power) |
| `12V` | 12 | `VCC` (power) |
| `GND` | 0 | `GND` (ground) |
| `PULLUP-5V` | 5, pull | `PU` (output) |
| `PULLUP-3V3` | 3.3, pull | `PU` (output) |
| `PULLUP-2V5` | 2.5, pull | `PU` (output) |
| `PULLUP-1V8` | 1.8, pull | `PU` (output) |
| `PULLUP-1V2` | 1.2, pull | `PU` (output) |
| `PULLUP-12V` | 12, pull | `PU` (output) |
| `PULLDOWN` | 0, pull | `PD` (output) |

A pull part drives its pin **weakly**: 1 for a pull-up, 0 for a pull-down.
Any strong driver on the net wins over it, so open-drain outputs and I2C
lines work, and a strong 0 against a pull-up is not a short. A pull-up and
a pull-down on one net read X (equal strength). Its pin has role `output`,
because it drives a signal net, not a power net. In the voltage check a
pull-up gives its net a level only when no supply and no powered output
sets that net, and a pull never clashes with a supply or another pull.

### `led`

A single LED. It lights while its anode reads 1 and its cathode reads 0;
`PartLed` and the lights hook report it (on at any time during the last
step). Device keys:

| Key | Value |
|---|---|
| `"colors"` | the colors a placed LED may have, a non-empty array of strings, e.g. `["Red","Green","Blue","Amber","White"]`; the first is the default. Each placed LED keeps its own color in the schematic file |
| `"anode"` | pin name of the anode (exactly one pin) |
| `"cathode"` | pin name of the cathode (exactly one pin) |

Parts carry no kind keys. The library has one LED part, `GENERIC-LED`, whose
`footprintChoices` list the eleven LED footprints (0402 … Radial; default
`LED-RADIAL-5MM`).

### `oscillator`

A clock source; the primary output toggles every half period. Device keys:

| Key | Value |
|---|---|
| `"family"` | oscillator family as shown, e.g. `"LMK6C"` (not a logic family) |
| `"minHz"`, `"maxHz"` | the frequency range the family can produce |
| `"outputs"` | `[primary]` or `[primary, complement]`, pin names (one pin each) |
| `"vcc"`, `"inputMax"` | the device's supply range (see [Supply voltage](#supply-voltage-vcc-and-inputmax)) |

Part keys:

| Key | Value |
|---|---|
| `"defaultHz"` | the part's frequency, from `minHz` to `maxHz` |
| `"vcc"`, `"inputMax"` | optional: the part's own range, for a supply option picked by a letter in the part number (LMK6 1.8 V parts) |
| `"enablePin"` | optional; the **designator** (in the part's pinout) of the output-enable / standby pin: a number for numbered pinouts (`2`), a ball name string for ball pinouts (`"A2"`). A LOW there puts every output in high impedance; HIGH or not connected keeps the clock running. A designator, not a name, because the LMK6D/H/P function pins 1 and 2 share the name `OE/ST/NC`. `PartOscillator.enable_pin` points at that pin's `PartPin.designator` (NULL = none). |

### `gate`

Combinational gates: every 74xx/CD4000 gate part. Each gate computes its op
over its inputs (X for unknown inputs unless a controlling value decides:
a 0 into AND/NAND, a 1 into OR/NOR) and drives its output strong after the
rise delay for 1, the fall delay for 0, or the longer of the two for X. A
newer drive cancels one still pending (inertial delay), so pulses shorter
than the delay are filtered out.

```json
"gates": [{"op": "nand", "inputs": ["1A", "1B"], "output": "1Y"}],
"output": "push-pull",
"units": 1,
"timing": {
  "HC": {"rise": 9, "fall": 9, "note": "Typical at 4.5 V, 50 pF"},
  "LS": {"otherLow": {"rise": 12, "fall": 10}, "otherHigh": {"rise": 20, "fall": 13}}
}
```

Device keys:

| Key | Value |
|---|---|
| `"gates"` | array of gates: `op` (`and`, `nand`, `or`, `nor`, `xor` = odd parity, `xnor` = even parity, `not` = inverter and `buf` = buffer (Y = A; X or Z in gives X), each exactly 1 input), `inputs` (1–13 pin names), `output` (a pin name). Each name must be exactly one pin in every pinout. |
| `"output"` | optional: `push-pull` (default) or `open-drain` (a 1 releases the pin to Z after the rise delay; an external pull-up takes it high; 74x266, 74x05/06) |
| `"units"` | optional: gates per package as shown in the drawer (`PartLogic.units`); default the number of `gates`. Differs only where one gate has two outputs (CD4068B: NAND and AND of the same 8 inputs, 1 unit). |
| `"timing"` | object: logic family → `{rise, fall}` (ns), or `{otherLow: {rise, fall}, otherHigh: {rise, fall}}`; either form also takes `vcc` and `inputMax` at its top level (not inside `otherLow`/`otherHigh`) |

`otherLow`/`otherHigh` (74x86): the delays depend on the input that did not
change — 0 picks `otherLow`, 1 picks `otherHigh`, and X (also at start, when
no edge has happened yet) takes the longer of each delay. Allowed only when
every gate has exactly 2 inputs.

Part key:

| Key | Value |
|---|---|
| `"family"` | the part's logic family; must be a key of `timing` |
| `"vcc"`, `"inputMax"` | optional: the part's own range when its datasheet row differs from the entry's (SN54 vs SN74, CD vs SN) |

### `dff`

D flip-flops. Each unit captures D on its clock edge while PRE and CLR are
inactive; PRE/CLR act at once. An undriven (Z) PRE/CLR reads as inactive,
X as unknown. Power-on state is Q = 0, QN = 1. Edges follow Verilog
posedge/negedge: any move into 1 (or 0) from another value, including from
X or Z. Outputs are driven strong, inertially: fall delay for 0, rise delay
for 1 (and Z), the longer for X.

```json
"units": [{"d": "1D", "clk": "1CLK", "pre": "1PRE", "clr": "1CLR", "q": "1Q", "qn": "1QN"}],
"outputEnable": "OE",
"presetActiveHigh": false,
"clearActiveHigh": false,
"clockEdge": "rising",
"timing": {"HC": {"rise": 20, "fall": 20, "clearWins": false, "note": "…"}}
```

Device keys:

| Key | Value |
|---|---|
| `"units"` | 1–8 flip-flops: `d` and `clk` required; `pre`, `clr`, `q`, `qn` optional (absent = no such pin). Units may share pins (a common clock or clear). |
| `"outputEnable"` | optional, active-low output enable: 1 puts Q/QN in Z, X makes them X |
| `"output"` | optional: `push-pull` (default) or `open-drain`: a 0 drives 0 after the fall delay, a 1 (or Z) releases Q/QN to Z after the rise delay, X drives X; the voltage check treats Q/QN as carrying no voltage (TPIC6273) |
| `"presetActiveHigh"`, `"clearActiveHigh"` | `true` when PRE / CLR is asserted high |
| `"clockEdge"` | `"rising"` or `"falling"` |
| `"timing"` | logic family → `{rise, fall, clearWins}`, plus `vcc` and `inputMax`; `clearWins` true: PRE and CLR both asserted give Q = 0, QN = 1; false: Q = QN = 1 |

Part keys: `"family"`, a key of `timing`, and optional `"vcc"` and
`"inputMax"`. `PartLogic.units` is the number of
units.

### `jkff`

JK flip-flops. On its clock edge (PRE and CLR inactive) a unit sets (J),
resets (K), holds or toggles (both). An unknown J or K gives X unless every
level it could have leads to the same Q. PRE/CLR as for `dff`; both
asserted give Q = QN = 1.

Device keys: as `dff`, except `"units"` (1–2) take `j`, `k`, `clk`
(required) and `pre`, `clr`, `q`, `qn` (optional); there is no
`outputEnable`; `"kInverted"` (bool) means the K input is K-bar (74x109);
timing entries are `{rise, fall}` plus `vcc` and `inputMax`. Part keys:
`"family"`, and optional `"vcc"` and `"inputMax"`.

### `switch`

DIP switches. A switch drives nothing; each position is a contact between
two pins. While a position is ON the nets on its two pins act as one net
(both ways; strong drivers that disagree across it short). Every position
starts OFF; the app turns positions on and off (`circuit_set_switch`).

```json
"actuator": "Low profile",
"contacts": [["1A", "1B"], ["2A", "2B"], ["3A", "3B"], ["4A", "4B"]]
```

| Key | Value |
|---|---|
| `"actuator"` | actuator style as shown, e.g. `"Piano actuator"` |
| `"contacts"` | position 1, 2, …: `[pin name, pin name]`, two different pins (exactly one pin each) |
| `"ratedVolts"` | the contacts' **switching** voltage rating (the "Contact rating, switching" row, not the non-switching carry rating), 0 < v ≤ 1000; `PartSwitch.rated_mv`. Required. |

`PartSwitch.positions` is the number of contacts. Parts carry no kind keys.

### `connector`

Barrel power connectors are passive `inout` terminals. `centerPin` is the
mating plug centre-pin diameter in millimetres; `ratedVolts` and `ratedAmps`
are the connector ratings. A two-terminal jack has centre and sleeve pins.
A switched jack additionally names `"contacts": [["Sleeve", "Switch"]]`:
the listed contact is normally closed while unplugged and opens when a plug
is inserted through `circuit_set_connector_plug`.

### `header`

A generic pin header: passive `inout` terminals, each pin just its net (the
model drives nothing). Device keys:

| Key | Value |
|---|---|
| `"rows"` | 1 or 2 |
| `"genders"` | the genders a placed header may have, a non-empty array of strings, e.g. `["Male","Female"]`; the first is the default. The app shows and saves the gender; the backend does not act on it |

Parts carry no kind keys. Every pinout's pin count must be at least 1 and a
multiple of `rows` (`a header's pin count must be a multiple of its rows`).

The library's headers are `Parts/header.json`, written by
`node scripts/gen-headers.js` (kept; running it twice changes nothing): 80
parts named `GENERIC-HEADER-<R>X<NN>` (R = 1 or 2 rows, NN = 01 … 40 pins
per row), in devices `header-1row` and `header-2row`. Pinout `<r>x<nn>`
lists pins 1 … R·NN in order, named by number. Each part's
`footprintChoices` are `Straight` (`PINHDR-<R>X<NN>-V`, the default) and
`Right angle` (`PINHDR-<R>X<NN>-RA`); the script also writes those 160
`header`-style footprints (every `PINHDR-` line of `footprints.json`) from
the Würth WR-PHD drawings in `datasheets/connector-header/`.

### `sram`

Asynchronous SRAMs. Every instance has its own storage (all X at power-on).
The chip is selected while every `chipEnable` (active low) is low and every
`chipEnableHigh` is high. It writes while selected with WE# low and the
lane's byte enable low; data and address are taken at the end of the write.
It reads while selected with OE# low, WE# high and the byte enable low. An
undriven (Z) control reads inactive; X unknown. Read data appears inside the
datasheet window (tCLZ/tOLZ/tBLZ/tOW … tAA/tACE/tOE/tBA … tCHZ/tOHZ/tBHZ/tWHZ,
old data held for tOH); write timing (tWP, tCW, tBW, tAW, tAS, tDW, tDH, tWR)
is checked and a violation stores X.

```json
"address": ["A0", "A1", "A2"],
"data": ["DQ0", "DQ1"],
"chipEnable": ["CE#"],
"chipEnableHigh": ["CE2"],
"outputEnable": "OE#",
"writeEnable": "WE#",
"byteEnable": ["LB#", "UB#"],
"grades": {"55": {"trc": 55, "taa": 55, "tace": 55, "toe": 30, "note": "…"}}
```

| Key | Value |
|---|---|
| `"address"` | pin names; `address[i]` is address bit i (1–24) |
| `"data"` | pin names; `data[i]` is data bit i (1–32) |
| `"chipEnable"` | active-low chip enables (0–2) |
| `"chipEnableHigh"` | active-high chip enables (0–2), may be `[]` |
| `"outputEnable"`, `"writeEnable"` | OE#, WE# (active low) |
| `"byteEnable"` | active-low byte enables, `byteEnable[i]` = lane i (DQ 8i … 8i+7); `[]` = the whole word is one lane |
| `"grades"` | speed grade → timing in ns: `trc taa tace toe tba toh tclz tolz tblz tchz tohz tbhz twc taw tcw tbw tas twp twpOeLow twr tdw tdh twhz tow`; a missing name is 0. A grade also takes `note`, `vcc` and `inputMax`. |

Part keys: `"grade"`, a key of `grades`, and optional `"vcc"` and `"inputMax"`. `PartSram`: `words` = 2^len(address),
`bits` = len(data), `access_ps` = the grade's `taa`, `grade`.

### `eeprom`

Parallel EEPROMs (byte-wide, page-write; `backend/src/models/eeprom.c`).
Every instance has its own storage, erased (every byte FF) at power-up.
The controls are active low; an undriven (Z) control reads inactive, X
unknown.

- **Reads** use the SRAM read window with no low-Z delay: with CE# and OE#
  low and WE# high the outputs drive X, then data at the latest of
  address + tACC, CE# + tCE and OE# + tOE; old data holds tOH after an
  address change; X for tDF after the read ends, then high-Z.
- **Loads.** A load runs while CE# and WE# are low and OE# is high. The
  address is latched when it starts, the data when it ends. A load stores X
  when the pulse is shorter than tWP, the address is set up less than tAS
  before it or changes within tAH after it starts, the data is set up less
  than tDS before it ends, OE# went high less than tOES before it, or (after
  the first load of a window) it starts less than tWPH after the last one
  ended. Data or OE# changing within tDH / tOEH after the end also stores X.
- **Window and write cycle.** Loads that each start within tBLC of the last
  one's end form one window. tBLC after the last load the write cycle
  starts and runs for tWC; then the loaded bytes are stored all at once. The
  window's page is its first data byte's address with the low
  log2(`pageSize`) bits cleared; a data byte in another page makes every
  loaded byte of the first page X (and writes nothing in the other page).
  The same byte loaded twice keeps the later value.
- **During the write cycle** loads are ignored, and a read returns DATA#
  polling: I/O7 = the complement of bit 7 of the last byte loaded, I/O6 =
  the toggle bit (0 on the first read of the cycle, flipping on each read
  after it), I/O0–I/O5 X.
- **Software data protection.** With A and B the two `commandAddresses`, a
  window that starts with `A/AA, B/55, A/A0` turns protection on at the end
  of its cycle; one that starts with `A/AA, B/55, A/80, A/AA, B/55, A/20`
  turns it off (only when `protection` is `"off"`). The rest of such a
  window is data; command bytes are never data, and loads that match part
  of a sequence and then differ are data. While protection is on, a window
  that starts with neither sequence writes nothing, but its cycle still
  runs. Protection is off at power-up for `"off"` and always on for
  `"always"`.
- **Unknown write enable.** CE#, WE# or OE# making a load X stores X at the
  address (every word matching its known bits when some are X), as part of
  the window.

Not modeled: the 12 V chip erase and device-ID modes, the power-on write
inhibit, the write-pulse noise filter.

```json
"address": ["A0", "A1", "…", "A14"],
"data": ["I/O0", "I/O1", "I/O2", "I/O3", "I/O4", "I/O5", "I/O6", "I/O7"],
"chipEnable": "CE#", "outputEnable": "OE#", "writeEnable": "WE#",
"pageSize": 64,
"commandAddresses": ["5555", "2AAA"],
"protection": "off",
"grades": {"15": {"tacc": 150, "tce": 150, "toe": 70, "tdf": 50, "tah": 50, "twp": 100,
                  "tds": 50, "twph": 50, "tblc": 150000, "twc": 10000000, "note": "…"}}
```

| Key | Value |
|---|---|
| `"address"` | pin names; `address[i]` is address bit i (1–24) |
| `"data"` | exactly 8 pin names; `data[i]` is data bit i |
| `"chipEnable"`, `"outputEnable"`, `"writeEnable"` | one pin name each: CE#, OE#, WE# (active low) |
| `"pageSize"` | bytes per page: a power of two from 1 to 2^len(address) |
| `"commandAddresses"` | two hex strings (`[0-9A-F]+`), each below 2^len(address): the SDP addresses A and B |
| `"protection"` | `"off"` (SDP starts off; can be turned on and off) or `"always"` (always on; no disable) |
| `"grades"` | speed grade → timing in ns: `tacc tce toe tdf toh tas tah tcs tch twp twph tds tdh toes toeh tblc twc` (a missing name is 0; `tblc` and `twc` must be more than 0) and an optional `note`, `vcc` and `inputMax`. `tcs` and `tch` are read but not checked (zero on every part) |

Part keys: `"grade"`, a key of `grades`, and optional `"vcc"` and `"inputMax"`. `PartEeprom`: `words` =
2^len(address), `bits` = 8, `access_ps` = the grade's `tacc`, `grade`,
`page_bytes` = `pageSize`, `write_cycle_ps` = the grade's `twc`.

### `cpu`

Microprocessors that run programs inside the simulation (plan-09;
`backend/src/models/cpu.c`, one adapter per core). The device's `"core"`
picks the bus adapter: `"6502"` (NMOS 6502) and `"65c02"` (WDC W65C02S),
`backend/src/models/cpu6502.c`, and `"z80"` (Zilog Z84C00),
`backend/src/models/cpuz80.c`. Each adapter wraps a vendored, unmodified
floooh/chips core (`backend/src/third_party/chips/`) and drives the pins at
the datasheet's times. `circuit_cpu_state` reads the state back (status,
cycle count, registers, the last instruction).

`cpu.json` holds four devices: `W65C02S` (2 parts), `W65C816S` (2 parts,
plan-14-I), `MCS6502` (the 4 plastic MPS6502 parts, obsolete) and `Z84C00`
(13 parts, obsolete). The example below is the `W65C02S` device's keys and
grade.

```json
"core": "65c02",
"address": ["A0", "A1", "…", "A15"],
"data": ["D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"],
"clock": "PHI2", "rw": "RWB", "sync": "SYNC", "irq": "IRQB", "nmi": "NMIB",
"reset": "RESB", "so": "SOB", "rdy": "RDY", "phi1Out": "PHI1O", "phi2Out": "PHI2O",
"vectorPull": "VPB", "memoryLock": "MLB", "busEnable": "BE",
"grades": {"14": {"tcyc": 70, "tpwh": 35, "tpwl": 35, "tads": 30, "tah": 10, "tsync": 30,
                  "tmds": 25, "tdhw": 10, "tdsr": 10, "tpcs": 10, "tbvd": 25,
                  "vcc": [1.71, 5.25]}}
```

| Key | Value |
|---|---|
| `"core"` | `"6502"`, `"65c02"`, `"z80"` (Z80 keys below) or `"65816"` (W65C816S keys below) |
| `"address"` | exactly 16 pin names; `address[i]` is A*i* |
| `"data"` | exactly 8 pin names; `data[i]` is D*i* |
| `"clock"` | the clock input (NMOS `PHI0IN`, W65C02S `PHI2`) |
| `"rw"`, `"sync"` | outputs: R/W (1 = read), SYNC (1 = opcode fetch) |
| `"irq"`, `"nmi"`, `"reset"`, `"so"` | active-low inputs: IRQ, NMI, RES, SO |
| `"rdy"` | RDY (high = run): an input on the NMOS chip, an `inout` on the W65C02S (driven low during WAI) |
| `"phi1Out"`, `"phi2Out"` | the clock outputs (Φ1 OUT / Φ2 OUT, PHI1O / PHI2O) |
| `"vectorPull"`, `"memoryLock"`, `"busEnable"` | `65c02` only: VPB, MLB (active-low outputs), BE (input, high = bus driven) |
| `"grades"` | speed grade → timing in ns: `tcyc tpwh tpwl tads tah tsync tmds tdhw tdsr tpcs tbvd` (a missing name is 0; `tcyc`, `tpwh` and `tpwl` must be more than 0) and an optional `note`, `vcc` and `inputMax` |

Every key of the device's core is required, each control names exactly one
pin in every pinout, and a key of another core is an error (`"vectorPull"
is not a key of core "6502"`). Part keys: `"grade"`, a key of `grades`, and
optional `"vcc"` and `"inputMax"`. `PartCpu`: `core`, `grade`, `max_hz` =
floor(10^12 / `tcyc` in ps).

The timing names, each measured from:

| Name | From | Use |
|---|---|---|
| `tcyc` | — | minimum clock period (fall to fall) |
| `tpwh`, `tpwl` | — | minimum clock high and low widths |
| `tads` | PHI2 fall | address and R/W valid |
| `tah` | PHI2 fall | address and R/W held; X after it |
| `tsync` | PHI2 fall | SYNC valid |
| `tmds` | PHI2 rise | write data valid |
| `tdhw` | PHI2 fall | write data held, then Z |
| `tdsr` | before PHI2 fall | read data setup |
| `tpcs` | before PHI2 fall | setup for RDY, IRQ, NMI, RES, SO |
| `tbvd` | BE edge | BE to bus valid / Z |

Behaviour (the 6502 family). An undriven (Z) input reads inactive: IRQ,
NMI, RES and SO high, RDY high, BE high. X is unknown.

- **Power-up: LOST.** Address, data, R/W, SYNC, VPB and MLB are driven X.
  Only a reset leaves LOST. Φ1/Φ2 OUT follow the clock from the start, with
  no added delay (X stays X).
- **The clock.** A clock that reads X, a low phase shorter than `tpwl`, a
  high phase shorter than `tpwh`, or falls closer than `tcyc` make the CPU
  LOST.
- **The commit, at each PHI2 fall.** The CPU samples the data bus (on a
  read cycle) and RDY, IRQ, NMI, RES and SO. A pin that changed less than
  `tdsr` (data) or `tpcs` (controls) before the fall reads X. Then, in
  order:
  1. A write cycle's data goes Z at + `tdhw`.
  2. **Reset.** RES X: LOST. RES 0: status RESET; address X, data Z, R/W 1,
     SYNC 0, VPB 1, MLB 1 from + `tah`. RES back to 1 after at least 2
     cycles at 0: the core starts its reset sequence (reads at 0000, 0100,
     01FF, 01FE, then the vector at FFFC/FFFD); after fewer: LOST.
     RES is the one input whose setup violation is not X: a change less
     than `tpcs` before the fall (to 0 or 1) takes effect at the next fall,
     as an asynchronous reset switch would. RES itself X is still LOST.
  3. **RDY.** RDY X: LOST. RDY 0 holds the cycle (no change, the cycle
     count still grows) on a read, and on the W65C02S also on a write; the
     NMOS chip ignores RDY on a write.
  4. **SO.** SO falling since the last commit sets V. SO X: LOST unless V
     is already 1.
  5. **The tick.** The core runs one cycle on the sampled data, IRQ and
     NMI. When some of those are X it runs twice, with every X bit 0 and
     then 1; if the two results differ, the CPU is LOST (the unknown value
     mattered); if they agree it goes on. So dummy reads of uninitialised
     memory (the reset sequence's stack reads) are harmless, and a used
     unknown byte is not.
  6. **The next cycle.** Each address bit and R/W that changes goes X at
     + `tah` and to its value at + `tads`; SYNC likewise at + `tsync`. A
     write cycle's data goes X at the next PHI2 rise and valid at
     rise + `tmds`.
- **W65C02S extras.** VPB is low on the vector reads (FFFA–FFFF) of BRK,
  IRQ, NMI and reset. MLB is low on the three cycles of a read-modify-write
  (the read, the repeat read and the write at one address; ASL ROL LSR ROR
  DEC INC TSB TRB RMB SMB on memory). BE low floats address, data and RWB
  `tbvd` after the change (X while BE is X); the CPU keeps running, and BE
  high drives them again. WAI drives RDY low until an interrupt (IRQ, even
  with I set, or NMI) or reset ends it; STP stops the CPU until reset.
- **NMOS JAM.** The 12 JAM opcodes (02 12 22 32 42 52 62 72 92 B2 D2 F2)
  hang the CPU: status JAMMED until reset.
- **State** (`circuit_cpu_state`): status, the cycles since the last reset
  ended, registers `A X Y S P PC` (none while LOST or RESET), and the last
  completed instruction (address, bytes, text from the vendored
  disassembler, e.g. `LDA #$41`).

Not modeled: the NMOS 6502's minimum clock (50 kHz, a dynamic-logic limit);
the read-data hold time tDHR (data is sampled at the PHI2 fall); the
cycle-accurate IRQ pipeline while RDY holds the CPU; timing at supply
voltages other than the grade's (W65C02S: the 5 V column only); the exact
bus pattern of the 12 JAM opcodes, and the SHA/SHX/SHY page-cross address
(the vendored core's known deviations, plan-09-A §4.4).

**The Z80** (`"core": "z80"`, Zilog Z84C00; `backend/src/models/cpuz80.c`).
Keys, each naming one pin, all required, and each rejected on the 6502
family (as the 6502 family's are on the Z80):

| Key | Value |
|---|---|
| `"clock"` | CLK |
| `"m1"`, `"mreq"`, `"iorq"`, `"rd"`, `"wr"`, `"rfsh"`, `"halt"`, `"busack"` | outputs, active low: M1#, MREQ#, IORQ#, RD#, WR#, RFSH#, HALT#, BUSACK# |
| `"wait"`, `"int"`, `"nmi"`, `"reset"`, `"busreq"` | inputs, active low: WAIT#, INT#, NMI#, RESET#, BUSREQ# |
| `"grades"` | speed grade → timing in ns (the PS0178 AC rows below; `tcc`, `twch` and `twcl` must be more than 0), and an optional `note`, `vcc` and `inputMax` |

`"address"` has 16 names and `"data"` 8, as for the 6502 family.
`PartCpu.max_hz` = floor(10^12 / `tcc` in ps).

| Name | PS0178 row | Meaning |
|---|---|---|
| `tcc`, `twch`, `twcl` | 1–3 | clock period, high and low width (minimum) |
| `tdcra` | 6 | CLK rise → address valid |
| `tdcfmreqf`, `tdcrmreqr`, `tdcfmreqr` | 8, 9, 12 | CLK fall → MREQ# low; CLK rise → MREQ# high; CLK fall → MREQ# high |
| `tdcfrdf`, `tdcrrdr`, `tdcfrdr`, `tdcrrdf` | 13, 14, 23, 24 | CLK fall → RD# low; rise → RD# high; fall → RD# high; rise → RD# low (I/O) |
| `tsdcr`, `tsdcf` | 15, 25 | data setup to CLK rise (fetch, int-ack) / fall (memory and I/O reads) |
| `tswaitcf` | 17 | WAIT# setup to CLK fall |
| `tdcrm1f`, `tdcrm1r` | 19, 20 | CLK rise → M1# low / high |
| `tdcrrfshf`, `tdcrrfshr` | 21, 22 | CLK rise → RFSH# low / high |
| `tdcriorqf`, `tdcfiorqr`, `tdcfiorqf` | 27, 28, 51 | CLK rise → IORQ# low; fall → IORQ# high; fall → IORQ# low (int-ack) |
| `tdcfwrf`, `tdcfwrr`, `tdcrwrf` | 30, 32, 34 | CLK fall → WR# low; fall → WR# high; rise → WR# low (I/O) |
| `tdcfd` | 53 | CLK fall → write data valid |
| `tdcfhalt` | 36 | CLK fall → HALT# |
| `twnmi` | 37 | NMI# low width (minimum) |
| `tsbusreq`, `tsreset`, `tsint` | 38, 46, 48 | BUSREQ#, RESET#, INT# setup to CLK rise |
| `tdcrbusackf`, `tdcfbusackr` | 40, 41 | CLK rise → BUSACK# low; fall → BUSACK# high |
| `tdcrdz`, `tdcrctz`, `tdcraz` | 42–44 | CLK rise → data, control strobes, address float |

Behaviour (the Z80). Z inputs read inactive (high); X is unknown.

- **Power-up: LOST**, every output X. **RESET#** is sampled at each CLK
  rise: low → status RESET, address and data Z, every control output high;
  back high after at least 3 low rises → that rise is T1 of the fetch at
  0000 (after fewer: LOST); X → LOST. A change of RESET# less than `tsreset` before a
  rise takes effect at the next rise (as for the 6502 family's RES).
- **The clock.** A high phase shorter than `twch`, a low phase shorter than
  `twcl`, rises closer than `tcc`, or CLK X: LOST.
- **Machine cycles.** At each CLK rise that starts a machine cycle the
  adapter looks ahead in the core for its next bus request and drives that
  cycle's waveform at the PS0178 edges:
  - opcode fetch (4 T): A at T1 rise + `tdcra`, M1# low; MREQ# and RD#
    low after T1 fall; data sampled at T3 rise; at T3 rise MREQ#, RD#, M1#
    high, A = the refresh address (I, R), RFSH# low; MREQ# low at T3 fall
    and high at T4 fall; RFSH# high at the next T1 rise;
  - memory read (3 T): MREQ# and RD# from T1 fall to T3 fall, data sampled
    at T3 fall;
  - memory write (3 T): MREQ# from T1 fall, data valid at T1 fall +
    `tdcfd`, WR# from T2 fall to T3 fall, data Z at the next T1 rise;
  - I/O read and write (4 T, one automatic wait state TW*): A = the full
    16-bit port address; IORQ# and RD#/WR# low from T2 rise to T3 fall;
    read data sampled at T3 fall;
  - interrupt acknowledge (6 T: T1, T2, TW*, TW*, T3, T4): A = PC, M1# low
    from T1 rise, IORQ# low from the first TW* fall; the vector is sampled
    at T3 rise; then the refresh as in a fetch.
  WAIT# low at the fall the chip samples (T2 of a fetch or memory cycle,
  TW* of an I/O cycle or int-ack) adds a wait state; the strobes stay low.
  Other T-states (internal ones) drive nothing.
- **Unknown inputs** follow the 6502 family's rules: an input that changed
  within its setup time reads X; a tick with X inputs runs twice and the
  CPU is LOST only if the results differ. An NMI# low pulse shorter than
  `twnmi` reads X. INT# X is harmless while interrupts are disabled.
- **NMI#** is edge-triggered (a fall is kept until the next rise); NMI
  runs a 5-T fetch at PC (its data unused), two pushes and a fetch at 0066.
  **INT#** with interrupts enabled runs an int-ack in IM 0 (the vector byte
  executed: `FF` is RST 38h), IM 1 (to 0038) or IM 2 (the word at I:vector).
- **HALT**: HALT# low at the T4 fall (+ `tdcfhalt`) of the HALT opcode's
  fetch; the halted M1 cycles keep refreshing; an interrupt ends it (HALT#
  high at the next fall).
- **BUSREQ#** low at the last T-state of a machine cycle releases the bus
  at the next rise: BUSACK# low, address, data, MREQ#, IORQ#, RD# and WR#
  Z, no machine cycles. BUSREQ# high again: BUSACK# high at the next fall,
  and the following rise starts the pending machine cycle. BUSREQ# X: LOST.
- **State**: registers `AF BC DE HL AF' BC' DE' HL' IX IY SP PC I R WZ IM
  IFF1 IFF2`; status HALTED while halted, BUS_RELEASED while the bus is
  released.

Known differences of the vendored Z80 core (pinned by `cpu_core_test`):
during HALT the bus shows the HALT opcode's own address in the halted M1
cycles (the real chip shows the next address; execution resumes correctly);
the undocumented flag bits 3 and 5 after SCF/CCF (the Q register); the
flags of an interrupted repeating block instruction. Not modeled: the
read-data and input hold times; the 4 MHz Z84C0004 grade; the peripheral
daisy chain (IEI/IEO are not CPU pins).


**The W65C816S** (`"core": "65816"`, WDC W65C816S; plan-14-H;
`backend/src/models/cpu65816.c` on the vendored jsmooch core,
`backend/src/third_party/jsmooch/`). Its own keys besides `address`, `data`
and `grades` — every one required, and a key of another core (`sync`, `so`,
`phi1Out`, `phi2Out`, …) is refused:

```json
"core": "65816",
"clock": "PHI2", "rw": "RWB", "irq": "IRQB", "nmi": "NMIB", "reset": "RESB",
"rdy": "RDY", "abort": "ABORTB", "vda": "VDA", "vpa": "VPA",
"vectorPull": "VPB", "memoryLock": "MLB", "emulation": "E", "mx": "MX",
"busEnable": "BE",
"grades": {"14": {"tcyc": 70, "tpwh": 35, "tpwl": 35, "tads": 30, "tah": 10,
                  "tbas": 33, "tbh": 10, "tdsr": 10, "tmds": 30, "tdhw": 10,
                  "tpcs": 10, "tes": 10, "teh": 5, "tbvd": 25, "vcc": [1.71, 5.25]}}
```

Timing names (Table 4-2): `tcyc`, `tpwh`, `tpwl` (> 0), `tads`/`tah`
(address, RWB, VDA, VPA, VPB, MLB valid / held from the PHI2 fall),
`tbas`/`tbh` (the bank address on D0–D7 valid from the fall / held past the
rise), `tdsr` (read data setup), `tmds` (write data valid from the rise),
`tdhw` (write data held past the fall), `tpcs` (control input setup),
`tes`/`teh` (E and MX setup / hold), `tbvd` (BE to bus valid / Z).

Behaviour: the 6502 family's rules above (LOST at power-up and on a bad
clock or a used X; RES for at least 2 cycles; RDY; the differential rule;
the commit at each PHI2 fall), plus:

- **The bus cycle.** A0–A15, RWB, VDA, VPA, VPB (low on vector reads) and
  MLB (low through a read-modify-write's read, modify and write cycles):
  X at fall + `tah`, valid at + `tads`. **D0–D7 carry the bank address
  while PHI2 is low** (X at fall + `tdhw`, valid at + `tbas`; the board
  latches it, Figure 5-1); at the rise the bank holds to + `tbh`, then a read
  floats D0–D7 and a write drives X, then its data at rise + `tmds`, held to
  the next fall + `tdhw`. An internal cycle (VDA = VPA = 0) still drives the
  address (PC) and the bank.
- **E** follows the emulation flag, changing at fall + `teh` from the first
  cycle after the XCE. **MX** shows M while PHI2 is high and X while it is
  low, changing at + `teh` after each edge; an instruction's cycles show the
  E/M/X it began with (REP, SEP, PLP, RTI, XCE take effect at the next
  opcode fetch).
- **RESB** low: address X, data Z, RWB 1, VDA = VPA = 0, VPB = MLB = 1, E =
  1, MX = 1 from + `tah`; released, the reset sequence ends in the vector
  read at 00FFFC/D with VPB low (a reset also restarts a stopped CPU).
- **RDY** low holds the cycle, reads and writes; D0–D7 stay in their
  transfer state through PHI2 low (no bank, §7.6). WAI pulls RDY low until
  IRQ (even with I set), NMI, ABORTB or reset; STP stops it until reset.
- **ABORTB** low at a PHI2 fall marks the instruction in progress: when it
  ends (the next opcode fetch) every register goes back to the snapshot taken
  at its opcode fetch, and the core pushes its address (PBR in native mode),
  PC and P and jumps through 00FFE8 (native) / 00FFF8 (emulation) with VPB
  low; memory writes it made stay. ABORTB X: LOST.
- **BE** low floats A0–A15, D0–D7 and RWB `tbvd` later; VDA, VPA, VPB, MLB,
  E and MX stay driven; the CPU keeps running.
- **State**: registers `C` 16, `X` 16, `Y` 16, `S` 16, `D` 16, `DBR` 8,
  `PBR` 8, `PC` 16, `P` 8, `E` 1; `instruction_address` is 24-bit
  (bank:address) and the text is jsmooch's disassembly, uppercased
  (`LDA $123456`, `LDA #$1234` with M = 0).

Not modeled: the read data hold `tDHR`; timing at supply columns other than
5 V; the §7.4.1 ABORT exceptions (the snapshot restores every register).

### `serial`

Bus-controller ICs that move bytes over a serial line (plan-12): UARTs,
USARTs, ACIAs and the I2C/SPI bridges (`backend/src/models/serial.c`). The
device's `"model"` picks the behavioral model; each model is a full model of
the chip's digital behavior (registers, FIFOs, baud generator, framing,
modem lines, interrupts). Every model shares the line engine
(`serial_line.c`: baud generator, transmitter, receiver) and the host-bus
helpers (`serial_bus.c`).

| Key | Value |
|---|---|
| `"model"` | `"ns16550"` (the 16450/16550 family and the I2C/SPI bridges built on it), `"max3100"` (MAX3100, MAX3140), `"max310x"` (MAX3107, MAX3108, MAX3109, MAX14830), `"sc18is604"` (the SC18IS604 SPI-to-I2C bridge); `"ay1013"`, `"i8251"`, `"i82510"`, `"acia6551"`, `"hs82c52"`, `"com78c80x"` (the classic UARTs, plan-12-H); more models in later plans |
| `"grades"` | grade name → the model's timing names (ns; a missing name is 0), plus optional `"vcc"`, `"inputMax"`, `"note"` |
| the model's keys | below; a key of another model is rejected (`"x" is not a key of model "y"`) |

Part keys: `"grade"` (a key of `"grades"`), optional `"vcc"`, `"inputMax"`
(the part's own range when its row differs from its grade's). `PartSerial`
holds the model, the channel count and the grade.

**Signal rules (every model).** Power-on is the reset state, then the reset
pin applies at start; registers the reset does not touch power on X. An
input at Z reads X, except reset pins and chip selects, which read inactive.
An X on a strobe or chip select makes the cycle X (a read drives X, a write
stores X). Serial outputs drive strong; the data bus drives only during a
selected read, after the grade's access delay, and floats after its float
delay. The clock input is counted edge by edge (wire an oscillator part to
it); the crystal output pin is never driven. Undefined behavior is X; clock
and baud limits are not checked.

**Terminal.** `circuit_serial_sent` gives the frames a channel sent on its
TX pin; `circuit_serial_send` types bytes into its RX pin as a real
waveform at the channel's current baud and format, while nothing else
drives that net (a pull part may).

#### `ns16550`

The PC16550D (`plan-12-data/specs/ti-a.md` § PC16550D) and, with
`"fifo": 0`, the 16450.

```json
"model": "ns16550",
"bus": {"style": "intel", "data": ["D0", "…", "D7"], "address": ["A0", "A1", "A2"],
        "read": "RDBAR", "readHigh": "RD", "write": "WRBAR", "writeHigh": "WR",
        "select": ["CS2"], "selectHigh": ["CS0", "CS1"], "addressStrobe": "ADS",
        "driverDisable": "DDIS", "reset": "MR", "resetActiveHigh": true},
"clock": {"in": "XIN", "out": "XOUT", "receive": "RCLK", "baudOut": "BAUDOUT"},
"channels": [{"tx": "SOUT", "rx": "SIN", "rts": "RTS", "dtr": "DTR", "cts": "CTS",
              "dsr": "DSR", "dcd": "DCD", "ri": "RI", "out1": "OUT1", "out2": "OUT2",
              "int": "INTR", "txrdy": "TXRDY", "rxrdy": "RXRDY"}],
"fifo": 16, "rxTrigger": [1, 4, 8, 14], "intPin": "driven", "dmaClass": "P", "timeout": "4ch",
"grades": {"X": {"trvd": 60, "thz": 100, "trdd": 60, "tbld": 175, "trint": 1000, "thr": 175,
                 "tmdo": 200, "tsim": 250, "trim": 250, "twxi": 195, "twrr": 290, "tir": 250,
                 "vcc": [4.5, 5.5]}}
```

| Key | Value |
|---|---|
| `"bus"` | `"style": "intel"`; `"data"` 8 names; `"address"` 3 names (A0 first); `"read"` (active low) and/or `"readHigh"`; `"write"` and/or `"writeHigh"`; `"select"` (active-low names) and `"selectHigh"` (active-high); optional `"addressStrobe"` (ADS: transparent while low, latched on its rising edge), `"driverDisable"` (DDIS, low while the chip drives the bus), `"selectOut"` (CSOUT), `"reset"` with `"resetActiveHigh"` (default true); optional `"iowOnly"`: `{"modePin", "rw"}` (TL16C750E: while the MODE strap is low, CS is the strobe and the `rw` pin, IOW, picks read (high) or write (low) at its fall; the read pin is unused; RESET keeps its polarity) |
| `"clock"` | `"in"` (XIN, counted); optional `"out"` (XOUT, never driven), `"receive"` (RCLK; absent = the 16× tick also clocks the receiver), `"baudOut"` (BAUDOUT: the 16× clock, divisor × clock period) |
| `"channels"` | one channel: pin names `"tx"`, `"rx"`, `"int"` (required), `"rts"`, `"dtr"`, `"cts"`, `"dsr"`, `"dcd"`, `"ri"`, `"out1"`, `"out2"`, `"txrdy"`, `"rxrdy"` |
| `"fifo"` | `16`, or `0` for the 16450 (no FCR, IIR[7:6] = 00, no timeout) |
| `"rxTrigger"` | the 4 RX trigger levels of FCR[7:6] (default `[1, 4, 8, 14]`) |
| `"intPin"` | `"driven"`: INTR active high, always driven |
| `"dmaClass"` | `"P"`: the PC16550D's TXRDY/RXRDY mode 0/1 rules |
| `"timeout"` | `"4ch"`: 4 character times with data in the FIFO |

Timing names (ns): `trvd` read to data, `thz` read end to float, `trdd`
read to DDIS, `tbld` clock to BAUDOUT, `trint` RBR/LSR read to INTR clear,
`thr` THR write to INTR clear, `tir` IIR read to THRE clear, `tmdo` MCR
write to the modem outputs, `tsim` modem input to INTR, `trim` MSR read to
INTR clear, `twxi` THR write to TXRDY, `twrr` RBR read to RXRDY.

Behavior: the PC16550D register set at A2..A0 with DLAB (LCR[7]); MR
resets IER, IIR, FCR, LCR, MCR, LSR (0x60), the MSR deltas and SCR (0x00)
and leaves DLL, DLM, RBR and THR; the FIFOs with trigger levels, the three
error bits per byte, LSR[7], the timeout, the transmitter-interrupt delay
rule (a lone byte delays THRE to its last stop bit) and DMA modes 0/1;
interrupt priority line status > RX data / timeout > THRE > modem status;
loopback (SOUT held 1, the transmitter feeds the receiver, MSR from MCR,
the modem output pins inactive); break, stick parity and the scratch
register. The receiver detects a start bit on 1 → 0, checks it mid-bit (a
false start is ignored), samples data, parity and the first stop bit
mid-bit, takes a framing error as the next start bit (PC16550D resync) and
loads one 0x00 for a break (BI with FE), then waits for the line to return
to 1. A character reaches the RBR 1 RCLK after its stop-bit sample (3 in
FIFO mode).

**Option keys (plan-12-D).** Every key is optional; absent keeps the PC16550D
behavior above. They cover the TI, National, Intersil, SMSC and NXP members
of the family.

| Key | Values |
|---|---|
| `"channels"` | 1–8 channel objects. A channel may add `"select"` / `"selectHigh"` (its own chip selects: TL16C554, TL16C2550; then every channel needs one), `"op"` (OP pin = NOT MCR[3]) and `"mf"` (a multi-function pin chosen by AFR[2:1]: OP, BAUDOUT, RXRDY, reserved). `"int"` may be absent. |
| `"channelSelect"` | `{"pin": name, "aIsHigh": bool}`: one chip select plus a pin that picks channel A or B (TL16C2552, NS16C2552/2752). |
| `"broadcastWrite"` | `"none"` (several channel selects at once: an X cycle), `"multi-cs"` (a write goes to each selected unit; a read is X), `"afr-conc"` (the AFR concurrent-write bit sends every write to all channels). |
| `"fifo"` | `0`, `16`, `32`, `64`, `128`, or `"750"` (16 bytes, or 64 when FCR[5] is written with LCR[7] = 1; IIR[5] shows it). |
| `"rxTrigger"`, `"rxTrigger64"`, `"txTrigger"` | 4 levels each: the RX levels of FCR[7:6] (and in the 750's 64-byte mode), the TX levels of FCR[5:4] (in free spaces; written only with EFR[4] = 1). |
| `"tlr"` | `4` or `8`: TLR's nibbles × this override the FCR levels when non-zero. |
| `"dmaClass"` | `"P"` (mode-1 TXRDY low when empty, high when full), `"N"` and `"B"` (low while a slot is free), `"T"` (low while free spaces ≥ the TX trigger; RXRDY also inactive while LSR[7] = 1). |
| `"dmaPins"`, `"dma"` | `"sharedAnd"` (TL16C554: one RXRDY and one TXRDY, low when any channel's is low) or `"sharedWireOr"` (TL16C754B: the same, open drain), with `"dma": {"txrdy", "rxrdy"}` naming the pins. |
| `"intPin"`, `"intControl"` | `"driven"`; `"out2-gated"` (3-state unless MCR[3] = 1); `"intn"` (`intControl` names INTN: high = driven, low or open = gated); `"intsel"` (enabled by `intControl` INTSEL OR MCR[3]); `"mcr5-open-source"` (MCR[5] = 1 drives only the high level). |
| `"lcrReset"`, `"sprReset"`, `"tcrReset"`, `"mcrReset"`, `"afrReset"` | register values after reset (default 0; `sprReset` absent leaves SPR alone, X at power-on). `"xonReset"`: power-up Xon1, Xon2, Xoff1, Xoff2 (absent: X). |
| `"iirPriority"` | `"standard"` (RX data above the timeout), `"nxpEqual"` (one level, shown as the timeout while one is pending). |
| `"lsr7Clear"` | `"noErrors"` (an LSR read clears LSR[7] when no errored byte is left), `"lsrRead"` (any LSR read clears it). |
| `"lsrErrors"` | `"sticky"` (16550: LSR[4:1] latch until an LSR read) or `"head"` (752 style: LSR[4:2] are the head byte's error bits; the line-status interrupt is OE or an errored byte anywhere in the FIFO). |
| `"stopCheck"` | `"first"` or `"both"` (82C50A: a 0 in either of 2 stop bits is a framing error). |
| `"statusReadDefer"`, `"busSync"`, `"majorityVote"` | `true`: status bits that change during an LSR/MSR read show after it (82C50A, TL16C552/554); the strobes are sampled on XIN rising edges (TL16C754B, SC16C752B); each bit is the majority of 3 samples around mid-bit. |
| `"timeout"`, `"timeoutDelay"` | `"4ch"` or `"4w12"` (4 characters + 12 bit times); RCLKs added to the 4ch timeout (default 8; 9 on the TL16C550C). |
| `"clock"` | adds `"single"` (one clock input for TX and RX, no RCLK), `"select"` (the CLKSEL pin), `"baudOutShape"` (`"half"`, or `"low2"`: low for 2 clocks, high for N − 2). |
| `"divisorZero"` | the divisor a 0 acts as (COM92C451: 3); absent: 0 stops the generator. |
| `"extended"` | `"afe"` (MCR[5] auto flow), `"750"` (FCR[5] 64-byte mode, IER[4] sleep, IER[5] low power, MCR[5] AFE), `"650"` (LCR = 0xBF bank: EFR, Xon1/2, Xoff1/2; EFR[4] write-enables IER[7:4], FCR[5:4], MCR[7:5]), `"752"` (650 plus TCR/TLR at 6/7 with EFR[4] and MCR[6], FIFORdy at 7 with MCR[2] and not MCR[4]), `"nsEfr"` (the NS16C2552/2752 bank). |
| `"afr"` | `"ti752"` (AFR at 2 when LCR[7:5] = 100: CONC, IREN, 485EN, 485LG, RCVEN, DLY), `"ti2552"` / `"ns"` (AFR at 2 with LCR[7] = 1: CONC, MF select). |
| `"dlf"` | `true`: TL16C750E DLF at 7 (fraction n/64, DLF[7] 8× sampling). |
| `"autoFlow"`, `"rtsTable"`, `"xoffTable"` | `"afe"` (RTS off at the RX trigger, on when empty; CTS gates TX; no ΔCTS meanwhile), `"tcr"` (RTS off at TCR[3:0] × m, on at TCR[7:4] × m), `"efrTable"` (EFR[6]/[7] with the per-trigger `rtsTable` `[[off, on] × 4]`). `xoffTable` gives the Xoff/Xon send levels per trigger (else TCR). |
| `"swFlow"` | `"tiAnd"` (EFR[3:0] = 1011/0111 compare either character; 1111/0011 sequential pairs) or `"nxpAnd"` (1011/0111 are sequential pairs too). |
| `"specialChar"`, `"specialNeedsNoFlow"`, `"specialClear"`, `"ctsRtsClear"` | EFR[5] special character (Xoff2 match: IIR[4], stored); only with EFR[3:0] = 0 (SC16C650B); cleared by `"iir"` or `"lsr"`; the CTS/RTS interrupt cleared by `"iir"`, `"msr"` or `"iirOrMsr"`. |
| `"sleep"` | `"ier4"` (EFR[4] and IER[4]: the baud clock stops when idle; wakes on RX, a modem input or a THR write), `"ier4-all"` (only when every channel asks). BAUDOUT reads X while asleep. |
| `"irda"` | `"mcr6"` or `"afr1"`: TX idles low and sends a 3/16-bit pulse per 0; RX takes a falling edge as a 0 for one bit. |
| `"rs485"` | `"afr-dtr"`: with AFR 485EN, DTR goes high one bit before the first start bit and stays DLY bit times after the last stop bit. |
| `"prescaler"` | `"mcr7"` (MCR[7] = 1 divides XIN by 4) or `"mcr7-clksel"` (MCR[7] = NOT CLKSEL after reset). |
| `"tcrNeedsDlab0"`, `"dlabHidesUpper"` | TCR/TLR only with LCR[7] = 0 (SC16C752B); with LCR[7] = 1 only DLL, DLM and LCR answer (SC16C650B). |
| `"idRegs"` | `{"drev": byte, "dvid": byte}`: read at 0/1 with LCR[7] = 1 and DLL = DLM = 0. |
| `"tristate"` | the TRI pin: while high every output floats (TL16C552). |
| `"printerPort"` | `{"style": "ti451" \| "ti552" \| "com92c451", "data": [PD0…PD7], "select", "strobe", "autoFeed", "init", "selectIn", "ack", "busy", "paperEnd", "slct", "error", "irq", "inputEnable" (ti451 LPTOE), "pemd", "enirq" (ti552)}`: the PC printer port at its own chip select, A1..A0 (data, status, control). |

**I2C/SPI bridges (plan-12-I).** The SC16IS740, SC16IS750/760, SC16IS752/762,
PI7C9X760/762, XR20M1170 and XR20M1280 are an `ns16550` core behind an I2C
or SPI target host port (`backend/src/models/ns16550_bridge.c`,
`ns16550_xr1280.c`).

```json
"bus": {"style": "i2c-spi", "selectPin": "I2C/SPI", "spiLevel": 0, "selectSampled": "reset",
        "spi": {"cs": "CS/A0", "sclk": "SCL/SCLK", "mosi": "SI/A1", "miso": "SO", "mode": 0},
        "i2c": {"scl": "SCL/SCLK", "sda": "SDA", "address": ["CS/A0", "SI/A1"], "table": [76, 72, …]},
        "reset": "RESET", "resetActiveHigh": false, "nack": ["reset"], "sdaTimeout": 25000000},
"clock": {"in": "XTAL1", "out": "XTAL2"},
"channels": [{"tx": "TX", "rx": "RX", "rts": "RTS", "cts": "CTS", "dsr": "GPIO4/DSR",
              "dtr": "GPIO5/DTR", "dcd": "GPIO6/CD", "ri": "GPIO7/RI"}],
"interrupt": "IRQ",
"gpio": {"pins": ["GPIO0", "GPIO1", "GPIO2", "GPIO3", "GPIO4/DSR", "GPIO5/DTR", "GPIO6/CD", "GPIO7/RI"],
         "registers": "sc16is", "pullUp": true},
"extended": "sc16is", "fastIrda": ["760"], "fifo": 64, "lcrReset": 29, …
```

| Key | Value |
|---|---|
| `"bus"` | a host port style of [SPI and I2C host ports](#spi-and-i2c-host-ports-bus-styles) (SPI mode 0) instead of `"intel"`, plus `"reset"` / `"resetActiveHigh"` (as the Intel bus), `"registerBits"` (`4`: sub-address and SPI command bits [6:3]; `3`: [5:3], XR20M1280), `"nack"` (`"full"`: an I2C THR write with the TX FIFO full is NACKed and dropped; `"reset"`: the byte that sets IOControl[3] is NACKed) and `"sdaTimeout"` (ns of SDA held low that reset the I2C port: SC16IS7xx 25 ms) |
| `"interrupt"` | the IRQ pin: open drain, active low, low while any channel's IIR shows a pending interrupt (and, XR20M1280, its GPIO or wake-up interrupt) |
| `"extended"` | `"sc16is"` (the SC16IS7xx map), `"sc16is-sfr"` (plus the PI7C9X SFR bank), `"sc16is-dld"` (plus the XR20M1170 DLD), `"xr1280"` (the XR20M1280 map) |
| `"gpio"` | `{"pins": [GPIO0 …], "registers": "sc16is" \| "xr1280", "pullUp": bool (inputs read Z as 1), "stateReset": byte (the output latch after reset; absent: X)}`. A GPIO pin that a channel also names as `dsr`/`dtr`/`dcd`/`ri` is that modem pin while IOControl bit 1 + channel is set (`sc16is`) or MCR[2] = 0 (`xr1280`); its MSR bit reads X while it is a GPIO. |
| `"fastIrda"` | `true` or a list of grade names: EFCR[7] (SFR[3] on the XR20M1280) picks 1/4-bit IrDA pulses; elsewhere the bit is stored and the pulse stays 3/16 |
| `"dlPowerUp"` | `[DLL, DLM]` at power-up (a byte or null = X); the reset leaves them |
| `"straps"`, `"sleepPin"` | XR20M1280: `{"en485", "enir"}` (sampled as reset ends: EN485# low sets FCTR[3], ENIR# low turns IrDA on) and the SLEEP/PWRDN# pin |

Timing names (ns), besides the core's: `tdo` SPI SCLK falling to SO, `ttr`
CS high to SO float, `tvd` I2C SCL falling to SDA (data and ACK), `tgpio`
GPIO output valid. The core's `trint`, `thr`, `tir`, `trim`, `tsim`, `tmdo`
time the IRQ and DTR changes.

Behavior. The host port: I2C sub-address byte (or SPI command byte with bit
7 = 1 for a read) [6:3] register, [2:1] channel (reserved channels: reads X,
writes ignored), the address from the A1/A0 ties (`"table"`); every data byte
of one transfer goes to the same register (the FIFO bursts), except with
PI7C9X SFR[4] = 1 (auto-increment); SO is Z during the command byte and
writes. Registers (`sc16is`): 0-7 the 16550 set; 8 TXLVL (64 − TX count), 9
RXLVL, 10 IODir, 11 IOState (write: the output latch; read: the pins, or the
value IOLATCH latched), 12 IOIntEna, 13 reserved (X), 14 IOControl (bit 0
IOLATCH, 1/2 the modem pins of channel A/B, 3 a software reset as the reset
pin; SC16IS740: no GPIO, these read X but bit 3 still resets), 15 EFCR (0
9-bit multidrop: an address byte is the parity bit = 1 with LCR[5:3] = 111;
EFR[5] = 1 auto address against Xoff2; 1 RX disable; 2 TX disable; 4 RS-485
RTS low from the THR write to the last bit; 5 inverts it; 7 fast IrDA); the
650 bank at LCR = 0xBF; TCR/TLR at 6/7 with EFR[4] and MCR[2]; TCR = 0 uses
the FCR trigger for auto-RTS and Xoff; with LCR[7] = 1 (not 0xBF) only DLL,
DLH and LCR answer (and DLD at 2 with EFR[4], `sc16is-dld`: fraction
DLD[3:0]/16, DLD[5:4] 16x/8x/4x). The TX trigger applies in FIFO mode with or
without EFR[4]. Re-enabling IER[1] raises no THR interrupt on `sc16is` and
`sc16is-sfr`. The GPIO change interrupt (IIR 0x30, between modem and Xoff)
shows in channel A's IIR only: IOLATCH 0, an enabled input differs from its
level at the last IOState read (an IODir write also clears it); IOLATCH 1,
a change is latched until IOState is read. `sc16is-sfr` (PI7C9X): at LCR =
0xBF, SFREN (13) = 0x5A turns address 7 into SFR, and SFR[2] = 1 maps ASR
(2, read-only flow state), CPR (4: XIN / 2^(M − 1), M = 0 stops the
generator; N), RFD/RLS (5, SFR[6]), TFD (6), TIDLE (8, stored only), TRCTL
(9: [7:4] SCR, sample rate 16 − SCR + N; [2] THR interrupt enable; [1] RX
timeout enable) and ISCR (15: the sources; bit 0 clears the latched ones);
SFR[0] transmits regardless of flow control, SFR[1] auto DTR/DSR, SFR[3]
echoes RX on TX. `xr1280`: see `ns16550_xr1280.c` (FCTR trigger tables A-D
with TRG, auto-RTS hysteresis, Xoff 2 characters after the trigger,
independent TX/RX generators by DLD[7:6], SHR RS-485 delays, SFR (TX
address bit, 9-bit mode, RX/TX disable, fast IR, GPIO), EMSR, FC, the GPIO
set with its interrupt in ISR[6], SLEEP/PWRDN#, the FCR[3] wake-up
interrupt).
**Exar option keys (plan-12-E).** The MaxLinear/Exar ST16C, XR16C, XR16L,
XR16M and XR16V parts (`backend/src/models/ns16550_exar.c`). As with D's keys,
each is optional and absent keeps the behavior above.

| Key | Values |
|---|---|
| `"bus"` | adds `"style": "intel-motorola"`: `"modePin"` (16/68: high = Intel, low = Motorola), `"rw"` (the Intel pin that is R/W in Motorola mode, 1 = read), optional `"strobe"` (the Motorola CS; default the first select), `"channelBits"` (A3, A4: the Intel select pins that pick the channel in Motorola mode), `"irq"` (the INT pin that is the open-drain IRQ in Motorola mode: low while any channel interrupts, never gated by MCR[3]) and `"irqOthers"` (`"low"` or `"z"`: the other INT pins in Motorola mode). In Motorola mode the strobe frames the cycle, a write is taken at its rising edge, and the reset pin is active low. `"address"` has 8 names (A0–A7) with `"extended": "5g"`. |
| `"extended"` | adds `"ier5"` (ST16C1450/1550: IER[5] enables ISR[5:4] TXRDY/RXRDY status, FCR[5:4] TX trigger, MCR[7] power-down and MCR[2] to `resetOut`), `"exar650"` (LCR = 0xBF: EFR at 2, Xon1/Xon2/Xoff1/Xoff2 at 4–7; 0/1 there read X; EFR[4] write-enables IER[7:4], FCR[5:4], MCR[7:5]; with LCR[7] = 1 address 2 is DLD while EFR[4] = 1, else AFR or ISR/FCR), `"exar854"` (0xBF bank: FC (read) / TRG (write) at 0, FCTR at 1, EFR, Xon/Xoff R/W; FCTR[6] swaps SPR for FLVL (read) / EMSR (write); FCTR[2] IrDA RX invert, FCTR[3] TX interrupt at TSR empty), `"m752"` (XR16M752: the 752 bank — TCR/TLR with EFR[4] and MCR[6], FIFO Rdy at 7 with MCR[2] = 1 and MCR[4] = 0, taking priority over TLR/SPR — plus DLD), `"5g"` (XR16V798/XR16L784: the flat map below) |
| `"triggerTables"` | `{"rx": [A, B, C, "trg"], "tx": [A, B, C, "trg"]}`: tables A–C of 4 levels each (FCR[7:6] / FCR[5:4] pick the level), table D programmable (TRG, RXTRG/TXTRG); FCTR[5:4] (854) or FCTR[7:6] (5G) pick the table for both directions. TRG 0 or above 128 makes the RX trigger X. |
| `"fifoCounters"` | `"fcFlvl"`: FC (RX or TX count by FCTR[7]) and FLVL (EMSR[1:0]: 00/10 RX, 01 TX, 11 alternating RX, TX, … from the first read after an EMSR write) |
| `"idRegs"` | as D; on the 5G the global DREV/DVID at 0x8C/0x8D |
| `"dld"` | `true`: DLD — divisor + DLD[3:0]/16, and (not 5G) DLD[5:4] sampling 16×/8×/4×; XR16M752 DLD[7] IrDA (`"irda": "dld7"`) and DLD[6] RS-485 (`"rs485": "dld6"`) |
| `"sampling"` | `"8xmode"`: the 5G 8XMODE register picks 8× per channel |
| `"startSample"` | `7.5`: the start bit is checked 7½ clocks after its edge (ST16C450/550); `8` is the default |
| `"autoFlow"` | adds `"exarTable"` (854/5G: RTS off at the next table level above the trigger, on at the next below; table D: off at trigger + hysteresis, on below trigger − hysteresis, from EMSR[5:4]:FCTR[1:0] or FCTR[3:0]: 0, 4, 6, 8, 8, 16, 24, 32, 40, 44, 48, 52, 12, 20, 28, 36) and `"dtrDsr"` (as `exarTable`, on DTR/DSR while MCR[2] = 1). With `"efrTable"`/`"rtsTable"` (D's keys) the Exar 650 parts use their fixed tables; RTS needs MCR[1] = 1; no auto flow in loopback unless `"autoFlowLoopback": true` (XR16L580) |
| `"swFlow"` | adds `"exarOr"`: EFR[3:0] 1011/0111 compare either Xon (Xoff) character, 1111/0011 the pairs. Xoff goes out two character times after the RX FIFO reaches its Xoff level (XR16M752: at once), Xon at the level below; an ISR read clears the Xoff interrupt (XR16M752: only Xon); the special-character interrupt clears with the next character (XR16M752: an ISR read) |
| `"xonAny"` | `"mcr5"`: MCR[5]: any received character resumes the transmitter |
| `"sleep"` | adds `"ier4-exar"` (IER[4] with EFR[4]: the clock stops when nothing is pending, MSR[3:0] = 0, RX idle high, nothing to send; wakes on an RX start bit, a THR write or a modem input), `"ier4-exar-all"` (only when every channel sets IER[4]), `"global"` (5G SLEEP register, per channel). `"sleepNeedsDivisor": true` also needs a non-zero divisor; `"wakeIndicator": true` raises INT on waking with the ISR reading 0x01, cleared by the ISR read (XR16L784: an INT0 read) |
| `"powerSave"` | `{"pin": name}`: PWRSAVE high while asleep isolates the bus (cycles ignored, D7–D0 not driven) |
| `"irda"` | adds `"mcr6-exar"` (MCR[6], RX polarity by MCR[2]; on the 854 FCTR[2], on the 5G FCTR[4], with the receiver off while transmitting) and `"dld7"` |
| `"rs485"` | adds `"fctr-rts"` (5G FCTR[5] on RTS), `"fctr-mcr2"` (XR16V798: on RTS, or DTR with MCR[2] = 1), with the MSR-write [7:4] turnaround of 0–15 bit times; `"dld6"` (XR16M752 RTS, released a bit after the last stop bit); `"fctr3-op1"` (XR16C864 OP1, low while transmitting). The pin goes high (OP1 low) when a byte is loaded, and the TX interrupt waits for the TSR |
| `"prescaler"` | adds `"clksel"`: the CLKSEL pin (`"clock"`.`"select"`; absent pin = ÷1) sets ÷4 (low) or ÷1 at power-up and reset; MCR[7] still reads 0 (decisions.md P1) and an MCR[7] write overrides it |
| `"mfPins"` | `["op2", "baudout", "rxrdy", "x"]`: the function of each AFR[2:1] code for the channels' `"mf"` pins |
| `"afr"` | adds `"exar"`: AFR at 2 with LCR[7] = 1 (with `"dld"`, only while EFR[4] = 0): b0 concurrent write (one shared bit), b2:1 MF |
| `"broadcastWrite"` | adds `"afr0-perChannel"` (XR16L2552: AFR[0] per channel; a write goes to both while the addressed channel's AFR[0] = 1) |
| `"iirPriority"` | adds `"exarTimeoutFirst"`: the RX timeout above RX data |
| `"thrIntClear"` | `"ier1Rewrite"` (ST16C1450/1550): an ISR read does not clear the TX interrupt; a THR write, or IER[1] = 0, does |
| `"isrStatus"` | `"high"` / `"inverted"`: the ier5 ISR[4] (TXRDY) and ISR[5] (RXRDY) polarity |
| `"resetOut"` | the RST pin (ier5): RESET OR (IER[5] AND MCR[2]), MCR[2] cut off in loopback |
| `"divisorPowerUp"` | the DLL value at power-up (DLM 0), not changed by reset |
| `"mcr2Enhanced"`, `"xonWriteOnly"` | XR16L580: EFR[4] also gates MCR[2]; Xon/Xoff are write-only (reads at 4–7 in the 0xBF bank are the normal registers) |
| `"lsrBreakHold"` | LSR[4] stays set while RX is still low after the break |
| `"statusWrite"`, `"dlabHidesStatus"` | `false`: LSR/MSR writes are ignored (XR16V654); `true`: LSR/MSR read X with LCR[7] = 1 (XR16V554) |
| `"fstat"` | the FSRS pin: low reads FSTAT at any address (b3–0 NOT TXRDYD–A, b7–4 RXRDYD–A pins) |
| `"dmaHost"` | `{"dack": [one per channel], "aen", "tc"}` with `"dmaClass": "X"` (XR16C864): EMSR[2]/[3] turn TXRDY/RXRDY into active-high TXDRQ/RXDRQ (TX: below the trigger until TC or full to it; RX: at the trigger or a timeout until TC or empty); DACKx low and AEN high read the RX FIFO / write the TX FIFO whatever CS and A2–A0 are; a TC pulse ends the requests of the channel whose DACK is low |
| `"chcClock"` | the CHCCLK pin: channel C's baud generator counts it instead of XTAL1 |
| channel `"bclk"`, `"irtx"` | the channel's 16× clock out (XR16C864), its always-IrDA-encoded TX (854/864 QFP100) |
| `"interrupt"`, `"timer"`, `"variant5g"`, `"irdaStrap"` | 5G: the open-drain device INT#; `{"clock": TMRCK, "style": "commands" \| "bits"}`; `"xr16v798"` or `"xr16l784"`; the ENIR pin sampled into MCR[6] at power-up and reset |

Part key `"intPin"` (`"driven"` or `"out2-gated"`) overrides the device's for
one part: the D variants (ST16C554D, XR16C854D, XR16V564D, XR16V654D) bond
INTSEL high, so their INT pins are always driven.

The 5G map: A7 = 0 is channel space (A6..A4 channel on the XR16V798, A5..A4 on
the XR16L784 with 0x40–0x7F reserved), A3..A0 the register: 0–7 the 16550 set
(DLD at 2 with LCR[7] = 1 on the XR16V798; LSR read-only; a write to 6 is the
MSR write: [7:4] RS-485 delay, [3] TX disable, [2] RX disable on the XR16V798),
8 FCTR, 9 EFR, A TXCNT/TXTRG, B RXCNT/RXTRG, C XCHAR (read, clears) / XOFF1,
D–F XOFF2/XON1/XON2 (write). MCR[3] (XR16V798) arms send-immediate: the next
THR byte goes out ahead of the FIFO. A7 = 1 is the global block: INT0 (pending
channels), INT1–INT3 (3-bit codes: 1 RX data/line status, 2 timeout, 3 TX,
4 modem/flow, 7 timer on channel 0), TIMERCNTL (XR16V798 command codes 1–6,
9–B; XR16L784 bits: interrupt, start, one-shot, TMRCK; a read clears the timer
interrupt), TIMERLSB/MSB (a 16-bit down count, N ≥ 2, else X), 8XMODE, REGA,
RESET (soft reset per channel), SLEEP, DREV, DVID, REGB (b0: writes to every
channel). One open-drain INT# carries every channel and the timer. The TX
interrupt comes twice (below the trigger, then empty); an errored byte
interrupts as it arrives. RST# is active low. The Exar TX trigger counts bytes:
the interrupt when the count falls below it, or at empty when it never reached
it since the last reload.

#### SPI and I2C host ports (`"bus"` styles)

Serial-hosted models describe their host port with one of three `"bus"`
styles (`backend/src/models/serial_target.c`, shared by the Maxim models,
the `ns16550` bridges and the `sc18is604`). The front end does the pin protocol; the model owns its command
and sub-address bytes.

```json
"bus": {"style": "spi", "cs": "CS", "sclk": "SCLK", "mosi": "DIN", "miso": "DOUT", "mode": 0}
"bus": {"style": "i2c", "scl": "SCL", "sda": "SDA", "address": ["A0", "A1"], "table": [44, 45, null, …]}
"bus": {"style": "i2c-spi", "selectPin": "I2C/SPI", "spiLevel": 1, "selectSampled": "live",
        "spi": {"cs": "CS/A0", "sclk": "SCLK/SCL", "mosi": "DIN/A1", "miso": "DOUT/SDA", "mode": 0},
        "i2c": {"scl": "SCLK/SCL", "sda": "DOUT/SDA", "address": ["CS/A0", "DIN/A1"], "table": […]}}
```

| Key | Value |
|---|---|
| `"style"` | `"spi"`, `"i2c"` or `"i2c-spi"` (both on shared pins, picked by `selectPin`) |
| `"cs"`, `"sclk"`, `"mosi"`, `"miso"` | SPI pins; CS is active low |
| `"mode"` | SPI mode 0–3 (CPOL, CPHA); default 0 |
| `"scl"`, `"sda"` | I2C pins |
| `"address"` | I2C address strap pins, A0 first (0–3) |
| `"table"` | the 7-bit address (or `null`: none) for each strap combination: 4^n entries, index Σ tie(Aₖ) × 4ᵏ with tie 0 = low (GND), 1 = high (VDD/VL), 2 = tied to SCL, 3 = tied to SDA |
| `"selectPin"` | `i2c-spi`: the pin that picks the port |
| `"spiLevel"` | `i2c-spi`: the select level that picks SPI (default 1) |
| `"selectSampled"` | `i2c-spi`: `"live"` (the pin is followed) or `"reset"` (default: sampled when reset ends) |

SPI: MISO is 3-state while CS is high; while CS is low it drives the bits
the model marks (others Z), shifting on the clock edge opposite the sampling
edge (mode 0: sample on SCLK rising, shift on falling, the first bit at CS
falling). I2C: START, repeated START and STOP; the address byte is ACKed
only when the model claims it; data bytes in are ACKed or NACKed by the
model; bytes out shift on SCL falling edges and the controller's ACK/NACK
is sampled; SDA is open drain (0 strong or Z); there is no clock
stretching. A strap's tie is told from its level in three bus states (idle,
at the START, at the first SCL fall). Signal rules: SCLK, MOSI, SCL and SDA
at Z read X; CS at Z reads inactive; an X on CS or SCLK makes the rest of an
SPI transaction X (X bits in, MISO X); an X on SCL, or on SDA while SCL is
high, abandons an I2C transaction until the next START. SDA and SCL are
open-drain pins for the voltage check.

#### `sc18is604`

The NXP SC18IS604 SPI-to-I2C bridge (`plan-12-data/specs/bridges.md` §
SC18IS604): an SPI target (mode 3) that is a single I2C bus controller. No
UART: it has no terminal channel.

```json
"model": "sc18is604",
"bus": {"style": "spi", "cs": "CS", "sclk": "SCLK", "mosi": "MOSI", "miso": "MISO", "mode": 3},
"i2c": {"scl": "SCL", "sda": "SDA"}, "reset": "RESET", "interrupt": "INT",
"gpio": ["GPIO0", "GPIO1", "GPIO2", "GPIO3", "GPIO4"],
"grades": {"I": {"tvq": 28, "tstart": 35000, "tsa": 125, "vcc": [1.71, 3.6], "inputMax": 5.4}}
```

| Key | Value |
|---|---|
| `"bus"` | `"style": "spi"`, mode 3 |
| `"i2c"` | the controller's SCL and SDA (open drain) |
| `"reset"` | RESET (active low; Z reads inactive) |
| `"interrupt"` | INT (open drain, active low) |
| `"gpio"` | GPIO0-GPIO3 (input, push-pull or open drain by IOConfig) and GPIO4 (input) |

Timing names (ns): `tvq` SCLK falling to MISO, `ttr` CS high to MISO Z,
`tstart` the end of a message (CS high) to the I2C START, `tsa` the RESET
low time that counts as a reset (shorter pulses are filtered).

Behavior: one message per CS-low frame (MSB first, or LSB first after 0x18
0x81; 0x18 0x42 back): 0x00 N, address, N bytes (write); 0x01 N, address
(read into the 255-byte buffer); 0x02 Nw, Nr, address+W, Nw bytes,
address+R and 0x03 N1, N2, address 1, N1 bytes, address 2, N2 bytes (a
repeated START between the two transfers); 0x06 (MISO gives the buffer);
0x20 register, value and 0x21 register, dummy (MISO gives the value);
0x30 0x5A 0xA5 deep power-down (the next CS frame wakes it and is ignored);
0xFE the version string "SC18IS604 1.0.0\0" into the buffer. Registers:
IOConfig (0), IOState (1, read: the pins), I2CClock (2, reset 0x19), I2CTO
(3: [7:1] TO, [0] enable), I2CStat (4, reset 0xF0; reading it releases INT)
and I2CAdr (5). The controller runs on the internal 15 MHz oscillator (a
model time base): SCL = 15 MHz / (8 × I2CClock), 50 % duty (I2CClock 0
stops it); it waits for a free bus (no START without a STOP), follows a
target's clock stretching, loses arbitration when SDA reads 0 for its 1
(then waits and starts again), ACKs every read byte but the last, and ends
with I2CStat 0xF0 (done), 0xF1 (address NACK), 0xF2 (data NACK), 0xF8
(timeout: ((I2CClock + 1) / 15 MHz) × (1 + TO) × 512) or 0xF9 (a byte count
of 0 or more than the bytes sent), asserting INT; 0xF3 while busy. No
internal pull-ups: put pull parts on SCL, SDA and INT. MISO is Z while CS is
high, during the command byte and where a command defines no output.

#### `max3100`

The MAX3100 SPI/MICROWIRE UART and the MAX3140 (the same UART with an
RS-485/RS-422 transceiver) (`plan-12-data/specs/maxim.md` § MAX3100,
§ MAX3140).

```json
"model": "max3100",
"bus": {"style": "spi", "cs": "CS", "sclk": "SCLK", "mosi": "DIN", "miso": "DOUT", "mode": 0},
"clock": {"in": "X1", "out": "X2"},
"channels": [{"tx": "TX", "rx": "RX", "rts": "RTS", "cts": "CTS", "int": "IRQ"}],
"shutdown": "SHDN", "testMode": "loopback",
"transceiver": {"y": "Y", "z": "Z", "a": "A", "b": "B", "de": "DE", "re": "RE", "ro": "RO",
                "di": "DI", "hf": "H/F", "srl": "SRL", "txp": "TXP", "rxp": "RXP"},
"grades": {"E": {"tdv": 100, "ttr": 100, "tdo": 100, "vcc": [2.7, 5.5]}}
```

| Key | Value |
|---|---|
| `"bus"` | `"style": "spi"`, mode 0 |
| `"clock"` | `"in"` X1 (counted), optional `"out"` X2 (never driven) |
| `"channels"` | one channel: `"tx"`, `"rx"`, `"rts"`, `"cts"`, `"int"` (IRQ, open drain, active low) |
| `"shutdown"` | the SHDN pin (active low; Z reads inactive) |
| `"testMode"` | `"loopback"` (MAX3100: RTS shows the 16× clock and TX loops to RX while CS is low) or `"txLow"` (MAX3140: TX held low while CS is low) |
| `"transceiver"` | MAX3140 only: the transceiver pins; `"srl"`, `"txp"`, `"rxp"` optional |

Timing names (ns): `tdv` CS low to DOUT valid, `ttr` CS high to DOUT Z,
`tdo` SCLK falling to DOUT; transceiver: `tdp115`/`tdp500`/`tdp10m` DI to
Y/Z, `tdz115`/`tdz500`/`tdz10m` driver enable, `tdd` driver disable, `trp`
(115k and 500k) / `trp10m` A/B to RO, `trz` receiver enable/disable,
`tshdn` RE = 1 and DE = 0 to transceiver shutdown, `tdzs115`/`tdzs500`/
`tdzs10m` and `trzs` enable times out of that shutdown.

Behavior: one 16-bit word per CS-low frame (DIN bits 15–14: 11 Write
Config, 01 Read Config, 10 Write Data, 00 Read Data; DOUT bit 15 R and 14 T
from CS falling). Effects happen on CS rising and only for exactly 16 bits,
except a Write Config's interrupt masks (at the 16th SCLK rising edge) and
a data word's pop (at the 16th SCLK falling edge). Baud = X1 / (16 × BRD),
BRD from B3–B0 (1, 2, 4 … 128, 3, 6 … 384); 7 or 8 data bits, the 9th bit
(Pt/Pr) when PE = 1 (no parity is computed), 1 or 2 stop bits sent and one
checked, 3-sample majority, an 8-word receive FIFO (FEN = 1: one word)
cleared when it overflows (RA/FE then set), FE until the next good frame,
and after a framing error the receiver waits for a new start bit. IRQ =
R·RM + T·TM + Pr·PM + RA/FE·RAM; the T source is a latch set when the
transmit buffer empties and cleared on CS rising of a Read or Write Data.
A Write Config clears the receiver and the transmit buffer (T = 1) and
takes effect when the transmitter is idle. IrDA (IR = 1): a 0 bit is a
3/16-bit TX low pulse; RX idles low and a high pulse is a 0. Shutdown: the
SHDN pin (immediate, aborts the transmitter) or SHDNi (when the transmitter
is idle) stops the clock, clears the receiver, sets T, and sets RA on any RX
transition; RTS and the configuration stay writable. The transceiver is
independent of the UART: Y = DI xor TXP and Z = not Y while DE = 1 (Z while
DE = 0); RO = (A − B ≥ −50 mV) xor RXP while RE = 0, reading Y/Z when H/F = 1;
A ≠ B gives A, A = B and both open give 1 (true fail-safe), an X or one open
input gives X; H/F, TXP and RXP read Z as 0 (internal pull-downs); SRL 0, 1
or open picks the delays. The MAX3140's terminal drives A (true) and B
(complement); its UART receiver hears the typed waveform directly.

#### `max310x`

The MAX3107, MAX3108, MAX3109 and MAX14830 SPI/I2C UARTs with 128-word
FIFOs (`plan-12-data/specs/maxim.md` § MAX3107 … § MAX14830).

```json
"model": "max310x", "variant": "max3107",
"bus": {"style": "i2c-spi", "selectPin": "I2C/SPI", "spiLevel": 1, "selectSampled": "live",
        "spi": {"cs": "CS/A0", "sclk": "SCLK/SCL", "mosi": "DIN/A1", "miso": "DOUT/SDA", "mode": 0},
        "i2c": {"scl": "SCLK/SCL", "sda": "DOUT/SDA", "address": ["CS/A0", "DIN/A1"],
                "table": [44, 45, null, null, 46, 47, null, …]}},
"reset": "RST", "clock": {"in": "XIN", "out": "XOUT"},
"channels": [{"tx": "TX", "rx": "RX", "rts": "RTS/CLKOUT", "cts": "CTS"}],
"interrupt": "IRQ", "gpio": ["GPIO0", "GPIO1", "GPIO2", "GPIO3"],
"grades": {"E": {"tdo": 20, "tft": 10, "vcc": [2.35, 3.6]}}
```

| Key | Value |
|---|---|
| `"variant"` | `"max3107"`, `"max3108"`, `"max3109"` (2 UARTs) or `"max14830"` (4): picks the register differences of each datasheet |
| `"bus"` | `"style": "i2c-spi"`, SPI mode 0; with more than one UART, `"uartOffsets"`: the 7-bit I2C address offset of each UART from the table's address (MAX3109 `[0, -16]`, MAX14830 `[0, -16, -64, -80]`) |
| `"reset"` | RST (active low; Z reads inactive) |
| `"clock"` | `"in"` XIN, optional `"out"` XOUT (never driven) |
| `"channels"` | one per UART: `"tx"`, `"rx"`, `"rts"`, `"cts"` |
| `"interrupt"` | IRQ (open drain, active low) |
| `"gpio"` | 4 GPIO pin names per UART, UART0's first |

Timing names (ns): `tdo` SCLK falling to MISO, `tft` CS rising to MISO Z,
`tready` reset end to ready (IRQ is held low until then where IRQSel exists).

Behavior: SPI command byte bit 7 = 1 writes; the address is A6–A0
(MAX3107/3108), 0 U A4–A0 (MAX3109) or U1 U0 A4–A0 (MAX14830); MISO is Z
during the command byte and writes, and on the MAX3109/MAX14830 a read
drives the GlobalIRQ bits on clocks 5–8 (an 8-clock frame is the fast
read). I2C: the register address byte is ACKed only when valid. Bursts
auto-increment unless they start at 0x00 (THR/RHR). GloblComnd (0x1F,
MAX3109/MAX14830): 0xCE/0xCD turn the SPI extended map (0x20–0x25) on and
off (0x1F stays GloblComnd), 0xE0–0xEF trigger transmitters (TxSynch,
SynchDelay, TxAutoDis). Clock: PLLBypass = 1 counts XIN edges; PLLEn = 1
measures the XIN period and runs PreDiv and the ×6/×48/×96/×144 PLL on
timers; neither gives no clock; the fractional generator divides by DIV +
FRACT/16 (16×, or 8×/4× in 2x/4x mode); PLLConfig and CLKSource are UART0's
on the MAX3109/MAX14830. The full register set: interrupts (ISR levels and
clear-on-read events, LSR of the word last read from RHR, special
characters, STS with GPIO change interrupts), MODE1/MODE2 (IRQSel, sleep,
RS-485 direction control with setup/hold, TX/RTS 3-state, TX/RX disable,
echo suppression, multidrop with address filtering, loopback, special
characters, FIFO and software reset), LCR (incl. forced parity and break),
RxTimeOut, FlowLvl/FlowCtrl (auto RTS/CTS, XON/XOFF both ways), IrDA
SIR/MIR with TX/RX inversion (the MAX3100's pulse polarity), GPIOs (push-pull
or open drain; inputs sampled on the reference clock), TIMER and CLKtoGPIO,
CLKtoRTS. Internal pulls are read, not driven: RX reads Z as 1, GPIO inputs
read Z as 0. Control bits written X act as 0, except the line format (X
frames) and the divisor and clock registers (the generator stops).

Every plan-12-H model below sends and receives through the line engine
(start bit checked mid-bit; data, parity and the first stop bit sampled
mid-bit), and after a framing error looks for a fresh 1 → 0 (no PC16550D
resync). Each channel is a terminal channel.

#### `ay1013`

The pin-parallel UARTs: AY-5-1013A / AY-3-1015D, COM2017, COM1863 /
COM8018, HD-6402, and the CDP1854A (its mode 1 with `"cpuMode"`). No
register bus: transmit data, receive data, control and status are pins.

```json
"model": "ay1013",
"transmitData": ["DB1", "…", "DB8"], "receiveData": ["RD1", "…", "RD8"],
"receiveEnable": "RDE", "statusEnable": "SWE",
"status": {"dav": "DAV", "tbmt": "TBMT", "eoc": "EOC", "or": "OR", "fe": "FE", "pe": "PE"},
"control": {"strobe": "CS", "noParity": "NP", "stops": "TSB", "wordLength": ["NB1", "NB2"],
            "evenParity": "EPS"},
"dataStrobe": "DS", "resetDav": "RDAV", "serialOut": "SO", "serialIn": "SI",
"clocks": {"transmit": "TCP", "receive": "RCP"},
"reset": "XR", "resetClearsData": false, "fiveBitStops": 2
```

| Key | Value |
|---|---|
| `"transmitData"`, `"receiveData"` | 8 names each, LSB first; unused high RD bits read 0 |
| `"receiveEnable"`, `"statusEnable"` | low drives RD / the status pins (PE FE OR DAV TBMT), high floats them; EOC and SO are always driven |
| `"status"` | the six status pins |
| `"control"` | the control word pins and their strobe: a level latch, transparent while the strobe is high, held when it falls |
| `"dataStrobe"` | low: TBMT falls; the TD byte is taken on the rising edge |
| `"resetDav"` | low clears DAV (level) |
| `"clocks"` | `"transmit"`, `"receive"` (16× clocks; TCP falling edges shift, RCP falling edges are counted); optional `"hiacc"`: 32× clocks while that pin is high (COM8018), plus the COM1863 receive rules (OR reset only with RDA 0 and RDAR 1 at the transfer, RDA notched, RDA waits for RDAR high) |
| `"reset"` | high resets (TBMT, EOC, SO high; DAV OR FE PE low; the holding register empty) |
| `"resetClearsData"` | the reset also clears the receive holding register (power-on 0, else X) |
| `"resetClearsControl"` | optional: the reset also clears the control word (CDP1854A) |
| `"fiveBitStops"` | `1.5` or `2`: the stop length the stop select gives with 5-bit words |
| `"cpuMode"` | optional (CDP1854A): `{"modePin", "select1", "select2", "select3", "registerSelect", "readWrite", "tpb", "cts", "psi", "es"}`. MODE high is the CDP1800 bus: selected = CS1 high, CS2 low, CS3 high; RSEL and RD/WR pick THR, RHR, control, status (Table III); writes are taken on TPB's falling edge, a read drives the receive-data pins for the whole selected read; INT (the `pe` pin), DA and THRE pins are active low, FE and PE/OE (the `or` pin) always driven, RTS on the `eoc` pin; the reset pin is CLEAR (active low) |

Part keys: optional `"fiveBitStops"` and `"resetClearsData"` override the
device's (the AY-3-1015D on the AY-5-1013A device). Timing names (ns):
`tpd` clock to SO/EOC/TBMT, `tds` DS to TBMT, `ten` enable to valid,
`tdis` enable to float, `tdav` transfer to DAV, `trdav` RDAV to DAV low,
`tacc` mode-1 read access, `tint` TPB to INT / DA / THRE.

#### `i8251`

The 8251A USART, asynchronous and synchronous.

```json
"model": "i8251",
"bus": {"data": ["D0", "…", "D7"], "controlData": "CD", "read": "RD", "write": "WR",
        "select": "CS", "reset": "RESET"},
"clock": "CLK",
"serial": {"tx": "TXD", "rx": "RXD", "txClock": "TXC", "rxClock": "RXC", "txrdy": "TXRDY",
           "rxrdy": "RXRDY", "txempty": "TXEMPTY", "syndet": "SYNDET", "dsr": "DSR", "dtr": "DTR",
           "cts": "CTS", "rts": "RTS"}
```

After a reset the first control write (C/D = 1) is the mode; a synchronous
mode then takes SYNC1 (and SYNC2), then commands until IR. Asynchronous:
×1, ×16, ×64 of TxC (falling edges shift) and RxC (rising edges sample),
5–8 bits, parity, 1 / 1.5 / 2 stops (transmit only), BRKDET after two
characters of low. Synchronous: one bit per clock, the bit-by-bit hunt for
SYNC1 (then SYNC2) after EH, SYNDET (an output, or an input with ESD =
1), SYNC fill when the CPU is late. TxRDY (pin: buffer empty, CTS low,
TxEN) and RxRDY reset on the leading edge of the data write / read. CLK
sets only maximum delays, which are not modeled. The terminal refuses a
channel that is not in an asynchronous mode. Timing names: `trd`, `tdf`
(read access, float), `tclr` (strobe to TxRDY / RxRDY), `tdtx` (TxC to
TxD), `twc` (write to DTR / RTS).

#### `i82510`

The 82510 asynchronous serial controller.

```json
"model": "i82510",
"bus": {"data": ["D0", "…", "D7"], "address": ["A0", "A1", "A2"], "read": "RD", "write": "WR",
        "select": "CS", "reset": "RESET"},
"clock": {"in": "CLK/X1", "out": "OUT2/X2"},
"serial": {"tx": "TXD", "rx": "RXD", "int": "INT", "cts": "CTS", "rts": "RTS", "dtr": "DTR/TB",
           "ri": "RI/SCLK", "dsr": "DSR/TA/OUT0", "dcd": "DCD/ICLK/OUT1"}
```

The four banks of Table 9 (an address Table 9 leaves empty reads X), the
wake-up values of Table 4, the RTS / DTR strap as RESET falls (crystal or
external clock, the divide by 2; pin 8 is OUT2 only with an external
clock), BRGA and BRGB (BRG or timer mode, sources, the cascade), the 16×
or 1× (SCLK) clocks, the 1- or 4-deep FIFOs with RXF / TXF, the voting
filter, start check, DPLL and noise flag, control-character and µLAN
recognition, quarter-bit stops, 9-bit characters, software parity, the
handshake modes, loopbacks and automatic echo, the PMD pin functions,
GSR / GIR / GER with automatic or manual acknowledge, the ICM commands and
power down. Timing names: `trldv`, `trhdz` (read access, float), `ttxd`
(1× TXD after SCLK).

#### `acia6551`

The CDP65C51 / CDP65C51A ACIA.

```json
"model": "acia6551",
"bus": {"data": ["D0", "…", "D7"], "registerSelect": ["RS0", "RS1"], "select": "CS1",
        "selectHigh": "CS0", "phi2": "PHI2", "readWrite": "RW", "reset": "RES"},
"clock": {"in": "XTLI", "out": "XTLO", "receive": "RXC"},
"serial": {"tx": "TXD", "rx": "RXD", "rts": "RTS", "cts": "CTS", "dtr": "DTR", "dsr": "DSR",
           "dcd": "DCD", "irq": "IRQ"}
```

A selected cycle acts while PHI2 is high (writes taken as it falls). The
Table II baud divisors of XTLI (or XTLI as the 16× clock with SBR 0000);
RXC is the receiver's 16× input, or the generator's output with RCS = 1.
IRQ is open drain: RDRF, DCD / DSR changes, and TDRE as a level with TIC
01 (it recurs while idle). Part key `"ctsA": true` gives the CDP65C51A's
CTS rule (finish the character, no TDRE interrupt while CTS is high);
without it CTS high marks TXD at once. Timing names: `tcdr`, `thr` (read
access, hold), `tdd` (clock to TXD), `tdly` (PHI2 to DTR / RTS), `tirq`.

Device key `"w65c51n": true` (optional, default `false`; plan-14-E) is the
WDC W65C51N (`datasheets/controller-uart/w65c51n.pdf`): TDRE (status bit
4) reads 1 always (p.10: the TSR loads when the TDR is written), so a TIC
01 interrupt is set again right after every status read; a TDR write while
a frame is going out cuts it off at the next bit boundary, where the new
byte's start bit begins (the cut-off character is not in the sent log; a
frame cut in its stop bits had sent its data and is logged); no parity —
command bits 5–7 are ignored and PE reads 0 (p.14); an overrun transfers
the new byte into the RDR and flags OVR (p.7), and an RDR read clears OVR,
FE and PE (p.10). CTSB high marks TxD at once (no `"ctsA"`). Everything
else is the model above.

#### `hs82c52`

The 82C52 and, with `"hd6406"`, the HD-6406.

```json
"model": "hs82c52",
"bus": {"data": ["D0", "…", "D7"], "address": ["A0", "A1"], "read": "RD", "write": "WR",
        "select": "CS0", "selectHigh": "CS1", "addressLatch": "ALE", "reset": "RST"},
"clock": {"in": "IX", "out": "OX", "clockOut": "CO"},
"serial": {"tx": "SDO", "rx": "SDI", "cts": "CTS", "dsr": "DSR", "dtr": "DTR", "rts": "RTS",
           "int": "INTR", "tbre": "TBRE", "dr": "DR"},
"hd6406": {"rlsd": "RLSD", "ri": "RI", "tc": "TC", "oe": "OE", "fe": "FE", "pe": "PE",
           "sie": "SIE", "sfd": "SFD", "txdack": "TXDACK", "rxdack": "RXDACK"}
```

`"selectHigh"` and `"addressLatch"` are optional (the HD-6406's CS1 and
ALE). TBR/RBR, UCR (separate TX and RX parity), BRSR (prescaler, the Fig 2
divisors with 16/3, 32/3, 58/3; undefined codes stop the generator and CO
reads X), MCR (normal, transmit break, echo, loop), USR (DR and TBRE live,
the rest latched and cleared by a read), MSR, and INTR from the edge
detectors. HD-6406: RXDACK / TXDACK cycles, the status pins, SIE and SFD,
RLSD and RI. Timing names: `trldv`, `trhdz`, `tco` (IX to CO), `tbrg`
(BRG to CO), `tsdo`, `tintr`, `tflag` (status pins).

#### `com78c80x`

The SMC COM78C804 / COM78804 (4 lines) and COM78C802 (2 lines).

```json
"model": "com78c80x",
"bus": {"data": ["DL0", "…", "DL7"], "address": ["ADD0", "…", "ADD4"], "select": "CS",
        "strobe": "DS", "write": "WR", "ready": "RDY", "reset": "RESET"},
"clock": "CLK", "lines": 4,
"channels": [{"tx": "TXD0", "rx": "RXD0", "dsr": "DSR0", "dcd": "DCD0"}, "…"],
"interrupt": {"irq": "IRQ", "line": ["IRQLN0", "IRQLN1"], "txrx": "IRQTXRX"}
```

| Key | Value |
|---|---|
| `"bus"` | `"address"`: 5 names (4 lines) or 4 (2 lines); the high bits pick the line. A name used by two pins of a pinout (DS on the DIP-48) is one input: the pins must agree, else it reads X |
| `"lines"` | `2` or `4`; `"channels"` has that many lines |
| `"interrupt"` | IRQ (open drain), the line-number outputs (2, or 1 for 2 lines) and IRQTXRX; they show the scanner position while IRQ is low, X otherwise |

Per line: MR1 / MR2 (an alternating pointer, reset by a Command read),
Command, Status, a two-entry receive FIFO, the THR, independent TX and RX
rates from CLK (`CLK / (16 × rate)`); the scanner steps one position per
CLK (receivers, then transmitters) and stops on an interrupt; the
Interrupt Summary and Data Set Change Summary (write 1 to clear). RDY (open
drain) follows CS. Timing names: `tdd`, `tddz` (read access, float), `tid`
(DS to IRQ release), `trdl`, `trdh` (CS to RDY).

### `pio`

Parallel I/O peripherals on the 65xx bus (plan-14): the WDC W65C21 PIA and
W65C22 VIA (`backend/src/models/pio.c`). The device's `"model"` picks the
behavioral model, a full pin-level model at the datasheet's times. Every
model talks to the bus through `bus65.c` (below).

| Key | Value |
|---|---|
| `"model"` | required. Models: B and C add `pia6521` and `via6522`. An unknown name is refused (`unknown pio model "x"`) |
| `"grades"` | required: grade name → the model's timing names (ns; a missing name is 0), plus optional `"vcc"`, `"inputMax"`, `"note"`. Every model's first timing name is `tcyc` |
| the model's keys | below; a key of another model is rejected (`"x" is not a key of model "y"`) |

Part keys: `"grade"` (a key of `"grades"`, required), optional `"vcc"`,
`"inputMax"`. `PartPio` holds the model, the grade and `max_hz` =
floor(10^12 / `tcyc` in ps). The listing's facts column reads
`model=<m> grade=<g> max_hz=<n>`.

Pin groups (presentation, § Pinouts and pins): D0–D7 `"data"`, PA0–PA7
`"port A"`, PB0–PB7 `"port B"`, RS0–RS3 `"register select"`, CS1 and CS2B
`"chip select"` (W65C22 only); every other pin the default group.

**The bus (`bus65`).** Chip selects (active-high `cs_high`, active-low
`cs_low`), RWB and the register selects are sampled at each PHI2 rise.
Selected = every active-high select reads 1 and every active-low one 0. A
select, RWB or RS that reads X or Z, or changed less than `tacr` (read) /
`tacw` (write) before the rise, makes the cycle unknown; so does one that
changes at the rise or within `tcar` / `tcaw` after it. A read drives the
data pins X from the rise and the register's value from rise + `tcdr`,
until fall + `thr`; an unknown read drives X for the same window. A write
takes the data at the PHI2 fall: a bit that changed less than `tdcw` before
the fall, or changes within `thw` after it, is unknown. An unknown write
makes every register unknown. PHI2 X makes every register unknown and the
data pins X until PHI2 is known again; so does a high or low phase shorter
than the model's minimum widths, or a cycle shorter than `tcyc`. Not
selected, the data pins stay Z. An unknown read makes every read side
effect unknown (a flag a read would clear goes X).

**Shared keys (every model).** `"phi2"`, `"rw"`, `"reset"` (pin names),
`"data"` (8 names, D0 first), `"csHigh"` (active-high selects), `"csLow"`
(active-low selects), `"rs"` (register selects, RS0 first), `"pa"`, `"pb"`
(8 names each, bit 0 first), `"ca1"`, `"ca2"`, `"cb1"`, `"cb2"`. A list of
the wrong length is refused (`"pa" needs exactly 8 pin names`).

#### Model `pia6521`

The W65C21 PIA (`datasheets/pio/w65c21.pdf`). Keys: the shared ones with
`"csHigh"` 2 names (CS0, CS1), `"csLow"` 1 (CS2B), `"rs"` 2; plus `"irqa"`,
`"irqb"` (the open-drain IRQAB/IRQBB pins). Timing names (ns, p.8): `tcyc`
tCYC and `tc` the PHI2 pulse width (both > 0; the bus checks both phases
against `tc`), `tacr`, `tcar`, `tpcr`, `tcdr`, `thr` (read), `tacw`,
`tcaw`, `tdcw`, `thw`, `tcpw` (write), `tca2` (PHI2 low to CA2 low),
`tca2h` (PHI2 low to CA2 high, pulse mode), `trs2` (CA1/CB1 active to
CA2/CB2 high), `tcb2` (PHI2 high to CB2 low), `tcb2h` (PHI2 high to CB2
high), `tdc`, `pwi` (interrupt input pulse width, a minimum), `trs3`
(interrupt response), `tir` (interrupt clear).

- **Registers.** RS1 picks the side; RS0 = 1 is CR, RS0 = 0 is OR when CR
  bit 2 = 1 and DDR when it is 0. CR bits 0–5 are written; bits 6 and 7
  are the flags (read only). A write reaches the register at the PHI2 fall;
  a port output follows `tcpw` later.
- **Ports.** DDR bit 1: output. Port A reads its pins (output bits too),
  drives an output 0 strong and an output 1 weak (its resistive pull-ups,
  p.16: a stronger driver wins and the read shows the pin), and pulls an
  input up (weak 1). Port B drives its outputs strong and reads them back
  from ORB; its inputs float (Z reads X). A pin that changed less than
  `tpcr` before the read's PHI2 rise reads X. Reading OR clears that side's
  flags at the PHI2 fall; IRQ is released `tir` later.
- **CA1, CB1.** The edge CR bit 1 picks (1 = rising) sets CR bit 7; IRQ goes
  low `trs3` later when CR bit 0 = 1. A pulse that returns within `pwi` of
  its active edge sets the flag X (the chip may have missed it). X on the
  input sets the flag X.
- **CA2, CB2 as inputs** (CR bit 5 = 0): the edge CR bit 4 picks sets CR
  bit 6, onto IRQ when CR bit 3 = 1; the same pulse rule. CA2 is pulled up
  (weak 1), CB2 floats.
- **CA2, CB2 as outputs** (CR bit 5 = 1; bit 6 reads 0), CR bits 4–3: `00`
  handshake — CA2 low `tca2` after the PHI2 fall of a read of ORA, CB2 low
  `tcb2` after the PHI2 rise that follows a write of ORB; high `trs2` after
  the next active CA1/CB1 edge. `01` pulse — low as for `00`, high again
  `tca2h` after the next PHI2 fall (CA2) / `tcb2h` after the next rise
  (CB2). `10`/`11` — the line is CR bit 3. Entering `00` or `01` sets the
  line high.
- **IRQAB, IRQBB** (open drain): low while (bit 7 ∧ bit 0) ∨ (bit 6 ∧ bit
  3 ∧ ¬bit 5), else Z; X when a term is X.
- **Reset and power-up.** RESB low clears every register (ports and CA2/CB2
  inputs, IRQs Z) and holds them; writes and edges are ignored. RESB X
  makes every register X. RESB at Z reads high. Registers power up X.
- Not modeled: port drive currents and the N/S buffer difference (the two
  dies are one model with the same timing).

#### Model `via6522`

The W65C22 VIA (`datasheets/pio/w65c22.pdf`). Keys: the shared ones with
`"csHigh"` 1 name (CS1), `"csLow"` 1 (CS2B), `"rs"` 4; plus `"irq"` (IRQB),
`"irqOpenDrain"` (bool, required: `true` = IRQB only pulls low, the
W65C22N; `false` = totem pole, the W65C22S) and `"busHold"` (bool,
required: `true` = every pin but PHI2 holds its last level when nothing
drives it, the W65C22S). Timing names (ns; Tables 4-3, 4-4): `tcyc`,
`tpwh`, `tpwl` (all > 0), `tacr`, `tcar`, `tcdr`, `thr`, `tpcr`, `tacw`,
`tcaw`, `tdcw`, `thw`, `tcpw`, `tca2`, `trs1`, `trs2`, `twhs`, `tds`,
`trs3`, `trs4`, `t21`, `til`, `tsr1`, `tsr2`, `tal`, `tpdh`. `tRS1` is
printed in µs; it is ns. `tipw`, `ticw`, `tips`, `tics` are two PHI2
cycles (measured), not keys.

- **Registers** (Table 2-1), RS 0–F: ORB/IRB, ORA/IRA (handshake), DDRB,
  DDRA, T1C-L, T1C-H, T1L-L, T1L-H, T2C-L, T2C-H, SR, ACR, PCR, IFR, IER,
  ORA/IRA (no handshake). Writes act at the PHI2 fall; port outputs follow
  `tcpw` later. A read's side effects (flag clears, the CA2 read strobe) act
  at its PHI2 fall.
- **Ports.** DDR 1 drives from OR (strong), DDR 0 floats. IRA reads the PA
  pins (output bits too); IRB reads ORB for outputs and the pins for
  inputs. A pin that changed less than `tpcr` before the read's rise reads
  X. Latching (ACR bit 0 PA, bit 1 PB): the read returns the pins as they
  were at the last active CA1/CB1 edge — X where a pin changed less than
  `til` before the edge or within `tpdh` after it — and, once read, the
  pins again until the next edge (p.8). Bus hold: an input at Z reads its
  last level (X before the first); without it Z reads X.
- **CA1, CB1.** The edge PCR bit 0 / bit 4 picks (1 = rising) sets IFR1 /
  IFR4 and latches the port when enabled.
- **CA2, CB2** (PCR bits 3–1 / 7–5): `000`/`010` input, falling/rising
  edge sets IFR0/IFR3, cleared by an ORA (RS 1) / ORB access along with
  IFR1/IFR4; `001`/`011` independent input (an ORA/ORB access leaves the
  flag); `100` handshake output (CA2 low `tca2` after the fall of a read of
  RS 1; CA2/CB2 low `twhs` after the rise that follows a write of RS 1/RS 0;
  high on the next active CA1/CB1 edge, `trs2` after a read strobe, `trs4`
  after a write strobe); `101` pulse output (after a read: low `tca2` after
  the fall, high `trs1` after the next fall; after a write: low `twhs` after
  the next rise, high `trs3` after the rise after that); `110`/`111` low /
  high. Entering `100`/`101` sets the line high. RS F reads and writes ORA
  with no handshake and no flag clear.
- **Timer 1.** A write of RS 5 loads the counter from the latches, clears
  IFR6 and starts it; the counter decrements at every PHI2 fall but the one
  that loaded it, so it reads N in the next cycle and 0xFFFF in cycle N + 2,
  and the time-out is at that cycle's PHI2 rise (N + 1.5 cycles after the
  write). One-shot (ACR bit 6 = 0): IFR6 once per start; the counter rolls
  on. Free-run: the counter reloads from the latch at the next fall, so
  time-outs come every L + 2 cycles; IFR6 each time. ACR bit 7: T1 drives
  PB7 whatever DDRB bit 7 is — low from the start write (+ `tcpw`), high at
  a one-shot time-out, toggled at each free-run time-out — and IRB bit 7
  reads it. RS 7 writes the high latch only and clears IFR6.
- **Timer 2.** One-shot as T1 one-shot, from a write of RS 9 (counter high
  ← data, counter low ← the low latch). Pulse counting (ACR bit 5): the
  counter decrements once for each PB6 low sampled at a PHI2 rise after a
  high one, and sets IFR5 when it reaches 0 (once per start); a PB6 pulse
  narrower than two PHI2 cycles, or one starting less than two cycles after
  the last ended, makes the count X.
- **Interrupts.** IFR bits 0 CA2, 1 CA1, 2 SR, 3 CB2, 4 CB1, 5 T2, 6 T1;
  IFR bit 7 reads (IFR ∧ IER) ≠ 0. Writing IFR clears the bits written 1.
  IER bit 7 = 1 sets, 0 clears the bits written 1; IER bit 7 reads 1. IRQB
  follows (IFR ∧ IER) ≠ 0 with no delay: low, or released (Z open drain, 1
  totem pole).
- **Reset and power-up.** RESB low clears every register but the T1 and T2
  counters and latches and the SR; every pin is an input; the timers stop
  until their start write. RESB X makes every register X; RESB at Z reads
  X, or its held level with bus hold. Registers power up X.
- **Shift register** (ACR bits 4–2, §2.11–2.13). Shifting out sends bit 7
  first on CB2 and rotates it into bit 0; shifting in enters at bit 0. A
  read or write of the SR restarts its bit count, clears IFR2 and starts the
  internal clock. `000`: no shifting, IFR2 held 0, CB1/CB2 under the PCR.
  `001`/`101` (T2 rate): CB1 is an output clock that toggles every N + 2
  PHI2 cycles (N = the T2 low latch; T2's low counter reloads from it at
  each toggle and T2 sets no IFR5); `010`/`110` (PHI2 rate): CB1 low one
  cycle, high the next. Out (`101`, `110`): CB2 changes `tsr1` after the
  PHI2 fall where CB1 falls; after 8 bits IFR2 is set, CB1 stays high and
  CB2 holds the last bit. In (`001`, `010`): CB2 is sampled at the PHI2
  rise after CB1 rises (X if it changed less than `tsr2` before) and shifted
  in at that cycle's fall; after 8, IFR2, CB1 stays high. `100`: as `101`
  but it never stops and never sets IFR2. External clock (`011` in on CB1
  rises, `111` out on CB1 falls): the shift happens at the PHI2 fall after
  the edge, IFR2 every 8 bits, and it keeps going; an edge less than `tpcr`
  before that fall (the sheet's "no shift" hazard, p.19) makes the SR and
  its count X. In shift modes CB2 is the serial line whatever the PCR says
  (its PCR edge flags are off); CB1 edges still set IFR4 by PCR bit 4,
  including the VIA's own shift clock.

### `7seg`

LED 7-segment displays. A display is a set of LEDs, each between one
segment pin and its digit's common pin(s). An LED is on while its anode
reads 1 and its cathode reads 0 (common anode: the common is the anode;
common cathode: the common is the cathode). A digit's common reads as the
first of its common pins that is not Z; two common pins of one digit are
joined inside the part. The display drives nothing. Lights report "on at
any time during the last step", so a multiplexed digit counts as lit.

```json
"digits": 4, "commonAnode": false, "colonAfter": 2, "dotAfter": 0,
"color": "Red", "size": "0.4 in", "chip": "AlInGaP Super Red",
"leds": [{"digit": 1, "segment": "A"}],
"segmentPins": {"A/L1": {"digit": 0, "segments": ["A", "L1"]}},
"commonPins": {"CC1": 1, "CC L": "extras"},
"segmentSide": ["A/L1", "B/L2"],
"commonSide": ["CC1", "CC2", "CC L"]
```

| Key | Value |
|---|---|
| `"digits"` | number of digits (1–8) |
| `"commonAnode"` | `true` for common anode, `false` for common cathode |
| `"colonAfter"`, `"dotAfter"` | the digit the colon (L1, L2) / raised dot (L3) sits after; 0 = none |
| `"color"`, `"size"`, `"chip"` | shown as is (`"Red"`, `"0.56 in"`, LED chip) |
| `"leds"` | every LED in display (and lights) order: `digit` (1…, or 0 = the colon/dot group) and `segment` (`A`–`G`, `DP`, `LDP`, `HJ`, `L1`–`L3`) |
| `"segmentPins"` | segment pin name → `digit` (0 = shared by every digit, multiplexed) and the `segments` it drives |
| `"commonPins"` | common pin name → its digit, or `"extras"` for the colon/dot common. A name used by several pins means all of them. |
| `"segmentSide"`, `"commonSide"` | drawing order of the symbol's two sides, by name; a shared name expands to its pins in pinout order. `PartDisplay.segment_pins`/`common_pins` hold these pins' indices into `Part.pins`. |
| `"vf"` | `[min, max]` volts: the segment forward voltage (datasheet "Forward Voltage Per Segment"), 0 < min ≤ max ≤ 10; min is the datasheet minimum, or its typical value when no minimum is printed. `PartDisplay.vf_min_mv`/`vf_max_mv`. Shown, not checked. Required. |

An LED's segment pin is the first pin (in pin order) in `segmentPins` that
drives its segment for its digit (or every digit); its commons are the
first two pins of its digit in `commonPins`. Unused positions are pins with
role `nc` (no connect) or `absent` (no lead). Parts carry no kind keys.

### `charlcd`

Character LCD modules: HD44780-compatible modules with the controller,
character ROM, glass and backlight built in (plan-11). The parts drawer
lists them under Displays → Character. The example is the
`NHD-0216K1Z-NSW` device (part NHD-0216K1Z-NSW-BBW-L):

```json
"columns": 16, "rows": 2, "controller": "ST7066U", "rom": "A00",
"panel": "blue-negative",
"bus": {"rs": "RS", "rw": "R/W", "e": "E",
        "data": ["DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"]},
"contrast": "V0",
"backlight": {"anode": "LED+", "cathode": "LED-", "color": "White"},
"timing": {"tc": 1200, "tpw": 140, "tas": 0, "tah": 10, "tdsw": 40, "th": 10,
           "tddr": 100, "tdhr": 10},
"exec": {"instruction": 37000, "data": 37000, "clearHome": 1520000,
         "powerOn": 40000000},
"vcc": [4.5, 5.5]
```

| Key | Value |
|---|---|
| `"columns"`, `"rows"` | characters per line (1–40) and lines (1–2) |
| `"controller"` | shown as is, e.g. `"ST7066U"` |
| `"rom"` | `"A00"`, the only character ROM the model has |
| `"panel"` | `"yellow-green"` (STN positive: dark dots on yellow-green) or `"blue-negative"` (STN negative: light dots on blue) |
| `"bus"` | `{"rs", "rw", "e", "data"}`: pin names; `data` is 8 names, DB0 first. Each names exactly one pin. `PartCharLcd.bus_pins` holds RS, R/W, E, DB0–DB7 as indices into `Part.pins` |
| `"contrast"` | the V0 pin's name (one pin); the model ignores it |
| `"backlight"` | optional `{"anode", "cathode", "color"}`: pin names (one pin each) and the shown color. Without it `PartCharLcd.backlight_color` is NULL |
| `"timing"` | ns: `tc tpw tas tah tdsw th tddr tdhr` (the datasheet's bus timing), every key required, each ≥ 0; `tc` and `tpw` > 0 |
| `"exec"` | ns: `instruction` (every instruction but Clear and Home), `data` (a data read or write), `clearHome`, `powerOn` (busy time after power-up), each > 0 |
| `"vcc"`, `"inputMax"` | the supply range (§ Supply voltage); also allowed on the part |

Part keys: `"vcc"`, `"inputMax"` (optional; replace the device's). Parts
name a `"connectorFootprint"` as well as their `"footprint"`.

Behavior (the HD44780 family as the module datasheets print it;
`planning/completed/plan-11-B-charlcd-kind.md` §4 has every rule):

- **Pins.** RS, R/W, E, V0 and the backlight pins are inputs; DB0–7 are
  inout. Z reads as X on every input. V0 is ignored.
- **Bus.** An access starts when E rises and ends when it falls. A read
  drives DB X from the rise, the data from + `tddr`, and Z from the fall
  + `tdhr`. A write samples DB, RS and R/W at the fall and takes effect at
  the fall + max(`th`, `tah`). Timing violations: less than `tas` from RS
  or R/W to the rise, less than `tc` between rises, E high less than
  `tpw`, a sampled DB pin changing less than `tdsw` before the fall or
  less than `th` after it, RS or R/W changing less than `tah` after it.
- **4-bit mode** (DL = 0): only DB4–7 are read and driven; a byte is two
  accesses, high nibble first.
- **Busy.** A write that takes effect sets BF for its `exec` time
  (`clearHome` for Clear and Home, `instruction` for other instructions,
  `data` for data). A write while BF = 1 is not accepted. Power-up: BF = 1
  for `powerOn`, with the HD44780 internal-reset state (8-bit, 1 line,
  display off, I/D = 1, DDRAM all spaces, CGRAM X).
- **Instructions:** Clear, Home, Entry mode, Display on/off, Cursor or
  display shift, Function set, Set CGRAM and DDRAM address; data reads and
  writes move AC by I/D (and shift the display when S = 1 on a write).
  DDRAM has 80 cells (2-line: addresses 00–27 and 40–67; 1-line: 00–4F);
  an address with no cell reads X and takes no write.
- **X.** Registers are tracked one field at a time, each known or X. A
  data write with a violation stores its cell as X; an instruction write
  with a violation or an unknown instruction, RS or R/W X at a write, an
  access with R/W X, or E reading X while R/W does not read 1 makes every
  field, every RAM cell and BF X ("LOST"), except during the power-on
  reset. E reading X with R/W reading 1 is a possible read: only AC (unless
  RS reads 0) and the 4-bit nibble phase become X. A write of `0011` on DB7–4 with RS = 0 while
  DL or the 4-bit nibble phase is X is Function Set with DL = 1 (the
  datasheets' initialization by instruction).
- **Image.** The backend renders the 5×8 dot image from DDRAM, CGRAM (codes
  00–0F), the character ROM A00 (10–FF), the display shift, the cursor
  (dot row 7 of AC's cell) and blink (all dots of AC's cell, on while
  `floor(t / 379 259 259 ps)` is even). A dot is X when anything it depends
  on is X. `circuit_lcd_state` reads it.
- **Backlight.** One light, on while the anode reads 1 and the cathode
  reads 0. The voltage check carries the anode's level to the cathode net,
  as for an LED, and checks VDD against `vcc`.

### `buffer`

Buffers with output enables, bus transceivers, registered and latched
transceivers, and level translators (plan-20). Each device names a
**model**: `tristate`, `transceiver`, `registered` or `translator`; the
model decides the rest of the device keys (`registered`: below). `timing` entries are as in the
sequential kinds: family → `{rise, fall}` (ns, the model's main path:
A → Y, A → B, clock → B for `registered`), plus `note`, `vcc`, `inputMax`.
Part keys: `family`, optional `vcc` and `inputMax`. `PartLogic.units` is
the number of bits (buffers or transceiver channels) of the part.

Shared keys:

| Key | Value |
|---|---|
| `"units"` | 1–8 unit objects (the model's keys below); a unit is one group of bits sharing its enables. At most 40 bits per unit and 64 per part. |
| `"invert"` | optional, default `false`: every path inverts ('240, '540, '640). A unit's own `"invert"` overrides it. |
| `"output"` | optional: `push-pull` (default) or `open-drain` ('641, '756, open-collector parts): a 1 releases the pin. Applies to every output and port. |

An **enable** (`oe`, `oeab`, `oeba`) is one control `{"pin", "active"}` or
an array of 0–4 controls that must all be asserted ('541 OE1·OE2; `[]` =
always enabled). An enable pin at X or Z makes what it gates X.

**Ports.** A pin a part drives is never read back as an input: after a
port's drive is released (a direction change), the port reads X until the
release has taken effect (the rise delay), then the net's value. So a
transceiver turning around shows X on the new destination for that time
rather than its own old output.

Signal rules otherwise as in [Sequential kinds](#sequential-kinds-latch-counter-shift):
a data input at Z reads X; outputs are inertial (fall delay for 0, rise
for 1 and Z, the longer for X); bus hold is not modeled.

#### Model `tristate`

Y = A (or NOT A) while every enable of the unit is asserted; Z while one is
deasserted; X while one is unknown and none is deasserted.

| Unit key | Value |
|---|---|
| `"a"` | input pin names (a name may repeat: '16344 drives four outputs from one input) |
| `"y"` | output pin names, as many as `a` |
| `"oe"` | optional enable; absent = always enabled |
| `"invert"` | optional, overrides the device's |

#### Model `transceiver`

Two ports per unit, `"a"` and `"b"` (inout pins, as many each), in one of
two forms:

| Form | Unit keys |
|---|---|
| DIR + OE ('245) | `"dir": {"pin": <name>, "ab": "high"\|"low"}` — the DIR level that means A → B; `"oe"`: optional enable (absent: always on, 1T45) |
| one enable per direction ('243, '620, '623, '638 inverted) | `"oeab"`, `"oeba"`: enables of the A → B and B → A buffers |

The enabled direction's destination drives the source's value (inverted
with `invert`); a port of a direction that is off is Z and reads. X on DIR
(with OE not deasserted) makes both ports X. Both directions on (the '623
latch row, `oeab` and `oeba` asserted together): each port drives the
value last passed in either direction — the two buffers form a loop that
holds the bus value present when the second enable arrived (X if both
came at once).

#### Model `registered`

Transceivers with a latch or a register in each direction ('646/'648,
'652/'654, '651/'653, '543/'544, '2952/'2953 and their 16-bit versions).
Main path: clock → B (or LE → B for latches). Unit keys: `"a"`, `"b"` (inout,
as many each), `"ab"` and `"ba"` (the A → B and B → A directions), optional
`"invert"`; or additionally `"dir"` and `"oe"` at the unit ('646 form: one
OE, DIR picks the driven side, as in the '245).

| Key (inside `ab` / `ba`) | Value |
|---|---|
| `"store"` | `"latch"` or `"register"` |
| `"le"` | latch only, required: control; the latch is transparent while asserted ('543 LEAB low) |
| `"clk"` | register only, required: `{"pin", "edge": "rising"\|"falling"}` |
| `"ce"` | optional control. A latch: transparent only while `le` and `ce` are asserted, and the destination drives only while `oe` and `ce` are ('543 CEAB). A register: the edge loads only while `ce` is asserted ('2952 CLKENAB). |
| `"oe"` | enable: the destination drives while it is asserted; required unless the unit has `dir` (then not allowed) |
| `"select"` | optional control: asserted → the destination shows the stored value; deasserted → the source's real-time value ('646 SAB). Without it the destination always shows the stored value (the '543 latch output, the '2952 register). |
| `"invert"` | optional: the destination shows the inverse ('648, '651, '653, '654) |

Stored bits are 0 at power-on. A latch or register takes its source
port's value whether or not the part itself drives that port (the '646
stores the bus it drives). A clock edge from X or Z, or a move from the
pre-edge level into X or Z, may have been an edge: bits it would change go
X. Both directions enabled and both showing real time hold the value last
passed, as the transceiver model's both-enabled row.

#### Model `translator`

Dual-supply level translators. The logic is the transceiver's (units with
`"b"`: DIR-controlled, `oe` optional — 1T45 has none; per-bit DIR parts
such as the 4T774 have one unit per bit) or the tristate's (units with
`"y"`: fixed direction, TXU/TXV, 8T540-style, CD4504B with no enable). The
device also has `"domains"` (required) and every timing entry `vccB`
([Two supply domains](#two-supply-domains-domains-vccb-inputmaxb)); the
domains change only the voltage check, not the logic. Timing is one pair
per family, from the datasheet's VCCA 3.3 V ↔ VCCB 5 V row or the nearest
one. A translator's outputs do not float when one side is unpowered (the
engine has no supply level per part).

### Sequential kinds (`latch`, `counter`, `shift`)

The three sequential kinds share their device and part format and their
signal rules. Each device names a **model**; the model decides the rest of
the device's keys.

| Key | Value |
|---|---|
| `"model"` | required: one of the kind's models (see each kind). A key of another model of the same kind is an error: `"<key>" is not a key of model "<model>"`. |
| `"timing"` | logic family → `{rise, fall}` (ns, the model's main path), plus optional `note`, `vcc`, `inputMax` and the model's extra timing keys (named in the model's table). Families as in [Values](#values). |

Part keys: `"family"` (a key of `timing`), optional `"vcc"` and
`"inputMax"`, as on `dff`. `PartLogic` is {family, units}; each model
says what `units` counts.

**Controls.** A control is an object
`{"pin": "<name>", "active": "high" | "low"}`; the pin name must be exactly
one pin in every pinout. A model says whether each control acts at once
(asynchronous: clear, preset, reset, set, asynchronous load) or is sampled
at a clock edge. Where a model allows both, the control also has
`"when": "async" | "clock"`.

**Signal rules** (every model):

- **Power-on**: every stored bit is 0 (latches, counters, registers, shift
  stages; a FIFO starts empty); then the asynchronous controls apply. The
  datasheets say "unknown"; 0 keeps circuits deterministic (as `dff`).
- **Edges** follow Verilog posedge/negedge: any move into 1 (rising) or into
  0 (falling) from another value, including from X or Z.
- **Asynchronous controls**: Z reads inactive; X makes every bit they could
  change X.
- **Output enables**: active → driven, inactive → Z, X or Z → X.
- **Every other input** (data, clocks, enables, mode and select pins): Z
  reads as X. An X on a synchronous enable or mode pin at a clock edge makes
  every bit that edge could change X; an X data bit stores X.
- **Outputs** are driven strong and inertially: the fall delay for 0, the
  rise delay for 1 and Z, the longer for X; rescheduled only when their value
  changes. Open-drain outputs (`"output": "open-drain"`) drive 0 and release
  to Z for 1; the voltage check treats them as carrying no voltage.
- **Bus hold** (ALVCH, LVCH, LVTH) is not modeled: a floating input reads X.

### `latch`

Level-sensitive storage: transparent D latches, addressable latches, S-R
latches, a BCD-to-7-segment latch/decoder and a multiplexed bus latch. A
sequential kind (above): `"model"` is one of `transparent`, `addressable`,
`sr`, `bcd7seg`, `bus-mux`, and `timing` entries are `{rise, fall}` for the
model's main path. Every model re-evaluates on any pin change; an unknown
control makes X exactly the bits whose result depends on it.

#### Model `transparent`

Main path D → Q; `units` = the number of units.

| Key | Value |
|---|---|
| `"units"` | 1–40 objects: `d`, `le` (required pin names), `q`, `qn`, `oe`, `pre`, `clr` (optional; at least one of `q`/`qn`). Units may share pins (a common LE or OE). |
| `"enableActive"` | `"high"` or `"low"`: the `le` level that makes a unit transparent. Required unless `"polarity"`. |
| `"polarity"` | optional pin name (CD4042B): a unit is transparent while its `le` level equals this pin's level. Not with `"enableActive"`. |
| `"outputEnableActive"` | `"high"`/`"low"`: the `oe` level that enables the outputs. Required when any unit has `oe`. |
| `"presetActive"`, `"clearActive"` | `"high"`/`"low"`; required when any unit has `pre` / `clr`. |
| `"presetWins"` | bool; required when some unit has both: true → both asserted store 1, false → 0. |
| `"output"` | `"push-pull"` (default) or `"open-drain"`. |

Per unit: `clr`/`pre` act at once; else while transparent the stored bit
follows D; else it holds. `q` shows the bit, `qn` its complement, both
through the unit's `oe` (OE gates only the outputs; the latch keeps working).

#### Model `addressable`

Main path D → Q; `units` = the number of outputs.

| Key | Value |
|---|---|
| `"address"` | 1–3 pin names, LSB first |
| `"data"` | pin name |
| `"outputs"` | 2^(address pins) pin names; `outputs[i]` is latch i |
| `"enable"` | control: the addressed latch is transparent while it is asserted |
| `"clear"` | optional control (asynchronous) |
| `"output"` | `"push-pull"` (default) / `"open-drain"` |
| `"invert"` | bool, default false: the pin shows NOT the bit (TPIC6259: DRAIN on when the bit is 1) |

Per latch i: enable asserted and address = i → bit = D; else clear asserted
→ 0; else hold. This gives the 74x259's four modes (addressable latch,
memory, demultiplexer, clear).

#### Model `sr`

Main path S → Q; `units` = the number of units.

| Key | Value |
|---|---|
| `"units"` | 1–8 objects: `s` (1–2 pin names; any asserted sets), `r`, `q` |
| `"inputsActive"` | `"high"` / `"low"` (S and R) |
| `"both"` | `"set"` / `"reset"`: Q while S and R are both asserted |
| `"outputEnable"` | optional control, common to every Q (active → driven, else Z) |

Set and not reset → 1; reset and not set → 0; both → per `"both"`; neither
→ hold. Releasing S first from both-asserted gives 0, releasing R first
gives 1 (the 74x279's gates).

#### Model `bcd7seg`

Main path input → segment; `units` = 1.

| Key | Value |
|---|---|
| `"inputs"` | 4 pin names, LSB first |
| `"latch"` | control: the code is held while asserted (74HC4511 LE active high) |
| `"lampTest"`, `"blank"` | optional controls |
| `"segments"` | 7 pin names, a … g |
| `"glyphs"` | 16 strings of 7 characters `0`/`1` (segments a…g) for codes 0–15 |

The stored code follows the inputs while `latch` is not asserted. Lamp test
asserted → every segment 1; else blank asserted → every segment 0; else the
glyph of the stored code.

#### Model `bus-mux`

Main path B → A; `units` = 12.

| Key | Value |
|---|---|
| `"a"`, `"b1"`, `"b2"` | 12 pin names each (inout) |
| `"select"` | pin name: high → A shows the 1B latch, low → the 2B latch |
| `"latchEnables"` | `{"b1", "b2", "a1", "a2"}` → pin names (transparent while high) |
| `"outputEnables"` | `{"a", "b1", "b2"}` → pin names (enabled while low) |

Latch L1B takes pin 1Bn, L2B takes 2Bn, LA1B and LA2B take An;
An = OEA low ? (SEL ? L1B : L2B) : Z; 1Bn = OE1B low ? LA1B : Z;
2Bn = OE2B low ? LA2B : Z. Latches read the pin's net value (the part's own
drive included, as in the hardware). Bus hold is not modeled.

### `decoder`

Combinational decoders and encoders (plan-19): binary decoders and
demultiplexers (74x138, 74x139, BCD-to-decimal 74x42 and 4028B, 4-to-16
74x154 and 4514B, the address-latched 74x137, the SN74BCT2414 memory
decoder, the 1G parts), priority encoders (74x147, 74x148, 74x348, 4532B,
40147B) and BCD-to-7-segment decoders/drivers (SN7447A, SN74LS49). It
shares the sequential kinds' device format (above): `"model"` is one of
`binary`, `priority`, `seg7`; `timing` entries are `{rise, fall}` for the
model's main path; part keys `family`, `vcc`, `inputMax`. Every model
re-evaluates all of its outputs on any pin change and takes `"output"`:
`"push-pull"` (default) or `"open-drain"` (open-collector parts: drive 0,
release to Z). Signal rules as the sequential kinds: an address latch or
register and an input latch hold 0 at power-on; Z on an input reads X.

#### Model `binary`

Main path address → output; `units` = the number of unit objects.

| Key | Value |
|---|---|
| `"units"` | 1–4 objects: `address` (1–4 pin names, LSB first), `outputs` (1–16 pin names, at most 2^address; `outputs[i]` is selected by code i, and a code ≥ the number of outputs selects none: 74x42, 4028B), `enables` (optional, 0–3 controls; the unit is enabled while every one is asserted), `latch` (optional pin name: a transparent latch on the address) or `clock` (optional pin name: the address is registered on its rising edge). Units may share pins: '155 units share the address, BCT2414 units share G and SD, '1G29's second unit takes the first unit's address pin as an enable. |
| `"outputsActive"` | `"high"` / `"low"`: the level of the selected output |
| `"unselected"` | `"inactive"` (default) or `"z"`: what an output that is not selected shows (SN74LVC1G18: high impedance) |
| `"disabled"` | `"inactive"` (default) or `"z"`: what every output shows while the unit is not enabled (with `"unselected": "z"`, only the selected output) |
| `"latchActive"` | `"high"` / `"low"`: the `latch` level that **holds** the address (74x137 LE: `"high"`; 4514B STROBE passes while high: `"low"`). Required when a unit has `latch`, an error otherwise. |
| `"monitor"` | optional pin name: an open-collector supply-good output (SN74BCT2414 VS), driven low; the engine has no supply failure |

Enabled and code = i → output i active, the others inactive (or Z per
`unselected`); not enabled → every output inactive (or Z per `disabled`).
An X or Z on an enable makes every output it gates X; an X address bit
makes X exactly the outputs that differ across its completions.

#### Model `priority`

Main path input → code; `units` = 1.

| Key | Value |
|---|---|
| `"inputs"` | 4–10 pin names; `inputs[i]` has priority i (the highest active index wins) |
| `"inputsActive"` | `"high"` / `"low"` |
| `"code"` | 2–4 pin names, LSB first: the winning index plus `offset`, as a binary number |
| `"codeActive"` | `"high"` / `"low"` (`"low"`: each pin shows the inverted bit, 74x148) |
| `"offset"` | optional, default 0: added to the index (74x147: inputs 1–9 → 1) |
| `"idle"` | optional, default 0: the code value shown while no input is active (before inversion; 40147B: 15) |
| `"enableIn"` | optional control (EI) |
| `"groupSelect"`, `"enableOut"` | optional controls naming an output pin and its active level: GS is active while enabled and some input is active, EO while enabled and none is |
| `"disabled"` | `"inactive"` (default: the code shows `idle`) or `"z"` (74x348: the code pins are Z while disabled or idle; GS and EO stay driven) |

`offset` + the number of inputs must fit the code, and `idle` must be
below 2^code. An X input makes X the outputs whose value differs between
the cases it allows; an X `enableIn` makes every output X.

#### Model `seg7`

Main path BCD input → segment; `units` = 1.

| Key | Value |
|---|---|
| `"inputs"` | 4 pin names, LSB first |
| `"segments"` | 7 pin names, a … g |
| `"segmentsActive"` | `"high"` / `"low"`: the pin level of a lit segment (SN7447A: low) |
| `"glyphs"` | 16 strings of 7 characters `0`/`1` (segments a…g lit) for codes 0–15, from the datasheet's display figure |
| `"lampTest"`, `"blank"` | optional controls (LT, BI) |
| `"rbi"`, `"rbo"` | optional pin names, given together (SN7447A): RBI is read active low; RBO (role `inout`, open drain) is pulled low while RBI is low, the code is 0 and lamp test is off; read as an input, a low on it blanks every segment (Z reads high) |
| `"latch"` | optional control: the code is held while asserted (CD4056B strobe) |
| `"phase"` | optional pin name: every segment level is XORed with it (CD4056B DF, liquid-crystal drive) |

Priority: blank (`blank` asserted or RBO low) > lamp test (every segment
lit) > ripple blanking (RBI low and code 0) > the glyph of the code. The
SN7447A's BI wins over LT (sn7447a p.3, notes 2 and 4).

### `mux`

Multiplexers / data selectors (plan-21): the plain selectors (74x151,
'153, '157, '158, '251, '253, '257, '258, the 2G157 and 1T157/1T158 single
gates, 74AC11257, CD4512B, CD40257B, CY74FCT2257T), the selectors with
storage ('298, '399, '354, '356) and the '670 register file. It shares
the sequential kinds' device format (above): `"model"` is one of `select`,
`stored`, `regfile`; `timing` entries are
`{rise, fall}` for the model's main path; part keys `family`, `vcc`,
`inputMax`. Every model re-evaluates all of its outputs on any pin change.
Signal rules as the sequential kinds: stored bits hold 0 at power-on; Z on
an input reads X; an output enable deasserted → Z, X → X.

**Selector units** (`select`, `stored`): `"units"` is 1–6 objects:

| Unit key | Value |
|---|---|
| `data` | 2–16 pin names; `data[i]` is chosen by select code i. Exactly 2^(select pins) names. |
| `select` | 1–4 pin names, LSB first. Usually shared by every unit. |
| `y` | optional pin name: the true output |
| `w` | optional pin name: the complement output (W = NOT Y). At least one of `y`/`w`. |
| `strobe` | optional control: deasserted forces the unit's outputs to the `disabled` state |
| `oe` | optional control: output enable, separate from the strobe ('251 G, CD4512B DISABLE) |

with device keys `"disabled"` (`"low"`, the default: a deasserted strobe
gives Y low and W high, '151/'153/'157; or `"z"`: both Z, '253/'257),
`"invert"` (bool, default false: Y shows NOT the chosen data, '158/'258/'352)
and `"output"` (`"push-pull"`, the default, or `"open-drain"`).

Per unit: OE deasserted → Y and W Z; else strobe deasserted → the
`disabled` state (with `invert`, Y high); else Y = data[code] (inverted per
`invert`), W = NOT Y. An X or Z select bit gives X unless every data input
it chooses between has the same known value; an X strobe or OE gives X.

#### Model `select`

Main path data → Y; `units` = the number of unit objects. Keys: `units`,
`disabled`, `invert`, `output` (above), plus:

| Key | Value |
|---|---|
| `"selectLatch"` | optional control: a transparent latch on every unit's select inputs, holding while asserted |

#### Model `stored`

A selector with storage in front of or behind it. Main path: data → Y
(data latches), clock → Q (registers); `units` = the number of unit
objects. Keys of `select` (`units`, `disabled`, `invert`, `output`,
`selectLatch`) plus at least one storage key:

| Key | Value |
|---|---|
| `"dataLatch"` | optional control: a transparent latch on every data input, holding while asserted (CD74HC354 E̅: `{"pin": "E", "active": "high"}`) |
| `"dataClock"` | optional clock `{"pin", "edge": "rising"|"falling"}`: an edge-triggered register on every data input (CD74HCT356 CP). Not with `dataLatch`. |
| `"outputClock"` | optional clock: an edge-triggered register on each unit's selected value; Y shows the register ('298 CLK falling, '399 rising) |
| `"oes"` | optional, 1–3 controls shared by every unit: the outputs are driven only while all are asserted ('354 OE1̅, OE2̅, OE3) |

Data and select latches and the registers are 0 at power-on. A clock that
moves from its idle level to X or Z makes X the stored bits it could
change; an X latch control does the same while the input differs from
the stored bit.

#### Model `regfile`

Register files with independent write and read ports ('670). Main path:
read address → Q; `units` = the number of files.

| Key | Value |
|---|---|
| `"words"` | 2, 4, 8 or 16 |
| `"bits"` | 1–8 |
| `"files"` | 1–2 objects: `d` (`bits` pin names), `wa` (log2 `words` pin names, LSB first), `we` (control: while asserted the addressed word follows D, transparently), `ra` (read address, as `wa`), `re` (control: read enable; deasserted → Q Z), `q` (`bits` pin names) |
| `"output"` | `"push-pull"` (default) / `"open-drain"` |

Q = the word at the read address, read combinationally from the array
(a write to the word being read shows on Q). Every word is 0 at power-on.
An X write enable or write address bit makes X the bits it could change;
an X read address bit gives X unless every word it chooses between has
the same bit.

### `counter`

Counters and dividers. A sequential kind (above); `"model"` is one of
`sync`, `ripple` (and the models of the sections below). The main timing
path is clock → Q (or clock → the register's Q where the only Q outputs
are a register's). No counter has open-drain outputs.

#### Model `sync`

Synchronous counters, including the counters with input or output
registers. Device keys: `"units"` (1–2 unit objects; `PartLogic.units` =
their number) and, on the 74x8154 only, `"byteBus"`. Unit keys:

| Key | Value |
|---|---|
| `"stages"` | 1–32: count bits |
| `"code"` | `"binary"` (mod 2^stages) or `"bcd"` (stages a multiple of 4; each digit counts 0–9; max = every digit 9) |
| `"codeSelect"` | instead of `"code"` (4029B): `{"pin", "binary": "high"|"low"}`, read at each edge; `stages` is 4 |
| `"q"` | optional: `stages` pin names, LSB first |
| `"clock"` | `{"pin", "edge": "rising"|"falling"}`; or instead `"clockUp"` and `"clockDown"` (pin names): count up on `clockUp` rising while `clockDown` is 1, down on `clockDown` rising while `clockUp` is 1 |
| `"clockGate"` | optional `{"pin", "op"}`: the counter steps on the edge of the gated clock, CLK·¬G (`and-not`), CLK+G (`or`) or CLK+¬G (`or-not`) |
| `"count"` | `"up"` or `"down"`; or `"direction"`: `{"pin", "up": "high"|"low"}`; or neither when `"modes"` or the dual clocks give it |
| `"enables"` | optional controls, all asserted to count (sampled at the edge) |
| `"enablesAny"` | optional controls, at least one asserted to count |
| `"clears"` | optional, in priority order: `{"pin", "active", "when": "async"|"clock", "to": "zero"|"max"}` |
| `"loads"` | optional, after the clears: `{"pin", "active", "when", "from": "data"|"register"}` (an asynchronous load is level-transparent) |
| `"data"` | load data pin names, LSB first (required by a load from `data` and by `modes`) |
| `"modes"` | optional (74x867/869): `{"pins": [S0, S1], "table": [4 of "clear", "down", "load", "up", "hold"], "clear": "async"|"clock"}`, row = S0 + 2·S1. A `load` row loads `data` at the edge regardless of the enables; `up`/`down` rows count when the enables are asserted |
| `"illegal"` | optional, `bcd` only: `{"up": [16], "down": [16]}`, the next value of a digit 0–15 (entries 0–9 must be the normal sequence). Without it a digit 10–15 steps to value+1 / value−1 within 0–15. A digit carries to the next one only from 9 (up) or 0 (down). |
| `"carries"` | optional: `{"pin", "active", "at": "max"|"min"|"terminal", "gates": [controls], "clockLow": <pin>}` — asserted while the count is at max / 0 / the terminal of the current direction (max up, 0 down; never in a non-counting `modes` row), every gate asserted, and the `clockLow` pin 0 |
| `"outputEnable"` | optional control gating `q` |
| `"register"` | optional output register: `{"clock": {"pin","edge"}, "outputs": [stages names], "outputEnable": control, "select": {"pin", "register": "high"|"low"}}` — takes the count (as it was before a counter edge at the same instant, so tied clocks leave it one count behind); `outputs` show the register, or with `select` the register while the pin is at `register` and the count otherwise |
| `"inputRegister"` | optional: `{"clock": {"pin","edge"}, "data": [names] (absent: the `bus` pins), "enable": control}`; a load `from` `"register"` reads it |
| `"bus"` | optional (74x593): `{"pins": [stages names, inout], "outputEnables": [controls]}` — the count drives the pins while every enable is asserted |
| `"zeroLatch"` | optional (4522B): `{"pin", "cascade": <pin>}` — a set/reset latch driving `pin`: set while the count is 0 and `cascade` is 1; reset (winning) while the first load is asserted with the gated clock at 0, or the count is not 0 with the load not asserted |

`"byteBus"` (74x8154): `{"outputs": [8 names], "selects": [{"pin", "unit", "byte"}]}`
— the outputs show byte `byte` (0 = low) of unit `unit`'s register while
that select pin is low; the first low select in list order wins; none low →
Z; an X select → X.

Order on any input change, per unit: the input register and the output
register take their clocks' edges; then the asynchronous clears in order,
the asynchronous loads, an asynchronous `modes` clear; when none is asserted
and the (gated or dual) clock has its edge: the clock clears, the clock
loads, the `modes` row, then counting when the enables allow it.

#### Model `ripple`

Ripple counters. Device keys `"units"` (the number shown in the drawer) and
`"sections"` (1–4). Section keys:

| Key | Value |
|---|---|
| `"clock"` | pin; every structure counts on its falling edge |
| `"structure"` | `"binary"` (`"stages"` 1–14; stage k toggles when stage k−1 falls), `"div2"` (one stage), `"div5"` (QB, QC, QD count 0–4: the 74x90/390 ÷5; illegal 5, 6 → 2, 7 → 0), `"div6"` (QB, QC a ÷3, QD toggles when QC falls: the 74x92) |
| `"q"` | a pin name per stage, or `null` for a stage with no pin (4020B Q2, Q3) |
| `"reset"` | optional `{"pins": [...], "active"}`: asserted while every listed pin is at `active` |
| `"set"` | optional `{"pins", "active", "value"}` (74x90 R9: the ÷2 section's value 1, the ÷5 section's 4) |

Priority: set > reset > clock. Timing entries need the extra key
`"stage"` (ns): an output's delay is the entry's rise/fall plus `stage` × its
ripple depth (binary: k for stage k from 0; div5: QC 1; div6: QD 1; else 0).

#### Model `johnson`

Johnson (twisted-ring) counters with decoded outputs: 4017B, 4022B,
74HC4017, 4018B. `units` = 1; main path clock → decoded output.

| Key | Value |
|---|---|
| `"stages"` | 4 or 5 |
| `"clock"` | pin; the stages advance on the rising edge of the (gated) clock |
| `"clockGate"` | optional `{"pin", "op"}` (as `sync`; 4017B CLOCK INHIBIT and 74HC4017 CE use `and-not`) |
| `"reset"` | control, asynchronous: every stage 0 |
| `"outputs"` | optional: 2 × stages decoded pins in count order |
| `"carry"` | optional pin: NOT the last stage (high for the first half of the cycle) |
| `"data"` | optional pin (4018B): stage 1's D; without it, NOT the last stage |
| `"presetEnable"` | optional control, asynchronous (4018B): stage n takes `jam`[n] |
| `"jam"` | pin names, one per stage (with `presetEnable`) |
| `"qbar"` | optional: one pin per stage showing NOT the stage (4018B's outputs) |

At the edge: D1 = the data pin or NOT Q_S; D2 = Q1; D3 = Q2·(Q1 + Q3) (the
anti-lock gate, which brings every illegal state into the cycle); Dk =
Q(k−1) for k ≥ 4. Decoded outputs (stages Q1…QS): output 0 = ¬Q1·¬QS;
output k (1 ≤ k < S) = Qk·¬Q(k+1); output S = Q1·QS; output S+k = ¬Qk·Q(k+1).
Priority: reset > preset enable > clock.

#### Model `seg7`

Decade counters 0–9 with decoded 7-segment outputs: 4026B, 4033B, 40110B.
`units` = 1; main path clock → segment.

| Key | Value |
|---|---|
| `"clock"` + optional `"clockGate"` | as `johnson`; or `"clockUp"` and `"clockDown"` pins (40110B: every rising edge of either counts) |
| `"reset"` | control, asynchronous: count 0 |
| `"segments"`, `"glyphs"` | 7 pins a…g; 10 strings of 7 `0`/`1` |
| `"carry"` | optional pin: 1 for counts 0–4, 0 for 5–9 |
| `"carryPulse"`, `"borrowPulse"` | optional pins (40110B): normally 1; a 0 pulse when the count steps 9 → 0 (carry) or 0 → 9 (borrow), as long as the timing entry's extra keys `carryPulse` / `borrowPulse` (ns) |
| `"displayEnable"` | optional `{"in", "out"}` (4026B): segments forced 0 while `in` is 0; `out` = `in` |
| `"ungatedC"` | optional pin (4026B): segment c of the count, not gated |
| `"lampTest"` | optional control (4033B): every segment 1 |
| `"rippleBlank"` | optional `{"in", "out"}` (4033B): count 0 and `in` = 0 → segments 0; `out` = NOT(count = 0 AND `in` = 0) |
| `"toggleEnable"` | optional control (40110B): counting only while asserted |
| `"latchEnable"` | optional control (40110B): while asserted the segments hold the count they showed; the counter keeps counting |

Segments: lamp test > (display enable 0, or ripple blank) > glyph.

#### Model `divn`

Programmable divide-by-N (4059A, 74HC4059). `units` = 1; main path clock →
output.

| Key | Value |
|---|---|
| `"clock"` | pin (rising edge) |
| `"jam"` | 16 pins, J1…J16 |
| `"modes"` | 3 pins Ka, Kb, Kc |
| `"latchEnable"` | pin |
| `"output"` | pin |

Modes (Ka Kb Kc): 111 ÷2 (first digit d1 = J1; last d5 = J2–J4), 011 ÷4
(J1–J2; J3–J4), 101 ÷5 (J1–J3; J4), 001 ÷8 (J1–J3; J4), 110 ÷10 (J1–J4;
none), X00 master preset; 010 is not a mode (the count goes X). d2 = J5–J8,
d3 = J9–J12, d4 = J13–J16, binary with the lowest pin the LSB (values above 9
are the "extended" range). N = mode × (1000·d5 + 100·d4 + 10·d3 + d2) + d1,
at least 3. At each rising edge: master preset → jammed and held; the first
edge after it → the extra count (C = N); else C ≤ 1 → C = N and the output is
1 until the next edge; else C − 1 and the output 0. Latch enable 1 keeps the
output 1 after a pulse until it returns to 0. Power-on: C = N from the pins.

#### Model `rate`

Rate multipliers (4089B, 4527B, 74x97). `units` = 1; main path clock → out.

| Key | Value |
|---|---|
| `"variant"` | `"binary4"` (count 0–15), `"bcd"` (0–9), `"binary6"` (0–63) |
| `"clock"` | pin (counts on the rising edge) |
| `"rate"` | pins, LSB first (4 or 6) |
| `"enable"` | control: counts while asserted |
| `"strobe"` | control: asserted → no pulses |
| `"cascade"` | control: asserted → `out` forced 1 |
| `"clear"`, `"set"` | controls, asynchronous: count 0 / max; set beats clear; `set` optional |
| `"out"`, `"outBar"` | pins |
| `"terminal"` | optional pin: 1 while the count is at max |
| `"enableOut"` | pin: 0 while enable is asserted and the count is at max |

G = (strobe not asserted) · (clock 0) · (some rate bit set whose states
contain the count); `outBar` = ¬G, `out` = G + cascade. States: binary, a
rate bit j places from the MSB selects the counts whose low j+1 bits are
2^j − 1 (the MSB: even counts); bcd: A {4}, B {2, 7}, C {1, 3, 6, 8},
D {0–3, 5–8}.

### `shift`

Shift registers. A sequential kind (above); `"model"` is `shift` (and the
special models of the sections below). Main timing path: clock → Q (the
storage clock → Q for devices with a storage register; the shift clock →
the serial output for parallel-in serial-out devices).

#### Model `shift`

Device key `"units"`: 1–2 unit objects (`PartLogic.units` = their number).
Unit keys:

| Key | Value |
|---|---|
| `"stages"` | 1–64 |
| `"clock"` | `{"pin", "edge"}` |
| `"clockGate"` | optional `{"pin", "op"}` (as `counter` `sync`; 74x165/166 CLK INH: `or`) |
| `"serial"` | pin names ANDed into stage 1 on a right shift; or `"serialJK"` `{"j", "kbar"}` (stage 1 ← J·¬Q1 + K̄·Q1); or `"serialSelect"` `{"select", "low", "high"}` (stage 1 ← the `low` pin while `select` is 0, the `high` pin while 1). Exactly one of the three. |
| `"leftSerial"` | optional pin: stage N's input on a left shift |
| `"modes"` | optional `{"pins": [S0, S1], "table": [4 of "hold", "right", "left", "load"]}`, row = S0 + 2·S1; without it every active edge shifts right (unless a load says otherwise) |
| `"enables"` | optional controls, all asserted for an edge to act (sampled at the edge) |
| `"load"` | optional `{"pin", "active", "when": "async"|"clock", "from": "data"|"inputRegister"|"io"}`; an asynchronous load is level-transparent |
| `"data"` | parallel input pins in stage order |
| `"clear"` | optional `{"pin", "active", "when"}`: the stages go to 0 |
| `"clearScope"` | `"shift"` (default) or `"all"`: the (asynchronous) clear also clears `storage` and `serialOutputs` |
| `"outputs"` | `[{"stage", "pin", "invert"}]` (stage 1-based, `invert` default false) |
| `"trueComplement"` | optional control (4035B): while not asserted every `outputs` pin shows NOT its stage |
| `"io"` | optional `{"pins": [stage names, inout], "outputEnables": [controls], "notInModes": [mode names]}`: the stages drive the pins while every enable is asserted and the current `modes` row is not listed; a load `from` `io`, a `load` mode row without `data` and the input register read the pins |
| `"serialOutputs"` | optional `[{"pin", "stage"}]`: a flip-flop that takes that stage on the clock's opposite edge (4094B Q'S, 4031B Q', TPIC 596 SER OUT) |
| `"storage"` | optional: exactly one of `"clock": {"pin","edge"}` or `"latch": control` (transparent while asserted), plus `"clear"` (control, asynchronous), `"outputs"` (stages names), `"outputEnable"` (control), `"output"` (`push-pull` / `open-drain`), `"invert"` (bool) |
| `"inputRegister"` | optional `{"clock": {"pin","edge"}, "data": [names]}` (no `data`: the `io` pins) |
| `"clockOut"` | optional pin: follows the clock pin, delayed by the entry's rise/fall (4031B CLD) |

Order on any change, per unit: the input register takes its edge; the
asynchronous clear, then the asynchronous load (clear wins when both are
asserted — the 74x597/598 "invalid" case); on the (gated) clock's edge with
the enables asserted: the clock clear, else the clock load, else the `modes`
row, else a right shift (stage 1 ← the serial input, stage k ← stage k−1;
left: stage N ← `leftSerial`, stage k ← stage k+1). A storage clock edge
takes the stages as they were before a shift at the same instant (tied
clocks leave the storage one clock behind); a storage latch follows the
stages while asserted. The serial-output flip-flops take their stage on the
opposite edge. `part_pin_open_drain` is 1 for the outputs of an `open-drain`
storage.

#### Model `ls673`

74LS673/674 16-bit shift registers. `units` = 1; main path clock → SER/Q15.

| Key | Value |
|---|---|
| `"clock"` | pin; the gated clock CLK·(CS = 0) acts on its falling edge (a CS edge with the clock high is an edge too, as on the chip) |
| `"chipSelect"`, `"readWrite"`, `"mode"` | pins CS (active low), R/W (1 = read), MODE/STRCLK |
| `"serialIo"` | pin SER/Q15 (inout): driven with Q15 while CS = 0 and R/W = 1, else Z |
| `"storageOutputs"` | 16 pins (74LS673): the storage register, always driven |
| `"storageClear"` | control (74LS673): clears the storage at once, not gated by CS |
| `"parallel"` | 16 input pins (74LS674); exactly one of `storageOutputs` / `parallel` |

At a falling gated-clock edge: read with MODE 1 → Q ← storage (673) or the
parallel pins (674); read with MODE 0 → shift, Q0 ← Q15; write → shift,
Q0 ← the SER/Q15 pin. 673: the store clock MODE·(CS = 0)·(R/W = 0) loads the
storage from Q on its rising edge.

#### Model `tap64`

CD4517B dual 64-stage register. Device key `"units"`: 1–2 objects
`{"clock", "data", "writeEnable", "taps"}` (taps: Q16, Q32, Q48 inout, Q64
output). 64 stages on the clock's rising edge, stage 1 ← data. Write enable 0:
the taps drive stages 16/32/48/64. Write enable 1: every tap Z, and stages
17/33/49 take the tap 16/32/48 pins at the edge. `units` = the number of
units; main path clock → tap.

#### Model `bus-register`

CD4034B bidirectional bus register. `units` = 1; main path clock → output
bus. Keys `"a"`, `"b"` (8 inout pins each), `"clock"` (rising), `"aEnable"`
(AE), `"direction"` (A/B), `"parallelSerial"` (P/S), `"asyncSync"` (A/S),
`"serialIn"` (SI). The input bus is A when A/B = 1 (the register itself when
AE = 0: recirculation), B when A/B = 0. P/S = 0: the clock shifts SI in;
P/S = 1, A/S = 0: the clock loads the input bus; P/S = 1, A/S = 1: the
register follows the input bus. Output bus: A/B = 1 → B; A/B = 0 → A while
AE = 1, else Z.

#### Model `fifo`

CD40105B / 74HC40105 4 × 16 FIFO. `units` = 1; main path SO → Q.

| Key | Value |
|---|---|
| `"data"`, `"outputs"` | 4 pins each |
| `"shiftIn"`, `"shiftOut"`, `"inputReady"`, `"outputReady"` | pins SI, SO, DIR, DOR |
| `"reset"` | control MR, asynchronous: every position empty (data kept), DIR 1, DOR 0 |
| `"outputEnable"` | control gating Q |
| `"words"` | 16 |
| `"unloadNeedsEnable"` | bool: true (40105B) — unloading is the falling edge of (SO OR outputs disabled); false (HC) — SO alone |
| `"loadWhileHigh"` | bool: true (HC) — a word is taken when position 1 frees (or MR ends) while SI is high |

Timing entries need the extra key `"fallThrough"` (ns): the ripple time
from position 1 to 16; words move one position every fallThrough / 15. SI
rising with position 1 empty loads it; DIR = NOT (position 1 full); DOR =
position 16 full; Q shows position 16's word (holding the last one while DOR
is low).

#### Model `serial-in`

SN74LV8153 serial-to-parallel interface. `units` = 1; main path RESET → Y.

| Key | Value |
|---|---|
| `"data"` | pin D (idle high) |
| `"address"` | 3 pins A0–A2 |
| `"outputs"` | 8 pins Y0–Y7 |
| `"outputSelect"` | pin OUTSEL: 1 push-pull (Y = bit), 0 open collector inverted (Y = 0 for a 1 bit, else Z) |
| `"reset"` | control: clears the register and the frame decoder |
| `"outputEnable"` | control: Y is Z while not asserted |
| `"serialOut"` | pin SOUT: 0 for one bit period after a byte is taken |

A D falling edge while idle starts a frame (t0); the next rising edge
measures the bit period P (the two start bits); bits 2–8 (A0–A2, then the
nibble) are sampled at t0 + (k + ½)·P. A frame whose address differs from
the A0–A2 pins is ignored. Frames alternate first / second from reset: a
second frame with the first's address completes the byte at its last sample
point; with another address the pair is dropped.
