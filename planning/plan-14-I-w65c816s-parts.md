# plan-14-I: The W65C816S parts and the W65C816S board

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-14-H

I ships the `W65C816S` device in `cpu.json` with its 2 parts, and proves it
on a whole board of library parts: a W65C816S, an SN74HC573AN bank latch, an
AT28C256 and a 128 KB AS6C1008 SRAM, running a native-mode program that
writes to bank 0 and bank 1 and reads both back.

Outcome of I: `cpu.json` gains device `W65C816S` (2 parts); `parts.txt` and
`traces.txt` gain 2 lines each, `footprints.txt` none; `cpu_test` case
`board_w65c816s` passes.

References: `datasheets/cpu/w65c816s.pdf` — pinouts p.10, Table 4-2 p.27,
Figure 5-1 (bank address latch) p.44, ordering p.55;
`backend/tests/support/cpu_board.{h,c}` (plan-09-D boards); plan-14-H §3
(the adapter and its keys).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-H landed | `/tmp/logic-p14/cpu_test \| grep -c '^ok w65c816s'` → 13 | NOT MET (2026-11-01) |

## 1. Goal

- `cpu_test` → `board_w65c816s ok` and every earlier case `ok`.
- Goldens: + 2 `parts.txt`, + 2 `traces.txt`; no removed or changed line.

### Non-goals

- No W65C816S6TQG-14 (no WDC QFP outline; `datasheets/skipped-parts.md`).
- No app change (J).

## 2. Current State

After H: core `65816` exists; `cpu_test` runs it on an inline device.
`cpu.json` has `W65C02S`, `MCS6502`, `Z84C00`. `cpu_board.{h,c}` builds the
6502-family and Z80 boards with an AS6C62256 (32 KB) and reads it with
`cpu_board_sram(board, 0..7FFF)`.

## 3. Design

### 3.1 The device and parts

`cpu.json` + device (keys and grade exactly plan-14-H §3.1):

```json
{"id": "W65C816S", "function": "65C816", "drawer": ["Processor"],
 "description": "8/16-bit CMOS microprocessor (WDC W65C816S)",
 "note": "W65C816S (datasheets/cpu/w65c816s.pdf): pin functions p.12-14, pinouts p.10 (PDIP-40, PLCC-44), AC characteristics p.27, timing diagram p.28, bank latch p.44, ordering p.55",
 "core": "65816", … , "pinouts": {"p40": […], "plcc44": […]}, "parts": [ … ]}
```

Pinouts (p.10):

- `p40`: 1 VPB, 2 RDY, 3 ABORTB (printed "ABORT"), 4 IRQB, 5 MLB, 6 NMIB,
  7 VPA, 8 VDD, 9–20 A0–A11, 21 VSS, 22–25 A12–A15, 26 D7, 27 D6, 28 D5,
  29 D4, 30 D3, 31 D2, 32 D1, 33 D0, 34 RWB, 35 E, 36 BE, 37 PHI2, 38 MX,
  39 VDA, 40 RESB.
- `plcc44`: 1 VSS, 2 VPB, 3 RDY, 4 ABORTB, 5 IRQB, 6 MLB, 7 NMIB, 8 VPA,
  9 VDD, 10 A0, 11 A1, 12 NC, 13–22 A2–A11, 23 VSS, 24 VSS, 25–28 A12–A15,
  29 D7, 30 D6, 31 D5, 32 D4, 33 D3, 34 D2, 35 D1, 36 D0, 37 VDD, 38 RWB,
  39 E, 40 BE, 41 PHI2, 42 MX, 43 VDA, 44 RESB.

