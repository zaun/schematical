# plan-39-L: Active-low pins — landing tier

Last updated: 2026-11-13
Effort: small (< 1h)
Depends on: plan-39-K

L runs plan-39's landing tier once, on the landed tree, looks at the result
in the real app, and archives the plan. Every gate's command is in the table;
the observed values are recorded under it.

References:

- plan-39-A § 1 (the key), B § 1 (the model), C § 1 (the drawing), D § 3
  (the rules) and E–K § 1 (the reviews).
- `planning/completed/plan-37-H-register-landing.md` (the precedent for a
  landing letter).
- `AGENTS.md` § Testing in the app (move the pointer to (0, 0) afterwards).

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-A … K complete | `ls planning/completed/plan-39-[A-K]-*.md \| wc -l` → 11 | NOT MET until K lands |
| The ledger has a row for every in-scope pinout | `awk 'END{print NR-1}' planning/plan-39-data/ledger.tsv` → 2129 | NOT MET until K lands |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue. If you stop, report all
> prerequisites.

## 1. Goal

- The whole tree passes the landing tier: the review tool's full check
  (R1–R8, R7 included), the backend suite, the app build and the app tests
  of every area plan-39 touched.
- A flagged part is seen with its overlines in the real app.
- The plan's files are archived.

### Non-goals (explicit constraints)

- No part-file, ledger or source change. A failure found here is fixed in
  the letter that owns it (a Correction there), not patched in L.
- The full-tier sweeps (the whole app test suite) are not a plan step; they
  run in the background per the project's policy.

## 2. Current State

Nothing new: L reads what A–K landed. Populations to record, each with the
command that measures it:

| What | Command |
|---|---|
| Flagged pins per kind | `grep -o '"activeLow"' schematical/schematical/Parts/<kind>.json \| wc -l` for each of the 19 in-scope kinds |
| Ledger rows | `awk 'END{print NR-1}' planning/plan-39-data/ledger.tsv` → 2129 |
| Golden lines with `:low` | `grep -c ':low' backend/tests/golden/parts.txt` |

## 3. Design Overview

One phase, no design. Gate class: landing tier with the targeted gates for
every touched area (backend loader and goldens; app model, editor, drawing,
agent control; the review tool).

## 4. Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; `- [~]` partial; moot tasks stay with a reason; fill `Commit:`
> when a phase lands; an unticked box means NOT DONE.

### Phase 1 — gates, the real app, archive

- [ ] Run the gates of the table below; record each observed value under it.
- [ ] Run the app (build, open a new schematic) and place `AS6C1008-55PCN`
      (SRAM), `TL16C550DIPT` (UART), `ADS8548SPM` (ADC) and `AT90CAN128-16AU`
      (MCU; its pinout has a pin named `RESET`). For each, run
      `node scripts/pin-polarity.js pins <device> <pinout>` (`AS6C1008 p32`,
      `TL16C550D tqfp48`, `ADS8548 p64`, `at90can128 <its pinout>`) and
      compare: the pins the tool prints as flagged are exactly the pins whose
      names are overlined in the block, with `#` removed from `CE#`, `OE#`,
      `WE#`; a multiplexed ADC name (`CS/FS`, `REFEN/WR`) is overlined only
      over the alternatives the tool lists. Export the schematic as SVG and
      open it: the overlines are in the file. Move the pointer to (0, 0).
- [ ] Move `planning/plan-39-L-*.md` and the rest of plan-39's files that are
      still in `planning/` to `planning/completed/` (`plan-39-data/` stays,
      as `plan-38-data/` does).
- [ ] Fill every `Commit:` line of A–L from `git log --oneline`.

| Gate | Command | Expected | Est. |
|---|---|---|---|
| Review tool, all rules | `node scripts/pin-polarity.js check && node --test scripts/pin-polarity.test.js` | exit 0; all tests pass | 1 min |
| Golden moved only by flags since plan-39 began | `node scripts/pin-polarity.js golden-check --since <A1>^` where `<A1>` is the commit on plan-39-A Phase 1's `Commit:` line | exit 0 | 1 min |
| Backend | `cmake -S backend -B /tmp/logic-p39-land && cmake --build /tmp/logic-p39-land -j8 && ctest --test-dir /tmp/logic-p39-land` | `100% tests passed` | 10 min |
| App build | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p39-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` | 6 min |
| App tests (touched areas) | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p39-dd test -only-testing:schematicalTests/ChipPinActiveLowTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/PinNameTextTests -only-testing:schematicalTests/PinNameOverlineTests -only-testing:schematicalTests/SVGPainterTests -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/SchematicSceneTests -only-testing:schematicalTests/PartPreviewTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` | 12 min |
| Docs name the key | `grep -c 'activeLow' docs/parts-format.md docs/backend.md docs/agent-control.md` | ≥ 4, ≥ 1, ≥ 1 | 1 min |
| Real app | Phase 1's second task | the overlined names equal `pins`' flagged names for all four parts, in the canvas and the SVG | 15 min |

Acceptance: every gate's expected value was observed and written under the
table; the files are archived.
  Check: `ls planning/completed/plan-39-*.md \| wc -l` → 12 (est. 1 min).
Commit: —

## Validation Plan

- Tests: the gates above.
- Coverage check: the app tests listed include the three new files
  (`ChipPinActiveLowTests`, `PinNameTextTests`, `PinNameOverlineTests`); each
  run reports ≥ 1 executed test.
- Runtime proof: the real-app task.
- Doc sync: the docs gate.
- Final gate: this phase.

## Decisions

- Names kept, per-alternative flags, one datasheet per pinout — owner,
  2026-11-13.
- Author's calls: L changes no source; failures go back to the owning
  letter.

## Corrections

(empty until execution)

## Summary

L is verification: the whole-library rules, the backend and app suites, and
a look at the real thing. The engineering risk was in A–K.
