# plan-44-D: Footprints — through-hole crystals and ceramic resonators

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-44-C

D draws the land patterns of the through-hole crystal packages (HC-49/U,
HC-49/US, HC-46/X, radial cylinder) and of every ceramic resonator (two-pin
and three-pin surface-mount packages and the 1512 (3731 metric) tab package)
of the final pick, from the datasheets' package
drawings, reusing a library footprint when its pads equal the drawing's. It
writes `planning/plan-44-data/footprints-D.json` and the D rows of
`footprint-map.tsv`. The behavioral outcome: every D line of
`footprints-needed.txt` has a footprint id in `footprint-map.tsv`, and the
new objects load through `footprint_check`.

References:

- plan-44-C § 3 (ids, reuse, shared ids, styles, `fp-compare.py`) and its
  tools (`census/fp-load.sh`, `fp-merge.py`, `fp-check.py`).
- `docs/parts-format.md` § Footprints (`explicit`, `drill`, `slots`, R7 for
  leads, no overlapping pads, a through-hole `drill` smaller than both pad
  sides).
- Library precedents for through-hole pad sizing: `LED-RADIAL-5MM`, the axial
  resistor footprints of plan-34, `TO-92-3-ONSEMI`.

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-C complete | `ls planning/completed/plan-44-C-*.md` → exists | MET (2026-11-22: archived with this letter) |
| The shared tools exist | `ls planning/plan-44-data/census/fp-load.sh planning/plan-44-data/census/fp-check.py planning/plan-44-data/census/fp-compare.py` → all exist | MET (2026-11-22: C Phase 1 wrote them) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-44-data/footprints-needed.txt \| wc -l` → the D lines (29 at planning time, before B's pick) | MET (2026-11-22: 29 lines from B's 600 `add` rows (107 parts; 124 before the pick), 27 with no existing footprint) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-44-data/census/fp-check.py D` prints `<n> lines, 0
  without a footprint`, `<n>` = `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-44-data/footprints-needed.txt | wc -l`
  (29 at planning time).
- `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json planning/plan-44-data/footprints-D.json`
  prints `<k> new footprints: load ok` (the objects of both files; D has at
  most 29, 27 lines having no same-size existing footprint).
- `footprint-map.tsv` has, for every D line, the footprint id and the pinouts
  it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the app.
- No edit of an existing footprint.
- No footprint for a package no `add` row uses.

## 2. Current State

The library has through-hole footprints for LEDs, axial resistors and TO
packages but none for crystal cans or resonator packages. D's lines are the
packages of the download set that C does not cover.

### Measured populations

| What | Count | Command |
|---|---|---|
| D lines (maker, DigiKey Package / Case, supplier package, size), download set | 29 (124 parts) | `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-44-data/footprints-needed.txt \| wc -l` |
| …by package | Cylindrical Can Radial 9, 3-SMD Non-Standard 8, HC-49/US 3, 3-SMD No Lead 3, HC-49/U 2, 1512 (3731 Metric) 2, HC-46/X 1, 2-SMD No Lead 1 | `awk -F'\t' '!/^#/ && $1=="D"{print $4}' … \| sort \| uniq -c \| sort -rn` |
| …with no existing footprint of the same body size code | 27 | `awk -F'\t' '!/^#/ && $1=="D" && $11==""' … \| wc -l` |
| Phase split | Phase 1: through-hole crystals (HC-49/U, HC-49/US, HC-46/X, radial cylinder) 15 lines; Phase 2: ceramic resonator packages (3-SMD, 1512, 2-SMD) 14 lines | the package counts above: 9 + 3 + 2 + 1; 8 + 3 + 2 + 1 |

### Verified properties

- **A through-hole pad needs `drill` smaller than both pad sides** and the
  footprint a `body`; `explicit` through-hole pads take `shape` `round` or
  `rect` with `drill` (`docs/parts-format.md` § Footprints, Common keys and
  `explicit`).
- **A three-lead part has three numbered pads**: a footprint's `pins` is the
  pad count; pad 2 of a resonator is the middle (ground) lead, the pinout's
  `2=GND` of plan-44-B § 3.3.

## 3. Design

Ids, reuse and shared ids are plan-44-C § 3.1 (`<label>-<MAKER>`, the label
from `footprints-needed.txt`): `XTAL-HC49U-2-ABRACON`, `XTAL-HC49US-2-ECS`,
`XTAL-HC46X-2-ECS`, `XTAL-CYL-3X8-2-ABRACON`; ceramic resonators
`CRES-3731-3-ABRACON`, `CRES-6030-3-ECS`. Through-hole pads are `round` (or
`rect` for pad 1 when the datasheet marks it) with the datasheet's
recommended hole and pad (R1); with no recommendation: hole = lead diameter +
0.3 mm, pad = hole + 0.6 mm, mid-values (R7). Surface-mount resonator pads
are `explicit` rectangles from the recommended land pattern; the `note`
names the drawing, the PDF under `datasheets/crystal/` and the page.

