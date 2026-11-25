# plan-62-F: AVR simulation — USART, SPI master and I2C master at the pin level

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-E

avr8js models its serial peripherals as byte callbacks: a byte leaves, a
byte arrives, no pin moves. F writes the layer between those callbacks and
the engine's nets, so firmware talks to the other parts of a schematic:
frames on TXD and RXD, clock and data on SCK/MOSI/MISO, and an open-drain
I2C master on SDA/SCL. This layer is ours; avr8js cannot check it, so it is
tested against shipped parts that already speak these buses.

Outcome: `ctest -R avr_serial_test` passes: a fixture ATmega328 sends "HI"
through an `MCP2200-I/SO` and echoes a typed byte; writes and reads back an
`AT25320B-SSHL-B` SPI EEPROM; sets and reads the seconds register of a
`PCF8563T/5,518` over I2C.

References:

- plan-62-C § 3.1 (the callback API, `avr_usart_bit_cycles`,
  `avr_twi_bit_cycles`), plan-62-E (§ 3.2 the clock, § 3.4 GPIO,
  `mcu_avr.c`), plan-62-D (the 328PB's second USART, SPI and TWI and their
  pins).
- ATmega328P datasheet (DS40002061): § 19.6–19.7 USART frame and receiver
  sampling, § 18.2–18.4 SPI modes and pin overrides (Table 18-1), § 21.5–
  21.7 TWI bit transfer and master modes.
- Partner parts: `serial.json` `MCP2200` (model `usbuart`),
  `serialmem.json` `AT25320B`, `rtc.json` `PCF8563` (7-bit address 0x51).
  Their tests: `usbuart_test`, `serialmem_test`, `rtc_test`.
- `docs/backend.md` (the `circuit_serial_*` notes).

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-E is complete | `ls planning/completed/plan-62-E-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after 4b51faa0) |

If plan-62-E is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- With TXEN set, the USART's TXD pin carries each written byte as a frame;
  with RXEN set, a frame on RXD arrives in UDR.
- With SPE and MSTR set, a SPDR write clocks 8 bits out on MOSI/SCK and
  reads 8 bits from MISO.
- With TWEN set, the TWI master's start, address, data, acknowledge and
  stop conditions appear on SDA and SCL, and a target's acknowledge and
  data are read back.
- All of it for every USART, SPI and TWI of the layout (two of each on the
  `atmega328pb`).

### Non-goals (explicit constraints)

- `avr_usart.c`, `avr_spi.c` and `avr_twi.c` do not change: the scenarios
  of plan-62-C must still equal avr8js.
- Not modeled, as in avr8js: the USART's synchronous mode (XCK), SPI slave
  mode (a chip with MSTR clear drives nothing), TWI slave mode,
  multi-master arbitration, frame, parity and overrun error flags (a frame
  with a bad stop bit is dropped; a parity bit is sent but not checked).
- No terminal on the chip itself: `circuit_serial_channels` stays 0 for an
  mcu. A terminal comes from a USB-to-UART bridge part wired to TXD/RXD.

## 2. Current State

After E, `mcu_avr.c` runs the chip and maps port state to pin drives; the
serial peripherals complete their transfers through the default callbacks
and touch no pin. The adapter knows the CPU cycle's length in picoseconds
(`cycle_ps`: 125 000 × prescaler internal; the measured PB6 period ×
prescaler external) and runs each instruction at a known engine time.

The partner parts' pins (read in the part files, 2026-11-24):

- `MCP2200-I/SO` (`soic20`): `Tx` 10 (output), `Rx` 12 (input), `RTS` 11,
  `CTS` 13, `RST` 4, `VDD` 1, `VSS` 20, `Vusb` 17. Its channel is set with
  `circuit_serial_configure` and read with `circuit_serial_sent` /
  typed into with `circuit_serial_send` (`usbuart_test`).
- `AT25320B-SSHL-B` (`soic8`): `CS#` 1, `SO` 2, `WP#` 3, `GND` 4, `SI` 5,
  `SCK` 6, `HOLD#` 7, `VCC` 8; SPI modes 0 and 3, 5 ms write cycle.
- `PCF8563T/5,518` (`so8`): `SDA` 5, `SCL` 6, `VDD` 8, `VSS` 4; register
  0x02 is seconds (BCD, bit 7 = VL).

