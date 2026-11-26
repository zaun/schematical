# plan-79-H: Adam Tech and Assmann WSW battery holders

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-G

H adds every non-skipped `in` row of the two smaller makers: Adam Tech (74 PCB-mount parts at the
planning census) and Assmann WSW Components (53), all functions, with D's tools and rules.

References: plan-79-D § 3; plan-79-E § 2; plan-79-F § 2; `drawings.tsv` for each maker's PDFs.
Assmann already ships 38 parts of other kinds (`grep -c 'mfr=Assmann' backend/tests/golden/parts.txt`
→ 38 at planning); H adds only `battery-` devices and leaves those untouched.

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-G complete | `ls planning/completed/plan-79-G-*.md` → exists | NOT MET (2026-11-25: G not started) |
| Goldens at H's start | `wc -l < backend/tests/golden/parts.txt`, `… footprints.txt`; `grep -c 'mfr=Assmann' backend/tests/golden/parts.txt` | H records its own (A0 = the Assmann count) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Adam Tech and Assmann row is `add` or `skip`; footprints `BAT-ADAM-…` and `BAT-ASM-…` in
  `footprints-adam.json` and `footprints-assmann.json`, merged; devices `battery-adamtech-…` and
  `battery-assmann-…`; `datasheets.json` entries.

### Non-goals

- No change to D's tools or rules, or to Assmann's existing parts.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Adam Tech

- [ ] Specs, footprints, merge; `datasheets.json`; `gen-battery.py --write`; goldens.

Acceptance: Adam Tech's parts load and its count is complete.
  Check: `ctest --test-dir /tmp/logic-p79 -R 'listing_check|trace_check|erc_pins_check|footprint_check|library_check'` → `100% tests passed` after regenerating; `grep -c 'mfr=Adam Tech' backend/tests/golden/parts.txt` → `build.py --expect adamtech`; the plan-79-D Phase 2 hand check on one part (est. 15 min).
  Result:
Commit:

### Phase 2 — Assmann WSW

- [ ] Specs, footprints, merge; `datasheets.json`; `gen-battery.py --write`; goldens.

Acceptance: Assmann's battery parts load; its other parts did not move.
  Check: the same `ctest` → `100% tests passed`; `grep -c 'mfr=Assmann' backend/tests/golden/parts.txt` → A0 + `build.py --expect assmann`; `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0 (est. 15 min).
  Result:
Commit:

## Validation Plan

- Phase checks above. Final gate: plan-79-J.

## Decisions

- Author: the two small makers share one letter: 127 parts at most, about one sitting.

## Corrections

## Summary

The last two makers' battery holders, completing the library's battery parts.
