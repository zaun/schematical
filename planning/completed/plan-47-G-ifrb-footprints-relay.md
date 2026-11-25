# plan-47-G: Footprints — relays

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-F

G draws the land patterns of the relays of the final pick: through-hole and
surface-mount electromechanical and reed relays, whose hole pattern belongs
to the series and its contact arrangement, and solid-state relays in DIP,
SOP and smaller packages. The behavioral outcome: every G line of
`footprints-needed.txt` has a footprint id in `footprint-map.tsv`, and the
new objects of `planning/plan-47-data/footprints-G.json` load through
`footprint_check`.

References:

- plan-47-D § 3.1 and § 3.3 (ids, reuse by `fp-compare.py`, shared ids, the
  map) and its tools; G's footprints are `explicit` or `dual` objects
  written by hand (no generator).
- plan-47-C § 3.2 (the relay pinouts: designators are the datasheet's
  terminal numbers; `specs.tsv` `pinout` and `pads`).
- `docs/parts-format.md` § Footprints (`explicit`, `dual` with `slots`;
  R1–R6, mid-values; a pin may have several pads).
- Library precedents: `SW-DIP-8` and the tactile-switch footprints (hole
  patterns from a PCB-layout drawing), `PDIP-8`, `SOIC-4`, `SOP-8-PSA`,
  `SSOP-8` (the candidates for a solid-state relay's reuse).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-F complete | `ls planning/completed/plan-47-F-*.md` → exists | NOT MET (2026-11-16: not started) |
| The relay pinouts are final | `python3 -I planning/plan-47-data/census/pinout-check.py relay` → `<m> specs rows: 0 errors` | NOT MET (2026-11-16: C not started) |
| No `RELAY-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith('RELAY-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/fp-check.py G` → `<n> lines, 0
  without a footprint` (267 lines at planning time, from the download set).
- `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-G.json`
  → `<k> new footprints: load ok`.
- Every relay `specs.tsv` row's `pads` equals its footprint's pad-designator
  count (`census/fp-pins.py G`, Phase 1).

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the
  app.
- No socket footprint (socketed and plug-in relays are out of scope,
  plan-47-A § 3).
- No edit of an existing footprint.

## 2. Current State

`footprints.json` has no relay footprint. It has DIP, SOP and SSOP
footprints of logic and optocoupler-sized packages (`PDIP-8`, `SOIC-4`,
`SOP-8-PSA`, `SSOP-8`, `SIP-6-2.54-VISHAY` …) that a solid-state relay of
the same land pattern can reuse.

### Measured populations

At planning time, from the download set (re-measure at G's start).

| What | Count | Command |
|---|---|---|
| G lines | 267 (738 parts): TE 122, Panasonic 113, Franjobaim 32 | `awk -F'\t' '!/^#/ && $1=="G"{print $3}' planning/plan-47-data/footprints-needed.txt \| sort \| uniq -c` |
| …distinct (maker, series) | 157 | `awk -F'\t' '!/^#/ && $1=="G"{print $3"\|"$4}' … \| sort -u \| wc -l` |
| …electromechanical and reed (no DigiKey package) | 248 lines (TE 118, Panasonic 98, Franjobaim 32) | `awk -F'\t' '!/^#/ && $1=="G" && $6==""' … \| wc -l` |
| …solid-state (a DigiKey package) | 19 lines, 54 parts (Panasonic 15, TE 4) | `$6!=""` |
| …by mounting | through hole 228 lines (550 parts), surface mount 39 (188) | `awk -F'\t' '!/^#/ && $1=="G"{print $5}' … \| sort \| uniq -c` |
| …with an existing footprint of the same package (the reuse hint) | 4 | `awk -F'\t' '!/^#/ && $1=="G" && $13!=""' … \| wc -l` |
| Datasheets behind the lines | 171 distinct PDFs for the type | plan-47-C § 2 |

### Verified properties

- **A line's key carries the series, the contact form, the coil type and
  the termination** (`footprint-lines.py`: `series` and `detail` for a
  relay), the four things a relay's hole pattern depends on besides the
  maker.
- **Pads are matched to pins by designator** (`docs/parts-format.md` § The
  `"footprint"` part key), so a relay whose datasheet numbers its terminals
  1, 2, 5, 6, 9, 10 gets a pinout and a footprint that agree only when both
  use the same numbering; plan-47-C § 3.2 fixes the pinout to the
  datasheet's terminal numbers and § 3.2 here renumbers when they are not
  1…n.

## 3. Design

### 3.1 Ids and sharing

`RELAY-<SERIES>-<n>-<MAKER>`: `<SERIES>` the series name reduced to `A-Z0-9`
(`RT1`, `IM`, `TQ2SA`, `FRA`), `<n>` the pad-designator count, MAKER `TE`,
`FRANJOBAIM`, `PANASONIC` (`RELAY-RT1-5-TE`, `RELAY-TQ-10-PANASONIC`). A
series with two patterns of the same pad count (a through-hole and a
surface-mount version, a sensitive-coil version with other pin spacing)
adds `-SMD`, `-THT` or `-B`. Lines of one series that differ only in coil
voltage, contact material or sealing share one footprint; the map row of
each names it. A solid-state relay whose land pattern equals a library
footprint reuses it (`fp-compare.py`); a new one is
`RELAY-<PACKAGE>-<n>-<MAKER>` (`RELAY-SOP-4-PANASONIC`).

### 3.2 Drawing a relay

- The source is the datasheet's **PC board pattern** (the recommended
  mounting holes or lands), not the outline; hole diameters are the
  pattern's (R1); a pattern that gives none takes R5 from the terminal's
  width (a flat terminal: its diagonal).
- **Views.** A pattern drawn as a bottom view (copper side) is mirrored left
  to right into the format's top view (x grows right, y grows down, pin 1 at
  the top left); a top view is used as drawn. The `note` says which view the
  drawing is and that it was mirrored.
