# Drag Speed Plan

Last updated: 2026-11-07
Effort: small (<1h)

Dragging a part or a bus on the schematic canvas is slow, while panning is
smooth. The cause is the parts library in the sidebar. It is hidden while
something is selected, but it still rebuilds its whole catalog list on every
drag step. This plan stops that rebuild. Afterward, a drag step costs about as
much as a pan step.

References:

- `schematical/schematical/Drawer/SchematicSidebarView.swift` (the sidebar
  that hosts the library)
- `schematical/schematical/Drawer/ChipDrawerView.swift` (the library)
- `schematical/schematical/Canvas/SchematicCanvasView.swift:handleChanged`
  (the drag gesture)
- `scripts/harness/build.sh` (the headless harness used for the timings)

## Prerequisites

None. Every file this plan touches is on `main` today.

## 1. Goal

- With the sidebar shown and the 40-pin W65C02S (U8) in `be-6502.schematical`
  selected, one drag step of that part takes at most 15 ms (median) in the
  harness timing script. Today it takes 163 ms. One drag step of a selected
  bus line drops the same way.

### Non-goals (explicit constraints)

- The library keeps every behavior it has today: search, the category,
  package and voltage filters, the Settings visibility toggles, the open state
  of its groups, its scroll position, and filling in once the catalog loads.
- Part drags still snap to the 10-pt grid as they move (`moveChip` is not
  changed).
- No change to the file format, the document model or the canvas drawing code.
- Search speed is not part of this plan. Each `catalog.groups(matching:)` call
  costs 48–50 ms in the debug harness, so typing in the search field does that
  much work per keystroke.

## 2. Current State

- Every drag step mutates the document. `handleChanged` calls
  `document.moveChip` for a part and `document.dragBus` for a bus. Both write to
  `SchematicDocument.chips` / `.buses`, which are `@Observable`
  (`Canvas/SchematicModel.swift:916`, `:1697`). Panning changes only
  `CanvasViewport.offset`, so the document is not touched.
- `SchematicSidebarView.body` reads `document.chips` through `selected`, and
  `document.buses` / ports through `selectedBus` / `selectedPort`
  (`Drawer/SchematicSidebarView.swift:15-28`). So when a selected part or bus
  moves, the sidebar's body runs again.
- That body creates `ChipDrawerView(catalog: catalog)` inside a `ZStack`.
  SwiftUI does not skip the library's body even though its only input (the
  catalog reference) is unchanged. The library's body then calls
  `catalog.groups(matching:)` and rebuilds its `List` of every category,
  subcategory and function (`Drawer/ChipDrawerView.swift:41-95`).
- Wire drags (`setWirePath`) mutate only `document.wires`, which the sidebar
  does not read, so wire drags are not affected.

### Measured populations

All timings come from the debug harness build of `/tmp/drag-probe/main.swift`
(described in Phase 1). It hosts `SchematicCanvasView` (1200×900), plus
the 350-pt sidebar where noted, in a real `NSWindow`, with
`be-6502.schematical` open. Each step pumps the run loop, lays out, displays
and flushes. The tables show the median of 120 steps.

| What | Count | Command |
|---|---|---|
| Parts / wires in `be-6502.schematical` | 15 / 66 | `harness be-6502.schematical` → `parts 15 wires 66` |
| Pan step, canvas only | 3.76 ms | same run → `pan: median 3.76 ms` |
| Part drag step, canvas only | 5.57 ms | same run → `moveChip: median 5.57 ms` |
| Pan step, canvas + sidebar | 4.43 ms | `harness be-6502.schematical sidebar` → `pan: median 4.43 ms` |
| Part drag step, canvas + sidebar | 168.69 ms | same run → `moveChip: median 168.69 ms` |
| Part drag step, sidebar = library only (re-evaluated parent) | 163.09 ms | `… sidebar drawer` → `moveChip: median 163.09 ms` |
| Part drag step, sidebar = part panel only (Form) | 8.40 ms | `… sidebar form` → `moveChip: median 8.40 ms` |
| Part drag step, library behind an always-equal `.equatable()` wrapper | 7.52 ms | `… sidebar drawereq` → `moveChip: median 7.52 ms` |
| Bus line drag step, full sidebar (p90; even steps are no-ops) | 171.59 ms | `… sidebar full` → `dragBusLine: … p90 171.59 ms` |
| Bus line drag step, library isolated (p90) | 5.63 ms | `… sidebar drawereq` → `dragBusLine: … p90 5.63 ms` |
| One `catalog.groups(matching: DrawerFilter())` call | 47.8–50.5 ms | probe loop of 10 calls → `catalog.groups(matching:): 50.452 ms` |

### Verified properties

- **The Phase 1 change fixes the drag.** Tested in a scratch copy
  (`/tmp/fixcopy`) with exactly the two edits from Phase 1, using the full
  sidebar: `moveChip: median 9.24 ms`, `dragBusLine: … p90 9.07 ms`, and
  `pan: median 4.16 ms`.
- **The library still updates on its own.** Tested in the same scratch copy
  with `/tmp/drawer-probe/main.swift`. It hosts the sidebar with a catalog
  that has not loaded yet, then calls `catalog.load()`. The rendered PNG shows
  the full category list and the footer "7,385 parts from backend". Typing
  "74HC00" into the search field then narrows the list to Logic › NAND. So
  the library still redraws when the catalog it observes changes or its own
  `@State` changes. `.equatable()` only blocks redraws caused by its parent.

