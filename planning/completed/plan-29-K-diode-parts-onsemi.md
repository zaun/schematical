# plan-29-K: Parts — onsemi (all diode categories)

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-J

K runs plan-29-I's `gen-parts.py` for every onsemi diode and regenerates the
goldens. Everything about devices, parts, datasheet entries and footprint
merging is plan-29-I § 3.

References: plan-29-I § 3, plan-29-H § 5.1, `docs/backend.md` (goldens).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-J complete | `ls planning/completed/plan-29-J-*.md` → exists | MET (2026-11-09) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/diode.json` gains one part per `add` row of `maker` = onsemi (all five categories); that
  count is **N_K**.
- `parts.txt` and `traces.txt` grow by exactly N_K lines each; no existing
  golden line changes (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0).
- `ctest` passes (T0 + 1 programs).

Planning-time population (2026-11-08), before the datasheet rules:

| What | Count | Command |
|---|---|---|
| onsemi singles / Zeners / Zener arrays / arrays / bridges, in stock, Active | 1113 / 1547 / 27 / 259 / 101 | `scripts/digikey.js --category <280\|287\|295\|286\|299> --manufacturer 488 --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` |
| …`out` by DigiKey data | SiC 94 / 0 / 0 / 12 / QC 25 | same calls, `Technology` and `Mounting Type` facets |

### Non-goals (explicit constraints)

- No change to any part outside the scope, nor to the generator's output for
  earlier scopes (re-running it leaves their lines identical).
- No app change.

## 2. Current State

- onsemi's single-diode listing hit the 300-result cap at planning time; plan-29-A Phase 1 re-lists that range prefix by prefix, so `candidates.tsv` has every onsemi row A could find (A § Corrections records any shortfall against DigiKey's 1113).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generate

- [x] `python3 -I planning/plan-29-data/census/gen-parts.py --maker onsemi --category single,zener,zener-array,array,bridge --dry-run`
      → `N_K parts, 0 without spec, 0 unknown footprint`; then without
      `--dry-run`. Result 2026-11-09: `2492 parts, 0 without spec, 0 unknown
      footprint; 26 devices, 70 new footprints, 504 new datasheets`
      (dry run and real run alike); N_K = 2492; `library_check` exit 0.
- [x] ~~Shared footprint ids: every `reused <id>` line the run prints is a
      footprint already shipped; nothing to add for it~~ — moot by the
      printout: 6 `reused` lines, each a footprint already in
      `Parts/footprints.json` (gen-parts.py maps a ready object identical to a
      shipped one onto the shipped id).

Acceptance: the parts load.
  Check: `cmake --build /tmp/logic-p29 -j8 && /tmp/logic-p29/library_check` → pass (est. 3 min).
Commit: 3194c1b

### Phase 2 — Goldens

- [x] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`
      (`docs/backend.md`) and review the diff: only added lines, all diode
      parts or new footprints. Result: removed lines 0; added parts.txt lines
      2492 (all `|diode|`, 0 others); footprints.txt +70; `ctest` →
      `100% tests passed, 0 tests failed out of 30`.

Acceptance: tests pass and the goldens only grew by the scope.
  Check: `ctest --test-dir /tmp/logic-p29` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^+[^+].*|diode|'` → N_K; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
Commit: 3194c1b

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `library_check`.
- Coverage check: the Phase 2 count equals the scope's `add` count in
  `candidates.tsv` (`awk -F'\t' '$2=="onsemi" && $24=="add"' planning/plan-29-data/candidates.tsv | wc -l`).
- Runtime proof: `trace_check --verbose <first part of the scope>` prints its
  net changes (as plan-29-I § Validation).
- Doc sync: none.
- Final gate: the Phase 2 check (est. 6 min).

## Decisions

- As plan-29-I § Decisions.

## Corrections

- **N_K = 2492** (2026-11-09; § 1's planning population replaced by A's
  decisions and B–G's keys, plan-29-B § Corrections).

## Summary

K adds all of onsemi in one run; its listing is the one the cap touched, so its population is the least certain until A has re-checked it part by part.
