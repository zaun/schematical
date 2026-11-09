# plan-27 B — C&K datasheet reading (slug ck)

Rows: the pre-census working copy (/tmp/p27/pre-cand.tsv; 409 add + 47 pending C&K rows, 5 of them `check`). Read in five batches (PTS6xx; PTS8xx/RK/SFS/TLS; ATS/ED/EL2/KM*; KS*/KT/KXT3/NanoT; KSC family/RS COMAX), merged here.

## Merge: shared land patterns (plan-27-B § 4.2)

- SW-TACT-EL2 = EL2 + every KSC J termination (KSC, KSC-OF, KSC-PF, KSC-ST, KSC-TE, KSC-XA, KSC10, KSC11, KSC4 TE, KSC4D, KSC5) + KSE/KST J (batch ids SW-TACT-KSC-J, SW-TACT-KSE-J dropped). Every pad equal, same numbering, bodies 6.35-6.45.
- SW-TACT-KSC-G = KSC G terminations + KSE/KST G (SW-TACT-KSE-G dropped).
- SW-TACT-ATS = ATS + KSR (SW-TACT-KSR dropped).
- SW-TACT-PTS636-SMD-J25 = PTS636 25 mm J-lead + RS, COMAX RS285 (SW-TACT-RS-COMAX-285 dropped).
- Near but kept separate: SW-TACT-PTS526-SMD / -SMD-H08 (same pads, body 5.2 vs 4.8); SW-TACT-KMR2 / SW-TACT-KMS (same pads, different numbering).

## Check rows (series `*` in candidates.tsv)

All five keep; their specs rows use series `*` and the mounting from checks.tsv: PTS830GG140GSMTRLFS (SW-TACT-PTS830-G), KMR241GULCLFS (SW-TACT-KMR2), NANOT160BP (SW-TACT-NANOT-PIP, Surface Mount, Right Angle as its twin NANOT 160 BP), KSC421G70SHLFS (SW-TACT-KSC-G), KSC441JST2LFS (SW-TACT-EL2).

## fp-load note

`footprint_check` prints at most 10 "extra footprint" lines, so fp-load.sh on all 55 new ids reports FAILED ("new but not reported" for 45 ids) although its out.txt has only extra/unused lines (110 = 55 + 55) and nothing else. Loaded in six chunks of at most 10 (/tmp/ckw/chunk*.json): every chunk `load ok`, which also runs the overlap test on every pad.

## Batch 1

### PTS125 (Surface Mount 2, Through Hole 12)

- PDF: littelfuse-c-k-tactile-pts125-series-datasheet (5 pp.). Key p.5 (standard
  `PTS125-S-[J]-force-height-[P]-2|SMTR2-LFS` and long-life
  `PTS125-S-[J]-force-height-[P]-[SMTR]-[G]-21M|25M-LFS`). Drawings p.2–4.
- All 14 rows were pending; three regex rows (standard; long-life 21M; long-life
  25M, which note 2 limits to 130 gf = L). J (snap fit) only with height 73.
  14 add, 0 skip. THT standard parts carry `-2` (key shows "2: THT in bags"
  with dash separators).
- THT: 4 holes Ø1.3 at 12.5 × 5 (p.2), no land printed → pad 2.2. Pegs-version
  holes Ø1.7 not drawn. SMT (rows are long-life SMTR, no pegs, no ground pin):
  pads 2 × 1.5 at 14.3 × 5; the no-pegs drawing letters terminals A–D at the
  places of 1–4 (A–B, C–D joined), taken as 1–4.
- Schematic p.2: 1–2 joined, 3–4 joined → `A=1,2;B=3,4`.

### PTS525 (Surface Mount 2)

- PDF: littelfuse-c-k-tactile-pts525-series-datasheet p.2 (all add).
- SM08/SK08 and SM15/SK15 layouts are the same (8/4.8 × 4.8/2.8 → pad 1.6 × 1.0,
  centres 6.4 × 3.8). Body 5.2 × 5.2 from the 0.8 mm drawing; the 1.5 mm drawing
  prints 5.1 × 5.1 (within 0.1, one footprint). PTS525SK08SMTR-2 LFS (DigiKey,
  with dash) is listed by DigiKey; the PDF prints PTS525SK08SMTR2 LFS.

### PTS526 (Surface Mount 9)

- PDF: littelfuse-c-k-tactile-pts526-series-datasheet; key p.4, drawings p.1–3,
  table p.5. All add.
- Three footprints: gull-wing 1.5/1.7/2.0 mm with or without ground pin
  (`SW-TACT-PTS526-SMD`, body 5.2), gull-wing 0.8 mm (`-SMD-H08`, same pads,
  body 4.8 × 4.8), J-lead (ground pin only; `-SMD-J`, 1.3 × 0.7 at 4.7 × 3.7).
  Ground pads 5, 6 left out (not switch terminals).
- Circuit diagram p.2: 1–2, 3–4 joined.

### PTS530 (Surface Mount 4)

- PDF: littelfuse-c-k-tactile-pts530-series-datasheet p.2 (all add). Used the
  "Recommended PCB Layout" (not the "Alternative PCB Layout"). Body mid-values
  4.525 × 4.55. Schematic 1–2 / 3–4.

### PTS540 (Surface Mount 2, 1 pending)

- PDF: littelfuse-c-k-tactile-pts540-series-datasheet; key p.2
  (`PTS540-J-[MK]-035-SMTR-LFS`) → PTS540JK035SMTR LFS add. Drawing p.1.

