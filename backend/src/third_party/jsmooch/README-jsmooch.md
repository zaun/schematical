# jsmooch WDC 65816 core (vendored)

Source: `howprice/jsmooch-emus` at commit
`57366f22500671b5e3c208d4283d33c66299acee` (2025-07-21). The original
`raddad772/jsmooch-emus` is gone (`gh api repos/raddad772/jsmooch-emus` →
404, 2026-11-01); this copy carries the same licence and is not a GitHub
fork. Licence: MIT, "Copyright (c) 2024 raddad772" (`LICENSE-jsmooch`).
This file is `README-jsmooch.md` and the licence is not `LICENSE`: the app
copies `backend/src` files flat into its resources, where `README.md`
(floooh/chips) and `LICENSE` (cJSON) are taken. plan-14-G.

Files from upstream `jsmooch-lib/src/component/cpu/wdc65816/`:

- `wdc65816.c`, `wdc65816.h` — the cycle-stepped core (one bus cycle per
  `WDC65816_cycle` call; the pins struct carries address, bank, data, RWB).
- `wdc65816_generated_opcodes.c` — every opcode's cycles, in five tables
  (native `mx Mx mX MX`, emulation `E`). Generated: do not edit by hand.
  `node backend/tools/jsmooch-gen/run.js` regenerates it from the generator
  in `backend/tools/jsmooch-gen/` (upstream
  `jsmooch-lib/code_gen/wdc65816_and_spc700/`: `generation.js`,
  `wdc65816_opcodes.js`, `wdc65816_core_generator.js`, MIT, same
  copyright); `cpu_core_test` checks that it reproduces this file.
- `wdc65816_disassembler.c`, `wdc65816_disassembler.h` — the disassembler.

Ours (MIT, this project), not upstream:

- `helpers/int.h`, `helpers/jsm_string.h`, `helpers/debug.h`,
  `helpers/debugger/debugger.h`, `helpers/debugger/debuggerdefs.h` — stubs
  for the upstream helper headers the five files include: the integer types,
  a fixed-buffer string, the read-trace hook, and empty debug/trace macros.
  Only what the core needs to compile.
- `backend/tools/jsmooch-gen/run.js` — the Node driver that runs the
  generator.

The three C files compile directly (`backend/CMakeLists.txt`, and the app
through the synchronized `backend/src` folder); their `#include
"helpers/…"` lines resolve next to the including file.

`backend/tests/cpu_core_test.c` runs the core against
`backend/tests/cpu/sst-65816.jsonl` (the first 10 tests of each
SingleStepTests/65816 file at `307c63f5673c8c6b861c52f71ddb89212a227480`,
MIT, `LICENSE-SingleStepTests-65816`).

## Changes from upstream

The first commit holds the unmodified upstream files (plan-14-G Phase 1), so
the diff of every change is visible. Each fix below was found by
`cpu_core_test` against SingleStepTests and checked there (5120 tests, both
modes, every opcode, every cycle's address, data and flags).

In the generator (`backend/tools/jsmooch-gen/wdc65816_core_generator.js`);
`wdc65816_generated_opcodes.c` is regenerated from it, never edited:

1. `WDC_PINS_SEPERATE_PDV = true`: VDA, VPA and VPB are separate pins
   (upstream merged them into `PDV`).
2. Absolute read-modify-write (`Ad`): the data read is VDA (was VPA). JMP
   `(a,x)` and JSR `(a,x)`: the pointer reads are VDA (were VPA).
3. MLB: a new `ML_on()` sets `pins.ML` from a read-modify-write's first data
   read to its write, in the four RMW modes (a, a,x, d, d,x); the
   instruction's end clears it.
4. `decode_opcodes_c`: the emulation table is built with M = X = 1 (it was
   built with M = X = 0, so emulation mode pushed and pulled 16 bits,
   block moves used 16-bit X/Y, and so on); `decode_opcodes()` already did
   this.
5. Direct page indexed (d,x; d,y; d,x RMW) in emulation mode: the index
   wraps within D's page only when DL = 0; with DL ≠ 0 the address is
   `(D + d + index) & FFFF` (`WDC_dp_indexed`, w65c816s.pdf §7.2). Upstream
   wrapped `(d + index + D) & FF` into page 0.
6. REP in emulation mode keeps M and X set (`REPE` cleared them).
7. TXS in emulation mode keeps S in page 1 (`S = 01xx`).
8. Emulation-mode interrupts push P with bit 5 set; BRK and COP with B
   (bit 4) set, IRQ, NMI and ABORT with B clear. D is pushed as it is
   (upstream: BRK `P | 0x38`, others `P & 0xF7`).
9. Relative branches: not taken, the instruction ends after the operand;
   taken, one idle cycle, and a second only in emulation mode when the
   target is on another page (upstream always took the second one in
   emulation mode).
10. WAI: two idle cycles at PBR,PC (VDA = VPA = 0), then the processor waits
    with no bus cycles until an interrupt clears `regs.WAI`; an interrupt
    already pending does not wait (upstream re-presented the opcode fetch).
11. ABORT sets I and clears D, as IRQ and NMI.

By hand (not generated):

12. `wdc65816.c`: removed the "CRAP!" `dbg_break` hack in the opcode fetch
    and the dbglog trace code (`pprint_context`, `wdc_trace_format`,
    `irqdump`).
13. `wdc65816.h`, `wdc65816.c`: `pins.M` and `pins.X` (with `pins.E`) show
    P.M, P.X and E, updated when an instruction ends (`TCU = 0`): an
    instruction's cycles show the values it began with, as SingleStepTests
    records them. The adapter multiplexes the MX pin (M at PHI2 fall, X at
    PHI2 rise; plan-14-H).
14. `wdc65816.h`, `wdc65816.c`: `regs.ABORT_pending`, serviced at the next
    instruction boundary before NMI and IRQ by the generator's S_ABORT
    sequence (opcode 0x101): PC and PBR are set back to the aborted
    instruction's address (`trace.ins_PC`) so it is the one pushed.
    Restoring the other registers the aborted instruction changed is the
    adapter's job (w65c816s.pdf §2.13).
15. `wdc65816_disassembler.c`: the 16-bit immediates were formatted with
    `snprintf(buf, 5, "#$%04x", n)`, which cut `#$1234` to `#$12`; the size
    is 50, as every other formatter.

Ours, alongside: the `helpers/` stubs and `run.js` (above).
