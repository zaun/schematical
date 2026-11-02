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

- [x] `serial_ay1013.c`, `serial_i8251.c`, `serial_i82510.c`,
  `serial_acia6551.c`, `serial_hs82c52.c`, `serial_com78c80x.c` (with the
  shared helpers `serial_util.h`); `CMakeLists.txt`; `MODELS[]`,
  `DEVICE_KEYS[]` and the kind's part keys in `serial.c`;
  `docs/parts-format.md` § `serial` gains §3 (a `####` section per model).
  Check: `cmake -S backend -B /tmp/logic-p12h-clean && cmake --build
  /tmp/logic-p12h-clean -j8 2>&1 | grep -ci warning` → `0` (also clean with
  `-Wall -Wextra` for the six files).
- [x] Fixtures and cases H1–H8 with the terminal round trips. Fixtures:
  `backend/tests/support/serial_fixture_classic.h` (the 13 devices,
  Corrections). Check: `/tmp/logic-p12h-clean/serial_test` → `ok   H1
  AY-5-1013A: control latch, every TX format, DAV/RDAV, OR/FE/PE, RDE/SWE,
  reset, terminal`, `ok   H1 (COM2017, HD-6402): pin names, 5-bit 1.5
  stops, receive, reset keeps RD, terminal`, `ok   H2 COM1863/COM8018:
  HIACC 32x clocks, 5-bit 1.5 stops, RDA/ROR rule, terminal`, `ok   H3
  CDP1854A: mode 0 as H1; mode 1 registers, TR/RTS, INT sources, break,
  CTS, terminal`, `ok   H4 8251A: async 1x/16x/64x, every command bit,
  errors, break; sync hunt, SYNDET, fill, double sync, external sync;
  terminal`, `ok   H5 82510: every bank's registers, both BRGs and a timer,
  FIFOs, interrupt blocks and priority, hold state, control characters,
  break, stops, 9-bit, soft parity, handshake, loopbacks, echo, PMD, ack
  modes, power down, terminal`, `ok   H6 CDP65C51: baud table at 1.8432
  MHz, TDRE level IRQ, RX/errors, programmed reset, DCD/DSR, echo, CTS,
  break, terminal`, `ok   H7 82C52, HD-6406: baud select, USR, interrupts,
  errors, parity modes, MSR, echo/loop/break; ALE, CS1, DMA acks, status
  pins, SIE/SFD; terminal`, `ok   H8 COM78C804/802/78804: MR pointer,
  per-line rates and formats, FIFO/ORR/FER/PER/RERR, scanner and
  summaries, DSCHNG, break, echo and loopbacks, 2-line variant, terminal`;
  C's 12 cases and `C outcome` still `ok`; `PASS` (22 `ok` lines).
- [x] `circuit_check`: `circuit_serial_send` on an 8251 in synchronous mode
  → -1, `channel 0 of part N is not asynchronous` (plan-12-C Corrections: no
  C model can make this error). Check: `/tmp/logic-p12h-clean/circuit_check`
  → `PASS: 8251A rig starts`, `PASS: send to an 8251A in synchronous mode:
  channel 0 of part 0 is not asynchronous`, `PASS: after IR and an async
  mode the same 8251A takes the terminal`, `0 failure(s)`.

Acceptance: `serial_test` → H1–H8 and every earlier case `ok`; `ctest` 100%
(about 1 min). Observed: `ctest --test-dir /tmp/logic-p12h-clean` →
`100% tests passed, 0 tests failed out of 18`; `git diff --stat
backend/tests/golden` → empty.
Commit: 7602351 (ay1013, i8251, H1–H4), 47e1825 (acia6551, H6), eb30981
(hs82c52, H7), 224e6d7 (com78c80x, H8), 34139db (i82510, H5), c631192
(circuit_check; H1 for COM2017 / HD-6402), 796edb6 (docs), and the plan
update after them.

### Phase 2 — Parts

- [x] Stock re-check (`AY-`, `COM2017`, `COM8018`, `HD3-6402`, `CDP1854`,
  `CP1854`, `8251A`, `82510`, `CDP65C51`, `82C52`, `HD3-6406`, `COM78`: 12
  calls); drops in Corrections. Observed (`node scripts/digikey.js
  --in-stock --list`): COM2017 3, COM8018 2, HD3-6402 1, CDP1854 1, CP1854
  4, 8251A 16, 82510 163, CDP65C51 3, 82C52 7, HD3-6406 1, COM78 4. `AY-`
  returned 5346 unrelated parts without the AY UARTs, so two narrower calls
  were added: `AY-5-1013` 4 (AY-5-1013, AY-5-1013A, AY-5-1013A-HR,
  AY-5-1013A-HRS), `AY-3-1015` 1 (AY-3-1015D). All 29 H part names in
  `parts.tsv` are in these lists: no drops.