### Measured populations

| What | Count | Source |
|---|---|---|
| Serial peripherals per layout | `atmega328`, `atmega168pb`: 1 USART, 1 SPI, 1 TWI; `atmega328pb`: 2 of each | plan-62-D § 2 |
| Pins | USART0 RXD PD0, TXD PD1; SPI0 SS PB2, MOSI PB3, MISO PB4, SCK PB5; TWI0 SDA PC4, SCL PC5; USART1 RXD PB4, TXD PB3; SPI1 MISO PC0, SCK PC1, SS PE2, MOSI PE3; TWI1 SDA PE0, SCL PE1 | `planning/plan-62-data/atdf_signals.txt` |

### Verified properties

- **A serial peripheral's timing is in CPU cycles** (`usart.ts`
  `cyclesPerChar`, `spi.ts` `transferCycles`, `twi.ts` uses no time at
  all), so the pin layer converts cycles to picoseconds with `cycle_ps`.
- **The engine queues future drives**: `sim_schedule_drive` appends an
  event at `now + delay` for each call (`sim.c:574`), so a whole frame can
  be scheduled when the byte is written.

## 3. Design Overview

One new file, `backend/src/models/mcu_avr_serial.c`, called from
`mcu_avr.c`. Three independent pieces (USART, SPI, TWI) over one shared
mechanism, the pin override.

Correctness risk concentrates in the TWI master: it is a bit-level state
machine on two open-drain lines with an external party. It is tested
against a real target model (`PCF8563`) and by measuring the waveform's
timing and order.

Rejected alternatives:

- *Timing the bits from avr8js's clock events*: `avr_cpu_tick` runs events
  only between instructions, so an SPI bit at fosc/2 (one cycle per half
  bit) could not be placed. The pin layer schedules in engine time.
- *Reusing `serial_line.c`'s transmitter and receiver*: they are driven by
  a 16× baud clock tick the UART models generate; the AVR has cycle counts,
  and a frame scheduled in one go is simpler than a tick source.

### 3.1 Pin overrides

`mcu_avr.c`'s drive for a port pin becomes: the serial layer's override
when one is set for that pin, else the port's state (plan-62-E § 3.4). An
override is `drive strong 0`, `drive strong 1`, `release` (Z), or `none`.
A pin's input still reaches the port (`avr_port_set_pin`) and the serial
layer.

### 3.2 USART

- **TXD.** While TXEN is set the pin is overridden: strong 1 when idle.
  `on_transmit(byte)` appends a frame to the pin's schedule, starting at
  the later of now and the end of the previous frame: start bit 0, the data
  bits LSB first (5–8 from the byte; the ninth from TXB8), the parity bit
  when enabled (even or odd over the data bits), 1 or 2 stop bits; each bit
  lasts `avr_usart_bit_cycles × cycle_ps`. Clearing TXEN drops the override
  once the last frame has ended.
- **RXD.** While RXEN is set and the receiver is idle, a 1 to 0 change on
  RXD starts a frame: the adapter schedules a timer at 1.5 bit times and
  then every bit time, samples the pin at each, and after the stop bit
  calls `avr_usart_write_byte(value, immediate)` when the stop bit read 1
  and `avr_usart_rx_busy` is false. A start bit that reads 1 at 0.5 bit
  times is ignored.

### 3.3 SPI master

