# plan-54-H: Footprints — memory / SIM card sockets and module sockets (and the row generator I, J and K use)

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-G

H draws the land patterns of the **memory / SIM card sockets** (87 footprint keys
at planning time, 62 variants) and the **module sockets** (30 keys, 21 variants)
that B left in the pick, and writes the row generator (`census/rowgen.py`, § 3.3)
that turns a few numbers read off a drawing into the hundreds of pads of a module
or card-edge footprint; I, J and K use it. The behavioral outcome:
`footprints-H.json` holds one footprint per footprint key of B's `add` rows (or
the id of an existing one), each loads (`footprint_check`), and the map says for
every `footprints-needed.txt` line of letter `H` which footprint serves it.

References:

- plan-54-A § 3 (rule 9), plan-54-B § 3.3 (pin names), `footprints-needed.txt`
  (letter `H` lines).
- `docs/parts-format.md` § Footprints (the styles, `explicit`, the pad rules
  R1 – R6 → `planning/completed/plan-03-B-logic-footprints.md` § 3); the USB
  footprints (`USB-…`: `explicit`, a note with every number used).
- `planning/completed/plan-44-C-crystal-footprints-smd.md`,
  `planning/completed/plan-44-D-crystal-footprints-leaded-resonators.md` and
  `planning/plan-44-data/census/` (`fp-skeleton.py`, `fp-check.py`, `fp-compare.py`,
  `fp-load.sh`, `fp-merge.py`: the tools H copies and adapts).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-G complete | `ls planning/completed/plan-54-G-*.md` → exists | MET (2026-11-23: F and G are committed on worktree-P-54 (814e4406, b80d3818); archived when the plan lands) |
| The format has the opposite-pad and edge-mount keys | `grep -c 'int opposite' backend/include/parts.h` → 1 | MET (2026-11-23: 1) |
| The pick of the two families is final | `awk -F'\t' '($4=="card"\|\|$4=="module") && $20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-23: 0) |
| `footprints-needed.txt` has the `H` lines | `awk -F'\t' '$1=="H"' planning/plan-54-data/footprints-needed.txt \| wc -l` → the variants of the pick (83 at planning time, before skips) | MET (2026-11-23: 144 H lines before the footprint skips, 131 after) |
| F0 is recorded | `wc -l < backend/tests/golden/footprints.txt` at the start; H uses its own | MET (2026-11-22 at 7b449718: 1895) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-54-data/footprints-H.json` (the format of `footprints.json`) holds
  one footprint per footprint key (variant, positions) of the `H` lines that no
  existing footprint serves, and `planning/plan-54-data/footprint-map-H.tsv` maps
  every `H` line to the id that serves it.
- `census/rowgen.py` and `census/rowgen-check.py` exist and I, J and K run them
  unchanged.
- `python3 -I planning/plan-54-data/census/fp-check.py H` prints `<n> lines, 0
  without a footprint`; `census/fp-load.sh planning/plan-54-data/footprints-H.json`
  prints `<k> new footprints: load ok`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app: the
  footprints enter the library in J.
- No footprint for a part B did not `add`; a line whose drawing the format cannot
  express (A § 3 rule 9) is not drawn (§ Phases, Phase 4).

## 2. Current State

`footprints.json` has 1895 ids and none for a card, module or card-edge socket;
the styles are `dual`, `quad`, `grid`, `header` and `explicit` (`pins` 1 – 1000;
`explicit` pads `{pin, x, y, width, height, shape}` plus `drill`, `slotLength`,
`plated`; `pin: 0` an unnumbered pad, `plated: false` a non-plated hole). The
USB connectors' footprints are the precedent for a connector's shell legs and
pegs. plan-44's `fp-*.py` tools draw footprints from pad tables read by parallel
readers.

### Measured populations

