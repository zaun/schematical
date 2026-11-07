# plan-13-C: Buses — editing a bus that has ports, sliding ports

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-13-B

After B, ports work, but A's bus drags ignore them. A drag could shorten a
line under a port, or slide a T into one. This sub-plan:

- makes every bus drag (a line, a free end, a corner, a T end) refuse a step
  that would leave a port not fitting its line. Ports ride along with their
  line, and wires on their pins re-fit when the drag ends. That includes
  ports on a line that stretches because a line joined to it moved;
- lets the Pointer **slide** a port along its own line (never onto another
  line), clamped by the line's ends, the neighbouring ports and the Ts on
  that line;
- runs plan-13's landing gate.

References:

- `planning/plan-13-A-bus-lines.md` § 4–5 (joints, `BusEdit`,
  `setBusLines`), § Prerequisites.
- `planning/plan-13-B-bus-ports.md` § 4 (geometry and the fit rule).

## Prerequisites

The gate is plan-13-A's table. Re-run it, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-13-B complete | `ls planning/plan-13-B-* 2>/dev/null` → no output; `ls planning/completed/plan-13-B-bus-ports.md` → exists | MET (2026-11-07: archived in `0558a28`) |
| The fit rule exists | `grep -n "func fit(" schematical/schematical/Canvas/BusModel.swift` → one line | MET (2026-11-07: line 441) |

## 1. Goal

- Dragging a bus line sideways moves the ports on it, and the wires on their
  pins follow. A drag that would leave a port not fitting its line (the line
  got too short, or a T slid into the port) stops at the last frame where
  every port fits.
- Dragging a port with the Pointer slides it along its line in 10-unit grid
  steps, never past a line end, into another port, or over a T. The wires on
  its pins follow.

### Non-goals (explicit constraints)

