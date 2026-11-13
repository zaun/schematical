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
| plan-36-A … H complete | `ls planning/completed/plan-36-[A-H]-*.md \| wc -l` → 8 | NOT MET (2026-11-12: plans written) |

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

- [ ] Backend: `cmake -S backend -B /tmp/logic-p36 && cmake --build
      /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36` → `100% tests
      passed`, T + 1 programs (F § Goal) (est. 3 min).
- [ ] Goldens: `git diff <commit before plan-36-G> -- backend/tests/golden
      | grep -c '^-[^-]'` → 0 (only additions over plan-36) (est. 1 min).
- [ ] Datasheet links: `node scripts/check-datasheets.js <the new ids>`
      (it takes ids as arguments; the new ids are the `datasheets.json`
      lines G added: `git diff <commit before plan-36-G> --
      schematical/schematical/Parts/datasheets.json | grep -o '^+.*"id": "[^"]*"'`)
      → exit 0 (est. 10 min for at most 189 URLs at 4 jobs; nothing smaller
      checks the URLs G recorded).
- [ ] App: `xcodebuild test -project schematical/schematical.xcodeproj
      -scheme schematical -derivedDataPath /tmp/p36-dd
      CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min).
- [ ] Real app, once: H Phase 3's check again on this commit. Also place
      an ATmega328P-AU, an SC0914(13) and an ESP32-WROOM-32E-N4 on the PCB
      page, and see each one's pads. Move the pointer to (0, 0) at the end.

Acceptance: every line above passes on one commit.
  Check: the five results recorded here with the commit hash (est. 35 min in total).
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

## Summary

I is a gate, not work.
