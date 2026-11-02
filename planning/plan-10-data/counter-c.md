# cntC — CMOS (CD4000 + CD74HC/HCT) synchronous counters

Source PDFs are under `datasheets/` (repo-relative paths below).
**The eight CD4000 PDFs are scanned images. `pdftotext` returns only the packaging addendum.**
Everything for them was read from 300-dpi renders: OCR (tesseract), with each pinout, truth table and timing diagram then checked by eye on the image.
Page numbers are given as "PDF page (printed page)". The two CD74HC PDFs have real text.

## PDF → devices

| PDF | Devices |
|---|---|
| counters/cd4029b.pdf | 4029B |
| counters/cd4510b.pdf | 4510B, 4516B |
| counters/cd4518b.pdf | 4518B, 4520B |
| counters/cd74hc4518.pdf | 74_4518 (HC only), 74_4520 (HC, HCT) |
| counters/cd40102b.pdf | 40102B, 40103B |
| counters/cd74hc40103.pdf | 74_40103 (HC, HCT) |
| counters/cd40160b.pdf | 40160B, 40161B (the PDF also describes 40162B/40163B, but neither has census rows, so they are not orderable — see Notes) |
| counters/cd40192b.pdf | 40192B, 40193B |
| counters/cd4026b.pdf | 4026B, 4033B |
| counters/cd40110b.pdf | 40110B |

**Census rows that could not be mapped:** none. Military/SMD rows:
- `8101602EA` → 4029B (marking `8101602EACD4029BF3A`).
- `7702301EA` → 4520B (marking `7702301EACD4520BF3A`; NRND).
- `5962-9055301EA` → 74_40103 HC (CD54HC40103F3A).
- `5962-8995401EA` → 74_4520 HC (CD54HC4520F3A).

**Common to all CD4000 parts here:**
- Recommended VDD is 3–18 V. Absolute maximum is −0.5 to +20 V. Inputs may go from −0.5 V to VDD + 0.5 V, so there is no inputMax above VDD.
- Dynamic tables are at TA = 25 °C, CL = 50 pF, RL = 200 kΩ, tr = tf = 20 ns.
- Each timing line below gives typ (max) at VDD = 5 V. A note under each device gives the 10 V and 15 V typicals for the clock→Q path.
- Every package is 16-pin. Across all 16-pin codes (N, NS, PW, J, D) there is one pin arrangement per device, and no device has a thermal pad.

---

## 4029B
- **Title:** "CMOS Presettable Up/Down Counter, Binary or BCD-Decade" (CD4029B).
- **Datasheet:** counters/cd4029b.pdf, pp1–5 (3-84 … 3-88).
  - p1: text, recommended conditions, terminal diagram.
  - p3: logic diagram, Fig. 10 binary timing.
  - p4: control-logic table, Fig. 12 decade timing.
  - p5: dynamic characteristics, Fig. 17 cascading.
- **Families:** C: CD4029B* (BE, BF, BF3A, BM, BM96, BNSR, BPWR).
- **Military:** 8101602EA (= CD4029BF3A).
- **Pinout p16 (N, J, D, NS, PW):**
  - 1 PRESET ENABLE input
  - 2 Q4 output
  - 3 JAM4 input
  - 4 JAM1 input
  - 5 CARRYIN input (printed CARRY IN with overbar; also called "CLOCK ENABLE")
  - 6 Q1 output
  - 7 CARRYOUT output (overbar)
  - 8 VSS ground
  - 9 BINARY/DECADE input
  - 10 UP/DOWN input
  - 11 Q2 output
  - 12 JAM2 input
  - 13 JAM3 input
  - 14 Q3 output
  - 15 CLOCK input
  - 16 VDD power
- **Behavior** (p1 text; p4 control table):
  - **PRESET ENABLE** is active-high and **asynchronous**. While it is high, Q1..Q4 = JAM1..JAM4: "A high PRESET ENABLE signal allows information on the JAM INPUTS to preset the counter to any state asynchronously with the clock." A low on each JAM line resets that bit. It is level-sensitive and transparent while high, and it inhibits counting.
  - **CARRY IN** is active-low. "The counter is advanced one count at the positive transition of the clock when the CARRY-IN and PRESET ENABLE signals are low. Advancement is inhibited when the CARRY-IN or PRESET ENABLE signals are high."
  - CARRY IN is a synchronous enable. Its setup to the clock is 200 ns min at 5 V (p1), and it is sampled at the CLOCK rising edge. The datasheet says "the CARRY-IN terminal must be connected to VSS when not in use."
  - **Priority:** PRESET ENABLE (async load) > CARRY IN high (hold) > count on CLOCK ↑.
  - **BINARY/DECADE:** 1 = binary (mod 16), 0 = decade (mod 10, BCD).
  - **UP/DOWN:** 1 = count up, 0 = count down.
  - U/D and B/D are synchronous controls with setup to the clock (340 ns min at 5 V, p1). Ripple mode: "the clock input to the first counting stage must be high" when U/D changes (Fig. 17, p5).
  - **CARRYOUT** is active-low and combinational: "normally high and goes low when the counter reaches its maximum count in the UP mode or the minimum count in the DOWN mode provided the CARRY-IN signal is low."
    - CARRYOUT = NOT( NOT CARRYIN · ( UP·(count = 15 binary / 9 decade) + DOWN·(count = 0) ) ).
    - It is not gated by the clock. It stays low for the whole terminal state (Figs. 10 and 12).
    - The datasheet's Carry-In→Carry-Out delay (170 ns) confirms the combinational path.
  - **Out-of-range decade states (10–15):** no state diagram is printed. Only the logic diagram is given (p3–p4, not traced).
  - **Cascading** (Fig. 17): parallel clocking ties CO to the next stage's CI; the note says CO may glitch. Ripple clocking ties CO to the next stage's CLOCK, OR-ed with the clock.
  - Power-up state is not stated.
