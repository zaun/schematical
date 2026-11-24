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

- `planning/completed/plan-55-C-powerswitch-verdicts-ideal-load.md` § 3; `planning/completed/plan-55-A-powerswitch-kind.md` § 3.1;
  `planning/plan-42-data/census/` (`footprint-lines.py`, `prune.py`: the scripts E adapts).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats), `AGENTS.md`.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-D complete | `ls planning/completed/plan-55-D-*.md` → exists | MET (2026-11-23: archived at 482dca1e) |

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
| Candidate rows after B | the Microchip and Diodes `limited` rows with `state` `candidate` | `awk -F'\t' '$1=="limited" && ($2=="microchip"\|\|$2=="diodes") && $12=="candidate"' planning/plan-55-data/candidates.tsv \| wc -l` (recorded here when E starts: Microchip 148 and Diodes 155 candidate rows after B) |

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

- [x] Read every Microchip (with Micrel) and Diodes PDF of the `limited` family; fill `specs.tsv` (C § 3). Observed: read in the readers' packets (plan-55-C Correction 2); `pinout-check.py --family limited --complete` → `0 pinout errors` (the whole family: `568 rows checked; 0 pinout errors` over all three families).

Acceptance: every Microchip and Diodes `limited` row has a spec row or a skip reason.
  Check: `python3 -I planning/plan-55-data/census/pinout-check.py --family limited` → `0 pinout errors` (est. 2 min).
Commit: cced7528

### Phase 2 — Verdicts, skips, the final pick, prune, footprint lines

- [x] `verdicts.py` over all three families; the skips of the two companies in `datasheets/skipped-parts.md`;
      hand-check five `add` rows (two Microchip, three Diodes) against their PDFs (name, pin order,
      enable polarity, `vin_max`, `ilimit`); record in § Corrections (Correction 1). Observed: `verdicts.py --report` → `ideal: 32 add, 58 skip`, `load: 197 add, 44 skip`, `limited: 588 add, 111 skip` (Microchip 135, Diodes 154, TI 299), `rows without a verdict: 0`; `--verify-skipped` → `0 skip rows missing from skipped-parts.md`.
- [x] `census/footprint-lines.py` → `footprints-needed.txt` (§ 3.1); `census/prune.py` (§ 3.2). Observed: `footprint-lines.py` → 51 `F` lines and 69 `G` lines; `prune.py` → `264 files; 225 used by add rows; keep 225, delete 39`, `datasheets.tsv: 225 lines`.

Acceptance: every candidate row of the three families has a verdict; the lines and the folder agree
with the `add` rows.
  Check (observed: 817 = 32 + 197 + 588 `add` rows; `awk -F'\t' '!/^#/{s+=$5} END{print s}' planning/plan-55-data/footprints-needed.txt` → 817; `add pdfs: 225` and `find datasheets/powerswitch -iname '*.pdf' | wc -l` → 225): `python3 -I planning/plan-55-data/census/verdicts.py --report` → no row without a verdict; `… --verify-skipped` → `0 skip rows missing from skipped-parts.md`; `awk -F'\t' '{s+=$5} END{print s}' planning/plan-55-data/footprints-needed.txt` → the `add` count (the sum of the `add` rows of the report); `verdicts.py --report` prints `add pdfs: <n>` (the distinct PDFs the `add` rows cite) and `find datasheets/powerswitch -iname '*.pdf' \| wc -l` → the same `<n>` (est. 3 min).
Commit: (filled by the next commit)

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

1. **Hand checks (2026-11-23), all agree with the PDFs**: MIC2025-1YM SOIC-8 (Microchip, `MIC2025-2075-…DS20006030A.pdf` Table 3-1: EN 1 active high (-1), FLG 2, GND 3, NC 4 and 5, OUT 6 and 8, IN 7; `ilimit` 0.85 A, `vin_max` 5.5); MIC2026-1YM dual SOIC-8 (Table 3-1: ENA 1, FLGA 2, FLGB 3, ENB 4, OUTB 5, GND 6, IN 7, OUTA 8; ENx active high on -1; `ilimit` 1.0 A); AP2161WG-7 SOT-25 (Diodes, `AP2161_71.pdf` Pin Descriptions: OUT 1, GND 2, FLG 3, EN 4 active low, IN 5; `vin_max` 5.5, `ilimit` 1.5 A); AP2142SG-13 dual SO-8 (`AP2142_52.pdf`: GND 1, IN 2, EN1 3, EN2 4, FLG2 5, OUT2 6, OUT1 7, FLG1 8; ENx active low (AP2142)); AP22804AW5-7 SOT-25 (`AP22804_14.pdf`: OUT 1, GND 2, FLG 3, EN 4 active high (A), IN 5; `ilimit` 3.0 A for AP22804).
2. **The final pick is 817 `add` rows**: ideal 32 (Analog Devices 28, TI 4; Monolithic Power Systems 0, its 9 rows have no fetchable datasheet, plan-55-B Correction 4), load 197 (TI 127, Microchip 38, onsemi 32), limited 588 (TI 299, Microchip 135, Diodes 154), against the planning census's at most 1037. The 225 PDFs of the `add` rows are kept (312 MB, committed here, plan-55-B Correction 9); the 39 others were deleted.
3. **`footprints-needed.txt` has 120 lines** (F 51, G 69): one per (company, `Package / Case`, `Supplier Device Package`) of the `add` rows (column 5 = the `add` rows it serves, summing to 817).

## Summary

E closes the data side: after it the pick is fixed, so F and G draw exactly the packages the parts
need and H and I load exactly the rows that passed.
