# plan-36-D: Microcontrollers — quad and grid footprints (Microchip, RP2040)

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-C

D does for the quad and grid package codes what C did for the dual-row
ones: TQFP, LQFP, QFN/VQFN/WQFN/UQFN, dual-row QFN, PLCC, BGA and WLCSP
for the AVR rows, plus the RP2040's QFN-56. New objects go in
`planning/plan-36-data/footprints-D.json`. G merges them.

Outcome of D: every `quad`, `drqfn`, `plcc` and `grid` row of
`package-codes.tsv` names a footprint. `fp-load.sh footprints-C.json
footprints-D.json` reports only `extra` / `unused` lines for exactly the
new ids.

References:

- plan-36-A § Prerequisites; plan-36-C § 3 (reuse rule, ids, `package`
  values), which D follows unchanged.
- `docs/parts-format.md` § `quad` — four sides, § `grid` — ball grid,
  § `explicit`, `centre`, `slots`.
- `datasheets/mcu/raspberry-pi/rp2040-datasheet.pdf` § 5.1.2 "Recommended
  PCB Footprint" (`pdftotext` line 45822) and § 5.1 (package drawing).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-C complete | `ls planning/completed/plan-36-C-*.md` → exists | NOT MET (2026-11-12: plan written) |

## 1. Goal

- Each `package-codes.tsv` row of geometry `quad`, `drqfn`, `plcc` or
  `grid` has a `footprint`. Planning saw 31 AVR (package, supplier
  package) lines over 981 parts, plus the RP2040 (`footprints-needed.txt`).
- `fp-load.sh footprints-C.json footprints-D.json` → only `extra` /
  `unused` lines for the new ids.

### Non-goals (explicit constraints)

- No change to `Parts/`, goldens or the format. No module footprint (E).

## 2. Current State

- Reusable candidates (`grep -o '"id": "[^"]*"' …/footprints.json`):
  `TQFP-48-7X7`, `TQFP-32-PBS`, `TQFP-32-PJT`, `TQFP-64-PAG`,
  `LQFP-32-7X7`, `LQFP-44-10X10`, `LQFP-48-PT`, `LQFP-64-10X10`,
  `LQFP-100-14X14`, `QFN-32-5X5`, `QFN-48-7X7`, `QFN-24-4X4`,
  `QFN-28-5X5-XR`, `VQFN-20`, `VQFN-32-5X5`, `PLCC-44`, and one BGA
  (`BGA-24-MICROSTAR`, not a match). There is none for 7×7 QFN-56 at
  0.4 mm (`QFN-56-RTQ` and `VQFN-56-RSH` are TI drawings of other bodies,
  ruled out by the reuse rule's pitch/span test in Phase 2).
- The planning proxy's quad and grid packages: 32-TQFP (159 parts),
  32-VQFN 5×5 (142), 64-TQFP 14×14 (110) and 10×10 (13), 44-TQFP (85),
  20-VQFN 3×3 (69), 64-QFN 9×9 (75), 44-VQFN 7×7 (60), 20-QFN 4×4 (45),
  28-VQFN 4×4 (45), 48-TQFP (39), 100-TQFP (38), 24-VQFN (31), 48-VQFN
  (27), the BGAs (49-VFBGA 9, 15-UFBGA 6, 100-VFBGA 4, 32-UFBGA 3,
  12-WLCSP 1), 44-PLCC (4), the dual-row QFNs (44 and 64, 6) and the rest
  (`footprints-needed.txt`).
- The RP2040 is 7×7 mm QFN-56 at 0.4 mm pitch with an exposed pad. The
  datasheet warns that "standard 0.4mm QFN-56 footprints provided with CAD
  tools may need adjusting" (`pdftotext` line 45789), so its footprint
  follows § 5.1.2 exactly.

## 3. Design

- C § 3's reuse rule, pad rules and ids, with the families `TQFP`, `LQFP`,
  `QFN` (any QFN/VQFN/WQFN/UQFN/MLF), `DRQFN`, `PLCC`, `BGA`, `WLCSP`.
  Example ids: `TQFP-32-MC-32A`, `QFN-32-MC-32M1-A`, `QFN-56-RP2040`.
- **Exposed pads**: AVR QFN exposed pads are `centre` pads with no pin
  (unnumbered; B § Decisions). The RP2040's exposed pad is pin 57, so its
  footprint has `pins` 57 and `slots` with pin 57 → 0, the centre (the
  `slots`/`centre` rule of `docs/parts-format.md` § Common keys).
- **Dual-row QFN** (`drqfn`): style `explicit`. The pads are as the
  drawing places them, with designators as the datasheet numbers them.
  B's `drqfn` pinout uses those designators, and the loader matches pads
  by designator.
- **BGA / WLCSP**: style `grid`, R6 pads; ball names as the drawing prints
  them (the B pinout uses the same names).
- **PLCC-44**: `PLCC-44` exists (plan-03-B); reuse it if the reuse rule
  passes, otherwise `PLCC-44-MC-<code>`.
- **`package` values**: `TQFP` (LQFP-48 too, as `LQFP`), `QFN` (all
  QFN/VQFN/MLF, dual-row included), `PLCC`, `BGA`, `WLCSP`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — AVR quad and grid footprints

- [ ] `census/fp-codes.py --geometry quad,drqfn,plcc,grid` →
      `fp-pages.tsv` rows for these codes.
- [ ] For each code: the reuse rule or a new object in `footprints-D.json`.
      Then fill `package-codes.tsv`'s `footprint`.

Acceptance: every AVR quad/grid code has a footprint, and the new ones load.
  Check: `awk -F'\t' '$2~/^(quad|drqfn|plcc|grid)$/ && $6==""' planning/plan-36-data/package-codes.tsv | wc -l` → 0 except the RP2040 row; `fp-load.sh footprints-C.json footprints-D.json` → only `extra` / `unused` lines (est. 4 min).
Commit: —

### Phase 2 — RP2040

- [ ] `QFN-56-RP2040` from § 5.1.2 (R1: the pad size, pitch and exposed-pad
      size as printed), `pins` 57, centre pad = pin 57. Its row in
      `package-codes.tsv` names it.
- [ ] Load both files again.

Acceptance: all D codes have footprints, and they load.
  Check: `awk -F'\t' '$2~/^(quad|drqfn|plcc|grid)$/ && $6==""' planning/plan-36-data/package-codes.tsv | wc -l` → 0; `fp-load.sh footprints-C.json footprints-D.json` → only `extra` / `unused` lines (est. 3 min).
Commit: —

## Validation Plan

- Tests: the real loader through `fp-load.sh`; `footprint_check` in G.
- Coverage check: Phase 2's empty-`footprint` count over all four geometries.
- Runtime proof: plan-36-I places an ATmega328P-AU and an RP2040 on the
  PCB page.
- Doc sync: none.
- Final gate: plan-36-I.

## Decisions

- **The dual-row QFN as `explicit`** — author: `quad` places one row per
  side. A dual-row QFN has two staggered rows per side, so literal pads
  are the only exact form the format has.
- **The RP2040 footprint from its own § 5.1.2**, never reused — author:
  the datasheet itself says generic QFN-56 footprints need adjusting.

## Corrections

## Summary

D is the bulk of the land patterns: most parts are TQFP or QFN. Its risk
is the same transcription risk as C's, and the same loader and drawing
checks cover it. The RP2040 pad is the one place where a generic pattern
would be wrong.