- **Timing at VDD = 5 V, typ (max), ns** (p5):
  - CLOCK→Q 250 (500)
  - CLOCK→CARRYOUT 280 (560)
  - PRESET ENABLE→Q 235 (470)
  - PRESET ENABLE→CARRYOUT 320 (640)
  - CARRY IN→CARRYOUT 170 (340)
  - Transition time 100 (200)
  - fmax 4 MHz typ (2 min)
- **10/15 V typ, clock→Q:** 120 ns / 90 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** CD4029BM.
- **Notes:** this is the only part in the group whose preset is a transparent async level ("jam") input.

## 4510B / 4516B
- **Title:** "CMOS Presettable Up/Down Counters": 4510B = BCD type, 4516B = binary type.
- **Datasheet:** counters/cd4510b.pdf, pp1–6 (3-249 … 3-254).
  - p1: text, recommended conditions, terminal assignment.
  - p2: logic diagram (4510B).
  - p3: logic diagram (4516B).
  - p4: dynamic characteristics.
  - p5: truth table, timing diagrams.
  - p6: cascading.
- **Families:** C: CD4510B*, CD4516B*.
- **Military:** none.
- **Pinout p16 (N, NS, PW, J)** — the same for both chips:
  - 1 PRESET ENABLE input
  - 2 Q4 output
  - 3 P4 input
  - 4 P1 input
  - 5 CARRYIN input (overbar)
  - 6 Q1 output
  - 7 CARRYOUT output (overbar)
  - 8 VSS ground
  - 9 RESET input
  - 10 UP/DOWN input
  - 11 Q2 output
  - 12 P2 input
  - 13 P3 input
  - 14 Q3 output
  - 15 CLOCK input
  - 16 VDD power
- **Truth table (p5)**, columns CL, CI(bar), U/D, PE, R:

  | CL | CI | U/D | PE | R | Action |
  |---|---|---|---|---|---|
  | X | 1 | X | 0 | 0 | NO COUNT |
  | ↑ | 0 | 1 | 0 | 0 | COUNT UP |
  | ↑ | 0 | 0 | 0 | 0 | COUNT DOWN |
  | X | X | X | 1 | 0 | PRESET |
  | X | X | X | X | 1 | RESET |

- **Behavior:**
  - RESET is active-high and asynchronous: "cleared by a high level on the RESET line".
  - PRESET ENABLE is active-high and asynchronous: "preset to any binary number present on the jam inputs by a high level on the PRESET ENABLE line". It is level-transparent. There is a PE→Q delay, a PE→Jn setup/hold, and a removal time ("time required after the falling edge of reset or preset enable before the rising edge of clock will trigger the counter").
  - **Priority:** RESET > PRESET ENABLE > CARRY IN high (hold) > count on CLOCK ↑.
  - CARRY IN low enables counting. U/D 1 = up, 0 = down.
  - Modulus: 4510B counts 0–9 (BCD), 4516B counts 0–15.
  - **4510B out-of-range states:** "will count out of non-BCD counter states in a maximum of two clock pulses in the up mode, and a maximum of four clock pulses in the down mode." Exact next states are not printed.
  - **CARRYOUT** is active-low and combinational: CARRYOUT = NOT( NOT CARRYIN · ( UP·(Q = 9 or 15) + DOWN·(Q = 0) ) ).
    - Fig. 15 shows CO low while the count is 9 in up mode. It returns high the moment U/D switches to down, mid-state. In down mode it is low at count 0 only after CI goes low.
    - It is not clock-gated. The text says: "If the UP/DOWN input changes during a terminal count, the CARRY-OUT must be gated with the clock".
  - **Cascading:** synchronous mode ties all clocks together and CO to the next CI. Ripple mode feeds CO to the next CLOCK (Fig. 18).
- **Timing at VDD = 5 V, typ (max), ns** (p4):
  - Clock→Q 200 (400)
  - Preset or Reset→Q 210 (420)
  - Clock→CO 240 (480)
  - CI→CO 125 (250)
  - Preset or Reset→CO 320 (640)
  - Transition time 100 (200)
  - fmax 4 MHz typ (2 min)
