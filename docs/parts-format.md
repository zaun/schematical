# Part file format

Every orderable part (gate, flip-flop, latch, counter, shift register, SRAM, EEPROM, switch, LED,
7-segment display, oscillator, supply, microprocessor, serial controller) is described in a JSON part file in
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
| `counter.json` | `counter` |
| `shift.json` | `shift` |
| `cpu.json` | `cpu` |
| `serial.json` | `serial` |

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
- **Logic families** (`family` on gate, dff and jkff parts) must be one of:
  HC, LS, ACT, HCT, F, AC, AHCT, ALS, AS, AHC, S, C, H, HCS, LV, LVC, AUP,
  AUC, ALVC, TTL, ABT, BCT, LVT, AVC, TPIC. The last five:
  - `ABT`, `BCT`: BiCMOS bus-interface logic (5 V).
  - `LVT`: 3.3 V ABT; LVTH (bus-hold) parts use it too.
  - `AVC`: advanced very-low-voltage CMOS.
  - `TPIC`: TI power-logic parts with DMOS open-drain outputs.
- **`note`** is allowed on a device and on any timing entry; it is ignored.
- **Voltages** are volts (JSON numbers, fractions allowed). The loader
  converts them to millivolts (`uint32_t`) with `llround(v * 1000)`.

### Supply voltage (`vcc` and `inputMax`)

Chips carry their datasheet's Recommended Operating Conditions supply range,
for the voltage check (`docs/backend.md`). The keys go on the gate, dff and
jkff **timing entries**, the sram and eeprom **grades**, the oscillator
**device**, and on any **part** of those kinds:

| Key | Value |
|---|---|
| `"vcc"` | `[min, max]` volts: the recommended supply range, 0 < min < max ≤ 30 |
| `"inputMax"` | optional: the highest voltage an input (or inout) takes when it is above `vcc` max, e.g. `5.5` for 5 V-tolerant LVC inputs; at least the resolved `vcc` max, at most 30. Without it an input takes at most the `vcc` max. |

Resolution, per part:

1. The base is the part's timing entry (gate, dff, jkff), grade (sram,
   eeprom) or device (oscillator): its `vcc` and `inputMax`, each optional.
2. The part's own `vcc` replaces the base range; its own `inputMax` replaces
   the base input max. A part that overrides only `vcc` keeps the base's
   `inputMax` if one was given, otherwise its input max is its own `vcc`
   max.
3. `inputMax` with no `vcc` at that level or below it is an error
   (`"inputMax" without "vcc"`), and so is an `inputMax` below the resolved
   `vcc` max.
4. `Part.vcc` points at the resolved `PartVoltage` (`min_mv`, `max_mv`,
   `input_max_mv`); parts with no part-level key share their entry's.

Every gate, dff, jkff, sram, eeprom and oscillator part must resolve to a
`vcc` range (a part with none is rejected: `missing "vcc"`). Supplies,
LEDs, 7-segment displays and switches never take these keys; their
`Part.vcc` is NULL.

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

### `explicit` — literal pads

For land patterns the parametric styles cannot express (mixed pad sizes).
`"pads"` is a non-empty array of `{"pin", "x", "y", "width", "height",
"shape"}` plus optional `"drill"` and corner keys, in the coordinates above. Every pin
1…`pins` appears exactly once; `"pin": 0` is an unnumbered pad (any
number of them). `pad`, `drill`, `slots`, `centre` and `pin1Shape` are not
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
  uses.
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
| `"color"` | shown color, e.g. `"Red"` |
| `"anode"` | pin name of the anode (exactly one pin) |
| `"cathode"` | pin name of the cathode (exactly one pin) |

Parts carry no kind keys.

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
| `"gates"` | array of gates: `op` (`and`, `nand`, `or`, `nor`, `xor` = odd parity, `xnor` = even parity), `inputs` (1–13 pin names), `output` (a pin name). Each name must be exactly one pin in every pinout. |
| `"output"` | optional: `push-pull` (default) or `open-drain` (a 1 releases the pin to Z after the rise delay; an external pull-up takes it high; 74x266) |
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

`cpu.json` holds three devices: `W65C02S` (2 parts), `MCS6502` (the 4
plastic MPS6502 parts, obsolete) and `Z84C00` (13 parts, obsolete). The
example below is the `W65C02S` device's keys and grade.

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
| `"core"` | `"6502"`, `"65c02"` or `"z80"` (Z80 keys below) |
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
| `"model"` | `"ns16550"` (the 16450/16550 family); more models in later plans |
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
| `"bus"` | `"style": "intel"`; `"data"` 8 names; `"address"` 3 names (A0 first); `"read"` (active low) and/or `"readHigh"`; `"write"` and/or `"writeHigh"`; `"select"` (active-low names) and `"selectHigh"` (active-high); optional `"addressStrobe"` (ADS: transparent while low, latched on its rising edge), `"driverDisable"` (DDIS, low while the chip drives the bus), `"selectOut"` (CSOUT), `"reset"` with `"resetActiveHigh"` (default true) |
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
