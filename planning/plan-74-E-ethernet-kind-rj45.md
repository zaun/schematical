# plan-74-E: Backend: the `ethernet` kind, the connector's `"rj45"` mode, tests, docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-74-D

E adds two loaders:

- the `ethernet` kind: Ethernet PHYs and controllers, facts only, `"simulated": false`;
- a fifth `connector` mode, `"rj45"`: an RJ45 jack, with or without magnetics, whose pins are
  passive terminals like a USB connector's.

No part file changes here. F and G–I add the parts.

References:

- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § `generic`, § `connector`, § `opamp`
  (the facts-struct precedent).
- `backend/src/models/generic.c` (the `"simulated": false` shape E copies: a passive engine model,
  `circuit_add_part` refuses the part); `backend/src/models/connector.c` (the modes); 
  `backend/tests/connector_test.c` (the USB fixture's shape).
- `planning/completed/plan-48-F-opamp-kind.md` (the letter's precedent).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-74-D complete | `ls planning/completed/plan-74-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| The kind and mode are absent | `grep -c 'PART_ETHERNET' backend/include/parts.h` → 0; `ls backend/src/models/ethernet.c` → no such file; `grep -c 'rj45' backend/src/models/connector.c` → 0 | MET (2026-11-25: 0; no file; 0) |
| T and goldens at E's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. E records its own T1 and P1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ethernet.json` files load: kind `PART_ETHERNET`, `Part.ethernet` facts, `simulated` 0, the
  rejections of § 5. `circuit_add_part` refuses such a part (`part "X" is not simulated`).
- A connector device with `"rj45"` loads: `PartConnector.rj45` `'F'` (10/100) or `'G'` (1000),
  `magnetics`, `poe`. The other modes' keys are rejected on it, and `"rj45"` is rejected on them
  (§ 3.2). In a circuit, two of its pins on two driven nets join nothing, as for USB.
- `ctest` passes T1 + 1 programs (`ethernet_test`), with `connector_test`'s new cases. The goldens are
  unchanged: no shipped part uses either yet.

### Non-goals (explicit constraints)

- No simulation of any Ethernet IC: no MDIO, no SPI register file, no link.
- A MagJack's LEDs are terminals, not `led` parts: they do not light in play mode.
- No change to the other connector modes, to `circuit.c`, or to the voltage check.
- No part file and no app change (F–J).

## 2. Current State

- `circuit_add_part` refuses a part whose device says `"simulated": false`
  (`backend/src/circuit.c:146-149`). The app turns Run off while one is placed
  (`Simulation/SimulationAvailability.swift`).
- `generic.c` is that shape: `load_device` demands `"simulated": false`, `bind_pinout` needs one
  pin, and the engine model is passive (`create` returns the `Part`; no callbacks).
- `connector.c` checks a device's mode by its keys (`DEVICE_KEYS`, line 17: `centerPin`, `ratedVolts`,
  `ratedAmps`, `contacts`, `usb`, `card`, `cardPitch`, `testPoint`, `terminal`, `jst`, `pitch`).
  The modes exclude each other (lines 147–170: `a connector is one of "usb", "terminal" and "jst"`).
  `PartConnector` (`backend/include/parts.h:184`) ends with `pitch_um`, `terminal`, `jst`.
- `PartKind` ends with `PART_TIMER, PART_OPAMP` (`parts.h:32`). `KINDS[]` ends with `&OPAMP_KIND`
  (`backend/src/library/kinds.c:106`). `TRACE_LAST_KIND` is `PART_OPAMP`
  (`backend/tests/trace_check.c:22`). `listing_check.c:50-51` sizes two arrays by `PART_OPAMP + 1`.
  `backend/tests/support/listing.c` names each kind (line 57) and prints its facts (line 326).
- Listing line of a USB connector (`backend/tests/golden/parts.txt`): `…|USB-A Port|||usb=A
  rated=30V/1800mA|1:VBUS:inout:default,…`. A `"simulated": false` part's line ends with
  `|unsimulated` (`listing.c:566`).

## 3. Design

### 3.1 The `ethernet` kind (`backend/src/models/ethernet.c`)

Device keys:

| Key | Value |
|---|---|
| `"simulated"` | required, `false` (`an Ethernet part has no simulation model: "simulated" must be false`) |
| `"type"` | `"phy"` or `"controller"` (`unknown Ethernet type "X"`) |
| `"speed"` | 10, 100 or 1000: the highest twisted-pair rate in Mbit/s (`"speed" must be 10, 100 or 1000`) |
| `"host"` | non-empty array of distinct strings from `mii`, `rmii`, `rgmii`, `gmii`, `sgmii`, `spi`, `parallel` (`unknown host interface "X"`, `host interface "X" is named twice`). A `phy` takes only the first five, and a `controller` only `spi` and `parallel` (`a PHY's host interface is MII, RMII, RGMII, GMII or SGMII`, `a controller's host interface is "spi" or "parallel"`) |
| `"tcpip"` | optional boolean, default false: a hardwired TCP/IP stack. `controller` only (`"tcpip" is only for a controller`) |
| `"vcc"`, `"inputMax"` | the I/O supply range, as every kind; required on the device or every part (`kind_part_voltage`) |

Part keys: `"vcc"`, `"inputMax"`.

Pins: at least one `power` and one `ground` pin (`an Ethernet part needs a power pin and a ground
pin`). No other pin rule: no key names a pin.

```c
/* An Ethernet PHY or controller (plan-74-E): facts only, "simulated": false.
   type "phy" or "controller"; speed_mbps 10, 100 or 1000; host: ETHERNET_HOST_* bits;
   tcpip 1 for a hardwired TCP/IP stack. */
enum { ETHERNET_HOST_MII = 1u << 0, ETHERNET_HOST_RMII = 1u << 1, ETHERNET_HOST_RGMII = 1u << 2,
       ETHERNET_HOST_GMII = 1u << 3, ETHERNET_HOST_SGMII = 1u << 4, ETHERNET_HOST_SPI = 1u << 5,
       ETHERNET_HOST_PARALLEL = 1u << 6 };
typedef struct { const char *type; uint32_t speed_mbps; uint32_t host; uint8_t tcpip; } PartEthernet;
```

`const PartEthernet *ethernet;` on `Part`. Engine model: `generic.c`'s passive shape.
Listing facts: `type=<t> speed=<n> host=<names, comma, bit order> tcpip=<0|1>`.

### 3.2 The connector's `"rj45"` mode (`connector.c`)

| Key | Value |
|---|---|
| `"rj45"` | `"10/100"` or `"1000"`: the rate the jack's magnetics (or contacts) are rated for (`"rj45" must be "10/100" or "1000"`) |
| `"magnetics"` | optional boolean, default false: integrated magnetics. Only with `"rj45"` (`"magnetics" is only for RJ45 jacks`) |
| `"poe"` | optional boolean, default false: the magnetics pass Power over Ethernet. Only with `"magnetics": true` (`"poe" needs "magnetics"`) |
| `"ratedVolts"`, `"ratedAmps"` | optional, as for USB |

There is no `centerPin`, `contacts` or `pitch` (`an RJ45 jack has no "centerPin"`, …).
`"rj45"` with `usb`, `terminal` or `jst` is `a connector is one of "usb", "terminal", "jst" and
"rj45"` (the existing message, extended). A card slot and a test point reject it with their existing
FORBIDDEN lists, which gain `"rj45"`, `"magnetics"` and `"poe"`.

`PartConnector` gains `char rj45` (`'F'` 10/100, `'G'` 1000, 0 otherwise), `uint8_t magnetics, poe`.
Listing facts: `rj45=10/100 magnetics=1 poe=0` before `rated=` (which is printed only when given,
as for USB). Every pin is a passive `inout` terminal or `nc`, as for USB.

### 3.3 Shared lines (each only if absent, after whatever is last; grep first)

- `parts.h`: `PART_ETHERNET` after the current last `PartKind`.
- `kinds.c`: the `extern` and `&ETHERNET_KIND` at the end of `KINDS[]`.
- `backend/CMakeLists.txt`: `src/models/ethernet.c` and `logic_test(ethernet_test)`.
- `listing.c`: `case PART_ETHERNET: return "ethernet"` and the facts line.
- `trace_check.c:22` `TRACE_LAST_KIND` and `listing_check.c:50-51`: the last `PartKind` entry in
  `parts.h` after `PART_ETHERNET` is added (read the enum at this step).

## 4. Docs

- `docs/parts-format.md`:
  - a `### ethernet` section after `### opamp`, or after the last kind section other plans added:
    keys, pin rule, `"simulated": false`, an example W5500 device;
  - § `connector`: an "**RJ45 jacks** (plan-74)" paragraph with the keys of § 3.2, the pin naming
    rule of plan-74-G § 3.1, and the drawer `["Connector", "RJ45"]`.
- `docs/backend.md` § Layout: `ethernet.c`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The `ethernet` kind

- [ ] `backend/include/parts.h`, `backend/src/models/ethernet.c` per § 3.1.
- [ ] The shared lines of § 3.3 (grep each first; record the greps here).
- [ ] `backend/tests/support/ethernet_fixture.h`, `backend/tests/ethernet_test.c`: § 5 (a)–(f).

Acceptance: the kind loads, reports its facts, and its parts are refused by a circuit.
  Check: `cmake --build /tmp/logic-p74 -j8 && /tmp/logic-p74/ethernet_test` → `ethernet_test: all passed`; `ctest --test-dir /tmp/logic-p74 -R 'listing_check|trace_check|library_check|footprint_check|erc_pins_check'` → `100% tests passed` (est. 4 min: an incremental build).
  Result:
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check`
  → exit 0 and `node scripts/coverage.js --check --backend` → exit 0, so no new or
  changed backend file is over 1000 code lines or has an uncovered line (est. 6 min:
  one instrumented backend build with `ctest`, the only tool that shows every new
  line runs).
Commit:

### Phase 2 — The `"rj45"` mode

- [ ] `connector.c` and `PartConnector` per § 3.2; `listing.c`'s connector facts.
- [ ] `backend/tests/connector_test.c`: § 5 (g)–(j).

Acceptance: an RJ45 jack loads with its facts and stays a passive terminal block. The other modes
are unchanged (goldens unchanged).
  Check: `cmake --build /tmp/logic-p74 -j8 && /tmp/logic-p74/connector_test && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed`, T1 + 1 programs; `git diff --stat backend/tests/golden` → empty (est. 5 min).
  Result:
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check`
  → exit 0 and `node scripts/coverage.js --check --backend` → exit 0, so no new or
  changed backend file is over 1000 code lines or has an uncovered line (est. 6 min:
  one instrumented backend build with `ctest`, the only tool that shows every new
  line runs).
Commit:

### Phase 3 — Docs

- [ ] § 4.

Acceptance: the docs name the kind and the mode.
  Check: `grep -c '^### `ethernet`' docs/parts-format.md` → 1; `grep -c '"rj45"' docs/parts-format.md` → above 0; `grep -c 'ethernet.c' docs/backend.md` → above 0 (est. < 1 min).
  Result:
Commit:

## 5. Test cases

`ethernet_fixture.h`:
- a `controller` device: 8 pins (`VDD` power, `GND` ground, `SCSn` input, `SCLK` input, `MOSI`
  input, `MISO` output, `TXP` inout, `TXN` inout), `"speed": 100`, `"host": ["spi"]`,
  `"tcpip": true`, `"vcc": [2.97, 3.63]`; part `TESTETH1` (footprint `SOIC-8`);
- a `phy` device with `"host": ["mii", "rmii"]`, `"speed": 100`; part `TESTPHY1`.

- (a) `TESTETH1`: kind `PART_ETHERNET`, `simulated` 0, `type` "controller", `speed_mbps` 100,
  `host` `ETHERNET_HOST_SPI`, `tcpip` 1, `vcc` 2970–3630 mV.
- (b) `TESTPHY1`: `type` "phy", `host` MII | RMII, `tcpip` 0.
- (c) `circuit_add_part(c, "TESTETH1", …)` → -1, error `part "TESTETH1" is not simulated`.
- (d) `"simulated": true` → the `"simulated" must be false` error; `"type": "switch"` →
  `unknown Ethernet type "switch"`; `"speed": 2500` → `"speed" must be 10, 100 or 1000`.
- (e) a `phy` with `"host": ["spi"]` → `a PHY's host interface is MII, RMII, RGMII, GMII or SGMII`;
  a `controller` with `["rmii"]` → `a controller's host interface is "spi" or "parallel"`;
  `["spi", "spi"]` → `host interface "spi" is named twice`; `"tcpip": true` on a `phy` →
  `"tcpip" is only for a controller`.
- (f) a pinout with no `ground` pin → `an Ethernet part needs a power pin and a ground pin`.

`connector_test.c` (an `rj45-test` device: 12 pins `TD+`, `TD-`, `TCT`, `RD+`, `RD-`, `RCT`,
`LED1A`, `LED1K`, `LED2A`, `LED2K`, `NC`, `SHIELD`, `"rj45": "10/100"`, `"magnetics": true`;
part `RJ45-TEST`):
- (g) `rj45` `'F'`, `magnetics` 1, `poe` 0, `usb` 0, `pitch_um` 0.
- (h) `"rj45": "2.5G"` → `"rj45" must be "10/100" or "1000"`; `"poe": true` without magnetics →
  `"poe" needs "magnetics"`; `"magnetics": true` on a USB device → `"magnetics" is only for RJ45
  jacks`; `"rj45"` with `"usb"` → `a connector is one of "usb", "terminal", "jst" and "rj45"`;
  `"pitch"` with `"rj45"` → `an RJ45 jack has no "pitch"`.
- (i) a circuit with `5V` on `TD+`'s net and `GND` on `TD-`'s net: `circuit_start` returns 0 and,
  after 1 ms, `circuit_shorted_net` reports neither net (the USB case's shape,
  `connector_test.c:74-87`).
- (j) `"rj45": "1000"` → `'G'`.

## Validation Plan

- `ethernet_test`, `connector_test`; `listing_check`, `trace_check`, `footprint_check`,
  `erc_pins_check` unchanged goldens.
- Coverage check: `ctest --test-dir /tmp/logic-p74 -N | grep -c ethernet_test` → 1.
- Final gate: plan-74-K.

## Decisions

- Owner, 2026-11-25: Ethernet ICs are facts only, `"simulated": false`. A placed one turns Run off,
  as an `mcu` without a model does.
- Author: **a new kind, not `generic`.** `generic` takes no facts (`docs/parts-format.md`
  § `generic`), and the app needs type, speed and host interface for the drawer, the details and the
  kind name.
- Author: **jacks are a `connector` mode, not a kind.** A MagJack is a passive terminal block like
  the USB, terminal and JST connectors: same pin role (`inout`), same drawing (a block of pins), same
  "joins nothing" model. Its own facts are three keys.
- Author: **`"rj45"` names the speed, not just presence.** A 10/100 jack has two pairs of magnetics
  and a 1000 jack four, so the speed is the jack's one fact a designer matches to the PHY.

## Corrections

## Summary

A facts-only kind for Ethernet ICs on the `generic` shape, and a fifth passive connector mode for
RJ45 jacks, each with its own test cases.
