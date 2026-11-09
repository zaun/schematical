# Same Sky (Formerly CUI Devices) — plan-27 reader notes

Rows come from the pre-census copy `/tmp/p27/pre-cand.tsv`. It has 405 Same Sky rows. 90 of them are mine (`add` 1, `pending` 89). The other 315 are `skip`, 289 + 2 `check` + TS10 with "no datasheet PDF (HTTP safari)", or `out` (24, `lit`). The dispatch expected about 379 rows over about 36 groups. That count needs the PDFs the census is still fetching (TS02, TS04, TS10, TS14, TS15, TS19, TS21, …). Those series are **not read here**. Rerun a reader on them once their PDFs are in.

Each datasheet has its key on p.1 and its drawing, schematic and recommended PCB layout on p.2 (all viewed as 300 dpi images).

| series | PDF | mounting | key / regex | footprint | pins |
|---|---|---|---|---|---|
| TS06 | ts06.pdf | SMD | `TS06-667-(25\|30\|34\|43\|50\|55)-(R\|BK\|WT)-(100\|160\|260)-G-SMT-TR` | SW-TACT-TS06 | A=1,3;B=2,4 |
| TS08 | ts08.pdf | SMD | `TS08-66-43-BK-160-SMT-67(-TR)?` (key fixes every field except packaging: blank = bag, TR = tape & reel) | SW-TACT-TS08 | A=1,3;B=2,4 |
| TS11 | ts11.pdf | THT right angle | `TS11-674-(43\|50\|55\|65\|73\|80\|85\|95\|105\|120\|135\|153)-BK-(100\|160\|260)-RA-D` | SW-TACT-TS11 | A=1;B=2 |
| TS13 | ts13.pdf | THT | `TS13-1212-73-BK-(100\|160\|260)-D` | SW-TACT-TS13 | A=1,2;B=3,4 |
| TS17 | ts17.pdf | SMD | `TS17-48-05-YE-(160\|260)-SMT-TR` | SW-TACT-TS17 | A=1,2;B=3,4 |
| TS18 | ts18.pdf | SMD | `TS18-5-(15\|17\|20\|25)-SL-(160\|260)-SMT-TR` | SW-TACT-TS18 | A=1,2;B=3,4 |
| TS24 | ts24.pdf | SMD | `TS24-62-14-(CL\|BL)-(200\|250)-SMT-TR-67` | SW-TACT-TS24 | A=1,2;B=3,4 |
| TS42 | ts42.pdf | SMD | `TS42-5252-09-Y-(160\|260\|350)-SMT-TR` | SW-TACT-TS42 | A=1,2;B=3,4 |

## Odd things

- **TS06**: there is a 5th land, terminal 5, a ground terminal (schematic: 5 to earth), left of centre. It is not a switch terminal, so it is left out. Its pads have the same size and pitch as TS24 (1.6 × 1.4, 8.4 × 4.5), but the body differs (6.0 × 6.7 vs 6.2 × 6.2) and so does the numbering (TS06 numbers down the columns: 1 TL, 2 BL, 3 TR, 4 BR). So the two get separate ids.
- **TS08** uses the same column numbering as TS06: 1–3 joined, 2–4 joined.
- **TS11** (right angle): the schematic shows a 1–2 switch, with 3–4 joined to ground (the metal frame legs, Ø1.3 holes 7 mm apart). The footprint therefore has 2 terminals, the Ø1.0 holes 4.5 mm apart.
  - Orientation is the PCB layout's top view. The terminal row is 2.5 mm below (in front of) the frame-hole row, so the actuator faces +y (down).
  - Pin numbers are printed only on the front view (looking at the actuator), with pin 2 on the left and pin 1 on the right. Seen from above with the actuator facing down, the left/right sides are the same, so pin 1 is at x = +2.25.
  - The hole row is 1.15 behind the body's back face (side view). With the body at 7.4 × 3.6, that puts it at y = −0.65.
  - This is the one footprint where I had to work out positions from more than one view. Please check it in the K render.
- **TS24**: none of its drawings print terminal numbers. I numbered it 1 TL, 2 TR, 3 BL, 4 BR, following the schematic layout (1–2 top, 3–4 bottom). The layout drawing has a switch symbol that joins the top pads together and the bottom pads together, so the pairing is drawn and not guessed. Only the labels 1…4 are inferred.
- **TS13**: hole layout only (4 × Ø1.3). The pad is 1.7 × 1.3 = 2.21, rounded to 2.2. `pin1Shape` is not set because the datasheet does not mark pin 1.
- No ranges were needed for the pads: every land pattern is printed with single values.

