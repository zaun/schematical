# plan-56-C: Parallel-port LED display drivers: ICM7218, ICM7228, ICM7211, ICM7212, ICM7244, ICM7245

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-B

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

Parts and models for the microprocessor-port LED/LCD display chips: ICM7218A/B (serial in) and C/D (parallel) 8-digit drivers (`icm7218-icm7228-maxim.pdf`, `icm7218-renesas.pdf`), ICM7228A–D (`icm7228-renesas.pdf`, `icm7228-intersil-2002.pdf`, `icm7228-intersil-2015.pdf`), ICM7211/ICM7212 4-digit drivers (`icm7211-icm7212.pdf`; 7211 LCD, 7212 LED), ICM7244 (`icm7244-renesas.pdf`) and ICM7245 (`icm7245-renesas.pdf`, `icm7245-intersil.pdf`) alphanumeric 8-character drivers. A host write through the chip's bus (parallel data + WR/CS/MODE, or the 7218's serial ID/data frame) drives the segment and digit pins of a connected `7seg` (or, for the 7211, an `lcd`).

Measured: 9 PDFs (`ls datasheets/led-drivers | grep -cE "icm72(18|28|11|44|45)"` → 9; ICM7224/25 are plan-56-F).

## 2. Current State

plan-56-B's `digits` and `static` output drivers and the parallel `bus` style (`parts-format.md` § converter bus) exist after B. Stock (looked up in Safari per A): ICM7218AIPI+, AIQI+, BIJI, BIPI+, BIQI+, CIJI, CIPI+, CIQI+, DIPI+, DIQI+, ICM7228AIBIZ, BIBIZ, BIJI, BIPI, CIBIZ, CIJI, CIPI, CMJI883B, DMJI, ICM7211AIM44, AIPL+, AIQH+D, AMIPL+, AMIQH+D, ICM7212AMIPL+, ICM7244BIM44Z were in stock on 2026-11-23 (script lookup before the Safari decision; re-run in Safari in Phase 1).

## Phases

### Phase 1 — specs and stock
- [ ] Re-check every stock row above in Safari; write `planning/plan-56-data/specs/parallel-led.md`: per chip pins, the write protocol (ICM7218 serial frame / parallel mode+address, ICM7228 mode bits and address, ICM7211 digit-select, ICM7244/45 ASCII memory), decode tables, scan period, supply, package, one citation per number. Datasheets whose parts are all obsolete or out of stock go to `datasheets/obsolete-not-included.md`.
Acceptance: `grep -c "^## " planning/plan-56-data/specs/parallel-led.md` → one section per device (6 families). Commit: —

### Phase 2 — devices and parts
- [ ] `leddriver.json` devices and parts for each family from the spec, datasheet ids/URLs in `datasheets.json`, footprints (PLCC-28/44, DIP-28/40, QFP-44, SOIC-28 exist? missing ones added from the drawings per `parts-format.md` § Footprints), goldens.
- [ ] Tests: a bench per family writing a digit and reading the scanned pins.
Acceptance: `ctest --test-dir /tmp/logic-build` 100%; `git diff --stat backend/tests/golden` additions only. Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

(none yet)
