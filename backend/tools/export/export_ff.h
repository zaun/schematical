#ifndef EXPORT_FF_H
#define EXPORT_FF_H

/*
 * Shared part of the flip-flop exporters (plan-01-D section 3.4). The
 * caller builds the kind keys (units, polarity, edge, ...) in order, the
 * timing object (family -> entry, with "@def:<id>/<FAMILY>" notes) and the
 * parts array; export_ff_device() writes a device, or merges an _FK
 * definition into its base device as an "lccc20" pinout when every kind key
 * is equal and the timings agree on every family both define.
 */

#include "export.h"

/* "p<pin count>", or "lccc20" for an _FK definition. */
const char *export_ff_pinout(const ChipDefinition *def, char *out, size_t cap);

/* Takes ownership of keys, timing and parts. */
int export_ff_device(const ChipDefinition *def, cJSON *keys, cJSON *timing, cJSON *parts);

int export_dff(void);
int export_jkff(void);

#endif
