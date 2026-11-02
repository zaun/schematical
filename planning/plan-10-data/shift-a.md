# Plan-10 extraction — group shA (plain shift registers)

Source PDFs are under `datasheets/` (paths below are relative to it). Page numbers are PDF page indexes (1-based). Scanned pages were read from 300-dpi renders (OCR + visual check). Typical values use the brief's reference conditions; where a datasheet prints no typical at that condition, the MAX is given and labelled.

## PDF → devices

| PDF | devices |
|---|---|
| shifters/cd4014b.pdf | 4014B, 4021B |
| shifters/cd4015b.pdf | 4015B |
| shifters/cd40194b.pdf | 40194B |
| shifters/cd4035b.pdf | 4035B |
| shifters/cd74ac164.pdf | 74_164 (AC, ACT) |
| shifters/cd74ac299.pdf | 74_299 (AC, ACT), 74_323 (AC) |
| shifters/cd74hc164.pdf | 74_164 (HC, HCT) |
| shifters/cd74hc165.pdf | 74_165 (HC, HCT) |
| shifters/cd74hc166.pdf | 74_166 (HC, HCT) |
| shifters/cd74hc194.pdf | 74_194 (HC, HCT) |
| shifters/cd74hc195.pdf | 74_195 (HC) |
| shifters/cd74hc299.pdf | 74_299 (HC, HCT) |
| shifters/cd74hc4015.pdf | 74_4015 (HC) |
| shifters/sn74ahc164-q1.pdf | 74_164 (AHC, -Q1) |
| shifters/sn74ahc164.pdf | 74_164 (AHC) |
| shifters/sn74ahc165.pdf | 74_165 (AHC) |
| shifters/sn74ahct164.pdf | 74_164 (AHCT) |
| shifters/sn74als299.pdf | 74_299 (ALS) |
| shifters/sn74f299.pdf | 74_299 (F) |
| shifters/sn74hc164.pdf | 74_164 (HC) |
| shifters/sn74hc165-q1.pdf | 74_165 (HC, -Q1) |
| shifters/sn74hc165.pdf | 74_165 (HC) |
| shifters/sn74hc166.pdf | 74_166 (HC) |
| shifters/sn74hcs164-q1.pdf | 74_164 (HCS, -Q1) |
| shifters/sn74hcs164.pdf | 74_164 (HCS) |
| shifters/sn74hcs165-q1.pdf | 74_165 (HCS, -Q1) |
| shifters/sn74hcs165.pdf | 74_165 (HCS) |
| shifters/sn74hcs264.pdf | 74_264 (HCS) |
| shifters/sn74hct165.pdf | 74_165 (HCT) |
| shifters/sn74ls164.pdf | 74_164 (LS; std TTL in text only) |
| shifters/sn74ls165a.pdf | 74_165 (LS; std TTL obsolete, text only) |
| shifters/sn74ls166a.pdf | 74_166 (LS, TTL) |
| shifters/sn74ls194a.pdf | 74_194 (LS, S; std TTL text only) |
| shifters/sn74ls299.pdf | 74_299 (LS; S text only) |
| shifters/sn74lv164a-q1.pdf | 74_164 (LV, -Q1) |
| shifters/sn74lv164a.pdf | 74_164 (LV) |
| shifters/sn74lv165a-q1.pdf | 74_165 (LV, -Q1) |
| shifters/sn74lv165a.pdf | 74_165 (LV) |
| shifters/sn74lv166a.pdf | 74_166 (LV) |
| shifters/sn74lvc165a.pdf | 74_165 (LVC) |

## Census rows that could not be mapped

- `shifters/cd4014b.pdf`: **JM38510/05754BEA**, **M38510/05754BEA** — the PDF covers both CD4014B and CD4021B and never says which one the slash-sheet number is (package addendum only lists "Military: CD4014B-MIL, CD4021B-MIL").
- `shifters/sn74ls194a.pdf`: **JM38510/07601BEA**, **M38510/07601BEA** — mapped by inference only (S194; the PDF prints no cross-reference). The 30601 rows are LS194A and 7604001 rows are S194 by their marks.
- All other military/SMD rows are mapped in each device section.

## Families with no current orderable rows (text only)

SN54/74164 (TTL), SN54/74165 (TTL, declared obsolete), SN54/74194 (TTL), SN74S194, SN54/74S299, SN54F299 (declared obsolete), SN74166 (TTL, 74 side). CD74HCT195 / ACT323 / HCT4015 are mentioned or implied in text but have no rows.

## 74_164

- **Title / function:** "8-Bit Parallel-Out Serial Shift Registers" (SN74HC164 p1); "8-Bit Serial-In, Parallel-Out Shift Register" (CD74HC164 p1, CD74AC164 p1). 8-bit SIPO, AND-gated serial inputs A·B, asynchronous active-low clear.
- **Datasheets:** `shifters/sn74ls164.pdf` (scanned, pp1–6; covers SN54/74164 std TTL and SN54/74LS164), `shifters/sn74hc164.pdf` (pp1,4–6,10,14), `shifters/cd74hc164.pdf` (pp1,3,4,7,9), `shifters/cd74ac164.pdf` (pp1,3,4,7,10), `shifters/sn74ahc164.pdf` (pp1,3,4,6,7,10), `shifters/sn74ahc164-q1.pdf` (pp3,4,6,7,11), `shifters/sn74ahct164.pdf` (pp1,3,4,5,6,9), `shifters/sn74hcs164.pdf` (pp1,3,4,6,12), `shifters/sn74hcs164-q1.pdf` (pp3,4,6), `shifters/sn74lv164a.pdf` (pp1,3,5,7,8,11), `shifters/sn74lv164a-q1.pdf` (pp1,3,5,7).
- **Families & prefixes:**
  - TTL: SN54164, SN74164 (in sn74ls164.pdf text only; no census rows)
  - LS: SN54LS164*, SN74LS164*, SNJ54LS164*
  - HC: SN54HC164*, SN74HC164*, SNJ54HC164*, SNV54HC164 (5962-8416201V*), CD54HC164*, CD74HC164*
  - HCT: CD54HCT164*, CD74HCT164*
  - AC: CD54AC164*, CD74AC164*
  - ACT: CD54ACT164*, CD74ACT164*
  - AHC: SN74AHC164*, SN74AHC164Q*Q1 (-Q1 automotive)
  - AHCT: SN74AHCT164*
  - HCS: SN74HCS164*, SN74HCS164Q*Q1 (Schmitt-trigger inputs on all inputs)
  - LV: SN74LV164A*, SN74LV164AQ*Q1
- **Military numbers:** single-function PDFs → all rows. Per family: JM38510/30605* and M38510/30605* (B2A FK, BCA J, BDA W, SCA J, SDA W) = SN54LS164 (sn74ls164.pdf); 5962-8416201VCA/VDA (SNV54HC164 J/W), 84162012A (SNJ54HC164FK), 8416201CA (SNJ54HC164J) = HC164 (sn74hc164.pdf); CD54HC164F3A mark "8416201CA…" = HC; 5962-8970401CA (mark …CD54HCT164F3A) = HCT164 (cd74hc164.pdf).
- **Pinouts:**
  - p14 (all 14-pin: TTL/LS J,W,N,D,NS; HC D,N,NS,J,W,PW; CD HC/HCT J,D,N; AC/ACT J,D,N,PW; AHC/AHCT PW; HCS D,PW,DYY; LV D,DB,DGV,NS,PW): 1 A input, 2 B input, 3 QA output, 4 QB output, 5 QC output, 6 QD output, 7 GND ground, 8 CLK input, 9 CLR input, 10 QE output, 11 QF output, 12 QG output, 13 QH output, 14 VCC power. (sn74hc164 p4; cd74hc164 p3; cd74ac164 p3; sn74ls164 p1)
  - p14 QFN (BQA: AC/ACT, AHC/AHCT incl. -Q1 WBQA, HCS incl. -Q1, LV incl. -Q1; RGY: LV): same numbering 1–14 as above, plus exposed thermal PAD ("can be connected to GND or left floating", cd74ac164 p3; sn74lv164a p3 Fig 5-2).
  - FK (LCCC-20; SNJ54HC164FK, SNJ54LS164FK): 1 NC, 2 A, 3 B, 4 QA, 5 NC, 6 QB, 7 NC, 8 QC, 9 QD, 10 GND, 11 NC, 12 CLK, 13 CLR, 14 QE, 15 NC, 16 QF, 17 NC, 18 QG, 19 QH, 20 VCC (sn74hc164 p5 pin table; sn74ls164 p1 drawing agrees).