- **10/15 V typ, clock→Q:** 100 ns / 75 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** 4510B none; 4516B CD4516BPW.
- **Notes:** the datasheet says these are similar to the MC14510/MC14516. The 4510B's CO in the down direction is at count 0.

## 4518B / 4520B
- **Title:** "CMOS Dual Up-Counters": 4518B = dual BCD, 4520B = dual binary.
- **Datasheet:** counters/cd4518b.pdf, pp1–5 (3-272 … 3-276).
  - p1: text, truth table, terminal assignment.
  - p3: recommended conditions, dynamic characteristics.
  - p4: logic diagrams (Figs. 14/15), timing (Fig. 16).
- **Families:** C: CD4518B*, CD4520B*.
- **Military:** 7702301EA → 4520B.
- **Pinout p16 (N, J, D, NS, PW)** — the same for both chips:
  - 1 CLOCK A input
  - 2 ENABLE A input
  - 3 Q1A output
  - 4 Q2A output
  - 5 Q3A output
  - 6 Q4A output
  - 7 RESET A input
  - 8 VSS ground
  - 9 CLOCK B input
  - 10 ENABLE B input
  - 11 Q1B output
  - 12 Q2B output
  - 13 Q3B output
  - 14 Q4B output
  - 15 RESET B input
  - 16 VDD power
- **Truth table (p1, read from the image):**

  | CLOCK | ENABLE | RESET | Action |
  |---|---|---|---|
  | ↑ | 1 | 0 | Increment Counter |
  | 0 | ↓ | 0 | Increment Counter |
  | ↓ | X | 0 | No Change |
  | X | ↑ | 0 | No Change |
  | ↑ | 0 | 0 | No Change |
  | 1 | ↓ | 0 | No Change |
  | X | X | 1 | Q1 thru Q4 = 0 |

- **Exact rule:**
  - The counter increments on a rising edge of CLOCK while ENABLE = 1, or on a falling edge of ENABLE while CLOCK = 0.
  - Equivalently: internal clock = CLOCK OR (NOT ENABLE), and the counter advances on its rising edge. This is my derivation, but it reproduces all six rows of the table.
  - All other edges do nothing.
- **Behavior:**
  - RESET is active-high and asynchronous (it clears Q1..Q4 and has a Reset→Q delay). It dominates.
  - The two counters are fully independent.
  - Modulus: 4518B counts 0–9 BCD; 4520B counts 0–15.
  - There are no carry outputs. Ripple cascading uses Q4 → next ENABLE with the next CLOCK held low, so the stage advances on Q4's falling edge.
  - **4518B states 10–15:** no state diagram is printed (logic diagram only, Fig. 14).
- **Timing at VDD = 5 V, typ (max), ns** (p3):
  - Clock or Enable→Q 280 (560)
  - Reset→Q 330 (650)
  - Transition time 100 (200)
  - fmax 3 MHz typ (1.5 min)
- **10/15 V typ, clock→Q:** 115 ns / 80 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:**
  - 4518B: CD4518BM, CD4518BPW.
  - 4520B: CD4520BM, CD4520BPW.
  - Also NRND: 7702301EA / CD4520BF3A.

## 74_4518 / 74_4520 (CD74HC4518, CD54/74HC4520, CD74HCT4520)
- **Title:** "High-Speed CMOS Logic Dual Synchronous Counters".
  - HC4518 = dual BCD up-counter.
  - HC4520 / HCT4520 = dual binary up-counter.
- **Datasheet:** counters/cd74hc4518.pdf, pp1–6.
  - p1: description, pinout.
  - p2: truth table.
  - p3: operating conditions.
  - p4–6: switching.
  - p6: timing diagram.
- **Families:**
  - HC: CD74HC4518E (BCD); CD74HC4520*, CD54HC4520F*.
  - HCT: CD74HCT4520* (binary only; there is no HCT4518).
- **Military:** 5962-8995401EA → HC4520.
- **Pinout p16 (N, D, J)** — the same for both:
  - 1 1CP input
  - 2 1E input
  - 3 1Q0 output
  - 4 1Q1 output
  - 5 1Q2 output
  - 6 1Q3 output
  - 7 1MR input
  - 8 GND ground
  - 9 2CP input
  - 10 2E input
  - 11 2Q0 output
  - 12 2Q1 output
  - 13 2Q2 output
  - 14 2Q3 output
  - 15 2MR input
  - 16 VCC power
- **Behavior:** the same as 4518B/4520B. The truth table on p2 has the same rows: CP↑·E=H → increment; CP=L·E↓ → increment; MR = H → Q0..Q3 = L (asynchronous, active-high "MASTER RESET").
  - **Datasheet error:** the legend says "↑ = High-to-Low Transition, ↓ = Low-to-High Transition". This is inverted relative to the text, which says "incrementing on ... positive-going ... transition of CLOCK", and relative to the CD4518B table. Use ↑ = rising.
  - Ripple cascade: Q3 → next E, with the next CP held low.
  - Pin names differ from the CD4000 version (CP/E/MR/Q0–Q3 vs CLOCK/ENABLE/RESET/Q1–Q4), so this is kept as a separate device.
