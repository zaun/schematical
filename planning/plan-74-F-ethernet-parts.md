# plan-74-F: Ethernet IC parts: gen-ethernet.py, Parts/ethernet.json, datasheets, footprints, goldens

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-74-E

F turns `specs-ic.tsv` and its pinout files into `schematical/schematical/Parts/ethernet.json`. It
merges D's footprints, adds the datasheet entries, and regenerates the goldens. After F the library
lists every IC `add` row of `candidates.tsv`.

References:

- plan-74-B § 3 (roles, facts), plan-74-E § 3.1 (keys), plan-74-A § 3.4 (PDF names).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets; `docs/backend.md` § goldens.
- `planning/completed/plan-48-G-timer-opamp-parts.md` (the generator precedent: one device per
  (datasheet, package, pinout), idempotent, `--check`).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-E complete | `ls planning/completed/plan-74-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25: E not started) |
| The part file is absent | `ls schematical/schematical/Parts/ethernet.json` → no such file | MET (2026-11-25) |
| Goldens at F's start | `wc -l < backend/tests/golden/parts.txt` / `footprints.txt` / `erc_pins.txt` | F records P1, F1, E1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/ethernet.json` holds one part per IC `add` row of `candidates.tsv`. Each part names its
  datasheet and its footprint. `"flags": ["obsolete"]` is set where the datasheet's ordering table
  marks a listed part obsolete (AGENTS.md); `"automotive"` is set for AEC-Q100 parts.
- `footprints.json` gains `footprints-ic.json`'s ids (`fp-merge.py`; ids another plan added are
  skipped).
- `Parts/datasheets.json` gains one entry per distinct PDF, with the maker URL of
  `datasheets/ethernet/<maker>/datasheets.tsv`.
- `parts.txt`, `traces.txt`, `footprints.txt` and `erc_pins.txt` grow by exactly the new parts and
  footprints. No existing line changes.

### Non-goals (explicit constraints)

- No generic Ethernet part (`GENERIC-…`): an Ethernet IC is never generic, its pins are its
  datasheet's.
- No Markdown datasheet (`"md"`).
- No app change (J).

## 2. Current State

E's loader accepts `ethernet.json`. `specs-ic.tsv` (B, C) gives each (maker, datasheet, package):
pinout file, facts and footprint. `footprints-ic.json` (D) loads through `fp-load.sh`.

## 3. Design

### 3.1 Devices (`census/gen-ethernet.py`)

One device per (datasheet, pinout file, type). The device's fields:

- `id`: `<datasheet id>-<package>` in lower case.
- `function`: `10/100 PHY`, `10/100/1000 PHY`, `10BASE-T controller`, `10/100 controller` or
  `10/100 controller, TCP/IP`.
- `description`: from the datasheet title.
- `drawer`: `["Interface", "Ethernet", "PHY"]` or `["Interface", "Ethernet", "Controller"]`.
- `note`: PDF, pages of the pin table, ordering table and package drawing.
- Facts: `type`, `speed`, `host`, `tcpip`, `vcc`; `"simulated": false`.
- `pinouts`: `{"<package>": <pinout file>}`.

Each part has: `name` (DigiKey's part number, verbatim), `manufacturer` (`Microchip Technology`,
`Texas Instruments`, `Analog Devices`, `WIZnet`, `Renesas Electronics`), `package` (`QFN`, `TQFP`,
`SSOP`, …), `pinout`, `footprint`, `datasheet`, and `flags` where they apply. The generator writes
the whole file sorted by device id, parts by name. `--check` fails when the file on disk differs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Parts, footprints, datasheets, goldens

- [ ] `planning/plan-74-data/census/gen-ethernet.py` per § 3.1; write `Parts/ethernet.json`.
- [ ] `python3 -I planning/plan-74-data/census/fp-merge.py schematical/schematical/Parts/footprints.json planning/plan-74-data/footprints-ic.json`.
- [ ] `Parts/datasheets.json` entries.
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`, `erc_pins.txt` (`docs/backend.md`
      § goldens) and review the diffs: only added lines.

Acceptance: the library holds exactly the IC `add` rows, and every check passes.
  Check: `cmake --build /tmp/logic-p74 -j8 && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed`; `grep -c '|ethernet|' backend/tests/golden/parts.txt` equals `awk -F'\t' 'NR>1 && $18=="add" && ($1=="phy" || $1=="controller")' planning/plan-74-data/candidates.tsv | wc -l`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `python3 -I planning/plan-74-data/census/gen-ethernet.py --check` → exit 0 (est. 6 min: an incremental build and the suite).
  Result:
Commit:

## Validation Plan

- The phase check. Final gate: plan-74-K.

## Decisions

- Author: **no generic Ethernet part.** The library's generics stand in for interchangeable parts
  (a 555, an op-amp, a resistor). No two Ethernet ICs share a pinout across makers.

## Corrections

## Summary

One generator run puts every in-scope Ethernet IC, its footprints and its datasheets in the library.