### PTS636 (Surface Mount 20, Through Hole 6; 1 pending)

- PDFs: littelfuse-c-k-tactile-pts636-series-datasheet and
  c-amp-k-tactile-pts636-series-datasheet are byte-identical (8 pp.). Key p.7,
  drawings p.2–5, table p.8. 2-terminal switch, pin 1 left.
- Pending PTS636SMG25SMTR LFS → add (key: S, force, G, 25, gull-wing, SMTR).
- Seven footprints by lead type / height: THT, SMD-G and SMD-J (4.3/5.0 mm,
  body 6 × 3.5), SMD-G25, SMD-G25G (gull-wing with ground pin: pads 6.1/9,
  differ from the no-ground 6.5/9), SMD-J25 (J with or without ground: same
  4/8 pads), SMD-F25 (body 6.1 × 3.7). Ground pads 3, 4 left out.
- GSPTS004102 is Littelfuse's code for PTS636SM50JSMTR LFS (p.8); the table's
  terminal column says "Gull Wing SMT" but the number has J (J lead) → mapped to
  SMD-J. Doubt: worth a look.
- Doubt: SMT J 4.3/5.0 land (p.3) prints 5.6 inner / 7.4 outer, 1.4 high
  (pad 0.9 × 1.4), but the pads are drawn square (≈1.4 × 1.4, i.e. inner ≈4.6).
  The printed numbers are used.

### PTS641 (Surface Mount 5, all pending)

- PDF: littelfuse-c-k-tactile-pts641-series-datasheet; key p.2
  (`PTS641-[ST]-[MKP]-(25|31|34)-SMTR2-LFS`) → 5 add. Layout p.2 (no ground pin):
  1.3 × 1 at 8.4 × 4.5 (centre-to-centre). Body 6.3 × 6.4.

### PTS645 (Surface Mount 20, SM right angle 2, Through Hole 24, TH right angle 17; 6 pending)

- Top push: littelfuse-c-k-tactile-pts645-series-datasheet (9 pp.; key p.7,
  drawings p.2–4). littelfuse-ck-tactile-pts645-series-datasheet is the same file.
  THT 4 × Ø1 at 6.5 × 4.5 (pad 1.7); SMT G 1.55 × 1.3 at 7.95 × 4.5; SMT J
  1.5 × 1.3 at 6 × 4.5 (J drawing letters A–D = 1–4). T (ESD pin) parts: pin 5
  left out, same 4 holes.
- Side actuated: littelfuse-c-k-tactile-pts645v-series-datasheet (5 pp.; keys
  p.4, table p.5, SMT drawing p.2, THT drawing p.3). 2-terminal.
  - SMT RA: two 1 × 1.9 pads 4.5 apart; the four bracket tabs not drawn.
    Explicit, body 7.1 × 6.6, pads at y +0.55.
  - THT RA: two Ø1 holes at 4.5; the two Ø1.3 bracket legs not drawn. Layout
    orientation kept (rear up, actuator down); body 7.5 × 8.1 (rear to actuator
    face, 4.2 + 3.9: the housing depth alone is not dimensioned), holes at
    y +0.15. Pin 1 left (front view).
- The 6 pending rows are side-actuated (V) parts whose DigiKey datasheet link is
  the top-push PDF, which has no V key; they are decided by the PTS645V key
  (p.4): VH58, VL39, VM39, VM58, VM83 add (also printed verbatim on p.5 of the
  V PDF); PTS645VJK61-2 LFS skip (no J actuator, no 61 length in the V key).

### PTS647 (Surface Mount 18)

- PDF: littelfuse-c-k-tactile-pts647-series-datasheet; layout and schematic
  p.2, body p.1. All add. 1.55 × 1.0 at 5.25 × 3.2. The body drawing prints
  terminal pitch 3 ± 0.2; the layout (3.2 centres) is used.

## Batch 2

## C&K batch 2 — PTS8xx, RK, RKC2, SFS, TLS, PTS831 (series "-")

Pinout convention: unless said otherwise, terminals are numbered 1 top-left, 2 top-right,
3 bottom-left, 4 bottom-right in the top view (dual `slots` [1,4,2,3]), and the schematic
joins 1–2 and 3–4 (`A=1,2;B=3,4`).

### PTS810 (Surface Mount, 4 rows, all verbatim)
- littelfuse-c-k-tactile-pts810-series-datasheet: p.1 dimensions, Recommended PCB Layout and
  schematic; p.3 ordering key / table.
- SW-TACT-PTS810: pads 1.05 × 0.65 (inner 3.1 / outer 5.2; rows 1.5 / 2.8). Body 4.2 × 3.2.
- Not the same as PTS815 (pads 1.0 × 0.7 at 4.5): kept separate.

### PTS815 (Surface Mount, 3 rows, verbatim)
- littelfuse-c-k-tactile-pts815-series-datasheet p.1 (land, circuit diagram), p.3 key.
- SW-TACT-PTS815: pads 1.0 × 0.7, centres 4.5 × 2.15. RK's RKB2 land (rk datasheet p.2)
  is printed with the same numbers and shares it.

### PTS820 (Surface Mount, 5 rows, verbatim)
- littelfuse-c-k-tactile-pts820-series-datasheet p.1: two drawings (2.0/2.5 mm high, and
  1.5 mm high with pegs); same land. p.3 key (J, height 15/20/25, force M/K, P pegs).
