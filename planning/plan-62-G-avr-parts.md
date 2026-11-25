# plan-62-G: AVR simulation — the 18 devices become simulated

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-62-F

G flips the ATmega48/88/168/328 family in `mcu.json` from facts-only to
simulated: each of the 18 devices names its `"model"` and drops
`"simulated": false`. The goldens move for exactly those 126 parts, a board
test runs real part numbers, and the app tests that used an ATmega328P as
their example of an unsimulated part take another part.

Outcome: `ctest` is 100% with `avr_board_test` added; `git diff --stat
backend/tests/golden` shows `parts.txt` and `traces.txt` with 126 changed
lines each at most, every one naming one of the 126 parts.

References:

- plan-62-E § 3.1 (the `"model"` key), plan-62-D § 2 (which device has
  which layout).
- `schematical/schematical/Parts/mcu.json`; `docs/parts-format.md` § `mcu`;
  `docs/backend.md` § Tests (regenerating goldens).
- `AGENTS.md` § Datasheets: no part is added here (a part already in the
  catalog stays, in stock or not), so no stock lookup.
- App tests named below: `schematical/schematicalTests/`.

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-F is complete | `ls planning/completed/plan-62-F-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after 0d7abab4) |
| The app builds for testing | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p62-dd CODE_SIGNING_ALLOWED=NO build-for-testing 2>&1 \| tail -1` → `** TEST BUILD SUCCEEDED **` | not run (2026-11-24; first needed here) |

If plan-62-F is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The 18 devices of § 2 carry `"model"` and no `"simulated"` key; every
  other `mcu` device is unchanged.
- `circuit_add_part` accepts all 126 parts, and each starts and runs 2 ms
  with firmware loaded.
- Goldens regenerated; only the 126 parts' lines differ.
- The app's unit tests that named an ATmega328P as unsimulated pass with
  another part.

### Non-goals (explicit constraints)

- No device outside the 18 changes; no part is added, removed or renamed;
  no pinout, footprint or fact changes.
- No app feature (H): after G the app lets a schematic with an ATmega run,
  with erased flash, and a double-click on the chip does nothing yet.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Devices to flip | 18 | python over `mcu.json`: ids matching `atmega(48\|88\|168\|328)(a\|p\|pa\|pb)?$` with core `avr` → 18 |
| Their parts | 126 | same census: `sum(len(parts))` → 126 |
| Model per device | `atmega328`: 14 (48, 48a, 48p, 48pa, 88, 88a, 88p, 88pa, 168, 168a, 168p, 168pa, 328, 328p); `atmega168pb`: 3 (48pb, 88pb, 168pb); `atmega328pb`: 1 | plan-62-D § 2 |
| Pinouts used | `p28` on 10 devices, `quad32` on 18, `quad28` on 8 | same census |
| `mcu` parts the Part Editor round trip counts | 1424 | `PartKindRoundTripTests.swift:19` (`"mcu": 1424`); unchanged by G |

### Verified properties

- **Every pin name in the 18 devices' pinouts is one the loader binds**
  (plan-62-E § 2: `PBn`, `PCn`, `PDn`, `PEn`, `ADC6`, `ADC7`, `AREF`,
  `AVCC`, `VCC`, `GND`); `PEn` appears only on the four PB devices (16
  pins = 4 devices × 4), whose layouts have port E.
- **App tests that assert an ATmega is unsimulated**
  (read 2026-11-24): `SimulationAvailabilityTests` uses `ATMEGA328P-PU` as
  the blocking part in `testOneMicrocontrollerBlocksRun`,
  `testThreeMicrocontrollersNameTheFirst`, `testReasonFollowsReferenceOrder`,
  `testToggleDoesNotStart` and `testPcbPageIsNotAvailable`;
  `McuTests.testAtmega328pFacts` asserts `!part.isSimulated` and the
  `"Not simulated"` facts row. `ATTINY85-20PU` (device `attiny85`, not in
  the 18) stays unsimulated.

## 3. Design Overview

A data edit by script, a golden regeneration with a line-level check, one
new backend test, and the test edits the data change forces.

**Byte-identity is the gate for everything outside the 126 parts**: every
other golden line must be identical. The 126 `parts.txt` lines are expected
to change (they lose `|unsimulated` and gain ` model=…`), and the 126
`traces.txt` lines may change (the model now runs). A changed line that
names any other part is a defect in E's loader or model, root-caused on
that line.

