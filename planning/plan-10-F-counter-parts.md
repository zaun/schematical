# plan-10-F: Counter and divider parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-E

F writes `schematical/schematical/Parts/counter.json`: 57 devices, 730
parts (counters and dividers), from `plan-10-data/counter-a.md`,
`counter-b.md`, `counter-c.md`, `divider.md` and `parts.tsv`; adds the one
new footprint the counters use first (`VQFN-16`); regenerates the goldens.

Outcome of F: goldens grow by exactly 730 part lines and 1 footprint line;
`counter_test` also passes on real parts; `ctest` 14 of 14.

References: plan-10-A §3.4, plan-10-D §5 and plan-10-E §4 (models, device →
keys tables), plan-10-C (the same procedure for latches), the spec files
above, `plan-10-data/footprints.md` § VQFN-16.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-E complete | `ls planning/completed/plan-10-E-counter-models.md` → exists | NOT MET (2026-11-01: written) |

## 1. Goal

- `counter.json`: 57 devices, 730 parts (the C §1 python one-liner on
  `counter.json` → `57 730`).
- `wc -l < backend/tests/golden/parts.txt` → 5525 (4795 + 730), `traces.txt`
  → 5525, `footprints.txt` → 156 (155 + 1); added lines only.
- `grep -c '|counter|' backend/tests/golden/parts.txt` → 730;
  `grep -c '|counter|.*|Divider|' backend/tests/golden/parts.txt` → 58
  (the listing's function column; 4022B, 4017B, 74_4017, 4018B, 4059A,
  74_4059, 4089B, 4522B, 4527B, 74_97).

### Non-goals (explicit constraints)

- No existing golden line changes. No model change (a gap is a D/E bug:
  Corrections entry plus a `counter_test` case).

## 2. Current State

After E: the `counter` kind with six models and 53 fixture cases; no
`counter.json`. 156 − 1 = 155 footprints.

### Measured populations

| What | Count | Command |
|---|---|---|
| Counter devices / parts | 57 / 730 | `awk -F'\t' 'NR>1 && ($10 ~ /^counters\// \|\| $10 ~ /^divider\//)' planning/plan-10-data/parts.tsv \| wc -l` → 730; `cut -f2 … \| sort -u \| wc -l` → 57 |
| … `"function": "Divider"` | 10 devices / 58 parts | `awk -F'\t' '$10 ~ /^divider\//' parts.tsv \| wc -l` → 58 |
| … `"function": "Counter"` | 47 / 672 | the rest |
| 16-pin and 14-pin counter devices with FK-20 parts | 74_161 (10), 74_163 (10), 74_169 (2), 74_191 (5), 74_193 (4), 74_390 (3), 74_393 (5), 74_4020 (2), 74_4040 (2), 74_590 (4) | `awk -F'\t' '$7=="LCCC-20"' parts.tsv` (counter devices) |
| 20-pin counter devices with FK parts (FK = DIP numbering) | 74_569 (2) | same |
| Obsolete counter parts | 109 | `awk -F'\t' 'NR>1 && ($10 ~ /^counters\// \|\| $10 ~ /^divider\//) && $9=="Obsolete"' parts.tsv \| wc -l` |

## 3. Design Overview

Pure data, as C. File order: D §5.1.3 devices, D §5.2, E §4.1–§4.4. Each
device: `id`, `function` (`"Counter"`, or `"Divider"` for the 10 divider-folder
devices), `description`, `drawer` `["Logic"]`, `note`, `model`, `pinouts`, the
model keys from the D/E tables, `timing` (one entry per family in
`parts.tsv`), `parts`.

## 4. Detailed Design

### 4.1 Devices

- **Main timing path** per D §5 / E §4: clock → Q; for the register counters
  (590, 697, 8154) the register clock → Q; ripple devices add `"stage"`
  (the spec's per-stage delay, or (last-stage delay − first-stage delay) /
  (stages − 1) where only those two are printed); 40110B adds `carryPulse`
  230 and `borrowPulse` 275 (5 V typ, `counter-c.md` § 40110B).
- **FK-20 pinouts** for the devices listed in §2: an `fk20` pinout copied from
  the map each device's spec section prints (all ten print one). For the
  16-pin devices it is the standard map — NC at terminals 1, 6, 11, 16; DIP
  1–4 → 2–5, 5–8 → 7–10, 9–12 → 12–15, 13–16 → 17–20 (`counter-a.md`
  § 74_161) — and 74_393 (14-pin) has its own printed map. `counter-b.md`
  § 74_390 flags a misprint (FK pin 19 printed `3CKA`): the pin is `2CKA`.
- **74_867 / 74_869 vs the _AS devices**: the same pinouts and keys, split by
  `parts.tsv`'s device column (AS rows → `_AS`).
- **4029B `codeSelect`** and **4522B `zeroLatch`**: as D §5.1.3.
- **74_4518**: one HC part (`CD74HC4518E`); the same keys as 74_4520 with
  `code` bcd.
- **Military numbers**: `parts.tsv` already carries the device and family of
  every 5962-/JM38510/M38510 row (`plan-10-data/README.md`).

### 4.2 Footprint

Add `VQFN-16` from `plan-10-data/footprints.md` (verbatim) after C's 15. Its
first parts: SN74LV163ARGYR, SN74LV4040ARGYR (and the counter RGY rows of
`parts.tsv`).

### 4.3 Real-part checks

`counter_test` gains: `SN74HC161N` (D cases 1–4), `SN74LS193N` (case 9),
`SN74HC590AN` (case 13), `SN74LV8154N` (case 17), `CD4040BE` (Q12 toggles
every 2048 clocks), `CD4017BE` (case 30), `CD4059AM` (case 42),
`SN7497N` (case 50).

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — `counter.json` and `VQFN-16`

- [ ] Footprint (§4.2) and the 57 devices / 730 parts.
- [ ] Name/device check (C Phase 2 method): `730 parts, 57 devices, names
  and devices match`.
- [ ] Regenerate the three goldens; added lines only; 730 / 730 / 1.
- [ ] Spot checks, one line each here: `SN74HC163N` (`vcc=2000-6000`,
  `units=1`), `SNJ54HC163FK` (`fk20`, `LCCC-20`, `military`), `SN74LS90N`
  (14 pins, `LS`), `CD4029BE` (`C`, `vcc=3000-18000`), `SN74LV8154PW`
  (`units=2`), `CD74HC4059M96` (24 pins, `SOIC-24W`, `|Divider|`),
  `SN54AS867JT` (device `74_867_AS`, `CDIP-24`, `military`).

Acceptance: `ctest` 14 of 14; goldens 5525 / 5525 / 156, added only.
Commit: —

### Phase 2 — Real-part cases

- [ ] §4.3 cases in `counter_test`.

Acceptance: `counter_test` all `PASS`; `ctest` 14 of 14.
Commit: —

## Validation Plan

Phase 1 and 2 acceptance (`ctest`, about 6 s).

## Decisions

- Dividers live in `counter.json` with `"function": "Divider"` (plan-10-A
  §3.2).

## Corrections

(none yet)

## Summary

F adds the 730 counter and divider parts and `VQFN-16`.
