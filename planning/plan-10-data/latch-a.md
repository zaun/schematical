# latchA — transparent D-type latches (373, 533, 563, 573, 580, 841, 843, 16373, 162373, 32373, 16841, 162841, 1G373) + LS374

Page numbers are PDF page indices (pdftotext page = form-feed count + 1); paths relative to `datasheets/`.
Census = /tmp/p10/census2.json. Pin names are given without overbars; active levels are in Behavior.

## PDF → devices

| PDF | devices (families) |
|---|---|
| latch/cd74ac373.pdf | 74_373 (AC, ACT) |
| latch/cd74ac573.pdf | 74_573 (AC, ACT); also covers CD74AC563/ACT563 (74_563), but no census rows. Scan without a text layer. |
| latch/cd74hc373.pdf | 74_373 (HC) |
| latch/cd74hc533.pdf | 74_533 (HC, HCT) and 74_563 (HC, HCT) |
| latch/cd74hc573.pdf | 74_573 (HC) |
| latch/cd74hct373.pdf | 74_373 (HCT) |
| latch/cd74hct573.pdf | 74_573 (HCT) |
| latch/sn74abt162841.pdf | 74_162841 (ABT) |
| latch/sn74abt16373a.pdf | 74_16373 (ABT) |
| latch/sn74abt16841.pdf | 74_16841 (ABT) |
| latch/sn74abt373.pdf | 74_373 (ABT) |
| latch/sn74abt533a.pdf | 74_533 (ABT) |
| latch/sn74abt573a.pdf | 74_573 (ABT) |
| latch/sn74abt841a.pdf | 74_841 (ABT) |
| latch/sn74abt843.pdf | 74_843 (ABT) |
| latch/sn74ac373.pdf | 74_373 (AC) |
| latch/sn74ac533.pdf | 74_533 (AC) |
| latch/sn74ac563.pdf | 74_563 (AC) |
| latch/sn74ac573.pdf | 74_573 (AC) |
| latch/sn74act373.pdf | 74_373 (ACT) |
| latch/sn74act533.pdf | 74_533 (ACT) |
| latch/sn74act563.pdf | 74_563 (ACT) |
| latch/sn74act573.pdf | 74_573 (ACT) |
| latch/sn74ahc16373.pdf | 74_16373 (AHC) |
| latch/sn74ahc373.pdf | 74_373 (AHC) |
| latch/sn74ahc573-q1.pdf | 74_573 (AHC) |
| latch/sn74ahc573.pdf | 74_573 (AHC) |
| latch/sn74ahct16373.pdf | 74_16373 (AHCT) |
| latch/sn74ahct373.pdf | 74_373 (AHCT) |
| latch/sn74ahct573.pdf | 74_573 (AHCT) |
| latch/sn74als373a.pdf | 74_373 (ALS, AS) |
| latch/sn74als533a.pdf | 74_533 (ALS; AS533A in text, no census rows) |
| latch/sn74als573c.pdf | 74_573 (ALS, AS) |
| latch/sn74als580b.pdf | 74_580 (ALS; AS580 in text, no census rows) |
| latch/sn74als841.pdf | 74_841 (ALS; AS841A and ALS842 in text, no census rows) |
| latch/sn74als843.pdf | 74_843 (ALS) |
| latch/sn74alvch162841.pdf | 74_162841 (ALVC, H) |
| latch/sn74alvch16373.pdf | 74_16373 (ALVC, H) |
| latch/sn74alvch16841.pdf | 74_16841 (ALVC, H) |
| latch/sn74auc16373.pdf | 74_16373 (AUC) |
| latch/sn74avc16373.pdf | 74_16373 (AVC) |
| latch/sn74bct573.pdf | 74_573 (BCT) |
| latch/sn74f373.pdf | 74_373 (F) |
| latch/sn74f573.pdf | 74_573 (F) |
| latch/sn74hc373.pdf | 74_373 (HC) |
| latch/sn74hc563.pdf | 74_563 (HC) |
| latch/sn74hc573a-q1.pdf | 74_573 (HC) |
| latch/sn74hc573a.pdf | 74_573 (HC) |
| latch/sn74hcs373.pdf | 74_373 (HCS) |
| latch/sn74hcs573-q1.pdf | 74_573 (HCS) |
| latch/sn74hcs573.pdf | 74_573 (HCS) |
| latch/sn74hct373.pdf | 74_373 (HCT) |
| latch/sn74hct573.pdf | 74_573 (HCT) |
| latch/sn74ls373.pdf | 74_373 (LS, S) **and 74_374 (LS, S) — dff, joins existing device 74_374** |
| latch/sn74lv373a-q1.pdf | 74_373 (LV) |
| latch/sn74lv373a.pdf | 74_373 (LV) |
| latch/sn74lv573a.pdf | 74_573 (LV) |
| latch/sn74lvc16373a.pdf | 74_16373 (LVC) |
| latch/sn74lvc1g373.pdf | 74_1G373 (LVC) |
| latch/sn74lvc373a.pdf | 74_373 (LVC) |
| latch/sn74lvc573a-q1.pdf | 74_573 (LVC) |
| latch/sn74lvc573a.pdf | 74_573 (LVC) |
| latch/sn74lvc841a.pdf | 74_841 (LVC) |
| latch/sn74lvch16373a.pdf | 74_16373 (LVC, H) |
| latch/sn74lvch32373a.pdf | 74_32373 (LVC, H) |
| latch/sn74lvt573.pdf | 74_573 (LVT) |
| latch/sn74lvth162373.pdf | 74_162373 (LVT, H) |
| latch/sn74lvth16373.pdf | 74_16373 (LVT, H) |
| latch/sn74lvth373.pdf | 74_373 (LVT, H) |
| latch/sn74lvth573.pdf | 74_573 (LVT, H) |

### Census rows that could not be mapped
- latch/sn74ls373.pdf: the JM38510/32502{B2A,BRA,BSA,SRA,SSA} and JM38510/32503{B2A,BRA,BSA} rows, plus their M38510/ twins. The PDF covers LS373, LS374, S373 and S374, and neither the PDF nor the census mark column says which slash number is which function. They are probably one 373 and one 374 (likely 32502 = 54LS373 and 32503 = 54LS374), but this is **unverified**. All other military rows in this PDF map by name: SNJ54LS373*, SNJ54S373* → 373; SNJ54LS374*, SNJ54S374*, 7801102xx → 374.
- Every other PDF's rows map; see the per-device Military lines. The only cross-device row is 5962-8606201RA in cd74hc533.pdf, which is CD54HC563F3A → 74_563.

### BGA/VQFN packages printed in datasheets with NO census rows (informational)
GQN/ZQN 20-ball maps in sn74lvc373a p3, sn74lvc573a p3, sn74abt573a p2–3, sn74lvth573. GQL/ZQL 56-ball and GRD/ZRD 54-ball maps in several 16373 PDFs (listed under 74_16373). The census does have an orderable BGA for the 74_32373 (NMJ) and the 74_1G373 (YZP).

---

## 74_373

- Title / function: "Octal Transparent D-Type Latches With 3-State Outputs" (e.g. sn74hc373 p1). LS PDF: "Octal D-Type Transparent Latches and Edge-Triggered Flip-Flops".
- Datasheets (pages used): latch/sn74hc373.pdf (p3 pins, p4 ROC, p6 sw, p8 overview, p9 FT); latch/sn74lvc373a.pdf (p3 pins + FK/RGY/GQN, p5 ROC, p7 sw, p9 desc, p10 FT); latch/sn74ls373.pdf (p1 pins/desc, p3 FT, p6–7 LS ROC/sw, p9–10 S); latch/sn74hcs373.pdf (p3 pins incl. RKS pad, p4, p6); latch/cd74ac373.pdf (p3 pins, p4 ROC, p7–8 sw); cd74hc373.pdf (p4, p6); cd74hct373.pdf (p4, p5); sn74abt373.pdf (p3, p5); sn74ac373.pdf (p4, p6); sn74act373.pdf (p4, p6); sn74ahc373.pdf (p4, p6; RKS pad); sn74ahct373.pdf (p3 pins + pad note, p5, p7); sn74als373a.pdf (p3–6); sn74f373.pdf (p1–3); sn74hct373.pdf (p4, p5); sn74lv373a.pdf (p3 pins incl. RGY thermal pad, p5, p7); sn74lv373a-q1.pdf (p3, p5–7); sn74lvth373.pdf (p1, p3, p5).
- Families & prefixes:
  - TTL-era bipolar: LS: SN74LS373*, SN54LS373*, SNJ54LS373*. S: SN74S373*, SN54S373*, SNJ54S373*.
  - ALS: SN74ALS373A*, SN54ALS373A*, SNJ54ALS373A*. AS: SN74AS373*.
  - F: SN74F373*, SN54F373*, SNJ54F373*.
  - HC: SN74HC373*, SN54HC373*, SNJ54HC373*, SNV54HC373*, CD74HC373*, CD54HC373*.
  - HCT: SN74HCT373*, SN54HCT373*, SNJ54HCT373*, SNV54HCT373*, CD74HCT373*, CD54HCT373*.
  - HCS: SN74HCS373*.
  - AC: SN74AC373*, SN54AC373*, SNJ54AC373*, SNV54AC373*, CD74AC373*, CD54AC373*.
  - ACT: SN74ACT373*, SN54ACT373*, SNJ54ACT373*, SNV54ACT373*, CD74ACT373*, CD54ACT373*.
  - AHC: SN74AHC373*, SNJ54AHC373*. AHCT: SN74AHCT373*, SNJ54AHCT373*.
  - LV: SN74LV373A*, SN74LV373A-Q1 (census row names SN74LV373AQPWRQ1 style).
  - LVC: SN74LVC373A*, SNJ54LVC373A*.
  - LVT (LVTH, bus hold): SN74LVTH373*, SNJ54LVTH373*.
  - ABT: SN74ABT373*, SNJ54ABT373*.
- Military (from the census mark column):
  - ABT: 5962-9321801Q2A/QRA/QSA → SNJ54ABT373FK/J/W.
  - AC: 5962-87555012A / 8755501RA / 8755501SA / 8755501VSA → SNJ54AC373FK/J/W, SNV54AC373W.
  - ACT: 5962-87556012A / 8755601RA / 8755601SA / 8755601VRA → SNJ54ACT373FK/J/W, SNV54ACT373J.
  - AHC: 5962-9686601Q2A/QRA/QSA. AHCT: 5962-9686701Q2A/QRA/QSA.
  - ALS: JM38510/37203B2A/BRA (and M38510/), 83020012A, 8302001RA.
  - F: 5962-9758901Q2A/QRA/QSA, JM38510/34601B2A/BRA/BSA (and M38510/).
  - HC: 5962-8407201VRA/VSA, JM38510/65403B2A/BRA (and M38510/), 84072012A, 8407201RA, 8407201SA. CD54HC373F3A is marked 8407201RA.
  - HCT: 5962-86867012A / 8686701RA / 8686701VSA, JM38510/65453BRA/BSA (and M38510/).
  - LVC: 5962-9757301Q2A/QRA/QSA. LVTH: 5962-9950901Q2A/QRA/QSA.
  - LS/S: SNJ54LS373*, SNJ54S373* → 373. The JM38510/32502 and /32503 rows are unmapped (see top).
