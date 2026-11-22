# plan-44-C / D: drawing the crystal and resonator footprints — the reader's brief

You draw the land patterns for one *worksheet*: `planning/plan-44-data/fp-parts/<name>.tsv`
(`c1`–`c4`: surface-mount crystals, plan-44-C; `d1`: through-hole crystals,
`d2`: ceramic resonators, plan-44-D). Paths below are relative to the worktree
root `/Users/justinzaun/Development/logic/.claude/worktrees/P-44` — use absolute
paths in tool calls.

## What a worksheet is

One row per *line* of `footprints-needed.txt`: a (maker, DigiKey Package / Case,
supplier package, size) combination of parts that will be in the library. Columns:

| Column | |
|---|---|
| `letter maker package supplier size label` | the line (do not change). `label` is the footprint id without its maker suffix, e.g. `XTAL-3225-4`: `XTAL-` (crystal) or `CRES-` (ceramic resonator) + the body in mm × 10 long side first (`3225` = 3.2 × 2.5 mm; `118X55` = 11.8 × 5.5; `HC49US-SMD`, `HC49U`, `CYL-2X6` (diameter × length), `SOJ-5.5`…) + the **pad count** |
| `footprint` | **you fill**: the footprint id this line uses |
| `pinouts` | the pinouts of the parts of this line (`1=X1,2=NC,3=X2,4=NC` …): the footprint's pad numbers are the pinout's designators, so the footprint must have exactly this many pads, numbered `1`…`N` as the datasheet numbers its pads |
| `note` | **you fill**: why this id (`reuse` / `shared with <id>` / the drawing it comes from) |
| `parts example datasheet pdfs existing` | for you: part count, the most-stocked part, its DigiKey datasheet link, **the local PDFs** (under `datasheets/crystal/…`; read them with `pdftotext -layout` and, for drawings, the Read tool's PDF page view), and existing library footprints with the same body size code |

The result of your work is: every row's `footprint` and `note` filled in the
worksheet, and every *new* footprint object in `planning/plan-44-data/fp-parts/<name>.json`
(`{"format": "schematical-footprints", "version": 1, "footprints": [ … ]}`).

## The rules

Read first: `docs/parts-format.md` § Footprints (lines 460–794: styles `dual` and
`explicit`, `pad`, `drill`, `body`, `pin1Shape`, `slots`; the rules R1–R9,
mid-values, no overlapping pads) and `planning/completed/plan-03-B-logic-footprints.md`
§ 3 (the pad rules R1–R6). Then `planning/plan-44-C-crystal-footprints-smd.md` § 3 (ids,
reuse, shared ids, styles) and, for d1 / d2, `planning/plan-44-D-crystal-footprints-leaded-resonators.md` § 3.

In short:

1. **Pads come from the datasheet's recommended land pattern / "recommended
   footprint" / "suggested PCB layout"** (rule R1). Use that, in mm, with
   mid-values where the drawing gives min and max. When the datasheet prints no
   land pattern, draw one from the package outline with the outline rules R2–R6
   (surface mount) / R7–R8 (through hole: hole = lead diameter + 0.3 mm, pad =
   hole + 0.6 mm unless the datasheet recommends otherwise), and say so in the
   `note`. Many makers' drawings give only the *outline* and the terminal
   (electrode) dimensions: then pads extend past the terminal edges by the usual
   crystal practice (a pad about 0.1–0.2 mm longer than the terminal on each
   side) — say what you did.
2. **Ids:** `<label>-<MAKER>`, MAKER `ABRACON`, `ECS` or `SUNTSU`
   (e.g. `XTAL-3225-4-ABRACON`). Two drawings of the same label and maker that
   differ get `-B`, `-C` after the maker. Lines whose drawings are equal (same pad
   centres and sizes within 0.02 mm, same numbering) **share one id** — the id of
   the first one you draw; the other rows say `shared with <id>`.
3. **Reuse an existing footprint** only when its pads equal the drawing's:
   `python3 -I planning/plan-44-data/census/fp-compare.py <existing id> <pad table file>` (pad table: one pad per line,
   `pin x y width height shape`, mm, origin at the body centre, x right, y down)
   prints the differences or `equal`; `fp-compare.py --pads <id>` prints an
   existing footprint's pads. The `existing` column lists same-size library
   footprints (mostly active oscillators `OSC-…-4`): a crystal's numbering and pad
   positions often differ — compare, do not assume. If equal, `footprint` = that
   existing id and `note` = `reuse`; no object is added.
4. **Style:** a two-pad part is `dual`, `pins` 2, `perSide` 1, `span` = pad-centre
   distance (pad 1 left, pad 2 right; a perSide of 1 needs a `pitch`: any positive
   number, use 1); a four-pad part is `dual` with `pins` 4 (pads 1, 2 down the left
   side, 3, 4 up the right side) when the pads are one regular rectangle of equal
   pads, else `explicit` (HC-49/US surface-mount tabs, SOJ legs, mixed pad sizes,
   three-pad resonators with a middle pad, anything with unequal pads). Every
   footprint has a `body` (the package outline: `width` along x, `height` along y,
   `"round": true` for a round can seen from above), a `description`, and a
   `note` naming the drawing, the PDF file under `datasheets/crystal/` and the
   page; record the mid-values you used and any arithmetic in the `note` (e.g.
   `lead 0.46 mid (0.38-0.53)`).
5. **Numbering and orientation:** the footprint's pad numbers are the datasheet's
   pad numbers (the `pinouts` column's designators; pad 1 is the pad the datasheet
   marks as 1). The library's frame is: origin at the body centre, x right, y down,
   `dual` pad 1 at the top of the left side, numbering counter-clockwise seen from
   the top. Datasheets often draw the **bottom** view of the terminals (a mirror
   image of the top view) next to a **top-view** recommended land pattern — keep
   the numbering that the *top view* of the board gives (mirror a bottom view
   before writing coordinates). Rotate the drawing by multiples of 90° so pad 1
   lands where the style wants it (top of the left column for `dual`); if the
   datasheet's numbering then does not follow `dual`'s order, use `explicit`
   pads with the datasheet's numbers at the drawing's positions.
6. **No overlapping pads**; a through-hole `drill` smaller than both pad sides;
   no `pin1Shape`/rect marking needed for a crystal (no polarity), except a
   through-hole three-lead can whose datasheet marks pad 1.

## Checks (your feedback loop)

```
python3 -I planning/plan-44-data/census/fp-check.py <C|D> --map planning/plan-44-data/fp-parts/<name>.tsv --json planning/plan-44-data/fp-parts/<name>.json
planning/plan-44-data/census/fp-load.sh planning/plan-44-data/fp-parts/<name>.json
```

`fp-check.py` must end `<n> lines, 0 without a footprint` (every row has an id that
exists, with as many pads as its pinouts have pins). `fp-load.sh` builds the
backend's `footprint_check` in a temp copy (about 3 minutes) and must end
`<k> new footprints: load ok` — it rejects a malformed object, overlapping pads
and an unknown key. Run `fp-check.py` often, `fp-load.sh` when your objects are
done (and after fixing anything it complained about).

## Hand check (required)

For each *family* of your rows (one per package type and size class — e.g. a 2-pad
3.2 × 1.5, a 4-pad 2.0 × 1.6, an HC-49/US tab pattern), check **one** object
against its drawing's dimensions by recomputing the pad centres and sizes from the
dimensions, and write the arithmetic in the object's `note`.

## Constraints

- Edit only your worksheet `<name>.tsv` (columns `footprint` and `note`; do not
  change the other columns, keep tabs as tabs, quote nothing yourself) and your
  `<name>.json`. Do not touch `Parts/footprints.json`, other worksheets, other
  scripts or any part file; no git commands; no network or DigiKey calls.
- Do not delete PDFs.
- The shell here refuses heredocs, loops and some compound commands: put any
  script you need in a file with the Write tool under
  `/private/tmp/claude-501/-Users-justinzaun-Development-logic/ede5c4a1-c37d-46ad-aa3b-165359fd7ae6/scratchpad/`
  and run it with `python3 -I <file>`. Write the worksheet with a script (read it
  with `csv`, set `footprint` / `note` per row, write it back with tabs) rather
  than hand-editing 20 rows; keep the columns and their order.

## Report (your final message)

1. The final output of `fp-check.py` and `fp-load.sh`.
2. Per label: the id(s) you used, drawn or `reuse` / `shared with`, with the
   source (PDF, page) in one line.
3. Every place you had to guess (no land pattern in the datasheet, a drawing you
   could not read, an ambiguous pad numbering, a pad size you derived from the
   outline) — `label | maker | what you did`.
4. Any line whose pinout cannot be placed on the pads the drawing shows (a
   pad-numbering conflict with `pinouts`).
