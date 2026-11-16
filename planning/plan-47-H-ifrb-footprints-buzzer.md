# plan-47-H: Footprints — buzzers

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-G

H draws the land patterns of the buzzers of the final pick: through-hole
buzzers (a round or square can on two pins, some with a third pin) and
surface-mount buzzers (two to four pads under a square body). The behavioral
outcome: every H line of `footprints-needed.txt` has a footprint id in
`footprint-map.tsv`, and the new objects of
`planning/plan-47-data/footprints-H.json` (and `-H-explicit.json`) load
through `footprint_check`. With H the footprint work of plan-47 is complete.

References:

- plan-47-D § 3 (ids, reuse, the generator and `dims-X.tsv`, the map) and
  its tools; plan-47-G § 3.2 (`fp-pins.py`).
- plan-47-C § 3.3 (buzzer pinouts: `1=+,2=-`, a third terminal `NC`).
- `docs/parts-format.md` § The `"footprint"` part key (R1; R8 radial leads:
  "A polarized part's pin 1 is `+` and has `pin1Shape` `rect`").
- Library precedent: `CAP-RADIAL-2.54` and the electrolytic can footprints
  of plan-33 (a round body on two polarized pins).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-G complete | `ls planning/completed/plan-47-G-*.md` → exists | NOT MET (2026-11-16: not started) |
| The buzzer pinouts are final | `python3 -I planning/plan-47-data/census/pinout-check.py buzzer` → `<m> specs rows: 0 errors` | NOT MET (2026-11-16: C not started) |
| No `BUZZER-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith('BUZZER-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/fp-check.py H` → `<n> lines, 0
  without a footprint` (190 lines at planning time, from the download set).
- `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-H.json planning/plan-47-data/footprints-H-explicit.json`
  → `<k> new footprints: load ok`.
- `python3 -I planning/plan-47-data/census/fp-pins.py H` → `0 errors`.
- `python3 -I planning/plan-47-data/census/fp-check.py D E F G H` → `<N>
  lines, 0 without a footprint` (every footprint line of the plan).

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the
  app.
- No sound-port, keep-out or wash-label data: `body` only.

## 2. Current State

`footprints.json` has no buzzer footprint. Each buzzer has its own
datasheet (Same Sky, PUI Audio and Mallory publish one PDF per part), with a
dimension drawing and, for surface-mount parts, a recommended land pattern.

### Measured populations