`on_byte(value)` with MSTR set: the adapter overrides SCK and MOSI **only
where their DDR bit is set** (the datasheet's Table 18-1: in master mode
those directions are the user's), leaves SS and MISO to the port, and
schedules 16 half bits of `avr_spi_clock_divider / 2 × cycle_ps`:

- SCK idles at CPOL; the leading edge is the first change.
- CPHA 0: MOSI holds bit 1 from the start, MISO is sampled on each leading
  edge, MOSI changes on each trailing edge. CPHA 1: MOSI changes on each
  leading edge, MISO is sampled on each trailing edge.
- Bit order from `avr_spi_lsb_first`.

MISO samples are timers; X or Z reads 0. A clock event at
`avr_spi_transfer_cycles` (the time avr8js completes the transfer) calls
`avr_spi_complete_transfer(received)`. After the last half bit SCK is back
at CPOL and MOSI keeps its last value; the overrides drop when SPE clears.

### 3.4 TWI master

While TWEN is set, SDA and SCL are overridden: `release` when the bus
should read 1, `drive strong 0` otherwise (open drain; the bus needs
pull-ups: a resistor part, or the chip's own port pull-ups, which E's GPIO
still applies as weak 1 when the override releases and PORT is 1). One bit
is `avr_twi_bit_cycles × cycle_ps`, half low, half high.

| avr8js event | On the pins | Then |
|---|---|---|
| `start(repeated)` | (repeated: SDA released, SCL released, half bit) SDA low while SCL high, half bit, SCL low | `avr_twi_complete_start` |
| `connect(address, write)` | 8 bits of `address << 1 \| !write`, MSB first, each set while SCL is low and held while it is high; a ninth clock with SDA released, sampled while SCL is high | `avr_twi_complete_connect(ack = SDA read 0)` |
| `write_byte(value)` | the same with `value` | `avr_twi_complete_write(ack)` |
| `read_byte(ack)` | 8 clocks with SDA released, sampled high; a ninth with SDA low for an ack, released for a nack | `avr_twi_complete_read(value)` |
| `stop()` | SDA low while SCL low, SCL released, half bit, SDA released | `avr_twi_complete_stop` |

After releasing SCL the master waits until the SCL net reads 1 before it
times the high half (a target that stretches the clock). An X or Z on SDA
when sampled reads 1 (no acknowledge).

### 3.5 Timers and reset

The serial layer's timers carry a generation number; a reset (PC6, the
watchdog) or the circuit's stop bumps it, drops every override and clears
the schedules, so nothing of a transfer outlives a reset.

## Compatibility / Format Impact

None: no file format, public header or golden changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — overrides and the USART

- [x] `backend/src/models/mcu_avr_serial.c` (+ `mcu.h`): the override
      table, § 3.2, § 3.5; `mcu_avr.c`: the drive rule of § 3.1, the
      callbacks installed for every USART of the layout;
      `backend/CMakeLists.txt`.
- [x] Test firmware `usart_hi` (9600 baud at 1 MHz with U2X, UBRR 12; send
      "HI", then echo each received byte + 1).
- [x] `backend/tests/avr_serial_test.c` (`logic_test`): `TESTAVR` with TXD
      on the `MCP2200-I/SO`'s `Rx` and RXD on its `Tx`, the bridge
      configured to 9600 8N1: the bridge's sent log is "HI"
      (`circuit_serial_sent`); typing 'a' (`circuit_serial_send`) brings
      back 'b'; the TXD net is 1 before the first byte and each bit lasts
      104 000 000 ps (104 cycles of 1 µs); a 7E2 firmware variant's frame
      has 7 data bits, even parity and 2 stop bits (read from the net's
      change times).

Acceptance: the USART cases pass.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_serial_test --output-on-failure` → `ok usart_hi`, `ok usart_echo`, `ok usart_bit_time`, `ok usart_7e2` (est. 2 min).
  Observed: 2026-11-24: `ok usart_hi`, `ok usart_type`, `ok usart_echo`, `ok usart_bit_time`, `ok usart_7e2`.
Commit: —

### Phase 2 — SPI master

- [x] `mcu_avr_serial.c`: § 3.3 for every SPI of the layout.
- [x] Test firmware `spi_eeprom` (SS on PB2 as GPIO; WREN, WRITE 3 bytes at
      0x0010, poll RDSR until ready, READ them back to PORTD).
- [x] `avr_serial_test.c`: `TESTAVR` with an `AT25320B-SSHL-B`: after the
      run `circuit_memory_read` of the EEPROM shows the 3 bytes and PORTD's
      nets show the last one read back; SCK has 8 pulses per byte at the
      programmed divider in mode 0 and, with a mode 3 variant, idles high;
      with DDR of SCK clear nothing is driven.

Acceptance: the SPI cases pass.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_serial_test --output-on-failure` → 4 USART lines plus `ok spi_eeprom`, `ok spi_mode3`, `ok spi_ddr` (est. 2 min).
  Observed: 2026-11-24: `ok spi_eeprom`, `ok spi_mode3`, `ok spi_ddr`.
