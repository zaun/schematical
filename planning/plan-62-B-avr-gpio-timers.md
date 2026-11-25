# plan-62-B: AVR simulation — GPIO ports, the clock prescaler, timers and the chip assembly

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-A

B ports avr8js's GPIO port, clock prescaler and timer to C, introduces
`avr_chip.c` (a chip is a CPU plus the peripherals of a register layout),
and adds the scenario comparison: a firmware program with pin stimuli runs
in avr8js and in the C chip, and the two instruction traces must be equal
line for line.

Outcome: `ctest -R avr_periph_test` prints `ok <scenario>` for the 7
scenarios of § 3.4, each equal to avr8js in exact mode.

References:

- plan-62-A (§ 3.2 the core's API, § 3.3 exact mode, § 3.4 the hash and the
  live-oracle pattern, Decisions).
- avr8js 0.21.1 sources ported here: `src/peripherals/gpio.ts` (513 lines),
  `src/peripherals/clock.ts` (81), `src/peripherals/timer.ts` (810).
- ATmega48A/PA/88A/PA/168A/PA/328/P datasheet (DS40002061): § 14 I/O ports,
  § 15–18 timers, § 9.12 CLKPR.

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-A is complete | `ls planning/completed/plan-62-A-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after 67ce943d) |

If plan-62-A is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `avr_gpio.c`, `avr_clock.c`, `avr_timer.c` and `avr_chip.c` exist and the
  `atmega328` layout assembles a CPU with ports B, C, D, the clock prescaler
  and timers 0, 1, 2.
- `oracle.js` gains `asm`, `layout-check` and `run`.
- `avr_periph_test` passes with 7 scenarios equal to avr8js.

### Non-goals (explicit constraints)

- No USART, SPI, TWI, ADC, EEPROM or watchdog (C). No layout but
  `atmega328` (D). No engine, kind or app change.
- The asynchronous timer 2 clock (TOSC1/TOSC2, the `AS2` bit) is not
  modeled; avr8js does not model it.
- A timer's pin changes land on instruction boundaries, as in avr8js: the
  count is brought up to date by clock events that `avr_cpu_tick` runs
  between instructions.

## 2. Current State

After A the tree has the CPU core and `avr_core_test`; nothing constructs a
peripheral.

In avr8js a peripheral is a class whose constructor installs read and write
hooks on the CPU for its register addresses and keeps its state in fields.
All register addresses, bit positions and interrupt vectors come from a
config object passed to the constructor; the package exports the ATmega328P
configs (`portBConfig`, `portCConfig`, `portDConfig`, `timer0Config`,
`timer1Config`, `timer2Config`, `clockConfig`).

- `AVRIOPort` (`gpio.ts`): hooks on PIN, DDR, PORT (a write to PIN toggles
  PORT), the external interrupts of the port's pins (EICRA, EIMSK, EIFR;
  low level is a `constant` interrupt) and its pin-change interrupt (PCICR,
  PCIFR, PCMSK). `setPin(index, value)` is the outside world driving a pin;
  `pinState(index)` is what the chip drives (`Low`, `High`, `Input`,
  `InputPullUp`); `timerOverridePin` lets a timer take a pin; a pin's
  `externalClockListeners` entry feeds a timer's external clock.
- `AVRClock` (`clock.ts`): the CLKPR write sequence (CLKPCE, then the
  prescaler within 4 cycles) and `cyclesDelta`.
- `AVRTimer` (`timer.ts`): 8- and 16-bit timers, all waveform modes,
  compare outputs A, B and C, the 16-bit TEMP high-byte latch, input capture
  register as TOP, force output compare, prescaler dividers, the external
  clock pin, and the overflow and compare interrupts.

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines to port in B | 1404 | `wc -l src/peripherals/gpio.ts src/peripherals/clock.ts src/peripherals/timer.ts` (513 + 81 + 810) |
| Ports of the `atmega328` layout | 3 (B, C, D) | `planning/plan-62-data/atdf_cmp.txt`: `PINB`…`PORTD` at 0x23–0x2B, no port A or E on the 14 non-PB devices |
| Timers of the layout | 3 | same file: `TCCR0A`, `TCCR1A`, `TCCR2A` |

### Verified properties

- **avr8js's exported configs are the ATmega328P's registers.** Read in
  `gpio.ts` / `timer.ts` and compared with `ATmega328P.atdf`
  (`planning/plan-62-data/atdf_cmp.py`): port B at 0x23/0x24/0x25, INT0 and
  INT1 on port D bits 2 and 3 (vectors 1 and 2, word addresses 2 and 4),
  PCINT0–2 at word addresses 6, 8, 10; timer 0 compare A at word address
  0x1c (vector 14), OC0A on PD6, OC0B on PD5, T0 on PD4.
- **The assembler in the vendored package builds the test firmware.**
  `dist/cjs/utils/assembler.js` exports `assemble(source)` returning
  `{bytes, errors, lines}` (plan-62-A § Verified properties, the probe).

## 3. Design Overview

Four pieces: the three ported peripherals, the chip assembly, the scenario
runner on both sides, the scenarios.

Correctness risk concentrates in `avr_timer.c`: avr8js computes the count
lazily from `cpu.cycles` with JavaScript number arithmetic (division,
`Math.floor`, modulo on doubles). The C code uses `uint64_t` cycles and the
same expression order; where avr8js divides, the C divides integers only
where the operands are integers in avr8js too, and uses `double` where
avr8js's value can be fractional (`AVRClock.cyclesDelta`). **Byte-identity
is not a gate**; the scenario traces are the gate and any difference is a
bug to root-cause on the first differing line.

Rejected alternatives:

- *Cycle-stepped timers* (count every clock): faster to write, but the
  traces would differ from avr8js wherever avr8js's lazy update shows
  through (a TCNT read mid-prescale), and the oracle would stop being exact.
- *A JSON layout file read by both sides*: two parsers and a file the app
  would have to bundle. The C tables are the source; the oracle receives
  the layout as JSON printed by the C side (§ 3.3).

### 3.1 The peripherals

One C file per avr8js file, a struct per class, a constructor per
constructor. Hooks are `(fn, ctx)` with `ctx` the peripheral.

| avr8js | C |
|---|---|
| `AVRPortConfig`, `AVRExternalInterrupt`, `AVRPinChangeInterrupt` | `AvrPortConfig`, `AvrExternalInterruptConfig`, `AvrPinChangeConfig` |
| `new AVRIOPort(cpu, config)` | `avr_port_init(AvrPort *, AvrCpu *, const AvrPortConfig *)` |
| `port.setPin`, `pinState`, `timerOverridePin`, `addListener`, `externalClockListeners[i]`, `openCollector` | `avr_port_set_pin`, `avr_port_pin_state`, `avr_port_override_pin`, one listener `{fn, ctx}`, `external_clock[8]`, `open_collector` |
| `cpu.gpioPorts`, `cpu.gpioByPort[PORT]` | `cpu->ports[]` (up to 12) and `avr_cpu_port_at(cpu, port_address)` |
| `AVRClockConfig`, `new AVRClock(cpu, baseFreqHz, config)`, `prescaler`, `timeNanos` | `avr_clock_init(AvrClock *, AvrCpu *, uint32_t base_hz, const AvrClockConfig *)`, `avr_clock_prescaler`, `avr_clock_time_nanos` |
| `AVRTimerConfig`, `new AVRTimer(cpu, config)`, `reset()` | `AvrTimerConfig`, `avr_timer_init`, `avr_timer_reset` |

`avr_clock_init` takes one value avr8js lacks: `reset_index`, the CLKPR
value after reset (0 on avr8js's path; E passes 3 for the internal
oscillator with CKDIV8). The scenarios run with 0.

### 3.2 The chip assembly

`backend/src/models/avr_chip.h` / `avr_chip.c`:

```c
typedef struct AvrLayout AvrLayout;      /* a register layout: its peripherals' configs */
typedef struct AvrChip AvrChip;          /* a CPU and the peripherals of a layout */