- **Behavior** (function table sn74hc164 p14 Table 1; sn74ls164 p1; cd74ac164 p10):
  - CLR active LOW, **asynchronous**, overrides everything: all QA..QH = L immediately.
  - CLK: rising edge. On ↑ with CLR=H: QA ← A·B; QB ← QA; QC ← QB; … QH ← QG (QH old value discarded). CLK low/high/falling: hold.
  - Priority: CLR (async) > CLK↑ shift. No enable/inhibit pin; either A or B can be used as data-enable (low forces a 0 into stage A).
  - Function table rows (HC p14): `CLR CLK A B | QA QB … QH`: `L X X X | L L L`; `H L X X | QA0 QB0 QH0`; `H ↑ H H | H QAn QGn`; `H ↑ L X | L QAn QGn`; `H ↑ X L | L QAn QGn`.
  - Outputs = register bits directly (no output register, no 3-state). Serial out = QH.
  - Power-up state: not stated.
- **Timing** (CLK→Q, CLR→Q; typical unless noted):
  - TTL 164: CL=15 pF: CLK→Q tPLH 17 / tPHL 21 typ; CL=50 pF: tPLH 20 / tPHL 25 typ; CLR→Q tPHL 24 typ (15 pF), 28 typ (50 pF) (sn74ls164 p4).
  - LS164: CL=15 pF, RL=2 kΩ (only load printed): CLK→Q tPLH 17 / tPHL 21 typ (max 27/32); CLR→Q tPHL 24 typ (max 36) (sn74ls164 p5).
  - HC (SN): 4.5 V, 50 pF: CLK→Q tpd 23 typ (max 35); CLR→Q tPHL 28 typ (max 41) (sn74hc164 p10).
  - HC (CD): 4.5 V 50 pF MAX: CLK→Q 34, CLR→Q 28; typical printed only at 5 V CL=15 pF: CLK→Q 14, CLR→Q 11 (cd74hc164 p7).
  - HCT (CD): 4.5 V 50 pF MAX: CLK→Q 36, CLR→Q 38; typ 5 V 15 pF: CLK→Q 15, CLR→Q 16 (cd74hc164 p7).
  - AC: no typ in table; 5 V (4.5 V worst case) 50 pF MAX −40..85 °C: CP→Qn 11.4, MR(CLR)→Qn 12.6 (cd74ac164 p7); feature list "typical propagation delay 6 ns at VCC=5 V, 25 °C, CL=50 pF" (p1).
  - ACT: 5 V 50 pF MAX −40..85 °C: CLK→Q 13.5, CLR→Q 14.4 (cd74ac164 p7). No typ.
  - AHC: 5 V±0.5 V, 50 pF: CLK→Q 6.4 typ (max 11); CLR→Q tPHL 4.2 typ (max 9.6) (sn74ahc164 p7; -Q1 p7 identical).
  - AHCT: 5 V±0.5 V, 50 pF: CLK→Q 4.9 typ (max 11); CLR→Q 5 typ (max 10.6) (sn74ahct164 p5–6).
  - HCS: 4.5 V, 50 pF: CLK→Q tpd 8 typ (max 12); CLR→Q tPHL 8 typ (max 12) (sn74hcs164 p6; -Q1 p6 same).
  - LV: 3.3 V±0.3 V, 50 pF: CLK→Q 8.3 typ (max 16.3); CLR→Q 7.9 typ (max 16.3) (sn74lv164a p7); at 5 V 50 pF: 6 / 5.8 typ (p8).
