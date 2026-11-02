# plan-12-I: I2C/SPI-to-UART bridges and the SC18IS604 I2C controller

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-H

I adds the I2C target and SPI target host front ends to the serial models,
the seven UART bridges that put an `ns16550` core behind them, and the
`sc18is604` model (an SPI target that is an I2C bus controller): 8 devices,
22 parts, 4 new footprints.

| Device | Model / host | Parts |
|---|---|---|
| SC16IS740 | `ns16550`, I2C or SPI by pin, no GPIO | 3 |
| SC16IS750 (SC16IS750, SC16IS760 parts) | `ns16550`, I2C/SPI, 8 GPIO | 3 |
| SC16IS752 (SC16IS752, SC16IS762 parts) | `ns16550`, 2 channels, I2C/SPI, GPIO | 7 |
| PI7C9X760, PI7C9X762 | `ns16550`, I2C/SPI, SFR bank | 1, 1 |
| XR20M1170 | `ns16550`, I2C/SPI, DLD | 4 |
| XR20M1280 | `ns16550`, its own register layout (3-bit addresses, separate TX/RX baud generators) | 2 |
| SC18IS604 | `sc18is604`: SPI target, I2C controller | 1 |

(22 parts in all: `awk -F'\t' '$16=="add"' plan-12-data/parts.tsv` rows of
these devices.)

Outcome of I: `serial_test` cases I1–I9 pass, including an end-to-end chain
in which a bench SPI master tells an SC18IS604 to write `0x55` to an
SC16IS750's THR over I2C (with `PULLUP-3V3` on SDA and SCL) and the
SC16IS750's TX sends `0x55`; `serial.json` gains the 8 devices; goldens + 22,
+ 22, + 4 footprints, only added lines.

References: `plan-12-data/specs/bridges.md` (all 8 devices: pins, address
tables, sub-address and SPI command layouts, registers, SC18IS604 commands
and status codes); `matrix.md` Table A (bridge rows) and Table B (host
protocols); `decisions.md` (bridges; planner decisions for SC16IS IIR/THR,
SC16IS740, SC16IS752 GPIO interrupt, PI7C9X TIDLE, SC18IS604 repeated START
and pull-ups); `footprints.md` § QFN-24-4X4, TQFN-24-4X4-ZD, TQFN-32-5X5-ZH,
TSSOP-28; plan-12-A §3.6 (pull parts: I2C needs them).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-H landed | `/tmp/logic-p12/serial_test \| grep -c '^ok H'` → 8 | NOT MET (2026-11-02) |

## 1. Goal

- `serial_test` I1–I9 `ok`, every earlier case `ok`.
- Goldens + 22 / + 22 / + 4, no removed line; `ctest` 100%.

### Non-goals

- No change to the Intel/Motorola bus paths of D/E. No I2C target part other
  than these bridges (the library has no I2C slave parts; owner scope).

## 2. Current State

After H: every host bus is parallel (`serial_bus.c`: Intel, Motorola). The
pull parts (A) give an open-drain bus its 1.

## 3. Design

### 3.1 Front ends (`serial_bus.c`)

- **I2C target**: `"bus": {"style": "i2c", "scl", "sda", "address": [A0, A1]
  pin names, "table": [16 or 8 seven-bit addresses indexed by the A-pin
  combination (`bridges.md` address tables, VDD/VSS/SCL/SDA ties as the
  table lists)]}`. START (SDA 1→0 while SCL 1), STOP, repeated START; the
  address byte; ACK by driving SDA 0 (open-drain: 0 strong or Z) for the
  ninth clock; register access per the device's sub-address byte (register
  and channel bits); reads shift out on SCL falling edges. No clock
  stretching (`decisions.md`).
- **SPI target**: `"bus": {"style": "spi", "cs", "sclk", "mosi", "miso",
  "mode": 0}` and the device's command byte layout (R/W bit, register bits,
  channel bits); MISO 3-state while CS is high; burst rules per spec.
- **Pin-selected**: `"style": "i2c-spi"` with `"selectPin"` (I2C/SPI) and both
  sets; the pin's level at reset picks the front end (the SC16IS7xx rule in
  `bridges.md`).
- `model_pins.h` `part_pin_open_drain`: SDA, SCL (as outputs) and the
  bridges' open-drain IRQ are open-drain.

### 3.2 Bridge `ns16550` options

