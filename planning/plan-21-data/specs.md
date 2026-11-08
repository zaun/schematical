# plan-21 device specs

One section per device that plan-21 adds (every device with an `add` row in
`parts.tsv`), read 2026-11-07 from the datasheets in `datasheets/mux/`.
Page numbers are PDF page indices. Pin names drop overbars (a complement
output is `W` or `YBAR`, as its sheets name it); active levels are in each
section's Function. Each section ends with the device's model keys
(docs/parts-format.md § `mux`) and the points where the model is not exact.
Devices in plan-21-A §3.1 order: model `select`, then `stored`, then
`regfile`.

Devices of plan-21-A §3.1 with no section, because no part of theirs is
added (`parts.tsv` reasons):

- No TI datasheet (TI has no product page and no ti.com/lit PDF; DigiKey
  links only Rochester Electronics or DigiKey-hosted copies): 74_352
  (SN74ALS352D, SN74AS352N, SN74F352N, SNJ54LS352W), 74_353 (SN54ALS353J,
  SNJ54ALS353J, SN74AS353BD/BDR, SNJ54LS353FK), 74_850 (SN74AS850ADW/AFN),
  74_851 (SN74AS851BDW), 74_604 (SN74LS604N), 74_355 (SN74LS355DW/N), the
  LS354 (SN74LS354N; the HC/HCT354 are added), 74_11151 and 74_11251
  (74ACT11151D/N, 74ACT11251D/N), 74_11157 (74AC11157DWR, 74ACT11157N),
  74_162460 (SN74ABT162460DL), the AHC 2G157 (SN74AHC2G157HDCT3/HDCTR) and
  SNJ54F158AJ.
- Every candidate Obsolete or Discontinued at DigiKey: 74_250
  (SN74AS250A), 74_857 (SN74ALS857), 74_870 (SN74ALS870), 74_16460
  (SN74ABTH16460) and the ABTH162460 parts.

So plan-21-C's models `universal` ('857) and `bus4` ('16460/'162460) carry
no part and are not built (plan-21-C Corrections).

## 74_151 — 8-line to 1-line data selectors/multiplexers

- **PDFs:** mux/sn74hc151.pdf p.3 (pinout D/J/N/NS/PW, FK), p.4 (ROC), p.5 (timing, 50 pF), p.9 (function table); mux/sn74hc151-q1.pdf p.1 (D pinout, function table), p.3 (ROC), p.4 (timing); mux/cd74hc151.pdf p.2 (pin-name changes), p.3 (pinout J/N/D), p.4 (ROC HC and HCT), p.6 (timing), p.9 (function table); mux/sn74hcs151.pdf p.3 (pinout D/PW, WBQB), p.4 (ROC), p.5 (timing, read from the page image), p.12 (function table); mux/sn74hcs151-q1.pdf p.3 (D/PW, WBQB), p.4, p.6, p.13; mux/cd74ac151.pdf p.3 (D/N/PW, BQB), p.4 (ROC), p.6 (timing, max only), p.12 (function table); mux/sn74ac151-q1.pdf p.3 (PW, BQB), p.4 (ROC), p.6–7 (timing with typ), p.14; mux/cd74act151.pdf p.3 (J/D/PW, BQB), p.4, p.5–6, p.11; mux/sn74act151-q1.pdf p.3 (PW, BQB), p.4, p.5, p.12; mux/sn74ls151.pdf p.1 (D/J/N/W pinout, FK pinout), p.2 (function table), p.5 (LS ROC and timing) — scanned, read from the page images; mux/sn74als151.pdf p.1 (D/J/N, FK, function table), p.3 (ALS ROC), p.4 (ALS timing, AS ROC), p.5 (AS timing); mux/sn74f151b.pdf p.1 (D/J/N, FK, function table), p.4 (ROC, timing). (txt copies sn54ls151 = sn74ls151, sn74as151 = sn74als151, cd74hct151 = cd74hc151, same files.)
- **Families:** HC (SN74HC151, SN74HC151-Q1, CD74HC151), HCT (CD74HCT151), HCS (SN74HCS151, SN74HCS151-Q1), AC (CD74AC151, SN74AC151-Q1), ACT (CD74ACT151, SN74ACT151-Q1), LS (SN74LS151, SNJ54LS151), ALS (SN74ALS151), AS (SN74AS151), F (SN74F151B).
- **Pin names:** every sheet prints the classic SN names D0–D7, A, B, C, G (strobe, overbar), Y, W (sn74hc151 p.3, sn74hc151-q1 p.1, cd74hc151 p.3, sn74hcs151 p.3, cd74ac151 p.3, sn74ac151-q1 p.3, cd74act151 p.3, sn74act151-q1 p.3, sn74ls151 p.1, sn74als151 p.1, sn74f151b p.1). No sheet uses S0–S2 / E for this device; cd74hc151 p.2 records that its older revision used I0–I7, S0–S2 (S0 = A), E (= G) and Y/Ȳ (now Y/W), so the current names match. Checked pin by pin: same numbers on every sheet. A = LSB (function table: A toggles fastest).
- **Pinouts:**

**p16** — N, D, PW, NS, BQB (sn74hc151 p.3: J/D/N/NS/PW; cd74ac151 p.3: D/N/PW and BQB "pad can be connected to GND or left floating"; sn74ac151-q1, cd74act151, sn74act151-q1 p.3: PW and BQB; sn74hcs151-q1 p.3: WBQB; sn74ls151 p.1, sn74als151 p.1, sn74f151b p.1: D/N. NS for LS, ALS, AS and F is only in those sheets' package addendum; same 16-pin order assumed, as on sn74hc151 p.3)

| Pin | Name | Role |
|---|---|---|
| 1 | D3 | input |
| 2 | D2 | input |
| 3 | D1 | input |
| 4 | D0 | input |
| 5 | Y | output |
| 6 | W | output |
| 7 | G | input |
| 8 | GND | ground |
| 9 | C | input |
| 10 | B | input |
| 11 | A | input |
| 12 | D7 | input |
| 13 | D6 | input |
| 14 | D5 | input |
| 15 | D4 | input |
| 16 | VCC | power |

**fk20** — FK (SNJ54LS151FK; sn74ls151 p.1, same as sn74hc151 p.3, sn74als151 p.1, sn74f151b p.1)

| Pin | Name | Role |
|---|---|---|
| 1 | NC | nc |
| 2 | D3 | input |
| 3 | D2 | input |
| 4 | D1 | input |
| 5 | D0 | input |
| 6 | NC | nc |
| 7 | Y | output |
| 8 | W | output |
| 9 | G | input |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | C | input |
| 13 | B | input |
| 14 | A | input |
| 15 | D7 | input |
| 16 | NC | nc |
| 17 | D6 | input |
| 18 | D5 | input |
| 19 | D4 | input |
| 20 | VCC | power |

- **Function:** every family prints the same table (sn74hc151 p.9, sn74ls151 p.2, sn74als151 p.1, sn74f151b p.1, cd74ac151 p.12; cd74hc151 p.9 expands it per data input):

  | C | B | A | G | Y | W |
  |---|---|---|---|---|---|
  | X | X | X | H | L | H |
  | L | L | L | L | D0 | D̅0 |
  | L | L | H | L | D1 | D̅1 |
  | L | H | L | L | D2 | D̅2 |
  | L | H | H | L | D3 | D̅3 |
  | H | L | L | L | D4 | D̅4 |
  | H | L | H | L | D5 | D̅5 |
  | H | H | L | L | D6 | D̅6 |
  | H | H | H | L | D7 | D̅7 |

  Model `select`, one unit: `data` D0..D7, `select` [A, B, C] (A = LSB), `y` Y, `w` W, `strobe` {G, low}; `disabled` "low" (G high: Y low, W high).

- **Timing:** (main path data to Y; rise = tPLH, fall = tPHL)

- HC: rise 23 ns, fall 23 ns; vcc 2–6 V. Any D to Y/W typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc151 p.4, p.5)
- HCT: rise 38 ns, fall 38 ns; vcc 4.5–5.5 V. Any D to Y max, 4.5 V 50 pF, 25 C (typ 16 ns only at 5 V 15 pF); VCC 4.5-5.5 V (cd74hc151 p.4, p.6)
- HCS: rise 9 ns, fall 9 ns; vcc 2–6 V. Any D to Y/W typ, 4.5 V 50 pF; VCC 2-6 V (sn74hcs151 p.4, p.5)
- AC: rise 6.3 ns, fall 6.3 ns; vcc 1.5–5.5 V. D to Y typ, 5 V 50 pF (sn74ac151-q1 p.7); VCC 1.5-5.5 V (cd74ac151 p.4; sn74ac151-q1 p.4 allows 1.5-6 V)
- ACT: rise 4.6 ns, fall 5.6 ns; vcc 4.5–5.5 V. D to Y typ, 5 V 50 pF (sn74act151-q1 p.5); VCC 4.5-5.5 V (cd74act151 p.4, sn74act151-q1 p.4)
- LS: rise 20 ns, fall 16 ns; vcc 4.75–5.25 V. Any D to Y typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls151 p.5)
- ALS: rise 10 ns, fall 15 ns; vcc 4.5–5.5 V. Any D to Y max, 4.5-5.5 V 50 pF, SN74; VCC 4.5-5.5 V (sn74als151 p.3, p.4)
- AS: rise 10.5 ns, fall 11 ns; vcc 4.5–5.5 V. Any D to Y max, 4.5-5.5 V 50 pF; VCC 4.5-5.5 V (sn74als151 p.4, p.5)
- F: rise 3.7 ns, fall 4.2 ns; vcc 4.5–5.5 V. Any D to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74f151b p.4)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls151 p.5) — `sn54vcc` LS [4.5, 5.5]. HC 2–6 V (SN54 same, sn74hc151 p.4; -Q1 p.3 same); HCT 4.5–5.5 V (cd74hc151 p.4); HCS 2–6 V; ALS and AS 4.5–5.5 V (SN54ALS same); F 4.5–5.5 V (SN54F same); ACT 4.5–5.5 V. AC: CD74AC151 ROC 1.5–5.5 V (cd74ac151 p.4) while SN74AC151-Q1 allows 1.5–6 V (sn74ac151-q1 p.4); one entry per family, the common 1.5–5.5 V (as plan-19 74_139). No family has 5.5 V-tolerant inputs (VI 0–VCC on every CMOS sheet).
- **Checks against the model:** exact (G high → Y low, W high = `disabled` "low"; W is the complement of Y in every row). Timing notes: CD74AC151 prints only max (12.3 ns D to Y at 5 V, -40 to 85 C), so the AC typical comes from the -Q1 sheet; CD74ACT151 prints only max (14.1 ns), the ACT typical comes from SN74ACT151-Q1, whose typ tPHL (5.6 ns) is above tPLH (4.6 ns). ALS and AS print only max. The sn74als151 p.1 function table prints W for code 000 as "D0" without the overbar (a typo; every other row and every other sheet shows D̅0).

## 74_153 — Dual 4-line to 1-line data selectors/multiplexers

- **PDFs:** mux/sn74hc153.pdf p.3 (pinouts J/W/D/N/NS/PW and FK), p.4 (ROC), p.5 (switching), p.9 (function table); mux/cd74hc153.pdf p.1 (pinout, CD names), p.2 (truth table), p.3 (supply), p.4–5 (switching); mux/sn74hcs153.pdf p.3 (D/PW), p.4 (ROC), p.5 (switching), p.10 (function table); mux/sn74hcs153-q1.pdf p.3 (D/PW, WBQB), p.4, p.6, p.12; mux/sn74ls153.pdf p.1 (pinouts J/W/D/N and FK, function table), p.5 (LS ROC + switching) — scanned, read from the page images; mux/sn74als153.pdf p.1 (pinouts J/D/N, FK; function table), p.3 (ALS ROC), p.4 (ALS switching, AS ROC), p.5 (AS switching); mux/sn74f153.pdf p.1 (pinouts J/D/N, FK; function table), p.4 (ROC), p.5 (switching); mux/cd74ac153.pdf p.3 (J/D/N/PW and BQB), p.4 (ROC), p.6 (switching), p.9–10 (function table); mux/sn74ac153-q1.pdf p.3 (PW, BQB), p.4 (ROC), p.6 (switching), p.14 (function table); mux/cd74act153.pdf p.1 (pinout F/E/M, function table), p.3 (ROC), p.4 (switching); mux/sn74act153.pdf p.3 (PW, BQB), p.4 (ROC), p.5 (switching), p.10 (function table); mux/sn74act153-q1.pdf p.3 (PW, BQB), p.4, p.5, p.11
- **Families:** HC (SN74HC153, SN54HC153, CD74HC153), HCT (CD74HCT153), HCS (SN74HCS153, SN74HCS153-Q1), LS (SN74LS153, SNJ54LS153), ALS (SN74ALS153, SN54ALS153), AS (SN74AS153), F (SN74F153, SNJ54F153), AC (CD74AC153, SN74AC153-Q1), ACT (CD74ACT153, SN74ACT153, SN74ACT153-Q1)
- **Pin names:** the classic SN names 1G, 2G (strobes, active low), A, B (selects, A = LSB), 1C0–1C3, 2C0–2C3, 1Y, 2Y — printed by sn74hc153, sn74hcs153(-q1), sn74ls153, sn74als153, sn74f153, cd74act153, sn74act153(-q1), sn74ac153-q1 and the current cd74ac153 (rev C, 2025). cd74hc153 (Harris) prints other names, checked pin by pin against its p.1 pinout and p.2 functional diagram: 1E = 1G (pin 1), S1 = B (pin 2), 1I3..1I0 = 1C3..1C0 (pins 3–6), 1Y (7), 2Y (9), 2I0..2I3 = 2C0..2C3 (pins 10–13), S0 = A (pin 14), 2E = 2G (pin 15). Its truth table (p.2) selects I0 with S1 S0 = L L and I1 with S1 S0 = L H, so S0 = A = LSB. Every sheet has the same pin numbers.
- **Pinouts:**

**p16** — N, D, PW, NS, J, W, BQB. sn74hc153 p.3 (J, W, D, N, NS, PW); sn74ls153 p.1 (J/W, D, N); sn74als153 p.1 and sn74f153 p.1 (J, D, N); cd74ac153 p.3 and sn74ac153-q1 / sn74act153 / sn74act153-q1 p.3 (BQB: pins 1–16 in DIP order, thermal pad "can be connected to GND or left floating", not a pin); sn74hcs153-q1 p.3 (WBQB, the same drawing, code BQB). The old ALS/F/LS sheets draw only D/N (and J/W) but their NS (SOP) parts are listed in their orderable addenda; NS has the DIP order on sn74hc153 p.3. SNJ54ALS153W is not drawn on sn74als153 (only J); W is the DIP order on sn74hc153 p.3 and sn74ls153 p.1. CD74ACT153NSR is in cd74act153's addendum only (the 2003 sheet draws E/M); NS = DIP order as above.

| Pin | Name | Role |
|---|---|---|
| 1 | 1G | input |
| 2 | B | input |
| 3 | 1C3 | input |
| 4 | 1C2 | input |
| 5 | 1C1 | input |
| 6 | 1C0 | input |
| 7 | 1Y | output |
| 8 | GND | ground |
| 9 | 2Y | output |
| 10 | 2C0 | input |
| 11 | 2C1 | input |
| 12 | 2C2 | input |
| 13 | 2C3 | input |
| 14 | A | input |
| 15 | 2G | input |
| 16 | VCC | power |