At planning time, from the download set (re-measure at H's start).

| What | Count | Command |
|---|---|---|
| H lines | 190 (571 parts): Same Sky 68, PUI Audio 70, Mallory 52 | `awk -F'\t' '!/^#/ && $1=="H"{print $3}' planning/plan-47-data/footprints-needed.txt \| sort \| uniq -c` |
| …through hole (PC pins) | 111 lines, 365 parts (Same Sky 44, PUI 40, Mallory 27) | `awk -F'\t' '!/^#/ && $1=="H" && $5=="Through Hole"' … \| wc -l` |
| …surface mount (solder pads) | 79 lines, 206 parts (PUI 30, Mallory 25, Same Sky 24) | `$5=="Surface Mount"` |
| Datasheets behind the lines | 569 distinct PDFs, one per part | plan-47-C § 2 |
| The line key | (maker, mounting, DigiKey Size / Dimension, termination): parts of one line share a body size, not necessarily a pin spacing | `footprint-lines.py` |

### Verified properties

- **R8 fits a round through-hole buzzer**: `dual`, `pins` 2, `span` the pin
  spacing, body `{"width": D, "height": D, "round": true}`, pin 1 `+` with
  `pin1Shape` `rect` (`docs/parts-format.md` R8).
- **A line's parts can differ in pin spacing**: the key is DigiKey's body
  size; the map takes a `series` (plan-47-D § 3.3) and, for H, a part number
  in the `series` column when two parts of one line and series still differ.

## 3. Design

Ids (plan-47-D § 3.1): through hole `BUZZER-RAD-<D>-<P>-<MAKER>` (body
diameter or side, pin spacing, mm × 10: `BUZZER-RAD-120-65-PUI`); surface
mount `BUZZER-<LLWW>-<pads>-<MAKER>` (`BUZZER-8585-2-SAMESKY`). MAKER
`SAMESKY`, `PUI`, `MALLORY`. Equal drawings share an id across parts,
series and makers (`fp-compare.py`): body sizes recur (12.00 mm round is
the listing's most frequent, 81 of 652 rows), so the distinct footprints
are fewer than the lines; the count is recorded in § Corrections.

Through hole: one `dims-H.tsv` row per distinct (body, pin spacing, pin
diameter), `family` `radial`, with a `polarized` column the generator turns
into `pin1Shape` `rect` on pad 1 (`+`); `fp-gen.py` gains that column in
Phase 1. A square body is `round` 0. A buzzer with a third pin (a feedback
electrode, a mounting pin) is an `explicit` object in
`footprints-H-explicit.json` with pads `1`, `2`, `3` as plan-47-C's pinout
numbers them.

Surface mount: the datasheet's recommended land pattern (R1); two pads →
`dims-H.tsv` (`family` `chip`); three or four pads → `explicit`, the pads
numbered as the pinout (the `NC` pads numbered, pads the pinout does not
name `"pin": 0`).

Pin 1 is `+`. Where a datasheet marks `+` on the outline only, the pad
under the mark is pad 1; the `note` quotes the page.

Where the risk concentrates: polarity (pad 1 on the wrong pin makes an
indicator silent on a real board) and the per-part pin spacing inside one
line. Every `dims-H.tsv` row carries the part numbers it was read for
(`parts` column) and `fp-pins.py H` checks pad counts; the hand check is
five parts per maker and mounting, polarity first.

Rejected alternative: one footprint per body size (rejected: DigiKey's size
text carries no pin spacing, and the footprint is the pins).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Through-hole buzzers

- [ ] `fp-gen.py`: the `polarized` column (pad 1 `pin1Shape` `rect`).
- [ ] The through-hole lines (111 at planning time): `dims-H.tsv` rows,
      `explicit` objects for three-pin parts, map rows (split by series or
      part number where the spacing differs).
- [ ] Hand-check five parts per maker, polarity first.

Acceptance: those lines have footprints that load and match their pinouts.
  Check: `python3 -I planning/plan-47-data/census/fp-gen.py planning/plan-47-data/dims-H.tsv > planning/plan-47-data/footprints-H.json && planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-H.json planning/plan-47-data/footprints-H-explicit.json` → `<k> new footprints: load ok`; `python3 -I planning/plan-47-data/census/fp-pins.py H` → `0 errors` (est. 4 min).
Commit: —

### Phase 2 — Surface-mount buzzers

- [ ] The surface-mount lines (79 at planning time) as Phase 1 (§ 3).
- [ ] Record the number of distinct buzzer footprints in § Corrections.

Acceptance: every H line, and every line of the plan, has a footprint that loads.
  Check: `python3 -I planning/plan-47-data/census/fp-check.py H` → `<n> lines, 0 without a footprint`; `python3 -I planning/plan-47-data/census/fp-check.py D E F G H` → `<N> lines, 0 without a footprint`; `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-[D-H]*.json` → `<K> new footprints: load ok` (all of plan-47's objects in one copy: no id clash, no overlap); `python3 -I planning/plan-47-data/census/fp-pins.py H` → `0 errors` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`, once for H and once for
  all five letters' objects together.
- Coverage check: `fp-check.py D E F G H`.
- Runtime proof: n/a (L puts the ids on parts).
- Doc sync: none.
- Final gate: plan-47-O.

## Decisions

- **Pin 1 is `+`**, marked with a square pad on through-hole footprints
  (R8's polarized rule).
- **A buzzer's footprint is its own pattern**; equal patterns share an id
  across parts and makers.
- **A line is split by series or part number in the map** when its parts'
  pin spacings differ.

## Corrections

## Summary

H's risk is polarity and per-part pin spacing across 569 one-part
datasheets; the `parts` column, `fp-pins.py` and the polarity-first hand
check bound it. After H every footprint line of plan-47 has an id.
Untouched: every existing footprint, the part files, the backend and the
app.
