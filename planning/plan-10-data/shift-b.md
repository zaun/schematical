# shB — shift registers with storage, power-logic shift registers, FIFOs, serial converters

Extracted from the TI PDFs in `datasheets/shifters/` (paths below are relative to `datasheets/`).
Page numbers are **PDF page numbers** (1-based, as `pdftotext -f`/the PDF viewer count them), not the
printed folio. Scanned PDFs (no text layer for the datasheet body): `sn74ls595`, `sn74ls597`,
`sn74ls673`, `cd4094b`, `cd4031b`, `cd4034b`, `cd4517b`, `cd40105b` — read from page images.

Naming: the pin names are as printed, without overbars. Where the datasheet prints a prime (QH′, Q′S, Q′),
I keep it as `'` (e.g. `QH'`). The repo has no precedent for primes, so the planner should pick a spelling
(for example `QHP` or `QH_`). No device in this group has a pin that collides with another pin once its
overbar is dropped, except the CD4031B `Q`/`Q̄` pair, which becomes `Q`/`QBAR`.

## PDF → devices

| PDF | devices |
|---|---|
| shifters/sn74hc595.pdf | 74_595 (HC; SN54/SN74/SNJ54/SNV54) |
| shifters/sn74hct595.pdf | 74_595 (HCT) |
| shifters/cd74hc595.pdf | 74_595 (HC, CD74 prefix) |
| shifters/sn74ahc595.pdf | 74_595 (AHC) |
| shifters/sn74ahc595-q1.pdf | 74_595 (AHC, -Q1) |
| shifters/sn74ahct595.pdf | 74_595 (AHCT) |
| shifters/sn74hcs595.pdf | 74_595 (HCS) |
| shifters/sn74hcs595-q1.pdf | 74_595 (HCS, -Q1) |
| shifters/sn74lv595a.pdf | 74_595 (LV) |
| shifters/sn74lv595a-q1.pdf | 74_595 (LV, -Q1) |
| shifters/sn74lvc595a.pdf | 74_595 (LVC) |
| shifters/sn74ls595.pdf | 74_595 (LS). The PDF also covers LS596, but no LS596 rows are in the census. |
| shifters/sn74hc594.pdf | 74_594 (HC) |
| shifters/sn74ahc594.pdf | 74_594 (AHC) |
| shifters/sn74ahc594-q1.pdf | 74_594 (AHC, -Q1) |
| shifters/sn74ahct594.pdf | 74_594 (AHCT) |
| shifters/sn74lv594a.pdf | 74_594 (LV) |
| shifters/sn74hcs596.pdf | 74_596 (HCS) |
| shifters/sn74hcs596-q1.pdf | 74_596 (HCS, -Q1) |
| shifters/cd74hc597.pdf | 74_597 (HC, HCT) |
| shifters/sn74ls597.pdf | 74_597 (LS) **and** 74_598 (LS) |
| shifters/sn74ls673.pdf | 74_673 (LS) **and** 74_674 (LS) |
| shifters/tpic6595.pdf | TPIC6595 |
| shifters/tpic6a595.pdf | TPIC6A595 |
| shifters/tpic6b595.pdf | TPIC6B595 |
| shifters/tpic6c595.pdf | TPIC6C595 |
| shifters/tpic6a596.pdf | TPIC6A596 |
| shifters/tpic6b596.pdf | TPIC6B596 |
| shifters/tpic6c596.pdf | TPIC6C596 |
| shifters/cd4094b.pdf | 4094B |
| shifters/cd74hc4094.pdf | 74_4094 (HC, HCT) |
| shifters/cd4031b.pdf | 4031B |
| shifters/cd4517b.pdf | 4517B |
| shifters/cd4034b.pdf | 4034B |
| shifters/cd40105b.pdf | 40105B |
| shifters/cd74hc40105.pdf | 74_40105 (HC, HCT) |
| shifters/sn74lv8153.pdf | 74_8153 (LV) |
| shifters/sn74lv8153-q1.pdf | 74_8153 (LV, -Q1) |

**Census rows I could not map:** none. Every census row maps to a device below. Notes:
- `sn74ls597`: `SN74LS598N` is the only 598 row. All other rows are 597.
- `sn74ls673`: 673 and 674 rows are split by part name.
- Packages that appear in a datasheet but have no census row, and so are not orderable: HCS595 DYY (SOT-23-16);
  SN54AHC594 / SN54AHCT595 J/W/FK; SN54HC594; LS596; SN54LS595 FK.

---

## 74_595

- **Title:** "8-Bit Shift Registers With 3-State Output Registers" (HC p1). LS: "8-Bit Shift Registers with
  Output Latches" (sn74ls595 p1).
- **Datasheets:** sn74hc595 p1,3,4,6,8,11,12; sn74hct595 p1,3–6,12; cd74hc595 p3,4,8,12; sn74ahc595 p4,5,8,10,15;
  sn74ahc595-q1 p3,4,7,10; sn74ahct595 p3,4,6,10; sn74hcs595 p3,4,6,11,12; sn74hcs595-q1 (switching ≈ lines 415–455 of
  text; same values as HCS595); sn74lv595a p3,5,8,9,14,15; sn74lv595a-q1 p3,5,7,8, function table p≈15;
  sn74lvc595a p3,5,7,18; sn74ls595 p1–5.
- **Families & prefixes:**
  - HC: SN74HC595*, SN54HC595*, SNJ54HC595*, SNV54HC595*, CD74HC595*
  - HCT: SN74HCT595*
  - AHC: SN74AHC595*, SN74AHC595Q*-Q1
  - AHCT: SN74AHCT595* (the PDF title also names SN54AHCT595, but it has no census rows)
  - HCS: SN74HCS595*, SN74HCS595Q*-Q1
  - LV: SN74LV595A*, SN74LV595AI*/AQ*-Q1
  - LVC: SN74LVC595A*
  - LS: SN74LS595*, SN54LS595*, SNJ54LS595*
- **Military:**
  - sn74hc595:
    - 5962-86816012A = SNJ54HC595FK (FK)
    - 5962-8681601EA = SNJ54HC595J
    - 5962-8681601VEA = SNV54HC595J
    - 5962-8681601VFA = SNV54HC595W (CFP W16)
  - sn74ls595:
    - 5962-8671701EA = SNJ54LS595J
    - 5962-8671701FA = SNJ54LS595W
- **Pin names differ for LS595.** LS prints `G` for OE, `RCK` for RCLK and `SRCK` for SRCLK (sn74ls595 p1). The function and
  pin numbers are identical. The planner decides: one device with aliased names, or a separate pinout naming.
