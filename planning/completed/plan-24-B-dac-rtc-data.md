# plan-24-B: DAC and RTC datasheets and part data

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-A

B does for the 945 DAC and 44 RTC candidates what A did for the ADCs.

Outcome of B: `datasheets/dac/` and `datasheets/rtc/` hold every PDF the
DAC/RTC `add` rows cite; `parts.tsv` covers every candidate (2707 rows);
`specs.md` has a section per DAC and RTC device.

References: plan-24-A (§3 design, §4 data layout); `planning/plan-24-data/candidates.tsv`.

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-A complete | `ls planning/completed/plan-24-A-*.md` → exists | MET (`planning/completed/plan-24-A-converter-data.md`, 2026-11-07 at 868ae12) |
| plan-24-A § Prerequisites rows | (A's commands) | tests `100% tests passed, 0 tests failed out of 24`; goldens 11126 / 423; serial target 32; no converter/rtc files; DigiKey: `0 left today` (the shared daily quota is spent; the script works) — B needs no new calls (Corrections) |

## 1. Goal

- `tail -n +2 planning/plan-24-data/parts.tsv | wc -l` → 2787 (1798 + 945 + 44; plan-24-A Corrections), each `add` or `skip`.
- `ls datasheets/dac/*.pdf datasheets/rtc/*.pdf | wc -l` = distinct DAC/RTC `pdf` values.

## 2. Design

- DAC `specs.md` sections: bus, register map / commands (plan-24-D §3
  schema), input/DAC register pairs, LDAC (sync/async), CLR, power-down
  modes as register state, reset values, output code format, digital VCC.
- RTC sections (plan-24-F §2 schema): bus (I2C address; parallel: CE/OE/WE
  or the DP857x Intel/Motorola bus), register map with BCD/binary formats,
  12/24-hour mode, alarm and periodic-interrupt registers, watchdog,
  battery-backed RAM size (bq4802 none; bq4845 none; DP8570 RAM bytes per
  datasheet), oscillator stop/start bits, the time-update/read-latch
  mechanism the datasheet describes.
- The 3 RTC rows with no DigiKey base product are identified from their PDF
  and mapped to a device or `skip`.

## Phases

> **Status discipline:** as plan-24-A.

### Phase 1 — Stock and datasheets

- [x] Stock re-check: done by plan-24-A's category listings the same day (`stock/list-701-active.tsv` 792, `list-701-ltb.tsv` 6, `list-690-active.tsv` 9, `list-690-ltb.tsv` 8); `census/stockcheck.py` → every in-scope Active/LTB DAC and RTC census row is listed, no new names, no status change (Corrections).
- [x] Download the PDFs (`census/dl.sh`; 249 DAC + 7 RTC names, 14 + 3 found by hand, `dp8570a`/`dp8573a` for the National RTCs; 218 + 8 documents). Check: `ls datasheets/dac/*.pdf datasheets/rtc/*.pdf | wc -l` → 224 = distinct `pdf` of DAC (217) + RTC (7) `add` rows. No TI product page (`ti.com/product/<P>` → 404): DAC530A2, DAC532A3, DAC702, BQ3285L.

Acceptance: both checks.
Commit: 5287733

### Phase 2 — Part data

- [x] DAC and RTC rows of `parts.tsv`, `specs.md` sections (`census/specs_md.py` → ADC 516, DAC 252, RTC 7). Checks: `tail -n +2 parts.tsv | wc -l` → 2787 (Corrections: 1798 + 945 + 44); `census/checks.py DAC` / `RTC` → `undecided 0; add without footprint 0; highspeed/other added 0`; `census/validate.py` → `775 spec files, 0 with problems`; `census/coverage.py DAC dac` → `945 rows, 725 add, 252 devices, 217 PDFs, 217 cited; 0 problems`, `RTC rtc` → `44 rows, 14 add, 7 devices, 7 PDFs, 7 cited; 0 problems`.
- [x] `datasheets/skipped-parts.md` § TI DACs, § TI RTCs (`census/skipped_md.py DAC RTC` → `DAC 220 skipped rows in 9 reasons`, `RTC 30 skipped rows in 3 reasons`).
- [x] Spec writing: 26 DAC batches + 1 RTC batch by subagents (as plan-24-A), plus 2 DAC fix-up batches (update-all, strap-selected resets, per-grade resets, clear codes, register-write triggers, per-package I2C straps, active-high LOAD pins, the DAC60508C/DAC80508C/DAC61404 splits) and an RTC pass for the adopted clock forms.

Acceptance: the checks above (minutes).
Commit: —

## Validation Plan

B has no code; Phase 2's checks are its gate.

## Decisions

As plan-24-A.

## Corrections

- **Stock re-check reused, no new DigiKey calls.** plan-24-A's in-stock
  Active/LTB listings of categories 701 and 690 (2026-11-07, `stock/`)
  cover every DAC/RTC row; the shared quota was at 0 when B started.
- **Ordering tables decide `add`** as in A: DAC 725 rows match (12 in no
  table, e.g. `DAC715UK-TI`, `DAC811KUG4`), RTC 14 (BQ3285ESS-N/-TR1 and
  BQ3285LCS are not in `bq3285e.pdf`'s table: skip, as is the whole
  BQ3285E/L document). `census/devices.py` gained a designator-strip rule
  and overrides for DigiKey's truncated base products of -Q National
  parts (`DAC101S101QCMK/NOPB` → DAC101S101, …): 249 DAC devices, 7 RTC
  devices. 27 more TI packages (`footprint-ids.tsv`, 24 new).
- **Schema additions for DACs and RTCs** (`schema.md`): `updateall`,
  LDAC `"edge": "level"`, `partKeys.resets`, `protocol.triggers`, `then` on
  pointer writes, latch-on-change parallel DACs, `packages.<code>.i2c`,
  SPI `csActive`, `pins.resetSelect`, edge-triggered `clear`, pin/field
  `clearCode`; RTC: `h12Level`, `stop.running`, alarm `maskLevel` /
  `dontCare`, `update.latch` `writeHalts`, periodic arrays, the watchdog
  form, the multiplexed-bus `ale`, map-valued `protocol.reads`.
- **Device splits from the datasheets**: DAC60508C / DAC80508C (pin 15 CLR
  instead of SDO/ALARM), DAC61404 (the 12-bit part DigiKey filed under
  DAC81404). 252 DAC devices.
- The "3 RTC rows with no base product" are BQ3285ESS-N, BQ3285ESS-TR1
  (bq3285E, not in its ordering table) and BQ3285LCS (bq3285L, no TI page).

## Summary

B completes plan-24's data: every part decided, every device described.
