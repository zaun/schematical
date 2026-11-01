# Part file format

Every orderable part (gate, flip-flop, SRAM, EEPROM, switch, LED, 7-segment
display, oscillator, supply) is described in a JSON part file in
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
can carry several pinouts (DIP and LCCC, DIP and sTSOP, SOT-23 and DSBGA).
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
| `"package"` | package name as shown, e.g. `"DIP"`, `"TSSOP"`, `"2520 VSON"`; may be empty (supplies) |
| `"pinout"` | the name of one of the device's pinouts |
| `"flags"` | optional, any of `obsolete`, `military`, `automotive`, `enhanced` |
| `"footprint"` | the `id` of a footprint in `footprints.json` (see [Footprints](#footprints)). Required on every part except supplies (`VCC`, `GND`, `5V`, …), which reject it |
| `"vcc"`, `"inputMax"` | on gate, dff, jkff, sram, eeprom and oscillator parts only: the part's own supply range / input limit, replacing the one it inherits (see [Supply voltage](#supply-voltage-vcc-and-inputmax)) |

Every other key belongs to the kind.

Flags:

- `obsolete`: the datasheet's ordering table lists the part as Obsolete.
- `military`: SN54/SNJ54/SNV54/CD54 names, 5962- and (J)M38510/ numbers, SMD
  drawing numbers, CD4000 …F3A parts.
- `automotive`: AEC-Q100 qualified (-Q1) parts.
- `enhanced`: Enhanced Product (-EP) parts, V62/ numbers.

## Values

- **Times** are nanoseconds (JSON numbers, fractions allowed). The loader
  converts them to the engine's picoseconds with `llround(ns * 1000)`.
- **Logic families** (`family` on gate, dff and jkff parts) must be one of:
  HC, LS, ACT, HCT, F, AC, AHCT, ALS, AS, AHC, S, C, H, HCS, LV, LVC, AUP,
  AUC, ALVC, TTL.
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
| `"pad"` | `{"width", "height", "shape"}`, sizes > 0, shape `"rect"` or `"round"` (an ellipse; a circle when width = height). Not on `explicit` |
| `"drill"` | optional, default 0 (surface mount); a through-hole diameter, smaller than both pad sides. Not on `explicit` |
| `"pin1Shape"` | optional, `dual`/`quad` only: pin 1's pad shape (through-hole parts use `"rect"` to mark pin 1) |
| `"centre"` | optional, `dual`/`quad` only: one pad at the origin, `{"width", "height", "shape"}` — an exposed pad, or the middle terminal of an X2SON-5 |
| `"slots"` | optional, `dual`/`quad` only: `slots[i]` = the slot (1-based) pin i+1 sits on; `0` = the pin is the `centre` pad. Default: pin n on slot n |

`slots` has exactly `pins` entries; non-zero values are distinct and no
larger than the slot count; at most one `0`, and only with `centre`. A slot
no pin names gets no pad. A `centre` no pin names is an unnumbered pad
(designator `NULL`), listed last.

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
"shape"}` plus optional `"drill"`, in the coordinates above. Every pin
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
