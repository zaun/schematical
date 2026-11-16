# plan-46-J: Part Wizard — step 6 (footprint): thumbnails, ranking, parametric footprints

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-46-I

Today the footprint is a dropdown of ids ("SOIC-14 — 14-pin SOIC, 3.9 mm
body…") filtered by pin count, or a raw pad table. J makes step 6 three
choices a hobbyist can follow:

1. **Use a library footprint** — a grid of thumbnails (the pads drawn to
   scale, pin 1 marked) of every bundled footprint with the pinout's pin
   count, the ones matching the package typed on step 2 first, with a
   search box.
2. **Make one from the datasheet drawing** — pick the shape (two rows /
   four sides / ball grid / pin header), and fill in four to six numbers
   beside a labeled diagram (pitch, row spacing, pad width and length,
   drill). The wizard writes it as a **parametric** inline footprint
   (`dual`, `quad`, `grid` or `header` style, `docs/parts-format.md`
   § Footprints), the same styles 1082 of the library's 1646 footprints
   use, so the pads come from the backend's own expansion.
3. **Place pads one by one** — the existing pad table (plan-32-D), for
   the odd shapes.

Each choice shows the live footprint preview, and the step's guidance says
which drawing to read: the **recommended land pattern** if the datasheet
has one, not the package outline; mid-values `(min + max) / 2`; pin 1 at
the top of the left side; top view.

Behavioral outcome: for a 14-pin part with package `SOIC`, the SOIC-14
footprints are first in the grid; "Make one…" with style two rows, 14 pins,
pitch 1.27, row spacing 5.4, pad 1.55 × 0.6 gives a footprint whose
expanded pads are identical to the bundled `SOIC-14`; a ball pinout gets a
`grid` footprint whose balls are the pinout's designators.

References:

- `docs/parts-format.md` § Footprints (:434–), § Common keys, § `dual`,
  § `quad`, § `grid`, § `header`, § Custom footprints in a part file
  (:663–675: the editor writes one `explicit` entry; "a ball pinout has no
  custom footprint"), § The "footprint" part key (mid-values, R1–R9).
- `Editor/CustomFootprintDraft.swift` (`CustomFootprintDraft`, `starter`,
  `init(footprint:)` :164 — an expanded footprint as explicit pads),
  `Editor/CustomFootprintEditor.swift`, `Editor/PartDraft.swift`
  `customFootprint` (:457–476: only `explicit` entries are read back).
- `Editor/FootprintPreviewView.swift`, `Backend/PartCatalog.swift`
  `Footprint` (:247, pads with centre, size, shape, drill).
- `PartValidator.footprints` (the bundled footprints, expanded).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-I complete | `ls planning/completed/plan-46-I-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-I-wizard-kind-steps.md`) |
| `CustomFootprintDraftTests` green (added while executing, § Corrections 1) | `xcodebuild … test -only-testing:schematicalTests/CustomFootprintDraftTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` | MET (2026-11-16: NOT MET at 2760fe46 — a stale count; fixed in this plan, § Corrections 1; then `Executed 2 tests, with 0 failures`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/Wizard/FootprintThumbnail.swift`: draws a `Footprint`'s pads
  (rect / round, drill holes) and body outline fitted to its frame, pin 1
  in the accent color.
- `FootprintRanking.rank(_ footprints:, package: String) -> [Footprint]`:
  package tokens (uppercase, split on non-alphanumerics, e.g. `SOIC`,
  `14`) scored against each footprint's id and description tokens
  (exact token match 2, prefix match 1), then by id; the step shows them in
  a `LazyVGrid` with a search field over id and description.
- `Editor/ParametricFootprintDraft.swift`: `style` (`dual`, `quad`, `grid`,
  `header`), `pins`, `body` (width, height, round), `pad` (width, height,
  shape), `drill`, `pin1Shape`, and per style: dual `pitch`, `span`,
  `perSide`; quad `sides` (left, bottom, right, top), `pitch`, `spanX`,
  `spanY`; grid `rows`, `columns`, `pitch`, `balls`; header `rows`,
  `pitch`, `rowPitch`, `firstRowY`. `encoded()` writes exactly the keys of
  § Footprints for the style (omitting defaults); `init?(json:)` reads one
  back; id `CUSTOM-<first 8 of uuid>`, as today.
- `PartDraft.inlineFootprint: InlineFootprint?` (`.explicit(CustomFootprintDraft)`
  or `.parametric(ParametricFootprintDraft)`), the one inline footprint the
  part names; `customFootprint` keeps its meaning (explicit only) for the
  All fields form and its tests.
- Step 6 page `FootprintStepPage`: the three choices as tabs; "Make one…"
  shows the style picker, a `FootprintDiagram` (a drawn sketch of the style
  with each dimension's arrow and name), the number fields, and the live
  preview; "Start from this one" on a selected library footprint copies
  its parameters into the form when it is parametric (the four styles),
  or its pads into the pad table when it is `explicit`
  (`CustomFootprintDraft(footprint:)`).
- Ball pinouts: the grid style fills `balls` from the pinout's
  designators and `rows`/`columns` from the highest row letter and column
  number; the doc's "a ball pinout has no custom footprint" becomes "a
  ball pinout's custom footprint is a `grid` (or `explicit` with `ball`
  pads)".
- Guidance (on the page, and the step's Find terms from G): the land
  pattern rule, mid-values, pin 1 position, top view, and "pins must equal
  the pinout's pin count".

### Non-goals (explicit constraints)

- **`footprints.json` is never written**; a custom footprint lives in the
  `.schpart` (§ Custom footprints in a part file).
- **The All fields footprint section is unchanged** (picker, connector
  footprint, choices, Custom Footprint…).
- **No `centre`, `slots`, corner radius or chamfer controls** in the
  parametric form; parts that need them use "Start from this one" on a
  bundled footprint that has them, or the pad table. The form keeps those
  keys when it read them from a file (they stay in the entry).

## 2. Current State

- The footprint picker lists bundled footprints whose pad count equals the
  pinout's pin count (`PartEditorWindow.swift:177–211`), by id and
  description.
- Custom Footprint… writes one `explicit` entry with one pad per pin
  (`CustomFootprintDraft.starter`); `customFootprint` reads back only
  `explicit` entries, so a parametric inline entry would be kept in
  `footprints` but have no editor.
- The loader accepts any style in a `.schpart`'s `"footprints"`
  (`footprint_read_entries`, the same reader as `footprints.json`;
  `docs/parts-format.md` § Single part files: "each entry is a
  `footprints.json` entry").

### Measured populations

| What | Count | Command |
|---|---|---|
| Bundled footprints by style | dual 763, explicit 564, header 181, quad 110, grid 28 (1646) | `python3 -I -c "import json,collections;print(collections.Counter(f['style'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` |
| Parametric (the four styles J writes) | 1082 | 763 + 181 + 110 + 28 |
| Candidates for common pin counts | 2 pins 409, 4 pins 245, 8 pins 72, 14 pins 21, 16 pins 69 | the same file, `Counter(f['pins'] …)` |

The 409 two-pad candidates are why the grid is lazy and ranked.

## 3. Test design (`ParametricFootprintDraftTests`)

For each style, take one bundled footprint of that style
(`SOIC-14` dual; the first `quad`, `grid` and `header` entries of
`footprints.json` in file order, chosen in the test by style), build a
`ParametricFootprintDraft` from its JSON keys, encode it under the
`CUSTOM-` id into a gate-or-generic draft with a matching pinout (numbered
1…N, or the grid's balls), validate, and compare the validated part's
footprint pads with the bundled footprint's expanded pads
(`PartValidator.footprints`): same designators, centres, sizes, shapes and
drills. Plus: `init?(json:)` → `encoded()` is identity for the four
entries; `FootprintRanking` puts `SOIC-14` before `PDIP-14` for package
`SOIC`, and `PDIP-14` before `SOIC-14` for `PDIP`.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Parametric drafts and ranking

- [x] `ParametricFootprintDraft.swift`, `PartDraft.inlineFootprint`,
      `FootprintRanking`. (Also `ParametricFootprintDraft.starter` per style,
      `gridSize(balls:)`, `FootprintRanking.search`, `BundledFootprints.entry`
      for Phases 2–3. Keys the form has no control for (`note`, `centre`,
      `slots`, a pad's corner keys) are kept in `extras` / `padExtras`.)
- [x] `schematicalTests/ParametricFootprintDraftTests.swift` per § 3.
      (`dual: SOIC-14, 14 pads match`; `quad: VQFN-14, 15 pads match`;
      `grid: DSBGA-4-YFP, 4 pads match`; `header: PINHDR-1X01-V, 1 pads
      match`; all 1082 bundled parametric entries read and write back the
      same; SOIC-14 from the behavioral outcome's numbers matches;
      `xcodebuild … test -only-testing:schematicalTests/ParametricFootprintDraftTests
      -only-testing:schematicalTests/CustomFootprintDraftTests …` → `Executed
      7` and `Executed 2 tests, with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: each style's draft expands to exactly the bundled footprint it was made from.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/CustomFootprintDraftTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 2 — Thumbnails and the library tab

- [ ] `FootprintThumbnail.swift`; the library tab with ranking and search.

Acceptance: the grid shows the ranked candidates.
  Check: the run skill: a 14-pin gate with package `SOIC` → step 6 → the first row of thumbnails are SOIC-14 footprints; a 2-pin generic part → scrolling the 409 candidates stays smooth (no beach ball) (est. 10 min).
Commit: —

### Phase 3 — "Make one…" and the pad table tab

- [ ] `FootprintDiagram.swift` (four sketches), the form, "Start from this
      one", the pad-table tab hosting `CustomFootprintEditor`; the ball
      pinout grid fill.
- [ ] `docs/parts-format.md` § Custom footprints in a part file: the
      editor may write a parametric entry; the ball-pinout sentence per § 1.
- [ ] Runtime check: the SOIC-14 numbers of the behavioral outcome →
      preview matches the bundled SOIC-14 thumbnail → Save Part… → File ›
      Load Part… loads it and the PCB shows the footprint. Pointer to (0, 0).

Acceptance: a footprint made from a few numbers is saved, loads and draws.
  Check: the walk above (est. 15 min).
Commit: —

## Validation Plan

- Tests: `ParametricFootprintDraftTests`, `CustomFootprintDraftTests`
  (unchanged, still green), `PartEditorFootprintTests`.
- Coverage: § 3 checks the backend's expansion of each style J writes.
- Runtime proof: Phase 3's walk.
- Doc sync: Phase 3's doc edit.
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions. Author's calls: parametric styles over explicit pads
for "Make one…" (the backend expands them; four to six numbers instead of
one row per pad); no corner/centre/slot controls in the form.

## Corrections

1. **A prerequisite no row covered: `CustomFootprintDraftTests` was not
   green at the base.** Phase 1's check names it ("unchanged, still
   green"); it failed `testEveryExplicitFootprintRoundTrips`:
   `XCTAssertEqual failed: ("564") is not equal to ("482")` while printing
   `checked 564 explicit footprints, 0 mismatched`. The test hard-codes the
   number of bundled `explicit` footprints; later plans added footprints
   without moving it (§ 2 of this plan already measures 564), and plan-46
   changes no footprint (`git diff --stat 4218bb74 --
   schematical/schematical/Parts/footprints.json` → empty). The count is
   now 564; landed as its own commit before Phase 1.

## Summary

The risk is a parametric draft that encodes keys the loader expands
differently from the bundled entry it mimics; § 3 compares pad for pad
against the backend's own expansion for every style.
