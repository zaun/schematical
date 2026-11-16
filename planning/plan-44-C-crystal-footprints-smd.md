# plan-44-C: Footprints — surface-mount crystals

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-44-B

C draws the land patterns of the surface-mount crystal packages of the final
pick — two-pad (1.2×1.0 … 8.0×4.5 mm), four-pad (1.0×0.8 … 12.5×4.6 mm),
the three-pad and flat-lead variants, the HC-49/US surface-mount family, the
4-pin SOJ crystals and the surface-mount cylinder — from the datasheets'
package drawings, reusing a library footprint when its pads equal the
drawing's. It writes `planning/plan-44-data/footprints-C.json` and the C rows
of `footprint-map.tsv`. The behavioral outcome: every C line of
`footprints-needed.txt` has a footprint id in `footprint-map.tsv`, and the new
objects load through `footprint_check`.

References:

- `docs/parts-format.md` § Footprints (`dual`, `explicit`, `slots`; the rules
  R1–R9, mid-values, no overlapping pads); `docs/backend.md` § Tests
  (`footprint_check`, `footprints.txt` golden).
- `planning/completed/plan-40-C-transistor-footprints-through-hole.md` § 3.1 (ids,
  reuse, shared ids, ready objects) and its tools
  (`planning/plan-40-data/census/fp-load.sh`, `fp-merge.py`, `fp-check.py`).
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (the pad rules R1–R6).
- Library precedents: the 4-pad oscillator footprints `OSC-3215-4`,
  `OSC-2012-4`, `OSC-1610-4`, `OSC-5032-4`, `OSC-ECS-3225-4-MV`,
  `OSC-ECS-2520-4-MV`, `OSC-ECS-2016-4-MV`, `OSC-ABRACON-3225-4-ASE` (same
  outline, active parts: their pads are compared, § 3.1).

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-B complete | `ls planning/completed/plan-44-B-*.md` → exists | NOT MET (2026-11-16: not started) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-44-data/footprints-needed.txt \| wc -l` → the C lines (70 at planning time, before B's pick) | NOT MET (2026-11-16: 70 from the 744-row download set) |
| The plan-40 tools exist | `ls planning/plan-40-data/census/fp-load.sh planning/plan-40-data/census/fp-merge.py planning/plan-40-data/census/fp-check.py` → all three exist | MET (2026-11-16) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-44-data/census/fp-check.py C` prints `<n> lines, 0
  without a footprint`, `<n>` = `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-44-data/footprints-needed.txt | wc -l`
  (70 at planning time).
- `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json`
  prints `<k> new footprints: load ok` (at most 70; 30 C lines have no
  existing footprint of the same body size code, `awk -F'\t' '!/^#/ && $1=="C" && $11==""' …
  | wc -l` → 30, the others are compared in Phase 1).
- `footprint-map.tsv` has, for every C line, the footprint id and the pinouts
  it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the app.
- No edit of an existing footprint.
- No footprint for a package no `add` row uses.

## 2. Current State

