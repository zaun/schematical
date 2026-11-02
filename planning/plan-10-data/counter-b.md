# cntB — ripple counters and counters with registers

Paths below are relative to `datasheets/`. Page numbers are PDF page numbers.

Several of the old TI LS sheets have **scanned-image** pages, so pdftotext returns no text for them. These pages were rendered and read visually:
- sn74ls90 pp. 1–11
- sn74ls293 pp. 1–9
- sn74ls390 pp. 1–9
- sn74ls590 pp. 1–4
- sn74ls592 pp. 1–8
- sn74ls697 pp. 1–10
- cd4020b pp. 1–4

All LS switching tables were measured at CL = 15 pF (counters), 30 pF (RCO) or 45 pF (3-state Q), as noted per row. The LS sheets have no 50 pF condition.

## PDF → devices

| PDF | devices |
|---|---|
| counters/sn74ls90.pdf (SDLS940A) | 74_90, 74_92, 74_93. Also covers TTL '90A/'92A/'93A, which have no census rows. |
| counters/sn74ls293.pdf (SDLS097) | 74_293. Also covers '290/'LS290 and TTL '293, which have no census rows. |
| counters/sn74ls390.pdf (SDLS107) | 74_390 (LS), 74_393 (LS). Also covers TTL 54/74390 and 54/74393, which have no census rows. |
| counters/cd74hc390.pdf | 74_390 (HC, HCT) |
| counters/cd74hc393.pdf | 74_393 (HC, HCT) |
| counters/sn74hc393.pdf | 74_393 (HC). The addendum also lists SN74HCS393DYYR, which is not in the census. |
| counters/sn74lv393a.pdf | 74_393 (LV) |
| counters/sn74lv393a-q1.pdf | 74_393 (LV, -Q1) |
| counters/cd4020b.pdf | 4020B, 4024B, 4040B |
| counters/cd74hc4020.pdf | 74_4020 (HC, HCT) |
| counters/sn74hc4020.pdf | 74_4020 (HC) |
| counters/cd74hc4024.pdf | 74_4024 (HC, HCT) |
| counters/cd74hc4040.pdf | 74_4040 (HC, HCT) |
| counters/scls160e.pdf | 74_4040 (HC; the SN54/74HC4040 sheet) |
| counters/sn74lv4040a.pdf | 74_4040 (LV) |
| counters/sn74ls590.pdf (SDLS003) | 74_590 (LS). Also covers LS591 (open-collector), which has no census rows. |
| counters/sn74hc590a.pdf | 74_590 (HC) |
| counters/sn74ls592.pdf (SDLS004) | 74_592, 74_593 |
| counters/sn74ls697.pdf (SDLS199) | 74_697. Also covers LS696/LS699, which have no census rows; the LS698 logic diagram is printed too. |
| counters/sn74lv8154.pdf | 74_8154 |

**Census rows that could not be mapped exactly.** All rows are mapped. Three mappings are inferred, because the PDF has no cross-reference table:
- **JM38510/31501BCA and M38510/31501BCA → LS90.** JM38510/31502BCA/BDA and M38510/31502BCA/BDA → LS93. Reasoning:
  - The addendum's "Military" list names only SN54LS90 and SN54LS93.
  - 31502 has a CFP (W) row, like the LS93 SMD 7700101DA. The LS90 has no W orderable.
- **JM38510/32701 → LS390** and **JM38510/32702 → LS393.** The 32702SDA mark contains "SNV54LS393W". Pin counts also fit: 32701 is 16-pin and 32702 is 14-pin.
- **JM38510/05653 → 4020B** and **05655 → 4024B.** Reasoning is under 4020B.

**Pin-name differences between families of one device:**
- 74_390: LS `1CKA/1CKB/1CLR` vs HC `1CLKA/1CLKB/1CLR`.
- 74_393: LS `1A/1CLR`, SN-HC/LV `1CLK/1CLR`, CD-HC `1CP/1MR/1Q0..1Q3`.
- 74_590: LS `CCK/RCK/G` vs HC `CCLK/RCLK/OE`.
- 4020/4024/4040: CD `Q1../CP/MR` vs SN `QA../CLK/CLR`.

The pin numbers and functions are identical in each case.

---

## 74_90

- **Title / function:** "DECADE, DIVIDE-BY-TWELVE AND BINARY COUNTERS"; '90A, 'LS90 . . . Decade Counters (sn74ls90.pdf p1).
- **Datasheets:** counters/sn74ls90.pdf.
  - p1: pinouts, description.
  - p2: logic symbols.
  - p3: count sequences, reset/count tables.
  - p4: logic diagrams.
  - p6: TTL ratings and timing.
  - p7: TTL switching.
  - p8: LS recommended operating conditions.
  - p9: LS switching.
- **Families & prefixes:**
  - LS: SN74LS90*, SN54LS90*, SNJ54LS90*.
  - TTL: SN7490A, SN5490A are printed in the sheet but have no census rows.
- **Military:**
  - 7603201CA → SNJ54LS90J (from the mark).
  - JM38510/31501BCA and M38510/31501BCA → LS90 (**inferred**, see top).

**Pinout p14** (J, W, D, N; the census also has DR). From p1:

| pin | name | role |
|---|---|---|
| 1 | CKB | input |
| 2 | R0(1) | input |
| 3 | R0(2) | input |
| 4 | NC | nc |
| 5 | VCC | power |
| 6 | R9(1) | input |
| 7 | R9(2) | input |
| 8 | QC | output |
| 9 | QB | output |
| 10 | GND | ground |
| 11 | QD | output |
| 12 | QA | output |
| 13 | NC | nc |
| 14 | CKA | input |

Notes on the pinout:
- VCC and GND are on pins 5 and 10, not on the corners.
- R0(1) etc. are printed with subscripts, "R0(1)". A plain name such as `R01`/`R02`/`R91`/`R92` would fit the repo convention.

**Behavior** (p1 text, p2 symbol, p3 tables, p4 diagram):
- **Structure.** Four master-slave flip-flops, all triggered on the **falling edge** of their clock (bubbled CK in the logic diagram). They form two independent sections:
  - **÷2 section.** CKA (pin 14) clocks the QA flip-flop, which toggles on every CKA↓ (symbol "DIV2").
  - **÷5 section.** Three flip-flops, QB, QC and QD (symbol "DIV5", CKB = pin 1):
    - QB and QD are clocked by **CKB↓**.
    - QC is clocked by **QB↓**.
    - QB has J = /QD.
    - QD is set from the AND of QB and QC.
  - The ÷5 sequence of (QD QC QB) is 000 → 001 → 010 → 011 → 100 → 000.
  - Timing confirms that QD is clocked directly by CKB: CKB→QD is 21/23 ns, the same as CKB→QC.
- **External connections.** The sections are not joined inside the chip:
  - **BCD (decade):** connect QA → CKB and apply the input to CKA. The count is 0–9 in plain binary (p3 table, QD QC QB QA = LLLL … HLLH).
  - **Bi-quinary (5-2):** connect QD → CKA and apply the input to CKB. QA then gives a symmetrical ÷10 square wave. The p3 table, with column order QA QD QC QB:
    - 0 LLLL
    - 1 LLLH
    - 2 LLHL
    - 3 LLHH
    - 4 LHLL
    - 5 HLLL
    - 6 HLLH
    - 7 HLHL
    - 8 HLHH
    - 9 HHLL
- **Reset gating.** R0 = R0(1)·R0(2) (2-input NAND in the diagram) and R9 = R9(1)·R9(2) (2-input NAND). Both are **asynchronous and active high**. They are level inputs, not clocked.
- **Reset/count function table** (p3, quoted):

  ```
  R0(1) R0(2) R9(1) R9(2) | QD QC QB QA
    H     H     L     X   |  L  L  L  L
    H     H     X     L   |  L  L  L  L
    X     X     H     H   |  H  L  L  H
    X     L     X     L   |  COUNT
    L     X     L     X   |  COUNT
    L     X     X     L   |  COUNT
    X     L     L     X   |  COUNT
  ```
  - **Priority: R9 (set-to-9) > R0 (set-to-0) > count.** When R9(1)=R9(2)=H the output is 1001 regardless of R0.
  - Counting requires at least one R0 input low **and** at least one R9 input low.
  - Set-to-9 sets QA and QD and clears QB and QC (diagram: the R9 NAND drives the QA and QD set/preset and the QB and QC clear).
- **Illegal states of the ÷5 section** (101, 110 and 111 in QD QC QB): the datasheet prints no state diagram and does not list them. Deriving them from the JK structure is possible, but treat that as unverified.
- **Outputs.** Plain totem-pole outputs, not inverted.
- **Power-up state:** not stated.

**Timing**

LS, p9: VCC = 5 V, 25 °C, CL = 15 pF, RL = 2 kΩ. Values are typ tPLH/tPHL, max in parentheses.
- CKA → QA: 10 / 12 (16 / 18)
- CKA → QD (QA tied to CKB, i.e. full BCD ripple): 32 / 34 (48 / 50)
- CKB → QB: 10 / 14 (16 / 21)
- CKB → QC: 21 / 23 (32 / 35)
- CKB → QD: 21 / 23 (32 / 35)
- Set-to-0 → any (tPHL): 26 (40)
- Set-to-9 → QA, QD (tPLH): 20 (30)
- Set-to-9 → QB, QC (tPHL): 26 (40)
- fmax: CKA 32 min / 42 typ MHz; CKB 16 min MHz.

TTL '90A (p7, CL = 15 pF, RL = 400 Ω) has the same typ values. There are no census rows for it.

**Recommended operating conditions (LS, p8):**
- tw: A 15 ns, B 30 ns, reset 30 ns.
- Reset inactive-state setup: 25 ns.

**VCC** (p8):
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** SN74LS90D.

**Notes:**
- The logic diagram on p4 is partly illegible at gate level (the 5-input gating around QD). The ÷5 structure above is from the symbol, the count tables and the timing paths.
- Note D (p3): "H = high level, L = low level, X = irrelevant".

## 74_92

- **Title / function:** '92A, 'LS92 . . . Divide-By-Twelve Counters (sn74ls90.pdf p1).
- **Datasheets:** counters/sn74ls90.pdf p1–4, p8–9 (same sheet as 74_90).
- **Families & prefixes:**
  - LS: SN74LS92* (the census has SN74LS92D/N/NSR).
  - The sheet also prints SN54LS92, SN5492A and SN7492A, but they have no census rows.
- **Military:** none.

**Pinout p14** (D, N, NS). From p1:

| pin | name | role |
|---|---|---|
| 1 | CKB | input |
| 2 | NC | nc |
| 3 | NC | nc |
| 4 | NC | nc |
| 5 | VCC | power |
| 6 | R0(1) | input |
| 7 | R0(2) | input |
| 8 | QD | output |
| 9 | QC | output |
| 10 | GND | ground |
| 11 | QB | output |
| 12 | QA | output |
| 13 | NC | nc |
| 14 | CKA | input |

The 92 pinout differs from the 90/93: R0 is on pins 6/7, and QB/QD are swapped compared with the 90/93.

**Behavior:**
- **Structure.** Falling-edge master-slave flip-flops in three blocks (symbol p2: DIV2, DIV3, DIV2):
  - **÷2:** CKA↓ toggles QA.
  - **÷6 = ÷3 then ÷2:**
    - QB and QC form a ÷3 clocked by **CKB↓**. QB: J = /QC, K = 1. QC: J = QB, K = 1.
    - The ÷3 sequence of (QC QB) is 00 → 01 → 10 → 00.
    - QD is a toggle flip-flop clocked by **QC↓** (symbol: DIV2 clocked by "4̄", the falling edge of the QC weight-4 output).
