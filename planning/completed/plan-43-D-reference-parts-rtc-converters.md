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
| plan-43-C complete | `ls planning/completed/plan-43-C-*.md` → exists | MET (2026-11-15: archived in 6f1be566) |

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

- [x] Add `DS3231M` and `PCF8563` with their parts, footprints and
      datasheet entries (§ 4). — `python3 -I planning/plan-43-data/add/mkrtc.py`
      (writes `add/rtc.json`), `devadd.py add/rtc.json` → `datasheets +2`,
      `footprints +2`, `rtc +2 devices`; `library_check: all passed`.
- [x] Tests: the `check_rtc_time` helper and its three calls (§ 5). —
      `circuit_check`: the seven RTC expectations PASS for `BQ32000DR`,
      `DS3231MZ+TRL` (`SECONDS=02 MINUTES=01`) and `PCF8563T/5,518`
      (`VL_SECONDS=82 MINUTES=01`); `0 failure(s)`.
- [x] Regenerate the goldens. — 32894 / 32894 / 1645 lines; 0 removed;
      `|rtc|` 14 → 16.

Acceptance: both RTCs count from 2000-01-01 and show their BCD registers.
  Check: `cmake --build /tmp/logic-p43 -j8 && ctest --test-dir /tmp/logic-p43 -R 'library_check|listing_check|trace_check|footprint_check|rtc_test|circuit_check'`
  → `100% tests passed` (est. 2 min). — `100% tests passed, 0 tests failed
  out of 6`.
Commit: 09a77e73

### Phase 2 — Converters

- [x] Add `MCP3428`, `DS4432`, `AD5621` and `MCP47A1` with their parts,
      footprints and datasheet entries (§ 4). — three added (`MCP47A1` is
      skipped, § Corrections): `mkconv.py`, `devadd.py
      add/converters.json` → `datasheets +3`, `footprints +1`, `converter
      +3 devices`; `library_check: all passed`.
- [x] (added) Skip `MCP47A1T-A0E/LTY`: `census/skip.tsv`, `targets.tsv`,
      its PDF, a row in `datasheets/skipped-parts.md`; `classify.py` →
      `plan-43 43`, `skipped 9`; `lookup.py` → `43 rows; 0 not found; 0
      with no stock`.
- [x] Tests: `check_reference_converters` (§ 5). — `circuit_check`: the
      board starts; `MCP3428 CONFIG=90 DS4432 F8=00 F9=00 AD5621 DAC=000`;
      four PASS lines; `0 failure(s)`.
- [x] Regenerate the goldens. — 32897 / 32897 / 1646 lines; 0 removed;
      `|converter|` 1911 → 1914.

Acceptance: the four power-on expectations of § 5 hold; `parts.txt` has
32901 lines.
  Check: the Phase 1 command with `converter_test` added; `wc -l <
  backend/tests/golden/parts.txt` → 32901 (est. 2 min). — `100% tests
  passed, 0 tests failed out of 7`; 32897 (§ Corrections).
Commit: 9830cb18

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

- **D starts at 32892 and ends at 32898** (plan-43-B and -C § Corrections).
  Every 32895 above reads 32892 and every 32901 reads 32898. The build
  directory is `/tmp/logic-p43w`.
- **`rtc.json` and `converter.json` are edited by
  `planning/plan-43-data/add/devadd.py`**, which appends a device as text:
  both files are in `json.dump(indent=1)` layout but do not reload
  byte-for-byte, so they are not rewritten whole. The devices are built by
  `add/mkrtc.py` and `add/mkconv.py`, which hold the transcription.
- **`DS3231M` has no `stop`.** § 4 named `EOSC` as the stop bit, but the
  datasheet (p.13) says it stops the oscillator only when the part runs
  from VBAT; on VCC the oscillator always runs. The model has no battery
  supply, so `EOSC` is a stored bit and the clock always counts.
- **`DS3231M` is 24-hour only.** Bit 5 of its hours registers is the
  20-hour digit in 24-hour mode and AM/PM in 12-hour mode; the engine
  rejects overlapping fields (`converter_load.c`: "overlaps another
  field"), so `h12` / `pm` cannot name it. The 12/24 bits are stored and
  the device `note` says so. Its alarms match the date only (DY/DT stored).
- **`PCF8563` alarm with every `AE` bit 1**: the chip raises no alarm, the
  engine treats four masked entries as a match, so `AF` sets each second
  (INT stays released while `AIE` = 0). In the device `note`.
- **Two new SO-8 footprints.** `SOIC-8-MAXIM-S8` from Maxim's land pattern
  90-0096 rev M (1.98 x 0.53 pads, span 4.93; the datasheet p.19 names it
  and prints none) and `SOIC-8-NXP-SOT96-1` from the SOT96-1 outline by R2
  (1.80 x 0.54, span 5.40). `SOIC-8` (TI) has 1.55 x 0.6 pads.
  analog.com answers Python's `urllib` and not `curl`; the land-pattern
  documents are cited by URL, as plan-12-J did, not filed.
- **`DS3231MZ+TRL`**: the ordering table lists `DS3231MZ+`; `+TRL` is its
  tape and reel, as the library's `MAX…+T` parts.
- **D adds 5 parts, not 6: `MCP47A1T-A0E/LTY` is skipped; `parts.txt`
  ends at 32897.** Its datasheet (DS25154A p.67 Product Identification
  System) knows the package code `LT` only (`MCP47A1T-A0E/LT`); `/LTY` is
  not in it, the same reason plan-40 skipped parts. plan-43 as a whole
  adds 43 parts (14 diodes, 10 transistors, 7 oscillators, 3 switches,
  3 converters, 2 buffers, 2 connectors, 2 clocks). The `MCP47A1`
  expectation of § 5 falls away with it; `|converter|` grows by 3.
- **`MCP3428`'s test reads `CONFIG` masked with 7Fh**: bit 7 is the ready
  status, 1 at power-on (90h, as observed) and 0 once the first continuous
  conversion is done. The address table has entries for the four tied
  states of `Adr0` / `Adr1` (68h, 6Ch, 6Ah, 6Eh); the floating states have
  none (the engine tells ties from levels, § `converter`), and the test
  ties both pins low.
- **Footprints**: `MCP3428-E/ST` reuses `TSSOP-14` (Microchip C04-2087A:
  0.45 x 1.45 pads, C1 5.90; each centre 0.05 from 5.8); `DS4432U+`
  reuses `VSSOP-8-DGK` (Maxim 90-0092 rev D: 1.35 x 0.40 pads, span 4.30;
  0.05 from 1.4 x 0.45 at 4.4); `AD5621AKSZ-REEL7` gets `SC70-6-ADI-KS-6`
  (outline only, R2: 1.26 x 0.35, span 2.14).
- **`check_rtc_time` finds the two registers by address** (the PCF8563's
  are not the first two of its register list) and reads up to 32.

## Summary

Six transcriptions against one engine. The checks cover what a user sees
first: the clocks run and the registers start at the datasheet's values.
