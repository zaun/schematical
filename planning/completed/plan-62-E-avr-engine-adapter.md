# plan-62-E: AVR simulation — the engine adapter (the `mcu` kind's `"model"`)

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-D

E connects the C chip to the event engine. An `mcu` device that names a
`"model"` gets a simulation model: its port pins drive and read nets, it
picks its clock, it obeys its reset pin, its flash loads through
`circuit_memory_load`, and `circuit_cpu_state` reports it. No shipped
device names a model yet (G); the tests use fixture devices.

Outcome: `ctest -R mcu_test` runs firmware on a fixture ATmega328 in
circuits of library parts: an LED blinks at the programmed rate on the
internal 1 MHz clock and 16 times faster on a 16 MHz crystal, a button with
the internal pull-up reads, a PWM pin has the programmed duty, an external
interrupt fires, the reset pin restarts the program.

References:

- plan-62-A § Decisions (the clock rule), plan-62-B § 3.1 / § 3.2 (ports,
  `avr_chip`), plan-62-C (ADC), plan-62-D (layouts, `avr_dasm`).
- `backend/src/engine/sim.h` (`SimComponentType`, `sim_schedule_drive`,
  `sim_schedule_timer`, `sim_read_pin`); `backend/src/models/mcu.c`;
  `backend/src/models/cpu.c` (the `cpu_state` hook precedent);
  `backend/src/models/crystal.c` (a crystal drives X1 and X2 weakly from
  time 0); `backend/include/cpu_state.h`; `backend/src/circuit.c:642`
  (`circuit_memory_load`), `:692` (`circuit_cpu_state`).
- `docs/backend.md` § Adding a kind ("A new key on an existing kind"),
  `docs/parts-format.md` § `mcu`; `scripts/part-editor-schema.py`,
  `scripts/part-editor-schema-extra.py`.
