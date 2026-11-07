# Backend

The backend is a C library: an event-driven logic simulator plus a part
library loaded from JSON. The app (and the tests) use three public headers in
`backend/include/`:

- `parts.h` — the part library: load the part files, look parts up, read
  their facts as C structs.
- `circuit.h` — build a circuit from library parts, run it, read nets,
  lights and memory back.
- `memory_image.h` — turn a load file (raw binary or Intel HEX) into a
  memory image for `circuit_memory_load`. It takes bytes; the backend never
  opens files.

Part data lives in `schematical/schematical/Parts/<kind>.json`, one file per
kind; the format is in [parts-format.md](parts-format.md).

## Using it

```c
#include "parts.h"
#include "circuit.h"

/* 1. Load footprints.json, then the part files (the app reads its bundle
      resources; tests use tests/support/read_file.c). Each call takes one
      file's text. */
PartLibrary *library = part_library_create();
char *error = NULL;
if (load_footprints(library, footprints_json_text, "footprints.json", &error) != 0) {
    fprintf(stderr, "%s\n", error);
    free(error);
}
if (load_parts(library, gate_json_text, "gate.json", &error) != 0) {
    fprintf(stderr, "%s\n", error);   /* e.g. gate.json: devices[3] (74_00): ... */
    free(error);
}

/* 2. Read a part's facts. */
const Part *part = part_library_find(library, "SN74HC00N");
/* part->kind == PART_GATE, part->logic->family == "HC", part->pins[...],
   part->footprint->pads[...] (mm, designator matches part->pins[i].designator) */

/* 3. Build and run a circuit. Pins are named by designator ("1", "A1"). */
Circuit *c = circuit_create(library);
size_t vcc, gnd, led, anode, cathode;
circuit_add_part(c, "VCC", &vcc);
circuit_add_part(c, "GND", &gnd);
circuit_add_part(c, "GENERIC-LED", &led);
circuit_add_net(c, &anode);
circuit_add_net(c, &cathode);
circuit_connect(c, vcc, "1", anode);
circuit_connect(c, led, "2", anode);    /* LED pin 2 = A */
circuit_connect(c, led, "1", cathode);  /* LED pin 1 = K */
circuit_connect(c, gnd, "1", cathode);
circuit_start(c);
circuit_advance(c, 1000000);            /* 1 us, in picoseconds */

/* 4. Read it back. */
CircuitValue v = circuit_net_value(c, anode);          /* CIRCUIT_1 */
unsigned char lit;
circuit_part_lights(c, led, &lit, 1);                  /* 1 */

circuit_destroy(c);
part_library_destroy(library);         /* the library outlives its circuits */
```

Notes:

- Building calls (`circuit_add_part`, `_add_net`, `_connect`) work only
  before `circuit_start`; `circuit_set_switch` works before and after.
- `circuit_advance` returns 0 once the run has stopped (a short circuit is
  `"smoke: ..."`, and `circuit_shorted_net` names the net).
- **Voltages** (`voltage.h`, plan-07). The simulation is logic-only; a
  separate static check, `voltage_check`, works out each net's voltage from
  its supplies (`PartSupply.mv`), the outputs of powered parts and what an
  LED carries from anode to cathode, and reports every power pin outside
  its part's `vcc` range, input above its part's input max, switch contact
  above its rating, and two different non-zero supplies on one net.
  `voltage_describe` gives the text, e.g. `voltage: 5 V on pin 14 (VCC) of
  SN74AUC00PWR; it needs 0.8-2.7 V`. It takes plain arrays (parts,
  pin→net connections, closed contacts), so the app runs it on a candidate
  netlist to refuse an edit.
  - `circuit_start` runs it first and refuses a circuit with a problem
    (-1, `circuit_error` = the text; building calls stay usable).
  - A switch that closes during a run and makes the circuit fail stops the
    run like a short: `circuit_advance` returns 0 and `circuit_error` is the
    text (the contact is still applied).
  - `circuit_voltage_problem` returns the first problem (net, part, pin,
    voltages) after either refusal.
  - A supply and GND on one net is still a short (`smoke:`), not a voltage
    problem; a net with no known voltage, and a part with no `vcc` data,
    are not checked.
  - A pull-up part (`PartSupply.pull`) is not a supply here: it gives its
    group a level only when no supply and no powered output sets it
    (E = S, else D, else P, else C), and never clashes.