const AvrLayout *avr_layout_find(const char *name);   /* "atmega328" in B */
AvrChip *avr_chip_create(const AvrLayout *layout, uint32_t flash_bytes,
                         uint32_t sram_bytes, uint32_t eeprom_bytes, uint32_t base_hz);
void avr_chip_destroy(AvrChip *chip);
void avr_chip_reset(AvrChip *chip);      /* cpu reset, registers 0x20… cleared, each peripheral reset */
void avr_chip_step(AvrChip *chip);       /* avr_instruction, then avr_cpu_tick */
```

`AvrLayout` holds a name, the port configs with their letters, the timer
configs, the clock config, and (C, D) the other peripherals' configs and
the vector size. `AvrChip` exposes its `AvrCpu *`, ports and timers by
index for the engine adapter (E) and the test.

### 3.3 The oracle's `run`, `asm` and `layout-check`

- `oracle.js asm <file.asm>` prints the program as Intel HEX (one data
  record per 16 bytes, then the end record).
- `oracle.js layout-check <layout.json>` compares a layout printed by the C
  side with avr8js's own exported configs and prints `layout equal` or the
  first differing field. It is how the `atmega328` table is proven to be
  avr8js's ATmega328P.
- `oracle.js run <scenario.json> <layout.json>` builds a `CPU` and the
  peripherals from the layout JSON, loads the scenario's program, and prints
  the trace.

`avr_periph_test --layout <name>` prints a layout as JSON (the configs'
fields by avr8js's field names).

A scenario file (`backend/tests/avr/<name>.json`):

```json
{ "layout": "atmega328", "flash": 32768, "sram": 2048, "eeprom": 1024,
  "program": "<Intel HEX text>", "steps": 20000,
  "stimuli": [[1200, "pin", "D", 2, 0], [1300, "pin", "D", 2, 1]] }