- **Timing** (p5–6). The 4.5 V / 50 pF condition prints only a max, so those are maxes; typicals are printed only at 5 V, 15 pF.
  - HC:
    - CP→Qn max 48 ns (typ 20 at 5 V, 15 pF)
    - E→Qn max 48 (typ 20)
    - MR→Qn max 30 (typ 12)
    - fmax 30 MHz min at 4.5 V
  - HCT:
    - CP→Qn max 53 (typ 22)
    - E→Qn max 55 (typ 23)
    - MR→Qn max 35 (typ 14)
- **VCC:** HC 2–6 V; HCT 4.5–5.5 V. Inputs are 0..VCC, with no inputMax above VCC.
- **Obsolete:**
  - 74_4518: none.
  - 74_4520: CD74HC4520M, CD74HC4520MT, CD74HCT4520M.

## 40102B / 40103B
- **Title:** "CMOS 8-Stage Presettable Synchronous Down Counters": 40102B = 2-decade BCD type; 40103B = 8-bit binary type.
- **Datasheet:** counters/cd40102b.pdf, pp1–7 (3-376 … 3-382).
  - p1: text, recommended conditions.
  - p3: dynamic characteristics.
  - p4–5: logic diagrams, truth table.
  - p6: Fig. 15 timing, terminal assignment.
  - p7: applications, cascading.
- **Families:** C: CD40102B*, CD40103B*.
- **Military:** none.
- **Pinout p16 (N, NS, PW, J)** — the same for both. Overbars are on CLEAR, CI/CE, APE, SPE and CO/ZD; the abbreviations are the datasheet's own.
  - 1 CLOCK input
  - 2 CLEAR (CLR) input
  - 3 CARRY IN/COUNTER ENABLE (CI/CE) input
  - 4 J0 input
  - 5 J1 input
  - 6 J2 input
  - 7 J3 input
  - 8 VSS ground
  - 9 ASYNCHRONOUS PRESET ENABLE (APE) input
  - 10 J4 input
  - 11 J5 input
  - 12 J6 input
  - 13 J7 input
  - 14 CARRY OUT/ZERO DETECT (CO/ZD) output
  - 15 SYNCHRONOUS PRESET ENABLE (SPE) input
  - 16 VDD power
- **Truth table (p5):**

  | CLR | APE | SPE | CI/CE | Mode | Action |
  |---|---|---|---|---|---|
  | 1 | 1 | 1 | 1 | Synchronous | Inhibit counter |
  | 1 | 1 | 1 | 0 | Synchronous | Count down |
  | 1 | 1 | 0 | X | Synchronous | Preset on next positive clock transition |
  | 1 | 0 | X | X | Asynchronous | Preset asynchronously |
  | 0 | X | X | X | Asynchronous | Clear to maximum count |

  Notes printed with the table:
  - "Synchronous operation: changes occur on negative-to-positive clock transitions."
  - JAM: 40102B is BCD with MSD = J7..J4 (J7 MSB) and LSD = J3..J0 (J3 MSB). 40103B is binary with MSB = J7 and LSB = J0.
  - "*At zero count, the counters will jump to the maximum count on the next clock transition to 'High'."
- **Behavior:**
  - Every control and CO/ZD is active-low.
  - **CLR low** asynchronously sets the count to its maximum (99 decimal for 40102B, 255 for 40103B), "regardless of the state of any other input".
  - **APE low** asynchronously forces the count to J, regardless of SPE, CI/CE and CLOCK. It is a level, so it is transparent while low.
  - **SPE low** loads J on the next CLOCK ↑, regardless of CI/CE.
  - **CI/CE low** enables counting down on CLOCK ↑. High inhibits counting, but not the synchronous load.
  - **Priority:** CLR > APE > SPE > CI/CE.
  - From 0 the counter wraps to 99/255 on the next clock edge. With CO/ZD tied to SPE (Fig. 18), the counter divides by N+1.
  - **CO/ZD** = NOT(count = 0 · NOT CI/CE). Text: "goes low when the count reaches zero if the CI/CE input is low, and remains low for one full clock period." It is combinational. Fig. 15 shows it low exactly during the 0 state, from the clock edge that enters 0 to the next edge. CI/CE→CO/ZD delay is listed.
  - **Fig. 15 sequence** (40102B / 40103B):
    1. 99/255 after CLR is released, then 98/254.
    2. SPE pulses low → loads 3, then 2, 1, 0 (CO/ZD low), then 99/255, 98/254.
    3. CI/CE pulses high → 98/254 is held for one clock.
    4. 97/253, then an APE pulse loads 8 asynchronously in mid-period.
    5. 7, 6, 5, 4, then a CLR pulse → 99/255, 98/254, …
  - **40102B out-of-range BCD digits:** not described.
  - p7 cascading note: "An output spike (160 ns @ VDD = 5 V) occurs whenever two or more devices are cascaded in the parallel-clocked mode because the clock-to-carry out delay is greater than the carry-in-to-carry out delay." It can be eliminated by gating the last device's output with the clock.
