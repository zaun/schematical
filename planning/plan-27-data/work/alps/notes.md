# Alps Alpine — plan-27 B/C reader notes

Rows: pre-census working copy `/tmp/p27/pre-cand.tsv` (maker `Alps Alpine`,
decision `add` 30 / `pending` 75 = 105 rows), PDFs from `/tmp/p27/pre-paths.tsv`.
No `check;` rows, so `checks.tsv` is header only.

## Two kinds of PDF

1. **Series catalog pages** (Alps TACT Switch catalog or the newer
   `product_catalog_ta_02_*` sheets): Product Line table, Dimensions, "PC board
   land dimensions (viewed from switch mounting face)", Circuit Diagram. Drawn
   from these.
2. **Per-part PRODUCT SPECIFICATIONS** (`product_spec_<mpn>_en_*.pdf`, 6-8
   pages): the part number is printed on every page header in full-width
   characters (`ＳＫＨＨＡＪＡ０１０`), which is why the census's verbatim search
   missed it; NFKC-normalised, every one of them names its own part. But these
   documents contain **no drawing at all**: section 2.2 says "Style and
   dimensions: Refer to the assembly drawings" and 4. says "Details of contact
   arrangement are given in the assembly drawings"; the only images are a
   logo banner and a dust-invasion sketch (checked with `pdfimages -list` and
   by viewing pages). So the series that only have these PDFs cannot be drawn
   (list below). `schemes.tsv` still records each of their numbers as a
   literal regex sourced to its own PDF (the number is in the PDF), so the
   scheme-check marks them `add`; the main session should then skip them as
   "Cannot draw". Alps's series catalog pages (e.g. the TACT Switch catalog
   SKHH / SKQM / SKRK pages) would make most of them drawable if fetched.

## Series drawn

| Series | PDF (key page / drawing page) | Footprint | Pins |
|---|---|---|---|
| SKHU | PdfFile_260198.pdf p.1 / p.2 | SW-TACT-SKHU | A=1,2;B=3,4 |
| SKPS | skps.pdf p.1 / p.1 | SW-TACT-SKPS (2 lands) | A=1;B=2 |
| SKQG | product_catalog_ta_02_skqg_en_d003713f01.pdf p.1 / p.1 | SW-TACT-SKQG | A=1,2;B=3,4 |
| SKRA | skra.pdf p.1 / p.1 | SW-TACT-SKRA | A=1,2;B=3,4 |
| SKRP | product_catalog_ta_02_skrp_en.pdf p.1 / p.2 | SW-TACT-SKRP | A=1,2;B=3,4 |
| SKRW | product_catalog_ta_02_skrw_en_e2e67a2472.pdf p.1 / p.1 | SW-TACT-SKRW | A=1,3;B=2,4 |
| SKSC | sksc.pdf p.1 / p.2 | SW-TACT-SKSC | A=1,2;B=3,4 |
| SKSG | sksg.pdf p.1 / p.2 | SW-TACT-SKSG | A=1,3;B=2,4 |
| SKSW | sksw.pdf p.1 / p.2 | SW-TACT-SKSW (2 lands) | A=1;B=2 |
| SKTA | skta1.pdf p.1 / p.1 | SW-TACT-SKTA | A=1,3;B=2,4 |
| SKTH | skth.pdf p.1 / p.1-2 | SW-TACT-SKTH (drawings 1, 2), SW-TACT-SKTH-AE (drawing 3, SKTHAEE010) | A=1,2;B=3,4 |
| SKHH (Through Hole) | skhh.pdf p.1-2 / p.3 | SW-TACT-SKHH-THT | A=1,2;B=3,4 |
| SKHH (Through Hole, Right Angle) | skhh.pdf p.2 / p.4 | SW-TACT-SKHH-THT-RA (explicit, 2 holes) | A=1;B=2 |
| SKRT | PdfFile_191666.pdf p.1 / p.1 | SW-TACT-SKRT (explicit, pin 1 on two lands) | A=1;B=2 |

