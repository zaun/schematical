# plan-58-B: The `switch` kind learns slide-switch states

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-58-A (gate only: B uses inline fixtures, but the letters run A → B → C …)

A slide switch has one latching knob with two or three positions, and each
position closes some of its contacts: an SPDT On-On switch closes common–A in
position 1 and common–B in position 2; an On-Off-On switch has a third, middle
position that closes nothing. The `switch` kind today models independent
contacts (DIP) and spring-back contacts (tactile). B adds two optional device
keys, `"states"` and `"rest"`, and a circuit call that moves the knob. No slide
part is added yet (C–F); B proves the keys with inline fixtures.

Outcome: a fixture SPDT On-On switch in a circuit starts with common–A joined
and common–B open; `circuit_set_switch_state(…, 2)` joins common–B and opens
common–A; a fixture DPDT On-Off-On switch starts in its middle position (all
four contacts open), `…(1)` closes contacts 1 and 3, `…(3)` closes 2 and 4; a DIP
switch and a tactile switch behave byte for byte as before (every existing
golden line unchanged); `ctest` runs T0 + 1 = 62 programs (`slide_test`).

References:

- `docs/parts-format.md` § `switch`, § Parts; `docs/backend.md` (voltage check,
  `circuit_set_switch`, tests and goldens).
- `backend/src/models/switch.c` (`DEVICE_KEYS`, `switch_load_device`,
  `switch_bind_pinout`, `switch_contact_pins`, `switch_set`),
  `backend/src/circuit.c` (`circuit_add_part` line 172, `circuit_set_switch`
  line 320, `circuit_start` line 421), `backend/include/circuit.h:48`,
  `backend/include/parts.h` (`PartSwitch`), `backend/src/voltage.c:317`
  (`switch_contact_pins` use), `backend/tests/support/listing.c:302`,
  `backend/tests/tactile_test.c` (the model for `slide_test.c`).
- `planning/completed/plan-27-D-tactile-kind.md` (the same kind of change).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-A is complete | `ls planning/completed/plan-58-A-*` → one file | MET (2026-11-24, P-58: A archived in the worktree, commit df339161) |
| `PartSwitch` is still the plan-27 shape | `grep -c 'unsigned momentary; unsigned normally_closed; unsigned joined;' backend/include/parts.h` → 1 | MET (2026-11-24; re-run at P-58 start → 1) |
| The kind has no slide keys yet | `grep -c '"states"' backend/src/models/switch.c` → 0 | MET (2026-11-24; re-run at P-58 start → 0) |

If plan-58-A is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The `switch` device accepts `"states"` (array of arrays of contact numbers)
  and `"rest"` (state number), with the loader errors of § 4.1.
- `PartSwitch` gains `states` (count, 0 for a switch that has none), `rest`
  and `state_mask[3]`; `part_switch_state_contacts` reads a state's contacts.
- `circuit_add_part` records a slide switch's rest-state contacts as closed;
  `circuit_start` re-applies them; `circuit_set_switch_state` moves the knob.
- The voltage check sees the same closed contacts the simulation has.
- `ctest` runs 62 programs; `backend/tests/golden/{parts,traces,footprints}.txt`
  are byte-identical.

### Non-goals (explicit constraints)

- DIP and tactile switches: JSON, golden lines, behavior unchanged
  (`parts.txt`, `traces.txt` byte-identical; `slide_test` is the only new test program).
- No new kind, enum or model (`PART_SWITCH`, `SWITCH_MODEL` stay).
- No change to the engine (`backend/src/engine/`).
- `circuit_set_switch` keeps addressing single contacts, for every switch; the
  MCP / agent `set_switch` tool is the app's concern (plan-58-G).
