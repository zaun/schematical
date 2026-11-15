# plan-40-D: Footprints — small-signal surface-mount transistor packages

Last updated: 2026-11-14
Effort: large (3h–1d)
Depends on: plan-40-C

D draws the land patterns of the small-signal surface-mount packages of the
final pick — SOT-23 and its variants, SC-70 / SOT-323, SC-74, SC-88 / SOT-363,
SC-75, SC-89, SOT-523 / 553 / 563 / 666 / 723, TO-243AA (SOT-89), and the
leadless and chip-scale ones (SOT-883, SOT-963, SOT-923F, SOT-1123, small
DFN, LGA, BGA / WLCSP, 0402) — from the datasheets' package drawings, reusing
a library footprint when its pads equal the drawing's. It writes
`planning/plan-40-data/footprints-D.json` and the D rows of `footprint-map.tsv`.
The behavioral outcome: every D line of `footprints-needed.txt` has a
footprint id in `footprint-map.tsv`, and the new objects load through
`footprint_check`.

References:

- plan-40-C § 3.1 (ids, reuse, shared ids, ready objects) and its tools
  (`census/fp-load.sh`, `fp-merge.py`, `fp-check.py`).
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `grid`, `explicit`,
  `slots`, `centre`; the rules R1–R9, mid-values, no overlapping pads).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` through
  `plan-29-G` (the SMD diode footprints of the same families).
- Library precedents: `SOT23-3-ONSEMI`, `SOT23-3`, `SOT23-3-318BM`,
  `SOT23-6`, `SC59-3`, `SC74-6`, `SC75-3`, `SC88-6`, `SC88A-5`, `SC89-3`,
  `SOT323-3`, `SOT363-6`, `SOT563-6`, `SOT723-3`, `SOT89-3`, `DFN6546A-3`.

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-C complete | `ls planning/completed/plan-40-C-*.md` → exists | MET (2026-11-15: done with this letter, run in parallel; Corrections 1) |
| The shared tools exist | `ls planning/plan-40-data/census/fp-load.sh planning/plan-40-data/census/fp-check.py` → both exist | MET (2026-11-15: both exist, 500cce40) |
| `footprints-needed.txt` is final | `grep -c '^D' planning/plan-40-data/footprints-needed.txt` → the D lines (63 at planning time) | MET (2026-11-15: 56, after B's final pick) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-40-data/census/fp-check.py D` prints `<n> lines,
  0 without a footprint`, `<n>` = `awk -F'\t' '$1=="D"' planning/plan-40-data/footprints-needed.txt | wc -l`
  (63 at planning time).
- `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json planning/plan-40-data/footprints-D.json`
  prints `<k> new footprints: load ok` (the objects of both files; D has at
  most 29 at planning time, the lines with no same-outline footprint).
- `footprint-map.tsv` has, for every D line, the footprint id and the pinouts
  it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the app.
- No edit of an existing footprint.
- No footprint for a package no `add` row uses.

## 2. Current State

The library has small-signal SMD footprints from the diode plans (§ References
precedents). Their pad numbering follows the diode datasheets, so a
transistor reuses one only when every pad of the pinout sits where the
datasheet's pin of that number does (plan-40-C § 2; the reuse rule compares
pad geometry).

### Measured populations

| What | Count | Command |
|---|---|---|
| D lines (maker, DigiKey Package / Case), picked rows | 63 (591 parts), 39 distinct packages | `awk -F'\t' '!/^#/ && $1=="D"' planning/plan-40-data/footprints-needed.txt \| wc -l` |
| …leaded (SOT / SC / TO-236 / TO-243 / 5-6-lead TSSOP) | 37 | the lines whose package matches `^(SOT-\|SC-7\|SC-8\|SC-74\|SC-75\|TO-236\|TO-243\|6-TSSOP\|6-VSSOP\|5-TSSOP)` and not `SC-10\|SOT-883\|SOT-963\|SOT-923\|SOT-1123` |
| …leadless and chip-scale | 26 | the rest of the D lines |
| …with no existing footprint of the same outline | 29: SOT-963 ×2, SOT-923F, SOT-553, SOT-523, SOT-1123, SC-101, all the leadless / BGA / LGA / DFN lines | the D lines with an empty 10th column |
| …by package, leaded | TO-236 / SOT-23-3 5, SOT-23-3 flat 1, SOT-23-6 2, TSOT-23-6 3, SC-74 3, SC-70 4, SOT-363 5, SC-70-5 1, SC-75 1, SC-89 2, SOT-523 1, SOT-553 1, SOT-563/666 4, SOT-723 1, TO-243AA 3 | `awk -F'\t' '!/^#/ && $1=="D" {print $4}' planning/plan-40-data/footprints-needed.txt \| sort \| uniq -c` |

