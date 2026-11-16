# plan-43-D: Reference-board parts — 2 real-time clocks and 4 converters

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-43-C

D adds the 6 `letter` = `D` rows of `planning/plan-43-data/targets.tsv` as
declarative devices: `DS3231M` and `PCF8563` in `Parts/rtc.json`; `MCP3428`
(ADC), `DS4432`, `AD5621` and `MCP47A1` (DACs) in `Parts/converter.json`.
The behavioral outcome: `listing_check` lists 6 more parts (32895 → 32901);
`circuit_check` shows each RTC counting from 2000-01-01 00:00:00 and each
converter's registers at their power-on values; `ctest` passes 36 programs.

References:

- `docs/parts-format.md` § `converter` (device keys, `bus`, `registers`,
  `protocol`, `conversion`, `dac`, pin roles and the `"analog"` list),
  § `rtc` (`clock`).
- `planning/plan-24-data/schema.md` (the authoring rules for transcribing a
  datasheet) and `specs.md`.
- Precedents in the part files: `BQ32000` (I2C RTC with a register pointer),
  `ADS1110` (I2C ADC read as data then configuration, no register pointer),
  `DAC7311` (16-bit SPI write-only frame), any `("dac", "i2c")` device.
- `backend/tests/circuit_check.c` `check_rtc_time` (the real-part RTC test)
  and the `ADS1115IDGSR` board (lines 1184–1252).
- The six datasheets (`targets.tsv` column `pdf`).

## Prerequisites

plan-43-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-43-C complete | `ls planning/completed/plan-43-C-*.md` → exists | NOT MET (2026-11-15: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `wc -l < backend/tests/golden/parts.txt` → 32901; no golden line removed.
- `ctest --test-dir /tmp/logic-p43 -R circuit_check` passes with the new
  cases of § 5.
- `grep -c '|rtc|' backend/tests/golden/parts.txt` grows by 2 and
  `grep -c '|converter|' …` by 4.

### Non-goals (explicit constraints)

- No change to `converter.c`, `converter_load.c`, `converter_par.c`,
  `rtc.c` or the schema: a device feature the engine cannot express is left
  out of the device and named in its `note` (§ 4 lists them).
- No analog behaviour: an ADC returns the code of 0 V, a DAC stores its
  code (§ `converter`).
- No other part of these datasheets (other packages, grades or siblings
  such as AD5601 / AD5611, MCP47A1 address options, DS3231MPMB1).

## 2. Current State

`converter.json` holds 764 devices on four bus styles (`python3 -I -c
"import json,collections; print(collections.Counter((x['type'],
x['bus'].get('style')) for x in
json.load(open('schematical/schematical/Parts/converter.json'))['devices']))"`
→ adc/spi 345, dac/spi 164, adc/clocked 77, adc/parallel 58, dac/parallel
55, dac/i2c 33, adc/i2c 32); `rtc.json` holds 7 (`BQ32000`, `BQ32002` on
I2C; five parallel). Device id = the base product; drawer `["Interface"]`
with function `ADC` / `DAC` / `RTC`.

### Verified properties

- **Register maps and power-on values** (read with `pdftotext -layout`,
  2026-11-15):
  - `MCP3428` (`22226a.pdf` § 4.1.1, § 5.2): one configuration byte, read
    after the data bytes; power-on: channel 1, continuous conversion, 240 SPS
    (12 bits), gain ×1, with the ready bit as the datasheet's register
    figure gives it.
  - `MCP47A1` (`jp559281.pdf` p.1, p.8 "Wiper POR/BOR 20h"): one 6-bit
    wiper register at command code 00h, power-on 20h (mid-scale); write
    and read with the command code.
  - `DS4432` (`DS4432.pdf` p.7–8): registers F8h (OUT0) and F9h (OUT1),
    sign bit and 7-bit magnitude; "On power-up the DS4432 outputs zero
    current".
  - `PCF8563` (`PCF8563.pdf` Table 4): registers 00h–0Fh; time counters at
    02h (`VL_seconds`) through 08h (years); `Control_status_1` bit `STOP`.
  - `DS3231M` (`DS3231M.pdf` register map): seconds at 00h, I2C pointer
    with auto-increment as `BQ32000`, oscillator enable bit `EOSC`.
    `AD5621` (`AD5601_5611_5621.pdf` input register figure): one 16-bit
    write-only frame as `DAC7311`.
  - I2C addresses: `PCF8563` A2h / A3h (7-bit 51h, p.1), `DS4432` 90h
    (7-bit 48h, p.8), `MCP47A1` option A0 `0101110` (2Eh, § 5 control
    byte), `MCP3428` device code 1101 with three bits from `Adr0` / `Adr1`
    (68h–6Fh), `DS3231M` 1101000 (68h).
- **The engine has each shape**: the three precedents above are in the part
  files and pass `trace_check` today.

## 3. Design Overview

Six devices transcribed by `schema.md`'s rules, each copied from the
precedent of its shape and corrected field by field against its datasheet.
The risk is a register or protocol detail the engine reads differently from
the chip; § 5's real-part cases check the visible ones (time counting,
power-on register values).

## 4. Detailed design

Per device: `id` (column `device`), `function`, `description`, `drawer`
`["Interface"]`, `note` (file and pages), `pinouts` (the row's pinout name,
roles per § `converter`: supplies `power`, grounds `ground`, analog pins in
`"analog"`), `vcc` and `inputMax` from the recommended operating conditions,
then:

