/*
 * trace_check: every part in the JSON part files, simulated with the new
 * models under the plan-01-A trace (tests/support/trace_core.c), must hash
 * to the golden value from the old backend (tests/golden/traces.txt), for
 * every kind registered in KINDS[].
 *   trace_check                   all parts
 *   trace_check --verbose <part>  that part's change log
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "golden.h"
#include "kind.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"
#include "trace_core.h"

/* The last PartKind (plan-10 moves it as kinds are added). */
#define TRACE_LAST_KIND PART_DECODER

static int set_switch(Sim *sim, SimInstance *instance, size_t position, int on) {
    const SimComponentType *type = sim_instance_type(instance);

    return type->set_switch(sim, instance, sim_instance_state(instance), position, on);
}

static size_t lights(const Sim *sim, const SimInstance *instance, SimTime since,
                     char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(instance);
    unsigned char values[256];
    size_t count = type->lights(sim, instance, sim_instance_state(instance), since, values,
                                cap < sizeof values ? cap : sizeof values);
    size_t i;

    for (i = 0; i < count && i < cap && i < sizeof values; i++) {
        out[i] = values[i] ? '1' : '0';
    }
    return count;
}

static TraceRole trace_role(PinRole role) {
    switch (role) {
        case PIN_INPUT: return TRACE_INPUT;
        case PIN_INOUT: return TRACE_INOUT;
        case PIN_POWER: return TRACE_POWER;
        case PIN_GROUND: return TRACE_GROUND;
        case PIN_OUTPUT:
        case PIN_NC:
        case PIN_ABSENT: return TRACE_NONE;
    }
    return TRACE_NONE;
}

static uint64_t trace_part(const Part *part, FILE *verbose) {
    TraceRole roles[256];
    TraceSubject subject = {0};
    size_t i;

    if (part->pin_count > 256) {
        fprintf(stderr, "%s: too many pins\n", part->name);
        exit(2);
    }
    for (i = 0; i < part->pin_count; i++) {
        roles[i] = trace_role(part->pins[i].role);
    }
    subject.name = part->name;
    subject.type = kind_model(part);
    subject.config = part;
    subject.roles = roles;
    subject.pin_count = part->pin_count;
    if (part->kind == PART_SWITCH) {
        subject.set_switch = set_switch;
        subject.switch_positions = part->dip_switch->positions;
    }
    if (part->kind == PART_7SEG) {
        subject.lights = lights;
    }
    return trace_run(&subject, verbose);
}

static int by_name(const void *a, const void *b) {
    return strcmp((*(const Part *const *)a)->name, (*(const Part *const *)b)->name);
}

int main(int argc, char **argv) {
    PartLibrary *library = part_library_create();
    GoldenPart *golden;
    size_t golden_count;
    const Part **parts;
    size_t count;
    size_t i = 0;
    size_t g = 0;
    size_t compared = 0;
    size_t skipped = 0;
    size_t failures = 0;
    size_t per_kind[TRACE_LAST_KIND + 1] = {0};

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    if (argc == 3 && strcmp(argv[1], "--verbose") == 0) {
        const Part *part = part_library_find(library, argv[2]);

        if (part == NULL) {
            fprintf(stderr, "unknown part %s\n", argv[2]);
            return 1;
        }
        printf("%s\t%016llx\n", part->name, (unsigned long long)trace_part(part, stdout));
        return 0;
    }

    count = part_library_count(library);
    parts = malloc((count + 1) * sizeof(*parts));
    for (i = 0; i < count; i++) {
        parts[i] = part_library_at(library, i);
    }
    qsort(parts, count, sizeof(*parts), by_name);
    /* --print: the current hashes, to regenerate tests/golden/traces.txt. */
    if (argc == 2 && strcmp(argv[1], "--print") == 0) {
        for (i = 0; i < count; i++) {
            printf("%s\t%016llx\n", parts[i]->name, (unsigned long long)trace_part(parts[i], NULL));
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
        order = i >= count ? 1 : g >= golden_count ? -1 : strcmp(parts[i]->name, golden[g].name);
        if (order < 0) {
            if (failures++ < 10) {
                printf("extra part (not in the golden): %s\n", parts[i]->name);
            }
            i++;
        } else if (order > 0) {
            if (failures++ < 10) {
                printf("missing part: %s\n", golden[g].name);
            }
            g++;
        } else {
            char hash[32];

            snprintf(hash, sizeof hash, "%016llx", (unsigned long long)trace_part(parts[i], NULL));
            if (strcmp(hash, golden[g].hash) != 0 && failures++ < 10) {
                printf("trace differs: %s (%s): new %s, old %s\n", parts[i]->name,
                       listing_kind_name(parts[i]->kind), hash, golden[g].hash);
            }
            per_kind[parts[i]->kind]++;
            compared++;
            i++;
            g++;
        }
    }

    for (i = 0; i <= TRACE_LAST_KIND; i++) {
        if (per_kind[i] > 0) {
            printf("  %s %zu\n", listing_kind_name((PartKind)i), per_kind[i]);
        }
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
    free(parts);
    part_library_destroy(library);
    return 0;
}
