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

/* One bit per driven value (LOGIC_0, LOGIC_1, LOGIC_X). */
#define NET_MASK(value) (1u << (value))

/*
 * The values driven on the nets, as masks: strong gets a bit per value some
 * pin drives strong, weak per value some pin drives weak. Z drives are left
 * out. net_resolve_group is these masks read by net_mask_value.
 */
void net_drivers(SimNet *const *nets, size_t count,
                 unsigned *strong, unsigned *weak);
/* Z for an empty mask, the value for a single bit, else X. */
LogicValue net_mask_value(unsigned mask);
/* Whether a strong mask holds both a 0 and a 1 (a short). */
int net_mask_shorted(unsigned strong);

#endif
