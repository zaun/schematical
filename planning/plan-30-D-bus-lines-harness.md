# plan-30-D: bus-lines harness — current bus joins

Last updated: 2026-11-08
Effort: small (<1h)
Depends on: nothing. plan-30-C's landing tier depends on D.

Added while landing plan-30 (follow-plan § 4: a prerequisite no letter
covered). plan-30-C's landing tier runs `scripts/harness/bus-lines.swift` and
expects no `FAIL` lines. On the commit before plan-30 (48645a4, run from a
detached checkout) it already printed 7 `FAIL` lines and then crashed
(`harness/main.swift:167: Fatal error: Unexpectedly found nil`, exit 133):

- `corner refused` and `two buses refused`: step 5 still expects the old
  refusals. Since then, leaving an L straight out of one of its lines makes
  it a T, and a drag from one bus to another joins them
  (`SchematicCanvasView.swift` header, Bus tool; `BusJoinTests`
  `testLeavingCornerStraightMakesTee`, `testJoiningTwoBusesMergesThem`).
- The other 5 failures and the crash follow from the extra lines those joins
  leave: step 6a counts 3 buses instead of 2, 6b's four fixed erase points no
  longer clear everything, so the "Extra" and "x3" steps start dirty, and
  `doc.selectedPortID!` is nil at :167.

plan-30 changes none of that behavior; the failures are the same before and
after it.

## Phases

### Phase 1 — Expect today's joins

- [x] Step 5a: a drag straight out of the corner extends the line to
      (420,100) and turns the L at (360,100) into a T.
- [x] Step 5b: the second bus and the joining drag leave one bus, no
      message, with the line (150,300)–(150,100).
- [x] Step 6a: 2 buses (the y 30 line is its own bus).
- [x] Step 6b: erase one line at a time, at its midpoint, until no bus is
      left (at most 20 erases), instead of four fixed points.
- [x] Observed: `scripts/harness/build.sh scripts/harness/bus-lines.swift`
      and run (`APP=/tmp/p30-dd/...`, out dir `/tmp/p30-harness`) → 35 ok
      lines, 0 FAIL, `ALL OK`, exit 0.

Acceptance: bus-lines prints no `FAIL` line and exits 0.
  Check: the harness run above.
Commit: —

## Corrections

## Decisions

- Update the harness to the documented, unit-tested behavior instead of
  dropping it from plan-30-C's gate — author's call: the gate stays as
  strong as written.
