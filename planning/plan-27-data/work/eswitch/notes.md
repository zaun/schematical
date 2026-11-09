# E-Switch — plan-27 B reader notes

Worked from the pre-census copy (`/tmp/p27/pre-cand.tsv`, `/tmp/p27/pre-paths.tsv`):
71 E-Switch rows with decision `add` (8) or `pending` (63), 13 PDFs (12 series).
There are no `check;` rows, so `checks.tsv` has only its header.

## Per series

- **320**: `320.04E11GRYVERT.pdf`, 1 page (one drawing per part number; the `add` row is verbatim).
  PCB LAYOUT + bottom view p.1. It has two terminals, numbered **1 and 3** (SPST). Pin 2 here is
  datasheet terminal 3 (the 1…N rule). The two dia 1.00 alignment pins are not drawn. The holes
  sit on a 2.54 grid, and the bottom view's 2.42 offsets put that grid on the centre of the
  12.45 square body. → `SW-TACT-320`.
- **GS**: `GS%20Series.pdf`. The key is on p.1. The lengths marked * are right-angle only. They
  run 1.64 mm longer than the standard lengths, so the regex gives the standard lengths to GS/GSE
  and the starred ones to GSR. TLE-style earth ground (GSE) stays in the regex because the key
  allows it, but no GSE row exists. GS THT p.1: four Ø0.90 holes on 7.62 × 5.00, schematic
  1–3 / 2–4 → `SW-TACT-GS-THT`. GSR p.2 has only terminals 1 and 2 (5.08 apart). The Ø2.00
  bracket-leg holes are not drawn. The front edge is at the top as in the P.C. MOUNTING
  drawing, and the front view puts terminal 1 on the viewer's left, i.e. +x here. The pin
  side does not matter electrically for a 2-terminal part. Body depth is 6.00 from the side
  view (2.00 + 4.00) → `SW-TACT-GS-THT-RA`.
- **TL1018**: `TL1018.pdf`, key p.1, layout + schematic p.2 (and p.3 for the ground-terminal
  option B). The ground pads 5/6 of option B are left out. Both rows share `SW-TACT-TL1018`.
- **TL3301**: `TL3301.pdf`, key p.1. Gull-wing layout p.2 (and the SP/T actuator pages p.4, p.5,
  which are identical); J-lead layout p.3. Two footprints, `-G` and `-J`. DigiKey numbers add
  the packaging suffixes `TUBE` and `TR`, which the key does not list (p.1 mentions tape & reel
  packaging), so the regex has an optional `(TR|TUBE)?`. Not widened for `TL3301NF160QG-KR`
  (`-KR` meaning unknown) or `TL3301SPF260QJT` (bare `T`). Both are skipped.
- **TL3303**: `TL3303.pdf` p.1. The drawing is small and was read at 400 dpi. Terminals 4, 2
  are at the top and 3, 1 at the bottom (leads exit top/bottom). The pads are 1.40 × 1.60
  at 4.50 × 8.40. The regex allows TL and LL (the drawing is shared). TLE (earth ground, a
  different drawing) is left out.
