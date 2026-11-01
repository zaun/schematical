#include "golden.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "read_file.h"

static char *copy_span(const char *start, size_t length) {
    char *copy = malloc(length + 1);

    memcpy(copy, start, length);
    copy[length] = '\0';
    return copy;
}

size_t golden_read(const char *dir, GoldenPart **out) {
    char path[4096];
    char *listing;
    char *traces;
    char *line;
    GoldenPart *parts = NULL;
    size_t count = 0;
    size_t capacity = 0;
    size_t i = 0;

    snprintf(path, sizeof path, "%s/parts.txt", dir);
    listing = read_file(path);
    snprintf(path, sizeof path, "%s/traces.txt", dir);
    traces = read_file(path);
    if (listing == NULL || traces == NULL) {
        fprintf(stderr, "cannot read the goldens in %s\n", dir);
        exit(2);
    }

    for (line = listing; *line != '\0';) {
        char *end = strchr(line, '\n');
        char *bar1 = strchr(line, '|');
        char *bar2 = bar1 != NULL ? strchr(bar1 + 1, '|') : NULL;

        if (end == NULL || bar2 == NULL || bar2 > end) {
            fprintf(stderr, "bad parts.txt line\n");
            exit(2);
        }
        if (count == capacity) {
            capacity = capacity == 0 ? 4096 : capacity * 2;
            parts = realloc(parts, capacity * sizeof(*parts));
        }
        parts[count].name = copy_span(line, (size_t)(bar1 - line));
        parts[count].kind = copy_span(bar1 + 1, (size_t)(bar2 - bar1 - 1));
        parts[count].line = copy_span(line, (size_t)(end - line));
        parts[count].hash = NULL;
        count++;
        line = end + 1;
    }

    for (line = traces; *line != '\0'; i++) {
        char *end = strchr(line, '\n');
        char *tab = strchr(line, '\t');

        if (end == NULL || tab == NULL || tab > end || i >= count ||
            strncmp(parts[i].name, line, (size_t)(tab - line)) != 0 ||
            parts[i].name[tab - line] != '\0') {
            fprintf(stderr, "traces.txt does not line up with parts.txt at line %zu\n", i + 1);
            exit(2);
        }
        parts[i].hash = copy_span(tab + 1, (size_t)(end - tab - 1));
        line = end + 1;
    }
    if (i != count) {
        fprintf(stderr, "traces.txt has %zu lines, parts.txt %zu\n", i, count);
        exit(2);
    }
    free(listing);
    free(traces);
    *out = parts;
    return count;
}

int golden_kind_registered(const char *kind) {
    return kind_find(kind) != NULL;
}

size_t golden_registered_count(void) {
    return KIND_COUNT;
}
