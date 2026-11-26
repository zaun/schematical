# plan-72-B: IO expanders and I2C bus parts — pinouts, register templates, address tables, the footprint list

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-A

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions,
which apply to every letter). B reads every `add` row's datasheet and fills
`planning/plan-72-data/specs.tsv`: the pinout of each (PDF, package), the
**template** each device is generated from (§ 3), its bus pins and address
table, and the footprint each package needs. It writes the final
`footprints-needed.txt` that sizes C.

References:

- plan-72-A § 4.5 (`specs.tsv` columns), its `candidates.tsv`, the PDFs in
  `datasheets/ioexpander/` and `datasheets/i2cbus/`.
- `docs/parts-format.md` § Pinouts and pins (roles, `group`, `activeLow`),
  § `converter` (`bus`, the I2C `address` / `table` rule), § Footprints.
- `planning/completed/plan-48-B-timer-opamp-specs.md` (the same letter for
  timers and op-amps: `pinout-check.py`, `footprint-lines.py`).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-A complete | `grep -c '^- \[ \]' planning/plan-72-A-ioexp-data.md` → 0 | NOT MET (2026-11-25: A not started) |
| `specs.tsv` skeleton present | `python3 -I planning/plan-72-data/census/specs-skeleton.py --verify` → `0 problems` | NOT MET (written by A Phase 3) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Every `specs.tsv` row has `template`, `io` (the number of port pins), `bus`
  (`i2c`, `spi`, `i2c-spi` or `none`), `address_pins`, `footprint` and a
  `pinout` that `census/pinout-check.py` accepts, or the row's parts are
  `skip` in `candidates.tsv` with reason `no pinout for <package>`.
- `planning/plan-72-data/templates/<template>.json`: one device skeleton per
  template of § 3 (registers, protocol, `io` section), the input of
  `gen-parts.py` in F, G and H.
- `planning/plan-72-data/footprints-needed.txt`: every package the `add` rows
  use, each line `<footprint id> <exists|new> <pdf page of the drawing>`.

### Non-goals

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new device behaviour beyond what § 3's templates name; a register a
  template stores but does not act on is listed in the template's `note`.

## 2. Current State

`specs.tsv` exists with its first five columns (A Phase 3). The upper bound is
459 parts (plan-72-A § 2); the distinct (PDF, package) count is A's output.