**fk20** — FK (SNJ54HC153FK, SNJ54LS153FK, SNJ54F153FK; sn74hc153 p.3, sn74ls153 p.1, sn74f153 p.1, sn74als153 p.1 — all identical)

| Pin | Name | Role |
|---|---|---|
| 1 | NC | nc |
| 2 | 1G | input |
| 3 | B | input |
| 4 | 1C3 | input |
| 5 | 1C2 | input |
| 6 | NC | nc |
| 7 | 1C1 | input |
| 8 | 1C0 | input |
| 9 | 1Y | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | 2Y | output |
| 13 | 2C0 | input |
| 14 | 2C1 | input |
| 15 | 2C2 | input |
| 16 | NC | nc |
| 17 | 2C3 | input |
| 18 | A | input |
| 19 | 2G | input |
| 20 | VCC | power |

- **Function:** every sheet prints the same table (sn74hc153 p.9; sn74ls153 p.1; sn74als153 p.1; sn74f153 p.1; cd74act153 p.1; cd74ac153 p.9–10; sn74act153 p.10; sn74hcs153 p.10; cd74hc153 p.2 with S1/S0/I/E names):

  | B | A | C0 | C1 | C2 | C3 | G | Y |
  |---|---|---|---|---|---|---|---|
  | X | X | X | X | X | X | H | L |
  | L | L | L | X | X | X | L | L |
  | L | L | H | X | X | X | L | H |
  | L | H | X | L | X | X | L | L |
  | L | H | X | H | X | X | L | H |
  | H | L | X | X | L | X | L | L |
  | H | L | X | X | H | X | L | H |
  | H | H | X | X | X | L | L | L |
  | H | H | X | X | X | H | L | H |

  Select inputs A and B are common to both sections. Model `select`, two units: unit 1 `data` [1C0, 1C1, 1C2, 1C3], `select` [A, B] (A = LSB), `y` 1Y, `strobe` {1G, low}; unit 2 the same with 2C0..2C3, 2Y, 2G. `disabled` "low" (G high → Y low), `invert` false, push-pull, no `w`.

- **Timing:** (main path data → Y; rise = tPLH, fall = tPHL)

- HC: rise 17 ns, fall 17 ns; vcc 2–6 V. Data (any C) to Y tpd typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc153 p.4, p.5). (cd74hc153 p.5: I to Y 29 max at 4.5 V 50 pF, 12 typ at 5 V 15 pF.)
- HCT: rise 34 ns, fall 34 ns; vcc 4.5–5.5 V. I to Y max, 4.5 V 50 pF, 25 °C (cd74hc153 p.5). The HCT block prints two "I to Y" rows: 24 max (9 typ at 15 pF) and 34 max (14 typ at 15 pF); the second matches the S to Y row and the first matches HC's E to Y row, so one is mislabeled; the slower 34 is used. Only typ is at 5 V 15 pF.
- HCS: rise 8 ns, fall 8 ns; vcc 2–6 V. Data (any C) to Y tpd typ, 4.5 V 50 pF; VCC 2-6 V (sn74hcs153 p.4, p.5; sn74hcs153-q1 p.6 same)
- LS: rise 10 ns, fall 17 ns; vcc 4.75–5.25 V. Data to Y typ, 5 V 15 pF, RL 2 kΩ; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls153 p.5)
- ALS: rise 10 ns, fall 15 ns; vcc 4.5–5.5 V. Data (any C) to Y max SN74ALS153, 4.5-5.5 V 50 pF; no typ printed (sn74als153 p.3, p.4). SN54ALS153: 15 / 18.
- AS: rise 7 ns, fall 8 ns; vcc 4.5–5.5 V. Data (any C) to Y max, 4.5-5.5 V 50 pF; no typ printed (sn74als153 p.4, p.5)
- F: rise 4.9 ns, fall 4.7 ns; vcc 4.5–5.5 V. C to Y typ, 5 V 50 pF (sn74f153 p.4, p.5)
- AC: rise 13.3 ns, fall 13.3 ns; vcc 1.5–5.5 V. Any C to Y max, 5 V 50 pF, −55 to 125 °C (cd74ac153 p.6; sn74ac153-q1 p.6 same 13.3 for −40 to 125 °C; −40 to 85 °C column 12.1); no typ printed on either sheet. VCC 1.5-5.5 V (cd74ac153 p.4; sn74ac153-q1 p.4 allows 1.5-6 V)
- ACT: rise 5.8 ns, fall 6.1 ns; vcc 4.5–5.5 V. C to Y typ, 5 V 50 pF (sn74act153 p.4, p.5; sn74act153-q1 p.5 same). (cd74act153 p.4: 16.4 max −40 to 85 °C, 18 max −55 to 125 °C.)

- **Supply:** HC 2–6 V for SN54HC/SN74HC (sn74hc153 p.4) and CD74HC (cd74hc153 p.3); HCT 4.5–5.5 V (cd74hc153 p.3); HCS 2–6 V (sn74hcs153 p.4, sn74hcs153-q1 p.4); LS SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls153 p.5) → `sn54vcc` LS [4.5, 5.5] for SNJ54LS153FK; ALS 4.5–5.5 V both SN54 and SN74 (sn74als153 p.3); AS 4.5–5.5 V (p.4); F 4.5–5.5 V both (sn74f153 p.4); AC 1.5–5.5 V (cd74ac153 p.4; the -Q1 sheet's 1.5–6 V is wider, family entry kept at the CD74 range); ACT 4.5–5.5 V (cd74act153 p.3, sn74act153 p.4). No input tolerance above VCC on any family (VI 0–VCC).
- **Checks against the model:** exact. Strobe high forces Y low on every sheet (`disabled` "low"); selects shared by both units; no complement output, no 3-state. Unused by the model: the separate select→Y and G→Y delays (only the data path is timed).
- **Sheet notes:** cd74ac153 p.3, sn74ac153-q1 p.3 and sn74hcs153(-q1) p.3 list pin 9 (2Y) as type "I" in the pin-function table; the pin drawing, logic diagram and description ("Channel 2, data output") make it an output. SN74ALS153NSR is "Last" (last-time-buy) in sn74als153's addendum.

## 74_157 — Quadruple 2-line to 1-line data selectors/multiplexers

- **PDFs:** mux/sn74hc157.pdf p.3 (J/D/DB/N/NS/PW, FK), p.4 (ROC), p.5 (timing), p.9 (function table); mux/cd74hc157.pdf p.1 (pinout, CERDIP/PDIP/SOIC; ordering), p.2 (truth table), p.3 (supply), p.4–5 (timing); mux/sn74hct157.pdf p.1 (J/W/D/N, FK), p.2 (function table), p.3 (ROC), p.4 (timing); mux/sn74hcs157.pdf p.3 (D/PW), p.4, p.5, p.10; mux/sn74hcs157-q1.pdf p.3 (D/PW, WBQB), p.4, p.6, p.12; mux/sn74ls157.pdf p.1 (pinout J/W/D/N, FK; function table), p.4 ('157 ROC + timing), p.5 (LS ROC + timing), p.6 (S ROC + timing), scanned, read from the page images; mux/sn74als157a.pdf p.1 (J/D/N, FK; function table), p.4–5 (ALS), p.6–7 (AS); mux/sn74f157a.pdf p.1 (J/D/N, FK; function table), p.2 (ROC), p.3 (timing); mux/cd74ac157.pdf p.3 (J, LCCC numbering), p.4 (BQB, D/N), p.6 (ROC), p.8 (timing), p.11 (function table); mux/sn74ac157-q1.pdf p.3 (PW, BQB), p.4 (ROC), p.6 (timing), p.12–13 (function table); mux/cd74act157.pdf p.3 (BQB, D/N/PW), p.4, p.5, p.10; mux/sn74act157-q1.pdf p.3 (PW, BQB), p.4, p.5, p.11; mux/sn74ahc157.pdf p.3 (J/W/D/DB/DGV/N/NS/PW, RGY, FK), p.5 (ROC), p.7 (timing 5 V), p.10 (function table); mux/sn74ahc157-q1.pdf p.3 (PW, WBQB), p.4, p.5–6, p.10–11; mux/sn74ahct157.pdf p.3 (J/W/D/DB/DGV/N/NS/PW, FK), p.4, p.5, p.8; mux/sn74lv157a.pdf p.3 (D/DB/DGV/NS/PW), p.5, p.6, p.9; mux/sn74lvc157a.pdf p.3 (J/W/D/DB/NS/PW, BQB/RGY, FK), p.5 (ROC), p.7 (timing), p.8 (load), p.9 (function table); mux/sn74lvc157a-q1.pdf p.3 (D/PW, BQB), p.4, p.5, p.7; mux/sn74lvc157a-ep.pdf p.1 (D/PW, function table), p.3; mux/cy74fct157t.pdf p.1 (Q/SO, L), p.2 (pin description, ordering, function table), p.4 (ROC), p.5 (timing), addendum (D, DW codes)
- **Families:** HC (SN74HC157, SN54HC157, CD74HC157, CD54HC157), HCT (SN74HCT157, CD74HCT157), HCS (SN74HCS157, SN74HCS157-Q1), LS (SN74LS157, SNJ54LS157), S (SN54S157, SNJ54S157), ALS (SN74ALS157A, SNJ54ALS157A), AS (SN74AS157), F (SN74F157A), AC (CD74AC157, SN74AC157-Q1), ACT (CD74ACT157, SN74ACT157-Q1), AHC (SN74AHC157, SNJ54AHC157, SN74AHC157-Q1), AHCT (SN74AHCT157, CAHCT157QWBQBRQ1 from the AHC-Q1 sheet), LV (SN74LV157A), LVC (SN74LVC157A, SN74LVC157A-Q1, SN74LVC157A-EP), FCT (CY74FCT157T, AT and CT speed grades)
- **Pin names:** the classic SN names A/B (select, printed A̅/B), 1A, 1B, 1Y … 4A, 4B, 4Y, G (strobe, printed G̅), on every SN sheet (HC, HCT, HCS, HCS-Q1, LS/S, ALS/AS, F, AHC, AHC-Q1, AHCT, LV, LVC, LVC-Q1, LVC-EP) and on the modernized CD74AC157, CD74ACT157 and the AC/ACT-Q1 sheets. Two sheets print other names; checked pin by pin they match:
  - cd74hc157 p.1: S (1), 1I0 (2), 1I1 (3), 1Y (4), 2I0 (5), 2I1 (6), 2Y (7), GND, 3Y (9), 3I1 (10), 3I0 (11), 4Y (12), 4I1 (13), 4I0 (14), E̅ (15), VCC. So S = A/B, I0 = A, I1 = B, E̅ = G̅. The p.1 pinout prints the outputs with overbars because the same drawing serves the '158; the p.2 functional diagram and truth table give true outputs (157 column) for the '157.
  - cy74fct157t p.1: S (1), I0a (2), I1a (3), Ya (4), I0b (5), I1b (6), Yb (7), GND, Yc (9), I1c (10), I0c (11), Yd (12), I1d (13), I0d (14), E (15, "active low" per p.2), VCC. So a–d = units 1–4, I0x = xA, I1x = xB, Yx = xY, E = G.
  - The SN74HC, HCS, HCS-Q1, LV, AHC-Q1, CD74AC157 and CD74ACT157 Pin Functions tables print wrong I/O types (Y as I, 2A/2B as O); the pinout drawings and logic diagrams are unambiguous, roles taken from them.
- **Pinouts:**

**p16** — N, D, DW, PW, DB, DGV, NS, J, W, RGY, BQB (pin n = DIP pin n on every sheet). Drawings: sn74hc157 p.3 (J/D/DB/N/NS/PW); cd74hc157 p.1 (CERDIP J, PDIP N, SOIC D); sn74hct157 p.1 (J/W/D/N); sn74hcs157 p.3 and sn74hcs157-q1 p.3 (D/PW); sn74ls157 p.1 (J/W, N, D); sn74als157a p.1 and sn74f157a p.1 (J, D/N); cd74ac157 p.3–4 (J, D/N); cd74act157 p.3 (D/N/PW); sn74ac157-q1, sn74act157-q1, sn74ahc157-q1 p.3 (PW); sn74ahc157 p.3 and sn74ahct157 p.3 (J/W, D/DB/DGV/N/NS/PW); sn74lv157a p.3 (D/DB/DGV/NS/PW); sn74lvc157a p.3 (J/W, D/DB/NS/PW); sn74lvc157a-q1 p.3 and sn74lvc157a-ep p.1 (D/PW). RGY: sn74ahc157 p.3 Fig 3-2 and sn74lvc157a p.3 Fig 3-2, pins 1–16 in DIP order (A/B 1, VCC 16 on the top edge, GND 8, 3Y 9 on the bottom), pad not a pin. BQB (the "WBQB" wettable-flank parts too): cd74ac157 p.4, cd74act157 p.3, sn74ac157-q1, sn74act157-q1, sn74ahc157-q1 (Fig 4-2 "WBQB"), sn74hcs157-q1, sn74lvc157a, sn74lvc157a-q1 p.3, same order, pad "can be connected to GND or left floating". DW: cy74fct157t p.1 "SO" package = DW per the addendum. Not drawn on their sheets but the standard 16-lead order: NS on sn74ls157/sn74als157a/sn74f157a (SN74LS157NSR, SN74ALS157ANSR, SN74F157ANSR) and D on cy74fct157t (CY74FCT157ATDR, CY74FCT157CTD).

| Pin | Name | Role |
|---|---|---|
| 1 | A/B | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Y | output |
| 5 | 2A | input |
| 6 | 2B | input |
| 7 | 2Y | output |
| 8 | GND | ground |
| 9 | 3Y | output |
| 10 | 3B | input |
| 11 | 3A | input |
| 12 | 4Y | output |
| 13 | 4B | input |
| 14 | 4A | input |
| 15 | G | input |
| 16 | VCC | power |

**fk20** — FK (20-pad LCCC of the 16-pin die): sn74hc157 p.3 (SNJ54HC157FK), sn74ls157 p.1 (SN54LS157/SN54S157 FK; SNJ54S157FK); same map on sn74hct157 p.1, sn74als157a p.1, sn74f157a p.1, sn74ahc157 p.3, sn74ahct157 p.3, sn74lvc157a p.3 and the cd74ac157 p.3 LCCC column.

| Pad | Name | Role |
|---|---|---|
| 1 | NC | nc |
| 2 | A/B | input |
| 3 | 1A | input |
| 4 | 1B | input |
| 5 | 1Y | output |
| 6 | NC | nc |
| 7 | 2A | input |
| 8 | 2B | input |
| 9 | 2Y | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | 3Y | output |
| 13 | 3B | input |
| 14 | 3A | input |
| 15 | 4Y | output |
| 16 | NC | nc |
| 17 | 4B | input |
| 18 | 4A | input |
| 19 | G | input |
| 20 | VCC | power |

- **Function:** every sheet prints the same table (sn74hc157 p.9; sn74ls157 p.1 '157/'LS157/'S157 column; sn74als157a p.1; sn74f157a p.1; cd74hc157 p.2 with E̅/S/I0/I1; cy74fct157t p.2 with E/S/I0/I1, rows in another order):

  | G | A/B | A | B | Y |
  |---|---|---|---|---|
  | H | X | X | X | L |
  | L | L | L | X | L |
  | L | L | H | X | H |
  | L | H | X | L | L |
  | L | H | X | H | H |

  Model `select`, four units: unit n `data` [nA, nB] (A/B low → A = data[0]), `select` [A/B] (shared), `y` nY, `strobe` {G, low} (shared). `disabled` "low" (strobe high → every Y low). No `w`, no `oe`, `invert` false.

- **Timing:** (main path data → Y; rise = tPLH, fall = tPHL)

- HC: rise 13 ns, fall 13 ns; vcc 2–6 V. A or B to Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc157 p.5, p.4)
- HCT: rise 18 ns, fall 18 ns; vcc 4.5–5.5 V. A or B to Y typ, 4.5 V 50 pF; VCC 4.5-5.5 V (sn74hct157 p.4, p.3)
- HCS: rise 8 ns, fall 8 ns; vcc 2–6 V. A or B to Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hcs157 p.5, p.4)
- LS: rise 9 ns, fall 9 ns; vcc 4.75–5.25 V. Data to Y typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls157 p.5)
- S: rise 5 ns, fall 4.5 ns; vcc 4.75–5.25 V. Data to Y typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls157 p.6)
- ALS: rise 9 ns, fall 6 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74als157a p.5, p.4)
- AS: rise 6 ns, fall 5.5 ns; vcc 4.5–5.5 V. A or B to Y max (no typ printed), 4.5-5.5 V 50 pF; VCC 4.5-5.5 V (sn74als157a p.7, p.6)
- F: rise 4.1 ns, fall 3.6 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74f157a p.3, p.2)
- AC: rise 5.6 ns, fall 5.5 ns; vcc 1.5–5.5 V. A or B to Y typ, 5 V 50 pF (sn74ac157-q1 p.6); VCC 1.5-5.5 V (cd74ac157 p.6; sn74ac157-q1 p.4 allows 1.5-6 V, see partVcc)
- ACT: rise 6.4 ns, fall 6.6 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF (sn74act157-q1 p.5); VCC 4.5-5.5 V (cd74act157 p.4, sn74act157-q1 p.4)
- AHC: rise 5.6 ns, fall 5.6 ns; vcc 2–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF; VCC 2-5.5 V, inputs to 5.5 V (sn74ahc157 p.7, p.5)
- AHCT: rise 5.6 ns, fall 5.6 ns; vcc 4.5–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V, inputs to 5.5 V (sn74ahct157 p.5, p.4)
- LV: rise 6.7 ns, fall 6.7 ns; vcc 2–5.5 V, inputMax 5.5 V. A or B to Y typ, 3.3 V 50 pF; VCC 2-5.5 V, inputs to 5.5 V (sn74lv157a p.6, p.5)
- LVC: rise 3 ns, fall 3 ns; vcc 1.65–3.6 V, inputMax 5.5 V. A or B to Y typ, 3.3 V 50 pF (load p.8); VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc157a p.7, p.5)
- FCT: rise 5 ns, fall 5 ns; vcc 4.75–5.25 V. I to Y max (no typ printed), CY74FCT157AT grade (CT grade 4.3 ns); VCC CY74 4.75-5.25 V, CY54 4.5-5.5 V (cy74fct157t p.5, p.4)