The library has no crystal footprint. It has surface-mount oscillator
footprints of the same body sizes (`OSC-3215-4`, `OSC-2012-4`, `OSC-1610-4`,
`OSC-5032-4`, the ECS and Abracon oscillator ones); a crystal pad pattern
equals one of them only when its pads sit where the crystal datasheet's
recommended land pattern puts them (pad numbering included: pad 1 is the
crystal's first terminal).

### Measured populations

| What | Count | Command |
|---|---|---|
| C lines (maker, DigiKey Package / Case, supplier package, size), download set | 70 (620 parts) | `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-44-data/footprints-needed.txt \| wc -l` |
| …by package | 4-SMD No Lead 29, 2-SMD No Lead 21, HC-49/US 6, 4-SOJ 5.50 mm pitch 3, HC-49/US 3 Lead 2, Cylindrical Can Radial 2, 4-SMD Flat Lead 3 Leads 2, HC-49/US 4 Lead 1, 4-SOJ 9.40 mm pitch 1, 3-SMD No Lead 1, 2-SMD Flat Lead 1, `-` 1 | `awk -F'\t' '!/^#/ && $1=="C"{print $4}' … \| sort \| uniq -c \| sort -rn` |
| …by maker | Abracon, ECS, Suntsu | `awk -F'\t' '!/^#/ && $1=="C"{print $2}' … \| sort \| uniq -c` |
| …with no existing footprint of the same body size code | 30 | `awk -F'\t' '!/^#/ && $1=="C" && $11==""' … \| wc -l` |
| Phase split (by DigiKey package) | Phase 2: 2-SMD, 3-SMD 23 lines; Phase 3: 4-SMD 31 lines; Phase 4: HC-49/US, SOJ, cylinder, `-` 16 lines | the `by package` counts above: 21 + 1 + 1; 29 + 2; 6 + 2 + 1 + 3 + 1 + 2 + 1 |

### Verified properties

- **`footprint_check` loads any object of `footprints.json`'s format.**
  `fp-load.sh` appends the objects to a copy of `footprints.json`, builds
  `footprint_check` and expects only `extra footprint` / `unused footprint`
  lines for the new ids (`planning/plan-40-data/census/fp-load.sh` header).
- **Every footprint numbers its pads from the top of the left side.** A `dual`
  footprint's pad 1 is the top of the left column and the slots run
  counter-clockwise, and an `explicit` one places each pad by its `pin` and
  coordinates (`docs/parts-format.md` § Footprints, Units and axes, `dual`).
  A crystal's pad 1 is whatever corner the datasheet marks, so the pad table
  of a drawing is rotated into that frame before it is compared or written.

## 3. Design

### 3.1 Ids, reuse, ready objects

Id: `<label>-<MAKER>`, `<label>` the line's last column of
`footprints-needed.txt` (`footprint-lines.py` `label()`): `XTAL-` + the body
in mm × 10, long side first (`XTAL-3225-4` for 3.2 × 2.5 mm and 4 pads; a
side of 10 mm or more is spelled `118X55`) + the pad count; `XTAL-HC49US-2`,
`XTAL-CYL-2X6-2` (diameter × length in mm), `XTAL-SOJ-5.5-4` for the other
families; MAKER `ABRACON`, `ECS` or `SUNTSU` in capitals. The label comes from
DigiKey's size and package text, not from the datasheet's drawing, so it is
stable before the drawing is read; the `label` column of `footprint-map.tsv`
is the id without its maker suffix. Two drawings of the same label and maker
that differ get `-B`, `-C` after the maker.

Reuse: `census/fp-compare.py <footprint id> <pad table>` (Phase 1) prints
the difference between an existing footprint's pads and a drawing's (centre
x, y, width, height, shape per pad number, tolerance 0.02 mm); an existing
footprint serves a line when there is no difference (the id is then the
`footprint` of the map row, `note` `reuse`). A new footprint is one object
of `footprints.json`'s format in `footprints-C.json`.

Shared ids: lines of different makers whose drawings are equal (same body,
same pads within 0.02 mm) share the id of the first one drawn; the map row
says `shared with <id>`.

### 3.2 Styles and pads

A two-pad part is `dual` with `pins` 2 and `span` the pad-centre distance
(pad 1 left, pad 2 right); a four-pad part is `dual` with `pins` 4 (pads 1, 2
down the left side, 3, 4 up the right side), or `explicit` when the pads are
not one regular rectangle (HC-49/US surface-mount tabs, SOJ legs, mixed pad
sizes). Pads are the datasheet's recommended land pattern (rule R1); when the
datasheet prints none, the outline rules R2–R6, mid-values throughout; the
`note` names the drawing, the PDF under `datasheets/crystal/` and the page.

Risk: the 1.0 × 0.8 to 2.0 × 1.6 mm bodies (a 0.05 mm error is a different
pad) and the SOJ / HC-49/US tab families. The hand check of Phase 2–4 covers
one object per family against the drawing's dimensions, with the arithmetic
in the `note`.

Rejected alternative: one footprint per body code shared by every maker
(rejected: the library keeps one per maker drawing, and the makers'
recommended land patterns differ by up to 0.1 mm; reuse by geometry already
merges the equal ones).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools and the map skeleton

- [ ] Copy `planning/plan-40-data/census/fp-load.sh`, `fp-merge.py` to
      `planning/plan-44-data/census/` with `p40` → `p44`; adapt
      `fp-check.py` to the columns of this plan's `footprints-needed.txt`
      (letter, maker, mounting, package, supplier package, size, classes,
      parts, example, datasheet, existing, label): the map's key is (letter, maker,
      package, supplier, size), `*` matching any supplier or size; known ids
      from `footprints-C.json`, `footprints-D.json` and `footprints.json`.
