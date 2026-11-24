# plan-56-E: LCD segment drivers on I2C/SPI: PCF8576D, PCF85176, PCA85176, PCA85276, PCF8553, PCA8553, BU91795, BU9796

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-D

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

Parts and models for the bus-programmed LCD drivers: NXP PCF8576D, PCF85176, PCA85176 (automotive), PCA85276 (automotive) 40x4 I2C drivers (`pcf8576d.pdf`, `pcf85176.pdf`, `pca85176.pdf`, `pca85276.pdf`), PCF8553 and PCA8553 (I2C or SPI; `pcf8553.pdf`, `pca8553.pdf`), ROHM BU91795MUF-ME2 (`bu91795muf-m-e.pdf`) and BU9796A/BU97960 (`bu9796axxx-e.pdf`). A host sets mode (static, 1:2, 1:3, 1:4 multiplex), loads the display RAM through the bus (with the chip's data-pointer and sub-address rules), and the segment/backplane pins carry the `lcd` driver's slot waveforms; a `GENERIC-LCD-*` glass wired to them shows the RAM.

Measured: 8 PDFs (`ls datasheets/lcd-drivers | grep -cE "pcf8576|pcf85176|pca85176|pca85276|pcf8553|pca8553|bu9|bu91"` → 8).

## Phases

### Phase 1 — specs and stock
- [x] Safari stock check (PCF8576DT/2,518 and DU/2DA/2,02, PCF85176T/1Y, PCA85176H/Q900/1,5 (obsolete), BU91795MUF-ME2, BU9796AFS-E2, BU9796AMUV-E2, BU97960MUV-E2 were in stock; PCA85276, PCF8553, PCA8553 had no script hit — re-search Safari). `planning/plan-56-data/specs/lcd-bus.md`: command bytes (mode set, load data pointer, device select, bank select, blink), RAM size and addressing, I2C address straps, SPI framing for 8553/91795, which pins are segments and backplanes per package.
Acceptance: one section per family. Commit: see plan-56-F Corrections

### Phase 2 — devices and parts
- [x] `lcddriver.json` devices (driver `lcd`), parts, datasheets, footprints (TQFP-64, VSSOP-56, TSSOP-56, VQFN-24/48, SSOP-32 as the packages need), goldens; benches: PCF8576 writes a digit pattern and a 1:4 `GENERIC-LCD-8D-1:4` glass reports the cells.
Acceptance: `ctest --test-dir /tmp/logic-build` 100%; goldens additions only. Commit: see plan-56-F Corrections

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Shipped (lcddriver, one `PCF85176` device covers PCF8576D and PCF85176 TSSOP56/TQFP64): PCF8576DT/2,518, PCF85176T/1Y, PCA85176H/Q900/1,5 (obsolete); BU91795MUF-ME2; BU9796AFS-E2; BU9796AMUV-E2. No part in stock for PCA85276, PCF8553, PCA8553 (their PDFs went to not-included.md); BU97960MUV-E2 is a different sheet (8 backplanes) and is not modeled. `pcf8576-lcd-mode-after-max695x.patch` (mode and load by register, per-mode backplane slots) was applied.
- Not modeled: blink, bias/VLCD, cascade pins, the I2C NACK of another subaddress; the PCF/PCA 64 Hz frame for every mode.
- 2026-11-24 (follow-plan): PCA85276, PCF8553 and PCA8553 re-checked with `digikey.js --opt2`: `0 of 0 parts` each (not in DigiKey's catalog), so they stay out (moot with evidence). BU97960MUV-E2 is in DigiKey stock (2 units) but has 8 backplanes and its own datasheet, which is not in the tree (`bu9796axxx-e.pdf` is the BU9796A sheet): not added.

