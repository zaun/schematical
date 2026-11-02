# cntA — 74-series synchronous 4-bit / 8-bit counters (161/163, 169, 190/191, 192/193, 561, 569, 867/869)

All PDF paths are relative to `datasheets/`. "p" = PDF page number (1-based in the PDF file, which matches the
printed page for every PDF here except the 2−x-numbered ALS ones where noted).
Four PDFs are scanned images (sn74ls161a, sn74ls169b, sn74ls191, sn74ls193). They were OCR'd, and every
number quoted from them was checked against the rendered page image.

Conventions used below: `·` = AND, `+` = OR, `¬` = NOT. "Q=15" means QA·QB·QC·QD (QA = LSB).
Pin names are given without overbars; the active level is stated in Behavior.

## PDF → devices

| PDF | Devices (census-backed) | Also printed in the PDF, no census rows |
|---|---|---|
| counters/cd74ac161.pdf | 74_161 (AC) | — |
| counters/cd74ac163.pdf | 74_163 (AC) | — |
| counters/cd74act161.pdf | 74_161 (ACT) | — |
| counters/cd74act163.pdf | 74_163 (ACT) | — |
| counters/cd74hc161.pdf | 74_161 (HC, HCT), 74_163 (HC, HCT) | — |
| counters/cd74hc190.pdf | 74_190 (HC), 74_191 (HC, HCT) | — |
| counters/cd74hc192.pdf | 74_192 (HC), 74_193 (HC, HCT) | — |
| counters/sn74als161b.pdf | 74_161 (ALS, AS), 74_163 (ALS, AS) | SN54ALS162B (decade, synchronous clear) |
| counters/sn74als169b.pdf | 74_169 (ALS, AS) | — |
| counters/sn74als191a.pdf | 74_191 (ALS) | — |
| counters/sn74als193a.pdf | 74_193 (ALS) | — |
| counters/sn74als561a.pdf | 74_561 (ALS) | — |
| counters/sn74als569a.pdf | 74_569 (ALS) | SN74ALS568A (decade version of 569) |
| counters/sn74als867a.pdf | 74_867 (ALS, AS), 74_869 (ALS, AS) | — |
| counters/sn74f161a.pdf | 74_161 (F) | — |
| counters/sn74f163a.pdf | 74_163 (F) | — |
| counters/sn74hc161.pdf | 74_161 (HC) | — |
| counters/sn74hc163-q1.pdf | 74_163 (HC, automotive -Q1) | — |
| counters/sn74hc163.pdf | 74_163 (HC) | — |
| counters/sn74hc191.pdf | 74_191 (HC) | — |
| counters/sn74hc193.pdf | 74_193 (HC) | — |
| counters/sn74ls161a.pdf (scanned) | 74_161 (LS), 74_163 (TTL, LS, S) | SN54/74160, 161, 162, 163 TTL; LS160A, LS162A; S162 |
| counters/sn74ls169b.pdf (scanned) | 74_169 (LS, S) | — |
| counters/sn74ls191.pdf (scanned) | 74_191 (LS) | SN54/74190, 191 TTL; LS190 |
| counters/sn74ls193.pdf (scanned) | 74_192 (TTL), 74_193 (LS) | SN54/74193 TTL; LS192; SN74192 |
| counters/sn74lv161a.pdf | 74_161 (LV) | — |
| counters/sn74lv163a.pdf | 74_163 (LV) | — |

No census row exists for any 160 or 162 part, the LS190 or TTL 190/191, the LS192, the ALS568A, or the
ALS162B. Per the brief, they are not given sections; their datasheet facts are noted under Notes where relevant.

### Census rows that the datasheets themselves do not map

The datasheets have no SMD/JM cross-reference table. The rows below were mapped using the census `mark`
column; where that was not enough, a web lookup was used (flagged).

- **sn74ls161a.pdf**
  - JM38510/31504 (B2A, BEA, BFA) and M38510/31504* → **LS161A**. This came from a web lookup, not the
    datasheet: distributor listings (DigiKey, 1sourcecomponents) show "JM38510/31504BEA 54LS161A".
  - JM38510/31512 (B2A, BEA, BFA) and M38510/31512* → **LS163A**, by elimination: the PDF's only other LS
    military type is 54LS163A. This is not confirmed by the datasheet.
  - 5962-9754801QEA/QFA → **TTL 163** (mark SNJ54163J/W).
  - 5962-7600801VEA → LS161A (mark SNV54LS161AJ, space grade).
  - 7600801*, 76008012A → LS161A.
  - 7603401*, 76034012A → LS163A.
- **sn74als161b.pdf**
  - JM38510/38001* and M38510/38001* → **ALS161B**.
  - JM38510/38002* and M38510/38002* → **ALS163B**.
  - Both come from a web lookup, not the datasheet: sierraic lists "54ALS161B JM38510/38001BEA", and
    distributors list 38002BEA as the 54ALS163B.
  - 8302201* and 83022012A → ALS161B; 8302202* and 83022022A → ALS163B (by mark).
- **sn74als169b.pdf**: JM38510/38003BEA and M38510/38003BEA (both Last) → ALS169B. This is the PDF's only
  military type (AS169A has no SN54 rows); the mark column is truncated to "JM38510/".
- **sn74ls191.pdf**: JM38510/31509* → LS191, the PDF's only censused function.
- **sn74ls193.pdf**: JM38510/31508* (including SFA) → LS193. 5962-9558401Q* → **TTL 192** (mark SNJ54192).
- **cd74hc161.pdf** (by mark): 8407501 → HC161; 8607601 → HC163.
- **cd74hc190.pdf** (by mark): 5962-8994601 → HC190; 5962-8689101 → HC191; 5962-8867101 → HCT191.
- **cd74hc192.pdf** (by mark): 5962-8780801 → HC192; 5962-8772401 → HC193; 5962-9084801M → HCT193.
- **sn74als867a.pdf** (by mark): 5962-8966801LA → AS867; 5962-8952601 → AS869.

---

## 74_161

- **Title / function:** "Synchronous 4-bit binary counter" — presettable 4-bit binary counter with
  asynchronous (direct) clear and synchronous load.
- **Datasheets:**
  - counters/sn74ls161a.pdf: p1–2 description, p5/p7 logic diagrams, p11 sequence, p12–15 TTL/LS tables.
  - counters/sn74als161b.pdf: p1–2, p4, p6–10.
  - counters/sn74f161a.pdf: p1–7.
  - counters/sn74hc161.pdf: p1–2, p6–8.
  - counters/cd74hc161.pdf: p1–3, p4, p6–8.
  - counters/cd74ac161.pdf: p3–6, p9, p11–12.
  - counters/cd74act161.pdf: p1–2, p6–8.
  - counters/sn74lv161a.pdf: p1–2, p6, p9–11.
- **Families & prefixes:**
  - LS: SN74LS161A*, SN54LS161A*, SNJ54LS161A*, SNV54LS161A
  - ALS: SN74ALS161B*, SN54ALS161B*, SNJ54ALS161B*
  - AS: SN74AS161*, SNJ54AS161J
  - F: SN74F161A*
  - HC: SN74HC161*, SN54HC161*, SNJ54HC161*, CD74HC161*, CD54HC161*
  - HCT: CD74HCT161*
  - AC: CD74AC161*, CD54AC161F3A
  - ACT: CD74ACT161*, CD54ACT161F3A
  - LV: SN74LV161A*
  - TTL: SN54161/SN74161 are printed in sn74ls161a.pdf but have no census rows.