- Lights (`circuit_part_lights`) are LEDs (one light), 7-segment
  displays (`PartDisplay.leds` order) and character LCD backlights (one
  light, on while the anode reads 1 and the cathode 0; a module without a
  backlight has none); a light counts as on if it was on at any time during
  the last `circuit_advance`, so multiplexed digits show.
- Memory contents (SRAM, EEPROM): `circuit_memory_size`,
  `circuit_memory_read` (a 0 bit in `known` is X).
- Processors (`cpu` kind): `circuit_cpu_state` fills a `CpuState`
  (`cpu_state.h`): status (running, lost, reset, waiting, stopped, halted,
  jammed, bus released), cycles since the last reset ended, the registers
  (none while lost or in reset) and the last completed instruction
  (address, bytes, disassembly). Any other kind: -1, `part N is not a
  processor`. The app's processor window reads it on every simulation
  frame, as the memory window reads `circuit_memory_read`.
- Character LCDs (`charlcd` kind): `circuit_lcd_state` fills an
  `LcdState` (`lcd_state.h`): the dot image the display shows now
  (`dots[row * 8 + y][column * 5 + x]`, each `LCD_DOT_OFF`, `_ON` or `_X`;
  the cursor blink phase comes from the circuit time), the busy flag (1, 0
  or -1 for X), and DDRAM and CGRAM with known masks. The image does not
  depend on the backlight or V0; the app dims the face by the backlight
  light. Any other kind: -1, `part N is not a character LCD`.
