#ifndef CATALOG_VALIDATE_H
#define CATALOG_VALIDATE_H

#include <stddef.h>

/*
 * Validation report for the catalog. Every item gets an entry, whether or not
 * it has errors, so a caller can show "checked and fine" as well as failures.
 *
 * Items are:
 *   - each chip definition, by id (e.g. "74_00"): its pin table,
 *   - each chip part number, as "<id>/<part>" (e.g. "74_00/SN74HC00N"),
 *   - each LED part (e.g. "LED-RED-0603"),
 *   - each supply (e.g. "VCC").
 */

typedef struct {
    char *item;        /* owned: item name as above */
    char **errors;     /* owned: one message per error, empty when OK */
    size_t error_count;
} CatalogItemErrors;

typedef struct {
    CatalogItemErrors *items; /* owned */
    size_t item_count;
} CatalogErrorReport;

/* Checks every catalog item. Free the result with catalog_report_free(). */
CatalogErrorReport catalog_validate(void);

/* Total number of error messages across all items. */
size_t catalog_report_error_total(const CatalogErrorReport *report);

/* Frees a report returned by catalog_validate(). Safe on a zeroed report. */
void catalog_report_free(CatalogErrorReport *report);

#endif