- Pinouts:
  - **p20**: N, D/DW, DB, NS, PW, J, W, DGV, DGS, FK, RGY*, RKS* (FK pin n = DIP pin n, no NC; drawing sn74lvc373a p3, sn74ls373 p1). *RGY/RKS have an exposed thermal pad: "can be connected to GND or left floating. Do not connect to any other signal or supply" (sn74hcs373 p3, sn74ahct373 p3; sn74lv373a p3 "Thermal Pad, normally tied to GND").
    1 OE input; 2 1Q output; 3 1D input; 4 2D input; 5 2Q output; 6 3Q output; 7 3D input; 8 4D input; 9 4Q output; 10 GND ground; 11 LE input; 12 5Q output; 13 5D input; 14 6D input; 15 6Q output; 16 7Q output; 17 7D input; 18 8D input; 19 8Q output; 20 VCC power.
  - Name variants at the same positions:
    - **LS373/S373** (sn74ls373 p1): pin 1 is `OC`, pin 11 is `C` ("C for 'LS373 and 'S373; CLK for 'LS374 and 'S374").
    - **CD74AC/ACT373** (cd74ac373 p3) number bits from 0: pin 2 `0Q`, 3 `0D`, 4 `1D`, 5 `1Q`, … 18 `7D`, 19 `7Q`, so CD nD/nQ = SN (n+1)D/(n+1)Q.
    - All other PDFs (incl. cd74hc373, cd74hct373) use the 1D/1Q names above.
  - GQN/ZQN 20-ball (sn74lvc373a p3, not in census): A1 1Q, A2 OE, A3 VCC, A4 8Q; B1 2D, B2 7D, B3 1D, B4 8D; C1 3Q, C2 2Q, C3 6Q, C4 7Q; D1 4D, D2 5D, D3 3D, D4 6D; E1 GND, E2 4Q, E3 LE, E4 5Q.
- Behavior (Function Table, sn74hc373 p9; same table in sn74ls373 p3, sn74lvc373a p10, etc.):
  | OE | LE | D | Q |
  |---|---|---|---|
  | L | H | H | H |
  | L | H | L | L |
  | L | L | X | Q0 |
  | H | X | X | Z |
  - LE (C on LS/S) is active HIGH and level-sensitive. While LE = H, Q follows D (transparent). When LE goes low, Q holds the level set up at D. There is no clock edge, no clear and no preset. All eight bits share LE and OE.
  - OE (OC on LS/S) is active LOW. It controls the 3-state buffers only: "OC does not affect the internal operation of the latches or flip-flops. That is, the old data can be retained or new data can be entered, even while the outputs are off" (sn74ls373 p1). sn74hc373 p8: "The active low output enable (OE) does not have any impact on the stored state in the latches."
  - Outputs are true (non-inverted).
  - Power-up: "At startup, Q0 is unknown" (sn74hc373 p9 FT note 3). "If the latch enable (LE) input is held low during startup, the output state of all channels is unknown until the latch enable (LE) input is driven high with valid input signals at all data (D) inputs" (sn74hc373 p8).
  - LVTH: bus hold on data inputs. 'S373: Schmitt-trigger C input (~400 mV hysteresis, sn74ls373 p1). HCS: Schmitt-trigger inputs.
- Timing per family (ns; tPLH/tPHL; single value = tpd/ten/tdis as printed):
  - LS (sn74ls373 p7, 5 V, 25 °C, **CL = 45 pF**, RL = 667 Ω, typ): D→Q 12/12; C(LE)→Q 20/18; tPZH 15, tPZL 25; tPHZ 15, tPLZ 12 (CL = 5 pF).
  - S (p10, 5 V, 25 °C, **CL = 15 pF**, RL = 280 Ω, typ): D→Q 7/7; C→Q 7/12; tPZH 8, tPZL 11; tPHZ 6, tPLZ 8 (CL = 5 pF).
  - ALS (sn74als373a p4, VCC 4.5–5.5 V, CL = 50 pF, full temp, **max**, SN74 values): D→Q 12/16; LE→Q 22/23; tPZH 18, tPZL 20; tPHZ 10, tPLZ 12. SN54: 17/19, 29/27, 22/24, 16/24.
  - AS (sn74als373a p6, **max**, SN74): D→Q 6/6; LE→Q 11.5/7.5; tPZH 6.5, tPZL 9.5; tPHZ 6.5, tPLZ 7.
  - F (sn74f373 p3, 5 V, 25 °C, typ): D→Q 4.9/3.3; LE→Q 8.6/4.8; tPZH 4.6, tPZL 5.2; tPHZ 4.1, tPLZ 3.4.
  - HC:
    - SN (sn74hc373 p6, 4.5 V, typ): D→Q 15; LE→Q 18; ten 17; tdis 15.
    - CD74HC (cd74hc373 p6, 4.5 V, **max** at 25 °C): D→Q 30; LE→Q 35; ten 30; tdis 30.
  - HCT:
    - SN (sn74hct373 p5, 4.5 V, typ): D→Q 25; LE→Q 28; ten 26; tdis 23.
    - CD74HCT (cd74hct373 p5, 4.5 V, **max** at 25 °C): D→Q 32; LE→Q 35; ten 35; tdis 35.
  - HCS (sn74hcs373 p6, 4.5 V, typ): D→Q 9.9; LE→Q 9.9; ten 7; tdis 9.
  - AC:
    - SN (sn74ac373 p6, 5 V ± 0.5 V, typ): D→Q 7/7; LE→Q 7.5/7; tPZH 7, tPZL 6.5; tPHZ 8, tPLZ 6.5.
    - CD74AC (cd74ac373 p7, 5 V, **max** −40..85 °C): D→Q 7.7; LE→Q 10.9; ten 9.5; tdis 10.5.
  - ACT:
    - SN (sn74act373 p6, typ): D→Q 8.5/8; LE→Q 8.5/8; tPZH 8, tPZL 7.5; tPHZ 9, tPLZ 7.5.
    - CD74ACT (cd74ac373 p8, **max** −40..85 °C): D→Q 9.5; LE→Q 11.4; ten 12.3; tdis 11.4.
  - AHC (sn74ahc373 p6, 5 V ± 0.5 V, 50 pF, typ): D→Q 6.5/6.5; LE→Q 6.4/6.4; ten 7; tdis 6.5.
  - AHCT (sn74ahct373 p7, typ): D→Q 5.9/5.9; LE→Q 8.5/8.5; ten 7.1; tdis 6.8.
  - ABT (sn74abt373 p5, 5 V, 25 °C, typ): D→Q 3.9/4.2; LE→Q 4.6/5.2; tPZH 3.2, tPZL 4.7; tPHZ 4.9, tPLZ 4.5.
  - LV:
    - SN74LV373A (sn74lv373a p7, 5 V ± 0.5 V, 50 pF, typ): D→Q 5.1; LE→Q 5.5; ten 5.5; tdis 4. At 3.3 V ± 0.3 V: 7.3 / 7.8 / 7.7 / 6.
    - SN74LV373A-Q1 (p7, **max** −40..125 °C, 5 V): 10.5 / 10.5 / 11.5 / 10.5.
  - LVC (sn74lvc373a p7, 3.3 V ± 0.3 V, 50 pF, **max**, SN74): D→Q 6.8; LE→Q 7.6; ten 7.7; tdis 7. SN54: 7.5 / 8.5 / 7.7 / 7.
  - LVT/LVTH (sn74lvth373 p5, 3.3 V ± 0.3 V, SN74 typ): D→Q 2.6/2.6; LE→Q 2.7/2.7; tPZH 3, tPZL 3; tPHZ 3, tPLZ 3.
- VCC per family (ROC):
  - LS: SN74 4.75–5.25 V; SN54 printed "4.5 5 5" (p6; likely 4.5–5.5).
  - S: SN74 4.75–5.25 V, SN54 4.5–5.5 V (p9).
  - ALS and AS: 4.5–5.5 V (both). F: 4.5–5.5 V. ABT: 4.5–5.5 V (both), VI ≤ VCC.
  - HC and HCS: 2–6 V (SN and CD).
  - HCT, ACT, AHCT: 4.5–5.5 V. AHCT has inputMax 5.5 V.
  - AC: SN 2–6 V (both); CD74AC 1.5–5.5 V (cd74ac373 p4).
  - AHC: 2–5.5 V, inputMax 5.5 V. LV (both PDFs): 2–5.5 V, inputMax 5.5 V.
  - LVC: SN74 1.65–3.6 V, SN54 2–3.6 V, inputMax 5.5 V.
  - LVTH: 2.7–3.6 V (both), inputMax 5.5 V.
- Obsolete (census, 373 only): CD74AC373M, CD74ACT373M, CD74HC373M, CD74HCT373M, SN74AC373PW, SN74ACT373DW, SN74ACT373PW, SN74AHC373DW, SN74AHC373PW, SN74AHCT373DW, SN74AHCT373PW, SN74ALS373ADW, SN74F373DW, SN74HC373DW, SN74HC373PW, SN74HCT373DW, SN74HCT373PW, SN74LS373DW, SN74LV373ADW, SN74LV373APW, SN74LV373APWT.
- Notes:
  - The repo's dff 74_374 uses `OE`/`CLK`. For the latch, use `OE`/`LE` and treat LS `OC`/`C` as aliases. The CD74AC373 0-based names are also only a naming difference.
  - The cd74ac373 switching tables (p7–8) also print "533" rows; that PDF has no 533 census rows.
  - Many sn74ac/act/hc/lv PDFs print NRND/Last-buy rows ("N"/"L" in census); these are not obsolete.

## 74_374 — dff, joins existing device 74_374

- Source: latch/sn74ls373.pdf (p1 pinout/description, p3 FT, p6–7 LS, p9–10 S). The families here are new for the existing device: LS (SN74LS374*, SN54LS374J, SNJ54LS374*) and S (SN74S374N, SN54S374J, SNJ54S374*).
- Military: 7801102RA / 7801102SA / 78011022A = SNJ54LS374J / W / FK (census mark). The JM38510/32502–32503 rows are unmapped (see top).
- Pinout p20 (N, DW, DB, NS, J, W, FK; FK pin n = DIP n): identical to the existing 74_374 p20. The LS sheet's names are `OC` (pin 1) and `CLK` (pin 11), i.e. OE → `OC`.
- Behavior (FT p3): OC=L, CLK↑, D=H → H; OC=L, CLK↑, D=L → L; OC=L, CLK=L → Q0; OC=H → Z. Positive-edge triggered. OC gates only the outputs; the flip-flops still load while OC = H (p1). 'S374 has a Schmitt-trigger CLK input.
- Timing:
  - LS374 (p7, 5 V, 25 °C, CL = 45 pF, typ): fmax 50 MHz (min 35); CLK→Q tPLH 15 / tPHL 19; tPZH 20, tPZL 21; tPHZ 15, tPLZ 12 (CL = 5 pF). Requirements (p6): tsu 20 ns, th 5 (SN54) / 0 (SN74), tw 15.
  - S374 (p10, CL = 15 pF, typ): fmax 100 MHz (min 75); CLK→Q 8 / 11; tPZH 8, tPZL 11; tPHZ 5, tPLZ 7. tsu 5, th 2, tw 6 (high) / 7.3 (low).