- **Pinouts**
  - p16 (all of the following share it): HC D, DB, DW, N, NS, PW, J, W; CD74HC595 D(M), DW, N(E), NS, DB(SM);
    HCT PW; AHC D, DB, N, PW; AHC-Q1 PW; AHCT D, DB, N, PW; HCS D, PW; LV D, NS, PW; LV-Q1 PW;
    LVC PW; LS D, N, J, W.
    `1 QB output, 2 QC output, 3 QD output, 4 QE output, 5 QF output, 6 QG output, 7 QH output, 8 GND ground,
    9 QH' output, 10 SRCLR input, 11 SRCLK input, 12 RCLK input, 13 OE input, 14 SER input, 15 QA output,
    16 VCC power` (HC p3; HCT p3; LS p1 with LS names 10 SRCLR, 11 SRCK, 12 RCK, 13 G).
  - p16 + thermal pad (same 1–16 map plus `PAD` exposed thermal pad): AHC BQB, AHC-Q1 BQB (WBQB), AHCT BQB,
    HCS BQB, HCS-Q1 BQB/WBQB, LV BQB and RGY, LV-Q1 WBQB, LVC BQB (AHC p4; LV p3: "BQB or RGY Package 16-Pin
    WQFN or VQFN"; HCS p3, footnote "BQB package only" on PAD). Treat PAD as nc/thermal (not a logic pin).
  - FK20 (SNJ54HC595FK, 5962-86816012A). From the drawing on HC p3:
    `1 NC nc, 2 QB output, 3 QC output, 4 QD output, 5 QE output, 6 NC nc, 7 QF output, 8 QG output,
    9 QH output, 10 GND ground, 11 NC nc, 12 QH' output, 13 SRCLR input, 14 SRCLK input, 15 RCLK input, 16 NC nc,
    17 OE input, 18 SER input, 19 QA output, 20 VCC power`.
    **The pin table on the same page has errors:** it lists RCLK as LCCC pin 14 (the drawing shows 15), lists the NC pins
    as "1, 16, 11, 16" (they are 1, 6, 11, 16), and leaves VCC's DIP pin blank (it is 16).
- **Behavior** (HC p11–12; HCT p12; LV p14–15; LS p1–2 logic diagram):
  - Shift register (8 D flip-flops). On **SRCLK ↑** (with SRCLR = H): stage A ← SER, stage n ← stage n−1.
    SRCLK ↓ does nothing.
  - **SRCLR** is active low, **asynchronous** and direct-overriding. It clears **only the shift register** (the "R" inputs are
    on the shift stages; storage flops have none: HC p11, LS p2, HCS596 p10). It does not touch the storage register
    or OE. Priority in the shift register: SRCLR = L wins over SRCLK.
  - Storage register (8 D flip-flops). On **RCLK ↑**, storage ← current shift register contents. RCLK is independent of
    SRCLR. The function table row "X X X ↑ X" (HC p12) has SRCLR = X, so if SRCLR is low the storage loads zeros.
    Setup "SRCLR low before RCLK↑" is specified (HC p6).
  - **SRCLK and RCLK tied together:** "the shift register always is one clock pulse ahead of the storage register"
    (HC p11; footnote HC p6). On a common rising edge the storage register captures the shift contents **before**
    that edge's shift (the function table's row "X ↑ H ↑ X: Data in shift register is stored in the storage
    register, the data is then shifted through", HCT p12, CD74HC p12, HCS p12, LVC p18).
  - **QH'** = shift-register stage H (the last shift stage, not the storage register). It changes on SRCLK ↑ and on
    SRCLR ↓. It is **always driven (push-pull); OE has no effect on it** ("Internal register data and serial output (QH')
    are not impacted by the operation of the OE input", HCT p1; LV function table "QH' remains enabled", LV p14).
  - **QA–QH** = storage register bits, through 3-state buffers. **OE active low**: OE = H → QA–QH Hi-Z; OE = L → enabled.
    OE is asynchronous and does not alter register contents.
  - Power-up state: unknown ("When the device is powered on, the state of each latch is unknown", HCT p11; LV p14).
  - LS595 has no printed function table; its behavior comes from the description and logic diagram (LS p1–2), which match the above.
- **Timing (typical unless noted)**

  | family | condition | SRCLK→QH' | RCLK→Q | SRCLR→QH' (tPHL) | OE→Q en | OE→Q dis | source |
  |---|---|---|---|---|---|---|---|
  | HC, CD74HC | 4.5 V, 50 pF | 17 | 17 | 18 | 15 | 23 | HC p8; CD74HC p8 |
  | HCT | 4.5 V, 50 pF | 42 (25 °C max; no typical printed) | 40 (max) | 40 (max) | 35 (max) | 30 (max) | HCT p6 |
  | AHC, AHC-Q1, AHCT | 5 V, 50 pF | 6.4 | 5.6 | 6.4 | tPZH 5.7 / tPZL 6.8 | tPHZ 3.5 / tPLZ 3.4 | AHC p10; AHC-Q1 p7; AHCT p6 |
  | HCS, HCS-Q1 | 4.5 V, 50 pF | 8 | 9 | 8 | 9 | 11 | HCS p6 |
  | LV, LV-Q1 | 3.3 V, 50 pF | 9.2 | 7.9 | 9.0 | tPZH 7.8 / tPZL 9.6 | tPHZ 8.1 / tPLZ 9.3 | LV p8; LV-Q1 p8 |
  | LVC | 3.3 V, 50 pF | 3.4 | 3.4 | 3.5 | 3.5 / 3.5 | 3.5 / 3.5 | LVC p7 (TYP column is −40..125 °C) |
  | LS | 5 V | 12 (LH) / 17 (HL), 30 pF | 12 / 24, 45 pF | 24, 30 pF | tPZH 20 / tPZL 25, 45 pF | tPHZ 20 / tPLZ 25, 5 pF | LS p5 |

  At 5 V the AHC and LV RCLK→Q and SRCLK→QH' figures are identical (LV p9).
- **VCC (Recommended Operating Conditions)**
  - HC 2–6 V; SN54 and SN74 are the same (HC p4). CD74HC 2–6 (p4).
  - HCT 4.5–5.5 (p4).
  - AHC 2–5.5, inputMax 5.5 (p5). AHC-Q1 2–5.5, inputMax 5.5 (p4).
  - AHCT 4.5–5.5, inputMax 5.5 (p4).
  - HCS 2–6 (VI 0–VCC).
  - LV 2–5.5, inputMax 5.5 (p5). LV-Q1 same.
  - LVC 1.1–3.6, inputMax 5.5 (ROC VI max 5.5; p5).
  - LS: SN54 4.5–5.5, SN74 4.75–5.25 (p3). Absolute input 7 V.
- **Obsolete (census):** CD74HC595DW, CD74HC595M, CD74HC595MT, SN74AHC595D, SN74AHC595PW, SN74AHCT595D,
  SN74AHCT595PW, SN74HC595DT, SN74HC595DW, SN74HC595PW, SN74LS595D, SN74LV595AD, SN74LV595APWT.
- **Notes:**
  - HCS inputs are Schmitt triggers.
  - LS595 QH' is a totem-pole output with a different drive from QA–QH.
  - The HCS595 PDF lists a DYY (SOT-23-16) package with the same pin map, but it has no census row.
  - LVC switching gives a single "typ" column over −40 to 125 °C.

## 74_594

- **Title:** "8-Bit Shift Registers With Output Registers" (no 3-state; storage clear instead of OE).
- **Datasheets:** sn74hc594 p3,4,6,12; sn74ahc594 p3(pin page),5,7–8 (switching), 11 (logic diagram), function table ≈ p12;
  sn74ahc594-q1 p3–7; sn74ahct594 p3–6; sn74lv594a p3–7.
- **Families:**
  - HC: SN74HC594* (the PDF also titles SN54HC594, which has no census rows)
  - AHC: SN74AHC594*, SN74AHC594Q*-Q1
  - AHCT: SN74AHCT594*
  - LV: SN74LV594A*
- **Military:** none in the census.
- **Pinouts**
  - p16: HC D, DW, N; AHC D, DB, N, NS, PW; AHC-Q1 PW; AHCT D, DB, N, NS, PW; LV D, DB, PW.
    `1 QB output, 2 QC output, 3 QD output, 4 QE output, 5 QF output, 6 QG output, 7 QH output, 8 GND ground,
    9 QH' output, 10 SRCLR input, 11 SRCLK input, 12 RCLK input, 13 RCLR input, 14 SER input, 15 QA output,
    16 VCC power` (HC p3 drawing; AHC p3).
  - p16 + PAD: AHC-Q1 WBQB, LV BQB (same map + exposed `PAD`, LV p3).
  - The AHC594 PDF also prints SN54 FK20 (1 NC, 2 QB, 3 QC, 4 QD, 5 QE, 6 NC, 7 QF, 8 QG, 9 QH, 10 GND, 11 NC,
    12 QH', 13 SRCLR, 14 SRCLK, 15 RCLK, 16 NC, 17 RCLR, 18 SER, 19 QA, 20 VCC). No census row uses it.
  - **The HC594 pin table (p3) has errors:** pin 13 is printed as "RCLK — Storage clear" (it is RCLR), and pin 9 is described as
    "QH inverted", but the logic diagram (AHC p11) shows QH' as the non-inverted output of shift stage H.
- **Behavior** (HC p12 function table; AHC p11 diagram):
  - Shift register: SRCLK ↑ with SRCLR = H shifts (stage A ← SER). "L ↓ H X X: shift register state is not changed".
  - **SRCLR** (active low, async) clears the shift register only. This also forces QH' low.
  - **RCLR** (active low, async) clears the storage register only. QA–QH go low.
  - RCLK ↑ with RCLR = H: storage ← shift register. RCLK ↓: no change. RCLR = L overrides RCLK.
  - QA–QH are **always-driven push-pull** (no OE).
  - QH' = shift stage H. Clocks tied: the storage register is one clock behind (HC p6 note: "the output register is one clock
    pulse behind the shift register"; AHC p7 note: "shift register is one clock pulse ahead").
- **Timing (typical)**

  | family | condition | SRCLK→QH' | RCLK→Q | SRCLR→QH' | RCLR→Q (tPHL) | source |
  |---|---|---|---|---|---|---|
  | HC | 4.5 V, 50 pF | 30 | 30 | 30 | 25 | HC p6 |
  | AHC | 5 V, 50 pF | 5.5 LH / 6.0 HL | 4.9 LH / 5.8 HL | 6.0 | 6.6 | AHC p8 |
  | AHC-Q1 | 5 V, 50 pF | 6.4 | 5.6 | 6.4 | 6.6 | AHC-Q1 p7 |
  | AHCT | 5 V, 50 pF | 5.5 / 6.0 | 4.9 / 5.8 | 6.0 | 6.6 | AHCT p6 |
  | LV | 3.3 V, 50 pF | no typ; −40..85 °C max 12.4 LH / 13.9 HL | max 11.1 / 13.1 | max 14.0 | max 14.4 | LV p7 |

  For LV at 3.3 V, 15 pF the typicals are: SRCLK→QH' 5.4/5.5, RCLK→Q 4.6/4.9, RCLR→Q 6.0, SRCLR→QH' 5.6.
  pdftotext mis-aligns the LV594A FROM column; I aligned it by matching AHC594's identical numbers.
- **VCC:**
  - HC 2–6 (p4).
  - AHC 2–5.5, inputMax 5.5 (AHC ROC). AHC-Q1 2–5.5, inputMax 5.5.
  - AHCT 4.5–5.5, inputMax 5.5.
  - LV 2–5.5, inputMax 5.5.
- **Obsolete:** SN74AHC594D, SN74AHC594PW, SN74AHCT594D, SN74HC594D, SN74HC594DT, SN74HC594DW, SN74LV594AD.

## 74_596

- **Title:** "SN74HCS596 8-Bit Shift Register With Schmitt-Trigger Inputs and Open-Drain Output Registers" (hcs596 p1).
- **Datasheets:** sn74hcs596 p1,3,6,10,12; sn74hcs596-q1 (same tables).
- **Families:** HCS: SN74HCS596*, SN74HCS596Q*-Q1. (The LS596 is printed in sn74ls595.pdf, open-collector, but has no
  census rows.)
- **Military:** none.
- **Pinout** p16: D, PW (and -Q1 D, PW). Same pin map as 74_595: `1 QB, 2 QC, 3 QD, 4 QE, 5 QF, 6 QG, 7 QH, 8 GND,
  9 QH', 10 SRCLR, 11 SRCLK, 12 RCLK, 13 OE, 14 SER, 15 QA, 16 VCC` (p3). QA–QH are outputs (open-drain), and QH' is a push-pull output.
- **Behavior:**
  - Identical to 74_595 (same function table p12, including the tied-clock row) except for the outputs.
  - **QA–QH are open-drain and non-inverting.** Storage bit 0 → output pulled low. Storage bit 1 → output off (Hi-Z, needs a pull-up).
    Sources: "Outputs QA through QH are open-drain which require a pull-up resistor to output a High" (p10); logic
    diagram p10 shows storage Q → buffer (no inversion) → open-drain output; "When in the high logical state,
    open-drain outputs will be in a high-impedance state" (p10).
  - **OE = H** → all QA–QH off (Hi-Z). Equation: Qn pulled low ⇔ OE = L ∧ storage bit n = 0.
  - **QH' is push-pull** (p3, p10), always enabled.
- **Timing** (4.5 V, 50 pF, typ; p6): SRCLK→QH' 8, RCLK→Q 9, SRCLR→QH' 8, OE ten 9, tdis 11.
  The 85–125 °C max is 25 ns, which differs from HCS595.
- **VCC:** 2–6 V (ROC). VI 0–VCC.
- **Obsolete:** none.

## 74_597

- **Title:** LS: "8-Bit Shift Registers With Input Latches" (sn74ls597 p1). HC: "8-Bit Shift Register with Input Storage"
  (cd74hc597 p1).
- **Datasheets:** sn74ls597 p1–3,5–8; cd74hc597 p1,2,3,5–7.
- **Families:**
  - LS: SN74LS597*, SN54LS597*, SNJ54LS597*
  - HC: CD74HC597*, CD54HC597*
  - HCT: CD74HCT597*
- **Military:**
  - 5962-89444012A = SNJ54LS597FK
  - 5962-8944401EA = SNJ54LS597J
  - 5962-8944401FA = SNJ54LS597W
  - 5962-8681701EA = CD54HC597F3A
- **Pin names differ between LS and HC** (same positions and function):

  | pin | LS597 (p1) | CD74HC597 (p1) | role |
  |---|---|---|---|
  | 1 | B | D1 | input |
  | 2 | C | D2 | input |
  | 3 | D | D3 | input |
  | 4 | E | D4 | input |
  | 5 | F | D5 | input |
  | 6 | G | D6 | input |
  | 7 | H | D7 | input |
  | 8 | GND | GND | ground |
  | 9 | QH' | Q7 | output |
  | 10 | SRCLR | MR | input |
  | 11 | SRCK | SHCP | input |
  | 12 | RCK | STCP | input |
  | 13 | SRLOAD | PL | input |
  | 14 | SER | DS | input |
  | 15 | A | D0 | input |
  | 16 | VCC | VCC | power |

  - p16: LS N, D, J, W; HC/HCT N(E), D(M), NS, J(F3A).
  - FK20 (SNJ54LS597FK, LS p1 drawing): `1 NC, 2 B, 3 C, 4 D, 5 E, 6 NC, 7 F, 8 G, 9 H, 10 GND, 11 NC, 12 QH',
    13 SRCLR, 14 SRCK, 15 RCK, 16 NC, 17 SRLOAD, 18 SER, 19 A, 20 VCC`.
- **Behavior:**
  - **Storage register** (8 D flip-flops; LS prints them "1D ▷C1"): on **RCK/STCP ↑**, storage ← A–H / D0–D7. It is
    independent of every other input ("STCP ↑ X X X: Data Loaded to Input Flip-Flops", HC p2).
    The LS description says both clocks are positive-edge triggered (LS p1).
  - **SRLOAD/PL (active low, asynchronous):** while it is low, the shift register is forced (direct S/R) to the storage register
    contents. This is level-transparent: "No clock edge, X, L, H: Data Transferred from Input Flip-Flops to Shift Register".
    A STCP ↑ while PL = L loads the inputs into storage and the new data flows straight into the shift register
    ("↑ X L H: Data Loaded from Inputs to Shift Register", HC p2).
  - **SRCLR/MR (active low, asynchronous):** clears the shift register only. The storage register is not cleared.
  - **PL = L and MR = L together: "Invalid Logic, State of Shift Register Indeterminate when Signals Removed"** (HC p2).
    The LS logic diagram drives both S and R of each stage in this case.
  - **Shift:** on SRCK/SHCP ↑ with PL = H and MR = H, Q0 ← DS/SER and Qn ← Qn−1 (HC p2: "Qn = Qn-1, Q0 = DS").
  - **Output:** QH'/Q7 = last shift stage. It is push-pull and always enabled (there is no OE).
  - Priority on the shift register: (PL ∧ MR both low = undefined) > MR low (clear) > PL low (load) > SHCP ↑ (shift).
  - Setup: "RCK↑ before SRLOAD↑" 40 ns (LS p5), "STCP to SHCP setup" (HC p5).
- **Timing**
  - LS (5 V, CL 30 pF, typical; LS p7): SRCK↑→QH' 15 LH / 20 HL; SRLOAD↓→QH' 38 / 29; SRCLR↓→QH' 24;
    RCK↑→QH' (SRLOAD = L) 41 / 32.
  - HC (4.5 V, 50 pF; only a 25 °C MAX is printed, typicals are given at 5 V / 15 pF; HC p6):
    SHCP→Q7 35 (typ 14), PL→Q7 40 (17), STCP→Q7 48 (20), MR→Q7 35 (14).
  - HCT (4.5 V, 50 pF max; typ 5 V/15 pF; p6–7): SHCP→Q7 38 (16), PL→Q7 48 (20), STCP→Q7 56 (23), MR→Q7 44 (18).
- **VCC:**
  - LS: SN54 4.5–5.5, SN74 4.75–5.25 (LS p5).
  - HC 2–6 (HC p3).
  - HCT 4.5–5.5 (HC p1 features).
- **Obsolete:** CD74HC597M, CD74HC597MT, CD74HCT597M, CD74HCT597MT.
- **Notes:** the LS597/598 PDF prints no function table, so the behavior is taken from the logic diagram (p3–4), the typical operating
  sequences (p8) and the CD74HC597 function table.

## 74_598

- **Title:** "8-Bit Shift Registers With Input Latches" ('LS598, 20-pin; LS p1).
- **How it differs from 597** (LS p1):
  - "has all the features of the 'LS597 plus 3-state I/O ports that provide parallel shift register outputs and also
    has multiplexed serial data inputs".
  - The parallel pins A/QA…H/QH are **bidirectional**.
  - It adds **G** (output enable), **SRCKEN** (shift clock enable), **DS** (serial-input select) and **SER0/SER1**.
- **Datasheets:** sn74ls597 p1,2,4,5,7,8.
- **Families:** LS: SN74LS598* (and SN54LS598, which has no census rows).
- **Military:** none.
- **Pinout** p20 (N; the PDF also lists J/W/DW; LS p1):
  `1 A/QA inout, 2 B/QB inout, 3 C/QC inout, 4 D/QD inout, 5 E/QE inout, 6 F/QF inout, 7 G/QG inout, 8 H/QH inout,
  9 SRLOAD input, 10 GND ground, 11 QH' output, 12 SRCLR input, 13 SRCK input, 14 SRCKEN input, 15 RCK input,
  16 G input, 17 SER1 input, 18 SER0 input, 19 DS input, 20 VCC power`.
  The SN54LS598 FK drawing exists (p1) but has no census row.
  Suggested names: `A`…`H` printed as "A/QA" etc. The planner should choose (e.g. `AQA`).
- **Behavior** (logic diagram LS p4; sequence p8):
  - Storage flip-flops (RCK ↑) capture from the A/QA…H/QH pins. When G = L the pins carry the shift register's own outputs,
    so RCK then stores the shift register contents.
  - **G active low:** G = L drives the shift-register stage outputs QA–QH onto the I/O pins. G = H puts the pins in Hi-Z (input mode).
  - **Serial input = DS ? SER1 : SER0.** The diagram shows DS inverted ANDed with SER0 and DS ANDed with SER1, then ORed.
    So DS = L selects SER0 and DS = H selects SER1. This is read from the gate drawing; no table is printed.
  - **SRCKEN active low:** SRCK ↑ shifts only if SRCKEN = L. Setup "SRCKEN low before SRCK↑" is 20 ns (p5). Internally
    SRCKEN is captured by a small latch on the SRCK path. Model it as enable sampled at the SRCK rising edge.
  - SRLOAD (async parallel load from storage), SRCLR (async clear of the shift register only) and the conflict case are
    as for 74_597.
  - **QH'** = last shift stage, always driven.
- **Timing** (5 V, typical, LS p7):
  - SRCK↑→QH' 11/15; SRLOAD↓→QH' 28/20; SRCLR↓→QH' 18; RCK↑→QH' 32/24 (30 pF).
  - SRCK↑→Q 12/19; SRLOAD↓→Q 32/27; SRCLR↓→Q 25 (45 pF).
  - G↓→Q tPZH 26 / tPZL 29; G↑→Q tPHZ 25 / tPLZ 20 (5 pF).
- **VCC:** SN74 4.75–5.25, SN54 4.5–5.5. The I/O ports' absolute input maximum is 5.5 V; the other inputs' is 7 V (p5).
- **Obsolete:** none (SN74LS598N is Active).

## 74_673

- **Title:** "16-Bit Shift Registers" — 'LS673: "16-Bit Serial-In, Serial-Out Shift Register with 16-Bit Parallel-Out
  Storage Register" (sn74ls673 p1).
- **Datasheets:** sn74ls673 p1–6.
- **Families:** LS: SN74LS673*, SN54LS673*, SNJ54LS673*.
- **Military:**
  - 5962-88602013A = SNJ54LS673FK
  - 5962-8860201JA = SNJ54LS673J
  - 5962-8860201LA = SNJ54LS673JT (CDIP 24, 300 mil)
- **Pinouts**
  - p24: DW, N, J, JT, W (p1):
    `1 CS input, 2 SHCLK input, 3 RW input, 4 STRCLR input, 5 MODESTRCLK input, 6 SERQ15 inout, 7 Y0 output, 8 Y1, 9 Y2,
    10 Y3, 11 Y4 output, 12 GND ground, 13 Y5 output, 14 Y6, 15 Y7, 16 Y8, 17 Y9, 18 Y10, 19 Y11, 20 Y12, 21 Y13, 22 Y14,
    23 Y15 output, 24 VCC power`.
    Printed names are "SH CLK", "R/W̄", "MODE/STRCLK", "SER/Q15". The `/`, spaces and overbars are dropped above; the planner chooses.
  - FK28 (p1):
    `1 NC, 2 CS, 3 SHCLK, 4 RW, 5 STRCLR, 6 MODESTRCLK, 7 SERQ15, 8 NC, 9 Y0, 10 Y1, 11 Y2, 12 Y3, 13 Y4, 14 GND,
    15 NC, 16 Y5, 17 Y6, 18 Y7, 19 Y8, 20 Y9, 21 Y10, 22 NC, 23 Y11, 24 Y12, 25 Y13, 26 Y14, 27 Y15, 28 VCC`.
- **Function table** (p2), quoted. Columns are CS̄, R/W̄, SH CLK, STRCLR̄, MODE/STRCLK → SER/Q15 | shift | read | write | parallel load |
  storage clear | storage load:

  | CS | R/W | SHCLK | STRCLR | MODE/STRCLK | SER/Q15 | shift | read | write | par load | st clear | st load |
  |---|---|---|---|---|---|---|---|---|---|---|---|
  | H | X | X | X | X | Z | NO | NO | NO | NO | | NO |
  | X | X | X | L | X | | | | | | YES | |
  | L | L | ↓ | X | X | Z | YES | NO | YES | NO | | |
  | L | H | X | X | X | Q15 | | YES | NO | | | NO |
  | L | H | ↓ | X | L | Q14n | YES | YES | NO | NO | | NO |
  | L | H | ↓ | L | H | L | NO | YES | | YES | YES | NO |
  | L | H | ↓ | H | H | Y15n | NO | YES | | YES | NO | NO |
  | L | L | X | H | ↑ | Z | | NO | | NO | NO | YES |

  The table's legend: Q14n = content of the 14th bit before the most recent ↓ of the clock; Q15 = present content of the 15th
  bit; Y15n = storage bit 15 before the most recent ↓.
- **Behavior:**
  - The shift register (Q0–Q15) shifts on **SH CLK ↓** (falling edge) when CS = L. Data moves Q0 → Q15.
  - **Serial input:** the shift register's SER IN is wired to the SER/Q15 pin (block diagram p4).
    - R/W = L ("write"): SER/Q15 is Hi-Z (an input), so Q0 ← the external pin.
    - R/W = H ("read"): SER/Q15 drives Q15 and the register recirculates, so Q0 ← Q15.
  - **SER/Q15** is a 3-state output, enabled ⇔ CS = L ∧ R/W = H. It shows Q15.
  - **MODE/STRCLK** is dual-purpose:
    - With R/W = H it is MODE. MODE = L means shift. MODE = H means **parallel load the shift register from the storage register**
      (Q ← Y) on SH CLK ↓, with no shifting. If STRCLR = L at the same time, the storage register is cleared and zeros are loaded.
    - With R/W = L (and CS = L) its **rising edge** loads storage ← shift register (Y ← Q).
  - **STRCLR active low, asynchronous:** clears the storage register (Y0–Y15 → L). It is **not** disabled by CS (p1).
  - **CS = H** disables both the shift clock and the store clock and sets SER/Q15 to Z.
  - Clock gating is an AND with CS (block diagram p4). The datasheet warns: "The shift clock should be low during the
    low-to-high transition of chip select and the store clock should be low during the high-to-low transition of chip select"
    (p1). A CS edge with the clock in the wrong state creates a false clock edge, so model the gated clocks literally.
  - **Y0–Y15** are always-driven totem-pole outputs of the storage register.
- **Timing** (5 V, 25 °C, typical; p6):
  - SH CLK→SER/Q15 tPLH 21 / tPHL 26 (45 pF).
  - STRCLR→Y tPHL 25 (15 pF).
  - MODE/STRCLK→Y tPLH 28 / tPHL 30 (15 pF).
  - CS or R/W → SER/Q15: tPZH 30 / tPZL 30 (45 pF); tPHZ 25 / tPLZ 25 (5 pF).
- **VCC:** SN74 4.75–5.25, SN54 4.5–5.5 (p5). The SER/Q15 absolute input maximum is 5.5 V.
- **Obsolete:** none.

## 74_674

- **Title:** 'LS674: "16-Bit Parallel-In, Serial-Out Shift Register" (p1).
- **Datasheets:** sn74ls673 p1–6.
- **Families:** LS: SN74LS674*, SN54LS674*, SNJ54LS674*.
- **Military:**
  - 5962-88607013A = SNJ54LS674FK
  - 5962-8860701JA = SNJ54LS674J
- **Pinouts**
  - p24: DW, N, J, W (p2):
    `1 CS, 2 CLK, 3 RW, 4 NC, 5 MODE, 6 SERQ15 inout, 7 P0, 8 P1, 9 P2, 10 P3, 11 P4, 12 GND, 13 P5, 14 P6, 15 P7, 16 P8,
    17 P9, 18 P10, 19 P11, 20 P12, 21 P13, 22 P14, 23 P15, 24 VCC`. P0–P15 are inputs.
  - FK28 (p2):
    `1 NC, 2 CS, 3 CLK, 4 RW, 5 NC, 6 MODE, 7 SERQ15, 8 NC, 9 P0, 10 P1, 11 P2, 12 P3, 13 P4, 14 GND, 15 NC, 16 P5, 17 P6,
    18 P7, 19 P8, 20 P9, 21 P10, 22 NC, 23 P11, 24 P12, 25 P13, 26 P14, 27 P15, 28 VCC`.
- **Function table** (p2):

  | CS | R/W | MODE | CLK | SER/Q15 | operation |
  |---|---|---|---|---|---|
  | H | X | X | X | Z | Do nothing |
  | L | L | X | ↓ | Z | Shift and write (serial load) |
  | L | H | L | ↓ | Q14n | Shift and read |
  | L | H | H | ↓ | P15 | Parallel load |

- **Behavior:**
  - Clocks on CLK ↓, gated by CS.
  - **Parallel load is synchronous:** Q ← P0–P15 on CLK ↓ when CS = L, R/W = H and MODE = H. The block diagram note says "When PE is active, data
    is synchronously parallel loaded … and no shifting takes place".
  - Serial in, SER/Q15 enable and recirculation behave as in 74_673: write mode takes the pin, read mode recirculates Q15.
  - SER/Q15 is enabled ⇔ CS = L ∧ R/W = H.
  - "Low-to-high-level changes at the chip select input should be made only when the clock input is low" (p1).
- **Timing** (5 V, typical; p6): CLK→SER/Q15 21 / 26 (45 pF); CS, R/W→SER/Q15 tPZH 30 / tPZL 30, tPHZ 25 / tPLZ 25.
- **VCC:** as 74_673.
- **Obsolete:** none.

---

## TPIC6595

- **Title:** "Power Logic 8-Bit Shift Register" (tpic6595).
- **Datasheets:** tpic6595 p1,3–5,10 (logic diagram).
- **Families:** TPIC: TPIC6595*.
- **Military:** none.
- **Pinout** p20, DW and N (p3 pin table; the logic diagram p10 agrees):
  `1 PGND ground, 2 VCC power, 3 SERIN input, 4 DRAIN0 output, 5 DRAIN1, 6 DRAIN2, 7 DRAIN3 output, 8 SRCLR input,
  9 G input, 10 PGND ground, 11 PGND ground, 12 RCK input, 13 SRCK input, 14 DRAIN4 output, 15 DRAIN5, 16 DRAIN6,
  17 DRAIN7 output, 18 SEROUT output, 19 LGND ground, 20 PGND ground`.
  The names print as "SER IN"/"SER OUT".
- **Behavior** (shared by all TPIC 595s; the 596 differences are in their own sections):
  - Shift register on **SRCK ↑**. Storage register on **RCK ↑**.
  - **SRCLR (active low, async):** per the logic diagram p10, it **clears only the shift register** (the storage flip-flops have no CLR).
    **This contradicts** §7.3.2 text "A logical low on (SRCLR) clears all registers in the device" (text ≈ line 425) and the
    description "When SRCLR is low, the input shift register is cleared". Recommend following the diagram and the p1 description.
  - **G (active low):** G = H forces "all data in the output buffers … low, and all drain outputs are off". G = L makes
    storage data "transparent to the output buffers".
  - **DRAINn polarity:** buffer data high → the DMOS conducts (sinks; the pin pulls low). Data low → off. So
    **DRAINn ON (low) ⇔ G = L ∧ storage bit n = 1** (an AND gate in the logic diagram). DRAINn is open-drain with no high drive. This is
    inverting in the sense that a stored 1 gives a low pin.
  - **SER OUT** = the last shift stage (stage 7, i.e. the bit that will feed DRAIN7 on the next RCK), clocked on **SRCK ↑**.
    The diagram takes the Q̄ of FF8 through an inverter, so the output is non-inverted.
  - "Write data and read data are valid only when RCK is low" (description) is a timing caution, not a mode.
  - "The storage register transfers data to the output buffer when SRCLR is high" (description) contradicts the logic diagram, where SRCLR does
    not gate the outputs. Treat it as a description artefact.
- **Timing** (VCC 5 V, TC 25 °C, CL 30 pF, ID 250 mA; p5): G→DRAIN tPLH 650 (turn-off), tPHL 150 (turn-on);
  tr 750, tf 425 ns. No SRCK→SER OUT figure is printed.
- **VCC:** logic supply 4.5–5.5 V (p4). VIH 0.85 VCC, VIL 0.15 VCC. Abs VI −0.3…7 V.
- **Ratings (Notes):** VDS 45 V; 250 mA continuous per output (all on); 750 mA pulsed; 2 A peak single output.
- **Obsolete:** TPIC6595DW, TPIC6595DWG4, TPIC6595DWRG4.

## TPIC6A595

- **Datasheets:** tpic6a595 p3–5 (pin tables), 12 (logic diagram).
- **Families:** TPIC: TPIC6A595*.
- **Pinouts** (pin tables p3–4):
  - NE20 (PDIP):
    `1 DRAIN2 output, 2 DRAIN3 output, 3 SRCLR input, 4 G input, 5 PGND ground, 6 PGND ground, 7 RCK input,
    8 SRCK input, 9 DRAIN4, 10 DRAIN5, 11 DRAIN6, 12 DRAIN7 output, 13 SEROUT output, 14 LGND ground, 15 PGND, 16 PGND,
    17 VCC power, 18 SERIN input, 19 DRAIN0, 20 DRAIN1 output`.
  - DW24 (SOIC):
    `1 DRAIN2, 2 DRAIN3, 3 SRCLR, 4 G, 5 PGND, 6 PGND, 7 PGND, 8 PGND, 9 RCK, 10 SRCK, 11 DRAIN4, 12 DRAIN5, 13 DRAIN6,
    14 DRAIN7, 15 SEROUT, 16 LGND, 17 PGND, 18 PGND, 19 PGND, 20 PGND, 21 VCC, 22 SERIN, 23 DRAIN0, 24 DRAIN1`.
  - There are no NC pins; the extra DW pins are PGND. The pin table calls LGND "Line ground", a typo for logic ground.
- **Behavior:** same as TPIC6595.
  - Logic diagram p12: SRCLR clears the shift register only; SER OUT is from FF8 on SRCK ↑. The p12 text again says
    "clears all registers", which is the same contradiction.
  - There is an internal current limit and charge pump.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 350 mA; p5): G→DRAIN tPHL 30, tPLH 125; tr 60, tf 30.
- **VCC:** 4.5–5.5. **Ratings:** VDS 50 V; 350 mA continuous each; 1.1 A pulsed.
- **Obsolete:** TPIC6A595DW, TPIC6A595DWG4.

## TPIC6B595

- **Datasheets:** tpic6b595 p1,3,4,6,11,12.
- **Families:** TPIC: TPIC6B595*.
- **Pinout** p20, DW and N (p3 drawing):
  `1 NC, 2 VCC, 3 SERIN, 4 DRAIN0, 5 DRAIN1, 6 DRAIN2, 7 DRAIN3, 8 SRCLR, 9 G, 10 GND, 11 GND, 12 RCK, 13 SRCK,
  14 DRAIN4, 15 DRAIN5, 16 DRAIN6, 17 DRAIN7, 18 SEROUT, 19 GND, 20 NC`.
  **The pin table on p3 says SRCK = 15. That is wrong;** the drawing and the logic diagram p11 say 13.
- **Behavior:**
  - Same as TPIC6595. The logic diagram p11 shows the SRCLR clearing only the shift FFs and SER OUT from FF8 Q̄ → inverter on SRCK ↑.
  - The text contradiction is repeated at p12 §7.3.2.
  - DRAIN ON ⇔ G = L ∧ storage bit = 1.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 100 mA; p6): G→DRAIN tPLH 150, tPHL 90; tr 200, tf 200.
- **VCC:** 4.5–5.5 (ROC). **Ratings:** VDS 50 V; 150 mA continuous; 500 mA pulsed/current limit.
- **Obsolete:** TPIC6B595DW, TPIC6B595DWG4, TPIC6B595DWRG4. (TPIC6B595N is NRND.)

## TPIC6C595

- **Datasheets:** tpic6c595 p1,4,5 (pins), 6 (ROC), switching (≈p7), 11 (logic diagram).
- **Families:** TPIC: TPIC6C595*.
- **Pinout** p16, D, N and PW (p4):
  `1 VCC power, 2 SERIN input, 3 DRAIN0, 4 DRAIN1, 5 DRAIN2, 6 DRAIN3, 7 CLR input, 8 G input, 9 SEROUT output,
  10 RCK input, 11 DRAIN4, 12 DRAIN5, 13 DRAIN6, 14 DRAIN7, 15 SRCK input, 16 GND ground`. The clear pin is named **CLR** here.
- **Behavior:**
  - The logic diagram p11 differs from the larger TPIC 595s: **CLR clears both the shift and the storage registers** (the storage FFs
    have CLR). This agrees with the text "A logical low on (CLR) clears all registers".
  - SER OUT is from FF8 on **SRCK ↑** per the diagram.
  - **However, the switching table lists "tpd Propagation delay time, SRCK↓ to SEROUT 15 ns"**. This conflicts with the diagram;
    it looks copied from the 6C596. Recommend following the diagram (rising edge). Flag for the planner.
  - DRAIN ON ⇔ G = L ∧ storage bit = 1.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 75 mA): G→DRAIN tPLH 80, tPHL 50; "SRCK↓→SEROUT" 15; tr 100, tf 80.
- **VCC:** 4.5–5.5. **Ratings:** VDS 33 V; 100 mA continuous; 250 mA pulsed.
- **Obsolete:** TPIC6C595PWG4.

## TPIC6A596

- **Datasheets:** tpic6a596 p1,3–5,12.
- **Families:** TPIC: TPIC6A596*.
- **Pinouts:** NE20 and DW24 are identical to TPIC6A595 (p3–4 pin tables).
- **How the 596 differs from the 595** (logic diagram p12; text p1/§7.3):
  1. **SER OUT is clocked on the falling edge of SRCK.** An extra flip-flop, clocked by inverted SRCK, has D = shift stage 8,
     and its output drives SER OUT. Text: "The serial output (SER OUT) is clocked out of the device on the falling edge of SRCK to
     provide additional hold time for cascaded applications."
  2. **SRCLR clears all registers:** the shift register, the storage register and the SER OUT flip-flop all have CLR in the diagram.
  - Everything else (G, the RCK ↑ storage, DRAIN ON ⇔ G = L ∧ bit = 1) is as in the 595.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 350 mA; p5): G→DRAIN tPHL 30, tPLH 125; **SRCK↓→SEROUT tpd 20**;
  f(SRCK) 10 MHz (cascaded); tr 60, tf 30.
