/*
 * erc_pins_check: the ERC pin type of every pin of every part in the JSON
 * part files, one line per part (plan-63-B), must equal the golden
 * tests/golden/erc_pins.txt:
 *
 *   <part name>|<kind>|<one letter per pin, in pin order>
 *
 * Letters, in ErcPinType order: i input, o output, b bidirectional, t tri_state,
 * p passive, f free, u unspecified, P power_in, O power_out, c open_collector,
 * e open_emitter, n no_connect; '-' for an absent pin.
 *
 * erc_pins_check --print writes the current lines, to regenerate the golden.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "erc.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"

static const char LETTERS[ERC_PIN_TYPE_COUNT + 1] = "iobtpfuPOcen";

typedef struct {
    const char *name;
    char *line;
} Line;

static int by_name(const void *a, const void *b) {
    return strcmp(((const Line *)a)->name, ((const Line *)b)->name);
}

static char *line_of(const Part *part) {
    size_t length = strlen(part->name) + 1 + 32 + 1 + part->pin_count + 1;
    char *line = malloc(length);
    size_t used, i;

    if (line == NULL) {
        return NULL;
    }
    used = (size_t)snprintf(line, length, "%s|%s|", part->name, listing_kind_name(part->kind));
    for (i = 0; i < part->pin_count; i++) {
        ErcPinType type;

        line[used++] = erc_pin_type(part, i, &type) ? LETTERS[type] : '-';
    }
    line[used] = '\0';
    return line;
}

static int check(PartLibrary *library, int argc, char **argv);

/* The library outlives every line (they point at part names), so it goes last. */
int main(int argc, char **argv) {
    PartLibrary *library = part_library_create();
    int result = check(library, argc, argv);

    part_library_destroy(library);
    return result;
}

static int check(PartLibrary *library, int argc, char **argv) {
    size_t count, i, differences = 0, golden_lines = 0;
    Line *lines;
    char path[1024];
    FILE *file;
    char *text;
    long size;
    const char *cursor;

    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    count = part_library_count(library);
    lines = calloc(count + 1, sizeof(*lines));
    for (i = 0; i < count; i++) {
        const Part *part = part_library_at(library, i);

        lines[i].name = part->name;
        lines[i].line = line_of(part);
    }
    qsort(lines, count, sizeof(*lines), by_name);
    if (argc == 2 && strcmp(argv[1], "--print") == 0) {
        for (i = 0; i < count; i++) {
            printf("%s\n", lines[i].line);
        }
        return 0;
    }

    snprintf(path, sizeof path, "%s/erc_pins.txt", GOLDEN_DIR);
    file = fopen(path, "rb");
    if (file == NULL) {
        printf("FAIL: cannot open %s\n", path);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    text = malloc((size_t)size + 1);
    if (text == NULL || fread(text, 1, (size_t)size, file) != (size_t)size) {
        printf("FAIL: cannot read %s\n", path);
        return 1;
    }
    fclose(file);
    text[size] = '\0';

    cursor = text;
    for (i = 0; i < count || *cursor != '\0'; i++) {
        const char *end = strchr(cursor, '\n');
        size_t length = end != NULL ? (size_t)(end - cursor) : strlen(cursor);
        int same;

        if (*cursor != '\0') {
            golden_lines++;
        }
        same = i < count && *cursor != '\0' && strlen(lines[i].line) == length &&
               strncmp(lines[i].line, cursor, length) == 0;
        if (!same) {
            if (differences++ < 20) {
                printf("line %zu differs:\n  now:    %s\n  golden: %.*s\n", i + 1,
                       i < count ? lines[i].line : "(none)", (int)length, cursor);
            }
        }
        if (*cursor == '\0') {
            continue;
        }
        cursor += length;
        if (*cursor == '\n') {
            cursor++;
        }
    }
    free(text);
    if (differences > 0) {
        printf("FAIL: %zu lines differ (%zu parts, %zu golden lines)\n", differences, count, golden_lines);
        return 1;
    }
    printf("erc_pins_check: %zu parts match the golden\n", count);
    return 0;
}