## SPST-NC row TS10-63-26-BE-250-SMT-TR

Decided in round 3 from ts10.pdf: **keep (add)**.
- p.1 calls it a "normally closed switch". The actuator is blue and non-illuminated, and the part is surface mount.
- The p.2 schematic shows terminals 1-2, ON when NORMAL and OFF when DOWN.
- The key is the single fixed code `TS10-63-26-BE-250-SMT-TR`.
- Footprint SW-TACT-TS10: 2 pads 1.5 x 1.4, 6.5 mm apart, pin 1 left. Pins `A=1;B=2` (the two contact sides).

## `check` rows (checks.tsv)

TS06-667-34-BK-100-G-SMT-T and TS06-667-34-BK-260-G-SMT-T both get `keep`:
- SPST-NO (ts06.pdf p.2 schematic: OFF normally, ON when pressed)
- non-illuminated, Surface Mount
- actuator Standard (as the other TS06 rows)
- rating 0.05A @ 12VDC (p.1: 50 mA max, 12 Vdc max)

Note: these DigiKey numbers end in `-SMT-T`, while the TS06 key ends in `-SMT-TR`, so the TS06 regex does **not** match them. In the round-3 copy they are `pending` with series `*`. A `*` scheme row carries the TS06 key as printed, so scheme-check turns them `skip` ("not in the datasheet's ordering table"). I did not widen the regex. If DigiKey's `-T` is a real cut-tape listing, the owner can decide to add `(TR|T)`.

## Round 2 (2026-11-08 16:38 rebuild)

TS08-66-43-BK-160-SMT-67 became `pending` under the stricter hit. It got a scheme: `TS08-66-43-BK-160-SMT-67(-TR)?`. The 16:38 rebuild brought no new PDFs.

## Round 3 (2026-11-08 16:43 rebuild: whole PDFs for every series)

29 more series were read: TS01-05, TS07, TS09, TS10, TS12, TS14-16, TS20-23, TS30-37, TS40, TS41, TS44, TS45.

Shared footprints:
- TS07 uses SW-TACT-TS06.
- TS14 and TS15 use SW-TACT-TS13.
- TS02 and TS03 use SW-TACT-TS01.
- TS36 uses SW-TACT-TS34.

Their notes say so.

Points to check in the K render:
- **TS03 numbering** is taken from TS01/TS02: same 6 x 6 THT build and holes, and ts03 prints no numbers.
- **TS21 numbering** is inferred (1 TL, 2 TR, 3 BL, 4 BR; top pair joined). No TS21 drawing numbers the terminals, and unlike TS24 there is no switch symbol on the layout. If this is wrong, it is the one that could put a wrong connection on a board.
- **TS41**: the body position against the pads is estimated (+-0.4), because the drawing gives no pad-to-body dimension.
- **TS22, TS30-TS33 (right angle / edge)**: the body position against the pads is estimated (+-0.15-0.2); the pads relative to each other are exact. TS22 also needs a 3.80 x 1.35 board-edge cutout, which the format cannot express; it is stated in the note.

### Per-series detail

### TS01 (ts01.pdf)
Key p.1: `TS01-66-73-BK-(100|160|260)-D`. Drawing p.2: 4-d1.0 holes at 6.5 x 4.5, with 1 TL, 2 TR, 3 BL and 4 BR as printed on the layout. Schematic: 1-2 joined, 3-4 joined. New footprint SW-TACT-TS01.

### TS02 (ts02.pdf)
Key p.1. Heights 55, 120, 140 and 160 are LCR-only (`*` note), and the regex splits them out. Terminals are SCR or LCR. The SCR (p.2) and LCR (p.3) drawings have the same layout as TS01, so both use SW-TACT-TS01.

### TS03 (ts03.pdf)
Key p.1: `TS03-66-(50|70|80|95)-BK-(160|260)-LCR-D-67`. Layout p.2 has the same holes as TS01 (6.50 x 4.50, 4-d1.00); the body is 6.1, within 0.1, so it uses SW-TACT-TS01. **Unsure:** ts03.pdf prints no terminal numbers on any view, only the schematic (1-2 / 3-4). I took the numbering and pairing from TS01/TS02, the same 6x6 THT construction. Check this, or drop TS03 to cannot-draw if that inference is not acceptable.

### TS04 (ts04.pdf)
Key p.1: heights 43/50/55/65/70/73/75/85/95, packaging blank (bag) or `-TR`. The 7.3 mm models (p.2) and the others (p.3) share one layout: pads 1.6 x 1.4 at 8.6 x 4.5, with 1 TL, 2 BL, 3 TR, 4 BR. Schematic: 1-3 / 2-4. New footprint SW-TACT-TS04.

