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
| plan-12-E landed | `/tmp/logic-p12/serial_test \| grep -c '^ok E'` → 28 | NOT MET (2026-11-01; E runs in parallel, see Corrections). F's devices are all on D's model; F is first in the golden order (plan-12-A Corrections). |
| No `serial.json` yet | `ls schematical/schematical/Parts/serial.json 2>/dev/null` → no output | MET (2026-11-01, before this phase) |

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

- [x] Stock re-check (§3); drops recorded in Corrections. Observed
  (`/tmp/p12/stock.sh`, `node scripts/digikey.js --in-stock --list`):
  TL16C45 4, TL16C55 46, TL16C75 16, TL16C2 11, NS16C2 3, PC16550 4, 82C50
  12, COM92C451 1, plus TL16CP754 2 (the TL16C754C parts' names). All 81 F
  part names are in these lists: no drops.
- [x] `footprints.json` + the 7 objects; `serial.json` with the 22 devices
  and their parts. Built by `python3 -I backend/tests/fixtures/serial/merge.py
  --ship F` from `keys/` + `devices/`: `serial.json: 22 devices, 81 parts`.
- [x] `serial_test` case `shipped`. Observed: `ok   shipped: the fixture
  devices are the shipped device objects` (22 compared); `serial_test` →
  `PASS`.
- [x] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`. Check:
  `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; added lines:
  `parts.txt` 81, `traces.txt` 81, `footprints.txt` 7. Observed: 0; 81, 81, 7.
- [x] Hand-check each of the 7 new `footprints.txt` lines against its
  `footprints.md` section (pad count, pitch, span, body, centre pad). Checked:
  TQFP-48-7X7 48 pads, 1.35×0.25 at ±4.25 (span 8.5), first −2.75 (pitch
  0.5), body 7; PLCC-68 0.65×2.39 at ±11.8 (span 23.6), pitch 1.27, body
  24.23; LQFP-64 1.5×0.3 at ±5.7 (11.4), first −3.75; LQFP-80 at ±6.7
  (13.4), first −4.75, body 12; VQFN-32 0.6×0.25 at ±2.4 (4.8), first
  −1.75, centre 3.45×3.45; PDIP-48 at ±7.62 (15.24), first −29.21 (2.54),
  pin-1 rect, drill 0.75; BGA-24 5×5 grid at 0.5 (A1 −1,−1), 24 balls, no B2.
- [x] Spot-check listing lines: `TL16C550CPTR` (`|serial|model=ns16550|
  channels=1`, footprint `TQFP-48-7X7`, vcc 3000-5250), `TL16C554AFN`
  (`channels=4`, `PLCC-68`), `TL16C550DIZQSR` (`obsolete`,
  `BGA-24-MICROSTAR`, vcc 2250-5500). Observed: `TL16C550CPTR|serial|LQFP|…
  |model=ns16550 channels=1 grade=C|…|footprint=TQFP-48-7X7|vcc=3000-5250`;
  `TL16C554AFN|…|model=ns16550 channels=4 grade=C|…|footprint=PLCC-68`;
  `TL16C550DIZQSR|…|obsolete|model=ns16550 channels=1 grade=I|…
  |footprint=BGA-24-MICROSTAR|vcc=2250-5500`.

Acceptance: `ctest` 100%, T0 + 1 programs; the golden checks above (about
2 min). Observed: `100% tests passed, 0 tests failed out of 19` (T0 = 18).
Commit: 4fd2c85

## Validation Plan

The golden programs and `serial_test` (`shipped` plus every D case on the
same device objects) are the gate. The landing tier is plan-12-K's.

## Decisions

- Each footprint lands with its first user (`footprint_check` rejects unused
  footprints), so `TQFP-48-7X7`, `PLCC-68` and `LQFP-64-10X10`, shared with G,
  land here.

## Corrections

- **Run before E landed.** F ships only devices on D's model, so it does not
  depend on E's code. Its golden lines are the first of the data sub-plans
  (plan-12-A Corrections, "Execution order").
- **Device objects come from two files** (plan-12-D Corrections, "Fixture
  format"). `serial.json` is generated by `merge.py --ship F` from
  `tests/fixtures/serial/keys/<ID>.json` (the model keys, D) and
  `devices/<ID>.json` (pinouts, grades, parts: the data delegate, which
  checked 150 parts against `parts.tsv` with 0 errors). The fixtures carry
  ids `SFX-<ID>` so they load beside the shipped devices, and `shipped`
  compares them with the prefix removed.
- **§2 said the device objects were C strings in `serial_fixture.h`**; they
  are the JSON pair above.
- **Listing format.** §Spot-check wrote `|serial|model=ns16550|channels=1`;
  the facts are one field, `model=ns16550 channels=1 grade=C` (plan-12-C
  Corrections).
- **Footprint text.** The 7 objects are the `footprints.md` JSON, copied by
  the data delegate into `tests/fixtures/serial/footprints.json` and from
  there into `footprints.json` (same keys and values, one object per line).
