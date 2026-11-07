# plan-19 device specs

One section per device that plan-19 adds (every device with an `add` row in
`parts.tsv`), read 2026-11-07 from the datasheets in `datasheets/decoder/`
and `datasheets/encoder/`. Page numbers are PDF page indices. Pin names drop
overbars; active levels are in each section's Function. Each section ends
with the device's model keys (plan-19-B §4) and the points where the model is
not exact. Devices in plan-19-A §3.1 order: model `binary`, then `priority`,
then `seg7`.

Devices of §3.1 with no section, because no part of theirs is added
(`parts.tsv` reasons): 74_131 (SN74ALS131N, SN74AS131AD: no TI datasheet),
74_159 (SNJ54159J: no TI datasheet), 74_445 (SN74LS445N: no TI datasheet),
74_11238 (74AC11238N: no TI datasheet), 74_278 (no TI datasheet), 4056B
(CD4056BNS: not in the cd4056b ordering table, which lists CD4056BE, BF3A,
BM96, BPW) and 4511B (CD4511BNS: not in the cd4511b ordering table, which
lists CD4511BNSR).

## 74_138 — 3-line to 8-line decoders/demultiplexers

- **PDFs:** decoder/sn74hc138.pdf p.3 (pinout, pin functions), p.4 (ROC), p.6 (timing), p.10 (function table); decoder/sn74hct138.pdf p.3 (pinout), p.4 (ROC), p.5 (timing); decoder/sn54hct138.pdf p.3 (pinout), p.4 (ROC), p.5 (timing; same content as sn74hct138); decoder/sn74hcs138.pdf p.3 (pinout: D, PW, DYY, BQB), p.4 (ROC), p.6 (timing), p.11 (function table); decoder/sn74hcs138-q1.pdf p.3 (D, PW, DYY, WBQB); decoder/cd74hc138.pdf p.3 (pinout: J, N, D, NS, PW), p.4 (ROC), p.6 (timing), p.10 (function tables); decoder/cd74hc138-q1.pdf p.1 (pinout D); decoder/sn74hc138-q1.pdf p.1 (pinout D, PW); decoder/cd74ac138.pdf p.3 (pinout: J, D, N, PW, BQB), p.4 (ROC), p.6 (timing at 5 V), p.10 (function table); decoder/sn74ac138-q1.pdf p.3 (PW, BQB), p.4 (ROC); decoder/cd74act138.pdf p.3 (pinout: J, D, N, PW, BQB), p.4 (ROC), p.5 (timing); decoder/sn74act138-q1.pdf p.3 (PW, BQB); decoder/sn74ahc138.pdf p.3 (pinout: D, DB, DGV, N, NS, PW, RGY; J/W), p.5 (ROC), p.7 (timing at 5 V); decoder/sn74ahc138-q1.pdf p.3 (PW, WBQB); decoder/sn74ahct138.pdf p.3 (pinout: D, DB, DGV, N, NS, PW, RGY; J/W), p.5 (ROC), p.6 (timing); decoder/sn74ahct138q-q1.pdf p.3 (D, PW, BQB), p.4 (ROC); decoder/sn74ahct138-ep.pdf p.1 (PW), p.3 (ROC); decoder/sn74als138a.pdf p.1 (pinout D, J, N), p.2 (function table), p.4 (ALS ROC + timing), p.5 (AS ROC), p.6 (AS timing); decoder/sn74f138.pdf p.1 (pinout D, J, N), p.2 (function table), p.4 (ROC), p.5 (timing); decoder/sn74ls138.pdf p.1 (pinout D, J, N, W; scanned), p.2 (function table), p.4 (LS ROC + timing), p.5 (S ROC), p.6 (S timing); decoder/sn74lv138a.pdf p.4 (pinout: D, DB, DGV, NS, PW, RGY, BQB), p.6 (ROC), p.7 (timing); decoder/sn74lv138a-q1.pdf p.3 (BQB), p.4 (ROC); decoder/sn74lv138at.pdf p.1 (pinout: D, DB, DGV, NS, PW, RGY), p.2 (function table), p.4 (ROC), p.5 (timing); decoder/sn74lvc138a.pdf p.3 (pinout: D, DB, DGV, NS, PW, RGY, RSV), p.4 (pin functions), p.6 (ROC), p.7 (timing), p.11 (function table); decoder/sn74lvc138a-q1.pdf p.1 (D, PW), p.3 (ROC); decoder/sn74lvc138a-ep.pdf p.1 (PW), p.4 (ROC); decoder/cy74fct138t.pdf p.1 (pinout D, Q, SO), p.2 (function table), p.4 (ROC), p.5 (timing)
- **Families:** AC (CD54AC138, CD74AC138, SN74AC138-Q1), ACT (CD74ACT138, SN74ACT138-Q1), AHC (SN74AHC138, SN74AHC138-Q1, SNJ54AHC138), AHCT (SN74AHCT138, SN74AHCT138-Q1 / CAHCT138Q-Q1, SN74AHCT138-EP), ALS (SN74ALS138A), AS (SN74AS138), F (SN54F138, SN74F138), FCT (CY74FCT138T, -AT, -CT speed grades), HC (SN74HC138, SN74HC138-Q1, CD54HC138, CD74HC138, CD74HC138-Q1), HCS (SN74HCS138, SN74HCS138-Q1), HCT (SN54HCT138, SN74HCT138, CD54HCT138, CD74HCT138), LS (SN74LS138), LV (SN74LV138A, SN74LV138A-Q1, SN74LV138AT), LVC (SN74LVC138A, SN74LVC138A-Q1 / CLVC138AQ-Q1, SN74LVC138A-EP), S (SN74S138A)
- **Pin names:** SN74 names A, B, C (select, A = LSB), G1 (active-high enable), G2A, G2B (active-low enables), Y0–Y7 (active-low outputs), used by sn74hc138, sn74hct138, sn74ahct138, sn74lvc138a, sn74lv138at, sn74ls138, sn74als138a, sn74f138, cd74ac138, cd74act138. Other sheets print different names **at the same pins** (checked pin by pin in each pin table / drawing): cd74hc138, sn74hcs138, sn74ahc138, sn74lv138a use A0 A1 A2 (pins 1–3), G0 (pin 4, active low), G1 (pin 5, active low), G2 (pin 6, active high) — note their "G1" is pin 5 (= SN G2B), not pin 6; cd74hc138 p.2 records the rename from E1/E2/E3. cy74fct138t uses A0 A1 A2, E1 E2 (pins 4, 5, active low), E3 (pin 6, active high), O0–O7 (O7 at pin 7, O0 at pin 15). Every sheet has Y7/O7 at pin 7, GND 8, Y6..Y0 at 9..15, VCC 16. The device uses the SN names for every part.
- **Pinouts:** one style, `p16` (pin n of every package = DIP pin n):

  | Pin | Name | Role |
  |---|---|---|
  | 1 | A | input |
  | 2 | B | input |
  | 3 | C | input |
  | 4 | G2A | input |
  | 5 | G2B | input |
  | 6 | G1 | input |
  | 7 | Y7 | output |
  | 8 | GND | ground |
  | 9 | Y6 | output |
  | 10 | Y5 | output |
  | 11 | Y4 | output |
  | 12 | Y3 | output |
  | 13 | Y2 | output |
  | 14 | Y1 | output |
  | 15 | Y0 | output |
  | 16 | VCC | power |

  Codes on `p16`: N, D, NS, DB, DGV, PW, J, W (sn74hc138 p.3 one pin column "SOIC, SSOP, PDIP, SO, TSSOP, CDIP, CFP"); RGY (sn74ahc138 p.3, sn74lv138a p.4, sn74lv138at p.1 drawing: pin 1 A … 16 VCC); BQB/WBQB (cd74ac138 p.3, sn74hcs138 p.3, sn74lv138a p.4: same numbering, thermal pad unnumbered); DYY (sn74hcs138 p.3, same column as D and PW); RSV (sn74lvc138a p.3 drawing: 1 A, 2 B, 3 C, 4 G2A, 5 G2B, 6 G1, 7 Y7, 8 GND, 9 Y6 … 15 Y0, 16 VCC — same numbering as the DIP although the drawing is rotated; p.4 lists RSV in the same column as SOIC); DBQ (QSOP "Q") and DW (SOIC "SO") (cy74fct138t p.1). No FK part is in the list, so no `fk20`.
- **Function:** (sn74hc138 p.10, Table 8-1)

  | G1 | G2A | G2B | C | B | A | Y0 | Y1 | Y2 | Y3 | Y4 | Y5 | Y6 | Y7 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | X | H | X | X | X | X | H | H | H | H | H | H | H | H |
  | X | X | H | X | X | X | H | H | H | H | H | H | H | H |
  | L | X | X | X | X | X | H | H | H | H | H | H | H | H |
  | H | L | L | L | L | L | L | H | H | H | H | H | H | H |
  | H | L | L | L | L | H | H | L | H | H | H | H | H | H |
  | H | L | L | L | H | L | H | H | L | H | H | H | H | H |
  | H | L | L | L | H | H | H | H | H | L | H | H | H | H |
  | H | L | L | H | L | L | H | H | H | H | L | H | H | H |
  | H | L | L | H | L | H | H | H | H | H | H | L | H | H |
  | H | L | L | H | H | L | H | H | H | H | H | H | L | H |
  | H | L | L | H | H | H | H | H | H | H | H | H | H | L |

  Model `binary`: one unit, `address` [A, B, C] (LSB first), `outputs` [Y0 … Y7], `enables` [{G1 high}, {G2A low}, {G2B low}], `outputsActive` "low"; `unselected`/`disabled` default "inactive" (H).
- **Timing:** rise = tPLH, fall = tPHL, path A/B/C → any Y.

  | Family | Sheet | Rise / fall (ns) | Basis | Page |
  |---|---|---|---|---|
  | AC | cd74ac138 (5 of 7 AC parts) | 10 / 10 | max, 5 V ± 0.5 V, 50 pF, −40 to 85 °C (no typ printed) | p.6 |
  | ACT | cd74act138 (4 of 6) | 10.9 / 10.9 | max, 5 V, 50 pF, −40 to 85 °C (no typ) | p.5 |
  | AHC | sn74ahc138 | 7.2 / 7.2 | typ, 5 V, 50 pF | p.7 |
  | AHCT | sn74ahct138 | 8.1 / 8.1 | typ, 5 V, 50 pF | p.6 |
  | ALS | sn74als138a | 22 / 18 | max, 4.5–5.5 V, 50 pF, SN74 (no typ) | p.4 |
  | AS | sn74als138a | 10 / 9.5 | max, 4.5–5.5 V, 50 pF, SN74 (no typ) | p.6 |
  | F | sn74f138 | 5.2 / 5.7 | typ, 5 V, 50 pF | p.5 |
  | FCT | cy74fct138t | 5.8 / 5.8 | max, 50 pF, CY74FCT138AT grade (no typ; see checks) | p.5 |
  | HC | sn74hc138 | 18 / 18 | typ, 4.5 V, 50 pF (tpd) | p.6 |
  | HCS | sn74hcs138 | 8 / 8 | typ, 4.5 V, 50 pF (tpd) | p.6 |
  | HCT | sn74hct138 | 23 / 23 | typ, 4.5 V, 50 pF (tpd) | p.5 |
  | LS | sn74ls138 | 21 / 20 | typ, 5 V, 15 pF, the 3-levels-of-delay row (2-level row 11 / 18) | p.4 |
  | LV | sn74lv138a | 10.3 / 10.3 | typ, 3.3 V, 50 pF (tpd) | p.7 |
  | LVC | sn74lvc138a | 6.7 / 6.7 | max, 3.3 V ± 0.3 V, 50 pF (no typ) | p.7 |
  | S | sn74ls138 | 7.5 / 8 | typ, 5 V, 15 pF, 3-levels row (2-level row 4.5 / 7) | p.6 |

- **Supply:** (SN74 Recommended Operating Conditions)

  | Family | VCC | Notes |
  |---|---|---|
  | AC | 1.5–5.5 V (cd74ac138 p.4; CD54 same) | SN74AC138-Q1 is 1.5–6 V (sn74ac138-q1 p.4): parts SN74AC138PWRQ1, SN74AC138WBQBRQ1 need part `"vcc": [1.5, 6]` |
  | ACT | 4.5–5.5 V (cd74act138 p.4; -Q1 same) | |
  | AHC | 2–5.5 V (sn74ahc138 p.5; SN54 and -Q1 same) | inputs 0–5.5 V = VCC max, no inputMax |
  | AHCT | 4.5–5.5 V | sn74ahct138 p.5 ROC prints VCC 1.5–5.5 V (SN54 and SN74); sn74ahct138q-q1 p.4 and sn74ahct138-ep p.3 print 4.5–5.5 V and every AHCT spec is at 5 V ± 0.5 V, so 4.5–5.5 V is used |
  | ALS, AS | 4.5–5.5 V (sn74als138a p.4, p.5) | |
  | F | 4.5–5.5 V (sn74f138 p.4; SN54 same) | |
  | FCT | 4.75–5.25 V (cy74fct138t p.4) | CY54: 4.5–5.5 V (no CY54 part listed) |
  | HC | 2–6 V (sn74hc138 p.4; cd74hc138, -Q1 same) | |
  | HCS | 2–6 V (sn74hcs138 p.4; -Q1 same) | |
  | HCT | 4.5–5.5 V (sn74hct138 p.4; SN54, CD54/74 same) | |
  | LS | 4.75–5.25 V (sn74ls138 p.4) | SN54: 4.5–5.5 V (no SN54 part listed) |
  | LV | 2–5.5 V (sn74lv138a p.6; -Q1 same) | SN74LV138AT is 4.5–5.5 V (sn74lv138at p.4, TTL-level inputs): SN74LV138ATDBR, SN74LV138ATDGVR, SN74LV138ATDR, SN74LV138ATPWR, SN74LV138ATRGYR need part `"vcc": [4.5, 5.5]` |
  | LVC | 1.65–3.6 V, inputMax 5.5 (sn74lvc138a p.6) | SN54LVC138A 2–3.6 V; sn74lvc138a-q1 and sn74lvc138a-ep print 2–3.6 V: CLVC138AQPWRG4Q1, SN74LVC138AQDRG4Q1, SN74LVC138AQDRQ1, SN74LVC138AMPWTEP, SN74LVC138AQPWREP need part `"vcc": [2, 3.6]` |
  | S | 4.75–5.25 V (sn74ls138 p.5) | SN54: 4.5–5.5 V |

- **Checks against the model:**
  - Exact for the logic (one enabled 3-to-8 unit, active-low outputs).
  - One timing entry per family: FCT has three speed grades in one sheet (CY74FCT138T 9 ns, -AT 5.8 ns, -CT 5 ns max); the entry uses AT (3 of the 6 FCT parts); CT parts are faster and the T part slower than modeled.
  - TTL LS/S print delays by "levels of delay"; the model has one delay per family, so the 3-level row is used for every address change.
  - The LV-AT parts are TTL-input, 5 V-only parts in family LV; only the VCC range is carried per part (above).
  - Package drawings missing in the cited sheets (numbering assumed = DIP, standard SOP): SN74AS138NSR, SN74F138NSR, SN74LS138NSR (NS not drawn; sheets show D/J/N only). CD74AC138BQBR / CD74ACT138BQBR: drawn. SN74AHC138 sheet cites a "WBQB" thermal pad but no SN74AHC138 BQB part is listed (the BQB parts are -Q1).

