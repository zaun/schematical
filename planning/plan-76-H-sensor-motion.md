# plan-76-H: Motion: specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-G

H fills `specs-motion.tsv` (accelerometers 515, gyroscopes 555 and IMUs 567: ADI ≤ 90, ST ≤ 28,
TE ≤ 22, Memsic ≤ 10; plan-76-A table) by plan-76-C § 3 and § 3 below, then draws and merges the
motion footprints. Most motion parts are LGA / LFCSP / CLCC packages with no footprint in the
library yet (`LGA` ids: 1, `LGA-6-MAXIM-Y61A1-1`).

References:

- plan-76-C § 3, plan-76-D § 3.2; plan-76-L § 3.1 (`acceleration` g, `rotation` °/s,
  `magnetic-field` mT, `pressure` kPa, `"axes"`).
- `docs/parts-format.md` § Footprints (`quad`, `explicit`).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-G complete | `ls planning/completed/plan-76-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `specs-motion.tsv` passes `specs-check76.py --family motion`, or is `skip` with a
  reason of § 3.2.
- Every new motion footprint id is drawn in `footprints-motion.json`, merged, and its golden line
  added; no existing footprint line changes.

### Non-goals (explicit constraints)

- No sensor-fusion, FIFO or interrupt-map facts: the kind has no keys for them.

## 2. Current State

Parts seen at planning (the census pages' first 100 rows, read at planning and not kept):

- ADI: accelerometers `AD22037`, `AD22293`, `ADXL1003`/`1004`/`1005`, `ADXL203`, `ADXL206`,
  `ADXL213`, `ADXL312`, `ADXL313`, `ADXL316`, `ADXL327`, `ADXL335`, `ADXL343`, … (59 rows of 515);
  IMUs `ADIS16470`, `ADIS16475`, `ADIS16477`, `ADIS16500`, `ADIS16505`, `ADIS16507` (9 rows of
  567); gyroscopes 13 rows of 555, mostly 32-BFCBGA (`show.py raw/m555.json "Package / Case"`).
- ST: `AIS25BA`, `AIS2DW12`, `AIS2IH`, `AIS328DQ`, `H3LIS100DL`/`200DL`/`331DL`, `IIS2DH`,
  `LIS2HH12`, `MIS2DH`, and NXP-heritage `FXLS8964`/`8967`/`8971`/`8974` (515); `ASM330LHB`,
  `ASM330LHH`, `ISM330DLC`, `ISM330ISN`, `LSM6DSL`, `LSM6DSM`, `LSM6DSV`, `LSM6DSV16B`/`16X`/`32X`
  (567).
- TE: `3038-…`, `805-0500-01`, `820M1-…`, `830M1-…` (board-mount accelerometer modules).
- Memsic: `MC3416`, `MC3419`, `MC3479`, `MC3630`, `MC3632`, `MC3635`, `MXC3638AL`, `MXC4005XC`,
  `MXC6655XA`.

DigiKey files 21 of the 42 board-mount IMU rows as module packages (`show.py raw/b567.json
"Package / Case" | grep -i module` → 100-BBGA Module 6, 14-VFLGA Module 3, 16-WFLGA Module 3,
44-BBGA Module 3, 18-SMD Module 2, 28-TFLGA Module 2, 44-BLGA Module 1, Module 1). The rule of
§ 3.2 sorts each module by its datasheet's mounting section: solder pads stay in, a mating
connector is a skip.

## 3. Design

### 3.1 Motion facts

- `measures`: `acceleration`, `rotation`, `magnetic-field`, `pressure`, `temperature`, as the part
  senses them (a 6-axis IMU: acceleration + rotation; a 9-axis: + magnetic-field).
- `extra`: `axes=<1|2|3>`, the axes per sensing element (an IMU with 3-axis accel and gyro: 3).
- `ranges`: the widest selectable full-scale range, symmetric: acceleration `[-g, g]`, rotation
  `[-dps, dps]`.
- `output`: `analog`, `i2c`, `spi`, `i3c`, `pwm` (ADXL213-style duty-cycle outputs).
- `accuracy`: absent (motion datasheets print sensitivity and noise, not a measurement accuracy).

### 3.2 Skips

- `connector-mounted module`: a module whose datasheet mounts it by a connector, not by solder
  pads (the ADIS IMUs above). Recorded in `verdicts.tsv` and `datasheets/skipped-parts.md`
  § "Sensors (plan-76)", and its PDF deleted unless another `add` row cites it.

### 3.3 Footprints

plan-76-D § 3.2: LGA and LFCSP from the datasheet's recommended land pattern (ADI, ST and Memsic
print one); `quad` with an unnumbered centre pad where the drawing has one; BGA parts with the
existing `grid` shape (`docs/parts-format.md` § `grid`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs

- [ ] Fill every row of `specs-motion.tsv` and its pinout files; the § 3.2 skips moved to
      `verdicts.tsv` and `skipped-parts.md`.

Acceptance: every motion row passes or is a recorded skip.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family motion` → `<n> rows ok, 0 errors`; `grep -c 'connector-mounted module' planning/plan-76-data/verdicts.tsv` equals the count in `datasheets/skipped-parts.md` § "Sensors (plan-76)" (est. < 1 min).
  Result:
Commit:

### Phase 2 — Footprints

- [ ] Draw `footprints-motion.json`; merge; regenerate `footprints.txt`; hand-check each added
      line.

Acceptance: every motion footprint id exists, and the golden only grew.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check motion` → `0 missing`; `ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- H's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **connector-mounted IMU modules are skipped**: a schematic part needs a footprint to
  solder, and theirs is a mating connector the datasheet names but does not sell with the part.
- Author: **ranges are the widest full scale**: a programmable range is a setting, and the widest
  one bounds what the part can measure.
- Author: **no accuracy for motion parts**, since their datasheets give none in the quantity's
  unit.

## Corrections

## Summary

H fills the motion specs, skips the connector-mounted modules, and lands the LGA / LFCSP / BGA
footprints the rest need.