- **TL3312**: `TL3312.pdf` key p.1, drawing p.2. The PC keep-out area is not drawn.
- **TL3313**: `TL3313.pdf` p.1. Body is 4.60 per the drawing (DigiKey says 4.80 for two rows).
- **TL3340**: `TL3340.pdf` p.1. The key lists actuator A only, so `TL3340BF160QG` → skip.
  The part has **3 terminals**: the schematic shows 1 and 3 common and 2 as the other side. It
  is drawn `explicit` with 2 pins: pin 1 has two pads (terminals 1 and 3, the "same connection
  in more places" form), and pin 2 is terminal 2. Spec `terminals` 2, `A=1;B=2`. The two
  bracket pads and the two Ø0.90 peg holes are not drawn. Pad y positions come from the side
  view: the pegs are 1.20 behind the front face, the terminal pads end on the peg line, and
  the body depth is 3.40 − 0.85 actuator = 2.55. **Please double-check that this 3-pad/2-pin
  modelling is acceptable to the merge.**
- **TL3360**: `TL3360.pdf`, key p.1, which allows solder-pad option `A`, tab option `K`, and
  plating `Q`/`R` (so `TL3360AF185KR` is with solder pad, with tab, gold). The terminal pads are
  identical on p.2/p.3/p.4. The bracket solder pads (with-solder-pad option) are mechanical and
  not drawn. One footprint for all. Pad x: inner 2.54 / outer 4.95 → 1.205 wide at ±1.8725.
  Pad y: from 1.70 to 5.46 behind the front face, so 1.395 from the body centre (body depth
  4.37).
- **TL3701**: `TL3701AF100QG.pdf` p.1 (`add` row; one drawing for the part number).
- **TL52**: `TL52.pdf` p.1. 2 holes Ø1.00, 5.00 apart. The mounting drawing does not number
  them, so the numbers follow the schematic (1 left). The front view shows the leads symmetric
  about the body. → `SW-TACT-TL52`.
- **TS**: `06-TS35M943160TR.pdf` p.1 (`add` row). P.C.B LAYOUT 9.50/6.40, pads 1.55 × 1.60.
- **TSM9W**: `TSW_DS.pdf` (a Lamb Industries TSW-family page). It has a suggested PCB layout
  (11.0/6.8 × 5.4/2.6), but **no terminal numbers and no circuit diagram**, so the pinout
  cannot be read. See "Cannot draw".

## Cannot draw

- `TSM9W-5.2-260-TR`: `TSW_DS.pdf` has no terminal numbering or schematic, so the 4-terminal
  side pairing cannot be read from the datasheet. (Set to skip in my /tmp copy only, for
  specs-check.)

## Counts (rounds 1 + 2, rebuilt pre-cand.tsv)

- Rows: 9 verbatim `add` + 321 `pending` (no `check;` rows, so 0 `out`).
- scheme-check: 313 add, 8 skip (TL3301NF160QG-KR, TL3301SPF260QJT, TL3340BF160QG,
  TL3342F450QG, TL59AF260Q-KR, TL59DF260Q, TL6190AF220R, TL6700NF160QJ), 0 still pending.
- Kept: 321 (TSM9W-5.2-260-TR is in cannot-draw.tsv).
- Footprints: 67 (round 2's SW-TACT-TL6150-G was identical to SW-TACT-TL3304-G and was folded
  into it). Specs rows: 73.
- fp-load (no longer capped): `67 new footprints: load ok`.

# Round 2 (whole PDFs; series read by parallel helpers, merged here)

# E-Switch round 2 — fork G1 (320, LL3301, TL1014, TL1015, TL1016, TL1017, TL1030)

- **320**: `320.pdf`, 3 pages (the two local copies are byte-identical). Key p.1: cap style
  (320, .01, .02, .03, .04, .22, .08*, .09*, .094*, .2, .21), circuit E11, cap colour (BLK GRY
  YEL BLU GRN WHT RED). The "additional cap" field fits over styles .02 and .03 only, and its
  codes are the starred styles (.08/.09/.094, "assembled using the 320.02 plungered cap"). That
  gives the DigiKey forms 320.02E11.08BLK etc. `VERT` (320.04E11BLKVERT) is not in the series
  key. It comes from 320.04E11GRYVERT.pdf ("320.04 GRY CAP APPLIED VERTICAL"), so the third
  regex allows it on 320.04 only. **Doubtful but kept.**
  - Every cap-style drawing (p.1 320; p.2 .01/.02/.03/.04; p.3 .22/.2/.21) uses the same
    "320 E1-1 BODY (B01ME11)" and the same P.C. MOUNTING: PCB pins 1 and 3 (dia 1.22) and two
    dia 1.00 alignment pins on the 2.54 grid. Several drawings show it rotated 90°, and I checked
    the hole vectors are a pure rotation of the GRYVERT pattern. So **SW-TACT-320 is reused**,
    and the existing spec row `E-Switch 320 Through Hole .*` stays as it is (no replacement rows).
  - 320.21 (rocker cap 22.00 × 12.00) and 320.2 overhang the 12.45 body. The footprint body is
    the switch housing, so the cap overhang is not drawn.
- **LL3301**: `LL3301.pdf` is the long-life TL3301. Key p.1 gives force F065 only. Layouts p.2
  (gull wing) and p.3 (J-lead) are identical to TL3301 (11.20/7.00/4.50/1.40 and
  7.60/2.03/4.50/1.40), with the same schematic, so it reuses **SW-TACT-TL3301-G / -J**. The
  regex has the same optional `(TR|TUBE)?` as TL3301; none of the 3 rows uses it.
- **TL1014** (right-angle SMD): `TL1014.pdf`, key p.1 (boss A/B), drawings p.2. Pads 1.20 ×
  1.00 at 5.60 × 1.70; terminals 3, 1 on top and 4, 2 below. The schematic joins 1–3 and 2–4.
  The boss option adds two dia 0.75 holes, not drawn. The terminals are centred on the
  4.70 × 3.50 housing. → SW-TACT-TL1014 (A and B).
- **TL1015**: `TL1015.pdf` p.2. Two terminals, pads 0.95 × 1.40 at 4.15. The four optional G
  ground pads are not drawn. → SW-TACT-TL1015.
- **TL1016** (right-angle SMD): `TL1016.pdf`, key p.1 (bosses, ground terminal), p.2–p.5 give
  one page per option pair with identical terminal pads (1.40 × 1.05 at 3.60 × 1.45;
  terminals 1, 2 on top and 3, 4 below; 1–2 and 3–4 joined). Boss holes and ground terminal 5
  are not drawn. The PCB row pitch 1.45 is the layout's (the terminals are 1.50 apart).
  → SW-TACT-TL1016.
- **TL1017**: `TL1017.pdf` p.2–p.5, 2 terminals, pads 0.80 × 1.70 at 3.40. Ground pad and boss
  hole are not drawn. → SW-TACT-TL1017.
- **TL1030** (J-lead): `TL1030.pdf` p.2 (without ground) and p.3 (with ground, option B) have
  the same pads (1.15 × 1.05 at 3.35 × 1.45) but **number them differently**:
  - p.2 has 1, 3 on top and 2, 4 below (1–3 joined).
  - p.3 has 1, 2 on top and 3, 4 below (1–2 joined, plus ground 5).
  - So there are two ids: SW-TACT-TL1030 (…AQJ) and SW-TACT-TL1030-G (…BQJ). The ground pad
    5 is not drawn.

Counts: 55 rows, all regex add, 0 skip, 0 cannot draw. 6 new footprints (TL1014, TL1015,
TL1016, TL1017, TL1030, TL1030-G); reused SW-TACT-320, SW-TACT-TL3301-G and -J. 8 specs rows
(320 is covered by the existing row).

## Round 2, fork G2: TL1100, TL1105, TL1107, TL1150, TL1220, TL1240, TL1250, TL1265

- **TL1100**: `TL1100.pdf` p.1 holds the key and the TL 1100 drawing. A, C, D and E are on
  p.2; F and G are on p.3.
  - Vertical (None/A/D/E/F): all share 4 holes Ø1.30 at 12.50 × 5.00, terminals 3, 4 top and
    1, 2 bottom → `SW-TACT-TL1100-THT`.
  - Right angle C: 2 terminals Ø1.50, 5.00 apart. The bracket slots are not drawn. The
    footprint is drawn as printed, front to the left. Body 9.90 deep × 12.00, from the side
    view (3.05 + 2.75 + 4.10) → `SW-TACT-TL1100-THT-RA`.
  - G has no row, and its mounting differs (Ø1.3), so the RA spec row is limited to `TL1100C.*`.
  - Caps only on the `*` options (None, C). The cap code and colour come from a separate Cap
    Options catalogue, so the regex allows only the example `TAK` and the codes in the DigiKey
    numbers (`6J`, `.08`), each + a 3-letter colour. **Doubtful:** the cap list is not in this
    PDF.
- **TL1105**: `TL1105.pdf`, key p.1.
  - Vertical: TL 1105 and T/SP (p.2) share 4 holes Ø1.00 at 6.50 × 4.50, 3, 4 top, 1, 2 bottom
    → `SW-TACT-TL1105-THT`.
  - Right angle V/W/X/R/M/S/N/Q and J/JA (p.3): terminals Ø1.00 4.50 (J: 4.55) apart,
    2.55 (J: 2.50) behind the front face. The Ø1.30 bracket legs (2.50 further back) are not
    drawn. J's pads are within 0.025 and its body within 0.1, so both share
    `SW-TACT-TL1105-THT-RA`. Front edge at the bottom; terminal 1 on the left per the front
    view.
  - The p.3 side-view graphics are not to scale. The labels were used (front→terminal 2.55,
    terminal→bracket 2.50, terminal→back 4.00; J: 2.50 / 2.50 / 4.00), and they agree with
    the P.C. MOUNTING row spacing.
  - Caps: SP (vertical) and JA (RA) only, as `1R` + a 3-letter colour (example
    `TL-1105-SP-F160-Q-1R RED`). The colour list is not in the PDF, so `[A-Z]{3}`.
    **Doubtful.**
  - TLE (earth ground) is left out of the regex because it has a different drawing.
- **TL1107**: `TL1107.pdf`, key p.1. Every row is W (side retention lead), p.2: 2 holes Ø1.3,
  6.50 apart on the body centre line, 2 top and 1 bottom → `SW-TACT-TL1107-W`. The regex
  allows B/E/W per the key, but the spec row covers only `TL1107.*WQ` (B and E have their
  own drawings and no rows).
- **TL1150**: `TL1150.pdf` p.1, with the drawing rotated on the page. 2 holes Ø1.20, 4.90
  apart, symmetric about the 6.20 square body → `SW-TACT-TL1150` (dual, perSide 1).
- **TL1220**: `TL1220.pdf`, configurator p.1, S2 page p.8. The non-illuminated PCB layout is
  a 5.08 square grid of Ø1.00 holes (3, 1 top / 4, 2 bottom) plus a locating hole, which is
  not drawn. Housing 6.00 × 7.80. The grid is taken as centred, because the bottom view is
  symmetric. Spec row limited to `TL1220S2.*`; other button pages were not checked.
  **Doubtful:** the body/grid centring is assumed.
- **TL1240**: `TL1240.pdf` p.2. Holes are 6.50 × 4.50 Ø1.00, 3, 4 top, 1, 2 bottom, body 6.00
  square. This is identical to TL1105, so it **reuses `SW-TACT-TL1105-THT`**. The LED holes
  (+/−, 5.40 apart) are not drawn.
- **TL1250**: `TL1250.pdf` p.1, with mounting drawn front to the left. 1, 3 are the front
  column, 2, 4 sit 5.00 behind, rows 4.50 apart. The LED holes 5, 6 are not drawn. Body
  10.30 deep (4.40 + 5.90 about the front terminals) × 8.30 → `SW-TACT-TL1250` (explicit).
  Schematic 1–2 / 3–4.
- **TL1265**: `TL1265.pdf` p.2. Holes Ø1.0 at 6.0 × 4.5: 4, 3 top, 2, 1 bottom. The Ø0.8 LED
  holes are not drawn. Schematic 4–3 / 2–1. Body 7.0 × 6.8 → `SW-TACT-TL1265`.

Counts (G2): 84 rows, all regex add / add, 0 skip, 0 cannot draw; 9 new footprints (+1 reuse);
10 specs rows.

## Round 2, fork G3 (series TL, TL3145, TL3300, TL3302, TL3304, TL3305, TL3315, TL3330, TL3336, TL3342, TL3365)

- **TL** (DigiKey series literally "TL"): two rows. TL3305BF100QG is decided by TL3305.pdf's key
  and uses `SW-TACT-TL3305`. TL6170AF250QG is decided by TL6170.pdf's key (p.1:
  TL6170 [AB] F180/F250 Q G). Its drawing is on p.2: pad centres 9.00 MIN apart, inner
  edges 7.50 MAX, so pads are 1.50 × 1.00 at 9.00 × 4.00. 1, 2 are the top row (joined), 3, 4
  the bottom row (joined) → `SW-TACT-TL6170`. The spec rows for series TL are split by
  `TL3305.*` / `TL6170.*`.
