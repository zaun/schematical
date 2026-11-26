# plan-76-L: Backend: the `sensor` kind, `sensor_test`, docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-76-K

L adds the `sensor` kind: board-mount sensor ICs, facts only, `"simulated": false`. No part file
changes here; M adds the parts.

References:

- `docs/backend.md` § Adding a kind (line 802); `docs/parts-format.md` § `mcu` (the
  `"simulated": false` rule), § `generic`, § Supply voltage, § The drawer's categories (line 121).
- `backend/src/models/generic.c` (the passive, refused shape L copies: `circuit_add_part` refuses
  the part, `backend/src/circuit.c:146-149`); `backend/src/library/kind.h:28-54` (`PartKindOps`);
  `backend/src/models/opamp.c` (the newest facts-struct kind).
- Precedent letters: `planning/plan-74-E-ethernet-kind-rj45.md` § 3.1 (a facts-only kind's keys,
  struct and listing line); `planning/completed/plan-48-F-opamp-kind.md`.

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open; `cat planning/plan-68-*.md \| grep -c -- '- \[ \]'` → 126 unticked boxes; no plan-68 letter in `planning/completed/`) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-76-K complete | `ls planning/completed/plan-76-K-*.md` → exists, or every K box ticked | NOT MET (2026-11-25: K not started) |
| The kind is absent | `grep -c 'PART_SENSOR' backend/include/parts.h` → 0; `ls backend/src/models/sensor.c` → no such file | MET (2026-11-25: 0; no file) |
| T and goldens at L's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. L records its own T1 and P1 (plans 69–75 add kinds and tests first) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A `sensor.json` file loads: kind `PART_SENSOR`, `Part.sensor` facts, `simulated` 0, the
  rejections of § 5. `circuit_add_part` refuses such a part (`part "X" is not simulated`).
- The listing prints each part's facts (§ 3.3), so the goldens carry every fact M ships.
- `ctest` passes T1 + 1 programs (`sensor_test`). The goldens are unchanged: no shipped part uses
  the kind yet. `Parts/sensor.json` ships with `"devices": []` (the plan-72-D precedent), so the
  app's bundle and `BundledPartFile` have a file for the kind from the start.

### Non-goals (explicit constraints)

- No simulation of any sensor: no register file, no analog output, no value set in the app.
- No change to `circuit.c`, the voltage check or the ERC beyond the shared kind lines.
- No app change (N).

## 2. Current State

- `PartKind` ends with `PART_TIMER, PART_OPAMP` (`backend/include/parts.h:32`). `KINDS[]` ends
  with `&OPAMP_KIND` (`backend/src/library/kinds.c:106`). `TRACE_LAST_KIND` is `PART_OPAMP`
  (`backend/tests/trace_check.c:22`). `listing_check.c:50-51` sizes two arrays by
  `PART_OPAMP + 1`. `backend/tests/support/listing.c` names each kind (line 57) and prints its
  facts (line 326); a `"simulated": false` part's line ends with `|unsimulated` (line 566).
  Plans 69–75 add six kinds after `PART_OPAMP` before or alongside this letter; § 3.4.
- `generic.c`: `load_device` demands `"simulated": false`, and the engine model is passive
  (`create` returns the `Part`; no callbacks).
- `mcu.c` checks `"simulated"` with the message `an mcu has no simulation model: "simulated"
  must be false`; `sensor.c` uses the same wording.

## 3. Design

### 3.1 Device keys (`backend/src/models/sensor.c`)