- **Supply:** LS and S SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls157 p.5, p.6) — `sn54vcc` LS, S [4.5, 5.5] (SNJ54LS157W, SN54S157J, SNJ54S157FK); FCT CY74 4.75–5.25 V, CY54 4.5–5.5 V (cy74fct157t p.4) — `sn54vcc` FCT [4.5, 5.5] (no CY54 add part; kept for completeness). HC 2–6 V (SN54 same, sn74hc157 p.4; CD54/74HC 2–6 V, cd74hc157 p.3); HCT 4.5–5.5 V; HCS 2–6 V (VI 0–VCC); ALS, AS, F 4.5–5.5 V (SN54 same); ACT 4.5–5.5 V; AHC 2–5.5 V, AHCT 4.5–5.5 V, LV 2–5.5 V, all with VI 0–5.5 V (inputMax 5.5); LVC 1.65–3.6 V, VI 0–5.5 V (inputMax 5.5). AC: CD74AC157 1.5–5.5 V (cd74ac157 p.6); SN74AC157-Q1 1.5–6 V (sn74ac157-q1 p.4). `partVcc`: SN74AC157PWRQ1, SN74AC157WBQBRQ1 [1.5, 6] (their sheet's ROC); SN74LVC157ADRQ1, SN74LVC157AQDRG4Q1, SN74LVC157AQPWRQ1, SN74LVC157AWBQBRQ1 [2, 3.6] (sn74lvc157a-q1 p.4: operating 2–3.6 V, 1.5 V data retention only) and SN74LVC157AQPWREP [2, 3.6] (sn74lvc157a-ep p.3), narrower than the SN74LVC157A 1.65–3.6 V.
- **Checks against the model:** exact. Notes: the CY74FCT157T title says "with 3-state outputs", but its description and function table (p.1–2) say E high forces every Y low, and no Z state or enable/disable times are printed, so `disabled` "low" is used like the other families. LS/S print typ at 15 pF only. ALS typ is from the 25 °C column (p.5). AS and FCT print only max. AC and ACT typicals come from the -Q1 sheets (CD74AC157/CD74ACT157 print only max: 7.7 ns and 8.6 ns at 5 V, −40–85 °C). CAHCT157QWBQBRQ1 is listed only in the SN74AHC157-Q1 ordering table (marking AT157Q); the TSV gives it family AHCT, so it takes the AHCT entry (4.5–5.5 V) although its sheet specifies AHC levels and 2–5.5 V.

## 74_158 — Quadruple 2-line to 1-line data selectors/multiplexers

- **PDFs:** mux/cd74ac158.pdf p.1 (M = SOIC pinout, ordering, function table), p.2 (logic diagram), p.3 (ROC), p.4 (timing); mux/cd74act158.pdf p.1 (M pinout, ordering, function table), p.3 (ROC), p.4 (timing); mux/cd74hc157.pdf p.1 (pinout, CD74HCT158 PDIP, ordering), p.2 (truth table, functional diagram), p.3 (operating conditions), p.5 (HC/HCT158 switching); mux/sn74ls157.pdf p.1 (J/W/D/N and FK pinouts, function table), p.3 (logic diagram), p.5 (LS ROC + timing), p.8 (addendum: SN74LS158NSR, SN74LS158DR, SN54LS158J), p.9 (SNJ54LS158J) — scanned, read from the page images; mux/sn74ahc158.pdf p.1 (pinouts, ordering), p.2 (function table, logic diagram), p.3 (ROC), p.5 (5 V timing); mux/sn74ahct158.pdf p.1 (pinouts, ordering), p.2 (function table, logic diagram), p.3 (ROC), p.4 (timing); mux/sn74als157a.pdf p.1 (J/D/N and FK pinouts, function table), p.2 (logic symbol), p.4 (ALS ROC), p.5 (ALS158 timing), p.6 (AS ROC), p.7 (AS158 timing), p.9–10 (addendum: SN74ALS158NSR, SN74AS158N); mux/sn74hc158.pdf p.1 (pinouts, ordering), p.2 (function table, logic diagram), p.3 (ROC), p.4 (timing); mux/sn74lvc158a.pdf p.3 (D/PW pinout, pin functions), p.4 (ROC), p.6 (timing), p.12 (function table)
- **Families:** AC (CD74AC158), ACT (CD74ACT158), HCT (CD74HCT158), LS (SN74LS158; SN54LS158J/SNJ54LS158J military), ALS (SN74ALS158), AS (SN74AS158), AHC (SN74AHC158), AHCT (SN74AHCT158), HC (SN74HC158), LVC (SN74LVC158A)
- **Pin names:** the classic SN names Ā/B (pin 1), 1A/1B…4A/4B, 1Y–4Y, Ḡ (pin 15) on cd74ac158 p.1, cd74act158 p.1, sn74ls157 p.1, sn74ahc158 p.1, sn74ahct158 p.1, sn74als157a p.1, sn74hc158 p.1, sn74lvc158a p.3 (all print Ḡ, "strobe"; none use OE̅). Outputs are printed plain 1Y–4Y on every SN/CD74AC(T) sheet (inversion only in the logic diagram bubbles: sn74ls157 p.3, sn74ahc158 p.2, sn74hc158 p.2, cd74ac158 p.2 text "provides inverted data"). cd74hc157 p.1 names the CD74HCT158 pins S, 1I0/1I1…4I0/4I1, 1Y̅–4Y̅, E̅ (I0 = A, I1 = B; same numbers). Checked pin by pin: same numbers on every sheet. Names `A/B` and `G` as in 74_258 (overbars dropped).
- **Pinouts:**

**p16** — N, D, PW, DB, NS, J (cd74ac158 p.1 and cd74act158 p.1 (M = SOIC, the tsv's D); cd74hc157 p.1 (CD74HCT158 PDIP = N); sn74ls157 p.1 (J, W, D, N; NS only in the addendum p.8, same 16-pin SOP); sn74ahc158 p.1 (D, DB, DGV, N, NS, PW, J, W); sn74ahct158 p.1 (same set); sn74als157a p.1 (J, D, N; NS only in addendum p.9); sn74hc158 p.1 (D, N, NS, PW, J, W); sn74lvc158a p.3 (D, PW))

| Pin | Name | Role |
|---|---|---|
| 1 | A/B | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Y | output |
| 5 | 2A | input |
| 6 | 2B | input |
| 7 | 2Y | output |
| 8 | GND | ground |
| 9 | 3Y | output |
| 10 | 3B | input |
| 11 | 3A | input |
| 12 | 4Y | output |
| 13 | 4B | input |
| 14 | 4A | input |
| 15 | G | input |
| 16 | VCC | power |

- **Function:** every sheet prints the same table (cd74ac158 p.1, cd74act158 p.1, cd74hc157 p.2 '158 column with E̅/S/I0/I1/Y̅, sn74ls157 p.1 'LS158 column, sn74ahc158 p.2, sn74ahct158 p.2, sn74als157a p.1 'ALS158/SN74AS158 column, sn74hc158 p.2, sn74lvc158a p.12):

  | G | A/B | A | B | Y |
  |---|---|---|---|---|
  | H | X | X | X | H |
  | L | L | L | X | H |
  | L | L | H | X | L |
  | L | H | X | L | H |
  | L | H | X | H | L |

  Model `select`, four units sharing `select` [A/B]: unit n `data` [nA, nB] (A/B low → A = data[0]), `y` nY, `strobe` {G, active low}; `disabled` "low" (default) with `invert` true, so G high → every Y high, matching row 1; G low → Y = NOT the chosen data.

- **Timing:** (main path data A/B inputs to Y; rise = tPLH, fall = tPHL)

- LS: rise 7 ns, fall 10 ns; vcc 4.75–5.25 V. Data to Y typ 'LS158, 5 V 15 pF (sn74ls157 p.5)
- ALS: rise 9 ns, fall 5 ns; vcc 4.5–5.5 V. A or B to Y typ 'ALS158, 5 V 50 pF (sn74als157a p.4, p.5)
- AS: rise 5 ns, fall 4.5 ns; vcc 4.5–5.5 V. A or B to Y max SN74AS158, 4.5–5.5 V 50 pF, no typ printed (sn74als157a p.6, p.7)
- HC: rise 13 ns, fall 13 ns; vcc 2–6 V. A or B to Y tpd typ, 4.5 V 50 pF (sn74hc158 p.3, p.4)
- HCT: rise 13 ns, fall 13 ns; vcc 4.5–5.5 V. Data to output typ HCT158, 5 V 15 pF (at 50 pF only max, 28 ns at 4.5 V) (cd74hc157 p.3, p.5)
- AC: rise 7.3 ns, fall 7.3 ns; vcc 1.5–5.5 V. A or B to any Y max, 5 V 50 pF, −40 to 85 °C column, no typ printed (cd74ac158 p.3, p.4)
- ACT: rise 8.4 ns, fall 8.4 ns; vcc 4.5–5.5 V. A or B to any Y max, 5 V 50 pF, −40 to 85 °C, no typ printed (cd74act158 p.3, p.4)
- AHC: rise 5.6 ns, fall 5.6 ns; vcc 2–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF (sn74ahc158 p.3, p.5)
- AHCT: rise 5.6 ns, fall 5.6 ns; vcc 4.5–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF (sn74ahct158 p.3, p.4)
- LVC: rise 3 ns, fall 3 ns; vcc 1.1–3.6 V, inputMax 5.5 V. A or B to Y tpd typ, 3.3 V 50 pF (sn74lvc158a p.4, p.6)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls157 p.5) — `sn54vcc` LS [4.5, 5.5] for SN54LS158J/SNJ54LS158J. ALS 4.5–5.5 V SN54 and SN74 (sn74als157a p.4); AS 4.5–5.5 V (p.6); HC 2–6 V SN54 and SN74 (sn74hc158 p.3); HCT 4.5–5.5 V (cd74hc157 p.3); AC 1.5–5.5 V (cd74ac158 p.3); ACT 4.5–5.5 V (cd74act158 p.3); AHC 2–5.5 V, VI 0–5.5 V (sn74ahc158 p.3); AHCT 4.5–5.5 V, VI 0–5.5 V (sn74ahct158 p.3); LVC 1.1–3.6 V operating (2024 sheet), VI to 5.5 V (sn74lvc158a p.4). AHC/AHCT/LVC get inputMax 5.5.
- **Checks against the model:** exact — G is the only control; per every function table G high forces all four Y high, which is `disabled` "low" plus `invert` (Y = NOT low = high). Timing basis notes: LS prints typ only at 15 pF; HCT prints typ only at 15 pF; AC, ACT and AS print only max. No FK/W/DGV parts are in the add rows, so no fk20 map is given (sn74ls157 p.1, sn74ahc158 p.1, sn74ahct158 p.1, sn74als157a p.1, sn74hc158 p.1 show FK).

## 74_251 — 1-of-8 data selectors/multiplexers with 3-state outputs

- **PDFs:** mux/sn74hc251.pdf p.3 (pinout J/D/DB/N/NS/PW, FK — read from the page image), p.4 (ROC), p.5 (timing), p.9 (function table); mux/cd74hc251.pdf p.3 (pinout J/N/D, image), p.4 (ROC HC and HCT), p.6 (timing), p.10 (truth table, image); mux/sn74hcs251.pdf p.3 (pinout D/PW, image), p.4 (ROC), p.5 (timing, image), p.11 (function table, image); mux/sn74hcs251-q1.pdf p.3, p.4, p.5, p.10; mux/cd74ac251.pdf p.1 (features: 1.5–5.5 V), p.3 (D pinout, image), p.4 (ROC, image), p.5 (timing, image), p.8 (truth table); mux/sn74ls251.pdf p.1 (J/W/N/D pinout, FK, function table), p.5 (LS ROC), p.6 (LS timing) — scanned, read from the page images; mux/sn74als251.pdf p.1 (J, D/N, FK, function table, image), p.3 (ROC), p.4 (timing); mux/sn74f251b.pdf p.1 (J, D/N, FK, function table, image), p.4 (ROC), p.5 (timing). (cd74hct251 is the same file as cd74hc251; sn54als251 is the same file as sn74als251.)
- **Families:** HC (SN74HC251, CD74HC251), HCT (CD74HCT251), HCS (SN74HCS251, SN74HCS251-Q1), AC (CD74AC251), LS (SN74LS251), ALS (SN74ALS251, SNJ54ALS251), F (SN74F251B).
- **Pin names:** the classic SN names D0–D7, A, B, C, G (overbar), Y, W (sn74ls251 p.1, sn74f251b p.1, where G is called "strobe"/"enable"). Other sheets print the same pins differently: sn74hc251 p.3, sn74als251 p.1 and sn74hcs251(-q1) p.3 call pin 7 OE̅ (output enable); cd74hc251 p.3 and cd74ac251 p.3 use I0–I7 (pins 4, 3, 2, 1, 15, 14, 13, 12 = D0–D7), S0/S1/S2 (pins 11/10/9 = A/B/C), Y (5), Ȳ (6 = W) and OE̅ (7 = G). Checked pin by pin: same numbers on every sheet; S0 = A = LSB (truth tables). The device uses G.
- **Pinouts:**

**p16** — N, D, DB, NS, PW, J (sn74hc251 p.3: J/D/DB/N/NS/PW; cd74hc251 p.3: J/N/D; sn74hcs251 p.3: D/PW; cd74ac251 p.3: D; sn74ls251 p.1: J/W, N, D; sn74als251 p.1: J (SN54ALS251) and D/N; sn74f251b p.1: J, D/N. NS for LS is only in the sn74ls251 addendum; same order as sn74hc251 p.3)

| Pin | Name | Role |
|---|---|---|
| 1 | D3 | input |
| 2 | D2 | input |
| 3 | D1 | input |
| 4 | D0 | input |
| 5 | Y | output |
| 6 | W | output |
| 7 | G | input |
| 8 | GND | ground |
| 9 | C | input |
| 10 | B | input |
| 11 | A | input |
| 12 | D7 | input |
| 13 | D6 | input |
| 14 | D5 | input |
| 15 | D4 | input |
| 16 | VCC | power |

  (No FK part is an add row; the FK pinout on sn74hc251 p.3 / sn74ls251 p.1 / sn74als251 p.1 / sn74f251b p.1 is the '151 fk20 order with G at pad 9.)

- **Function:** every family prints the same table (sn74ls251 p.1, sn74hc251 p.9, sn74als251 p.1, sn74f251b p.1, sn74hcs251 p.11, cd74hc251 p.10 and cd74ac251 p.8 with S/I/OE̅/Ȳ names):

  | C | B | A | G | Y | W |
  |---|---|---|---|---|---|
  | X | X | X | H | Z | Z |
  | L | L | L | L | D0 | D̅0 |
  | L | L | H | L | D1 | D̅1 |
  | L | H | L | L | D2 | D̅2 |
  | L | H | H | L | D3 | D̅3 |
  | H | L | L | L | D4 | D̅4 |
  | H | L | H | L | D5 | D̅5 |
  | H | H | L | L | D6 | D̅6 |
  | H | H | H | L | D7 | D̅7 |

  Model `select`, one unit: `data` D0..D7, `select` [A, B, C] (A = LSB), `y` Y, `w` W, `oe` {G, low} (G high: Y and W Z). No strobe, so `disabled` is not used.

- **Timing:** (main path data to Y; rise = tPLH, fall = tPHL)

- HC: rise 17 ns, fall 17 ns; vcc 2–6 V. Any D to W/Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc251 p.4, p.5)
- HCT: rise 35 ns, fall 35 ns; vcc 4.5–5.5 V. Data to outputs max, 4.5 V 50 pF, 25 C (typ 12 ns only at 5 V 15 pF); VCC 4.5-5.5 V (cd74hc251 p.4, p.6)
- HCS: rise 7 ns, fall 7 ns; vcc 2–6 V. Any D to W/Y typ, 4.5 V 50 pF; VCC 2-6 V (sn74hcs251 p.4, p.5)
- AC: rise 12.3 ns, fall 12.3 ns; vcc 1.5–5.5 V. Data to Y max, 5 V 50 pF, -40 to 85 C (cd74ac251 p.5); VCC 1.5-5.5 V (cd74ac251 p.1; the p.4 ROC misprints max as 1.5)
- LS: rise 17 ns, fall 18 ns; vcc 4.75–5.25 V. Any D to Y typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls251 p.5, p.6)
- ALS: rise 10 ns, fall 15 ns; vcc 4.5–5.5 V. Any D to Y max, 4.5-5.5 V 50 pF, SN74; VCC 4.5-5.5 V (sn74als251 p.3, p.4)
- F: rise 3.8 ns, fall 4.5 ns; vcc 4.5–5.5 V. Any D to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74f251b p.4, p.5)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls251 p.5) — `sn54vcc` LS [4.5, 5.5]. HC 2–6 V (SN54 same); HCT 4.5–5.5 V; HCS 2–6 V; ALS 4.5–5.5 V (SN54ALS same, so SNJ54ALS251J needs no entry); F 4.5–5.5 V; AC 1.5–5.5 V (cd74ac251 p.1 features; the p.4 ROC table prints "1.5 / 1.5"). No family has 5.5 V-tolerant inputs.
- **Checks against the model:** exact (G high → both outputs Z = `oe` low; G is an output enable, not a strobe: no state forces Y low). Sheet errata noted, not used: cd74ac251 p.3 pin-function TYPE column marks Y/Ȳ as inputs and OE̅/S0–S2/I4–I7 as outputs (the pinout and description are right); cd74ac251 p.5 input-load table says "S0, S1, S3"; sn74hcs251(-q1) p.3 captions the 16-pin drawing "14-Pin SOIC or TSSOP". The AC entry uses the table max (12.3 ns); cd74ac251 p.1 also states a generic "typical propagation delay 6 ns at 5 V, 50 pF" without a path.