- Two terminals (1, 2) at 3.2 / 5.1, 1.4 tall: SW-TACT-PTS820, `A=1;B=2`. The four corner
  pads are frame tabs (grounded in the schematic) and the 2× Ø0.8 holes are pegs: not drawn.

### PTS830 (Surface Mount, 4 rows) + check row PTS830GG140GSMTRLFS
- littelfuse-c-k-tactile-pts830-series-datasheet p.1: two land patterns —
  no ground pin (SW-TACT-PTS830, pads 0.7 × 0.75 at 3.2 × 1.65) and ground-pin "G" version
  (SW-TACT-PTS830-G, pads 0.65 × 0.6 at 3.15 × 1.7; ground pads 5, 6 omitted). p.3 key.
- Body 3.05 ±0.3 × 2.6 ±0.3 (3.5 is over the leads).
- Check row PTS830GG140GSMTRLFS (pts830.pdf, same document as the series datasheet,
  p.1 / p.3): SPST N.O., non-illuminated, SMT gull-wing, 12 VDC 50 mA → keep, Surface Mount,
  Standard, 0.05A @ 12VDC; G version → SW-TACT-PTS830-G. Its spec row uses series `*`.

### PTS831 (DigiKey series "-", Surface Mount, 3 rows) — Cannot draw
- PTS831GX140SMTR_LFS_ProdSpec_RevB.pdf and PTS831GX140GSMTR_LFS_ProdSpec_RevA.pdf are
  11-page product specifications (ratings, tests, packaging, BOM). Both refer to "product
  drawing G01000444" / "G01016434" for structure and dimensions, which is not in either
  PDF: no dimensions, no land pattern, no terminal numbering. Listed under Cannot draw.

### PTS840 (Surface Mount, Right Angle, 8 rows, verbatim)
- littelfuse-c-k-tactile-pts840-series-datasheet p.2 (G leads) and p.3 (P front-PIP leads),
  key p.2.
- SW-TACT-PTS840 (G): pads 1.4 × 1.05 at 3.6 × 1.45. ESD pad 5 and peg holes omitted.
- SW-TACT-PTS840-P (P): pads 1.65 × 1.05 at 3.85 × 1.45; pads 3 and 4 carry a metallized
  through hole Ø0.65 +0.1/-0 (drill 0.7) at x ±2.025, y +0.8 — off the pad centre — so each is
  drawn as three touching rects, the one with the hole centred on it. Worth a look in K's render.
- Spec rows split by the SMT-style letter: `(ESD)?G[MK]P?` → G, `(ESD)?P[MK]` → P.

### PTS841 (Surface Mount, Right Angle, 7 rows: 5 verbatim, 2 pending)
- littelfuse-c-k-tactile-pts841-series-datasheet p.1 dimensions, p.2 land and circuit, p.3 key
  `PTS841 - [ESD] - G - M/K - [P] - SMTR - LFS`.
- Regex `PTS841 ?(ESD ?)?G ?[MK] ?P? ?SMTR ?LFS` (optional spaces between key fields, as
  DigiKey writes them). The two pending numbers (ESD GKP, ESDGMP) combine ESD and pegs: the
  key allows it, the p.2 "standard options" table lists only ESD without pegs — added per
  the key.
- SW-TACT-PTS841: pads 1.2 × 0.9 at 4.0 × 1.5.

### PTS845 (Surface Mount, Right Angle, 6 rows, verbatim; two PDFs)
- littelfuse-c-k-tactile-pts845-series-datasheet p.1 (with peg), p.2 (without peg), p.4 table;
  PTS845.pdf (2024 edition) p.2 shows the same lands.
- **Three terminals** in a row; the circuit diagram joins 1 and 3, the switch goes to 2.
  Modelled as a 2-terminal switch: pin 1 = datasheet terminal 1 with terminal 3 as its second
  pad (explicit style allows several pads per pin), pin 2 = terminal 2; `A=1;B=2`.
  The main session may prefer another treatment.
- Pad y from the side view: peg-hole line 2.05 from the front (0.85 + 1.2), body 3.4 deep,
  so the terminal pads (1.80 long behind that line) are at y −2.15…−0.35 from the body centre.
  Bracket pads and peg holes omitted.

### PTS847 (Edge Mount, Right Angle, 4 rows, all pending)
- littelfuse-c-k-tactile-pts847-series-datasheet p.2 (standard), p.3 (L-shape shield), p.4
  (soft actuator), key p.6 `PTS847 - MM - 160/350 - [L] - [S] - SMTR2 - LFS`.
- Regex `PTS847 ?MM ?(160|350) ?L? ?S? ?SMTR2 ?LFS`: all 4 add.
- Three terminals, 1 and 3 joined (same treatment as PTS845, `A=1;B=2`).
- SW-TACT-PTS847: pads 0.8 × 1.25 at x −1.15/0/+1.15. The board needs a 3.8 × 1.35 edge
  cut-out (not expressible in a footprint; noted). The hold-down pads and the L version's
  plated shield slots are not drawn. p.3 does not dimension the L version's pad length;
  the p.2 lands are used for both (same printed widths/pitch). Body = top-view outline
  4.6 × 2.6 (actuator tip 0.35 past the board edge), pads at y −1.425 from its centre.
  The y-placement relative to the body is derived from the side view; least certain.

### PTS850 (Surface Mount, Right Angle, 4 rows, verbatim)
- littelfuse-c-k-tactile-pts850-series-datasheet p.2 (without boss), p.3 (with boss, same lands),
  table p.1.
