# plan-05-B: Engine accuracy — outputs switch on time; change counts and light on-time

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-05-A

B fixes the one timing bug found in the engine and adds two read-backs
plan-05-C needs to show a real-time run honestly.

1. **Outputs switch late.** When an input changes while a gate's output
   edge is still pending, and the output value doesn't change, today's
   inertial rule cancels the pending edge and schedules the same value again
   a full delay later. A 74HC32 (OR, 10 ns) whose A rises at 100 ns and B
   rises at 105 ns switches at 115 ns instead of 110 ns. The fix keeps the
   earlier pending edge.
2. **Change counts per net.** `circuit_net_changes` returns how many times a
   net's value has changed since start. C compares two frames to draw a net
   that toggled as "toggling" instead of flickering.
3. **On-time per light.** `circuit_part_light_time` returns, per light, the
   picoseconds it has been on since start. C divides the frame's increase by
   the frame's length to get LED and segment brightness.

**Outcome of B:** `engine_test` passes (output edge at exactly 110 000 ps);
the regenerated `traces.txt` differs from the old one in exactly 2479 lines
(gate 2039, dff 286, jkff 154, every other kind 0); `circuit_check` reads
21 changes on a 1 MHz clock net and 5 000 000 ps of LED on-time after 10 µs.

References:

- plan-05-A (engine design; this plan edits the same `sim.c`).
- `docs/backend.md` § Tests (golden regeneration), § Using it (API notes).
- `backend/include/circuit.h`, `backend/src/circuit.c`,
  `backend/src/models/lights.c`, `lights.h`, `led.c`, `display7.c`.

## Prerequisites

The plan-05 gate is in plan-05-A § Prerequisites. B adds:

| Must be true | Command | Status |
|---|---|---|
| plan-05-A complete | `ls planning/plan-05-A-* 2>/dev/null` → no output; `ls planning/completed/plan-05-A-engine-speed.md` → exists | MET (2026-11-01, worktree-P-05 at 37c5c75) |
| Lazy cancel in place | `grep -n 'is_stale' backend/src/engine/sim.c` → at least one match | MET (2026-11-01, lines 450, 459) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- An inertial drive that schedules the value already pending on that pin, no
  earlier than the pending one, leaves the pending edge in place.
- `circuit_net_changes` and `circuit_part_light_time` exist, are documented
  in `circuit.h` and `docs/backend.md`, and are tested in `circuit_check`.

### Non-goals (explicit constraints)

- No change to any model's logic, any part file, or `parts.h`.
- `circuit_part_lights` keeps its meaning (on at any time during the last
  `circuit_advance`); `trace_check` still hashes it for 7-segment parts.
- Transport drives (`sim_schedule_drive`) keep today's rule: every edge is
  kept.
- No other golden changes: `parts.txt` and `footprints.txt` are untouched.

## 2. Current State

- After plan-05-A, `schedule_drive`'s inertial branch is `pin->gen++`:
  every new inertial drive cancels the pending one, even when both carry the
  same value (`backend/src/engine/sim.c`, `schedule_drive`). Before A it was
  the same rule as an array filter (line 395 at 5aa1fee).
- Gate, dff, jkff and sram outputs use inertial drives
  (`grep -ln sim_schedule_inertial_drive backend/src/models/*.c` → 4 files).
  The gate model re-evaluates a gate on every input change
  (`gate.c` `gate_pin_changed`), so any input change during a pending edge
  re-schedules it.
- Lights: `Light` (`lights.h`) keeps `on`, `ever_on`, `last_on`;
  `lights_set` is called by `led.c` `led_update` and `display7.c`
  `update_leds`; `lights_read` answers "on at any time since T".
- `SimComponentType` (`sim.h`) ends with `lights`, `set_switch`,
  `memory_size`, `memory_read`. Its 13 initializers
  (`grep -rn 'SimComponentType [A-Z_]* = {' backend basicsim | wc -l` → 13)
  are positional and end at `memory_read`; a new last field defaults to
  NULL without touching them.
- `SimNet` (`net.h`) has no change counter; values change only in the round's
  notify loop.

### Measured populations

| What | Count | Command |
|---|---|---|
| Traces that change with the fix | 2479 of 3817 | scratch probe (below): `diff <(sort backend/tests/golden/traces.txt) <(sort /tmp/newtraces.txt) \| grep -c '^>'` → 2479 |
| …by kind | gate 2039, dff 286, jkff 154; sram, switch, led, 7seg, oscillator, supply 0 | python join of the diff with `parts.txt` kind column (field 2) → `Counter({'gate': 2039, 'dff': 286, 'jkff': 154})` |

### Verified properties

- **The fix is right, by example.** Scratch probe (array-scan version of the
  fix on the pre-A engine, `/tmp/simprof`): `trace_check --verbose SN74HC00N`
  before vs after — the first output edges move from 12427 ps / 16885 ps to
  10000 ps / 10000 ps, the part's datasheet 10 ns delay; later edges move
  earlier by the same mechanism.