```

A stimulus `[cycle, "pin", port, bit, value]` is applied before the first
instruction that starts at or after `cycle` (`port.setPin(bit, value)`).

The trace: one line per instruction, `i %x %x %02x %04x %08x` (pc, cycles,
SREG, SP, FNV-1a 32 of `data`, all after the instruction and its tick), and
one line per port listener call, `p %x %c %02x %02x` (cycles, port letter,
the listener's value, DDR), in the order they happen. Both sides run in
exact mode. The hash of the trace (FNV-1a 64) and its line count sit in
`backend/tests/avr/golden.txt` as `run <name> <lines> <hash>`, written from
the oracle's output; with `node` the test also diffs the live oracle output
and prints the first differing line with its line number.

### 3.4 The scenarios

Firmware sources are `backend/tests/avr/src/<name>.asm`, assembled into the
scenario JSON by `oracle.js asm`. Each must exercise:

| Scenario | Exercises |
|---|---|
| `gpio_out` | writes to DDRB/PORTB/PINB (the toggle), `SBI`/`CBI` on PORTD, pull-up state changes, reads of PINx on outputs |
| `gpio_int` | INT0 and INT1 in all four sense modes with stimuli on PD2/PD3, the low-level interrupt held and released, EIFR cleared by writing 1, PCINT0–2 with PCMSK masks |
| `timer0` | normal, CTC, fast PWM (TOP 0xFF and OCR0A) and phase-correct modes; OC0A/OC0B set, clear and toggle; the overflow and both compare interrupts; a TCNT0 read and write mid-count; every prescaler |
| `timer1` | 16-bit normal, CTC with OCR1A and with ICR1 as TOP, fast and phase-correct PWM at 8, 9 and 10 bits and with ICR1/OCR1A TOP, phase-and-frequency-correct; the TEMP latch on 16-bit reads and writes; force output compare |
| `timer2` | the timer 2 prescaler set (32 and 128), fast PWM on OC2A/OC2B, its interrupts |
| `timer_ext` | timer 0 clocked from T0 (PD4) on the falling and on the rising edge, timer 1 from T1 (PD5), with pin stimuli |
| `clkpr` | the CLKPR change-enable sequence, a late write that is ignored, each prescaler value 0–8 |

## Compatibility / Format Impact

None outside `backend/tests/avr/` and `backend/tools/`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — GPIO, clock and the chip

- [x] `backend/src/models/avr_gpio.c` (+ declarations in `avr.h`): `gpio.ts`.
- [x] `backend/src/models/avr_clock.c`: `clock.ts`, with `reset_index`.
- [x] `backend/src/models/avr_chip.h`, `avr_chip.c`: § 3.2 with the
      `atmega328` layout's ports B, C, D and clock.
- [x] `oracle.js`: `asm`, `layout-check`, `run` (§ 3.3).
- [x] `backend/tests/avr_periph_test.c`: `--layout`, the scenario runner,
      the golden and live comparisons; `backend/CMakeLists.txt`: the four
      new sources and `logic_test(avr_periph_test)` with `AVR_DIR`,
      `AVR_NODE`, `AVR_ORACLE`.
- [x] Scenarios `gpio_out`, `gpio_int`, `clkpr`: `.asm`, `.json`, golden lines.

Acceptance: the three scenarios equal avr8js and the layout equals
avr8js's configs.
  Check: `ctest --test-dir /tmp/sim-rel -R avr_periph_test --output-on-failure` → `layout atmega328 equal`, `ok gpio_out`, `ok gpio_int`, `ok clkpr` (est. 1 min).
  Observed: 2026-11-24: `layout atmega328 equal`, `ok gpio_out (521 lines, 121 port events, 0 interrupts)`, `ok gpio_int (6000 lines, 0 port events, 1489 interrupts)`, `ok clkpr (160 lines, 0 port events, 0 interrupts)`, each against the live oracle.
Commit: —

### Phase 2 — timers

- [x] `backend/src/models/avr_timer.c`: `timer.ts`.
- [x] `avr_chip.c`: timers 0, 1, 2 in the `atmega328` layout;
      `backend/CMakeLists.txt`: the source.
- [x] Scenarios `timer0`, `timer1`, `timer2`, `timer_ext`.
- [x] `docs/backend.md`: § Layout (the new files), § Tests (`avr_periph_test`).

Acceptance: all 7 scenarios equal avr8js.
  Check: `ctest --test-dir /tmp/sim-rel -R 'avr_' --output-on-failure` → 7 `ok <scenario>` lines, 2 tests passed (est. 1 min).
  Observed: 2026-11-24: 7 `ok` lines against the live oracle (`timer0` 60199 lines, 199 port events, 229 interrupts; `timer1` 45271, 271, 278; `timer2` 56198, 198, 151; `timer_ext` 2613, 113, 242); every program reaches its `end` label; `ctest -R avr_` 2 of 2 passed.
Commit: —

## Validation Plan

- Tests: `avr_periph_test`; `avr_core_test` unchanged and passing.
- Coverage check: each scenario's `.asm` is reviewed against its row of
  § 3.4 when written; the test prints, per scenario, how many port events
  and taken interrupts the trace holds, and requires both above 0 for the
  scenarios whose row names interrupts or pin outputs.
- Runtime proof: none in B (E).
- Doc sync: `docs/backend.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions. None added.