- SW-TACT-PTS850: pads 1.4 × 1.5, x centres 3.4, inner gap 4.3 +0.2/-0 (4.4 mid) → pitch 5.9.
- Schematic: **1–3 joined and 2–4 joined** (`A=1,3;B=2,4`), unlike the other series.

### RK (Surface Mount, 7 rows, verbatim)
- littelfuse-c-k-tactile-rk-series-datasheet p.1 (RKA2), p.2 (RKB2), key p.4.
- RKA2: T-shaped lands (0.5 × 0.7 inner part + 0.2 × 1.1 outer flange), drawn as two touching
  rects per pin: SW-TACT-RK (explicit). Body 4.5 × 4.5.
- RKB2: same land as PTS815 → SW-TACT-PTS815.

### RKC2 (Surface Mount, 1 row, verbatim)
- littelfuse-c-k-tactile-rkc2-series-datasheet p.2 standard version (no ground pin), key p.4.
- SW-TACT-RKC2: pads 1.2 × 0.975 (outer 2.5, gap 0.55) at 4.0 × 1.525. Body 3.5 × 2.7.

### SFS (Surface Mount, 3 rows, verbatim incl. orderable number G01743323 = SFS12YSSMTR LFS) — Cannot specify
- littelfuse-ck-tactile-sfs-series-datasheet (identical to littelfuse-c-k-tactile-sfs-series-datasheet)
  p.1 SMT drawing and PCB layout: pads 2.25 × 1.6 (11.30 max / 15.80 min, 5.00 pitch) —
  drawable, but **no terminal numbering and no circuit diagram** anywhere in the PDF, so which
  terminals are joined is not given. Listed under Cannot draw.

### TLS (Surface Mount, 6 rows, verbatim)
- littelfuse-c-k-tactile-tls-series-datasheet p.1 PCB layout, circuit, drawing; key p.2.
- Two terminals, 1.4 × 2.5 (2.5 min used) at 8.0 centres; not numbered in the datasheet
  (left = 1, right = 2; symmetric). Circuit: double-break bridge between them. SW-TACT-TLS.

### Cannot draw
- PTS831GG140GSMTR LFS, PTS831GG140SMTR LFS, PTS831GM140SMTR LFS — product specs only; the
  dimension drawing (G01000444 / G01016434) is not in the PDFs.
- SFS12RSSMTR LFS, SFS12WSSMTR LFS, G01743323 — land pattern printed, but no terminal
  numbering or circuit diagram, so the pinout cannot be read.

## Batch 3

### ATS (Surface Mount) — 2 rows, all add
- PDF: littelfuse-ck-ats-series-datasheet (3 p.). Key p.2 (ATS-2-D-x-G-NC-LFx), drawing + Recommended PCB layout p.2.
- SPST-NC anti-tamper switch, 2 gull-wing terminals. Layout: pads 1.4 high, inner 6, outer 8 → 1.0 × 1.4, centres 7.0. Body 6 × 3.8.
- No terminal numbers printed: left = 1, right = 2. Footprint SW-TACT-ATS, A=1;B=2.

### ED / EDM (Surface Mount, Through Hole) — ED 5 rows, EDM 7 rows (1 pending → add)
- PDF: littelfuse-c-k-tactile-ed-edm-series-datasheet (3 p.). Keys p.3; drawings p.2 (ED/EDM SC with tabs, ED/EDM with pc pins).
- 2-terminal sealed disc keyswitch, SPST-NO. No terminal numbers printed: left = 1, right = 2.
- SC (SMD): "implantation circuit imprime" pads 5.4 ±0.5 high, inner 10 +0.2/−0 (mid 10.1), outer 15 ±0.2 → 2.45 × 5.4 at ±6.275. Body Ø11.8 ±0.15 (round). SW-TACT-ED-SMD (shared ED/EDM).
- AC (THT): pins Ø0.8 ±0.01 at 11.36 (basic), no hole layout printed → drill 1.05 (pin + 0.25, plan-03-B R5), pad 1.8 (1.7 × drill). SW-TACT-ED-THT (shared ED/EDM). Doubt: drill is derived, not printed.
- EDM regex `EDM(450|650)A[GU](AC0LFS|SC0LF[SG])`: the key (EDM-force-S/G-AC/SC-LFx; AC only with LFS) is written in the table's orderable form (A + G = silver / U = gold, terminals, 0, LFx). EDM650AUSC0LFG (650 gold SC LFG, the table only lists the 450 version) → add.

### EL2 (Surface Mount) — 2 rows, all add
- PDF: littelfuse-c-k-tactile-el2-series-datasheet (3 p.). Key p.3; body p.1; Layout - Pad p.2 (3.5 and 5.2 mm heights identical).
- Pads 1 high at 4 pitch, inner 3 max, outer 8.6 min → 2.8 × 1.0, span 5.8. Body 6.2 +0.3/−0 square → mid 6.35 × 6.35.
- No terminal numbers printed: numbered counter-clockwise from top left; the layout's switch symbol joins the top pair and the bottom pair → A=1,4;B=2,3. SW-TACT-EL2.

