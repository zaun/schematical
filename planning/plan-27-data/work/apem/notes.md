# plan-27 C — APEM datasheet reading (slug apem)

Rows: the pre-census working copy (/tmp/p27/pre-cand.tsv): 18 add + 113 pending
APEM Inc. rows (4 of the pending are `check` rows; the fifth check row,
3FTH946, is already `skip`, no PDF). PDFs from /tmp/p27/pre-paths.tsv.

## ADTS (Through Hole, Through Hole Right Angle, Surface Mount) — adts6-adtsm-ktsc6.pdf

- Key p.2 (HOW TO ORDER): `ADTS [M] <size 6|2> <H> <stem K/N/R/S/Y> V [B]`.
  6 x 6 heights 1, 2, 3, 44, 48, 5, 6; 12 x 12 heights 1, 4, 5 with K, Y
  unavailable; B (tube) not for 63, 644, 648, 65, 66. The key's "N/K = Black,
  160gf" has no printed spelling: left out (ADTSM62KSVTR is skip). TR (DigiKey's
  tape-and-reel suffix on the M parts) is not in the key: optional group on the
  M rows only.
- Second scheme row from AS-ADTSM25KSVB.pdf (APEM drawing B7606, ADTSA6**V
  right-angle, the PDF DigiKey links for ADTSM25KSVB): `ADTSA6[12356][KNRSY]V`
  → ADTSA62NV/62RV/63RV add; ADTSA644NV/644RV/648NV skip (44/48 not DIM-L codes).
- Skip (not in any key): ADTSG66RV, ADTSGMW66NSVTR, ADTSJW*, ADTSMW* (the
  6.3 x 6.2 "W" families are another datasheet), ADTSM62KSVTR.
- Drawings p.3–4. P.C. layouts print no terminal numbers; the CIRCUIT figure
  (p.2) shows T1–T2 joined on the top line, T3–T4 on the bottom. Numbered as a
  top view with the joined pairs across the long span (as Omron B3F's
  numbered figure): 1 TL, 2 TR, 3 BL, 4 BR (slots 1,4,2,3), `A=1,2;B=3,4`.
  - THT 6 x 6: 4 × Ø1.00, 6.50 × 4.50 → SW-TACT-ADTS-THT (body 6.00);
    ADTS644/648 body 6.20 → SW-TACT-ADTS-THT-64.
  - THT 12 x 12: Ø1.20, 12.50 × 5.00 → SW-TACT-ADTS-THT-12.
  - SMD 6 x 6 (ADTSM6, ADTSM64 identical): 1.55 × 1.30 at span 7.95 (6.40/9.50),
    pitch 4.50, body 6.20 → SW-TACT-ADTS-SMD.
  - SMD 12 x 12 (ADTSM2, ADTSM24): 1.55 × 1.60 at span 13.95 (12.40/15.50),
    pitch 5.00 → SW-TACT-ADTS-SMD-12.
  - ADTSA right angle (B7606, rotated sheet): terminals 1, 2 = the Ø1.00 holes
    4.50 apart; the 7.00 pair 2.50 behind are ground legs 3, 4 (schematic) —
    not drawn; 2 terminals, explicit, row 2.55 behind the front face, body
    7.40 × 6.55 → SW-TACT-ADTSA.

## ADTS3 (Surface Mount) — adts3.pdf

- Key p.2: `ADTS [M|ML] 3 <1|2> <N/R/S/Y> V [B | T/R | bag]`, T/R written TR.
  All 5 rows add (ADTSM3 J-lead).
- Layout p.3 ADTSM3: 2.0 × 1.6 pads at span 5.40 (3.40/7.40); body 6.00 × 3.51;
  circuit T1–T2 → SW-TACT-ADTS3-SMD, `A=1;B=2`.

## APTC (Surface Mount, Right Angle) — aptclgtv.pdf

- Single drawing APTCLGTV; regex `APTCLGTV(TR)?` (TR = DigiKey reel suffix).
- P.C. layout: 4 pads 1.45 × 1.00 (3.70/6.60), pitch 1.70; small ground pad
  (terminal 5) and peg holes left out. Top view numbers 1 TL, 2 BL, 3 TR, 4 BR;
  schematic 1–3, 2–4 joined → `A=1,3;B=2,4`, slots 1,2,4,3 → SW-TACT-APTC.

## AMPT — PCB-MJTP-PHAP33 series-12mm-TH.pdf

- The only PDF DigiKey links for AMPTFP2VTR is the MJTP 12 mm through-hole
  page; it prints no AMPT number or key. Scheme = that page's numbers → skip.

## MJTP (Through Hole, Through Hole Right Angle) — MJTP1117.pdf, PCB-MJTP-PHAP33 series-12mm-TH.pdf

