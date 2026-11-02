# plan-12-D: `ns16550` — TI, NXP and National extensions; the printer port

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-C

C's `ns16550` is the PC16550D. D adds the options the TI, NXP, National,
Intersil and SMSC 16550-family devices need, and the PC parallel printer
port block of the TL16C451/452/552/552A and COM92C451. Each option is a
device key with the values `plan-12-data/matrix.md` §3 lists; behavior comes
from the device's spec section and `decisions.md`.

The D-model devices (their parts land in F, except SC16C650B and SC16C752B,
whose parts land with the Exar set in G): TL16C450, TL16C451, TL16C452,
TL16C550B, TL16C550C, TL16C550D, TL16C552, TL16C552A, TL16C554, TL16C554A,
TL16C750, TL16C750E, TL16C754B, TL16C754C, TL16C752C, TL16C2550, TL16C2552,
NS16C2552, NS16C2752, PC16550D, 82C50A, COM92C451, SC16C650B, SC16C752B
(24 devices: `devices.tsv`).

Outcome of D: `serial_test` has a fixture device for each of the 24 devices
(the exact device JSON F and G will ship, §3.3) and one case per option
value of §3.1 and per decision row of `decisions.md` for those devices that
changes behavior; all pass.

References: plan-12-C §3.6 (core keys); `plan-12-data/matrix.md` Table A
(rows for the 24 devices) and §3 (`ns16550` parameters);
`plan-12-data/specs/ti-a.md`, `ti-b.md`, `ti-c.md`, `exar-d.md` (§ SC16C650B,
§ SC16C752B), `classic-b.md` (§ COM92C451); `plan-12-data/decisions.md`
(the 24 devices' sections).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-C landed | `ls backend/src/models/ns16550.c` → exists; `/tmp/logic-p12/serial_test` → 12 `ok` | MET (2026-11-01 at 627f50d: exists; 12 `ok` + outcome) |

## 1. Goal

- `serial_test` prints `ok` for each case of §4 (one line per case, named
  `D<n> <device> <option>`); `ctest` → 100%, T0 + 1 programs.
- `git diff --stat backend/tests/golden` → no change.

### Non-goals

- C's behavior with no D key present stays as it is (every C case still
  passes).
- No part file (F, G); no Exar-only option (E); no Motorola bus (E).

## 2. Current State

After C: `ns16550.c` handles one channel, Intel bus, 16-byte FIFO or none,
`intPin: driven`, `dmaClass: P`, `timeout: 4ch` (plan-12-C §3.6).
`matrix.md` Table A gives, per device, the option value each key below takes.

## 3. Design

### 3.1 Keys D adds

Every key is optional; absent = C's PC16550D behavior. Values are exactly
those named here; each value's behavior is in the cited device's spec
section.

