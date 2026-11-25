# plan-62-A: AVR simulation — the avr8js oracle and the CPU core in C

Last updated: 2026-11-24
Overall Effort: huge (> 3d)
Effort: large (3h–1d)
Depends on: nothing

plan-62 makes the ATmega48/88/168/328 family run firmware in the simulator.
The simulation core is a C port of [avr8js](https://github.com/wokwi/avr8js)
0.21.1 (MIT, TypeScript), written directly into `backend/src/models/`, and
avr8js itself, run under Node, is the oracle every ported piece is checked
against. The single outcome of the whole plan: an `ATMEGA328P-PU` on a
schematic with a firmware `.hex` chosen in its Processor window runs that
firmware when the user presses Run, at the pin level, and the C core's
behavior equals avr8js's except where this plan says otherwise.

The letters, in implementation order:

| Letter | Delivers |
|---|---|
| A (this file) | the vendored oracle, the oracle driver, the CPU core (`avr_cpu.c`, `avr_instruction.c`), `avr_core_test` |
| B | GPIO ports, the clock prescaler, timers 0/1/2, the chip assembly (`avr_chip.c`), scenario traces |
| C | USART, SPI, TWI, ADC, EEPROM, watchdog at register level |
| D | the three register layouts of the 18 devices, vector size, the disassembler |
| E | the engine adapter: the `mcu` kind's `"model"` key, pins, clock, reset, GPIO, ADC, flash load, `circuit_cpu_state` |
| F | pin-level USART, SPI master and I2C master |
| G | the 18 devices become simulated in `mcu.json`; goldens, board tests, docs |
| H | the app: firmware load file, Processor window, tests, docs |
| I | the landing tier |

A delivers the part with no callers: the oracle and a CPU core that matches
it instruction by instruction. Nothing in the library, the engine or the app
changes.

References:

- avr8js 0.21.1: `https://registry.npmjs.org/avr8js/-/avr8js-0.21.1.tgz`
  (sha256 `febfe96e73a840cdb6cc6e583433af2badb8c85421f38545cf5d802802ed35e8`),
  MIT, "Copyright (c) Uri Shaked and contributors"; source at
  `wokwi/avr8js` commit `bee6f0a94e0e27786f6bc21aee3c775849fb50fd`
  (`package.json` version 0.21.1). The TypeScript sources this letter ports:
  `src/cpu/cpu.ts` (272 lines), `src/cpu/instruction.ts` (801),
  `src/cpu/interrupt.ts` (25).
- Microchip, *AVR Instruction Set Manual* (DS40002198): flags and cycle
  counts for § 3.3.
- `docs/backend.md` (layout, tests); `backend/CMakeLists.txt`;
  `planning/completed/plan-14-G-65816-core.md` (the precedent: a core checked
  against vectors, a Node tool run by a test when CMake finds `node`).
- `planning/plan-62-data/` (the ATDF comparison, used from D on).

## Prerequisites

These are a precondition on the whole feature; letters B–I point here.

| Must be true | Command | Status |
|---|---|---|
| The worktree is on its own branch | `git branch --show-current` → `worktree-sim` | MET (2026-11-24: `worktree-sim`) |
| The backend builds and its tests pass | `cmake -S backend -B /tmp/sim-rel -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/sim-rel -j8 && ctest --test-dir /tmp/sim-rel -j8` → `100% tests passed, 0 tests failed out of 66` | MET (2026-11-24: 66 of 66, 20 s) |
| Node.js 18 or later | `node --version` | MET (2026-11-24: `v24.12.0`) |
| The npm registry is reachable (A vendors the tarball once) | `npm view avr8js@0.21.1 dist.integrity` → `sha512-lS1vPaB0…9rBaw==` | MET (2026-11-24) |
| The Microchip pack server is reachable (D reads the ATDF files) | `curl -s -o /dev/null -w '%{http_code}' https://packs.download.microchip.com/` → `200` | MET (2026-11-24: 200) |
| No other plan in flight edits `mcu.c`, `mcu.json` or `Cpu/CpuWindow.swift` | `grep -l 'mcu\.c\|mcu\.json\|CpuWindow' planning/plan-*.md \| grep -v plan-62` → no output | MET (2026-11-24: no output) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of all rows.

## 1. Goal

- `backend/tools/avr8js/` holds avr8js 0.21.1's `dist/cjs` JavaScript,
  unmodified, with its licence; `backend/tools/avr8js-oracle/oracle.js` runs
  it.
- `backend/src/models/avr.h`, `avr_cpu.c` and `avr_instruction.c` are the
  CPU core: registers and data memory, read and write hooks, the interrupt
  queue, clock events, and every instruction.
- `ctest -R avr_core_test` passes and prints
  `step: 262144 vectors match avr8js` (exact mode, all 65536 opcode words,
  4 random states each) and `fixes: 7 ok`.

### Non-goals (explicit constraints)

- No peripheral (B, C), no part, kind or engine change (E, G), no app
  change (H). `ctest` keeps its 66 programs and gains one; goldens do not
  move.
- avr8js's JavaScript is not edited. Anything the oracle needs beyond its
  public classes lives in `oracle.js`.
- The JavaScript is not bundled in the app: it sits under `backend/tools/`,
  outside the Xcode synchronized folder `backend/src/`.
- The instructions avr8js leaves empty stay empty: `SLEEP`, `SPM`, `BREAK`
  and every unassigned opcode are a one-cycle no-op. No sleep modes, no
  self-programming.

## 2. Current State

- The `mcu` kind is facts only: `backend/src/models/mcu.c` refuses any
  device that does not say `"simulated": false` (`mcu_load_device`) and
  ships a passive model. `circuit_add_part` refuses unsimulated parts
  (`backend/src/circuit.c:141`).
- Vendored cores today: `backend/src/third_party/chips/` (zlib) and
  `third_party/jsmooch/` (MIT) with its generator in
  `backend/tools/jsmooch-gen/`; `cpu_core_test` runs that generator when
  CMake finds `node` (`backend/CMakeLists.txt`, `find_program(LOGIC_NODE node)`).
- avr8js's CPU is one class and two functions. `CPU` owns `data`
  (`Uint8Array(sramBytes + 0x100)`, registers at 0–31, I/O from 32, SP at
  93–94, SREG at 95), `progMem` (`Uint16Array`), per-address `readHooks` and
  `writeHooks`, `pendingInterrupts[128]` with `nextInterrupt` /
  `maxInterrupt`, and a linked list of clock events ordered by cycle (a new
  event goes before existing events of the same cycle). `tick()` runs at most
  one due clock event, then takes the lowest pending interrupt when I is set.
  `avrInstruction` decodes by a chain of mask tests and always ends with
  `pc = (pc + 1) % progMem.length; cycles++`.

### Measured populations

| What | Count | Command |
|---|---|---|
| ctest programs before A | 66 | `ctest --test-dir /tmp/sim-rel -N \| tail -1` → `Total Tests: 66` |
| Lines to port in A | 1098 | `wc -l src/cpu/cpu.ts src/cpu/instruction.ts src/cpu/interrupt.ts` (272 + 801 + 25) |
| Decode branches in `avrInstruction` | 99 | `grep -c 'else if\|^  if (' src/cpu/instruction.ts` → 99 |
| Vendored JavaScript | 18 files, 174765 bytes | `find package/dist/cjs -name '*.js' \| wc -l`; `… -exec cat {} + \| wc -c` |
| Oracle time for 262144 single steps | 2.4 s | probe `probe2.js` (below) → `262144 vectors 5553336 bytes 2381 ms` |

### Verified properties

- **avr8js runs from its published `dist/cjs` under Node 24 with no
  dependency, and its assembler is in the package.** Probe (2026-11-24):
  `require('dist/cjs/index.js')` and `require('dist/cjs/utils/assembler.js')`;
  a 5-instruction program assembled to `0fef04b9139515b9fdcf`, ran 12
  instructions on `new CPU(prog)` with `new AVRIOPort(cpu, portBConfig)`,
  and the port listener logged `[[1,0],[3,1],[7,2],[11,3]]`.
- **The 7 places where avr8js's CPU disagrees with the manual** (read in
  `instruction.ts` and `interrupt.ts`, 2026-11-24), which § 3.3 fixes:
  1. H is taken from bit 0 of the half-carry expression, not bit 3, in ADD,
     ADC, SUB, SUBI, SBC, SBCI, CP, CPC, CPI and NEG (`1 & (…) ? 0x20 : 0`).
     ADD 0x08 + 0x08: avr8js H = 0; the manual gives H = 1.
  2. SBIW rewrites H; the manual leaves H unchanged.
  3. CPC, SBC and SBCI test `!R` on the unmasked difference, so 0 − 255 − 1
     (= −256, byte 0) clears Z where the manual keeps the previous Z.
  4. NEG tests `128 === R` with `R = 0 − value` (−128 for 0x80), so V is
     never set; the manual sets V when the result is 0x80.
  5. FMULSU sets Z when the product is not zero (`0xffff & R ? 2 : 0`).
  6. CBI takes 1 cycle (no `cycles++`); the manual says 2.
  7. An interrupt adds 2 cycles (`avrInterrupt`); the ATmega328P datasheet
     says the response takes 4.
