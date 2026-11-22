# plan-54-F: The app — opposite-side pads and edge-mounted footprints in the board model, canvas, routing and checks

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-E

F makes the app understand the two footprint keys E added. A footprint pad with `opposite`
is copper on the other face of the board from the part's own side: the ratsnest, the router,
the pours, the design-rule check, the PCB canvas and its hit testing, and the board's layer
visibility must all treat it as a pad of that other layer, and a part flipped to the other
side swaps its pads' layers. An `edgeMount` footprint is checked against the board outline
by its pads only. The Part Editor's custom footprint editor round-trips and edits the key.
G does the same for the exports and the 3D preview. The behavioral outcome: in a test document, a part whose footprint has one top pad
and one opposite pad has its two pads on Top and Bottom, a Bottom trace reaches the
opposite pad and a Top trace does not, flipping the part swaps the layers, a 1-layer board
reports the opposite pad, and an `edgeMount` part hanging over the outline is not reported
as off the board while its pads are on it.

References:

- plan-54-E § 3 (the keys and their meaning); `docs/parts-format.md` § `explicit`.
- `schematical/schematical/Backend/PartCatalog.swift` (`Footprint`, `Footprint.Pad`, the copy
  of the backend's pads at line 893), `Canvas/BoardModel.swift` (`PlacedPad` line 109,
  `PlacedChip.boardPads` line 171, `boardBounds`), `Canvas/Ratsnest.swift`
  (`BoardConnectivity.Pad`, the pad list built at line 94), `Canvas/SchematicModel.swift`
  (`chipPads` line 1414, `padLabel` line 3296, `pads(on:)` line 3335, `setFootprintSide` line
  2781), `Canvas/PCBCanvasView.swift` (the pad drawing at lines 355 – 410), `Canvas/BoardSettings.swift`
  (`showsPad`, `showsFootprint`), `DesignRuleCheck/DesignRuleCheck.swift` (`checkPlacement` line 333, the layer check at line 398, `rectIsOnBoard` line 363), `Editor/CustomFootprintDraft.swift`,
  `Editor/CustomFootprintEditor.swift`, `Editor/FootprintPreviewView.swift`.

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-E complete | `ls planning/completed/plan-54-E-*.md` → exists | NOT MET (2026-11-22: not started) |
| `FootprintPad` has `opposite` | `grep -c 'int opposite' backend/include/parts.h` → 1 | NOT MET (2026-11-22: E adds it) |
| No `opposite` in Swift yet | `grep -rc 'opposite' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Canvas/BoardModel.swift` → `0` and `0` | MET (2026-11-22 at 7b449718: 0, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Footprint.Pad` has `opposite: Bool` and `Footprint` has `edgeMount: Bool`, copied from the
  backend's `FootprintPad.opposite` and `PartFootprint.edge_mount`; `PlacedPad` has `opposite`.
- `PlacedPad.layerSide(on side: BoardSide) -> BoardSide` returns `side` for an ordinary pad and
  the other side for an opposite pad; **every** site that decides a pad's copper layer from its
  part's side uses it (§ 2 lists them).
- A surface-mount opposite pad is on the other layer for connectivity, routing, pours,
  clearance, canvas drawing, hit testing and visibility; through-hole pads are unchanged.
- The design-rule check reports a part with an opposite pad on a board with 1 copper layer
  (`.layer`, "Has pads on Bottom, but a 1-layer board has only Top") and does not report an
  `edgeMount` part for hanging off the edge when its pads are on the board.
- The custom footprint editor keeps and edits `opposite` and `edgeMount`.
- `OppositePadTests` passes; the existing test classes the sites belong to stay green.

### Non-goals (explicit constraints)

- No change to a part's own side, rotation or flip behavior: `setFootprintSide` still mirrors the
  footprint and now swaps its opposite pads' layers through `layerSide`.
- No change to through-hole pads, non-plated holes, fingers, or the board's own pads.
- No export or 3D-preview change (G); no new part (M and N).

## 2. Current State

A pad's layer is not stored: it is the placed part's `board.side` (`BoardSide`) through
`BoardSide.copperLayer`; `drill > 0` pads are on every layer. The connectivity builder copies
the part's side into each `BoardConnectivity.Pad.side` (`Ratsnest.swift:100` and `:105`); the
consumers that read the layer of a `BoardConnectivity.Pad` therefore follow that one assignment:
`Routing/RoutingProblemBuilder.swift:63`, `:106`, `:109`, `DesignRuleCheck.swift:270`, `:444`,
`:845`, `:892`, `PourConnectivity.swift:17`, `PCBCanvasView.swift:1289`,
`SchematicModel.swift:3913`, `Canvas/Ratsnest.swift:167` (12 lines in 6 files; the Gerber, KiCad
and EasyEDA writers' reads of `pad.side` are G's). The sites that read the **part's** side
instead, and so need `layerSide`, are the ones that iterate `chipPads` or `boardPads` under
`chip.board?.side`:

| Site | What it does with the part's side |
|---|---|
| `Canvas/Ratsnest.swift:100` – `:107` | creates each `BoardConnectivity.Pad` with the part's `side` |
| `Canvas/SchematicModel.swift:3302` – `:3305` (`padLabel`) | hit-tests a surface-mount pad under the part's side and `visible.showsPad(drill:side:)` |
| `Canvas/SchematicModel.swift:3335` – `:3339` (`pads(on:)`) | a surface-mount pad is on its part's side layer |
| `Canvas/PCBCanvasView.swift:359` – `:361` | draws each side's surface-mount pads from the parts on that side |
| `Canvas/BoardSettings.swift:506` (`showsFootprint`) | a footprint shows when any pad shows on the part's side |
| `Editor/FootprintPreviewView.swift:69` | draws every pad as the part's side's copper |

`PlacedChip.boardPads` builds each `PlacedPad` from `Footprint.Pad` and mirrors its corners for
a bottom part (`BoardModel.swift:171`); `Footprint.Pad` is built from the backend pad at
`PartCatalog.swift:893`. `DesignRuleCheck.checkPlacement` flags a part whose `boardBounds` (body
and pads) is not on the board (`rectIsOnBoard`), and its layer check reports a part on Bottom of a
1-layer board (`DesignRuleCheck.swift:398`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines that read a `BoardConnectivity.Pad`'s layer in F's directories (follow the one assignment) | the 12 lines listed in § 2 (read one by one on 2026-11-22) | `grep -n 'pad\.side\|entry\.side\|\$0\.side' schematical/schematical/Canvas/Ratsnest.swift schematical/schematical/Canvas/SchematicModel.swift schematical/schematical/Canvas/PourConnectivity.swift schematical/schematical/Canvas/PCBCanvasView.swift schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift schematical/schematical/Routing/RoutingProblemBuilder.swift` → those lines plus unrelated `side` of pins, texts and a board pad's own side (`Ratsnest.swift:118`) |
| Sites that read the part's side for its pads (the table above) | 6 | `grep -rn 'boardPads\|chipPads' --include='*.swift' schematical/schematical \| grep -v 'BoardModel.swift'` (the lines listed, plus G's exporters) |
| Part Editor files that know a pad's keys | 3 (`CustomFootprintDraft.swift`, `CustomFootprintEditor.swift`, `FootprintPreviewView.swift`) | `grep -rln 'plated' schematical/schematical/Editor` |
| Existing test classes the sites belong to | the PCB canvas, ratsnest, routing and DRC test classes | `ls schematical/schematicalTests \| grep -i -E 'ratsnest\|route\|drc\|designrule\|pcb\|board'` (the harness build is the test run) |

### Verified properties

- `BoardConnectivity.Pad.side` is the only layer information the consumer lines of § 2 use; they
  do not look at the part (each reads `pad.side` or `entry.side`), so assigning the right `side`
  at creation fixes them together.
- `CopperLayer.bottom` is Bottom on every board and a 1-layer board has only Top
  (`CopperLayer.allCases(for:)`), so an opposite pad of a Top part on a 1-layer board names a
  layer the board lacks.
- `PlacedChip.boardPads` already mirrors a pad's x and corners for a bottom part; a pad's
  opposite flag does not change its geometry, only its layer.

## 3. Design

- **Model.** `Footprint.Pad.opposite`, `Footprint.edgeMount` (copied at `PartCatalog.swift:893`
  and the footprint init), `PlacedPad.opposite` (set in `boardPads`), and
  `PlacedPad.layerSide(on:)`; one definition, no other code computes a pad's layer from the part's
  side.
- **Connectivity and consumers.** `BoardConnectivity.Pad(side:)` is assigned
  `pad.layerSide(on: chip side)` at `Ratsnest.swift:105`; the consumer lines of § 2 follow. The six sites
  of § 2 use `layerSide` instead of the part's side.
- **Checks.** `DesignRuleCheck`: the 1-layer check also reports a part whose pads include one with
  `layerSide(on: .top)` = Bottom on a board without Bottom; placement uses `boardPadBounds` (the
  pads only) for an `edgeMount` footprint and `boardBounds` for every other. A part off the board
  by its pads is still `Off the board` / `Hangs off the board's edge`.
- **Editor.** `CustomFootprintDraft` gets `opposite` (like `plated`: written only when true,
  read back, compared, in the known keys) and a `Toggle` in the pad table; the footprint's
  `edgeMount` is a toggle on the footprint row; `FootprintPreviewView` draws an opposite pad in
  the Bottom copper colour.
- **Risk** concentrates in the six chip-side sites: a missed one draws or hit-tests an opposite
  pad on the wrong face. `OppositePadTests` exercises each (draw / hit-test / visibility /
  connectivity) with the synthetic footprint, on both sides.

Rejected alternative: storing a layer on `BoardConnectivity.Pad` separately from `side`
(rejected: `side` already is that, and 12 consumer lines read it).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The model and connectivity

- [ ] `Backend/PartCatalog.swift`: `Footprint.Pad.opposite`, `Footprint.edgeMount` and the copies from
      the backend's fields; `Canvas/BoardModel.swift`: `PlacedPad.opposite`, `layerSide(on:)`,
      `boardPads` setting it, `boardPadBounds`.
- [ ] `Canvas/Ratsnest.swift:105`: the pad's `side` is `pad.layerSide(on: side)`.
- [ ] `schematicalTests/OppositePadTests.swift`: a `CatalogPart` made from a library USB part with a
      synthetic footprint (`Footprint` value: a top pad 1 and an opposite pad 2 near the same x) placed
      on Top: `connectivity()`'s pad 1 is on `.top` and pad 2 on `.bottom`; a Bottom trace on pad 2's
      net touching it joins it, a Top trace does not; the same part on Bottom has pad 1 on Bottom and
      pad 2 on Top; `setFootprintSide` swaps them.

Acceptance: connectivity sees the opposite pad on the other layer, on both sides of the board.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p54-dd test -only-testing:schematicalTests/OppositePadTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: app build and one test class).
Commit: —

