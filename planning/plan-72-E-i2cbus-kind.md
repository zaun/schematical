# plan-72-E: Backend — `io.links`, `io.splits`, the `i2cbus` kind

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-D

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
E adds the kind `i2cbus` for I2C muxes, switches, buffers, repeaters, bus
extenders, accelerators, address translators and isolated buffers, on the
converter engine and the `io` section of plan-72-D. Two new `io` keys carry
the bus: **links** (engine contacts between two pins, closed while a
condition holds) and **splits** (one bidirectional pin split into a TX/RX
pair, the P82B96 way).

References:

- plan-72-D (`io`, `CV_MODE_IO`, the test host `tests/support/bushost.c`),
  plan-72-B § 3 (`i2cbus` templates).
- `backend/src/engine/sim.h` (`sim_set_contact`, `sim_contact_closed`: "A
  contact joins two pins of one instance. While it is closed, the nets on
  those pins act as one net"), `backend/src/models/relay.c` (a model that
  moves contacts), `backend/src/voltage.c` (step 1: nets grouped by closed
  switch contacts and parts' fixed contacts).
- `docs/parts-format.md` § `switch`, § `relay`, § Two supply domains,
  § `converter` `bus` (`{"style": "none"}`, plan-56-F).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-D complete | `grep -c '^- \[ \]' planning/plan-72-D-ioexpander-kind.md` → 0 | NOT MET (2026-11-25) |
| `i2cbus` absent | `grep -c PART_I2CBUS backend/include/parts.h` → 0 | MET (2026-11-25: 0) |
| Golden sizes at E's start | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` | recorded here; unchanged at E's end (no part) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `PART_I2CBUS`, `I2CBUS_KIND` (`backend/src/models/i2cbus.c`),
  `Parts/i2cbus.json` (empty `devices`), its section in
  `docs/parts-format.md`, the drawer rows (Interface → I2C Bus → Buffers /
  Muxes & Switches).
- `backend/tests/i2cbus_test.c` (new program, T0 + 2 with D's) passes with
  five fixtures: a `switch` (TCA9548A-like: control 0x04 lets plan-72-D's
  `pca9555` fixture on SD2/SC2 ACK its address and a second one, at the same
  address on SD0/SC0, stay silent; the library has no I2C EEPROM to use,
  `grep -c -E '^(24LC|24AA|AT24|M24)' backend/tests/golden/parts.txt` → 0), a
  `mux` (enable + select), a `buffer` with EN (a write passes only while EN
  is high), a `split` (Sx low from a driver → Tx low; Rx low → Sx low; no
  latch-up when Tx is wired to Rx), a `passive` device that does not stop a
  run, and a `facts` device that `circuit_add_part` refuses.
- No existing golden changes; every new line covered.

### Non-goals

- No part (H adds them). No change to `switch`, `relay` or the voltage check's
  rules for other kinds.
- Not modeled: bus capacitance and rise time (accelerators drive nothing),
  offset voltages, hot-swap precharge, stuck-bus recovery and fault timers
  (stored in registers where the device has them), address translation.

## 2. Current State

- Contacts are model-driven already: `relay.c` closes and opens them with
  `sim_set_contact`; a closed contact makes two nets one, so open-drain I2C
  lines with pull-ups on either side behave as one bus.
- The voltage check joins nets only through closed **switch** contacts it is
  told about and parts' **fixed** contacts (`voltage.c` step 1,
  `part_fixed_contact`); a contact a model closes during a run is not one of
  them, so each side of a buffer keeps its own net in the voltage check.
- A converter-engine device may have no bus (`{"style": "none"}`), for
  `leddriver` / `lcddriver` only (`docs/parts-format.md` § `converter` `bus`).

## 3. Design Overview

`i2cbus` = `cv_load_device(ctx, json, CV_MODE_I2CBUS)`: the `io` section with
`links` and/or `splits` and optional `ports` (LTC4302 GPIO, INT outputs);
`bus` optional, and `{"style": "none"}` allowed (a buffer has no control
bus). The links are evaluated with the ports — after every register write,
pin change and reset — and a changed link calls `sim_set_contact`.

A device with neither links nor splits nor ports is **passive** (a rise-time
accelerator): it loads, drives nothing and does not turn Run off (the
`opamp.c` precedent of `docs/backend.md` § Adding a kind). An address
translator or an isolated pair is `"simulated": false` with the passive model
behind it (`docs/backend.md`: "still ships a passive model").

**Voltage check.** A buffer that translates levels (VCCA/VCCB) declares
`domains`, so each side's pins are checked against its own supply; since run
time contacts are not voltage-check joins (§ 2), a 3.3 V side and a 5 V side
never meet in the check. A switch/mux has one supply; its downstream pins are
checked against it (the TCA9548A's channels tolerate 5 V at a 3.3 V VCC: B
writes `inputMax` from the datasheet).

**Split semantics** (`io.splits`: `{"bidir": "SX", "tx": "TX", "rx": "RX"}`):
TX drives 0 while SX reads 0 and the model is not itself driving SX, else
releases (open drain); the model drives SX 0 while RX reads 0, else
releases. While the model drives SX it holds TX where it was, which breaks the
SX → TX → RX → SX loop a Tx-to-Rx wiring makes, the role the real part's
offset voltage plays.

Correctness risk concentrates in two places: a link that closes while both
sides are driven strong and opposite (the engine reports a short, which is
right for a real bus fight and must not be masked), and the split's loop
breaking. Both get dedicated fixture cases.

Rejected: modeling a buffer as two `buffer`-kind gates (it has no direction;
an open-drain bus needs both ways at once); joining buffer sides in the
voltage check (it would refuse every 3.3 V ↔ 5 V buffer the parts exist for);
modeling address translators by rewriting bits in the I2C front end (a
byte-level filter on a contact the engine does not have).

## 4. Detailed Design

### 4.1 `io.links`

| Key | Value |
|---|---|
| `"links"` | array of `{"a": pin, "b": pin, "closedWhen": condition or array}`; no `closedWhen` = always closed (fixed). A pin may be in several links (a mux's SDA). `a` ≠ `b` |

Conditions are the converter's (`"REG.FIELD=v"`, arrays all hold); a pin
level enters a condition through `pins.levels` (EN → `PIN.EN`). During reset
(the `pins` reset pin, power-on) every link reads its conditions against the
reset register values.

### 4.2 `io.splits`

Array of `{"bidir", "tx", "rx"}` (§ 3); `bidir` `inout`, `tx` `output`
(open drain), `rx` `input`.

### 4.3 The kind

`DEVICE_KEYS` as `ioexpander` plus `simulated`; `CV_MODE_I2CBUS` accepts
`io` without ports, requires `bus` (any style) or `{"style": "none"}`,
rejects what `CV_MODE_IO` rejects. `PartI2cBus` facts: `channels` (links
whose `a` is the upstream SDA, counted by distinct `b`; 1 for a buffer),
`type` (`switch`, `mux`, `buffer`, `split`, `extender`, `passive`, `facts`:
the device key `"type"`, required, checked against the sections present:
a `switch`/`mux` needs `bus` and links; `buffer` links; `split` splits;
`passive` nothing; `facts` `"simulated": false`).

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — the kind shell

- [ ] Grep first, then append after the last kind: `PART_I2CBUS`, `PartI2cBus`, `I2CBUS_KIND`, `CV_MODE_I2CBUS`, `i2cbus.c`, CMake, `Parts/i2cbus.json`, listing / trace / listing_check lines.
- [ ] `library_check` cases: each `type` with a missing or a foreign section; a link naming one pin twice; a split pin with the wrong role; `{"style": "none"}` accepted on `i2cbus`, still rejected on `converter`.

Acceptance: `ctest --test-dir /tmp/logic-p72 -R 'library_check|listing_check|trace_check'` → 3 passed; goldens unchanged (est. 3 min).
Commit:

### Phase 2 — links and splits

- [ ] `io.links` evaluation and `sim_set_contact` calls; `io.splits`.
- [ ] `i2cbus_test.c` with the five fixtures of § 1 and the two risk cases of § 3 (a link closing onto a bus fight reports `smoke:`; split with TX wired to RX releases when the outside driver lets go).
- [ ] `docs/parts-format.md` § `i2cbus`; § `converter` `bus` (`none` also for `i2cbus`); `docs/backend.md` § Layout.

Acceptance: `ctest --test-dir /tmp/logic-p72 -R i2cbus_test` → 1 passed; `node scripts/coverage.js --backend --check` exit 0 (est. 6 min).
Commit:

## Validation Plan

- Tests: `i2cbus_test` (new), `library_check`, `listing_check`, `trace_check`.
- Goldens: unchanged.
- Doc sync: `docs/parts-format.md` § `i2cbus`, § `converter` `bus`; `docs/backend.md` § Layout.
- Final gate (once): `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed` (T0 + 2), `node scripts/source-size.js --check` exit 0 (est. 5 min).

## Decisions

- Muxes and buffers as engine contacts; translators and isolated pairs facts only — owner's rule "logic where it's cheap; facts only where the engine cannot pass the line" (plan-72-A Decisions), applied by the author.
- Run-time contacts are not voltage-check joins; level-translating buffers declare `domains` — the author's call (§ 3).
- Kind name `i2cbus` — the author's call.

## Corrections

(none yet)
