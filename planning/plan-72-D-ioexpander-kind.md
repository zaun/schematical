# plan-72-D: Backend — the converter engine's `io` section, register `next` / `alias`, the `ioexpander` kind

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-C

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
D adds the kind `ioexpander` on the converter engine, the way `leddriver`
sits on it (`backend/src/models/leddriver.c`: `cv_load_device(ctx, json,
CV_MODE_DISPLAY)`), with one new device section, `"io"`: port pins (the
`ports` driver's keys plus polarity, pulls and per-bank open drain), change
interrupts and interrupt output pins. Two register keys every template of
plan-72-B § 3 needs join the converter's register format: `next` and
`alias`. A shared bit-bang I2C/SPI host for tests lands with it.

References:

- `docs/parts-format.md` § `converter` (`bus`, `registers`, `protocol`,
  `pins`, `timing`), § `leddriver` `ports`; `docs/backend.md` § Adding a kind.
- `backend/src/models/converter.h` (`CvMode`, `CvRegister`, `CvField`,
  `CvConds`), `converter.c` (`reg_next`, the pointer increment, register
  reads), `converter_load.c` (or the files plan-68-J split it into:
  `converter_load_proto.c` Registers/Commands, `converter_load_display.c`
  `load_display_ports`), `display_out.c` (`apply_ports`), `display_out.h`
  (`DispPort`), `serial_target.h` (the I2C/SPI target front ends).
- plan-72-B § 3 (the templates and the keys each uses).
- Fixture precedents: `backend/tests/display_out_test.c`, `max6957_test.c`
  (ports), `max6954_max6955_test.c` (its `i2c_*` bit-bang helpers, which the
  shared host replaces for the new tests only).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-72-C complete | `grep -c '^- \[ \]' planning/plan-72-C-ioexp-footprints.md` → 0 | NOT MET (2026-11-25) |
| `ioexpander` absent | `grep -c PART_IOEXPANDER backend/include/parts.h` → 0 | MET (2026-11-25: 0) |
| Last `PartKind` at D's start | `sed -n '/^typedef enum {/,/} PartKind;/p' backend/include/parts.h \| grep -o 'PART_[A-Z0-9]*' \| tail -1` | `PART_OPAMP` at planning (63e2f7d2); plans 69–71, 73, 74 may append theirs first: D appends after whatever is last |
| Golden sizes at D's start | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` | recorded here as P1, R1, F1 (planning: 40832 / 40832 / 3993). D adds no part: all three unchanged at D's end |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `PART_IOEXPANDER`, `IOEXPANDER_KIND` (`backend/src/models/ioexpander.c`),
  `Parts/ioexpander.json` (empty `devices`), its section in
  `docs/parts-format.md`, the drawer row (Interface → I/O Expanders).
- The `"io"` section loads on `ioexpander` (required) and `i2cbus` (plan-72-E)
  devices and is rejected on every other converter-engine kind.
- `backend/tests/ioexpander_test.c` (new program, T0 + 1) runs four fixture
  devices through a test host and passes: a `quasi` 8-port I2C device, a
  `pca9555` I2C device, an `mcp23x17` I2C device, an `mcp23x17` SPI device with
  HAEN (the fixtures are the templates of plan-72-B § 3 with fixture names).
- Every existing golden is unchanged; every new line is covered (plan-68's
  rule: `node scripts/coverage.js --check` exit 0).

### Non-goals

- No part is added (F, G and H add them).
- `leddriver` / `lcddriver` behaviour and goldens are unchanged: the ports
  driver gains keys, and a device without them behaves as before.
- Not modeled: IOCON.BANK = 1 (plan-72-B Decisions), input latch, drive
  strength, glitch filters, I2C clock stretching, quasi-bidirectional
  strong-pull-up pulses, analog input thresholds.

## 2. Current State

- The ports driver (`display_out.c:apply_ports`, `DispPort` in
  `display_out.h`) drives each pin from `dataField`, releases it while
  `dirField` says input, sinks (open drain) or weakly pulls up by `dirField`
  value, and refreshes `readField` with the data bit (output) or the pin's
  level (input; Z and X read 0) on every register or pin change. Its loader
  is `load_display_ports` (`converter_load.c`, `converter_load_display.c`
  after plan-68-J).
- Registers (`CvRegister` in `converter.h`): `name`, `address`, `space`,
  `width`, `reset`, `ro`, `w1c`, `cor` (clear-on-read mask), fields. The I2C
  pointer form (`pointer` command with `increment`) steps to the next higher
  address of the same space (`converter.c:reg_next`); with no higher register
  the pointer stays.
- `CvMode` is `{CV_MODE_CONVERTER, CV_MODE_RTC, CV_MODE_DISPLAY}`
  (`converter.h:403` at 63e2f7d2).
- Conditions (`"REG.FIELD=v"`, arrays = all hold) select commands (`when`),
  enable pins, and gate display drivers; `pins.levels` copies a pin's level
  into a field.

## 3. Design Overview

The expander is a register file on the converter engine plus port pins; the
engine already does the bus, the pointer protocol and the registers, so D adds
only what plan-72-B's templates name and the engine lacks:

1. **Port keys** (on `io.ports`, also accepted by `display.ports`):
   `readInvertField` (Input = pin XOR polarity), `pullEnableField` /
   `pullSelectField` (a pull independent of the direction: MCP GPPU, PCAL pull
   registers), `openDrainField` (an output that drives 0 or Z by a register
   bit: PCAL output configuration).
2. **Change interrupts** (`io.interrupts`, `io.interruptPins`): per-source
   compare (`snapshot` = levels at the last clear, `previous` = edge, `field`
   = per-port compare field picked by a mode field), enable / mask, flags,
   capture, clear events (`read:<REG>`, `write:<REG>`, `read`, `write`), and
   output pins whose polarity, open drain and mirroring come from fields.
3. **Register `next`** (the pointer's step target: TCA9555 pairs, MCP SEQOP
   wrap, `ai24` bank wrap) and **register `alias`** (a register whose bits
   are other registers' fields: MAX7300's 8-port registers).

Correctness risk concentrates in the interrupt logic (when a flag sets, what
it captures, which read clears it, and the re-assertion of a compare-to-field
interrupt still mismatched after a clear) and in reads with side effects
happening once per byte read, not once per bit shifted. Both are pinned by
fixture tests that follow the datasheets' own timing diagrams (TCA9555 Figure
"Interrupt", MCP23017 § 3.6 "Interrupt Logic").