- **Military numbers:** see the mapping above. sn74hc161.pdf is single-function, so all of its rows (JM38510/66302,
  5962-8407501*, 84075012A) are HC161.
- **Pinouts:**
  - **p16** — D, N, NS, PW, DB, DGV, J, W, plus CD packages E, M, F:
    1 CLR input, 2 CLK input, 3 A input, 4 B input, 5 C input, 6 D input, 7 ENP input, 8 GND ground,
    9 LOAD input, 10 ENT input, 11 QD output, 12 QC output, 13 QB output, 14 QA output, 15 RCO output,
    16 VCC power.
  - **Different pin names, same pins** (cd74hc161.pdf p2, HC/HCT161 only): MR=CLR, CP=CLK, P0..P3=A..D,
    PE=ENP, SPE=LOAD, TE=ENT, Q0..Q3=QA..QD, TC=RCO.
    - cd74ac161 and cd74act161 use the SN names.
    - Treat this as the same pinout under alternative names.
  - **p20 FK** (sn74hc161 p1, sn74ls161a p1, sn74als161b p1):
    1 NC nc, 2 CLR input, 3 CLK input, 4 A input, 5 B input, 6 NC nc, 7 C input, 8 D input, 9 ENP input,
    10 GND ground, 11 NC nc, 12 LOAD input, 13 ENT input, 14 QD output, 15 QC output, 16 NC nc, 17 QB output,
    18 QA output, 19 RCO output, 20 VCC power.
  - The 16-to-20 mapping above is the standard one for every 16-pin part in this file: DIP 1–4 → FK 2–5,
    5–8 → 7–10, 9–12 → 12–15, 13–16 → 17–20; FK 1, 6, 11, 16 are NC.
- **Behavior:**
  - **Clock:** CLK, rising edge, buffered. It is a fully independent clock circuit on LS-A/ALS/AS/F/HC/AC/LV:
    changes at ENP, ENT and LOAD take effect only at CLK↑ (setup/hold).
  - **CLR:** active low, **asynchronous**. All four Q go low regardless of CLK, LOAD, ENP and ENT. Highest
    priority.
    - The async clear also drives RCO low, because RCO is combinational on Q. CLR→RCO delays are printed
      (HC, AC, ACT, F, ALS, AS, LV).
  - **LOAD:** active low, synchronous. At CLK↑ with LOAD low, Q ← D,C,B,A (QA←A), regardless of ENP and ENT.
  - **Count:** at CLK↑ with LOAD high and ENP = ENT = H, Q ← Q+1 mod 16 (15 → 0).
  - **Hold:** at CLK↑ with LOAD high and (ENP=L or ENT=L), Q is unchanged.
  - **Priority:** CLR (async) > LOAD > count enable > hold.
    - Function table (cd74ac161 p11, cd74hc161 p3): "L X X X X X → L, L"; "H ↑ X X l …" load; "H ↑ h h h"
      count; "H X l X h" inhibit (qn); "H X X l h" inhibit with RCO = L.
  - **Enables:** ENP and ENT are both active high and AND together to enable counting. They gate the count
    synchronously; they are not ORed into the clock.
  - **RCO = ENT · QA · QB · QC · QD**, combinational. It does **not** depend on ENP, CLK or LOAD.
    - "RCO is the product of ENT, QA, QB, QC, and QD" — sn74hc163 p12, which the same text family applies to.
    - "ENP … does not affect RCO; ENT … affects RCO" — cd74ac161 p3 pin table.
    - Note 1 (cd74ac161 p12): "RCO output is high when ENT is high and the counter is at terminal count."
    - The RCO pulse lasts while the count is 15 (≈ one clock period, the "high portion of QA").
  - **Power-up state:** not stated.
- **Timing — 161:**

  | Family | CLK→Q (LH/HL) | CLK→RCO | ENT→RCO | CLR→Q (HL) | CLR→RCO (HL) | Basis |
  |---|---|---|---|---|---|---|
  | LS (LS161A) | 13/18 | 20/18 | 9/9 | 20 | — | typ, 5 V, **CL 15 pF**, ls161a p15 |
  | ALS (SN74ALS161B) | 15/20 | 20/20 | 13/13 | 24 | 23 | max, 4.5–5.5 V, 50 pF, als161b p8 |
  | ALS (SN54) | 19/25 | 34/27 | 18/17 | 27 | 32 | max, same conditions |
  | AS (SN74AS161) | 7/13 | 8 (LOAD H) / 16.5 (LOAD L) LH; 12.5 HL | 9/8.5 | 13 | 12.5 | max, als161b p10 |
  | F (F161A) | LOAD H 5.1/7.1; LOAD L 5.6/5.6 | 9.6/9.6 | 4.1/4.1 | 8.6 | 7.6 | typ, 5 V, 50 pF, f161a p7 |
  | HC (SN74HC161) | 25 | 24 | 17 | 21 | 22 | typ, 4.5 V, 50 pF, hc161 p8 |
  | HC (CD74HC161) | 37 max | 37 max | 24 max | 42 max | 42 max | 4.5 V, 50 pF, cd74hc161 p7–8 |
  | HCT (CD74HCT161) | 39 max | 42 max | 32 max | 50 max | 50 max | 4.5 V, 50 pF, cd74hc161 p8 |
  | AC | 15 max | 15.2 max | 9.4 max | 15 max | 15 max | 5 V±0.5, 50 pF, −40–85 °C, cd74ac161 p6 |
  | ACT | 15 max | 15.2 max | 9.8 max | 15 max | 15 max | cd74act161 p8 |
  | LV (3.3 V) | 7.8 | 8.7 count / 10.6 preset | 8.3 | 8.4 | 9.2 | typ, 50 pF, lv161a p10 |
  | LV (5 V) | 5.9 | 6.6 / 7.8 | 6.1 | 6.3 | 6.9 | typ, 50 pF, lv161a p11 |

  - CD74HC161 also prints typical values at 5 V and 15 pF: CP→Q 15, CP→TC 15, TE→TC 9, MR→Q 18 (p7–8).
  - CD74HCT161 at 5 V, 15 pF: 16, 18, 13, 21.
  - AC/ACT: the −55–125 °C column is 16.5 / 16.7 / 10.3 (AC) or 10.8 (ACT) / 16.5.
  - TTL SN74161 (ls161a p13; no census rows): CLK→Q 13/15 with LOAD H and 17/19 with LOAD L; CLK→RCO 23/23;
    ENT→RCO 11/11; CLR→Q 26.
- **VCC:**

  | Family | VCC range (V) | Source |
  |---|---|---|
  | LS | SN54 4.5–5.5; SN74 4.75–5.25 | ls161a p14 |
  | ALS, AS | 4.5–5.5 | als161b p7, p9 |
  | F | 4.5–5.5 | f161a p6 |
  | HC | 2–6 | hc161 p6; cd74hc161 p4 |
  | HCT | 4.5–5.5 | cd74hc161 p4 |
  | AC | 1.5–5.5 | ac161 p4 |
  | ACT | 4.5–5.5 | act161 p6 |
  | LV | 2–5.5, **inputMax 5.5 V** (VI 0–5.5) | lv161a p6 |