- **Count sequence.** With QA → CKB and the input on CKA (p3, quoted):

  | count | QD | QC | QB | QA |
  |---|---|---|---|---|
  | 0 | L | L | L | L |
  | 1 | L | L | L | H |
  | 2 | L | L | H | L |
  | 3 | L | L | H | H |
  | 4 | L | H | L | L |
  | 5 | L | H | L | H |
  | 6 | H | L | L | L |
  | 7 | H | L | L | H |
  | 8 | H | L | H | L |
  | 9 | H | L | H | H |
  | 10 | H | H | L | L |
  | 11 | H | H | L | H |

  This is **not plain binary**: the count goes 5 → 8 in binary terms (0101 → 1000) and ends at 1101. QD is a ÷12 output with a 50 % duty cycle.
- **Reset.** R0 = R0(1)·R0(2). It is asynchronous and active high and clears all four Qs. There is no set-to-9.

  ```
  R0(1) R0(2) | QD QC QB QA
    H     H   |  L  L  L  L
    L     X   |  COUNT
    X     L   |  COUNT
  ```
- **Illegal ÷3 state** (QC QB = 11): not printed. From the JK structure above it would go to 00 on the next CKB↓, and that QC fall would toggle QD. This is derived, not stated.

**Timing** (LS, p9, 15 pF; typ tPLH / tPHL, max in parentheses):
- CKA → QA: 10 / 12 (16 / 18)
- CKA → QD (QA tied to CKB): 32 / 34 (48 / 50)
- CKB → QB: 10 / 14 (16 / 21)
- CKB → QC: 10 / 14 (16 / 21)
- CKB → QD: 21 / 23 (32 / 35)
- Set-to-0 → any: 26 (40)
- fmax: A 32/42, B 16 MHz.

**VCC:**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** none (SN74LS92D, N and NSR are all Active).

**Notes:**
- In the p3 count table both QB and QC appear as functions of the ÷3 section. The CKB→QB and CKB→QC delays are equal (10/14) because both are clocked directly by CKB.

## 74_93

- **Title / function:** '93A, 'LS93 . . . 4-Bit Binary Counters (sn74ls90.pdf p1).
- **Datasheets:** counters/sn74ls90.pdf p1–4, p8–9.
- **Families & prefixes:**
  - LS: SN74LS93*, SN54LS93*, SNJ54LS93*.
  - The sheet also prints SN5493A and SN7493, but they have no census rows.
- **Military:**
  - 7700101CA → SNJ54LS93J.
  - 7700101DA → SNJ54LS93W.
  - JM38510/31502BCA and BDA, and M38510/31502BCA and BDA → LS93 (**inferred**).

**Pinout p14** (J, W, D, N). From p1:

| pin | name | role |
|---|---|---|
| 1 | CKB | input |
| 2 | R0(1) | input |
| 3 | R0(2) | input |
| 4 | NC | nc |
| 5 | VCC | power |
| 6 | NC | nc |
| 7 | NC | nc |
| 8 | QC | output |
| 9 | QB | output |
| 10 | GND | ground |
| 11 | QD | output |
| 12 | QA | output |
| 13 | NC | nc |
| 14 | CKA | input |

The p4 footnote gives different pin numbers for the old 54L93; these are not orderable.

**Behavior:**
- **Structure.** All flip-flops trigger on the falling edge. Two blocks:
  - **÷2:** CKA↓ toggles QA.
  - **÷8:** a 3-bit binary ripple. CKB↓ toggles QB, QB↓ toggles QC, and QC↓ toggles QD.
- **Full 4-bit counter.** Connect QA → CKB and apply the input to CKA. The count is binary 0–15 (p3 table).
- **Reset.** R0(1)·R0(2) is asynchronous and active high. It clears all Qs (same table as the 92). There is no set-to-9.
- There are no illegal states.

**Timing** (LS, p9, 15 pF; typ tPLH / tPHL, max in parentheses):
- CKA → QA: 10 / 12 (16 / 18)
- CKA → QD (QA tied to CKB): 46 / 46 (70 / 70)
- CKB → QB: 10 / 14 (16 / 21)
- CKB → QC: 21 / 23 (32 / 35)
- CKB → QD: 34 / 34 (51 / 51)
- Set-to-0 → any: 26 (40)
- fmax: A 32/42, B 16 MHz.

The printed rows are clock → each output. There are no Qn → Qn+1 rows.

**VCC:**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** none.

## 74_293

- **Title / function:** "DECADE AND 4-BIT BINARY COUNTERS"; '293, 'LS293 . . . 4-BIT BINARY COUNTERS (sn74ls293.pdf p1).
- The p1 description says the '293/'LS293 are "electrically and functionally identical to" the '93A/'LS93. **Only the pin arrangement differs.** GND and VCC are on the corner pins 7 and 14.
- **Datasheets:** counters/sn74ls293.pdf.
  - p1: pinouts incl. FK.
  - p2: symbols.
  - p3: tables and logic diagram.
  - p6: LS recommended operating conditions.
  - p7: LS switching.