| Key | Values (devices) |
|---|---|
| `"channels"` | 1, 2, 4 entries; each channel may add `"select"` / `"selectHigh"` (its own chip selects: TL16C552/554, TL16C2550) |
| `"channelSelect"` | `{"pin": name, "aIsHigh": bool}` — one CS plus a channel pin (TL16C2552, NS16C2552/2752) |
| `"broadcastWrite"` | `"multi-cs"` (several channel CS low on a write: each writes, P2), `"afr-conc"` (TI AFR CONC bit), `"none"` |
| `"fifo"` | 16, 64, 128, `"750"` (16 or 64 by FCR[5] with LCR[7] = 1) |
| `"rxTrigger"` | 4 numbers per table (`[1,4,8,14]`, `[8,16,56,60]`, `[1,4,56,60]`, `[1,4,120,124]`, `[1,16,32,56]`); with `"fifo": "750"` a second table `"rxTrigger64"` |
| `"txTrigger"` | 4 numbers (`[8,16,32,56]` …) or absent |
| `"tlr"` | absent, `4` (TLR nibble × 4) or `8` (× 8, TL16C750E) |
| `"extended"` | `"afe"` (TL16C550C/D MCR[5] auto flow), `"750"` (FCR[5], IER[4]/[5] sleep, TL16C750/750E), `"650"` (LCR = 0xBF bank: EFR, Xon1/2, Xoff1/2; SC16C650B), `"752"` (650 bank plus MCR[2]-gated TCR/TLR, TL16C754B/C, TL16C752C, SC16C752B), `"nsEfr"` (NS16C2752 enhanced bank), absent |
| `"afr"` | `"ti752"` (TI AFR at LCR = 0xBF address 2: CONC, IREN, RS-485 + DLY), `"ti2552"`, `"ns"`, absent |
| `"dlf"` | true: TL16C750E fractional divisor (n/64) and 8× sampling |
| `"autoFlow"` | `"afe"` (RTS by RX trigger, CTS gates TX), `"tcr"` (halt/resume levels from TCR), `"efrTable"` (SC16C650B: the fixed table of `decisions.md`) |
| `"swFlow"` | `"nxpAnd"` (Xon/Xoff 1011/0111 = both characters in sequence), `"tiAnd"` (TI: as NXP per the TI tables) |
| `"specialChar"` | true: Xoff2 match raises the special-character interrupt (EFR[5]) |
| `"sleep"` | `"ier4"` (per channel), `"ier4-all"` (chip sleeps when every channel sets it), absent |
| `"irda"` | `"mcr6"` (SC16C752B), `"afr1"` (TI AFR IREN), absent — IrDA 3/16-bit pulses on TX, pulse decode on RX |
| `"rs485"` | `"afr-dtr"` (TI: DTR follows TX activity, DLY 0–7 bit times), absent |
| `"prescaler"` | `"mcr7"` (÷1/÷4 by MCR[7]), `"mcr7-clksel"` (power-on value from the CLKSEL pin, TL16C754B), absent |
| `"dmaPins"` | `"perChannel"`, `"sharedAnd"` (TL16C554: one TXRDY/RXRDY for all channels, ANDed), `"sharedWireOr"` (TL16C754B), absent (no pins) |
| `"dmaClass"` | `"P"`, `"N"`, `"T"`, `"X"`, `"B"` (`matrix.md` legend) |
| `"intPin"` | `"driven"`, `"out2-gated"` (3-state unless MCR[3] = 1: TL16C450/451/452, 82C50A, COM92C451), `"intn"` (TL16C554: INTN pin low = always driven, high = MCR[3] gating), `"intsel"` (TL16C754B: INTSEL pin), `"mcr5-open-source"` (SC16C650B) |
| `"resetActiveHigh"` | false for TL16C451/452/552/552A (`bus` key, C) |
| `"lcrReset"` | `0` (default) or `29` (= 0x1D: TL16C750E, TL16C754B/C, TL16C752C, SC16C752B) |
| `"sprReset"` | absent (unchanged) or 255 |
| `"tcrReset"` | 0 or 15 |
| `"iirPriority"` | `"standard"`, `"nxpEqual"` (RX data and timeout one level: SC16C650B, SC16C752B) |
| `"lsr7Clear"` | `"noErrors"` (default) or `"lsrRead"` (SC16C650B) |
| `"stopCheck"` | `"first"` (default) or `"both"` (82C50A) |
| `"statusReadDefer"` | true: a status bit changing during an LSR/MSR read is held until the read ends (82C50A, TL16C552) |
| `"busSync"` | true: IOR/IOW sampled on XTAL1 edges (TL16C754B, SC16C752B, `decisions.md`) |
| `"majorityVote"` | true: RX bit = majority of 3 samples around mid-bit (TI EFR parts) |
| `"timeout"` | `"4ch"`, `"4w12"` (4 words + 12 bit times) |
| `"clock"` | C's keys plus `"single": name` (one CLK input feeding both TX and RX, TL16C451/452/552/552A, COM92C451) |
| `"printerPort"` | §3.2 |

### 3.2 Printer port block

