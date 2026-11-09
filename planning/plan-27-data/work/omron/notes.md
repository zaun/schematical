# Omron (Aratas) — plan-27 B reader notes

Rows: `/tmp/p27/pre-cand.tsv` (pre-census copy), maker `Aratas (Formerly
Omron Components)`, decision `add`/`pending`: 148 rows (145 add, 3 pending).
PDFs from `/tmp/p27/pre-paths.tsv`. Drawings read as images at 300–600 dpi.

General: every Omron datasheet numbers terminals by a "Bottom View" key and
then draws "Terminal Arrangement/Internal Connections (Top View)" with the
terminals left/right: **4 top-left, 3 top-right, 2 bottom-left, 1
bottom-right**; 1–2 joined, 3–4 joined (`A=1,2;B=3,4`), slots `[3,2,4,1]`.
The PCB drawings are top view and their hole/pad grid matches the outline
(terminals on the left and right). Two-terminal parts draw 2 left, 1 right
(slots `[2,1]`). Ground terminals (5th pin of the "with ground terminal"
models, pin 3 of B3U/B3F-6100, the frame pins 3/4 of B3F-3000), locating
bosses and boss holes are not drawn (§ 4.1 rule 2). Through-hole pads: no land
is printed, so pad = 1.7 × drill.

## B3F (through hole) — en-b3f.pdf (Cat. A070-E1-08), also B3F.pdf (old US catalog), A070-E1.pdf (A070-E1-09, same text)
- Key: none; p.1–2 list every model. Schemes enumerate them (en-b3f bags + -G gold models; B3F.pdf sticks `…S`).
- Pending: B3F-1170, B3F-1172 → **skip**: not in either table (en-b3f lists 1070/1072/1075 only *without* ground terminal; B3F.pdf has no 9.5 mm models).
- 6 × 6 (B3F-1xxx): p.4–5, 6.5 × 4.5 grid, 1.0 holes → `SW-TACT-B3F-THT`. B3F-1110 (7.5 mm pitch) is not among the rows.
- 12 × 12 (B3F-4xxx/5xxx): p.6, 12.5 × 5, 1.2 holes → `SW-TACT-B3F-THT-12` (shared by B3W-4xxx).
- Radial taped (B3F-6xxx): p.7–8, two holes 5 apart on the centre line, 1 +0.1/−0 → drill 1.05 mid → `SW-TACT-B3F-THT-R`; 2 terminals.
- Side-operated (B3F-31xx, "Through Hole, Right Angle"): p.5. Switch terminals 1, 2 = the 1.0 holes 4.5 apart; terminals 3, 4 = the 1.5 holes (frame; 4 grounded) 7 apart, 2.5 behind — not drawn. Terminal y from the side view: 2.25 from the body front face (plunger side) on a 6.25-deep body → y = +0.875 (explicit footprint `SW-TACT-B3F-THT-RA`, body 7.3 × 6.25). Least certain coordinate of the set (the 2.25 is read off the side view); ±0.2 at most.
- Rating 1–50 mA, 3–24 VDC; SPST-NO.

## B3W (through hole) — en-b3w.pdf (+ B3W.pdf old catalog for B3W-4050S)
- Pending B3W-4055S → **add** via B3W.pdf p.1 (stick-packaged `S` models are printed there).
- 6 × 6: p.3–4, same grid as B3F but body 6 × 6.6 → `SW-TACT-B3W`. 12 × 12: p.5 → shares `SW-TACT-B3F-THT-12`.

## B3WN (through hole) — en-b3wn.pdf (A112-E1-05), A112-E1.pdf (-06)
- p.2: two 1 +0.1/−0 holes 5 apart, body 8 × 8 → `SW-TACT-B3WN`. `B3WN-6002(S)` is DigiKey's mpn verbatim (with the parentheses).

## B3FS (SMD) — en-b3fs.pdf; B3FS-1092P.pdf / B3FS-1095P.pdf / A113-E1.pdf (all A113-E1-08, identical text)
- p.3: pads 1.6 × 1.4, span 8.0, pitch 4.5; body 6 × 6.3 → `SW-TACT-B3FS`.
- Key-top-integrated B3FS-1092P/-1095P/-1092P-A: A113-E1 p.3–4, same pads, body = 7.8 × 7.8 key top → `SW-TACT-B3FS-KT`.

## B3FS-4 (SMD) — en-b3fs_4.pdf, B3FS-4092P-B.pdf (A203-E1-05)
- p.2 / p.3: pads 2.3 × 2.4 (from 12/16.6 and 2.6/7.4), span 14.3, pitch 5.0; body 12 × 12 → `SW-TACT-B3FS-4`. B3FS-4092P-B (key top, boss): same pads; its two 2 dia. boss holes are not drawn.

## B3S (SMD) — B3S-Datasheet.pdf p.2
- pads 1.55 × 1.3 (6.4/9.5), span 7.95, pitch 4.5; body 6 × 6.6 → `SW-TACT-B3S`; ground pad of B3S-11xx left out.

## B3SE (SMD) — en-b3se.pdf p.2
- pads 2.75 × 1.0 (3/8.5, 3/5), span 5.75, pitch 4; body 6.2 × 7 → `SW-TACT-B3SE`.
- Odd: row **B3SE-1002** (no `P`) is `add` only because "B3SE-1002" is a substring of the printed B3SE-1002P; the datasheet lists only the embossed-tape `P` models. Same for **B3AL-1004** (legend `B3AL-100@@-@`, packaging symbol P; only B3AL-1004P is printed). The main session may want to re-decide these two.

## B3SL (SMD) — en-b3sl.pdf p.2
- Same pads as B3SE, body 6.2 × 6.5 (0.5 shorter) → own id `SW-TACT-B3SL`.

## B3SN (SMD) — en-b3sn.pdf p.3
- pads 2.75 × 1.4 (4/9.5, 2.6/5.4), span 6.75, pitch 4; body 6 × 6 → `SW-TACT-B3SN`; ground pad 5 of B3SN-31xx left out.

## B3U (SMD and DigiKey "Surface Mount, Right Angle") — en-b3u.pdf p.2–3
- Top- and side-actuated share pads (0.8 × 1.7, 2.6/4.2 → span 3.4) on the body centre line and body 3 × 2.5 → one id `SW-TACT-B3U`, 2 terminals. B3U-1000PM-B (top-actuated) is filed by DigiKey under right angle; kept in that group with the same footprint.

## B3AL (SMD) — B3AL Data Sheet (1).pdf p.2
- pads 2 × 2, span 6, on the centre line; body 6 × 6.9 → `SW-TACT-B3AL`; 2 terminals.

## check rows
The five Aratas `check;` rows (B3FS6003H, B3W 1000G, B3WZ0038M, B3WZ0084C,
B3WZ0137H) are all `skip` (no datasheet PDF), so none is mine:
`checks.tsv` is header-only.

## Cannot draw
None.

## Load check note
`footprint_check` prints at most 10 "extra footprint" lines, so a 15-object
file fails `fp-merge.py --expect` ("new but not reported"). Loaded in two
halves (8 + 7 objects): both `load ok`.

## Counts
- Kept rows: 146 (145 add + 1 regex add).
- scheme-check: 1 add (B3W-4055S), 2 skip (B3F-1170, B3F-1172), 0 pending.
- Footprints: 15. Specs rows: 17.
