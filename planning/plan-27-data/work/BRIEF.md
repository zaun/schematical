# plan-27 B/C brief — reading tactile-switch datasheets for one maker

You are one of several parallel readers. Each reader owns one or more makers
of `planning/plan-27-data/candidates.tsv` and writes ONLY into its own work
folder `planning/plan-27-data/work/<slug>/` (slug: ck, eswitch, omron,
panasonic, alps, samesky, te, apem, nkk, wurth). Do not edit
`candidates.tsv`, `specs.tsv`, `schemes.tsv`, any `footprints-*.json`,
anything under `schematical/` or `backend/`, and do not run git. The main
session merges your files.

Repo root (a git worktree): /Users/justinzaun/Development/logic/.claude/worktrees/P-27
Read first: `planning/plan-27-B-tactile-footprints-1.md` § 3–4 (the design you
apply), `docs/parts-format.md` § Footprints (styles `dual`, `explicit`, pad
keys, mid-values, no overlapping pads), and look at three or four existing
footprints in `schematical/schematical/Parts/footprints.json` (e.g. ids
`SW-DIP-4`, `SW-SMD-4`, `SOIC-14`, `SON-6-DRY`) for the JSON shape.

## Your rows

`candidates.tsv` is tab-separated; columns: maker mpn status qty series circuit
function mounting actuator orientation termination outline rating
illumination ingress height datasheet_url decision reason pdf hit dead.
Your rows are those whose `maker` starts with your maker key and whose
`decision` is `add` or `pending`. The local PDF of a row: look up its
`datasheet_url` in `datasheets/switch/tactile/datasheets.tsv` (path, url,
bytes, sha256), or run
`python3 -I planning/plan-27-data/census/place.py --path planning/plan-27-data/candidates.tsv | grep <mpn>`.

- `add`: the part number is printed verbatim in its PDF. Keep it.
- `pending`: it has a PDF but the number is not printed verbatim (the PDF gives
  an ordering-code key). You decide it with a regex (below).
- A row whose `reason` starts with `check;` had blank DigiKey parameters
  (circuit, mounting, actuator, rating all `-`). Read them from the datasheet.

## Reading PDFs

`pdftotext -layout <pdf> -` gives the text (ordering tables, specs, part-number
keys). Drawings are scrambled in text: view the page as an image with the Read
tool (`pages: "3"`) to read the PCB land pattern / "PC board layout" /
"Recommended footprint" and the circuit diagram (terminal numbering). Use
`pdfinfo` for page counts. Read each distinct PDF once for everything.

## What to produce (all tab-separated with a header line, UTF-8)

### 1. `work/<slug>/schemes.tsv` — header `maker series regex source`

For every (maker, series) that has `pending` rows: a Python `re` pattern that
`re.fullmatch`es exactly the part numbers the datasheet's part-number key /
"How to order" table allows for that series — spell the key's fields out
(e.g. `PTS636 ?S[KM](15|25|43|50)(J|F|G)?(SMTR|TR)? LF[SG]`), never a catch-all
like `TL1105.*`. Include the DigiKey suffixes that are packaging/RoHS only if
the datasheet's key shows them; if DigiKey numbers carry a packaging suffix
the key does not list, prefer listing it as an optional group and say so in
`source`. `maker` is the full DigiKey maker name as in candidates.tsv;
`source` is `<pdf file name> p.<page>`. Several rows per (maker, series) are
allowed (alternatives, e.g. two datasheets). A pending row that no regex
matches will become `skip` ("not in the datasheet's ordering table") — that
is the correct outcome for a number the key does not produce; do not widen
a regex to swallow it. Test with:

    python3 -I planning/plan-27-data/census/scheme-check.py --maker '<maker key>' \
      --tsv /tmp/p27-<slug>-cand.tsv --schemes planning/plan-27-data/work/<slug>/schemes.tsv --dry-run

(copy `candidates.tsv` to `/tmp/p27-<slug>-cand.tsv` first; it prints each
add/skip and the counts).

### 2. `work/<slug>/footprints.json` — `{"footprints":[ … ]}`

One footprint object per distinct land pattern your kept rows use (rules:
plan-27-B § 4.1, § 4.2):
- Source: the datasheet's recommended PCB land pattern, top view. Through-hole
  with only hole positions: pad = printed land diameter, else 1.7 × drill
  rounded to 0.05 mm, round. SMD: `drill` omitted.
