# plan-55-H: Parts — ideal diodes and load switches, datasheets, goldens, the Part Editor's kind lists

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-G

H adds the `powerswitch` parts of the families `ideal` and `load`: one part per `add` row (at most 331,
plan-55-A § 2), their devices, the footprints they name (from `footprints-F.json` and
`footprints-G.json`), the datasheet entries, the regenerated goldens, and the Part Editor's kind
lists for the new kind (plan-46's rule, `docs/backend.md` § Adding a kind step 6). There is **no
generic part**. The behavioral outcome: the library loads with P0 plus these parts, every new part
has a manufacturer, a datasheet and a footprint, `ctest` passes, the goldens only gain lines and the
app's kind-list tests agree.

References:

- `planning/completed/plan-55-A-powerswitch-kind.md` § 3.1, § 3.2 (the keys and the naming conventions);
  plan-55-C § 3 (the spec rows); `docs/parts-format.md` § Parts, § Datasheets, § Footprints,
  § `powerswitch`; `docs/backend.md` § Adding a kind, § Tests (golden regeneration).
- `planning/completed/plan-42-H-regulator-parts.md` (the precedent, Phases 1 and 2 and its
  Corrections 2: the app count assertions a new kind and new footprints move);
  `planning/plan-42-data/census/gen-parts.py`, `fp-merge.py`.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-G complete | `ls planning/completed/plan-55-G-*.md` → exists | NOT MET (2026-11-22: not started) |
| The goldens' sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` re-measured at the start of H; H uses its own | at planning (7c7b4d31): 34375 / 34375 / 1895 |
| The editor's lists do not name the kind | `grep -c '"powerswitch"' schematical/schematical/Editor/KindSchema.swift schematical/schematical/Editor/KindInfo.swift` → 0 and 0 | MET (2026-11-22 at 7c7b4d31: 0, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/powerswitch.json` holds one device per (series, package, pin assignment) of the `add` rows
  of `ideal` and `load` and one part per `add` row; `grep -o '"manufacturer"'
  schematical/schematical/Parts/powerswitch.json | wc -l` → the number of those `add` rows.
- `Parts/footprints.json` gains the footprints those parts name and no other (`footprint_check`
  passes: none unused, none missing); `Parts/datasheets.json` gains one entry per cited PDF.
- `backend/tests/golden/{parts,traces,footprints}.txt` only gain lines
  (`git diff --numstat backend/tests/golden` → removed lines 0).
- The Part Editor lists the kind: `PartKinds.all`, `KindInfo`, `BundledPartFile`, `PowerswitchDraft`,
  `testPowerswitch`; the app's kind-list tests pass.

### Non-goals (explicit constraints)

- No change to `backend/src`, `docs/` (A wrote the section; H adds only the drawer paths to it if A's
  § 5 left them out) or the app beyond the editor's lists and the counts the tests moved.
- No part from the family `limited` (I adds them); no generic part.

## 2. Current State

The kind loads (A); the verdicts and footprints are drawn (C to G). The editor has no entry for the
kind; the regulator's are the template (`Editor/KindInfo.swift:156`, `KindSchema.swift:206, 230`,
`BundledPartFile.swift`, `scripts/part-editor-schema.py:34`, `PartKindKeysTests.swift:23`,
`PartKindRoundTripTests.swift:22, 69`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows of `ideal` and `load` | N (at most 331) | `python3 -I planning/plan-55-data/census/verdicts.py --report` → the `ideal` and `load` `add` lines (recorded here when H starts) |
| Files naming the regulator kind | 10 | plan-55-A § 2 |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The generator and every `add` row (backend side)

- [ ] `census/gen-parts.py` (adapted from plan-42's): from `verdicts.tsv`, `specs.tsv` and
      `candidates.tsv` it writes the devices and parts of the given families to
      `Parts/powerswitch.json` (A § 3.2's ids, `function`, `drawer` and part fields; `datasheet` the id of
      the entry it adds; `flags` from `verdicts.tsv`) and prints `<d> devices, <p> parts`.
- [ ] `gen-parts.py --family ideal --family load`; `fp-merge.py` merges the footprints these parts name
      (`--only`); one `datasheets.json` entry per cited PDF (`id` the PDF's lower-case name, `url` the
      row's `maker_url`); record the device and part counts here.
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt` (`/tmp/logic-p55/listing_check --print >
      backend/tests/golden/parts.txt`, `trace_check --print`, `footprint_check --print`, `docs/backend.md`
      § Tests); review the diff: added lines only.

Acceptance: every `add` row is a loaded part and nothing else moved.
  Check: `cmake --build /tmp/logic-p55 -j8 && ctest --test-dir /tmp/logic-p55 -R 'listing_check|trace_check|library_check|footprint_check|powerswitch_test'` → 5 of 5; `grep -c '|powerswitch|' backend/tests/golden/parts.txt` → N; `git diff --numstat backend/tests/golden | awk '{d+=$2} END{print d}'` → 0 (est. 4 min). The app's kind-list tests are red between this phase and the next (a kind with parts and no editor entry) and are not run here.
Commit: —

### Phase 2 — The Part Editor's kind lists, the app's counts

- [ ] The editor's lists, each only if absent: `"powerswitch"` in `PartKinds.all` (`Editor/KindSchema.swift`)
      and `infoOrder` (`Editor/KindInfo.swift`); `KindInfo(kind: "powerswitch", name: "Power Switch", blurb:
      "Load switches, current-limited and USB power switches and integrated ideal diodes: an input
      passed to an output, gated by an enable pin.", examples: <the part names of the most-stocked `add`
      row of the ideal diodes and of the load switches>, group: .power)`; `PART_POWERSWITCH` →
      `"powerswitch"` in `PartKinds.name(of:)` (`Editor/BundledPartFile.swift`).
- [ ] The editor schema: `"powerswitch": ["powerswitch.c"]` in `scripts/part-editor-schema.py` `KIND_FILES`
      and in `PartKindKeysTests.files`; `python3 scripts/part-editor-schema.py . powerswitch` → `missing []`
      (a key no shipped part uses yet goes in `scripts/part-editor-schema-extra.py` `EXTRA`) →
      `Editor/PowerswitchDraft.swift`, added to `PartKinds.schemas`; `testPowerswitch` in
      `PartKindRoundTripTests` with `expected["powerswitch"]` = the `"manufacturer"` count of Goal 1.
- [ ] Update the app's count assertions the new kind and footprints move: `KindInfoTests`
      (`PartKinds.info.count`), `ParametricFootprintDraftTests` (the entry count and the parametric
      count), `WizardStepsTests` (the combinations and those with no variant), as the failing run prints
      them (plan-42-H Corrections 2).
- [ ] Spot-check five parts' listing lines against their PDFs (two ideal diodes, three load switches, from
      different companies): name, pinout, enable, `vinMax`, footprint; record in § Corrections.

Acceptance: the editor agrees with the part file and the kind-list tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p55-dd test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/WizardStepsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 20 min: a Debug build and six classes that read the part, kind and footprint counts; no smaller check covers the schema, the lists and the part file together; the full suite is not a plan step).
Commit: —

## Validation Plan

- Tests: `library_check` (the kind's rules on every real part), `listing_check` / `trace_check` (goldens),
  `footprint_check`, `powerswitch_test` (unchanged), the six app classes of Phase 2.
- Coverage check: `grep -c '|powerswitch|'` equals N, so every `add` row is in the goldens' denominator.
- Runtime proof: plan-55-J.
- Doc sync: `docs/parts-format.md` § `powerswitch` (A wrote it; H adds the library's drawer paths and
  the per-maker counts).
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): no generic part.
- Author: **the Part Editor's kind lists land with the first parts** (as plan-42-H did): the
  kind-list tests fail as soon as the kind has parts and no editor entry.
- Author: **the editor's example parts are real `add` rows**, since there is no generic.

## Corrections

(none yet)

## Summary

The risk is a transcription error between a datasheet and a part line, which the loader's role
rules, the spot checks and the goldens' review (added lines only) contain. The kind-list tests are
the other cost of a new kind; plan-42-H's Corrections 2 lists the counts they move.
