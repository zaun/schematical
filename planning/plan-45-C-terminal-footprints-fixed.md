# plan-45-C: Footprints — fixed (one-piece) terminal blocks, and the generator

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-B

C writes the footprint generator `scripts/gen-terminals.js` and its table
`scripts/terminal-footprints.json`, and draws the land patterns of the fixed
(one-piece, wire-to-board) terminal blocks of the final pick — 17 variants of
Phoenix Contact, Weidmüller and Würth Elektronik at planning time (screw,
spring-cage and push-in, 3.50 to 7.62 mm) — from the datasheets' hole
patterns. Its output is `planning/plan-45-data/footprints-C.json` (ready
objects, not yet in `footprints.json`: that is G's, with the parts that use
them). The behavioral outcome: every C line of `footprints-needed.txt` has an
object for each of its position counts, and the objects load through
`footprint_check`.

References:

- `docs/parts-format.md` § Footprints (`header`, `explicit`, the rules R1–R9,
  R5 for pads, mid-values, no overlapping pads); `docs/backend.md` § Tests
  (`footprint_check`, `footprints.txt` golden).
- `scripts/gen-headers.js` (the precedent: a generator that writes
  `PINHDR-` footprints from the Würth WR-PHD drawings, idempotent).
- `planning/completed/plan-40-C-transistor-footprints-through-hole.md`
  § 3.1 and its tools `planning/plan-40-data/census/fp-load.sh`,
  `fp-merge.py` (C adapts them as `census/fp-load.sh`, `fp-merge.py`).