## 74_253 — Dual 4-line to 1-line data selectors/multiplexers with 3-state outputs

- **PDFs:** mux/sn74hc253.pdf p.3 (pinouts J/D/DB/N/NS and FK), p.4 (ROC), p.5 (switching), p.8 (description), p.9 (function table); mux/sn74hc253-ep.pdf p.1 (D pinout), p.2 (function table), p.4 (ROC), p.5 (switching); mux/sn74hc253-q1.pdf p.3 (D pinout, pin functions), p.4 (ROC), p.5 (switching); mux/cd74hc253.pdf p.1 (pinout, CD names), p.2 (truth table), p.3 (supply), p.4–5 (switching); mux/sn74ls253.pdf p.1 (pinouts J/W/D/N and FK, function table), p.2 (logic diagram), p.4 (ROC + switching) — scanned, read from the page images; mux/sn74als253.pdf p.1 (pinouts J/D/N and FK, function table), p.3 (ALS ROC), p.4 (ALS switching, AS ROC), p.5 (AS switching); mux/sn74f253.pdf p.1 (pinouts J/D/N and FK, function table), p.4 (ROC), p.5 (switching); mux/cd74ac253.pdf p.1 (pinout, CD names), p.2 (truth table), p.3 (operating conditions), p.5 (switching)
- **Families:** HC (SN74HC253, SN74HC253-EP, SN74HC253-Q1, SNJ54HC253, CD74HC253), HCT (CD74HCT253), LS (SN74LS253), ALS (SN74ALS253, SNJ54ALS253), AS (SN74AS253A), F (SN74F253), AC (CD74AC253), ACT (CD74ACT253)
- **Pin names:** the classic SN names 1G, 2G (output controls, active low), A, B (selects, A = LSB), 1C0–1C3, 2C0–2C3, 1Y, 2Y — sn74ls253 and sn74f253 print G ("output control" / "strobe (G)"). The other SN sheets name the controls 1OE/2OE with everything else the same: sn74hc253, sn74hc253-ep, sn74hc253-q1, sn74als253 (1OE pin 1, 2OE pin 15). The CD sheets (cd74hc253, cd74ac253, both Harris) use other names, checked pin by pin against their p.1 pinouts and p.2 functional diagrams: 1OE = 1G (pin 1), S1 = B (pin 2), 1I3..1I0 = 1C3..1C0 (pins 3–6), 1Y (7), GND (8), 2Y (9), 2I0..2I3 = 2C0..2C3 (pins 10–13), S0 = A (pin 14), 2OE = 2G (pin 15), VCC (16). Their truth tables (p.2) select I1 with S1 S0 = L H, so S0 = A = LSB. G is kept (not OE) so the '253 uses the '153's names; the pins are the same.
- **Pinouts:**

**p16** — N, D, DB, NS, J. sn74hc253 p.3 (J, D, DB, N, NS); sn74hc253-ep p.1 (D); sn74hc253-q1 p.3 (D); sn74ls253 p.1 (J/W, D, N); sn74als253 p.1 (J, D, N); sn74f253 p.1 (J, D, N); cd74hc253 p.1 and cd74ac253 p.1 (PDIP, SOIC; CERDIP for CD54ACT253).

| Pin | Name | Role |
|---|---|---|
| 1 | 1G | input |
| 2 | B | input |
| 3 | 1C3 | input |
| 4 | 1C2 | input |
| 5 | 1C1 | input |
| 6 | 1C0 | input |
| 7 | 1Y | output |
| 8 | GND | ground |
| 9 | 2Y | output |
| 10 | 2C0 | input |
| 11 | 2C1 | input |
| 12 | 2C2 | input |
| 13 | 2C3 | input |
| 14 | A | input |
| 15 | 2G | input |
| 16 | VCC | power |

**fk20** — FK (SNJ54HC253FK; sn74hc253 p.3; identical on sn74ls253 p.1, sn74als253 p.1, sn74f253 p.1)

| Pin | Name | Role |
|---|---|---|
| 1 | NC | nc |
| 2 | 1G | input |
| 3 | B | input |
| 4 | 1C3 | input |
| 5 | 1C2 | input |
| 6 | NC | nc |
| 7 | 1C1 | input |
| 8 | 1C0 | input |
| 9 | 1Y | output |
| 10 | GND | ground |
| 11 | NC | nc |
| 12 | 2Y | output |
| 13 | 2C0 | input |
| 14 | 2C1 | input |
| 15 | 2C2 | input |
| 16 | NC | nc |
| 17 | 2C3 | input |
| 18 | A | input |
| 19 | 2G | input |
| 20 | VCC | power |

- **Function:** every sheet prints the same table (sn74hc253 p.9; sn74hc253-ep p.2; sn74ls253 p.1; sn74als253 p.1; sn74f253 p.1; cd74hc253 p.2 and cd74ac253 p.2 with S1/S0/I/OE names):

  | B | A | C0 | C1 | C2 | C3 | G (OE) | Y |
  |---|---|---|---|---|---|---|---|
  | X | X | X | X | X | X | H | Z |
  | L | L | L | X | X | X | L | L |
  | L | L | H | X | X | X | L | H |
  | L | H | X | L | X | X | L | L |
  | L | H | X | H | X | X | L | H |
  | H | L | X | X | L | X | L | L |
  | H | L | X | X | H | X | L | H |
  | H | H | X | X | X | L | L | L |
  | H | H | X | X | X | H | L | H |

  Select inputs A and B are common to both sections. Model `select`, two units: unit 1 `data` [1C0, 1C1, 1C2, 1C3], `select` [A, B] (A = LSB), `y` 1Y, `strobe` {1G, low}; unit 2 the same with 2C0..2C3, 2Y, 2G. `disabled` "z", `invert` false, push-pull, no `w`.

  **strobe + "z" rather than `oe`:** each section has exactly one control, and its only effect in every table is H → Y = Z, L → Y = selected data. Both encodings give that; the model's `oe` is documented as an output enable *separate from* the strobe ('251 G, CD4512B DISABLE), for parts that have both, and the docs name '253/'257 as the `"disabled": "z"` case. So the single per-section control is the strobe with `disabled` "z" (as 74_258/74_257 do). An X on G gives X either way.

- **Timing:** (main path data → Y; rise = tPLH, fall = tPHL)

- HC: rise 16 ns, fall 16 ns; vcc 2–6 V. Data (any C) to Y tpd typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc253 p.4, p.5; sn74hc253-ep p.5 and sn74hc253-q1 p.5 the same)
- HCT: rise 38 ns, fall 38 ns; vcc 4.5–5.5 V. Data to outputs max, 4.5 V 50 pF, 25 °C (cd74hc253 p.5; the only typ, 16, is at 5 V 15 pF); VCC 4.5-5.5 V (cd74hc253 p.3)
- LS: rise 17 ns, fall 13 ns; vcc 4.75–5.25 V. Data to Y typ, 5 V 15 pF, RL 2 kΩ (sn74ls253 p.4)
- ALS: rise 10 ns, fall 14 ns; vcc 4.5–5.5 V. Data (any C) to Y max SN74ALS253, 4.5-5.5 V 50 pF; no typ printed (sn74als253 p.3, p.4). SN54ALS253: 15 / 18.
- AS: rise 7.5 ns, fall 8 ns; vcc 4.5–5.5 V. Data (any C) to Y max SN74AS253A, 4.5-5.5 V 50 pF; no typ printed (sn74als253 p.4, p.5)
- F: rise 5.1 ns, fall 4.1 ns; vcc 4.5–5.5 V. Any C to any Y typ, 5 V 50 pF (sn74f253 p.4, p.5)
- AC: rise 13.3 ns, fall 13.3 ns; vcc 1.5–5.5 V. nI to Y max, 5 V 50 pF, −55 to 125 °C (cd74ac253 p.5; −40 to 85 °C 12.1). The "typical 6.3 ns at 5 V, 50 pF" of the p.1 feature list names no path, so the table max is used. VCC 1.5-5.5 V (cd74ac253 p.3)
- ACT: rise 18 ns, fall 18 ns; vcc 4.5–5.5 V. nI to Y max, 5 V 50 pF, −55 to 125 °C (cd74ac253 p.5; −40 to 85 °C 16.4); no typ. VCC 4.5-5.5 V (cd74ac253 p.3)

- **Supply:** HC 2–6 V for SN54HC/SN74HC (sn74hc253 p.4), the -EP and -Q1 parts (sn74hc253-ep p.4, sn74hc253-q1 p.4) and CD74HC (cd74hc253 p.3); HCT 4.5–5.5 V (cd74hc253 p.3); LS SN74 4.75–5.25 V (sn74ls253 p.4; SN54LS 4.5–5.5 V, but no SN54LS253 is added, so no `sn54vcc`); ALS 4.5–5.5 V for SN54 and SN74 (sn74als253 p.3); AS 4.5–5.5 V (p.4); F 4.5–5.5 V (sn74f253 p.4); AC 1.5–5.5 V, ACT 4.5–5.5 V (cd74ac253 p.3). No input tolerance above VCC.
- **Checks against the model:** exact (with the strobe-as-output-control encoding above). Not modeled: the separate select→Y and enable/disable (tPZx/tPxZ) delays.

## 74_257 — Quadruple 2-line to 1-line data selectors/multiplexers with 3-state outputs

