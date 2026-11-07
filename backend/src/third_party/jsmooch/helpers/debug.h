/*
 * Stub for jsmooch's helpers/debug.h (plan-14-G): the read-trace hook the
 * 65816 disassembler reads memory through, and an empty dbg_break. Ours,
 * not jsmooch's (README.md).
 */
#ifndef JSMOOCH_STUB_DEBUG_H
#define JSMOOCH_STUB_DEBUG_H

#include "int.h"

struct jsm_debug_read_trace {
    void *ptr;
    u32 (*read_trace)(void *ptr, u32 addr);
    u32 (*read_trace_m68k)(void *ptr, u32 addr, u32 uds, u32 lds);
};

static inline void dbg_break(const char *why, u64 clock) {
    (void)why;
    (void)clock;
}

#endif
