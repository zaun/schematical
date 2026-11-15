# plan-40-F: Backend — the `transistor` kind, gate-controlled channels in the engine, the rating test

Last updated: 2026-11-14
Effort: large (3h–1d)
Depends on: plan-40-E (F reads no census output; it follows E because letter
order is implementation order)

F adds the `transistor` part kind. A transistor part is one or more
**elements** (a BJT or a MOSFET each); an element's base or gate pin
controls whether its collector–emitter or drain–source pins are joined. The
join is the engine's existing **contact** (`sim_set_contact`, the switch
mechanism), opened and closed by the model from a timer 1 ns after the
control pin changes, so a conducting transistor passes strong values both
ways and no zero-time loop is possible. The voltage check also refuses a net
above the part's rating. The behavioral outcome: `transistor_test` passes
the engine and kind cases of § 6, `ctest` passes T0 + 1 programs, and every
existing golden is unchanged.

References:

- `backend/src/engine/sim.h` (`SimComponentType` `start`, `pin_changed`,
  `timer` hooks; `sim_set_contact`, `sim_schedule_timer`),
  `backend/src/engine/sim.c` (`sim_start`: `collect_links`, `resolve_nets`,
  then each instance's `start`; `process_round`'s `EVENT_CONTACT` branch).
- `backend/src/models/switch.c` (`switch_set`: the contact precedent, `joined`
  pairs always closed), `backend/src/models/capacitor.c` (the rating-only
  kind F mirrors for its facts and rating), `backend/src/models/diode.c`
  (tied same-name pins).
- `backend/include/parts.h` (`PartKind`, `PartResistor`, `Part`),
  `backend/src/voltage.c` (the capacitor rating branch, `voltage_describe`),
  `backend/include/voltage.h` (`VoltageProblemKind`).
- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Kinds (`diode`,
  `capacitor`, `resistor`).
- `planning/completed/plan-34-E-resistor-kind.md` and
  `planning/completed/plan-33-E-capacitor-kind.md` (the kind precedents).

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-A § Prerequisites MET | plan-40-A's table | MET (2026-11-15: every row MET, quota 999 before A Phase 1) |
| ~~plan-40-E complete~~ | `ls planning/completed/plan-40-E-*.md` → exists | moot (2026-11-15): an ordering edge, not a dependency — F reads no census output and touches no file C/D/E touch (§ Corrections 1) |
| No `transistor` kind yet | `grep -c 'PART_TRANSISTOR' backend/include/parts.h` → 0 | MET (2026-11-15 at 54a82b99, before F: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p40` → `100% tests passed` with T0 + 1
  programs (T0 = 35 at 943924ba: `git show HEAD:backend/CMakeLists.txt | grep -c '^logic_test'` → 35).
- `transistor_test` passes the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` are
  byte-identical (no part uses the kind yet; G adds them).

### Non-goals (explicit constraints)

- No analog behaviour: no gain, no threshold voltage, no Rds(on), no body
  diode, no switching speed beyond the fixed 1 ns. A transistor is a
  digital switch: base/gate reading 1 (NPN, N-channel) or 0 (PNP,
  P-channel) joins the channel; any other reading (0, X, Z) leaves it open.
- A conducting channel is a strong, two-way contact: a strong 1 on one end
  and a strong 0 on the other is a short (`smoke:`), as for a closed switch.
  Weak links (resistors, pulls) are not changed.
- Existing kinds are unchanged: every circuit without a transistor resolves
  exactly as before (the goldens are the check). The engine gets no new
  hook and no new function.
- The voltage check does not carry a level through a channel (its state is
  dynamic; `voltage_check` takes only switch contacts as closed contacts).
- No part-editor form for the `transistor` kind (plan-40-H § Decisions).

## 2. Current State

- The engine has contacts: `sim_set_contact(sim, instance, pin_a, pin_b,
  closed)` joins two pins' nets while closed (`sim.h` comment above it). Before
  `sim_start` it applies at once; after, it appends an `EVENT_CONTACT` at
  `sim->now` (`sim.c:sim_set_contact`), which `process_round` applies,
  marks every net dirty for and rebuilds the groups. `switch.c:switch_set`
  and `connector.c` call it from the app-driven `set_switch` hook, and
  `display7.c:display_start` calls it with `closed` = 1 from the model's
  `start` hook (`display7.c:471`), the precedent for ties made when a part
  starts.
- `sim_start` runs `collect_links`, then `resolve_nets`, then every
  instance's `start` hook (`sim.c:sim_start`), so a model's `start` can read
  its resolved pins and schedule a timer.
- A model's `timer` hook fires at `now + delay` (`sim_schedule_timer`); the
  hook may call `sim_set_contact`.
- `PartKind` ends with `PART_RESISTOR` (`backend/include/parts.h:29`);
  `TRACE_LAST_KIND` is `PART_RESISTOR` (`tests/trace_check.c:22`);
  `listing_check.c:50–51` size two arrays by `PART_RESISTOR + 1`;
  `tests/support/listing.c` prints every kind's facts (`case PART_RESISTOR`
  :37, :122).