## 74_238 — 3-line to 8-line decoders/demultiplexers, noninverting outputs

- **PDFs:** decoder/cd74hc138.pdf p.3 (pinout J, N, D, NS, PW; covers CD74HC238/HCT238), p.4 (ROC), p.6 (timing), p.10 (Table 7-2 function table 'HC238/'HCT238); decoder/cd74ac238.pdf p.3 (pinout D, N, PW, BQB), p.4 (ROC), p.6 (timing at 5 V), p.9–10 (function table); decoder/cd74act238.pdf p.1 (pinout E = PDIP), p.2 (function table); decoder/sn74ac238-q1.pdf p.3 (pinout PW, BQB), p.4 (ROC), p.6 (timing); decoder/sn74act238.pdf p.3 (pinout PW, BQB), p.4 (ROC), p.5 (timing), p.9–10 (function table); decoder/sn74act238-q1.pdf p.3; decoder/sn74ahc238.pdf p.3 (pinout PW, BQB), p.4 (ROC), p.5–6 (timing); decoder/sn74ahc238-q1.pdf p.3 (PW), p.4 (ROC); decoder/sn74hcs238.pdf p.3 (pinout D, PW, DYY, BQB), p.4 (ROC), p.6 (timing), p.10 (function table); decoder/sn74hcs238-q1.pdf p.3
- **Families:** AC (CD74AC238, SN74AC238-Q1), ACT (CD74ACT238, SN74ACT238, SN74ACT238-Q1), AHC (SN74AHC238, SN74AHC238-Q1), HC (CD74HC238), HCS (SN74HCS238, SN74HCS238-Q1), HCT (CD74HCT238)
- **Pin names:** A, B, C, G1, G2A, G2B, Y0–Y7 — the same names as 74_138 (identical pinout; only output polarity differs), as printed by cd74ac238 p.3 and cd74act238 p.1. The SN74AHC238/ACT238/AC238-Q1/HCS238 and CD74HC238 sheets print A0 A1 A2 (pins 1–3), G0 (pin 4, active low), G1 (pin 5, active low), G2 (pin 6, active high) at the same pins (checked in each pin table). sn74ahc238 itself mixes both: its pin table says A0/G0/G1/G2, its switching table (p.5–6) says A, B, C, G1, G2A, G2B. Choice: keep the '138 names so the two devices read alike (alternative: A0–A2/G0–G2, the majority of '238 sheets; switching is a rename only).
- **Pinouts:** `p16`, identical to 74_138 (pin 1 A, 2 B, 3 C, 4 G2A, 5 G2B, 6 G1, 7 Y7, 8 GND, 9 Y6, 10 Y5, 11 Y4, 12 Y3, 13 Y2, 14 Y1, 15 Y0, 16 VCC). Codes on `p16`: N, D, NS, PW (cd74hc138 p.3), BQB (cd74ac238 p.3, sn74act238 p.3, sn74hcs238 p.3 — one pin column with the leaded packages, thermal pad unnumbered), DYY (sn74hcs238 p.3).
- **Function:** (sn74hcs238 p.10; cd74act238 p.2 the same with SN names)

  | G2 (=G1 SN) | G1 (=G2B) | G0 (=G2A) | A2 | A1 | A0 | Y0 | Y1 | Y2 | Y3 | Y4 | Y5 | Y6 | Y7 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | X | X | H | X | X | X | L | L | L | L | L | L | L | L |
  | L | X | X | X | X | X | L | L | L | L | L | L | L | L |
  | X | H | X | X | X | X | L | L | L | L | L | L | L | L |
  | H | L | L | L | L | L | H | L | L | L | L | L | L | L |
  | H | L | L | L | L | H | L | H | L | L | L | L | L | L |
  | H | L | L | L | H | L | L | L | H | L | L | L | L | L |
  | H | L | L | L | H | H | L | L | L | H | L | L | L | L |
  | H | L | L | H | L | L | L | L | L | L | H | L | L | L |
  | H | L | L | H | L | H | L | L | L | L | L | H | L | L |
  | H | L | L | H | H | L | L | L | L | L | L | L | H | L |
  | H | L | L | H | H | H | L | L | L | L | L | L | L | H |

  Model `binary`: one unit, `address` [A, B, C], `outputs` [Y0 … Y7], `enables` [{G1 high}, {G2A low}, {G2B low}], `outputsActive` "high".
- **Timing:** rise = tPLH, fall = tPHL, path A/B/C → any Y.

  | Family | Sheet | Rise / fall (ns) | Basis | Page |
  |---|---|---|---|---|
  | AC | cd74ac238 (3 of 4 AC parts) | 13.6 / 13.6 | max, 5 V ± 0.5 V, 50 pF, −40 to 85 °C (no typ) | p.6 |
  | ACT | sn74act238 | 7.2 / 7.3 | typ, 5 V, 50 pF | p.5 |
  | AHC | sn74ahc238 | 6.5 / 6.5 | typ, 5 V ± 0.5 V, 50 pF | p.6 |
  | HC | cd74hc138 | 30 / 30 | max, 4.5 V, 50 pF, 25 °C (the only typ, 13 ns, is at 5 V 15 pF) | p.6 |
  | HCS | sn74hcs238 | 8 / 8 | typ, 4.5 V, 50 pF | p.6 |
  | HCT | cd74hc138 | 35 / 35 | max, 4.5 V, 50 pF, 25 °C | p.6 |

- **Supply:** AC 1.5–5.5 V (cd74ac238 p.4) — SN74AC238PWRQ1 is 1.5–6 V (sn74ac238-q1 p.4) and needs part `"vcc": [1.5, 6]`; ACT 4.5–5.5 V (sn74act238 p.4; -Q1 same); AHC 2–5.5 V (sn74ahc238 p.4; -Q1 same); HC 2–6 V, HCT 4.5–5.5 V (cd74hc138 p.4); HCS 2–6 V (sn74hcs238 p.4; -Q1 same). No SN54/CD54 part listed.
- **Checks against the model:** exact (one 3-to-8 unit, active-high outputs, unselected and disabled outputs low = inactive).

## 74_137 — 3-line to 8-line decoders/demultiplexers with address latches

- **PDFs:** decoder/sdls132.pdf p.1 (pinout D, J, N, W, FK; scanned), p.2 (function table), p.3 (logic diagram), p.4 (ROC, timing); decoder/sn74als137a.pdf p.1 (pinout D, J, N, FK; function table), p.3 (ROC), p.4 (timing); decoder/cd74hc137.pdf p.2 (pinout, truth tables), p.4 (operating conditions), p.6–7 (switching); decoder/sn74hcs137.pdf p.3 (pinout D, PW), p.4 (ROC), p.6 (timing), p.11 (function table); decoder/sn74hcs137-q1.pdf p.3, p.4
- **Families:** ALS (SN74ALS137A), HC (CD74HC137), HCS (SN74HCS137, SN74HCS137-Q1), HCT (CD74HCT137), LS (SN54LS137)
- **Pin names:** A, B, C, LE, G2, G1, Y0–Y7 (sn74als137a p.1). sdls132 names pin 4 GL (latch enable, overbar); cd74hc137 names A0 A1 A2 (pin 3 misprinted "A3" in the drawing), LE, OE1 (pin 5, active low), OE0 (pin 6, active high); sn74hcs137 names A0 A1 A2, LE, G1 (pin 5, active low), G0 (pin 6, active high). All sheets: pin 4 = latch enable, pin 5 = active-low enable, pin 6 = active-high enable, Y7 pin 7, GND 8, Y6..Y0 pins 9..15, VCC 16. LE is used (3 of 4 sheets) rather than GL; G2/G1 follow the SN sheets (G2 = active low, G1 = active high, as in '138's G2A/G2B vs G1).
- **Pinouts:** `p16`:

  | Pin | Name | Role |
  |---|---|---|
  | 1 | A | input |
  | 2 | B | input |
  | 3 | C | input |
  | 4 | LE | input |
  | 5 | G2 | input |
  | 6 | G1 | input |
  | 7 | Y7 | output |
  | 8 | GND | ground |
  | 9–15 | Y6, Y5, Y4, Y3, Y2, Y1, Y0 | output |
  | 16 | VCC | power |

  Codes on `p16`: N, PW, D (cd74hc137 p.2: "PDIP, TSSOP", "PDIP, SOIC"; sn74hcs137 p.3: D, PW), J (sdls132 p.1, "pin numbers shown are for D, J, N, and W").
- **Function:** (sdls132 p.2; sn74als137a p.1 identical)

  | LE (GL) | G1 | G2 | C | B | A | Y0 | Y1 | Y2 | Y3 | Y4 | Y5 | Y6 | Y7 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | X | X | H | X | X | X | H | H | H | H | H | H | H | H |
  | X | L | X | X | X | X | H | H | H | H | H | H | H | H |
  | L | H | L | L | L | L | L | H | H | H | H | H | H | H |
  | L | H | L | L | L | H | H | L | H | H | H | H | H | H |
  | L | H | L | L | H | L | H | H | L | H | H | H | H | H |
  | L | H | L | L | H | H | H | H | H | L | H | H | H | H |
  | L | H | L | H | L | L | H | H | H | H | L | H | H | H |
  | L | H | L | H | L | H | H | H | H | H | H | L | H | H |
  | L | H | L | H | H | L | H | H | H | H | H | H | L | H |
  | L | H | L | H | H | H | H | H | H | H | H | H | H | L |
  | H | H | L | X | X | X | output for the stored address L; all others H |

  sdls132 p.1: "When GL goes from low to high, the address present at the select inputs is stored in the latches. Further address changes are ignored as long as GL remains high"; the enables act on the outputs independently of the latch (p.3 logic diagram: the enables gate the output NANDs after the latches). sn74hcs137 p.3 calls LE "active low" (transparent when low), same behaviour (p.11 table). Model `binary`: one unit, `address` [A, B, C], `outputs` [Y0 … Y7], `enables` [{G1 high}, {G2 low}], `latch` "LE", `latchActive` "high" (LE high holds), `outputsActive` "low".
- **Timing:** rise = tPLH, fall = tPHL, path A/B/C → Y (latch transparent).

  | Family | Sheet | Rise / fall (ns) | Basis | Page |
  |---|---|---|---|---|
  | ALS | sn74als137a | 20 / 20 | max, 4.5–5.5 V, 50 pF, SN74 (no typ) | p.4 |
  | HC | cd74hc137 | 36 / 36 | max, 4.5 V, 50 pF, 25 °C (typ only at 5 V 15 pF) | p.6 |
  | HCS | sn74hcs137 | 7 / 7 | typ, 4.5 V, 50 pF | p.6 |
  | HCT | cd74hc137 | 38 / 38 | max, 4.5 V, 50 pF, 25 °C | p.7 |
  | LS | sdls132 | 16 / 19 | typ, 5 V, 15 pF, the 3/3-levels row (other row: 11 tPLH 2 levels / 25 tPHL 4 levels) | p.4 |

- **Supply:** ALS 4.5–5.5 V (sn74als137a p.3, SN54 same); HC 2–6 V, HCT 4.5–5.5 V (cd74hc137 p.4); HCS 2–6 V (sn74hcs137 p.4; -Q1 same); LS SN74 4.75–5.25 V, SN54 4.5–5.5 V (sdls132 p.4) — the only LS part, SN54LS137J, is military and takes the SN54 range (`sn54vcc`).
- **Checks against the model:** exact for the logic. Latch setup/hold and LE→Y delay are not modeled separately (the address path delay is used when LE releases the latch).

## 74_237 — 3-line to 8-line decoders/demultiplexers with address latches, noninverting outputs

