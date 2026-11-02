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

- [x] I2C target, SPI target, pin-selected front end (§3.1); open-drain pins.
  The front ends are J's (`serial_target.c`, Corrections); I added the NXP
  sub-address / SPI command layouts and the `ns16550` wiring
  (`ns16550_bridge.c`), and `ns16550`'s `pin_open_drain` (IRQ, SDA, SCL).
  `serial_test | grep '^ok   I[1-7]'` → 7 lines.
- [x] Bridge options (§3.2); fixtures and cases I1–I7;
  `docs/parts-format.md` § `serial` gains the host styles.
  `cmake -S backend -B /tmp/logic-p12i && cmake --build /tmp/logic-p12i -j8
  && ctest --test-dir /tmp/logic-p12i` → "100% tests passed, 0 tests failed
  out of 19", 0 warnings (clean build); `serial_test | grep -c '^ok'` → 67,
  "PASS".

Acceptance: I1–I7 and every earlier case `ok`; `ctest` 100% (about 1 min).
Commit: 96425c7 (models), 0986197 (fixtures), 57c9163 (cases), 256135a (docs)

### Phase 2 — SC18IS604 and parts

- [x] `serial_sc18is604.c` (§3.3); cases I8, I9.
  `serial_test | grep '^ok   I[89]'` → "ok   I8 SC18IS604: commands
  0x00-0x03, …" and "ok   I9 SC18IS604 -> SC16IS750: …"; ctest 100%.
- [x] Stock re-check (`SC16IS7`, `SC18IS604`, `PI7C9X76`, `XR20M1`: 4 calls);
  drops in Corrections.
  `source ~/.zshrc; node scripts/digikey.js --in-stock --list <family>`
  (2026-11-01) → SC16IS7: 17 of 17 listed, all 15 `add` parts present;
  SC18IS604: SC18IS604PWJ; PI7C9X76: PI7C9X760CZDEX, PI7C9X762Q2ZHEX;
  XR20M1: XR20M1170IG16-F, IG24-F, IL24-F, IL24TR-F, XR20M1280IL24TR-F,
  IL32-F. All 22 `add` parts in stock: none dropped. Re-checked on
  2026-11-02 before shipping: SC16IS7 17, SC18IS604 1, PI7C9X76 2, XR20M1
  15; the same 22 names.
- [ ] `footprints.json` + 4; `serial.json` + 8 devices; regenerate the
  goldens. Check: no removed line; added 22, 22, 4; hand-check the 4
  footprint lines. (Parent's step. The 4 footprints are in the fixture
  `backend/tests/fixtures/serial/footprints.json`; `merge.py --ship …,I`
  writes the 8 devices.)

Acceptance: `serial_test` I1–I9 `ok`; `ctest` 100%; the golden checks (about
2 min).
Commit: 96425c7 (sc18is604), 57c9163 (I8, I9); parts, footprints, goldens: —

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
- **Ran before its prerequisite.** plan-12-H has not landed
  (`serial_test | grep -c '^ok H'` → 0); I was done on the parent
  session's instruction on 4fd2c85 (plan-12-F). The worktree branch was at
  cd892a7 (an ancestor of 4fd2c85); it was fast-forwarded to 4fd2c85 before
  any work (`git merge --ff-only`, no branch created or switched).