- Serial parts (`serial` kind, plan-12): `circuit_serial_channels` gives
  a part's channel count (0 for any other kind);
  `circuit_serial_sent_count` and `circuit_serial_sent` give the frames a
  channel has sent on its TX pin since start (each the frame's data bits),
  recorded on the line, so loopback, break and a stopped transmitter show
  as they are. `circuit_serial_send` types bytes into a channel: the model
  drives its own RX pin strong, one bit per 16 receive ticks in the
  channel's current format, and releases it after the last stop bit. It is
  refused before start, for a part that is not serial, for a channel that
  does not exist or is not asynchronous, and while another part's output,
  in/out or strong supply pin is on the RX net (pull parts are fine); the
  messages are in `circuit.h`. The app's terminal window is built on these.
- Memory load: `circuit_memory_load` writes words into an EEPROM before
  `circuit_start` (refused after it, and on every other kind, SRAM
  included); words it does not write stay erased (FF). Build the words with
  `memory_image_parse` (`memory_image.h`): every rule and message is in
  `planning/completed/plan-04-B-memory-load.md` §4.3.
- `circuit_net_changes` counts how often a net's value has changed since
  `circuit_start`; compare two reads to see a net that toggled in between.
- `circuit_part_light_time` gives, per light (same order as
  `circuit_part_lights`), the picoseconds it has been on since start;
  divide the increase over a frame by the frame's length for brightness.
- Gate and flip-flop outputs are inertial: a newer drive cancels a pending
  one (pulses shorter than the delay vanish), except that a drive of the
  value already pending, no earlier than it, keeps the pending edge.

## Layout

```
backend/
  include/                 public API: parts.h, circuit.h, voltage.h,
                           memory_image.h
  src/
    engine/                sim.c sim.h net.c net.h (the event engine)
    library/               library.c (load_parts, load_footprints), kinds.c
                           (KINDS[]), kind.h + kind_util.c (the kind
                           interface), footprints.c + footprint.h (parse,
                           check and expand footprints.json)
    models/                one file per kind: gate.c dff.c jkff.c sram.c
                           eeprom.c switch.c led.c display7.c oscillator.c
                           supply.c latch.c,
                           charlcd.c (character LCD modules: loader and
                           the HD44780 controller model), charlcd_font.c
                           + charlcd_font.h (character ROM A00),
                           seq.c + seq.h (helpers the sequential kinds
                           share: timing entries, controls, X rules,
                           outputs), and lights.c (on-time tracking for
                           lights);
                           model_pins.h (LED, switch and open-drain pin
                           facts for the voltage check);
                           cpu.c + cpu.h (the cpu kind: loader, the model
                           that forwards to a core adapter, input sampling,
                           the instruction tracker), cpu6502.c (the
                           6502/65C02 bus adapter), cpuz80.c (the Z80 bus
                           adapter: a commit scheduler around the core),
                           cpu65816.c (the W65C816S bus adapter: the bank
                           on D0-D7, VDA/VPA, E and MX, ABORTB, on the
                           jsmooch core);
                           chips_m6502.c chips_w65c02.c chips_z80.c
                           chips_dasm.c (each instantiates one vendored
                           floooh/chips core, or the three disassemblers);
                           serial.c + serial.h (the serial kind: loader,
                           model table, the model that forwards to a serial
                           model, the terminal hooks), serial_line.c (baud
                           generator, transmitter, receiver, sent log,
                           terminal sender), serial_bus.c (Intel host bus),
                           ns16550.c (the 16450/16550 family);
                           pio.c + pio.h (the pio kind: loader, model
                           table, the model that forwards to a pio model),
                           bus65.c + bus65.h (the 65xx peripheral bus the
                           pio models share)
    circuit.c
    voltage.c              the voltage check (voltage.h)
    memory_image.c         load files (binary, Intel HEX) to memory images
    third_party/cjson/     cJSON v1.7.18
    third_party/chips/     floooh/chips m6502.h w65c02.h z80.h and their
                           disassemblers (zlib), unmodified; README.md
                           names the commit
    third_party/jsmooch/   jsmooch's cycle-stepped WDC 65816 core and
                           disassembler (MIT), fixed through its code
                           generator, plus our helpers/ stubs;
                           README-jsmooch.md names the commit and lists
                           every change
  tools/jsmooch-gen/       the 65816 core's JavaScript code generator (MIT)
                           and run.js, which regenerates
                           wdc65816_generated_opcodes.c (`node run.js`);
                           outside src/, so the app does not bundle it
  tests/                   CTest programs; golden/ holds the regression
                           fixtures (one listing line and one trace hash
                           per part); cpu/ holds the SingleStepTests
                           subsets (MIT) for cpu_core_test and the script
                           that made them
```

The app compiles `backend/src/` directly (an Xcode synchronized folder).

## Adding a kind

A new kind of part is added the same way every time:

1. `backend/include/parts.h`: a `PART_<KIND>` value and, if the app needs
   facts about it, an info struct and its pointer on `Part`.
2. `backend/src/models/<kind>.c`: the `PartKindOps` (device and part keys,
   `load_device`, `bind_pinout`, `load_part`, and optional `pin_open_drain`
   for a kind with open-drain outputs, which the voltage check asks through
   `part_pin_open_drain`) and the engine callbacks
   (`SimComponentType`; the engine config is the `const Part *`, model data
   through `part_internal(part)`). Use the `KindContext` helpers in
   `kind.h` for keys, pins by name, times and errors.
3. One line in `KINDS[]` (`backend/src/library/kinds.c`).
4. `schematical/schematical/Parts/<kind>.json`, and its section in
   `docs/parts-format.md`.
5. The app picks a symbol for the new `PartKind`.

Nothing else: the loader, the listing, the circuit API and the app's
drawer work from the kind table and the `Part` structs. The tests need
three lines: the kind's name and facts in `tests/support/listing.c`,
`TRACE_LAST_KIND` in `tests/trace_check.c` moved to the new last
`PartKind` (its per-kind count array is sized by it), and the same last
`PartKind` in the per-kind arrays of `tests/listing_check.c`.

## Tests

`cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build`

- `library_check` — the loader's rules, every rejection case.
- `listing_check`, `trace_check` — every part's facts and a hash of its
  simulated behavior against `tests/golden/` (recorded from the original C
  tables before the move to JSON). A change to a part file that changes
  facts or behavior, or adds a part, shows up here. For an intended change,
  regenerate the goldens and review the diff:

  ```
  /tmp/logic-build/listing_check --print > backend/tests/golden/parts.txt
  /tmp/logic-build/trace_check --print > backend/tests/golden/traces.txt
  git diff --stat backend/tests/golden
  ```
- `footprint_check` — every footprint in `footprints.json`, expanded to
  pads, against `tests/golden/footprints.txt` (one line per footprint:
  `id|pins=N|body=WxH|pads=<designator>:x,y,wxh,shape[,d<drill>][,r<radius>][,c<chamfer>:<corners>];…`,
  corners `tl+tr+br+bl` order; `-`
  for an unnumbered pad), and every footprint used by some part (`footprint`
  or `connectorFootprint`). After an
  intended change: `/tmp/logic-build/footprint_check --print >
  backend/tests/golden/footprints.txt`, then hand-check the changed lines
  against the drawing. Listing lines in `parts.txt` end in
  `|footprint=<id>` for parts that name one, then `|connector=<id>` for
  parts with a `connectorFootprint`.
- `switch_test`, `sram_test`, `display7_test` — model benches.
- `latch_test` — the `latch` kind's five models, one case per function-table
  row of the devices that use them (inline fixture devices copied from the
  real ones), plus loader errors and the open-drain answer. Its bench,
  `tests/support/bench.{h,c}`, drives any part's input pins by name and is
  shared by the sequential kinds' tests.
- `eeprom_test` — the EEPROM model's read window, page loads, write cycle,
  DATA# polling and toggle bit, timing violations and software data
  protection (an inline fixture device, then real parts).
- `engine_test` — engine timing with a real gate (SN74HC32N): an output
  edge that is already pending is kept, a too-short pulse is swallowed.
- `circuit_check` — the circuit API, including the voltage refusal at
  start and the running switch stop (fixture parts from
  `tests/support/voltage_fixture.h`).
- `voltage_test` — `voltage_check` and `voltage_describe`, one case per
  rule (supplies, power range, input max, driven and carried voltages,
  open-drain, closed contacts, switch rating, ordering), on the fixture
  parts and the real supplies.
- `memory_image_test` — the load-file parser: format by file name, raw
  binary, every Intel HEX rule and message.
- `cpu_core_test` — the vendored CPU cores (`third_party/chips/`) against
  `tests/cpu/`: the first SingleStepTests case of every opcode file (6502
  256, 65C02 254, Z80 1604; `make_sst_subset.py` regenerates them). The
  6502 family is checked cycle by cycle (address, data, R/W) and on its final
  registers and RAM; the Z80 on its final registers, RAM, T-state count and
  bus transactions. The test names the cores' known deviations (6502 JAM and
  SHA/SHY/SHX, Z80 SCF/CCF, interrupted block repeats, HALT). Each of those
  must still fail, so a core update that fixes one is noticed; any other
  failure, or a listed test that passes, fails the test. It prints
  `6502: 241 pass, 15 expected failures` and the same for 65C02 and Z80.
  The 65816 (`third_party/jsmooch/`, plan-14-G) runs the first 10 tests of
  each of the 512 SingleStepTests/65816 files (`sst-65816.jsonl`, 5120;
  `make_sst_subset.py … 10`), checking every cycle's 24-bit address, data
  and the eight flags VDA VPA VPB RWB E M X MLB, then the final registers
  and RAM: `65816: 5120 pass, 0 expected failures`. Then an ABORT case
  (`65816: abort ok`) and, when CMake finds `node`, a check that
  `tools/jsmooch-gen/run.js` reproduces the committed
  `wdc65816_generated_opcodes.c` (`65816: generator reproduces …`). Set
  `SST65816="<test name>"` to print one test's cycles side by side.