- **PDFs:** decoder/cd74hc137.pdf p.1 (ordering), p.2 (pinout, 'HC237/'HCT237 truth table), p.4 (operating conditions), p.6–7 (switching); decoder/sn74hcs237.pdf p.3 (pinout D, PW), p.4 (ROC), p.6 (timing), p.11 (function table); decoder/sn74hcs237-q1.pdf p.3, p.4
- **Families:** HC (CD74HC237), HCS (SN74HCS237, SN74HCS237-Q1), HCT (CD74HCT237)
- **Pin names:** A, B, C, LE, G2, G1, Y0–Y7, the same as 74_137 (same pinout, outputs active high). cd74hc137 prints A0 A1 A2, LE, OE1 (pin 5, active low), OE0 (pin 6, active high); sn74hcs237 prints A0 A1 A2, LE, G1 (pin 5, active low), G0 (pin 6, active high) — same pins.
- **Pinouts:** `p16`, identical to 74_137 (1 A, 2 B, 3 C, 4 LE, 5 G2, 6 G1, 7 Y7, 8 GND, 9 Y6 … 15 Y0, 16 VCC). Codes on `p16`: N, NS, PW, D (cd74hc137 p.2 "CD74HC237 (PDIP, SOIC, SOP, TSSOP)"; sn74hcs237 p.3), DYY (CD74HC237DYYR: **no DYY drawing in cd74hc137**, a 2003 sheet; numbering assumed = DIP as on every TI DYY 16-pin logic part, e.g. sn74hcs238 p.3).
- **Function:** (cd74hc137 p.2, 'HC237 truth table; sn74hcs237 p.11 the same)

  | LE | OE0 (=G1) | OE1 (=G2) | A2 | A1 | A0 | Y0 | Y1 | Y2 | Y3 | Y4 | Y5 | Y6 | Y7 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | X | X | H | X | X | X | L | L | L | L | L | L | L | L |
  | X | L | X | X | X | X | L | L | L | L | L | L | L | L |
  | L | H | L | L | L | L | H | L | L | L | L | L | L | L |
  | L | H | L | L | L | H | L | H | L | L | L | L | L | L |
  | L | H | L | L | H | L | L | L | H | L | L | L | L | L |
  | L | H | L | L | H | H | L | L | L | H | L | L | L | L |
  | L | H | L | H | L | L | L | L | L | L | H | L | L | L |
  | L | H | L | H | L | H | L | L | L | L | L | H | L | L |
  | L | H | L | H | H | L | L | L | L | L | L | L | H | L |
  | L | H | L | H | H | H | L | L | L | L | L | L | L | H |
  | H | H | L | X | X | X | depends upon the address previously applied while LE was low |

  Model `binary`: `address` [A, B, C], `outputs` [Y0 … Y7], `enables` [{G1 high}, {G2 low}], `latch` "LE", `latchActive` "high", `outputsActive` "high".
- **Timing:** rise = tPLH, fall = tPHL, path A → Y.

  | Family | Sheet | Rise / fall (ns) | Basis | Page |
  |---|---|---|---|---|
  | HC | cd74hc137 ('HC237 row) | 32 / 32 | max, 4.5 V, 50 pF, 25 °C (typ 13 only at 5 V 15 pF) | p.6 |
  | HCS | sn74hcs237 | 7 / 7 | typ, 4.5 V, 50 pF | p.6 |
  | HCT | cd74hc137 | 38 / 38 | max, 4.5 V, 50 pF, 25 °C | p.7 |

- **Supply:** HC 2–6 V, HCT 4.5–5.5 V (cd74hc137 p.4); HCS 2–6 V (sn74hcs237 p.4; -Q1 same).
- **Checks against the model:** exact for the logic.

## 74_11138 — 3-line to 8-line decoder/demultiplexer

- **PDFs:** decoder/74ac11138.pdf p.1 (package information D, N, NS, PW), p.3 (pinout, pin functions), p.4 (ROC), p.6 (timing at 5 V), p.7 (load circuit, CL = 50 pF), p.9 (function table)
- **Families:** AC (74AC11138)
- **Pin names:** A, B, C, G1, G2A, G2B, Y0–Y7, VCC, GND as printed (p.3).
- **Pinouts:** `p16` (center-pin power; its own pin map, not the '138's):

  | Pin | Name | Role |
  |---|---|---|
  | 1 | Y1 | output |
  | 2 | Y2 | output |
  | 3 | Y3 | output |
  | 4 | GND | ground |
  | 5 | Y4 | output |
  | 6 | Y5 | output |
  | 7 | Y6 | output |
  | 8 | Y7 | output |
  | 9 | G2B | input |
  | 10 | G2A | input |
  | 11 | G1 | input |
  | 12 | VCC | power |
  | 13 | C | input |
  | 14 | B | input |
  | 15 | A | input |
  | 16 | Y0 | output |

  Codes on `p16`: D, N (Figure 3-1 "D, N, or PW Package"), NS (listed in the p.1 package table, not named in Figure 3-1; SOP numbering = DIP).
- **Function:** (p.9) the 74_138 table: G1 = L or G2A = H or G2B = H → all Y high; G1 = H, G2A = G2B = L → Y(CBA) low, others high. Model `binary`: `address` [A, B, C], `outputs` [Y0 … Y7], `enables` [{G1 high}, {G2A low}, {G2B low}], `outputsActive` "low".
- **Timing:** AC rise 5.7, fall 6.2 ns: A/B/C → Y typ, 5 V ± 0.5 V, 25 °C, CL = 50 pF (p.6; load p.7).
- **Supply:** AC VCC 3–5.5 V (p.4). Inputs 0–VCC.
- **Checks against the model:** exact.

## 74_2414 — Memory decoder with on-chip supply voltage monitor

- **PDFs:** decoder/sn74bct2414.pdf p.1 (pinout DW, N; description), p.2 (logic symbol), p.3 (logic diagram), p.4 (function tables, note on 3-to-8 use), p.5 (ROC, electrical: VS), p.6 (switching)
- **Families:** BCT (SN74BCT2414)
- **Pin names:** as printed (p.1), overbars dropped: VS, SD (printed S̄D̄), 1A, 2A, 1B, 2B, 1G (printed 1Ḡ), 2G, G (printed Ḡ), GND, 2Y3–2Y0, 1Y3–1Y0, VBAT (printed V_bat), VCC.
- **Pinouts:** `p20` (DW and N, p.1):

  | Pin | Name | Role |
  |---|---|---|
  | 1 | VS | output |
  | 2 | SD | input |
  | 3 | 1A | input |
  | 4 | 2A | input |
  | 5 | 1B | input |
  | 6 | 2B | input |
  | 7 | 1G | input |
  | 8 | 2G | input |
  | 9 | G | input |
  | 10 | GND | ground |
  | 11 | 2Y3 | output |
  | 12 | 2Y2 | output |
  | 13 | 2Y1 | output |
  | 14 | 2Y0 | output |
  | 15 | 1Y3 | output |
  | 16 | 1Y2 | output |
  | 17 | 1Y1 | output |
  | 18 | 1Y0 | output |
  | 19 | VBAT | power |
  | 20 | VCC | power |

  Codes on `p20`: DW, N.
- **Function:** (p.4)

  | G | 1G | SD | 1B | 1A | 1Y0 | 1Y1 | 1Y2 | 1Y3 |
  |---|---|---|---|---|---|---|---|---|
  | H | X | X | X | X | H | H | H | H |
  | X | H | X | X | X | H | H | H | H |
  | X | X | L | X | X | H | H | H | H |
  | L | L | H | L | L | L | H | H | H |
  | L | L | H | L | H | H | L | H | H |
  | L | L | H | H | L | H | H | L | H |
  | L | L | H | H | H | H | H | H | L |

  | G | 2G | SD | 2B | 2A | 2Y0 | 2Y1 | 2Y2 | 2Y3 |
  |---|---|---|---|---|---|---|---|---|
  | H | X | X | X | X | H | H | H | H |
  | X | L | X | X | X | H | H | H | H |
  | X | X | L | X | X | H | H | H | H |
  | L | H | H | L | L | L | H | H | H |
  | L | H | H | L | H | H | L | H | H |
  | L | H | H | H | L | H | H | L | H |
  | L | H | H | H | H | H | H | H | L |

  NOTE (p.4): "For a 3-line to 8-line decoder, the following pins must be shorted: 1G to 2G, 1A to 2A and 1B to 2B" — 1G is active low and 2G active high, so the shared line becomes the third address bit. The logic symbol (p.2) agrees: Ḡ (pin 9) has a polarity indicator, SD (pin 2) has none (enable when high despite the overbar name: low = shut down), 1Ḡ has one, 2G has none; Ḡ and SD feed one common EN for both units. The units have **separate** address pins (1A/1B, 2A/2B); they share only G and SD.

  Model `binary`, two units: unit 0 `address` [1A, 1B], `outputs` [1Y0 … 1Y3], `enables` [{G low}, {1G low}, {SD high}]; unit 1 `address` [2A, 2B], `outputs` [2Y0 … 2Y3], `enables` [{G low}, {2G high}, {SD high}]; `outputsActive` "low"; `monitor` "VS".
- **Timing:** BCT rise 5, fall 5.8 ns: A or B → any Y typ, VCC = 5 V, 50 pF, 25 °C (p.6). Other rows: Ḡ → Y 4.5 / 5.5, SD → Y 6.5 / 6.5; VCC (fail) → Y 25 / 45 and VCC → VS 28 / 50 (not modeled).
- **Supply:** VCC 4.5–5.5 V and Vbat 4.5–5.5 V (p.5). Vbat powers the logic, VCC feeds the voltage monitor only (p.1). Inputs at most VCC + 0.5 V (absolute max, p.4).
- **Checks against the model:**
  - **VS polarity is opposite to the plan.** The plan (plan-19-B §4.1) and `decoder.c` (line 434–435) drive `monitor` to 1 push-pull for "supply good". The datasheet: VS is an open-collector output (p.3 logic diagram: VCOMP → inverter → NPN transistor to ground; p.2 symbol shows the open-circuit output mark), VOL(VS) ≤ 1 V at IOL = 20 mA with VCC = 4.5 V (p.5), and "in case VCC drops below 3.65 V … the voltage monitor forces the voltage-control (VS) … to the high level … This output is switched off when the on-chip supply voltage monitor detects a power failure" (p.1). So with the supply good VS is **low** (transistor on, sinking up to 20 mA); it goes high (released, pulled up externally) only on VCC failure. To match, the model should drive VS 0 (open-drain: drive 0, never release) while the supply is good. As written the JSON only names the pin; the drive level is the model's.
  - The Y outputs are totem-pole (IOH −400 µA, p.5): push-pull is right.
  - VCC-fail behaviour (all Y forced high below 3.65 V) and the Vbat/VCC split are not modeled; the part has two power pins (VCC, VBAT), both role power.
  - The plan's §4.1 example "units may share address pins: '155/'156/'BCT2414" is wrong for the BCT2414: its address pins are separate (sharing is done externally, p.4 note).

## 74_139 — Dual 2-line to 4-line decoders/demultiplexers

- **PDFs:** decoder/sn74hc139.pdf p.3 (pinout), p.4 (ROC), p.5 (timing), p.7 (function table); decoder/sn74hc139-q1.pdf p.1 (pinout, function table), p.3, p.4; decoder/cd74hc139.pdf p.1 (pinout), p.2 (truth table), p.3 (supply); decoder/sn74hct139.pdf p.3, p.4, p.5, p.7; decoder/sn74hcs139.pdf p.3, p.4, p.5, p.9; decoder/sn74hcs139-q1.pdf p.3, p.4, p.5, p.9; decoder/sn74ls139a.pdf p.1 (pinout, function table), p.3 (LS ROC + timing), p.4 (S ROC + timing) — scanned, read from the page images; decoder/sn74als139.pdf p.1, p.3; decoder/cd74ac139.pdf p.1, p.2, p.3, p.4; decoder/sn74ac139-q1.pdf p.3 (PW, BQB), p.5, p.7, p.13; decoder/cd74act139.pdf p.1, p.2, p.3, p.4; decoder/sn74act139.pdf p.3, p.4, p.5, p.9; decoder/sn74ahc139.pdf p.3 (D/DB/DGV/N/NS/PW, RGY), p.4, p.6, p.9; decoder/sn74ahc139-q1.pdf p.3 (PW, BQB), p.4, p.5, p.11; decoder/sn74ahct139.pdf p.1, p.2, p.3, p.4; decoder/sn74ahct139-q1.pdf p.3 (PW, BQB), p.4, p.5, p.10; decoder/sn74lv139a.pdf p.3 (D/DB/NS/PW, RGY), p.4, p.6, p.8; decoder/sn74lvc139a.pdf p.1–2 (D/DB/DGV/NS/PW, RGY), p.2, p.3, p.4; decoder/sn74lvc139a-q1.pdf p.1, p.3, p.4
- **Families:** HC (SN74HC139, SN74HC139-Q1, CD74HC139), HCT (SN74HCT139, CD74HCT139, CD54HCT139), HCS (SN74HCS139, SN74HCS139-Q1), LS (SN74LS139A, SN54LS139A), S (SN74S139A, SN54S139), ALS (SN74ALS139), AC (CD74AC139, SN74AC139-Q1), ACT (CD74ACT139, SN74ACT139), AHC (SN74AHC139, SN74AHC139-Q1), AHCT (SN74AHCT139, SN74AHCT139-Q1, CAHCT139…Q1), LV (SN74LV139A), LVC (SN74LVC139A, SN74LVC139A-Q1, CLVC139A…Q1)
- **Pin names:** the classic SN names 1G, 1A, 1B, 1Y0–1Y3, 2G, 2A, 2B, 2Y0–2Y3 (sn74hc139, sn74hct139, sn74ls139a, sn74als139, sn74lvc139a, sn74lvc139a-q1, sn74hc139-q1). Other sheets print the same pins differently: CD74HC/HCT/AC/ACT 1E, 1A0, 1A1 / 2E, 2A0, 2A1 (enable E at pins 1/15, A0 at 2/14, A1 at 3/13); the newer SN sheets (AHC, AHCT, AC-Q1, ACT, HCS, LV) keep G but name the selects 1A0/1A1 and 2A0/2A1 (pin 14 = 2A0, pin 13 = 2A1). Confirmed: A = A0 = LSB, B = A1, same pin numbers on every sheet.
- **Pinouts:**

**p16** — N, D, PW, DB, NS, DGV, J, RGY, BQB (pin n = DIP pin n on every sheet: sn74hc139 p.3 (J/W/D/DB/N/NS/PW), sn74ahc139 p.3 and sn74lv139a p.3 and sn74lvc139a p.2 (RGY: pins 1–16 in DIP order, thermal pad not a pin), sn74ac139-q1/sn74ahc139-q1/sn74ahct139-q1 p.3 (BQB: same, pad "can be connected to GND or left floating"))

| Pin | Name | Role |
|---|---|---|
| 1 | 1G | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Y0 | output |
| 5 | 1Y1 | output |
| 6 | 1Y2 | output |
| 7 | 1Y3 | output |
| 8 | GND | ground |
| 9 | 2Y3 | output |
| 10 | 2Y2 | output |
| 11 | 2Y1 | output |
| 12 | 2Y0 | output |
| 13 | 2B | input |
| 14 | 2A | input |
| 15 | 2G | input |
| 16 | VCC | power |

- **Function:** every family prints the same table (sn74hc139 p.7, sn74ls139a p.1, cd74hc139 p.2 with E/A1/A0 names):

  | G | B | A | Y0 | Y1 | Y2 | Y3 |
  |---|---|---|---|---|---|---|
  | H | X | X | H | H | H | H |
  | L | L | L | L | H | H | H |
  | L | L | H | H | L | H | H |
  | L | H | L | H | H | L | H |
  | L | H | H | H | H | H | L |

  Model `binary`, two units: unit 1 `address` [1A, 1B] (A = LSB), `outputs` 1Y0..1Y3, `enables` [{1G, low}]; unit 2 the same with 2A, 2B, 2Y0..2Y3, 2G. `outputsActive` "low"; `disabled`/`unselected` default ("inactive": all outputs high when G is high).

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- HC: rise 14 ns, fall 14 ns; vcc 2–6 V. A/B to Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc139 p.4, p.5)
- HCT: rise 14 ns, fall 14 ns; vcc 4.5–5.5 V. A/B to Y typ, 4.5 V 50 pF; VCC 4.5-5.5 V (sn74hct139 p.4, p.5)
- HCS: rise 7 ns, fall 7 ns; vcc 2–6 V. A/B to Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hcs139 p.4, p.5)
- LS: rise 18 ns, fall 25 ns; vcc 4.75–5.25 V. A/B to Y (3 gate levels) typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls139a p.3)
- S: rise 7 ns, fall 8 ns; vcc 4.75–5.25 V. A/B to Y (3 gate levels) typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls139a p.4)
- ALS: rise 14 ns, fall 14 ns; vcc 4.5–5.5 V. A/B to Y max, 4.5-5.5 V 50 pF; VCC 4.5-5.5 V (sn74als139 p.3)
- AC: rise 6.1 ns, fall 6.4 ns; vcc 1.5–5.5 V. A to Y typ, 5 V 50 pF (sn74ac139-q1 p.7); VCC 1.5-5.5 V (cd74ac139 p.3; sn74ac139-q1 p.5 allows 1.5-6 V)
- ACT: rise 6.3 ns, fall 6.9 ns; vcc 4.5–5.5 V. A to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74act139 p.4, p.5)
- AHC: rise 6.5 ns, fall 6.5 ns; vcc 2–5.5 V, inputMax 5.5 V. A/B to Y typ, 5 V 50 pF; VCC 2-5.5 V, inputs to 5.5 V (sn74ahc139 p.4, p.6)
- AHCT: rise 6.5 ns, fall 6.5 ns; vcc 4.5–5.5 V, inputMax 5.5 V. A/B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V, inputs to 5.5 V (sn74ahct139 p.3, p.4)
- LV: rise 7.3 ns, fall 7.3 ns; vcc 2–5.5 V, inputMax 5.5 V. A/B to Y typ, 3.3 V 50 pF; VCC 2-5.5 V, inputs to 5.5 V (sn74lv139a p.4, p.6)
- LVC: rise 6.2 ns, fall 6.2 ns; vcc 1.65–3.6 V, inputMax 5.5 V. A/B to Y max, 3.3 V 50 pF; VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc139a p.3, p.4)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls139a p.3); S SN74 4.75–5.25 V, SN54S 4.5–5.5 V (p.4) — `sn54vcc` LS and S [4.5, 5.5]. HC 2–6 V (SN54 same); HCT 4.5–5.5 V (SN54/CD54 same); HCS 2–6 V; ALS 4.5–5.5 V; ACT 4.5–5.5 V; AHC 2–5.5 V, AHCT 4.5–5.5 V, LV 2–5.5 V, all three with VI 0–5.5 V at any VCC (inputMax 5.5); LVC 1.65–3.6 V operating, VI 0–5.5 V (inputMax 5.5). AC: CD74AC139 ROC is 1.5–5.5 V (cd74ac139 p.3) while SN74AC139-Q1 allows 1.5–6 V (sn74ac139-q1 p.5); one entry per family, so the entry uses the common 1.5–5.5 V.
- **Checks against the model:** exact. Timing basis notes: LS and S print "2 levels" and "3 levels" of delay for select-to-output; the 3-level (slower) typical is used. ALS and LVC print only max; AC typical comes from the -Q1 sheet (CD74AC139 prints only max, 9.5 ns SN74 at 5 V). No FK/W parts are in the TSV, so no fk20 map is given (sn74hc139 p.3 and sn74ls139a p.1 show it).

