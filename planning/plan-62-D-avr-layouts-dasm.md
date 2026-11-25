# plan-62-D: AVR simulation — the three register layouts, vector size, the disassembler

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-C

After C one layout exists, `atmega328`, and it is avr8js's ATmega328P. D
makes the chip cover all 18 devices: the vector size follows the flash
size, two more layouts cover the PB devices, and a disassembler gives the
Processor window its "Last instruction" text.

Outcome: `avr_periph_test` prints 16 `ok` scenario lines and
`dasm: <N> words round-trip`; the layouts `atmega328`, `atmega168pb` and
`atmega328pb` each load.

References:

- plan-62-B § 3.2 (the chip), § 3.3 (scenario format); plan-62-C.
- `planning/plan-62-data/atdf_cmp.txt`, `atdf_signals.txt` (made by
  `atdf_cmp.py` and `atdf_signals.py` from
  `Microchip.ATmega_DFP.3.6.299.atpack`, Apache-2.0, not vendored).
- `datasheets/mcu/microchip/ATmega48P_88P_168P-DS40002065A.pdf` § 12
  (vector tables); `datasheets/mcu/microchip/40001907A.pdf` (ATmega328PB,
  pin table p. 13–15); `datasheets/mcu/microchip/ATmega48PB-88PB-168PB-DataSheet-DS40001909B.pdf`.

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-C is complete | `ls planning/completed/plan-62-C-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after 6ae1ebef) |

If plan-62-C is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `avr_chip_create` gives a chip of 8192 bytes of flash or less one-word
  interrupt vectors and a larger one two-word vectors.
- `avr_layout_find` knows `atmega328`, `atmega168pb` and `atmega328pb`.
- `avr_dasm` (`backend/src/models/avr_dasm.c`) turns one instruction into
  text, and every word it names assembles back to itself in avr8js's
  assembler.
- `avr_periph_test`: 3 more scenarios (`m48_vectors`, `m168pb_porte`,
  `m328pb_extras`), 16 in all, and the round-trip line.

### Non-goals (explicit constraints)

- The `atmega328` layout's tables do not change: `layout atmega328 equal`
  must still print.
- On the ATmega328PB these stay unmodeled: the peripheral touch controller
  (PTC), clock failure detection (CFD, `XFDCSR`), the output compare
  modulator, the USART start-frame detector (`UCSRnD`), and the external
  clock pins T3 and T4 (a timer 3 or 4 set to an external clock source
  stops, as avr8js stops a timer with divider 0). `ACSRB`, `DEVID0–8` and
  `PRR0/PRR1` are plain memory.
- No engine, kind or app change.

## 2. Current State

`AvrLayout` (plan-62-B § 3.2) holds avr8js-shaped configs whose interrupt
fields are word addresses for two-word vectors (`timer0Config`'s
`compAInterrupt: 0x1c` is vector 14).

### Measured populations

All from `planning/plan-62-data/atdf_cmp.txt` and `atdf_signals.txt`
(`python3 planning/plan-62-data/atdf_cmp.py <atdf dir>`, 2026-11-24).

| What | Value |
|---|---|
| Devices | 18: ATmega48, 48A, 48P, 48PA, 48PB, 88, 88A, 88P, 88PA, 88PB, 168, 168A, 168P, 168PA, 168PB, 328, 328P, 328PB |
| Devices whose registers and 26 vectors equal the 328P's | 14 (all but the four PB); the only difference printed is `EEARL`/`EEARH` named separately on the ATmega48 family |
| Flash / SRAM / EEPROM bytes | 48: 4096 / 512 / 256; 88: 8192 / 1024 / 512; 168: 16384 / 1024 / 512; 328: 32768 / 2048 / 1024 |
| 48PB, 88PB, 168PB additions | port E (`PINE` 0x2C, `DDRE` 0x2D, `PORTE` 0x2E, pins PE0–PE3), `ACSRB`, `UCSR0D`, `DEVID0–8`, vector 26 `USART_START`; ADC6 on PE2, ADC7 on PE3 |
| 328PB additions | the above plus vectors 27–44, `PCMSK3` (0x73), timers 3 and 4 (`TCCR3A` 0x90, `TCCR4A` 0xA0, `TIFR3` 0x38, `TIFR4` 0x39, `TIMSK3` 0x71, `TIMSK4` 0x72), USART1 (`UCSR1A` 0xC8 … `UDR1` 0xCE), SPI1 (`SPCR1` 0xAC, `SPSR1` 0xAD, `SPDR1` 0xAE), TWI1 (`TWBR1` 0xD8 … `TWAMR1` 0xDD) |
| 328PB vectors | 27 PCINT3, 28–30 USART1 RX/UDRE/TX, 32–35 TIMER3 CAPT/COMPA/COMPB/OVF, 39 SPI1 STC, 40 TWI1, 41–44 TIMER4 CAPT/COMPA/COMPB/OVF |
| 328PB pins | OC3A PD0, OC3B PD2, ICP3 PE2; OC4A PD1, OC4B PD2, ICP4 PE0; RXD1 PB4, TXD1 PB3; MISO1 PC0, SCK1 PC1, MOSI1 PE3, SS1 PE2; SDA1 PE0, SCL1 PE1; PCINT24–27 on PE0–PE3 |

### Verified properties

- **Vector size.** `ATmega48P_88P_168P-DS40002065A.pdf` Table 12-1
  (ATmega48P) and Table 12-2 (ATmega88P) list program addresses 0x000,
  0x001, 0x002 … (one word per vector); Table 12-4 (ATmega168P) lists
  0x0000, 0x0002, 0x0004 … (two words) (`pdftotext -layout`, lines 3457,
  3576, 3807). The 48 and 88 have 4096 and 8192 bytes of flash, the 168 and
  328 more.
- **avr8js's assembler covers the whole instruction set** and takes a
  relative jump's operand as a byte offset from the next instruction and an
  absolute jump's as a byte address (`assembler.ts`: `RJMP` and `BRBS` use
  `constOrLabel(…, byteLoc + 2)` then `k >> 1`; `JMP` uses `k >> 1` of the
  operand). Its `OPTABLE` has every mnemonic of the manual, including the
  aliases (`BREQ`, `SEC`, `LSL`, `TST`, `CLR`).
- **PE0–PE3 functions on the 328PB** (`40001907A.pdf`, pin table): PE0
  PCINT24/ICP4/SDA1, PE1 PCINT25/T4/SCL1, PE2 PCINT26/ADC6/ICP3/SS1, PE3
  PCINT27/ADC7/T3/MOSI1.

## 3. Design Overview

Three pieces: vector size, the two PB layouts, the disassembler.

Correctness risk concentrates in the 328PB layout: 4 more peripherals whose
tables come from the ATDF, not from avr8js, so `layout-check` cannot vouch
for them. The scenario `m328pb_extras` runs the same avr8js classes built
from our layout JSON, so a wrong address or vector shows as firmware that
gets no interrupt or reads a wrong register.

### 3.1 Vector size

`AvrLayout`'s interrupt fields become vector indices. `avr_chip_create`
multiplies each by `vector_words` (1 when `flash_bytes <= 8192`, else 2)
as it builds the peripherals. `--layout <name> <flash bytes>` prints the
JSON with the multiplied addresses, so `layout-check` on
`atmega328 32768` still compares against avr8js's configs.

### 3.2 The PB layouts

`atmega168pb` = `atmega328` plus port E (no external or pin-change
interrupt). `atmega328pb` = `atmega168pb` plus `PCINT3` on port E
(`PCICR`/`PCIFR` bit 3, `PCMSK3`, vector 27, mask 0x0F), timers 3 and 4
(16-bit, the `timer1` config with the addresses, vectors and pins of § 2,
dividers as timer 1 with entries 6 and 7 set to 0), USART1, SPI1, TWI1.
The JSON printed by `--layout` carries lists (`ports`, `timers`, `usarts`,
`spis`, `twis`), and `oracle.js run` builds one avr8js object per entry.

### 3.3 The disassembler

```c
/* Disassembles the instruction whose first word is `word` (and `next`, the
   following word) at byte address `address` into out. Returns its length in
   words (1 or 2). */