- VCC: LS SN74 4.75–5.25 V, SN54 4.5–(5.5) V. S SN74 4.75–5.25 V, SN54 4.5–5.5 V.
- Obsolete: SN74LS374DW.

## 74_573

- Title / function: "Octal Transparent D-Type Latches With 3-State Outputs". Same logic as the 373, but with a flow-through pinout (all D on one side, all Q on the other).
- Datasheets (pages): latch/sn74hc573a.pdf (p3 pins, p4 ROC, p7 sw, p11 FT/desc); latch/sn74lvc573a.pdf (p3 pins + GQN, p5, p7); cd74ac573.pdf (scanned, p2 pins/ROC, p5–6 sw); cd74hc573.pdf (p3, p4, p6); cd74hct573.pdf (p3, p4, p5); sn74abt573a.pdf (p1–3, p5); sn74ac573.pdf (p3, p4, p6, p8 FT); sn74act573.pdf (p5, p7); sn74ahc573.pdf (p5, p7); sn74ahc573-q1.pdf (p4, p6); sn74ahct573.pdf (p3, p4, p6); sn74als573c.pdf (p1–3 ALS, p4–5 AS); sn74bct573.pdf (p3, p4); sn74f573.pdf (p2, p3); sn74hc573a-q1.pdf (p4, p6); sn74hcs573.pdf and sn74hcs573-q1.pdf (p3, p4, p6); sn74hct573.pdf (p4, p5); sn74lv573a.pdf (p4, p7–8); sn74lvc573a-q1.pdf (p3, p4); sn74lvt573.pdf (p3, p5); sn74lvth573.pdf (p4, p6).
- Families & prefixes:
  - ALS: SN74ALS573C*, SN54ALS573C*, SNJ54ALS573C*. AS: SN74AS573A*, SN54AS573AJ, SNJ54AS573AJ.
  - F: SN74F573*. BCT: SN74BCT573*. ABT: SN74ABT573A*, SNJ54ABT573*.
  - HC: SN74HC573A*, SNJ54HC573A*, SNV54HC573AJ, SN74HC573A-Q1, CD74HC573*, CD54HC573*.
  - HCT: SN74HCT573*, CD74HCT573*, CD54HCT573*.
  - HCS: SN74HCS573*, SN74HCS573-Q1.
  - AC: SN74AC573*, CD74AC573*, CD54AC573*.
  - ACT: SN74ACT573*, SNJ54ACT573*, CD74ACT573*, CD54ACT573*.
  - AHC: SN74AHC573*, SNJ54AHC573*, SN74AHC573-Q1.
  - AHCT: SN74AHCT573*, SNJ54AHCT573*.
  - LV: SN74LV573A*.
  - LVC: SN74LVC573A*, SNJ54LVC573A*, SN74LVC573A-Q1 (census names CLVC573AQ…).
  - LVT: SN74LVT573*. LVT (LVTH, bus hold): SN74LVTH573*, SNJ54LVTH573*.
- Military (census mark):
  - ABT: 5962-9321901Q2A/QRA/QSA → SNJ54ABT573FK/J/W.
  - ACT: 5962-87664012A / 8766401RA / 8766401SA → SNJ54ACT573FK/J/W.
  - AHC: 5962-9685601Q2A/QRA/QSA. AHCT: 5962-9685501QRA/QSA.
  - ALS: JM38510/38201B2A/BRA (and M38510/), 84012012A / 8401201RA / 8401201SA → SNJ54ALS573CFK/J/W. AS: SNJ54AS573AJ.
  - HC: 5962-8512801VRA → SNV54HC573AJ; JM38510/65406BRA (and M38510/); 85128012A / 8512801RA / 8512801SA → SNJ54HC573AFK/J/W.
  - HCT: 5962-8685601RA → CD54HCT573F3A.
  - LVC: 5962-9757501Q2A/QRA/QSA. LVTH: 5962-9583101Q2A/QRA/QSA.
- Pinouts:
  - **p20**: N, DW, DB, NS, PW, J, W, DGV, DGS, FK, RGY*, RKS* (FK pin n = DIP pin n, no NC). *RGY/RKS have an exposed thermal pad (GND or floating; sn74ac573/sn74hcs573 pin tables). Pins (sn74hc573a p3):
    1 OE input; 2 1D input; 3 2D input; 4 3D input; 5 4D input; 6 5D input; 7 6D input; 8 7D input; 9 8D input; 10 GND ground; 11 LE input; 12 8Q output; 13 7Q output; 14 6Q output; 15 5Q output; 16 4Q output; 17 3Q output; 18 2Q output; 19 1Q output; 20 VCC power.
  - Name variants at the same positions:
    - cd74ac573 (scan, p2): `D0..D7` on pins 2–9 and `Q0`(19)..`Q7`(12), 0-based. Its drawing puts an overbar on pin 11 LE; the text and truth table say LE is active high, so the overbar is a drawing error.
    - sn74hcs573 and sn74hcs573-q1 pin tables: `D1..D8` / `Q1..Q8`, with Q1 on pin 19.
  - GQN/ZQN 20-ball (sn74lvc573a p3; also abt573a, lvth573; no census rows): A1 1D, A2 OE, A3 VCC, A4 1Q; B1 3D, B2 3Q, B3 2D, B4 2Q; C1 5D, C2 4D, C3 5Q, C4 4Q; D1 7D, D2 7Q, D3 6D, D4 6Q; E1 GND, E2 8D, E3 LE, E4 8Q.
- Behavior: identical to 74_373 (Function Table "Each Latch", sn74hc573a p11): OE=L, LE=H → Q = D; OE=L, LE=L → Q0; OE=H → Z (Hi-Z). LE is active HIGH. OE is active LOW and gates outputs only ("Old data can be retained or new data can be entered while the outputs are in the high-impedance state", sn74hc573a p11). True outputs. Startup state is unknown (the 373 wording applies; no specific 573 statement). HCS has Schmitt inputs; LVTH has bus hold.
- Timing per family (ns):
  - ALS (sn74als573c p3, **max**, SN74): D→Q 14/14; LE→Q 20/19; tPZH 18, tPZL 18; tPHZ 10, tPLZ 15.
  - AS (p5, **max**, SN74): D→Q 8/7; LE→Q 13/7.5; tPZH 6.5, tPZL 9.5; tPHZ 6.5, tPLZ 7.
  - F (sn74f573 p3, typ): D→Q 4.9/3.3; LE→Q 8.6/4.8; tPZH 4.6, tPZL 5.2; tPHZ 4.1, tPLZ 3.4.
  - BCT (sn74bct573 p4, typ): D→Q 5/5.9; LE→Q 6.1/5.2; tPZH 6.2, tPZL 7.1; tPHZ 3.9, tPLZ 3.6.
  - ABT (sn74abt573a p5, typ): D→Q 3.2/4.2; LE→Q 4.0/5.2; tPZH 3.2, tPZL 4.7; tPHZ 4.9, tPLZ 4.2.
  - HC:
    - SN74HC573A and -Q1 (p7 / p6, 4.5 V, typ): D→Q 26; LE→Q 27; ten 24; tdis 23.
    - CD74HC (cd74hc573 p6, **max** 25 °C): 35 / 35 / 30 / 30.
  - HCT:
    - SN (sn74hct573 p5, typ): D→Q 25; LE→Q 28; ten 26; tdis 23.
    - CD74HCT (cd74hct573 p5, **max** 25 °C): 35 / 35 / 35 / 35.
  - HCS (p6, typ, both PDFs): D→Q 9.9; LE→Q 9.9; ten 7; tdis 9.
  - AC:
    - SN74AC573 (p6, **max** 25 °C, 5 V): D→Q 10/9.5; LE→Q 9.5/8.5; tPZH 9, tPZL 8.5; tPHZ 11, tPLZ 8.
    - CD74AC (p5, **max** −40..85 °C): D→Q 7.7; LE→Q 10.9; ten 9.5; tdis 10.5.
  - ACT:
    - SN74ACT573 (p7, typ): D→Q 6/6; LE→Q 6/5.5; tPZH 5.5, tPZL 5.5; tPHZ 6.5, tPLZ 5.
    - CD74ACT (p6, **max**): D→Q 9.4; LE→Q 11.4; ten 12.3; tdis 11.4.
  - AHC (p7, and -Q1 p6, typ 50 pF): D→Q 6; LE→Q 6.5; ten 6.7; tdis 6.7.
  - AHCT (p6, typ): D→Q 6.1; LE→Q 6.6; ten 6.5; tdis 6.7.
  - LV (sn74lv573a p8, 5 V, typ): D→Q 5.3; LE→Q 5.7; ten 5.7; tdis 4.2. At 3.3 V: 7.7 / 8.2 / 8 / 6.2.
  - LVC (sn74lvc573a p7, 3.3 V, **max**): D→Q 6.9; LE→Q 7.7; ten 7.5; tdis 6.5 (SN54 6.7). The -Q1 is the same (tdis 6.7).
  - LVT (sn74lvt573 p5, typ): D→Q 2.5/2.7; LE→Q 3.5/4.3; tPZH 2.8, tPZL 3.3; tPHZ 3.7, tPLZ 3.0.
  - LVTH (sn74lvth573 p6, typ): D→Q 2.6/2.9; LE→Q 2.9/2.9; tPZH 3.2, tPZL 3.9; tPHZ 3.5, tPLZ 3.2.
- VCC per family:
  - ALS, AS, F, BCT, ABT, HCT, ACT: 4.5–5.5 V (SN54 = SN74).
  - HC and HCS: 2–6 V.
  - AC: SN 2–6 V; CD74AC 1.5–5.5 V.
  - AHC: 2–5.5 V, inputMax 5.5 V. AHCT: 4.5–5.5 V, inputMax 5.5 V. LV: 2–5.5 V, inputMax 5.5 V.
  - LVC: SN74 1.65–3.6 V, SN54 and -Q1 2–3.6 V, inputMax 5.5 V.
  - LVT and LVTH: 2.7–3.6 V, inputMax 5.5 V.
- Obsolete: CD74AC573M, CD74ACT573M, CD74HC573M, CD74HCT573M, SN74AC573DW, SN74AC573PW, SN74ACT573DW, SN74ACT573PW, SN74AHC573DW, SN74AHC573PW, SN74AHCT573DW, SN74AHCT573PW, SN74ALS573CDW, SN74F573DW, SN74HC573ADW, SN74HC573APWT, SN74HCT573DW, SN74HCT573PW, SN74LV573ADW, SN74LV573APW.
- Notes:
  - cd74ac573.pdf has no text layer; its numbers were read from page renders. It also prints AC563/ACT563 data: AC563 D→Q 9.5 (−40..85 max), and ACT563 D→Q 10.4. There are no census rows for these.
  - sn74als573c.pdf contains two datasheets (ALS573C, then AS573A from p4).
  - SN54HCT573 is marked "product preview" (sn74hct573 p5).