- Contact timing (DigiKey's "Shorting (MBB)" / "Non-Shorting (BBM)") is not
  modelled beyond the order below: opening before closing.

## 2. Current State

- `switch.c` loads `actuator`, `contacts` (each `[pin, pin]`), `ratedVolts`,
  `momentary`, `normallyClosed`, `joined`, `bodySize`; `switch_set` calls
  `sim_set_contact` for position `p` (1-based) and re-closes the joined pairs.
- `switch_bind_pinout` binds each contact's two pin names with `kind_pin` per
  contact. `kind_pin` (`kind_util.c:370`) refuses a pin *name* that two pins of
  the pinout share; it does not refuse a name used by two contacts, so a common
  terminal can appear in several contacts (`["C","A"]`, `["C","B"]`).
- The engine keys a contact by the pair `(pin_a, pin_b)` (`sim.c:629`,
  `find_contact`), so two contacts that share a pin are two independent contacts.
- `circuit.c:172` records a normally-closed switch's positions as closed when the
  part is added; `circuit_start` (`circuit.c:421`) re-applies recorded contacts
  for normally-closed and joined switches, because pins have nets only after
  building. `circuit_set_switch` (`circuit.c:320`) calls the model hook, records
  the contact and runs the voltage check when a contact closes during a run.
- Golden: `grep -c '|switch|' backend/tests/golden/parts.txt` → 2070, each
  `positions=N actuator=… rated=…` (plus `momentary nc joined= body=` for tactile).

### Measured populations

| What | Count | Command |
|---|---|---|
| Switch parts / devices today | 2070 / 186 | `grep -c '\|switch\|' backend/tests/golden/parts.txt`; `grep -c '"id"' schematical/schematical/Parts/switch.json` |
| Places that read `PartSwitch` fields | 28 lines | `grep -rn 'dip_switch' backend/src backend/tests schematical/schematical --include='*.c' --include='*.swift' --include='*.h' \| wc -l` → 28 |
| Places that set `PartSwitch.positions` | 1 (line 128; line 171 reads it) | `grep -n 'info.positions' backend/src/models/switch.c` → `switch.c:128` and `:171` (the second reads it) |
| ctest programs T0 | 61 | `grep -c '^logic_test' backend/CMakeLists.txt` |

### Verified properties

- A pin may be in several contacts: read `kind_pin` (it counts pins with the
  name in the pinout, not uses) and `apply_contact` / `find_contact` (the
  contact is the ordered pin pair).
- `voltage.c:317` takes any position that `switch_contact_pins` accepts and
  joins the nets of every contact recorded closed, so a slide switch's
  contacts (numbered `1 … positions`) reach the voltage check unchanged.
- `trace_check` drives a switch at engine level through the model's
  `set_switch` (`trace_check.c:76`), never through `Circuit`; new defaults in
  `circuit.c` do not affect it.

## 3. Design Overview

A slide switch is a `switch` device with a table of states. Contacts stay the
backend's primitive: `contacts` numbers them (1-based, as now); each entry of
`"states"` lists the contact numbers closed in that knob position, and `"rest"`
picks the position a new circuit starts in. The backend keeps no knob state of
its own beyond the recorded contacts (the same rule as `momentary`: the contact
state is `circuit_set_switch`'s). `circuit_set_switch_state` is the one new
call: it opens the contacts the state does not list and then closes the ones it
does (break before make), each through the existing hook and `record_contact`,
then runs the voltage check once, like `circuit_set_switch`.

Risk is small and sits in `circuit.c` start-up order (recorded contacts are
applied before nets exist). Byte-identity is the gate for the DIP and tactile
parts only (`parts.txt`, `traces.txt`, `footprints.txt` unchanged); the new
behavior is gated by `slide_test`.

Rejected: a separate `slide` kind (the contact model, the voltage check and the
`PartSwitch` readers are shared); storing the knob position in the backend
(the app already keeps per-contact booleans and the file's `switches` array
stays the saved form); making `circuit_set_switch` refuse a single contact on
a slide switch (breaks the low-level contract and `trace_check`).

## 4. Detailed Design

### 4.1 Device keys (`switch.c`, `DEVICE_KEYS`)

| Key | Value |
|---|---|
| `"states"` | optional: an array of 2 or 3 arrays; each is a (possibly empty) list of distinct contact numbers in `1 … positions`, the contacts closed in that knob position, position 1 first. Errors: `"states" must be 2 or 3 arrays of contact numbers`, `states[i]: contact N does not exist (the device has M contacts)`, `states[i]: contact N twice`, `"states" needs "rest" no later than state K`. |
| `"rest"` | optional, needs `"states"`: the knob position (1-based, at most the number of states) a switch starts in. Default 1. Error: `"rest" needs "states"`, `"rest" must be 1 … K`. |

A device with `"states"` is a **slide switch**: it may not also name
`"momentary"`, `"normallyClosed"` or `"joined"` (`"states" cannot be combined
with "momentary"` …). `PartSwitch` gains (`parts.h`):

```c
unsigned states;          /* knob positions; 0 for a switch with no "states" */
unsigned rest;            /* the starting position, 1-based; 0 when states == 0 */
uint8_t state_mask[3];    /* bit n-1 set: contact n is closed in that position */
```

and `int part_switch_state_contacts(const Part *part, unsigned state, uint32_t *mask)`
(returns 1 and fills `*mask` for a slide switch's `state` 1 … `states`, else 0).
Contacts are at most 8 (`positions <= 8` when `states > 0`; error otherwise).

### 4.2 Circuit (`circuit.c`, `circuit.h`)

- `circuit_add_part`: for a slide switch, `record_contact(position, 1)` for each
  contact of its rest state (and nothing else; the existing normally-closed and
  joined branches are untouched).
- `circuit_start`: the condition at `circuit.c:428` also holds for
  `states > 0`; the loop already re-applies `contact_recorded` for each position.
- `int circuit_set_switch_state(Circuit *circuit, size_t part, unsigned state)`:
  errors `"%s is not a slide switch"` and `"%s has no position %u (1 … %u)"`;
  otherwise: for each contact not in the state, `set_switch(…, 0)` + `record_contact`;
  then for each contact in it, `set_switch(…, 1)` + `record_contact`; then,
  when `circuit->started` and any contact closed and `!voltage_stopped`, the
  same `voltage_failed` check as `circuit_set_switch` (`circuit.c:320`).
  Declared in `circuit.h` next to `circuit_set_switch`, documented in `docs/backend.md`.

### 4.3 Pin and contact naming of the slide parts (used by C–G)

| Circuit | Pins | `contacts` | `states` (On-On) | `states` (On-Off-On) | `states` (On-Off) | `rest` |
|---|---|---|---|---|---|---|
| SPST | `A`, `B` | `[["A","B"]]` | — | — | `[[1],[]]` | 2 |
| SPDT | `C`, `A`, `B` | `[["C","A"],["C","B"]]` | `[[1],[2]]` | `[[1],[],[2]]` | — | 1 (On-On), 2 (On-Off-On) |
| DPST | `A1`, `B1`, `A2`, `B2` | `[["A1","B1"],["A2","B2"]]` | — | — | `[[1,2],[]]` | 2 |
| DPDT | `C1`, `A1`, `B1`, `C2`, `A2`, `B2` | `[["C1","A1"],["C1","B1"],["C2","A2"],["C2","B2"]]` | `[[1,3],[2,4]]` | `[[1,3],[],[2,4]]` | — | 1 (On-On), 2 (On-Off-On) |

Position 1 is the one the datasheet's circuit table or diagram lists first
(`A` side). A switch that has an On position and an Off position starts Off
(`rest` = the Off state), like every DIP position.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay with evidence; fill `Commit:` when
> the phase lands. An unticked box means NOT DONE.

### Phase 1 — loader, struct, docs

- [x] `backend/src/models/switch.c`: add `"states"` and `"rest"` to `DEVICE_KEYS`;
      read and validate them (§ 4.1) in `switch_load_device`; fill the new
      `PartSwitch` fields; bind nothing new (`switch_bind_pinout` already binds
      every contact).
- [x] `backend/include/parts.h`: the three fields and `part_switch_state_contacts`
      (defined in `switch.c`); update the `PartSwitch` comment.
- [x] `backend/tests/support/listing.c:302`: append ` states=<n> rest=<r>` to a
      switch's facts only when `states > 0` (existing lines unchanged).
- [x] `docs/parts-format.md` § `switch`: the two keys, the slide-switch rules,
      § 4.3's naming table, the drawer `["Switches & Relays","Slide"]`.
- [x] Tests: `backend/tests/slide_test.c` (new; Phase 1 part: `case_loader`, `case_errors`) loads the inline fixture
      (SPDT On-On `SLIDE-SPDT`, SPDT On-Off-On `SLIDE-SPDT3`, DPDT On-Off-On
      `SLIDE-DPDT3`, SPST On-Off `SLIDE-SPST`, one `ratedVolts` 3 part
      `SLIDE-LV`) and checks the loader: `part_switch_state_contacts` for every
      state, `rest`, and every error string of § 4.1 with a bad fixture; register it
      with `logic_test(slide_test)` in `backend/CMakeLists.txt`.

Acceptance: the fixture loads; each bad fixture fails with its message; the
golden files do not change.
  Check: `cmake -S backend -B /tmp/logic-p58 && cmake --build /tmp/logic-p58 -j8 --target slide_test listing_check && /tmp/logic-p58/slide_test && /tmp/logic-p58/listing_check --print | diff - backend/tests/golden/parts.txt` → every line `ok`, empty diff (est. 4 min). Measured: 22 `ok` lines and `PASS`, `slide_test` exit 0, empty diff of `listing_check --print` against `parts.txt`.
Commit: —

### Phase 2 — circuit behavior

- [ ] `backend/src/circuit.c`: the rest-state recording in `circuit_add_part`, the
      `circuit_start` condition, `circuit_set_switch_state` (§ 4.2);
      `backend/include/circuit.h` declaration.
- [ ] `slide_test.c`: SPDT On-On between a 5 V supply and two `GENERIC-LED`s:
      LED 1 lit at start, `circuit_set_switch_state(…, 2)` → LED 2 lit and LED 1
      dark; DPDT On-Off-On starts all-open, state 1 closes contacts 1 and 3,
      state 3 closes 2 and 4; SPST On-Off starts open, state 1 closes, state 2
      opens; an unwired common pin does not break start-up; a bad `state` (0, 4)
      and a non-slide part fail with the messages of § 4.2; `SLIDE-LV` (3 V) on
      a 5 V net: moving the knob that would join the 5 V net to its terminals
      is refused by the voltage check the way `tactile_test` checks it.
- [ ] `docs/backend.md`: `circuit_set_switch_state` in § Using it, the slide
      switch under the voltage check's list of closed contacts.

Acceptance: `slide_test` prints only `ok` lines; `tactile_test` and `switch_test`
still pass; the three golden files are unchanged.
  Check: `cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -R 'slide_test|tactile_test|switch_test|listing_check|trace_check' --output-on-failure` → `100% tests passed` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `slide_test` (loader errors, states, rest, start-up, state moves, voltage refusal).
- Coverage check: `grep -c 'slide_test' backend/CMakeLists.txt` → 1, and
  `ctest --test-dir /tmp/logic-p58 -N | tail -1` → `Total Tests: 62`.
- Runtime proof: the LED fixture of Phase 2 (the knob moves the 5 V between two LEDs).
- Doc sync: `docs/parts-format.md` § `switch`; `docs/backend.md`.
- Final gate (once, after Phase 2): `cmake -S backend -B /tmp/logic-p58 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8` → `100% tests passed, 0 tests failed out of 62` (est. 15 min).

## Decisions

- At most 3 knob positions and 8 contacts per slide switch — the author's call: the
  catalog's slide switches are 2- and 3-position (DigiKey functions On-On, On-Off,
  On-Off-On) with at most 4 contacts (DPDT). (§ 4.1)
- A switch with an Off position starts Off; an On-On switch starts in position 1 —
  the author's call, matching DIP and tactile switches starting open. (§ 4.3)
- Latching only (no spring-back slide); no panel-mount, no illuminated — owner and
  author, plan-58-A § Decisions.

## Corrections

- **2026-11-24, Phase 1: § 4.1 lists one error too many.** `"states" needs "rest" no later than state K` and `"rest" must be 1 … K` describe the same condition (`rest` above the number of states; `rest` is optional, default 1, so it can never be missing). The loader has the second only, and `slide_test` checks it for `rest` = 3 and `rest` = 0. The other five § 4.1 errors, the three `cannot be combined with` errors and `"rest" needs "states"` are each refused with their message by a bad fixture in `slide_test.c` (`case_errors`).

## Summary

The risk is `circuit.c` start-up order and keeping every DIP and tactile golden
line identical; both are tested. Untouched: the engine, the DIP and tactile
data, `circuit_set_switch`, the file format.