- **VCC:** 4.5–5.5. **Ratings:** VDS 50 V, 350 mA continuous.
- **Obsolete:** TPIC6A596DW.

## TPIC6B596

- **Datasheets:** tpic6b596 p3–5, 11–12.
- **Families:** TPIC: TPIC6B596*.
- **Pinout** p20, DW and N. It is identical to TPIC6B595:
  `1 NC, 2 VCC, 3 SERIN, 4–7 DRAIN0–3, 8 SRCLR, 9 G, 10 GND, 11 GND, 12 RCK, 13 SRCK, 14–17 DRAIN4–7, 18 SEROUT, 19 GND, 20 NC`.
  **The pin table p3 is wrong:** it lists SRCK 15 and SRCLR 3. The logic diagram p11 shows SRCK 13, SRCLR 8, SER IN 3.
- **Behavior:**
  - As TPIC6A596: SER OUT on the SRCK falling edge, and SRCLR clears the shift register, the storage register and the SER OUT FF (diagram p11).
  - The text at p12 says the same.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 100 mA; p5): G→DRAIN tPLH 150, tPHL 90; **SRCK↓→SEROUT 15**; f(SRCK) 10 MHz;
  tr 200, tf 200.
- **VCC:** 4.5–5.5. **Ratings:** VDS 50 V, 150 mA continuous.
- **Obsolete:** TPIC6B596DW, TPIC6B596DWG4.

