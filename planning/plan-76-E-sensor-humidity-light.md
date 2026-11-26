# plan-76-E: Humidity and light: specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-D

E fills `specs-humidity.tsv` and `specs-light.tsv` by plan-76-C § 3's rules, then draws and
merges every new footprint those rows name. The two families are the smallest: humidity ≤ 92 and
light ≤ 71 rows (plan-76-A table), many in leadless packages with their own land patterns.

References:

- plan-76-C § 3 (rules), plan-76-D § 3.2 (footprints, `fp-merge76.py`), plan-76-L § 3.1 (keys:
  `humidity`, `light`, `infrared`, `ultraviolet`, `proximity`; outputs `capacitance` for passive
  elements).
- `docs/parts-format.md` § Footprints.

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-D complete | `ls planning/completed/plan-76-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| `fp-merge76.py` exists | `ls planning/plan-76-data/census/fp-merge76.py` → exists | NOT MET (written by D) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `specs-humidity.tsv` and `specs-light.tsv` passes `specs-check76.py`.
- Every new footprint id of those rows is drawn in `footprints-humidity.json` /
  `footprints-light.json`, merged into `footprints.json`, and its golden line added; no existing
  footprint line changes.

### Non-goals (explicit constraints)

- No change to `Parts/*.json` other than `footprints.json`; no backend or app change.
- A light sensor's optical window, lens or aperture is not drawn (copper and the courtyard only,
  as every footprint).

## 2. Current State

Parts seen at planning (the census pages' first 100 rows, read at planning and not kept):

| Maker | Rows | Parts |
|---|---|---|
| Sensirion (humidity) | 49 of 49 | SHT30, SHT30A, SHT31, SHT31A, SHT35 (DFN-8, `-ARP` analog / `-DIS` I2C), SHT40 (`-AD1B`, `-AD1F`, `-AD1P`, `-BD…`: address and option codes) and the rest of the SHT4x rows |
| Amphenol Telaire (humidity) | 20 of 21 | ChipCap 2 `CC2A23`…`CC2D35S` (SMD and `-SIP`), `HS-20`, `HS12SP`, `HS30P` |
| TE (humidity) | 12 of 13 | `HPP845E…` (capacitive element), `HTU20D`, `HTU20DF`, `10142048-xx` |
| IST (humidity) | 7 of 9 | `HYT 221`, `HYT 271`, `HYT 939` (modules), `MK33-W`, `K5` (capacitive elements), `P14 Rapid` |
| ams OSRAM (light) | 26 of 26 | `SFH 5701`, `SFH 5711`, `SFH 5721`, `SFH 7061`, `TSL25403`, `TSL25721`, `TCS37013`, `TMD26353`, `TMD27721`, `AS7261`, `AS7263`, `AS7341`, `TMF8701` |
| Vishay (light) | 22 of 22 | `VEML7700`, `VEML6030`, `VEML6035`, `VEML3235`, `VEML4031`, `VEML6031`, `VEML6046`, `VCNL4035`, `TEMT6202`, `TEMD6200`, `VEMD4200`, `TSSP95038` |
| Broadcom (light) | 12 of 12 | `APDS-9007`, `-9008`, `-9151`, `-9160`, `-9250`, `-9251`, `-9253`, `-9306`, `-9500`, `-9999`, `AFBR-S4P…` |
| TI (light) | 11 of 11 | `OPT101P`, `OPT3002`, `OPT3004`, `OPT3005`, `OPT3006`, `OPT3007`, `OPT4001`, `OPT4003` |

Light packages (`show.py raw/b536.json "Package / Case"`): 6-SMD no lead 11, 8-SMD module 9,
4-SMD no lead 8, 6-SMD module 5, 10-SMD module 4.

Some rows are not ambient-light sensors in the hobby sense but are in DigiKey's 536 and the
family's rules: TSSP95038 (an IR receiver for presence sensing), TMF8701 (a time-of-flight
proximity sensor: `measures` `proximity`), AS7261/AS7341 (spectral sensors: `light`).

## 3. Design

### 3.1 Humidity facts

- `measures`: `humidity`, plus `temperature` when the part reports it.
- `output`: `i2c`, `analog`, `pwm`, `capacitance` (the IST MK33 / K5 and TE HPP845 elements: two
  `inout` pins, no `vcc`, `ranges` humidity only).
- `ranges`: humidity in %RH (0–100 unless printed otherwise), temperature in °C.
- `accuracy`: the guaranteed ±%RH and ±°C (plan-76-C § 3.2).

### 3.2 Light facts

- `measures`: `light` (an ambient-light / lux sensor), `infrared`, `ultraviolet`, `proximity`,
  each as the datasheet's features list.
- `output`: `i2c`, `analog` (a photodiode or phototransistor current or voltage output), `pwm`.
- `ranges`: light in lux, the datasheet's maximum detectable illuminance; no range for `infrared`,
  `ultraviolet`, `proximity` (plan-76-L § 3.1: no unit).

### 3.3 Footprints

plan-76-D § 3.2. A part with a recommended land pattern in its datasheet (Sensirion, Vishay, ams
OSRAM, Broadcom print one) uses it; the package outline is used only when none is printed.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs

- [ ] Fill every row of `specs-humidity.tsv` and `specs-light.tsv`, their pinout files and their
      new footprint ids.

Acceptance: both families pass.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family humidity && python3 -I planning/plan-76-data/census/specs-check76.py --family light` → `<n> rows ok, 0 errors` twice (est. < 1 min).
  Result:
Commit:

### Phase 2 — Footprints

- [ ] Draw `footprints-humidity.json` and `footprints-light.json`; merge both; regenerate
      `backend/tests/golden/footprints.txt`; hand-check each added line.

Acceptance: every humidity and light footprint id exists, and the golden only grew.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check humidity && python3 -I planning/plan-76-data/census/fp-merge76.py --check light` → `0 missing` twice; `ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- E's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **humidity and light share a letter**: together they are under 170 rows, the smallest
  families, and both are mostly leadless parts with printed land patterns.
- Author: **passive humidity elements stay in**: they are board-mount sensors in DigiKey's 529;
  their `capacitance` output and two `inout` pins carry what a designer needs.
- Author: **every 536 row keeps its own `measures`**, so a time-of-flight or IR-receiver part is
  listed under Light with what it actually senses.

## Corrections

## Summary

E finishes the humidity and light specs and lands their footprints.
