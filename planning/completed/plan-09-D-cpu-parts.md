# plan-09-D: CPU parts, footprints, goldens and whole-system tests

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-09-C

This sub-plan ships the parts:

- `Parts/cpu.json` with 3 devices and 19 orderable parts:
  - WDC W65C02S: 2 parts, in production;
  - MOS/CSG NMOS 6502: 4 parts, obsolete;
  - Zilog Z84C00: 13 parts, obsolete.
- 3 new footprints in `footprints.json`, from Zilog's packaging spec
  PS0072.
- Regenerated goldens.
- Whole-system tests: each CPU family runs a program from a real EEPROM
  into a real SRAM through a real SN74HC00N decoder.

Outcome of D:

- `listing_check`, `trace_check` and `footprint_check` match goldens that
  differ from P0/F0 by exactly the 19 new part lines (each golden) and the 3
  new footprint lines.
- `cpu_test` gains three system cases that pass:
  - W65C02S6TPG-14 + AT28C256-15PU + AS6C62256-55PCN + SN74HC00N stores
    `10` at SRAM `0200` and stops;
  - the same with MPS6502;
  - Z84C0010PEG with the same chips stores `10` at `8200` and halts.

References:

- plan-09-A (datasheets, § Decisions), plan-09-B §4.1 and plan-09-C §4.1
  (keys), plan-07-A §4.1 (`vcc`).
- `docs/parts-format.md` § Footprints (rules R1–R6 in
  `planning/completed/plan-03-B-logic-footprints.md` §3), `docs/backend.md`
  § Tests (golden regeneration).
- `datasheets/cpu/w65c02s.pdf` p.12–14, 24–25, 32;
  `mos_6500_mpu_nov_1985.pdf` p.1, 5–6, 10; `ps0178.pdf` PDF p.1–2,
  printed p.35–36; `ps0072.pdf` printed p.6, 10, 16.

## Prerequisites

plan-09-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-09-C complete | `ls planning/plan-09-C-* 2>/dev/null` → no output; `ls planning/completed/plan-09-C-*.md` → exists | MET (2026-11-01 at f3e6366; plan-09-A's rows re-run at 1da038c, all MET) |
| The system-test parts exist | `grep -c '"AT28C256-15PU"\|"AS6C62256-55PCN"\|"SN74HC00N"' schematical/schematical/Parts/{eeprom,sram,gate}.json` → 1 each | MET (2026-11-01 at f3e6366: 1, 1, 1) |
| No footprint ids taken | `grep -c '"PDIP-40-600"\|"PLCC-44"\|"LQFP-44-10X10"' schematical/schematical/Parts/footprints.json` → 0 | MET (2026-11-01 at f3e6366: 0) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of *all*
> prerequisites.**

## 1. Goal

- `grep -c '"pinout":' schematical/schematical/Parts/cpu.json` → 19.
- `wc -l < backend/tests/golden/parts.txt` → P0 + 19, `traces.txt` → P0 +
  19, `footprints.txt` → F0 + 3. `git diff backend/tests/golden` shows added
  lines only.
- `grep -c '|cpu|.*obsolete' backend/tests/golden/parts.txt` → 17.
- `ctest` → T0 + 2 tests, all passing, with `cpu_test` including §5.

### Non-goals (explicit constraints)

- No existing part, footprint or golden line changes.
- No W65C02S6TQG-14 (QFP-44): left out by the owner; WDC publishes no
  outline.
- No ceramic NMOS 6502 parts (MCS6502, `C` package code). The sheet gives
  no package drawing, and PS0072 has no ceramic DIP. Only the plastic `MPS`
  parts are added.
- Of the Z84C00 codes, only the 13 that PCN ZAC24-0029 lists as the
  production line at EOL. The `…EC` (leaded) and `…F…` (QFP) codes ended
  earlier, and PS0072 has no 44-QFP outline.
- No app change (E).

## 2. Current State

- `footprints.json`: F0 footprints (140 at 5992f1c). None has 40 or 44 DIP
  or quad pins: `grep -o '"pins":4[04]' footprints.json` matches only
  `SOJ-44-400` and `TSOP2-44-400`, which are dual footprints with other
  pitches.
- The PLCC-32 footprint (`footprints.json:136`) sets the precedent for pin 1
  at the top centre: `quad`, `sides`, and a `slots` list that puts pin 1 on
  the middle top slot.
- Plan-07 is complete (prerequisite): every non-supply part needs `"vcc"`.
  `cpu` grades take `vcc` like `sram` grades (plan-09-B §4.1).

### Measured populations

| What | Count | Source |
|---|---|---|
| Parts | W65C02S 2, NMOS 6502 4, Z84C00 13 = 19 | §4.2 |
| Parts flagged obsolete | 17 (4 + 13) | §4.2 |
| Parts per footprint | PDIP-40-600 9 (W65C02S6TPG-14, 4 MPS6502, 4 Z84C00…PEG); PLCC-44 6 (W65C02S6TPLG-14, 5 Z84C00…VEG incl. `Z84C0010VEG00TR`); LQFP-44-10X10 4 (Z84C00…AEG) | §4.2 |
| New footprints | 3 | §4.1 |

### Verified properties

- **Pinouts** were read from the datasheet figures:
  - W65C02S: p.13 (PDIP-40, PLCC-44);
  - NMOS 6502: MOS p.10;
  - Z84C00: PS0178 PDF p.1 (DIP) and p.2 (LQFP, PLCC; the PLCC and LQFP
    tables below were checked against the PDF p.2 drawing on 2026-11-01).
- **Package codes.**
  - Zilog: P = 40-pin PDIP, A = 44-pin LQFP, V = 44-pin PLCC. Taken from
    Zilog's own ordering table in PS0199 (Z8F6421**PM** "40-pin PDIP",
    **AN** "44-pin LQFP", **VN** "44-pin PLCC"); the same scheme is used
    across Zilog.
  - MOS: the order code `M{C|P}S65{XX}{suffix}` (p.1): C ceramic, P
    plastic; suffix none/A/B/C = 1/2/3/4 MHz.
  - WDC: `…TPG-14` PDIP-40, `…TPLG-14` PLCC-44 (p.32, wdc65xx.com chip
    page).