- **JavaScript behaviors the C code must reproduce** (read in the same
  files): a store to `data[]` keeps the low 8 bits of any number
  (`cpu.data[d] = value - 1` with value 0 stores 255); a read past the end
  of `data` gives `undefined`, which stores as 0, and a write past the end
  is dropped; `ELPM(INC)` computes `% (progBytes.length >> 16)`, which is
  `% 0` on a flash of 64 KB or less and stores 0 in RAMPZ.

## 3. Design Overview

Three independent pieces: the vendored oracle, the oracle driver, the C
core. The test compares them.

Correctness risk concentrates in `avr_instruction.c`: 99 decode branches,
each with flag arithmetic that JavaScript evaluates on doubles and 32-bit
bitwise integers. The step test covers every opcode word, so a wrong branch
shows as a named opcode. **Byte-identity is not a gate of this letter**: no
existing output is expected to stay or to move.

Probed before writing: the oracle runs (above); 262144 single steps take
2.4 s in Node, so the full sweep is the test, not a subset.

Rejected alternatives:

- *Committing step vectors* (the `tests/cpu/` SingleStepTests precedent):
  262144 lines are 5.5 MB. Both sides generate the same states from one
  xorshift32 seed instead, and the repository keeps one hash.
- *Running avr8js from TypeScript source*: needs a TypeScript build step;
  the published `dist/cjs` is the same code, already JavaScript.
