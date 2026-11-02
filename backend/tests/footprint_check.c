/*
 * footprint_check: every footprint in Parts/footprints.json, expanded to its
 * pads, must equal its line in tests/golden/footprints.txt (plan-03-A §4.6),
 * and every footprint must be used by at least one part.
 *
 *     footprint_check --print > tests/golden/footprints.txt   regenerates it
 *
 * A pad is `designator:x,y,wxh,shape[,d<drill>][,r<radius>][,c<chamfer>:<corners>]`,
 * corners joined by '+' in tl, tr, br, bl order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parts.h"
#include "read_file.h"

static void print_number(FILE *out, double value) {
    /* -0.000 and 0.000 are the same place. */
    if (value > -0.0005 && value < 0.0005) {
        value = 0;
    }
    fprintf(out, "%.3f", value);
}

static char *footprint_line(const PartFootprint *fp) {
    char *text = NULL;
    size_t length = 0;
    FILE *out = open_memstream(&text, &length);
    size_t i;

    if (out == NULL) {
        return NULL;
    }
    fprintf(out, "%s|pins=%u|body=", fp->id, fp->pins);
    print_number(out, fp->body_width);
    fputc('x', out);
    print_number(out, fp->body_height);
    if (fp->body_round) {
        fputs(" round", out);
    }
    fputs("|pads=", out);
    for (i = 0; i < fp->pad_count; i++) {
        const FootprintPad *pad = &fp->pads[i];

        fprintf(out, "%s%s:", i > 0 ? ";" : "", pad->designator != NULL ? pad->designator : "-");
        print_number(out, pad->x);
        fputc(',', out);
        print_number(out, pad->y);
        fputc(',', out);
        print_number(out, pad->width);
        fputc('x', out);
        print_number(out, pad->height);
        fputs(pad->shape == PAD_ROUND ? ",round" : ",rect", out);
        if (pad->drill > 0) {
            fputs(",d", out);
            print_number(out, pad->drill);
        }
        if (pad->radius > 0) {
            fputs(",r", out);
            print_number(out, pad->radius);
        }
        if (pad->chamfer > 0) {
            static const char *const names[] = {"tl", "tr", "br", "bl"};
            const char *join = ":";
            unsigned c;

            fputs(",c", out);
            print_number(out, pad->chamfer);
            for (c = 0; c < 4; c++) {
                if (pad->corners & (1u << c)) {
                    fprintf(out, "%s%s", join, names[c]);
                    join = "+";
                }
            }
        }
    }
    fclose(out);
    return text;
}

int main(int argc, char **argv) {
    PartLibrary *library = part_library_create();
    size_t count;
    size_t parts;
    size_t i, j;
    size_t failures = 0;
    char **lines;
    char *golden;
    char *cursor;

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    count = part_library_footprint_count(library);
    lines = calloc(count + 1, sizeof(*lines));
    for (i = 0; i < count; i++) {
        lines[i] = footprint_line(part_library_footprint_at(library, i));
    }
    if (argc == 2 && strcmp(argv[1], "--print") == 0) {
        for (i = 0; i < count; i++) {
            printf("%s\n", lines[i]);
        }
        return 0;
    }

    /* Read after --print, so the output can be redirected onto the golden file. */
    golden = read_file(GOLDEN_DIR "/footprints.txt");
    if (golden == NULL) {
        printf("FAIL: cannot read %s/footprints.txt\n", GOLDEN_DIR);
        return 1;
    }
    /* Golden lines, in order, against the library's footprints, in order. */
    cursor = golden;
    for (i = 0; i < count || *cursor != '\0'; i++) {
        const char *expected = NULL;

        if (*cursor != '\0') {
            char *end = strchr(cursor, '\n');

            expected = cursor;
            cursor = end != NULL ? end + 1 : cursor + strlen(cursor);
            if (end != NULL) {
                *end = '\0';
            }
        }
        if (expected == NULL) {
            if (failures++ < 10) {
                printf("extra footprint (not in the golden): %s\n", lines[i]);
            }
        } else if (i >= count) {
            if (failures++ < 10) {
                printf("missing footprint: %s\n", expected);
            }
        } else if (strcmp(lines[i], expected) != 0 && failures++ < 10) {
            printf("differs:\n  new: %s\n  old: %s\n", lines[i], expected);
        }
    }

    /* Every footprint is named by some part, as its footprint or connectorFootprint. */
    parts = part_library_count(library);
    for (i = 0; i < count; i++) {
        const PartFootprint *fp = part_library_footprint_at(library, i);

        for (j = 0; j < parts; j++) {
            const Part *part = part_library_at(library, j);

            if (part->footprint == fp || part->connector_footprint == fp) {
                break;
            }
        }
        if (j == parts) {
            printf("unused footprint: %s\n", fp->id);
            failures++;
        }
    }

    printf("%zu footprints compared, %zu parts loaded\n", count, parts);
    if (failures > 0) {
        printf("FAIL: %zu mismatches\n", failures);
        return 1;
    }
    for (i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
    free(golden);
    part_library_destroy(library);
    return 0;
}
