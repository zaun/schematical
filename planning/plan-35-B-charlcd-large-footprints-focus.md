# plan-35-B: 20x4 / 40x2 / 40x4 LCDs — the 16 Focus module footprints

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-A

B draws the module footprints of the 32 Focus parts from their
mechanical drawings: 16 footprints, one per drawing sheet. They are kept
as ready objects in `planning/plan-35-data/footprints-B.json`.
plan-35-F merges them into `footprints.json` together with the parts that
use them, because `footprint_check` fails on a footprint no part uses
(`docs/parts-format.md` § The `"footprint"` part key).

Outcome of B: `footprints-B.json` holds the 16 objects of § 2;
`census/fp-load.sh planning/plan-35-data/footprints-B.json` reports only
`extra footprint` / `unused footprint` for exactly those 16 ids.

References:

- plan-35-A § Prerequisites (the gate for all of plan-35), § Decisions.
- `docs/parts-format.md` § Footprints, § `explicit` — literal pads, § The
  `"footprint"` part key (mid-values, the `note`).
- `planning/completed/plan-11-C-charlcd-parts.md` § 3 (the module pad
  rules this copies) and the four `LCM-*` footprints it added.
- `planning/plan-35-data/footprints-needed.txt` (ids, parts, drawing pages,
  header layout); `datasheets/module-disp-char/` (the PDFs, from A).

## Prerequisites

