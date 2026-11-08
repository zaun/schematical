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
| plan-24-G complete | `ls planning/completed/plan-24-G-*.md` → exists | MET (`planning/completed/plan-24-G-adc-parts.md`, 2026-11-07 at ae7490b) |

## 1. Goal

- Both files load; DAC and RTC coverage complete; golden counts as above; `ctest` T0 + 2.

## 2. Design

Same generator as G; RTC devices per plan-24-F §2.

## Phases

### Phase 1 — Devices and goldens

- [x] Generate DAC devices and `rtc.json`; coverage; regenerate goldens. `gen.py converter ADC,DAC` → `764 devices, 1911 parts` (ADC first, then DAC); `gen.py rtc RTC` → `7 devices, 14 parts`; coverage vs parts.tsv `add` rows: ADC+DAC 1911 parts / 764 devices, RTC 14 / 7 — missing 0, extra 0 each; goldens `git diff --numstat`: parts.txt `738 0`, traces.txt `738 0` (724 DAC + 14 RTC); `grep -c '|converter|'` → 1911 (NADC 1187 + NDAC 724), `grep -c '|rtc|'` → 14.
- [x] Move the remaining footprints of `planning/plan-24-data/footprints-new.json` into `footprints.json` (plan-24-C Corrections; the file is then empty and deleted); `footprints.txt` + that many lines, 0 removed; `footprint_check` passes. Moved 25, 0 left (`len(footprints)` → 0); the file is deleted; footprints.txt `25 0`; `ctest` → `100% tests passed, 0 tests failed out of 26`.
- [x] Spot checks: `DAC8562SDGSR`, `DAC5571IDBVR`, one parallel DAC, `BQ32000DR` (`|rtc|`), `BQ4802YDWR` (was `BQ4802YDSWR`: Corrections). parts.txt: `DAC8562SDGSR` `type=dac bits=16 channels=2`, SYNC/SCLK/DIN, LDAC, CLR, `VSSOP-10`; `DAC5571IDBVR` `bits=8 channels=1`, SDA/SCL/A0, `SOT23-6`, vcc 2700-5500; parallel `DAC0808LCM/NOPB` `bits=8`, A1-A8, IO output, `SOIC-16`; `BQ32000DR|rtc|SOIC|…|clock|…5:SDA:inout,6:SCL:input,7:IRQ:output…|footprint=SOIC-8|vcc=3000-3600`; `BQ4802YDWR|rtc|SOIC|…|clock|` A0-A3, DQ0-7, CS/OE/WE, WDI/WDO, `SOIC-28W`, vcc 4500-5500.
- [x] `trace_check --verbose BQ32000DR` shows no SDA drive outside acknowledged transfers: `end 3000000 ok`; SDA (pin 5) is driven weak by the stimulus, so a part drive would override it: rebuilding the stimulus's SDA schedule (`/tmp/p24/g/sdacheck.py`), all 39 SDA net changes are the stimulus's values (0 not), the one stimulus edge with no net change is at 3086818 ps (after the run); no START + address byte occurs, so no transfer, and the part never drives SDA.

Acceptance: `ctest` T0 + 2 of T0 + 2; golden counts (seconds): `100% tests passed, 0 tests failed out of 26`; golden counts above.
Commit: 00236f4

## Validation Plan

H's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-24-A.

## Corrections

- **`BQ4802YDSWR` is not an orderable part**: the BQ4802 rows in
  `parts.tsv` are BQ4802LYDW, BQ4802LYDWR, BQ4802LYPW, BQ4802YDW,
  BQ4802YDWR and BQ4802YPW (all Last Time Buy, added) and BQ4802YDSH
  (Obsolete, skipped). The spot check uses `BQ4802YDWR`, the SOIC (DW)
  tape-and-reel part the name meant.
- **The trace check has no transfer to look at**: the random stimulus
  never forms a START plus address byte on BQ32000DR, so "no SDA drive
  outside acknowledged transfers" is checked as no part drive on SDA at
  all, by comparing the SDA net with the stimulus's own weak drives.

## Summary

H completes plan-24's part data.
