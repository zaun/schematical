# plan-62-C: AVR simulation — USART, SPI, TWI, ADC, EEPROM and watchdog at register level

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-B

C ports the six remaining avr8js peripherals to C and adds them to the
`atmega328` layout. At this level a serial peripheral talks through
callbacks (a byte sent, a byte to receive), exactly as in avr8js; the pins
come in F.

Outcome: `ctest -R avr_periph_test` prints `ok` for 6 more scenarios
(`usart`, `spi`, `twi`, `adc`, `eeprom`, `watchdog`), 13 in all.

References:

- plan-62-A (core API, exact mode), plan-62-B (§ 3.2 the chip, § 3.3 the
  scenario format and trace).
- avr8js 0.21.1 sources ported here: `src/peripherals/usart.ts` (278
  lines), `spi.ts` (168), `twi.ts` (209), `adc.ts` (264), `eeprom.ts` (163),
  `watchdog.ts` (141).
- ATmega328P datasheet (DS40002061): § 19 USART, § 18 SPI, § 21 TWI, § 23
  ADC, § 7.4 EEPROM, § 10.8 watchdog.

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-B is complete | `ls planning/completed/plan-62-B-* \| wc -l` → 1 | NOT MET (2026-11-24: B not started) |

If plan-62-B is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `avr_usart.c`, `avr_spi.c`, `avr_twi.c`, `avr_adc.c`, `avr_eeprom.c` and
  `avr_watchdog.c` exist; the `atmega328` layout has USART0, SPI, TWI, the
  ADC, the EEPROM and the watchdog, and `layout-check` still prints
  `layout atmega328 equal`.
- `avr_periph_test` passes with 13 scenarios.

### Non-goals (explicit constraints)

- No pin is touched by USART, SPI or TWI here (F).
- What avr8js does not model stays unmodeled: the USART's synchronous and
  SPI-master modes, multi-processor mode, frame and parity errors, data
  overrun; SPI slave mode; TWI slave mode and arbitration; the ADC's
  auto-trigger sources, differential inputs and noise; the analog
  comparator; EEPROM contents that outlive a run.
- No change to the 7 fixes of plan-62-A; they are CPU-only.

## 2. Current State

After B, `avr_chip.c` assembles a CPU, ports B/C/D, the clock and timers
0–2, and `avr_periph_test` runs scenarios against the oracle.

The six avr8js classes (read 2026-11-24):

- `AVRUSART(cpu, config, freqHz)`: UCSRA/B/C, UBRR, UDR hooks. A UDR write
  calls `onByteTransmit(value)` at once, clears UDRE and TXC, and adds a
  clock event `cyclesPerChar` later that sets both.
  `writeByte(value, immediate)` delivers a received byte: at once when
  `immediate`, else after `cyclesPerChar` with `rxBusy` set meanwhile. A UDR
  read returns the byte masked to the character size and clears RXC.
  `cyclesPerChar = (UBRR + 1) × (U2X ? 8 : 16) × (1 + bits + stop + parity)`.
- `AVRSPI(cpu, config, freqHz)`: a SPDR write while SPE is set calls
  `onByte(value)`; the default `onByte` adds a clock event `transferCycles`
  (= `clockDivider × 8`) later that calls `completeTransfer(received)`,
  which stores SPDR and sets SPIF. A write during a transfer sets WCOL.
- `AVRTWI(cpu, config, freqHz)`: a TWCR write that clears TWINT with TWEN
  set adds a 0-cycle clock event that calls one of the event handler's
  `start(repeated)`, `stop()`, `connectToSlave(address, write)`,
  `writeByte(value)`, `readByte(ack)`; the handler answers with
  `completeStart`, `completeStop`, `completeConnect(ack)`,
  `completeWrite(ack)`, `completeRead(value)`, each of which sets the status
  in TWSR and the TWINT flag. Master mode only (`// TODO: add slave states`).
- `AVRADC(cpu, config)`: an ADSC write starts a conversion; `onADCRead`
  (default: `channelValues[channel]` volts against the selected reference)
  ends with `completeADCRead(value)` after `sampleCycles`.
- `AVREEPROM(cpu, backend, config)`: EECR/EEDR/EEAR hooks, the EEMPE then
  EEPE write sequence, erase and write times in cycles
  (`eraseCycles`, `writeCycles`), the ready interrupt.
