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

- [x] (added: plan-12-I's front ends, pulled into J because J needs them
  first; see Corrections) SPI target and I2C target front ends,
  byte-level, in `backend/src/models/serial_target.c/.h` (not
  `serial_bus.c`), with the `"bus"` loader for the `spi`, `i2c` and
  `i2c-spi` styles; `pin_open_drain` reports SDA/SCL and the IRQ pins.
  Check: `/tmp/logic-p12j/serial_test | grep '^ok   JF'` → `ok   JF1 SPI
  target: MISO Z while CS is high, bytes both ways, X on CS`, `ok   JF2
  I2C target: START, repeated START, STOP, ACK/NACK, reads, straps on
  SCL/SDA`.
- [x] `serial_max3100.c`, `serial_max310x.c`; `docs/parts-format.md` §
  `serial` gains §3 (the bus styles, `max3100`, `max310x`). Check: build
  clean, `cmake --build /tmp/logic-p12j -j8 2>&1 | grep -ci warning` → `0`
  (also `cc -std=c11 -Wall -Wextra -pedantic` on the four serial sources:
  no output).
- [x] Fixtures (`backend/tests/support/serial_fixture_maxim.h`) and cases
  J1–J6. Observed: `/tmp/logic-p12j/serial_test` → every earlier case
  `ok`, then `ok   J1 MAX3100: commands, baud table, FIFO, IRQ masks, T
  latch, loopback, 9-bit, shutdown`, `ok   J2 MAX3140: J1 registers, Y/Z
  true/complement and Z, A/B fail-safe receive, test mode TX low`, `ok
  J3 MAX3107: SPI and I2C command bytes, bursts, FIFO levels, flow
  control, special characters, IrDA`, `ok   J4 MAX3107 clocking: bypass
  counts XIN, PLL x6 and x48 measure it; fraction, 2x`, `ok   J5 MAX3109:
  U bit, per-channel registers, GlobalIRQ, fast read, extended map,
  triggers`, `ok   J6 MAX14830 U1:U0 channels and I2C offsets; MAX3108 WLP
  balls and its address table`, `PASS`. Four model mutations (IrDA width,
  noise flag, RS-485 hold, multidrop filter) each turned J3 to `FAIL`.

Acceptance: J1–J6 and every earlier case `ok`; `ctest` 100% (about 1 min).
Observed: `cmake -S backend -B /tmp/logic-p12j && cmake --build
/tmp/logic-p12j -j8 && ctest --test-dir /tmp/logic-p12j` → `100% tests
passed, 0 tests failed out of 18`; `git diff --stat backend/tests/golden`
→ no output.
Commit: ad60d85 (front ends), 2a71c5c (models), 92e96e5 (fixtures and
cases), 2bfe355 (docs)

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

- **Phase 1 ran before its prerequisite.** plan-12-I had not landed (the
  Prerequisites row stays NOT MET); Phase 1 was done on the parent
  session's instruction, on 627f50d (plan-12-C), models and tests only.
  Phase 2 (parts, footprints, goldens, stock) waits for I.
- **Front ends pulled into J.** plan-12-I §3.1's SPI and I2C target front
  ends landed in J Phase 1 (added task), because J's models are the first
  to need them. They are byte-level helpers in a new
  `backend/src/models/serial_target.c/.h`, not in `serial_bus.c` (§2's
  "SPI and I2C target front ends exist (`serial_bus.c`)"), which stays the
  Intel bus helper. API for I's bridges: `SerialSpi` (events `begin`,
  `next(index)` → the byte to shift out with a per-bit drive mask, `bit`
  (lets a reply depend on the command's first bits), `received(index)`,
  `shift`, `end(bits)`), `SerialI2c` (events `begin(repeated)`,
  `address(byte)` → ACK/NACK/X, `received(index)` → ACK/NACK/X,
  `next(index)`, `acked(index, ack)`, `end(stop)`; `serial_i2c_strap`
  tells a strap tied low/high/SCL/SDA), and `SerialHost` for a `"bus"`
  object (`serial_host_load/bind/init/start/pin/hold`,
  `serial_host_i2c_address` from the strap table). Timing (`t_valid`,
  `t_out`, `t_float`) is per instance, set by the model from its grade.