## 74_155 — Dual 2-line to 4-line decoders/demultiplexers

- **PDFs:** decoder/sn74ls155a.pdf p.1 (pinout), p.3 (logic diagram, function tables), p.6 (LS155A ROC + timing) — scanned, read from the page images
- **Families:** LS (SN74LS155A, SNJ54LS155A)
- **Pin names:** as printed: 1C, 1G, 2C, 2G, A, B, 1Y0–1Y3, 2Y0–2Y3 (overbars on 1G, 2G, 2C dropped).
- **Pinouts:**

**p16** — N, D, NS, J (sn74ls155a p.1: J/W, N, D packages share the map)

| Pin | Name | Role |
|---|---|---|
| 1 | 1C | input |
| 2 | 1G | input |
| 3 | B | input |
| 4 | 1Y3 | output |
| 5 | 1Y2 | output |
| 6 | 1Y1 | output |
| 7 | 1Y0 | output |
| 8 | GND | ground |
| 9 | 2Y0 | output |
| 10 | 2Y1 | output |
| 11 | 2Y2 | output |
| 12 | 2Y3 | output |
| 13 | A | input |
| 14 | 2G | input |
| 15 | 2C | input |
| 16 | VCC | power |

- **Function:** sn74ls155a p.3:

  | B | A | 1G | 1C | 1Y0 | 1Y1 | 1Y2 | 1Y3 |
  |---|---|---|---|---|---|---|---|
  | X | X | H | X | H | H | H | H |
  | L | L | L | H | L | H | H | H |
  | L | H | L | H | H | L | H | H |
  | H | L | L | H | H | H | L | H |
  | H | H | L | H | H | H | H | L |
  | X | X | X | L | H | H | H | H |

  Section 2 is the same with 2G and 2C, except 2C is active low (row "L L L L → 2Y0 low", "X X X H → all high"). The logic diagram confirms: unit 1 = 1G low AND 1C high, unit 2 = 2G low AND 2C low; A and B are shared.

  Model `binary`: unit 1 `address` [A, B], outputs 1Y0..1Y3, `enables` [{1G, low}, {1C, high}]; unit 2 `address` [A, B], outputs 2Y0..2Y3, `enables` [{2G, low}, {2C, low}]; `outputsActive` "low"; totem-pole (push-pull).

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LS: rise 17 ns, fall 19 ns; vcc 4.75–5.25 V. A/B to Y (3 gate levels) typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls155a p.6)

- **Supply:** SN74LS155A 4.75–5.25 V, SN54LS155A 4.5–5.5 V (sn74ls155a p.6) — `sn54vcc` LS [4.5, 5.5].
- **Checks against the model:** exact. The 3-to-8 mode (1C tied to 2C, 1G tied to 2G) follows from the two units. Timing: 3-level A/B path typ (2-level 10/19 ns is the strobe/2C path).

## 74_156 — Dual 2-line to 4-line decoders/demultiplexers with open-collector outputs

- **PDFs:** decoder/sn74ls155a.pdf p.1 (pinout), p.3 (function tables), p.7 (LS156 ROC + timing) — scanned; decoder/sn74als156.pdf p.1 (pinout), p.2 (function tables), p.4 (ROC), p.5 (timing)
- **Families:** LS (SN74LS156, SNJ54LS156), ALS (SN74ALS156)
- **Pin names:** as printed on both sheets: 1C, 1G, 2C, 2G, A, B, 1Y0–1Y3, 2Y0–2Y3; same pin numbers as the 74_155.
- **Pinouts:**

**p16** — N, D, NS, J (sn74ls155a p.1 (J/W, N, D), sn74als156 p.1 (D, N): identical maps)

| Pin | Name | Role |
|---|---|---|
| 1 | 1C | input |
| 2 | 1G | input |
| 3 | B | input |
| 4 | 1Y3 | output |
| 5 | 1Y2 | output |
| 6 | 1Y1 | output |
| 7 | 1Y0 | output |
| 8 | GND | ground |
| 9 | 2Y0 | output |
| 10 | 2Y1 | output |
| 11 | 2Y2 | output |
| 12 | 2Y3 | output |
| 13 | A | input |
| 14 | 2G | input |
| 15 | 2C | input |
| 16 | VCC | power |

- **Function:** identical to the 74_155 (sn74ls155a p.3; sn74als156 p.2 prints the same three tables). Outputs are open-collector (sn74ls155a p.1 "Open-Collector ('156, 'LS156)", schematic p.3; sn74als156 title "with open-collector outputs", VOH 5.5 V max).

  Model `binary`: same units as 74_155, `outputsActive` "low", `output` "open-drain" (selected output pulls low, others released).

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LS: rise 31 ns, fall 34 ns; vcc 4.75–5.25 V. A/B to Y (3 gate levels) typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls155a p.7)
- ALS: rise 55 ns, fall 25 ns; vcc 4.5–5.5 V. A/B to Y max, 4.5-5.5 V 50 pF; VCC 4.5-5.5 V (sn74als156 p.4, p.5)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls155a p.7) — `sn54vcc` LS [4.5, 5.5]; ALS 4.5–5.5 V (sn74als156 p.4). Outputs may be pulled up to 5.5 V (VOH max), not modeled.
- **Checks against the model:** exact (open-collector = open-drain). ALS prints only max: A/B to Y tPLH 55 ns, tPHL 25 ns (the large tPLH is the open-collector pull-up time with RL = 500 Ω).

## 74_11139 — Dual 2-line to 4-line decoder/demultiplexer

- **PDFs:** decoder/74act11139.pdf p.1 (pinout, function table), p.2 (logic symbol), p.3 (ROC), p.4 (timing)
- **Families:** ACT (74ACT11139)
- **Pin names:** as printed: 1G, 1A, 1B, 1Y0–1Y3, 2G, 2A, 2B, 2Y0–2Y3 (same names as the 74_139; center-pin VCC/GND).
- **Pinouts:**

**p16** — D, PW (74act11139 p.1: "D, N, OR PW PACKAGE" share the map)

| Pin | Name | Role |
|---|---|---|
| 1 | 1Y1 | output |
| 2 | 1Y2 | output |
| 3 | 1Y3 | output |
| 4 | GND | ground |
| 5 | 2Y0 | output |
| 6 | 2Y1 | output |
| 7 | 2Y2 | output |
| 8 | 2Y3 | output |
| 9 | 2B | input |
| 10 | 2A | input |
| 11 | 2G | input |
| 12 | VCC | power |
| 13 | 1G | input |
| 14 | 1B | input |
| 15 | 1A | input |
| 16 | 1Y0 | output |

- **Function:** 74act11139 p.1:

  | G | B | A | Y0 | Y1 | Y2 | Y3 |
  |---|---|---|---|---|---|---|
  | H | X | X | H | H | H | H |
  | L | L | L | L | H | H | H |
  | L | L | H | H | L | H | H |
  | L | H | L | H | H | L | H |
  | L | H | H | H | H | H | L |

  Model `binary`, same units as the 74_139 (address [1A, 1B] / [2A, 2B], enables [{1G, low}] / [{2G, low}]), `outputsActive` "low".

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- ACT: rise 5.7 ns, fall 5.5 ns; vcc 4.5–5.5 V. A/B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (74act11139 p.3, p.4)

- **Supply:** ACT 4.5–5.5 V (74act11139 p.3).
- **Checks against the model:** exact. (The p.1 feature list says "Fully Synchronous Operation for Counting", a copy-paste leftover; the device is combinational per the function table and logic diagram.)

## 74_1G139 — 2-to-4 line decoder

- **PDFs:** decoder/sn74lvc1g139.pdf p.3 (pinouts DCT, DCU, YZP, pin functions), p.5 (ROC), p.6 (timing), p.8 (load tables), p.10 (function table)
- **Families:** LVC (SN74LVC1G139)
- **Pin names:** as printed: A, B, Y0–Y3. No enable input.
- **Pinouts:**

**p8** — DCT, DCU (sn74lvc1g139 p.3: DCT and DCU have the same pin numbers)

| Pin | Name | Role |
|---|---|---|
| 1 | A | input |
| 2 | B | input |
| 3 | Y3 | output |
| 4 | GND | ground |
| 5 | Y2 | output |
| 6 | Y1 | output |
| 7 | Y0 | output |
| 8 | VCC | power |

**dsbga** — YZP (sn74lvc1g139 p.3 Pin Functions: YZP balls A1 A, B1 B, C1 Y3, D1 GND, D2 Y2, C2 Y1, B2 Y0, A2 VCC)

| Ball | Name | Role |
|---|---|---|
| A1 | A | input |
| B1 | B | input |
| C1 | Y3 | output |
| D1 | GND | ground |
| D2 | Y2 | output |
| C2 | Y1 | output |
| B2 | Y0 | output |
| A2 | VCC | power |

- **Function:** sn74lvc1g139 p.10:

  | B | A | Y0 | Y1 | Y2 | Y3 |
  |---|---|---|---|---|---|
  | L | L | L | H | H | H |
  | L | H | H | L | H | H |
  | H | L | H | H | L | H |
  | H | H | H | H | H | L |

  Model `binary`, one unit: `address` [A, B], `outputs` [Y0, Y1, Y2, Y3], no `enables`; `outputsActive` "low".

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LVC: rise 5.9 ns, fall 5.9 ns; vcc 1.65–5.5 V, inputMax 5.5 V. A/B to Y max, 3.3 V 30 pF; VCC 1.65-5.5 V, inputs to 5.5 V (sn74lvc1g139 p.5, p.6)

- **Supply:** LVC 1.65–5.5 V operating, VI 0–5.5 V at any VCC (inputMax 5.5) (sn74lvc1g139 p.5).
- **Checks against the model:** exact. Timing is max at 3.3 V with the 30 pF load (Table 3); the sheet has no 50 pF row.

## 74_1G18 — 1-of-2 noninverting demultiplexer with 3-state deselected output

- **PDFs:** decoder/sn74lvc1g18.pdf p.3 (pinouts DBV/DCK/DRY/DSF/YZP, pin functions), p.6 (ROC), p.7 (timing), p.12 (function table)
- **Families:** LVC (SN74LVC1G18)
- **Pin names:** as printed: S (select), A (data), Y0, Y1.
- **Pinouts:**

**p6** — DBV, DCK, DRY, DSF (sn74lvc1g18 p.3: DRY/DSF (transparent top view) and DBV/DCK drawings and the Pin Functions table give one map)

| Pin | Name | Role |
|---|---|---|
| 1 | S | input |
| 2 | GND | ground |
| 3 | A | input |
| 4 | Y1 | output |
| 5 | VCC | power |
| 6 | Y0 | output |

**dsbga** — YZP (sn74lvc1g18 p.3 Pin Functions: A1 S, B1 GND, C1 A, C2 Y1, B2 VCC, A2 Y0)

| Ball | Name | Role |
|---|---|---|
| A1 | S | input |
| B1 | GND | ground |
| C1 | A | input |
| C2 | Y1 | output |
| B2 | VCC | power |
| A2 | Y0 | output |

- **Function:** sn74lvc1g18 p.12:

  | S | A | Y0 | Y1 |
  |---|---|---|---|
  | L | L | L | Z |
  | L | H | H | Z |
  | H | L | Z | L |
  | H | H | Z | H |

  The selected output follows A (driven low when A is low, not Z); the deselected output is Z.

  Model `binary`, one unit: `address` [S], `outputs` [Y0, Y1], `enables` [{A, high}]; `outputsActive` "high", `unselected` "z", `disabled` default "inactive" (so with A low the selected output is driven low, the other stays Z). An X on A makes the selected output X; an X on S makes both X.

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LVC: rise 4.2 ns, fall 4.2 ns; vcc 1.65–5.5 V, inputMax 5.5 V. A to Y max, 3.3 V 30/50 pF; VCC 1.65-5.5 V, inputs to 5.5 V (sn74lvc1g18 p.6, p.7)

- **Supply:** LVC 1.65–5.5 V operating, VI 0–5.5 V (inputMax 5.5) (sn74lvc1g18 p.6).
- **Checks against the model:** exact. Timing: A to Y tpd max at 3.3 V, CL 30/50 pF, −40 to 85 °C (p.7); S to Y is ten/tdis (4.6/4.9 ns), not modeled separately.

## 74_1G19 — 1-of-2 decoder/demultiplexer

- **PDFs:** decoder/sn74lvc1g19.pdf p.3 (pinouts DBV/DCK/DRL/DRY/YZP, pin functions), p.5 (ROC), p.6 (timing), p.10 (function table); decoder/sn74auc1g19.pdf p.3 (pinouts DBV/DCK/DRL/YZP, pin functions with balls), p.4 (ROC), p.5 (timing), p.7 (function table)
- **Families:** LVC (SN74LVC1G19), AUC (SN74AUC1G19)
- **Pin names:** as printed on both: A, E, Y0, Y1.
- **Pinouts:**

**p6** — DBV, DCK, DRL, DRY (sn74lvc1g19 p.3 and sn74auc1g19 p.3: every 6-pin package shares one map)

| Pin | Name | Role |
|---|---|---|
| 1 | A | input |
| 2 | GND | ground |
| 3 | E | input |
| 4 | Y1 | output |
| 5 | VCC | power |
| 6 | Y0 | output |

**dsbga** — YZP (sn74auc1g19 p.3 Pin Functions: A1 A, B1 GND, C1 E, C2 Y1, B2 VCC, A2 Y0; sn74lvc1g19 p.3 draws the same ball layout (bottom view, pin numbers only))

| Ball | Name | Role |
|---|---|---|
| A1 | A | input |
| B1 | GND | ground |
| C1 | E | input |
| C2 | Y1 | output |
| B2 | VCC | power |
| A2 | Y0 | output |

- **Function:** sn74lvc1g19 p.10 (sn74auc1g19 p.7 identical):

  | E | A | Y0 | Y1 |
  |---|---|---|---|
  | L | L | L | H |
  | L | H | H | L |
  | H | X | H | H |

  Model `binary`, one unit: `address` [A], `outputs` [Y0, Y1], `enables` [{E, low}]; `outputsActive` "low".

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LVC: rise 5.2 ns, fall 5.2 ns; vcc 1.65–5.5 V, inputMax 5.5 V. A/E to Y max, 3.3 V 30/50 pF; VCC 1.65-5.5 V, inputs to 5.5 V (sn74lvc1g19 p.5, p.6)
- AUC: rise 2.8 ns, fall 2.8 ns; vcc 0.8–2.7 V, inputMax 3.6 V. A/E to Y max, 1.8 V 30 pF; VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc1g19 p.4, p.5)

- **Supply:** LVC 1.65–5.5 V, VI 0–5.5 V (inputMax 5.5) (sn74lvc1g19 p.5); AUC 0.8–2.7 V, VIH up to 3.6 V (inputMax 3.6) (sn74auc1g19 p.4).
- **Checks against the model:** exact. sn74auc1g19 p.3 calls Y0 "true output" and Y1 "complemented output", but its function table is the decoder table above. LVC timing max at 3.3 V 30/50 pF; AUC max at 1.8 V 30 pF.

