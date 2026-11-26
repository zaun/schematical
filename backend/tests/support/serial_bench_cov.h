#ifndef SERIAL_BENCH_COV_H
#define SERIAL_BENCH_COV_H

/*
 * plan-68-O: variants of the shipped bridge devices (tests/fixtures/serial/
 * bridges.json) for loader rejections and for benches on an edited device.
 *
 * `edits` is a JSON object: each key is a path into the device object
 * ("bus.i2c.table", "gpio.pins.0", dots between keys and array indexes) and
 * its value replaces what is there (or adds the key); the string "-" deletes
 * it. The variant gets a fresh id and part names ("<part>-COV<n>").
 */

#include "serial_bench.h"
#include "serial_bench_bridge.h"

/* Loads a variant of device `base_id`; 0 on success, else -1 with the loader's
   message in *error (free it). `name_out` (may be NULL) gets the first part's
   new name (static buffer, valid until the next call). */
int cov_variant(const char *base_id, const char *edits, const char **name_out, char **error);
/* Expects the variant to be refused with a message containing `want`. */
void cov_reject(const char *base_id, const char *edits, const char *want);
/* Loads the variant (it must load) and returns its first part's name. */
const char *cov_part(const char *base_id, const char *edits);

/* X-capable host helpers ('0', '1', 'X' bit characters). */
LogicValue lv(char c);
/* One SPI frame (mode 0); spaces in `bits` are skipped; `miso` gets SO per bit. */
void spx(MB *b, const char *bits, char *miso);
/* A bridge register write over SPI whose value may have X bits. */
void wx(Br *r, unsigned ch, unsigned reg, const char *value);
/* An I2C byte of 8 bit characters; returns the ACK slot (0 = ACK). */
LogicValue i2c_bits(MB *b, const char *bits);

#endif