The gate is plan-35-A § Prerequisites. B adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-A complete | `grep -c '^Commit: —' planning/plan-35-A-charlcd-large-data.md` → 0 (or the file is in `planning/completed/`) | NOT MET (2026-11-11: A not started) |
| The Focus PDFs are in the tree | `awk -F'\t' '$1=="Focus LCDs" && $4=="add"{print "datasheets/module-disp-char/"$13}' planning/plan-35-data/candidates.tsv \| xargs ls \| wc -l` → the Focus `add` count (32 at planning) | NOT MET (A Phase 2) |
| The loader probe works | `echo '{"footprints":[]}' > /tmp/p35-fp0.json && planning/plan-35-data/census/fp-load.sh /tmp/p35-fp0.json` → `0 new footprints: load ok` | MET (2026-11-11) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-B.json`
  → `16 footprints appended` and failures only of the form `extra footprint`
  / `unused footprint` for the 16 ids of § 2 (one less per group A emptied).
- Every object's `pins` equals its parts' pin count (`candidates.tsv`
  column `pins`), and every pin 1…`pins` has a pad.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, the part files and the goldens are not touched
  (F merges).
- No connector footprint is drawn: `HDR-1X16-P2.54`, `PINHDR-1X18-V` and
  `PINHDR-1X20-V` already exist (`grep -c '"id":"HDR-1X16-P2.54"\|"id":"PINHDR-1X18-V"\|"id":"PINHDR-1X20-V"' schematical/schematical/Parts/footprints.json`
  → 3).

## 2. Current State

The 16 Focus footprints (`grep '^LCM-FOCUS' planning/plan-35-data/footprints-needed.txt`;
outline is DigiKey's, which matches each datasheet's module size; drawing
pages are PDF pages):

| Id | Outline | Pins | Header (drawing) | Drawing | Parts |
|---|---|---|---|---|---|
| LCM-FOCUS-C204A-BW | 98 × 60 | 16 | 1x16 top-left | C204A-BW-LW25_Spec.pdf p.4 | C204A-BW-LW25 |
| LCM-FOCUS-C204ABB | 98 × 60 | 16 | 1x16, drawing rotated 90° | C204ABBSYSY6WT55PAB_Spec.pdf p.5 | C204ABBSYSY6WT55PAB |
| LCM-FOCUS-C204ADB | 98 × 60 | 16 | 1x16 top-left | C204ADBFGSW6WT55PAB_Spec.pdf p.2 | C204ADBFGSW6WT55PAB |
| LCM-FOCUS-C204ALB-98 | 98 × 60 | 16 | 1x16 top-left | C204ALBSYLY6WM55PAA_Spec.pdf p.3 | C204ALBSYLY6WM55PAA |
| LCM-FOCUS-C204AXB | 98 × 60 | 16 | 1x16 top | C204AXBSYLY6WT_Spec.pdf p.2 | C204AXBSYLY6WT, C204AXBSGN06WR50XAA |
| LCM-FOCUS-C204BLB-146 | 146 × 62.5 | 16 | 1x16 top-left | C204BLBFWSW6WT33XAA_Spec.pdf p.3 | C204BLBFWSW6WT33XAA, C204BLBSBSW6WN33PAB, C204BLBSYLY6WT33PAB |
| LCM-FOCUS-C204DLB | 146 × 62.5 | 16 | 1x16 top-left | C204DLBSBSW6WN55XAA_Spec.pdf p.3 | C204DLBSBSW6WN55XAA |
| LCM-FOCUS-C204ALB-155 | 155.5 × 73 | 16 | 1x16 top-left | C204ALBFWSW6WT33XAA_Spec.pdf p.3 | C204ALBFWSW6WT33XAA, C204ALBSBSW6WN33XAA, C204ALBSYLY6WT33XAA |
| LCM-FOCUS-C204BLB-155 | 155.5 × 73 | 16 | 1x16 top-left | C204BLBSBSW6WN55XAA_Spec.pdf p.3 | C204BLBSBSW6WN55XAA, C204BLBSYLY6WT55XAA |
| LCM-FOCUS-C204CXB | 77 × 47 | 16 | 1x16 top-left | C204CXBSGSW6WT33XAA_Spec.pdf p.3 | C204CXBSGSW6WT33XAA, C204CXBSGSW6WT55XAA |
| LCM-FOCUS-C204HLB | 96 × 66 | 20 | 1x20 bottom | C204HLBSYLY6WM_Spec.pdf p.3 | C204HLBSYLY6WM |
| LCM-FOCUS-C402ALB-285 | 285 × 56 | 16 | 2x8 left (15–16 NC) | C402ALBSGN06WT50PAB_Spec.pdf p.3 | C402ALBSGN06WT50PAB |
| LCM-FOCUS-C402A3 | 182 × 33.5 | 16 | 2x8 left | C402A3-BW-LW65_Spec.pdf p.3 | C402A3-BW-LW65 |
| LCM-FOCUS-C402ALB | 182 × 33.5 | 16 | 2x8 left | C402ALBFKSW6WT55XAA_Spec.pdf p.3 | C402ALBFKSW6WT55XAA, C402ALBFWSW6WT33XAA, C402ALBSBSW6WN33XAA, C402ALBSBSW6WN55XAA, C402ALBSYLY6WT33XAA, C402ALBSYLY6WT55XAA |
| LCM-FOCUS-C402ASB | 182 × 33.5 | 16 | 1x16 | C402ASBSYLY6WT55PAB_Spec.pdf p.2 | C402ASBSYLY6WT55PAB |
| LCM-FOCUS-C404ALB | 190 × 54 | 18 | 2x9 left | C404ALBFKSW6WT55PAB_Spec.pdf p.3 | C404ALBFKSW6WT55PAB, C404ALBFWSW6WT33PAB, C404ALBSBSW6WF33PAB, C404ALBSBSW6WN55PAB, C404ALBSYLY6WT33PAB |

### Measured populations

| What | Count | Command |
|---|---|---|
| Focus footprints | 16 | `grep -c '^LCM-FOCUS' planning/plan-35-data/footprints-needed.txt` |
| Focus parts they serve | 32 | `awk -F'\t' '$1=="Focus LCDs" && $4=="add"' planning/plan-35-data/candidates.tsv \| wc -l` |
| Existing `LCM-FOCUS-` ids (a clash) | 0 | `grep -c '"id":"LCM-FOCUS' schematical/schematical/Parts/footprints.json` |

### Verified properties

- **Grouping.** Parts share a footprint only when their datasheets carry
  the same drawing sheet: the same template, the same outline and the
  same dimension layout. This was checked on 40–70 dpi renders of every
  member's drawing page (2026-11-11). The 98 × 60 Focus parts use five
  different sheets (rendered side by side), so they get five footprints.
  The 146 and 155.5 outlines each come in two sheets.
- **Where the data is.** The drawings are vector line art with their
  dimensions drawn as outlines, and `pdftotext` finds no dimension numbers
  on most of them. So each sheet is read from a 300 dpi render
  (`pdftoppm -r 300 -f <p> -l <p>`), not from text.
- **The C404ALB pin table** (p.3): 1–8 DB7–DB0, 9 E1, 10 R/W, 11 RS, 12 V0,
  13 VSS, 14 VDD, 15 E2, 16 NC, 17 LED+, 18 LED−. The header is 2x9
  zigzag: pins 1 and 2 side by side, 17 and 18 last.
- **The C204HLB pin table** (p.3): 1–14 as usual, 15 VSS, 16 NC, 17 NC,
  18 FGND, 19 LED+, 20 LED−, in a single 1x20 row.

## 3. Design

The pad rules are plan-11-C's, so the new modules match the five 16 × 2
footprints:

- **Coordinates:** origin at the PCB's centre, x right, y down, mm, front
  view. Body = the PCB outline (W × H as drawn).
- **Interface pins:** the drawn hole (Ø1.0 on every Focus sheet viewed);
  pad Ø1.8 round, pin 1 square, unless the drawing prints a pad size.
  Positions come from the drawing's pitch chain (`P2.54*15=38.1`) and the
  pin-1 offset.
- **Mounting holes:** `"pin": 0` pads, drilled as drawn (Ø2.5, R1.25
  half-holes, or Ø3.5 on the 40 × 4). Pad = the drawn pad, otherwise
  drill + 2.5 (Ø2.5 → Ø5.0, plan-11's size). Half-holes and edge slots
  are drawn as full holes at their centres.
- **Mid-values:** a toleranced dimension uses its mid-value (`98±0.5` →
  98). The `note` names the sheet (file and page) and every number used,
  and says which pad sizes were assumed.
- **Description:** `"Focus <family> <size> LCD module, <W> x <H> mm,
  <header>, <n> mounting holes"`.