- **Timing at VDD = 5 V, typ (max), ns** (p3):
  - Clock→CO/ZD 300 (600). Note 1 says this "also applies to Synchronous Preset Mode should a preset condition of JAM zero exist".
  - CI/CE→CO/ZD 200 (400)
  - APE→CO/ZD 650 (1300)
  - CLR→CO/ZD 375 (750)
  - Transition time 100 (200)
  - fmax 1.4 MHz typ (0.7 min)
- **10/15 V typ, clock→output:** 130 ns / 95 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** 40102B CD40102BPW (CD40102BPWR is Last-time-buy); 40103B none.
- **Notes:** the only output is CO/ZD. The count bits are internal, so a model needs internal state only.

## 74_40103 (CD54/74HC40103, CD74HCT40103)
- **Title:** "High-Speed CMOS Logic 8-Stage Synchronous Down Counters" (binary only).
- **Datasheet:** counters/cd74hc40103.pdf, pp1–7.
  - p1: description.
  - p2: pinout, truth table.
  - p3: operating conditions.
  - p5–7: prerequisites and switching.
  - p7: Fig. 1 timing.
- **Families:** HC: CD74HC40103*, CD54HC40103F*. HCT: CD74HCT40103*.
- **Military:** 5962-9055301EA → HC.
- **Pinout p16 (N, D, J).** Overbars are on MR, TE, PE, PL and TC.
  - 1 CP input
  - 2 MR input
  - 3 TE input
  - 4 P0 input
  - 5 P1 input
  - 6 P2 input
  - 7 P3 input
  - 8 GND ground
  - 9 PL input (printed "PL (ASYNC)")
  - 10 P4 input
  - 11 P5 input
  - 12 P6 input
  - 13 P7 input
  - 14 TC output
  - 15 PE input (printed "PE (SYNC)")
  - 16 VCC power
- **Behavior:** identical to 40103B with these renames: CLR → MR, APE → PL, SPE → PE, CI/CE → TE, CO/ZD → TC, J → P.
  - The truth table (p2) is the same five rows. MR low clears to the maximum count, 255.
  - Text (p1): "TC output goes low when the count reaches zero if the TE input is low, and remains low for one full clock period."
  - Fig. 1 (p7) is the same sequence as the CD40103B, with TC low only during count 0.
  - It wraps 0 → 255 (a 256-pulse cycle).
- **Timing** (p6–7). At 4.5 V / 50 pF only a max is printed, so those are maxes; the bracketed typicals are at 5 V, 15 pF.
  - HC:
    - CP→TC 60 [25] (printed as "CP to any TC (Async Preset)" and "CP to TC (Sync Preset)")
    - TE→TC 40 [17]
    - PL→TC 55 [23]
    - MR→TC 55 [23]
    - fCP min 15 MHz at 4.5 V
  - HCT:
    - CP→TC 60 [25]
    - "CE to TC (Sync Preset)" 63 [26]; this is a label error and almost certainly means CP→TC with sync preset.
    - TE→TC 50 [21]
    - PL→TC 68 [28]
    - MR→TC 55 [23]
- **VCC:** HC 2–6 V; HCT 4.5–5.5 V.
- **Obsolete:** CD74HC40103M, CD74HC40103MT, CD74HCT40103M.
- **Notes:** I kept this separate from 40103B because the pin names differ; the function is identical. The HC sheet has no BCD (40102) version.

## 40160B / 40161B
- **Title:** "CMOS Synchronous Programmable 4-Bit Counters": 40160B = decade with asynchronous clear; 40161B = binary with asynchronous clear.
- **Datasheet:** counters/cd40160b.pdf, pp1–7 (3-415 … 3-421).
  - p1: text.
  - p2: logic diagrams.
  - p3: recommended conditions, truth table.
  - p4: terminal assignment.
  - p5: dynamic characteristics.
  - p6: timing diagrams.
  - p7: cascading.
- **Families:** C: CD40160B* (only CD40160BF3A is orderable), CD40161B*.
- **Military:** none.
- **Pinout p16 (J, N, NS, PW).** Overbars are on CLEAR and LOAD.
  - 1 CLEAR input
  - 2 CLOCK input
  - 3 P1 input
  - 4 P2 input
  - 5 P3 input
  - 6 P4 input
  - 7 PE input
  - 8 VSS ground
  - 9 LOAD input
  - 10 TE input
  - 11 Q4 output
  - 12 Q3 output
  - 13 Q2 output
  - 14 Q1 output
  - 15 CARRY OUT output (active-high, printed "COUT" in the logic diagram)
  - 16 VDD power
- **Truth table (p3):**

  | CLOCK | CLR | LOAD | PE | TE | Operation |
  |---|---|---|---|---|---|
  | ↑ | 1 | 0 | X | X | PRESET |
  | ↑ | 1 | 1 | 0 | X | NC |
  | ↑ | 1 | 1 | X | 0 | NC |
  | ↑ | 1 | 1 | 1 | 1 | COUNT |
  | X | 0 | X | X | X | RESET (40160B/40161B) |

  The 40162B/40163B rows (synchronous clear) do not apply here.