- **TL3145**: TL3145.pdf key p.1 (actuator A/B, F180/F260/F350, Q, J only). Layout p.2: 7.00/3.00,
  5.40 tall, pads 2.00 × 1.40 at 5.00 × 4.00. 1, 2 top, 3, 4 bottom.
- **TL3300**: TL3300.pdf key p.1 (A/C/D/E; F130–F520). Layout p.2 is identical on p.3–p.5:
  15.50/12.40 × 6.60/3.40, pads 1.55 × 1.60 at 13.95 × 5.00. The PCB layout numbers 1, 2 on
  the top row and 3, 4 on the bottom row. The schematic draws 3, 4 at the top, but it shows
  1–2 joined and 3–4 joined, so `A=1,2;B=3,4`.
- **TL3302**: two PDFs with the same content: ebb28… (text) and e9ece… (scanned copy, for
  TL3302BF180QG/TR). The drawing page p.2 is laid out turned 90°, so it is read with its text
  upright. The part has 2 terminals with 1 on the left. Gull-wing: 13.20/6.50 × 2.16 →
  `SW-TACT-TL3302-G`. J-lead: 7.62/2.03 × 2.16 → `SW-TACT-TL3302-J`. The key has no packaging
  field, but p.2 shows a "TL 3302 standard reel", so `(/TR)?` is an optional group.
