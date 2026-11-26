# plan-78-I: Potentiometers: landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-78-H

I runs plan-78's landing tier once, on the finished library and app, records the results, ticks
`planning/todo.md`, and archives the letters.

References:

- plan-78-A (the outcome sentence); plan-78-E § 1, plan-78-F § 1 (the behavior gates).
- `docs/backend.md` § tests; `planning/plan-74-K-ethernet-landing.md` (the landing precedent).

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-H complete | `ls planning/completed/plan-78-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |
| Every other letter archived | `ls planning/completed/plan-78-{A,B,C,D,E,F,G,H}-*.md \| wc -l` → 8 | NOT MET (2026-11-25: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The backend suite passes, with as many programs as at E's start (E's T1): plan-78 adds none (the
  cases are in `resistor_test`).
- The goldens hold N pot parts and K pot footprints, N = the `add` rows of `verdicts.tsv`, K = the
  lines of `footprints-needed.txt` (other plans' parts may land meanwhile, so the gate counts pot
  lines, not file sizes).
- The app's tests for pots and the editor pass on the real catalog.
- `planning/todo.md:42` is ticked `[x]` with `(plan-78)`.

### Non-goals (explicit constraints)

- No new work: a failing gate goes back to the letter that owns it (§ Corrections).

## 2. Current State

After H: every pot part, footprint and datasheet is in; the letters' own checks passed.

## 3. Design

The landing tier is the targeted gates for every area plan-78 touched: the backend suite (the
resistor kind, the listing, traces, footprints, ERC), plan-68's size and coverage checks, the app's
pot, resistor and editor tests, and one on-screen look at real parts.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Gates

- [ ] Backend: `cmake -S backend -B /tmp/logic-p78 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p78 -j8 && ctest --test-dir /tmp/logic-p78 -j8`.
- [ ] Counts: `grep -c '^[^|]*|resistor|.*pot=' backend/tests/golden/parts.txt` = N; `grep -c '^POT-' backend/tests/golden/footprints.txt` = K.
- [ ] plan-68's checks: `node scripts/source-size.js --check && node scripts/coverage.js --check --backend`.
- [ ] App: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p78-dd test -only-testing:schematicalTests/PotentiometerTests -only-testing:schematicalTests/ResistorTests -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/DrawerPerformanceTests CODE_SIGNING_ALLOWED=NO`.
- [ ] On screen: place `3296W-1-103LF` from **Discrete → Potentiometers → Trimmer** and one rotary
      pot; check the trimmer mark, `RV1` / `RV2`, the facts ("10 kΩ", "Linear", "25 turns"), the
      footprint in the board view, and that Run is enabled; move the pointer to (0, 0).

Acceptance: every gate passes.
  Check: the five commands above → `100% tests passed, 0 tests failed out of <T1>`; the two counts equal; exit 0; `** TEST SUCCEEDED **`; a screenshot `planning/plan-78-data/i-onscreen.png` showing the trimmer (est. 35 min: the backend suite 1 min, coverage 6 min, an app build and tests 15 min, on screen 10 min).
  Result:
Commit:

### Phase 2 — Records and archive

- [ ] `planning/todo.md:42`: `- [x] Add top 3 Potentiometer Resitor manufactuerers and their active resistors (plan-78: top 4, 300 each)`.
- [ ] `planning/todo.md:157-158` (the audio amplifier example): append "potentiometers: plan-78"
      to its "Needs" line; the item stays open (it needs the board itself).
- [ ] Move `plan-78-I-pot-landing.md` to `planning/completed/`.

Acceptance: the todo line is ticked and every plan-78 letter is archived.
  Check: `grep -c '\[x\] Add top 3 Potentiometer' planning/todo.md` → 1; `ls planning/completed/plan-78-*.md | wc -l` → 9 (est. 1 min).
  Result:
Commit:

## Validation Plan

- This letter is the validation plan of plan-78: Phase 1 is the landing tier, run once.
- The full tier (whole app suite, the 27-minute Debug run) runs in the background per the
  project's policy; it is not a step here.

## Decisions

- Author: **no new backend test program**: the pot cases live in `resistor_test`, so the program
  count stays E's T1.

## Corrections

## Summary

plan-78 lands: the suite, goldens, coverage and app tests pass on the real pots, the todo item is
ticked, and the letters are archived.
