# plan-05-A: Event engine speed — same results, cost no longer grows with circuit size

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d) — plan-05 is A → B → C
Effort: large (3h–1d)
Depends on: nothing

plan-05 makes the simulator fast and accurate. The owner's report: "it's
about 1000 times too slow". Two causes, both measured (§2):

1. **The app runs circuits at 1/1600 of real time on purpose.** It advances
   10 µs of circuit time per 16 ms frame (`Simulation.swift`
   `picosecondsPerTick`, `tickInterval`).
2. **The engine's cost per event grows with the size of the whole circuit.**
   Every event step copies the whole event queue, resolves every net and
   scans every net twice, so idle chips slow down the busy ones. A 1 MHz
   counter next to 400 idle chips runs at 0.0012× real time.

plan-05 is three sub-plans, in this order:

- **A (this file)** rebuilds the engine core so each event step costs what
  the events touch, not what the circuit holds. Results are byte-identical.
- **B** fixes a timing bug (gate and flip-flop outputs switch late) and adds
  read-backs the app needs at real time: per-net change counts and per-light
  on-time.
- **C** runs the simulation in the app at real time on a background thread,
  with a speed picker, the reached speed when the engine can't keep up,
  "toggling" wires and LED brightness by on-time.

**Outcome of A:** `sim_bench LMK6CE001000DDLER 16 10 400` (Release build)
runs at ≥ 1.5× real time (today 0.0012×), and every one of the 3817 part
traces in `backend/tests/golden/traces.txt` is unchanged.

References:

- `docs/backend.md` (layout, tests, golden regeneration).
- `backend/src/engine/sim.c`, `sim.h`, `net.c`, `net.h` — the whole engine.
- `backend/tests/support/trace_core.c` — the trace harness behind
  `trace_check`.

## Prerequisites