The TL16C451/452/552/552A and COM92C451 carry a PC-compatible parallel port:
`"printerPort": {"data": [PD0..PD7], "select": name, "address": [2 names],
"strobe", "autoFeed", "init", "selectIn", "ack", "busy", "paperEnd", "slct",
"error", "irq", "inputEnable"}`. Registers (data, status, control at the
port's own select and A1..A0), the pin polarities, the IRQ enable and the
bidirectional data direction (TL16C452's input enable) follow
`specs/ti-a.md` § TL16C451/452 and § TL16C552(A) (`ti-b.md`), and
`classic-b.md` § COM92C451, with `decisions.md` for the inverted bits. The
port has no terminal (it is not a serial channel).

### 3.3 Fixtures are the shipped devices

For each of the 24 devices, `serial_test` holds the device's full JSON
(keys, pinouts for every package its parts use, grades) as a fixture string
`SFX_<ID>` in `backend/tests/support/serial_fixture.h`, with one fixture part
per pinout named `SFX-<id>-<pinout>` on the footprint the real parts use.
F and G copy each device object into `serial.json` unchanged except for the
`parts` array (a test in F/G compares them, plan-12-F §3). So the device
tested here is the device shipped.

## 4. Tests (added to `serial_test`)

One case per row, on the named fixture (D1–D24 cover every key value of
§3.1; the device's `decisions.md` rows that change behavior are folded into
its case):

| Case | Fixture | Checks |
|---|---|---|
| D1 | TL16C550C | auto flow (`afe`): RTS drops at the trigger, CTS high holds TX after the current frame |
| D2 | TL16C550D | `majorityVote`; package without modem inputs (BGA): MSR bits X, deltas 0 |
| D3 | TL16C750 | `fifo: "750"` 16/64 switch, `rxTrigger64`, sleep (IER[4]) stops the generator and wakes on RX |
| D4 | TL16C750E | `lcrReset` 0x1D, `dlf` fractional divisor (period = (n + f/64) × 16 clocks), `tlr: 8` |
| D5 | TL16C754B | `752` bank: EFR enable, TCR/TLR via MCR[2]; `autoFlow: tcr`; Xon/Xoff `tiAnd`; special char; `intsel`; `mcr7-clksel`; `sharedWireOr` DMA; `busSync` |
| D6 | TL16C754C | 754C trigger table `[1,4,56,60]`; its `decisions.md` rows |
| D7 | TL16C752C | `afr: ti752`: CONC broadcast, IrDA (`afr1`) pulse widths, RS-485 DTR with DLY 3 |
| D8 | TL16C554 | four channels with own CS; `sharedAnd` TXRDY/RXRDY; `intn` both levels |
| D9 | TL16C554A | its differences from TL16C554 (spec § TL16C554A) |
| D10 | TL16C552 | two channels + printer port; `resetActiveHigh: false`; `statusReadDefer` |
| D11 | TL16C552A | its differences from TL16C552 |
| D12 | TL16C451 | 16450 channel + printer port, single CLK, `out2-gated` |
| D13 | TL16C452 | two 16450 channels + printer port; bidirectional data via input enable |
| D14 | TL16C450 | 16450 single channel, `out2-gated` |
| D15 | TL16C550B | its DMA class `B` per `decisions.md` |
| D16 | TL16C2550 | per-channel CS, dual |
| D17 | TL16C2552 | `channelSelect`, `afr: ti2552` |
| D18 | NS16C2552 | `channelSelect` with NS polarity, `afr: ns` |
| D19 | NS16C2752 | `nsEfr` bank, 64-byte FIFO, its trigger tables |
| D20 | 82C50A | `stopCheck: both`, `statusReadDefer`, `out2-gated`, no FIFO |
| D21 | COM92C451 | 16450 + printer port per classic-b |
| D22 | SC16C650B | `650` bank, `efrTable` auto flow, `nxpAnd` Xon/Xoff, `mcr5-open-source`, `nxpEqual`, `lsr7Clear: lsrRead` |
| D23 | SC16C752B | `752` bank, `lcrReset` 29, `irda: mcr6`, FIFO Rdy register (1 = ready, `decisions.md`), `busSync` |
| D24 | PC16550D | the C fixture replaced by the shipped device object; C's 12 cases still pass on it |

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Multi-channel, bus schemes, FIFO sizes, DMA, INT pin

- [x] Keys `channels` (>1), `channelSelect`, `broadcastWrite`, `fifo` 64/128/750,
  `rxTrigger64`, `txTrigger`, `tlr`, `dmaPins`, `dmaClass`, `intPin`, reset
  and register reset keys, `iirPriority`, `lsr7Clear`, `stopCheck`,
  `statusReadDefer`, `busSync`, `majorityVote`, `timeout`, `clock.single`.
  Implemented (8272fde: loader, decode, behavior). Every key is exercised by
  a D case that prints `ok` (`/tmp/logic-p12/serial_test`).
- [x] Fixtures and cases D2, D8, D9, D12–D16, D18, D20, D24. The fixtures are
  `tests/fixtures/serial/keys/<ID>.json` merged into the data child's
  `devices/<ID>.json` by `merge.py` (see Corrections). Observed on the real
  device data (15723fb): `ok   D2 TL16C550D: timeout delay 9 RCLKs; bga24
  without modem inputs`, `ok   D8 TL16C554: four channels, shared AND TXRDY,
  INTN both levels`, `D9`, `D12`–`D16`, `D18`, `ok   D20 82C50A: both stop
  bits checked, deferred status, driven INTRPT`, `ok   D24 PC16550D: the C
  cases on the shipped device object`; `grep -c '^ok   D'` → 24.

Acceptance: `serial_test` → every listed case `ok`; `ctest` 100% (about 1 min).
Observed: `PASS`; `100% tests passed, 0 tests failed out of 19`.
Commit: 15723fb

### Phase 2 — Extended banks, flow control, sleep, IrDA, RS-485, printer port

- [x] Keys `extended`, `afr`, `dlf`, `autoFlow`, `swFlow`, `specialChar`,
  `sleep`, `irda`, `rs485`, `prescaler`, `printerPort`;
  `docs/parts-format.md` § `serial` gains §3.1/§3.2.
  Implemented in `ns16550_ext.c` and `ns16550_lpt.c`, with the core hooks.
  `docs/parts-format.md` § `serial` documents every D key (2ac9b98).
- [~] Fixtures and cases D1, D3–D7, D10, D11, D17, D19, D21–D23. Observed on
  the real device data (15723fb): each prints `ok` (`ok   D1 TL16C550C afe:
  …`, `ok   D4 TL16C750E: LCR 0x1D, DLF fraction n/64, TLR access`, …,
  `ok   D23 SC16C752B: LCR 0x1D, 752 decode X rule, FIFO Rdy, IrDA`).
  Remaining: D4's MODE-pin (IOW-only) bus once E's `intel-motorola` style
  merges.

Acceptance: `serial_test` → all 24 D cases and C's 12 `ok`; `ctest` 100%;
`git diff --stat backend/tests/golden` → none (about 1 min).
Commit: —

## Validation Plan

`serial_test` is the gate: each D case exercises one key value on the device
that uses it, before F/G ship any part. The landing tier is plan-12-K's.

## Decisions

- The fixture is the shipped device object (§3.3), so a model test cannot
  pass on a device that differs from the part file.
- SC16C650B and SC16C752B are modeled in D (their registers are the 650/752
  styles D implements) though their parts ship in G with the other non-TI
  parts.

## Corrections

- **Fixture format (§3.3).** §3.3 said C strings `SFX_<ID>` in
  `serial_fixture.h`. Instead each device is two JSON files merged by
  `tests/fixtures/serial/merge.py` into `ns16550.json`, which `serial_test`
  loads: `keys/<ID>.json` (the model keys, written here) and
  `devices/<ID>.json` (pinouts, grades, parts: the data child for F/G). F and
  G build `serial.json`'s devices from the same pair, so the tested object is
  the shipped one. A device without its data file gets a stand-in pinout of
  the pins its keys name; the merge reports which.
- **Keys added beyond §3.1** (each needed by a device's spec; all
  documented in `docs/parts-format.md`): `dma` (shared DMA pin names),
  `intControl` (the INTN/INTSEL pin), channel `op` / `mf` pins, `mcrReset`,
  `afrReset`, `xonReset`, `sprPowerUp`, `lsrErrors` (`head`: the 752 style's
  LSR[4:2]), `specialNeedsNoFlow`, `specialClear`, `ctsRtsClear`,
  `tcrNeedsDlab0`, `dlabHidesUpper`, `idRegs` (NS DREV; E reuses it),
  `timeoutDelay` (TL16C550C/D: 9 RCLKs), `divisorZero` (COM92C451 ÷3),
  `tristate` (TL16C552 TRI), `dmaMode1: false` (SC16C650B), `clock.select`
  (CLKSEL), `clock.baudOutShape` (`low2`), `rtsTable` / `xoffTable` (the
  `efrTable` data), and `absentTies` / `optionalPins` (package pin subsets:
  TL16C550D RHB/ZQS, TL16C2550 RHB, SC16C650B HVQFN32, …).
- **INT pin of TL16C450 and 82C50A.** §3.1 lists them under `out2-gated`.
  Their specs say the pin is always driven: ti-a.md § TL16C450 "INTRPT …
  no OUT2 gating and no 3-state", § 82C50A "OUT2 does not gate INTRPT".
  Both use `intPin: driven`; D14/D20 check it.
- **D2 majority vote.** D2 listed `majorityVote` for the TL16C550D. Its spec
  samples once, 8 RCLKs into the bit (ti-a.md § TL16C550D). D2 checks its
  9-RCLK timeout delay and the bga24 package without modem inputs instead.
  `majorityVote` is checked on the TL16C754B, TL16C752C and TL16C750E parts.
- **`swFlow: tiAnd`** follows the TI EFR table: 1011/0111 compare either
  character, and 1111/0011 compare sequential pairs (ti-b.md § TL16C754B
  Table 1; ti-c.md § TL16C752C). `nxpAnd` makes 1011/0111 sequential too
  (exar-d.md § SC16C650B; decisions.md, file-wide note).
- **`statusReadDefer` on the TL16C554/554A as well** (ti-b.md § TL16C554:
  "Status set during an IOR is deferred to the trailing edge of the read").
- **IIR order `standard`.** RX data shows above the timeout. C's first cut
  showed the timeout first. Matrix A4 lists "timeout above RX data" as an
  Exar oddity (ST16C2550), so the TI/National default is RX data first. C's
  cases are unaffected (no case has both pending).
- **AFR concurrent write** works whenever the device has an AFR, whatever
  `broadcastWrite` says. The TL16C752C both broadcasts with both CS low
  (`multi-cs`) and has AFR CONC.
- **`busSync`.** The strobes are sampled on XTAL1 rising edges. The write
  data is latched at the strobe's trailing edge itself, because the
  datasheets time data hold from IOW↑ (ti-b.md § TL16C754B T16h). The bench
  waits 400 ns after a strobe for these parts (the 4P cycle spacing).
- **BAUDOUT shape.** TL16C450/550B/C/D, 750, 82C50A and the 2552's MF BAUDOUT
  are low for 2 clocks and high for N − 2 (low 1, high 1 at N = 2): `clock.baudOutShape: "low2"`.
- **COM92C451 IRQ0** (decisions.md "IRQ0 clear condition → NPD"; the
  planner entry describes the ACE's interrupt). IRQ0 is the PIA's: it is
  latched on ACK's falling edge while control bit 4 is set (Fig 9: "IRQ0 rises ≤ 35 ns after ACK falls"),
  cleared by a status read, and 3-state while disabled.
- **Found in self-review** (keys added, each with its spec):
  - `lsrClearOnGood` (NS16C2552/2752: LSR[4:1] "reset when the next valid
    character is loaded into RBR"); D18/D19 check it.
  - `breakMarkTicks: 2` (TL16C550C/D, TL16C750: "the next character is
    accepted after SIN is marking for at least 2 RCLK samples").
  - Compare mode 01 with EFR[5]: Xoff2 is not stored and raises both the
    Xoff and the special-character interrupt (NS16C2552 Table 20).
- **TL16C750E MODE pin** (IOW-only: CS is the strobe, IOW is R/W). It uses
  E's `intel-motorola` bus style when that merges; D4 covers the rest.
