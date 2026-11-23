# plan-47-D: Footprints — inductors (and the footprint tools E to H share)

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-C

D draws the land patterns of the inductors of the final pick from their
datasheets' drawings: chip inductors (0201 to 1812), other surface-mount
inductors (moulded and shielded power inductors, mostly two pads), and
radial and axial through-hole inductors. It also writes the tools the other
footprint letters (E fuses, F PTCs, G relays, H buzzers) reuse: a generator
for two-terminal footprints from a dimension table, a geometry compare, and
the map from footprint lines to ids. The behavioral outcome: every D line of
`footprints-needed.txt` has a footprint id in `footprint-map.tsv`, and the
new objects of `planning/plan-47-data/footprints-D.json` load through
`footprint_check`.

References:

- `docs/parts-format.md` § Footprints (`dual`, `explicit`; § The
  `"footprint"` part key: the pad rules R1–R6, mid-values, R7 axial leads,
  R8 radial leads, no overlapping pads); `docs/backend.md` § Tests
  (`footprint_check`, the `footprints.txt` golden).
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (R1–R6).
- `planning/completed/plan-40-C-transistor-footprints-through-hole.md` § 3.1
  (ids, reuse, shared ids, ready objects) and its tools
  (`planning/plan-40-data/census/fp-load.sh`, `fp-merge.py`, `fp-check.py`).
- Library precedents: `RES-0603` (a `dual` two-pad chip: `pins` 2,
  `perSide` 1, `span`, one `pad`), `RES-AXIAL-10.16` (R7),
  `CAP-RADIAL-2.54` (R8).
- plan-47-B § 3.3 (the pinouts: which pads are terminals, which `NC`).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-C complete | `ls planning/completed/plan-47-C-*.md` → exists | NOT MET (2026-11-16: not started) |
| The pick is final | `awk -F'\t' '$23=="add"' planning/plan-47-data/candidates.tsv \| wc -l` → the pick C recorded (2953 at planning time) | NOT MET (2026-11-16: 0 `add` rows) |
| `footprints-needed.txt` is the `add` rows' | `python3 -I planning/plan-47-data/census/footprint-lines.py planning/plan-47-data/candidates.tsv schematical/schematical/Parts/footprints.json --add \| cmp - planning/plan-47-data/footprints-needed.txt && echo same` → `same` | NOT MET (2026-11-16: the file is the download set's) |
| No `IND-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith('IND-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/fp-check.py D` → `<n> lines, 0
  without a footprint` (`<n>` = the D lines; 229 at planning time, from the
  download set).
- `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-D.json`
  → `<k> new footprints: load ok`.
- `dims-D.tsv` has one row per new two-terminal footprint with the page of
  its drawing.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the
  app (K merges the objects when the parts that use them land).
- No edit of an existing footprint.
- No footprint for a package no `add` row uses.
- No 3D, courtyard or silkscreen data beyond the format's `body`.

## 2. Current State

`footprints.json` has 1646 footprints and none for an inductor
(`python3 -I` over its ids for the prefix `IND-` → 0). Its chip footprints
(`RES-0402` … `RES-2512-YAGEO`, `CAP-0402-KEMET` …) belong to resistor and
capacitor drawings; an inductor of the same size code reuses one only when
its datasheet's land pattern equals it (§ 3.1).

### Measured populations

