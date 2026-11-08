# plan-24-H: DAC and RTC parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-24-G

H adds the DAC devices to `converter.json`, writes `rtc.json`, and
regenerates the goldens.

Outcome of H: `grep -c '|converter|' parts.txt` = NADC + NDAC;
`grep -c '|rtc|' parts.txt` = NRTC; goldens + NDAC + NRTC each; none removed.

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-G complete | `ls planning/completed/plan-24-G-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- Both files load; DAC and RTC coverage complete; golden counts as above; `ctest` T0 + 2.

## 2. Design

Same generator as G; RTC devices per plan-24-F §2.

## Phases

### Phase 1 — Devices and goldens

- [ ] Generate DAC devices and `rtc.json`; coverage; regenerate goldens.
- [ ] Move the remaining footprints of `planning/plan-24-data/footprints-new.json` into `footprints.json` (plan-24-C Corrections; the file is then empty and deleted); `footprints.txt` + that many lines, 0 removed; `footprint_check` passes.
- [ ] Spot checks: `DAC8562SDGSR`, `DAC5571IDBVR`, one parallel DAC, `BQ32000DR` (`|rtc|`), `BQ4802YDSWR`.
- [ ] `trace_check --verbose BQ32000DR` shows no SDA drive outside acknowledged transfers.

Acceptance: `ctest` T0 + 2 of T0 + 2; golden counts (seconds).
Commit: —

## Validation Plan

H's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-24-A.

## Corrections

## Summary

H completes plan-24's part data.
