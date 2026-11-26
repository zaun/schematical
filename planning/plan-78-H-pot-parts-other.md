# plan-78-H: Potentiometer parts: Amphenol Piher, Vishay Sfernice, Same Sky

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-78-G

H runs G's `gen-pot.py` for the other three makers, merges D's footprints, adds their datasheet
entries, and regenerates the goldens. After H the library lists every `add` row of
`verdicts.tsv`.

References:

- plan-78-G § 3 (devices, parts, merge, datasheets: H follows it unchanged).
- `docs/backend.md` § goldens.

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-G complete | `ls planning/completed/plan-78-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |
| Goldens at H's start | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` / `erc_pins.txt` | H records P2, R2, F2, E2 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `resistor.json` gains the devices and parts of every `piher`, `sfernice` and `samesky` `add` row;
  the fixed resistors and G's Bourns devices are unchanged.
- Every footprint of `footprints-other.json` is in `footprints.json` and used by a part.
- `datasheets.json` has one entry per PDF of `datasheets/potentiometer/{piher,sfernice,samesky}/`.
- Goldens grow by exactly H's parts and footprints; no existing line changes.

### Non-goals (explicit constraints)

- No backend, app or key change.

## 2. Current State

After G: Bourns's pots are in `resistor.json`; `gen-pot.py` takes a maker slug.

## 3. Design

As plan-78-G § 3. Manufacturer names in the part files: `Amphenol Piher` (DigiKey's "Amphenol Piher
Sensing Systems", shortened as `Amphenol ICC` is), `Vishay Sfernice` (the division, as `Vishay
Dale` and `Vishay Siliconix` are named today), `Same Sky` (the name 799 parts already use:
`grep -ho '"manufacturer": *"Same Sky"' schematical/schematical/Parts/*.json | wc -l` → 799).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Parts

- [ ] `gen-pot.py piher`, `sfernice`, `samesky`; merge `footprints-other.json`; datasheet entries.

Acceptance: the library loads with every `add` row as a part.
  Check: `cmake --build /tmp/logic-p78 -j8 && /tmp/logic-p78/listing_check --print | grep -c '^[^|]*|resistor|.*pot='` → `awk -F'\t' '$3=="add"' planning/plan-78-data/verdicts.tsv | wc -l`; `python3 -I planning/plan-78-data/census/gen-pot.py --check bourns piher sfernice samesky` → exit 0 (est. 3 min).
  Result:
Commit:

### Phase 2 — Goldens

- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`, `erc_pins.txt`; review the diff;
      hand-check five changed footprint lines against their drawings.

Acceptance: the goldens grow by exactly H's parts and footprints.
  Check: `git diff -U0 backend/tests/golden | grep -c '^-[^-]'` → 0; `ctest --test-dir /tmp/logic-p78 -R 'resistor_test|listing_check|trace_check|footprint_check|erc' --output-on-failure` → all pass (est. 3 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-78-I.

## Decisions

- Author: **division names as the makers**: `Vishay Sfernice` and `Amphenol Piher` match how the
  library already names Vishay's divisions, and match the datasheets' own branding.

## Corrections

## Summary

The other three makers' pots join Bourns's in `resistor.json`, with footprints, datasheets and
goldens.