- **Obsolete / not active (census):** CD74AC161M, CD74ACT161M, CD74HC161M, CD74HCT161M, SN74ALS161BD,
  SN74F161AD, SN74HC161D, SN74HC161DT, SN74HC161PW, SN74HC161PWT, SN74LS161AD, SN74LV161AD, SN74LV161APW,
  SN74LV161APWT.
  - NRND: CD74ACT161E, CD74HCT161E, CD74HCT161EE4, 84075012A, SNJ54HC161FK, SN74LV161ADGVR.
  - Last: SN74ALS161BNSR, SN74LV161ANSR.
- **Notes:**
  - **CD74HC161 errors** (p1): it says "'HC161 and 'HCT161 are asynchronous reset decade and binary
    counters". This is wrong — all four parts are 4-bit binary (Features list and the TC note "HHHH"). Its
    illegal-state sentence is decade boilerplate.
  - **CD74HC161 transition notes** (p3), Notes 2/3: "High-to-Low transition of PE or TE … and Low-to-High of
    SPE … should only occur while CP is HIGH for conventional operation". The SN datasheets say ENP/ENT
    transitions are allowed regardless of CLK. TTL '160–'163 (ls161a p1–2) carry the same restriction.
  - **LV161A function table** (p2, verified on the image): the last row reads "H X X X ↑ → No Change", which
    contradicts the count row. It is evidently meant to be a non-rising CLK edge. Datasheet error.
  - **CLK→RCO with LOAD low** is slower on AS and LV (the "preset mode" rows).
  - **RCO glitch:** sn74hc163 p10–13 reports a 7–12 ns glitch on cascaded RCO, due to a race between ENT
    falling and QA rising. It is "common to all HC vendors … and bipolar (LS, ALS, AS)". The behavioral model
    is RCO = ENT·Q=15 with no glitch.

## 74_163

- **Title / function:** "Synchronous 4-bit binary counter" with **synchronous** clear. It is identical to
  74_161 except for CLR.
- **Datasheets:**
  - counters/sn74ls161a.pdf: p1–2, p5, p7, p9, p11–17.
  - counters/sn74als161b.pdf: p1–2, p4, p6–10.
  - counters/sn74f163a.pdf: p1–7.
  - counters/sn74hc163.pdf: p1–2, p6–8, p10–13.
  - counters/sn74hc163-q1.pdf: p1–7.
  - counters/cd74hc161.pdf: p1–3, p6–8.
  - counters/cd74ac163.pdf: p1–2, p6–9.
  - counters/cd74act163.pdf: p1–2, p6–8.
  - counters/sn74lv163a.pdf: p3–7, p11–13.
- **Families & prefixes:**
  - TTL: SN54163J, SNJ54163*
  - LS: SN74LS163A*, SN54LS163A*, SNJ54LS163A*
  - S: SN74S163N, SN54S163J, SNJ54S163J
  - ALS: SN74ALS163B*, SN54ALS163B*, SNJ54ALS163B*
  - AS: SN74AS163*
  - F: SN74F163A*
  - HC: SN74HC163* (incl. SN74HC163IPWRG4Q1), SN54HC163J, SNJ54HC163*, CD74HC163*, CD54HC163F3A
  - HCT: CD74HCT163*, CD54HCT163F*
  - AC: CD74AC163*, CD54AC163F3A
  - ACT: CD74ACT163*, CD54ACT163F3A
  - LV: SN74LV163A*
- **Military numbers:** see the mapping above. sn74hc163.pdf is single-function, so all rows (JM38510/66304,
  8607601*, 86076012A) are HC163.
- **Pinouts:** identical to 74_161 — p16 (D, N, NS, PW, DB, DGV, J, W, E, M, F) and p20 FK, with the same
  CD-name aliases (cd74hc161 p2). Additional pinouts:
  - **p16 RGY** (VQFN 4×3.5 mm, lv163a p3): same pin numbers and names 1–16 as the DIP/SOIC.
    - Corner placement: pins 1 (CLR) and 16 (VCC) are top; 2–7 left; 8 (GND) and 9 (LOAD) bottom; 10–15 right.
    - It has an **exposed thermal pad** (dashed in the drawing). Its connection is not stated in the
      datasheet text.
  - SN74HC163-Q1 is PW only.
- **Behavior:** as 74_161 except:
  - **CLR:** active low, **synchronous**. At CLK↑ with CLR low, Q ← 0000, regardless of LOAD, ENP and ENT.
    - "after the next low-to-high transition of CLK, regardless of the levels of the enable inputs"
      (hc163 p2; ac163 p1).
    - ac163 p2 function table: "L ↑ X X X X → L, L"; load and count rows require CLR = h.
  - **Priority at CLK↑:** CLR > LOAD > (ENP·ENT count) > hold.
  - **RCO = ENT·QA·QB·QC·QD**, combinational. It goes low after a sync clear only when the clock edge has
    changed Q (there is no CLR→RCO path).
  - TTL '163 (ls161a p1): avoid L→H on CLR or LOAD while CLK is low with enables high. This does not apply
    to LS-A or S.
- **Timing — 163:**

  | Family | CLK→Q | CLK→RCO | ENT→RCO | Basis |
  |---|---|---|---|---|
  | TTL (SN54163) | 13/15 (LOAD H), 17/19 (LOAD L) | 23/23 | 11/11 | typ, 5 V, 15 pF, ls161a p13 |
  | LS163A | 13/18 | 20/18 | 9/9 | typ, 15 pF, ls161a p15 |
  | S163 | 8/10 | 14/17 | 10/10 | typ, 5 V, 15 pF, ls161a p17 |
  | ALS163B (SN74) | 15/20 | 20/20 | 13/13 | max, 50 pF, als161b p8 |
  | ALS163B (SN54) | 18/25 | 25/25 | 16/16 | max |
  | AS163 (SN74) | 7/13 | 8 / 16.5 LH (LOAD H / LOAD L), 12.5 HL | 9/8.5 | max, als161b p10 |
  | F163A | 5.1/7.1 (LOAD H), 5.6/5.6 (LOAD L) | 9.6/9.6 | 4.1/4.1 | typ, 50 pF, f163a p7 |
  | HC (SN74HC163, -Q1) | 25 | 24 | 17 | typ, 4.5 V, 50 pF, hc163 p8; Q1 p7 |
  | HC (CD) | 37 max | 37 max | 24 max | cd74hc161 p7 |
  | HCT (CD) | 39 max | 42 max | 32 max | cd74hc161 p8 |
  | AC | 15 max | 15.2 max | 9.4 max | 5 V, ac163 p9 |
  | ACT | 15 max | 15.2 max | 9.8 max | act163 p8 |
  | LV (3.3 V) | 8 | 8.8 count / 10.7 preset | 8.2 | typ, 50 pF, lv163a p7 |
  | LV (5 V) | 6.1 | 6.6 / 7.8 | 6.3 | typ, lv163a p7 |

  - The TTL and LS clear delay is measured from CLK (Note 5/Note 8): TTL 26, LS 20 typ.