Pin roles for these parts (rules of `docs/parts-format.md` § Pinouts and
pins): port pins `inout` (`input` on an input-only port such as MAX7319's,
`output` on an output-only port such as MAX7320's); SDA `inout`, SCL `input`
(the I2C target never drives SCL: `serial_target.h` "No clock stretching");
SPI CS/SCLK/MOSI `input`, MISO `output`; INT / INTA / INTB / RESET# as
`output` / `input` with `activeLow`; A0–A2 `input` with group `address`;
VCC / VDD / VCCA / VCCB / VDDIO `power`; GND / VSS `ground`; a mux's SDn/SCn
`inout`. Exposed pads are not pins (the footprint's unnumbered centre pad).

## 3. Design: the templates

A template is a device skeleton in JSON (`templates/<name>.json`) with
`{IO}`, `{PORTS}` and `{ADDRESS_TABLE}` slots that `gen-parts.py` fills per
device. Every device gets exactly one template; B assigns it from the
datasheet's register map, and writes a new template file for a device whose
map none of these matches (its own skeleton, from its datasheet, using only
the engine keys of plan-72-D § 4 and plan-72-E § 4).

#### `ioexpander` templates

| Template | Devices (by datasheet) | Registers and behaviour | Engine keys used |
|---|---|---|---|
| `quasi` | PCF8574, PCF8574A, PCF8575, PCA8574/A, MAX7328, MAX7329, MAX7321 | no registers addressed: each written byte (two for 16 ports) goes to the output latch; a read returns the pins' levels. A latch bit 1 is a weak pull-up input (quasi-bidirectional), 0 drives low. INT active low, open drain, on any pin change since the last read or write of the port; cleared by a read or a write, or when the pins return to the captured state | ports `dirField` = the latch bit, `dirOutput` 0, `pullupValues` 1; `io.interrupts` `compare: "snapshot"`, `clearOn: ["read", "write"]` |
| `pca9554` | TCA9534, TCA9554, TCA6408A, PCA9534, PCA9554, PCA9538, PCA9557, PCA6408A, MAX7310 | Input, Output, Polarity Inversion, Configuration at 0–3 (MAX7310: plus the timeout register 4, stored); command byte = pointer, no auto-increment; Input reads pins XOR polarity; INT on change of an input pin vs the last Input read, cleared by reading Input | ports + `readInvertField`; `io.interrupts` `compare: "snapshot"`, `clearOn: ["Input"]` |
| `pca9555` | TCA9555, TCA9535, TCA9539, TCA6416A, PCA9555, PCA9535, PCA9539, PCA6416A, MAX7311, MAX7312 | the 8 registers 0–7 in pairs; the pointer toggles within its pair after each byte | register `next` (0↔1, 2↔3, 4↔5, 6↔7); the rest as `pca9554` |
| `pca9555-led` | MAX7313, MAX7314, MAX7315, MAX7318 | `pca9555` plus blink-phase and intensity registers, stored; outputs are the static port value (blink and PWM are not simulated, as plan-56 left LED intensity) | as `pca9555` |
| `ai24` | TCA6424A, PCA9505, PCA9506, PCA9698 — every device whose command byte's bit 7 is the auto-increment flag and whose banks are 3 or 5 ports wide | pointer = command bits 6–0; bit 7 = AI: increment through the bank and wrap to its first register; mask registers stored where present | `pointer` with `increment` from capture of bit 7; register `next` for the wrap |
| `pcal` | PCAL6408A, PCAL6416A, PCAL9554B, PCAL9555A, PCAL6524, PCAL6534, TCAL6408, TCAL6416 | the base map (`pca9554` / `pca9555` / `ai24`) plus the agile registers at 0x40–0x4F: output drive strength (stored), input latch (stored, not simulated), pull enable, pull select, interrupt mask, interrupt status (read-only, set by the change logic), output port configuration (push-pull / open drain per bank) | `pullEnableField`, `pullSelectField`, `openDrainField`; `io.interrupts` `maskField`, `statusField` |
| `mcp23x08` | MCP23008, MCP23S08, MCP23009, MCP23S09 | IODIR, IPOL, GPINTEN, DEFVAL, INTCON, IOCON, GPPU, INTF, INTCAP, GPIO, OLAT at 0x00–0x0A; IOCON.SEQOP = 0 auto-increments the pointer (wrap 0x0A → 0x00); INT compares to DEFVAL (INTCON bit 1) or the previous value (bit 0), sets INTF, captures INTCAP, clears on a read of GPIO or INTCAP; IOCON.ODR, IOCON.INTPOL pick the INT drive; MCP23009/S09 outputs open drain; S08: SPI opcode `0100 0 A1 A0 R/W` with IOCON.HAEN | `pointer` twice with `when` IOCON.SEQOP; register `next`; ports `pullEnableField` = GPPU; `io.interrupts` `compare: {"field", "selectField"}`, `flagField`, `captureField`, `clearOn: ["GPIO", "INTCAP"]`, `pin.activeField`, `pin.openDrainField`; SPI opcode commands enumerated per HAEN and address-pin state (`pins.levels`) |
| `mcp23x17` | MCP23017, MCP23S17, MCP23018, MCP23S18 | the IOCON.BANK = 0 map (22 registers, A/B interleaved); MIRROR joins INTA and INTB; the rest as `mcp23x08` per bank | as `mcp23x08`, two `io.interrupts` sources and `io.interruptPins` with `mirrorWhen` |
| `max7300` | MAX7300, MAX7301 | 28 ports (P4–P31), 2-bit port configuration (output / input / input with pull-up), single-port registers 0x20–0x3F and 8-port registers 0x44–0x5F that alias them, the transition-detect mask and INT on P31 | register `alias`; ports `dirField` 2-bit, `pullupValues`; `io.interrupts` `maskField` |
| `maxsimple` | MAX7319, MAX7320, MAX7322, MAX7323, MAX7324–MAX7327 | byte-wide reads and writes with no pointer; per device: input ports with transition flags (MAX7319, MAX7322), push-pull or open-drain output ports, power-up states | ports; `io.interrupts` `compare: "snapshot"`, `flagField` |
| `keypad-gpio` | ADP5585, ADP5587, ADP5589 (the ADI 505-label parts) | the GPIO registers (direction, data, pull, INT) are modeled; the key-scan matrix, FIFO and PWM are stored and not simulated (the plan-56 precedent for key scanning) | `pca9554`-style keys |

#### `i2cbus` templates

| Template | Devices | Behaviour | Engine keys used |
|---|---|---|---|
| `switch` | TCA9548A, TCA9546A, TCA9545A, TCA9543A, PCA9846, PCA9848, PCA9849, PI4MSD5V9543A/45A/46A/48A | one control register; bit n closes channel n (SDA–SDn and SCL–SCn); RESET# low opens every channel and clears the register; TCA9545A / TCA9543A: INTn inputs read in bits 7–4, INT output open drain, low while any INTn input is low | `io.links` `closedWhen: "CTRL.Bn=1"`; `pins.levels` for INTn; ports for INT |
| `mux` | TCA9544A, TCA9540B, PCA9847, PI4MSD5V9540B | bit 2 enables, bits 1–0 (bit 0) select one channel | `io.links` `closedWhen: ["CTRL.EN=1", "CTRL.SEL=n"]` |
| `buffer` | PCA9515A/B, TCA9515, PCA9517(A), TCA9517(A), PCA9617A, PCA9527, TCA9509, TCA4307, TCA9803, PCA9510A–PCA9514A, LTC4300A, LTC4301, LTC4303, LTC4304, LTC4307, LTC4308, LTC4313, LTC4314, LTC4315 | the two sides joined while EN is high (EN tied high when the device has none); READY (where present) high while joined; two supply domains where the datasheet has VCCA/VCCB | `io.links` `closedWhen` on `pins.levels` EN; `domains` (`docs/parts-format.md` § Two supply domains) |
| `split` | P82B96 (NXP, TI), PCA9600, PCA9601 | each Sx/Sy bidirectional pin splits into Tx (output, open drain, follows Sx when another driver pulls Sx low) and Rx (input: Rx low pulls Sx low) | `io.splits` |
| `extender` | LTC4302-1/-2, LTC4305, LTC4306, PCA9641 | an I2C target with control registers whose fields close links (LTC4302: connect bit; LTC4305/06: one bit per downstream bus; PCA9641: the grant to master 0 or 1); GPIO pins as ports; fault timers and stuck-bus recovery stored, not simulated | `io.links`, ports |
| `passive` | LTC4311, LTC1694-1 (rise-time accelerators with no inline path) | drives nothing; a placed part does not turn Run off | an `i2cbus` device with an empty `io` (plan-72-E § 4.4) |
| `facts` | LTC4316, LTC4317 (address translators), LTC4310-1/-5 (isolated hot-swap pair) | `"simulated": false`: a translator rewrites address bits and an isolated pair crosses two chips, which a contact cannot carry | none |

The device names in these tables are the datasheets the author knows of; B
assigns templates to whatever A's `add` rows are, writing a new template for
a device that matches none.

Rejected: one C model per register family (the converter engine already
executes declarative registers, the plan-56 decision); BANK = 1 layout for
MCP23x17 (a write that sets IOCON.BANK is stored and the map stays BANK = 0's,
documented in the template's `note`: the BANK = 1 remap changes every
register address, which the engine's fixed register addresses cannot follow,
and hosts in practice keep BANK = 0).

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — pinouts

- [ ] `census/pinout-check.py` (from plan-48's): parses the `pinout` column
      (`number:name:role[:group][:low]`, `;`-separated), checks the roles of
      § 2, one pin per number, the package's pin count, and names every
      `bus` / `address_pins` pin; `--selftest`.
- [ ] Fill `pinout`, `io`, `bus`, `address_pins` for every row from the PDF's
      pin table; a package without a pin table in its PDF → its parts `skip`.

Acceptance: `python3 -I planning/plan-72-data/census/pinout-check.py` → `<rows> rows, 0 errors` (est. 1 min per run; the reading is the work).
Commit:

### Phase 2 — templates and address tables

- [ ] `templates/*.json` for the templates of § 3 that `specs.tsv` uses, plus any new one; each with a `note` listing what is stored and not simulated.
- [ ] `template` per row; the I2C address `table` per device (4^straps entries, `null` where the datasheet gives none) in `planning/plan-72-data/address.tsv` (`device straps table`).
- [ ] `census/template-check.py`: every row has a template that exists; every template's slots are filled by its rows; every address table has 4^n entries; `--selftest`.

Acceptance: `python3 -I planning/plan-72-data/census/template-check.py` → `<rows> rows, <t> templates, 0 errors` (est. 1 min).
Commit:

### Phase 3 — the footprint list

- [ ] `footprint` per row: an existing `footprints.json` id whose drawing matches the datasheet's package drawing, or a new id (`docs/parts-format.md` § Footprints naming).
- [ ] `census/footprint-lines.py` → `planning/plan-72-data/footprints-needed.txt` (`<id> exists|new <pdf>:<page>`).

Acceptance: `grep -c ' new ' planning/plan-72-data/footprints-needed.txt` prints the count C works from, and `python3 -I planning/plan-72-data/census/footprint-lines.py --verify` → `0 rows without a footprint` (est. 1 min).
Commit:

## Validation Plan

- Tests: the three scripts' `--selftest` and `--verify`.
- Runtime proof: not applicable (data only).
- Doc sync: `planning/plan-72-data/README.md` (the new files).
- Final gate (once): `pinout-check.py`, `template-check.py`, `footprint-lines.py --verify` all exit 0; `git status --short` shows only `planning/plan-72-*` and `planning/plan-72-data/` (est. 2 min).

## Decisions

- Templates over per-device hand-written JSON: 459 candidate parts share about a dozen register maps — the author's call.
- MCP23x17 is modeled in BANK = 0 only (§ 3, rejected alternatives).
- Blink, PWM, key scan, input latch, drive strength and bus-fault timers are stored and not simulated — the plan-56 precedent (LED intensity, key scanning).

## Corrections

(none yet)
