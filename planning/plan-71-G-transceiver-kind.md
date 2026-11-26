# plan-71-G: Backend — the `transceiver` kind: loader, logic model, tests, docs

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-F

G adds the `transceiver` kind: the loader and facts of a CAN, RS-485 /
RS-422, RS-232 or LIN line transceiver, and a logic model.

- A driver turns its logic input into the line level.
- A receiver turns the line level back into logic.
- Enables turn either off, as the datasheet's mode pins do.

The line pins are analog pins for the voltage check. A part with a VIO pin
or an isolation barrier has two supply domains, read by the shared loader
plan-75 moves out of `buffer.c` (`backend/src/library/kind_domains.c`, a
prerequisite).

G does not depend on the data of B–F. It is ordered after them so that
plan-71 runs straight through. Its tests use their own fixture devices.

References:

- `docs/backend.md` § Adding a kind, § ERC pin types; `docs/parts-format.md`
  § Kinds, § Two supply domains (`domains`, `vccB`), § `buffer` (enables:
  one control `{"pin", "active"}` or an array of up to four that must all be
  asserted).
- `backend/src/library/kind_domains.c` and `kind.h` (plan-75: the two-supply
  loader G calls), `backend/src/models/buffer.h` (`BufEnableNames`, `buf_enable_names`,
  `buf_bind_enable`, `buf_enabled`: the enable helpers G reuses).
- `backend/src/models/serial_max3100.c:33-41`: the MAX3140's RS-485 reading,
  which G's RS-485 receiver matches.
- `backend/src/models/powerswitch.c` (a recent kind with `pin_analog`,
  `pin_open_drain`, timers); `backend/src/models/opamp.c`, `opamp_test.c`
  (the newest kind and test layout).
- `backend/src/engine/sim.h` (`sim_schedule_inertial_drive`,
  `DRIVE_WEAK` / `DRIVE_STRONG`); `backend/src/engine/net.h`
  (`net_mask_shorted`: two strong drivers that disagree are a short).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-71-F complete | `ls planning/completed/plan-71-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |
| The kind is absent | `grep -c 'PART_TRANSCEIVER' backend/include/parts.h` → 0; `ls backend/src/models/transceiver.c` → no such file | MET (2026-11-25: 0; no such file) |
| plan-75 complete (the shared supply-domain loader) | `ls planning/completed/plan-75-*.md` → exists; `grep -c 'kind_domains_load' backend/src/library/kind.h` → 1 | NOT MET (2026-11-25: plan-75 written, not started) |
| Golden sizes at G's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/erc_pins.txt` | recorded here at G's start (P0 40832, R0 40832, E0 40832 at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `transceiver.json` files load: kind `PART_TRANSCEIVER`, `Part.transceiver`
  facts, `simulated` 1, and the rejections of § 5 (b).
- In a circuit, two CAN parts on one CANH / CANL pair, two RS-485 parts on
  one A / B pair, two RS-232 parts joined T1OUT → R1IN, and two LIN parts on
  one LIN wire each carry a level from one part's logic input to the other
  part's logic output (§ 5).
- An `MCP2200-I/SO` byte sent across two fixture CAN parts arrives at a
  second `MCP2200-I/SO`.
- `ctest` passes at T0 + 1 programs (`transceiver_test`), with the goldens
  unchanged.

### Non-goals (explicit constraints)