| Key | Value |
|---|---|
| `"simulated"` | required, `false` (`a sensor has no simulation model: "simulated" must be false`) |
| `"family"` | `"temperature"`, `"humidity"`, `"pressure"`, `"motion"`, `"magnetic"`, `"light"` or `"current"` (`unknown sensor family "X"`) |
| `"measures"` | non-empty array of distinct quantities (`unknown sensor quantity "X"`, `quantity "X" is named twice`), each allowed for the family (`a <family> sensor cannot measure "X"`), and including the family's own (`a <family> sensor must measure <q>`): see the table below |
| `"output"` | non-empty array of distinct outputs from `analog`, `bridge`, `capacitance`, `resistance`, `i2c`, `spi`, `i3c`, `1-wire`, `pwm`, `sent`, `uart`, `frequency`, `switch` (`unknown sensor output "X"`, `output "X" is named twice`). `i2c` covers SMBus; `analog` covers voltage and current outputs; `bridge` is a Wheatstone bridge (mV/V); `capacitance` / `resistance` are passive sensing elements |
| `"ranges"` | optional object: a measured quantity → `[min, max]` in the quantity's unit, min < max (`"ranges" names "X", which the sensor does not measure`, `the range of "X" must be [min, max] with min below max`, `"ranges" has no unit for "X"`) |
| `"accuracy"` | optional object: a measured quantity → a number above 0, ± in the quantity's unit (`the accuracy of "X" must be above 0`; same naming rule as `"ranges"`) |
| `"axes"` | optional, 1, 2 or 3: sensing axes of a motion or magnetic sensor (`"axes" is only for motion and magnetic sensors`, `"axes" must be 1, 2 or 3`) |
| `"pressureType"` | required for a pressure sensor, else absent: `"absolute"`, `"gauge"` or `"differential"` (`a pressure sensor needs "pressureType"`, `"pressureType" must be absolute, gauge or differential`, `"pressureType" is only for pressure sensors`) |
| `"switchType"` | required for a magnetic sensor with output `switch`, else absent: `"unipolar"`, `"omnipolar"`, `"bipolar"` or `"latch"` (`a magnetic switch needs "switchType"`, `"switchType" must be unipolar, omnipolar, bipolar or latch`, `"switchType" is only for magnetic switches`) |
| `"isolated"` | optional boolean, default false, current sensors only (`"isolated" is only for current sensors`): the measured path is galvanically isolated from the output (Hall and fluxgate parts) |
| `"address"` | optional array of distinct 7-bit I2C addresses, each 0x08–0x77 (JSON integers), only with output `i2c` (`"address" needs output "i2c"`, `I2C address N is out of range`) |
| `"vcc"`, `"inputMax"` | the supply range, as every kind (§ Supply voltage); required on the device or every part, except for a part whose outputs are only `bridge`, `capacitance` and `resistance` |

Quantities and their units (the unit of `"ranges"` and `"accuracy"`):

