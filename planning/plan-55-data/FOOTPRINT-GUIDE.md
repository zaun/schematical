# Footprint reader guide — land patterns of the power-switch packages (plan-55-F and plan-55-G)

You map every package of your packet to a footprint of `schematical/schematical/Parts/footprints.json`: a **library footprint that
already has the datasheet drawing's pads (reuse)** or a **new footprint object you draw from the datasheet's package drawing**.
Format and rules: `docs/parts-format.md` § Footprints (read § Common keys, `dual`, `quad`, `grid`, `explicit`, "The `footprint`
part key"), pad rules R1–R6 below. Work in `/Users/justinzaun/Development/logic/.claude/worktrees/P-55`. **Touch no file but your
own packet's `footprints.json` and `map.tsv`; run no `git` command.**

## Your packet

`/tmp/p55/fpackets/fNN/assign.tsv` (tab separated, header first); one line per **unit**: `letter` (F = Texas Instruments, G = every
other company), `company`, `package` and `supplier_package` (DigiKey's two labels), `pins` (the number of pads the footprint must have: the
pinout's pin count; **an exposed pad or tab the datasheet numbers as a pin counts**, an unnumbered one does not), `add_rows` (how many
parts), `pdfs` (the placed datasheets of those parts, most parts first), `texts` (their `pdftotext -layout` text, same order) and `example`
(the most stocked part).

For each unit write:

* `/tmp/p55/fpackets/fNN/footprints.json`: `{"format": "schematical-footprints", "version": 1, "footprints": [ ...the NEW objects... ]}`
  (only footprints you draw; none for a reuse).
* `/tmp/p55/fpackets/fNN/map.tsv`: header `letter company package supplier_package pins footprint source drawing pdf note` (tab
  separated, written with Python's `csv.writer(delimiter='\t', lineterminator='\n')`): **one row per unit**: copy `letter`, `company`,
  `package`, `supplier_package`, `pins` from the assignment; `footprint` = the id (existing or yours); `source` = `reuse` or `new`;
  `drawing` = the datasheet's package code and the page you took it from (`TI DBV0005A board layout, tps2042b.pdf p.28`); `pdf` = `*`
  (the footprint serves every PDF of the unit) or one PDF's file name (a row for one PDF wins over a `*` row: use it when the PDFs'
  drawings differ for the same package labels); `note` = anything a reviewer should check.

## What the footprint must satisfy

* **The pads are the pinout's pins.** The part's pinout (see below) names its pins `1`…`N` (numbers as the package drawing) or ball
  names `A1`, `B2` (DSBGA / WLCSP / BGA); the footprint's `pins` equals N and every pin designator has a pad of that designator. `dual` and
  `quad` pads are numbered `1`…`pins` by slot (pin 1 top of the left side, counter-clockwise: down the left column, then up the right);
  where the package skips a slot (SOT-23-5: no pin on slot 5) use `slots`; a numbered exposed pad that is a pin is the `centre` pad and
  its pin has slot `0` in `slots`; an unnumbered exposed pad is a `centre` no pin names. `grid` pads are named by ball.
* **See the pinouts you serve:** `python3 -I planning/plan-55-data/census/show.py <pdf file name> "<package label>"` prints the spec
  rows (`pins` = the pinout `[number, name, role]` list) of that PDF and package; check its numbers against your footprint.
* **Reuse beats drawing.** `python3 -I planning/plan-55-data/census/fp-find.py --pins N --pitch P [--span S] [--id REGEX]` lists library
  footprints (and the new ones other packets already wrote to `planning/plan-55-data/footprints-[FG]*.json`). A footprint is reused
  when its pad count, pitch, pad size and span equal the drawing's within the datasheet's tolerance (pad size within 0.05 mm, span within
  0.06 mm) and its `centre` (exposed pad) matches. `grep -n '"id": "SOT23-5"' schematical/schematical/Parts/footprints.json` shows one.
* **Pad rules** (plan-03-B § 3): **R1** a board layout / "recommended land pattern" / "suggested pad layout" in the datasheet: use its
  pad size, pitch and pad-centre span (when it gives the outer extent X and the pad length L, span = X − L). **R2** gull-wing or flat
  leads, outline only: pad length = L(max) + 0.8, pad width = b(max) + 0.05, span = E(max, tip to tip) + 1.0 − pad length. **R3** J leads.
  **R4** leadless terminals (SON, QFN, DFN, LCCC), outline only: pad length = terminal length + 0.3, pad width = terminal width + 0.05, span
  = terminal-centre span + 0.3; the exposed pad is `centre` (rect, nominal size). **R5** through-hole (DIP): drill = largest lead cross-section
  + 0.25 rounded up to 0.05, round pad of drill + 0.7 (at least 1.5), `pin1Shape` `rect`, span = the row spacing. **R6** BGA / DSBGA /
  WLCSP: `grid`, round pad, diameter = 0.8 × the nominal ball diameter, rounded to 0.05 (when the datasheet gives a land pattern (R1) use its
  pad). **Mid-values:** every dimension taken from a drawing is (min + max) / 2 when both are given; a single or BSC value is used
  as printed. **Body** = the nominal body size without leads, rounded to 0.01. Pads must not overlap each other.
* **Ids:** `<FAMILY>-<pins>-<MAKER>-<DRAWING CODE>` in upper case like the library's (`SOT23-5`, `SOIC-8`, `DSBGA-6-YZP`,
  `DFN-8-ADI-DD8`, `TDFN-6-MCHP-...`): family from the package (SOT23, TSOT23, SC70, SOIC, MSOP, TSSOP, HTSSOP, SSOP, SON, WSON, VSON,
  DFN, UDFN, WQFN, VQFN, UQFN, TQFN, DSBGA, WLCSP, X2SON, ...), the pin count, then the maker (`TI`, `ONSEMI`, `DIODES`, `MCHP`, `ADI`,
  `MPS`) and the drawing code (the datasheet's package code) when the pads are that maker's own (a generic JEDEC package that matches an
  existing id takes it). `A-Z`, `0-9`, `.`, `-` only; **unique** among all packets (the maker and code make it so).
* `description`: one line (`6-pin SOT-23 (DBV), 1.6 x 2.9 mm body, 0.95 mm pitch`); `note`: drawing code, file, page and the mid-values
  and arithmetic (`lead 0.46 mid (0.38-0.53)`), like the library's.

## Where the drawing is

The package drawing and its land pattern are in the last pages of the datasheet text (`grep -n -i "EXAMPLE BOARD LAYOUT\|LAND PATTERN\|RECOMMENDED
LAND\|SUGGESTED PAD\|SOLDERING FOOTPRINT\|PACKAGE OUTLINE\|MECHANICAL DATA" <text>`): TI "EXAMPLE BOARD LAYOUT" (R1; take the stencil-free copper
pad dimensions, not the solder-mask or stencil ones), Microchip "Recommended Land Pattern" (C-codes), Diodes "Suggested Pad Layout" (X, Y, C, G
values), onsemi "SOLDERING FOOTPRINT", Analog Devices / Linear / Maxim "RECOMMENDED SOLDER PAD LAYOUT" / land pattern. The text often has the
numbers; when a drawing is an image, render the page and read the image: `pdftoppm -r 110 -f P -l P -png <pdf> /tmp/p55/fpackets/fNN/pg` then
`Read` the PNG. Find the page with `pdftotext -f P -l P -layout`. Say in `drawing`/`note` which page.

## Check your work

From the worktree: `planning/plan-55-data/census/fp-load.sh /tmp/p55/fpackets/fNN/footprints.json` (a minute or two: it builds the loader in a
temporary copy) must print `<k> new footprints: load ok` with `k` your number of objects (nothing for an empty file: skip then). And for the
map: `python3 -I planning/plan-55-data/census/fp-check.py <F or G> --map /tmp/p55/fpackets/fNN/map.tsv --json /tmp/p55/fpackets/fNN/footprints.json`
prints a line per package line still without a footprint (other packets' lines will show; **none of yours may**) and no `unknown id` and
no `has N pins` line for yours.

Report (short): packet, units, how many reuse / new, the ids, anything uncertain (a drawing you could not find, a pad you doubt). Do not paste the files.
