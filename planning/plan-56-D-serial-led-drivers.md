# plan-56-D: Serial LED drivers: MAX6950/51, MAX6955, MAX6957, AS1115, IS31FL3707, TLC6983

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-C

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

Parts and models for the register-programmed LED drivers: MAX6950/MAX6951 (SPI, 5/8 digits; `max6950-max6951.pdf`), MAX6955 (I2C, 16 digits 7-segment or 8 digits 14/16-segment with fonts, GPIO and key scan; `max6955.pdf`), MAX6957 (SPI, 28 ports; `max6957.pdf`), AS1115 (I2C, 8 digits, 64 LEDs, key scan; `as1115.pdf`), IS31FL3707 (I2C 8x8 / 7x9 matrix with PWM and current scaling; `is31fl3707.pdf`), TLC6983 (48x16 common-cathode matrix, serial; `tlc6983.pdf`, 70 pages).

Measured: 6 PDFs (`ls datasheets/led-drivers | grep -cE "max695|max6957|as1115|is31|tlc6983"` → 6).

## Phases

### Phase 1 — specs and stock
- [ ] Safari stock check (MAX6950CEE+, EEE+ (Last Time Buy), MAX6951CEE+T, EEE+, MAX6955AAX+, AAX+T, ATL+, MAX6957AAI+T, AAX+, AAX+T, ATL+, ATL+T, AS1115-BSST, TLC6983ZXLR were in stock; IS31FL3707 had no DigiKey hit with the script — re-search Safari for "IS31FL3707"); `planning/plan-56-data/specs/serial-led.md`: register map, command byte format, decode modes, scan, key-scan registers as pins, per chip.
Acceptance: one section per chip family in the spec file. Commit: —

### Phase 2 — devices and parts
- [ ] `leddriver.json`: devices (`digits` for 6950/51, 6955, AS1115; `matrix` for IS31FL3707, TLC6983; `ports` for 6957) with the converter `bus`/`registers`/`protocol` from the spec; parts; datasheets; footprints; goldens; a bench per device (write registers over the bus, read the scanned pins; MAX6955's fonts decode `ascii7`; TLC6983's PWM stored only).
Acceptance: `ctest --test-dir /tmp/logic-build` 100%; goldens additions only. Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

(none yet)