- [ ] Write `census/fp-compare.py <id> <pad table>` (§ 3.1).
- [ ] `footprint-map.tsv` skeleton: one row per C line, the footprint column
      empty.

Acceptance: the tools run.
  Check: `python3 -I planning/plan-44-data/census/fp-check.py C` → `<n> lines, <n> without a footprint` (est. 1 min).
Commit: —

### Phase 2 — Two- and three-pad packages

- [ ] Draw the 2-SMD and 3-SMD lines (23 at planning time) into
      `footprints-C.json` from the PDFs of `datasheets/crystal/`, or map them
      to an existing footprint by § 3.1; fill the map rows.
- [ ] Hand-check one footprint per body code against the drawing.

Acceptance: those lines have footprints that load.
  Check: `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json` → `<k> new footprints: load ok` (est. 3 min: it builds `footprint_check` in a copy).
Commit: —

### Phase 3 — Four-pad packages

- [ ] Draw the 4-SMD lines (31 at planning time; two are flat-lead three-lead
      parts) as Phase 2.
- [ ] Hand-check one footprint per body code; the ones compared with
      `OSC-…-4` footprints print their `fp-compare.py` difference in the map
      `note` (`reuse` or the largest difference in mm).

Acceptance: those lines have footprints that load.
  Check: `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json` → `<k> new footprints: load ok` (est. 3 min).
Commit: —

### Phase 4 — HC-49/US, SOJ, cylinder and the rest

- [ ] Draw the HC-49/US (surface-mount tabs), HC-49/US 3- and 4-lead, 4-SOJ
      (5.50 and 9.40 mm pitch), cylindrical-can and `-` lines (16 at
      planning time) as Phase 2, `explicit` style.
- [ ] Hand-check one footprint per family.

Acceptance: every C line has a footprint that loads.
  Check: `python3 -I planning/plan-44-data/census/fp-check.py C` → `<n> lines, 0 without a footprint`, and `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json` → `<k> new footprints: load ok` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh` (pads load, no overlap, the new
  ids are the only extras).
- Coverage check: `fp-check.py C` joins every C line with a map row.
- Runtime proof: n/a (F puts the ids on parts and regenerates
  `backend/tests/golden/footprints.txt`).
- Doc sync: none (`docs/parts-format.md` § Footprints already covers the
  styles used).
- Final gate: plan-44-H.

## Decisions

- **Ids carry the maker** (`-ABRACON`, `-ECS`, `-SUNTSU`), as the library's
  oscillator and diode footprints do; equal drawings of two makers share one
  id (§ 3.1).
- **Oscillator footprints are reused only on a geometry match** (§ 3.1), never
  by body size alone: an active oscillator's pad 3 is its output and its pad
  numbering may start at another corner than the crystal's.
- **Pad 1 is a crystal terminal**: whatever the datasheet's drawing marks as
  pad 1; the four-pad parts' pads 2 and 4 are the case pads.

## Corrections

## Summary

C's risk is the smallest packages (a 0.05 mm error is a different pad) and
the HC-49/US tab and SOJ families (explicit pads). The geometry compare and
the per-family hand check bound it. Untouched: every existing footprint, the
backend and the app.
