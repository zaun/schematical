/*
 * Stub for jsmooch's helpers/debugger/debugger.h (plan-14-G): the trace-log
 * view the 65816 core writes to when one is attached (never, here). Ours,
 * not jsmooch's (README.md).
 */
#ifndef JSMOOCH_STUB_DEBUGGER_H
#define JSMOOCH_STUB_DEBUGGER_H

#include "../int.h"
#include "../jsm_string.h"   /* wdc65816.h uses struct jsm_string through this header */

enum { DBGLS_TRACE = 0 };

struct dbglog_view {
    int ids_enabled[64];
};

static inline void dbglog_view_add_printf(struct dbglog_view *view, u32 id, u64 clock, int severity,
                                          const char *format, ...) {
    (void)view;
    (void)id;
    (void)clock;
    (void)severity;
    (void)format;
}

static inline void dbglog_view_extra_printf(struct dbglog_view *view, const char *format, ...) {
    (void)view;
    (void)format;
}

#endif
