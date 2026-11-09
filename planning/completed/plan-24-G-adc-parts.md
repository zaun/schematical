# plan-24-G: ADC parts

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-F

G writes the ADC devices of `planning/plan-24-data/specs.md` and `parts.tsv`
into `schematical/schematical/Parts/converter.json` and regenerates the
goldens.

Outcome of G: `grep -c '|converter|' backend/tests/golden/parts.txt` = NADC
(ADC `add` rows); goldens +NADC each; none removed.

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-F complete | `ls planning/completed/plan-24-F-*.md` → exists | MET (`planning/completed/plan-24-F-rtc-kind.md`, 2026-11-07 at 293d961) |

## 1. Goal

- `converter.json` loads; ADC device coverage complete; goldens +NADC, 0 removed; `ctest` T0 + 2.

### Non-goals

- No engine change (a needed one is a D/E bug: fixed there with a test, recorded).

## 2. Design

`converter.json`, `"kind": "converter"`, ADC devices grouped by interface
(SPI frame, SPI register, I2C, parallel); parts generated from `parts.tsv`
by a script under `/tmp/p24/` (not committed).

## Phases

### Phase 1 — Devices and goldens

- [x] Generate the ADC devices (`gen.py converter ADC` → `512 devices, 1187 parts`, ordered SPI frame, SPI register, I2C, parallel, clocked); `ctest -R library_check` → `100% tests passed`; coverage: parts.tsv ADC `add` rows vs `converter.json` → parts missing 0, extra 0, devices missing 0 (of 512).
- [x] Move the footprints the ADC parts use from `planning/plan-24-data/footprints-new.json` into `footprints.json` (plan-24-C Corrections); regenerate `footprints.txt` (+ that many lines, 0 removed); `footprint_check` passes. Moved 60 (25 stay staged for H); `git diff --numstat` footprints.txt → `60 0`; `footprint_check` passes.
- [x] Regenerate `parts.txt`, `traces.txt` (+NADC each, 0 removed): `git diff --numstat` → `1187 0` each; `grep -c '|converter|' parts.txt` → 1187. `listing_check` and `trace_check` sized their per-kind tables by `PART_HEADER` (the last kind before plan-24): now `PART_RTC` (Corrections).
- [x] Spot checks: `ADS1115IDGSR` (I2C, `VSSOP-10`), `ADS7042IDCUR`, `ADS1256IDBR`, one parallel ADC, one Last Time Buy part. parts.txt: `ADS1115IDGSR|converter|VSSOP|…|type=adc bits=16 channels=4|1:ADDR … 9:SDA:inout,10:SCL:input|footprint=VSSOP-10|vcc=2000-5500,in=5500`; `ADS7042IDCUR` 12 bits, 1 channel, `VSSOP-8`, vcc 1650-3600; `ADS1256IDBR` 24 bits, 8 channels, `SSOP-28`, vcc 1800-3600; parallel `ADC0802LCWM/NOPB` 8 bits, CS/RD/WR/INTR/DB0-7, `SOIC-20W`; Last Time Buy `ADS7834E/250` 12 bits, `VSSOP-8-DGK` (as parts.tsv; its status is not a flag); the obsolete flag shows on `ADC0848CCVX/NOPB` (`|obsolete|`).
- [x] Runtime proof: a `circuit_check` case with a 6502 test program (the plan-12 serial test circuits' pattern) that bit-bangs I2C on a W65C22 port to an ADS1115, starts a conversion, polls the config OS bit and reads 0x0000. `check_adc_from_cpu` (W65C02S, AT28C256, AS6C62256, W65C22S, ADS1115IDGSR, ECS-100A-010, io_board's decoding through the circuit API; a 464-byte program, listed in the test): `results: 00 05 03 85 00 00 00 AA` — every byte ACKed, first poll CONFIG 05xx (OS = 0), 3 busy polls, then 85xx, CONVERSION 0x0000; `9499 cycles`; all `adc board` checks PASS.

Acceptance: `ctest` T0 + 2 of T0 + 2; golden counts; the runtime case passes (seconds): `100% tests passed, 0 tests failed out of 26`; goldens +60/+1187/+1187, 0 removed; `circuit_check` `0 failure(s)`.
Commit: 284ef4e

## Validation Plan

G's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-24-A.

## Corrections

- **The golden checks sized per-kind tables by `PART_HEADER`**, the last
  `PartKind` before plan-24-D/F appended `PART_CONVERTER` and `PART_RTC`:
  with converter parts loaded `listing_check` crashed in `count_kind`
  (`kinds[PART_HEADER + 1]` overrun) and `trace_check` aborted. Both now
  size by `PART_RTC`. D/F could not see it: no converter or rtc part was
  in the library until G.
- **Footprints**: 60 of the 85 staged footprints are used by ADC parts and
  moved; 25 stay in `footprints-new.json` for H (rewritten with
  `indent=1`, so its diff is larger than the 60 removed entries).
- **The runtime board pulls PB2-PB7 low through `PULLDOWN`**, not GND:
  the CPU goes LOST reading a port byte with floating (X) bits, and tying
  the pins to the GND net put the VIA's pre-reset X outputs on ground
  (RES read X, the CPU never left LOST). The program bit-bangs I2C at
  DR 250 SPS (4 ms) so the polls see OS = 0 before it reads 1.

## Summary

G lands the ADCs on the tested engine and proves one end to end from a CPU.
