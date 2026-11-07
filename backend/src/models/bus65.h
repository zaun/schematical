#ifndef BUS65_H
#define BUS65_H

/*
 * bus65: the 65xx peripheral bus (plan-14-A section 4.3), shared by the pio
 * models (W65C21, W65C22). Not a component: the model calls it from its
 * pin_changed and timer hooks.
 *
 * At each PHI2 rise the bus samples the chip selects, RWB and the register
 * selects. Selected = every cs_high reads 1 and every cs_low reads 0. A
 * select, RWB or RS that reads X or Z, or changed less than tacr (read) /
 * tacw (write) before the rise, makes the cycle unknown.
 *
 *   read   read() at the rise; data X from the rise, the value at rise +
 *          tcdr, driven until fall + thr, then Z. An unknown read drives X
 *          for the same window and calls read_unknown() instead of read()
 *          (a read can have side effects: clearing flags).
 *   write  data sampled at the PHI2 fall (a bit that changed less than tdcw
 *          before it is X); write() at the fall. A bit that changes within
 *          thw after the fall calls write() again with that bit unknown. An
 *          unknown write calls write(reg, 0, 0) on every register.
 *   hold   a select, RWB or RS change within tcar (read) / tcaw (write) of
 *          the rise, or at the rise itself, makes the cycle unknown from then
 *          on (a read in progress drives X until fall + thr).
 *   PHI2 X every register write(reg, 0, 0); the data pins X until PHI2 is
 *          known again.
 *   widths (when tpwh, tpwl or tcyc is nonzero) a high or low phase shorter
 *          than tpwh / tpwl, or rises closer than tcyc: every register
 *          unknown, as PHI2 X, and the cycle is unknown.
 *   cycle  cycle() at every PHI2 fall, selected or not, after read/write.
 *
 * With bus_hold set (the W65C22S, plan-14-C), a select, RWB, RS or data pin
 * that reads Z reads the last known level it had (X before the first).
 */

#include <stddef.h>
#include <stdint.h>

#include "sim.h"

enum {
    BUS65_DATA = 8,
    BUS65_MAX_CS = 2,
    BUS65_MAX_RS = 4,
    BUS65_MAX_REGS = 1 << BUS65_MAX_RS
};

/* Timer ids the bus uses: every id with these top bits. Models keep theirs
   below BUS65_TIMER. */
#define BUS65_TIMER 0x40000000u
#define BUS65_TIMER_MASK 0xC0000000u

typedef struct {
    size_t phi2, rwb, data[BUS65_DATA];
    size_t cs_high[BUS65_MAX_CS], cs_high_count;  /* active-high selects (W65C21 CS0, CS1; W65C22 CS1) */
    size_t cs_low[BUS65_MAX_CS], cs_low_count;    /* active-low selects (CS2B) */
    size_t rs[BUS65_MAX_RS], rs_count;            /* register selects, RS0 first */
} Bus65Pins;

typedef struct {
    SimTime tacr, tcar, tcdr, thr, tacw, tcaw, tdcw, thw;  /* ps, from the grade */
    SimTime tcyc, tpwh, tpwl;                              /* 0 = not checked */
} Bus65Timing;

typedef struct {
    /* A read of reg at the PHI2 rise. 1 = drive value (bits known = 0 drive X). */
    int (*read)(void *model, unsigned reg, uint8_t *value, uint8_t *known);
    /* A write at the PHI2 fall; known: the defined bits of value. */
    void (*write)(void *model, unsigned reg, uint8_t value, uint8_t known);
    /* Every PHI2 fall, after read/write (the VIA's timers count here). May be NULL. */
    void (*cycle)(void *model);
    /* An unknown read cycle (any register may have been read): the model
       makes every read side effect unknown. Called at the rise, or when a
       hold violation turns the cycle unknown. May be NULL. */
    void (*read_unknown)(void *model);
} Bus65Ops;

typedef enum {
    BUS65_IDLE,      /* not selected */
    BUS65_READ,
    BUS65_WRITE,
    BUS65_UNKNOWN    /* selected or not, read or write: not known */
} Bus65Cycle;

typedef struct {
    const Bus65Pins *pins;
    Bus65Timing timing;
    const Bus65Ops *ops;
    void *model;
    int bus_hold;

    LogicValue phi2;           /* the level last seen */
    int edges;                 /* bit 0: a rise seen, bit 1: a fall seen */
    SimTime last_rise, last_fall;

    /* The cycle sampled at the last rise. */
    Bus65Cycle cycle;
    int rwb;                   /* RWB at the rise: 0, 1, or -1 unknown */
    unsigned reg;
    LogicValue cs_value[2 * BUS65_MAX_CS];   /* each select at the rise: 1 = active, 0, X */

    /* Change times of the controls (selects, then RWB, then RS) and the data. */
    SimTime control_at[2 * BUS65_MAX_CS + 1 + BUS65_MAX_RS];
    int control_seen[2 * BUS65_MAX_CS + 1 + BUS65_MAX_RS];
    SimTime data_at[BUS65_DATA];
    int data_seen[BUS65_DATA];
    LogicValue held[2 * BUS65_MAX_CS + 1 + BUS65_MAX_RS + BUS65_DATA];  /* bus hold levels */

    /* The read drive. */
    uint32_t seq;              /* bumps on each new drive window: stale timers drop */
    int driving;               /* the data pins are driven (not yet released) */
    int read_valid;            /* the value of the read (not X) */
    uint8_t read_value, read_known;

    /* The write just done, for the hold check. */
    int wrote;
    uint8_t write_value, write_known;
} Bus65;

void bus65_init(Bus65 *bus, const Bus65Pins *pins, const Bus65Timing *timing,
                const Bus65Ops *ops, void *model);

/* The register count: 1 << rs_count. */
unsigned bus65_reg_count(const Bus65 *bus);

/* Reads PHI2 and the data pins' starting levels; drives nothing. */
void bus65_start(Bus65 *bus, Sim *sim, SimInstance *instance);

/* 1 when pin is one of the bus's pins (the call handled it). */
int bus65_pin_changed(Bus65 *bus, Sim *sim, SimInstance *instance, size_t pin,
                      LogicValue old_value, LogicValue new_value);

/* 1 when timer_id is the bus's (the call handled it). */
int bus65_timer(Bus65 *bus, Sim *sim, SimInstance *instance, uint32_t timer_id);

/* A pin's level as an input: 0, 1 or X. Z reads X, or with bus hold the
   last known level of a bus pin. */
LogicValue bus65_level(const Bus65 *bus, const Sim *sim, const SimInstance *instance, size_t pin);

#endif
