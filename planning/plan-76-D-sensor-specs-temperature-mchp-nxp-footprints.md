# plan-76-D: Temperature specs, Microchip and NXP; the temperature footprints; `fp-merge76.py`

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-C

D fills the `microchip` and `nxp` rows of `specs-temperature.tsv` by plan-76-C § 3's rules, then
draws every new footprint the temperature rows name, merges them into `footprints.json` with a new
merge tool, and regenerates the footprint golden. Microchip ≤ 181 and NXP ≤ 27 rows (plan-76-A
table). After D the temperature family is ready for M.

References:

- plan-76-C § 3 (pin roles, facts, footprint naming, `specs-check76.py`).
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `explicit`, common keys, rounded corners),
  § The `"footprint"` part key; `docs/backend.md` § goldens (`footprint_check --print`).
- Precedents: `planning/plan-74-D-ethernet-footprints-ic.md` (drawing IC footprints from package
  drawings, `fp-merge.py`); `planning/completed/plan-48-D-timer-opamp-footprints-leadless.md`.

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-C complete | `ls planning/completed/plan-76-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |
| Footprint golden at D's start | `wc -l < backend/tests/golden/footprints.txt` | 3993 at planning; D records F1 (plans 69–75 and 77–82 add footprints too) |
| A build to run `footprint_check` | `cmake --build /tmp/logic-p76 -j8 && ls /tmp/logic-p76/footprint_check` → exists | MET at planning (the A prerequisite build) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `microchip` and `nxp` row of `specs-temperature.tsv` passes
  `specs-check76.py --family temperature`.
- Every footprint id of a temperature row that is in `footprints-needed.txt` is drawn in
  `planning/plan-76-data/footprints-temperature.json`, merged into
  `schematical/schematical/Parts/footprints.json`, and its golden line added; no existing
  footprint line changes.

### Non-goals (explicit constraints)

- No change to `Parts/*.json` other than `footprints.json`; no backend or app change.
- No change to an existing footprint. A datasheet whose land pattern differs from an existing id
  gets its own id.

## 2. Current State

Parts seen at planning (the census page `b518`'s first 100 rows, product rows read at planning and
not kept; A's listing gives every row): Microchip 23 of its 181 — AT30TS74, AT30TS750A, EMC1438,
MCP9700/MCP9700A, MCP9701/MCP9701A (TO-92, SC-70, SOT-23), MCP9800, MCP9801, MCP9804, MCP9808,
TC74. NXP all 27 (`raw/t568.json`, rows kept): PCT2075 (D, DP, TP), LM75AD, LM75B (D, DP, TP),
P3T1750, P3T1755, P3T1084, P3T1035 (A–H), P3T2030 (A–H). The reference-board part `TCN75AVOA713`
is a Microchip temperature part (`planning/plan-76-data/plan-76.md`).

## 3. Design

### 3.1 Specs

plan-76-C § 3, unchanged.

### 3.2 Footprints (`footprints-temperature.json`)

- One footprint per new id of `footprints-needed.txt` whose row's family is temperature, drawn
  from the cited drawing's recommended land pattern (where the datasheet prints one), else from
  the package outline by the rules of `docs/parts-format.md` § Footprints.
- Shapes: `dual` for SOIC / MSOP / TSSOP / SOT / SC-70 / DFN / SON, with `"pad"` for an exposed pad;
  `explicit` for TO-92 / TO-220 and metal cans (TO-39, TO-46: round pads on the lead circle).
- `census/fp-merge76.py`: merges a `footprints-<family>.json` into `footprints.json` in id order,
  skipping (and printing) an id already present (A § Prerequisites: shared ids once); `--check`
  lists ids of the family's specs rows that are in neither file.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Microchip and NXP specs

- [ ] Fill every `microchip` and `nxp` row, its pinout files and its new footprint ids.

Acceptance: the whole temperature specs file passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family temperature` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — Temperature footprints

- [ ] `planning/plan-76-data/census/fp-merge76.py` per § 3.2.
- [ ] Draw `footprints-temperature.json`; merge it; regenerate
      `backend/tests/golden/footprints.txt` (`/tmp/logic-p76/footprint_check --print >
      backend/tests/golden/footprints.txt`) and hand-check each added line against its drawing.

Acceptance: every temperature footprint id exists, and the golden only grew.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check temperature` → `0 missing`; `cmake --build /tmp/logic-p76 -j8 && ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- D's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **footprints land per family, before the kind**: footprints are data (`footprints.json`
  and its golden), not backend code, so they need no plan-68 gate, and a family's footprints are
  checked against that family's drawings while they are fresh.
- Author: **one merge tool for every family** (`fp-merge76.py`), so the "shared ids once" rule is
  enforced in one place.

## Corrections

## Summary

D finishes the temperature specs and lands the temperature footprints, so M can generate the
temperature parts without drawing anything.