- **Behavior:** functionally equivalent to and pin-compatible with the 74LS160/161 (p1).
  - CLEAR low asynchronously sets Q1..Q4 to 0, "regardless of the state of the CLOCK, LOAD, or ENABLE inputs".
  - LOAD low loads P1..P4 at the next CLOCK ↑, regardless of PE/TE.
  - It counts when PE = TE = 1.
  - **Priority:** CLEAR > LOAD > (PE·TE) count.
  - "Logic transitions at the PE or TE inputs may occur when the clock is either high or low."
  - **CARRY OUT** is active-high: "TE is fed forward to enable COUT ... produces a positive output pulse with a duration approximately equal to the positive portion of the Q1 output."
    - Decade (40160B): the logic diagram (Fig. 3, p2) shows COUT = TE·Q1·Q4.
    - Binary (40161B): COUT = TE·Q1·Q2·Q3·Q4. This is from the 74LS161 equivalence; I did not trace Fig. 4.
  - **40160B states 10–15:** no state diagram is printed.
- **Timing at VDD = 5 V, typ (max), ns** (p5):
  - Clock→Q 200 (400)
  - Clock→COUT 225 (450)
  - TE→COUT 125 (250)
  - Clear→Q (tPHL) 250 (500)
  - fmax 3 MHz typ (2 min)
- **10/15 V typ, clock→Q:** 80 ns / 60 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** none. CD40161BE is Last-time-buy.
- **Notes:** CD40162B/40163B appear in the PDF, but a header note says they "were not acquired from Harris" and the census has no rows for them.

## 40192B / 40193B
- **Title:** "CMOS Presettable Up/Down Counters (Dual Clock With Reset)": 40192B = BCD type, 40193B = binary type.
- **Datasheet:** counters/cd40192b.pdf, pp1–5 (3-432 … 3-436).
  - p1: text, terminal assignment.
  - p2: timing, truth table.
  - p3: recommended conditions.
  - p5: dynamic characteristics.
- **Families:** C: CD40192B*, CD40193B*.
- **Military:** none.
- **Pinout p16 (N, J, NS, PW).** Overbars are on BORROW, CARRY and PRESET ENABLE.
  - 1 J2 input
  - 2 Q2 output
  - 3 Q1 output
  - 4 CLOCK DOWN input
  - 5 CLOCK UP input
  - 6 Q3 output
  - 7 Q4 output
  - 8 VSS ground
  - 9 J4 input
  - 10 J3 input
  - 11 PRESET ENABLE input
  - 12 CARRY output
  - 13 BORROW output
  - 14 RESET input
  - 15 J1 input
  - 16 VDD power
- **Truth table (p2):**

  | CLOCK UP | CLOCK DOWN | PRESET ENABLE | RESET | Action |
  |---|---|---|---|---|
  | ↑ | 1 | 1 | 0 | COUNT UP |
  | ↓ | 1 | 1 | 0 | NO COUNT |
  | 1 | ↑ | 1 | 0 | COUNT DOWN |
  | 1 | ↓ | 1 | 0 | NO COUNT |
  | X | X | 0 | 0 | PRESET |
  | X | X | X | 1 | RESET |

- **Behavior:**
  - RESET is active-high and asynchronous; all outputs go low.
  - PRESET ENABLE is active-low and asynchronous: "each output is individually programmable asynchronously with the clock to the level on the corresponding jam input when the PRESET ENABLE control is low". It is level-transparent.
  - **Priority:** RESET > PRESET ENABLE > clocks.
  - The counter counts up on CLOCK UP ↑ while CLOCK DOWN is high, and down on CLOCK DOWN ↑ while CLOCK UP is high.
  - Modulus: 40192B 0–9; 40193B 0–15.
  - **CARRY** = NOT( count = max(9/15) · CLOCK UP = 0 ). Text: "goes low one-half clock cycle after the counter reaches its maximum count in the count-up mode", i.e. while CLOCK UP is low in the max state.
  - **BORROW** = NOT( count = 0 · CLOCK DOWN = 0 ).
  - Cascading: BORROW → next CLOCK DOWN, CARRY → next CLOCK UP.
  - This is the same behavior as the 74192/74193.
  - **40192B states 10–15:** not printed.
- **Timing at VDD = 5 V, typ (max), ns** (p5):
  - CLOCK UP/DOWN→Q and RESET→Q 250 (500)
  - PE→Q 200 (400)
  - CLOCK UP→CARRY and CLOCK DOWN→BORROW 160 (320)
  - RESET or PE→BORROW/CARRY 300 (600)
  - Transition time 100 (200)
  - fmax 4 MHz typ (2 min)
- **10/15 V typ, clock→Q:** 120 ns / 90 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** 40192B none; 40193B CD40193BPW.

