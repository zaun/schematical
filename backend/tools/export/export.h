#ifndef EXPORT_H
#define EXPORT_H

/*
 * export_parts: old C tables -> part JSON files (plan-01-B section 3.6).
 * Temporary; deleted with the old backend in plan-01-G.
 *
 * Each export_<kind>.c #includes one old model .c file and walks its static
 * tables. Devices are built as cJSON objects with keys in the order of
 * plan-01-A section 3.2 and handed to export_device(); export_write()
 * writes the file with one pin and one part per line.
 */

#include <stdio.h>

#include "cJSON.h"
#include "chip_definition.h"

/* Starts a file of the given kind; devices go into a fresh array. */
void export_begin(const char *kind);
void export_device(cJSON *device);              /* takes ownership */
cJSON *export_find_device(const char *id);      /* NULL when not added yet */
int export_write(const char *path);

/* Helpers for building devices. */
cJSON *export_new_device(const char *id, const char *function, const char *description,
                         const char *drawer0, const char *drawer1, const char *note);
/* A pinout array from an old pin table (hidden group -> role "absent"). */
cJSON *export_pins(const ChipPin *pins, size_t count);
void export_add_pinout(cJSON *device, const char *name, cJSON *pins);
/* Adds the "parts" array (created on first use) and returns it. */
cJSON *export_parts_array(cJSON *device);
cJSON *export_new_part(const char *name, const char *package, const char *pinout);
void export_part_flags(cJSON *part, unsigned flags);
/* Nanoseconds from picoseconds, as a JSON number. */
cJSON *export_ns(unsigned long long ps);
/* Two pin tables have the same pins (number, name, role, group). */
int export_same_pins(const ChipPin *a, size_t a_count, const ChipPin *b, size_t b_count);
/* Two JSON values are equal (cJSON_Compare, case-sensitive). */
int export_same_json(const cJSON *a, const cJSON *b);

/* Old enum names, shared by the exporters. */
const char *export_package_name(int package);   /* PackageType */
const char *export_family_name(int family);     /* LogicFamily, real families only */

int export_supply(void);
int export_led(void);
int export_osc(void);
int export_switch(void);
int export_sram(void);
int export_7seg(void);

#endif
