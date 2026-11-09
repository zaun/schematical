# Same Sky (Formerly CUI Devices) — plan-27 reader notes

Rows come from the pre-census copy `/tmp/p27/pre-cand.tsv`. It has 405 Same Sky rows. 90 of them are mine (`add` 1, `pending` 89). The other 315 are `skip`, 289 + 2 `check` + TS10 with "no datasheet PDF (HTTP safari)", or `out` (24, `lit`). The dispatch expected about 379 rows over about 36 groups. That count needs the PDFs the census is still fetching (TS02, TS04, TS10, TS14, TS15, TS19, TS21, …). Those series are **not read here**. Rerun a reader on them once their PDFs are in.

Each datasheet has its key on p.1 and its drawing, schematic and recommended PCB layout on p.2 (all viewed as 300 dpi images).

| series | PDF | mounting | key / regex | footprint | pins |
|---|---|---|---|---|---|
| TS06 | ts06.pdf | SMD | `TS06-667-(25\|30\|34\|43\|50\|55)-(R\|BK\|WT)-(100\|160\|260)-G-SMT-TR` | SW-TACT-TS06 | A=1,3;B=2,4 |
| TS08 | ts08.pdf | SMD | (only an `add` row: TS08-66-43-BK-160-SMT-67, bag; key `…-67-XX`, blank = bag, TR = reel) | SW-TACT-TS08 | A=1,3;B=2,4 |
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

Decision: **left `skip`, not decided**. Its datasheet (ts10.pdf) is not on disk in the pre-census copy (reason "no datasheet PDF (HTTP safari)"), so there is no key or drawing to read. SPST-NC is in scope (plan-27 keeps non-illuminated SPST-NO/NC), so once the census fetches ts10.pdf the row should be read like any other: key regex, land pattern, and pins (NC: the A/B sides are still the two contact sides).

## `check` rows (checks.tsv)

TS06-667-34-BK-100-G-SMT-T and TS06-667-34-BK-260-G-SMT-T both get `keep`:
- SPST-NO (ts06.pdf p.2 schematic: OFF normally, ON when pressed)
- non-illuminated, Surface Mount
- actuator Standard (as the other TS06 rows)
- rating 0.05A @ 12VDC (p.1: 50 mA max, 12 Vdc max)

Note: these DigiKey numbers end in `-SMT-T`, while the TS06 key ends in `-SMT-TR`, so the TS06 regex does **not** match them. Once they are `pending` they will turn into `skip` ("not in the datasheet's ordering table"). I did not widen the regex. If DigiKey's `-T` is a real cut-tape listing, the owner can decide to add `(TR|T)`.

## Cannot draw

None.

## Counts

- Rows: 90 (1 `add` + 89 `pending`); kept 90.
- scheme-check: 89 add, 0 skip, 0 still pending.
- Footprints: 8 (TS06, TS08, TS11, TS13, TS17, TS18, TS24, TS42).
- Specs rows: 8.
- checks.tsv: 2 rows (both keep; they are skip in the copy, see above).
- specs-check: `specs ok (90 add rows)`.
- fp-load: `8 new footprints: load ok`.
