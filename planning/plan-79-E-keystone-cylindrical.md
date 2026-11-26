# plan-79-E: Keystone cylindrical and 9 V holders, clips and contacts

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-D

E adds Keystone's remaining `in` rows: every function that is not a coin cell (cylindrical holders,
clips and single / dual contacts; 9 V holders, snaps and snap contacts). Together with D they are at
most 300 parts (plan-79-A's cap). E uses D's tools and footprint rules unchanged.

References: plan-79-D § 3 (footprint rules, `merge-fp.py`, `gen-battery.py`, datasheets);
plan-79-C § 3.3 (pin names: `LINK` for a dual contact, `1+` … for a holder with separate cells);
`specs.tsv`, `drawings.tsv`, `verdicts.tsv`.

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-D complete | `ls planning/completed/plan-79-D-*.md` → exists; `python3 -I planning/plan-79-data/gen-battery.py --check` → exit 0 | NOT MET (2026-11-25: D not started) |
| Goldens at E's start | `wc -l < backend/tests/golden/parts.txt`, `… footprints.txt` | E records its own P2 and F2 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Keystone non-coin row of `verdicts.tsv` is `add` (with a footprint) or `skip` (with a
  reason, in `datasheets/skipped-parts.md`).
- `BAT-KEY-…` footprints for their land patterns; `battery-keystone-…` devices and parts for
  `Cylindrical` and `9 V`; `datasheets.json` entries.
- Keystone has min(300, its non-skipped `in` + `reserve` rows) parts in all
  (`grep -c 'mfr=Keystone Electronics' backend/tests/golden/parts.txt`).

### Non-goals

- No change to D's tools or rules: a drawing they cannot express is a `skip` with the reason, and
  the rule gap is recorded under Corrections for the owner, not patched here.

## 2. Design

- **Multi-cell holders:** the pins follow the holder's wiring. A holder whose drawing shows the
  cells in series between two terminals is `+` / `-`; one with a terminal pair per cell is
  `1+`, `1-` … (plan-79-C § 3.3), and its device carries pinout `cells<k>`.
- **9 V snap contacts** ("9V, Snap Contact (Single)") are `contact` devices (one pin, `+` or `-` as
  the drawing marks it; a drawing that marks neither is a `skip`, `polarity not given`), function
  `9 V contact`.
- **Clips** whose ends touch the cell's ends are `clip` devices with `+` / `-`; a clip that only
  grips the body was `out` in A (`mechanical only`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs and footprints

- [ ] Fill `specs.tsv` for the Keystone non-coin rows (plan-79-D § 3.1); `skip` rows and promotions.
- [ ] Add their footprints to `footprints-keystone.json`; merge; regenerate `footprints.txt`.

Acceptance: the new footprints expand and match their drawings.
  Check: `/tmp/logic-p79/footprint_check` passes after regenerating `footprints.txt`;
  `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0; the hand check of
  plan-79-D Phase 2 on one cylindrical holder and one 9 V part (est. 10 min).
  Result:
Commit:

### Phase 2 — Parts

- [ ] `datasheets.json`; `gen-battery.py --write`; regenerate `parts.txt`, `traces.txt`, `erc_pins.txt`.

Acceptance: the parts load and Keystone's count meets the cap rule.
  Check: `ctest --test-dir /tmp/logic-p79 -R 'listing_check|trace_check|erc_pins_check|footprint_check|library_check'` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0; `grep -c 'mfr=Keystone Electronics' backend/tests/golden/parts.txt` → the number `python3 -I planning/plan-79-data/census/build.py --expect keystone` prints (est. 5 min).
  Result:
Commit:

## Validation Plan

- Phase checks above. Final gate: plan-79-J.

## Decisions

- Author: **Keystone's coin and other parts are two letters** by function, because the coin parts
  carry the PCB-contact rule and the generator work (D) and the rest carry multi-cell wiring (E).

## Corrections

## Summary

Keystone's cylindrical and 9 V battery parts, completing its 300.
