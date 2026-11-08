# plan-24 device schema (`converter` and `rtc` kinds)

This is the exact form of plan-24-D §3 (converter) and plan-24-F §2 (rtc).
Every `specs/<DEVICE>.json` file is written in it; plan-24-G/H copy the
device objects into `converter.json` / `rtc.json` and add the parts. The
engine (`backend/src/models/converter.c`, `rtc.c`) implements exactly what
is written here; `census/validate.py` checks a spec file against it.

Only the **digital side** is modeled (plan-24-A §1 non-goals). An ADC
conversion's result is the code of a 0 V input on every channel (§ Result).
A DAC stores what it is written; its analog outputs are never driven.

## 1. Spec file

`planning/plan-24-data/specs/<DEVICE>.json`, one per device:

```json
{
  "device": { ...a converter.json / rtc.json device, without "parts"... },
  "packages": {"DGS": {"pinout": "vssop10", "footprint": "VSSOP-10", "package": "VSSOP"}},
  "pdf": "ads1115.pdf",
  "pages": "p.1-4 (pinout), p.15-28 (registers), p.10 (timing)",
  "notModeled": ["comparator hysteresis (analog)", "..."],
  "variants": "ADS1113: no MUX/PGA/comparator, same pinout … (devices sharing this file)"
}
```

- `packages`: TI package designator (the letters in parentheses in the
  datasheet's Package Option Addendum, `VSSOP (DGS) | 10`) → the device
  pinout used by every orderable part in that package, the footprint id
  (`footprints.json` id, or the id plan-24-C will add, `planning/plan-24-data/footprint-ids.tsv`), and the
  part `"package"` family name (`docs/parts-format.md` § Parts: `SOIC`,
  `VSSOP`, `TSSOP`, `VQFN`, `WQFN`, `DSBGA`, …; never a pin count).
- `notModeled`: datasheet features outside the schema (also everything
  analog). Keep it honest: it is the list a reader checks the model against.
- `packages.<code>.pins`: the package's pin count (from the ordering table;
  the pinout must have that many pins). `packages.<code>.i2cAddress`:
  optional fixed 7-bit address for a package without the strap pins;
  `packages.<code>.i2c`: optional `{"address": [...], "table": [...]}`
  replacing the bus's straps and table for that package (a QFN with two
  strap pins where the TSSOP has one).
- `parts`: the orderable parts `[name, code, pins, TI status]` (copied from
  the work skeleton; the generator reads it).
- `pages`, `variants`: free text.
- `partKeys`: optional object part name → extra part keys for that part
  (`{"ADS1100A1IDBVR": {"i2cAddress": 73}}`); the generator adds them.
  Part keys: `i2cAddress`, `resets` (`{"DAC0": 32768}`: register reset
  values that differ for that part, e.g. mid-scale "M" grades), `vcc`,
  `inputMax`.

## 2. Device keys (both kinds)

Common keys as in `docs/parts-format.md` § Devices: `id` (the TI base
product, e.g. `ADS1115`), `function` (`"ADC"`, `"DAC"`, `"RTC"`),
`description`, `drawer` (`["Interface", "ADC"]` / `["Interface", "DAC"]` /
`["Interface", "RTC"]`, with `function` the leaf: use `["Interface"]` and
function `"ADC"`), `note` (datasheet file and pages), `pinouts`.

