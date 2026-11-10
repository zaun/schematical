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
    net_resolve_group(&net, 1);
}

void net_drivers(SimNet *const *nets, size_t count,
                 unsigned *strong, unsigned *weak) {
    unsigned s = 0;
    unsigned w = 0;
    size_t n;
    size_t i;

    for (n = 0; n < count; n++) {
        const SimNet *net = nets[n];

        for (i = 0; i < net->pin_count; i++) {
            const SimNetPin *pin = &net->pins[i];

            if (pin->drive == LOGIC_Z) {
                continue;
            }
            if (pin->strength == DRIVE_STRONG) {
                s |= NET_MASK(pin->drive);
            } else {
                w |= NET_MASK(pin->drive);
            }
        }
    }
    *strong = s;
    *weak = w;
}

LogicValue net_mask_value(unsigned mask) {
    switch (mask) {
    case 0:
        return LOGIC_Z;
    case NET_MASK(LOGIC_0):
        return LOGIC_0;
    case NET_MASK(LOGIC_1):
        return LOGIC_1;
    default:
        return LOGIC_X;
    }
}

int net_mask_shorted(unsigned strong) {
    return (strong & NET_MASK(LOGIC_0)) && (strong & NET_MASK(LOGIC_1));
}

void net_resolve_group(SimNet *const *nets, size_t count) {
    unsigned strong;
    unsigned weak;
    LogicValue result;
    int shorted;
    size_t n;

    net_drivers(nets, count, &strong, &weak);
    result = net_mask_value(strong != 0 ? strong : weak);
    shorted = net_mask_shorted(strong);

    for (n = 0; n < count; n++) {
        nets[n]->value = result;
        nets[n]->shorted = shorted;
    }
}

const char *sim_net_name(const SimNet *net) {
    return net != NULL ? net->name : NULL;
}

LogicValue sim_net_value(const SimNet *net) {
    return net != NULL ? net->value : LOGIC_X;
}

uint64_t sim_net_changes(const SimNet *net) {
    return net != NULL ? net->changes : 0;
}
