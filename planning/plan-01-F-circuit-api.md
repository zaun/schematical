# plan-01-F: `circuit.h` — the simulation API as C calls

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-E

Replaces `netlist_sim.h` (text netlist in, text snapshot out) with
`circuit.h`: build a circuit from library parts with function calls, run it,
and read nets, lights and memory back as values. Ports the netlist-level
tests and `basicsim` onto the new API.

Outcome: `circuit_check` reproduces every scenario `netlist_check.c`,
`switch_check.c:check_netlist` and `sram_check.c:check_netlist_inspect` test
today, through `circuit.h`; `basicsim` passes on the new library.

References:

- `planning/plan-01-A-parts-baseline.md` §3.4 (the `circuit.h` API, verbatim).
- `planning/plan-01-B-library-core.md` §3.3 (hooks).
- Old code: `backend/netlist_sim.c`, `netlist_sim.h`, `netlist_check.c`,
  `switch_check.c:278-326`, `sram_check.c:443-480`, `basicsim/main.c`.

## Prerequisites

See plan-01-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-E complete | `ls planning/completed/plan-01-E-sram-7seg.md` → exists | MET (2026-10-31: exists, af14eb6) |

## 1. Goal

- `backend/include/circuit.h` + `backend/src/circuit.c` implement plan-01-A
  §3.4 exactly.
- `tests/circuit_check.c` passes; `basicsim` builds against `logic` and exits 0
  with the same four checkpoint lines it prints today.

### Non-goals

- The app keeps using `netlist_sim` until plan-01-G.
- No engine changes beyond what B added.

## 2. Current State (read)

- `netlist_sim.c:46-79` resolves a part name by trying LED, supply,
  oscillator, then every chip definition; `netlist_sim.c:23-38` keeps a union
  of four config types and an `is_led` array.
- Snapshot (`netlist_sim.h:21-30`): `time`, `net`, `lit` (LED: anode 1 and
  cathode 0 now), `glow` (7-segment: on at any time since `step_start`),
  `shorted`, `error`.
- `basicsim/main.c`: an engine-level demo — a custom 2-pin source drives an
  `SN74LS08N` and an `SN74LS00N`; four checkpoints; today it prints
  `10 ns: AND = X, NAND = Z` … `55 ns: AND = 0, NAND = 1` and exits 0 (run
  2026-10-31 against `/tmp/plancat/bs/basicsim`).

## 3. Design

### 3.1 `circuit.c`

- `Circuit` holds the library pointer, a `Sim`, the parts (`const Part *`,
  `SimInstance *`), nets, `step_start`, and an error buffer.
- `circuit_add_part`: `part_library_find`; unknown → `-1`, error
  `unknown part "<name>"`. The instance is created at once with the part's
  kind model (`kind_model(part)`, an internal accessor in `kind.h` reading
  `Part.internal`) and `config = part`.
- `circuit_connect`: pin number 1..`pin_count`; a pin with role `absent` is
  rejected (`pin 10 of LTC-4727JR has no lead`); after `circuit_start` all
  building calls fail with `circuit already started`.
- `circuit_set_switch`: the `set_switch` hook (any time); parts without it →
  `-1`, `part 3 is not a switch`.
- `circuit_advance`: sets `step_start = now`, runs `sim_run_until`; returns 0
  once `sim_error` is set (error text copied to the circuit's error).
- `circuit_net_value`: engine value mapped to `CircuitValue`.
- `circuit_shorted_net`: `sim_shorted_net` → index.
- `circuit_part_lights`: the `lights` hook with `since = step_start`; 0 for
  parts without it. A plain LED now reports through this (on at any time
  during the step), replacing the snapshot's `lit`.
- `circuit_memory_size`/`_read`: the memory hooks; 0 for other parts.

### 3.2 Tests — `tests/circuit_check.c`

Each old scenario, rebuilt with `circuit.h` and parts loaded from `PARTS_DIR`:

| Old test | New case |
|---|---|
| `netlist_check.c:check_lit_led` | VCC→anode, GND→cathode: light 1 after one advance |
| `check_dark_led` | reversed: light 0 |
| `check_short` | VCC and GND on one net: advance returns 0, error starts `smoke:`, shorted net reported |
| `check_unknown_part` | `circuit_add_part(c, "NOPE", …)` → -1 with message |
| `check_oscillator_blinks` | oscillator → LED: light toggles across steps |
| `switch_check.c:check_netlist` | VCC → switch position → net: 1 when ON, Z when OFF, after `circuit_set_switch` |
| `sram_check.c:check_netlist_inspect` | SRAM words/bits and `circuit_memory_read` of an unwritten word → known = 0 |
| new | connect to an absent pin → -1; add part after start → -1 |

### 3.3 `basicsim`

`basicsim/main.c` keeps its custom source (engine-level, as today) but gets
the two gates from the library: read `gate.json` (path from a compile
definition `PARTS_DIR`), `load_parts`, `part_library_find("SN74LS08N")`,
instance via `kind_model(part)` with `config = part`. `basicsim/CMakeLists.txt`
links `logic` (and adds `src/library` to its includes for `kind.h`).

### 3.4 Docs

`docs/backend.md` (new): the public API (`parts.h`, `circuit.h`) with a short
example (load files, build a circuit, advance, read nets/lights), and the
"Adding a kind" checklist from plan-01-A §3.5.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — `circuit.h` and its tests

- [x] `backend/include/circuit.h` (plan-01-A §3.4), `backend/src/circuit.c` (§3.1),
      `kind_model()` in `src/library/kind.h` (already there since plan-01-B,
      where `trace_check` needed it).
- [x] `tests/circuit_check.c` (§3.2) + CTest. Every row of §3.2, plus: the
      LED counts as lit for the step its switch opened in and dark the next
      (the new light rule, plan-01-A §3.6 item 2), pin past the end, missing
      net, net/connect after start, SRAM has no lights.
- [x] `docs/backend.md` (§3.4) (layout section written for the final
      plan-01-G layout; G re-checks it).

Acceptance: every scenario in §3.2 passes.
  Check: `ctest --test-dir /tmp/logic-build -R circuit_check --output-on-failure` → passed (est. 1 min).
  Result: `circuit_check` → 36 PASS, `0 failure(s)`; ctest passed.
Commit: —

### Phase 2 — basicsim

- [ ] `basicsim/main.c`, `basicsim/CMakeLists.txt` (§3.3).
- [ ] Move `planning/plan-01-F-circuit-api.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: same output, exit 0.
  Check: `cmake -S basicsim -B /tmp/basicsim && cmake --build /tmp/basicsim -j8 && /tmp/basicsim/basicsim` → the four lines in §2, exit 0 (est. 1 min).
Commit: —

## Validation Plan

- Tests: `circuit_check` (all old netlist-level scenarios + two new error cases).
- Coverage check: the §3.2 table maps every old netlist-level test function
  to a case.
- Runtime proof: `basicsim` output identical to today's.
- Doc sync: `docs/backend.md`.
- Final gate: `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → all pass (est. 3 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

- plan-01-E's last commit (archive + Phase 2) is af14eb6.
- §3.2 switch row: with the new light rule, the old "turning the switch OFF
  puts the LED out" after one step does not hold: the contact opens at the
  first instant of the next step, so the LED was on during that step and
  reads lit; it reads dark one step later. `circuit_check` pins both (the
  net itself reads Z immediately, as before).

## Summary

Small, well-bounded API layer over the engine and library; the only behavior
change is the LED light rule (plan-01-A §3.6, item 2), and `circuit_check`
pins the new rule.
