# plan-56-F: Other LCD and display chips: ICM7231/7232, MAX7231–34, CD22105, ICM7224/7225, PCF1179C, SN74143

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-E

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

The remaining display ICs: ICM7231/7232 (Harris triplexed numeric/alphanumeric LCD drivers; `icm7231-icm7232-harris.pdf`), MAX7231–MAX7234 (`max7231-max7234.pdf`, `max7231-max7234-maxim-old.pdf`), CD22105/CD22105A (4-digit LCD decoder-drivers; `cd22105-harris.pdf`), ICM7224/ICM7225 (4½-digit counter, LCD / LED; `icm7224-icm7225-maxim.pdf`, `icm7224-icm7225-harris.pdf`), PCF1179C (4-digit duplex LCD car clock; `pcf1179c.pdf`) and the SN74143 (4-bit counter/latch with 7-segment driver; `counters/sn74143.pdf`, needs `PDIP-24-600` footprint).

Measured: 7 PDFs (`ls datasheets/lcd-drivers datasheets/led-drivers | grep -ciE "icm7231|max7231|cd22105|icm7224|pcf1179"` → 7) plus `counters/sn74143.pdf`.

## Phases

### Phase 1 — specs and stock
- [x] Safari stock check (ICM7232AFIPL, BFIPL, FIPL; MAX7231BFIPL+, BFIQH+D, CFIPL+; CD22105AE; ICM7224IJL, IPL+; PCF1179CT,118; SN74143N (obsolete at TI, 1,397 in DigiKey stock on 2026-11-23); ICM7225, ICM7231, MAX7232–34 had no script hit — re-search Safari). `planning/plan-56-data/specs/other-lcd.md`.
Acceptance: one section per chip. Commit: —

### Phase 2 — devices and parts
- [~] `lcddriver.json`: ICM7231/32, MAX7231–34 (driver `lcd`, parallel/serial bus), CD22105 (`static` with BCD decode), ICM7224 (`static` counter with a `counter` register: the 4½-digit count increments on COUNT; the 7225 on LED outputs), PCF1179C (a clock `registers` + `lcd` duplex drive).
- [ ] SN74143: extend `counter` `seg7` model with `segmentsActive`, the latched Q outputs, BI/RBO, DP pass-through and MAX COUNT (new keys, Part Editor `EXTRA`, tests, docs), then the part.
- [x] Footprints, datasheets, goldens, benches.
Acceptance: `ctest --test-dir /tmp/logic-build` 100%; goldens additions only. Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Phase 1 done (specs for all six chips). Phase 2 is partly done: shipped ICM7232A/B (AFIPL, BFIPL), MAX7231B/C (BFIPL+, BFIQH+D, CFIPL+), CD22105A (CD22105AE), ICM7224 (IJL, IPL+, IQH+D obsolete). Not shipped: ICM7231 and MAX7232 (no part in stock), MAX7233/7234 (character-ROM decode not transcribed: the glyph scans are not readable to cell level), ICM7225 (no part in stock), PCF1179C (the engine has no bus-less device, no `clock` + `display` combination and no BCD decode in the `lcd` driver: exact needs in `planning/plan-56-data/specs/pcf1179-sn74143.md`), SN74143 (the counter `seg7` model lacks `segmentsActive`, open-drain outputs, a second enable, MAX COUNT, latched outputs, BI, RBI/RBO and DP: same spec; footprint PDIP-24-600 drawn there). The PDFs `pcf1179c.pdf`, `counters/sn74143.pdf` and `tlc6983.pdf` stay in the tree for the work that remains.
- Remaining work list (a follow-on plan): MAX6955/MAX6957 28-port footprints (Maxim 21-0040/90-0098, 21-0141/90-0055), PCF1179C and SN74143 engine work, TLC6983's CCSI bus, the Code B selection by register/pin, `lcd`-driver extra output pins (ICM7224 CARRY, LZB OUT).
