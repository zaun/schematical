# plan-34-C: Resistor footprints — chip resistors, and the shared rules

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-34-B

C draws the land pattern of every chip-resistor package the `add` rows use,
from each datasheet's recommended land pattern, and records the package
name, pinout and footprint id of each chip `specs.tsv` row. § 4 holds the
footprint rules D reuses. The footprints go into a ready file,
`planning/plan-34-data/footprints-C.json`; F appends them to
`Parts/footprints.json` with the parts that use them (a footprint no part
uses fails `footprint_check`'s `unused footprint` test).

References:

- `docs/parts-format.md` § Footprints (styles, `dual`, mid-values, no
  overlapping pads, the `"footprint"` part key).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` § 4 (R1
  rule, reuse within 0.05 mm, ready objects, `fp-load.sh`: the precedent C
  copies); `planning/plan-29-data/census/fp-load.sh`, `fp-merge.py`.
- `planning/completed/plan-26-A-pull-resistor-data.md` § 4 (the generic
  `RES-0402` … `RES-1206`, `RES-AXIAL-10.16`, `RES-RADIAL-2.54`).
- `planning/plan-34-data/footprints-needed.txt`, `specs.tsv` (plan-34-B).

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-34-B complete | `ls planning/completed/plan-34-B-*.md` → exists | NOT MET at writing (2026-11-11) |
| `footprints-needed.txt` from `add` rows | `grep -c '^chip' planning/plan-34-data/footprints-needed.txt` → B's count (34 at planning time) | NOT MET at writing (B Phase 3 regenerates it) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every chip line of `footprints-needed.txt` (34 at planning time) has a
  footprint: an object of `footprints-C.json` or an existing id of
  `Parts/footprints.json` that matches its drawing (§ 4.3).
- Every chip `specs.tsv` row has `package`, `pinout` (`two-pin`) and
  `footprint` filled.
- `census/fp-load.sh footprints-C.json` loads the objects with no load
  error; its only failures are `extra footprint (not in the golden)` and
  `unused footprint` for exactly the new ids.

### Non-goals (explicit constraints)

- `Parts/footprints.json` and the goldens are not changed (F appends).
- The six generic `RES-*` footprints and their parts (the pulls) are not
  changed.
- No 3D body, no courtyard beyond what `dual` draws.

## 2. Current State

Chip lines at planning time (plan-34-A § 2, `grep '^chip'
footprints-needed.txt`): YAGEO `PYU-RC_GROUP_51_ROHS_L` 01005, 0201, 0402,
0603, 0805, 1206, 1210, 2010, 2512, Wide 1218 (10 lines, 1168 rows);
`PYU-RC_51_ROHS_P` 0201, 0402, 0603; `PYU-RT_1-TO-0-01_ROHS_L` 0603,
0805, 1206; `PYU-RE_105_ROHS_L.pdf` 0805; `PYU-AC_51_ROHS_L` 0402;
`YAGEO-MMF_DATASHEET` MELF 0204; Vishay Dale `crcwce3.pdf` 0402, 0603,
0805, 1206; `crcw0201e3.pdf` 0201; `crcw01005e3.pdf` 01005;
`dcrcwhre3.pdf` 0603; `rcge3.pdf` 0402, 0603; Viking Tech `CR_Series.pdf`
0201, 0402, 0603, 0805; `AS_Series.pdf` / `AS%20Series%20Datasheet.pdf`
0402. Existing ids that a drawing may match: `RES-0402`, `RES-0603`,
`RES-0805`, `RES-1206` (generic, plan-26-A § 4), `DIODE-MELF`,
`DIODE-MINIMELF`, `DIODE-MICROMELF` (Vishay diode drawings, plan-29)
(`grep -o '"id":"\(RES\|DIODE-[A-Z]*MELF\)[^"]*"' schematical/schematical/Parts/footprints.json`).

## 3. Design

C works down the chip `specs.tsv` rows, one PDF at a time: reads the
recommended land pattern (YAGEO: "FOOTPRINT AND SOLDERING PROFILES" or the
"Recommended land pattern" table of the series; Vishay: "SOLDER PAD
DIMENSIONS" / "RECOMMENDED SOLDER PAD DIMENSIONS"; Viking: "Recommended
land pattern"), writes one `dual` object per (maker drawing, size), checks
it with `fp-load.sh`, and fills the spec row.

**Risk:** one wrong span or pad size lands on hundreds of parts (YAGEO 0402
alone is 321 rows). Each object's `note` repeats the drawing's numbers and
the page; the reviewer of the commit compares the note with the PDF.

## 4. Footprint rules (shared by C and D)

### 4.1 Which rule

- **R1** — the datasheet (or the maker's land-pattern document it points
  to) gives a recommended land pattern: use its pad sizes and centres.
- **R4** (plan-03-B) — a chip with no land pattern in its PDF: leadless
  terminals; pads from the terminal sizes per R4.
- **R7** (plan-29-B § 4.1) — axial leads (D): `dual`, `pins` 2, `perSide`
  1, `pitch` 1; `span` = the datasheet's stated lead spacing for the body
  (YAGEO MFR/FMP/MF0 print "lead spacing" per power rating), else the
  smallest multiple of 2.54 mm that is at least body length (max) + 3.0
  mm; `drill` and round `pad` per R5 from the lead diameter (max). The
  `note` gives the body, the lead diameter and the arithmetic.
- Mid-values and "no overlapping pads" as `docs/parts-format.md` says.

### 4.2 Pin numbers and pinouts

- **Two-terminal resistors** (chip, MELF, axial): pin 1 and pin 2, no
  polarity; pinout `two-pin`
  (`[{"number":1,"name":"1","role":"inout"},{"number":2,"name":"2","role":"inout"}]`).
  On the footprint pin 1 is the left pad.
- **Arrays** (D): the datasheet's pin numbers; each resistor element is a
  (pin, pin) pair read from the datasheet's circuit diagram (isolated:
  pin `k` to pin `N + 1 - k`; bussed: the common pin(s) to each other pin,
  as drawn). Pinout names: `iso-<n>` (isolated, `n` pins), `bus-<n>`
  (bussed, common pin 1), or the datasheet's own shape in words when it
  differs; the `pinout` column of `specs.tsv` holds the name and the
  pairs: `iso-8:1-8,2-7,3-6,4-5`.

### 4.3 Footprint ids, reuse, ready objects

- **Ids.** Chip: `RES-<size>-<MAKER>` (`RES-0402-YAGEO`, `RES-0603-VISHAY`,
  `RES-0805-VIKING`); a second drawing of the same maker and size (another
  series' sheet that differs) appends the series: `RES-0603-YAGEO-RT`.
  MELF: `RES-MELF0204-YAGEO`. Axial (D): `RES-AX-<span>-<body length>x<body
  diameter>-<MAKER>` (`RES-AX-10.16-6.3x2.4-YAGEO`). Arrays (D):
  `RES-ARRAY-<size>-<pins>-<MAKER>` (`RES-ARRAY-0804-8-YAGEO`), SIP / SOIC
  by their style's name (`SIP-8-2.54-VISHAY`, `SOIC-16-5.59-VISHAY`).
- **Reuse.** A drawing whose land pattern equals an existing footprint
  (every pad's size and centre within 0.05 mm, same drill) reuses it; the
  first object written for a package owns its id. C checks
  `Parts/footprints.json` first; D checks `footprints-C.json` too.
- **Shared ids.** Before adding an id that another open plan may also add
  (none of the `RES-<size>-<MAKER>` ids at planning time: `grep -l
  'RES-[0-9]*-\(YAGEO\|VISHAY\|VIKING\)' planning/plan-*.md | grep -v
  plan-34` → none), run `grep -c '"id":"<id>"'
  schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`;
  a hit means reuse it (its pads are checked against this drawing as any
  reuse is) and the add step is marked moot with that grep's output.
- **Ready objects.** `footprints-C.json` is `{"footprints":[…]}` of objects
  in `footprints.json`'s format, each with a `note` naming the drawing, the
  PDF path under `datasheets/resistor/` and the page.
- **`census/fp-load.sh`** (written in C Phase 1, plan-29's copy): copies
  `backend/` and `Parts/` to `/tmp/p34-fp`, appends the objects of the
  `footprints-*.json` files named on the command line to the copy's
  `footprints.json`, builds `footprint_check` there and runs it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools and the YAGEO chip sheets

- [ ] Copy `plan-29-data/census/fp-load.sh` and `fp-merge.py` to
      `plan-34-data/census/` (paths `/tmp/p34-fp`, `datasheets/resistor`).
- [ ] Write `census/fp-check.py <category>`: joins the category's lines of
      `footprints-needed.txt` with the `footprint` column of their
      `specs.tsv` rows and the ids of `footprints-C.json`,
      `footprints-D.json` and `Parts/footprints.json`; prints `<n> lines,
      <m> without a footprint` and each missing line.
- [ ] YAGEO `PYU-RC_GROUP_51_ROHS_L` (10 sizes), `PYU-RC_51_ROHS_P`,
      `PYU-RT_1-TO-0-01_ROHS_L`, `PYU-RE_105_ROHS_L`, `PYU-AC_51_ROHS_L`,
      `YAGEO-MMF_DATASHEET` (MELF 0204): one object per distinct land
      pattern (§ 4.3), the spec rows filled.

Acceptance: the YAGEO objects load.
  Check: `planning/plan-34-data/census/fp-load.sh planning/plan-34-data/footprints-C.json` → only `extra footprint` / `unused footprint` failures, for exactly the new ids (est. 2 min).
Commit: —

### Phase 2 — Vishay Dale and Viking Tech chip sheets

- [ ] Vishay Dale `crcwce3.pdf` (0402–1206), `crcw0201e3.pdf`,
      `crcw01005e3.pdf`, `dcrcwhre3.pdf`, `rcge3.pdf`; Viking Tech
      `CR_Series.pdf`, `AS_Series.pdf`: objects and spec rows as Phase 1.
- [ ] Any other chip line B's regenerated `footprints-needed.txt` holds.

Acceptance: every chip line has a footprint and every chip spec row is filled.
  Check: `python3 -I planning/plan-34-data/census/fp-check.py chip` → `<n> lines, 0 without a footprint` with `<n>` = `awk -F'\t' '$1=="chip"' planning/plan-34-data/footprints-needed.txt | wc -l` (34 at planning time), and `fp-load.sh footprints-C.json` as Phase 1 (est. 3 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` on the copy (`fp-load.sh`); the real goldens
  change in F.
- Coverage check: Phase 2's `0 without a footprint`.
- Runtime proof: not applicable until F loads parts on them.
- Doc sync: none (`docs/parts-format.md` § Footprints already covers
  `dual`).
- Final gate (once, after Phase 2): `cmake --build /tmp/logic-p34 -j8 &&
  ctest --test-dir /tmp/logic-p34` → `100% tests passed` (est. 1 min;
  nothing in the tree changed).

## Decisions

- **One footprint per maker drawing, not one per size** — author: the
  makers' recommended land patterns differ (pad lengths, gaps), and
  `AGENTS.md` asks for the datasheet's drawing; the 0.05 mm reuse rule
  merges the ones that agree.
- **The generic `RES-*` ids stay generic** — author: they were sized
  without a datasheet (plan-26-A § 4) and the pulls and `GENERIC-RESISTOR`
  use them; a maker part reuses one only when its drawing matches.
- **Pin names `1` and `2`** — author: a resistor has no polarity; the
  numbers are what the footprint and the netlist show.
- Owner decisions: plan-34-A § Decisions.

## Corrections

(none yet)

## Summary

C's risk is a land pattern copied wrong across hundreds of parts; each
object's `note` carries the drawing's numbers for review, and `fp-load.sh`
catches malformed or overlapping pads. Left untouched: `Parts/`,
`backend/`, the app.