This table is the gate for all of plan-05; B and C point here and add their
own rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p05 && cmake --build /tmp/logic-p05 -j8 && ctest --test-dir /tmp/logic-p05` → `100% tests passed, 0 tests failed out of 8` | MET (2026-11-01) |
| plan-04 not started (owner: plan-05 lands first) | `ls backend/src/models/eeprom.c schematical/schematical/Parts/eeprom.json 2>/dev/null` → no output | MET (2026-11-01) |
| Engine files as cited | `wc -l backend/src/engine/sim.c` → `874` | MET (2026-11-01) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify — a
> prerequisite recorded NOT MET may have landed since, and one recorded MET may
> have regressed.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- The engine's work per event step depends only on the events in that step
  and the nets they touch, not on the number of nets, instances or pending
  events in the circuit.
- `trace_check` passes with the existing goldens: no simulated behavior
  changes in A.

### Non-goals (explicit constraints)

- No change to `sim.h`, `net.h` public declarations, `circuit.h`, `parts.h`,
  or any model file. Models keep calling `sim_schedule_drive`,
  `sim_schedule_inertial_drive`, `sim_schedule_timer`, `sim_set_contact`,
  `sim_read_pin` with today's meaning.
- No behavior change: same event order within a time step, same
  "did not settle" limit (`SIM_MAX_ROUNDS` 10000 rounds at one time), same
  short detection (the lowest-index shorted net is the one reported).
- The timing fix is plan-05-B, not A. A keeps today's inertial rule exactly
  (a new inertial drive cancels every pending drive on that pin).
- No multithreading inside the engine.

## 2. Current State

All citations are to `backend/src/engine/sim.c` at 5aa1fee.

- `append_event` (line 341) appends to an unsorted array `sim->events`.
- `schedule_drive` (line 361), inertial branch (line 395): walks the **whole**
  event array and removes every `EVENT_DRIVE` on the same instance and pin.
  O(pending events) per gate evaluation.
- `sim_run_until` (line 829): each round scans the **whole** array for the
  minimum time.
- `process_round` (line 729), per round:
  - `malloc` a batch the size of the whole queue and an `old_values` array
    the size of all nets (lines 736, 743);
  - copies every event into batch-or-remaining, keeping order (stable);
  - snapshots every net's value;
  - `resolve_nets` (line 600) resolves **every** net; with any closed contact
    it also runs a union-find over all nets with four more `malloc`s;
  - scans every net for `shorted`, then every net for a changed value, and
    calls `pin_changed` for each pin of each changed net, in net-index order;
  - calls `timer` for each timer event in batch order.
- Event order today: the array is always in insertion order (append at the
  end; both filters above are stable). A round takes every event with
  `time == T` in that order. So "ordered by (time, insertion number)"
  reproduces it exactly.
- `net_resolve_group` (`net.c`) is O(pins on the nets in the group). It is
  not a hot spot and is not changed.
- `sim_start` (line 692) resolves all nets once and calls every `start`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Part traces in the golden | 3817 | `grep -c . backend/tests/golden/traces.txt` → 3817 |
| ctest programs | 8 | `ctest --test-dir /tmp/logic-p05 -N \| tail -1` → `Total Tests: 8` |
| `SimComponentType` initializers (unchanged in A) | 13 in 12 files | `grep -rn 'SimComponentType [A-Z_]* = {' backend basicsim \| wc -l` → 13 |
| Callers of the inertial API | 4 models | `grep -ln sim_schedule_inertial_drive backend/src/models/*.c` → gate.c, dff.c, jkff.c, sram.c (4 files; sram uses delay 0) |

### Measured speed today (Mac14,5, Apple M2 Max; Release `-O2`)

Bench circuit (§4.4): a VCC, a GND, an oscillator, 16 D flip-flop stages
(8 × SN74HC74N) as a ripple counter with an LED on each Q, plus N idle
SN74HC00N chips with every gate pin on its own net.
`/tmp/simbench/bench <osc> <stages> <sim ms> <idle chips>`:

| Circuit | Old engine |
|---|---|
| 1 MHz (`LMK6CE001000DDLER`), 16 stages, 0 idle | 0.41× real time |
| same, 10 idle chips | 0.13× |
| same, 100 idle chips | 0.0069× |
| same, 400 idle chips | 0.0012× |
| 100 MHz (`LMK6CE10000CDLER`), 16 stages, 100 idle | 0.0001× |

Instrumented build (counters added to a copy of `sim.c`): 1 ms of the
16-stage 1 MHz bench is 6009 rounds and 8077 events with 0 idle chips and
6010 rounds and 9677 events with 400 idle chips — the same work, yet wall
time goes from 0.002 s to 0.381 s. The cost is the per-round full scans.

### Verified properties

- **Prototype, byte-identical.** The design in §3–§4 was built in a scratch
  copy (`/tmp/simfast/backend`, heap + per-pin generation + dirty nets +
  cached contact groups). `ctest` → `100% tests passed, 0 tests failed out
  of 8`, so `trace_check` matched all 3817 golden hashes, and
  `switch_test` / `display7_test` (contacts) and `circuit_check` passed.
- **Prototype speed** (same machine, same bench):

  | Circuit | Old | Prototype |
  |---|---|---|
  | 1 MHz, 16 stages, 0 idle | 0.41× | 2.88× |
  | 1 MHz, 10 idle | 0.13× | 2.89× |
  | 1 MHz, 100 idle | 0.0069× | 2.83× |
  | 1 MHz, 400 idle | 0.0012× | 2.27× |
  | 100 MHz, 100 idle | 0.0001× | 0.078× |

- **100 MHz cannot reach real time with any event engine design here.** At
  100 MHz the clock alone is 200 M edges per second before any downstream
  event; the prototype spends ≈ 43 ns per event (2.9× real time on ≈ 8 M
  events per simulated second). The app handles this by showing the reached
  speed (plan-05-C).

## 3. Design Overview

Four independent changes inside `sim.c`, layered:

1. **Priority queue.** `sim->events` becomes a binary min-heap ordered by
   `(time, seq)`, where `seq` is a `uint64_t` stamped by `append_event` from
   a counter in `Sim`. Same order as today (§2).
2. **Lazy inertial cancel.** `InstancePin` gets `uint32_t gen`. An inertial
   drive does `pin->gen++` and stores the new value in the event; a transport
   drive stores the current `gen`. A popped `EVENT_DRIVE` whose `gen` differs
   from its pin's `gen` is stale and is dropped. This cancels exactly the
   events today's filter removes: every drive on that pin scheduled before
   the inertial one, transport or inertial.
3. **Dirty nets.** A round marks the net of each applied drive dirty (saving
   its old value the first time). Only dirty nets are resolved, checked for
   a short and checked for a change. Dirty indexes are sorted ascending
   before the short check and the `pin_changed` calls, so both run in
   today's net-index order.
4. **Cached contact groups and reused buffers.** The union-find grouping is
   rebuilt only when a contact event is applied (and once at start), not
   every round. A dirty net in a group dirties the whole group. All per-round
   arrays are allocated once in `sim_start` (sized by `net_count`) or grown
   by doubling (the batch).

Correctness risk concentrates in (2) and (3): a stale event that is not
skipped, or a net whose value changes but is not marked dirty, changes
behavior. Both are caught by `trace_check` (3817 parts × 30 steps of random
stimulus, hashed), and contacts by `switch_test` and `display7_test`.

**Correctness gate: byte-identity.** A is a provably-neutral refactor, so the
gate is "every golden trace hash unchanged" (`trace_check`). A mismatch is a
bug in the new code: run `trace_check --verbose <part>` on the old and new
builds, diff, and fix. It is never a reason to regenerate goldens in A.

**Rejected alternatives**

- *Timing wheel / calendar queue instead of a heap.* Faster for dense,
  regular delays, but event times here range from 0 ps to multi-ms timers;
  a heap is simpler, and the prototype already reaches 2–3× real time on the
  target circuits.
- *Eager removal of cancelled events from the heap.* Needs a per-event
  index back-pointer and sift on removal; lazy deletion is simpler and the
  stale count is bounded by the schedule rate.
- *Incremental driver counts per net (O(1) resolve).* `net_resolve_group`
  is O(pins on the net) and only runs on dirty nets; not a measured hot spot.
- *Keeping full `resolve_nets` and only fixing the queue.* The 400-idle-chip
  case is dominated by the per-round full net scans (§2), so it would stay
  slow.

## 4. Detailed Design

### 4.1 Data (all private to `sim.c`)

```c
typedef struct {
    SimNet *net;
    size_t net_pin_index;
    uint32_t gen;          /* bumped by each inertial drive on this pin */
} InstancePin;

