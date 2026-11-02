# latchB — non-octal latches (plan-10 extraction)

## PDF → devices

| PDF (datasheets/…) | Devices |
|---|---|
| latch/sn74ls75.pdf | 74_75 (LS; PDF also covers TTL 7475 and the 5477/LS77 variant — no census rows for those) |
| latch/cd74hc75.pdf | 74_75 (HC, HCT — same pin functions, different pin names; see section) |
| latch/sn74ls375.pdf | 74_375 (LS) |
| latch/sn74ls279a.pdf | 74_279 (LS) |
| latch/sn74ls259b.pdf | 74_259 (LS; also covers TTL SN54259/SN74259, no census rows) |
| latch/sn74hc259.pdf | 74_259 (HC) |
| latch/cd74hc259.pdf | 74_259 (HC, HCT) |
| latch/sn74hcs259.pdf | 74_259 (HCS) |
| latch/cd4099b.pdf | 4099B |
| latch/cd4724b.pdf | 4724B |
| latch/cd4042b.pdf | 4042B |
| latch/cd4043b.pdf | 4043B (CD4043B* rows), 4044B (CD4044B* rows) |
| latch/cd4508b.pdf | 4508B |
| latch/cd74hc4511.pdf | 74_4511 (HC, HCT) |
| latch/sn74alvch16260.pdf | 74_16260 (ALVC, bus-hold ALVCH) |
| latch/tpic6259.pdf | TPIC6259 |
| latch/tpic6a259.pdf | TPIC6A259 |
| latch/tpic6b259.pdf | TPIC6B259 |
| latch/tpic6273.pdf | TPIC6273 |
| latch/tpic6b273.pdf | TPIC6B273 |

PDFs with census rows that could not be mapped: **none**. Military rows: 8551901EA, 85519012A, JM38510/65402BEA, M38510/65402BEA → 74_259 HC; 5962-8985201EA → 74_259 HCT; JM38510/17601BEA, M38510/17601BEA → 4099B; 5962-8773301EA → 74_4511 HC; 8407001EA → 74_75 HC; 5962-9075801MEA → 74_75 HCT; 7601201EA, 7601201FA, JM38510/31601BEA, M38510/31601BEA → 74_75 LS; 76018012A, 7601801EA, 7601801FA → 74_279 LS.

Scanned (image-only) datasheet pages were read from rendered images: cd4042b, cd4043b, cd4099b, cd4508b, cd4724b, sn74ls259b, sn74ls279a, sn74ls75.


## 74_75
- Title / function: "4-BIT BISTABLE LATCHES" (SN54LS75/SN74LS75, SDLS120); "Dual 2-Bit Bistable Transparent Latch" (CD54/74HC75, CD54/74HCT75, SCHS135F). Four transparent D latches with true and complementary outputs; the latches are gated in pairs by two shared enables.
- Datasheets: `datasheets/latch/sn74ls75.pdf` (scanned; pp. 1–5 used: p1 pinout/function table/description, p2 logic diagram, p3 standard-TTL ROC/switching, p4 LS ROC/switching, p5 waveforms); `datasheets/latch/cd74hc75.pdf` (text; pp. 1–6 used: p1 pinout/description, p2 truth table/logic diagram, p3 operating conditions, p4–6 timing).
- Families & part-number prefixes:
  - LS: SN74LS75*, SN54LS75*, SNJ54LS75*
  - HC: CD74HC75*, CD54HC75*
  - HCT: CD74HCT75*, CD54HCT75*
  - TTL: SN7475/SN5475 are in the PDF but have no census rows. The PDF also covers SN5477/SN54LS77, a different 14-pin W-package variant with Q outputs only and no QBAR (pp. 1–2, 4). It has no orderable rows in the census, so it is not part of this device.
- Military numbers:
  - sn74ls75.pdf: all rows are LS75. 7601201EA = SNJ54LS75J; 7601201FA = SNJ54LS75W; JM38510/31601BEA and M38510/31601BEA are CDIP J (mark JM38510/31601BEA).
  - cd74hc75.pdf: 8407001EA = CD54HC75F3A (HC); 5962-9075801MEA = CD54HCT75F3A (HCT).
