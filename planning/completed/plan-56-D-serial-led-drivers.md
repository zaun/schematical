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
- [x] Safari stock check (MAX6950CEE+, EEE+ (Last Time Buy), MAX6951CEE+T, EEE+, MAX6955AAX+, AAX+T, ATL+, MAX6957AAI+T, AAX+, AAX+T, ATL+, ATL+T, AS1115-BSST, TLC6983ZXLR were in stock; IS31FL3707 had no DigiKey hit with the script — re-search Safari for "IS31FL3707"); `planning/plan-56-data/specs/serial-led.md`: register map, command byte format, decode modes, scan, key-scan registers as pins, per chip.
Acceptance: one section per chip family in the spec file. Commit: see plan-56-F Corrections

### Phase 2 — devices and parts
- [x] `leddriver.json`: devices (`digits` for 6950/51, 6955, AS1115; `matrix` for IS31FL3707, TLC6983; `ports` for 6957) with the converter `bus`/`registers`/`protocol` from the spec; parts; datasheets; footprints; goldens; a bench per device (write registers over the bus, read the scanned pins; MAX6955's fonts decode `ascii7`; TLC6983's PWM stored only).
- [x] TLC6983 (found unshipped by the follow-plan pass; its CCSI fits no converter bus style, so it is a display driver of its own: `ccsi`): `backend/src/models/display_ccsi.c`, the part `TLC6983ZXLR` (DigiKey Active, 2,315 in stock, `digikey.js --opt2` 2026-11-24), the footprint `NFBGA-96-ZXL`, `datasheets.json` `tlc6983` (`node scripts/check-datasheets.js tlc6983` -> `1 of 1 URLs serve a PDF`), docs, goldens, bench `tlc6983_test`: `/tmp/logic-build-p56/tlc6983_test` -> `tlc6983_test: all passed` (27 checks).
- [x] IS31FL3707 re-checked: no DigiKey listing (`--opt2` 2026-11-24 -> `0 of 0 parts`), so still not added (moot with evidence; its PDF is in `datasheets/not-included.md`).
Acceptance: `ctest --test-dir /tmp/logic-build-p56 -j8` -> `100% tests passed, 0 tests failed out of 60`; goldens additions only (TLC6983ZXLR: parts 1, traces 1, footprints 1). Commit: see plan-56-F Corrections; 52169311, a78dbed5, ba779a72 (TLC6983)

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Shipped: MAX6950 (CEE+, EEE+), MAX6951 (CEE+T, EEE+), MAX6957-20 (AAI+T), AS1115 (BSST). Not shipped: MAX6957 28-port and MAX6955 (AAX and ATL parts are in stock but their packages A36-2/A36-4 and T4066 have no drawing in the sheets and analog.com was not reachable, so there is no footprint); IS31FL3707 (no part in stock: `is31fl3707.pdf` went to not-included.md); TLC6983 (its CCSI interface fits no converter bus style: spec only, `planning/plan-56-data/specs/is31fl3707-tlc6983.md`). The `max695x-display.patch` engine additions (shared digit/segment pins, scan limit, blink, port sink/pull-up/read-back modes) were applied.
- AS1115 models Code B decode only; MAX6955's 16-segment font is transcribed in `planning/plan-56-data/results/max6955-as1115.json`.
- Later: MAX6954 (AAX+, ATL+), MAX6955 (AAX+, AAX+T, ATL+) and the 28-port MAX6957 (AAX+, AAX+T, ATL+, ATL+T) are shipped, with `SSOP-36-A36` (Maxim 21-0040 outline, 90-0098 land pattern) and `TQFN-40-T4066` (21-0141, 90-0055) drawn from the maker's drawings; bench `backend/tests/max6954_max6955_test.c`, and `max6957_test.c` checks the AAX and ATL pinouts again. The MAX6954 (4-wire SPI, frame and read-back as the MAX6957) has the MAX6955's registers; its 0x20-0x2F and 0x40-0x4F plane writes are one command each, the key masks are stored, and R (global clear of both planes) is modeled; the MAX6955's I2C pointer commands cannot reach R or the masks, so there they are not modeled. The MAX6955's TQFN pins P2 and P3 (33 and 34) were swapped in the first transcription.
- Not modeled in MAX6954 / MAX6955 (still beyond the engine): the LED outputs O0-O18, BLINK and OSC_OUT stay released. The `digits` driver's `sharedPins` takes exactly nine pins with one eight-segment line map (MAX6950/51), while these chips share 19 pins: in scan period k pin Ok is the cathode and the other 17 pins except the slot's two cathode pins carry the segments, the order of those 17 given by the slot's digit type (a 16-segment, 14-segment or pair of 7-segment list, Tables 2-4 and 37-40, picked per slot by the digit-type register) and the font (`ascii14` / `ascii16` decodes, tables in `planning/plan-56-data/results/max6955-as1115.json` engineNeeds); a device also has one `display` driver, so the GPIO `ports` driver (modeled) and the scan cannot both be present. The datasheet does not say what separates a 16-segment digit from a pair of 7-segment digits for a clear digit-type bit. Also not modeled: blink (phase, the T and P bits), intensity, scan limit, display test and shutdown on the pins, key scanning (Key_A..D read 0, port bits D7:D5 do not turn P0-P3 into keys or P4 into IRQ), reading 0x0C returns Key_A pressed (0) on the MAX6954 and the stored digit type on the MAX6955.
- 2026-11-24 (follow-plan): TLC6983 is shipped (supersedes "TLC6983 ... spec only" above). `display.driver` `ccsi` (docs/parts-format.md § `leddriver` `ccsi` driver): SCLK / SIN / SOUT frames (START, 17-bit words with the check bit, END = 18 highs starting on a word boundary: a check bit of 1 next to the END's highs must not count as part of the run, found by the bench), FC0-FC4 / FC10 / FC11 with the reset values of the register figures (FC0 1000 700F 0107h), FC0 and FC1 applied at VSYNC (p.50), reads answered on SOUT, WRTGS through the line and channel counters into the bank that is not displayed, VSYNC bank swap, Soft_Reset, and a scan of SCAN_NUM + 1 lines (one LINE low, the others released) with each pixel's R / G / B high while its 16-bit gray is nonzero. A slot's outputs are computed when the slot starts (9 us). Not modeled: the SOUT pass-through of a chain, DS-PWM gray levels, current setting, LED open / short detection, power saving, protection. VR, VG, VB and IREF are `input` pins listed as `analog` (the engine's `domains` exist only for the `buffer` kind).