- **The fix on the A engine gives the same traces.** The per-pin pending
  record of §4.1 on the prototype A engine (`/tmp/simfast`):
  `cmp /tmp/fasttraces.txt /tmp/newtraces.txt` → identical. Speed with it:
  3.2x real time (1 MHz, 400 idle) and 0.09x (100 MHz, 100 idle).
- **Exact counts for the tests.** The oscillator model (`oscillator.c`)
  drives 0 at t = 0 (net Z → 0, one change) and toggles every half period
  (500 000 ps at 1 MHz); `sim_run_until` processes events at times ≤ the
  target. Over 10 µs: 1 + 20 = 21 changes. The output is 1 during
  [0.5, 1), [1.5, 2), … [9.5, 10) µs: 10 × 500 000 = 5 000 000 ps.

## 3. Design Overview

Three independent pieces, in risk order: the read-backs are additive (no
golden can change); the fix changes behavior and regenerates goldens, so it
lands with its RED test first.

**Correctness gate.** Not byte-identity: B changes behavior on purpose. The
gate is (a) `engine_test` RED before the fix, GREEN after; (b) the
regenerated `traces.txt` diff is exactly the predicted 2479 lines, by kind
as in §2. A diff in sram, switch, led, 7seg, oscillator or supply traces is
a bug, not the plan working. `parts.txt` and `footprints.txt` must not
change.

**Rejected alternatives**

- *VHDL-style full inertial rule (pulse-rejection window separate from the
  delay).* Datasheets give no rejection width for these parts; the rule
  "same value already pending earlier → keep it" removes the bug without
  inventing a parameter.
- *Brightness computed inside the backend per frame.* The backend would
  need to know frame boundaries. Cumulative on-time is stateless and the
  app divides by its own frame length.
- *A "toggled since T" query.* Needs the backend to keep per-net history.
  A cumulative counter answers it by subtraction.

## 4. Detailed Design

### 4.1 Inertial fix (`sim.c`)

`InstancePin` gains:

```c
int has_pending;              /* an inertial drive on this pin is pending */
SimTime pending_time;
LogicValue pending_value;
DriveStrength pending_strength;
```

`schedule_drive`:

```c
if (inertial) {
    InstancePin *p = &instance->pins[pin];
    if (p->has_pending && p->pending_value == value &&
        p->pending_strength == strength && p->pending_time <= sim->now + delay) {
        return;                         /* the same edge is already coming, sooner */
    }
    p->gen++;
    p->has_pending = 1;
    p->pending_time = sim->now + delay;
    p->pending_value = value;
    p->pending_strength = strength;
} else {
    instance->pins[pin].has_pending = 0;
}
```

The round clears `has_pending` on the pin of every drive it applies.
`sim.h`'s comment on `sim_schedule_inertial_drive` gains one sentence: a
drive of the value already pending on the pin, no earlier than it, keeps
the pending one.

### 4.2 Change counter

- `net.h`: `SimNet` gains `uint64_t changes`; new
  `uint64_t sim_net_changes(const SimNet *net)` (0 for NULL).
- `sim.c`: in the round's notify loop, `net->changes++` for each dirty net
  whose value differs from its old value (before the `pin_changed` calls).
- `circuit.h` / `circuit.c`:
  `uint64_t circuit_net_changes(const Circuit *circuit, size_t net);` —
  "times the net's value has changed since circuit_start; 0 for a bad index."

### 4.3 Light on-time

- `lights.h`: `Light` gains `SimTime on_total` (on-time up to `changed_at`)
  and `SimTime changed_at`. `lights_set`: if the light was on,
  `on_total += now - changed_at`; then `changed_at = now`; then today's code.
  New `size_t lights_time(const Light *lights, size_t count, SimTime now,
  uint64_t *out, size_t cap)`: `out[i] = on_total + (on ? now - changed_at : 0)`,
  returns `count`.
- `sim.h`: new last member of `SimComponentType`:
  `size_t (*light_time)(const Sim *sim, const SimInstance *instance,
  const void *state, uint64_t *out, size_t cap);` with a comment: cumulative
  picoseconds each light has been on since start, at `sim_now`.
- `led.c`, `display7.c`: add `led_light_time` / `display_light_time`
  (call `lights_time(..., sim_now(sim), ...)`) as the new last initializer.
- `circuit.h` / `circuit.c`: `size_t circuit_part_light_time(const Circuit
  *circuit, size_t part, uint64_t *on_ps, size_t cap);` — same lights and
  order as `circuit_part_lights`; returns the light count (0 when the part
  has none).

### 4.4 Tests

