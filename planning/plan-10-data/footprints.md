# plan-10 footprints (spec)

Rules: plan-03-B §3 R1–R6 + docs/parts-format.md mid-value / no-overlap rules. Pages are PDF page numbers. JSON objects are paste-ready for `footprints.json` (one object per footprint). Nothing in the repo was edited.

| id | package | rule | status |
|---|---|---|---|
| `CDIP-24-600` | CDIP J 24 | R5 | from drawing |
| `CDIP-24` | CDIP JT 24 | R5 | from drawing |
| `CFP-48` | CFP WD 48 | R2 | from drawing |
| `LCCC-28` | LCCC FK 28 | R4 | from drawing |
| `NFBGA-96` | NFBGA NMJ 96 | R1 (=R6) | from drawing |
| `PDIP-20-NE` | PDIP NE 20 | R5 (PDIP-20 leads) | partial (no NE drawing) |
| `SOIC-16W` | SOIC DW 16 | R1 | from drawing |
| `SOIC-24W` | SOIC DW 24 | R1 | from drawing |
| `SOP-24` | SOP NS 24 | R1 (NS0016A pads) | from drawing |
| `SSOP-24` | SSOP DB 24 | R1 (DB0020A pads) | from drawing |
| `SSOP-48` | SSOP DL 48 | R2 | from drawing |
| `SSOP-56` | SSOP DL 56 | R2 | from drawing |
| `TSSOP-48` | TSSOP DGG 48 | R1 | from drawing |
| `TSSOP-56` | TSSOP DGG 56 | R1 | from drawing |
| `TSSOP-24` | TSSOP PW 24 | R1 | from drawing |
| `TVSOP-48` | TVSOP DGV 48 | R2 | from drawing |
| `VQFN-16` | VQFN RGY 16 | R1 | from drawing |
| `VQFN-20-RGY` | VQFN RGY 20 | R1 by analogy | unresolved (analogy) |
| `WQFN-16` | WQFN BQB 16 | R1 | from drawing |

## `CDIP-24-600` — CDIP J 24

```json
{"id": "CDIP-24-600", "description": "24-pin 600 mil ceramic DIP (J), 13.4 x 31.75 mm body, 2.54 mm pitch, 15.24 mm row spacing", "note": "TI J (R-GDIP-T**) outline 4040084/C (R5; lead 0.46 mid (0.36-0.56); row spacing 0.600 in, A 0.590-0.624 over the shoulders; body B 31.75 mid (31.37-32.13), C 13.40 mid (13.06-13.74, NARR column)), latch/cd4508b.pdf p.10 (same in shifters/cd4034b.pdf p.11, shifters/sn74ls673.pdf p.12)", "style": "dual", "pins": 24, "body": {"width": 13.4, "height": 31.75}, "pitch": 2.54, "span": 15.24, "pad": {"width": 1.5, "height": 1.5, "shape": "round"}, "drill": 0.75, "pin1Shape": "rect"}
```