### KMR2 / KMR6 / KMR7 (Surface Mount) — 19 / 6 / 5 rows, all add
- PDFs: littelfuse-c-k-tactile-kmr2-series-datasheet (layout p.1, diagram p.2, key p.3, table p.4), littelfuse-c-k-tactile-kmr6-series-datasheet (p.1-3), littelfuse-c-k-tactile-kmr7-series-datasheet (p.1-3).
- All three: recommended layout 5 / 3.2 / 2.6 / 0.6 / 1 → pads 0.9 × 1.0 at x ±2.05, y ±0.8; terminals 4 TL, 1 TR, 2 BR, 3 BL; diagram 4–1, 3–2 → A=1,4;B=2,3. Body 4.2 ±0.2 × 2.8 ±0.2. One footprint SW-TACT-KMR2 (dual, slots [4,3,2,1]).
- KMR2 G versions have a ground pin 5 (1.7 × 0.55 pad, top centre): not a switch terminal, not drawn. KMR6/KMR7 are NG only.
- Y78B62324FP (KMR6) is the Littelfuse orderable number of KMR623NG ULC LFG (kmr6 p.3).

### KMS (Surface Mount, Right Angle) — 8 rows (1 pending → skip)
- PDFs: littelfuse-c-k-tactile-kms-series-datasheet and kms.pdf (same content). Drawings p.1 (without and with pegs), key/table p.3.
- Layout same pads as KMR2 (5 / 3.2 / 2.6 / 0.6 / 1), centred on the 2.8 body (bottom view 2.2 tab span symmetric). Body 4.2 × 2.8 housing; actuator protrudes 0.75 below (3.55 overall).
- Diagram: 2–3–4 on one line, 1–5 on the other. Terminal 3 is the top-centre frame tab (1.7 × 0.55, joined to 2 and 4): not drawn (4-terminal model). Pegs (P, Ø0.9 holes) not drawn.
- DOUBT: the drawing does not put numbers on the pads. Placement inferred from the diagram (3 on the 2–4 line, at the top tab): top row 2 (left), 4 (right); bottom row 1 (left), 5 (right). Renumbered to 4 pads: pad 1 = DS 1 (BL), pad 2 = DS 2 (TL), pad 3 = DS 4 (TR), pad 4 = DS 5 (BR); pins A=2,3;B=1,4 (top row / bottom row, as on KMR). SW-TACT-KMS (slots [2,1,4,3]).
- Regex `KMS2[23][13]GP? ?LF[SG]` (KMS-2-force 2/3-contact 1/3-G-peg none/P-LFx). KMS231G LFS SERIES 2 → skip (the "SERIES 2" suffix is not in the key).

### KMT / KMT0 (Surface Mount) — 4 / 8 rows, all add
- PDFs: littelfuse-c-k-tactile-kmt2-series-datasheet = kmt2.pdf (byte-identical; layout + diagram p.2, key p.3), littelfuse-c-k-tactile-kmt0-series-datasheet (layout + numbered diagram p.1, key/table p.3).
- Both layouts: outer 3.6, inner 2.3, 2.6 overall, 1.2 gap → pads 0.65 × 0.7 at x ±1.475, y ±0.95. Body 3.0 × 2.6. One footprint SW-TACT-KMT.
- KMT0 numbering 1 TL, 2 TR, 3 BR, 4 BL; diagram 1–2, 4–3 → A=1,2;B=3,4. KMT2 prints no numbers (diagram: top pair / bottom pair joined) → KMT0 numbering used, same pins. KMT2 G versions' side ground pin not drawn.

### Check row
- KMR241GULCLFS (series `*`, kmr2.pdf = the KMR2 datasheet; KMR241G ULC LFS): keep, SPST-NO, Surface Mount, Standard, 0.05A @ 32VDC (silver). Specs row under series `*`, mounting Surface Mount, footprint SW-TACT-KMR2.

## Batch 4

## C&K batch 4 — KSA KSE KSJ KSR KSS KST KT KXT3 NanoT (+ check NANOT160BP)

### KSA (Through Hole) — ksa_ksl.pdf
- Key p.5: `KS-A-[0|1]-[M|V]-[2|3|4|9|5]-[1|3]-1-LF[S|G]`; regex `KSA[01][MV][23459][13]1 ?LF[SG]`.
- Pending `KSA0M411LFTR` → **skip**. Its `LFTR` is not a value of the LFx field (S or G only), and the datasheet
  lists only bulk boxes (p.1 "Delivered in boxes of 500 pieces"), no tape and reel. `TR` takes the place of the plating
  letter, so it is not a packaging suffix added on the end. If the main session wants it anyway, the alternative is
  `KSA[01][MV][23459][13]1 ?LF(S|G|TR)`. But note that the PDF has **no terminal numbering or circuit diagram** (p.2–3 show only
  holes: 4 x dia 0.9 +0.1/0 on a 5.08 x 7.62 grid). Its A/B pairing could not be read, so it would land in "Cannot draw (pinout)".

### KSJ (Through Hole) — ksj.pdf
- Key p.3: `KSJ-0-[M|V]-[2|4|9]-[1|3]-1-[60|80]SH-LF[S|G]`; regex `KSJ0[MV][249][13]1 ?(60|80)SH ?LF[SG]`.
- Pending `KSJ0M41180SHLFTR` → **skip**, for the same reason as KSA: LFTR is not in the key, and p.1 says "boxes of 250 pieces".
  KSJ has the same 7.4 mm family layout and also no circuit diagram.