| What | Count | Command |
|---|---|---|
| Footprint keys (variant, positions): card, module | **87**, **30** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` (before B's skips; H re-measures from `footprints-needed.txt`) |
| Variants (lines): card, module | **62**, **21** | same |
| Module keys by positions | 72 to 694 (110 of the 122 module parts above 196) | `awk -F'\t' '$20=="candidate" && $4=="module"{print $8}' planning/plan-54-data/candidates.tsv \| sort -n \| uniq -c` |
| Keys serving one part | 286 of the 525 keys of the three families | `python3 -I -c` over `candidates.tsv` |
| Footprint format limit | `pins` at most 1000 (`MAX_PINS 1000u`) | `grep -n 'MAX_PINS' backend/src/library/footprints.c` → line 17 |

### Verified properties

- A footprint's `pins` is the number of designated pads; `explicit` pads may share a
  pin (a shell tab on several pads) and `"pin": 0` pads are unnumbered, so a
  socket's shell tabs and pegs need no pin of the pinout other than `SHIELD`
  (`docs/parts-format.md` § `explicit`).
- `footprint_check` loads every footprint of `footprints.json` and compares its
  pad list with the golden `footprints.txt` (`docs/backend.md` § Tests); H's check
  is `fp-load.sh`, which builds `footprint_check` in a copy and loads the new
  file (plan-44-C Phase 2 ran it the same way).

## 3. Design

### 3.1 Ids, reuse

Id: `<stem>-<N>`, `<stem>` the line's last column of `footprints-needed.txt`
(`SOCK-<MAKER>-<series slug>` for a card socket, `DIMM-<MAKER>-<series slug>` for
a module socket; `<MAKER>` is `SAMESKY`, `AMPHENOL`, `GCT`, `TE` or `MOLEX`;
the slug is the series (the B `label` when DigiKey gives none) in upper case,
every run of characters outside `A-Z0-9` one `-`), `<N>` the key's positions (not
the pad count). Two drawings of the same id that differ get `-B`, `-C` after `<N>`.

Reuse: `census/fp-compare.py <footprint id> <pad table>` prints the difference
between an existing footprint's pads and a drawing's (centre x, y, width, height,
shape per pad number, tolerance 0.02 mm); two keys whose pads are equal within
0.02 mm share the id of the first one drawn (map note `shared with <id>`).

### 3.2 Card sockets

`explicit`, pads from the datasheet's recommended land pattern (R1; the outline
rules R2 – R6 only when the print has none), origin the body's centre, the card
entry toward −y as the USB connectors' mating face. Contact pads are pins 1 … k in
the pinout's order, the shell tabs are pads of the `SHIELD` pin, the detent and
locating pegs are `"pin": 0` holes (`plated: false` when the drawing says
non-plated). The `note` names the drawing, the PDF and the page and gives the
arithmetic of any number not printed.

### 3.3 Module sockets and the row generator

A module socket's footprint is two rows of contacts (front / back of the card) with
a key gap and pegs: regular, so it is generated. `census/rowgen.py
rowgen-params.tsv > footprints-<L>.json` writes an `explicit` footprint per row of
the parameter table, one row per footprint key (columns, all mm):

| Column | Meaning |
|---|---|
| `id`, `n`, `description`, `note` | the footprint id (§ 3.1), its pins N, the text |
| `numbering` | `halves` (pins 1 … N/2 on side 0, the rest on side 1), `odd-even` (odd pins on side 0, even on side 1), `single` (all on side 0) |
| `pitch` | pad pitch along a side |
| `dir` | `ltr` or `rtl`: pin 1 of a side at the left or right end |
| `row_y` | the y of side 0, then of side 1 (`-2.1,2.1`); one number for `single` |
| `row_layer` | `same` or `opposite` for side 0, then side 1 (`same,opposite`): an `opposite` side's pads get `"opposite": true` (plan-54-E); default `same,same` |
| `edge_mount` | `1` writes `"edgeMount": true` on the footprint (a straddle-mount housing); default 0 |
| `pad_w`, `pad_h`, `shape`, `drill` | pad size (x by y), `rect` / `round`, 0 for surface mount |
| `stagger` | y offset added to every second pad of a side (staggered through-hole rows), 0 for none |
| `gaps` | `<after pad>:<mm>,…`: extra distance after that pad of a side (a key notch), counted per side |
| `extras` | a JSON list of `explicit` pads (pegs, tabs, holes) in the body's coordinates |
| `body_w`, `body_h` | the housing outline |

Pad `i` (0-based) of a side with `m` pads is at x = (i − (m − 1) / 2) × pitch +
the gaps before it, centred so the side's pads span symmetrically about x = 0 (the
gap widths re-centre the row); `rtl` mirrors it. `rowgen-check.py rowgen-ref.tsv`
regenerates each footprint and compares the drawing's printed dimensions (the
reader records, per variant and for at least two position counts, the
first-to-last pad centre distance and each peg's x, in `rowgen-ref.tsv`:
`id`, `what`, `value`) with the generated ones, tolerance 0.02 mm. **Risk
concentrates here**: a wrong pitch or gap rule yields hundreds of wrong pads from
one row; the reference dimensions for two or more counts per variant are the
gate.

Rejected alternative: drawing the 30 module footprints pad by pad (rejected: 200 –
694 pads each; and the same generator serves I, J and K's 408 card-edge keys).
Rejected: the `dual` style (its pads are two rows facing each other with the slot
numbering of an IC; a module's key gap and mixed pegs need `explicit`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools and the map skeleton

- [x] Copy `planning/plan-44-data/census/fp-skeleton.py`, `fp-check.py`,
      `fp-compare.py`, `fp-load.sh`, `fp-merge.py` to `planning/plan-54-data/census/`
      with `p44` → `p54`; adapt `fp-check.py` to the columns of this plan's
      `footprints-needed.txt` (letter, family, maker, series, mounting, card,
      pitch, positions, parts, example, datasheet, existing, id stem): the map's
      key is (letter, variant), known ids from `footprints-H.json` … `footprints-K.json`
      and `footprints.json`.
- [x] Write `census/rowgen.py`, `census/rowgen-check.py` (§ 3.3) and a tiny test
      fixture (`census/rowgen-test.tsv`: a 2 × 4-pad row with a gap) whose output
      is compared by `cmp` with a hand-written `rowgen-test.json`.
- [x] `footprint-map-H.tsv` skeleton: one row per `H` line, the footprint column
      empty.

      Result: tools written for this data layout (fresh, not copies: `fp-check.py`, `fp-brief.py`, `fp-assemble.py`, `fp-skip.py`, `rowgen.py`,
      `rowgen-check.py`; `fp-merge.py` and `fp-load.sh` copied from plan-44 with the opposite-pad-aware overlap test); `rowgen.py rowgen-test-params.json | cmp - rowgen-test.json` → `ok`
      (the fixture is JSON, § Corrections); `fp-check.py H` → `147 footprint keys, 147 without a footprint` on the empty map.

Acceptance: the tools run.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py H` → `<n> lines, <n> without a footprint`; `python3 -I planning/plan-54-data/census/rowgen.py planning/plan-54-data/census/rowgen-test.tsv \| cmp - planning/plan-54-data/census/rowgen-test.json && echo ok` → `ok` (est. 30 min).
Commit: ae339fd3

