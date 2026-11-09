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

## Checker note

`fp-load.sh` on all 14 footprints at once reports `FAILED` with "new but not reported
['SW-TACT-TL3360', 'SW-TACT-TL3701', 'SW-TACT-TL52', 'SW-TACT-TS']". The cause is
`backend/tests/footprint_check.c` line 129, which prints only the first 10 failures, so the
11th–14th "extra" lines are never printed. That run had 28 mismatches (14 extra + 14 unused)
and no other failure. Split into two files of 7 (`/tmp/p27-es/fp-a.json`, `fp-b.json`), each
half ends `7 new footprints: load ok`. The merge will hit the same cap when it loads more
than 10 new footprints at once.

## Counts

- Kept rows: 67 (7 verbatim `add` + 60 regex adds; TSM9W excluded as "Cannot draw").
- scheme-check: 60 add, 3 skip (TL3301NF160QG-KR, TL3301SPF260QJT, TL3340BF160QG), 0 still pending.
- Out: 0 (no check rows).
- Footprints: 14. Specs rows: 14.
