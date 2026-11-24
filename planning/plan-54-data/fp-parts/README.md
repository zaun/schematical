# plan-54-H / I / J / K: drawing the footprints — the reader's brief

You draw the land patterns of a share of the card-slot footprint *lines* (one line = one footprint family: a datasheet
drawing for one series / mounting, with a list of position counts). Everything is relative to the worktree
`/Users/justinzaun/Development/logic/.claude/worktrees/P-54` (use absolute paths in tool calls; never touch another
checkout). The library's footprint format is in `docs/parts-format.md` § Footprints (read § `explicit` and § Holes
on explicit pads, and § `connector`'s USB paragraph for the connector-shell conventions); the USB connectors'
footprints in `schematical/schematical/Parts/footprints.json` (ids starting `USB-`) are the precedent for a
connector (`grep -o '"id":"USB-[^"]*"' …`, look at two).

## What you are given

`python3 -I planning/plan-54-data/census/fp-brief.py <letter> [--family F] [--maker M] --slice i/n` (your command is in
your task text) prints each line: `== <stem>`, maker, series, mounting, card, pitch, rows, the position counts, the
status (`edge-mount`: a straddle-mount connector), the datasheet path, and for each position count `N` the **pinout**
the parts of that count use: `N=<positions>: pins <P>  1=NAME,2=NAME,…`. The pins are the footprint's pad numbers
`1 … P`, in this order. The datasheet is the PDF named on the `pdf` line (read it with `pdftotext -layout`, the Read
tool's PDF `pages` option for drawings, or `pdftoppm -r 200 -png` + `tesseract`).

## The footprint you write (explicit style)

```json
{"id": "SOCK-GCT-MEM1052-11", "description": "…", "note": "drawing, PDF, page; every number not printed with its arithmetic",
 "style": "explicit", "pins": 12, "body": {"width": 14, "height": 15},
 "pads": [{"pin": 1, "x": -3.2, "y": 4.5, "width": 0.7, "height": 1.6, "shape": "rect"}, …]}
```

Rules (the same as the USB connectors'): millimetres, origin the **centre of the housing's top-view outline**, x right, y down.
A card socket's **card entry (mating opening) faces up (−y)**; a module or card-edge socket's contact rows run along x with
pin 1 at the left of side 0 unless the drawing says otherwise (the generator's `dir`). **Pad coordinates come from the
datasheet's recommended PCB land pattern / footprint drawing** (mirror it if the drawing is a bottom view: the
footprint is drawn as seen from the top of the board, looking down). Contact pads are pins `1 … P` in the pinout's order;
the shell / shield tabs are pads of the **last pin** (`SHIELD`), one pad per tab, all with that pin number; locating
pegs, detent holes and any pad the pinout does not name are `"pin": 0` pads (an unnumbered pad); a non-plated hole is
`{"pin":0,…,"drill":d,"plated":false}` with the pad equal to the hole (round, `width` = `height` = drill, or a slot with
`slotLength`). `pins` is the number of pin-numbered pads (= the pinout's pin count). `shape` is `rect` or `round` (a
stadium for a long pad). A through-hole pad has `drill` (smaller than both pad sides). Rotated (non-axis-aligned)
slots and keyhole slots cannot be expressed: if the drawing needs one, **do not invent** — report the line as
`footprint not expressible: <feature>`. **Pads on both faces** (a straddle-mount edge connector, status `edge-mount`):
the pads of the face the part's own side is on are ordinary; the pads on the far face carry `"opposite": true`
(surface mount only) and the footprint `"edgeMount": true`; the footprint's `note` says where the board edge is relative to
the pads. Pad sizes: the print's; use `round` only when the print shows round pads.

`note` must name the drawing (document number), the PDF and the page, and give the arithmetic of every number not printed.
Never guess a dimension: if a print omits it, say so in your report and in the note, and use the nearest printed
value (e.g. the pad pitch times the count).

## Two kinds of lines

1. **Card sockets (letter H, family `card`)**: 7–23 pins. Write each footprint out in full (one per position count N of the
   line, id `<stem>-<N>`; the stem is the line's last-but-one column printed after `==`).
2. **Module sockets and card-edge sockets (families `module`, `edge`)**: 72–694 contacts in two regular rows. Do **not** write
   pads out: write a **generator parameter row** for the variant, see `planning/plan-54-data/census/rowgen.py` (its
   docstring is the format; `census/rowgen-test-params.json` is an example): one row per line with every `N` of the line in
   `positions`; the pad pitch, the side numbering (`halves` or `odd-even`), the rows' y, the pad size, the key notch as
   `gaps`, the pegs / shield tabs as `extras` (with expressions of `L0`, `N`, … when their x depends on the length), the body.
   Then write the **reference dimensions the drawing prints** in your `rowgen-ref.tsv` (`id`, `what`, `value`; `what`:
   `span0`, `span1`, `rows`, `pad<pin>.x`, `peg<k>.x`, `pins`): for each variant **at least two position counts** when the
   print gives more than one (the first-to-last pad distance, the distance from pin 1 to the key, each peg's x), a variant with one count
   gets the drawing's pad-to-pad and peg dimensions. The check `rowgen-check.py` fails when the generator disagrees with the
   print by more than 0.02 mm. Where the print gives a pad-by-pad table or a notch, get the notch in `gaps` right (the generator's
   pad `i` of a side is at `-span/2 + i*pitch + the gaps before it`).

If a line's footprint is the **same land pattern** as an existing library footprint or another line's (same pad centres,
sizes and count within 0.02 mm), map it instead of redrawing: a row in your map file
`footprint-map-<yourname>.tsv` (`stem`, `positions`, `footprint`, `note`), `footprint` the existing id (`python3 -I
planning/plan-54-data/census/fp-compare.py` is not available: compare by hand with `grep` in
`schematical/schematical/Parts/footprints.json` and say what you compared).

## Your files

Write only these (names in your task text): `planning/plan-54-data/fp-parts/<name>.json` (the footprints, format of
`footprints.json`: `{"format":"schematical-footprints","version":1,"footprints":[…]}`; for rowgen lines, the rows go in
`<name>-params.json` and `rowgen.py` writes the footprints into `<name>.json`), `<name>-ref.tsv` (rowgen lines) and
`<name>.map.tsv` (mapped lines), plus scratch files in
`/private/tmp/claude-501/-Users-justinzaun-Development-logic/8343ba34-242d-4698-8fa3-e4a4af725824/scratchpad/`. Do not edit
`specs.tsv`, `candidates.tsv`, any census script, `Parts/*.json`, or anything else.

## Checks (your feedback loop)

```
python3 -I planning/plan-54-data/census/fp-check.py <letter> --scoped --json planning/plan-54-data/fp-parts/<name>.json --map planning/plan-54-data/fp-parts/<name>.map.tsv
python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/fp-parts/<name>-params.json planning/plan-54-data/fp-parts/<name>-ref.tsv
planning/plan-54-data/census/fp-load.sh planning/plan-54-data/fp-parts/<name>.json
```

`fp-check` (with `--scoped`: only the keys your files cover) must end `<n> footprint keys, 0 without a footprint` and print no
`unused footprint`; `rowgen-check` `0 differ`; `fp-load.sh` (it builds the backend in a copy, ~3 min the first time) must end
`<k> new footprints: load ok`. Fix everything they print. Render at least one footprint of each kind to look at it: draw
the pads with a small Python script (matplotlib is not installed; write an SVG and convert with
`qlmanage -t -s 800 -o <dir> <file>.svg` or just check the numbers) — at minimum check by arithmetic that the pad rows,
pegs and body look like the drawing.

## Constraints

- No git commands (do not write that word in a command), no network, no DigiKey. Do not delete or move PDFs.
- The shell refuses heredocs, loops and some compound commands: put scripts in files (Write tool, under the scratchpad above)
  and run with `python3 -I <file>`; keep Bash commands plain and separate.

## Report (your final message)

1. The final output of the three checks.
2. Every line where a dimension is read approximately, assumed, or the drawing is unclear: `stem | what`.
3. Every line not drawn and why (`footprint not expressible: …`, or no drawing in the PDF) — those parts become `skip`.
4. Lines you mapped to an existing footprint (which id, what you compared).