- Pads are the electrical terminals only, numbered as the datasheet numbers
  them (1…N). No pegs, mounting holes, shield/ground tabs (a "ground pin"
  option's extra terminal is not a switch terminal: leave it out and note it).
- Style `dual` (two columns; `pins` 4 + `pitch` + `span` (+`perSide`/`slots`
  when the numbering is not the default counter-clockwise order); `pins` 2 +
  `perSide` 1 for a pair) when it fits, else `explicit` with pad `x`, `y` in
  mm, origin at the body centre, y down. Orientation: the drawing's top view,
  unrotated.
- `body`: the body outline width × height (mid-values for ranges).
- `note`: the PDF file name and page of the drawing, every mid-value used with
  its range, and every other series that shares this footprint.
- `id`: `SW-TACT-<SERIES>` (series upper-case, characters outside A-Z0-9.-
  dropped/replaced by `-`), with `-SMD`, `-THT`, `-SMD-RA`, `-THT-RA`, `-EDGE`
  when one series needs several; if one (series, mounting) has several land
  patterns (e.g. by height or lead type), add a short distinguishing suffix
  (`-G`, `-J`, `-H25`). Two series with the same land pattern (every pad within
  0.05 mm, same numbering, body within 0.1 mm) share one id.
- Validate the JSON shape; load-check with
  `planning/plan-27-data/census/fp-load.sh planning/plan-27-data/work/<slug>/footprints.json`
  → it must end `N new footprints: load ok` (only "extra" and "unused" lines
  for your new ids). It copies the tree to /tmp/p27-fp; if another reader is
  running it at the same moment it may clash — just run it again.

### 3. `work/<slug>/specs.tsv` — header `maker series mounting mpn_regex terminals pins footprint note`

One or more rows per (maker, series, mounting) of your kept rows:
- `mounting` exactly as in candidates.tsv (for `check` rows: the value you
  set in checks.tsv).
- `mpn_regex`: which part numbers of the group the row covers (`.*` for all);
  several rows when drawing or numbering differs. Every kept part must match
  exactly one row of its (maker, series, mounting).
- `terminals`: `2` or `4` (count of electrical switch terminals).
- `pins`: `A=<n>[,<n>];B=<n>[,<n>]` — side A and side B of the contact; pins on
  one side are connected inside the part (4-terminal: `A=1,2;B=3,4` means 1–2
  always connected, 3–4 always connected, pressing joins A to B). Read it from
  the circuit diagram / "schematic" / terminal-connection figure. Both sides
  list the same count. Numbers 1…terminals, each once.
- `footprint`: the id from your footprints.json.
- `note`: the PDF and page of the circuit diagram.
Check with:

    python3 -I planning/plan-27-data/census/specs-check.py --maker '<maker key>' \
      --tsv /tmp/p27-<slug>-cand.tsv --specs planning/plan-27-data/work/<slug>/specs.tsv \
      --footprints planning/plan-27-data/work/<slug>/footprints.json

after applying your schemes to the /tmp copy (scheme-check without --dry-run on
the copy) → `specs ok (N add rows)`. Rows you set `out` in checks.tsv must
also be set `out` in the copy before this check (edit the copy only).

### 4. `work/<slug>/checks.tsv` — header `mpn decision reason circuit mounting actuator rating`

One row per `check;` row of your makers: `decision` `keep` (it is a
non-illuminated SPST-NO or SPST-NC tactile switch with an in-scope mounting:
Surface Mount, Through Hole, Through Hole Right Angle, Surface Mount Right
Angle, Edge Mount Right Angle) or `out` with reason `circuit` / `lit` /
`mounting`. For `keep`: `circuit` `SPST-NO`/`SPST-NC`; `mounting` one of
`Surface Mount`, `Through Hole`, `Through Hole, Right Angle`, `Surface Mount,
Right Angle`, `Edge Mount, Right Angle`; `actuator` in DigiKey's vocabulary
(`Standard`, `Plunger for Cap`, `Extended`, `Flat`, …: match what DigiKey uses
for the same series' other rows); `rating` `<amps>A @ <volts>VDC` (the DC
switching rating from the specification table). Empty file (header only) if
you have none.

### 5. `work/<slug>/notes.md`

Per series: PDF(s) read, key page, drawing page, decisions, anything odd
(e.g. a ground-pin variant, a part whose drawing differs, a regex you were
unsure of). End with counts: kept rows, regex add / skip from scheme-check,
footprints, specs rows.

## Quality bar

A wrong pad coordinate or pinout is silent and ships on every part of the
series. Read the drawing, not memory; check that pad positions are consistent
with the body and pitch printed; mid-values for ranges. When a datasheet has
no land pattern at all for a group, use the terminal/lead drawing per
plan-27-B § 4.1 rule 1 and say so in the note. If a group truly cannot be
drawn (no dimensions at all), list its part numbers in notes.md under
"Cannot draw" with the reason — the main session will skip them.

Report back (your final message): per maker, kept / regex-add / regex-skip /
out counts, number of footprints, number of specs rows, the three checker
results (exact last lines), and any "Cannot draw" list.