Rejected: a C model per expander family (about a dozen models for what the
declarative engine expresses; plan-56's decision); building on the `serial`
kind's GPIO (`docs/parts-format.md` § `serial` `gpio` is a UART bridge's
fixed `sc16is`/`xr1280` register set, not a general one); reusing
`"display"` as the section name (an expander displays nothing; one loader
serves both names).

## 4. Detailed Design

### 4.1 The kind

`backend/src/models/ioexpander.c` as `leddriver.c`: `DEVICE_KEYS` = `vcc`,
`inputMax`, `analog`, `bus`, `timing`, `registers`, `protocol`, `pins`,
`io`, `domains`; `PART_KEYS` = `vcc`, `inputMax`, `i2cAddress`, `i2c`,
`resets`; `load_device` = `cv_load_device(ctx, json, CV_MODE_IO)`; the rest
`cv_bind_pinout`, `cv_load_part`, `cv_pin_open_drain` (port pins in
`io.openDrain`, INT pins with `openDrain`), `cv_pin_analog`, `cv_pin_tristate`.
`CV_MODE_IO` requires `io` with at least one port and rejects `display`,
`conversion`, `ready`, `dac`, `clock`, `type`, `bits`, `channels`.
`Part.kind` = `PART_IOEXPANDER`; facts on `PartIoExpander` (`io` = port
count, `bus` = `i2c`/`spi`/`i2c-spi`, `interrupts` = interrupt pin count) for
the app's details, read by `tests/support/listing.c`.