- **No undo** (owner, 2026-11-01).
- **A port never moves to another line** (owner, 2026-11-01: "a slide stays
  on its segment"; a bus line is now the only kind of segment).
- **No change to wire dragging, the netlist, or the file format.** A slide or
  a bus drag changes only `start`, the bus lines, and wire bends, all of
  which B already saves.
- **Ports never push each other or a T.** A blocked step is refused.

## 2. Current State

- `BusEdit.moveLine` / `moveEnd` (plan-13-A § 5) return a new line list or
  nil, and know nothing of ports. Canvas bus drags call them against the
  drag-start `base` and store non-nil results via `setBusLines`, which
  checks validity and unchanged joints and returns false to refuse
  (plan-13-A § 4).
- A port stores `line` and `start` (plan-13-B § 4). Its across coordinate
  comes from its line, so a sideways line move already carries it. Only the
  along-axis room can break.
- `Bus.fit` (plan-13-B § 4) answers whether a port's extent fits at a start,
  with Ts counted as obstacles.
- `settleWires(attachedToPort:)` (plan-13-B § 6) re-fits wires on a port's
  pins.
- Chip moves settle wires on release: `handleEnded`'s `.moveChip` case calls
  `settleWires(attachedTo:)` (`SchematicCanvasView.swift:408`). During a drag,
  `path(forWire:)` re-fits on the fly (`SchematicModel.swift:1038`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Bus drag call sites | 2 | after B: `grep -n "BusEdit.move" schematical/schematical/Canvas/SchematicCanvasView.swift` → 2 (`.dragBusEnd`, `.dragBusLine`); re-measured 2026-11-07 → lines 367, 372. Both already go through `setBusLines`, so the guard needs no canvas change beyond settling on release |

### Verified properties

- **A sideways move keeps every along-axis coordinate on the moved line.**
  `BusEdit.moveLine` drops the along-axis component (A § 5), so a port's
  `start` stays where it was on its line. Only the lengths of joined lines,
  and the position of a T end on another line, change.
- **A drag changes the dragged line(s) and one end of each line joined to
  them** (A § 5). Line IDs never change during a drag, so a port's `line`
  stays valid.

## 3. Design

- **`setBusLines` port guard.** After plan-13-A's checks pass, it also
  refuses (returns false, changes nothing) unless every port on the bus
  still fits: its line still exists, the line's orientation is unchanged,
  and `fit(port, slots, preferredStart: start) == start` on the edited bus.
  Because `fit` counts Ts, this one guard also stops a T end sliding into a
  port. Canvas drags already keep the last good frame on false. It covers
  every drag kind.
- **On release** of `.dragBusEnd` / `.dragBusLine`, call
  `settleWires(attachedToPort:)` for each port on that bus.
- **Slide.** `Interaction.slidePort(port:, base: CGFloat)`. The pointer hit
  on a port (B § 7) starts it. Each frame computes `base + along-axis delta`,
  snapped to the grid, and clamps it to the largest range where `fit` returns
  the value unchanged: the free interval around the port (line ends,
  neighbouring ports' extents, and Ts), minus the port's own length. Then
  `document.slidePort(_:to:)` stores it. On release, call
  `settleWires(attachedToPort:)`. A press that does not move only selects.
- Canvas header comment: the Pointer line gains "drag a bus line, end, corner
  or T (ports ride along); drag a port to slide it along its line".

**Rejected alternatives:**

- *Push ports along when a line shrinks.* That moves ports the user
  placed, and can cascade into neighbours. Refusing the step keeps every
  port where it was, the same choice as refusing a too-big tick in B.
- *Let a slide continue onto the next line.* The owner said a slide stays
  on its segment.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot with evidence; fill `Commit:`. **An unticked
> box means NOT DONE.**

### Phase 1 — Bus drags respect ports

- [ ] `SchematicModel.swift`: the port guard in `setBusLines`.
- [ ] `SchematicCanvasView.swift`: bus drags settle port wires on release
      (they already keep the last good frame on false, from A).

Acceptance: on the plan-13-B Phase 2 drawing (port 1 on the horizontal line,
port 2 on the vertical line joined at a T), with a wire on `B0` of each:
dragging the horizontal line down by 50 carries port 1 and its wire, and the
vertical line gets shorter under port 2 without moving it. Dragging it
further, so the vertical line would no longer hold port 2, stops there.
Dragging the vertical line's T end toward port 1 stops at port 1's edge.
Dragging the horizontal line's free end inward stops at port 1's edge.
  Check: build; perform each step and screenshot after each (est. 10 min).
Commit: —

### Phase 2 — Sliding ports

- [ ] `SchematicModel.swift`: `slidePort(_:to:)`, which stores only a start
      `fit` returns unchanged.
- [ ] `SchematicCanvasView.swift`: the `.slidePort` interaction, clamping,
      settle on release, and the header comment.

Acceptance: dragging a port left stops at the line's left end, at the
neighbouring port's edge, or at a T, in 10-unit steps, and its wires
follow. It never leaves its line.
  Check: build; place two ports on one line, slide the right one left into
  the other and past the end, and screenshot; draw a T line off that line
  between them and slide one port into it, and screenshot (est. 7 min).
Commit: —

### Phase 3 — plan-13 landing

- [ ] Draw the plan-13 proof schematic fresh: a bus drawn as an L
      (horizontal, then down), plus a vertical line joined to the
      horizontal line at a T. 5V → `TE-GDS0204` position 1 → port 1 `B0` on
      the horizontal line; port 2 `B0` on the T line (Side Left) →
      `LED-RED-0603` → GND. Rename `B0` to `D0`. Save as
      `/tmp/p13.schematical`.
- [ ] Quit, reopen it, run, turn position 1 on, and screenshot the lit LED
      and the `D0` labels on both ports.

Acceptance: the plan-13 behavior after a save and reopen, with an L, a T, a
port on a vertical line and a renamed wire.
  Check: the screenshots above; `jq '.version, (.buses[0].lines|length), (.buses[0].ports|length), .buses[0].wires[0].name' /tmp/p13.schematical`
  → `3`, `3`, `2`, `"D0"` (est. 10 min).
Commit: —

## Validation Plan

- Tests: no app test target; screenshots and `jq`, per phase.
- Coverage check: Phase 1 exercises the guard on a line drag, a T-end drag
  and a free-end drag, and a port carried by its line. Phase 2 exercises
  every clamp (end, port, T). Phase 3 exercises save and open of a bus with
  an L and a T and a port on a vertical line.
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

- Ports slide along their own segment only — owner, 2026-11-01. (A bus
  line is the segment since 2026-11-07.)
- Moving a bus works like a wire's bend and segment drags but keeps only
  right angles — owner, 2026-11-01. Since 2026-11-07 that means line, end,
  corner and T drags (plan-13-A § 5).
- A drag that would squeeze a port is refused (it stops at the last good
  frame) rather than pushing ports — author's call, 2026-11-01 (§ 3).

## Corrections

## Summary

C makes bus editing safe once ports exist, with one guard in `setBusLines`,
and adds sliding. Bus lines have no nodes, so nothing renumbers. The format,
the netlist and the wires are unchanged. The plan-13 landing gate runs here.
