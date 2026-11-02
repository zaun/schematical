# plan-12-C: The `serial` kind, the line engine, the `ns16550` core and the terminal API

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-B

C builds everything the later sub-plans hang off: the `serial` kind and its
model table, the serial line engine (baud generator, transmitter, receiver),
the Intel host bus helpers, the `ns16550` model with the PC16550D's behavior
(the 16450/16550 baseline every 16550-family device starts from), the
terminal hooks and the circuit API, and `serial_test`. No part file lands
yet; `serial_test` runs on inline fixture devices.

Outcome of C:

- `serial_test` (new ctest program) passes every case of §4.6 on a
  PC16550D-shaped fixture device and a 16450-shaped one (`"fifo": 0`).
- `circuit_serial_*` (§4.5) work: a fixture UART programmed to 8N1 at
  divisor 1 on a 1.8432 MHz oscillator (115 200 baud) sends `H`, `i`, and
  `circuit_serial_sent` returns `0x48 0x69`; `circuit_serial_send` of `OK`
  makes the RBR read `0x4F`, then `0x4B`.
- `ctest` → T0 + 1 programs; no golden line changes (no part file yet).

References: plan-12-A §3 (design overview), §3.5 (signal rules), §3.7
(terminal); `plan-12-data/specs/ti-a.md` § PC16550D (registers, pins,
behavior, timing — the core's source), § TL16C450 (the 16450 shape);
`plan-12-data/decisions.md` (P1–P11, § PC16550D, § TL16C450);
`planning/completed/plan-10-B-latch-kind.md` (kind with `"model"` dispatch
and `seq.h`), `planning/completed/plan-09-B-cpu-kind-6502.md` (`cpu_state`
hook, `circuit_cpu_state`).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-B landed | `grep -n 'pullUp' schematical/schematical/Canvas/SchematicModel.swift` → matches | NOT MET (2026-11-02) |

## 1. Goal

- `ls backend/src/models/serial.c serial.h serial_line.c serial_line.h
  serial_bus.c serial_bus.h ns16550.c` (in `backend/src/models/`) → all
  exist; `grep -n SERIAL_KIND backend/src/library/kinds.c` → 2 lines.
- `ctest --test-dir /tmp/logic-p12` → `100% tests passed`, T0 + 1 programs.
- `git diff --stat backend/tests/golden` → no change.

### Non-goals

- No part file (`serial.json` lands in F); no D/E extension; no bridge front
  end (I); no app change (K).
- No change to any existing kind or test expectation.

## 2. Current State

- Kinds: `KINDS[]` in `backend/src/library/kinds.c` (14 today; plan-11 adds
  its LCD kind); each kind is a `PartKindOps` plus one `SimComponentType`.
  Model dispatch inside a kind: `latch.c`/`counter.c`/`shift.c` (`"model"`
  key, `seq_check_model_keys` in `seq.h:93`) and `cpu.c` (`"core"` key with a
  core ops table, `cpu.h`).
- `SimComponentType` (`sim.h:92-150`) has optional hooks `lights`,
  `set_switch`, `memory_size`, `memory_read`, `light_time`, `memory_load`,
  `cpu_state`; `circuit.c:491` `circuit_cpu_state` is the pattern: check the
  part, look up the hook, fail with `part N is not a processor`.
- `sim_schedule_timer` (`sim.h:252`) and `pin_changed` give the model its
  clock edges and delays; `sim_schedule_inertial_drive` and
  `sim_schedule_drive` its outputs.
- `listing_check` / `trace_check`: a new kind needs its name in
  `listing_kind_name` and `TRACE_LAST_KIND` moved (`trace_check.c:22`).
- Tests are listed in `backend/CMakeLists.txt` (`add_test` per program).

## 3. Design

### 3.1 Kind and model table (`serial.c`, `serial.h`)

- `PART_SERIAL` appended to `PartKind` (`parts.h`); `PartSerial { const char
  *model; unsigned channels; const char *grade; }`; `const PartSerial *serial`
  on `Part`.
- Device keys: `"model"` (required) plus the model's keys; `"grades"`
  (required): grade name → timing in ns (the model's timing names; a missing
  name is 0) plus optional `vcc`, `inputMax`, `note`. Part keys: `"grade"`
  (a key of `grades`), optional `"vcc"`, `"inputMax"` (plan-12-A §3.4).
- A model is `SerialModel { name; device_keys; load_device; bind;
  timing_names; component callbacks; terminal callbacks }`. `serial.c` checks
  `"model"`, hands the device JSON to the model (unknown keys rejected with the
  model's name, as `seq_check_model_keys`), and forwards every
  `SimComponentType` hook to the instance's model. C registers `ns16550`; D, E
  extend it; H, I, J register theirs.
- `listing_check`: `|serial|model=<m>|channels=<n>|grade=<g>` then the
  common columns; `listing_kind_name(PART_SERIAL)` → `serial`;
  `TRACE_LAST_KIND` → the last `PartKind`.

### 3.2 Line engine (`serial_line.c`, `serial_line.h`)

Per channel, independent of the host bus:

- **Baud generator.** Counts rising edges of the clock input (the model passes
  each edge in). Divisor register (16 bits, D/E add a fractional part and a
  prescaler); every `divisor` input clocks produce one 16× tick (D/E: 8×, 4×).
  Divisor 0 → no ticks; BAUDOUT reads X (P2). BAUDOUT: high for the first half
  of each divisor period, low for the second (`tbld` after the clock edge); the
  divisor-1 case passes the clock through.
- **Transmitter.** Idle marking (1). A frame starts on the tick after a
  character is available and the shift register is empty: start (0), 5–8 data
  bits LSB first (9 for models that set it), parity (none, odd, even, stick 1,
  stick 0), stop 1, 1.5 (5-bit words) or 2; each bit 16 ticks. Break holds TX
  at 0 from the next tick while set. Each completed frame's data bits are
  appended to the channel's sent log (terminal, §3.4).
- **Receiver.** Idle until a 1→0 on RX; samples at tick 8 of each bit (P11
  values per device later: `startSample` 7½ is a D/E option); a start bit
  that reads 1 at its middle is rejected (false start). Data, parity and the
  first stop bit are sampled at mid-bit; a 0 stop bit sets framing error and
  the receiver resynchronizes as the PC16550D (p16: assumes the bad stop bit
  was the next start bit). A break is a frame of all 0 including stop;
  one break character is loaded and the receiver waits for RX to return to 1.
  X on RX at a sample point: that bit is X; an X start bit is not a start.
- Frame events go to the model through callbacks (`on_rx_char(data, flags)`,
  `on_tx_empty`, `on_tx_shift_empty`).

### 3.3 Host bus helpers (`serial_bus.c`, `serial_bus.h`)

Intel style: address lines, chip selects active-low and active-high lists,
optional address strobe (ADS: address and selects latched while it is low,
held while high), read strobes (an active-low and/or active-high pin; a read
cycle is selected ∧ (RDBAR low ∨ RD high)), write strobes likewise. A read
drives the data bus `trvd` after the cycle starts, floats it `thz` after it
ends; DDIS is low while the chip drives the bus (`trdd`). A write stores the
data present at the end of the strobe (the trailing edge). Several strobes
active together or an X strobe/select: P2 (X data / X stored). Reset pin:
active level per device key, Z reads inactive.

### 3.4 Terminal hooks

`SimComponentType` gains, after `cpu_state`:

```c
/* Serial parts: number of channels (0 = not a serial part). */
size_t (*serial_channels)(const void *state);
/* Frames the channel has sent since start: total count, and a copy of
   frames [start, start + cap). Each frame is its data bits, LSB = bit 0. */
size_t (*serial_sent)(const void *state, unsigned channel, size_t start,
                      uint16_t *frames, size_t cap);
/* Queue bytes to drive onto the channel's RX pin (plan-12-A §3.7). 1 when
   queued; 0 when the channel is not in an asynchronous mode. */
int (*serial_send)(Sim *sim, SimInstance *instance, void *state, unsigned channel,
                   const uint8_t *bytes, size_t count);
```

The line engine owns the log (a growable array per channel) and the send
queue: when the queue is non-empty and the RX line is not mid-frame, it drives
the channel's RX pin strong, bit by bit, each bit 16 receive ticks long, in
the channel's current format; then releases it (Z). While sending, the
receiver reads the driven level (also when RX is unconnected).

### 3.5 Circuit API (`circuit.h`, `circuit.c`)

```c
/* Serial parts (serial kind): channel count, or 0 for any other part. */
size_t circuit_serial_channels(const Circuit *circuit, size_t part);
/* Frames sent on a channel since start (0 when none or not serial). */
size_t circuit_serial_sent_count(const Circuit *circuit, size_t part, unsigned channel);
/* Copies frames [start, start + cap) of a channel; returns the number copied. */
size_t circuit_serial_sent(const Circuit *circuit, size_t part, unsigned channel,
                           size_t start, uint16_t *frames, size_t cap);
/* Types bytes into a channel's RX pin; 0, or -1 with circuit_error. Only
   after circuit_start. Refused when the RX pin's net has another part's
   output or in/out pin or a strong supply on it (pull parts are fine). */
int circuit_serial_send(Circuit *circuit, size_t part, unsigned channel,
                        const uint8_t *bytes, size_t count);
```

Errors (exact text): `part N is not a serial part`; `channel C of part N does
not exist`; `channel C of part N is not asynchronous`; `the RX pin of channel
C of part N is driven by part M` (M = the lowest such part index);
`circuit_serial_send` before start: `the circuit is not running`. The RX pin
of a channel comes from the model (`serial_rx_pin(state, channel)` internal
helper, so `circuit.c` can find its net). `docs/backend.md` § Using it gets a
paragraph on the terminal API.

### 3.6 The `ns16550` model, core (`ns16550.c`)

Device keys (C's set; D and E add theirs, plan-12-D §3, plan-12-E §3):

| Key | Value |
|---|---|
| `"bus"` | `{"style": "intel", "data": [8 names], "address": [3 names], "read", "readHigh", "write", "writeHigh" (each optional, at least one read and one write), "select": [active-low names], "selectHigh": [active-high names], "addressStrobe", "driverDisable", "selectOut", "reset", "resetActiveHigh": bool}` |
| `"clock"` | `{"in": XIN name, "out": XOUT name (never driven), "receive": RCLK name (optional; absent = the 16× tick drives the receiver), "baudOut": BAUDOUT name (optional)}` |
| `"channels"` | array (C: exactly 1; D: more) of `{"tx", "rx", "rts", "dtr", "cts", "dsr", "dcd", "ri", "out1", "out2", "int", "txrdy", "rxrdy"}` — pin names, each optional except `tx`, `rx`, `int` |
| `"fifo"` | `0` (16450: no FCR, IIR[7:6] = 00) or `16` |
| `"rxTrigger"` | 4 numbers, FCR[7:6] = 00…11 (PC16550D: `[1, 4, 8, 14]`) |
| `"intPin"` | `"driven"` (INTR active high, always driven: PC16550D, TL16C550x) |
| `"dmaClass"` | `"P"` (the PC16550D TXRDY/RXRDY mode 0/1 rules, `matrix.md` legend) |
| `"timeout"` | `"4ch"` (four character times of no RX activity with data in the FIFO) |

Timing names (PC16550D symbols): `trvd thz trdd tbld trint thr tmdo tsim trim
twxi twrr` (read→data, read end→float, read→DDIS, clock→BAUDOUT, RBR/LSR
read→INT clear, THR write→INT clear, MCR write→modem output, modem input→INT,
MSR read→INT clear, THR write→TXRDY, RBR read→RXRDY).

Behavior — every rule from `specs/ti-a.md` § PC16550D, with its decisions:

- Registers at A2..A0 with LCR[7] (DLAB): RBR/THR, IER, IIR/FCR, LCR, MCR,
  LSR, MSR, SCR, DLL/DLM; reset values from the PC16550D table (MR: IER, IIR
  = 01, FCR, LCR, MCR, LSR = 60, MSR[3:0] cleared; DLL/DLM, RBR, THR, SCR
  unchanged; SOUT, OUT1/2, RTS, DTR high; INTR low).
- FIFO mode (FCR[0]), FCR[1]/[2] clear, trigger levels, the 3 error bits per
  RX byte, LSR[7] (error in FIFO) set and clear rules, DMA mode 0/1 TXRDY and
  RXRDY, character timeout (4 character times; IIR = C), the transmitter
  interrupt delay rule (THRE interrupt one character after FIFO empty when
  the FIFO was more than one byte), 16450 mode with `fifo: 0`.
- Interrupt priorities (IIR): line status (6) > RX data / timeout (4 / C) >
  THRE (2) > modem status (0); set and clear conditions per the table; INTR
  high while any enabled source is pending; THRE cleared by an IIR read that
  reports it or a THR write.
- Line status: OE, PE, FE, BI, THRE, TEMT, DR; modem status and its four delta
  bits (TERI on RI 1→0 at the pin, i.e. trailing edge of ring).
- Loopback (MCR[4]): SOUT held 1, SIN disconnected, TX shift register to RX,
  modem outputs forced inactive, MSR from MCR (DTR→DSR, RTS→CTS, OUT1→RI,
  OUT2→DCD); interrupts operate (P4).
- Scratch register; divisor latch; break (LCR[6]); stick parity.

### 3.7 Rejected alternatives

- **Computing baud timing from a measured clock period** instead of counting
  edges: the oscillator part already toggles its net every half period, so
  one more listener costs one callback per edge; counting is exact under
  clock changes and needs no estimator.
- **Logging bytes at the THR write** instead of on the line: a terminal must
  show what the pin sends (loopback, break and a stopped transmitter would
  log wrongly).
- **Terminal input bypassing the pin**: the owner chose the pin.

## 4. Tests (`backend/tests/serial_test.c`, new ctest program)

Fixture devices are inline JSON (the `eeprom_test` pattern): `SFX-16550`
(the PC16550D plcc44 pinout and keys of §3.6, footprint `PLCC-44`, grade `X`
with the PC16550D timing, `vcc` [4.5, 5.5]) and `SFX-16450` (same with
`"fifo": 0`). A bench drives the bus pins, an oscillator part's net into XIN
(`circuit_*` API, the `5V`/`GND` supplies). One case per item:

1. Reset values of every register; output pins after MR.
2. Divisor write/read through DLAB; BAUDOUT period = divisor × clock period;
   divisor 0 → BAUDOUT X.
3. TX frame 8N1, 7E2, 5O1.5, stick parity: pin waveform bit times = 16 ×
   divisor clocks; sent log holds the data.
4. RX: a frame driven on SIN is read from RBR; false start rejected; framing
   error and resync; parity error; break (BI, one zero character).
5. FIFO: 16 bytes in, trigger levels 1/4/8/14 raise RX data interrupts;
   overrun on the 17th; LSR[7] set and cleared; FCR[1]/[2] clears.
6. Timeout interrupt after 4 character times; cleared by an RBR read.
7. Interrupt priority: line status over RX data over THRE over modem; IIR
   codes; THRE cleared by IIR read and by THR write; the transmitter
   interrupt delay rule.
8. Modem: delta bits, TERI on the trailing RI edge, MSR read clears; modem
   interrupt.
9. Loopback: SOUT stays 1, TX data appears in RBR, MSR mirrors MCR.
10. DMA mode 0 and 1 on TXRDY/RXRDY.
11. Bus: read access `trvd` and float `thz` on the data pins; DDIS; ADS latch;
    an X on RD during a read drives X; simultaneous RDBAR low and RD high is
    one read (both strobes are the same cycle).
12. 16450 fixture: FCR writes ignored, IIR[7:6] = 00, no FIFO, no timeout.

`circuit_check` gains the terminal API cases: the §Outcome send/receive pair;
`circuit_serial_channels` on a gate → 0; each error message of §3.5
(another part driving the RX net; channel out of range; before start).

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Kind, engine, bus helpers

- [ ] `parts.h` (`PART_SERIAL`, `PartSerial`), `serial.c/.h` (model table,
  grades), `kinds.c`, `CMakeLists.txt`, `listing_check`/`trace_check` (§3.1).
- [ ] `serial_line.c/.h` (§3.2), `serial_bus.c/.h` (§3.3).
- [ ] `library_check` cases: `"model": "x"` → `unknown model "x"`; a missing
  `grades` → `missing "grades"`; a part `"grade"` not in `grades` → rejected
  with the grade name.

Acceptance: `ctest` → 100%, T0 programs; `git diff --stat backend/tests/golden`
→ none (about 1 min).
Commit: —

### Phase 2 — `ns16550` core and the terminal API

- [ ] `ns16550.c` (§3.6); the terminal hooks (§3.4) and `circuit_serial_*`
  (§3.5); `docs/backend.md` paragraph; `docs/parts-format.md` § `serial` (the
  kind, the model table, grades, the `ns16550` keys of §3.6).
- [ ] `serial_test` (§4) added to `CMakeLists.txt`; the `circuit_check`
  cases.

Acceptance: `ctest --test-dir /tmp/logic-p12` → `100% tests passed`, T0 + 1
programs; `/tmp/logic-p12/serial_test` prints each of the 12 cases `ok`
(about 1 min).
Commit: —

## Validation Plan

`serial_test` and `circuit_check` are the gate: they exercise every rule C
adds; no part data exists yet, so the goldens must not move. The landing tier
is plan-12-K's.

## Decisions

- Baud by edge counting (§3.7); the sent log is recorded on the line (§3.7).
- The receiver samples at tick 8 by default (PC16550D: mid-bit); devices that
  print 7½ set it in D/E.
- One `ns16550` model with option keys, not a model per register style
  (plan-12-A §3.2).

## Corrections

(none yet)