### 4.2 `io.ports`

The `ports` object array of § `leddriver` `ports`, unchanged, plus:

| Key | Value |
|---|---|
| `"readInvertField"` | optional field: `readField` gets the input level XOR this field (outputs still read the data bit) |
| `"pullEnableField"` | optional field: while nonzero and the pin is not an output, the pin is a weak pull (1 unless `pullSelectField` reads 0, then a weak 0) |
| `"pullSelectField"` | optional, needs `pullEnableField`: 1 pull-up, 0 pull-down |
| `"openDrainField"` | optional field: while nonzero an output drives 0 for data 0 and releases (Z) for data 1 |
| `"int*"` | the per-port interrupt keys of § 4.3 |

`io.openDrain` (pin names, fixed open drain) as `display.openDrain`.

### 4.3 `io.interrupts` and `io.interruptPins`

```json
"interrupts": [{"name": "A", "ports": ["GPA0", "GPA1"], "compare": "field",
                "clearOn": ["read:GPIOA", "read:INTCAPA"]}],
"interruptPins": [{"pin": "INTA", "sources": ["A"], "activeHigh": "IOCON.INTPOL",
                   "openDrain": "IOCON.ODR", "mirror": {"when": "IOCON.MIRROR=1", "sources": ["A", "B"]}}]
```

