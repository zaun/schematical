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
| plan-24-A complete | `ls planning/completed/plan-24-A-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `tail -n +2 planning/plan-24-data/parts.tsv | wc -l` → 2707 (1718 + 945 + 44), each `add` or `skip`.
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

- [ ] Stock re-check per DAC/RTC base name (240 base names, about 240 calls); non-Active/LTB and out-of-stock rows `skip`.
- [ ] Download the PDFs. Check: PDF count = distinct DAC/RTC `pdf` values.

Acceptance: both checks.
Commit: —

### Phase 2 — Part data

- [ ] DAC and RTC rows of `parts.tsv`, `specs.md` sections. Checks: 2707 rows total; 0 undecided; 0 `add` rows without a footprint id.
- [ ] `datasheets/skipped-parts.md` § TI DACs, § TI RTCs.

Acceptance: the checks above (minutes).
Commit: —

## Validation Plan

B has no code; Phase 2's checks are its gate.

## Decisions

As plan-24-A.

## Corrections

## Summary

B completes plan-24's data: every part decided, every device described.