---


### Pin-naming note for the inverting octals (533 / 563 / 580)

TI draws the outputs of these parts with an overbar (seen directly in the SN74HC563 pinout image, p3:
`1Q̄ … 8Q̄`); pdftotext drops the overbar in every other PDF, so the text shows `1Q … 8Q` (SN) or `Q0 … Q7`
(CD). No part has a true Q beside the Q̄, so the pin tables below use the printed names **without** the
overbar (`1Q`…`8Q`) per the parent's rule "BAR suffix only on collision". The repo does have a precedent
for a BAR suffix with no collision (jkff.json `1KBAR` for the 74x109 K̄ input), so the planner may prefer
`1QBAR`…`8QBAR`; the inversion is stated in each Behavior section either way.

CD74HC/HCT533/563 (Harris-derived) number the bits from 0: `D0…D7`, `Q0…Q7`. SN parts use `1D…8D`, `1Q…8Q`.
Map: CD `Dn`/`Qn` = SN `(n+1)D`/`(n+1)Q`. Same pin positions, so they are the same pinout. Only the names differ.

### FK (20-pin LCCC) rule used below

Every 20-pin FK drawing in these PDFs uses the same pin numbers as the DIP (FK pin n = DIP pin n,
no NC). The drawings checked are SN54ABT533 p1, SN54AC533/ACT533 p1, SN54AC563/ACT563 p1,
SN54HC563 p3 (image), and SN54ALS580B p1. The 28-pin FK for the 24-pin parts (841/843) has NC at 1, 8, 15 and 22.
That map is FK 2–7 = DIP 1–6, FK 9–14 = DIP 7–12, FK 16–21 = DIP 13–18, FK 23–28 = DIP 19–24.
I checked it against every label in the SN54ABT841 and SN54ABT843 FK drawings, p1.

---

## 74_533

- Title: "Octal Transparent D-Type Latches With 3-State Outputs" (SN74ABT533A p1). The SN74ALS533A/AS533A
  PDF prints "Octal D-Type Transparent Latches With 3-State Outputs". The CD PDF prints "High-Speed CMOS
  Logic Octal Inverting Transparent Latch, Three-State Outputs" (p1). Function: 8 transparent latches,
  common LE and OE, **inverting** 3-state outputs. This is the 373 pinout with Q inverted.
- Datasheets:
  - latch/cd74hc533.pdf (SCHS187C): p1 ordering, p2 pinout and truth table, p3 operating conditions, p5–6 switching. This PDF also covers HC563/HCT563 (see 74_563).
  - latch/sn74abt533a.pdf (SCBS186D): p1 pinout and FK, p2 function table, p3 rec-op, p4 SN54 switching, p5 SN74 switching.
  - latch/sn74ac533.pdf (SCAS555C): p1 pinout and FK, p2 function table, p3 rec-op, p4 switching.
  - latch/sn74act533.pdf (SCAS553C): p1, p2, p3, p4.
  - latch/sn74als533a.pdf (SDAS270): p1 pinout and function table, p3 ALS rec-op, p4 ALS switching and AS rec-op, p5 AS switching.
- Families and prefixes:
  - HC: CD54HC533F3A, CD74HC533E.
  - HCT: CD54HCT533F3A, CD74HCT533E.
  - ABT: SN74ABT533A*, SN54ABT533 / SNJ54ABT533*.
  - AC: SN74AC533* (SN54AC533 appears in the drawings only).
  - ACT: SN74ACT533* (the SN54ACT533 columns are marked PRODUCT PREVIEW, p3–4).
  - ALS: SN74ALS533A*.
  - AS: SN74AS533A. It is in the PDF but has **no census rows**.
- Military:
  - cd74hc533: `5962-8681301RA` → HC533 (mark CD54HC533F3A); `CD54HC533F3A` → HC533. `5962-8606201RA` → **HC563, not 533** (mark CD54HC563F3A).
  - sn74abt533a: `5962-9584301Q2A` and `SNJ54ABT533FK` → ABT533 FK; `5962-9584301QRA` and `SNJ54ABT533J` → ABT533 J.
- Pinouts:
  - **p20** for N, DW, DB, PW, NS, J, W (PDIP/SOIC/SSOP/TSSOP/SOP/CDIP/CFP) and FK (LCCC-20, same numbering). Sources: ABT533A p1, AC533 p1, ACT533 p1, ALS533A p1, CD p2.

    | Pin | Name | Role | CD name |
    |---|---|---|---|
    | 1 | OE | input | |
    | 2 | 1Q | output | Q0 |
    | 3 | 1D | input | D0 |
    | 4 | 2D | input | D1 |
    | 5 | 2Q | output | Q1 |
    | 6 | 3Q | output | Q2 |
    | 7 | 3D | input | D2 |
    | 8 | 4D | input | D3 |
    | 9 | 4Q | output | Q3 |
    | 10 | GND | ground | |
    | 11 | LE | input | |
    | 12 | 5Q | output | Q4 |
    | 13 | 5D | input | D4 |
    | 14 | 6D | input | D5 |
    | 15 | 6Q | output | Q5 |
    | 16 | 7Q | output | Q6 |
    | 17 | 7D | input | D6 |
    | 18 | 8D | input | D7 |
    | 19 | 8Q | output | Q7 |
    | 20 | VCC | power | |

    The CD PDF prints these as `Qn` and `Dn` (n = 0–7) in the same positions.
- Behavior:
  - Function table, the same in every PDF (ABT533A p2, AC533 p2, ACT533 p2, ALS533A p1, CD p2):

    | OE | LE | D | Q |
    |---|---|---|---|
    | L | H | H | L |
    | L | H | L | H |
    | L | L | X | Q0 |
    | H | X | X | Z |

  - LE is active **high** and level-sensitive. While LE = H, nQ = NOT nD (transparent, inverted). On LE↓ the latch stores D; the CD table shows `l`/`h` = level one setup time before the LE high→low transition. While LE = L the output holds Q0, which is already the inverted level.
  - Simulation model: store `latch = D` while LE = H; drive `Q = NOT latch`.
  - OE is active **low**, a single common pin for all 8 outputs. OE = H puts every output in high-Z.
  - "OE does not affect the internal operations of the latches. Previously stored data can be retained or new data can be entered while the outputs are in the high-impedance state." (ABT533A p1; same wording in AC/ACT/ALS.) So OE gates the outputs only.
  - No clear or preset. No power-up state is given. ABT p1 says to tie OE to VCC through a pullup to keep high-Z during power up and down.
- Timing:
  - HC (CD p5): CL = 50 pF, VCC = 4.5 V. Only a 25 °C MAX is printed at 50 pF; the typicals are at 5 V, CL = 15 pF.

    | Path | Max @ 4.5 V, 50 pF | Typ @ 5 V, 15 pF |
    |---|---|---|
    | D→Q (tPLH/tPHL) | 33 ns | 13 ns |
    | LE→Q | 35 ns | 14 ns |
    | OE enable tPZH/tPZL (p6) | 30 ns | 12 ns |
    | OE disable tPHZ/tPLZ (p6) | 30 ns | 12 ns |

  - HCT (CD p6): 4.5 V, 50 pF, 25 °C MAX; typicals at 5 V, 15 pF.

    | Path | Max @ 4.5 V, 50 pF | Typ @ 5 V, 15 pF |
    |---|---|---|
    | D→Q | 34 ns | 14 ns |
    | LE→Q | 38 ns | 16 ns |
    | OE enable | 35 ns | 14 ns |
    | OE disable | 30 ns | 12 ns |

    The disable row is printed with the symbol "tTLH, tTHL"; see Notes.
  - ABT: SN74ABT533A p5, VCC = 5 V, 25 °C, CL = 50 pF, TYP.

    | Path | Typ |
    |---|---|
    | D→Q tPLH / tPHL | 4.2 / 4.9 ns |
    | LE→Q tPLH / tPHL | 4.9 / 5.4 ns |
    | tPZH / tPZL | 3.7 / 4.2 ns |
    | tPHZ / tPLZ | 5.1 / 4.1 ns |

    The SN54ABT533 typicals on p4 are the same.
  - AC: p4, VCC = 5 V ± 0.5 V, 25 °C. No TYP column, so these are MAX.

    | Path | Max |
    |---|---|
    | D→Q tPLH / tPHL | 10 / 9.5 ns |
    | LE→Q | 10.5 / 10 ns |
    | tPZH / tPZL | 9.5 / 9.5 ns |
    | tPHZ / tPLZ | 10 / 10 ns |

    The 3.3 V table is on the same page; for example D→Q is 14 / 13 ns max at 25 °C.
  - ACT: p4, 5 V ± 0.5 V, 25 °C MAX (no typ).

    | Path | Max |
    |---|---|
    | D→Q | 10.5 / 10 ns |
    | LE→Q | 10.5 / 10.5 ns |
    | tPZH / tPZL | 10 / 10 ns |
    | tPHZ / tPLZ | 10 / 10 ns |

  - ALS: p4, VCC 4.5–5.5 V, TA = MIN to MAX. Only min/max are printed, so these are MAX.

    | Path | Max |
    |---|---|
    | D→Q | 19 / 13 ns |
    | LE→Q | 23 / 18 ns |
    | tPZH / tPZL | 17 / 18 ns |
    | tPHZ / tPLZ | 10 / 16 ns |

  - AS (no orderable rows): p5, MAX.

    | Path | Max |
    |---|---|
    | D→Q | 7.5 / 7 ns |
    | LE→Q | 9 / 8 ns |
    | tPZH / tPZL | 6.5 / 9.5 ns |
    | tPHZ / tPLZ | 6.5 / 7 ns |

- VCC:
  - HC: 2–6 V (CD p3).
  - HCT: 4.5–5.5 V (CD p3).
  - ABT: 4.5–5.5 V, the same for SN54 and SN74 (p3). VI is 0–VCC; no inputMax.
  - AC: 2–6 V for SN54 and SN74 (p3). VI is 0–VCC. p1 says "Inputs accept voltages to 6 V", which is the absolute range, not above-VCC tolerance.
  - ACT: 4.5–5.5 V (p3).
  - ALS: 4.5–5.5 V (p3).
  - AS: 4.5–5.5 V (p4).
- Obsolete: SN74ACT533PW.
  - Last Time Buy: SN74AC533DW (the only AC533 row) and SN74ALS533ANSR.
- Notes:
  - CD p6 HCT block has mislabeled rows; the parameter names are shifted by one row:
    - The "LE to Qn (HC/HCT563)" row carries the symbol "tPZL, tPZH".
    - The "Enable Times (HC/HCT533)" row carries "tPLZ, tPZH".
    - The "Disable Times (HC/HCT533)" row carries "tTLH, tTHL".

    I read these as LE→Q (563) = 35, enable (533) = 35, disable (533) = 30 ns max.
  - CD p5 HCT prerequisite table labels the setup time with the symbol "tw". It is tSU.
  - SN74AC533 has a single census row and it is Last Time Buy.

## 74_563