`planning/plan-62-data/set_models.py` rewrites `mcu.json` in place. The
file has one key per line (`      "simulated": false,` sits between a
device's `"note"` and `"core"` lines, read at line 5165 for `atmega328p`):
for each of the 18 ids the script replaces that line of that device with
`      "model": "<layout>",` and touches nothing else (a line edit, not a
JSON re-dump).

`avr_board_test` (`backend/tests/avr_board_test.c`), on real part numbers:

- `ATMEGA328P-PU` + `GENERIC-CRYSTAL-16M` on PB6/PB7 + `GENERIC-LED` on
  PB5 + `5V`/`GND`, firmware `blink.hex`: the LED's on-time grows by half
  of the elapsed time over 100 ms (`circuit_part_light_time`).
- `ATMEGA48PA-PU`, no crystal, the one-word-vector `blink`: PB5 toggles
  every 1 ms.
- `ATMEGA328PB-AU`: a firmware that toggles PE0 moves that pin's net.
- Every one of the 126 parts: added alone with `5V`/`GND`, `blink` (the
  build matching its flash size) loaded, started, advanced 2 ms: no error.
  The test counts 126.

## Compatibility / Format Impact

- `mcu.json`: 18 devices change two keys. A document that holds one of the
  126 parts opens as before; it can now run.
- Goldens: 126 lines of `parts.txt` and up to 126 of `traces.txt`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — the data, the goldens, the board test, the forced test edits

One phase: the data change, the goldens and the tests that depend on it
cannot land apart.

- [ ] `planning/plan-62-data/set_models.py` (as § 3); run it;
      `git diff --stat schematical/schematical/Parts/mcu.json` → 18
      insertions, 18 deletions.
- [ ] Regenerate `backend/tests/golden/parts.txt` and `traces.txt`
      (`docs/backend.md` § Tests); write
      `planning/plan-62-data/golden_check.py [base]` (it compares the
      working tree's goldens with the ones at commit `base`, `HEAD` by
      default), which fails unless every
      changed line of both files starts with one of the 126 part names and
      each changed `parts.txt` line differs only by the removed
      `|unsimulated` and the added ` model=<layout>`; run it.
- [ ] `backend/tests/avr_board_test.c`, `backend/CMakeLists.txt`.
- [ ] `scripts/part-editor-schema-extra.py`: remove the `model` line E
      added (shipped devices use the key now); regenerate
      `Editor/McuDraft.swift` (`python3 scripts/part-editor-schema.py . mcu`
      → `missing []`); `simulated` becomes an optional field.
- [ ] `schematicalTests/SimulationAvailabilityTests.swift`: `ATTINY85-20PU`
      in place of `ATMEGA328P-PU` (and in the expected strings);
      `schematicalTests/McuTests.swift`: `testAtmega328pFacts` asserts
      `part.isSimulated` and no `"Simulation"` row, and a new
      `testAttiny85IsNotSimulated` keeps the unsimulated assertions.
- [ ] `docs/parts-format.md` § `mcu`: the kind is no longer facts-only for
      a device with a `"model"`; the example; which devices have one.
      `docs/backend.md`: § Adding a kind's sentence on kinds with no model
      (`mcu` now has one for some devices); § Tests gains `avr_board_test`.

Acceptance: goldens moved only for the 126 parts; all backend tests pass;
the four app test classes pass.
  Check: `python3 planning/plan-62-data/golden_check.py` → `126 parts.txt lines, N traces.txt lines, all within the 126 parts`; `ctest --test-dir /tmp/sim-rel -j8` → `100% tests passed` (est. 1 min); `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p62-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SimulationAvailabilityTests -only-testing:schematicalTests/McuTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 15 min: the app builds once; no smaller check runs the Swift tests).
Commit: —

## Validation Plan

- Tests: `avr_board_test`, `listing_check`, `trace_check`, the four app
  classes.
- Coverage check: `avr_board_test` prints `126 parts run`; `golden_check.py`
  prints the changed-line counts.
- Runtime proof: the `ATMEGA328P-PU` blink circuit.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions (the 18 devices; the plan includes the app). None added.

## Corrections

## Summary

A scripted two-key edit on 18 devices; the line-level golden check proves
nothing else moved.