- **TL3304**: key p.1 (N/A, F160–F500, Q, G/J). Gull layout p.2 (12.00 MIN / 5.80 MAX) and J
  layout p.3 (9.60 MIN / 3.00 MAX) carry no pin numbers. The numbers come from the top view on
  the same page (1, 2 top, 3, 4 bottom). The MIN/MAX values are used as printed.
- **TL3305**: key p.1. Layout p.2: 8.80/5.60 × 4.40/1.60 → pads 1.60 × 1.40 at 7.20 × 3.00.
- **TL3315**: TL3315.pdf p.1 MOUNTING draws each land as a stepped "T". The outer tab is
  0.20 wide × 1.10 tall and the inner part 0.50 × 0.70. Each land is drawn as its bounding
  rectangle, 0.70 × 1.10 (x 1.875–2.575, y 1.20–2.30). This adds a little copper in the two
  notch corners. Doubtful but harmless; the exact shape would need two abutting pads per pin.
  The body is 3.70 (housing) × 4.50.
- **TL3330**: TL3330.pdf is image-only (no text layer). It has 2 terminals 4.20 apart (pads
  0.90 × 1.27; pin 1 left with the front at the bottom, matching the bottom view). The two
  1.45 × 0.80 bracket pads are not drawn. The pad y comes from the crosshair, taken as the
  body centre. The side/bottom views put it about 0.09 behind the body centre, within reading
  error.