- Title: "Octal D-Type Transparent Latches With 3-State Outputs" (AC563 and ACT563 p1). SN74HC563 p1 prints "SNx4HC563 Octal Transparent D-Type Lataches [sic] With 3-State Outputs". Function: the 573 flow-through pinout with **inverting** outputs.
- Datasheets:
  - latch/cd74hc533.pdf: HC563 and HCT563 parts; p2 pinout and truth table, p5–6 timing.
  - latch/sn74ac563.pdf (SCAS552C): p1, p2 function table, p3 rec-op, p4 switching.
  - latch/sn74act563.pdf (SCAS550B): p1, p2, p3, p4.
  - latch/sn74hc563.pdf (SCLS145E): p3 pinout images, p4 rec-op, p6 switching, p8 function table.
- Families:
  - HC: CD54HC563F3A, CD74HC563E (CD PDF); SN74HC563*, SN54HC563 (SN PDF).
  - HCT: CD74HCT563E, CD74HCT563M.
  - AC: SN74AC563*.
  - ACT: SN74ACT563*.
- Military: `5962-8606201RA` and `CD54HC563F3A` → HC563 (cd74hc533.pdf). There are no others.
- Pinouts:
  - **p20** for N, DW, DB, PW, NS, J, W and FK (same numbering). Sources: AC563 p1, ACT563 p1, CD p2, HC563 p3 image.

    | Pin | Name | Role | CD name |
    |---|---|---|---|
    | 1 | OE | input | |
    | 2 | 1D | input | D0 |
    | 3 | 2D | input | D1 |
    | 4 | 3D | input | D2 |
    | 5 | 4D | input | D3 |
    | 6 | 5D | input | D4 |
    | 7 | 6D | input | D5 |
    | 8 | 7D | input | D6 |
    | 9 | 8D | input | D7 |
    | 10 | GND | ground | |
    | 11 | LE | input | |
    | 12 | 8Q | output | Q7 |
    | 13 | 7Q | output | Q6 |
    | 14 | 6Q | output | Q5 |
    | 15 | 5Q | output | Q4 |
    | 16 | 4Q | output | Q3 |
    | 17 | 3Q | output | Q2 |
    | 18 | 2Q | output | Q1 |
    | 19 | 1Q | output | Q0 |
    | 20 | VCC | power | |

    The SN74HC563 pinout (p3) is an **image only**; pdftotext gives just the headings. I extracted the image and read it: it is the same map, with an overbar on every Q. The FK image on the same page matches the DIP numbering, and pin 11 is LE.
- Behavior:
  - Function table (AC563 p2, ACT563 p2, HC563 p8, CD p2):

    | OE | LE | D | Q |
    |---|---|---|---|
    | L | H | H | L |
    | L | H | L | H |
    | L | L | X | Q0 |
    | H | X | X | Z |

  - LE is active high and transparent. Q = NOT D while LE = H, and the inverted level is latched on LE↓.
  - OE is active low and common to all 8 outputs. "OE does not affect internal operations of the latches. Old data can be retained or new data can be entered while the outputs are in the high-impedance state." (HC563 p8; AC563 and ACT563 p1.)
  - There is no clear or preset.
- Timing:
  - HC, SN74HC563 p6: 4.5 V, CL = 50 pF, 25 °C TYP.

    | Path | Typ |
    |---|---|
    | D→Q tpd | 26 ns |
    | LE→Q tpd | 27 ns |
    | ten (OE) | 24 ns |
    | tdis (OE) | 23 ns |

    The CD HC563 figures (p5–6) at 4.5 V, 50 pF are MAX only (typicals at 5 V, 15 pF):

    | Path | Max | Typ @ 15 pF |
    |---|---|---|
    | D→Q | 30 ns | 12 ns |
    | LE→Q | 33 ns | 13 ns |
    | OE enable and disable | 30 ns | 12 ns |

  - HCT (CD p6): 4.5 V, 50 pF MAX.

    | Path | Max | Typ @ 5 V, 15 pF |
    |---|---|---|
    | D→Q | 30 ns | 12 ns |
    | LE→Q | 35 ns | 14 ns |
    | OE enable and disable | 35 ns | 14 ns |

  - AC: p4, 5 V ± 0.5 V, 25 °C TYP.

    | Path | Typ |
    |---|---|
    | D→Q tPLH / tPHL | 5.3 / 5.6 ns |
    | LE→Q | 4.6 / 4.8 ns |
    | tPZH / tPZL | 5.3 / 5.4 ns |
    | tPHZ / tPLZ | 6 / 5.1 ns |

    The 5 V MAX values are D→Q 10 / 9.5 and LE→Q 9.5 / 8.5 ns.
  - ACT: p4, 5 V ± 0.5 V TYP.

    | Path | Typ |
    |---|---|
    | D→Q | 7 / 6 ns |
    | LE→Q | 6.5 / 5.5 ns |
    | tPZH / tPZL | 5.5 / 5.5 ns |
    | tPHZ / tPLZ | 6.5 / 4.5 ns |

- VCC:
  - HC: 2–6 V for SN54 and SN74 (HC563 p4; CD p3).
  - HCT: 4.5–5.5 V.
  - AC: 2–6 V.
  - ACT: 4.5–5.5 V.
  - No inputMax: VI is 0–VCC in every PDF.
- Obsolete: SN74AC563PW, SN74ACT563DW, SN74ACT563PW, SN74HC563DW.
  - Last Time Buy: SN74AC563DW, SN74AC563N, SN74AC563PWR, SN74ACT563N.
- Notes:
  - The AC563 and ACT563 FK drawings (p1) label pin 11 "**CLK**". This is a drawing error: the DIP drawing and the function text call it LE.
  - The AC563 typicals in the 5 V table are identical to the 3.3 V table's typicals (p4). This is probably a copy error; treat the 5 V typicals with caution.
  - CD p5 gives the HC563 hold time as 4 ns at every VCC, and the 533 as 35/7/6 ns.

## 74_580

- Title: "Octal D-Type Transparent Latches With 3-State Outputs" (SN54ALS580B, SN74ALS580B, SN74AS580; SDAS277, p1). Features: "Inverting-Logic Outputs", "Bus-Structured Pinout".
- Datasheet: latch/sn74als580b.pdf: p1 pinout, FK and function table; p2 ALS rec-op; p3 ALS switching; p4 AS rec-op; p5 AS switching.
- Families:
  - ALS: SN74ALS580B*, SN54ALS580B*, SNJ54ALS580B*.
  - AS: SN74AS580. It is in the PDF but has **no census rows**.
- Military, all ALS580B:
  - `84012022A` and `SNJ54ALS580BFK` → FK.
  - `8401202RA` and `SNJ54ALS580BJ` → J.
  - `8401202SA` and `SNJ54ALS580BW` → W.
  - Also `SN54ALS580BJ`.
- Pinouts:
  - **p20** for N, DW, J, W and FK (same numbering, checked against the p1 FK drawing). It is **identical to the 74_563 pinout**:
    - 1 OE (input)
    - 2–9 = 1D–8D (inputs)
    - 10 GND (ground)
    - 11 LE (input)
    - 12 = 8Q, 13 = 7Q, 14 = 6Q, 15 = 5Q, 16 = 4Q, 17 = 3Q, 18 = 2Q, 19 = 1Q (outputs)
    - 20 VCC (power)
- Behavior:
  - Function table (p1):

    | OE | LE | D | Q |
    |---|---|---|---|
    | L | H | H | L |
    | L | H | L | H |
    | L | L | X | Q0 |
    | H | X | X | Z |

  - The outputs are **inverted** (Q = NOT D while LE = H). LE is active high. OE is active low, common, and gates the outputs only: "OE does not affect internal operations of the latches" (p1).
  - The p1 description text says "outputs (Q) respond to the data (D) inputs" without saying "complement". The function table and the "Inverting-Logic Outputs" feature are authoritative.
  - **This is functionally identical to 74_563.** The planner could model 74_580 as an alias of 74_563.
- Timing: the datasheet prints no typical, so these are MAX over TA MIN to MAX at VCC 4.5–5.5 V.

  | Path | SN74ALS580B (p3) | SN54ALS580B (p3) | SN74AS580 (p5, no rows) |
  |---|---|---|---|
  | D→Q tPLH / tPHL | 18 / 14 ns | 26 / 15 ns | 7.5 / 7 ns |
  | LE→Q | 22 / 21 ns | 29 / 22 ns | 9 / 8 ns |
  | tPZH / tPZL | 18 / 18 ns | 25 / 21 ns | 6.5 / 9.5 ns |
  | tPHZ / tPLZ | 10 / 15 ns | 12 / 22 ns | 6.5 / 7 ns |

- VCC: ALS 4.5–5.5 V for SN54 and SN74 (p2). AS 4.5–5.5 V (p4).
- Obsolete: none.

## 74_841

- Title: "10-Bit Bus-Interface D-Type Latches With 3-State Outputs" (ABT841A p1; ALS841 p1). LVC841A prints "10-Bit Bus-Interface D-Type Latch With 3-State Outputs". Function: 10 transparent latches, true (non-inverted) outputs, one LE, one OE.
- Datasheets:
  - latch/sn74abt841a.pdf (SCBS196D): p1 pinout and FK-28, p2 function table, p3 rec-op, p5 switching.
  - latch/sn74als841.pdf (SDAS059C): p1 pinout, p2 function tables, p4 ALS rec-op, p5 ALS and ALS842 switching, p6 AS841A rec-op. This PDF also covers SN74AS841A and **SN74ALS842**; see Notes.
  - latch/sn74lvc841a.pdf (SCAS307J): p1 pinout, p2 function table, p3 rec-op, p4 switching.
- Families:
  - ABT: SN74ABT841A*, SN54ABT841 / SNJ54ABT841*.
  - ALS: SN74ALS841*.
  - AS: SN74AS841A (no census rows).
  - LVC: SN74LVC841A*.
- Military: `5962-9676901QLA` and `SNJ54ABT841JT` → ABT841 (JT).
- Pinouts:
  - **p24** for DW, DB, PW, DGV, NT, JT, W (ABT p1, ALS p1, LVC p1):
    - 1 OE (input)
    - 2–11 = 1D–10D (inputs)
    - 12 GND (ground)
    - 13 LE (input)
    - 14 = 10Q, 15 = 9Q, 16 = 8Q, 17 = 7Q, 18 = 6Q, 19 = 5Q, 20 = 4Q, 21 = 3Q, 22 = 2Q, 23 = 1Q (outputs)
    - 24 VCC (power)
  - **fk28** (SN54ABT841 FK, ABT p1; no census rows have FK):

    | Pin | Name | Pin | Name | Pin | Name | Pin | Name |
    |---|---|---|---|---|---|---|---|
    | 1 | NC | 8 | NC | 15 | NC | 22 | NC |
    | 2 | OE | 9 | 6D | 16 | LE | 23 | 5Q |
    | 3 | 1D | 10 | 7D | 17 | 10Q | 24 | 4Q |
    | 4 | 2D | 11 | 8D | 18 | 9Q | 25 | 3Q |
    | 5 | 3D | 12 | 9D | 19 | 8Q | 26 | 2Q |
    | 6 | 4D | 13 | 10D | 20 | 7Q | 27 | 1Q |
    | 7 | 5D | 14 | GND | 21 | 6Q | 28 | VCC |

    NC pins have role nc; the roles of the other pins follow the p24 map.