- **VCC per family:** TTL/LS SN54 4.5–5.5, SN74 4.75–5.25 (sn74ls164 p4, p5); HC 2–6 (sn74hc164 p6); HCT 4.5–5.5 (cd74hc164 p4); AC 1.5–5.5, ACT 4.5–5.5 (cd74ac164 p4); AHC 2–5.5, inputMax 5.5 (VI 0–5.5, sn74ahc164 p4); AHCT 4.5–5.5, inputMax 5.5 (sn74ahct164 p4); HCS 2–6 (VI 0–VCC, sn74hcs164 p4); LV 2–5.5, inputMax 5.5 (sn74lv164a p5).
- **Obsolete (census):** CD74AC164M, CD74ACT164M, CD74HC164M, CD74HC164MT, CD74HCT164M, SN74HC164D, SN74HC164DT, SN74HC164PW, SN74HC164PWT, SN74LS164D, SN74LV164AD, SN74LV164APW, SN74LV164APWT. (Std TTL SN54164/SN74164 have no census rows at all.)
- **Notes:**
  - cd74hc164 p9 truth table prints QB–QH as "qA – qF" (should be qA–qG; 8 stages) — datasheet error.
  - sn74lv164a p3 Table 5-1 has a spurious extra row "11 QH' O QH inverted" (pin 11 is QF; no QH' pin exists on the drawing) and p11 text says "NAND-gated serial inputs" while the function table is AND (A=B=H → QA=H). Treat as AND, no QH' pin. Same spurious row in sn74lv164a-q1 p3.
  - sn74ahc164 p3 / sn74hcs164 p3 figure captions say "16-Pin" for the 14-pin packages — typo.
  - HCS: Schmitt-trigger inputs, VI max = VCC (not overvoltage tolerant).
  - cd74ac164 timing column header is -40..85 °C / -55..125 °C only; "5 V" values are at VCC=4.5 V (max) per footnote (2).

## 74_264  (SN74HCS264)

- **Title:** "SN74HCS264 8-Bit Parallel-Out Serial Shift Registers With Schmitt-Trigger Inputs and Inverted Outputs" (sn74hcs264 p1).
- **Datasheets:** `shifters/sn74hcs264.pdf` pp1,3,4,6,9,10.
- **Families:** HCS: SN74HCS264* only.
- **Military:** none.
- **Pinouts:** p14 (D, PW): identical pin numbers AND names to 74_164: 1 A in, 2 B in, 3 QA out, 4 QB out, 5 QC out, 6 QD out, 7 GND, 8 CLK in, 9 CLR in, 10 QE out, 11 QF out, 12 QG out, 13 QH out, 14 VCC (p3; output names printed WITHOUT overbars even though inverted).
- **Behavior / exact difference from 164:** identical register (AND-gated A·B into stage 1, CLK rising edge, CLR active-low async clears all 8 internal flip-flops; priority CLR > CLK). The ONLY difference: every output pin is driven through an inverter from its flip-flop (logic diagram p9 Fig 8-1 shows inverter bubbles on all 8 output buffers): QA = NOT(stage1) … QH = NOT(stage8). Function table p10: CLR=L → "All internal storage registers go low; all outputs are high"; ↑ with A=L or B=L → "first stage goes low; the output at QA goes high"; ↑ with A=B=H → "first stage goes high; the output at QA goes low"; "All outputs are inverted from the internally stored data." So after clear all outputs = H; data shifts as inverted bits. Pinout, timing, supply identical to SN74HCS164.
- **Timing:** HCS 4.5 V, 50 pF: CLK→Q tpd 8 typ (max 12), CLR→Q tPHL 8 typ (max 12) (p6). Note: CLR→any Q is labeled tPHL in the table even though outputs go HIGH on clear (copied from HCS164) — datasheet inconsistency; transition is actually L→H.
- **VCC:** 2–6 V, VI 0–VCC (p4).
- **Obsolete:** none.
- **Notes:** p1 Device Information and p4 thermal table say "TSSOP (16)/SOIC (16)" but the package is 14-pin (p3 and census pins=14) — typo.
## 74_165

- **Title / function:** "Parallel-Load 8-Bit Shift Registers" (sn74ls165a p1; SNx4HC165 "8-Bit Parallel-Load Shift Registers" sn74hc165 p1; CDx4HC165 "8-Bit Parallel-In/Serial-Out Shift Register" cd74hc165 p1). PISO with asynchronous parallel load, gated (inhibitable) clock, complementary serial outputs from the 8th stage.
- **Datasheets:** `shifters/sn74ls165a.pdf` (pp1–8; covers SN54/74165 std TTL [marked obsolete] and SN54/74LS165A), `shifters/sn74hc165.pdf` (pp1,3,4,6,7,13), `shifters/sn74hc165-q1.pdf` (pp1,3,4,7), `shifters/cd74hc165.pdf` (pp1,3,4,7,10), `shifters/sn74ahc165.pdf` (pp1,3,4,6,7,11,12), `shifters/sn74hct165.pdf` (pp3,4,6,11), `shifters/sn74hcs165.pdf` (pp3,4,7,8,12,13), `shifters/sn74hcs165-q1.pdf` (pp3,4,6,7,12), `shifters/sn74lv165a.pdf` (pp3,5,7,8,13), `shifters/sn74lv165a-q1.pdf` (pp3,5,8,13), `shifters/sn74lvc165a.pdf` (pp3,4,8,9,16).
- **Families & prefixes:**
  - TTL: SN54165, SN74165 (sn74ls165a p1: "obsolete and no longer supplied"; no census rows)
  - LS: SN54LS165A*, SN74LS165A*, SNJ54LS165A*, SNV54LS165A (5962-7700601V*)
  - HC: SN54HC165*, SN74HC165*, SNJ54HC165*, SN74HC165Q*Q1, CD54HC165*, CD74HC165*
  - HCT: CD54HCT165*, CD74HCT165*, SN74HCT165*
  - AHC: SN74AHC165*
  - HCS: SN74HCS165*, SN74HCS165Q*Q1 (Schmitt inputs)
  - LV: SN74LV165A*, SN74LV165AQ*Q1
  - LVC: SN74LVC165A*
- **Military numbers:** 84095012A (SNJ54HC165FK), 8409501EA (SNJ54HC165J; also mark of CD54HC165F3A), 8409501FA (SNJ54HC165W) = HC165. 5962-8685501EA (mark …CD54HCT165F3A) = HCT165 (cd74hc165.pdf). 5962-7700601VEA/VFA (SNV54LS165A J/W), 7700601EA/FA (SNJ54LS165A J/W), JM38510/30608B2A/BEA/BFA and M38510/30608B2A/BEA/BFA = LS165A (sn74ls165a.pdf).
- **Pinouts:**
  - p16 (TTL/LS J,W,N,D,NS; HC D,DB,N,NS,PW,J,W; CD HC/HCT J,N,D; AHC PW; HCT PW; HCS D,PW,DYY; LV D,DB,DGV,NS,PW; LVC PW,DYY): 1 SH/LD input, 2 CLK input, 3 E input, 4 F input, 5 G input, 6 H input, 7 QHBAR output (complementary serial output, printed Q̄H), 8 GND ground, 9 QH output, 10 SER input, 11 A input, 12 B input, 13 C input, 14 D input, 15 CLK INH input, 16 VCC power (sn74hc165 p3; sn74ls165a p1; cd74hc165 p3).
  - p16 QFN (AHC BQB; HCS BQB/WBQB; LV RGY, BQB, WBQB; LVC BQB): same 1–16 numbering + exposed thermal pad (GND or float) (sn74ahc165 p3; sn74lv165a p3; sn74lvc165a p3).
  - FK (LCCC-20; SNJ54HC165FK, SNJ54LS165AFK): 1 NC, 2 SH/LD, 3 CLK, 4 E, 5 F, 6 NC, 7 G, 8 H, 9 QHBAR, 10 GND, 11 NC, 12 QH, 13 SER, 14 A, 15 B, 16 NC, 17 C, 18 D, 19 CLK INH, 20 VCC (sn74hc165 p3 pin table; LS165A FK drawing p1 agrees).
- **Behavior:**
  - Register: 8 flip-flops QA..QH (internal; only QH and QHBAR are pins). Logic diagram (sn74ls165a p4; sn74hc165 p1): each stage has async S and R driven by gates enabled by SH/LD low: while SH/LD = L, stage X is forced to parallel input X (QA=A … QH=H) — **asynchronous, level-sensitive, transparent** (QH follows input H while SH/LD is low; H→QH is a specified delay path). "While SH/LD is low, the parallel inputs to the register are enabled independently of the levels of the CLK, CLK INH, or serial (SER) inputs" (sn74hc165 p1).
  - Clock: CLK and CLK INH enter a 2-input OR (drawn as OR gate in sn74ls165a p4; text "two-input positive-NOR gate" p1) whose output clocks all flip-flops; shift happens on the **rising edge of (CLK OR CLK INH)** while SH/LD = H. "The functions of CLK and CLK INH are interchangeable. Because a low CLK and a low-to-high transition of CLK INH also accomplish clocking, CLK INH must be changed to the high level only while CLK is high" (sn74hc165 p1). So: CLK INH = H blocks clocking; CLK INH↑ while CLK=L IS a clock edge.
  - Shift: on edge: QA ← SER, QB ← QA, … QH ← QG ("shifts toward serial output QH; data at SER is shifted into the first register", sn74hcs165 p13 note 2).
  - Priority: SH/LD=L (async load) > clocking. Clock edges while SH/LD=L have no effect.
  - Function table (sn74hc165 p13): `SH/LD CLK CLK INH`: `L X X` parallel load; `H H X` no change; `H X H` no change; `H L ↑` shift; `H ↑ L` shift. LS table (sn74ls165a p2) adds `H L L` → QA0 QB0 QH0 (hold).
  - Outputs: QH = stage H; QHBAR = NOT(stage H) (output function table, sn74hcs165 p13 Table 8-2). Push-pull, no 3-state.
  - CD74HC165 recommendation (p10): "CLK and CLK INH should be LOW before the LOW-to-HIGH transition of SH/LD to prevent shifting the data when SH/LD goes HIGH" (timing advice only; logic diagram has no SH/LD gating of the clock).
  - Power-up: "the state of each latch is unknown" (sn74hcs165 p12).
- **Timing** (paths CLK/CLK INH→QH/QHBAR, SH/LD→QH/QHBAR (load→Q), H→QH/QHBAR (data→Q while loading)):
  - TTL 165 (5 V, CL 15 pF, RL 400 Ω, typ): LD→any tPLH 21/tPHL 27; CLK→any 16/21; H→QH 11/24; H→QHBAR 18/18 (sn74ls165a p6).
  - LS165A (5 V, CL 15 pF, RL 2 kΩ, typ): LD→any 21/26; CLK→any 14/16; H→QH 13/24; H→QHBAR 19/17 (sn74ls165a p8). fmax 35 MHz typ.
  - HC (SN, 4.5 V 50 pF typ): SH/LD→Q 20, CLK→Q 15, H→Q 15 (max 30/30/30) (sn74hc165 p6; -Q1 p7 identical).
  - HC (CD): 4.5 V 50 pF MAX: CLK/CLK INH→Q 33, SH/LD→Q 35, H→Q 30; typ only at 5 V/15 pF: 13, 14, 12 (cd74hc165 p7).
  - HCT (CD): 4.5 V 50 pF MAX: CLK→Q 40, SH/LD→Q 40, H→Q 35; typ 5 V/15 pF: 17, 17, 14 (cd74hc165 p7). HCT (SN74HCT165): no typ; 4.5 V MAX 25 °C: SH/LD→Q 40, CLK→Q 40, H→Q 35 (sn74hct165 p6).
  - AHC: table has MIN/MAX only (no TYP; verified on rendered page): 5 V±0.5 V, 50 pF, 25 °C MAX: CLK→Q tPLH 9.4/tPHL 9.2, H→Q 11.1/10.9, SH/LD→Q 11.2/10.9 (sn74ahc165 p7). Features p1 "Low delay, 6 ns (25 °C, 5 V)".
  - HCS: 4.5 V 50 pF, MAX only (25 °C): SH/LD→Q 19, CLK→Q 16, H→Q 15 (sn74hcs165 p7; -Q1 p6–7 same).
  - LV: 3.3 V±0.3 V, 50 pF typ: CLK→Q 10.9, SH/LD→Q 11.3, H→Q 11.1 (sn74lv165a p7); 5 V 50 pF typ 7.7/7.7/7.6 (p8).
  - LVC: 3.3 V±0.3 V, 50 pF typ (−40..125 °C table): CLK or CLK INH→Q 4.2, H→Q 4.6, SH/LD→Q 5.3 (max 14.5/17.8/19.9) (sn74lvc165a p9).
- **VCC per family:** TTL & LS: SN54 4.5–5.5, SN74 4.75–5.25 (sn74ls165a p5, p7); HC 2–6 (sn74hc165 p4); HCT 4.5–5.5 (cd74hc165 p4; sn74hct165 p4); AHC 2–5.5, inputMax 5.5 (sn74ahc165 p4); HCS 2–6, VI ≤ VCC (sn74hcs165 p4); LV 2–5.5, inputMax 5.5 (sn74lv165a p5); LVC 1.1–3.6 ("Operating", p4) — switching specified from 1.2 V — inputMax 5.5 (sn74lvc165a p4).
- **Obsolete (census):** CD74HC165M, CD74HC165MT, CD74HCT165M, SN74HC165PW, SN74HC165PWT, SN74LS165AD, SN74LV165AD, SN74LV165APW. (SN54165/SN74165: no rows; declared obsolete in text.)
- **Notes:**
  - sn74ahc165 p1 Description is copy-pasted from a '595 ("serial-in, parallel-out … storage register … 3-state outputs … SRCLR … OE") and p3 pin table says pin 10 SER = "Shift register clear, active low" — both wrong; title also says "With 3-State Output Registers". Logic diagram, pinout and function tables are the normal '165. Pin 16 VCC.
  - sn74lv165a p3 pin table calls A–H "Serial input", CLK/CLK INH "Storage clock" — misnomers.
  - LS/TTL FK map read from drawing (rotated labels); matches HC FK pin table.
## 74_166

- **Title / function:** "Parallel-Load 8-Bit Shift Registers" (sn74ls166a p1; SNx4HC166 sn74hc166 p1; SN74LV166A p1); CDx4HC(T)166 "8-Bit Parallel-In/Serial-Out Shift Register" (cd74hc166 p1). PISO with **synchronous** parallel load, gated clock, async clear, single serial output QH.
- **Datasheets:** `shifters/sn74ls166a.pdf` (scanned pp1,4,5,6; covers SN54/74166 std TTL and SN54/74LS166A), `shifters/sn74hc166.pdf` (pp1,3,4,7,11), `shifters/cd74hc166.pdf` (pp1,3,7,11), `shifters/sn74lv166a.pdf` (pp3,4,7,10).
- **Families & prefixes:** TTL: SN54166*, SNJ54166*, SN74166 (N; no 74 census rows); LS: SN54LS166A*, SN74LS166A*, SNJ54LS166A*; HC: SN54HC166*, SN74HC166*, SNJ54HC166*, SNV54HC166 (5962-9050101V*), CD54HC166*, CD74HC166*; HCT: CD54HCT166*, CD74HCT166*; LV: SN74LV166A*.
- **Military numbers:** 5962-9050101Q2A (SNJ54HC166FK), 5962-9050101QEA (SNJ54HC166J), 5962-9050101VEA (SNV54HC166J) = HC166. 5962-9558301QEA/QFA (marks SNJ54166J/W) = std TTL 166. 8001701EA/FA (SNJ54LS166AJ/W), JM38510/30609B2A/BEA, M38510/30609B2A/BEA = LS166A. CD54HC166F3A / CD54HCT166F3A: no SMD number.
- **Pinouts:**
  - p16 (TTL/LS J,W,N,D; HC J,D,DB,N,NS,PW; CD HC/HCT J,N,D; LV D,DB,DGV,NS,PW): 1 SER input, 2 A input, 3 B input, 4 C input, 5 D input, 6 CLK INH input, 7 CLK input, 8 GND, 9 CLR input, 10 E input, 11 F input, 12 G input, 13 QH output, 14 H input, 15 SH/LD input, 16 VCC (sn74hc166 p3; sn74ls166a p1; cd74hc166 p3).
  - FK (LCCC-20; SNJ54HC166FK, SNJ54LS166AFK): 1 NC, 2 SER, 3 A, 4 B, 5 C, 6 NC, 7 D, 8 CLK INH, 9 CLK, 10 GND, 11 NC, 12 CLR, 13 E, 14 F, 15 G, 16 NC, 17 QH, 18 H, 19 SH/LD, 20 VCC (sn74hc166 p3 drawing; sn74ls166a p1 drawing agrees).
- **Behavior** (sn74ls166a p1 function table & text, p4 logic diagram; cd74hc166 p11; sn74lv166a p10):
  - CLR: active LOW, **asynchronous**, "overrides all other inputs, including the clock, and sets all flip-flops to zero" → QH = L.
  - Clock: CLK and CLK INH into a 2-input positive-NOR whose output clocks the flip-flops (logic diagram p4); effective clock = rising edge of (CLK OR CLK INH). "Holding either of the clock inputs high inhibits clocking; holding either low enables the other clock input… CLK INH should be changed to the high level only while the clock input is high."
  - SH/LD (**synchronous**): H = shift: on edge QA←SER, QB←QA … QH←QG. L = parallel load "synchronous loading occurs on the next clock pulse" — on edge QA..QH ← A..H; "During parallel loading, serial data flow is inhibited."
  - Priority: CLR (async) > clock gating (CLK INH) > SH/LD mode select.
  - Function table (LS p1): `CLR SH/LD CLKINH CLK SER A..H | QA QB QH`: `L X X X X X | L L L`; `H X L L X X | QA0 QB0 QH0`; `H L L ↑ X a..h | a b h`; `H H L ↑ H X | H QAn QGn`; `H H L ↑ L X | L QAn QGn`; `H X H ↑ X X | QA0 QB0 QH0`.
  - Output: QH = stage H only (no complement, no 3-state).
- **Timing:**
  - TTL 166 (5 V, CL 15 pF, RL 400 Ω, typ): CLR→QH tPHL 23; CLK→QH tPHL 20 / tPLH 17 (sn74ls166a p5).
  - LS166A (5 V, CL 15 pF, RL 2 kΩ, typ): CLR→QH tPHL 19; CLK→QH tPHL 14 / tPLH 11 (p6).
  - HC (SN, 4.5 V 50 pF typ): CLK→QH 15 (max 30), CLR→QH tPHL 18 (max 24) (sn74hc166 p7).
  - HC (CD): 4.5 V 50 pF MAX: CLK→Q 32, MR→Q 32; typ CLK→Q 13 at 5 V/15 pF (cd74hc166 p7). HCT (CD): 4.5 V 50 pF MAX: CLK→Q 40, MR→Q 40; no typ.
  - LV: 3.3 V±0.3 V, 50 pF typ: CLK→QH 8.3, CLR→QH 7.9 (sn74lv166a p7); 5 V 50 pF: 6.1 / 5.7.
- **VCC:** TTL & LS: SN54 4.5–5.5, SN74 4.75–5.25 (sn74ls166a p5, p6); HC 2–6 (sn74hc166 p4); HCT 4.5–5.5 (cd74hc166 p4); LV 2–5.5, inputMax 5.5 (sn74lv166a p4).
- **Obsolete (census):** CD74HC166M, CD74HCT166M, CD74HCT166MT, SN74HC166D, SN74HC166PW, SN74HC166PWT, SN74LS166AD, SN74LV166AD, SN74LV166APW.
- **Notes:**
  - sn74hc166 p11 Operating Mode table omits CLR and is written like the 165 ("Parallel load" for SH/LD=L with CLK X); its footnote clarifies "loaded … synchronously with the clock". Use the LS/CD tables.
  - cd74hc166 p1 functional diagram shows wrong pin numbers (A=11, SH/LD=1, SER=10, CLR=1 …) — copied from another part; the p3 pinout above is correct. cd74hc166 p11 truth table uses Q0..Q7/D0..D7 naming (Q7 = QH) and lists CLOCK ENABLE for CLK INH.
  - sn74lv166a p3 pin table omits pin 6 CLK INH (drawing has it) and calls SER "Serial Output".

## 74_194

- **Title / function:** "4-Bit Bidirectional Universal Shift Registers" (sn74ls194a p1); CDx4HC194 "4-Bit Bidirectional Universal Shift Register" (cd74hc194 p1).
- **Datasheets:** `shifters/sn74ls194a.pdf` (scanned pp1,2,6,7,8; covers SN54/74194 TTL, SN54/74LS194A, SN54/74S194), `shifters/cd74hc194.pdf` (pp1,2,3,6).
- **Families:** TTL: SN54194, SN74194 (no census rows); LS: SN54LS194A*, SN74LS194A*, SNJ54LS194A*; S: SN54S194*, SNJ54S194*, SN74S194 (no 74S rows); HC: CD54HC194*, CD74HC194*; HCT: CD74HCT194*.
- **Military numbers:** 7604001EA/FA (SNJ54S194J/W) = S194; JM38510/30601B2A/BEA/BFA, M38510/30601B2A/BEA/BFA = LS194A; JM38510/07601BEA, M38510/07601BEA = not stated in the PDF — inferred S194 (JM38510 slash 076 = Schottky family; mark gives no type) — verify; 5962-8682601EA (mark …CD54HC194F3A) = HC194.
- **Pinouts — pin names DIFFER between TI-TTL and CD-HC datasheets (same positions):**
  - p16 TTL/LS/S (J,W,N,D): 1 CLR in, 2 SR SER in, 3 A in, 4 B in, 5 C in, 6 D in, 7 SL SER in, 8 GND, 9 S0 in, 10 S1 in, 11 CLK in, 12 QD out, 13 QC out, 14 QB out, 15 QA out, 16 VCC (sn74ls194a p1).
  - p16 CD HC/HCT (F/E/M/NS/PW): 1 MR, 2 DSR, 3 D0, 4 D1, 5 D2, 6 D3, 7 DSL, 8 GND, 9 S0, 10 S1, 11 CP, 12 Q3, 13 Q2, 14 Q1, 15 Q0, 16 VCC (cd74hc194 p1). Mapping: MR=CLR, DSR=SR SER, D0..D3=A..D, DSL=SL SER, CP=CLK, Q0..Q3=QA..QD.
  - FK (SN54LS194A, SN54S194): 1 NC, 2 CLR, 3 SR SER, 4 A, 5 B, 6 NC, 7 C, 8 D, 9 SL SER, 10 GND, 11 NC, 12 S0, 13 S1, 14 CLK, 15 QD, 16 NC, 17 QC, 18 QB, 19 QA, 20 VCC (sn74ls194a p1 drawing).
- **Behavior** (function table sn74ls194a p2; cd74hc194 p2):
  - CLR/MR active LOW, asynchronous, all Q = L, overrides everything.
  - CLK rising edge. Modes by S1,S0 (sampled at the edge, synchronous): `S1 S0`: `L L` inhibit clock / hold; `L H` shift right (toward QD): QA←SR SER, QB←QA, QC←QB, QD←QC; `H L` shift left (toward QA): QD←SL SER, QC←QD, QB←QC, QA←QB; `H H` parallel load QA..QD←A..D ("serial data flow is inhibited").
  - Priority: CLR > (S1,S0 on CLK↑). Function table rows: `CLR S1 S0 CLK SL SR A B C D | QA QB QC QD`: `L X X X X X X X X X | L L L L`; `H X X L X X … | QA0 QB0 QC0 QD0`; `H H H ↑ X X a b c d | a b c d`; `H L H ↑ X H … | H QAn QBn QCn`; `H L H ↑ X L … | L QAn QBn QCn`; `H H L ↑ H X … | QBn QCn QDn H`; `H H L ↑ L X … | QBn QCn QDn L`; `H L L X X X … | QA0 QB0 QC0 QD0`.
  - Serial outputs: QD (right shift) / QA (left shift) — no separate pins. Outputs push-pull.
  - Note (p1): "The mode controls of the SN54194/SN74194 should be changed only while the clock input is high" (std TTL only).
- **Timing:**
  - TTL 194 (5 V, CL 15 pF, RL 400 Ω, typ): CLR→Q tPHL 19; CLK→Q tPLH 14 / tPHL 17 (sn74ls194a p6).
  - LS194A (5 V, 15 pF, RL 2 kΩ, typ): CLR→Q 19; CLK→Q 14 / 17 (p7).
  - S194 (5 V, 15 pF, RL 280 Ω, typ): CLR→Q 12.5; CLK→Q 8 / 11 (p8).
  - HC (CD): 4.5 V 50 pF MAX: CP→Q 35, MR→Q 28; typ CP→Q 14 at 5 V (load not stated, "—") (cd74hc194 p6).
  - HCT (CD): 4.5 V 50 pF MAX: CP→Q 37, MR→Q 40; typ CP→Q 15 at 5 V (p6).
- **VCC:** TTL, LS, S: SN54 4.5–5.5, SN74 4.75–5.25 (sn74ls194a p6–8); HC 2–6, HCT 4.5–5.5 (cd74hc194 p3).
- **Obsolete (census):** CD74HC194M, CD74HC194PW, CD74HC194PWT. (No LS/S rows obsolete.)
- **Notes:** CD-HC naming (MR/DSR/DSL/D0–D3/CP/Q0–Q3) vs TTL naming — pin names differ, functions identical.

## 74_195

- **Title / function:** "High-Speed CMOS Logic 4-Bit Parallel Access Register" (cd74hc195 p1). 4-bit shift-right/parallel-load register with J-K̄ first stage, complementary last-stage output.
- **Datasheets:** `shifters/cd74hc195.pdf` pp1,2,3,4.
- **Families:** HC: CD54HC195*, CD74HC195* only. (p3 operating conditions list "HCT Types 4.5–5.5 V" but no HCT195 is offered.)
- **Military:** none.
- **Pinouts:** p16 (CD54 F; CD74 E, M, NS, PW): 1 MR in (active low), 2 J in, 3 K in (printed K̄, active-low K), 4 D0 in, 5 D1 in, 6 D2 in, 7 D3 in, 8 GND, 9 PE in (printed P̄Ē, active-low parallel enable), 10 CP in, 11 Q3BAR out (printed Q̄3), 12 Q3 out, 13 Q2 out, 14 Q1 out, 15 Q0 out, 16 VCC (p1).
- **Behavior** (truth table p2, description p1):
  - MR active LOW, asynchronous: Q0..Q3 = L, Q3BAR = H, "independent of any other input condition".
  - CP rising edge; everything else synchronous.
  - PE (active low) = mode: PE = L → parallel load: Q0..Q3 ← D0..D3. PE = H → shift right: Q1←Q0, Q2←Q1, Q3←Q2, and Q0 ← J·NOT(Q0) + K·Q0 where K is the K̄ pin level (J=H,K̄=H → set H; J=L,K̄=L → reset L; J=H,K̄=L → toggle; J=L,K̄=H → retain). Tying J and K̄ together gives a D input.
  - Q3BAR = NOT(Q3) always.
  - Priority: MR > PE mode at CP↑.
  - Truth table rows: `MR CP PE J K̄ Dn | Q0 Q1 Q2 Q3 Q̄3`: `L X X X X X | L L L L H`; `H ↑ h h h X | H q0 q1 q2 q̄2`; `H ↑ h l l X | L q0 q1 q2 q̄2`; `H ↑ h h l X | q̄0 q0 q1 q2 q̄2`; `H ↑ h l h X | q0 q0 q1 q2 q̄2`; `H ↑ l X X dn | d0 d1 d2 d3 d̄2` (sic).
  - Shift-left possible externally by tying Qn to Dn−1 with PE low (p1).
- **Timing:** HC: 4.5 V 50 pF MAX: CP→Q 35, MR→Q 30; typ at 5 V CL 15 pF: CP→Qn 14, MR→Qn 13 (p4).
- **VCC:** HC 2–6 (p3).
- **Obsolete (census):** CD74HC195M, CD74HC195PW.
- **Notes:** truth table p2 prints Q̄3 for parallel load as "d̄2" — should be d̄3 (datasheet error). Its legend defines h as "…prior to the High to Low Clock Transition" — typo (L→H).
## 74_299

- **Title / function:** "8-Bit Universal Shift/Storage Registers" (sn74ls299 p1); "…With 3-State Outputs" (sn74als299 p1, sn74f299 p1); "8-Bit Universal Shift Register; Three-State" (cd74hc299 p1); "8-Input Universal Shift/Storage Register with Common Parallel I/O Pins" (cd74ac299 p1, title misprints "8-Lnput"). 8-bit universal (hold / shift right / shift left / sync parallel load) with 8 multiplexed 3-state I/O pins, asynchronous clear, two always-active serial outputs.
- **Datasheets:** `shifters/sn74ls299.pdf` (scanned pp1,2,4,6; covers SN54/74LS299 and SN54/74S299), `shifters/sn74als299.pdf` (pp1,2,4,5), `shifters/sn74f299.pdf` (pp1,2,3,5), `shifters/cd74hc299.pdf` (pp1,2,3,6,7), `shifters/cd74ac299.pdf` (pp1,3,4,7,9,10).
- **Families & prefixes:** LS: SN54LS299*, SN74LS299*, SNJ54LS299*; S: SN54S299, SN74S299 (text only, no census rows); ALS: SN54ALS299*, SN74ALS299*, SNJ54ALS299*; F: SN74F299* (SN54F299 "obsolete and no longer supplied", p1); HC: CD54HC299*, CD74HC299*; HCT: CD54HCT299*, CD74HCT299*; AC: CD54AC299*, CD74AC299*; ACT: CD54ACT299*, CD74ACT299*.
- **Military numbers:** 78024012A (SNJ54LS299FK), 7802401RA (J), 7802401SA (W) = LS299. 83021012A (SNJ54ALS299FK), 8302101RA (J), 8302101SA (W) = ALS299. 5962-8780601RA (mark …CD54HC299F3A) = HC299; 5962-8943601MRA (mark …CD54HCT299F3A) = HCT299. CD54AC299F3A, CD54ACT299F3A: no SMD.
- **Pinouts — one physical pinout, three naming schemes (pin names DIFFER):**
  - p20 TI-TTL LS/S (J, W, DW, N; FK identical numbering): 1 S0 in, 2 G1 in (printed Ḡ1, active-low output control), 3 G2 in (Ḡ2), 4 G/QG inout, 5 E/QE inout, 6 C/QC inout, 7 A/QA inout, 8 QA' out (printed QA′), 9 CLR in, 10 GND, 11 SR in, 12 CLK in, 13 B/QB inout, 14 D/QD inout, 15 F/QF inout, 16 H/QH inout, 17 QH' out, 18 SL in, 19 S1 in, 20 VCC (sn74ls299 p1).
  - p20 ALS/F (J, DW, N, NS; FK identical numbering): same as above but pins 2/3 named OE1/OE2 (sn74als299 p1; sn74f299 p1).
  - p20 CD HC/HCT/AC/ACT (F, E, M/DW): 1 S0, 2 OE1, 3 OE2, 4 I/O6, 5 I/O4, 6 I/O2, 7 I/O0, 8 Q0, 9 MR, 10 GND, 11 DS0, 12 CP, 13 I/O1, 14 I/O3, 15 I/O5, 16 I/O7, 17 Q7, 18 DS7, 19 S1, 20 VCC (cd74hc299 p1; cd74ac299 p3). Mapping: I/O0..I/O7 = A/QA..H/QH; Q0 = QA'; Q7 = QH'; DS0 = SR; DS7 = SL; MR = CLR; CP = CLK.
  - FK (SNJ54LS299FK, SNJ54ALS299FK): for these 20-pin parts the FK pin numbers equal the DIP numbers (drawings sn74ls299 p1, sn74als299 p1: 1 S0, 2 G1/OE1, 3 G2/OE2, 4 G/QG … 20 VCC).
- **Behavior** (function tables sn74als299 p2, sn74f299 p2, sn74ls299 p1; cd74hc299 p2; cd74ac299 p9–10):
  - Register: 8 D flip-flops QA..QH (first stage QA for right shift). CLK rising edge; S0, S1, SR, SL and I/O data are sampled at CLK↑ (synchronous).
  - CLR (MR) active LOW, **asynchronous** on 299: all 8 flip-flops = L immediately "whether the outputs are enabled or off" (LS p1); QA' = QH' = L.
  - Modes (S1,S0): `L L` hold; `L H` shift right: QA←SR, QB←QA … QH←QG; `H L` shift left: QH←SL, QG←QH … QA←QB; `H H` parallel load: QA..QH ← levels on A/QA..H/QH pins at CLK↑.
  - Priority: CLR (async) > mode at CLK↑. Output enable affects only the pins: "Taking either OE1 or OE2 high disables the outputs, but has no effect on clearing, shifting, or storing data" (ALS p1).
  - I/O pin drive equation (logic symbol "&…3EN13", sn74als299 p2; LS logic diagram p2 AND of Ḡ1,Ḡ2 and NAND(S0,S1)): I/O pins drive QA..QH **iff OE1 = L AND OE2 = L AND NOT(S0 = H AND S1 = H)**; otherwise high-Z (and act as inputs). "When both S0 and S1 are HIGH, I/O terminals are in the high-impedance state … regardless of the status of OE1 and OE2" (cd74ac299 p9). Pins are 3-state outputs + always-connected input buffers.
  - Serial outputs QA' = stage QA, QH' = stage QH; push-pull, **never 3-state** (ALS/F tables p2: QA'/QH' columns valid in all rows incl. load with OE=X). In shift right QH' = QGn (new QH); in shift left QA' = QBn (new QA).
  - Full function table (ALS p2; F p2 identical; LS p1 identical with Ḡ1/Ḡ2): `MODE CLR S1 S0 OE1 OE2 CLK SL SR | A/QA … H/QH | QA' QH'`: Clear `L X L L L X X X | L…L | L L`; Clear `L L X L L X X X | L…L | L L`; Clear `L H H X X X X X | X…X (inputs/Z) | L L`; Hold `H L L L L X X X | QA0…QH0 | QA0 QH0`; Hold `H X X L L L X X | QA0…QH0 | QA0 QH0`; Shift right `H L H L L ↑ X H | H QAn…QGn | H QGn`, `… X L | L QAn…QGn | L QGn`; Shift left `H H L L L ↑ H X | QBn…QHn H | QBn H`, `… L X | QBn…QHn L | QBn L`; Load `H H H X X ↑ X X | a…h | a h`. Footnote: "When one or both output-enable inputs are high, the eight I/O terminals are disabled to the high-impedance state; however, sequential operation or clearing of the register is not affected."
- **Timing:**
  - LS299 (5 V, 25 °C typ): CLK→QA'/QH' tPLH 22 / tPHL 26 (RL 2 kΩ, CL 15 pF); CLR→QA'/QH' tPHL 27; CLK→QA..QH 17 / 26 (RL 665 Ω, CL 45 pF); CLR→QA..QH 26; Ḡ→QA..QH tPZH 13 / tPZL 19; disable tPHZ 10 / tPLZ 10 (CL 5 pF) (sn74ls299 p4).
  - S299 (5 V typ): CLK→QA'/QH' 12/13 (CL 15 pF); CLR→QA'/QH' 14; CLK→QA..QH 15/15 (CL 45 pF); CLR→Q 16; Ḡ→Q tPZH 10 / tPZL 12; tPHZ 7 / tPLZ 7 (sn74ls299 p6).
  - ALS299: no typ printed; SN74ALS299 MAX (4.5–5.5 V, CL 50 pF, 0–70 °C): CLK→QA..QH tPLH 13 / tPHL 19; CLK→QA'/QH' 15 / 18; CLR→QA..QH 22; CLR→QA'/QH' 22; OE→Q tPZH 16 / tPZL 22; S0,S1→Q tPZH 17 / tPZL 22; OE→Q tPHZ 8 / tPLZ 15; S0,S1→Q tPHZ 12 / tPLZ 25 (sn74als299 p5). (SN54 MAX: 19/25, 21/25, 29, 29, 22/27, 27/26, 15/38, 16/34.)
  - F299 (5 V, 50 pF, 25 °C typ): CLK→QA'/QH' 6.6 / 6.1; CLK→QA..QH 6.6 / 8.1; CLR→QA'/QH' 7.1; CLR→QA..QH 10.6; OE→Q tPZH 5.6 / tPZL 6.6; tPHZ 4.1 / tPLZ 3.6 (sn74f299 p5). No S0/S1→Q enable/disable path printed.
  - HC (CD): 4.5 V 50 pF MAX: CP→I/O, CP→Q0/Q7 and MR→output all 40; tPZH 31, tPZL 26, tPHZ 37, tPLZ 31. Typ (5 V, 15 pF): propagation 17; tPZL 10, tPZH/tPLZ 13, tPHZ 15 (cd74hc299 p6).
  - HCT (CD): 4.5 V 50 pF MAX: CP→outputs 45; MR→outputs 46; tPZH 32, tPHZ 37, tPLZ 32, tPZL 30. Typ (5 V, 15 pF): 19; enable/disable 10, 13, 15 (p7).
  - AC: no typ in table (p1 feature: "typical propagation delay 6 ns @ 5 V, 25 °C, 50 pF"); 5 V (MAX at 4.5 V) −40..85 °C MAX: CP→Q0,Q7 11.7; CP→I/On 12.3; MR→Q0,Q7 10.2; MR→I/On 12.6; enable/disable 13.5 (cd74ac299 p7).
  - ACT: 5 V MAX: CP→Q0,Q7 11.7; CP→I/On 13.2; MR→Q0,Q7 11.1; MR→I/On 16.9; enable/disable 13.5 (p7).
- **VCC:** LS, S: SN54 4.5–5.5, SN74 4.75–5.25 (sn74ls299 p3, p5 per OCR); ALS: 4.5–5.5 both SN54/SN74 (sn74als299 p4); F: 4.5–5.5 (sn74f299 p3); HC 2–6, HCT 4.5–5.5 (cd74hc299 p3); AC 1.5–5.5, ACT 4.5–5.5 (cd74ac299 p4).
- **Obsolete (census):** CD74HC299M, CD74HCT299M, SN74F299DW. (CD74AC299M96 = Last Time Buy; CD74HC299M96/M96E4 = NRND.)
- **Notes:**
  - cd74ac299 p3 pin table lists I/O0..I/O7 as type "O" (they are I/O) and writes DS0 as "DSO"/"SO" for S0.
  - cd74hc299 p1 text says "'HC259" (typo for 299); p2 truth table shift-right row prints "Q6" for q6.
  - The F/ALS/LS tables show clear rows with S1=S0=H giving I/O "X": during load-mode the I/O pins are inputs (outputs Z) even while clearing.

## 74_323

- **Title / function:** CD74AC323: same document as 74_299 (cd74ac299 pp1,3,7,9,10). 8-bit universal shift/storage register with common parallel I/O; identical to 299 except **synchronous** master reset.
- **Datasheets:** `shifters/cd74ac299.pdf`. (sn74ls299 p1 mentions "SN54LS323 and SN74LS323 are similar but have synchronous clear" — not otherwise covered.)
- **Families:** AC: CD74AC323* only (census: CD74AC323M Active). Text p9 mentions ACT323/CD54AC323 but no such rows.
- **Military:** none.
- **Pinout:** identical to the CD 299 pinout above (1 S0, 2 OE1, 3 OE2, 4 I/O6, 5 I/O4, 6 I/O2, 7 I/O0, 8 Q0, 9 MR, 10 GND, 11 DS0, 12 CP, 13 I/O1, 14 I/O3, 15 I/O5, 16 I/O7, 17 Q7, 18 DS7, 19 S1, 20 VCC) — DW package (cd74ac299 p3).
- **Behavior:** as 74_299 (modes, I/O enable = OE1=L ∧ OE2=L ∧ ¬(S0∧S1), Q0/Q7 always driven) except: "With the CD54/74AC/ACT323, the Master Reset (MR) clears the register in sync with the clock input" (p9) — i.e. MR = L at CP↑ clears all 8 stages (and Q0/Q7) on that edge; MR has priority over S0/S1 mode at that edge (implied by "clears the register"; no separate 323 function table is printed — the p10 table shows MR=L, CP=X for the 299). No MR→output delay applies ("MR to Q0, Q7 (299 only)", p7); MR→(I/O)n row is not marked 299-only (ambiguity).
- **Timing:** AC, 5 V MAX −40..85 °C: CP→Q0,Q7 11.7; CP→I/On 12.3; enable/disable 13.5 (p7). No typ (p1 feature "6 ns typ").
- **VCC:** AC 1.5–5.5 (p4).
- **Obsolete:** none.
- **Notes:** TTL '323 rule (LS p1): only difference from '299 is synchronous clear.
## 4014B

- **Title / function:** "CMOS 8-Stage Static Shift Registers … CD4014B: Synchronous Parallel or Serial Input/Serial Output" (cd4014b p1). 8-stage PISO/SISO, all-synchronous; outputs from stages 6, 7, 8.
- **Datasheets:** `shifters/cd4014b.pdf` (scanned, pp1–4; shared with 4021B).
- **Families:** C: CD4014B* (E, F3A, M/M96/MT, PW/PWR).
- **Military:** none mapped (see 4021B for JM38510/05754BEA).
- **Pinout p16 (E, F3A, D, PW):** 1 PI-8 in, 2 Q6 out, 3 Q8 out, 4 PI-4 in, 5 PI-3 in, 6 PI-2 in, 7 PI-1 in, 8 VSS ground, 9 PARALLEL/SERIAL CONTROL in, 10 CLOCK in, 11 SERIAL IN in, 12 Q7 out, 13 PI-5 in, 14 PI-6 in, 15 PI-7 in, 16 VDD power (terminal diagram p1; logic diagram p2 labels P1..P8, "SERIAL INPUT", "CLOCK", "PARALLEL/SERIAL CONTROL").
- **Behavior** (text p1; truth table & Fig. 1 p2):
  - CLOCK rising edge (positive transition). No reset.
  - PARALLEL/SERIAL CONTROL (P/S): H = parallel: on CLOCK↑ stage n ← PI-n (Q1..Q8 ← PI-1..PI-8) — **synchronous**. L = serial: on CLOCK↑ Q1 ← SERIAL IN, Qn ← Qn−1.
  - Truth table (p2): `CL SER P/S PI-1 PI-n | Q1(internal) Qn`: `↑ X 1 0 0 | 0 0`; `↑ X 1 1 0 | 1 0`; `↑ X 1 0 1 | 0 1`; `↑ X 1 1 1 | 1 1`; `↑ 0 0 X X | 0 Qn−1`; `↑ 1 0 X X | 1 Qn−1`; `↓ X X X X | Q1 Qn (no change)`.
  - Outputs: Q6, Q7, Q8 = stages 6, 7, 8 (buffered); stages 1–5 internal. Push-pull.
- **Timing** (VDD 5 V, CL 50 pF, 25 °C, typ): propagation tPLH/tPHL 160 (max 320); 10 V 80; 15 V 60 (cd4014b p4). Only one delay row (clock→Q); fCL typ 6 MHz at 5 V.
- **VDD:** 3–18 V (p1 recommended operating conditions).
- **Obsolete (census):** CD4014BM, CD4014BMT, CD4014BPW.
- **Notes:** none.

## 4021B

- **Title / function:** "CD4021B: Asynchronous Parallel Input or Synchronous Serial Input/Serial Output" (cd4014b p1).
- **Datasheets:** `shifters/cd4014b.pdf` pp1–4.
- **Families:** C: CD4021B* (E, F, F3A, M/M96, PWR).
- **Military:** JM38510/05754BEA and M38510/05754BEA are in this PDF's census but the PDF does not say whether they are 4014B or 4021B (package addendum p18 lists only "Military: CD4014B-MIL, CD4021B-MIL"). **Unmapped** — needs the MIL-M-38510/057 slash sheet.
- **Pinout:** identical to 4014B (same terminal diagram, p1).
- **Behavior** (text p1; truth table & Fig. 2 p2):
  - P/S = H: **asynchronous** parallel entry: Q1..Q8 ← PI-1..PI-8 while P/S is high, regardless of CLOCK ("data is jammed into the 8-stage register via the parallel input lines and asynchronous with the positive transition of the clock line. In the CD4021B, the CLOCK input of the internal stage is 'forced' when asynchronous parallel entry is made").
  - P/S = L: serial shift on CLOCK↑: Q1 ← SERIAL IN, Qn ← Qn−1.
  - Truth table (p2): `CL SER P/S PI-1 PI-n | Q1 Qn`: `X X 1 0 0 | 0 0`; `X X 1 0 1 | 0 1`; `X X 1 1 0 | 1 0`; `X X 1 1 1 | 1 1`; `↑ 0 0 X X | 0 Qn−1`; `↑ 1 0 X X | 1 Qn−1`; `↓ X 0 X X | Q1 Qn (no change)`.
  - Priority: P/S = H (async load) overrides clock. Recommended: P/S removal time before CLOCK (tREM 140 ns typ at 5 V, p4).
  - Outputs Q6, Q7, Q8 as 4014B.
- **Timing** (5 V, 50 pF typ): propagation 160 (max 320) (p4). No separate P/S→Q delay printed (only P/S pulse width 80 typ and removal time 140 typ).
- **VDD:** 3–18 V.
- **Obsolete (census):** CD4021BM.
- **Notes:** CD4021BF3A NRND.

## 4015B

- **Title / function:** "CMOS Dual 4-Stage Static Shift Register With Serial Input/Parallel Output" (cd4015b p1).
- **Datasheets:** `shifters/cd4015b.pdf` (scanned pp1,2,4).
- **Families:** C: CD4015B* (E, F, F3A, M/M96/MT, PW/PWR).
- **Military:** none.
- **Pinout p16:** 1 CLOCK B in, 2 Q4B out, 3 Q3A out, 4 Q2A out, 5 Q1A out, 6 RESET A in, 7 DATA A in, 8 VSS, 9 CLOCK A in, 10 Q4A out, 11 Q3B out, 12 Q2B out, 13 Q1B out, 14 RESET B in, 15 DATA B in, 16 VDD (terminal diagram p1; functional diagram agrees).
- **Behavior** (text & truth table p1): two independent registers. CLOCK rising edge: Q1 ← DATA, Q2 ← Q1, Q3 ← Q2, Q4 ← Q3. RESET **active HIGH, asynchronous**: all four stages = 0. Truth table `CL D R | Q1 Qn`: `↑ 0 0 | 0 Qn−1`; `↑ 1 0 | 1 Qn−1`; `↓ X 0 | Q1 Qn (no change)`; `X X 1 | 0 0`. Priority RESET > CLOCK.
- **Timing** (5 V, CL 50 pF, 25 °C typ): clock→Q tPHL/tPLH 160 (max 320); reset→Q 200 (max 400) (cd4015b p4).
- **VDD:** 3–18 V (p2).
- **Obsolete:** CD4015BM, CD4015BMT, CD4015BPW.

## 74_4015  (CD74HC4015)

- **Title / function:** "High Speed CMOS Logic Dual 4-Stage Static Shift Register" (cd74hc4015 p1). Same function as 4015B, different pinout and names.
- **Datasheets:** `shifters/cd74hc4015.pdf` pp1,2,3,4.
- **Families:** HC: CD54HC4015*, CD74HC4015* only (no HCT in this datasheet/census).
- **Military:** 5962-8995301EA (mark …CD54HC4015F3A) = HC4015.
- **Pinout p16 (F, E, M):** 1 2CP in, 2 2Q3 out, 3 1Q2 out, 4 1Q1 out, 5 1Q0 out, 6 1MR in, 7 1D in, 8 GND, 9 1CP in, 10 1Q3 out, 11 2Q2 out, 12 2Q1 out, 13 2Q0 out, 14 2MR in, 15 2D in, 16 VCC (p1). (Same positions as CD4015B: Q0..Q3 = Q1..Q4, MR = RESET, CP = CLOCK, D = DATA.)
- **Behavior** (truth table p2): per register, CP↑: Q0 ← D, Q1 ← Q0, Q2 ← Q1, Q3 ← Q2; CP↓ no change; MR (printed R in table) **active HIGH**, asynchronous ("Overriding Reset"; "resetting of all stages is accomplished by a high level", p1): all Q = L.
- **Timing:** HC 4.5 V 50 pF MAX: CP→Qn 35; MR→Qn 55 (clock high) / 65 (clock low); typ at 5 V 15 pF: CP→Qn 14, MR→Qn 25 (p4).
- **VCC:** HC 2–6 V (p3).
- **Obsolete:** none.
- **Notes:** MR→Q delay depends on clock level (two rows).

## 40194B

- **Title / function:** "CMOS 4-Bit Bidirectional Universal Shift Register" (cd40194b p1; stamped "NOT RECOMMENDED FOR NEW DESIGNS" on p1 though census row is Active).
- **Datasheets:** `shifters/cd40194b.pdf` (scanned pp1,2,3,5).
- **Families:** C: CD40194B* (census only CD40194BE).
- **Military:** none.
- **Pinout p16:** 1 RESET in (printed R̄ĒS̄ĒT̄, active low), 2 SHIFT RIGHT in (serial-right input), 3 D0 in, 4 D1 in, 5 D2 in, 6 D3 in, 7 SHIFT LEFT in (serial-left input), 8 VSS, 9 S0 in, 10 S1 in, 11 CLOCK in, 12 Q3 out, 13 Q2 out, 14 Q1 out, 15 Q0 out, 16 VDD (terminal diagram p5; functional diagram p1; logic diagram p3).
- **Behavior** (control truth table p2; text p1): RESET active LOW, asynchronous ("Asynchronous master reset"; "When low, the RESET input resets all stages and forces all outputs low"). CLOCK rising edge. `CLOCK S0 S1 RESET → action`: `X 0 0 1` no change; `↑ 1 0 1` shift right (Q0 toward Q3): Q0←SHIFT RIGHT, Q1←Q0, Q2←Q1, Q3←Q2; `↑ 0 1 1` shift left (Q3 toward Q0): Q3←SHIFT LEFT, Q2←Q3, Q1←Q2, Q0←Q1; `↑ 1 1 1` parallel load Q0..Q3←D0..D3; `X X X 0` reset. Priority RESET > modes. Same function as 74_194 (Q0..Q3 = QA..QD).
- **Timing** (5 V, 50 pF, 25 °C typ): clock→Q 220 (max 440); reset→Q tPRHL 230 (max 460) (p5).
- **VDD:** 3–18 V (p2).
- **Obsolete:** none.
- **Notes:** static table p4 includes a "3-State Output Leakage" row — boilerplate; the device has no 3-state outputs.

## 4035B

- **Title / function:** "CMOS 4-Stage Parallel In/Parallel Out Shift Register with J-K̄ Serial Inputs and True/Complement Outputs" (cd4035b p1).
- **Datasheets:** `shifters/cd4035b.pdf` (scanned pp1,2,4,5).
- **Families:** C: CD4035B* (E, F, F3A, M/M96, PWR).
- **Military:** 8101701EA (mark "8101701EA CD4035BF3A") = 4035B (CD4035BF3A shares the mark).
- **Pinout p16:** 1 Q1/Q1BAR out (printed Q1/Q̄1), 2 TRUE/COMP in (T/C), 3 K in (printed K̄), 4 J in, 5 RESET in, 6 CLOCK in, 7 P/S in, 8 VSS, 9 PI-1 in, 10 PI-2 in, 11 PI-3 in, 12 PI-4 in, 13 Q4/Q4BAR out, 14 Q3/Q3BAR out, 15 Q2/Q2BAR out, 16 VDD (terminal diagram p5; functional diagram p1).
- **Behavior** (text & first-stage truth table p1; logic diagram p2):
  - CLOCK rising edge; all transfers synchronous ("In the parallel or serial mode information is transferred on positive clock transitions").
  - P/S (PARALLEL/SERIAL control): H = parallel entry: stage n ← PI-n on CLOCK↑; L = serial: stage 1 ← J-K̄ function, stage 2 ← stage 1, 3 ← 2, 4 ← 3.
  - First stage (serial mode): Q1⁺ = J·¬Q1 + K̄·Q1 (K̄ = level on pin 3). Table: `CL J K̄ R Qn−1 | Qn`: `↑ 0 X 0 0 | 0`; `↑ 1 X 0 0 | 1`; `↑ X 0 0 1 | 0`; `↑ 1 0 0 Qn−1 | Q̄n−1 (toggle)`; `↑ X 1 0 1 | 1`; `↓ X X 0 Qn−1 | Qn−1`; `X X X 1 X | 0`. J tied to K̄ = D input.
  - RESET: **active HIGH, asynchronous** ("asynchronous common RESET"; table row R=1 → 0 with CL X). Priority RESET > clock.
  - TRUE/COMPLEMENT: H → outputs = register contents; L → outputs = complements; "functions asynchronously with respect to the CLOCK signal" (combinational, all 4 outputs: pin_n = stage_n XOR NOT(T/C), i.e. pin = stage when T/C=1, NOT stage when T/C=0). Logic diagram legend p2: "P/S = 0 serial mode, T/C = 1 true outputs".
- **Timing** (5 V, 50 pF, 25 °C typ): clock→Q 250 (max 500); reset→Q 230 (max 460) (p4). No T/C→Q delay printed.
- **VDD:** 3–18 V (p2).
- **Obsolete:** CD4035BM.