- **Families & prefixes:**
  - LS: SN74LS293* (census: SN74LS293D, SN74LS293N).
  - Also printed but not in the census: SN54LS293, SN54293, SN74293, and the '290/'LS290 decade versions (the '290 is functionally the '90A/'LS90).
- **Military:** none.

**Pinout p14** (D, N, J, W). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | NC | nc |
| 3 | NC | nc |
| 4 | QC | output |
| 5 | QB | output |
| 6 | NC | nc |
| 7 | GND | ground |
| 8 | QD | output |
| 9 | QA | output |
| 10 | CKA | input |
| 11 | CKB | input |
| 12 | R0(1) | input |
| 13 | R0(2) | input |
| 14 | VCC | power |

The logic diagram calls CKA "INPUT A" and CKB "INPUT B".

**FK pinout fk20** (SN54LS293 only, no census rows). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | NC | nc |
| 3 | NC | nc |
| 4 | NC | nc |
| 5 | NC | nc |
| 6 | QC | output |
| 7 | NC | nc |
| 8 | QB | output |
| 9 | NC | nc |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | QD | output |
| 13 | QA | output |
| 14 | CKA | input |
| 15 | NC | nc |
| 16 | CKB | input |
| 17 | NC | nc |
| 18 | R0(1) | input |
| 19 | R0(2) | input |
| 20 | VCC | power |

**Behavior** (identical to 74_93; p3):
- CKA↓ toggles QA.
- CKB↓ → QB, QB↓ → QC, QC↓ → QD (ripple ÷8).
- R0(1)·R0(2) = H is an asynchronous clear of all four Qs. Table: H H → LLLL; L X → COUNT; X L → COUNT.
- For the full 4-bit count, connect QA to input B. The count is binary 0–15.

**Timing** (LS293, p7, 15 pF, RL 2 kΩ; typ tPLH / tPHL, max in parentheses):
- A → QA: 10 / 12 (16 / 18)
- A → QD: 46 / 46 (70 / 70)
- B → QB: 10 / 14 (16 / 21)
- B → QC: 21 / 23 (32 / 35)
- B → QD: 34 / 34 (51 / 51)
- Set-to-0 → any: 26 (40)
- fmax: A 32/42, B 16 MHz.

**VCC** (p6):
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** none.

**Notes:**
- 74_293 could be modelled as a re-pinned 74_93.
- The census has no '290 rows, so no 74_290 device is needed.

---

## 74_390

- **Title / function:** "DUAL 4-BIT DECADE AND BINARY COUNTERS"; '390, 'LS390 . . . "Individual Clocks for A and B Flip-Flops Provide Dual ÷2 and ÷5 Counters" (sn74ls390.pdf p1). HC: "Dual Decade Ripple Counter" (cd74hc390.pdf p1).
- **Datasheets:**
  - counters/sn74ls390.pdf: p1 (pinouts incl. FK), p2 (tables, logic diagram, symbols), p4 (TTL ROC), p5 (TTL switching), p7 (LS ROC), p8 (LS switching).
  - counters/cd74hc390.pdf (see the HC/HCT part below).
- **Families & prefixes:**
  - LS: SN74LS390*, SN54LS390*, SNJ54LS390*.
  - HC: CD74HC390*.
  - HCT: CD54HCT390*, CD74HCT390*.
  - TTL SN54390/SN74390 are printed in the LS sheet but have no census rows.
- **Military:**
  - 7802601EA → SNJ54LS390J.
  - 7802601FA → SNJ54LS390W.
  - JM38510/32701BEA (CDIP16) and JM38510/32701B2A (LCCC20), plus the matching M38510 rows → LS390 (inferred, see top).
  - 5962-9098401MEA → CD54HCT390F3A.

**LS pinout p16** (J, W, D, N, NS). From p1:

| pin | name | role |
|---|---|---|
| 1 | 1CKA | input |
| 2 | 1CLR | input |
| 3 | 1QA | output |
| 4 | 1CKB | input |
| 5 | 1QB | output |
| 6 | 1QC | output |
| 7 | 1QD | output |
| 8 | GND | ground |
| 9 | 2QD | output |
| 10 | 2QC | output |
| 11 | 2QB | output |
| 12 | 2CKB | input |
| 13 | 2QA | output |
| 14 | 2CLR | input |
| 15 | 2CKA | input |
| 16 | VCC | power |

This is the same map as HC, which names the clocks `nCLKA`/`nCLKB`.

**LS FK pinout fk20** (SNJ54LS390FK and JM38510/32701B2A). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | 1CKA | input |
| 3 | 1CLR | input |
| 4 | 1QA | output |
| 5 | 1CKB | input |
| 6 | NC | nc |
| 7 | 1QB | output |
| 8 | 1QC | output |
| 9 | 1QD | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | 2QD | output |
| 13 | 2QC | output |
| 14 | 2QB | output |
| 15 | 2CKB | input |
| 16 | NC | nc |
| 17 | 2QA | output |
| 18 | 2CLR | input |
| 19 | 2CKA | input |
| 20 | VCC | power |

The datasheet prints pin 19 as "**3CKA**", which is clearly a misprint for 2CKA.

**Behavior** (LS p2; this agrees with HC p9–10):
- **Each half** has two sections:
  - A **÷2**: CKA↓ toggles QA.
  - A **÷5**: QB, QC and QD clocked from CKB↓. The sequence of (QD QC QB) is 000 → 001 → 010 → 011 → 100 → 000.
- **Internal structure of the ÷5** (logic diagram p2):
  - The QB toggle is clocked by CKB gated (AND) with /QD.
  - QC is clocked by QB↓.
  - QD is clocked by CKB through an AND-OR gate fed from QB, QC and /QD.
  - Timing agrees: B→QD is 13/14 ns, the same as B→QB, while B→QC is 24/26 ns, which is a ripple through QB.
- **All flip-flops trigger on the falling edge.**
- **CLR is active high and asynchronous.** The diagram shows a CLEAR input with an inverter to every flip-flop's CLEAR. CLR clears all four outputs of its half. The recovery spec is "Clear inactive-state setup time 25↓", referenced to the falling clock edge (p7).
- **BCD connection** (QA → CKB, input on CKA): the count is 0–9 in plain binary (p2 table).
- **Bi-quinary connection** (QD → CKA, input on CKB): QA is a ÷10 square wave. The p2 table, with column order QA QD QC QB:
  - 0 LLLL
  - 1 LLLH
  - 2 LLHL
  - 3 LLHH
  - 4 LHLL
  - 5 HLLL
  - 6 HLLH
  - 7 HLHL
  - 8 HLHH
  - 9 HHLL
- **Illegal ÷5 states** (101, 110, 111): not printed.
- **No set-to-9.** The two halves are fully independent.

**LS timing** (p8, VCC 5 V, 25 °C, CL 15 pF, RL 2 kΩ; typ tPLH / tPHL, max in parentheses):
- A → QA: 12 / 13 (20 / 20)
- A → QC (BCD-connected, as printed "QC of 'LS390"): 37 / 39 (60 / 60)
- B → QB: 13 / 14 (21 / 21)
- B → QC: 24 / 26 (39 / 39)
- B → QD: 13 / 14 (21 / 21)
- Clear → any (tPHL): 24 (39)
- fmax: A 25 min / 35 typ; B 12.5 min / 20 typ MHz.

TTL '390 (p5, 15 pF, 400 Ω) has the same typ values, but its B fmax is 20/30.

**Recommended operating conditions (LS, p7):**
- tw: A 20 ns, B 40 ns, clear high 20 ns.
- fcount: A 25 MHz, B 12.5 MHz.

**VCC (LS, p7):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:**
- LS: none.
- HC/HCT: listed in the part below.

**Notes:**
- The LS table "A → QC of 'LS390" (BCD-connected) is plausible as printed. It is the longest ripple path: A→QA (12), then QA→B→QB (13), then QB→QC (≈ 24 − 13). QD is clocked directly from B, so it is faster. The same reasoning may apply to the HCT "nCLK A to nQC 84" row that the HC part below flags as a probable typo; the HC row says nQD. Treat the HCT row as uncertain rather than wrong.

### 74_390 — HC/HCT part

- Title: "CD74HC390, CDx4HCT390 High-Speed CMOS Logic Dual Decade Ripple Counter"
  (cd74hc390.pdf p1).
- Datasheets: counters/cd74hc390.pdf p1, p3 (pinout, text), p4 (ROC), p6–7 (switching),
  p9 (overview + block diagram), p10 (truth table, BCD and bi-quinary sequences).
- Families & prefixes:
  * HC: CD74HC390*.
  * HCT: CD54HCT390*, CD74HCT390*.
  * There is no CD54HC390.
- Military: 5962-9098401MEA → CD54HCT390F3A (HCT390).
- Pinout p16 (J, N, D; p3): `1 1CLKA input`, `2 1CLR input`, `3 1QA output`, `4 1CLKB input`,
  `5 1QB output`, `6 1QC output`, `7 1QD output`, `8 GND ground`, `9 2QD output`, `10 2QC output`,
  `11 2QB output`, `12 2CLKB input`, `13 2QA output`, `14 2CLR input`, `15 2CLKA input`,
  `16 VCC power`.
  * The clocks are printed with overbars (nCLK A, nCLK B) because they are falling-edge inputs.
  * Old names (p2 revision history): 1CP0 → 1CLKA, 1MR → 1CLR, 1Q0..1Q3 → 1QA..1QD, 1CP1 → 1CLKB,
    and the same for counter 2.
- Behavior (p9–10):
  * Each half is a ÷2 section (CLKA → QA) plus a ÷5 section (CLKB → QB, QC, QD). Both sections
    are triggered on the **High-to-Low** transition of their clock.
  * nCLR is **active-high and asynchronous**. It clears all four outputs of that half and
    "overrides the clock". The block diagram on p9 shows CLR also gated (NOR) into the clock paths.
  * Truth table: CLK ↑, CLR L → no change; CLK ↓, CLR L → count; CLR H → all Qs low.
  * BCD connection: QA → CLKB, input on CLKA. The sequence is 0–9 binary
    (QD QC QB QA = LLLL … HLLH).
  * Bi-quinary connection: QD → CLKA, input on CLKB; QA is the ÷10 output (p10 Table 7-3).
    Sequence (QD QC QB QA): 0 LLLL, 1 LLHL, 2 LHLL, 3 LHHL, 4 HLLL, 5 LLLH, 6 LLHH, 7 LHLH, 8 LHHH,
    9 HLLH.
  * The ÷5 section counts QD QC QB = 000 → 001 → 010 → 011 → 100 → 000.
  * Timing hint about the ÷5 structure: CLKB → QD (36 ns) is faster than CLKB → QC (49 ns). This
    agrees with the LS390 structure, where the QD flip-flop is clocked directly by CLKB and QC is
    clocked by QB.
- Timing (p6–7, 25 °C; max at 4.5 V/50 pF; typ at 5 V/15 pF where printed):
  * HC: CLKA → QA **35** (typ 14); CLKB → QB **37**; CLKB → QC **49**; CLKB → QD **36** (typ 15);
    CLKA → QD (BCD-connected) **73**; CLR → Qn **38** (typ 16).
  * HCT: CLKA → QA **40** (typ 17); CLKB → QB **43**; CLKB → QC **55**; CLKB → QD **42**
    (typ 18); "nCLK A to nQC" **84** (probably meant nQD; see notes); CLR → Qn **42** (typ 18).
- VCC: HC 2–6 V; HCT 4.5–5.5 V (p4).
- Obsolete: CD74HC390M, CD74HCT390M, CD74HCT390MT.
- Notes:
  * p1 description says "The SN74HC390 …", but this is the CD74 sheet.
  * The HCT row "nCLK A to nQC 84 ns" is probably a typo for nQD (the HC row is nCLK A → nQD 73).
  * Revision E (p2) corrected Table 7-3, QC at count 6.

---

## 74_393

- **Title / function:** '393, 'LS393 . . . "Dual 4-Bit Binary Counter with Individual Clocks"; "All Have Direct Clear for Each 4-Bit Counter" (sn74ls390.pdf p1).
- **Datasheets:**
  - counters/sn74ls390.pdf p1 (pinouts incl. FK), p2 (count sequence, symbol), p3 (logic diagram), p7 (LS ROC), p8 (LS switching).
  - The HC/HCT/LV PDFs are covered in the part below.
- **Families & prefixes:**
  - LS: SN74LS393*, SN54LS393*, SNJ54LS393*, SNV54LS393W (the mark of JM38510/32702SDA).
  - HC: SN74HC393*, SN54HC393*, SNJ54HC393*, CD54HC393*, CD74HC393*.
  - HCT: CD54HCT393*, CD74HCT393*.
  - LV: SN74LV393A*, SN74LV393A-Q1.
  - TTL SN54393/SN74393 are printed in the LS sheet but have no census rows.
- **Military (LS):** JM38510/32702BCA (CDIP14), JM38510/32702BDA (CFP14), JM38510/32702SDA (CFP14) and JM38510/32702B2A (LCCC20), plus the matching M38510 rows → LS393. HC military rows are mapped in the part below.

**LS pinout p14** (J, W, D, N, NS). From p1:

| pin | name | role |
|---|---|---|
| 1 | 1A | input |
| 2 | 1CLR | input |
| 3 | 1QA | output |
| 4 | 1QB | output |
| 5 | 1QC | output |
| 6 | 1QD | output |
| 7 | GND | ground |
| 8 | 2QD | output |
| 9 | 2QC | output |
| 10 | 2QB | output |
| 11 | 2QA | output |
| 12 | 2CLR | input |
| 13 | 2A | input |
| 14 | VCC | power |

The pin numbers are the same as the SN-HC/LV map, which names pins 1 and 13 `1CLK`/`2CLK`.

**LS FK pinout fk20** (SNJ54LS393FK and JM38510/32702B2A). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | 1A | input |
| 3 | 1CLR | input |
| 4 | 1QA | output |
| 5 | NC | nc |
| 6 | 1QB | output |
| 7 | NC | nc |
| 8 | 1QC | output |
| 9 | 1QD | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | 2QD | output |
| 13 | 2QC | output |
| 14 | 2QB | output |
| 15 | NC | nc |
| 16 | 2QA | output |
| 17 | NC | nc |
| 18 | 2CLR | input |
| 19 | 2A | input |
| 20 | VCC | power |

This is identical to the SNJ54HC393FK map, apart from the clock name.

**Behavior** (LS p2–3):
- Each half is a 4-stage binary ripple counter:
  - Input A↓ toggles QA.
  - QA↓ → QB, QB↓ → QC, QC↓ → QD.
- CLR is active high and asynchronous. Through an inverter it drives every flip-flop's CLEAR and clears the half to LLLL.
- The count is binary 0–15 and wraps.
- There are no illegal states.

**LS timing** (p8, 15 pF, RL 2 kΩ; typ tPLH / tPHL, max in parentheses):
- A → QA: 12 / 13 (20 / 20)
- A → QD: 40 / 40 (60 / 60)
- Clear → any (tPHL): 24 (39)
- fmax: 25 min / 35 typ MHz.

QB and QC are not printed separately for the LS part.

**Recommended operating conditions (LS, p7):**
- tw: A 20 ns, clear high 20 ns.
- Clear inactive-state setup: 25 ns, referenced to the clock ↓.

**VCC (LS):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:**
- LS: SN74LS393D.
- HC/HCT/LV: listed in the part below.

### 74_393 — HC/HCT/LV part

- Title:
  * "CDx4HC393, CDx4HCT393 High-Speed CMOS Logic Dual 4-Stage Binary Counter" (cd74hc393.pdf p1).
  * "SNx4HC393 Dual 4-Bit Binary Counters" (sn74hc393.pdf p1).
  * "SN74LV393A Dual 4-Bit Binary Counters" (sn74lv393a.pdf p1).
  * "SN74LV393A-Q1 Dual 4-Bit Binary Counter" (sn74lv393a-q1.pdf p1).
- Datasheets:
  * counters/cd74hc393.pdf p1, p3 (pinout), p4 (ROC), p6–7 (switching), p9–10 (truth tables).
  * counters/sn74hc393.pdf p1, p3 (pinouts incl. FK, pin table), p4 (ROC), p5 (timing),
    p6 (switching), p8 (logic diagram), p9 (count sequence).
  * counters/sn74lv393a.pdf p3 (pin table), p5 (ROC), p6–7 (switching), p10 (function table).
  * counters/sn74lv393a-q1.pdf p3, p5, p6–7, p10.
- Families & prefixes:
  * HC: CD54HC393*, CD74HC393*, SN54HC393*, SN74HC393*, SNJ54HC393*.
  * HCT: CD54HCT393*, CD74HCT393*.
  * LV: SN74LV393A*, SN74LV393A-Q1 (orderables SN74LV393ATPWRQ1, SN74LV393ATPWRG4Q1).
- Military:
  * 5962-8989001CA → CD54HCT393F3A (HCT).
  * 84100012A (FK), 8410001CA (J), 8410001DA (W) → SNJ54HC393FK/J/W (HC). CD54HC393F3A is also
    marked 8410001CA.
  * JM38510/66309BCA and M38510/66309BCA → HC393 (sn74hc393 is a single-function PDF).
- Pinouts:
  * p14, CD naming (cd74hc393 p3; J, N, D): `1 1CP input` (overbar), `2 1MR input`, `3 1Q0 output`,
    `4 1Q1 output`, `5 1Q2 output`, `6 1Q3 output`, `7 GND ground`, `8 2Q3 output`, `9 2Q2 output`,
    `10 2Q1 output`, `11 2Q0 output`, `12 2MR input`, `13 2CP input`, `14 VCC power`.
  * p14, SN naming:
    - Packages: sn74hc393 p3 lists J, W, D, DB, DYY, N, NS, PW; LV393A lists D, DB, DGV, NS, PW;
      -Q1 lists PW only.
    - Pins: `1 1CLK input`, `2 1CLR input`, `3 1QA output`, `4 1QB output`, `5 1QC output`,
      `6 1QD output`, `7 GND ground`, `8 2QD output`, `9 2QC output`, `10 2QB output`,
      `11 2QA output`, `12 2CLR input`, `13 2CLK input`, `14 VCC power`.
  * fk20 (SNJ54HC393FK, sn74hc393 p3): `1 NC nc`, `2 1CLK input`, `3 1CLR input`, `4 1QA output`,
    `5 NC nc`, `6 1QB output`, `7 NC nc`, `8 1QC output`, `9 1QD output`, `10 GND ground`,
    `11 NC nc`, `12 2QD output`, `13 2QC output`, `14 2QB output`, `15 NC nc`, `16 2QA output`,
    `17 NC nc`, `18 2CLR input`, `19 2CLK input`, `20 VCC power`.
- Behavior:
  * Each half is a 4-bit binary ripple counter. It advances on the CLK **falling edge**
    ("negative-going transition", sn74lv393a p10). In the logic diagram (sn74hc393 p8), each T
    flip-flop is clocked by the falling edge of the previous Q.
  * CLR/MR is **active high and asynchronous** and sets all outputs L (direct clear).
  * Function table (LV p10): CLK ↑, CLR L → no change; CLK ↓, CLR L → advance; CLR H → all L.
    cd74hc393 p10 Table 7-2: MR H → LLLL.
  * The count is binary 0–15 (sn74hc393 p9; cd74hc393 p9 with Q0 as the LSB).
  * LV: Ioff partial power-down; inputs tolerate 5.5 V.
- Timing:
  * HC, sn74hc393 p6 (4.5 V, 50 pF, typ / max 25 °C):
    - CLK → QA **15** / 24; CLK → QB **22** / 38; CLK → QC **28** / 48; CLK → QD **32** / 58.
    - CLR → any (tPHL) **17** / 33 ns.
  * HC, cd74hc393 p6 (max at 4.5 V/50 pF):
    - CP → Q0 30 (typ 12 at 5 V, 15 pF); CP → Q1 38; CP → Q2 48; CP → Q3 57.
    - MR → Qn 27 (typ 11).
  * HCT, cd74hc393 p7 (4.5 V/50 pF max):
    - CP → Q0 32 (typ 13); Q1 44; Q2 50; Q3 62.
    - MR → Qn 32 (typ 13).
  * LV, sn74lv393a p7 (3.3 V, 50 pF, typ / max 25 °C / max −40…85 °C):
    - CLK → QA **6.7** / 16.7 / 19; QB **7.8** / 19.3 / 22; QC **8.7** / 21.5 / 24.5;
      QD **9.5** / 23.2 / 26.5.
    - CLR → Qn (tPHL) **6.8** / 15.8 / 18 ns.
  * LV-Q1 (sn74lv393a-q1 p7): identical numbers.
  * Rows are given as clock → each output, not as Qn → Qn+1.
- VCC:
  * HC: 2–6 V (SN54 and SN74 rows the same).
  * HCT: 4.5–5.5 V.
  * LV: 2–5.5 V, inputMax 5.5 V (both LV PDFs, p5).
- Obsolete: CD74HC393M, CD74HCT393MT, SN74HC393D, SN74HC393DT, SN74HC393PW, SN74HC393PWT,
  SN74LV393AD, SN74LV393APW. SN74LV393ADGVR and SN74LV393ATPWRG4Q1 are NRND.
- Notes:
  * pdftotext glues footnote "1" onto LV numbers (e.g. "7.11" = 7.1⁽¹⁾); the values above are
    corrected for this.
  * sn74hc393 pin table says "Counter 1 B Output" for 1QC and 1QD (copy error).
  * The sn74hc393 addendum (p13–14) lists **SN74HCS393DYYR** (SOT-23-THIN DYY, Active, marking
    HCS393). It is not in the census. DYY appears only in the sn74hc393 pinout caption. HCS is a
    different (Schmitt-input) family.
  * cd74hc393 HC CP → Q0 max over −55…125 °C is printed as 59 (p6), which looks like a typo.
  * The -Q1 ROC has no TA row.

---

## 74_590

- **Title / function:** LS: "8-BIT BINARY COUNTERS WITH OUTPUT REGISTERS"; SN54LS590/SN74LS590 have 3-state register outputs (sn74ls590.pdf p1). HC: "8-BIT BINARY COUNTERS WITH 3-STATE OUTPUT REGISTERS".
- **Datasheets:**
  - counters/sn74ls590.pdf: p1 (pinouts incl. FK, description), p2 (logic diagram), p3 (symbol, ROC), p4 (electrical, switching).
  - counters/sn74hc590a.pdf (see the HC part below).
- **Families & prefixes:**
  - LS: SN74LS590*, SN54LS590*, SNJ54LS590*.
  - HC: SN74HC590A*, SN54HC590A*, SNJ54HC590A*.
  - The LS591 (open-collector Q) is in the same LS sheet but has no census rows.
- **Military:**
  - LS: 5962-8751701EA → SNJ54LS590J; 5962-87517012A → SNJ54LS590FK (from the marks).
  - HC: see the part below.
- **Pin names differ between families. LS / HC:**
  - CCK / CCLK
  - RCK / RCLK
  - G / OE (both active low)
  - CCKEN, CCLR and RCO are the same name in both, and all three are active low.

  The functions and pin numbers are identical. Both families are one device.

**LS pinout p16** (J, W, N; the census also has D and NS). From p1:

| pin | name | role |
|---|---|---|
| 1 | QB | output |
| 2 | QC | output |
| 3 | QD | output |
| 4 | QE | output |
| 5 | QF | output |
| 6 | QG | output |
| 7 | QH | output |
| 8 | GND | ground |
| 9 | RCO | output |
| 10 | CCLR | input |
| 11 | CCK | input |
| 12 | CCKEN | input |
| 13 | RCK | input |
| 14 | G | input |
| 15 | QA | output |
| 16 | VCC | power |

**LS FK pinout fk20** (SNJ54LS590FK and 5962-87517012A). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | QB | output |
| 3 | QC | output |
| 4 | QD | output |
| 5 | QE | output |
| 6 | NC | nc |
| 7 | QF | output |
| 8 | QG | output |
| 9 | QH | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | RCO | output |
| 13 | CCLR | input |
| 14 | CCK | input |
| 15 | CCKEN | input |
| 16 | NC | nc |
| 17 | RCK | input |
| 18 | G | input |
| 19 | QA | output |
| 20 | VCC | power |

This is the same as the HC FK map.

**LS behavior** (p1 text, p2 logic diagram, p3 symbol). It matches the HC part below.
- **Counter.** An 8-bit synchronous binary up-counter (T flip-flops with an AND carry chain).
  - It counts on **CCK↑** when **CCKEN = L**. The symbol shows G1 on CCKEN and "1+" on CCK.
  - "Internal circuitry prevents clocking from the clock enable" (p1). The CCKEN/CCK gating in the diagram is a NOR/AND latch structure.
  - Setup: CCKEN low before CCK↑, 20 ns. Hold: CCKEN low after CCK↑, 0 ns (p3).
- **CCLR.** Active low and **asynchronous** ("direct clear", p1; symbol "CT=0" without a clock dependency). It clears **the counter only**: the R inputs exist only on the counter flip-flops, and the register is not cleared. Recovery: CCLR inactive before CCK↑, 20 ns.
- **Register.** An 8-bit storage register loaded from the counter on **RCK↑** (symbol C2 / 2D).
  - "Both the counter and register clocks are positive-edge triggered. If the user wishes to connect both clocks together, the counter state will always be one count ahead of the register" (p1).
  - p3 note 2: "The clocks may be tied together in which case the register state will be one clock pulse behind the counter."
  - Setup: CCK↑ before RCK↑, 40 ns.
- **Outputs.** QA–QH are 3-state and driven from the **register**.
  - **G = L enables them; G = H gives Hi-Z** (symbol EN3).
  - G affects only the Q buffers.
- **RCO.** Active low and never 3-state. It is an 8-input NAND of the **counter** flip-flop outputs (p2), so **RCO = L exactly when the counter = 255**.
  - It is not gated by CCKEN or G. The symbol reads "(CT=255) Z4 → RCO".
  - CCLR↓ forces it high (CCLR↓→RCO tPLH).
- **Cascading** (p1):
  - Two stages: RCO → CCKEN of the second stage.
  - Larger chains: RCO of each stage → CCK of the following stage.
- **Power-up:** not stated.

**LS timing** (p4, VCC 5 V, 25 °C; typ, max in parentheses):
- CCK↑ → RCO (RL 1 kΩ, 30 pF): tPLH 14 (22), tPHL 20 (30)
- CCLR↓ → RCO (tPLH): 30 (45)
- RCK↑ → Q (RL 667 Ω, 45 pF): tPLH 12 (18), tPHL 22 (33)
- G↓ → Q: tPZH 25 (38), tPZL 30 (45)
- G↑ → Q (5 pF): tPHZ 20 (30), tPLZ 25 (38)
- fmax (RCK → Q): 20 min / 35 typ MHz.

The recommended conditions (p3) give fCCK 0–20 MHz and fRCK 0–25 MHz.

**VCC (LS, p3):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:**
- LS: none Obsolete. **SN74LS590D and SN74LS590NSR are Last Time Buy.**
- HC: listed in the part below.

### 74_590 — HC part

- Title: "8-BIT BINARY COUNTERS WITH 3-STATE OUTPUT REGISTERS" (SN54HC590A, SN74HC590A).
- Datasheet: counters/sn74hc590a.pdf. Pages used: p1 (pinouts, description), p2 (timing diagram), p3 (logic diagram), p4 (rec. op. cond.), p6 (timing req.), p7 (SN54 switching), p8 (SN74 switching, 50 pF), p9 (150 pF).
  There is **no function table** in this datasheet. Behavior below comes from the p1 text, the p2 timing diagram and the p3 logic diagram.
- Families & prefixes: HC: SN74HC590A*, SN54HC590A*, SNJ54HC590A*.
- Military numbers: single-function PDF, so all rows are HC590A. 5962-89603012A = SNJ54HC590AFK (LCCC 20); 5962-8960301EA = SNJ54HC590AJ (CDIP 16); 5962-8960301FA = SNJ54HC590AW (CFP 16).

#### Pinouts
p16: J, W, D, DW, N (p1)
```
1 QB out        9 RCO out   (printed RCO-bar: active low)
2 QC out       10 CCLR in   (active low)
3 QD out       11 CCLK in
4 QE out       12 CCKEN in  (active low)
5 QF out       13 RCLK in
6 QG out       14 OE in     (active low)
7 QH out       15 QA out
8 GND ground   16 VCC power
```
p20: FK (LCCC 20, SN54 only) (p1)
```
1 NC nc     6 NC nc     11 NC nc     16 NC nc
2 QB out    7 QF out    12 RCO out   17 RCLK in
3 QC out    8 QG out    13 CCLR in   18 OE in
4 QD out    9 QH out    14 CCLK in   19 QA out
5 QE out   10 GND gnd   15 CCKEN in  20 VCC power
```
QA is the LSB and QH the MSB (timing diagram p2: hex counts on QA–QH).

#### Behavior (p1, p2, p3)
- Structure: an 8-bit synchronous binary up-counter (T flip-flops with an AND carry chain) feeds an 8-bit D storage register. The register's outputs drive QA–QH through 3-state buffers. The register has no clear input.
- CCLK: the counter counts up on the CCLK rising edge when CCKEN = L. CCKEN is active low and synchronous: setup is "CCKEN low before CCLK↑" and hold is "CCKEN low after CCLK↑" (p6). p1 says "Internal circuitry prevents clocking from the clock enable". The logic diagram (p3) gates CCLK with a NOR/AND structure that feeds back the internal clock, so a CCKEN edge while CCLK is high does not produce a count. Model it as: count on CCLK↑ if and only if CCKEN = L at that edge.
- CCLR: active low and asynchronous (direct clear). It clears **the counter only**; the storage register is unaffected (p3: R inputs only on the counter T flip-flops). Timing sequence step 1 on p2 reads "Clear Counter (asynchronous)". There is a setup of CCLR high (inactive) before CCLK↑ (p6). Priority: CCLR over CCLK.
- RCLK: on the RCLK rising edge the register loads the current counter value. If CCLK and RCLK are tied together, the register is one count behind the counter (p1: "the counter state always is one count ahead of the register"; p6 footnote: "The clocks may be tied together, in which case the register is one clock pulse behind the counter"). Setup is CCLK↑ before RCLK↑ (p6). With tied clocks, the register gets the pre-edge counter value.
- OE: active low. OE = L enables QA–QH; OE = H puts them in Hi-Z (p2 steps 5–7). OE does not affect the counter, the register or RCO. While the outputs are in Hi-Z, counting and storing continue.
- RCO: active low and never 3-state. RCO = NOT(QA·QB·…·QH of the **counter**, not the register). It is not gated by CCKEN or CCLK: p3 shows an 8-input NAND fed only by the counter stages, and on p2 RCO is low during the whole Hex FF counter state while the register holds 05. CCLR↓ makes RCO go high (tPLH CCLR→RCO).
- Cascading (p1): for two stages, connect RCO of stage 1 to CCKEN of stage 2 (synchronous, common CCLK). For longer chains, connect RCO of each stage to CCLK of the next stage. The rising edge of RCO happens when the counter leaves FF, and that edge clocks the next stage.
- Power-up state: not stated.

#### Timing (HC, SN74 typ, 25 °C, VCC 4.5 V, CL 50 pF, p8; the SN54 typ values on p7 are identical)
- CCLK↑ → RCO: 20 ns typ (max 30 at 25 °C; SN74 max 38 over temperature, SN54 max 45)
- CCLR↓ → RCO (tPLH): 18 ns typ (max 26; SN54 max 28)
- RCLK↑ → Q: 18 ns typ (max 28; SN54 max 31)
- OE↓ → Q (ten): 20 ns typ (max 25; SN54 max 30)
- OE↑ → Q (tdis): 20 ns typ (max 25; SN54 max 30)
- fmax: 35 MHz typ.
- CL = 150 pF (p9): RCLK→Q 24 typ, ten 23 typ.
- The datasheet prints no CCLK→Q path (Q comes only from the register).
- Timing requirements at 4.5 V, SN74 (p6): tw CCLK/RCLK 31 ns; CCLR low 25 ns; setups and hold as listed above.
- VCC: SN54 and SN74 both 2–6 V (p4). VI 0..VCC, so there is no inputMax above VCC.
- Obsolete (census): SN74HC590AD, SN74HC590ADT, SN74HC590ADW.
- Notes: the p4 footnote mentions "shift, count, or toggle operating modes"; this is boilerplate. The DW package (SOIC-16 wide) has the same pinout as D. RCO VOH/VOL is specified at 4 mA, while QA–QH are specified at 6 mA.

---

## 74_592

- **Title / function:** "8-BIT BINARY COUNTERS WITH INPUT REGISTERS". The 'LS592 is described as "a parallel input, 8-bit storage register feeding an 8-bit binary counter. Both the register and the counter have individual positive-edge-triggered clocks. In addition, the counter has direct load and clear functions." (sn74ls592.pdf p1)
- **Datasheets:** counters/sn74ls592.pdf.
  - p1: pinouts incl. FK, description, enable tables.
  - p2: symbols.
  - p3: 'LS592 logic diagram.
  - p5: typical operating sequences.
  - p6: ROC and timing requirements.
  - p8: switching.
- **Families & prefixes:** LS: SN74LS592*, SN54LS592*, SNJ54LS592*.
- **Military:** 5962-8762101EA → SNJ54LS592J; 5962-8762101FA → SNJ54LS592W (from the marks).

**Pinout p16** (J, W, N; the census also has D and NS). From p1:

| pin | name | role |
|---|---|---|
| 1 | B | input |
| 2 | C | input |
| 3 | D | input |
| 4 | E | input |
| 5 | F | input |
| 6 | G | input |
| 7 | H | input |
| 8 | GND | ground |
| 9 | RCO | output |
| 10 | CCLR | input |
| 11 | CCK | input |
| 12 | CCKEN | input |
| 13 | RCK | input |
| 14 | CLOAD | input |
| 15 | A | input |
| 16 | VCC | power |

Notes on the pinout:
- A is the LSB and H the MSB.
- Pin 6 is the data input "G". It is not an enable.
- RCO, CCLR, CCKEN and CLOAD are printed with overbars, meaning active low.

**FK pinout fk20** (SN54LS592 only, no census rows). From p1:

| pin | name | role |
|---|---|---|
| 1 | NC | nc |
| 2 | B | input |
| 3 | C | input |
| 4 | D | input |
| 5 | E | input |
| 6 | NC | nc |
| 7 | F | input |
| 8 | G | input |
| 9 | H | input |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | RCO | output |
| 13 | CCLR | input |
| 14 | CCK | input |
| 15 | CCKEN | input |
| 16 | NC | nc |
| 17 | RCK | input |
| 18 | CLOAD | input |
| 19 | A | input |
| 20 | VCC | power |

**Behavior** (p1–3, p5–6):
- **Input register.** 8 D flip-flops load A–H on **RCK↑**. There is no register enable on the '592, and the register has no clear.
  - Setup: data before RCK↑, 20 ns. Hold: 0 ns.
- **Counter.** An 8-bit synchronous binary up-counter (T flip-flops) that counts on **CCK↑** when **CCKEN = L**.
  - Symbol: G3 / "3+"; enable table p1: CCKEN-bar L → Enable, H → Disable.
  - Setup: CCKEN low before CCK↑, 30 ns.
  - The CCK/CCKEN gating (p3) is the same NOR/AND latch structure as the '590, which prevents clocking from the enable.
- **CLOAD.** Active low, an **asynchronous ("direct overriding") parallel load** of the counter from the **register**. The symbol has "C2" on CLOAD with no edge, and "2D" from the register into the counter.
  - Gate level (p3): for each bit, S̄ = NAND(regQ, LOAD) and R̄ = NAND(regQ̄, LOAD), ANDed with the clear. So while CLOAD = L the counter continuously equals the register.
  - Because of this, an RCK↑ while CLOAD = L passes straight through to the counter. The datasheet specifies RCK↑ → RCO with "CLOAD = L", and its note says "RCK↑ before CLOAD↑", 30 ns: "This time insures the data saved by RCK↑ will also be loaded into the counter."
  - Recovery: CLOAD inactive before CCK↑, 20 ns. Pulse width tw(CLOAD) is 40 ns.
- **CCLR.** Active low, an **asynchronous clear of the counter only**. The symbol shows CT=0 on the counter; the register is not cleared.
  - Recovery: CCLR inactive before CCK↑, 20 ns. tw is 20 ns.
- **CCLR vs CLOAD priority: not specified.** The gate level (p3) shows CCLR forcing R̄ low while LOAD·regQ = 1 can still force S̄ low. Both asserted with a register bit = 1 would therefore assert S and R together, which is undefined in the datasheet. A model should pick clear-wins and flag it.
- **Async controls vs clock:** CCLR and CLOAD override the clock. Normal priority: CCLR/CLOAD (async) > CCK count.
- **RCO.** Active low. It is an 8-input NAND of the **counter** bits (p3), so **RCO = L when the counter = FF**. The timing sequence on p5 shows a "low-going RCO pulse" at count FF, ending at 00.
  - It is not gated by CCKEN or CCK. In the p5 sequence RCO goes low at FF and stays low while CCKEN is high.
  - Cascading: two stages use RCO → CCKEN; longer chains use RCO → CCK of the next stage.
- **Typical sequence** (p5): CCLR pulse, then RCK↑ stores FC, then a CLOAD pulse loads FC. CCK↑ then gives FD and FE, a CCKEN-high clock is ignored, and further clocks give FF (RCO low) and then 00.
- **Power-up:** not stated.

**Timing** (p8, VCC 5 V, 25 °C, RL 1 kΩ, CL 30 pF; typ, max in parentheses):
- CCK↑ → RCO: tPLH 15 (23), tPHL 20 (30)
- CLOAD↓ → RCO: tPLH 31 (47), tPHL 27 (41)
- CCLR↓ → RCO (tPLH): 30 (45)
- RCK↑ → RCO (CLOAD = L): tPLH 35 (53), tPHL 30 (45)
- fmax (CCK → RCO): 20 min / 35 typ MHz.

The '592 has no Q outputs, so RCO is its only output.

**VCC (p6):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** none.

## 74_593

- **Title / function:** same sheet. "The 'LS593 comes in a 20-pin package and has all the features of the 'LS592 plus 3-state I/O, which provides parallel counter outputs … A register clock enable (RCKEN) is also provided." (sn74ls592.pdf p1). This is a separate device: different pin count and extra pins.
- **Datasheets:** counters/sn74ls592.pdf.
  - p1: pinouts, enable tables.
  - p2: symbol.
  - p4: 'LS593 logic diagram.
  - p5: operating sequence.
  - p6: ROC.
  - p7: electrical.
  - p8: switching.
- **Families & prefixes:** LS: SN74LS593*, SN54LS593*, SNJ54LS593*.
- **Military:** none (no SMD rows).

**Pinout p20** (J, W, DW, N). From p1:

| pin | name | role | notes |
|---|---|---|---|
| 1 | A/QA | inout | |
| 2 | B/QB | inout | |
| 3 | C/QC | inout | |
| 4 | D/QD | inout | |
| 5 | E/QE | inout | |
| 6 | F/QF | inout | |
| 7 | G/QG | inout | |
| 8 | H/QH | inout | |
| 9 | CLOAD | input | |
| 10 | GND | ground | |
| 11 | RCO | output | |
| 12 | CCLR | input | |
| 13 | CCK | input | |
| 14 | CCKENBAR | input | printed CCKEN with an overbar |
| 15 | CCKEN | input | printed without an overbar |
| 16 | RCK | input | |
| 17 | RCKEN | input | overbar |
| 18 | GBAR | input | printed G with an overbar |
| 19 | G | input | no overbar |
| 20 | VCC | power | |

Notes on the pinout:
- Two pairs of names collide once the overbars are dropped, so the BAR suffix is used: **CCKEN (15) / CCKENBAR (14)** and **G (19) / GBAR (18)**.
- The I/O pins are printed as "A/QA" etc. A repo name such as `A` or `QA` (or `AQA`) has to be chosen.
- 'LS593 **FK** (fk20, p1): the pin numbers and names are the same as the DIP, 1–20 in the same order. Pins 1–3 and 19–20 are on top, 4–8 on the left, 9–13 on the bottom and 14–18 on the right. There are no NC pins. No FK rows are in the census.

**Behavior** (p1 tables, p2 symbol, p4 diagram, p5 sequence):
- **Output enable control** (p1, quoted):

  ```
  G  G̅ | A/QA thru H/QH
  L  L |  input mode
  L  H |  input mode
  H  L |  output mode
  H  H |  input mode
  ```
  - **Output mode (pins driven with the counter Q) ⇔ G = H AND GBAR = L.** Otherwise the pins are Hi-Z and act as register inputs.
  - Symbol: "&" of G and G̅ → EN6. The output buffers are "3D Z5 ▽", which drive the **counter** state.
- **Counter clock enable** (p1, quoted):

  ```
  CCKEN CCKEN̅ | EFFECT ON CCK
    L     L    | Enable
    L     H    | Disable
    H     L    | Enable
    H     H    | Enable
  ```
  - **Count enable = CCKEN OR NOT(CCKENBAR).** Symbol: "≥1" → G4.
  - The counter counts on CCK↑ when enabled.
  - Setup: "CCKEN̅ low or CCKEN high, 'LS593", before CCK↑, 30 ns.
- **Register.** The D inputs come from the I/O pins (symbol "2D"). It loads on **RCK↑ when RCKEN = L** (symbol G1 on RCKEN̅, 1C2 on RCK). Setup: RCKEN low before RCK↑, 20 ns. p4 note: "Gating for RCK is similar in detail to that shown for CCK."
  - In output mode the pins carry the counter outputs, so an enabled RCK↑ would capture the counter value. This follows from the diagram (the register D comes from the pin node) but is not stated in the text.
- **CLOAD (asynchronous load of the counter from the register), CCLR (asynchronous clear of the counter only), RCO and the cascading rules** are the same as 74_592.
  - RCO = L when the counter = FF. It is a NAND of the counter bits and is not 3-state.
  - The CCLR vs CLOAD conflict is likewise unspecified.
- **Typical sequence** (p5):
  1. With G=H and G̅=L the outputs show 00.
  2. G and G̅ go to input mode (G̅=H). The bus is driven externally with FC, and RCK↑ with RCKEN̅ = L stores it.
  3. A CLOAD pulse loads FC.
  4. G=H and G̅=L show FC.
  5. CCK clocks count FD, FE, FF (RCO low) and 00. One clock with CCKEN = L and CCKEN̅ = H is ignored.
- **Power-up:** not stated.

**Timing** (p8, VCC 5 V, 25 °C; typ, max in parentheses).

Q paths at RL 667 Ω, CL 45 pF:
- CCK↑ → Q: tPLH 14 (21), tPHL 26 (39)
- CLOAD↓ → Q: tPLH 34 (51), tPHL 28 (42)
- CCLR↓ → Q (tPHL): 25 (38)
- G↑ → Q: tPZH 31 (47), tPZL 27 (40)
- G̅↓ → Q: tPZH 29 (45), tPZL 31 (47)

Disable paths at CL 5 pF:
- G↓ → Q: tPHZ 33 (50), tPLZ 35 (52)
- G̅↑ → Q: tPHZ 26 (39), tPLZ 28 (42)

RCO paths at RL 1 kΩ, CL 30 pF:
- CCK↑ → RCO: tPLH 14 (21), tPHL 20 (30)
- CLOAD↓ → RCO: tPLH 31 (47), tPHL 27 (41)
- CCLR↓ → RCO (tPLH): 30 (45)
- RCK↑ → RCO (CLOAD = L): tPLH 42 (63), tPHL 33 (50)

fmax: 20 min / 35 typ MHz.

**VCC (p6):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.
- Off-state output voltage (including the I/O ports): 5.5 V abs max.

**Obsolete:** SN74LS593DW. SN74LS593DWR is Last Time Buy.

**Notes:**
- The p6 setup row "CCKEN̅ low, 'LS592 / CCKEN̅ low or CCKEN high, 'LS593" matches the enable table.
- The '593 IIL for RCK is −0.8 mA versus −0.2 mA on the '592 (p7). This is irrelevant to logic.

## 74_697

- **Title / function:** "SYNCHRONOUS UP/DOWN COUNTERS WITH OUTPUT REGISTERS AND MULTIPLEXED 3-STATE OUTPUTS"; 'LS697 . . . Binary Counter, Direct Clear (sn74ls697.pdf p1).
- **Datasheets:** counters/sn74ls697.pdf.
  - p1: pinouts incl. FK, description.
  - p2: symbols.
  - p4: 'LS697 logic diagram.
  - p7: ROC.
  - p8: electrical and switching.
  - p10: typical sequence.
- **Families & prefixes:**
  - LS: SN74LS697* and SNJ54LS697* (census: SN74LS697DW, SN74LS697N, SNJ54LS697J).
  - LS696 (decade, async clear) and LS699 (binary, sync clear) are in the same sheet but have no census rows.
- **Military:** none (no SMD rows). SNJ54LS697J is a plain SNJ part.

**Pinout p20** (J, W, DW, N). From p1:

| pin | name | role | notes |
|---|---|---|---|
| 1 | U/D | input | printed U/D̅: up when H, down when L |
| 2 | CCK | input | |
| 3 | A | input | |
| 4 | B | input | |
| 5 | C | input | |
| 6 | D | input | |
| 7 | ENP | input | active low |
| 8 | CCLR | input | active low |
| 9 | RCK | input | |
| 10 | GND | ground | |
| 11 | R/C | input | printed R/C̅ |
| 12 | G | input | active low |
| 13 | LOAD | input | active low |
| 14 | ENT | input | active low |
| 15 | QD | output | 3-state |
| 16 | QC | output | 3-state |
| 17 | QB | output | 3-state |
| 18 | QA | output | 3-state |
| 19 | RCO | output | active low |
| 20 | VCC | power | |

A is the LSB and D the MSB.

**FK pinout fk20** (SN54LS697 only, no census rows). From p1:

| pin | name | role |
|---|---|---|
| 1 | U/D | input |
| 2 | CCK | input |
| 3 | A | input |
| 4 | B | input |
| 5 | C | input |
| 6 | D | input |
| 7 | ENP | input |
| 8 | CCLR | input |
| 9 | RCK | input |
| 10 | GND | ground |
| 11 | R/C | input |
| 12 | G | input |
| 13 | LOAD | input |
| 14 | ENT | input |
| 15 | QD | output |
| 16 | QC | output |
| 17 | QB | output |
| 18 | QA | output |
| 19 | RCO | output |
| 20 | VCC | power |

The FK numbering is the same as the DIP. There are no NC pins.

**Behavior.** The sheet prints **no function table**. Everything below comes from the p1 text, the p2 IEEE symbol (quoted terms) and the p4 diagram.
- **Blocks:**
  - a 4-bit synchronous up/down binary counter;
  - a 4-bit D register loaded from the counter;
  - a 2:1 mux (register or counter) feeding 3-state outputs QA–QD.
- **CCLR:** active low and **asynchronous** for the 'LS697 ("asynchronous on the 'LS696 and 'LS697, synchronous on the 'LS699", p1; symbol "R8"). It clears **the counter only**: in the p4 diagram only the 1D/C1/R counter flip-flops have R, and the 2D/C2 register has none. Recovery: CCLR inactive before CCK↑, 25 ns. tw(CCLR low) is 20 ns.
- **LOAD:** active low and **synchronous**: "Loading of the counter is accomplished when LOAD is taken low and a positive transition occurs on the counter clock CCK" (p1). Symbol: M1 [LOAD] when low, M2 [COUNT] when high, data inputs "1,7D". **The load does not depend on ENP or ENT.**
- **Count:** on CCK↑ when LOAD = H, **ENP = L and ENT = L**. The direction is set by U/D: H = up (M3, "2,3,5,6+") and L = down (M4, "2,4,5,6−"). Count modulo 16 in both directions.
  - Setups before CCK↑: A–D 30 ns; ENP or ENT 30 ns; LOAD 30 ns; U/D 35 ns.
- **Counter priority:** CCLR (async) > LOAD (sync) > count (needs ENP = ENT = L) > hold.
- **RCO:** active low, driven from the **counter** state, not the register.
  - Symbol: "3,5CT=15 → Z22", "4,5CT=0 → V23", and "22,23" → RCO.
  - So **RCO = L ⇔ ENT = L AND ((U/D = H AND count = 15) OR (U/D = L AND count = 0))**.
  - ENP is not in the RCO term. RCO is combinational in ENT, as the ENT→RCO delay row shows.
  - RCO is a totem-pole output and is not 3-state.
  - Cascading: RCO of stage 1 → ENT of stage 2, etc. "All ENP inputs can be tied common and used as a master enable or disable control."
- **Register:** loads the counter outputs on **RCK↑** (symbol C11, "11D"). There is no enable and no clear.
  - p7 note 2: "The clocks may be tied together in which case the register state will be one clock pulse behind the counter." Setup: CCK↑ before RCK↑, 30 ns.
- **R/C:** selects what drives the outputs. "selects the counter when low and the register when high for the three-state outputs" (p1). Symbol: G21, with "21" on the register path and "21̅" on the counter path.
- **G:** active low output enable (symbol EN24). G = L → QA–QD driven. G = H → Hi-Z.
  - p10 sequence: Hi-Z while G is high; the counter keeps counting.
- **Output equation:** Qn = G̅ ? (R/C ? reg[n] : ctr[n]) : Z.
- **Typical sequence** (p10, 'LS697): async clear, sync load of 13, count up 14, 15 (RCO low at 15 while counting up), 0, 1, 2, 3 …, then count down … 0 → 15 (RCO low at 0 while counting down), then inhibit when ENP/ENT go high.
- **Power-up:** not stated.

**Timing** (p8, VCC 5 V, 25 °C; typ, max in parentheses).

RCO paths at RL 2 kΩ, CL 15 pF:
- CCK↑ → RCO: tPLH 23 (40), tPHL 23 (40)
- ENT → RCO: 13 (20) / 13 (20)

Q paths at RL 667 Ω, CL 45 pF:
- CCK↑ → Q (counter selected): tPLH 12 (20), tPHL 17 (25)
- RCK↑ → Q: 12 (20) / 17 (25)
- CCLR↓ → Q (tPHL): 23 (40)
- R/C → Q: 16 (25) / 16 (25)
- G↓ → Q: tPZH 19 (30), tPZL 19 (30)

Disable path at 5 pF:
- G↑ → Q: tPHZ 17 (30), tPLZ 17 (30)

Clock frequency: CCK and RCK 0–20 MHz (ROC p7).

**VCC (p7):**
- SN54LS: 4.5–5.5 V.
- SN74LS: 4.75–5.25 V.

**Obsolete:** none.

**Notes:**
- The p1 description mentions "enable P̅ and enable T̅", meaning the ENP and ENT pins.
- The symbol's register box carries an "8" label next to "11D" and "21̅". In IEEE notation this suggests the R8 clear also affects that cell. However, the p4 logic diagram shows no R input on the register (2D/C2) flip-flops. The diagram was followed: CCLR clears the counter only. A CCLR with R/C = L (counter selected) clears the outputs, which matches "CCLR↓ → Q".

---

## 74_8154

- Title: "SN74LV8154 Dual 16-Bit Binary Counters With 3-State Output Registers".
- Datasheet: counters/sn74lv8154.pdf. Pages used: p1, p3 (pinout/pin functions), p4 (abs max, rec. op.), p5 (timing req.), p6 (switching), p7 (Figure 1 timing diagram), p9 (overview, block diagram, feature description), p10 (Table 2), p11–12 (application, Table 3).
- Families & prefixes: LV: SN74LV8154*.
- Military numbers: none (single-function PDF; all rows are this device).

### Pinout
p20: N (PDIP), PW (TSSOP) (p3)
```
1 CLKA in        11 CCLR in   (active low)
2 CLKB in        12 Y7 out    (MSB, 3-state)
3 GAL in (act L) 13 Y6 out
4 GAU in (act L) 14 Y5 out
5 GBL in (act L) 15 Y4 out
6 GBU in (act L) 16 Y3 out
7 RCLK in        17 Y2 out
8 RCOA out (act L, totem-pole, not 3-state)
9 CLKBEN in (act L)  18 Y1 out
10 GND ground    19 Y0 out    (LSB)
                 20 VCC power
```
Naming caveat: the pin table and drawing (p3) use CLKA/CLKB/CLKBEN. The block diagram (p9), timing diagram (p7) and §8.3 call them CCKA/CCKB/CCKBEN. The pin-table names are the ones to use.

### Behavior
- Two independent 16-bit binary up-counters, A and B (p9 §8.3). "All clocks are positive edge triggered. All other inputs are active low." (p1)
- CLKA↑: counter A += 1 (mod 65536). There is no enable for A.
- CLKB↑: counter B += 1 (mod 65536) **only if CLKBEN = L**. The enable is synchronous: setup is "CLKBEN low before CLKB↑" = 10 ns and hold is "CLKBEN low after CLKB↑" = 0 ns (p5). On p7, CCKBEN high for one clock makes B hold at 0100 while A advances.
- CCLR: active low and asynchronous. It resets **both** counters to 0 (p3, p9). It does not clear the storage register (p9 block diagram: CCLR goes only to the counter blocks). Recovery is "CCLR high (inactive) before CLKA↑ or CLKB↑" setup = 10 ns. Priority: CCLR over CLKA/CLKB.
- RCOA: active low. RCOA = L when counter A = 0xFFFF ("active low when counter A is full count and ready to overflow on next clock A", p3). It is a pure decode of counter A: it is not gated by CLKA (p7 shows it low for the whole FFFF state) and it is not affected by CLKBEN or B. CCLR forces it high (tPLH CCLR→RCOA). RCOA is always driven; it is not 3-state.
- 32-bit cascade (p9 §8.1, p12): tie CLKA and CLKB together and connect RCOA to CLKBEN. B then increments on the common clock edge at which A rolls FFFF→0000, because RCOA = L (A = FFFF) is sampled at that same edge. If only one counter is needed, the p12 application says to drive both clocks from one source with CLKBEN = RCOA.
- RCLK↑: one 32-bit storage register (A 16 bits and B 16 bits) loads both counters at once. It holds until the next RCLK↑ (p9 §8.3). Timing is setup "CLKA↑ or CLKB↑ before RCLK↑" 10 ns and hold "CLKA or CLKB after RCLK" 0 ns (p5). If RCLK is tied to the count clock, the register gets the pre-edge count (p7: on the first tied edges the output shows 00, 01, 02 while the counters show 0001, 0002, 0003).
- Output bus Y7..Y0: 3-state, driven from the storage register (never directly from the counters), selected by Table 2 (p10). Quoted:
  ```
  GAL GAU GBL GBU | Yn
   L   H   H   H  | Lower byte in A storage register
   H   L   H   H  | Upper byte in A storage register
   H   H   L   H  | Lower byte in B storage register
   H   H   H   L  | Upper byte in B storage register
   H   H   H   H  | Z
  ```
  Lower byte = bits 7..0 → Y7..Y0 (Y0 = LSB); upper byte = bits 15..8.
  In the p9 block diagram the four G inputs are buffered (non-inverting) into a 4-input AND that drives the output-buffer enables (OE = NOT(GAL·GAU·GBL·GBU), so the outputs are enabled when any G is low). The G signals also act as selects for eight "4 to 1 Dec" muxes. **More than one G low at a time is not specified.** The datasheet also requires tz = 200 ns min ("GAL, GAU, GBL, GBU all high before one of them switches low", p5). Setups are "RCLK↑ before GAL or GAU or GBL or GBU low" 10 ns and "G high (inactive) before RCLK↑" 10 ns.
- Power-up: not stated. To keep the bus in Hi-Z through power up/down, p9 recommends tying the G pins to VCC through pull-ups. Ioff: the outputs are disabled when VCC = 0.
- p12 Table 3 is only an application decode with an LVC138 (C/B/A address → G pins, RCLK, CCLR). It is not chip behavior.

### Timing (LV, typ, CL = 50 pF; p6)
| path | 3.3 V ± 0.3 V typ (max) | 5 V ± 0.5 V typ (max) |
|---|---|---|
| RCLK → Y (tpd) | 25 (42) | 16 (27) |
| CLKA → RCOA (tpd) | 28 (46) | 17 (28) |
| CCLR → RCOA (tPLH) | 20 (35) | 13 (21) |
| G → Y enable (ten) | 30 (50) | 18 (30) |
| G → Y disable (tdis) | 14 (24) | 9 (16) |
- CL = 15 pF at 3.3 V: 22 / 26 / 18 / 27 / 12. CL = 15 pF at 5 V: 14 / 16 / 12 / 16 / 8.
- fMAX (min): 25 MHz at 50 pF and 40 MHz at 15 pF, at both voltages.
- The datasheet prints no clock→Y path; Y comes only via RCLK.
- Timing requirements are given only at 5 V (p5): tw CLK/RCLK 10 ns; CCLR low 20 ns.
- VCC: 2–5.5 V (p4). inputMax 5.5 V (VI 0–5.5 V at any VCC; abs max 7 V). In the 3-state or power-off condition the outputs tolerate 5.5 V.
- Obsolete (census): SN74LV8154PW.
- Notes:
  - The datasheet says nothing about CLKBEN gating producing a clock edge, or about whether B can count while RCOA is glitching.
  - Pin-name inconsistency CLK vs CCK: see the naming caveat under Pinout.
  - The p1 feature "Maximum tpd of 25 ns at 5 V (RCLK to Y)" matches the 15 pF max at 5 V.

---

## 4020B

- Title: "CMOS Ripple-Carry Binary Counter/Dividers", CD4020B: 14 stage (cd4020b.pdf p1).
- Datasheets: counters/cd4020b.pdf p1 (description, terminal assignment, functional diagram),
  p2 (recommended operating conditions, Fig. 1 logic diagram), p4 (dynamic characteristics).
- Families & prefixes: C: CD4020B* (CD4020BE, CD4020BEE4, CD4020BF, CD4020BF3A, CD4020BNSR,
  CD4020BPW, CD4020BPWR).
- Military: JM38510/05653BEA and M38510/05653BEA (16-pin CDIP, Active) → **4020B, inferred**.
  The 14-pin 05655BCA can only be the 4024B. Slash-sheet /056 device numbers in the census are
  01 = CD4017B (cd4017b.pdf) and 02 = CD4018B (cd4018b.pdf). That ascending order makes
  03 = 4020 and 05 = 4024 (04 would be 4022). The PDF text does not state this; the
  addendum lists only "CD4020B-MIL, CD4024B-MIL, CD4040B-MIL".
- Pinout p16: E (N PDIP), F/F3A (J CDIP), NSR (NS SOP), PW (TSSOP); p1 terminal assignments:
  `1 Q12 output`, `2 Q13 output`, `3 Q14 output`, `4 Q6 output`, `5 Q5 output`, `6 Q7 output`,
  `7 Q4 output`, `8 VSS ground`, `9 Q1 output`, `10 φ input` (clock; "INPUT PULSES" in the functional
  diagram; suggest the name `CLK`), `11 RESET input`, `12 Q9 output`, `13 Q8 output`, `14 Q10 output`,
  `15 Q11 output`, `16 VDD power`.
- Stages brought out: **Q1, Q4, Q5, Q6, Q7, Q8, Q9, Q10, Q11, Q12, Q13, Q14** (12 outputs).
  **Q2 and Q3 are internal only.** The functional diagram on p1 says "12 BUFFERED OUTPUTS".
- Behavior (p1 text, p2 Fig. 1):
  * 14-stage binary ripple counter. All stages are master-slave flip-flops. The counter advances one
    count on the **negative (falling) transition** of φ. The φ input has a Schmitt trigger, so rise and
    fall times are unlimited.
  * RESET is **active high and asynchronous**. A high level resets the counter to all zeros (all Q
    low). Priority: RESET > clock. In Fig. 1, φ and R pass through input buffers into a NOR gate,
    so RESET also blocks the clock into FF1. Every FF has an R input tied to the reset line.
  * Ripple: FF_n is clocked by the falling edge of Q_(n−1). The count is binary 0 … 16383
    (mod 2^14) and wraps to 0.
  * Outputs are buffered and non-inverted. There are no illegal states.
  * Power-up state: not stated.
- Timing (p4, TA = 25 °C, VDD = 5 V, CL = 50 pF; typ / max):
  * φ → Q1, tPHL/tPLH: **180** / 360 ns.
  * Qn → Qn+1, tPHL/tPLH: **100** / 330 ns.
  * Reset → Q, tPHL: **140** / 280 ns.
  * Transition time: 100 / 200 ns.
  * Minimum input-pulse width: typ 70, max 140 ns.
  * Minimum reset pulse width: typ 100, max 200 ns.
  * Reset removal time: typ 175, max 350 ns.
  * fmax: min 3.5, typ 7 MHz.
  * No clock → Qn for the last stage is printed. Derive it as φ→Q1 + (n−1)·(Qn→Qn+1).
- VDD: recommended 3 to 18 V (p2).
- Obsolete: CD4020BPW.
- Notes: the p4 table is one shared table for 4020B, 4024B and 4040B.

## 4024B

- Title: same sheet, "CD4024B — 7 Stage" (cd4020b.pdf p1).
- Datasheets: counters/cd4020b.pdf p1, p2 (Fig. 2), p4.
- Families & prefixes: C: CD4024B* (CD4024BE, CD4024BEE4, CD4024BF, CD4024BF3A, CD4024BM,
  CD4024BM96, CD4024BMT, CD4024BNSR, CD4024BPW, CD4024BPWR).
- Military: JM38510/05655BCA and M38510/05655BCA (14-pin CDIP, NRND) → 4024B. It is the only
  14-pin function in the PDF.
- Pinout p14: E (N), F/F3A (J), M/M96/MT (D SOIC), NSR (NS), PW: `1 φ input` (clock; suggest
  `CLK`), `2 RESET input`, `3 Q7 output`, `4 Q6 output`, `5 Q5 output`, `6 Q4 output`, `7 VSS ground`,
  `8 NC nc`, `9 Q3 output`, `10 NC nc`, `11 Q2 output`, `12 Q1 output`, `13 NC nc`, `14 VDD power`.
- Stages brought out: **Q1–Q7, all of them** ("7 BUFFERED OUTPUTS").
- Behavior: same as 4020B, with 7 stages. The count is mod 128. Fig. 2 has the same φ/R NOR input
  gating as Fig. 1.
- Timing: same table as 4020B (p4). φ→Q1 typ 180 ns; Qn→Qn+1 typ 100 ns; reset→Q typ 140 ns
  (VDD 5 V, 50 pF).
- VDD: 3 to 18 V.
- Obsolete: CD4024BM, CD4024BMT, CD4024BPW. CD4024BNSR is NRND.

## 4040B

- Title: same sheet, "CD4040B — 12 Stage" (cd4020b.pdf p1).
- Datasheets: counters/cd4020b.pdf p1, p2 (Fig. 3), p4.
- Families & prefixes: C: CD4040B* (CD4040BE, CD4040BEE4, CD4040BF, CD4040BF3A, CD4040BM,
  CD4040BM96, CD4040BNSR, CD4040BPW, CD4040BPWR, CD4040BPWRG4).
- Military: none in the census. 05653 is assigned to 4020B; see 4020B → Military.
- Pinout p16: E, F/F3A, M/M96, NSR, PW: `1 Q12 output`, `2 Q6 output`, `3 Q5 output`, `4 Q7 output`,
  `5 Q4 output`, `6 Q3 output`, `7 Q2 output`, `8 VSS ground`, `9 Q1 output`, `10 φ input` (suggest
  `CLK`), `11 R input` (printed "R" in the pinout and "RESET" in the functional diagram; suggest
  `RESET`), `12 Q9 output`, `13 Q8 output`, `14 Q10 output`, `15 Q11 output`, `16 VDD power`.
- Stages brought out: **Q1–Q12, all of them**.
- Behavior: same as 4020B, with 12 stages. The count is mod 4096.
- Timing: same table (p4). φ→Q1 typ 180 ns; Qn→Qn+1 typ 100 ns; reset→Q typ 140 ns.
- VDD: 3 to 18 V.
- Obsolete: CD4040BPW. CD4040BF3A is Last Time Buy.

---

## 74_4020

- Title: "CDx4HC4020, CDx4HCT4020 High-Speed CMOS Logic 14-Stage Binary Counter"
  (cd74hc4020.pdf p1); "SNx4HC4020 14-Bit Asynchronous Binary Counters" (sn74hc4020.pdf p1).
- Datasheets: counters/cd74hc4020.pdf p1, p3 (pinout), p4 (ROC), p6–7 (switching),
  p10 (truth table); counters/sn74hc4020.pdf p1, p3 (pinouts incl. FK), p4 (ROC), p5 (timing),
  p6 (switching), p8 (function table).
- Families & prefixes:
  * HC: CD54HC4020*, CD74HC4020*, SN54HC4020*, SN74HC4020*, SNJ54HC4020*.
  * HCT: CD54HCT4020*, CD74HCT4020*.
- Military:
  * 5962-8945801EA → CD54HCT4020F3A (HCT). Source: cd74hc4020 census mark.
  * 85003012A (FK), 8500301EA (J), 8500301FA (W) → SNJ54HC4020FK/J/W (HC). Source: sn74hc4020.
  * CD54HC4020F3A is also marked 8500301EA.
- Pinouts:
  * p16, CD naming (cd74hc4020 p3; J, N, D): `1 Q12 output`, `2 Q13 output`, `3 Q14 output`,
    `4 Q6 output`, `5 Q5 output`, `6 Q7 output`, `7 Q4 output`, `8 GND ground`, `9 Q1 output` (printed
    "Q1'"), `10 CP input` (printed with an overbar, i.e. falling-edge clock), `11 MR input`,
    `12 Q9 output`, `13 Q8 output`, `14 Q10 output`, `15 Q11 output`, `16 VCC power`.
  * p16, SN naming (sn74hc4020 p3; J, W, D, DB, N, NS, PW): `1 QL output`, `2 QM output`,
    `3 QN output`, `4 QF output`, `5 QE output`, `6 QG output`, `7 QD output`, `8 GND ground`,
    `9 QA output`, `10 CLK input`, `11 CLR input`, `12 QI output`, `13 QH output`, `14 QJ output`,
    `15 QK output`, `16 VCC power`. This is pin-for-pin the CD map with QA=Q1, QD=Q4 … QN=Q14.
  * fk20 (SNJ54HC4020FK, sn74hc4020 p3): `1 NC nc`, `2 QL output`, `3 QM output`, `4 QN output`,
    `5 QF output`, `6 NC nc`, `7 QE output`, `8 QG output`, `9 QD output`, `10 GND ground`,
    `11 NC nc`, `12 QA output`, `13 CLK input`, `14 CLR input`, `15 QI output`, `16 NC nc`,
    `17 QH output`, `18 QJ output`, `19 QK output`, `20 VCC power`.
  * The 4000-series pinout is identical (CD4020B).
- Stages brought out: Q1, Q4–Q14 (QA, QD–QN). **Q2/QB and Q3/QC are not pinned out.**
- Behavior:
  * 14-stage ripple counter. It advances on the **falling edge** of CP/CLK. The truth table
    reads: CP ↑, MR L → no change; CP ↓, MR L → advance to next state; MR H, CP X → all outputs low
    (cd74hc4020 p10; sn74hc4020 p8).
  * MR/CLR is **active high and asynchronous**: "reset to zero (all outputs low) independently of
    the clock" (sn74hc4020 p1, p8). Priority: CLR > CLK.
  * Recovery requirement: "Setup time, CLR inactive before CLK↓" (sn74hc4020 p5) /
    tREM (cd74hc4020 p6).
  * The count is binary mod 16384. There are no illegal states.
- Timing (25 °C):
  * HC, cd74hc4020 p6 (no typ at 4.5 V/50 pF; MAX at 4.5 V, 50 pF):
    - CP → Q1: **28 ns max** (typ 11 ns at 5 V, 15 pF).
    - Qn → Qn+1: **15 ns max** (typ 6 at 5 V, 15 pF).
    - MR → Qn: **34 ns max** (typ 14 at 5 V, 15 pF).
  * HC, sn74hc4020 p6 (4.5 V, 50 pF, typ / max):
    - CLK → QA: **16** / 30 ns.
    - CLR → any (tPHL): **17** / 28 ns.
    - Qn → Qn+1 is not printed.
  * HCT, cd74hc4020 p7 (4.5 V, 50 pF, max):
    - CP → Q1: **40 ns** (typ 17 at 5 V, 15 pF).
    - Qn → Qn+1: **15 ns** (typ 6 at 5 V, 15 pF).
    - MR → Qn: **40 ns** (typ 17 at 5 V, 15 pF).
  * No clock → last-stage delay is printed.
- VCC:
  * HC: 2–6 V (cd74hc4020 p4; sn74hc4020 p4, SN54 and SN74 rows the same).
  * HCT: 4.5–5.5 V.
- Obsolete: CD74HC4020M, CD74HCT4020M, CD74HCT4020MT, SN74HC4020D, SN74HC4020PW, SN74HC4020PWT.
  SN74HC4020NSR is Last Time Buy.
- Notes:
  * sn74hc4020 p3 lists DB in the package drawing, but there is no DB row in the census.
  * cd74hc4020 calls the reset "MR" on p1 and p10 but "CLR" in the overview text on p9.

## 74_4024

- Title: "CDx4HC4024, CDx4HCT4024 High-Speed CMOS Logic 7-Stage Binary Ripple Counter"
  (cd74hc4024.pdf p1).
- Datasheets: counters/cd74hc4024.pdf p1, p3 (pinout), p4 (ROC), p6 (prerequisites),
  p7 (switching), p9 (truth table).
- Families & prefixes:
  * HC: CD54HC4024*, CD74HC4024*.
  * HCT: CD54HCT4024*, CD74HCT4024*.
- Military: none (all rows are commercial or CD54 F/F3A).
- Pinout p14 (J, N, D, PW; p3): `1 CP input` (overbar, falling edge), `2 MR input`, `3 Q7 output`,
  `4 Q6 output`, `5 Q5 output`, `6 Q4 output`, `7 GND ground`, `8 NC nc`, `9 Q3 output`,
  `10 NC nc`, `11 Q2 output`, `12 Q1 output` (printed "Q1'"), `13 NC nc`, `14 VCC power`.
  This is identical to CD4024B.
- Stages brought out: Q1–Q7 (all).
- Behavior:
  * Truth table (p9): CP ↑, MR L → no change; CP ↓, MR L → advance to next state; MR H → all
    outputs low.
  * MR is active high and asynchronous, and overrides the clock.
  * The count is mod 128.
- Timing (p7, 25 °C, max at 4.5 V/50 pF; typ given only at 5 V/15 pF):
  * HC: CP → Q1 **28** (typ 11); Qn → Qn+1 **15** (typ 6); MR → Qn **34** (typ 14).
  * HCT: CP → Q1 **40** (typ 17); Qn → Qn+1 **15** (typ 6); MR → Qn **40** (typ 17).
- VCC: HC 2–6 V; HCT 4.5–5.5 V (p4).
- Obsolete: CD74HC4024M, CD74HC4024MT, CD74HC4024PW.
- Notes: in the p7 table, the HC 6 V row reads "13 13 19" and the MR row at 6 V reads "29 27 43".
  The middle values look like typos for 16 and 37 (the 4020 sheet prints 16 and 37).

## 74_4040

- Title: "CDx4HC4040, CDx4HCT4040 High-Speed CMOS Logic 12-Stage Binary Counter"
  (cd74hc4040.pdf p1); "SNx4HC4040 12-Bit Asynchronous Binary Counters" (scls160e.pdf p1);
  "SN74LV4040A 12-Bit Asynchronous Binary Counters" (sn74lv4040a.pdf p1).
- Datasheets:
  * counters/cd74hc4040.pdf p1, p3 (pinout), p4 (ROC), p6–7 (switching), p11 (function table).
  * counters/scls160e.pdf p1, p3 (pinouts incl. FK), p4, p5, p6, p8.
  * counters/sn74lv4040a.pdf p1, p3 (pinouts incl. RGY, pin table), p4–5 (ROC), p6–7 (switching),
    p9 (function table).
- Families & prefixes:
  * HC: CD54HC4040*, CD74HC4040*, SN54HC4040*, SN74HC4040*, SNJ54HC4040*.
  * HCT: CD54HCT4040*, CD74HCT4040*.
  * LV: SN74LV4040A*.
- Military:
  * 5962-8994701MEA → CD54HCT4040F3A (HCT).
  * 85004012A (FK), 8500401EA (J), 8500401FA (W) → SNJ54HC4040FK/J/W (HC).
  * CD54HC4040F3A is also marked 8500401EA.
- Pinouts:
  * p16, CD naming (cd74hc4040 p3; J, D, N): `1 Q12 output`, `2 Q6 output`, `3 Q5 output`,
    `4 Q7 output`, `5 Q4 output`, `6 Q3 output`, `7 Q2 output`, `8 GND ground`, `9 Q1 output`,
    `10 CP input` (falling edge), `11 MR input`, `12 Q9 output`, `13 Q8 output`, `14 Q10 output`,
    `15 Q11 output`, `16 VCC power`.
  * p16, SN naming (scls160e p3: J, W, D, DB, N, NS, PW; sn74lv4040a p3: D, DB, DGV, N, NS, PW):
    `1 QL output`, `2 QF output`, `3 QE output`, `4 QG output`, `5 QD output`, `6 QC output`,
    `7 QB output`, `8 GND ground`, `9 QA output`, `10 CLK input`, `11 CLR input`, `12 QI output`,
    `13 QH output`, `14 QJ output`, `15 QK output`, `16 VCC power`.
  * rgy16 (SN74LV4040ARGY, VQFN 16; sn74lv4040a p3): **same numbering 1–16 as the DIP map
    above**. The drawing shows an exposed centre pad (dashed outline); its connection is not stated.
  * fk20 (SNJ54HC4040FK, scls160e p3): `1 NC nc`, `2 QL output`, `3 QF output`, `4 QE output`,
    `5 QG output`, `6 NC nc`, `7 QD output`, `8 QC output`, `9 QB output`, `10 GND ground`,
    `11 NC nc`, `12 QA output`, `13 CLK input`, `14 CLR input`, `15 QI output`, `16 NC nc`,
    `17 QH output`, `18 QJ output`, `19 QK output`, `20 VCC power`.
  * The CD4040B pinout is identical.
- Stages brought out: Q1–Q12 / QA–QL (all).
- Behavior:
  * Advances on the CLK **falling edge**. CLR/MR is **active high and asynchronous** and sets all
    outputs low (scls160e p1 and p8; sn74lv4040a p3 pin table: "CLK Clock, falling edge
    triggered", "CLR Clear, active high").
  * Function table: CLK ↑, CLR L → no change; CLK ↓, CLR L → advance; CLR H → all outputs L.
  * The count is mod 4096.
  * LV: Ioff partial power-down (outputs are disabled when VCC = 0); inputs tolerate 5.5 V.
- Timing:
  * HC, cd74hc4040 p6 (25 °C, max at 4.5 V/50 pF):
    - CP → Q1: **28** (typ 11 at 5 V, 15 pF).
    - Qn → Qn+1: **15** (typ **4** at 5 V, 15 pF).
    - MR → Qn: **34** (typ 14).
  * HCT, cd74hc4040 p7 (4.5 V/50 pF max): CP → Q1 **40** (typ 17); Qn → Qn+1 **15** (typ 4);
    MR → Qn **40** (typ 17).
  * HC, scls160e p6 (4.5 V, 50 pF, typ / max):
    - CLK → QA: **16** / 30 ns.
    - CLR → any (tPHL): **17** / 28 ns.
    - Qn → Qn+1 is not printed.
  * LV, sn74lv4040a p6–7 (3.3 V ± 0.3 V, 50 pF, typ / max at 25 °C / max over −40…85 °C):
    - CLK → QA: **7.5** / 15.4 / 17.5 ns.
    - CLR → any Q (tPHL): **9** / 16.3 / 18.5 ns.
    - **Δtpd Qn → Qn+1: 1.2** / 4.4 / 5 ns.
  * LV at 5 V, 50 pF (p7): CLK → QA typ 5.3; CLR typ 6.8; Δtpd typ 0.8 ns.
- VCC and inputMax:
  * HC: 2–6 V.
  * HCT: 4.5–5.5 V.
  * LV: 2–5.5 V, with inputMax 5.5 V (sn74lv4040a p4: "VI Input voltage 0 5.5").
- Obsolete: CD74HC4040M, CD74HCT4040M, SN74HC4040D, SN74HC4040DT, SN74HC4040PW, SN74HC4040PWT,
  SN74LV4040AD, SN74LV4040APW, SN74LV4040APWT.
- Notes:
  * Datasheet error in cd74hc4040 p1 and p9: the text says "'HC4040 and 'HCT4040 are 14-stage
    ripple-carry binary counters", but the title and pinout give 12 stages.
  * sn74lv4040a p1 lists switch-type features ("Low crosstalk between switches", "Individual
    switch controls"). These are copy errors.
  * cd74hc4040 has no CD74 PW or NS rows in the census (the thermal table mentions NS).
