# plan-27-D: The `switch` kind learns momentary, normally-closed and joined contacts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-27-C

A tactile switch needs three things the DIP-switch model lacks: it **springs
back** (the app presses and releases it), it can be **normally closed**
(NC contact closed until pressed), and a four-terminal one has pins that are
**joined** inside the part (both left terminals are one node, both right
terminals the other). D adds three optional device keys to the existing
`switch` kind and the backend behavior behind them. No tactile part is added
yet (E–I); D proves the keys with inline fixtures.

Outcome: a fixture 4-terminal NC tactile switch in a circuit conducts from
its left side to its right side at `circuit_start`, stops conducting after
`circuit_set_switch(position 1, off)`, keeps its two left pins one net
throughout, and a DIP switch behaves byte-for-byte as before (all 36 golden
switch lines unchanged).

References:

- `docs/parts-format.md` § `switch`, § Parts; `docs/backend.md` (voltage check,
  `circuit_set_switch`).
- `backend/src/models/switch.c`, `backend/src/circuit.c` (`record_contact`,
  `circuit_add_part`, `circuit_start`, `circuit_set_switch`),
  `backend/src/voltage.c:149` (`switch_contact_pins`),
  `backend/src/models/model_pins.h:21`, `backend/include/parts.h:50`
  (`PartSwitch`), `backend/tests/support/listing.c:77`.
- Precedent: the barrel-jack switched contact (`connector.c`,
  `circuit.c:151`, `circuit.c:350`), `backend/tests/connector_test.c`.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-C is complete | `ls planning/completed/plan-27-C-*` → one file | NOT MET until C archives (2026-11-07) |
| `PartSwitch` still has three fields | `grep -c 'unsigned positions; const char \*actuator; uint32_t rated_mv; }' backend/include/parts.h` → 1 | MET (2026-11-07) |

If plan-27-C is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The `switch` device accepts `"momentary"` (bool), `"normallyClosed"` (bool,
  needs `"momentary": true`) and `"joined"` (array of `[pin, pin]`), with the
  loader errors of § 4.1.
- `PartSwitch` gains `momentary`, `normally_closed`, `joined` (count).
- `circuit_start` leaves a normally-closed contact closed, every joined pair
  closed, and `circuit_set_switch` toggles only real positions.
- The voltage check sees the same closed contacts the simulation has.
- `ctest` runs T0 + 1 programs (`tactile_test`); every existing golden line is
  unchanged.

### Non-goals (explicit constraints)

- DIP switches: JSON, golden lines, behavior unchanged (byte-identical
  `parts.txt` and `traces.txt`).
- No new kind, enum or model (`PART_SWITCH`, `SWITCH_MODEL` stay).
- The backend keeps no pressed state of its own: "momentary" is data for the
  app; the contact state is `circuit_set_switch`'s.
- No change to the engine (`backend/src/engine/`).

## 2. Current State

- `switch.c` loads `actuator`, `contacts` (each `[pin, pin]`, exactly one pin
  per name via `kind_pin`) and `ratedVolts`; `set_switch` calls
  `sim_set_contact` for position `p` (1-based); "Every position starts OFF".
- `switch_contact_pins(part, position, &a, &b)` (`switch.c:101`) serves the
  voltage check: `voltage.c:149` joins the nets of every recorded closed
  contact `(part, position)`.
- `circuit.c`: `circuit_add_part` records a connector's contacts as closed
  (line 151); `circuit_start` re-applies recorded contacts for connectors
  (line 350) because pins have nets only after building; `circuit_set_switch`
  (line 296) calls the model's `set_switch` and `record_contact`.
- Golden: 36 switch lines in `backend/tests/golden/parts.txt`
  (`grep -c '|switch|'` → 36), all `positions=N actuator=… rated=24000`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Switch parts / devices today | 36 / 27 | `grep -c '|switch|' backend/tests/golden/parts.txt`; `grep -c '"id"' schematical/schematical/Parts/switch.json` |
| Places that set `PartSwitch.positions` | 1 | `grep -n 'info.positions' backend/src/models/switch.c` → `switch.c:64` |
| Lines naming `dip_switch` (voltage.c 4, switch.c 1, trace_check.c 1, library_check.c 1, listing.c 4, PartCatalog.swift 1) | 12 | `grep -rn 'dip_switch' backend/src backend/tests schematical/schematical --include='*.c' --include='*.swift' --include='*.h' \| wc -l` → 12 |
| ctest programs T0 | 24 | `grep -c '^logic_test' backend/CMakeLists.txt` |

