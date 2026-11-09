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

int main(int argc, char **argv) {
    PartLibrary *library = part_library_create();
    GoldenPart *golden;
    size_t golden_count;
    NewLine *lines;
    size_t count;
    size_t i = 0;
    size_t g = 0;
    size_t compared = 0;
    size_t skipped = 0;
    size_t failures = 0;
    const char *kinds[PART_RTC + 1];   /* one per PartKind: the last kind sizes it */
    size_t kind_counts[PART_RTC + 1];
    size_t kind_n = 0;

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    count = part_library_count(library);
    /* Every shipped part names its manufacturer; all but Generic parts and
       the few whose maker no longer hosts one name a datasheet. */
    {
        size_t no_maker = 0;
        size_t no_sheet = 0;

        for (i = 0; i < count; i++) {
            const Part *part = part_library_at(library, i);

            if (part->manufacturer == NULL) {
                if (no_maker++ < 10) {
                    printf("FAIL: %s has no \"manufacturer\"\n", part->name);
                }
            } else if (part->datasheet == NULL && strcmp(part->manufacturer, "Generic") != 0) {
                no_sheet++;
            }
        }
        if (no_maker > 0) {
            printf("FAIL: %zu parts have no \"manufacturer\"\n", no_maker);
            return 1;
        }
        /* Every Markdown datasheet a datasheets.json entry names is in the
           bundled Datasheets folder, beside Parts. */
        for (i = 0; i < part_library_datasheet_count(library); i++) {
            const PartDatasheet *sheet = part_library_datasheet_at(library, i);
            char path[1024];
            FILE *file;

            if (sheet->md == NULL) {
                continue;
            }
            snprintf(path, sizeof path, "%s/../Datasheets/%s", PARTS_DIR, sheet->md);
            file = fopen(path, "rb");
            if (file == NULL) {
                printf("FAIL: datasheet %s: \"md\" file %s not found\n", sheet->id, sheet->md);
                return 1;
            }
            fclose(file);
        }
        if (argc != 2) {
            printf("   %zu parts have a manufacturer but no hosted datasheet\n", no_sheet);
        }
    }
    lines = calloc(count + 1, sizeof(*lines));
    for (i = 0; i < count; i++) {
        const Part *part = part_library_at(library, i);

        lines[i].name = part->name;
        lines[i].kind = listing_kind_name(part->kind);
        lines[i].line = listing_line(part);
    }
    qsort(lines, count, sizeof(*lines), by_name);
    /* --print: the current listing, to regenerate tests/golden/parts.txt. */
    if (argc == 2 && strcmp(argv[1], "--print") == 0) {
        for (i = 0; i < count; i++) {
            printf("%s\n", lines[i].line);
        }
        return 0;
    }

    /* Read after --print, so the output can be redirected onto the golden file. */
    golden_count = golden_read(GOLDEN_DIR, &golden);
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
    /* Every kind is registered (plan-01-E), so a skipped golden line is a lost part. */
    if (skipped > 0) {
        printf("FAIL: %zu golden lines skipped (kind not registered)\n", skipped);
        failures++;
    }
    if (failures > 0) {
        printf("FAIL: %zu mismatches\n", failures);
        return 1;
    }
    part_library_destroy(library);
    return 0;
}
