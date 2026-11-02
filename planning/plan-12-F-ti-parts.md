# plan-12-F: Parts — TI, National, Intersil and SMSC 16550-family devices

Last updated: 2026-11-02
Effort: medium (1h–2h)
Depends on: plan-12-E

F creates `schematical/schematical/Parts/serial.json` with the 22 devices
whose model D finished and whose parts are TI, National, Intersil and SMSC:
81 parts, and the 7 new footprints they are the first to use.

Devices (`devices.tsv`, the F rule of plan-12-A §3.4): TL16C450, TL16C451,
TL16C452, TL16C550B, TL16C550C, TL16C550D, TL16C552, TL16C552A, TL16C554,
TL16C554A, TL16C750, TL16C750E, TL16C754B, TL16C754C, TL16C752C, TL16C2550,
TL16C2552, NS16C2552, NS16C2752, PC16550D, 82C50A, COM92C451.

Outcome of F: P0 + 7 + 81 part lines in `parts.txt` and `traces.txt`
(A's 7 included), F0 + 7 lines in `footprints.txt`, only added lines; the
shipped device objects equal D's fixtures; `ctest` 100%.

References: plan-12-A §3.4 (data conventions); plan-12-D §3.3 (fixtures);
`plan-12-data/parts.tsv` (the 81 rows), `devices.tsv`, `footprints.md`
(§ TQFP-48-7X7, PLCC-68, LQFP-64-10X10, LQFP-80-12X12, VQFN-32-5X5,
PDIP-48-600, BGA-24-MICROSTAR); `planning/completed/plan-10-C-latch-parts.md`
(the data sub-plan pattern).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-E landed | `/tmp/logic-p12/serial_test \| grep -c '^ok E'` → 28 | NOT MET (2026-11-02) |
| No `serial.json` yet | `ls schematical/schematical/Parts/serial.json 2>/dev/null` → no output | MET (2026-11-02) |

## 1. Goal

- `grep -c '|serial|' backend/tests/golden/parts.txt` → 81 (minus parts the
  stock re-check drops, named in Corrections).
- `wc -l < backend/tests/golden/footprints.txt` → F0 + 7.
- `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- `serial_test` case `shipped` → `ok`: every device in `serial.json` that has
  a fixture equals it, key for key, except `parts`.

### Non-goals

- No model change. No existing part, footprint or golden line changes.

## 2. Current State

`serial.json` does not exist; the 22 device objects exist as fixtures in
`backend/tests/support/serial_fixture.h` (D). `footprints.json` has F0
footprints, none of the 7 below.

### Measured populations

| What | Count | Command |
|---|---|---|
| F parts | 81 | plan-12-A §3.4 grouping over `parts.tsv` (`add` rows) |
| … obsolete | 14 | same rows with `obsolete` in `flags` |
| New footprints first used here | 7 | §1 list (`TQFP-48-7X7` 53 parts in all of plan-12, `PLCC-68` 18, `LQFP-64-10X10` 16, `LQFP-80-12X12` 7, `VQFN-32-5X5` 9, `PDIP-48-600` 2, `BGA-24-MICROSTAR` 2) |

## 3. Design

- **Stock re-check first**: `node scripts/digikey.js --in-stock --list
  "<family>"` for `TL16C45`, `TL16C55`, `TL16C75`, `TL16C2`, `NS16C2`,
  `PC16550`, `82C50`, `COM92C451` (8 calls); a `parts.tsv` row not in the
  output is dropped and named in Corrections.
- `serial.json`: `{"format": "schematical-parts", "version": 1, "kind":
  "serial", "devices": [...]}`; each device = its D fixture object plus
  `"parts"` generated from `parts.tsv` (plan-12-A §3.4). Devices in
  `devices.tsv` order.
- Footprints: the 7 JSON objects verbatim from `footprints.md`, appended to
  `footprints.json`.
- `serial_test` case `shipped` (new): loads `Parts/serial.json`, and for each
  device whose id has a fixture, compares the fixture's device object with
  the file's (cJSON compare, `parts` removed from both).
- Goldens regenerated (`docs/backend.md` § Tests).

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Footprints and parts

- [ ] Stock re-check (§3); drops recorded in Corrections.
- [ ] `footprints.json` + the 7 objects; `serial.json` with the 22 devices
  and their parts.
- [ ] `serial_test` case `shipped`.
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`. Check:
  `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; added lines:
  `parts.txt` 81, `traces.txt` 81, `footprints.txt` 7.
- [ ] Hand-check each of the 7 new `footprints.txt` lines against its
  `footprints.md` section (pad count, pitch, span, body, centre pad).
- [ ] Spot-check listing lines: `TL16C550CPTR` (`|serial|model=ns16550|
  channels=1`, footprint `TQFP-48-7X7`, vcc 3000-5250), `TL16C554AFN`
  (`channels=4`, `PLCC-68`), `TL16C550DIZQSR` (`obsolete`,
  `BGA-24-MICROSTAR`, vcc 2250-5500).

Acceptance: `ctest` 100%, T0 + 1 programs; the golden checks above (about
2 min).
Commit: —

## Validation Plan

The golden programs and `serial_test` (`shipped` plus every D case on the
same device objects) are the gate. The landing tier is plan-12-K's.

## Decisions

- Each footprint lands with its first user (`footprint_check` rejects unused
  footprints), so `TQFP-48-7X7`, `PLCC-68` and `LQFP-64-10X10`, shared with G,
  land here.

## Corrections

(none yet)