- Behavior:
  - Function table (ABT p2, ALS p2, LVC p2):

    | OE | LE | D | Q |
    |---|---|---|---|
    | L | H | H | H |
    | L | H | L | L |
    | L | L | X | Q0 |
    | H | X | X | Z |

  - LE is active high and transparent; the latch stores on LE↓. OE is active low and common.
  - OE gates the outputs only: "OE does not affect the internal operations of the latch[es]" (ABT p1, ALS p1, LVC p2).
  - The control pins are named **LE** in all three PDFs (no "C"). There is no clear or preset.
  - Power-up behavior:
    - ABT p1: "When VCC is between 0 and 2.1 V, the device is in the high-impedance state during power up or power down."
    - ALS p1 feature: "Power-Up High-Impedance State".
    - LVC has Ioff (partial power-down).
- Timing:
  - ABT: SN74ABT841A p5, 5 V, 25 °C, CL = 50 pF, TYP.

    | Path | Typ |
    |---|---|
    | D→Q tPLH / tPHL | 4.1 / 4.0 ns |
    | LE→Q | 4.1 / 4.6 ns |
    | tPZH / tPZL | 3.0 / 4.1 ns |
    | tPHZ / tPLZ | 4.7 / 4.6 ns |

    The SN54ABT841 typicals are the same.
  - ALS: p5, MAX at 4.5–5.5 V, TA MIN to MAX (no typ).

    | Path | Max |
    |---|---|
    | D→Q | 13 / 13 ns |
    | LE→Q | 21 / 26 ns |
    | tPZH / tPZL | 12 / 12 ns |
    | tPHZ / tPLZ | 10 / 12 ns |

  - LVC: p4, 3.3 V ± 0.3 V, MAX (no typ). The 1.8 V and 2.5 V columns are "(1) not available at the time of publication".

    | Path | 3.3 V max | 2.7 V max |
    |---|---|---|
    | D→Q tpd | 6.7 ns | 7.5 ns |
    | LE→Q tpd | 7.6 ns | 8.6 ns |
    | ten | 7.2 ns | 8.5 ns |
    | tdis | 5.9 ns | 6.6 ns |

- VCC:
  - ABT: 4.5–5.5 V for SN54 and SN74 (p3).
  - ALS: 4.5–5.5 V (p4).
  - LVC: 1.65–3.6 V (data retention 1.5 V), inputMax 5.5 V (VI 0–5.5 V, p3).
- Obsolete: none.
- Notes:
  - sn74als841.pdf also documents **SN74ALS842**, the same pinout with **inverting D inputs**. Its table is OE L, LE H, D H → Q L, and its timing is on p5. It also documents SN74AS841A. Neither has census rows, so I did not write a device for them. If 842 is ever wanted, it would be "74_842" (Q = NOT D).
  - The parent brief said "C/LE"; every PDF here names the pin LE.

## 74_843

- Title: "9-Bit Bus-Interface D-Type Latches With 3-State Outputs" (ABT843 p1). ALS843 prints "9-Bit Bus-Interface D-Type Latch With 3-State Outputs". Function: 9 transparent latches with true outputs, plus asynchronous PRE and CLR, one LE and **one** OE.
- Datasheets:
  - latch/sn74abt843.pdf (SCBS197D): p1 pinout and FK-28, p2 function table and logic symbol, p3 logic diagram, p4 rec-op, p5 timing requirements and switching.
  - latch/sn74als843.pdf (SDAS232A): p1 pinout and function table, p2 symbol and diagram, p3 rec-op, p4 switching.
- Families:
  - ABT: SN74ABT843*, SN54ABT843 / SNJ54ABT843*.
  - ALS: SN74ALS843*.
- Military: `5962-9571201QLA` and `SNJ54ABT843JT` → ABT843 (JT).
- Pinouts:
  - **p24** for DW, DB, NT, JT, W (ABT p1, ALS p1):
    - 1 OE (input)
    - 2–10 = 1D–9D (inputs)
    - 11 CLR (input)
    - 12 GND (ground)
    - 13 LE (input)
    - 14 PRE (input)
    - 15 = 9Q, 16 = 8Q, 17 = 7Q, 18 = 6Q, 19 = 5Q, 20 = 4Q, 21 = 3Q, 22 = 2Q, 23 = 1Q (outputs)
    - 24 VCC (power)
  - **fk28** (SN54ABT843 FK, ABT p1; no FK census rows):

    | Pin | Name | Pin | Name | Pin | Name | Pin | Name |
    |---|---|---|---|---|---|---|---|
    | 1 | NC | 8 | NC | 15 | NC | 22 | NC |
    | 2 | OE | 9 | 6D | 16 | LE | 23 | 5Q |
    | 3 | 1D | 10 | 7D | 17 | PRE | 24 | 4Q |
    | 4 | 2D | 11 | 8D | 18 | 9Q | 25 | 3Q |
    | 5 | 3D | 12 | 9D | 19 | 8Q | 26 | 2Q |
    | 6 | 4D | 13 | CLR | 20 | 7Q | 27 | 1Q |
    | 7 | 5D | 14 | GND | 21 | 6Q | 28 | VCC |

- Behavior:
  - Function table, identical in ABT p2 and ALS p1:

    | PRE | CLR | OE | LE | D | Q |
    |---|---|---|---|---|---|
    | L | X | L | X | X | H |
    | H | L | L | X | X | L |
    | H | H | L | H | L | L |
    | H | H | L | H | H | H |
    | H | H | L | L | X | Q0 |
    | X | X | H | X | X | Z |

  - PRE is active low and asynchronous. It sets all 9 latches (Q = H) regardless of LE and D.
  - CLR is active low and asynchronous. It resets all 9 latches (Q = L) regardless of LE and D.
  - **PRE has priority over CLR.** When both are low, Q = H (row 1, CLR = X). There is no "both low, not allowed" note in either PDF.
  - PRE and CLR act whether LE is high or low (LE = X in rows 1–2).
  - The logic diagram (ABT p3, ALS p2) shows each latch as an S / C1 / 1D / R element: PRE drives S, CLR drives R, LE drives C1. So PRE and CLR set and reset the **stored** bit.
  - After PRE or CLR returns high:
    - With LE = L, the latch keeps the preset or cleared value.
    - With LE = H, Q follows D again.

    ABT p5 lists PRE→Q tPHL and CLR→Q tPLH paths, which are consistent with Q returning to D on PRE/CLR release while LE = H. ABT p5 also gives "setup time, PRE inactive / CLR inactive" before LE↓ (1.6 / 2 ns).
  - Priority order: PRE (low) > CLR (low) > LE (high: Q = D; low: hold). OE is independent of all of these.
  - There is **one** OE (pin 1), active low and common to all 9 outputs. Q is enabled iff OE = L.
  - "OE does not affect the internal operations of the latch. Previously stored data can be retained or new data can be entered while the outputs are in the high-impedance state." (ABT p1; ALS p1 is similar.) So PRE and CLR still change the stored bit while the outputs are high-Z.
  - Power: ABT p1 says the outputs are high-Z during power up and power down. ALS p1 has the feature "Power-Up High-Impedance State".
- Timing:
  - ABT: SN74ABT843 p5, 5 V, 25 °C, CL = 50 pF, TYP.

    | Path | Typ |
    |---|---|
    | D→Q tPLH / tPHL | 3.8 / 3.4 ns |
    | LE→Q | 4.4 / 4.1 ns |
    | PRE→Q tPLH / tPHL | 5.0 / 4.1 ns |
    | CLR→Q tPLH / tPHL | 4.4 / 4.5 ns |
    | tPZH / tPZL | 3.4 / 4.3 ns |
    | tPHZ / tPLZ | 4.9 / 4.2 ns |

    The typicals are shared by the SN54 and SN74 columns.
  - ALS: p4, MAX at 4.5–5.5 V, TA MIN to MAX (no typ).

    | Path | Max |
    |---|---|
    | D→Q | 13 / 18 ns |
    | LE→Q | 21 / 26 ns |
    | PRE→Q tPLH | 22 ns |
    | CLR→Q tPHL | 23 ns |
    | tPZH / tPZL | 12 / 14 ns |
    | tPHZ / tPLZ | 10 / 12 ns |

  - Pulse widths: ABT p5 has CLR low 5.5 ns, PRE low 4.5 ns, and "LE low" 3.3 ns (3.4 ns for SN74). ALS p3 has CLR or PRE low 35 ns and LE high 20 ns.
- VCC: ABT 4.5–5.5 V for SN54 and SN74 (p4). ALS 4.5–5.5 V (p3).
- Obsolete: SN74ALS843DW. SN74ALS843DWR is Active.
- Notes:
  - The parent prompt expected "several OE pins". Both TI 843 PDFs have **a single OE**; there are no OE1/OE2/OE3.
  - ABT p5 lists the LE pulse width as "LE low". The ALS PDF says "LE high". It is probably a typo in the ABT sheet; for a transparent latch the LE-high width is the one that matters.

## 74_1G373

- Title: "SN74LVC1G373 Single D-Type Latch With 3-State Output" (SCES528F, p1).
- Datasheet: latch/sn74lvc1g373.pdf: p1 description, p3 pinouts and pin functions, p5 rec-op, p6 timing requirements, p7–8 switching, p11 load table, p13 function table.
- Families: LVC: SN74LVC1G373*, 74LVC1G373* (all census rows).
- Military: none.
- Pinouts:
  - **p6**: DBV (SOT-23-6) and DCK (SC70-6) (p3):

    | Pin | Name | Role |
    |---|---|---|
    | 1 | LE | input |
    | 2 | GND | ground |
    | 3 | D | input |
    | 4 | Q | output |
    | 5 | VCC | power |
    | 6 | OE | input |

  - **YZP** (DSBGA-6, bottom view, p3): A1 LE, A2 OE, B1 GND, B2 VCC, C1 D, C2 Q.
- Behavior:
  - Function table (p13):

    | OE | LE | D | Q |
    |---|---|---|---|
    | L | H | L | L |
    | L | H | H | H |
    | L | L | X | Q0 |
    | H | X | X | Hi-Z |

  - LE is active high: "output follows D input when high" (p3). Data is latched on LE↓. The output is non-inverting.
  - OE is active low: "Active low output enable; Hi-Z output when high" (p3).
  - "OE does not affect the internal operations of the latch. Old data can be retained or new data can be entered while the outputs are in the high-impedance state." (p1)
  - Ioff gives partial power-down; the output is disabled when VCC = 0.
- Timing: p7, table 6.9, −40 to 85 °C, VCC 3.3 V ± 0.3 V, CL = 50 pF (p11 Figure 3). Only MAX is printed (no typ).

  | Path | 3.3 V, 50 pF | 5 V, 50 pF | 3.3 V, 15 pF (table 6.8) |
  |---|---|---|---|
  | D→Q tpd | 5.4 ns | 4 ns | 4 ns |
  | LE→Q tpd | 5.5 ns | 4 ns | 4 ns |
  | ten | 5.1 ns | 3.7 ns | 4 ns |
  | tdis | 6.5 ns | 4.6 ns | 7.9 ns |