- `cpu_test` — the cpu kind's bus adapters on inline fixture devices (the
  W65C02S and MOS 6502 data of `cpu.json`) and a bench that clocks the CPU
  at 1 MHz and serves a 64 KB memory (X where it was given nothing): the
  power-up LOST state, the reset sequence, the address and write windows at
  the datasheet times, setup violations and used unknowns (LOST), IRQ, NMI,
  RDY, SO, the register and instruction read-back, clock-width checks,
  and the W65C02S VPB, MLB, BE, WAI and STP and NMOS JAM rules; plus
  `circuit_cpu_state`. A second bench drives a Z84C00 fixture the same way
  and logs every machine cycle (type, address, data, T1, strobe times):
  the fetch, memory, I/O and int-ack waveforms at the PS0178 edges, WAIT,
  IM 0/1/2, NMI, HALT, BUSREQ, data setup and clock checks, and a program
  from reset to HALT. Last, three whole boards of library parts
  (`tests/support/cpu_board.{h,c}`: the CPU, an AT28C256-15PU, an
  AS6C62256-55PCN and an SN74HC00N decoder) run a program for 2 ms: the
  W65C02S6TPG-14 and MPS6502 count to 10 in SRAM, the Z84C0010PEG also
  CALLs a store and halts. The W65C816S cases (`w65c816s_h1` … `h13`,
  plan-14-H) run an inline W65C816S on a 24-bit bench that latches the
  bank at each PHI2 rise: reset and the vector pull, the bank on D0–D7 and
  the write window at the Table 4-2 edges, VDA/VPA/VPB equal to the bare
  core's, MX and E, MLB, ABORTB, RDY, WAI/STP, BE, setup, and the 24-bit
  state and tracker (`ok w65c816s_…`). The W65C816S board (plan-14-I:
  `cpu_board`'s `65816` layout, an SN74HC573AN bank latch and an
  AS6C1008-55PCN) runs a native-mode program that stores to banks 0 and 1
  and reads both back (`board_w65c816s ok`). The I/O board (`tests/support/io_board.{h,c}`,
  plan-14-E) adds a W65C22S VIA, a W65C21N PIA and a W65C51N ACIA (TxD
  looped to RxD, on an ECS-2100A-018) with three SN74HC00N decoders: its
  program sets the ports, sends `HI`, takes a VIA Timer 1 interrupt and
  reads the looped-back byte (`io_board ok`).