unsigned avr_dasm(uint16_t word, uint16_t next, uint32_t address, char *out, size_t cap);
```

Decoding follows `avr_instruction.c`'s branch order, so the two agree on
what a word is. Text: upper-case mnemonic, operands separated by `, `;
registers `R0`…`R31`; I/O addresses, constants and absolute targets as
`0x…`; a relative target as `.+N` or `.-N` (bytes from the next
instruction); pointer forms `X`, `X+`, `-X`, `Y+q`, `Z+q`. Aliases chosen:
the flag forms (`SEC`, `CLI`, …) for BSET/BCLR and the condition forms
(`BREQ`, `BRNE`, …) for BRBS/BRBC; never `LSL`, `ROL`, `TST`, `CLR`, `SER`,
`CBR` or `SBR`. A word avr8js treats as a no-op that is not `NOP` prints
`.DW 0x….`.

The round trip: `avr_periph_test --dasm` prints, for every word 0…65535
with next word 0x1234 at address 0x100, `%04x %04x <text>`;
`oracle.js dasm-check` reads those lines, turns `.+N`/`.-N` into the
assembler's offset, assembles each text that is not `.DW`, and compares the
words. It prints `dasm: <N> words round-trip` or the first failure. The
test runs it when `node` is found; the count N and an FNV-1a 64 hash of the
`--dasm` output go in `golden.txt` (`dasm <N> <hash>`) for a machine
without `node`.

### 3.4 The scenarios

| Scenario | Layout, sizes | Exercises |
|---|---|---|
| `m48_vectors` | `atmega328`, flash 4096, SRAM 512, EEPROM 256 | one-word vectors: `RJMP` at 0x001…0x019, INT0, timer 0 overflow and USART RX interrupts land on their handlers; the stack at 0x2FF |
| `m168pb_porte` | `atmega168pb`, 16384 / 1024 / 512 | DDRE/PORTE/PINE writes and the PINE toggle; PE pin stimuli read back |
| `m328pb_extras` | `atmega328pb`, 32768 / 2048 / 1024 | timer 3 and 4 in CTC and fast PWM with OC3A/OC4A, their interrupts; USART1 transmit and receive; SPI1 and TWI1 transfers; PCINT3 from PE0–PE3 stimuli |

## Compatibility / Format Impact

None outside `backend/tests/avr/` and `backend/tools/`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — vector size and the PB layouts

- [ ] `avr_chip.h` / `avr_chip.c`: interrupt fields as vector indices,
      `vector_words`, the lists of timers, USARTs, SPIs and TWIs, the
      layouts `atmega168pb` and `atmega328pb` from § 2's values.
- [ ] `avr_periph_test.c`: `--layout <name> <flash>`; `oracle.js`: lists in
      `run` and `layout-check`.
- [ ] Scenarios `m48_vectors`, `m168pb_porte`, `m328pb_extras`.

Acceptance: 16 scenarios equal avr8js; the 328 layout still equals
avr8js's configs.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_periph_test --output-on-failure` → `layout atmega328 equal`, 16 `ok` lines (est. 1 min).
Commit: —

### Phase 2 — the disassembler

- [ ] `backend/src/models/avr_dasm.c`, its declaration in `avr.h`,
      `backend/CMakeLists.txt`.
- [ ] `avr_periph_test.c`: `--dasm`, the hash check, the live round trip;
      `oracle.js`: `dasm-check`; `golden.txt`: the `dasm` line.
- [ ] `docs/backend.md`: § Layout, § Tests.

Acceptance: every named word round-trips.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_periph_test --output-on-failure` → `dasm: <N> words round-trip` with N above 60000 (est. 1 min).
Commit: —

## Validation Plan

- Tests: `avr_periph_test` (16 scenarios, layout check, round trip).
- Coverage check: `m328pb_extras`'s trace must hold a taken interrupt at
  each of the vectors 27, 28, 30, 33, 35, 39, 40, 42 and 44 (the test
  checks the `pc` values); the round-trip count excludes only `.DW` words.
- Runtime proof: none in D (E).
- Doc sync: `docs/backend.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions. None added.

## Corrections

## Summary

The 328PB's extra peripherals are the one place where the tables are not
avr8js's own; the scenario runs avr8js's classes on those tables and checks
each new vector is reached.