- **Outlines** (PS0072, read 2026-11-01):
  - 40-PDIP MKT71C0005 rev N: E1 13.59–14.22, D 52.07–52.58, B
    0.38–0.53, e 2.54, row 15.24.
  - 44-PLCC MKT71C1001 rev O: D/E 17.40–17.65 tip to tip, D1/E1
    16.51–16.66 body, lead width 0.36–0.51, e 1.27, pin 1 top centre with
    7–17 down the left side.
  - 44-LQFP MKT71C1182 rev F: D/E 9.90–10.10, HD/HE 11.75–12.25, b
    0.30–0.45, L 0.45–0.75, e 0.80.
- **Supply ranges:**
  - W65C02S: VDD 1.71–5.25 V across its voltage columns (p.24). The 14 MHz
    timing is the 5.0 V ± 5 % column.
  - NMOS 6502: 5 V ± 5 % = 4.75–5.25 (p.5).
  - Z84C00: 5 V ± 10 % = 4.5–5.5 (PS0178 printed p.35).

## 3. Design Overview

Data entry plus one test file:

1. Footprints, from PS0072 drawings with the R-rules.
2. `cpu.json`, from the tables below.
3. Goldens regenerated and reviewed.
4. System tests.

Correctness risk:

- a pinout typo, which the system tests catch for the PDIP parts and a
  hand check of the PLCC/LQFP tables against the figures catches for the
  rest;
- footprint numbers, hand-checked as plan-03 did.

Gate class: behaviour changes by design (new parts). Only added golden
lines are expected; any changed existing line is a bug.

Rejected alternatives:

- **Footprints from a generic JEDEC table, or another vendor's PIC
  datasheet.** Zilog's own PS0072 covers all three packages. The WDC parts
  use the same JEDEC outlines: a 600 mil PDIP-40 and a 44-PLCC with 1.27 mm
  pitch.
- **One device per speed grade.** Grades already carry timing, so one
  device per chip.

## 4. Detailed Design