- `serial_test` — the serial kind on fixture devices
  (`tests/support/serial_fixture.h`: a PC16550D-shaped `ns16550` in plcc44
  and dip40, and a 16450 twin) with a bench that clocks XIN and wires
  BAUDOUT to RCLK: reset values, divisor and BAUDOUT, transmit frames and
  bit times, receive (false start, framing error and resync, parity,
  break), FIFO triggers and overrun, the timeout, interrupt priorities and
  the transmitter-interrupt delay, modem status, loopback, DMA modes, bus
  timing and strobes, the 16450, and the terminal (8N1 at 115200 on
  1.8432 MHz sends `H i`; typed `OK` reads back). `circuit_check` drives
  the same UART through DIP switches and pull-ups for the
  `circuit_serial_*` API and its error messages.
- `pio_test` — the pio kind (plan-14). A1–A8 drive the 65xx bus helper
  (`bus65`) through a test-only 16-register component on the bench at the
  W65C22S 14 MHz times: the read window (X at the rise, data at + `tcdr`,
  Z at fall + `thr`), write data setup and hold, select/RS setup and hold,
  not selected, PHI2 X, the per-fall `cycle()` hook and PHI2 width checks.

Not a ctest: `sim_bench` (`tests/sim_bench.c`) measures engine speed — a
D flip-flop ripple counter on an oscillator plus N idle SN74HC00N chips,
reported as a multiple of real time. Build it Release; wall-clock numbers
would make ctest flaky, so it has no `add_test`.

```
cmake -S backend -B /tmp/logic-rel -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/logic-rel --target sim_bench
/tmp/logic-rel/sim_bench LMK6CE001200CDLFR 16 10 400   # <osc> <stages> <sim ms> <idle chips>
```

`cpu_bench` (`tests/cpu_bench.c`, also not a ctest) runs the W65C02S board
of `cpu_test` with its program looping forever and reports the speed:

```
cmake --build /tmp/logic-rel --target cpu_bench
/tmp/logic-rel/cpu_bench 100   # <sim ms>
```

The bench runs on the `3V3` supply, so the oscillator must take 3.3 V
(LMK6CE001200CDLFR is a 1.2 MHz, 2.37-3.5 V part; the 1.8 V-option
LMK6CE001000DDLER is refused at start by the voltage check).
