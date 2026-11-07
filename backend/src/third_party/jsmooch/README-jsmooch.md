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

None yet: the first commit is the unmodified upstream files (plan-14-G
Phase 1), so the diff of every later change is visible.
