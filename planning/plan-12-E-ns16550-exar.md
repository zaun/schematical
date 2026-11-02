# plan-12-E: `ns16550` — Exar extensions, Motorola bus, the XR16V798/XR16L784 flat map

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-D

E adds the options the MaxLinear/Exar (ST16C, XR16C, XR16L, XR16M, XR16V)
devices need: Exar's extended register styles, the FCTR/EMSR trigger tables,
the fractional divisor and 8×/4× sampling, Exar Xon/Xoff, power-save and
sleep variants, IrDA and RS-485 variants, multi-function pins, device-ID
registers and FIFO counters, the Motorola host bus (XR16L2551/M2551/M752's
16/68 select), and the "5G" flat register map with its global block
(XR16V798, XR16L784).

The E-model devices (28; their parts land in G): ST16C450, ST16C1450,
ST16C550, ST16C1550, ST16C2550, ST16C554, XR16C2550, XR16L580, XR16L2551,
XR16L2552, XR16M2550, XR16M2551, XR16M2650, XR16M2651, XR16M752, XR16V2550,
XR16V2551, XR16V2552, XR16V2650, XR16V2651, XR16V2652, XR16V554, XR16V564,
XR16V654, XR16C854, XR16C864, XR16L784, XR16V798.

Outcome of E: `serial_test` has a fixture device for each of the 28 devices
(the device JSON G ships, plan-12-D §3.3) and one case per option value of
§3.1; all pass, with C's and D's cases.

References: plan-12-C §3.6, plan-12-D §3.1, §3.3; `plan-12-data/matrix.md`
Table A (the 28 rows), Table B (§ XR16V798/XR16L784 flat map), §3;
`plan-12-data/specs/exar-a.md`, `exar-b.md`, `exar-c.md`, `exar-d.md`;
`plan-12-data/decisions.md` (the 28 devices' sections and the planner
decisions for XR16L2552, XR16M752, XR16L784, ST16C1450).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-D landed | `/tmp/logic-p12/serial_test \| grep -c '^ok D'` → 24 | MET (2026-11-02 at 24a0932: `grep -c '^ok   D'` → 24) |

## 1. Goal

- `serial_test` prints `ok` for every case of §4 (`E<n> <device> <option>`)
  and still for every C and D case; `ctest` → 100%, T0 + 1 programs.
- `git diff --stat backend/tests/golden` → no change.

### Non-goals

- No change to C or D behavior when no E key is present.
- No part file (G). No I2C/SPI host (I).

## 2. Current State

After D: multi-channel `ns16550` with the TI/NXP/NS banks, flow control,
sleep, IrDA, RS-485 (TI style), DMA classes and the printer port.

## 3. Design

### 3.1 Keys E adds (absent = C/D behavior)