Pad sizes and centres are derived from the outside / inside land dimensions
(each footprint's `note` gives the arithmetic) and cross-checked against the
terminal pitch on the Style drawing (SKQG: the 2.6 hatched gap between the
lands equals the gap the derived pads leave). Every footprint was compared
pad by pad with `footprint_check`'s printed line. No two series share a land
pattern.

Per series:

- **SKHU**: the four styles (top push, with ground terminal, dustproof,
  dustproof + ground terminal) have the same 4 switch lands; the ground
  terminal land (terminal 5) is left out. Scheme spells the Product Line's 9
  numbers; the 4 pending numbers (SKHUAKE/ALE/AME/BHE010) are in it.
- **SKPS**: 2 terminals. Other SKPS numbers (ACE, AAE) not ours.
- **SKQG**: all four numbers already `add`; the two 13-page
  `SKQG*_KQG_7xx_EN` specifications have no land drawing, the catalog sheet
  (Drawing No.1) is used for all four.
- **SKRP**: DigiKey lists SKRPAKE010 and SKRPAME010 as "Surface Mount, Right
  Angle"; the catalog lists them as top push, Drawing No.1, like the rest. Kept
  under DigiKey's mounting, same footprint.
- **SKRW**: scheme `SKRWA[DEMN]E030` = the Product List's four numbers.
- **SKSC**: every SKSC is side push (DigiKey calls three of them plain
  "Surface Mount"). Ground terminal land (SKSCP*, "no solder shall be
  applied") and the two dia 0.8 guide-boss holes left out. Body is the 3.5 x
  2.9 housing, which is centred on the terminals; the 3.55 catalog depth adds
  the push actuator on the bottom edge (would push the origin 0.325 off the
  terminal centre).
- **SKSG**: SKSGPJE010 is not in the Product Line (ACE, AFE, AAE, PCE, PGE,
  PAE) → skip. Ground terminal lands 5, 6 left out. DigiKey lists SKSGPCE010
  as right angle; the catalog lists it as top push with ground terminal.
- **SKSW**: drawings 1 and 2 share the land; 2 terminals.
- **SKTH**: drawing 3 (SKTHAEE010) has bigger lands (4.5 / 2.2 / 2.5) than
  drawings 1 and 2 (5 / 2.9 / 2.2), hence two footprints. DigiKey lists
  SKTHAHE010 / SKTHBAE010 as right angle; the catalog lists them as top push.

- **SKHH** (top-up, skhh.pdf became whole): the series catalog lists every
  SKHH number we have (the 25 spec-only ones too), so all 35 SKHH rows are
  drawn from it. Drawings 1 (top push), 2 (joint stem) and 3 (with ground
  terminal) have the same 4 holes, dia 1 at 6.5 x 4.5 (no land printed: pad
  1.7 = 1.7 x drill); drawing 3's ground terminal hole is left out. DigiKey's
  "Through Hole, Right Angle" rows are exactly the side push types (SKHHL*,
  SKHHN*, drawing 4): 2 terminals in dia 1 holes 4.5 apart, 2 dia 1.3 frame
  holes left out; the terminals sit 2.55 behind the body front (side view), so
  the explicit pads are at y = -0.55 from the 6.2 x 4 body centre.
- **SKRT** (top-up, as C&K PTS845 / E-Switch TL3340): 3 terminals, the
  Circuit Diagram joins 1 and 3 against 2. Drawn `explicit`, `pins` 2: pin 1
  has two pads (both outer terminal lands, datasheet terminals 1 and 3), pin 2
  the centre land; spec `terminals` 2, `A=1;B=2`. Fixing lands, no-copper area
  and guide-boss holes left out. Body is the 4.5 x 2.56 housing (the 0.84 stem
  at the bottom edge); the guide-boss line is 1.21 behind the housing front,
  the terminal lands run 1.8 from it towards the rear.

## Cannot draw

`cannot-draw.tsv` (36 rows) is the list. Per-part product specifications
only, no drawing (see above):

- SKHC: SKHCBEA010, SKHCBFA010
- SKHL: SKHLADA010, SKHLLBA010, SKHLLFA010
- SKQE: SKQEABA010
- SKQJ: SKQJAAA010
- SKQM: SKQMAQE010, SKQMARE010, SKQMASE010, SKQMBAE010
- SKQY: SKQYAAE010
- SKRB: SKRBAAE010, SKRBACE010, SKRBAGE010, SKRBAKE010
- SKRK: SKRKAEE020, SKRKAGE020, SKRKAHE020
- SKRM: SKRMABE010
- SKRN: SKRNPAE010, SKRNPBE010
- SKRR: SKRRAAE010, SKRRABE010, SKRRACE010
- SKSH: SKSHAAE010
- SKSL: SKSLLAE010
- SKSN: SKSNLAE010, SKSNLHE010
- SKST: SKSTAAE010, SKSTACE010
- SKSU: SKSUAAE010 (was `add`: verbatim, but its spec has no drawing either)
- SKSV: SKSVCAE010, SKSVCCE010
- SKTD: SKTDLDE010
- SKTQ: SKTQAAE010

(SKRRACE010, SKSVCCE010, SKTDLDE010 came with the top-up; their
product_spec PDFs also say "Refer to the assembly drawings" and carry no
drawing.)

## Counts

First pass (105 rows: 30 add, 75 pending): scheme-check 74 add, 1 skip.

After the top-up (rebuilt pre-cand.tsv, full-width numbers now verbatim):

- Rows: 120 (119 add, 1 pending).
- scheme-check: 0 add, 1 skip (SKSGPJE010), 0 still pending.
- Cannot draw: 36 (cannot-draw.tsv; all product-spec-only).
- Kept (drawn): 83.
- Footprints: 15 (fp-load: `15 new footprints: load ok`). Specs rows: 20.
  Checks rows: 0.
