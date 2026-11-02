# plan-14-E: The W65C51N on `acia6551`; the W65C02S I/O board

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-14-D

E adds the WDC W65C51N ACIA as a variant of plan-12-H's `acia6551` serial
model — its documented transmitter quirk, no parity, and its pinouts — with
the new `LQFP-32-7X7` footprint and 3 parts. Then a whole W65C02S board with
a W65C22S, a W65C21N and a W65C51N runs a program: ports out, a Timer 1
interrupt, and `HI` through the ACIA to its terminal.

Outcome of E: `serial_test` cases E1–E7 and `cpu_test` case `io_board`
pass; `serial.json` gains device `W65C51N` (3 parts); `parts.txt` and
`traces.txt` gain 3 lines each, `footprints.txt` 1.

References: `datasheets/controller-uart/w65c51n.pdf` — pinouts p.6,
internal organization p.7, status register p.9–10 (TDRE note p.10),
control register p.11–12, command register p.13–14, CTSB p.17, Table 3
(TX/RX timing) and Table 4 (bus AC) p.20–21, DC p.22, outlines p.23 and
p.25, ordering p.26; plan-12-H §3.4 (`acia6551`), plan-12-C §3.2 (line
engine), §3.4–§3.5 (terminal hooks, `circuit_serial_sent`/`_send`);
`backend/tests/support/cpu_board.{h,c}` (plan-09-D boards);
`docs/parts-format.md` § Footprints (rules R2 and mid-values).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-D landed | `grep -c -F '|pio|' backend/tests/golden/parts.txt` → 7 | NOT MET (2026-11-01) |

## 1. Goal

- `serial_test` E1–E7 `ok`; every plan-12 case still `ok`.
- `cpu_test` → `io_board ok` with the checks of §3.4.
- Goldens: + 3 `parts.txt`, + 3 `traces.txt`, + 1 `footprints.txt`; no
  removed or changed line.

### Non-goals

- `acia6551` behavior without the `"w65c51n"` key is unchanged: the
  CDP65C51 cases (plan-12-H H6) pass untouched, and the CDP65C51 golden
  lines do not change.
- No `serial` kind or terminal API change. No app change (plan-12-K's
  serial support shows the W65C51N; F checks it).

## 2. Current State

After plan-12: `acia6551` (`backend/src/models/serial_acia6551.c`) models
the CDP65C51/CDP65C51A: CS0/CS1, RS0/RS1, R/W, PHI2 bus; status, command,
control; the 16-rate baud table from 1.8432 MHz or the external ×16 clock;
TDRE level interrupt; `"ctsA"` grade key. After D: 7 `pio` parts.

## 3. Design

### 3.1 The `"w65c51n"` device key

A boolean device key of `acia6551` (default `false`). With `true`:

1. **TDRE reads 1, always** (status bit 4; p.10: "always a 1 because the
   TSR is loaded when the TDR is written to").
2. **A TDR write loads the transmit shift register at once.** Idle line: the
   frame starts at the next 16× tick, as before. A frame in progress is cut
   off: at the next bit boundary TxD starts the new byte's start bit; the
   cut-off character is not added to the sent log (the terminal never shows
   a byte the line did not finish). The sheet's instruction is to wait with a
   delay loop (p.10); this makes a program that does not wait visibly lose
   characters.
3. **No parity.** Command bits 5–7 are ignored: no parity bit is sent or
   checked (p.14: "should always be 0 … any combination of these bits is
   acceptable"). Status bit 0 (PE) reads 0.
4. **Transmit interrupt.** TIC = `01` ("do not use", p.13) with DTR (command
   bit 0) = 1: status bit 7 is set and IRQB asserted at once and again right
   after every status read, because TDRE is always 1 — the plan-12 TDRE level
   rule applied to a TDRE that never clears.
5. **CTSB high** stops the transmitter at once (p.17; `"ctsA"` false).
6. Reset values as p.11 and p.13 (control and command 0 on RESB; a programmed
   reset clears command bits 4–0 and leaves control).

Everything else (registers, baud table p.11, echo mode, DCD/DSR interrupts,
RxC output when RCS = 1) is the `acia6551` model unchanged.

### 3.2 Device, timing, pinouts, parts

```json
{"id": "W65C51N", "function": "UART", "drawer": ["Interface"],
 "description": "Asynchronous communications interface adapter (WDC W65C51N)",
 "note": "W65C51N (datasheets/controller-uart/w65c51n.pdf): pinouts p.6, registers p.9-14, AC p.20-21; TDRE always 1 (p.10)",
 "model": "acia6551", "w65c51n": true, … the acia6551 pin keys …,
 "grades": {"14": { … }}}
```

- **Grade `14`** uses Table 4's **4 MHz column** (tCYC 250, tC 100, tAC 30,
  tCAH 0, tWC 30, tCWH 0, tDCW 35, tHW 5, tCDR 50, tHR 10, tCDA 10 ns) and
  Table 3 (tCCY 400, tCH 175, tCL 175, tDD 500, tDLY 500, tIRQ 500 ns), under
  the `acia6551` timing names plan-12-H defines. The part code says 14 MHz
  but the sheet publishes no 14 MHz column; the fastest published column is
  the one the model can check against. `vcc [4.75, 5.25]` (p.20 operating
  conditions, 5 V ± 5 %). The grade's `note`: "-14 part code; timing from
  the 4 MHz column of Table 4 (no faster column published)".
- Pinouts (p.6):
  - `p28` (PDIP and PLCC share it): 1 VSS, 2 CS0, 3 CS1B, 4 RESB, 5 RxC, 6
    XTLI, 7 XTLO, 8 RTSB, 9 CTSB, 10 TxD, 11 DTRB, 12 RxD, 13 RS0, 14 RS1, 15
    VDD, 16 DCDB, 17 DSRB, 18 D0, 19 D1, 20 D2, 21 D3, 22 D4, 23 D5, 24 D6,
    25 D7, 26 IRQB, 27 PHI2, 28 RWB.
  - `lqfp32`: 1 RxC, 2 XTLI, 3 XTLO, 4 RTSB, 5 CTSB, 6 TxD, 7 DTRB, 8 NC,
    9 RxD, 10 RS0, 11 RS1, 12 VDD, 13 VDD, 14 DCDB, 15 DSRB, 16 D0, 17 D1,
    18 D2, 19 D3, 20 D4, 21 D5, 22 D6, 23 D7, 24 NC, 25 IRQB, 26 PHI2, 27 RWB,
    28 VSS, 29 VSS, 30 CS0, 31 CS1B, 32 RESB.
  - Pin roles as the CDP65C51's (plan-12-H): IRQB open-drain, RxC `inout`,
    XTLO output never driven (plan-12-A § Clock: an oscillator drives XTLI).

| Part | package | pinout | footprint |
|---|---|---|---|
| W65C51N6TPG-14 | DIP | p28 | PDIP-28-600 |
| W65C51N6TPLG-14 | PLCC | p28 | PLCC-28 |
| W65C51N6TQG-14 | LQFP | lqfp32 | LQFP-32-7X7 |

### 3.3 The `LQFP-32-7X7` footprint

From WDC Figure 12 (p.25) by rule R2 with mid-values: L 0.60 mid
(0.45–0.75) → pad length 1.40; b 0.375 mid (0.30–0.45) → pad width 0.425;
E 9.00 BSC → span 9.00 + 1.0 − 1.40 = 8.60; body D1/E1 7.00 BSC; e 0.80.

```json
{"id": "LQFP-32-7X7", "description": "32-pin LQFP, 7 x 7 mm body, 0.8 mm pitch",
 "note": "WDC W65C51N 32-pin LQFP, Figure 12 (R2; L 0.60 mid (0.45-0.75), b 0.375 mid (0.30-0.45), D/E 9.00 BSC, D1/E1 7.00 BSC, e 0.80), controller-uart/w65c51n.pdf p.25",
 "style": "quad", "pins": 32, "body": {"width": 7.0, "height": 7.0},
 "sides": [8, 8, 8, 8], "pitch": 0.8, "spanX": 8.6, "spanY": 8.6,
 "pad": {"width": 1.4, "height": 0.425, "shape": "rect"}}
```

Pin 1 is the top of the left side (p.6 Figure 1c), the `quad` default.
Expected `footprints.txt` line starts
`LQFP-32-7X7|pins=32|body=7x7|pads=1:-4.3,-2.8,1.4x0.425,rect;`.

### 3.4 The I/O board (`tests/support/io_board.{h,c}`, a `cpu_test` case)

Library parts on the engine, `5V` supply: W65C02S6TPG-14, AT28C256-15PU,
AS6C62256-55PCN, W65C22S6TPG-14, W65C21N6TPG-14, W65C51N6TPG-14,
ECS-2100A-018 (1.8432 MHz default), three SN74HC00N, one PULLUP-5V
(plan-12-A). The bench drives PHI2 at 1 MHz 50 % and RESB low for the first
4 cycles, as `cpu_board`.

Decoding (NAND gates g1–g10):

| Gate | Inputs | Output | Use |
|---|---|---|---|
| g1 | A15, A15 | ¬A15 | EEPROM CE# (8000–FFFF) |
| g2 | A14, A14 | ¬A14 | |
| g3 | g1, g2 | A15 ∨ A14 | SRAM CE# (0000–3FFF) |
| g4 | g1, A14 | ¬(¬A15 ∧ A14) | I/O select, low at 4000–7FFF → CS2B (VIA, PIA), CS1B (ACIA) |
| g5 | A13, A13 | ¬A13 | PIA CS1 |
| g6 | A12, A12 | ¬A12 | |
| g7 | g5, g6 | A13 ∨ A12 | |
| g8 | g7, g7 | ¬(A13 ∨ A12) | ACIA CS0 |
| g9 | RWB, RWB | ¬RWB | EEPROM and SRAM OE# |
| g10 | PHI2, g9 | | SRAM WE# |

VIA CS1 = A13 (6000–7FFF), RS0–RS3 = A0–A3; PIA CS0 = A12 (5000–5FFF),
RS0/RS1 = A0/A1; ACIA (4000–4FFF) RS0/RS1 = A0/A1, XTLI = the oscillator,
CTSB, DCDB, DSRB = GND, RxD = TxD (loopback). CPU IRQB = VIA IRQB (W65C22S,
totem pole); PIA IRQAB/IRQBB and ACIA IRQB on the PULLUP-5V net (unused by
the CPU); NMIB, RDY, SOB, BE = 1. Every chip's PHI2 = the bench clock, RESB
= the bench reset.

Program (at 8000; vectors FFFC = `00 80`, FFFE = the `IRQ` address), hand
assembled into the test with this listing as its comment:

```
RESET: LDX #$FF / TXS / STZ $0200
       LDA #$FF / STA $6002        ; VIA DDRB
       LDA #$55 / STA $6000        ; VIA ORB
       LDA #$FF / STA $5000        ; PIA DDRA (CRA bit 2 = 0 after reset)
       LDA #$04 / STA $5001        ; PIA CRA: select ORA
       LDA #$AA / STA $5000        ; PIA ORA
       STA $4001                   ; ACIA programmed reset
       LDA #$1F / STA $4003        ; 1 stop, 8 bits, baud generator, 19200
       LDA #$0B / STA $4002        ; no parity, RTS low, TX int off, RX int off, DTR
       LDA #'H' / STA $4000 / JSR DELAY
       LDA #'I' / STA $4000 / JSR DELAY
       LDA #$00 / STA $600B        ; ACR: T1 one-shot
       LDA #$C0 / STA $600E        ; IER: enable T1
       LDA #$E8 / STA $6004 / LDA #$03 / STA $6005   ; T1 = 1000
       CLI
WAIT:  LDA $0200 / BEQ WAIT
       STA $0201
DONE:  BRA DONE
IRQ:   PHA / LDA $6004 / LDA #$01 / STA $0200 / PLA / RTI   ; reading T1C-L clears IFR6
DELAY: LDX #$80
D1:    DEX / BNE D1 / RTS          ; 640 cycles > one 19200-baud character (521 µs)
```

Run 4 ms. Checks: CPU status RUNNING; SRAM 0200 = 01 and 0201 = 01 (the T1
interrupt ran); VIA PB0–PB7 pins = 0x55; PIA PA0–PA7 pins = 0xAA;
`circuit_serial_sent` of the ACIA's channel 0 = `HI`; the ACIA's received
data register holds `I` (the loopback).

## 4. Tests

`serial_test` (fixture: the shipped `W65C51N` device):

| Case | Checks |
|---|---|
| E1 TDRE | status bit 4 = 1 after reset, during and after a frame |
| E2 cut-off | write `A`, then `B` mid-frame: TxD starts `B`'s start bit at the next bit boundary; sent log = `B` |
| E3 no parity | command 0x2B (PME set): 10-bit frames, no parity bit; PE stays 0 |
| E4 TIC 01 | command 0x05: IRQB low at once; a status read clears bit 7 and it is set again at once |
| E5 CTSB | CTSB high mid-frame stops TxD at once (marking) |
| E6 baud | control 0x1F at 1.8432 MHz → 52.08 µs bits; 0x10 → XTLI/16 (115200) |
| E7 terminal | `circuit_serial_send` of `OK` → RDR `O`, then `K`; a sent `A` appears in `circuit_serial_sent` |

`cpu_test`: `io_board` (§3.4).

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — The `w65c51n` variant and the parts

- [ ] `serial_acia6551.c`: the `"w65c51n"` key and §3.1; docs
      `parts-format.md` § `serial` → `acia6551`: the key.
- [ ] `footprints.json` + `LQFP-32-7X7` (§3.3); `serial.json` + `W65C51N`
      (§3.2), 3 parts.
- [ ] `serial_test` E1–E7.
- [ ] Regenerate the goldens. Check: no removed line; + 3, + 3, + 1.
- [ ] Hand-check the `LQFP-32-7X7` line against §3.3 (pad 1 at
      (−4.3, −2.8), pad 9 at (−2.8, 4.3) turned 0.425 × 1.4).

Acceptance: `serial_test` E1–E7 and every plan-12 case `ok`; the golden diff
is exactly the 7 added lines.
  Check: `/tmp/logic-p14/serial_test` → every case `ok`; `git diff --numstat backend/tests/golden` → `3 0 parts.txt`, `3 0 traces.txt`, `1 0 footprints.txt` (est. 3 min).
Commit: —

### Phase 2 — The I/O board

- [ ] `backend/tests/support/io_board.{h,c}` (§3.4), built into
      `logic_test_support`.
- [ ] `cpu_test` case `io_board` with the program and checks of §3.4.
- [ ] `docs/backend.md` § Tests → `cpu_test`: the I/O board.

Acceptance: `cpu_test` → `io_board ok`; `ctest` 100%.
  Check: `/tmp/logic-p14/cpu_test | grep io_board` → `io_board ok` (est. 1 min with the build; the board simulates 4 ms).
Commit: —

## Validation Plan

`serial_test` E1–E7, the golden programs, `cpu_test io_board` (all three
peripherals on a real CPU). The landing tier is plan-14-J's.

## Decisions

- Author: the W65C51N is a variant key of `acia6551`, not a new model: the
  register model is the 6551's, and the four differences are the sheet's
  own (§3.1).
- Author: grade `14` uses the 4 MHz column, the fastest the sheet publishes
  (§3.2).
- Author: a TDR write mid-frame cuts the frame off at the next bit boundary
  and drops it from the sent log (§3.1 item 2); the sheet says only that
  the TSR is loaded on the write.

## Corrections

(none yet)