- *Vendoring under `backend/src/third_party/`*: the C is our own port, not
  avr8js's files, and the JavaScript must stay out of the app bundle.
- *Masking the 7 fixed behaviors in the comparison*: the interrupt cost
  shifts every later cycle count of a scenario, so nothing downstream could
  be compared. An exact mode (§ 3.3) keeps the comparison total.

### 3.1 The vendored oracle

`backend/tools/avr8js/`: the 18 `.js` files of the tarball's
`package/dist/cjs/` at their paths, `LICENSE` (the tarball's), and
`README.md` naming the version, the tarball URL, its sha256 and the source
commit. No `.d.ts`, no `esm`.

`backend/tools/avr8js-oracle/oracle.js` (Node, CommonJS, no dependency) has
one subcommand in A:

- `oracle.js step <seed> <states>`: for every opcode word 0…65535 and
  `states` states each, sets the state, runs `avrInstruction(cpu)` once, and
  prints one line (§ 3.4). The CPU is `new CPU(new Uint16Array(0x4000), 2048)`
  (ATmega328P sizes).

B adds `run`, `asm` and `layout` to the same file.

### 3.2 The C core

`backend/src/models/avr.h` declares it; `avr_cpu.c` is `cpu.ts` plus
`interrupt.ts`; `avr_instruction.c` is `instruction.ts`. Function for
function:

| avr8js | C |
|---|---|
| `new CPU(progMem, sramBytes)` | `AvrCpu *avr_cpu_create(uint32_t flash_bytes, uint32_t sram_bytes)`; `avr_cpu_destroy` |
| `cpu.reset()` | `avr_cpu_reset` |
| `data`, `progMem`, `progBytes`, `pc`, `cycles`, `pc22Bits` | fields `uint8_t *data; uint32_t data_size; uint8_t *flash; uint32_t flash_words; uint32_t pc; uint64_t cycles; int pc22` (flash is bytes, little-endian words) |
| `readHooks[addr]`, `writeHooks[addr]` | `AvrReadHook` / `AvrWriteHook` arrays indexed by address, each `{fn, ctx}`; a write hook returns 1 when it handled the write |
| `readData`, `writeData(addr, value, mask)` | `avr_read_data`, `avr_write_data` |
| `AVRInterruptConfig` | `AvrInterrupt {address, enable_register, enable_mask, flag_register, flag_mask, constant, inverse_flag}` |
| `setInterruptFlag`, `updateInterruptEnable`, `queueInterrupt`, `clearInterrupt`, `clearInterruptByFlag` | `avr_set_interrupt_flag`, `avr_update_interrupt_enable`, `avr_queue_interrupt`, `avr_clear_interrupt`, `avr_clear_interrupt_by_flag` |
| `addClockEvent`, `updateClockEvent`, `clearClockEvent` | `avr_add_clock_event(cpu, fn, ctx, cycles)`, `avr_update_clock_event`, `avr_clear_clock_event`; an event's identity is its `(fn, ctx)` pair |
| `tick()` | `avr_cpu_tick` |
| `avrInstruction(cpu)` | `avr_instruction(cpu)` |
| `avrInterrupt(cpu, addr)` | `avr_interrupt(cpu, addr)` |
| `onWatchdogReset` | `cpu->watchdog_reset {fn, ctx}` |
| `gpioPorts`, `gpioByPort` | added in B |

Rules of the port:

- `pc` is the word index, as in avr8js. After each instruction
  `pc = (pc + 1) mod flash_words`, computed so a `pc` of −1 (an `IJMP` to 0)
  becomes 0, as JavaScript's `%` does.
- A data read at or past `data_size` gives 0 and a write there is dropped
  (the typed-array behavior).
- Clock events are a singly linked list in `cpu`. Nodes are allocated on
  demand and recycled through a free list that `avr_cpu_destroy` frees: the
  USART adds one event per UDR write (`usart.ts`, the UDR write hook), so no
  fixed count bounds them. Insertion keeps avr8js's order: before the first
  event whose cycle is not smaller.

### 3.3 The fixes and exact mode

`AvrCpu` has `int exact`. With `exact` 0 (the default, what the simulator
runs) the 7 behaviors of § 2 follow the manual. With `exact` 1 they follow
avr8js; only the oracle tests set it. Each of the 7 sites carries a comment
in the C code that states what avr8js does, what the manual says, and that
`exact` selects avr8js's result. The form:

```c
/* avr8js takes H from bit 0 of this expression; the instruction set manual
   takes it from bit 3 (the carry out of the low nibble). We follow the
   manual. `exact` keeps avr8js's result so the oracle tests compare whole
   states. */
```

The interrupt fix: `avr_interrupt` adds 4 cycles (2 in exact mode).

### 3.4 The step comparison

State per vector, drawn in this order from xorshift32 (`s ^= s << 13;
s ^= s >> 17; s ^= s << 5`, 32-bit, seed from the command line):

1. every 4 bytes of `data` from one draw, little-endian;
2. `pc = 0x1000 + draw % 0x2000` (a relative branch of ±2048 words stays
   inside the 0x4000-word flash);
3. `flash[pc] = opcode`, `flash[pc + 1] = draw & 0xffff`,
   `flash[pc + 2] = draw & 0xffff`;
4. `SP = 0x108 + draw % 0x7f0` (a 3-byte push or pop stays inside `data`);
5. `cycles = 0`.

The line: `%04x %04x %x %02x %04x %08x\n` = opcode, `pc` after, `cycles`
after, SREG, SP, FNV-1a 32 of `data`. `avr_core_test` generates the same
lines with `exact` 1, hashes all of them with FNV-1a 64, and compares with
the committed `backend/tests/avr/golden.txt` line `step <seed> <states>
<lines> <hash>`. When CMake found `node`, the test also runs `oracle.js
step` through `popen`, compares its output line by line and prints the first
difference as `step: opcode XXXX state N: avr8js "<line>" ours "<line>"`.
The golden hash is written from the oracle's output, never from the C
side's.

### 3.5 The fix tests

`avr_core_test` then runs with `exact` 0 and checks, against reference
functions written from the manual in the test itself:

1. H for ADD, ADC, SUB, SUBI, SBC, SBCI, CP, CPC, CPI over every operand
   pair and both carries, and NEG over every operand;
2. SBIW leaves H as it was, both values, over every 16-bit value and K;
3. Z for CPC, SBC, SBCI over every operand pair, both carries, both
   previous Z;
4. V for NEG over every operand;
5. Z for FMULSU over every operand pair;
6. CBI takes 2 cycles;
7. a taken interrupt adds 4 cycles and pushes the return address.

It prints `fixes: 7 ok`.

## Compatibility / Format Impact

None. No public header, part file, document format or golden changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Fill `Commit:`
> when the phase lands. An unticked box means NOT DONE.

### Phase 1 — the oracle in the tree

- [x] Download `https://registry.npmjs.org/avr8js/-/avr8js-0.21.1.tgz` to
      `/tmp/p62/`, check `shasum -a 256` equals the hash in References, and
      copy `package/dist/cjs/**/*.js` (18 files) and `package/LICENSE` to
      `backend/tools/avr8js/` (the `.js` files under `dist/cjs/`).
- [x] Write `backend/tools/avr8js/README.md`: version, tarball URL, sha256,
      source commit, "unmodified", and that only `oracle.js` uses it.
- [x] Write `backend/tools/avr8js-oracle/oracle.js` with the `step`
      subcommand of § 3.1 and § 3.4.

Acceptance: the oracle prints the step lines.
  Check: `node backend/tools/avr8js-oracle/oracle.js step 305419896 4 | wc -l` → `262144` (est. 5 s).
  Observed 2026-11-24: `262144`; sha256 of the tarball equal; 18 `.js` files; no `nan`/`undefined` in the output.
Commit: b1932c90

### Phase 2 — the CPU core

- [x] `backend/src/models/avr.h`: the types and functions of § 3.2.
- [x] `backend/src/models/avr_cpu.c`: `cpu.ts` and `interrupt.ts`, with the
      interrupt fix and its comment.
- [x] `backend/src/models/avr_instruction.c`: `instruction.ts`, all 99
      branches in the same order, with fixes 1–6 and their comments.
- [x] `backend/src/models/LICENSE-avr8js`: avr8js's MIT licence text, and a
      line that `avr_*.c` are C ports of avr8js 0.21.1. Each `avr_*.c` and
      `avr.h` starts with a comment naming the avr8js file it ports and the
      licence file.
- [x] `backend/CMakeLists.txt`: add `src/models/avr_cpu.c` and
      `src/models/avr_instruction.c` to the `logic` library.

Acceptance: the library builds with the two files.
  Check: `cmake --build /tmp/sim-rel --target logic 2>&1 | grep -c 'warning\|error'` → `0` (est. 1 min).
  Observed 2026-11-24: `0`.
Commit: 67ce943d

### Phase 3 — `avr_core_test`

- [x] `backend/tests/avr_core_test.c`: the step comparison of § 3.4 (golden
      hash always; the live oracle when `AVR_NODE` is defined), the fix tests
      of § 3.5, and three unit cases for the parts the step test cannot
      reach: clock events fire in cycle order with a same-cycle event added
      later firing first; `avr_update_clock_event` moves an event and
      `avr_clear_clock_event` removes one; with two interrupts queued the
      lower address is taken first and a `constant` interrupt stays queued.
- [x] `backend/tests/avr/golden.txt`: the `step` line, hash computed from
      `oracle.js step 305419896 4`.
- [x] `backend/CMakeLists.txt`: `logic_test(avr_core_test)` with
      `AVR_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/avr"` and, inside the
      existing `if(LOGIC_NODE)`, `AVR_NODE` and
      `AVR_ORACLE="${CMAKE_CURRENT_SOURCE_DIR}/tools/avr8js-oracle/oracle.js"`.
- [x] `docs/backend.md`: § Layout gains `tools/avr8js/`,
      `tools/avr8js-oracle/` and the `avr_*.c` files; § Tests gains
      `avr_core_test`.

Acceptance: the C core equals avr8js on every opcode word, and the 7 fixes
hold.
  Check: `cmake --build /tmp/sim-rel -j8 && ctest --test-dir /tmp/sim-rel -R avr_core_test --output-on-failure` → `step: 262144 vectors match avr8js`, `fixes: 7 ok`, `1/1 Test … Passed` (est. 1 min).
  Observed 2026-11-24: `step: 262144 vectors match avr8js`, `step: 99 of 99 decode branches ran`, `fixes: 7 ok`, `unit: ok`; golden line `step 305419896 4 262144 7780baee2f7b9ffa` from `avr_core_test --print-golden`.
Commit: 67ce943d

## Validation Plan

- Tests: `avr_core_test` (above). Negative case: the test's own
  self-check flips one golden hash digit in memory and expects the
  comparison to fail, so a comparison that always passes is caught.
- Coverage check: the step test executes all 99 branches because every
  opcode word is run; the test counts the distinct branch ids the core
  reports in a test-only counter array and requires 99.
- Runtime proof: none in A (no part uses the core yet; E).
- Doc sync: `docs/backend.md` § Layout and § Tests.
- Final gate: plan-62-I.

## Decisions

- The port matches avr8js except the 7 CPU behaviors of § 2, which follow
  Microchip's manual, each with a comment in the C code explaining the
  difference — decided by the owner, 2026-11-24. (§ 3.3)
- The chips this plan simulates are the ATmega48/88/168/328 family: 18
  devices, 126 parts — decided by the owner, 2026-11-24. (D, G)
- A chip takes its clock from XTAL1 when that pin is clocked, otherwise from
  its internal oscillator at 1 MHz (8 MHz divided by 8, the factory fuses) —
  decided by the owner, 2026-11-24. (E)
- USART, SPI and I2C work at the pin level in this plan — decided by the
  owner, 2026-11-24. (F)
- The plan includes the app: firmware loading, the Processor window, and the
  chips flipped to simulated — decided by the owner, 2026-11-24. (G, H)
- The firmware is chosen in the Processor window, which opens on a
  double-click of the chip; no flash viewer — decided by the owner,
  2026-11-24. (H)
- The C port goes directly into `backend/src/models/`, with avr8js as the
  oracle — decided by the owner, 2026-11-24.

## Corrections

- § 3.2: the three pairs of adjacent decode branches RET/RETI, SBC/SBCI and
  SUB/SUBI share one block of C each; the order of the tests and the 99
  branch ids are unchanged (`step: 99 of 99 decode branches ran`).
- § 3.2: a clock event's cycle is a `double`, as avr8js's is a JavaScript
  number (B's timers pass computed values). Nodes avr8js would leave to the
  garbage collector go to a spare list (`spare_events`) that new nodes are
  taken from, so memory does not grow.
- § 3.2: a jump below address 0 wraps to the top of the flash in C; avr8js
  keeps a negative pc there and never recovers. The step vectors keep the pc
  where this cannot happen, so the comparison is unaffected.
- Phase 3: `avr_core_test --print-golden` writes the golden line (it was not
  named in the plan); the unit cases also cover one event per tick and 20
  events through avr8js's pool of 10.

## Summary

The risk in A is arithmetic fidelity across 99 decode branches; the full
opcode sweep against the live oracle is what contains it. Nothing outside
`backend/tools/`, three new source files, one test and `docs/backend.md` is
touched.
