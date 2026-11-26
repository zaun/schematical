# plan-77-F: STM32 parts: `Parts/mcu.json`, `datasheets.json`, goldens

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-77-E

F merges B's `devices.json` into `Parts/mcu.json` with each part's footprint from C and D, adds
the datasheets to `Parts/datasheets.json`, and regenerates the goldens.

References:

- `planning/plan-77-B-stm32-pinouts.md` (devices, pinouts, facts), `planning/plan-77-data/
  footprints-needed.txt` (package → footprint id), `planning/plan-77-data/datasheets.tsv`.
- `docs/parts-format.md` § Parts (line 238), § Datasheets (line 483), § `mcu`; `docs/backend.md`
  § goldens. AGENTS.md § Datasheets (every new part names a footprint).
- Precedent: `planning/completed/plan-36-G-mcu-parts.md`.

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-77-E complete | `ls planning/completed/plan-77-E-*.md` → exists | NOT MET (2026-11-25) |
| Every footprint of `footprints-needed.txt` exists | `python3 -I planning/plan-77-data/gen-stm32.py --footprints --exist` → `missing 0` | NOT MET (C, D not started) |
| Goldens at F's start | `wc -l < backend/tests/golden/parts.txt`; `grep -c '\|mcu\|' backend/tests/golden/parts.txt` | P2 and M2 recorded at F's start (P0 = 40832, M0 = 1424 at planning, before E) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Parts/mcu.json` has the STM32 devices after the existing ones; every one says
  `"simulated": false`, and their parts are the 300 `add` rows, each with `manufacturer`
  `"STMicroelectronics"`, `datasheet`, `package`, `pinout`, `footprint`.
- `Parts/datasheets.json` has one entry per file of `datasheets.tsv` (id = file name without
  `.pdf`, url = the URL fetched).
- `parts.txt` and `traces.txt` grow by exactly 300 lines each (P2 + 300); `grep -c '\|mcu\|'` →
  M2 + 300; `footprints.txt` does not change; `ctest` passes.

### Non-goals

- No loader or app change. No part outside the 300.

## 2. Current State

After E: `mcu.json` has 299 devices (RP2040 with `core2`), the loader accepts the Cortex-M cores.
C and D have merged the STM32 footprints; nothing uses them yet.

## 3. Design

- `gen-stm32.py --emit` writes the devices with `footprint` per part (from
  `footprints-needed.txt` by the part's package and datasheet), and `census/merge-mcu.py` appends
  them to `Parts/mcu.json`, ordered by drawer family then line then id. A device id already
  present stops the merge.
- Package strings (`"package"`) are ST's names (`LQFP48`, `UFQFPN32`, `TSSOP20`, `UFBGA100`,
  `WLCSP49`), as the census rows print them.
- Datasheet ids are the file names (`stm32f103c8`); `datasheets.json` entries appended in id
  order where the file keeps id order.
- Goldens regenerated (`docs/backend.md` § goldens); the diff is reviewed as: only added lines in
  `parts.txt` and `traces.txt`, every added line `|mcu|`, every one an `add` part.
- `docs/parts-format.md` § `mcu` and § The drawer's categories: the STM32 families present.

## 4. Phases

### Phase 1 — Emit and merge

- [ ] `gen-stm32.py --emit`, `census/merge-mcu.py`; `datasheets.json` entries.

Acceptance: `python3 -I planning/plan-77-data/census/merge-mcu.py --check` → `stm32 parts 300,
missing footprints 0, missing datasheets 0`.

Commit:

### Phase 2 — Goldens and docs

- [ ] Regenerate the goldens; docs.

Acceptance: `git diff --numstat backend/tests/golden/` → `parts.txt` 300 0, `traces.txt` 300 0,
nothing else; `ctest --test-dir /tmp/logic-p77 -j8` → 100% of T1.

Commit:

## 5. Validation Plan

F's acceptance commands: the merge check (seconds) and the backend suite (about 25 s; the goldens
move). The landing tier is H's.