| Quantity | Unit | Allowed in families |
|---|---|---|
| `temperature` | °C | all seven (the family's own in `temperature`) |
| `humidity` | %RH | humidity (own) |
| `pressure` | kPa | pressure (own), motion (a 10-axis IMU) |
| `acceleration` | g | motion (own: acceleration or rotation) |
| `rotation` | °/s | motion |
| `magnetic-field` | mT | magnetic (own), motion |
| `light` | lux | light (own: light, infrared or ultraviolet) |
| `infrared` | — (no range) | light |
| `ultraviolet` | — (no range) | light |
| `proximity` | — (no range) | light |
| `current` | A | current (own) |

Part keys: `"vcc"`, `"inputMax"`, `"ranges"`, `"accuracy"`, `"address"`. A part's `"ranges"`,
`"accuracy"` or `"address"` replaces the device's whole object (a Honeywell device's parts differ
by pressure range; an ACS712 device's by current range).

Pins: at least one `power` and one `ground` pin (`a sensor needs a power pin and a ground pin`),
except a device whose outputs are only `bridge`, `capacitance` and `resistance`, which needs two
or more pins (`a sensor needs two or more pins`) (a bridge's excitation pins are `power` / `ground`; a capacitive humidity element's
two pins are `inout`). No key names a pin.

### 3.2 Facts struct (`backend/include/parts.h`)

```c
/* A sensor IC (plan-76-L): facts only, "simulated": false. Values are stored × 1000 in the
   quantity's unit (°C, %RH, kPa, g, °/s, mT, lux, A): range_milli[0..1], accuracy_milli. */
enum { SENSOR_TEMPERATURE, SENSOR_HUMIDITY, SENSOR_PRESSURE, SENSOR_ACCELERATION, SENSOR_ROTATION,
       SENSOR_MAGNETIC_FIELD, SENSOR_LIGHT, SENSOR_INFRARED, SENSOR_ULTRAVIOLET, SENSOR_PROXIMITY,
       SENSOR_CURRENT, SENSOR_QUANTITY_COUNT };
enum { SENSOR_OUT_ANALOG = 1u << 0, SENSOR_OUT_BRIDGE = 1u << 1, SENSOR_OUT_CAPACITANCE = 1u << 2,
       SENSOR_OUT_RESISTANCE = 1u << 3, SENSOR_OUT_I2C = 1u << 4, SENSOR_OUT_SPI = 1u << 5,
       SENSOR_OUT_I3C = 1u << 6, SENSOR_OUT_ONEWIRE = 1u << 7, SENSOR_OUT_PWM = 1u << 8,
       SENSOR_OUT_SENT = 1u << 9, SENSOR_OUT_UART = 1u << 10, SENSOR_OUT_FREQUENCY = 1u << 11,
       SENSOR_OUT_SWITCH = 1u << 12 };
typedef struct {
    uint8_t quantity;          /* SENSOR_* */
    uint8_t has_range, has_accuracy;
    int64_t range_milli[2];
    int64_t accuracy_milli;
} SensorQuantity;
typedef struct {
    const char *family;
    uint32_t output;           /* SENSOR_OUT_* bits */
    size_t quantity_count;
    SensorQuantity quantities[SENSOR_QUANTITY_COUNT];   /* "measures" order */
    uint8_t axes;              /* 0 = not given */
    char pressure_type;        /* 'A', 'G', 'D', or 0 */
    char switch_type;          /* 'U', 'O', 'B', 'L', or 0 */
    uint8_t isolated;
    size_t address_count;
    const uint8_t *addresses;
} PartSensor;
```

`const PartSensor *sensor;` on `Part`. Each part gets its own `PartSensor` (its ranges, accuracy
and addresses may differ from the device's). Engine model: `generic.c`'s passive shape.

### 3.3 Listing facts (`backend/tests/support/listing.c`)

`family=<f> measures=<q,…> output=<o,…, bit order>` then, only when given, `range=<q>:<min>..<max>`
per quantity (`;`-separated, values with up to 3 decimals, trailing zeros dropped),
`accuracy=<q>:<v>` likewise, `axes=<n>`, `pressure=absolute|gauge|differential`,
`switch=unipolar|omnipolar|bipolar|latch`, `isolated=1`, `address=0x48,0x49`. Example:
`family=temperature measures=temperature output=i2c range=temperature:-55..150 accuracy=temperature:0.1 address=0x48,0x49,0x4a,0x4b`.

### 3.4 Shared lines (each only if absent, after whatever is last; grep first)

- `parts.h`: `PART_SENSOR` after the current last `PartKind`.
- `kinds.c`: the `extern` and `&SENSOR_KIND` at the end of `KINDS[]`.
- `backend/CMakeLists.txt`: `src/models/sensor.c` and `logic_test(sensor_test)`.
- `listing.c`: `case PART_SENSOR: return "sensor"` and the facts line.
- `trace_check.c:22` `TRACE_LAST_KIND` and `listing_check.c:50-51`: the last `PartKind` entry in
  `parts.h` after `PART_SENSOR` is added (read the enum at this step).
- `erc_pins.c` `kind_is_passive`: not touched (an unsimulated part's pins are checked by role like
  any IC's).

## 4. Docs

- `docs/parts-format.md`:
  - a `### sensor` section after the last kind section: keys, units table, pin rule,
    `"simulated": false`, an example `TMP117` device;
  - § The drawer's categories: a row **Sensors** — Temperature; Humidity; Pressure; Motion;
    Magnetic; Light; Current (plan-76). The reason for a new top-level category: none of the
    existing ones fits an input transducer (Analog is converters, timers and op-amps).
- `docs/backend.md` § Layout: `sensor.c`.
- `schematical/schematical/Parts/sensor.json`: `{"format": …, "version": …, "kind": "sensor",
  "devices": []}` with the header fields of `opamp.json`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The kind

- [ ] `backend/include/parts.h`, `backend/src/models/sensor.c` per § 3.1–3.2.
- [ ] The shared lines of § 3.4 (grep each first; record the greps here).
- [ ] `listing.c` facts per § 3.3.
- [ ] `backend/tests/support/sensor_fixture.h`, `backend/tests/sensor_test.c`: § 5 (a)–(l).
- [ ] `Parts/sensor.json` with no devices.

Acceptance: the kind loads, reports its facts, and its parts are refused by a circuit; the goldens
do not move.
  Check: `cmake --build /tmp/logic-p76 -j8 && /tmp/logic-p76/sensor_test` → `sensor_test: all passed`; `ctest --test-dir /tmp/logic-p76 -j8` → `100% tests passed`, T1 + 1 programs; `git diff --stat backend/tests/golden` → empty (est. 5 min).
  Result:
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check`
  → exit 0 and `node scripts/coverage.js --check --backend` → exit 0, so no new or
  changed backend file is over 1000 code lines or has an uncovered line (est. 6 min:
  one instrumented backend build with `ctest`, the only tool that shows every new
  line runs).
Commit:

### Phase 2 — Docs

- [ ] § 4.

Acceptance: the docs name the kind and the category.
  Check: `grep -c '^### `sensor`' docs/parts-format.md` → 1; `grep -c '^| Sensors |' docs/parts-format.md` → 1; `grep -c 'sensor.c' docs/backend.md` → above 0 (est. < 1 min).
  Result:
Commit:

## 5. Test cases

`sensor_fixture.h`:
- `TESTTEMP1`: a temperature device, 6 pins (`V+` power, `GND` ground, `SCL` input, `SDA` inout,
  `ALERT` output, `ADD0` input), `"measures": ["temperature"]`, `"output": ["i2c"]`,
  `"ranges": {"temperature": [-55, 150]}`, `"accuracy": {"temperature": 0.1}`,
  `"address": [72, 73, 74, 75]`, `"vcc": [1.7, 5.5]`; footprint `SON-6-DRY`.
- `TESTP1`: a pressure device, 4 pins (`+EXC` power, `-EXC` ground, `+OUT` output, `-OUT` output),
  `"output": ["bridge"]`, `"pressureType": "gauge"`; two parts, `TESTP1-005` with
  `"ranges": {"pressure": [0, 34.474]}` and `TESTP1-030` with `[0, 206.843]`, no `vcc`.
- `TESTHALL1`: magnetic, 3 pins, `"output": ["switch"]`, `"switchType": "omnipolar"`, `"axes": 1`.

- (a) `TESTTEMP1`: kind `PART_SENSOR`, `simulated` 0, family "temperature", `output`
  `SENSOR_OUT_I2C`, one quantity `SENSOR_TEMPERATURE` with range −55000..150000 and accuracy 100,
  4 addresses 0x48–0x4B, `vcc` 1700–5500 mV.
- (b) `TESTP1-005` and `TESTP1-030`: ranges 0..34474 and 0..206843 (the part's object replaces
  the device's), `pressure_type` 'G', no `vcc` and no error.
- (c) `circuit_add_part(c, "TESTTEMP1", …)` → -1, error `part "TESTTEMP1" is not simulated`.
- (d) `"simulated": true` → the `"simulated" must be false` error; `"family": "gas"` →
  `unknown sensor family "gas"`.
- (e) `"measures": ["humidity"]` on a temperature device → `a temperature sensor cannot measure
  "humidity"`; a humidity device with `["temperature"]` → `a humidity sensor must measure humidity`;
  `["temperature", "temperature"]` → `quantity "temperature" is named twice`.
- (f) `"output": ["can"]` → `unknown sensor output "can"`.
- (g) `"ranges": {"humidity": [0, 100]}` on a temperature-only device → `"ranges" names
  "humidity", which the sensor does not measure`; `[125, -40]` → `the range of "temperature" must
  be [min, max] with min below max`; a light device's `"ranges": {"infrared": [0, 1]}` →
  `"ranges" has no unit for "infrared"`.
- (h) `"accuracy": {"temperature": 0}` → `the accuracy of "temperature" must be above 0`.
- (i) `"axes": 3` on a temperature device → `"axes" is only for motion and magnetic sensors`;
  `"axes": 4` on a motion device → `"axes" must be 1, 2 or 3`.
- (j) a pressure device without `"pressureType"` → `a pressure sensor needs "pressureType"`;
  `TESTHALL1` without `"switchType"` → `a magnetic switch needs "switchType"`; `"switchType"` on a
  magnetic device with output `["analog"]` → `"switchType" is only for magnetic switches`;
  `"isolated": true` on a temperature device → `"isolated" is only for current sensors`.
- (k) `"address": [72]` with output `["analog"]` → `"address" needs output "i2c"`; `[7]` →
  `I2C address 7 is out of range`.
- (l) `TESTTEMP1` with no `ground` pin → `a sensor needs a power pin and a ground pin`; `TESTP1`
  with only one pin → `a sensor needs two or more pins`; `TESTTEMP1` with no `vcc` anywhere →
  the existing `missing "vcc"` error.

## Validation Plan

- `sensor_test`; `listing_check`, `trace_check`, `footprint_check`, `erc_pins_check` with
  unchanged goldens.
- Coverage check: `ctest --test-dir /tmp/logic-p76 -N | grep -c sensor_test` → 1.
- Final gate: plan-76-O.

## Decisions

- Owner, 2026-11-25: sensors are facts only, `"simulated": false`. A placed one turns Run off, as
  an `mcu` without a model does.
- Author: **one kind for all seven families**, with `"family"` as its variant key: the facts
  share one shape (quantities with ranges, outputs, supply), and one kind gives the Part Editor one
  schema.
- Author: **values stored × 1000 in a fixed unit per quantity**, so every range is an integer in
  the goldens and the app converts once per quantity.
- Author: **a part may replace ranges, accuracy and addresses**: one datasheet's parts commonly
  differ only by those (pressure ranges, current ranges, address options), and a device per range
  would duplicate every pinout.
- Author: **passive elements need no supply**: a Wheatstone bridge's excitation and a capacitive
  humidity element have no rated supply range in their datasheets, only an excitation or
  measuring condition.

## Corrections

## Summary

A facts-only `sensor` kind on the `generic` shape, with quantities, ranges, accuracy, outputs and
family-specific facts, its own test program, and an empty part file for M to fill.