### Verified properties

- **Chip-scale packages have ball designators.** A BGA / WLCSP / LGA pinout is a
  ball pinout (B's `<ball>=<name>`), whose footprint pads are named by
  `"ball"` (docs/parts-format.md § `explicit`: a footprint uses pins or
  balls, never both), so each such object is `grid` or `explicit` with
  `"ball"` pads and its `pins` is the ball count.

## 3. Design

Ids, reuse, shared ids and ready objects are plan-40-C § 3.1: `<outline>-<MAKER>`
(`SOT23-3-DIODES`, `SOT23-3-NEXPERIA`, `SC70-3-DIODES`, `SOT363-6-ONSEMI`,
`SOT883-3-NEXPERIA`, `DFN1006-3-DIODES`), variants appended. Styles: SOT / SC
leaded packages `dual` (pins per side from the drawing; a 3-lead SOT with two
pins on one side and one on the other is `explicit` or `dual` with `slots`),
SOT-89 `explicit` (as `SOT89-3`: leads 1 and 3 and the tab pad 2 with the
center lead), leadless packages `explicit` or `dual` with `centre` for an
exposed pad, chip-scale `grid` / `explicit` with ball names. Pads come from
the datasheet's recommended land pattern when it prints one (rule R1), else
from the outline drawing by the plan-03-B rules (R2–R6), mid-values throughout;
the `note` names the drawing, the PDF under `datasheets/transistor/` and the
page.

Risk: tiny packages (SOT-723, SOT-883, SOT-963, XFBGA) where a 0.05 mm
error is a different pad; the `note` carries the arithmetic and the hand check
below covers one object per family.

Rejected alternative: one footprint per JEDEC outline shared by every maker
(rejected: the library keeps one per maker drawing, and the makers' recommended
land patterns differ by up to 0.1 mm; reuse by geometry already merges the
equal ones).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Leaded packages (37 lines)

- [x] One object per distinct land pattern for the TO-236 / SOT-23-3 (and flat
      leads), SOT-23-6 and TSOT-23-6, SC-74, SC-70 / SOT-323, SC-88 / SOT-363 /
      SC-70-5, SC-75, SC-89, SOT-523, SOT-553, SOT-563 / 666, SOT-723 and
      TO-243AA lines, from each line's example part's datasheet
      (`footprints-needed.txt` 8th column); reuse where an existing footprint's
      pads equal the drawing's (plan-40-C § 3.1); the `footprint-map.tsv` rows.

Acceptance: the leaded objects load and their lines are covered.
  Check: `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json planning/plan-40-data/footprints-D.json` → only `extra footprint` / `unused footprint` failures, for exactly the new ids; no load error (est. 4 min: copies `backend/`, builds `footprint_check`).
  Result (2026-11-15): `fp-load.sh footprints-C.json footprints-D.json` → `69 new footprints: load ok`.
Commit: 39fbc0e6

### Phase 2 — Leadless and chip-scale packages (26 lines)

- [x] One object per distinct land pattern for the SOT-883, SOT-963, SOT-923F,
      SOT-1123, 3- and 6-pad DFN / UDFN / XFDFN / WDFN, XFLGA, 0402, and the
      4- and 6-ball UFBGA / XFBGA / WLCSP lines (ball pads for the chip-scale
      ones), from each line's example part's datasheet; reuse as Phase 1.
- [x] Hand-check one object per family (a SOT, an SC, a SOT-89, a DFN, a
      chip-scale one): pad centres and sizes against the drawing, recorded in
      § Corrections.

Acceptance: every D line has a footprint and the objects load.
  Check: `python3 -I planning/plan-40-data/census/fp-check.py D` → `<n> lines, 0 without a footprint` (n = the D lines); `fp-load.sh footprints-C.json footprints-D.json` as Phase 1 (est. 6 min).
  Result (2026-11-15): `fp-load.sh footprints-C.json footprints-D.json` → `69 new footprints: load ok`; `fp-check.py D` → `56 lines, 0 without a footprint`.
Commit: 39fbc0e6

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py D` joins every D line.
- Runtime proof: n/a here (G's goldens and H's harness render the parts).
- Doc sync: none.
- Final gate: plan-40-I.

## Decisions

- **Objects stay in `planning/plan-40-data/footprints-D.json` until G** —
  author: `footprint_check` fails on a footprint no part uses.
- **Reuse by pad geometry** — author: plan-40-C § 2.
- **Ball packages use ball pads** — author: a ball pinout needs ball
  designators (§ 2 Verified properties).
- Owner decisions: plan-40-A § Decisions.

## Corrections

1. **Done by an agent per letter, in parallel;** the plan's work was done by one agent per letter from the brief
   `/tmp/p40/fp/BRIEF.md` (the plan's § 3 rules, the parts-format footprint
   rules, the precedents), run in parallel with plan-40-B's reading; its
   objects are in `planning/plan-40-data/footprints-D.json` and its map
   rows were merged into `planning/plan-40-data/footprint-map.tsv` (columns
   `letter maker package supplier footprint label note`: one row per
   supplier device package of a line, or `*`, because one DigiKey Package /
   Case can hold several body sizes, e.g. `X2-DFN0606-3 | X2-DFN1006-3`).
   The lines were redone once against B's final pick (`footprints-needed.txt`
   from the `add` rows: 135 lines, C 20, D 56, E 59, against the planning
   time's 28, 63, 73); objects for lines the final pick dropped stay in the
   file and G merges only the ids the final map uses.
2. **Hand checks** (one per family, as images; also a contact sheet):
   SOT — `SOT23-3-NEXPERIA`, `nexperia/BC856_BC857_BC858.pdf` p.10; SC —
   `SC70-3-VISHAY`, `vishay-siliconix/si1308edl.pdf` p.10; SOT-89 —
   `SOT89-3-DIODES`, `diodes/BST39.pdf` p.6; DFN — `UDFN6-2X2-ONSEMI`,
   `onsemi/ntlus4930n-d.pdf` p.7; chip-scale — `MICROFOOT-4-0808-VISHAY`,
   `vishay-siliconix/si8808db.pdf` p.7.
3. **Judgment calls.** Reuse at exactly the 0.05 mm limit: `SOT23-3-TSC`
   for the Diodes SOT-23 and the Infineon PG-SOT23. Pads a maker numbers but
   its land omits are added at outline size (`DFN1010D-3-NEXPERIA` 4 pins,
   pin 4 the heat bar; `DFN1010B-6-NEXPERIA` 8, pins 7–8, matching B's
   pinouts with trailing pads); unnumbered flags are pads of pin 0.
   Unnumbered Diodes DFNs: pads in the drawn order 1 B, 2 E, 3 C, as B's
   pinouts. Vishay MICRO FOOT and Fairchild WLCSP use ball names; onsemi
   EFCP / XLLGA bumps are numbered pins. `SOT23-3-VISHAY-AN826` carries a
   suffix because `SOT23-3-VISHAY` exists in plan-29's data. Every example
   part's pinout length equals its footprint's pin count (the agent's
   check over `D-final.tsv`).

## Summary

D is the most numerous drawing letter (63 lines, up to 29 new objects). The
risk is a wrong pad on a tiny package; the note carries the arithmetic and
one object per family is hand-checked. Nothing in the tree changes except
`planning/plan-40-data/`.