### 4.1 Footprints (`footprints.json`, appended)

| id | style | Numbers (R-rule) | note |
|---|---|---|---|
| `PDIP-40-600` | dual, pins 40, pitch 2.54, span 15.24 | R5: drill 0.53 + 0.25 = 0.78 → 0.80; round pad 1.5; `pin1Shape` rect; body 13.91 × 52.33 (nominal E1 × D) | `"Zilog MKT71C0005 40-lead PDIP (R5; B 0.53 max), cpu/ps0072.pdf p.6"` |
| `PLCC-44` | quad, pins 44, sides [11,11,11,11], pitch 1.27, spanX = spanY = 16.525 | R3: pad 2.0 × 0.61 rect (b 0.51 + 0.1), span = E nom 17.525 − 1.0; body 16.59 × 16.59; `slots` = [39,40,41,42,43,44,1,2,…,38] (pin 1 → top-centre slot 39, pin 7 → slot 1) | `"Zilog MKT71C1001 44-lead PLCC (R3), cpu/ps0072.pdf p.10"` |
| `LQFP-44-10X10` | quad, pins 44, sides [11,11,11,11], pitch 0.8, spanX = spanY = 11.70 | R2: pad 1.55 × 0.50 rect (L 0.75 + 0.8, b 0.45 + 0.05), span = HE 12.25 + 1.0 − 1.55; body 10 × 10; default slots (pin 1 at the top of the left side) | `"Zilog MKT71C1182 44-lead LQFP 10x10x1.4 (R2), cpu/ps0072.pdf p.16"` |

Descriptions follow the existing pattern, e.g. `"44-pin PLCC, 16.59 x 16.59
mm body, 1.27 mm pitch, pin 1 top centre"`.

### 4.2 `Parts/cpu.json`

`"kind": "cpu"`. Every device has `"drawer": ["Processor"]`. Address pins
use group `"address"`, data pins group `"data"`. Names ending `#` (and the
WDC `…B` names) are active low.

**Device `W65C02S`.**
- function `"65C02"`; description `"8-bit CMOS microprocessor (WDC
  W65C02S)"`; core `"65c02"`.
- Keys: clock `PHI2`; rw `RWB`; sync `SYNC`; irq `IRQB`; nmi `NMIB`; reset
  `RESB`; so `SOB`; rdy `RDY`; phi1Out `PHI1O`; phi2Out `PHI2O`;
  vectorPull `VPB`; memoryLock `MLB`; busEnable `BE`.
- Grade `"14"` (5.0 V column, p.25): tcyc 70, tpwh 35, tpwl 35, tads 30,
  tah 10, tsync 30, tmds 25, tdhw 10, tdsr 10, tpcs 10, tbvd 25; `"vcc":
  [1.71, 5.25]`.
- Pinout `p40`:
  - 1 VPB (output), 2 RDY (inout), 3 PHI1O (output), 4 IRQB (input),
    5 MLB (output), 6 NMIB (input), 7 SYNC (output), 8 VDD (power);
  - 9–20 A0–A11 (output), 21 VSS (ground), 22–25 A12–A15 (output);
  - 26 D7, 27 D6, 28 D5, 29 D4, 30 D3, 31 D2, 32 D1, 33 D0 (inout);
  - 34 RWB (output), 35 NC (nc), 36 BE (input), 37 PHI2 (input), 38 SOB
    (input), 39 PHI2O (output), 40 RESB (input).
- Pinout `plcc44`:
  - 1 VSS, 2 VPB, 3 RDY, 4 PHI1O, 5 IRQB, 6 MLB, 7 NMIB, 8 SYNC, 9 VDD;
  - 10 A0, 11 A1, 12 NC, 13–17 A2–A6, 18–22 A7–A11, 23 VSS, 24 VSS,
    25–28 A12–A15;
  - 29 D7 … 36 D0;
  - 37 VDD, 38 RWB, 39 NC, 40 BE, 41 PHI2, 42 SOB, 43 PHI2O, 44 RESB.
  - Roles as for `p40`.
