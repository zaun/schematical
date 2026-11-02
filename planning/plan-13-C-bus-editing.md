# plan-13-C: Buses — editing a bus that has ports, sliding ports

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-13-B

After B, ports work, but A's bus drags ignore them. A drag could shorten a
segment under a port, and a node could be added inside one. This sub-plan:

- makes every bus drag (segment, node) refuse a step that would leave a port
  not fitting its segment. Ports ride along with their segment, and wires on
  their pins re-fit when the drag ends;
- refuses a double-click node inside a port, and renumbers ports' segments
  when a node is added elsewhere;
- lets the Pointer **slide** a port along its own segment (not past a bend),
  clamped by the segment ends and the neighbouring ports;
- runs plan-13's landing gate.

References:

- `planning/plan-13-A-bus-lines.md` § 5 (`BusRoute`), § Prerequisites.
- `planning/plan-13-B-bus-ports.md` § 4 (geometry and the fit rule).

## Prerequisites

The gate is plan-13-A's table. Re-run it, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-13-B complete | `ls planning/plan-13-B-* 2>/dev/null` → no output; `ls planning/completed/plan-13-B-bus-ports.md` → exists | NOT MET (2026-11-01: not started) |
| The fit rule exists | `grep -n "func fit(" schematical/schematical/Canvas/BusModel.swift` → one line | NOT MET (lands in B) |

## 1. Goal

- Dragging a bus segment perpendicular moves the ports on it, and the wires on
  their pins follow. Dragging a node or segment so that a port would no longer
  fit stops at the last frame where every port fits.
- Double-clicking a bus inside a port's extent adds no node. Elsewhere it adds
  one, and every port keeps its place on the bus.
- Dragging a port with the Pointer slides it along its segment in 10-unit grid
  steps, never past a segment end or into another port. The wires on its pins
  follow.

### Non-goals (explicit constraints)

- **No undo** (owner, 2026-11-01).
- **A port never moves to another segment** (owner, 2026-11-01).
- **No change to wire dragging, the netlist, or the file format.** A slide or
  a bus drag changes only `start`, the route points, and wire bends, all of
  which B already saves.

## 2. Current State

- `BusRoute.moveSegment` / `moveNode` / `insertNode` (plan-13-A § 5) return a
  new route or nil, and know nothing of ports. Canvas bus drags call them
  against the drag-start `base` and store non-nil results via `setBusPoints`.
- A port stores `segment` and `start` (plan-13-B § 4). Its across coordinate
  comes from its segment, so a perpendicular segment move already carries it.
  Only the along-axis room can break.
- `Bus.fit` (plan-13-B § 4) answers whether a port's extent fits at a start.
- `settleWires(attachedToPort:)` (plan-13-B § 6) re-fits wires on a port's
  pins.
