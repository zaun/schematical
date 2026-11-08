# plan-22 device specs

One section per device that plan-22 adds (every device with an `add` row in
`parts.tsv`), read 2026-11-07 from the datasheets in `datasheets/comparator/`,
`datasheets/arith/` and `datasheets/parity/`. Page numbers are PDF page
indices. Pin names drop overbars and the Σ sign; active levels are in each
section's Function. Each section ends with the device's model keys
(docs/parts-format.md § `arith`) and the points where the model is not exact.
Devices in plan-22-A §3.1 order: model `compare`, then `parity`, `adder`,
`alu181`, `wallace`.

Devices of plan-22-A §3.1 with no section, because no part of theirs is
added (`parts.tsv` reasons):

- 74_182 (SN74S182N, the only in-stock '182) and 74_180 (SN74180N,
  SNJ54HC180J): TI's current sheets (sn54s182 = sdls206, sn54180 = sdls071)
  list only SN54 J/W/FK parts, none of them in stock at DigiKey; the in-stock
  SN74 parts are not in any TI ordering table. So plan-22-C's model
  `lookahead` and the '180's EVEN/ODD inputs carry no part and are not built
  (plan-22-A Corrections).
- No TI datasheet (no ti.com product page, no `lit/ds` symlink, and the TI
  literature numbers printed on the Rochester copies return 404 at ti.com;
  DigiKey links only Rochester Electronics or DigiKey-hosted copies):
  74ACT8832, 74ACT8847, '381, '382, '881, '882, '183, '385, '80, '264,
  '261, '384, '685, '686, '689, F518/F519 (SN54F518J, SNJ54F518J,
  SNJ54F519J), '677, '679, '680, '866, '527, '528, 29806, 74AC11181,
  74AC11521, SN700526.
- Every candidate Obsolete: CY74FCT480T (sccs025b).

## 74_85 — 4-bit magnitude comparators

- **PDFs:** comparator/sn74ls85.pdf (SDLS123, scanned; read from the page images) p.1 (J/W, N, D/N pinout; FK; function table), p.2 (logic diagram, logic symbol: "pin numbers shown are for D, J, N, and W packages"), p.4 ('85 ROC + timing), p.5 ('LS85 ROC + timing), p.6 ('S85 ROC + timing), addendum (JM38510/08201BEA = SN54S85 J, JM38510/31101BEA = SN54LS85 J, SN54LS85J, SN74LS85DR/N/NSR, SN74S85D/N); comparator/cd74hc85.pdf (SCHS136F) p.3 (J/N/D/NS/PW pinout), p.4 (ROC), p.6 (switching), p.10 (Table 7-1 truth table), addendum (8601301EA = CD54HC85F3A, CDIP J, marking 8601301EA)
- **Families:** HC (CD74HC85, CD54HC85 as 8601301EA), HCT (CD74HCT85), LS (SN74LS85, SN54LS85, JM38510/31101BEA), S (SN74S85, JM38510/08201BEA = SN54S85)
- **Pin names:** sn74ls85 p.1 prints B3, A<Bin, A=Bin, A>Bin, A>Bout, A=Bout, A<Bout, GND, B0, A0, B1, A1, A2, B2, A3, VCC (no overbars; every pin active-high); its p.2 logic symbol uses P0–P3/Q0–Q3 and P<Q, P=Q, P>Q for the same pins. cd74hc85 p.3 prints (A < B) IN, (A = B) IN, (A > B) IN, (A > B) OUT, (A = B) OUT, (A < B) OUT with A0–A3, B0–B3 on the same pin numbers; checked pin by pin, identical. Spec names: A0–A3, B0–B3, A<BIN, A=BIN, A>BIN, A>BOUT, A=BOUT, A<BOUT.
- **Pinouts:**

**p16** — N, D, NS, PW, J. Drawings: sn74ls85 p.1 (SN54LS85/SN54S85 J or W; SN74LS85/SN74S85 D or N); cd74hc85 p.3 (J, N, D, NS, PW). NS is not drawn on sn74ls85 (SN74LS85NSR, addendum only) but is the same 16-lead order on cd74hc85 p.3.

| Pin | Name | Role |
|---|---|---|
| 1 | B3 | input |
| 2 | A<BIN | input |
| 3 | A=BIN | input |
| 4 | A>BIN | input |
| 5 | A>BOUT | output |
| 6 | A=BOUT | output |
| 7 | A<BOUT | output |
| 8 | GND | ground |
| 9 | B0 | input |
| 10 | A0 | input |
| 11 | B1 | input |
| 12 | A1 | input |
| 13 | A2 | input |
| 14 | B2 | input |
| 15 | A3 | input |
| 16 | VCC | power |

(FK 20-pad is on sn74ls85 p.1 but no FK part is an add row.)

- **Function:** sn74ls85 p.1 function table (columns A>B, A<B, A=B in / A>B, A<B, A=B out). Rows with unequal words (A3>B3 … A0<B0, cascade X) give A>B out H only or A<B out H only. A = B rows, transcribed exactly:

  | A>B in | A<B in | A=B in | A>B out | A<B out | A=B out |
  |---|---|---|---|---|---|
  | H | L | L | H | L | L |
  | L | H | L | L | H | L |
  | X | X | H | L | L | H |
  | H | H | L | L | L | L |
  | L | L | L | H | H | L |

  The scanned p.1 table has two printed typos in the comparing columns: row 5 reads "A3 = B2" and row 7 "A2 = B3" (both mean A3 = B3); they don't touch the cascade rows.

  cd74hc85 p.10 Table 7-1: same unequal rows; "single device or series cascading" rows H L L → H L L, L H L → L H L, **L L H → L L H**; "parallel cascading" rows X X H → L L H, H H L → L L L, L L L → H H L. So it has one extra row (L L H), which X X H already covers; otherwise the CD74HC85 table **matches** the SN74LS85 one row for row, including the two-high and all-low rows.

  Model `compare`: `a` A0–A3, `b` B0–B3; `outputs` gt A>BOUT, eq A=BOUT, lt A<BOUT, all active high; `cascade` gt A>BIN, eq A=BIN, lt A<BIN; `cascadeTable` (in gt,eq,lt → out gt,eq,lt): HLL→HLL, LLH→LLH, XHX→LHL, HLH→LLL, LLL→HLH (the five sn74ls85 rows, in datasheet order; the five rows cover all 8 input combinations). No enable, latch or signed.

- **Timing:** (main path A/B data → A>B, A<B; rise = tPLH, fall = tPHL)

- HC: rise 16 ns, fall 16 ns; vcc 2–6 V. An, Bn to (A > B)/(A < B) OUT typ at 5 V, 15 pF (one figure for tPLH, tPHL; to (A = B) OUT 14 ns); cd74hc85 p.6, ROC p.4
- HCT: rise 15 ns, fall 15 ns; vcc 4.5–5.5 V. Same row, HCT (to (A = B) OUT 17 ns); cd74hc85 p.6, p.4
- LS: rise 24 ns, fall 20 ns; vcc 4.75–5.25 V. Any A or B to A<B, A>B, 3 gate levels, typ, 5 V 15 pF (to A=B, 4 levels: 27/23 ns; p.1 quotes "24 ns" 4-bit delay); sn74ls85 p.5
- S: rise 10.5 ns, fall 11 ns; vcc 4.75–5.25 V. Same row, 'S85 (to A=B: 12/11 ns; p.1 quotes 11 ns); sn74ls85 p.6

- **Supply:** LS, S SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls85 p.5, p.6) — `sn54vcc` LS, S [4.5, 5.5] (SN54LS85J, JM38510/31101BEA, JM38510/08201BEA). HC 2–6 V, HCT 4.5–5.5 V for CD54 and CD74 alike (cd74hc85 p.4; TA −55–125 °C).
- **Checks against the model:** exact. Unequal words: the datasheet rows give gt only / lt only regardless of the cascade inputs, as the model does. A = B: the five-row `cascadeTable` covers every cascade combination. HC/HCT print typ only at 15 pF (one value for tPLH and tPHL); LS/S typ at 15 pF.

## 4063B — CMOS 4-bit magnitude comparator

- **PDFs:** comparator/cd4063b.pdf (SCHS050C, Harris scan; read from the page images) p.1 (description, terminal assignment, ROC, Fig. 1 cascading), p.2 (truth table), p.3 (Fig. 2 logic diagram, dynamic characteristics), addendum (CD4063BE PDIP N, CD4063BM96 SOIC D, CD4063BNSR SOP NS)
- **Families:** C (CD4063B)
- **Pin names:** p.1 terminal assignment prints B3, (A<B)IN, (A=B)IN, (A>B)IN, (A>B)OUT, (A=B)OUT, (A<B)OUT, VSS, B0, A0, B1, A1, A2, B2, A3, VDD — no overbars, all active-high; p.3 logic diagram uses the same pin numbers (A0 10, A1 12, A2 13, A3 15, B0 9, B1 11, B2 14, B3 1, (A<B)IN 2, (A=B)IN 3, (A>B)IN 4, outputs 5/6/7). "Pin-compatible with the standard 7485" (p.1). Spec names: A0–A3, B0–B3, A<BIN, A=BIN, A>BIN, A>BOUT, A=BOUT, A<BOUT, VDD, VSS.
- **Pinouts:**

**p16** — N, D, NS. p.1 terminal assignment (the one 16-lead drawing; p.1 text: E = DIP, M/M96/MT/NSR = small outline, all 16-lead).

| Pin | Name | Role |
|---|---|---|
| 1 | B3 | input |
| 2 | A<BIN | input |
| 3 | A=BIN | input |
| 4 | A>BIN | input |
| 5 | A>BOUT | output |
| 6 | A=BOUT | output |
| 7 | A<BOUT | output |
| 8 | VSS | ground |
| 9 | B0 | input |
| 10 | A0 | input |
| 11 | B1 | input |
| 12 | A1 | input |
| 13 | A2 | input |
| 14 | B2 | input |
| 15 | A3 | input |
| 16 | VDD | power |