- **Designators.** The format numbers pads `1…pins`. A relay whose terminals
  are numbered 1…n without gaps keeps its numbers. One whose datasheet skips
  numbers (DIP-style relays: 1, 2, 5, 6, 9, 10; relays with terminals 11,
  12, 14, 21, A1, A2) is renumbered 1…n in ascending order of the
  datasheet's terminal names (digits before letters), and the `renumber`
  column of `specs.tsv` records the mapping (`1=1,2=2,5=3,6=4,9=5,10=6`);
  plan-47-C's `pinout` is rewritten with the new designators by
  `census/fp-pins.py --apply`, and the device `note` (plan-47-L) carries the
  datasheet's own numbers.
- **Doubled terminals** (two pins on one contact) are two pads with two
  designators (the pinout gives both the same name); **mechanical pins** the
  pinout does not name (case tabs, stand-offs) are `"pin": 0` pads, plated
  when the drawing shows a plated hole.
- Style `explicit` for every electromechanical relay; `dual` for a
  solid-state relay in a DIP, SOP or SSOP whose lands are two regular rows.
- `body` is the relay's length × width (mid-values).

`census/fp-pins.py G` checks, per map row: the footprint's designators are
exactly `1…<pads>` of the `specs.tsv` rows that use it, and every pinout
designator has a pad.

Where the risk concentrates: mirrored views and renumbering. A footprint
that is the mirror image of the real one loads, looks right and puts the
coil on the contact pads. The checks: the `note` records the view; one part
per series is hand-checked by following pin 1 and the coil from the
datasheet's schematic through the pinout to the footprint's pads; and
`fp-pins.py`.

Rejected alternatives: (1) keeping the datasheet's sparse terminal numbers
as designators (rejected: `explicit` pads are numbered 1…`pins`, and
`"ball"` names are for ball pinouts); (2) drawing from the outline with the
pad rules (rejected: every relay datasheet in the set prints a board
pattern, and R1 prefers it).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The pin tool, and the solid-state relays

- [x] (2026-11-23: `fp-pins.py G` → `210 rows: 0 errors`, `--apply` wrote the `renumber` column and the new pinouts into specs.tsv; `pinout-check.py … relay` still `0 errors`) `census/fp-pins.py` (§ 3.2: the check, and `--apply` for the
      renumbering); the `renumber` column in `specs.tsv`.
- [x] (2026-11-23: slice G-panasonic (PhotoMOS): 12 new footprints, no reuse (PDIP-8 drill 0.75 against 0.8)) The solid-state lines (19 at planning time): reuse by `fp-compare.py`
      or a new `dual` object in `footprints-G.json`; map rows.