### KSE (Surface Mount) — littelfuse-c-k-tactile-kse-series-datasheet
- All rows were verbatim adds. Drawing p.2: G pad layout 12 mini / 5.8 maxi / pad 1 / row pitch 4; J pad layout 8.6 mini / 3 maxi.
- The circuit is drawn inside the pad layout figure: the two top pads are joined, the two bottom pads are joined, and the
  blade runs between them. The terminals are not numbered, so the pins are numbered by the dual default
  (counter-clockwise from top-left) → `A=1,4;B=2,3`.
- Body: *6.2 +0.3/0 → 6.35 wide, *6.25 +0.3/0 → 6.4 high.
- Footprints SW-TACT-KSE-G, SW-TACT-KSE-J.

### KST (Surface Mount) — littelfuse-ck-tactile-kst-series-datasheet
- All rows were verbatim adds. Drawing p.1: J terminations 8.6 mini / 3 maxi / 1 / 4, G terminations 12 mini / 5.8 maxi / 1 / 4.
  These are identical to KSE, with the same circuit figure. Body 6.2 +0.3/0 square (6.35), within 0.05 of KSE → **shares
  SW-TACT-KSE-G / -J**.

### KSR (Surface Mount) — littelfuse-c-k-tactile-ksr-series-datasheet
- Keys p.3 (`KSR-2-[1|2|3|5]-[1|3]-G-[NC]-LF[S|G]`, tape option T16 in the table) and p.6 (long life,
  `KSR-2-x-[2|4]-G-LF[S|G]`). Regexes `KSR2[1235][13]G( T16)?( NC)? ?LF[SG]` and `KSR2[1235][24]G ?LF[SG]`.
- Pending `KSR551G LFS` → **skip**. The actuator field is only `2` (soft actuator); 5 is not in the key.
- Drawing p.1 (p.5 is identical): 2 pads, inner 6, outer 8, height 1.4 → pad 1 x 1.4, centres 7. Body 6 x 3.8.
  The terminals are not numbered, so pin 1 = left pad. The NC versions (KSR223G NC LFG) use the same pads.
- Y35A21410FPLFG (Littelfuse number of KSR214G LFG, long life) is covered by `.*`.

### KSS (Surface Mount, Right Angle) — littelfuse-c-k-tactile-kss-series-datasheet
- All rows were verbatim adds. PCB proposed layout p.1: 7.3 outer, pads 1.55 (2x) x 3.2, actuator protrudes 1.05 below.
- The layout is turned 90° from the top view. The body is the housing 5.5 x 3.5 +0.15/0 (3.575) in the layout orientation.
  The 3.1 terminal tab is centred on the 3.5 housing (`==` marks), so the pads are on the body centre line.
- The terminals are not numbered; pin 1 = left pad.

### KT (Surface Mount; Surface Mount, Right Angle) — littelfuse-c-k-tactile-kt-series-datasheet
- Key p.5: `KT11-[B0|B1|B2|P2|P3|P4|S1|S2]-[SM|JM|SAM|SA1M|SA2M|SA3M]-3-[4|5]-LF[S|G]`; regex
  `KT11(B[012]|P[234]|S[12])(SM|JM|SAM|SA[123]M)3[45] ?LF[SG]`.
- The key's feasibility notes are not enforced in the regex (B0–B2 with A/A1/A3; P with A1/A2; S with A1/A2). The p.5 table
  itself lists combinations outside those notes, e.g. P2SA3M and S1SA3M.
- Pending `KT11B0SAM34LFS` → add. B0 is allowed with the "A" (SAM) bracket per note 1.
- Top SM (gull-wing) p.3: 10.92 outer / 5.59 inner horizontal, 5.59 outer / 2.41 inner vertical → pad 2.665 x 1.59, span 8.255,
  pitch 4.0. Top JM (J-bend) p.3: 8.38 / 2.03 → pad 3.175 x 1.59, span 5.205, pitch 4.0.
- Pins are printed 1 TL, 2 TR, 3 BL, 4 BR (slots [1,4,2,3]). Schematic p.3: 1–2 joined, 3–4 joined → `A=1,2;B=3,4`.
  Body 6.4 ±0.3 x 6.3 ±0.3. Footprints SW-TACT-KT-SMD-G, SW-TACT-KT-SMD-J.
- Right angle p.4 (SAM/SA3M and SA1M/SA2M pad layouts): only terminals **2 and 4** are electrical (schematic p.4: 2–4).
  The mounting-bracket pads and the SA1M/SA2M front solder pad are not drawn. Terminals 2 and 4 are renumbered
  **pins 1 and 2** (1 = terminal 2, the upper pad).
- Terminal pads 3.76 x 1.205: inner gap 2.54, outer 4.95, so centres ±1.8725. The front edge is 1.70 behind the switch face.
  Body 4.38 deep (from the switch face, SA1M side view p.3) x 6.30 wide. The pads extend 1.08 behind the body.
  Lead pitch is printed as 4.00 on p.3; the pad layout gives 3.745 between pad centres. The 0.76 leads fall within the pads.
  The pad layout was used. Footprint SW-TACT-KT-SMD-RA.

### KXT3 (Surface Mount) — littelfuse-c-k-tactile-kxt3-series-datasheet
- All rows were verbatim adds. Recommended PCB layout p.1: 3.8 outer, 2.7 inner, 1.5 high → pad 0.55 x 1.5, centres 3.25.
  Body 3 x 2. Pin 1 = left pad (not numbered).

