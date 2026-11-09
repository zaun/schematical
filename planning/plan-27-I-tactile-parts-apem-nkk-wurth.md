# plan-27-I: Tactile switch parts — APEM, NKK and Würth

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-H

Runs plan-27-E's generator for APEM + NKK + Würth: the parts go into `Parts/switch.json`
(Normal Open and Normal Close devices under drawer `["Switch","Tactile"]`), the
footprints they use move from `footprints-C.json` into `Parts/footprints.json`, and
the goldens are regenerated. Design, device ids, package names and the golden
procedure are plan-27-E § 4; they are not repeated here.

Outcome: the library loads with one new part per `add` row of these makers; the
goldens grow by exactly those lines; no earlier golden line changes.

References:

- `planning/plan-27-E-tactile-parts-ck.md` § 4 (generator, footprints, goldens),
  `planning/plan-27-D-tactile-kind.md` § 4 (the keys),
  `planning/plan-27-A-tactile-data.md` § 3 (selection rules).
- `planning/plan-27-data/candidates.tsv`, `specs.tsv`, `footprints-C.json`.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-H is complete | `ls planning/completed/plan-27-H-*` → one file | NOT MET until H archives (2026-11-07) |
| Golden size at start | `wc -l < backend/tests/golden/parts.txt` → **P_I** | record at start (13158 after main's merge + the earlier letters' parts) |

If plan-27-H is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `add` row of APEM + NKK + Würth is a part of `Parts/switch.json`:
  `wc -l < backend/tests/golden/parts.txt` = P_I + N_apem + N_nkk + N_würth, with N_x the
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
| APEM candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 129 / 18 / 111 / 5 | `awk -F'\t' '$20==1 && index($1,"APEM")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| NKK candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 88 / 7 / 81 / 0 | `awk -F'\t' '$20==1 && index($1,"NKK")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| Würth candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 136 / 136 / 0 / 0 | `awk -F'\t' '$20==1 && index($1,"Würth")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| `add` rows of this letter's makers (known after B–C) | N_apem + N_nkk + N_würth | `awk -F'\t' '$18=="add" && index($1,"APEM")==1' planning/plan-27-data/candidates.tsv \| wc -l` for each maker key ('APEM' 'NKK' 'Würth') |
| Distinct (circuit, actuator, volts) among them | D_I | `awk -F'\t' '$18=="add" && (index($1,"APEM")==1) {print $6"|"$9"|"$13}' planning/plan-27-data/candidates.tsv \| sort -u \| wc -l` (per maker) |

### Verified properties

- The generator merges with the region it finds in `switch.json`, so the
  earlier letters' devices stay (plan-27-E § 4.1); a device shared with an
  earlier maker (same `id`) simply gains parts.

## 3. Design Overview

Same as plan-27-E § 3: one generator run per maker, loader and goldens as the
gate (new data, so byte-identity applies only to what the generator does not
own). Risk: a `specs.tsv` row that is wrong for a maker's series — here
APEM + NKK + Würth.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generate the parts and move the footprints

- [ ] `python3 -I planning/plan-27-data/gen-parts.py --maker 'APEM' --maker 'NKK' --maker 'Würth'`.
- [ ] `python3 -I planning/plan-27-data/append-footprints.py 'APEM' 'NKK' 'Würth'`.
- [ ] Load check: `cmake --build /tmp/logic-p27 -j8 --target library_check`
      then `/tmp/logic-p27/library_check`.

Acceptance: `Parts/switch.json` and `footprints.json` load; the generator is
idempotent for these makers.
  Check: `python3 -I planning/plan-27-data/gen-parts.py --maker 'APEM' --maker 'NKK' --maker 'Würth' --dry-run` → `no changes` and `/tmp/logic-p27/library_check` passes (est. 4 min; the loader is the smallest check that reads the new JSON).
Commit: —

### Phase 2 — Goldens and tests

- [ ] Goldens: plan-27-E § 4.3 (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`); review `git diff --numstat
      backend/tests/golden` (zero deletions).
- [ ] Tests: `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check|tactile_test'`.

Acceptance: goldens grew by exactly the new parts and footprints.
  Check: `echo $(( $(wc -l < backend/tests/golden/parts.txt) - P_I ))` equals N_apem + N_nkk + N_würth; `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed, 0 tests failed out of 4` (est. 8 min: full rebuild and the golden sweeps).
Commit: —

### Phase 3 — Prune unused datasheets and close the ledger

- [ ] `python3 -I planning/plan-27-data/census/place.py --prune
      planning/plan-27-data/candidates.tsv`: delete every PDF under
      `datasheets/switch/tactile/` that no `add` row's `datasheet_url` needs,
      and drop its line from `datasheets.tsv`. A PDF whose every part ended
      `skip` or `out` is listed in `datasheets/obsolete-not-included.md` only if
      the reason is `datasheet status`; for every other reason its rows are
      already in `datasheets/skipped-parts.md`.
- [ ] `datasheets/skipped-parts.md`: add the rows B and C decided `skip`
      (reason `not in the datasheet's ordering table`), grouped by series.
- [ ] § Summary: the totals — `add` rows per maker and overall, `skip` and `out`
      by reason, devices (`grep -c '"id": "tact_"'`), footprints added.

Acceptance: every PDF in the tree is used by an `add` row; every `skip` row is in `skipped-parts.md`.
  Check: `python3 -I planning/plan-27-data/census/place.py --verify planning/plan-27-data/candidates.tsv` → `0 missing, 0 unused` and `python3 -I planning/plan-27-data/census/decide.py --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 2 min).
Commit: —

## Validation Plan

- Tests: the four golden checks (Phase 2), `tactile_test`.
- Coverage check: `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` = the running total of `add` rows landed so far.
- Runtime proof: none beyond the goldens (the app proof is K).
- Doc sync: none (format and generator docs landed in D and E).
- Final gate (once, after the last phase): `ctest --test-dir /tmp/logic-p27 -j8` → `100% tests passed, 0 tests failed out of 28` (est. 6 min).

## Decisions

- Design, device identity, package names, rating source: plan-27-E § Decisions.
- Scope and drawer: owner, 2026-11-07 (plan-27-A § Decisions).

## Corrections

1. 2026-11-08 — after main's merge (plan-27-A Correction 3) ctest runs **28** programs (T0 = 27 + `tactile_test`), not 25, and the goldens start at P0 = R0 = **13158**, F0 = **508**; the gates above say so. Parts also carry `manufacturer` and `datasheet` (plan-27-E Correction 1), which the generator writes; nothing else changes here.
2. 2026-11-08 — **two generator runs.** C's readers finished per maker, and a
   maker's parts need only its own merged rows, so APEM and NKK ran with the
   first finished makers of G, H and I (`gen-parts.py --maker Alps --maker 'Same Sky'
   --maker NKK --maker APEM`: `50 tactile devices, 1498 tactile parts written; 104
   datasheets.json entries added`; `49 footprints appended`; goldens `545 0` ×2,
   `49 0`; ctest 28/28) and Würth runs when its reader is merged; the boxes
   are ticked after that second run, whose counts close the acceptance.

## Summary

Risk: pinout and footprint correctness of the APEM + NKK + Würth series, set in
plan-27-C; this
letter only joins and generates. Untouched: other makers' parts, the DIP
devices, the backend, the app.
