# plan-72-G: Parts — ADI and Microchip I/O expanders

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-F

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
G runs plan-72-F's `gen-parts.py` for the ADI (Maxim MAX73xx, ADI ADP558x)
and Microchip (MCP230xx / MCP23Sxx) `ioexpander` rows. These two makers carry
the templates with the most engine surface: `mcp23x08`, `mcp23x17` (SPI
opcode with HAEN, SEQOP, DEFVAL/INTCON, MIRROR), `max7300` (`alias`),
`maxsimple`, `keypad-gpio`.

References: plan-72-F (the generator, the golden procedure), plan-72-B § 3
(templates), plan-72-D § 4 (keys); the PDFs in
`datasheets/ioexpander/{adi,microchip}/`.

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-F complete | `grep -c '^- \[ \]' planning/plan-72-F-ioexp-parts-ti-nxp.md` → 0 | NOT MET (2026-11-25) |
| ADI/Microchip rows complete in `specs.tsv` | `template-check.py --family ioexpander --maker adi --maker microchip` → `0 errors` | NOT MET (B) |
| Golden sizes at G's start | `wc -l backend/tests/golden/{parts,traces,footprints,erc_pins}.txt` | recorded here as P3, R3, F3, E3 |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Parts/ioexpander.json` gains one part per ADI and Microchip `add` row (planning counts: ADI 78, Microchip 62, plan-72-A § 2).
- Goldens grow by exactly those parts and their merged footprints; no line changes.
- Smoke cases on shipped parts: MCP23017 (I2C, the plan-72-D sequence), MCP23S17 (SPI, HAEN with A2–A0 = 101), MCP23008, MAX7300 (an 8-port alias write sets eight single-port registers), MAX7320 (outputs only), ADP5589 (GPIO direction and data only).

### Non-goals

- Key scanning, PWM, LED blink of ADI parts are stored, not simulated (plan-72-B Decisions).

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — Microchip

- [ ] `gen-parts.py --family ioexpander --maker microchip`; `fp-merge.py`; datasheets; goldens; smoke cases.

Acceptance: `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed`; `grep -c '|ioexpander|' backend/tests/golden/parts.txt` → TI + NXP + Microchip `add` rows; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 8 min).
Commit:

### Phase 2 — ADI

- [ ] The same for `--maker adi`.

Acceptance: as Phase 1 with all four makers; `gen-parts.py --verify --family ioexpander` → `0 problems` (est. 8 min).
Commit:

## Validation Plan

- Tests: `ctest` goldens and `ioexpander_test` smoke cases; `gen-parts.py --verify`.
- Doc sync: `docs/parts-format.md` § `ioexpander` template list; `planning/plan-72-data/README.md`.
- Final gate (once): `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed`; `node scripts/coverage.js --check` exit 0 (est. 8 min).

## Decisions

- As plan-72-F.

## Corrections

(none yet)