- `planning/plan-45-data/footprints-needed.txt` (the C lines; B rewrites it).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-B complete | `ls planning/completed/plan-45-B-*.md` → exists | MET (2026-11-24: not archived (`ls planning/completed/plan-45-B-*.md` → no match), every B box ticked: `grep -c '\[ \]' planning/plan-45-B-terminal-jst-verdicts.md` → 0; B's commits ec922b72, 41b89bed, d5d472ee) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the C lines (17 at planning time, before B's pick) | MET (2026-11-24: 17 lines, 133 footprints (17 variants, 123 parts: B's pick plus the generics' 2…12), `final.py` wrote it) |
| The plan-40 tools exist | `ls planning/plan-40-data/census/fp-load.sh planning/plan-40-data/census/fp-merge.py` → both exist | MET (2026-11-24) |
| No `TB-` or `JST-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith(('TB-','JST-')) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-24: 0 of 2293) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `scripts/gen-terminals.js` exists, reads `scripts/terminal-footprints.json`
  and writes ready footprint objects for the table's variants of one letter
  (`--letter C --out planning/plan-45-data/footprints-C.json`), and, for G,
  writes into `footprints.json` the footprints the parts name (`--write`).
- `planning/plan-45-data/footprints-C.json` has one object per (variant,
  position count) of the C lines of `footprints-needed.txt` (124 at planning
  time: `awk -F'\t' '!/^#/ && $1=="C"{n+=gsub(/,/,",",$8)+1} END{print n}'
  planning/plan-45-data/footprints-needed.txt` → 124), and for each generic
  representative variant (B) all position counts 2 to 12.
- The objects pass `census/fp-load.sh`: no load error, no overlapping pads,
  only `extra footprint` and `unused footprint` lines for exactly the new ids.

### Non-goals (explicit constraints)

- No change to `footprints.json`, `Parts/*.json`, the goldens, the backend or
  the app (G merges).
- No existing footprint changes. There are no terminal-block footprints to
  reuse (`footprints-needed.txt` `existing` column empty for every line).
- A row whose hole pattern cannot be drawn as `header` pads (a staggered or
  multi-row pattern, a pad on the bottom layer) is made `skip` in
  `candidates.tsv` with the reason `no footprint` and listed in
  `datasheets/skipped-parts.md`, as the USB connectors' unreproducible parts
  were (`docs/parts-format.md` § `connector`); the letter's numbers are then
  corrected under § Corrections.

## 2. Current State

`footprints.json` has 1646 footprints; the header-style ones are the 160
`PINHDR-` lines of `scripts/gen-headers.js`: `style` `header`, `rows` 1,
`pitch`, `firstRowY`, `pad`, `drill`, `pin1Shape` `rect`, a `body` box; a
right-angle one puts the pad row above the housing (`firstRowY` −2.77) so the
mating face points down. The loader expands `header` to pads (§ Footprints
`header`: x = (c − (perRow − 1)/2)·pitch). `footprint_check` fails on a
footprint no part uses, which is why C's objects wait for G in
`footprints-C.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| C lines (variants) | **17**: Phoenix Contact 5, Weidmüller 6, Würth Elektronik 6 | `awk -F'\t' '!/^#/ && $1=="C"{n[$3]++} END{for(k in n) print k, n[k]}' planning/plan-45-data/footprints-needed.txt` |
| C footprints (variant × position count) | **124** | the formula in § 1 |
| Parts on them | 128 | `awk -F'\t' '!/^#/ && $1=="C"{n+=$9} END{print n}' planning/plan-45-data/footprints-needed.txt` |
| By pitch (mm) | 3.50: 3, 3.81: 3, 5.00: 3, 5.08: 3, 7.50: 3, 7.62: 2 (variants) | `awk -F'\t' '!/^#/ && $1=="C"{n[$5]++} END{for(k in n) print k, n[k]}' …` |
| By mounting | through hole 17, surface mount 0 | `$7` of the same |
| By orientation | vertical 4, horizontal 7, angled 6 | `$6` |

### Verified properties

- `header` style covers a single row of through-hole pads of one size with a
  body box (`gen-headers.js` writes 160 of them; the loader accepts them).
  A terminal block's pads are one row of equal holes at the pitch (the
  Phoenix `Drilling diagram`, Würth `Recommended Hole Pattern` and
  Weidmüller `Solder eyelet hole diameter` of plan-45-A § 2), so `header`
  draws every C variant except one with a staggered row, which is `skip`.
- No terminal drawing gives a pad size (A § 2 shows hole diameters only), so
  pads follow R5, as `gen-headers.js` does.

## 3. Design

### 3.1 The table and the generator

`scripts/terminal-footprints.json`: `{"variants": [ … ]}`, one entry per
variant of `footprints-needed.txt`:

| Key | Value |
|---|---|
| `"stem"` | the line's `label` (`PHO-COMBICON-PT-5.00-HT`); a footprint's id is `TB-<stem>-<NN>`, NN the position count in two digits (`TB-PHO-COMBICON-PT-5.00-HT-03`) |
| `"letter"` | `"C"`, `"D"` or `"E"`; E's stems already start with `JST-`, so its ids are `<stem>-<NN>` (`JST-XH-2.50-VT-03`) |
| `"positions"` | the position counts to write: the line's (B), plus 2…12 for a generic representative |
| `"style"` | `"header"` (C, D, E's through hole) or `"explicit"` (E's surface mount) |
| `"describe"` | the description's start: `Phoenix Contact COMBICON PT 1,5 PCB terminal block, 5.00 mm, horizontal` |
| `"note"` | the provenance: maker, item, file, page, and the numbers read (`Phoenix 1935161 p.5: hole Ø1.3, pitch 5.00, body 9.0 deep; pad R5-T`) |
| `"pitch"`, `"drill"`, `"pad"` | as the library's keys (`pad` `{width, height, shape}`) |
| `"firstRowY"` | the pad row's y from the body centre, mm, y down |
| `"bodyExtra"`, `"bodyDepth"` | body width = n·pitch + `bodyExtra`; body height = `bodyDepth` |

Output objects: `{"id", "description", "note", "style": "header", "pins": n,
"rows": 1, "body": {"width", "height"}, "pitch", "firstRowY", "pad", "drill",
"pin1Shape": "rect"}`, one per (variant, n), JSON on one line each, as
`gen-headers.js` writes them. `--letter X --out F` writes `{"footprints":
[…]}` for the letter's entries; `--write` replaces every `TB-` and `JST-`
line of `footprints.json` by the objects of the ids the `connector` parts name
as their `footprint` (idempotent: a second run changes nothing; G runs it, so
`footprint_check` never sees an unused footprint); `--check` exits 1 when
`footprints.json` differs from what `--write` would write.

### 3.2 Rules

- **R5-T (pads).** Round pad, diameter = drill + 0.7, but at most pitch −
  0.8 (so the copper gap is at least 0.8 mm at 3.50 mm); the datasheet's
  pad or land diameter when it prints one. Pin 1 is square (`pin1Shape`).
  Drill = the drawing's hole diameter, mid-value when it gives a min and a
  max; the note records it.
- **Orientation.** A vertical part (wire entry up from the board) has its
  pads centred on the body (`firstRowY` 0 when the housing is centred on the
  holes, else the drawing's offset). A horizontal or angled part has the pad
  row above the housing and the wire entry pointing down (+y), as the right-
  angle `PINHDR-` footprints: `firstRowY` = −(`bodyDepth`/2) + the drawing's
  distance from the housing's rear edge to the hole row.
- **Body.** The housing's outline from the dimension drawing: width n·pitch +
  `bodyExtra` (the end margins, from a drawing with a stated length for one
  position count and checked on a second), height `bodyDepth`. Where the
  datasheet gives lengths only for some n, the formula is the datasheet's
  (`L = n · P + k`).
- **Ids and notes** as the table; every number of a footprint is in its
  variant's `note`.

### 3.3 Risk

Correctness risk is in reading dimensions: a wrong `bodyExtra` or
`firstRowY` shifts every position count of a variant. Each variant's
smallest and largest position counts are checked against the drawing by
hand (Phase 5); the middle ones follow the formula. Nothing is merged into
the library yet.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator and its tests

- [x] `scripts/gen-terminals.js` (§ 3.1: `--letter`, `--out`, `--write`,
      `--check`) and an empty `scripts/terminal-footprints.json`
      (`{"variants": []}`).
      Also `--table`, `--footprints`, `--parts` overrides for the tests. `--check` on the real library → `ok: 0 TB-/JST- footprints`.
- [x] `scripts/gen-terminals.test.js` (`node --test`): a table of two
      variants (one 5.00 mm vertical, one 3.50 mm horizontal) gives the
      expected objects (positions, `body.width`, `firstRowY`, pad ≤ pitch −
      0.8); `--write` twice changes nothing; ids are unique and match
      `[A-Z0-9.-]+`.
      Six tests (the two variants and a JST one; the cap at pitch − 0.8; `--write` keeps the other lines, drops a stale `TB-` line and is idempotent; `--check`; an unknown named id; a duplicate id and a pad that does not fit): `ℹ tests 6, pass 6, fail 0`.
- [x] `census/fp-load.sh`, `census/fp-merge.py` (copies of plan-40's with
      `p45` in the temp directory name) and `census/fp-check.py` (every
      (variant, n) of the letter's lines has an object in
      `footprints-<letter>.json`).
      The copies are of plan-44's (which are plan-40's with its name); `fp-check.py C` → `17 lines, 17 without a footprint`, `133 footprints, 133 missing` (before any object exists).

Acceptance: the generator works on its test table.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min). Result: `ℹ pass 6`, `ℹ fail 0` (8 tests after the explicit style, Corrections).
Commit: 8e9d4165

### Phase 2 — Phoenix Contact (5 variants)

- [x] One table entry per Phoenix C line, from the placed per-item PDFs
      (`datasheets/connector-terminal/phoenix/<item>.pdf`, the `Drilling
      diagram` and the dimension drawing); `positions` from the final line.
      Six entries (the five lines and `PHO-COMBICON-SMKDS3-5.08-AT`, split off, Corrections). Read from the drilling diagrams (rendered and enlarged: `census/drawing.sh`): PT 5.00 H (1935161: Ø1.3, row 4 from the housing edge, depth 9), SMKDS 5.08 A (1705469: Ø1.4, row 5.4, depth 14.25), SPT-THR 3.81 V (1822422: two holes per pole 5.5 apart, Ø1.1, first hole 2 from the end), SPTA 3.50 A (1752117: Ø1.1, first hole 2.35, row 5.7 from the front of a 10 deep housing), SPT 7.50 V (1719325: poles alternate between rows 14 apart, Ø2.1, first hole 6.55, depth 18.5), SMKDS 3 (1713040: Ø1.3, row 7.5 from the front of a 16 deep housing).
- [x] `node scripts/gen-terminals.js --letter C --out planning/plan-45-data/footprints-C.json`
      after each maker (the file grows).
      Run after all three makers were in the table (one `--letter C` run writes the whole letter: the table is data, so the file did not need to grow step by step).

Acceptance: the Phoenix lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → `<n> lines, <k> without a footprint` with the Phoenix lines absent from the missing list (est. 1.5 h of reading five drawings). Result: `19 lines, 0 without a footprint` (no Phoenix line missing).
Commit: 4f629283

### Phase 3 — Weidmüller (6 variants)

- [x] As Phase 2, from `datasheets/connector-terminal/weidmuller/<item>.pdf`
      (the raster dimensional drawing on the `Drawings` pages; the
      `Solder eyelet hole diameter` line gives the drill).
      Seven entries (the six lines and `WEI-OMNIMATE-LM1N-5.08-HT`, split off). The Dimensions table gives depth, width (a formula checked at two position counts) and the hole Ø (`Solder eyelet hole diameter (D)`, "+0,1": the mid-value is the drill); the rasters (`census/wdm.sh` extracts them with `pdfimages`) give the rows: LLFS 7.50 (2473000000: poles alternate between rows 13.2 apart, first hole 6.6), LM 5.08 (1716020000: one row, pin 4.8 from the edge of a 10 deep housing), LM1N (1766320000: depth 12.6, pin 1.6 from the front), LSF-SMT 3.50 / 3.81 / 5.00 / 7.62 (two solder pins per pole: rows 7 / 5 / 5.5 / 5 apart).

Acceptance: the Weidmüller lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → the Weidmüller lines absent from the missing list (est. 1.5 h). Result: `19 lines, 0 without a footprint`.
Commit: 4f629283

### Phase 4 — Würth Elektronik (6 variants)

- [x] As Phase 2, from `datasheets/connector-terminal/wurth/<item>.pdf` (the
      `Dimensions` and `Recommended Hole Pattern` of page 1).
      Six entries from page 1 of 691137710002 (1377: row 3.5 / 4 from the edges of a 7.5 deep housing), 950008102500000 (0008: row on the housing edge), 950011202000000 (0011), 691411410002 (4114: two holes per pole 5 apart), 691412320002M (4123: staggered, numbered from the right: "start on the top right corner") and the series sheet 691412120002B (412 B: staggered, "start on the bottom left corner", A = 3.5 x NB PINS + 1.5).

Acceptance: every C line has its objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → `<n> lines, 0 without a footprint` (est. 1.5 h). Result: `19 lines, 0 without a footprint`, `135 footprints, 0 missing`.
Commit: 4f629283

### Phase 5 — Load and hand-check

- [x] `census/fp-load.sh planning/plan-45-data/footprints-C.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
      `135 footprints appended`, `135 new footprints: load ok` (the script's own wording: it fails on any other line and on overlapping pads).
- [x] For each variant, check its smallest and largest position counts
      against the drawing (pitch, hole, body, pad row), and record the check
      in the variant's `note` as the numbers the check changed.
      `census/fp-widths.py C --ends`: 91 body widths and 31 hole spans (Phoenix `Dimension a`) that the sheets print are compared with the footprints', every row, not only the ends: all equal except `691412320011` (Würth prints 44.41 for 11 positions where 10 and 12 give n x 3.81 + 1.5 = 43.41: a datasheet typo, in the variant's note). The smallest and largest compared counts per variant are equal (e.g. LSF 3.81: 2 pos 8.01, 11 pos 42.3). `census/fp-consistency.py C` found the two housings that did not share a land pattern (Corrections). Every variant's `note` says what was checked; no number changed after the check apart from the two splits.
- [x] `docs/parts-format.md` § Footprints: a paragraph "Terminal-block
      footprints (`TB-`): `header` style, generated by
      `scripts/gen-terminals.js` from `scripts/terminal-footprints.json`, pads
      by R5-T" (G adds the JST and the merge).
      Added as "### Terminal-block footprints (`TB-`)" before "Custom footprints in a part file": the id scheme, the generator and its `--write` / `--check`, `header` for a centred single row and `explicit` for the rest, R5-T, the split rule.

Acceptance: the objects load and match their drawings.
  Check: `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-C.json` → exits 0, `<n> new footprints, 0 errors` (est. 10 min: a temp backend build). Result: exit 0, `135 new footprints: load ok` (4 s: the build is incremental).
Commit: 4f629283

## Validation Plan

- Tests: `scripts/gen-terminals.test.js` (Phase 1), `fp-load.sh` (Phase 5).
- Doc sync: `docs/parts-format.md` § Footprints.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **A generator and a table, not 124 hand-written objects**, because
  a variant's footprints for 2 to 12 positions differ only by the position
  count (the `gen-headers.js` precedent), and a wrong dimension is then one
  table row to fix.
- Author: **`header` style for every through-hole terminal footprint**; the
  explicit style is for E's surface-mount parts, whose pads differ in size.
- Author: **Pad = drill + 0.7 at most pitch − 0.8 (R5-T)**: the datasheets
  print holes, not pads; the cap keeps 3.50 mm pads apart.
- Author: **The wire entry points down (+y) for horizontal and angled
  parts**, the library's right-angle header convention.

## Corrections

- **2026-11-24, Prerequisites.** `footprints-needed.txt` after B has 17 C lines and **133** footprints (123 parts), not 124 / 128: B's pick plus the generics' 2…12; after C's splits (below) **19 lines and 135 footprints** (`fp-check.py C`: `19 lines`, `135 footprints`). Test count and the other A-letter numbers are B's.
- **2026-11-24, Phase 1 and 2–4: 13 of the 19 variants are not `header` footprints; `explicit` draws them, no row is skipped.** § 2 "Verified properties" said `header` covers every C variant except a staggered one, which is `skip` (`no footprint`). The drawings show otherwise: two solder pins per pole (Phoenix SPT-THR, Weidmüller LSF-SMT ×4, Würth 4114), poles that alternate between two rows (Phoenix SPT, Weidmüller LLFS, Würth 4123 and 412 B) and housings whose first hole is not centred (Phoenix SPTA, Weidmüller LM and LM1N). `docs/parts-format.md` § `explicit` takes several pads on one pin, so the generator got an `explicit` style (`holes`, `stagger`, `edgeX`, `reverse` in the table; `scripts/gen-terminals.test.js` has the two new tests, 8 tests in all) and every row keeps its footprint: 6 variants are `header` (Phoenix PT, SMKDS, SMKDS 3; Würth 0008, 0011, 1377), 13 `explicit`. The two "no footprint" skips the plan allowed (§ Non-goals) are not needed.
- **2026-11-24, Phase 3 / 5: two housings that share a DigiKey series are split into their own variants.** A's variant key (maker, series, pitch, orientation, …) merged them. `census/fp-widths.py C` found `1766320000` and `1766330000` printing a width of n x 5.08 + 1.0 (11.16, 16.24) where the other `Omnimate LM` items print + 0.55 (the LM1N housing: depth 12.6, pin 1.6 from the front, against 10 and 5.2), and `census/fp-consistency.py C` found Phoenix `1713040` (SMKDS 3: depth 16, hole Ø1.3, row 7.5 from the front) among the `SMKDS 2,5` items (depth 14.25, Ø1.4). `census/split-variant.py` gave them the variants `WDM|Omnimate-LM1N|5.08|H|tht|screw` (positions 2, 3) and `PHX|COMBICON-SMKDS3|5.08|A|tht|screw` (position 4) in `candidates.tsv` and `specs.tsv`, and `footprint-lines.py` takes the id stem from the variant's series token so each gets its own (`WEI-OMNIMATE-LM1N-5.08-HT`, `PHO-COMBICON-SMKDS3-5.08-AT`; the other 17 labels are unchanged: `cmp` of the regenerated file before the splits). `footprints-needed.txt` has 19 C lines. D and E run the same two checks on their variants.
- **2026-11-24, Orientation.** § 3.2 puts the wire entry of a horizontal or angled part at +y. The sheets' plan views are drawn first- or third-angle and do not always show which edge is the wire entry, and turning a plan view through 180° reverses the pole order (Würth's 4123 and 412 B state which corner pole 1 is at). Each footprint therefore follows the datasheet's plan view as drawn (poles left to right, or as the sheet says, rows and housing edges as printed there), and each `note` says which dimensions were read from which edge. Where only a 400 to 800 px raster is available (Weidmüller) the housing outline's offset from the hole rows is good to about 0.3 mm; pitch, row distance and hole Ø are printed numbers.
- **2026-11-24, Drill and body.** Drill = the drawing's Ø, the middle of a "+0,1" (Weidmüller Ø1.3 +0.1 → 1.35, 1.1 +0.1 → 1.15, 2 +0.1 → 2.05); body depth = the Dimensions table's `Depth` / the drawing's housing depth, body width = `n x pitch + bodyExtra` from the printed length at two position counts. Würth `691412320011` prints 44.41 for 11 positions (10 and 12 follow n x 3.81 + 1.5 = 43.41): kept as a datasheet typo, in the variant's `note`.
- **2026-11-24, `fp-load.sh`** is a copy of plan-44's (themselves plan-40's); it finished in 4 s (the temp build reuses the cmake cache).

## Summary

C is drawing work: 17 variants read from 17 PDFs, written as table rows. The
risk is a misread dimension; the generator makes it one number per variant.
Nothing reaches `footprints.json` until G.