- [x] `footprints.json` + `CDIP-40-600`, `PLCC-28`; `serial.json` + 13
  devices. The 2 objects verbatim from `footprints.md`; the devices from
  `tests/fixtures/serial/devices/<ID>.json` (whole device objects, see
  Corrections): `merge.py --ship F,G,H` → `serial.json: 65 devices, 179
  parts`; `serial_test` `shipped` → `compared: 52 ns16550, 0 bridges, 13
  classic`.
- [x] Regenerate the goldens. Check: no removed line; added 29, 29, 2.
  Observed: removed 0 / added 29, 29, 2.
- [x] Hand-check the 2 footprint lines against `footprints.md`. PLCC-28:
  pads at ±5.723 (span 11.445), pitch 1.27, 0.530×2.000 on the top row
  (2.000×0.530 on the sides), pin 1 top centre (0, −5.723), body 11.51;
  CDIP-40-600: ±7.620 (15.24), first −24.130 (pitch 2.54), 1.5 round, drill
  0.8, pin 1 rect, body 14.35×53.24. Both as the JSON.
- [x] Spot-check: `AY-5-1013A` (`PDIP-40-600`), `IS82C52Z` (`PLCC-28`),
  `MD8251A/B` (`military`, `ceramic`, `CDIP-28-600`). Observed:
  `AY-5-1013A|serial|DIP|…|model=ay1013 channels=1 grade=A|…
  |footprint=PDIP-40-600|vcc=4750-5250`; `IS82C52Z|…|model=hs82c52
  channels=1 grade=I|…|footprint=PLCC-28`; `MD8251A/B|…|military,ceramic
  |model=i8251 …|footprint=CDIP-28-600`; also `AY-3-1015D … grade=D` and
  `CP1854ACD … vcc=4000-6500`.

Acceptance: `ctest` 100%; the golden checks (about 2 min). Observed:
`100% tests passed, 0 tests failed out of 19`; `serial_test` → `PASS`.
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

Phase 1 (models). Where §3 disagreed with the spec, the model follows the
spec and `decisions.md`:

- **`"fiveBitStops"` is not 1.5 for every device.** §3.1 said 1.5 for
  every `ay1013` device. `classic-a.md` § AY-5-1013A (device table, row
  "TSB=1 with 5-bit char") gives 2 stop bits on the AY-5-1013A (and
  AY-6-1013) and 1.5 on the AY-3-1014A/1015D; COM2017 1.5 (com2017.pdf p5),
  COM1863/COM8018 1.5 (planner decision), HD-6402 1.5 (p3 table), CDP1854A
  1.5 (p13, Table IV). The AY-5-1013A device carries `2`.
- **AY-3-1015D is a variant on the AY-5-1013A device.** The same spec
  table: pin 2 NC (no VGG), 1.5 stops, XR clears the receive holding
  register ("resets every register except control register", p12 Fig.35),
  DAV one RCP after the transfer (Fig.32). The model takes part keys
  `"fiveBitStops"` and `"resetClearsData"` over the device's, and the
  fixture adds pinout `dip40n` (pin 2 `NC`) and grade `D`. Phase 2: the
  AY-3-1015D row should use `dip40n` (parts.tsv says `dip40`), grade `D`,
  `"fiveBitStops": 1.5, "resetClearsData": true`.
- **`"resetClearsControl"` added** (not in §3.1): the CDP1854A's MR (mode
  0, p12) and CLEAR (mode 1, p4) also reset the control register; the AY
  family's XR does not.
