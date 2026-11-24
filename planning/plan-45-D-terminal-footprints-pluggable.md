# plan-45-D: Footprints — board-side headers of plug-in terminal blocks

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-C

D draws the land patterns of the board-side headers of plug-in terminal
blocks of the final pick — 18 variants at planning time (Phoenix Contact BCH,
COMBICON GMSTBVA, ICV, PC and PST; Weidmüller Omnimate 4.0 MHS, SC and SL;
Würth WR-TBL series 0016, 0018, 311, 3114, 321 and 3221) at 3.50 to 7.62 mm,
vertical and right-angle, shrouded and unshrouded —
into `scripts/terminal-footprints.json` (C's generator) and
`planning/plan-45-data/footprints-D.json`. D also extends the generator with
the `explicit` style for headers whose drawing has flange screw holes
(non-plated) beside the pad row. The behavioral outcome: every D line of
`footprints-needed.txt` has an object for each of its position counts, and
the objects load through `footprint_check`.

References:

- plan-45-C § 3 (the table, the generator, the rules R5-T, orientation, body)
  and its tools; `docs/parts-format.md` § Footprints (`explicit`, unnumbered
  pads, non-plated holes).
- `planning/plan-45-data/footprints-needed.txt` (the D lines).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-C complete | `ls planning/completed/plan-45-C-*.md` → exists | MET (2026-11-24: not archived, every C box ticked: `grep -c '\[ \]' planning/plan-45-C-terminal-footprints-fixed.md` → 0; C's commits 8e9d4165, 4f629283) |
| The generator exists | `ls scripts/gen-terminals.js scripts/gen-terminals.test.js` → both exist | MET (2026-11-24: both exist) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the D lines (18 at planning time, before B's pick) | MET (2026-11-24: B's pick with the generics gave 18 D lines, 139 footprints, 197 parts; after D's splits and skips (Corrections) **24 lines, 152 footprints, 187 parts**) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-45-data/footprints-D.json` has one object per (variant,
  position count) of the D lines (142 at planning time: `awk -F'\t' '!/^#/ &&
  $1=="D"{n+=gsub(/,/,",",$8)+1} END{print n}' planning/plan-45-data/footprints-needed.txt`
  → 142).
- The generator writes `explicit` entries (§ 3) and passes its tests.
- The objects pass `census/fp-load.sh`.

### Non-goals (explicit constraints)

- No change to `footprints.json`, `Parts/*.json`, the goldens, the backend or
  the app. C's table entries and `footprints-C.json` are not touched.
- A header whose drawing needs a pad on the bottom layer, a slotted hole, or
  a staggered row is made `skip` (`no footprint`, in `candidates.tsv` and
  `datasheets/skipped-parts.md`), recorded under § Corrections.

## 2. Current State

C's generator writes `header`-style objects. A pluggable header is often a
shrouded block with the same single row of holes; its flange variants add two
mounting holes beyond the ends of the row. The loader's `explicit` style takes
literal pads, including unnumbered (`"pin": 0`) non-plated holes
(`docs/parts-format.md` § `explicit`).

### Measured populations

| What | Count | Command |
|---|---|---|
| D lines (variants) | **18**: Phoenix Contact 6, Weidmüller 6, Würth Elektronik 6 | `awk -F'\t' '!/^#/ && $1=="D"{n[$3]++} END{for(k in n) print k, n[k]}' planning/plan-45-data/footprints-needed.txt` |
| D footprints (variant × position count) | **142** | the formula in § 1 |
| Parts on them | 201 | `awk -F'\t' '!/^#/ && $1=="D"{n+=$9} END{print n}' planning/plan-45-data/footprints-needed.txt` |
| By pitch (mm) | 3.50: 3, 3.81: 3, 5.00: 3, 5.08: 3, 7.50: 3, 7.62: 3 (variants) | `awk -F'\t' '!/^#/ && $1=="D"{n[$5]++} END{for(k in n) print k, n[k]}' …` |
| By orientation | vertical 11, horizontal 7 | `$6` |
| By mounting | through hole 18 | `$7` |
| Candidate rows by header Type | Male Pins Shrouded (4 Side) 180, Blade Pins 7, Female Sockets 7, Male Pins Unshrouded 7 | `awk -F'\t' '$22=="candidate" && $6=="pluggable"{print $13}' planning/plan-45-data/candidates.tsv \| sort \| uniq -c` |

## 3. Design

- **Same table, same ids** as C: `TB-<stem>-<NN>` with the line's `label`.
- **`explicit` entries.** A table entry may say `"style": "explicit"` with
  `"flange": {"dx", "y", "drill"}`: the generator writes, for each n, pads 1…n
  at x = (c − (n − 1)/2)·pitch (pin 1 the `rect` one, `drill` and `pad` as
  `header`), and two non-plated holes (`"pin": 0`, round, width = height =
  `flange.drill`, `"plated": false`) at x = ±((n − 1)/2·pitch + `dx`) on y =
  `firstRowY` + `flange.y`. Output: `{"style": "explicit", "pins": n, "pads":
  […]}` plus `body`. Entries without `flange` stay `header`.
- **Pad rules** as C (R5-T), orientation as C: a vertical header's pads are
  centred on the shroud; a right-angle header's pad row is above the shroud
  and the mating face points down.
- **Body** is the shroud's outline (width n·pitch + `bodyExtra`).
- **Pin 1**: the pad nearest the datasheet's pin-1 mark; where the drawing
  numbers the pins right to left, pin 1 is still the left pad of the
  footprint and the drawing is read mirrored (the footprint is the same
  pattern: pins are 1…n along the row).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The `explicit` entries

- [x] `scripts/gen-terminals.js`: `style: explicit` with `flange` (§ 3).
      C already added the `explicit` style (`holes`, `stagger`, `edgeX`, `reverse`: C's Corrections); D adds `flange` ({dx, dy, drill, plated}: non-plated `pin: 0` pegs, or plated `pin: 0` unnumbered solder pads) and `gaps` (an empty slot between two poles).
- [x] `scripts/gen-terminals.test.js`: a 5.08 mm flange entry for n = 2 and
      n = 5 gives the expected pads (n signal pads, two `pin: 0` non-plated
      holes at the computed x); an entry without `flange` is unchanged.
      New test: a 5.08 flange (n = 2, 5), a plated solder flange (3.81, below the row) and a gap (4 poles on 5 slots); the existing tests stand for the entries without `flange`.

Acceptance: the explicit entries work.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min). Result: `ℹ tests 9`, `ℹ pass 9`, `ℹ fail 0`.
Commit: this commit (hash in the next one)

### Phase 2 — Phoenix Contact (6 variants)

- [x] Table entries from `datasheets/connector-terminal/phoenix/<item>.pdf`
      (`Drilling diagram`, dimension drawing).
      Six entries: BCH 3.50 V (5441278, dimensional drawing only), BCH 3.81 H (5444262), GMSTBVA 7.50 V (1766699), ICV 5.08 V (1785955: two holes per position), PC 7.62 H (1720466: three holes per position at 0, 5.08, 7.62; read on the diagram enlarged to 500 dpi: "1.83" is the distance from the housing edge to the first row), PST 5.00 V (1933189: an unshrouded pin strip 2.8 wide).
- [x] `node scripts/gen-terminals.js --letter D --out planning/plan-45-data/footprints-D.json`.
      Run once all three makers were in the table.

Acceptance: the Phoenix D lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → the Phoenix lines absent from the missing list (est. 2 h). Result: `24 lines, 0 without a footprint`.
Commit: this commit (hash in the next one)

### Phase 3 — Weidmüller (6 variants)

- [x] As Phase 2, from `datasheets/connector-terminal/weidmuller/<item>.pdf`.
      Twelve entries after the splits: MHS 5.00 and 7.50, SC (plain and flanged), SL 3.50 (plain and flanged), SL 5.08 (plain, solder flange, screw flange), SL 7.62 (plain, flanged, gap). Read from the dimensional-drawing rasters (`pdfimages`; for the headers the "Hole pattern" is in the same image).

Acceptance: the Weidmüller D lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → the Weidmüller lines absent from the missing list (est. 2 h). Result: `24 lines, 0 without a footprint`.
Commit: this commit (hash in the next one)

### Phase 4 — Würth Elektronik (6 variants), load and hand-check

- [x] As Phase 2, from `datasheets/connector-terminal/wurth/<item>.pdf`.
      Six entries from page 1 (Dimensions and Recommended PCB Layout / Hole Pattern): 0016 tab 5.00 V, 0018 7.50 V, 3114 7.62 V (a series sheet: the drawing prints the layout as (N-1) x 7.62 + 7.6), 311 5.08 V, 321 3.81 V, 3221 3.50 H.
- [x] `census/fp-load.sh planning/plan-45-data/footprints-D.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
      `152 footprints appended`, `152 new footprints: load ok`.
- [x] For each variant, check its smallest and largest position counts
      against the drawing.
      `census/fp-widths.py D --ends`: 145 widths (33 against a "widest" outline: the footprint is never narrower than a sheet) and 35 hole spans compared on every row: 0 mismatches; smallest and largest counts per variant listed in the run (e.g. WE 311: 2 pos 11.56, 12 pos 62.36; WDM SL-F1: 2 pos 19.96, 6 pos 40.28). `census/fp-groups.py D` and `fp-consistency.py D` found the housings that needed splitting and the one hole diameter that needed raising (Corrections).
- [x] `docs/parts-format.md` § Footprints: the `TB-` paragraph names the
      flange entries.
      The paragraph names `holes`, `stagger`, `edgeX`, `reverse`, `gaps` and `flange` (non-plated pegs, or plated unnumbered pads).

Acceptance: every D line has objects that load and match their drawings.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, then `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-D.json` → exits 0 (est. 2.5 h; the load builds a temp backend, 10 min). Result: `24 lines, 0 without a footprint`, `152 footprints, 0 missing`; `152 new footprints: load ok`.
Commit: this commit (hash in the next one)

## Validation Plan

- Tests: `scripts/gen-terminals.test.js`, `fp-load.sh`.
- Doc sync: `docs/parts-format.md` § Footprints.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **Pins are 1…n along the row, left to right**, whichever way a
  datasheet numbers them: the part's pinout numbers its pins the same way
  (B § 3 rule 4), so the footprint and the symbol agree.
- Author: **Flange holes only as non-plated pegs** when the drawing shows
  holes without a copper ring; a drawing with a plated flange pad is `skip`
  (it would need a pad that is neither a signal pin nor a hole).

## Corrections

- **2026-11-24, the D lines.** After B the D lines are 18 / 139 footprints / 197 parts (the plan's 142 / 201 were planning-time). D splits the Weidmüller variants by housing and skips 10 rows (below): **24 lines, 152 footprints, 187 parts** (`awk` over `footprints-needed.txt`: `24 D lines, 152 D footprints, 187 parts`). C's lines are unchanged (19 / 135).
- **2026-11-24, Weidmüller variants mix housings; each pad pattern is its own variant.** `census/fp-groups.py D` groups a variant's rows by (width − n x pitch, depth) from the sheets: Weidmüller's `SC`, `SL 3.50`, `SL 5.08` and `SL 7.62` mix unflanged headers with flanged ones (screw flange, solder flange, a 4 pin part on a 5 slot housing) under one DigiKey series. `census/split-variant.py` moved them to `Omnimate-SC-F` (4 rows), `-SL-F` at 3.50 (7) and 7.62 (3), `-SL-F1` (solder flange, 5) and `-SL-F2` (screw flange, 2) at 5.08, and `-SL-GAP` at 7.62 (1): 6 new lines. Within an unflanged family the housing outline differs by up to 1.5 mm (SL 5.08: n x 5.08 + 1.76, + 1.9, + 3.2; SC: + 0.69 and + 1.4) with the same holes: the variant draws the **widest** outline (`"widest": true` in the table; `fp-widths.py` accepts a footprint wider than a sheet, never narrower). The 4 position `GAP` item has pins in slots 1, 2, 3 and 5 (`gaps`), which `header` cannot draw.
- **2026-11-24, flanges: plated flange pins are unnumbered pads, not a skip.** § Decisions said a plated flange is `skip` ("a pad that is neither a signal pin nor a hole"). `docs/parts-format.md` § `explicit` has `"pin": 0`, an unnumbered pad with any number allowed and a drill, so Weidmüller's solder flange (`SL-F1`: the flange holes are the size of the pin holes, 5.0 beyond the end pins) is drawn as two plated `pin: 0` pads, and the screw flanges (Ø1.55, 2.3 or 2.85, drawn as a plain hole without a pad) as non-plated pegs. `flange` in the table writes both (`plated: true`).
- **2026-11-24, the generator.** C's `explicit` style and D's `flange` and `gaps` in one table; 9 tests (`ℹ tests 9`).
- **2026-11-24, 10 rows are `skip` (`no footprint`), all in `datasheets/skipped-parts.md` (`final.py --verify-skipped` → `12 skip rows, 0 missing`) and their PDFs deleted (`place.py --prune`: 308 files left).** Weidmüller `8000078338`, `8000078341`, `8000078340`, `8000078343` (MHS 7.50, 5 to 8 positions): mixed-pitch headers (the drawing's `L1 + L2 + 13.9`: two pitches in one housing). Phoenix `5452093`, `5452094`, `5452027`, `5452029` (BCH wave-soldering versions), `1966509` (GMSTBVA 2 pos) and `1941650` (ICV 12 pos): the sheets print the pitch and `Dimension a` only, no drilling diagram, no hole diameter. B's "every land pattern is in a placed PDF" was false for these (B's Corrections named four of them); the BCH, GMSTBVA and ICV items that do have a drawing keep their footprints (`5441278`, `5444262`, `1766699`, `1785955` …).
- **2026-11-24, drill.** SL 5.08's four items `1146890000`, `1149670000`, `1539220000`, `1147640000` print an eyelet of Ø1.5 +0.1, the rest Ø1.4 +0.1: the family draws 1.55 (a hole that is too big is harmless, one that is too small is not); SC prints 1.2 and 1.3 (+0.1): 1.35. Würth 0016 (a blade pin): the Ø1.5 of the hole pattern, drawn round.
- **2026-11-24, orientation and body.** As C's Corrections: the drawing's plan view, rows measured from the edge the sheet dimensions (±0.3 mm on the rasters).

## Summary

D is C's work again with shrouds and flanges. The risk is the same misread
dimension; the extra thing is the `explicit` entries, which have a test.
