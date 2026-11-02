# plan-12-J: Maxim SPI/I2C UARTs — `max3100`, `max310x`

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-I

J adds the two Maxim register models and their 18 parts with the 7 new
footprints they first use.

| Model | Devices (parts) |
|---|---|
| `max3100` | MAX3100 (7), MAX3140 (2) |
| `max310x` | MAX3107 (4), MAX3108 (1), MAX3109 (2), MAX14830 (2) |

Outcome of J: `serial_test` cases J1–J6 pass; `serial.json` gains the 6
devices; goldens + 18 / + 18 / + 7 footprints (`QSOP-16`, `QSOP-28`,
`TQFN-24-3.5X3.5`, `TQFN-24-4X4-T2444`, `TQFN-32-5X5-T3255`, `TQFN-48-7X7`,
`WLP-25`), only added lines.

References: `plan-12-data/specs/maxim.md` (all six: pins, SPI/I2C protocol,
registers, clocking, timing); `matrix.md` Table B; `decisions.md` (Maxim;
planner decisions for test mode and the T bit); `footprints.md` §§ of the 7
footprints; plan-12-I §3.1 (SPI and I2C target front ends).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-I landed | `/tmp/logic-p12/serial_test \| grep -c '^ok I'` → 9 | NOT MET (2026-11-02) |

## 1. Goal

- `serial_test` J1–J6 and every earlier case `ok`; goldens + 18 / + 18 / + 7,
  no removed line; `ctest` 100%.

### Non-goals

- No change to I's front ends beyond the Maxim command layouts.

## 2. Current State

After I: SPI and I2C target front ends exist (`serial_bus.c`); the line engine
supports the frame formats the Maxim parts use.

## 3. Design

### 3.1 `max3100` (`serial_max3100.c`)

MAX3100: a 16-bit SPI word per transaction (write configuration, read
configuration, write data, read data: `maxim.md` § MAX3100), 8-word RX FIFO,
the baud rate table from the crystal/clock pin (X1 driven by an oscillator,
owner decision), IRQ (open-drain) by its mask bits, RTS/CTS, the T bit cleared
on CS rising (planner decision), test mode per the part's own datasheet:
MAX3100 digital loopback, MAX3140 TX held low while CS is low (planner
decision), the 9-bit (parity-as-data) mode, shutdown.

MAX3140 adds the RS-485/422 transceiver's digital side (`"transceiver":
{"y", "z", "a", "b", "de", "re", "ro", "di", "hf", "srl"…}`, pins per
`maxim.md` § MAX3140): Y and Z drive the true and complement of the UART's TX
while the driver is enabled (RTS-controlled per its pins), Z (high impedance)
otherwise; the receiver input is A when A ≠ B (A = 1, B = 0 reads 1; A = 0,
B = 1 reads 0) and X when A = B; half/full duplex by H/F.

### 3.2 `max310x` (`serial_max310x.c`)

MAX3107 (1 UART), MAX3108 (1, WLP), MAX3109 (2), MAX14830 (4); SPI or I2C
host (`maxim.md` per device; I's front ends with the Maxim command byte: R/W
bit 7 = 1 for write, `maxim.md` note, and the U/U1:U0 channel bits), extended
register access, 128-word FIFOs, the full register set (IRQ, LSR, special
character, STS, MODE1/2, LCR, RxTimeOut, HDplxDelay, IrDA, flow level/control,
Xon/Xoff, GPIO, PLL, BRG, clock source, global commands).

Clocking: crystal (XIN/XOUT, 1–4 MHz) or external clock (0.5–35 MHz) on XIN,
from an oscillator part (owner decision); prescaler ÷1–63 and PLL ×6/×48/×96/
×144, then the 16-bit + 4-bit fractional divisor with 2×/4× rate modes. With
the PLL bypassed the model counts XIN edges (as every serial model); with the
PLL on, a counted clock cannot be multiplied, so the model measures the XIN
period over the last two rising edges and schedules its baud ticks with
`sim_schedule_timer` at period × prescaler ÷ PLL × divisor (recomputed on
each XIN edge, so a changed clock takes effect within one input period).

LCR resets to 0x05 (6 data bits, 2 stops, `maxim.md`); MAX3109's revision
register per `decisions.md`; MAX14830's TxAutoDis bit position per
`decisions.md`.

### 3.3 Terminal

Each UART channel is a terminal channel (MAX14830: 4). MAX3140's terminal RX
drives the transceiver's A/B pair as true/complement (§3.1) — its RX
pin — under the same "nothing else drives it" rule (plan-12-C §3.5), checked
on A.

## 4. Tests (added to `serial_test`)

| Case | Fixture | Checks |
|---|---|---|
| J1 | MAX3100 | each 16-bit command; baud table; FIFO; IRQ masks; T bit; loopback test mode; 9-bit mode; shutdown |
| J2 | MAX3140 | J1's register cases; Y/Z true/complement and high impedance; A/B receive with A = B → X; test mode TX low while CS low |
| J3 | MAX3107 (SPI and I2C) | command byte; extended registers; FIFO levels; flow control; special characters; IrDA |
| J4 | MAX3107 clocking | PLL bypass (edge count) and PLL ×6 (measured period): frame bit time = expected within one XIN period |
| J5 | MAX3109 | channel bit U; per-channel registers |
| J6 | MAX14830, MAX3108 | U1:U0 channels; WLP ball pinout |

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Models

- [ ] `serial_max3100.c`, `serial_max310x.c`; `docs/parts-format.md` §
  `serial` gains §3.
- [ ] Fixtures and cases J1–J6.

Acceptance: J1–J6 and every earlier case `ok`; `ctest` 100% (about 1 min).
Commit: —

### Phase 2 — Parts

- [ ] Stock re-check (`MAX3100`, `MAX3140`, `MAX3107`, `MAX3108`, `MAX3109`,
  `MAX14830`: 6 calls); drops in Corrections.
- [ ] `footprints.json` + 7; `serial.json` + 6 devices; regenerate the goldens.
  Check: no removed line; added 18, 18, 7; hand-check the 7 footprint lines
  (the Maxim ones against the ADI land-pattern documents cited in
  `footprints.md`).

Acceptance: `ctest` 100%; golden checks (about 2 min).
Commit: —

## Validation Plan

`serial_test` J1–J6 and the golden programs. The landing tier is plan-12-K's.

## Decisions

- The PLL-on clock path uses a measured period (§3.2); everything else counts
  edges (plan-12-C §3.7).
- MAX3140's differential pins are modeled as true/complement logic (§3.1).

## Corrections

(none yet)
