# plan-72-H: Parts — I2C buffers, repeaters, extenders, muxes and switches (`i2cbus`)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-G

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
H runs plan-72-F's `gen-parts.py --family i2cbus` for every `i2cbus` `add`
row: buffers of ADI, NXP and TI; muxes and switches of TI (TCA954x), Diodes
(PI4MSD5V954x) and NXP (PCA984x).

References: plan-72-F (generator, goldens), plan-72-B § 3 (`i2cbus`
templates), plan-72-E § 4 (`links`, `splits`, `type`); the PDFs in
`datasheets/i2cbus/{adi,nxp,ti,diodes}/`.

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-G complete | `grep -c '^- \[ \]' planning/plan-72-G-ioexp-parts-adi-microchip.md` → 0 | NOT MET (2026-11-25) |
| Every `i2cbus` row complete in `specs.tsv` | `template-check.py --family i2cbus` → `0 errors` | NOT MET (B) |
| Golden sizes at H's start | `wc -l backend/tests/golden/{parts,traces,footprints,erc_pins}.txt` | recorded here as P4, R4, F4, E4 |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Parts/i2cbus.json` holds one part per `i2cbus` `add` row (planning upper bound 153: buffers ADI 92, NXP 23, TI 13; muxes TI 11, Diodes 10, NXP 4 — plan-72-A § 2).
- Every device's `type` matches its template (plan-72-E § 4.3); `facts` devices carry `"simulated": false`.
- Goldens grow by exactly those parts and merged footprints; no line changes (`|unsimulated` ends the listing line of a `facts` part).
- Smoke cases on shipped parts in `i2cbus_test.c`: TCA9548A (the plan-72-E switch case), TCA9544A (mux), PCA9515B (buffer with EN), TCA9517 (two domains, 3.3 V ↔ 5 V: the voltage check accepts it), P82B96 (split), LTC4311 (passive: a run starts), LTC4316 (`circuit_add_part` refuses it).
- plan-43's five leftovers (PCF8574ADWR in F, TCA9548APWR, PCA9617ADPJ, TCA4307DRGR, LTC4317IDHC#PBF here) are in the library when their rows are `add`.

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — muxes and switches

- [ ] `gen-parts.py --family i2cbus --template switch --template mux`; `fp-merge.py`; datasheets; goldens; smoke cases.

Acceptance: `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 8 min).
Commit:

### Phase 2 — buffers, splits, extenders, passive and facts parts

- [ ] The rest of `--family i2cbus`.

Acceptance: as Phase 1; `grep -c '|i2cbus|' backend/tests/golden/parts.txt` → the `i2cbus` `add` rows (`awk -F'\t' 'NR>1 && $1=="i2cbus" && $17=="add"' planning/plan-72-data/candidates.tsv | wc -l`); `gen-parts.py --verify --family i2cbus` → `0 problems` (est. 8 min).
Commit:

## Validation Plan

- Tests: `ctest` goldens, `i2cbus_test` smoke cases, `gen-parts.py --verify`.
- Doc sync: `docs/parts-format.md` § `i2cbus` (shipped types); `planning/plan-72-data/README.md`.
- Final gate (once): `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed`; `node scripts/coverage.js --check` exit 0 (est. 8 min).

## Decisions

- As plan-72-F; facts-only parts as plan-72-E Decisions.

## Corrections

(none yet)