Commit: —

### Phase 3 — TWI master

- [x] `mcu_avr_serial.c`: § 3.4 for every TWI of the layout.
- [x] Test firmware `twi_rtc` (write 0x25 to register 0x02 of address 0x51,
      repeated start, read it back to PORTB, stop) and `twi_nack` (address
      0x20: no target; writes TWSR's status to PORTB).
- [x] `avr_serial_test.c`: `TESTAVR` with a `PCF8563T/5,518` and the port
      pull-ups enabled by the firmware: PORTB shows 0x25
      (`circuit_converter_registers` on the RTC shows seconds 0x25); the
      SDA/SCL change log is a start (SDA falls while SCL is 1), 9 clocks per
      byte, and a stop; `twi_nack` shows status 0x20; a `TESTAVR328PB`
      fixture (added to `mcu_fixture.h`, pinout `quad32` of `atmega328pb`)
      runs `twi_rtc` on TWI1 (PE0/PE1).
- [x] `docs/backend.md`: the Microcontrollers note gains the serial pins
      (what is driven when, what is not modeled); § Layout; § Tests
      (`avr_serial_test`).

Acceptance: the TWI cases pass and plan-62-C's scenarios still equal
avr8js.
  Check: `ctest --test-dir /tmp/sim-rel -R 'avr_|mcu_test' --output-on-failure` → 4 tests passed, `avr_serial_test` printing `ok twi_rtc`, `ok twi_waveform`, `ok twi_nack`, `ok twi1_rtc` (est. 3 min).
  Observed: 2026-11-24: `ok twi_start`, `ok twi_rtc`, `ok twi_waveform`, `ok twi_nack`, `ok twi1_rtc`; `avr_serial_test: all passed`; the 16 scenarios still equal avr8js.
Commit: —

## Validation Plan

- Tests: `avr_serial_test` (11 cases); `avr_periph_test` unchanged.
- Coverage check: each case asserts on the partner part's own state (the
  bridge's sent log, the EEPROM's memory, the RTC's register), so a
  waveform the partner does not accept fails the case.
- Runtime proof: the three partner circuits above.
- Doc sync: `docs/backend.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions (serial pins in this plan). None added.

## Corrections

- § 3.4 (a bug found by the partner part, fixed in the design): the first
  version changed SDA at the instant SCL fell (half-bit steps), which a
  target reads as SDA moving while SCL is high: a start or stop. Traced on
  the `twi_rtc` rig: `1071 us SDA 0 SCL 0` (both in one step), and the
  PCF8563 did not acknowledge (SDA 1 on the ninth clock). A bit is now four
  quarters: SCL low for two with SDA changing between them, SCL high for
  two. After the fix the clock acknowledges and SCL changes 132 times for the
  transaction, the count derived in the test (1 + 7 x 9 x 2 + 2 x 2 + 1).
- § 3.2: the transmitter sends bit by bit with a timer per bit (not a whole
  frame scheduled at once), so a reset cancels a frame. Bytes written while a
  frame is going out wait in a queue of 8.
- § 3.3: the SPI transfer is completed by the adapter's last clock-edge
  timer (edge 16), not by a CPU clock event: with CPHA set the last sample is
  at that same time, and the two would race. With MSTR clear the transfer
  completes with 0 through a clock event.
- § 3.1: the peripherals' pins are tables of the layout
  (`AvrLayout.usart_pins`, `spi_pins`, `twi_pins`). The adapter hears SPCR
  and TWCR writes by chaining its own write hook in front of the
  peripheral's; `avr_usart.c`, `avr_spi.c` and `avr_twi.c` are unchanged (the
  16 scenarios still equal avr8js).
- The rig moved to `tests/support/avr_rig.h` and finds nets by pin name, so
  it takes any pinout; the fixture `TESTAVR328PB` (quad32) was added.
- Phase 2: the mode 3 case is its own firmware (`spi_mode3`: WREN, RDSR, the
  status 0x02 on PORTD), and the no-DDR case is `spi_noddr`.
- All three phases landed in one commit (one file, one test).

## Summary

This letter is the one piece avr8js cannot check; shipped parts that
already implement the other end of each bus check it instead.