## 4026B / 4033B
- **Title:** "CMOS Decade Counters/Dividers with Decoded 7-Segment Display Outputs": 4026B adds Display Enable; 4033B adds Ripple Blanking.
- **Datasheet:** counters/cd4026b.pdf, pp1–4 (3-70 … 3-73).
  - p1: text, terminal diagrams.
  - p2: text, recommended conditions.
  - p3: logic diagrams, Figs. 3/4 timing.
  - p4: dynamic characteristics.
- **Families:** C: CD4026B*, CD4033B*.
- **Military:** none.
- **Pinout p16, 4026B (N, NS, PW).** No overbars on any pin.
  - 1 CLOCK input
  - 2 CLOCK INHIBIT input
  - 3 DISPLAY ENABLE IN input
  - 4 DISPLAY ENABLE OUT output
  - 5 CARRY OUT output
  - 6 f output
  - 7 g output
  - 8 VSS ground
  - 9 d output
  - 10 a output
  - 11 e output
  - 12 b output
  - 13 c output
  - 14 UNGATED "C" SEGMENT OUT output
  - 15 RESET input
  - 16 VDD power
- **Pinout p16, 4033B (N, PW).** The same except:
  - 3 RIPPLE BLANKING IN input
  - 4 RIPPLE BLANKING OUT output
  - 14 LAMP TEST input
- **Behavior (common):**
  - The counter is a 5-stage Johnson decade counter with "antilock gating ... assuring proper counting sequence"; the illegal-state recovery path is not printed.
  - RESET is active-high and asynchronous to count 0. There is a Reset→output delay, and the reset removal time is 0 typ.
  - **Clock gating** (logic diagram): internal CL = CLOCK · NOT CLOCK INHIBIT. Both inputs are Schmitt-triggered, and rise/fall time is unlimited.
    - The counter advances on CLOCK ↑ while CLOCK INHIBIT = 0.
    - It also advances on CLOCK INHIBIT ↓ while CLOCK = 1. Text: "CLOCK INHIBIT signal can be used as a negative-edge clock if the clock line is held high."
  - **CARRY OUT** = Johnson stage output: high for counts 0–4, low for 5–9 (Fig. 3). Its rising edge occurs at the 9→0 transition, so it is a ÷10 square wave that clocks the next decade directly.
  - **Segment outputs are active-high (1 = segment on).** Read from the Fig. 3 waveforms and the digit glyphs in Fig. 4; 6 and 9 have tails.

    | count | a | b | c | d | e | f | g |
    |---|---|---|---|---|---|---|---|
    | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
    | 1 | 0 | 1 | 1 | 0 | 0 | 0 | 0 |
    | 2 | 1 | 1 | 0 | 1 | 1 | 0 | 1 |
    | 3 | 1 | 1 | 1 | 1 | 0 | 0 | 1 |
    | 4 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |
    | 5 | 1 | 0 | 1 | 1 | 0 | 1 | 1 |
    | 6 | 1 | 0 | 1 | 1 | 1 | 1 | 1 |
    | 7 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
    | 8 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
    | 9 | 1 | 1 | 1 | 1 | 0 | 1 | 1 |

- **4026B specifics:**
  - "In the CD4026B these outputs go high only when the DISPLAY ENABLE IN is high. When DISPLAY ENABLE IN is low the seven decoded outputs are forced low regardless of the state of the counter."
  - Fig. 3 confirms that all a–g are low while DEI is low.
  - DISPLAY ENABLE OUT = DISPLAY ENABLE IN (a buffered copy; Fig. 3 shows identical waveforms).
  - UNGATED "C" SEGMENT = the c column above (low only at count 2). It is not gated by DEI.
  - CARRY OUT is also ungated. Text: "CARRY OUT and UNGATED C-SEGMENT signals are not gated by the DISPLAY ENABLE ... a requirement in ... divide-by-60 and divide-by-12."
- **4033B specifics** (p2 text, Fig. 2, Fig. 4):
  - LAMP TEST high forces all seven outputs high. In the logic diagram LT is ORed into every output stage, so it overrides blanking.
  - Ripple blanking: when RBI = 0 and count = 0, all segments are low (blank) and RBO = 0. Otherwise RBO = 1, and segments follow the table.
    - In other words, RBO = NOT(NOT RBI · count0). The logic diagram has a NOR of RBI and NOT zero-detect.
    - Fig. 4 shows RBO low only during the blanked 0.
  - Chain: connect the MSD's RBI to low, then each RBO to the next lower digit's RBI. On the fraction side the chain runs the other way.
  - "The 7-segment outputs go high on selection in the CD4033B." There is no display-enable gating.
- **Timing at VDD = 5 V, typ (max), ns** (p4):
  - Clock→Carry Out 250 (500)
  - Clock→decode outputs 350 (700)
  - Reset→Carry Out (tPLH) 275 (550)
  - Reset→decode outputs 300 (600)
  - Carry-out transition time 100 (200)
  - fmax 5 MHz typ (2.5 min)