### Verified properties

- `kind_bool` exists (`backend/src/library/kind_util.c:205`) and `kind_names`
  reads string arrays; `kind_pin` rejects a name used by two pins
  (`kind_util.c:370`), so joined pins need distinct names (`A1`, `A2`).
- `voltage.c:149` takes any `position` that `switch_contact_pins` accepts, so
  joined contacts, numbered after the real positions, reach the voltage check
  unchanged.
- `trace_check` drives a switch through the model's `set_switch` at engine
  level (`trace_check.c:74`), without a `Circuit`; it never calls
  `circuit_start`, so the new defaults do not affect it and its 36 switch
  lines stay identical.

## 3. Design Overview

Contacts are numbered: positions `1…positions` are the button's contacts (what
`circuit_set_switch` addresses), then `positions + 1 … positions + joined` are
the joined pairs. The backend treats joined pairs as contacts that are always
closed and not addressable. A normally-closed switch has its positions
recorded as closed when it is added to a circuit (like the barrel jack's
contact), so `circuit_set_switch(…, 0)` opens it and the voltage check sees
it. Risk is small and sits in `circuit.c` (start-up order); byte-identity is
the gate for DIP switches only (`parts.txt`, `traces.txt` unchanged); the new
behavior is gated by `tactile_test`.

## 4. Detailed Design

### 4.1 Device keys (`switch.c`, `DEVICE_KEYS`)

| Key | Value | Errors (path-style, via `kind_error`) |
|---|---|---|
| `"momentary"` | optional bool, default false | `"momentary" must be true or false` |
| `"normallyClosed"` | optional bool, default false | same wording; `"normallyClosed" needs "momentary"` |
| `"joined"` | optional non-empty array of `[pin name, pin name]` | `"joined" must be a non-empty array of [pin, pin]`; `a joined pair joins two different pins, not "X" twice`; bind: `no pin named "X"` / `pin name "X" is used by 2 pins; one is needed` |

`PartSwitch` (`parts.h`): `unsigned positions; const char *actuator; uint32_t
rated_mv; unsigned momentary; unsigned normally_closed; unsigned joined;`
(`joined` = number of pairs).

### 4.2 Model (`switch.c`)

- `SwitchPins` holds `positions` contact pairs then `joined` pairs
  (`pins[positions + j]`).
- `switch_contact_pins` accepts `1 … positions + joined`.
- `switch_set(position, on)`: refuses (`return 0`) `position < 1` or `>
  positions`; sets that contact; then re-asserts every joined pair closed
  (`sim_set_contact(…, 1)`), idempotent.

### 4.3 Circuit (`circuit.c`)

- `circuit_add_part`: for `PART_SWITCH`, `record_contact(…, p, 1)` for every
  position `p` when `normally_closed`, and for every joined position
  `positions + j` always.
- `circuit_start`: the connector loop becomes: for `PART_CONNECTOR` as today;
  for `PART_SWITCH` with `normally_closed` or `joined`, call the model's
  `set_switch(position, contact_recorded(position))` for `1 … positions`
  (which also asserts the joined pairs). DIP switches are skipped, so their
  start-up is unchanged.
- `circuit_set_switch` is unchanged: a joined position fails in the model →
  `"<name> has no position N"`.

### 4.4 Listing and docs

- `tests/support/listing.c`: after `rated=…` append ` momentary` when set,
  ` nc` when `normally_closed`, ` joined=<n>` when `joined > 0` (nothing for
  DIP switches).