| Device | Bus | Device-specific |
|---|---|---|
| `DS3231M` | I2C, address 68h, register pointer with auto-increment | `clock.fields` seconds … year at 00h–06h (BCD, 12/24-hour bit, century bit), two alarms on the `INT/SQW` pin (open drain), `stop` = the `EOSC` bit (active-low enable: `running` form), the status register's `OSF`; left out and named in the `note`: temperature registers (read as 0), aging offset, 32 kHz output and the 1 Hz square wave |
| `PCF8563` | I2C, address 51h, register pointer with auto-increment | `clock.fields` at 02h–08h (BCD; `VL` bit separate), `stop` = `Control_status_1.STOP`, one alarm (minute, hour, day, weekday with their `AE` mask bits, `maskLevel` 1) and the timer flag on `INT` (open drain); left out and named: `CLKOUT`, the countdown timer's counting |
| `MCP3428` | I2C, address 68h–6Fh by the two address pins (the device's `Adr0` / `Adr1` as `bus` address pins; floating and the tied states the engine cannot tell apart use the first value) | `type` `adc`, `bits` 16, `channels` 4, `codeFormat` `twos`; one configuration register; read = data bytes then the configuration byte, as `ADS1110`; `conversion` rates 240 / 60 / 15 SPS by the rate field, one-shot by the `O/C` and ready bits |
| `DS4432` | I2C, address 48h (7-bit; the datasheet's 90h) | `type` `dac`, `bits` 7, `channels` 2; registers F8h, F9h; `dac` registers the two magnitudes (the sign bit is a register field) |
| `AD5621` | SPI, write-only, 16-bit frame on `SYNC` low | `type` `dac`, `bits` 12, `channels` 1; frame = 2 mode bits, 12 data bits, 2 unused, as the datasheet's input register figure; power-down modes stored, not acted on |
| `MCP47A1` | I2C, address 2Eh for the `A0` option (the part number's `A0`) | `type` `dac`, `bits` 6, `channels` 1; register 00h, power-on 20h |

Parts: `name` = the row's part number, manufacturer `Analog Devices`
(DS3231MZ+TRL, DS4432U+, AD5621AKSZ-REEL7), `NXP` (PCF8563T/5,518),
`Microchip Technology` (MCP3428-E/ST, MCP47A1T-A0E/LTY) — the spellings the
library already uses for these makers (`grep -o '"manufacturer": "[^"]*"'
schematical/schematical/Parts/*.json | sort | uniq -c`), package and
footprint by plan-43-B § 4.3 against column `footprint_candidates`,
datasheet id column `datasheet_id`.

## 5. Tests

In `backend/tests/circuit_check.c`:

- `check_rtc_time` becomes a helper taking the part name, the names of its
  data, clock and interrupt pins and the addresses of its seconds and
  minutes registers; `main` calls it for `BQ32000DR` (as today),
  `DS3231MZ+TRL` (00h, 01h) and `PCF8563T/5,518` (02h, 03h). Same
  expectations: refused before start; 2000-01-01 00:00:00 Saturday running;
  2 s at 2.5 s; 00:01:02 a minute later; the two registers 02 and 01 in BCD
  (masked with 7Fh).
- `check_reference_converters`: one board with `MCP3428-E/ST`, `DS4432U+`,
  `AD5621AKSZ-REEL7` and `MCP47A1T-A0E/LTY` powered from `3V3`, their bus
  pins on pull-ups; after `circuit_start`, `circuit_converter_registers`
  shows `MCP47A1`'s register 00h = 20h, `DS4432`'s F8h and F9h = 00h, and
  `MCP3428`'s configuration register at its power-on value; the AD5621
  starts without a `smoke:` or voltage problem.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Real-time clocks

- [ ] Add `DS3231M` and `PCF8563` with their parts, footprints and
      datasheet entries (§ 4).
- [ ] Tests: the `check_rtc_time` helper and its three calls (§ 5).
- [ ] Regenerate the goldens.

Acceptance: both RTCs count from 2000-01-01 and show their BCD registers.
  Check: `cmake --build /tmp/logic-p43 -j8 && ctest --test-dir /tmp/logic-p43 -R 'library_check|listing_check|trace_check|footprint_check|rtc_test|circuit_check'`
  → `100% tests passed` (est. 2 min).
Commit: —

### Phase 2 — Converters

- [ ] Add `MCP3428`, `DS4432`, `AD5621` and `MCP47A1` with their parts,
      footprints and datasheet entries (§ 4).
- [ ] Tests: `check_reference_converters` (§ 5).
- [ ] Regenerate the goldens.

Acceptance: the four power-on expectations of § 5 hold; `parts.txt` has
32901 lines.
  Check: the Phase 1 command with `converter_test` added; `wc -l <
  backend/tests/golden/parts.txt` → 32901 (est. 2 min).
Commit: —

## Validation Plan

- Tests: `circuit_check` gains two RTC runs and one converter board;
  `trace_check` records each device's bus behaviour in `traces.txt`.
- Coverage check: 6 new lines in `parts.txt` and `traces.txt`.
- Doc sync (in E): `docs/parts-format.md` § `rtc` names no device list, so
  nothing; `docs/backend.md` nothing.
- Final gate: plan-43-E.

## Decisions

- Features the engine cannot express are left out and named in the device
  `note` rather than extending the engine — author; plan-43 adds no model
  code (plan-43-A § 1).

## Corrections

## Summary

Six transcriptions against one engine. The checks cover what a user sees
first: the clocks run and the registers start at the datasheet's values.
