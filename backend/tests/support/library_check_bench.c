/* library_check_bench: shared by the programs split from library_check.c (plan-68-H). */
#include "library_check_bench.h"


int failures;

/* Single quotes stand for double quotes, to keep the cases readable. */
char *json(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

/* Gives a library with no footprints the fixture set. */
void load_fixture_footprints(PartLibrary *library) {
    char *source;
    char *error = NULL;

    if (part_library_footprint_count(library) > 0) {
        return;
    }
    source = json(FIXTURE_FOOTPRINTS);
    if (load_footprints(library, source, "fixtures.json", &error) != 0) {
        printf("FAIL fixture footprints: %s\n", error != NULL ? error : "(none)");
        failures++;
    }
    free(error);
    free(source);
}

/* Loads text into a fresh library and expects failure with `expect` in the message. */
void expect_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result;

    load_fixture_footprints(library);
    result = load_parts(library, source, "test.json", &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    free(source);
    part_library_destroy(library);
}

int load(PartLibrary *library, const char *text, char **error) {
    char *source = json(text);
    int result;

    load_fixture_footprints(library);
    result = load_parts(library, source, "test.json", error);

    free(source);
    return result;
}