- **VCC:**

  | Family | VCC range (V) |
  |---|---|
  | TTL, LS, S | SN54 4.5–5.5; SN74 4.75–5.25 (ls161a p12, p14, p16) |
  | ALS, AS, F | 4.5–5.5 |
  | HC (incl. -Q1) | 2–6 |
  | HCT | 4.5–5.5 |
  | AC | 1.5–5.5 |
  | ACT | 4.5–5.5 |
  | LV | 2–5.5, **inputMax 5.5** |

- **Obsolete / not active (census):** CD74AC163M, CD74ACT163M, CD74HC163M, CD74HC163MT, CD74HCT163M,
  SN74ALS163BD, SN74F163AD, SN74HC163D, SN74HC163DT, SN74HC163PW, SN74HC163PWT, SN74LS163AD, SN74LV163AD,
  SN74LV163APW.
  - NRND: SN74LV163ADGVR, SN74LV163ADR, JM38510/31512BEA, M38510/31512BEA.
  - Last: SN74ALS163BNSR.
- **Notes:**
  - **LV163A function table** (p13, image) is broken. The CLK column is blank, and the clear row reads
    "L X X X X → L" (asynchronous-looking). This contradicts the text on p11 (synchronous clear). Trust the
    text.
  - LV163A recommended-conditions rows misprint "VCC = 4.5 V to 1.5 V".
  - HC163 p12–13: above 2 cascaded stages, fmax is limited (18 MHz typ at 4.5 V) by the RCO glitch.

## 74_169

- **Title / function:** "Synchronous 4-bit up/down binary counters". Fully synchronous load; no clear.
- **Datasheets:**
  - counters/sn74ls169b.pdf (scanned): p1–9.
  - counters/sn74als169b.pdf: p1–7, printed pages 2−1…2−7.
- **Families & prefixes:**
  - LS: SN74LS169B*, SN54LS169B*, SNJ54LS169B*
  - S: SN54S169J, SNJ54S169J
  - ALS: SN74ALS169B*, SN54ALS169B*, SNJ54ALS169B*
  - AS: SN74AS169AN
- **Military numbers:**
  - 8001802*, 80018022A → LS169B.
  - 8302501EA → ALS169B.
  - JM38510/38003BEA, M38510/38003BEA → ALS169B.
- **Pinouts:**
  - **p16** — D, N, NS, J, W:
    1 U/D input, 2 CLK input, 3 A input, 4 B input, 5 C input, 6 D input, 7 ENP input, 8 GND ground,
    9 LOAD input, 10 ENT input, 11 QD output, 12 QC output, 13 QB output, 14 QA output, 15 RCO output,
    16 VCC power.
  - **p20 FK** (standard map): 1 NC, 2 U/D, 3 CLK, 4 A, 5 B, 6 NC, 7 C, 8 D, 9 ENP, 10 GND, 11 NC, 12 LOAD,
    13 ENT, 14 QD, 15 QC, 16 NC, 17 QB, 18 QA, 19 RCO, 20 VCC (als169b p1).
- **Behavior:**
  - **Clock:** CLK, rising edge, fully independent clock circuit. ENP, ENT, LOAD and U/D act only at CLK↑.
  - **LOAD:** active low, synchronous. At CLK↑ with LOAD low, Q ← A..D, regardless of the enables (the
    symbol has 1,7D with no G dependency).
  - **Count:** at CLK↑ with LOAD high and **ENP = ENT = L** (both active LOW).
    - U/D = H → up, mod 16.
    - U/D = L → down, mod 16 (0 → 15).
  - **Hold:** LOAD high and (ENP=H or ENT=H).
  - **Priority:** LOAD > count > hold. There is no clear.
  - **RCO (active low):** RCO = ¬( ¬ENT · [ (U/D · Q=15) + (¬U/D · Q=0) ] ). Combinational; depends on ENT
    only, not on ENP or CLK.
    - Symbol (als169b p2): "3,5CT=15" and "4,5CT=0" with G5 = ENT. M3 = UP and M4 = DOWN, both derived from
      U/D.
    - Changing U/D at Q=0 or Q=15 toggles RCO directly; the U/D→RCO delay is specified. The LS footnote says
      "at minimum (0) the RCO transition is in phase, at maximum (15) out of phase".
  - LS text: the RCO pulse lasts ≈ the high portion of QA when counting up and ≈ the low portion when
    counting down.
  - **Power-up:** not stated.
- **Timing — 169:**

  | Family | CLK→Q | CLK→RCO | ENT→RCO | U/D→RCO | Basis |
  |---|---|---|---|---|---|
  | LS169B | 16/17 (**CL 45 pF**, RL 667 Ω) | 26/17 | 15/11 | 23/15 | typ, 5 V, 15 pF, ls169b p7 |
  | S169, U/D = H | 8/11 | 14/20 | 7.5/15 | 9/10 | typ, 5 V, 15 pF, ls169b p9 |
  | S169, U/D = L | 8/11 | 14/20 | 6/15 | 8/16 | same |
  | ALS169B (SN74) | 15/20 | 20/20 | 13/16 | 19/19 | max, 50 pF, als169b p6 |
  | ALS169B (SN54) | 20/23 | 20/25 | 16/24 | 22/26 | max |
  | AS169A (SN74) | 7/13 | 16.5/13 (LOAD high or low) | 9/9 | 12/13 | max, als169b p7 |

  - S169 fmax: 70 MHz typ counting up, 55 MHz counting down.
- **VCC:**
  - LS, S: SN54 4.5–5.5; SN74 4.75–5.25 (ls169b p6, p8).
  - ALS, AS: 4.5–5.5 (als169b p5–6).
- **Obsolete / not active:** SN74ALS169BD (Obsolete). Last: JM38510/38003BEA, M38510/38003BEA.
- **Notes:** The ALS/AS datasheet has no function table; behavior comes from the text and the symbol.

## 74_190

- **Title / function:** "Synchronous up/down counters with down/up mode control" — BCD decade up/down
  counter with asynchronous parallel load.
- **Datasheets:**
  - counters/cd74hc190.pdf: p1–4, p8, p10–14.
  - counters/sn74ls191.pdf (scanned): p1–3, p5. This covers TTL and LS190, which have no census rows; it is
    used for the behavior text and the symbol.
- **Families & prefixes:** HC: CD74HC190*, CD54HC190F3A.
- **Military numbers:** 5962-8994601EA → HC190.
- **Pinouts:**
  - **p16** — E, NS, PW, F:
    1 B input, 2 QB output, 3 QA output, 4 CTEN input, 5 D/U input, 6 QC output, 7 QD output, 8 GND ground,
    9 D input, 10 C input, 11 LOAD input, 12 MAX/MIN output, 13 RCO output, 14 CLK input, 15 A input,
    16 VCC power.
  - Same as 74_191; there is no FK row in the census.
