# plan-55-E: Verdicts — current-limited and USB switches, Microchip and Diodes; the final pick

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-D

E gives every Microchip Technology (with Micrel) and Diodes Incorporated candidate row of the
`limited` family (173 and 167 at most, plan-55-A § 2) a verdict by plan-55-C § 3, then closes the
data side of the plan: the final pick, the skipped-parts list, the PDFs no `add` row cites are
removed, and `footprints-needed.txt` names every package F and G draw. The behavioral outcome:
every candidate row of all three families has a verdict; `footprints-needed.txt` lists one line per
(company, `Package / Case`, `Supplier Device Package`) of the `add` rows, lettered `F` (Texas
Instruments) or `G` (every other company); `datasheets/powerswitch/` holds only PDFs an `add` row
cites. No part file, footprint, backend or app file changes.

References:

- `planning/plan-55-C-powerswitch-verdicts-ideal-load.md` § 3; `planning/plan-55-A-powerswitch-kind.md` § 3.1;
  `planning/plan-42-data/census/` (`footprint-lines.py`, `prune.py`: the scripts E adapts).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats), `AGENTS.md`.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-D complete | `ls planning/completed/plan-55-D-*.md` → exists | NOT MET (2026-11-22: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Microchip and Diodes row of the family `limited` has a verdict; `pinout-check.py --family
  limited` → `0 pinout errors`.
- `python3 -I planning/plan-55-data/census/verdicts.py --report` → every candidate row of the three
  families has `add` or `skip`, and prints the `add` count per family and company (the final pick).
- `planning/plan-55-data/footprints-needed.txt` exists with the lines of § 3.1.
- `datasheets/powerswitch/` holds only PDFs an `add` row cites; `datasheets.tsv` matches.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app; no footprint is drawn.
- No rule change: C § 3 holds.

## 2. Current State

C and D's verdicts for the other two families and Texas Instruments; B's PDFs.

### Measured populations

| What | Count | Command |
|---|---|---|
| Microchip current-limited candidates at most | 173 (label 150: 156, label 576, Micrel: 17) | `grep '^pick.*limited.*Microchip' planning/plan-55-data/families.out` → `parts 173` |
| Diodes current-limited candidates at most | 167 | `grep '^pick.*limited.*Diodes' planning/plan-55-data/families.out` → `parts 167` |
| `Package / Case` labels: Microchip, Diodes (family `limited`) | 17 and 17 | same lines, `packages(case)` |
| Candidate rows after B | the Microchip and Diodes `limited` rows with `state` `candidate` | `awk -F'\t' '$1=="limited" && ($2=="microchip"\|\|$2=="diodes") && $12=="candidate"' planning/plan-55-data/candidates.tsv \| wc -l` (recorded here when E starts) |

## 3. Design

### 3.1 `footprints-needed.txt`

`census/footprint-lines.py` (plan-42's, adapted) writes one tab-separated line per distinct
(company, `package`, `supplier_package`) of the `add` rows: the letter of the footprint letter that
draws it (`F` for `ti`, `G` for every other company), company, `Package / Case`, `Supplier Device
Package`, the number of `add` rows, one example part number. `F` lines are plan-55-F's, `G` lines
plan-55-G's.

### 3.2 Pruning

`census/prune.py` deletes every PDF under `datasheets/powerswitch/` that no `add` row's `pdf` names and
rewrites `datasheets.tsv`. A deleted PDF stays re-downloadable from its recorded URL. A PDF whose
every part is obsolete is deleted and recorded in `datasheets/obsolete-not-included.md` (PDF name,
family and drawer path), as `AGENTS.md` says; DigiKey lists only Active parts, so none is expected.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Microchip and Diodes

- [ ] Read every Microchip (with Micrel) and Diodes PDF of the `limited` family; fill `specs.tsv` (C § 3).

Acceptance: every Microchip and Diodes `limited` row has a spec row or a skip reason.
  Check: `python3 -I planning/plan-55-data/census/pinout-check.py --family limited` → `0 pinout errors` (est. 2 min).
Commit: —

### Phase 2 — Verdicts, skips, the final pick, prune, footprint lines

- [ ] `verdicts.py` over all three families; the skips of the two companies in `datasheets/skipped-parts.md`;
      hand-check five `add` rows (two Microchip, three Diodes) against their PDFs (name, pin order,
      enable polarity, `vin_max`, `ilimit`); record in § Corrections.
- [ ] `census/footprint-lines.py` → `footprints-needed.txt` (§ 3.1); `census/prune.py` (§ 3.2).

Acceptance: every candidate row of the three families has a verdict; the lines and the folder agree
with the `add` rows.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --report` → no row without a verdict; `… --verify-skipped` → `0 skip rows missing from skipped-parts.md`; `awk -F'\t' '{s+=$5} END{print s}' planning/plan-55-data/footprints-needed.txt` → the `add` count (the sum of the `add` rows of the report); `verdicts.py --report` prints `add pdfs: <n>` (the distinct PDFs the `add` rows cite) and `find datasheets/powerswitch -iname '*.pdf' \| wc -l` → the same `<n>` (est. 3 min).
Commit: —

## Validation Plan

- Tests: the scripts' output is the check; no backend or app test is touched.
- Coverage check: `verdicts.py --report` counts every candidate row of the three families once.
- Runtime proof: none (data only).
- Doc sync: `datasheets/skipped-parts.md`; `planning/plan-55-data/README.md` lists every script and file.
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): scope, catalog rule, naming.
- Author: **the final pick is every `add` row**; there is no quota (plan-55-B § Decisions), so the
  pick is what the verdicts leave.

## Corrections

(none yet)

## Summary

E closes the data side: after it the pick is fixed, so F and G draw exactly the packages the parts
need and H and I load exactly the rows that passed.
