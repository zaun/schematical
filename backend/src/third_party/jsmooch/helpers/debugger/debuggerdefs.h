/*
 * Stub for jsmooch's helpers/debugger/debuggerdefs.h (plan-14-G): the debug
 * struct macros, reduced to the event ids the 65816 core stores. Ours, not
 * jsmooch's (README.md).
 */
#ifndef JSMOOCH_STUB_DEBUGGERDEFS_H
#define JSMOOCH_STUB_DEBUGGERDEFS_H

#include "../int.h"

#define DBG_START struct {
#define DBG_EVENT_VIEW_START struct { void *view; i32
#define DBG_EVENT_VIEW_END ; } events;
#define DBG_TRACE_VIEW void *trace_view;
#define DBG_END } dbg;

#define DBG_EVENT_VIEW_INIT do { } while (0)
#define DBG_EVENT(id) do { (void)(id); } while (0)

#endif