Keys for the seven bridges, values per `matrix.md` Table A: `"gpio"`
(`{"pins": [...], "registers": "sc16is"}`, absent on SC16IS740: its GPIO
addresses read X), `"extended": "sc16is"` / `"sc16is-sfr"` (PI7C9X76x) /
`"sc16is-dld"` (XR20M1170) / `"xr1280"` (XR20M1280: 3-bit register space,
independent TX/RX generators), `"efcr"` (SC16IS RS-485, IrDA fast mode),
`"lcrReset": 29`, interrupt open-drain IRQ, the GPIO interrupt in channel
A's IIR (SC16IS752, planner decision), TIDLE stored only (PI7C9X, planner
decision), an IIR read clears the THR interrupt (planner decision).

### 3.3 `sc18is604` (`serial_sc18is604.c`)

SPI target (§3.1) with the command set of `bridges.md` § SC18IS604 (write
N bytes, read N bytes, read after write and write after write with a
repeated START, read buffer, write/read internal registers, power-down);
the I2C controller drives SCL and SDA open-drain at the rate set by its clock
register from its internal oscillator (a model time base, `sim_schedule_timer`:
the chip has no clock pin), samples SDA, reports ACK/NACK and arbitration
loss in its status register with the datasheet's codes, timeout by its timeout
register, INT (open-drain) at the end of each transaction. No internal
pull-ups (planner decision). `serial_channels` → 0 (no UART; no terminal).

## 4. Tests (added to `serial_test`)

A bench I2C master and SPI master (bit-banged from the test through
`circuit_*` with `PULLUP-3V3` on SDA/SCL) drive the targets.

| Case | Fixture | Checks |
|---|---|---|
| I1 | SC16IS750 (I2C) | address table for every A1/A0 tie; write LCR/DLL/THR, TX frame; read RHR after an RX frame; NACK for a wrong address |
| I2 | SC16IS750 (SPI) | command byte R/W, register, channel bits; MISO 3-state; burst FIFO write |
| I3 | SC16IS752 | both channels; GPIO input change interrupt in channel A's IIR |
| I4 | SC16IS740 | no GPIO: those addresses read X |
| I5 | PI7C9X760, PI7C9X762 | SFR bank; TIDLE reads back, no effect |
| I6 | XR20M1170 | DLD fractional divisor; its address table |
| I7 | XR20M1280 | 3-bit register layout; independent TX/RX baud |
| I8 | SC18IS604 | each command; START/STOP/repeated START waveforms; ACK and NACK status codes; clock register rates; timeout |
| I9 | SC18IS604 → SC16IS750 | the end-to-end chain of the Outcome |

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Front ends and bridge options

- [ ] I2C target, SPI target, pin-selected front end (§3.1); open-drain pins.
- [ ] Bridge options (§3.2); fixtures and cases I1–I7;
  `docs/parts-format.md` § `serial` gains the host styles.

Acceptance: I1–I7 and every earlier case `ok`; `ctest` 100% (about 1 min).
Commit: —

### Phase 2 — SC18IS604 and parts

- [ ] `serial_sc18is604.c` (§3.3); cases I8, I9.
- [ ] Stock re-check (`SC16IS7`, `SC18IS604`, `PI7C9X76`, `XR20M1`: 4 calls);
  drops in Corrections.
- [ ] `footprints.json` + 4; `serial.json` + 8 devices; regenerate the
  goldens. Check: no removed line; added 22, 22, 4; hand-check the 4
  footprint lines.

Acceptance: `serial_test` I1–I9 `ok`; `ctest` 100%; the golden checks (about
2 min).
Commit: —

## Validation Plan

`serial_test` (front ends, bridges, the SPI→I2C→UART chain) and the golden
programs are the gate. The landing tier is plan-12-K's.

## Decisions

- The SC18IS604's I2C clock comes from a model time base (the chip's internal
  oscillator has no pin); every other serial model counts its clock pin.
- SC16IS740 is its own device (planner decision, `decisions.md`).

## Corrections

- **The SPI and I2C target front ends landed with plan-12-J** (an added
  task in J Phase 1, J needed them first), as byte-level helpers in
  `backend/src/models/serial_target.c/.h` rather than `serial_bus.c`, with
  the `"bus"` loader for the `spi`, `i2c` and `i2c-spi` styles (the latter
  with `"selectSampled"`: `"reset"`, this plan's rule, is the default). The
  API is summarized in plan-12-J's Corrections. Nothing in this plan is
  ticked: §3.1's box also covers the bridges' sub-address layouts and the
  `ns16550` wiring, which are still I's.