## 74_1G29 — 2-of-3 decoder/demultiplexer

- **PDFs:** decoder/sn74lvc1g29.pdf p.1 (pinouts DCT, DCU, YZP), p.2 (function table, logic diagram), p.3 (ROC), p.4 (timing)
- **Families:** LVC (SN74LVC1G29)
- **Pin names:** as printed: G, A0, A1, Y0, Y1, Y2.
- **Pinouts:**

**p8** — DCT, DCU (sn74lvc1g29 p.1: DCT and DCU share the map)

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | Y1 | output |
| 3 | A0 | input |
| 4 | GND | ground |
| 5 | Y2 | output |
| 6 | A1 | input |
| 7 | Y0 | output |
| 8 | VCC | power |

**dsbga** — YZP (sn74lvc1g29 p.1 draws the YZP bottom view with pin numbers only (1 G, 2 Y1, 3 A0, 4 GND | 5 Y2, 6 A1, 7 Y0, 8 VCC); ball letters inferred from the TI 8-ball YZP convention that sn74lvc1g139 p.3 prints (pin 1 = A1 … 4 = D1, 5 = D2 … 8 = A2))

| Ball | Name | Role |
|---|---|---|
| A1 | G | input |
| B1 | Y1 | output |
| C1 | A0 | input |
| D1 | GND | ground |
| D2 | Y2 | output |
| C2 | A1 | input |
| B2 | Y0 | output |
| A2 | VCC | power |

- **Function:** sn74lvc1g29 p.2:

  | G | A1 | A0 | Y0 | Y1 | Y2 |
  |---|---|---|---|---|---|
  | L | L | X | L | H | H |
  | L | H | L | H | L | H |
  | L | H | H | H | H | L |
  | H | X | X | H | H | H |

  **This differs from the plan's "code 3 selects none":** with A1 low, Y0 is selected whatever A0 is (codes 0 and 1 → Y0); A1 high, A0 low → Y1; A1 high, A0 high → Y2. No code selects nothing while G is low.

  Model `binary`, two units sharing G: unit 1 `address` [A1], `outputs` [Y0], `enables` [{G, low}] (code A1=0 → Y0, code 1 → none); unit 2 `address` [A0], `outputs` [Y1, Y2], `enables` [{G, low}, {A1, high}]. `outputsActive` "low". An X on A1 makes all three outputs X (unit 1 address X, unit 2 enable X); an X on A0 with A1 low leaves Y1, Y2 high.

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- LVC: rise 6.1 ns, fall 6.1 ns; vcc 1.65–5.5 V, inputMax 5.5 V. A/G to Y max, 3.3 V 30/50 pF; VCC 1.65-5.5 V, inputs to 5.5 V (sn74lvc1g29 p.3, p.4)

- **Supply:** LVC 1.65–5.5 V, VI 0–5.5 V (inputMax 5.5) (sn74lvc1g29 p.3).
- **Checks against the model:** exact with the two-unit form above (a single unit with outputs selected by code i cannot express Y0 for codes 0 and 1). The YZP ball letters are inferred, not printed on this sheet.

## 4555B — Dual binary to 1 of 4 decoder/demultiplexer, outputs high on select

- **PDFs:** decoder/cd4555b.pdf p.1 (terminal assignments, ROC), p.2 (dynamic characteristics), p.3 (logic diagrams, truth table) — scanned, read from the page images
- **Families:** C (CD4555BE, CD4555BF3A, CD4555BM96, CD4555BNSR, CD4555BPWR)
- **Pin names:** the sheet prints E, A, B, Q0–Q3 (overbars on E, and on the 4556B outputs) for each half without unit prefixes; the device uses 1E, 1A, 1B, 1Q0–1Q3 (pins 1–7) and 2E, 2A, 2B, 2Q0–2Q3 (pins 15–9), VSS 8, VDD 16 — the unit-prefix style of 4013B. The logic diagrams on p.3 pair pins 2(14) A, 3(13) B, 1(15) E, 4(12) Q0, 5(11) Q1, 6(10) Q2, 7(9) Q3.
- **Pinouts:**

**p16** — N, J, D, NS, PW (cd4555b p.1 terminal assignments (one drawing per type; 4555B and 4556B have the same pin numbers))

| Pin | Name | Role |
|---|---|---|
| 1 | 1E | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Q0 | output |
| 5 | 1Q1 | output |
| 6 | 1Q2 | output |
| 7 | 1Q3 | output |
| 8 | VSS | ground |
| 9 | 2Q3 | output |
| 10 | 2Q2 | output |
| 11 | 2Q1 | output |
| 12 | 2Q0 | output |
| 13 | 2B | input |
| 14 | 2A | input |
| 15 | 2E | input |
| 16 | VDD | power |

- **Function:** cd4555b p.3 truth table:

  | E | B | A | 4555B Q3 Q2 Q1 Q0 | 4556B Q3 Q2 Q1 Q0 |
  |---|---|---|---|---|
  | 0 | 0 | 0 | 0 0 0 1 | 1 1 1 0 |
  | 0 | 0 | 1 | 0 0 1 0 | 1 1 0 1 |
  | 0 | 1 | 0 | 0 1 0 0 | 1 0 1 1 |
  | 0 | 1 | 1 | 1 0 0 0 | 0 1 1 1 |
  | 1 | X | X | 0 0 0 0 | 1 1 1 1 |

  Model `binary`, two units: `address` [1A, 1B] / [2A, 2B] (A = LSB), outputs 1Q0..1Q3 / 2Q0..2Q3, `enables` [{1E, low}] / [{2E, low}]; `outputsActive` "high" (E high → all outputs low, the inactive level).

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- C: rise 220 ns, fall 220 ns; vcc 3–18 V. A/B to Q typ, VDD 5 V 50 pF; VDD 3-18 V (cd4555b p.1, p.2)

- **Supply:** VDD 3–18 V (cd4555b p.1 recommended operating conditions); CD4555BF3A is the same range (no `sn54vcc`).
- **Checks against the model:** exact. Timing: A or B to any output tPHL/tPLH typ 220 ns at VDD 5 V, CL 50 pF (cd4555b p.2); E to output is 200 ns typ.

## 4556B — Dual binary to 1 of 4 decoder/demultiplexer, outputs low on select

- **PDFs:** decoder/cd4555b.pdf p.1 (terminal assignments, ROC), p.2 (dynamic characteristics), p.3 (logic diagrams, truth table) — scanned, read from the page images
- **Families:** C (CD4556BE, CD4556BM96)
- **Pin names:** the sheet prints E, A, B, Q0–Q3 (overbars on E, and on the 4556B outputs) for each half without unit prefixes; the device uses 1E, 1A, 1B, 1Q0–1Q3 (pins 1–7) and 2E, 2A, 2B, 2Q0–2Q3 (pins 15–9), VSS 8, VDD 16 — the unit-prefix style of 4013B. The logic diagrams on p.3 pair pins 2(14) A, 3(13) B, 1(15) E, 4(12) Q0, 5(11) Q1, 6(10) Q2, 7(9) Q3.
- **Pinouts:**

**p16** — N, D (cd4555b p.1 terminal assignments (one drawing per type; 4555B and 4556B have the same pin numbers))

| Pin | Name | Role |
|---|---|---|
| 1 | 1E | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Q0 | output |
| 5 | 1Q1 | output |
| 6 | 1Q2 | output |
| 7 | 1Q3 | output |
| 8 | VSS | ground |
| 9 | 2Q3 | output |
| 10 | 2Q2 | output |
| 11 | 2Q1 | output |
| 12 | 2Q0 | output |
| 13 | 2B | input |
| 14 | 2A | input |
| 15 | 2E | input |
| 16 | VDD | power |

- **Function:** cd4555b p.3 truth table:

  | E | B | A | 4555B Q3 Q2 Q1 Q0 | 4556B Q3 Q2 Q1 Q0 |
  |---|---|---|---|---|
  | 0 | 0 | 0 | 0 0 0 1 | 1 1 1 0 |
  | 0 | 0 | 1 | 0 0 1 0 | 1 1 0 1 |
  | 0 | 1 | 0 | 0 1 0 0 | 1 0 1 1 |
  | 0 | 1 | 1 | 1 0 0 0 | 0 1 1 1 |
  | 1 | X | X | 0 0 0 0 | 1 1 1 1 |

  Model `binary`, two units: `address` [1A, 1B] / [2A, 2B] (A = LSB), outputs 1Q0..1Q3 / 2Q0..2Q3, `enables` [{1E, low}] / [{2E, low}]; `outputsActive` "low" (E high → all outputs high, the inactive level).

- **Timing:** (main path address to output; rise = tPLH, fall = tPHL)

- C: rise 220 ns, fall 220 ns; vcc 3–18 V. A/B to Q typ, VDD 5 V 50 pF; VDD 3-18 V (cd4555b p.1, p.2)

- **Supply:** VDD 3–18 V (cd4555b p.1 recommended operating conditions); CD4555BF3A is the same range (no `sn54vcc`).
- **Checks against the model:** exact. Timing: A or B to any output tPHL/tPLH typ 220 ns at VDD 5 V, CL 50 pF (cd4555b p.2); E to output is 200 ns typ.

## 74_42 — 4-line BCD to 10-line decimal decoder

- **PDFs:** decoder/sn74hc42.pdf p.1 (pinout, ordering), p.2 (function table), p.4 (recommended operating conditions), p.5 (switching); decoder/cd74hc42.pdf p.1 (pinout, ordering), p.2 (truth table), p.3 (operating conditions), p.4 (HC switching), p.5 (HCT switching); decoder/sn74ls42.pdf p.1 (pinout), p.4 (function table), p.6 (SN54LS42/SN74LS42 recommended operating conditions, switching)
- **Families:** HC (SN74HC42, CD74HC42), HCT (CD74HCT42), LS (SN74LS42)
- **Pin names:** inputs `A B C D` (SN74HC42, SN74LS42; A = LSB, weight 1); outputs `Y0`…`Y9` (CD74HC42). The SN sheets name the outputs bare `0`…`9`; the CD74 sheet names the inputs `A0 A1 A2 A3` (A0 = A … A3 = D, same pins 15…12). The SN input names and the CD output names are used for every part (a bare digit is a poor pin name).
- **Pinouts:** `p16` for N, D (every part in the TSV: SN74HC42N/DR, CD74HC42E/M, CD74HCT42E, SN74LS42N/D); the D, N, NS, J, W packages share one map (sn74hc42 p.1, sn74ls42 p.1, cd74hc42 p.1).

  | Pin | Name | Role |
  |---|---|---|
  | 1 | Y0 | output |
  | 2 | Y1 | output |
  | 3 | Y2 | output |
  | 4 | Y3 | output |
  | 5 | Y4 | output |
  | 6 | Y5 | output |
  | 7 | Y6 | output |
  | 8 | GND | ground |
  | 9 | Y7 | output |
  | 10 | Y8 | output |
  | 11 | Y9 | output |
  | 12 | D | input |
  | 13 | C | input |
  | 14 | B | input |
  | 15 | A | input |
  | 16 | VCC | power |

  (The SN54 FK LCCC map exists on sn74hc42 p.1 / sn74ls42 p.1 but no FK part is in the TSV, so no `fk20` pinout.)
- **Function** (sn74hc42 p.2; identical in sn74ls42 p.4 and cd74hc42 p.2): no enable.

  | No. | D C B A | Y0 Y1 Y2 Y3 Y4 Y5 Y6 Y7 Y8 Y9 |
  |---|---|---|
  | 0 | L L L L | L H H H H H H H H H |
  | 1 | L L L H | H L H H H H H H H H |
  | 2 | L L H L | H H L H H H H H H H |
  | 3 | L L H H | H H H L H H H H H H |
  | 4 | L H L L | H H H H L H H H H H |
  | 5 | L H L H | H H H H H L H H H H |
  | 6 | L H H L | H H H H H H L H H H |
  | 7 | L H H H | H H H H H H H L H H |
  | 8 | H L L L | H H H H H H H H L H |
  | 9 | H L L H | H H H H H H H H H L |
  | 10–15 (invalid) | H L H L … H H H H | all H |

  Codes 10–15 select nothing, all outputs high: confirmed on all three sheets ("All Outputs Are High for Invalid BCD Conditions", sn74hc42 p.1; "Non-valid BCD inputs result in none of the outputs being selected (all outputs are high)", cd74hc42 p.1).
  Model: `binary`, one unit, `address` [A, B, C, D], `outputs` [Y0…Y9] (10 outputs for 4 address bits: codes ≥ 10 select nothing), no `enables`, `outputsActive` low, push-pull.
- **Timing:**
  - HC: 18 ns typ tpd A/B/C/D to 0–9, 4.5 V, CL 50 pF (sn74hc42 p.5; max 30). rise = fall = 18. (CD74HC42 prints only max 30 at 4.5 V 50 pF and 12 typ at 5 V 15 pF, cd74hc42 p.4; the SN typ is used.)
  - HCT: 35 ns max tPLH/tPHL input to Y, 4.5 V, 50 pF (cd74hc42 p.5; only typ printed is 14 at 5 V 15 pF). rise = fall = 35.
  - LS: 20 ns typ tPLH and tPHL through 3 levels of logic (the slower of the two printed paths; 2 levels: 15/15), 5 V, CL 15 pF, RL 2 kΩ (sn74ls42 p.6). rise = fall = 20.
- **Supply:** HC 2–6 V (sn74hc42 p.4; SN54HC42 the same); HCT 4.5–5.5 V (cd74hc42 p.3); LS SN74LS42 4.75–5.25 V, SN54LS42 4.5–5.5 V (sn74ls42 p.6; no SN54 part in the TSV). No inputMax (HC/HCT VI 0…VCC; LS input abs max 7 V is not an operating rating).
- **Checks against the model:** exact. Pin 8/16 roles are GND/VCC.

## 74_45 — BCD-to-decimal decoder/driver

- **PDFs:** decoder/sn7445.pdf p.1 (pinout, function table, logic diagram, description), p.2 (absolute maximum ratings, recommended operating conditions, electrical and switching characteristics)
- **Families:** TTL (SN7445; SN5445/SNJ5445 are the military versions, none in the TSV)
- **Pin names:** inputs `A B C D` (A = LSB); outputs `Y0`…`Y9` (the sheet prints bare `0`…`9`; same names as 74_42 and 74_145).
- **Pinouts:** `p16` for N (SN7445N, the only part); J, N and W share the map (sn7445 p.1). Same map as 74_42.

  | Pin | Name | Role |
  |---|---|---|
  | 1–7 | Y0–Y6 | output |
  | 8 | GND | ground |
  | 9 | Y7 | output |
  | 10 | Y8 | output |
  | 11 | Y9 | output |
  | 12 | D | input |
  | 13 | C | input |
  | 14 | B | input |
  | 15 | A | input |
  | 16 | VCC | power |
- **Function** (sn7445 p.1): the same table as 74_42 (codes 0–9 pull the one numbered output low, every other output high/off; the six invalid codes 10–15 leave all outputs H = off). "H = high level (off), L = low level (on)". Outputs are open-collector n-p-n transistors ("high-breakdown output transistors (30 volts) will sink up to 80 milliamperes", p.1; output schematic p.2). "All Outputs Are Off for Invalid BCD Input Conditions" (p.1).
  Model: `binary`, one unit, `address` [A, B, C, D], `outputs` [Y0…Y9], no `enables`, `outputsActive` low, `"output": "open-drain"`.