## Corrections

- Phase 1 also landed `avr_timer.c` and the three timers of the layout
  (Phase 2's first two tasks): `layout-check` compares the whole layout, so
  the timer tables had to exist for `layout atmega328 equal`. Phase 2 is the
  four timer scenarios and the docs.
- § 3.2 / plan-62-D § 3.1: the layout already holds vector numbers, and
  `avr_layout_scale(layout, flash_bytes, out)` turns them into word
  addresses (one word per vector for 8192 bytes of flash or less).
  `avr_periph_test --layout <name> <flash bytes>` prints the scaled layout.
  D's Phase 1 keeps only the PB layouts, the peripheral lists and the
  scenarios.
- § 3.2: `avr_chip_create` takes `clkpr_reset` as a sixth argument (the
  CLKPR reset value E needs); `AvrChip` is a public struct.
- § 3.3: a scenario file is built by a new `oracle.js build <name>.asm
  <name>.meta.json`; the settings files come from
  `tests/avr/src/make_meta.py`, `build.sh` rebuilds every scenario and
  `regen_golden.py` rewrites the `run` golden lines from the oracle. With
  `node` the test also checks each committed `.json` equals its build.
- § 3.3: the oracle and the C runner fill the flash with 0xFF before loading
  the program (avr8js's `Uint16Array` would start at 0).
- § 3.4: `gpio_int` drives no output, so its trace has no port event; the
  test requires a taken interrupt there and a port event in `gpio_out` and
  the timer scenarios (measured: `gpio_int` 1489 interrupts, 0 port events).

## Summary

The risk is the timer's lazy arithmetic; the scenario traces compare every
instruction's whole state with avr8js, so a drift shows at the instruction
where it starts.
