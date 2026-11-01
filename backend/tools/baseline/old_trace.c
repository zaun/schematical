/*
 * old_trace: one line per orderable part of the old backend,
 * "name<TAB>hash", sorted by name (plan-01-A section 4.3).
 *   old_trace                    all parts
 *   old_trace --verbose <part>   that part's change log, then its hash
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "led7.h"
#include "old_parts.h"
#include "switch.h"
#include "trace_core.h"

static TraceRole trace_role(const ChipPin *pin) {
    if (pin->group != NULL && strcmp(pin->group, CHIP_PIN_GROUP_HIDDEN) == 0) {
        return TRACE_NONE;
    }
    switch (pin->role) {
        case PIN_ROLE_INPUT: return TRACE_INPUT;
        case PIN_ROLE_INOUT: return TRACE_INOUT;
        case PIN_ROLE_POWER: return TRACE_POWER;
        case PIN_ROLE_GROUND: return TRACE_GROUND;
        case PIN_ROLE_OUTPUT:
        case PIN_ROLE_NO_CONNECT: return TRACE_NONE;
    }
    return TRACE_NONE;
}

static uint64_t trace_part(const OldPart *part, FILE *verbose) {
    TraceRole roles[256];
    TraceSubject subject = {0};
    size_t i;

    if (part->pin_count > 256) {
        fprintf(stderr, "%s: too many pins\n", part->name);
        exit(2);
    }
    for (i = 0; i < part->pin_count; i++) {
        roles[i] = trace_role(&part->pins[i]);
    }
    subject.name = part->name;
    subject.type = part->type;
    subject.config = old_part_config(part);
    subject.roles = roles;
    subject.pin_count = part->pin_count;
    if (strcmp(part->kind, "switch") == 0) {
        subject.set_switch = switch_set_position;
        subject.switch_positions = switch_config_positions(&part->logic);
    }
    if (strcmp(part->kind, "7seg") == 0) {
        subject.lights = led7_instance_glow;
    }
    return trace_run(&subject, verbose);
}

int main(int argc, char **argv) {
    OldPart *parts;
    size_t count = old_parts_collect(&parts);
    size_t i;

    if (argc == 3 && strcmp(argv[1], "--verbose") == 0) {
        for (i = 0; i < count; i++) {
            if (strcmp(parts[i].name, argv[2]) == 0) {
                printf("%s\t%016llx\n", parts[i].name,
                       (unsigned long long)trace_part(&parts[i], stdout));
                return 0;
            }
        }
        fprintf(stderr, "unknown part %s\n", argv[2]);
        return 1;
    }
    if (argc != 1) {
        fprintf(stderr, "usage: old_trace [--verbose <part>]\n");
        return 2;
    }
    for (i = 0; i < count; i++) {
        printf("%s\t%016llx\n", parts[i].name, (unsigned long long)trace_part(&parts[i], NULL));
    }
    return 0;
}