- ATmega328P datasheet (DS40002061): § 9.2 clock sources, § 9.12 CLKPR
  (CKDIV8), § 11 reset, § 14.2 port pin states.

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-D is complete | `ls planning/completed/plan-62-D-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after de782de2) |

If plan-62-D is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- An `mcu` device with `"model": "atmega328" | "atmega168pb" |
  "atmega328pb"` loads without `"simulated": false`, and a circuit accepts
  its parts.
- In a circuit the part behaves as § 3 says: startup, clock choice, reset,
  GPIO, PWM, external and pin-change interrupts, ADC.
- `circuit_memory_size` / `_read` / `_load` address the part's flash (bytes),
  and `circuit_cpu_state` reports status, cycles, PC, SP, SREG, R0–R31 and
  the last instruction.
- `mcu_test` and `library_check` pass with the new cases; `trace_check` and
  `listing_check` pass with goldens unchanged.

### Non-goals (explicit constraints)

- No shipped device changes: `mcu.json` is untouched (G), so
  `backend/tests/golden/` does not move.
- USART, SPI and TWI drive no pin (F).
- No fuses. The clock rule is fixed (§ 3.2); brown-out, the reset-disable
  fuse, debugWIRE and lock bits do not exist. The supply is not sensed: the
  chip runs whatever VCC reads (the voltage check judges the supply, as for
  every part).
- Analog levels do not exist in the engine: an ADC input reads 0 V or 5 V
  (§ 3.5). The analog comparator is not modeled.
- Speed is not a goal: a chip on a 16 MHz crystal costs one engine event
  per clock edge. No look-ahead or batching.
- `CpuState`'s meaning for the `cpu` kind does not change; only
  `CPU_MAX_REGISTERS` grows.

## 2. Current State

- `backend/src/models/mcu.c`: `mcu_load_device` refuses a device unless
  `"simulated"` is `false`; `MCU_MODEL` has no callbacks. `DEVICE_KEYS` is
  `core, clock, flash, ram, eeprom, vcc, inputMax`.
- `PartMcu` (`backend/include/parts.h:321`): `core, max_hz, flash, ram,
  eeprom, psram`.
- `CpuState` (`backend/include/cpu_state.h`): `CPU_MAX_REGISTERS = 24`,
  statuses `CPU_RUNNING … CPU_BUS_RELEASED`.
- `circuit_cpu_state` answers `part N is not a processor` when the model
  has no `cpu_state` hook or the hook returns 0; `circuit_memory_load`
  answers `part N (name) has no loadable memory` when the model lacks
  `memory_load` or `memory_size`, or `memory_size` returns 0.
- The engine: `sim_schedule_drive` queues any number of drives on a pin;
  `sim_schedule_timer` queues a timer that cannot be cancelled (a model
  ignores a stale one by a generation number); `pin_changed` reports every
  net change on a connected pin.
- The app's Part Editor checks that its form has every key of a kind's
  `*_KEYS[]` arrays (`PartKindKeysTests`, `scripts/part-editor-schema.py`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Pin names of the 18 devices' pinouts | `PB0–7`, `PC0–6`, `PD0–7`, `PE0–3`, `ADC6`, `ADC7`, `AREF`, `AVCC`, `VCC`, `GND` | the name census (python over `mcu.json`, 2026-11-24): `{'PC#': 252, 'PD#': 288, 'VCC': 50, 'GND': 86, 'PB#': 288, 'AVCC': 36, 'AREF': 36, 'ADC#': 28, 'PE#': 16}` |
| Their roles | `PBn`/`PCn`/`PDn`/`PEn` `inout`; `ADCn`, `AREF` `input`; `AVCC`, `VCC` `power`; `GND` `ground` | same census |
| Registers the Processor window gets | 35 | PC, SP, SREG, R0–R31 |
| Engine speed today | a 1 MHz W65C02S board runs at 0.18× real time; a 1.2 MHz oscillator with a 16-stage counter at 2.3× | `/tmp/sim-rel/cpu_bench 100`; `/tmp/sim-rel/sim_bench LMK6CE001200CDLFR 16 10 0` (2026-11-24) |

### Verified properties

- **XTAL1 is PB6 and XTAL2 is PB7, RESET is PC6, on all 18 devices**
  (`planning/plan-62-data/atdf_signals.txt`: `CPU XTAL1 PB6`, `CPU XTAL2
  PB7`, no device differs; the pinouts in `mcu.json` name the pins `PB6`,
  `PB7`, `PC6`, never `XTAL1` or `RESET`).
- **A crystal part clocks both its terminals from time 0** (`crystal.c`
  header: X1 at the level, X2 at the complement, both weak), so a chip that
  watches PB6 sees edges from the first half period.
- **`circuit_cpu_state` and `circuit_memory_load` need no change**: both
  work through the model's hooks (read at `circuit.c:642–707`).

## 3. Design Overview

One new file, `backend/src/models/mcu_avr.c`, holds the adapter; `mcu.c`
keeps the loader and forwards its model hooks to the adapter for a device
with a model. Pieces: the loader, the clock, reset, GPIO, ADC, the
inspection hooks.

Correctness risk concentrates in the clock: the adapter decides when each
instruction runs in engine time, in two modes, and every later letter's
timing rests on it. `mcu_test` measures periods in picoseconds in both
modes. **Byte-identity is a gate here in one place only**: goldens must not
move, because no shipped part changes; a diff in `backend/tests/golden/`
means the loader changed a shipped device's facts, and is root-caused on
that one part's line.

Rejected alternatives:

- *A separate `avr` kind*: the parts are already `mcu` parts with
  footprints and pinouts; a second kind would split the catalog.
- *Choosing the clock from the crystal's facts*: a model cannot see other
  parts. Counting edges on PB6 during startup needs nothing from the engine
  and matches what the pin really does.
- *Running many instructions per engine event*: an instruction can read a
  pin, so the chip may not run ahead of engine time.

### 3.1 The loader

`mcu.c`:

- `DEVICE_KEYS` gains `"model"`. `mcu_load_device`: with `"model"`, the
  core must be `"avr"` (`"model" needs core "avr"`), the name must be one
  `avr_layout_find` knows (`unknown mcu model "X"`), and `"simulated"` must
  be absent or `true` (`an mcu with a "model" is simulated: remove
  "simulated": false`). Without `"model"` the existing rule stands.
- `PartMcu` gains `const char *model` (NULL = none), last field.
- `mcu_bind_pinout` for a model device resolves pins by name into
  `McuAvrPins`: `port[letter][bit]` for `P<B–E><0–7>`, `adc6`, `adc7`
  (pins named `ADC6`, `ADC7`), each a pin index or none. A pin named
  `P<letter><digit>` that the layout has no port for is an error (`mcu
  model "atmega328" has no pin "PE0"`). `AREF`, `AVCC`, `VCC`, `GND` and
  `nc` pins take no part.
- `tests/support/listing.c`: the mcu facts print ` model=<name>` after
  `ram=` when the part has one.

`MCU_MODEL` gains every hook the adapter needs; each forwards to
`mcu_avr.c` when the part has a model and otherwise does nothing
(`cpu_state` and `memory_size` return 0, so the circuit answers as today).

### 3.2 Startup and the clock

Times are engine picoseconds. `AVR_STARTUP_PS` = 1 000 000 000 (1 ms).

- **Startup.** From `start` until `AVR_STARTUP_PS` the chip is in reset:
  every port pin Z. It counts rising edges (0 to 1) on PB6.
- **Clock choice**, made once per circuit start, at the end of startup:
  - 2 or more rising edges on PB6 during startup → **external clock**. PB6
    and PB7 are clock pins from then on: never driven, and PINB6/PINB7 read
    0. The CPU advances one clock cycle per rising edge of PB6 (per
    `prescaler` edges, CLKPR). CLKPR resets to 0.
  - otherwise → **internal oscillator**: 8 MHz with CKDIV8, so CLKPR
    resets to 3 and a CPU cycle is 125 000 ps × `prescaler` (1 µs until the
    firmware writes CLKPR). PB6 and PB7 are port pins.
- **Running an instruction.** The adapter keeps `due`, the cycle at which
  the next instruction starts. When the clock reaches `due` it calls
  `avr_chip_step` and adds the cycles the step took (the instruction, a
  taken interrupt, an EEPROM read's 4) to `due`. Internal: after each step
  it schedules a timer for that many cycles. External: each counted edge
  increments the cycle and steps when it equals `due`.
- The chip is created with the instance (its flash is loaded before
  `start`), so its clock frequency is set at the end of startup with a new
  `avr_chip_set_base_hz(chip, hz)` (the EEPROM write time, the watchdog's
  timeout): 8 000 000 internal; external, `10^12 / period` rounded, from
  the last two rising edges of startup.

A pin write lands on its net at the time of the instruction that made it
(drive delay 0).

### 3.3 Reset

PC6 is the reset pin, not a port pin: the adapter drives it weak 1 from
`start` (the internal pull-up), PORTC6/DDRC6 have no effect on it, and
PINC6 reads 0.

- PC6 reading 0 or X holds the chip in reset: every port pin Z, no
  instruction runs.
- PC6 back at 1 (or Z, which the pull-up makes 1): `avr_chip_reset`, then
  the first instruction runs one cycle later (the next counted edge, or
  125 000 ps × prescaler). Flash is kept; SRAM is kept (the datasheet
  leaves it undefined); the clock choice is kept.
- A watchdog reset (`cpu->on_reset`) releases every pin and restarts the
  same way without touching PC6.

### 3.4 GPIO

- **Out.** The port listener recomputes each changed pin from
  `avr_port_pin_state`: `Low` → strong 0, `High` → strong 1, `Input` → Z,
  `InputPullUp` → weak 1. Timer compare outputs arrive through the same
  state (avr8js's pin override).
- **In.** `pin_changed` on a port pin calls `avr_port_set_pin(bit, value ==
  LOGIC_1)`: 1 reads 1; 0, X and Z read 0. At the end of startup and after
  a reset every pin is read once with `sim_read_pin`.

### 3.5 ADC

`avcc` and `aref` are 5.0. An ADC input pin (PC0–PC5; `ADC6`/`ADC7`, or
PE2/PE3 on a PB layout) sets its channel's volts to 5.0 when its net reads
1 and 0.0 otherwise, on every change. So a conversion gives 1023 or 0 with
the AVCC or AREF reference and 1023 or 0 with the 1.1 V reference.

### 3.6 Inspection hooks

- `memory_size` → flash bytes, 8 bits. `memory_read` → flash, every bit
  known. `memory_load` → flash. An unloaded flash is 0xFF.
- `cpu_state`: status `CPU_RESET` during startup and while PC6 holds reset,
  else `CPU_RUNNING`; `cycles` = `cpu->cycles`; registers `PC` (16 bits,
  the byte address, `pc × 2`), `SP` (16), `SREG` (8), `R0`…`R31` (8), in
  that order, none while in reset; the last instruction: its byte address,
  its 2 or 4 bytes (words little-endian, as in the HEX file), `avr_dasm`'s
  text. `CPU_MAX_REGISTERS` becomes 40.

## Compatibility / Format Impact

- Part files: a new optional `mcu` device key, `"model"`. No shipped file
  uses it until G.
- `parts.h`: `PartMcu.model` appended. `cpu_state.h`: `CPU_MAX_REGISTERS`
  24 → 40 (the app recompiles against it; `CpuSnapshot` reads
  `CPU_MAX_REGISTERS`).
- Goldens: unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — the loader

- [x] `backend/include/parts.h`: `PartMcu.model`.
- [x] `backend/src/models/mcu.c`: `"model"` in `DEVICE_KEYS`, the rules of
      § 3.1, `mcu_bind_pinout` for a model device (result type in a new
      `backend/src/models/mcu.h`).
- [x] `backend/tests/support/listing.c`: ` model=`.
- [x] `backend/tests/support/mcu_fixture.h`: a second device `mfx328`
      (model `atmega328`, flash 32768, RAM 2048, EEPROM 1024, pinout `p28`
      copied from `mcu.json`'s `atmega328p` `p28`, part `TESTAVR`, footprint
      of `ATMEGA328P-PU`) and a third `mfx48` (flash 4096, RAM 512, EEPROM
      256, part `TESTAVR48`).
- [x] `backend/tests/library_check.c`: the valid model device, and the four
      rejections (`needs core "avr"`, `unknown mcu model`, the `"simulated":
      false` clash, a pin the layout lacks).
- [x] `scripts/part-editor-schema-extra.py`: `("device", "model",
      ("string",), [])` in `EXTRA["mcu"]`; `scripts/part-editor-schema.py`:
      `"mcu"` in `NO_VARIANT`; run `python3 scripts/part-editor-schema.py .
      mcu` and commit the regenerated
      `schematical/schematical/Editor/McuDraft.swift`.

Acceptance: the loader accepts and rejects as specified; goldens hold; the
Part Editor schema has the key.
  Check: `ctest --test-dir /tmp/sim-rel -R 'library_check|listing_check|trace_check|mcu_test' --output-on-failure` → 4 tests passed; `git diff --stat backend/tests/golden` → no output; `python3 scripts/part-editor-schema.py . mcu` → prints `missing []` (est. 2 min).
  Observed: 2026-11-24: 4 tests passed; no change under backend/tests/golden; `part-editor-schema.py . mcu` printed `missing []`.
Commit: 4b51faa0

### Phase 2 — the model: clock, reset, GPIO, flash

- [x] `backend/src/models/mcu_avr.c`: create/destroy (an `AvrChip` per
      instance), `start`, `pin_changed`, `timer`, §§ 3.2–3.4, the memory
      hooks; `mcu.c`: the forwarding hooks; `backend/CMakeLists.txt`.
- [x] `backend/tests/avr/src/` + `backend/tests/avr/*.hex`: the test
      firmware `blink` (toggle PB5 every 1000 cycles by a timer 1 compare
      interrupt), `button` (PD2 input with pull-up copied to PB0), `pwm`
      (OC0A fast PWM, duty 64/256), `int0` (falling edge on PD2 toggles
      PB1), each built with `oracle.js asm`; `avr_periph_test` checks, with
      `node`, that each `.hex` equals `oracle.js asm` of its `.asm`.
- [x] `backend/tests/mcu_test.c`: circuits of `TESTAVR`, `5V`, `GND`,
      `GENERIC-LED`, `GENERIC-CRYSTAL-16M`, a tactile switch, loading each
      `.hex` with `memory_image_parse` + `circuit_memory_load`:
      - before 1 ms every port pin's net is Z; PC6 reads 1;
      - `blink`, no crystal: PB5 changes every 1 000 000 000 ps (1000
        cycles of 1 µs), the first at 1 ms + the program's lead-in;
      - `blink` with the crystal on PB6/PB7: PB5 changes every 62 500 000 ps
        (1000 cycles of 62.5 ns); PB6/PB7 are not driven by the chip;
      - a CLKPR write to 0 in the internal mode makes the period 125 000 ps
        per cycle;
      - `button`: PB0 follows the switch; with the switch open PD2's net
        reads 1 (the weak pull-up);
      - `pwm`: OC0A (PD6) is high 64 and low 192 of every 256 cycles;
      - `int0`: a falling edge on PD2 toggles PB1; a rising edge does not;
      - PC6 pulled to GND by a switch: every port pin Z; released: the
        program starts over;
      - `TESTAVR48` runs `blink` built with one-word vectors;
      - an unloaded chip runs without an engine error for 10 ms;
      - `circuit_memory_load` past 32768 bytes is refused; `circuit_memory_read`
        returns the loaded bytes.

Acceptance: the cases above pass with the stated picosecond values.
  Check: `ctest --test-dir /tmp/sim-rel -R 'mcu_test|avr_' --output-on-failure` → 3 tests passed (est. 2 min).
  Observed: 2026-11-24: `mcu_test: all passed` with the period cases as corrected below (average 1000000000, 62500000 and 125000000 ps); `ok firmware (7 files equal their sources)`.
Commit: 4b51faa0

### Phase 3 — ADC and `circuit_cpu_state`

- [x] `mcu_avr.c`: § 3.5 and the `cpu_state` hook of § 3.6;
      `backend/include/cpu_state.h`: `CPU_MAX_REGISTERS = 40`.
- [x] Test firmware `adc` (read channel 0, write the high bits to PORTB).
- [x] `mcu_test.c`: `adc` with PC0 on 5 V gives 1023, on GND 0, left open
      0; `circuit_cpu_state` during startup is `CPU_RESET` with no
      registers; while running it is `CPU_RUNNING` with 35 registers, `PC`
      even, `SP` inside SRAM, and the last instruction's text equal to
      `avr_dasm` of the bytes it reports; on `TESTMCU` (no model) it is
      still refused with `is not a processor`.
- [x] `docs/backend.md`: the notes on `circuit_cpu_state` and memory load
      name the mcu; § Layout lists `mcu_avr.c`, `mcu.h`; § Tests updates
      `mcu_test`; a new note "Microcontrollers" states §§ 3.2–3.6 in user
      terms. `docs/parts-format.md` § `mcu`: the `"model"` key and its
      three values, the pin names a model device needs.

Acceptance: the ADC and state cases pass; the cpu kind's tests still pass
with the larger register array.
  Check: `ctest --test-dir /tmp/sim-rel -R 'mcu_test|cpu_test|circuit_check' --output-on-failure` → 3 tests passed (est. 3 min).
  Observed: 2026-11-24: `mcu_test`, `cpu_test`, `circuit_check` passed (8 of 8 with the avr and library tests); last instruction shown: `004C  RJMP .-2`.
Commit: 4b51faa0

## Validation Plan

- Tests: `mcu_test`, `library_check`; `cpu_test` and `circuit_check` for
  the `CpuState` size change.
- Coverage check: `mcu_test` prints the count of instructions each case
  executed (from `cycles`) and requires it above 0, so a chip that never
  left reset cannot pass a case that expects a quiet pin.
- Runtime proof: the `blink` cases: an LED's light toggles at the computed
  period in both clock modes (`circuit_part_light_time`).
- Doc sync: `docs/backend.md`, `docs/parts-format.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions (the clock rule). None added.

## Corrections

- § 3.2, Phase 2 (a miscalibrated prediction): a pin toggled from a timer
  interrupt does not change every 1000 cycles exactly. One gap is a CPU
  cycle long or short, and a whole period (two changes) is exact. Root cause,
  on one fixture: `oracle.js run` of `blink` in avr8js itself prints PB5
  changes at cycles 0x3fd, 0x7e6, 0xbcd, 0xfb6 (1021, 2022, 3021, 4022: gaps
  1001, 999, 1001), because avr8js brings a timer up to date between
  instructions; the C core is equal to it by the plan-62-B scenarios. The
  engine clock is exact: measured PB5 gaps of 1001 and 999 us on the internal
  clock and of 62562500 and 62437500 ps on the 16 MHz crystal, each pair
  summing to 2000 cycles. The acceptance is strengthened to what
  can be exact: the average over a whole period equals 1000 cycles to the
  picosecond (`toggle_period`: 1000000000, 62500000 and 125000000 ps), each
  gap within one cycle of it. The compare output itself is exact: OC0A is
  high 64 and low 192 cycles.
- § 3.1: two more loader rules: a model device's `"flash"` must be 512 bytes
  or more and even, and a part of a model device cannot override `"flash"`
  (the chip is built from the device's size). Both have `library_check`
  cases.
- § 3.2: `avr_chip_set_base_hz` already existed (plan-62-C). The chip is
  created with the instance, and `AvrChip.clock.reset_index` is set to 3 or
  0 at the end of startup, before `avr_chip_reset`.
- Phase 3: `circuit_cpu_state`'s refusal is checked on a `5V` supply part:
  `TESTMCU` cannot be added to a circuit at all (it is not simulated).
- Phases 1 to 3 landed in one commit: `mcu.c` forwards to `mcu_avr.c`, so
  the loader could not be built and tested without the adapter.
- The firmware files are `backend/tests/avr/<name>.hex`, built by
  `build.sh` from `tests/avr/src/<name>.asm` (a program with no settings
  file); `avr_periph_test` prints `ok firmware (7 files equal their
  sources)`. `blink_fast` (CLKPR = 0 first) and `blink48` (one-word vectors)
  were added to the four the plan named.

## Summary

The adapter's clock is the foundation every later timing rests on; it is
tested in picoseconds in both modes. Shipped parts and goldens are not
touched.