- **`"cpuMode"` keys.** §3.1 named only `"modePin"`. The model takes
  `modePin, select1, select2, select3, registerSelect, readWrite, tpb, cts,
  psi, es`; INT, FE, PE/OE, DA, THRE and RTS are the `"status"` pins (the
  same package pins as mode 0's PE, FE, OE, DA, THRE, TSRE), the reset pin
  is CLEAR (active low) in mode 1, and the R BUS / T BUS are the receive /
  transmit data pins. Pin names give both functions (`RRD/CS2`,
  `PE/INT`, `OE/PE_OE`, `SFD/RSEL`, `DAR/TPB`, `MR/CLEAR`, `THRL/CS1`,
  `TSRE/RTS`, `CRL/RD_WR`, `PI/CS3`, `WLS2/PSI`, `WLS1/ES`, `EPE/CTS`):
  the part has no Intel mode to take names from (plan-12-A §3.4).
- **CDP1854A mode 1 after CLEAR: THRE = TSRE = 1.** p4 says CLEAR resets
  the status register; the transmitter is empty, and Table IV's "initial
  THRE interrupt" when TR is set needs THRE = 1 (P3 reading).
- **`ay1013` clock edges.** decisions.md (AY family): TCP falling edge,
  start-bit centre at clock 8. The model counts RCP falling edges too
  (classic-a p4: a start while the clock is high is timed from its fall),
  and uses count 8 for the HD-6402 and CDP1854A, whose "7½" is the same
  half-clock ambiguity. The COM1863's transfer half an RCP before the
  stop-bit centre (p5) is made at the centre; DAV still waits for RDAR.
- **COM1863 receive rules are tied to `"hiacc"`.** The RDAR rule for ROR
  ("RDAR must have gone low and come high again", p4), RDA waiting for RDAR
  high (p7) and the HIACC notch apply to the device whose `"clocks"` names
  `"hiacc"` (COM1863/COM8018 only).
- **8251A terminal.** §3.2: "`serial_send` returns 0 in sync mode". It also
  returns 0 before a mode is programmed (after a reset or IR the format is
  not known): `async` is 1 only in a programmed asynchronous mode.
- **8251A synchronous receiver** (spec silent, P11 readings): no character
  boundary exists until EH (or, with ESD, the SYNDET input); after the
  hunt every character, sync characters included, is loaded as data; a
  double-sync hunt that fails on SYNC2 resumes bit by bit with that window.
  The CLK pin only bounds delays (tCY multiples, P5); it is not counted.
- **CDP65C51 `"ctsA"` is a part key**, not a grade key (§3.4 said
  grade-level): grades hold only timing names, `vcc`, `inputMax` and
  `note` (`serial.c` load_grade). No in-stock part is a CDP65C51A
  (parts.tsv leaves CDP65C51ASM2-96 out), so no shipped part sets it. The
  CDP65C51's RDRF with 1.5 stop bits is set at the stop's middle, not half
  way through the trailing half stop (p12 Fig 17).
- **82C52 / HD-6406 interrupt edges.** The spec's formula `posedge(DR |
  TBRE)` (classic-b § HD-6406, Fig 9) can never fire on DR while TBRE is
  high (both are levels). The model detects each bit's rising edge (DR, or
  TBRE; and each USR group bit), so SIE gives an interrupt per received
  character. MS (USR bit 4) is set on any modem change but, per Fig 9, is
  not itself an INTR source. Back to back, TBRE rises as the TR takes the
  next character (end of the last stop) rather than at its 15th CO (p14
  note 4); idle, TBRE rises at the 4th CO and the start bit at the 5th, as
  printed. The HD-6406 divisor conflicts resolve to the 82C52 values
  (decisions.md), so one table serves both and no key separates them.
- **No `"nmos"` key** (§3.6). `classic-b.md` § COM78804: register set,
  address map, modes, scanner and summaries "identical to COM78C804"; the
  differences are DC and tDSU (0 ns, a setup the model does not check),
  so COM78804 differs only in its grade. No reset difference is printed.
- **COM78C80x scanner outputs.** IRQLN / IRQTXRX and Interrupt Summary bits
  3–0 are "valid only while IRQ is asserted" / "a free-running counter"
  (p8–9): the model drives / reads them X while no interrupt is held (P2).
  The scanner steps one position per CLK (the rate is not printed).
- **82510 readings** (spec silent, P3 / P4 / P11): the TX and RX machines
  are enabled at wake-up (8250 compatibility, p6); TMCR resets to gates open
  (TMST 30H); in local loopback the modem output pins are inactive as on the
  PC16550D (the spec names only OUT0); DRI is set by the RI pin going high
  to low, as printed; RXF bit 7 reads 0; a break's null character takes part
  in control-character and ACR matching (p41's RST 5CH during a break shows
  PCRF).
- **Fixtures** (plan-12-D §3.3). The library rejects a device id that is
  loaded twice, so the fixture ids are `SFX-` + the device id (the shipped
  objects keep the plain id); parts are `SFX-<id>-<pinout>`. Footprints not
  in `footprints.json` yet use one of the same pin count: `PLCC-28`
  (82510, 82C52 plcc28) → `LCCC-28`; `CDIP-40-600` (CDP1854A) →
  `PDIP-40-600`; `PDIP-48-600` (COM78804, which plan-12-F adds) →
  `SSOP-48`. The worktree started at cf44937; it was fast-forwarded to
  627f50d (plan-12-C) before the work.
- **Merge with D/J: the bus select callback returns a unit mask.** H was
  written against C's `serial_bus.h`, where the select callback set one unit
  index. D/J changed it to a bitmask (`uint32_t *units`, a bit per selected
  unit) before H merged; `unsigned *` and `uint32_t *` are the same type, so
  the merge compiled and only H7 failed (65 checks: writes to register 2 went
  to register 1). `serial_hs82c52.c`'s `bus_select` now returns `1u << unit`;
  after the fix `serial_test` → `PASS`, 67 `ok`; `ctest` 19/19.
- **Phase 2 data source.** The device objects ship from
  `tests/fixtures/serial/devices/<ID>.json`, each the fixture device of
  `serial_fixture_classic.h` (id without `SFX-`) with its real parts from
  `parts.tsv`; `merge.py --ship` takes a whole device file when there is no
  `keys/` file, and `shipped` compares the header's devices with
  `serial.json` (6d4929b). Grades by part name: CDP65C51M1 → 1, M4 → 4;
  82C52 `CS` → C, `I…` → I; AY-3-1015D → D on `dip40n` with the part keys
  above; every other device has one grade. CP1854ACD (CDP1854AC, VDD
  4–6.5 V, classic-a.md) carries its own `vcc`, the grade being 4–10.5 V.