### TS05 (ts05.pdf)
Key p.1: `TS05-66-(43|50|80)-BK-260-SMT-67`. Layout p.2 (viewed at 500 dpi): pad height 1.5, centre pitch 4.5, outer 10, inner 6.5, so pads are 1.75 x 1.5 at 8.25 x 4.5. Numbering 1 TL, 2 BL, 3 TR, 4 BR; schematic 1-3 / 2-4. New footprint SW-TACT-TS05.

### TS07 (ts07.pdf)
Key p.1: `TS07-667-(25|30|37|43|50)-BK-(100|160|260)-SMT-TR`. Layout p.2 has pads 1.6 x 1.4 at 8.4 x 4.5, body 6 x 6.7, numbering 1 TL, 2 BL, 3 TR, 4 BR. That is identical to TS06, so it uses **SW-TACT-TS06** (add TS07 to that footprint's note). Note: DigiKey's outline says 6.10 x 6.10, but the drawing says 6 x 6.7.

### TS09 (ts09.pdf)
Key p.1: `TS09-63-25-(R|WT)-(160|260)-SMT-TR`. Two terminals. Layout p.2: pads 1.1 x 1.0 at 7.5. The layout has no numbers, so I put 1 left and 2 right as on the schematic; a 2-terminal SPST is symmetric, so the order does not matter. New footprint SW-TACT-TS09 (dual, pins 2, perSide 1).

### TS10 (ts10.pdf) — SPST-NC row TS10-63-26-BE-250-SMT-TR
Decision **keep**. ts10.pdf p.1 says "normally closed switch" with a blue (non-illuminated) actuator, SMD. Its schematic p.2 reads NORMAL = ON, DOWN = (OFF): 1-2. The key p.1 is the single fixed code `TS10-63-26-BE-250-SMT-TR`. Layout p.2: 2 pads 1.5 x 1.4 at 6.5, pin 1 left and 2 right (numbered on the top view). Pins A=1;B=2: the two sides of the NC contact. New footprint SW-TACT-TS10.

### TS12 (ts12.pdf) — cannot draw
All 5 rows are `add` (number printed). However, every drawing (p.2-4) shows a 5-way navigation switch: A/B/C/D directions plus centre push, 6 terminals (Public/common plus F1-F5). It is not a single SPST contact, so the 2/4-terminal spec format cannot express it. All 5 are listed under cannot (reason: circuit). They should arguably be `out` (circuit); the parent decides.

### TS14 (ts14.pdf)
Key p.1: heights 43/50/55/60/65/70/80/90/100/120/140/150, forces 100/160/260, SCR only. Layout p.2: 4-d1.3 at 12.5 x 5, body 12. The top view numbers 1 TL, 2 TR, 3 BL, 4 BR; schematic 1-2 / 3-4. Identical to TS13, so it uses **SW-TACT-TS13** (add TS14 to that footprint's note).

### TS15 (ts15.pdf; key p.1, drawing p.2)
- Key: `TS15-1212-(50|65|70|80|95|100|110|125|140|150)-BK-(160|260)-SCR-D`. The key prints "95 = 95 mm"; I read it as 9.5 mm.
- Through hole: 4 × Ø1.3 holes at 12.5 × 5, numbered 1 TL, 2 TR, 3 BL, 4 BR, with 1–2 and 3–4 joined. Body 12 × 12.
- This is identical to TS13, so TS15 shares **SW-TACT-TS13**.

### TS16 (ts16.pdf; key p.1, drawing p.2)
- Key: `TS16-1212-(75|85|95)-(R|BK)-(100|160|260)-SMT-TR`.
- SMD pads 2.1 × 2.1 at 14.3 × 5.0, numbered 1 TL, 2 TR, 3 BL, 4 BR, with 1–2 and 3–4 joined. Body 12 × 12.
- The drawing on p.2 is the one used. I did not check the extra p.5 (not needed).

### TS20 (ts20.pdf; key p.1, drawing p.2)
- Key: one fixed number, `TS20-42-25-WT-160-SMT-TR`.
- Pads 1.05 × 0.65 at 4.15 × 2.15, numbered 1 TL, 2 TR, 3 BL, 4 BR, with 1–2 and 3–4 joined. Body 4.2 × 3.2.

### TS21 (ts21.pdf; key p.1, drawing p.2)
- Key: `TS21-34-035-BK-(160|260)-SMT-TR`.
- Four side pads 1.25 × 0.6 at 3.25 × 2.0.
- The two 1.0 × 0.5 lands at top and bottom centre are cover tabs, so they are left out.
- **No drawing numbers the terminals.** I numbered them 1 TL, 2 TR, 3 BL, 4 BR after the schematic's layout, with 1–2 and 3–4 joined. On every Same Sky drawing that does print numbers, the joined pair runs across the body at the same y. This pairing is **inferred, not printed**; please review.

### TS22 (ts22.pdf; key p.1, drawing p.2), Edge Mount, Right Angle
- Key: `TS22-45-035-BK-(160|260)-SMT-TR`. Rated 20 mA at 15 Vdc (different from the rest of the family).
- **The board needs a 3.80 × 1.35 edge cutout**, which the footprint format cannot express (the note says so).
- Three signal pads, 0.8 × 1.25, numbered 1-2-1. The schematic shows the two pin-1 pads joined, with the switch between pin 1 and pin 2. So this is a 2-terminal switch, and pin 1 has two pads.
- Ground tabs 4 and 5 sit beside the cutout and are left out.
- The body (4.9 × 2.9) position relative to the pads is estimated from the side view (±0.2). The pads relative to each other are exact.

### TS23 (ts23.pdf; key p.1, drawing p.2)
- Key: `TS23-46-06-BK-(160|260)-SMT-TR`.
- Pads 1.0 × 0.7 at 4.0 × 1.6. The 2 × Ø0.7 peg holes are left out.
- Numbered 1 TL, 2 TR, 3 BL, 4 BR, with 1–2 and 3–4 joined. Body 3.5 × 3.0.

### TS30, TS31, TS32, TS33 (Surface Mount, Right Angle; 2-terminal side-push switches)
- Keys:
  - TS30: `TS30-46-(34|35)-BK-160-RA(-H)?-SMT-TR`
  - TS31: `TS31-46-35-BK-B-(160|260)-RA(-H)?-SMT-TR`
  - TS32: `TS32-7-35-BK(-B)?-(160|260)-RA(-H)?-SMT-TR` (B = boss)
  - TS33: `TS33-46-35-BK-(160|260)(-H)?-SMT-TR`
- In each key, a 'blank' option drops its dash. DigiKey's numbers confirm this, e.g. TS30-46-34-BK-160-RA-SMT-TR.
- Each schematic has only switch 1–2, and pads 1 and 2 are the two terminal pads. Cover/frame tab lands (TS31 calls them 3/4, ground), peg and boss holes are all left out.
- Pin 1 vs pin 2 left/right: the numbered views are bottom views (pegs and solder faces visible). So in the top view pin 1 is at +x for TS31, TS32 and TS33 (actuator toward −y). TS30's layout has the actuator toward +y, so its pin 1 is at −x. For a 2-terminal switch a swap is harmless.
- Body position against the pads (±0.15–0.2):
  - TS31: from the peg distance (front face 0.9 ahead of the peg line).
  - TS32: from the boss distance on p.3 (1.0).
  - TS33: from the tab-leg slot distance (0.85).
  - TS30: from the pink body outline drawn on the layout.
- TS32: the no-boss (p.2) and with-boss (p.3) layouts have identical copper, so there is one footprint.

### Skips
None. Every pending row matches its key.

### TS34 (ts34.pdf, 8 pp.)
- The key is on p.1: `TS34-62-{2|25|31|35}-BK-{blank|G}-{100|160|260}-{blank|H}-SMT-TR`.
  - G = with ground; H = halogen free. A blank field drops its dash as well, as the DigiKey numbers show.
- There are four drawings: p.2 (2.0 mm, no ground), p.3 (2.0 mm, ground), p.4 (2.5/3.1/3.5 mm, no ground) and p.5 (2.5/3.1/3.5 mm, ground).
- p.2 and p.4 have the same land pattern (9.5/4 × 5.4/2.6) and the same body, so they share one footprint, SW-TACT-TS34.
- The layout pads carry no numbers. Their numbers come from the top view next to them: 1 top-left, 2 top-right, 3 bottom-left, 4 bottom-right. The schematic joins 1–2 and 3–4.
- G (ground) models: no rows exist. The scheme accepts them, but the spec row excludes them, so a G row would show as uncovered rather than land on the wrong footprint.
- Result: 11 rows kept.

### TS35 (ts35.pdf)
- Key on p.1: `TS35-6-15-BK-{160|260}-{blank|H}-JSMT-TR`.
- Drawing on p.2: a 2-terminal switch (1 left, 2 right) with pads 2.0 × 2.0 at 6.0 centres.
- Result: 2 rows kept.

### TS36 (ts36.pdf, IP67)
- Key on p.1: `TS36-62-{31|35}-BK-{blank|G}-{160|260}-{blank|H}-SMT-TR-67`.
- The p.2 no-ground land pattern and body are identical to TS34's, so it reuses SW-TACT-TS34.
- G models: as for TS34.
- Result: 4 rows kept.

### TS37 (ts37-6368-41-r-400-smt-tr.pdf, a single-model sheet)
- The one row is `add`.
- p.2 drawing: 2 castellated terminals on an FR-4 base, pads 2.0 × 2.0 at 6.0 centres. That matches TS35's pads, but the body is 6.8 × 6.3, so TS37 gets its own id.
- The terminal numbers are printed only on the upper view, which is probably the bottom view (it shows no actuator). I kept 1 left, 2 right as printed. A 2-terminal switch has no polarity, so this does not change the circuit.
- The sheet gives a 16 V rating (DigiKey row data, not mine to change).

### TS40 (ts40.pdf)
- Key on p.1: `TS40-66-{33|38|43|45|50|55|70}-{BK|R}-{160|260}-SMT-TR`. The note says 33 comes only with BK, and the regex encodes that.
- Drawing on p.2: pads 1.8 × 1.1, centres 8.2 × 4.5, numbered 1 TL, 2 TR, 3 BL, 4 BR. The schematic joins 1–2 and 3–4.
- Result: 18 rows kept.

### TS41 (ts41.pdf)
- Key p.1: `TS41-4539-((35|40)-WT|(50|60|65)-BK)-(160|260)-SMT-TR` (35/40 white only, 50/60/65 black only); all 6 rows match.
- 3 terminals: the p.2 schematic joins 1 and 3 as one common terminal that switches to 2.
- Drawn per plan-27-B Correction 3 as a 2-terminal switch: footprint SW-TACT-TS41 is `explicit` with 2 pins, pin 1 has TWO pads (the outer terminals 1 and 3), pin 2 is the centre pad; spec `A=1;B=2`.
- The layout labels the pads 3-2-1 left to right, the front view 1-2-3. Since 1 and 3 are the same pin here, the conflict does not matter.
- Pads 0.9 x 1.7 at x = -1.3 / 0 / +1.3. Cover-tab lands and peg slots are left out.
- The drawing gives no pad-to-body dimension, so the body position is estimated (+-0.4). The pads relative to each other are exact.

### TS44 (ts44.pdf, IP67)
- Both rows are `add`.
- Key on p.1: `TS44-6262-{34|51}-{BL|WT}-{200*|260*|350}-SMT-TR-67`, where * means white only.
- Both rows are 5.1 mm models, drawn on p.3: pads 2.75 × 1.0, centres 5.75 × 4.0, body 6.2 × 6.2, numbered 1 TL, 2 TR, 3 BL, 4 BR.
- The spec covers only `TS44-6262-51-.*`. The 3.4 mm drawing on p.2 has no rows and I did not read it.

### TS45 (ts45.pdf)
- Key on p.1: `TS45-8383-405-{WT|Y|YG}-{260|400|700}-SMT-TR`. The key prints no colour/force pairing restriction.
- The p.2 (260/400 gf) and p.3 (700 gf) layouts are identical: 2 pads of 1.8 × 2.7 at 8.2 centres, 1 left, 2 right. Body 8.3 × 8.3.
- Result: 3 rows kept.

## Cannot draw (cannot-draw.tsv)

- **TS12, 5 rows (all `add`)**: a 5-way navigation switch (common + F1-F5, 6 terminals). It is not one SPST contact, so it is really out of scope by circuit.

TS41 (3 terminals, 1 and 3 common) is drawn, not listed: per plan-27-B Correction 3 it is a 2-terminal switch whose pin 1 has two pads (see TS41 above).

## Counts (round 3)

- Same Sky add/pending rows in the copy: 381 (8 add + 373 pending, incl. the 2 `*` check rows).
- scheme-check: `371 add, 2 skip, 0 still pending`. The 2 skips are the TS06 `-SMT-T` check rows.
- Cannot draw: 5 (TS12). Kept: 374.
- Footprints: 29. Specs rows: 35. Schemes rows: 35. checks.tsv: 2.
- specs-check (cannot-draw rows set skip in the copy): `specs ok (374 add rows)`.
- fp-load: `29 new footprints: load ok`.
- uncovered.py: `0 uncovered rows`.