- No key; regex lists the printed numbers (6 mm through-hole p.2–7; 12 mm
  p.2). Pending add: MJTP1230B (printed in the MJTP1230 table, though its own
  DigiKey PDF is the 12 mm page), MJTP1212, MJTP1212C. Skip: every SMD MJTP
  (1106, 1122B, 1138*, 1140*, 1162*, 1164TR, SJW*, SMW* — no SMD datasheet
  linked), MJTP1141, MJTP1212B, MJTP1230E/F, MJTP1232B/F, MJTP1236B/D/E,
  MJTPSHWA, and MJTP1236G: the MJTP1236 table prints "MJTP1230G / PHAP3305G"
  for L = 5.8 (likely a typo for 1236G, but not printed as such; not added).
- PCB layouts p.7 (no terminal numbers anywhere); schematics p.7: 4-terminal
  figure for MJTP1109/1119/1193/1230/1234/2205/MJTPSHW (numbered as ADTS),
  2-terminal for MJTP1105/1117/1129/1236/1243/1250.
  - MJTP1230, MJTP1234: 6.5 × 4.5 Ø1.0, body 6.0 → shares SW-TACT-ADTS-THT.
  - MJTP1119/1119A: 7.9 × 5.0 Ø1.0, body 7.2 → SW-TACT-MJTP1119.
  - MJTPSHW/SHWB: 6.5 × 4.0 Ø1.0, body 6.3 × 6.2 → SW-TACT-MJTPSHW.
  - MJTP1212/1212C: 12.5 × 5.0 Ø1.2, body 12, schematic 1 TL 2 TR 3 BL 4 BR
    → shares SW-TACT-ADTS-THT-12.
  - Right angle (frame/ground legs not drawn, 2 terminals, pin 1 left):
    MJTP1105T and MJTP1236 (same layout: Ø1.0 pair 4.5 apart in front, Ø1.3
    pair 7.0 apart 2.5 behind) → SW-TACT-MJTP1236; MJTP1117 (Ø1.0 pair 5.0
    apart) → SW-TACT-MJTP1117; MJTP1129 (Ø1.0 5.0 pair in front, 7.8 pair
    behind, layout drawn with the front at the top) → SW-TACT-MJTP1129.
  - Doubt: two body offsets are scaled, not printed: MJTP1236's body depth
    (6.5, scaled; APEM's ADTSA6 drawing of the same layout prints 6.55) and
    MJTP1129's terminal row 1.5 behind the front face. Hole spacing and
    pairing are printed; only the outline's placement around them is scaled.

## Multimec® 3 / Multimec™ 3A (Surface Mount) — as-3ash9-44158219.pdf, as-3ash9r-45211430.pdf

- Key (as-3ash9 p.1): `3A <T|S> <L6|H9>` (+ LED code with T); 3ASH9R = tape
  and reel (as-3ash9r p.1 standard versions). 3ASH9 pending → add; 3ASH9R add.
- SMD solder pad (as-3ash9r p.3, same in as-3ash9 p.1): 2.5 × 1.2 at
  12.5 × 7.62. Circuit diagram p.3: 2 TL–1 TR joined, 4 BL–3 BR joined
  (slots 4,1,3,2), `A=1,2;B=3,4` → SW-TACT-MULTIMEC-3A-SMD (both series).
- Doubt: the bare switch body is not dimensioned (only lead spans 14 / 7.6,
  cap 12.6, bezel 15.1); body 10.1 × 10.1 from DigiKey's size, consistent with
  the front view scaled against the 14 span.

## check rows

- 5GSH9358222: Multimec 5G key (AS-5GSH9358222.pdf p.3) — SH9 35 + LED 8222
  (red/green bicolor): illuminated → `out` (`lit`). Note: the series `*`
  scheme row does match it (scheme-check adds it on a copy where it is not yet
  `out`); the main session must apply checks.tsv first.
- DTSA62NV: keep (SPST-NO, Through Hole, Right Angle) but its PDF is the ADTS6
  datasheet whose key has no DTSA prefix → skip.
- ADTS21N: keep (SPST-NO, Through Hole); its PDF is the DTS(M)-2 sheet whose
  key spells `DTS…` (no A prefix, no N-only ADTS form) → skip.
- ADTSM25KSVB: keep (SPST-NO, Surface Mount); its PDF is the ADTSA6 drawing;
  in the ADTS key K is unavailable for 12 x 12 and KS is no code → skip.

## Cannot draw

None.

## Counts

- Kept rows: 74 (18 verbatim add + 56 regex add); out 1 (5GSH9358222).
- scheme-check (with 5GSH9358222 set out on the copy): 56 add, 56 skip, 0 still pending.
- Footprints: 14. Specs rows: 17.
- fp-load: `14 new footprints: load ok`.