typedef struct { ...today's fields...; uint64_t seq; uint32_t gen; } Event;

struct Sim {
    ...today's fields...
    uint64_t next_seq;
    Event *batch;    size_t batch_capacity;     /* one round's events */
    size_t *dirty;   size_t dirty_count;        /* net indexes, this round */
    unsigned char *is_dirty;                    /* per net */
    LogicValue *old_values;                     /* per net, valid while dirty */
    size_t *group_parent, *group_root, *group_head, *group_next;  /* per net */
    SimNet **group_nets;                        /* per net, scratch for net_resolve_group */
    int groups_valid;
};
```

`sim_destroy` frees the new arrays.

### 4.2 Queue

- `append_event`: stamp `seq = sim->next_seq++`, push, sift up with
  `less(a,b) = a.time < b.time || (a.time == b.time && a.seq < b.seq)`.
- `heap_pop`: move the last element to the root, sift down.
- `is_stale(e)`: `e->type == EVENT_DRIVE && e->gen != e->instance->pins[e->pin].gen`.
- `schedule_drive`: inertial → `instance->pins[pin].gen++`; then
  `event.gen = instance->pins[pin].gen` for both kinds; append.
- `sim_run_until` loop: pop stale events off the top; stop if the heap is
  empty or the top time > target; otherwise the round bookkeeping
  (`round_time`, `rounds_at_time`, `SIM_MAX_ROUNDS`) is unchanged and the
  round runs. Dropping stale tops first keeps "no round for a time whose
  events were all cancelled", as today.

### 4.3 Round

```
batch = pop every non-stale event with time == T (in heap order = today's order)
dirty_count = 0; contact_changed = 0
for each event in batch:
    DRIVE:   if pin has a net: mark_dirty(net); apply_drive
    CONTACT: mark_dirty(every net); apply_contact; contact_changed = 1
if closed_contact_count == 0:
    resolve each dirty net alone; if contact_changed: groups_valid = 0
else:
    if contact_changed or !groups_valid: build_groups()     (today's union-find, into the cached arrays)
    for each originally dirty net: mark_dirty every net in its group
    for each dirty net that is its group's root: net_resolve_group(group members)
sort dirty ascending
short check over dirty (first shorted → shorted_net + "smoke:" error), as today
clear is_dirty for all dirty
for each dirty net in order whose old value != value: pin_changed for each pin (today's loop)
for each TIMER event in batch: timer callback (today's loop)
```

`mark_dirty(k)`: if `!is_dirty[k]`: set it, `old_values[k] = nets[k]->value`,
append `k`. Use insertion sort when `dirty_count` ≤ 16, `qsort` above.

Why this matches today exactly:

- A net's value only changes when a drive on one of its pins is applied or
  its contact group changes; both mark it dirty, so every net today's loop
  would see as changed is in `dirty`.
- A net that was not dirty was already resolved with the same inputs, so
  re-resolving it (today) leaves value and `shorted` unchanged.
- No net can be already shorted at round start: a short sets the error and
  the run stops.

### 4.4 Bench tool

`backend/tests/sim_bench.c`, built as an executable only (no `add_test`; a
wall-clock check would make ctest flaky). It is the scratch bench from §2
(`/tmp/simbench/bench.c`) moved into the repo:

- args `<oscillator part> <stages> <sim ms> <idle chips>`;
- loads footprints, supply, led, oscillator, dff, gate from `PARTS_DIR`;
- builds VCC, GND, the oscillator (pins 4=VDD, 2=GND, 3=OUT), `stages`
  D flip-flops from SN74HC74N units (CLR, PRE tied to VCC; D = QBAR;
  CLK = previous stage's QBAR, the first stage's CLK = OUT) with an
  LED-RED-0603 from each Q to GND, plus `idle chips` SN74HC00N with each of
  the 12 gate pins on its own net;
- advances in 10 µs steps (`circuit_advance`) to `sim ms`, prints
  `osc=<part> stages=<n> extra_chips=<n> sim=<ms>ms wall=<s>s  => <x>x real time`;
- exits 1 with the circuit error on any failure.

Build it with `-DCMAKE_BUILD_TYPE=Release`; numbers in this plan are Release.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial with one line on what remains;
> moot tasks as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Bench tool and baseline

Adds the measuring stick; no engine change, so it lands alone safely.

- [x] `backend/tests/sim_bench.c` as §4.4. Speed printed with `%.4g`
      (`%.4f` showed the 100 MHz baseline as `0.0000x`).
- [x] `backend/CMakeLists.txt`: `add_executable(sim_bench tests/sim_bench.c)`,
      linked to `logic_test_support`, with the same `PARTS_DIR` definition as
      `logic_test` — no `add_test`.
- [x] Record the baseline in this plan under "Bench results" (both commands
      in the acceptance line).

Acceptance: the bench builds and reproduces the §2 baseline order of
magnitude.
  Check: `cmake -S backend -B /tmp/logic-p05-rel -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p05-rel -j8 --target sim_bench && /tmp/logic-p05-rel/sim_bench LMK6CE001000DDLER 16 2 400 && /tmp/logic-p05-rel/sim_bench LMK6CE10000CDLER 16 1 100`
  → two lines; first ≤ 0.01x real time, second ≤ 0.001x (est. 15 s).
Commit: fea28b7

### Phase 2 — Heap queue, lazy inertial cancel, reused batch

- [x] `sim.c`: `seq`/`gen` fields, `next_seq`, heap `append_event` and
      `heap_pop`, `is_stale`, the inertial branch of `schedule_drive`
      replaced by `gen++` (§4.2). Plus `drop_stale` (pop stale tops).
- [x] `sim.c`: `sim_run_until` pops stale tops and reads the min from the
      heap top; `process_round` pops its batch from the heap into the reused
      `sim->batch` (`take_batch`; still resolving all nets — dirty nets are Phase 3).
- [x] `sim_destroy` frees `batch`.
- [x] Check run: `cmake --build /tmp/logic-p05 -j8 && ctest --test-dir /tmp/logic-p05`
      → `100% tests passed, 0 tests failed out of 8` (no compiler warnings).

Acceptance: byte-identical behavior.
  Check: `cmake --build /tmp/logic-p05 -j8 && ctest --test-dir /tmp/logic-p05` →
  `100% tests passed, 0 tests failed out of 8` (est. 30 s). `trace_check` is
  the decisive one; on a mismatch see §3 gate.
Commit: —

### Phase 3 — Dirty nets and cached contact groups

- [ ] `sim.c`: per-net arrays allocated in `sim_start` (`dirty`, `is_dirty`,
      `old_values`, `group_*`, `group_nets`), freed in `sim_destroy`;
      allocation failure sets `"out of memory"` and `sim_start` returns 0.
- [ ] `sim.c`: round body as §4.3; `resolve_nets` stays only for
      `sim_start` and is rewritten to fill the cached groups (`build_groups`)
      instead of its own `malloc`s.
- [ ] Remove the old per-round `malloc`s; `grep -n malloc backend/src/engine/sim.c`
      shows only creation paths (`copy_string`, `sim_add_instance`, grow
      helpers, `sim_start`).

Acceptance: byte-identical behavior and the speed target.
  Check 1: `cmake --build /tmp/logic-p05 -j8 && ctest --test-dir /tmp/logic-p05` →
  `100% tests passed, 0 tests failed out of 8` (est. 30 s).
  Check 2: `cmake --build /tmp/logic-p05-rel -j8 --target sim_bench && /tmp/logic-p05-rel/sim_bench LMK6CE001000DDLER 16 10 400 && /tmp/logic-p05-rel/sim_bench LMK6CE10000CDLER 16 10 100`
  → first ≥ 1.5x real time, second ≥ 0.05x (prototype: 2.27–3.2x and
  0.078–0.09x) (est. 10 s).
Commit: —

## Bench results

(Filled in by Phases 1 and 3: command, line printed, date.)

Phase 1 baseline (old engine, Release, 2026-11-01), the Phase 1 Check:

- `sim_bench LMK6CE001000DDLER 16 2 400` →
  `osc=LMK6CE001000DDLER stages=16 extra_chips=400 sim=2.000ms wall=3.235s  => 0.0006183x real time`
- `sim_bench LMK6CE10000CDLER 16 1 100` →
  `osc=LMK6CE10000CDLER stages=16 extra_chips=100 sim=1.000ms wall=25.164s  => 3.974e-05x real time`

Both within the acceptance bounds (≤ 0.01x, ≤ 0.001x).

## Validation Plan

- Tests: no new ctest in A (behavior is unchanged; `trace_check`,
  `switch_test`, `display7_test`, `circuit_check` are the coverage). The
  timing test that changes behavior is plan-05-B's `engine_test`.
- Coverage check: every round goes through the new code — there is no other
  path (`grep -n process_round backend/src/engine/sim.c` → the definition
  and the one call in `sim_run_until`).
- Runtime proof: Phase 3 Check 2 (bench).
- Doc sync: `docs/backend.md` § Tests — add `sim_bench` (what it builds,
  the Release build line, that it is not a ctest).
- Final gate: runs once, at the end of plan-05-C (see there). A's last
  phase check is its own exit.

## Decisions

- Real-time speed is the target; plan-05 lands before plan-04 — owner,
  2026-11-01. (Order: plan-04-A gains a prerequisite row "plan-05 complete".)
- Byte-identical traces are A's gate; the timing fix is separate (B) so a
  golden diff in A always means a bug — author, 2026-11-01.

## Corrections

## Summary

The risk is in the stale-event check and the dirty set: miss one and
behavior changes, which `trace_check` catches across 3817 parts. Models,
headers, the circuit API and the goldens are untouched.
