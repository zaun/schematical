# Panasonic — plan-27 B/C reader notes

Rows: `/tmp/p27/pre-cand.tsv` (pre-census copy, rebuilt 16:43 after the
truncated Panasonic PDFs were re-fetched), maker `Panasonic Industry`,
decision `add`/`pending`: 193 rows (183 add, 10 pending). PDFs from
`/tmp/p27/pre-paths.tsv`; many hash-named copies are byte-identical in text
(245491 = sw_lt_eng_6s_th, 245483 = sw_lt_eng_6s_mid_2t, 245487 =
sw_lt_eng_6s_long_2t, 245469 = sw_lt_eng_3529s, 245460 = sw_lt_eng_3020s,
245458 = sw_lt_eng_2819s, 245495 = sw_lt_eng_3417s_side_eg, EVQ_series =
Tactile Switches Catalog), so 28 distinct documents were read. Drawings read as
images at 400–1000 dpi.

**Page numbers** in every note are physical PDF pages (1-based), not the
printed "−n−" page numbers (the catalogs' cover is p.1, so printed −2− = p.3).

## General

- Panasonic letters its terminals (A, A', B, B'; (1), (1)', (2), (2)'; A–D) and
  prints the circuit diagram beside every drawing. Two layouts occur:
  - **rows joined** (top pair joined, bottom pair joined): footprint pins 1 =
    top-left, 2 = bottom-left, 3 = bottom-right, 4 = top-right (default dual
    slots), `A=1,4;B=2,3`. EVP0A/EVPAF, EVP3A/EVPAA, EVPAY, EVPBF, EVPBK,
    EVPBL, EVPBT, EVQQ2/6Q2/7Q2, EVQP0 (thin), EVQP7, EVQP3/9P7.
  - **columns joined** (left pair, right pair): same numbering, `A=1,2;B=3,4`.
    EVPBB, EVQP1 (and EVQP0405M), EVQPL, EVQQ1.
  - Where the datasheet numbers terminals (EVQP2/P9/3P2: (1)–(4); EVQPU:
    (1)–(4)) the footprint keeps those numbers via `slots`: EVQP2 `[1,4,2,3]`
    `A=1,3;B=2,4`; EVQPU `[4,3,1,2]` `A=1,3;B=2,4`.
- Ground-terminal pads/holes, boss holes, fixing tabs and the "no soldering"
  areas are not drawn (§ 4.1 rule 2).
- SMD pads are the printed land; "Min. 1" (thin 6 mm) used as 1.0.

## Two-terminal parts with more than two pads (modelled as 2-terminal switches)

- **EVPAE** (edge mount): terminals (1) (2) (1)', (1) and (1)' joined →
  `SW-TACT-EVPAE` is `explicit`, pin 1 has two pads ((1), (1)'), pin 2 is (2);
  `terminals` 2, `pins` `A=1;B=2` (as the C&K/E-Switch readers did for PTS845
  / TL3340).
- **EVPAD** (10 mm centre-space): A1–A4 one side, B1–B2 the other →
  `SW-TACT-EVPAD` pin 1 = four pads (A1–A4), pin 2 = two pads (B1, B2).

## Per series

- **EVP0A, EVPAF** — sw_lt_eng_3026s.pdf p.3–4 → `SW-TACT-EVP0A`. EVP-AFGB65:
  its own 2010 spec (EVP-AFGB65.pdf p.3) prints a slightly larger land
  (0.65 × 0.7 at ±1.475/±0.95 vs 0.55 × 0.6 at ±1.475/±0.9) → own id
  `SW-TACT-EVPAF-G`. Pending **EVP-AFFG65 → skip**: key EVP[0A][FA][force]**B**[height],
  position 7 is fixed B.
- **EVP0B, EVPBD, EVPBX, EVPCD** — 245483 / sw_lt_eng_6s_mid_2t p.3–5, 2 terminals,
  pads 2.0 × 2.2 span 6 → `SW-TACT-EVP0B`.
- **EVP3A, EVPAA** — sw_lt_eng_3529s p.3–4 → `SW-TACT-EVP3A`.
- **EVPAD** — Tactile Switches.pdf p.3 (see above).
- **EVPAE** (Edge Mount) — sw_lt_eng_4522s_side_eg p.3–4. **Least certain body
  placement**: no body-to-board-edge dimension is printed; the body front is put
  on the board edge (GND pads 0.3–1.0 behind the edge sit under the front
  tabs). Pad positions relative to each other are exact; the body outline may be
  off by ~0.3 in y.
- **EVPAK** (SMD right angle) — sw_lt_eng_3819s_side p.3; pads located from
  the boss line using the side view's (1.4) back-face-to-boss dimension.
- **EVPAS** — 245487 / 6s_long_2t p.4–5 → `SW-TACT-EVPAS` (2 pads 2.0 × 2.0; not
  shared with EVP0B: pad height 2.0 vs 2.2).
- **EVPAT** (Edge Mount) — 245495 p.4; body placed from ℓ (plate tip past the
  edge, mid 0.5575) and the (1.9) depth; pad y ±0.1 uncertain.
- **EVPAW** — 245460 / 3020s p.3, 2 terminals. Pending **EVP-AWBD4A,
  EVP-AWCD4A, EVP-AWED4A → skip**: the key is EVP[6A/AW][force]D4**0**; these
  DigiKey numbers end in `4A`, which the key does not produce (possibly a newer
  ordering code — main session may want to look; not widened).
- **EVPAY** — 3429s p.3 (same pads as EVP0A, body 3.4 × 2.9).
- **EVPBB** — 2616s p.3 (EVP-BB_Series_DS p.2 identical) → columns joined.
  Pending **EVP-BB2A9B000 → add** via the 2018 EVP-BB_Series_DS.pdf (key
  …B000, EVPBB2A9B000 printed); the 2025 catalog has the …E000 numbers.
- **EVPBF** — EVP-BF_Series_DS p.2 and sw_lt_eng_6s_mid.pdf p.3 (identical land).
- **EVPBK** — 245468 p.3–4, EVP-BK_Type_DS p.2.
- **EVPBL** — 245458 / 2819s p.3. Pending **EVP-BL9A2B000, EVP-BL2A1B000 →
  skip**: key EVPBL[force]A[height]**F**000; the printed models are …F000.
- **EVPBT** — 6s_th: J-bent C/J (p.6) → `SW-TACT-EVPBT-J` (shared with EVQQ2,
  EVQ6Q2, EVQ7Q2, EVQP0 thin), straight A/G (p.7) → `SW-TACT-EVPBT-S`
  (8.1 × 4.5 grid, 1.6 × 1.4 pads). Two specs rows by the 9th character.
- **EVQ6Q2, EVQ7Q2, EVQQ2** — 6s_th p.4–5 → `SW-TACT-EVPBT-J`. Pending
  **EVQ-Q2503W → skip**: force letter 5 is not in the EVQQ2 tables (scheme
  lists every printed force letter).
- **EVQP0** — thin type (6s_th p.3–4) → `SW-TACT-EVPBT-J`; EVQP0405M is in the
  long-travel catalog (6s_long p.3) → `SW-TACT-EVQP1` (scheme row and specs row
  each). Pending **EVQ-P0D07K (Through Hole) → skip**: not in either catalog.
- **EVQP1** — 6s_long p.3 → `SW-TACT-EVQP1` (columns).
- **EVQP2, EVQP9, EVQ3P2** — 4735s p.3–6 (EVQ-P9_Series_DS identical) →
  `SW-TACT-EVQP2`; all four drawings (middle/short travel, with/without
  ground) give the same terminal land.
- **EVQP4** (Edge Mount) — 6225s_side_eg p.4; (3), (4) are fixing tabs (not in
  the circuit). Body placed with the actuator tip on the board edge (the frame
  tabs line up with the (3)/(4) pads that way).
- **EVQP7** — 245500 p.3–5: all eight variants (straight/J-bent, with/without
  boss) share one land → `SW-TACT-EVQP7`.
- **EVQP3, EVQ9P7** (L-shape terminals) — 245500 p.5. The top two pads carry a
  through hole (0.6 +0.1/−0, mid 0.65) offset 0.1/0.075 from the pad centre;
  the format puts a pad's drill at its centre, so those pads are centred on
  the holes (copper moved ≤ 0.1, inside the drawing's ±0.1). Flag for review.
- **EVQPE** — SMD (Tactile Switches Catalog p.3): the land drawing is turned
  90° from the top view; drawn in top-view orientation (pads on the vertical
  axis) → `SW-TACT-EVQPE-SMD`, shared by EVQPN (EVQ_series, same document).
  Through hole (6035 p.3): holes 1.3, 6.5 apart → `SW-TACT-EVQPE-THT`.
  Both catalogs are marked "To Be Discontinued" / "Not Recommended for New
  Design" (not obsolete; kept).
- **EVQPL** — 49s p.3 → columns; GND pads not drawn.
- **EVQPU** — smalls_side: J-bent B/D/K/M p.4 → `SW-TACT-EVQPU-J`, straight
  A/C/J/L p.3 → `SW-TACT-EVQPU-S`.
- **EVQPV** — 6_long_2r p.3: holes 1.0, 5.0 apart; orientation from the
  front elevation (leads along the top view's vertical axis). Pending
  **EVQ-PV205K → add** (printed in 6_long_2r p.3; the row's own PDF
  EVQ-PV_Rev2003 lists A/G/F/H only).
- **EVQQ1** — 245489 p.3 → columns (A–B, C–D).

## check rows

None (checks.tsv header only).

## Cannot draw

None (cannot-draw.tsv header only).

## Counts

- Kept rows: 185 (183 add + 2 pending accepted).
- scheme-check: 2 add, 8 skip, 0 still pending.
- Footprints: 29. Specs rows: 43.