- Pinouts (both are 16-pin; the pin functions match position for position, but the PIN NAMES DIFFER between the TI-LS and Harris-HC sheets):
  - LS p16: J, W (SN54LS75 W is the 16-pin '75 layout per p1 header "SN5475, SN54LS75 … J OR W PACKAGE"), N, D, NS (NS has a census row but is not named on p1; assumed same). Printed names (p1):
    1 1QBAR output · 2 1D input · 3 2D input · 4 "3C, 4C" input (suggest `3C4C`) · 5 VCC power · 6 3D input · 7 4D input · 8 4QBAR output · 9 4Q output · 10 3Q output · 11 3QBAR output · 12 GND ground · 13 "1C, 2C" input (suggest `1C2C`) · 14 2QBAR output · 15 2Q output · 16 1Q output
  - HC/HCT p16: J (CERDIP), N (PDIP), D (SOIC), PW (TSSOP); NS appears in the ordering table but has no census row. Printed names (p1):
    1 1Q0BAR output · 2 1D0 input · 3 1D1 input · 4 2E input (printed with an overbar) · 5 VCC power · 6 2D0 input · 7 2D1 input · 8 2Q1BAR output · 9 2Q1 output · 10 2Q0 output · 11 2Q0BAR output · 12 GND ground · 13 1E input (printed with an overbar) · 14 1Q1BAR output · 15 1Q1 output · 16 1Q0 output
  - Name correspondence: 1D↔1D0, 2D↔1D1, 3D↔2D0, 4D↔2D1, 1Q↔1Q0, 2Q↔1Q1, 3Q↔2Q0, 4Q↔2Q1 (same for the BAR outputs), "1C,2C"↔1E (pin 13), "3C,4C"↔2E (pin 4).
  - Note: VCC is pin 5 and GND is pin 12 (not the corner pins).
- Behavior:
  - Function table (each latch; LS p1 and HC p2 are the same):

    | D | C (HC: E) | Q | QBAR |
    |---|---|---|---|
    | L | H | L | H |
    | H | H | H | L |
    | X | L | Q0 | Q0BAR |

    LS defines Q0 as "the level of Q before the high-to-low transition of G". HC defines Q0 as "the level of Q before the transition of E".
  - Enable is level-sensitive, asynchronous and transparent while the pin is HIGH. While it is high, Q = D and QBAR = NOT D. On the high→low edge the latch keeps D (subject to setup/hold); while it is low the outputs hold. There is no clock edge, clear or OE.
  - Which enable controls which latches:
    - Pin 13 ("1C,2C" / 1E) gates latches 1 and 2 (LS 1D,2D → 1Q,2Q; HC 1D0,1D1).
    - Pin 4 ("3C,4C" / 2E) gates latches 3 and 4 (LS 3D,4D; HC 2D0,2D1).
  - QBAR is always the complement of Q, with no undefined state.
    - LS logic (p2): QBAR = NOT(D·C + Q·NOT C); Q = NOT QBAR.
    - HC logic (p2): the E pin goes through an inverter to LE̅ and through a second inverter to LE; the latch is transparent when LE = 1, i.e. when the pin is high.
  - HC datasheet inconsistency (p1): the description says the enables "are active LOW", and the pins are drawn with overbars. The same paragraph says "When the Enable input is HIGH data enters the latch", and the truth table and logic diagram (p2) agree: pin HIGH = transparent. Model it as pin-high-transparent, the same as the LS75. Name the pins `1E`/`2E` with no BAR suffix.
  - Power-up state is not stated.
- Timing:
  - LS (p4, VCC = 5 V, TA = 25 °C, CL = 15 pF, RL = 2 kΩ; there is NO 50 pF condition, so these are typ/max at 15 pF), tPLH/tPHL:

    | Path | typ (ns) | max (ns) |
    |---|---|---|
    | D→Q | 15/9 | 27/17 |
    | D→QBAR | 12/7 | 20/15 |
    | C→Q | 15/14 | 27/25 |
    | C→QBAR | 16/7 | 30/15 |

    The p4 footnote prints "tPLH" twice; the second should read tPHL. Also on p4: tw(enable) ≥ 20 ns, tsu 20 ns, th 5 ns (both SN54 and SN74).
  - (TTL '75, for reference only, p3, CL = 15 pF, RL = 400 Ω, typ: D→Q 16/14, D→QBAR 24/7, C→Q 16/7, C→QBAR 16/7.)
  - HC (p5): no typical is printed at 50 pF, so MAX at VCC = 4.5 V, CL = 50 pF, 25 °C (tPLH = tPHL):

    | Path | max (ns) | typ at 5 V, CL = 15 pF (ns) |
    |---|---|---|
    | D→Q | 22 | 9 |
    | D→QBAR | 26 | 10 |
    | E→Q | 26 | 10 |
    | E→QBAR | 26 | 11 |

    Also: tw(E) 16 ns, tsu 12 ns, th 3 ns at 4.5 V (pp. 4–5).
  - HCT (pp. 5–6), MAX at 4.5 V, 50 pF: D→Q 28, D→QBAR 28, E→Q 28, E→QBAR 30. Typical at 5 V, 15 pF: 11, 11, 11, 12. Also: tw 16, tsu 12, th 3 ns (p5).
  - pdftotext drops the overbars, so the Q/QBAR row labels were checked against page images.
- VCC:
  - LS: SN54LS75 4.5–5.5 V; SN74LS75 4.75–5.25 V (p4).
  - HC: 2–6 V. HCT: 4.5–5.5 V. HC/HCT inputs 0 to VCC, so no inputMax (p3).
  - LS absolute-max input is 7 V (p1). That is an absolute rating, not a recommended input max.
- Obsolete:
  - sn74ls75: SN74LS75D (SN74LS75NSR is Last Time Buy).
  - cd74hc75: CD74HC75M, CD74HC75MT, CD74HC75PW, CD74HC75PWT.
- Notes:
  - The '77 (14-pin W: 1 1D, 2 2D, 3 "3C,4C", 4 VCC, 5 3D, 6 4D, 7 NC, 8 4Q, 9 3Q, 10 NC, 11 GND, 12 "1C,2C", 13 2Q, 14 1Q) is the same function without QBAR. It has no census rows, so skip it.
  - The 74_375 is functionally identical with a different pin arrangement; see below.

## 74_375
- Title / function: "4-BIT BISTABLE LATCHES" (SN54LS375, SN74LS375, SDLS166). Description (p1): "electrically and functionally identical to the SN54LS75 and SN74LS75, respectively. Only the arrangement of the terminals has been changed", with "Supply Voltage and Ground on Corner Pins".
- Datasheets: `datasheets/latch/sn74ls375.pdf` (scanned; p1 pinouts/function table/logic diagram, p2 ROC/electrical/switching).
- Families & part-number prefixes: LS: SN74LS375*, SN54LS375*, SNJ54LS375*.
- Military numbers: none in the census (all rows are commercial or SNJ54).
- Pinouts:
  - p16: J, W, N, D (p1):
    1 1D input · 2 1QBAR output · 3 1Q output · 4 "1C,2C" input (suggest `1C2C`) · 5 2Q output · 6 2QBAR output · 7 2D input · 8 GND ground · 9 3D input · 10 3QBAR output · 11 3Q output · 12 "3C,4C" input (suggest `3C4C`) · 13 4Q output · 14 4QBAR output · 15 4D input · 16 VCC power
  - The overbar placement was confirmed by the logic symbol (p1): 1Q (3), 1Q̄ (2), 2Q (5), 2Q̄ (6), 3Q (11), 3Q̄ (10), 4Q (13), 4Q̄ (14).
  - FK (SN54LS375 LCCC-20, p1; there are no FK census rows):
    1 NC · 2 1D · 3 1QBAR · 4 1Q · 5 "1C,2C" · 6 NC · 7 2Q · 8 2QBAR · 9 2D · 10 GND · 11 NC · 12 3D · 13 3QBAR · 14 3Q · 15 "3C,4C" · 16 NC · 17 4Q · 18 4QBAR · 19 4D · 20 VCC
- Behavior:
  - Same as 74_75. Function table (each latch, p1):

    | D | G | Q | QBAR |
    |---|---|---|---|
    | L | H | L | H |
    | H | H | H | L |
    | X | L | Q0 | Q0BAR |

    The column is headed "G", while the pins are named C. Q0 is "the level of Q before the high-to-low transition of C".
  - Enable is level-sensitive and transparent while HIGH; outputs hold while it is LOW.
  - Pin 4 ("1C,2C") gates latches 1 and 2; pin 12 ("3C,4C") gates latches 3 and 4.
  - Logic (p1) is the same AND-OR-invert as the LS75: QBAR = NOT(D·C + Q·NOT C); Q = NOT QBAR.
- Timing (p2, VCC = 5 V, 25 °C, CL = 15 pF, RL = 2 kΩ; no 50 pF condition), typ/max tPLH/tPHL. The numbers are identical to the LS75.

  | Path | typ (ns) | max (ns) |
  |---|---|---|
  | D→Q | 15/9 | 27/17 |
  | D→QBAR | 12/7 | 20/15 |
  | C→Q | 15/14 | 27/25 |
  | C→QBAR | 16/7 | 30/15 |

  Also: tw 20 ns, tsetup 20 ns, thold **0 ns** (the LS75 sheet says 5 ns).
- VCC: SN54LS375 4.5–5.5 V; SN74LS375 4.75–5.25 V (p2). Absolute-max input is 7 V.
- Obsolete: none (SN74LS375D is Last Time Buy).
- Notes: The hold time differs from the LS75 sheet (0 vs 5 ns), even though the parts are stated to be identical.

## 74_279
- Title / function: "QUADRUPLE S̄-R̄ LATCHES" (SN54279, SN54LS279A, SN74279, SN74LS279A, SDLS093). Four active-low set/reset latches. Latches 1 and 3 have two S̄ inputs; latches 2 and 4 have one. There is only a Q output (no QBAR).
- Datasheets: `datasheets/latch/sn74ls279a.pdf` (scanned; p1 pinout/function table/logic diagram, p2 abs max, p3 '279 ROC/switching, p4 'LS279A ROC/switching).
- Families & part-number prefixes:
  - LS: SN74LS279A*, SN54LS279A*, SNJ54LS279A*
  - TTL: SN74279/SN54279 are in the PDF but have no census rows.
- Military numbers: all rows are LS279A. 7601801EA = SNJ54LS279AJ; 7601801FA = SNJ54LS279AW; 76018012A = SNJ54LS279AFK.
- Pinouts:
  - p16: J, W, N, D, NS (NS has a census row but is not named on p1; assumed same) (p1):
    1 1R input · 2 1S1 input · 3 1S2 input · 4 1Q output · 5 2R input · 6 2S input · 7 2Q output · 8 GND ground · 9 3Q output · 10 3R input · 11 3S1 input · 12 3S2 input · 13 4Q output · 14 4R input · 15 4S input · 16 VCC power
    (All R/S inputs are printed with overbars, i.e. active low.)
  - FK (SNJ54LS279AFK, LCCC-20, p1):
    1 NC · 2 1R · 3 1S1 · 4 1S2 · 5 1Q · 6 NC · 7 2R · 8 2S · 9 2Q · 10 GND · 11 NC · 12 3Q · 13 3R · 14 3S1 · 15 3S2 · 16 NC · 17 4Q · 18 4R · 19 4S · 20 VCC
- Behavior:
  - Logic (p1) is cross-coupled NANDs. Latches 1 and 3: Q = NAND(S1, S2, n), where n = NAND(R, Q). Latches 2 and 4: Q = NAND(S, n), where n = NAND(R, Q). Fully asynchronous; there is no clock.
  - Function table (each latch, p1). For double-S latches, "S̄" = H means both S̄ inputs are high, and "S̄" = L means one or both S̄ inputs are low.

    | S̄ | R̄ | Q |
    |---|---|---|
    | H | H | Q0 (hold: the level of Q before the input conditions were established) |
    | L | H | H (set) |
    | H | L | L (reset) |
    | L | L | H‡ |

    Footnote ‡: "This configuration is nonstable: that is, it may not persist when the S̄ and R̄ inputs return to their inactive (high) level."
  - The description (p1) says: "Normally, the S̄-R̄ inputs should not be taken low simultaneously. The Q output will be unpredictable in this condition."
  - Exact model per the gates:
    - While S̄(any) = L and R̄ = L, Q = H and the internal n = H.
    - If S̄ returns high first, with R̄ still low, Q goes L (the reset wins). This is the S̄→Q tPHL path below.
    - If R̄ returns high first, Q stays H (set).
    - If both return high simultaneously, the result is undefined (race). Choose a deterministic or random rule.
  - Set priority: while both are low, Q is forced high.
  - Power-up state is not stated.
- Timing:
  - LS (p4, VCC = 5 V, 25 °C, CL = 15 pF, RL = 2 kΩ; no 50 pF condition), typ/max:

    | Path | typ (ns) | max (ns) |
    |---|---|---|
    | S̄→Q tPLH | 12 | 22 |
    | S̄→Q tPHL | 13 | 21 |
    | R̄→Q tPHL | 15 | 27 |

    The S̄→Q tPHL can only occur when R̄ is low while S̄ rises. There is no R̄→Q tPLH.
  - Minimum low pulse duration: 20 ns.
  - (TTL '279, p3, CL = 15 pF, RL = 400 Ω, typ/max: S̄→Q tPLH 12/22, tPHL 9/15; R̄→Q tPHL 15/27.)
- VCC: SN54LS279A 4.5–5.5 V; SN74LS279A 4.75–5.25 V (p4). Absolute-max input is 7 V (p2).
- Obsolete: SN74LS279AD.
- Notes: The pin names repeat across latches, e.g. 2S and 4S have no digit suffix, while latches 1 and 3 use S1/S2.


## 74_259

- **Title / function:** "8-Bit Addressable Latches" (SN54259/SN74259/SN54LS259B/SN74LS259B, SN54HC259/SN74HC259); "CDx4HC(T)259 High-Speed SMOS Logic 8-Bit Addressable Latch"; "SN74HCS259 8-Bit Addressable Latches with Schmitt-Trigger Inputs". 8 D-latches with a 3-to-8 address decoder, a common data input D, an active-low enable G, and an active-low clear CLR. It works as an addressable latch, an active-high 1-of-8 demultiplexer/decoder, or a clear.
- **Datasheets:**
  - `datasheets/latch/sn74ls259b.pdf`: SDLS086, Mar 1988, scanned. Used p1 (pinouts, description), p2 (function table, abs max), p3 (SN54259/SN74259 ROC and switching), p4 (SN54LS259B/SN74LS259B ROC and switching). The PDF also covers standard-TTL SN54259/SN74259.
  - `datasheets/latch/sn74hc259.pdf`: SCLS134F, Mar 2022. Used p1, p3 (pinouts as an image, rendered), p4 (ROC), p5 (timing requirements), p6 (switching), p8 (overview), p9 (per-latch logic and function table).
  - `datasheets/latch/cd74hc259.pdf`: SCHS173D, Nov 2021. Used p1, p3 (pinout), p4 (ROC), p6 (timing prerequisites), p7 (switching), p10–11 (modes and function table).
  - `datasheets/latch/sn74hcs259.pdf`: SCLS836, Sep 2020. Used p3 (pinout and pin table), p4 (ROC), p5 (timing), p6 (switching), p9 (modes and block diagram), p11 (function table).
- **Families & part-number prefixes:**
  - TTL: SN54259*, SN74259*. In the LS PDF only; no orderable rows in the census.
  - LS: SN54LS259B*, SN74LS259B*. Census has SN74LS259B rows only.
  - HC: SN54HC259*, SN74HC259*, SNJ54HC259*, CD54HC259*, CD74HC259*.
  - HCT: CD54HCT259*, CD74HCT259*.
  - HCS: SN74HCS259*.
- **Military numbers:**
  - `8551901EA`: SNJ54HC259J, CDIP J16, **HC**. The CD54HC259F3A census mark also starts with "8551901EA", so the same SMD covers the CD54HC259F3A part.
  - `85519012A`: SNJ54HC259FK, LCCC FK20, **HC**.
  - `JM38510/65402BEA` and `M38510/65402BEA`: CDIP J16, **HC** (sn74hc259.pdf).
  - `5962-8985201EA`: CD54HCT259F3A, CDIP J16, **HCT** (cd74hc259.pdf).
- **Pinouts:** Pin names are identical across TTL, LS, HC, HCT and HCS.
  - **p16** covers N, D, NS, PW, J, W (SN HC); E/N, M/D, PW, F3A/J (CD); D, PW (HCS); J, W, N, D (LS/TTL). Pins:
    - `1 S0 input`, `2 S1 input`, `3 S2 input`
    - `4 Q0 output`, `5 Q1 output`, `6 Q2 output`, `7 Q3 output`
    - `8 GND ground`
    - `9 Q4 output`, `10 Q5 output`, `11 Q6 output`, `12 Q7 output`
    - `13 D input`, `14 G input` (active low), `15 CLR input` (active low), `16 VCC power`
    - Sources: sn74hc259 p3, cd74hc259 p3, sn74hcs259 p3, sn74ls259b p1. CD74 page 3 only says "J, D or PW Package", but the E (PDIP) rows exist in the census with the same arrangement.
  - **fk20** (LCCC FK: SNJ54HC259FK, and SN54LS259B FK, which has no census row). Pins:
    - `1 NC nc`, `2 S0 input`, `3 S1 input`, `4 S2 input`, `5 Q0 output`
    - `6 NC nc`, `7 Q1 output`, `8 Q2 output`, `9 Q3 output`, `10 GND ground`
    - `11 NC nc`, `12 Q4 output`, `13 Q5 output`, `14 Q6 output`, `15 Q7 output`
    - `16 NC nc`, `17 D input`, `18 G input`, `19 CLR input`, `20 VCC power`
    - Sources: sn74hc259 p3 and sn74ls259b p1; the two maps are identical. No thermal pads.
- **Behavior:** Function table, identical in all four PDFs (sn74ls259b p2, sn74hc259 p9, cd74hc259 p11, sn74hcs259 p11). "Addressed" means the latch selected by S2..S0. Qi0 is that latch's level before the input conditions were applied.

  | CLR | G | Addressed latch | Each other output | Function |
  |---|---|---|---|---|
  | H | L | D | Qi0 | Addressable latch |
  | H | H | Qi0 | Qi0 | Memory |
  | L | L | D | L | 8-line demultiplexer |
  | L | H | L | L | Clear |

  - **Address decode** (latch selection table): addressed latch n = 4·S2 + 2·S1 + S0. LLL → 0, LLH → 1, …, HHH → 7, with the columns in the order S2 S1 S0.
  - **Per-latch logic** (sn74hc259 p9, Fig 7-1, each internal latch):
    - The latch enable for latch n is `En = NOT G AND (S2..S0 == n)`. The latch is transparent while En = 1, so Qn follows D, and it holds when En falls.
    - The reset for latch n is active when `NOT CLR AND NOT En`. The per-latch AND gate has both inputs inverted, taking R-bar and C. So CLR resets every latch that is not currently enabled.
    - The addressed, enabled latch is not reset and follows D. This produces the demux row, where the addressed output = D and all others = L.
  - **Model:** For a cycle-accurate model:
    - If CLR = L: Qn = (G = L and n = addr) ? D : 0, for every n.
    - Else if G = L: Q[addr] = D (transparent) and the other latches hold.
    - Else: all latches hold.
    - CLR is asynchronous. It is level-sensitive, not edge-triggered. G is a level enable; the latch captures on G rising (L→H). That edge is the setup/hold reference: "data or address before G↑".
  - **Priority:** CLR does not override the enabled latch. With CLR = L and G = L, the addressed latch still follows D. Otherwise CLR (low) forces all latches low.
  - **Address changes while G = L** (addressable-latch mode): the newly addressed latch becomes transparent and takes D. The previously addressed latch keeps the value it had when it was deselected. This is why TI says: "To eliminate the possibility of entering erroneous data in the latches, G should be held high (inactive) while the address lines are changing" (sn74hc259 p8; LS p1). LS259B also gives "Setup time, address before G↓ = 0 ns" (LS p4), so the address must be stable before G falls.
  - **Demux mode with a changing address:** the old output goes low (it is reset) and the new one follows D.
  - **Clear mode:** all outputs are low and unaffected by the address and data inputs.
  - **Outputs:** All are push-pull and non-inverting. There are no 3-state outputs and no complementary outputs.
  - **Power-up:** No power-up state is stated.
- **Timing per family** (all paths go to "Any Q"):
  - **TTL** (SN74259, sn74ls259b p3). VCC = 5 V, 25 °C, **CL = 15 pF**, RL = 400 Ω. Values are typ (max):
    - CLR→Q tPHL 16 (25)
    - Data→Q tPLH 14 (24), tPHL 11 (20)
    - Address→Q tPLH 15 (28), tPHL 17 (28)
    - G→Q tPLH 12 (20), tPHL 11 (20)
    - No 50 pF data is printed.
  - **LS** (LS259B, sn74ls259b p4). VCC = 5 V, 25 °C, **CL = 15 pF**, RL = 2 kΩ. Values are typ (max):
    - CLR→Q tPHL 12 (18)
    - Data→Q tPLH 19 (30), tPHL 13 (20)
    - Address→Q tPLH 17 (27), tPHL 14 (20)
    - G→Q tPLH 15 (24), tPHL 15 (24)
  - **HC** (sn74hc259 p6). VCC = 4.5 V, CL = 50 pF, 25 °C. Values are typ (max at 25 °C / SN54 max / SN74 max):
    - CLR→Q tPHL 18 (30/45/38)
    - Data→Q tpd 17 (26/39/33)
    - Address→Q tpd 21 (40/60/50)
    - G→Q tpd 20 (34/51/43)
    - tt 8 (15)
  - **HC, CD74HC259 cross-check** (cd74hc259 p7). At 4.5 V and 50 pF only maxima are printed: D→Q 37, G→Q 34, S→Q 37, CLR→Q 31. The typicals 15/14/15/13 are given at CL = 15 pF, 5 V.
  - **HCT** (CD74HCT259, cd74hc259 p7). There is no typ at 50 pF, so these are **MAX** at 4.5 V, CL = 50 pF, 25 °C:
    - D→Q 39, G→Q 38, S (address)→Q 41, CLR→Q 39, tt 15
    - The typicals at CL = 15 pF, VCC = 5 V are 16, 16, 17, 16.
  - **HCS** (sn74hcs259 p6). VCC = 4.5 V, CL = 50 pF, 25 °C. Values are typ (max 25 °C / max −40..125 °C):
    - Data→Q 7 (12/18)
    - Address→Q 7 (12/20)
    - G→Q 7 (16/24)
    - CLR→Q tPHL 7 (12/20)
    - tt max 5
- **Setup/hold and pulse widths:**
  - TTL (p3): tw 15; tsu data 15, address 5; th data 0, address 20 (ns). All are referenced to G↑.
  - LS (p4): tw G 17, CLR 10; tsu data 20, address 17 (to G↑), address 0 (to G↓); th 0, 0.
  - HC at 4.5 V (sn74hc259 p5): tw 16, tsu 15, th 5 (25 °C).
  - CD HC at 4.5 V (p6): tw 14, tsu 16, th 0.
  - HCT (p6): tw 18, tsu 17, th 0.
  - HCS at 4.5 V (p5): tw 5, tsu 3, th 5.
- **VCC:**
  - TTL: SN54 4.5–5.5 V, SN74 4.75–5.25 V (p3).
  - LS: SN54 4.5–5.5 V, SN74 4.75–5.25 V (p4). Abs-max input is 7 V for LS and 5.5 V for TTL (p2); the ROC table gives no input range.
  - HC: 2–6 V for SN54 and SN74 (sn74hc259 p4) and for CD (cd74hc259 p4).
  - HCT: 4.5–5.5 V (cd74hc259 p4).
  - HCS: 2–6 V (sn74hcs259 p4). VI is 0..VCC for all CMOS families, so there is no inputMax.
- **Obsolete:** CD74HC259M, CD74HC259MT, CD74HCT259M, CD74HCT259MT, SN74HC259D, SN74HC259DT, SN74HC259PWT, SN74LS259BD.
  - Also of note: SN74HC259NSR is NRND.
  - Standard TTL SN54259/SN74259 and SN54LS259B have no census rows.
- **Notes:**
  - **HCS pin table error:** sn74hcs259 p3 lists pin 15 CLR with I/O "O". It is an input.
  - **HC hold-time typo:** sn74hc259 p5 labels the hold time "after GG↑", which is a typo for G↑.
  - **CD74 table typos (p6/p7):** the HC G pulse-width row lists VCC "5" instead of 6. The HCT CLR→Q and hold-time rows carry the unit "pF" instead of ns.
  - **Logic-diagram signal names:** sn74hc259 p9 names the per-latch inputs D-bar, C and R-bar.
  - **LS vs HC:** the LS259B G input has a different input structure from the other inputs: IIL is −0.4 mA for every input on LS259B. On TTL 74259, G has IIH 80 µA and IIL −3.2 mA, double the other inputs. This is electrical only; the function is identical across LS, HC, HCT and HCS.
  - **CD74HC259 description (p1)** calls the clear mode "one reset mode". The pin is still named CLR.

## 4099B

- **Title / function:** "CMOS 8-Bit Addressable Latch, High-Voltage Types (20-Volt Rating)". It has a serial data input, a 3-bit address A0–A2, an active-high WRITE DISABLE, an active-high master RESET, and 8 parallel outputs.
- **Datasheets:** `datasheets/latch/cd4099b.pdf` (SCHS066C, Oct 2003, scanned). Used p1 (description, logic diagram, functional diagram, terminal assignment), p2 (ROC, mode selection), p4 (dynamic characteristics, master timing).
- **Families & part-number prefixes:** C: CD4099B*.
- **Military numbers:** all rows. `JM38510/17601BEA` and `M38510/17601BEA` (CDIP J16) are 4099B.
- **Pinouts:**
  - **p16** covers N/E, J/F3A, D/M, NS, PW (p1 "TERMINAL ASSIGNMENT"). Pins:
    - `1 Q7 output`, `2 RESET input`, `3 DATA input`, `4 WRITE DISABLE input`
    - `5 A0 input`, `6 A1 input`, `7 A2 input`, `8 VSS ground`
    - `9 Q0 output`, `10 Q1 output`, `11 Q2 output`, `12 Q3 output`
    - `13 Q4 output`, `14 Q5 output`, `15 Q6 output`, `16 VDD power`
    - The printed name is "WRITE DISABLE", two words. The mode table and logic diagram abbreviate it to "WD", which is suggested as the pin id. The mode table abbreviates RESET to "R".
- **Behavior:** Mode selection table (p2), quoted exactly:

  | WD | R | Addressed latch | Unaddressed latch |
  |---|---|---|---|
  | 0 | 0 | Follows Data | Holds Previous State |
  | 0 | 1 | Follows Data | Reset to "0" (Active High 8-Channel Demultiplexer) |
  | 1 | 0 | Holds Previous State | Holds Previous State |
  | 1 | 1 | Reset to "0" | Reset to "0" |

  - **Function is the same as 74_259** with G = WD (active-high disable) and CLR = NOT RESET. **RESET is active HIGH**, the opposite of the 259's active-low CLR. Both WD and RESET are level-sensitive. RESET is asynchronous. Data is captured when WD rises (0→1).
  - **Description (p1):** data are written into the addressed bit when WRITE DISABLE is low. When WD is high, data entry is inhibited. All 8 outputs can be read continuously, independent of WD and the address inputs. With RESET = H and WD = H, all bits reset to 0. With RESET = H and WD = L, the device is a 1-of-8 demultiplexer: the addressed bit follows DATA and all unaddressed bits are held at 0.
  - **Address decode:** Latch n is selected when A2A1A0 = n, with A0 as the LSB (logic diagram p1: latch 0 gate takes A0-bar, A1-bar, A2-bar).
  - **Write-disable / address timing note (p2):** "a WRITE DISABLE ON time (the time that WRITE DISABLE is at a high level) must be observed during an address change for the total time that the external address lines A0, A1, and A2 are settling to a stable level, to prevent a wrong cell from being addressed" (Fig 3).
  - **Outputs:** push-pull, non-inverting.
- **Timing (C):** p4, TA = 25 °C, CL = 50 pF, RL = 200 kΩ. Values are typ (max) at VDD = 5 V; the 10 V and 15 V typicals follow in brackets.
  - Data→Q tPLH/tPHL 200 (400) [10 V 75, 15 V 50]
  - WRITE DISABLE→Q tPLH/tPHL 200 (400) [80, 60]
  - Reset→Q tPHL 175 (350) [80, 65]
  - Address→Q tPLH/tPHL 225 (450) [100, 75]
  - Transition time 100 (200)
- **Timing requirements at 5 V** (p2 ROC minimums, equal to the p4 max column):
  - Pulse width: data 200, address 400, reset 150
  - Setup, data to WD: 100
  - Hold, data to WD: 150
- **VDD:** 3–18 V (p2).
- **Obsolete:** CD4099BM.
- **Notes:**
  - **Different pinout from 4724B and 74_259:** Q7 is on pin 1 and RESET/DATA/WD are on pins 2–4.
  - **Packages in the census:** the p1 text lists M, MT, NSR, PW, PWR packages. The census has CD4099BM96, NSR and PWR. There are no FK/LCCC variants.

## 4724B

- **Title / function:** "CMOS 8-Bit Addressable Latch, High-Voltage Types (20-Volt Rating)". The description and mode table are word-for-word the same as CD4099B.
- **Datasheets:** `datasheets/latch/cd4724b.pdf` (SCHS092C, Jul 2003, scanned). Used p1 (description, logic diagram, functional diagram, terminal assignment), p2 (ROC, mode selection), p4 (dynamic characteristics).
- **Families & part-number prefixes:** C: CD4724B*.
- **Military numbers:** none in the census. All rows are CD4724BE and CD4724BF3A.
- **Pinouts:**
  - **p16** covers E/N, F3A/J (p1 "TERMINAL ASSIGNMENT"). Pins:
    - `1 A0 input`, `2 A1 input`, `3 A2 input`
    - `4 Q0 output`, `5 Q1 output`, `6 Q2 output`, `7 Q3 output`
    - `8 VSS ground`
    - `9 Q4 output`, `10 Q5 output`, `11 Q6 output`, `12 Q7 output`
    - `13 DATA input`, `14 WRITE DISABLE input` (WD), `15 RESET input`, `16 VDD power`
  - The functional diagram on p1 agrees: WD 14, DATA 13, A0/A1/A2 1/2/3, RESET 15, Q0–Q3 4–7, Q4–Q7 9–12.
- **Behavior:** Identical to 4099B. The mode table on p2 is the same:
  - WD = 0, R = 0: the addressed latch follows Data and the others hold.
  - WD = 0, R = 1: the addressed latch follows Data and the others reset to "0" (active-high 8-channel demultiplexer).
  - WD = 1, R = 0: all latches hold.
  - WD = 1, R = 1: all latches reset to "0".
  - RESET is active HIGH and WRITE DISABLE is active HIGH (inhibit). Data is captured when WD rises. The WD-ON-during-address-change note is the same as 4099B (p2).
- **Timing (C):** p4. The numbers are identical to 4099B. At VDD = 5 V, CL = 50 pF, values are typ (max):
  - Data→Q 200 (400)
  - WD→Q 200 (400)
  - Reset→Q tPHL 175 (350)
  - Address→Q 225 (450)
  - Transition 100 (200)
  - At 10 V: 75 / 80 / 80 / 100. At 15 V: 50 / 60 / 65 / 75.
- **VDD:** 3–18 V (p2).
- **Obsolete:** none.
- **Notes:**
  - **Pin positions match 74_259:** S0..S2 → A0..A2, D → DATA, G → WRITE DISABLE, CLR → RESET. The differences are that **RESET is active high**, where CLR is active low, and that the pins are named differently. WD has the same polarity as G: low = write.
  - **Relation to 4099B:** same function and same pin names, but a different pin order.
  - **Packages:** the p1 text mentions NSR and PW/PWR packages, but they are not in the census.


## 4042B
- Title / function: "CMOS Quad Clocked "D" Latch", High-Voltage Types (20-Volt Rating) — four D latches with a common CLOCK and a CLOCK-POLARITY control; Q and Q-bar outputs on each.
- Datasheets: datasheets/latch/cd4042b.pdf (SCHS040D) — PDF p1 (printed 3-112: description, functional diagram, terminal assignment, Fig. 1 logic + truth table), p2 (3-113: abs max, recommended operating conditions), p3 (3-114: dynamic characteristics, Fig. 12 timing notes). Pages 1–4 are scanned images; read visually.
- Families & part-number prefixes: C: CD4042B*.
- Military numbers: none in census (no 5962/JM38510 rows). The F3A row (CD4042BF3A) is the hermetic ceramic part; all rows are this device.
- Pinouts:
  - p16: N, D, DW, NS, PW, J (all codes in census) — terminal assignment, PDF p1:
    - 1 Q4 output
    - 2 Q1 output
    - 3 Q1BAR output (printed Q̄1)
    - 4 D1 input
    - 5 CLOCK input
    - 6 POLARITY input
    - 7 D2 input
    - 8 VSS ground
    - 9 Q2BAR output (printed Q̄2)
    - 10 Q2 output
    - 11 Q3 output
    - 12 Q3BAR output (printed Q̄3)
    - 13 D3 input
    - 14 D4 input
    - 15 Q4BAR output (printed Q̄4)
    - 16 VDD power
    (Functional diagram, p1, agrees: D1=4, D2=7, D3=13, D4=14; Q1=2, Q̄1=3, Q2=10, Q̄2=9, Q3=11, Q̄3=12, Q4=1, Q̄4=15; CLOCK=5, POLARITY=6, VDD=16, VSS=8.) The repo's 4013B uses `1Q`/`1QBAR` ordering; printed names here are Q1/D1 style.
- Behavior (PDF p1 text, functional diagram, Fig. 1 truth table; p3 Fig. 12 notes):
  - Internal latch enable CL = (CLOCK AND POLARITY) OR (NOT CLOCK AND NOT POLARITY) = CLOCK XNOR POLARITY (functional diagram: two AND gates, one with both inputs inverted, into an OR → CL). All four latches share CL.
  - CL = 1 → latches are transparent (Q = D, QBAR = NOT D). CL = 0 → latched (hold).
  - Truth table, Fig. 1 (quoted exactly):
    | CLOCK | POLARITY | Q |
    |---|---|---|
    | 0 | 0 | D |
    | ↑ (rising) | 0 | LATCH |
    | 1 | 1 | D |
    | ↓ (falling) | 1 | LATCH |
  - Full CLOCK × POLARITY table (from the XNOR / text "information present at the input during the CLOCK transition is retained at the outputs until an opposite CLOCK transition occurs"):
    | CLOCK | POLARITY | state |
    |---|---|---|
    | 0 | 0 | transparent |
    | 1 | 0 | latched (data captured on CLOCK rising edge) |
    | 1 | 1 | transparent |
    | 0 | 1 | latched (data captured on CLOCK falling edge) |
  - Text: "For POLARITY = 0 the transfer occurs during the 0 CLOCK level and for POLARITY = 1 the transfer occurs during the 1 CLOCK level ... When a CLOCK transition occurs (positive for POLARITY = 0 and negative for POLARITY = 1) the information present at the input during the CLOCK transition is retained at the outputs until an opposite CLOCK transition occurs." Fig. 12 notes: "1. For positive clock edge, input data is latched when POLARITY is low. 2. For negative clock edge, input data is latched when POLARITY is high."
  - Because CL is a pure XNOR, a change of POLARITY with CLOCK steady also opens/closes the latch (the datasheet does not discuss this; the logic diagram implies it).
  - Outputs: Q = stored bit, QBAR = complement; both buffered push-pull, always driven (no 3-state). No reset/preset. No power-up state stated.
- Timing (C, VDD = 5 V, CL = 50 pF, typical; PDF p3 / 3-114): D→Q 110 ns (max 220); D→QBAR 150 (max 300); CLOCK→Q 225 (max 450); CLOCK→QBAR 250 (max 500); transition tTHL/tTLH 100 (max 200). Min clock pulse width 100 typ/200 max; hold 60 typ/120; setup 0 typ/50 (p3). Recommended conditions (p2) at 5 V: tW ≥ 200, tS ≥ 50, tH ≥ 120 ns.
- VCC: VDD 3–18 V (recommended, p2). Abs max −0.5 to +20 V; inputs −0.5 to VDD+0.5 V (p2).
- Obsolete: CD4042BD, CD4042BDT.
- Notes: CD4042BF is NRND. Datasheet setup/hold in the dynamic table (typ 0 / 60 ns) and the recommended-conditions table (50 / 120 ns at 5 V) both appear; recommended values equal the dynamic-table max.

## 4043B
- Title / function: "CMOS Quad 3-State R/S Latches ... Quad NOR R/S Latch – CD4043B" — four cross-coupled NOR R-S latches, active-high S and R, 3-state Q outputs with common ENABLE.
- Datasheets: datasheets/latch/cd4043b.pdf (SCHS041D, covers CD4043B and CD4044B) — PDF p1 (3-116: text, functional diagrams, terminal assignments, truth tables, Fig. 1 logic diagrams, recommended conditions), p2 (3-117: static incl. 3-state leakage), p3 (3-118: dynamic characteristics), p4 (3-119: Fig. 13 enable test). Pages 1–4 scanned.
- Families & part-number prefixes: C: CD4043B*.
- Military numbers: none in census. All CD4043B* rows are this device.
- Pinouts:
  - p16: N, D, DW, NS, PW, J — terminal assignment CD4043B, PDF p1:
    - 1 Q4 output (3-state)
    - 2 Q1 output (3-state)
    - 3 R1 input
    - 4 S1 input
    - 5 ENABLE input
    - 6 S2 input
    - 7 R2 input
    - 8 VSS ground
    - 9 Q2 output (3-state)
    - 10 Q3 output (3-state)
    - 11 R3 input
    - 12 S3 input
    - 13 NC nc
    - 14 S4 input
    - 15 R4 input
    - 16 VDD power
    (Functional diagram p1 agrees: S1=4, R1=3, S2=6, R2=7, S3=12, R3=11, S4=14, R4=15, Q1=2, Q2=9, Q3=10, Q4=1, ENABLE=5, NC=13.)
- Behavior (p1 text, truth table, Fig. 1):
  - Truth table CD4043B (quoted): 
    | S | R | E | Q |
    |---|---|---|---|
    | X | X | 0 | OC* (open circuit) |
    | 0 | 0 | 1 | NC+ (no change) |
    | 1 | 0 | 1 | 1 |
    | 0 | 1 | 1 | 0 |
    | 1 | 1 | 1 | Δ "DOMINATED BY S=1 INPUT" → Q = 1 |
  - S and R are active HIGH, level-sensitive, asynchronous. Both high: Q = 1 (S dominates). Fig. 1 logic: each input is inverted and fed to a cross-coupled NAND pair; Q is taken from the S-side gate, so S=R=1 forces the stored node to 1. On simultaneous release of S=R=1 to 0/0 the resulting state is not specified (race); after releasing only R, Q stays 1.
  - ENABLE (pin 5) active HIGH, common to all four outputs: "A logic "1" or high on the ENABLE input connects the latch states to the Q outputs. A logic "0" or low on the ENABLE input disconnects the latch states from the Q outputs, resulting in an open circuit condition on the Q outputs." Output stage (Fig. 1): PMOS gate = NAND(E, latch), NMOS gate = NOR(Ē, latch) → E=0 gives high-Z. ENABLE only gates outputs; the latches keep responding to S/R while disabled (the latch is ahead of the output gating in Fig. 1).
  - Q output is non-inverting relative to the stored state; no QBAR output. No power-up state stated.
- Timing (C, VDD = 5 V, CL = 50 pF, RL = 200 kΩ, typical; PDF p3 / 3-118, "ALL TYPES" = 4043B and 4044B): SET or RESET→Q tPHL/tPLH 150 ns (max 300); ENABLE→Q tPHZ/tPZH 115 (max 230); tPLZ/tPZL 90 (max 180); transition 100 (max 200); min SET/RESET pulse width 80 typ/160 max. Recommended (p1): S/R pulse width ≥ 160 ns at 5 V.
- VCC: VDD 3–18 V (p1 recommended conditions).
- Obsolete: CD4043BD, CD4043BDT, CD4043BDW.
- Notes: 3-state leakage IOUT ±0.4 µA max at 25 °C, 18 V (p2).

## 4044B
- Title / function: "Quad NAND R/S Latch – CD4044B" — four cross-coupled NAND R-S latches, active-LOW S and R (printed S1/R1 with no overbar), 3-state Q outputs with common ENABLE.
- Datasheets: datasheets/latch/cd4043b.pdf — same pages as 4043B (p1 terminal assignment/truth table/logic, p3 timing).
- Families & part-number prefixes: C: CD4044B*.
- Military numbers: none in census. All CD4044B* rows are this device (map by name prefix within cd4043b.pdf).
- Pinouts (pin order DIFFERS from 4043B):
  - p16: N, D, DW, NS, PW, J — terminal assignment CD4044B, PDF p1:
    - 1 Q4 output (3-state)
    - 2 NC nc
    - 3 S1 input
    - 4 R1 input
    - 5 ENABLE input
    - 6 R2 input
    - 7 S2 input
    - 8 VSS ground
    - 9 Q2 output (3-state)
    - 10 Q3 output (3-state)
    - 11 S3 input
    - 12 R3 input
    - 13 Q1 output (3-state)
    - 14 R4 input
    - 15 S4 input
    - 16 VDD power
    (Functional diagram p1 agrees: R1=4, S1=3, R2=6, S2=7, R3=12, S3=11, R4=14, S4=15, Q1=13, Q2=9, Q3=10, Q4=1, ENABLE=5, NC=2.)
- Behavior (p1):
  - Truth table CD4044B (quoted):
    | S | R | E | Q |
    |---|---|---|---|
    | X | X | 0 | OC* (open circuit) |
    | 1 | 1 | 1 | NC+ (no change) |
    | 0 | 1 | 1 | 1 |
    | 1 | 0 | 1 | 0 |
    | 0 | 0 | 1 | ΔΔ "DOMINATED BY R=0 INPUT" → Q = 0 |
  - S and R active LOW, level-sensitive, asynchronous. Both low: Q = 0 (R dominates). Fig. 1: inputs inverted into a cross-coupled NOR pair; Q taken from the R-side gate. Release of S=R=0 simultaneously: unspecified.
  - ENABLE active HIGH (same as 4043B): 1 → outputs driven, 0 → high-Z; latches keep operating while disabled.
- Timing: identical to 4043B ("ALL TYPES", p3): S/R→Q 150 ns typ; ENABLE→Q tPHZ/tPZH 115, tPLZ/tPZL 90; transition 100; at VDD 5 V, CL 50 pF.
- VCC: VDD 3–18 V (p1).
- Obsolete: CD4044BD, CD4044BDT.
- Notes: CD4044BDW is NRND. Pin names are printed S/R without overbars even though active-low; datasheet bullet "CD4044B for negative logic systems".

## 4508B
- Title / function: "CMOS Dual 4-Bit Latch" — two independent 4-bit transparent latches, each with its own STROBE, RESET and OUTPUT DISABLE, 3-state outputs ("similar to industry type MC14508").
- Datasheets: datasheets/latch/cd4508b.pdf (SCHS070B) — PDF p1 (3-245: description, features, functional diagram, recommended conditions), p2 (3-246: Fig. 7 logic diagram + truth table), p3 (3-247: dynamic characteristics, Fig. 12), p4 (3-248: terminal assignment). Pages 1–4 scanned.
- Families & part-number prefixes: C: CD4508B*.
- Military numbers: none in census. All rows are this device.
- Pinouts:
  - p24: DW (census "M"/"M96" = DW), NS, PW, J — terminal assignment, PDF p4 (printed names with spaces, e.g. "RESET A", "D0 A", "OUTPUT DISABLE A"; suggested repo names in brackets):
    - 1 RESET A [RESETA] input
    - 2 STROBE A [STROBEA] input
    - 3 OUTPUT DISABLE A [ODA] input
    - 4 D0 A [D0A] input
    - 5 Q0 A [Q0A] output (3-state)
    - 6 D1 A [D1A] input
    - 7 Q1 A [Q1A] output (3-state)
    - 8 D2 A [D2A] input
    - 9 Q2 A [Q2A] output (3-state)
    - 10 D3 A [D3A] input
    - 11 Q3 A [Q3A] output (3-state)
    - 12 VSS ground
    - 13 RESET B [RESETB] input
    - 14 STROBE B [STROBEB] input
    - 15 OUTPUT DISABLE B [ODB] input
    - 16 D0 B [D0B] input (printed "DOB")
    - 17 Q0 B [Q0B] output (3-state)
    - 18 D1 B [D1B] input
    - 19 Q1 B [Q1B] output (3-state)
    - 20 D2 B [D2B] input
    - 21 Q2 B [Q2B] output (3-state)
    - 22 D3 B [D3B] input
    - 23 Q3 B [Q3B] output (3-state)
    - 24 VDD power
    (Fig. 7 p2 confirms A section: OUTPUT DISABLE-A = 3, RESET-A = 1, STROBE-A = 2, Dn-A = 4(6,8,10), Qn-A = 5(7,9,11).)
- Behavior (p1 text, p2 Fig. 7 truth table + logic diagram), per half (A and B fully independent):
  - Truth table (quoted):
    | RESET | DISABLE | STROBE | D INPUT | Q OUTPUT |
    |---|---|---|---|---|
    | 0 | 0 | 1 | 1 | 1 |
    | 0 | 0 | 1 | 0 | 0 |
    | 0 | 0 | 0 | X | LATCHED |
    | 1 | 0 | X | X | 0 |
    | X | 1 | X | X | Z |
  - STROBE active HIGH = transparent (Q follows D, non-inverting); STROBE high→low latches. Text: "With the STROBE line in the high state, the data on the "D" inputs appear at the corresponding "Q" outputs provided the DISABLE line is in the low state. Changing the STROBE line to the low state locks the data into the latch."
  - RESET active HIGH, asynchronous, overrides STROBE/D: "A high on the reset line forces the outputs to a low level regardless of the state of the STROBE input." From Fig. 7: stored node passes through NOR(RESET, node) whose output is also the latch feedback (via an inverter and the STROBE-low transmission gate), so RESET also clears the stored bit when STROBE is low. Equivalent model per bit: q = RESET ? 0 : (STROBE ? D : q). If RESET falls while STROBE is high, Q = D immediately.
  - OUTPUT DISABLE active HIGH, highest priority for the pins only: 1 → all four Q of that half high-Z ("forced to the high-impedance state for bus line applications by a high level on the DISABLE input"). Output stage: PMOS gate = NAND(NOT OD, q), NMOS gate = NOR(OD, q). DISABLE does not affect the stored data (latch/reset still operate while disabled).
  - Priority: pins: OUTPUT DISABLE (Z) > RESET (0) > STROBE (transparent/hold). Storage: RESET > STROBE.
  - No QBAR outputs. No power-up state stated.
- Timing (C, VDD = 5 V, CL = 50 pF, RL = 200 kΩ, typical; PDF p3 / 3-247): STROBE→Q tPHL/tPLH 130 ns (max 260); D→Q 105 (max 210); RESET→Q 90 (max 180); tPHZ 90, tPZH 90, tPLZ 90, tPZL 90 (each max 180); transition 100 (max 200). Min reset pulse 100 typ/200 max; min strobe pulse 70/140; setup 25/50; hold 0/0. Recommended (p1) at 5 V: tW(R) ≥ 200, tW(st) ≥ 140, tSU ≥ 50, tH ≥ 0 ns. Feature bullet (p1): tPHL = tPLH = 70 ns typ at VDD 10 V.
- VCC: VDD 3–18 V (p1 recommended).
- Obsolete: CD4508BM.
- Notes: text lists an E (PDIP) package but the census has no N row. "OUTPUT DISABLE" is called "DISABLE" in the truth table/text.


## 74_4511

- Title / function: "CDx4HC4511, CD74HCT4511 BCD-to-7 Segment Latch/Decoder/Drivers" (p1). Four BCD inputs D0–D3 go into a 4-bit transparent latch, then a BCD-to-7-segment decoder, then active-high segment drivers a–g. Inputs: active-low blanking (BL), lamp test (LT) and latch enable (LE). Function diagram: Latch → Decoder → Driver (p10, Fig 7-1).
- Datasheets: datasheets/latch/cd74hc4511.pdf pp. 1 (features, glyphs), 3 (pinout), 4–5 (ROC), 6–7 (timing), 10 (function diagram and function table).
- Families & part-number prefixes:
  - HC: CD54HC4511*, CD74HC4511*
  - HCT: CD74HCT4511* (N package only, p1 Device Information)
- Military numbers: all rows. 5962-8773301EA = CD54HC4511F3A (HC, J/CDIP16).
- Pinouts: p16, used by J, N, D and PW (p3, "J, N, D, PW package 16-Pin CDIP, PDIP, SOIC, TSSOP"). None of these has an exposed pad.
  ```
  1  D1   input
  2  D2   input
  3  LT   input      (printed with overbar: active low)
  4  BL   input      (printed with overbar: active low)
  5  LE   input      (printed with overbar: low = transparent, high = latch/hold)
  6  D3   input
  7  D0   input
  8  GND  ground
  9  e    output
  10 d    output
  11 c    output
  12 b    output
  13 a    output
  14 g    output
  15 f    output
  16 VCC  power
  ```
  Segment letters are lower-case as printed. Segment geometry is the standard one (p1 drawing): a top, b upper right, c lower right, d bottom, e lower left, f upper left, g middle.
- Behavior (p10 text and Table 7-1, p1):
  - **Latch stores the BCD input, not the decoded segments.** The latch comes before the decoder (Fig 7-1). LT and BL act after the latch, on the decoder/driver, so they are combinational and asynchronous and do not change the stored code.
  - LE (pin 5): when LE = L the latch is transparent and the outputs follow D0–D3. When LE = H the latch holds the code that was present when LE went high. Timing is specified as "Setup time, BCD inputs before LE↑" and "Hold time … LE↑" (p6/p7), so the code is captured on the LE rising edge. Text (p10): "a latch-enable (LE) input that, when high, enables the latches to store the BCD inputs. When LE is low, the latches are disabled, making the outputs transparent to the BCD inputs."
  - Priority (Table 7-1): **LT = L beats everything.** Then BL = L. Then the decoder output of the latched or transparent code.
    1. LT = L (BL, LE, D = X) → a..g all H. Displays "8".
    2. else BL = L (LE, D = X) → a..g all L. Display is blank.
    3. else (LT = H, BL = H) → segments = decode(Q), where Q = D when LE = L and Q = the stored code when LE = H. In the table the LE = H row shows "t" in every column. The footnote marks this as a caret: "caret = Depends on BCD code previously applied when LE = L".
  - Full segment truth table (LT = H, BL = H, LE = L). H = segment driven high (on).

    | code | D3 D2 D1 D0 | a b c d e f g | display |
    |---|---|---|---|
    | 0 | L L L L | H H H H H H L | 0 |
    | 1 | L L L H | L H H L L L L | 1 |
    | 2 | L L H L | H H L H H L H | 2 |
    | 3 | L L H H | H H H H L L H | 3 |
    | 4 | L H L L | L H H L L H H | 4 |
    | 5 | L H L H | H L H H L H H | 5 |
    | 6 | L H H L | L L H H H H H | 6 (no top tail: a = L) |
    | 7 | L H H H | H H H L L L L | 7 (a, b, c only; f off) |
    | 8 | H L L L | H H H H H H H | 8 |
    | 9 | H L L H | H H H L L H H | 9 (no bottom tail: d = L) |
    | 10–15 | H L H L … H H H H | L L L L L L L | Blank |

    Footnote (1): "Display is blank for all illegal input codes (BCD > HLLH)."
  - Glyphs 6 and 9: by the function table, 6 and 9 have no tails. Segment a is off for 6 and segment d is off for 9, matching the classic CD4511B. **Datasheet inconsistency:** the decorative digit picture on p1 draws 6 *with* a top bar and 9 *with* a bottom bar. Treat Table 7-1 as authoritative.
  - Outputs: push-pull drivers that source current; there is no 3-state output. The table's footnote (2) defines Z, but no row uses it. The text (p10) says: "standard-size output transistors, but are capable of sourcing (at standard VOH levels) up to 7.5 mA at 4.5 V. The HC types can supply up to 10 mA at 6 V." Features (p1): 7.5 mA at 4.5 V (HCT), 10 mA at 6 V ('HC). Electrical characteristics (p5) give IOH = −4 mA at 4.5 V for VOH = 3.98 V, and IOL = 4 mA. Absolute max is ±25 mA per pin (p4). The outputs are not the high-current bipolar emitter-follower outputs of the CD4511B.
  - Power-up state: not stated.
- Timing (pp. 6–7). No typical is printed at CL = 50 pF. The values below are **MAX at VCC = 4.5 V, CL = 50 pF, TA = 25 °C**. The typical at VCC = 5 V, CL = 15 pF is in brackets.

  | path (tpd) | HC | HCT |
  |---|---|---|
  | Dn → segment output (transparent, LE = L) | 60 [typ 25 @5V/15pF] (p6) | 60 [25] (p7) |
  | LE → segment output | 54 [23] (p6) | 54 [23] (p7) |
  | BL → segment output | 44 [18] (p6) | 44 [18] (p7) |
  | LT → segment output | 32 [13] (p6) | 33 [13] (p7) |
  | tt output transition | 15 max | 15 max |

  - Timing requirements at 4.5 V, 25 °C:
    - HC: tW (LE low) 16 ns, tsu (BCD before LE↑) 12 ns, th 3 ns (p6).
    - HCT: tW 16 ns, tsu 16 ns, th 5 ns (p7).
- VCC:
  - HC: 2–6 V (p4, same for −55/125 and −40/85 rows).
  - HCT: 4.5–5.5 V (p4).
  - VI max is VCC for both. No inputMax above VCC.
- Obsolete: CD74HC4511M, CD74HC4511PWT. (CD74HCT4511E is NRND.)
- Notes:
  - Pin-name overbars: LT, BL and LE are drawn with overbars on p3. Only LT and BL are truly active-low. LE-bar is "transparent when low".
  - The datasheet calls BL "BL". The CD4000 name for it is "BI".
  - The "t" entries in the LE = H row are a typesetting error for a caret, meaning "previous state".
  - The HCT LT→output max is 33 ns at 25 °C, versus 32 ns for HC.

## 74_16260

- Title / function: "SN74ALVCH16260 12-BIT TO 24-BIT MULTIPLEXED D-TYPE LATCH WITH 3-STATE OUTPUTS" (p1). It has three 12-bit I/O ports: A1–A12, 1B1–1B12 and 2B1–2B12. A→B demultiplexes A into two latched B banks. B→A latches each B bank and multiplexes onto A under SEL.
- Datasheets: datasheets/latch/sn74alvch16260.pdf pp. 1 (pinout, description), 2 (function tables), 3 (logic diagram), 4 (ROC), 5–6 (timing), 9 (3.3 V load: CL = 50 pF).
- Families & part-number prefixes: ALVC (ALVCH, with bus hold): SN74ALVCH16260*.
- Military numbers: none (all rows are commercial SN74).
- Pinouts: p56, used by DGG (TSSOP) and DL (SSOP). Both share one pinout, "DGG OR DL PACKAGE (TOP VIEW)" (p1), with no exposed pad. Pin numbers below were cross-checked against the logic diagram on p3. Data I/O pins are inout. LE, OE and SEL pins are inputs.
  ```
  1  OEA   input (overbar, active low)   29 OE1B  input (overbar, active low)
  2  LE1B  input                         30 LEA1B input
  3  2B3   inout                         31 1B4   inout
  4  GND   ground                        32 GND   ground
  5  2B2   inout                         33 1B5   inout
  6  2B1   inout                         34 1B6   inout
  7  VCC   power                         35 VCC   power
  8  A1    inout                         36 1B7   inout
  9  A2    inout                         37 1B8   inout
  10 A3    inout                         38 1B9   inout
  11 GND   ground                        39 GND   ground
  12 A4    inout                         40 1B10  inout
  13 A5    inout                         41 1B11  inout
  14 A6    inout                         42 1B12  inout
  15 A7    inout                         43 2B12  inout
  16 A8    inout                         44 2B11  inout
  17 A9    inout                         45 2B10  inout
  18 GND   ground                        46 GND   ground
  19 A10   inout                         47 2B9   inout
  20 A11   inout                         48 2B8   inout
  21 A12   inout                         49 2B7   inout
  22 VCC   power                         50 VCC   power
  23 1B1   inout                         51 2B6   inout
  24 1B2   inout                         52 2B5   inout
  25 GND   ground                        53 GND   ground
  26 1B3   inout                         54 2B4   inout
  27 LE2B  input                         55 LEA2B input
  28 SEL   input                         56 OE2B  input (overbar, active low)
  ```
  There are 4 VCC pins (7, 22, 35, 50) and 8 GND pins (4, 11, 18, 25, 32, 39, 46, 53).
- Behavior (p1 text, p2 tables, p3 logic diagram). Each of the 12 channels n has four latches:
  - **B→A path:** latch L1B (D = pin 1Bn, C = LE1B) and latch L2B (D = pin 2Bn, C = LE2B). A 2:1 mux (G1 = SEL; input "1" = L1B output, input "1̄" = L2B output) drives a buffer onto An, enabled by OEA = L.
    - `An = OEA==L ? (SEL==H ? Q1B : Q2B) : Z`
    - The latches come before the mux, so SEL is a combinational select between two stored or transparent values. tpd SEL→A is specified.
  - **A→B path:** pin An feeds two latches.
    - Latch LA1B (C = LEA1B) drives 1Bn, enabled by OE1B = L: `1Bn = OE1B==L ? QA1B : Z`.
    - Latch LA2B (C = LEA2B) drives 2Bn, enabled by OE2B = L: `2Bn = OE2B==L ? QA2B : Z`.
  - **Latches:** all are D-type and transparent while their LE is HIGH. They latch when LE goes low and hold until LE returns high (p1: "When the latch-enable input is high, the latch is transparent. When the latch-enable input goes low, the data present at the inputs is latched"). Setup and hold are referenced to the LE falling edge. Latches have no clear or preset.
  - **Output enables:** OEA, OE1B and OE2B are all active-low, independent and asynchronous. OE1B and OE2B give independent bank control: each one gates only its own bank (table rows "L H → 1B Active, 2B Z" etc.). OE affects only the output buffers. The latches keep loading from the pins, so the B latches capture whatever is on the B pins even when the B outputs are enabled.
  - B TO A table (p2, heading "OEB = H", meaning OE1B = OE2B = H):

    | 1B | 2B | SEL | LE1B | LE2B | OEA | A |
    |---|---|---|---|---|---|---|
    | H | X | H | H | X | L | H |
    | L | X | H | H | X | L | L |
    | X | X | H | L | X | L | A0 |
    | X | H | L | X | H | L | H |
    | X | L | L | X | H | L | L |
    | X | X | L | X | L | L | A0 |
    | X | X | X | X | X | H | Z |

    A0 is the level stored in the selected latch. If SEL changes while both LEs are low, A switches to the other bank's stored value; the table writes this simply as A0.
  - A TO B table (p2, "OEA = H"):

    | A | LEA1B | LEA2B | OE1B | OE2B | 1B | 2B |
    |---|---|---|---|---|---|---|
    | H | H | H | L | L | H | H |
    | L | H | H | L | L | L | L |
    | H | H | L | L | L | H | 2B0 |
    | L | H | L | L | L | L | 2B0 |
    | H | L | H | L | L | 1B0 | H |
    | L | L | H | L | L | 1B0 | L |
    | X | L | L | L | L | 1B0 | 2B0 |
    | X | X | X | H | H | Z | Z |
    | X | X | X | L | H | Active | Z |
    | X | X | X | H | L | Z | Active |
    | X | X | X | L | L | Active | Active |

  - Bus contention: enabling OEA together with OE1B or OE2B, with transparent latches, forms a loop through pins. The datasheet neither forbids nor describes this case; the tables are written with the opposite direction disabled.
  - Bus hold (p1, p5): "Active bus-hold circuitry is provided to hold unused or floating data inputs at a valid logic level." Bus hold is on all A and B data I/O pins, not on control inputs. Unused control inputs must be tied (p4 note). II(hold) at 3 V is 75 µA min at VI = 0.8 V and −75 µA at 2 V. The overdrive needed at 3.6 V is ±500 µA max. In a simulator: a floating (undriven) A/B pin keeps its last driven level.
  - Power-up: not specified. The datasheet only advises an OE pull-up to VCC so outputs stay Z during power-up and power-down (p1).
  - Outputs are 3-state, ±24 mA at 3 V (p4).
- Timing at **VCC = 3.3 V ± 0.3 V, CL = 50 pF** (p6; load p9). No typical is printed (marked "not available at time of publication"), so these are **MIN / MAX** in ns:
  - tpd A→B or B→A (data through transparent latch): 1.2 / **4.3**
  - tpd LE→A or B: 1.0 / **4.4**
  - tpd SEL→A: 1.1 / **5.6**
  - ten OE→A or B (tPZH/tPZL): 1.0 / **5.4**
  - tdis OE→A or B (tPHZ/tPLZ): 1.3 / **4.6**
  - At 2.7 V (max only): 5.1, 5.2, 6.6, 6.4, 5.0 ns.
  - At 2.5 V ± 0.2 V (min/max): 1/5.4, 1/5.6, 1/6.9, 1/6.7, 1/5.7 ns.
  - Timing requirements at 3.3 V (p5): tw LE high 3.3 ns, tsu 1.1 ns, th 1.5 ns. (At 2.7 V: tsu 1.1, th 1.9. At 2.5 V: tsu 1.4, th 1.6.)
- VCC: 1.65–3.6 V (p4).
  - Absolute max VI is 4.6 V on control inputs and VCC + 0.5 V on I/O ports (p4).
  - Recommended VI is 0 to VCC.
  - No 5 V tolerance; no inputMax above VCC.
- Obsolete: none (SN74ALVCH16260DGGR and SN74ALVCH16260DL are both Active).
- Notes:
  - The 1.8 V timing columns are all "not available".
  - The pin table orders the pins 1..28 down the left and 56..29 down the right. In the pdftotext dump the pairs "LE2B 27 30 LEA1B" and "SEL 28 29 OE1B" are adjacent; the logic diagram confirms LEA1B = 30, OE1B = 29, LEA2B = 55, OE2B = 56, OEA = 1, LE1B = 2, LE2B = 27, SEL = 28.


### TPIC shared facts (apply to the five TPIC sections below)

- All five datasheets are from 1992–1997 (SLIS009A, SLIS004B, SLIS030, SLIS011A, SLIS031).
- Supply: Recommended Operating Conditions VCC 4.5–5.5 V. Absolute max VCC 7 V.
- Logic inputs: absolute max input range −0.3 V to 7 V, so inputMax = 7 V (abs-max, not a recommended value). VIH min = 0.85·VCC, VIL max = 0.15·VCC. These are CMOS-like thresholds, not TTL.
- Outputs: every output is a low-side open-drain DMOS transistor. Its source connects internally to PGND/GND. **DRAINn is ON (sinking, pulled low) when the stored bit is 1, and OFF (high-Z, pulled high by the load) when the bit is 0.** Clear turns every output OFF ("H").
- None of these parts has an output-enable pin. On the 259s, G is the latch write enable. It does not gate the outputs.
- Timing is printed only as TYP at VCC = 5 V, TC = 25 °C, CL = 30 pF (not 50 pF), with a resistive load to 24 V. The figures are hundreds of ns, set by the DMOS outputs.

## TPIC6259
- Title: "POWER LOGIC 8-BIT ADDRESSABLE LATCH" (p1). It has eight addressable latches with 3-to-8 decoding/demultiplexing and active-low DMOS open-drain outputs.
- Datasheets: datasheets/latch/tpic6259.pdf (SLIS009A, Sept 1995). Pages used: p1 (pinout, function table, latch selection table, description), p3 (logic diagram), p4 (absolute max), p5 (ROC, electrical, switching), p6 (Fig. 1 and 2 waveforms and test circuits).
- Families & prefixes: TPIC: TPIC6259DW, TPIC6259DWR, TPIC6259DWG4, TPIC6259DWRG4, TPIC6259N.
- Military numbers: none (all rows commercial).
- Pinouts: one arrangement, p20: DW (SOIC-20 wide), N (PDIP-20). No exposed pad. Source: p1.
  ```
  1 PGND ground      11 PGND ground
  2 VCC power        12 S2 input
  3 S0 input         13 G input        (active low, printed G-bar)
  4 DRAIN0 output    14 DRAIN4 output
  5 DRAIN1 output    15 DRAIN5 output
  6 DRAIN2 output    16 DRAIN6 output
  7 DRAIN3 output    17 DRAIN7 output
  8 S1 input         18 D input
  9 LGND ground      19 CLR input      (active low, printed CLR-bar)
  10 PGND ground     20 PGND ground
  ```
  - p1: "Pins 1, 10, 11, and 20 are internally connected" (PGND). Pin 9 LGND is the logic ground. LGND and PGND must be joined externally at a single point. There are no NC pins.
  - All DRAIN outputs are open-drain (role output, open-drain).
- Behavior (p1 function table and text; p3 logic diagram):
  - Address: S2 S1 S0 is binary, with S0 = LSB. LLL selects DRAIN0 and HHH selects DRAIN7 (latch selection table, p1).
  - Function table (p1), quoted. Qio = previous state:

    | CLR | G | D | addressed DRAIN | each other DRAIN | function |
    |---|---|---|---|---|---|
    | H | L | H | L | Qio | Addressable Latch |
    | H | L | L | H | Qio | Addressable Latch |
    | H | H | X | Qio | Qio | Memory |
    | L | L | H | L | H | 8-Line Demultiplexer |
    | L | L | L | H | H | 8-Line Demultiplexer |
    | L | H | X | H | H | Clear |

    - Addressable latch (CLR = H, G = L): only the addressed latch changes. It is transparent and follows D. The other seven hold.
    - Memory (CLR = H, G = H): no latch changes.
    - Demultiplexer (CLR = L, G = L): the addressed latch follows D. The other seven are forced to 0 (outputs off/H).
    - Clear (CLR = L, G = H): all eight are forced to 0 (outputs off/H).
  - Exact per-latch logic, read from the p3 diagram:
    - Each latch i gets `Ci = dec_i(S2,S1,S0) AND NOT G` from a NAND decoder followed by a NOR with G.
    - Its active-low clear is `CLRi_n = Ci OR CLR`. So a latch clears only when CLR = L **and** it is not currently enabled.
    - Model for each latch i:
      - if `G=L and addr==i` → `bit_i = D` (transparent, level-sensitive);
      - else if `CLR=L` → `bit_i = 0`;
      - else hold.
    - Priority: write-enable of the addressed latch > CLR > hold. CLR is asynchronous; it needs no clock.
    - This reproduces every row of the table, including demux mode. In demux mode the addressed latch is transparent. When G rises with CLR still L, its Ci falls and it clears too, giving Clear mode.
  - Latching edge: the latch closes on G↑. ROC gives "Setup time, D high before G↑, 10 ns; Hold time, D high after G↑, 5 ns; Pulse duration 15 ns" (p5).
    - Changing the address while G = L moves the transparent window. p1 says "enable G should be held high (inactive) while the address lines are changing."
    - The ">" dynamic marks drawn on the C1 inputs in the p3 diagram contradict the function table and Fig. 2, which measures D→output with G held low. Treat the part as a level latch.
  - Output polarity: `DRAINn = bit_n ? ON (low) : OFF (H/high-Z)`. p1: "The addressed DMOS transistor output inverts the data input." D = H stores 1, which gives DRAIN = L.
  - Power-up state: not stated.
- Ratings:
  - V(BR)DSX min 45 V at ID = 1 mA (p5). The output clamp is "45 V" (p1 feature list; p4 schematic shows a 45 V clamp). VDS abs max 45 V (p4).
  - Continuous ID per output, all outputs on: 250 mA (TA = 25 °C). Pulsed per output, all on: 750 mA. Peak single output IDM: 2 A (p4).
  - ROC pulsed drain output current −1.8 A to 1.5 A (p5). The feature list says "1.5-A Pulsed Current Per Output" (p1).
  - rDS(on) typ 1.3 Ω, max 2 Ω, at 250 mA and VCC = 4.5 V (p5). EAS 75 mJ.
  - ICC typ 15 µA, max 100 µA (p5).
- Timing (p5, TYP, VCC = 5 V, TC = 25 °C, CL = 30 pF, ID = 250 mA, RL = 95 Ω to 24 V per Fig. 1/2 p6):
  - D→DRAIN tPLH = 625 ns. The output goes high when D falls (Fig. 2 p6).
  - D→DRAIN tPHL = 140 ns. The output goes low when D rises.
  - tr = 650 ns; tf = 400 ns.
  - These are measured with G held L (transparent). No G→output, CLR→output or address→output delay is printed.
- VCC: 4.5–5.5 V (p5). inputMax 7 V (abs max, p4).
- Obsolete: none.
- Notes:
  - Function is the same as 74_259, but the "Q" sense is inverted at the pin (open-drain, active low).
  - TPIC6A259 and TPIC6B259 have the same function table and logic.
  - TPIC6B259 has the same pinout except pins 1 and 20 become NC and pins 9, 10, 11 become GND.

## TPIC6A259
- Title: "POWER LOGIC 8-BIT ADDRESSABLE LATCH" (p1). It has eight addressable latches or an 8-line demultiplexer, active-low DMOS open-drain outputs, and **per-output chopping current limiting**.
- Datasheets: datasheets/latch/tpic6a259.pdf (SLIS004B, Sept 1995). Pages used: p1 (pinouts, function table), p2 (packages), p3 (logic diagram), p4 (absolute max), p5 (ROC, electrical, switching), p6 (Fig. 1/2).
- Families & prefixes: TPIC: TPIC6A259DW, TPIC6A259DWG4, TPIC6A259DWRG4 (DW-24), TPIC6A259NE (NE-20).
- Military numbers: none.
- Pinouts: the two packages have **different pinouts** (p1, verified visually). The pin names are the same; DW-24 adds four extra PGND pins. Neither package has NC pins.
  - p20, NE (thermally enhanced PDIP-20):
    ```
    1 DRAIN2 output   11 DRAIN6 output
    2 DRAIN3 output   12 DRAIN7 output
    3 S1 input        13 D input
    4 LGND ground     14 CLR input (active low)
    5 PGND ground     15 PGND ground
    6 PGND ground     16 PGND ground
    7 S2 input        17 VCC power
    8 G input (active low)  18 S0 input
    9 DRAIN4 output   19 DRAIN0 output
    10 DRAIN5 output  20 DRAIN1 output
    ```
  - p24, DW (SOIC-24 wide):
    ```
    1 DRAIN2 output   13 DRAIN6 output
    2 DRAIN3 output   14 DRAIN7 output
    3 S1 input        15 D input
    4 LGND ground     16 CLR input (active low)
    5 PGND ground     17 PGND ground
    6 PGND ground     18 PGND ground
    7 PGND ground     19 PGND ground
    8 PGND ground     20 PGND ground
    9 S2 input        21 VCC power
    10 G input (active low)  22 S0 input
    11 DRAIN4 output  23 DRAIN0 output
    12 DRAIN5 output  24 DRAIN1 output
    ```
  - p1: "All PGND terminals are internally connected". LGND and PGND are joined externally at a single point.
  - This pin order (drains at the corners, PGND in the middle) is completely different from the TPIC6259/TPIC6B259 20-pin order.
- Behavior:
  - Identical to TPIC6259. The function table (p1) is the same row for row. The latch selection table is the same (S0 = LSB).
  - The p3 logic diagram is the same topology: `Ci = dec_i·NOT G`, `CLRi_n = Ci OR CLR`. Use the same per-latch model and priority (addressed write-enable > CLR > hold). Outputs are ON (low) when the bit is 1. Clear means all outputs OFF (H).
  - Extra in this part: a "Current Limit and Charge Pump" block sits between the latches and the gates (p3). Each output has "an independent chopping current-limiting circuit to prevent damage in the case of a short circuit" (p1). IOK (output current at which chopping starts) is 0.6 / 0.8 / 1.1 A min / typ / max (p5). This is analog only and has no logic effect.
  - ROC (p5): tsu 10 ns (D before G↑), th 5 ns, tw 15 ns. ROC VIH max = VCC and VIL min = 0, which are printed here only.
- Ratings:
  - V(BR)DSX min 50 V (p5). VDS abs max 50 V (p4). Feature list: "50-V Switching Capability".
  - Continuous ID per output, all on: 350 mA. Pulsed per output, all on: 1.1 A. Peak single output: 1.1 A (p4).
  - ROC pulsed drain current −1.8 A to 0.6 A (p5).
  - rDS(on) typ 1 Ω, max 1.5 Ω, at 350 mA (p5). EAS 75 mJ.
  - ICC typ 0.5 mA, max 5 mA.
- Timing (p5, TYP, VCC = 5 V, TC = 25 °C, CL = 30 pF, ID = 350 mA, RL = 68 Ω to 24 V per p6):
  - D→DRAIN tPHL = 30 ns.
  - D→DRAIN tPLH = 125 ns.
  - tr = 60 ns; tf = 30 ns.
  - No G, CLR or address delays are printed.
- VCC: 4.5–5.5 V (p5). inputMax 7 V (abs max, p4).
- Obsolete: none.
- Notes:
  - Datasheet typo on p5: "Hold time, D high **before** G↑, th". This should read "after" (the TPIC6259 and TPIC6B259 say "after").
  - The p6 test circuits print no pin numbers.

## TPIC6B259
- Title: "POWER LOGIC 8-BIT ADDRESSABLE LATCH" (p1). It has eight latches and a 3-to-8 decoder/demultiplexer with active-low DMOS outputs and a 500 mA typical current limit.
- Datasheets: datasheets/latch/tpic6b259.pdf (SLIS030, July 1995). Pages used: p1 (pinout, tables, text), p3 (logic diagram), p4 (absolute max), p5 (ROC, electrical, switching), p6–7 (Fig. 1/2).
- Families & prefixes: TPIC: TPIC6B259DW, TPIC6B259DWR, TPIC6B259DWG4, TPIC6B259DWRG4, TPIC6B259N.
- Military numbers: none.
- Pinouts: one arrangement, p20: DW (SOIC-20), N (PDIP-20). Source: p1.
  ```
  1 NC nc            11 GND ground
  2 VCC power        12 S2 input
  3 S0 input         13 G input (active low)
  4 DRAIN0 output    14 DRAIN4 output
  5 DRAIN1 output    15 DRAIN5 output
  6 DRAIN2 output    16 DRAIN6 output
  7 DRAIN3 output    17 DRAIN7 output
  8 S1 input         18 D input
  9 GND ground       19 CLR input (active low)
  10 GND ground      20 NC nc
  ```
  - "NC – No internal connection" (p1).
  - There is a single ground net, GND, on pins 9, 10 and 11. There is no LGND/PGND split.
  - Same pin positions as the TPIC6259, except: TPIC6259 pin 1/20 PGND → NC here; pin 9 LGND → GND; pins 10/11 PGND → GND.
- Behavior:
  - Identical to TPIC6259. The function table (p1) is the same. The p3 logic diagram is the same: `Ci = dec_i·NOT G`, `CLRi_n = Ci OR CLR`.
  - p1 text says this explicitly: "all other outputs are off. In the clear mode, all outputs are off ... When data is low for a given output, the DMOS-transistor output is off. When data is high, the DMOS-transistor output has sink-current capability."
  - ROC (p5): tsu 20 ns (D before G↑), th 20 ns (after G↑), tw 40 ns.
  - The 500 mA typical current limit at TC = 25 °C decreases with temperature (p1). This is analog only.
- Ratings:
  - V(BR)DSX min 50 V. Output clamp "50 V" (p1, p5). VDS abs max 50 V (p4).
  - Continuous ID per output, all on: 150 mA. Pulsed per output, all on: 500 mA. Peak single output: 500 mA (p4).
  - ROC pulsed drain current ±500 mA (p5).
  - rDS(on) typ 4.2 Ω, max 5.7 Ω, at 100 mA and VCC = 4.5 V (p5). EAS 30 mJ.
  - ICC: all outputs off 20 µA typ / 100 µA max; all outputs on 150 µA typ / 300 µA max.
- Timing (p5, TYP, VCC = 5 V, TC = 25 °C, CL = 30 pF, ID = 100 mA, RL = 235 Ω to 24 V per p6–7):
  - D→DRAIN tPLH = 150 ns.
  - D→DRAIN tPHL = 90 ns.
  - tr = 200 ns; tf = 200 ns.
- VCC: 4.5–5.5 V (p5). inputMax 7 V (abs max, p4).
- Obsolete: none.
- Notes: the TPIC6B273 has the same electrical and timing values as this part (same B-series output stage).

## TPIC6273
- Title: "POWER LOGIC OCTAL D-TYPE LATCH" (p1).
  - Despite the title, it is **edge-triggered**. p1: "contains eight positive-edge-triggered D-type flip-flops with a direct clear input".
  - Its function is the 74_273 with open-drain inverted-sense outputs.
- Datasheets: datasheets/latch/tpic6273.pdf (SLIS011A, Oct 1995). Pages used: p1 (pinout, function table, text), p2 (logic diagram), p3 (absolute max), p4 (ROC, electrical, switching), p5 (Fig. 1/2).
- Families & prefixes: TPIC: TPIC6273DW, TPIC6273DWR, TPIC6273DWG4, TPIC6273DWRG4, TPIC6273N.
- Military numbers: none.
- Pinouts: one arrangement, p20: DW (SOIC-20), N (PDIP-20). Source: p1.
  ```
  1 CLR input (active low)  11 CLK input
  2 D1 input                12 D5 input
  3 D2 input                13 D6 input
  4 DRAIN1 output           14 DRAIN5 output
  5 DRAIN2 output           15 DRAIN6 output
  6 DRAIN3 output           16 DRAIN7 output
  7 DRAIN4 output           17 DRAIN8 output
  8 D3 input                18 D7 input
  9 D4 input                19 D8 input
  10 GND ground             20 VCC power
  ```
  - There is one GND pin. All DMOS sources connect to GND internally (p3, note 2). There are no NC pins.
  - **There is no G/enable pin on this device.** Channels are numbered 1–8, not 0–7.
- Behavior:
  - Function table, per channel (p1):

    | CLR | CLK | D | DRAIN |
    |---|---|---|---|
    | L | X | X | H |
    | H | ↑ | H | L |
    | H | ↑ | L | H |
    | H | L | X | Latched |

  - p1 text: "When the clock input (CLK) is at either the high or low level, the D input signal has no effect at the output." The table only lists the L row; the text covers H as well.
  - CLR is active low and **asynchronous**: "An asynchronous CLR is provided to turn all eight DMOS-transistor outputs off." Each flip-flop has R driven from CLR (p2).
  - Priority: CLR = L overrides the clock. Otherwise CLK↑ captures D.
  - Clock: rising edge, common to all eight flip-flops. There is no clock enable and no output enable.
  - Polarity: the stored bit Q = D captured at CLK↑. DRAINn is ON (low, sinking) when Q = 1 and OFF (H) when Q = 0 (table: D = H → DRAIN = L). Clear gives Q = 0, so all outputs are off.
  - ROC (p4): tsu 10 ns (D before CLK↑), th 15 ns (after CLK↑), tw 25 ns.
  - Power-up state: not stated.
- Ratings:
  - V(BR)DSX min 45 V. "Output Clamp Voltage up to 45 V" (p1). VDS abs max 45 V (p3).
  - Continuous ID per output, all on: 250 mA (TA = 25 °C). Pulsed per output, all on: 750 mA. IDM single output: 2 A (p3).
  - ROC pulsed drain current −1.8 A to 1.5 A (p4).
  - rDS(on) typ 1.3 Ω, max 2 Ω, at 250 mA. EAS 75 mJ.
  - ICC typ 15 µA, max 100 µA.
- Timing (p4, TYP, VCC = 5 V, TC = 25 °C, CL = 30 pF, ID = 250 mA, RL = 95 Ω to 24 V per p5):
  - CLK→DRAIN tPLH = 625 ns.
  - CLK→DRAIN tPHL = 150 ns.
  - tr = 675 ns; tf = 400 ns.
  - No CLR→output delay is printed. Fig. 1 (p5) only shows CLR low forcing the output high.
- VCC: 4.5–5.5 V (p4). inputMax 7 V (abs max, p3).
- Obsolete: none.
- Notes:
  - The title says "latch" but the device is an edge-triggered register.
  - The electrical and timing figures closely match TPIC6259 (same output stage).

## TPIC6B273
- Title: "POWER LOGIC OCTAL D-TYPE LATCH" (p1). It is eight positive-edge-triggered D flip-flops with direct clear, open-drain DMOS outputs, and a 500 mA typical current limit.
- Datasheets: datasheets/latch/tpic6b273.pdf (SLIS031, July 1995). Pages used: p1, p2 (logic diagram), p3 (absolute max), p4 (ROC, electrical, switching), p5 (figures).
- Families & prefixes: TPIC: TPIC6B273DW, TPIC6B273DWR, TPIC6B273DWG4, TPIC6B273DWRG4, TPIC6B273N.
- Military numbers: none.
- Pinouts: p20: DW, N. The pinout is **identical to TPIC6273** (p1): 1 CLR, 2 D1, 3 D2, 4–7 DRAIN1–4, 8 D3, 9 D4, 10 GND, 11 CLK, 12 D5, 13 D6, 14–17 DRAIN5–8, 18 D7, 19 D8, 20 VCC. There is one GND pin, no NC pins and no G pin.
- Behavior:
  - Identical to TPIC6273: same function table (p1) and same text.
  - It adds this text (p1): "When data is low for a given output, the DMOS-transistor output is off. When data is high, the DMOS-transistor output has sink-current capability."
  - Rising-edge CLK. Asynchronous active-low CLR has priority and turns all outputs off. No enable.
  - ROC (p4): tsu 20 ns, th 20 ns, tw 40 ns.
  - The 500 mA typical current limit decreases with temperature. This is analog only.
- Ratings:
  - V(BR)DSX min 50 V. Clamp 50 V. VDS abs max 50 V.
  - Continuous ID per output, all on: 150 mA. Pulsed per output, all on: 500 mA. IDM: 500 mA (p3).
  - ROC ±500 mA (p4).
  - rDS(on) typ 4.2 Ω, max 5.7 Ω, at 100 mA. EAS 30 mJ.
  - ICC: 20 / 100 µA (off), 150 / 300 µA (on) (p4).
- Timing (p4, TYP, VCC = 5 V, TC = 25 °C, CL = 30 pF, ID = 100 mA, RL = 235 Ω to 24 V):
  - CLK→DRAIN tPLH = 150 ns.
  - CLK→DRAIN tPHL = 90 ns.
  - tr = 200 ns; tf = 200 ns.
- VCC: 4.5–5.5 V (p4). inputMax 7 V (abs max, p3).
- Obsolete: none.
- Notes: none beyond the shared notes.