Roles: A0–A15 outputs, group `address`; D0–D7 `inout`, group `data`; RDY
`inout`; RWB, VDA, VPA, VPB, MLB, E, MX outputs; the rest inputs, power,
ground, nc (as the W65C02S's in `cpu.json`).

| Part | package | pinout | footprint | grade |
|---|---|---|---|---|
| W65C816S6TPG-14 | DIP | p40 | PDIP-40-600 | 14 |
| W65C816S6TPLG-14 | PLCC | plcc44 | PLCC-44 | 14 |

(WDC's where-to-buy table lists the DIP as `W65C816S6TPG-14`; its Mouser
link text says `W65C816S6PG-14`. The ordering page (p.55) gives `6T` as the
process code for every package, so the part name is `W65C816S6TPG-14`.)

### 3.2 The board (`cpu_board` gains a `65816` layout)

Library parts on the engine, `5V` supply: W65C816S6TPG-14, SN74HC573AN,
AT28C256-15PU, AS6C1008-55PCN, two SN74HC00N. PHI2 1 MHz 50 %, RESB low for
the first 4 cycles (the bench of `cpu_board`).

| Net / gate | Function |
|---|---|
| g1 = NAND(A15, A15) | EEPROM CE# (A15 = 1, every bank) |
| A15 | SRAM CE# (A15 = 0) |
| g2 = NAND(PHI2, RWB) | EEPROM and SRAM OE#: low only while PHI2 is high on a read, so no memory drives D0–D7 while the CPU puts the bank there |
| g3 = NAND(RWB, RWB) | ¬RWB |
| g4 = NAND(PHI2, g3) | SRAM WE# |
| g5 = NAND(PHI2, PHI2) | latch LE: transparent while PHI2 is low, holds from the rise (Figure 5-1, §7.6) |
| SN74HC573AN | 1D–8D = D0–D7, OE# = GND; 1Q = BA0 |
| SRAM | A0–A14 = CPU A0–A14, A15 = BA0, A16 = GND (banks 0 and 1 each see 32 KB at 0000–7FFF) |
| EEPROM | A0–A14 = CPU A0–A14, WE# = 1 |
| CPU | IRQB, NMIB, ABORTB, RDY, BE = 1 |

`cpu_board_sram` takes chip addresses up to the SRAM's size (0–1FFFF for
the AS6C1008).

Program (EEPROM offset 0 = CPU 8000; reset vector FFFC = `00 80`):

```
8000  18           CLC
8001  FB           XCE                  ; native mode
8002  E2 30        SEP #$30             ; 8-bit A, X, Y
8004  A9 11        LDA #$11
8006  8F 00 01 00  STA $000100
800A  A9 22        LDA #$22
800C  8F 00 01 01  STA $010100
8010  AF 00 01 00  LDA $000100
8014  8F 00 02 00  STA $000200
8018  AF 00 01 01  LDA $010100
801C  8F 01 02 00  STA $000201
8020  C2 20        REP #$20             ; 16-bit A
8022  A9 44 33     LDA #$3344
8025  8F 02 02 00  STA $000202
8029  80 FE        BRA $8029
```

Run 2 ms. Checks: status RUNNING; SRAM chip 00100 = 11, 08100 = 22 (bank 1
→ A15 = 1), 00200 = 11, 00201 = 22, 00202 = 44, 00203 = 33; registers E = 0,
PBR = 00, PC = 8029 or 802A (inside the loop), C = 3344.

## 4. Tests

- `cpu_test` `board_w65c816s` (§3.2).
- Golden lines for the 2 parts (the generic trace stimulus runs every part).

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — Parts

- [ ] `cpu.json` + `W65C816S` (§3.1), 2 parts; `cpu_test` H-cases switch from
      the inline device to the shipped one.
- [ ] Regenerate the goldens. Check: no removed line; + 2, + 2, + 0.
- [ ] Spot-check `W65C816S6TPLG-14`: pin 12 `NC`, pins 23–24 `VSS`,
      `|core=65816 grade=14 max_hz=14285714`, `|footprint=PLCC-44`.
- [ ] `docs/parts-format.md` § `cpu`: "`cpu.json` holds four devices …
      `W65C816S` (2 parts)".

Acceptance: `ctest` 100%; the golden diff is exactly the 4 added lines.
  Check: `ctest --test-dir /tmp/logic-p14` → `100% tests passed`; `git diff --numstat backend/tests/golden` → `2 0 parts.txt`, `2 0 traces.txt` (est. 2 min).
Commit: —

### Phase 2 — The board

- [ ] `cpu_board.{h,c}`: the `65816` layout (§3.2); `cpu_board_sram` range.
- [ ] `cpu_test` `board_w65c816s` with the §3.2 program and checks.
- [ ] `docs/backend.md` § Tests → `cpu_test`: the W65C816S board.

Acceptance: `board_w65c816s ok`; the existing board cases still `ok`.
  Check: `/tmp/logic-p14/cpu_test | grep board_` → every board `ok` (est. 1 min with the build).
Commit: —

## Validation Plan

`cpu_test board_w65c816s` (the chip, its bank multiplexing and a real latch
on library parts) and the golden programs. The landing tier is plan-14-J's.

## Decisions

- Author: the part name is `W65C816S6TPG-14` (§3.1).
- Author: the board gates memory OE# with PHI2 so no memory drives the data
  bus during the bank phase (§3.2) — the board a real W65C816S needs.

## Corrections

(none yet)