Acceptance: the solid-state lines have footprints that load and match their pinouts.
  Check: `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-G.json` → `<k> new footprints: load ok`; `python3 -I planning/plan-47-data/census/fp-pins.py G` → `<m> map rows with a footprint: 0 errors` (est. 4 min).
Commit: 95a174b4

### Phase 2 — Panasonic and Franjobaim relays

- [x] (2026-11-23: slices G-panasonic (86 new) and G-te-franjobaim (70 new)) The Panasonic (98 lines) and Franjobaim (32 lines) electromechanical
      relays (§ 3.2): `explicit` objects, map rows, `renumber` where needed.
- [x] (2026-11-23: by the slice agents (their reports list the doubtful series)) Hand-check one part per series (pin 1 and the coil, schematic →
      pinout → pads).

Acceptance: those lines have footprints that load and match their pinouts.
  Check: the Phase 1 commands → `load ok`, `0 errors`; `python3 -I planning/plan-47-data/census/fp-check.py G` → the lines without a footprint are TE's (est. 4 min).
Commit: 95a174b4

### Phase 3 — TE relays

- [x] (2026-11-23: slice G-te-franjobaim; `fp-pins.py --apply` run once for both slices) The TE lines (118 electromechanical and reed at planning time) as
      Phase 2; `fp-pins.py --apply`, then `pinout-check.py relay` again.

- [x] (2026-11-25: G 194 lines; one new: Panasonic ACTP112 (1 Form C slim), the same pattern as `RELAY-ACT-5-PANASONIC` (mech_eng_ctp.pdf p.6 against mech_eng_ct.pdf p.6), mapped to it; one gone (CQ)) After A's last refill and C's final pick: regenerate `footprints-needed.txt` (`footprint-lines.py … --add`), `fp-check.py G` for lines new to the final pick (draw them), `fp-load.sh` over `footprints-*.json`, re-record the counts.
Acceptance: every G line has a footprint that loads and matches its pinout.
  Check: `python3 -I planning/plan-47-data/census/fp-check.py G` → `<n> lines, 0 without a footprint`; `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-G.json` → `<k> new footprints: load ok`; `python3 -I planning/plan-47-data/census/fp-pins.py G` → `0 errors`; `python3 -I planning/plan-47-data/census/pinout-check.py relay` → `0 errors` (est. 5 min).
Commit: 6a053e7a

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`; `fp-pins.py`.
- Coverage check: `fp-check.py G`; `fp-pins.py G` over every map row.
- Runtime proof: n/a (L puts the ids on parts; the loader's `has N pads,
  pinout has M pins` rule is the last check).
- Doc sync: none.
- Final gate: plan-47-O.

## Decisions

- **The footprint is the datasheet's board pattern, in top view** (§ 3.2).
- **Sparse terminal numbers are renumbered 1…n** and the mapping is kept in
  `specs.tsv` and the device `note` (§ 3.2, rejected alternative 1).
- **One footprint per (series, pattern)**, shared by its coil voltages and
  contact materials (§ 3.1).
- **Solid-state relays reuse library package footprints on a geometry
  match.**

## Corrections

- **2026-11-25, the final pick's lines.** `census/footprint-lines.py … --add` → D 208, E 91, F 250, G 194, H 179 = 922 lines (the provisional pick had 923 lines; 2 lines new, 3 gone: the F and G ones above, a Mallory 12.5 mm buzzer line, a Panasonic CQ relay line, a Bel Fuse 10.2 mm PTC line). `fp-check.py D E F G H` → `922 lines, 0 without a footprint`; `fp-load.sh footprints-*.json` → load ok. `PTC-RAD-210-102-BELFUSE` was merged into `Parts/footprints.json` (`fp-merge.py --only`).
- **2026-11-23.** Done in the wave described in plan-47-D § Corrections, slices `fp-lines/*-G-*` / `G-*.txt`; `fp-check.py G` → `0 without a footprint`. The lines are the provisional pick's (planning/plan-47-data/footprints-needed.txt is the PROVISIONAL pick's (candidates-provisional.tsv: C's dry run); the final pick follows A's last quota day).

## Summary

G's risk is a mirrored or mis-numbered pattern, which loads cleanly and is
wrong; the view note, the per-series hand check and `fp-pins.py` bound it.
Untouched: every existing footprint, the part files, the backend and the
app.