Per-port keys used by a source's ports: `intEnableField` (nonzero: the pin
can interrupt; absent: always), `intMaskField` (nonzero: masked; PCAL),
`intModeField` and `intCompareField` (`compare: "field"`: mode 1 compares to
the compare field, mode 0 to the previous level), `intFlagField` (set when
the pin interrupts; read-only to the host), `intCaptureField` (the pin's
level when its source's first flag set since the last clear).

Source keys: `compare` — `"snapshot"` (active while an enabled input-mode pin
differs from its level at the last clear event; no flags needed), `"previous"`
(a change of an enabled pin sets its flag), `"field"` (per port as above);
`clearOn` — events that clear the source's flags and retake its snapshot:
`"read:<REG>"` / `"write:<REG>"` (a completed byte read or write of that
register), `"read"` / `"write"` (any completed read / write transaction: the
`quasi` template). A source with `compare: "field"` whose pin still differs
from its compare value sets its flag again right after the clear (MCP23017
§ 3.6.1). Interrupts are evaluated after every pin change and register write,
`tIv` (`timing.tIv`, ns, default 0) before the output pin changes.

Output pins: `sources` (active while any source has a flag set or, for
`snapshot`, differs); `activeHigh` a field or `true`/`false` (default false:
active low); `openDrain` a field or a boolean (default true); `mirror` a
condition under which the pin follows the other sources too.

### 4.4 Register keys

| Key | Value |
|---|---|
| `"next"` | optional register name: the pointer's next register after this one (overrides the next higher address); `when`-conditioned form `{"when": cond, "next": name}` array, first match wins |
| `"alias"` | optional array of field names (`"REG.FIELD"`), MSB first, total width = `width`: reading returns them, writing writes them; an alias register has no `reset` and no fields of its own |

### 4.5 Read side effects

A register read counts when its last bit has been shifted out and the
controller ACKed or NACKed it (I2C `acked`) or the SPI byte completed; `cor`
masks and `clearOn: "read:<REG>"` both fire there, once per byte. The engine's
existing hook for conversion `onRead` is the precedent.

### 4.6 Test host

`backend/tests/support/bushost.c/.h`: a bit-bang I2C controller (start,
stop, byte out with ACK read, byte in with ACK/NACK, open-drain SDA/SCL with
the bench's pull-ups) and SPI controller (mode 0–3, CS), on the existing
bench (`tests/support/bench.h`). Used by `ioexpander_test` and
`i2cbus_test`; the older tests keep their own helpers.

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — the kind shell

- [ ] Grep first, then append after the last kind: `PART_IOEXPANDER` and `PartIoExpander` in `parts.h`; `IOEXPANDER_KIND` in `kinds.c`; `CV_MODE_IO`; `ioexpander.c`; CMake; `Parts/ioexpander.json` `{"format": …, "kind": "ioexpander", "devices": []}`; `tests/support/listing.c`; `TRACE_LAST_KIND` in `tests/trace_check.c`; the last-kind entries of `tests/listing_check.c`.
- [ ] `library_check` rejection cases: `io` on a `converter`/`rtc`/`leddriver` device; an `ioexpander` without `io` or with `display`/`conversion`; an unknown `io` key; a port naming no pin.

Acceptance: `ctest --test-dir /tmp/logic-p72 -R 'library_check|listing_check|trace_check'` → 3 passed, and `wc -l backend/tests/golden/*.txt` equals P1/R1/F1 (est. 3 min).
Commit:

### Phase 2 — register `next` / `alias`, port keys

- [ ] `next`, `alias` in the register loader and `reg_next` / register read/write; `readInvertField`, `pullEnableField`, `pullSelectField`, `openDrainField` in the ports loader and `apply_ports`.
- [ ] `converter_test` cases: a pair-toggling pointer (0→1→0), a wrap (last → first), an alias read and write; `display_out_test` cases for the four port keys on a `leddriver` fixture (so the shared driver is covered from both kinds).

Acceptance: `ctest --test-dir /tmp/logic-p72 -R 'converter_test|display_out_test'` → 2 passed, goldens unchanged (est. 3 min).
Commit:

### Phase 3 — interrupts, the test host, `ioexpander_test`

- [ ] § 4.3 and § 4.5 in the engine; `tests/support/bushost.c/.h`.
- [ ] `ioexpander_test.c` with the four fixtures: `quasi` (write 0x0F, LEDs on P4–P7 low; P0 pulled low by the bench → INT low; a read returns 0x0E and INT goes high); `pca9555` (config 0x00, output 0x01 → P00 high; polarity on an input inverts the read; reading Input 1 after Input 0 in one transaction; INT clears on the Input read); `mcp23x17` I2C (IODIR, GPPU pull-ups read 1 on open pins, DEFVAL compare re-asserts after a GPIO read, INTCAP holds the captured level, MIRROR joins INTA/INTB, SEQOP=1 keeps the pointer); `mcp23x17` SPI with HAEN=1 answering only its A2–A0 opcode.
- [ ] `docs/parts-format.md` § `ioexpander` (the kind, `io`, the port keys, interrupts, the two register keys in § `converter` `registers`, the drawer row); `docs/backend.md` § Layout (`ioexpander.c`, `bushost.c`).

Acceptance: `ctest --test-dir /tmp/logic-p72 -R ioexpander_test` → 1 passed; `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --backend --check` → exit 0 (plan-68's rules; est. 6 min).
Commit:

## Validation Plan

- Tests: `ioexpander_test` (new), `converter_test`, `display_out_test`, `library_check`, `listing_check`, `trace_check`.
- Goldens: unchanged (no part).
- Runtime proof: `ioexpander_test`'s fixtures (the app sees parts only in I).
- Doc sync: `docs/parts-format.md` § `ioexpander`, § `converter` `registers`, § `leddriver` `ports`; `docs/backend.md` § Layout.
- Final gate (once, after Phase 3): `ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed` with T0 + 1 programs, and `node scripts/source-size.js --check` → exit 0 (est. 5 min).

## Decisions

- Expanders on the converter engine, a new `io` section — owner (simulate on the existing bus engines) and the author (the section; § 3 rejected alternatives).
- The new port keys are accepted on `display.ports` too (one driver, one loader) — the author's call.
- Kind name `ioexpander`, drawer Interface → I/O Expanders → I2C / SPI — the author's call, following `leddriver`'s Interface → Driver.

## Corrections

(none yet)
