# plan-27-C — TE Connectivity ALCOSWITCH datasheet reading (slug te)

Rows: TE Connectivity ALCOSWITCH Switches, `add` / `pending` in
`planning/plan-27-data/candidates.tsv` (220 rows: 137 add, 83 pending, 1 of
them `check`). Started on /tmp/p27/pre-cand.tsv (211 rows); the final census
added 9 rows (FSM*JS6*SMTR x7, UMSASMTR, UMSDSMTR), covered below.

PDF names: te.com DocumentDelivery files are cited by their DocNm (or the
DocId file name). The ~124 te.com URLs resolve to a handful of documents:

| DocNm | what | rows |
|---|---|---|
| 8-1773450-9_SECTION_C_TACTILE_SWITCHES | 2008 catalog, Section C, 50 pp: per series a key page and a drawing/table page | most FSM* series (46 rows link it) |
| 1308111-1_SWITCHES_CORE_PROGRAM_CATALOG | 30-pp switch overview, preferred-part tables only (p.17-19), no drawings, no keys | 48 rows link it; their drawings and keys are taken from Section C (same series) |
| ALCOSWITCH_Sealed_Tactile_Switches | 2026 STS/FSMJS6/UMS catalog, 9 pp: key p.6/8/9, drawings p.4-6 | STS, FSMJS6, UMS |
| 2337232-1 / 2337236-1 | MCSLPT / MSLPT family sheets (text) | LPT |
| 2337224-1, 2337225-1, 2337227-1, 2337229-1, 2337230-1 | USLPT family sheets, image only (read as images) | LPT (USLPT) |
| customer drawings 1977120, 1571300 (ENG_CD_1571300_K1), 1825107, 1437566-3, 1-1437565-8, ... | image-only single drawings | used where they list the row |
| 1654270-2 (116-pp mobile catalog), FSM Series.pdf (DigiKey copy) | old FSM.JSMA numbering tables | FSMJSMA/JSML pending rows |
| alcoswitch-tactile-switches 2212 | 2-pp brochure, part list only | 2425578-1, 2425754-1 (cannot draw), 2425753-x (drawn from the STS catalog p.4) |

Drawings were read as rendered images (pdftoppm 200-700 dpi crops); every
footprint `note` carries the arithmetic. Orientation rule: the land/hole view
as printed; where that view has no numbers, the numbers come from the top view.

## Per series

### FSMJ (Through Hole, 24) + FSMCD (TH, 2)
- Section C p.10 key `FSM<len>J[L|A|AA]H`, p.11 drawing, p.12 table (CPNs).
  Pending 1977120-2/-6, 1-1977120-0: drawing 1977120 (image) lists them
  (FSMxJ..HX, high-temp) -> CPN regex from its table.
- 4 holes dia 1.00 on 4.5 x 6.5 (top view 3 TL, 1 TR, 4 BL, 2 BR, 1-2 / 3-4).
  The hole view is drawn turned 90 deg against the top view; the footprint
  follows the top view (the only view with numbers). Ground-terminal codes
  (1825955, odd length codes) add an ESD terminal 5: not drawn.
- FSMCD vertical (p.14) has the same holes/numbering -> SW-TACT-FSMJ.

### FSMRACD (TH RA, 2), FSMRAJ (TH RA, 11), FSMRAJS THT (1571610-2)
- FSMRACD: p.14; FSMRAJ: p.17 key, p.18 drawing + CPN table (pending
  1-1825027-2, 1825027-6, 2-1825027-4 are in it). FSMRAJS THT p.33 = FSMRAJ layout.
- Right-angle bodies: no printed dimension ties the case to the terminal hole
  line; the offset was measured from the side view at drawing scale
  (FSMRACD 0.25 behind centre, FSMRAJ 0.6). Frame legs not drawn.

### FSMJRT (TH, 4) and `*` check row 1571010-1
- p.15 key `FSM(2|4|6|8|14)JA?RT`, p.16 drawing (2 holes 0.99+0.10/-0 at 5.0,
  vertical pair). 1571010-1 = FSM14JRT (p.16 table): check row -> keep,
  SPST-NO, Through Hole, Standard, 0.05A @ 24VDC (p.15 spec).

### FSMJSM / FSMJSMA / FSMJSML / FSMJSMXL (SMT, 29) and FSM14JSMASTR (series FSM)
- p.19 key `FSM(2|4|6|8|14)JSM(XL|L|A|AA)S(TR)?`, p.20 drawing + table.
  Older numbering without the S (FSM2JSMA, FSM4JSMATR, ...) from FSM Series.pdf
  p.8 / 1654270-2 p.95, plus a CPN regex.
- Skips: 1571563-6, 1571563-8, 1571563-9, 1-1571563-4 (no document lists
  these numbers).
- Land: the `.122 (3.0)` dimension contradicts 5.9 outer - 2 x 1.3 pads; used
  5.9 outer with 1.3 pads -> 4.6 centres, matching the 4.52 terminal pitch.

### FSM1LP (7), FSMLPA (1)
- p.21 key `FSM[14]LPA?S(TR)?`; old numbers FSM1LP/A/TR from drawing
  1-1437565-8 (image, same land); 1977263-1 from the p.22 table. Ground 5 not drawn.

### Extended Life (12)
- p.23 key, p.24 table (1571625-x). Two footprints: G/GE (`4-1.4x1.6`, read
  width x height — the drawing does not say which way) and J (pad width (3.2)
  REF, gap 3.6, outer 10.0; height 4-1.0 MIN). GE ground 5 not drawn.