Risk: three-pad resonators whose datasheets draw one pattern for several
body sizes; the line's size column picks the drawing, and the hand check
compares one object per package family.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Through-hole crystals

- [x] Draw the HC-49/U, HC-49/US, HC-46/X and radial-cylinder lines (15 at
      planning time) into `footprints-D.json`, or map them to an existing
      footprint by plan-44-C § 3.1; fill the map rows; hand-check one object
      per package against the drawing.

Acceptance: those lines have footprints that load.
  Check: `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json planning/plan-44-data/footprints-D.json` → `<k> new footprints: load ok` (est. 3 min). Output 2026-11-22 (reader d1, 15 lines): `15 lines, 0 without a footprint`, `9 new footprints: load ok`.
Commit: —

### Phase 2 — Ceramic resonator packages

- [x] Draw the 3-SMD (non-standard and no-lead), 1512 (3731 metric) and 2-SMD
      ceramic-resonator lines (14 at planning time) as Phase 1; hand-check one
      object per package family.

Acceptance: every D line has a footprint that loads.
  Check: `python3 -I planning/plan-44-data/census/fp-check.py D` → `<n> lines, 0 without a footprint`, and `planning/plan-44-data/census/fp-load.sh planning/plan-44-data/footprints-C.json planning/plan-44-data/footprints-D.json` → `<k> new footprints: load ok` (est. 3 min). Output 2026-11-22 (reader d2, 14 lines, then everything): `14 lines, 0 without a footprint`, `8 new footprints: load ok`; after `fp-skeleton.py --assemble`: `fp-check.py D` → `29 lines, 0 without a footprint`; `fp-load.sh footprints-C.json footprints-D.json` → `72 new footprints: load ok` (55 of C, 17 of D).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py D` joins every D line with a map row.
- Runtime proof: n/a (F regenerates the footprint golden).
- Doc sync: none.
- Final gate: plan-44-H.

## Decisions

- **Resonators carry their own id prefix** (`CRES-`), because `RES-` is the
  resistor family's.
- **Through-hole holes follow the datasheet's recommended hole**, else lead +
  0.3 mm (R7).

## Corrections

1. **Shared tools and the worksheets are plan-44-C's** (Corrections 2 there):
   readers `d1` (through-hole crystals, 15 lines) and `d2` (ceramic resonators,
   14 lines) wrote `fp-parts/d1`, `d2` (`.tsv` + `.json`); `fp-skeleton.py
   --assemble` made `footprints-D.json` (17 footprints, none reused) and the D
   rows of `footprint-map.tsv`.
2. **No datasheet of the through-hole crystals prints a PCB land pattern**, so d1
   drew every one by R8 from the lead spacing and lead diameter; for the radial
   cylinders (lead spacings 0.45–1.1 mm) R8's pad (hole + 0.6 mm) overlaps, so the
   round pad is `span − 0.1` and the drill `min(lead + 0.3, pad − 0.2)`, never
   below lead + 0.05 (`XTAL-CYL-2X5-2-ECS`: drill 0.25, ring 0.05 mm: the leads
   are bent apart in practice). The body of a can is round (diameter).
3. **Shared ids and merged lines:** `XTAL-CYL-3X10-2-ECS` serves 3X9 too (same pad
   data), `XTAL-CYL-3X8-2-ABRACON` serves AB308 and AB38T, `XTAL-HC49U-2-ABRACON`
   serves the `HC49US` label, `XTAL-HC49U-2-ECS` the U, US and USS lines.
4. **Resonator land patterns** (`d2`): three-pad resonators are `explicit`,
   pins numbered left to right (`1=X1,2=GND,3=X2`); `CRES-2520-3` has two pads per
   terminal (six pads for three pins, allowed by § `explicit`); `CRES-3731-2`
   is the two-pad 1512 (3731) part (label `-2`: the pad count comes from the
   pinout); `CRES-4741-3-ABRACON` is drawn from `N_AWSCR-MT.pdf` (the AWSCR-MTD
   rows; plan-44-B Corrections 9).

## Summary

D is the plan's small footprint letter: a few dozen objects, through-hole
pads whose drill rule is the only new check. Untouched: C's objects, every
existing footprint, the backend and the app.
