# plan-24-D: The `converter` kind — SPI/I2C engine, frames, registers, commands

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-C

D adds the part kind `converter` (`PART_CONVERTER`) and the engine that runs
a device's declarative digital side over the plan-12 SPI and I2C target
front ends, with `converter_test`.

Outcome of D: `ctest` T0 + 1 (`converter_test`); goldens unchanged.

References: plan-24-A §3; `backend/src/models/serial_target.h` (SerialSpi,
SerialI2c, SerialHost, the `"bus"` object), `serial_max310x.c` /
`ns16550_bridge.c` (models that sit on the front ends), `serial.c` (how a kind
owns a `SerialHost`), `backend/src/library/kind.h`, `kinds.c`,
`backend/include/parts.h:25-27`, `docs/parts-format.md` § `serial` (`bus`).

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-C complete | `ls planning/completed/plan-24-C-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p24` → T0 + 1 of T0 + 1; `converter_test` all PASS; golden diff empty.

### Non-goals

- No analog values (plan-24-A). No change to the serial kind's behavior; the
  front ends are reused as they are (moved behind a shared header only if
  their current header is serial-private — it is `serial_target.h` in
  `models/`, already shared).

## 2. Kind

`backend/src/models/converter.c` + `converter.h`: kind `"converter"`, device
keys `bus`, `frame`, `registers`, `commands`, `conversion`, `ready`,
`dac`, `codeFormat`, `bits`, `channels`; part keys `vcc`, `inputMax`.
`Part.serial`-style public facts: `PartConverter { const char *type /* "adc"
| "dac" */; unsigned bits, channels; }` in `parts.h` for the app (I).
`PART_CONVERTER` appended to `PartKind` (after any value other plans
appended).

## 3. Device schema

| Key | Value |
|---|---|
| `"bus"` | the serial kind's `bus` object: `{"spi": {cs, sclk, mosi, miso, mode}}`, `{"i2c": {scl, sda, address, straps}}`, or `{"parallel": …}` (E) |
| `"frame"` | simple SPI ADCs with no registers (ADS7042-style): `{"start": "cs-fall", "leading": n, "bits": n, "trailing": n, "edge": "fall"\|"rise", "input": [<bit fields MOSI carries, e.g. channel select>]}`: CS falling starts the conversion, the result shifts out MSB first after `leading` zero bits |
| `"registers"` | array `{name, address, width, reset, readOnly (mask), writeOnly (mask), fields: [{name, bits: [hi, lo]}]}` |
| `"commands"` | SPI register parts: array `{name, opcode, mask, action: "wreg"\|"rreg"\|"rdata"\|"start"\|"stop"\|"reset"\|"standby"\|"wakeup"\|"nop", addressField, countField}`; I2C parts use the pointer-register form `{"pointer": {width}}` (ADS1115: pointer byte then 2 data bytes) |
| `"conversion"` | `{"time": ns or a table keyed by a register field (data rate), "mode": "single"\|"continuous" or a register field, "start": register field / command / pin}` |
| `"ready"` | optional `{"pin", "active", "pulse": ns or "until-read"}` (DRDY, BUSY, EOC, ALERT/RDY) |
| `"codeFormat"` | `"straight"`, `"twos"`, `"offset"` (plan-24-A §3.3) |
| `"dac"` | DACs: `{"input": register(s), "dac": register(s), "ldac": {"pin","active"} or "sync", "clear": {"pin","active","code"}, "update": [commands that copy input → dac]}` |
| `"channels"`, `"bits"` | counts; result data register(s) hold the §3.3 code |

Engine: the front end delivers bytes; the engine decodes commands/pointer
writes against `commands`/`registers`, applies masks, schedules conversions
(`sim_schedule` after `conversion.time`), raises `ready`, and serves result
reads. Register writes to fields named in `conversion` reconfigure timing.

## 4. Tests — `backend/tests/converter_test.c`

Fixtures in the schema, each the shape of a real device in `specs.md`:

| Fixture | Cases |
|---|---|
| ADS7042-shape frame ADC | CS fall → after `leading` zeros 12 result bits = 0; MISO Z with CS high |
| ADS1115-shape I2C | write config via pointer 1, read back; start single-shot (OS bit), ALERT/RDY asserts after the data-rate time, conversion register reads 0; wrong address NACK |
| ADS1256-shape SPI commands | RESET, WREG/RREG of 3 registers, SYNC/WAKEUP start, DRDY low after the data-rate time, RDATA returns 24 zero bits |
| bipolar offset-binary ADC | result = `0x8000` for 16 bits |
| DAC8562-shape SPI DAC | write input register, read back where supported, LDAC pin copies to DAC register, software update command, CLR resets to its code |
| DAC5571-shape I2C DAC | write and read back |
| X/abort | CS rises mid-frame: frame discarded, state unchanged |

`library_check`: failure cases for schema errors (unknown action, field
outside its register width, overlapping registers).

## Phases

> **Status discipline:** as plan-24-A.

### Phase 1 — Kind, SPI frames, I2C pointer form

- [ ] `PART_CONVERTER`, `PartConverter`; `KINDS[]`; `converter.c/.h`; CMake lines; frame and I2C cases; test entry; docs § `converter`.

Acceptance: `ctest -R converter_test` passes (seconds).
Commit: —

### Phase 2 — Commands, conversion timing, DAC registers

- [ ] Command engine, `conversion`, `ready`, `dac`; the remaining §4 cases; `library_check` cases.

Acceptance: `ctest --test-dir /tmp/logic-p24` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: —

## Validation Plan

D's gate: Phase 2. Landing tier at the end of I.

## Decisions

- One declarative engine (plan-24-A §3.2).
- Reuse the plan-12 front ends unchanged (they already implement the pin
  protocols and their X rules).

## Corrections

## Summary

D is the core of plan-24: about 1500 of the in-scope parts run on it.
