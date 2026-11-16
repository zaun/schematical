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
| plan-45-B complete | `ls planning/completed/plan-45-B-*.md` → exists | NOT MET (2026-11-16: not started) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the C lines (17 at planning time, before B's pick) | NOT MET (2026-11-16: 17 from the candidates) |
| The plan-40 tools exist | `ls planning/plan-40-data/census/fp-load.sh planning/plan-40-data/census/fp-merge.py` → both exist | MET (2026-11-16) |
| No `TB-` or `JST-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith(('TB-','JST-')) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0 of 1646) |

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

- [ ] `scripts/gen-terminals.js` (§ 3.1: `--letter`, `--out`, `--write`,
      `--check`) and an empty `scripts/terminal-footprints.json`
      (`{"variants": []}`).
- [ ] `scripts/gen-terminals.test.js` (`node --test`): a table of two
      variants (one 5.00 mm vertical, one 3.50 mm horizontal) gives the
      expected objects (positions, `body.width`, `firstRowY`, pad ≤ pitch −
      0.8); `--write` twice changes nothing; ids are unique and match
      `[A-Z0-9.-]+`.
- [ ] `census/fp-load.sh`, `census/fp-merge.py` (copies of plan-40's with
      `p45` in the temp directory name) and `census/fp-check.py` (every
      (variant, n) of the letter's lines has an object in
      `footprints-<letter>.json`).

Acceptance: the generator works on its test table.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min).
Commit: —

### Phase 2 — Phoenix Contact (5 variants)

- [ ] One table entry per Phoenix C line, from the placed per-item PDFs
      (`datasheets/connector-terminal/phoenix/<item>.pdf`, the `Drilling
      diagram` and the dimension drawing); `positions` from the final line.
- [ ] `node scripts/gen-terminals.js --letter C --out planning/plan-45-data/footprints-C.json`
      after each maker (the file grows).

Acceptance: the Phoenix lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → `<n> lines, <k> without a footprint` with the Phoenix lines absent from the missing list (est. 1.5 h of reading five drawings).
Commit: —

### Phase 3 — Weidmüller (6 variants)

- [ ] As Phase 2, from `datasheets/connector-terminal/weidmuller/<item>.pdf`
      (the raster dimensional drawing on the `Drawings` pages; the
      `Solder eyelet hole diameter` line gives the drill).

Acceptance: the Weidmüller lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → the Weidmüller lines absent from the missing list (est. 1.5 h).
Commit: —

### Phase 4 — Würth Elektronik (6 variants)

- [ ] As Phase 2, from `datasheets/connector-terminal/wurth/<item>.pdf` (the
      `Dimensions` and `Recommended Hole Pattern` of page 1).

Acceptance: every C line has its objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py C` → `<n> lines, 0 without a footprint` (est. 1.5 h).
Commit: —

### Phase 5 — Load and hand-check

- [ ] `census/fp-load.sh planning/plan-45-data/footprints-C.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
- [ ] For each variant, check its smallest and largest position counts
      against the drawing (pitch, hole, body, pad row), and record the check
      in the variant's `note` as the numbers the check changed.
- [ ] `docs/parts-format.md` § Footprints: a paragraph "Terminal-block
      footprints (`TB-`): `header` style, generated by
      `scripts/gen-terminals.js` from `scripts/terminal-footprints.json`, pads
      by R5-T" (G adds the JST and the merge).

Acceptance: the objects load and match their drawings.
  Check: `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-C.json` → exits 0, `<n> new footprints, 0 errors` (est. 10 min: a temp backend build).
Commit: —

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

## Summary

C is drawing work: 17 variants read from 17 PDFs, written as table rows. The
risk is a misread dimension; the generator makes it one number per variant.
Nothing reaches `footprints.json` until G.
