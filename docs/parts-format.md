# Part file format

Every orderable part (gate, flip-flop, SRAM, switch, LED, 7-segment display,
oscillator, supply) is described in a JSON part file in
`schematical/schematical/Parts/`, one file per kind:

| File | Kind |
|---|---|
| `gate.json` | `gate` |
| `dff.json` | `dff` |
| `jkff.json` | `jkff` |
| `sram.json` | `sram` |
| `switch.json` | `switch` |
| `led.json` | `led` |
| `7seg.json` | `7seg` |
| `oscillator.json` | `oscillator` |
| `supply.json` | `supply` |

The backend loads each file with `load_parts()` (`backend/include/parts.h`).
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

A pinout is an array of pins numbered exactly 1..N in order.

```json
{"number": 1, "name": "1A", "role": "input"}
{"number": 9, "name": "A0", "role": "input", "group": "address"}
```

| Key | Value |
|---|---|
| `"number"` | the package pin number; the array must be 1, 2, … N |
| `"name"` | datasheet pin name |
| `"role"` | `input`, `output`, `inout`, `power`, `ground`, `nc` (no connect) or `absent` (a numbered position with no lead; never drawn, nothing can be wired to it) |
| `"group"` | optional, default `"default"`; pins in one group are drawn together (e.g. `"address"`) |

**Kind keys refer to pins by name, never by number**, so one device can carry
several pinouts (DIP and LCCC, DIP and sTSOP). Every name a kind key uses must
exist in every pinout of the device. A name used by several pins means all of
them, in pin-number order; kinds that need exactly one pin reject such names.
(One exception, a part key: an oscillator's `enablePin` is a pin number; see
below.)

## Parts

```json
{"name": "SN74HC00N", "package": "DIP", "pinout": "p14", "family": "HC", "flags": ["obsolete"]}
```

| Key | Value |
|---|---|
| `"name"` | the orderable part number; unique across the whole library (all files) |
| `"package"` | package name as shown, e.g. `"DIP"`, `"TSSOP"`, `"2520 VSON"`; may be empty (supplies) |
| `"pinout"` | the name of one of the device's pinouts |
| `"flags"` | optional, any of `obsolete`, `military`, `automotive`, `enhanced` |

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

## Loading rules

- A key appearing twice in one object is an error.
- Part names are unique across the whole library (across files); device ids
  are unique within a kind.
- A failed `load_parts` adds nothing from that file; parts from earlier files
  stay.

## Kinds

### `supply`

Constant sources. Device key:

| Key | Value |
|---|---|
| `"level"` | `1` (drives its pin to 1, strong) or `0` (drives 0, strong) |

The device has one pinout with one pin. Parts carry no kind keys.

```json
{"id": "VCC", "function": "VCC", "description": "Supply, logic 1", "drawer": [],
 "pinouts": {"pin": [{"number": 1, "name": "VCC", "role": "power"}]},
 "level": 1,
 "parts": [{"name": "VCC", "package": "", "pinout": "pin"}]}
```

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

Part keys:

| Key | Value |
|---|---|
| `"defaultHz"` | the part's frequency, from `minHz` to `maxHz` |
| `"enablePin"` | optional; the **pin number** (in the part's pinout) of the output-enable / standby pin. A LOW there puts every output in high impedance; HIGH or not connected keeps the clock running. A number, not a name, because the LMK6D/H/P function pins 1 and 2 share the name `OE/ST/NC`. |

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
| `"timing"` | object: logic family → `{rise, fall}` (ns), or `{otherLow: {rise, fall}, otherHigh: {rise, fall}}` |

`otherLow`/`otherHigh` (74x86): the delays depend on the input that did not
change — 0 picks `otherLow`, 1 picks `otherHigh`, and X (also at start, when
no edge has happened yet) takes the longer of each delay. Allowed only when
every gate has exactly 2 inputs.

Part key:

| Key | Value |
|---|---|
| `"family"` | the part's logic family; must be a key of `timing` |

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
| `"timing"` | logic family → `{rise, fall, clearWins}`; `clearWins` true: PRE and CLR both asserted give Q = 0, QN = 1; false: Q = QN = 1 |

Part key: `"family"`, a key of `timing`. `PartLogic.units` is the number of
units.

### `jkff`

JK flip-flops. On its clock edge (PRE and CLR inactive) a unit sets (J),
resets (K), holds or toggles (both). An unknown J or K gives X unless every
level it could have leads to the same Q. PRE/CLR as for `dff`; both
asserted give Q = QN = 1.

Device keys: as `dff`, except `"units"` (1–2) take `j`, `k`, `clk`
(required) and `pre`, `clr`, `q`, `qn` (optional); there is no
`outputEnable`; `"kInverted"` (bool) means the K input is K-bar (74x109);
timing entries are `{rise, fall}`. Part key: `"family"`.

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
| `"grades"` | speed grade → timing in ns: `trc taa tace toe tba toh tclz tolz tblz tchz tohz tbhz twc taw tcw tbw tas twp twpOeLow twr tdw tdh twhz tow`; a missing name is 0 |

Part key: `"grade"`, a key of `grades`. `PartSram`: `words` = 2^len(address),
`bits` = len(data), `access_ps` = the grade's `taa`, `grade`.

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
| `"segmentSide"`, `"commonSide"` | drawing order of the symbol's two sides, by name; a shared name expands to its pins in number order (`PartDisplay.segment_pins`/`common_pins`) |

An LED's segment pin is the first pin (in pin order) in `segmentPins` that
drives its segment for its digit (or every digit); its commons are the
first two pins of its digit in `commonPins`. Unused positions are pins with
role `nc` (no connect) or `absent` (no lead). Parts carry no kind keys.
