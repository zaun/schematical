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
| plan-10-E complete | `ls planning/completed/plan-10-E-counter-models.md` → exists | MET (2026-11-01: exists, archived at 428808b) |

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

- [x] Footprint (§4.2) and the 57 devices / 730 parts. (`/tmp/p10/gen_f.py`;
  the C §1 one-liner on `counter.json` → `57 730`; VQFN-16 hand-checked: 16
  pads at 0.5 mm, pads at x ±1.65 / y ±1.9 = spanX 3.3 / spanY 3.8, centre
  2.05 × 2.55 unnumbered.)
- [x] Name/device check (C Phase 2 method): `730 parts, 57 devices, names
  and devices match`. (Ran `/tmp/p10/check_names.py`: that line, family and
  footprint per part included.)
- [x] Regenerate the three goldens; added lines only; 730 / 730 / 1. (Ran: 0
  removed, 1461 added; 5525 / 5525 / 156; `grep -c '|counter|'` → 730,
  `'|counter|.*|Divider|'` → 58.)
- [x] Spot checks, one line each here: `SN74HC163N` (`vcc=2000-6000`,
  `units=1`), `SNJ54HC163FK` (`fk20`, `LCCC-20`, `military`), `SN74LS90N`
  (14 pins, `LS`), `CD4029BE` (`C`, `vcc=3000-18000`), `SN74LV8154PW`
  (`units=2`), `CD74HC4059M96` (24 pins, `SOIC-24W`, `|Divider|`),
  `SN54AS867JT` (device `74_867_AS`, `CDIP-24`, `military`). (Ran, from
  `parts.txt`: `SN74HC163N` `HC`, `units=1`, 16 pins, `vcc=2000-6000`;
  `SNJ54HC163FK` 20 pins (1 `NC`), `LCCC-20`, `military`; `SN74LS90N` 14
  pins, `LS`, `vcc=4750-5250`; `CD4029BE` `C`, `vcc=3000-18000`;
  `SN74LV8154PW` `units=2`, `TSSOP-20`; `CD74HC4059M96` 24 pins,
  `SOIC-24W`, `|Divider|`; `SN54AS867JT` in device `74_867_AS` (from
  `counter.json`), `CDIP-24`, `AS|military`.)

Acceptance: `ctest` 14 of 14; goldens 5525 / 5525 / 156, added only. (Ran:
`100% tests passed, 0 tests failed out of 14`.)
Commit: 108a814

### Phase 2 — Real-part cases

- [x] §4.3 cases in `counter_test`. (The D/E case functions take a real
  part through `real_part` and a longer step: 100 ns for HC/LS/LV/TTL, 1 µs
  for CD4017BE and CD4059AM, 2 µs for CD4040BE; output lines carry the part
  name, e.g. `PASS: [SN74HC161N] 1 …`. `docs/parts-format.md`'s file table
  gains `counter.json`.)

Acceptance: `counter_test` all `PASS`; `ctest` 14 of 14. (Ran: 64 `PASS` —
53 fixture cases and 11 real-part lines — and no `FAIL`; `100% tests
passed, 0 tests failed out of 14`.)
Commit: —

## Validation Plan

Phase 1 and 2 acceptance (`ctest`, about 6 s).

## Decisions

- Dividers live in `counter.json` with `"function": "Divider"` (plan-10-A
  §3.2).

## Corrections

- 2026-11-01, Phase 1: where one device's families print different pin names
  the device takes one set (plan-10-A §3.4), recorded in each device's
  `note`: SN names for 74_161/163/192/193 (not the CD74HC MR/CP/P0…), LS names
  for 74_590 (CCK/RCK/G), `1CLK`/`2CLK` for 74_393 (39 of its 56 parts are
  HC/HCT/LV; the LS sheet prints 1A/2A), and the CD names CP/MR/Q1… for
  74_4020/4024/4040 (as CD4020B/4040B number their outputs; the SN sheets'
  QA… are the same pins).
- 2026-11-01, Phase 1: ripple `stage` values are derived per §4.1 from the
  printed clock-to-output rows: (last − first) / depth, averaged over tPLH
  and tPHL (74_90 10, 74_92 10, 74_93/293 11, 74_390 LS 11.5, HC/HCT 12,
  74_393 LS 9.2, HC 5.7, HCT 10, LV 0.9). Where a family prints only the
  Qn → Qn+1 maximum (74_4020/4024/4040 HC/HCT) that maximum is the stage.
- 2026-11-01, Phase 1: the 74_867_AS / 74_869_AS parts are all SN54AS
  (military) rows, but the timing entry carries the SN74 maximum (11/15)
  with the SN54 value in its note, as every other family entry does; their
  `vcc` is the same for SN54 and SN74 (4.5–5.5 V).
- 2026-11-01, Phase 1: 4059A's recommended VDD is 3–12 V (an A-series part),
  not the B-series 3–18 V.

## Summary

F adds the 730 counter and divider parts and `VQFN-16`.
