#ifndef CIRCUIT_H
#define CIRCUIT_H

/*
 * A circuit of library parts, simulated with the event engine.
 *
 *     Circuit *c = circuit_create(library);
 *     size_t vcc, led, net;
 *     circuit_add_part(c, "VCC", &vcc);
 *     circuit_add_part(c, "LED-RED-0603", &led);
 *     circuit_add_net(c, &net);
 *     circuit_connect(c, vcc, "1", net);    // pins are named by designator
 *     circuit_connect(c, led, "2", net);    // the LED's anode
 *     circuit_start(c);
 *     circuit_advance(c, 1000000);          // 1 us
 *
 * Indices (parts, nets) count from 0 in the order they were added. Times
 * are picoseconds. A call that fails returns -1 (or 0 where noted) and sets
 * circuit_error.
 */

#include <stddef.h>
#include <stdint.h>

#include "parts.h"

typedef struct Circuit Circuit;
typedef enum { CIRCUIT_0, CIRCUIT_1, CIRCUIT_X, CIRCUIT_Z } CircuitValue;

Circuit *circuit_create(const PartLibrary *library);   /* library must outlive it */
void circuit_destroy(Circuit *circuit);
const char *circuit_error(const Circuit *circuit);      /* NULL when none */

/* Building: 0 on success, -1 with circuit_error set. Before circuit_start. */
int circuit_add_part(Circuit *circuit, const char *part_name, size_t *index);
int circuit_add_net(Circuit *circuit, size_t *index);
/* pin is the pin's designator, e.g. "1" or "A1". */
int circuit_connect(Circuit *circuit, size_t part, const char *pin, size_t net);

/* Switches: position is 1-based; works before and after start. */
int circuit_set_switch(Circuit *circuit, size_t part, unsigned position, int on);

int circuit_start(Circuit *circuit);
int circuit_advance(Circuit *circuit, uint64_t picoseconds);  /* 0 once stopped */
uint64_t circuit_time(const Circuit *circuit);

CircuitValue circuit_net_value(const Circuit *circuit, size_t net);
/* Times the net's value has changed since circuit_start; 0 for a bad index. */
uint64_t circuit_net_changes(const Circuit *circuit, size_t net);
int circuit_shorted_net(const Circuit *circuit, size_t *net);  /* 1 = a net shorted */

/* LEDs (1 light) and 7-segment displays (PartDisplay.leds order): a light is
   on when it was on at any time during the last circuit_advance. */
size_t circuit_part_lights(const Circuit *circuit, size_t part, unsigned char *on, size_t cap);
/* Same lights and order: picoseconds each has been on since circuit_start.
   Returns the light count (0 when the part has none). */
size_t circuit_part_light_time(const Circuit *circuit, size_t part, uint64_t *on_ps, size_t cap);

/* Memory contents (SRAM, EEPROM). */
int circuit_memory_size(const Circuit *circuit, size_t part, uint32_t *words, unsigned *bits);
size_t circuit_memory_read(const Circuit *circuit, size_t part, uint32_t start, size_t count,
                           uint32_t *values, uint32_t *known);
/* Memory load (EEPROM): writes count words from start, before circuit_start.
   Each value must fit the word width. 0, or -1 with circuit_error set. */
int circuit_memory_load(Circuit *circuit, size_t part, uint32_t start,
                        const uint32_t *values, size_t count);

#endif
