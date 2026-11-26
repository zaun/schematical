# plan-74-H: MagJacks, Pulse Electronics: pinouts, footprints, parts, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-G

H adds Pulse's in-scope RJ45 jacks with magnetics (at most 200 of 222, plan-74-A § 2) with G's
rules and generator. Pulse publishes one PDF per part number on `yageogroup.com` (plan-74-A
§ 3.4). Its PDFs carry text: the planning probe's `J0011D01NL` PDF has a part-number table with
"Part Number / Mech. / EMI / LEDs" columns and LED notes. Each PDF covers a series (the J0
series: "100 Base-TX RJ45 1x1 Tab-DOWN with LEDs 8-pin"), with dimensions in inches.

References:

- plan-74-G § 3 (pins, footprints, generator), plan-74-E § 3.2.
- `planning/plan-74-data/census/gen-rj45.py`, `rj45-bel.json` (G).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-G complete | `ls planning/completed/plan-74-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |
| Goldens at H's start | `wc -l < backend/tests/golden/parts.txt` / `footprints.txt`; `grep -c '\|connector\|.*rj45=' backend/tests/golden/parts.txt` | H records its own; the last is G's Bel count |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Pulse `add` row is one `rj45-pulse-` part in `connector.json`, with its PDF's pins and a
  footprint from its PDF's recommended PCB layout.
- `footprints-rj45-pulse.json` holds one footprint per distinct layout (`RJ45-PULSE-<series>`),
  merged. Its inch dimensions are converted to mm, and the `note` carries both.
- G's `rj45-bel-` blocks and every other block of `connector.json` are unchanged. The goldens only
  grow.

### Non-goals (explicit constraints)

- As plan-74-G § Non-goals. No change to G's rules: a Pulse drawing that the rules cannot express
  (a pad the `explicit` style cannot hold) is `skip` (`footprint`), in `skipped-parts.md`.

## 2. Current State

G's generator writes the Bel devices from `rj45-bel.json`. The Pulse rows of `specs-rj45.tsv` are
skeletons (A).

## 3. Design

G's § 3 unchanged. Pulse specifics:

- Pins as each PDF's schematic labels them (`TD+`, `TCT`, `RD+`, `RCT`, …, `TRD1+`… for
  gigabit), LED pins per G § 3.1.
- Inch figures × 25.4, rounded to 0.01 mm. The `note` keeps the inch figure.
- `rj45-pulse.json` is the table G's generator reads. The generator itself changes only in its
  maker list.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Drawings read

- [ ] `planning/plan-74-data/rj45-pulse.json` (pins, LED colours, footprint per PDF series) and the
      Pulse rows of `specs-rj45.tsv`.
- [ ] `planning/plan-74-data/footprints-rj45-pulse.json`.

Acceptance: every footprint loads.
  Check: `planning/plan-74-data/census/fp-load.sh planning/plan-74-data/footprints-rj45-pulse.json` → exit 0, only `extra footprint` / `unused footprint` lines for the new ids (est. 5 min).
  Result:
Commit:

### Phase 2 — Parts, goldens

- [ ] `gen-rj45.py` writes the Pulse devices; `fp-merge.py`; `datasheets.json`; goldens
      regenerated and reviewed (only added lines).

Acceptance: `connector.json` holds the Pulse `add` rows, and the suite passes.
  Check: `cmake --build /tmp/logic-p74 -j8 && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed`; `grep -c '|connector|.*rj45=' backend/tests/golden/parts.txt` equals the `add` rows of `bel` and `pulse` (`awk -F'\t' 'NR>1 && ($2=="bel" || $2=="pulse") && $18=="add"' planning/plan-74-data/candidates.tsv | wc -l`); `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `gen-rj45.py --check` → exit 0 (est. 6 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks, and a 1:1 hand check of three footprints recorded in Phase 1's Result.
- Final gate: plan-74-K.

## Decisions

- Author: **inch drawings convert at 0.01 mm**, finer than any pad tolerance in the drawings.

## Corrections

## Summary

Pulse's jacks with G's rules: one table file, one footprint file, one generator run.