- **Function:** p.2 truth table (columns A<B, A=B, A>B in / A<B, A=B, A>B out; 1 = H). Unequal rows (cascade X): A>B words → 0 0 1 (A>B out only), A<B words → 1 0 0. A = B rows, exactly as printed:

  | A<B in | A=B in | A>B in | A<B out | A=B out | A>B out |
  |---|---|---|---|---|---|
  | 0 | 0 | 1 | 0 | 0 | 1 |
  | 0 | 1 | 0 | 0 | 1 | 0 |
  | 1 | 0 | 0 | 1 | 0 | 0 |

  p.1: a single (or least significant) stage ties A<B low, A=B high, A>B low.

  Model `compare`: `a` A0–A3, `b` B0–B3; `outputs` gt A>BOUT, eq A=BOUT, lt A<BOUT, active high; `cascade` gt A>BIN, eq A=BIN, lt A<BIN; `cascadeTable` (gt,eq,lt): HLL→HLL, LHL→LHL, LLH→LLH.

- **Timing:** C: rise 625 ns, fall 625 ns; vcc 3–18 V. Comparing inputs to outputs, tPHL/tPLH typ at VDD 5 V, 50 pF (max 1250; cascading inputs to outputs 500 typ); p.3.
- **Supply:** VDD 3–18 V (p.1 ROC, full package temperature range); no separate military range.
- **Checks against the model:** exact. The truth table (p.2) prints only the three one-hot cascade rows; the other five equal-word rows of `cascadeTable` are read from the p.3 logic diagram (Fig. 2): (A<B)IN and (A>B)IN gate straight into the A<B and A>B outputs at equal words, and the A=B output's AND gate takes (A=B)IN with both other outputs inverted, so A>B out = A>B in, A<B out = A<B in, A=B out = A=B in AND NOT A>B in AND NOT A<B in (LLL→LLL, LHH→LLH, HLH→HLH, HHL→HLL, HHH→HLH; checked against the diagram 2026-11-07). Unequal words: gt only / lt only, as the model does.

## 4585B — CMOS 4-bit magnitude comparator

- **PDFs:** comparator/cd4585b.pdf (SCHS091B, Harris scan; read from the page images) p.1 (description, functional diagram with pin numbers, ROC), p.2 (truth table, Fig. 4 logic diagram), p.3 (dynamic characteristics), p.4 (terminal assignment, Fig. 13 cascading), addendum (CD4585BE PDIP N, CD4585BF3A CDIP J = 7703702EA, CD4585BNSR SOP NS, CD4585BPW TSSOP PW)
- **Families:** C (CD4585B)
- **Pin names:** p.4 terminal assignment prints B2, A2, (A=B)OUT, (A>B)IN, (A<B)IN, (A=B)IN, A1, VSS, B1, A0, B0, (A<B)OUT, (A>B)OUT, B3, A3, VDD — no overbars, all active-high. Checked pin by pin against the p.1 functional diagram (A0 10, A1 7, A2 2, A3 15, A>B in 4, A=B in 6, A<B in 5, B0 11, B1 9, B2 1, B3 14, outputs A>B 13, A=B 3, A<B 12, VDD 16, VSS 8) and the p.2 logic diagram: same. **Not** the 7485 pin order. Spec names: A0–A3, B0–B3, A<BIN, A=BIN, A>BIN, A>BOUT, A=BOUT, A<BOUT, VDD, VSS.
- **Pinouts:**

**p16** — N, J, NS, PW. p.4 terminal assignment (the one 16-lead drawing; p.1 text: F3A ceramic DIP, E plastic DIP, NSR small outline, PW TSSOP, all 16-lead).

| Pin | Name | Role |
|---|---|---|
| 1 | B2 | input |
| 2 | A2 | input |
| 3 | A=BOUT | output |
| 4 | A>BIN | input |
| 5 | A<BIN | input |
| 6 | A=BIN | input |
| 7 | A1 | input |
| 8 | VSS | ground |
| 9 | B1 | input |
| 10 | A0 | input |
| 11 | B0 | input |
| 12 | A<BOUT | output |
| 13 | A>BOUT | output |
| 14 | B3 | input |
| 15 | A3 | input |
| 16 | VDD | power |

- **Function:** p.2 truth table (columns A<B, A=B, A>B in / A<B, A=B, A>B out; 1 = H, 0 = L):

  | words | A<B in | A=B in | A>B in | A<B out | A=B out | A>B out |
  |---|---|---|---|---|---|---|
  | A > B (A3>B3 … A0>B0, 4 rows) | X | X | 1 | 0 | 0 | 1 |
  | A = B | 0 | 0 | 1 | 0 | 0 | 1 |
  | A = B | 0 | 1 | X | 0 | 1 | 0 |
  | A = B | 1 | 0 | X | 1 | 0 | 0 |
  | A < B (A0<B0 … A3<B3, 4 rows) | X | X | X | 1 | 0 | 0 |

  p.1: a single stage ties A<B low, A=B high, A>B **high**; p.4 Fig. 13 ties the first stage's A>B and A=B in to VDD and A<B in to VSS.

  Model `compare`: `a` A0–A3, `b` B0–B3; `outputs` gt A>BOUT, eq A=BOUT, lt A<BOUT, active high; `cascade` gt A>BIN, eq A=BIN, lt A<BIN; `cascadeTable` (gt,eq,lt): HLL→HLL, XHL→LHL, XLH→LLH (the three A = B rows).

- **Timing:** C: rise 300 ns, fall 300 ns; vcc 3–18 V. Comparing inputs to outputs, tPHL/tPLH typ at VDD 5 V, 50 pF (max 600; cascading inputs to outputs 200 typ); p.3.
- **Supply:** VDD 3–18 V (p.1 ROC, full package temperature range, CD4585BF3A included).
- **Checks against the model:** exact, with rows taken from the p.2 logic diagram (Fig. 4) where the truth table is silent.
  1. A > B words: the table defines them only with A>B in = H. The logic diagram ANDs A>B out with A>B in, so with A>B in = L and A > B words all three outputs are L. `cascadeTable` rows with `"when": "gt"`: HXX→HLL, LXX→LLL.
  2. A = B words: the table prints HLL→HLL, XHL→LHL, XLH→LLH. From the diagram A<B out = A<B in, A=B out = A=B in and A>B out = A>B in AND NOT A<B in AND NOT A=B in, which adds LLL→LLL and XHH→LHH (the remaining combinations).
  3. A < B words: lt only regardless of cascade, as the model does.

## 74_682 — 8-bit magnitude comparators