- **PDFs:** mux/sn74hc257.pdf p.3 (pinout J/D/N/NS/PW, FK), p.4 (ROC), p.6 (timing), p.10 (function table); mux/cd74hc257.pdf p.1 (pinout CERDIP/PDIP/SOIC, ordering), p.2 (truth table), p.3 (supply HC/HCT); mux/sn74hct257.pdf p.3 (pinout J/N/D, pin table), p.4 (ROC), p.5 (timing), p.9 (function table); mux/sn74ls257b.pdf p.1 (pinout J/W/D/N, FK, function table), p.2 (logic diagram, pin numbers), p.4 (ROC, timing) — scanned, read from the page images; p.9 (addendum: NS); mux/sn74als257a.pdf p.1 (pinout J/D/N, FK, function table — page image), p.3 (ALS ROC), p.4 (ALS timing), p.5 (AS ROC), p.7 (AS timing), p.9 (addendum: NS for ALS and AS); mux/sn74f257.pdf p.1 (pinout J/D/N, FK, function table — page image), p.3 (ROC), p.4 (timing), p.5 (addendum: NS); mux/cd74ac257.pdf p.1 (typical delay, packages D/N/PW/BQB), p.3 (pinout J/D/N/PW, BQB, pin table), p.4 (ROC), p.6 (timing), p.10 (truth table); mux/sn74ac257-q1.pdf p.3 (PW, BQB), p.4 (ROC), p.6–7 (timing), p.14 (function table); mux/sn74act257-q1.pdf p.3 (PW, BQB), p.4 (ROC), p.5 (timing), p.11 (function table); mux/sn74ahc257.pdf p.3 (PW, BQB — page image), p.4 (ROC), p.5 (timing), p.11 (function table); mux/sn74ahc257-q1.pdf p.3 (PW, WBQB), p.4, p.5, p.11; mux/sn74ahct257.pdf p.3 (PW, BQB), p.4, p.5, p.9; mux/sn74ahct257-q1.pdf p.3 (PW, BQB), p.4, p.5, p.10; mux/sn74lvc257a.pdf p.3 (D/DB/NS/J/W/PW, BQB/RGY, FK — page image), p.4 (ROC), p.6 (timing), p.9 (function table); mux/sn74lvc257a-q1.pdf p.3 (D/PW, BQB, pin table), p.4 (ROC), p.5 (timing), p.8 (function table); mux/sn74lvc257a-ep.pdf p.1 (pinout D/PW), p.2 (function table), p.3 (ROC), p.4 (timing); mux/cy74fct257t.pdf p.1 (pinout Q/SO), p.2 (ordering, function table), p.3 (ROC), p.5 (timing), addendum (D, DW, DBQ).
- **Families:** HC (SN74HC257, SN54HC257, CD74HC257), HCT (SN74HCT257, CD74HCT257, CD54HCT257), LS (SN74LS257B), ALS (SN74ALS257A), AS (SN74AS257), F (SN74F257), AC (CD74AC257, SN74AC257-Q1), ACT (CD74ACT257, SN74ACT257-Q1), AHC (SN74AHC257, SN74AHC257-Q1), AHCT (SN74AHCT257, SN74AHCT257-Q1 / CAHCT257Q…Q1), LVC (SN74LVC257A, SN74LVC257A-Q1 / CLVC257AQ…Q1, SN74LVC257A-EP), FCT (CY74FCT257T, -AT, -CT speed grades).
- **Pin names:** device names are A/B (pin 1, Ā/B: low selects A), 1A, 1B, 1Y … 4A, 4B, 4Y, G (pin 15, active low), GND, VCC — the classic names of sn74hc257 p.3 and sn74ls257b p.1 (both print G̅). Other sheets, same pin numbers, checked pin by pin: sn74hct257 p.3, sn74als257a p.1, sn74f257 p.1, sn74lvc257a p.3, sn74lvc257a-q1, sn74lvc257a-ep, sn74ahc257/-q1, sn74ahct257/-q1, sn74ac257-q1, sn74act257-q1 call pin 15 O̅E̅ (sn74hct257's drawing says OE̅ while its pin table says G̅; its pin table I/O column is wrong — 1Y "I", 2A/2B "O" — a sheet typo, the descriptions are right). CD74HC/HCT257 and CD74AC/ACT257 (pin table) print S (1), 1I0 (2), 1I1 (3), 1Y (4), 2I0 (5), 2I1 (6), 2Y (7), 3Y (9), 3I1 (10), 3I0 (11), 4Y (12), 4I1 (13), 4I0 (14), OE̅ (15): I0 = A, I1 = B, S = A/B (truth tables: S low → I0); cd74ac257's BQB drawing p.3 uses A/B, 1A… instead. CY74FCT257T p.1 prints S (1), I0a (2), I1a (3), Ya (4), I0b (5), I1b (6), Yb (7), GND (8), Yd (9), I1d (10), I0d (11), Yc (12), I1c (13), I0c (14), OE̅ (15), VCC (16): FCT's channel c is SN's channel 4 (pins 12–14) and d is channel 3 (pins 9–11) — the letters swap, the pin functions per number are identical (each channel is identical and shares S and OE̅), so the same p16 pinout fits.
- **Pinouts:**

**p16** — N, D, DW, DB, DBQ, NS, PW, J, BQB, RGY (pin n = DIP pin n on every sheet: sn74hc257 p.3 (J/D/N/NS/PW), sn74lvc257a p.3 (D/DB/NS/PW; BQB/RGY: pins 1–16 in DIP order, thermal pad not a pin), sn74ahc257 p.3 page image and the -Q1 / AHCT / AC-Q1 / ACT-Q1 p.3 (BQB/WBQB: same, "pad can be connected to GND or left floating"), cd74ac257 p.3 (BQB), cd74hc257 p.1 (CERDIP J), cy74fct257t p.1 (Q = DBQ QSOP, SO; D and DW from the addendum), sn74ls257b/sn74als257a/sn74f257 p.1 (D/N; NS from the addenda)). The FK 20-pad pinout exists (sn74hc257 p.3, sn74lvc257a p.3) but no FK part is an add row, so it is not mapped.

| Pin | Name | Role |
|---|---|---|
| 1 | A/B | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Y | output |
| 5 | 2A | input |
| 6 | 2B | input |
| 7 | 2Y | output |
| 8 | GND | ground |
| 9 | 3Y | output |
| 10 | 3B | input |
| 11 | 3A | input |
| 12 | 4Y | output |
| 13 | 4B | input |
| 14 | 4A | input |
| 15 | G | input |
| 16 | VCC | power |

- **Function:** every sheet prints the same table (sn74hc257 p.10, sn74ls257b p.1, sn74als257a p.1, cd74ac257 p.10 with OE/S/I0/I1, cy74fct257t p.2 rows reordered):

  | G (OE) | A/B | A | B | Y |
  |---|---|---|---|---|
  | H | X | X | X | Z |
  | L | L | L | X | L |
  | L | L | H | X | H |
  | L | H | X | L | L |
  | L | H | X | H | H |

  Model `select`, four units: unit n `data` [nA, nB], `select` [A/B], `y` nY, `strobe` {G, low}; device `disabled` "z". G high forces every Y to Z (no "low" state exists), which is exactly the strobe with `disabled: "z"`; modelling G as `oe` instead would behave identically (OE deasserted → Z) — strobe + "z" is used to match the select fixture (fx_select.py) and the '257 example in docs § `mux`. No `w`, no `invert` ('258 is the inverting twin, not in this device).

- **Timing:** (main path data → Y; rise = tPLH, fall = tPHL)

- HC: rise 10 ns, fall 10 ns; vcc 2–6 V. A or B to Y typ, 4.5 V 50 pF (tpd; no tPLH/tPHL split); VCC 2-6 V (sn74hc257 p.4, p.6)
- HCT: rise 20 ns, fall 20 ns; vcc 4.5–5.5 V. A or B to Y typ, 4.5 V 50 pF (tpd); VCC 4.5-5.5 V (sn74hct257 p.4, p.5)
- LS: rise 8 ns, fall 10 ns; vcc 4.75–5.25 V. Data to Y typ, 5 V 45 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls257b p.4)
- ALS: rise 10 ns, fall 12 ns; vcc 4.5–5.5 V. A or B to Y max, SN74ALS257A, 4.5-5.5 V 50 pF (no typ printed) (sn74als257a p.3, p.4)
- AS: rise 5.5 ns, fall 6 ns; vcc 4.5–5.5 V. A or B to Y max, SN74AS257, 50 pF (no typ printed) (sn74als257a p.5, p.7)
- F: rise 4.1 ns, fall 3.8 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF (sn74f257 p.3, p.4)
- AC: rise 4.4 ns, fall 4.4 ns; vcc 1.5–5.5 V. Typical 4.4 ns at 5 V 50 pF from the cd74ac257 p.1 feature list; its table (p.6) prints only min/max (In to Y 8.5 ns max at 5 V) and sn74ac257-q1 p.7 also has no typ (8.2/7.7 ns max)
- ACT: rise 5.7 ns, fall 6 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF (sn74act257-q1 p.5; cd74ac257 p.6 prints max only, 9.7 ns); VCC 4.5-5.5 V (both sheets)
- AHC: rise 4.2 ns, fall 4.2 ns; vcc 2–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF; VI 0–5.5 V (sn74ahc257 p.4, p.5; -Q1 the same)
- AHCT: rise 4.9 ns, fall 4.9 ns; vcc 4.5–5.5 V, inputMax 5.5 V. A or B to Y typ, 5 V 50 pF; VI 0–5.5 V (sn74ahct257 p.4, p.5; -Q1 the same)
- LVC: rise 4.6 ns, fall 4.6 ns; vcc 1.65–3.6 V, inputMax 5.5 V. A or B to Y max, 3.3 V (no typ printed) (sn74lvc257a p.4, p.6; -Q1 p.5 and -EP p.4 the same 4.6 ns)
- FCT: rise 5 ns, fall 5 ns; vcc 4.75–5.25 V. I to Y max, 50 pF, CY74FCT257AT grade (no typ; T 6 ns, CT 4.3 ns) (cy74fct257t p.3, p.5)

- **Supply:** HC 2–6 V (SN54HC the same, sn74hc257 p.4; CD74HC p.3); HCT 4.5–5.5 V (SN54/CD54 the same); LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls257b p.4) — `sn54vcc` LS [4.5, 5.5] (no SN54LS add row; kept as plan-19 does); ALS, AS, F 4.5–5.5 V; AC 1.5–5.5 V (cd74ac257 p.4) — sn74ac257-q1 p.4 allows 1.5–6 V, so `partVcc` [1.5, 6] for SN74AC257PWRQ1 and SN74AC257WBQBRQ1; ACT 4.5–5.5 V; AHC 2–5.5 V and AHCT 4.5–5.5 V with VI 0–5.5 V (inputMax 5.5); LVC 1.65–3.6 V operating, VI 0–5.5 V (sn74lvc257a p.4) — the -Q1 sheet (p.4) and -EP sheet (p.3) specify 2–3.6 V, so `partVcc` [2, 3.6] for SN74LVC257ADRQ1, SN74LVC257AQDRG4Q1, CLVC257AQPWRG4Q1, SN74LVC257AMPWREP, SN74LVC257AQPWREP; FCT 4.75–5.25 V (cy74fct257t p.3).
- **Checks against the model:** exact (G/OE̅ high → all Y Z; else Y = A when A/B low, B when high). Timing basis notes: HC/HCT print only tpd (max of tPLH/tPHL), so rise = fall; ALS, AS, LVC and FCT print only max; AC typ comes from the CD74AC257 feature bullet, not a table. One FCT entry for three speed grades (AT chosen as in plan-19's 74_138; T parts slower, CT faster than modeled). LS ’257B load is 45 pF (sheet's standard).

## 74_258 — Quadruple 2-line to 1-line inverting data selectors/multiplexers with 3-state outputs

- **PDFs:** mux/sn74ls257b.pdf p.1 (J/W/D/N and FK pinouts, function table), p.2 (logic diagram), p.4 (LS ROC + timing), p.5 (S ROC + timing) — scanned, read from the page images; mux/sn74als257a.pdf p.1 (J/D/N and FK pinouts, function table), p.2 (logic diagram), p.4 (ALS ROC), p.5 (ALS258A timing, AS ROC), p.7 (AS258 timing); mux/sn74f258.pdf p.1 (J/D/N and FK pinouts, function table), p.3 (ROC), p.4 (timing); mux/sn74hc257.pdf p.3 (J/D/N/NS/PW and FK pinouts), p.4 (ROC), p.7 (HC258 timing), p.10 (function table, '258 block diagram); mux/cd74hct258.pdf p.1 (E/F pinout, ordering, function table), p.3 (ROC), p.4 (timing); mux/cd74ac257.pdf p.3 (D/N/PW/J and BQB pinouts), p.4 (ROC), p.6–7 (timing), p.10 (truth table); mux/sn74lvc258a.pdf p.3 (D/PW pinout), p.4 (ROC), p.6 (timing), p.13 (function table)
- **Families:** LS (SN74LS258B; SN54LS258B/SNJ54LS258B on the sheet, not add rows), S (SNJ54S258J only — military, on sn74ls257b), ALS (SN74ALS258A, SNJ54ALS258AJ), AS (SN74AS258), F (SN74F258), HC (SN74HC258), HCT (CD74HCT258), ACT (CD74ACT258), LVC (SN74LVC258A)
- **Pin names:** the classic SN names Ā/B (select, pin 1), 1A/1B…4A/4B, 1Y–4Y, Ḡ (pin 15) — sn74ls257b p.1 and sn74hc257 p.3 and cd74hct258 p.1 print Ḡ; sn74als257a p.1, sn74f258 p.1, sn74lvc258a p.3 print the same pin 15 as OE̅. The outputs are printed plain 1Y–4Y on every SN sheet and on cd74hct258 (the inversion shown only by the bubble in the logic diagrams, sn74ls257b p.2, sn74als257a p.2, sn74hc257 p.10). cd74ac257 p.3 names the CD74ACT258 pins S, 1I0/1I1…4I0/4I1, 1Y̅–4Y̅, OE̅ (I0 = A, I1 = B; same numbers). Checked pin by pin: same numbers on every sheet. The device uses `A/B` and `G` as pin names (the overbars are dropped as in the rest of the library).
- **Pinouts:**

**p16** — N, D, PW, J (sn74ls257b p.1 (J, W, D, N), sn74als257a p.1 (J, D, N), sn74f258 p.1 (J, D, N), sn74hc257 p.3 (J, D, N, NS, PW), cd74hct258 p.1 (E = PDIP, the tsv's N), cd74ac257 p.3 (D, N, PW), sn74lvc258a p.3 (D, PW))

| Pin | Name | Role |
|---|---|---|
| 1 | A/B | input |
| 2 | 1A | input |
| 3 | 1B | input |
| 4 | 1Y | output |
| 5 | 2A | input |
| 6 | 2B | input |
| 7 | 2Y | output |
| 8 | GND | ground |
| 9 | 3Y | output |
| 10 | 3B | input |
| 11 | 3A | input |
| 12 | 4Y | output |
| 13 | 4B | input |
| 14 | 4A | input |
| 15 | G | input |
| 16 | VCC | power |

- **Function:** every sheet prints the same table (sn74ls257b p.1 'LS258B/'S258 column, sn74als257a p.1, sn74f258 p.1, sn74hc257 p.10, cd74hct258 p.1, cd74ac257 p.10 with OE̅/S/I0/I1/Y̅, sn74lvc258a p.13):

  | G (OE) | A/B | A | B | Y |
  |---|---|---|---|---|
  | H | X | X | X | Z |
  | L | L | L | X | H |
  | L | L | H | X | L |
  | L | H | X | L | H |
  | L | H | X | H | L |

  Model `select`, four units sharing `select` [A/B]: unit n `data` [nA, nB] (A/B low → A = data[0]), `y` nY, `strobe` {G, active low}; `disabled` "z" (G high → all Y Z), `invert` true (Y = NOT the chosen data).

- **Timing:** (main path data A/B inputs to Y; rise = tPLH, fall = tPHL)

- LS: rise 7 ns, fall 11 ns; vcc 4.75–5.25 V. A or B to Y typ, 5 V 45 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls257b p.4)
- S: rise 4 ns, fall 4 ns; vcc 4.75–5.25 V. A or B to Y typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls257b p.5)
- ALS: rise 8 ns, fall 7 ns; vcc 4.5–5.5 V. A or B to Y max SN74ALS258A, 4.5-5.5 V 50 pF (no typ printed); VCC 4.5-5.5 V (sn74als257a p.4, p.5)
- AS: rise 5 ns, fall 4 ns; vcc 4.5–5.5 V. A or B to Y max SN74AS258, 4.5-5.5 V 50 pF (no typ printed); VCC 4.5-5.5 V (sn74als257a p.5, p.7)
- F: rise 3.6 ns, fall 3.1 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74f258 p.3, p.4)
- HC: rise 13 ns, fall 13 ns; vcc 2–6 V. A or B to Y tpd typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc257 p.4, p.7)
- HCT: rise 11 ns, fall 11 ns; vcc 4.5–5.5 V. A or B to Y tpd typ, 5 V 15 pF (at 50 pF only max, 27 ns at 4.5 V); VCC 4.5-5.5 V (cd74hct258 p.3, p.4)
- ACT: rise 8.5 ns, fall 8.5 ns; vcc 4.5–5.5 V. In to Y̅ tPLH/tPHL max, 5 V 50 pF, −40 to 85 °C (no typ printed); VCC 4.5-5.5 V (cd74ac257 p.4, p.7)
- LVC: rise 3 ns, fall 3 ns; vcc 1.1–3.6 V, inputMax 5.5 V. A or B to Y tpd typ, 3.3 V 50 pF; VCC 1.1-3.6 V, inputs to 5.5 V (sn74lvc258a p.4, p.6)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (sn74ls257b p.4); S SN74 4.75–5.25 V, SN54S 4.5–5.5 V (p.5) — `sn54vcc` LS and S [4.5, 5.5] (the only S add row, SNJ54S258J, is military, so it takes the SN54 range). ALS 4.5–5.5 V for SN54 and SN74 (sn74als257a p.4); AS 4.5–5.5 V (p.5); F 4.5–5.5 V SN54 and SN74 (sn74f258 p.3); HC 2–6 V SN54 and SN74 (sn74hc257 p.4); HCT 4.5–5.5 V (cd74hct258 p.3); ACT 4.5–5.5 V (cd74ac257 p.4); LVC 1.1–3.6 V operating (the 2024 sheet's ROC, not the usual 1.65 V), VI 0–5.5 V at any VCC (inputMax 5.5) (sn74lvc258a p.4).
- **Checks against the model:** exact — G (OE̅) is the only control and, per every function table, it puts all four outputs in Z, so it is expressed as each unit's strobe with `disabled` "z" (the same as the '257). Timing basis notes: LS and S print data-to-output typicals at 45 pF and 15 pF; ALS, AS and ACT print only max (ACT: −40 to 85 °C column; the 4.4 ns typical on cd74ac257 p.1 is for the '257 parts generally, not a '258 data path figure); HCT prints a typical only at 15 pF; LVC's 25 °C column gives typ 3 / max 5 ns at 3.3 V 50 pF. No FK/W/NS/BQB parts are in the add rows, so no fk20 map is given (sn74ls257b p.1, sn74als257a p.1, sn74f258 p.1 and sn74hc257 p.3 show FK; cd74ac257 p.3 shows BQB in DIP order).