- Parts:
  - `W65C02S6TPG-14` (package `"PDIP"`, `p40`, `PDIP-40-600`, grade `14`);
  - `W65C02S6TPLG-14` (`"PLCC"`, `plcc44`, `PLCC-44`, grade `14`).

**Device `MCS6502`.**
- function `"6502"`; description `"8-bit NMOS microprocessor (MOS 6502)"`;
  core `"6502"`.
- Keys: clock `PHI0IN`; rw `R/W`; sync `SYNC`; irq `IRQ#`; nmi `NMI#`; reset
  `RES#`; so `SO#`; rdy `RDY`; phi1Out `PHI1OUT`; phi2Out `PHI2OUT`.
- Grades (p.5–6; `vcc` [4.75, 5.25] each). Columns, in order: tcyc, tpwh,
  tpwl, tads, tah, tsync, tmds, tdhw, tdsr, tpcs, tbvd.

  | grade | tcyc | tpwh | tpwl | tads | tah | tsync | tmds | tdhw | tdsr | tpcs | tbvd |
  |---|---|---|---|---|---|---|---|---|---|---|---|
  | `1` | 1000 | 460 | 480 | 300 | 30 | 360 | 200 | 30 | 100 | 100 | 0 |
  | `A` | 500 | 240 | 240 | 150 | 30 | 175 | 100 | 30 | 50 | 50 | 0 |
  | `B` | 333 | 160 | 163 | 125 | 10 | 120 | 100 | 10 | 50 | 50 | 0 |
  | `C` | 250 | 123 | 123 | 85 | 10 | 100 | 90 | 10 | 40 | 40 | 0 |

  - `tpwl` = tCYC − PWHΦ0 max (1000 − 520, 500 − 260, 333 − 170,
    250 − 127).
  - `tpcs` = tRDY min at 1/2 MHz, and TS.O. min at 3/4 MHz. The 3/4 MHz
    tRDY column prints only a max of 15, a column error.
  - The 3 MHz PWHΦ0 prints min 180 > max 170. Grade `B` uses tpwh 160, the
    6512's 3 MHz Φ2 width on the same page; each grade's `note` says so.
- Pinout `p40`:
  - 1 VSS (ground), 2 RDY (input), 3 PHI1OUT (output), 4 IRQ# (input),
    5 NC (nc), 6 NMI# (input), 7 SYNC (output), 8 VCC (power);
  - 9–20 A0–A11, 21 VSS (ground), 22–25 A12–A15;
  - 26 D7 … 33 D0;
  - 34 R/W (output), 35 NC, 36 NC, 37 PHI0IN (input), 38 SO# (input),
    39 PHI2OUT (output), 40 RES# (input).
- Parts: `MPS6502`, `MPS6502A`, `MPS6502B`, `MPS6502C`. Each has package
  `"PDIP"`, `p40`, `PDIP-40-600`, grade `1`/`A`/`B`/`C`, and `"flags":
  ["obsolete"]`.

**Device `Z84C00`.**
- function `"Z80"`; description `"8-bit CMOS microprocessor (Zilog Z80)"`;
  core `"z80"`.
- Keys: clock `CLK`; m1 `M1#`; mreq `MREQ#`; iorq `IORQ#`; rd `RD#`; wr
  `WR#`; rfsh `RFSH#`; halt `HALT#`; busack `BUSACK#`; wait `WAIT#`; int
  `INT#`; nmi `NMI#`; reset `RESET#`; busreq `BUSREQ#`.
- Grades `6`, `8`, `10`, `20` with plan-09-C §4.2's values; `vcc` [4.5,
  5.5].
- Pinout `p40`:
  - 1–5 A11–A15, 6 CLK;
  - 7 D4, 8 D3, 9 D5, 10 D6, 11 +5V (power), 12 D2, 13 D7, 14 D0, 15 D1;
  - 16 INT#, 17 NMI#, 18 HALT#, 19 MREQ#, 20 IORQ#, 21 RD#, 22 WR#,
    23 BUSACK#, 24 WAIT#, 25 BUSREQ#, 26 RESET#, 27 M1#, 28 RFSH#;
  - 29 GND (ground), 30–40 A0–A10.