- **TL3336**: PCB layout p.2 has the bracket face at the top, so terminal 2 is on the left and
  1 on the right. This agrees with the bottom view after turning it 180°. The terminal pads
  are 1.00 × 1.90 at ±2.25, 2.80–4.70 behind the bracket face. The body is 6.00 × 6.60
  (bracket face to back), which puts the pads at y +0.45. The bracket support pads and the
  rectangular hole (for the bracket leg) are not drawn. The bottom view puts the terminal
  feet 2.40–4.40 behind the front, about 0.35 forward of the PCB layout. The PCB layout is
  used.
- **TL3342**: two local copies with the same md5. The key is TL3342 F160/F260 Q G only, so
  TL3342F450QG → skip. TL3342F160QG/TR is accepted via an optional `(/TR)?`, as packaging. Note
  that this datasheet does not mention reels. P.C. MOUNTING: 8.00/4.60 × 4.80/2.80 → pads
  1.70 × 1.00 at 6.30 × 3.80.
- **TL3365**: key p.1 (A, F180/F250/F400, Q, G = J lead). Layout p.2: 5.50/3.50 × 2.90/1.40 →
  pads 1.00 × 0.75 at 4.50 × 2.15.

None share a land pattern with each other or with round-1 E-Switch footprints. 13 new footprints,
14 spec rows. Regex check over the 44 rows: 43 add, 1 skip (TL3342F450QG). Cannot draw: none.

## G4 series (E-Switch round 2)

- **TL3780**: `TL3780.pdf`, key p.1, drawing p.2. 2 terminals. PCB Layout outer 3.80 / inner 2.70,
  pads 0.55 × 1.50 on the body centre line → `SW-TACT-TL3780`.
