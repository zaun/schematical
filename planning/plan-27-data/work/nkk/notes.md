# NKK Switches — plan-27 reader notes

Rows: /tmp/p27/pre-cand.tsv (pre-census copy), maker `NKK Switches`, decision
add/pending: 88 rows (7 add, 81 pending). No `check;` rows (checks.tsv is
header only).

All four drawn series share NKK's layout convention: the "Typical switch
dimensions" row on p.3 shows a bottom view (1 3 / 2 4) and, at the right, the
PC board layout as a top view, mirrored (3 1 / 4 2). The footprints follow the
top view: terminal 3 top-left, 1 top-right, 4 bottom-left, 2 bottom-right
(`dual`, slots [4,3,1,2]). Schematic on every sheet: terminals 1–2 joined and
3–4 joined, so the button joins them (`A=1,2;B=3,4`). "Terminal numbers are
not actually on the switch."

## CB (CB.pdf, 4 pp.)
- Key p.3: CB 15 F P [-S]; packaging S = stick tube, no code = tray. Regex
  `CB15FP(-S)?`. CB15FP was already add; CB15FP-S added.
- Drawing p.3: holes (1.0) dia typ at (4.0) x (6.5); body (6.2) square. Pad
  1.7 (1.7 x drill). → SW-TACT-CB.

## CB3 (CB3.pdf, 4 pp.)
- Key p.3: CB3 15 F P [-R]; R = tape-reel, no code = tray. Both rows already
  add. Regex `CB315FP(-R)?`.
- Land layout p.3: pads (1.4) x (1.7) typ, centres (4.0) x (8.9); body (6.2).
  → SW-TACT-CB3.

## HP02 (HP02.pdf, 4 pp.)
- Key p.3: HP02 15A F K P2|P4 - S. The packaging table lists only S, but the
  same page's photo caption prints `HP0215AFKP2` (no suffix) and DigiKey sells
  both forms, so the suffix is an optional group: `HP0215AFKP[24](-S)?`.
  Unsure point: strictly the key always carries `-S`.
- Hole layout p.3: (1.0) dia typ at (4.5) x (6.7); body (6.2). Pad 1.7.
  → SW-TACT-HP02.
- P2 = silver 3VA DC, P4 = gold 0.4VA @ 28V AC/DC (DigiKey ratings agree).

## HP03 (HP03.pdf, 4 pp.)
- Key p.3: HP03 15A F K P2|P4 - S|R (no "no code" entry), but the caption
  prints `HP0315AFKP4`, so the suffix is optional: `HP0315AFKP[24](-[SR])?`.
- Land layout p.3: pads (1.4) x (2.2) typ, centres (4.5) x (9.0); body (6.2).
  → SW-TACT-HP03.

## JB (JBilluminated.pdf, 6 pp.) — all 52 rows skip
- The PDF is the *illuminated* JB sheet. Its key (p.3) is
  JB15 [H] L|B P C|E|F - cap (B/C/E/F, JB/JC/JE/JF, BB/BC/BE/BF/BH; diffuser
  colour must match the LED), every part with an LED. The scheme row spells
  that key out; it matches none of DigiKey's rows, which are the
  non-illuminated JB15KP / JB15HAP / JB15HKP / JB15FP / JB15KH (right-angle)
  family with K/HA/HK/F actuator and H terminal codes that this PDF does not
  have. Result: all 52 JB rows `skip` (not in the datasheet's ordering table).
  To keep them, the main session would need NKK's non-illuminated JB sheet.
- Odd numbers seen: `JB15HANP2/H7E-2H`, `JB15KP-2A/1`, `JB15KP/AT4060JB`
  (custom/cap-assembly suffixes).

## JF (JFilluminated-1.pdf, 6 pp.) — all 22 rows skip
- Illuminated JF sheet; key (p.3) JF15 R|A P 1|2|3 <actuator colour> <LED>
  (full face A: square actuator only, LED colour = actuator colour). Regex
  `JF15(RP[123][CDFGH][CEF]|AP1(CC|EE|FF))`. DigiKey rows JF15CP2x / JF15SPnx
  use illumination codes C/S (non-illuminated) not in this key → all `skip`.
  Same remedy as JB (non-illuminated JF sheet).

## Cannot draw
None.

## Counts
- kept rows: 14 (CB 2, CB3 2, HP02 4, HP03 6)
- scheme-check: 7 add, 74 skip, 0 still pending
- out: 0
- footprints: 4 (SW-TACT-CB, SW-TACT-CB3, SW-TACT-HP02, SW-TACT-HP03)
- specs rows: 4