- **Behavior:** identical to 74_191 (below), except:
  - **Modulus 10.** Up: 9 → 0. Down: 0 → 9.
  - **MAX/MIN** = (¬D/U · Q=9) + (D/U · Q=0). Symbol "3(CT=9)Z6" and "2(CT=0)Z6" (ls191 p2).
  - **Illegal states** — state diagram, cd74hc190 p10 (read from the image):
    - Count up: 10→9, 11→4, 12→9, 13→0, 14→9, 15→0 ("corrected in one count").
    - Count down: 10→9, 11→10, 12→3, 13→4, 14→5, 15→6 ("corrected in one or two counts").
    - Normal sequences: up 0→1…→9→0; down 9→8…→0→9.
  - MAX/MIN and RCO behavior while in an illegal state is not stated; the gate-level decode is not given
    clearly enough.
- **Timing:** CD74HC190 is printed together with HC191 (p14):
  - Max at 4.5 V, 50 pF, 25 °C: LOAD→Q 39, data→Q 35, CLK→Q 34, CLK→RCO 25, CLK→MAX/MIN 42, D/U→RCO 30,
    D/U→MAX/MIN 33, CTEN→RCO 25.
  - Typ at 5 V, 15 pF: 16, 14, 14, 10, 18, 12, 13, 10.
- **VCC:** HC 2–6 (p11).
- **Obsolete:** CD74HC190PW.
- **Notes:** The decade TTL/LS190 are in sn74ls191.pdf but have no census rows.

## 74_191

- **Title / function:** "Synchronous up/down counters with down/up mode control" — 4-bit binary up/down,
  with asynchronous load, ripple-clock and MAX/MIN cascade outputs.
- **Datasheets:**
  - counters/sn74ls191.pdf (scanned): p1–2, p4, p6, p9–10.
  - counters/sn74als191a.pdf: p1–2, p5–6.
  - counters/sn74hc191.pdf: p1–2, p5–8.
  - counters/cd74hc190.pdf: p1–2, p5–6, p9–15.
- **Families & prefixes:**
  - LS: SN74LS191*, SN54LS191J, SNJ54LS191*
  - ALS: SN74ALS191A*, SNJ54ALS191A*
  - HC: SN74HC191*, SN54HC191J, SNJ54HC191*, CD74HC191*, CD54HC191F3A
  - HCT: CD74HCT191*, CD54HCT191F3A
- **Military numbers:**
  - 7600901*, JM38510/31509*, M38510/31509* → LS191 (76009012A appears as the mark of SNJ54LS191FK).
  - 5962-86840* → ALS191A.
  - 5962-86891* and 5962-8689101 → HC191.
  - 5962-8867101 → HCT191.
- **Pinouts:**
  - **p16** — D, N, NS, J, W, E, M, F:
    1 B input, 2 QB output, 3 QA output, 4 CTEN input, 5 D/U input, 6 QC output, 7 QD output, 8 GND ground,
    9 D input, 10 C input, 11 LOAD input, 12 MAX/MIN output, 13 RCO output, 14 CLK input, 15 A input,
    16 VCC power.
  - **p20 FK** (ls191 p1 image; hc191 p1; als191a p1):
    1 NC, 2 B, 3 QB, 4 QA, 5 CTEN, 6 NC, 7 D/U, 8 QC, 9 QD, 10 GND, 11 NC, 12 D, 13 C, 14 LOAD, 15 MAX/MIN,
    16 NC, 17 RCO, 18 CLK, 19 A, 20 VCC.
- **Behavior:**
  - **LOAD:** active low, **asynchronous, level-sensitive**. While LOAD is low, Q follows A..D
    (transparent), "independently of the level of the clock". LOAD→Q and data→Q delays are both specified.
    LOAD overrides counting. Symbol: LOAD = C5 with 5D (ls191 p2).
  - **Clock:** CLK, rising edge. It counts when CTEN = L (active low) and LOAD = H.
    - D/U = L → up; D/U = H → down. Modulo 16.
    - CTEN = H inhibits.
  - **Priority:** LOAD (async) > CTEN inhibit > count.
  - **CTEN gating:**
    - HC/ALS ("fully independent clock circuit"): CTEN and D/U are sampled with setup and hold to CLK↑
      (hc191 p7, als191a p5). Model them as synchronous enables.
    - LS/TTL (ls191 p1): "Level changes at the enable input should be made only when the clock input is high
      … A false clock may occur if the down/up input changes while the clock is low."
    - cd74hc190 p2: "D/U or CTEN should be changed only when clock is high."
    - These old parts effectively gate CLK with CTEN/D/U.
  - **MAX/MIN** (active high) = (¬D/U · Q=15) + (D/U · Q=0). It does **not** depend on CTEN or CLK.
    - Symbol: "2(CT=0)Z6 [DOWN], 3(CT=15)Z6 [UP]".
    - It is high for ≈ one full clock cycle at overflow/underflow.
  - **RCO** (active low) = ¬( MAX/MIN · ¬CTEN · ¬CLK ).
    - Symbol (ls191 p2, als191a p2): RCO labelled "6,1,4", where Z6 = MAX/MIN, G1 = CTEN (active low) and
      G4 = CLK with a negation indicator (active when CLK is low).
    - Text: RCO "produces a low-level output pulse under those same conditions, but only while the clock
      input is low" (als191a p2; hc191 p2). LS p1: "equal in width to the low-level portion of the clock".
    - Timing relation:
      1. In the cycle where Q reaches 15 (up) or 0 (down) with CTEN low, RCO falls after CLK falls
         (CLK→RCO tPHL).
      2. It rises again after the next CLK↑ (CLK→RCO tPLH), at which the counter also leaves max/min.
    - It also follows CTEN (CTEN→RCO) and D/U (D/U→RCO) combinationally.
  - LS p1 hazard: "A false ripple carry may occur if both the clock and enable are low and the down/up input
    is high during a load pulse."
  - **Power-up:** not stated.
- **Timing — 191:**

  | Family | LOAD→Q | Data→Q | CLK→Q | CLK→RCO | CLK→MAX/MIN | D/U→RCO | D/U→MAX/MIN | CTEN→RCO | Basis |
  |---|---|---|---|---|---|---|---|---|---|
  | LS191 | 22/33 | 20/27 | 16/24 | 13/16 | 28/37 | 30/30 | 21/22 | 21/22 | typ, 5 V, 15 pF, ls191 p10 |
  | ALS191A (SN74) | 30/30 | 21/21 | 18/18 | 20/20 | 31/31 | 37/28 | 25/25 | 18/18 | max, 50 pF, als191a p6 |
  | HC (SN74HC191) | 40 | 36 | 31 | 17 | 39 | 29 | 24 | 15 | typ, 4.5 V, 50 pF, hc191 p8 |
  | HC (CD74HC191) | 39 | 35 | 34 | 25 | 42 | 30 | 33 | 25 | max, 4.5 V, 25 °C, cd74hc190 p14 |
  | HCT (CD74HCT191) | 40 | 38 | 27 | 35 | 42 | 30 | 38 | 27 | max, 4.5 V, 25 °C, cd74hc190 p15 |

  - hc191 p8: the D/U rows have no "FROM" label in the extracted text. Order matches the CD table.
  - TTL '191 (ls191 p8; no census rows): Data→Q 14/35; other values equal the LS191 values.
- **VCC:**
  - LS: SN54 4.5–5.5; SN74 4.75–5.25 (ls191 p9).
  - ALS: 4.5–5.5.
  - HC: 2–6.
  - HCT: 4.5–5.5 (cd74hc190 p11).