- Pinout `plcc44`:
  - 1–5 A11–A15, 6 NC, 7 CLK;
  - 8 D4, 9 D3, 10 D5, 11 D6, 12 NC, 13 +5V, 14 D2, 15 D7, 16 D0, 17 D1;
  - 18 INT#, 19 NMI#, 20 HALT#, 21 MREQ#, 22 IORQ#, 23 RD#, 24 NC, 25 NC,
    26 WR#, 27 BUSACK#, 28 WAIT#, 29 BUSREQ#, 30 RESET#, 31 M1#, 32 RFSH#;
  - 33 GND, 34–44 A0–A10.
- Pinout `lqfp44`:
  - 1 CLK, 2 D4, 3 D3, 4 D5, 5 D6, 6 +5V, 7 D2, 8 D7, 9 D0, 10 D1, 11 NC;
  - 12 INT#, 13 NMI#, 14 HALT#, 15 MREQ#, 16 IORQ#, 17 NC, 18 RD#, 19 WR#,
    20 BUSACK#, 21 WAIT#, 22 BUSREQ#, 23 RESET#, 24 M1#, 25 RFSH#;
  - 26 GND, 27–32 A0–A5, 33 NC, 34–38 A6–A10, 39 NC, 40–44 A11–A15.
- Roles: INT#, NMI#, WAIT#, BUSREQ#, RESET# and CLK are inputs. M1#,
  MREQ#, IORQ#, RD#, WR#, RFSH#, HALT#, BUSACK# and A0–A15 are outputs.
  D0–D7 are inout.
- Parts (13, all `"flags": ["obsolete"]`, grade = the speed digits):
  - `Z84C0006PEG`, `Z84C0008PEG`, `Z84C0010PEG`, `Z84C0020PEG`: package
    `"PDIP"`, `p40`, `PDIP-40-600`.
  - `Z84C0006VEG`, `Z84C0008VEG`, `Z84C0010VEG`, `Z84C0010VEG00TR`,
    `Z84C0020VEG`: package `"PLCC"`, `plcc44`, `PLCC-44`.
  - `Z84C0006AEG`, `Z84C0008AEG`, `Z84C0010AEG`, `Z84C0020AEG`: package
    `"LQFP"`, `lqfp44`, `LQFP-44-10X10`.

Device `note`s name the PDF and page of each fact, as in `eeprom.json`.

## 5. Whole-system tests (`cpu_test`, appended)

Engine-level, like `eeprom_test`'s real-part cases:
- instances of library parts through `kind_model(part)`;
- a bench component for the clock (1 MHz, 50 %) and RESET (low for 4
  cycles at start);
- the program loaded into the EEPROM with its `memory_load` hook.

There is no `circuit_start`, so plan-07's voltage check is not involved.
Unconnected inputs read inactive (Z).

**6502 board** (W65C02S6TPG-14, then MPS6502):
- AT28C256-15PU at `8000`–`FFFF`:
  - A0–A14 and I/O0–7 to the CPU buses;
  - CE# = NAND(A15, A15) (SN74HC00N gate 3);
  - OE# = NAND(RWB, RWB) (gate 1);
  - WE# tied to a 1 supply.
- AS6C62256-55PCN at `0000`–`7FFF`:
  - CE# = A15; OE# = gate 1's output;
  - WE# = NAND(PHI2, gate 1's output) (gate 2): low only while PHI2 is high
    on a write.
- EEPROM contents:
  - offset 0 (`8000`):
    `A2 FF 9A A9 00 85 10 E6 10 A5 10 8D 00 02 C9 10 D0 F5` and then `DB`
    (STP) for the W65C02S, or `4C 12 80` (JMP $8012) for the MPS6502;
  - offset `7FFC`: `00 80`.
- After 2 ms (1 MHz):
  - SRAM `0010` = `10`; SRAM `0200` = `10`;
  - CPU A = `10`, X = `FF`, S = `FF`;
  - status STOPPED (W65C02S) or RUNNING with PC in `8012`–`8014`
    (MPS6502).

