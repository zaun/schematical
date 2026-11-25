# plan-55-G: Footprints — Microchip, Diodes, onsemi, Analog Devices and Monolithic Power Systems packages

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-F

G draws the land patterns of the packages of the other five companies of the final pick (the `G`
lines of `footprints-needed.txt`: at most 69 DigiKey `Package / Case` labels, plan-55-A § 2), by the
method and with the tools of plan-55-F, into `planning/plan-55-data/footprints-G.json` and G's rows
of `footprint-map.tsv`. The behavioral outcome: every `G` line has a footprint id in
`footprint-map.tsv`, and the new objects load through `footprint_check`.

References:

- `planning/completed/plan-55-F-powerswitch-footprints-ti.md` § 3 (ids, reuse, the map, the two classes) and its tools;
  `docs/parts-format.md` § Footprints; `planning/plan-55-data/footprints-needed.txt`.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-F complete | `ls planning/completed/plan-55-F-*.md` → exists | MET (2026-11-23: archived at 49008af1) |
| `footprints-needed.txt` has its `G` lines | `grep -c '^G' planning/plan-55-data/footprints-needed.txt` → E's recorded `G` count | MET (2026-11-23: 69) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-55-data/census/fp-check.py G` prints `<n> lines, 0 without a footprint`,
  `<n>` = `awk -F'\t' '$1=="G"' planning/plan-55-data/footprints-needed.txt | wc -l`.
- `fp-load.sh planning/plan-55-data/footprints-G.json` prints `<k> new footprints: load ok`.
- `footprint-map.tsv` has, for every `G` line, the footprint id and the pinouts it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, any `Parts/*.json`, `backend/` or the app.
- No footprint F already drew: a `G` line whose pads equal an `F` footprint's takes it
  (`source` `reuse`).

## 2. Current State

F's footprints are in `footprints-F.json` and the library's in `Parts/footprints.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `Package / Case` labels at most: Microchip 9 + 17, Diodes 17, onsemi 11, Analog Devices 13, Monolithic Power Systems 2 | 69 | `grep '^pick' planning/plan-55-data/families.out` → `packages(case)`: ideal ADI 13 and MPS 2, load onsemi 11 and Microchip 9, limited Microchip 17 and Diodes 17 |
| `G` lines after E | the distinct (company, package, supplier package) of the other companies' `add` rows | `awk -F'\t' '$1=="G"' planning/plan-55-data/footprints-needed.txt \| wc -l` (recorded here when G starts: **69**; 69 units, the pin count splits none) |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The reuse pass

- [x] Map every `G` line to a library footprint or an `F` footprint whose pads equal its drawing's. Observed: 26 of the 78 `G` map rows are `reuse`.

Acceptance: every reusable line is mapped.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py G` → prints the count of lines still without a footprint (est. 1 min).
Commit: 257917ba

### Phase 2 — Leaded and through-hole packages

- [x] Draw each remaining leaded `G` line into `footprints-G.json` (F § 3.2's class, § Footprints rules). Observed: the readers' packets f09 to f19 (plan-55-F Correction 1) drew 43 new footprints (leaded and leadless together); `fp-load.sh planning/plan-55-data/footprints-F.json planning/plan-55-data/footprints-G.json` → `70 new footprints: load ok`.

Acceptance: every leaded `G` line has a footprint and the objects load.
  Check: `planning/plan-55-data/census/fp-load.sh planning/plan-55-data/footprints-G.json` → `<k> new footprints: load ok` (est. 1 min).
Commit: 257917ba

### Phase 3 — Leadless and ball-grid packages

- [x] The same for the leadless `G` lines. Observed: `fp-check.py G` → `69 lines, 0 without a footprint`.

Acceptance: every `G` line has a footprint and the objects load.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py G` → `<n> lines, 0 without a footprint`; `fp-load.sh planning/plan-55-data/footprints-G.json` → `<k> new footprints: load ok` (est. 2 min).
Commit: 257917ba

## Validation Plan

- Tests: `fp-check.py` and `fp-load.sh`; no backend or app test is touched.
- Coverage check: `fp-check.py G` counts every `G` line once.
- Runtime proof: none (plan-55-J's harness draws a board).
- Doc sync: none.
- Final gate: plan-55-K.

## Decisions

- Author: **reuse beats drawing**, across F's footprints too.

## Corrections

1. **The method is F's** (plan-55-F Correction 1): reader packets f09 to f19 by (company, package, supplier package, pin count), `fp-combine.py G`; F's footprints were visible to the readers (`fp-find.py`) and reused where the pads agree.
2. **Numbered pads that are pins** (plan-55-C Correction 6) are `centre` pads with `slots` 0: `SOIC-8-EP-DIODES-SO-8EP-TYPE-E` (pin 9), `SOIC-14-EP-DIODES-SO-14EP` (pin 15), `DFN-12-DIODES-VDFN3030-12-B` (pin 13), `DFNW-6-ONSEMI-507AF`, `WDFN-6-ONSEMI-511CY` (pin 7), `DFN-8-ONSEMI-506CC` (pin 9), `DFN-12-ONSEMI-506CD` (pin 13).
3. **For a reviewer** (from the readers' reports; all load, none overlaps): the Diodes `U-DFN2020-6` T-shaped exposed pad is taken to join pin 2 (GND) and is drawn as pin 2's two touching pads; `VQFN-17-DIODES-V-QFN4040-17` splits the datasheet's joined IN and OUT lands into three pads of 0.42 mm each (same copper); `DFN-16-ADI-DHC` draws the notched corner pads plain (the drawing gives no size); the MAX40200 bump size (0.22 mm) is assumed from the MAX40203's drawing; the 4-ball WLP bodies reuse `DSBGA-4-YKE`'s 0.65 mm body (pads and pitch match); the old Micrel PDFs print an outline only, so `SOIC-8-MCHP-SOICN-8LD`, `MSOP-8-MCHP-MSOP-8LD-PL-1`, `SOT23-5-MCHP-5LD-PL-1`, `TSOT-6-MCHP-YD6` and `SC70-6-MCHP-C6` apply Microchip's land pattern of the identical outline (R2 would give larger pads); `MLF-6-MCHP-2X2-ML` is drawn by R4 where the library's `DFN-6-MCHP-2X2-ML` (0.40 x 0.26 pads, span 1.80) is probably the real pattern; `TMLF-6-MCHP-MLF1616D` reads the Micrel drawing's 1.8 as the outer extent; `TSSOP-14-MCHP-TS`, `SOIC-16-MCHP-M` and `TSSOP-16-MCHP-TS` are R2 from the outline only; the 8-SO (4.995 mm span, 1.505 pads) pattern of the newer Diodes datasheets is a separate footprint `SOIC-8-DIODES-SO-8-L1.505` reached by per-PDF map rows; `UDFN-8-DIODES-U-DFN3030-8E-2023` is the 2023 revision of the Type E land pattern for `AP2162A_72A.pdf` alone; `WLCSP-6-ONSEMI-567KB` is `explicit` because its balls run A to C along the long side.

## Summary

The same slow, exact work as F, for five companies; no part depends on it until H and I merge it.