### Phase 2 — Card sockets

- [x] Draw every card-socket line (the `H` lines of family `card`; 62 at planning
      time) into `footprints-H.json` from the PDFs of
      `datasheets/connector-card-socket/`, six parallel readers on a share of the
      lines each (pad tables in `fp-parts/<reader>.tsv`, assembled by
      `fp-skeleton.py --assemble`), or map a line to an existing footprint or an
      earlier one by § 3.1; fill the map rows. A line whose drawing needs a bottom
      pad, a rotated slot or a keyhole slot is not drawn: its parts become `skip`
      (`footprint not expressible: <feature>`) in `candidates.tsv` and
      `skipped-parts.md`, and the number goes in § Corrections.
- [x] Hand-check one footprint per card type (SD, microSD, SIM, Micro SIM, Nano SIM,
      Smart card) against the drawing's dimensions; the arithmetic is in the `note`.

      Result: six readers drew the card-socket footprints pad by pad (`fp-parts/card-1.json` … `card-6.json`, 88 lines); `fp-assemble.py H` → 116 footprints (card and module);
      lines with no land pattern in the PDF are not drawn: 20 rows `skip` (`no land pattern in the datasheet`) plus SD7-5-A / SD7-7-A (no pad for pin 17); each reader's
      `fp-load.sh` → `load ok`; the Same Sky MSD-1-A footprint was re-derived by hand from its print's numbers (9 pads 0.70 x 1.60 at 1.10 pitch, § note).

Acceptance: the card-socket lines have footprints that load.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py H` → the card lines have a footprint; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-H.json` → `<k> new footprints: load ok` (est. 3 min: it builds `footprint_check` in a copy).
Commit: ae339fd3

### Phase 3 — Module sockets

- [x] Fill `rowgen-params.tsv` and `rowgen-ref.tsv` for every module line (21 at
      planning time, 30 keys) from `datasheets/connector-memory-module/`: pitch, row
      y, pad size, key gap, pegs, body; at least two position counts per variant in
      `rowgen-ref.tsv` (a variant with one position count has the drawing's pad-to-pad
      and peg dimensions); run `rowgen-check.py` and `rowgen.py` into
      `footprints-H.json`.
