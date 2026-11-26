# plan-68-AK: Landing — the ratchets deleted, both checks strict

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68-AJ

Every earlier letter emptied part of the two ratchet files. This letter
deletes them and the code that reads them, so that `SZ` means "no in-scope
file over 1000 code lines" and `COV --check` means "no uncovered line", with
no list to add a file to. Then it runs every gate once on the finished tree
and archives plan-68.

Prerequisites: plan-68-A's table, and every other plan-68 letter archived in
`planning/completed/` (`ls planning/plan-68-*.md` → only this file), with both
ratchets empty (`grep -c . scripts/source-size-ratchet.tsv
scripts/coverage-ratchet.tsv` → `0` and `0`).

References:

- `scripts/source-size.js`, `scripts/coverage.js` and their tests (plan-68-A
  §§ 4, 6); `AGENTS.md` § Source size and coverage; `docs/testing.md` (the
  ratchet section).

## 1. Goal

- `scripts/source-size-ratchet.tsv` and `scripts/coverage-ratchet.tsv` do not
  exist, and neither script reads a ratchet: `grep -c ratchet
  scripts/source-size.js scripts/coverage.js` → 0 and 0.
- `SZ` → exit 0; `COV --check` → exit 0 on all three tiers.
- `docs/testing.md` no longer mentions ratchets; `AGENTS.md`'s two rules are
  unchanged.

### Non-goals

- No product code changes. A gate that fails here means an earlier letter's
  acceptance was wrong: fix the code or the test it names, record it under
  that letter's Corrections, and rerun — never restore a ratchet.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — strict checks

- [x] `scripts/source-size.js`: `--check` fails on any file over 1000; the
  ratchet reading, the stale/grew messages and their tests removed (`check`
  takes the files only; the test now shows a left-behind ratchet file
  changes nothing).
- [x] `scripts/coverage.js`: `--check` fails on any uncovered line;
  `--write-ratchet` and the ratchet compare removed with their tests
  (`--write-ratchet` is now an unknown argument, tested).
- [x] Delete both `.tsv` files; `docs/testing.md`'s ratchet section removed
  (and its `--check` / `--only` lines say what they do now). Observed:
  `grep -c -i ratchet scripts/source-size.js scripts/coverage.js
  docs/testing.md` → 0, 0, 0; `SZ` → exit 0; `node --test
  --experimental-test-coverage scripts/source-size.test.js
  scripts/coverage.test.js` → pass 25, fail 0, `coverage.js` and
  `source-size.js` both 100.00 (lines, branches, functions). Prerequisite
  re-checked first: `ls planning/plan-68-*.md` → only this file;
  `grep -c .` on both ratchets → 0 and 0.

Acceptance: the strict checks pass, and both tools stay fully covered.
  Check: `SZ` → exit 0; `node --test --experimental-test-coverage
  scripts/source-size.test.js scripts/coverage.test.js` → both files `100.00`
  (est. 30 s).
Commit: —

### Phase 2 — the final gates

- [ ] Run once each: `CT`; `NT`; `python3 -m unittest discover -s
  scripts/tests`; the full `XB`; `COV --check`.
- [ ] Record each result here; archive plan-68-AK to `planning/completed/`.

Acceptance: every gate green on the finished tree.
  Check: `CT` → `100% tests passed`; `NT` and the Python tests → no failure;
  the full `XB` → no failure beyond the standing set in
  `planning/plan-68-data/app-coverage.txt`; `COV --check` → exit 0 (est. the
  full Debug suite twice — plain and instrumented; this is the plan's one
  landing tier).
Commit: —

## Validation Plan

- This letter is the plan's landing tier: Phase 2 runs every gate once.

## Decisions

- (author) The ratchets are deleted, not left empty, so nothing can be
  re-added to them.

## Corrections

(none yet)

## Summary

Removes the temporary machinery and proves the end state. No product code.
