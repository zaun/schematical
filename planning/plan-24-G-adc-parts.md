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
| plan-24-F complete | `ls planning/completed/plan-24-F-*.md` → exists | NOT MET (2026-11-07) |

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

- [ ] Generate the ADC devices; `ctest -R library_check`; coverage (set difference → empty).
- [ ] Move the footprints the ADC parts use from `planning/plan-24-data/footprints-new.json` into `footprints.json` (plan-24-C Corrections); regenerate `footprints.txt` (+ that many lines, 0 removed); `footprint_check` passes.
- [ ] Regenerate `parts.txt`, `traces.txt` (+NADC each, 0 removed).
- [ ] Spot checks: `ADS1115IDGSR` (I2C, `VSSOP-10`), `ADS7042IDCUR`, `ADS1256IDBR`, one parallel ADC, one Last Time Buy part.
- [ ] Runtime proof: a `circuit_check` case with a 6502 test program (the plan-12 serial test circuits' pattern) that bit-bangs I2C on a W65C22 port to an ADS1115, starts a conversion, polls the config OS bit and reads 0x0000.

Acceptance: `ctest` T0 + 2 of T0 + 2; golden counts; the runtime case passes (seconds).
Commit: —

## Validation Plan

G's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-24-A.

## Corrections

## Summary

G lands the ADCs on the tested engine and proves one end to end from a CPU.
