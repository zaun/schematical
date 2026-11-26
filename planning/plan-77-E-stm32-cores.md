# plan-77-E: Backend: the Cortex-M `core` values and the `core2` key

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-77-D

E lets the `mcu` loader accept the STM32 cores and a second core:

- `CORES` gains `cortex-m0`, `cortex-m3`, `cortex-m4`, `cortex-m7`, `cortex-m33`, `cortex-m55`
  (`cortex-m0plus` is already there).
- A new optional device key `"core2"`: the second core of a dual-core MCU, one of the same
  values. The STM32H745/H747/H755/H757 have an M7 and an M4; the RP2040 has two M0+ and gets
  `"core2": "cortex-m0plus"` so the app can stop hard-coding "Dual" for `cortex-m0plus` (G).

No STM32 part ships here (F). The only shipped data change is the RP2040 device's `core2`.

References:

- `backend/src/models/mcu.c` (`DEVICE_KEYS` :20, `CORES` :24-25, `mcu_load_device` :58-124),
  `backend/include/parts.h:341-351` (`PartMcu`), `backend/tests/mcu_test.c` (450 lines),
  `backend/tests/support/listing.c:175-185` (the `core=… clock=… flash=… ram=…` facts).
- `docs/parts-format.md` § `mcu` (line 2889; the `"core"` row), § The drawer's categories (line
  121; the Microcontroller row, line 139); `docs/backend.md` § goldens.
- Precedent: `planning/completed/plan-36-F-mcu-backend.md`.

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters of plans 69–75 wait for it; plans 76–82 follow the same rule) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-77-D complete | `ls planning/completed/plan-77-D-*.md` → exists | NOT MET (2026-11-25) |
| The values are absent | `grep -c 'cortex-m3\|core2' backend/src/models/mcu.c` → 0 | MET (2026-11-25: 0) |
| RP2040 golden lines | `grep -c 'core=cortex-m0plus' backend/tests/golden/parts.txt` → 2 (`SC0914(13)`, `SC0914(7)`) | MET (2026-11-25: 2) |
| T and goldens at E's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. E records its own T1 and P1 |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- An `mcu` device with `"core": "cortex-m4"` (and each other new value) loads; `"core2":
  "cortex-m4"` with `"core": "cortex-m7"` loads and sets `PartMcu.core2`; `"core2": "z80"` fails
  with `unknown mcu core "z80"`; `"core2"` on a device with a `"model"` fails with `an mcu with a
  "model" has one core: remove "core2"`.
- The listing prints ` core2=<value>` after `ram=` only when `core2` is set.
- `parts.txt` changes on exactly the two RP2040 lines (` core2=cortex-m0plus` added); `traces.txt`,
  `footprints.txt`, `erc_pins.txt` do not change.
- `ctest` passes with T1 programs (the new cases go into `mcu_test`).

### Non-goals

- No simulation of any Cortex core; `"model"` still needs core `avr`.
- No other `mcu` key changes. The source-size and coverage ratchets of plan-68 hold.

## 2. Current State

`mcu.c:24-25`: `CORES[] = {"avr", "avrrc", "avrxt", "avrxm", "cortex-m0plus", "xtensa-lx6",
"xtensa-lx7", "riscv", NULL}`; an unknown value fails `unknown mcu core "X"` (:86-88).
`PartMcu` has one `const char *core`. The app shows `cortex-m0plus` as "Dual Cortex-M0+"
(`schematical/schematical/Backend/PartFacts.swift:760`), true only of the RP2040.

## 3. Design

- `CORES` gets the six values, in the order `cortex-m0 cortex-m0plus cortex-m3 cortex-m4
  cortex-m7 cortex-m33 cortex-m55`.
- `DEVICE_KEYS` gets `"core2"`. `mcu_load_device` looks the value up in `CORES` with the same
  error; `PartMcu` gets `const char *core2` (NULL = one core). A `"model"` device with `core2` is
  refused (message in § 1). `parts.h`'s comment lists the values.
- `listing.c` prints ` core2=%s` when non-NULL, so no other golden line moves.
- `Parts/mcu.json`: the RP2040 device gets `"core2": "cortex-m0plus"`.
- Docs: `docs/parts-format.md` § `mcu`: the `"core"` row lists the new values; a `"core2"` row;
  the intro names STM32 among the facts-only families; the drawer table's Microcontroller row gets
  `STM32 › C0 / F0 / … / U5` (the Families F's parts use, filled in by F).

Tests (`mcu_test.c`, inline-JSON fixtures as the file's others): each new core loads; core2
loads and reads back; core2 unknown; core2 with a model; the listing line with and without core2.

### Rejected alternatives

- **One combined value** (`"cortex-m7+m4"`): every consumer would parse it; two keys keep each a
  plain enum.
- **A `"cores"` count**: the H7's two cores differ; a count cannot say which.

## 4. Phases

### Phase 1 — Loader, facts, listing, tests

- [ ] `mcu.c`, `parts.h`, `listing.c`, `mcu_test.c` (§ 3).

Acceptance: `cmake --build /tmp/logic-p77 -j8 && ctest --test-dir /tmp/logic-p77 -R mcu_test` →
passes, with the five new cases.

Commit:

### Phase 2 — RP2040 core2, goldens, docs

- [ ] `Parts/mcu.json` RP2040 `core2`; regenerate goldens; docs (§ 3).

Acceptance: `git diff --numstat backend/tests/golden/` → `parts.txt` 2 insertions, 2 deletions,
no other file; `ctest --test-dir /tmp/logic-p77 -j8` → 100% of T1; `node scripts/source-size.js
--check` and `node scripts/coverage.js --check --backend` → exit 0.

Commit:

## 5. Validation Plan

E's acceptance commands: `mcu_test` (seconds), the full backend suite (about 25 s, needed because
the goldens move), plan-68's two ratchets. The landing tier is H's.