- **PDFs:** comparator/sn74hc682.pdf p.3 (J/W/DW/N pinout, FK), p.4 (ROC), p.5 (switching), p.8 (function table, Table 7-1); comparator/sn74ls682.pdf (SDLS008, SN54/74LS682/684/685/687/688, scanned, read from the page images) p.1 (pinouts: 'LS682/'LS684/'LS685 J, DW or N; FK; type table), p.2 (function table), p.4 (logic diagram), p.7 (ROC, totem-pole types), p.8 (switching), addendum p.1–2 (ordering: SNJ54LS682J = 8415101RA, SNJ54LS682W = 8415101SA).
- **Families:** HC (SN74HC682; SN54HC682 is "product preview", no add row), LS (SN74LS682, SNJ54LS682).
- **Pin names:** both sheets print P̅>̅Q̅ (pin 1), P0–P7, Q0–Q7, P̅=̅Q̅ (pin 19), VCC, GND. Model names: `P>Q`, `P=Q` (overbar on the active level), checked pin by pin: identical on sn74hc682 p.3 and sn74ls682 p.1.
- **Pinouts:**

**p20** — N, DW, J, W. Drawings: sn74hc682 p.3 ("J, W, DW, or N Package"); sn74ls682 p.1 (SN54LS682 J, SN74LS682 DW or N). The LS sheet draws no W package; SNJ54LS682W (CFP W, addendum p.1) is taken on the J order, as sn74hc682 p.3 draws W on the same order as J. (The FK drawings on both sheets put pad n = DIP pin n; no FK add row.)

| Pin | Name | Role |
|---|---|---|
| 1 | P>Q | output |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** sn74hc682 p.8 Table 7-1, and the first three rows of sn74ls682 p.2 (the enable rows apply only to 'LS686–'LS688, note 1):

  | P, Q | P̅=̅Q̅ | P̅>̅Q̅ |
  |---|---|---|
  | P = Q | L | H |
  | P > Q | H | L |
  | P < Q | H | H |

  Model `compare`: `a` P0–P7, `b` Q0–Q7 (LSB first; the p.4 logic diagram compares P7/Q7 as MSB), `outputs` gt {P>Q, low}, eq {P=Q, low}. No cascade, no enable, no lt (the sheets note P̅<̅Q̅ = NAND of the two outputs).
- **Timing:** (rise = tPLH, fall = tPHL)
  - HC: rise 26 ns, fall 26 ns; vcc 2–6 V. P or Q to any output, tpd typ (one figure for both edges), 4.5 V 50 pF (sn74hc682 p.5); VCC 2–6 V (p.4).
  - LS: rise 21 ns, fall 19 ns; vcc 4.75–5.25 V. Typ at 5 V, 45 pF, RL 667 Ω (sn74ls682 p.8). 'LS682 typ: P→P̅=̅Q̅ 13/15, Q→P̅=̅Q̅ 14/15, P→P̅>̅Q̅ 20/15, Q→P̅>̅Q̅ 21/19 (tPLH/tPHL); the slowest path (Q→P̅>̅Q̅) is used.
- **Supply:** HC 2–6 V, SN54HC682 the same (sn74hc682 p.4). LS: SN74LS' 4.75–5.25 V, SN54LS' 4.5–5.5 V (sn74ls682 p.7) — `sn54vcc` LS [4.5, 5.5] (SNJ54LS682J, SNJ54LS682W). The scan of p.7 reads the SN74LS' minimum as "4.85"; that is taken as the standard 4.75 (the scanned 7 is blurred; every other SN74LS sheet gives 4.75).
- **Checks against the model:** logic exact. Not expressible: the Q inputs have pull-up resistors (SN74HC682 100 kΩ, sn74hc682 p.1/p.7, IIL −50 µA typ on Q, p.5; 'LS682 20 kΩ, sn74ls682 p.1–3, Q-input max 5.5 V abs-max p.6). An open Q input reads H on the real part; the model has no input pull-up, so a floating Q input is not defined as H. P and Q inputs of the 'LS682 also have 0.4 V hysteresis (sn74ls682 p.7), which the model does not need.

## 74_684 — 8-bit magnitude comparators

- **PDFs:** comparator/sn74hc684.pdf (SCLS340B) p.1 (pinouts J/W, DW/N, FK; ordering; function table), p.3 (ROC), p.4 (switching); comparator/sn74ls682.pdf (SDLS008, scanned, read from the page images) p.1 (pinouts: SN54LS684 J, SN74LS684 DW or N; FK), p.2 (function table), p.4 (logic diagram), p.7 (ROC), p.8 (switching), addendum p.1–2 (ordering).
- **Families:** HC (SN74HC684), LS (SN74LS684, SN54LS684).
- **Pin names:** both sheets print P̅>̅Q̅ (pin 1), P0–P7, Q0–Q7, P̅=̅Q̅ (pin 19), VCC, GND; identical pin by pin on sn74hc684 p.1 and sn74ls682 p.1. Model names `P>Q`, `P=Q`.
- **Pinouts:**

**p20** — J, DW, N. Drawings: sn74hc684 p.1 (SN54HC684 J or W, SN74HC684 DW or N); sn74ls682 p.1 (SN54LS684 J, SN74LS684 DW or N). Same order as 74_682 (FK drawings on both sheets give pad n = pin n; no FK add row).

| Pin | Name | Role |
|---|---|---|
| 1 | P>Q | output |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** sn74hc684 p.1 and sn74ls682 p.2 (first three rows):

  | P, Q | P̅=̅Q̅ | P̅>̅Q̅ |
  |---|---|---|
  | P = Q | L | H |
  | P > Q | H | L |
  | P < Q | H | H |

  Model `compare`: `a` P0–P7, `b` Q0–Q7 (LSB first), `outputs` gt {P>Q, low}, eq {P=Q, low}. No cascade, enable or lt.
- **Timing:** (rise = tPLH, fall = tPHL)
  - HC: rise 26 ns, fall 26 ns; vcc 2–6 V. P or Q to any output, tpd typ, 4.5 V 50 pF (sn74hc684 p.4); VCC 2–6 V (p.3).
  - LS: rise 24 ns, fall 20 ns; vcc 4.75–5.25 V. Typ at 5 V, 45 pF (sn74ls682 p.8). 'LS684 typ: P→P̅=̅Q̅ 15/17, Q→P̅=̅Q̅ 16/15, P→P̅>̅Q̅ 22/17, Q→P̅>̅Q̅ 24/20; the slowest (Q→P̅>̅Q̅) is used.
- **Supply:** HC 2–6 V (SN54 the same, sn74hc684 p.3). LS SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls682 p.7; the scan's SN74 min reads "4.85", taken as 4.75) — `sn54vcc` LS [4.5, 5.5] (SN54LS684J).
- **Checks against the model:** exact (the '684 has no Q-input pull-ups, unlike the '682). LS inputs have 0.4 V hysteresis (sn74ls682 p.7), not needed by the model.

## 74_688 — 8-bit identity comparators

- **PDFs:** comparator/sn74hc688.pdf (SCLS010F) p.1 (device info), p.3 (J/DW/N/PW pinout, FK), p.4 (ROC), p.5 (switching), p.7 (block diagram, function table 7-1); comparator/cd74hc688.pdf (SCHS196E, CD54/74HC688, CD54/74HCT688) p.1, p.3 (J/N/DW/NS/PW pinout), p.4 (ROC), p.6 (switching), p.8 (block diagram, truth table 7-1), addendum (5962-8685701RA = CD54HCT688F3A, CDIP J); comparator/sn74als688.pdf (SDAS228A) p.1 (J, DW/N, FK pinouts; function table), p.4 (ROC, switching), addendum (SNJ54ALS688W = 5962-8857801SA, CFP W); comparator/sn74ls682.pdf (SDLS008, scanned, read from the page images) p.1 (SN54LS688 J, SN74LS688 DW/N; FK), p.2 (function table with G̅), p.3 (logic symbol), p.6 (logic diagram), p.7 (ROC), p.8 (switching), addendum p.3 (SNJ54LS688J = 8415301RA).
- **Families:** HC (SN74HC688, SNJ54HC688, CD74HC688), HCT (CD74HCT688, CD54HCT688 as 5962-8685701RA), ALS (SN74ALS688, SN54ALS688, SNJ54ALS688), LS (SN74LS688, SNJ54LS688 as 8415301RA).
- **Pin names:** one name set: `G` (enable, active low), `P0`–`P7`, `Q0`–`Q7`, `P=Q` (output, active low). Every sheet uses the same pin positions (checked pin by pin); the names differ:
  - sn74als688 p.1, sn74ls682 p.1: G̅ (1), P0 (2), Q0 (3), P1 (4), Q1 (5), P2 (6), Q2 (7), P3 (8), Q3 (9), GND, P4 (11), Q4 (12), P5 (13), Q5 (14), P6 (15), Q6 (16), P7 (17), Q7 (18), P̅=̅Q̅ (19), VCC.
  - sn74hc688 p.3: same, but the enable is printed O̅E̅ (pin 1). OE = G.
  - cd74hc688 p.3: E̅ (1), A0 (2), B0 (3), A1 (4), B1 (5), A2 (6), B2 (7), A3 (8), B3 (9), GND, A4 (11), B4 (12), A5 (13), B5 (14), A6 (15), B6 (16), A7 (17), B7 (18), Y (19), VCC. So E = G, An = Pn, Bn = Qn, Y = P=Q. Y is printed without a bar, but the p.8 truth table gives Y = L when A = B and E̅ = L: it is the same active-low output.
  - The cd74hc688 title and p.1/p.8 text call it a "magnitude comparator"; the function is identity only (truth table p.8: A = B / A ≠ B).
- **Pinouts:**

**p20** — J, N, NS, PW, DW, W, FK. Drawings: sn74hc688 p.3 (J, DW, N or PW) and FK (p.3: pad 1 O̅E̅, 2 P0, 3 Q0, 4 P1, 5 Q1, 6 P2, 7 Q2, 8 P3, 9 Q3, 10 GND, 11 P4, 12 Q4, 13 P5, 14 Q5, 15 P6, 16 Q6, 17 P7, 18 Q7, 19 P̅=̅Q̅, 20 VCC — pad n = DIP pin n, so FK shares p20); cd74hc688 p.3 (J, N, DW, NS, PW); sn74als688 p.1 (SN54ALS688 J, SN74ALS688 DW or N; FK the same pad = pin map); sn74ls682 p.1 (SN54LS688 J, SN74LS688 DW or N; FK the same). W (SNJ54ALS688W) is not drawn on sn74als688; taken on the J order (TI's 20-pin W packages use the DIP order, e.g. sn74hc682 p.3 draws "J, W, DW, or N" as one pinout).

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** sn74hc688 p.7 (with O̅E̅), sn74als688 p.1, sn74ls682 p.2 (rows with G̅); cd74hc688 p.8 has the same three cases as A = B / A ≠ B / E̅ = H:

  | P, Q | G̅ | P̅=̅Q̅ |
  |---|---|---|
  | P = Q | L | L |
  | P > Q | L (X on sn74hc688) | H |
  | P < Q | L (X on sn74hc688) | H |
  | X | H | H |

  Model `compare`: `a` P0–P7, `b` Q0–Q7, `outputs` eq {P=Q, low}, `enable` {G, low} (G high → P=Q at its inactive level H). No gt, lt or cascade.
- **Timing:** (rise = tPLH, fall = tPHL; P or Q → P=Q)
  - HC: rise 30 ns, fall 30 ns; vcc 2–6 V. tpd typ at 4.5 V 50 pF (sn74hc688 p.5). cd74hc688 p.6 prints 14 ns typ at 5 V 15 pF (34 ns max at 4.5 V 50 pF); the SN 50 pF typ is used.
  - HCT: rise 14 ns, fall 14 ns; vcc 4.5–5.5 V. An/Bn to output typ at 5 V 15 pF, the only typ printed (34 ns max at 4.5 V 50 pF) (cd74hc688 p.6).
  - ALS: rise 12 ns, fall 20 ns; vcc 4.5–5.5 V. SN74ALS688 max (no typ printed), 4.5–5.5 V 50 pF (sn74als688 p.4; SN54ALS688 16/25 max).
  - LS: rise 12 ns, fall 17 ns; vcc 4.75–5.25 V. 'LS688 typ, 5 V 45 pF, P and Q → P̅=̅Q̅ both 12/17 (sn74ls682 p.8).
- **Supply:** HC 2–6 V (sn74hc688 p.4, SN54 same; cd74hc688 p.4 HC 2–6 V); HCT 4.5–5.5 V for CD54 and CD74 (cd74hc688 p.4); ALS 4.5–5.5 V, SN54 the same (sn74als688 p.4); LS SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls682 p.7; the scan's SN74 min reads "4.85", taken as 4.75) — `sn54vcc` LS [4.5, 5.5] (8415301RA = SNJ54LS688J).
- **Checks against the model:** exact. The 'LS688 inputs have 0.4 V hysteresis (sn74ls682 p.7), not needed by the model.

## 74_518 — 8-bit identity comparators

- **PDFs:** comparator/sn74als518.pdf (SDAS224B, SN54ALS520/SN74ALS518/SN74ALS520/SN74ALS521) p.1 (feature list, type table, J/DW/N pinout, FK pinout, function table, read from the page image for the overbars), p.2 (logic symbol, SN74ALS518 output P=Q without bar), p.3 (SN74ALS518 logic diagram), p.5 (SN74ALS518 ROC + electrical, Q-input IIL −0.6 mA from the pull-ups), p.6 (SN74ALS518 switching), Addendum-Page 1 (SN74ALS518DW, SN74ALS518N).
- **Families:** ALS (SN74ALS518 only; the sheet has no SN54ALS518).
- **Pin names:** p.1 prints G̅ (1), P0–P7, Q0–Q7, GND (10), "P = Q / P̅ ̅=̅ ̅Q̅" (19) with the footnote "P = Q for SN74ALS518", VCC (20). So pin 19 is P=Q active high on the '518 (also p.2 symbol and p.1 type table "P = Q open collector").
- **Pinouts:**

**p20** — DW, N: p.1 "SN74ALS518, SN74ALS520, SN74ALS521 . . . DW OR N PACKAGE".

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** p.1 function table (P = Q column):

  | P, Q | G̅ | P = Q |
  |---|---|---|
  | P = Q | L | H |
  | P > Q | L | L |
  | P < Q | L | L |
  | X | H | L |

  Model `compare`: `a` P0–P7, `b` Q0–Q7, `outputs` {eq: P=Q, high}, `enable` {G, low} (G̅ high → P=Q at its inactive level L, i.e. the open collector off), no cascade. `output` "open-drain" (p.1 "P = Q open collector"; p.5 IOH 0.1 mA at VOH 5.5 V, no VOH spec).
- **Timing:** ALS: rise 33 ns, fall 15 ns; vcc 4.5–5.5 V. P or Q to P=Q max (no typ printed), 50 pF, RL = 680 Ω (sn74als518 p.6); VCC 4.5–5.5 V (p.5).
- **Supply:** SN74ALS518 4.5–5.5 V (p.5). No SN54 part.
- **Checks against the model:** exact for the logic. Not modeled: the 20-kΩ pull-up resistors on the Q inputs (p.1 feature list and type table; p.5 Q-input IIH −0.2 mA / IIL −0.6 mA): a floating Q input reads H on the real part but X in the model. The open-collector output needs an external pull-up (RL 680 Ω in the test load, p.6).

## 74_520 — 8-bit identity comparators

- **PDFs:** comparator/sn74als518.pdf (SDAS224B) p.1 (feature list, type table, J/DW/N and FK pinouts, function table, read from the page image for the overbars), p.2 (logic symbol 'ALS520/SN74ALS521, output P̅=̅Q̅), p.4 (logic diagram), p.6 ('ALS520 ROC), p.7 (electrical + switching), Addendum-Page 1 (SN74ALS520DW, SN74ALS520N, SN74ALS520NSR SOP (NS) | 20).
- **Families:** ALS (SN74ALS520; SN54ALS520 J/FK on the sheet are not add rows).
- **Pin names:** p.1 prints G̅ (1), P0–P7, Q0–Q7, GND (10), "P = Q / P̅ ̅=̅ ̅Q̅" (19) with the footnote "P̅ ̅=̅ ̅Q̅ for 'ALS520 and SN74ALS521", VCC (20). Pin 19 is P=Q active low here.
- **Pinouts:**

**p20** — DW, N: p.1 "SN74ALS518, SN74ALS520, SN74ALS521 . . . DW OR N PACKAGE". NS: not drawn on the sheet (p.1 lists DW, N, J, FK only); SN74ALS520NSR is listed on Addendum-Page 1 as SOP (NS) | 20 and takes the standard 20-lead order (same as DW).

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** p.1 function table (P̅ ̅=̅ ̅Q̅ column):

  | P, Q | G̅ | P̅ ̅=̅ ̅Q̅ |
  |---|---|---|
  | P = Q | L | L |
  | P > Q | L | H |
  | P < Q | L | H |
  | X | H | H |

  Model `compare`: `a` P0–P7, `b` Q0–Q7, `outputs` {eq: P=Q, low}, `enable` {G, low} (G̅ high → P=Q at its inactive level H). Totem-pole output (p.1 type table).
- **Timing:** ALS: rise 12 ns, fall 20 ns; vcc 4.5–5.5 V. P or Q to P=Q max (no typ printed), SN74ALS520 column, 50 pF, RL 500 Ω (sn74als518 p.7); VCC 4.5–5.5 V (p.6).
- **Supply:** SN74ALS520 4.5–5.5 V; SN54ALS520 also 4.5–5.5 V (p.6) — no `sn54vcc`.
- **Checks against the model:** exact for the logic. Not modeled: the 20-kΩ pull-up resistors on the Q inputs of the 'ALS520 (p.1 feature list and type table; p.7 "'ALS520 Q inputs" IIH −0.2 mA, IIL −0.6 mA): a floating Q input reads H on the real part but X in the model.

## 74_521 — 8-bit identity comparators

- **PDFs:** comparator/sn74als518.pdf (SDAS224B) p.1 (type table: SN74ALS521 no pull-ups, P̅=̅Q̅ totem pole, "identical to 'ALS688"; DW/N pinout; function table; read from the page image), p.2 (logic symbol), p.4 (logic diagram), p.6 (ROC), p.7 (electrical + switching), Addendum-Page 1 (SN74ALS521DWR, SN74ALS521N, SN74ALS521NSR SOP (NS) | 20, Last Time Buy); comparator/sn74f521.pdf (SDFS091) p.1 (printed 2−1: J, DW/N and FK pinouts, function table, logic symbol; read from the page image), p.2 (logic diagram), p.3 (printed 2−3: ROC, electrical), p.4 (printed 2−4: switching), Addendum-Page 1 (SN54F521J, SN74F521DWR, SN74F521N, SN74F521NSR SOP (NS), JM38510/34701BSA CFP (W) | 20, SNJ54F521J CDIP (J) | 20 on Addendum-Page 2).
- **Families:** ALS (SN74ALS521), F (SN74F521, SN54F521, SNJ54F521, JM38510/34701BSA = a 54F521 per its addendum row on the F521 sheet).
- **Pin names:** sn74als518 p.1: G̅ (1), P0–P7, Q0–Q7, GND (10), P̅ ̅=̅ ̅Q̅ for SN74ALS521 (19), VCC (20). sn74f521 p.1: O̅E̅ (1), same P/Q/GND, P̅ ̅=̅ ̅Q̅ (19), VCC. Checked pin by pin they match; O̅E̅ = G̅, named `G` here. The F sheet's function table (P ≠ Q, X → H; O̅E̅ H → H) shows O̅E̅ is an enable, not a 3-state control (no Z, no tPZ/tPLZ on p.4).
- **Pinouts:**

**p20** — DW, N: sn74als518 p.1 and sn74f521 p.1 ("DW OR N PACKAGE"); J: sn74f521 p.1 ("SN54F521 . . . J PACKAGE", same drawing). NS: not drawn on either sheet; SN74ALS521NSR and SN74F521NSR are SOP (NS) | 20 on the addenda, standard 20-lead order. W: not drawn; JM38510/34701BSA is CFP (W) | 20 on the sn74f521 addendum, taken as the standard 20-lead flatpack order (= DIP order).

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | P0 | input |
| 3 | Q0 | input |
| 4 | P1 | input |
| 5 | Q1 | input |
| 6 | P2 | input |
| 7 | Q2 | input |
| 8 | P3 | input |
| 9 | Q3 | input |
| 10 | GND | ground |
| 11 | P4 | input |
| 12 | Q4 | input |
| 13 | P5 | input |
| 14 | Q5 | input |
| 15 | P6 | input |
| 16 | Q6 | input |
| 17 | P7 | input |
| 18 | Q7 | input |
| 19 | P=Q | output |
| 20 | VCC | power |

- **Function:** sn74als518 p.1 (P̅=̅Q̅ column) and sn74f521 p.1 agree:

  | P, Q | G̅ / O̅E̅ | P̅ ̅=̅ ̅Q̅ |
  |---|---|---|
  | P = Q | L | L |
  | P ≠ Q (P > Q, P < Q) | L | H |
  | X | H | H |

  Model `compare`: `a` P0–P7, `b` Q0–Q7, `outputs` {eq: P=Q, low}, `enable` {G, low}. Totem-pole output.
- **Timing:**
  - ALS: rise 12 ns, fall 20 ns; vcc 4.5–5.5 V. P or Q to P=Q max (no typ printed), SN74ALS521 column, 50 pF (sn74als518 p.7); VCC (p.6).
  - F: rise 6.6 ns, fall 6.6 ns; vcc 4.5–5.5 V. P or Q to P=Q typ, 5 V 50 pF, 'F521 25 °C column (sn74f521 p.4); VCC (p.3).
- **Supply:** ALS SN74 4.5–5.5 V (sn74als518 p.6). F SN54F521 and SN74F521 both 4.5–5.5 V (sn74f521 p.3) — no `sn54vcc`.
- **Checks against the model:** exact. No input pull-ups on the '521 (sn74als518 p.1 type table "No"). The SN54F521 max times (14/12 ns) are wider than SN74F521 (11/11 ns), not modeled (typ used).

## 74_885 — 8-bit magnitude comparators

- **PDFs:** comparator/sn74as885.pdf (SDAS236A) p.1 (features incl. "Latchable P-Input Ports With Power-Up Clear"; JT/DW/NT pinout; FK pinout; description: latch transparent while PLE high, latched while PLE low; read from the page image for the L/A̅ bar), p.2 (function table, logic symbol), p.3 (logic diagram), p.4 (ROC, tsu/th to PLE↓, electrical), p.5 (switching), p.6–7 (cascading application), Addendum (SN74AS885DW, SNJ54AS885JT, SN54AS885JT, SNJ54AS885FK). sn54as885.pdf is the same file.
- **Families:** AS (SN74AS885, SN54AS885 / SNJ54AS885).
- **Pin names:** p.1 prints L/A̅ (1), P < QIN (2), P > QIN (3), Q7–Q0 (4–11), GND (12), P > QOUT (13), P < QOUT (14), P0–P7 (15–22), PLE (23), VCC (24). No other bars. Names used: `L/A`, `P<QIN`, `P>QIN`, `P>QOUT`, `P<QOUT`, `PLE`.
- **Pinouts:**

**p24** — DW, JT: p.1 "SN54AS885 . . . JT PACKAGE / SN74AS885 . . . DW OR NT PACKAGE" (one drawing). FK (28-pad) is not an add row.

| Pin | Name | Role |
|---|---|---|
| 1 | L/A | input |
| 2 | P<QIN | input |
| 3 | P>QIN | input |
| 4 | Q7 | input |
| 5 | Q6 | input |
| 6 | Q5 | input |
| 7 | Q4 | input |
| 8 | Q3 | input |
| 9 | Q2 | input |
| 10 | Q1 | input |
| 11 | Q0 | input |
| 12 | GND | ground |
| 13 | P>QOUT | output |
| 14 | P<QOUT | output |
| 15 | P0 | input |
| 16 | P1 | input |
| 17 | P2 | input |
| 18 | P3 | input |
| 19 | P4 | input |
| 20 | P5 | input |
| 21 | P6 | input |
| 22 | P7 | input |
| 23 | PLE | input |
| 24 | VCC | power |

- **Function:** p.2 function table:

  | Comparison | L/A̅ | Data | P > QIN | P < QIN | P > QOUT | P < QOUT |
  |---|---|---|---|---|---|---|
  | Logical | H | P > Q | X | X | H | L |
  | Logical | H | P < Q | X | X | L | H |
  | Logical† | H | P = Q | H or L | H or L | H or L | H or L |
  | Arithmetic | L | P AG Q | X | X | H | L |
  | Arithmetic | L | Q AG P | X | X | L | H |
  | Arithmetic† | L | P = Q | H or L | H or L | H or L | H or L |

  † P > QOUT follows P > QIN and P < QOUT follows P < QIN. AG = arithmetically greater than.

  Model `compare`: `a` P0–P7 (the latched word), `b` Q0–Q7, `outputs` {gt: P>QOUT high, lt: P<QOUT high}, `cascade` {gt: P>QIN, lt: P<QIN}, no `cascadeTable` (each output follows its same-named input on equal words, as the footnote says; so both cascade inputs H give both outputs H, both L give both L, exactly as "follows" states). `latch` {PLE, low}: p.1 "transparent when PLE is high; the P-input port is latched when PLE is low" (setup/hold to PLE↓ on p.4). `signed` {L/A, low}: L/A̅ low = arithmetic (two's complement), p.2 and p.1. Unequal words: gt only / lt only, cascade ignored — matches the model.
- **Power-up clear:** claimed on p.1 ("Latchable P-Input Ports With Power-Up Clear"); the p.2 logic symbol marks the first latch cell "1=0"; no further detail (no clear pin, no timing). Consistent with the model's latch holding 0 at power-on.
- **Timing:** AS: rise 13.5 ns, fall 10 ns; vcc 4.5–5.5 V. Any P or Q data input to P<QOUT, P>QOUT typ, 5 V 50 pF RL 500 Ω (sn74as885 p.5; max SN74 17.5/15, SN54 21/17); VCC (p.4). Other typ paths: L/A̅ → out 8.5/7.5 ns, cascade in → out 5/5.5 ns (p.5); no PLE → out delay printed.
- **Supply:** SN74AS885 and SN54AS885 both 4.5–5.5 V (p.4) — no `sn54vcc`.
- **Checks against the model:** exact for the function table. Notes: the model uses the single data-path delay; the faster cascade path (5/5.5 ns typ) and L/A̅ path are not separately modeled. Power-on state of the P latch = 0 matches the sheet's power-up clear claim.

## 74_280 — 9-bit odd/even parity generators/checkers

- **PDFs:** parity/sn74ls280.pdf p.1 (pinout "SN54LS280, SN54S280 … J OR W PACKAGE / SN74LS280, SN74S280 … D OR N PACKAGE", FK; function table; logic symbol "pin numbers shown are for D, J, N, and W packages"), p.3 (LS ROC + timing), p.4 (S ROC + timing), p.5 (logic diagram), addendum (JM38510/32901BCA, SN54LS280J, SN54S280J, SN74LS280N/NSR, SNJ54LS280J/W, SNJ54S280J/W); scanned, read from the page images. parity/sn74as280.pdf p.1 (D or N pinout, function table, logic symbol), p.4 (AS ROC + timing), addendum (SN74AS280N, SN74AS280NSR). parity/sn74f280b.pdf p.1 (J, D or N pinout, FK, function table), p.3 (ROC), p.4 (timing). parity/cd74hc280.pdf p.1 (pinout CERDIP/PDIP/SOIC, ordering, description), p.2 (operating conditions), p.3 (HC timing), p.4 (HCT timing). parity/cd74ac280.pdf p.1 (pinout CERDIP/PDIP/SOIC, features, description, ordering), p.2 (operating conditions), p.4 (timing).
- **Families:** LS (SN74LS280, SN54LS280, SNJ54LS280, JM38510/32901BCA = 54LS280 per the sn74ls280 addendum), S (SN54S280, SNJ54S280), AS (SN74AS280), F (SN74F280B), HC (CD74HC280), HCT (CD74HCT280), AC (CD74AC280), ACT (CD74ACT280).
- **Pin names:** SN sheets (sn74ls280 p.1, sn74as280 p.1, sn74f280b p.1) print A–I, NC, ΣEVEN (Σ EVEN), ΣODD (Σ ODD), GND, VCC. Model names drop the Σ: `EVEN`, `ODD` (ASCII, like dropped overbars). The CD sheets (cd74hc280 p.1, cd74ac280 p.1) print I0–I8, ΣE, ΣO; checked pin by pin: I0 (8) = A, I1 (9) = B, I2 (10) = C, I3 (11) = D, I4 (12) = E, I5 (13) = F, I6 (1) = G, I7 (2) = H, I8 (4) = I, ΣE (5) = ΣEVEN, ΣO (6) = ΣODD, NC 3, GND 7, VCC 14. No overbars anywhere (both outputs true-sense; every input enters symmetrically). Note the input order: G, H on pins 1–2, NC on pin 3, I on pin 4 (the '180 expander pin position), A–F on 8–13.
- **Pinouts:**

**p14** — N, D, NS, J, W. Drawings: sn74ls280 p.1 (J or W for SN54LS/S280, D or N for SN74LS/S280: one drawing, so the W flat pack has the same pinout as J); sn74as280 p.1 (D or N); sn74f280b p.1 (J, D or N); cd74hc280 p.1 (CERDIP, PDIP, SOIC = J, N, D); cd74ac280 p.1 (CERDIP, PDIP, SOIC). NS (SN74LS280NSR, SN74AS280NSR, SN74F280BNSR) is not drawn on its sheet but is the standard 14-lead order. FK (sn74ls280 p.1, sn74f280b p.1) has no add row, so no fk20 pinout.

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | H | input |
| 3 | NC | nc |
| 4 | I | input |
| 5 | EVEN | output |
| 6 | ODD | output |
| 7 | GND | ground |
| 8 | A | input |
| 9 | B | input |
| 10 | C | input |
| 11 | D | input |
| 12 | E | input |
| 13 | F | input |
| 14 | VCC | power |

- **Function:** sn74ls280 p.1, sn74as280 p.1, sn74f280b p.1 (same table); the CD sheets state it in the p.1 description (ΣE high when an even number of inputs is high, ΣO high when odd):

  | Number of inputs A–I high | ΣEVEN | ΣODD |
  |---|---|---|
  | 0, 2, 4, 6, 8 | H | L |
  | 1, 3, 5, 7, 9 | L | H |

  Model `parity`: `inputs` [A … I], `even` EVEN, `odd` ODD. No bus, no error.

- **Timing:** (main path data → ΣEVEN; rise = tPLH, fall = tPHL)

- LS: rise 33 ns, fall 29 ns; vcc 4.75–5.25 V. Data to ΣEVEN typ, 5 V 15 pF (ΣODD 23/31); SN54 4.5–5.5 V (sn74ls280 p.3)
- S: rise 14 ns, fall 11.5 ns; vcc 4.75–5.25 V. Data to ΣEVEN typ, 5 V 15 pF (ΣODD same); SN54 4.5–5.5 V (sn74ls280 p.4)
- AS: rise 12 ns, fall 11 ns; vcc 4.5–5.5 V. Any input to ΣEVEN max (no typ printed), 50 pF (ΣODD 12/11.5) (sn74as280 p.4)
- F: rise 6.1 ns, fall 6.6 ns; vcc 4.5–5.5 V. Any input to ΣEVEN typ, 5 V 50 pF (sn74f280b p.4, p.3)
- HC: rise 17 ns, fall 17 ns; vcc 2–6 V. Any input to ΣE typ, 5 V 15 pF (the only typ printed; 50 pF max 40 ns at 4.5 V) (cd74hc280 p.3, p.2)
- HCT: rise 18 ns, fall 18 ns; vcc 4.5–5.5 V. Any input to ΣE typ, 5 V 15 pF (ΣO 19; 50 pF max 42 ns at 4.5 V) (cd74hc280 p.4, p.2)
- AC: rise 10 ns, fall 10 ns; vcc 1.5–5.5 V. Typ 10 ns at 5 V 50 pF from the p.1 feature list; the switching table prints only max (any input to ΣE 18.2 ns, −40–85 °C, 5 V) (cd74ac280 p.1, p.4, p.2)
- ACT: rise 10 ns, fall 10 ns; vcc 4.5–5.5 V. Same p.1 feature typ (the feature list covers 'AC280 and 'ACT280); table max any input to ΣE 19.6 ns (cd74ac280 p.1, p.4, p.2)

- **Supply:** LS and S SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls280 p.3, p.4) — `sn54vcc` LS, S [4.5, 5.5] (SN54LS280J, SNJ54LS280J/W, JM38510/32901BCA, SN54S280J, SNJ54S280W). AS 4.5–5.5 V (sn74as280 p.4). F 4.5–5.5 V, SN54 same (sn74f280b p.3). HC 2–6 V, HCT 4.5–5.5 V (cd74hc280 p.2). AC 1.5–5.5 V, ACT 4.5–5.5 V (cd74ac280 p.2).
- **Checks against the model:** exact. Notes: LS/S typ only at 15 pF; LS ΣEVEN and ΣODD delays differ (33/29 vs 23/31), the model takes ΣEVEN. AS prints only max. AC/ACT typ comes from the p.1 feature bullet (one figure for both families); the table has only max. The sn74as280 sheet also covers the SN74ALS280 (no add row).

## 74_286 — 9-bit parity generators/checkers with bus-driver parity I/O port

- **PDFs:** parity/sn74as286.pdf p.1 (pinout "SN54AS286 … J / SN74AS286 … D OR N", FK; description; function table), p.2 (logic symbol, logic diagram; "pin numbers shown are for the D, J, and N packages"), p.3 (ROC, electrical characteristics), p.4 (switching characteristics), addendum (SN74AS286D). Overbars read from the page images.
- **Families:** AS (SN74AS286).
- **Pin names:** A–I, XMIT (printed X̅M̅I̅T̅, active low), PARITY ERROR (no overbar printed; active low per the function table), PARITY I/O, GND, VCC (p.1 pinout, p.2 symbol and diagram agree).
- **Pinouts:**

**p14** — D (sn74as286 p.1 "SN74AS286 … D OR N PACKAGE"; same drawing for J/N, which have no add row).

| Pin | Name | Role |
|---|---|---|
| 1 | G | input |
| 2 | H | input |
| 3 | XMIT | input |
| 4 | I | input |
| 5 | PARITY ERROR | output |
| 6 | PARITY I/O | inout |
| 7 | GND | ground |
| 8 | A | input |
| 9 | B | input |
| 10 | C | input |
| 11 | D | input |
| 12 | E | input |
| 13 | F | input |
| 14 | VCC | power |

- **Function:** sn74as286 p.1 (h/l = input level, H/L = output level):

  | Inputs A–I high | X̅M̅I̅T̅ | PARITY I/O | PARITY ERROR |
  |---|---|---|---|
  | 0, 2, 4, 6, 8 | l | H (output) | H |
  | 1, 3, 5, 7, 9 | l | L (output) | H |
  | 0, 2, 4, 6, 8 | h | h | H |
  | 0, 2, 4, 6, 8 | h | l | L |
  | 1, 3, 5, 7, 9 | h | h | L |
  | 1, 3, 5, 7, 9 | h | l | H |

  Model `parity`: `inputs` [A … I]; `bus` {pin PARITY I/O, xmit {XMIT, low}, drives "even"}; `error` {PARITY ERROR, low}; `errorOn` "even".
  - `xmit` low: the X̅M̅I̅T̅ = l rows show PARITY I/O as an output level (H/L), the h rows as an input level; the logic symbol (p.2) has X̅M̅I̅T̅ → EN1 through a negation and PARITY I/O as 1▽ (3-state, enabled by EN1).
  - `drives` "even": with X̅M̅I̅T̅ low, PARITY I/O = H for 0, 2, 4, 6, 8 high inputs, L for an odd count, i.e. the ΣEVEN value.
  - `error` active low and `errorOn` "even": with X̅M̅I̅T̅ high, PARITY ERROR is L for (even count, I/O l) and (odd count, I/O h), i.e. when the H count of A–I plus PARITY I/O is even; H otherwise. While transmitting PARITY ERROR is H (inactive) in both rows, as the model says. The p.1 description says the same in words.
  - PARITY ERROR is totem-pole: p.3 gives it VOH ≥ VCC − 2 V at IOH −2 mA and 2.4 V at −3 mA (high-level drive), and the p.2 symbol marks no open-circuit output; so no `openDrain`. PARITY I/O is a 3-state 48 mA bus driver (IOL 48 mA SN74).
  - The p.1 description says "When X̅M̅I̅T̅ is low, the parity tree is disabled"; it means the checking path (PARITY ERROR held high). The function table and the p.2 diagram show the tree still drives PARITY I/O while X̅M̅I̅T̅ is low, so the table is followed.
- **Timing:** (main path A–I → PARITY I/O; rise = tPLH, fall = tPHL)

- AS: rise 15 ns, fall 14 ns; vcc 4.5–5.5 V. Any A–I to PARITY I/O max (no typ printed), SN74AS286, 4.5–5.5 V 50 pF (sn74as286 p.4, ROC p.3). Others (max, SN74): A–I → PARITY ERROR 16.5/16.5, PARITY I/O → PARITY ERROR 9/9, X̅M̅I̅T̅ → PARITY I/O tPZH 13, tPZL 16, tPHZ 11.5, tPLZ 10.

- **Supply:** 4.5–5.5 V, SN54 the same (sn74as286 p.3); no `sn54vcc`.
- **Checks against the model:** exact. The power-up/down high-impedance behaviour of PARITY I/O (p.1) is outside the model.

## 74_11286 — 9-bit parity generator/checker with bus driver parity I/O ports

- **PDFs:** parity/74act11286.pdf p.1 (pinout "D OR N PACKAGE", description, function table), p.2 (logic symbol, logic diagram), p.3 (ROC), p.4 (electrical and switching characteristics), addendum (74ACT11286D). Overbars read from the page images.
- **Families:** ACT (74ACT11286).
- **Pin names:** A–I, XMIT (printed X̅M̅I̅T̅, active low), PARITY ERROR (no overbar; active low per the function table), PARITY I/O, GND, VCC (p.1 pinout and the p.2 symbol/diagram pin numbers agree: A 2, B 1, C 14, D 13, E 12, F 10, G 9, H 8, I 7, X̅M̅I̅T̅ 6, PARITY I/O 3, PARITY ERROR 5). EPIC centre-pin power: GND 4, VCC 11.
- **Pinouts:**

**p14** — D (74act11286 p.1 "D OR N PACKAGE"; N has no add row).

| Pin | Name | Role |
|---|---|---|
| 1 | B | input |
| 2 | A | input |
| 3 | PARITY I/O | inout |
| 4 | GND | ground |
| 5 | PARITY ERROR | output |
| 6 | XMIT | input |
| 7 | I | input |
| 8 | H | input |
| 9 | G | input |
| 10 | F | input |
| 11 | VCC | power |
| 12 | E | input |
| 13 | D | input |
| 14 | C | input |

- **Function:** 74act11286 p.1 — identical to the SN74AS286 table (sn74as286 p.1), row for row:

  | Inputs A–I high | X̅M̅I̅T̅ | PARITY I/O | PARITY ERROR |
  |---|---|---|---|
  | 0, 2, 4, 6, 8 | l | H (output) | H |
  | 1, 3, 5, 7, 9 | l | L (output) | H |
  | 0, 2, 4, 6, 8 | h | h | H |
  | 0, 2, 4, 6, 8 | h | l | L |
  | 1, 3, 5, 7, 9 | h | h | L |
  | 1, 3, 5, 7, 9 | h | l | H |

  Model `parity`: `inputs` [A … I]; `bus` {pin PARITY I/O, xmit {XMIT, low}, drives "even"}; `error` {PARITY ERROR, low}; `errorOn` "even". Derivation as in § 74_286: X̅M̅I̅T̅ low rows drive PARITY I/O (H for an even count → `drives` "even"; symbol p.2: X̅M̅I̅T̅ → EN1, PARITY I/O 1▽); X̅M̅I̅T̅ high rows give PARITY ERROR L when the H count of A–I plus PARITY I/O is even (→ active low, `errorOn` "even"), H while transmitting. PARITY ERROR is a push-pull CMOS output (p.4 VOH/VOL rows at ±24 mA cover all outputs; IOZ is listed for PARITY I/O only), so no `openDrain`. The p.1 description's "parity tree is disabled" while X̅M̅I̅T̅ is low again means the error path; the table is followed.
- **Timing:** (main path A–I → PARITY I/O; rise = tPLH, fall = tPHL)

- ACT: rise 6.1 ns, fall 7.3 ns; vcc 4.5–5.5 V. Any A–I to PARITY I/O typ, TA = 25 °C, CL 50 pF (max 9/10.8 at 25 °C, 10.4/12 over range) (74act11286 p.4; ROC p.3). Others typ: A–I → PARITY ERROR 6.9/7.7, PARITY I/O → PARITY ERROR 4.6/5.6.

- **Supply:** 4.5–5.5 V, inputs 0–VCC (74act11286 p.3).
- **Checks against the model:** exact; function table matches the '286 exactly. Only the pinout differs (EPIC end/centre-pin layout).

## 74_283 — 4-bit binary full adders with fast carry

- **PDFs:** arith/sn74ls283.pdf (SDLS095A, scanned; read from the page images) p.1 (pinouts J/W, N, D; FK; function table), p.2 (logic symbol and logic diagram, "pin numbers shown are for D, J, N, and W packages"), p.3 ('283 standard TTL ROC + timing), p.4 (LS ROC + timing), p.5 (S ROC + timing), addendum (ordering: SN54LS283J, SN74LS283D, SN74LS283N, SN74LS283NSR, SN74S283N, SNJ54S283J, …); arith/sn74f283.pdf (SDFS069A) p.1 (printed 2-1: pinouts J, D/N; FK; logic symbol), p.2 (2-2: logic diagram), p.3 (2-3: function table), p.4 (2-4: ROC, timing), addendum (SN54F283J, SN74F283D, SN74F283N, SNJ54F283J); arith/cd74hc283.pdf (SCHS176E) p.1 (HC 2–6 V, HCT 4.5–5.5 V), p.3 (pinout J/N/D), p.4 (ROC), p.6 (timing), p.9 (functional block diagram), addendum (CD74HC283E, CD74HC283M96, CD74HCT283E, CD74HCT283M96); arith/cd74ac283.pdf (SCHS251E) p.3 (pinout Fig 3-1 J/D/N, Pin Functions table 3-1), p.4 (ROC), p.7 (timing), p.9 (functional block diagram), addendum (CD74AC283E, CD74ACT283E, CD74ACT283M).
- **Families:** TTL standard '283 is on sn74ls283 but has no add row (SN74283N skipped). LS (SN54LS283J, SN74LS283D/N/NSR — sn74ls283), S (SN74S283N, SNJ54S283J — sn74ls283), F (SN54F283J, SNJ54F283J, SN74F283D/N — sn74f283), HC (CD74HC283E, CD74HC283M96 — cd74hc283), HCT (CD74HCT283E, CD74HCT283M96 — cd74hc283), AC (CD74AC283E — cd74ac283), ACT (CD74ACT283E, CD74ACT283M — cd74ac283).
- **Pin names:** spec names `A1`–`A4`, `B1`–`B4`, `S1`–`S4` (sum), `C0` (carry in), `C4` (carry out), GND, VCC — the SN numbering with Σ spelled `S`. What each sheet prints, checked pin by pin:
  - sn74ls283 p.1 and sn74f283 p.1: Σ2 (1), B2 (2), A2 (3), Σ1 (4), A1 (5), B1 (6), C0 (7), GND (8), C4 (9), Σ4 (10), B4 (11), A4 (12), Σ3 (13), A3 (14), B3 (15), VCC (16). Σn = `Sn`. No overbars anywhere (true-form logic, p.1 "implemented in its true form").
  - cd74hc283 p.3 and cd74ac283 p.3: S1 (1), B1 (2), A1 (3), S0 (4), A0 (5), B0 (6), CIN (7), GND (8), COUT (9), S3 (10), B3 (11), A3 (12), S2 (13), A2 (14), B2 (15), VCC (16). Bits numbered 0–3: CD `An`/`Bn`/`Sn` = spec `A(n+1)`/`B(n+1)`/`S(n+1)`, CIN = `C0`, COUT = `C4`. Same function on every pin (pin 5 is the LSB A input on every sheet: SN A1, CD A0; the p.9 block diagrams confirm A0 5, B0 6, S0 4, CIN 7, COUT 9).
  - Note the clash: CD `S1` (pin 1) is the second sum bit, spec `S2`; CD `S0` (pin 4) is spec `S1`. Map by pin number, not by name.
  - cd74ac283 p.3 Table 3-1 prints VCC as pin "14" (typo); the Fig 3-1 drawing and the p.9 block diagram ("VCC = 16") give 16.
- **Pinouts:**

**p16** — N, D, J, NS (pin n = DIP pin n). Drawings: sn74ls283 p.1 ("SN54283, SN54LS283 … J or W; SN54S283 … J; SN74283 … N; SN74LS283, SN74S283 … D or N"); sn74f283 p.1 ("SN54F283 … J; SN74F283 … D or N"); cd74hc283 p.3 and cd74ac283 p.3 ("J, N, or D package"). NS (SN74LS283NSR) is not drawn on the sheet (listed only in the addendum, SOP (NS) 16); standard 16-lead order assumed, same as D. FK (SNJ54LS283FK, SNJ54F283FK; drawn on sn74ls283 p.1 and sn74f283 p.1, same pad map) has no add row and is not mapped.

| Pin | Name | Role |
|---|---|---|
| 1 | S2 | output |
| 2 | B2 | input |
| 3 | A2 | input |
| 4 | S1 | output |
| 5 | A1 | input |
| 6 | B1 | input |
| 7 | C0 | input |
| 8 | GND | ground |
| 9 | C4 | output |
| 10 | S4 | output |
| 11 | B4 | input |
| 12 | A4 | input |
| 13 | S3 | output |
| 14 | A3 | input |
| 15 | B3 | input |
| 16 | VCC | power |

- **Function:** sn74ls283 p.1 and sn74f283 p.3 print the same 2-bit table (rows read A1 B1 A2 B2 → Σ1 Σ2 C2, and identically A3 B3 A4 B4 → Σ3 Σ4 C4 with C2 as the carry in); checked row by row, both sheets agree:

  | A1/A3 | B1/B3 | A2/A4 | B2/B4 | C0=L: Σ1/Σ3 | Σ2/Σ4 | C2/C4 | C0=H: Σ1/Σ3 | Σ2/Σ4 | C2/C4 |
  |---|---|---|---|---|---|---|---|---|---|
  | L | L | L | L | L | L | L | H | L | L |
  | H | L | L | L | H | L | L | L | H | L |
  | L | H | L | L | H | L | L | L | H | L |
  | H | H | L | L | L | H | L | H | H | L |
  | L | L | H | L | L | H | L | H | H | L |
  | H | L | H | L | H | H | L | L | L | H |
  | L | H | H | L | H | H | L | L | L | H |
  | H | H | H | L | L | L | H | H | L | H |
  | L | L | L | H | L | H | L | H | H | L |
  | H | L | L | H | H | H | L | L | L | H |
  | L | H | L | H | H | H | L | L | L | H |
  | H | H | L | H | L | L | H | H | L | H |
  | L | L | H | H | L | L | H | H | L | H |
  | H | L | H | H | H | L | H | L | H | H |
  | L | H | H | H | H | L | H | L | H | H |
  | H | H | H | H | L | H | H | H | H | H |

  (For Σ3/Σ4/C4 the "C0" column heads mean C2 = L / C2 = H.) The CD sheets print no table; cd74hc283 p.9 and cd74ac283 p.9 say "add two 4-bit binary numbers and generate a carry-out bit if the sum exceeds 15". Every row is Σ = A + B + Cin (A1, B1 weight 1; A2, B2 weight 2; carry weight 4). Model `adder`: `a` [A1, A2, A3, A4], `b` [B1, B2, B3, B4], `sum` [S1, S2, S3, S4], `cin` C0, `cout` C4, all active-high.

- **Timing:** (main path A/B → Σ; rise = tPLH, fall = tPHL)

- LS: rise 15 ns, fall 15 ns; vcc 4.75–5.25 V. Ai or Bi to Σi typ, 5 V 15 pF; SN54 4.5–5.5 V (sn74ls283 p.4)
- S: rise 12 ns, fall 11.5 ns; vcc 4.75–5.25 V. Ai or Bi to Σi typ, 5 V 15 pF; SN54 4.5–5.5 V (sn74ls283 p.5)
- F: rise 6.6 ns, fall 6.6 ns; vcc 4.5–5.5 V. A or B to Σ typ, 5 V 50 pF; SN54 same (sn74f283 p.4, printed 2-4)
- HC: rise 18 ns, fall 18 ns; vcc 2–6 V. An, Bn to Sn typ at 5 V 15 pF (no 50 pF typ printed; 4.5 V 50 pF max 42 ns) (cd74hc283 p.6, ROC p.4)
- HCT: rise 21 ns, fall 21 ns; vcc 4.5–5.5 V. An, Bn to Sn typ at 5 V 15 pF (4.5 V 50 pF max 49 ns) (cd74hc283 p.6; supply p.1 — the p.4 ROC lists only HC types)
- AC: rise 16.5 ns, fall 16.5 ns; vcc 1.5–5.5 V. An or Bn to Sn max (no typ printed), "5 V" column (max at 4.5 V), 50 pF, −40 to 85 °C (cd74ac283 p.7, ROC p.4)
- ACT: rise 16.5 ns, fall 16.5 ns; vcc 4.5–5.5 V. same row, ACT types (cd74ac283 p.7, ROC p.4)

- **Supply:** LS and S: SN74 4.75–5.25 V, SN54 4.5–5.5 V (sn74ls283 p.4, p.5) — `sn54vcc` LS, S [4.5, 5.5] (SN54LS283J, SNJ54S283J). F: SN54 and SN74 both 4.5–5.5 V (sn74f283 p.4). HC 2–6 V (cd74hc283 p.4, p.1), HCT 4.5–5.5 V (p.1). AC 1.5–5.5 V, ACT 4.5–5.5 V (cd74ac283 p.4). CD54/CD74 parts share one range. No `partVcc`.
- **Checks against the model:** exact. The '283 is a pure 4-bit adder with carry in/out; every pin is used. LS/S print typ only at 15 pF; HC/HCT typ only at 5 V 15 pF; AC/ACT print only max. All four sheets note the adder also works on all-active-low operands (same function by symmetry) — no model impact.

## 74_181 — Arithmetic logic units/function generators

- **PDFs:** arith/sn54ls181.pdf (SDLS136, scanned; read from the page images) p.1 (pinout J/W, DW/N; FK), p.2 (pin-designation table active-low vs active-high data; A = B and Cn+4 comparator use), p.3 (Figure 1 logic symbol + Table 1, active-low data), p.4 (Figure 2 logic symbol + Table 2, active-high data), p.5 (logic diagram, "pin numbers shown are for DW, J, N, and W packages"), p.6 (LS absolute max, ROC, electrical), p.7 (LS switching, output schematics incl. open-collector A = B), addendum (SN54LS181J, SNJ54LS181J — CDIP (J) 24).
- **Families:** LS only among add rows (SNJ54LS181J). The sheet also covers SN54S181/SN74S181 and SN74LS181 (no add rows).
- **Pin names:** the p.1 pinout prints the active-low data convention (Table 1 / Figure 1): B̅0 (1), A̅0 (2), S3 (3), S2 (4), S1 (5), S0 (6), Cn (7), M (8), F̅0 (9), F̅1 (10), F̅2 (11), GND (12), F̅3 (13), A = B (14), P̅ (15), Cn+4 (16), G̅ (17), B̅3 (18), A̅3 (19), B̅2 (20), A̅2 (21), B̅1 (22), A̅1 (23), VCC (24). The p.2 table re-designates the same pin numbers for active-high data (Table 2 / Figure 2): A0 (2), B0 (1), A1 (23), B1 (22), A2 (21), B2 (20), A3 (19), B3 (18), F0 (9), F1 (10), F2 (11), F3 (13), C̅n (7), C̅n+4 (16), X (15), Y (17). Spec names drop the bars: `A0`–`A3`, `B0`–`B3`, `S0`–`S3`, `M`, `Cn`, `F0`–`F3`, `A=B`, `P` (pin 15, P̅ / X), `Cn+4` (pin 16), `G` (pin 17, G̅ / Y). The pin numbers are identical in both conventions; only the names (bars) change. The model reads the pins as **active-high data, the sheet's Table 2 / Figure 2** (note: the sheet numbers the active-high table *Table 2*, not Table 1): A, B, F active-high; Cn (C̅n) and Cn+4 (C̅n+4) active-low carries (C̅n = L means "with carry"); P (X) and G (Y) are the active-low group propagate/generate (the '182 P̅/G̅ inputs). Checked pin by pin against the p.5 logic diagram ("B̅3 or B3 (18)", "A̅3 or A3 (19)", … "Cn or C̅n (7)", "M (8)", "F̅0 or F0 (9)", "A = B (14)", "P̅ or X (15)", "Cn+4 or C̅n+4 (16)", "G̅ or Y (17)") and the p.3/p.4 logic symbols.
- **Pinouts:**

**p24** — J (24-pin CDIP). Drawing: sn54ls181 p.1 "SN54LS181, SN54S181 … J or W package; SN74LS181, SN74S181 … DW or N package (top view)". FK (28-pad) is drawn on p.1 but has no add row.

| Pin | Name | Role |
|---|---|---|
| 1 | B0 | input |
| 2 | A0 | input |
| 3 | S3 | input |
| 4 | S2 | input |
| 5 | S1 | input |
| 6 | S0 | input |
| 7 | Cn | input |
| 8 | M | input |
| 9 | F0 | output |
| 10 | F1 | output |
| 11 | F2 | output |
| 12 | GND | ground |
| 13 | F3 | output |
| 14 | A=B | output (open collector) |
| 15 | P | output |
| 16 | Cn+4 | output |
| 17 | G | output |
| 18 | B3 | input |
| 19 | A3 | input |
| 20 | B2 | input |
| 21 | A2 | input |
| 22 | B1 | input |
| 23 | A1 | input |
| 24 | VCC | power |

- **Function:** both tables transcribed from the 300-dpi page images (each of the 32 rows checked twice). Notation: `~X` = X with an overbar; `~(…)` = overbar over the whole expression; `+` = OR, juxtaposition = AND, `⊕` = XOR; "PLUS"/"MINUS" are arithmetic. ‡/† "each bit is shifted to the next more significant position".

  **Table 2 — active-high data (sn54ls181 p.4; the model's convention).** M = L columns are headed C̅n = H (no carry) and C̅n = L (with carry).

  | S3 | S2 | S1 | S0 | M = H logic | M = L, C̅n = H (no carry) | M = L, C̅n = L (with carry) |
  |---|---|---|---|---|---|---|
  | L | L | L | L | F = ~A | F = A | F = A PLUS 1 |
  | L | L | L | H | F = ~(A + B) | F = A + B | F = (A + B) PLUS 1 |
  | L | L | H | L | F = ~A B | F = A + ~B | F = (A + ~B) PLUS 1 |
  | L | L | H | H | F = 0 | F = MINUS 1 (2's COMPL) | F = ZERO |
  | L | H | L | L | F = ~(A B) | F = A PLUS A~B | F = A PLUS A~B PLUS 1 |
  | L | H | L | H | F = ~B | F = (A + B) PLUS A~B | F = (A + B) PLUS A~B PLUS 1 |
  | L | H | H | L | F = A ⊕ B | F = A MINUS B MINUS 1 | F = A MINUS B |
  | L | H | H | H | F = A~B | F = A~B MINUS 1 | F = A~B |
  | H | L | L | L | F = ~A + B | F = A PLUS AB | F = A PLUS AB PLUS 1 |
  | H | L | L | H | F = ~(A ⊕ B) | F = A PLUS B | F = A PLUS B PLUS 1 |
  | H | L | H | L | F = B | F = (A + ~B) PLUS AB | F = (A + ~B) PLUS AB PLUS 1 |
  | H | L | H | H | F = AB | F = AB MINUS 1 | F = AB |
  | H | H | L | L | F = 1 | F = A PLUS A † | F = A PLUS A PLUS 1 |
  | H | H | L | H | F = A + ~B | F = (A + B) PLUS A | F = (A + B) PLUS A PLUS 1 |
  | H | H | H | L | F = A + B | F = (A + ~B) PLUS A | F = (A + ~B) PLUS A PLUS 1 |
  | H | H | H | H | F = A | F = A MINUS 1 | F = A |

  **Table 1 — active-low data (sn54ls181 p.3).** Same pins read with the p.1 names (A̅, B̅, F̅ active-low, Cn and Cn+4 active-high carries). M = L columns are headed Cn = L (no carry) and Cn = H (with carry).

  | S3 | S2 | S1 | S0 | M = H logic | M = L, Cn = L (no carry) | M = L, Cn = H (with carry) |
  |---|---|---|---|---|---|---|
  | L | L | L | L | F = ~A | F = A MINUS 1 | F = A |
  | L | L | L | H | F = ~(AB) | F = AB MINUS 1 | F = AB |
  | L | L | H | L | F = ~A + B | F = A~B MINUS 1 | F = A~B |
  | L | L | H | H | F = 1 | F = MINUS 1 (2's COMP) | F = ZERO |
  | L | H | L | L | F = ~(A + B) | F = A PLUS (A + ~B) | F = A PLUS (A + ~B) PLUS 1 |
  | L | H | L | H | F = ~B | F = AB PLUS (A + ~B) | F = AB PLUS (A + ~B) PLUS 1 |
  | L | H | H | L | F = ~(A ⊕ B) | F = A MINUS B MINUS 1 | F = A MINUS B |
  | L | H | H | H | F = A + ~B | F = A + ~B | F = (A + ~B) PLUS 1 |
  | H | L | L | L | F = ~A B | F = A PLUS (A + B) | F = A PLUS (A + B) PLUS 1 |
  | H | L | L | H | F = A ⊕ B | F = A PLUS B | F = A PLUS B PLUS 1 |
  | H | L | H | L | F = B | F = A~B PLUS (A + B) | F = A~B PLUS (A + B) PLUS 1 |
  | H | L | H | H | F = A + B | F = (A + B) | F = (A + B) PLUS 1 |
  | H | H | L | L | F = 0 | F = A PLUS A ‡ | F = A PLUS A PLUS 1 |
  | H | H | L | H | F = A~B | F = AB PLUS A | F = AB PLUS A PLUS 1 |
  | H | H | H | L | F = AB | F = A~B PLUS A | F = A~B PLUS A PLUS 1 |
  | H | H | H | H | F = A | F = A | F = A PLUS 1 |

  Comparator use (p.2): A = B (open collector) goes high when all four F outputs are high (p.5 logic diagram: an AND of F0–F3); with the ALU in subtract mode (S3..S0 = L H H L, M = L) and Cn = H (active-low data convention) it signals equal words. Cn+4 table (p.2): Cn = H, Cn+4 = H → active-low A ≥ B / active-high A < B; H, L → A < B / A > B; L, H → A > B / A < B; L, L → A ≤ B / A ≥ B.

  Model `alu181`: `a` [A0, A1, A2, A3], `b` [B0, B1, B2, B3], `s` [S0, S1, S2, S3], `m` M, `cn` Cn, `f` [F0, F1, F2, F3], `cn4` Cn+4, `p` P, `g` G, `aeqb` A=B; `openDrain` [A=B]. Pin names assume active-high data (Table 2, p.4).

- **Timing:** (main path A/B → F; rise = tPLH, fall = tPHL)

- LS: rise 21 ns, fall 13 ns; vcc 4.75–5.25 V. A̅i or B̅i to F̅i typ, SUM mode (M = 0 V, S0 = S3 = 4.5 V, S1 = S2 = 0 V), 5 V 15 pF, RL 2 kΩ (sn54ls181 p.7); DIFF mode 21/21 ns, logic mode (M = 4.5 V) 22/26 ns. VCC SN74 4.75–5.25 V, SN54 4.5–5.5 V (p.6).

- **Supply:** SN54LS181 4.5–5.5 V, SN74LS181 4.75–5.25 V (sn54ls181 p.6) — `vcc` LS [4.75, 5.25], `sn54vcc` LS [4.5, 5.5]; the only add part (SNJ54LS181J) is SN54, so it runs on the sn54vcc range. No `partVcc`.
- **Checks against the model:** exact as far as the pin map goes — every pin is a model key and A = B is the one open-collector output (`openDrain`; p.6: IOH 100 µA max at VOH 5.5 V for A = B only; p.7 schematic). The model is the '181 circuit (p.5 logic diagram), so both tables follow from the same gate network; the transcription above is the reference for the model tests (Table 2 for active-high pin semantics). Note for the caller: the brief called the active-high table "Table 1"; on this sheet the active-high data table is **Table 2** (p.4) and Table 1 (p.3) is the active-low one. LS typ is at 15 pF only.

## 74_275 — 7-bit-slice Wallace trees with 3-state outputs

- **PDFs:** arith/snx4ls27x.pdf — a 10-page scan of TI bulletin DL-S 7612121 (October 1976, 'LS275 pages revised August 1977) covering SN54/74S274, SN54/74S275 and SN54/74LS275; image only, read from the page images. p.1 (features, description, pinouts: "SN54LS275, SN54S275 . . . J PACKAGE; SN74LS275, SN74S275 . . . J OR N PACKAGE"), p.2 (functional block diagram with 'LS275 and 'S275 pin numbers; input/output schematics), p.3 ('LS275 recommended operating conditions, electrical and switching characteristics), p.4 ('S274/'S275 characteristics), p.5–p.6 (typical application data: bit-slice Wallace trees, 32×32 and 64×64 multipliers). DigiKey hosts the scan (`mm.digikey.com/…/SNx4(L)S27x.PDF`); TI has no product page or `lit/ds` PDF for the '275 (plan-22-A §2; owner decision to transcribe it).
- **Families:** LS (SN54LS275J, SN74LS275N). The S275 parts are not candidates; SN74LS275D is in stock but the sheet lists no D package (`parts.tsv`: not in the datasheet ordering table).
- **Pin names:** the sheet labels every slice input "2ⁿ", both carry inputs "C2ⁿ", the outputs "2ⁿ⁺⁰", "C2ⁿ⁺¹", "2ⁿ⁺¹", "2ⁿ⁺²" and the enable "G̅". The catalog needs unique names: slice inputs `S1`–`S7` in pin order (pins 1, 2, 3, 4, 7, 14, 15), carry inputs `C1` (pin 5), `C2` (pin 6), outputs `Y0` (2ⁿ⁺⁰, pin 9), `CY` (C2ⁿ⁺¹, pin 10), `Y1` (2ⁿ⁺¹, pin 11), `Y2` (2ⁿ⁺², pin 12), enable `G` (pin 13).
- **Pinouts:**

**p16** — J, N (p.1 'LS275/'S275 J and N top view; p.2 'LS275 pin numbers in the block diagram agree)

| Pin | Name | Role |
|---|---|---|
| 1 | S1 | input |
| 2 | S2 | input |
| 3 | S3 | input |
| 4 | S4 | input |
| 5 | C1 | input |
| 6 | C2 | input |
| 7 | S5 | input |
| 8 | GND | ground |
| 9 | Y0 | output |
| 10 | CY | output |
| 11 | Y1 | output |
| 12 | Y2 | output |
| 13 | G | input |
| 14 | S6 | input |
| 15 | S7 | input |
| 16 | VCC | power |

- **Function** (p.2 functional block diagram, 'LS275 pin numbers): five full adders (inputs C, B, A; outputs carry C and sum Σ).
  - FA1: C = pin 3, B = pin 4, A = pin 7 → c1, s1.
  - FA2: C = pin 1, B = pin 14, A = pin 15 → c2, s2.
  - FA3: C = pin 2, B = s1 (FA1 Σ), A = s2 (FA2 Σ) → c3, s3 (FA1 Σ and FA2 C cross on the drawing).
  - FA4: C = c1, B = c2, A = c3 → carry = 2ⁿ⁺² (pin 12), sum = 2ⁿ⁺¹ (pin 11).
  - FA5: C = s3, B = C2ⁿ (pin 5), A = C2ⁿ (pin 6) → carry = C2ⁿ⁺¹ (pin 10), sum = 2ⁿ⁺⁰ (pin 9).
  - All four outputs are 3-state buffers enabled by G̅ (pin 13): "When G̅ is high, all four outputs are off" (p.1).
  - So Y0 + 2·(Y1 + CY) + 4·Y2 = the number of high inputs among the 7 slice and 2 carry inputs. The sheet prints no truth table; the model is the drawn adder netlist.
  - Model `wallace`: `adders` lists FA1…FA5 (in evaluation order, inputs as pin or node names), `outputs` maps Y0/CY/Y1/Y2 to the FA5/FA4 outputs, `enable` `{"pin": "G", "active": "low"}`.
- **Timing:** LS: tPLH 35 ns, tPHL 42 ns typ, any slice or carry input to any output, VCC = 5 V, CL = 45 pF, RL = 667 Ω (p.3); max 62/66 ns. Enable: tPZH 8, tPZL 13, tPHZ 10, tPLZ 10 ns typ (p.3; not modeled separately — the enable uses the main-path delays like every other 3-state output in the catalog).
- **Supply:** SN74LS275 4.75–5.25 V, SN54LS275 4.5–5.5 V (p.3) → `vcc` [4.75, 5.25], `sn54vcc` LS [4.5, 5.5].
- **Checks against the model:** exact for the logic (the netlist as drawn, X propagated through each adder). p.2 note: an unused C2ⁿ input "must be grounded" — a floating input reads X in the simulator, so the outputs it feeds go X, which flags the same mistake.
