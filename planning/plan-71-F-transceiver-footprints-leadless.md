# plan-71-F: Line transceivers — footprints for leadless packages (DFN, SON, QFN, WQFN, CSP)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-E

F gives every `F` (leadless) line of `footprints-needed.txt` a footprint,
with E's tools and rules. It writes `planning/plan-71-data/footprints-F.json`
and adds its rows to `footprint-map.tsv`.

References:

- plan-71-E § 3 (rules and tools); `planning/completed/plan-48-D-timer-opamp-footprints-leadless.md`
  (the leadless precedent: exposed pads, `slots`, pull-back pads, ball grids).
- `docs/parts-format.md` § Footprints (`quad`, `grid`, `centre`, `slots`).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-E complete | `ls planning/completed/plan-71-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25: E not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `F` line of `footprints-needed.txt` is mapped:
  `python3 -I planning/plan-71-data/census/fp-check.py F` → `<n> lines, 0 without a footprint`.
- `footprints-E.json` and `footprints-F.json` load together through
  `footprint_check` with no error.

### Non-goals (explicit constraints)

- As plan-71-E § 1.

## 2. Current State

253 leadless footprints exist (`cut -d'|' -f1 backend/tests/golden/footprints.txt | grep -cE '^(VSON|SON|WSON|DFN|TDFN|VDFN|UDFN|WDFN|VQFN|WQFN|QFN|UQFN|DSBGA|WLCSP|LFCSP)'`
→ 253 at 63e2f7d2), among them `SON-8-DRB` (TI DRB, the TCAN / TLIN
8-VSON), `VSON-10-DRC`, `VSON-14-DMD`, `VQFN-16`, `VQFN-20`, `QFN-24-4X4`,
`QFN-32-5X5`. At planning, the four protocols' listings (all makers) show 44
leadless Package / Case values on 484 part listings. The most common are
8-VDFN (CAN 85, LIN 26), 14-VDFN (CAN 51, LIN 14), 8-WFDFN (RS-485 35, CAN
12) and 10-WFDFN (RS-485 33).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — DFN and SON

- [ ] Every DFN / SON / VSON / WSON / TDFN line: reuse check or draw (the
      exposed pad as a numbered pin when the pinout numbers it, else
      `centre`); map rows.

Acceptance: these lines are mapped and the objects load.
  Check: `python3 -I planning/plan-71-data/census/fp-check.py F | grep -E 'DFN|SON' | grep -c missing` → 0; `planning/plan-71-data/census/fp-load.sh planning/plan-71-data/footprints-E.json planning/plan-71-data/footprints-F.json` → `load ok` (est. 4 min).
  Result:
Commit:

### Phase 2 — QFN, WQFN, CSP, BGA

- [ ] Every QFN / VQFN / WQFN line and any ball-grid line (`grid` style;
      ball names from the pinout): reuse check or draw; map rows.

Acceptance: every `F` line is mapped, and both files load.
  Check: `python3 -I planning/plan-71-data/census/fp-check.py F` → `<n> lines, 0 without a footprint`, `0 spec rows whose pin designators differ from their footprint`; `fp-load.sh footprints-E.json footprints-F.json` → `<n> new footprints: load ok` (est. 4 min).
  Result:
Commit:

## Validation Plan

- As plan-71-E. Final gate: plan-71-J.

## Corrections

(none yet)
