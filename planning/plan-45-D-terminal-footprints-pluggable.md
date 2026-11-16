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
| plan-45-C complete | `ls planning/completed/plan-45-C-*.md` → exists | NOT MET (2026-11-16: not started) |
| The generator exists | `ls scripts/gen-terminals.js scripts/gen-terminals.test.js` → both exist | NOT MET (2026-11-16: C writes them) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the D lines (18 at planning time, before B's pick) | NOT MET (2026-11-16: 18 from the candidates) |

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

- [ ] `scripts/gen-terminals.js`: `style: explicit` with `flange` (§ 3).
- [ ] `scripts/gen-terminals.test.js`: a 5.08 mm flange entry for n = 2 and
      n = 5 gives the expected pads (n signal pads, two `pin: 0` non-plated
      holes at the computed x); an entry without `flange` is unchanged.

Acceptance: the explicit entries work.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min).
Commit: —

### Phase 2 — Phoenix Contact (6 variants)

- [ ] Table entries from `datasheets/connector-terminal/phoenix/<item>.pdf`
      (`Drilling diagram`, dimension drawing).
- [ ] `node scripts/gen-terminals.js --letter D --out planning/plan-45-data/footprints-D.json`.

Acceptance: the Phoenix D lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → the Phoenix lines absent from the missing list (est. 2 h).
Commit: —

### Phase 3 — Weidmüller (6 variants)

- [ ] As Phase 2, from `datasheets/connector-terminal/weidmuller/<item>.pdf`.

Acceptance: the Weidmüller D lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → the Weidmüller lines absent from the missing list (est. 2 h).
Commit: —

### Phase 4 — Würth Elektronik (6 variants), load and hand-check

- [ ] As Phase 2, from `datasheets/connector-terminal/wurth/<item>.pdf`.
- [ ] `census/fp-load.sh planning/plan-45-data/footprints-D.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
- [ ] For each variant, check its smallest and largest position counts
      against the drawing.
- [ ] `docs/parts-format.md` § Footprints: the `TB-` paragraph names the
      flange entries.

Acceptance: every D line has objects that load and match their drawings.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, then `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-D.json` → exits 0 (est. 2.5 h; the load builds a temp backend, 10 min).
Commit: —

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

## Summary

D is C's work again with shrouds and flanges. The risk is the same misread
dimension; the extra thing is the `explicit` entries, which have a test.
