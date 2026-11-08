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
| `"codeFormat"` | ADC: `"straight"` (unipolar straight binary), `"twos"` (two's complement), `"offset"` (bipolar offset binary). DAC: omitted |
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

- SPI without MOSI (read-only ADCs): omit `"mosi"`. Without CS (rare): not
  allowed; record it in `notModeled` and use the frame-sync pin as `cs`
  only when the datasheet's timing is CS-like (active low frame).
- MISO pin shared with a ready function (ADS1220 `DOUT/DRDY`): name the
  pin in `bus.miso` and again in `ready` (§ 7) with `"onMiso": true`.
- I2C `"table"` index: Σ tie(Aₖ)·4ᵏ, tie 0 = GND, 1 = VDD, 2 = SCL, 3 = SDA.
  With no strap pins, `"address": []` and `"table": [<the 7-bit address>]`.
  Parts with a fixed address that differs per orderable part (ADS1110A0…A7)
  are separate devices or carry the part key `"i2cAddress"` (§ 11).
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
  as one transaction (`frame` `"fixed"`).
- Clocked (free-running pipelined converters and delta-sigma modulators):

```json
{"style": "clocked", "clock": {"pin": "CLK", "edge": "rise"}, "data": ["D11", "...", "D0"],
 "oe": {"pin": "OE", "active": 0}, "clockOut": {"pin": "DRY", "invert": false}}
```

  Each `clock` edge is a conversion: the data pins (1–24, MSB first) drive
  the next `data`-width bits of `protocol.read` (the pattern repeats: a
  12-bit ADC uses `"R{12}"`; a modulator's 0 V bit stream is `"10"` on one
  pin, 50 % ones) `tOut` after the edge, while `oe` is active (no `oe`:
  always); else Z. `clockOut`: an output that follows the clock (inverted
  when `invert`), `tOut` later. Pipeline latency is not modeled (the code
  is constant).
- `"configBus"` (device key, optional): a second, SPI or I2C bus object
  (same forms as `bus`) for parts with a parallel or clocked data bus and a
  serial configuration port (ADS5xxx parallel + SPI). Its bits run through
  the same `protocol.commands` and `out`.

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
| `"result"` | `true`: a conversion's completion loads it with the result code (§ 6); `"channel": n` limits that to conversions of channel n |
| `"clearOnRead"` | optional mask: bits cleared after the register is read |

A field is referred to as `"REG.FIELD"`, a whole register as `"REG"`.

## 5. `protocol`

How bus bits map onto registers and actions. One engine for SPI, I2C and
parallel: the bus delivers a **bit stream in** (MOSI bits; I2C write bytes
after the address byte, MSB first; a parallel write's address bits then its
data bits) and shifts a **bit stream out** (MISO; I2C read bytes; a
parallel read's data bus).

| Key | Value |
|---|---|
| `"frame"` | `"fixed"`: matching starts at each transaction start (CS fall, I2C START, a parallel write) and stops after one command; `"stream"`: after a command (and its payload) the next bits are matched again, until the transaction ends |
| `"out"` | pattern shifted out from each SPI transaction start (default `"0"`, i.e. zeros) |
| `"idle"` | what is shifted after the queue empties: `"0"`, `"1"`, `"z"`, `"repeat"` (the last pattern again), default `"0"` (SPI), `"repeat"` (I2C reads) |
| `"commands"` | array, below |
| `"read"` | I2C/parallel reads: a pattern, or `"pointer"` (the register at the pointer, MSB first, repeating), or for parallel an object address-value → pattern (`{"0": "[R:11:4]", "1": "[R:3:0]0000"}`; with `byteSelect` the key is `"high"`/`"low"`) |
| `"generalCall"` | I2C: `"reset"` when the part resets on the general-call reset (address 0, byte 0x06) |

### Patterns

A pattern is a string of tokens, MSB first; spaces and `_` are ignored.

| Token | In (matched against bits in) | Out (shifted) |
|---|---|---|
| `0` `1` | the bit must be this | this bit |
| `x` | any bit | — (not allowed) |
| `z` | — | MISO/SDA released for one bit |
| `a`–`w`, `y` (lowercase) | captured into that letter (letters concatenate MSB first) | the command's capture of that letter |
| `R` | — | one bit of the result code, from its MSB: `n` R's give the top `n` bits |
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
| `"channelMap"` | object captured-value → array of channel indices (`{"7": [0, 1]}` = broadcast); values not in it map to themselves |
| `"set"` | object field → capture letter or number, applied when the command completes |
| `"reply"` | pattern shifted out right after the command's `in` bits (replaces the rest of the queue) |
| `"next"` | pattern shifted out from the start of the **next** transaction instead of `out` (pipelined read-back) |
| `"commit"` | `"now"` (default: when the pattern completes) or `"end"` (when the transaction ends: CS rise / STOP; a transaction that ends early discards the command) |
| `"then"` | array of further actions run after this one (`["update"]`, `["start"]`) |

Actions:

| Action | Effect |
|---|---|
| `nop` | none (only `set`, `reply`, `next`) |
| `wreg` | the next `count` registers' worth of bits in (each its width) are written to the registers from `address` up (address order) |
| `rreg` | reply (`@` in `reply`, default reply `"@"`) the `count` registers from `address` up |
| `write` | writes capture `data` into the register at `address` / `register`; DACs: into the input register of `channel` |
| `read` | sets the read pointer to `address` / `register` (the next read or `"next"` `[@]`) |
| `pointer` | I2C pointer form: sets the pointer; further bytes of this write go to the register at the pointer (MSB first, its width) |
| `start` | starts a conversion (on `channel` if given) |
| `stop` | stops continuous conversions |
| `reset` | every register to `reset`, conversion stopped, DAC outputs to their reset |
| `powerdown`, `wakeup` | power-down state: no conversions start; read-back still works |
| `update` | DAC: copies input → DAC register for `channel` (all channels when no channel) |
| `clear` | DAC: input and DAC registers to `dac.clearCode` |

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
| `"time"` | ns, or `{"field", "values": {field value → ns}}` (a missing value uses the largest) |
| `"start"` | array of triggers: `"cs-fall"`, `"cs-rise"`, `"command"` (only `start` actions), `"pin"` (§ 8 `convst`/`start`), `"write:REG.FIELD=v"` (a write whose value has v in that field — the value written, before `readOnly` masking, so a read-only status bit can double as a start bit), `"read"` (each completed read of the result starts the next), `"power-on"` |
| `"mode"` | `"single"` (default) or `"continuous"` or `{"field", "continuous": v}` (continuous while the field = v); continuous restarts at each completion, and entering continuous mode (a write that sets the field to v, or power-on/reset with it at v, or `"continuous"` from power-on) starts the first conversion |
| `"busy"` | field values set while a conversion runs |
| `"done"` | field values set when it completes (with the `result` registers loaded) |
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
| `"clears"` | `"done"` only: `"read"` (the result is read), `"start"` (the next conversion starts, default), `"cs-fall"`, or ns (a pulse) |
| `"openDrain"` | `true` for an open-drain pin (drives active 0 or Z) |
| `"onMiso"` | `true`: the pin is `bus.miso`; it shows ready while CS is high and data while CS is low |
| `"enable"` | optional condition `"REG.FIELD=v"` / `"REG.FIELD!=v"`, or an array of them (all must hold): the pin works only then (else inactive) |

## 8. `pins` (control inputs)

```json
"pins": {"reset": {"pin": "RESET", "active": 0}, "convst": {"pin": "CONVST", "edge": "rise"},
         "start": {"pin": "START", "active": 1}, "powerdown": {"pin": "PWDN", "active": 0},
         "ldac": {"pin": "LDAC", "active": 0, "edge": "fall"}, "clear": {"pin": "CLR", "active": 0}}
```

`reset` held active: registers at reset, bus ignored. `convst`: the edge
starts a conversion. `start`: while active conversions run (continuous;
single mode: the edge starts one). `powerdown` active: no conversions,
the bus ignored. `ldac`: `edge` → copy input → DAC on that edge; `"level"`
→ copy while active. `clear` active: DAC registers to `clearCode`. A pin at
Z reads inactive.

## 9. `dac`

```json
"dac": {"input": ["INA", "INB"], "output": ["DACA", "DACB"], "clearCode": 0, "ldac": "pin"}
```

`input` / `output`: register names per channel (equal counts; for a DAC
with no double buffering both lists name the same registers). `ldac`:
`"pin"` (§ 8), `"command"` (`update` actions only) or `"sync"` (writes
also update: `then: ["update"]` implied). `clearCode`: what `clear` loads.

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

## 11. Part keys

`name`, `package`, `pinout`, `footprint`, `flags` (docs § Parts), optional
`vcc`/`inputMax` (the part's own), optional `"i2cAddress"` (overrides the
bus table: the part's fixed 7-bit address).