### NanoT (Surface Mount; Surface Mount, Right Angle) + check row NANOT160BP — littelfuse-c-k-tactile-nanot-series-datasheet
- All series rows were verbatim adds. Key p.3: `NanoT-[100|160|240]-[A|B]-[S|P]` (S top only, P side only).
- AS (top) p.2 PCB layout: hatched land 2.8 outer, 1.8 inner, 1.65 high. It is drawn as plain rects 0.5 x 1.65, centres 2.3.
  The land's notched outline is simplified to rectangles. Body 2.1 x 1.7 (p.1). Pins: schematic 1–2; pin 1 = left.
  Footprint SW-TACT-NANOT-SMD.
- BP (side, PIP = pin-in-paste through-hole reflow, edge-mount) p.2 view F: 2 holes dia 0.4, lands dia 0.6, centres 2.1,
  pins 1 left / 2 right as printed. Body 2.2 x 1.7 +0.15/-0.10 (1.725) deep. Hole line 0.25 from the body back
  (p.1: lead centre 0.85 behind the 0.6 front block; total 1.7), so holes at y = -0.6. The actuator faces +y, past the board edge.
  Footprint SW-TACT-NANOT-PIP.
- DigiKey lists NANOT 100 BP as "Surface Mount" but 160/240 BP as "Surface Mount, Right Angle". The specs carry a BP row
  in both groups, with the same footprint.
- Doubt: the datasheet calls BP a "PIP edge-mount". "Edge Mount, Right Angle" is arguably the truer mounting, but DigiKey's
  values were kept.
- Check row NANOT160BP (series `*`): keep; SPST-NO, non-illuminated (p.1, "1 make contact = SPST, NO").
  Mounting "Surface Mount, Right Angle" to match DigiKey's twin listing NANOT 160 BP. Actuator Standard (as the twin).
  Rating 0.02A @ 15VDC (p.1: 15 V, 20 mA). Specs row uses series `*`, regex `NANOT160BP`, footprint SW-TACT-NANOT-PIP.

### Cannot draw
- none among kept rows. KSA0M411LFTR and KSJ0M41180SHLFTR were skipped by the key; they would also lack a pinout, see KSA above.

## Batch 5

### KSC family (batch 5): KSC, KSC-OF, KSC-PF, KSC-ST, KSC-TE, KSC-XA, KSC10, KSC11, KSC4 TE, KSC4D, KSC5

PDFs read (page 1 drawings as images, ordering keys as text):
- KSC series spans seven datasheets: littelfuse-c-k-tactile-ksc1-series-datasheet (p.1 drawing + terminations, p.3 key),
  ksc2.pdf (p.1, key p.3), littelfuse-c-k-tactile-ksc3-series-datasheet (terminations p.2, key p.3),
  littelfuse-c-k-tactile-ksc4-series-datasheet (p.1, key p.2), littelfuse-ck-tactile-ksc6-series-datasheet (p.1),
  littelfuse-ck-tactile-ksc7-series-datasheet (terminations p.2), littelfuse-c-k-tactile-ksc8-series-datasheet (p.1).
- KSC-ST rows are in the KSC4 datasheet (p.4 acoustic versions); KSC-OF littelfuse-ck-tactile-ksc-of-series-datasheet p.1;
  KSC-PF littelfuse-ck-tactile-ksc-pf-series-datasheet p.1; KSC-TE littelfuse-ck-tactile-ksc2-te-series-datasheet p.1;
  KSC4 TE littelfuse-c-k-tactile-ksc4-te-series-datasheet p.1; KSC-XA KSC%20XA_Datasheet.pdf p.2 (PCB Layout);
  KSC10 littelfuse-ck-tactile-ksc10-series-datasheet p.1; KSC11 littelfuse-c-k-tactile-ksc11-series-datasheet p.1;
  KSC5 littelfuse-c-k-tactile-ksc5-series-datasheet p.2 (Pad Layout for J/G Versions); KSC4D littelfuse-c-k-ksc-d-series-datasheet p.2
  and KSC4D_Series.pdf p.1-2.
- Every one prints the same "J Terminations" / "G Terminations" LAYOUT-PAD figure (scale 5): pads 1 tall, 4 between
  rows; J: 3 max inner, 8.6 min outer -> pad 2.8 x 1.0 at x = +-2.9; G: 5.8 max inner, 12 min outer -> pad 3.1 x 1.0 at
  x = +-4.45. So two shared footprints, SW-TACT-KSC-J and SW-TACT-KSC-G, chosen per part by its termination letter
  (first G/J after "KSC": mpn_regex `KSC[^GJ]*J.*` / `KSC[^GJ]*G.*`).
- Body 6.2 +0.3/0 square -> 6.35 x 6.35. KSC XA, KSC2 TE, KSC4 TE print 6.3 +0.3/0 wide (6.45): within 0.1 of 6.35, shared.
- Terminal numbering: none of these datasheets numbers the four terminals. The digits moulded on the body top view
  ("1"/"0", "1"/"2", "1"/"3") are housing markings, one per terminal row, not pin numbers. The circuit in the
  terminations figure joins the two pads of the top row and the two of the bottom row, the switch between rows. The
  footprint uses the default dual order (1 top-left, 2 bottom-left, 3 bottom-right, 4 top-right), so pins A=1,4;B=2,3.
- KSC4D is a "Sealed Detect Switch" (momentary, compatible with permanent actuation), SPST-NO, same pad layout.
  Pending rows decided by the key: littelfuse-c-k-ksc-d-series-datasheet p.2 `KSC4-D-{1,2,3}-{G,J}-50SH-{None,ULC}-{LFS,LFG}`
  (ULC gold only) and KSC4D_Series.pdf p.1 (same without ULC). All three pending rows add.