Pin roles: digital pins as their direction (`input`, `output`, `inout`).
Supplies (AVDD, DVDD, IOVDD, VDD, VCC, VBAT) and reference inputs (REFP,
REFIN, VREF) `power`; grounds (AGND, DGND, GND, VSS, REFN/REFGND when tied
to ground) `ground`. Analog signal pins by direction: inputs (AIN0, CH3,
VIN+, REFOUT-style outputs are `output`) `input`, DAC outputs (VOUTA, IOUT)
and reference/bias outputs `output`, bypass/cap pins (CAP, REFCAP, BYPASS)
`nc`. Every analog pin — analog supplies, references, analog inputs and
outputs — is listed in the device key `"analog"`: the voltage check skips
those pins (plan-24-A §3.4) and the model never drives them. The digital
supply pin(s) the `vcc` range belongs to are the `power` pins **not** in
`"analog"`; a device needs at least one. Thermal pads are not pins (the
footprint's unnumbered centre pad). Crystal pins (X1/X2 on RTCs): `X1`
`input`, `X2` `output`, both in `"analog"`.

| Key | Value |
|---|---|
| `"type"` | converter: `"adc"` or `"dac"` |
| `"bits"` | resolution, 1–32 |
| `"channels"` | ADC input channels / DAC outputs, ≥ 1 |
| `"codeFormat"` | ADC: `"straight"` (unipolar straight binary), `"twos"` (two's complement), `"offset"` (bipolar offset binary); or chosen by a pin or field: `{"pin": "DF", "values": {"0": "offset", "1": "twos"}}` / `{"field": "CFG.FMT", "values": {...}}` (a level not listed, or a multi-level analog strap, uses the first value). DAC: omitted |
| `"vcc"`, `"inputMax"` | the **digital** supply range in volts (DVDD/IOVDD where separate, else VDD) and the highest digital input voltage (recommended operating conditions); parts may override |
| `"analog"` | names of every analog pin (above); may be `[]` |
| `"bus"` | § 3 |
| `"configBus"` | optional second (SPI/I2C) bus, § 3 |
| `"timing"` | optional `{"tValid", "tOut", "tFloat", "tAcc", "tDf"}` ns: SPI CS fall → first bit valid, clock edge → next bit (also I2C SCL fall → SDA), CS rise → Z; parallel RD/CS → data valid, release → float. Missing = 0 |
| `"registers"` | § 4 (may be `[]` for frame-only ADCs) |
| `"protocol"` | § 5 |
| `"conversion"` | ADC: § 6 |
| `"ready"` | optional, § 7 |
| `"pins"` | optional control pins, § 8 |
| `"dac"` | DAC: § 9 |
| `"clock"` | RTC only, § 10 |

## 3. `bus`

The serial kind's object (`docs/parts-format.md` § SPI and I2C host ports):

```json
{"style": "spi", "cs": "CS", "sclk": "SCLK", "mosi": "DIN", "miso": "DOUT", "mode": 1}
{"style": "i2c", "scl": "SCL", "sda": "SDA", "address": ["ADDR"], "table": [72, 73, 75, 74]}
{"style": "i2c-spi", ...}
```

- SPI without MOSI (read-only ADCs): omit `"mosi"`. Without MISO
  (write-only configuration ports): omit `"miso"`. One pin as both
  (3-wire SDIO): name it as `mosi` and `miso`; the part drives it only
  while it shifts reply bits.
- `"csActive": 1` for a frame pin that is active high (TLC5620 LOAD: data
  shift while it is high, the word latches when it falls); default 0.
- **2-wire, no CS** (ADS1230/1232-style DRDY/DOUT + SCLK): omit `"cs"`. A
  transaction begins when a conversion completes (the data-ready pin, the
  MISO pin with `ready.onMiso`, goes active) and ends at the next
  conversion's completion; SCLK edges in between shift `protocol.out` (for
  example `"R{24}"`, with `"idle": "1"` for parts that force the pin high
  after the data). `mode` gives the clock phase as usual.
- MISO pin shared with a ready function (ADS1220 `DOUT/DRDY`): name the
  pin in `bus.miso` and again in `ready` (§ 7) with `"onMiso": true`.
- I2C `"table"` index: Σ tie(Aₖ)·4ᵏ over the `address` pins in array
  order (the first is k = 0), tie digit 0 = GND, 1 = VDD, 2 = SCL, 3 = SDA.
  Datasheet tables usually list GND, VDD, SDA, SCL — remap. `null` for a
  combination the datasheet does not give (a floating strap cannot be
  expressed: `notModeled`). With no strap pins, `"address": []` and
  `"table": [<the 7-bit address>]`.
  Parts with a fixed address that differs per orderable part (ADS1110A0…A7)
  carry the part key `"i2cAddress"` (§ 11). A package without the strap pins
  (ADC081C021 SOT-6: fixed 0x54) sets `"i2cAddress"` in its `packages` entry
  (§ 1); its pinout may then lack the strap pins.
- Parallel (plan-24-E):

```json
{"style": "parallel", "data": ["DB11", "...", "DB0"], "cs": {"pin": "CS", "active": 0},
 "rd": {"pin": "RD", "active": 0}, "wr": {"pin": "WR", "active": 0},
 "address": ["A1", "A0"], "byteSelect": {"pin": "HBE", "high": 1}}
```

  `data` MSB first (4–24 pins); `cs`/`rd`/`wr` each optional (`active`
  0 or 1); a combined R/W pin: `"rw": {"pin": "R/C", "read": 1}` instead of
  `rd`/`wr` (an access is then CS active, R/W giving the direction);
  `address`: register/channel-select pins MSB first; `byteSelect`: the pin
  that picks the high byte (`high` = its level for the high byte).
  A read is CS and RD active (or CS active with R/W = read; with no `rd`
  and no `rw`, CS active alone): the data pins drive the `protocol.read`
  pattern for the current address `tAcc` after it begins and float `tDf`
  after it ends. A write is CS and WR active: at its end (WR or CS
  released) the address bits then the data bits go to `protocol.commands`
  as one transaction (`frame` `"fixed"`). With neither `cs` nor `wr`
  (unlatched DACs, DAC0800), every change of the data pins is a write.
- Clocked (free-running pipelined converters and delta-sigma modulators):

```json
{"style": "clocked", "clock": {"pin": "CLK", "edge": "rise"}, "data": ["D11", "...", "D0"],
 "oe": {"pin": "OE", "active": 0}, "clockOut": {"pin": "DRY", "invert": false}}
```

  Each `clock` edge is a conversion: the data pins (1–24, MSB first) drive
  the next `data`-width bits of `protocol.read` (the pattern repeats: a
  12-bit ADC uses `"R{12}"`; a modulator's 0 V bit stream is `"10"` on one
  pin, 50 % ones) `tOut` after the edge, while `oe` is active (no `oe`:
  always); else Z. `"edge": "both"` for double-data-rate outputs (each edge
  takes the next word). `clockOut`: an output that follows the clock
  (inverted when `invert`), `tOut` later; an array of such objects for
  complementary or several clock outputs (`{"pin": "CLKOUT", "divide": 2}`
  divides the clock). Pipeline latency is not modeled (the code is
  constant).
- `"configBus"` (device key, optional): a second, SPI or I2C bus object
  (same forms as `bus`) for parts with a parallel or clocked data bus and a
  serial configuration port (ADS5xxx parallel + SPI). Its bits run through
  the same `protocol.commands` and `out`.
- Parts with several strap-selected interfaces (parallel / serial / I2C):
  model one — SPI if register read-back works there, else I2C, else
  parallel — and list the others in `notModeled`.

## 4. `registers`

```json
{"name": "CONFIG", "address": 1, "width": 16, "reset": 34179,
 "readOnly": 0, "fields": [{"name": "OS", "bits": [15, 15]}, {"name": "MUX", "bits": [14, 12]}]}
```

| Key | Value |
|---|---|
| `"name"` | unique in the device, `A-Z0-9_` |
| `"address"` | optional (internal registers have none); unique within `space` |
| `"space"` | optional, default `"reg"`; another name (`"dac"`, `"input"`, `"page1"`) gives a separate address space |
| `"width"` | bits, 1–32 (usually 8, 16 or 24) |
| `"reset"` | power-on / reset value, default 0 |
| `"readOnly"` | mask of bits writes do not change (`true` = all) |
| `"fields"` | `{name, bits: [hi, lo]}`, inside the width, not overlapping; names unique in the register |
| `"result"` | `true`: a conversion's completion loads it with the result code (§ 6), right-aligned; or a field name (`"D"`): the code goes into that field (left-justified results), other bits unchanged; `"channel": n` limits that to conversions of channel n |
| `"resultBits"` | with `result`: `[hi, lo]` — only bits hi..lo of the code go in (a 24-bit result in three byte registers DOR2/DOR1/DOR0: `[23, 16]`, `[15, 8]`, `[7, 0]`) |
| `"clearOnRead"` | optional mask: bits cleared after the register is read |
| `"writeOneClears"` | optional mask: writing 1 to these bits clears them, writing 0 leaves them (W1C status flags) |

A field is referred to as `"REG.FIELD"`, a whole register as `"REG"`.
Register and field names may contain `_` (`RST_PWRCTL`); inside `[...]`
the `_` is part of the name.

## 5. `protocol`

How bus bits map onto registers and actions. One engine for SPI, I2C and
parallel: the bus delivers a **bit stream in** (MOSI bits; I2C write bytes
after the address byte, MSB first; a parallel write's address bits then its
data bits) and shifts a **bit stream out** (MISO; I2C read bytes; a
parallel read's data bus).

| Key | Value |
|---|---|
| `"frame"` | `"fixed"`: matching starts at each transaction start (CS fall, I2C START, a parallel write) and stops after one command; `"stream"`: after a command (and its payload) the next bits are matched again, until the transaction ends. In `stream`, a byte (8 bits from the command boundary) that no command can still match is dropped and matching restarts after it; in `fixed`, the rest of the transaction is ignored |
| `"out"` | pattern shifted out from each SPI transaction start (default `"0"`, i.e. zeros) |
| `"idle"` | what is shifted after the queue empties: `"0"`, `"1"`, `"z"`, `"repeat"` (the last pattern again), default `"0"` (SPI), `"repeat"` (I2C reads) |
| `"commands"` | array, below |
| `"read"` | I2C/parallel reads: a pattern, or `"pointer"` (the register at the pointer, MSB first, repeating), or for parallel an object address-value → pattern (`{"0": "[R:11:4]", "1": "[R:3:0]0000"}`; with `byteSelect` the key is `"high"`/`"low"`) |
| `"outs"` | optional array `[{"when": cond, "out": pattern}]`: the first whose condition holds replaces `out` (frames whose layout a register field picks: optional status/CRC bytes, channel-ID headers, word length); `"reads"` the same for `read` |
| `"triggers"` | optional array `[{"write": "REG.FIELD=v", "action": "reset"}]`: after any write that leaves the field at v, run the action (`reset`, `start`, `stop`, `update`, `updateall`, `clear`, `powerdown`, `wakeup`) — software-reset, LDAC and clear bits written through a pointer or register write |
| `"generalCall"` | I2C: `"reset"` when the part resets on the general-call reset (address 0, byte 0x06) |
| `"align"` | `"start"` (default) or `"end"`: in `fixed` frames, match the commands against the **last** bits before the transaction ends (parts that decode the final 16 bits before CS rises); the command then commits at the end |

### Patterns

A pattern is a string of tokens, MSB first; spaces and `_` outside `[...]`
are ignored.

| Token | In (matched against bits in) | Out (shifted) |
|---|---|---|
| `0` `1` | the bit must be this | this bit |
| `x` | any bit | — (not allowed) |
| `z` | — | MISO/SDA released for one bit |
| `a`–`w`, `y` (lowercase) | captured into that letter (letters concatenate MSB first) | the command's capture of that letter |
| `R` | — | one bit of the result code, from its MSB: a run of `n` R's gives the top `n` bits (a run is at most `bits` long; for two results back to back write `[R][R]`) |
| `[NAME]` | the bits go into register/field NAME (applied when the command completes, `readOnly` masked) | NAME's value, its width MSB first |
| `[NAME:hi:lo]` | bits hi..lo of NAME | bits hi..lo of NAME |
| `@` | — | `rreg`: the registers read, each its width, MSB first |
| `t{n}` | token `t` repeated n times (e.g. `x{8}`, `R{12}`, `0{4}`) | same |

`[R]` / `[R:hi:lo]` name the result code (width `bits`). `[CH]` is the
channel of the conversion being output (§ 6).

### Commands

```json
{"name": "RREG", "in": "0010aaaa 0000nnnn", "action": "rreg", "address": "a", "count": "n", "countOffset": 1}
```

| Key | Value |
|---|---|
| `"name"` | the datasheet's name |
| `"in"` | the pattern that selects it. A command fires when the bits since the command boundary equal its length and match; the first match in file order wins; a pattern that is a prefix of a longer one fires first |
| `"action"` | below; default `"nop"` |
| `"address"` | capture letter or number: the register address (`rreg`, `wreg`, `write`, `read`, `pointer`) |
| `"space"` | the address space for `address` (default `"reg"`) |
| `"register"` | a register name instead of `address` |
| `"count"`, `"countOffset"` | registers to move: a capture letter or number, plus the offset (`n` + 1 registers: `countOffset` 1). Default 1 |
| `"data"` | capture letter: the value for `write` |
| `"channel"` | capture letter or number: the channel (ADC: `start`; DAC: `write`/`update`) |
| `"channelMap"` | object captured-value → array of channel indices (`{"7": [0, 1]}` = broadcast; ADC `start`: the channel the code selects, `{"1": [2]}`); values not in it map to themselves |
| `"set"` | object field → capture letter or number, applied when the command completes |
| `"reply"` | pattern shifted out right after the command's `in` bits (replaces the rest of the queue) |
| `"next"` | pattern shifted out from the start of the **next** transaction instead of `out` — the next SPI transaction, or the next I2C/parallel read instead of `read` (pipelined / one-shot read-back) |
| `"commit"` | `"now"` (default: when the pattern completes) or `"end"` (when the transaction ends: CS rise / STOP; a transaction that ends early discards the command) |
| `"when"` | optional condition `"REG.FIELD=v"` / `"!=v"` or an array (all must hold): the command matches only then. With `set` on an internal state register this writes multi-frame sequences (frame 1 `set {"SEQ.STATE": 1}`; frame 2 `when "SEQ.STATE=1"` writes the data and sets the state back), unlock keys (`REG_ACCESS=170`) and mode-dependent decoding (a pin copied into a field with `pins.levels`) |
| `"wordBits"` | `wreg`/`rreg`/`@`: each register occupies this many bus bits (≥ its width), the register in the top bits and zeros below (16-bit registers in 24-bit words) |
| `"then"` | array of further actions run after this one (`["update"]`, `["start"]`) |

Actions:

| Action | Effect |
|---|---|
| `nop` | none (only `set`, `reply`, `next`) |
| `wreg` | the next `count` registers' worth of bits in (each its width) are written to the registers from `address` up (address order) |
| `rreg` | reply (`@` in `reply`, default reply `"@"`) the `count` registers from `address` up |
| `write` | writes capture `data` into the register at `address` / `register`; DACs: into the input register of `channel` |
| `read` | sets the read pointer to `address` / `register` (the next read or `"next"` `[@]`) |
| `pointer` | I2C pointer form: sets the pointer; further bytes of this write go to the register at the pointer (MSB first, its width); the command's `then` actions run after each register written that way. With `"increment"` (a capture letter or number): when it is 1, the pointer steps to the next address after each register written or read (auto-increment) |
| `start` | starts a conversion (on `channel` if given) |
| `stop` | stops continuous conversions |
| `reset` | every register to `reset`, conversion stopped, DAC outputs to their reset |
| `powerdown`, `wakeup` | power-down state: no conversions start; read-back still works |
| `update` | DAC: copies input → DAC register for `channel` (all channels when no channel). As a `then` action after a `write` it updates the written channel(s) |
| `updateall` | DAC: copies input → DAC register for every channel (a "write channel n and update all" command: `then: ["updateall"]`) |
| `clear` | DAC: input and DAC registers to `dac.clearCode` |
| `setbits`, `clearbits` | the register at `address` / `register`: OR in / clear the bits of capture `data` (read-modify-write opcodes) |

## 6. `conversion` (ADC)

```json
"conversion": {
  "time": {"field": "CONFIG.DR", "values": {"0": 125000000, "4": 7812500}},
  "start": ["write:CONFIG.OS=1"],
  "mode": {"field": "CONFIG.MODE", "continuous": 0},
  "busy": {"CONFIG.OS": 0},
  "done": {"CONFIG.OS": 1},
  "channel": "CONFIG.MUX"
}
```

| Key | Value |
|---|---|
| `"time"` | ns, or `{"field", "values": {field value → ns}}`, or `{"fields": [f1, f2], "values": {"v1,v2" → ns}}` (a missing value uses the largest). Use the datasheet's maximum conversion time or 1 / data rate |
| `"start"` | array of triggers: `"write:REG"` (any write to register REG, e.g. a restart-on-write or one-shot register), `"cs-fall"`, `"cs-rise"`, `"command"` (only `start` actions), `"pin"` (§ 8 `convst`/`start`), `"write:REG.FIELD=v"` (a write whose value has v in that field — the value written, before `readOnly` masking, so a read-only status bit can double as a start bit), `"read"` (each completed read of the result starts the next), `"power-on"` |
| `"mode"` | `"single"` (default) or `"continuous"` or `{"field", "continuous": v}` (continuous while the field = v) or `{"field", "single": v}` (continuous while it is not v); continuous restarts at each completion, and entering continuous mode (a write that sets the field, or power-on/reset with it set, or `"continuous"` from power-on) starts the first conversion — unless `"startOnEnter": false` (in the object), when only a `start` trigger begins them |
| `"busy"` | field values set while a conversion runs |
| `"done"` | field values set when it completes (with the `result` registers loaded) |
| `"onRead"` | field values set when the result is read (a `result` register read, or an output pattern's `R`/`[R]` bits shifted out), e.g. `{"STATUS.DRDY": 0}` |
| `"channel"` | the field holding the channel for `[CH]` (optional) |

**Result.** The code of 0 V: `straight` 0; `twos` 0; `offset`
`1 << (bits - 1)`. It is what `R` and `[R]` output and what a `result`
register loads. A conversion still has to run its `time` before the
`result` registers, `done` and `ready` change.

## 7. `ready`

```json
"ready": [{"pin": "DRDY", "active": 0, "asserts": "done", "clears": "read"}]
```

| Key | Value |
|---|---|
| `"pin"` | output pin |
| `"active"` | 0 or 1 |
| `"asserts"` | `"busy"` (active while converting) or `"done"` (from completion) |
| `"clears"` | `"done"` only: `"read"` (the result is read), `"start"` (the next conversion starts, default), `"cs-fall"`, or ns (a pulse), or an array of these (the first that happens) |
| `"openDrain"` | `true` for an open-drain pin (drives active 0 or Z) |
| `"onMiso"` | `true`: the pin is `bus.miso`. While CS is low and no bits are shifting it drives the ready state (ADS1220 `DOUT/DRDY`: low = data ready); while bits shift it is data; while CS is high it floats. With no `cs` (2-wire parts) it shows ready between transfers |
| `"enable"` | optional condition `"REG.FIELD=v"` / `"REG.FIELD!=v"`, or an array of them (all must hold): the pin works only then (else inactive) |

## 8. `pins` (control inputs)

```json
"pins": {"reset": {"pin": "RESET", "active": 0}, "convst": {"pin": "CONVST", "edge": "rise"},
         "start": {"pin": "START", "active": 1}, "powerdown": {"pin": "PWDN", "active": 0},
         "ldac": {"pin": "LDAC", "active": 0, "edge": "fall"}, "clear": {"pin": "CLR", "active": 0}}
```

`"levels"`: an array `[{"pin": "SPEED", "field": "CTRL.SPEED"}]` — the
pin's level (0/1; Z or X reads 0) is copied into the field whenever it
changes (mode/speed/range strap pins; an internal register holds them).

`"resetSelect"`: an array `[{"pin": "RSTSEL", "level": 1, "resets":
{"DACA": 32768}}]` — at power-on and at every reset, registers take these
values instead of their `reset` while the pin is at that level (zero- or
mid-scale strap pins).

`clear` may take `"edge": "rise"` / `"fall"`: one clear on that edge
instead of holding the registers cleared while active.

`reset` held active: registers at reset, bus ignored. `convst`: the edge
starts a conversion. `start`: while active conversions run (continuous;
single mode: the edge starts one). `powerdown` active: no conversions,
the bus ignored. `ldac`: `"edge": "rise"` / `"fall"` → copy input → DAC on
that edge; `"edge": "level"` → copy while the pin is at `active` (writes
made while it is active update at once; LDAC tied active = synchronous
mode). No other spelling (`"level": true` is an error). `clear` active: DAC registers to `clearCode`. A pin at
Z reads inactive.

## 9. `dac`

```json
"dac": {"input": ["INA", "INB"], "output": ["DACA", "DACB"], "clearCode": 0, "ldac": "pin"}
```

`input` / `output`: register names per channel (equal counts; for a DAC
with no double buffering both lists name the same registers). `ldac`:
`"pin"` (§ 8), `"command"` (`update` actions only) or `"sync"` (writes
also update: `then: ["update"]` implied). `clearCode`: what `clear` loads — a
number, or chosen by a pin or field: `{"pin": "CLRSEL", "values": {"0": 0,
"1": 32768}}` / `{"field": "CFG.CLRSEL", "values": {...}}`.

## 10. `clock` (rtc kind, plan-24-F)

```json
"clock": {
  "fields": [{"field": "SEC.SECONDS", "unit": "second", "format": "bcd"},
             {"field": "HOUR.HOURS", "unit": "hour", "format": "bcd", "h12": "HOUR.H12", "pm": "HOUR.PM"},
             {"field": "DAY.DAY", "unit": "weekday", "first": 1},
             {"field": "DATE.DATE", "unit": "date"}, {"field": "MONTH.MONTH", "unit": "month"},
             {"field": "YEAR.YEAR", "unit": "year", "base": 2000}],
  "stop": {"field": "SEC.STOP", "stopped": 1, "powerOn": 0},
  "update": "none",
  "alarms": [{"match": [{"field": "ASEC.SECONDS", "unit": "second", "mask": "ASEC.MASK"}], "flag": "FLAGS.AF", "enable": "CTRL.AIE", "pin": {"pin": "INT", "active": 0, "openDrain": true}}],
  "periodic": {"field": "CTRLA.RS", "periods": {"3": 122070, "15": 500000000}, "flag": "FLAGS.PF", "enable": "CTRLB.PIE", "pin": "INT"},
  "watchdog": {"field": "WD.TIME", "unitNs": 62500000, "flag": "FLAGS.WDF", "pin": "RST"},
  "ram": {"space": "ram", "bytes": 114, "address": 14}
}
```

`unit`: `second`, `minute`, `hour`, `weekday`, `date`, `month`, `year`,
`century`, `hundredth`; `format` `"bcd"` (default) or `"binary"`, or
`{"field": "CTRLB.DM", "binary": 1}`; `h12`/`pm` the 12-hour mode bit and PM
bit (12-hour when `h12` = 1). `stop`: the oscillator stop bit (`stopped` =
its level that halts counting, `powerOn` its value at power on).
`update`: `"none"`; `{"uip": "REGA.UIP", "inhibit": "REGB.SET"}` (UIP
set 244 µs before each update; writing `inhibit` 1 halts updates);
`{"latch": "CTRL.R", "write": "CTRL.W"}` (reading frozen while the read
bit is 1; counting continues; the write bit halts counting and loads the
written registers when cleared). Power-on time 2000-01-01 00:00:00 (a
Saturday), oscillator per `stop.powerOn`.

Further forms (from the RTC datasheets, plan-24-B):

- `h12Level`: the level of `h12` that means 12-hour mode (default 1; `0`
  for "1 = 24-hour" bits).
- `stop.running`: instead of `stopped`, the one field value at which the
  oscillator runs (BQ3285 OS2–OS0 = 010); any other value halts it.
- Alarm `match` entries: `mask` + `maskLevel` (the mask field's level that
  makes the entry always match, default 1); `dontCare`: a bit pattern
  (`"11xxxxxx"`) that, when the alarm byte matches it, makes the entry
  always match.
- `update.latch` with `"writeHalts": false`: the latch bit freezes the
  user copy; counting continues; values written while it is set load into
  the counters when it clears (BQ4802 UTI).
- `periodic` as an array of `{"periodNs", "flag", "enable", "summary"?,
  "pin"}`: several fixed-rate interrupts at once (DP857x), each setting its
  flag (and the summary flag) every period while enabled.
- `watchdog`: `{"field", "periods": {value: ns}, "kick": {"pin", "edge":
  "rise"|"fall"|"both"}, "pin": {"pin", "active"}, "resetPin": {"pin",
  "active", "openDrain", "pulse": {value: ns}}}` — a kick edge restarts the
  timeout; at timeout `pin` asserts (and `resetPin` pulses).
- Parallel `bus.ale`: `{"pin": "AS", "edge": "fall", "bits": 8}` — a
  multiplexed address/data bus: that edge latches the low `bits` of the
  data pins as the address for the following read or write.
- `protocol.reads` entries may give `"read"` as an address map (an object,
  like `protocol.read`): pages/blocks selected by a field (DP8570).

## 11. Rules for transcribing

- **Pin names** ASCII only: `-` for en dashes and minus signs, `u` for µ;
  drop annotations (`DB0 (LSB)` → `DB0`, `VCC (or VREF)` → `VCC`); keep
  slashes and the datasheet's case; where the pin table and drawings differ,
  use the pin table.
- **Times**: maximum values (worst case) for delays and conversion times,
  at the lowest supply the digital `vcc` range allows; fractions are fine.
- **codeFormat**: the code of 0 V in the power-on input range: `offset`
  when that range is bipolar and 0 V reads mid-scale even if the datasheet
  calls the format "straight binary".
- **vcc** with several specified ranges: from the lowest min to the highest
  max of the digital supply.
- Clocked buses (`clocked`) and 2-wire parts need no `conversion` (the clock
  or the data rate paces them; give `conversion.time` = 1 / data rate for
  2-wire parts so the engine can time DRDY).
- Two different chips in one skeleton device (an "A" revision with more
  registers): model the one the newest datasheet describes and report it.

## 12. Part keys

`name`, `package`, `pinout`, `footprint`, `flags` (docs § Parts), optional
`vcc`/`inputMax` (the part's own), optional `"i2cAddress"` (overrides the
bus table: the part's fixed 7-bit address).
