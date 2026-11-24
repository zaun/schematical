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
- [x] SN74143: extend `counter` `seg7` model with `segmentsActive`, the latched Q outputs, BI/RBO, DP pass-through and MAX COUNT (new keys, Part Editor `EXTRA`, tests, docs), then the part. `ctest --test-dir /tmp/logic-build-sn74143 -j8` -> `100% tests passed, 0 tests failed out of 58` (new `counter_sn74143_test`: 42 checks; `counter_test` case 54: loader rules); `git diff --stat backend/tests/golden` -> 3 files, 3 insertions(+), 0 deletions (SN74143N line in parts/traces, PDIP-24-600 in footprints); `xcodebuild test -only-testing:schematicalTests/PartKindKeysTests`, `PartKindRoundTripTests` (40 tests, counter 733 parts, 0 mismatched / extras / rejected), `WizardStepsTests` -> all passed.
- [x] Footprints, datasheets, goldens, benches.
Acceptance: `ctest --test-dir /tmp/logic-build` 100%; goldens additions only. Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Phase 1 done (specs for all six chips). Phase 2 is partly done: shipped ICM7232A/B (AFIPL, BFIPL), MAX7231B/C (BFIPL+, BFIQH+D, CFIPL+), CD22105A (CD22105AE), ICM7224 (IJL, IPL+, IQH+D obsolete). Not shipped: ICM7231 and MAX7232 (no part in stock), MAX7233/7234 (character-ROM decode not transcribed: the glyph scans are not readable to cell level), ICM7225 (no part in stock), PCF1179C (the engine has no bus-less device, no `clock` + `display` combination and no BCD decode in the `lcd` driver: exact needs in `planning/plan-56-data/specs/pcf1179-sn74143.md`), SN74143 (the counter `seg7` model lacks `segmentsActive`, open-drain outputs, a second enable, MAX COUNT, latched outputs, BI, RBI/RBO and DP: same spec; footprint PDIP-24-600 drawn there). The PDFs `pcf1179c.pdf`, `counters/sn74143.pdf` and `tlc6983.pdf` stay in the tree for the work that remains.
- Remaining work list (a follow-on plan): MAX6955/MAX6957 28-port footprints (Maxim 21-0040/90-0098, 21-0141/90-0055), PCF1179C and SN74143 engine work, TLC6983's CCSI bus, the Code B selection by register/pin, `lcd`-driver extra output pins (ICM7224 CARRY, LZB OUT).
- SN74143 done (phase 2, second item). New optional keys of the counter `seg7` model (`counter_div.c` `SEG7_KEYS`, `counter.c` `DEVICE_KEYS`, `docs/parts-format.md` Model `seg7`, `CounterDraft.swift` regenerated with `scripts/part-editor-schema.py`, no `EXTRA` line needed because the part uses every key): `segmentsActive`, `segmentsOutput` (open-drain needs `segmentsActive` low; `CounterModel` got an optional `open_drain` hook, so `counter_pin_open_drain` is now per model: segments, dp and `BI/RBO` report open drain), `toggleEnable` as an object or an array of 1-4 controls, `maxCount {pin, enable}`, `latchOutputs` (4 pins), `blank`, `rbi` + `rbo` (rbo role inout, open drain, also read as a blanking input), `decimalPoint {in, out}`, and the timing extras `maxRise` / `maxFall`. All absent on the existing parts: every existing golden line is byte-identical. Added the footprint `PDIP-24-600` (dimensions re-checked against sn74143.pdf p.9) and the part `SN74143N` (flag `obsolete`, no `datasheet` key as the spec says). SN74143N is TI-obsolete and AGENTS.md says a PDF whose parts are all obsolete is skipped; it is added anyway because this plan names it explicitly and DigiKey shows it in stock (1,397, 2026-11-24).
- Spec/plan divergences resolved: (1) with several `toggleEnable` controls an unknown one with another deasserted holds the count (three-valued AND), where the spec said any X makes the count unknown; the physical answer is a known inhibit, the unknown-with-all-others-asserted case still goes unknown (both tested). (2) The first draft let lamp test win over BI; the spec's priority (and the decoder `seg7`) is BI / external low on BI/RBO first, so `blank` and the `rbo` pin low beat lamp test (no SN74143 pin uses lamp test; documented). (3) The released `BI/RBO` output is Z, as in the decoder 7447A model, so the cascade of two digits needs a pull-up on the BI/RBO to RBI net (the real chip has an internal one); the bench uses a weak 1. (4) `PartKindRoundTripTests.expected` had a stale `connector` count (952, plan-45 shipped 1541); fixed with the counter count (732 -> 733). `KindGuideTests` still fails for the crystal and powerswitch blurbs (pre-existing).
