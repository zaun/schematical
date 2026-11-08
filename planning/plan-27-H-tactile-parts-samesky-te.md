# plan-27-H: Tactile switch parts — Same Sky and TE

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-G

Runs plan-27-E's generator for Same Sky + TE ALCOSWITCH: the parts go into `Parts/switch.json`
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
| plan-27-G is complete | `ls planning/completed/plan-27-G-*` → one file | NOT MET until G archives (2026-11-07) |
| Golden size at start | `wc -l < backend/tests/golden/parts.txt` → **P_H** | record at start (11126 + the earlier letters' parts) |

If plan-27-G is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `add` row of Same Sky + TE ALCOSWITCH is a part of `Parts/switch.json`:
  `wc -l < backend/tests/golden/parts.txt` = P_H + N_same + N_te, with N_x the
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
| Same Sky candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 379 / 11 / 368 / 2 | `awk -F'\t' '$20==1 && index($1,"Same Sky")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| TE ALCOSWITCH candidate rows with a PDF / verbatim / `pending` in B–C / `check` rows | 176 / 117 / 59 / 1 | `awk -F'\t' '$20==1 && index($1,"TE Conn")==1' planning/plan-27-data/candidates.tsv \| wc -l` (`$21==1`, `$21==0`; check rows: `$18=="check"` before A) |
| SPST-NC row of this letter | 1 (`TS10-63-26-BE-250-SMT-TR`: `pdf`=1, `hit`=1, so `add` from A on) | `awk -F'\t' '$6=="SPST-NC" && index($1,"Same Sky")==1' planning/plan-27-data/candidates.tsv` |
| `add` rows of this letter's makers (known after B–C) | N_same + N_te | `awk -F'\t' '$18=="add" && index($1,"Same Sky")==1' planning/plan-27-data/candidates.tsv \| wc -l` for each maker key ('Same Sky' 'TE Conn') |
| Distinct (circuit, actuator, volts) among them | D_H | `awk -F'\t' '$18=="add" && (index($1,"Same Sky")==1) {print $6"|"$9"|"$13}' planning/plan-27-data/candidates.tsv \| sort -u \| wc -l` (per maker) |

### Verified properties

- The generator merges with the region it finds in `switch.json`, so the
  earlier letters' devices stay (plan-27-E § 4.1); a device shared with an
  earlier maker (same `id`) simply gains parts.

## 3. Design Overview

Same as plan-27-E § 3: one generator run per maker, loader and goldens as the
gate (new data, so byte-identity applies only to what the generator does not
own). Risk: a `specs.tsv` row that is wrong for a maker's series — here
Same Sky + TE ALCOSWITCH.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generate the parts and move the footprints

- [ ] `python3 -I planning/plan-27-data/gen-parts.py --maker 'Same Sky' --maker 'TE Conn'`.
- [ ] `python3 -I planning/plan-27-data/append-footprints.py 'Same Sky' 'TE Conn'`.
- [ ] Load check: `cmake --build /tmp/logic-p27 -j8 --target library_check`
      then `/tmp/logic-p27/library_check`.

Acceptance: `Parts/switch.json` and `footprints.json` load; the generator is
idempotent for these makers.
  Check: `python3 -I planning/plan-27-data/gen-parts.py --maker 'Same Sky' --maker 'TE Conn' --dry-run` → `no changes` and `/tmp/logic-p27/library_check` passes (est. 4 min; the loader is the smallest check that reads the new JSON).
Commit: —

### Phase 2 — Goldens and tests

- [ ] Goldens: plan-27-E § 4.3 (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`); review `git diff --numstat
      backend/tests/golden` (zero deletions).
- [ ] Tests: `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check|tactile_test'`.

Acceptance: goldens grew by exactly the new parts and footprints.
  Check: `echo $(( $(wc -l < backend/tests/golden/parts.txt) - P_H ))` equals N_same + N_te; `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed, 0 tests failed out of 4` (est. 8 min: full rebuild and the golden sweeps).
Commit: —

## Validation Plan

- Tests: the four golden checks (Phase 2), `tactile_test`.
- Coverage check: `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` = the running total of `add` rows landed so far.
- Runtime proof: none beyond the goldens (the app proof is K).
- Doc sync: none (format and generator docs landed in D and E).
- Final gate (once, after the last phase): `ctest --test-dir /tmp/logic-p27 -j8` → `100% tests passed, 0 tests failed out of 25` (est. 6 min).

## Decisions

- Design, device identity, package names, rating source: plan-27-E § Decisions.
- Scope and drawer: owner, 2026-11-07 (plan-27-A § Decisions).

## Corrections

<!-- filled in during execution -->

## Summary

Risk: pinout and footprint correctness of the Same Sky + TE ALCOSWITCH series, set in
plan-27-C; this
letter only joins and generates. Untouched: other makers' parts, the DIP
devices, the backend, the app.
