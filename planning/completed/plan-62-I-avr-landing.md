# plan-62-I: AVR simulation — the landing tier

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-62-H

I runs the gates of the whole plan once, on the finished tree, records the
numbers, and archives the plan. No design and no new code beyond a fix for
a failure a gate finds (that fix belongs to the letter that owns it and is
recorded in its Corrections).

Outcome: every gate of § Phases passes on the tree after H; plan-62 A–I sit
in `planning/completed/`.

References:

- `planning/completed/plan-62-A-*.md` … `plan-62-H-*.md` (each letter's
  Acceptance and Validation Plan).
- `docs/backend.md` (tests, goldens); `AGENTS.md` (§ Git).
- `planning/completed/plan-58-H-slide-switch-landing.md` (the precedent
  landing letter and its `xcodebuild` form).

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-H is complete | `ls planning/completed/plan-62-H-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after ca385021) |
| A–G are archived | `ls planning/completed/plan-62-[A-G]-* \| wc -l` → 7 | MET (2026-11-24: 7) |

If plan-62-H is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The backend, oracle, golden, app-test, harness and doc gates below pass
  on one tree.
- The counts recorded here match the tree.

### Non-goals (explicit constraints)

- No new feature, no data change, no golden regeneration unless a gate
  found a defect and its owning letter's Corrections say so.
- No merge to `main` and no branch operation: the work stays on
  `worktree-sim` until the owner asks.

## 2. Current State

All letters A–H have landed; their `Commit:` lines hold the hashes.

### Measured populations

| What | Count | Command |
|---|---|---|
| ctest programs | 70 | `ctest --test-dir /tmp/p62-land -N \| tail -1` → `Total Tests: 70`: the 66 before plan-62 plus `avr_core_test` (A), `avr_periph_test` (B), `avr_serial_test` (F), `avr_board_test` (G) |
| Simulated mcu parts | 126 | `grep -c '|mcu|' backend/tests/golden/parts.txt` minus `grep -c '|mcu|.*|unsimulated$' backend/tests/golden/parts.txt` |
| Scenarios | 16 | `grep -c '^run ' backend/tests/avr/golden.txt` |

## 3. Design Overview

Gates only. A failing gate is root-caused in the letter that owns the code;
the gate is then re-run from the start of its phase.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — the gates

- [x] Backend, clean build: `rm -rf /tmp/p62-land && cmake -S backend -B
      /tmp/p62-land -DCMAKE_BUILD_TYPE=Release && cmake --build
      /tmp/p62-land -j8 2>&1 | grep -c 'warning:'` → `0`; `ctest
      --test-dir /tmp/p62-land -j8` → `100% tests passed, 0 tests failed
      out of 70`.
- [x] Debug build of the AVR tests (assertions on): `cmake -S backend -B
      /tmp/p62-dbg -DCMAKE_BUILD_TYPE=Debug && cmake --build /tmp/p62-dbg
      -j8 --target avr_core_test avr_periph_test avr_serial_test
      avr_board_test mcu_test && ctest --test-dir /tmp/p62-dbg -R
      'avr_|mcu_test'` → 5 tests passed.
- [x] The oracle ran live: the outputs of `avr_core_test` and
      `avr_periph_test` hold `step: 262144 vectors match avr8js`, `layout
      atmega328 equal`, 16 `ok` scenario lines and `dasm: <N> words
      round-trip` (each line is printed only when `node` ran the oracle).
- [x] The vendored oracle is unmodified: `curl -sL
      https://registry.npmjs.org/avr8js/-/avr8js-0.21.1.tgz | tar xz -C
      /tmp/p62-land-npm && diff -r /tmp/p62-land-npm/package/dist/cjs
      backend/tools/avr8js/dist/cjs --exclude='*.d.ts'` → no output.
- [x] Goldens: `python3 planning/plan-62-data/golden_check.py
      7f51ec52` (the script's base argument: the commit before plan-62) →
      all changed lines within the 126 parts.
- [x] Sanitizers on the core: `cmake -S backend -B /tmp/p62-asan
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS='-fsanitize=address,undefined'
      && cmake --build /tmp/p62-asan -j8 --target avr_core_test
      avr_periph_test avr_serial_test avr_board_test mcu_test && ctest
      --test-dir /tmp/p62-asan -R 'avr_|mcu_test'` → 5 tests passed, no
      sanitizer report.
- [x] App tests: `xcodebuild -project schematical/schematical.xcodeproj
      -scheme schematical -derivedDataPath /tmp/p62-dd
      CODE_SIGNING_ALLOWED=NO test
      -only-testing:schematicalTests/AvrAppTests
      -only-testing:schematicalTests/McuTests
      -only-testing:schematicalTests/SimulationAvailabilityTests
      -only-testing:schematicalTests/SchematicUndoTests
      -only-testing:schematicalTests/KindInfoTests
      -only-testing:schematicalTests/PartKindKeysTests
      -only-testing:schematicalTests/PartKindRoundTripTests
      -only-testing:schematicalTests/SlideSwitchTests
      -only-testing:schematicalTests/PlaySwitchResetTests` → `** TEST
      SUCCEEDED **`.
- [x] Harness: the plan-62-H Phase 2 command → every line `ok`; view the
      three PNGs.
- [x] Docs name what exists: `grep -c 'avr_core_test\|avr_periph_test\|avr_serial_test\|avr_board_test'
      docs/backend.md` → at least 4; `grep -c '"model"' docs/parts-format.md`
      → at least 1 in § `mcu`; `grep -c 'microcontroller'
      docs/file-format.md` → at least 1.

Acceptance: every box above ticked with its observed output recorded next
to it.
  Check: the commands above (est. 45 min in all; the app test build is 15
  min of it and no smaller run exercises the Swift side; the sanitizer
  build is 5 min and is the only run that catches a read past `data`).
  Observed: 2026-11-24: all nine gates pass; the outputs are in Corrections.
Commit: f4f6a5d1

### Phase 2 — close-out

- [x] Move `planning/plan-62-I-avr-landing.md` to `planning/completed/`
      (A–H were archived as they completed); keep `planning/plan-62-data/`.
- [x] Record the Phase 1 commit hash here. Phase 1: f4f6a5d1.

Acceptance: `ls planning/plan-62-*.md | wc -l` → 0; `ls
planning/completed/plan-62-* | wc -l` → 9.
  Check: the two commands (est. 1 min).
  Observed: 2026-11-24: `ls planning/plan-62-*.md | wc -l` → 0; `ls planning/completed/plan-62-* | wc -l` → 9.
Commit: PHASE2

## Validation Plan

- This letter is the plan's validation: Phase 1 is the final gate, run
  once.

## Decisions

plan-62-A § Decisions. None added.

## Corrections

- Phase 1, observed 2026-11-24 on the tree at b586d138 (gate by gate):
  1. Clean Release build: 0 warnings; `100% tests passed, 0 tests failed
     out of 70` (20.6 s).
  2. Debug build of the five AVR tests: 0 warnings, 5 of 5 passed.
  3. The oracle ran live: `step: 262144 vectors match avr8js`, `step: 99 of
     99 decode branches ran`, `fixes: 7 ok`, `layout atmega328 equal`, 16
     `ok <scenario>` lines, `ok watchdog_full_reset`, `dasm: 63902 words
     round-trip`, `ok firmware (16 files equal their sources)`.
  4. The vendored oracle: `diff -r` of a fresh tarball's `dist/cjs` against
     `backend/tools/avr8js/dist/cjs` (without `.d.ts`): no output, exit 0.
  5. Goldens against 7f51ec52: `126 parts.txt lines, 126 traces.txt lines,
     all within the 126 parts`.
  6. Sanitizers (address, undefined): 5 of 5 passed. `ctest` hides a
     passing test's output and UBSan only prints, so each binary was also
     run directly: exit 0 and 0 lines matching `runtime error`,
     `AddressSanitizer` or `LeakSanitizer` for all five.
  7. App tests, the nine classes: `Executed 146 tests, with 0 failures`,
     `** TEST SUCCEEDED **`.
  8. Harness: run on the same sources one commit earlier (ca385021; the
     commits since changed only planning files): 9 `ok` lines, exit 0, the
     three PNGs viewed and kept in `planning/plan-62-data/h-avr-*.png`.
  9. Docs: `docs/backend.md` names the four tests 9 times;
     `docs/parts-format.md` has `"model"` 30 times; `docs/file-format.md`
     has `microcontroller` 3 times.
  Counts: `Total Tests: 70`; 16 `run` lines in `backend/tests/avr/golden.txt`.
- § Non-goals: the work stays on `worktree-sim`, the branch the owner named
  for it. The follow-plan skill's own ending (a `worktree-P-62` branch,
  merged into `main` and removed) was not used: the owner had the branch
  already and `AGENTS.md` forbids creating one unasked. Merging is the
  owner's call.
- No gate found a defect, so no letter was reopened.

## Summary

Nine gates on one tree; nothing lands here but their results.
