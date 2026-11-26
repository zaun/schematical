#ifndef SERIAL_BENCH_N68_H
#define SERIAL_BENCH_N68_H

/*
 * plan-68-N: edited variants of the ns16550 fixture devices
 * (tests/fixtures/serial/ns16550.json, then bridges.json) for loader rejections and for benches
 * on an edited device.
 *
 * `edits` is a JSON object: each key is a dotted path into the device object
 * ("bus.address.2", "channels.0.tx"; array steps are indexes) and its value
 * replaces what is there (or adds the key; an index one past the end
 * appends); the string "-" deletes it. The variant gets a fresh id and part
 * names ("<part>-N68-<n>").
 */

#include "serial_bench.h"

/* Loads a variant of device `base_id`; 0 on success, else -1 with the loader's
   message in *error (free it). `name_out` (may be NULL) gets the first part's
   new name (static buffer, valid until the next call). */
int n68_variant(const char *base_id, const char *edits, const char **name_out, char **error);
/* Expects the variant to be refused with a message containing `want`. */
void n68_reject(const char *base_id, const char *edits, const char *want);
/* Loads the variant (it must load) and returns its first part's name. */
const char *n68_part(const char *base_id, const char *edits);

/* The frames channel `ch` has sent (the serial_sent log), their count, and
   whether `want` is among them from index `from` on. */
size_t n68_sent(Bench *b, unsigned ch, uint16_t *frames, size_t cap);
size_t n68_sent_count(Bench *b, unsigned ch);
int n68_sent_has(Bench *b, unsigned ch, size_t from, unsigned want);
/* A bus write whose data bits in `xmask` are X. */
void n68_write_x(Bench *b, unsigned a, unsigned v, unsigned xmask);
/* Whether a pin toggles (seen 0 and 1) over `n` steps of `step`. */
int n68_toggles(Bench *b, const char *pin, unsigned n, SimTime step);
/* The length of the next low pulse on a pin (0 when none within `limit`). */
SimTime n68_low_len(Bench *b, const char *pin, SimTime limit);

#endif