- New `backend/tests/engine_test.c` (`logic_test(engine_test)` in
  CMake): loads `gate.json` (as `basicsim/main.c` does), builds an
  SN74HC32N with an engine-level source instance (transport drives at set
  times) and a probe instance that records `(time, value)` of pin 3 (1Y).
  Cases:
  1. A (pin 1) 0→1 at 100 000 ps, B (pin 2) 0→1 at 105 000 ps → 1Y first
     becomes 1 at **110 000** ps (pre-fix engine: 115 000).
  2. A 0→1 at 100 000, A 1→0 at 105 000, B 0 → 1Y never becomes 1
     (pulse shorter than the delay is still swallowed).
  3. A 0→1 at 100 000 only → 1Y becomes 1 at 110 000.
  VCC/GND pins (14, 7) driven strong 1/0 at 0; all inputs start 0 at 0.
- `circuit_check.c`: new case — LMK6CE001000DDLER (pin 3 OUT on net N, 2 to
  GND net, 4 to VCC net) and LED-RED-0603 (2 = A on N, 1 = K on GND);
  `circuit_advance(c, 10000000)` → `circuit_net_changes(N) == 21`,
  `circuit_part_light_time(led) == 1` with `on_ps[0] == 5000000`;
  `circuit_net_changes` on an out-of-range index → 0.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; moot tasks struck through with
> evidence; fill `Commit:` the moment a phase lands. **An unticked box means
> NOT DONE.**

### Phase 1 — Change counter and light on-time (additive)

No behavior change; no golden can move.

- [x] `net.h`/`sim.c`: `changes` + `sim_net_changes` (§4.2; defined in `net.c`
      beside `sim_net_value`).
- [x] `circuit.h`/`circuit.c`: `circuit_net_changes`.
- [x] `lights.h`/`lights.c`: `on_total`, `changed_at`, `lights_time` (§4.3).
- [x] `sim.h`: `light_time` member; `led.c`, `display7.c` implement it.
- [x] `circuit.h`/`circuit.c`: `circuit_part_light_time`.
- [x] `circuit_check.c`: the §4.4 case (`check_changes_and_light_time`) →
      `PASS: 1 MHz clock net changed 21 times in 10 us`, `PASS: LED on for
      5 000 000 ps of 10 us`, `PASS: net changes for a bad index is 0`.
- [x] Check: `ctest --test-dir /tmp/logic-p05` → `100% tests passed, 0 tests
      failed out of 8`; golden files unmodified (`status --short
      backend/tests/golden` → empty). `basicsim` builds clean with the new
      `SimComponentType` member.

Acceptance: the new case passes and no golden moves.
  Check: `cmake --build /tmp/logic-p05 -j8 && ctest --test-dir /tmp/logic-p05` →
  `100% tests passed, 0 tests failed out of 8` (est. 30 s).
Commit: —

### Phase 2 — Inertial fix, RED test first

- [ ] `backend/tests/engine_test.c` + `logic_test(engine_test)`; commit-local
      run shows case 1 failing with 115000 (RED), cases 2–3 passing.
- [ ] `sim.c`: §4.1.
- [ ] Regenerate: `/tmp/logic-p05/trace_check --print > backend/tests/golden/traces.txt`.
- [ ] Check the diff is the predicted one (acceptance), and spot-check
      `trace_check --verbose SN74HC00N`: first 1Y/2Y/3Y/4Y rising edges at
      10000 ps.
- [ ] `docs/backend.md`: § Tests lists `engine_test`; § Using it notes the
      two new calls and the inertial rule in one line each.

Acceptance: the timing test passes and the golden diff is exactly as
predicted.
  Check 1: `ctest --test-dir /tmp/logic-p05` → `100% tests passed, 0 tests failed out of 9` (est. 30 s).
  Check 2: `git diff -U0 backend/tests/golden/traces.txt | grep -c '^+[^+]'` → `2479`, and
  `git diff --stat backend/tests/golden` → only `traces.txt` (est. 5 s).
Commit: —

## Validation Plan

- Tests: `engine_test` (new: timing, pulse filter), `circuit_check` (new
  case: change count, on-time, bad index), `trace_check` (regenerated).
- Coverage check: case 1 of `engine_test` fails on the pre-fix engine
  (Phase 2 RED step), so the fix is in the test's denominator.
- Runtime proof: plan-05-C's runtime proof reads both new calls in the app.
- Doc sync: `docs/backend.md` (§ Using it, § Tests); `circuit.h` and `sim.h`
  comments.
- Final gate: once, at the end of plan-05-C.

## Decisions

- LED brightness follows on-time; fast wires show as "toggling" — owner,
  2026-11-01. B provides the read-backs for both.
- Keep-the-earlier-same-value-edge as the inertial rule — author,
  2026-11-01 (§3 rejected alternatives).

## Corrections

## Summary

Risk: the golden regeneration. The predicted per-kind diff makes a wrong fix
visible (any non-gate/dff/jkff change is a bug). The read-backs are
additive.
