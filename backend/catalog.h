#ifndef CATALOG_H
#define CATALOG_H

#include <stdio.h>
#include <stddef.h>

#include "chip_definition.h"

/* Every chip definition the backend knows about, in a stable order. */
size_t catalog_definition_count(void);
const ChipDefinition *catalog_definition_at(size_t index);

/*
 * Writes the catalog as JSON:
 *   {"chips":[{"id":..,"function":..,"description":..,
 *              "parts":[{"name":..,"package":..,"family":..,"obsolete":bool,
 *                        (for SRAMs, "family" is the speed grade, e.g. "55 ns")
 *                        "military":bool,"automotive":bool,"enhanced":bool}],
 *              "pins":[{"number":..,"name":..,"role":..,"group":..}]}],
 *    "leds":[{"name":..,"color":..,"package":..,
 *             "pins":[{"number":..,"name":..,"role":..,"group":..}]}],
 *    "supplies":[{"name":..,"pins":[{"number":..,"name":..,"role":..,"group":..}]}]}
 * Only part numbers the backend can actually simulate are written.
 */
void catalog_write_json(FILE *out);

/*
 * Returns the catalog JSON as a newly allocated, NUL-terminated string, or
 * NULL on allocation failure. The caller frees it with free().
 */
char *catalog_json_copy(void);

#endif
