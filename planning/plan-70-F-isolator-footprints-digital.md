# plan-70-F: Footprints: digital-isolator packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-E

F gives every `F` line of `footprints-needed.txt` a footprint: the digital
isolators' SOIC (narrow, 5.3 mm and wide body, 8 to 20 pins), SSOP, QSOP and
leadless packages (DFN, QFN, LGA). It uses E's tools and map, and adds its
new objects to `planning/plan-70-data/footprints-F.json`.

References:

- plan-70-D (`footprints-needed.txt`), plan-70-E (the tools, the map, § 3).
- `planning/completed/plan-48-D-timer-opamp-footprints-leadless.md` (the
  exposed-pad and ball rules) and plan-48-C § 3.1.
- `docs/parts-format.md` § Footprints (`centre`, `slots`, `grid`).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-E complete | `ls planning/completed/plan-70-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25: E not started) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-70-data/census/fp-check.py F` →
  `<n> lines, 0 without a footprint, 0 spec rows whose pin designators differ`.
- `fp-load.sh planning/plan-70-data/footprints-E.json planning/plan-70-data/footprints-F.json`
  → `<k> new footprints: load ok`. No id is in both files.

### Non-goals (explicit constraints)

- As plan-70-E § 1: nothing existing changes; `Parts/footprints.json` is
  not touched; no unused footprint.

## 2. Current State

The reuse candidates (`python3 -I` over `footprints.json`, 2026-11-25):
`SOIC-8` (8, 1.27, span 5.4, 3.9 × 4.9), `SOIC-16` (16, 1.27, 5.4, 3.9 ×
9.91), `SOIC-16W` (16, 1.27, span 9.3, 7.5 × 10.3: TI DW), the 5.3 mm
`SOIC-8W-MC-8S2` and `SOIC-8W-ONSEMI-751BE` (span 7.61 / 7.57), `SSOP-16`
and `SSOP-20` (0.65, span 7, 5.3 mm body), `QSOP-16` (0.635, span 5.31),
and `SOIC-8-ADI-S8` (ADI R-8, plan-48). The 8-pin wide-body SOIC (ADI RI-8,
TI DWV: 7.5 mm body, 11.5 mm span for 8 mm creepage), ADI's RW-16 / RI-16,
and Skyworks' and TI's 16-pin wide-body variants with their own land
patterns are drawn when no existing pattern matches within 0.05 mm.

## 3. Design

plan-70-E § 3 and plan-48-C § 3.1, unchanged. Leadless and exposed-pad
packages follow plan-48-D. A numbered exposed pad is a pin (`slots` with
`0` on `centre`); an unnumbered one is `centre` with no pin.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: SOIC lines

- [ ] Every 8-, 14-, 16- and 20-pin SOIC line (3.9 mm, 5.3 mm, 7.5 mm
      bodies; standard and long-creepage spans), per maker: reuse check
      (`SOIC-8`, `SOIC-8-ADI-S8`, `SOIC-16`, `SOIC-16W`, `SOIC-8W-*`) or
      draw; map rows.

Acceptance: every SOIC `F` line is mapped and loads.
  Check: `fp-check.py F --outline SOIC` → `0 without a footprint`;
  `fp-load.sh footprints-E.json footprints-F.json` → `load ok`. Takes about
  4 min.
Commit:

### Phase 2: SSOP, QSOP and leadless lines

- [ ] Every SSOP, QSOP, DFN, QFN and LGA line: reuse check (`SSOP-16`,
      `SSOP-20`, `QSOP-16`, and the plan-48-D leadless ids when present) or
      draw; map rows.

Acceptance: every `F` line is mapped; every new object loads.
  Check: `python3 -I planning/plan-70-data/census/fp-check.py F` →
  `<n> lines, 0 without a footprint, 0 spec rows whose pin designators differ`;
  `fp-load.sh footprints-E.json footprints-F.json` → `<k> new footprints:
  load ok`. Takes about 4 min.
Commit:

## Validation Plan

- Tests: `fp-check.py`, `fp-load.sh`.
- Coverage check: the object counts of `footprints-F.json` are recorded in
  § Corrections.
- Doc sync: none.
- Final gate: plan-70-J.

## Decisions

- Author: **creepage variants are separate footprints.** A long-creepage
  SOIC's span differs from the standard one, and a reuse would put pads
  where the leads are not.

## Corrections

## Summary

Most digital-isolator packages are standard SOICs, and reuse covers them.
The new work is the long-creepage wide bodies.
