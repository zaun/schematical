# plan-56-B: Driver foundation: kinds `leddriver` and `lcddriver`, the `display` section, the output drivers

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-A

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

Two kinds, `leddriver` (`leddriver.json`, drawer Displays & Indicators › LED drivers) and `lcddriver` (`lcddriver.json`, Displays & Indicators › LCD drivers), built on the converter engine exactly as `rtc` is (`backend/src/models/rtc.c`, `converter_load.c:cv_load_device(ctx, json, rtc)`): the device describes its `bus`, `registers`, `protocol` as `docs/parts-format.md` § `converter` says, and a new `"display"` section says what its pins do with the register contents. A circuit with a MAX6951 on SPI shows the digits a host wrote on a `7seg` display; a circuit with a PCF8576 on I2C shows what a host wrote on an `lcd` glass.

### Non-goals

- `converter` and `rtc` devices, parts and goldens are unchanged (their loader path gets one more caller, not a change).
- No analog behaviour: LED current, PWM duty (stored, not drawn), LCD bias and contrast levels.

## 2. Current State

- The converter engine already runs I2C (`style: "i2c"`), SPI, `i2c-spi` and parallel buses with registers and bit-pattern commands: `docs/parts-format.md` § `converter` `bus`, `registers`, `protocol`; `backend/src/models/converter_par.c` (parallel), `serial_target.c` (SPI/I2C front end).
- `rtc` is the precedent for a second kind on this engine: `RTC_KIND` in `kinds.c`, `cv_load_device(..., rtc=1)`.
- Measured: the 30 driver PDFs (`ls datasheets/led-drivers datasheets/lcd-drivers | grep -c pdf` → 30) cover 8 buses: parallel microprocessor ports (ICM7218C/D, ICM7228, ICM7211/12, ICM7244/45, ICM7231/32 parallel, MAX7231), SPI/QSPI (MAX6950/51, MAX6957, MAX7232, TLC6983, PCF8553, PCA8553, BU91795), I2C (MAX6955, AS1115, IS31FL3707, PCF8576D, PCF85176, PCA85176, PCA85276, PCF8553, PCA8553, BU9796), serial 3-wire inputs (ICM7218A/B, CD22105, PCF1179C buttons only).

## 3. Design Overview

`"display"` names one **output driver** from a closed list implemented in C (`backend/src/models/display_out.c`), with parameters in JSON; each driver reads the engine's registers and drives pins on a scan timer:

| driver | what it does | used by |
|---|---|---|
| `digits` | multiplexed digit scan: for each of `n` digits, drive the digit pin (active level `digitActive`) and the 7/14/16 segment pins from the digit's RAM byte, through a decode (`none`, `hex`, `codeB`, `ascii7`, `ascii16`, `bcd`); blank, test, intensity (stored only), shutdown | ICM7218/7228, MAX6950/51/55, AS1115, ICM7244/45 |
| `static` | non-multiplexed outputs: a pin per segment from RAM bits (7211/7212, 7224/7225 counters, CD22105 after decode) | ICM7211/12/24/25 |
| `matrix` | row/column scan or constant-current columns from a bit RAM | IS31FL3707, TLC6983, MAX6957 (as ports) |
| `ports` | each port pin a latched output/input per a register bit | MAX6957, MAX6955 GPIO |
| `lcd` | backplane/segment drive from a display RAM: `mode` static/1:2/1:3/1:4; backplane pins driven in turn (others Z), segment pins carry RAM bit XOR backplane level | PCF/PCA 8576 family, 8553, BU91795, BU9796, MAX7231–34, ICM7231/32 |

Scan timing is real simulated time (the chip's oscillator period, from the datasheet, in the `display` section), so an LED display shows a multiplexed digit as `7seg` does ("on at any time during the last step"). Correctness risk concentrates in the `lcd` driver's slot sequence and in the converter loader accepting a device with a `display` section but no conversion: both are covered by fixtures before any real part uses them.

Rejected: one model per chip in C (about 25 models; the converter engine already does the bus and registers declaratively); putting the drivers into `serial` (wrong drawer, no register engine).

## Phases

### Phase 1 — Safari lookup helper
- [ ] `scripts/safari-digikey.sh URL [wait] [js]` (the osascript helper used during planning), documented in `AGENTS.md` § Datasheets as the way to look up DigiKey stock.
Acceptance: `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/lcd-oled-character-and-numeric/99" 8 'document.title'` → prints a title containing "LCD, OLED Character and Numeric" (est. 20 s). Commit: —

### Phase 2 — kinds and loader
- [ ] `parts.h` `PART_LEDDRIVER`, `PART_LCDDRIVER`; `leddriver.c`, `lcddriver.c` (`PartKindOps` forwarding to `cv_load_device(..., rtc=0, display=1)`), `kinds.c`, CMake, docs, the Part Editor recipe items for both kinds, `tests/support/listing.c`, `TRACE_LAST_KIND`.
- [ ] `converter_load.c`: accept the `"display"` object for these kinds only; reject it on `converter`/`rtc`; reject an unknown `driver`, unknown register/pin names.
Acceptance: `/tmp/logic-build/library_check` passes with the new rejection cases (est. 30 s). Commit: —

### Phase 3 — output drivers
- [ ] `display_out.c`: `digits`, `static`, `matrix`, `ports`, `lcd` as in the table, each with a fixture device in `backend/tests/display_out_test.c` (an SPI digit driver on a 4-digit `7seg`; an I2C 1:4 LCD driver on `GENERIC-LCD-8D-1:4`).
Acceptance: `ctest --test-dir /tmp/logic-build -R display_out_test` passes (est. 1 min). Commit: —


## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

(none yet)