| Key | Values (devices) |
|---|---|
| `"extended"` | adds `"ier5"` (ST16C1450/1550: IER[5]/MCR[7] power-down, their THR-interrupt clear rule), `"exar650"` (LCR = 0xBF bank: EFR, Xon/Xoff, with Exar's DLD/MCR[7] prescaler), `"exar854"` (FCTR, EMSR, TRG, FC/FLVL, trigger tables A–D: XR16C854/864, XR16L2551/2552, XR16M25xx/26xx, XR16V25xx/26xx, XR16V554/564/654), `"m752"` (XR16M752: TCR, TLR, FIFO Rdy, DLD), `"5g"` (§3.2) |
| `"triggerTables"` | the A–D tables for RX and TX per device (`matrix.md` A1), selected by FCTR[5:4]; `"trg"`: the TRG programmable level |
| `"fifoCounters"` | `"fcFlvl"`, `"txlvlRxlvl"`, `"rfdTfd"`, absent |
| `"idRegs"` | `{"dvid": byte, "drev": byte}` read with DLL = DLM = 0 and LCR[7] = 1 (values in `matrix.md` A4) |
| `"dld"` | true: DLD register — fractional divisor n/16, 8×/4× sampling (DLD[5:4]), and on XR16M752 IrDA/RS-485 bits (DLD[7]/[6]) |
| `"sampling"` | `"emsr7"` (8× by EMSR[7]), `"8xmode"` (5G), absent |
| `"autoFlow"` | adds `"exarTable"` (fixed hysteresis tables of FCTR[3:0], table D's 16-entry list) and `"dtrDsr"` (MCR[2]-selected DTR/DSR flow, 5G) |
| `"swFlow"` | adds `"exarOr"` (1011/0111 = either character, P9) |
| `"xonAny"` | `"mcr5"` absent |
| `"sleep"` | adds `"ier4-exar"` (wake rules per spec), `"global"` (5G SLEEP register) |
| `"powerSave"` | `{"pin": name}`: PWRSAVE isolates the bus inputs and sleeps (XR16L2551, XR16M2551/2651, XR16V2551/2651) |
| `"irda"` | adds `"mcr6-exar"` (MCR[6] with MCR[2] polarity), `"dld7"` |
| `"rs485"` | adds `"fctr-rts"` / `"fctr-dtr"` (FCTR[5], delay 0–15 bit times in MSR[7:4] write-only), `"dld6"` (XR16M752 RTS), `"fctr3-op1"` (XR16C864 OP1) |
| `"prescaler"` | adds `"clksel"` (power-on ÷4 by the CLKSEL pin; MCR[7] reads 0 after reset, `decisions.md` P1) |
| `"mfPins"` | `{"pin": name, "functions": [...]}`: XR16V2552/2652's multi-function pins (OP2, BAUDOUT, RXRDY) by AFR |
| `"broadcastWrite"` | adds `"afr0-perChannel"` (XR16L2552: write broadcast while the addressed channel's AFR[0] = 1, planner decision) |
| `"startSample"` | `7.5` (Exar parts that print it) |
| `"iirPriority"` | adds `"exarTimeoutFirst"` (timeout above RX data), `"5g"` (RX above timeout) |
| `"thrIntClear"` | `"ier1Rewrite"` (ST16C1450/1550 rule) |
| `"bus"` | adds `"style": "intel-motorola"` with `"modePin"` (16/68: high = Intel), Motorola `"rw"`, `"strobe"` (the Intel read pin's Motorola meaning), `"irq"` (open-drain IRQ# in Motorola mode, not gated by MCR[3] on XR16M752, planner decision) |
| part key `"intPin"` | overrides the device's `intPin` for one part: the "D" variants (ST16C554D, XR16C854D, XR16V564D, XR16V654D) whose INT outputs are always driven (`specs/exar-a.md` § ST16C554, `exar-d.md`) |

### 3.2 The 5G flat map (`"extended": "5g"`)

XR16V798 (8 channels) and XR16L784 (4): one CS, address A7..A0 (A6..A4 or
A5..A4 select the channel, A3..A0 the register: 16 registers per channel), a
global block at 0x80–0x8F (INT0–INT3 summary, TIMER control/LSB/MSB, 8XMODE,
REGA/REGB (broadcast write, wake), RESET (per-channel soft reset), SLEEP,
DREV, DVID 0x48/0x24), the 16-bit timer clocked by CLK or TMRCK with its
interrupt, per-channel FIFO counters and trigger tables; RS-485 on RTS# only
for XR16L784 (planner decision). Details: `matrix.md` Table B § 5G and
`specs/exar-d.md` § XR16V798, § XR16L784.

### 3.3 Fixtures

As plan-12-D §3.3: the 28 device objects G ships are fixtures in
`serial_fixture.h`.

## 4. Tests (added to `serial_test`)

| Case | Fixture | Checks |
|---|---|---|
| E1 | ST16C550 | Exar baseline differences from PC16550D (its `decisions.md` rows) |
| E2 | ST16C450 | 16450 Exar |
| E3 | ST16C1450 | `ier5` power-down; `thrIntClear: ier1Rewrite`; timing per ST16C1550 (planner decision) |
| E4 | ST16C1550 | `ier5`, MCR[7] per `decisions.md` |
| E5 | ST16C2550 | dual, per-channel CS |
| E6 | ST16C554 | quad; part key `intPin` (D variant) both ways |
| E7 | XR16C2550 | as ST16C2550 (same JSON keys) |
| E8 | XR16L580 | `exar650`, `dld`, auto flow in loopback |
| E9 | XR16L2551 | `intel-motorola` both modes; `powerSave` |
| E10 | XR16L2552 | `afr0-perChannel` broadcast; `idRegs` |
| E11 | XR16M2550 | `exar854` tables A–D; `fcFlvl` |
| E12 | XR16M2551 | `clksel`; `powerSave`; Motorola |
| E13 | XR16M2650 | 32-byte FIFO tables |
| E14 | XR16M2651 | as XR16M2551 with 32-byte FIFO |
| E15 | XR16M752 | `m752`, `dld7`/`dld6`, FIFO Rdy, Motorola IRQ# not gated |
| E16–E21 | XR16V2550, 2551, 2552, 2650, 2651, 2652 | `exar854`, `mfPins` (2552/2652), `powerSave` (2551/2651), `idRegs` 0x02/0x06 |
| E22 | XR16V554 | quad, tables, `xonAny` |
| E23 | XR16V564 | 32-byte FIFO, `clksel`, D variant |
| E24 | XR16V654 | 64-byte FIFO, D variant |
| E25 | XR16C854 | `exar854`, `trg`, `exarOr` Xon/Xoff, D variant |
| E26 | XR16C864 | 128-byte FIFO, `fctr3-op1` RS-485, DRQ pins per `decisions.md` |
| E27 | XR16V798 | 5G map, global block, timer interrupt, `8xmode`, SLEEP, RESET, broadcast |
| E28 | XR16L784 | 5G map with 4 channels, RS-485 on RTS# |

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Exar banks, tables, divisors, flow control

- [x] Keys `ier5`, `exar650`, `exar854`, `m752`, `triggerTables`, `trg`,
  `fifoCounters`, `idRegs`, `dld`, `sampling`, `autoFlow` additions, `swFlow:
  exarOr`, `xonAny`, `startSample`, `iirPriority` additions, `thrIntClear`,
  part key `intPin`. In `backend/src/models/ns16550_exar.c`; the core and
  `ns16550_ext.c` call it through hooks marked `plan-12-E` (see Corrections
  for the values that differ from this list).
- [x] Fixtures and cases E1–E8, E10, E11, E13, E16–E26. The fixtures are
  `tests/fixtures/serial/keys/<ID>.json` merged into the data child's
  `devices/<ID>.json` (plan-12-D Corrections). Observed with Phase 2 (both
  phases landed together): `/tmp/logic-p12e/serial_test` prints `ok   E1
  ST16C550: SPR 0xFF, start at 7.5, DMA class N, 4w12 timeout, driven INT`
  … `ok   E26 XR16C864: 128-byte FIFO, FCTR[3] RS-485 on OP1, BCLK,
  DRQ/DACK/AEN/TC`.

Acceptance: those cases `ok`; `ctest` 100% (about 1 min). Observed:
`100% tests passed, 0 tests failed out of 19`.
Commit: 908486b (model), 81f86a0 (fixtures), e691735 (cases), with 5d2fbe4
(Motorola bus cycles) and Phase 2

### Phase 2 — Motorola bus, power save, sleep, IrDA/RS-485 variants, MF pins, 5G

- [x] `intel-motorola` bus, `powerSave`, `sleep` additions, `irda`/`rs485`
  additions, `prescaler: clksel`, `mfPins`, `broadcastWrite:
  afr0-perChannel`, `5g` (§3.2); `docs/parts-format.md` § `serial` gains §3.1
  (the "Exar option keys" table, the part key and the 5G map, 0a516c1).
- [x] Cases E9, E12, E14, E15, E27, E28 and the remaining parts of E-cases.
  Observed: `ok   E9 XR16L2551: Intel and Motorola (A3 channel, IRQ, RESET
  low), PowerSave isolation`, `ok   E12 XR16M2551: CLKSEL ÷4 and the MCR[7]
  override, Motorola, PowerSave`, `ok   E14 XR16M2651: …`, `ok   E15
  XR16M752: m752 TCR/TLR, FIFO Rdy, DLD[7] IrDA, DLD[6] RS-485, Motorola
  IRQ`, `ok   E27 XR16V798: 5G map, global block (INT0-3, DVID, REGB, RESET,
  8XMODE, SLEEP), timer, send-immediate, TX disable, DTR/DSR flow, RS-485
  delay`, `ok   E28 XR16L784: 5G map with 4 channels, reserved 0x40, RS-485
  on RTS, bit-mapped timer (TMRCK), timeout and wake-up clear rules`;
  `grep -c '^ok   E'` → 28, `grep -c '^ok   D'` → 24, C's 12 and the
  outcome, `shipped`, JF1/JF2 and J1–J6 `ok`; 86 `ok`, then `PASS`. Each E
  case was checked to fail when its key is taken away (startSample,
  iirPriority, lsrBreakHold, thrIntClear, autoFlowLoopback, swFlow,
  dlabHidesStatus, statusWrite, afr0-perChannel).

Acceptance: every E case and every C/D case `ok`; `ctest` 100%;
`git diff --stat backend/tests/golden` → none (about 1 min). Observed:
`cmake --build /tmp/logic-p12e` with no warnings; `100% tests passed, 0
tests failed out of 19`; `git diff --stat backend/tests/golden` → empty.
Commit: 5d2fbe4, 908486b, 81f86a0, e691735, 3c39acb, 0c381bb, 0a516c1

## Validation Plan

`serial_test` (§4) is the gate. The landing tier is plan-12-K's.

## Decisions

- D variants are a part-level `intPin` override, not separate devices: they
  share every other fact and their datasheet section.
- XR68M752 is a part of device XR16M752 (the same chip with the 16/68 pin,
  `specs/exar-b.md` § XR16M752).

## Corrections

- **`exar854` is the XR16C854/864 only.** §3.1 lists XR16L2551/2552,
  XR16M25xx/26xx, XR16V25xx/26xx and XR16V554/564/654 under `exar854`. Their
  register maps have no FCTR, EMSR, TRG, FC or FLVL (exar-b.md § XR16L2551
  Registers, § XR16M2550 Registers; exar-c.md § A.1; exar-d.md § XR16V564
  Registers; matrix.md A2 rows 32–47). They take `"exar650"` (the 0xBF bank
  with Exar's DLD, AFR, DREV/DVID), and the XR16V554 has no enhanced bank at
  all (exar-d.md § XR16V554: "Plain 16C550 register set"), so it has no
  `"extended"`. `exar650` is not D's `650` (SC16C650B): 0/1 in the 0xBF bank
  read X, address 2 with LCR[7] = 1 is DLD/AFR/ISR, Xoff goes out two
  characters late, an ISR read clears the Xoff interrupt, the special
  character clears with the next character, and auto RTS needs MCR[1].
- **Cases follow from that.** E11 checks `exar650` + DLD on the XR16M2550
  (fraction, 8× sampling, DVID, the RTS table); tables A–D, TRG and
  `fcFlvl` are checked on the XR16C854 (E25). E16–E21 check DLD, IrDA
  polarity, DVID 0x02/0x06, MF, AFR/DLD and PowerSave rather than `exar854`.
  E22 (XR16V554, no EFR) checks INTSEL, the shared RXRDY, LSR X with DLAB and
  the Motorola A4:A3 decode; `xonAny` is checked in E25.
- **Values not implemented, with no device.** `"fifoCounters"`:
  `"txlvlRxlvl"` and `"rfdTfd"` belong to the SC16IS/XR20M1280 bridges
  (plan-12-I), none of E's devices; the 5G TXCNT/RXCNT come with `"5g"`.
  `"sampling": "emsr7"` was the TL16C2752's, dropped from scope
  (decisions.md header). `"iirPriority": "5g"` (RX data above the timeout) is
  D's `"standard"` order, so the 5G devices use the default.
- **Value names changed.** `"rs485": "fctr-dtr"` is `"fctr-mcr2"`: the
  XR16V798 pin is RTS# or DTR# by MCR[2] (decisions.md § XR16V798/XR16L784).
  `"mfPins"` is the list of AFR[2:1] functions (`["op2", "baudout",
  "rxrdy", "x"]`); the pins are D's channel `"mf"` keys (one per channel), and
  the XR16L2552 has them too (exar-b.md § XR16L2552 AFR). `"intel-motorola"`
  names `"modePin"`, `"rw"`, `"strobe"` (the Motorola CS, default the first
  select), `"channelBits"` and `"irq"`; "strobe" in §3.1 ("the Intel read
  pin's Motorola meaning") does not apply to any E device: the Intel read pin
  is unused in Motorola mode.
- **Keys and values added beyond §3.1** (each from its spec section; all in
  `docs/parts-format.md`): `"afr": "exar"` (XR16L2552, XR16V2552/2652);
  `"sleep": "ier4-exar-all"` (sleep needs IER[4] on every channel:
  XR16L2551/2552, XR16M/V25xx/26xx, XR16V564/654, XR16C854/864);
  `"sleepNeedsDivisor"` (XR16L580/2551/2552), `"wakeIndicator"` (XR16L580,
  XR16M2551/2651, XR16V2551/2651; decisions.md exar-c: the INT pin asserts);
  `"isrStatus"` (ier5 polarity, below), `"resetOut"` (the RST pin),
  `"divisorPowerUp"` (DLL/DLM 0x0001 at power-up only, not reset);
  `"mcr2Enhanced"`, `"xonWriteOnly"`, `"autoFlowLoopback"` (XR16L580);
  `"lsrBreakHold"` ("the break indication remains until the RX input returns
  to the idle condition": ST16C2550/XR16C2550, XR16L580/2551/2552);
  `"statusWrite": false` (XR16V654 LSR/MSR read-only), `"dlabHidesStatus"`
  (XR16V554: LSR/MSR with LCR[7] = 1 read X, decisions.md); `"fstat"` (FSRS),
  `"chcClock"` and channel `"irtx"` (XR16C854/864 QFP100); `"dmaHost"` and
  channel `"bclk"` (XR16C864); `"interrupt"`, `"timer"`, `"variant5g"`,
  `"irdaStrap"` (the 5G INT#, timer, variant and ENIR); bus `"irqOthers"`
  (the other INT pins in Motorola mode: XR16L2551/XR16M2551/2651 driven low
  (decisions.md), XR16V2551/2651 NC (exar-c.md), XR16M752 and the quads
  unstated: Z).
- **ST16C1450/1550 ISR[5:4].** decisions.md § ST16C1450 says "ISR[4] =
  RXRDY and ISR[5] = TXRDY, active high". The spec puts TXRDY in bit 4 on
  both parts (exar-a.md § ST16C1450: Figures 11/12 "RXRDY (ISR bit-5)",
  "TXRDY (ISR bit-4)"; § ST16C1550: "[4] = NOT TXRDY, [5] = NOT RXRDY"). The
  model follows the spec for the bits, the decision for the 1450's polarity
  (active high) and the spec for the 1550's (inverted): `"isrStatus"`.
- **ST16C1450 timing per ST16C1550** (planner decision): the data child's
  ST16C1450 grade carries the ST16C1550 values; its TXRDY/RXRDY are ISR bits,
  not pins, so `twxi`/`twrr` time nothing on this part.
- **Exar TX trigger levels count bytes** (exar-a.md § ST16C1550 FCR, exar-b.md
  § XR16M2550 FCR, exar-d.md § XR16V564 FCR): the interrupt when the TX FIFO
  count falls below the level, or when it empties if it never reached the
  level since the last reload; the 5G interrupts at both (exar-d.md
  § XR16V798 IER[1]). The XR16M752 counts free spaces, TI style
  (decisions.md § XR16M752). D's core works in spaces, so the hook returns
  depth − level + 1.
- **Decode with LCR[7] = 1 and EFR[4] = 0 at address 2** is ISR/FCR on the
  XR16M/V25xx/26xx (decisions.md exar-c). On the XR16V564/654 and
  XR16C854/864 the map lists ISR/FCR (and 100–111) only with LCR[7] = 0, so
  with LCR[7] = 1 those read X and writes are dropped (decisions.md
  § XR16V564 "100–111 … X", extended to 010 on the same table rule): D's
  `"dlabHidesUpper"` on those devices.
- **The D variants** are fixture parts `SFX-<ID>-lqfp64-driven`: E adds
  `"intPin": "driven"` to ST16C554DCQ64-F, ST16C554DIQ64-F, XR16C854DIV-F,
  XR16V564DIV and XR16V654DIV-F in `devices/<ID>.json` (the data child's
  files: G ships these part keys), and `merge.py` makes one fixture part per
  part-level `intPin`.
- **Motorola mode** makes the reset pin active low on every
  `"intel-motorola"` device (Exar specs: "RESET (Intel) / RESET# (Motorola)");
  the 5G RST# is always active low (`"resetActiveHigh": false`). The
  TL16C750E's MODE pin (IOW-only: CS the strobe, IOW R/W; plan-12-D
  Corrections) is a candidate for the same style (`"modePin"`, `"rw"`),
  checking its MODE polarity and whether its reset flips.
- **5G timeout.** "4 character times plus 12 bits" (exar-d.md § XR16V798) is
  D's `"4w12"` as implemented (4 characters + 12 bit times).
- **XR16C864 class X**: with EMSR[2]/[3] off the pins are TXRDY/RXRDY with
  the class-N rules (decisions.md § XR16C864: "DRQ disabled (pins act as
  TXRDY/RXRDY)").
- **Shared DMA pins on receive events.** The core drives D's shared
  TXRDY/RXRDY (TL16C554) only from bus accesses, so a received byte did not
  move them; E refreshes them for its devices from its device-pin hook (the
  XR16V554/564/654 and XR16C854/864 shared AND). D's TL16C554 keeps the core
  behavior (reported to the plan-12-D owner, not changed here).
- **Factory test mode** (XR16C854/864: IOR, IOW and CS low together) is not
  described; the core's both-strobes cycle applies.
- **Fixtures** are `tests/fixtures/serial/keys/<ID>.json` (plan-12-D
  Corrections), not `serial_fixture.h` strings (§3.3).
