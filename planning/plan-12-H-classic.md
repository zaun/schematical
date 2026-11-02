# plan-12-H: Classic UARTs — `ay1013`, `i8251`, `i82510`, `acia6551`, `hs82c52`, `com78c80x`

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-G

H adds the six models for the in-stock UARTs whose register model is not the
16550's, and their 29 parts with the 2 new footprints they first use.

| Model | Devices | Parts |
|---|---|---|
| `ay1013` | AY-5-1013A (AY-5-1013A, AY-5-1013A-HR, AY-3-1015D), COM2017 (COM2017P), COM1863 (COM8018P), HD-6402 (HD3-6402C-9), CDP1854A (CDP1854ADS2082, CP1854ACD, CP1854AE) | 9 |
| `i8251` | 8251A (D8251A, LD8251A, MD8251A/B, N8251A, P8251A, P8251A-G) | 6 |
| `i82510` | 82510 (N82510, TN82510) | 2 |
| `acia6551` | CDP65C51 (CDP65C51M1, CDP65C51M4S2357) | 2 |
| `hs82c52` | 82C52 (6 parts), HD-6406 (HD3-6406-5) | 7 |
| `com78c80x` | COM78C804 (COM78C804LJP), COM78C802 (COM78C802P), COM78804 (COM78804P) | 3 |

Outcome of H: `serial_test` cases H1–H8 pass on fixtures that are the
shipped devices; `serial.json` gains the 13 devices; `parts.txt` and
`traces.txt` gain 29 lines, `footprints.txt` 2 (`CDIP-40-600`, `PLCC-28`),
only added lines.

