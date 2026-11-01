#ifndef NETLIST_SIM_H
#define NETLIST_SIM_H

/*
 * Runs the simulator on a netlist given as text, so the schematic app can keep
 * one simulation alive and advance it in slices while the circuit is running.
 *
 * Input, one directive per line. Part and net indices start at 0:
 *
 *     nets <count>
 *     part <index> <part number>          e.g. "part 0 LED-RED-0603"
 *     pin <part index> <pin number> <net> pin numbers are 1-based, as on the part
 *
 * Parts must be declared before their pins. A pin that is not listed is
 * unconnected and reads as Z.
 *
 * Snapshot, one item per line:
 *
 *     time <picoseconds>
 *     net <index> <0|1|X|Z>
 *     lit <part index> <0|1>     LEDs only: anode high and cathode low
 *     shorted <net index>        present when two strong drivers disagree
 *     error <message>            present when the simulation has stopped
 */

#include <stdint.h>

#include "sim.h"

typedef struct NetlistSim NetlistSim;

/*
 * Builds the simulation and starts it. Returns NULL on failure and, when
 * error_out is not NULL, stores a newly allocated message in *error_out.
 * Free that message with free().
 */
NetlistSim *netlist_sim_open(const char *text, char **error_out);

/*
 * Runs the simulation forward by the given amount of simulated time. Returns 0
 * once the simulation has stopped with an error. The last state stays readable.
 */
int netlist_sim_advance(NetlistSim *sim, SimTime picoseconds);

/*
 * Returns the current state as a newly allocated, NUL-terminated string, or
 * NULL on allocation failure. Free it with free().
 */
char *netlist_sim_snapshot(const NetlistSim *sim);

/*
 * Memory parts (SRAMs). Returns 1 and fills the number of words and the word
 * width in bits when the part is a memory, 0 otherwise.
 */
int netlist_sim_memory_size(
    const NetlistSim *sim,
    size_t part,
    uint32_t *word_count,
    unsigned *word_bits
);

/*
 * Reads count words of a memory part, starting at word address start.
 * values[i] gets the word's bits and known[i] a 1 for every bit that is
 * defined (0 = X; an X bit reads as 0 in values). Either array may be NULL.
 * Returns the number of words read: fewer than count when the range runs
 * past the end of the memory, 0 when the part is not a memory or start is
 * out of range.
 */
size_t netlist_sim_inspect_memory(
    const NetlistSim *sim,
    size_t part,
    uint32_t start,
    size_t count,
    uint32_t *values,
    uint32_t *known
);

void netlist_sim_close(NetlistSim *sim);

#endif
