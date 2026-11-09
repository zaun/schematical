# plan-29-J: Parts — Vishay Zeners, diode arrays and bridge rectifiers

Last updated: 2026-11-08
Effort: large (3h–1d)
Depends on: plan-29-I

J runs plan-29-I's `gen-parts.py` for the Vishay Zeners, Zener arrays, diode arrays and bridge rectifiers and regenerates the
goldens. Everything about devices, parts, datasheet entries and footprint
merging is plan-29-I § 3.

References: plan-29-I § 3, plan-29-H § 5.1, `docs/backend.md` (goldens).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-I complete | `ls planning/completed/plan-29-I-*.md` → exists | NOT MET (2026-11-08: plan written) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/diode.json` gains one part per `add` row of `maker` = Vishay General Semiconductor - Diodes Division and `category` in `zener`, `zener-array`, `array`, `bridge`; that
  count is **N_J**.
- `parts.txt` and `traces.txt` grow by exactly N_J lines each; no existing
  golden line changes (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0).
- `ctest` passes (T0 + 1 programs).

Planning-time population (2026-11-08), before the datasheet rules:

| What | Count | Command |
|---|---|---|
| Vishay single Zeners / Zener arrays / diode arrays / bridges, in stock, Active | 2242 / 39 / 693 / 382 | `scripts/digikey.js --category <287\|295\|286\|299> --manufacturer 112 --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` |
| …of those, `out` by DigiKey data (chassis / QC terminal / SiC) | 0 / 0 / 93 / 93 | same calls: arrays Chassis Mount 76 + SiC 17; bridges Chassis 27 + QC 66 |

### Non-goals (explicit constraints)

- No change to any part outside the scope, nor to the generator's output for
  earlier scopes (re-running it leaves their lines identical).
- No app change.

## 2. Current State

- After I, `Parts/diode.json` holds the Vishay single diodes and the generator exists.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generate

- [ ] `python3 -I planning/plan-29-data/census/gen-parts.py --maker Vishay --category zener,zener-array,array,bridge --dry-run`
      → `N_J parts, 0 without spec, 0 unknown footprint`; then without
      `--dry-run`.
- [ ] Shared footprint ids: every `reused <id>` line the run prints is a
      footprint already shipped; nothing to add for it (moot by the printout).

Acceptance: the parts load.
  Check: `cmake --build /tmp/logic-p29 -j8 && /tmp/logic-p29/library_check` → pass (est. 3 min).
Commit: —

### Phase 2 — Goldens

- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`
      (`docs/backend.md`) and review the diff: only added lines, all diode
      parts or new footprints.

Acceptance: tests pass and the goldens only grew by the scope.
  Check: `ctest --test-dir /tmp/logic-p29` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^+[^+].*|diode|'` → N_J; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
Commit: —

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `library_check`.
- Coverage check: the Phase 2 count equals the scope's `add` count in
  `candidates.tsv` (`awk -F'\t' '$2 ~ /^Vishay/ && $1 != "single" && $24=="add"' planning/plan-29-data/candidates.tsv | wc -l`).
- Runtime proof: `trace_check --verbose <first part of the scope>` prints its
  net changes (as plan-29-I § Validation).
- Doc sync: none.
- Final gate: the Phase 2 check (est. 6 min).

## Decisions

- As plan-29-I § Decisions.

## Corrections

## Summary

J is the generator run over four categories at once; the new device shapes (arrays, bridges) are the risk, checked by `library_check` loading them and by the pin maps of plan-29-E–G.