## 74_2G157 — Single 2-line to 1-line data selector multiplexer

- **PDFs:** mux/sn74lvc2g157.pdf (SCES207N) p.3 (DCT, DCU and YZP pinouts, pin functions — read from the page image for the overbars), p.4 (ROC), p.6 (switching), p.7 (load circuit: 3.3 V row 50 pF), p.9 (Table 1, function table — page image)
- **Families:** LVC (SN74LVC2G157). The AHC rows (SN74AHC2G157H…) are skipped in parts.tsv (no TI datasheet).
- **Pin names:** the sheet prints A, B, A̅/B (select), G̅ (strobe), Y, Y̅ (inverted output), GND, VCC. Here: `A`, `B`, `A/B`, `G`, `Y`, `YBAR` (the overbar of Y̅ written as BAR, as QBAR in dff.json; the A̅/B and G̅ bars dropped, as the classic '157 names).
- **Pinouts:**

**p8** — DCT, DCU (p.3: DCT 8-pin SSOP and DCU 8-pin VSSOP, same order)

| Pin | Name | Role |
|---|---|---|
| 1 | A | input |
| 2 | B | input |
| 3 | YBAR | output |
| 4 | GND | ground |
| 5 | Y | output |
| 6 | A/B | input |
| 7 | G | input |
| 8 | VCC | power |

**dsbga** — YZP (p.3 bottom view and the pin-functions table DSBGA column)

| Ball | Name | Role |
|---|---|---|
| A1 | A | input |
| B1 | B | input |
| C1 | YBAR | output |
| D1 | GND | ground |
| D2 | Y | output |
| C2 | A/B | input |
| B2 | G | input |
| A2 | VCC | power |

- **Function:** Table 1 (p.9):

  | G̅ | A̅/B | A | B | Y | Y̅ |
  |---|---|---|---|---|---|
  | H | X | X | X | L | H |
  | L | L | L | X | L | H |
  | L | L | H | X | H | L |
  | L | H | X | L | L | H |
  | L | H | X | H | H | L |

  Model `select`, one unit: `data` [A, B] (A/B low → A), `select` [A/B], `y` Y, `w` YBAR, `strobe` {G, low}. `disabled` default "low" (G̅ high → Y low, Y̅ high, as the table's first row), no `invert`.

- **Timing:**

- LVC: rise 6 ns, fall 6 ns; vcc 1.65–5.5 V, inputMax 5.5 V. A or B to Y max, 3.3 V 50 pF; VCC 1.65-5.5 V, inputs to 5.5 V (sn74lvc2g157 p.4, p.6, p.7)

- **Supply:** ROC VCC 1.65–5.5 V operating (1.5 V data retention only), VI 0–5.5 V at any VCC (p.4) → inputMax 5.5.
- **Checks against the model:** exact. Only max tpd printed (one tpd per path for Y or Y̅, no tPLH/tPHL split), so rise = fall = 6 ns. A/B → Y and G → Y are also 6 ns max at 3.3 V.

## 74_1T157 — Low power, 1.8/2.5/3.3-V input, 3.3-V CMOS output, single 2-input Schmitt-trigger buffer multiplexer (noninverted)

- **PDFs:** mux/sn74aup1t157.pdf (SCES807) p.1 (DCK pinout), p.2 (logic symbol, function table, input-threshold tables), p.5 (ROC), p.6–8 (switching, one table per VCC/VI pair)
- **Families:** AUP (SN74AUP1T157)
- **Pin names:** A, B (data), C (select), Y, GND, VCC (p.1, p.2; no overbars). Not a "configurable" gate: the sheet presents it as a plain 2-to-1 selector.
- **Pinouts:**

**p6** — DCK (p.1, 6-pin SC-70 top view; p.2 repeats it)

| Pin | Name | Role |
|---|---|---|
| 1 | A | input |
| 2 | GND | ground |
| 3 | B | input |
| 4 | C | input |
| 5 | VCC | power |
| 6 | Y | output |

- **Function:** function table (p.2), "When C is L, Y = B; when C is H, Y = A":

  | A | B | C | Y |
  |---|---|---|---|
  | X | L | L | L |
  | X | H | L | H |
  | L | X | H | L |
  | H | X | H | H |

  Model `select`, one unit: `data` [B, A] (note the order: C low → B, so data[0] = B), `select` [C], `y` Y; no strobe, no W, no invert.

- **Timing:**

- AUP: rise 3.8 ns, fall 3.8 ns; vcc 2.3–3.6 V. A, B or C to Y typ, VCC 3.3 V, VI 3.3 V, 30 pF (largest load printed); VCC 2.3-3.6 V, VI 0-3.6 V at any VCC, so no inputMax (sn74aup1t157 p.5, p.8)

- **Supply:** ROC VCC 2.3–3.6 V, VI 0–3.6 V (p.5). The inputs are Schmitt triggers with low thresholds (VT+ max 1.1 V at 2.3–2.7 V, 1.19 V at 3–3.6 V; p.2, p.6) so they accept 1.8/2.5/3.3 V signals; at VCC 2.5 V an input may take 3.3 V (down-translation, p.1), but 3.6 V is also the VCC max, so `inputMax` would equal the vcc max and is omitted. Other switching tables (p.6–8): VCC 2.5 V with VI 1.8/2.5/3.3 V give 4.4/4.4/4.7 ns typ at 30 pF; VCC 3.3 V with VI 1.8/2.5 V give 3.9/3.8 ns.
- **Checks against the model:** exact logic. Not modelled: Schmitt hysteresis and the level-translating low input thresholds (the simulator's inputs are logic levels). tpd only (no tPLH/tPHL split), so rise = fall.

## 74_1T158 — Low power, 1.8/2.5/3.3-V input, 3.3-V CMOS output, single 2-input Schmitt-trigger buffer multiplexer (inverted)

- **PDFs:** mux/sn74aup1t158.pdf (SCES808) p.1 (DCK pinout), p.2 (logic symbol with inverting bubble, function table — page image), p.5 (ROC), p.6–8 (switching). Text is otherwise the same as sn74aup1t157 (diffed; all electrical and switching tables identical).
- **Families:** AUP (SN74AUP1T158)
- **Pin names:** A, B (data), C (select), Y, GND, VCC (p.1, p.2). The output is printed Y with no overbar; figure 1 reads "When C is L, Y = B̅; when C is H, Y = A̅".
- **Pinouts:**

**p6** — DCK (p.1, 6-pin SC-70 top view; p.2 repeats it)

| Pin | Name | Role |
|---|---|---|
| 1 | A | input |
| 2 | GND | ground |
| 3 | B | input |
| 4 | C | input |
| 5 | VCC | power |
| 6 | Y | output |

- **Function:** function table (p.2):

  | A | B | C | Y |
  |---|---|---|---|
  | X | L | L | H |
  | X | H | L | L |
  | L | X | H | H |
  | H | X | H | L |

  Model `select`, one unit: `data` [B, A] (C low → B), `select` [C], `y` Y; `invert` true; no strobe, no W.

- **Timing:**

- AUP: rise 3.8 ns, fall 3.8 ns; vcc 2.3–3.6 V. A, B or C to Y typ, VCC 3.3 V, VI 3.3 V, 30 pF (largest load printed); VCC 2.3-3.6 V, VI 0-3.6 V at any VCC, so no inputMax (sn74aup1t158 p.5, p.8)

- **Supply:** ROC VCC 2.3–3.6 V, VI 0–3.6 V (p.5); Schmitt inputs with low thresholds for 1.8/2.5/3.3 V signals, as 74_1T157. inputMax would equal the vcc max, so omitted.
- **Checks against the model:** exact logic. Schmitt hysteresis and level-translating thresholds not modelled; tpd only, rise = fall.

## 74_11257 — Quadruple 2-line to 1-line data selector/multiplexer with 3-state outputs

- **PDFs:** mux/74ac11257.pdf p.1 (pinout DB/DW/N/PW, ordering, function table — page image), p.2 (logic diagram, pin numbers), p.3 (ROC), p.4 (timing), p.6 (addendum); mux/74act11257.pdf p.1 (pinout DB/DW/N, function table — page image), p.3 (ROC), p.4 (timing), p.6 (addendum: 74ACT11257DWR, SOIC DW 20).
- **Families:** AC (74AC11257), ACT (74ACT11257).
- **Pin names:** both sheets print the same names, checked pin by pin on the p.1 images: Ā/B (1), 1Y (2), 2Y (3), GND (4–7), 3Y (8), 4Y (9), O̅E̅ (10), 4B (11), 4A (12), 3B (13), 3A (14), VCC (15, 16), 2B (17), 2A (18), 1B (19), 1A (20). The logic diagram (74ac11257 p.2) confirms 1A/1B = 20/19 → 1Y 2, 2A/2B = 18/17 → 2Y 3, 3A/3B = 14/13 → 3Y 8, 4A/4B = 12/11 → 4Y 9, OE 10, A/B 1. Device names keep OE (not the 74_257 G) because these sheets only print OE.
- **Pinouts:**

**p20** — N, DW, PW (74ac11257 p.1 "DB, DW, N, OR PW PACKAGE"; 74act11257 p.1 "DB, DW, OR N PACKAGE"; flow-through, center-pin power: four GND pins 4–7, two VCC pins 15–16)

| Pin | Name | Role |
|---|---|---|
| 1 | A/B | input |
| 2 | 1Y | output |
| 3 | 2Y | output |
| 4 | GND | ground |
| 5 | GND | ground |
| 6 | GND | ground |
| 7 | GND | ground |
| 8 | 3Y | output |
| 9 | 4Y | output |
| 10 | OE | input |
| 11 | 4B | input |
| 12 | 4A | input |
| 13 | 3B | input |
| 14 | 3A | input |
| 15 | VCC | power |
| 16 | VCC | power |
| 17 | 2B | input |
| 18 | 2A | input |
| 19 | 1B | input |
| 20 | 1A | input |

- **Function:** both sheets p.1:

  | OE | A/B | A | B | Y |
  |---|---|---|---|---|
  | H | X | X | X | Z |
  | L | L | L | X | L |
  | L | L | H | X | H |
  | L | H | X | L | L |
  | L | H | X | H | H |

  Model `select`, four units: unit n `data` [nA, nB], `select` [A/B], `y` nY, `strobe` {OE, low}; device `disabled` "z" (same as 74_257: OE high → all Y Z; strobe + "z" and `oe` behave identically).

- **Timing:** (main path data → Y; rise = tPLH, fall = tPHL)

- AC: rise 3.6 ns, fall 4.1 ns; vcc 3–5.5 V. A or B to Y typ, 5 V 50 pF; VCC 3-5.5 V (74ac11257 p.3, p.4)
- ACT: rise 4.4 ns, fall 5 ns; vcc 4.5–5.5 V. A or B to Y typ, 5 V 50 pF; VCC 4.5-5.5 V (74act11257 p.3, p.4)

- **Supply:** 74AC11257 3–5.5 V (p.3; not the 1.5/2 V low end of CD74AC), VI 0–VCC; 74ACT11257 4.5–5.5 V (p.3). Both −40 to 85 °C only, no military parts.
- **Checks against the model:** exact. The multiple GND and VCC pins are separate pin entries with the same name; check_device.py loads them without complaint. No DB part is an add row (74AC11257DBR not in parts.tsv; 74ACT11257DBR obsolete), so DB is not mapped.

## 4512B — CMOS 8-channel data selector

- **PDFs:** mux/cd4512b.pdf p.1 (terminal assignment, functional diagram, logic diagram Fig. 1, truth table, ROC), p.3 (dynamic electrical characteristics) — scanned Harris sheet, read from the page images (terminal assignment re-rendered at 300 dpi); package addendum (orderables N, D, NS, PW).
- **Families:** C (CD4512B).
- **Pin names:** the sheet prints D0–D7, A, B, C ("SEL. CONT."; A = LSB per the truth table), INHIBIT, 3-STATE DISABLE, SEL. OUTPUT ("SELECT OUTPUT" in the functional diagram), VDD, VSS. The device uses INHIBIT, DISABLE (for "3-STATE DISABLE") and OUTPUT (for "SEL. OUTPUT"); docs/parts-format.md § mux already names the pin DISABLE.
- **Pinouts:**

**p16** — N, D, NS, PW (p.1 terminal assignment, one 16-lead drawing; p.1 text lists E (N), M/M96/MT/NSR (D, NS) and PW/PWR suffixes)

| Pin | Name | Role |
|---|---|---|
| 1 | D0 | input |
| 2 | D1 | input |
| 3 | D2 | input |
| 4 | D3 | input |
| 5 | D4 | input |
| 6 | D5 | input |
| 7 | D6 | input |
| 8 | VSS | ground |
| 9 | D7 | input |
| 10 | INHIBIT | input |
| 11 | A | input |
| 12 | B | input |
| 13 | C | input |
| 14 | OUTPUT | output |
| 15 | DISABLE | input |
| 16 | VDD | power |

  Cross-checked against the functional diagram (inhibit 10, 3-state disable 15, D0–D6 1–7, D7 9, A/B/C 11/12/13, select output 14, VDD 16, VSS 8) and the logic diagram pin callouts (p.1).

- **Function:** truth table (p.1):

  | A | B | C | INH | 3-STATE DISABLE | SEL OUTPUT |
  |---|---|---|---|---|---|
  | 0 | 0 | 0 | 0 | 0 | D0 |
  | 1 | 0 | 0 | 0 | 0 | D1 |
  | 0 | 1 | 0 | 0 | 0 | D2 |
  | 1 | 1 | 0 | 0 | 0 | D3 |
  | 0 | 0 | 1 | 0 | 0 | D4 |
  | 1 | 0 | 1 | 0 | 0 | D5 |
  | 0 | 1 | 1 | 0 | 0 | D6 |
  | 1 | 1 | 1 | 0 | 0 | D7 |
  | X | X | X | 1 | 0 | 0 |
  | X | X | X | X | 1 | High Z |

  Model `select`, one unit: `data` D0..D7, `select` [A, B, C] (A = LSB), `y` OUTPUT (no complement output), `strobe` {INHIBIT, low} (INHIBIT high forces the output low = `disabled` "low"), `oe` {DISABLE, low} (DISABLE high → Z, over INHIBIT, as the model's OE-first rule).

- **Timing:** (main path data to output; rise = tPLH, fall = tPHL)

- C: rise 180 ns, fall 180 ns; vcc 3–18 V. Data to Output typ, 5 V 50 pF; VDD 3-18 V (cd4512b p.1, p.3)

- **Supply:** VDD 3–18 V over the full package temperature range (p.1 ROC); one range for all parts (CD4512B is -55 to 125 C, no CD54 split).
- **Checks against the model:** exact. The sheet gives one combined tPHL/tPLH figure per path (Data to Output typ 180 ns, max 360 ns at 5 V; Inhibit to Output 140 ns; "A" select 200 ns), so rise = fall.

## 40257B — Quad 2-line-to-1-line data selector/multiplexer

- **PDFs:** mux/cd40257b.pdf (scanned Harris sheet, read from the page images) p.1 (features, functional diagram with pin numbers, Fig. 1 logic diagram, truth table, recommended operating conditions), p.2 (dynamic electrical characteristics), p.3 (terminal assignment); package option addendum (CD40257BE PDIP (N), CD40257BF3A CDIP (J), CD40257BM SOIC (D), all Active).
- **Families:** C (CD40257B; the F3A ceramic part is the same die and the same 3–18 V ROC).
- **Pin names:** the sheet prints INPUT SELECT, A1–A4, B1–B4, outputs D1–D4, OUTPUT DISABLE, VDD, VSS. Kept as `SELECT`, `A1`…`A4`, `B1`…`B4`, `D1`…`D4`, `DISABLE`, `VDD`, `VSS`. Checked pin by pin against the functional diagram (p.1: A1-2, B1-3, A2-5, B2-6, A3-11, B3-10, A4-14, B4-13, D1-4, D2-7, D3-9, D4-12, OUTPUT DISABLE 15, INPUT SELECT 1, VDD 16, VSS 8) and the terminal assignment (p.3). No overbars on any pin.
- **Pinouts:**

**p16** — N, J, D (the one terminal assignment, p.3; the sheet covers E/F3A/M packages with the same 16-lead order)

| Pin | Name | Role |
|---|---|---|
| 1 | SELECT | input |
| 2 | A1 | input |
| 3 | B1 | input |
| 4 | D1 | output |
| 5 | A2 | input |
| 6 | B2 | input |
| 7 | D2 | output |
| 8 | VSS | ground |
| 9 | D3 | output |
| 10 | B3 | input |
| 11 | A3 | input |
| 12 | D4 | output |
| 13 | B4 | input |
| 14 | A4 | input |
| 15 | DISABLE | input |
| 16 | VDD | power |

- **Function** (truth table, p.1; 1 = high, 0 = low, X = don't care, Z = high impedance):

  | 3-STATE OUTPUT DISABLE | SELECT | A | B | D |
  |---|---|---|---|---|
  | 1 | X | X | X | Z |
  | 0 | 0 | 0 | X | 0 |
  | 0 | 0 | 1 | X | 1 |
  | 0 | 1 | X | 0 | 0 |
  | 0 | 1 | X | 1 | 1 |

  One select input shared by all four sections (Fig. 1 shows a single INPUT SELECT, pin 1) and one shared OUTPUT DISABLE (pin 15). Model `select`, four units: unit n `data` [An, Bn], `select` [SELECT] (low → A), `y` Dn, `strobe` {DISABLE, active low}; device `disabled` "z"; `invert` false (outputs true, non-inverting). The brief's "two select inputs" does not apply: A and B are the data inputs, not selects.
- **Timing:** C: rise 150 ns, fall 150 ns; vcc 3–18 V. Data input to output tPHL, tPLH typ 150 ns (300 max) at VDD 5 V, CL 50 pF, RL 200 kΩ (p.2). (Select to output 190 ns typ; output disable to output 95 ns typ.)
- **Supply:** VDD 3–18 V (recommended operating conditions, p.1). Inputs −0.5 V to VDD + 0.5 V (absolute max), so no inputMax.
- **Checks against the model:** exact.

## 74_2257 — Quad 2-input multiplexer with 3-state outputs

- **PDFs:** mux/cy74fct2257t.pdf p.1 (Q/SO pinout, description, pin description), p.2 (ordering information, function table, logic diagram with pin numbers), p.3 (recommended operating conditions), p.5 (switching characteristics), p.6 (load circuit, CL 50 pF).
- **Families:** FCT (CY74FCT2257T, AT and CT speed grades; the add row CY74FCT2257ATQCT is the AT grade in QSOP).
- **Pin names:** the sheet's own names, kept: S, I0a–I0d, I1a–I1d, Ya–Yd, OE̅ (stored as `OE`), VCC, GND (p.1 pinout, checked pin by pin against the p.2 logic diagram numbers). It is the FCT257T function with 25 Ω series output resistors ("can replace the FCT257T", p.1); outputs are non-inverting (p.1 description; p.2 table). Classic SN '257 names would be A/B (select), 1A/1B…, 1Y…, G̅/OE̅.
- **Pinouts:**

**p16** — DBQ (the sheet's "Q" QSOP, p.1 "Q OR SO PACKAGE"; ordering table p.2)

| Pin | Name | Role |
|---|---|---|
| 1 | S | input |
| 2 | I0a | input |
| 3 | I1a | input |
| 4 | Ya | output |
| 5 | I0b | input |
| 6 | I1b | input |
| 7 | Yb | output |
| 8 | GND | ground |
| 9 | Yd | output |
| 10 | I1d | input |
| 11 | I0d | input |
| 12 | Yc | output |
| 13 | I1c | input |
| 14 | I0c | input |
| 15 | OE | input |
| 16 | VCC | power |

- **Function** (function table, p.2):

  | OE̅ | S | I0 | I1 | Y |
  |---|---|---|---|---|
  | H | X | X | X | Z |
  | L | H | X | L | L |
  | L | H | X | H | H |
  | L | L | L | X | L |
  | L | L | H | X | H |

  Model `select`, four units (a–d): `data` [I0x, I1x], `select` [S] (low → I0), `y` Yx, `strobe` {OE, active low}; device `disabled` "z"; `invert` false.
- **Timing:** FCT: rise 5 ns, fall 5 ns; vcc 4.75–5.25 V. Ia/Ib to Y tPLH/tPHL max, AT grade (the add row), CL 50 pF; no typical printed (CT grade 4.7 ns max) (p.5, p.6).
- **Supply:** VCC 4.75–5.25 V (recommended operating conditions, p.3; commercial −40 to 85 °C only, no military part). DC input range −0.5 V to 7 V is only an absolute-max rating, so no inputMax.
- **Checks against the model:** exact. The 25 Ω series output resistors (edge-rate control) are not modelled.

## 74_298 — Quadruple 2-input multiplexers with storage

- **PDFs:** mux/sn74as298a.pdf (SDAS219B; mux/sdas219b.pdf is the same file) p.1 (D/N pinout, description, function table), p.2 (logic symbol, logic diagram), p.3 (ROC, timing requirements), p.4 (switching), p.6 (load circuit 50 pF), addendum (ordering: SN74AS298AD, SN74AS298AN); mux/sn74ls298.pdf (SDLS098; mux/sdls098.pdf is the same file) — scanned, read from the page images: p.1 (J/W, N, D/N pinout, FK pinout, description, function table), p.2 (logic symbol, logic diagram), p.4 (SN54LS298/SN74LS298 ROC + switching), addendum (SN74LS298N only).
- **Families:** AS (SN74AS298A: D, N), LS (SN74LS298: N). SDLS098 also covers SN54298/SN74298 (TTL, p.3) and SN54LS298 (not in the add rows).
- **Pin names:** both sheets print the same names pin for pin: A1, A2, B1, B2, C1, C2, D1, D2, WS (word select), CLK, QA–QD (sn74as298a p.1, sn74ls298 p.1; logic symbols p.2 agree). No overbars on any pin.
- **Pinouts:**

**p16** — N, D (sn74as298a p.1 "D or N package"; sn74ls298 p.1 "SN74LS298 … D or N package")

| Pin | Name | Role |
|---|---|---|
| 1 | B2 | input |
| 2 | A2 | input |
| 3 | A1 | input |
| 4 | B1 | input |
| 5 | C2 | input |
| 6 | D2 | input |
| 7 | D1 | input |
| 8 | GND | ground |
| 9 | C1 | input |
| 10 | WS | input |
| 11 | CLK | input |
| 12 | QD | output |
| 13 | QC | output |
| 14 | QB | output |
| 15 | QA | output |
| 16 | VCC | power |

- **Function:** (sn74as298a p.1; sn74ls298 p.1 prints the same with "word select" / "clock")

  | WS | CLK | QA | QB | QC | QD |
  |---|---|---|---|---|---|
  | L | ↓ | a1 | b1 | c1 | d1 |
  | H | ↓ | a2 | b2 | c2 | d2 |
  | X | H | QA0 | QB0 | QC0 | QD0 |

  Both descriptions: WS low applies word 1 (A1, B1, C1, D1) to the flip-flops, WS high word 2; "the selected word is clocked to the output terminals on the negative-going edge of the clock pulse". The LS sheet agrees: function table ↓, and the p.2 logic diagram drives every flip-flop's CK through an inverting bubble (falling edge). AS setup/hold are "before/after CLK↓" (p.3).

  Model `stored`, four units {data [A1, A2], select [WS], y QA}, … {data [D1, D2], select [WS], y QD}; `outputClock` {CLK, falling}. No strobe, no OE.

- **Timing:** (main path CLK → Q; rise = tPLH, fall = tPHL)

- AS: rise 9 ns, fall 11 ns; vcc 4.5–5.5 V. CLK to Q max (no typical printed), 4.5-5.5 V 50 pF; VCC 4.5-5.5 V (sn74as298a p.3, p.4)
- LS: rise 18 ns, fall 21 ns; vcc 4.75–5.25 V. CLK to Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V (sn74ls298 p.4)

- **Supply:** AS 4.5–5.5 V (SN74AS298A only, p.3). LS SN74 4.75–5.25 V; SN54LS 4.5–5.5 V (p.4) but no SN54 part is an add row, so no `sn54vcc`.
- **Checks against the model:** exact (a '157-style 2:1 select feeding four falling-edge D flip-flops; no reset or enable). The LS switching table gives one tPLH/tPHL pair without FROM/TO columns; the only path is clock → output.

## 74_399 — Quadruple 2-input multiplexers with storage

- **PDFs:** mux/sn74ls399.pdf (SDLS174; mux/sn54ls399.pdf is the same file) — scanned, read from the page images: p.1 (J/W, D/N pinout, FK pinout, description, function table, logic symbol), p.2 (logic diagram), p.3 (ROC, switching), addendum (SN54LS399J, SNJ54LS399J, SN74LS399N); mux/cy74fct399t.pdf (SCCS024A) p.1 (SO pinout, description, pin description, ordering), p.2 (function table, logic diagram), p.3 (ROC), p.5 (timing requirements, switching), p.6 (load circuit 50 pF), addendum (SOIC (DW)).
- **Families:** LS (SN74LS399 N, SN54LS399 J), FCT (CY74FCT399AT and CY74FCT399CT, SOIC DW).
- **Pin names:** the classic SN names (sn74ls399 p.1): WS, CLK, A1, A2, B1, B2, C1, C2, D1, D2, QA–QD. CY74FCT399T prints S, CP, I0A/I1A … I0D/I1D, QA–QD. Checked pin by pin (cy74fct399t p.1 pinout and p.2 logic diagram vs sn74ls399 p.1 pinout and logic symbol): S = WS (1), CP = CLK (9), I0x = x1 (A1 3, B1 6, C1 11, D1 14), I1x = x2 (A2 4, B2 5, C2 12, D2 13); outputs QA 2, QB 7, QC 10, QD 15 on both. FCT S low selects I0 (p.2 table) = LS WS low selects word 1. No overbars.
- **Pinouts:**

**p16** — N, J (sn74ls399 p.1: "SN54LS399 … J or W package, SN74LS399 … D or N package", one drawing), DW (cy74fct399t p.1 "SO package"; addendum: SOIC (DW) 16)

| Pin | Name | Role |
|---|---|---|
| 1 | WS | input |
| 2 | QA | output |
| 3 | A1 | input |
| 4 | A2 | input |
| 5 | B2 | input |
| 6 | B1 | input |
| 7 | QB | output |
| 8 | GND | ground |
| 9 | CLK | input |
| 10 | QC | output |
| 11 | C1 | input |
| 12 | C2 | input |
| 13 | D2 | input |
| 14 | D1 | input |
| 15 | QD | output |
| 16 | VCC | power |

- **Function:** (sn74ls399 p.1)

  | Word select | Clock | QA | QB | QC | QD |
  |---|---|---|---|---|---|
  | L | ↑ | a1 | b1 | c1 | d1 |
  | H | ↑ | a2 | b2 | c2 | d2 |
  | X | L | QA0 | QB0 | QC0 | QD0 |

  The LS399 edge is rising: function table ↑, description "clocked to the output terminals on the positive-going edge of the clock pulse" (p.1), and the p.2 logic diagram puts an inverter on CLOCK ahead of flip-flop C1 inputs that carry an inverting bubble (net: rising). Note the '298 is the opposite (falling). FCT (cy74fct399t p.1–2): "transferred to a 4-bit output register synchronous with the low-to-high transition of the clock (CP)"; p.2 table: S l → Q = I0, S h → Q = I1 (levels one setup time before CP↑).

  Model `stored`, four units {data [A1, A2], select [WS], y QA} … {data [D1, D2], select [WS], y QD}; `outputClock` {CLK, rising}.

- **Timing:** (main path CLK → Q; rise = tPLH, fall = tPHL)

- LS: rise 18 ns, fall 21 ns; vcc 4.75–5.25 V. CLK to Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn74ls399 p.3)
- FCT: rise 7 ns, fall 7 ns; vcc 4.75–5.25 V. CP to Q max (no typical printed), CY74FCT399AT grade (CT grade 6.1 ns), 50 pF; VCC 4.75-5.25 V (cy74fct399t p.3, p.5, p.6)

- **Supply:** LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (p.3) → `sn54vcc` LS [4.5, 5.5] (SN54LS399J is an add row). FCT 4.75–5.25 V (cy74fct399t p.3); inputs 7 V absolute max, no 5.5 V-above-VCC rating needed (5 V part), no `inputMax`.
- **Checks against the model:** exact. One FCT timing entry covers both grades; it uses the slower AT max (7 ns), the CT parts (CY74FCT399CTSOC) are 6.1 ns max (p.5).

## 74_354 — 8-line to 1-line data selector/multiplexer/register with 3-state outputs

- **PDFs:** mux/cd74hc354.pdf (SCHS277D; mux/cd74hct354.pdf is the same file) — text loses the overbars, so p.1–4 read from the page images: p.1 (pinout CERDIP/PDIP, description, ordering), p.2–3 (functional diagram, truth table), p.3 (block diagram), p.4 (logic diagram), p.5 (operating conditions), p.7 (HC switching), p.8 (HCT switching).
- **Families:** HC (CD74HC354E PDIP N, CD54HC354F3A CERDIP J), HCT (CD74HCT354E PDIP N).
- **Pin names:** the sheet prints D0–D7, S0–S2, E̅ (pin 9, "enable data"), LE̅ (11), OE̅1 (15), OE̅2 (16), OE3 (17, active high), Y̅ (18), Y (19). Here, overbars dropped as the classic names: `E`, `LE`, `OE1`, `OE2`, `OE3`, `Y`; the complement output Y̅ is `YBAR` (as 74_2G157 and QBAR in dff.json). Overbars checked on the p.1 pinout, p.2 functional diagram and p.3 block diagram.
- **Pinouts:**

**p20** — N, J (p.1: one drawing for CD54HC354 (CERDIP) and CD74HC354/CD74HCT354 (PDIP))

| Pin | Name | Role |
|---|---|---|
| 1 | D7 | input |
| 2 | D6 | input |
| 3 | D5 | input |
| 4 | D4 | input |
| 5 | D3 | input |
| 6 | D2 | input |
| 7 | D1 | input |
| 8 | D0 | input |
| 9 | E | input |
| 10 | GND | ground |
| 11 | LE | input |
| 12 | S2 | input |
| 13 | S1 | input |
| 14 | S0 | input |
| 15 | OE1 | input |
| 16 | OE2 | input |
| 17 | OE3 | input |
| 18 | YBAR | output |
| 19 | Y | output |
| 20 | VCC | power |

- **Function:** truth table (p.2–3; the select columns "show the input address setup with LE̅ low"):

  | S2 S1 S0 | E̅ | OE̅1 | OE̅2 | OE3 | Y̅ | Y |
  |---|---|---|---|---|---|---|
  | X | X | H | X | X | Z | Z |
  | X | X | X | H | X | Z | Z |
  | X | X | X | X | L | Z | Z |
  | n (LLL … HHH) | L | L | L | H | D̅n | Dn |
  | n | H | L | L | H | D̅n(n) | Dn(n) |

  (one row pair per n = 0…7; Dn(n) = "the level of steady-state inputs D0 through D7 … before the most recent low-to-high transition of data control".) So E̅ low = data latches transparent, E̅ rising = latch, E̅ high = hold. The p.3 block diagram and p.4 logic diagram agree (E̅ buffered into the data-latch gates; LE̅ likewise into the address latches, transparent while LE̅ low). **Contradiction:** the p.1 description says the E̅ latches "pass data to the outputs when E̅ is high and latches in new data when E̅ is low"; the truth table and the logic diagrams win. LE̅: "stored in transparent latches that are enabled by a low latch enable input" (p.1) — no conflict.

  Model `stored`, one unit {data [D0..D7], select [S0, S1, S2] (S0 = LSB), y Y, w YBAR}; `dataLatch` {E, active high} (holds while E̅ high); `selectLatch` {LE, active high}; `oes` [{OE1, low}, {OE2, low}, {OE3, high}] — Y and Y̅ driven only while all three are asserted.

- **Timing:** (main path Dn → Y with E̅ and LE̅ low (transparent); rise = tPLH, fall = tPHL; the sheet prints one value for both)

- HC: rise 18 ns, fall 18 ns; vcc 2–6 V. Dn to Y, YBAR typ (E and LE low), 5 V 15 pF (50 pF max only: 42 ns at 4.5 V); VCC 2-6 V (cd74hc354 p.5, p.7)
- HCT: rise 20 ns, fall 20 ns; vcc 4.5–5.5 V. Dn to Y, YBAR typ (E and LE low), 5 V 15 pF (50 pF max only: 47 ns at 4.5 V); VCC 4.5-5.5 V (cd74hc354 p.5, p.8)

  (Other typicals at 5 V 15 pF, HC/HCT: E̅ → Y 21/23, Sn → Y 22/25, LE̅ → Y 24/25 ns.)

- **Supply:** HC 2–6 V, HCT 4.5–5.5 V (p.5); CD54HC354 shares the HC range (one table, −55…125 °C for all, p.1), no `sn54vcc`.
- **Checks against the model:** exact — transparent data latches (hold while E̅ high), transparent select latches (hold while LE̅ high), three shared output enables (two active low, one active high), complementary 3-state outputs. Typical timing exists only at 15 pF; the 50 pF figures are max.

## 74_356 — 8-input multiplexer/register, three-state

- **PDFs:** mux/cd74hct356.pdf (SCLS459A) — text loses the overbars, p.1–4 read from the page images: p.1 (pinout PDIP/SOIC, description, ordering), p.2–3 (functional diagram, truth table), p.3 (block diagram), p.4 (logic diagram), p.5 (operating conditions, prerequisite timing), p.7 (switching).
- **Families:** HCT (CD74HCT356E PDIP N). The ordering table (p.1) also lists CD74HCT356M96 (20 Ld SOIC), which has no parts.tsv row.
- **Pin names:** the sheet prints D0–D7, S0–S2, CP (pin 9), LE̅ (11), OE̅1 (15), OE̅2 (16), OE3 (17), Y̅ (18), Y (19). Here `CP`, `LE`, `OE1`, `OE2`, `OE3`, `Y`, `YBAR` (overbars dropped; Y̅ as YBAR, as 74_354). Same pin numbers as the '354 except pin 9 (CP instead of E̅).
- **Pinouts:**

**p20** — N (p.1, "CD74HCT356 (PDIP or SOIC)")

| Pin | Name | Role |
|---|---|---|
| 1 | D7 | input |
| 2 | D6 | input |
| 3 | D5 | input |
| 4 | D4 | input |
| 5 | D3 | input |
| 6 | D2 | input |
| 7 | D1 | input |
| 8 | D0 | input |
| 9 | CP | input |
| 10 | GND | ground |
| 11 | LE | input |
| 12 | S2 | input |
| 13 | S1 | input |
| 14 | S0 | input |
| 15 | OE1 | input |
| 16 | OE2 | input |
| 17 | OE3 | input |
| 18 | YBAR | output |
| 19 | Y | output |
| 20 | VCC | power |

- **Function:** truth table (p.2–3; selects "with LE̅ low"):

  | S2 S1 S0 | CP | OE̅1 | OE̅2 | OE3 | Y̅ | Y |
  |---|---|---|---|---|---|---|
  | X | X | H | X | X | Z | Z |
  | X | X | X | H | X | Z | Z |
  | X | X | X | X | L | Z | Z |
  | n | ↑ | L | L | H | D̅n | Dn |
  | n | H or L | L | L | H | D̅n(n) | Dn(n) |

  (one row pair per n = 0…7.) Description (p.1): "The data is stored in edge-triggered flip-flops that are triggered by a low-to-high clock transition"; the select bits "are stored in transparent latches that are enabled by a low latch enable input (LE̅)". The p.4 logic diagram shows a master-slave flip-flop per data input clocked by CP, and the select latches transparent while LE̅ low.

  Model `stored`, one unit {data [D0..D7], select [S0, S1, S2], y Y, w YBAR}; `dataClock` {CP, rising}; `selectLatch` {LE, active high}; `oes` [{OE1, low}, {OE2, low}, {OE3, high}].

- **Timing:** (main path CP → Y; rise = tPLH, fall = tPHL, one value printed for both)

- HCT: rise 22 ns, fall 22 ns; vcc 4.5–5.5 V. CP to Y, YBAR typ, 5 V 15 pF (50 pF max only: 51 ns at 4.5 V); VCC 4.5-5.5 V (cd74hct356 p.5, p.7)

- **Supply:** HCT 4.5–5.5 V (p.5).
- **Checks against the model:** exact. (The p.5 prerequisite table labels data setup/hold "Dn → E", copied from the '354 sheet; it means CP here.)

## 74_670 — 4-by-4 register files with 3-state outputs

- **PDFs:** mux/cd74hc670.pdf (SCHS195C; mux/cd74hct670.pdf is the same file) p.1 (pinout CERDIP/PDIP/SOIC, description, ordering — page image for the WE̅/RE̅ overbars), p.2 (functional diagram, write/read mode tables — page image), p.3 (operating conditions), p.5 (HC switching), p.6 (HCT switching); mux/sn74ls670.pdf (SDLS193) — scanned, read from the page images: p.1 (J/W and D/N pinout, FK pinout, description), p.2 (logic symbol, write/read function tables), p.3 (logic diagram), p.4 (ROC), p.5 (switching), addendum (SN74LS670D, N, NSR; SN54LS670J, SNJ54LS670J/W/FK).
- **Families:** HC (CD74HC670E N, CD74HC670M96 D), HCT (CD74HCT670E N, CD74HCT670M D), LS (SN74LS670D, SN74LS670N, SN74LS670NSR NS).
- **Pin names:** the classic SN names (sn74ls670 p.1–2): D1–D4, WA, WB, G̅W (write enable), RA, RB, G̅R (read enable), Q1–Q4. The CD sheet numbers from 0: D0–D3, WA0, WA1, WE̅, RA0, RA1, RE̅, Q0–Q3. Pin by pin (cd74hc670 p.1 vs sn74ls670 p.1): D0 = D1 (15), D1 = D2 (1), D2 = D3 (2), D3 = D4 (3), RA1 = RB (4), RA0 = RA (5), Q3 = Q4 (6), Q2 = Q3 (7), Q1 = Q2 (9), Q0 = Q1 (10), RE̅ = G̅R (11), WE̅ = G̅W (12), WA1 = WB (13), WA0 = WA (14). WA/RA are the LSB (LS write table: WB L, WA H → word 1; logic symbol WA = 0, WB = 1). Here `GW`, `GR` with the overbars dropped.
- **Pinouts:**

**p16** — N, D, NS (cd74hc670 p.1 "PDIP, SOIC"; sn74ls670 p.1 "SN74LS670 … D or N package"; NS: the addendum lists SN74LS670NSR as SOP (NS) 16 — the p.1 drawing predates the NS option and does not name it; TI's NS SOP keeps the D/N pin order)

| Pin | Name | Role |
|---|---|---|
| 1 | D2 | input |
| 2 | D3 | input |
| 3 | D4 | input |
| 4 | RB | input |
| 5 | RA | input |
| 6 | Q4 | output |
| 7 | Q3 | output |
| 8 | GND | ground |
| 9 | Q2 | output |
| 10 | Q1 | output |
| 11 | GR | input |
| 12 | GW | input |
| 13 | WB | input |
| 14 | WA | input |
| 15 | D1 | input |
| 16 | VCC | power |

- **Function:** sn74ls670 p.2 write function table:

  | WB | WA | G̅W | word 0 | 1 | 2 | 3 |
  |---|---|---|---|---|---|---|
  | L | L | L | Q = D | Q0 | Q0 | Q0 |
  | L | H | L | Q0 | Q = D | Q0 | Q0 |
  | H | L | L | Q0 | Q0 | Q = D | Q0 |
  | H | H | L | Q0 | Q0 | Q0 | Q = D |
  | X | X | H | Q0 | Q0 | Q0 | Q0 |

  read function table:

  | RB | RA | G̅R | Q1 | Q2 | Q3 | Q4 |
  |---|---|---|---|---|---|---|
  | L | L | L | W0B1 | W0B2 | W0B3 | W0B4 |
  | L | H | L | W1B1 | W1B2 | W1B3 | W1B4 |
  | H | L | L | W2B1 | W2B2 | W2B3 | W2B4 |
  | H | H | L | W3B1 | W3B2 | W3B3 | W3B4 |
  | X | X | H | Z | Z | Z | Z |

  cd74hc670 p.2 write/read mode tables say the same (WE̅ L: latches follow Dn; WE̅ H: no change; RE̅ L: Qn = latch; RE̅ H: Z; read address selection "not constrained by WE̅ or RE̅"); p.1 description: "When WE̅ is low the word is entered into the address location and it remains transparent to the data". sn74ls670 p.3 logic diagram: gated D latches (transparent), AND-OR-INVERT read select.

  Model `regfile`, `words` 4, `bits` 4, one file {d [D1..D4], wa [WA, WB], we {GW, low}, ra [RA, RB], re {GR, low}, q [Q1..Q4]}.

- **Timing:** (main path read address → Q; rise = tPLH, fall = tPHL)

- HC: rise 16 ns, fall 16 ns; vcc 2–6 V. read address to Q (reading any word) typ, 5 V 15 pF (50 pF max only: 39 ns at 4.5 V); VCC 2-6 V (cd74hc670 p.3, p.5)
- HCT: rise 17 ns, fall 17 ns; vcc 4.5–5.5 V. read address to Q (reading any word) typ, 5 V 15 pF (50 pF max only: 40 ns at 4.5 V); VCC 4.5-5.5 V (cd74hc670 p.3, p.6)
- LS: rise 23 ns, fall 25 ns; vcc 4.75–5.25 V. read select to any Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V (sn74ls670 p.4, p.5)

- **Supply:** HC 2–6 V, HCT 4.5–5.5 V (cd74hc670 p.3). LS SN74 4.75–5.25 V, SN54LS 4.5–5.5 V (p.4); no SN54 part is an add row, so no `sn54vcc`.
- **Checks against the model:** exact for the logic. Not modelled: the "latch time for new data" (t_latch 25 ns LS, p.4; HC/HCT tLATCH) — the delay before a just-written word reads back; the model reads the array combinationally. A write address that changes while G̅W is low writes into every address it passes through (LS note 2), as the model does.
