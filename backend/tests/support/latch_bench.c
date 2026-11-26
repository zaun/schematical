/* latch_bench: shared by the programs split from latch_test.c (plan-68-H). */
#include "latch_bench.h"


int failures = 0;
PartLibrary *library;

void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* Generated from the spec sections (plan-10-data/latch-a.md, latch-b.md). */
/* The fixture parts: tests/fixtures/latch/parts.json (moved out of C by plan-68-H). */
char *FIXTURE;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
/* Their footprints: tests/fixtures/latch/footprints.json. */
char *FIXTURE_FOOTPRINTS;

char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

/* Loads text (single-quote style); returns the error (caller frees) or NULL. */
char *load(PartLibrary *lib, const char *text, const char *source) {
    char *json = dequote(text);
    char *error = NULL;

    if (load_parts(lib, json, source, &error) == 0) {
        free(error);
        error = NULL;
    } else if (error == NULL) {
        error = strdup("(no message)");
    }
    free(json);
    return error;
}

PartLibrary *fixture_library(void) {
    PartLibrary *lib = part_library_create();
    char *json = dequote(FIXTURE_FOOTPRINTS);
    char *error = NULL;

    if (lib == NULL || load_footprints(lib, json, "fx-footprints.json", &error) != 0) {
        fprintf(stderr, "fixture footprints: %s\n", error != NULL ? error : "(no message)");
        exit(1);
    }
    free(json);
    free(error);
    return lib;
}

/* ---- Bench helpers -------------------------------------------------------- */

const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LX = LOGIC_X, LZ = LOGIC_Z;

Bench *open_fx(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = part_library_find(library, name);
    if (part == NULL || part->kind != PART_LATCH) {
        fprintf(stderr, "no latch part %s\n", name);
        exit(1);
    }
    return bench_create(part);
}

void step(Bench *b) {
    bench_run(b, NS(50));
}

void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Sets pins fmt % (i + base) for i < n from value's bits. */
void set_bus(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i + base);
        set(b, name, bit(value, i));
    }
}

/* 1 when every pin fmt % (i + base) shows want(i) (values[i]). */
int bus_is(Bench *b, const char *fmt, unsigned base, unsigned n, const LogicValue *want) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got;

        snprintf(name, sizeof name, fmt, i + base);
        got = bench_get(b, name);
        if (got != want[i]) {
            printf("  %s = %c, want %c\n", name, "01XZ"[got], "01XZ"[want[i]]);
            ok = 0;
        }
    }
    return ok;
}

int bus_value(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = bit(value, i);
    }
    return bus_is(b, fmt, base, n, want);
}

int bus_all(Bench *b, const char *fmt, unsigned base, unsigned n, LogicValue v) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = v;
    }
    return bus_is(b, fmt, base, n, want);
}

int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}


/* ---- Real parts (plan-10-C section 4.3) ----------------------------------- */

PartLibrary *real_library;