- [x] Hand-check one footprint per style (DIMM, SODIMM, SIMM, MiniDIMM, SOCAMM) that is
      in the pick against the drawing.

      Result: two readers filled the module generator rows (`module-1-params.json`, `module-2-params.json` and their refs): `rowgen-check.py` → `257 dimensions, 0 differ` and
      `241 dimensions, 0 differ`; TE 2309413's footprint checked by hand against the drawing (130 pads a row at 0.50 pitch, rows ±4.10, 73.0 x 9.2 body, 260 pins);
      `fp-check.py H` → `133 footprint keys, 0 without a footprint` after the skips; `fp-load.sh planning/plan-54-data/footprints-H.json` → `116 new footprints: load ok`.

Acceptance: the module lines have footprints that load, and the reference dimensions hold.
  Check: `python3 -I planning/plan-54-data/census/rowgen-check.py planning/plan-54-data/census/rowgen-ref.tsv` → `<n> dimensions, 0 differ`; `python3 -I planning/plan-54-data/census/fp-check.py H` → `<n> lines, 0 without a footprint`; `planning/plan-54-data/census/fp-load.sh planning/plan-54-data/footprints-H.json` → `<k> new footprints: load ok` (est. 5 min).
Commit: ae339fd3

### Phase 4 — Hand checks and the lines that cannot be drawn

- [x] Re-run Phase 2's rule for any module line whose drawing needs a feature the
      format lacks (A § 3 rule 9); update § Corrections with the totals.
- [x] Record `python3 -I -c "import json;d=json.load(open('planning/plan-54-data/footprints-H.json'))['footprints'];print(len(d), max(len(f.get('pads', [])) for f in d))"`
      (footprints, largest explicit pad count) in § Corrections.

      Result: no line needed `footprint not expressible` for a slot; 22 rows are `skip` for want of a land pattern (Amphenol ICC brochures, Molex 151080, TE 2513069 and the
      catalog 6-1773454-7, the two Same Sky SD7 parts); `python3 -I -c …` → `116 653` (116 footprints, the largest has 653 pads).

Acceptance: every `H` line has a footprint.
  Check: `python3 -I planning/plan-54-data/census/fp-check.py H` → `<n> lines, 0 without a footprint` (est. 1 min).
Commit: ae339fd3

## Validation Plan

- Tests: `fp-load.sh` (the loader accepts every footprint), `rowgen-check.py` (the
  drawings' dimensions), the hand checks of Phases 2 and 3.
- Coverage check: every footprint of `footprints-H.json` is the id of a line of the
  map (`fp-check.py H` reports unused ids as an error).
- Runtime proof: M's `footprint_check` run.
- Doc sync: none yet; `docs/parts-format.md` § `connector` is L's.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **The footprints are `explicit`, generated from a parameter row for
  module and card-edge sockets** (§ 3.3), and drawn pad by pad for card sockets
  (7 – 23 pads).
- Author: **Ids end in the key's positions**, not the pad count, so a part's
  `footprint` is derivable from its row (`<stem>-<positions>`).

## Corrections

- **The generator parameters are JSON, per variant, with expressions** (`rowgen-test-params.json`, `rowgen-test.json`), not the plan's TSV per footprint key: one row serves every
  position count of a drawing and an expression of `N` / `L0` gives pegs, body and gaps (`rowgen.py`'s docstring). Added keys beyond the plan: `pins`, `ids`, `x_off`, per-side
  `stagger`; the readers hand-built the footprints it cannot make (a CAMM2 matrix, irregular pad orders) into their `fp-parts/<name>.json`.
- **A land pattern is often not in a catalog PDF**: Amphenol ICC's `ssio_*` brochures and Molex's 151080 brochure print no PCB layout; those lines are not drawn and their
  parts are `skip` (`footprint not drawn: no land pattern in the datasheet`, `fp-notdrawn.tsv`, `fp-skip.py`); a line is mapped to another footprint only where its print or a
  sibling print shows the same pattern (the map notes say what was compared; three Amphenol / TE maps rest on a catalog's "footprint compatible" statement).
- **Dimensions a print does not give** are assumptions recorded in the footprint `note`s: hole plating (drawn non-plated), a plated-through pad's land size (drill + 0.5 to 0.6),
  a housing outline's position against the pads (centred), the card-entry side where unmarked. They are listed in each reader's report; none changes a pad's centre.
- **`trace_check.c` had a 256-pin buffer** (`TraceRole roles[256]`); the module sockets go to 694 pins, so it is 1024 (the footprint limit is 1000).

## Summary

H is the first footprint letter and the one that writes the generator. Its risk is
the row generator: one wrong parameter row is a whole wrong footprint, which the
reference dimensions of two position counts per variant gate.