- **Obsolete / not active:** CD74HC191M, CD74HC191MT, SN74ALS191AD, SN74HC191D, SN74HC191DT, SN74LS191D.
  - NRND: SN74ALS191ADR.
  - Last: SN74ALS191AN, SN74LS191NSR.
- **Notes:**
  - CD74HCT191 p15 lists CLK→RCO 35 and CLK→Q 27, whereas HC lists CLK→RCO 25 and CLK→Q 34. These are
    probably swapped rows in the datasheet.
  - LS191 p9 lists "tenable count enable time 40 ns" (Note 3).

## 74_192

- **Title / function:** "Synchronous 4-bit up/down counters (dual clock with clear)" — BCD decade.
- **Datasheets:**
  - counters/sn74ls193.pdf (scanned): p1–2, p4–5, p7.
  - counters/cd74hc192.pdf: p1–3, p6–8.
- **Families & prefixes:**
  - TTL: SN54192J, SNJ54192J/W
  - HC: CD74HC192*, CD54HC192F3A
- **Military numbers:**
  - 5962-9558401QEA/QFA → TTL 192.
  - 5962-8780801EA → HC192.
- **Pinouts — p16** (J, W, E, NS, PW, F), SN names:
  1 B input, 2 QB output, 3 QA output, 4 DOWN input, 5 UP input, 6 QC output, 7 QD output, 8 GND ground,
  9 D input, 10 C input, 11 LOAD input, 12 CO output, 13 BO output, 14 CLR input, 15 A input, 16 VCC power.
  - **CD names for the same pins** (cd74hc192 p1): 1 P1, 2 Q1, 3 Q0, 4 CPD, 5 CPU, 6 Q2, 7 Q3, 8 GND, 9 P3,
    10 P2, 11 PL, 12 TCU, 13 TCD, 14 MR, 15 P0, 16 VCC. The names differ but the function is the same.
- **Behavior:** as 74_193, except:
  - **Modulus 10.** Up: 9 → 0. Down: 0 → 9.
  - **CO** = ¬( ¬UP · Q=9 ). Symbol "1̄CT=9" (ls193 p4).
  - **BO** = ¬( ¬DOWN · Q=0 ).
  - **Illegal states:** cd74hc192 p1 says "it will return to the normal sequence in one count as shown in
    state diagram", but **no state diagram is printed** in the PDF. The TTL datasheet gives none either.
    Next states from 10–15 are therefore unspecified.
- **Timing:**
  - TTL SN54/74192 (ls193 p7, typ, 5 V, 15 pF): UP→CO 17/16; DOWN→BO 16/16; UP or DOWN→Q 25/31;
    LOAD→Q 27/29; CLR→Q 22.
  - CD74HC192 (p6–7), max at 4.5 V, 50 pF: CPU→TCU 25, CPD→TCD 25, CPU/CPD→Q 43, PL→Q 44, MR→Q 40.
    Typ at 5 V, 15 pF: 10, 10, 18, 18, 17.
- **VCC:**
  - TTL: SN54 4.5–5.5; SN74 4.75–5.25 (ls193 p7).
  - HC: 2–6 (cd74hc192 p3).
- **Obsolete:** CD74HC192PWT.
- **Notes:** the cd74hc192 p1 TCU/TCD description is garbled by a column merge.

## 74_193

- **Title / function:** "Synchronous 4-bit up/down binary counters (dual clock with clear)".
- **Datasheets:**
  - counters/sn74ls193.pdf (scanned): p1, p3–4, p6, p8.
  - counters/sn74als193a.pdf: p1, p5–6.
  - counters/sn74hc193.pdf: p1–2, p4–7.
  - counters/cd74hc192.pdf: p1–3, p6–8.
- **Families & prefixes:**
  - LS: SN74LS193*, SN54LS193J, SNJ54LS193*
  - ALS: SN74ALS193A*, SN54ALS193AJ, SNJ54ALS193A*
  - HC: SN74HC193*, SN54HC193J, SNJ54HC193J, CD74HC193*, CD54HC193F3A
  - HCT: CD74HCT193E, CD54HCT193F3A
- **Military numbers:**
  - 7600601*, 76006012A, JM38510/31508*, M38510/31508* → LS193.
  - 5962-8869801* → ALS193A.
  - 5962-8772401EA → HC193.
  - 5962-9084801MEA → HCT193.
- **Pinouts:**
  - **p16** — D, N, NS, PW, J, W, E, M, F: same as 74_192, including the CD alias names.
  - **p20 FK** (ls193 p1 image): 1 NC, 2 B, 3 QB, 4 QA, 5 DOWN, 6 NC, 7 UP, 8 QC, 9 QD, 10 GND, 11 NC, 12 D,
    13 C, 14 LOAD, 15 CO, 16 NC, 17 BO, 18 CLR, 19 A, 20 VCC.
