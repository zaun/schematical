# floooh/chips (vendored)

Upstream: https://github.com/floooh/chips at commit
`9e88298ce56319953ac7a43213a1120359f7a3a6` (2026-09-26). Licence: zlib
(`LICENSE-chips`, Copyright (c) 2018 Andre Weissflog).

Unmodified copies; see plan-09-A. The SHA-256 of each file is listed in
plan-09-A §4.2, and `backend/tests/cpu_core_test.c` checks the cores against
SingleStepTests. The wrappers `backend/src/models/chips_*.c` compile each one
once.

- `m6502.h` — cycle-stepped NMOS 6502 core (`chips/m6502.h`).
- `w65c02.h` — cycle-stepped WDC 65C02 core (`chips/w65c02.h`).
- `z80.h` — cycle-stepped Z80 core (`chips/z80.h`).
- `m6502dasm.h` — 6502 disassembler (`util/m6502dasm.h`).
- `w65c02dasm.h` — 65C02 disassembler (`util/w65c02dasm.h`).
- `z80dasm.h` — Z80 disassembler (`util/z80dasm.h`).
- `LICENSE-chips` — upstream `LICENSE`, the zlib licence, unchanged. Not named
  `LICENSE`: the app copies `backend/src` files into its resources, where
  that name is taken by cJSON's.