- **10/15 V typ, clock→Q:** carry 100 / 75 ns; decode outputs 125 / 90 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** 4026B CD4026BPW; 4033B CD4033BPW (CD4033BE is NRND).
- **Notes:** the 4033B has no display-enable input. LAMP TEST's effect on RBO is not stated; per the diagram LT only acts at the segment drivers.

## 40110B
- **Title:** "CMOS Decade Up-Down Counter/Latch/Display Driver".
- **Datasheet:** counters/cd40110b.pdf, pp1–9 (3-399 … 3-407).
  - p1: text, display glyphs.
  - p2: recommended conditions.
  - p3: static characteristics.
  - p4–5: logic diagram.
  - p5: terminal assignment.
  - p6: dynamic characteristics.
  - p7: truth table.
  - p9: cascading.
- **Families:** C: CD40110BE (the only orderable row).
- **Military:** none.
- **Pinout p16 (N):**
  - 1 a output
  - 2 g output
  - 3 f output
  - 4 TOGGLE ENABLE input (overbar; active-low)
  - 5 RESET input
  - 6 LATCH ENABLE input
  - 7 CLOCK DOWN input
  - 8 VSS ground
  - 9 CLOCK UP input
  - 10 CARRY output
  - 11 BORROW output
  - 12 e output
  - 13 d output
  - 14 c output
  - 15 b output
  - 16 VDD power
- **Truth table (p7):**

  | CLOCK UP | CLOCK DOWN | LATCH ENABLE | TOGGLE ENABLE | RESET | Counter | Display |
  |---|---|---|---|---|---|---|
  | ↑ | X | 0 | 0 | 0 | Increments by 1 | Follows counter |
  | X | ↑ | 0 | 0 | 0 | Decrements by 1 | Follows counter |
  | ↓ | ↓ | X | X | 0 | No change | No change |
  | X | X | 1 | X | 1 | Goes to 00000 | Remains fixed |
  | X | X | 0 | X | 1 | Goes to 00000 | Follows counter (display = 0) |
  | X | X | X | 1 | 0 | Inhibited | Remains fixed |
  | ↑ | X | 1 | 0 | 0 | Increments by 1 | Remains fixed |
  | X | ↑ | 1 | 0 | 0 | Decrements by 1 | Remains fixed |

  Footnote: "Typically 100 ns between clock-up and clock-down positive transitions are required to ensure proper counting."
- **Behavior:**
  - A dual-clock up/down counter with a "preconditioning circuit" that allows "the counter to be clocked, via positive going inputs, up or down regardless of the state or timing (within 100 ns typ.) of the other clock line".
    - Unlike the 40192B, the other clock does not need to be held high: each rising edge counts.
    - The maximum clock frequency applies to the sum of the CLK UP and CLK DN frequencies.
  - The counter is a 5-stage Johnson decade with anti-lock gating.
  - RESET is active-high and asynchronous: the counter goes to 0.
  - TOGGLE ENABLE is active-low: 1 inhibits counting.
  - LATCH ENABLE: 0 = the latch is transparent and the display follows the counter; 1 = the display holds while the counter keeps counting.
  - **Priority:** RESET > TOGGLE ENABLE (inhibit) > clocks. LATCH ENABLE acts only on the display path.
  - **CARRY / BORROW** are normally 1.
    - "A short duration negative-going pulse appears on the BORROW output when the count changes from 0 to 9 or the CARRY output when the count changes from 9 to 0."
    - Output pulse width typ: CARRY 230 ns, BORROW 275 ns at 5 V (min 115 / 140); 120/130 ns at 10 V; 80/85 ns at 15 V. This is a self-timed pulse, not clock-level gated.
    - Cascade: CARRY → next CLOCK UP, BORROW → next CLOCK DOWN.
  - **Segment outputs:** active-high (1 = on). They are bipolar source drivers ("can source up to 25 mA") for common-cathode LEDs, with VOH ≈ VDD − ~0.5…1.7 V under load (p3).
    - Glyphs (p1 "DISPLAY"): 6 has a top bar, 9 has a bottom bar, and 7 = a, b, c.
    - That gives the same table as the 4026B, but it is read from the glyph drawing only; no waveform is printed. The "1" glyph is drawn as a single slanted stroke, assumed to be b, c.
- **Timing at VDD = 5 V, typ (max), ns** (p6):
  - Clock→CARRY/BORROW 300 (600)
  - Clock→segment 925 (1850)
  - Reset→output 650 (1300)
  - Carry/borrow transition time 85 (170)
  - fmax 2.5 MHz typ (1 min)
  - Minimum delay between CLUP and CLDN: 100 typ
- **10/15 V typ, clock→segment:** 360 ns / 250 ns. Clock→carry/borrow is 100 / 70 ns.
- **VCC:** VDD 3–18 V.
- **Obsolete:** none. CD40110BE is Active and the only row.
- **Notes:**
  - The minimum reset removal time is printed as a negative typ (−275 ns at 5 V, max 0).
  - The text says D/F ceramic and H chip variants are supplied, but the census has only the E package.