- **Bridge keys added** (all in `docs/parts-format.md` § `ns16550`, "I2C/SPI
  bridges"): in the host `"bus"`: `"reset"`, `"resetActiveHigh"`,
  `"registerBits"` (4 or 3), `"nack"` (`"full"`, `"reset"`) and
  `"sdaTimeout"`; device keys `"interrupt"` (the open-drain IRQ), `"gpio"`
  (`"pins"`, `"registers"`: `"sc16is"`/`"xr1280"`, `"pullUp"`,
  `"stateReset"`), `"fastIrda"` (true or grade names), `"dlPowerUp"`,
  `"straps"` (`"en485"`, `"enir"`), `"sleepPin"`; `"extended"` values
  `"sc16is"`, `"sc16is-sfr"`, `"sc16is-dld"`, `"xr1280"`; timing names
  `tdo`, `ttr`, `tvd`, `tgpio`. §3.2's `"efcr"` key is not a key: every
  bridge has EFCR (the `sc16is*` styles; SFR on the XR20M1280).
  sc18is604: `"bus"` (spi mode 3), `"i2c"`, `"reset"`, `"interrupt"`,
  `"gpio"`; timing `tvq`, `ttr`, `tstart` (35 us message to START), `tsa`
  (125 ns RESET acceptance).
- **Fixture file.** The bridges go to `bridges.json` (merge.py), not
  `ns16550.json` (byte-identical after the change), with one fixture part
  per pinout and grade (`SFX-SC16IS750-hvqfn24-760` …); `case_shipped`
  compares both files with `serial.json`.
- **Grades.** SC16IS740 `740`; SC16IS750 `750` and `760`; SC16IS752 `752`
  and `762` (the 760/762 are speed grades of one device, bridges.md
  Variants); the others `I`. One grade serves both host modes: the SPI
  table's td9–td14 (200 ns) time the IRQ/GPIO/DTR changes, I2C tVD;DAT
  (0.6 us) is `tvd`; tsim is I2C td2 (0.2 us). SC16IS752 uses the SC16IS750
  4 MHz SPI table (decisions.md).
- **Fast IrDA.** `"fastIrda": ["760"]` / `["762"]`: on the SC16IS750/752
  grades EFCR[7] is stored and the pulse stays 3/16 ("fast mode not
  available", sc16is750 p21 note 9: the feature is absent, not undefined).
- **SC16IS map, LCR = 0xBF, addresses 0/1:** X. The special set is "LCR[7] = 1
  and LCR ≠ 0xBF" (bridges.md Registers), so DLL/DLH are not there at 0xBF
  (P1, P2); 8–15 at 0xBF also X (PI7C9X's SFREN/SFR set aside).
- **TX trigger without EFR[4].** In FIFO mode the THR interrupt follows the
  TX trigger (FCR[5:4] reset 00 = 8 spaces) whether or not EFR[4] = 1;
  EFR[4] only gates writing FCR[5:4] (sc16is750 p21–22, IIR Table 21
  "spaces ≥ trigger"). XR20M1280 table A's TX trigger "1 only" is the
  16C550's THR-empty interrupt.
- **IER[1] re-enable:** no new THR interrupt on `sc16is`/`sc16is-sfr`
  (sc16is750 p27 note 1; matrix A4 lists the XR20M1170's re-enable
  interrupt as its oddity); `sc16is-dld` and `xr1280` raise it.
- **Loopback RI/CD** come from MCR[2]/MCR[3] on every bridge (P4; the
  PI7C9X and XR20M1170 print it; the SC16IS750 is silent).
- **GPIO judgment calls.** IOLATCH = 1 latches a change only on inputs with
  IOIntEna set (the latch is the interrupt mechanism); IOState of a bit with
  no pin in the package reads the latch if it is an output, else X; the
  GPIO pull-up (sc16is750 p10 note 2) also applies to the pins' modem
  function (an open DSR reads inactive); PI7C9X IOState 0xFF is the output
  latch's reset value (its inputs have no pull-up and read X); a software
  reset written X makes the chip X until a clean reset (P2). SC16IS740:
  IODir/IOState/IOIntEna/IOControl read X (planner decision) but an
  IOControl write still has SRESET (p15 notes 2-3: "IOControl only has the
  software-reset bit"). The I2C NACK on the SRESET byte is SC16IS only (the
  PI7C9X and XR datasheets do not state it).
- **9-bit auto address** keeps its receiver on/off state inside the chip
  (EFCR[1] reads back as written); it starts off when EFCR[0] turns on.
  Normal multidrop keeps address bytes while EFCR[1] = 1.
- **RS-485 RTSCON** stays asserted while flow characters wait too.
- **PI7C9X SFR bank readings** (TIDLE stored only: planner): ASR [5:4] = 01
  after an Xoff, 10 while we owe an Xon, 11 after an Xon (new channel field
  `xon_seen`), [1] Xoff sent or auto-RTS off, [0] stopped by Xoff; ISCR
  reads the pending-and-enabled sources, bit 0 clears the latched ones
  (THR, timeout, Xoff/special, CTS/RTS, modem deltas, OE); TRCTL[2] gates
  the THR interrupt, TRCTL[1] the RX timeout, TRCTL[0]/[3] stored; SFR[0]
  transmits regardless of flow control, SFR[1] drives DTR by the auto-RTS
  rule and holds TX while DSR is inactive, SFR[3] echoes the RX pin on TX,
  SFR[7] (crystal feedback) has no digital effect; CPR M = 0 (a ½
  prescaler) stops the generator (P2). SFR[4] bursts auto-increment
  (decisions.md).
- **XR20M1170** in auto RS-485 with the FIFOs off: the THR interrupt waits
  for the TSR to empty (bridges.md IRQ#).
- **XR20M1280 readings:** Xoff goes out 2 character times (RX ticks) after
  the trigger is reached, Xon at one level below; auto-RTS table D uses
  TRG ± the SHR hysteresis; RS-485 RTS# drops (1 + SHR[3:0]) bit times
  after the last stop bit and leads by SHR[7:4]; EN485#/ENIR# are sampled
  at start and as each reset ends; LSR[4] stays set while RX is held low
  after a break; EMSR[2] puts the next THR byte at the FIFO head; TRG 0
  acts as 1; ISR[5:4] keep working with EFR[4] = 0 (the "650" legend:
  EFR[4] latches the gated bits); the SLEEP pin holds the channel asleep
  until released; the FCR[3] wake-up interrupt is set as the channel wakes
  and cleared by an ISR read. DREV reads X; SLEEP high = sleep
  (decisions.md).
- **SC18IS604 readings:** MISO Z while CS is high, during the command byte
  and where no output is defined (as J's front ends); an I2C command while
  busy is ignored (P2); power-down only when idle; an X on SDA at an ACK or
  under a sent 1 ends the transfer with I2CStat 0xFX (low bits X) and INT;
  an unknown command that could be 0x20 makes the registers X; RESET pulses
  shorter than `tsa` are filtered (20–125 ns is unspecified: filtered);
  the buffer powers on X; bus free = no START since the last STOP and both
  lines high; the timeout runs from the START attempt and covers
  arbitration retries; SCL 50 % duty (not stated).
- **Cases.** I1–I7 use the bench weak pull-up on SCL, SDA and IRQ (§4
  allowed it); I8 and I9 use `PULLUP-3V3` parts. I9 sets I2CClock 7 (268
  kHz) to shorten the run. RFD in I5 counts the PC16550D resync byte after
  the framing error (3 bytes, RLS 1).
- **Merged into `worktree-P-12` after H.** The I branch was based on F
  (4fd2c85); H's classic models and the reset fix (plan-12-K) landed
  first. The conflicts were all both-sides additions (model table, key
  list, CMake sources, the case list). `shipped` now compares four fixture
  sets in one case (`ns16550.json`, `bridges.json`, the classic header, the
  Maxim header). After the merge: `serial_test` → `PASS`, 76 `ok` (67 + I1–I9);
  `ctest` 19/19; `merge.py` regenerates `ns16550.json` and `bridges.json`
  unchanged.