## 3. Design Overview

Make `ChipDrawerView` `Equatable`, comparing only the identity of its catalog,
and apply `.equatable()` where the sidebar creates it. Then a redraw of the
sidebar that does not swap the catalog no longer redraws the library. The
library still redraws from its own `@State` / `@AppStorage` and from
Observation on `catalog`, as the probe showed.

Correctness risk: if the library ever gets a new input that its `==` ignores,
changes to that input would not show. Today it has none: its only stored
input is `catalog`. The `openGroups` init argument only seeds `@State`. The
doc comment on `==` says to update it when an input is added.

This is not a byte-identity plan. Behavior is unchanged, and the gate is the
harness timing plus the drawer probe.

Rejected alternatives:

- **Stop the sidebar from reading `document.chips` / `.buses` during a drag**,
  for example by moving those lookups into a child view. That also works,
  but it is fragile: any later read of a document array in the sidebar body
  brings the 160 ms back. The equatable boundary protects the library no
  matter what the parent reads.
- **Cache `catalog.groups(matching:)`.** That cuts only the grouping, about
  50 of the 163 ms. Rebuilding the `List` is the rest.
- **Draw the dragged part at the raw cursor position and snap on drop.**
  This would change how dragging looks, and the timings show the snap is not
  why dragging is slow.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Fill `Commit:` when it lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Keep the library out of sidebar redraws

This fixes the drag, and adds the timing script so the slowdown can be
measured again later.

- [ ] `schematical/schematical/Drawer/ChipDrawerView.swift`: declare
      `struct ChipDrawerView: View, Equatable` and add
      `static func == (lhs:rhs:) -> Bool { lhs.catalog === rhs.catalog }`,
      with a doc comment saying the sidebar redraws whenever the selected part
      moves, and that any new stored input must be compared here.
- [ ] `schematical/schematical/Drawer/SchematicSidebarView.swift`: add
      `.equatable()` directly after `ChipDrawerView(catalog: catalog)`, before
      the `.disabled` / `.opacity` modifiers.
- [ ] Add `scripts/harness/drag-timing.swift`, based on
      `/tmp/drag-probe/main.swift`. It opens the file named in its first
      argument and selects the part with the most pins. It hosts
      `SchematicCanvasView` plus `SchematicSidebarView` (350 pt) in an
      `NSWindow`. It then prints the median and p90 of 120 steps each for
      `pan`, `moveChip` (±10-pt steps) and `dragBusLine` (first bus, first
      line, when the file has a bus). Each step runs
      `RunLoop.main.run(mode: .default, before: Date())` three times, then
      `layoutSubtreeIfNeeded()`, `displayIfNeeded()` and `CATransaction.flush()`.
      The usage comment at the top follows `open-file.swift`.
- [ ] Add `scripts/harness/drawer-updates.swift`, based on
      `/tmp/drawer-probe/main.swift`. It hosts the sidebar with an unloaded
      catalog, calls `catalog.load()`, types "74HC00" into the search field,
      and writes `1-before-load.png`, `2-after-load.png` and `3-search.png`
      to the directory named in its first argument.

Acceptance: a selected part's drag step and a selected bus line's drag step,
with the sidebar shown, each take at most 15 ms. The library still fills in
after the catalog loads, and still filters on search.
  Check: `scripts/harness/build.sh scripts/harness/drag-timing.swift && /tmp/schematical-harness/H.app/Contents/MacOS/harness be-6502.schematical`
  → `moveChip: median` ≤ 15 ms and `dragBusLine: … p90` ≤ 15 ms
  (est. 2 min including the build).
  Check: `scripts/harness/build.sh scripts/harness/drawer-updates.swift && /tmp/schematical-harness/H.app/Contents/MacOS/harness /tmp/drawer-check`
  → `2-after-load.png` lists the categories (Connector … Memory) and
  `3-search.png` shows only Logic › NAND (est. 2 min).
Commit: —

## Validation Plan

- Tests: the app has no XCTest target. The two harness scripts from Phase 1
  are the regression checks. Backend tests are not affected, because no C
  code or part file changes.
- Coverage check: the timing script uses the app's real
  `SchematicSidebarView` and `ChipDrawerView`, compiled from
  `schematical/schematical`, so the change being tested is in the build.
- Runtime proof: in the app, open `be-6502.schematical` with the parts drawer
  shown. Dragging U8 across the canvas and dragging the bus line should both
  follow the cursor as smoothly as panning does. Toggle "Show obsolete parts"
  in Settings with nothing selected: the library's list changes.
- Doc sync: none. No doc describes sidebar redraws.
- Final gate (run once, after Phase 1): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED`, plus both Phase 1 harness checks (est. 4 min).

## Decisions

- Fix the drag with an equatable boundary on the library rather than by
  changing what the sidebar reads. Decided by the plan author, 2026-11-07,
  from the measurements above (§3).

## Corrections

## Summary

The whole slowdown is the hidden parts library being rebuilt on every
drag step: 163 of the 169 ms. Part and bus drags are affected. Panning and
wire drags are not, because they do not touch the arrays the sidebar reads.
The fix is two lines plus two harness scripts. The remaining drag cost
(7–9 ms in the debug harness) is the canvas redrawing wires and the moved
part, which is in line with panning (4 ms) and is left alone.
