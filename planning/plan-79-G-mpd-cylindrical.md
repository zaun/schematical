# plan-79-G: MPD cylindrical and 9 V holders, clips and contacts

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-F

G adds MPD's remaining `in` rows (every function but coin cells), with D's tools and rules and F's
reading method for outlined drawings. After G, MPD has every non-skipped PCB-mount part.

References: plan-79-D § 3; plan-79-E § 2 (multi-cell wiring, 9 V contacts, clips);
plan-79-F § 2 (renders).

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-F complete | `ls planning/completed/plan-79-F-*.md` → exists | NOT MET (2026-11-25: F not started) |
| Goldens at G's start | `wc -l < backend/tests/golden/parts.txt`, `… footprints.txt` | G records its own |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every MPD non-coin row is `add` or `skip`; their footprints, devices, parts and datasheet entries.
- MPD ends with the number `build.py --expect mpd` prints.

### Non-goals

- No change to D's tools or rules.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs and footprints

- [ ] Renders, `specs.tsv` rows and `skip` rows for MPD non-coin parts.
- [ ] Add to `footprints-mpd.json`; merge; regenerate `footprints.txt`.

Acceptance: the new footprints expand and match their drawings.
  Check: `/tmp/logic-p79/footprint_check` passes after regenerating; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0; the plan-79-D Phase 2 hand check on one cylindrical holder and one 9 V part (est. 10 min).
  Result:
Commit:

### Phase 2 — Parts

- [ ] `datasheets.json`; `gen-battery.py --write`; regenerate `parts.txt`, `traces.txt`, `erc_pins.txt`.

Acceptance: the parts load and MPD's count is complete.
  Check: `ctest --test-dir /tmp/logic-p79 -R 'listing_check|trace_check|erc_pins_check|footprint_check|library_check'` → `100% tests passed`; `grep -c 'mfr=MPD' backend/tests/golden/parts.txt` → the number `python3 -I planning/plan-79-data/census/build.py --expect mpd` prints (est. 5 min).
  Result:
Commit:

## Validation Plan

- Phase checks above. Final gate: plan-79-J.

## Decisions

- Author: MPD is split by function as Keystone is (D / E), for the same reason.

## Corrections

## Summary

MPD's cylindrical and 9 V battery parts, completing MPD.