- Chip moves settle wires on release: `handleEnded`'s `.moveChip` case calls
  `settleWires(attachedTo:)` (`SchematicCanvasView.swift:408`). During a drag,
  `path(forWire:)` re-fits on the fly (`SchematicModel.swift:1038`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Bus drag call sites to guard | 2 | after B: `grep -n "BusRoute.move" schematical/schematical/Canvas/SchematicCanvasView.swift` → 2 (`.dragBusNode`, `.dragBusSegment`); re-measure before starting |

### Verified properties

- **A perpendicular move keeps every along-axis coordinate on its own
  segment.** `BusRoute.moveSegment` drops the along-axis component (A § 5), so
  a port's `start` stays inside the moved segment. Only the neighbouring
  segments change length.
- **`moveNode` changes the length of at most three segments** (k−2→k−1,
  k−1→k, k→k+1, and mirrored), and no segment's index. The point count is
  unchanged, so port `segment` indices stay valid during node and segment
  drags.

## 3. Design

- **`setBusPoints(_:_:) -> Bool`** (was `Void`) refuses (returns false,
  changes nothing) unless, for every port on the bus, `segment` is still a
  valid index, the segment's orientation is unchanged, and
  `fit(port, slots, preferredStart: start) == start`. Canvas drags keep the
  last good frame on false, as they already do for nil routes. This one guard
  covers both drag kinds.
- **On release** of `.dragBusNode` / `.dragBusSegment`, call
  `settleWires(attachedToPort:)` for each port on that bus.
- **Node insert** (`insertBusNode`): refused when the new node lies strictly
  inside any port's extent on that segment. Otherwise every port with
  `segment > i` gets `segment + 1`, and a port on segment i whose `start` is
  past the new node moves to segment i + 1.
- **Slide.** `Interaction.slidePort(port:, base: CGFloat)`. The pointer hit
  on a port (B § 7) starts it. Each frame computes `base + along-axis delta`,
  snapped to the grid, and clamps it to the largest range where `fit` returns
  the value unchanged: the free interval around the port (segment ends and
  neighbouring ports' extents), minus the port's own length. Then
  `document.slidePort(_:to:)` stores it. On release, call
  `settleWires(attachedToPort:)`. A press that does not move only selects.
- Canvas header comment: the Pointer line gains "drag a bus bend or segment
  (ports ride along); drag a port to slide it along its segment".

**Rejected alternatives:**

- *Push ports along when a segment shrinks.* That moves ports the user
  placed, and can cascade into neighbours. Refusing the step keeps every
  port where it was, the same choice as refusing a too-big tick in B.
- *Let a slide continue onto the next segment.* The owner said a slide stays
  on its segment.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot with evidence; fill `Commit:`. **An unticked
> box means NOT DONE.**

### Phase 1 — Bus drags respect ports

- [ ] `SchematicModel.swift`: `setBusPoints` returns `Bool` with the port
      guard; `insertBusNode` refusal and segment renumbering.
- [ ] `SchematicCanvasView.swift`: bus drags keep the last good frame on
      false, and settle port wires on release.

Acceptance: with a 4-pin port in the middle of a horizontal segment, dragging
that segment down by 50 carries the port and its wires. Dragging the next
node so the segment shrinks stops where the segment end meets the port's
extent. Double-clicking inside the port adds nothing; double-clicking left of
it adds a node and the port stays put.
  Check: build; perform each step on the plan-13-B Phase 2 drawing with a
  wire on `B0`, and screenshot after each (est. 8 min).
Commit: —

### Phase 2 — Sliding ports

- [ ] `SchematicModel.swift`: `slidePort(_:to:)`, which stores only a start
      `fit` returns unchanged.
- [ ] `SchematicCanvasView.swift`: the `.slidePort` interaction, clamping,
      settle on release, and the header comment.

Acceptance: dragging a port left stops at the segment's left end or at the
neighbouring port's edge, in 10-unit steps, and its wires follow. It never
leaves the segment.
  Check: build; place two ports on one segment, slide the right one left into
  the other and past the end, and screenshot (est. 5 min).
Commit: —

### Phase 3 — plan-13 landing

- [ ] Draw the plan-13 proof schematic fresh: 5V → `TE-GDS0204` position 1
      → bus port 1 `B0`; a vertical bend in the bus; port 2 `B0` on the
      vertical segment (Side Left) → `LED-RED-0603` → GND. Rename `B0` to
      `D0`. Save as `/tmp/p13.schematical`.
- [ ] Quit, reopen it, run, turn position 1 on, and screenshot the lit LED
      and the `D0` labels on both ports.

Acceptance: the plan-13 behavior after a save and reopen, with a port on a
vertical segment and a renamed wire.
  Check: the screenshots above; `jq '.version, (.buses[0].ports|length), .buses[0].wires[0].name' /tmp/p13.schematical`
  → `3`, `2`, `"D0"` (est. 10 min).
Commit: —

## Validation Plan

- Tests: no app test target; screenshots and `jq`, per phase.
- Coverage check: Phase 1 exercises the guard on both drag kinds plus both
  insert outcomes. Phase 2 exercises both clamps. Phase 3 exercises save and
  open of a vertical-segment port.
- Runtime proof: Phase 3.
- Doc sync: the canvas header comment (Phase 2); `docs/file-format.md` needs
  nothing new (no format change in C).
- **Final gate for plan-13** (run once, here): the app build → `** BUILD
  SUCCEEDED **`; `git status --short backend schematical/schematical/Parts` →
  no output (backend and library untouched); open `test.schematical` (version
  2) and `be-6502.schematical` and screenshot each, to check old files still
  open (est. 8 min). No backend `ctest` run: plan-13 changes no backend file,
  which the `git status` line proves.

## Decisions

- Ports slide along their own segment only — owner, 2026-11-01.
- Moving a bus works like a wire's bend and segment drags but keeps only
  right angles — owner, 2026-11-01.
- A drag that would squeeze a port is refused (it stops at the last good
  frame) rather than pushing ports — author's call, 2026-11-01 (§ 3).

## Corrections

## Summary

C makes bus editing safe once ports exist, with one guard in `setBusPoints`
and port renumbering on insert, and adds sliding. The format, the netlist and
the wires are unchanged. The plan-13 landing gate runs here.