**Derivation.** **600-mil row spacing in all three PDFs** (cd4508b p.10, cd4034b p.11, sn74ls673 p.12 all print the same J (R-GDIP-T**) 4040084/C, "A" = 0.590–0.624 in for 24 pins; there is no 300-mil J24). R5: lead 0.014–0.022 in (0.36–0.56) → mid 0.46 → drill 0.46 + 0.25 = 0.71 → 0.75; pad 0.75 + 0.7 = 1.45 → 1.5 (minimum); span 15.24 = the 0.600 in row spacing (A's 0.590–0.624 straddles it; A is measured over the lead shoulders, so its mid 15.42 is not the row pitch — same choice as CDIP-28-600). Body: B 31.75 mid (31.37–32.13); C: the table gives NARR 13.06–13.74 and WIDE 14.50–15.19 for 24 pins without saying which TI builds — NARR mid 13.40 used (the WIDE mid would be 14.85; body is cosmetic). Name: `-600` suffix like `CDIP-28-600`.

**PDFs (census2) using it:** `latch/cd4508b.pdf`, `shifters/cd4034b.pdf`, `shifters/sn74ls673.pdf`

## `CDIP-24` — CDIP JT 24

```json
{"id": "CDIP-24", "description": "24-pin 300 mil ceramic DIP (JT), 6.92 x 32.0 mm body, 2.54 mm pitch, 7.62 mm row spacing", "note": "TI JT (R-GDIP-T**) outline 4040110/C (R5; lead 0.48 mid (0.38-0.58); row spacing 0.300 in, 0.290-0.320 over the shoulders; body A 32.00 mid (31.50-32.51), B 6.92 mid (6.22-7.62)), counters/sn74als867a.pdf p.20 (same in latch/sn74abt841a.pdf p.13, latch/sn74abt843.pdf p.13, shifters/sn74ls673.pdf p.13)", "style": "dual", "pins": 24, "body": {"width": 6.92, "height": 32.0}, "pitch": 2.54, "span": 7.62, "pad": {"width": 1.5, "height": 1.5, "shape": "round"}, "drill": 0.75, "pin1Shape": "rect"}
```

**Derivation.** 300-mil. R5 from JT (R-GDIP-T**) 4040110/C: lead 0.015–0.023 in (0.38–0.58) → mid 0.48 → drill 0.73 → 0.75; pad 1.5; span 7.62 (row spacing 0.300; the drawing's 0.290–0.320 is over the lead shoulders, as with existing CDIP-14/16/20 which use A = 0.300 BSC). Body A 32.00 mid (31.50–32.51), B 6.92 mid (6.22–7.62). Name: plain `CDIP-24` (existing CDIP-14/16/20 are 300 mil without suffix; only the 600-mil one carries `-600`).

**PDFs (census2) using it:** `counters/sn74als867a.pdf`, `latch/sn74abt841a.pdf`, `latch/sn74abt843.pdf`, `shifters/sn74ls673.pdf`

## `CFP-48` — CFP WD 48

```json
{"id": "CFP-48", "description": "48-pin ceramic flatpack (WD), 9.66 x 15.88 mm body, 0.635 mm pitch", "note": "TI WD (R-GDFP-F**) outline 4040176/D (R2; lead b 0.28 mid (0.20-0.36), L 7.875 mid (6.35-9.40), E 25.40 mid (22.10-28.70); body 9.66 mid (9.40-9.91) x 15.88 mid (15.49-16.26)), latch/sn74abt16373a.pdf p.17 (same in latch/sn74lvth16373.pdf p.13); pad inner ends overlap the body edge by 0.30", "style": "dual", "pins": 48, "body": {"width": 9.66, "height": 15.88}, "pitch": 0.635, "span": 17.73, "pad": {"width": 8.67, "height": 0.33, "shape": "rect"}}
```

**Derivation.** R2 (old image/text-only outline, no land pattern; precedent CFP-14/16/20). WD (R-GDFP-F**) 4040176/D: lead b 0.008–0.014 in (0.20–0.36) → mid 0.28 → pad width 0.33; L 0.250–0.370 (6.35–9.40) → mid 7.875 → pad length 8.675 → 8.67; E tip-to-tip 0.870–1.130 (22.10–28.70) → mid 25.40; span = 25.40 + 1.0 − 8.675 = 17.725 → 17.73. Body: width 0.370–0.390 (9.40–9.91) → 9.66; A (48) 0.610–0.640 (15.49–16.26) → 15.88. Pitch 0.025 in = 0.635. As on CFP-14/16 the R2 pads' inner ends (x = ±4.53) reach under the body edge (±4.83) by 0.30 — kept as R2 gives it.

**PDFs (census2) using it:** `latch/sn74abt16373a.pdf`, `latch/sn74lvth162373.pdf`, `latch/sn74lvth16373.pdf`

## `LCCC-28` — LCCC FK 28

```json
{"id": "LCCC-28", "description": "28-terminal LCCC (FK), 11.43 x 11.43 mm body, 1.27 mm pitch, pin 1 top centre", "note": "TI FK (S-CQCC-N**) outline 4040140/D (R4; terminal 1.27 mid (1.14-1.40) x 0.635 mid (0.54-0.71); A 11.43 mid (11.23-11.63)), shifters/sn74ls673.pdf p.14; pin map p.1", "style": "quad", "pins": 28, "body": {"width": 11.43, "height": 11.43}, "sides": [7, 7, 7, 7], "pitch": 1.27, "spanX": 10.46, "spanY": 10.46, "pad": {"width": 1.57, "height": 0.685, "shape": "rect"}, "slots": [25, 26, 27, 28, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24]}
```

**Derivation.** R4 like LCCC-20 (same FK (S-CQCC-N**) 4040140/D drawing, image-only): terminal length 0.045–0.055 in (1.14–1.40) → mid 1.27 → pad length 1.57; terminal width 0.022–0.028 in (0.54–0.71) → mid 0.025 in = 0.635 → pad width 0.685 (LCCC-20 used the inch mid, kept identical); A (28) 0.442–0.458 in (11.23–11.63) → mid 11.43; terminal-centre span = 11.43 − 1.27 = 10.16, + 0.3 → spanX = spanY = 10.46. Body 11.43 square. `sides` [7,7,7,7]; pin 1 top centre, counter-clockwise (2–4 go left along the top, 5–11 down the left side…): pin n → slot ((n + 23) mod 28) + 1, i.e. slots [25,26,27,28,1…24] (same rule as LCCC-20's ((n+16) mod 20)+1). Corner check: top pads reach x = ±4.15, side pads start at |x| = 4.45 — no overlap.

**24-pin device in the 28-terminal FK** (sn74ls673.pdf p.1, "SN54LS673 … FK PACKAGE (TOP VIEW)", rendered): the only 24-pin part in FK is SN54LS673 and the census gives it 28 pins (FK 28), so it needs the 28-pad footprint with NC terminals. Map (DIP pin → FK terminal): 1 CS→2, 2 SH CLK→3, 3 R/W→4, 4 STRCLR→5, 5 MODE/STRCLK→6, 6 SER/Q15→7, 7 Y0→9, 8 Y1→10, 9 Y2→11, 10 Y3→12, 11 Y4→13, 12 GND→14, 13 Y5→16, 14 Y6→17, 15 Y7→18, 16 Y8→19, 17 Y9→20, 18 Y10→21, 19 Y11→23, 20 Y12→24, 21 Y13→25, 22 Y14→26, 23 Y15→27, 24 VCC→28; NC = 1, 8, 15, 22. That is the standard TI 24-in-28 FK mapping (one NC at the first terminal of each side, pin 1 → terminal 2). The part's `fk` pinout must name designators "1"–"28" with 1/8/15/22 as NC/absent.

**PDFs (census2) using it:** `shifters/sn74ls673.pdf`

## `NFBGA-96` — NFBGA NMJ 96

```json
{"id": "NFBGA-96", "description": "96-ball NFBGA (NMJ), 16 x 6 grid, 5.5 x 13.5 mm, 0.8 mm pitch", "note": "TI NMJ0096A board layout (R1, 96X \u00d80.4), latch/sn74lvch32373a.pdf p.23; body 5.5 mid (5.4-5.6) x 13.5 mid (13.4-13.6) and ball \u00d80.5 mid (0.45-0.55) from the outline p.22", "style": "grid", "pins": 96, "rows": 16, "columns": 6, "body": {"width": 5.5, "height": 13.5}, "pitch": 0.8, "pad": {"width": 0.4, "height": 0.4, "shape": "round"}, "balls": ["A1", "A2", "A3", "A4", "A5", "A6", "B1", "B2", "B3", "B4", "B5", "B6", "C1", "C2", "C3", "C4", "C5", "C6", "D1", "D2", "D3", "D4", "D5", "D6", "E1", "E2", "E3", "E4", "E5", "E6", "F1", "F2", "F3", "F4", "F5", "F6", "G1", "G2", "G3", "G4", "G5", "G6", "H1", "H2", "H3", "H4", "H5", "H6", "J1", "J2", "J3", "J4", "J5", "J6", "K1", "K2", "K3", "K4", "K5", "K6", "L1", "L2", "L3", "L4", "L5", "L6", "M1", "M2", "M3", "M4", "M5", "M6", "N1", "N2", "N3", "N4", "N5", "N6", "P1", "P2", "P3", "P4", "P5", "P6", "R1", "R2", "R3", "R4", "R5", "R6", "T1", "T2", "T3", "T4", "T5", "T6"]}
```

**Derivation.** R1 (NMJ0096A EXAMPLE BOARD LAYOUT p.23: 96X (Ø0.4), (0.8) TYP pitch). R6 cross-check: ball Ø0.45–0.55 → mid 0.5 × 0.8 = 0.4 — same. Grid: 16 rows A B C D E F G H J K L M N P R T (I, O, Q, S skipped, as the format does) × 6 columns, fully populated (96 = 16 × 6); row A at the top, column 1 at the left in the top-view land pattern. Body 5.5 mid (5.4–5.6) × 13.5 mid (13.4–13.6). Name `NFBGA-96` (only 96-ball BGA; add `-NMJ` if a second NFBGA-96 ever appears).

**PDFs (census2) using it:** `latch/sn74lvch32373a.pdf`

## `PDIP-20-NE` — PDIP NE 20 (NOT ADDED: planner decision, NE parts use `PDIP-20`)

```json
{"id": "PDIP-20-NE", "description": "20-pin power PDIP (NE), 6.86 x 24.0 mm body, 2.54 mm pitch, 7.62 mm row spacing", "note": "TI NE: no outline in the datasheets; body 24.00 x 6.86 nominal from the device-information table, shifters/tpic6a595.pdf p.1 (same in shifters/tpic6a596.pdf p.1); pitch, row spacing and pads (R5, lead 0.46 mid (0.38-0.53)) as PDIP-20 from TI N (R-PDIP-T**) 4040049/E, ff-d/scls136f.pdf p.25", "style": "dual", "pins": 20, "body": {"width": 6.86, "height": 24.0}, "pitch": 2.54, "span": 7.62, "pad": {"width": 1.5, "height": 1.5, "shape": "round"}, "drill": 0.75, "pin1Shape": "rect"}
```

**Derivation.** **Partly resolved — no NE drawing anywhere in the repo.** All three TPIC6A PDFs end with only the DW (R-PDSO-G24) outline (tpic6a259 p.16, tpic6a595 p.20, tpic6a596 p.20 — image pages); the NE outline/NE0020A is absent. NE is TI's 20-pin, 300-mil, 2.54-pitch power PDIP; the only geometry available is the device-information table (tpic6a595.pdf / tpic6a596.pdf p.1: "PDIP(20) 24.00 mm × 6.86 mm", body nominal). So: pitch 2.54 and span 7.62 (20-pin PDIP, 300 mil), pads/drill copied from `PDIP-20` (N outline 4040049/E: lead 0.46 mid → drill 0.75, pad 1.5), body 6.86 × 24.00 from the table (precedent: CFP-20's body from the device table). It differs from `PDIP-20` only in body (24.00 × 6.86 vs 25.4 × 6.35), so either reuse `PDIP-20` outright or add this; I'd add it. Name: `PDIP-20-NE` follows the designator-suffix convention for same-pin-count variants (`SON-6-DRY`, `DSBGA-6-YZP`). Fetch TI's NE0020A (e.g. older TPIC6A595 revisions) to confirm before landing.

**PDFs (census2) using it:** `latch/tpic6a259.pdf`, `shifters/tpic6a595.pdf`, `shifters/tpic6a596.pdf`

## `SOIC-16W` — SOIC DW 16

```json
{"id": "SOIC-16W", "description": "16-pin wide SOIC (DW), 7.5 x 10.3 mm body, 1.27 mm pitch", "note": "TI DW0016A board layout (R1), shifters/sn74hc595.pdf p.29; body 7.5 mid (7.4-7.6) x 10.3 mid (10.1-10.5) from DW0016A outline p.28", "style": "dual", "pins": 16, "body": {"width": 7.5, "height": 10.3}, "pitch": 1.27, "span": 9.3, "pad": {"width": 2, "height": 0.6, "shape": "rect"}}
```

**Derivation.** R1 (DW0016A EXAMPLE BOARD LAYOUT, text): 16X (2) × 16X (0.6), 14X (1.27), (9.3) pad-centre span — identical to SOIC-20W's DW0020A. Body from the DW0016A outline: 7.5 mid (7.4–7.6) × 10.3 mid (10.1–10.5) (device table: "SOIC (16) 10.30 mm × 7.50 mm", sn74hc595.pdf p.1). All 7 DW-16 PDFs carry DW0016A (4220721/A).

**PDFs (census2) using it:** `counters/sn74hc590a.pdf`, `divider/cd4536b.pdf`, `latch/cd4042b.pdf`, `latch/cd4043b.pdf`, `shifters/cd74hc595.pdf`, `shifters/sn74hc594.pdf`, `shifters/sn74hc595.pdf`

## `SOIC-24W` — SOIC DW 24

```json
{"id": "SOIC-24W", "description": "24-pin wide SOIC (DW), 7.5 x 15.4 mm body, 1.27 mm pitch", "note": "TI DW (R-PDSO-G24) land pattern 4209202-5/F (R1), latch/sn74abt841a.pdf p.18; body 7.50 mid (7.40-7.60) x 15.40 mid (15.20-15.60) from outline 4040000-5/G p.17", "style": "dual", "pins": 24, "body": {"width": 7.5, "height": 15.4}, "pitch": 1.27, "span": 9.4, "pad": {"width": 2, "height": 0.6, "shape": "rect"}}
```

**Derivation.** R1 (image-only "LAND PATTERN DATA DW (R-PDSO-G24)" 4209202-5/F, rendered): pad 2.0 × 0.6, 22x1,27 pitch, 9,4 between pad-row centres (stencil 1.95 × 0.55 ignored). Body from DW (R-PDSO-G24) 4040000-5/G: 7.50 mid (7.40–7.60) × 15.40 mid (15.20–15.60). Note the span is 9.4 here vs 9.3 on the newer DW0016A/DW0020A layouts — kept as this drawing prints it. The same outline is in sn74ls673 p.15, cd4508b p.15, cd4059a p.11, sn74als867a p.21, tpic6a259 p.16, tpic6a595/596 p.20; the layout page is in sn74abt841a p.18, sn74abt843 p.15, sn74lvc841a p.16.

**PDFs (census2) using it:** `counters/sn74als867a.pdf`, `divider/cd4059a.pdf`, `divider/cd74hc4059.pdf`, `latch/cd4508b.pdf`, `latch/sn74abt841a.pdf`, `latch/sn74abt843.pdf`, `latch/sn74als841.pdf`, `latch/sn74als843.pdf`, `latch/sn74lvc841a.pdf`, `latch/tpic6a259.pdf`, `shifters/cd4034b.pdf`, `shifters/sn74abt8996.pdf`, `shifters/sn74ls673.pdf`, `shifters/tpic6a595.pdf`, `shifters/tpic6a596.pdf`

## `SOP-24` — SOP NS 24

```json
{"id": "SOP-24", "description": "24-pin SOP (NS), 5.3 x 15.0 mm body, 1.27 mm pitch", "note": "TI NS0016A board layout pads/pitch/span (R1), ff-d/scls119e.pdf p.18; body 5.30 mid (5.00-5.60) x 15.00 mid (14.70-15.30) from NS (R-PDSO-G**) 4040062/C, latch/cd4508b.pdf p.14", "style": "dual", "pins": 24, "body": {"width": 5.3, "height": 15.0}, "pitch": 1.27, "span": 7, "pad": {"width": 1.85, "height": 0.6, "shape": "rect"}}
```

**Derivation.** As SOP-14/16/20 (plan-03-B §4.1): R1 with NS0016A's pads, pitch and span (1.85 × 0.6, 1.27, (7)) — the NS (R-PDSO-G**) family drawing covers 14–24 pins with one lead geometry; body width 5.30 mid (5.00–5.60), length A(24) 15.00 mid (14.70–15.30) from 4040062/C (image-only, rendered cd4508b.pdf p.14).

**PDFs (census2) using it:** `latch/cd4508b.pdf`

## `SSOP-24` — SSOP DB 24

```json
{"id": "SSOP-24", "description": "24-pin SSOP (DB), 5.3 x 8.2 mm body, 0.65 mm pitch", "note": "TI DB0020A board layout pads/pitch/span (R1), ff-d/cd74act273.pdf p.22; body 5.30 mid (5.00-5.60) x 8.20 mid (7.90-8.50) from DB (R-PDSO-G**) 4040065/E, latch/sn74abt841a.pdf p.12 (same in latch/sn74abt843.pdf p.16, latch/sn74lvc841a.pdf p.17)", "style": "dual", "pins": 24, "body": {"width": 5.3, "height": 8.2}, "pitch": 0.65, "span": 7, "pad": {"width": 1.85, "height": 0.45, "shape": "rect"}}
```

**Derivation.** No DB0024A in the repo; the 24-pin PDFs give only the old DB (R-PDSO-G**) 4040065/E family outline (text; 14–38 pins, one lead geometry). Following SOIC-16/SOP-* precedent: R1 with a sibling's layout — DB0020A (1.85 × 0.45, 0.65, span 7, as SSOP-20). Body 5.30 mid (5.00–5.60) × A(24) 8.20 mid (7.90–8.50). (R2 from the outline would give 1.55 × 0.35, span 7.25 — L 0.55–0.95, b 0.22–0.38, E 7.40–8.20.)

**PDFs (census2) using it:** `latch/sn74abt841a.pdf`, `latch/sn74abt843.pdf`, `latch/sn74lvc841a.pdf`

## `SSOP-48` — SSOP DL 48

```json
{"id": "SSOP-48", "description": "48-pin SSOP (DL), 7.49 x 15.88 mm body, 0.635 mm pitch", "note": "TI DL (R-PDSO-G48) outline 4040048-3/F (R2; lead b 0.273 mid (0.203-0.343), L 0.765 mid (0.51-1.02), E 10.35 mid (10.03-10.67); body 7.49 mid (7.39-7.59) x 15.88 mid (15.75-16.00)), latch/sn74abt16373a.pdf p.12 (same in latch/sn74ahc16373.pdf p.12)", "style": "dual", "pins": 48, "body": {"width": 7.49, "height": 15.88}, "pitch": 0.635, "span": 9.79, "pad": {"width": 1.57, "height": 0.32, "shape": "rect"}}
```

**Derivation.** R2 (image-only old outline DL (R-PDSO-G48) 4040048-3/F, no land pattern): b 0.008–0.0135 in (0.203–0.343) → mid 0.273 → pad width 0.32; L 0.020–0.040 (0.51–1.02) → mid 0.765 → pad length 1.565 → 1.57; E 0.395–0.420 (10.03–10.67) → mid 10.35; span = 10.35 + 1.0 − 1.565 = 9.785 → 9.79. Body 7.49 mid (7.39–7.59) × 15.88 mid (15.75–16.00) (device table: 15.88 × 7.49). Pitch 0.025 in = 0.635.

**PDFs (census2) using it:** `latch/sn74abt16373a.pdf`, `latch/sn74ahc16373.pdf`, `latch/sn74ahct16373.pdf`, `latch/sn74alvch16373.pdf`, `latch/sn74lvc16373a.pdf`, `latch/sn74lvch16373a.pdf`, `latch/sn74lvth162373.pdf`, `latch/sn74lvth16373.pdf`

## `SSOP-56` — SSOP DL 56

```json
{"id": "SSOP-56", "description": "56-pin SSOP (DL), 7.49 x 18.42 mm body, 0.635 mm pitch", "note": "TI DL (R-PDSO-G56) outline 4040048-4/F (R2; lead b 0.273 mid (0.203-0.343), L 0.765 mid (0.51-1.02), E 10.35 mid (10.03-10.67); body 7.49 mid (7.39-7.59) x 18.42 mid (18.29-18.54)), latch/sn74abt16841.pdf p.11 (same in latch/sn74alvch16841.pdf p.15)", "style": "dual", "pins": 56, "body": {"width": 7.49, "height": 18.42}, "pitch": 0.635, "span": 9.79, "pad": {"width": 1.57, "height": 0.32, "shape": "rect"}}
```

**Derivation.** R2, same lead numbers as SSOP-48 (DL (R-PDSO-G56) 4040048-4/F, image-only): pad 1.57 × 0.32, span 9.79, pitch 0.635. Body 7.49 × 18.42 mid (18.29–18.54).

**PDFs (census2) using it:** `latch/sn74abt162841.pdf`, `latch/sn74abt16841.pdf`, `latch/sn74alvch16260.pdf`, `latch/sn74alvch162841.pdf`, `latch/sn74alvch16841.pdf`

## `TSSOP-48` — TSSOP DGG 48

```json
{"id": "TSSOP-48", "description": "48-pin TSSOP (DGG), 6.1 x 12.5 mm body, 0.5 mm pitch", "note": "TI DGG0048A board layout (R1), latch/sn74lvc16373a.pdf p.26; body 6.1 mid (6.0-6.2) x 12.5 mid (12.4-12.6) from DGG0048A outline p.25", "style": "dual", "pins": 48, "body": {"width": 6.1, "height": 12.5}, "pitch": 0.5, "span": 7.5, "pad": {"width": 1.5, "height": 0.3, "shape": "rect"}}
```

**Derivation.** R1 (DGG0048A EXAMPLE BOARD LAYOUT, text): 48X (1.5) × 48X (0.3), 46X (0.5), (7.5) pad-centre span. Body 6.1 mid (6.0–6.2) × 12.5 mid (12.4–12.6) from the DGG0048A outline (device table 12.50 × 6.10). sn74lvth16373/162373 carry only the old DGG (R-PDSO-G**) 4040078/F — same dimensions (6.00–6.20, 12.40–12.60, b 0.17–0.27, 0.50 pitch).

**PDFs (census2) using it:** `latch/sn74abt16373a.pdf`, `latch/sn74ahc16373.pdf`, `latch/sn74ahct16373.pdf`, `latch/sn74alvch16373.pdf`, `latch/sn74auc16373.pdf`, `latch/sn74avc16373.pdf`, `latch/sn74lvc16373a.pdf`, `latch/sn74lvch16373a.pdf`, `latch/sn74lvth162373.pdf`, `latch/sn74lvth16373.pdf`

## `TSSOP-56` — TSSOP DGG 56

```json
{"id": "TSSOP-56", "description": "56-pin TSSOP (DGG), 6.1 x 14.0 mm body, 0.5 mm pitch", "note": "TI DGG0056A board layout (R1), latch/sn74alvch16841.pdf p.17; body 6.1 mid (6.0-6.2) x 14.0 mid (13.9-14.1) from DGG0056A outline p.16", "style": "dual", "pins": 56, "body": {"width": 6.1, "height": 14.0}, "pitch": 0.5, "span": 7.5, "pad": {"width": 1.5, "height": 0.3, "shape": "rect"}}
```

**Derivation.** R1 (DGG0056A layout, text): 56X (1.5) × 56X (0.3), 54X (0.5), (7.5). Body 6.1 mid (6.0–6.2) × 14.0 mid (13.9–14.1).

**PDFs (census2) using it:** `latch/sn74abt162841.pdf`, `latch/sn74alvch16260.pdf`, `latch/sn74alvch162841.pdf`, `latch/sn74alvch16841.pdf`

## `TSSOP-24` — TSSOP PW 24

```json
{"id": "TSSOP-24", "description": "24-pin TSSOP (PW), 4.4 x 7.8 mm body, 0.65 mm pitch", "note": "TI PW0024A board layout (R1), latch/sn74lvc841a.pdf p.13; body 4.4 mid (4.3-4.5) x 7.8 mid (7.7-7.9) from PW0024A outline p.12", "style": "dual", "pins": 24, "body": {"width": 4.4, "height": 7.8}, "pitch": 0.65, "span": 5.8, "pad": {"width": 1.5, "height": 0.45, "shape": "rect"}}
```

**Derivation.** R1 (PW0024A layout, text): 24X (1.5) × 24X (0.45), 22X (0.65), (5.8). Body 4.4 mid (4.3–4.5) × 7.8 mid (7.7–7.9).

**PDFs (census2) using it:** `latch/cd4508b.pdf`, `latch/sn74abt841a.pdf`, `latch/sn74lvc841a.pdf`, `shifters/sn74abt8996.pdf`

## `TVSOP-48` — TVSOP DGV 48

```json
{"id": "TVSOP-48", "description": "48-pin TVSOP (DGV), 4.4 x 9.7 mm body, 0.4 mm pitch", "note": "TI DGV (R-PDSO-G**) outline 4073251/E (R2; lead b 0.18 mid (0.13-0.23), L 0.625 mid (0.50-0.75), E 6.40 mid (6.20-6.60); body 4.40 mid (4.30-4.50) x 9.70 mid (9.60-9.80)), latch/sn74ahc16373.pdf p.13", "style": "dual", "pins": 48, "body": {"width": 4.4, "height": 9.7}, "pitch": 0.4, "span": 5.97, "pad": {"width": 1.43, "height": 0.23, "shape": "rect"}}
```

**Derivation.** R2 from DGV (R-PDSO-G**) 4073251/E (text) — exactly as TVSOP-14/16/20: b 0.13–0.23 → 0.18 → pad width 0.23; L 0.50–0.75 → 0.625 → pad length 1.425 → 1.43; E 6.20–6.60 → 6.40; span 6.40 + 1.0 − 1.425 = 5.975 → 5.97 (TVSOP-20's value). Body 4.40 mid (4.30–4.50) × A(48) 9.70 mid (9.60–9.80). Alternative: sn74lvc16373a p.23 has a new DGV0024A layout (24X (1.4) × (0.2), (5.9)) for the same lead geometry; not used, to stay identical to the existing TVSOP family.

**PDFs (census2) using it:** `latch/sn74ahc16373.pdf`, `latch/sn74ahct16373.pdf`, `latch/sn74auc16373.pdf`, `latch/sn74avc16373.pdf`, `latch/sn74lvc16373a.pdf`, `latch/sn74lvch16373a.pdf`

## `VQFN-16` — VQFN RGY 16

```json
{"id": "VQFN-16", "description": "16-pin VQFN (RGY), 3.5 x 4.0 mm body, 0.5 mm pitch, exposed pad", "note": "TI RGY0016A board layout (R1), counters/sn74lv163a.pdf p.25; body 3.5 mid (3.35-3.65) x 4.0 mid (3.85-4.15) from outline p.24", "style": "quad", "pins": 16, "body": {"width": 3.5, "height": 4.0}, "sides": [6, 4, 6, 4], "pitch": 0.5, "spanX": 3.3, "spanY": 3.8, "pad": {"width": 0.6, "height": 0.24, "shape": "rect"}, "centre": {"width": 2.05, "height": 2.55, "shape": "rect"}, "slots": [20, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 17]}
```

**Derivation.** R1 (RGY0016A layout, text + rendered p.25): 16X (0.6) × 16X (0.24), 10X (0.5); (3.3) between left/right pad centres, (3.8) between top/bottom; exposed pad (2.05) × (2.55). Pins: 2–7 down the left, 8 and 9 on the bottom, 10–15 up the right, 16 and 1 on the top; top/bottom pins are **2X (1.5)** apart, i.e. on the outer slots of a 4-slot side (as VQFN-14 = RHL0014A). `sides` [6,4,6,4] (20 slots, 4 unused): pin 1 → slot 20 (top-left), 2–7 → 1–6, 8 → 7, 9 → 10, 10–15 → 11–16, 16 → 17. Body 3.5 mid (3.35–3.65) × 4.0 mid (3.85–4.15). Centre = exposed pad 17 (unnumbered pad; parts don't name it).

**PDFs (census2) using it:** `counters/sn74lv163a.pdf`, `counters/sn74lv4040a.pdf`, `shifters/sn74lv165a.pdf`, `shifters/sn74lv595a.pdf`

## `VQFN-20-RGY` — VQFN RGY 20

```json
{"id": "VQFN-20-RGY", "description": "20-pin VQFN (RGY), 3.5 x 4.5 mm body, 0.5 mm pitch, exposed pad", "note": "TI RGY0020A board layout (R1), 4225320/A 09/2019 (not in the repo datasheets; TI drawing copy at dl7avf.info/charts/outline2/QFND041R.pdf p.2: pads 20X (0.6) x 20X (0.24), 14X (0.5), (3.3) x (4.3), exposed pad (2.05) x (3.05), pins 10/11 and 20/1 at (0.75) from centre); body 3.5 mid (3.35-3.65) x 4.5 mid (4.35-4.65) from the outline p.1", "style": "quad", "pins": 20, "body": {"width": 3.5, "height": 4.5}, "sides": [8, 4, 8, 4], "pitch": 0.5, "spanX": 3.3, "spanY": 4.3, "pad": {"width": 0.6, "height": 0.24, "shape": "rect"}, "centre": {"width": 2.05, "height": 3.05, "shape": "rect"}, "slots": [24, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21]}
```

**Derivation (planner, 2026-11-01).** Confirmed against TI RGY0020A, drawing 4225320/A (board layout: pads 0.6 × 0.24, pitch 0.5, spans 3.3 / 4.3, exposed pad 2.05 × 3.05; pins 10, 11, 20 and 1 sit 0.75 from the centre line, i.e. on the outer slots of a 4-slot side, as the entry below has). The JSON above is final. Original agent note follows. **Not resolved from a drawing.** No RGY0020A outline or layout exists in any repo PDF (all six RGY-20 datasheets, and TI's current TXS0108E/TXB0108 fetched for this, carry only the "GENERIC PACKAGE VIEW RGY 20, 3.5 x 4.5, 0.5 mm pitch" 4225264/A) — the same gap plan-03-B hit for RGY-14. Proposed entry by family analogy with RGY0016A (R1, above) and RHL0014A (VQFN-14), which share pad 0.6 × 0.24, pitch 0.5, spanX = body − 0.2 = 3.3, top/bottom pins on the outer slots 1.5 apart, exposed pad 2.05 wide and (body length − 1.45) long: body 3.5 × 4.5 (generic view), spanY 4.3, exposed pad 2.05 × 3.05, `sides` [8,4,8,4] with pin 1 top-left, 2–9 left, 10 and 11 bottom outer slots, 12–19 right, 20 top-right. Confirm against TI RGY0020A (or RHL0020A) before landing; name `VQFN-20-RGY` (existing `VQFN-20` is RKS).

**PDFs (census2) using it:** `latch/sn74abt573a.pdf`, `latch/sn74lv373a.pdf`, `latch/sn74lv573a.pdf`, `latch/sn74lvc373a.pdf`, `latch/sn74lvc573a.pdf`, `latch/sn74lvth573.pdf`

## `WQFN-16` — WQFN BQB 16

```json
{"id": "WQFN-16", "description": "16-pin WQFN (BQB), 2.5 x 3.5 mm body, 0.5 mm pitch, exposed pad", "note": "TI BQB0016A board layout (R1), shifters/sn74ahc595.pdf p.31; body 2.5 mid (2.4-2.6) x 3.5 mid (3.4-3.6), exposed pad 1.0 mid (0.9-1.1) x 2.0 mid (1.9-2.1) from outline p.30", "style": "quad", "pins": 16, "body": {"width": 2.5, "height": 3.5}, "sides": [6, 2, 6, 2], "pitch": 0.5, "spanX": 2.3, "spanY": 3.3, "pad": {"width": 0.6, "height": 0.24, "shape": "rect"}, "centre": {"width": 1, "height": 2, "shape": "rect"}, "slots": [16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]}
```

**Derivation.** R1 (BQB0016A layout, text, 4224640/B): 16X (0.6) × 16X (0.24), 10X (0.5), 2X (0.5) between pins 1/16 and 8/9 (adjacent top/bottom pins, unlike RGY); (2.3) between left/right pad centres, (3.3) between top/bottom; exposed pad (1) × (2). Like WQFN-14 (BQA): `sides` [6,2,6,2], pin 1 top-left → slots [16,1…15]. Body 2.5 mid (2.4–2.6) × 3.5 mid (3.4–3.6) (device table says 3.60 × 2.60 — the outline is used).

**PDFs (census2) using it:** `shifters/sn74ahc165.pdf`, `shifters/sn74ahc594-q1.pdf`, `shifters/sn74ahc595-q1.pdf`, `shifters/sn74ahc595.pdf`, `shifters/sn74ahct595.pdf`, `shifters/sn74hcs165-q1.pdf`, `shifters/sn74hcs165.pdf`, `shifters/sn74hcs595-q1.pdf`, `shifters/sn74hcs595.pdf`, `shifters/sn74lv165a-q1.pdf`, `shifters/sn74lv165a.pdf`, `shifters/sn74lv594a.pdf`, `shifters/sn74lv595a-q1.pdf`, `shifters/sn74lv595a.pdf`, `shifters/sn74lvc165a.pdf`, `shifters/sn74lvc595a.pdf`

## Checks requested

### Existing `VQFN-20` (RKS) for the census's VQFN RKS 20 parts — reuse OK
`VQFN-20` = TI RKS0020A layout (ff-d/sn74ahc273.pdf p.33). The latch PDFs print the same RKS0020A (e.g. latch/sn74ahc573.pdf, outline + layout text): body 2.4–2.6 × 4.4–4.6, terminals 0.18–0.30 × 0.3–0.5, exposed pad 1 × 3 (±0.1), layout 20X (0.6) × 20X (0.24), 16X (0.5), (2.3) × (4.3), exposed pad (1) × (3) — identical to `VQFN-20`. 8 of the 10 RKS PDFs carry RKS0020A (sn74ac573, sn74act573, sn74ahc373, sn74ahc573, sn74ahct373, sn74ahct573, sn74hcs373, sn74hcs573). `sn74ahc573-q1` and `sn74hcs573-q1` (wettable-flank "WRKS") have no RKS outline at all (only tape/reel data: same 2.8 × 4.8 pocket as the other RKS parts; device table 4.50 × 2.50) — reuse `VQFN-20` for them too (wettable flank changes the terminal side, not the land pattern).

### Old "SO" type, NS code, 16 pins → existing `SOP-16` — reuse OK
In `/tmp/p10/census.json` the "SO"/NS/16 rows come from 4 PDFs: counters/cd4060b, latch/cd4042b, latch/cd4043b, shifters/cd4014b (census2 normalises them to SOP NS 16). cd4060b p.26–28 (and cd4042b, cd4043b) print NS0016A with layout 16X (1.85) × 16X (0.6), 14X (1.27), (7) — exactly `SOP-16`'s source (NS0016A, ff-d/scls119e.pdf p.18). cd4014b has no NS drawing (only D, J, N, PW); its NS reel data (8.2 × 10.5 pocket) is the NS-16 one, so `SOP-16` applies.

### CDIP J 24 row spacing
All three J-24 PDFs (cd4508b p.10, cd4034b p.11, sn74ls673 p.12) use the same J (R-GDIP-T**) 4040084/C 24/28/32/40-pin drawing with A = 0.590–0.624 in → **600 mil** in every one → `CDIP-24-600`. The 300-mil 24-pin ceramic DIP is TI's **JT** (4040110/C: 0.290–0.320) → `CDIP-24`. sn74ls673 has both (SN54LS673 J = 600 mil; the JT rows are 300 mil).

### 24-pin devices in the 28-terminal FK
Only SN54LS673 (shifters/sn74ls673.pdf) — map in the `LCCC-28` section above (DIP n → FK: 1–6 → 2–7, 7–12 → 9–14, 13–18 → 16–21, 19–24 → 23–28; NC 1, 8, 15, 22).

## Open items
- `VQFN-20-RGY`: geometry by family analogy, no RGY0020A in any repo PDF or in TI's current TXS0108E/TXB0108 (generic view only). Needs TI RGY0020A/RHL0020A to confirm spanY 4.3 and exposed pad 2.05 × 3.05.
- `PDIP-20-NE`: no NE drawing in the repo; body from the device table, everything else from the N 20-pin. Alternatively assign the NE parts to existing `PDIP-20` (only the body differs).
- `CDIP-24-600` body width: NARR vs WIDE column not identified by the drawing; NARR used.
- Rounding: R2 spans x.xx5 are rounded half-up here (SSOP-48/56 9.785 → 9.79, CFP-48 17.725 → 17.73); TVSOP-48 keeps TVSOP-20's 5.97 for identity.
