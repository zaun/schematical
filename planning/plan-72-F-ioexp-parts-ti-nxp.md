# plan-72-F: Parts — TI and NXP I/O expanders; `gen-parts.py`

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-E

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
F writes `planning/plan-72-data/gen-parts.py` (templates + `specs.tsv` +
`address.tsv` + `footprint-map.tsv` → part-file devices), generates the TI
and NXP `ioexpander` devices into `Parts/ioexpander.json`, adds their
datasheet entries and the footprints they use, and regenerates the goldens.
G (ADI, Microchip) and H (`i2cbus`) reuse the script.

References:

- plan-72-A (`candidates.tsv` `add` rows), -B (`specs.tsv`, `templates/`,
  `address.tsv`), -C (`footprints-C.json`, `footprint-map.tsv`), -D (§ 4: the
  `io` keys).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets, § `ioexpander`;
  `docs/backend.md` § Tests (goldens: `parts.txt`, `traces.txt`,
  `footprints.txt`, `erc_pins.txt`).
- `planning/plan-48-data/census/gen-parts.py`, `fp-merge.py` and
  `planning/plan-58-data/gen-parts.py` (the shape copied: region ownership,
  `--dry-run`, `--verify`, `--selftest`).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-E complete | `grep -c '^- \[ \]' planning/plan-72-E-i2cbus-kind.md` → 0 | NOT MET (2026-11-25) |
| Every TI/NXP expander row has a template, a pinout and a footprint | `python3 -I planning/plan-72-data/census/template-check.py --family ioexpander --maker ti --maker nxp` → `0 errors`; `fp-check.py all` → `0 without a footprint` | NOT MET (B, C) |
| Golden sizes at F's start | `wc -l backend/tests/golden/{parts,traces,footprints,erc_pins}.txt` | recorded here as P2, R2, F2, E2 |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Parts/ioexpander.json` holds one device per (maker, datasheet, register
  template) of the TI and NXP `add` rows and one part per row (`name` = the
  DigiKey part number, `package`, `pinout`, `footprint`, `flags` obsolete /
  automotive from the datasheet, `vcc`/`inputMax`, `i2cAddress` default).
- `datasheets.json` has an entry per PDF those parts cite; the PDFs are in
  `datasheets/ioexpander/{ti,nxp}/` (A).
- The footprints those parts use are merged from `footprints-C.json`.
- `parts.txt`, `traces.txt`, `erc_pins.txt` grow by exactly the new parts,
  `footprints.txt` by the merged footprints; no existing line changes.
- One real-part smoke case per template in `ioexpander_test.c` (TCA9555,
  PCF8574, TCA6408A, TCA6424A, PCAL6416A, PCA9555 where in the pick): the
  fixture sequence of plan-72-D Phase 3 on the shipped part.

### Non-goals

- No engine change. A part a template cannot express is not forced in: it
  goes back to B (Corrections) for its own template.

## 2. Current State

`Parts/ioexpander.json` exists with no devices (D). TI's planning count is
101, NXP's 65 (`planning/plan-72-A-ioexp-data.md` § 2); A's `add` rows are the
real population.

## 3. Design

`gen-parts.py [--family ioexpander|i2cbus] [--maker M]... [--dry-run]
[--verify] [--selftest]` owns its region of the part file: it replaces every
device whose `note` starts `plan-72:` and leaves others alone (plan-58's
region rule). Device = template with `{IO}`, `{PORTS}`, `{ADDRESS_TABLE}`
filled; the `note` cites the PDF and its pages (pin table, register map,
ordering table). `--verify` reloads the file and checks one part per `add`
row of the selection. The device `id` is the datasheet's base number
(`TCA9555`); a datasheet covering several register variants (PCA9534/PCA9554
in one PDF) gives one device per variant.

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — the generator

- [ ] `gen-parts.py` with `--selftest` (a template fill, an address table, a region replace).
- [ ] `fp-merge.py` copied from plan-48 (merges the footprint ids the selection uses).

Acceptance: `python3 -I planning/plan-72-data/gen-parts.py --selftest` → `ok` (est. 1 min).
Commit:

### Phase 2 — TI

- [ ] `gen-parts.py --family ioexpander --maker ti`; `fp-merge.py`; datasheet entries; goldens regenerated (`listing_check --print`, `trace_check --print`, `footprint_check --print`, `erc_pins_check --print`), the diff reviewed.
- [ ] Smoke cases for TI templates in `ioexpander_test.c`.

Acceptance: `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed`; `grep -c '|ioexpander|' backend/tests/golden/parts.txt` → the TI `add` rows (`awk -F'\t' 'NR>1 && $1=="ioexpander" && $2=="ti" && $17=="add"' planning/plan-72-data/candidates.tsv | wc -l`); `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 8 min).
Commit:

### Phase 3 — NXP

- [ ] The same for `--maker nxp`.

Acceptance: as Phase 2 with TI + NXP rows; `gen-parts.py --verify --family ioexpander --maker ti --maker nxp` → `0 problems` (est. 8 min).
Commit:

## Validation Plan

- Tests: `ctest` (listing, trace, footprint, erc_pins goldens; `ioexpander_test` smoke cases); `gen-parts.py --verify`.
- Doc sync: `docs/parts-format.md` § `ioexpander` lists the shipped templates; `planning/plan-72-data/README.md`.
- Final gate (once, after Phase 3): `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed` and `node scripts/coverage.js --check` exit 0 (est. 8 min).

## Decisions

- Part name = DigiKey part number; device id = datasheet base number — plan-48 / plan-58 precedent.

## Corrections

(none yet)
