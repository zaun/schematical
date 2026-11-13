# plan-36-I: Microcontrollers — landing tier

Last updated: 2026-11-12
Effort: small (< 1h, plus the 15-minute Swift suite)
Depends on: plan-36-H

I runs the landing tier once, on the tree with A–H landed. It ticks the
to-do item and archives plan-36.

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-A … H complete | `ls planning/completed/plan-36-[A-H]-*.md \| wc -l` → 8 | MET 2026-11-13 (8; H archived in 2d4cf1d) |

## 1. Goal

Every gate below passes on one commit, and plan-36 is in
`planning/completed/`.

### Non-goals

No new work. A failure here is fixed in the letter it belongs to: the
fix is recorded in that letter's § Corrections and the gate is re-run.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.

### Phase 1 — Landing tier

- [x] Backend: `cmake -S backend -B /tmp/logic-p36 && cmake --build
      /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36` → `100% tests
      passed`, T + 1 programs (F § Goal) (est. 3 min).
      → `100% tests passed, 0 tests failed out of 34` (T = 33 after main's plan-37 `register_test`; see Corrections).
- [x] Goldens: `git diff <commit before plan-36-G> -- backend/tests/golden
      | grep -c '^-[^-]'` → 0 (only additions over plan-36) (est. 1 min).
      → `0` against `79d11b7^` and `0` against `main` (1167c96): the goldens only add (63 / 1424 / 1424 lines).
- [x] Datasheet links: `node scripts/check-datasheets.js <the new ids>`
      (it takes ids as arguments; the new ids are the `datasheets.json`
      lines G added: `git diff <commit before plan-36-G> --
      schematical/schematical/Parts/datasheets.json | grep -o '^+.*"id": "[^"]*"'`)
      → exit 0 (est. 10 min for at most 189 URLs at 4 jobs; nothing smaller
      checks the URLs G recorded).
      → 151 ids (against `main`); `151 of 151 URLs serve a PDF; 0 failed`, exit 0.
- [x] App: `xcodebuild test -project schematical/schematical.xcodeproj
      -scheme schematical -derivedDataPath /tmp/p36-dd
      CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min).
      → `Executed 652 tests, with 0 failures`; `** TEST SUCCEEDED **`.
- [x] Real app, once: H Phase 3's check again on this commit. Also place
      an ATmega328P-AU, an SC0914(13) and an ESP32-WROOM-32E-N4 on the PCB
      page, and see each one's pads. Move the pointer to (0, 0) at the end.
      → On the worktree build (pid 55024; the owner's pid 16510 untouched): ATMEGA328P-PU dragged from the drawer → Run greyed with `U1 (ATMEGA328P-PU) is not simulated`; ⇧⌘R → still Run; Edit › Delete → Run enabled; Run → Stop shown; Stop. A file with ATMEGA328P-AU, SC0914(13), ESP32-WROOM-32E-N4 (throwaway test, deleted) on the PCB page: TQFP-32-MC-32A (8 leads a side), QFN-56-RP2040 (56 pads and the exposed pad), ESP32-WROOM-32E (edge pads, 3×3 GND grid). App quit; pointer at (0, 0).

Acceptance: every line above passes on one commit.
  Check: the five results recorded here with the commit hash (est. 35 min in total).
  Result 2026-11-13: all five pass on 2d4cf1d (its code is ae134a5's; `main` 1167c96 is merged, and main has not moved since).
Commit: —

### Phase 2 — Close

- [ ] `planning/todo.md`: tick "Add hobby microcontrollers: ATmega328P,
      ATtiny, RP2040, ESP32 modules…" (§ Parts) and add `(plan-36)`. Touch
      only that line: the owner's working copy may hold other edits to the
      file, so commit only this hunk (`git add -p`).
- [ ] Move `planning/plan-36-*.md` to `planning/completed/` (one commit).

Acceptance: the plan is archived.
  Check: `ls planning/plan-36-*.md` → no such file; `ls planning/completed/plan-36-*.md | wc -l` → 9 (est. 1 min).
Commit: —

## Validation Plan

- Final gate: Phase 1 is plan-36's landing tier. It runs once.

## Decisions

- **The real-app check runs again here** — author: H's run was on H's tree,
  and the PCB check needs C–E's footprints merged in G as well, so one
  more run on the final commit covers the whole feature.

## Corrections

- **2026-11-13: T is 33, so the backend has 34 programs.** Main's plan-37 added `register_test` while plan-36 ran; F's `mcu_test` is the +1.

## Summary

I is a gate, not work.