- **MAX3140 transceiver is independent of the UART** (§3.1 said Y and Z
  drive the UART's TX, RTS-controlled). `maxim.md` § MAX3140: "the two
  halves share only supply/ground (p1)", "connect TX→DI, RX←RO, RTS→DE/RE
  externally (p1, p10)"; driver "DE = 1 → Y = DI XOR TXP, Z = NOT Y. DE = 0
  → Y, Z high-Z". The model drives Y/Z from DI, TXP and DE.
- **A = B reads 1, not X** (§3.1 and the J2 row said X). `maxim.md` §
  MAX3140 receiver: RO = ((A−B) ≥ −50 mV) XOR RXP, and "Open/shorted/
  idle-terminated inputs → RO = 1 XOR RXP ('true fail-safe')" (p11–12,
  p24–25). The model reads A = B (shorted) and both open as 1; an X input,
  or one open input, gives X. J2 checks all four.
- **MAX3140 terminal** (§3.3): kept as planned (A true, B complement,
  driven-net check on A); since the UART's RX is a separate pin, the UART
  receiver also hears the typed waveform directly (plan-12-C §3.4 rule).
- **Mode pin followed live.** plan-12-I §3.1 samples the I2C/SPI select at
  reset (the SC16IS7xx rule). The Maxim pin tables say "When I2C/SPI is
  high, DOUT/SDA functions as the DOUT SPI serial-data output" (max3107
  p12) with no sampling time, so the Maxim devices use `"selectSampled":
  "live"`; the front end supports both.
- **MISO during the command byte and writes on the MAX3107/MAX3108:** Z.
  `maxim.md`: "SDO state outside the data byte is not described"; Figures
  15/16 (max3107 p44) draw no SDO there, and the register-compatible
  MAX3109/MAX14830 print Z (max3109 p57–58). An X on a shared MISO would
  break every other target on it, so the model uses Z.
- **GloblComnd stays at 0x1F in the extended map.** The extended address is
  "generated as 0010 A3A2A1A0" (max3109 p53), which would send 0x1F to
  0x2F, but the same page says "To return to standard addressing mode, the
  SPI master sends the 0xCD command to register 0x1F". The model keeps 0x1F
  on GloblComnd while extended access is on.
- **MAX3107 has no extended registers** (J3 row "extended registers"): its
  map ends at 0x1F (`maxim.md` register map). J3 checks that register 0x20
  is refused (I2C NACK); the extended map is checked on the MAX3109 (J5)
  and MAX14830 (J6).
- **PLLBypass = 0 with PLLEn = 0 gives no clock** (not in §3.2): max3107
  p17 Figure 7 puts PLLByps after the PLL, and "If PLLEn is set low, the
  internal PLL is disabled" (p42). J4 checks that nothing is sent.
- **IrDA polarity on the max310x** is not printed (`maxim.md` § MAX3107
  IrDA: only pulse widths). The model uses the MAX3100's (TX idles high, a
  0 is a low pulse; RX idles low, a 0 is a high pulse; P9, vendor
  convention); TxInv/RxInv invert it.
- **Receiver after a framing error** (max310x): the MAX3107 family does not
  say; the model waits for a new start bit as the MAX3100 does ("After FE
  the receiver resyncs to look for a start bit", P9) instead of the line
  engine's PC16550D resync.
- **MAX3100 overflow:** "The FIFO is cleared if it overflows" (p7); the
  word that overflowed is not kept either (RA/FE set, decisions.md).
- **CTSInt** counts CTS 0 ↔ 1 transitions only; a pin coming out of X at
  power-up is no transition.
- **Ready time** (`tready`): only the MAX3109 prints one (about 200 µs,
  p62); the other grades leave it 0.
- **MAX3109 MODE2[0]** resets the addressed UART only, as the MAX14830
  prints (p40); the MAX3109 text is silent.
- **Internal pulls are read, not driven:** RX reads Z as 1 (weak pull-up),
  GPIO inputs and the MAX3140's H/F, TXP, RXP read Z as 0 (weak
  pull-downs). Driving them would collide with the terminal's drive on RX.
- **Control bits written X** act as 0, except the line format (X frames)
  and the divisor and clock registers (the generator stops). An SPI
  command with X address bits stores X in every register it could name
  (P2; J3 checks LCR).
- **Fixture footprints:** existing footprints of the same pin count
  (PDIP-14, SSOP-16, SSOP-24, SOIC-28W, TSSOP-24, PLCC-32, TSSOP-48). No
  25-pad footprint exists, so the MAX3108 fixture part uses `SFX-WLP-25`, a
  copy of `footprints.md`'s `WLP-25` object under a fixture id, loaded by
  the fixture header; the real `WLP-25` lands in Phase 2.
- **Case names:** the front ends' direct cases print as `JF1`, `JF2`.
