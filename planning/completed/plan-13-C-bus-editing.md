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

- [x] `SchematicModel.swift`: the port guard in `setBusLines` (`Bus.portsFit`,
      added in plan-13-B Phase 1 for the Bus tool).
- [x] `SchematicCanvasView.swift`: bus drags settle port wires on release
      (they already keep the last good frame on false, from A).
      (`settleWires(attachedToBus:)` settles every port of the bus.)
- [x] Added: `SchematicDocument.dragBus`, which walks each drag frame from
      the last stored offset toward the cursor in grid steps and stops at
      the first refused step; the two canvas bus drags carry that offset
      (`done`). Without it a T end jumped over port 1 (§ Corrections).
- [x] Added: `scripts/harness/support/BusDrag.swift` (drags exactly as the
      canvas does) and `scripts/harness/bus-drag-ports.swift` (the check
      below); `bus-lines.swift` now drags through `BusDrag` too and still
      passes.

Acceptance: on the plan-13-B Phase 2 drawing (port 1 on the horizontal line,
port 2 on the vertical line joined at a T), with a wire on `B0` of each:
dragging the horizontal line down by 50 carries port 1 and its wire, and the
vertical line gets shorter under port 2 without moving it. Dragging it
further, so the vertical line would no longer hold port 2, stops there.
Dragging the vertical line's T end toward port 1 stops at port 1's edge.
Dragging the horizontal line's free end inward stops at port 1's edge.
  Check: build; perform each step and screenshot after each (est. 10 min).
  Result (2026-11-07): build → `** BUILD SUCCEEDED **`; harness
  `bus-drag-ports.swift` → `ALL OK`: down 50 → line at y 150, port 1 tip
  (260,200) with its wire's end on it, vertical line (380,150)–(380,320)
  with port 2's start unchanged; further → stops at y 210 (T at port 2's
  top edge, extent 210..230); the T end toward port 1 → stops at x 270
  (port 1's right edge), also in a single-frame drag; the free end inward →
  stops at x 250. PNGs `c1-1` … `c1-4`.
Commit: 5bd9343

### Phase 2 — Sliding ports

- [x] `SchematicModel.swift`: `slidePort(_:to:)`, which stores only a start
      `fit` returns unchanged. (Plus `slidePort(_:from:by:)`, the clamped
      frame step the canvas calls, so the harness drives the same code.)
- [x] `SchematicCanvasView.swift`: the `.slidePort` interaction, clamping,
      settle on release, and the header comment. (The clamp range is
      `Bus.slideRange`, from `Bus.freeIntervals`, which `fit` now shares.)
- [x] Added: `scripts/harness/bus-slide.swift` (the check below).

Acceptance: dragging a port left stops at the line's left end, at the
neighbouring port's edge, or at a T, in 10-unit steps, and its wires
follow. It never leaves its line.
  Check: build; place two ports on one line, slide the right one left into
  the other and past the end, and screenshot; draw a T line off that line
  between them and slide one port into it, and screenshot (est. 7 min).
  Result (2026-11-07): build → `** BUILD SUCCEEDED **`; harness
  `bus-slide.swift` → `ALL OK`: the right port slides 290, 280, … 180 and
  stops touching the left port (extent 170..190), its wire's end on its
  pin; the left port stops at the line's end (extent 100..140), still on its
  line; with a T at x 280 the right port stops at it (extent 260..280); a
  press with no move selects without moving. PNGs `c2-1` … `c2-3`.
Commit: 8a444a8

### Phase 3 — plan-13 landing

- [x] Draw the plan-13 proof schematic fresh: a bus drawn as an L
      (horizontal, then down), plus a vertical line joined to the
      horizontal line at a T. 5V → `TE-GDS0204` position 1 → port 1 `B0` on
      the horizontal line; port 2 `B0` on the T line (Side Left) →
      `LED-RED-0603` → GND. Rename `B0` to `D0`. Save as
      `/tmp/p13.schematical`.
      → `scripts/harness/bus-landing.swift draw /tmp/p13.schematical`: bus
      lines (200,100)–(520,100), (520,100)–(520,300) (the L),
      (380,100)–(380,320) (the T); "saved /tmp/p13.schematical". (The LED is
      flipped so its anode faces port 2; unflipped, the wire to the anode
      ran across the LED body.)
- [x] Quit, reopen it, run, turn position 1 on, and screenshot the lit LED
      and the `D0` labels on both ports.
      → a second process, `bus-landing.swift open /tmp/p13.schematical`:
      "4 parts, 4 wires, 1 bus, 2 ports", LED 0.0 with the switch off and
      1.0 on, "LANDING OK", pin labels `["D0", "D0"]`; PNG
      `c3-2-reopened-lit` shows the lit LED, green nets across the bus, and
      `D0` on both ports.

Acceptance: the plan-13 behavior after a save and reopen, with an L, a T, a
port on a vertical line and a renamed wire.
  Check: the screenshots above; `jq '.version, (.buses[0].lines|length), (.buses[0].ports|length), .buses[0].wires[0].name' /tmp/p13.schematical`
  → `3`, `3`, `2`, `"D0"` (est. 10 min).
  Result (2026-11-07): the screenshots above; the `jq` line → `3`, `3`, `2`,
  `"D0"`.
  Final gate for plan-13: build → `** BUILD SUCCEEDED **`; `git status
  --short backend schematical/schematical/Parts` → no output (and `git diff
  --stat main -- backend schematical/schematical/Parts` → no output);
  harness `open-file.swift`: `test.schematical` → "5 parts, 6 wires, 0
  buses", `be-6502.schematical` → "15 parts, 32 wires, 0 buses", PNGs
  `c3-test`, `c3-be6502` drawn as before.
Commit: 2f7e0db

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

- **Checks run in the harness** (2026-11-07), as plan-13-A and B record:
  `scripts/harness/build.sh` drives the document calls the gestures make
  and renders the real views to PNG.
- **A drag could jump over a port** (2026-11-07). A's drags computed each
  frame from the press-time bus and stored it if valid. The guard refuses a
  T end inside a port, but once the T end is past the port the shape is
  valid again: dragging the T end from x 380 to x 200 left it at x 210, on
  the far side of port 1 (extent 250..270) — measured in
  `bus-drag-ports.swift` before the fix. "Stops at the last good frame"
  needs the drag to stop at the first refused step, so
  `SchematicDocument.dragBus` walks from the last stored offset toward the
  cursor one grid step at a time and stops there. After the fix the same
  drag, and the same drag in a single frame, stop at x 270.
- plan-13-A and plan-13-B now live in `planning/completed/`.
- **`test.schematical` is a version 1 file, not version 2** (`jq .version
  test.schematical` → `1`; `be-6502.schematical` → `2`). The final gate
  opens both, so versions 1 and 2 are both checked.

## Summary

C makes bus editing safe once ports exist, with one guard in `setBusLines`,
and adds sliding. Bus lines have no nodes, so nothing renumbers. The format,
the netlist and the wires are unchanged. The plan-13 landing gate runs here.