- No change to any `buffer` behavior, key or message, and no change to
  `kind_domains.c` (plan-75's API is used as it is).
- No analog behavior: no slew, no edge rates, no termination or bus load, no
  common-mode range, no threshold voltages, no current limit, no thermal
  shutdown (§ 3.5).
- No change to `circuit.c`, `voltage.c`'s rules or the engine.
- No part file and no app change (H, I).

## 2. Current State

- `PartKind` ends with `PART_OPAMP` (`backend/include/parts.h:32`), and
  `KINDS[]` with `&OPAMP_KIND` (`backend/src/library/kinds.c:106`).
  `TRACE_LAST_KIND` is `PART_OPAMP` (`backend/tests/trace_check.c:22`), and
  so are `listing_check.c`'s two arrays (`backend/tests/listing_check.c:50-51`).
  Plans 69, 70, 72 and 74 add kinds after it at the same time, so every such
  line here greps first and goes after whatever is last.
- `Part.domains` / `domain_count` (`parts.h:491-493`, `PartSupplyDomain`
  `parts.h:185-188`) are generic. The voltage check judges each pin of a
  domain part against its own domain (`backend/src/voltage.c:32-36`,
  `domain_of` at `voltage.c:93`). At planning only `buffer.c` fills them
  (`buffer.c:909-940`), from its static `load_domains` (`buffer.c:710`),
  `voltage_b` (`:744`) and `bind_domain` (`:833`); plan-75 (a prerequisite)
  moves these, unchanged, to `kind_domains.c` with the API of § 3.0.
- Enables: `buf_enable_names` / `buf_bind_enable` / `buf_enabled`
  (`buffer.h`, exported) parse and evaluate `{"pin", "active"}` controls
  (1 asserted, 0 deasserted, −1 unknown).
- `part_pin_analog` (`model_pins.h`) makes the voltage check skip a pin.
  `part_pin_open_drain` turns an output into `open_collector` for the ERC
  and into "never drives high" for the voltage check. `part_pin_tristate`
  gives the ERC's `tri_state` (`docs/backend.md` § ERC pin types).
- The engine: `sim_schedule_inertial_drive(sim, inst, pin, value, DRIVE_STRONG | DRIVE_WEAK, delay)`;
  strong 0 against strong 1 on one net is a short (`net_mask_shorted`), and
  `circuit_shorted_net` reports it.
- The MAX3140 reading (`serial_max3100.c:36-39`): A ≠ B gives A; A = B
  (shorted) or both Z gives 1 (true fail-safe); X, or one input Z, gives X.

## 3. The kind

### 3.0 The domain helpers (plan-75)

G calls plan-75's API (`backend/src/library/kind_domains.c`, declared in
`kind.h`):

```c
/* Two supply domains (plan-20-F; moved from buffer.c by plan-75). */
typedef struct { const char *power; const char *const *pins; size_t pin_count; } KindDomainNames;
typedef struct { int present; KindDomainNames d[2]; } KindDomains;
int kind_domains_load(KindContext *ctx, const cJSON *json, KindDomains *out);   /* "domains": 2 objects, or absent */
int kind_voltage_b(KindContext *ctx, const cJSON *json, const PartVoltage *base,
                   const PartVoltage **out);                                    /* "vccB" / "inputMaxB" */
/* Per pinout: the power pins and other pins of each domain, as indices; every
   pin except ground, nc and absent in exactly one domain. */
typedef struct { size_t *power[2], power_count[2], *pins[2], pin_count[2]; } KindDomainPins;
int kind_domains_bind(KindContext *ctx, const PartPin *pins, size_t count,
                      const KindDomains *names, KindDomainPins *out);
/* Part.domains / domain_count from the bound pins, vcc (domain 0) and vcc_b. */
int kind_domains_part(KindContext *ctx, const KindDomainPins *b, const PartVoltage *vcc,
                      const PartVoltage *vcc_b, Part *out);
```

Its messages are the buffer kind's (`must be an array of 2 domains`,
`"domains": "%s" is not a power pin`, `… is in two domains`, `… is in no
domain`, `"inputMaxB" without "vccB"`, `"inputMaxB" below the "vccB" max`),
and the `transceiver` loader reports them unchanged.

### 3.1 Part file format (`"kind": "transceiver"`)

Device keys:

| Key | Value |
|---|---|
| `"protocol"` | required: `"can"`, `"rs485"` (RS-485 and RS-422), `"rs232"` or `"lin"`; else `unknown protocol "x"` |
| `"drivers"` | 0–8 driver objects (`"drivers" must have 0 to 8 entries`) |
| `"receivers"` | 0–8 receiver objects; drivers + receivers ≥ 1 (`a transceiver needs a driver or a receiver`) |
| `"failSafe"` | `rs485` only, required there: `true` when an idle (both Z) or shorted bus reads 1, `false` when it reads X. On another protocol: `"failSafe" is only for rs485` |
| `"isolated"` | optional, default `false`: a fact (an isolation barrier between the domains) |
| `"analog"` | optional: pin names the voltage check skips beyond the bus pins (charge-pump pins, INH, SPLIT, VREF, slew and mode-select pins the model does not use) |
| `"domains"` | optional: two supply domains, the format of § Two supply domains (logic side first). Requires `"vccB"` on the device or each part |
| `"driverDelay"`, `"receiverDelay"` | ns, above 0 and at most 100000; each required when the device has a driver / a receiver (`missing "driverDelay"`), and an error when it has none (`"receiverDelay" without a receiver`) |
| `"dataRate"` | bits per second, 1 to 1000000000, required (DigiKey's Data Rate parameter) |
| `"vcc"`, `"inputMax"`, `"vccB"`, `"inputMaxB"` | as every kind; `vccB` / `inputMaxB` only with `domains` |

Driver object: `{"input": <pin>, "outputs": [<pin>, …], "enable": <control or array>}`.
`outputs` has 2 entries for `can` (CANH, CANL) and `rs485` (A or Y, B or
Z), and 1 for `rs232` and `lin` (`a can driver needs 2 outputs`, `an rs232
driver needs 1 output`). `enable` is optional: absent means always enabled,
in the `buffer` kind's format (`buf_enable_names`).

Receiver object: `{"inputs": [<pin>, …], "output": <pin>, "enable": …, "disabled": "z" | "high", "outputStyle": "push-pull" | "open-drain"}`.
`inputs` counts as `outputs` above. `disabled` (default `"z"`) is what the
output does while the enable is deasserted: CAN standby holds RXD high, and
everything else releases it. `outputStyle` defaults to `"push-pull"`
(LIN's RXD is open drain).

Part keys: `"vcc"`, `"inputMax"`, `"vccB"`, `"inputMaxB"`, `"dataRate"`
(a grade's own rate).

### 3.2 Pin rules (`bind_pinout`)

- A driver's `input` is role `input`.
- Its `outputs` are `output` (full duplex, RS-232, RS-422) or `inout` (a pin
  some receiver also names: half duplex, CAN, LIN).
- A receiver's `inputs` are `input` or `inout`, and its `output` is `output`.
- An enable pin is `input`.
- A pin named as an output by two drivers: `transceiver pin "A" is driven twice`.
- An `inout` pin no receiver reads: `transceiver pin "A" is inout but no receiver reads it`.
- Every other pin is `power`, `ground`, `nc`, `absent`, or named in
  `analog`. Otherwise: `transceiver pin "X" is not a driver, receiver,
  enable or analog pin`.
- At least one `power` and one `ground` pin: `a transceiver needs a power
  pin and a ground pin`.
- A name not in the pinout: the `kind_pin` message
  (`no pin named "X"`).
- Role errors read `transceiver pin "TXD" must be an input` /
  `… must be an output` / `… must be an input or inout`.

**Bus pins** are every driver output and receiver input. `pin_analog`
answers 1 for them and for the `analog` names. `pin_open_drain` answers 1
for an open-drain receiver's output. `pin_tristate` answers 1 for:

- an `output`-role driver output of a driver with an enable;
- the output of a receiver with an enable and `"disabled": "z"`.

### 3.3 The model (`TRANSCEIVER_MODEL`)

Every driver and receiver re-evaluates on any change of its pins (its
input, its enable pins, and for a receiver the bus pins, including those
the part drives itself, so a CAN or LIN part hears its own transmission as
the real parts do). Each one drives through `sim_schedule_inertial_drive`
after its delay (`driverDelay` / `receiverDelay`). An enable at X or Z
makes what it gates X (the `buffer` rule). A driver that is disabled drives
Z (LIN: weak 1, below).

| Protocol | Driver (enabled) | Receiver (enabled) |
|---|---|---|
| `can` | input 0: CANH strong 1, CANL strong 0 (dominant). Input 1 or Z (TXD's internal pull-up): both Z (recessive). Input X: both X | CANH 1 and CANL 0: output 0. Both Z, or CANH = CANL: 1. Any other mix (an X, or one side Z): X |
| `rs485` | A (Y) = input, B (Z) = NOT input, strong. Input Z or X: both X | A ≠ B, both known: A. Both Z or A = B: 1 if `failSafe`, else X. Otherwise X (the MAX3140 reading) |
| `rs232` | output = NOT input, strong. Input Z reads 1 (the T_IN pull-up): output 0 | input Z reads 0 (the 5 kΩ R_IN pull-down). Output = NOT input |
| `lin` | input 0: bus strong 0. Otherwise, and while disabled or unpowered: bus **weak 1** (the responder's 30 kΩ termination). Input Z reads 1 | output = bus (0 or 1; X gives X). Open drain: 1 releases the pin |

A disabled receiver drives its output Z (`"disabled": "z"`) or strong 1
(`"high"`). An open-drain receiver's 1 is Z.

Consequences, each checked by a § 5 case:

- CAN is a wired-AND. Any number of dominant drivers agree, and a lone
  dominant driver against recessive ones drives the pair alone.
- Two RS-485 drivers enabled at once with opposite inputs are a short on A
  and B. The engine reports it (`circuit_shorted_net`), which is how a
  logic simulator shows bus contention.
- LIN needs no external part for the bus. A commander's 1 kΩ pull-up (a
  resistor or `PULLUP-*`) is a weak 1 and agrees.
- A MAX3140 (`serial`) and a `transceiver` RS-485 part on one A / B pair
  read each other, because both use the reading of § 2.

### 3.4 Facts and the voltage check

```c
/* A line transceiver (plan-71-G). protocol "can", "rs485", "rs232", "lin";
   driver_count, receiver_count 0..8; duplex 0 none (drivers or receivers
   only), 1 half (a receiver reads a driver's outputs), 2 full; fail_safe
   (rs485), isolated; data_rate_bps; driver_delay_ps, receiver_delay_ps (0
   when the part has no such direction). */
typedef struct { const char *protocol; size_t driver_count, receiver_count;
                 uint8_t duplex, fail_safe, isolated; uint32_t data_rate_bps;
                 uint64_t driver_delay_ps, receiver_delay_ps; } PartTransceiver;
```

`const PartTransceiver *transceiver;` goes on `Part`. Supply checks work
with no new voltage code:

- the `power` pins against `Part.vcc`;
- a domain part's pins against their own domain (`domains` through
  `kind_domains_part`);
- logic inputs against `inputMax`;
- bus pins and `analog` pins skipped.

### 3.5 Not modeled

The docs' `### transceiver` section lists these:

- slew-rate and edge-rate control;
- data-rate limits (a part passes any rate; `dataRate` is a fact);
- termination resistance and bus loading;
- common-mode range and receiver thresholds, beyond the digital reading of
  § 3.3;
- driver current limit and thermal shutdown;
- the TXD-dominant time-out;
- remote and local wake-up and wake flags, and INH;
- bus-fault detection and nFAULT / ERR_N pins (never driven);
- RS-232 charge-pump voltages, auto-shutdown and `INVALID`;
- an integrated LIN regulator's output (not driven);
- the isolation barrier's delay and common-mode transient immunity;
- CAN FD timing;
- the half/full-duplex select, polarity and slew pins (`analog`, so
  ignored).

A part's either-polarity enable pair (`G` / `G#` of an RS-422 quad driver)
is written as one control (plan-71-C § 3).

## 4. Docs

- `docs/parts-format.md`: a `### transceiver` section (§ 3.1–3.3 keys, pin
  rules and logic, § 3.5, an example CAN device and an example RS-232
  device). Add a row `Transceiver › CAN / LIN / RS-232 / RS-485 (plan-71)` to
  the Interface line of § The drawer's categories.
- `docs/backend.md`:
  - § Layout: `transceiver.c` (plan-75 added `kind_domains.c`);
  - § ERC pin types: a row of the gating table for `transceiver`
    (`drivers[].enable`, `receivers[].enable` with `disabled` `z` → the
    output-role driver outputs, the receiver outputs).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind and its tests

- [ ] `backend/include/parts.h`: `PART_TRANSCEIVER` after the current last
      `PartKind` (`grep -c PART_TRANSCEIVER` → 0 first), `PartTransceiver`,
      `const PartTransceiver *transceiver;` on `Part`.
- [ ] `backend/src/models/transceiver.c`: `TRANSCEIVER_KIND` and
      `TRANSCEIVER_MODEL` per § 3.1–3.4 (`pin_analog`, `pin_open_drain`,
      `pin_tristate`; no `units`).
- [ ] `backend/src/library/kinds.c`: the `extern` and `&TRANSCEIVER_KIND`
      at the end of `KINDS[]` (`grep -c TRANSCEIVER_KIND` → 0 before, 2
      after); `backend/CMakeLists.txt`: `src/models/transceiver.c` and
      `logic_test(transceiver_test)`.
- [ ] `backend/tests/support/listing.c`: `case PART_TRANSCEIVER: return "transceiver"`
      and the facts line `proto=%s drv=%zu rcv=%zu duplex=%u fs=%u iso=%u rate=%u td=%llu rd=%llu`.
      `TRACE_LAST_KIND` and `listing_check.c`'s two array sizes → the last
      `PartKind` entry in `parts.h` after `PART_TRANSCEIVER` is added (read the
      enum at this step).
- [ ] `backend/tests/support/transceiver_fixture.h` and
      `backend/tests/transceiver_test.c`: the cases of § 5.

Acceptance: the kind loads, its logic follows § 3.3, and nothing else changes.
  Check: `cmake --build /tmp/logic-p71-g -j8 && /tmp/logic-p71-g/transceiver_test` → every line `PASS`; `ctest --test-dir /tmp/logic-p71-g -j8` → `100% tests passed`, T0 + 1 programs; `git diff --stat backend/tests/golden/` → empty (est. 2 min: incremental build plus the suite's 23 s).
  Result:
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check`
  → exit 0 and `node scripts/coverage.js --check --backend` → exit 0, so no new or
  changed backend file is over 1000 code lines or has an uncovered line (est. 6 min:
  one instrumented backend build with `ctest`, the only tool that shows every new
  line runs).
Commit:

### Phase 2 — Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name the kind and its gates.
  Check: `grep -n '^### `transceiver`' docs/parts-format.md` → 1 line; `grep -c 'transceiver.c' docs/backend.md` → 1 or more; `grep -c 'Transceiver ›' docs/parts-format.md` → 1 (est. < 1 min).
  Result:
Commit:

## 5. `transceiver_test` cases

The fixture (`transceiver_fixture.h`) is one part file string with six
devices. Every part has footprint `SOIC-8` or `SOIC-16`.

| Device | Part | Pinout | Keys |
|---|---|---|---|
| `test-can` | `TESTCAN` | 1 TXD in, 2 GND, 3 VCC, 4 RXD out, 5 NC, 6 CANL io, 7 CANH io, 8 S in | driver TXD → [CANH, CANL], enable `{"pin": "S", "active": 0}`; receiver [CANH, CANL] → RXD; `driverDelay` 50, `receiverDelay` 60, `dataRate` 1000000, `vcc` [4.5, 5.5] |
| `test-can-vio` | `TESTCANV` | 1 TXD, 2 GND, 3 VCC, 4 RXD, 5 VIO, 6 CANL, 7 CANH, 8 STB | driver enable `{"pin": "STB", "active": 0}`; receiver enable the same with `"disabled": "high"`; domains [VIO: TXD RXD STB], [VCC: CANH CANL]; `vcc` [1.7, 5.5], `vccB` [4.5, 5.5] |
| `test-485` | `TEST485` (`failSafe` true), `TEST485B` (a second device `test-485b`, `failSafe` false) | 1 RO out, 2 RE in, 3 DE in, 4 DI in, 5 GND, 6 A io, 7 B io, 8 VCC | driver DI → [A, B] enable `{"pin": "DE", "active": 1}`; receiver [A, B] → RO enable `{"pin": "RE", "active": 0}`; delays 30 / 40; `dataRate` 10000000; `vcc` [4.75, 5.25] |
| `test-488` | `TEST488` | 1 VCC, 2 RO, 3 DI, 4 GND, 5 Y out, 6 Z out, 7 B in, 8 A in | driver DI → [Y, Z]; receiver [A, B] → RO; `failSafe` true |
| `test-232` | `TEST232` | the 16-pin MAX3232 order (1 C1+ … 16 VCC; 11 T1IN, 14 T1OUT, 13 R1IN, 12 R1OUT, 10 T2IN, 7 T2OUT, 8 R2IN, 9 R2OUT) | two drivers, two receivers; `analog` C1+ C1- C2+ C2- V+ V-; delays 300 / 300; `dataRate` 250000; `vcc` [3, 5.5] |
| `test-lin` | `TESTLIN` | 1 RXD out, 2 SLP_N in, 3 WAKE_N in, 4 TXD in, 5 GND, 6 LIN io, 7 VBAT, 8 INH out | driver TXD → [LIN] enable `{"pin": "SLP_N", "active": 1}`; receiver [LIN] → RXD, the same enable, `"disabled": "z"`, `"outputStyle": "open-drain"`; `analog` INH WAKE_N; delays 2000 / 6000; `dataRate` 20000; `vcc` [5, 27] |

Each circuit uses the library's supplies (`5V`, `3V3`, `12V`, `GND`,
`PULLUP-5V`) for fixed levels. A case reads nets after
`circuit_advance(c, 10 µs)` unless it says otherwise.

- (a) Facts:
  - `TESTCAN`: kind `PART_TRANSCEIVER`, `simulated` 1, 8 pins, protocol
    "can", 1 driver, 1 receiver, duplex 1, `driver_delay_ps` 50000,
    `receiver_delay_ps` 60000, `data_rate_bps` 1000000, `domain_count` 0;
  - `TESTCANV`: `domain_count` 2;
  - `TEST488`: duplex 2, `fail_safe` 1;
  - `TEST485B`: `fail_safe` 0;
  - `TEST232`: 2 drivers and 2 receivers, duplex 0;
  - `TESTLIN`: duplex 1.
- (b) Rejections, one device each, with the § 3.1–3.2 messages:
  - `"protocol": "usb"`;
  - a CAN driver with 1 output;
  - an RS-485 device without `failSafe`;
  - `failSafe` on a CAN device;
  - no drivers and no receivers;
  - 9 drivers;
  - a driver but no `driverDelay`;
  - `receiverDelay` without a receiver;
  - `"disabled": "low"`;
  - TXD with role `output`;
  - an `output`-role pin named nowhere;
  - A named as an output by two drivers;
  - no `ground` pin;
  - `vccB` without `domains`.
- (c) CAN pair: two `TESTCAN` share CANH and CANL; both S on `GND`, VCC on
  `5V`.
  - A's TXD on `GND`: CANH 1, CANL 0, both RXD 0.
  - A's TXD on `5V`: CANH and CANL Z, both RXD 1.
  - A's TXD unconnected: both RXD 1.
- (d) Both TXD on `GND`: CANH 1, CANL 0, `circuit_shorted_net` 0.
- (e) A's S on `5V` (silent), A's TXD on `GND`, B's TXD on `GND`: the bus
  is dominant from B alone, and A's RXD is 0 (its receiver stays on).
  With B's TXD on `5V`: the bus is Z and both RXD are 1.
- (f) `TESTCANV` with STB on the VIO side's `3V3`: RXD 1 (standby holds it
  high) while another `TESTCAN` drives dominant. With STB on `GND`: RXD 0.
- (g) RS-485 pair: `TEST485` A (DE `5V`, RE `5V`) and `TEST485` B (DE
  `GND`, RE `GND`).
  - DI on `5V`: A 1, B 0, B's RO 1, A's RO Z.
  - DI on `GND`: B's RO 0.
  - A's DE on `GND`: the bus is Z and B's RO is 1. The same with a
    `TEST485B` as receiver: RO X.
- (h) Contention: two `TEST485` with DE on `5V`, DI on `5V` and `GND`:
  `circuit_shorted_net` returns 1.
- (i) `TEST488`: DI on `GND` drives Y 0 and Z 1. A second `TEST488`'s A/B
  wired to Y/Z reads RO 0.
- (j) RS-232: two `TEST232`, A's T1OUT to B's R1IN.
  - A's T1IN on `5V`: T1OUT 0, B's R1OUT 1.
  - A's T1IN on `GND`: B's R1OUT 0.
  - B's R2IN unconnected: R2OUT 1.
- (k) LIN pair: two `TESTLIN`, SLP_N on `5V`, VBAT on `12V`, each RXD
  through `PULLUP-5V`.
  - A's TXD on `GND`: LIN 0 and both RXD 0.
  - A's TXD on `5V`: LIN reads 1 from the weak drives, and both RXD 1.
  - B's SLP_N on `GND` with A's TXD on `GND`: B's RXD 1 (released, pulled
    up), A's RXD 0.
- (l) Voltage:
  - `TESTCAN` VCC on `12V`: `circuit_start` → -1, `VOLTAGE_POWER_RANGE` on
    pin 3.
  - CANH on a net with `12V` and VCC on `5V`: starts (bus pins analog).
  - `TESTCANV` VIO on `3V3`, VCC on `5V`, TXD on `5V`:
    `VOLTAGE_INPUT_OVER` on pin 1.
  - The same with TXD on `3V3`: starts.
- (m) Delays: in (c), A's TXD is on `PULLUP-5V` and position 1 of a
  `TE-GDS0204` DIP switch (the library part `switch_test` uses) to `GND`.
  After `circuit_set_switch(…, 1, 1)`, B's RXD is still 1 at 100 ns and is
  0 at 120 ns (50 + 60 ns).
- (n) End to end: two `MCP2200-I/SO` (9600 8N1; `circuit_serial_configure`
  on both), bridge 1 TX → `TESTCAN` A TXD, `TESTCAN` B RXD → bridge 2 RX, A
  and B on one CANH / CANL pair. `circuit_serial_send` of `0x55 0xA3` to
  bridge 1; after 3 ms `circuit_serial_sent_count` of bridge 2 is 2 and the
  frames are 0x55, 0xA3.
- (o) The same as (n) with two `TEST485` (A DE on `5V`, B RE on `GND`),
  two `TEST232`, and two `TESTLIN` (RXD pulled up): bridge 2 gets 0x55,
  0xA3 each time.
- (p) Hooks:
  - `part_pin_analog` is 1 for `TESTCAN` pins 6 and 7 and 0 for 1, 4, 8;
  - `TEST232` pins 1–6 are analog;
  - `part_pin_open_drain` is 1 for `TESTLIN` pin 1;
  - `part_pin_tristate` is 1 for `TEST485` pin 1 and 0 for `TESTCAN` pin 4.

## Validation Plan

- Phase 1: `transceiver_test` (§ 5) and the suite, with the goldens
  unchanged (no part file yet), plus plan-68's size and coverage checks.
- Final gate: plan-71-J.

## Decisions

- Author: **one kind for four protocols**, with `protocol` choosing the
  line logic. The pin rules, enables, delays and facts are the same for all
  four, and the app lists them together. Rejected: four kinds (four copies
  of the loader).
- Author: **logic levels on the line**, the line pins analog. The engine
  has no voltage, and the voltage check would otherwise judge RS-232's
  ±5.5 V and LIN's VBAT-level bus against the logic supply. Rejected:
  modeling line voltages (no analog engine).
- Author: **RS-485 reading = MAX3140's**, so the two kinds interoperate on
  one bus.
- Author: **share the domain code rather than copy it** (plan-75, a
  prerequisite, moves it for this kind, `isolator` and `buffer`). Two copies
  of the same rules would drift.
- Owner, 2026-11-25: **backend letters wait for plan-68** and meet its size
  and coverage checks (§ Prerequisites, Phase 1's acceptance).
- Author: **CAN and LIN loop back** (a receiver reads the bus pins
  including its own drive), as the parts do. A bridge's own RX then sees its
  echo. Case (n) reads bridge 2 only.

## Corrections

(none yet)