- KSC8 rows with "OL" (KSC821GOLLFS, KSC841G OL LFS …) were verbatim adds; same layout.

### KSC4V2 (Through Hole, Right Angle)

littelfuse-ck-tactile-ksc-v2-series-datasheet p.2 "PCB Layout, Pin in Paste, Top View": two metalized holes 1.1 +-0.05,
lands 1.6 +-0.1, 4 apart (2 + 2). Three SMD ground pads (two side tabs 0.8 x 1.5 and a front bar) are "SMD ground
terminals", not switch terminals: not drawn. Two terminals, SPST-NO (p.1), A=1;B=2 (1 left, 2 right, not numbered in the PDF).
Odd: the body depth on the PCB is not dimensioned; 2.5 is scaled from the drawing (housing about -1.15..+1.25 about the
hole line) and the holes are kept on the body centre line. Body width 6.2 from the title "6.2 x 6.5 mm".

### RS, COMAX (C&K-sold COMAX drawings)

- RS-014R05B1-SMA10 RT — RS-014R05__-SMA%20_RT.pdf p.1, item `RS-014R05__-SMA RT` (L A/B, force H/0/1/3/4/5). The "10"
  after SMA is not in the item number: **skip** (regex `RS-014R05[AB][H0-5]-SMA ?RT`).
- RS-032G05A3-SM RT — RS-032G05_-SM_RT.pdf p.1, item `RS-032G05___-SM RT`, H table A/B, force 1/3/4 → add. Piercing plan
  2-2x2 LAND at 8 centres; body 6 x 3.5. SW-TACT-RS-COMAX-032.
- RS-282G05A3-SM RT — RS-282G05A-SM_RT.pdf p.1, item `RS-282G05A_-SM RT`, force 1/3 → add. 2-1.5x1.5 LAND at 7.8 centres;
  body 6 x 3.6 (3.6 from the side view). SW-TACT-RS-COMAX-282.
- RS285G05A2URT — RS285G05A2URT_Dwg.pdf p.1, item `RS-285G05A2-U RT` (a single part; regex allows DigiKey's dropped
  hyphens/space) → add. Switch pads a, b: 8 outer, 4 inner → 2 x 1 at x = +-3; terminal c is a ground (two 1 x 1 pads)
  — not a switch terminal, not drawn. Body 6 x 3.6. SW-TACT-RS-COMAX-285.
- RS022R05B0 (Through Hole, Right Angle) — RS022R_Dwg.pdf p.1, item just `RS-022R` with an L table (A–H) and a force
  table (A/H/0/1/3). **Unsure**: the drawing does not print the order of the suffix; I accepted `RS-?022R05[A-H][AH013]`
  by analogy with the other COMAX items (`…R05` + L + force, e.g. RS-014R05B1). Main session may prefer to skip it.
  Piercing plan: terminals 2-0.9 holes 4.5 apart, 2.5 in front of the 2-1.3 bracket-leg holes (7.0 apart, not drawn).
  Pad 1.7 x 0.9 → 1.55. Body 7.15 x 6.45: depth not dimensioned, scaled from the side view with the printed 2.5
  dimensions; terminals 0.75 below body centre (actuator side down). SW-TACT-RS-COMAX-022.
- All RS: 1P-1T schematic, no terminal numbers: 1 left, 2 right, A=1;B=2.

### Check rows

- KSC421G70SHLFS (ksc4.pdf, KSC421G 70 SH LFS): keep; SPST-NO, Surface Mount, Standard, 0.05A @ 32VDC (silver). Footprint SW-TACT-KSC-G.
- KSC441JST2LFS (ksc4.pdf, KSC441J ST2 LFS): keep; SPST-NO, Surface Mount, Standard, 0.05A @ 32VDC. Footprint SW-TACT-KSC-J.
- Their specs rows use series `*` (as in candidates.tsv) and mounting `Surface Mount`; ksc4.pdf p.1 prints the same
  3 max / 8.6 min and 5.8 max / 12 min terminations.

### Cannot draw

None.

## Cannot draw

The main session skips these (they were `add`, verbatim):
- PTS831GG140GSMTR LFS, PTS831GG140SMTR LFS, PTS831GM140SMTR LFS (series `-`): PTS831GX140SMTR_LFS_ProdSpec_RevB.pdf and PTS831GX140GSMTR_LFS_ProdSpec_RevA.pdf are product specifications that refer to drawings G01000444 / G01016434, not included: no dimensions, no land pattern, no terminal numbers.
- SFS12RSSMTR LFS, SFS12WSSMTR LFS, G01743323 (SFS): littelfuse-ck-tactile-sfs-series-datasheet p.1 prints a 4-pad land pattern but no terminal numbering and no circuit diagram, so which terminals are joined cannot be read.

## Counts

- C&K rows read: 456 (409 verbatim add, 47 pending).
- scheme-check: 41 add, 6 skip, 0 still pending (skips: KMS231G LFS SERIES 2, KSA0M411LFTR, KSJ0M41180SHLFTR, KSR551G LFS, PTS645VJK61-2 LFS, RS-014R05B1-SMA10 RT).
- checks.tsv: 5 rows, all keep, 0 out.
- Cannot draw: 6.
- Kept rows (add, with a spec row): 444.
- Footprints: 55. Specs rows: 89. Schemes rows: 24.