References: `plan-12-data/specs/classic-a.md` (AY-5-1013A, COM2017, COM1863,
HD-6402, CDP1854A), `classic-b.md` (8251A, 82510, CDP65C51, 82C52, HD-6406,
COM78C804/802, COM78804); `matrix.md` Table B; `decisions.md` (these
devices; planner decisions for COM1863/COM8018 and CDP65C51);
`footprints.md` § CDIP-40-600, § PLCC-28; plan-12-C §3.2–§3.4 (line engine,
terminal hooks).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-G landed | `grep -c '\|serial\|' backend/tests/golden/parts.txt` → 150 (or F + G's recorded counts) | NOT MET (2026-11-02) |

## 1. Goal

- `serial_test` cases H1–H8 `ok`; `shipped` `ok` (now covering H's devices).
- Goldens: + 29 `parts.txt`, + 29 `traces.txt`, + 2 `footprints.txt`; no
  removed line.
- `ctest` 100%, T0 + 1 programs.

### Non-goals

- No `ns16550` change. No app change.

## 2. Current State

After G: `serial.json` has the 52 parallel 16550-family devices; the line
engine (C) supports 5–8 data bits, parity, 1/1.5/2 stops, break, 16× clocks.

## 3. Design

Each model is a file under `backend/src/models/` registered in the serial
model table (plan-12-C §3.1), using the line engine for framing and the
terminal hooks for its channels. Keys name pins; behavior is the spec
section's.

### 3.1 `ay1013` (`serial_ay1013.c`) — pin-parallel UART

No register bus: transmit data, receive data, control and status are pins.

| Key | Value |
|---|---|
| `"transmitData"` | 8 names (TD1…TD8 / DB1…DB8; LSB first) |
| `"receiveData"` | 8 names (RD1…RD8 / RR1…RR8) |
| `"receiveEnable"` | RDE (low: RD pins driven, else Z) |
| `"statusEnable"` | SWE (low: status pins driven, else Z) |
| `"status"` | `{"dav", "tbmt", "eoc", "or", "fe", "pe"}` names |
| `"control"` | `{"strobe": CS, "noParity": NP, "stops": TSB, "wordLength": [NB1, NB2], "evenParity": EPS}` — loaded while the strobe is high (a level latch) |
| `"dataStrobe"` | DS (low loads TD into the transmit holding register) |
| `"resetDav"` | RDAV (low clears DAV) |
| `"serialOut"`, `"serialIn"` | SO, SI |
| `"clocks"` | `{"transmit": TCP, "receive": RCP}` (16× clocks; `"factor"`: 16, or 32 when `"hiacc"` names the HIACC pin and it is high: COM1863/COM8018) |
| `"reset"` | XR (high resets) |
| `"resetClearsData"` | bool (per device, `classic-a.md`) |
| `"fiveBitStops"` | `1.5` (TSB = 1 with 5 bits: COM1863/COM8018 planner decision; every `ay1013` device) |
| `"cpuMode"` | CDP1854A only: `{"modePin": MODE, …}` — mode 1 (MODE high) is the CDP1800 bus interface (CS1/CS2/CS3, RSEL, RD/WR, TPB, INT, status/control registers) per `classic-a.md` § CDP1854A; mode 0 is the pin-parallel behavior above |

Terminal: one channel; its format comes from the latched control word, its
clock from RCP.

### 3.2 `i8251` (`serial_i8251.c`)

8251A USART, `classic-b.md` § 8251A: C/D, RD, WR, CS, RESET, CLK, D0–D7,
TxD, RxD, TxC, RxC, TxRDY, TxEMPTY, RxRDY, SYNDET/BD, DSR, DTR, CTS, RTS.
The mode instruction after reset (async: baud factor ×1/×16/×64, length,
parity, stops; sync: sync character count, internal/external SYNDET), sync
characters, then command instructions (TxEN, DTR, RxE, SBRK, ER, RTS, IR,
EH). Asynchronous framing uses the line engine with TxC/RxC as the clocks
(the factor divides them). Synchronous mode: hunt for the sync character(s),
SYNDET, sync-character fill when the transmitter underruns, per the spec.
Terminal: `serial_send` returns 0 (not asynchronous) in sync mode.

### 3.3 `i82510` (`serial_i82510.c`)

82510, `classic-b.md` § 82510: four register banks selected by the bank
pointer, two baud generators/timers, 4-byte FIFOs, the modem and general
purpose pins, all bits per the spec.

### 3.4 `acia6551` (`serial_acia6551.c`)

CDP65C51, `classic-b.md` § CDP65C51: CS0 (high), CS1 (low), RS0/RS1, R/W,
PHI2 (the bus is clocked by PHI2: a selected cycle reads or writes while PHI2
is high), IRQ (open-drain), XTAL1 (clock input; owner decision), RxC,
DCD, DSR, CTS, RTS, DTR, TxD, RxD. Registers: transmit/receive data, status
(read; a write is a programmed reset), command, control; the internal baud
rate table (16 rates from 1.8432 MHz) or the external ×16 clock; TDRE
interrupt level-triggered (planner decision); CDP65C51A's CTS handling per the
spec (a grade-level key `"ctsA": true`).

### 3.5 `hs82c52` (`serial_hs82c52.c`)

82C52 and HD-6406 (`classic-b.md` §§): the 82C52 register core (RBR/TBR,
UCR, BRSR, MCR, USR) with its baud rate select table (`decisions.md` for the
HD-6406 divisor conflict) and pins; HD-6406's extra DMA, address-latch and
status pins as `"hd6406": {…}` names.

### 3.6 `com78c80x` (`serial_com78c80x.c`)

COM78C804 (4 lines), COM78C802 (2 lines), COM78804 (NMOS, 4 lines):
`classic-b.md` § COM78C804 and the two differences sections; SMC's own
register layout, the scanner, per-line baud selection. Key `"lines"`: 2 or
4; `"nmos"`: true for COM78804's timing and reset differences.

## 4. Tests (added to `serial_test`)

Fixtures are the shipped device objects (plan-12-D §3.3), one per device.

| Case | Fixture | Checks |
|---|---|---|
| H1 | AY-5-1013A | control-word latch, TX frames for each NB/NP/EPS/TSB combination, DAV/RDAV, OR/FE/PE, RDE/SWE 3-state, reset |
| H2 | COM1863 | HIACC 32× clock; 5-bit 1.5 stop |
| H3 | CDP1854A | mode 0 as H1; mode 1 bus registers and INT |
| H4 | 8251A | async mode ×16 and ×64, every command bit; sync mode hunt and SYNDET, fill characters; terminal refused in sync mode |
| H5 | 82510 | every bank, both baud generators, FIFO, interrupts |
| H6 | CDP65C51 | internal baud table at 1.8432 MHz, TDRE level IRQ, programmed reset, echo mode |
| H7 | 82C52, HD-6406 | baud select, USR, interrupts; HD-6406 DMA/status pins |
| H8 | COM78C804, COM78C802, COM78804 | per-line registers and scanning; 2-line variant |

Also, for each model, a terminal round trip: `circuit_serial_send` of `OK`
appears in the receive data (pins or register), and a transmitted `A` appears
in `circuit_serial_sent`.

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Models

- [ ] `serial_ay1013.c`, `serial_i8251.c`, `serial_i82510.c`,
  `serial_acia6551.c`, `serial_hs82c52.c`, `serial_com78c80x.c`;
  `CMakeLists.txt`; `docs/parts-format.md` § `serial` gains §3.
- [ ] Fixtures and cases H1–H8 with the terminal round trips.

Acceptance: `serial_test` → H1–H8 and every earlier case `ok`; `ctest` 100%
(about 1 min).
Commit: —

### Phase 2 — Parts

- [ ] Stock re-check (`AY-`, `COM2017`, `COM8018`, `HD3-6402`, `CDP1854`,
  `CP1854`, `8251A`, `82510`, `CDP65C51`, `82C52`, `HD3-6406`, `COM78`: 12
  calls); drops in Corrections.
- [ ] `footprints.json` + `CDIP-40-600`, `PLCC-28`; `serial.json` + 13
  devices.
- [ ] Regenerate the goldens. Check: no removed line; added 29, 29, 2.
- [ ] Hand-check the 2 footprint lines against `footprints.md`.
- [ ] Spot-check: `AY-5-1013A` (`PDIP-40-600`), `IS82C52Z` (`PLCC-28`),
  `MD8251A/B` (`military`, `ceramic`, `CDIP-28-600`).

Acceptance: `ctest` 100%; the golden checks (about 2 min).
Commit: —

## Validation Plan

`serial_test` H1–H8 (models) and the golden programs (parts) are the gate.
The landing tier is plan-12-K's.

## Decisions

- The 8251A's synchronous mode is modeled (owner: full models); the terminal
  works in its asynchronous mode only.
- CDP1854A's two modes are one model (`ay1013` with `cpuMode`): mode 0 is the
  AY-5-1013 machine.

## Corrections

(none yet)
