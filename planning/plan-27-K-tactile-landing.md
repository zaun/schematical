# plan-27-K: Tactile switches — landing tier

Last updated: 2026-11-07
Effort: small (< 1h)
Depends on: plan-27-J

The last letter. It runs the plan's landing tier once: the full backend and app
test suites, a harness script that drives the real document, simulation and
views for a Normal Open and a Normal Close tactile switch, a real-app check of
the drawer and the press, and the doc and ledger close-out.

Outcome: every gate of the plan is green in one run; the drawer, the symbol and
the press are seen working; all eleven sub-plans are archived with their commit
hashes.

References:

- `planning/plan-27-D-tactile-kind.md` … `plan-27-J-tactile-app.md`.
- `scripts/harness/build.sh`, `scripts/harness/support/BusProof.swift:88`
  (`ledLevel`: the precedent for running the simulation and reading an LED),
  `scripts/harness/pull-resistor-ui.swift` (precedent: rendered views + image reads).
- The `run` skill; `AGENTS.md` § Testing in the app.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-J is complete | `ls planning/completed/plan-27-J-*` → one file | NOT MET until J archives (2026-11-07) |
| Letters A–I archived | `ls planning/completed/plan-27-[A-I]-* \| wc -l` → 9 | NOT MET (2026-11-07) |

If plan-27-J is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest` and `xcodebuild … test` pass in full.
- `scripts/harness/tactile-switch.swift` passes and its images show the
  drawer rows, the three symbols (NO released, NO pressed, NC released), and the
  sidebar facts.
- In the running app: place a Normal Open tactile switch, hold the mouse on its
  button, see the LED light; place a Normal Close one, see the LED lit at rest
  and dark while held.
- Docs and ledger are consistent.

### Non-goals (explicit constraints)

- No new feature code; a defect found here is fixed in the letter that owns the
  code (its Corrections section) before K continues.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Tactile parts landed (N_total) | `add` rows | `awk -F'\t' '$18=="add"' planning/plan-27-data/candidates.tsv \| wc -l` equals `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` |
| Golden growth | N_total each in `parts.txt` and `traces.txt` | `wc -l backend/tests/golden/parts.txt` = P0 + N_total (P0 = 11126 at planning time) |
| ctest programs | 25 | `ctest --test-dir /tmp/logic-p27 -N \| tail -1` → `Total Tests: 25` |
| Tactile devices | D_total | `grep -c '"id": "tact_' schematical/schematical/Parts/switch.json` |

### Verified properties

- `BusProof.ledLevel` (`support/BusProof.swift:88`) starts the real simulation on
  a document, flips a switch through `Simulation.setSwitch`, steps, and reads the
  LED's light; the new script uses the same calls with `setButton`.

## 3. Design Overview

One run of everything, once. Byte-identity: the DIP goldens are unchanged
(`git diff --stat backend/tests/golden` over the plan shows only added lines).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Harness script

- [ ] `scripts/harness/tactile-switch.swift` (kept): picks the first Normal Open
      two-terminal part, the first Normal Open four-terminal part and the first
      Normal Close part from the catalog; places each with `5V`, an LED
      (`GENERIC-LED`) and `GND` wired in series; checks (print `ok:`/`FAIL:`):
      (1) the drawer's Switch → Tactile has the leaves `Normal Open`,
      `Normal Close`, and prints the seconds `groups(matching:)` takes with the
      leaf expanded; (2) NO: LED dark at rest, lit after `setButton(pressed:
      true)` + `simulation.setSwitch`, dark after release; (3) NC: lit at rest,
      dark pressed; (4) four-terminal NO: its joined pins are one net at rest;
      (5) save and reopen: released; (6) renders `Harness.renderCanvas` of the
      three released symbols and the NO pressed, and `renderSidebar` with the NO
      selected.
- [ ] Run it and read every image.

Acceptance: all checks `ok`; images read.
  Check: `rm -rf /tmp/p27-h && scripts/harness/build.sh scripts/harness/tactile-switch.swift /tmp/p27-h && /tmp/p27-h/H.app/Contents/MacOS/harness /tmp/p27-img` → every line `ok:`, exit 0 (est. 6 min incl. building the harness).
Commit: —

### Phase 2 — Landing tier

- [ ] `cmake -S backend -B /tmp/logic-p27 && cmake --build /tmp/logic-p27 -j8 &&
      ctest --test-dir /tmp/logic-p27 -j8` → `100% tests passed, 0 tests failed out of 25`.
- [ ] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical
      test CODE_SIGNING_ALLOWED=NO` → 0 failures.
- [ ] `git diff --numstat backend/tests/golden` over the plan's commits shows no
      deletion (`git log --format=%h --grep='plan-27' ` lists them; diff the
      range's first parent against HEAD).
- [ ] Doc sync: `grep -l 'tactile\|momentary' docs/*.md` lists `parts-format.md`,
      `backend.md`, `file-format.md`.
- [ ] Real app (the `run` skill): build and launch; open a new schematic; in the
      drawer expand Switch → Tactile → Normal Open and Normal Close; place a Normal
      Open part, `5V`, `GENERIC-LED` and `GND` in series; ⌘R; hold the mouse on
      the button — the LED lights, release — dark; repeat with a Normal Close part
      (lit at rest, dark while held); stop; move the pointer to (0, 0) when done
      (`AGENTS.md`).

Acceptance: both suites are green and the real app behaves as in Goal.
  Check: the two commands above plus the runtime sequence (est. 25 min: the two suites dominate; nothing smaller covers backend, app and the real gesture together).
Commit: —

### Phase 3 — Ledger

- [ ] Every sub-plan's Phase `Commit:` lines are filled; `Corrections` sections
      reflect what happened; each lettered sub-plan is moved to
      `planning/completed/` as it completes (K last).
- [ ] § Summary: N_total, per-maker add counts, skipped-parts rows, devices,
      footprints, the drawer expansion time from Phase 1.

Acceptance: nothing of plan-27 remains in `planning/` outside `plan-27-data/`.
  Check: `ls planning/plan-27-[A-K]-* 2>/dev/null | wc -l` → 0 (est. 1 min).
Commit: —

## Validation Plan

- Tests: the full suites (Phase 2) and the harness (Phase 1).
- Coverage check: `grep -c 'tactile' schematical/schematicalTests/TactileSwitchTests.swift` ≥ 10 and `grep -c '^logic_test(tactile_test)' backend/CMakeLists.txt` → 1.
- Runtime proof: Phase 2's real-app sequence.
- Doc sync: Phase 2.
- Final gate (run ONCE): Phase 2's commands (est. 25 min).

## Decisions

- Scope, drawer, momentary hold: owner, 2026-11-07 (plan-27-A § Decisions).
- The landing tier is its own letter, run once — author.

## Corrections

<!-- filled in during execution -->

## Summary

The only plan-wide risks left at this point are visual (drawings) and gestural
(release); both are seen here. Nothing else is touched.