- **Behavior:**
  - **CLR:** active **HIGH**, asynchronous. Q ← 0. Overrides LOAD, data and count ("Clear overrides load,
    data, and count inputs", ls193 p5–6).
  - **LOAD:** active low, **asynchronous, level-sensitive**. While low, Q follows A..D (transparent; LOAD→Q
    is specified). Symbol: C3/3D. Overrides counting.
  - **Count:** there are two clocks.
    - Q+1 mod 16 on **UP↑ while DOWN = H**.
    - Q−1 on **DOWN↑ while UP = H**.
    - "When counting up, count-down input must be high; when counting down, count-up input must be high."
    - Behavior with the other clock low is not specified. ALS gives a hold time "UP high after DOWN↑".
  - **Priority:** CLR > LOAD > count.
  - **CO** (active low) = ¬( ¬UP · Q=15 ). Symbol "1̄CT=15" with G1 = UP (ls193 p4). Text: "CO produces a
    low-level pulse while the count is maximum … and UP is low" (hc193 p2).
    - It therefore follows the UP clock: it goes low on UP↓ while at 15 and returns high on the next UP↑
      (the counter goes to 0).
    - The pulse width equals the UP low time.
  - **BO** (active low) = ¬( ¬DOWN · Q=0 ). It follows DOWN in the same way.
  - Cascade: BO → DOWN and CO → UP of the next stage.
- **Timing — 193:**

  | Family | UP→CO | DOWN→BO | UP/DOWN→Q | LOAD→Q | CLR→Q | Basis |
  |---|---|---|---|---|---|---|
  | LS193 | 17/18 | 16/15 | 27/30 | 24/25 | 23 | typ, 5 V, 15 pF, ls193 p8 |
  | ALS193A (SN74) | 16/18 | 16/18 | 19/17 | 30/28 | 17 | max, 50 pF, als193a p6 |
  | HC (SN74HC193) | 24 | 24 | 40 | 40 | 36 | typ, 4.5 V, 50 pF, hc193 p7 |
  | HC (CD74HC193) | 25 | 25 | 43 | 44 | 40 | max, 4.5 V, cd74hc192 p6–7 |
  | HCT (CD74HCT193) | 27 | 27 | 40 | 46 | 43 | max; typ at 5 V/15 pF: 11, 11, 17, 21, 18 |

- **VCC:**
  - LS: SN54 4.5–5.5; SN74 4.75–5.25.
  - ALS: 4.5–5.5.
  - HC: 2–6.
  - HCT: 4.5–5.5.
- **Obsolete / not active:** CD74HC193M, CD74HC193MT, SN74HC193D, SN74HC193PW, SN74LS193D.
  Last: SN74HC193NSR.
- **Notes:**
  - als193a p1 says CO is active "while the count is 9 or 15". This is boilerplate; the part is binary
    (15 only).
  - The cd74hc192 HCT table prints "CPU to TCD", which is a typo for CPD→TCD.

## 74_561

- **Title / function:** "Synchronous 4-bit counters with 3-state outputs" — binary. It has both synchronous
  and asynchronous clear, and both synchronous and asynchronous load.
- **Datasheets:** counters/sn74als561a.pdf: p1–2, p5, p7.
- **Families & prefixes:** ALS: SN74ALS561AN (the only census row; Last).
- **Military:** none.
- **Pinouts:**
  - **p20** — N (census); DW and J in the datasheet:
    1 ALOAD input, 2 CLK input, 3 A input, 4 B input, 5 C input, 6 D input, 7 ENP input, 8 ACLR input,
    9 SCLR input, 10 GND ground, 11 SLOAD input, 12 ENT input, 13 QD output, 14 QC output, 15 QB output,
    16 QA output, 17 OE input, 18 CCO output, 19 RCO output, 20 VCC power.
  - The FK in the datasheet uses identical numbering, but there is no census row.
- **Behavior** (function table p2):
  - **OE:** active low. It 3-states QA–QD only; RCO and CCO are always driven. Counting is independent of OE.
  - **ACLR:** active low, async. Q ← 0. "Overrides all other functions."
  - **ALOAD:** active low, async, level-sensitive. Q follows A..D while low (data→Q path specified).
  - **SCLR:** active low, sync. At CLK↑, Q ← 0. "SCLR overrides only the other synchronous functions."
  - **SLOAD:** active low, sync. At CLK↑, Q ← A..D.
  - **Count:** at CLK↑ when ENP = ENT = H (active high) and ACLR, ALOAD, SCLR and SLOAD are all H. Mod 16.
  - **Priority:** ACLR > ALOAD > SCLR > SLOAD > count (ENP·ENT) > hold. This is the table order.
  - **RCO** (active high) = ENT · Q=15. Symbol "1 (CT=15) G9", G1 = ENT. It does not depend on ENP.
  - **CCO** (clocked carry, active high) = RCO · ENT · ENP · ¬CLK.
    - Symbol "7,1,2,9": Z7 = CLK with a negation indicator, G1 = ENT, G2 = ENP, G9 = RCO.
    - Text: "a high-level pulse for a duration equal to that of the low level of the clock when RCO is high
      and the counter is enabled (ENP and ENT are high); otherwise, CCO is low … does not have the glitches".
    - It does not become active until CLK returns low.
  - **Power-up:** not stated.
- **Timing** (SN74ALS561A max, 4.5–5.5 V, 50 pF, p7):
  - CLK→Q 12/18; CLK→RCO 29/24; CLK→CCO 26/16.
  - ALOAD→Q 35/23; ALOAD→RCO 40/30; ALOAD→CCO 55/33.
  - Data→Q 30/22.
  - ENT→RCO 16/14; ENT→CCO 32/12; ENP→CCO 18/12.
  - ACLR→Q 22.
  - OE: tPZH 19, tPZL 23, tPHZ 10, tPLZ 15.
  - SN54 values: 15/21, 35/29, 35/20, 38/27, 50/35, 65/42, 35/27, 20/18, 35/15, 22/14, 28; OE 24/28/12/20.
- **VCC:** 4.5–5.5 (SN54 and SN74, p5).
- **Obsolete / not active:** SN74ALS561AN (Last).
- **Notes:** none.

## 74_569

- **Title / function:** "Synchronous 4-bit up/down decade and binary counters with 3-state outputs" — 569
  is the binary member. It has synchronous and asynchronous clear and a synchronous load only.
- **Datasheets:** counters/sn74als569a.pdf: p1–3, p8, p10.
- **Families & prefixes:** ALS: SN74ALS569A*, SN54ALS569AJ, SNJ54ALS569A*.
- **Military numbers:** 8302502RA and 83025022A → ALS569A.
- **Pinouts:**
  - **p20** — N, DW, J:
    1 U/D input, 2 CLK input, 3 A input, 4 B input, 5 C input, 6 D input, 7 ENP input, 8 ACLR input,
    9 SCLR input, 10 GND ground, 11 LOAD input, 12 ENT input, 13 QD output, 14 QC output, 15 QB output,
    16 QA output, 17 OE input, 18 CCO output, 19 RCO output, 20 VCC power.
  - **p20 FK:** identical numbering 1–20 (p1: top 3 2 1 20 19 = A, CLK, U/D, VCC, RCO).
- **Behavior** (function table p2):
  - **OE:** active low, 3-states Q only.
  - **ACLR:** active low, async. Q ← 0, overrides all.
  - **SCLR:** active low, sync. At CLK↑, Q ← 0.
  - **LOAD:** active low, sync. At CLK↑, Q ← A..D.
  - **Count:** at CLK↑ when **ENT = ENP = L** (active low) and ACLR, SCLR and LOAD are H.
    - U/D = H → up; U/D = L → down. Mod 16.
  - **Priority:** ACLR > SCLR > LOAD > count > hold.
  - **RCO** (active low) = ¬( ¬ENT · [ (U/D · Q=15) + (¬U/D · Q=0) ] ). Symbol "1,7(CT=15) G9 / 2,7(CT=0)
    G9", G7 = ENT. It does not depend on ENP.
  - **CCO** (active low) = ¬( RCO_active · ¬ENT · ¬ENP · ¬CLK ). Symbol "6,7,8,9": Z6 = CLK low,
    G7 = ENT, G8 = ENP, G9 = RCO.
    - Text: it is a low pulse for the CLK-low duration when RCO is low and both enables are low.
- **Timing** (SN74ALS569A max, 50 pF, p10):
  - CLK→Q 13/16; CLK→RCO 28/19; CLK→CCO 13/25.
  - U/D→RCO 23/19.
  - ENT→RCO 15/13; ENT→CCO 13/23; ENP→CCO 12/14.
  - ACLR→Q 20.
  - OE: tPZH 18, tPZL 24, tPHZ 10, tPLZ 13.
  - SN54: 21/19, 37/28, 17/30, 31/33, 21/20, 18/32, 18/18, 25; OE 23/29/12/29.
- **VCC:** 4.5–5.5 (p8).
- **Obsolete / not active:** SN74ALS569ADWR (Last).
- **Notes:**
  - SN74ALS568A (the decade version; CTRDIV10, RCO at CT=9 up) is in the same PDF but has no census rows.
  - No state diagram is printed for 568A.

## 74_867

- **Title / function:** "Synchronous 8-bit up/down counters" with S0/S1 mode select. **Asynchronous
  clear.**
- **Datasheets:** counters/sn74als867a.pdf: p1–3, p4, p7–9, p12–13.
- **Families & prefixes:**
  - ALS: SN74ALS867ADW
  - AS: SN54AS867JT, SNJ54AS867JT (SN74AS867 is printed but has no census row)
- **Military numbers:** 5962-8966801LA → AS867.
- **Pinouts:**
  - **p24** — DW, JT, NT:
    1 S0 input, 2 S1 input, 3 A input, 4 B input, 5 C input, 6 D input, 7 E input, 8 F input, 9 G input,
    10 H input, 11 ENT input, 12 GND ground, 13 RCO output, 14 CLK input, 15 QH output, 16 QG output,
    17 QF output, 18 QE output, 19 QD output, 20 QC output, 21 QB output, 22 QA output, 23 ENP input,
    24 VCC power.
  - **p28 FK** (p1; no census row):
    1 NC, 2 S0, 3 S1, 4 A, 5 B, 6 C, 7 D, 8 NC, 9 E, 10 F, 11 G, 12 H, 13 ENT, 14 GND, 15 NC, 16 RCO, 17 CLK,
    18 QH, 19 QG, 20 QF, 21 QE, 22 NC, 23 QD, 24 QC, 25 QB, 26 QA, 27 ENP, 28 VCC.
- **Behavior:**
  - **Mode table** (p2):

    | S1 | S0 | Function |
    |---|---|---|
    | L | L | Clear |
    | L | H | Count down |
    | H | L | Load |
    | H | H | Count up |

  - **Clear (S1=S0=L):** **asynchronous on 867**. Q ← 0 immediately, without a clock. Symbol "0R"
    (p3–4). A minimum clear pulse width is specified (ALS 10 ns; AS 10/12.5 ns).
  - **Load (S1=H, S0=L):** synchronous at CLK↑, Q ← A..H. It is not gated by the enables (symbol "2,6D").
  - **Count:** at CLK↑ in mode 01 (down) or 11 (up), only when **ENP = ENT = L** (active low). Mod 256.
    Otherwise hold.
  - **Priority:** clear and load are selected by mode, so they are mutually exclusive. The enables only
    gate counting.
  - **RCO** (active low), active only in the count modes:
    - **ALS867A:** RCO = ¬( ¬ENT · [ (S1·S0 · Q=255) + (¬S1·S0 · Q=0) ] ). Symbol "1,4CT=0 / 3,4CT=255",
      G4 = ENT. It does **not** depend on ENP: "For the SN74ALS867A and SN74ALS869, any time ENT is taken
      high, RCO either goes or remains high."
    - **AS867:** RCO = ¬( ¬ENT · ¬ENP · [ … same … ] ). Symbol "1,4,5CT=0", G5 = ENP. "For the 'AS867 and
      'AS869, any time ENP and/or ENT is taken high, RCO goes or remains high." ENP→RCO delays are printed
      for AS only.
    - Combinational; it follows S0/S1 (S→RCO delays).
  - **Mode-change hazard:** AS867 specifies tskew 7–8 ns max between S0 and S1 "to avoid inadvertent clear",
    because passing through S1S0=00 asynchronously clears the 867. A model should clear whenever S1=S0=L,
    even transiently.
  - **Power-up:** not stated.
- **Timing:**
  - SN74ALS867A max, 50 pF (p9): CLK→Q 16/16; CLK→RCO 14/14; ENT→RCO 14/9; S0/S1 (clear mode)→Q 26;
    S0/S1 (count)→RCO 16/16; S0/S1 (clear mode)→RCO tPLH 16.
  - AS867 max (p13), SN74 / SN54: CLK→Q 11/15 and 12/16; CLK→RCO 22/16 and 31/19; ENT→RCO 10/17 and 19/21;
    ENP→RCO 14/17 and 16/21; clear (S0 or S1 low)→Q 21 and 23.
- **VCC:** 4.5–5.5 (ALS p8; AS p12, SN54 and SN74).
- **Obsolete / not active:** SN54AS867JT (Last).
- **Notes:** none.

## 74_869

- **Title / function:** "Synchronous 8-bit up/down counters" — the same as 74_867 except that clear is
  **synchronous**.
- **Datasheets:** counters/sn74als867a.pdf: p1–3, p4, p7, p10–11, p14–15.
- **Families & prefixes:**
  - ALS: SN74ALS869DW, SN74ALS869DWE4
  - AS: SN74AS869DW, SN54AS869JT, SNJ54AS869JT
- **Military numbers:** 5962-8952601LA → AS869 (mark of SNJ54AS869JT).
- **Pinouts:** identical to 74_867 (p24 DW/JT/NT; p28 FK).
- **Behavior:** identical to 74_867 except:
  - **Clear (S1=S0=L) is synchronous:** Q ← 0 at CLK↑. Symbol "0,6R"; setup "S0 and S1 low (clear)" is
    13 ns ALS and 11/13 ns AS.
  - Holding S1=S0=L between clocks does not change Q.
  - **How 867 and 869 differ:** this is the **only** functional difference. 867 = asynchronous clear (mode
    00 clears immediately, "0R"); 869 = synchronous clear (mode 00 clears on the next CLK↑, "0,6R").
    - Pinout, modes, count, load and RCO are identical.
    - The ALS-vs-AS RCO difference (AS also gated by ENP) applies to both numbers.
- **Timing:**
  - SN74ALS869 max (p11): CLK→Q 16/16; CLK→RCO 14/14; ENT→RCO 14/9; S1 (count up/down)→RCO 15/15;
    S0 (clear/load)→RCO 16/12.
  - AS869 SN74 / SN54 (p15): CLK→Q 11/15 and 12/16; CLK→RCO 35/18 and 35/20; ENT→RCO 15/17 and 25/21;
    ENP→RCO 19/18 and 27/21.
- **VCC:** 4.5–5.5.
- **Obsolete / not active:** SN74AS869DW (Last).
- **Notes:** none.

---

## Global notes

- **Timing basis:**
  - ALS, AS, AC, ACT and the CD HC/HCT datasheets print **no typical** at 50 pF, so the values above are
    maxima (stated in each case).
  - The scanned LS, S and TTL datasheets give typicals at **CL = 15 pF** only.
- **Census packages:** no census row uses packages other than those listed. The NT (24-pin skinny DIP) and
  the 867 FK appear only in the datasheet.
- **TTL 160–163 timing:** the SN74160 TTL series (ls161a p12–13) shares one table: CLK→Q 13/15 (LOAD high),
  CLK→RCO 23/23, CLR→Q 26 typ. This could be reused for the TTL family of 74_161 if it is ever added.
- **Web sources** used only for the military-number mapping noted above:
  - [microchipusa JM38510/31504BEA](https://www.microchipusa.com/product/texas-instruments/counters-dividers/JM38510-31504BEA)
  - [1sourcecomponents JM38510/31504BEX 54LS161](https://www.1sourcecomponents.com/availability/JM38510/31504BEX--54LS161.htm)
  - [DigiKey JM38510/31504BEA](https://www.digikey.de/en/products/detail/texas-instruments/JM38510-31504BEA/13470487)
  - [sierraic 54ALS161B JM38510/38001BEA](https://www.sierraic.com/54ALS161BJM3851038001BEA)
  - [DigiKey JM38510/38002BEA](https://www.digikey.de/en/products/detail/texas-instruments/JM38510-38002BEA/13455014)