- `voltage.c` has one rating branch per kind in the per-pin loop (switch
  contacts, capacitor); `VOLTAGE_CAPACITOR_OVER` is the last
  `VoltageProblemKind`.
- Pins that share a name on one part are tied by the diode and resistor
  kinds (`diode_pair`, `resistor_pair`), for a tab and a lead.

### Measured populations

| What | Count | Command |
|---|---|---|
| Existing part files (one per kind) | 30 | `ls schematical/schematical/Parts/*.json \| grep -vc 'footprints\|datasheets'` → 30 |
| Test programs T0 | 35 | `git show HEAD:backend/CMakeLists.txt \| grep -c '^logic_test'` → 35 |
| Golden sizes P0 / R0 / F0 | 31608 / 31608 / 1497 | `git show HEAD:backend/tests/golden/{parts,traces,footprints}.txt \| wc -l` |
| Call sites of `sim_set_contact` outside the engine | 4 in 3 models: `display7.c:471` (ties at start), `switch.c:234,236`, `connector.c:146` | `grep -rn 'sim_set_contact' backend/src \| grep -v 'engine/sim' \| grep -v '^backend/src/models/switch.c:4:'` → 4 lines |

### Verified properties

- **A model may start timers and set contacts from them.** `switch_set`
  calls `sim_set_contact` from an app-driven hook while the sim runs; the
  contact event is queued at `sim->now` and applied by the next
  `process_round`. A timer hook has the same `(Sim *, SimInstance *, state)`
  arguments (`sim.h`), so it can queue the same event.
- **A channel change is never made in the round that read the pin.** The
  model queues it from a timer 1 ns after the read (`sim_schedule_timer`, then
  the `timer` hook calls `sim_set_contact`), so it is a later event than the
  `pin_changed` that caused it; a gate tied to its own drain therefore toggles
  every 1 ns instead of re-queuing at one time. Test case (n) of § 6 is the
  check (it must return from `circuit_advance`).
- **Contacts need both pins on nets.** `sim.c` leaves a contact whose pin
  has no net out (`sim.h` comment above `sim_set_contact`); the tab of a
  leaded package is tied by a permanently closed contact and so joins its
  lead only when both pins are wired.

## 3. Design

### 3.1 The kind

`backend/src/models/transistor.c`, `TRANSISTOR_KIND` (`"transistor"`),
file `Parts/transistor.json`.

Device keys:

| Key | Value |
|---|---|
| `"elements"` | non-empty array, one object per transistor in the package. Each has `"type"`: `"npn"`, `"pnp"`, `"nmos"` or `"pmos"`; for `npn`/`pnp` the pin names `"base"`, `"collector"`, `"emitter"`; for `nmos`/`pmos` `"gate"`, `"drain"`, `"source"` (a key of the other group is an error: `"gate" is not a bjt key`); optional `"darlington": true` (npn/pnp only) |