- **Output rating (not modeled):** off-state output voltage VO(off) max 30 V (recommended, p.2); VO(on) max 0.9 V at IO(on) = 80 mA, 0.4 V at 20 mA; IO(off) max 250 µA at 30 V; maximum current into any output (off-state) 1 mA (abs max, p.2).
- **Timing:** TTL: tPLH 50 ns max, tPHL 50 ns max, VCC 5 V, CL 15 pF, RL 100 Ω (p.2); no typical printed. rise = fall = 50.
- **Supply:** SN7445 4.75–5.25 V; SN5445 4.5–5.5 V (p.2). Input voltage abs max 5.5 V (no inputMax: TTL).
- **Checks against the model:** exact apart from the 30 V / 80 mA output rating, which the model does not use (open-drain releases to Z for H).

## 74_145 — BCD-to-decimal decoder/driver

- **PDFs:** decoder/sn74145.pdf p.1 (pinouts, function table, logic diagram, description), p.2 (SN54145/SN74145 ratings, recommended operating conditions, switching), p.3 (SN54LS145/SN74LS145 ratings, recommended operating conditions, switching), p.4–5 (package option addendum)
- **Families:** TTL (SN74145), LS (SN74LS145)
- **Pin names:** inputs `A B C D` (A = LSB); outputs `Y0`…`Y9` (sheet: bare `0`…`9`); same names as 74_42 / 74_45.
- **Pinouts:** `p16` for N, D, NS (SN74145N, SN74LS145N, SN74LS145DR, SN74LS145NSR). The D, J, N, W packages share the map (sn74145 p.1, "Pin numbers shown are for D, J, N, and W packages"); NS is not drawn but is the SO variant of the same 16-pin map (TI convention, same as the D package). Same map as 74_42. (The SN54LS145 FK map on p.1 is not needed: no FK part in the TSV.)

  | Pin | Name | Role |
  |---|---|---|
  | 1–7 | Y0–Y6 | output |
  | 8 | GND | ground |
  | 9 | Y7 | output |
  | 10 | Y8 | output |
  | 11 | Y9 | output |
  | 12 | D | input |
  | 13 | C | input |
  | 14 | B | input |
  | 15 | A | input |
  | 16 | VCC | power |
- **Function** (p.1): identical to 74_42 / 74_45: code n (0–9) drives output n low (on), the others high (off); codes 10–15 ("INVALID") all H. "H = high level (off), L = low level (on)". Open-collector outputs ("high-breakdown output transistors (15 volts) of the SN54145, SN74145, or SN74LS145 will sink up to 80 milliamperes", p.1).
  Model: `binary`, one unit, `address` [A, B, C, D], `outputs` [Y0…Y9], no `enables`, `outputsActive` low, `"output": "open-drain"`.
- **Output rating (not modeled):** VO(off) max 15 V for both families (p.2, p.3). TTL: VO(on) max 0.9 V at 80 mA, 0.4 V at 20 mA; IO(off) 250 µA at 15 V (p.2). LS: VO(on) max 0.4 V at 12 mA, 0.5 V at 24 mA, 3 V (typ 2.3) at 80 mA (SN74LS145 only); IO(off) 250 µA at 15 V (p.3).
- **Timing:**
  - TTL: tPLH, tPHL 50 ns max, 5 V, CL 15 pF, RL 100 Ω (p.2; no typ printed). rise = fall = 50.
  - LS: tPLH, tPHL 50 ns max, 5 V, CL 45 pF, RL 665 Ω (p.3; no typ printed). rise = fall = 50.
- **Supply:** TTL SN74145 4.75–5.25 V, SN54145 4.5–5.5 V (p.2); LS SN74LS145 4.75–5.25 V, SN54LS145 4.5–5.5 V (p.3). No SN54 part in the TSV.
- **Checks against the model:** exact apart from the output rating. Datasheet defect: the running headers of p.2 and p.3 are swapped — p.2 is headed "SN54LS145, SN74LS145" but its tables are SN54145/SN74145 (4 kΩ input, 1 mA/5.5 V input), p.3 is headed "SN54145, SN74145" but its tables are SN54LS145/SN74LS145. The table column headings were used, not the page headers.

## 74_154 — 4- to 16-line decoder/demultiplexer

- **PDFs:** decoder/cd74hc154.pdf p.1 (pinout, ordering, description), p.2 (truth table), p.3 (operating conditions), p.4 (HC switching), p.5 (HC enable paths, HCT switching), package option addendum (CD74HC154M96, CD74HCT154M96 = SOIC (DW) 24)
- **Families:** HC (CD74HC154), HCT (CD74HCT154)
- **Pin names:** as the CD74 sheet: address `A0 A1 A2 A3` (A0 = LSB), enables `E1 E2` (active low), outputs `Y0`…`Y15`. (The SN74154 sheet, not in this group, calls them A B C D, G1 G2, 0…15; no SN part is in the TSV, so the CD names are used.)
- **Pinouts:** `p24` for DW (both parts); the CERDIP, PDIP and SOIC share the map (p.1).

  | Pin | Name | Role |
  |---|---|---|
  | 1–11 | Y0–Y10 | output |
  | 12 | GND | ground |
  | 13 | Y11 | output |
  | 14 | Y12 | output |
  | 15 | Y13 | output |
  | 16 | Y14 | output |
  | 17 | Y15 | output |
  | 18 | E1 | input |
  | 19 | E2 | input |
  | 20 | A3 | input |
  | 21 | A2 | input |
  | 22 | A1 | input |
  | 23 | A0 | input |
  | 24 | VCC | power |
- **Function** (p.2): E1 = E2 = L: output Yn (n = A3A2A1A0) L, all others H. E1 = L, E2 = H; E1 = H, E2 = L; E1 = E2 = H: all outputs H (address X). "A High on either enable input forces the output into the High state" (p.1).
  Model: `binary`, one unit, `address` [A0, A1, A2, A3], `outputs` [Y0…Y15], `enables` [{E1, low}, {E2, low}], `outputsActive` low; `disabled` default (inactive = H).
- **Timing:**
  - HC: address to output tPLH/tPHL 35 ns max, 4.5 V, 50 pF (p.4; typ only at 5 V 15 pF = 14). rise = fall = 35.
  - HCT: address to output 35 ns max, 4.5 V, 50 pF (p.5; typ 14 at 5 V 15 pF). rise = fall = 35.
  (E1/E2 to output: HC 35 max, HCT 34 max, not modeled separately.)
- **Supply:** HC 2–6 V, HCT 4.5–5.5 V (p.3). No inputMax.
- **Checks against the model:** exact. Only DW parts in the TSV (CD74HC154M/CD74HCT154M are Obsolete in the addendum and not added).

## 4028B — BCD-to-decimal decoder