At planning time, from the download set (C regenerates the file from the
`add` rows; re-measure at D's start).

| What | Count | Command |
|---|---|---|
| D lines | 229 (750 parts): Würth 114, Bourns 88, Murata 27 | `awk -F'\t' '!/^#/ && $1=="D"{print $3}' planning/plan-47-data/footprints-needed.txt \| sort \| uniq -c` |
| …chip codes (`0201` … `1919`) | 71 lines, 398 parts | `awk -F'\t' '!/^#/ && $1=="D" && $6 ~ /^[0-9]+ \(/' … \| wc -l` |
| …other surface mount (`Nonstandard` 75 of them) | 75 lines, 202 parts | `awk -F'\t' '!/^#/ && $1=="D" && $5=="Surface Mount" && $6 !~ /^[0-9]+ \(/' … \| wc -l` |
| …radial | 68 lines, 117 parts | `$5=="Through Hole" && $6 !~ /^Axial/` |
| …axial | 15 lines, 33 parts | `$6 ~ /^Axial/` |
| …lines with an existing footprint of the same size code (the reuse hint) | 53 | `awk -F'\t' '!/^#/ && $1=="D" && $13!=""' … \| wc -l` |
| Datasheets behind the lines | 416 distinct PDFs for the type; Würth's are one per part | plan-47-B § 2 |

### Verified properties

- **`footprint_check` loads any object of `footprints.json`'s format.**
  `fp-load.sh` appends the objects to a copy of `footprints.json`, builds
  `footprint_check` there and expects only `extra footprint` / `unused
  footprint` lines for the new ids (`planning/plan-40-data/census/fp-load.sh`
  header).
- **A two-pad `dual` footprint is `pins` 2, `perSide` 1, `pitch` 1, `span`,
  one `pad`**, with `drill` for through holes (`RES-0603` and
  `CAP-RADIAL-2.54` in `footprints.json`).
- **An `explicit` footprint may give one pin several pads** and unnumbered
  pads with `"pin": 0` (`docs/parts-format.md` § `explicit`): the shape for
  an inductor with two pads per terminal or with mechanical pads that the
  pinout does not name.

## 3. Design

### 3.1 Ids, reuse, shared ids

One id scheme for the five footprint letters (prefix `IND`, `FUSE`, `PTC`,
`RELAY`, `BUZZER`); dimensions are the drawing's body in mm × 10, a side of
10 mm or more spelled out with `X` between the two (`IND-125X125`):

| Family | Id | Example |
|---|---|---|
| two-pad surface mount | `<PREFIX>-<LLWW>-<MAKER>` (body length × width) | `IND-1608-MURATA`, `IND-6060-BOURNS` |
| …with more pads | the same plus `-<pads>` | `IND-4040-4-WURTH` |
| radial leads | `<PREFIX>-RAD-<D>-<P>-<MAKER>` (body diameter or length, lead spacing) | `IND-RAD-78-50-BOURNS` |
| axial leads | `<PREFIX>-AX-<L>-<P>-<MAKER>` (body length, pad span) | `IND-AX-71-127-BOURNS` |

MAKER in capitals (`BOURNS`, `MURATA`, `WURTH`). Two drawings of one maker
that give the same id but different pads get `-B`, `-C`.

Reuse: `census/fp-compare.py <footprint id> <pad table>` prints the
difference between an existing footprint's pads and a drawing's (centre x,
y, width, height, shape and drill per pad number, tolerance 0.02 mm); an
existing footprint (a `RES-…` or `CAP-…` chip, or one drawn earlier in
plan-47) serves a line when there is no difference: the map row names it,
`note` `reuse`. Shared ids: lines of different makers whose drawings are
equal share the id of the first one drawn (`shared with <id>`).

### 3.2 The generator and the dimension table

`census/fp-gen.py <dims-X.tsv> > footprints-X.json` writes one `dual`
object per row of a dimension table (`dims-D.tsv` here; E, F and H have
their own):

| Column | Value |
|---|---|
| `id` | § 3.1 |
| `family` | `chip` (two pads), `radial`, `axial` |
| `body_w`, `body_h`, `round` | the body as seen from above, mm (mid-values); `round` 1 for a cylinder standing on the board |
| `span` | pad-centre distance: the land pattern's (R1), a radial part's lead spacing (R8), an axial part's by R7 |
| `pad_w`, `pad_h`, `shape` | the land (R1; else the outline rules R2–R6); a through-hole pad round by R5 |
| `drill` | by R5 from the lead diameter; empty for surface mount |
| `pdf`, `page`, `arith` | the drawing's file and page; the arithmetic for the `note` (`lead 0.6 mid (0.55-0.65) -> drill 0.9`) |

A row is one footprint; the table is filled by hand from the drawing (the
land-pattern table when the datasheet prints one). A footprint the generator
cannot express (more than two pads, pads of two sizes) is written by hand as
an `explicit` object into `footprints-D-explicit.json`; `fp-load.sh` takes
both files.

The unpolarized rule: an inductor has no pin-1 mark (`pin1Shape` unset, the
`RES-RADIAL-2.54` precedent); pad 1 is the left pad.

An inductor lying on the board with radial leads ("Radial, Horizontal"):
the body is its length × diameter beside the pads; the footprint is
`explicit` with the two pads at the lead spacing and the body offset as the
drawing shows.

### 3.3 The map

`footprint-map.tsv`: one row per footprint line, key = the line's first nine
columns (letter, type, company, series, mounting, package, supplier_package,
size, detail), then `footprint` (the id), `label` (the part's `"package"`,
plan-47-B § 3.3) and `note` (`reuse`, `shared with <id>`, or the drawing).
`census/fp-check.py <letter>…` (adapted from plan-40's to these columns;
one or more letters) joins the letters' lines with the map and with the ids known
(`footprints-*.json` of plan-47 and `Parts/footprints.json`).

Where the risk concentrates: the smallest chips (0201, 0402: a 0.05 mm
error is another pad) and DigiKey's `Nonstandard` lines, whose key is
DigiKey's size text, so two series with different lands can share a line;
the map's key includes the size and the rows are checked per datasheet
(each line's `datasheets` count is in the file), and a line whose datasheets
disagree is split by adding the series to its map rows (`fp-check.py`
accepts a `series` on any letter's row).

Rejected alternatives: (1) one footprint per size code shared by every
maker (rejected: the library keeps one per maker drawing and reuse by
geometry already merges the equal ones); (2) generating footprints from
DigiKey's size text (rejected: the rule is the datasheet's drawing;
`AGENTS.md` § Datasheets).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools and the map skeleton

- [x] (2026-11-23: tools copied with p40 → p47, `fp-check.py` rewritten for the line and map columns) Copy `planning/plan-40-data/census/fp-load.sh`, `fp-merge.py` to
      `planning/plan-47-data/census/` with `p40` → `p47`; adapt
      `fp-check.py` to this plan's line columns (§ 3.3).
- [x] (2026-11-23: `fp-gen.py` and `fp-compare.py` written and run (IND-TEST: `1 new footprints: load ok`; RES-0603 `same`, RES-0402 differences)) Write `census/fp-gen.py` (§ 3.2) and `census/fp-compare.py` (§ 3.1).
- [x] (2026-11-23: `census/map-skeleton.py` → 929 rows) `footprint-map.tsv` skeleton: one row per line of every letter (D to
      H), the footprint column empty.

Acceptance: the tools run.
  Check: `python3 -I planning/plan-47-data/census/fp-check.py D` → `<n> lines, <n> without a footprint`; `printf 'id\tfamily\tbody_w\tbody_h\tround\tspan\tpad_w\tpad_h\tshape\tdrill\tpdf\tpage\tarith\nIND-TEST\tchip\t1.6\t0.8\t0\t1.6\t0.8\t0.9\trect\t\tx.pdf\t1\ttest\n' > /tmp/p47-dims.tsv && python3 -I planning/plan-47-data/census/fp-gen.py /tmp/p47-dims.tsv > /tmp/p47-fp.json && planning/plan-47-data/census/fp-load.sh /tmp/p47-fp.json` → `1 new footprints: load ok` (est. 4 min: it builds `footprint_check` in a copy).
Commit: —

### Phase 2 — Chip inductors

- [x] (2026-11-23: D-bourns-murata and D-wurth slices; reuse by `fp-compare.py` found 1 (Würth 0201 = RES-0201-VIKING) and 3 Bourns CAP-AX bodies) The chip-code lines (71 at planning time): a `dims-D.tsv` row per
      distinct drawing, or a map row to an existing footprint by
      `fp-compare.py`; fill the map rows.
- [x] (2026-11-23: by the slice agents (their reports), pad numbers from the printed land tables) Hand-check one footprint per size code against its drawing.

Acceptance: those lines have footprints that load.
  Check: `python3 -I planning/plan-47-data/census/fp-gen.py planning/plan-47-data/dims-D.tsv > planning/plan-47-data/footprints-D.json && planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-D.json` → `<k> new footprints: load ok` (est. 3 min).
Commit: —

### Phase 3 — Other surface-mount inductors

- [x] (2026-11-23: explicit objects for the 4-pad and staggered-pad parts) The remaining surface-mount lines (75 at planning time): `dims-D.tsv`
      rows for two-pad parts, `explicit` objects in
      `footprints-D-explicit.json` for the rest (pads per plan-47-B's
      pinout: `NC` pads numbered, mechanical pads the pinout does not name
      `"pin": 0`).
- [x] (2026-11-23: by the slice agents) Hand-check one footprint per series.

Acceptance: those lines have footprints that load.
  Check: `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-D.json planning/plan-47-data/footprints-D-explicit.json` → `<k> new footprints: load ok` (est. 3 min).
Commit: —

### Phase 4 — Radial and axial inductors

- [x] (2026-11-23: R7 / R8 rows and explicit hole patterns (toroids, rod chokes, HIDA slots)) The through-hole lines (83 at planning time: radial 68, axial 15): R8
      and R7 rows in `dims-D.tsv`; horizontal radial parts `explicit` (§
      3.2).
- [x] (2026-11-23: the arithmetic is in each dims row and `note`) Hand-check one footprint per series; the arithmetic is in `arith`.

- [ ] After A's last refill and C's final pick: regenerate `footprints-needed.txt` (`footprint-lines.py … --add`), `fp-check.py D` for lines new to the final pick (draw them), `fp-load.sh` over `footprints-*.json`, re-record the counts.
Acceptance: every D line has a footprint that loads.
  Check: `python3 -I planning/plan-47-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, and `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-D.json planning/plan-47-data/footprints-D-explicit.json` → `<k> new footprints: load ok` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh` (pads load, no overlap, the
  new ids are the only extras).
- Coverage check: `fp-check.py D` joins every D line with a map row.
- Runtime proof: n/a (K puts the ids on parts and regenerates
  `backend/tests/golden/footprints.txt`).
- Doc sync: none (`docs/parts-format.md` § Footprints covers the styles and
  rules used).
- Final gate: plan-47-O.

## Decisions

- **Ids carry the maker and the drawing's dimensions** (§ 3.1), as the
  library's `CAP-0603-KEMET` and `RES-0603-YAGEO` do; equal drawings share
  one id.
- **Two-terminal footprints come from a dimension table and a generator**
  (§ 3.2): 619 of the plan's 1076 lines are inductors, fuses and PTCs, and a table
  row per drawing keeps the source of each number (file, page, arithmetic)
  next to it.
- **A resistor or capacitor chip footprint is reused only on a geometry
  match**, never by size code alone.
- **No pin-1 mark on an inductor.**

## Corrections

- **2026-11-23, order and method.** The footprint letters D to H were worked in one
  wave on the provisional pick (`census/verdicts.py --out candidates-provisional.tsv`;
  `footprints-needed.txt` is its lines: D 208, E 91, F 250, G 194, H 180 = 923), because A's
  Phase 2 waits for DigiKey's quota; C's final pick (after A) can add lines the pick
  did not have. The work was split into slices (`fp-lines/<slice>.txt`, 11 agents) with
  per-slice files instead of one file per letter: `dims-<L>-<slice>.tsv`,
  `footprints-<L>-<slice>.json` (`fp-gen.py`), `footprints-<L>-<slice>-explicit.json`,
  `footprint-map-<L>-<slice>.tsv` (`fp-check.py`, `gen-parts.py` and `fp-load.sh` read them all), and
  `footprint-parts-H-*.tsv` where parts of one line need different footprints. All 22
  `footprints-*.json` loaded together: `census/fp-load.sh footprints-*.json` → `889 new
  footprints: load ok` (no duplicate id across slices); `fp-check.py D E F G H` → every
  letter `0 without a footprint`; 885 of them are used by the 2816 provisional parts and
  now in `Parts/footprints.json` (IND 190, FUSE 57, PTC 209, RELAY 156, BUZZER 273).
- **Footprint map.** A map row may carry a `series_split` (a series, or a part
  number: `gen-parts.py` tries the part number, then the series, then the whole line);
  `footprint-map.tsv` is the skeleton (empty rows serve nothing). Lines whose parts
  differ in pad pattern are served per part by `footprint-parts-*.tsv`
  (mpn or pdf, footprint).
- **Doubts the slice agents recorded** (hand-check candidates): Würth WE-SD 744711015 (no land
  pattern: outline), Bourns CV201210 pad height scaled, LQP03 / LQW15 / LQW18 lines whose
  datasheets disagree within one series, several Mallory / PUI / Same Sky SMD buzzers
  with no land pattern (R2 outline pads, polarity assumed), the CII hermetic TE relays'
  pin numbers (no numbers on the bottom views), Franjobaim FAM-4 and TE V23700 (no PC board
  pattern in the datasheet), TO-5 relay pads (0.9 x 1.5 ovals so the loader does not see
  bounding boxes touching).

## Summary

D's risk is the smallest chips and DigiKey's `Nonstandard` lines; the
geometry compare, the per-size and per-series hand checks and the page
reference on every row bound it. D also leaves the tools E, F, G and H use.
Untouched: every existing footprint, the part files, the backend and the
app.