### Phase 2 — The canvas, hit testing, visibility and the editor

- [ ] `Canvas/SchematicModel.swift` (`padLabel`, `pads(on:)`), `Canvas/PCBCanvasView.swift:359` – `:361`,
      `Canvas/BoardSettings.swift:506`: use `layerSide` (the table of § 2).
- [ ] `Editor/CustomFootprintDraft.swift`, `CustomFootprintEditor.swift`, `FootprintPreviewView.swift`: the
      key, the toggles and the colour (§ 3); `PartEditorWindowTests` gains a check that a draft with an
      opposite pad writes `"opposite": true` and reads it back.
- [ ] `OppositePadTests`: `pads(on: .bottom)` has the opposite pad of a Top part and not its top pad;
      `padLabel(at:)` finds the opposite pad only while Bottom copper is visible; `showsFootprint` is true
      when only Bottom copper is visible (the opposite pad shows); an opposite pad is drawn in the
      Bottom layer's pass (the canvas test's existing render helper).

Acceptance: the canvas, hit testing and visibility follow the pad's layer; the editor keeps the key.
  Check: `xcodebuild … test -only-testing:schematicalTests/OppositePadTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).
Commit: —

### Phase 3 — The checks and the routing

- [ ] `DesignRuleCheck/DesignRuleCheck.swift`: the 1-layer message and the `edgeMount` placement bounds (§ 3).
- [ ] `OppositePadTests`: on a 1-layer board a part with an opposite pad gets one `.layer` issue ("Has pads on
      Bottom, but a 1-layer board has only Top"); on a 2-layer board none; an `edgeMount` footprint whose body
      hangs over the outline with its pads on the board has no `.placement` issue, the same footprint without
      `edgeMount` has "Hangs off the board's edge"; a pad of an `edgeMount` footprint off the board is still
      reported; the router (`RoutingProblemBuilder`) puts the opposite pad on Bottom (an autoroute of two such
      pads uses a Bottom trace or a via, never a Top trace to the pad).

Acceptance: the checks and the router know the layers.
  Check: `xcodebuild … test -only-testing:schematicalTests/OppositePadTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`, plus the existing DRC and routing classes the build lists (`ls schematical/schematicalTests \| grep -i -E 'drc\|designrule\|route'`, each `-only-testing`) → `** TEST SUCCEEDED **` (est. 15 min).
Commit: —

## Validation Plan

- Tests: `OppositePadTests` (every site of § 2, both sides, 1- and 2-layer boards), `PartEditorWindowTests`, the
  existing DRC, routing, ratsnest and canvas classes.
- Coverage check: `grep -rn 'boardPads\|chipPads' --include='*.swift' schematical/schematical` lists no site that
  decides a pad's layer from the part's side without `layerSide` (recorded under § Corrections after Phase 2).
- Runtime proof: O's harness places a straddle-mount socket (an opposite-pad part) in the real app.
- Doc sync: `docs/parts-format.md` (E); the Part Editor's help text is generated from the draft.
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): straddle-mount parts are supported (plan-54-E).
- Author: **A 1-layer board with an opposite pad is a design-rule error, not a placement refusal**: the part
  can still be placed and the user told; this is the existing shape for a Bottom part on a 1-layer board.
- Author: **`edgeMount` relaxes only the placement check**; the copper-to-edge check stays, so a pad too near
  the outline is still reported.

## Corrections

## Summary

F is one model change (`layerSide`), one assignment that fixes twelve consumer lines, six sites that read the part's
side, two checks and the editor. The risk is a missed chip-side site, which `OppositePadTests` covers site by site.