- **PDFs:** decoder/cd4028b.pdf p.1 (features, logic diagram, terminal diagram, Table I truth table), p.2 (recommended operating conditions, dynamic characteristics), p.3 (Table II code conversion chart), package option addendum (CD4028BE PDIP (N), CD4028BM96 SOIC (D), CD4028BNSR SOP (NS), CD4028BPWR TSSOP (PW), Last Time Buy)
- **Families:** C (CD4028B)
- **Pin names:** inputs `A B C D` (A = LSB, as printed); outputs `Q0`…`Q9` (the sheet prints bare `0`…`9`); supply `VDD` / `VSS`.
- **Pinouts:** `p16` for N, D, NS, PW (the one terminal diagram, p.1; confirmed against the logic diagram's pin numbers, p.1).

  | Pin | Name | Role |
  |---|---|---|
  | 1 | Q4 | output |
  | 2 | Q2 | output |
  | 3 | Q0 | output |
  | 4 | Q7 | output |
  | 5 | Q9 | output |
  | 6 | Q5 | output |
  | 7 | Q6 | output |
  | 8 | VSS | ground |
  | 9 | Q8 | output |
  | 10 | A | input |
  | 11 | D | input |
  | 12 | C | input |
  | 13 | B | input |
  | 14 | Q1 | output |
  | 15 | Q3 | output |
  | 16 | VDD | power |
- **Function** (Table I, p.1; 1 = high, 0 = low; active-high, "decoded outputs go high on selection"):

  | D C B A | 0 1 2 3 4 5 6 7 8 9 |
  |---|---|
  | 0000 | 1 0 0 0 0 0 0 0 0 0 |
  | 0001 | 0 1 0 0 0 0 0 0 0 0 |
  | 0010 | 0 0 1 0 0 0 0 0 0 0 |
  | 0011 | 0 0 0 1 0 0 0 0 0 0 |
  | 0100 | 0 0 0 0 1 0 0 0 0 0 |
  | 0101 | 0 0 0 0 0 1 0 0 0 0 |
  | 0110 | 0 0 0 0 0 0 1 0 0 0 |
  | 0111 | 0 0 0 0 0 0 0 1 0 0 |
  | 1000 | 0 0 0 0 0 0 0 0 1 0 |
  | 1001 | 0 0 0 0 0 0 0 0 0 1 |
  | 1010 | 0 0 0 0 0 0 0 0 0 0 |
  | 1011 | 0 0 0 0 0 0 0 0 0 0 |
  | 1100 | 0 0 0 0 0 0 0 0 0 0 |
  | 1101 | 0 0 0 0 0 0 0 0 0 0 |
  | 1110 | 0 0 0 0 0 0 0 0 0 0 |
  | 1111 | 0 0 0 0 0 0 0 0 0 0 |

  Codes 10–15 select nothing (every output low): fully decoded, no partial decoding of 8/9 (Table I rows 1010–1111 are all 0; Table II, p.3, shows the same for the 4-bit binary column). The binary model (codes ≥ number of outputs select nothing) matches exactly. "Binary-to-octal" use (p.1) is just D held 0.
  Model: `binary`, one unit, `address` [A, B, C, D], `outputs` [Q0…Q9], no `enables`, `outputsActive` high.
- **Timing:** C: tPHL, tPLH 175 ns typ (350 max), VDD 5 V, CL 50 pF, RL 200 kΩ (p.2). rise = fall = 175.
- **Supply:** VDD 3–18 V (recommended operating conditions, p.2). Inputs −0.5 V to VDD + 0.5 V (abs max), so no inputMax.
- **Checks against the model:** exact.

## 4514B — 4-bit latch/4-to-16 line decoder, output high on select

- **PDFs:** decoder/cd4514b.pdf p.1 (description, functional diagram with pin numbers, recommended operating conditions), p.3 (dynamic characteristics, logic diagram), p.4 (decode truth table, terminal assignment), package option addendum (CD4514BM96, CD4514BM96G4 = SOIC (DW) 24)
- **Families:** C (CD4514B)
- **Pin names:** `STROBE`, `DATA1`…`DATA4` (sheet "DATA 1"…"DATA 4"; space dropped; DATA1 = A = LSB, DATA4 = D, functional diagram p.1), `INHIBIT`, outputs `S0`…`S15` (as printed), `VDD`/`VSS`. The CD74HC4514 (device 74_4514) names the same pins LE, A0–A3, E, Y0–Y15, GND/VCC.
- **Pinouts:** `p24` for DW (terminal assignment p.4, identical numbers in the functional diagram p.1 and logic diagram p.3).

  | Pin | Name | Role | | Pin | Name | Role |
  |---|---|---|---|---|---|---|
  | 1 | STROBE | input | | 13 | S13 | output |
  | 2 | DATA1 | input | | 14 | S12 | output |
  | 3 | DATA2 | input | | 15 | S15 | output |
  | 4 | S7 | output | | 16 | S14 | output |
  | 5 | S6 | output | | 17 | S9 | output |
  | 6 | S5 | output | | 18 | S8 | output |
  | 7 | S4 | output | | 19 | S11 | output |
  | 8 | S3 | output | | 20 | S10 | output |
  | 9 | S1 | output | | 21 | DATA3 | input |
  | 10 | S2 | output | | 22 | DATA4 | input |
  | 11 | S0 | output | | 23 | INHIBIT | input |
  | 12 | VSS | ground | | 24 | VDD | power |
- **Function:** decode truth table (Strobe = 1), p.4: INHIBIT = 0, D C B A = n → Sn selected, CD4514B selected output = logic 1 (high), others 0; INHIBIT = 1, data X → all outputs 0 (CD4514B). Latch: "The latches hold the last input data presented prior to the strobe transition from 1 to 0" (p.1): STROBE high = transparent, low = hold. "Inhibit control allows all outputs to be placed at 0 (CD4514B) … regardless of the state of the data or strobe inputs" (p.1); the logic diagram (p.3) gates INHIBIT into the output stage after the latch.
  Model: `binary`, one unit, `address` [DATA1, DATA2, DATA3, DATA4], `outputs` [S0…S15], `enables` [{INHIBIT, low}], `latch` STROBE, `latchActive` low (STROBE low holds), `outputsActive` high.
- **Timing:** C: tPHL, tPLH strobe or data to output 485 ns typ (970 max), VDD 5 V, CL 50 pF, RL 200 kΩ (p.3). rise = fall = 485. (Inhibit 250 typ, not modeled separately; setup 75 typ/150 max, strobe pulse width 125 typ/250 max, not modeled.)
- **Supply:** VDD 3–18 V (recommended operating conditions, p.1). No inputMax.
- **Checks against the model:** exact for the logic. The model's latch holds 0 at power-on (library rule); the sheet gives no power-on state. Setup and strobe-width minimums are not checked by the model.

## 4515B — 4-bit latch/4-to-16 line decoder, output low on select

- **PDFs:** decoder/cd4514b.pdf p.1 (description, functional diagram, recommended operating conditions), p.3 (dynamic characteristics, logic diagram: "these inverters used only on CD4515B"), p.4 (decode truth table, terminal assignment "CD4514B CD4515B"), package option addendum (CD4515BM96 = SOIC (DW) 24)
- **Families:** C (CD4515B)
- **Pin names:** as 4514B: `STROBE`, `DATA1`…`DATA4` (DATA1 = LSB), `INHIBIT`, `S0`…`S15`, `VDD`/`VSS`.
- **Pinouts:** `p24` for DW; the same terminal assignment as 4514B (p.4, one drawing for both), see 4514B for the table.
- **Function:** as 4514B with inverted outputs: INHIBIT = 0, D C B A = n → Sn = logic 0 (low), others 1; INHIBIT = 1 → all outputs 1 (p.4, p.1). STROBE high transparent, low holds (p.1).
  Model: `binary`, one unit, `address` [DATA1…DATA4], `outputs` [S0…S15], `enables` [{INHIBIT, low}], `latch` STROBE, `latchActive` low, `outputsActive` low.
- **Timing:** C: strobe or data to output 485 ns typ, VDD 5 V, CL 50 pF (p.3; one table for both types). rise = fall = 485.
- **Supply:** VDD 3–18 V (p.1).
- **Checks against the model:** exact for the logic (power-on latch state 0 per library rule; the sheet gives none).

## 74_4514 — 4- to 16-line decoder/demultiplexer with input latches, output high on select

- **PDFs:** decoder/cd74hc4514.pdf p.1 (description, pinout, ordering), p.2 (functional diagram, decode truth table), p.3 (operating conditions), p.4 (setup/pulse prerequisites, switching: select and LE to outputs), p.5 (E to outputs), package option addendum (CD74HC4514M96 = SOIC (DW) 24)
- **Families:** HC (CD74HC4514)
- **Pin names:** as printed: `LE`, `A0`…`A3` (A0 = LSB), `E` (enable, active low), `Y0`…`Y15`, `VCC`/`GND`. The pin positions are the same as CD4514B (LE = STROBE pin 1, A0/A1 = DATA1/DATA2 pins 2/3, A2/A3 = DATA3/DATA4 pins 21/22, E = INHIBIT pin 23, Yn = Sn), but every name differs, so this is its own device (plan-10-A §3.4 rule for CMOS 4000 functions in a 74 family).
- **Pinouts:** `p24` for DW (p.1: CERDIP, PDIP, SOIC share the map).

  | Pin | Name | Role | | Pin | Name | Role |
  |---|---|---|---|---|---|---|
  | 1 | LE | input | | 13 | Y13 | output |
  | 2 | A0 | input | | 14 | Y12 | output |
  | 3 | A1 | input | | 15 | Y15 | output |
  | 4 | Y7 | output | | 16 | Y14 | output |
  | 5 | Y6 | output | | 17 | Y9 | output |
  | 6 | Y5 | output | | 18 | Y8 | output |
  | 7 | Y4 | output | | 19 | Y11 | output |
  | 8 | Y3 | output | | 20 | Y10 | output |
  | 9 | Y1 | output | | 21 | A2 | input |
  | 10 | Y2 | output | | 22 | A3 | input |
  | 11 | Y0 | output | | 23 | E | input |
  | 12 | GND | ground | | 24 | VCC | power |
- **Function** (decode truth table LE = 1, p.2): E = 0, A3A2A1A0 = n → Yn addressed, 4514 = logic 1 (high); E = 1, address X → all outputs 0 (4514). "When Latch Enable (LE) is high the output follows changes in the inputs … When LE is low the output is isolated from changes in the input" (p.1): LE high transparent, low holds.
  Model: `binary`, one unit, `address` [A0…A3], `outputs` [Y0…Y15], `enables` [{E, low}], `latch` LE, `latchActive` low, `outputsActive` high.
- **Timing:** HC: select (A) to outputs tPHL/tPLH 55 ns max, 4.5 V, 50 pF (p.4; typ only at 5 V 15 pF = 23). rise = fall = 55. (LE to outputs 45 max, E to outputs 35 max, not modeled separately.)
- **Supply:** HC 2–6 V (p.3).
- **Checks against the model:** logic exact. Datasheet text defects (not affecting the model): the description p.1 says that with LE low the output "remains at the level (high for the 4514, low for the 4515) it had before the latches were enabled" — read literally that is the inactive level, contradicting the truth table and the "isolated from changes in the input" sentence; the latch holds the address as on CD4514B. The truth table header p.2 prints "4515 = LOGIC 0 (HIGH)" (should be LOW). Power-on latch state 0 per library rule (not given).

## 74_4515 — 4- to 16-line decoder/demultiplexer with input latches, output low on select

- **PDFs:** decoder/cd74hc4514.pdf p.1 (description, pinout "CD74HC4514, CD74HC4515", ordering), p.2 (functional diagram, decode truth table), p.3 (operating conditions), p.4 (switching), package option addendum (CD74HC4515M96 = SOIC (DW) 24)
- **Families:** HC (CD74HC4515)
- **Pin names:** as 74_4514: `LE`, `A0`…`A3`, `E`, `Y0`…`Y15`, `VCC`/`GND` (one pinout drawing for both, p.1). Same pin positions as CD4515B, different names.
- **Pinouts:** `p24` for DW; identical to 74_4514 (see that section for the table).
- **Function** (p.2): E = 0, address n → Yn = logic 0 (low), others high; E = 1 → all outputs 1 (4515). LE high transparent, low holds (p.1).
  Model: `binary`, one unit, `address` [A0…A3], `outputs` [Y0…Y15], `enables` [{E, low}], `latch` LE, `latchActive` low, `outputsActive` low.
- **Timing:** HC: select to outputs 55 ns max, 4.5 V, 50 pF (p.4; one table for 4514 and 4515). rise = fall = 55.
- **Supply:** HC 2–6 V (p.3).
- **Checks against the model:** logic exact; same datasheet text defects as 74_4514 (p.1 hold-level sentence, p.2 "LOGIC 0 (HIGH)").

## 74_147 — 10- to 4-line priority encoder

- **PDFs:** encoder/cd74hc147.pdf p.3 (pinout drawing, pin table), p.4 (Recommended Operating Conditions), p.6 (switching), p.8 (truth table)
- **Families:** HC (CD74HC147), HCT (CD74HCT147)
- **Pin names:** the CD74 sheet's I1..I9 (active-low inputs), Y0..Y3 (active-low BCD outputs, Y0 = LSB), NC at pin 15. The LS sheet (sn74ls148.pdf, not cited by any part) names the same pins 1..9 and A B C D.
  The p.3 pin-function table is wrong at pin 9: it lists "I0 (input)", but the drawing on the same page shows Y0 at pin 9, there is no input 0 (zero is implied), and Y0 is otherwise missing. The drawing (and the LS '147 pinout, pin 9 = A) is followed: pin 9 = Y0, output.
- **Pinouts:** `p16` for N, D, PW (p.3, "CD74HC(T)147 N, D, or PW Package").

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | I4 | input | 9 | Y0 | output |
  | 2 | I5 | input | 10 | I9 | input |
  | 3 | I6 | input | 11 | I1 | input |
  | 4 | I7 | input | 12 | I2 | input |
  | 5 | I8 | input | 13 | I3 | input |
  | 6 | Y2 | output | 14 | Y3 | output |
  | 7 | Y1 | output | 15 | NC | nc |
  | 8 | GND | ground | 16 | VCC | power |

- **Function:** truth table p.8 (I9 highest priority):

  | I1 | I2 | I3 | I4 | I5 | I6 | I7 | I8 | I9 | Y3 | Y2 | Y1 | Y0 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|
  | H | H | H | H | H | H | H | H | H | H | H | H | H |
  | X | X | X | X | X | X | X | X | L | L | H | H | L |
  | X | X | X | X | X | X | X | L | H | L | H | H | H |
  | X | X | X | X | X | X | L | H | H | H | L | L | L |
  | X | X | X | X | X | L | H | H | H | H | L | L | H |
  | X | X | X | X | L | H | H | H | H | H | L | H | L |
  | X | X | X | L | H | H | H | H | H | H | L | H | H |
  | X | X | L | H | H | H | H | H | H | H | H | L | L |
  | X | L | H | H | H | H | H | H | H | H | H | L | H |
  | L | H | H | H | H | H | H | H | H | H | H | H | L |

  Model `priority`: `inputs` I1..I9 (index 0..8), `inputsActive` low, `code` Y0..Y3, `codeActive` low, `offset` 1, `idle` 0; no enable, GS or EO.
  Check: input In wins at index n-1 → value n-1+1 = n, outputs = NOT n (I9: 9 = 1001 → Y3..Y0 = L H H L ✓; I7: 0111 → H L L L ✓; I1: 0001 → H H H L ✓); none active → idle 0 → H H H H ✓. offset + inputs = 10 ≤ 16.
- **Timing:** HC: input to output tPLH/tPHL max 32 ns at 4.5 V, CL = 50 pF (p.6). The table's only typical (13 ns at 5 V) is the CL = 15 pF figure of the features list (p.1), so the 50 pF maximum is used (the cd74hc4511 / cd74hc259 HCT precedent). HCT: max 35 ns at 4.5 V 50 pF (p.6). rise = fall (one tPLH/tPHL row).
- **Supply:** HC 2–6 V, HCT 4.5–5.5 V (p.4). No SN54/CD54 parts in the list (CD54HC147 J is not added).
- **Checks against the model:** exact.

## 74_148 — 8-line to 3-line priority encoder

- **PDFs:** encoder/sn74hc148.pdf p.3 (pinouts), p.4 (Recommended Operating Conditions), p.5 (switching), p.9 (function table); encoder/sn74ls148.pdf p.1 (pinouts incl. SN54LS148 FK), p.3 (function table), p.9 (Recommended Operating Conditions), p.10 (switching)
- **Families:** HC (SN74HC148), LS (SN74LS148, SNJ54LS148)
- **Pin names:** both sheets print the data inputs as bare digits 0..7, outputs A0 A1 A2, GS, EO (the LS sheet prints "E0" for EO), EI. Bare digits are not usable pin names, so the inputs are I0..I7 (the CD74HC147 "I" convention); the rest as printed.
- **Pinouts:** `p16` for N, NS, D (sn74hc148 p.3, sn74ls148 p.1, identical); `fk20` for FK (SNJ54LS148FK; sn74ls148 p.1, identical to sn74hc148 p.3 FK).

  p16:

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | I4 | input | 9 | A0 | output |
  | 2 | I5 | input | 10 | I0 | input |
  | 3 | I6 | input | 11 | I1 | input |
  | 4 | I7 | input | 12 | I2 | input |
  | 5 | EI | input | 13 | I3 | input |
  | 6 | A2 | output | 14 | GS | output |
  | 7 | A1 | output | 15 | EO | output |
  | 8 | GND | ground | 16 | VCC | power |

  fk20:

  | Term | Name | Role | Term | Name | Role |
  |---|---|---|---|---|---|
  | 1 | NC | nc | 11 | NC | nc |
  | 2 | I4 | input | 12 | A0 | output |
  | 3 | I5 | input | 13 | I0 | input |
  | 4 | I6 | input | 14 | I1 | input |
  | 5 | I7 | input | 15 | I2 | input |
  | 6 | NC | nc | 16 | NC | nc |
  | 7 | EI | input | 17 | I3 | input |
  | 8 | A2 | output | 18 | GS | output |
  | 9 | A1 | output | 19 | EO | output |
  | 10 | GND | ground | 20 | VCC | power |

- **Function:** function table (sn74hc148 p.9 = sn74ls148 p.3, identical):

  | EI | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | A2 | A1 | A0 | GS | EO |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | H | X | X | X | X | X | X | X | X | H | H | H | H | H |
  | L | H | H | H | H | H | H | H | H | H | H | H | H | L |
  | L | X | X | X | X | X | X | X | L | L | L | L | L | H |
  | L | X | X | X | X | X | X | L | H | L | L | H | L | H |
  | L | X | X | X | X | X | L | H | H | L | H | L | L | H |
  | L | X | X | X | X | L | H | H | H | L | H | H | L | H |
  | L | X | X | X | L | H | H | H | H | H | L | L | L | H |
  | L | X | X | L | H | H | H | H | H | H | L | H | L | H |
  | L | X | L | H | H | H | H | H | H | H | H | L | L | H |
  | L | L | H | H | H | H | H | H | H | H | H | H | L | H |

  Model `priority`: `inputs` I0..I7, `inputsActive` low, `code` A0 A1 A2, `codeActive` low, `offset` 0, `idle` 0 (defaults), `enableIn` EI low, `groupSelect` GS low, `enableOut` EO low, `disabled` inactive (default).
  Check: EI H → code = idle 0 → HHH, GS/EO inactive → H H ✓; EI L, none → idle 0 → HHH, GS H, EO L ✓; In wins → A = NOT n (7 → LLL, 0 → HHH), GS L, EO H ✓.
- **Timing:** main path data 1–7 to A0–A2.
  HC: tpd typ 23 ns at 4.5 V, CL = 50 pF (sn74hc148 p.5); one tpd row → rise = fall = 23.
  LS: in-phase output (A follows the data level) tPLH 14 / tPHL 15 typ at 5 V, CL = 15 pF, RL = 2 kΩ (sn74ls148 p.10). (Out-of-phase: 20 / 16.)
- **Supply:** HC 2–6 V for SN54HC and SN74HC (sn74hc148 p.4). LS: SN74LS 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls148 p.9) → `sn54vcc` LS [4.5, 5.5] for SNJ54LS148FK.
- **Checks against the model:** exact for every table row. The sheet notes (sn74hc148 p.9) a transient low on GS when EI goes high-to-low with all inputs high — a glitch, not modeled.

## 74_348 — 8-line to 3-line priority encoder with 3-state outputs

- **PDFs:** encoder/sn74ls348.pdf (scanned) p.1 (pinouts, function table), p.3 (Recommended Operating Conditions), p.4 (switching)
- **Families:** LS (SN74LS348)
- **Pin names:** as 74_148: the sheet prints inputs 0..7 → I0..I7; EI is printed "E1" (E-subscript-I in the scan) → EI; A0 A1 A2, GS, EO.
- **Pinouts:** `p16` for D, N (p.1, "SN74LS348 ... D or N package"); identical to 74_148 p16:

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | I4 | input | 9 | A0 | output |
  | 2 | I5 | input | 10 | I0 | input |
  | 3 | I6 | input | 11 | I1 | input |
  | 4 | I7 | input | 12 | I2 | input |
  | 5 | EI | input | 13 | I3 | input |
  | 6 | A2 | output | 14 | GS | output |
  | 7 | A1 | output | 15 | EO | output |
  | 8 | GND | ground | 16 | VCC | power |

  (The sheet also draws an SN54LS348 FK map; no FK part is in the list.)
- **Function:** function table p.1:

  | EI | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | A2 | A1 | A0 | GS | EO |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | H | X | X | X | X | X | X | X | X | Z | Z | Z | H | H |
  | L | H | H | H | H | H | H | H | H | Z | Z | Z | H | L |
  | L | X | X | X | X | X | X | X | L | L | L | L | L | H |
  | L | X | X | X | X | X | X | L | H | L | L | H | L | H |
  | L | X | X | X | X | X | L | H | H | L | H | L | L | H |
  | L | X | X | X | X | L | H | H | H | L | H | H | L | H |
  | L | X | X | X | L | H | H | H | H | H | L | L | L | H |
  | L | X | X | L | H | H | H | H | H | H | L | H | L | H |
  | L | X | L | H | H | H | H | H | H | H | H | L | L | H |
  | L | L | H | H | H | H | H | H | H | H | H | H | L | H |

  Model `priority`: as 74_148 plus `"disabled": "z"`. Confirmed by the table: A0–A2 are Z both when EI is high and when no input is active, GS/EO stay driven (H H, and H L). Model: disabled → code Z, GS/EO inactive (H H) ✓; idle → code Z, GS H, EO L ✓; active rows as '148 ✓.
- **Timing:** data 1–7 to A0–A2 in-phase output tPLH 11 / tPHL 20 typ at 5 V, CL = 45 pF, RL = 667 Ω (p.4). (Out-of-phase 23 / 23; EI to A tPZH 25, tPZL 24, tPHZ 18, tPLZ 23 — enable/disable times not modeled separately.)
- **Supply:** SN74LS348 4.75–5.25 V; SN54LS348 4.5–5.5 V (p.3). No SN54 part in the list, so no `sn54vcc`.
- **Checks against the model:** exact for every table row; the 3-state enable/disable times use the main-path delays.

## 4532B — 8-bit priority encoder

- **PDFs:** encoder/cd4532b.pdf (scanned) p.1 (terminal assignment, Recommended Operating Conditions, description), p.2 (dynamic characteristics), p.3 (logic diagram, truth table)
- **Families:** C (CD4532B)
- **Pin names:** as printed: D0..D7, EI (E-subscript-I), Q0 Q1 Q2, GS, EO (E-subscript-O), VDD, VSS.
- **Pinouts:** `p16` for N, D, NS, PW (p.1 terminal assignment; the description lists E, M/M96, NSR, PW/PWR packages with the same terminal assignment).

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | D4 | input | 9 | Q0 | output |
  | 2 | D5 | input | 10 | D0 | input |
  | 3 | D6 | input | 11 | D1 | input |
  | 4 | D7 | input | 12 | D2 | input |
  | 5 | EI | input | 13 | D3 | input |
  | 6 | Q2 | output | 14 | GS | output |
  | 7 | Q1 | output | 15 | EO | output |
  | 8 | VSS | ground | 16 | VDD | power |

  Cross-checked against the logic diagram pin numbers (p.3).
- **Function:** truth table p.3 (1 = high, 0 = low):

  | EI | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | GS | Q2 | Q1 | Q0 | EO |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | 0 | X | X | X | X | X | X | X | X | 0 | 0 | 0 | 0 | 0 |
  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
  | 1 | 1 | X | X | X | X | X | X | X | 1 | 1 | 1 | 1 | 0 |
  | 1 | 0 | 1 | X | X | X | X | X | X | 1 | 1 | 1 | 0 | 0 |
  | 1 | 0 | 0 | 1 | X | X | X | X | X | 1 | 1 | 0 | 1 | 0 |
  | 1 | 0 | 0 | 0 | 1 | X | X | X | X | 1 | 1 | 0 | 0 | 0 |
  | 1 | 0 | 0 | 0 | 0 | 1 | X | X | X | 1 | 0 | 1 | 1 | 0 |
  | 1 | 0 | 0 | 0 | 0 | 0 | 1 | X | X | 1 | 0 | 1 | 0 | 0 |
  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | X | 1 | 0 | 0 | 1 | 0 |
  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 |

  Model `priority`: `inputs` D0..D7, `inputsActive` high, `code` Q0 Q1 Q2, `codeActive` high, `offset`/`idle` 0 (defaults), `enableIn` EI high, `groupSelect` GS high, `enableOut` EO high, `disabled` inactive. Check: EI 0 → code idle 0, GS 0, EO 0 ✓; EI 1, none → 000, GS 0, EO 1 ✓; Dn wins → Q = n, GS 1, EO 0 ✓ (D0 → 000 with GS 1).
- **Timing:** Dn to Qm tPHL/tPLH typ 220 ns at VDD = 5 V, CL = 50 pF (p.2). rise = fall = 220.
- **Supply:** VDD 3–18 V (p.1). The F3A (military) part is not in the list.
- **Checks against the model:** exact.

## 40147B — 10-line to 4-line BCD priority encoder

- **PDFs:** encoder/cd40147b.pdf (scanned) p.1 (description, logic diagram, Recommended Operating Conditions, truth table), p.3 (dynamic characteristics, terminal assignment)
- **Families:** C (CD40147B)
- **Pin names:** the sheet prints the data inputs as bare digits 0..9 → I0..I9 (as 74_147); outputs A B C D as printed (A = 1, D = 8); VDD, VSS.
- **Pinouts:** `p16` for N, D (p.3 terminal assignment).

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | I4 | input | 9 | A | output |
  | 2 | I5 | input | 10 | I9 | input |
  | 3 | I6 | input | 11 | I1 | input |
  | 4 | I7 | input | 12 | I2 | input |
  | 5 | I8 | input | 13 | I3 | input |
  | 6 | C | output | 14 | D | output |
  | 7 | B | output | 15 | I0 | input |
  | 8 | VSS | ground | 16 | VDD | power |

  Cross-checked against the logic diagram pin numbers (p.1: 0 = 15, 1 = 11, 2 = 12, 3 = 13, 4 = 1 … 8 = 5, 9 = 10; A = 9, B = 7, C = 6, D = 14).
- **Function:** truth table p.1, printed in **negative logic** ("0 = High level, 1 = Low level"):

  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | D | C | B | A |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 |
  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
  | X | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
  | X | X | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
  | X | X | X | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
  | X | X | X | X | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 |
  | X | X | X | X | X | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 1 |
  | X | X | X | X | X | X | 1 | 0 | 0 | 0 | 0 | 1 | 1 | 0 |
  | X | X | X | X | X | X | X | 1 | 0 | 0 | 0 | 1 | 1 | 1 |
  | X | X | X | X | X | X | X | X | 1 | 0 | 1 | 0 | 0 | 0 |
  | X | X | X | X | X | X | X | X | X | 1 | 1 | 0 | 0 | 1 |

  So inputs are active low (a "1" input is a low level), and the code bits are active low (a "1" output is a low level). The text (p.1) agrees: "All four output lines are logic 1 (VSS) when all input lines are logic 0", i.e. with no input low every output is LOW; "functionally similar to the 74147 if pin 15 is tied low" (input 0 permanently active → code 0 → all outputs high, the '147 idle state).

  Model `priority`: `inputs` I0..I9 (index = digit), `inputsActive` low, `code` A B C D (LSB first), `codeActive` low, `offset` 0, `idle` 15.
  Arithmetic: input n wins → value 0 + n = n; each code bit b is driven NOT b (codeActive low). n = 0 → 0000 → D C B A = H H H H (negative logic 0000 ✓); n = 5 → 0101 → H L H L (neg. 0101 ✓); n = 9 → 1001 → L H H L (neg. 1001 ✓). No input low → value = idle = 15 = 1111 → every output NOT 1 = L (neg. 1111 ✓). offset + inputs = 10 ≤ 16; idle 15 < 16.
- **Timing:** any input to any output, in-phase output, tPLH/tPHL typ 450 ns at VDD = 5 V, CL = 50 pF (p.3). Input low → code bit low is in-phase (both active low), so the in-phase figure is the main path. (Out-of-phase 425.) rise = fall = 450.
- **Supply:** VDD 3–18 V (p.1).
- **Checks against the model:** exact. Note the sheet's 40147B differs from the '147 in having an explicit input 0 and an all-low idle code; both are expressed by `idle` 15 and 10 inputs.

## 74_47 — BCD-to-seven-segment decoder/driver, open-collector active-low outputs

- **PDFs:** decoder/sn7447a.pdf (scanned) p.1 (pinouts), p.2 (output configuration table), p.3 (display figure, function table T1, notes), p.5 (logic diagram), p.9 (Recommended Operating Conditions, switching)
- **Families:** TTL (SN7447A)
- **Pin names:** as printed, without overbars: A B C D (A = 1, D = 8), LT, RBI, BI/RBO, segments a..g.
- **Pinouts:** `p16` for N (p.1, "SN7446A, SN7447A, SN7448 ... N package").

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | B | input | 9 | e | output |
  | 2 | C | input | 10 | d | output |
  | 3 | LT | input | 11 | c | output |
  | 4 | BI/RBO | inout | 12 | b | output |
  | 5 | RBI | input | 13 | a | output |
  | 6 | D | input | 14 | g | output |
  | 7 | A | input | 15 | f | output |
  | 8 | GND | ground | 16 | VCC | power |

- **Function:** function table T1 ('46A, '47A, 'LS47), p.3; ON = segment output low (sinking), OFF = released:

  | Code | LT | RBI | D C B A | BI/RBO | a | b | c | d | e | f | g |
  |---|---|---|---|---|---|---|---|---|---|---|---|
  | 0 | H | H | L L L L | H | ON | ON | ON | ON | ON | ON | OFF |
  | 1 | H | X | L L L H | H | OFF | ON | ON | OFF | OFF | OFF | OFF |
  | 2 | H | X | L L H L | H | ON | ON | OFF | ON | ON | OFF | ON |
  | 3 | H | X | L L H H | H | ON | ON | ON | ON | OFF | OFF | ON |
  | 4 | H | X | L H L L | H | OFF | ON | ON | OFF | OFF | ON | ON |
  | 5 | H | X | L H L H | H | ON | OFF | ON | ON | OFF | ON | ON |
  | 6 | H | X | L H H L | H | OFF | OFF | ON | ON | ON | ON | ON |
  | 7 | H | X | L H H H | H | ON | ON | ON | OFF | OFF | OFF | OFF |
  | 8 | H | X | H L L L | H | ON | ON | ON | ON | ON | ON | ON |
  | 9 | H | X | H L L H | H | ON | ON | ON | OFF | OFF | ON | ON |
  | 10 | H | X | H L H L | H | OFF | OFF | OFF | ON | ON | OFF | ON |
  | 11 | H | X | H L H H | H | OFF | OFF | ON | ON | OFF | OFF | ON |
  | 12 | H | X | H H L L | H | OFF | ON | OFF | OFF | OFF | ON | ON |
  | 13 | H | X | H H L H | H | ON | OFF | OFF | ON | OFF | ON | ON |
  | 14 | H | X | H H H L | H | OFF | OFF | OFF | ON | ON | ON | ON |
  | 15 | H | X | H H H H | H | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
  | BI | X | X | X X X X | L (input) | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
  | RBI | H | L | L L L L | L (output) | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
  | LT | L | X | X X X X | H | ON | ON | ON | ON | ON | ON | ON |

  Notes (p.3): 1. BI must be open or high for functions 0–15; RBI open or high unless zero blanking is wanted. 2. "When a low logic level is applied directly to the blanking input (BI), all segment outputs are off regardless of the level of any other input." 3. RBI low and A–D low with LT high → all segments off and RBO goes low. 4. "When the blanking input/ripple blanking output (BI/RBO) is open or held high and a low is applied to the lamp-test input, all segment outputs are on." The description adds "Lamp test (LT) ... may be performed at any time when the BI/RBO node is at a high level."

  `glyphs` (a..g, 1 = ON) transcribed from T1 and checked against the p.3 display figure (6 and 9 without tails; 10–14 the special symbols; 15 blank) and against the '48 table T2 / 'LS49 table T3 (p.4), which give the same patterns:
  0 `1111110`, 1 `0110000`, 2 `1101101`, 3 `1111001`, 4 `0110011`, 5 `1011011`, 6 `0011111`, 7 `1110000`, 8 `1111111`, 9 `1110011`, 10 `0001101`, 11 `0011001`, 12 `0100011`, 13 `1001011`, 14 `0001111`, 15 `0000000`.

  Model `seg7`: `inputs` A B C D, `segments` a..g, `segmentsActive` low, `output` open-drain (open-collector, p.2), `lampTest` LT low, `rbi` RBI, `rbo` BI/RBO (role inout, wire-AND: driven low by ripple blanking, else released; read low → blank), no `blank` key (BI is the same pin).
  **Priority:** the datasheet gives BI (low on BI/RBO) > LT (note 2 "regardless of the level of any other input"; note 4 / description: LT only when BI/RBO is high) > ripple blank (note 3: only with LT high) > glyph. `decoder.c` implements exactly this ("blank (BI) > lamp test > ripple blank > glyph"), and RBO is pulled low only with RBI low, code 0 and LT inactive (note 3 ✓). **The plan text (plan-19-B §4.3, "Priority: lamp test > blank …") is wrong and should read blank > lamp test.**
- **Timing:** only maxima are printed: turn-off / turn-on time from A input, max 100 / 100 ns at VCC = 5 V, CL = 15 pF, RL = 120 Ω (p.9). Segment off = output high = rise = toff = 100; on = fall = ton = 100.
- **Supply:** SN7447A 4.75–5.25 V; SN5447A 4.5–5.5 V (p.9). No SN54 part in the list, so no `sn54vcc`.
- **Checks against the model:** BI/RBO has an internal pull-up (p.7: 2.4 kΩ / 6 kΩ); the model releases RBO to Z and reads Z as "not blanked", which gives the same result. Open TTL inputs (LT, RBI, A–D) read high on the real part (notes 1, 4) but read X under the library's signal rules (plan-10-A §3.5) — the user must tie them. The 15 V segment rating and lamp-intensity modulation via BI are outside the model.

## 74_49 — BCD-to-seven-segment decoder/driver, open-collector active-high outputs

- **PDFs:** decoder/sn7447a.pdf (scanned) p.1 (pinouts), p.2 (output configuration table), p.3 (display figure), p.4 (function table T3), p.6 (logic diagram), p.13 (Recommended Operating Conditions, switching)
- **Families:** LS (SNJ54LS49)
- **Pin names:** as printed, without overbars: A B C D, BI, segments a..g.
- **Pinouts:** `p14` for J (p.1, "SN54LS49 ... J or W package"; 14-pin).

  | Pin | Name | Role | Pin | Name | Role |
  |---|---|---|---|---|---|
  | 1 | B | input | 8 | d | output |
  | 2 | C | input | 9 | c | output |
  | 3 | BI | input | 10 | b | output |
  | 4 | D | input | 11 | a | output |
  | 5 | A | input | 12 | g | output |
  | 6 | e | output | 13 | f | output |
  | 7 | GND | ground | 14 | VCC | power |

  Cross-checked against the 'LS49 logic diagram / logic symbol pin numbers (p.2, p.6).
- **Function:** function table T3 ('LS49), p.4; H = output off (released, segment on with active-high drive):

  | Code | D C B A | BI | a | b | c | d | e | f | g |
  |---|---|---|---|---|---|---|---|---|---|
  | 0 | L L L L | H | H | H | H | H | H | H | L |
  | 1 | L L L H | H | L | H | H | L | L | L | L |
  | 2 | L L H L | H | H | H | L | H | H | L | H |
  | 3 | L L H H | H | H | H | H | H | L | L | H |
  | 4 | L H L L | H | L | H | H | L | L | H | H |
  | 5 | L H L H | H | H | L | H | H | L | H | H |
  | 6 | L H H L | H | L | L | H | H | H | H | H |
  | 7 | L H H H | H | H | H | H | L | L | L | L |
  | 8 | H L L L | H | H | H | H | H | H | H | H |
  | 9 | H L L H | H | H | H | H | L | L | H | H |
  | 10 | H L H L | H | L | L | L | H | H | L | H |
  | 11 | H L H H | H | L | L | H | H | L | L | H |
  | 12 | H H L L | H | L | H | L | L | L | H | H |
  | 13 | H H L H | H | H | L | L | H | L | H | H |
  | 14 | H H H L | H | L | L | L | H | H | H | H |
  | 15 | H H H H | H | L | L | L | L | L | L | L |
  | BI | X X X X | L | L | L | L | L | L | L | L |

  Note 2: "When a low logic level is applied directly to the blanking input (BI), all segment outputs are low regardless of the level of any other input."
  `glyphs` (1 = H = segment on), identical to 74_47 and to the p.3 display figure: 0 `1111110`, 1 `0110000`, 2 `1101101`, 3 `1111001`, 4 `0110011`, 5 `1011011`, 6 `0011111`, 7 `1110000`, 8 `1111111`, 9 `1110011`, 10 `0001101`, 11 `0011001`, 12 `0100011`, 13 `1001011`, 14 `0001111`, 15 `0000000`.

  Model `seg7`: `inputs` A B C D, `segments` a..g, `segmentsActive` high, `output` open-drain (open-collector, active high, p.2: segment on = output released), `blank` BI low; no lamp test, no RBI/RBO.
- **Timing:** only maxima are printed: A input to output tPHL / tPLH max 100 / 100 ns at VCC = 5 V, CL = 15 pF, RL = 4 kΩ (p.13). (The same table also lists "from RBI input" rows, which the 'LS49 does not have — a copy from the 'LS48 table on p.12.)
- **Supply:** SN74LS49 4.75–5.25 V, SN54LS49 4.5–5.5 V (p.13). The only part is SNJ54LS49J → `sn54vcc` LS [4.5, 5.5].
- **Checks against the model:** exact for every table row. An open BI reads high on the real part (note 1) but X in the model (signal rules). "High" output = open-collector released: in the simulator the segment pin goes Z (needs a pull-up to read 1), matching the real part.