**Z80 board** (Z84C0010PEG):
- AT28C256-15PU at `0000`–`7FFF`; AS6C62256-55PCN at `8000`–`FFFF`.
- SN74HC00N:
  - gate 1 = NAND(MREQ#, MREQ#) (= MREQ);
  - gate 2 = NAND(A15, A15);
  - gate 3 = NAND(gate 1, gate 2) → EEPROM CE#;
  - gate 4 = NAND(gate 1, A15) → SRAM CE#.
- OE# of both = RD#; SRAM WE# = WR#; EEPROM WE# tied high.
- EEPROM at 0:
  `31 00 00 3E 00 3C 32 00 82 FE 10 20 F8 CD 20 00 76`; at `0020`:
  `32 01 82 C9`.
- After 2 ms:
  - SRAM `0200` = `10` and `0201` = `10` (the CALLed store);
  - SRAM `7FFE`–`7FFF` = `10 00` (the return address pushed at
    `FFFE`/`FFFF`);
  - status HALTED; A = `10`;
  - the last instruction `HALT`.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes** — never in a
>   batch at the end of a phase, never "I'll update the plan after". If it is a
>   separate step it will be skipped, and the list becomes a lie.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>` rather than
>   deleting it. A vanished task is indistinguishable from a forgotten one.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed; an unlisted task that got done is a
>   task the next reader will do twice.
>
> **An unticked box means NOT DONE.** Someone reading this file — including the
> person who has to decide whether to stop — is entitled to trust that.

### Phase 1 — Footprints and parts

- [x] `footprints.json`: the 3 footprints of §4.1. `footprint_check` →
      163 footprints, passes after regeneration.
- [x] `Parts/cpu.json`: §4.2. `grep -c '"pinout":'` → 19;
      `library_check` passes.
- [x] Hand-check every pin of the `plcc44` and `lqfp44` pinouts against
      W65C02S p.13 and PS0178 PDF p.2, and every new `footprints.txt` line
      against its PS0072 page. Record "checked" per table in the commit
      message. Done from the rendered pages: W65C02S p.13 (p40, plcc44),
      MOS p.10 (p40), PS0178 PDF p.1 (p40) and p.2 (lqfp44, plcc44), PS0072
      printed p.6/10/16 = PDF p.9/13/19 (see Corrections). The PLCC-44 line
      puts pin 1 at (0, −8.262) top centre, pin 7 at the top of the left
      side, 18–28 along the bottom.
- [x] Regenerate `parts.txt`, `traces.txt` and `footprints.txt`
      (`docs/backend.md` § Tests). The golden numstat → `3 0
      footprints.txt`, `19 0 parts.txt`, `19 0 traces.txt`; `wc -l` → 6189
      (P0 + 19) and 163 (F0 + 3); `grep -c '|cpu|.*obsolete' parts.txt` →
      17.
- [x] `docs/parts-format.md`: `cpu.json` in the file table; the example
      device in § `cpu` points at `W65C02S`.

Acceptance: the library loads with the 19 parts, and the goldens grow by
exactly 19 / 19 / 3 added lines.
  Check: `ctest --test-dir /tmp/logic-p09 -R 'listing_check|trace_check|footprint_check|library_check'` → 4/4 passed; `git diff --numstat backend/tests/golden` → `19 0 …parts.txt`, `19 0 …traces.txt`, `3 0 …footprints.txt` (est. 2 min).
Commit: c9da192

### Phase 2 — Whole-system tests and the speed record

- [x] `cpu_test.c`: the three boards of §5 (built by the new
      `tests/support/cpu_board.{h,c}`, shared with `cpu_bench`). All 9
      board checks PASS: SRAM contents, registers, and status for each
      board.
- [x] `backend/tests/cpu_bench.c` (not a ctest, like `sim_bench`): the
      W65C02S board of §5 looping forever (`4C 07 80` in place of the STP).
      It runs N simulated ms and prints `×real time`. Add it to
      `backend/CMakeLists.txt` and `docs/backend.md`.

Acceptance: the three boards reach their end states.
  Check: `ctest --test-dir /tmp/logic-p09 -R cpu_test -V | grep -c ': FAIL'` → 0 (est. 1 min). Measured: 0. Record (no threshold): `cmake -S backend -B /tmp/logic-rel -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-rel --target cpu_bench && /tmp/logic-rel/cpu_bench 100` → `0.12x real time` (2026-11-01: 100 ms simulated in 0.843 s, 99995 cycles).
Commit: 3f08307

## Validation Plan

- Tests: `cpu_test` §5; the golden checks; `footprint_check`, which also
  fails on an unused footprint, so all 3 must be used.
- Coverage check: each new part is in `listing_check` and `trace_check` (19
  lines each). The system boards cover one PDIP part per family through
  real memories.
- Runtime proof: the §5 boards.
- Doc sync: `docs/parts-format.md`, `docs/backend.md` (`cpu_bench`).
- Final gate (once, after Phase 2):
  `rm -rf /tmp/logic-land-p09d && cmake -S backend -B /tmp/logic-land-p09d && cmake --build /tmp/logic-land-p09d -j8 && ctest --test-dir /tmp/logic-land-p09d`
  → `100% tests passed`, T0 + 2 tests (est. 2 min); `xcodebuild …` →
  `** BUILD SUCCEEDED **` (est. 2 min; `cpu.json` is bundled through the
  synchronized `Parts/` folder).

## Decisions

- **The 19 parts are added although DigiKey has none in stock.** AGENTS.md
  gained "add only parts that are in stock" (commit 69dda95, 2026-11-01)
  after this plan was written. Measured 2026-11-01 with
  `node scripts/digikey.js`: the 13 Z84C00 codes are Obsolete with no stock,
  and DigiKey lists no W65C02S or MPS6502 chips (only WDC boards). Asked the
  owner the same day, who chose "add all 19 as planned". It extends plan-09-A's
  obsolete exception to stock. AGENTS.md is unchanged.

Owner: plan-09-A § Decisions (obsolete exception, no WDC QFP). Author:
§ Non-goals (plastic NMOS only; the PCN's 13 Z84C00 codes) and § 3.

## Corrections

- **Package `"DIP"`, not `"PDIP"`** (§4.2). The library calls a plastic DIP
  `"DIP"`: 1654 parts use `"package":"DIP"` and none `"PDIP"`. The app
  shows the string and groups by it. The PDIP parts use `"DIP"`; `"PLCC"`
  (38 existing parts) and `"LQFP"` (new) are as planned.
- **PS0072 page numbers** are printed numbers. The PDF pages are +3:
  printed p.6/10/16 = PDF p.9/13/19 (PDF p.6 is the 18-lead PDIP). The
  footprint notes keep the printed numbers.
- **MPS6502 board end state** (§5). "PC in `8012`–`8014`" was too narrow.
  The core's PC is the next byte to read. In the `JMP $8012` loop it runs
  8012 (after the jump), 8013 (after the fetch), 8014, then 8015 (after the
  high byte). Measured at 2 ms: PC 8015, status RUNNING. The check is now
  8012–8015, plus the stronger last instruction `JMP $8012` at 8012. The
  W65C02S board also checks last instruction `STP`.
- **The boards' WE#/address margin.** On the 6502 board the SRAM's write
  ends when WE# = NAND(PHI2, gate 1) rises, 9 ns (HC00) after PHI2 falls.
  The W65C02S address goes X at fall + tAH = 10 ns, and the AS6C62256 takes
  address and data at the write's end (`twr` 0). So the write ends 1 ns
  inside the hold. The board passes (SRAM 0010/0200 = 10). This is noted
  because a slower gate family on that board would store X, as on the real
  board.
- **LQFP-44 orientation.** PS0072's outline draws pin 1 at the left end
  of the bottom row. PS0178's pin figure (PDF p.2) puts pin 1 at the top of
  the left side. Both run counter-clockwise, and a footprint may be placed
  rotated. The default slots (pin 1 at the top of the left side) match
  PS0178.

## Summary

D is data entry with a strong net: three whole-system programs and the
golden diffs. The obsolete NMOS and Z80 parts are hidden in the app's
drawer until Settings → "Obsolete parts" is on. That is existing behaviour
for every obsolete part, and E says so in its runtime proof.