- **TL3901**: `TL3901.pdf`, key p.1. The order is mounting A, termination G, contact Q, force
  F180, so the only number is `TL3901AGQF180`. Drawing p.2: pads 1.40 × 1.50, 3.40 between
  column centres, 4.30 gap between rows (so rows 5.80 apart). Terminals 3, 4 are at the top,
  1, 2 at the bottom, the same layout as the bottom view mirrored. The schematic joins 1–3
  and 2–4. The dia 0.90 boss holes are not drawn. The actuator faces +x, and the board edge
  for edge mounting is at the body's right face (x = +2.70) → `SW-TACT-TL3901`.
- **TL4100**: `TL4100.pdf`, key p.1 (termination G or L). The rows are G only. G layout p.2:
  terminal pads 1.45 × 1.10, 2.00–3.10 behind the board edge. The 0.60 × 0.85 bracket pads
  and the 5.10 × 1.65 edge notch are not drawn. The body (6.20 × 2.55) has its front face on
  the board edge; the board edge is the footprint's bottom side (y = +1.275). The spec's
  mpn_regex is `TL4100.*G`, so a future L row (different layout, p.3) needs its own row →
  `SW-TACT-TL4100-G`.
- **TL4105**: `TL4105.pdf` p.2. **3 terminals**: the schematic joins 1 and 3, with 2 on the
  other side. Modelled with 2 pins, pin 1 having pads at terminals 1 and 3 (as TL3340). Pads
  are 0.80 × 1.40 at 1.15 pitch, 1.60–3.00 behind the edge. The bracket slots and the notch
  are not drawn. Body 4.80 × 2.20, board edge at y = +1.10 → `SW-TACT-TL4105`.
- **TL4110**: `TL4110.pdf` p.2. The layout has two pad pairs. I drew the pair that is
  0.80 × 0.40 and 0.45–0.85 behind the mounting edge as terminals 1/2: in the bottom view the
  terminals also start 0.45 behind the front plate, at 1.90/2.80. The other pair (0.65 high,
  1.42–2.07 behind the edge) sits where the cover's rear tabs are, and is left out. If both
  pairs were tied to pins 1/2 and they are the cover tabs (one metal part), the switch would
  be shorted. **Doubtful point:** the PCB drawing's "1"/"2" labels sit between the two rows.
  Body 2.80 × 2.46, board edge at y = +0.77 → `SW-TACT-TL4110`.
- **TL4115**: `TL4115AF160Q.pdf` and `TL4115AF220Q.pdf` (one drawing per part, identical
  geometry; both rows are verbatim `add`). Terminal pads 0.70 × 0.60, 2.4 wide overall with a
  1.0 gap, 2.0–2.6 from the board edge. They match the 0.50-wide terminals 1, 2 at 2.20
  overall. The 0.75 × 1.85 bracket pads and the 3.4 × 1.75 notch are not drawn. The body
  (4.50 × 2.20) is placed from the edge into the board, with the edge at y = −1.10 (top).
  The body offset is my estimate; the pads are as printed → `SW-TACT-TL4115`.
- **TL58**: `TL59.pdf` p.1 key (V/W/X, F100/160/260, Q); drawing p.2 (the same drawing is in
  `TL59-TL58.pdf` p.2). Terminals are dia 1.00, 5.00 apart. The dia 1.30 bracket legs are not
  drawn. The terminals are 2.15 behind the front face: the legs are 1.00 wide at the back,
  and the terminals are 4.00 in front of them. The front is at the bottom, with 1 left and 2
  right per the front view. Body 7.40 × 6.65 → `SW-TACT-TL58`.
- **TL59**: `TL59.pdf` key p.1 (N A B C F E P). `TL59-TL58.pdf` p.1 adds SP, so there are
  two scheme rows. Skipped: `TL59DF260Q` (there is no D actuator in either key) and
  `TL59AF260Q-KR` (`-KR` is not in the key). Drawing p.2: 2 holes dia 1.00, 5.00 apart, leads
  symmetric about the body, which is 6.20 square. The terminals are not numbered; the switch
  is 2-terminal, so pin 1 is put on the left → `SW-TACT-TL59`. Not shared with
  `SW-TACT-TL52`, whose body is 8.00.