- VCC: 1.65–5.5 V (data retention 1.5 V) (p5). inputMax is 5.5 V: VIH max is 5.5 V at every VCC (p5), and p1 says "Inputs Accept Voltages to 5.5 V".
- Obsolete: none. All 7 census rows are Active.
- Notes:
  - p7 table 6.8 row "D→Q, VCC = 2.5 V" prints MIN 15, MAX 5. MIN and MAX look swapped or mistyped; the LE row has 1.5 / 5.
  - The DSBGA (YZP) is rated for −40 to 85 °C only.

---

## 74_16373

- Title / function: "16-Bit Transparent D-Type Latches With 3-State Outputs" (two independent 8-bit sections, each with its own LE and OE).
- Datasheets (PDF page index): latch/sn74abt16373a.pdf (p1 pinout, p2 FT, p3 ROC, p5 sw), latch/sn74ahc16373.pdf (p1, p2, p3, p5), latch/sn74ahct16373.pdf (p3 pinout, p5 ROC, p7 sw, p10–11 FT), latch/sn74alvch16373.pdf (p1, p3 FT, p4 ROC, p6 sw), latch/sn74auc16373.pdf (p1, p2 FT, p3 ROC, p4 sw), latch/sn74avc16373.pdf (p2, p3 FT, p5 ROC, p6 sw), latch/sn74lvc16373a.pdf (p3–4 pinout, p7 ROC, p8 sw, p11 FT), latch/sn74lvch16373a.pdf (p3, p7 ROC, p9 sw, p11–12 FT), latch/sn74lvth16373.pdf (p1, p3 FT, p4 ROC, p6 sw).
- Families & prefixes:
  - ABT: SN74ABT16373A*, 74ABT16373A* (G4/E4 rows), SN54ABT16373A / SNJ54ABT16373AWD
  - AHC: SN74AHC16373* (SN54AHC16373 WD shown in drawing header; no WD rows in census)
  - AHCT: SN74AHCT16373*
  - ALVC (ALVCH — bus hold on data inputs): SN74ALVCH16373*, 74ALVCH16373*
  - AUC: SN74AUC16373*
  - AVC: SN74AVC16373*
  - LVC: SN74LVC16373A*, 74LVC16373A*
  - LVC (LVCH — bus hold on data inputs): SN74LVCH16373A*, 74LVCH16373A*
  - LVT (LVTH — bus hold on data inputs): SN74LVTH16373*, 74LVTH16373*, SNJ54LVTH16373WD
- Military: 5962-9320001QXA = SNJ54ABT16373AWD (ABT, WD); 5962-9681001QXA = SNJ54LVTH16373WD (LVT/LVTH, WD) — from census mark column.
- Pinouts:
  - p48: DGG, DGV, DL, WD (all 16373 PDFs show the identical drawing; verified for every PDF):
    1 1OE input; 2 1Q1 output; 3 1Q2 output; 4 GND ground; 5 1Q3 output; 6 1Q4 output; 7 VCC power; 8 1Q5 output; 9 1Q6 output; 10 GND ground; 11 1Q7 output; 12 1Q8 output; 13 2Q1 output; 14 2Q2 output; 15 GND ground; 16 2Q3 output; 17 2Q4 output; 18 VCC power; 19 2Q5 output; 20 2Q6 output; 21 GND ground; 22 2Q7 output; 23 2Q8 output; 24 2OE input; 25 2LE input; 26 2D8 input; 27 2D7 input; 28 GND ground; 29 2D6 input; 30 2D5 input; 31 VCC power; 32 2D4 input; 33 2D3 input; 34 GND ground; 35 2D2 input; 36 2D1 input; 37 1D8 input; 38 1D7 input; 39 GND ground; 40 1D6 input; 41 1D5 input; 42 VCC power; 43 1D4 input; 44 1D3 input; 45 GND ground; 46 1D2 input; 47 1D1 input; 48 1LE input.
  - BGA maps printed in some PDFs but with NO orderable rows in the census (informational only):
    - GQL/ZQL 56-ball (sn74lvc16373a p3, sn74lvch16373a p3, sn74auc16373, sn74avc16373, sn74alvch16373, sn74lvth16373, sn74lvth162373 p2), rows A–K × cols 1–6:
      A: 1OE NC NC NC NC 1LE · B: 1Q2 1Q1 GND GND 1D1 1D2 · C: 1Q4 1Q3 VCC VCC 1D3 1D4 · D: 1Q6 1Q5 GND GND 1D5 1D6 · E: 1Q8 1Q7 – – 1D7 1D8 · F: 2Q1 2Q2 – – 2D2 2D1 · G: 2Q3 2Q4 GND GND 2D4 2D3 · H: 2Q5 2Q6 VCC VCC 2D6 2D5 · J: 2Q7 2Q8 GND GND 2D8 2D7 · K: 2OE NC NC NC NC 2LE (E3,E4,F3,F4 not populated).
    - GRD/ZRD 54-ball (sn74lvc16373a p3 "54-Ball GRD or ZRD", also sn74lvch16373a, sn74alvch16373, sn74lvth16373), rows A–J:
      A: 1Q1 NC 1OE 1LE NC 1D1 · B: 1Q3 1Q2 NC NC 1D2 1D3 · C: 1Q5 1Q4 VCC VCC 1D4 1D5 · D: 1Q7 1Q6 GND GND 1D6 1D7 · E: 2Q1 1Q8 GND GND 1D8 2D1 · F: 2Q3 2Q2 GND GND 2D2 2D3 · G: 2Q5 2Q4 VCC VCC 2D4 2D5 · H: 2Q7 2Q6 NC NC 2D6 2D7 · J: 2Q8 NC 2OE 2LE NC 2D8.
- Behavior (function table "each 8-bit section", e.g. sn74lvth162373 p2, sn74abt16373a p2, sn74lvc16373a p11):
  | OE | LE | D | Q |
  |---|---|---|---|
  | L | H | H | H |
  | L | H | L | L |
  | L | L | X | Q0 |
  | H | X | X | Z |
  - Section n (n = 1, 2): latch bits nD1..nD8 → nQ1..nQ8, controlled by nLE and nOE only. 1LE/1OE never affect section 2 and vice versa.
  - LE active HIGH, level-sensitive (transparent while nLE = H; captures on the falling edge of nLE and holds while L). No clock edge, no clear/preset.
  - nOE active LOW, gates only the output buffers: "OE does not affect internal operations of the latch. Old data can be retained or new data can be entered while the outputs are in the high-impedance state" (sn74abt16373a p1, lvc16373a p11, etc.). So the latch keeps tracking D while nLE = H even with nOE = H.
  - Outputs are true (non-inverting). No initial state is stated for these PDFs (treat as unknown).
  - Bus-hold (ALVCH, LVCH, LVTH): data inputs hold the last valid level when floating (alvch16373 p1, lvch16373a p11, lvth16373 p1). LVTH also has bus hold on control inputs? Text: "Active bus-hold circuitry holds unused or undriven inputs at a valid logic state" (lvth162373 p2), while the feature bullet says "Bus Hold on Data Inputs".
  - Power-up 3-state: ABT outputs Z while VCC is 0–2.1 V (abt16373a p1); LVTH outputs Z while VCC is 0–1.5 V (lvth16373 p2).
- Timing per family (ns):
  - ABT (sn74abt16373a p5, VCC = 5 V, TA = 25 °C, CL = 50 pF, typ; same typ in SN54 and SN74 tables): D→Q tPLH 3.7 / tPHL 4.0; LE→Q 4.1 / 4.3; tPZH 3.4, tPZL 3.5; tPHZ 5.1, tPLZ 4.4.
  - AHC (sn74ahc16373 p5, VCC = 5 V ± 0.5 V, CL = 50 pF, typ at 25 °C): D→Q 6.5 / 6.5; LE→Q 6.4 / 6.4; tPZH 6, tPZL 6; tPHZ 6.5, tPLZ 7.5. (At 3.3 V, 50 pF typ: D→Q 9.8, LE→Q 9.5.)
  - AHCT (sn74ahct16373 p7, VCC = 5 V ± 0.5 V, CL = 50 pF, typ): D→Q 5.9 / 5.9; LE→Q tPLH 6.4 / tPHL 5.9; tPZH 6, tPZL 6; tPHZ 6.8, tPLZ 7.8.
  - ALVC/ALVCH (sn74alvch16373 p6, VCC = 3.3 V ± 0.3 V, max; no typ): D→Q tpd 3.6; LE→Q 3.9; ten 4.7; tdis 4.1.
  - AUC (sn74auc16373 p4): at 1.8 V ± 0.15 V typ: D→Q 1.5, LE→Q 1.6, ten 1.7, tdis 2.7 (max 2.4 / 2.8 / 2.9 / 4.6). At 2.5 V ± 0.2 V max (no typ): D→Q 1.9, LE→Q 2.1, ten 2.2, tdis 2.5. The AUC is not specified at 3.3 V (VCC max 2.7 V).
  - AVC (sn74avc16373 p6, VCC = 3.3 V ± 0.3 V, max; typ printed only at 1.2 V): D→Q 2.8; LE→Q 3.2; ten 3.4; tdis 3.9.
  - LVC (sn74lvc16373a p8, 3.3 V ± 0.3 V, max; no typ): D→Q 4.2; LE→Q 4.6; ten 4.7; tdis 5.9.
  - LVC/LVCH (sn74lvch16373a p9, −40 to 85 °C, 3.3 V ± 0.3 V, max): D→Q 4.2; LE→Q 4.6; ten 4.7; tdis 5.9. (−40 to 125 °C table: 4.8 / 5.4 / 5.5 / 6.5.)
  - LVT/LVTH (sn74lvth16373 p6, SN74 columns, 3.3 V ± 0.3 V, CL = 50 pF, typ): D→Q tPLH 2.7 / tPHL 2.5; LE→Q 3.0 / 2.9; tPZH 2.8, tPZL 2.8; tPHZ 3.5, tPLZ 3.2.
- VCC per family (Recommended Operating Conditions):
  - ABT 4.5–5.5 V (SN54 and SN74) (p3); VI max = VCC.
  - AHC 2–5.5 V (p3); inputMax 5.5 V.
  - AHCT 4.5–5.5 V (p5); inputMax 5.5 V.
  - ALVCH 1.65–3.6 V (p4); VI max = VCC (no over-voltage inputs).
  - AUC 0.8–2.7 V (p3); inputMax 3.6 V.
  - AVC 1.4–3.6 V operating, data retention to 1.2 V (p5); VI max 3.6 V.
  - LVC 1.65–3.6 V, data retention 1.5 V (p7); inputMax 5.5 V.
  - LVCH 1.65–3.6 V (p7); inputMax 5.5 V.
  - LVTH 2.7–3.6 V (SN54 and SN74) (p4); inputMax 5.5 V.
