#include "net.h"

#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        text = "";
    }

    length = strlen(text) + 1;
    copy = malloc(length);

    if (copy != NULL) {
        memcpy(copy, text, length);
    }

    return copy;
}

SimNet *net_create(Sim *owner, const char *name) {
    SimNet *net = calloc(1, sizeof(*net));

    if (net == NULL) {
        return NULL;
    }

    net->name = copy_string(name);
    if (net->name == NULL) {
        free(net);
        return NULL;
    }

    net->owner = owner;
    net->value = LOGIC_Z;
    return net;
}

void net_destroy(SimNet *net) {
    if (net == NULL) {
        return;
    }

    free(net->pins);
    free(net->name);
    free(net);
}

int net_attach(SimNet *net, SimInstance *instance, size_t pin) {
    SimNetPin *new_pins;
    size_t capacity;

    if (net->pin_count == net->pin_capacity) {
        capacity = net->pin_capacity == 0 ? 4 : net->pin_capacity * 2;

        if (capacity < net->pin_capacity ||
            capacity > SIZE_MAX / sizeof(*new_pins)) {
            return 0;
        }

        new_pins = realloc(net->pins, capacity * sizeof(*new_pins));
        if (new_pins == NULL) {
            return 0;
        }

        net->pins = new_pins;
        net->pin_capacity = capacity;
    }

    net->pins[net->pin_count].instance = instance;
    net->pins[net->pin_count].pin = pin;
    net->pins[net->pin_count].drive = LOGIC_Z;
    net->pins[net->pin_count].strength = DRIVE_STRONG;
    net->pin_count++;

    return 1;
}

void net_resolve(SimNet *net) {
    int strongest = -1;
    LogicValue result = LOGIC_Z;
    int strong_high = 0;
    int strong_low = 0;
    size_t i;

    for (i = 0; i < net->pin_count; i++) {
        const SimNetPin *pin = &net->pins[i];
        int strength;

        if (pin->drive == LOGIC_Z) {
            continue;
        }

        if (pin->strength == DRIVE_STRONG) {
            if (pin->drive == LOGIC_1) {
                strong_high = 1;
            } else if (pin->drive == LOGIC_0) {
                strong_low = 1;
            }
        }

        strength = (int)pin->strength;

        if (strength > strongest) {
            strongest = strength;
            result = pin->drive;
        } else if (strength == strongest && result != pin->drive) {
            result = LOGIC_X;
        }
    }

    net->value = result;
    net->shorted = strong_high && strong_low;
}

const char *sim_net_name(const SimNet *net) {
    return net != NULL ? net->name : NULL;
}

LogicValue sim_net_value(const SimNet *net) {
    return net != NULL ? net->value : LOGIC_X;
}
