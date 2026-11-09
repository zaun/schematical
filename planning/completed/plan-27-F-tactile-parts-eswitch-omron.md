# plan-27-F: Tactile switch parts — E-Switch and Omron

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-E

Runs plan-27-E's generator for E-Switch + Omron (DigiKey: Aratas): the parts go into `Parts/switch.json`
(Normal Open and Normal Close devices under drawer `["Switch","Tactile"]`), the
footprints they use move from `footprints-B.json` into `Parts/footprints.json`, and
the goldens are regenerated. Design, device ids, package names and the golden
procedure are plan-27-E § 4; they are not repeated here.

Outcome: the library loads with one new part per `add` row of these makers; the
goldens grow by exactly those lines; no earlier golden line changes.

References:

- `planning/plan-27-E-tactile-parts-ck.md` § 4 (generator, footprints, goldens),
  `planning/plan-27-D-tactile-kind.md` § 4 (the keys),
  `planning/plan-27-A-tactile-data.md` § 3 (selection rules).
- `planning/plan-27-data/candidates.tsv`, `specs.tsv`, `footprints-B.json`.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-E is complete | `ls planning/completed/plan-27-E-*` → one file | MET 2026-11-08 (1f61382); plan-27-B archived too (a03d720) |
| Golden size at start | `wc -l < backend/tests/golden/parts.txt` → **P_F** | **P_F = 13646** (2026-11-08) |

If plan-27-E is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `add` row of E-Switch + Omron (DigiKey: Aratas) is a part of `Parts/switch.json`:
  `wc -l < backend/tests/golden/parts.txt` = P_F + N_e-switch + N_aratas, with N_x the
  maker's `add` count (§ 2).
- Footprints those parts name that `Parts/footprints.json` lacks are appended
  (moot per id when `grep -c '"id":"<ID>"' schematical/schematical/Parts/footprints.json` → 1).
- `ctest` passes; goldens change by added lines only.

### Non-goals (explicit constraints)

- Parts of other makers, the DIP devices, the backend and the app are untouched.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| E-Switch candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 330 / 14 / 316 / 0 | `awk -F'\t' '$20==1 && index($1,"E-Switch")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| Omron (DigiKey: Aratas) candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 148 / 145 / 3 / 5 | `awk -F'\t' '$20==1 && index($1,"Aratas")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| `add` rows of this letter's makers (known after B–C) | N_e-switch + N_aratas | `awk -F'\t' '$18=="add" && index($1,"E-Switch")==1' planning/plan-27-data/candidates.tsv \| wc -l` for each maker key ('E-Switch' 'Aratas') |
| Distinct (circuit, actuator, volts) among them | D_F | `awk -F'\t' '$18=="add" && (index($1,"E-Switch")==1) {print $6"|"$9"|"$13}' planning/plan-27-data/candidates.tsv \| sort -u \| wc -l` (per maker) |

### Verified properties

- The generator merges with the region it finds in `switch.json`, so the
  earlier letters' devices stay (plan-27-E § 4.1); a device shared with an
  earlier maker (same `id`) simply gains parts.

## 3. Design Overview

Same as plan-27-E § 3: one generator run per maker, loader and goldens as the
gate (new data, so byte-identity applies only to what the generator does not
own). Risk: a `specs.tsv` row that is wrong for a maker's series — here
E-Switch + Omron (DigiKey: Aratas).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generate the parts and move the footprints

- [x] `python3 -I planning/plan-27-data/gen-parts.py --maker 'E-Switch' --maker 'Aratas'` → `44 tactile devices, 953 tactile parts written; 82 datasheets.json entries added` (C&K's kept).
- [x] `python3 -I planning/plan-27-data/append-footprints.py 'E-Switch' 'Aratas'` → `77 footprints appended`.
- [x] Load check: `cmake --build /tmp/logic-p27 -j8 --target library_check`
      then `/tmp/logic-p27/library_check` → `library_check: all passed`.

Acceptance: `Parts/switch.json` and `footprints.json` load; the generator is
idempotent for these makers.
  Check: `python3 -I planning/plan-27-data/gen-parts.py --maker 'E-Switch' --maker 'Aratas' --dry-run` → `no changes` and `/tmp/logic-p27/library_check` passes (est. 4 min; the loader is the smallest check that reads the new JSON).
  Result 2026-11-08: `no changes`; `library_check: all passed`.
Commit: 6880fdf

### Phase 2 — Goldens and tests

- [x] Goldens: plan-27-E § 4.3 (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`); review `git diff --numstat
      backend/tests/golden` (zero deletions). → `77 0 footprints.txt`, `465 0
      parts.txt`, `465 0 traces.txt`.
- [x] Tests: `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check|tactile_test'` → `100% tests passed, 0 tests failed out of 5`.

Acceptance: goldens grew by exactly the new parts and footprints.
  Check: `echo $(( $(wc -l < backend/tests/golden/parts.txt) - P_F ))` equals N_e-switch + N_aratas; `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed, 0 tests failed out of 4` (est. 8 min: full rebuild and the golden sweeps).
  Result 2026-11-08: 14111 − 13646 = 465 = 321 (E-Switch) + 144 (Aratas); the 5-test run includes the four.
Commit: 6880fdf

## Validation Plan

- Tests: the four golden checks (Phase 2), `tactile_test`.
- Coverage check: `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` = the running total of `add` rows landed so far.
- Runtime proof: none beyond the goldens (the app proof is K).
- Doc sync: none (format and generator docs landed in D and E).
- Final gate (once, after the last phase): `ctest --test-dir /tmp/logic-p27 -j8` → `100% tests passed, 0 tests failed out of 28` (est. 6 min). Result 2026-11-08: `100% tests passed, 0 tests failed out of 28`; `grep -c 'Switch/Tactile'` → 953.

## Decisions

- Design, device identity, package names, rating source: plan-27-E § Decisions.
- Scope and drawer: owner, 2026-11-07 (plan-27-A § Decisions).

## Corrections

1. 2026-11-08 — after main's merge (plan-27-A Correction 3) ctest runs **28** programs (T0 = 27 + `tactile_test`), not 25, and the goldens start at P0 = R0 = **13158**, F0 = **508**; the gates above say so. Parts also carry `manufacturer` and `datasheet` (plan-27-E Correction 1), which the generator writes; nothing else changes here.

## Summary

Risk: pinout and footprint correctness of the E-Switch + Omron (DigiKey: Aratas) series, set in
plan-27-B; this
letter only joins and generates. Untouched: other makers' parts, the DIP
devices, the backend, the app.
