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

## Cannot draw

Per-part product specifications only, no drawing (see above):

- SKHC: SKHCBEA010, SKHCBFA010
- SKHH: SKHHAJA010, SKHHAKA010, SKHHAMA010, SKHHANA010, SKHHAPA010, SKHHAQA010, SKHHBSA010, SKHHBVA010, SKHHBYA010, SKHHCRA010, SKHHCWA010, SKHHDAA010, SKHHDHA010, SKHHDJA010, SKHHDTA010, SKHHDUA010, SKHHLMA010, SKHHLNA010, SKHHLPA010, SKHHLQA010, SKHHLRA010, SKHHLUA010, SKHHLVA010, SKHHLWA010, SKHHPNA010
- SKHL: SKHLADA010, SKHLLBA010, SKHLLFA010
- SKQE: SKQEABA010
- SKQJ: SKQJAAA010
- SKQM: SKQMAQE010, SKQMARE010, SKQMASE010, SKQMBAE010
- SKQY: SKQYAAE010
- SKRB: SKRBAAE010, SKRBACE010, SKRBAGE010, SKRBAKE010
- SKRK: SKRKAEE020, SKRKAGE020, SKRKAHE020
- SKRM: SKRMABE010
- SKRN: SKRNPAE010, SKRNPBE010
- SKRR: SKRRAAE010, SKRRABE010
- SKSH: SKSHAAE010
- SKSL: SKSLLAE010
- SKSN: SKSNLAE010, SKSNLHE010
- SKST: SKSTAAE010, SKSTACE010
- SKSU: SKSUAAE010 (was `add`: verbatim, but its spec has no drawing either)
- SKSV: SKSVCAE010
- SKTQ: SKTQAAE010

Three-terminal switch, not expressible as 2 or 4 terminals with equal sides:

- SKRT: SKRTLAE010, SKRTLBE010 — PdfFile_191666.pdf p.1: side push, terminals
  1, 2, 3 in a row; the Circuit Diagram joins 1 and 3 (one side) against 2.
  The land drawing is clear (3 terminal lands 0.75 / 0.6 / 0.75 wide, 1.8
  high, 3.2 outside; plus two 1.2-high fixing lands and 2 dia 0.9 boss holes),
  so it could be drawn as an `explicit` 2-pin footprint with pin 1 on both
  outer lands if the main session wants that; not done here because the
  brief asks for datasheet numbering 1…N with N = 2 or 4.

## Checks

- fp-load.sh: footprint_check prints at most 10 "extra" lines, so with all
  12 new ids in one run the merge script reports "new but not reported
  ['SW-TACT-SKTH', 'SW-TACT-SKTH-AE']" (FAILED) although nothing failed to
  load (520 footprints compared, 24 = 12 extra + 12 unused). Run in two
  halves of 6, each ends `6 new footprints: load ok`.

## Counts

- Rows: 105 (30 add, 75 pending).
- scheme-check: 74 add, 1 skip (SKSGPJE010), 0 still pending.
- Cannot draw: 60 (58 product-spec-only incl. SKSUAAE010, 2 SKRT).
- Kept (drawn): 44.
- Footprints: 12. Specs rows: 17. Checks rows: 0.
