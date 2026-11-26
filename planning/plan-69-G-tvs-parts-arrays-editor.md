# plan-69-G: Parts — ESD steering arrays (Semtech, Littelfuse, Diodes Inc.); the Part Editor; docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-69-F

G adds the steering `add` rows to `Parts/tvs.json`, regenerates the goldens, makes
the Part Editor know the `tvs` kind (plan-46's step 6, now that both types are
shipped), and writes the kind's library paragraph in the docs.

References:

- plan-69-F § 3 (the generator, drawer, datasheets, footprints; G runs it with
  `--type steering`).
- `docs/backend.md` § Adding a kind, step 6 (the editor's lists);
  `planning/completed/plan-48-G-timer-opamp-parts.md` § 3.4 and its Corrections 4
  (the editor steps and the four test classes whose kind counts change).
- `scripts/part-editor-schema.py`, `scripts/part-editor-schema-extra.py`.

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-F complete | `ls planning/completed/plan-69-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |
| Golden sizes at G's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | recorded here at G's start as **P2**, **R2**, **F2**, **E2** |
| The editor does not know the kind | `grep -c '"tvs"' schematical/schematical/Editor/KindInfo.swift schematical/schematical/Editor/KindSchema.swift scripts/part-editor-schema.py` → 0, 0, 0 | MET (2026-11-25: 0, 0, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/tvs.json` also holds one part per steering `add` row (at most 155), in
  `["Discrete", "TVS diodes", "ESD arrays"]` with F's line-count functions.
- The goldens grow by exactly those parts and the footprints they name; no golden
  line is removed or changed.
- The Part Editor round-trips the kind: `PartKindRoundTripTests.testTvs` with the
  shipped part count, 0 mismatched, 0 with extras, 0 rejected.
- `docs/parts-format.md` § `tvs` has its library paragraph.

### Non-goals (explicit constraints)

- No change to an existing part; no hand-written part; no app view change (H).

## 2. Current State

After F, `Parts/tvs.json` has the Zener-type parts only; the editor's kind lists
(`Editor/KindInfo.swift` `PartKinds.all`, `infoOrder`; `Editor/BundledPartFile.swift`
`name(of:)`; `scripts/part-editor-schema.py` `KIND_FILES`;
`schematicalTests/PartKindKeysTests.swift` `files`; `Editor/KindSchema.swift`
`PartKinds.schemas`) do not name `tvs`. `KIND_FILES` has the `opamp` entry as its
last kind added (`scripts/part-editor-schema.py:44`: `"timer": ["timer.c"], "opamp":
["opamp.c"]`). The `type` key every `tvs` device has makes it the schema's
`variantKey` (`docs/backend.md` § Adding a kind), so `PartKinds.variantBlurbs` needs
an entry for `diode` and `steering` (`KindGuideTests` requires one per model).

## 3. Design

### 3.1 Steering parts

`tvs-gen-parts.py --type steering` (F § 3.1) with: device keys `type` `steering`,
`lines` and `rail` from `elements`; pin roles from the `pinout` (`ground` for GND /
VN); `description` `"<series> <n>-line ESD steering array"`; `manufacturer`
`Semtech`, `Littelfuse`, `Diodes Incorporated`.

### 3.2 The editor (each added only if absent; plans 70–74 add their kinds to the same lists)

- `Editor/KindInfo.swift`: `KindInfo(kind: "tvs", name: "TVS diode", blurb: "TVS
  diodes and ESD arrays: they protect lines from surges. Not simulated; the voltage
  check rates them by their standoff voltage.", examples: [<the most-stocked
  unidirectional SMB part>, <the most-stocked steering part>], group: .discretes)`;
  `"tvs"` in `PartKinds.all` and `infoOrder` after `diode`.
- `Editor/BundledPartFile.swift` `name(of:)`: `case PART_TVS: "tvs"`.
- `scripts/part-editor-schema.py` `KIND_FILES`: `"tvs": ["tvs.c"]`; the same in
  `PartKindKeysTests.files`.
- `python3 scripts/part-editor-schema.py . tvs` → `missing []` →
  `Editor/TvsDraft.swift`, added to `PartKinds.schemas`; a key no shipped part uses
  (`cj` on no steering part, say) goes in `scripts/part-editor-schema-extra.py`
  `EXTRA`.
- `Editor/Wizard/Guides/VariantBlurbs.swift` `PartKinds.variantBlurbs["tvs"]`: `diode` — "One or
  more TVS diodes, each unidirectional (anode and cathode) or bidirectional (two
  terminals)."; `steering` — "A steering array: each line is steered to a rail and
  to ground."
- `PartKindRoundTripTests`: `testTvs` with the shipped count
  (`grep -o '"manufacturer"' schematical/schematical/Parts/tvs.json | wc -l`); the
  kind and model counts in `KindGuideTests`, `PartEditorWindowTests` and
  `WizardStepsTests` re-set to the counts the tests print (plan-48-G Corrections 4).

### 3.3 Docs

`docs/parts-format.md` § `tvs`: the library paragraph — the counts per type and
maker (`grep -c '|tvs|' backend/tests/golden/parts.txt`; per maker from
`tvs-gen-parts.py --verify`), the drawer paths, the standoff bands, the line-count
functions, and that ST's USBLC6 and TI's TPD arrays are not in the library
(plan-69-A § 1 non-goals).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Steering parts

- [ ] `tvs-gen-parts.py --type steering`, `datasheets.json`, `tvs-fp-merge.py --only`;
      regenerate the four goldens.
- [ ] `node scripts/check-datasheets.js --jobs 4 <the new ids>` → no failure (the
      Semtech distribution URLs answer HTML to a script; record them as expected in
      § Corrections, as `docs/parts-format.md` does for Suntsu).

Acceptance: the steering pick loads and the goldens grow by exactly it.
  Check: `ctest --test-dir /tmp/logic-p69-f -j8` after a rebuild → `100% tests passed`; `grep -c '|tvs|' backend/tests/golden/parts.txt` → the Zener-type plus steering `add` counts; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 5 min).
Commit:

### Phase 2 — The editor and the docs

- [ ] § 3.2's steps.
- [ ] § 3.3's paragraph.

Acceptance: the editor round-trips every shipped TVS part.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **`; `grep -n '^### `tvs`' -A80 docs/parts-format.md | grep -c 'Semtech'` → 1 or more (est. 12 min: app build and six test classes; these six are the classes that hold the editor's kind lists and counts).
Commit:

## Validation Plan

- Tests: the four backend goldens, `tvs_test`; the six app test classes of
  Phase 2.
- Coverage check: `tvs-gen-parts.py --verify` → `steering: <n> add rows, <n> parts`.
- Doc sync: § 3.3.
- Final gate: plan-69-I.

## Decisions

- Author: **the editor steps come after both types ship**, so
  `part-editor-schema.py` sees `lines` and `rail` in shipped parts and needs the
  fewest `EXTRA` keys.
- Author: **the editor group is Passives and discretes**, beside `diode`.

## Corrections

## Summary

The smaller parts letter, plus the editor's lists; the risk is only the counts
the editor's tests hold, which are re-set from their own output.