- **Two-row headers** keep the module's numbering: pin 1, then pin 2 across
  from it, down the column (2x8 / 2x9 zigzag, as the drawings print it).

**Correctness gate:** `fp-load.sh` loads every object (pad overlap,
drill < pad, every pin padded), and each object's pin-1 position, pitch
and hole positions are compared once with the 300 dpi render. A misread
number is the risk here; the comparison is the check.

**Rejected alternatives**

- *One footprint per outline size.* Rejected: same-size Focus sheets
  differ in template and dimension layout, and pin-row offsets of 0.6 mm
  (plan-11-C's 162H vs AMC1602G) are not visible without reading each sheet.
- *Writing straight into `footprints.json`.* Rejected: `footprint_check`
  fails on unused footprints until F adds the parts (plan-29-B precedent).

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — 20 × 4 footprints (11)

- [ ] Render each sheet of the eleven `LCM-FOCUS-C204*` rows at 300 dpi
      into `/tmp/p35-fp-img/`, read outline, holes, pin row and pitch, and
      write one object per row into `planning/plan-35-data/footprints-B.json`
      (`{"footprints":[…]}`, `footprints.json` object format, style
      `explicit`).
- [ ] For each row with several parts, open every other member's sheet
      once and check its outline, holes and pin-1 position against the
      object. A member whose numbers differ gets its own footprint
      (`<id>-<part-suffix>`), recorded in Corrections with both sets of
      numbers, and `census/footprint-groups.tsv` plus `footprints-needed.txt`
      are updated.

Check: `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-B.json`
→ 11 appended; only extra/unused lines for those ids (est. 3 min).

Commit: —

### Phase 2 — 40 × 2 and 40 × 4 footprints (5)

- [ ] As Phase 1, for `LCM-FOCUS-C402ALB-285`, `C402A3`, `C402ALB`,
      `C402ASB`, `C404ALB`. For the 2x8 and 2x9 headers, number the pads
      zigzag as the drawing does. `C402ALB-285` has pads 15 and 16 (NC)
      because its pin table lists them.
- [ ] Member check for `C402ALB` (6 parts) and `C404ALB` (5 parts) as in
      Phase 1.

Check: `fp-load.sh planning/plan-35-data/footprints-B.json` → 16 appended;
only extra/unused lines for the 16 ids (est. 3 min).

Commit: —

## Validation Plan

Per-phase `fp-load.sh` runs. The footprints reach `footprint_check`'s
golden in F, and the landing tier is H.

## Decisions

- **Pad rules from plan-11-C** (author): Ø1.8 pads on Ø1.0 holes, square
  pin 1, mounting pads drill + 2.5 when not drawn, half-holes as full
  holes. The five existing LCD footprints use them.
- **Connector Only footprints are reused** (author, plan-11 owner rule:
  Connector Only is a plain 1xN header in pin order): `HDR-1X16-P2.54`
  for 16 pins, `PINHDR-1X18-V` / `PINHDR-1X20-V` for 18 and 20.
- **Grouping by drawing sheet** (author, § Verified properties).

## Corrections

(none yet)

## Summary

The risk is misreading a dimension. Each object is checked once against
its 300 dpi render, and every multi-part group is checked member by
member.
