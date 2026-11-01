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
    /* Set when two strong drivers on this net disagree (a 0 and a 1). */
    int shorted;
    /* Position in the simulation's net list, for grouping nets joined by contacts. */
    size_t index;
    /* Times the value has changed since sim_start. */
    uint64_t changes;
};

const char *sim_net_name(const SimNet *net);
LogicValue sim_net_value(const SimNet *net);
/* Times the net's value has changed since sim_start; 0 for NULL. */
uint64_t sim_net_changes(const SimNet *net);

/* Internal functions used by the simulation engine. */
SimNet *net_create(Sim *owner, const char *name);
void net_destroy(SimNet *net);
int net_attach(SimNet *net, SimInstance *instance, size_t pin);
void net_resolve(SimNet *net);

/*
 * Resolves nets joined by closed contacts as one net: every driver on any of
 * them counts, and each gets the same value and short flag.
 */
void net_resolve_group(SimNet *const *nets, size_t count);

#endif