- `AVRWatchdog(cpu, config, clock)`: the WDTCSR change-enable sequence, the
  timeout on a 128 kHz base scaled by the clock, the interrupt, the reset
  (sets WDRF in MCUSR and calls `cpu.reset()`), `WDR` through
  `cpu.onWatchdogReset`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines to port in C | 1223 | `wc -l src/peripherals/{usart,spi,twi,adc,eeprom,watchdog}.ts` (278 + 168 + 209 + 264 + 163 + 141) |
| Scenarios before / after C | 7 / 13 | plan-62-B § 3.4 (7 rows) + § 3.3 below (6 rows) |

### Verified properties

- **The exported configs are the ATmega328P's.** `usart0Config` (UCSR0A at
  0xC0, UDR0 at 0xC6, vectors at word addresses 0x24/0x26/0x28 = vectors
  18/19/20), `spiConfig` (SPCR 0x4C, vector 17), `twiConfig` (TWBR 0xB8,
  vector 24), `adcConfig` (ADMUX 0x7C, vector 21), `eepromConfig` (EECR
  0x3F, vector 22), `watchdogConfig` (WDTCSR 0x60, vector 6) against
  `planning/plan-62-data/atdf_cmp.txt` (the 328P's 26 interrupt names in
  index order and the register list).
- **The EEPROM times in `eepromConfig` assume 16 MHz** (`eraseCycles:
  28800, // 1.8ms at 16MHz`). They are fields of the config, so the chip
  sets them from its clock (§ 3.1).

## 3. Design Overview

Six ported files, their place in the layout, the scenario stimuli for the
callback side, six scenarios.

Correctness risk is lowest here of the three core letters: each class is
small and register-driven. The watchdog's reset path calls `cpu.reset()`
from inside a clock event; the C code must leave the event list in the
state avr8js leaves it (reset clears `nextClockEvent`, then `tick` assigns
`nextClockEvent = nextClockEvent.next` from the event that just ran).

### 3.1 The peripherals

Same shape as plan-62-B § 3.1: a struct and an init function per class,
callbacks as `{fn, ctx}`.

| avr8js | C |
|---|---|
| `new AVRUSART(cpu, config, freqHz)`; `onByteTransmit`, `onRxComplete`, `onConfigurationChange`; `writeByte`, `rxBusy`, `bitsPerChar`, `stopBits`, `parityEnabled`, `parityOdd`, `rxEnable`, `txEnable`; `reset` | `avr_usart_init`; `on_transmit`, `on_rx_complete`, `on_config`; `avr_usart_write_byte`, `avr_usart_rx_busy`, `avr_usart_bits`, `avr_usart_stop_bits`, `avr_usart_parity`, `avr_usart_rx_enabled`, `avr_usart_tx_enabled`, `avr_usart_bit_cycles` (new: `(UBRR + 1) × multiplier`, for F); `avr_usart_reset` |
| `new AVRSPI(cpu, config, freqHz)`; `onByte`, `completeTransfer`; `isMaster`, `dataOrder`, `spiMode`, `clockDivider`, `transferCycles`; `reset` | `avr_spi_init`; `on_byte`, `avr_spi_complete_transfer`; `avr_spi_is_master`, `avr_spi_lsb_first`, `avr_spi_mode`, `avr_spi_clock_divider`, `avr_spi_transfer_cycles`; `avr_spi_reset` |
| `new AVRTWI(cpu, config, freqHz)`; `eventHandler`; `completeStart/Stop/Connect/Write/Read`; `prescaler` | `avr_twi_init`; `AvrTwiHandler {start, stop, connect, write_byte, read_byte, ctx}`; `avr_twi_complete_*`; `avr_twi_bit_cycles` (new: `16 + 2 × TWBR × prescaler`, for F) |
| `new AVRADC(cpu, config)`; `channelValues`, `avcc`, `aref`, `onADCRead`, `completeADCRead` | `avr_adc_init`; `channel_volts[]`, `avcc`, `aref`, `on_read`, `avr_adc_complete_read` |
| `new AVREEPROM(cpu, backend, config)`, `EEPROMMemoryBackend` | `avr_eeprom_init` with the memory inside the struct (erased 0xFF), `avr_eeprom_data` for the test |
| `new AVRWatchdog(cpu, config, clock)` | `avr_watchdog_init` |

The line-buffering helper `onLineTransmit` is not ported (it is a
convenience for JavaScript callers).

