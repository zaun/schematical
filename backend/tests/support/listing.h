#ifndef LISTING_H
#define LISTING_H

#include "parts.h"

/* The part's kind as the listing names it ("gate", "7seg", ...). */
const char *listing_kind_name(PartKind kind);

/*
 * One part as a plan-01-A section 4.2 listing line, without the newline:
 *   name|kind|package|drawer|function|family|flags|details|pins
 * malloc'd; the caller frees it.
 */
char *listing_line(const Part *part);

#endif