Every name names at least one pin of every pinout; a name that several pins
carry (a tab and a lead) ties those pins. The control pin (base, gate) must
carry exactly one pin and be `input` (`transistor pin "B" must be input`); the
channel pins are `inout` (`… must be inout`); other pins are `nc` or
`absent`. Every pin of the pinout is in some element or is `nc`/`absent`
(`pin "X" is in no element`).

Part keys (numbers; the group is the device's first element's type, and an
array's facts hold for every element):

| Key | Types | Value |
|---|---|---|
| `"vceo"` | npn, pnp, optional | max collector–emitter voltage, V, above 0 and at most 10000 (the generic parts give none) |
| `"ic"` | npn, pnp, optional | max collector current, A, above 0 and at most 1000 |
| `"pd"` | all, optional | max power, W, above 0 and at most 10000 |
| `"hfe"` | npn, pnp, optional | minimum DC gain, above 0 and at most 100000 |
| `"r1"`, `"r2"` | npn, pnp, optional | built-in base series / base–emitter resistor, ohms, at least 0 and at most 1e9 (pre-biased parts) |
| `"vds"` | nmos, pmos, optional | max drain–source voltage, V, as `vceo` |
| `"id"` | nmos, pmos, optional | max continuous drain current, A, as `ic` |
| `"rdson"` | nmos, pmos, optional | max Rds(on), ohms, above 0 and at most 1e6 |
| `"vgsth"` | nmos, pmos, optional | max gate threshold, V, above 0 and at most 100 |
| `"vgs"` | nmos, pmos, optional | max gate–source voltage, V, above 0 and at most 1000 |
| `"logicLevel"` | nmos, pmos, optional | `true` when the datasheet specifies Rds(on) at a gate drive of 4.5 V or less |

A key of the other group is an error (`"vds" is only for a mosfet`).

Facts: `typedef struct { const char *type; size_t element_count; uint8_t
mixed; uint8_t darlington; uint8_t mosfet; uint8_t logic_level; uint32_t
vmax_mv, imax_ma, power_mw, hfe, vgsth_mv, vgs_mv; uint64_t r1_mohm,
r2_mohm, rdson_mohm; } PartTransistor;` (0 = not given), `const
PartTransistor *transistor;` on `Part` after `resistor`. `type` is the first
element's type; `mixed` is 1 when the elements' types differ (a
complementary pair); `darlington` is 1 when any element is one.
`model_pins.h`: `transistor_element(part, k, &type, &control, &a, &b)` (pin
indices into `Part.pins`; 0 past the last element) and
`transistor_pin_limit_mv(part, pin)` (the rating the pin is checked against:
`vmax_mv` for a channel pin, `vgs_mv` for a gate pin, 0 for a base).

### 3.2 The model

`SimComponentType TRANSISTOR_MODEL`: `pin_count` = `part->pin_count`;
`create` returns a state with one `on` flag per element (all 0);
`start`: for each element, closes the contacts of its tied pin pairs (tab
and lead) and schedules timer `k` after `TRANSISTOR_DELAY_PS` = 1000 (1 ns);
`pin_changed`: when the pin is an element's control, schedules timer `k`
after 1 ns; `timer(k)`: reads the control pin, `want` = (npn/nmos: value is
1; pnp/pmos: value is 0), and when `want` differs from the element's `on`
flag calls `sim_set_contact(channel a, channel b, want)` and stores it. The
timer re-reads the pin, so a pulse shorter than 1 ns never moves the
channel. X and Z read as not wanted.

### 3.3 The rating test

`voltage.c`: a branch after the capacitor's. For a `PART_TRANSISTOR` part
and a pin with `transistor_pin_limit_mv(part, pin) > 0`: when the pin's net
is above the limit, `VOLTAGE_TRANSISTOR_OVER` (appended to
`VoltageProblemKind`, `max_mv` = the limit). `voltage_describe` prints it
with the switch and capacitor message (`voltage: 12 V on pin 3 (D) of
TESTFET; it is rated 5 V`). A BJT base has no limit; a part whose limit is
0 is never refused.

### 3.4 Where the risk is

Risk concentrates in the model's timing and in the rating branch's pin
classification. The engine is unchanged, so the goldens are unchanged;
`transistor_test` is the check, including a ring (gate tied to drain, which
toggles each ns), a short (strong 1 and 0 across a conducting channel),
and every high-side/low-side case.

Rejected alternatives: (a) a new `transistors` engine hook with weak
links — rejected: a conducting channel must pass strong values (an emitter
follower, a high-side PMOS) and the contact already does; (b) switching in
`pin_changed` with no delay — rejected: a diode-connected transistor would
re-queue contact events at one time forever; (c) comparing base to emitter
(`B = 1 and E = 0`) — rejected: an emitter follower would oscillate against
itself; (d) carrying levels through the channel in `voltage_check` —
rejected above (non-goal).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind: keys, loader, facts, listing

- [x] `backend/include/parts.h`: `PART_TRANSISTOR` appended as the last
      `PartKind` value; `PartTransistor` (§ 3.1) and `const PartTransistor
      *transistor;` after `resistor` on `Part`.
- [x] `backend/src/models/transistor.c`: `DEVICE_KEYS` (`elements`),
      `PART_KEYS` (§ 3.1), `transistor_load_device` (validates elements),
      `transistor_bind_pinout` (resolves names to pins, checks roles and
      that every pin is in an element or `nc`/`absent`),
      `transistor_load_part` (reads and range-checks the numbers, builds
      `PartTransistor`), `transistor_element`, `transistor_pin_limit_mv`
      (declared in `model_pins.h`); `TRANSISTOR_KIND` with a passive model
      for now (`pin_count`, `create`, no callbacks).
- [x] `backend/src/library/kinds.c`: `extern const PartKindOps
      TRANSISTOR_KIND;` and one `&TRANSISTOR_KIND,` line in `KINDS[]`;
      `backend/CMakeLists.txt`: `src/models/transistor.c` in the sources.
- [x] Tests: `tests/support/listing.c` (`case PART_TRANSISTOR`: the kind name
      `transistor` at :37 and the facts at :122, `elements=%zu type=%s mixed=%u
      vmax=%u imax=%u power=%u hfe=%u rdson=%llu vgsth=%u vgs=%u
      logic=%u r1=%llu r2=%llu`), `tests/trace_check.c` `TRACE_LAST_KIND` →
      `PART_TRANSISTOR`, `tests/listing_check.c` :50–51 → `PART_TRANSISTOR + 1`.
- [x] `tests/support/transistor_fixture.h` (as `resistor_fixture.h`): fixture
      parts `TFX-Q1` (npn, 3 pins E B C, vceo 40, ic 0.2), `TFX-Q2` (pnp),
      `TFX-M1` (nmos, G D S, vds 60, id 0.3, rdson 5, vgsth 2.1, vgs 20,
      logicLevel false), `TFX-M2` (pmos), `TFX-DT` (nmos with a tab: pins
      G D S D, the tab and drain tied), `TFX-PB` (npn with r1 10000, r2
      47000), `TFX-ARR` (npn + pnp complementary pair, 6 pins), a gate
      fixture for the inverter cases, and `TRANSISTOR_DEVICE` for the
      rejection cases.
- [x] `tests/transistor_test.c` kind cases (§ 6 a–f) and
      `logic_test(transistor_test)` in `backend/CMakeLists.txt` after
      `resistor_test`.

Acceptance: the kind loads, lists and rejects what § 6 lists.
  Check: `cmake -S backend -B /tmp/logic-p40 && cmake --build /tmp/logic-p40 -j8 && ctest --test-dir /tmp/logic-p40 -R 'transistor_test|listing_check|trace_check|library_check|footprint_check'` → `100% tests passed, 0 tests failed out of 5` (est. 8 min: first build of the tree).
  Result (2026-11-15, build dir `/tmp/logic-p40w`): `100% tests passed, 0 tests failed out of 5`; `transistor_test` 42 PASS lines, `0 failure(s)`. Mutation check: every element made to conduct on 1 (`on_level = 1`) fails (i) twice and (q).
Commit: 5f3c9a25

### Phase 2 — The model: channel from the control pin

- [x] `transistor.c`: the model of § 3.2 (`start`, `pin_changed`, `timer`;
      `TRANSISTOR_DELAY_PS` 1000); the model's state is the per-element `on`
      flags; the tab ties are closed contacts made in `start`.
- [x] `tests/transistor_test.c` engine cases (§ 6 g–q).

Acceptance: every engine case of § 6 passes and no existing golden moves.
  Check: `cmake --build /tmp/logic-p40 -j8 && ctest --test-dir /tmp/logic-p40` → `100% tests passed` with T0 + 1 programs, then `git diff --stat backend/tests/golden` → empty (est. 4 min: the whole suite is what shows "no golden moved"; nothing smaller covers every kind).
  Result (2026-11-15): `100% tests passed, 0 tests failed out of 36` (T0 35 + 1); `git diff --stat -- backend/tests/golden` → empty.
Commit: 5f3c9a25

### Phase 3 — The voltage rating test

- [x] `backend/include/voltage.h`: `VOLTAGE_TRANSISTOR_OVER` appended;
      `voltage.c`: the branch of § 3.3 and the `voltage_describe` case (shared
      with `VOLTAGE_SWITCH_OVER` and `VOLTAGE_CAPACITOR_OVER`).
- [x] `tests/transistor_test.c` rating cases (§ 6 r–u).
- [x] Docs: `docs/parts-format.md` — `transistor.json` in the kinds table
      (:9–29 area) and a `### transistor` section after `resistor` (keys,
      rules, the digital-switch semantics, the rating test);
      `docs/backend.md` — a **Transistors** paragraph after Resistors (the
      contact rule, the 1 ns delay, the table of § 6's circuits) and the
      voltage paragraph's list (`transistor rating`); the Layout block
      (`transistor.c`); `docs/backend.md` § Adding a kind needs no change.

Acceptance: the rating cases pass and the docs name the kind.
  Check: `cmake --build /tmp/logic-p40 -j8 && ctest --test-dir /tmp/logic-p40 -R 'transistor_test|voltage'` → `100% tests passed`; `grep -c 'transistor' docs/parts-format.md docs/backend.md` → both above 0 (est. 3 min).
  Result (2026-11-15): `100% tests passed, 0 tests failed out of 2`; `docs/backend.md:7`, `docs/parts-format.md:10`. The app still builds (`** BUILD SUCCEEDED **`).
Commit: 5f3c9a25

## 6. Test cases (`transistor_test.c`)

Kind: (a) the six fixture parts load and list; (b) an unknown `type`; (c) a
`bjt` key on a MOSFET and the reverse; (d) a base on two pins; (e) a pin in
no element; (f) `"vds"` on a BJT, `"vceo"` 0, `"darlington"` on a MOSFET, a generic
fixture with no rating (loads, never refused).

Engine (VCC 5 V, GND, a pull-up or a gate as the source of each level; each
case runs 5 ns): (g) NPN, base driven 1 by a gate, collector pulled up,
emitter GND → collector 0 after 1 ns; base 0 → collector 1 (the pull); (h)
the same with an N-channel MOSFET; (i) PNP and P-channel high side: source
on VCC, gate 0 → drain 1, gate 1 → drain Z; (j) base X or Z (floating) →
channel open; (k) a pulse of 0.5 ns on the base → channel never moves
(inertial); (l) strong 1 on the collector and strong 0 on the emitter with
the base 1 → `smoke:`; with the base 0 → no short; (m) emitter follower:
collector VCC, base 1, emitter with a pull-down → emitter 1; (n) a
diode-connected NMOS (gate tied to drain, drain pulled up, source GND):
the net toggles each ns and `circuit_advance` returns; (o) an inverter
(NPN + resistor pull-up) driving an LED lights it with the input 1; (p) a
tab-tied part: driving the tab reaches the lead; (q) a complementary array
(NPN + PNP): each element switches on its own base.

Rating (supply 12 V via `circuit_voltage_problem`): (r) a MOSFET with `vds`
5 on a 12 V net on its drain → `VOLTAGE_TRANSISTOR_OVER`, text `voltage: 12 V
on pin 3 (D) of TFX-M1; it is rated 5 V`; (s) a BJT base on 12 V is not
refused; (t) a gate above `vgs` is refused; (u) a part under its rating is
accepted.

## Validation Plan

- Tests: `transistor_test` (new); `library_check`, `listing_check`,
  `trace_check`, `footprint_check` and every other program unchanged.
- Coverage check: Phase 2's check compares the goldens; the whole suite
  passing with T0 + 1 programs is the denominator.
- Runtime proof: the app harness of plan-40-H runs a transistor in the real
  simulation controller.
- Doc sync: `docs/parts-format.md`, `docs/backend.md` (Phase 3).
- Final gate: plan-40-I.

## Decisions

- **A digital switch driven by the base/gate level alone** — author: the
  logic simulator has no analog state; a low-side switch, high-side
  switch, emitter follower and inverter all work from the level.
- **A contact, not a weak link** — author: a conducting channel must pass
  strong values; a switch contact already does and has a short rule.
- **1 ns switching delay, re-read at the timer** — author: breaks
  zero-time loops and filters glitches.
- **Rating: channel pins against `vceo`/`vds`, gate against `vgs`, base
  unchecked** — author: DigiKey gives `Vce`/`Vds` and `Vgs` for every part
  and no base limit.
- Owner decisions: plan-40-A § Decisions.

## Corrections

1. **F ran before C, D and E (2026-11-15).** The A → … → I chain is
   implementation order; F's own Depends-on line says it reads no census
   output, and F touches only `backend/` and `docs/` (C–E touch
   `footprints.json` and `planning/plan-40-data/`). F was done while A's
   DigiKey listing and downloads ran in the background. The dependency
   graph's real edges for F: none inside plan-40; G needs F.
2. **Phases 1–3 landed in one commit.** The model (Phase 2) and the rating
   branch (Phase 3) were written with the kind, so there was no
   passive-model state to commit; each phase's own check was run and is
   recorded on its Result line.
3. **The rating fixture.** § 6 (r) names `TFX-M1` with `vds` 5 and the drain
   on pin 3, but Phase 1 gives `TFX-M1` `vds` 60 on pins G D S (drain pin
   2). The rating cases use a ninth fixture part, `TFX-M5` (pins 1 G, 2 S,
   3 D; `vds` 5, `vgs` 8), and the text is `voltage: 12 V on pin 3 (D) of
   TFX-M5; it is rated 5 V`; `TFX-M1` (60 V) is the accepted case (u). `TFX-Q2`
   gets `vceo` 5 so case (s) (a base on 12 V is not refused) is not
   trivially true, and `TFX-GEN` is the generic fixture with no rating.
4. **No gate fixture.** The inverter case (o) uses the library's
   `SN74HC04N` as the gate (as `resistor_test` does) instead of a fixture
   gate; the engine cases use a scripted driver.
5. **Channels and ties are every pin pair.** A channel name on several pins
   (a tab and its lead) gives a contact from each of its pins to each of the
   other name's pins, and same-named pins are tied pairwise, because the
   engine leaves out a contact whose pin has no net (§ 2): a part wired by
   its tab alone still switches (case (p), second half).
6. **Extra checks.** `transistor_test` also refuses a base that is not
   `input`, a collector that is not `inout`, `"vceo"` on a MOSFET, `"ic"`
   2000 and a device with no `"elements"`.

## Summary

F is the plan's design risk: the model's timing and the pin classification
of the rating test. The engine and every existing kind are untouched. Left
for later letters: the parts (G) and the app (H).