- Obsolete: SN74AHC16373DL, SN74AHCT16373DL.
- Notes:
  - Datasheet errors: sn74lvc16373a p3 pin table lists pin 1 as "OE" (it is 1OE in the drawing) and p4 lists 2OE (pin 24) with I/O "O" (it is an input). sn74ahct16373 p7 switching-table header has the FROM/TO captions swapped ("FROM (OUTPUT) TO (INPUT)"). sn74lvch16373a GQL map prints "2F6" for 2D6 (ball H5); sn74lvth16373 GQL map prints "2CLK" for 2LE (ball K6).
  - AHC/AHCT tables with typ values marked * are "on products compliant to MIL-PRF-38535, not production tested" (CL = 15 pF rows).

## 74_162373

- Title / function: "3.3-V ABT 16-Bit Transparent D-Type Latches With 3-State Outputs". Same logic, pin names and pinout as 74_16373. The difference is that the outputs "include equivalent 22-Ω series resistors to reduce overshoot and undershoot" and are rated ±12 mA (sn74lvth162373 p1–2).
- Datasheets: latch/sn74lvth162373.pdf (p1 pinout, p2 description/FT/GQL map, p3 ROC, p5 switching).
- Families & prefixes: LVT (LVTH — bus hold): SN74LVTH162373*, 74LVTH162373*, SN54LVTH162373 / SNV54LVTH162373WD.
- Military: 5962-9763801VXA = SNV54LVTH162373WD (WD).
- Pinouts: p48: DGG, DL, WD — identical to the 74_16373 p48 list (verified line for line). The GQL/ZQL 56-ball map is printed on p2, but GQL/ZQL has no census rows.
- Behavior: identical to 74_16373 (function table "each 8-bit section", p2; "OE does not affect internal operations of the latch", p2; power-up 3-state for VCC 0–1.5 V).
- Timing (p5, SN74 columns, 3.3 V ± 0.3 V, CL = 50 pF, typ): D→Q tPLH 3.1 / tPHL 2.8; LE→Q 3.4 / 3.2; tPZH 3.2, tPZL 3.2; tPHZ 3.8, tPLZ 3.5.
- VCC: 2.7–3.6 V (SN54 and SN74) (p3); inputMax 5.5 V.
- Obsolete: none.
- Notes: if the repo has no per-part output-impedance concept, this device can be modelled as 74_16373 with different timing. A separate id is only needed if 162-series parts are kept apart.

## 74_32373

- Title / function: "SN74LVCH32373A 32-Bit Transparent D-Type Latch With 3-State Outputs". Four independent 8-bit banks, each with its own xLE and xOE ("can be used as four 8-bit latches, two 16-bit latches, or one 32-bit latch", p1).
- Datasheets: latch/sn74lvch32373a.pdf (p1, p3–5 ball map/pin table, p7 ROC, p9 sw, p11 description, p14 FT).
- Families & prefixes: LVC (LVCH — bus hold on data inputs): SN74LVCH32373A*.
- Military: none.
- Pinouts: b96: NMJ (census: SN74LVCH32373ANMJR, the only row). The p3 figure is captioned "GKE 96-Pin LFBGA". The Device Information table (p1) lists both LFBGA-GKE and LFBGA-NMJ at 13.50 × 5.00 mm, and no separate NMJ map is printed, so NMJ is taken to share the GKE ballout. Balls (row: col1..col6):
  - A: 1Q2 1Q1 1OE 1LE 1D1 1D2
  - B: 1Q4 1Q3 GND GND 1D3 1D4
  - C: 1Q6 1Q5 VCC VCC 1D5 1D6
  - D: 1Q8 1Q7 GND GND 1D7 1D8
  - E: 2Q2 2Q1 GND GND 2D1 2D2
  - F: 2Q4 2Q3 VCC VCC 2D3 2D4
  - G: 2Q6 2Q5 GND GND 2D5 2D6
  - H: 2Q7 2Q8 2OE 2LE 2D8 2D7
  - J: 3Q2 3Q1 3OE 3LE 3D1 3D2
  - K: 3Q4 3Q3 GND GND 3D3 3D4
  - L: 3Q6 3Q5 VCC VCC 3D5 3D6
  - M: 3Q8 3Q7 GND GND 3D7 3D8
  - N: 4Q2 4Q1 GND GND 4D1 4D2
  - P: 4Q4 4Q3 VCC VCC 4D3 4D4
  - R: 4Q6 4Q5 GND GND 4D5 4D6
  - T: 4Q7 4Q8 4OE 4LE 4D8 4D7
  - Roles: xQn output; xDn, xOE, xLE input; VCC power; GND ground. 96 balls, no NC. Note the swapped order in rows H and T (col1 = x Q7, col2 = x Q8; col5 = xD8, col6 = xD7). The pin table (p4–5) agrees with these names.
- Behavior (Table 7-1, p14; p1 and p11 text): per bank x = 1..4: OE=L, LE=H, D=H → Q=H; OE=L, LE=H, D=L → Q=L; OE=L, LE=L → Q0; OE=H → Z. LE is active HIGH and transparent; data are latched when LE goes low. xOE is active LOW and gates outputs only ("OE does not affect internal operations of the latch…", p11). Bus hold on the data inputs. Ioff and partial power-down: outputs are Z when the device is powered down (p12).
- Timing (p9, VCC = 3.3 V ± 0.3 V, max; no typ printed; 1.8 and 2.5 V columns are blank, "(1)"): D→Q 4.2; LE→Q 4.6; ten 4.7; tdis 5.9. (2.7 V max: 4.9 / 5.3 / 5.7 / 6.3.)
- VCC: 1.65–3.6 V operating, 1.5 V data retention (p7); inputMax 5.5 V.
- Obsolete: none.
- Notes: datasheet errors. The pin table (p4–5) describes every xLE as "Latch Enable, Active Low", which contradicts the function table (LE = H is transparent); trust the function table. Ball H1 is described as "2Q7 … Channel 8" and H2 as "2Q8 … Channel 7" (name and description disagree). Use the names, which match the ball figure. The logic diagram (p1) labels the 8th output "xQ7". The pin table lists P3 (VCC) with I/O "Input".

## 74_16841

- Title / function: "20-Bit Bus-Interface D-Type Latches With 3-State Outputs" (two 10-bit sections, each with its own LE and OE). The 841 name aside, this is a plain transparent latch like the 16373, but 10 bits wide per section.
- Datasheets: latch/sn74abt16841.pdf (p1 pinout/description, p2 FT/logic symbol, p3 ROC, p5 sw), latch/sn74alvch16841.pdf (p1, p2 FT, p4 ROC, p6 sw).
- Families & prefixes: ABT: SN74ABT16841* (SN54ABT16841 WD in header, no census rows). ALVC (ALVCH — bus hold on data inputs): SN74ALVCH16841*, 74ALVCH16841*.
- Military: none.
- Pinouts: p56: DL, DGG (WD in the abt16841 header) — identical in sn74abt16841 p1/p2 (logic-symbol pin numbers) and sn74alvch16841 p1:
  1 1OE input; 2 1Q1 output; 3 1Q2 output; 4 GND ground; 5 1Q3 output; 6 1Q4 output; 7 VCC power; 8 1Q5 output; 9 1Q6 output; 10 1Q7 output; 11 GND ground; 12 1Q8 output; 13 1Q9 output; 14 1Q10 output; 15 2Q1 output; 16 2Q2 output; 17 2Q3 output; 18 GND ground; 19 2Q4 output; 20 2Q5 output; 21 2Q6 output; 22 VCC power; 23 2Q7 output; 24 2Q8 output; 25 GND ground; 26 2Q9 output; 27 2Q10 output; 28 2OE input; 29 2LE input; 30 2D10 input; 31 2D9 input; 32 GND ground; 33 2D8 input; 34 2D7 input; 35 VCC power; 36 2D6 input; 37 2D5 input; 38 2D4 input; 39 GND ground; 40 2D3 input; 41 2D2 input; 42 2D1 input; 43 1D10 input; 44 1D9 input; 45 1D8 input; 46 GND ground; 47 1D7 input; 48 1D6 input; 49 1D5 input; 50 VCC power; 51 1D4 input; 52 1D3 input; 53 GND ground; 54 1D2 input; 55 1D1 input; 56 1LE input.
- Behavior (FUNCTION TABLE "each 10-bit latch", abt16841 p2): OE=L, LE=H, D=H → H; OE=L, LE=H, D=L → L; OE=L, LE=L, D=X → Q0; OE=H → Z. 1LE/1OE control 1D1–1D10 → 1Q1–1Q10; 2LE/2OE control 2D1–2D10 → 2Q1–2Q10. LE is active HIGH and transparent. OE is active LOW and gates outputs only ("The output-enable input does not affect the internal operation of the latches", p1). True outputs. ABT power-up Z for VCC 0–2.1 V (p2). ALVCH has bus hold on the data inputs.
- Timing:
  - ABT (abt16841 p5, VCC = 5 V, 25 °C, typ; identical typ in both SN54/SN74 tables): D→Q tPLH 3.2 / tPHL 3.5; LE→Q 3.2 / 3.4; tPZH 3.2, tPZL 3.6; tPHZ 4.1, tPLZ 4.4.
  - ALVCH (alvch16841 p6, 3.3 V ± 0.3 V, max): D→Q 3.9; LE→Q 4.3; ten 4.9; tdis 4.1.
- VCC: ABT 4.5–5.5 V (both) (p3), VI ≤ VCC; ALVCH 1.65–3.6 V (p4), VI ≤ VCC.
- Obsolete: none.
- Notes: in the first ABT table (p5), tPLZ has MIN 1.9 in the 25 °C column but MIN 1.2 in the over-temperature column (likely typo).

## 74_162841

- Title / function: "20-Bit Bus-Interface D-Type Latches With 3-State Outputs". Same logic, pin names and pinout as 74_16841. The ABT outputs "include equivalent 25-Ω series resistors" and sink up to 12 mA (sn74abt162841 p1–2). For the ALVCH162841, the "162" likewise means series-damped outputs (the IOH/IOL limits in its ROC are lower, ±12 mA at 3 V, p4).
- Datasheets: latch/sn74abt162841.pdf (p1 pinout, p2 FT, p3 ROC, p5 sw), latch/sn74alvch162841.pdf (p1, p2 FT, p4 ROC, p6 sw).
- Families & prefixes: ABT: SN74ABT162841*. ALVC (ALVCH — bus hold): SN74ALVCH162841* (census row "SN74ALVCH162841GR" = DGG).
- Military: none.
- Pinouts: p56: DGG, DL — identical to the 74_16841 p56 list (verified).
- Behavior: identical to 74_16841 (FT "each 10-bit latch", p2; "OE does not affect the internal operation of the latches", p2 abt / p1 alvch).
- Timing:
  - ABT (abt162841 p5, VCC = 5 V, 25 °C, typ): D→Q tPLH 3.5 / tPHL 4.3; LE→Q 3.5 / 4.1; tPZH 3.6, tPZL 4.6; tPHZ 4.3, tPLZ 3.6.
  - ALVCH (alvch162841 p6, 3.3 V ± 0.3 V, max): D→Q 4.3; LE→Q 4.7; ten 5.3; tdis 4.4.
- VCC: ABT 4.5–5.5 V (p3), VI ≤ VCC; ALVCH 1.65–3.6 V (p4), VI ≤ VCC.
- Obsolete: none.
