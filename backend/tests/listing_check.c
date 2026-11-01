/*
 * listing_check: every part in the JSON part files, rendered as a listing
 * line, must equal the golden line from the old backend
 * (tests/golden/parts.txt), for every kind registered in KINDS[].
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "golden.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"

typedef struct {
    const char *name;
    const char *kind;
    char *line;
} NewLine;

static int by_name(const void *a, const void *b) {
    return strcmp(((const NewLine *)a)->name, ((const NewLine *)b)->name);
}

static void count_kind(const char **kinds, size_t *counts, size_t *n, const char *kind) {
    size_t i;

    for (i = 0; i < *n; i++) {
        if (strcmp(kinds[i], kind) == 0) {
            counts[i]++;
            return;
        }
    }
    kinds[*n] = kind;
    counts[(*n)++] = 1;
}

int main(void) {
    PartLibrary *library = part_library_create();
    GoldenPart *golden;
    size_t golden_count = golden_read(GOLDEN_DIR, &golden);
    NewLine *lines;
    size_t count;
    size_t i = 0;
    size_t g = 0;
    size_t compared = 0;
    size_t skipped = 0;
    size_t failures = 0;
    const char *kinds[16];
    size_t kind_counts[16];
    size_t kind_n = 0;

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    count = part_library_count(library);
    lines = calloc(count + 1, sizeof(*lines));
    for (i = 0; i < count; i++) {
        const Part *part = part_library_at(library, i);

        lines[i].name = part->name;
        lines[i].kind = listing_kind_name(part->kind);
        lines[i].line = listing_line(part);
    }
    qsort(lines, count, sizeof(*lines), by_name);

    i = 0;
    while (i < count || g < golden_count) {
        int order;

        if (g < golden_count && !golden_kind_registered(golden[g].kind)) {
            skipped++;
            g++;
            continue;
        }
        order = i >= count ? 1 : g >= golden_count ? -1 : strcmp(lines[i].name, golden[g].name);
        if (order < 0) {
            if (failures++ < 10) {
                printf("extra part (not in the golden): %s\n", lines[i].line);
            }
            i++;
        } else if (order > 0) {
            if (failures++ < 10) {
                printf("missing part: %s\n", golden[g].line);
            }
            g++;
        } else {
            if (strcmp(lines[i].line, golden[g].line) != 0 && failures++ < 10) {
                printf("differs:\n  new: %s\n  old: %s\n", lines[i].line, golden[g].line);
            }
            count_kind(kinds, kind_counts, &kind_n, lines[i].kind);
            compared++;
            i++;
            g++;
        }
    }

    for (i = 0; i < kind_n; i++) {
        printf("  %s %zu\n", kinds[i], kind_counts[i]);
    }
    printf("%zu parts compared, %zu skipped\n", compared, skipped);
    if (failures > 0) {
        printf("FAIL: %zu mismatches\n", failures);
        return 1;
    }
    part_library_destroy(library);
    return 0;
}