## TPIC6C596

- **Datasheets:** tpic6c596 p1,4–7,12.
- **Families:** TPIC: TPIC6C596* (including TPIC6C596DRQ1).
- **Pinout** p16, D, N and PW: identical to TPIC6C595 (`7 CLR`, `9 SEROUT`, `15 SRCK`, …).
- **Behavior:** CLR clears all registers, including the SER OUT FF. SER OUT is clocked on **SRCK ↓** (diagram p12; text "clocked out of
  the device on the falling edge of SRCK"). DRAIN ON ⇔ G = L ∧ bit = 1.
- **Timing** (5 V, 25 °C, CL 30 pF, ID 75 mA): G→DRAIN tPLH 80, tPHL 50; **SRCK↓→SEROUT 15**; f(SRCK) 10 MHz;
  tr 100, tf 80.
- **VCC:** 4.5–5.5. **Ratings:** VDS 33 V, 100 mA continuous.
- **Obsolete:** none.

TPIC summary of the SRCLR scope and the SER OUT edge, from the logic diagrams:

| device | clear clears | SER OUT clocked on |
|---|---|---|
| 6595, 6A595, 6B595 | shift register only | SRCK ↑ |
| 6C595 | shift + storage | SRCK ↑ per the diagram; the table says ↓ |
| 6A596, 6B596, 6C596 | shift + storage + SEROUT FF | SRCK ↓ |

---

## 4094B

- **Title:** "CMOS 8-Stage Shift-and-Store Bus Register" (cd4094b p1).
- **Datasheets:** cd4094b p1–4 (scanned).
- **Families:** C: CD4094B*.
- **Military:** 7702501EA = CD4094BF3A (census marks).
- **Pinout** p16, N(E), J(F, F3A), NS, PW (p1, Fig. 1):
  `1 STROBE input, 2 DATA input, 3 CLOCK input, 4 Q1 output, 5 Q2, 6 Q3, 7 Q4 output, 8 VSS ground, 9 QS output,
  10 Q'S output, 11 Q8 output, 12 Q7, 13 Q6, 14 Q5 output, 15 OUTPUT ENABLE input, 16 VDD power`.
  **The names differ from 74_4094** (Q1–Q8 vs Q0–Q7, QS/Q'S vs QS1/QS2, CLOCK vs CP, OUTPUT ENABLE vs OE). Same positions and function.
- **Truth table** (p2), with CL = level change:

  | CL | OE | STROBE | DATA | Q1 | QN | QS | Q'S |
  |---|---|---|---|---|---|---|---|
  | ↑ | 0 | X | X | OC | OC | Q7 | NC |
  | ↓ | 0 | X | X | OC | OC | NC | Q7 |
  | ↑ | 1 | 0 | X | NC | NC | Q7 | NC |
  | ↑ | 1 | 1 | 0 | 0 | QN−1 | Q7 | NC |
  | ↑ | 1 | 1 | 1 | 1 | QN−1 | Q7 | NC |
  | ↓ | 1 | 1 | 1 | NC | NC | NC | Q7 |

  OC = open circuit (Hi-Z). Footnote: "At the positive clock edge the information in the 7th shift register stage is transferred to the 8th
  register stage and the QS output."
- **Behavior** (p1 text and logic diagram):
  - Shift on CLOCK ↑ (stage 1 ← DATA).
  - **STROBE is a level-sensitive transparent latch enable:** while STROBE = H the storage latches follow the shift stages. When STROBE goes low they hold.
    ("The data in each shift register stage is transferred to the storage register when the STROBE input is high.")
  - **OUTPUT ENABLE is active high:** OE = 1 drives Q1–Q8 from the storage latches; OE = 0 sets Q1–Q8 to Hi-Z.
  - **QS** = shift stage 8. It updates on CLOCK ↑ and is not 3-stated.
  - **Q'S** = stage 8 re-latched on CLOCK ↓. It presents the same bit half a clock later and is not 3-stated.
  - Neither serial output is affected by STROBE or OE.
- **Timing** (VDD 5 V, CL 50 pF, typical; p4):
  - CLOCK→QS 300; CLOCK→Q'S 230; CLOCK→Q (parallel) 420; STROBE→Q 290.
  - OE→Q tPHZ/tPZH 140; tPLZ/tPZL 100.
- **VDD:** 3–18 V (p2).
- **Obsolete:** CD4094BPW. (7702501EA and CD4094BF3A are Last.)

## 74_4094

- **Title:** "CDx4HC4094, CD74HCT4094 High-Speed CMOS Logic 8-Stage Shift and Store Bus Register, Three-State" (cd74hc4094 p1).
- **Datasheets:** cd74hc4094 p1,3,4,6,7,10–11.
- **Families:**
  - HC: CD74HC4094*, CD54HC4094*
  - HCT: CD74HCT4094*
- **Military:** none (CD54HC4094F3A is not an SMD number).
- **Pinout** p16, J, N(E), D(M), NS, PW (p3):
  `1 STROBE input, 2 DATA input, 3 CP input, 4 Q0 output, 5 Q1, 6 Q2, 7 Q3 output, 8 GND, 9 QS1 output, 10 QS2 output,
  11 Q7 output, 12 Q6, 13 Q5, 14 Q4 output, 15 OE input, 16 VCC`.
- **Truth table** (p11, Table 7-1):

  | CP | OE | STR | D | Q0 | Qn | QS1 | QS2 |
  |---|---|---|---|---|---|---|---|
  | ↑ | L | X | X | Z | Z | Q6 | NC |
  | ↓ | L | X | X | Z | Z | NC | Q7 |
  | ↑ | H | L | X | NC | NC | Q6 | NC |
  | ↑ | H | H | L | L | Qn−1 | Q6 | NC |
  | ↑ | H | H | H | H | Qn−1 | Q6 | NC |
  | ↓ | H | H | H | NC | NC | NC | Q7 |

  Footnote: "At the positive clock edge the information in the seventh register stage is transferred to the eighth register stage and QS1
  output."
- **Behavior:**
  - Identical to 4094B: STR is a transparent-high latch enable; OE is active **high** (L → Hi-Z).
  - QS1 = stage 8 (Q7) on CP ↑. QS2 = stage 8 re-latched on CP ↓ (p10 text).
- **Timing** (4.5 V, 50 pF; only the 25 °C MAX is printed at 4.5 V; typicals are given at 5 V, 15 pF; p7):
  - HC: CP→QS1 30 (typ 12), CP→QS2 27 (11), CP→Qn 39 (16), STR→Qn 36, OE en 35, OE dis 25 (typ 10).
  - HCT: CP→QS1 39 (16), CP→QS2 36 (15), CP→Qn 43 (18), STR→Qn 39, en 35, dis 35.
  - The HC STR→Qn row is printed under symbol "tt" (a datasheet typo).
- **VCC:** HC 2–6, HCT 4.5–5.5 (p4). VI 0–VCC.
- **Obsolete:** CD74HC4094M, CD74HC4094MT, CD74HC4094PW, CD74HCT4094M.

## 4031B

- **Title:** "CMOS 64-Stage Static Shift Register" (cd4031b p1).
- **Datasheets:** cd4031b p1–4 (scanned).
- **Families:** C: CD4031B*.
- **Military:** none.
- **Pinout** p16, N, J and PW (p1 terminal assignment; NC list "3,4,11,12,13,14"):
  `1 RECIRC input (printed "RECIRCULATE IN / DATA IN 2"), 2 CL input ("CL IN", clock), 3 NC, 4 NC, 5 Q' output,
  6 Q output, 7 QBAR output (Q̄), 8 VSS, 9 CLD output (delayed clock), 10 MODE input ("MODE CONTROL"), 11 NC, 12 NC,
  13 NC, 14 NC, 15 DATA input ("DATA IN"/"DATA 1"), 16 VDD`.
- **Behavior** (p1 tables):
  - Input select (the "Input control circuit truth table"): bit into stage 1 = MODE ? RECIRC : DATA.
    - MODE = 0 → DATA (1 → 1, 0 → 0).
    - MODE = 1 → RECIRC (1 → 1, 0 → 0).
  - Stages: D-type master-slave, Data+1 ← Data on **CL ↑**; CL ↓ → no change.
  - **Q** = stage 64 output. **Q̄** = its complement (pin 7).
  - **Q'** (pin 5) is a "1/2 stage": a D-type master flip-flop that loads stage 64 on **CL ↓** and holds on CL ↑ ("Truth table for
    output from Q' (terminal 5)": Data+64 → Data+64½ on ↓; ↑ → NC). So Q' = Q delayed by half a clock period. The description: "a third cascading
    option makes use of the Q' output from the 1/2 stage, which is available on the next negative-going transition of the clock
    after the Q output occurs".
  - **CLD** (pin 9) is a buffered, delayed copy of CL, in phase (non-inverted). The logic diagram p2 shows CL → buffers → CLD, and the timing
    table lists "Clock to CLD" 100 ns typical at 5 V. Its purpose: "a delayed clock output (CLD) is provided that enables cascading
    register packages while allowing reduced clock drive fan-out and transition-time requirements" (p1). The next package
    is clocked from CLD.
  - The device is fully static.
- **Timing** (VDD 5 V, CL 50 pF, typical; p3):
  - CL→Q̄ tPHL/tPLH 250 and CL→Q tPLH 250.
  - CL→Q' tPHL/tPLH 190 and CL→Q tPHL 190.
  - CL→CLD 100.
  - Q tTHL 50 (the Q output has stronger sink, "TTL drive").
- **VDD:** 3–18 V (p1).
- **Obsolete:** CD4031BPW.

## 4517B

- **Title:** "CMOS Dual 64-Stage Static Shift Register" (cd4517b p1).
- **Datasheets:** cd4517b p1–4 (scanned).
- **Families:** C: CD4517B*.
- **Military:** none.
- **Pinout** p16, N(E) and J(F3A) (p1):
  `1 Q16A inout, 2 Q48A inout, 3 WEA input, 4 CLA input, 5 Q64A output (3-state), 6 Q32A inout, 7 DA input, 8 VSS,
  9 DB input, 10 Q32B inout, 11 Q64B output (3-state), 12 CLB input, 13 WEB input, 14 Q48B inout, 15 Q16B inout, 16 VDD`.
- **Truth table** (per register; p1). The taps are Q16, Q32, Q48 and Q64:

  | Clock | WE | Data | Tap 16 | Tap 32 | Tap 48 | Tap 64 |
  |---|---|---|---|---|---|---|
  | 0 | 0 | X | Q16 | Q32 | Q48 | Q64 |
  | 0 | 1 | X | Z | Z | Z | Z |
  | 1 | 0 | X | Q16 | Q32 | Q48 | Q64 |
  | 1 | 1 | X | Z | Z | Z | Z |
  | ↑ | 0 | DI In | Q16 | Q32 | Q48 | Q64 |
  | ↑ | 1 | DI In | D17 In | D33 In | D49 In | Z |
  | ↓ | 0 | X | Q16 | Q32 | Q48 | Q64 |
  | ↓ | 1 | X | Z | Z | Z | Z |

- **Behavior:**
  - Shift on **CL ↑** (Schmitt-trigger clock input).
  - With **WE = 0**, the four taps are 3-state outputs that are enabled and drive stages 16/32/48/64. Stage 17 ← stage 16, etc.
  - With **WE = 1**, all four taps go Hi-Z. Taps 16/32/48 become **inputs**: on CL ↑, stage 17 ← the tap-16 pin, stage 33 ← the tap-32 pin,
    and stage 49 ← the tap-48 pin (Fig. 1 switch "WE=0 / WE=1" selects the stage-17 D source: internal Q16 vs the pin).
  - Stage 1 always ← D. Tap 64 is output-only (Z when WE = 1).
  - This "allows entry of 64 bits into the register with 16 clock pulses" (p1).
  - WE is level-sensitive: it gates the 3-state buffers asynchronously and selects the mux. Timing gives "write enable-to-clock setup" (−50 typ,
    i.e. WE may change up to 50 ns after the clock) and "release time".
  - The two registers (A and B) are fully independent.
- **Timing** (VDD 5 V, CL 50 pF, typical; p4): CL→tap 16 tPHL/tPLH 200; WE→tap 3-state (tPHZ/tPLZ/tPZH/tPZL) 75.
- **VDD:** 3–18 V (p1).
- **Obsolete:** none (CD4517BF3A is NRND).

## 4034B

- **Title:** "CMOS 8-Stage Static Bidirectional Parallel/Serial Input/Output Bus Register" (cd4034b p1).
- **Datasheets:** cd4034b p1–6 (scanned).
- **Families:** C: CD4034B*.
- **Military:** none.
- **Pinout** p24, J(F3A) and DW(M) (terminal diagram p6):
  `1 B8 inout, 2 B7, 3 B6, 4 B5, 5 B4, 6 B3, 7 B2, 8 B1 inout, 9 AE input ("A" ENABLE), 10 SI input (SERIAL INPUT),
  11 AB input (A/B), 12 VSS, 13 PS input (P/S), 14 AS input (A/S), 15 CLOCK input, 16 A1 inout, 17 A2, 18 A3, 19 A4,
  20 A5, 21 A6, 22 A7, 23 A8 inout, 24 VDD`.
  The names print as "A/B", "P/S", "A/S", "\"A\" ENABLE", "SERIAL INPUT", and the data lines are labelled 1–8 per bus.
- **Register:** 8 static master-slave D stages. Serial data enters stage 1 and shifts toward stage 8 (Fig. 11). Each stage connects to
  line An and line Bn.
- **Complete truth table** (p4, "Truth table for register input-levels and resulting register operation"):

  | AE | P/S | A/B | A/S | Operation |
  |---|---|---|---|---|
  | 0 | 0 | 0 | X | Serial mode; synch. serial data input; "A" parallel data outputs disabled |
  | 0 | 0 | 1 | X | Serial mode; synch. serial data input; "B" parallel data output |
  | 0 | 1 | 0 | 0 | Parallel mode; "B" synch. parallel data inputs; "A" parallel data outputs disabled |
  | 0 | 1 | 0 | 1 | Parallel mode; "B" asynch. parallel data inputs; "A" parallel data outputs disabled |
  | 0 | 1 | 1 | 0 | Parallel mode; "A" parallel data inputs disabled; "B" parallel data outputs; synch. data recirculation |
  | 0 | 1 | 1 | 1 | Parallel mode; "A" parallel data inputs disabled; "B" parallel data outputs; asynch. data recirculation |
  | 1 | 0 | 0 | X | Serial mode; synch. serial data input; "A" parallel data output |
  | 1 | 0 | 1 | X | Serial mode; synch. serial data input; "B" parallel data output |
  | 1 | 1 | 0 | 0 | Parallel mode; "B" synch. parallel data input; "A" parallel data output |
  | 1 | 1 | 0 | 1 | Parallel mode; "B" asynch. parallel data input; "A" parallel data output |
  | 1 | 1 | 1 | 0 | Parallel mode; "A" synch. parallel data input; "B" parallel data output |
  | 1 | 1 | 1 | 1 | Parallel mode; "A" asynch. parallel data input; "B" parallel data output |

  Footnote: "*Outputs change at positive transition of clock in the serial mode and when the A/S control input is 'low' in the
  parallel mode. During transfer from parallel to serial operation A/S should remain low in order to prevent DS transfer into
  Flip Flops."
- **Behavior** (p1 text):
  - **Direction is set by A/B.** A/B = 1: A lines are inputs and B lines are outputs. A/B = 0: B lines are inputs and A lines are outputs.
  - **AE gates only the A side.** AE = 0 disables the A lines, both as inputs (a 3-state input is ignored) and as outputs (Hi-Z). In parallel mode with A/B = 1
    and AE = 0, the register reloads its own contents, which is the "recirculation".
  - The B lines are always enabled in their selected role.
  - **P/S = 0 (serial):** on CLOCK ↑, stage 1 ← SI and stage n ← stage n−1. A/S is "internally disabled" ("asynchronous serial operation is
    not allowed"). The data appears on the B lines (A/B = 1) or on the A lines (A/B = 0 and AE = 1).
  - **P/S = 1 (parallel):** with A/S = 0, the register loads the selected input bus on CLOCK ↑. With A/S = 1 the transfer is
    "independent of the clock": it is asynchronous and level-transparent while A/S = 1.
  - Model: the output bus pins are 3-state outputs driven from the register; the input bus pins are high-impedance inputs.
- **Timing** (VDD 5 V, CL 50 pF, typical; p5):
  - A(B) parallel data in → B(A) parallel out, and serial → parallel out: tPHL/tPLH 350.
  - 3-state A/B or AE → A out: tPLZ/tPHZ/tPZL/tPZH 200.
  - Clock max 4 MHz typ.
- **VDD:** 3–18 V (p2).
- **Obsolete:** none (CD4034BF3A is Last).

## 40105B

- **Title:** "CMOS FIFO Register, 4 Bits × 16 Words" (cd40105b p1).
- **Datasheets:** cd40105b p1–5 (scanned).
- **Families:** C: CD40105B*.
- **Military:** none.
- **Pinout** p16, N(E) and J(F, F3A) (p4 terminal assignment):
  `1 3-STATE CONTROL input (suggest OE), 2 DIR output (DATA-IN READY), 3 SI input (SHIFT IN), 4 D0 input, 5 D1, 6 D2,
  7 D3 input, 8 VSS, 9 MR input (MASTER RESET), 10 Q3 output (3-state), 11 Q2, 12 Q1, 13 Q0 output, 14 DOR output
  (DATA-OUT READY), 15 SO input (SHIFT OUT), 16 VDD`.
- **Behavior** (p1 text, quoted where exact):
  - **Structure.** "Each word position in the register is clocked by a control flip-flop, which stores a marker bit. A '1' signifies that
    the position's data is filled and a '0' denotes a vacancy." An empty position that sees a full predecessor "generates a clock pulse that
    transfers data from the preceding four data latches into its own four data latches and resets the preceding flip-flop to '0'".
    Empty positions bubble to the input; valid data ripples (falls through) to the output.
  - **DIR** = status of the first control FF, inverted: DIR = H when position 1 is empty, i.e. ready. **DOR** = status of the last control FF:
    DOR = H when position 16 holds valid data.
  - **Loading:**
    - "Data can be entered whenever the DATA-IN READY (DIR) flag is high, by a low to high transition on the SHIFT-IN (SI) input.
      This input must go low momentarily before the next word is accepted by the FIFO."
    - "The DIR flag will go low momentarily, until the data have been transferred to the second location. The flag will remain low
      when all 16-word locations are filled with valid data, and further pulses on the SI input will be ignored until DIR goes high."
    - So: SI ↑ while DIR = H → D0–D3 is captured into position 1 and DIR goes L (tPHL 160 typ).
    - DIR returns H once the word has moved to position 2 (DIR low pulse width 260 typ), unless the FIFO is now full.
  - **Unloading:**
    - "As soon as the first word has rippled to the output, DATA-OUT READY (DOR) goes high, and data can be removed by a falling edge on the
      SO input. This falling edge causes the DOR signal to go low while the word on the output is dumped and the next word moves to the
      output. As long as valid data are available in the FIFO, the DOR signal will go high again signifying that the next word is
      ready at the output. When the FIFO is empty, DOR will remain low, and any further commands will be ignored until a '1' marker
      ripples down to the last control register."
    - So: SO ↓ while DOR = H → DOR goes L (tPHL 185 typ). Q0–Q3 change to the next word (SO→Qn 210 typ). DOR returns H when the next word
      arrives (DOR low pulse 220 typ).
  - **Fall-through.** A word written into an empty FIFO reaches the output after the "ripple-through delay" (input to output, tPLH) of
    **2 µs typ (4 µs max) at 5 V** (p4). The timing diagram p5 says "≈2 µs at VDD = 5 V — ripple time from position 1 to position 16",
    and likewise for the bubble from 16 to 1.
  - **3-state control (pin 1):**
    - High → Q0–Q3 are Hi-Z. Note 1 (p4): "The Output Enable Line (Pin 1) should be low for limits specified".
    - "Unloading of data is inhibited while the 3-state control input is high."
    - "The 3-state control signal should not be shifted from high to low (data outputs turned on) while the SHIFT-OUT is at logic 0. This
      level change would cause the first word to be shifted out (unloaded) immediately and the data to be lost."
    - So, in the 40105B, the effective unload strobe is the falling edge of (SO OR pin-1).
  - **MR (active high, asynchronous):**
    - "A high on the MASTER RESET (MR) sets all the control logic marker bits to '0'. DOR goes low and DIR goes high. The contents of the data
      register are not changed, only declared invalid, and will be superseded when the first word is loaded. The shift-in must be low
      during Master Reset."
    - In cascade, MR must be pulsed after power-up.
  - **Cascade:** DIR → SO of the downstream part, DOR → SI of the next part. For width expansion, AND the flags.
  - "DATA VALID goes to high level in advance of the DATA OUT by a maximum of 50 ns at VDD = 5 V" (Fig. 14 note).
- **Timing** (VDD 5 V, CL 50 pF, typical; p4):
  - SO or MR → DOR tPHL 185; SI → DIR tPHL 160; SO → Qn 210.
  - 3-state control → Qn: tPZH/tPZL 140, tPHZ/tPLZ 100.
  - Ripple-through 2000.
  - Min SI pulse 100; min SO pulse 90; DIR pulse width 260; DOR pulse width 220.
  - Data setup 0; data hold 175; max shift rate 3 MHz typ.
- **VDD:** 3–18 V (p2).
- **Obsolete:** none.

## 74_40105

- **Title:** "High-Speed CMOS Logic 4-Bit x 16-Word FIFO Register" (cd74hc40105 p1).
- **Datasheets:** cd74hc40105 p1–4, 7–9.
- **Families:**
  - HC: CD74HC40105*, CD54HC40105*
  - HCT: CD74HCT40105* (the CD54HCT40105 is printed but has no census row)
- **Military:** none.
- **Pinout** p16, J, N(E) and D(M) (p2):
  `1 OE input (printed "THREE-STATE CONTROL"; "OE" in the logic diagram), 2 DIR output, 3 SI input, 4 D0, 5 D1, 6 D2, 7 D3,
  8 GND, 9 MR input, 10 Q3 output, 11 Q2, 12 Q1, 13 Q0 output, 14 DOR output, 15 SO input, 16 VCC`.
- **Behavior:** same as 40105B, with these documented differences (p2):
  - "the shift-out signal, unlike that in the CD40105B, is independent of the three-state output control … In the high-speed
    CMOS version this restriction has been eliminated." So unloading is SO ↓ only, regardless of OE.
  - OE = H → Q0–Q3 Hi-Z.
  - "If during unloading SI is HIGH (FIFO is full) data on the data input of the FIFO is entered in the first location." When position 1
    frees up with SI already high, the word is loaded without a new SI edge.
  - "If the shift-in flag (SI) is HIGH during the master reset pulse, data present at the input (D0 to D3) are immediately moved into the
    first location upon completion of the reset process." (The 40105B instead requires SI low during MR.)
  - Logic diagram p4: per-position SR control flip-flops ("S overrides R" for FF1, "R overrides S" for FF2–16) and latches L1–L16. MR
    resets all the markers.
- **Timing** (HC, 4.5 V, 50 pF; only the 25 °C MAX is printed; typicals are at 5 V/15 pF; p8):
  - MR→DIR, DOR 35 (typ 15); SI→DIR 42 (18); SO→DOR 42 (18); SO→Qn 80 (35).
  - Ripple-through: SI→DOR 400, SO→DIR 500, SI→Qn 300.
  - OE→Qn: en 30, dis 28.
  - HCT (p9): MR→DIR/DOR 36; SI→DIR 42; SO→DOR 42; SO→Qn 80; SI→DOR 400; SO→DIR 500; SI→Qn 300; en 35; dis 30.
  - Prerequisites (HC 4.5 V): SI pulse 16, SO pulse 24, DIR/DOR pulse widths ≥ 40, MR pulse 24, MR→SI removal 10,
    Dn→SI setup 5, hold 25 (p7).
- **VCC:** HC 2–6, HCT 4.5–5.5 (p5). VI 0–VCC.
- **Obsolete:** CD74HC40105M.

## 74_8153

- **Title:** "SN74LV8153 Serial-to-Parallel Interface" (sn74lv8153 p1).
- **Datasheets:** sn74lv8153 p1–7; sn74lv8153-q1 (same content; TA −40…125 °C; slightly different OE/RESET max).
- **Families:** LV: SN74LV8153*, SN74LV8153Q*-Q1.
- **Military:** none.
- **Pinout** p20, N and PW (p1):
  `1 VCC1 power, 2 A0 input, 3 A1 input, 4 A2 input, 5 D input, 6 OUTSEL input, 7 RESET input, 8 OE input, 9 SOUT output,
  10 GND ground, 11 GND ground, 12 Y7 output, 13 Y6, 14 Y5, 15 Y4, 16 Y3, 17 Y2, 18 Y1, 19 Y0 output, 20 VCC2 power`.
  **This part has two supplies:** VCC1 feeds the logic and SOUT; VCC2 feeds Y0–Y7.
- **Serial frame** (p3, "data transmission protocol"). The sequence is "2START-3ADDRESS-4DATA-1STOP", LSB first. One byte needs two frames:

  | bit # | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
  |---|---|---|---|---|---|---|---|---|---|---|
  | frame 1 | ST0 = 0 | ST1 = 1 | A0 | A1 | A2 | D0 | D1 | D2 | D3 | SP = 1 |
  | frame 2 | ST0 = 0 | ST1 = 1 | A0 | A1 | A2 | D4 | D5 | D6 | D7 | SP = 1 |

  - Idle is high (UART-compatible). This is exactly one UART 8N1 character: the UART start bit is ST0, and data bit 0 = 1 (ST1), bits 1–3 = address,
    bits 4–7 = the nibble, then the stop bit. That is my inference from "Compatible With UART Serial-Data Format".
  - "The order of the two start bits must be 0, then 1 in any frame; otherwise, the data rate will not be detected correctly."
  - **Bit timing is auto-baud, per frame.** "The period between the falling edge of the first start bit (ST0) and the rising edge of the second
    start bit (ST1) is measured to generate an internal-clock synchronized data stream." That is one bit period, Pw. An internal oscillator plus counter
    measures it ("Internal Oscillator and Counter for Automatic Data-Rate Detection", p1).
  - Bits are sampled at mid-bit ("the internal clock is synchronized at the middle of the data pulse", p6 note 1). The timing chart p3 shows 8 internal
    clock pulses per frame, at ST1 and A0…D3.
  - Data rate is 2–24 kbps (p6).
  - **Address:** the A0–A2 bits must match the device's A0–A2 pins. This is implied by "Up to Eight Devices … Can Share the Same Bus by Using Different
    Combinations of A0, A1, A2" and the logic diagram's "Address Verification" block. **"The three address bits (in the consecutive frame) must
    be the same as those in the first frame; otherwise, the data will be dropped."**
  - The datasheet does not state how the part tells frame 1 from frame 2 (presumably alternation since RESET), or what happens after a dropped pair.
    Flag as ambiguous.
- **What latches when:**
  - Frame 1's nibble goes into a 4-bit register.
  - After frame 2, the full byte is transferred to the 8-bit output register, and Y0–Y7 change Pw/2 after the start of the D7 bit, i.e. at
    D7's sample point ("tpd D7→Y = Pw/2").
  - **SOUT** is push-pull, supplied by VCC1, with IOH/IOL specified; it is not open-drain. It "goes low when the data is received correctly and
    maintains a low level for one data-pulse period" (tw = Pw), starting Pw/2 after D7 begins (tpd D7→SOUT = Pw/2). Pin table: "Outputs a pulse
    when latch data is changed".
- **Outputs** (function table p1, "each buffer"; Dn is the latched data bit):

  | OUTSEL | RESET | OE | Dn | Yn | structure |
  |---|---|---|---|---|---|
  | L | H | L | H | L | open collector |
  | L | H | L | L | H (off) | open collector |
  | L | X | H | X | H (off) | open collector |
  | L | L | X | X | H (off) | open collector |
  | H | H | L | H | H | push-pull |
  | H | H | L | L | L | push-pull |
  | H | X | H | X | Z | push-pull |
  | H | L | L | X | L | push-pull |

  - "In the open-collector mode (OUTSEL = L), the outputs are inverted, e.g., Y1 = L when D1 = H."
  - OC mode: Yn sinks ⇔ OE = L ∧ RESET = H ∧ Dn = 1.
  - PP mode: Yn = Dn when OE = L. RESET = L clears the register, so Y = L.
  - **RESET is active low** and clears the data register; tpd RESET→Y is ≤ 200 ns.
  - **OE is active high-disable.** OE = H puts the push-pull outputs in Hi-Z and the open-collector outputs off. The datasheet advises a pull-up on OE for power-up.
- **Timing** (25 °C max; no typicals):
  - Non-Q1 (p6–7): RESET→Y 200 (3.3 V) / 150 (5 V); OE→Y (OC) 200/150; PP ten/tdis 200/150; D7→Y and D7→SOUT Pw/2.
  - Q1: 220 at 3.3 V, 200 at 5 V.
- **VCC:** VCC1 3–5.5 V, VCC2 3–13.2 V; inputs 0–5.5 V (inputMax 5.5) (p5). All inputs are Schmitt triggers (VT+/VT− in the electrical
  table). Y is OC 40 mA at VCC2 = 4.5 V; push-pull −24 mA at VCC2 = 12 V.
- **Obsolete:** SN74LV8153PW.

---

## Cross-device notes / datasheet errors found

1. sn74hc595 p3 (FK pin table): RCLK is printed as 14 (it is 15), NC is printed as "1,16,11,16" (it is 1,6,11,16), and the VCC DIP pin is missing (16).
2. sn74hc594 p3 pin table: pin 13 is named "RCLK / Storage clear" (it is RCLR); QH' is described as "QH inverted" (the logic diagram shows it non-inverted).
3. tpic6b595 p3: SRCK is printed as 15 (it is 13). tpic6b596 pin table: SRCK 15 / SRCLR 3 (they are 13 / 8).
4. In TPIC6595/6A595/6B595, the §7.3.2 text says SRCLR "clears all registers", but the logic diagrams clear the shift register only. The 6C595 and all the 596s do
   clear all registers.
5. TPIC6C595 lists "SRCK↓ to SEROUT 15 ns" while its logic diagram clocks SER OUT on SRCK ↑.
6. The LV594A switching table text is mis-aligned in pdftotext; I verified it against the AHC594 numbers.
7. cd74hc4094 p7: the HC STR→Qn row is printed under symbol "tt".
8. The LS597/LS598 have no function table; the DS→SER0/SER1 polarity was read from the gate drawing (DS = L → SER0).
9. CD74HC597/HCT597 VCC: the Operating Conditions list only "HC Types 2V to 6V"; the HCT 4.5–5.5 V comes from the features list.
