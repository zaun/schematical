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
| plan-36-C complete | `ls planning/completed/plan-36-C-*.md` → exists | MET 2026-11-13 (cbac6c6) |

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

- [x] `census/fp-codes.py --geometry quad,drqfn,plcc,grid` →
      `fp-pages.tsv` rows for these codes.
      → 56 D rows in `fp-pages.tsv` (with C's, 92 lines). 19 come from `fp-pages-hand.tsv`: image drawings, plus 44MC, where the title match had taken the single-row C04-21419.
- [x] For each code: the reuse rule or a new object in `footprints-D.json`.
      Then fill `package-codes.tsv`'s `footprint`.
      `census/fp-specs-D.py`: R1 for the Microchip land patterns, R2 for the Atmel TQFPs, R3 for 44J, R4 for the Atmel QFNs, R6 for the BGAs. Reuse is within 0.10 mm, centre pad included, within the family: LQFP-32-NEY (PT-32, A-32, TQFP32), LQFP-44-10X10 (44A, 44ML), LQFP-64-10X10 (PT-64), HVQFN-32-5X5 (RXB, M-32, QZB, 32MS1, 32PN, 32M1-A, 32Z), QFN-20-RTJ (20M1), VQFN-32-RHM (QFN32 of AT90USB), QFN-32-RSN (32MA), VQFN-64-RGC-H (PS). 25 new quad / grid / PLCC objects, plus DRQFN-44-MC-44MC, DRQFN-64-MC-64MC and WLCSP-12-MC-12U-3 as `explicit` with ball-named pads (plan-36-J). QFN32 names two bodies, so `footprint-device.tsv` gives AT90PWM3B/316 QFN-32-MC-PWM7X7 and AT90PWM81/161 QFN-32-MC-PN.

Acceptance: every AVR quad/grid code has a footprint, and the new ones load.
  Check: `awk -F'\t' '$2~/^(quad|drqfn|plcc|grid)$/ && $6==""' planning/plan-36-data/package-codes.tsv | wc -l` → 0 except the RP2040 row; `fp-load.sh footprints-C.json footprints-D.json` → only `extra` / `unused` lines (est. 4 min).
  Result 2026-11-13: `0` (the RP2040 row is filled too, Phase 2); `42 new footprints: load ok`.
Commit: —

### Phase 2 — RP2040

- [x] `QFN-56-RP2040` from § 5.1.2 (R1: the pad size, pitch and exposed-pad
      size as printed), `pins` 57, centre pad = pin 57. Its row in
      `package-codes.tsv` names it.
      Pads 0.875 x 0.20 at span 6.875 (the 7.75 / 6.00 extents), pitch 0.40, 14 a side, centre 3.20; `pins` 57 with `slots` 1-56 and 57 → 0.
- [x] Load both files again.
      → `42 footprints appended` / `42 new footprints: load ok` (13 C + 29 D). `census/fp-pads-vs-pins.py footprints-C.json footprints-D.json pinouts.tsv --all-rows` → the only mismatches are the 12 ESP32 modules (E): every AVR and RP2040 pinout has a pad per designator.

Acceptance: all D codes have footprints, and they load.
  Check: `awk -F'\t' '$2~/^(quad|drqfn|plcc|grid)$/ && $6==""' planning/plan-36-data/package-codes.tsv | wc -l` → 0; `fp-load.sh footprints-C.json footprints-D.json` → only `extra` / `unused` lines (est. 3 min).
  Result 2026-11-13: `0`; `42 new footprints: load ok`.
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

- **2026-11-13: dual-row QFN and WLCSP pads need ball names, a missing prerequisite.** § 3 has DRQFN `explicit` pads "with designators as the datasheet numbers them". The format numbered explicit pads 1…N only, and `grid` cannot hold the 12U-3 WLCSP's staggered array. plan-36-J (new letter, before D's footprints) adds `"ball"` to explicit pads.
- **2026-11-13: one code, two packages.** QFN32 is 7 x 7 at 0.65 pitch for AT90PWM3B/316 and 5 x 5 for AT90PWM81/161 and AT90USB82/162, so the (code, pins) key is not enough. `footprint-device.tsv` (device, code, pins → footprint) carries the per-device footprints, and G reads it before `package-codes.tsv`.
- **2026-11-13: MR's drawing changed.** DS40002183B prints MR as C04-149 (7.15 mm exposed pad). The current editions print C04-154 (5.40 pad, land C 8.90, centre 5.50). All MR rows use the current C04-154.
- **2026-11-13: 12U-1 is drawn as 12U-3.** ATtiny20's ordering table prints 12U-1, and the packaging chapter draws 12U-3 ("Changed text in Section 7.1 from 12U-1 to 12U-3"). The footprint follows 12U-3.
- **2026-11-13: 64MC has no land pattern.** R4 applies to each row: outer pads extend 0.3 beyond the edge, inner pads (which cannot) are L + 0.1, centred.
- **2026-11-13: ATXMEGA8E5-MUR's code.** It is 32Z, not 32MA (A's `package-overrides.tsv`, with the ordering-table evidence).
- **2026-11-13: reuse across families.** A Microchip TQFP may reuse an LQFP or QFP footprint (same leads), and a QFN any QFN / VQFN / WQFN / UQFN / TQFN / HVQFN footprint, with the centre pad inside the 0.10 mm test too. Footprints with `slots` are not reused.

## Summary

D is the bulk of the land patterns: most parts are TQFP or QFN. Its risk
is the same transcription risk as C's, and the same loader and drawing
checks cover it. The RP2040 pad is the one place where a generic pattern
would be wrong.