`avr_chip_create` sets the EEPROM's `erase_cycles` and `write_cycles` to
`base_hz × 1.8 ms` (28800 at 16 MHz, avr8js's value); a scenario runs at
`base_hz` 16000000.

`avr_chip_reset` (B) now also resets the USART and SPI, and the watchdog's
reset calls `avr_chip_reset` through `cpu->on_reset {fn, ctx}` so every
peripheral restarts, where avr8js resets only the CPU. In exact mode
`on_reset` is not called, so the scenario matches avr8js.

### 3.2 Scenario stimuli for the callback side

`run` and the C runner give the same scripted answers:

- USART: every transmitted byte prints `u %x %d %02x` (cycles, usart
  index, byte). A stimulus `[cycle, "rx", usart, byte]` calls
  `writeByte(byte)` (not immediate).
- SPI: `on_byte(value)` prints `s %x %d %02x` and completes after
  `transferCycles` with `value ^ 0xA5`.
- TWI: the handler prints `t %x %d <event> %02x` and answers through a
  clock event 40 cycles later: `start`/`stop` complete; `connect` acks
  address 0x50 and nacks any other; `write_byte` acks; `read_byte` returns
  0x10, 0x11, … in turn.
- ADC: a stimulus `[cycle, "adc", channel, millivolts]` sets
  `channelValues[channel]`.

### 3.3 The scenarios

| Scenario | Exercises |
|---|---|
| `usart` | 9600 and 115200 baud settings, U2X, 5- to 9-bit frames, UDRE and TXC interrupts, RXC interrupt with `rx` stimuli, reading UDR, TXEN toggled |
| `spi` | each clock divider and SPI2X, a transfer by polling SPIF and by the interrupt, the write-collision flag, SPE off |
| `twi` | start, address + W with ack and with nack, data writes, repeated start, address + R, reads with ack then nack, stop; by polling and by the interrupt; each prescaler |
| `adc` | a single conversion per channel 0–7, the 1.1 V and 0 V inputs, ADLAR, each prescaler, the interrupt, references AVCC and internal |
| `eeprom` | a read of erased memory, write then read back, erase only, write only, the EEMPE timeout, the ready interrupt, a write while busy |
| `watchdog` | the change-enable sequence, `WDR` holding it off, the interrupt mode, the reset mode (the program restarts and reads WDRF) |

## Compatibility / Format Impact

None outside `backend/tests/avr/` and `backend/tools/`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — USART, SPI, TWI

- [ ] `backend/src/models/avr_usart.c`, `avr_spi.c`, `avr_twi.c` and their
      declarations in `avr.h`; `backend/CMakeLists.txt`.
- [ ] `avr_chip.c`: USART0, SPI and TWI in the `atmega328` layout and in
      `--layout`'s JSON; `oracle.js`: build them from the layout, the
      scripted answers of § 3.2, `layout-check` covers them.
- [ ] `avr_periph_test.c`: the same scripted answers and the `rx` stimulus.
- [ ] Scenarios `usart`, `spi`, `twi`.

Acceptance: 10 scenarios equal avr8js.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_periph_test --output-on-failure` → `layout atmega328 equal` and 10 `ok` lines (est. 1 min).
Commit: —

### Phase 2 — ADC, EEPROM, watchdog

- [ ] `backend/src/models/avr_adc.c`, `avr_eeprom.c`, `avr_watchdog.c`;
      `backend/CMakeLists.txt`.
- [ ] `avr_chip.c`: the three in the layout, the EEPROM times from
      `base_hz`, `on_reset`; `oracle.js` and `avr_periph_test.c`: the `adc`
      stimulus.
- [ ] Scenarios `adc`, `eeprom`, `watchdog`.
- [ ] A C-only case in `avr_periph_test`: with `exact` 0, a watchdog reset
      leaves the USART's UCSR0B and the timers' registers cleared
      (`avr_chip_reset` ran); prints `ok watchdog_full_reset`.
- [ ] `docs/backend.md`: § Layout, § Tests.

Acceptance: 13 scenarios equal avr8js; the full reset holds.
  Check: `ctest --test-dir /tmp/sim-rel -R 'avr_' --output-on-failure` → 13 `ok <scenario>` lines, `ok watchdog_full_reset`, 2 tests passed (est. 1 min).
Commit: —

## Validation Plan

- Tests: `avr_periph_test` (13 scenarios + the reset case).
- Coverage check: as plan-62-B; in addition the test requires at least one
  `u`, `s` and `t` line in the `usart`, `spi` and `twi` traces.
- Runtime proof: none in C (E, F).
- Doc sync: `docs/backend.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions. None added.

## Corrections

## Summary

Six small register-driven ports, each pinned by a scenario. The pins of the
three serial peripherals are F's.