### FSMJM (2), FSM 1825966-1 (FSM1L THT), FSMSM (2), FSMJMLP (1), FSMSMLPA (1), FSMCT (2)
- p.4-5 (FSM.SM/JM), drawing 1437566-3 (FSMSM, lists FSMSM/FSMSMTR), p.8-9
  (JMLP/SMLP: JM and SM land identical -> one footprint), p.26 (FSMCT; 8-1437565-1
  from its table). All 2-terminal except FSMCT.

### FSM100 ~ 104 (6), FSM 1571218-3, `-` 1571220-1
- p.36 key `FSM10[0-4]A?`; FSM100-103 THT holes 1.30 on 12.5 x 5.0;
  FSM104 is right-angle, 2 terminals (holes 1.50 at 5.0; body offset from side view).
- Sealed FSM100S THT (1571220-1, holes 1.30+0.10 -> 1.35, separate footprint)
  and FSM100SSMTR (1571218-3, p.39 SMT pads 2.25 x 2.2).

### Series `-` (DigiKey gave no series) — one group per drawing
- FSMJS SMT (FSM1JSSMTR, FSM2/4JSSMTR, 1571294-3 = FSM4JSSM): p.29 key, p.30 land, p.31 table.
- FSMJX (1571634-1/2): p.25; identical land to STS drawing 2506465 -> shared id SW-TACT-FSMJX.
- FSMRAJS SMT (1571407-1, SMD RA): p.33; terminal pads only; body offset derived from the side view.
- FSMRAJSA (1571300-2, -3): p.35 and drawing 1571300 (lists -2, -3; it also
  confirms the pad extent 1.00 / 2.50 about the guide-pin line, which p.35 leaves undimensioned).
- FSMMST (1571262-1/2/4, side-actuated): p.46; A/B/C/D = 1/2/3/4, A-C and B-D joined.
- 1825107-1: drawing 1825107 (image) = STS 2506467 land within 0.05 -> SW-TACT-STS-J.
- UMS 2425753-1/2 (and series UMS: UMSASMTR, UMSDSMTR): STS catalog p.4 drawing 2425753, 2 pads.
- FSM2JS6SMTR (and series FSM: FSM2JS6ASMTR, FSM3JS6ASMTR, FSM4/5/6JS6SMTR, FSM6JS6BSMTR):
  STS catalog p.4 drawing 1977265 prints **no land pattern**; pads derived from the
  lead drawing (feet 8.40-10.00, leads 0.70 at 4.50): foot + 0.3 heel/toe, lead + 0.15
  per side = 1.4 x 1.0 at 9.2 x 4.5. Flag for review.
- Cannot draw: 2425578-1, 2425754-1 (brochure only, no drawing).

### FSMMSH (1977067-1; FSMMSHP skipped)
- p.41 key `FSMMSHP?TR` (packaging TR only) -> FSMMSHP is not producible: skip.
- 3 terminals, 1 and 3 common: explicit footprint with pin 1 on both outer pads.
  The terminal row's offset from the body was measured at drawing scale (no dimension).

### STS (30)
- Key p.6 `STS[len][version][force][colour][mount][pack]` spelled out from the
  code table (length codes incl. blank, versions A-E or blank, forces
  10/16/18/20/25/26/35/50, colours A B C E G H O R Y W, mounts SM TH JT RS RT,
  packaging blank/R/TR). Note the catalog's own table violates the key in places
  (STS26A26RJ TTR, STS52BLOJTR, force 28 in STS31A28CSMTR, which is `add` verbatim).
- Skip: STS35B18JTTR (no colour letter; the key has no blank colour).
- The drawing is per TCPN (2506465 ... 2506476); rows are mapped to drawings by
  the p.7 product table (height/version), giving one spec row per drawing:
  26A SM -> 2506465 (= SW-TACT-FSMJX), 25/31/50A SM -> 2506469, 72/77A SM -> 2506470,
  26/34A JT -> 2506467, 31A JT -> 2506472, 35/52B -> 2506471, 43/70A TH -> 2506473
  (2 terminals), 12/13D TH -> 2506474 (2 terminals), STS80JS6 -> 2506475, STSC -> 2506476.
- STSC18BRTTR: DigiKey says Through Hole, Right Angle; drawing 2506476 is a reflow
  pad pattern with two locating pegs. Mounting left as DigiKey's; pads per drawing.
- 2506473/2506474 hole views have no numbers: top = 1.

### LPT (50)
- MCSLPT 2337232-1: 4644 (p.2 circuit/key, p.3 land; PH variant has a different
  land -> own footprint), 4848A / 4848C (p.8 circuit T1-T3 / T2-T4, p.9 lands).
- MSLPT 2337236-1: AG/BG/CG one land, AL/BL/CL one land (p.3-5); T5/T6 ground not drawn.
- USLPT (image sheets, pending): keys on p.2 of each give
  USLPT2616CT2TR, USLPT2819DT[24]TR, USLPT3020DT[236]TR, USLPT3026[BE]L[12]TR,
  USLPT3737AL[25]TR; lands p.3. USLPT2616 joins 1-3 / 2-4; USLPT3020 is 2-terminal.

## Cannot draw
- 2425578-1 — brochure (alcoswitch-tactile-switches 2212) only; no drawing.
- 2425754-1 — same brochure; no drawing.

## Counts
- Rows 220 (137 add + 83 pending incl. 1 check). Scheme-check: 77 add, 6 skip
  (STS35B18JTTR, FSMMSHP, 1571563-6, 1571563-8, 1571563-9, 1-1571563-4).
  Check rows: 1 keep, 0 out. Cannot draw: 2. Kept rows with specs: 212.
- Footprints: 45 (fp-load: `45 new footprints: load ok`). Specs rows: 58. Schemes rows: 37.