- `docs/parts-format.md` § `switch`: the three keys; `docs/backend.md`: a
  bullet under Using it ("a normally-closed switch starts closed;
  `circuit_set_switch` is ON = closed for every switch").

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Loader keys and model

- [ ] `backend/include/parts.h`: `PartSwitch` fields (§ 4.1).
- [ ] `backend/src/models/switch.c`: keys, `SwitchDevice`/`SwitchPins`,
      `switch_contact_pins`, `switch_set` (§ 4.1, § 4.2).
- [ ] `backend/tests/support/listing.c`: § 4.4.
- [ ] Tests: `backend/tests/library_check.c` `check_switch_cases()` gains
      cases (message strings of § 4.1): `momentary` not bool;
      `normallyClosed` without `momentary`; `joined` empty; `joined` pair
      naming one pin twice; `joined` naming a missing pin; `joined` naming a
      pin used by two pins; and one valid file with all three keys
      (`part->dip_switch->momentary == 1`, `normally_closed == 1`,
      `joined == 2`).

Acceptance: the loader accepts the three keys and rejects each malformed case;
DIP switches parse as before.
  Check: `cmake --build /tmp/logic-p27 -j8 --target library_check && /tmp/logic-p27/library_check` → `FAIL`-free output, last line as before plus the new `ok` lines; then `/tmp/logic-p27/listing_check` → passes (est. 4 min; a full build of one target is the smallest check that links the loader).
Commit: —

### Phase 2 — Circuit start-up and `tactile_test`

- [ ] `backend/src/circuit.c`: § 4.3.
- [ ] `backend/tests/tactile_test.c` (new) and `logic_test(tactile_test)` in
      `backend/CMakeLists.txt`: loads `PARTS_DIR` (`load_parts_dir`), then
      `load_parts` of an inline fixture with devices `TACT-T2` (NO, pins
      `A`,`B`, footprint `RES-0402`), `TACT-T4NC` (NC, 4 pins `A1 A2 B1 B2`,
      `joined [[A1,A2],[B1,B2]]`, footprint `SW-DIP-4`) and `TACT-T4` (NO, same
      pins), all `momentary`, `ratedVolts` 3 (below the library's lowest supply, `5V`). Cases (net values via
      `circuit_net_value`, a `5V` supply on side A, nothing driving side B):
      1. `TACT-T2`: B is not 1 after `circuit_start` + `circuit_advance`;
         after `circuit_set_switch(1, on)` and an advance, B is 1; after `off`,
         B is Z again.
      2. `TACT-T4NC`: B is 1 at start; `off` → B Z; `on` → B 1.
      3. `TACT-T4`: pin `A2` (wired to its own net) is 1 while the button is
         up (joined to A1); `B2` is Z until pressed.
      4. `circuit_set_switch(TACT-T4NC, 2, on)` returns -1 and
         `circuit_error` contains `has no position 2`.
      5. Voltage (supply `5V` on side A): `TACT-T2` (rated 3 V) is open at
         `circuit_start`, which succeeds; pressing it during a run stops the run
         (`circuit_advance` returns 0 and `circuit_error` names the switch
         rating); `TACT-T4NC` (rated 3 V), closed by default: `circuit_start`
         returns -1 with the same text.
- [ ] `docs/parts-format.md`, `docs/backend.md`: § 4.4.

Acceptance: the five cases pass and the DIP goldens are unchanged.
  Check: `cmake --build /tmp/logic-p27 -j8 && ctest --test-dir /tmp/logic-p27 -R 'tactile_test|switch_test|connector_test|library_check|listing_check|trace_check'` → `100% tests passed, 0 tests failed out of 6` (est. 6 min: a full build; `listing_check` and `trace_check` need it).
Commit: —

## Validation Plan

- Tests: `library_check` (key cases), `tactile_test` (behavior, voltage),
  `switch_test`, `connector_test`, `listing_check`, `trace_check`
  (DIP regression: goldens untouched — `git diff --stat backend/tests/golden`
  → empty).
- Coverage check: `grep -c 'tactile_test' backend/CMakeLists.txt` → 1 and
  `ctest --test-dir /tmp/logic-p27 -N | tail -1` → `Total Tests: 25`.
- Runtime proof: `tactile_test` is the runtime proof at library level; the app
  proof is K.
- Doc sync: `docs/parts-format.md` § `switch`, `docs/backend.md`.
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/logic-p27 -j8` →
  `100% tests passed, 0 tests failed out of 25` (est. 6 min).

## Decisions

- **Reuse `PART_SWITCH`** — author (plan-27-A § Decisions).
- **Joined pairs are non-addressable contacts after the real positions** —
  author. Rejected: a second list in `Part` for the voltage check (the
  voltage check already walks `closed[]` through `switch_contact_pins`).
- **`circuit_set_switch` means ON = contact closed for every switch** — author;
  the app maps "pressed" to it (J). Rejected: an inverted flag for NC parts
  (two meanings of one call).
- **Pressed state is not in the backend** — author.

## Corrections

<!-- filled in during execution -->

## Summary

Risk: start-up order in `circuit_start` (contacts must be applied after pins
have nets) — the connector precedent covers it, `tactile_test` cases 2 and 5
prove it. Untouched: the engine, DIP switch data, every golden line, the app.