- **TL6059**: `TL6059.pdf`, key p.1, drawings p.2 (actuator A) and p.3 (actuator B), which
  are identical. 2 holes dia 1.00, 5.00 apart, body 6.00 square, terminals not numbered
  (2-terminal) → `SW-TACT-TL6059`. Not shared with TL59 (body 6.20 vs 6.00).

Cannot draw: none.

Counts (G4): 24 rows. 22 kept (2 verbatim add + 20 regex add), 2 regex skip. 9 footprints,
9 specs rows.

## G5 series (round 2)

- **TL6100**: `TL6100.pdf`, key p.1, drawing + PCB Layout + schematic p.2. Four Ø0.95 holes on
  7.62 × 5.08. Terminals 3, 4 are at the top and 1, 2 at the bottom; the schematic joins 3–4
  and 1–2. Pad 1.7 × 0.95 = 1.60. → `SW-TACT-TL6100`.
- **TL6105**: `TL6105.pdf` p.1/p.2. This is TL6100 with an earth-ground pin 5 (the Ø0.95
  hole 2.54 from 4/2 on the right). Pin 5 is not a switch terminal and is not drawn. It shares
  `SW-TACT-TL6100`. Note that the board then has no hole for the ground pin.
- **TL6110**: `TL6110.pdf` p.2. The schematic has terminals 1 and 2 only (Ø0.90, 5.08 apart).
  3 and 4 in the PCB layout (Ø1.10, 7.70 apart, one row 2.54 behind) are bracket legs, so they
  are not drawn. The front edge is at the bottom. The terminals are 1.55 behind the front
  face; that distance is **scaled from the side view** (it has no dimension), and the body
  depth is 5.80 (top view). → `SW-TACT-TL6110`.
- **TL6120**: `TL6120.pdf` p.2. Outer 12.00, inner 5.80, pads 1.00 high, rows 4.00 apart. The
  numbering is like TL3301 (3, 4 top). → `SW-TACT-TL6120`.
- **TL6150**: `TL6150.pdf` p.2 (gull wing) and p.3 (J lead). Gull wing: 5.80 MAX / 12.00 MIN,
  so pads 3.10 × 1.00 at 8.90 × 4.00. J lead: 3.00 MAX / 8.60 MIN, so pads 2.80 × 1.00 at
  5.80 × 4.00. The MAX/MIN values were taken as printed. Here 1, 2 are at the top and 3, 4 at
  the bottom; the schematic joins 1–2 and 3–4. The gull-wing geometry equals TL6120's, but the
  numbering differs, so it is its own footprint. Body 6.20 +0.30/−0.10 → 6.30. →
  `SW-TACT-TL6150-G` / `-J`.
- **TL6155**: `TL6155.pdf` p.2. **Doubtful.** p.1 says "right angle surface mount design", but
  the PCB layout has two plated holes for terminals 1, 2 (Ø1.10 with Ø1.60 land, 4.00 apart),
  and DigiKey says Through Hole, Right Angle. The spec row uses DigiKey's mounting.
  - Not drawn: the 0.80 × 1.50 bracket-tab pads (6.60 apart) and a 4.00 × 1.25 rectangle
    1.25–2.50 below the hole line. I could not tell what the rectangle is; it is not a
    terminal in the schematic.
  - The body on the board is 6.50 (6.40 +0.30/−0.10) × 2.50. The hole line is 1.08 above the
    body bottom (side view), which places the body against the holes; that placement is my
    reading of the views.
  - → `SW-TACT-TL6155`.
- **TL6190**: `TL6190.pdf` p.1. The key has plating, button (R) and button colour
  (GRY/BLK/WHT/RED) as required fields. `TL6190AF220R` lacks the colour (and is ambiguous on
  plating vs button), so it is a skip. No footprint.
- **TL6200**: `TL6200.pdf`, key p.1, SMT drawing p.3. The SMT PCB layout is a low-resolution
  raster. Pads are 1.20 × 2.50, centres 7.62 apart across and 12.50 apart top to bottom.
  Terminals 1, 3 are at the top and 2, 4 at the bottom; the schematic joins 1–2 and 3–4. The
  regex also allows `P` (thru hole, p.2), but no P rows exist. → `SW-TACT-TL6200`.

