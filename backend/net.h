#ifndef NET_H
#define NET_H

#include "sim.h"

typedef struct {
    SimInstance *instance;
    size_t pin;
    LogicValue drive;
    DriveStrength strength;
} SimNetPin;

struct SimNet {
    Sim *owner;
    char *name;
    SimNetPin *pins;
    size_t pin_count;
    size_t pin_capacity;
    LogicValue value;
};

const char *sim_net_name(const SimNet *net);
LogicValue sim_net_value(const SimNet *net);

/* Internal functions used by the simulation engine. */
SimNet *net_create(Sim *owner, const char *name);
void net_destroy(SimNet *net);
int net_attach(SimNet *net, SimInstance *instance, size_t pin);
void net_resolve(SimNet *net);

#endif
