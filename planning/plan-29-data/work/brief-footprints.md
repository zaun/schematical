# plan-29 footprint brief (B–G owners)

Repo: `/Users/justinzaun/Development/logic/.claude/worktrees/P-29` (a git
worktree; work only there, use absolute paths, never `cd` to the main
checkout, never run git commands that change state: no commit, add, stash,
checkout, reset). DigiKey calls are not needed and not allowed.

You draw the land patterns of the diode packages your letter **owns** in
`planning/plan-29-data/packages.tsv` (column `owner`), one footprint per
distinct land pattern, from the maker's package drawing in the PDFs under
`datasheets/diode/<maker>/` (column `top_pdfs` names the three PDFs with
the most parts; any PDF of that maker and package may be used, prefer one
with a recommended pad layout). Read first:

- `docs/parts-format.md` § Footprints (all of it: styles `dual`, `quad`,
  `explicit`, pads, body, `drill`, rounded corners, the `"footprint"` key)
  and the existing objects in `schematical/schematical/Parts/footprints.json`
  (format examples: `RES-0402`, `SOT23-5`, `SOT23-6`, `LED-RADIAL-5MM`,
  explicit `SON-6-DRY`).
- `planning/completed/plan-03-B-logic-footprints.md` § 3 "Pad rules" (R1–R6).
- `planning/plan-29-B-diode-footprints-vishay-single.md` § 4 (R7 axial
  rule, pin numbering, package names, ids, reuse) — the rules for this work.

How to read a drawing: `pdftotext -layout <pdf> -` gives the text (often
"PACKAGE DIMENSIONS", "MOUNTING PAD LAYOUT", "SOLDERING FOOTPRINT",
"RECOMMENDED LAND PATTERN", "SUGGESTED PAD LAYOUT"; onsemi ends with a
"MECHANICAL CASE OUTLINE"); when the numbers are in the drawing image, read
the page as an image (the Read tool with `pages`) or render it
(`pdftoppm -r 150 -f N -l N -png <pdf> /tmp/<you>/p`) and read the PNG.
Inches vs millimetres: use the mm values. Use max values where the rules
say max, nominal (or the mid of min/max) for body sizes.

Pin numbering (B § 4.2): two-terminal packages: pin 1 = cathode (band /
marked end), pin 2 = anode. Numbered-lead packages (SOT-23, TO-220,
D2PAK, DPAK, TO-277, DFN, FlatPAK, bridges…): the datasheet's own lead
numbers; a tab or exposed pad that is a terminal is the LAST pin number.
Bridges: name terminals `AC1`, `AC2`, `+`, `-`. Arrays: `A1`, `A2`,
`K1`, `K2`, `K` (common), `A` (common), … A lead marked NC is `NC`.

Footprint ids (provisional; the merge renames them): `DIODE-<PKG>-<TAG>`
for two-terminal packages, `<PKG>-<pins>-<TAG>` for numbered-lead ones
(e.g. `TO-220-2-TAB-VISHAY`, `SOT23-3-ONSEMI`), where `<PKG>` is the
package's common name in capitals with spaces → `-`, and `<TAG>` is
`VISHAY`, `ONSEMI` or `TSC`. If the drawing gives exactly the land pattern
of an existing `Parts/footprints.json` object (every pad centre and size
within 0.05 mm, same drill), use that id instead (name it in the note). Two
package lines of the same maker whose drawings give the same land pattern
share one footprint.

Every footprint object: `id`, `description` (e.g. "SMA (DO-214AC) diode,
Vishay drawing, 5.2 x 2.6 mm body"), `note` (drawing name, PDF path under
`datasheets/diode/`, page, rule R1/R2/R4/R5/R7, the key numbers), style,
pins, body, pads. Pads must not overlap. Diode pads are often unequal
(e.g. SOD-123FL, TO-277A, D2PAK tab): use `explicit` then.

Outputs (write only these two files):

1. `planning/plan-29-data/footprints-<X>.json` — `{"footprints":[…]}`.
2. `planning/plan-29-data/packages-<X>.tsv` — header
   `maker	supplier_package	mounting	package	footprint	pins	pdf	page	note`,
   one line per owned package line of packages.tsv (all of them; maker as
   in packages.tsv: Vishay / onsemi / TSC; mounting as in packages.tsv,
   for a blank mounting write the one the drawing shows). `package` is the
   part's package name (B § 4.3, e.g. `SMA`, `DO-41`, `D2PAK`, `SMPC`).
   `pins` is the lead map for the package's configurations in the form
   `config=1:K,2:A;config2=…` with config one of `single`, `cc`
   (common cathode), `ca` (common anode), `series`, `ind` (independent),
   `bridge`, or the DigiKey configuration text if none fits
   (packages.tsv column `configurations` lists them for the line). Numbers
   must equal the footprint's pad designators 1..N. When a package line
   cannot be drawn (no drawing in any of the maker's PDFs of that package),
   write `footprint` = `NONE` and say why in `note`.

Check before you finish:
`planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-<X>.json`
must end with `N new footprints: load ok` (it builds the backend in /tmp;
about 2 minutes). Fix any reported problem.

Report back: the number of footprints, the lines marked NONE and why, any
package line where the maker's drawings disagree, and anything in the
rules that did not fit.