Counts: 33 regex add, 1 skip (TL6190AF220R), 7 footprints, 8 spec rows, 0 cannot draw.

## Round 2, fork G6 (TL6300, TL6330, TL6340, TL6400, TL6700, TL6800, TL9100, TL9210, TL9320)

All 18 rows are `pending`. None is a `check` row.

- **TL6300**: `TL6300.pdf`. The key is on p.1. Layout and schematic are on p.2 (actuator A)
  and p.3 (actuator B, identical). Terminal holes are dia 1.30 +.10/-.00, using the mid-value
  1.35 for the drill; the pad is 1.7 x 1.35 = 2.30. The holes sit on 5.00 x 12.50 with
  terminals 4, 2 at the top and 3, 1 at the bottom, the same numbering as TL3303. The two
  dia 1.70 peg holes (9.00 apart) are not drawn. The schematic joins 3-4 and 1-2. →
  `SW-TACT-TL6300`.
- **TL6330**: `TL6330.pdf`, key p.1, layout p.2. Pads are 0.90 x 1.00 at 4.10 x 1.60, with
  TL3301-style numbering (3, 4 top; 1, 2 bottom). → `SW-TACT-TL6330`.
- **TL6340**: `TL6340.pdf` p.2. This is a right-angle SMD part with 2 terminals.
  - The PCB layout has the front at the top. It is the top view turned 180 deg: the bottom
    view has terminal 2 on the left and 1 on the right with the front at the top, and the
    layout has 1 on the left.
  - Terminal pads are 0.70 x 1.10. Their x position is ±1.395 (pad inner edge 0.08 outside
    the peg centre, pegs 3.65 apart).
  - y is not printed relative to the body. I derived it from the peg: the side view puts the
    peg 1.55 behind the actuator tip, i.e. 0.60 behind the front face (actuator 0.95). The
    dia 0.80 ring around the peg touches the bracket-pad edge, and the terminal pads start
    0.40 below that edge, so the terminal-pad centre is 1.35 behind the peg.
  - With body depth 1.95, the pad centre lands at y = +0.975, on the rear body edge. That is
    plausible because the terminals exit at the rear, but it is **derived, not printed**.
  - The two bracket pads (with dia 0.60 peg holes) and the 1.40 x 1.10 centre bracket pad are
    not drawn. → `SW-TACT-TL6340`.
- **TL6400**: `TL6400.pdf` p.2. Pads are 2.00 x 1.50 at 5.40, 2 terminals. →
  `SW-TACT-TL6400`.
- **TL6700**: `TL6700.pdf` p.1. This PDF is an image only, with no text layer. The key gives
  actuator "(None) = Flush" or "A = 3.40mm" and termination G or J.
  - **`TL6700NF160QJ` is skipped.** The key prints no "N" letter; the flush actuator has no
    letter. This may be E-Switch's actual code for the flush option, but the key does not
    print it.
  - Gull-wing pads are 3.00 x 1.00 at 9.00 x 4.00. J-lead pads are 3.30 x 1.00 at
    6.30 x 4.00. Numbering is 3, 4 top and 1, 2 bottom. → `SW-TACT-TL6700-G` and
    `SW-TACT-TL6700-J`.
- **TL6800**: `TL6800.pdf` p.2. J-bend pads are 2.75 x 1.40 at 6.75 x 4.00. Numbering is
  1, 2 top and 3, 4 bottom; the schematic joins 1-2 and 3-4. → `SW-TACT-TL6800`.
- **TL9100**: `TL9100.pdf` p.2. Pads are 2.45 x 1.00 at 6.05 x 4.50 with TL3301-style
  numbering. → `SW-TACT-TL9100`.
- **TL9210**: `TL9210.pdf` p.2 (printed page number 3). Pads are 2.00 x 1.80 at 6.00,
  2 terminals. → `SW-TACT-TL9210`.
- **TL9320**: `TL9320.pdf` p.2. Pads are 1.70 x 2.70 at 8.10, 2 terminals. →
  `SW-TACT-TL9320`.

None of these land patterns matches an existing eswitch footprint, so all 10 ids are new.
Cannot draw: none.

Counts: 17 regex add, 1 skip (TL6700NF160QJ); 10 footprints; 10 specs rows.
